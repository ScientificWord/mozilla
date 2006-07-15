/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
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
 * the terms of any one of the NPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


#ifndef nsNativeScrollbar_h__
#define nsNativeScrollbar_h__

#include "nsChildView.h"

#include "nsINativeScrollbar.h"
#include "nsIContent.h"

#import "mozView.h"

class nsIScrollbarMediator;

@class NativeScrollbarView;

//
// nsNativeScrollbar
//
// A wrapper around a MacOS native scrollbar that knows how to work
// with a stub gecko frame to scroll in the GFXScrollFrame mechanism
//

class nsNativeScrollbar : public nsChildView, public nsINativeScrollbar
{
private:
  typedef nsChildView Inherited;

public:
                nsNativeScrollbar();
  virtual       ~nsNativeScrollbar();
  
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSINATIVESCROLLBAR

    // called from the NativeScrollbarView when someone hits a part
    // of the scrollbar.
  void DoScroll(NSScrollerPart inPart);
  
protected:

  // nsWindow Interface
  virtual PRBool DispatchMouseEvent(nsMouseEvent &aEvent);
  NS_IMETHOD Enable(PRBool aState);
  NS_IMETHOD IsEnabled(PRBool* outState);
  
  NSScroller* GetControl() { return mView; }

  void UpdateContentPosition(PRUint32 inNewPos);
  
  void RecreateHorizontalScrollbar();

  virtual NSView*   CreateCocoaView(NSRect inFrame);
  virtual GrafPtr   GetQuickDrawPort();

  void              UpdateScroller();
  
  NativeScrollbarView*    ScrollbarView() const { return (NativeScrollbarView*)mView; }
  
// DATA
private:

  nsIContent*       mContent;          // the content node that affects the scrollbar's value
  nsIScrollbarMediator* mMediator;     // for scrolling with outliners
  nsISupports*      mScrollbar;        // for calling into the mediator

  PRUint32          mValue;
  PRUint32          mMaxValue;
  PRUint32          mVisibleImageSize;
  PRUint32          mLineIncrement;
  PRBool            mIsEnabled;
};


@interface NativeScrollbarView : NSScroller<mozView>
{
    // Our window [WEAK]
  NSWindow* mWindow;
  
    // the nsNativeScrollbar that created this view. It retains this NSView, so
    // the link back to it must be weak. [WEAK]
  nsNativeScrollbar* mGeckoChild;

    // YES when we're in a tracking loop
  BOOL      mInTracking;
}

  // default initializer
- (id)initWithFrame:(NSRect)frameRect geckoChild:(nsNativeScrollbar*)inChild;
  // overridden parent class initializer
- (id)initWithFrame:(NSRect)frameRect;

- (IBAction)scroll:(NSScroller*)sender;

@end


#endif // nsNativeScrollbar_
