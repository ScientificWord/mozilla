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
 * The Original Code is Novell code.
 *
 * The Initial Developer of the Original Code is Novell Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Robert O'Callahan (rocallahan@novell.com)
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

#include "gfxRect.h"

#include "nsMathUtils.h"

gfxRect
gfxRect::Intersect(const gfxRect& aRect) const
{
  gfxRect result(0,0,0,0);

  gfxFloat x = PR_MAX(aRect.X(), X());
  gfxFloat xmost = PR_MIN(aRect.XMost(), XMost());
  if (x >= xmost)
    return result;

  gfxFloat y = PR_MAX(aRect.Y(), Y());
  gfxFloat ymost = PR_MIN(aRect.YMost(), YMost());
  if (y >= ymost)
    return result;

  result = gfxRect(x, y, xmost - x, ymost - y);
  return result;
}

gfxRect
gfxRect::Union(const gfxRect& aRect) const
{
  if (IsEmpty())
    return aRect;
  if (aRect.IsEmpty())
    return *this;

  gfxFloat x = PR_MIN(aRect.X(), X());
  gfxFloat xmost = PR_MAX(aRect.XMost(), XMost());
  gfxFloat y = PR_MIN(aRect.Y(), Y());
  gfxFloat ymost = PR_MAX(aRect.YMost(), YMost());
  return gfxRect(x, y, xmost - x, ymost - y);
}

void
gfxRect::Round()
{
    // Note that don't use NS_round here. See the comment for this method in gfxRect.h
    gfxFloat x0 = NS_floor(X() + 0.5);
    gfxFloat y0 = NS_floor(Y() + 0.5);
    gfxFloat x1 = NS_floor(XMost() + 0.5);
    gfxFloat y1 = NS_floor(YMost() + 0.5);

    pos.x = x0;
    pos.y = y0;

    size.width = x1 - x0;
    size.height = y1 - y0;
}

/* Clamp r to CAIRO_COORD_MIN .. CAIRO_COORD_MAX
 * these are to be device coordinates.
 */

#define CAIRO_COORD_MAX (16382.0)
#define CAIRO_COORD_MIN (-16383.0)

void
gfxRect::Condition()
{
    // if either x or y is way out of bounds;
    // note that we don't handle negative w/h here
    if (pos.x > CAIRO_COORD_MAX) {
        pos.x = CAIRO_COORD_MAX;
        size.width = 0.0;
    } 

    if (pos.y > CAIRO_COORD_MAX) {
        pos.y = CAIRO_COORD_MAX;
        size.height = 0.0;
    }

    if (pos.x < CAIRO_COORD_MIN) {
        size.width += pos.x - CAIRO_COORD_MIN;
        if (size.width < 0.0)
            size.width = 0.0;
        pos.x = CAIRO_COORD_MIN;
    }

    if (pos.y < CAIRO_COORD_MIN) {
        size.height += pos.y - CAIRO_COORD_MIN;
        if (size.height < 0.0)
            size.height = 0.0;
        pos.y = CAIRO_COORD_MIN;
    }

    if (pos.x + size.width > CAIRO_COORD_MAX) {
        size.width = CAIRO_COORD_MAX - pos.x;
    }

    if (pos.y + size.height > CAIRO_COORD_MAX) {
        size.height = CAIRO_COORD_MAX - pos.y;
    }
}
