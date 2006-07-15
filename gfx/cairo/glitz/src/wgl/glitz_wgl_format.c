/*
 * Copyright � 2004 David Reveman
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the names of
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
 * Based on glx code by David Reveman
 *
 * Contributors:
 *  Tor Lillqvist <tml@iki.fi>
 *  Vladimir Vukicevic <vladimir@pobox.com>
 *
 */

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitz_wglint.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int
_glitz_wgl_format_compare (const void *elem1,
			   const void *elem2)
{
    glitz_int_drawable_format_t *format[2];
    int				i, score[2];

    format[0] = (glitz_int_drawable_format_t *) elem1;
    format[1] = (glitz_int_drawable_format_t *) elem2;
    i = score[0] = score[1] = 0;

    for (; i < 2; i++)
    {
	if (format[i]->d.color.fourcc != GLITZ_FOURCC_RGB)
	    score[i] -= 1000;

	if (format[i]->d.color.red_size)
	{
	    if (format[i]->d.color.red_size >= 8)
		score[i] += 5;

	    score[i] += 10;
	}

	if (format[i]->d.color.alpha_size)
	{
	    if (format[i]->d.color.alpha_size >= 8)
		score[i] += 5;

	    score[i] += 10;
	}

	if (format[i]->d.stencil_size)
	    score[i] += 5;

	if (format[i]->d.depth_size)
	    score[i] += 5;

	if (format[i]->d.doublebuffer)
	    score[i] += 10;

	if (format[i]->d.samples > 1)
	    score[i] -= (20 - format[i]->d.samples);

	if (format[i]->types & GLITZ_DRAWABLE_TYPE_WINDOW_MASK)
	    score[i] += 10;

	if (format[i]->types & GLITZ_DRAWABLE_TYPE_PBUFFER_MASK)
	    score[i] += 10;

	if (format[i]->caveat)
	    score[i] -= format[i]->caveat * 500;
    }

    return score[1] - score[0];
}

static void
_glitz_add_format (glitz_wgl_screen_info_t     *screen_info,
		   glitz_int_drawable_format_t *format,
		   int                          id)
{
    int n = screen_info->n_formats;

    screen_info->format_ids =
	realloc (screen_info->format_ids,
		 sizeof (int) * (n + 1));
    screen_info->formats =
	realloc (screen_info->formats,
		 sizeof (glitz_int_drawable_format_t) * (n + 1));
    if (screen_info->format_ids && screen_info->formats)
    {
	screen_info->formats[n] = *format;
	screen_info->formats[n].d.id = n;
	screen_info->format_ids[n] = id;
	screen_info->n_formats++;
    }
}

static glitz_bool_t
_glitz_wgl_query_formats (glitz_wgl_screen_info_t *screen_info)
{
    glitz_int_drawable_format_t format;
    int i, num_pixel_formats;
    PIXELFORMATDESCRIPTOR pfd;

    pfd.nSize = sizeof (PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;

    num_pixel_formats =
	DescribePixelFormat (screen_info->root_dc, 1,
			     sizeof (PIXELFORMATDESCRIPTOR), &pfd);

    if (!num_pixel_formats)
	return 0;

    for (i = 1; i <= num_pixel_formats; i++) {
	format.d.id = 0;
	format.caveat = 0;
	format.types = 0;

	if (!DescribePixelFormat (screen_info->root_dc, i,
				  sizeof (PIXELFORMATDESCRIPTOR), &pfd))
	{
	    continue;
	}

#ifdef DEBUG
        printf ("Pixel format %d Flags: 0x%08x\n", i, pfd.dwFlags);
#endif

	if (!((pfd.dwFlags & PFD_DRAW_TO_WINDOW) &&
	      (pfd.dwFlags & PFD_SUPPORT_OPENGL)))
	    continue;

	if (pfd.iPixelType != PFD_TYPE_RGBA)
	    continue;

	if (pfd.dwFlags & (PFD_NEED_PALETTE |
			   PFD_NEED_SYSTEM_PALETTE |
			   PFD_STEREO))
	    continue;

#ifndef USE_MESA
	/* Can't use the Windows software implementation as glitz
	 * unconditionally requires OpenGL 1.2, and it's only 1.1.
	 */
	if (pfd.dwFlags & PFD_GENERIC_FORMAT)
	    continue;
#endif

	format.types = GLITZ_DRAWABLE_TYPE_WINDOW_MASK;

	format.d.color.red_size = pfd.cRedBits;
	format.d.color.green_size = pfd.cGreenBits;
	format.d.color.blue_size = pfd.cBlueBits;
	format.d.color.alpha_size = pfd.cAlphaBits;
	format.d.color.fourcc = GLITZ_FOURCC_RGB;

	format.d.depth_size = pfd.cDepthBits;
	format.d.stencil_size = pfd.cStencilBits;

	format.d.doublebuffer = (pfd.dwFlags & PFD_DOUBLEBUFFER) != 0;

	format.d.samples = 1;

	_glitz_add_format (screen_info, &format, i);
    }

    return 0;
}

static glitz_status_t
_glitz_wgl_query_formats_using_pixel_format (glitz_wgl_screen_info_t *screen_info)
{
    glitz_int_drawable_format_t format;
    int i, num_pixel_formats;
    int question, answer;
    glitz_wgl_static_proc_address_list_t *wgl = &screen_info->wgl;

    question = WGL_NUMBER_PIXEL_FORMATS_ARB;
    if (!wgl->get_pixel_format_attrib_iv (screen_info->root_dc, 0, 0, 1,
					  &question, &num_pixel_formats)) {
	/* wglGetPixelFormatAttribivARB didn't work, fall back to
	 * DescribePixelFormat.
	 */
	screen_info->wgl_feature_mask &= ~GLITZ_WGL_FEATURE_PIXEL_FORMAT_MASK;
	return GLITZ_STATUS_NOT_SUPPORTED;
    }

    for (i = 1; i <= num_pixel_formats; i++) {
	format.d.id = 0;
	format.types = 0;
	format.d.color.fourcc = GLITZ_FOURCC_RGB;

#define ASK_QUESTION(q,a) (question=(q),wgl->get_pixel_format_attrib_iv (screen_info->root_dc, i, 0, 1, &question,(a)))

	if (!ASK_QUESTION(WGL_SUPPORT_OPENGL_ARB, &answer)
	    || !answer)
	    continue;

	if (!ASK_QUESTION(WGL_PIXEL_TYPE_ARB, &answer)
	    || answer != WGL_TYPE_RGBA_ARB)
	    continue;

	if (!ASK_QUESTION(WGL_ACCELERATION_ARB, &answer))
	    continue;
	if (answer == WGL_NO_ACCELERATION_ARB)
	    format.caveat = 2;
	else if (answer == WGL_GENERIC_ACCELERATION_ARB)
	    format.caveat = 1;
	else
	    format.caveat = 0;

	/* Stereo is not supported yet */
	if (!ASK_QUESTION(WGL_STEREO_ARB, &answer)
	    || answer != 0)
	    continue;

	if (!ASK_QUESTION(WGL_DRAW_TO_WINDOW_ARB, &answer))
	    continue;
	format.types |= answer ? GLITZ_DRAWABLE_TYPE_WINDOW_MASK : 0;

	if (!ASK_QUESTION(WGL_DRAW_TO_PBUFFER_ARB, &answer))
	    continue;
	format.types |= answer ? GLITZ_DRAWABLE_TYPE_PBUFFER_MASK : 0;

	if (!(format.types & (GLITZ_DRAWABLE_TYPE_WINDOW_MASK |
			      GLITZ_DRAWABLE_TYPE_PBUFFER_MASK)))
	    continue;

	if (!ASK_QUESTION(WGL_RED_BITS_ARB, &answer))
	    continue;
	format.d.color.red_size = (unsigned short) answer;

	if (!ASK_QUESTION(WGL_GREEN_BITS_ARB, &answer))
	    continue;
	format.d.color.green_size = (unsigned short) answer;

	if (!ASK_QUESTION(WGL_BLUE_BITS_ARB, &answer))
	    continue;
	format.d.color.blue_size = (unsigned short) answer;

	if (!ASK_QUESTION(WGL_ALPHA_BITS_ARB, &answer))
	    continue;
	format.d.color.alpha_size = (unsigned short) answer;

	if (!ASK_QUESTION(WGL_DEPTH_BITS_ARB, &answer))
	    continue;
	format.d.depth_size = (unsigned short) answer;

	if (!ASK_QUESTION(WGL_STENCIL_BITS_ARB, &answer))
	    continue;
	format.d.stencil_size = (unsigned short) answer;

	if (!ASK_QUESTION(WGL_DOUBLE_BUFFER_ARB, &answer))
	    continue;
	format.d.doublebuffer = (answer) ? 1: 0;

	format.d.samples = 1;
	if (screen_info->wgl_feature_mask & GLITZ_WGL_FEATURE_MULTISAMPLE_MASK) {
	    if (!ASK_QUESTION(WGL_SAMPLE_BUFFERS_ARB, &answer))
		continue;

	    if (answer) {
		if (!ASK_QUESTION(WGL_SAMPLES_ARB, &answer))
		    continue;
		format.d.samples = (unsigned short) answer;
	    }
	}

	_glitz_add_format (screen_info, &format, i);
    }

    return GLITZ_STATUS_SUCCESS;
}

void
glitz_wgl_query_formats (glitz_wgl_screen_info_t *screen_info)
{
    glitz_status_t status = GLITZ_STATUS_NOT_SUPPORTED;
    int *new_ids, i;

    if (screen_info->wgl_feature_mask & GLITZ_WGL_FEATURE_PIXEL_FORMAT_MASK)
	status = _glitz_wgl_query_formats_using_pixel_format (screen_info);

    if (status)
	_glitz_wgl_query_formats (screen_info);

    if (!screen_info->n_formats)
	return;

    qsort (screen_info->formats, screen_info->n_formats,
	   sizeof (glitz_int_drawable_format_t), _glitz_wgl_format_compare);

    new_ids = malloc (sizeof (*new_ids) * screen_info->n_formats);

    for (i = 0; i < screen_info->n_formats; i++) {
	new_ids[i] = screen_info->format_ids[screen_info->formats[i].d.id];
	screen_info->formats[i].d.id = i;
    }

    free (screen_info->format_ids);
    screen_info->format_ids = new_ids;
}

glitz_drawable_format_t *
glitz_wgl_find_window_format (unsigned long                 mask,
			      const glitz_drawable_format_t *templ,
			      int                           count)
{
    glitz_int_drawable_format_t itempl;
    glitz_wgl_screen_info_t *screen_info =
	glitz_wgl_screen_info_get ();

    glitz_drawable_format_copy (templ, &itempl.d, mask);

    itempl.types = GLITZ_DRAWABLE_TYPE_WINDOW_MASK;
    mask |= GLITZ_INT_FORMAT_WINDOW_MASK;

    return glitz_drawable_format_find (screen_info->formats,
				       screen_info->n_formats,
				       mask, &itempl, count);
}

glitz_drawable_format_t *
glitz_wgl_find_pbuffer_format (unsigned long                 mask,
			       const glitz_drawable_format_t *templ,
			       int                           count)
{
    glitz_int_drawable_format_t itempl;
    glitz_wgl_screen_info_t *screen_info =
	glitz_wgl_screen_info_get ();

    glitz_drawable_format_copy (templ, &itempl.d, mask);

    itempl.types = GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;
    mask |= GLITZ_INT_FORMAT_PBUFFER_MASK;

    return glitz_drawable_format_find (screen_info->formats,
				       screen_info->n_formats,
				       mask, &itempl, count);
}
