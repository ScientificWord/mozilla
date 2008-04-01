/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
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
 * The Original Code is TransforMiiX XSLT processor.
 *
 * The Initial Developer of the Original Code is Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Jonas Sicking <jonas@sicking.cc>
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

#include "txExpr.h"
#include "txExprResult.h"
#include "txSingleNodeContext.h"

PRBool
txUnionNodeTest::matches(const txXPathNode& aNode,
                         txIMatchContext* aContext)
{
    PRUint32 i, len = mNodeTests.Length();
    for (i = 0; i < len; ++i) {
        if (mNodeTests[i]->matches(aNode, aContext)) {
            return PR_TRUE;
        }
    }

    return PR_FALSE;
}

double
txUnionNodeTest::getDefaultPriority()
{
    NS_ERROR("Don't call getDefaultPriority on txUnionPattern");
    return Double::NaN;
}

PRBool
txUnionNodeTest::isSensitiveTo(Expr::ContextSensitivity aContext)
{
    PRUint32 i, len = mNodeTests.Length();
    for (i = 0; i < len; ++i) {
        if (mNodeTests[i]->isSensitiveTo(aContext)) {
            return PR_TRUE;
        }
    }

    return PR_FALSE;
}

#ifdef TX_TO_STRING
void
txUnionNodeTest::toString(nsAString& aDest)
{
    aDest.AppendLiteral("(");
    for (PRUint32 i = 0; i < mNodeTests.Length(); ++i) {
        if (i != 0) {
            aDest.AppendLiteral(" | ");
        }
        mNodeTests[i]->toString(aDest);
    }
    aDest.AppendLiteral(")");
}
#endif
