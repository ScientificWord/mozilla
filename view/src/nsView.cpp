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

#include "nsView.h"
#include "nsIWidget.h"
#include "nsViewManager.h"
#include "nsGUIEvent.h"
#include "nsIDeviceContext.h"
#include "nsIComponentManager.h"
#include "nsIScrollableView.h"
#include "nsGfxCIID.h"
#include "nsIRegion.h"
#include "nsIInterfaceRequestor.h"


//mmptemp

static nsEventStatus PR_CALLBACK HandleEvent(nsGUIEvent *aEvent);


//#define SHOW_VIEW_BORDERS
//#define HIDE_ALL_WIDGETS

// {34297A07-A8FD-d811-87C6-000244212BCB}
#define VIEW_WRAPPER_IID \
{ 0x34297a07, 0xa8fd, 0xd811, { 0x87, 0xc6, 0x0, 0x2, 0x44, 0x21, 0x2b, 0xcb } }


/**
 * nsISupports-derived helper class that allows to store and get a view
 */
class ViewWrapper : public nsIInterfaceRequestor
{
  public:
    NS_DECLARE_STATIC_IID_ACCESSOR(VIEW_WRAPPER_IID)
    NS_DECL_ISUPPORTS
    NS_DECL_NSIINTERFACEREQUESTOR

    ViewWrapper(nsView* aView) : mView(aView) {}

    nsView* GetView() { return mView; }
  private:
    nsView* mView;
};

NS_DEFINE_STATIC_IID_ACCESSOR(ViewWrapper, VIEW_WRAPPER_IID)

NS_IMPL_ADDREF(ViewWrapper)
NS_IMPL_RELEASE(ViewWrapper)
#ifndef DEBUG
NS_IMPL_QUERY_INTERFACE2(ViewWrapper, ViewWrapper, nsIInterfaceRequestor)

#else
NS_IMETHODIMP ViewWrapper::QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  NS_ENSURE_ARG_POINTER(aInstancePtr);

  NS_ASSERTION(!aIID.Equals(NS_GET_IID(nsIView)) &&
               !aIID.Equals(NS_GET_IID(nsIScrollableView)),
               "Someone expects a viewwrapper to be a view!");
  
  *aInstancePtr = nsnull;
  
  if (aIID.Equals(NS_GET_IID(nsISupports))) {
    *aInstancePtr = static_cast<nsISupports*>(this);
  }
  else if (aIID.Equals(NS_GET_IID(ViewWrapper))) {
    *aInstancePtr = this;
  }
  else if (aIID.Equals(NS_GET_IID(nsIInterfaceRequestor))) {
    *aInstancePtr = this;
  }


  if (*aInstancePtr) {
    AddRef();
    return NS_OK;
  }

  return NS_NOINTERFACE;
}
#endif

NS_IMETHODIMP ViewWrapper::GetInterface(REFNSIID aIID, void** aInstancePtr)
{
  if (aIID.Equals(NS_GET_IID(nsIScrollableView))) {
    *aInstancePtr = mView->ToScrollableView();
    return NS_OK;
  }
  if (aIID.Equals(NS_GET_IID(nsIView))) {
    *aInstancePtr = mView;
    return NS_OK;
  }
  return QueryInterface(aIID, aInstancePtr);
}

/**
 * Given a widget, returns the stored ViewWrapper on it, or NULL if no
 * ViewWrapper is there.
 */
static ViewWrapper* GetWrapperFor(nsIWidget* aWidget)
{
  // The widget's client data points back to the owning view
  if (aWidget) {
    void* clientData;
    aWidget->GetClientData(clientData);
    nsISupports* data = (nsISupports*)clientData;
    
    if (data) {
      ViewWrapper* wrapper;
      CallQueryInterface(data, &wrapper);
      // Give a weak reference to the caller. There will still be at least one
      // reference left, since the wrapper was addrefed when set on the widget.
      if (wrapper)
        wrapper->Release();
      return wrapper;
    }
  }
  return nsnull;
}

//
// Main events handler
//
nsEventStatus PR_CALLBACK HandleEvent(nsGUIEvent *aEvent)
{ 
//printf(" %d %d %d (%d,%d) \n", aEvent->widget, aEvent->widgetSupports, 
//       aEvent->message, aEvent->point.x, aEvent->point.y);
  nsEventStatus result = nsEventStatus_eIgnore;
  nsView       *view = nsView::GetViewFor(aEvent->widget);

  if (view)
  {
    view->GetViewManager()->DispatchEvent(aEvent, &result);
  }

  return result;
}

nsView::nsView(nsViewManager* aViewManager, nsViewVisibility aVisibility)
{
  MOZ_COUNT_CTOR(nsView);

  mVis = aVisibility;
  // Views should be transparent by default. Not being transparent is
  // a promise that the view will paint all its pixels opaquely. Views
  // should make this promise explicitly by calling
  // SetViewContentTransparency.
  mVFlags = 0;
  mViewManager = aViewManager;
  mDirtyRegion = nsnull;
  mDeletionObserver = nsnull;
}

void nsView::DropMouseGrabbing() {
  // check to see if we are grabbing events
  if (mViewManager->GetMouseEventGrabber() == this) {
    // we are grabbing events. Move the grab to the parent if we can.
    PRBool boolResult; //not used
    // if GetParent() returns null, then we release the grab, which is the best we can do
    mViewManager->GrabMouseEvents(GetParent(), boolResult);
  }
}

nsView::~nsView()
{
  MOZ_COUNT_DTOR(nsView);

  if (this == nsViewManager::GetViewFocusedBeforeSuppression()) {
#ifdef DEBUG_FOCUS_SUPPRESSION
    if (GetViewManager()->IsFocusSuppressed()) {
      printf("*** 0 INFO TODO [CPEARCE] destroying view focused before suppression, while suppressed\n");
    }
#endif
    nsViewManager::SetViewFocusedBeforeSuppression(nsnull);
  }
  if (this == nsViewManager::GetCurrentlyFocusedView()) {
#ifdef DEBUG_FOCUS_SUPPRESSION
    if (GetViewManager()->IsFocusSuppressed()) {
      printf("*** 0 INFO TODO [CPEARCE] destroying view currently focused, while suppressed\n");
    }
#endif
    nsViewManager::SetCurrentlyFocusedView(nsnull);
  }

  while (GetFirstChild())
  {
    nsView* child = GetFirstChild();
    if (child->GetViewManager() == mViewManager) {
      child->Destroy();
    } else {
      // just unhook it. Someone else will want to destroy this.
      RemoveChild(child);
    }
  }

  if (mViewManager)
  {
    DropMouseGrabbing();
  
    nsView *rootView = mViewManager->GetRootView();
    
    if (rootView)
    {
      // Root views can have parents!
      if (mParent)
      {
        mViewManager->RemoveChild(this);
      }

      if (rootView == this)
      {
        // Inform the view manager that the root view has gone away...
        mViewManager->SetRootView(nsnull);
      }
    }
    else if (mParent)
    {
      mParent->RemoveChild(this);
    }
    
    mViewManager = nsnull;
  }
  else if (mParent)
  {
    mParent->RemoveChild(this);
  }

  // Destroy and release the widget
  if (mWindow)
  {
    // Release memory for the view wrapper
    ViewWrapper* wrapper = GetWrapperFor(mWindow);
    NS_IF_RELEASE(wrapper);

    mWindow->SetClientData(nsnull);
    if (!(mVFlags & NS_VIEW_DISOWNS_WIDGET)) {
      mWindow->Destroy();
    }
    NS_RELEASE(mWindow);
  }
  delete mDirtyRegion;

  if (mDeletionObserver) {
    mDeletionObserver->Clear();
  }
}

nsresult nsView::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  if (nsnull == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }

  NS_ASSERTION(!aIID.Equals(NS_GET_IID(nsISupports)),
               "Someone expects views to be ISupports-derived!");
  
  *aInstancePtr = nsnull;
  
  if (aIID.Equals(NS_GET_IID(nsIView))) {
    *aInstancePtr = (void*)(nsIView*)this;
    return NS_OK;
  }

  return NS_NOINTERFACE;
}

nsIView* nsIView::GetViewFor(nsIWidget* aWidget)
{           
  NS_PRECONDITION(nsnull != aWidget, "null widget ptr");

  ViewWrapper* wrapper = GetWrapperFor(aWidget);
  if (wrapper)
    return wrapper->GetView();  
  return nsnull;
}

void nsIView::Destroy()
{
  delete this;
}

void nsView::SetPosition(nscoord aX, nscoord aY)
{
  mDimBounds.x += aX - mPosX;
  mDimBounds.y += aY - mPosY;
  mPosX = aX;
  mPosY = aY;

  NS_ASSERTION(GetParent() || (aX == 0 && aY == 0),
               "Don't try to move the root widget to something non-zero");

  ResetWidgetBounds(PR_TRUE, PR_TRUE, PR_FALSE);
}

void nsView::SetPositionIgnoringChildWidgets(nscoord aX, nscoord aY)
{
  mDimBounds.x += aX - mPosX;
  mDimBounds.y += aY - mPosY;
  mPosX = aX;
  mPosY = aY;

  ResetWidgetBounds(PR_FALSE, PR_TRUE, PR_FALSE);
}

void nsView::ResetWidgetBounds(PRBool aRecurse, PRBool aMoveOnly,
                               PRBool aInvalidateChangedSize) {
  if (mWindow) {
    // If our view manager has refresh disabled, then do nothing; the view
    // manager will set our position when refresh is reenabled.  Just let it
    // know that it has pending updates.
    if (!mViewManager->IsRefreshEnabled()) {
      mViewManager->PostPendingUpdate();
      return;
    }

    DoResetWidgetBounds(aMoveOnly, aInvalidateChangedSize);
  } else if (aRecurse) {
    // reposition any widgets under this view
    for (nsView* v = GetFirstChild(); v; v = v->GetNextSibling()) {
      v->ResetWidgetBounds(PR_TRUE, aMoveOnly, aInvalidateChangedSize);
    }
  }
}

nsRect nsView::CalcWidgetBounds(nsWindowType aType)
{
  nsCOMPtr<nsIDeviceContext> dx;
  mViewManager->GetDeviceContext(*getter_AddRefs(dx));
  NS_ASSERTION(dx, "View manager can't be created without a device context");
  PRInt32 p2a = dx->AppUnitsPerDevPixel();

  nsRect viewBounds(mDimBounds);

  if (GetParent()) {
    // put offset into screen coordinates
    nsPoint offset;
    nsIWidget* parentWidget = GetParent()->GetNearestWidget(&offset);
    viewBounds += offset;

    if (parentWidget && aType == eWindowType_popup &&
        mVis == nsViewVisibility_kShow) {
      nsRect screenRect(0,0,1,1);
      parentWidget->WidgetToScreen(screenRect, screenRect);
      viewBounds += nsPoint(NSIntPixelsToAppUnits(screenRect.x, p2a),
                            NSIntPixelsToAppUnits(screenRect.y, p2a));
    }
  }

  nsRect newBounds(viewBounds);
  newBounds.ScaleRoundPreservingCentersInverse(p2a);

  nsPoint roundedOffset(NSIntPixelsToAppUnits(newBounds.x, p2a),
                        NSIntPixelsToAppUnits(newBounds.y, p2a));
  mViewToWidgetOffset = viewBounds.TopLeft() - roundedOffset;

  return newBounds;
}

void nsView::DoResetWidgetBounds(PRBool aMoveOnly,
                                 PRBool aInvalidateChangedSize) {
  // The geometry of a root view's widget is controlled externally,
  // NOT by sizing or positioning the view
  if (mViewManager->GetRootView() == this) {
    return;
  }
  
  nsRect curBounds;
  mWindow->GetBounds(curBounds);
  nsWindowType type;
  mWindow->GetWindowType(type);

  if (curBounds.IsEmpty() && mDimBounds.IsEmpty() && type == eWindowType_popup) {
    // Don't manipulate empty popup widgets. For example there's no point
    // moving hidden comboboxes around, or doing X server roundtrips
    // to compute their true screen position. This could mean that WidgetToScreen
    // operations on these widgets don't return up-to-date values, but popup
    // positions aren't reliable anyway because of correction to be on or off-screen.
    return;
  }

  NS_PRECONDITION(mWindow, "Why was this called??");

  nsRect newBounds = CalcWidgetBounds(type);

  PRBool changedPos = curBounds.TopLeft() != newBounds.TopLeft();
  PRBool changedSize = curBounds.Size() != newBounds.Size();

  if (changedPos) {
    if (changedSize && !aMoveOnly) {
      mWindow->Resize(newBounds.x, newBounds.y, newBounds.width, newBounds.height,
                      aInvalidateChangedSize);
    } else {
      mWindow->Move(newBounds.x, newBounds.y);
    }
  } else {
    if (changedSize && !aMoveOnly) {
      mWindow->Resize(newBounds.width, newBounds.height, aInvalidateChangedSize);
    } // else do nothing!
  }
}

void nsView::SetDimensions(const nsRect& aRect, PRBool aPaint, PRBool aResizeWidget)
{
  nsRect dims = aRect;
  dims.MoveBy(mPosX, mPosY);

  // Don't use nsRect's operator== here, since it returns true when
  // both rects are empty even if they have different widths and we
  // have cases where that sort of thing matters to us.
  if (mDimBounds.TopLeft() == dims.TopLeft() &&
      mDimBounds.Size() == dims.Size()) {
    return;
  }

  mDimBounds = dims;

  if (aResizeWidget) {
    ResetWidgetBounds(PR_FALSE, PR_FALSE, aPaint);
  }
}

NS_IMETHODIMP nsView::SetVisibility(nsViewVisibility aVisibility)
{

  mVis = aVisibility;

  if (aVisibility == nsViewVisibility_kHide)
  {
    DropMouseGrabbing();
  }

  if (nsnull != mWindow)
  {
#ifndef HIDE_ALL_WIDGETS
    if (mVis == nsViewVisibility_kShow)
    {
      DoResetWidgetBounds(PR_FALSE, PR_TRUE);
      mWindow->Show(PR_TRUE);
    }
    else
#endif
      mWindow->Show(PR_FALSE);
  }

  return NS_OK;
}

NS_IMETHODIMP nsView::SetFloating(PRBool aFloatingView)
{
	if (aFloatingView)
		mVFlags |= NS_VIEW_FLAG_FLOATING;
	else
		mVFlags &= ~NS_VIEW_FLAG_FLOATING;

#if 0
	// recursively make all sub-views "floating" grr.
	for (nsView* child = mFirstChild; chlid; child = child->GetNextSibling()) {
		child->SetFloating(aFloatingView);
	}
#endif

	return NS_OK;
}

void nsView::InvalidateHierarchy(nsViewManager *aViewManagerParent)
{
  if (aViewManagerParent) {
    // We're removed from the view hierarchy of aRemovalPoint, so make sure
    // we're not still grabbing mouse events.
    if (aViewManagerParent->GetMouseEventGrabber() == this) {
      PRBool res;
      aViewManagerParent->GrabMouseEvents(nsnull, res);
    }
  }

  if (mViewManager->GetRootView() == this)
    mViewManager->InvalidateHierarchy();

  for (nsView *child = mFirstChild; child; child = child->GetNextSibling())
    child->InvalidateHierarchy(aViewManagerParent);
}

void nsView::InsertChild(nsView *aChild, nsView *aSibling)
{
  NS_PRECONDITION(nsnull != aChild, "null ptr");

  if (nsnull != aChild)
  {
    if (nsnull != aSibling)
    {
#ifdef NS_DEBUG
      NS_ASSERTION(aSibling->GetParent() == this, "tried to insert view with invalid sibling");
#endif
      //insert after sibling
      aChild->SetNextSibling(aSibling->GetNextSibling());
      aSibling->SetNextSibling(aChild);
    }
    else
    {
      aChild->SetNextSibling(mFirstChild);
      mFirstChild = aChild;
    }
    aChild->SetParent(this);

    // If we just inserted a root view, then update the RootViewManager
    // on all view managers in the new subtree.

    nsViewManager *vm = aChild->GetViewManager();
    if (vm->GetRootView() == aChild)
    {
      aChild->InvalidateHierarchy(nsnull); // don't care about releasing grabs
    }
  }
}

void nsView::RemoveChild(nsView *child)
{
  NS_PRECONDITION(nsnull != child, "null ptr");

  if (nsnull != child)
  {
    nsView* prevKid = nsnull;
    nsView* kid = mFirstChild;
    PRBool found = PR_FALSE;
    while (nsnull != kid) {
      if (kid == child) {
        if (nsnull != prevKid) {
          prevKid->SetNextSibling(kid->GetNextSibling());
        } else {
          mFirstChild = kid->GetNextSibling();
        }
        child->SetParent(nsnull);
        found = PR_TRUE;
        break;
      }
      prevKid = kid;
	    kid = kid->GetNextSibling();
    }
    NS_ASSERTION(found, "tried to remove non child");

    // If we just removed a root view, then update the RootViewManager
    // on all view managers in the removed subtree.

    nsViewManager *vm = child->GetViewManager();
    if (vm->GetRootView() == child)
    {
      child->InvalidateHierarchy(GetViewManager());
    }
  }
}

// Native widgets ultimately just can't deal with the awesome power of
// CSS2 z-index. However, we set the z-index on the widget anyway
// because in many simple common cases the widgets do end up in the
// right order. We set each widget's z-index to the z-index of the
// nearest ancestor that has non-auto z-index.
static void UpdateNativeWidgetZIndexes(nsView* aView, PRInt32 aZIndex)
{
  if (aView->HasWidget()) {
    nsIWidget* widget = aView->GetWidget();
    PRInt32 curZ;
    widget->GetZIndex(&curZ);
    if (curZ != aZIndex) {
      widget->SetZIndex(aZIndex);
    }
  } else {
    for (nsView* v = aView->GetFirstChild(); v; v = v->GetNextSibling()) {
      if (v->GetZIndexIsAuto()) {
        UpdateNativeWidgetZIndexes(v, aZIndex);
      }
    }
  }
}

static PRInt32 FindNonAutoZIndex(nsView* aView)
{
  while (aView) {
    if (!aView->GetZIndexIsAuto()) {
      return aView->GetZIndex();
    }
    aView = aView->GetParent();
  }
  return 0;
}

nsresult nsIView::CreateWidget(const nsIID &aWindowIID,
                               nsWidgetInitData *aWidgetInitData,
                               nsNativeWidget aNative,
                               PRBool aEnableDragDrop,
                               PRBool aResetVisibility,
                               nsContentType aContentType,
                               nsIWidget* aParentWidget)
{
  if (NS_UNLIKELY(mWindow)) {
    NS_ERROR("We already have a window for this view? BAD");
    ViewWrapper* wrapper = GetWrapperFor(mWindow);
    NS_IF_RELEASE(wrapper);
    mWindow->SetClientData(nsnull);
    NS_RELEASE(mWindow);
  }

  nsView* v = static_cast<nsView*>(this);

  nsRect trect = v->CalcWidgetBounds(aWidgetInitData
                                     ? aWidgetInitData->mWindowType
                                     : eWindowType_child);

  if (NS_OK == v->LoadWidget(aWindowIID))
  {
    PRBool usewidgets;
    nsCOMPtr<nsIDeviceContext> dx;
    mViewManager->GetDeviceContext(*getter_AddRefs(dx));
    dx->SupportsNativeWidgets(usewidgets);

    if (PR_TRUE == usewidgets)
    {
      PRBool initDataPassedIn = PR_TRUE;
      nsWidgetInitData initData;
      if (!aWidgetInitData) {
        // No initData, we're a child window
        // Create initData to pass in params
        initDataPassedIn = PR_FALSE;
        initData.clipChildren = PR_TRUE; // Clip child window's children
        initData.clipSiblings = PR_TRUE; // Clip child window's siblings
        aWidgetInitData = &initData;
      }
      aWidgetInitData->mContentType = aContentType;

      if (aNative && aWidgetInitData->mWindowType != eWindowType_popup)
        mWindow->Create(aNative, trect, ::HandleEvent, dx, nsnull, nsnull, aWidgetInitData);
      else
      {
        if (!initDataPassedIn && GetParent() && 
          GetParent()->GetViewManager() != mViewManager)
          initData.mListenForResizes = PR_TRUE;
        if (aParentWidget) {
          NS_ASSERTION(aWidgetInitData->mWindowType == eWindowType_popup,
                       "popup widget type expected");
          mWindow->Create(aParentWidget, trect,
                          ::HandleEvent, dx, nsnull, nsnull, aWidgetInitData);
        }
        else {
          nsIWidget* parentWidget = GetParent() ? GetParent()->GetNearestWidget(nsnull)
                                                : nsnull;
          if (aWidgetInitData->mWindowType == eWindowType_popup) {
            // Without a parent, we can't make a popup.  This can happen
            // when printing
            if (!parentWidget)
              return NS_ERROR_FAILURE;
            mWindow->Create(parentWidget->GetNativeData(NS_NATIVE_WIDGET), trect,
                            ::HandleEvent, dx, nsnull, nsnull, aWidgetInitData);
          } else {
            mWindow->Create(parentWidget, trect,
                            ::HandleEvent, dx, nsnull, nsnull, aWidgetInitData);
          }
        }
      }
      if (aEnableDragDrop) {
        mWindow->EnableDragDrop(PR_TRUE);
      }
      
      // propagate the z-index to the widget.
      UpdateNativeWidgetZIndexes(v, FindNonAutoZIndex(v));
    } else {
      // We should tell the widget its size even if we don't create a
      // native widget.  (At the moment, this doesn't really matter,
      // but we might want it to work at some point.)
      mWindow->Resize(trect.x, trect.y, trect.width, trect.height,
                      PR_FALSE);
    }
  }

  //make sure visibility state is accurate

  if (aResetVisibility) {
    v->SetVisibility(GetVisibility());
  }

  return NS_OK;
}

void nsView::SetZIndex(PRBool aAuto, PRInt32 aZIndex, PRBool aTopMost)
{
  PRBool oldIsAuto = GetZIndexIsAuto();
  mVFlags = (mVFlags & ~NS_VIEW_FLAG_AUTO_ZINDEX) | (aAuto ? NS_VIEW_FLAG_AUTO_ZINDEX : 0);
  mZIndex = aZIndex;
  SetTopMost(aTopMost);
  
  if (HasWidget() || !oldIsAuto || !aAuto) {
    UpdateNativeWidgetZIndexes(this, FindNonAutoZIndex(this));
  }
}

//
// internal window creation functions
//
nsresult nsView::LoadWidget(const nsCID &aClassIID)
{
  ViewWrapper* wrapper = new ViewWrapper(this);
  if (!wrapper)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(wrapper); // Will be released in ~nsView

  nsresult rv = CallCreateInstance(aClassIID, &mWindow);

  if (NS_SUCCEEDED(rv)) {
    // Set the widget's client data
    mWindow->SetClientData(wrapper);
  } else {
    delete wrapper;
  }

  return rv;
}

#ifdef DEBUG
void nsIView::List(FILE* out, PRInt32 aIndent) const
{
  PRInt32 i;
  for (i = aIndent; --i >= 0; ) fputs("  ", out);
  fprintf(out, "%p ", (void*)this);
  if (nsnull != mWindow) {
    nsRect windowBounds;
    nsRect nonclientBounds;
    float p2t;
    nsIDeviceContext *dx;
    mViewManager->GetDeviceContext(dx);
    p2t = (float) dx->AppUnitsPerDevPixel();
    NS_RELEASE(dx);
    mWindow->GetClientBounds(windowBounds);
    windowBounds *= p2t;
    mWindow->GetBounds(nonclientBounds);
    nonclientBounds *= p2t;
    nsrefcnt widgetRefCnt = mWindow->AddRef() - 1;
    mWindow->Release();
    PRInt32 Z;
    mWindow->GetZIndex(&Z);
    fprintf(out, "(widget=%p[%d] z=%d pos={%d,%d,%d,%d}) ",
            (void*)mWindow, widgetRefCnt, Z,
            nonclientBounds.x, nonclientBounds.y,
            windowBounds.width, windowBounds.height);
  }
  nsRect brect = GetBounds();
  fprintf(out, "{%d,%d,%d,%d}",
          brect.x, brect.y, brect.width, brect.height);
  const nsView* v = static_cast<const nsView*>(this);
  fprintf(out, " z=%d vis=%d clientData=%p <\n",
          mZIndex, mVis, mClientData);
  for (nsView* kid = mFirstChild; kid; kid = kid->GetNextSibling()) {
    NS_ASSERTION(kid->GetParent() == this, "incorrect parent");
    kid->List(out, aIndent + 1);
  }
  for (i = aIndent; --i >= 0; ) fputs("  ", out);
  fputs(">\n", out);
}
#endif // DEBUG

nsPoint nsIView::GetOffsetTo(const nsIView* aOther) const
{
  nsPoint offset(0, 0);
  const nsIView* v;
  for (v = this; v != aOther && v; v = v->GetParent()) {
    offset += v->GetPosition();
  }

  if (v != aOther) {
    // Looks like aOther wasn't an ancestor of |this|.  So now we have
    // the root-VM-relative position of |this| in |offset|.  Convert back
    // to the coordinates of aOther
    while (aOther) {
      offset -= aOther->GetPosition();
      aOther = aOther->GetParent();
    }
  }

  return offset;
}

nsIntPoint nsIView::GetScreenPosition() const
{
  nsIntRect screenRect(0,0,0,0);  
  nsPoint toWidgetOffset(0,0);
  nsIWidget* widget = GetNearestWidget(&toWidgetOffset);
  if (widget) {
    nsCOMPtr<nsIDeviceContext> dx;
    mViewManager->GetDeviceContext(*getter_AddRefs(dx));
    PRInt32 p2a = dx->AppUnitsPerDevPixel();
    nsIntRect ourRect(NSAppUnitsToIntPixels(toWidgetOffset.x, p2a),
                      NSAppUnitsToIntPixels(toWidgetOffset.y, p2a),
                      0,
                      0);
    widget->WidgetToScreen(ourRect, screenRect);
  }
  
  return nsIntPoint(screenRect.x, screenRect.y);
}

nsIWidget* nsIView::GetNearestWidget(nsPoint* aOffset) const
{
  nsPoint pt(0, 0);
  const nsView* v;
  for (v = static_cast<const nsView*>(this);
       v && !v->HasWidget(); v = v->GetParent()) {
    pt += v->GetPosition();
  }
  if (!v) {
    if (aOffset) {
      *aOffset = pt;
    }
    return static_cast<const nsView*>(this)->GetViewManager()->GetWidget();
  }

  // pt is now the offset from v's origin to this's origin
  // The widget's origin is the top left corner of v's bounds, which may
  // not coincide with v's origin
  if (aOffset) {
    nsRect vBounds = v->GetBounds();
    *aOffset = pt + v->GetPosition() -  nsPoint(vBounds.x, vBounds.y) +
               v->ViewToWidgetOffset();
  }
  return v->GetWidget();
}

PRBool nsIView::IsRoot() const
{
  NS_ASSERTION(mViewManager != nsnull," View manager is null in nsView::IsRoot()");
  return mViewManager->GetRootView() == this;
}

PRBool nsIView::ExternalIsRoot() const
{
  return nsIView::IsRoot();
}

void
nsIView::SetDeletionObserver(nsWeakView* aDeletionObserver)
{
  if (mDeletionObserver && aDeletionObserver) {
    aDeletionObserver->SetPrevious(mDeletionObserver);
  }
  mDeletionObserver = aDeletionObserver;
}
