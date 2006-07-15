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

#include "txToplevelItems.h"
#include "txStylesheet.h"
#include "txInstructions.h"
#include "txXSLTPatterns.h"

TX_IMPL_GETTYPE(txAttributeSetItem, txToplevelItem::attributeSet)
TX_IMPL_GETTYPE(txImportItem, txToplevelItem::import)
TX_IMPL_GETTYPE(txOutputItem, txToplevelItem::output)
TX_IMPL_GETTYPE(txDummyItem, txToplevelItem::dummy)

TX_IMPL_GETTYPE(txStripSpaceItem, txToplevelItem::stripSpace)

txStripSpaceItem::~txStripSpaceItem()
{
    PRInt32 i, count = mStripSpaceTests.Count();
    for (i = 0; i < count; ++i) {
        delete NS_STATIC_CAST(txStripSpaceTest*, mStripSpaceTests[i]);
    }
}

nsresult
txStripSpaceItem::addStripSpaceTest(txStripSpaceTest* aStripSpaceTest)
{
    if (!mStripSpaceTests.AppendElement(aStripSpaceTest)) {
        return NS_ERROR_OUT_OF_MEMORY;
    }

    return NS_OK;
}

TX_IMPL_GETTYPE(txTemplateItem, txToplevelItem::templ)

txTemplateItem::txTemplateItem(nsAutoPtr<txPattern> aMatch,
                               const txExpandedName& aName,
                               const txExpandedName& aMode, double aPrio)
    : mMatch(aMatch), mName(aName), mMode(aMode), mPrio(aPrio)
{
}

TX_IMPL_GETTYPE(txVariableItem, txToplevelItem::variable)

txVariableItem::txVariableItem(const txExpandedName& aName,
                               nsAutoPtr<Expr> aValue,
                               PRBool aIsParam)
    : mName(aName), mValue(aValue), mIsParam(aIsParam)
{
}
