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
 * The Original Code is thebes gfx code.
 *
 * The Initial Developer of the Original Code is Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
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

#ifndef GFX_ATSUIFONTS_H
#define GFX_ATSUIFONTS_H

#include "gfxTypes.h"
#include "gfxFont.h"

#include <Carbon/Carbon.h>

class gfxAtsuiFontGroup;

class gfxAtsuiFont : public gfxFont {
public:
    gfxAtsuiFont(ATSUFontID fontID,
                 const gfxFontStyle *fontStyle);
    virtual ~gfxAtsuiFont();

    virtual const gfxFont::Metrics& GetMetrics();

    float GetCharWidth (PRUnichar c);

    ATSUFontID GetATSUFontID() { return mATSUFontID; }

    cairo_font_face_t *CairoFontFace() { return mFontFace; }
    cairo_scaled_font_t *CairoScaledFont() { return mScaledFont; }

    ATSUStyle GetATSUStyle() { return mATSUStyle; }

protected:
    const gfxFontStyle *mFontStyle;

    ATSUFontID mATSUFontID;
    ATSUStyle mATSUStyle;

    cairo_font_face_t *mFontFace;
    cairo_scaled_font_t *mScaledFont;

    gfxFont::Metrics mMetrics;
};

class THEBES_API gfxAtsuiFontGroup : public gfxFontGroup {
public:
    gfxAtsuiFontGroup(const nsAString& families,
                      const gfxFontStyle *aStyle);
    virtual ~gfxAtsuiFontGroup();

    virtual gfxTextRun *MakeTextRun(const nsAString& aString);
    virtual gfxTextRun *MakeTextRun(const nsACString& aCString) {
        return MakeTextRun(NS_ConvertASCIItoUTF16(aCString));
    }

    ATSUFontFallbacks *GetATSUFontFallbacksPtr() { return &mFallbacks; }
    
    gfxAtsuiFont* GetFontAt(PRInt32 i) {
        return NS_STATIC_CAST(gfxAtsuiFont*, NS_STATIC_CAST(gfxFont*, mFonts[i]));
    }

protected:
    static PRBool FindATSUFont(const nsAString& aName,
                               const nsACString& aGenericName,
                               void *closure);

    ATSUFontFallbacks mFallbacks;
};

class THEBES_API gfxAtsuiTextRun : public gfxTextRun {
    THEBES_DECL_ISUPPORTS_INHERITED
public:
    gfxAtsuiTextRun(const nsAString& aString, gfxAtsuiFontGroup *aFontGroup);
    ~gfxAtsuiTextRun();

    virtual void Draw(gfxContext *aContext, gfxPoint pt);
    virtual gfxFloat Measure(gfxContext *aContext);

    virtual void SetSpacing(const nsTArray<gfxFloat>& spacingArray);
    virtual const nsTArray<gfxFloat> *const GetSpacing() const;

private:
    nsString mString;
    gfxAtsuiFontGroup *mGroup;

    ATSUTextLayout mATSULayout;

    nsTArray<ATSUStyle> mStylesToDispose;
};

#endif /* GFX_ATSUIFONTS_H */
