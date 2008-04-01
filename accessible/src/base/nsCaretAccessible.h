/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1998
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

#ifndef __nsCaretAccessible_h__
#define __nsCaretAccessible_h__

#include "nsIWeakReference.h"
#include "nsIAccessibleText.h"
#include "nsICaret.h"
#include "nsIDOMNode.h"
#include "nsISelectionListener.h"
#include "nsRect.h"

class nsRootAccessible;
class nsIView;

/*
 * This special accessibility class is for the caret, which is really the currently focused selection.
 * There is only 1 visible caret per top level window (nsRootAccessible),
 * However, there may be several visible selections.
 *
 * The important selections are the one owned by each document, and the one in the currently focused control.
 *
 * The caret accessible is no longer an accessible object in its own right.
 * On Windows it is used to move an invisible system caret that shadows the Mozilla caret. Windows will
 * also automatically map this to the MSAA caret accessible object (via OBJID_CARET).
 * (as opposed to the root accessible tree for a window which is retrieved with OBJID_CLIENT)
 * For ATK and Iaccessible2, the caret accessible is used to fire
 * caret move and selection change events.
 *
 * The caret accessible is owned by the nsRootAccessible for the top level window that it's in.
 * The nsRootAccessible needs to tell the nsCaretAccessible about focus changes via
 * setControlSelectionListener().
 * Each nsDocAccessible needs to tell the nsCaretAccessible owned by the root to
 * listen for selection events via addDocSelectionListener() and then needs to remove the 
 * selection listener when the doc goes away via removeDocSelectionListener().
 */

class nsCaretAccessible : public nsISelectionListener
{
public:
  NS_DECL_ISUPPORTS

  nsCaretAccessible(nsRootAccessible *aRootAccessible);
  virtual ~nsCaretAccessible();
  void Shutdown();

  /* ----- nsISelectionListener ---- */
  NS_DECL_NSISELECTIONLISTENER

  /**
   * Listen to selection events on the focused control.
   * Only one control's selection events are listened to at a time, per top-level window.
   * This will remove the previous control's selection listener.
   * It will fail if aFocusedNode is a document node -- document selection must be listened
   * to via AddDocSelectionListener().
   * @param aFocusedNode   The node for the focused control
   */
  nsresult SetControlSelectionListener(nsIDOMNode *aCurrentNode);

  /**
   * Stop listening to selection events for any control.
   * This does not have to be called explicitly in Shutdown() procedures,
   * because the nsCaretAccessible implementation guarantees that.
   */
  nsresult ClearControlSelectionListener();

  /**
   * Start listening to selection events for a given document
   * More than one document's selection events can be listened to
   * at the same time, by a given nsCaretAccessible
   * @param aShell   PresShell for document to listen to selection events from.
   */
  nsresult AddDocSelectionListener(nsIPresShell *aShell);

  /**
   * Stop listening to selection events for a given document
   * If the document goes away, this method needs to be called for 
   * that document by the owner of the caret. We use presShell because
   * instead of document because it is more direct than getting it from
   * the document, and in any case it is unavailable from the doc after a pagehide.
   * @param aShell   PresShell for document to no longer listen to selection events from.
   */
  nsresult RemoveDocSelectionListener(nsIPresShell *aShell);

  nsRect GetCaretRect(nsIWidget **aOutWidget);

private:
  // The currently focused control -- never a document.
  // We listen to selection for one control at a time (the focused one)
  // Document selection is handled separately via additional listeners on all active documents
  // The current control is set via SetControlSelectionListener()
  nsCOMPtr<nsIDOMNode> mCurrentControl;  // Selection controller for the currently focused control
  nsCOMPtr<nsIWeakReference> mCurrentControlSelection;

  // Info for the the last selection event
  // If it was on a control, then mLastUsedSelection == mCurrentControlSelection
  // Otherwise, it's for a document where the selection changed
  nsCOMPtr<nsIWeakReference> mLastUsedSelection; // Weak ref to nsISelection
  nsCOMPtr<nsIAccessibleText> mLastTextAccessible;
  PRInt32 mLastCaretOffset;

  nsRootAccessible *mRootAccessible;
};

#endif
