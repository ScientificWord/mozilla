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
 * The Original Code is Mozilla Foundation code.
 *
 * The Initial Developer of the Original Code is Mozilla Foundation.
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

#define PANGO_ENABLE_BACKEND
#define PANGO_ENABLE_ENGINE

#include "gfxPlatformGtk.h"

#include "nsIAtom.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#include "gfxImageSurface.h"
#include "gfxXlibSurface.h"

#ifdef MOZ_ENABLE_GLITZ
#include "gfxGlitzSurface.h"
#include "glitz-glx.h"
#endif

#ifndef THEBES_USE_PANGO_CAIRO
#include <fontconfig/fontconfig.h>
#include <pango/pangoxft.h>
#endif // THEBES_USE_PANGO_CAIRO

#include <pango/pango-font.h>

#include "nsUnitConversion.h"

PRInt32 gfxPlatformGtk::sDPI = -1;

static cairo_user_data_key_t cairo_gdk_window_key;
static cairo_user_data_key_t cairo_gdk_pixmap_key;
static void do_gdk_pixmap_unref (void *data)
{
    GdkPixmap *pmap = (GdkPixmap*)data;
    gdk_pixmap_unref (pmap);
}

gfxPlatformGtk::gfxPlatformGtk()
{
#ifdef MOZ_ENABLE_GLITZ
    if (UseGlitz())
        glitz_glx_init(NULL);
#endif
}

already_AddRefed<gfxASurface>
gfxPlatformGtk::CreateOffscreenSurface(PRUint32 width,
                                       PRUint32 height,
                                       gfxASurface::gfxImageFormat imageFormat)
{
    gfxASurface *newSurface = nsnull;

    int glitzf;
    int xrenderFormatID;
    switch (imageFormat) {
        case gfxASurface::ImageFormatARGB32:
            glitzf = 0; // GLITZ_STANDARD_ARGB32;
            xrenderFormatID = PictStandardARGB32;
            break;
        case gfxASurface::ImageFormatRGB24:
            glitzf = 1; // GLITZ_STANDARD_RGB24;
            xrenderFormatID = PictStandardRGB24;
            break;
        case gfxASurface::ImageFormatA8:
            glitzf = 2; // GLITZ_STANDARD_A8;
            xrenderFormatID = PictStandardA8;
            break;
        case gfxASurface::ImageFormatA1:
            glitzf = 3; // GLITZ_STANDARD_A1;
            xrenderFormatID = PictStandardA1;
            break;
        default:
            return nsnull;
    }

    // XXX we really need a different interface here, something that passes
    // in more context, including the display and/or target surface type that
    // we should try to match
    Display* display = GDK_DISPLAY();
    if (!UseGlitz()) {
        GdkPixmap* pixmap = nsnull;
        XRenderPictFormat* xrenderFormat =
            XRenderFindStandardFormat(display, xrenderFormatID);
        if (!xrenderFormat) {
            // We don't have Render; see if we can just create a pixmap
            // of the requested depth.  Otherwise, create an Image surface.
            GdkVisual* vis;

            if (imageFormat == gfxASurface::ImageFormatRGB24) {
                vis = gdk_rgb_get_visual();
                if (vis->type == GDK_VISUAL_TRUE_COLOR)
                    pixmap = gdk_pixmap_new(nsnull, width, height, vis->depth);
            }

            if (pixmap) {
                gdk_drawable_set_colormap(GDK_DRAWABLE(pixmap), nsnull);
                newSurface = new gfxXlibSurface(display,
                                                GDK_PIXMAP_XID(GDK_DRAWABLE(pixmap)),
                                                GDK_VISUAL_XVISUAL(vis),
                                                width,
                                                height);
            } else {
                // we couldn't create a Gdk Pixmap; fall back to image surface for the data
                newSurface = new gfxImageSurface(imageFormat, width, height);
            }
        } else {
            pixmap = gdk_pixmap_new(nsnull, width, height,
                                    xrenderFormat->depth);
            gdk_drawable_set_colormap(GDK_DRAWABLE(pixmap), nsnull);

            newSurface = new gfxXlibSurface(display,
                                            GDK_PIXMAP_XID(GDK_DRAWABLE(pixmap)),
                                            xrenderFormat,
                                            width, height);
        }

        if (pixmap && newSurface) {
            // set up the surface to auto-unref the gdk pixmap when the surface
            // is released
            newSurface->SetData(&cairo_gdk_pixmap_key,
                                pixmap,
                                do_gdk_pixmap_unref);
        }

    } else {
#ifdef MOZ_ENABLE_GLITZ
        glitz_drawable_format_t *gdformat = glitz_glx_find_pbuffer_format
            (display,
             gdk_x11_get_default_screen(),
             0, NULL, 0);

        glitz_drawable_t *gdraw =
            glitz_glx_create_pbuffer_drawable(display,
                                              DefaultScreen(display),
                                              gdformat,
                                              width,
                                              height);
        glitz_format_t *gformat =
            glitz_find_standard_format(gdraw, (glitz_format_name_t)glitzf);

        glitz_surface_t *gsurf =
            glitz_surface_create(gdraw,
                                 gformat,
                                 width,
                                 height,
                                 0,
                                 NULL);

        glitz_surface_attach(gsurf, gdraw, GLITZ_DRAWABLE_BUFFER_FRONT_COLOR);
        newSurface = new gfxGlitzSurface(gdraw, gsurf, PR_TRUE);
#endif
    }

    NS_IF_ADDREF(newSurface);
    return newSurface;
}

GdkDrawable*
gfxPlatformGtk::GetSurfaceGdkDrawable(gfxASurface *aSurf)
{
    GdkDrawable *gd;
    gd = (GdkDrawable*) cairo_surface_get_user_data(aSurf->CairoSurface(), &cairo_gdk_pixmap_key);
    if (gd)
        return gd;

    gd = (GdkDrawable*) cairo_surface_get_user_data(aSurf->CairoSurface(), &cairo_gdk_window_key);
    if (gd)
        return gd;

    return nsnull;
}

void
gfxPlatformGtk::SetSurfaceGdkWindow(gfxASurface *aSurf,
                                    GdkWindow *win)
{
    cairo_surface_set_user_data(aSurf->CairoSurface(),
                                &cairo_gdk_window_key,
                                win,
                                nsnull);
}

// this is in nsFontConfigUtils.h
extern void NS_AddLangGroup (FcPattern *aPattern, nsIAtom *aLangGroup);

nsresult
gfxPlatformGtk::GetFontList(const nsACString& aLangGroup,
                            const nsACString& aGenericFamily,
                            nsStringArray& aListOfFonts)
{
#ifndef THEBES_USE_PANGO_CAIRO
    FcPattern *pat = NULL;
    FcObjectSet *os = NULL;
    FcFontSet *fs = NULL;
    nsresult rv = NS_ERROR_FAILURE;

    aListOfFonts.Clear();

    PRInt32 serif = 0, sansSerif = 0, monospace = 0, nGenerics;

    pat = FcPatternCreate();
    if (!pat)
        goto end;

    os = FcObjectSetBuild(FC_FAMILY, FC_FOUNDRY, 0, NULL);
    if (!os)
        goto end;

    // take the pattern and add the lang group to it
    if (!aLangGroup.IsEmpty()) {
        nsCOMPtr<nsIAtom> langAtom = do_GetAtom(aLangGroup);
        //XXX fix me //NS_AddLangGroup(pat, langAtom);
    }

    fs = FcFontList(0, pat, os);
    if (!fs)
        goto end;

    if (fs->nfont == 0) {
        rv = NS_OK;
        goto end;
    }

    // Fontconfig supports 3 generic fonts, "serif", "sans-serif", and
    // "monospace", slightly different from CSS's 5.
    if (aGenericFamily.IsEmpty())
        serif = sansSerif = monospace = 1;
    else if (aGenericFamily.LowerCaseEqualsLiteral("serif"))
        serif = 1;
    else if (aGenericFamily.LowerCaseEqualsLiteral("sans-serif"))
        sansSerif = 1;
    else if (aGenericFamily.LowerCaseEqualsLiteral("monospace"))
        monospace = 1;
    else if (aGenericFamily.LowerCaseEqualsLiteral("cursive") ||
             aGenericFamily.LowerCaseEqualsLiteral("fantasy"))
        serif = sansSerif = 1;
    else
        NS_NOTREACHED("unexpected CSS generic font family");
    nGenerics = serif + sansSerif + monospace;

    if (serif)
        aListOfFonts.AppendString(NS_LITERAL_STRING("serif"));
    if (sansSerif)
        aListOfFonts.AppendString(NS_LITERAL_STRING("sans-serif"));
    if (monospace)
        aListOfFonts.AppendString(NS_LITERAL_STRING("monospace"));

    for (int i = 0; i < fs->nfont; i++) {
        char *family;

        // if there's no family name, skip this match
        if (FcPatternGetString (fs->fonts[i], FC_FAMILY, 0,
                                (FcChar8 **) &family) != FcResultMatch)
        {
            continue;
        }

        aListOfFonts.AppendString(NS_ConvertASCIItoUTF16(nsDependentCString(family)));
    }

    aListOfFonts.Sort();
    rv = NS_OK;

  end:
    if (NS_FAILED(rv))
        aListOfFonts.Clear();

    if (pat)
        FcPatternDestroy(pat);
    if (os)
        FcObjectSetDestroy(os);
    if (fs)
        FcFontSetDestroy(fs);

    return rv;
#else
    // pango_cairo case; needs to be written
    return NS_ERROR_NOT_IMPLEMENTED;
#endif
}

static PRInt32
GetXftDPI()
{
  char *val = XGetDefault(GDK_DISPLAY(), "Xft", "dpi");
  if (val) {
    char *e;
    double d = strtod(val, &e);

    if (e != val)
      return NSToCoordRound(d);
  }

  return -1;
}

static PRInt32
GetDPIFromPangoFont()
{
#ifndef THEBES_USE_PANGO_CAIRO
    PangoContext* ctx = pango_xft_get_context(GDK_DISPLAY(), 0);
    gdk_pango_context_set_colormap(ctx, gdk_rgb_get_cmap());
#else
    PangoContext* ctx =
        pango_cairo_font_map_create_context(
          PANGO_CAIRO_FONT_MAP(pango_cairo_font_map_get_default()));
#endif

    if (!ctx) {
        return 0;
    }

    double dblDPI = 0.0f;
    GList *items = nsnull;
    PangoItem *item = nsnull;
    PangoFcFont *fcfont = nsnull;
    
    PangoAttrList *al = pango_attr_list_new();

    if (!al) {
        goto cleanup;
    }

    // Just using the string "a" because we need _some_ text.
    items = pango_itemize(ctx, "a", 0, 1, al, NULL);

    if (!items) {
        goto cleanup;
    }

    item = (PangoItem*)items->data;

    if (!item) {
        goto cleanup;
    }

    fcfont = PANGO_FC_FONT(item->analysis.font);

    if (!fcfont) {
        goto cleanup;
    }

    FcPatternGetDouble(fcfont->font_pattern, FC_DPI, 0, &dblDPI);

 cleanup:   
    if (al)
        pango_attr_list_unref(al);
    if (item)
        pango_item_free(item);
    if (items)
        g_list_free(items);
    if (ctx)
        g_object_unref(ctx);

    return NSToCoordRound(dblDPI);
}

/* static */
void
gfxPlatformGtk::InitDPI()
{
    sDPI = GetXftDPI();
    if (sDPI <= 0) {
        sDPI = GetDPIFromPangoFont();
        if (sDPI <= 0) {
            // Fall back to something sane
            sDPI = 96;
        }
    }
}

