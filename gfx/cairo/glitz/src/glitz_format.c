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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#include "glitzint.h"

#include <stdlib.h>

static struct _texture_format {
    glitz_gl_int_t texture_format;
    glitz_format_t format;
} _texture_formats[] = {
    { GLITZ_GL_ALPHA4,   { 0, { GLITZ_FOURCC_RGB,  0,  0,  0,  4 } } },
    { GLITZ_GL_ALPHA8,   { 0, { GLITZ_FOURCC_RGB,  0,  0,  0,  8 } } },
    { GLITZ_GL_ALPHA12,  { 0, { GLITZ_FOURCC_RGB,  0,  0,  0, 12 } } },
    { GLITZ_GL_ALPHA16,  { 0, { GLITZ_FOURCC_RGB,  0,  0,  0, 16 } } },
    { GLITZ_GL_R3_G3_B2, { 0, { GLITZ_FOURCC_RGB,  3,  3,  2,  0 } } },
    { GLITZ_GL_RGB4,     { 0, { GLITZ_FOURCC_RGB,  4,  4,  4,  0 } } },
    { GLITZ_GL_RGB5,     { 0, { GLITZ_FOURCC_RGB,  5,  6,  5,  0 } } },
    { GLITZ_GL_RGB8,     { 0, { GLITZ_FOURCC_RGB,  8,  8,  8,  0 } } },
    { GLITZ_GL_RGB10,    { 0, { GLITZ_FOURCC_RGB, 10, 10, 10,  0 } } },
    { GLITZ_GL_RGB12,    { 0, { GLITZ_FOURCC_RGB, 12, 12, 12,  0 } } },
    { GLITZ_GL_RGB16,    { 0, { GLITZ_FOURCC_RGB, 16, 16, 16,  0 } } },
    { GLITZ_GL_RGBA2,    { 0, { GLITZ_FOURCC_RGB,  2,  2,  2,  2 } } },
    { GLITZ_GL_RGB5_A1,  { 0, { GLITZ_FOURCC_RGB,  5,  5,  5,  1 } } },
    { GLITZ_GL_RGBA4,    { 0, { GLITZ_FOURCC_RGB,  4,  4,  4,  4 } } },
    { GLITZ_GL_RGBA8,    { 0, { GLITZ_FOURCC_RGB,  8,  8,  8,  8 } } },
    { GLITZ_GL_RGB10_A2, { 0, { GLITZ_FOURCC_RGB, 10, 10, 10,  2 } } },
    { GLITZ_GL_RGBA12,   { 0, { GLITZ_FOURCC_RGB, 12, 12, 12, 12 } } },
    { GLITZ_GL_RGBA16,   { 0, { GLITZ_FOURCC_RGB, 16, 16, 16, 16 } } }
};

static glitz_format_t _texture_format_yv12 = {
    0, { GLITZ_FOURCC_YV12,  0,  0,  0,  0 }
};

static void
_glitz_add_texture_format (glitz_format_t **formats,
			   glitz_gl_int_t **texture_formats,
			   int            *n_formats,
			   glitz_gl_int_t texture_format,
			   glitz_format_t *format)
{
    *formats = realloc (*formats, sizeof (glitz_format_t) * (*n_formats + 1));
    *texture_formats = realloc (*texture_formats,
				sizeof (glitz_gl_enum_t) * (*n_formats + 1));

    if (*formats && *texture_formats) {
	(*texture_formats)[*n_formats] = texture_format;
	(*formats)[*n_formats] = *format;
	(*formats)[*n_formats].id = *n_formats;
	(*n_formats)++;
    } else
	*n_formats = 0;
}

void
glitz_create_surface_formats (glitz_gl_proc_address_list_t *gl,
			      glitz_format_t               **formats,
			      glitz_gl_int_t               **texture_formats,
			      int                          *n_formats,
			      unsigned long                features)
{
    glitz_gl_int_t value;
    int i, n_texture_formats;

    n_texture_formats =
	sizeof (_texture_formats) / sizeof (struct _texture_format);

    for (i = 0; i < n_texture_formats; i++) {
	switch (_texture_formats[i].format.color.fourcc) {
	case GLITZ_FOURCC_RGB:
	    gl->tex_image_2d (GLITZ_GL_PROXY_TEXTURE_2D, 0,
			      _texture_formats[i].texture_format, 1, 1, 0,
			      GLITZ_GL_RGBA, GLITZ_GL_UNSIGNED_BYTE, NULL);

	    if (_texture_formats[i].format.color.red_size) {
		gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
						GLITZ_GL_TEXTURE_RED_SIZE,
						&value);
		if (value != _texture_formats[i].format.color.red_size)
		    continue;
	    }

	    if (_texture_formats[i].format.color.green_size) {
		gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
						GLITZ_GL_TEXTURE_GREEN_SIZE,
						&value);
		if (value != _texture_formats[i].format.color.green_size)
		    continue;
	    }

	    if (_texture_formats[i].format.color.blue_size) {
		gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
						GLITZ_GL_TEXTURE_BLUE_SIZE,
						&value);
		if (value != _texture_formats[i].format.color.blue_size)
		    continue;
	    }

	    if (_texture_formats[i].format.color.alpha_size) {
		gl->get_tex_level_parameter_iv (GLITZ_GL_PROXY_TEXTURE_2D, 0,
						GLITZ_GL_TEXTURE_ALPHA_SIZE,
						&value);
		if (value != _texture_formats[i].format.color.alpha_size)
		    continue;
	    }
	    break;
	default:
	    continue;
	}

	_glitz_add_texture_format (formats,
				   texture_formats,
				   n_formats,
				   _texture_formats[i].texture_format,
				   &_texture_formats[i].format);
    }

    /* formats used for YUV surfaces */
    if (features & GLITZ_FEATURE_FRAGMENT_PROGRAM_MASK)
    {
	_glitz_add_texture_format (formats, texture_formats, n_formats,
				   GLITZ_GL_LUMINANCE8, &_texture_format_yv12);
    }
}

static void
_glitz_add_drawable_format (glitz_int_drawable_format_t *format,
			    glitz_int_drawable_format_t **formats,
			    int                         *n_formats)
{
    void *ptr;

    ptr = realloc (*formats,
		   sizeof (glitz_int_drawable_format_t) * (*n_formats + 1));
    if (ptr)
    {
	*formats = ptr;
	(*formats)[*n_formats] = *format;
	(*n_formats)++;
    }
}

/* TODO: Available drawable formats needs to be validated in a similar way
   as surface formats. */
void
_glitz_add_drawable_formats (glitz_gl_proc_address_list_t *gl,
			     unsigned long		  feature_mask,
			     glitz_int_drawable_format_t  **formats,
			     int                          *n_formats)
{
    if (feature_mask & GLITZ_FEATURE_FRAMEBUFFER_OBJECT_MASK)
    {
	glitz_int_drawable_format_t format;
	glitz_drawable_format_t     d[] = {
	    { 0, { GLITZ_FOURCC_RGB, 8, 8, 8, 0 }, 0,  0, 1, 0 },
	    { 0, { GLITZ_FOURCC_RGB, 8, 8, 8, 8 }, 0,  0, 1, 0 },
	    { 0, { GLITZ_FOURCC_RGB, 8, 8, 8, 0 }, 24, 8, 1, 1 },
	    { 0, { GLITZ_FOURCC_RGB, 8, 8, 8, 8 }, 24, 8, 1, 1 }
	};
	int			    i;

	format.types  = GLITZ_DRAWABLE_TYPE_FBO_MASK;
	format.caveat = 0;
	format.u.val  = 0;

	for (i = 0; i < sizeof (d) / sizeof (d[0]); i++)
	{
	    format.d    = d[i];
	    format.d.id = *n_formats;

	    _glitz_add_drawable_format (&format, formats, n_formats);
	}
    }
}

void
glitz_drawable_format_copy (const glitz_drawable_format_t *src,
			    glitz_drawable_format_t	  *dst,
			    unsigned long		  mask)
{
    if (mask & GLITZ_FORMAT_ID_MASK)
	dst->id = src->id;

    if (mask & GLITZ_FORMAT_FOURCC_MASK)
	dst->color.fourcc = src->color.fourcc;

    if (mask & GLITZ_FORMAT_RED_SIZE_MASK)
	dst->color.red_size = src->color.red_size;

    if (mask & GLITZ_FORMAT_GREEN_SIZE_MASK)
	dst->color.green_size = src->color.green_size;

    if (mask & GLITZ_FORMAT_BLUE_SIZE_MASK)
	dst->color.blue_size = src->color.blue_size;

    if (mask & GLITZ_FORMAT_ALPHA_SIZE_MASK)
	dst->color.alpha_size = src->color.alpha_size;

    if (mask & GLITZ_FORMAT_DEPTH_SIZE_MASK)
	dst->depth_size = src->depth_size;

    if (mask & GLITZ_FORMAT_STENCIL_SIZE_MASK)
	dst->stencil_size = src->stencil_size;

    if (mask & GLITZ_FORMAT_DOUBLEBUFFER_MASK)
	dst->doublebuffer = src->doublebuffer;

    if (mask & GLITZ_FORMAT_SAMPLES_MASK)
	dst->samples = src->samples;
}

glitz_drawable_format_t *
glitz_drawable_format_find (glitz_int_drawable_format_t       *formats,
			    int                               n_formats,
			    unsigned long                     mask,
			    const glitz_int_drawable_format_t *templ,
			    int                               count)
{
    for (; n_formats; n_formats--, formats++)
    {
	if (mask & GLITZ_FORMAT_ID_MASK)
	    if (templ->d.id != formats->d.id)
		continue;

	if (mask & GLITZ_FORMAT_FOURCC_MASK)
	    if (templ->d.color.fourcc != formats->d.color.fourcc)
		continue;

	if (mask & GLITZ_FORMAT_RED_SIZE_MASK)
	    if (templ->d.color.red_size != formats->d.color.red_size)
		continue;

	if (mask & GLITZ_FORMAT_GREEN_SIZE_MASK)
	    if (templ->d.color.green_size != formats->d.color.green_size)
		continue;

	if (mask & GLITZ_FORMAT_BLUE_SIZE_MASK)
	    if (templ->d.color.blue_size != formats->d.color.blue_size)
		continue;

	if (mask & GLITZ_FORMAT_ALPHA_SIZE_MASK)
	    if (templ->d.color.alpha_size != formats->d.color.alpha_size)
		continue;

	if (mask & GLITZ_FORMAT_DEPTH_SIZE_MASK)
	    if (templ->d.depth_size != formats->d.depth_size)
		continue;

	if (mask & GLITZ_FORMAT_STENCIL_SIZE_MASK)
	    if (templ->d.stencil_size != formats->d.stencil_size)
		continue;

	if (mask & GLITZ_FORMAT_DOUBLEBUFFER_MASK)
	    if (templ->d.doublebuffer != formats->d.doublebuffer)
		continue;

	if (mask & GLITZ_FORMAT_SAMPLES_MASK)
	    if (templ->d.samples != formats->d.samples)
		continue;

	if (mask & GLITZ_INT_FORMAT_WINDOW_MASK)
	    if ((templ->types   & GLITZ_DRAWABLE_TYPE_WINDOW_MASK) !=
		(formats->types & GLITZ_DRAWABLE_TYPE_WINDOW_MASK))
		continue;

	if (mask & GLITZ_INT_FORMAT_PBUFFER_MASK)
	    if ((templ->types   & GLITZ_DRAWABLE_TYPE_PBUFFER_MASK) !=
		(formats->types & GLITZ_DRAWABLE_TYPE_PBUFFER_MASK))
		continue;

	if (mask & GLITZ_INT_FORMAT_FBO_MASK)
	    if ((templ->types   & GLITZ_DRAWABLE_TYPE_FBO_MASK) !=
		(formats->types & GLITZ_DRAWABLE_TYPE_FBO_MASK))
		continue;

	if (count-- == 0)
	    return &formats->d;
    }

    return NULL;
}

static glitz_format_t *
_glitz_format_find (glitz_format_t       *formats,
		    int                  n_formats,
		    unsigned long        mask,
		    const glitz_format_t *templ,
		    int                  count)
{
    for (; n_formats; n_formats--, formats++) {
	if (mask & GLITZ_FORMAT_ID_MASK)
	    if (templ->id != formats->id)
		continue;

	if (mask & GLITZ_FORMAT_FOURCC_MASK)
	    if (templ->color.fourcc != formats->color.fourcc)
		continue;

	if (mask & GLITZ_FORMAT_RED_SIZE_MASK)
	    if (templ->color.red_size != formats->color.red_size)
		continue;

	if (mask & GLITZ_FORMAT_GREEN_SIZE_MASK)
	    if (templ->color.green_size != formats->color.green_size)
		continue;

	if (mask & GLITZ_FORMAT_BLUE_SIZE_MASK)
	    if (templ->color.blue_size != formats->color.blue_size)
		continue;

	if (mask & GLITZ_FORMAT_ALPHA_SIZE_MASK)
	    if (templ->color.alpha_size != formats->color.alpha_size)
		continue;

	if (count-- == 0)
	    return formats;
    }

    return NULL;
}

glitz_format_t *
glitz_find_format (glitz_drawable_t     *drawable,
		   unsigned long        mask,
		   const glitz_format_t *templ,
		   int                  count)
{
    return _glitz_format_find (drawable->backend->formats,
			       drawable->backend->n_formats,
			       mask, templ, count);
}

glitz_format_t *
glitz_find_standard_format (glitz_drawable_t    *drawable,
			    glitz_format_name_t format_name)
{
    glitz_format_t templ;
    unsigned long mask = GLITZ_FORMAT_RED_SIZE_MASK |
	GLITZ_FORMAT_GREEN_SIZE_MASK | GLITZ_FORMAT_BLUE_SIZE_MASK |
	GLITZ_FORMAT_ALPHA_SIZE_MASK | GLITZ_FORMAT_FOURCC_MASK;

    templ.color.fourcc = GLITZ_FOURCC_RGB;
    templ.color.red_size = 0;
    templ.color.green_size = 0;
    templ.color.blue_size = 0;
    templ.color.alpha_size = 0;

    switch (format_name) {
    case GLITZ_STANDARD_ARGB32:
	templ.color.red_size = 8;
	templ.color.green_size = 8;
	templ.color.blue_size = 8;
	templ.color.alpha_size = 8;
	break;
    case GLITZ_STANDARD_RGB24:
	templ.color.red_size = 8;
	templ.color.green_size = 8;
	templ.color.blue_size = 8;
	break;
    case GLITZ_STANDARD_A8:
	templ.color.alpha_size = 8;
	break;
    case GLITZ_STANDARD_A1:
	templ.color.alpha_size = 1;
	break;
    }

    return glitz_find_format (drawable, mask, &templ, 0);
}

glitz_drawable_format_t *
glitz_find_drawable_format (glitz_drawable_t              *other,
			    unsigned long                 mask,
			    const glitz_drawable_format_t *templ,
			    int                           count)
{
    glitz_int_drawable_format_t itempl;

    glitz_drawable_format_copy (templ, &itempl.d, mask);

    itempl.types = GLITZ_DRAWABLE_TYPE_FBO_MASK;
    mask |= GLITZ_INT_FORMAT_FBO_MASK;

    return glitz_drawable_format_find (other->backend->drawable_formats,
				       other->backend->n_drawable_formats,
				       mask, &itempl, count);
}
slim_hidden_def(glitz_find_drawable_format);

glitz_drawable_format_t *
glitz_find_pbuffer_format (glitz_drawable_t              *other,
			   unsigned long                 mask,
			   const glitz_drawable_format_t *templ,
			   int                           count)
{
    glitz_int_drawable_format_t itempl;

    glitz_drawable_format_copy (templ, &itempl.d, mask);

    itempl.types = GLITZ_DRAWABLE_TYPE_PBUFFER_MASK;
    mask |= GLITZ_INT_FORMAT_PBUFFER_MASK;

    return glitz_drawable_format_find (other->backend->drawable_formats,
				       other->backend->n_drawable_formats,
				       mask, &itempl, count);
}
slim_hidden_def(glitz_find_pbuffer_format);
