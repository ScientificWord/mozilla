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
 *   Patrick C. Beard <beard@netscape.com>
 *   Kevin McCluskey  <kmcclusk@netscape.com>
 *   Robert O'Callahan <roc+@cs.cmu.edu>
 *   Roland Mainz <roland.mainz@informatik.med.uni-giessen.de>
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

#define PL_ARENA_CONST_ALIGN_MASK (sizeof(void*)-1)
#include "plarena.h"

#include "nsAutoPtr.h"
#include "nsViewManager.h"
#include "nsIRenderingContext.h"
#include "nsIDeviceContext.h"
#include "nsGfxCIID.h"
#include "nsIScrollableView.h"
#include "nsView.h"
#include "nsISupportsArray.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsGUIEvent.h"
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsRegion.h"
#include "nsInt64.h"
#include "nsScrollPortView.h"
#include "nsHashtable.h"
#include "nsCOMArray.h"
#include "nsThreadUtils.h"

#include "gfxContext.h"

static NS_DEFINE_IID(kBlenderCID, NS_BLENDER_CID);
static NS_DEFINE_IID(kRegionCID, NS_REGION_CID);
static NS_DEFINE_IID(kRenderingContextCID, NS_RENDERING_CONTEXT_CID);

/**
   XXX TODO XXX

   DeCOMify newly private methods
   Optimize view storage
*/

/**
   A note about platform assumptions:

   We assume all native widgets are opaque.
   
   We assume that a widget is z-ordered on top of its parent.
   
   We do NOT assume anything about the relative z-ordering of sibling widgets. Even though
   we ask for a specific z-order, we don't assume that widget z-ordering actually works.
*/

#define NSCOORD_NONE      PR_INT32_MIN

#ifdef NS_VM_PERF_METRICS
#include "nsITimeRecorder.h"
#endif

//-------------- Begin Invalidate Event Definition ------------------------

class nsInvalidateEvent : public nsViewManagerEvent {
public:
  nsInvalidateEvent(nsViewManager *vm) : nsViewManagerEvent(vm) {}

  NS_IMETHOD Run() {
    if (mViewManager)
      mViewManager->ProcessInvalidateEvent();
    return NS_OK;
  }
};

//-------------- End Invalidate Event Definition ---------------------------

static PRBool IsViewVisible(nsView *aView)
{
  for (nsIView *view = aView; view; view = view->GetParent()) {
    // We don't check widget visibility here because in the future (with
    // the better approach to this that's in attachment 160801 on bug
    // 227361), callers of the equivalent to this function should be able
    // to rely on being notified when the result of this function changes.
    if (view->GetVisibility() == nsViewVisibility_kHide)
      return PR_FALSE;
  }
  // Find out if the root view is visible by asking the view observer
  // (this won't be needed anymore if we link view trees across chrome /
  // content boundaries in DocumentViewerImpl::MakeWindow).
  nsIViewObserver* vo = aView->GetViewManager()->GetViewObserver();
  return vo && vo->IsVisible();
}

void
nsViewManager::PostInvalidateEvent()
{
  NS_ASSERTION(IsRootVM(), "Caller screwed up");

  if (!mInvalidateEvent.IsPending()) {
    nsRefPtr<nsViewManagerEvent> ev = new nsInvalidateEvent(this);
    if (NS_FAILED(NS_DispatchToCurrentThread(ev))) {
      NS_WARNING("failed to dispatch nsInvalidateEvent");
    } else {
      mInvalidateEvent = ev;
    }
  }
}

#undef DEBUG_MOUSE_LOCATION

PRInt32 nsViewManager::mVMCount = 0;
nsIRenderingContext* nsViewManager::gCleanupContext = nsnull;

// Weakly held references to all of the view managers
nsVoidArray* nsViewManager::gViewManagers = nsnull;
PRUint32 nsViewManager::gLastUserEventTime = 0;

nsViewManager::nsViewManager()
  : mMouseLocation(NSCOORD_NONE, NSCOORD_NONE)
  , mDelayedResize(NSCOORD_NONE, NSCOORD_NONE)
  , mRootViewManager(this)
{
  if (gViewManagers == nsnull) {
    NS_ASSERTION(mVMCount == 0, "View Manager count is incorrect");
    // Create an array to hold a list of view managers
    gViewManagers = new nsVoidArray;
  }
 
  if (gCleanupContext == nsnull) {
    /* XXX: This should use a device to create a matching |nsIRenderingContext| object */
    CallCreateInstance(kRenderingContextCID, &gCleanupContext);
    NS_ASSERTION(gCleanupContext,
                 "Wasn't able to create a graphics context for cleanup");
  }

  gViewManagers->AppendElement(this);

  mVMCount++;
  // NOTE:  we use a zeroing operator new, so all data members are
  // assumed to be cleared here.
  mDefaultBackgroundColor = NS_RGBA(0, 0, 0, 0);
  mHasPendingUpdates = PR_FALSE;
  mRecursiveRefreshPending = PR_FALSE;
  mUpdateBatchFlags = 0;
}

nsViewManager::~nsViewManager()
{
  if (mRootView) {
    // Destroy any remaining views
    mRootView->Destroy();
    mRootView = nsnull;
  }

  // Make sure to revoke pending events for all viewmanagers, since some events
  // are posted by a non-root viewmanager.
  mInvalidateEvent.Revoke();
  mSynthMouseMoveEvent.Revoke();
  
  if (!IsRootVM()) {
    // We have a strong ref to mRootViewManager
    NS_RELEASE(mRootViewManager);
  }

  mRootScrollable = nsnull;

  NS_ASSERTION((mVMCount > 0), "underflow of viewmanagers");
  --mVMCount;

#ifdef DEBUG
  PRBool removed =
#endif
    gViewManagers->RemoveElement(this);
  NS_ASSERTION(removed, "Viewmanager instance not was not in the global list of viewmanagers");

  if (0 == mVMCount) {
    // There aren't any more view managers so
    // release the global array of view managers
   
    NS_ASSERTION(gViewManagers != nsnull, "About to delete null gViewManagers");
    delete gViewManagers;
    gViewManagers = nsnull;

    // Cleanup all of the offscreen drawing surfaces if the last view manager
    // has been destroyed and there is something to cleanup

    // Note: A global rendering context is needed because it is not possible 
    // to create a nsIRenderingContext during the shutdown of XPCOM. The last
    // viewmanager is typically destroyed during XPCOM shutdown.
    NS_IF_RELEASE(gCleanupContext);
  }

  mObserver = nsnull;
  mContext = nsnull;
}

NS_IMPL_ISUPPORTS1(nsViewManager, nsIViewManager)

nsresult
nsViewManager::CreateRegion(nsIRegion* *result)
{
  nsresult rv;

  if (!mRegionFactory) {
    mRegionFactory = do_GetClassObject(kRegionCID, &rv);
    if (NS_FAILED(rv)) {
      *result = nsnull;
      return rv;
    }
  }

  nsIRegion* region = nsnull;
  rv = CallCreateInstance(mRegionFactory.get(), &region);
  if (NS_SUCCEEDED(rv)) {
    rv = region->Init();
    *result = region;
  }
  return rv;
}

// We don't hold a reference to the presentation context because it
// holds a reference to us.
NS_IMETHODIMP nsViewManager::Init(nsIDeviceContext* aContext)
{
  NS_PRECONDITION(nsnull != aContext, "null ptr");

  if (nsnull == aContext) {
    return NS_ERROR_NULL_POINTER;
  }
  if (nsnull != mContext) {
    return NS_ERROR_ALREADY_INITIALIZED;
  }
  mContext = aContext;

  mRefreshEnabled = PR_TRUE;

  mMouseGrabber = nsnull;

  return NS_OK;
}

NS_IMETHODIMP_(nsIView *)
nsViewManager::CreateView(const nsRect& aBounds,
                          const nsIView* aParent,
                          nsViewVisibility aVisibilityFlag)
{
  nsView *v = new nsView(this, aVisibilityFlag);
  if (v) {
    v->SetPosition(aBounds.x, aBounds.y);
    nsRect dim(0, 0, aBounds.width, aBounds.height);
    v->SetDimensions(dim, PR_FALSE);
    v->SetParent(static_cast<nsView*>(const_cast<nsIView*>(aParent)));
  }
  return v;
}

NS_IMETHODIMP_(nsIScrollableView *)
nsViewManager::CreateScrollableView(const nsRect& aBounds,
                                    const nsIView* aParent)
{
  nsScrollPortView *v = new nsScrollPortView(this);
  if (v) {
    v->SetPosition(aBounds.x, aBounds.y);
    nsRect dim(0, 0, aBounds.width, aBounds.height);
    v->SetDimensions(dim, PR_FALSE);
    v->SetParent(static_cast<nsView*>(const_cast<nsIView*>(aParent)));
  }
  return v;
}

NS_IMETHODIMP nsViewManager::GetRootView(nsIView *&aView)
{
  aView = mRootView;
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::SetRootView(nsIView *aView)
{
  nsView* view = static_cast<nsView*>(aView);

  NS_PRECONDITION(!view || view->GetViewManager() == this,
                  "Unexpected viewmanager on root view");
  
  // Do NOT destroy the current root view. It's the caller's responsibility
  // to destroy it
  mRootView = view;

  if (mRootView) {
    nsView* parent = mRootView->GetParent();
    if (parent) {
      // Calling InsertChild on |parent| will InvalidateHierarchy() on us, so
      // no need to set mRootViewManager ourselves here.
      parent->InsertChild(mRootView, nsnull);
    } else {
      InvalidateHierarchy();
    }

    mRootView->SetZIndex(PR_FALSE, 0, PR_FALSE);
  }
  // Else don't touch mRootViewManager

  return NS_OK;
}

NS_IMETHODIMP nsViewManager::GetWindowDimensions(nscoord *aWidth, nscoord *aHeight)
{
  if (nsnull != mRootView) {
    if (mDelayedResize == nsSize(NSCOORD_NONE, NSCOORD_NONE)) {
      nsRect dim;
      mRootView->GetDimensions(dim);
      *aWidth = dim.width;
      *aHeight = dim.height;
    } else {
      *aWidth = mDelayedResize.width;
      *aHeight = mDelayedResize.height;
    }
  }
  else
    {
      *aWidth = 0;
      *aHeight = 0;
    }
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::SetWindowDimensions(nscoord aWidth, nscoord aHeight)
{
  if (mRootView) {
    if (IsViewVisible(mRootView)) {
      mDelayedResize.SizeTo(NSCOORD_NONE, NSCOORD_NONE);
      DoSetWindowDimensions(aWidth, aHeight);
    } else {
      mDelayedResize.SizeTo(aWidth, aHeight);
    }
  }

  return NS_OK;
}

static void ConvertNativeRegionToAppRegion(nsIRegion* aIn, nsRegion* aOut,
                                           nsIDeviceContext* context)
{
  nsRegionRectSet* rects = nsnull;
  aIn->GetRects(&rects);
  if (!rects)
    return;

  PRInt32 p2a = context->AppUnitsPerDevPixel();

  PRUint32 i;
  for (i = 0; i < rects->mNumRects; i++) {
    const nsRegionRect& inR = rects->mRects[i];
    nsRect outR;
    outR.x = NSIntPixelsToAppUnits(inR.x, p2a);
    outR.y = NSIntPixelsToAppUnits(inR.y, p2a);
    outR.width = NSIntPixelsToAppUnits(inR.width, p2a);
    outR.height = NSIntPixelsToAppUnits(inR.height, p2a);
    aOut->Or(*aOut, outR);
  }

  aIn->FreeRects(rects);
}

static nsView* GetDisplayRootFor(nsView* aView)
{
  nsView *displayRoot = aView;
  for (;;) {
    nsView *displayParent = displayRoot->GetParent();
    if (!displayParent)
      return displayRoot;

    if (displayRoot->GetFloating() && !displayParent->GetFloating())
      return displayRoot;
    displayRoot = displayParent;
  }
}

/**
   aRegion is given in device coordinates!!
*/
void nsViewManager::Refresh(nsView *aView, nsIRenderingContext *aContext,
                            nsIRegion *aRegion, PRUint32 aUpdateFlags)
{
  NS_ASSERTION(aRegion != nsnull, "Null aRegion");

  if (! IsRefreshEnabled())
    return;

  nsRect viewRect;
  aView->GetDimensions(viewRect);

  // damageRegion is the damaged area, in twips, relative to the view origin
  nsRegion damageRegion;
  // convert pixels-relative-to-widget-origin to twips-relative-to-widget-origin
  ConvertNativeRegionToAppRegion(aRegion, &damageRegion, mContext);
  // move it from widget coordinates into view coordinates
  damageRegion.MoveBy(viewRect.x, viewRect.y);

  if (damageRegion.IsEmpty()) {
#ifdef DEBUG_roc
    nsRect damageRect = damageRegion.GetBounds();
    printf("XXX Damage rectangle (%d,%d,%d,%d) does not intersect the widget's view (%d,%d,%d,%d)!\n",
           damageRect.x, damageRect.y, damageRect.width, damageRect.height,
           viewRect.x, viewRect.y, viewRect.width, viewRect.height);
#endif
    return;
  }

#ifdef NS_VM_PERF_METRICS
  MOZ_TIMER_DEBUGLOG(("Reset nsViewManager::Refresh(region), this=%p\n", this));
  MOZ_TIMER_RESET(mWatch);

  MOZ_TIMER_DEBUGLOG(("Start: nsViewManager::Refresh(region)\n"));
  MOZ_TIMER_START(mWatch);
#endif

  NS_ASSERTION(!IsPainting(), "recursive painting not permitted");
  if (IsPainting()) {
    RootViewManager()->mRecursiveRefreshPending = PR_TRUE;
    return;
  }  
  SetPainting(PR_TRUE);

  nsCOMPtr<nsIRenderingContext> localcx;
  NS_ASSERTION(aView->GetWidget(),
               "Must have a widget to calculate coordinates correctly");
  if (nsnull == aContext)
    {
      localcx = CreateRenderingContext(*aView);

      //couldn't get rendering context. this is ok at init time atleast
      if (nsnull == localcx) {
        SetPainting(PR_FALSE);
        return;
      }
    } else {
      // plain assignment grabs another reference.
      localcx = aContext;
    }

  PRInt32 p2a = mContext->AppUnitsPerDevPixel();

  nsRefPtr<gfxContext> ctx = localcx->ThebesContext();

  ctx->Save();

  nsPoint vtowoffset = aView->ViewToWidgetOffset();
  ctx->Translate(gfxPoint(gfxFloat(vtowoffset.x) / p2a,
                          gfxFloat(vtowoffset.y) / p2a));

  ctx->Translate(gfxPoint(-gfxFloat(viewRect.x) / p2a,
                          -gfxFloat(viewRect.y) / p2a));

  nsRegion opaqueRegion;
  AddCoveringWidgetsToOpaqueRegion(opaqueRegion, mContext, aView);
  damageRegion.Sub(damageRegion, opaqueRegion);

  RenderViews(aView, *localcx, damageRegion);

  ctx->Restore();

  SetPainting(PR_FALSE);

  if (RootViewManager()->mRecursiveRefreshPending) {
    // Unset this flag first, since if aUpdateFlags includes NS_VMREFRESH_IMMEDIATE
    // we'll reenter this code from the UpdateAllViews call.
    RootViewManager()->mRecursiveRefreshPending = PR_FALSE;
    UpdateAllViews(aUpdateFlags);
  }

#ifdef NS_VM_PERF_METRICS
  MOZ_TIMER_DEBUGLOG(("Stop: nsViewManager::Refresh(region), this=%p\n", this));
  MOZ_TIMER_STOP(mWatch);
  MOZ_TIMER_LOG(("vm2 Paint time (this=%p): ", this));
  MOZ_TIMER_PRINT(mWatch);
#endif

}

void nsViewManager::DefaultRefresh(nsView* aView, nsIRenderingContext *aContext, const nsRect* aRect)
{
  NS_PRECONDITION(aView, "Must have a view to work with!");
  nsIWidget* widget = aView->GetNearestWidget(nsnull);
  if (! widget)
    return;

  nsCOMPtr<nsIRenderingContext> context = aContext;
  if (! aContext)
    context = CreateRenderingContext(*aView);

  if (! context)
    return;

  nscolor bgcolor = mDefaultBackgroundColor;

  if (NS_GET_A(mDefaultBackgroundColor) == 0) {
    NS_WARNING("nsViewManager: Asked to paint a default background, but no default background color is set!");
    return;
  }

  context->SetColor(bgcolor);
  context->FillRect(*aRect);
}

void nsViewManager::AddCoveringWidgetsToOpaqueRegion(nsRegion &aRgn, nsIDeviceContext* aContext,
                                                     nsView* aRootView) {
  NS_PRECONDITION(aRootView, "Must have root view");
  
  // We accumulate the bounds of widgets obscuring aRootView's widget into opaqueRgn.
  // In OptimizeDisplayList, display list elements which lie behind obscuring native
  // widgets are dropped.
  // This shouldn't really be necessary, since the GFX/Widget toolkit should remove these
  // covering widgets from the clip region passed into the paint command. But right now
  // they only give us a paint rect and not a region, so we can't access that information.
  // It's important to identifying areas that are covered by native widgets to avoid
  // painting their views many times as we process invalidates from the root widget all the
  // way down to the nested widgets.
  // 
  // NB: we must NOT add widgets that correspond to floating views!
  // We may be required to paint behind them
  aRgn.SetEmpty();

  nsIWidget* widget = aRootView->GetNearestWidget(nsnull);
  if (!widget) {
    return;
  }
  
  for (nsIWidget* childWidget = widget->GetFirstChild();
       childWidget;
       childWidget = childWidget->GetNextSibling()) {
    PRBool widgetVisible;
    childWidget->IsVisible(widgetVisible);
    if (widgetVisible) {
      nsView* view = nsView::GetViewFor(childWidget);
      if (view && view->GetVisibility() == nsViewVisibility_kShow
          && !view->GetFloating()) {
        nsRect bounds = view->GetBounds();
        if (bounds.width > 0 && bounds.height > 0) {
          nsView* viewParent = view->GetParent();
            
          while (viewParent && viewParent != aRootView) {
            viewParent->ConvertToParentCoords(&bounds.x, &bounds.y);
            viewParent = viewParent->GetParent();
          }
            
          // maybe we couldn't get the view into the coordinate
          // system of aRootView (maybe it's not a descendant
          // view of aRootView?); if so, don't use it
          if (viewParent) {
            aRgn.Or(aRgn, bounds);
          }
        }
      }
    }
  }
}

void nsViewManager::RenderViews(nsView *aView, nsIRenderingContext& aRC,
                                const nsRegion& aRegion)
{
  if (mObserver) {
    nsView* displayRoot = GetDisplayRootFor(aView);
    nsPoint offsetToRoot = aView->GetOffsetTo(displayRoot); 
    nsRegion damageRegion(aRegion);
    damageRegion.MoveBy(offsetToRoot);

    gfxContext* ctx = aRC.ThebesContext();
    nsCOMPtr<nsIDeviceContext> dc;
    aRC.GetDeviceContext(*getter_AddRefs(dc));
    double appPerDev = dc->AppUnitsPerDevPixel();
    gfxRect r(-offsetToRoot.x/appPerDev, -offsetToRoot.y/appPerDev, 1.0, 1.0);

    aRC.PushState();
    // Translate by the pixel-snapped offsetToRoot so that aRC's (0,0) will
    // be aligned to pixel boundaries. We use gfx pixel-snapping here to
    // ensure that the snapping we do here is consistent with other gfx
    // snapping. For example if someone drew a border around the outside
    // of aView, we want our (0,0) to be the inside top-left of that
    // border.
    if (ctx->UserToDevicePixelSnapped(r)) {
      ctx->Translate(ctx->DeviceToUser(r).pos);
    } else {
      aRC.Translate(-offsetToRoot.x, -offsetToRoot.y);
    }
    mObserver->Paint(displayRoot, &aRC, damageRegion);
    aRC.PopState();
  }
}

void nsViewManager::ProcessPendingUpdates(nsView* aView, PRBool aDoInvalidate)
{
  NS_ASSERTION(IsRootVM(), "Updates will be missed");

  // Protect against a null-view.
  if (!aView) {
    return;
  }

  if (aView->HasWidget()) {
    aView->ResetWidgetBounds(PR_FALSE, PR_FALSE, PR_TRUE);
  }

  // process pending updates in child view.
  for (nsView* childView = aView->GetFirstChild(); childView;
       childView = childView->GetNextSibling()) {
    ProcessPendingUpdates(childView, aDoInvalidate);
  }

  if (aDoInvalidate && aView->HasNonEmptyDirtyRegion()) {
    // Push out updates after we've processed the children; ensures that
    // damage is applied based on the final widget geometry
    NS_ASSERTION(mRefreshEnabled, "Cannot process pending updates with refresh disabled");
    nsRegion* dirtyRegion = aView->GetDirtyRegion();
    if (dirtyRegion) {
      UpdateWidgetArea(aView, *dirtyRegion, nsnull);
      dirtyRegion->SetEmpty();
    }
  }
}

NS_IMETHODIMP nsViewManager::Composite()
{
  if (!IsRootVM()) {
    return RootViewManager()->Composite();
  }
  
  if (UpdateCount() > 0)
    {
      ForceUpdate();
      ClearUpdateCount();
    }

  return NS_OK;
}

NS_IMETHODIMP nsViewManager::UpdateView(nsIView *aView, PRUint32 aUpdateFlags)
{
  // Mark the entire view as damaged
  nsView* view = static_cast<nsView*>(aView);

  nsRect bounds = view->GetBounds();
  view->ConvertFromParentCoords(&bounds.x, &bounds.y);
  return UpdateView(view, bounds, aUpdateFlags);
}

// This method accumulates the intersectons of all dirty regions attached to
// descendants of aSourceView with the cliprect of aTargetView into the dirty
// region of aTargetView, after offseting said intersections by aOffset.
static void
AccumulateIntersectionsIntoDirtyRegion(nsView* aTargetView,
                                       nsView* aSourceView,
                                       const nsPoint& aOffset)
{
  if (aSourceView->HasNonEmptyDirtyRegion()) {
    // In most cases, aSourceView is an ancestor of aTargetView, since most
    // commonly we have dirty rects on the root view.
    nsPoint offset = aTargetView->GetOffsetTo(aSourceView);
    nsRegion intersection;
    intersection = *aSourceView->GetDirtyRegion();
    if (!intersection.IsEmpty()) {
      nsRegion* targetRegion = aTargetView->GetDirtyRegion();
      if (targetRegion) {
        intersection.MoveBy(-offset + aOffset);
        targetRegion->Or(*targetRegion, intersection);
        // Random simplification number...
        targetRegion->SimplifyOutward(20);
      }
    }
  }

  if (aSourceView == aTargetView) {
    // No need to do this with kids of aTargetView
    return;
  }
  
  for (nsView* kid = aSourceView->GetFirstChild();
       kid;
       kid = kid->GetNextSibling()) {
    AccumulateIntersectionsIntoDirtyRegion(aTargetView, kid, aOffset);
  }
}

nsresult
nsViewManager::WillBitBlit(nsView* aView, nsPoint aScrollAmount)
{
  if (!IsRootVM()) {
    RootViewManager()->WillBitBlit(aView, aScrollAmount);
    return NS_OK;
  }

  NS_PRECONDITION(aView, "Must have a view");
  NS_PRECONDITION(aView->HasWidget(), "View must have a widget");

  ++mScrollCnt;
  
  // Since the view is actually moving the widget by -aScrollAmount, that's the
  // offset we want to use when accumulating dirty rects.
  AccumulateIntersectionsIntoDirtyRegion(aView, GetRootView(), -aScrollAmount);
  return NS_OK;
}

// Invalidate all widgets which overlap the view, other than the view's own widgets.
void
nsViewManager::UpdateViewAfterScroll(nsView *aView, const nsRegion& aUpdateRegion)
{
  NS_ASSERTION(RootViewManager()->mScrollCnt > 0,
               "Someone forgot to call WillBitBlit()");
  // Look at the view's clipped rect. It may be that part of the view is clipped out
  // in which case we don't need to worry about invalidating the clipped-out part.
  nsRect damageRect = aView->GetDimensions();
  if (damageRect.IsEmpty()) {
    // Don't forget to undo mScrollCnt!
    --RootViewManager()->mScrollCnt;
    return;
  }
  nsPoint offset = ComputeViewOffset(aView);
  damageRect.MoveBy(offset);

  // if this is a floating view, it isn't covered by any widgets other than
  // its children, which are handled by the widget scroller.
  if (aView->GetFloating()) {
    // Don't forget to undo mScrollCnt!
    --RootViewManager()->mScrollCnt;
    return;
  }

  UpdateWidgetArea(RootViewManager()->GetRootView(), nsRegion(damageRect), aView);
  if (!aUpdateRegion.IsEmpty()) {
    // XXX We should update the region, not the bounds rect, but that requires
    // a little more work. Fix this when we reshuffle this code.
    nsRegion update(aUpdateRegion);
    update.MoveBy(offset);
    UpdateWidgetArea(RootViewManager()->GetRootView(), update, nsnull);
    // FlushPendingInvalidates();
  }

  Composite();
  --RootViewManager()->mScrollCnt;
}

/**
 * @param aDamagedRegion this region, relative to aWidgetView, is invalidated in
 * every widget child of aWidgetView, plus aWidgetView's own widget
 * @param aIgnoreWidgetView if non-null, the aIgnoreWidgetView's widget and its
 * children are not updated.
 */
void
nsViewManager::UpdateWidgetArea(nsView *aWidgetView, const nsRegion &aDamagedRegion,
                                nsView* aIgnoreWidgetView)
{
  if (!IsRefreshEnabled()) {
    // accumulate this rectangle in the view's dirty region, so we can
    // process it later.
    nsRegion* dirtyRegion = aWidgetView->GetDirtyRegion();
    if (!dirtyRegion) return;

    dirtyRegion->Or(*dirtyRegion, aDamagedRegion);
    // Don't let dirtyRegion grow beyond 8 rects
    dirtyRegion->SimplifyOutward(8);
    nsViewManager* rootVM = RootViewManager();
    rootVM->mHasPendingUpdates = PR_TRUE;
    rootVM->IncrementUpdateCount();
    return;
    // this should only happen at the top level, and this result
    // should not be consumed by top-level callers, so it doesn't
    // really matter what we return
  }

  // If the bounds don't overlap at all, there's nothing to do
  nsRegion intersection;
  intersection.And(aWidgetView->GetDimensions(), aDamagedRegion);
  if (intersection.IsEmpty()) {
    return;
  }

  // If the widget is hidden, it don't cover nothing
  if (nsViewVisibility_kHide == aWidgetView->GetVisibility()) {
#ifdef DEBUG
    // Assert if view is hidden but widget is visible
    nsIWidget* widget = aWidgetView->GetNearestWidget(nsnull);
    if (widget) {
      PRBool visible;
      widget->IsVisible(visible);
      NS_ASSERTION(!visible, "View is hidden but widget is visible!");
    }
#endif
    return;
  }

  if (aWidgetView == aIgnoreWidgetView) {
    // the widget for aIgnoreWidgetView (and its children) should be treated as already updated.
    return;
  }

  nsIWidget* widget = aWidgetView->GetNearestWidget(nsnull);
  if (!widget) {
    // The root view or a scrolling view might not have a widget
    // (for example, during printing). We get here when we scroll
    // during printing to show selected options in a listbox, for example.
    return;
  }

  // Update all child widgets with the damage. In the process,
  // accumulate the union of all the child widget areas, or at least
  // some subset of that.
  nsRegion children;
  for (nsIWidget* childWidget = widget->GetFirstChild();
       childWidget;
       childWidget = childWidget->GetNextSibling()) {
    nsView* view = nsView::GetViewFor(childWidget);
    NS_ASSERTION(view != aWidgetView, "will recur infinitely");
    if (view && view->GetVisibility() == nsViewVisibility_kShow) {
      // Don't mess with views that are in completely different view
      // manager trees
      if (view->GetViewManager()->RootViewManager() == RootViewManager()) {
        // get the damage region into 'view's coordinate system
        nsRegion damage = intersection;
        nsPoint offset = view->GetOffsetTo(aWidgetView);
        damage.MoveBy(-offset);
        UpdateWidgetArea(view, damage, aIgnoreWidgetView);
        children.Or(children, view->GetDimensions() + offset);
        children.SimplifyInward(20);
      }
    }
  }

  nsRegion leftOver;
  leftOver.Sub(intersection, children);

  if (!leftOver.IsEmpty()) {
    NS_ASSERTION(IsRefreshEnabled(), "Can only get here with refresh enabled, I hope");

    const nsRect* r;
    for (nsRegionRectIterator iter(leftOver); (r = iter.Next());) {
      nsRect bounds = *r;
      ViewToWidget(aWidgetView, aWidgetView, bounds);
      widget->Invalidate(bounds, PR_FALSE);
    }
  }
}

NS_IMETHODIMP nsViewManager::UpdateView(nsIView *aView, const nsRect &aRect, PRUint32 aUpdateFlags)
{
  NS_PRECONDITION(nsnull != aView, "null view");

  nsView* view = static_cast<nsView*>(aView);

  nsRect damagedRect(aRect);

   // If the rectangle is not visible then abort
   // without invalidating. This is a performance 
   // enhancement since invalidating a native widget
   // can be expensive.
   // This also checks for silly request like damagedRect.width = 0 or damagedRect.height = 0
  nsRectVisibility rectVisibility;
  GetRectVisibility(view, damagedRect, 0, &rectVisibility);
  if (rectVisibility != nsRectVisibility_kVisible) {
    return NS_OK;
  }

  // if this is a floating view, it isn't covered by any widgets other than
  // its children. In that case we walk up to its parent widget and use
  // that as the root to update from. This also means we update areas that
  // may be outside the parent view(s), which is necessary for floats.
  if (view->GetFloating()) {
    nsView* widgetParent = view;

    while (!widgetParent->HasWidget()) {
      widgetParent->ConvertToParentCoords(&damagedRect.x, &damagedRect.y);
      widgetParent = widgetParent->GetParent();
    }

    UpdateWidgetArea(widgetParent, nsRegion(damagedRect), nsnull);
  } else {
    // Propagate the update to the root widget of the root view manager, since
    // iframes, for example, can overlap each other and be translucent.  So we
    // have to possibly invalidate our rect in each of the widgets we have
    // lying about.
    damagedRect.MoveBy(ComputeViewOffset(view));

    UpdateWidgetArea(RootViewManager()->GetRootView(), nsRegion(damagedRect), nsnull);
  }

  RootViewManager()->IncrementUpdateCount();

  if (!IsRefreshEnabled()) {
    return NS_OK;
  }

  // See if we should do an immediate refresh or wait
  if (aUpdateFlags & NS_VMREFRESH_IMMEDIATE) {
    Composite();
  } 

  return NS_OK;
}

NS_IMETHODIMP nsViewManager::UpdateAllViews(PRUint32 aUpdateFlags)
{
  if (RootViewManager() != this) {
    return RootViewManager()->UpdateAllViews(aUpdateFlags);
  }
  
  UpdateViews(mRootView, aUpdateFlags);
  return NS_OK;
}

void nsViewManager::UpdateViews(nsView *aView, PRUint32 aUpdateFlags)
{
  // update this view.
  UpdateView(aView, aUpdateFlags);

  // update all children as well.
  nsView* childView = aView->GetFirstChild();
  while (nsnull != childView)  {
    UpdateViews(childView, aUpdateFlags);
    childView = childView->GetNextSibling();
  }
}

nsView *nsViewManager::sCurrentlyFocusView = nsnull;
nsView *nsViewManager::sViewFocusedBeforeSuppression = nsnull;
PRInt32 nsViewManager::sSuppressCount = 0;

void nsViewManager::SuppressFocusEvents()
{
  sSuppressCount++;
  if (sSuppressCount == 1) {
    // We're turning on focus/blur suppression, remember what had
    // the focus.
    SetViewFocusedBeforeSuppression(GetCurrentlyFocusedView());
  }  
}

void nsViewManager::UnsuppressFocusEvents()
{
  sSuppressCount--;
  if (sSuppressCount > 0 ||
      GetCurrentlyFocusedView() == GetViewFocusedBeforeSuppression())
  return;
  
  // We're turning off suppression, synthesize LOSTFOCUS/GOTFOCUS.
  nsIWidget *widget = nsnull;
  nsEventStatus status;

  // Backup what is focused before we send the blur. If the
  // blur causes a focus change, keep that new focus change,
  // don't overwrite with the old "currently focused view".
  nsIView *currentFocusBeforeBlur = GetCurrentlyFocusedView();

  // Send NS_LOSTFOCUS to widget that was focused before
  // focus/blur suppression.
  if (GetViewFocusedBeforeSuppression()) {
    widget = GetViewFocusedBeforeSuppression()->GetWidget();
    if (widget) {
#ifdef DEBUG_FOCUS_SUPPRESSION
      printf("*** 0 INFO TODO [CPEARCE] Unsuppressing, dispatching NS_LOSTFOCUS\n");
#endif
      nsGUIEvent event(PR_TRUE, NS_LOSTFOCUS, widget);
      widget->DispatchEvent(&event, status);
    }
  }

  // Send NS_GOTFOCUS to the widget that we think should be focused.
  if (GetCurrentlyFocusedView() &&
      currentFocusBeforeBlur == GetCurrentlyFocusedView())
  {
    widget = GetCurrentlyFocusedView()->GetWidget();
    if (widget) {
#ifdef DEBUG_FOCUS_SUPPRESSION
      printf("*** 0 INFO TODO [CPEARCE] Unsuppressing, dispatching NS_GOTFOCUS\n");
#endif
      nsGUIEvent event(PR_TRUE, NS_GOTFOCUS, widget);
      widget->DispatchEvent(&event, status); 
    }
  }

}

static void ConvertRectAppUnitsToIntPixels(nsRect& aRect, PRInt32 p2a)
{
  aRect.x = NSAppUnitsToIntPixels(aRect.x, p2a);
  aRect.y = NSAppUnitsToIntPixels(aRect.y, p2a);
  aRect.width = NSAppUnitsToIntPixels(aRect.width, p2a);
  aRect.height = NSAppUnitsToIntPixels(aRect.height, p2a);
}

NS_IMETHODIMP nsViewManager::DispatchEvent(nsGUIEvent *aEvent, nsEventStatus *aStatus)
{
  *aStatus = nsEventStatus_eIgnore;

  switch(aEvent->message)
    {
    case NS_SIZE:
      {
        nsView* view = nsView::GetViewFor(aEvent->widget);

        if (nsnull != view)
          {
            nscoord width = ((nsSizeEvent*)aEvent)->windowSize->width;
            nscoord height = ((nsSizeEvent*)aEvent)->windowSize->height;
            width = ((nsSizeEvent*)aEvent)->mWinWidth;
            height = ((nsSizeEvent*)aEvent)->mWinHeight;

            // The root view may not be set if this is the resize associated with
            // window creation

            if (view == mRootView)
              {
                PRInt32 p2a = mContext->AppUnitsPerDevPixel();
                SetWindowDimensions(NSIntPixelsToAppUnits(width, p2a),
                                    NSIntPixelsToAppUnits(height, p2a));
                *aStatus = nsEventStatus_eConsumeNoDefault;
              }
          }

        break;
      }

    case NS_PAINT:
      {
        nsPaintEvent *event = static_cast<nsPaintEvent*>(aEvent);
        nsView *view = nsView::GetViewFor(aEvent->widget);

        if (!view || !mContext)
          break;

        *aStatus = nsEventStatus_eConsumeNoDefault;

        // The rect is in device units, and it's in the coordinate space of its
        // associated window.
        nsCOMPtr<nsIRegion> region = event->region;
        if (!region) {
          if (NS_FAILED(CreateRegion(getter_AddRefs(region))))
            break;

          const nsRect& damrect = *event->rect;
          region->SetTo(damrect.x, damrect.y, damrect.width, damrect.height);
        }
        
        if (region->IsEmpty())
          break;

        // Refresh the view
        if (IsRefreshEnabled()) {
          // If an ancestor widget was hidden and then shown, we could
          // have a delayed resize to handle.
          PRBool didResize = PR_FALSE;
          for (nsViewManager *vm = this; vm;
               vm = vm->mRootView->GetParent()
                      ? vm->mRootView->GetParent()->GetViewManager()
                      : nsnull) {
            if (vm->mDelayedResize != nsSize(NSCOORD_NONE, NSCOORD_NONE) &&
                IsViewVisible(vm->mRootView)) {
              vm->DoSetWindowDimensions(vm->mDelayedResize.width,
                                        vm->mDelayedResize.height);
              vm->mDelayedResize.SizeTo(NSCOORD_NONE, NSCOORD_NONE);

              // Paint later.
              vm->UpdateView(vm->mRootView, NS_VMREFRESH_NO_SYNC);
              didResize = PR_TRUE;

              // not sure if it's valid for us to claim that we
              // ignored this, but we're going to do so anyway, since
              // we didn't actually paint anything
              *aStatus = nsEventStatus_eIgnore;
            }
          }

          if (!didResize) {
            //NS_ASSERTION(IsViewVisible(view), "painting an invisible view");

            // Just notify our own view observer that we're about to paint
            // XXXbz do we need to notify other view observers for viewmanagers
            // in our tree?
            // Make sure to not send WillPaint notifications while scrolling
            nsRefPtr<nsViewManager> rootVM = RootViewManager();

            nsIWidget *widget = mRootView->GetWidget();
            PRBool transparentWindow = PR_FALSE;
            if (widget)
                widget->GetHasTransparentBackground(transparentWindow);

            if (rootVM->mScrollCnt == 0 && !transparentWindow) {
              nsIViewObserver* observer = GetViewObserver();
              if (observer) {
                // Do an update view batch.  Make sure not to do it DEFERRED,
                // since that would effectively delay any invalidates that are
                // triggered by the WillPaint notification (they'd happen when
                // the invalid event fires, which is later than the reflow
                // event would fire and could end up being after some timer
                // events, leading to frame dropping in DHTML).  Note that the
                // observer may try to reenter this code from inside
                // WillPaint() by trying to do a synchronous paint, but since
                // refresh will be disabled it won't be able to do the paint.
                // We should really sort out the rules on our synch painting
                // api....
                UpdateViewBatch batch(this);
                observer->WillPaint();
                batch.EndUpdateViewBatch(NS_VMREFRESH_NO_SYNC);

                // Get the view pointer again since the code above might have
                // destroyed it (bug 378273).
                view = nsView::GetViewFor(aEvent->widget);
              }
            }
            // Make sure to sync up any widget geometry changes we
            // have pending before we paint.
            if (rootVM->mHasPendingUpdates) {
              rootVM->ProcessPendingUpdates(mRootView, PR_FALSE);
            }
            
            if (view) {
              Refresh(view, event->renderingContext, region,
                      NS_VMREFRESH_DOUBLE_BUFFER);
            }
          }
        } else {
          // since we got an NS_PAINT event, we need to
          // draw something so we don't get blank areas.
          nsRect damRect;
          region->GetBoundingBox(&damRect.x, &damRect.y, &damRect.width, &damRect.height);
          PRInt32 p2a = mContext->AppUnitsPerDevPixel();
          damRect.ScaleRoundOut(float(p2a));
          DefaultRefresh(view, event->renderingContext, &damRect);
        
          // Clients like the editor can trigger multiple
          // reflows during what the user perceives as a single
          // edit operation, so it disables view manager
          // refreshing until the edit operation is complete
          // so that users don't see the intermediate steps.
          // 
          // Unfortunately some of these reflows can trigger
          // nsScrollPortView and nsScrollingView Scroll() calls
          // which in most cases force an immediate BitBlt and
          // synchronous paint to happen even if the view manager's
          // refresh is disabled. (Bug 97674)
          //
          // Calling UpdateView() here, is necessary to add
          // the exposed region specified in the synchronous paint
          // event to  the view's damaged region so that it gets
          // painted properly when refresh is enabled.
          //
          // Note that calling UpdateView() here was deemed
          // to have the least impact on performance, since the
          // other alternative was to make Scroll() post an
          // async paint event for the *entire* ScrollPort or
          // ScrollingView's viewable area. (See bug 97674 for this
          // alternate patch.)
          
          UpdateView(view, damRect, NS_VMREFRESH_NO_SYNC);
        }

        break;
      }

    case NS_CREATE:
    case NS_DESTROY:
    case NS_SETZLEVEL:
    case NS_MOVE:
      /* Don't pass these events through. Passing them through
         causes performance problems on pages with lots of views/frames 
         @see bug 112861 */
      *aStatus = nsEventStatus_eConsumeNoDefault;
      break;


    case NS_DISPLAYCHANGED:

      //Destroy the cached backbuffer to force a new backbuffer
      //be constructed with the appropriate display depth.
      //@see bugzilla bug 6061
      *aStatus = nsEventStatus_eConsumeDoDefault;
      break;

    case NS_SYSCOLORCHANGED:
      {
        // Hold a refcount to the observer. The continued existence of the observer will
        // delay deletion of this view hierarchy should the event want to cause its
        // destruction in, say, some JavaScript event handler.
        nsView *view = nsView::GetViewFor(aEvent->widget);
        nsCOMPtr<nsIViewObserver> obs = GetViewObserver();
        if (obs) {
          obs->HandleEvent(view, aEvent, aStatus);
        }
      }
      break; 

    default:
      {
        if (aEvent->message == NS_GOTFOCUS) {
#ifdef DEBUG_FOCUS_SUPPRESSION
          printf("*** 0 INFO TODO [CPEARCE] Focus changing%s\n",
            (nsViewManager::IsFocusSuppressed() ? " while suppressed" : ""));
#endif
          SetCurrentlyFocusedView(nsView::GetViewFor(aEvent->widget));
        }
        if ((aEvent->message == NS_GOTFOCUS || aEvent->message == NS_LOSTFOCUS) &&
             nsViewManager::IsFocusSuppressed())
        {
#ifdef DEBUG_FOCUS_SUPPRESSION
          printf("*** 0 INFO TODO [CPEARCE] Suppressing %s\n",
            (aEvent->message == NS_GOTFOCUS ? "NS_GOTFOCUS" : "NS_LOSTFOCUS"));
#endif          
          break;
        }
        
        if ((NS_IS_MOUSE_EVENT(aEvent) &&
             // Ignore moves that we synthesize.
             static_cast<nsMouseEvent*>(aEvent)->reason ==
               nsMouseEvent::eReal &&
             // Ignore mouse exit and enter (we'll get moves if the user
             // is really moving the mouse) since we get them when we
             // create and destroy widgets.
             aEvent->message != NS_MOUSE_EXIT &&
             aEvent->message != NS_MOUSE_ENTER) ||
            NS_IS_KEY_EVENT(aEvent) ||
            NS_IS_IME_EVENT(aEvent)) {
          gLastUserEventTime = PR_IntervalToMicroseconds(PR_IntervalNow());
        }

        if (aEvent->message == NS_DEACTIVATE) {
          PRBool result;
          GrabMouseEvents(nsnull, result);
        }

        //Find the view whose coordinates system we're in.
        nsView* baseView = nsView::GetViewFor(aEvent->widget);
        nsView* view = baseView;
        PRBool capturedEvent = PR_FALSE;
        
        if (!NS_IS_KEY_EVENT(aEvent) && !NS_IS_IME_EVENT(aEvent) &&
            !NS_IS_CONTEXT_MENU_KEY(aEvent) && !NS_IS_FOCUS_EVENT(aEvent) &&
            !NS_IS_QUERY_CONTENT_EVENT(aEvent) &&
             aEvent->eventStructType != NS_ACCESSIBLE_EVENT) {
          // will dispatch using coordinates. Pretty bogus but it's consistent
          // with what presshell does.
          view = GetDisplayRootFor(baseView);
        }

        //Find the view to which we're initially going to send the event 
        //for hittesting.
        if (NS_IS_MOUSE_EVENT(aEvent) || NS_IS_DRAG_EVENT(aEvent)) {
          nsView* mouseGrabber = GetMouseEventGrabber();
          if (mouseGrabber) {
            view = mouseGrabber;
            capturedEvent = PR_TRUE;
          }
        }

        if (nsnull != view) {
          PRInt32 p2a = mContext->AppUnitsPerDevPixel();

          if ((aEvent->message == NS_MOUSE_MOVE &&
               static_cast<nsMouseEvent*>(aEvent)->reason ==
                 nsMouseEvent::eReal) ||
              aEvent->message == NS_MOUSE_ENTER) {
            // aEvent->point is relative to the widget, i.e. the view top-left,
            // so we need to add the offset to the view origin
            nsPoint rootOffset = baseView->GetDimensions().TopLeft();
            rootOffset += baseView->GetOffsetTo(RootViewManager()->mRootView);
            RootViewManager()->mMouseLocation = aEvent->refPoint +
                nsPoint(NSAppUnitsToIntPixels(rootOffset.x, p2a),
                        NSAppUnitsToIntPixels(rootOffset.y, p2a));
#ifdef DEBUG_MOUSE_LOCATION
            if (aEvent->message == NS_MOUSE_ENTER)
              printf("[vm=%p]got mouse enter for %p\n",
                     this, aEvent->widget);
            printf("[vm=%p]setting mouse location to (%d,%d)\n",
                   this, mMouseLocation.x, mMouseLocation.y);
#endif
            if (aEvent->message == NS_MOUSE_ENTER)
              SynthesizeMouseMove(PR_FALSE);
          } else if (aEvent->message == NS_MOUSE_EXIT) {
            // Although we only care about the mouse moving into an area
            // for which this view manager doesn't receive mouse move
            // events, we don't check which view the mouse exit was for
            // since this seems to vary by platform.  Hopefully this
            // won't matter at all since we'll get the mouse move or
            // enter after the mouse exit when the mouse moves from one
            // of our widgets into another.
            RootViewManager()->mMouseLocation = nsPoint(NSCOORD_NONE, NSCOORD_NONE);
#ifdef DEBUG_MOUSE_LOCATION
            printf("[vm=%p]got mouse exit for %p\n",
                   this, aEvent->widget);
            printf("[vm=%p]clearing mouse location\n",
                   this);
#endif
          }

          //Calculate the proper offset for the view we're going to
          nsPoint offset(0, 0);

          if (view != baseView) {
            //Get offset from root of baseView
            nsView *parent;
            for (parent = baseView; parent; parent = parent->GetParent())
              parent->ConvertToParentCoords(&offset.x, &offset.y);

            //Subtract back offset from root of view
            for (parent = view; parent; parent = parent->GetParent())
              parent->ConvertFromParentCoords(&offset.x, &offset.y);
          }

          // Dispatch the event
          nsRect baseViewDimensions;
          if (baseView != nsnull) {
            baseView->GetDimensions(baseViewDimensions);
          }

          nsPoint pt;
          pt.x = baseViewDimensions.x + 
            NSFloatPixelsToAppUnits(float(aEvent->refPoint.x) + 0.5f, p2a);
          pt.y = baseViewDimensions.y + 
            NSFloatPixelsToAppUnits(float(aEvent->refPoint.y) + 0.5f, p2a);
          pt += offset;

          *aStatus = HandleEvent(view, pt, aEvent, capturedEvent);

          //
          // need to map the reply back into platform coordinates
          //
          switch (aEvent->message) {
            case NS_TEXT_TEXT:
              ConvertRectAppUnitsToIntPixels(
                ((nsTextEvent*)aEvent)->theReply.mCursorPosition, p2a);
              break;
            case NS_COMPOSITION_START:
            case NS_COMPOSITION_QUERY:
              ConvertRectAppUnitsToIntPixels(
                ((nsCompositionEvent*)aEvent)->theReply.mCursorPosition, p2a);
              break;
            case NS_QUERYCARETRECT:
              ConvertRectAppUnitsToIntPixels(
                ((nsQueryCaretRectEvent*)aEvent)->theReply.mCaretRect, p2a);
              break;
            case NS_QUERY_CHARACTER_RECT:
            case NS_QUERY_CARET_RECT:
              ConvertRectAppUnitsToIntPixels(
                ((nsQueryContentEvent*)aEvent)->mReply.mRect, p2a);
              break;
          }
        }
    
        break;
      }
    }

  return NS_OK;
}

nsEventStatus nsViewManager::HandleEvent(nsView* aView, nsPoint aPoint,
                                         nsGUIEvent* aEvent, PRBool aCaptured) {
//printf(" %d %d %d %d (%d,%d) \n", this, event->widget, event->widgetSupports, 
//       event->message, event->point.x, event->point.y);

  // Hold a refcount to the observer. The continued existence of the observer will
  // delay deletion of this view hierarchy should the event want to cause its
  // destruction in, say, some JavaScript event handler.
  nsCOMPtr<nsIViewObserver> obs = aView->GetViewManager()->GetViewObserver();
  nsEventStatus status = nsEventStatus_eIgnore;
  if (obs) {
     obs->HandleEvent(aView, aEvent, &status);
  }

  return status;
}

NS_IMETHODIMP nsViewManager::GrabMouseEvents(nsIView *aView, PRBool &aResult)
{
  if (!IsRootVM()) {
    return RootViewManager()->GrabMouseEvents(aView, aResult);
  }

  // Along with nsView::SetVisibility, we enforce that the mouse grabber
  // can never be a hidden view.
  if (aView && static_cast<nsView*>(aView)->GetVisibility()
               == nsViewVisibility_kHide) {
    aView = nsnull;
  }

#ifdef DEBUG_mjudge
  if (aView)
    {
      printf("capturing mouse events for view %x\n",aView);
    }
  printf("removing mouse capture from view %x\n",mMouseGrabber);
#endif

  mMouseGrabber = static_cast<nsView*>(aView);
  aResult = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::GetMouseEventGrabber(nsIView *&aView)
{
  aView = GetMouseEventGrabber();
  return NS_OK;
}

// Recursively reparent widgets if necessary 

void nsViewManager::ReparentChildWidgets(nsIView* aView, nsIWidget *aNewWidget)
{
  if (aView->HasWidget()) {
    // Check to see if the parent widget is the
    // same as the new parent. If not then reparent
    // the widget, otherwise there is nothing more
    // to do for the view and its descendants
    nsIWidget* widget = aView->GetWidget();
    nsIWidget* parentWidget = widget->GetParent();
    if (parentWidget != aNewWidget) {
#ifdef DEBUG
      nsresult rv =
#endif
        widget->SetParent(aNewWidget);
      NS_ASSERTION(NS_SUCCEEDED(rv), "SetParent failed!");
    }
    return;
  }

  // Need to check each of the views children to see
  // if they have a widget and reparent it.

  nsView* view = static_cast<nsView*>(aView);
  for (nsView *kid = view->GetFirstChild(); kid; kid = kid->GetNextSibling()) {
    ReparentChildWidgets(kid, aNewWidget);
  }
}

// Reparent a view and its descendant views widgets if necessary

void nsViewManager::ReparentWidgets(nsIView* aView, nsIView *aParent)
{
  NS_PRECONDITION(aParent, "Must have a parent");
  NS_PRECONDITION(aView, "Must have a view");
  
  // Quickly determine whether the view has pre-existing children or a
  // widget. In most cases the view will not have any pre-existing 
  // children when this is called.  Only in the case
  // where a view has been reparented by removing it from
  // a reinserting it into a new location in the view hierarchy do we
  // have to consider reparenting the existing widgets for the view and
  // it's descendants.
  nsView* view = static_cast<nsView*>(aView);
  if (view->HasWidget() || view->GetFirstChild()) {
    nsIWidget* parentWidget = aParent->GetNearestWidget(nsnull);
    if (parentWidget) {
      ReparentChildWidgets(aView, parentWidget);
      return;
    }
    NS_WARNING("Can not find a widget for the parent view");
  }
}

NS_IMETHODIMP nsViewManager::InsertChild(nsIView *aParent, nsIView *aChild, nsIView *aSibling,
                                         PRBool aAfter)
{
  nsView* parent = static_cast<nsView*>(aParent);
  nsView* child = static_cast<nsView*>(aChild);
  nsView* sibling = static_cast<nsView*>(aSibling);
  
  NS_PRECONDITION(nsnull != parent, "null ptr");
  NS_PRECONDITION(nsnull != child, "null ptr");
  NS_ASSERTION(sibling == nsnull || sibling->GetParent() == parent,
               "tried to insert view with invalid sibling");
  NS_ASSERTION(!IsViewInserted(child), "tried to insert an already-inserted view");

  if ((nsnull != parent) && (nsnull != child))
    {
      // if aAfter is set, we will insert the child after 'prev' (i.e. after 'kid' in document
      // order, otherwise after 'kid' (i.e. before 'kid' in document order).

#if 1
      if (nsnull == aSibling) {
        if (aAfter) {
          // insert at end of document order, i.e., before first view
          // this is the common case, by far
          parent->InsertChild(child, nsnull);
          ReparentWidgets(child, parent);
        } else {
          // insert at beginning of document order, i.e., after last view
          nsView *kid = parent->GetFirstChild();
          nsView *prev = nsnull;
          while (kid) {
            prev = kid;
            kid = kid->GetNextSibling();
          }
          // prev is last view or null if there are no children
          parent->InsertChild(child, prev);
          ReparentWidgets(child, parent);
        }
      } else {
        nsView *kid = parent->GetFirstChild();
        nsView *prev = nsnull;
        while (kid && sibling != kid) {
          //get the next sibling view
          prev = kid;
          kid = kid->GetNextSibling();
        }
        NS_ASSERTION(kid != nsnull,
                     "couldn't find sibling in child list");
        if (aAfter) {
          // insert after 'kid' in document order, i.e. before in view order
          parent->InsertChild(child, prev);
          ReparentWidgets(child, parent);
        } else {
          // insert before 'kid' in document order, i.e. after in view order
          parent->InsertChild(child, kid);
          ReparentWidgets(child, parent);
        }
      }
#else // don't keep consistent document order, but order things by z-index instead
      // essentially we're emulating the old InsertChild(parent, child, zindex)
      PRInt32 zIndex = child->GetZIndex();
      while (nsnull != kid)
        {
          PRInt32 idx = kid->GetZIndex();

          if (CompareZIndex(zIndex, child->IsTopMost(), child->GetZIndexIsAuto(),
                            idx, kid->IsTopMost(), kid->GetZIndexIsAuto()) >= 0)
            break;

          prev = kid;
          kid = kid->GetNextSibling();
        }

      parent->InsertChild(child, prev);
      ReparentWidgets(child, parent);
#endif

      // if the parent view is marked as "floating", make the newly added view float as well.
      if (parent->GetFloating())
        child->SetFloating(PR_TRUE);

      //and mark this area as dirty if the view is visible...

      if (nsViewVisibility_kHide != child->GetVisibility())
        UpdateView(child, NS_VMREFRESH_NO_SYNC);
    }
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::InsertChild(nsIView *aParent, nsIView *aChild, PRInt32 aZIndex)
{
  // no-one really calls this with anything other than aZIndex == 0 on a fresh view
  // XXX this method should simply be eliminated and its callers redirected to the real method
  SetViewZIndex(aChild, PR_FALSE, aZIndex, PR_FALSE);
  return InsertChild(aParent, aChild, nsnull, PR_TRUE);
}

NS_IMETHODIMP nsViewManager::RemoveChild(nsIView *aChild)
{
  nsView* child = static_cast<nsView*>(aChild);
  NS_ENSURE_ARG_POINTER(child);

  nsView* parent = child->GetParent();

  if (nsnull != parent)
    {
      UpdateView(child, NS_VMREFRESH_NO_SYNC);
      parent->RemoveChild(child);
    }

  return NS_OK;
}

NS_IMETHODIMP nsViewManager::MoveViewBy(nsIView *aView, nscoord aX, nscoord aY)
{
  nsView* view = static_cast<nsView*>(aView);

  nsPoint pt = view->GetPosition();
  MoveViewTo(view, aX + pt.x, aY + pt.y);
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::MoveViewTo(nsIView *aView, nscoord aX, nscoord aY)
{
  nsView* view = static_cast<nsView*>(aView);
  nsPoint oldPt = view->GetPosition();
  nsRect oldArea = view->GetBounds();
  view->SetPosition(aX, aY);

  // only do damage control if the view is visible

  if ((aX != oldPt.x) || (aY != oldPt.y)) {
    if (view->GetVisibility() != nsViewVisibility_kHide) {
      nsView* parentView = view->GetParent();
      UpdateView(parentView, oldArea, NS_VMREFRESH_NO_SYNC);
      UpdateView(parentView, view->GetBounds(), NS_VMREFRESH_NO_SYNC);
    }
  }
  return NS_OK;
}

void nsViewManager::InvalidateHorizontalBandDifference(nsView *aView, const nsRect& aRect, const nsRect& aCutOut,
  PRUint32 aUpdateFlags, nscoord aY1, nscoord aY2, PRBool aInCutOut) {
  nscoord height = aY2 - aY1;
  if (aRect.x < aCutOut.x) {
    nsRect r(aRect.x, aY1, aCutOut.x - aRect.x, height);
    UpdateView(aView, r, aUpdateFlags);
  }
  if (!aInCutOut && aCutOut.x < aCutOut.XMost()) {
    nsRect r(aCutOut.x, aY1, aCutOut.width, height);
    UpdateView(aView, r, aUpdateFlags);
  }
  if (aCutOut.XMost() < aRect.XMost()) {
    nsRect r(aCutOut.XMost(), aY1, aRect.XMost() - aCutOut.XMost(), height);
    UpdateView(aView, r, aUpdateFlags);
  }
}

void nsViewManager::InvalidateRectDifference(nsView *aView, const nsRect& aRect, const nsRect& aCutOut,
  PRUint32 aUpdateFlags) {
  if (aRect.y < aCutOut.y) {
    InvalidateHorizontalBandDifference(aView, aRect, aCutOut, aUpdateFlags, aRect.y, aCutOut.y, PR_FALSE);
  }
  if (aCutOut.y < aCutOut.YMost()) {
    InvalidateHorizontalBandDifference(aView, aRect, aCutOut, aUpdateFlags, aCutOut.y, aCutOut.YMost(), PR_TRUE);
  }
  if (aCutOut.YMost() < aRect.YMost()) {
    InvalidateHorizontalBandDifference(aView, aRect, aCutOut, aUpdateFlags, aCutOut.YMost(), aRect.YMost(), PR_FALSE);
  }
}

NS_IMETHODIMP nsViewManager::ResizeView(nsIView *aView, const nsRect &aRect, PRBool aRepaintExposedAreaOnly)
{
  nsView* view = static_cast<nsView*>(aView);
  nsRect oldDimensions;

  view->GetDimensions(oldDimensions);
  if (!oldDimensions.IsExactEqual(aRect)) {
    nsView* parentView = view->GetParent();
    if (parentView == nsnull)
      parentView = view;

    // resize the view.
    // Prevent Invalidation of hidden views 
    if (view->GetVisibility() == nsViewVisibility_kHide) {  
      view->SetDimensions(aRect, PR_FALSE);
    } else {
      if (!aRepaintExposedAreaOnly) {
        //Invalidate the union of the old and new size
        view->SetDimensions(aRect, PR_TRUE);

        UpdateView(view, aRect, NS_VMREFRESH_NO_SYNC);
        view->ConvertToParentCoords(&oldDimensions.x, &oldDimensions.y);
        UpdateView(parentView, oldDimensions, NS_VMREFRESH_NO_SYNC);
      } else {
        view->SetDimensions(aRect, PR_TRUE);

        InvalidateRectDifference(view, aRect, oldDimensions, NS_VMREFRESH_NO_SYNC);
        nsRect r = aRect;
        view->ConvertToParentCoords(&r.x, &r.y);
        view->ConvertToParentCoords(&oldDimensions.x, &oldDimensions.y);
        InvalidateRectDifference(parentView, oldDimensions, r, NS_VMREFRESH_NO_SYNC);
      } 
    }
  }

  // Note that if layout resizes the view and the view has a custom clip
  // region set, then we expect layout to update the clip region too. Thus
  // in the case where mClipRect has been optimized away to just be a null
  // pointer, and this resize is implicitly changing the clip rect, it's OK
  // because layout will change it back again if necessary.

  return NS_OK;
}

PRBool nsViewManager::CanScrollWithBitBlt(nsView* aView, nsPoint aDelta,
                                          nsRegion* aUpdateRegion)
{
  NS_ASSERTION(!IsPainting(),
               "View manager shouldn't be scrolling during a paint");
  if (IsPainting() || !mObserver) {
    return PR_FALSE; // do the safe thing
  }

  nsView* displayRoot = GetDisplayRootFor(aView);
  nsPoint displayOffset = aView->GetParent()->GetOffsetTo(displayRoot);
  nsRect parentBounds = aView->GetParent()->GetDimensions() + displayOffset;
  // The rect we're going to scroll is intersection of the parent bounds with its
  // preimage
  nsRect toScroll;
  toScroll.IntersectRect(parentBounds + aDelta, parentBounds);
  nsresult rv =
    mObserver->ComputeRepaintRegionForCopy(displayRoot, aView, -aDelta, toScroll,
                                           aUpdateRegion);
  NS_ENSURE_SUCCESS(rv, PR_FALSE);

  aUpdateRegion->MoveBy(-displayOffset);

#if defined(MOZ_WIDGET_GTK2) || defined(XP_OS2)
  return aUpdateRegion->IsEmpty();
#else
  return PR_TRUE;
#endif
}

NS_IMETHODIMP nsViewManager::SetViewFloating(nsIView *aView, PRBool aFloating)
{
  nsView* view = static_cast<nsView*>(aView);

  NS_ASSERTION(!(nsnull == view), "no view");

  view->SetFloating(aFloating);

  return NS_OK;
}

NS_IMETHODIMP nsViewManager::SetViewVisibility(nsIView *aView, nsViewVisibility aVisible)
{
  nsView* view = static_cast<nsView*>(aView);

  if (aVisible != view->GetVisibility()) {
    view->SetVisibility(aVisible);

    if (IsViewInserted(view)) {
      if (!view->HasWidget()) {
        if (nsViewVisibility_kHide == aVisible) {
          nsView* parentView = view->GetParent();
          if (parentView) {
            UpdateView(parentView, view->GetBounds(), NS_VMREFRESH_NO_SYNC);
          }
        }
        else {
          UpdateView(view, NS_VMREFRESH_NO_SYNC);
        }
      }
    }

    // Any child views not associated with frames might not get their visibility
    // updated, so propagate our visibility to them. This is important because
    // hidden views should have all hidden children.
    for (nsView* childView = view->GetFirstChild(); childView;
         childView = childView->GetNextSibling()) {
      if (!childView->GetClientData()) {
        childView->SetVisibility(aVisible);
      }
    }
  }
  return NS_OK;
}

void nsViewManager::UpdateWidgetsForView(nsView* aView)
{
  NS_PRECONDITION(aView, "Must have view!");

  if (aView->HasWidget()) {
    aView->GetWidget()->Update();
  }

  for (nsView* childView = aView->GetFirstChild();
       childView;
       childView = childView->GetNextSibling()) {
    UpdateWidgetsForView(childView);
  }
}

PRBool nsViewManager::IsViewInserted(nsView *aView)
{
  if (mRootView == aView) {
    return PR_TRUE;
  } else if (aView->GetParent() == nsnull) {
    return PR_FALSE;
  } else {
    nsView* view = aView->GetParent()->GetFirstChild();
    while (view != nsnull) {
      if (view == aView) {
        return PR_TRUE;
      }        
      view = view->GetNextSibling();
    }
    return PR_FALSE;
  }
}

NS_IMETHODIMP nsViewManager::SetViewZIndex(nsIView *aView, PRBool aAutoZIndex, PRInt32 aZIndex, PRBool aTopMost)
{
  nsView* view = static_cast<nsView*>(aView);
  nsresult  rv = NS_OK;

  NS_ASSERTION((view != nsnull), "no view");

  // don't allow the root view's z-index to be changed. It should always be zero.
  // This could be removed and replaced with a style rule, or just removed altogether, with interesting consequences
  if (aView == mRootView) {
    return rv;
  }

  PRBool oldTopMost = view->IsTopMost();
  PRBool oldIsAuto = view->GetZIndexIsAuto();

  if (aAutoZIndex) {
    aZIndex = 0;
  }

  PRInt32 oldidx = view->GetZIndex();
  view->SetZIndex(aAutoZIndex, aZIndex, aTopMost);

  if (oldidx != aZIndex || oldTopMost != aTopMost ||
      oldIsAuto != aAutoZIndex) {
    UpdateView(view, NS_VMREFRESH_NO_SYNC);
  }

  return rv;
}

NS_IMETHODIMP nsViewManager::SetViewObserver(nsIViewObserver *aObserver)
{
  mObserver = aObserver;
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::GetViewObserver(nsIViewObserver *&aObserver)
{
  if (nsnull != mObserver) {
    aObserver = mObserver;
    NS_ADDREF(mObserver);
    return NS_OK;
  } else
    return NS_ERROR_NO_INTERFACE;
}

NS_IMETHODIMP nsViewManager::GetDeviceContext(nsIDeviceContext *&aContext)
{
  NS_IF_ADDREF(mContext);
  aContext = mContext;
  return NS_OK;
}

already_AddRefed<nsIRenderingContext>
nsViewManager::CreateRenderingContext(nsView &aView)
{
  nsView              *par = &aView;
  nsIWidget*          win;
  nsIRenderingContext *cx = nsnull;
  nscoord             ax = 0, ay = 0;

  do
    {
      win = par->GetWidget();
      if (win)
        break;

      //get absolute coordinates of view, but don't
      //add in view pos since the first thing you ever
      //need to do when painting a view is to translate
      //the rendering context by the views pos and other parts
      //of the code do this for us...

      if (par != &aView)
        {
          par->ConvertToParentCoords(&ax, &ay);
        }

      par = par->GetParent();
    }
  while (nsnull != par);

  if (nsnull != win)
    {
      mContext->CreateRenderingContext(par, cx);

      if (nsnull != cx)
        cx->Translate(ax, ay);
    }

  return cx;
}

NS_IMETHODIMP nsViewManager::DisableRefresh(void)
{
  if (!IsRootVM()) {
    return RootViewManager()->DisableRefresh();
  }
  
  if (mUpdateBatchCnt > 0)
    return NS_OK;

  mRefreshEnabled = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::EnableRefresh(PRUint32 aUpdateFlags)
{
  if (!IsRootVM()) {
    return RootViewManager()->EnableRefresh(aUpdateFlags);
  }
  
  if (mUpdateBatchCnt > 0)
    return NS_OK;

  mRefreshEnabled = PR_TRUE;

  if (!mHasPendingUpdates) {
    // Nothing to do
    return NS_OK;
  }

  // nested batching can combine IMMEDIATE with DEFERRED. Favour
  // IMMEDIATE over DEFERRED and DEFERRED over NO_SYNC.
  if (aUpdateFlags & NS_VMREFRESH_IMMEDIATE) {
    FlushPendingInvalidates();
    Composite();
  } else if (aUpdateFlags & NS_VMREFRESH_DEFERRED) {
    PostInvalidateEvent();
  } else { // NO_SYNC
    FlushPendingInvalidates();
  }

  return NS_OK;
}

nsIViewManager* nsViewManager::BeginUpdateViewBatch(void)
{
  if (!IsRootVM()) {
    return RootViewManager()->BeginUpdateViewBatch();
  }
  
  nsresult result = NS_OK;
  
  if (mUpdateBatchCnt == 0) {
    mUpdateBatchFlags = 0;
    result = DisableRefresh();
  }

  if (NS_SUCCEEDED(result))
    ++mUpdateBatchCnt;

  return this;
}

NS_IMETHODIMP nsViewManager::EndUpdateViewBatch(PRUint32 aUpdateFlags)
{
  NS_ASSERTION(IsRootVM(), "Should only be called on root");
  
  nsresult result = NS_OK;

  --mUpdateBatchCnt;

  NS_ASSERTION(mUpdateBatchCnt >= 0, "Invalid batch count!");

  if (mUpdateBatchCnt < 0)
    {
      mUpdateBatchCnt = 0;
      return NS_ERROR_FAILURE;
    }

  mUpdateBatchFlags |= aUpdateFlags;
  if (mUpdateBatchCnt == 0) {
    result = EnableRefresh(mUpdateBatchFlags);
  }

  return result;
}

NS_IMETHODIMP nsViewManager::SetRootScrollableView(nsIScrollableView *aScrollable)
{
  mRootScrollable = aScrollable;
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::GetRootScrollableView(nsIScrollableView **aScrollable)
{
  *aScrollable = mRootScrollable;
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::GetWidget(nsIWidget **aWidget)
{
  *aWidget = GetWidget();
  NS_IF_ADDREF(*aWidget);
  return NS_OK;
}

NS_IMETHODIMP nsViewManager::ForceUpdate()
{
  if (!IsRootVM()) {
    return RootViewManager()->ForceUpdate();
  }

  // Walk the view tree looking for widgets, and call Update() on each one
  if (mRootView) {
    UpdateWidgetsForView(mRootView);
  }
  
  return NS_OK;
}

nsPoint nsViewManager::ComputeViewOffset(const nsView *aView)
{
  NS_PRECONDITION(aView, "Null view in ComputeViewOffset?");
  
  nsPoint origin(0, 0);
#ifdef DEBUG
  const nsView* rootView;
  const nsView* origView = aView;
#endif

  while (aView) {
#ifdef DEBUG
    rootView = aView;
#endif
    origin += aView->GetPosition();
    aView = aView->GetParent();
  }
  NS_ASSERTION(rootView ==
               origView->GetViewManager()->RootViewManager()->GetRootView(),
               "Unexpected root view");
  return origin;
}

void nsViewManager::ViewToWidget(nsView *aView, nsView* aWidgetView, nsRect &aRect) const
{
  while (aView != aWidgetView) {
    aView->ConvertToParentCoords(&aRect.x, &aRect.y);
    aView = aView->GetParent();
  }
  
  // intersect aRect with bounds of aWidgetView, to prevent generating any illegal rectangles.
  nsRect bounds;
  aWidgetView->GetDimensions(bounds);
  aRect.IntersectRect(aRect, bounds);
  // account for the view's origin not lining up with the widget's
  aRect.x -= bounds.x;
  aRect.y -= bounds.y;

  aRect += aView->ViewToWidgetOffset();

  // finally, convert to device coordinates.
  aRect.ScaleRoundOut(1.0f / mContext->AppUnitsPerDevPixel());
}

nsresult nsViewManager::GetVisibleRect(nsRect& aVisibleRect)
{
  nsresult rv = NS_OK;

  // Get the viewport scroller
  nsIScrollableView* scrollingView;
  GetRootScrollableView(&scrollingView);

  if (scrollingView) {   
    // Determine the visible rect in the scrolled view's coordinate space.
    // The size of the visible area is the clip view size
    nsScrollPortView* clipView = static_cast<nsScrollPortView*>(scrollingView);
    clipView->GetDimensions(aVisibleRect);

    scrollingView->GetScrollPosition(aVisibleRect.x, aVisibleRect.y);
  } else {
    rv = NS_ERROR_FAILURE;
  }

  return rv;
}

nsresult nsViewManager::GetAbsoluteRect(nsView *aView, const nsRect &aRect, 
                                        nsRect& aAbsRect)
{
  nsIScrollableView* scrollingView = nsnull;
  GetRootScrollableView(&scrollingView);
  if (nsnull == scrollingView) { 
    return NS_ERROR_FAILURE;
  }

  nsIView* scrolledIView = nsnull;
  scrollingView->GetScrolledView(scrolledIView);
  
  nsView* scrolledView = static_cast<nsView*>(scrolledIView);

  // Calculate the absolute coordinates of the aRect passed in.
  // aRects values are relative to aView
  aAbsRect = aRect;
  nsView *parentView = aView;
  while ((parentView != nsnull) && (parentView != scrolledView)) {
    parentView->ConvertToParentCoords(&aAbsRect.x, &aAbsRect.y);
    parentView = parentView->GetParent();
  }

  if (parentView != scrolledView) {
    return NS_ERROR_FAILURE;
  }

  return NS_OK;
}


NS_IMETHODIMP nsViewManager::GetRectVisibility(nsIView *aView, 
                                               const nsRect &aRect,
                                               PRUint16 aMinTwips, 
                                               nsRectVisibility *aRectVisibility)
{
  nsView* view = static_cast<nsView*>(aView);

  // The parameter aMinTwips determines how many rows/cols of pixels must be visible on each side of the element,
  // in order to be counted as visible

  *aRectVisibility = nsRectVisibility_kZeroAreaRect;
  if (aRect.width == 0 || aRect.height == 0) {
    return NS_OK;
  }

  // is this view even visible?
  if (view->GetVisibility() == nsViewVisibility_kHide) {
    return NS_OK; 
  }

  // nsViewManager::InsertChild ensures that descendants of floating views
  // are also marked floating.
  if (view->GetFloating()) {
    *aRectVisibility = nsRectVisibility_kVisible;
    return NS_OK;
  }

  // Calculate the absolute coordinates for the visible rectangle   
  nsRect visibleRect;
  if (GetVisibleRect(visibleRect) == NS_ERROR_FAILURE) {
    *aRectVisibility = nsRectVisibility_kVisible;
    return NS_OK;
  }

  // Calculate the absolute coordinates of the aRect passed in.
  // aRects values are relative to aView
  nsRect absRect;
  if ((GetAbsoluteRect(view, aRect, absRect)) == NS_ERROR_FAILURE) {
    *aRectVisibility = nsRectVisibility_kVisible;
    return NS_OK;
  }
 
  /*
   * If aMinTwips > 0, ensure at least aMinTwips of space around object is visible
   * The object is not visible if:
   * ((objectTop     < windowTop    && objectBottom < windowTop) ||
   *  (objectBottom  > windowBottom && objectTop    > windowBottom) ||
   *  (objectLeft    < windowLeft   && objectRight  < windowLeft) ||
   *  (objectRight   > windowRight  && objectLeft   > windowRight))
   */

  if (absRect.y < visibleRect.y  && 
      absRect.y + absRect.height < visibleRect.y + aMinTwips)
    *aRectVisibility = nsRectVisibility_kAboveViewport;
  else if (absRect.y + absRect.height > visibleRect.y + visibleRect.height &&
           absRect.y > visibleRect.y + visibleRect.height - aMinTwips)
    *aRectVisibility = nsRectVisibility_kBelowViewport;
  else if (absRect.x < visibleRect.x && 
           absRect.x + absRect.width < visibleRect.x + aMinTwips)
    *aRectVisibility = nsRectVisibility_kLeftOfViewport;
  else if (absRect.x + absRect.width > visibleRect.x  + visibleRect.width &&
           absRect.x > visibleRect.x + visibleRect.width - aMinTwips)
    *aRectVisibility = nsRectVisibility_kRightOfViewport;
  else
    *aRectVisibility = nsRectVisibility_kVisible;

  return NS_OK;
}

NS_IMETHODIMP
nsViewManager::IsPainting(PRBool& aIsPainting)
{
  aIsPainting = IsPainting();
  return NS_OK;
}

void
nsViewManager::FlushPendingInvalidates()
{
  NS_ASSERTION(IsRootVM(), "Must be root VM for this to be called!\n");
  NS_ASSERTION(mUpdateBatchCnt == 0, "Must not be in an update batch!");
  // XXXbz this is probably not quite OK yet, if callers can explicitly
  // DisableRefresh while we have an event posted.
  // NS_ASSERTION(mRefreshEnabled, "How did we get here?");

  // Let all the view observers of all viewmanagers in this tree know that
  // we're about to "paint" (this lets them get in their invalidates now so
  // we don't go through two invalidate-processing cycles).
  NS_ASSERTION(gViewManagers, "Better have a viewmanagers array!");

  // Make sure to not send WillPaint notifications while scrolling
  if (mScrollCnt == 0) {
    // Disable refresh while we notify our view observers, so that if they do
    // view update batches we don't reenter this code and so that we batch
    // all of them together.  We don't use
    // BeginUpdateViewBatch/EndUpdateViewBatch, since that would reenter this
    // exact code, but we want the effect of a single big update batch.
    PRBool refreshEnabled = mRefreshEnabled;
    mRefreshEnabled = PR_FALSE;
    ++mUpdateBatchCnt;
    
    PRInt32 index;
    for (index = 0; index < mVMCount; index++) {
      nsViewManager* vm = (nsViewManager*)gViewManagers->ElementAt(index);
      if (vm->RootViewManager() == this) {
        // One of our kids
        nsIViewObserver* observer = vm->GetViewObserver();
        if (observer) {
          observer->WillPaint();
          NS_ASSERTION(mUpdateBatchCnt == 1,
                       "Observer did not end view batch?");
        }
      }
    }
    
    --mUpdateBatchCnt;
    // Someone could have called EnableRefresh on us from inside WillPaint().
    // Only reset the old mRefreshEnabled value if the current value is false.
    if (!mRefreshEnabled) {
      mRefreshEnabled = refreshEnabled;
    }
  }
  
  if (mHasPendingUpdates) {
    ProcessPendingUpdates(mRootView, PR_TRUE);
    mHasPendingUpdates = PR_FALSE;
  }
}

void
nsViewManager::ProcessInvalidateEvent()
{
  NS_ASSERTION(IsRootVM(),
               "Incorrectly targeted invalidate event");
  // If we're in the middle of an update batch, just repost the event,
  // to be processed when the batch ends.
  PRBool processEvent = (mUpdateBatchCnt == 0);
  if (processEvent) {
    FlushPendingInvalidates();
  }
  mInvalidateEvent.Forget();
  if (!processEvent) {
    // We didn't actually process this event... post a new one
    PostInvalidateEvent();
  }
}

NS_IMETHODIMP
nsViewManager::SetDefaultBackgroundColor(nscolor aColor)
{
  mDefaultBackgroundColor = aColor;
  return NS_OK;
}

NS_IMETHODIMP
nsViewManager::GetDefaultBackgroundColor(nscolor* aColor)
{
  *aColor = mDefaultBackgroundColor;
  return NS_OK;
}


NS_IMETHODIMP
nsViewManager::GetLastUserEventTime(PRUint32& aTime)
{
  aTime = gLastUserEventTime;
  return NS_OK;
}

class nsSynthMouseMoveEvent : public nsViewManagerEvent {
public:
  nsSynthMouseMoveEvent(nsViewManager *aViewManager,
                        PRBool aFromScroll)
    : nsViewManagerEvent(aViewManager),
      mFromScroll(aFromScroll) {
  }

  NS_IMETHOD Run() {
    if (mViewManager)
      mViewManager->ProcessSynthMouseMoveEvent(mFromScroll);
    return NS_OK;
  }

private:
  PRBool mFromScroll;
};

NS_IMETHODIMP
nsViewManager::SynthesizeMouseMove(PRBool aFromScroll)
{
  if (!IsRootVM())
    return RootViewManager()->SynthesizeMouseMove(aFromScroll);

  if (mMouseLocation == nsPoint(NSCOORD_NONE, NSCOORD_NONE))
    return NS_OK;

  if (!mSynthMouseMoveEvent.IsPending()) {
    nsRefPtr<nsViewManagerEvent> ev =
        new nsSynthMouseMoveEvent(this, aFromScroll);

    if (NS_FAILED(NS_DispatchToCurrentThread(ev))) {
      NS_WARNING("failed to dispatch nsSynthMouseMoveEvent");
      return NS_ERROR_UNEXPECTED;
    }

    mSynthMouseMoveEvent = ev;
  }

  return NS_OK;
}

/**
 * Find the first floating view with a widget in a postorder traversal of the
 * view tree that contains the point. Thus more deeply nested floating views
 * are preferred over their ancestors, and floating views earlier in the
 * view hierarchy (i.e., added later) are preferred over their siblings.
 * This is adequate for finding the "topmost" floating view under a point,
 * given that floating views don't supporting having a specific z-index.
 * 
 * We cannot exit early when aPt is outside the view bounds, because floating
 * views aren't necessarily included in their parent's bounds, so this could
 * traverse the entire view hierarchy --- use carefully.
 */
static nsView* FindFloatingViewContaining(nsView* aView, nsPoint aPt)
{
  for (nsView* v = aView->GetFirstChild(); v; v = v->GetNextSibling()) {
    nsView* r = FindFloatingViewContaining(v, aPt - v->GetOffsetTo(aView));
    if (r)
      return r;
  }

  if (aView->GetFloating() && aView->HasWidget() &&
      aView->GetDimensions().Contains(aPt) && IsViewVisible(aView))
    return aView;
    
  return nsnull;
}

void
nsViewManager::ProcessSynthMouseMoveEvent(PRBool aFromScroll)
{
  // allow new event to be posted while handling this one only if the
  // source of the event is a scroll (to prevent infinite reflow loops)
  if (aFromScroll)
    mSynthMouseMoveEvent.Forget();

  NS_ASSERTION(IsRootVM(), "Only the root view manager should be here");

  if (mMouseLocation == nsPoint(NSCOORD_NONE, NSCOORD_NONE) || !mRootView) {
    mSynthMouseMoveEvent.Forget();
    return;
  }

  // Hold a ref to ourselves so DispatchEvent won't destroy us (since
  // we need to access members after we call DispatchEvent).
  nsCOMPtr<nsIViewManager> kungFuDeathGrip(this);
  
#ifdef DEBUG_MOUSE_LOCATION
  printf("[vm=%p]synthesizing mouse move to (%d,%d)\n",
         this, mMouseLocation.x, mMouseLocation.y);
#endif
                                                       
  nsPoint pt = mMouseLocation;
  PRInt32 p2a = mContext->AppUnitsPerDevPixel();
  pt.x = NSIntPixelsToAppUnits(mMouseLocation.x, p2a);
  pt.y = NSIntPixelsToAppUnits(mMouseLocation.y, p2a);
  // This could be a bit slow (traverses entire view hierarchy)
  // but it's OK to do it once per synthetic mouse event
  nsView* view = FindFloatingViewContaining(mRootView, pt);
  nsPoint offset(0, 0);
  if (!view) {
    view = mRootView;
  } else {
    offset = view->GetOffsetTo(mRootView);
    offset.x = NSAppUnitsToIntPixels(offset.x, p2a);
    offset.y = NSAppUnitsToIntPixels(offset.y, p2a);
  }
  nsMouseEvent event(PR_TRUE, NS_MOUSE_MOVE, view->GetWidget(),
                     nsMouseEvent::eSynthesized);
  event.refPoint = mMouseLocation - offset;
  event.time = PR_IntervalNow();
  // XXX set event.isShift, event.isControl, event.isAlt, event.isMeta ?

  nsEventStatus status;
  view->GetViewManager()->DispatchEvent(&event, &status);

  if (!aFromScroll)
    mSynthMouseMoveEvent.Forget();
}

void
nsViewManager::InvalidateHierarchy()
{
  if (mRootView) {
    if (!IsRootVM()) {
      NS_RELEASE(mRootViewManager);
    }
    nsView *parent = mRootView->GetParent();
    if (parent) {
      mRootViewManager = parent->GetViewManager()->RootViewManager();
      NS_ADDREF(mRootViewManager);
      NS_ASSERTION(mRootViewManager != this,
                   "Root view had a parent, but it has the same view manager");
    } else {
      mRootViewManager = this;
    }
  }
}
