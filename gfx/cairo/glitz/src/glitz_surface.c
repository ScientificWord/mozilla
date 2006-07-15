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
#include <string.h>

glitz_surface_t *
glitz_surface_create (glitz_drawable_t           *drawable,
		      glitz_format_t             *format,
		      unsigned int               width,
		      unsigned int               height,
		      unsigned long              mask,
		      glitz_surface_attributes_t *attributes)
{
    glitz_surface_t *surface;
    glitz_bool_t    unnormalized = 0;
    unsigned long   feature_mask = drawable->backend->feature_mask;

    if (!width || !height)
	return NULL;

    if (mask & GLITZ_SURFACE_UNNORMALIZED_MASK)
    {
	if (attributes->unnormalized)
	{
	    if (!(feature_mask & GLITZ_FEATURE_TEXTURE_RECTANGLE_MASK))
		return NULL;

	    unnormalized = 1;
	}
    }

    surface = (glitz_surface_t *) calloc (1, sizeof (glitz_surface_t));
    if (surface == NULL)
	return NULL;

    surface->drawable = drawable;
    glitz_drawable_reference (drawable);

    surface->ref_count = 1;
    surface->filter    = GLITZ_FILTER_NEAREST;
    surface->format    = format;
    surface->box.x2    = (short) width;
    surface->box.y2    = (short) height;
    surface->clip      = &surface->box;
    surface->n_clip    = 1;
    surface->buffer    = GLITZ_GL_FRONT;

    if (width == 1 && height == 1)
    {
	surface->flags |= GLITZ_SURFACE_FLAG_SOLID_MASK;
	surface->solid.alpha = 0xffff;

	REGION_INIT (&surface->texture_damage, &surface->box);
	REGION_INIT (&surface->drawable_damage, &surface->box);
    }
    else
    {
	REGION_INIT (&surface->texture_damage, NULL_BOX);
	REGION_INIT (&surface->drawable_damage, NULL_BOX);
    }

    glitz_texture_init (&surface->texture, width, height,
			drawable->backend->texture_formats[format->id],
			surface->format->color.fourcc,
			feature_mask, unnormalized);

    glitz_surface_set_filter (surface, GLITZ_FILTER_NEAREST, NULL, 0);

    if (width > 64 || height > 64)
    {
	glitz_surface_push_current (surface, GLITZ_CONTEXT_CURRENT);
	glitz_texture_size_check (drawable->backend->gl, &surface->texture,
				  drawable->backend->max_texture_2d_size,
				  drawable->backend->max_texture_rect_size);
	glitz_surface_pop_current (surface);

	if (TEXTURE_INVALID_SIZE (&surface->texture))
	{
	    glitz_surface_destroy (surface);
	    return NULL;
	}
    }

    return surface;
}

void
glitz_surface_destroy (glitz_surface_t *surface)
{
    if (!surface)
	return;

    surface->ref_count--;
    if (surface->ref_count)
	return;

    if (surface->attached)
    {
	surface->attached->backend->detach_notify (surface->attached, surface);
	if (surface->attached->front == surface)
	    surface->attached->front = NULL;
	else if (surface->attached->back == surface)
	    surface->attached->back = NULL;

	glitz_drawable_destroy (surface->attached);
	surface->attached = NULL;
    }

    if (surface->texture.name) {
	glitz_surface_push_current (surface, GLITZ_ANY_CONTEXT_CURRENT);
	glitz_texture_fini (surface->drawable->backend->gl, &surface->texture);
	glitz_surface_pop_current (surface);
    }

    REGION_UNINIT (&surface->texture_damage);
    REGION_UNINIT (&surface->drawable_damage);

    if (surface->geometry.buffer)
	glitz_buffer_destroy (surface->geometry.buffer);

    if (surface->geometry.array)
	glitz_multi_array_destroy (surface->geometry.array);

    if (surface->transform)
	free (surface->transform);

    if (surface->filter_params)
	free (surface->filter_params);

    glitz_drawable_destroy (surface->drawable);

    free (surface);
}

void
glitz_surface_reference (glitz_surface_t *surface)
{
    if (surface == NULL)
	return;

    surface->ref_count++;
}

void
_glitz_surface_sync_texture (glitz_surface_t *surface)
{
    if (REGION_NOTEMPTY (&surface->texture_damage))
    {
	glitz_box_t *box;
	int         n_box;

	GLITZ_GL_SURFACE (surface);

	if (!(TEXTURE_ALLOCATED (&surface->texture)))
	    glitz_texture_allocate (gl, &surface->texture);

	if (SURFACE_SOLID (surface) && (!SURFACE_SOLID_DAMAGE (surface)))
	{
	    glitz_gl_float_t color[4];

	    if (TEXTURE_ALLOCATED (&surface->texture))
	    {
		color[0] = surface->solid.red / 65535.0f;
		color[1] = surface->solid.green / 65535.0f;
		color[2] = surface->solid.blue / 65535.0f;
		color[3] = surface->solid.alpha / 65535.0f;

		glitz_texture_bind (gl, &surface->texture);
		gl->tex_sub_image_2d (surface->texture.target, 0,
				      surface->texture.box.x1,
				      surface->texture.box.y1,
				      1, 1, GLITZ_GL_RGBA,
				      GLITZ_GL_FLOAT, color);
		glitz_texture_unbind (gl, &surface->texture);
	    }
	    REGION_EMPTY (&surface->texture_damage);
	    return;
	}

	glitz_surface_push_current (surface, GLITZ_DRAWABLE_CURRENT);

	surface->drawable->backend->read_buffer (surface->drawable,
						 surface->buffer);

	gl->disable (GLITZ_GL_SCISSOR_TEST);

	glitz_texture_bind (gl, &surface->texture);

	box = REGION_RECTS (&surface->texture_damage);
	n_box = REGION_NUM_RECTS (&surface->texture_damage);

	while (n_box--)
	{
	    glitz_texture_copy_drawable (gl,
					 &surface->texture,
					 surface->attached,
					 box->x1 + surface->x,
					 box->y1 + surface->y,
					 box->x2 - box->x1,
					 box->y2 - box->y1,
					 box->x1,
					 box->y1);

	    box++;
	}

	REGION_EMPTY (&surface->texture_damage);

	glitz_texture_unbind (gl, &surface->texture);

	gl->enable (GLITZ_GL_SCISSOR_TEST);

	glitz_surface_pop_current (surface);
    }
}

void
glitz_surface_sync_drawable (glitz_surface_t *surface)
{
    if (REGION_NOTEMPTY (&surface->drawable_damage))
    {
	glitz_texture_t		   *texture;
	glitz_texture_parameters_t param;
	glitz_box_t		   *box, *ext;
	int			   n_box;

	GLITZ_GL_SURFACE (surface);

	texture = glitz_surface_get_texture (surface, 0);
	if (!texture)
	    return;

	box = REGION_RECTS (&surface->drawable_damage);
	ext = REGION_EXTENTS (&surface->drawable_damage);
	n_box = REGION_NUM_RECTS (&surface->drawable_damage);

	glitz_texture_bind (gl, texture);

	glitz_texture_set_tex_gen (gl, texture, NULL,
				   0, 0,
				   GLITZ_SURFACE_FLAGS_GEN_COORDS_MASK,
				   NULL);

	gl->tex_env_f (GLITZ_GL_TEXTURE_ENV, GLITZ_GL_TEXTURE_ENV_MODE,
		       GLITZ_GL_REPLACE);
	gl->color_4us (0x0, 0x0, 0x0, 0xffff);

	param.filter[0] = param.filter[1] = GLITZ_GL_NEAREST;
	param.wrap[0] = param.wrap[1] = GLITZ_GL_CLAMP_TO_EDGE;

	glitz_texture_ensure_parameters (gl, texture, &param);

	glitz_set_operator (gl, GLITZ_OPERATOR_SRC);

	gl->scissor (surface->x + ext->x1,
		     surface->attached->height - surface->y - ext->y2,
		     ext->x2 - ext->x1,
		     ext->y2 - ext->y1);

	if (n_box > 1)
	{
	    glitz_float_t *data;
	    void          *ptr;
	    int           vertices;

	    ptr = malloc (n_box * 8 * sizeof (glitz_float_t));
	    if (!ptr) {
		glitz_surface_status_add (surface,
					  GLITZ_STATUS_NO_MEMORY_MASK);
		return;
	    }

	    data = (glitz_float_t *) ptr;
	    vertices = n_box << 2;

	    while (n_box--)
	    {
		*data++ = (glitz_float_t) box->x1;
		*data++ = (glitz_float_t) box->y1;
		*data++ = (glitz_float_t) box->x2;
		*data++ = (glitz_float_t) box->y1;
		*data++ = (glitz_float_t) box->x2;
		*data++ = (glitz_float_t) box->y2;
		*data++ = (glitz_float_t) box->x1;
		*data++ = (glitz_float_t) box->y2;

		box++;
	    }

	    gl->vertex_pointer (2, GLITZ_GL_FLOAT, 0, ptr);
	    gl->draw_arrays (GLITZ_GL_QUADS, 0, vertices);

	    free (ptr);
	}
	else
	{
	    glitz_geometry_enable_none (gl, surface, ext);
	    gl->draw_arrays (GLITZ_GL_QUADS, 0, 4);
	}

	glitz_texture_unbind (gl, texture);

	REGION_EMPTY (&surface->drawable_damage);
    }
}

void
glitz_surface_sync_solid (glitz_surface_t *surface)
{
    if (SURFACE_SOLID_DAMAGE (surface))
    {
	glitz_gl_float_t *c, color[64];
	glitz_texture_t *texture;

	GLITZ_GL_SURFACE (surface);

	texture = glitz_surface_get_texture (surface, 0);

	c = &color[(texture->box.y1 * texture->width + texture->box.x1) * 4];
	if (texture)
	{
	    glitz_texture_bind (gl, texture);
	    gl->get_tex_image (texture->target, 0,
			       GLITZ_GL_RGBA, GLITZ_GL_FLOAT, color);
	    glitz_texture_unbind (gl, texture);
	}
	else
	{
	    c[0] = c[1] = c[2] = 0.0f;
	    c[3] = 1.0f;
	}

	surface->solid.red = c[0] * 65535.0f;
	surface->solid.green = c[1] * 65535.0f;
	surface->solid.blue = c[2] * 65535.0f;
	surface->solid.alpha = c[3] * 65535.0f;

	surface->flags &= ~GLITZ_SURFACE_FLAG_SOLID_DAMAGE_MASK;
    }
}

glitz_texture_t *
glitz_surface_get_texture (glitz_surface_t *surface,
			   glitz_bool_t    allocate)
{
    GLITZ_GL_SURFACE (surface);

    if (REGION_NOTEMPTY (&surface->texture_damage))
    {
	_glitz_surface_sync_texture (surface);
    }
    else if (allocate)
    {
	if (!(TEXTURE_ALLOCATED (&surface->texture)))
	    glitz_texture_allocate (gl, &surface->texture);
    }

    if (TEXTURE_ALLOCATED (&surface->texture))
	return &surface->texture;

    return NULL;
}

void
glitz_surface_damage (glitz_surface_t *surface,
		      glitz_box_t     *box,
		      int             what)
{
    if (surface->attached && !DRAWABLE_IS_FBO (surface->attached))
    {
	if (box)
	{
	    if (what & GLITZ_DAMAGE_DRAWABLE_MASK)
		REGION_UNION (&surface->drawable_damage, box);

	    if (surface->attached && (what & GLITZ_DAMAGE_TEXTURE_MASK))
		REGION_UNION (&surface->texture_damage, box);
	}
	else
	{
	    if (what & GLITZ_DAMAGE_DRAWABLE_MASK)
	    {
		REGION_EMPTY (&surface->drawable_damage);
		REGION_INIT (&surface->drawable_damage, &surface->box);
	    }

	    if (surface->attached && (what & GLITZ_DAMAGE_TEXTURE_MASK))
	    {
		REGION_EMPTY (&surface->texture_damage);
		REGION_INIT (&surface->texture_damage, &surface->box);
	    }
	}
    }

    if (what & GLITZ_DAMAGE_SOLID_MASK)
	surface->flags |= GLITZ_SURFACE_FLAG_SOLID_DAMAGE_MASK;
}

void
glitz_surface_status_add (glitz_surface_t *surface,
			  int             flags)
{
    surface->status_mask |= flags;
}

static void
_glitz_surface_update_state (glitz_surface_t *surface)
{
    glitz_drawable_t  *drawable;
    int               width, height;

    GLITZ_GL_SURFACE (surface);

    drawable = surface->attached;
    width    = drawable->width;
    height   = drawable->height;

    if (drawable->update_all                         ||
	drawable->viewport.x      != surface->x      ||
	drawable->viewport.y      != surface->y      ||
	drawable->viewport.width  != surface->box.x2 ||
	drawable->viewport.height != surface->box.y2)
    {
	gl->viewport (surface->x,
		      height - surface->y - surface->box.y2,
		      surface->box.x2,
		      surface->box.y2);
	gl->matrix_mode (GLITZ_GL_PROJECTION);
	gl->load_identity ();
	gl->ortho (0.0,
		   surface->box.x2,
		   height - surface->box.y2,
		   height,
		   -1.0, 1.0);
	gl->matrix_mode (GLITZ_GL_MODELVIEW);
	gl->load_identity ();
	gl->scale_f (1.0f, -1.0f, 1.0f);
	gl->translate_f (0.0f, -height, 0.0f);

	drawable->viewport.x      = surface->x;
	drawable->viewport.y      = surface->y;
	drawable->viewport.width  = surface->box.x2;
	drawable->viewport.height = surface->box.y2;

	drawable->update_all = 0;
    }

    drawable->backend->draw_buffer (drawable, surface->buffer);

    if (SURFACE_DITHER (surface))
	gl->enable (GLITZ_GL_DITHER);
    else
	gl->disable (GLITZ_GL_DITHER);
}

void
glitz_surface_attach (glitz_surface_t         *surface,
		      glitz_drawable_t        *drawable,
		      glitz_drawable_buffer_t buffer)
{
    if (drawable)
    {
	if (buffer == GLITZ_DRAWABLE_BUFFER_FRONT_COLOR)
	{
	    if (surface == drawable->front)
		return;

	    if (surface->format->color.fourcc != GLITZ_FOURCC_RGB)
		drawable = NULL;

	    if (drawable)
	    {
		glitz_drawable_reference (drawable);
		if (drawable->front)
		    glitz_surface_detach (drawable->front);

		drawable->front = surface;
	    }

	    surface->buffer = GLITZ_GL_FRONT;
	}
	else if ((buffer == GLITZ_DRAWABLE_BUFFER_BACK_COLOR) &&
		 drawable->format->d.doublebuffer)
	{
	    if (surface == drawable->back)
		return;

	    if (surface->format->color.fourcc != GLITZ_FOURCC_RGB)
		drawable = NULL;

	    if (drawable)
	    {
		glitz_drawable_reference (drawable);
		if (drawable->back)
		    glitz_surface_detach (drawable->back);

		drawable->back = surface;
	    }

	    surface->buffer = GLITZ_GL_BACK;
	}
	else
	    drawable = NULL;
    }

    if (surface->attached)
	glitz_surface_detach (surface);

    surface->attached = drawable;
    if (drawable)
    {
	surface->attached->backend->attach_notify (drawable, surface);

	if (TEXTURE_ALLOCATED (&surface->texture))
	    glitz_surface_damage (surface, NULL, GLITZ_DAMAGE_DRAWABLE_MASK);

	if ((!SURFACE_SOLID (surface)) || SURFACE_SOLID_DAMAGE (surface))
	    REGION_EMPTY (&surface->texture_damage);
    }
}

void
glitz_surface_detach (glitz_surface_t *surface)
{
    if (!surface->attached)
	return;

    if (REGION_NOTEMPTY (&surface->texture_damage))
    {
	glitz_surface_push_current (surface, GLITZ_DRAWABLE_CURRENT);
	_glitz_surface_sync_texture (surface);
	glitz_surface_pop_current (surface);
    }

    surface->attached->backend->detach_notify (surface->attached, surface);

    if (surface == surface->attached->front)
	surface->attached->front = NULL;

    if (surface == surface->attached->back)
	surface->attached->back = NULL;

    glitz_drawable_destroy (surface->attached);

    surface->attached = NULL;

    REGION_EMPTY (&surface->drawable_damage);
    REGION_INIT (&surface->drawable_damage, &surface->box);
}

glitz_drawable_t *
glitz_surface_get_drawable (glitz_surface_t *surface)
{
    return surface->drawable;
}
slim_hidden_def(glitz_surface_get_drawable);

glitz_drawable_t *
glitz_surface_get_attached_drawable (glitz_surface_t *surface)
{
    return surface->attached;
}
slim_hidden_def(glitz_surface_get_attached_drawable);

glitz_bool_t
glitz_surface_push_current (glitz_surface_t    *surface,
			    glitz_constraint_t constraint)
{
    glitz_drawable_t *drawable;

    if (surface->attached)
    {
	drawable = surface->attached;
	if (drawable->backend->push_current (drawable, surface,
					     constraint, NULL))
	{
	    if (constraint == GLITZ_DRAWABLE_CURRENT)
	    {
		if (drawable->backend->feature_mask &
		    GLITZ_FEATURE_FRAMEBUFFER_OBJECT_MASK)
		{
		    if (!surface->fb)
		    {
			GLITZ_GL_DRAWABLE (drawable);

			gl->bind_framebuffer (GLITZ_GL_FRAMEBUFFER, 0);

			_glitz_surface_update_state (surface);
			glitz_surface_sync_drawable (surface);
		    }
		    else
		    {
			_glitz_surface_update_state (surface);
		    }
		}
		else
		{
		    _glitz_surface_update_state (surface);
		    glitz_surface_sync_drawable (surface);
		}
	    }

	    return 1;
	}

	return 0;
    }
    else
    {
	drawable = surface->drawable;

	if (constraint == GLITZ_DRAWABLE_CURRENT)
	{
	    drawable->backend->push_current (drawable, surface,
					     GLITZ_ANY_CONTEXT_CURRENT, NULL);
	}
	else
	{
	    return drawable->backend->push_current (drawable, surface,
						    constraint, NULL);
	}
    }

    return 0;
}

void
glitz_surface_pop_current (glitz_surface_t *surface)
{
    glitz_drawable_t *drawable;
    glitz_surface_t  *other;

    drawable = (surface->attached) ? surface->attached : surface->drawable;

    other = drawable->backend->pop_current (drawable);
    if (other)
    {
	if (other->fb)
	    drawable->backend->gl->bind_framebuffer (GLITZ_GL_FRAMEBUFFER,
						     other->fb);

	_glitz_surface_update_state (other);
    }
}

void
glitz_surface_set_transform (glitz_surface_t   *surface,
			     glitz_transform_t *transform)
{
    static glitz_transform_t identity = {
	{
	    { FIXED1, 0x00000, 0x00000 },
	    { 0x00000, FIXED1, 0x00000 },
	    { 0x00000, 0x00000, FIXED1 }
	}
    };

    if (transform &&
	memcmp (transform, &identity, sizeof (glitz_transform_t)) == 0)
	transform = NULL;

    if (transform) {
	glitz_gl_float_t height, *m, *t;

	if (!surface->transform) {
	    surface->transform = malloc (sizeof (glitz_matrix_t));
	    if (surface->transform == NULL) {
		glitz_surface_status_add (surface,
					  GLITZ_STATUS_NO_MEMORY_MASK);
		return;
	    }
	}

	m = surface->transform->m;
	t = surface->transform->t;

	m[0] = FIXED_TO_FLOAT (transform->matrix[0][0]);
	m[4] = FIXED_TO_FLOAT (transform->matrix[0][1]);
	m[8] = 0.0f;
	m[12] = FIXED_TO_FLOAT (transform->matrix[0][2]);

	m[1] = FIXED_TO_FLOAT (transform->matrix[1][0]);
	m[5] = FIXED_TO_FLOAT (transform->matrix[1][1]);
	m[9] = 0.0f;
	m[13] = FIXED_TO_FLOAT (transform->matrix[1][2]);

	m[2] = 0.0f;
	m[6] = 0.0f;
	m[10] = 1.0f;
	m[14] = 0.0f;

	m[3] = FIXED_TO_FLOAT (transform->matrix[2][0]);
	m[7] = FIXED_TO_FLOAT (transform->matrix[2][1]);
	m[11] = 0.0f;
	m[15] = FIXED_TO_FLOAT (transform->matrix[2][2]);

	/* Projective transformation matrix conversion to normalized
	   texture coordinates seems to be working fine. However, it would be
	   good if someone could verify that this is actually a correct way for
	   doing this.

	   We need to do this:

	   scale (IDENTITY, 0, -1)
	   translate (IDENTITY, 0, -texture_height)
	   multiply (TEXTURE_MATRIX, IDENTITY, MATRIX)
	   scale (TEXTURE_MATRIX, 0, -1)
	   translate (TEXTURE_MATRIX, 0, -texture_height)

	   the following code does it pretty efficiently. */

	height = surface->texture.texcoord_height_unit *
	    (surface->texture.box.y2 - surface->texture.box.y1);

	t[0] = m[0];
	t[4] = m[4];
	t[8] = 0.0f;
	t[12] = surface->texture.texcoord_width_unit * m[12];

	t[3] = m[3] / surface->texture.texcoord_width_unit;
	t[7] = m[7] / surface->texture.texcoord_height_unit;
	t[11] = 0.0f;
	t[15] = m[15];

	t[1] = height * t[3] - m[1];
	t[5] = height * t[7] - m[5];
	t[9] = 0.0f;
	t[13] = height * t[15] - surface->texture.texcoord_height_unit * m[13];

	t[2] = 0.0f;
	t[6] = 0.0f;
	t[10] = 1.0f;
	t[14] = 0.0f;

	/* scale y = -1 */
	t[4] = -t[4];
	t[5] = -t[5];
	t[7] = -t[7];

	/* translate y = -texture_height */
	t[12] -= t[4] * height;
	t[13] -= t[5] * height;
	t[15] -= t[7] * height;

	/* Translate coordinates into texture. This only makes a difference
	   when GL_ARB_texture_border_clamp is missing as box.x1 and box.y1 are
	   otherwise always zero. This breaks projective transformations so
	   those wont work without GL_ARB_texture_border_clamp. */
	t[12] += surface->texture.texcoord_width_unit *
	    surface->texture.box.x1;
	t[13] += surface->texture.texcoord_height_unit *
	    surface->texture.box.y1;

	surface->flags |= GLITZ_SURFACE_FLAG_TRANSFORM_MASK;
	if (m[3] != 0.0f || m[7] != 0.0f || (m[15] != 1.0f && m[15] != -1.0f))
	    surface->flags |= GLITZ_SURFACE_FLAG_PROJECTIVE_TRANSFORM_MASK;
    } else {
	if (surface->transform)
	    free (surface->transform);

	surface->transform = NULL;
	surface->flags &= ~GLITZ_SURFACE_FLAG_TRANSFORM_MASK;
	surface->flags &= ~GLITZ_SURFACE_FLAG_PROJECTIVE_TRANSFORM_MASK;
    }
}
slim_hidden_def(glitz_surface_set_transform);

void
glitz_surface_set_fill (glitz_surface_t *surface,
			glitz_fill_t    fill)
{
    switch (fill) {
    case GLITZ_FILL_TRANSPARENT:
	surface->flags &= ~GLITZ_SURFACE_FLAG_REPEAT_MASK;
	surface->flags &= ~GLITZ_SURFACE_FLAG_MIRRORED_MASK;
	surface->flags &= ~GLITZ_SURFACE_FLAG_PAD_MASK;
	break;
    case GLITZ_FILL_NEAREST:
	surface->flags &= ~GLITZ_SURFACE_FLAG_REPEAT_MASK;
	surface->flags &= ~GLITZ_SURFACE_FLAG_MIRRORED_MASK;
	surface->flags |= GLITZ_SURFACE_FLAG_PAD_MASK;
	break;
    case GLITZ_FILL_REPEAT:
	surface->flags |= GLITZ_SURFACE_FLAG_REPEAT_MASK;
	surface->flags &= ~GLITZ_SURFACE_FLAG_MIRRORED_MASK;
	surface->flags &= ~GLITZ_SURFACE_FLAG_PAD_MASK;
	break;
    case GLITZ_FILL_REFLECT:
	surface->flags |= GLITZ_SURFACE_FLAG_REPEAT_MASK;
	surface->flags |= GLITZ_SURFACE_FLAG_MIRRORED_MASK;
	surface->flags &= ~GLITZ_SURFACE_FLAG_PAD_MASK;
	break;
    }

    glitz_filter_set_type (surface, surface->filter);
}
slim_hidden_def(glitz_surface_set_fill);

void
glitz_surface_set_component_alpha (glitz_surface_t *surface,
				   glitz_bool_t    component_alpha)
{
    if (component_alpha && surface->format->color.red_size)
	surface->flags |= GLITZ_SURFACE_FLAG_COMPONENT_ALPHA_MASK;
    else
	surface->flags &= ~GLITZ_SURFACE_FLAG_COMPONENT_ALPHA_MASK;
}
slim_hidden_def(glitz_surface_set_component_alpha);

void
glitz_surface_set_filter (glitz_surface_t    *surface,
			  glitz_filter_t     filter,
			  glitz_fixed16_16_t *params,
			  int                n_params)
{
    glitz_status_t status;

    status = glitz_filter_set_params (surface, filter, params, n_params);
    if (status) {
	glitz_surface_status_add (surface,
				  glitz_status_to_status_mask (status));
    } else {
	switch (filter) {
	case GLITZ_FILTER_NEAREST:
	    switch (surface->format->color.fourcc) {
	    case GLITZ_FOURCC_YV12:
		surface->flags |= GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
		break;
	    default:
		surface->flags &= ~GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
	    }
	    surface->flags &= ~GLITZ_SURFACE_FLAG_LINEAR_TRANSFORM_FILTER_MASK;
	    surface->flags &= ~GLITZ_SURFACE_FLAG_IGNORE_WRAP_MASK;
	    surface->flags &= ~GLITZ_SURFACE_FLAG_EYE_COORDS_MASK;
	    break;
	case GLITZ_FILTER_BILINEAR:
	    switch (surface->format->color.fourcc) {
	    case GLITZ_FOURCC_YV12:
		surface->flags |= GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
		break;
	    default:
		surface->flags &= ~GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
	    }
	    surface->flags |= GLITZ_SURFACE_FLAG_LINEAR_TRANSFORM_FILTER_MASK;
	    surface->flags &= ~GLITZ_SURFACE_FLAG_IGNORE_WRAP_MASK;
	    surface->flags &= ~GLITZ_SURFACE_FLAG_EYE_COORDS_MASK;
	    break;
	case GLITZ_FILTER_CONVOLUTION:
	case GLITZ_FILTER_GAUSSIAN:
	    surface->flags |= GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
	    surface->flags |= GLITZ_SURFACE_FLAG_LINEAR_TRANSFORM_FILTER_MASK;
	    surface->flags &= ~GLITZ_SURFACE_FLAG_IGNORE_WRAP_MASK;
	    surface->flags &= ~GLITZ_SURFACE_FLAG_EYE_COORDS_MASK;
	    break;
	case GLITZ_FILTER_LINEAR_GRADIENT:
	case GLITZ_FILTER_RADIAL_GRADIENT:
	    surface->flags |= GLITZ_SURFACE_FLAG_FRAGMENT_FILTER_MASK;
	    surface->flags &= ~GLITZ_SURFACE_FLAG_LINEAR_TRANSFORM_FILTER_MASK;
	    surface->flags |= GLITZ_SURFACE_FLAG_IGNORE_WRAP_MASK;
	    surface->flags |= GLITZ_SURFACE_FLAG_EYE_COORDS_MASK;
	    break;
	}
	surface->filter = filter;
    }
}
slim_hidden_def(glitz_surface_set_filter);

void
glitz_surface_set_dither (glitz_surface_t *surface,
			  glitz_bool_t    dither)
{
    if (dither)
	surface->flags |= GLITZ_SURFACE_FLAG_DITHER_MASK;
    else
	surface->flags &= ~GLITZ_SURFACE_FLAG_DITHER_MASK;
}
slim_hidden_def(glitz_surface_set_dither);

void
glitz_surface_flush (glitz_surface_t *surface)
{
    if (!surface->attached)
	return;

    if (!DRAWABLE_IS_FBO (surface->attached))
    {
	if (REGION_NOTEMPTY (&surface->drawable_damage))
	{
	    glitz_surface_push_current (surface, GLITZ_DRAWABLE_CURRENT);
	    glitz_surface_pop_current (surface);
	}
    }
}
slim_hidden_def(glitz_surface_flush);

unsigned int
glitz_surface_get_width (glitz_surface_t *surface)
{
    return (unsigned int) surface->box.x2;
}
slim_hidden_def(glitz_surface_get_width);

unsigned int
glitz_surface_get_height (glitz_surface_t *surface)
{
    return (unsigned int) surface->box.y2;
}
slim_hidden_def(glitz_surface_get_height);

glitz_status_t
glitz_surface_get_status (glitz_surface_t *surface)
{
    return glitz_status_pop_from_mask (&surface->status_mask);
}
slim_hidden_def(glitz_surface_get_status);

glitz_format_t *
glitz_surface_get_format (glitz_surface_t *surface)
{
    return surface->format;
}
slim_hidden_def(glitz_surface_get_format);

void
glitz_surface_translate_point (glitz_surface_t     *surface,
			       glitz_point_fixed_t *src,
			       glitz_point_fixed_t *dst)
{

    if (surface->texture.target == GLITZ_GL_TEXTURE_2D)
    {
	dst->x = (INT_TO_FIXED (surface->texture.box.x1) + src->x) /
	    surface->texture.width;
	dst->y = (INT_TO_FIXED (surface->texture.box.y2) - src->y) /
	    surface->texture.height;
    }
    else
    {
	dst->x = INT_TO_FIXED (surface->texture.box.x1) + src->x;
	dst->y = INT_TO_FIXED (surface->texture.box.y2) - src->y;
    }
}
slim_hidden_def(glitz_surface_translate_point);

void
glitz_surface_set_clip_region (glitz_surface_t *surface,
			       int             x_origin,
			       int             y_origin,
			       glitz_box_t     *box,
			       int             n_box)
{
    if (n_box)
    {
	surface->clip   = box;
	surface->n_clip = n_box;
	surface->x_clip = x_origin;
	surface->y_clip = y_origin;
    }
    else
    {
	surface->clip   = &surface->box;
	surface->n_clip = 1;
	surface->x_clip = surface->y_clip = 0;
    }
}
slim_hidden_def(glitz_surface_set_clip_region);
