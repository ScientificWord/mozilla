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
 * Axel Hecht.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Axel Hecht <axel@pike.org>
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

#include "txForwardContext.h"
#include "txNodeSet.h"

const txXPathNode& txForwardContext::getContextNode()
{
    return mContextNode;
}

PRUint32 txForwardContext::size()
{
    return (PRUint32)mContextSet->size();
}

PRUint32 txForwardContext::position()
{
    PRInt32 pos = mContextSet->indexOf(mContextNode);
    NS_ASSERTION(pos >= 0, "Context is not member of context node list.");
    return (PRUint32)(pos + 1);
}

nsresult txForwardContext::getVariable(PRInt32 aNamespace, nsIAtom* aLName,
                                       txAExprResult*& aResult)
{
    NS_ASSERTION(mInner, "mInner is null!!!");
    return mInner->getVariable(aNamespace, aLName, aResult);
}

MBool txForwardContext::isStripSpaceAllowed(const txXPathNode& aNode)
{
    NS_ASSERTION(mInner, "mInner is null!!!");
    return mInner->isStripSpaceAllowed(aNode);
}

void* txForwardContext::getPrivateContext()
{
    NS_ASSERTION(mInner, "mInner is null!!!");
    return mInner->getPrivateContext();
}

txResultRecycler* txForwardContext::recycler()
{
    NS_ASSERTION(mInner, "mInner is null!!!");
    return mInner->recycler();
}

void txForwardContext::receiveError(const nsAString& aMsg, nsresult aRes)
{
    NS_ASSERTION(mInner, "mInner is null!!!");
#ifdef DEBUG
    nsAutoString error(NS_LITERAL_STRING("forwarded error: "));
    error.Append(aMsg);
    mInner->receiveError(error, aRes);
#else
    mInner->receiveError(aMsg, aRes);
#endif
}
