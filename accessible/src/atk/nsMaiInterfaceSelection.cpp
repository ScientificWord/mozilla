/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
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
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Silvia Zhao (silvia.zhao@sun.com)
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

#include "nsMaiInterfaceSelection.h"

void
selectionInterfaceInitCB(AtkSelectionIface *aIface)
{
    NS_ASSERTION(aIface, "Invalid aIface");
    if (!aIface)
        return;

    aIface->add_selection = addSelectionCB;
    aIface->clear_selection = clearSelectionCB;
    aIface->ref_selection = refSelectionCB;
    aIface->get_selection_count = getSelectionCountCB;
    aIface->is_child_selected = isChildSelectedCB;
    aIface->remove_selection = removeSelectionCB;
    aIface->select_all_selection = selectAllSelectionCB;
}

gboolean
addSelectionCB(AtkSelection *aSelection, gint i)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aSelection));
    if (!accWrap)
        return FALSE;

    nsCOMPtr<nsIAccessibleSelectable> accSelection;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleSelectable),
                            getter_AddRefs(accSelection));
    NS_ENSURE_TRUE(accSelection, FALSE);

    return NS_SUCCEEDED(accSelection->AddChildToSelection(i));
}

gboolean
clearSelectionCB(AtkSelection *aSelection)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aSelection));
    if (!accWrap)
        return FALSE;

    nsCOMPtr<nsIAccessibleSelectable> accSelection;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleSelectable),
                            getter_AddRefs(accSelection));
    NS_ENSURE_TRUE(accSelection, FALSE);

    return NS_SUCCEEDED(accSelection->ClearSelection());
}

AtkObject *
refSelectionCB(AtkSelection *aSelection, gint i)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aSelection));
    if (!accWrap)
        return nsnull;

    nsCOMPtr<nsIAccessibleSelectable> accSelection;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleSelectable),
                            getter_AddRefs(accSelection));
    NS_ENSURE_TRUE(accSelection, nsnull);

    nsCOMPtr<nsIAccessible> accSelect;
    accSelection->RefSelection(i, getter_AddRefs(accSelect));
    if (!accSelect) {
        return nsnull;
    }

    AtkObject *atkObj = nsAccessibleWrap::GetAtkObject(accSelect);
    if (atkObj) {
        g_object_ref(atkObj);
    }
    return atkObj;
}

gint
getSelectionCountCB(AtkSelection *aSelection)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aSelection));
    if (!accWrap)
        return -1;

    nsCOMPtr<nsIAccessibleSelectable> accSelection;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleSelectable),
                            getter_AddRefs(accSelection));
    NS_ENSURE_TRUE(accSelection, -1);

    PRInt32 num = 0;
    nsresult rv = accSelection->GetSelectionCount(&num);
    return (NS_FAILED(rv)) ? -1 : num;
}

gboolean
isChildSelectedCB(AtkSelection *aSelection, gint i)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aSelection));
    if (!accWrap)
        return FALSE;

    nsCOMPtr<nsIAccessibleSelectable> accSelection;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleSelectable),
                            getter_AddRefs(accSelection));
    NS_ENSURE_TRUE(accSelection, FALSE);

    PRBool result = FALSE;
    nsresult rv = accSelection->IsChildSelected(i, &result);
    return (NS_FAILED(rv)) ? FALSE : result;
}

gboolean
removeSelectionCB(AtkSelection *aSelection, gint i)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aSelection));
    if (!accWrap)
        return FALSE;

    nsCOMPtr<nsIAccessibleSelectable> accSelection;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleSelectable),
                            getter_AddRefs(accSelection));
    NS_ENSURE_TRUE(accSelection, FALSE);

    nsresult rv = accSelection->RemoveChildFromSelection(i);
    return (NS_FAILED(rv)) ? FALSE : TRUE;
}

gboolean
selectAllSelectionCB(AtkSelection *aSelection)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aSelection));
    if (!accWrap)
        return FALSE;

    nsCOMPtr<nsIAccessibleSelectable> accSelection;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleSelectable),
                            getter_AddRefs(accSelection));
    NS_ENSURE_TRUE(accSelection, FALSE);

    PRBool result = FALSE;
    nsresult rv = accSelection->SelectAllSelection(&result);
    return (NS_FAILED(rv)) ? FALSE : result;
}
