/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2006 Red Hat, Inc.
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
 * The Initial Developer of the Original Code is University of Southern
 * California.
 *
 * Contributor(s):
 *	Carl D. Worth <cworth@cworth.org>
 */

#ifndef CAIRO_PDF_TEST_H
#define CAIRO_PDF_TEST_H

#include <cairo.h>

#if CAIRO_HAS_PDF_SURFACE

#include <cairo-pdf.h>

CAIRO_BEGIN_DECLS

struct _cairo_path_fixed;
struct _cairo_traps;
struct _cairo_trapezoid;
struct _cairo_clip;

void
cairo_pdf_test_force_fallbacks (void);

void
cairo_debug_dump_clip (struct _cairo_clip *clip,
                       FILE *fp);
void
cairo_debug_dump_path (struct _cairo_path_fixed *path,
                       FILE *fp);

void
cairo_debug_dump_traps (struct _cairo_traps *traps,
                        FILE *fp);

void
cairo_debug_dump_trapezoid_array (struct _cairo_trapezoid *traps,
                                  int num_traps,
                                  FILE *fp);

CAIRO_END_DECLS

#endif /* CAIRO_HAS_PDF_SURFACE */
#endif /* CAIRO_PDF_TEST_H */

