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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Jonas Sicking <jonas@sicking.cc>
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
#include "txSingleNodeContext.h"
#include "txInstructions.h"
#include "txStylesheet.h"
#include "txVariableMap.h"
#include "txRtfHandler.h"
#include "txXSLTProcessor.h"
#include "txLog.h"
#include "txURIUtils.h"
#include "txXMLParser.h"

const PRInt32 txExecutionState::kMaxRecursionDepth = 20000;

nsresult txLoadedDocumentsHash::init(txXPathNode* aSourceDocument)
{
    nsresult rv = Init(8);
    NS_ENSURE_SUCCESS(rv, rv);

    mSourceDocument = aSourceDocument;
    
    nsAutoString baseURI;
    txXPathNodeUtils::getBaseURI(*mSourceDocument, baseURI);

    txLoadedDocumentEntry* entry = PutEntry(baseURI);
    if (!entry) {
        return NS_ERROR_FAILURE;
    }

    entry->mDocument = mSourceDocument;

    return NS_OK;
}

txLoadedDocumentsHash::~txLoadedDocumentsHash()
{
    if (!IsInitialized()) {
        return;
    }

    nsAutoString baseURI;
    txXPathNodeUtils::getBaseURI(*mSourceDocument, baseURI);

    txLoadedDocumentEntry* entry = GetEntry(baseURI);
    if (entry) {
        delete entry->mDocument.forget();
    }
}

txExecutionState::txExecutionState(txStylesheet* aStylesheet,
                                   PRBool aDisableLoads)
    : mStylesheet(aStylesheet),
      mNextInstruction(nsnull),
      mLocalVariables(nsnull),
      mRecursionDepth(0),
      mTemplateRules(nsnull),
      mTemplateRulesBufferSize(0),
      mTemplateRuleCount(0),
      mEvalContext(nsnull),
      mInitialEvalContext(nsnull),
      mGlobalParams(nsnull),
      mKeyHash(aStylesheet->getKeyMap()),
      mDisableLoads(aDisableLoads)
{
}

txExecutionState::~txExecutionState()
{
    delete mResultHandler;
    delete mLocalVariables;
    delete mEvalContext;

    PRInt32 i;
    for (i = 0; i < mTemplateRuleCount; ++i) {
        NS_IF_RELEASE(mTemplateRules[i].mModeLocalName);
    }
    delete [] mTemplateRules;
    
    txStackIterator varsIter(&mLocalVarsStack);
    while (varsIter.hasNext()) {
        delete (txVariableMap*)varsIter.next();
    }

    txStackIterator contextIter(&mEvalContextStack);
    while (contextIter.hasNext()) {
        txIEvalContext* context = (txIEvalContext*)contextIter.next();
        if (context != mInitialEvalContext) {
            delete context;
        }
    }

    txStackIterator handlerIter(&mResultHandlerStack);
    while (handlerIter.hasNext()) {
        delete (txAXMLEventHandler*)handlerIter.next();
    }

    txStackIterator paramIter(&mParamStack);
    while (paramIter.hasNext()) {
        delete (txVariableMap*)paramIter.next();
    }
}

nsresult
txExecutionState::init(const txXPathNode& aNode,
                       txOwningExpandedNameMap<txIGlobalParameter>* aGlobalParams)
{
    nsresult rv = NS_OK;

    mGlobalParams = aGlobalParams;

    // Set up initial context
    mEvalContext = new txSingleNodeContext(aNode, this);
    NS_ENSURE_TRUE(mEvalContext, NS_ERROR_OUT_OF_MEMORY);

    mInitialEvalContext = mEvalContext;

    // Set up output and result-handler
    txAXMLEventHandler* handler = 0;
    rv = mOutputHandlerFactory->
        createHandlerWith(mStylesheet->getOutputFormat(), &handler);
    NS_ENSURE_SUCCESS(rv, rv);

    mOutputHandler = handler;
    mResultHandler = handler;
    mOutputHandler->startDocument();

    // Set up loaded-documents-hash
    nsAutoPtr<txXPathNode> document(txXPathNodeUtils::getOwnerDocument(aNode));
    NS_ENSURE_TRUE(document, NS_ERROR_FAILURE);

    rv = mLoadedDocuments.init(document);
    NS_ENSURE_SUCCESS(rv, rv);

    // loaded-documents-hash owns this now
    document.forget();

    // Init members
    rv = mKeyHash.init();
    NS_ENSURE_SUCCESS(rv, rv);
    
    mRecycler = new txResultRecycler;
    NS_ENSURE_TRUE(mRecycler, NS_ERROR_OUT_OF_MEMORY);
    
    rv = mRecycler->init();
    NS_ENSURE_SUCCESS(rv, rv);
    
    // The actual value here doesn't really matter since noone should use this
    // value. But lets put something errorlike in just in case
    mGlobalVarPlaceholderValue = new StringResult(NS_LITERAL_STRING("Error"), nsnull);
    NS_ENSURE_TRUE(mGlobalVarPlaceholderValue, NS_ERROR_OUT_OF_MEMORY);

    // Initiate first instruction. This has to be done last since findTemplate
    // might use us.
    txStylesheet::ImportFrame* frame = 0;
    txExpandedName nullName;
    txInstruction* templ = mStylesheet->findTemplate(aNode, nullName,
                                                     this, nsnull, &frame);
    rv = pushTemplateRule(frame, nullName, nsnull);
    NS_ENSURE_SUCCESS(rv, rv);

    return runTemplate(templ);
}

nsresult
txExecutionState::end(nsresult aResult)
{
    popTemplateRule();
    return mOutputHandler->endDocument(aResult);
}



nsresult
txExecutionState::getVariable(PRInt32 aNamespace, nsIAtom* aLName,
                              txAExprResult*& aResult)
{
    nsresult rv = NS_OK;
    txExpandedName name(aNamespace, aLName);

    // look for a local variable
    if (mLocalVariables) {
        mLocalVariables->getVariable(name, &aResult);
        if (aResult) {
            return NS_OK;
        }
    }

    // look for an evaluated global variable
    mGlobalVariableValues.getVariable(name, &aResult);
    if (aResult) {
        if (aResult == mGlobalVarPlaceholderValue) {
            // XXX ErrorReport: cyclic variable-value
            NS_RELEASE(aResult);
            return NS_ERROR_XSLT_BAD_RECURSION;
        }
        return NS_OK;
    }

    // Is there perchance a global variable not evaluated yet?
    txStylesheet::GlobalVariable* var = mStylesheet->getGlobalVariable(name);
    if (!var) {
        // XXX ErrorReport: variable doesn't exist in this scope
        return NS_ERROR_FAILURE;
    }
    
    NS_ASSERTION(var->mExpr && !var->mFirstInstruction ||
                 !var->mExpr && var->mFirstInstruction,
                 "global variable should have either instruction or expression");

    // Is this a stylesheet parameter that has a value?
    if (var->mIsParam && mGlobalParams) {
        txIGlobalParameter* param = mGlobalParams->get(name);
        if (param) {
            rv = param->getValue(&aResult);
            NS_ENSURE_SUCCESS(rv, rv);

            rv = mGlobalVariableValues.bindVariable(name, aResult);
            if (NS_FAILED(rv)) {
                NS_RELEASE(aResult);
                return rv;
            }
            
            return NS_OK;
        }
    }

    // Insert a placeholdervalue to protect against recursion
    rv = mGlobalVariableValues.bindVariable(name, mGlobalVarPlaceholderValue);
    NS_ENSURE_SUCCESS(rv, rv);

    // evaluate the global variable
    pushEvalContext(mInitialEvalContext);
    if (var->mExpr) {
        txVariableMap* oldVars = mLocalVariables;
        mLocalVariables = nsnull;
        rv = var->mExpr->evaluate(getEvalContext(), &aResult);
        NS_ENSURE_SUCCESS(rv, rv);

        mLocalVariables = oldVars;
    }
    else {
        nsAutoPtr<txRtfHandler> rtfHandler(new txRtfHandler);
        NS_ENSURE_TRUE(rtfHandler, NS_ERROR_OUT_OF_MEMORY);

        rv = pushResultHandler(rtfHandler);
        NS_ENSURE_SUCCESS(rv, rv);
        
        rtfHandler.forget();

        txInstruction* prevInstr = mNextInstruction;
        // set return to nsnull to stop execution
        mNextInstruction = nsnull;
        rv = runTemplate(var->mFirstInstruction);
        NS_ENSURE_SUCCESS(rv, rv);

        rv = pushTemplateRule(nsnull, txExpandedName(), nsnull);
        NS_ENSURE_SUCCESS(rv, rv);

        rv = txXSLTProcessor::execute(*this);
        NS_ENSURE_SUCCESS(rv, rv);

        popTemplateRule();

        mNextInstruction = prevInstr;
        rtfHandler = (txRtfHandler*)popResultHandler();
        rv = rtfHandler->getAsRTF(&aResult);
        NS_ENSURE_SUCCESS(rv, rv);
    }
    popEvalContext();

    // Remove the placeholder and insert the calculated value
    mGlobalVariableValues.removeVariable(name);
    rv = mGlobalVariableValues.bindVariable(name, aResult);
    if (NS_FAILED(rv)) {
        NS_RELEASE(aResult);

        return rv;
    }

    return NS_OK;
}

PRBool
txExecutionState::isStripSpaceAllowed(const txXPathNode& aNode)
{
    return mStylesheet->isStripSpaceAllowed(aNode, this);
}

void*
txExecutionState::getPrivateContext()
{
    return this;
}

txResultRecycler*
txExecutionState::recycler()
{
    return mRecycler;
}

void
txExecutionState::receiveError(const nsAString& aMsg, nsresult aRes)
{
    // XXX implement me
}

nsresult
txExecutionState::pushEvalContext(txIEvalContext* aContext)
{
    nsresult rv = mEvalContextStack.push(mEvalContext);
    NS_ENSURE_SUCCESS(rv, rv);
    
    mEvalContext = aContext;
    
    return NS_OK;
}

txIEvalContext*
txExecutionState::popEvalContext()
{
    txIEvalContext* prev = mEvalContext;
    mEvalContext = (txIEvalContext*)mEvalContextStack.pop();
    
    return prev;
}

nsresult
txExecutionState::pushBool(PRBool aBool)
{
    return mBoolStack.AppendElement(aBool) ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}

PRBool
txExecutionState::popBool()
{
    NS_ASSERTION(mBoolStack.Length(), "popping from empty stack");
    PRUint32 last = mBoolStack.Length() - 1;
    NS_ENSURE_TRUE(last != (PRUint32)-1, PR_FALSE);

    PRBool res = mBoolStack.ElementAt(last);
    mBoolStack.RemoveElementAt(last);

    return res;
}

nsresult
txExecutionState::pushResultHandler(txAXMLEventHandler* aHandler)
{
    nsresult rv = mResultHandlerStack.push(mResultHandler);
    NS_ENSURE_SUCCESS(rv, rv);
    
    mResultHandler = aHandler;

    return NS_OK;
}

txAXMLEventHandler*
txExecutionState::popResultHandler()
{
    txAXMLEventHandler* oldHandler = mResultHandler;
    mResultHandler = (txAXMLEventHandler*)mResultHandlerStack.pop();

    return oldHandler;
}

nsresult
txExecutionState::pushTemplateRule(txStylesheet::ImportFrame* aFrame,
                                   const txExpandedName& aMode,
                                   txVariableMap* aParams)
{
    if (mTemplateRuleCount == mTemplateRulesBufferSize) {
        PRInt32 newSize =
            mTemplateRulesBufferSize ? mTemplateRulesBufferSize * 2 : 10;
        TemplateRule* newRules = new TemplateRule[newSize];
        NS_ENSURE_TRUE(newRules, NS_ERROR_OUT_OF_MEMORY);
        
        memcpy(newRules, mTemplateRules,
               mTemplateRuleCount * sizeof(TemplateRule));
        delete [] mTemplateRules;
        mTemplateRules = newRules;
        mTemplateRulesBufferSize = newSize;
    }

    mTemplateRules[mTemplateRuleCount].mFrame = aFrame;
    mTemplateRules[mTemplateRuleCount].mModeNsId = aMode.mNamespaceID;
    mTemplateRules[mTemplateRuleCount].mModeLocalName = aMode.mLocalName;
    mTemplateRules[mTemplateRuleCount].mParams = aParams;
    NS_IF_ADDREF(mTemplateRules[mTemplateRuleCount].mModeLocalName);
    ++mTemplateRuleCount;
    
    return NS_OK;
}

void
txExecutionState::popTemplateRule()
{
    // decrement outside of RELEASE, that would decrement twice
    --mTemplateRuleCount;
    NS_IF_RELEASE(mTemplateRules[mTemplateRuleCount].mModeLocalName);
}

txIEvalContext*
txExecutionState::getEvalContext()
{
    return mEvalContext;
}

const txXPathNode*
txExecutionState::retrieveDocument(const nsAString& aUri)
{
    NS_ASSERTION(aUri.FindChar(PRUnichar('#')) == kNotFound,
                 "Remove the fragment.");

    if (mDisableLoads) {
        return nsnull;
    }

    PR_LOG(txLog::xslt, PR_LOG_DEBUG,
           ("Retrieve Document %s", NS_LossyConvertUTF16toASCII(aUri).get()));

    // try to get already loaded document
    txLoadedDocumentEntry *entry = mLoadedDocuments.PutEntry(aUri);
    if (!entry) {
        return nsnull;
    }

    if (!entry->mDocument) {
        // open URI
        nsAutoString errMsg;
        // XXX we should get the loader from the actual node
        // triggering the load, but this will do for the time being
        nsresult rv;
        rv = txParseDocumentFromURI(aUri, *mLoadedDocuments.mSourceDocument,
                                    errMsg,
                                    getter_Transfers(entry->mDocument));

        if (NS_FAILED(rv) || !entry->mDocument) {
            mLoadedDocuments.RawRemoveEntry(entry);
            receiveError(NS_LITERAL_STRING("Couldn't load document '") +
                         aUri + NS_LITERAL_STRING("': ") + errMsg, rv);

            return nsnull;
        }
    }

    return entry->mDocument;
}

nsresult
txExecutionState::getKeyNodes(const txExpandedName& aKeyName,
                              const txXPathNode& aRoot,
                              const nsAString& aKeyValue,
                              PRBool aIndexIfNotFound,
                              txNodeSet** aResult)
{
    return mKeyHash.getKeyNodes(aKeyName, aRoot, aKeyValue,
                                aIndexIfNotFound, *this, aResult);
}

txExecutionState::TemplateRule*
txExecutionState::getCurrentTemplateRule()
{
    return mTemplateRules + mTemplateRuleCount - 1;
}

txInstruction*
txExecutionState::getNextInstruction()
{
    txInstruction* instr = mNextInstruction;
    if (instr) {
        mNextInstruction = instr->mNext;
    }
    
    return instr;
}

nsresult
txExecutionState::runTemplate(txInstruction* aTemplate)
{
    NS_ENSURE_TRUE(++mRecursionDepth < kMaxRecursionDepth,
                   NS_ERROR_XSLT_BAD_RECURSION);

    nsresult rv = mLocalVarsStack.push(mLocalVariables);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = mReturnStack.push(mNextInstruction);
    NS_ENSURE_SUCCESS(rv, rv);
    
    mLocalVariables = nsnull;
    mNextInstruction = aTemplate;
    
    return NS_OK;
}

void
txExecutionState::gotoInstruction(txInstruction* aNext)
{
    mNextInstruction = aNext;
}

void
txExecutionState::returnFromTemplate()
{
    --mRecursionDepth;
    NS_ASSERTION(!mReturnStack.isEmpty() && !mLocalVarsStack.isEmpty(),
                 "return or variable stack is empty");
    delete mLocalVariables;
    mNextInstruction = (txInstruction*)mReturnStack.pop();
    mLocalVariables = (txVariableMap*)mLocalVarsStack.pop();
}

nsresult
txExecutionState::bindVariable(const txExpandedName& aName,
                               txAExprResult* aValue)
{
    if (!mLocalVariables) {
        mLocalVariables = new txVariableMap;
        NS_ENSURE_TRUE(mLocalVariables, NS_ERROR_OUT_OF_MEMORY);
    }
    return mLocalVariables->bindVariable(aName, aValue);
}

void
txExecutionState::removeVariable(const txExpandedName& aName)
{
    mLocalVariables->removeVariable(aName);
}

nsresult
txExecutionState::pushParamMap(txVariableMap* aParams)
{
    nsresult rv = mParamStack.push(mTemplateParams);
    NS_ENSURE_SUCCESS(rv, rv);

    mTemplateParams.forget();
    mTemplateParams = aParams;
    
    return NS_OK;
}

txVariableMap*
txExecutionState::popParamMap()
{
    txVariableMap* oldParams = mTemplateParams.forget();
    mTemplateParams = (txVariableMap*)mParamStack.pop();

    return oldParams;
}
