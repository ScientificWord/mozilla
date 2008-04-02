/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>  (Original Author)
 *  Michael Ventnor <m.ventnor@gmail.com>
 *  Teune van Steeg <t.vansteeg@gmail.com>
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

#include "nsNativeThemeGTK.h"
#include "nsThemeConstants.h"
#include "gtkdrawing.h"

#include "nsIObserverService.h"
#include "nsIServiceManager.h"
#include "nsIFrame.h"
#include "nsIPresShell.h"
#include "nsIDocument.h"
#include "nsIContent.h"
#include "nsIEventStateManager.h"
#include "nsIViewManager.h"
#include "nsINameSpaceManager.h"
#include "nsILookAndFeel.h"
#include "nsIDeviceContext.h"
#include "nsGfxCIID.h"
#include "nsTransform2D.h"
#include "nsIMenuFrame.h"
#include "prlink.h"
#include "nsIDOMHTMLInputElement.h"
#include "nsWidgetAtoms.h"

#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "gfxContext.h"
#include "gfxPlatformGtk.h"
#include "gfxXlibNativeRenderer.h"

NS_IMPL_ISUPPORTS2(nsNativeThemeGTK, nsITheme, nsIObserver)

static int gLastXError;

static inline bool IsCheckboxWidgetType(PRUint8 aWidgetType)
{
  return (aWidgetType == NS_THEME_CHECKBOX || aWidgetType == NS_THEME_CHECKBOX_SMALL);
}

static inline bool IsRadioWidgetType(PRUint8 aWidgetType)
{
  return (aWidgetType == NS_THEME_RADIO || aWidgetType == NS_THEME_RADIO_SMALL);
}

nsNativeThemeGTK::nsNativeThemeGTK()
{
  if (moz_gtk_init() != MOZ_GTK_SUCCESS) {
    memset(mDisabledWidgetTypes, 0xff, sizeof(mDisabledWidgetTypes));
    return;
  }

  // We have to call moz_gtk_shutdown before the event loop stops running.
  nsCOMPtr<nsIObserverService> obsServ =
    do_GetService("@mozilla.org/observer-service;1");
  obsServ->AddObserver(this, "xpcom-shutdown", PR_FALSE);

  memset(mDisabledWidgetTypes, 0, sizeof(mDisabledWidgetTypes));
  memset(mSafeWidgetStates, 0, sizeof(mSafeWidgetStates));
}

nsNativeThemeGTK::~nsNativeThemeGTK() {
}

NS_IMETHODIMP
nsNativeThemeGTK::Observe(nsISupports *aSubject, const char *aTopic,
                          const PRUnichar *aData)
{
  if (!nsCRT::strcmp(aTopic, "xpcom-shutdown")) {
    moz_gtk_shutdown();
  } else {
    NS_NOTREACHED("unexpected topic");
    return NS_ERROR_UNEXPECTED;
  }

  return NS_OK;
}

void
nsNativeThemeGTK::RefreshWidgetWindow(nsIFrame* aFrame)
{
  nsIPresShell *shell = GetPresShell(aFrame);
  if (!shell)
    return;

  nsIViewManager* vm = shell->GetViewManager();
  if (!vm)
    return;
 
  vm->UpdateAllViews(NS_VMREFRESH_NO_SYNC);
}

static PRBool IsWidgetTypeDisabled(PRUint8* aDisabledVector, PRUint8 aWidgetType) {
  return aDisabledVector[aWidgetType >> 3] & (1 << (aWidgetType & 7));
}

static void SetWidgetTypeDisabled(PRUint8* aDisabledVector, PRUint8 aWidgetType) {
  aDisabledVector[aWidgetType >> 3] |= (1 << (aWidgetType & 7));
}

static inline PRUint16
GetWidgetStateKey(PRUint8 aWidgetType, GtkWidgetState *aWidgetState)
{
  return (aWidgetState->active |
          aWidgetState->focused << 1 |
          aWidgetState->inHover << 2 |
          aWidgetState->disabled << 3 |
          aWidgetState->isDefault << 4 |
          aWidgetType << 5);
}

static PRBool IsWidgetStateSafe(PRUint8* aSafeVector,
                                PRUint8 aWidgetType,
                                GtkWidgetState *aWidgetState)
{
  PRUint8 key = GetWidgetStateKey(aWidgetType, aWidgetState);
  return aSafeVector[key >> 3] & (1 << (key & 7));
}

static void SetWidgetStateSafe(PRUint8 *aSafeVector,
                               PRUint8 aWidgetType,
                               GtkWidgetState *aWidgetState)
{
  PRUint8 key = GetWidgetStateKey(aWidgetType, aWidgetState);
  aSafeVector[key >> 3] |= (1 << (key & 7));
}

static GtkTextDirection GetTextDirection(nsIFrame* aFrame)
{
  if (!aFrame)
    return GTK_TEXT_DIR_NONE;

  switch (aFrame->GetStyleVisibility()->mDirection) {
    case NS_STYLE_DIRECTION_RTL:
      return GTK_TEXT_DIR_RTL;
    case NS_STYLE_DIRECTION_LTR:
      return GTK_TEXT_DIR_LTR;
  }

  return GTK_TEXT_DIR_NONE;
}

PRBool
nsNativeThemeGTK::GetGtkWidgetAndState(PRUint8 aWidgetType, nsIFrame* aFrame,
                                       GtkThemeWidgetType& aGtkWidgetType,
                                       GtkWidgetState* aState,
                                       gint* aWidgetFlags)
{
  if (aState) {
    if (!aFrame) {
      // reset the entire struct to zero
      memset(aState, 0, sizeof(GtkWidgetState));
    } else {

      // For XUL checkboxes and radio buttons, the state of the parent
      // determines our state.
      nsIFrame *stateFrame = aFrame;
      if (aFrame && ((aWidgetFlags && (IsCheckboxWidgetType(aWidgetType) ||
                                       IsRadioWidgetType(aWidgetType))) ||
                     aWidgetType == NS_THEME_CHECKBOX_LABEL ||
                     aWidgetType == NS_THEME_RADIO_LABEL)) {

        nsIAtom* atom = nsnull;
        nsIContent *content = aFrame->GetContent();
        if (content->IsNodeOfType(nsINode::eXUL)) {
          if (aWidgetType == NS_THEME_CHECKBOX_LABEL ||
              aWidgetType == NS_THEME_RADIO_LABEL) {
            // Adjust stateFrame so GetContentState finds the correct state.
            stateFrame = aFrame = aFrame->GetParent()->GetParent();
          } else {
            // GetContentState knows to look one frame up for radio/checkbox
            // widgets, so don't adjust stateFrame here.
            aFrame = aFrame->GetParent();
          }
          if (aWidgetFlags) {
            if (!atom) {
              atom = (IsCheckboxWidgetType(aWidgetType) ||
                      aWidgetType == NS_THEME_CHECKBOX_LABEL) ? nsWidgetAtoms::checked
                                                              : nsWidgetAtoms::selected;
            }
            *aWidgetFlags = CheckBooleanAttr(aFrame, atom);
          }
        } else {
          if (aWidgetFlags) {
            nsCOMPtr<nsIDOMHTMLInputElement> inputElt(do_QueryInterface(content));
            if (inputElt) {
              PRBool isHTMLChecked;
              inputElt->GetChecked(&isHTMLChecked);
              *aWidgetFlags = isHTMLChecked;
            }
          }
        }
      } else if (aWidgetType == NS_THEME_TOOLBAR_BUTTON_DROPDOWN ||
                 aWidgetType == NS_THEME_TREEVIEW_HEADER_SORTARROW) {
        stateFrame = aFrame->GetParent();
      }

      PRInt32 eventState = GetContentState(stateFrame, aWidgetType);

      aState->disabled = (IsDisabled(aFrame) || IsReadOnly(aFrame));
      aState->active  = (eventState & NS_EVENT_STATE_ACTIVE) == NS_EVENT_STATE_ACTIVE;
      aState->focused = (eventState & NS_EVENT_STATE_FOCUS) == NS_EVENT_STATE_FOCUS;
      aState->inHover = (eventState & NS_EVENT_STATE_HOVER) == NS_EVENT_STATE_HOVER;
      aState->isDefault = IsDefaultButton(aFrame);
      aState->canDefault = FALSE; // XXX fix me
      aState->depressed = FALSE;

      if (aFrame && aFrame->GetContent()->IsNodeOfType(nsINode::eXUL)) {
        // For these widget types, some element (either a child or parent)
        // actually has element focus, so we check the focused attribute
        // to see whether to draw in the focused state.
        if (aWidgetType == NS_THEME_TEXTFIELD ||
            aWidgetType == NS_THEME_TEXTFIELD_MULTILINE ||
            aWidgetType == NS_THEME_DROPDOWN_TEXTFIELD ||
            aWidgetType == NS_THEME_SPINNER_TEXTFIELD ||
            aWidgetType == NS_THEME_RADIO_CONTAINER ||
            aWidgetType == NS_THEME_RADIO_LABEL) {
          aState->focused = IsFocused(aFrame);
        } else if (IsRadioWidgetType(aWidgetType) ||
                   IsCheckboxWidgetType(aWidgetType)) {
          // In XUL, checkboxes and radios shouldn't have focus rings, their labels do
          aState->focused = FALSE;
        }

        if (aWidgetType == NS_THEME_SCROLLBAR_THUMB_VERTICAL ||
            aWidgetType == NS_THEME_SCROLLBAR_THUMB_HORIZONTAL) {
          // for scrollbars we need to go up two to go from the thumb to
          // the slider to the actual scrollbar object
          nsIFrame *tmpFrame = aFrame->GetParent()->GetParent();

          aState->curpos = CheckIntAttr(tmpFrame, nsWidgetAtoms::curpos);
          aState->maxpos = CheckIntAttr(tmpFrame, nsWidgetAtoms::maxpos);
        }

        if (aWidgetType == NS_THEME_SCROLLBAR_BUTTON_UP ||
            aWidgetType == NS_THEME_SCROLLBAR_BUTTON_DOWN ||
            aWidgetType == NS_THEME_SCROLLBAR_BUTTON_LEFT ||
            aWidgetType == NS_THEME_SCROLLBAR_BUTTON_RIGHT) {
          // set the state to disabled when the scrollbar is scrolled to
          // the beginning or the end, depending on the button type.
          PRInt32 curpos = CheckIntAttr(aFrame, nsWidgetAtoms::curpos);
          PRInt32 maxpos = CheckIntAttr(aFrame, nsWidgetAtoms::maxpos);
          if ((curpos == 0 && (aWidgetType == NS_THEME_SCROLLBAR_BUTTON_UP ||
                aWidgetType == NS_THEME_SCROLLBAR_BUTTON_LEFT)) ||
              (curpos == maxpos &&
               (aWidgetType == NS_THEME_SCROLLBAR_BUTTON_DOWN ||
                aWidgetType == NS_THEME_SCROLLBAR_BUTTON_RIGHT)))
            aState->disabled = PR_TRUE;

          // In order to simulate native GTK scrollbar click behavior,
          // we set the active attribute on the element to true if it's
          // pressed with any mouse button.
          // This allows us to show that it's active without setting :active
          else if (CheckBooleanAttr(aFrame, nsWidgetAtoms::active))
            aState->active = PR_TRUE;

          if (aWidgetFlags) {
            *aWidgetFlags = GetScrollbarButtonType(aFrame);
            if (aWidgetType - NS_THEME_SCROLLBAR_BUTTON_UP < 2)
              *aWidgetFlags |= MOZ_GTK_STEPPER_VERTICAL;
          }
        }

        // menu item state is determined by the attribute "_moz-menuactive",
        // and not by the mouse hovering (accessibility).  as a special case,
        // menus which are children of a menu bar are only marked as prelight
        // if they are open, not on normal hover.

        if (aWidgetType == NS_THEME_MENUITEM ||
            aWidgetType == NS_THEME_CHECKMENUITEM ||
            aWidgetType == NS_THEME_RADIOMENUITEM ||
            aWidgetType == NS_THEME_MENUSEPARATOR ||
            aWidgetType == NS_THEME_MENUARROW) {
          PRBool isTopLevel = PR_FALSE;
          nsIMenuFrame *menuFrame;
          CallQueryInterface(aFrame, &menuFrame);

          if (menuFrame) {
            isTopLevel = menuFrame->IsOnMenuBar();
          }

          if (isTopLevel) {
            aState->inHover = menuFrame->IsOpen();
            *aWidgetFlags |= MOZ_TOPLEVEL_MENU_ITEM;
          } else {
            aState->inHover = CheckBooleanAttr(aFrame, nsWidgetAtoms::mozmenuactive);
            *aWidgetFlags &= ~MOZ_TOPLEVEL_MENU_ITEM;
          }

          aState->active = FALSE;
        
          if (aWidgetType == NS_THEME_CHECKMENUITEM ||
              aWidgetType == NS_THEME_RADIOMENUITEM) {
            *aWidgetFlags = aFrame && aFrame->GetContent()->
              AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::checked,
                          nsWidgetAtoms::_true, eIgnoreCase);
          }
        }

        // A button with drop down menu open or an activated toggle button
        // should always appear depressed.
        if (aWidgetType == NS_THEME_BUTTON ||
            aWidgetType == NS_THEME_TOOLBAR_BUTTON ||
            aWidgetType == NS_THEME_TOOLBAR_DUAL_BUTTON ||
            aWidgetType == NS_THEME_TOOLBAR_BUTTON_DROPDOWN ||
            aWidgetType == NS_THEME_DROPDOWN ||
            aWidgetType == NS_THEME_DROPDOWN_BUTTON) {
          if (aWidgetType == NS_THEME_TOOLBAR_BUTTON_DROPDOWN)
            aFrame = aFrame->GetParent();

          PRBool menuOpen = CheckBooleanAttr(aFrame, nsWidgetAtoms::open);
          aState->depressed = IsCheckedButton(aFrame) || menuOpen;
          // we must not highlight buttons with open drop down menus on hover.
          aState->inHover = aState->inHover && !menuOpen;
        }

        // When the input field of the drop down button has focus, some themes
        // should draw focus for the drop down button as well.
        if (aWidgetType == NS_THEME_DROPDOWN_BUTTON && aWidgetFlags) {
          *aWidgetFlags = CheckBooleanAttr(aFrame, nsWidgetAtoms::parentfocused);
        }
      }
    }
  }

  switch (aWidgetType) {
  case NS_THEME_BUTTON:
  case NS_THEME_TOOLBAR_BUTTON:
  case NS_THEME_TOOLBAR_DUAL_BUTTON:
    if (aWidgetFlags)
      *aWidgetFlags = (aWidgetType == NS_THEME_BUTTON) ? GTK_RELIEF_NORMAL : GTK_RELIEF_NONE;
    aGtkWidgetType = MOZ_GTK_BUTTON;
    break;
  case NS_THEME_CHECKBOX:
  case NS_THEME_CHECKBOX_SMALL:
  case NS_THEME_RADIO:
  case NS_THEME_RADIO_SMALL:
    aGtkWidgetType = IsRadioWidgetType(aWidgetType) ? MOZ_GTK_RADIOBUTTON : MOZ_GTK_CHECKBUTTON;
    break;
  case NS_THEME_SCROLLBAR_BUTTON_UP:
  case NS_THEME_SCROLLBAR_BUTTON_DOWN:
  case NS_THEME_SCROLLBAR_BUTTON_LEFT:
  case NS_THEME_SCROLLBAR_BUTTON_RIGHT:
    aGtkWidgetType = MOZ_GTK_SCROLLBAR_BUTTON;
    break;
  case NS_THEME_SCROLLBAR_TRACK_VERTICAL:
    aGtkWidgetType = MOZ_GTK_SCROLLBAR_TRACK_VERTICAL;
    break;
  case NS_THEME_SCROLLBAR_TRACK_HORIZONTAL:
    aGtkWidgetType = MOZ_GTK_SCROLLBAR_TRACK_HORIZONTAL;
    break;
  case NS_THEME_SCROLLBAR_THUMB_VERTICAL:
    aGtkWidgetType = MOZ_GTK_SCROLLBAR_THUMB_VERTICAL;
    break;
  case NS_THEME_SCROLLBAR_THUMB_HORIZONTAL:
    aGtkWidgetType = MOZ_GTK_SCROLLBAR_THUMB_HORIZONTAL;
    break;
  case NS_THEME_SPINNER:
    aGtkWidgetType = MOZ_GTK_SPINBUTTON;
    break;
  case NS_THEME_SPINNER_UP_BUTTON:
    aGtkWidgetType = MOZ_GTK_SPINBUTTON_UP;
    break;
  case NS_THEME_SPINNER_DOWN_BUTTON:
    aGtkWidgetType = MOZ_GTK_SPINBUTTON_DOWN;
    break;
  case NS_THEME_SPINNER_TEXTFIELD:
    aGtkWidgetType = MOZ_GTK_SPINBUTTON_ENTRY;
    break;
  case NS_THEME_SCALE_HORIZONTAL:
    if (aWidgetFlags)
      *aWidgetFlags = GTK_ORIENTATION_HORIZONTAL;
    aGtkWidgetType = MOZ_GTK_SCALE_HORIZONTAL;
    break;
  case NS_THEME_SCALE_THUMB_HORIZONTAL:
    if (aWidgetFlags)
      *aWidgetFlags = GTK_ORIENTATION_HORIZONTAL;
    aGtkWidgetType = MOZ_GTK_SCALE_THUMB_HORIZONTAL;
    break;
  case NS_THEME_SCALE_VERTICAL:
    if (aWidgetFlags)
      *aWidgetFlags = GTK_ORIENTATION_VERTICAL;
    aGtkWidgetType = MOZ_GTK_SCALE_VERTICAL;
    break;
  case NS_THEME_TOOLBAR_SEPARATOR:
    aGtkWidgetType = MOZ_GTK_TOOLBAR_SEPARATOR;
    break;
  case NS_THEME_SCALE_THUMB_VERTICAL:
    if (aWidgetFlags)
      *aWidgetFlags = GTK_ORIENTATION_VERTICAL;
    aGtkWidgetType = MOZ_GTK_SCALE_THUMB_VERTICAL;
    break;
  case NS_THEME_TOOLBAR_GRIPPER:
    aGtkWidgetType = MOZ_GTK_GRIPPER;
    break;
  case NS_THEME_RESIZER:
    aGtkWidgetType = MOZ_GTK_RESIZER;
    break;
  case NS_THEME_TEXTFIELD:
  case NS_THEME_TEXTFIELD_MULTILINE:
    aGtkWidgetType = MOZ_GTK_ENTRY;
    break;
  case NS_THEME_LISTBOX:
  case NS_THEME_TREEVIEW:
    aGtkWidgetType = MOZ_GTK_TREEVIEW;
    break;
  case NS_THEME_TREEVIEW_HEADER_CELL:
    aGtkWidgetType = MOZ_GTK_TREE_HEADER_CELL;
    break;
  case NS_THEME_TREEVIEW_HEADER_SORTARROW:
    if (aWidgetFlags) {
      switch (GetTreeSortDirection(aFrame)) {
        case eTreeSortDirection_Ascending:
          *aWidgetFlags = GTK_ARROW_DOWN;
          break;
        case eTreeSortDirection_Descending:
          *aWidgetFlags = GTK_ARROW_UP;
          break;
        case eTreeSortDirection_Natural:
        default:
          /* GTK_ARROW_NONE is implemented since GTK 2.10
           * This prevents the treecolums from getting smaller
           * and wider when switching sort direction off and on
           * */
#if GTK_CHECK_VERSION(2,10,0)
          *aWidgetFlags = GTK_ARROW_NONE;
#else
          return PR_FALSE; // Don't draw when we shouldn't
#endif // GTK_CHECK_VERSION(2,10,0)
          break;
      }
    }
    aGtkWidgetType = MOZ_GTK_TREE_HEADER_SORTARROW;
    break;
  case NS_THEME_TREEVIEW_TWISTY:
    aGtkWidgetType = MOZ_GTK_TREEVIEW_EXPANDER;
    if (aWidgetFlags)
      *aWidgetFlags = GTK_EXPANDER_COLLAPSED;
    break;
  case NS_THEME_TREEVIEW_TWISTY_OPEN:
    aGtkWidgetType = MOZ_GTK_TREEVIEW_EXPANDER;
    if (aWidgetFlags)
      *aWidgetFlags = GTK_EXPANDER_EXPANDED;
    break;
  case NS_THEME_DROPDOWN:
    aGtkWidgetType = MOZ_GTK_DROPDOWN;
    break;
  case NS_THEME_DROPDOWN_TEXT:
    return PR_FALSE; // nothing to do, but prevents the bg from being drawn
  case NS_THEME_DROPDOWN_TEXTFIELD:
    aGtkWidgetType = MOZ_GTK_DROPDOWN_ENTRY;
    break;
  case NS_THEME_DROPDOWN_BUTTON:
    aGtkWidgetType = MOZ_GTK_DROPDOWN_ARROW;
    break;
  case NS_THEME_TOOLBAR_BUTTON_DROPDOWN:
    aGtkWidgetType = MOZ_GTK_TOOLBARBUTTON_ARROW;
    break;
  case NS_THEME_CHECKBOX_CONTAINER:
    aGtkWidgetType = MOZ_GTK_CHECKBUTTON_CONTAINER;
    break;
  case NS_THEME_RADIO_CONTAINER:
    aGtkWidgetType = MOZ_GTK_RADIOBUTTON_CONTAINER;
    break;
  case NS_THEME_CHECKBOX_LABEL:
    aGtkWidgetType = MOZ_GTK_CHECKBUTTON_LABEL;
    break;
  case NS_THEME_RADIO_LABEL:
    aGtkWidgetType = MOZ_GTK_RADIOBUTTON_LABEL;
    break;
  case NS_THEME_TOOLBAR:
    aGtkWidgetType = MOZ_GTK_TOOLBAR;
    break;
  case NS_THEME_TOOLTIP:
    aGtkWidgetType = MOZ_GTK_TOOLTIP;
    break;
  case NS_THEME_STATUSBAR_PANEL:
  case NS_THEME_STATUSBAR_RESIZER_PANEL:
    aGtkWidgetType = MOZ_GTK_FRAME;
    break;
  case NS_THEME_PROGRESSBAR:
  case NS_THEME_PROGRESSBAR_VERTICAL:
    aGtkWidgetType = MOZ_GTK_PROGRESSBAR;
    break;
  case NS_THEME_PROGRESSBAR_CHUNK:
  case NS_THEME_PROGRESSBAR_CHUNK_VERTICAL:
    aGtkWidgetType = MOZ_GTK_PROGRESS_CHUNK;
    break;
  case NS_THEME_TAB_SCROLLARROW_BACK:
  case NS_THEME_TAB_SCROLLARROW_FORWARD:
    if (aWidgetFlags)
      *aWidgetFlags = aWidgetType == NS_THEME_TAB_SCROLLARROW_BACK ?
                        GTK_ARROW_LEFT : GTK_ARROW_RIGHT;
    aGtkWidgetType = MOZ_GTK_TAB_SCROLLARROW;
    break;
  case NS_THEME_TAB_PANELS:
    aGtkWidgetType = MOZ_GTK_TABPANELS;
    break;
  case NS_THEME_TAB:
    {
      if (aWidgetFlags) {
        /* First bits will be used to store max(0,-bmargin) where bmargin
         * is the bottom margin of the tab in pixels  (resp. top margin,
         * for bottom tabs). */
        nscoord margin;
        if (IsBottomTab(aFrame)) {
            *aWidgetFlags = MOZ_GTK_TAB_BOTTOM;
            margin = aFrame->GetUsedMargin().top;
        } else {
            *aWidgetFlags = 0;
            margin = aFrame->GetUsedMargin().bottom;
        }

        *aWidgetFlags |= PR_MIN(MOZ_GTK_TAB_MARGIN_MASK,
                                PR_MAX(0, aFrame->PresContext()->
                                   AppUnitsToDevPixels(-margin) ));

        if (IsSelectedTab(aFrame))
          *aWidgetFlags |= MOZ_GTK_TAB_SELECTED;

        if (IsFirstTab(aFrame))
          *aWidgetFlags |= MOZ_GTK_TAB_FIRST;
      }

      aGtkWidgetType = MOZ_GTK_TAB;
    }
    break;
  case NS_THEME_SPLITTER:
    if (IsHorizontal(aFrame))
      aGtkWidgetType = MOZ_GTK_SPLITTER_VERTICAL;
    else 
      aGtkWidgetType = MOZ_GTK_SPLITTER_HORIZONTAL;
    break;
  case NS_THEME_MENUBAR:
    aGtkWidgetType = MOZ_GTK_MENUBAR;
    break;
  case NS_THEME_MENUPOPUP:
    aGtkWidgetType = MOZ_GTK_MENUPOPUP;
    break;
  case NS_THEME_MENUITEM:
    aGtkWidgetType = MOZ_GTK_MENUITEM;
    break;
  case NS_THEME_MENUSEPARATOR:
    aGtkWidgetType = MOZ_GTK_MENUSEPARATOR;
    break;
  case NS_THEME_MENUARROW:
    aGtkWidgetType = MOZ_GTK_MENUARROW;
    break;
  case NS_THEME_CHECKMENUITEM:
    aGtkWidgetType = MOZ_GTK_CHECKMENUITEM;
    break;
  case NS_THEME_RADIOMENUITEM:
    aGtkWidgetType = MOZ_GTK_RADIOMENUITEM;
    break;
  case NS_THEME_WINDOW:
  case NS_THEME_DIALOG:
    aGtkWidgetType = MOZ_GTK_WINDOW;
    break;
  default:
    return PR_FALSE;
  }

  return PR_TRUE;
}

static int
NativeThemeErrorHandler(Display* dpy, XErrorEvent* error) {
  gLastXError = error->error_code;
  return 0;
}

class ThemeRenderer : public gfxXlibNativeRenderer {
public:
  ThemeRenderer(GtkWidgetState aState, GtkThemeWidgetType aGTKWidgetType,
                gint aFlags, GtkTextDirection aDirection,
                const GdkRectangle& aGDKRect, const GdkRectangle& aGDKClip)
    : mState(aState), mGTKWidgetType(aGTKWidgetType), mFlags(aFlags),
      mDirection(aDirection), mGDKRect(aGDKRect), mGDKClip(aGDKClip) {}
  nsresult NativeDraw(Display* dpy, Drawable drawable, Visual* visual,
                      short offsetX, short offsetY,
                      XRectangle* clipRects, PRUint32 numClipRects);
private:
  GtkWidgetState mState;
  GtkThemeWidgetType mGTKWidgetType;
  gint mFlags;
  GtkTextDirection mDirection;
  GdkWindow* mWindow;
  const GdkRectangle& mGDKRect;
  const GdkRectangle& mGDKClip;
};

nsresult
ThemeRenderer::NativeDraw(Display* dpy, Drawable drawable, Visual* visual,
                          short offsetX, short offsetY,
                          XRectangle* clipRects, PRUint32 numClipRects)
{
  GdkRectangle gdk_rect = mGDKRect;
  gdk_rect.x += offsetX;
  gdk_rect.y += offsetY;

  GdkRectangle gdk_clip = mGDKClip;
  gdk_clip.x += offsetX;
  gdk_clip.y += offsetY;
  
  GdkDisplay* gdkDpy = gdk_x11_lookup_xdisplay(dpy);
  if (!gdkDpy)
    return NS_ERROR_FAILURE;

  GdkPixmap* gdkPixmap = gdk_pixmap_lookup_for_display(gdkDpy, drawable);
  if (gdkPixmap) {
    g_object_ref(G_OBJECT(gdkPixmap));
  } else {
    gdkPixmap = gdk_pixmap_foreign_new_for_display(gdkDpy, drawable);
    if (!gdkPixmap)
      return NS_ERROR_FAILURE;
    if (visual) {
      GdkScreen* gdkScreen = gdk_display_get_default_screen(gdkDpy);
      GdkVisual* gdkVisual = gdk_x11_screen_lookup_visual(gdkScreen, visual->visualid);
      Colormap cmap = DefaultScreenOfDisplay(dpy)->cmap;
      GdkColormap* colormap =
        gdk_x11_colormap_foreign_new(gdkVisual, cmap);
                                             
      gdk_drawable_set_colormap(gdkPixmap, colormap);
    }
  }

  NS_ASSERTION(numClipRects == 0, "We don't support clipping!!!");
  moz_gtk_widget_paint(mGTKWidgetType, gdkPixmap, &gdk_rect, &gdk_clip, &mState,
                       mFlags, mDirection);

  g_object_unref(G_OBJECT(gdkPixmap));
  return NS_OK;
}

static PRBool
GetExtraSizeForWidget(PRUint8 aWidgetType, nsIntMargin* aExtra)
{
  *aExtra = nsIntMargin(0,0,0,0);
  // Allow an extra one pixel above and below the thumb for certain
  // GTK2 themes (Ximian Industrial, Bluecurve, Misty, at least);
  // see moz_gtk_scrollbar_thumb_paint in gtk2drawing.c
  switch (aWidgetType) {
  case NS_THEME_SCROLLBAR_THUMB_VERTICAL:
    aExtra->top = aExtra->bottom = 1;
    return PR_TRUE;
  case NS_THEME_SCROLLBAR_THUMB_HORIZONTAL:
    aExtra->left = aExtra->right = 1;
    return PR_TRUE;
  default:
    return PR_FALSE;
  }
}

static GdkRectangle
ConvertToGdkRect(const nsRect &aRect, PRInt32 aP2A)
{
  GdkRectangle gdk_rect;
  gdk_rect.x = NSAppUnitsToIntPixels(aRect.x, aP2A);
  gdk_rect.y = NSAppUnitsToIntPixels(aRect.y, aP2A);
  gdk_rect.width = NSAppUnitsToIntPixels(aRect.XMost(), aP2A) - gdk_rect.x;
  gdk_rect.height = NSAppUnitsToIntPixels(aRect.YMost(), aP2A) - gdk_rect.y;
  return gdk_rect;
}

static GdkRectangle
ConvertGfxToGdkRect(const gfxRect &aRect, const gfxPoint &aTranslation)
{
  GdkRectangle gdk_rect;
  gdk_rect.x = NSToIntRound(aRect.X()) - NSToIntRound(aTranslation.x);
  gdk_rect.y = NSToIntRound(aRect.Y()) - NSToIntRound(aTranslation.y);
  gdk_rect.width = NSToIntRound(aRect.Width());
  gdk_rect.height = NSToIntRound(aRect.Height());
  return gdk_rect;
}

static gfxRect
ConvertToGfxRect(const nsRect &aRect, PRInt32 aP2A)
{
  gfxRect rect(NSAppUnitsToFloatPixels(aRect.x, aP2A),
               NSAppUnitsToFloatPixels(aRect.y, aP2A),
               NSAppUnitsToFloatPixels(aRect.width, aP2A),
               NSAppUnitsToFloatPixels(aRect.height, aP2A));
  return rect;
}

NS_IMETHODIMP
nsNativeThemeGTK::DrawWidgetBackground(nsIRenderingContext* aContext,
                                       nsIFrame* aFrame,
                                       PRUint8 aWidgetType,
                                       const nsRect& aRect,
                                       const nsRect& aClipRect)
{
  GtkWidgetState state;
  GtkThemeWidgetType gtkWidgetType;
  GtkTextDirection direction = GetTextDirection(aFrame);
  gint flags;
  if (!GetGtkWidgetAndState(aWidgetType, aFrame, gtkWidgetType, &state,
                            &flags))
    return NS_OK;
    
  nsCOMPtr<nsIDeviceContext> dctx = nsnull;
  aContext->GetDeviceContext(*getter_AddRefs(dctx));
  PRInt32 p2a = dctx->AppUnitsPerDevPixel();

  // This is the rectangle that will actually be drawn, in appunits
  nsRect drawingRect(aClipRect);
  nsIntMargin extraSize;
  GetExtraSizeForWidget(aWidgetType, &extraSize);
  // inflate drawing rect to account for the overdraw
  nsMargin extraSizeAppUnits(NSIntPixelsToAppUnits(extraSize.left, p2a),
                             NSIntPixelsToAppUnits(extraSize.top, p2a),
                             NSIntPixelsToAppUnits(extraSize.right, p2a),
                             NSIntPixelsToAppUnits(extraSize.bottom, p2a));
  drawingRect.Inflate(extraSizeAppUnits);

  // translate everything so (0,0) is the top left of the drawingRect
  nsIRenderingContext::AutoPushTranslation
    autoTranslation(aContext, drawingRect.x, drawingRect.y);

  NS_ASSERTION(!IsWidgetTypeDisabled(mDisabledWidgetTypes, aWidgetType),
               "Trying to render an unsafe widget!");

  PRBool safeState = IsWidgetStateSafe(mSafeWidgetStates, aWidgetType, &state);
  XErrorHandler oldHandler = nsnull;
  if (!safeState) {
    gLastXError = 0;
    oldHandler = XSetErrorHandler(NativeThemeErrorHandler);
  }

  gfxContext* ctx = aContext->ThebesContext();
  gfxMatrix current = ctx->CurrentMatrix();

  // We require the use of the default display and visual
  // because I'm afraid that otherwise the GTK theme may explode.
  // Some themes (e.g. Clearlooks) just don't clip properly to any
  // clip rect we provide, so we cannot advertise support for clipping within the
  // widget bounds. The gdk_clip is just advisory here, meanining "you don't
  // need to draw outside this rect if you don't feel like it!"
  GdkRectangle gdk_rect, gdk_clip;
  gfxRect gfx_rect = ConvertToGfxRect(aRect - drawingRect.TopLeft(), p2a);
  gfxRect gfx_clip = ConvertToGfxRect(aClipRect - drawingRect.TopLeft(), p2a);
  if (ctx->UserToDevicePixelSnapped(gfx_rect) &&
      ctx->UserToDevicePixelSnapped(gfx_clip)) {
    gfxPoint currentTranslation = current.GetTranslation();
    gdk_rect = ConvertGfxToGdkRect(gfx_rect, currentTranslation);
    gdk_clip = ConvertGfxToGdkRect(gfx_clip, currentTranslation);
  }
  else {
    gdk_rect = ConvertToGdkRect(aRect - drawingRect.TopLeft(), p2a);
    gdk_clip = ConvertToGdkRect(aClipRect - drawingRect.TopLeft(), p2a);
  }
  ThemeRenderer renderer(state, gtkWidgetType, flags, direction, gdk_rect, gdk_clip);

  // XXXbz do we really want to round here, then snap, then round again?
  gfxRect rect(0, 0, NSAppUnitsToIntPixels(drawingRect.width, p2a),
                     NSAppUnitsToIntPixels(drawingRect.height, p2a));

  PRUint32 rendererFlags = gfxXlibNativeRenderer::DRAW_SUPPORTS_OFFSET;
  // Don't snap if it's a non-unit scale factor. We're going to have to take
  // slow paths then in any case.
  PRBool snapXY = ctx->UserToDevicePixelSnapped(rect) &&
    !current.HasNonTranslation();
  if (snapXY) {
    gfxMatrix translation;
    translation.Translate(rect.TopLeft());
    ctx->SetMatrix(translation);
    renderer.Draw(gdk_x11_get_default_xdisplay(), ctx,
                  NSToCoordRound(rect.Width()), NSToCoordRound(rect.Height()),
                  rendererFlags, nsnull);
    ctx->SetMatrix(current);
  } else {
    renderer.Draw(gdk_x11_get_default_xdisplay(), ctx,
                  NSToIntCeil(NSAppUnitsToFloatPixels(drawingRect.width, p2a)),
                  NSToIntCeil(NSAppUnitsToFloatPixels(drawingRect.height, p2a)),
                  rendererFlags, nsnull);
  }

  if (!safeState) {
    gdk_flush();
    XSetErrorHandler(oldHandler);

    if (gLastXError) {
#ifdef DEBUG
      printf("GTK theme failed for widget type %d, error was %d, state was "
             "[active=%d,focused=%d,inHover=%d,disabled=%d]\n",
             aWidgetType, gLastXError, state.active, state.focused,
             state.inHover, state.disabled);
#endif
      NS_WARNING("GTK theme failed; disabling unsafe widget");
      SetWidgetTypeDisabled(mDisabledWidgetTypes, aWidgetType);
      // force refresh of the window, because the widget was not
      // successfully drawn it must be redrawn using the default look
      RefreshWidgetWindow(aFrame);
    } else {
      SetWidgetStateSafe(mSafeWidgetStates, aWidgetType, &state);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsNativeThemeGTK::GetWidgetBorder(nsIDeviceContext* aContext, nsIFrame* aFrame,
                                  PRUint8 aWidgetType, nsMargin* aResult)
{
  GtkTextDirection direction = GetTextDirection(aFrame);
  aResult->top = aResult->left = aResult->right = aResult->bottom = 0;
  switch (aWidgetType) {
  case NS_THEME_SCROLLBAR_TRACK_VERTICAL:
  case NS_THEME_SCROLLBAR_TRACK_HORIZONTAL:
    {
      MozGtkScrollbarMetrics metrics;
      moz_gtk_get_scrollbar_metrics(&metrics);
      aResult->top = aResult->left = aResult->right = aResult->bottom = metrics.trough_border;
    }
    break;
  case NS_THEME_TOOLBOX:
    // gtk has no toolbox equivalent.  So, although we map toolbox to
    // gtk's 'toolbar' for purposes of painting the widget background,
    // we don't use the toolbar border for toolbox.
    break;
  case NS_THEME_TOOLBAR_DUAL_BUTTON:
    // TOOLBAR_DUAL_BUTTON is an interesting case.  We want a border to draw
    // around the entire button + dropdown, and also an inner border if you're
    // over the button part.  But, we want the inner button to be right up
    // against the edge of the outer button so that the borders overlap.
    // To make this happen, we draw a button border for the outer button,
    // but don't reserve any space for it.
    break;
  case NS_THEME_TAB:
    // Top tabs have no bottom border, bottom tabs have no top border
    moz_gtk_get_widget_border(MOZ_GTK_TAB, &aResult->left, &aResult->top,
                              &aResult->right, &aResult->bottom, direction,
                              FALSE);
    if (IsBottomTab(aFrame))
        aResult->top = 0;
    else
        aResult->bottom = 0;
    break;
  default:
    {
      GtkThemeWidgetType gtkWidgetType;
      if (GetGtkWidgetAndState(aWidgetType, aFrame, gtkWidgetType, nsnull,
                               nsnull))
        moz_gtk_get_widget_border(gtkWidgetType, &aResult->left, &aResult->top,
                                  &aResult->right, &aResult->bottom, direction,
                                  aFrame && aFrame->GetContent()->
                                        IsNodeOfType(nsINode::eHTML));
    }
  }
  return NS_OK;
}

PRBool
nsNativeThemeGTK::GetWidgetPadding(nsIDeviceContext* aContext,
                                   nsIFrame* aFrame, PRUint8 aWidgetType,
                                   nsMargin* aResult)
{
  if (aWidgetType == NS_THEME_BUTTON_FOCUS ||
      aWidgetType == NS_THEME_TOOLBAR_BUTTON ||
      aWidgetType == NS_THEME_TOOLBAR_DUAL_BUTTON ||
      aWidgetType == NS_THEME_TAB_SCROLLARROW_BACK ||
      aWidgetType == NS_THEME_TAB_SCROLLARROW_FORWARD) {
    aResult->SizeTo(0, 0, 0, 0);
    return PR_TRUE;
  }

  return PR_FALSE;
}

PRBool
nsNativeThemeGTK::GetWidgetOverflow(nsIDeviceContext* aContext,
                                    nsIFrame* aFrame, PRUint8 aWidgetType,
                                    nsRect* aResult)
{
  nsMargin m;
  PRInt32 p2a;
  if (aWidgetType == NS_THEME_TAB)
  {
    if (!IsSelectedTab(aFrame))
      return PR_FALSE;

    p2a = aContext->AppUnitsPerDevPixel();

    if (IsBottomTab(aFrame)) {
      m = nsMargin(0, NSIntPixelsToAppUnits(moz_gtk_get_tab_thickness(), p2a)
                      + PR_MIN(0, aFrame->GetUsedMargin().top), 0, 0);
    } else {
      m = nsMargin(0, 0, 0,
                   NSIntPixelsToAppUnits(moz_gtk_get_tab_thickness(), p2a)
                   + PR_MIN(0, aFrame->GetUsedMargin().bottom));
    }
  } else {
    nsIntMargin extraSize;
    if (!GetExtraSizeForWidget(aWidgetType, &extraSize))
      return PR_FALSE;

    p2a = aContext->AppUnitsPerDevPixel();
    m = nsMargin(NSIntPixelsToAppUnits(extraSize.left, p2a),
                 NSIntPixelsToAppUnits(extraSize.top, p2a),
                 NSIntPixelsToAppUnits(extraSize.right, p2a),
                 NSIntPixelsToAppUnits(extraSize.bottom, p2a));
  }

  nsRect r(nsPoint(0, 0), aFrame->GetSize());
  r.Inflate(m);
  *aResult = r;
  return PR_TRUE;
}

NS_IMETHODIMP
nsNativeThemeGTK::GetMinimumWidgetSize(nsIRenderingContext* aContext,
                                       nsIFrame* aFrame, PRUint8 aWidgetType,
                                       nsSize* aResult, PRBool* aIsOverridable)
{
  aResult->width = aResult->height = 0;
  *aIsOverridable = PR_TRUE;

  switch (aWidgetType) {
    case NS_THEME_SCROLLBAR_BUTTON_UP:
    case NS_THEME_SCROLLBAR_BUTTON_DOWN:
      {
        MozGtkScrollbarMetrics metrics;
        moz_gtk_get_scrollbar_metrics(&metrics);

        aResult->width = metrics.slider_width;
        aResult->height = metrics.stepper_size;
        *aIsOverridable = PR_FALSE;
      }
      break;
    case NS_THEME_SCROLLBAR_BUTTON_LEFT:
    case NS_THEME_SCROLLBAR_BUTTON_RIGHT:
      {
        MozGtkScrollbarMetrics metrics;
        moz_gtk_get_scrollbar_metrics(&metrics);

        aResult->width = metrics.stepper_size;
        aResult->height = metrics.slider_width;
        *aIsOverridable = PR_FALSE;
      }
      break;
    case NS_THEME_SPLITTER:
    {
      gint metrics;
      if (IsHorizontal(aFrame)) {
        moz_gtk_splitter_get_metrics(GTK_ORIENTATION_HORIZONTAL, &metrics);
        aResult->width = metrics;
        aResult->height = 0;
      } else {
        moz_gtk_splitter_get_metrics(GTK_ORIENTATION_VERTICAL, &metrics);
        aResult->width = 0;
        aResult->height = metrics;
      }
      *aIsOverridable = PR_FALSE;
    }
    break;
    case NS_THEME_SCROLLBAR_THUMB_VERTICAL:
    case NS_THEME_SCROLLBAR_THUMB_HORIZONTAL:
      {
        MozGtkScrollbarMetrics metrics;
        moz_gtk_get_scrollbar_metrics(&metrics);

        nsRect rect = aFrame->GetParent()->GetRect();
        PRInt32 p2a = aFrame->PresContext()->DeviceContext()->
                        AppUnitsPerDevPixel();
        nsMargin margin;

        /* Get the available space, if that is smaller then the minimum size,
         * adjust the mininum size to fit into it.
         * Setting aIsOverridable to PR_TRUE has no effect for thumbs. */
        aFrame->GetMargin(margin);
        rect.Deflate(margin);
        aFrame->GetParent()->GetBorderAndPadding(margin);
        rect.Deflate(margin);

        if (aWidgetType == NS_THEME_SCROLLBAR_THUMB_VERTICAL) {
          aResult->width = metrics.slider_width;
          aResult->height = PR_MIN(NSAppUnitsToIntPixels(rect.height, p2a),
                                   metrics.min_slider_size);
        } else {
          aResult->height = metrics.slider_width;
          aResult->width = PR_MIN(NSAppUnitsToIntPixels(rect.width, p2a),
                                  metrics.min_slider_size);
        }

        *aIsOverridable = PR_FALSE;
      }
      break;
    case NS_THEME_SCALE_THUMB_HORIZONTAL:
    case NS_THEME_SCALE_THUMB_VERTICAL:
      {
        gint thumb_length, thumb_height;

        if (aWidgetType == NS_THEME_SCALE_THUMB_VERTICAL) {
          moz_gtk_get_scalethumb_metrics(GTK_ORIENTATION_VERTICAL, &thumb_length, &thumb_height);
          aResult->width = thumb_height;
          aResult->height = thumb_length;
        } else {
          moz_gtk_get_scalethumb_metrics(GTK_ORIENTATION_HORIZONTAL, &thumb_length, &thumb_height);
          aResult->width = thumb_length;
          aResult->height = thumb_height;
        }

        *aIsOverridable = PR_FALSE;
      }
      break;
    case NS_THEME_TAB_SCROLLARROW_BACK:
    case NS_THEME_TAB_SCROLLARROW_FORWARD:
      {
        moz_gtk_get_tab_scroll_arrow_size(&aResult->width, &aResult->height);
        *aIsOverridable = PR_FALSE;
      }
      break;
  case NS_THEME_DROPDOWN_BUTTON:
    {
      moz_gtk_get_dropdown_arrow_size(&aResult->width, &aResult->height);
      *aIsOverridable = PR_FALSE;
    }
    break;
  case NS_THEME_MENUSEPARATOR:
    {
      gint separator_height;

      moz_gtk_get_menu_separator_height(&separator_height);
      aResult->height = separator_height;
    
      *aIsOverridable = PR_FALSE;
    }
    break;
  case NS_THEME_CHECKBOX:
  case NS_THEME_CHECKBOX_SMALL:
  case NS_THEME_RADIO:
  case NS_THEME_RADIO_SMALL:
    {
      gint indicator_size, indicator_spacing;

      if (IsCheckboxWidgetType(aWidgetType)) {
        moz_gtk_checkbox_get_metrics(&indicator_size, &indicator_spacing);
      } else {
        moz_gtk_radio_get_metrics(&indicator_size, &indicator_spacing);
      }

      // Include space for the indicator and the padding around it.
      aResult->width = indicator_size + 2 * indicator_spacing;
      aResult->height = indicator_size + 2 * indicator_spacing;
      *aIsOverridable = PR_FALSE;
    }
    break;

  case NS_THEME_CHECKBOX_CONTAINER:
  case NS_THEME_RADIO_CONTAINER:
  case NS_THEME_CHECKBOX_LABEL:
  case NS_THEME_RADIO_LABEL:
  case NS_THEME_BUTTON:
  case NS_THEME_TOOLBAR_BUTTON:
  case NS_THEME_TREEVIEW_HEADER_CELL:
    {
      // Just include our border, and let the box code augment the size.

      nsCOMPtr<nsIDeviceContext> dc;
      aContext->GetDeviceContext(*getter_AddRefs(dc));

      nsMargin border;
      nsNativeThemeGTK::GetWidgetBorder(dc, aFrame, aWidgetType, &border);
      aResult->width = border.left + border.right;
      aResult->height = border.top + border.bottom;
    }
    break;
  case NS_THEME_TOOLBAR_SEPARATOR:
    {
      gint separator_width;
    
      moz_gtk_get_toolbar_separator_width(&separator_width);
    
      aResult->width = separator_width;
    }
    break;
  case NS_THEME_SPINNER:
    // hard code these sizes
    aResult->width = 14;
    aResult->height = 26;
    break;
  case NS_THEME_TREEVIEW_HEADER_SORTARROW:
  case NS_THEME_SPINNER_UP_BUTTON:
  case NS_THEME_SPINNER_DOWN_BUTTON:
    // hard code these sizes
    aResult->width = 14;
    aResult->height = 13;
    break;
  case NS_THEME_RESIZER:
    // same as Windows to make our lives easier
    aResult->width = aResult->height = 15;
    break;
  case NS_THEME_TREEVIEW_TWISTY:
  case NS_THEME_TREEVIEW_TWISTY_OPEN:
    {
      gint expander_size;

      moz_gtk_get_treeview_expander_size(&expander_size);
      aResult->width = aResult->height = expander_size;
      *aIsOverridable = PR_FALSE;
    }
    break;
  }
  return NS_OK;
}

NS_IMETHODIMP
nsNativeThemeGTK::WidgetStateChanged(nsIFrame* aFrame, PRUint8 aWidgetType, 
                                     nsIAtom* aAttribute, PRBool* aShouldRepaint)
{
  // Some widget types just never change state.
  if (aWidgetType == NS_THEME_TOOLBOX ||
      aWidgetType == NS_THEME_TOOLBAR ||
      aWidgetType == NS_THEME_STATUSBAR ||
      aWidgetType == NS_THEME_STATUSBAR_PANEL ||
      aWidgetType == NS_THEME_STATUSBAR_RESIZER_PANEL ||
      aWidgetType == NS_THEME_PROGRESSBAR_CHUNK ||
      aWidgetType == NS_THEME_PROGRESSBAR_CHUNK_VERTICAL ||
      aWidgetType == NS_THEME_PROGRESSBAR ||
      aWidgetType == NS_THEME_PROGRESSBAR_VERTICAL ||
      aWidgetType == NS_THEME_MENUBAR ||
      aWidgetType == NS_THEME_MENUPOPUP ||
      aWidgetType == NS_THEME_TOOLTIP ||
      aWidgetType == NS_THEME_MENUSEPARATOR ||
      aWidgetType == NS_THEME_WINDOW ||
      aWidgetType == NS_THEME_DIALOG) {
    *aShouldRepaint = PR_FALSE;
    return NS_OK;
  }

  if ((aWidgetType == NS_THEME_SCROLLBAR_BUTTON_UP ||
       aWidgetType == NS_THEME_SCROLLBAR_BUTTON_DOWN ||
       aWidgetType == NS_THEME_SCROLLBAR_BUTTON_LEFT ||
       aWidgetType == NS_THEME_SCROLLBAR_BUTTON_RIGHT) &&
      (aAttribute == nsWidgetAtoms::curpos ||
       aAttribute == nsWidgetAtoms::maxpos)) {
    *aShouldRepaint = PR_TRUE;
    return NS_OK;
  }

  // XXXdwh Not sure what can really be done here.  Can at least guess for
  // specific widgets that they're highly unlikely to have certain states.
  // For example, a toolbar doesn't care about any states.
  if (!aAttribute) {
    // Hover/focus/active changed.  Always repaint.
    *aShouldRepaint = PR_TRUE;
  }
  else {
    // Check the attribute to see if it's relevant.  
    // disabled, checked, dlgtype, default, etc.
    *aShouldRepaint = PR_FALSE;
    if (aAttribute == nsWidgetAtoms::disabled ||
        aAttribute == nsWidgetAtoms::checked ||
        aAttribute == nsWidgetAtoms::selected ||
        aAttribute == nsWidgetAtoms::focused ||
        aAttribute == nsWidgetAtoms::readonly ||
        aAttribute == nsWidgetAtoms::_default ||
        aAttribute == nsWidgetAtoms::mozmenuactive ||
        aAttribute == nsWidgetAtoms::open ||
        aAttribute == nsWidgetAtoms::parentfocused)
      *aShouldRepaint = PR_TRUE;
  }

  return NS_OK;
}

NS_IMETHODIMP
nsNativeThemeGTK::ThemeChanged()
{
  // this totally sucks.  this method is really supposed to be
  // static, which is why we can call it without any initialization.
  static NS_DEFINE_CID(kDeviceContextCID, NS_DEVICE_CONTEXT_CID);
  nsCOMPtr<nsIDeviceContext> dctx = do_CreateInstance(kDeviceContextCID);
  dctx->ClearCachedSystemFonts();

  memset(mDisabledWidgetTypes, 0, sizeof(mDisabledWidgetTypes));
  return NS_OK;
}

NS_IMETHODIMP_(PRBool)
nsNativeThemeGTK::ThemeSupportsWidget(nsPresContext* aPresContext,
                                      nsIFrame* aFrame,
                                      PRUint8 aWidgetType)
{
  if (IsWidgetTypeDisabled(mDisabledWidgetTypes, aWidgetType))
    return PR_FALSE;

  switch (aWidgetType) {
  case NS_THEME_BUTTON:
  case NS_THEME_BUTTON_FOCUS:
  case NS_THEME_RADIO:
  case NS_THEME_RADIO_SMALL:
  case NS_THEME_CHECKBOX:
  case NS_THEME_CHECKBOX_SMALL:
  case NS_THEME_TOOLBOX: // N/A
  case NS_THEME_TOOLBAR:
  case NS_THEME_TOOLBAR_BUTTON:
  case NS_THEME_TOOLBAR_DUAL_BUTTON: // so we can override the border with 0
  case NS_THEME_TOOLBAR_BUTTON_DROPDOWN:
  case NS_THEME_TOOLBAR_SEPARATOR:
  case NS_THEME_TOOLBAR_GRIPPER:
  case NS_THEME_STATUSBAR:
  case NS_THEME_STATUSBAR_PANEL:
  case NS_THEME_STATUSBAR_RESIZER_PANEL:
  case NS_THEME_RESIZER:
  case NS_THEME_LISTBOX:
    // case NS_THEME_LISTBOX_LISTITEM:
  case NS_THEME_TREEVIEW:
    // case NS_THEME_TREEVIEW_TREEITEM:
  case NS_THEME_TREEVIEW_TWISTY:
    // case NS_THEME_TREEVIEW_LINE:
    // case NS_THEME_TREEVIEW_HEADER:
  case NS_THEME_TREEVIEW_HEADER_CELL:
  case NS_THEME_TREEVIEW_HEADER_SORTARROW:
  case NS_THEME_TREEVIEW_TWISTY_OPEN:
    case NS_THEME_PROGRESSBAR:
    case NS_THEME_PROGRESSBAR_CHUNK:
    case NS_THEME_PROGRESSBAR_VERTICAL:
    case NS_THEME_PROGRESSBAR_CHUNK_VERTICAL:
    case NS_THEME_TAB:
    // case NS_THEME_TAB_PANEL:
    case NS_THEME_TAB_PANELS:
    case NS_THEME_TAB_SCROLLARROW_BACK:
    case NS_THEME_TAB_SCROLLARROW_FORWARD:
  case NS_THEME_TOOLTIP:
  case NS_THEME_SPINNER:
  case NS_THEME_SPINNER_UP_BUTTON:
  case NS_THEME_SPINNER_DOWN_BUTTON:
  case NS_THEME_SPINNER_TEXTFIELD:
    // case NS_THEME_SCROLLBAR:  (n/a for gtk)
    // case NS_THEME_SCROLLBAR_SMALL: (n/a for gtk)
  case NS_THEME_SCROLLBAR_BUTTON_UP:
  case NS_THEME_SCROLLBAR_BUTTON_DOWN:
  case NS_THEME_SCROLLBAR_BUTTON_LEFT:
  case NS_THEME_SCROLLBAR_BUTTON_RIGHT:
  case NS_THEME_SCROLLBAR_TRACK_HORIZONTAL:
  case NS_THEME_SCROLLBAR_TRACK_VERTICAL:
  case NS_THEME_SCROLLBAR_THUMB_HORIZONTAL:
  case NS_THEME_SCROLLBAR_THUMB_VERTICAL:
    // case NS_THEME_SCROLLBAR_GRIPPER_HORIZONTAL:  (n/a for gtk)
    // case NS_THEME_SCROLLBAR_GRIPPER_VERTICAL:  (n/a for gtk)
  case NS_THEME_TEXTFIELD:
  case NS_THEME_TEXTFIELD_MULTILINE:
    // case NS_THEME_TEXTFIELD_CARET:
  case NS_THEME_DROPDOWN_TEXTFIELD:
  case NS_THEME_SCALE_HORIZONTAL:
  case NS_THEME_SCALE_THUMB_HORIZONTAL:
  case NS_THEME_SCALE_VERTICAL:
  case NS_THEME_SCALE_THUMB_VERTICAL:
    // case NS_THEME_SCALE_THUMB_START:
    // case NS_THEME_SCALE_THUMB_END:
    // case NS_THEME_SCALE_TICK:
  case NS_THEME_CHECKBOX_CONTAINER:
  case NS_THEME_RADIO_CONTAINER:
  case NS_THEME_CHECKBOX_LABEL:
  case NS_THEME_RADIO_LABEL:
  case NS_THEME_MENUBAR:
  case NS_THEME_MENUPOPUP:
  case NS_THEME_MENUITEM:
  case NS_THEME_MENUARROW:
  case NS_THEME_MENUSEPARATOR:
  case NS_THEME_CHECKMENUITEM:
  case NS_THEME_RADIOMENUITEM:
  case NS_THEME_SPLITTER:
  case NS_THEME_WINDOW:
  case NS_THEME_DIALOG:
  case NS_THEME_DROPDOWN:
  case NS_THEME_DROPDOWN_TEXT:
    return !IsWidgetStyled(aPresContext, aFrame, aWidgetType);

  case NS_THEME_DROPDOWN_BUTTON:
    // "Native" dropdown buttons cause padding and margin problems, but only
    // in HTML so allow them in XUL.
    return (!aFrame || aFrame->GetContent()->IsNodeOfType(nsINode::eXUL)) &&
           !IsWidgetStyled(aPresContext, aFrame, aWidgetType);

  }

  return PR_FALSE;
}

NS_IMETHODIMP_(PRBool)
nsNativeThemeGTK::WidgetIsContainer(PRUint8 aWidgetType)
{
  // XXXdwh At some point flesh all of this out.
  if (aWidgetType == NS_THEME_DROPDOWN_BUTTON ||
      IsRadioWidgetType(aWidgetType) ||
      IsCheckboxWidgetType(aWidgetType) ||
      aWidgetType == NS_THEME_TAB_SCROLLARROW_BACK ||
      aWidgetType == NS_THEME_TAB_SCROLLARROW_FORWARD)
    return PR_FALSE;
  return PR_TRUE;
}

PRBool
nsNativeThemeGTK::ThemeDrawsFocusForWidget(nsPresContext* aPresContext, nsIFrame* aFrame, PRUint8 aWidgetType)
{
   if (aWidgetType == NS_THEME_DROPDOWN ||
      aWidgetType == NS_THEME_BUTTON || 
      aWidgetType == NS_THEME_TREEVIEW_HEADER_CELL)
    return PR_TRUE;
  
  return PR_FALSE;
}

PRBool
nsNativeThemeGTK::ThemeNeedsComboboxDropmarker()
{
  return PR_FALSE;
}
