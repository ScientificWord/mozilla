/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 sw=2 et tw=78: */
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

/* the caret is the text cursor used, e.g., when editing */

#ifndef nsICaret_h__
#define nsICaret_h__

#include "nsISupports.h"
#include "nsCOMPtr.h"
#include "nsCoord.h"
#include "nsFrameSelection.h"

struct nsRect;
struct nsPoint;

class nsIRenderingContext;
class nsIFrame;
class nsIView;
class nsIPresShell;
class nsISelection;
class nsIDOMNode;

// IID for the nsICaret interface
#define NS_ICARET_IID \
{ 0xb899c578, 0x0894, 0x4d3b, \
  { 0x81, 0xa0, 0x7a, 0xfe, 0x4f, 0xb9, 0xa9, 0x39 } }

class nsICaret: public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ICARET_IID)

  enum EViewCoordinates {
      eTopLevelWindowCoordinates,
      eRenderingViewCoordinates,
      eClosestViewCoordinates,
      eIMECoordinates
    };

  NS_IMETHOD Init(nsIPresShell *inPresShell) = 0;
  NS_IMETHOD Terminate() = 0;
  
  NS_IMETHOD GetCaretDOMSelection(nsISelection **aDOMSel) = 0;
  NS_IMETHOD SetCaretDOMSelection(nsISelection *aDOMSel) = 0;

  /** SetCaretVisible will set the visibility of the caret
   *  @param inMakeVisible PR_TRUE to show the caret, PR_FALSE to hide it
   */
  NS_IMETHOD SetCaretVisible(PRBool inMakeVisible) = 0;

  /** GetCaretVisible will get the visibility of the caret
   *  @param inMakeVisible PR_TRUE it is shown, PR_FALSE it is hidden
   */
  NS_IMETHOD GetCaretVisible(PRBool *outMakeVisible) = 0;

  /** SetCaretReadOnly set the appearance of the caret
   *  @param inMakeReadonly PR_TRUE to show the caret in a 'read only' state,
   *  PR_FALSE to show the caret in normal, editing state
   */
  NS_IMETHOD SetCaretReadOnly(PRBool inMakeReadonly) = 0;

  virtual PRBool GetCaretReadOnly() = 0;

  /** GetCaretCoordinates
   *  Get the position of the caret in coordinates relative to the typed
   *  specified (aRelativeToType).
   *  If the selection is collapsed, this returns the caret location
   *    and true in outIsCollapsed.
   *  If the selection is not collapsed, this returns the location of the focus pos,
   *    and false in outIsCollapsed.
   */
  NS_IMETHOD GetCaretCoordinates(EViewCoordinates aRelativeToType,
                                 nsISelection *aDOMSel,
                                 nsRect *outCoordinates,
                                 PRBool *outIsCollapsed,
                                 nsIView **outView) = 0;

  /** Erase Caret
   *  this will erase the caret if its drawn and reset drawn status
   */
  NS_IMETHOD EraseCaret() = 0;

  NS_IMETHOD SetVisibilityDuringSelection(PRBool aVisibilityDuringSelection) = 0;
  
  /** DrawAtPosition
   *  
   *  Draw the caret explicitly, at the specified node and offset.
   *  To avoid drawing glitches, you should call EraseCaret()
   *  after each call to DrawAtPosition().
   *
   *  Note: This call breaks the caret's ability to blink at all.
   **/
  NS_IMETHOD DrawAtPosition(nsIDOMNode* aNode, PRInt32 aOffset) = 0;

  /** GetCaretFrameForNodeOffset
   *  Get the frame and content offset at which the caret is drawn,
   *  invoking the bidi caret positioning algorithm if necessary
   **/
  NS_IMETHOD GetCaretFrameForNodeOffset(nsIContent* aContentNode,
                                        PRInt32 aOffset,
                                        nsFrameSelection::HINT aFrameHint,
                                        PRUint8 aBidiLevel,
                                        nsIFrame** aReturnFrame,
                                        PRInt32* aReturnOffset) = 0;

  /** GetCaretFrame
   *  Get the current frame that the caret should be drawn in. If the caret is
   *  not currently visible (i.e., it is between blinks), then this will
   *  return null.
   */
  virtual nsIFrame *GetCaretFrame() = 0;

  /** GetCaretRect
   *  Get the current caret rect. Only call this when GetCaretFrame returns
   *  non-null.
   */
  virtual nsRect GetCaretRect() = 0;

  /** GetCaretContent
   *  Get the content that the caret was last drawn in.
   */
  virtual nsIContent* GetCaretContent() = 0;

  /** InvalidateOutsideCaret
   *  Invalidate the area that the caret currently occupies if the caret is
   *  outside of its frame's overflow area. This is used when the content that
   *  the caret is currently drawn is is being deleted or reflowed.
   */
  virtual void InvalidateOutsideCaret() = 0;

  /** UpdateCaretPosition
   *  Update the caret's current frame and rect, but don't draw yet. This is
   *  useful for flickerless moving of the caret (e.g., when the frame the
   *  caret is in reflows and is moved).
   */
  virtual void UpdateCaretPosition() = 0;

  /** PaintCaret
   *  Actually paint the caret onto the given rendering context.
   */
  virtual void PaintCaret(nsDisplayListBuilder *aBuilder,
                          nsIRenderingContext *aCtx,
                          const nsPoint &aOffset,
                          nscolor aColor) = 0;

  /**
   * Sets whether the caret should only be visible in nodes that are not
   * user-modify: read-only, or whether it should be visible in all nodes.
   *
   * @param aIgnoreUserModify PR_TRUE to have the cursor visible in all nodes,
   *                          PR_FALSE to have it visible in all nodes except
   *                          those with user-modify: read-only
   */
  virtual void SetIgnoreUserModify(PRBool aIgnoreUserModify) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsICaret, NS_ICARET_IID)

nsresult
NS_NewCaret(nsICaret** aInstancePtrResult);


// handy stack-based class for temporarily disabling the caret

class StCaretHider
{
public:
               StCaretHider(nsICaret* aSelCon)
               : mWasVisible(PR_FALSE), mCaret(aSelCon)
               {
                 if (mCaret)
                 {
                   mCaret->GetCaretVisible(&mWasVisible);
                   if (mWasVisible)
                     mCaret->SetCaretVisible(PR_FALSE);
                 }
               }
               
               ~StCaretHider()
               {
                 if (mCaret && mWasVisible)
                   mCaret->SetCaretVisible(PR_TRUE);
                 // nsCOMPtr releases mPresShell
               }

protected:

    PRBool                  mWasVisible;
    nsCOMPtr<nsICaret>  mCaret;
};


#endif  // nsICaret_h__

