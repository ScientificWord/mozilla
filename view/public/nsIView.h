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

#ifndef nsIView_h___
#define nsIView_h___

#include "nsCoord.h"
#include "nsRect.h"
#include "nsPoint.h"
#include "nsIWidget.h"

class nsIViewManager;
class nsIScrollableView;
class nsViewManager;
class nsView;
class nsWeakView;

// Enumerated type to indicate the visibility of a layer.
// hide - the layer is not shown.
// show - the layer is shown irrespective of the visibility of 
//        the layer's parent.
enum nsViewVisibility {
  nsViewVisibility_kHide = 0,
  nsViewVisibility_kShow = 1
};

// IID for the nsIView interface
// 1377A30E-99E6-42FA-9A2E-EEEC6B31B7B6
#define NS_IVIEW_IID    \
{ 0x1377ae0e, 0x99e6, 0x42fa, \
{ 0x9a, 0x2e, 0xee, 0xec, 0x6b, 0x31, 0xb7, 0xb6 } }

// Public view flags are defined in this file
#define NS_VIEW_FLAGS_PUBLIC              0x00FF
// Private view flags are private to the view module,
// and are defined in nsView.h
#define NS_VIEW_FLAGS_PRIVATE             0xFF00

// Public view flags

// Indicates that the view is using auto z-indexing
#define NS_VIEW_FLAG_AUTO_ZINDEX          0x0004

// Indicates that the view is a floating view.
#define NS_VIEW_FLAG_FLOATING             0x0008

// If set it indicates that this view should be
// displayed above z-index:auto views if this view 
// is z-index:auto also
#define NS_VIEW_FLAG_TOPMOST              0x0010

// If set, the view disowns the widget and leaves it up
// to other code to destroy it.
#define NS_VIEW_DISOWNS_WIDGET             0x0020

struct nsViewZIndex {
  PRBool mIsAuto;
  PRInt32 mZIndex;
  PRBool mIsTopmost;
  
  nsViewZIndex(PRBool aIsAuto, PRInt32 aZIndex, PRBool aIsTopmost)
    : mIsAuto(aIsAuto), mZIndex(aZIndex), mIsTopmost(aIsTopmost) {}
};

//----------------------------------------------------------------------

/**
 * View interface
 *
 * Views are NOT reference counted. Use the Destroy() member function to
 * destroy a view.
 *
 * The lifetime of the view hierarchy is bounded by the lifetime of the
 * view manager that owns the views.
 *
 * Most of the methods here are read-only. To set the corresponding properties
 * of a view, go through nsIViewManager.
 */

class nsIView
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IVIEW_IID)

  /**
   * See if this view is scrollable.
   * @result an nsIScrollableView* if the view is scrollable, or nsnull if not.
   */
  virtual nsIScrollableView* ToScrollableView() { return nsnull; }

  /**
   * Find the view for the given widget, if there is one.
   * @return the view the widget belongs to, or null if the widget doesn't
   * belong to any view.
   */
  static nsIView* GetViewFor(nsIWidget* aWidget);

  /**
   * Get the view manager which "owns" the view.
   * This method might require some expensive traversal work in the future. If you can get the
   * view manager from somewhere else, do that instead.
   * @result the view manager
   */
  nsIViewManager* GetViewManager() const
  { return reinterpret_cast<nsIViewManager*>(mViewManager); }

  /**
   * Destroy the view.
   *
   * The view destroys its child views, and destroys and releases its
   * widget (if it has one).
   *
   * Also informs the view manager that the view is destroyed by calling
   * SetRootView(NULL) if the view is the root view and calling RemoveChild()
   * otherwise.
   */
  void Destroy();

  /**
   * Called to get the position of a view.
   * The specified coordinates are in the parent view's coordinate space.
   * This is the (0, 0) origin of the coordinate space established by this view.
   * @param x out parameter for x position
   * @param y out parameter for y position
   */
  nsPoint GetPosition() const {
    // Call ExternalIsRoot here so that we can get to it from other
    // components
    NS_ASSERTION(!ExternalIsRoot() || (mPosX == 0 && mPosY == 0),
                 "root views should always have explicit position of (0,0)");
    return nsPoint(mPosX, mPosY);
  }
  
  /**
   * Called to get the dimensions and position of the view's bounds.
   * The view's bounds (x,y) are in the coordinate space of the parent view.
   * The view's bounds (x,y) might not be the same as the view's position,
   * if the view has content above or to the left of its origin.
   * @param aBounds out parameter for bounds
   */
  nsRect GetBounds() const { return mDimBounds; }

  /**
   * Get the offset between the coordinate systems of |this| and aOther.
   * Adding the return value to a point in the coordinate system of |this|
   * will transform the point to the coordinate system of aOther.
   *
   * If aOther is null, this will return the offset of |this| from the
   * root of the viewmanager tree.
   * 
   * This function is fastest when aOther is an ancestor of |this|.
   *
   * NOTE: this actually returns the offset from aOther to |this|, but
   * that offset is added to transform _coordinates_ from |this| to aOther.
   */
  nsPoint GetOffsetTo(const nsIView* aOther) const;

  /**
   * Get the screen position of the view.
   * @return the pixel position of the top-left of the view in screen
   * coordinates.
   */
  nsIntPoint GetScreenPosition() const;
  
  /**
   * Called to query the visibility state of a view.
   * @result current visibility state
   */
  nsViewVisibility GetVisibility() const { return mVis; }

  /**
   * Called to query the z-index of a view.
   * The z-index is relative to all siblings of the view.
   * @result mZIndex: explicit z-index value or 0 if none is set
   *         mIsAuto: PR_TRUE if the view is zindex:auto
   *         mIsTopMost: used when this view is zindex:auto
   *                     PR_TRUE if the view is topmost when compared
   *                     with another z-index:auto view
   */
  nsViewZIndex GetZIndex() const { return nsViewZIndex((mVFlags & NS_VIEW_FLAG_AUTO_ZINDEX) != 0,
                                                       mZIndex,
                                                       (mVFlags & NS_VIEW_FLAG_TOPMOST) != 0); }

  /**
   * Get whether the view "floats" above all other views,
   * which tells the compositor not to consider higher views in
   * the view hierarchy that would geometrically intersect with
   * this view. This is a hack, but it fixes some problems with
   * views that need to be drawn in front of all other views.
   * @result PR_TRUE if the view floats, PR_FALSE otherwise.
   */
  PRBool GetFloating() const { return (mVFlags & NS_VIEW_FLAG_FLOATING) != 0; }

  /**
   * Called to query the parent of the view.
   * @result view's parent
   */
  nsIView* GetParent() const { return reinterpret_cast<nsIView*>(mParent); }

  /**
   * The view's first child is the child which is earliest in document order.
   * @result first child
   */
  nsIView* GetFirstChild() const { return reinterpret_cast<nsIView*>(mFirstChild); }

  /**
   * Called to query the next sibling of the view.
   * @result view's next sibling
   */
  nsIView* GetNextSibling() const { return reinterpret_cast<nsIView*>(mNextSibling); }

  /**
   * Set the view's link to client owned data.
   * @param aData - data to associate with view. nsnull to disassociate
   */
  void SetClientData(void *aData) { mClientData = aData; }

  /**
   * Query the view for it's link to client owned data.
   * @result data associated with view or nsnull if there is none.
   */
  void* GetClientData() const { return mClientData; }

  /**
   * Get the nearest widget in this view or a parent of this view and
   * the offset from the widget's origin to this view's origin
   * @param aOffset the offset from this view's origin to the widget's origin
   * (usually positive)
   * @return the widget closest to this view; can be null because some view trees
   * don't have widgets at all (e.g., printing), but if any view in the view tree
   * has a widget, then it's safe to assume this will not return null
   * XXX Remove this 'virtual' when gfx+widget are merged into gklayout;
   * Mac widget depends on this method, which is BOGUS!
   */
  virtual nsIWidget* GetNearestWidget(nsPoint* aOffset) const;

  /**
   * Create a widget to associate with this view.
   * @param aWindowIID IID for Widget type that this view
   *        should have associated with it. if nsull, then no
   *        width will be created for this view
   * @param aWidgetInitData data used to initialize this view's widget before
   *        its create is called.
   * @param aNative native window that will be used as parent of
   *        aWindowIID. if nsnull, then parent will be derived from
   *        parent view and it's ancestors
   * @param aWindowType is either content, UI or inherit from parent window.
   *        This is used to expose what type of window this is to 
   *        assistive technology like screen readers.
   * @param aParentWidget alternative parent to aNative used for popups. Must
   *        be null for non-popups.
   * @return error status
   */
  nsresult CreateWidget(const nsIID &aWindowIID,
                        nsWidgetInitData *aWidgetInitData = nsnull,
                        nsNativeWidget aNative = nsnull,
                        PRBool aEnableDragDrop = PR_TRUE,
                        PRBool aResetVisibility = PR_TRUE,
                        nsContentType aWindowType = eContentTypeInherit,
                        nsIWidget* aParentWidget = nsnull);

  /**
   * In 4.0, the "cutout" nature of a view is queryable.
   * If we believe that all cutout view have a native widget, this
   * could be a replacement.
   * @param aWidget out parameter for widget that this view contains,
   *        or nsnull if there is none.
   */
  nsIWidget* GetWidget() const { return mWindow; }

  /**
   * Returns PR_TRUE if the view has a widget associated with it.
   */
  PRBool HasWidget() const { return mWindow != nsnull; }

  /**
   * If called, will make the view disown the widget and leave it up
   * to other code to destroy it.
   */
  void DisownWidget() {
    mVFlags |= NS_VIEW_DISOWNS_WIDGET;
  }

#ifdef DEBUG
  /**
   * Output debug info to FILE
   * @param out output file handle
   * @param aIndent indentation depth
   * NOTE: virtual so that debugging tools not linked into gklayout can access it
   */
  virtual void List(FILE* out, PRInt32 aIndent = 0) const;
#endif // DEBUG

  /**
   * @result true iff this is the root view for its view manager
   */
  PRBool IsRoot() const;

  virtual PRBool ExternalIsRoot() const;

  void SetDeletionObserver(nsWeakView* aDeletionObserver);
protected:
  friend class nsWeakView;
  nsViewManager     *mViewManager;
  nsView            *mParent;
  nsIWidget         *mWindow;
  nsView            *mNextSibling;
  nsView            *mFirstChild;
  void              *mClientData;
  PRInt32           mZIndex;
  nsViewVisibility  mVis;
  nscoord           mPosX, mPosY;
  nsRect            mDimBounds; // relative to parent
  float             mOpacity;
  PRUint32          mVFlags;
  nsWeakView*       mDeletionObserver;

  virtual ~nsIView() {}
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIView, NS_IVIEW_IID)

// nsWeakViews must *not* be used in heap!
class nsWeakView
{
public:
  nsWeakView(nsIView* aView) : mPrev(nsnull), mView(aView)
  {
    if (mView) {
      mView->SetDeletionObserver(this);
    }
  }

  ~nsWeakView()
  {
    if (mView) {
      NS_ASSERTION(mView->mDeletionObserver == this,
                   "nsWeakViews deleted in wrong order!");
      // Clear deletion observer temporarily.
      mView->SetDeletionObserver(nsnull);
      // Put back the previous deletion observer.
      mView->SetDeletionObserver(mPrev);
    }
  }

  PRBool IsAlive() { return !!mView; }

  nsIView* GetView() { return mView; }

  void SetPrevious(nsWeakView* aWeakView) { mPrev = aWeakView; }

  void Clear()
  {
    if (mPrev) {
      mPrev->Clear();
    }
    mView = nsnull;
  }
private:
  static void* operator new(size_t) CPP_THROW_NEW { return 0; }
  static void operator delete(void*, size_t) {}
  nsWeakView* mPrev;
  nsIView*    mView;
};

#endif
