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
 *   Stuart Parmenter <stuart@mozilla.com>
 *   Vladimir Vukicevic <vladimir@pobox.com>
 *   Masayuki Nakano <masayuki@d-toybox.com>
 *   Masatoshi Kimura <VYV03354@nifty.ne.jp>
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

#include "gfxWindowsPlatform.h"

#include "gfxImageSurface.h"
#include "gfxWindowsSurface.h"

#include "nsUnicharUtils.h"

#include "nsIPref.h"
#include "nsServiceManagerUtils.h"

#include "nsIWindowsRegKey.h"

#include "gfxWindowsFonts.h"

#include <string>

#include "lcms.h"

//#define DEBUG_CMAP_SIZE 1

int PR_CALLBACK
gfxWindowsPlatform::PrefChangedCallback(const char *aPrefName, void *closure)
{
    // XXX this could be made to only clear out the cache for the prefs that were changed
    // but it probably isn't that big a deal.
    gfxWindowsPlatform *plat = static_cast<gfxWindowsPlatform *>(closure);
    plat->mPrefFonts.Clear();
    return 0;
}

gfxWindowsPlatform::gfxWindowsPlatform()
{
    mFonts.Init(200);
    mFontAliases.Init(20);
    mFontSubstitutes.Init(50);
    mPrefFonts.Init(10);

    UpdateFontList();

    nsCOMPtr<nsIPref> pref = do_GetService(NS_PREF_CONTRACTID);
    pref->RegisterCallback("font.", PrefChangedCallback, this);
    pref->RegisterCallback("font.name-list.", PrefChangedCallback, this);
    pref->RegisterCallback("intl.accept_languages", PrefChangedCallback, this);
    // don't bother unregistering.  We'll get shutdown after the pref service
}

gfxWindowsPlatform::~gfxWindowsPlatform()
{
}

already_AddRefed<gfxASurface>
gfxWindowsPlatform::CreateOffscreenSurface(const gfxIntSize& size,
                                           gfxASurface::gfxImageFormat imageFormat)
{
    gfxASurface *surf = new gfxWindowsSurface(size, imageFormat);
    NS_IF_ADDREF(surf);
    return surf;
}

int CALLBACK 
gfxWindowsPlatform::FontEnumProc(const ENUMLOGFONTEXW *lpelfe,
                                 const NEWTEXTMETRICEXW *nmetrics,
                                 DWORD fontType, LPARAM data)
{
    gfxWindowsPlatform *thisp = reinterpret_cast<gfxWindowsPlatform*>(data);

    const LOGFONTW& logFont = lpelfe->elfLogFont;
    const NEWTEXTMETRICW& metrics = nmetrics->ntmTm;

#ifdef DEBUG_pavlov
    printf("%s %d %d %d\n", NS_ConvertUTF16toUTF8(nsDependentString(logFont.lfFaceName)).get(),
           logFont.lfCharSet, logFont.lfItalic, logFont.lfWeight);
#endif

    // Ignore vertical fonts
    if (logFont.lfFaceName[0] == L'@') {
        return 1;
    }

    nsString name(logFont.lfFaceName);
    ToLowerCase(name);

    nsRefPtr<FontEntry> fe;
    if (!thisp->mFonts.Get(name, &fe)) {
        fe = new FontEntry(nsDependentString(logFont.lfFaceName), (PRUint16)fontType);
        thisp->mFonts.Put(name, fe);
    }

    // mark the charset bit
    fe->mCharset[metrics.tmCharSet] = 1;

    // put the default weight in the weight table
    fe->mWeightTable.SetWeight(PR_MAX(1, PR_MIN(9, metrics.tmWeight / 100)), PR_TRUE);

    // store the default font weight
    fe->mDefaultWeight = metrics.tmWeight;

    if (metrics.ntmFlags & NTM_TYPE1) {
        fe->mSymbolFont = PR_TRUE;
        fe->mUnicodeFont = PR_FALSE;
    }

    fe->mFamily = logFont.lfPitchAndFamily & 0xF0;
    fe->mPitch = logFont.lfPitchAndFamily & 0x0F;

    if (nmetrics->ntmFontSig.fsUsb[0] == 0x00000000 &&
        nmetrics->ntmFontSig.fsUsb[1] == 0x00000000 &&
        nmetrics->ntmFontSig.fsUsb[2] == 0x00000000 &&
        nmetrics->ntmFontSig.fsUsb[3] == 0x00000000) {
        // no unicode ranges
        fe->mUnicodeFont = PR_FALSE;
    } else {
        fe->mUnicodeFont = PR_TRUE;

        // set the unicode ranges
        PRUint32 x = 0;
        for (PRUint32 i = 0; i < 4; ++i) {
            DWORD range = nmetrics->ntmFontSig.fsUsb[i];
            for (PRUint32 k = 0; k < 32; ++k) {
                fe->mUnicodeRanges[x++] = (range & (1 << k)) != 0;
            }
        }
    }

    return 1;
}

// general cmap reading routines moved to gfxFontUtils.cpp

static nsresult
ReadCMAP(HDC hdc, FontEntry *aFontEntry)
{
    const PRUint32 kCMAP = (('c') | ('m' << 8) | ('a' << 16) | ('p' << 24));

    DWORD len = GetFontData(hdc, kCMAP, 0, nsnull, 0);
    if (len == GDI_ERROR || len == 0) // not a truetype font --
        return NS_ERROR_FAILURE;      // we'll treat it as a symbol font

    nsAutoTArray<PRUint8,16384> buffer;
    if (!buffer.AppendElements(len))
        return NS_ERROR_OUT_OF_MEMORY;
    PRUint8 *buf = buffer.Elements();

    DWORD newLen = GetFontData(hdc, kCMAP, 0, buf, len);
    NS_ENSURE_TRUE(newLen == len, NS_ERROR_FAILURE);
    
    return gfxFontUtils::ReadCMAP(buf, len, aFontEntry->mCharacterMap, aFontEntry->mUnicodeRanges,
                    aFontEntry->mUnicodeFont, aFontEntry->mSymbolFont);
}

PLDHashOperator PR_CALLBACK
gfxWindowsPlatform::FontGetCMapDataProc(nsStringHashKey::KeyType aKey,
                                        nsRefPtr<FontEntry>& aFontEntry,
                                        void* userArg)
{
    HDC hdc = GetDC(nsnull);

    LOGFONTW logFont;
    memset(&logFont, 0, sizeof(LOGFONTW));
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfPitchAndFamily = 0;
    PRUint32 l = PR_MIN(aFontEntry->mName.Length(), LF_FACESIZE - 1);
    memcpy(logFont.lfFaceName,
           nsPromiseFlatString(aFontEntry->mName).get(),
           l * sizeof(PRUnichar));
    logFont.lfFaceName[l] = 0;

    HFONT font = CreateFontIndirectW(&logFont);

    if (font) {
        HFONT oldFont = (HFONT)SelectObject(hdc, font);

        nsresult rv = ReadCMAP(hdc, aFontEntry);

        if (NS_FAILED(rv)) {
            aFontEntry->mUnicodeFont = PR_FALSE;
            //printf("%s failed to get cmap\n", NS_ConvertUTF16toUTF8(aFontEntry->mName).get());
        }

        SelectObject(hdc, oldFont);
        DeleteObject(font);
    }

    ReleaseDC(nsnull, hdc);

    if (!aFontEntry->mUnicodeFont) {
        /* non-unicode fonts.. boy lets just set all code points
           between 0x20 and 0xFF.  All the ones on my system do...
           If we really wanted to test which characters in this
           range were supported we could just generate a string with
           each codepoint and do GetGlyphIndicies or similar to determine
           what is there.
        */
        aFontEntry->mCharacterMap.SetRange(0x20, 0xFF);
        return PL_DHASH_NEXT;
    }

    return PL_DHASH_NEXT;
}


struct FontListData {
    FontListData(const nsACString& aLangGroup, const nsACString& aGenericFamily, nsStringArray& aListOfFonts) :
        mLangGroup(aLangGroup), mGenericFamily(aGenericFamily), mStringArray(aListOfFonts) {}
    const nsACString& mLangGroup;
    const nsACString& mGenericFamily;
    nsStringArray& mStringArray;
};

PLDHashOperator PR_CALLBACK
gfxWindowsPlatform::HashEnumFunc(nsStringHashKey::KeyType aKey,
                                 nsRefPtr<FontEntry>& aFontEntry,
                                 void* userArg)
{
    FontListData *data = (FontListData*)userArg;

    /* skip symbol fonts */
    if (aFontEntry->mSymbolFont)
        return PL_DHASH_NEXT;

    if (aFontEntry->SupportsLangGroup(data->mLangGroup) &&
        aFontEntry->MatchesGenericFamily(data->mGenericFamily))
        data->mStringArray.AppendString(aFontEntry->mName);

    return PL_DHASH_NEXT;
}

nsresult
gfxWindowsPlatform::GetFontList(const nsACString& aLangGroup,
                                const nsACString& aGenericFamily,
                                nsStringArray& aListOfFonts)
{
    FontListData data(aLangGroup, aGenericFamily, aListOfFonts);

    mFonts.Enumerate(gfxWindowsPlatform::HashEnumFunc, &data);

    aListOfFonts.Sort();
    aListOfFonts.Compact();

    return NS_OK;
}

static void
RemoveCharsetFromFontSubstitute(nsAString &aName)
{
    PRInt32 comma = aName.FindChar(PRUnichar(','));
    if (comma >= 0)
        aName.Truncate(comma);
}

static void
BuildKeyNameFromFontName(nsAString &aName)
{
    if (aName.Length() >= LF_FACESIZE)
        aName.Truncate(LF_FACESIZE - 1);
    ToLowerCase(aName);
}

nsresult
gfxWindowsPlatform::UpdateFontList()
{
    mFonts.Clear();
    mFontAliases.Clear();
    mNonExistingFonts.Clear();
    mFontSubstitutes.Clear();
    mPrefFonts.Clear();

    LOGFONTW logFont;
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfFaceName[0] = 0;
    logFont.lfPitchAndFamily = 0;

    // Use the screen DC here.. should we use something else for printing?
    HDC dc = ::GetDC(nsnull);
    EnumFontFamiliesExW(dc, &logFont, (FONTENUMPROCW)gfxWindowsPlatform::FontEnumProc, (LPARAM)this, 0);
    ::ReleaseDC(nsnull, dc);

    // Update all the fonts cmaps
    mFonts.Enumerate(gfxWindowsPlatform::FontGetCMapDataProc, nsnull);

    // Create the list of FontSubstitutes
    nsCOMPtr<nsIWindowsRegKey> regKey = do_CreateInstance("@mozilla.org/windows-registry-key;1");
    if (!regKey)
        return NS_ERROR_FAILURE;
     NS_NAMED_LITERAL_STRING(kFontSubstitutesKey, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes");

    nsresult rv = regKey->Open(nsIWindowsRegKey::ROOT_KEY_LOCAL_MACHINE,
                               kFontSubstitutesKey, nsIWindowsRegKey::ACCESS_READ);
    if (NS_FAILED(rv))
        return rv;

    PRUint32 count;
    rv = regKey->GetValueCount(&count);
    if (NS_FAILED(rv) || count == 0)
        return rv;
    for (PRUint32 i = 0; i < count; i++) {
        nsAutoString substituteName;
        rv = regKey->GetValueName(i, substituteName);
        if (NS_FAILED(rv) || substituteName.IsEmpty() ||
            substituteName.CharAt(1) == PRUnichar('@'))
            continue;
        PRUint32 valueType;
        rv = regKey->GetValueType(substituteName, &valueType);
        if (NS_FAILED(rv) || valueType != nsIWindowsRegKey::TYPE_STRING)
            continue;
        nsAutoString actualFontName;
        rv = regKey->ReadStringValue(substituteName, actualFontName);
        if (NS_FAILED(rv))
            continue;

        RemoveCharsetFromFontSubstitute(substituteName);
        BuildKeyNameFromFontName(substituteName);
        RemoveCharsetFromFontSubstitute(actualFontName);
        BuildKeyNameFromFontName(actualFontName);
        nsRefPtr<FontEntry> fe;
        if (!actualFontName.IsEmpty() && mFonts.Get(actualFontName, &fe))
            mFontSubstitutes.Put(substituteName, fe);
        else
            mNonExistingFonts.AppendString(substituteName);
    }

    return NS_OK;
}

struct ResolveData {
    ResolveData(gfxPlatform::FontResolverCallback aCallback,
                gfxWindowsPlatform *aCaller, const nsAString *aFontName,
                void *aClosure) :
        mFoundCount(0), mCallback(aCallback), mCaller(aCaller),
        mFontName(aFontName), mClosure(aClosure) {}
    PRUint32 mFoundCount;
    gfxPlatform::FontResolverCallback mCallback;
    gfxWindowsPlatform *mCaller;
    const nsAString *mFontName;
    void *mClosure;
};

nsresult
gfxWindowsPlatform::ResolveFontName(const nsAString& aFontName,
                                    FontResolverCallback aCallback,
                                    void *aClosure,
                                    PRBool& aAborted)
{
    if (aFontName.IsEmpty())
        return NS_ERROR_FAILURE;

    nsAutoString keyName(aFontName);
    BuildKeyNameFromFontName(keyName);

    nsRefPtr<FontEntry> fe;
    if (mFonts.Get(keyName, &fe) ||
        mFontSubstitutes.Get(keyName, &fe) ||
        mFontAliases.Get(keyName, &fe)) {
        aAborted = !(*aCallback)(fe->mName, aClosure);
        // XXX If the font has font link, we should add the linked font.
        return NS_OK;
    }

    if (mNonExistingFonts.IndexOf(keyName) >= 0) {
        aAborted = PR_FALSE;
        return NS_OK;
    }

    LOGFONTW logFont;
    logFont.lfCharSet = DEFAULT_CHARSET;
    logFont.lfPitchAndFamily = 0;
    PRInt32 len = aFontName.Length();
    if (len >= LF_FACESIZE)
        len = LF_FACESIZE - 1;
    memcpy(logFont.lfFaceName,
           nsPromiseFlatString(aFontName).get(), len * sizeof(PRUnichar));
    logFont.lfFaceName[len] = 0;

    HDC dc = ::GetDC(nsnull);
    ResolveData data(aCallback, this, &keyName, aClosure);
    aAborted =
        !EnumFontFamiliesExW(dc, &logFont,
                             (FONTENUMPROCW)gfxWindowsPlatform::FontResolveProc,
                             (LPARAM)&data, 0);
    if (data.mFoundCount == 0)
        mNonExistingFonts.AppendString(keyName);
    ::ReleaseDC(nsnull, dc);

    return NS_OK;
}

int CALLBACK 
gfxWindowsPlatform::FontResolveProc(const ENUMLOGFONTEXW *lpelfe,
                                    const NEWTEXTMETRICEXW *nmetrics,
                                    DWORD fontType, LPARAM data)
{
    const LOGFONTW& logFont = lpelfe->elfLogFont;
    // Ignore vertical fonts
    if (logFont.lfFaceName[0] == L'@' || logFont.lfFaceName[0] == 0)
        return 1;

    ResolveData *rData = reinterpret_cast<ResolveData*>(data);

    nsAutoString name(logFont.lfFaceName);

    // Save the alias name to cache
    nsRefPtr<FontEntry> fe;
    nsAutoString keyName(name);
    BuildKeyNameFromFontName(keyName);
    if (!rData->mCaller->mFonts.Get(keyName, &fe)) {
        // This case only occurs on failing to build
        // the list of font substitue. In this case, the user should
        // reboot the Windows. Probably, we don't have the good way for
        // resolving in this time.
        NS_WARNING("Cannot find actual font");
        return 1;
    }

    rData->mFoundCount++;
    rData->mCaller->mFontAliases.Put(*(rData->mFontName), fe);

    return (rData->mCallback)(name, rData->mClosure);

    // XXX If the font has font link, we should add the linked font.
}

struct FontSearch {
    FontSearch(const PRUnichar *aString, PRUint32 aLength, gfxWindowsFont *aFont) :
        string(aString), length(aLength), fontToMatch(aFont), matchRank(0) {
    }
    const PRUnichar *string;
    const PRUint32 length;
    nsRefPtr<gfxWindowsFont> fontToMatch;
    PRInt32 matchRank;
    nsRefPtr<FontEntry> bestMatch;
};

PLDHashOperator PR_CALLBACK
gfxWindowsPlatform::FindFontForStringProc(nsStringHashKey::KeyType aKey,
                                          nsRefPtr<FontEntry>& aFontEntry,
                                          void* userArg)
{
    // bitmap fonts suck
    if (aFontEntry->IsCrappyFont())
        return PL_DHASH_NEXT;

    FontSearch *data = (FontSearch*)userArg;

    PRInt32 rank = 0;

    for (PRUint32 i = 0; i < data->length; ++i) {
        PRUint32 ch = data->string[i];

        if ((i+1 < data->length) && NS_IS_HIGH_SURROGATE(ch) && NS_IS_LOW_SURROGATE(data->string[i+1])) {
            i++;
            ch = SURROGATE_TO_UCS4(ch, data->string[i]);
        }

        if (aFontEntry->mCharacterMap.test(ch)) {
            rank += 20;

            // fonts that claim to support the range are more
            // likely to be "better fonts" than ones that don't... (in theory)
            if (aFontEntry->SupportsRange(gfxFontUtils::CharRangeBit(ch)))
                rank += 1;
        }
    }

    // if we didn't match any characters don't bother wasting more time.
    if (rank == 0)
        return PL_DHASH_NEXT;


    if (aFontEntry->SupportsLangGroup(data->fontToMatch->GetStyle()->langGroup))
        rank += 10;

    if (data->fontToMatch->GetFontEntry()->mFamily == aFontEntry->mFamily)
        rank += 5;
    if (data->fontToMatch->GetFontEntry()->mFamily == aFontEntry->mPitch)
        rank += 5;

    /* weight */
    PRInt8 baseWeight, weightDistance;
    data->fontToMatch->GetStyle()->ComputeWeightAndOffset(&baseWeight, &weightDistance);
    PRUint16 targetWeight = (baseWeight * 100) + (weightDistance * 100);
    if (targetWeight == aFontEntry->mDefaultWeight)
        rank += 5;

    if (rank > data->matchRank ||
        (rank == data->matchRank && Compare(aFontEntry->mName, data->bestMatch->mName) > 0)) {
        data->bestMatch = aFontEntry;
        data->matchRank = rank;
    }

    return PL_DHASH_NEXT;
}


FontEntry *
gfxWindowsPlatform::FindFontForString(const PRUnichar *aString, PRUint32 aLength, gfxWindowsFont *aFont)
{
    FontSearch data(aString, aLength, aFont);

    // find fonts that support the character
    mFonts.Enumerate(gfxWindowsPlatform::FindFontForStringProc, &data);

    return data.bestMatch;
}

gfxFontGroup *
gfxWindowsPlatform::CreateFontGroup(const nsAString &aFamilies,
                                    const gfxFontStyle *aStyle)
{
    return new gfxWindowsFontGroup(aFamilies, aStyle);
}

FontEntry *
gfxWindowsPlatform::FindFontEntry(const nsAString& aName)
{
    nsString name(aName);
    ToLowerCase(name);

    nsRefPtr<FontEntry> fe;
    if (!mFonts.Get(name, &fe) &&
        !mFontSubstitutes.Get(name, &fe) &&
        !mFontAliases.Get(name, &fe)) {
        return nsnull;
    }
    return fe.get();
}

cmsHPROFILE
gfxWindowsPlatform::GetPlatformCMSOutputProfile()
{
    WCHAR str[1024+1];
    DWORD size = 1024;

    HDC dc = GetDC(nsnull);
    GetICMProfileW(dc, &size, (LPWSTR)&str);
    ReleaseDC(nsnull, dc);

    cmsHPROFILE profile =
        cmsOpenProfileFromFile(NS_ConvertUTF16toUTF8(str).get(), "r");
#ifdef DEBUG_tor
    if (profile)
        fprintf(stderr,
                "ICM profile read from %s successfully\n",
                NS_ConvertUTF16toUTF8(str).get());
#endif
    return profile;
}

PRBool
gfxWindowsPlatform::GetPrefFontEntries(const char *aLangGroup, nsTArray<nsRefPtr<FontEntry> > *array)
{
    nsCAutoString keyName(aLangGroup);
    return mPrefFonts.Get(keyName, array);
}

void
gfxWindowsPlatform::SetPrefFontEntries(const char *aLangGroup, nsTArray<nsRefPtr<FontEntry> >& array)
{
    nsCAutoString keyName(aLangGroup);
    mPrefFonts.Put(keyName, array);
}
