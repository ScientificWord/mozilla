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
 * The Original Code is thebes
 *
 * The Initial Developer of the Original Code is
 *   mozilla.org
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

#include "gfxGlitzSurface.h"

gfxGlitzSurface::gfxGlitzSurface(glitz_drawable_t *drawable, glitz_surface_t *surface, PRBool takeOwnership)
    : mGlitzDrawable (drawable), mGlitzSurface(surface), mOwnsSurface(takeOwnership)
{
    cairo_surface_t *surf = cairo_glitz_surface_create (mGlitzSurface);
    Init(surf);
}

gfxGlitzSurface::~gfxGlitzSurface()
{
    if (mOwnsSurface) {
        if (mGlitzSurface) {
            glitz_surface_flush(mGlitzSurface);
            glitz_surface_destroy(mGlitzSurface);
        }

        if (mGlitzDrawable) {
            glitz_drawable_flush(mGlitzDrawable);
            glitz_drawable_finish(mGlitzDrawable);
            glitz_drawable_destroy(mGlitzDrawable);
        }
    }
}

void
gfxGlitzSurface::SwapBuffers()
{
    glitz_drawable_swap_buffers (GlitzDrawable());
}

unsigned long
gfxGlitzSurface::Width()
{
    return glitz_drawable_get_width (GlitzDrawable());
}

unsigned long
gfxGlitzSurface::Height()
{
    return glitz_drawable_get_height (GlitzDrawable());
}
