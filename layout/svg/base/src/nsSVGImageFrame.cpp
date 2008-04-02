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
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
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

#include "nsSVGPathGeometryFrame.h"
#include "nsIDOMSVGMatrix.h"
#include "nsIDOMSVGAnimPresAspRatio.h"
#include "imgIContainer.h"
#include "gfxIImageFrame.h"
#include "nsStubImageDecoderObserver.h"
#include "nsImageLoadingContent.h"
#include "nsIDOMSVGImageElement.h"
#include "nsSVGElement.h"
#include "nsSVGUtils.h"
#include "nsSVGMatrix.h"
#include "gfxContext.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsThebesImage.h"

class nsSVGImageFrame;

class nsSVGImageListener : public nsStubImageDecoderObserver
{
public:
  nsSVGImageListener(nsSVGImageFrame *aFrame);

  NS_DECL_ISUPPORTS
  // imgIDecoderObserver (override nsStubImageDecoderObserver)
  NS_IMETHOD OnStopDecode(imgIRequest *aRequest, nsresult status,
                          const PRUnichar *statusArg);
  // imgIContainerObserver (override nsStubImageDecoderObserver)
  NS_IMETHOD FrameChanged(imgIContainer *aContainer, gfxIImageFrame *newframe,
                          nsRect * dirtyRect);
  // imgIContainerObserver (override nsStubImageDecoderObserver)
  NS_IMETHOD OnStartContainer(imgIRequest *aRequest,
                              imgIContainer *aContainer);

  void SetFrame(nsSVGImageFrame *frame) { mFrame = frame; }

private:
  nsSVGImageFrame *mFrame;
};


class nsSVGImageFrame : public nsSVGPathGeometryFrame
{
  friend nsIFrame*
  NS_NewSVGImageFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsStyleContext* aContext);

protected:
  nsSVGImageFrame(nsStyleContext* aContext) : nsSVGPathGeometryFrame(aContext) {}
  virtual ~nsSVGImageFrame();

public:
  // nsISVGChildFrame interface:
  NS_IMETHOD PaintSVG(nsSVGRenderState *aContext, nsRect *aDirtyRect);
  NS_IMETHOD GetFrameForPointSVG(float x, float y, nsIFrame** hit);

  // nsSVGGeometryFrame overload:
  // Lie about our fill/stroke so hit detection works
  virtual nsresult GetStrokePaintType() { return eStyleSVGPaintType_None; }
  virtual nsresult GetFillPaintType() { return eStyleSVGPaintType_Color; }

  // nsSVGPathGeometryFrame methods:
  virtual PRUint16 GetHittestMask();

  // nsIFrame interface:
  NS_IMETHOD  AttributeChanged(PRInt32         aNameSpaceID,
                               nsIAtom*        aAttribute,
                               PRInt32         aModType);
  NS_IMETHOD Init(nsIContent*      aContent,
                  nsIFrame*        aParent,
                  nsIFrame*        aPrevInFlow);

  /**
   * Get the "type" of the frame
   *
   * @see nsGkAtoms::svgImageFrame
   */
  virtual nsIAtom* GetType() const;

#ifdef DEBUG
  NS_IMETHOD GetFrameName(nsAString& aResult) const
  {
    return MakeFrameName(NS_LITERAL_STRING("SVGImage"), aResult);
  }
#endif

private:
  already_AddRefed<nsIDOMSVGMatrix> GetImageTransform();

  nsCOMPtr<imgIDecoderObserver> mListener;

  nsCOMPtr<imgIContainer> mImageContainer;

  friend class nsSVGImageListener;
};

//----------------------------------------------------------------------
// Implementation

nsIFrame*
NS_NewSVGImageFrame(nsIPresShell* aPresShell, nsIContent* aContent, nsStyleContext* aContext)
{
  nsCOMPtr<nsIDOMSVGImageElement> Rect = do_QueryInterface(aContent);
  if (!Rect) {
    NS_ERROR("Can't create frame! Content is not an SVG image!");
    return nsnull;
  }

  return new (aPresShell) nsSVGImageFrame(aContext);
}

nsSVGImageFrame::~nsSVGImageFrame()
{
  // set the frame to null so we don't send messages to a dead object.
  if (mListener) {
    nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
    if (imageLoader) {
      imageLoader->RemoveObserver(mListener);
    }
    reinterpret_cast<nsSVGImageListener*>(mListener.get())->SetFrame(nsnull);
  }
  mListener = nsnull;
}

NS_IMETHODIMP
nsSVGImageFrame::Init(nsIContent* aContent,
                      nsIFrame* aParent,
                      nsIFrame* aPrevInFlow)
{
  nsresult rv = nsSVGPathGeometryFrame::Init(aContent, aParent, aPrevInFlow);
  if (NS_FAILED(rv)) return rv;
  
  mListener = new nsSVGImageListener(this);
  if (!mListener) return NS_ERROR_OUT_OF_MEMORY;
  nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
  NS_ENSURE_TRUE(imageLoader, NS_ERROR_UNEXPECTED);
  imageLoader->AddObserver(mListener);

  return NS_OK; 
}

//----------------------------------------------------------------------
// nsIFrame methods:

NS_IMETHODIMP
nsSVGImageFrame::AttributeChanged(PRInt32         aNameSpaceID,
                                  nsIAtom*        aAttribute,
                                  PRInt32         aModType)
{
   if (aNameSpaceID == kNameSpaceID_None &&
       (aAttribute == nsGkAtoms::x ||
        aAttribute == nsGkAtoms::y ||
        aAttribute == nsGkAtoms::width ||
        aAttribute == nsGkAtoms::height ||
        aAttribute == nsGkAtoms::preserveAspectRatio)) {
     UpdateGraphic();
     return NS_OK;
   }

   return nsSVGPathGeometryFrame::AttributeChanged(aNameSpaceID,
                                                   aAttribute, aModType);
}

already_AddRefed<nsIDOMSVGMatrix>
nsSVGImageFrame::GetImageTransform()
{
  nsCOMPtr<nsIDOMSVGMatrix> ctm;
  GetCanvasTM(getter_AddRefs(ctm));

  float x, y, width, height;
  nsSVGElement *element = static_cast<nsSVGElement*>(mContent);
  element->GetAnimatedLengthValues(&x, &y, &width, &height, nsnull);

  PRInt32 nativeWidth, nativeHeight;
  mImageContainer->GetWidth(&nativeWidth);
  mImageContainer->GetHeight(&nativeHeight);

  nsCOMPtr<nsIDOMSVGImageElement> image = do_QueryInterface(mContent);
  nsCOMPtr<nsIDOMSVGAnimatedPreserveAspectRatio> ratio;
  image->GetPreserveAspectRatio(getter_AddRefs(ratio));

  nsCOMPtr<nsIDOMSVGMatrix> trans, ctmXY, fini;
  trans = nsSVGUtils::GetViewBoxTransform(width, height,
                                          0, 0,
                                          nativeWidth, nativeHeight,
                                          ratio);
  ctm->Translate(x, y, getter_AddRefs(ctmXY));
  ctmXY->Multiply(trans, getter_AddRefs(fini));

  nsIDOMSVGMatrix *retval = nsnull;
  fini.swap(retval);
  return retval;
}

//----------------------------------------------------------------------
// nsISVGChildFrame methods:
NS_IMETHODIMP
nsSVGImageFrame::PaintSVG(nsSVGRenderState *aContext, nsRect *aDirtyRect)
{
  nsresult rv = NS_OK;

  if (!GetStyleVisibility()->IsVisible())
    return NS_OK;

  float x, y, width, height;
  nsSVGElement *element = static_cast<nsSVGElement*>(mContent);
  element->GetAnimatedLengthValues(&x, &y, &width, &height, nsnull);
  if (width <= 0 || height <= 0)
    return NS_OK;

  if (!mImageContainer) {
    nsCOMPtr<imgIRequest> currentRequest;
    nsCOMPtr<nsIImageLoadingContent> imageLoader = do_QueryInterface(mContent);
    if (imageLoader)
      imageLoader->GetRequest(nsIImageLoadingContent::CURRENT_REQUEST,
                              getter_AddRefs(currentRequest));

    if (currentRequest)
      currentRequest->GetImage(getter_AddRefs(mImageContainer));
  }

  nsCOMPtr<gfxIImageFrame> currentFrame;
  if (mImageContainer)
    mImageContainer->GetCurrentFrame(getter_AddRefs(currentFrame));

  gfxASurface *thebesSurface = nsnull;
  if (currentFrame) {
    nsCOMPtr<nsIImage> img(do_GetInterface(currentFrame));

    nsThebesImage *thebesImage = nsnull;
    if (img)
      thebesImage = static_cast<nsThebesImage*>(img.get());

    if (thebesImage)
      thebesSurface = thebesImage->ThebesSurface();
  }

  if (thebesSurface) {
    gfxContext *gfx = aContext->GetGfxContext();

    if (GetStyleDisplay()->IsScrollableOverflow()) {
      gfx->Save();

      nsCOMPtr<nsIDOMSVGMatrix> ctm;
      GetCanvasTM(getter_AddRefs(ctm));

      if (ctm) {
        nsSVGUtils::SetClipRect(gfx, ctm, x, y, width, height);
      }
    }

    nsCOMPtr<nsIDOMSVGMatrix> fini = GetImageTransform();

    // fill-opacity doesn't affect <image>, so if we're allowed to
    // optimize group opacity, the opacity used for compositing the
    // image into the current canvas is just the group opacity.
    float opacity = 1.0f;
    if (nsSVGUtils::CanOptimizeOpacity(this)) {
      opacity = GetStyleDisplay()->mOpacity;
    }

    nsSVGUtils::CompositeSurfaceMatrix(gfx, thebesSurface, fini, opacity);

    if (GetStyleDisplay()->IsScrollableOverflow())
      gfx->Restore();
  }

  return rv;
}

NS_IMETHODIMP
nsSVGImageFrame::GetFrameForPointSVG(float x, float y, nsIFrame** hit)
{
  if (GetStyleDisplay()->IsScrollableOverflow() && mImageContainer) {
    PRInt32 nativeWidth, nativeHeight;
    mImageContainer->GetWidth(&nativeWidth);
    mImageContainer->GetHeight(&nativeHeight);

    nsCOMPtr<nsIDOMSVGMatrix> fini = GetImageTransform();

    if (!nsSVGUtils::HitTestRect(fini,
                                 0, 0, nativeWidth, nativeHeight,
                                 x, y)) {
      *hit = nsnull;
      return NS_OK;
    }
  }

  return nsSVGPathGeometryFrame::GetFrameForPointSVG(x, y, hit);
}

nsIAtom *
nsSVGImageFrame::GetType() const
{
  return nsGkAtoms::svgImageFrame;
}

//----------------------------------------------------------------------
// nsSVGPathGeometryFrame methods:

// Lie about our fill/stroke so that hit detection works properly

PRUint16
nsSVGImageFrame::GetHittestMask()
{
  PRUint16 mask = 0;

  switch(GetStyleSVG()->mPointerEvents) {
    case NS_STYLE_POINTER_EVENTS_NONE:
      break;
    case NS_STYLE_POINTER_EVENTS_VISIBLEPAINTED:
      if (GetStyleVisibility()->IsVisible()) {
        /* XXX: should check pixel transparency */
        mask |= HITTEST_MASK_FILL;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_VISIBLEFILL:
    case NS_STYLE_POINTER_EVENTS_VISIBLESTROKE:
    case NS_STYLE_POINTER_EVENTS_VISIBLE:
      if (GetStyleVisibility()->IsVisible()) {
        mask |= HITTEST_MASK_FILL;
      }
      break;
    case NS_STYLE_POINTER_EVENTS_PAINTED:
      /* XXX: should check pixel transparency */
      mask |= HITTEST_MASK_FILL;
      break;
    case NS_STYLE_POINTER_EVENTS_FILL:
    case NS_STYLE_POINTER_EVENTS_STROKE:
    case NS_STYLE_POINTER_EVENTS_ALL:
      mask |= HITTEST_MASK_FILL;
      break;
    default:
      NS_ERROR("not reached");
      break;
  }

  return mask;
}

//----------------------------------------------------------------------
// nsSVGImageListener implementation

NS_IMPL_ISUPPORTS2(nsSVGImageListener,
                   imgIDecoderObserver,
                   imgIContainerObserver)

nsSVGImageListener::nsSVGImageListener(nsSVGImageFrame *aFrame) :  mFrame(aFrame)
{
}

NS_IMETHODIMP nsSVGImageListener::OnStopDecode(imgIRequest *aRequest,
                                               nsresult status,
                                               const PRUnichar *statusArg)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  mFrame->UpdateGraphic();
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::FrameChanged(imgIContainer *aContainer,
                                               gfxIImageFrame *newframe,
                                               nsRect * dirtyRect)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  mFrame->UpdateGraphic();
  return NS_OK;
}

NS_IMETHODIMP nsSVGImageListener::OnStartContainer(imgIRequest *aRequest,
                                                   imgIContainer *aContainer)
{
  if (!mFrame)
    return NS_ERROR_FAILURE;

  mFrame->mImageContainer = aContainer;
  mFrame->UpdateGraphic();

  return NS_OK;
}

