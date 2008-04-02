/* vim:set sw=4 sts=4 et cin: */
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
 * The Original Code is BeOS code in Thebes.
 *
 * The Initial Developer of the Original Code is
 * Christian Biesinger <cbiesinger@web.de>.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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


#include <Bitmap.h>

#include "gfxBeOSSurface.h"

gfxBeOSSurface::gfxBeOSSurface(BView* aView) :
    mOwnsView(PR_FALSE), mView(aView), mBitmap(nsnull)
{
    Init(cairo_beos_surface_create(aView));
}

gfxBeOSSurface::gfxBeOSSurface(BView* aView, BBitmap* aBitmap) :
   mOwnsView(PR_FALSE), mView(aView), mBitmap(nsnull)
{
    Init(cairo_beos_surface_create_for_bitmap(aView, aBitmap));
}

gfxBeOSSurface::gfxBeOSSurface(unsigned long width, unsigned long height, color_space space)
{
    BRect bounds(0.0, 0.0, width - 1, height - 1);
    mBitmap = new BBitmap(bounds, space, true);
    mView = new BView(bounds, "Mozilla Bitmap view", B_FOLLOW_ALL_SIDES, 0);
    mBitmap->AddChild(mView);

    mOwnsView = PR_TRUE;
    Init(cairo_beos_surface_create_for_bitmap(mView, mBitmap));
}

gfxBeOSSurface::~gfxBeOSSurface()
{
    if (mOwnsView) {
        mBitmap->RemoveChild(mView);

        delete mView;
        delete mBitmap;
    }
}
