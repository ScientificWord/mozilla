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

/*
 * base class for rendering objects that can be split across lines,
 * columns, or pages
 */

#ifndef nsSplittableFrame_h___
#define nsSplittableFrame_h___

#include "nsFrame.h"

// Derived class that allows splitting
class nsSplittableFrame : public nsFrame
{
public:
  NS_IMETHOD Init(nsIContent*      aContent,
                  nsIFrame*        aParent,
                  nsIFrame*        aPrevInFlow);
  
  virtual nsSplittableType GetSplittableType() const;

  virtual void Destroy();

  /*
   * Frame continuations can be either fluid or not:
   * Fluid continuations ("in-flows") are the result of line breaking, 
   * column breaking, or page breaking.
   * Other (non-fluid) continuations can be the result of BiDi frame splitting.
   * A "flow" is a chain of fluid continuations.
   */
  
  // Get the previous/next continuation, regardless of its type (fluid or non-fluid).
  virtual nsIFrame* GetPrevContinuation() const;
  virtual nsIFrame* GetNextContinuation() const;

  // Set a previous/next non-fluid continuation.
  NS_IMETHOD SetPrevContinuation(nsIFrame*);
  NS_IMETHOD SetNextContinuation(nsIFrame*);

  // Get the first/last continuation for this frame.
  virtual nsIFrame* GetFirstContinuation() const;
  virtual nsIFrame* GetLastContinuation() const;

#ifdef DEBUG
  // Can aFrame2 be reached from aFrame1 by following prev/next continuations?
  static PRBool IsInPrevContinuationChain(nsIFrame* aFrame1, nsIFrame* aFrame2);
  static PRBool IsInNextContinuationChain(nsIFrame* aFrame1, nsIFrame* aFrame2);
#endif
  
  // Get the previous/next continuation, only if it is fluid (an "in-flow").
  nsIFrame* GetPrevInFlow() const;
  nsIFrame* GetNextInFlow() const;

  virtual nsIFrame* GetPrevInFlowVirtual() const { return GetPrevInFlow(); }
  virtual nsIFrame* GetNextInFlowVirtual() const { return GetNextInFlow(); }
  
  // Set a previous/next fluid continuation.
  NS_IMETHOD  SetPrevInFlow(nsIFrame*);
  NS_IMETHOD  SetNextInFlow(nsIFrame*);

  // Get the first/last frame in the current flow.
  virtual nsIFrame* GetFirstInFlow() const;
  virtual nsIFrame* GetLastInFlow() const;

  // Remove the frame from the flow. Connects the frame's prev-in-flow
  // and its next-in-flow
  static void RemoveFromFlow(nsIFrame* aFrame);
  
  // Detach from previous frame in flow
  static void BreakFromPrevFlow(nsIFrame* aFrame);

protected:
  nsSplittableFrame(nsStyleContext* aContext) : nsFrame(aContext) {}

#ifdef DEBUG
  virtual void DumpBaseRegressionData(nsPresContext* aPresContext, FILE* out, PRInt32 aIndent, PRBool aIncludeStyleData);
#endif

  nsIFrame*   mPrevContinuation;
  nsIFrame*   mNextContinuation;
};

#endif /* nsSplittableFrame_h___ */
