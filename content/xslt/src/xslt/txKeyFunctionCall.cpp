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
 * The Original Code is TransforMiiX XSLT processor code.
 *
 * The Initial Developer of the Original Code is
 * Jonas Sicking.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Jonas Sicking <sicking@bigfoot.com> (Original Author)
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
 * ***** END LICENSE BLOCK ***** */

#include "txExecutionState.h"
#include "txAtoms.h"
#include "txSingleNodeContext.h"
#include "txXSLTFunctions.h"
#include "nsReadableUtils.h"
#include "txKey.h"
#include "txXSLTPatterns.h"
#include "txNamespaceMap.h"

/*
 * txKeyFunctionCall
 * A representation of the XSLT additional function: key()
 */

/*
 * Creates a new key function call
 */
txKeyFunctionCall::txKeyFunctionCall(txNamespaceMap* aMappings)
    : mMappings(aMappings)
{
}

/*
 * Evaluates a key() xslt-function call. First argument is name of key
 * to use, second argument is value to look up.
 * @param aContext the context node for evaluation of this Expr
 * @param aCs      the ContextState containing the stack information needed
 *                 for evaluation
 * @return the result of the evaluation
 */
nsresult
txKeyFunctionCall::evaluate(txIEvalContext* aContext, txAExprResult** aResult)
{
    if (!aContext || !requireParams(2, 2, aContext))
        return NS_ERROR_XPATH_BAD_ARGUMENT_COUNT;

    txExecutionState* es =
        NS_STATIC_CAST(txExecutionState*, aContext->getPrivateContext());

    txListIterator iter(&params);

    nsAutoString keyQName;
    Expr* param = NS_STATIC_CAST(Expr*, iter.next());
    nsresult rv = param->evaluateToString(aContext, keyQName);
    NS_ENSURE_SUCCESS(rv, rv);

    txExpandedName keyName;
    rv = keyName.init(keyQName, mMappings, PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);

    nsRefPtr<txAExprResult> exprResult;
    rv = ((Expr*)iter.next())->evaluate(aContext, getter_AddRefs(exprResult));
    NS_ENSURE_SUCCESS(rv, rv);

    nsAutoPtr<txXPathNode> contextDoc(txXPathNodeUtils::getOwnerDocument(aContext->getContextNode()));
    NS_ENSURE_TRUE(contextDoc, NS_ERROR_FAILURE);

    nsRefPtr<txNodeSet> res;
    txNodeSet* nodeSet;
    if (exprResult->getResultType() == txAExprResult::NODESET &&
        (nodeSet = NS_STATIC_CAST(txNodeSet*,
                                  NS_STATIC_CAST(txAExprResult*,
                                                 exprResult)))->size() > 1) {
        rv = aContext->recycler()->getNodeSet(getter_AddRefs(res));
        NS_ENSURE_SUCCESS(rv, rv);

        PRInt32 i;
        for (i = 0; i < nodeSet->size(); ++i) {
            nsAutoString val;
            txXPathNodeUtils::appendNodeValue(nodeSet->get(i), val);

            nsRefPtr<txNodeSet> nodes;
            rv = es->getKeyNodes(keyName, *contextDoc, val, i == 0,
                                 getter_AddRefs(nodes));
            NS_ENSURE_SUCCESS(rv, rv);

            res->add(*nodes);
        }
    }
    else {
        nsAutoString val;
        exprResult->stringValue(val);
        rv = es->getKeyNodes(keyName, *contextDoc, val, PR_TRUE,
                             getter_AddRefs(res));
        NS_ENSURE_SUCCESS(rv, rv);
    }

    *aResult = res;
    NS_ADDREF(*aResult);

    return NS_OK;
}

Expr::ResultType
txKeyFunctionCall::getReturnType()
{
    return NODESET_RESULT;
}

PRBool
txKeyFunctionCall::isSensitiveTo(ContextSensitivity aContext)
{
    return (aContext & DOCUMENT_CONTEXT) || argsSensitiveTo(aContext);
}

#ifdef TX_TO_STRING
nsresult
txKeyFunctionCall::getNameAtom(nsIAtom** aAtom)
{
    *aAtom = txXSLTAtoms::key;
    NS_ADDREF(*aAtom);
    return NS_OK;
}
#endif

/**
 * Hash functions
 */

DHASH_WRAPPER(txKeyValueHash, txKeyValueHashEntry, txKeyValueHashKey&)
DHASH_WRAPPER(txIndexedKeyHash, txIndexedKeyHashEntry, txIndexedKeyHashKey&)

const void*
txKeyValueHashEntry::GetKey()
{
    return &mKey;
}

PRBool
txKeyValueHashEntry::MatchEntry(const void* aKey) const
{
    const txKeyValueHashKey* key =
        NS_STATIC_CAST(const txKeyValueHashKey*, aKey);

    return mKey.mKeyName == key->mKeyName &&
           mKey.mDocumentIdentifier == key->mDocumentIdentifier &&
           mKey.mKeyValue.Equals(key->mKeyValue);
}

PLDHashNumber
txKeyValueHashEntry::HashKey(const void* aKey)
{
    const txKeyValueHashKey* key =
        NS_STATIC_CAST(const txKeyValueHashKey*, aKey);

    return key->mKeyName.mNamespaceID ^
           NS_PTR_TO_INT32(key->mKeyName.mLocalName.get()) ^
           key->mDocumentIdentifier ^
           HashString(key->mKeyValue);
}

const void*
txIndexedKeyHashEntry::GetKey()
{
    return &mKey;
}

PRBool
txIndexedKeyHashEntry::MatchEntry(const void* aKey) const
{
    const txIndexedKeyHashKey* key =
        NS_STATIC_CAST(const txIndexedKeyHashKey*, aKey);

    return mKey.mKeyName == key->mKeyName &&
           mKey.mDocumentIdentifier == key->mDocumentIdentifier;
}

PLDHashNumber
txIndexedKeyHashEntry::HashKey(const void* aKey)
{
    const txIndexedKeyHashKey* key =
        NS_STATIC_CAST(const txIndexedKeyHashKey*, aKey);

    return key->mKeyName.mNamespaceID ^
           NS_PTR_TO_INT32(key->mKeyName.mLocalName.get()) ^
           key->mDocumentIdentifier;
}

/*
 * Class managing XSLT-keys
 */

nsresult
txKeyHash::getKeyNodes(const txExpandedName& aKeyName,
                       const txXPathNode& aDocument,
                       const nsAString& aKeyValue,
                       PRBool aIndexIfNotFound,
                       txExecutionState& aEs,
                       txNodeSet** aResult)
{
    NS_ENSURE_TRUE(mKeyValues.mHashTable.ops && mIndexedKeys.mHashTable.ops,
                   NS_ERROR_OUT_OF_MEMORY);

    *aResult = nsnull;

    PRInt32 identifier = txXPathNodeUtils::getUniqueIdentifier(aDocument);

    txKeyValueHashKey valueKey(aKeyName, identifier, aKeyValue);
    txKeyValueHashEntry* valueEntry = mKeyValues.GetEntry(valueKey);
    if (valueEntry) {
        *aResult = valueEntry->mNodeSet;
        NS_ADDREF(*aResult);

        return NS_OK;
    }

    // We didn't find a value. This could either mean that that key has no
    // nodes with that value or that the key hasn't been indexed using this
    // document.

    if (!aIndexIfNotFound) {
        // If aIndexIfNotFound is set then the caller knows this key is
        // indexed, so don't bother investigating.
        *aResult = mEmptyNodeSet;
        NS_ADDREF(*aResult);

        return NS_OK;
    }

    txIndexedKeyHashKey indexKey(aKeyName, identifier);
    txIndexedKeyHashEntry* indexEntry = mIndexedKeys.AddEntry(indexKey);
    NS_ENSURE_TRUE(indexEntry, NS_ERROR_OUT_OF_MEMORY);

    if (indexEntry->mIndexed) {
        // The key was indexed and apparently didn't contain this value so
        // return the empty nodeset.
        *aResult = mEmptyNodeSet;
        NS_ADDREF(*aResult);

        return NS_OK;
    }

    // The key needs to be indexed.
    txXSLKey* xslKey = (txXSLKey*)mKeys.get(aKeyName);
    if (!xslKey) {
        // The key didn't exist, so bail.
        return NS_ERROR_INVALID_ARG;
    }

    nsresult rv = xslKey->indexDocument(aDocument, mKeyValues, aEs);
    NS_ENSURE_SUCCESS(rv, rv);
    
    indexEntry->mIndexed = PR_TRUE;

    // Now that the key is indexed we can get its value.
    valueEntry = mKeyValues.GetEntry(valueKey);
    if (valueEntry) {
        *aResult = valueEntry->mNodeSet;
        NS_ADDREF(*aResult);
    }
    else {
        *aResult = mEmptyNodeSet;
        NS_ADDREF(*aResult);
    }

    return NS_OK;
}

nsresult
txKeyHash::init()
{
    nsresult rv = mKeyValues.Init(8);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mIndexedKeys.Init(1);
    NS_ENSURE_SUCCESS(rv, rv);
    
    mEmptyNodeSet = new txNodeSet(nsnull);
    NS_ENSURE_TRUE(mEmptyNodeSet, NS_ERROR_OUT_OF_MEMORY);
    
    return NS_OK;
}

/**
 * Class holding all <xsl:key>s of a particular expanded name in the
 * stylesheet.
 */
txXSLKey::~txXSLKey()
{
    txListIterator iter(&mKeys);
    Key* key;
    while ((key = (Key*)iter.next())) {
        delete key;
    }
}

/**
 * Adds a match/use pair.
 * @param aMatch  match-pattern
 * @param aUse    use-expression
 * @return PR_FALSE if an error occured, PR_TRUE otherwise
 */
PRBool txXSLKey::addKey(nsAutoPtr<txPattern> aMatch, nsAutoPtr<Expr> aUse)
{
    if (!aMatch || !aUse)
        return PR_FALSE;

    nsAutoPtr<Key> key(new Key);
    if (!key)
        return PR_FALSE;

    key->matchPattern = aMatch;
    key->useExpr = aUse;
    nsresult rv = mKeys.add(key);
    NS_ENSURE_SUCCESS(rv, PR_FALSE);
    
    key.forget();

    return PR_TRUE;
}

/**
 * Indexes a document and adds it to the hash of key values
 * @param aDocument     Document to index and add
 * @param aKeyValueHash Hash to add values to
 * @param aEs           txExecutionState to use for XPath evaluation
 */
nsresult txXSLKey::indexDocument(const txXPathNode& aDocument,
                                 txKeyValueHash& aKeyValueHash,
                                 txExecutionState& aEs)
{
    txKeyValueHashKey key(mName,
                          txXPathNodeUtils::getUniqueIdentifier(aDocument),
                          EmptyString());
    return indexTree(aDocument, key, aKeyValueHash, aEs);
}

/**
 * Recursively searches a node, its attributes and its subtree for
 * nodes matching any of the keys match-patterns.
 * @param aNode         Node to search
 * @param aKey          Key to use when adding into the hash
 * @param aKeyValueHash Hash to add values to
 * @param aEs           txExecutionState to use for XPath evaluation
 */
nsresult txXSLKey::indexTree(const txXPathNode& aNode,
                             txKeyValueHashKey& aKey,
                             txKeyValueHash& aKeyValueHash,
                             txExecutionState& aEs)
{
    nsresult rv = testNode(aNode, aKey, aKeyValueHash, aEs);
    NS_ENSURE_SUCCESS(rv, rv);

    // check if the node's attributes match
    txXPathTreeWalker walker(aNode);
    if (walker.moveToFirstAttribute()) {
        do {
            rv = testNode(walker.getCurrentPosition(), aKey, aKeyValueHash,
                          aEs);
            NS_ENSURE_SUCCESS(rv, rv);
        } while (walker.moveToNextAttribute());
        walker.moveToParent();
    }

    // check if the node's descendants match
    if (walker.moveToFirstChild()) {
        do {
            rv = indexTree(walker.getCurrentPosition(), aKey, aKeyValueHash,
                           aEs);
            NS_ENSURE_SUCCESS(rv, rv);
        } while (walker.moveToNextSibling());
    }

    return NS_OK;
}

/**
 * Tests one node if it matches any of the keys match-patterns. If
 * the node matches its values are added to the index.
 * @param aNode         Node to test
 * @param aKey          Key to use when adding into the hash
 * @param aKeyValueHash Hash to add values to
 * @param aEs           txExecutionState to use for XPath evaluation
 */
nsresult txXSLKey::testNode(const txXPathNode& aNode,
                            txKeyValueHashKey& aKey,
                            txKeyValueHash& aKeyValueHash,
                            txExecutionState& aEs)
{
    nsAutoString val;
    txListIterator iter(&mKeys);
    while (iter.hasNext())
    {
        Key* key = (Key*)iter.next();
        if (key->matchPattern->matches(aNode, &aEs)) {
            txSingleNodeContext evalContext(aNode, &aEs);
            nsresult rv = aEs.pushEvalContext(&evalContext);
            NS_ENSURE_SUCCESS(rv, rv);

            nsRefPtr<txAExprResult> exprResult;
            rv = key->useExpr->evaluate(&evalContext,
                                        getter_AddRefs(exprResult));
            NS_ENSURE_SUCCESS(rv, rv);

            aEs.popEvalContext();

            if (exprResult->getResultType() == txAExprResult::NODESET) {
                txNodeSet* res = NS_STATIC_CAST(txNodeSet*,
                                                NS_STATIC_CAST(txAExprResult*,
                                                               exprResult));
                PRInt32 i;
                for (i = 0; i < res->size(); ++i) {
                    val.Truncate();
                    txXPathNodeUtils::appendNodeValue(res->get(i), val);

                    aKey.mKeyValue.Assign(val);
                    txKeyValueHashEntry* entry = aKeyValueHash.AddEntry(aKey);
                    NS_ENSURE_TRUE(entry && entry->mNodeSet,
                                   NS_ERROR_OUT_OF_MEMORY);

                    if (entry->mNodeSet->isEmpty() ||
                        entry->mNodeSet->get(entry->mNodeSet->size() - 1) !=
                        aNode) {
                        entry->mNodeSet->append(aNode);
                    }
                }
            }
            else {
                exprResult->stringValue(val);

                aKey.mKeyValue.Assign(val);
                txKeyValueHashEntry* entry = aKeyValueHash.AddEntry(aKey);
                NS_ENSURE_TRUE(entry && entry->mNodeSet,
                               NS_ERROR_OUT_OF_MEMORY);

                if (entry->mNodeSet->isEmpty() ||
                    entry->mNodeSet->get(entry->mNodeSet->size() - 1) !=
                    aNode) {
                    entry->mNodeSet->append(aNode);
                }
            }
        }
    }
    
    return NS_OK;
}
