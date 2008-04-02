/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * mozilla.org.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@pavlov.net>
 *   Vladimir Vukicevic <vladimir@pobox.com>
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

#include "nsThebesRenderingContext.h"
#include "nsThebesDeviceContext.h"

#include "nsRect.h"
#include "nsString.h"
#include "nsTransform2D.h"
#include "nsIRegion.h"
#include "nsIServiceManager.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsGfxCIID.h"

#include "imgIContainer.h"
#include "gfxIImageFrame.h"
#include "nsIImage.h"

#include "nsIThebesFontMetrics.h"
#include "nsThebesRegion.h"
#include "nsThebesImage.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "gfxPlatform.h"

#ifdef XP_WIN
#include "gfxWindowsSurface.h"
#include "cairo-win32.h"
#endif

static NS_DEFINE_CID(kRegionCID, NS_REGION_CID);

//////////////////////////////////////////////////////////////////////

// XXXTodo: rename FORM_TWIPS to FROM_APPUNITS
#define FROM_TWIPS(_x)  ((gfxFloat)((_x)/(mP2A)))
#define FROM_TWIPS_INT(_x)  (NSToIntRound((gfxFloat)((_x)/(mP2A))))
#define TO_TWIPS(_x)    ((nscoord)((_x)*(mP2A)))
#define GFX_RECT_FROM_TWIPS_RECT(_r)   (gfxRect(FROM_TWIPS((_r).x), FROM_TWIPS((_r).y), FROM_TWIPS((_r).width), FROM_TWIPS((_r).height)))

//////////////////////////////////////////////////////////////////////

NS_IMPL_ISUPPORTS1(nsThebesRenderingContext, nsIRenderingContext)

nsThebesRenderingContext::nsThebesRenderingContext() :
    mLineStyle(nsLineStyle_kNone)
{
}

nsThebesRenderingContext::~nsThebesRenderingContext()
{
}

//////////////////////////////////////////////////////////////////////
//// nsIRenderingContext

NS_IMETHODIMP
nsThebesRenderingContext::Init(nsIDeviceContext* aContext, gfxASurface *aThebesSurface)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::Init ctx %p thebesSurface %p\n", this, aContext, aThebesSurface));

    nsThebesDeviceContext *thebesDC = static_cast<nsThebesDeviceContext*>(aContext);

    mDeviceContext = aContext;
    mWidget = nsnull;

    mThebes = new gfxContext(aThebesSurface);

    return (CommonInit());
}

NS_IMETHODIMP
nsThebesRenderingContext::Init(nsIDeviceContext* aContext, gfxContext *aThebesContext)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::Init ctx %p thebesContext %p\n", this, aContext, aThebesContext));

    mDeviceContext = aContext;
    mWidget = nsnull;

    mThebes = aThebesContext;

    return (CommonInit());
}

NS_IMETHODIMP
nsThebesRenderingContext::Init(nsIDeviceContext* aContext, nsIWidget *aWidget)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::Init ctx %p widget %p\n", this, aContext, aWidget));

    nsThebesDeviceContext *thebesDC = static_cast<nsThebesDeviceContext*>(aContext);

    mDeviceContext = aContext;
    mWidget = aWidget;

    mThebes = new gfxContext(aWidget->GetThebesSurface());

    //mThebes->SetColor(gfxRGBA(0.9, 0.0, 0.0, 0.3));
    //mThebes->Paint();

    //mThebes->Translate(gfxPoint(300,0));
    //mThebes->Rotate(M_PI/4);

    return (CommonInit());
}

NS_IMETHODIMP
nsThebesRenderingContext::CommonInit(void)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::CommonInit\n", this));

    mThebes->SetLineWidth(1.0);

    mP2A = mDeviceContext->AppUnitsPerDevPixel();

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::GetDeviceContext(nsIDeviceContext *& aDeviceContext)
{
    aDeviceContext = mDeviceContext;
    NS_IF_ADDREF(aDeviceContext);
    return NS_OK;
}

NS_IMETHODIMP 
nsThebesRenderingContext::PushTranslation(PushedTranslation* aState)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::PushTranslation\n", this));

    // XXX this is slow!
    PushState();
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::PopTranslation(PushedTranslation* aState)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::PopTranslation\n", this));

    // XXX this is slow!
    PopState();
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::SetTranslation(nscoord aX, nscoord aY)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::SetTranslation %d %d\n", this, aX, aY));

    gfxMatrix newMat(mThebes->CurrentMatrix());
    newMat.x0 = aX;
    newMat.y0 = aY;
    mThebes->SetMatrix(newMat);
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::GetHints(PRUint32& aResult)
{
    aResult = 0;

    aResult |= (NS_RENDERING_HINT_BIDI_REORDERING |
                NS_RENDERING_HINT_ARABIC_SHAPING |
                NS_RENDERING_HINT_REORDER_SPACED_TEXT |
                NS_RENDERING_HINT_NEW_TEXT_RUNS);

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::PushState()
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::PushState\n", this));

    mThebes->Save();
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::PopState()
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::PopState\n", this));

    mThebes->Restore();
    return NS_OK;
}

//
// clipping
//

NS_IMETHODIMP
nsThebesRenderingContext::SetClipRect(const nsRect& aRect,
                                      nsClipCombine aCombine)
{
    //return NS_OK;
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::SetClipRect [%d,%d,%d,%d] %d\n", this, aRect.x, aRect.y, aRect.width, aRect.height, aCombine));

    if (aCombine == nsClipCombine_kReplace) {
        mThebes->ResetClip();
    } else if (aCombine != nsClipCombine_kIntersect) {
        NS_WARNING("Unexpected usage of SetClipRect");
    }

    mThebes->NewPath();
    gfxRect clipRect(GFX_RECT_FROM_TWIPS_RECT(aRect));
    if (mThebes->UserToDevicePixelSnapped(clipRect, PR_TRUE)) {
        gfxMatrix mat(mThebes->CurrentMatrix());
        mThebes->IdentityMatrix();
        mThebes->Rectangle(clipRect);
        mThebes->SetMatrix(mat);
    } else {
        mThebes->Rectangle(clipRect);
    }

    mThebes->Clip();

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::SetClipRegion(const nsIRegion& pxRegion,
                                        nsClipCombine aCombine)
{
    //return NS_OK;
    // Region is in device coords, no transformation.
    // This should only be called when there is no transform in place, when we
    // we just start painting a widget. The region is set by the platform paint
    // routine.
    NS_ASSERTION(aCombine == nsClipCombine_kReplace,
                 "Unexpected usage of SetClipRegion");

    nsRegionComplexity cplx;
    pxRegion.GetRegionComplexity(cplx);

    gfxMatrix mat = mThebes->CurrentMatrix();
    mThebes->IdentityMatrix();

    mThebes->ResetClip();
    // GetBoundingBox, GetRects, FreeRects are non-const
    nsIRegion *evilPxRegion = const_cast<nsIRegion*>(&pxRegion);
    if (cplx == eRegionComplexity_rect) {
        PRInt32 x, y, w, h;
        evilPxRegion->GetBoundingBox(&x, &y, &w, &h);

        PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::SetClipRegion %d [%d,%d,%d,%d]", this, cplx, x, y, w, h));

        mThebes->NewPath();
        mThebes->Rectangle(gfxRect(x, y, w, h), PR_TRUE);
        mThebes->Clip();
    } else if (cplx == eRegionComplexity_complex) {
        nsRegionRectSet *rects = nsnull;
        nsresult rv = evilPxRegion->GetRects (&rects);
        PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::SetClipRegion %d %d rects", this, cplx, rects->mNumRects));
        if (NS_FAILED(rv) || !rects) {
            mThebes->SetMatrix(mat);
            return rv;
        }

        mThebes->NewPath();
        for (PRUint32 i = 0; i < rects->mNumRects; i++) {
            mThebes->Rectangle(gfxRect(rects->mRects[i].x,
                                       rects->mRects[i].y,
                                       rects->mRects[i].width,
                                       rects->mRects[i].height),
                               PR_TRUE);
        }
        mThebes->Clip();

        evilPxRegion->FreeRects (rects);
    }

    mThebes->SetMatrix(mat);

    return NS_OK;
}
//
// other junk
//

NS_IMETHODIMP
nsThebesRenderingContext::SetLineStyle(nsLineStyle aLineStyle)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::SetLineStyle %d\n", this, aLineStyle));
    switch (aLineStyle) {
        case nsLineStyle_kSolid:
            mThebes->SetDash(gfxContext::gfxLineSolid);
            break;
        case nsLineStyle_kDashed:
            mThebes->SetDash(gfxContext::gfxLineDashed);
            break;
        case nsLineStyle_kDotted:
            mThebes->SetDash(gfxContext::gfxLineDotted);
            break;
        case nsLineStyle_kNone:
        default:
            // nothing uses kNone
            NS_ERROR("SetLineStyle: Invalid line style");
            break;
    }

    mLineStyle = aLineStyle;
    return NS_OK;
}


NS_IMETHODIMP
nsThebesRenderingContext::SetColor(nscolor aColor)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::SetColor 0x%08x\n", this, aColor));
    mThebes->SetColor(gfxRGBA(aColor));
    
    mColor = aColor;
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::GetColor(nscolor &aColor) const
{
    aColor = mColor;
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::Translate(nscoord aX, nscoord aY)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::Translate %d %d\n", this, aX, aY));
    mThebes->Translate (gfxPoint(FROM_TWIPS(aX), FROM_TWIPS(aY)));
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::Scale(float aSx, float aSy)
{
    // as far as I can tell, noone actually calls this
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::Scale %f %f\n", this, aSx, aSy));
    mThebes->Scale (aSx, aSy);
    return NS_OK;
}

void
nsThebesRenderingContext::UpdateTempTransformMatrix()
{
    //PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::UpdateTempTransformMatrix\n", this));

    /*****
     * Thebes matrix layout:   gfx matrix layout:
     * | xx yx 0 |            | m00 m01  0 |
     * | xy yy 0 |            | m10 m11  0 |
     * | x0 y0 1 |            | m20 m21  1 |
     *****/

    const gfxMatrix& ctm = mThebes->CurrentMatrix();
    NS_ASSERTION(ctm.yx == 0 && ctm.xy == 0, "Can't represent Thebes matrix to Gfx");
    mTempTransform.SetToTranslate(TO_TWIPS(ctm.x0), TO_TWIPS(ctm.y0));
    mTempTransform.AddScale(ctm.xx, ctm.yy);
}

nsTransform2D&
nsThebesRenderingContext::CurrentTransform()
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::CurrentTransform\n", this));
    UpdateTempTransformMatrix();
    return mTempTransform;
}

/****
 **** XXXXXX
 ****
 **** On other gfx implementations, the transform returned by this
 **** has a built in twips to pixels ratio.  That is, you pass in
 **** twips to any nsTransform2D TransformCoord method, and you
 **** get back pixels.  This makes no sense.  We don't do this.
 **** This in turn breaks SVG and <object>; those should just be
 **** fixed to not use this!
 ****/

NS_IMETHODIMP
nsThebesRenderingContext::GetCurrentTransform(nsTransform2D *&aTransform)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::GetCurrentTransform\n", this));
    UpdateTempTransformMatrix();
    aTransform = &mTempTransform;
    return NS_OK;
}

void
nsThebesRenderingContext::TransformCoord (nscoord *aX, nscoord *aY)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::TransformCoord\n", this));

    gfxPoint pt(FROM_TWIPS(*aX), FROM_TWIPS(*aY));

    pt = mThebes->UserToDevice (pt);

    *aX = TO_TWIPS(pt.x);
    *aY = TO_TWIPS(pt.y);
}

NS_IMETHODIMP
nsThebesRenderingContext::DrawLine(nscoord aX0, nscoord aY0, nscoord aX1, nscoord aY1)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::DrawLine %d %d %d %d\n", this, aX0, aY0, aX1, aY1));

    gfxPoint p0 = gfxPoint(FROM_TWIPS(aX0), FROM_TWIPS(aY0));
    gfxPoint p1 = gfxPoint(FROM_TWIPS(aX1), FROM_TWIPS(aY1));

    // we can't draw thick lines with gfx, so we always assume we want pixel-aligned
    // lines if the rendering context is at 1.0 scale
    gfxMatrix savedMatrix = mThebes->CurrentMatrix();
    if (!savedMatrix.HasNonTranslation()) {
        p0 = mThebes->UserToDevice(p0);
        p1 = mThebes->UserToDevice(p1);

        p0.Round();
        p1.Round();

        mThebes->IdentityMatrix();

        mThebes->NewPath();

        // snap straight lines
        if (p0.x == p1.x) {
            mThebes->Line(p0 + gfxPoint(0.5, 0),
                          p1 + gfxPoint(0.5, 0));
        } else if (p0.y == p1.y) {
            mThebes->Line(p0 + gfxPoint(0, 0.5),
                          p1 + gfxPoint(0, 0.5));
        } else {
            mThebes->Line(p0, p1);
        }

        mThebes->Stroke();

        mThebes->SetMatrix(savedMatrix);
    } else {
        mThebes->NewPath();
        mThebes->Line(p0, p1);
        mThebes->Stroke();
    }

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::DrawRect(const nsRect& aRect)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::DrawRect [%d,%d,%d,%d]\n", this, aRect.x, aRect.y, aRect.width, aRect.height));

    mThebes->NewPath();
    mThebes->Rectangle(GFX_RECT_FROM_TWIPS_RECT(aRect), PR_TRUE);
    mThebes->Stroke();

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::DrawRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
    DrawRect(nsRect(aX, aY, aWidth, aHeight));
    return NS_OK;
}


/* Clamp r to (0,0) (2^23,2^23)
 * these are to be device coordinates.
 *
 * Returns PR_FALSE if the rectangle is completely out of bounds,
 * PR_TRUE otherwise.
 *
 * This function assumes that it will be called with a rectangle being
 * drawn into a surface with an identity transformation matrix; that
 * is, anything above or to the left of (0,0) will be offscreen.
 *
 * First it checks if the rectangle is entirely beyond
 * CAIRO_COORD_MAX; if so, it can't ever appear on the screen --
 * PR_FALSE is returned.
 *
 * Then it shifts any rectangles with x/y < 0 so that x and y are = 0,
 * and adjusts the width and height appropriately.  For example, a
 * rectangle from (0,-5) with dimensions (5,10) will become a
 * rectangle from (0,0) with dimensions (5,5).
 *
 * If after negative x/y adjustment to 0, either the width or height
 * is negative, then the rectangle is completely offscreen, and
 * nothing is drawn -- PR_FALSE is returned.
 *
 * Finally, if x+width or y+height are greater than CAIRO_COORD_MAX,
 * the width and height are clamped such x+width or y+height are equal
 * to CAIRO_COORD_MAX, and PR_TRUE is returned.
 */
#define CAIRO_COORD_MAX (8388608.0)

static PRBool
ConditionRect(gfxRect& r) {
    // if either x or y is way out of bounds;
    // note that we don't handle negative w/h here
    if (r.pos.x > CAIRO_COORD_MAX || r.pos.y > CAIRO_COORD_MAX)
        return PR_FALSE;

    if (r.pos.x < 0.0) {
        r.size.width += r.pos.x;
        if (r.size.width < 0.0)
            return PR_FALSE;
        r.pos.x = 0.0;
    }

    if (r.pos.x + r.size.width > CAIRO_COORD_MAX) {
        r.size.width = CAIRO_COORD_MAX - r.pos.x;
    }

    if (r.pos.y < 0.0) {
        r.size.height += r.pos.y;
        if (r.size.height < 0.0)
            return PR_FALSE;

        r.pos.y = 0.0;
    }

    if (r.pos.y + r.size.height > CAIRO_COORD_MAX) {
        r.size.height = CAIRO_COORD_MAX - r.pos.y;
    }
    return PR_TRUE;
}

NS_IMETHODIMP
nsThebesRenderingContext::FillRect(const nsRect& aRect)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::FillRect [%d,%d,%d,%d]\n", this, aRect.x, aRect.y, aRect.width, aRect.height));

    gfxRect r(GFX_RECT_FROM_TWIPS_RECT(aRect));

    /* Clamp coordinates to work around a design bug in cairo */
    nscoord bigval = (nscoord)(CAIRO_COORD_MAX*mP2A);
    if (aRect.width > bigval ||
        aRect.height > bigval ||
        aRect.x < -bigval ||
        aRect.x > bigval ||
        aRect.y < -bigval ||
        aRect.y > bigval)
    {
        gfxMatrix mat = mThebes->CurrentMatrix();

        r = mat.Transform(r);

        if (!ConditionRect(r))
            return NS_OK;

        mThebes->IdentityMatrix();
        mThebes->NewPath();

        PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::FillRect conditioned to [%f,%f,%f,%f]\n", this, r.pos.x, r.pos.y, r.size.width, r.size.height));

        mThebes->Rectangle(r, PR_TRUE);
        mThebes->Fill();
        mThebes->SetMatrix(mat);

        return NS_OK;
    }

    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::FillRect raw [%f,%f,%f,%f]\n", this, r.pos.x, r.pos.y, r.size.width, r.size.height));

    mThebes->NewPath();
    mThebes->Rectangle(r, PR_TRUE);
    mThebes->Fill();

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::FillRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
    FillRect(nsRect(aX, aY, aWidth, aHeight));
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::InvertRect(const nsRect& aRect)
{
    gfxContext::GraphicsOperator lastOp = mThebes->CurrentOperator();

    mThebes->SetOperator(gfxContext::OPERATOR_XOR);
    nsresult rv = FillRect(aRect);
    mThebes->SetOperator(lastOp);

    return rv;
}

NS_IMETHODIMP
nsThebesRenderingContext::InvertRect(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
    return InvertRect(nsRect(aX, aY, aWidth, aHeight));
}

NS_IMETHODIMP
nsThebesRenderingContext::DrawEllipse(const nsRect& aRect)
{
    return DrawEllipse(aRect.x, aRect.y, aRect.width, aRect.height);
}

NS_IMETHODIMP
nsThebesRenderingContext::DrawEllipse(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::DrawEllipse [%d,%d,%d,%d]\n", this, aX, aY, aWidth, aHeight));

    mThebes->NewPath();
    mThebes->Ellipse(gfxPoint(FROM_TWIPS(aX) + FROM_TWIPS(aWidth)/2.0,
                              FROM_TWIPS(aY) + FROM_TWIPS(aHeight)/2.0),
                     gfxSize(FROM_TWIPS(aWidth),
                             FROM_TWIPS(aHeight)));
    mThebes->Stroke();

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::FillEllipse(const nsRect& aRect)
{
    return FillEllipse(aRect.x, aRect.y, aRect.width, aRect.height);
}

NS_IMETHODIMP
nsThebesRenderingContext::FillEllipse(nscoord aX, nscoord aY, nscoord aWidth, nscoord aHeight)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::FillEllipse [%d,%d,%d,%d]\n", this, aX, aY, aWidth, aHeight));

    mThebes->NewPath();
    mThebes->Ellipse(gfxPoint(FROM_TWIPS(aX) + FROM_TWIPS(aWidth)/2.0,
                              FROM_TWIPS(aY) + FROM_TWIPS(aHeight)/2.0),
                     gfxSize(FROM_TWIPS(aWidth),
                             FROM_TWIPS(aHeight)));
    mThebes->Fill();

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::FillPolygon(const nsPoint twPoints[], PRInt32 aNumPoints)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::FillPolygon %d\n", this, aNumPoints));

    if (aNumPoints == 0)
        return NS_OK;

    if (aNumPoints == 4) {
    }

    nsAutoArrayPtr<gfxPoint> pxPoints(new gfxPoint[aNumPoints]);

    for (int i = 0; i < aNumPoints; i++) {
        pxPoints[i].x = FROM_TWIPS(twPoints[i].x);
        pxPoints[i].y = FROM_TWIPS(twPoints[i].y);
    }

    mThebes->NewPath();
    mThebes->Polygon(pxPoints, aNumPoints);
    mThebes->Fill();

    return NS_OK;
}

void*
nsThebesRenderingContext::GetNativeGraphicData(GraphicDataType aType)
{
    if (aType == NATIVE_GDK_DRAWABLE &&
        !gfxPlatform::GetPlatform()->UseGlitz())
    {
        if (mWidget)
            return mWidget->GetNativeData(NS_NATIVE_WIDGET);
    }
    if (aType == NATIVE_THEBES_CONTEXT)
        return mThebes;
    if (aType == NATIVE_CAIRO_CONTEXT)
        return mThebes->GetCairo();
#ifdef XP_WIN
    if (aType == NATIVE_WINDOWS_DC) {
        nsRefPtr<gfxASurface> surf(mThebes->CurrentSurface());
        if (!surf || surf->CairoStatus())
            return nsnull;
        return static_cast<gfxWindowsSurface*>(static_cast<gfxASurface*>(surf.get()))->GetDC();
    }
#endif

    return nsnull;
}

NS_IMETHODIMP
nsThebesRenderingContext::PushFilter(const nsRect& twRect, PRBool aAreaIsOpaque, float aOpacity)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG,
           ("## %p nsTRC::PushFilter [%d,%d,%d,%d] isOpaque: %d opacity: %f\n",
            this, twRect.x, twRect.y, twRect.width, twRect.height,
            aAreaIsOpaque, aOpacity));

    mOpacityArray.AppendElement(aOpacity);

    mThebes->Save();
    mThebes->Clip(GFX_RECT_FROM_TWIPS_RECT(twRect));
    mThebes->PushGroup(gfxASurface::CONTENT_COLOR_ALPHA);

    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::PopFilter()
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::PopFilter\n"));

    if (mOpacityArray.Length() > 0) {
        float f = mOpacityArray[mOpacityArray.Length()-1];
        mOpacityArray.RemoveElementAt(mOpacityArray.Length()-1);

        mThebes->PopGroupToSource();

        if (f < 0.0) {
            mThebes->SetOperator(gfxContext::OPERATOR_SOURCE);
            mThebes->Paint();
        } else {
            mThebes->SetOperator(gfxContext::OPERATOR_OVER);
            mThebes->Paint(f);
        }

        mThebes->Restore();
    }


    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::DrawTile(imgIContainer *aImage,
                                   nscoord twXOffset, nscoord twYOffset,
                                   const nsRect *twTargetRect)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::DrawTile %p %f %f [%f,%f,%f,%f]\n",
                                         this, aImage, FROM_TWIPS(twXOffset), FROM_TWIPS(twYOffset),
                                         FROM_TWIPS(twTargetRect->x), FROM_TWIPS(twTargetRect->y),
                                         FROM_TWIPS(twTargetRect->width), FROM_TWIPS(twTargetRect->height)));

    nscoord containerWidth, containerHeight;
    aImage->GetWidth(&containerWidth);
    aImage->GetHeight(&containerHeight);

    nsCOMPtr<gfxIImageFrame> imgFrame;
    aImage->GetCurrentFrame(getter_AddRefs(imgFrame));
    if (!imgFrame) return NS_ERROR_FAILURE;

    nsRect imgFrameRect;
    imgFrame->GetRect(imgFrameRect);

    nsCOMPtr<nsIImage> img(do_GetInterface(imgFrame));
    if (!img) return NS_ERROR_FAILURE;
    
    nsThebesImage *thebesImage = static_cast<nsThebesImage*>((nsIImage*) img.get());

    /* Phase offset of the repeated image from the origin */
    gfxPoint phase(FROM_TWIPS(twXOffset), FROM_TWIPS(twYOffset));

    /* The image may be smaller than the container (bug 113561),
     * so we need to make sure that there is the right amount of padding
     * in between each tile of the nsIImage.  This problem goes away
     * when we change the way the GIF decoder works to have it store
     * full frames that are ready to be composited.
     */
    PRInt32 xPadding = 0;
    PRInt32 yPadding = 0;

    if (imgFrameRect.width != containerWidth ||
        imgFrameRect.height != containerHeight)
    {
        xPadding = containerWidth - imgFrameRect.width;
        yPadding = containerHeight - imgFrameRect.height;

        phase.x -= imgFrameRect.x;
        phase.y -= imgFrameRect.y;
    }

    return thebesImage->ThebesDrawTile (mThebes, mDeviceContext, phase,
                                        GFX_RECT_FROM_TWIPS_RECT(*twTargetRect),
                                        xPadding, yPadding);
}

//
// text junk
//
NS_IMETHODIMP
nsThebesRenderingContext::SetRightToLeftText(PRBool aIsRTL)
{
    return mFontMetrics->SetRightToLeftText(aIsRTL);
}

NS_IMETHODIMP
nsThebesRenderingContext::GetRightToLeftText(PRBool* aIsRTL)
{
    *aIsRTL = mFontMetrics->GetRightToLeftText();
    return NS_OK;
}

void
nsThebesRenderingContext::SetTextRunRTL(PRBool aIsRTL)
{
	mFontMetrics->SetTextRunRTL(aIsRTL);
}

NS_IMETHODIMP
nsThebesRenderingContext::SetFont(const nsFont& aFont, nsIAtom* aLangGroup)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::SetFont %p\n", this, &aFont));

    nsCOMPtr<nsIFontMetrics> newMetrics;
    mDeviceContext->GetMetricsFor(aFont, aLangGroup, *getter_AddRefs(newMetrics));
    mFontMetrics = reinterpret_cast<nsIThebesFontMetrics*>(newMetrics.get());
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::SetFont(nsIFontMetrics *aFontMetrics)
{
    PR_LOG(gThebesGFXLog, PR_LOG_DEBUG, ("## %p nsTRC::SetFont[Metrics] %p\n", this, aFontMetrics));

    mFontMetrics = static_cast<nsIThebesFontMetrics*>(aFontMetrics);
    return NS_OK;
}

NS_IMETHODIMP
nsThebesRenderingContext::GetFontMetrics(nsIFontMetrics *&aFontMetrics)
{
    aFontMetrics = mFontMetrics;
    NS_IF_ADDREF(aFontMetrics);
    return NS_OK;
}

PRInt32
nsThebesRenderingContext::GetMaxStringLength()
{
  if (!mFontMetrics)
    return 1;
  return mFontMetrics->GetMaxStringLength();
}

NS_IMETHODIMP
nsThebesRenderingContext::GetWidth(char aC, nscoord &aWidth)
{
    if (aC == ' ' && mFontMetrics)
        return mFontMetrics->GetSpaceWidth(aWidth);

    return GetWidth(&aC, 1, aWidth);
}

NS_IMETHODIMP
nsThebesRenderingContext::GetWidth(PRUnichar aC, nscoord &aWidth, PRInt32 *aFontID)
{
    return GetWidth(&aC, 1, aWidth, aFontID);
}

NS_IMETHODIMP
nsThebesRenderingContext::GetWidthInternal(const char* aString, PRUint32 aLength, nscoord& aWidth)
{
#ifdef DISABLE_TEXT
    aWidth = (8 * aLength);
    return NS_OK;
#endif

    if (aLength == 0) {
        aWidth = 0;
        return NS_OK;
    }

    return mFontMetrics->GetWidth(aString, aLength, aWidth, this);
}

NS_IMETHODIMP
nsThebesRenderingContext::GetWidthInternal(const PRUnichar *aString, PRUint32 aLength,
                                           nscoord &aWidth, PRInt32 *aFontID)
{
#ifdef DISABLE_TEXT
    aWidth = (8 * aLength);
    return NS_OK;
#endif

    if (aLength == 0) {
        aWidth = 0;
        return NS_OK;
    }

    return mFontMetrics->GetWidth(aString, aLength, aWidth, aFontID, this);
}

NS_IMETHODIMP
nsThebesRenderingContext::GetTextDimensionsInternal(const char* aString, PRUint32 aLength,
                                                    nsTextDimensions& aDimensions)
{
  mFontMetrics->GetMaxAscent(aDimensions.ascent);
  mFontMetrics->GetMaxDescent(aDimensions.descent);
  return GetWidth(aString, aLength, aDimensions.width);
}

NS_IMETHODIMP
nsThebesRenderingContext::GetTextDimensionsInternal(const PRUnichar* aString,
                                                    PRUint32 aLength,
                                                    nsTextDimensions& aDimensions,
                                                    PRInt32* aFontID)
{
  mFontMetrics->GetMaxAscent(aDimensions.ascent);
  mFontMetrics->GetMaxDescent(aDimensions.descent);
  return GetWidth(aString, aLength, aDimensions.width, aFontID);
}

#if defined(_WIN32) || defined(XP_OS2) || defined(MOZ_X11) || defined(XP_BEOS) || defined(XP_MACOSX)
NS_IMETHODIMP
nsThebesRenderingContext::GetTextDimensionsInternal(const char*       aString,
                                                    PRInt32           aLength,
                                                    PRInt32           aAvailWidth,
                                                    PRInt32*          aBreaks,
                                                    PRInt32           aNumBreaks,
                                                    nsTextDimensions& aDimensions,
                                                    PRInt32&          aNumCharsFit,
                                                    nsTextDimensions& aLastWordDimensions,
                                                    PRInt32*          aFontID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsThebesRenderingContext::GetTextDimensionsInternal(const PRUnichar*  aString,
                                                    PRInt32           aLength,
                                                    PRInt32           aAvailWidth,
                                                    PRInt32*          aBreaks,
                                                    PRInt32           aNumBreaks,
                                                    nsTextDimensions& aDimensions,
                                                    PRInt32&          aNumCharsFit,
                                                    nsTextDimensions& aLastWordDimensions,
                                                    PRInt32*          aFontID)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
#endif

#ifdef MOZ_MATHML
NS_IMETHODIMP 
nsThebesRenderingContext::GetBoundingMetricsInternal(const char*        aString,
                                                     PRUint32           aLength,
                                                     nsBoundingMetrics& aBoundingMetrics)
{
    return mFontMetrics->GetBoundingMetrics(aString, aLength, this, aBoundingMetrics);
}

NS_IMETHODIMP
nsThebesRenderingContext::GetBoundingMetricsInternal(const PRUnichar*   aString,
                                                     PRUint32           aLength,
                                                     nsBoundingMetrics& aBoundingMetrics,
                                                     PRInt32*           aFontID)
{
    return mFontMetrics->GetBoundingMetrics(aString, aLength, this, aBoundingMetrics);
}
#endif // MOZ_MATHML

NS_IMETHODIMP
nsThebesRenderingContext::DrawStringInternal(const char *aString, PRUint32 aLength,
                                             nscoord aX, nscoord aY,
                                             const nscoord* aSpacing)
{
#ifdef DISABLE_TEXT
    return NS_OK;
#endif

    return mFontMetrics->DrawString(aString, aLength, aX, aY, aSpacing,
                                    this);
}

NS_IMETHODIMP
nsThebesRenderingContext::DrawStringInternal(const PRUnichar *aString, PRUint32 aLength,
                                             nscoord aX, nscoord aY,
                                             PRInt32 aFontID,
                                             const nscoord* aSpacing)
{
#ifdef DISABLE_TEXT
    return NS_OK;
#endif

    return mFontMetrics->DrawString(aString, aLength, aX, aY, aFontID,
                                    aSpacing, this);
}

NS_IMETHODIMP
nsThebesRenderingContext::GetClusterInfo(const PRUnichar *aText,
                                         PRUint32 aLength,
                                         PRUint8 *aClusterStarts)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

PRInt32
nsThebesRenderingContext::GetPosition(const PRUnichar *aText,
                                      PRUint32 aLength,
                                      nsPoint aPt)
{
  return -1;
}

NS_IMETHODIMP
nsThebesRenderingContext::GetRangeWidth(const PRUnichar *aText,
                                        PRUint32 aLength,
                                        PRUint32 aStart,
                                        PRUint32 aEnd,
                                        PRUint32 &aWidth)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsThebesRenderingContext::GetRangeWidth(const char *aText,
                                        PRUint32 aLength,
                                        PRUint32 aStart,
                                        PRUint32 aEnd,
                                        PRUint32 &aWidth)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsThebesRenderingContext::RenderEPS(const nsRect& aRect, FILE *aDataFile)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

