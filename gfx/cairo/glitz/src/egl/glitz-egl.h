/*
 * Copyright © 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * David Reveman not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * David Reveman makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DAVID REVEMAN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DAVID REVEMAN BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#ifndef GLITZ_EGL_H_INCLUDED
#define GLITZ_EGL_H_INCLUDED

#include <GL/gl.h>
#include <GLES/egl.h>

#include <glitz.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* glitz_egl_info.c */

void
glitz_egl_init (const char *gl_library);

void
glitz_egl_fini (void);


/* glitz_egl_config.c */

glitz_drawable_format_t *
glitz_egl_find_window_config (EGLDisplay                    egl_display,
			      EGLScreenMESA                 egl_screen,
			      unsigned long                 mask,
			      const glitz_drawable_format_t *templ,
			      int                           count);

glitz_drawable_format_t *
glitz_egl_find_pbuffer_config (EGLDisplay                    egl_display,
			       EGLScreenMESA                 egl_screen,
			       unsigned long                 mask,
			       const glitz_drawable_format_t *templ,
			       int                           count);


/* glitz_egl_surface.c */

glitz_drawable_t *
glitz_egl_create_surface (EGLDisplay              egl_display,
			  EGLScreenMESA           egl_screen,
			  glitz_drawable_format_t *format,
			  EGLSurface              egl_surface,
			  unsigned int            width,
			  unsigned int            height);

glitz_drawable_t *
glitz_egl_create_pbuffer_surface (EGLDisplay               egl_display,
				  EGLScreenMESA           egl_screen,
				  glitz_drawable_format_t *format,
				  unsigned int            width,
				  unsigned int            height);


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* GLITZ_EGL_H_INCLUDED */
