/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
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
 * Portions created by the Initial Developer are Copyright (C) 200
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsWindowDataSource.h"
#include "nsIXULWindow.h"
#include "rdf.h"
#include "nsIRDFContainerUtils.h"
#include "nsIServiceManager.h"
#include "nsReadableUtils.h"
#include "nsIObserverService.h"
#include "nsIWindowMediator.h"
#include "nsXPCOMCID.h"
#include "nsICategoryManager.h"
#include "nsIGenericFactory.h"

// just to do the reverse-lookup! sheesh.
#include "nsIInterfaceRequestorUtils.h"
#include "nsIDocShell.h"

PRUint32 nsWindowDataSource::windowCount = 0;

nsIRDFResource* nsWindowDataSource::kNC_Name = nsnull;
nsIRDFResource* nsWindowDataSource::kNC_WindowRoot = nsnull;
nsIRDFResource* nsWindowDataSource::kNC_KeyIndex = nsnull;

nsIRDFService*  nsWindowDataSource::gRDFService = nsnull;

PRUint32 nsWindowDataSource::gRefCnt = 0;

static const char kURINC_WindowRoot[] = "NC:WindowMediatorRoot";

DEFINE_RDF_VOCAB(NC_NAMESPACE_URI, NC, Name);
DEFINE_RDF_VOCAB(NC_NAMESPACE_URI, NC, KeyIndex);

nsresult
nsWindowDataSource::Init()
{
    nsresult rv;

    if (gRefCnt++ == 0) {
        rv = CallGetService("@mozilla.org/rdf/rdf-service;1", &gRDFService);
        if (NS_FAILED(rv)) return rv;

        gRDFService->GetResource(NS_LITERAL_CSTRING(kURINC_WindowRoot), &kNC_WindowRoot);
        gRDFService->GetResource(NS_LITERAL_CSTRING(kURINC_Name),       &kNC_Name);
        gRDFService->GetResource(NS_LITERAL_CSTRING(kURINC_KeyIndex),   &kNC_KeyIndex);
    }

    mInner = do_CreateInstance("@mozilla.org/rdf/datasource;1?name=in-memory-datasource", &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIRDFContainerUtils> rdfc =
        do_GetService("@mozilla.org/rdf/container-utils;1", &rv);
    if (NS_FAILED(rv)) return rv;

    rv = rdfc->MakeSeq(this, kNC_WindowRoot, getter_AddRefs(mContainer));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIWindowMediator> windowMediator =
        do_GetService(NS_WINDOWMEDIATOR_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    rv = windowMediator->AddListener(this);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIObserverService> observerService =
        do_GetService(NS_OBSERVERSERVICE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
        rv = observerService->AddObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID,
                                          PR_FALSE);
    }
    return NS_OK;
}

nsWindowDataSource::~nsWindowDataSource()
{
    if (--gRefCnt == 0) {
        NS_IF_RELEASE(kNC_Name);
        NS_IF_RELEASE(kNC_KeyIndex);
        NS_IF_RELEASE(kNC_WindowRoot);
        NS_IF_RELEASE(gRDFService);
    }
}

NS_IMETHODIMP
nsWindowDataSource::Observe(nsISupports *aSubject, const char* aTopic, const PRUnichar *aData)
{
    if (strcmp(aTopic, NS_XPCOM_SHUTDOWN_OBSERVER_ID) == 0) {
        // release these objects so that they release their reference
        // to us
        mContainer = nsnull;
        mInner = nsnull;
    }

    return NS_OK;
}

NS_IMPL_CYCLE_COLLECTION_CLASS(nsWindowDataSource)
NS_IMPL_CYCLE_COLLECTION_UNLINK_0(nsWindowDataSource)
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(nsWindowDataSource)
    // XXX mContainer?
    NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mInner)
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_IMPL_CYCLE_COLLECTING_ADDREF_AMBIGUOUS(nsWindowDataSource,
                                          nsIObserver)
NS_IMPL_CYCLE_COLLECTING_RELEASE_AMBIGUOUS(nsWindowDataSource,
                                           nsIObserver)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(nsWindowDataSource)
    NS_INTERFACE_MAP_ENTRY(nsIObserver)
    NS_INTERFACE_MAP_ENTRY(nsIWindowMediatorListener)
    NS_INTERFACE_MAP_ENTRY(nsIWindowDataSource)
    NS_INTERFACE_MAP_ENTRY(nsIRDFDataSource)
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIObserver)
NS_INTERFACE_MAP_END

// nsIWindowMediatorListener implementation
// handle notifications from the window mediator and reflect them into
// RDF

/* void onWindowTitleChange (in nsIXULWindow window, in wstring newTitle); */
NS_IMETHODIMP
nsWindowDataSource::OnWindowTitleChange(nsIXULWindow *window,
                                        const PRUnichar *newTitle)
{
    nsresult rv;
    
    nsVoidKey key(window);

    nsCOMPtr<nsISupports> sup =
        dont_AddRef(mWindowResources.Get(&key));

    // oops, make sure this window is in the hashtable!
    if (!sup) {
        OnOpenWindow(window);
        sup = dont_AddRef(mWindowResources.Get(&key));
    }
    
    NS_ENSURE_TRUE(sup, NS_ERROR_UNEXPECTED);

    nsCOMPtr<nsIRDFResource> windowResource =
        do_QueryInterface(sup);

    nsCOMPtr<nsIRDFLiteral> newTitleLiteral;
    rv = gRDFService->GetLiteral(newTitle, getter_AddRefs(newTitleLiteral));
    NS_ENSURE_SUCCESS(rv, rv);

    // get the old title
    nsCOMPtr<nsIRDFNode> oldTitleNode;
    rv = GetTarget(windowResource, kNC_Name, PR_TRUE,
                   getter_AddRefs(oldTitleNode));
    
    // assert the change
    if (NS_SUCCEEDED(rv) && oldTitleNode)
        // has an existing window title, update it
        rv = Change(windowResource, kNC_Name, oldTitleNode, newTitleLiteral);
    else
        // removed from the tasklist
        rv = Assert(windowResource, kNC_Name, newTitleLiteral, PR_TRUE);

    if (rv != NS_RDF_ASSERTION_ACCEPTED)
    {
      NS_ERROR("unable to set window name");
    }
    
    return NS_OK;
}

/* void onOpenWindow (in nsIXULWindow window); */
NS_IMETHODIMP
nsWindowDataSource::OnOpenWindow(nsIXULWindow *window)
{
    nsCAutoString windowId(NS_LITERAL_CSTRING("window-"));
    windowId.AppendInt(windowCount++, 10);

    nsCOMPtr<nsIRDFResource> windowResource;
    gRDFService->GetResource(windowId, getter_AddRefs(windowResource));

    nsVoidKey key(window);
    mWindowResources.Put(&key, windowResource);

    // assert the new window
    if (mContainer)
        mContainer->AppendElement(windowResource);

    return NS_OK;
}

/* void onCloseWindow (in nsIXULWindow window); */
NS_IMETHODIMP
nsWindowDataSource::OnCloseWindow(nsIXULWindow *window)
{
    nsVoidKey key(window);
    nsCOMPtr<nsIRDFResource> resource;

    nsresult rv;

    if (!mWindowResources.Remove(&key, getter_AddRefs(resource)))
        return NS_ERROR_UNEXPECTED;

    // make sure we're not shutting down
    if (!mContainer) return NS_OK;
    
    nsCOMPtr<nsIRDFNode> oldKeyNode;
    nsCOMPtr<nsIRDFInt> oldKeyInt;
    
    // get the old keyIndex, if any
    rv = GetTarget(resource, kNC_KeyIndex, PR_TRUE,
                   getter_AddRefs(oldKeyNode));
    if (NS_SUCCEEDED(rv) && (rv != NS_RDF_NO_VALUE))
        oldKeyInt = do_QueryInterface(oldKeyNode);

    
    // update RDF and keyindex - from this point forward we'll ignore
    // errors, because they just indicate some kind of RDF inconsistency
    PRInt32 winIndex = -1;
    rv = mContainer->IndexOf(resource, &winIndex);
        
    if (NS_FAILED(rv))
        return NS_OK;
            
    // unassert the old window, ignore any error
    mContainer->RemoveElement(resource, PR_TRUE);
    
    nsCOMPtr<nsISimpleEnumerator> children;
    rv = mContainer->GetElements(getter_AddRefs(children));
    if (NS_FAILED(rv))
        return NS_OK;

    PRBool more = PR_FALSE;

    while (NS_SUCCEEDED(rv = children->HasMoreElements(&more)) && more) {
        nsCOMPtr<nsISupports> sup;
        rv = children->GetNext(getter_AddRefs(sup));
        if (NS_FAILED(rv))
            break;

        nsCOMPtr<nsIRDFResource> windowResource = do_QueryInterface(sup, &rv);
        if (NS_FAILED(rv))
            continue;

        PRInt32 currentIndex = -1;
        mContainer->IndexOf(windowResource, &currentIndex);

        // can skip updating windows with lower indexes
        // than the window that was removed
        if (currentIndex < winIndex)
            continue;

        nsCOMPtr<nsIRDFNode> newKeyNode;
        nsCOMPtr<nsIRDFInt> newKeyInt;

        rv = GetTarget(windowResource, kNC_KeyIndex, PR_TRUE,
                       getter_AddRefs(newKeyNode));
        if (NS_SUCCEEDED(rv) && (rv != NS_RDF_NO_VALUE))
            newKeyInt = do_QueryInterface(newKeyNode);

        // changing from one key index to another
        if (oldKeyInt && newKeyInt)
            Change(windowResource, kNC_KeyIndex, oldKeyInt, newKeyInt);
        // creating a new keyindex - probably window going
        // from (none) to "9"
        else if (newKeyInt)
            Assert(windowResource, kNC_KeyIndex, newKeyInt, PR_TRUE);
        
        // somehow inserting a window above this one,
        // "9" to (none)
        else if (oldKeyInt)
            Unassert(windowResource, kNC_KeyIndex, oldKeyInt);
        
    }
    return NS_OK;
}

struct findWindowClosure {
    nsIRDFResource* targetResource;
    nsIXULWindow *resultWindow;
};

PR_STATIC_CALLBACK(PRBool)
findWindow(nsHashKey* aKey, void *aData, void* aClosure)
{
    nsVoidKey *thisKey = static_cast<nsVoidKey*>(aKey);

    nsIRDFResource *resource =
        static_cast<nsIRDFResource*>(aData);
    
    findWindowClosure* closure =
        static_cast<findWindowClosure*>(aClosure);

    if (resource == closure->targetResource) {
        closure->resultWindow =
            static_cast<nsIXULWindow*>
                       (thisKey->GetValue());
        return PR_FALSE;         // stop enumerating
    }
    return PR_TRUE;
}

// nsIWindowDataSource implementation

NS_IMETHODIMP
nsWindowDataSource::GetWindowForResource(const char *aResourceString,
                                         nsIDOMWindowInternal** aResult)
{
    nsCOMPtr<nsIRDFResource> windowResource;
    gRDFService->GetResource(nsDependentCString(aResourceString),
                             getter_AddRefs(windowResource));

    // now reverse-lookup in the hashtable
    findWindowClosure closure = { windowResource.get(), nsnull };
    mWindowResources.Enumerate(findWindow, (void*)&closure);
    if (closure.resultWindow) {

        // this sucks, we have to jump through docshell to go from
        // nsIXULWindow -> nsIDOMWindowInternal
        nsCOMPtr<nsIDocShell> docShell;
        closure.resultWindow->GetDocShell(getter_AddRefs(docShell));

        if (docShell) {
            nsCOMPtr<nsIDOMWindowInternal> result =
                do_GetInterface(docShell);
        
            *aResult = result;
            NS_IF_ADDREF(*aResult);
        }
    }

    return NS_OK;
}


// nsIRDFDataSource implementation
// mostly, we just forward to mInner, except:
// GetURI() - need to return "rdf:window-mediator"
// GetTarget() - need to handle kNC_KeyIndex


/* readonly attribute string URI; */
NS_IMETHODIMP nsWindowDataSource::GetURI(char * *aURI)
{
    NS_ENSURE_ARG_POINTER(aURI);
    
    *aURI = ToNewCString(NS_LITERAL_CSTRING("rdf:window-mediator"));

    if (!*aURI)
        return NS_ERROR_OUT_OF_MEMORY;
    
    return NS_OK;
}

/* nsIRDFNode GetTarget (in nsIRDFResource aSource, in nsIRDFResource aProperty, in boolean aTruthValue); */
NS_IMETHODIMP nsWindowDataSource::GetTarget(nsIRDFResource *aSource, nsIRDFResource *aProperty, PRBool aTruthValue, nsIRDFNode **_retval)
{
    NS_ENSURE_ARG_POINTER(_retval);

    // add extra NULL checking for top-crash bug # 146466
    if (!gRDFService) return NS_RDF_NO_VALUE;
    if (!mInner) return NS_RDF_NO_VALUE;
    if (!mContainer) return NS_RDF_NO_VALUE;
    // special case kNC_KeyIndex before we forward to mInner
    if (aProperty == kNC_KeyIndex) {

        PRInt32 theIndex = 0;
        nsresult rv = mContainer->IndexOf(aSource, &theIndex);
        if (NS_FAILED(rv)) return rv;

        // only allow the range of 1 to 9 for single key access
        if (theIndex < 1 || theIndex > 9) return(NS_RDF_NO_VALUE);

        nsCOMPtr<nsIRDFInt> indexInt;
        rv = gRDFService->GetIntLiteral(theIndex, getter_AddRefs(indexInt));
        if (NS_FAILED(rv)) return(rv);
        if (!indexInt) return(NS_ERROR_FAILURE);
        
        return CallQueryInterface(indexInt, _retval);
    }

    
    return mInner->GetTarget(aSource, aProperty, aTruthValue, _retval);
}

/* nsIRDFResource GetSource (in nsIRDFResource aProperty, in nsIRDFNode aTarget, in boolean aTruthValue); */
NS_IMETHODIMP nsWindowDataSource::GetSource(nsIRDFResource *aProperty, nsIRDFNode *aTarget, PRBool aTruthValue, nsIRDFResource **_retval)
{
    if (mInner)
        return mInner->GetSource(aProperty, aTarget, aTruthValue, _retval);
    return NS_OK;
}

/* nsISimpleEnumerator GetSources (in nsIRDFResource aProperty, in nsIRDFNode aTarget, in boolean aTruthValue); */
NS_IMETHODIMP nsWindowDataSource::GetSources(nsIRDFResource *aProperty, nsIRDFNode *aTarget, PRBool aTruthValue, nsISimpleEnumerator **_retval)
{
    if (mInner)
        return mInner->GetSources(aProperty, aTarget, aTruthValue, _retval);
    return NS_OK;
}

/* nsISimpleEnumerator GetTargets (in nsIRDFResource aSource, in nsIRDFResource aProperty, in boolean aTruthValue); */
NS_IMETHODIMP nsWindowDataSource::GetTargets(nsIRDFResource *aSource, nsIRDFResource *aProperty, PRBool aTruthValue, nsISimpleEnumerator **_retval)
{
    if (mInner)
        return mInner->GetTargets(aSource, aProperty, aTruthValue, _retval);
    return NS_OK;
}

/* void Assert (in nsIRDFResource aSource, in nsIRDFResource aProperty, in nsIRDFNode aTarget, in boolean aTruthValue); */
NS_IMETHODIMP nsWindowDataSource::Assert(nsIRDFResource *aSource, nsIRDFResource *aProperty, nsIRDFNode *aTarget, PRBool aTruthValue)
{
    if (mInner)
        return mInner->Assert(aSource, aProperty, aTarget, aTruthValue);
    return NS_OK;
}

/* void Unassert (in nsIRDFResource aSource, in nsIRDFResource aProperty, in nsIRDFNode aTarget); */
NS_IMETHODIMP nsWindowDataSource::Unassert(nsIRDFResource *aSource, nsIRDFResource *aProperty, nsIRDFNode *aTarget)
{
    if (mInner)
        return mInner->Unassert(aSource, aProperty, aTarget);
    return NS_OK;
}

/* void Change (in nsIRDFResource aSource, in nsIRDFResource aProperty, in nsIRDFNode aOldTarget, in nsIRDFNode aNewTarget); */
NS_IMETHODIMP nsWindowDataSource::Change(nsIRDFResource *aSource, nsIRDFResource *aProperty, nsIRDFNode *aOldTarget, nsIRDFNode *aNewTarget)
{
    if (mInner)
        return mInner->Change(aSource, aProperty, aOldTarget, aNewTarget);
    return NS_OK;
}

/* void Move (in nsIRDFResource aOldSource, in nsIRDFResource aNewSource, in nsIRDFResource aProperty, in nsIRDFNode aTarget); */
NS_IMETHODIMP nsWindowDataSource::Move(nsIRDFResource *aOldSource, nsIRDFResource *aNewSource, nsIRDFResource *aProperty, nsIRDFNode *aTarget)
{
    if (mInner)
        return mInner->Move(aOldSource, aNewSource, aProperty, aTarget);
    return NS_OK;
}

/* boolean HasAssertion (in nsIRDFResource aSource, in nsIRDFResource aProperty, in nsIRDFNode aTarget, in boolean aTruthValue); */
NS_IMETHODIMP nsWindowDataSource::HasAssertion(nsIRDFResource *aSource, nsIRDFResource *aProperty, nsIRDFNode *aTarget, PRBool aTruthValue, PRBool *_retval)
{
    if (mInner)
        return mInner->HasAssertion(aSource, aProperty, aTarget, aTruthValue, _retval);
    return NS_OK;
}

/* void AddObserver (in nsIRDFObserver aObserver); */
NS_IMETHODIMP nsWindowDataSource::AddObserver(nsIRDFObserver *aObserver)
{
    if (mInner)
        return mInner->AddObserver(aObserver);
    return NS_OK;
}

/* void RemoveObserver (in nsIRDFObserver aObserver); */
NS_IMETHODIMP nsWindowDataSource::RemoveObserver(nsIRDFObserver *aObserver)
{
    if (mInner)
        return mInner->RemoveObserver(aObserver);
    return NS_OK;
}

/* nsISimpleEnumerator ArcLabelsIn (in nsIRDFNode aNode); */
NS_IMETHODIMP nsWindowDataSource::ArcLabelsIn(nsIRDFNode *aNode, nsISimpleEnumerator **_retval)
{
    if (mInner)
        return mInner->ArcLabelsIn(aNode, _retval);
    return NS_OK;
}

/* nsISimpleEnumerator ArcLabelsOut (in nsIRDFResource aSource); */
NS_IMETHODIMP nsWindowDataSource::ArcLabelsOut(nsIRDFResource *aSource, nsISimpleEnumerator **_retval)
{
    if (mInner)
        return mInner->ArcLabelsOut(aSource, _retval);
    return NS_OK;
}

/* nsISimpleEnumerator GetAllResources (); */
NS_IMETHODIMP nsWindowDataSource::GetAllResources(nsISimpleEnumerator **_retval)
{
    if (mInner)
        return mInner->GetAllResources(_retval);
    return NS_OK;
}

/* boolean IsCommandEnabled (in nsISupportsArray aSources, in nsIRDFResource aCommand, in nsISupportsArray aArguments); */
NS_IMETHODIMP nsWindowDataSource::IsCommandEnabled(nsISupportsArray *aSources, nsIRDFResource *aCommand, nsISupportsArray *aArguments, PRBool *_retval)
{
    if (mInner)
        return mInner->IsCommandEnabled(aSources, aCommand, aArguments, _retval);
    return NS_OK;
}

/* void DoCommand (in nsISupportsArray aSources, in nsIRDFResource aCommand, in nsISupportsArray aArguments); */
NS_IMETHODIMP nsWindowDataSource::DoCommand(nsISupportsArray *aSources, nsIRDFResource *aCommand, nsISupportsArray *aArguments)
{
    if (mInner)
        return mInner->DoCommand(aSources, aCommand, aArguments);
    return NS_OK;
}

/* nsISimpleEnumerator GetAllCmds (in nsIRDFResource aSource); */
NS_IMETHODIMP nsWindowDataSource::GetAllCmds(nsIRDFResource *aSource, nsISimpleEnumerator **_retval)
{
    if (mInner)
        return mInner->GetAllCmds(aSource, _retval);
    return NS_OK;
}

/* boolean hasArcIn (in nsIRDFNode aNode, in nsIRDFResource aArc); */
NS_IMETHODIMP nsWindowDataSource::HasArcIn(nsIRDFNode *aNode, nsIRDFResource *aArc, PRBool *_retval)
{
    if (mInner)
        return mInner->HasArcIn(aNode, aArc, _retval);
    return NS_OK;
}

/* boolean hasArcOut (in nsIRDFResource aSource, in nsIRDFResource aArc); */
NS_IMETHODIMP nsWindowDataSource::HasArcOut(nsIRDFResource *aSource, nsIRDFResource *aArc, PRBool *_retval)
{
    if (mInner)
        return mInner->HasArcOut(aSource, aArc, _retval);
    return NS_OK;
}

/* void beginUpdateBatch (); */
NS_IMETHODIMP nsWindowDataSource::BeginUpdateBatch()
{
    if (mInner)
        return mInner->BeginUpdateBatch();
    return NS_OK;
}
                                                                                
/* void endUpdateBatch (); */
NS_IMETHODIMP nsWindowDataSource::EndUpdateBatch()
{
    if (mInner)
        return mInner->EndUpdateBatch();
    return NS_OK;
}

// The module goop

NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsWindowDataSource, Init)

static NS_METHOD
RegisterWindowDS(nsIComponentManager *aCompMgr,
                 nsIFile *aPath,
                 const char *registryLocation,
                 const char *componentType,
                 const nsModuleComponentInfo *info)
{
    nsresult rv;
    nsCOMPtr<nsICategoryManager> catman = do_GetService(NS_CATEGORYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    return catman->AddCategoryEntry("app-startup", "Window Data Source",
                                    "service," NS_RDF_DATASOURCE_CONTRACTID_PREFIX "window-mediator",
                                    PR_TRUE, PR_TRUE, nsnull);
    return NS_OK;
}

static const nsModuleComponentInfo components[] = {
    { "nsWindowDataSource",
      NS_WINDOWDATASOURCE_CID,
      NS_RDF_DATASOURCE_CONTRACTID_PREFIX "window-mediator",
      nsWindowDataSourceConstructor, RegisterWindowDS }
};

NS_IMPL_NSGETMODULE(nsWindowDataSourceModule, components)
