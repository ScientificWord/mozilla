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
 * The Original Code is thebes gfx
 *
 * The Initial Developer of the Original Code is
 * mozilla.org.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#include "nsThebesImage.h"
#include "nsThebesRenderingContext.h"

#include "gfxContext.h"
#include "gfxPattern.h"

#include "gfxPlatform.h"

#include "prenv.h"

static PRBool gDisableOptimize = PR_FALSE;

#ifdef XP_WIN
static PRUint32 gTotalDDBs = 0;
static PRUint32 gTotalDDBSize = 0;
// only use up a maximum of 64MB in DDBs
#define kMaxDDBSize (64*1024*1024)
// and don't let anything in that's bigger than 4MB
#define kMaxSingleDDBSize (4*1024*1024)
#endif

NS_IMPL_ISUPPORTS1(nsThebesImage, nsIImage)

nsThebesImage::nsThebesImage()
    : mFormat(gfxImageSurface::ImageFormatRGB24),
      mWidth(0),
      mHeight(0),
      mDecoded(0,0,0,0),
      mImageComplete(PR_FALSE),
      mSinglePixel(PR_FALSE),
      mFormatChanged(PR_FALSE),
      mAlphaDepth(0)
{
    static PRBool hasCheckedOptimize = PR_FALSE;
    if (!hasCheckedOptimize) {
        if (PR_GetEnv("MOZ_DISABLE_IMAGE_OPTIMIZE")) {
            gDisableOptimize = PR_TRUE;
        }
        hasCheckedOptimize = PR_TRUE;
    }

#ifdef XP_WIN
    mIsDDBSurface = PR_FALSE;
#endif
}

nsresult
nsThebesImage::Init(PRInt32 aWidth, PRInt32 aHeight, PRInt32 aDepth, nsMaskRequirements aMaskRequirements)
{
    mWidth = aWidth;
    mHeight = aHeight;

    // Reject over-wide or over-tall images.
    if (!AllowedImageSize(aWidth, aHeight))
        return NS_ERROR_FAILURE;

    gfxImageSurface::gfxImageFormat format;
    switch(aMaskRequirements)
    {
        case nsMaskRequirements_kNeeds1Bit:
            format = gfxImageSurface::ImageFormatARGB32;
            mAlphaDepth = 1;
            break;
        case nsMaskRequirements_kNeeds8Bit:
            format = gfxImageSurface::ImageFormatARGB32;
            mAlphaDepth = 8;
            break;
        default:
            format = gfxImageSurface::ImageFormatRGB24;
            mAlphaDepth = 0;
            break;
    }

    mFormat = format;

#ifdef XP_WIN
    if (!ShouldUseImageSurfaces()) {
        mWinSurface = new gfxWindowsSurface(gfxIntSize(mWidth, mHeight), format);
        if (mWinSurface && mWinSurface->CairoStatus() == 0) {
            // no error
            mImageSurface = mWinSurface->GetImageSurface();
        }
    }

    if (!mImageSurface)
        mWinSurface = nsnull;
#endif

    if (!mImageSurface)
        mImageSurface = new gfxImageSurface(gfxIntSize(mWidth, mHeight), format);

    if (!mImageSurface || mImageSurface->CairoStatus()) {
        mImageSurface = nsnull;
        // guess
        return NS_ERROR_OUT_OF_MEMORY;
    }

#ifdef XP_MACOSX
    mQuartzSurface = new gfxQuartzImageSurface(mImageSurface);
#endif

    mStride = mImageSurface->Stride();

    return NS_OK;
}

nsThebesImage::~nsThebesImage()
{
#ifdef XP_WIN
    if (mIsDDBSurface) {
        gTotalDDBs--;
        gTotalDDBSize -= mWidth*mHeight*4;
    }
#endif
}

PRInt32
nsThebesImage::GetBytesPix()
{
    return 4;
}

PRBool
nsThebesImage::GetIsRowOrderTopToBottom()
{
    return PR_TRUE;
}

PRInt32
nsThebesImage::GetWidth()
{
    return mWidth;
}

PRInt32
nsThebesImage::GetHeight()
{
    return mHeight;
}

PRUint8 *
nsThebesImage::GetBits()
{
    if (mImageSurface)
        return mImageSurface->Data();
    return nsnull;
}

PRInt32
nsThebesImage::GetLineStride()
{
    return mStride;
}

PRBool
nsThebesImage::GetHasAlphaMask()
{
    return mAlphaDepth > 0;
}

PRUint8 *
nsThebesImage::GetAlphaBits()
{
    return nsnull;
}

PRInt32
nsThebesImage::GetAlphaLineStride()
{
    return (mAlphaDepth > 0) ? mStride : 0;
}

void
nsThebesImage::ImageUpdated(nsIDeviceContext *aContext, PRUint8 aFlags, nsRect *aUpdateRect)
{
    mDecoded.UnionRect(mDecoded, *aUpdateRect);
#ifdef XP_MACOSX
    if (mQuartzSurface)
        mQuartzSurface->Flush();
#endif
}

PRBool
nsThebesImage::GetIsImageComplete()
{
    if (!mImageComplete)
        mImageComplete = (mDecoded == nsRect(0, 0, mWidth, mHeight));
    return mImageComplete;
}

nsresult
nsThebesImage::Optimize(nsIDeviceContext* aContext)
{
    if (gDisableOptimize)
        return NS_OK;

    if (mOptSurface || mSinglePixel)
        return NS_OK;

    /* Figure out if the entire image is a constant color */

    // this should always be true
    if (mStride == mWidth * 4) {
        PRUint32 *imgData = (PRUint32*) mImageSurface->Data();
        PRUint32 firstPixel = * (PRUint32*) imgData;
        PRUint32 pixelCount = mWidth * mHeight + 1;

        while (--pixelCount && *imgData++ == firstPixel)
            ;

        if (pixelCount == 0) {
            // all pixels were the same
            if (mFormat == gfxImageSurface::ImageFormatARGB32 ||
                mFormat == gfxImageSurface::ImageFormatRGB24)
            {
                mSinglePixelColor = gfxRGBA
                    (firstPixel,
                     (mFormat == gfxImageSurface::ImageFormatRGB24 ?
                      gfxRGBA::PACKED_XRGB :
                      gfxRGBA::PACKED_ARGB_PREMULTIPLIED));

                mSinglePixel = PR_TRUE;

                // XXX we can't do this until we either teach anyone
                // who calls GetSurface() about single-color stuff,
                // or until we make GetSurface() create a new temporary
                // surface to return (and that callers understand that
                // modifying that surface won't modify the image).
                // Current users are drag & drop and clipboard.
#if 0
                // blow away the older surfaces, to release data

                mImageSurface = nsnull;
                mOptSurface = nsnull;
#ifdef XP_WIN
                mWinSurface = nsnull;
#endif
#ifdef XP_MACOSX
                mQuartzSurface = nsnull;
#endif
#endif
                return NS_OK;
            }
        }

        // if it's not RGB24/ARGB32, don't optimize, but we never hit this at the moment
    }

    // if we're being forced to use image surfaces due to
    // resource constraints, don't try to optimize beyond same-pixel.
    if (ShouldUseImageSurfaces())
        return NS_OK;

    mOptSurface = nsnull;

#ifdef XP_WIN
    // we need to special-case windows here, because windows has
    // a distinction between DIB and DDB and we want to use DDBs as much
    // as we can.
    if (mWinSurface) {
        // Don't do DDBs for large images; see bug 359147
        // Note that we bother with DDBs at all because they are much faster
        // on some systems; on others there isn't much of a speed difference
        // between DIBs and DDBs.
        //
        // Originally this just limited to 1024x1024; but that still
        // had us hitting overall total memory usage limits (which was
        // around 220MB on my intel shared memory system with 2GB RAM
        // and 16-128mb in use by the video card, so I can't make
        // heads or tails out of this limit).
        //
        // So instead, we clamp the max size to 64MB (this limit shuld
        // be made dynamic based on.. something.. as soon a we figure
        // out that something) and also limit each individual image to
        // be less than 4MB to keep very large images out of DDBs.

        // assume (almost -- we don't quadword-align) worst-case size
        PRUint32 ddbSize = mWidth * mHeight * 4;
        if (ddbSize <= kMaxSingleDDBSize &&
            ddbSize + gTotalDDBSize <= kMaxDDBSize)
        {
            nsRefPtr<gfxWindowsSurface> wsurf = mWinSurface->OptimizeToDDB(nsnull, gfxIntSize(mWidth, mHeight), mFormat);
            if (wsurf) {
                gTotalDDBs++;
                gTotalDDBSize += ddbSize;
                mIsDDBSurface = PR_TRUE;
                mOptSurface = wsurf;
            }
        }
        if (!mOptSurface && !mFormatChanged) {
            // just use the DIB if the format has not changed
            mOptSurface = mWinSurface;
        }
    }
#endif

#ifdef XP_MACOSX
    if (mQuartzSurface) {
        mQuartzSurface->Flush();
        mOptSurface = mQuartzSurface;
    }
#endif

    if (mOptSurface == nsnull)
        mOptSurface = gfxPlatform::GetPlatform()->OptimizeImage(mImageSurface, mFormat);

    if (mOptSurface) {
        mImageSurface = nsnull;
#ifdef XP_WIN
        mWinSurface = nsnull;
#endif
#ifdef XP_MACOSX
        mQuartzSurface = nsnull;
#endif
    }

    return NS_OK;
}

nsColorMap *
nsThebesImage::GetColorMap()
{
    return NULL;
}

PRInt8
nsThebesImage::GetAlphaDepth()
{
    return mAlphaDepth;
}

void *
nsThebesImage::GetBitInfo()
{
    return NULL;
}

NS_IMETHODIMP
nsThebesImage::LockImagePixels(PRBool aMaskPixels)
{
    if (aMaskPixels)
        return NS_ERROR_NOT_IMPLEMENTED;
    if ((mOptSurface || mSinglePixel) && !mImageSurface) {
        // Recover the pixels
        mImageSurface = new gfxImageSurface(gfxIntSize(mWidth, mHeight),
                                            gfxImageSurface::ImageFormatARGB32);
        if (!mImageSurface || mImageSurface->CairoStatus())
            return NS_ERROR_OUT_OF_MEMORY;
        gfxContext context(mImageSurface);
        context.SetOperator(gfxContext::OPERATOR_SOURCE);
        if (mSinglePixel)
            context.SetColor(mSinglePixelColor);
        else
            context.SetSource(mOptSurface);
        context.Paint();
    }
    return NS_OK;
}

NS_IMETHODIMP
nsThebesImage::UnlockImagePixels(PRBool aMaskPixels)
{
    if (aMaskPixels)
        return NS_ERROR_NOT_IMPLEMENTED;
    mOptSurface = nsnull;
#ifdef XP_MACOSX
    if (mQuartzSurface)
        mQuartzSurface->Flush();
#endif
    return NS_OK;
}

/* NB: These are pixels, not twips. */
NS_IMETHODIMP
nsThebesImage::Draw(nsIRenderingContext &aContext,
                    const gfxRect &aSourceRect,
                    const gfxRect &aDestRect)
{
    if (NS_UNLIKELY(aDestRect.IsEmpty())) {
        NS_ERROR("nsThebesImage::Draw zero dest size - please fix caller.");
        return NS_OK;
    }

    nsThebesRenderingContext *thebesRC = static_cast<nsThebesRenderingContext*>(&aContext);
    gfxContext *ctx = thebesRC->ThebesContext();

#if 0
    fprintf (stderr, "nsThebesImage::Draw src [%f %f %f %f] dest [%f %f %f %f] trans: [%f %f] dec: [%f %f]\n",
             aSourceRect.pos.x, aSourceRect.pos.y, aSourceRect.size.width, aSourceRect.size.height,
             aDestRect.pos.x, aDestRect.pos.y, aDestRect.size.width, aDestRect.size.height,
             ctx->CurrentMatrix().GetTranslation().x, ctx->CurrentMatrix().GetTranslation().y,
             mDecoded.x, mDecoded.y, mDecoded.width, mDecoded.height);
#endif

    if (mSinglePixel) {
        // if a == 0, it's a noop
        if (mSinglePixelColor.a == 0.0)
            return NS_OK;

        // otherwise
        gfxContext::GraphicsOperator op = ctx->CurrentOperator();
        if (op == gfxContext::OPERATOR_OVER && mSinglePixelColor.a == 1.0)
            ctx->SetOperator(gfxContext::OPERATOR_SOURCE);

        ctx->SetColor(mSinglePixelColor);
        ctx->NewPath();
        ctx->Rectangle(aDestRect, PR_TRUE);
        ctx->Fill();
        ctx->SetOperator(op);
        return NS_OK;
    }

    gfxFloat xscale = aDestRect.size.width / aSourceRect.size.width;
    gfxFloat yscale = aDestRect.size.height / aSourceRect.size.height;

    gfxRect srcRect(aSourceRect);
    gfxRect destRect(aDestRect);

    if (!GetIsImageComplete()) {
      srcRect = srcRect.Intersect(gfxRect(mDecoded.x, mDecoded.y,
                                          mDecoded.width, mDecoded.height));

      // This happens when mDecoded.width or height is zero. bug 368427.
      if (NS_UNLIKELY(srcRect.size.width == 0 || srcRect.size.height == 0))
          return NS_OK;

      destRect.pos.x += (srcRect.pos.x - aSourceRect.pos.x)*xscale;
      destRect.pos.y += (srcRect.pos.y - aSourceRect.pos.y)*yscale;

      destRect.size.width  = srcRect.size.width * xscale;
      destRect.size.height = srcRect.size.height * yscale;
    }

    // if either rectangle is empty now (possibly after the image complete check)
    if (srcRect.IsEmpty() || destRect.IsEmpty())
        return NS_OK;

    // Reject over-wide or over-tall images.
    if (!AllowedImageSize(destRect.size.width + 1, destRect.size.height + 1))
        return NS_ERROR_FAILURE;

    nsRefPtr<gfxPattern> pat;

    /* See bug 364968 to understand the necessity of this goop; we basically
     * have to pre-downscale any image that would fall outside of a scaled 16-bit
     * coordinate space.
     */
    if (aDestRect.pos.x * (1.0 / xscale) >= 32768.0 ||
        aDestRect.pos.y * (1.0 / yscale) >= 32768.0)
    {
        gfxIntSize dim(NS_lroundf(destRect.size.width),
                       NS_lroundf(destRect.size.height));

        // nothing to do in this case
        if (dim.width == 0 || dim.height == 0)
            return NS_OK;

        nsRefPtr<gfxASurface> temp =
            gfxPlatform::GetPlatform()->CreateOffscreenSurface (dim,  mFormat);
        if (!temp || temp->CairoStatus() != 0)
            return NS_ERROR_FAILURE;

        gfxContext tempctx(temp);

        gfxPattern srcpat(ThebesSurface());
        gfxMatrix mat;
        mat.Translate(srcRect.pos);
        mat.Scale(1.0 / xscale, 1.0 / yscale);
        srcpat.SetMatrix(mat);

        tempctx.SetPattern(&srcpat);
        tempctx.SetOperator(gfxContext::OPERATOR_SOURCE);
        tempctx.NewPath();
        tempctx.Rectangle(gfxRect(0.0, 0.0, dim.width, dim.height));
        tempctx.Fill();

        pat = new gfxPattern(temp);

        srcRect.pos.x = 0.0;
        srcRect.pos.y = 0.0;
        srcRect.size.width = dim.width;
        srcRect.size.height = dim.height;

        xscale = 1.0;
        yscale = 1.0;
    }

    if (!pat) {
        pat = new gfxPattern(ThebesSurface());
    }

    gfxMatrix mat;
    mat.Translate(srcRect.pos);
    mat.Scale(1.0/xscale, 1.0/yscale);

    /* Translate the start point of the image (srcRect.pos)
     * to coincide with the destination rectangle origin
     */
    mat.Translate(-destRect.pos);

    pat->SetMatrix(mat);

#if !defined(XP_MACOSX) && !defined(XP_WIN)
    // See bug 324698.  This is a workaround.
    //
    // Set the filter to CAIRO_FILTER_FAST if we're scaling up -- otherwise,
    // pixman's sampling will sample transparency for the outside edges and we'll
    // get blurry edges.  CAIRO_EXTEND_PAD would also work here, if
    // available
    //
    // This effectively disables smooth upscaling for images.
    if (xscale > 1.0 || yscale > 1.0)
        pat->SetFilter(0);
#endif

#if defined(XP_WIN)
    // turn on EXTEND_PAD only for win32, and only when scaling;
    // it's not implemented correctly on linux in the X server.
    if (xscale != 1.0 || yscale != 1.0)
        pat->SetExtend(gfxPattern::EXTEND_PAD);
#endif

    gfxContext::GraphicsOperator op = ctx->CurrentOperator();
    if (op == gfxContext::OPERATOR_OVER && mFormat == gfxASurface::ImageFormatRGB24)
        ctx->SetOperator(gfxContext::OPERATOR_SOURCE);

    ctx->NewPath();
    ctx->SetPattern(pat);
    ctx->Rectangle(destRect);
    ctx->Fill();

    ctx->SetOperator(op);
    ctx->SetColor(gfxRGBA(0,0,0,0));

    return NS_OK;
}

nsresult
nsThebesImage::ThebesDrawTile(gfxContext *thebesContext,
                              nsIDeviceContext* dx,
                              const gfxPoint& offset,
                              const gfxRect& targetRect,
                              const PRInt32 xPadding,
                              const PRInt32 yPadding)
{
    NS_ASSERTION(xPadding >= 0 && yPadding >= 0, "negative padding");

    if (targetRect.size.width <= 0.0 || targetRect.size.height <= 0.0)
        return NS_OK;

    // don't do anything if we have a transparent pixel source
    if (mSinglePixel && mSinglePixelColor.a == 0.0)
        return NS_OK;

    PRBool doSnap = !(thebesContext->CurrentMatrix().HasNonTranslation());
    PRBool hasPadding = ((xPadding != 0) || (yPadding != 0));

    nsRefPtr<gfxASurface> tmpSurfaceGrip;

    if (mSinglePixel && !hasPadding) {
        thebesContext->SetColor(mSinglePixelColor);
    } else {
        nsRefPtr<gfxASurface> surface;
        PRInt32 width, height;

        if (hasPadding) {
            /* Ugh we have padding; create a temporary surface that's the size of the surface + pad area,
             * and render the image into it first.  Then we'll tile that surface. */
            width = mWidth + xPadding;
            height = mHeight + yPadding;

            // Reject over-wide or over-tall images.
            if (!AllowedImageSize(width, height))
                return NS_ERROR_FAILURE;

            surface = new gfxImageSurface(gfxIntSize(width, height),
                                          gfxASurface::ImageFormatARGB32);
            if (!surface || surface->CairoStatus()) {
                return NS_ERROR_OUT_OF_MEMORY;
            }

            tmpSurfaceGrip = surface;

            gfxContext tmpContext(surface);
            if (mSinglePixel) {
                tmpContext.SetColor(mSinglePixelColor);
            } else {
                tmpContext.SetSource(ThebesSurface());
            }
            tmpContext.SetOperator(gfxContext::OPERATOR_SOURCE);
            tmpContext.Rectangle(gfxRect(0, 0, mWidth, mHeight));
            tmpContext.Fill();
        } else {
            width = mWidth;
            height = mHeight;
            surface = ThebesSurface();
        }

        gfxMatrix patMat;
        gfxPoint p0;

        p0.x = - floor(offset.x + 0.5);
        p0.y = - floor(offset.y + 0.5);
        // Scale factor to account for CSS pixels; note that the offset (and 
        // therefore p0) is in device pixels, while the width and height are in
        // CSS pixels.
        gfxFloat scale = gfxFloat(dx->AppUnitsPerDevPixel()) /
                         gfxFloat(nsIDeviceContext::AppUnitsPerCSSPixel());
        patMat.Scale(scale, scale);
        patMat.Translate(p0);

        gfxPattern pat(surface);
        pat.SetExtend(gfxPattern::EXTEND_REPEAT);
        pat.SetMatrix(patMat);

#ifndef XP_MACOSX
        if (scale < 1.0) {
            // See bug 324698.  This is a workaround.  See comments
            // by the earlier SetFilter call.
            pat.SetFilter(0);
        }
#endif

        thebesContext->SetPattern(&pat);
    }

    gfxContext::GraphicsOperator op = thebesContext->CurrentOperator();
    if (op == gfxContext::OPERATOR_OVER && mFormat == gfxASurface::ImageFormatRGB24)
        thebesContext->SetOperator(gfxContext::OPERATOR_SOURCE);

    thebesContext->NewPath();
    thebesContext->Rectangle(targetRect, doSnap);
    thebesContext->Fill();

    thebesContext->SetOperator(op);
    thebesContext->SetColor(gfxRGBA(0,0,0,0));

    return NS_OK;
}

PRBool
nsThebesImage::ShouldUseImageSurfaces()
{
#ifdef XP_WIN
    static const DWORD kGDIObjectsHighWaterMark = 7000;

    // at 7000 GDI objects, stop allocating normal images to make sure
    // we never hit the 10k hard limit.
    // GetCurrentProcess() just returns (HANDLE)-1, it's inlined afaik
    DWORD count = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
    if (count == 0 ||
        count > kGDIObjectsHighWaterMark)
    {
        // either something's broken (count == 0),
        // or we hit our high water mark; disable
        // image allocations for a bit.
        return PR_TRUE;
    }
#endif

    return PR_FALSE;
}

// A hint from the image decoders that this image has no alpha, even
// though we created is ARGB32.  This changes our format to RGB24,
// which in turn will cause us to Optimize() to RGB24.  Has no effect
// after Optimize() is called, though in all cases it will be just a
// performance win -- the pixels are still correct and have the A byte
// set to 0xff.
void
nsThebesImage::SetHasNoAlpha()
{
    if (mFormat == gfxASurface::ImageFormatARGB32) {
        mFormat = gfxASurface::ImageFormatRGB24;
        mFormatChanged = PR_TRUE;
    }
}
