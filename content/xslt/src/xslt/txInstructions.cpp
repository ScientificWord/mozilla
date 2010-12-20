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

#include "txInstructions.h"
#include "txError.h"
#include "txExpr.h"
#include "txStylesheet.h"
#include "txNodeSetContext.h"
#include "txTextHandler.h"
#include "nsIConsoleService.h"
#include "nsServiceManagerUtils.h"
#include "txStringUtils.h"
#include "txAtoms.h"
#include "txRtfHandler.h"
#include "txNodeSorter.h"
#include "txXSLTNumber.h"
#include "txExecutionState.h"

nsresult
txApplyDefaultElementTemplate::execute(txExecutionState& aEs)
{
    txExecutionState::TemplateRule* rule = aEs.getCurrentTemplateRule();
    txExpandedName mode(rule->mModeNsId, rule->mModeLocalName);
    txStylesheet::ImportFrame* frame = 0;
    txInstruction* templ =
        aEs.mStylesheet->findTemplate(aEs.getEvalContext()->getContextNode(),
                                      mode, &aEs, nsnull, &frame);

    nsresult rv = aEs.pushTemplateRule(frame, mode, aEs.mTemplateParams);
    NS_ENSURE_SUCCESS(rv, rv);

    return aEs.runTemplate(templ);
}

nsresult
txApplyImportsEnd::execute(txExecutionState& aEs)
{
    aEs.popTemplateRule();
    aEs.popParamMap();
    
    return NS_OK;
}

nsresult
txApplyImportsStart::execute(txExecutionState& aEs)
{
    txExecutionState::TemplateRule* rule = aEs.getCurrentTemplateRule();
    // The frame is set to null when there is no current template rule, or
    // when the current template rule is a default template. However this
    // instruction isn't used in default templates.
    if (!rule->mFrame) {
        // XXX ErrorReport: apply-imports instantiated without a current rule
        return NS_ERROR_XSLT_EXECUTION_FAILURE;
    }

    nsresult rv = aEs.pushParamMap(rule->mParams);
    NS_ENSURE_SUCCESS(rv, rv);

    txStylesheet::ImportFrame* frame = 0;
    txExpandedName mode(rule->mModeNsId, rule->mModeLocalName);
    txInstruction* templ =
        aEs.mStylesheet->findTemplate(aEs.getEvalContext()->getContextNode(),
                                      mode, &aEs, rule->mFrame, &frame);

    rv = aEs.pushTemplateRule(frame, mode, rule->mParams);
    NS_ENSURE_SUCCESS(rv, rv);

    return aEs.runTemplate(templ);
}

txApplyTemplates::txApplyTemplates(const txExpandedName& aMode)
    : mMode(aMode)
{
}

nsresult
txApplyTemplates::execute(txExecutionState& aEs)
{
    txStylesheet::ImportFrame* frame = 0;
    txInstruction* templ =
        aEs.mStylesheet->findTemplate(aEs.getEvalContext()->getContextNode(),
                                      mMode, &aEs, nsnull, &frame);

    nsresult rv = aEs.pushTemplateRule(frame, mMode, aEs.mTemplateParams);
    NS_ENSURE_SUCCESS(rv, rv);

    return aEs.runTemplate(templ);
}

txAttribute::txAttribute(nsAutoPtr<Expr> aName, nsAutoPtr<Expr> aNamespace,
                         txNamespaceMap* aMappings)
    : mName(aName),
      mNamespace(aNamespace),
      mMappings(aMappings)
{
}

nsresult
txAttribute::execute(txExecutionState& aEs)
{
    nsAutoString name;
    nsresult rv = mName->evaluateToString(aEs.getEvalContext(), name);
    NS_ENSURE_SUCCESS(rv, rv);

    const PRUnichar* colon;
    if (!XMLUtils::isValidQName(name, &colon) ||
        TX_StringEqualsAtom(name, txXMLAtoms::xmlns)) {
        return NS_OK;
    }

    nsCOMPtr<nsIAtom> prefix;
    PRUint32 lnameStart = 0;
    if (colon) {
        prefix = do_GetAtom(Substring(name.get(), colon));
        lnameStart = colon - name.get() + 1;
    }

    PRInt32 nsId = kNameSpaceID_None;
    if (mNamespace) {
        nsAutoString nspace;
        rv = mNamespace->evaluateToString(aEs.getEvalContext(),
                                          nspace);
        NS_ENSURE_SUCCESS(rv, rv);

        if (!nspace.IsEmpty()) {
            nsId = txNamespaceManager::getNamespaceID(nspace);
        }
    }
    else if (colon) {
        nsId = mMappings->lookupNamespace(prefix);
    }

    nsAutoPtr<txTextHandler> handler(
        static_cast<txTextHandler*>(aEs.popResultHandler()));

    // add attribute if everything was ok
    return nsId != kNameSpaceID_Unknown ?
           aEs.mResultHandler->attribute(prefix, Substring(name, lnameStart),
                                         nsId, handler->mValue) :
           NS_OK;
}

txCallTemplate::txCallTemplate(const txExpandedName& aName)
    : mName(aName)
{
}

nsresult
txCallTemplate::execute(txExecutionState& aEs)
{
    txInstruction* instr = aEs.mStylesheet->getNamedTemplate(mName);
    NS_ENSURE_TRUE(instr, NS_ERROR_XSLT_EXECUTION_FAILURE);

    nsresult rv = aEs.runTemplate(instr);
    NS_ENSURE_SUCCESS(rv, rv);
    
    return NS_OK;
}

txCheckParam::txCheckParam(const txExpandedName& aName)
    : mName(aName), mBailTarget(nsnull)
{
}

nsresult
txCheckParam::execute(txExecutionState& aEs)
{
    nsresult rv = NS_OK;
    if (aEs.mTemplateParams) {
        nsRefPtr<txAExprResult> exprRes;
        aEs.mTemplateParams->getVariable(mName, getter_AddRefs(exprRes));
        if (exprRes) {
            rv = aEs.bindVariable(mName, exprRes);
            NS_ENSURE_SUCCESS(rv, rv);

            aEs.gotoInstruction(mBailTarget);
        }
    }
    
    return NS_OK;
}

txConditionalGoto::txConditionalGoto(nsAutoPtr<Expr> aCondition,
                                     txInstruction* aTarget)
    : mCondition(aCondition),
      mTarget(aTarget)
{
}

nsresult
txConditionalGoto::execute(txExecutionState& aEs)
{
    PRBool exprRes;
    nsresult rv = mCondition->evaluateToBool(aEs.getEvalContext(), exprRes);
    NS_ENSURE_SUCCESS(rv, rv);

    if (!exprRes) {
        aEs.gotoInstruction(mTarget);
    }

    return NS_OK;
}

nsresult
txComment::execute(txExecutionState& aEs)
{
    nsAutoPtr<txTextHandler> handler(
        static_cast<txTextHandler*>(aEs.popResultHandler()));
    PRUint32 length = handler->mValue.Length();
    PRInt32 pos = 0;
    while ((pos = handler->mValue.FindChar('-', (PRUint32)pos)) != kNotFound) {
        ++pos;
        if ((PRUint32)pos == length || handler->mValue.CharAt(pos) == '-') {
            handler->mValue.Insert(PRUnichar(' '), pos++);
            ++length;
        }
    }

    return aEs.mResultHandler->comment(handler->mValue);
}

nsresult
txCopyBase::copyNode(const txXPathNode& aNode, txExecutionState& aEs)
{
    switch (txXPathNodeUtils::getNodeType(aNode)) {
        case txXPathNodeType::ATTRIBUTE_NODE:
        {
            nsAutoString nodeValue;
            txXPathNodeUtils::appendNodeValue(aNode, nodeValue);

            nsCOMPtr<nsIAtom> localName =
                txXPathNodeUtils::getLocalName(aNode);
            return aEs.mResultHandler->
                attribute(txXPathNodeUtils::getPrefix(aNode),
                          localName, nsnull,
                          txXPathNodeUtils::getNamespaceID(aNode),
                          nodeValue);
        }
        case txXPathNodeType::COMMENT_NODE:
        {
            nsAutoString nodeValue;
            txXPathNodeUtils::appendNodeValue(aNode, nodeValue);
            return aEs.mResultHandler->comment(nodeValue);
        }
        case txXPathNodeType::DOCUMENT_NODE:
        case txXPathNodeType::DOCUMENT_FRAGMENT_NODE:
        {
            // Copy children
            txXPathTreeWalker walker(aNode);
            PRBool hasChild = walker.moveToFirstChild();
            while (hasChild) {
                copyNode(walker.getCurrentPosition(), aEs);
                hasChild = walker.moveToNextSibling();
            }
            break;
        }
        case txXPathNodeType::ELEMENT_NODE:
        {
            nsCOMPtr<nsIAtom> localName =
                txXPathNodeUtils::getLocalName(aNode);
            nsresult rv = aEs.mResultHandler->
                startElement(txXPathNodeUtils::getPrefix(aNode),
                             localName, nsnull,
                             txXPathNodeUtils::getNamespaceID(aNode));
            NS_ENSURE_SUCCESS(rv, rv);

            // Copy attributes
            txXPathTreeWalker walker(aNode);
            if (walker.moveToFirstAttribute()) {
                do {
                    nsAutoString nodeValue;
                    walker.appendNodeValue(nodeValue);

                    const txXPathNode& attr = walker.getCurrentPosition();
                    localName = txXPathNodeUtils::getLocalName(attr);
                    rv = aEs.mResultHandler->
                        attribute(txXPathNodeUtils::getPrefix(attr),
                                  localName, nsnull,
                                  txXPathNodeUtils::getNamespaceID(attr),
                                  nodeValue);
                    NS_ENSURE_SUCCESS(rv, rv);
                } while (walker.moveToNextAttribute());
                walker.moveToParent();
            }

            // Copy children
            PRBool hasChild = walker.moveToFirstChild();
            while (hasChild) {
                copyNode(walker.getCurrentPosition(), aEs);
                hasChild = walker.moveToNextSibling();
            }

            return aEs.mResultHandler->endElement();
        }
        case txXPathNodeType::PROCESSING_INSTRUCTION_NODE:
        {
            nsAutoString target, data;
            txXPathNodeUtils::getNodeName(aNode, target);
            txXPathNodeUtils::appendNodeValue(aNode, data);
            return aEs.mResultHandler->processingInstruction(target, data);
        }
        case txXPathNodeType::TEXT_NODE:
        case txXPathNodeType::CDATA_SECTION_NODE:
        {
            nsAutoString nodeValue;
            txXPathNodeUtils::appendNodeValue(aNode, nodeValue);
            return aEs.mResultHandler->characters(nodeValue, PR_FALSE);
        }
    }
    
    return NS_OK;
}

txCopy::txCopy()
    : mBailTarget(nsnull)
{
}

nsresult
txCopy::execute(txExecutionState& aEs)
{
    nsresult rv = NS_OK;
    const txXPathNode& node = aEs.getEvalContext()->getContextNode();

    switch (txXPathNodeUtils::getNodeType(node)) {
        case txXPathNodeType::DOCUMENT_NODE:
        case txXPathNodeType::DOCUMENT_FRAGMENT_NODE:
        {
            const nsAFlatString& empty = EmptyString();

            // "close" current element to ensure that no attributes are added
            rv = aEs.mResultHandler->characters(empty, PR_FALSE);
            NS_ENSURE_SUCCESS(rv, rv);

            rv = aEs.pushBool(PR_FALSE);
            NS_ENSURE_SUCCESS(rv, rv);

            break;
        }
        case txXPathNodeType::ELEMENT_NODE:
        {
            nsCOMPtr<nsIAtom> localName =
                txXPathNodeUtils::getLocalName(node);
            rv = aEs.mResultHandler->
                startElement(txXPathNodeUtils::getPrefix(node),
                             localName, nsnull,
                             txXPathNodeUtils::getNamespaceID(node));
            NS_ENSURE_SUCCESS(rv, rv);

            // XXX copy namespace nodes once we have them

            rv = aEs.pushBool(PR_TRUE);
            NS_ENSURE_SUCCESS(rv, rv);

            break;
        }
        default:
        {
            rv = copyNode(node, aEs);
            NS_ENSURE_SUCCESS(rv, rv);

            aEs.gotoInstruction(mBailTarget);
        }
    }

    return NS_OK;
}

txCopyOf::txCopyOf(nsAutoPtr<Expr> aSelect)
    : mSelect(aSelect)
{
}

nsresult
txCopyOf::execute(txExecutionState& aEs)
{
    nsRefPtr<txAExprResult> exprRes;
    nsresult rv = mSelect->evaluate(aEs.getEvalContext(),
                                    getter_AddRefs(exprRes));
    NS_ENSURE_SUCCESS(rv, rv);

    switch (exprRes->getResultType()) {
        case txAExprResult::NODESET:
        {
            txNodeSet* nodes = static_cast<txNodeSet*>
                                          (static_cast<txAExprResult*>
                                                      (exprRes));
            PRInt32 i;
            for (i = 0; i < nodes->size(); ++i) {
                rv = copyNode(nodes->get(i), aEs);
                NS_ENSURE_SUCCESS(rv, rv);
            }
            break;
        }
        case txAExprResult::RESULT_TREE_FRAGMENT:
        {
            txResultTreeFragment* rtf =
                static_cast<txResultTreeFragment*>
                           (static_cast<txAExprResult*>(exprRes));
            return rtf->flushToHandler(&aEs.mResultHandler);
        }
        default:
        {
            nsAutoString value;
            exprRes->stringValue(value);
            if (!value.IsEmpty()) {
                return aEs.mResultHandler->characters(value, PR_FALSE);
            }
            break;
        }
    }
    
    return NS_OK;
}

nsresult
txEndElement::execute(txExecutionState& aEs)
{
    // This will return false if startElement was not called. This happens
    // when <xsl:element> produces a bad name, or when <xsl:copy> copies a
    // document node.
    if (aEs.popBool()) {
        return aEs.mResultHandler->endElement();
    }

    return NS_OK;
}

nsresult
txErrorInstruction::execute(txExecutionState& aEs)
{
    // XXX ErrorReport: unknown instruction executed
    return NS_ERROR_XSLT_EXECUTION_FAILURE;
}

txGoTo::txGoTo(txInstruction* aTarget)
    : mTarget(aTarget)
{
}

nsresult
txGoTo::execute(txExecutionState& aEs)
{
    aEs.gotoInstruction(mTarget);

    return NS_OK;
}

txInsertAttrSet::txInsertAttrSet(const txExpandedName& aName)
    : mName(aName)
{
}

nsresult
txInsertAttrSet::execute(txExecutionState& aEs)
{
    txInstruction* instr = aEs.mStylesheet->getAttributeSet(mName);
    NS_ENSURE_TRUE(instr, NS_ERROR_XSLT_EXECUTION_FAILURE);

    nsresult rv = aEs.runTemplate(instr);
    NS_ENSURE_SUCCESS(rv, rv);
    
    return NS_OK;
}

txLoopNodeSet::txLoopNodeSet(txInstruction* aTarget)
    : mTarget(aTarget)
{
}

nsresult
txLoopNodeSet::execute(txExecutionState& aEs)
{
    aEs.popTemplateRule();
    txNodeSetContext* context =
        static_cast<txNodeSetContext*>(aEs.getEvalContext());
    if (!context->hasNext()) {
        delete aEs.popEvalContext();

        return NS_OK;
    }

    context->next();
    aEs.gotoInstruction(mTarget);
    
    return NS_OK;
}

txLREAttribute::txLREAttribute(PRInt32 aNamespaceID, nsIAtom* aLocalName,
                               nsIAtom* aPrefix, nsAutoPtr<Expr> aValue)
    : mNamespaceID(aNamespaceID),
      mLocalName(aLocalName),
      mPrefix(aPrefix),
      mValue(aValue)
{
    if (aNamespaceID == kNameSpaceID_None) {
        mLowercaseLocalName = TX_ToLowerCaseAtom(aLocalName);
    }
}

nsresult
txLREAttribute::execute(txExecutionState& aEs)
{
    nsRefPtr<txAExprResult> exprRes;
    nsresult rv = mValue->evaluate(aEs.getEvalContext(),
                                   getter_AddRefs(exprRes));
    NS_ENSURE_SUCCESS(rv, rv);

    const nsString* value = exprRes->stringValuePointer();
    if (value) {
        return aEs.mResultHandler->attribute(mPrefix, mLocalName,
                                             mLowercaseLocalName,
                                             mNamespaceID, *value);
    }

    nsAutoString valueStr;
    exprRes->stringValue(valueStr);
    return aEs.mResultHandler->attribute(mPrefix, mLocalName,
                                         mLowercaseLocalName,
                                         mNamespaceID, valueStr);
}

txMessage::txMessage(PRBool aTerminate)
    : mTerminate(aTerminate)
{
}

nsresult
txMessage::execute(txExecutionState& aEs)
{
    nsAutoPtr<txTextHandler> handler(
        static_cast<txTextHandler*>(aEs.popResultHandler()));

    nsCOMPtr<nsIConsoleService> consoleSvc = 
      do_GetService("@mozilla.org/consoleservice;1");
    if (consoleSvc) {
        nsAutoString logString(NS_LITERAL_STRING("xsl:message - "));
        logString.Append(handler->mValue);
        consoleSvc->LogStringMessage(logString.get());
    }

    return mTerminate ? NS_ERROR_XSLT_ABORTED : NS_OK;
}

txNumber::txNumber(txXSLTNumber::LevelType aLevel, nsAutoPtr<txPattern> aCount,
                   nsAutoPtr<txPattern> aFrom, nsAutoPtr<Expr> aValue,
                   nsAutoPtr<Expr> aFormat, nsAutoPtr<Expr> aGroupingSeparator,
                   nsAutoPtr<Expr> aGroupingSize)
    : mLevel(aLevel), mCount(aCount), mFrom(aFrom), mValue(aValue),
      mFormat(aFormat), mGroupingSeparator(aGroupingSeparator),
      mGroupingSize(aGroupingSize)
{
}

nsresult
txNumber::execute(txExecutionState& aEs)
{
    nsAutoString res;
    nsresult rv =
        txXSLTNumber::createNumber(mValue, mCount, mFrom, mLevel, mGroupingSize,
                                   mGroupingSeparator, mFormat,
                                   aEs.getEvalContext(), res);
    NS_ENSURE_SUCCESS(rv, rv);
    
    return aEs.mResultHandler->characters(res, PR_FALSE);
}

nsresult
txPopParams::execute(txExecutionState& aEs)
{
    delete aEs.popParamMap();

    return NS_OK;
}

txProcessingInstruction::txProcessingInstruction(nsAutoPtr<Expr> aName)
    : mName(aName)
{
}

nsresult
txProcessingInstruction::execute(txExecutionState& aEs)
{
    nsAutoPtr<txTextHandler> handler(
        static_cast<txTextHandler*>(aEs.popResultHandler()));
    XMLUtils::normalizePIValue(handler->mValue);

    nsAutoString name;
    nsresult rv = mName->evaluateToString(aEs.getEvalContext(), name);
    NS_ENSURE_SUCCESS(rv, rv);

    // Check name validity (must be valid NCName and a PITarget)
    // XXX Need to check for NCName and PITarget
    const PRUnichar* colon;
    if (!XMLUtils::isValidQName(name, &colon)) {
        // XXX ErrorReport: bad PI-target
        return NS_ERROR_FAILURE;
    }

    return aEs.mResultHandler->processingInstruction(name, handler->mValue);
}

txPushNewContext::txPushNewContext(nsAutoPtr<Expr> aSelect)
    : mSelect(aSelect), mBailTarget(nsnull)
{
}

txPushNewContext::~txPushNewContext()
{
    PRInt32 i;
    for (i = 0; i < mSortKeys.Count(); ++i)
    {
        delete static_cast<SortKey*>(mSortKeys[i]);
    }
}

nsresult
txPushNewContext::execute(txExecutionState& aEs)
{
    nsRefPtr<txAExprResult> exprRes;
    nsresult rv = mSelect->evaluate(aEs.getEvalContext(),
                                    getter_AddRefs(exprRes));
    NS_ENSURE_SUCCESS(rv, rv);

    if (exprRes->getResultType() != txAExprResult::NODESET) {
        // XXX ErrorReport: nodeset expected
        return NS_ERROR_XSLT_NODESET_EXPECTED;
    }
    
    txNodeSet* nodes = static_cast<txNodeSet*>
                                  (static_cast<txAExprResult*>
                                              (exprRes));
    
    if (nodes->isEmpty()) {
        aEs.gotoInstruction(mBailTarget);
        
        return NS_OK;
    }

    txNodeSorter sorter;
    PRInt32 i, count = mSortKeys.Count();
    for (i = 0; i < count; ++i) {
        SortKey* sort = static_cast<SortKey*>(mSortKeys[i]);
        rv = sorter.addSortElement(sort->mSelectExpr, sort->mLangExpr,
                                   sort->mDataTypeExpr, sort->mOrderExpr,
                                   sort->mCaseOrderExpr,
                                   aEs.getEvalContext());
        NS_ENSURE_SUCCESS(rv, rv);
    }
    nsRefPtr<txNodeSet> sortedNodes;
    rv = sorter.sortNodeSet(nodes, &aEs, getter_AddRefs(sortedNodes));
    NS_ENSURE_SUCCESS(rv, rv);
    
    txNodeSetContext* context = new txNodeSetContext(sortedNodes, &aEs);
    NS_ENSURE_TRUE(context, NS_ERROR_OUT_OF_MEMORY);

    context->next();

    rv = aEs.pushEvalContext(context);
    if (NS_FAILED(rv)) {
        delete context;
        return rv;
    }
    
    return NS_OK;
}

nsresult
txPushNewContext::addSort(nsAutoPtr<Expr> aSelectExpr,
                          nsAutoPtr<Expr> aLangExpr,
                          nsAutoPtr<Expr> aDataTypeExpr,
                          nsAutoPtr<Expr> aOrderExpr,
                          nsAutoPtr<Expr> aCaseOrderExpr)
{
    SortKey* sort = new SortKey(aSelectExpr, aLangExpr, aDataTypeExpr,
                                aOrderExpr, aCaseOrderExpr);
    NS_ENSURE_TRUE(sort, NS_ERROR_OUT_OF_MEMORY);

    if (!mSortKeys.AppendElement(sort)) {
        delete sort;
        return NS_ERROR_OUT_OF_MEMORY;
    }
   
    return NS_OK;
}

txPushNewContext::SortKey::SortKey(nsAutoPtr<Expr> aSelectExpr,
                                   nsAutoPtr<Expr> aLangExpr,
                                   nsAutoPtr<Expr> aDataTypeExpr,
                                   nsAutoPtr<Expr> aOrderExpr,
                                   nsAutoPtr<Expr> aCaseOrderExpr)
    : mSelectExpr(aSelectExpr), mLangExpr(aLangExpr),
      mDataTypeExpr(aDataTypeExpr), mOrderExpr(aOrderExpr),
      mCaseOrderExpr(aCaseOrderExpr)
{
}

nsresult
txPushNullTemplateRule::execute(txExecutionState& aEs)
{
    return aEs.pushTemplateRule(nsnull, txExpandedName(), nsnull);
}

nsresult
txPushParams::execute(txExecutionState& aEs)
{
    return aEs.pushParamMap(nsnull);
}

nsresult
txPushRTFHandler::execute(txExecutionState& aEs)
{
    txAXMLEventHandler* handler = new txRtfHandler;
    NS_ENSURE_TRUE(handler, NS_ERROR_OUT_OF_MEMORY);
    
    nsresult rv = aEs.pushResultHandler(handler);
    if (NS_FAILED(rv)) {
        delete handler;
        return rv;
    }

    return NS_OK;
}

txPushStringHandler::txPushStringHandler(PRBool aOnlyText)
    : mOnlyText(aOnlyText)
{
}

nsresult
txPushStringHandler::execute(txExecutionState& aEs)
{
    txAXMLEventHandler* handler = new txTextHandler(mOnlyText);
    NS_ENSURE_TRUE(handler, NS_ERROR_OUT_OF_MEMORY);
    
    nsresult rv = aEs.pushResultHandler(handler);
    if (NS_FAILED(rv)) {
        delete handler;
        return rv;
    }

    return NS_OK;
}

txRemoveVariable::txRemoveVariable(const txExpandedName& aName)
    : mName(aName)
{
}

nsresult
txRemoveVariable::execute(txExecutionState& aEs)
{
    aEs.removeVariable(mName);
    
    return NS_OK;
}

nsresult
txReturn::execute(txExecutionState& aEs)
{
    NS_ASSERTION(!mNext, "instructions exist after txReturn");
    aEs.returnFromTemplate();

    return NS_OK;
}

txSetParam::txSetParam(const txExpandedName& aName, nsAutoPtr<Expr> aValue)
    : mName(aName), mValue(aValue)
{
}

nsresult
txSetParam::execute(txExecutionState& aEs)
{
    nsresult rv = NS_OK;
    if (!aEs.mTemplateParams) {
        aEs.mTemplateParams = new txVariableMap;
        NS_ENSURE_TRUE(aEs.mTemplateParams, NS_ERROR_OUT_OF_MEMORY);
    }

    nsRefPtr<txAExprResult> exprRes;
    if (mValue) {
        rv = mValue->evaluate(aEs.getEvalContext(),
                              getter_AddRefs(exprRes));
        NS_ENSURE_SUCCESS(rv, rv);
    }
    else {
        nsAutoPtr<txRtfHandler> rtfHandler(
            static_cast<txRtfHandler*>(aEs.popResultHandler()));
        rv = rtfHandler->getAsRTF(getter_AddRefs(exprRes));
        NS_ENSURE_SUCCESS(rv, rv);
    }
    
    rv = aEs.mTemplateParams->bindVariable(mName, exprRes);
    NS_ENSURE_SUCCESS(rv, rv);
    
    return NS_OK;
}

txSetVariable::txSetVariable(const txExpandedName& aName,
                             nsAutoPtr<Expr> aValue)
    : mName(aName), mValue(aValue)
{
}

nsresult
txSetVariable::execute(txExecutionState& aEs)
{
    nsresult rv = NS_OK;
    nsRefPtr<txAExprResult> exprRes;
    if (mValue) {
        rv = mValue->evaluate(aEs.getEvalContext(), getter_AddRefs(exprRes));
        NS_ENSURE_SUCCESS(rv, rv);
    }
    else {
        nsAutoPtr<txRtfHandler> rtfHandler(
            static_cast<txRtfHandler*>(aEs.popResultHandler()));
        rv = rtfHandler->getAsRTF(getter_AddRefs(exprRes));
        NS_ENSURE_SUCCESS(rv, rv);
    }
    
    return aEs.bindVariable(mName, exprRes);
}

txStartElement::txStartElement(nsAutoPtr<Expr> aName,
                               nsAutoPtr<Expr> aNamespace,
                               txNamespaceMap* aMappings)
    : mName(aName),
      mNamespace(aNamespace),
      mMappings(aMappings)
{
}

nsresult
txStartElement::execute(txExecutionState& aEs)
{
    nsAutoString name;
    nsresult rv = mName->evaluateToString(aEs.getEvalContext(), name);
    NS_ENSURE_SUCCESS(rv, rv);


    PRInt32 nsId = kNameSpaceID_None;
    nsCOMPtr<nsIAtom> prefix;
    PRUint32 lnameStart = 0;

    const PRUnichar* colon;
    if (XMLUtils::isValidQName(name, &colon)) {
        if (colon) {
            prefix = do_GetAtom(Substring(name.get(), colon));
            lnameStart = colon - name.get() + 1;
        }

        if (mNamespace) {
            nsAutoString nspace;
            rv = mNamespace->evaluateToString(aEs.getEvalContext(),
                                              nspace);
            NS_ENSURE_SUCCESS(rv, rv);

            if (!nspace.IsEmpty()) {
                nsId = txNamespaceManager::getNamespaceID(nspace);
            }
        }
        else {
            nsId = mMappings->lookupNamespace(prefix);
        }
    }
    else {
       nsId = kNameSpaceID_Unknown;
    }

    PRBool success = PR_TRUE;

    if (nsId != kNameSpaceID_Unknown) {
        rv = aEs.mResultHandler->startElement(prefix,
                                              Substring(name, lnameStart),
                                              nsId);
    }
    else {
        rv = NS_ERROR_XSLT_BAD_NODE_NAME;
    }

    if (rv == NS_ERROR_XSLT_BAD_NODE_NAME) {
        success = PR_FALSE;
        // we call characters with an empty string to "close" any element to
        // make sure that no attributes are added
        rv = aEs.mResultHandler->characters(EmptyString(), PR_FALSE);
    }
    NS_ENSURE_SUCCESS(rv, rv);

    rv = aEs.pushBool(success);
    NS_ENSURE_SUCCESS(rv, rv);

    return NS_OK;
}


txStartLREElement::txStartLREElement(PRInt32 aNamespaceID,
                                     nsIAtom* aLocalName,
                                     nsIAtom* aPrefix)
    : mNamespaceID(aNamespaceID),
      mLocalName(aLocalName),
      mPrefix(aPrefix)
{
    if (aNamespaceID == kNameSpaceID_None) {
        mLowercaseLocalName = TX_ToLowerCaseAtom(aLocalName);
    }
}

nsresult
txStartLREElement::execute(txExecutionState& aEs)
{
    nsresult rv = aEs.mResultHandler->startElement(mPrefix, mLocalName,
                                                   mLowercaseLocalName,
                                                   mNamespaceID);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = aEs.pushBool(PR_TRUE);
    NS_ENSURE_SUCCESS(rv, rv);

    return NS_OK;
}

txText::txText(const nsAString& aStr, PRBool aDOE)
    : mStr(aStr),
      mDOE(aDOE)
{
}

nsresult
txText::execute(txExecutionState& aEs)
{
    return aEs.mResultHandler->characters(mStr, mDOE);
}

txValueOf::txValueOf(nsAutoPtr<Expr> aExpr, PRBool aDOE)
    : mExpr(aExpr),
      mDOE(aDOE)
{
}

nsresult
txValueOf::execute(txExecutionState& aEs)
{
    nsRefPtr<txAExprResult> exprRes;
    nsresult rv = mExpr->evaluate(aEs.getEvalContext(),
                                  getter_AddRefs(exprRes));
    NS_ENSURE_SUCCESS(rv, rv);

    const nsString* value = exprRes->stringValuePointer();
    if (value) {
        if (!value->IsEmpty()) {
            return aEs.mResultHandler->characters(*value, mDOE);
        }
    }
    else {
        nsAutoString valueStr;
        exprRes->stringValue(valueStr);
        if (!valueStr.IsEmpty()) {
            return aEs.mResultHandler->characters(valueStr, mDOE);
        }
    }

    return NS_OK;
}
