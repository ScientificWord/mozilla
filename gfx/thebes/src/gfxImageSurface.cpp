/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Oracle Corporation code.
 *
 * The Initial Developer of the Original Code is Oracle Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@pavlov.net>
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

#include <stdio.h>

#include "gfxImageSurface.h"

gfxImageSurface::gfxImageSurface(gfxImageFormat format, long width, long height) :
    mFormat(format), mWidth(width), mHeight(height)
{
    long stride = Stride();
    mData = new unsigned char[height * stride];

    //memset(mData, 0xff, height*stride);

    cairo_surface_t *surface =
        cairo_image_surface_create_for_data((unsigned char*)mData,
                                            (cairo_format_t)format,
                                            width,
                                            height,
                                            stride);
    Init(surface);
}

gfxImageSurface::gfxImageSurface(cairo_surface_t *csurf)
{
    mWidth = cairo_image_surface_get_width(csurf);
    mHeight = cairo_image_surface_get_height(csurf);
    mData = nsnull;
    mFormat = ImageFormatUnknown;

    Init(csurf, PR_TRUE);
}

gfxImageSurface::~gfxImageSurface()
{
    Destroy();

    delete[] mData;
}

long
gfxImageSurface::Stride() const
{
    long stride;

    if (mFormat == ImageFormatARGB32)
        stride = mWidth * 4;
    else if (mFormat == ImageFormatRGB24)
        stride = mWidth * 4;
    else if (mFormat == ImageFormatA8)
        stride = mWidth;
    else if (mFormat == ImageFormatA1) {
        stride = (mWidth + 7) / 8;
    } else {
        NS_WARNING("Unknown format specified to gfxImageSurface!");
        stride = mWidth * 4;
    }

    stride = ((stride + 3) / 4) * 4;

    return stride;
}
