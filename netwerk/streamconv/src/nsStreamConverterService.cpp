/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK *****
 *
 *
 * This Original Code has been modified by IBM Corporation.
 * Modifications made by IBM described herein are
 * Copyright (c) International Business Machines
 * Corporation, 2000
 *
 * Modifications to Mozilla code or documentation
 * identified per MPL Section 3.3
 *
 * Date         Modified by     Description of modification
 * 03/27/2000   IBM Corp.       Added PR_CALLBACK for Optlink
 *                               use in OS2
 */

#include "nsStreamConverterService.h"
#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsIComponentRegistrar.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsIAtom.h"
#include "nsDeque.h"
#include "nsIInputStream.h"
#include "nsIOutputStream.h"
#include "nsIStreamConverter.h"
#include "nsICategoryManager.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsXPIDLString.h"

////////////////////////////////////////////////////////////
// nsISupports methods
NS_IMPL_ISUPPORTS1(nsStreamConverterService, nsIStreamConverterService)


////////////////////////////////////////////////////////////
// nsIStreamConverterService methods

////////////////////////////////////////////////////////////
// nsStreamConverterService methods
nsStreamConverterService::nsStreamConverterService() : mAdjacencyList(nsnull) {
}

nsStreamConverterService::~nsStreamConverterService() {
    NS_ASSERTION(mAdjacencyList, "init wasn't called, or the retval was ignored");
    delete mAdjacencyList;
}

// Delete all the entries in the adjacency list
static PRBool PR_CALLBACK DeleteAdjacencyEntry(nsHashKey *aKey, void *aData, void* closure) {
    SCTableData *entry = (SCTableData*)aData;
    NS_ASSERTION(entry->key && entry->data.edges, "malformed adjacency list entry");
    delete entry->key;
    delete entry->data.edges;
    delete entry;
    return PR_TRUE;   
}

nsresult
nsStreamConverterService::Init() {
    mAdjacencyList = new nsObjectHashtable(nsnull, nsnull,
                                           DeleteAdjacencyEntry, nsnull);
    if (!mAdjacencyList) return NS_ERROR_OUT_OF_MEMORY;
    return NS_OK;
}

// Builds the graph represented as an adjacency list (and built up in 
// memory using an nsObjectHashtable and nsISupportsArray combination).
//
// :BuildGraph() consults the category manager for all stream converter 
// CONTRACTIDS then fills the adjacency list with edges.
// An edge in this case is comprised of a FROM and TO MIME type combination.
// 
// CONTRACTID format:
// @mozilla.org/streamconv;1?from=text/html&to=text/plain
// XXX curently we only handle a single from and to combo, we should repeat the 
// XXX registration process for any series of from-to combos.
// XXX can use nsTokenizer for this.
//

nsresult
nsStreamConverterService::BuildGraph() {

    nsresult rv;

    nsCOMPtr<nsICategoryManager> catmgr(do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsISimpleEnumerator> entries;
    rv = catmgr->EnumerateCategory(NS_ISTREAMCONVERTER_KEY, getter_AddRefs(entries));
    if (NS_FAILED(rv)) return rv;

    // go through each entry to build the graph
    nsCOMPtr<nsISupportsCString> entry;
    rv = entries->GetNext(getter_AddRefs(entry));
    while (NS_SUCCEEDED(rv)) {

        // get the entry string
        nsCAutoString entryString;
        rv = entry->GetData(entryString);
        if (NS_FAILED(rv)) return rv;
        
        // cobble the entry string w/ the converter key to produce a full contractID.
        nsCAutoString contractID(NS_ISTREAMCONVERTER_KEY);
        contractID.Append(entryString);

        // now we've got the CONTRACTID, let's parse it up.
        rv = AddAdjacency(contractID.get());
        if (NS_FAILED(rv)) return rv;

        rv = entries->GetNext(getter_AddRefs(entry));
    }

    return NS_OK;
}


// XXX currently you can not add the same adjacency (i.e. you can't have multiple
// XXX stream converters registering to handle the same from-to combination. It's
// XXX not programatically prohibited, it's just that results are un-predictable
// XXX right now.
nsresult
nsStreamConverterService::AddAdjacency(const char *aContractID) {
    nsresult rv;
    // first parse out the FROM and TO MIME-types.

    nsCAutoString fromStr, toStr;
    rv = ParseFromTo(aContractID, fromStr, toStr);
    if (NS_FAILED(rv)) return rv;

    // Each MIME-type is a vertex in the graph, so first lets make sure
    // each MIME-type is represented as a key in our hashtable.

    nsCStringKey fromKey(fromStr);
    SCTableData *fromEdges = (SCTableData*)mAdjacencyList->Get(&fromKey);
    if (!fromEdges) {
        // There is no fromStr vertex, create one.

        nsCStringKey *newFromKey = new nsCStringKey(ToNewCString(fromStr), fromStr.Length(), nsCStringKey::OWN);
        if (!newFromKey) return NS_ERROR_OUT_OF_MEMORY;

        SCTableData *data = new SCTableData(newFromKey);
        if (!data) {
            delete newFromKey;
            return NS_ERROR_OUT_OF_MEMORY;
        }

        nsCOMArray<nsIAtom>* edgeArray = new nsCOMArray<nsIAtom>;
        if (!edgeArray) {
            delete newFromKey;
            data->key = nsnull;
            delete data;
            return NS_ERROR_OUT_OF_MEMORY;
        }
        data->data.edges = edgeArray;

        mAdjacencyList->Put(newFromKey, data);
        fromEdges = data;
    }

    nsCStringKey toKey(toStr);
    if (!mAdjacencyList->Get(&toKey)) {
        // There is no toStr vertex, create one.
        nsCStringKey *newToKey = new nsCStringKey(ToNewCString(toStr), toStr.Length(), nsCStringKey::OWN);
        if (!newToKey) return NS_ERROR_OUT_OF_MEMORY;

        SCTableData *data = new SCTableData(newToKey);
        if (!data) {
            delete newToKey;
            return NS_ERROR_OUT_OF_MEMORY;
        }

        nsCOMArray<nsIAtom>* edgeArray = new nsCOMArray<nsIAtom>;
        if (!edgeArray) {
            delete newToKey;
            data->key = nsnull;
            delete data;
            return NS_ERROR_OUT_OF_MEMORY;
        }
        data->data.edges = edgeArray;
        mAdjacencyList->Put(newToKey, data);
    }
    
    // Now we know the FROM and TO types are represented as keys in the hashtable.
    // Let's "connect" the verticies, making an edge.

    nsCOMPtr<nsIAtom> vertex = do_GetAtom(toStr.get()); 
    if (!vertex) return NS_ERROR_OUT_OF_MEMORY;

    NS_ASSERTION(fromEdges, "something wrong in adjacency list construction");
    if (!fromEdges)
        return NS_ERROR_FAILURE;

    nsCOMArray<nsIAtom> *adjacencyList = fromEdges->data.edges;
    return adjacencyList->AppendObject(vertex) ? NS_OK : NS_ERROR_FAILURE;
}

nsresult
nsStreamConverterService::ParseFromTo(const char *aContractID, nsCString &aFromRes, nsCString &aToRes) {

    nsCAutoString ContractIDStr(aContractID);

    PRInt32 fromLoc = ContractIDStr.Find("from=");
    PRInt32 toLoc   = ContractIDStr.Find("to=");
    if (-1 == fromLoc || -1 == toLoc ) return NS_ERROR_FAILURE;

    fromLoc = fromLoc + 5;
    toLoc = toLoc + 3;

    nsCAutoString fromStr, toStr;

    ContractIDStr.Mid(fromStr, fromLoc, toLoc - 4 - fromLoc);
    ContractIDStr.Mid(toStr, toLoc, ContractIDStr.Length() - toLoc);

    aFromRes.Assign(fromStr);
    aToRes.Assign(toStr);

    return NS_OK;
}

// nsObjectHashtable enumerator functions.

// Initializes the BFS state table.
static PRBool PR_CALLBACK InitBFSTable(nsHashKey *aKey, void *aData, void* closure) {
    NS_ASSERTION((SCTableData*)aData, "no data in the table enumeration");
    
    nsHashtable *BFSTable = (nsHashtable*)closure;
    if (!BFSTable) return PR_FALSE;

    BFSState *state = new BFSState;
    if (!state) return PR_FALSE;

    state->color = white;
    state->distance = -1;
    state->predecessor = nsnull;

    SCTableData *data = new SCTableData(static_cast<nsCStringKey*>(aKey));
    if (!data) {
        delete state;
        return PR_FALSE;
    }
    data->data.state = state;

    BFSTable->Put(aKey, data);
    return PR_TRUE;   
}

// cleans up the BFS state table
static PRBool PR_CALLBACK DeleteBFSEntry(nsHashKey *aKey, void *aData, void *closure) {
    SCTableData *data = (SCTableData*)aData;
    BFSState *state = data->data.state;
    delete state;
    data->key = nsnull;
    delete data;
    return PR_TRUE;
}

class CStreamConvDeallocator : public nsDequeFunctor {
public:
    virtual void* operator()(void* anObject) {
        nsCStringKey *key = (nsCStringKey*)anObject;
        delete key;
        return 0;
    }
};

// walks the graph using a breadth-first-search algorithm which generates a discovered
// verticies tree. This tree is then walked up (from destination vertex, to origin vertex)
// and each link in the chain is added to an nsStringArray. A direct lookup for the given
// CONTRACTID should be made prior to calling this method in an attempt to find a direct
// converter rather than walking the graph.
nsresult
nsStreamConverterService::FindConverter(const char *aContractID, nsCStringArray **aEdgeList) {
    nsresult rv;
    if (!aEdgeList) return NS_ERROR_NULL_POINTER;
    *aEdgeList = nsnull;

    // walk the graph in search of the appropriate converter.

    PRInt32 vertexCount = mAdjacencyList->Count();
    if (0 >= vertexCount) return NS_ERROR_FAILURE;

    // Create a corresponding color table for each vertex in the graph.
    nsObjectHashtable lBFSTable(nsnull, nsnull, DeleteBFSEntry, nsnull);
    mAdjacencyList->Enumerate(InitBFSTable, &lBFSTable);

    NS_ASSERTION(lBFSTable.Count() == vertexCount, "strmconv BFS table init problem");

    // This is our source vertex; our starting point.
    nsCAutoString fromC, toC;
    rv = ParseFromTo(aContractID, fromC, toC);
    if (NS_FAILED(rv)) return rv;

    nsCStringKey *source = new nsCStringKey(fromC.get());
    if (!source) return NS_ERROR_OUT_OF_MEMORY;

    SCTableData *data = (SCTableData*)lBFSTable.Get(source);
    if (!data) {
        delete source;
        return NS_ERROR_FAILURE;
    }

    BFSState *state = data->data.state;

    state->color = gray;
    state->distance = 0;
    CStreamConvDeallocator *dtorFunc = new CStreamConvDeallocator();
    if (!dtorFunc) {
        delete source;
        return NS_ERROR_OUT_OF_MEMORY;
    }

    nsDeque grayQ(dtorFunc);

    // Now generate the shortest path tree.
    grayQ.Push(source);
    while (0 < grayQ.GetSize()) {
        nsCStringKey *currentHead = (nsCStringKey*)grayQ.PeekFront();
        SCTableData *data2 = (SCTableData*)mAdjacencyList->Get(currentHead);
        if (!data2) return NS_ERROR_FAILURE;

        nsCOMArray<nsIAtom> *edges = data2->data.edges;
        NS_ASSERTION(edges, "something went wrong with BFS strmconv algorithm");
        if (!edges) return NS_ERROR_FAILURE;

        // Get the state of the current head to calculate the distance of each
        // reachable vertex in the loop.
        data2 = (SCTableData*)lBFSTable.Get(currentHead);
        if (!data2) return NS_ERROR_FAILURE;

        BFSState *headVertexState = data2->data.state;
        NS_ASSERTION(headVertexState, "problem with the BFS strmconv algorithm");
        if (!headVertexState) return NS_ERROR_FAILURE;

        PRInt32 edgeCount = edges->Count();

        for (PRInt32 i = 0; i < edgeCount; i++) {
            nsIAtom* curVertexAtom = edges->ObjectAt(i);
            nsAutoString curVertexStr;
            curVertexAtom->ToString(curVertexStr);
            nsCStringKey *curVertex = new nsCStringKey(ToNewCString(curVertexStr), 
                                        curVertexStr.Length(), nsCStringKey::OWN);
            if (!curVertex) return NS_ERROR_OUT_OF_MEMORY;

            SCTableData *data3 = (SCTableData*)lBFSTable.Get(curVertex);
            if (!data3) {
                delete curVertex;
                return NS_ERROR_FAILURE;
            }
            BFSState *curVertexState = data3->data.state;
            NS_ASSERTION(curVertexState, "something went wrong with the BFS strmconv algorithm");
            if (!curVertexState) return NS_ERROR_FAILURE;

            if (white == curVertexState->color) {
                curVertexState->color = gray;
                curVertexState->distance = headVertexState->distance + 1;
                curVertexState->predecessor = (nsCStringKey*)currentHead->Clone();
                if (!curVertexState->predecessor) {
                    delete curVertex;
                    return NS_ERROR_OUT_OF_MEMORY;
                }
                grayQ.Push(curVertex);
            } else {
                delete curVertex; // if this vertex has already been discovered, we don't want
                                  // to leak it. (non-discovered vertex's get cleaned up when
                                  // they're popped).
            }
        }
        headVertexState->color = black;
        nsCStringKey *cur = (nsCStringKey*)grayQ.PopFront();
        delete cur;
        cur = nsnull;
    }
    // The shortest path (if any) has been generated and is represetned by the chain of 
    // BFSState->predecessor keys. Start at the bottom and work our way up.

    // first parse out the FROM and TO MIME-types being registered.

    nsCAutoString fromStr, toStr;
    rv = ParseFromTo(aContractID, fromStr, toStr);
    if (NS_FAILED(rv)) return rv;

    // get the root CONTRACTID
    nsCAutoString ContractIDPrefix(NS_ISTREAMCONVERTER_KEY);
    nsCStringArray *shortestPath = new nsCStringArray();
    if (!shortestPath) return NS_ERROR_OUT_OF_MEMORY;

    nsCStringKey toMIMEType(toStr);
    data = (SCTableData*)lBFSTable.Get(&toMIMEType);
    if (!data) {
        // If this vertex isn't in the BFSTable, then no-one has registered for it,
        // therefore we can't do the conversion.
        delete shortestPath;
        return NS_ERROR_FAILURE;
    }

    while (data) {
        BFSState *curState = data->data.state;

        nsCStringKey *key = data->key;
        
        if (fromStr.Equals(key->GetString())) {
            // found it. We're done here.
            *aEdgeList = shortestPath;
            return NS_OK;
        }

        // reconstruct the CONTRACTID.
        // Get the predecessor.
        if (!curState->predecessor) break; // no predecessor
        SCTableData *predecessorData = (SCTableData*)lBFSTable.Get(curState->predecessor);

        if (!predecessorData) break; // no predecessor, chain doesn't exist.

        // build out the CONTRACTID.
        nsCAutoString newContractID(ContractIDPrefix);
        newContractID.AppendLiteral("?from=");

        nsCStringKey *predecessorKey = predecessorData->key;
        newContractID.Append(predecessorKey->GetString());

        newContractID.AppendLiteral("&to=");
        newContractID.Append(key->GetString());
    
        // Add this CONTRACTID to the chain.
        rv = shortestPath->AppendCString(newContractID) ? NS_OK : NS_ERROR_FAILURE;  // XXX this method incorrectly returns a bool
        NS_ASSERTION(NS_SUCCEEDED(rv), "AppendElement failed");

        // move up the tree.
        data = predecessorData;
    }
    delete shortestPath;
    return NS_ERROR_FAILURE; // couldn't find a stream converter or chain.
}


/////////////////////////////////////////////////////
// nsIStreamConverterService methods
NS_IMETHODIMP
nsStreamConverterService::CanConvert(const char* aFromType,
                                     const char* aToType,
                                     PRBool* _retval) {
    nsCOMPtr<nsIComponentRegistrar> reg;
    nsresult rv = NS_GetComponentRegistrar(getter_AddRefs(reg));
    if (NS_FAILED(rv))
        return rv;

    nsCAutoString contractID;
    contractID.AssignLiteral(NS_ISTREAMCONVERTER_KEY "?from=");
    contractID.Append(aFromType);
    contractID.AppendLiteral("&to=");
    contractID.Append(aToType);

    // See if we have a direct match
    rv = reg->IsContractIDRegistered(contractID.get(), _retval);
    if (NS_FAILED(rv))
        return rv;
    if (*_retval)
        return NS_OK;

    // Otherwise try the graph.
    rv = BuildGraph();
    if (NS_FAILED(rv))
        return rv;

    nsCStringArray *converterChain = nsnull;
    rv = FindConverter(contractID.get(), &converterChain);
    *_retval = NS_SUCCEEDED(rv);

    delete converterChain;
    return NS_OK;
}

NS_IMETHODIMP
nsStreamConverterService::Convert(nsIInputStream *aFromStream,
                                  const char *aFromType, 
                                  const char *aToType,
                                  nsISupports *aContext,
                                  nsIInputStream **_retval) {
    if (!aFromStream || !aFromType || !aToType || !_retval) return NS_ERROR_NULL_POINTER;
    nsresult rv;

    // first determine whether we can even handle this conversion
    // build a CONTRACTID
    nsCAutoString contractID;
    contractID.AssignLiteral(NS_ISTREAMCONVERTER_KEY "?from=");
    contractID.Append(aFromType);
    contractID.AppendLiteral("&to=");
    contractID.Append(aToType);
    const char *cContractID = contractID.get();

    nsCOMPtr<nsIStreamConverter> converter(do_CreateInstance(cContractID, &rv));
    if (NS_FAILED(rv)) {
        // couldn't go direct, let's try walking the graph of converters.
        rv = BuildGraph();
        if (NS_FAILED(rv)) return rv;

        nsCStringArray *converterChain = nsnull;

        rv = FindConverter(cContractID, &converterChain);
        if (NS_FAILED(rv)) {
            // can't make this conversion.
            // XXX should have a more descriptive error code.
            return NS_ERROR_FAILURE;
        }

        PRInt32 edgeCount = converterChain->Count();
        NS_ASSERTION(edgeCount > 0, "findConverter should have failed");


        // convert the stream using each edge of the graph as a step.
        // this is our stream conversion traversal.
        nsCOMPtr<nsIInputStream> dataToConvert = aFromStream;
        nsCOMPtr<nsIInputStream> convertedData;

        for (PRInt32 i = edgeCount-1; i >= 0; i--) {
            nsCString *contractIDStr = converterChain->CStringAt(i);
            if (!contractIDStr) {
                delete converterChain;
                return NS_ERROR_FAILURE;
            }
            const char *lContractID = contractIDStr->get();

            converter = do_CreateInstance(lContractID, &rv);

            if (NS_FAILED(rv)) {
                delete converterChain;                
                return rv;
            }

            nsCAutoString fromStr, toStr;
            rv = ParseFromTo(lContractID, fromStr, toStr);
            if (NS_FAILED(rv)) {
                delete converterChain;
                return rv;
            }

            rv = converter->Convert(dataToConvert, fromStr.get(), toStr.get(), aContext, getter_AddRefs(convertedData));
            dataToConvert = convertedData;
            if (NS_FAILED(rv)) {
                delete converterChain;
                return rv;
            }
        }

        delete converterChain;
        *_retval = convertedData;
        NS_ADDREF(*_retval);

    } else {
        // we're going direct.
        rv = converter->Convert(aFromStream, aFromType, aToType, aContext, _retval);
    }
    
    return rv;
}


NS_IMETHODIMP
nsStreamConverterService::AsyncConvertData(const char *aFromType, 
                                           const char *aToType, 
                                           nsIStreamListener *aListener,
                                           nsISupports *aContext,
                                           nsIStreamListener **_retval) {
    if (!aFromType || !aToType || !aListener || !_retval) return NS_ERROR_NULL_POINTER;

    nsresult rv;

    // first determine whether we can even handle this conversion
    // build a CONTRACTID
    nsCAutoString contractID;
    contractID.AssignLiteral(NS_ISTREAMCONVERTER_KEY "?from=");
    contractID.Append(aFromType);
    contractID.AppendLiteral("&to=");
    contractID.Append(aToType);
    const char *cContractID = contractID.get();

    nsCOMPtr<nsIStreamConverter> listener(do_CreateInstance(cContractID, &rv));
    if (NS_FAILED(rv)) {
        // couldn't go direct, let's try walking the graph of converters.
        rv = BuildGraph();
        if (NS_FAILED(rv)) return rv;

        nsCStringArray *converterChain = nsnull;

        rv = FindConverter(cContractID, &converterChain);
        if (NS_FAILED(rv)) {
            // can't make this conversion.
            // XXX should have a more descriptive error code.
            return NS_ERROR_FAILURE;
        }

        // aListener is the listener that wants the final, converted, data.
        // we initialize finalListener w/ aListener so it gets put at the 
        // tail end of the chain, which in the loop below, means the *first*
        // converter created.
        nsCOMPtr<nsIStreamListener> finalListener = aListener;

        // convert the stream using each edge of the graph as a step.
        // this is our stream conversion traversal.
        PRInt32 edgeCount = converterChain->Count();
        NS_ASSERTION(edgeCount > 0, "findConverter should have failed");
        for (int i = 0; i < edgeCount; i++) {
            nsCString *contractIDStr = converterChain->CStringAt(i);
            if (!contractIDStr) {
                delete converterChain;
                return NS_ERROR_FAILURE;
            }
            const char *lContractID = contractIDStr->get();

            // create the converter for this from/to pair
            nsCOMPtr<nsIStreamConverter> converter(do_CreateInstance(lContractID, &rv));
            NS_ASSERTION(NS_SUCCEEDED(rv), "graph construction problem, built a contractid that wasn't registered");

            nsCAutoString fromStr, toStr;
            rv = ParseFromTo(lContractID, fromStr, toStr);
            if (NS_FAILED(rv)) {
                delete converterChain;
                return rv;
            }

            // connect the converter w/ the listener that should get the converted data.
            rv = converter->AsyncConvertData(fromStr.get(), toStr.get(), finalListener, aContext);
            if (NS_FAILED(rv)) {
                delete converterChain;
                return rv;
            }

            nsCOMPtr<nsIStreamListener> chainListener(do_QueryInterface(converter, &rv));
            if (NS_FAILED(rv)) {
                delete converterChain;
                return rv;
            }

            // the last iteration of this loop will result in finalListener
            // pointing to the converter that "starts" the conversion chain.
            // this converter's "from" type is the original "from" type. Prior
            // to the last iteration, finalListener will continuously be wedged
            // into the next listener in the chain, then be updated.
            finalListener = chainListener;
        }
        delete converterChain;
        // return the first listener in the chain.
        *_retval = finalListener;
        NS_ADDREF(*_retval);

    } else {
        // we're going direct.
        *_retval = listener;
        NS_ADDREF(*_retval);

        rv = listener->AsyncConvertData(aFromType, aToType, aListener, aContext);
    }
    
    return rv;

}

nsresult
NS_NewStreamConv(nsStreamConverterService** aStreamConv)
{
    NS_PRECONDITION(aStreamConv != nsnull, "null ptr");
    if (!aStreamConv) return NS_ERROR_NULL_POINTER;

    *aStreamConv = new nsStreamConverterService();
    if (!*aStreamConv) return NS_ERROR_OUT_OF_MEMORY;

    NS_ADDREF(*aStreamConv);
    nsresult rv = (*aStreamConv)->Init();
    if (NS_FAILED(rv))
        NS_RELEASE(*aStreamConv);

    return rv;
}
