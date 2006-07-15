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
 *   Bolian Yin (bolian.yin@sun.com)
 *   John Sun (john.sun@sun.com)
 *   Ginn Chen (ginn.chen@sun.com)
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

#include "nsMai.h"
#include "nsAutoPtr.h"
#include "nsRootAccessible.h"
#include "nsDocAccessibleWrap.h"
#include "nsAccessibleEventData.h"

#include <atk/atk.h>
#include <glib.h>
#include <glib-object.h>

//----- nsDocAccessibleWrap -----

/*
 * Must keep sychronization with enumerate AtkProperty in 
 * accessible/src/base/nsAccessibleEventData.h
 */
static char * sAtkPropertyNameArray[PROP_LAST] = {
    0,
    "accessible_name",
    "accessible_description",
    "accessible_parent",
    "accessible_value",
    "accessible_role",
    "accessible_layer",
    "accessible_mdi_zorder",
    "accessible_table_caption",
    "accessible_table_column_description",
    "accessible_table_column_header",
    "accessible_table_row_description",
    "accessible_table_row_header",
    "accessible_table_summary"
};

static  AtkStateType TranslateAState(PRUint32 aState, PRUint32 aExtState);

NS_IMPL_ISUPPORTS_INHERITED2(nsDocAccessibleWrap, nsDocAccessible, nsIAccessibleText, nsIAccessibleEditableText)

nsDocAccessibleWrap::nsDocAccessibleWrap(nsIDOMNode *aDOMNode,
                                         nsIWeakReference *aShell): 
  nsDocAccessible(aDOMNode, aShell), mActivated(PR_FALSE)
{
}

nsDocAccessibleWrap::~nsDocAccessibleWrap()
{
}

NS_IMETHODIMP nsDocAccessibleWrap::FireToolkitEvent(PRUint32 aEvent,
                                                    nsIAccessible* aAccessible,
                                                    void* aEventData)
{
    NS_ENSURE_ARG_POINTER(aAccessible);

    // First fire nsIObserver event for internal xpcom accessibility clients
    nsDocAccessible::FireToolkitEvent(aEvent, aAccessible, aEventData);

    nsresult rv = NS_ERROR_FAILURE;

    nsAccessibleWrap *accWrap =
        NS_STATIC_CAST(nsAccessibleWrap *, aAccessible);
    MAI_LOG_DEBUG(("\n\nReceived event: aEvent=%u, obj=0x%x, data=0x%x \n",
                   aEvent, aAccessible, aEventData));

    nsAccessibleWrap *oldAccWrap = nsnull, *newAccWrap = nsnull;
    AtkTableChange * pAtkTableChange = nsnull;

    switch (aEvent) {
    case nsIAccessibleEvent::EVENT_FOCUS:
      {
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_FOCUS\n"));
        nsRefPtr<nsRootAccessible> rootAccWrap = accWrap->GetRootAccessible();
        if (rootAccWrap && rootAccWrap->mActivated) {
          atk_focus_tracker_notify(accWrap->GetAtkObject());
        }
        rv = NS_OK;
      } break;

    case nsIAccessibleEvent::EVENT_STATE_CHANGE:
        AtkStateChange *pAtkStateChange;
        AtkStateType atkState;

        MAI_LOG_DEBUG(("\n\nReceived: EVENT_STATE_CHANGE\n"));
        if (!aEventData)
            break;

        pAtkStateChange = NS_REINTERPRET_CAST(AtkStateChange *, aEventData);

        switch (pAtkStateChange->state) {
        case nsIAccessible::STATE_INVISIBLE:
            atkState = ATK_STATE_VISIBLE;
            pAtkStateChange->enable = !pAtkStateChange->enable;
            break;
        case nsIAccessible::STATE_UNAVAILABLE:
            atkState = ATK_STATE_ENABLED;
            pAtkStateChange->enable = !pAtkStateChange->enable;
            break;
        case nsIAccessible::STATE_READONLY:
            atkState = ATK_STATE_EDITABLE;
            pAtkStateChange->enable = !pAtkStateChange->enable;
            break;
        default:
            atkState = TranslateAState(pAtkStateChange->state, pAtkStateChange->extState);
        }

        atk_object_notify_state_change(accWrap->GetAtkObject(),
                                       atkState, pAtkStateChange->enable);
        rv = NS_OK;
        break;
      
        /*
         * More complex than I ever thought.
         * Need handle them separately.
         */
    case nsIAccessibleEvent::EVENT_ATK_PROPERTY_CHANGE :
      {
        AtkPropertyChange *pAtkPropChange;
        AtkPropertyValues values = { NULL };

        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_PROPERTY_CHANGE\n"));
        if (!aEventData)
            break;

        pAtkPropChange = NS_REINTERPRET_CAST(AtkPropertyChange *, aEventData);
        values.property_name = sAtkPropertyNameArray[pAtkPropChange->type];
        
        MAI_LOG_DEBUG(("\n\nthe type of EVENT_ATK_PROPERTY_CHANGE: %d\n\n",
                       pAtkPropChange->type));

        switch (pAtkPropChange->type) {
        case PROP_TABLE_CAPTION:
        case PROP_TABLE_SUMMARY:

            if (pAtkPropChange->oldvalue)
                oldAccWrap = NS_REINTERPRET_CAST(nsAccessibleWrap *,
                                                 pAtkPropChange->oldvalue);

            if (pAtkPropChange->newvalue)
                newAccWrap = NS_REINTERPRET_CAST(nsAccessibleWrap *,
                                                 pAtkPropChange->newvalue);

            if (oldAccWrap && newAccWrap) {
                g_value_init(&values.old_value, G_TYPE_POINTER);
                g_value_set_pointer(&values.old_value,
                                    oldAccWrap->GetAtkObject());
                g_value_init(&values.new_value, G_TYPE_POINTER);
                g_value_set_pointer(&values.new_value,
                                    newAccWrap->GetAtkObject());
                rv = NS_OK;
            }
            break;

        case PROP_TABLE_COLUMN_DESCRIPTION:
        case PROP_TABLE_COLUMN_HEADER:
        case PROP_TABLE_ROW_HEADER:
        case PROP_TABLE_ROW_DESCRIPTION:
            g_value_init(&values.new_value, G_TYPE_INT);
            g_value_set_int(&values.new_value,
                            *NS_REINTERPRET_CAST(gint *,
                                                 pAtkPropChange->newvalue));
            rv = NS_OK;
            break;
        case PROP_VALUE:
            {
              // Old value not used for anything other than state change events
              nsCOMPtr<nsIAccessibleValue> accValue(do_QueryInterface(aAccessible, &rv));
              if (!NS_SUCCEEDED(rv)) {
                break;
              }
              double newValue;
              rv = accValue->GetCurrentValue(&newValue);
              if (!NS_SUCCEEDED(rv)) {
                break;
              }
              g_value_init(&values.new_value, G_TYPE_DOUBLE);
              g_value_set_double(&values.new_value, newValue);
            }
            break;
  
            //Perhaps need more cases in the future
        default:
            g_value_init (&values.old_value, G_TYPE_POINTER);
            g_value_set_pointer (&values.old_value, pAtkPropChange->oldvalue);
            g_value_init (&values.new_value, G_TYPE_POINTER);
            g_value_set_pointer (&values.new_value, pAtkPropChange->newvalue);
            rv = NS_OK;
        }
        if (NS_SUCCEEDED(rv)) {
            char *signal_name = g_strconcat("property_change::",
                                            values.property_name, NULL);
            g_signal_emit_by_name(accWrap->GetAtkObject(), signal_name,
                                  &values, NULL);
            g_free (signal_name);
        }
      }
        break;

    case nsIAccessibleEvent::EVENT_ATK_SELECTION_CHANGE:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_SELECTION_CHANGE\n"));
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "selection_changed");
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_TEXT_CHANGE:
        AtkTextChange *pAtkTextChange;

        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TEXT_CHANGE\n"));
        if (!aEventData)
            break;

        pAtkTextChange = NS_REINTERPRET_CAST(AtkTextChange *, aEventData);
        g_signal_emit_by_name (accWrap->GetAtkObject(),
                               pAtkTextChange->add ? \
                               "text_changed::insert":"text_changed::delete",
                               pAtkTextChange->start,
                               pAtkTextChange->length);
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_TEXT_SELECTION_CHANGE:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TEXT_SELECTION_CHANGE\n"));
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "text_selection_changed");
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_TEXT_CARET_MOVE:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TEXT_CARET_MOVE\n"));
        if (!aEventData)
            break;

        MAI_LOG_DEBUG(("\n\nCaret postion: %d", *(gint *)aEventData ));
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "text_caret_moved",
                              // Curent caret position
                              *(gint *)aEventData);
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_TABLE_MODEL_CHANGE:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TABLE_MODEL_CHANGE\n"));
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "model_changed");
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_TABLE_ROW_INSERT:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TABLE_ROW_INSERT\n"));
        if (!aEventData)
            break;

        pAtkTableChange = NS_REINTERPRET_CAST(AtkTableChange *, aEventData);

        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "row_inserted",
                              // After which the rows are inserted
                              pAtkTableChange->index,
                              // The number of the inserted
                              pAtkTableChange->count);
        rv = NS_OK;
        break;
        
    case nsIAccessibleEvent::EVENT_ATK_TABLE_ROW_DELETE:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TABLE_ROW_DELETE\n"));
        if (!aEventData)
            break;

        pAtkTableChange = NS_REINTERPRET_CAST(AtkTableChange *, aEventData);

        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "row_deleted",
                              // After which the rows are deleted
                              pAtkTableChange->index,
                              // The number of the deleted
                              pAtkTableChange->count);
        rv = NS_OK;
        break;
        
    case nsIAccessibleEvent::EVENT_ATK_TABLE_ROW_REORDER:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TABLE_ROW_REORDER\n"));
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "row_reordered");
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_TABLE_COLUMN_INSERT:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TABLE_COLUMN_INSERT\n"));
        if (!aEventData)
            break;

        pAtkTableChange = NS_REINTERPRET_CAST(AtkTableChange *, aEventData);

        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "column_inserted",
                              // After which the columns are inserted
                              pAtkTableChange->index,
                              // The number of the inserted
                              pAtkTableChange->count);
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_TABLE_COLUMN_DELETE:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TABLE_COLUMN_DELETE\n"));
        if (!aEventData)
            break;

        pAtkTableChange = NS_REINTERPRET_CAST(AtkTableChange *, aEventData);

        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "column_deleted",
                              // After which the columns are deleted
                              pAtkTableChange->index,
                              // The number of the deleted
                              pAtkTableChange->count);
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_TABLE_COLUMN_REORDER:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_TABLE_COLUMN_REORDER\n"));
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "column_reordered");
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_VISIBLE_DATA_CHANGE:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_VISIBLE_DATA_CHANGE\n"));
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "visible_data_changed");
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_LINK_SELECTED:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_LINK_SELECTED\n"));
        atk_focus_tracker_notify(accWrap->GetAtkObject());
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "link_selected",
                              // Selected link index 
                              *(gint *)aEventData);
        rv = NS_OK;
        break;

        // Is a superclass of ATK event children_changed
    case nsIAccessibleEvent::EVENT_REORDER:
        AtkChildrenChange *pAtkChildrenChange;

        MAI_LOG_DEBUG(("\n\nReceived: EVENT_REORDER(children_change)\n"));

        pAtkChildrenChange = NS_REINTERPRET_CAST(AtkChildrenChange *,
                                                 aEventData);
        nsAccessibleWrap *childAccWrap;
        if (pAtkChildrenChange && pAtkChildrenChange->child) {
            childAccWrap = NS_STATIC_CAST(nsAccessibleWrap *,
                                          pAtkChildrenChange->child);
            g_signal_emit_by_name (accWrap->GetAtkObject(),
                                   pAtkChildrenChange->add ? \
                                   "children_changed::add" : \
                                   "children_changed::remove",
                                   pAtkChildrenChange->index,
                                   childAccWrap->GetAtkObject(),
                                   NULL);
        }
        else {
            //
            // EVENT_REORDER is normally fired by "HTML Document".
            //
            // In GOK, [only] "children_changed::add" can cause foreground
            // window accessible to update it children, which will
            // refresh "UI-Grab" window.
            //
            g_signal_emit_by_name (accWrap->GetAtkObject(),
                                   "children_changed::add",
                                   -1, NULL, NULL);
        }

        rv = NS_OK;
        break;

        /*
         * Because dealing with menu is very different between nsIAccessible
         * and ATK, and the menu activity is important, specially transfer the
         * following two event.
         * Need more verification by AT test.
         */
    case nsIAccessibleEvent::EVENT_MENUSTART:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_MENUSTART\n"));
        atk_focus_tracker_notify(accWrap->GetAtkObject());
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "selection_changed");
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_MENUEND:
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_MENUEND\n"));
        g_signal_emit_by_name(accWrap->GetAtkObject(),
                              "selection_changed");
        rv = NS_OK;
        break;

    case nsIAccessibleEvent::EVENT_ATK_WINDOW_ACTIVATE:
      {
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_WINDOW_ACTIVATED\n"));
        nsDocAccessibleWrap *accDocWrap =
          NS_STATIC_CAST(nsDocAccessibleWrap *, aAccessible);
        accDocWrap->mActivated = PR_TRUE;
        AtkObject *accessible = accWrap->GetAtkObject();
        guint id = g_signal_lookup ("activate", MAI_TYPE_ATK_OBJECT);
        g_signal_emit(accessible, id, 0);
        rv = NS_OK;
      } break;

    case nsIAccessibleEvent::EVENT_ATK_WINDOW_DEACTIVATE:
      {
        MAI_LOG_DEBUG(("\n\nReceived: EVENT_ATK_WINDOW_DEACTIVATED\n"));
        nsDocAccessibleWrap *accDocWrap =
          NS_STATIC_CAST(nsDocAccessibleWrap *, aAccessible);
        accDocWrap->mActivated = PR_FALSE;
        AtkObject *accessible = accWrap->GetAtkObject();
        guint id = g_signal_lookup ("deactivate", MAI_TYPE_ATK_OBJECT);
        g_signal_emit(accessible, id, 0);
        rv = NS_OK;
      } break;

    default:
        // Don't transfer others
        MAI_LOG_DEBUG(("\n\nReceived an unknown event=0x%u\n", aEvent));
        break;
    }

    return rv;
}

/* static */
AtkStateType
TranslateAState(PRUint32 aState, PRUint32 aExtState)
{
    switch (aState) {
    case nsIAccessible::STATE_SELECTED:
        return ATK_STATE_SELECTED;
    case nsIAccessible::STATE_FOCUSED:
        return ATK_STATE_FOCUSED;
    case nsIAccessible::STATE_PRESSED:
        return ATK_STATE_PRESSED;
    case nsIAccessible::STATE_CHECKED:
        return ATK_STATE_CHECKED;
    case nsIAccessible::STATE_EXPANDED:
        return ATK_STATE_EXPANDED;
    case nsIAccessible::STATE_COLLAPSED:
        return ATK_STATE_EXPANDABLE;
        // The control can't accept input at this time
    case nsIAccessible::STATE_BUSY:
        return ATK_STATE_BUSY;
    case nsIAccessible::STATE_FOCUSABLE:
        return ATK_STATE_FOCUSABLE;
    case nsIAccessible::STATE_SELECTABLE:
        return ATK_STATE_SELECTABLE;
    case nsIAccessible::STATE_SIZEABLE:
        return ATK_STATE_RESIZABLE;
    case nsIAccessible::STATE_MULTISELECTABLE:
        return ATK_STATE_MULTISELECTABLE;

#if 0
        // The following states are opposite the MSAA states.
        // We need to deal with them specially
    case nsIAccessible::STATE_INVISIBLE:
        return !ATK_STATE_VISIBLE;

    case nsIAccessible::STATE_UNAVAILABLE:
        return !ATK_STATE_ENABLED;

    case nsIAccessible::STATE_READONLY:
        return !ATK_STATE_EDITABLE;
#endif
    }

    // The following state is
    // Extended state flags (for non-MSAA, for Java and Gnome/ATK support)
    switch (aExtState) {
    case nsIAccessible::EXT_STATE_ACTIVE:
        return ATK_STATE_ACTIVE;
    case nsIAccessible::EXT_STATE_EXPANDABLE:
        return ATK_STATE_EXPANDABLE;
#if 0
        // Need change definitions in nsIAccessible.idl to avoid
        // duplicate value
    case nsIAccessible::EXT_STATE_MODAL:
        return ATK_STATE_MODAL;
#endif
    case nsIAccessible::EXT_STATE_MULTI_LINE:
        return ATK_STATE_MULTI_LINE;
    case nsIAccessible::EXT_STATE_SENSITIVE:
        return ATK_STATE_SENSITIVE;
    case nsIAccessible::EXT_STATE_SHOWING:
        return ATK_STATE_SHOWING;
    case nsIAccessible::EXT_STATE_SINGLE_LINE:
        return ATK_STATE_SINGLE_LINE;
    case nsIAccessible::EXT_STATE_TRANSIENT:
        return ATK_STATE_TRANSIENT;
    case nsIAccessible::EXT_STATE_VERTICAL:
        return ATK_STATE_VERTICAL;
    }
    return ATK_STATE_INVALID;
}

NS_IMETHODIMP nsDocAccessibleWrap::FireDocLoadingEvent(PRBool aIsFinished)
{
  if (!mDocument || !mWeakShell)
    return NS_OK;  // Document has been shut down

  if (!aIsFinished) {
    // Load has been verified, it will occur, about to commence
    AtkChildrenChange childrenData;
    childrenData.index = -1;
    childrenData.child = 0;
    childrenData.add = PR_FALSE;
    FireToolkitEvent(nsIAccessibleEvent::EVENT_REORDER, this, &childrenData);
  }

  return nsDocAccessible::FireDocLoadingEvent(aIsFinished);
}
