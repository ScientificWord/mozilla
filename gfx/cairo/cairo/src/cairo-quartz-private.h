/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2004 Calum Robinson
 * Copyright (C) 2006,2007 Mozilla Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Calum Robinson
 *
 * Contributor(s):
 *    Calum Robinson <calumr@mac.com>
 *    Vladimir Vukicevic <vladimir@mozilla.com>
 */

#ifndef CAIRO_QUARTZ_PRIVATE_H
#define CAIRO_QUARTZ_PRIVATE_H

#include "cairoint.h"

#ifdef CAIRO_HAS_QUARTZ_SURFACE
#include <cairo-quartz.h>

typedef struct cairo_quartz_surface {
    cairo_surface_t base;

    CGContextRef cgContext;
    CGAffineTransform cgContextBaseCTM;

    void *imageData;
    cairo_surface_t *imageSurfaceEquiv;

    cairo_rectangle_int_t extents;

    /* These are stored while drawing operations are in place, set up
     * by quartz_setup_source() and quartz_finish_source()
     */
    CGImageRef sourceImage;
    cairo_surface_t *sourceImageSurface;
    CGAffineTransform sourceImageTransform;
    CGRect sourceImageRect;

    CGShadingRef sourceShading;
    CGPatternRef sourcePattern;
} cairo_quartz_surface_t;

typedef struct cairo_quartz_image_surface {
    cairo_surface_t base;

    cairo_rectangle_int_t extents;

    CGImageRef image;
    cairo_image_surface_t *imageSurface;
} cairo_quartz_image_surface_t;

cairo_bool_t
_cairo_quartz_verify_surface_size(int width, int height);

CGImageRef
_cairo_quartz_create_cgimage (cairo_format_t format,
			      unsigned int width,
			      unsigned int height,
			      unsigned int stride,
			      void *data,
			      cairo_bool_t interpolate,
			      CGColorSpaceRef colorSpaceOverride,
			      CGDataProviderReleaseDataCallback releaseCallback,
			      void *releaseInfo);

#endif /* CAIRO_HAS_QUARTZ_SURFACE */

#if CAIRO_HAS_ATSUI_FONT
ATSUStyle
_cairo_atsui_scaled_font_get_atsu_style (cairo_scaled_font_t *sfont);

ATSUFontID
_cairo_atsui_scaled_font_get_atsu_font_id (cairo_scaled_font_t *sfont);

CGFontRef
_cairo_atsui_scaled_font_get_cg_font_ref (cairo_scaled_font_t *sfont);
#endif /* CAIRO_HAS_ATSUI_FONT */

#endif /* CAIRO_QUARTZ_PRIVATE_H */
