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
 *   Masayuki Nakano <masayuki@d-toybox.com>
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

#ifndef GFX_FONT_H
#define GFX_FONT_H

#include "prtypes.h"
#include "gfxTypes.h"
#include "nsString.h"
#include "gfxPoint.h"
#include "nsTArray.h"
#include "nsTHashtable.h"
#include "nsHashKeys.h"
#include "gfxSkipChars.h"
#include "gfxRect.h"
#include "nsExpirationTracker.h"
#include "nsMathUtils.h"
#include "nsBidiUtils.h"

#ifdef DEBUG
#include <stdio.h>
#endif

class gfxContext;
class gfxTextRun;
class nsIAtom;
class gfxFont;
class gfxFontGroup;

#define FONT_STYLE_NORMAL              0
#define FONT_STYLE_ITALIC              1
#define FONT_STYLE_OBLIQUE             2

#define FONT_WEIGHT_NORMAL             400
#define FONT_WEIGHT_BOLD               700

#define FONT_MAX_SIZE                  2000.0

struct THEBES_API gfxFontStyle {
    gfxFontStyle(PRUint8 aStyle, PRUint16 aWeight, gfxFloat aSize,
                 const nsACString& aLangGroup,
                 float aSizeAdjust, PRPackedBool aSystemFont,
                 PRPackedBool aFamilyNameQuirks);
    gfxFontStyle(const gfxFontStyle& aStyle);

    // The style of font (normal, italic, oblique)
    PRUint8 style : 7;

    // Say that this font is a system font and therefore does not
    // require certain fixup that we do for fonts from untrusted
    // sources.
    PRPackedBool systemFont : 1;

    // True if the character set quirks (for treatment of "Symbol",
    // "Wingdings", etc.) should be applied.
    PRPackedBool familyNameQuirks : 1;
    
    // The weight of the font.  100, 200, ... 900 are the weights, and
    // single integer offsets request the next bolder/lighter font
    // available.  For example, for a font available in weights 200,
    // 400, 700, and 900, a weight of 898 should lead to the weight 400
    // font being used, since it is two weights lighter than 900.
    PRUint16 weight;

    // The logical size of the font, in pixels
    gfxFloat size;

    // the language group
    nsCString langGroup;

    // The aspect-value (ie., the ratio actualsize:actualxheight) that any
    // actual physical font created from this font structure must have when
    // rendering or measuring a string. A value of 0 means no adjustment
    // needs to be done.
    float sizeAdjust;

    // Return the final adjusted font size for the given aspect ratio.
    // Not meant to be called when sizeAdjust = 0.
    gfxFloat GetAdjustedSize(gfxFloat aspect) const {
        NS_ASSERTION(sizeAdjust != 0.0, "Not meant to be called when sizeAdjust = 0");
        gfxFloat adjustedSize = PR_MAX(NS_round(size*(sizeAdjust/aspect)), 1.0);
        return PR_MIN(adjustedSize, FONT_MAX_SIZE);
    }

    PLDHashNumber Hash() const {
        return ((style + (systemFont << 7) + (familyNameQuirks << 8) +
            (weight << 9)) + PRUint32(size*1000) + PRUint32(sizeAdjust*1000)) ^
            HashString(langGroup);
    }

    void ComputeWeightAndOffset(PRInt8 *outBaseWeight,
                                PRInt8 *outOffset) const;

    PRBool Equals(const gfxFontStyle& other) const {
        return (size == other.size) &&
            (style == other.style) &&
            (systemFont == other.systemFont) &&
            (familyNameQuirks == other.familyNameQuirks) &&
            (weight == other.weight) &&
            (langGroup.Equals(other.langGroup)) &&
            (sizeAdjust == other.sizeAdjust);
    }
};

/**
 * Font cache design:
 * 
 * The mFonts hashtable contains most fonts, indexed by (name, style).
 * It does not add a reference to the fonts it contains.
 * When a font's refcount decreases to zero, instead of deleting it we
 * add it to our expiration tracker.
 * The expiration tracker tracks fonts with zero refcount. After a certain
 * period of time, such fonts expire and are deleted.
 *
 * We're using 3 generations with a ten-second generation interval, so
 * zero-refcount fonts will be deleted 20-30 seconds after their refcount
 * goes to zero, if timer events fire in a timely manner.
 */
class THEBES_API gfxFontCache : public nsExpirationTracker<gfxFont,3> {
public:
    enum { TIMEOUT_SECONDS = 10 };
    gfxFontCache()
        : nsExpirationTracker<gfxFont,3>(TIMEOUT_SECONDS*1000) { mFonts.Init(); }
    ~gfxFontCache() {
        // Expire everything that has a zero refcount, so we don't leak them.
        AgeAllGenerations();
        // All fonts should be gone.
        NS_WARN_IF_FALSE(mFonts.Count() == 0,
                         "Fonts still alive while shutting down gfxFontCache");
        // Note that we have to delete everything through the expiration
        // tracker, since there might be fonts not in the hashtable but in
        // the tracker.
    }

    /*
     * Get the global gfxFontCache.  You must call Init() before
     * calling this method --- the result will not be null.
     */
    static gfxFontCache* GetCache() {
        return gGlobalCache;
    }

    static nsresult Init();
    // It's OK to call this even if Init() has not been called.
    static void Shutdown();

    // Look up a font in the cache. Returns an addrefed pointer, or null
    // if there's nothing matching in the cache
    already_AddRefed<gfxFont> Lookup(const nsAString &aName,
                                     const gfxFontStyle *aFontGroup);
    // We created a new font (presumably because Lookup returned null);
    // put it in the cache. The font's refcount should be nonzero. It is
    // allowable to add a new font even if there is one already in the
    // cache with the same key; we'll forget about the old one.
    void AddNew(gfxFont *aFont);

    // The font's refcount has gone to zero; give ownership of it to
    // the cache. We delete it if it's not acquired again after a certain
    // amount of time.
    void NotifyReleased(gfxFont *aFont);

    // This gets called when the timeout has expired on a zero-refcount
    // font; we just delete it.
    virtual void NotifyExpired(gfxFont *aFont);

protected:
    void DestroyFont(gfxFont *aFont);

    static gfxFontCache *gGlobalCache;

    struct Key {
        const nsAString&    mString;
        const gfxFontStyle* mStyle;
        Key(const nsAString& aString, const gfxFontStyle* aStyle)
            : mString(aString), mStyle(aStyle) {}
    };

    class HashEntry : public PLDHashEntryHdr {
    public:
        typedef const Key& KeyType;
        typedef const Key* KeyTypePointer;

        // When constructing a new entry in the hashtable, we'll leave this
        // blank. The caller of Put() will fill this in.
        HashEntry(KeyTypePointer aStr) : mFont(nsnull) { }
        HashEntry(const HashEntry& toCopy) : mFont(toCopy.mFont) { }
        ~HashEntry() { }

        PRBool KeyEquals(const KeyTypePointer aKey) const;
        static KeyTypePointer KeyToPointer(KeyType aKey) { return &aKey; }
        static PLDHashNumber HashKey(const KeyTypePointer aKey) {
            return HashString(aKey->mString) ^ aKey->mStyle->Hash();
        }
        enum { ALLOW_MEMMOVE = PR_TRUE };

        gfxFont* mFont;
    };

    nsTHashtable<HashEntry> mFonts;
};

/**
 * This stores glyph bounds information for a particular gfxFont, at
 * a particular appunits-per-dev-pixel ratio (because the compressed glyph
 * width array is stored in appunits).
 * 
 * We store a hashtable from glyph IDs to float bounding rects. For the
 * common case where the glyph has no horizontal left bearing, and no
 * y overflow above the font ascent or below the font descent, and tight
 * bounding boxes are not required, we avoid storing the glyph ID in the hashtable
 * and instead consult an array of 16-bit glyph XMost values (in appunits).
 * This array always has an entry for the font's space glyph --- the width is
 * assumed to be zero.
 */
class THEBES_API gfxGlyphExtents {
public:
    gfxGlyphExtents(PRUint32 aAppUnitsPerDevUnit) :
        mAppUnitsPerDevUnit(aAppUnitsPerDevUnit) {
        MOZ_COUNT_CTOR(gfxGlyphExtents);
        mTightGlyphExtents.Init();
    }
    ~gfxGlyphExtents();

    enum { INVALID_WIDTH = 0xFFFF };

    // returns INVALID_WIDTH => not a contained glyph
    // Otherwise the glyph has no before-bearing or vertical bearings,
    // and the result is its width measured from the baseline origin, in
    // appunits.
    PRUint16 GetContainedGlyphWidthAppUnits(PRUint32 aGlyphID) const {
        return mContainedGlyphWidths.Get(aGlyphID);
    }
    
    PRBool IsGlyphKnown(PRUint32 aGlyphID) const {
        return mContainedGlyphWidths.Get(aGlyphID) != INVALID_WIDTH ||
            mTightGlyphExtents.GetEntry(aGlyphID) != nsnull;
    }

    PRBool IsGlyphKnownWithTightExtents(PRUint32 aGlyphID) const {
        return mTightGlyphExtents.GetEntry(aGlyphID) != nsnull;
    }

    // Get glyph extents; a rectangle relative to the left baseline origin
    // Returns true on success. Can fail on OOM or when aContext is null
    // and extents were not (successfully) prefetched.
    PRBool GetTightGlyphExtentsAppUnits(gfxFont *aFont, gfxContext *aContext,
            PRUint32 aGlyphID, gfxRect *aExtents);

    void SetContainedGlyphWidthAppUnits(PRUint32 aGlyphID, PRUint16 aWidth) {
        mContainedGlyphWidths.Set(aGlyphID, aWidth);
    }
    void SetTightGlyphExtents(PRUint32 aGlyphID, const gfxRect& aExtentsAppUnits);

    PRUint32 GetAppUnitsPerDevUnit() { return mAppUnitsPerDevUnit; }

private:
    class HashEntry : public nsUint32HashKey {
    public:
        // When constructing a new entry in the hashtable, we'll leave this
        // blank. The caller of Put() will fill this in.
        HashEntry(KeyTypePointer aPtr) : nsUint32HashKey(aPtr) {}
        HashEntry(const HashEntry& toCopy) : nsUint32HashKey(toCopy) {
          x = toCopy.x; y = toCopy.y; width = toCopy.width; height = toCopy.height;
        }

        float x, y, width, height;
    };

    typedef unsigned long PtrBits;
    enum { BLOCK_SIZE_BITS = 7, BLOCK_SIZE = 1 << BLOCK_SIZE_BITS }; // 128-glyph blocks

    class GlyphWidths {
    public:
        void Set(PRUint32 aIndex, PRUint16 aValue);
        PRUint16 Get(PRUint32 aIndex) const {
            PRUint32 block = aIndex >> BLOCK_SIZE_BITS;
            if (block >= mBlocks.Length())
                return INVALID_WIDTH;
            PtrBits bits = mBlocks[block];
            if (!bits)
                return INVALID_WIDTH;
            PRUint32 indexInBlock = aIndex & (BLOCK_SIZE - 1);
            if (bits & 0x1) {
                if (GetGlyphOffset(bits) != indexInBlock)
                    return INVALID_WIDTH;
                return GetWidth(bits);
            }
            PRUint16 *widths = reinterpret_cast<PRUint16 *>(bits);
            return widths[indexInBlock];
        }

#ifdef DEBUG
        PRUint32 ComputeSize();
#endif
        
        ~GlyphWidths();

    private:
        static PRUint32 GetGlyphOffset(PtrBits aBits) {
            NS_ASSERTION(aBits & 0x1, "This is really a pointer...");
            return (aBits >> 1) & ((1 << BLOCK_SIZE_BITS) - 1);
        }
        static PRUint32 GetWidth(PtrBits aBits) {
            NS_ASSERTION(aBits & 0x1, "This is really a pointer...");
            return aBits >> (1 + BLOCK_SIZE_BITS);
        }
        static PtrBits MakeSingle(PRUint32 aGlyphOffset, PRUint16 aWidth) {
            return (aWidth << (1 + BLOCK_SIZE_BITS)) + (aGlyphOffset << 1) + 1;
        }

        nsTArray<PtrBits> mBlocks;
    };
    
    GlyphWidths             mContainedGlyphWidths;
    nsTHashtable<HashEntry> mTightGlyphExtents;
    PRUint32                mAppUnitsPerDevUnit;
};

/* a SPECIFIC single font family */
class THEBES_API gfxFont {
public:
    nsrefcnt AddRef(void) {
        NS_PRECONDITION(PRInt32(mRefCnt) >= 0, "illegal refcnt");
        if (mExpirationState.IsTracked()) {
            gfxFontCache::GetCache()->RemoveObject(this);
        }
        ++mRefCnt;
        NS_LOG_ADDREF(this, mRefCnt, "gfxFont", sizeof(*this));
        return mRefCnt;
    }
    nsrefcnt Release(void) {
        NS_PRECONDITION(0 != mRefCnt, "dup release");
        --mRefCnt;
        NS_LOG_RELEASE(this, mRefCnt, "gfxFont");
        if (mRefCnt == 0) {
            // Don't delete just yet; return the object to the cache for
            // possibly recycling within some time limit
            gfxFontCache::GetCache()->NotifyReleased(this);
            return 0;
        }
        return mRefCnt;
    }

    PRInt32 GetRefCount() { return mRefCnt; }

protected:
    nsAutoRefCnt mRefCnt;

public:
    gfxFont(const nsAString &aName, const gfxFontStyle *aFontGroup);
    virtual ~gfxFont();

    const nsString& GetName() const { return mName; }
    const gfxFontStyle *GetStyle() const { return &mStyle; }

    virtual nsString GetUniqueName() = 0;

    // Font metrics
    struct Metrics {
        gfxFloat xHeight;
        gfxFloat superscriptOffset;
        gfxFloat subscriptOffset;
        gfxFloat strikeoutSize;
        gfxFloat strikeoutOffset;
        gfxFloat underlineSize;
        gfxFloat underlineOffset;
        gfxFloat height;

        gfxFloat internalLeading;
        gfxFloat externalLeading;

        gfxFloat emHeight;
        gfxFloat emAscent;
        gfxFloat emDescent;
        gfxFloat maxHeight;
        gfxFloat maxAscent;
        gfxFloat maxDescent;
        gfxFloat maxAdvance;

        gfxFloat aveCharWidth;
        gfxFloat spaceWidth;
    };
    virtual const gfxFont::Metrics& GetMetrics() = 0;

    /**
     * We let layout specify spacing on either side of any
     * character. We need to specify both before and after
     * spacing so that substring measurement can do the right things.
     * These values are in appunits. They're always an integral number of
     * appunits, but we specify them in floats in case very large spacing
     * values are required.
     */
    struct Spacing {
        gfxFloat mBefore;
        gfxFloat mAfter;
    };
    /**
     * Metrics for a particular string
     */
    struct RunMetrics {
        RunMetrics() {
            mAdvanceWidth = mAscent = mDescent = 0.0;
            mBoundingBox = gfxRect(0,0,0,0);
        }

        void CombineWith(const RunMetrics& aOtherOnRight) {
            mAscent = PR_MAX(mAscent, aOtherOnRight.mAscent);
            mDescent = PR_MAX(mDescent, aOtherOnRight.mDescent);
            mBoundingBox =
                mBoundingBox.Union(aOtherOnRight.mBoundingBox + gfxPoint(mAdvanceWidth, 0));
            mAdvanceWidth += aOtherOnRight.mAdvanceWidth;
        }

        // can be negative (partly due to negative spacing).
        // Advance widths should be additive: the advance width of the
        // (offset1, length1) plus the advance width of (offset1 + length1,
        // length2) should be the advance width of (offset1, length1 + length2)
        gfxFloat mAdvanceWidth;
        
        // For zero-width substrings, these must be zero!
        gfxFloat mAscent;  // always non-negative
        gfxFloat mDescent; // always non-negative
        
        // Bounding box that is guaranteed to include everything drawn.
        // If aTightBoundingBox was set to true when these metrics were
        // generated, this will tightly wrap the glyphs, otherwise it is
        // "loose" and may be larger than the true bounding box.
        // Coordinates are relative to the baseline left origin, so typically
        // mBoundingBox.y == -mAscent
        gfxRect  mBoundingBox;
    };

    /**
     * Draw a series of glyphs to aContext. The direction of aTextRun must
     * be honoured.
     * @param aStart the first character to draw
     * @param aEnd draw characters up to here
     * @param aBaselineOrigin the baseline origin; the left end of the baseline
     * for LTR textruns, the right end of the baseline for RTL textruns. On return,
     * this should be updated to the other end of the baseline. In application
     * units, really!
     * @param aSpacing spacing to insert before and after characters (for RTL
     * glyphs, before-spacing is inserted to the right of characters). There
     * are aEnd - aStart elements in this array, unless it's null to indicate
     * that there is no spacing.
     * @param aDrawToPath when true, add the glyph outlines to the current path
     * instead of drawing the glyphs
     * 
     * Callers guarantee:
     * -- aStart and aEnd are aligned to cluster and ligature boundaries
     * -- all glyphs use this font
     * 
     * The default implementation builds a cairo glyph array and
     * calls cairo_show_glyphs or cairo_glyph_path.
     */
    virtual void Draw(gfxTextRun *aTextRun, PRUint32 aStart, PRUint32 aEnd,
                      gfxContext *aContext, PRBool aDrawToPath, gfxPoint *aBaselineOrigin,
                      Spacing *aSpacing);
    /**
     * Measure a run of characters. See gfxTextRun::Metrics.
     * @param aTight if false, then return the union of the glyph extents
     * with the font-box for the characters (the rectangle with x=0,width=
     * the advance width for the character run,y=-(font ascent), and height=
     * font ascent + font descent). Otherwise, we must return as tight as possible
     * an approximation to the area actually painted by glyphs.
     * @param aContextForTightBoundingBox when aTight is true, this must
     * be non-null.
     * @param aSpacing spacing to insert before and after glyphs. The bounding box
     * need not include the spacing itself, but the spacing affects the glyph
     * positions. null if there is no spacing.
     * 
     * Callers guarantee:
     * -- aStart and aEnd are aligned to cluster and ligature boundaries
     * -- all glyphs use this font
     * 
     * The default implementation just uses font metrics and aTextRun's
     * advances, and assumes no characters fall outside the font box. In
     * general this is insufficient, because that assumption is not always true.
     */
    virtual RunMetrics Measure(gfxTextRun *aTextRun,
                               PRUint32 aStart, PRUint32 aEnd,
                               PRBool aTightBoundingBox,
                               gfxContext *aContextForTightBoundingBox,
                               Spacing *aSpacing);
    /**
     * Line breaks have been changed at the beginning and/or end of a substring
     * of the text. Reshaping may be required; glyph updating is permitted.
     * @return true if anything was changed, false otherwise
     */
    PRBool NotifyLineBreaksChanged(gfxTextRun *aTextRun,
                                   PRUint32 aStart, PRUint32 aLength)
    { return PR_FALSE; }

    // Expiration tracking
    nsExpirationState *GetExpirationState() { return &mExpirationState; }

    // Get the glyphID of a space
    virtual PRUint32 GetSpaceGlyph() = 0;

    gfxGlyphExtents *GetOrCreateGlyphExtents(PRUint32 aAppUnitsPerDevUnit);

    // You need to call SetupCairoFont on the aCR just before calling this
    virtual void SetupGlyphExtents(gfxContext *aContext, PRUint32 aGlyphID,
                                   PRBool aNeedTight, gfxGlyphExtents *aExtents);

    // This is called by the default Draw() implementation above.
    virtual PRBool SetupCairoFont(gfxContext *aContext) = 0;

protected:
    // The family name of the font
    nsString                   mName;
    nsExpirationState          mExpirationState;
    gfxFontStyle               mStyle;
    nsAutoTArray<gfxGlyphExtents*,1> mGlyphExtentsArray;

    // some fonts have bad metrics, this method sanitize them.
    void SanitizeMetrics(gfxFont::Metrics *aMetrics);
};

class THEBES_API gfxTextRunFactory {
    THEBES_INLINE_DECL_REFCOUNTING(gfxTextRunFactory)

public:
    // Flags in the mask 0xFFFF0000 are reserved for textrun clients
    // Flags in the mask 0x0000F000 are reserved for per-platform fonts
    // Flags in the mask 0x00000FFF are set by the textrun creator.
    enum {
        CACHE_TEXT_FLAGS    = 0xF0000000,
        USER_TEXT_FLAGS     = 0x0FFF0000,
        PLATFORM_TEXT_FLAGS = 0x0000F000,
        TEXTRUN_TEXT_FLAGS  = 0x00000FFF,
        SETTABLE_FLAGS      = CACHE_TEXT_FLAGS | USER_TEXT_FLAGS,
      
        /**
         * When set, the text string pointer used to create the text run
         * is guaranteed to be available during the lifetime of the text run.
         */
        TEXT_IS_PERSISTENT           = 0x0001,
        /**
         * When set, the text is known to be all-ASCII (< 128).
         */
        TEXT_IS_ASCII                = 0x0002,
        /**
         * When set, the text is RTL.
         */
        TEXT_IS_RTL                  = 0x0004,
        /**
         * When set, spacing is enabled and the textrun needs to call GetSpacing
         * on the spacing provider.
         */
        TEXT_ENABLE_SPACING          = 0x0008,
        /**
         * When set, GetSpacing can return negative spacing.
         */
        TEXT_ENABLE_NEGATIVE_SPACING = 0x0010,
        /**
         * When set, GetHyphenationBreaks may return true for some character
         * positions, otherwise it will always return false for all characters.
         */
        TEXT_ENABLE_HYPHEN_BREAKS    = 0x0040,
        /**
         * When set, the text has no characters above 255 and it is stored
         * in the textrun in 8-bit format.
         */
        TEXT_IS_8BIT                 = 0x0080,
        /**
         * When set, the text may have UTF16 surrogate pairs, otherwise it
         * doesn't.
         */
        TEXT_HAS_SURROGATES          = 0x0100,
        /**
         * When set, the RunMetrics::mBoundingBox field will be initialized
         * properly based on glyph extents, in particular, glyph extents that
         * overflow the standard font-box (the box defined by the ascent, descent
         * and advance width of the glyph). When not set, it may just be the
         * standard font-box even if glyphs overflow.
         */
        TEXT_NEED_BOUNDING_BOX       = 0x0200,
        /**
         * When set, optional ligatures are disabled. Ligatures that are
         * required for legible text should still be enabled.
         */
        TEXT_DISABLE_OPTIONAL_LIGATURES = 0x0400,
        /**
         * When set, the textrun should favour speed of construction over
         * quality. This may involve disabling ligatures and/or kerning or
         * other effects.
         */
        TEXT_OPTIMIZE_SPEED          = 0x0800
    };

    /**
     * This record contains all the parameters needed to initialize a textrun.
     */
    struct Parameters {
        // A reference context suggesting where the textrun will be rendered
        gfxContext   *mContext;
        // Pointer to arbitrary user data (which should outlive the textrun)
        void         *mUserData;
        // A description of which characters have been stripped from the original
        // DOM string to produce the characters in the textrun. May be null
        // if that information is not relevant.
        gfxSkipChars *mSkipChars;
        // A list of where linebreaks are currently placed in the textrun. May
        // be null if mInitialBreakCount is zero.
        PRUint32     *mInitialBreaks;
        PRUint32      mInitialBreakCount;
        // The ratio to use to convert device pixels to application layout units
        PRUint32      mAppUnitsPerDevUnit;
    };

    virtual ~gfxTextRunFactory() {}
};

/**
 * gfxTextRun is an abstraction for drawing and measuring substrings of a run
 * of text. It stores runs of positioned glyph data, each run having a single
 * gfxFont. The glyphs are associated with a string of source text, and the
 * gfxTextRun APIs take parameters that are offsets into that source text.
 * 
 * gfxTextRuns are not refcounted. They should be deleted when no longer required.
 * 
 * gfxTextRuns are mostly immutable. The only things that can change are
 * inter-cluster spacing and line break placement. Spacing is always obtained
 * lazily by methods that need it, it is not cached. Line breaks are stored
 * persistently (insofar as they affect the shaping of glyphs; gfxTextRun does
 * not actually do anything to explicitly account for line breaks). Initially
 * there are no line breaks. The textrun can record line breaks before or after
 * any given cluster. (Line breaks specified inside clusters are ignored.)
 * 
 * It is important that zero-length substrings are handled correctly. This will
 * be on the test!
 * 
 * gfxTextRun stores a list of zero or more glyphs for each character. For each
 * glyph we store the glyph ID, the advance, and possibly an xoffset and yoffset.
 * The idea is that a string is rendered by a loop that draws each glyph
 * at its designated offset from the current point, then advances the current
 * point by the glyph's advance in the direction of the textrun (LTR or RTL).
 * Each glyph advance is always rounded to the nearest appunit; this ensures
 * consistent results when dividing the text in a textrun into multiple text
 * frames (frame boundaries are always aligned to appunits). We optimize
 * for the case where a character has a single glyph and zero xoffset and yoffset,
 * and the glyph ID and advance are in a reasonable range so we can pack all
 * necessary data into 32 bits.
 * 
 * gfxTextRun methods that measure or draw substrings will associate all the
 * glyphs in a cluster with the first character of the cluster; if that character
 * is in the substring, the glyphs will be measured or drawn, otherwise they
 * won't.
 */
class THEBES_API gfxTextRun {
public:
    // Override operator delete because we used custom allocation
    void operator delete(void* aPtr);
    virtual ~gfxTextRun();

    typedef gfxFont::RunMetrics Metrics;

    // Public textrun API for general use

    PRBool IsClusterStart(PRUint32 aPos) {
        NS_ASSERTION(0 <= aPos && aPos < mCharacterCount, "aPos out of range");
        return mCharacterGlyphs[aPos].IsClusterStart();
    }
    PRBool IsLigatureGroupStart(PRUint32 aPos) {
        NS_ASSERTION(0 <= aPos && aPos < mCharacterCount, "aPos out of range");
        return mCharacterGlyphs[aPos].IsLigatureGroupStart();
    }
    PRBool CanBreakLineBefore(PRUint32 aPos) {
        NS_ASSERTION(0 <= aPos && aPos < mCharacterCount, "aPos out of range");
        return mCharacterGlyphs[aPos].CanBreakBefore();
    }    

    PRUint32 GetLength() { return mCharacterCount; }

    // All PRUint32 aStart, PRUint32 aLength ranges below are restricted to
    // grapheme cluster boundaries! All offsets are in terms of the string
    // passed into MakeTextRun.
    
    // All coordinates are in layout/app units

    /**
     * Set the potential linebreaks for a substring of the textrun. These are
     * the "allow break before" points. Initially, there are no potential
     * linebreaks.
     * 
     * This can change glyphs and/or geometry! Some textruns' shapes
     * depend on potential line breaks (e.g., title-case-converting textruns).
     * This function is virtual so that those textruns can reshape themselves.
     * 
     * @return true if this changed the linebreaks, false if the new line
     * breaks are the same as the old
     */
    virtual PRBool SetPotentialLineBreaks(PRUint32 aStart, PRUint32 aLength,
                                          PRPackedBool *aBreakBefore,
                                          gfxContext *aRefContext);

    /**
     * Layout provides PropertyProvider objects. These allow detection of
     * potential line break points and computation of spacing. We pass the data
     * this way to allow lazy data acquisition; for example BreakAndMeasureText
     * will want to only ask for properties of text it's actually looking at.
     * 
     * NOTE that requested spacing may not actually be applied, if the textrun
     * is unable to apply it in some context. Exception: spacing around a
     * whitespace character MUST always be applied.
     */
    class PropertyProvider {
    public:
        // Detect hyphenation break opportunities in the given range; breaks
        // not at cluster boundaries will be ignored.
        virtual void GetHyphenationBreaks(PRUint32 aStart, PRUint32 aLength,
                                          PRPackedBool *aBreakBefore) = 0;

        // Returns the extra width that will be consumed by a hyphen. This should
        // be constant for a given textrun.
        virtual gfxFloat GetHyphenWidth() = 0;

        typedef gfxFont::Spacing Spacing;

        /**
         * Get the spacing around the indicated characters. Spacing must be zero
         * inside clusters. In other words, if character i is not
         * CLUSTER_START, then character i-1 must have zero after-spacing and
         * character i must have zero before-spacing.
         */
        virtual void GetSpacing(PRUint32 aStart, PRUint32 aLength,
                                Spacing *aSpacing) = 0;
    };

    /**
     * Draws a substring. Uses only GetSpacing from aBreakProvider.
     * The provided point is the baseline origin on the left of the string
     * for LTR, on the right of the string for RTL.
     * @param aDirtyRect if non-null, drawing outside of the rectangle can be
     * (but does not need to be) dropped. Note that if this is null, we cannot
     * draw partial ligatures and we will assert if partial ligatures
     * are detected.
     * @param aAdvanceWidth if non-null, the advance width of the substring
     * is returned here.
     * 
     * Drawing should respect advance widths in the sense that for LTR runs,
     * Draw(ctx, pt, offset1, length1, dirty, &provider, &advance) followed by
     * Draw(ctx, gfxPoint(pt.x + advance, pt.y), offset1 + length1, length2,
     *      dirty, &provider, nsnull) should have the same effect as
     * Draw(ctx, pt, offset1, length1+length2, dirty, &provider, nsnull).
     * For RTL runs the rule is:
     * Draw(ctx, pt, offset1 + length1, length2, dirty, &provider, &advance) followed by
     * Draw(ctx, gfxPoint(pt.x + advance, pt.y), offset1, length1,
     *      dirty, &provider, nsnull) should have the same effect as
     * Draw(ctx, pt, offset1, length1+length2, dirty, &provider, nsnull).
     * 
     * Glyphs should be drawn in logical content order, which can be significant
     * if they overlap (perhaps due to negative spacing).
     */
    void Draw(gfxContext *aContext, gfxPoint aPt,
              PRUint32 aStart, PRUint32 aLength,
              const gfxRect *aDirtyRect,
              PropertyProvider *aProvider,
              gfxFloat *aAdvanceWidth);

    /**
     * Renders a substring to a path. Uses only GetSpacing from aBreakProvider.
     * The provided point is the baseline origin on the left of the string
     * for LTR, on the right of the string for RTL.
     * @param aAdvanceWidth if non-null, the advance width of the substring
     * is returned here.
     * 
     * Drawing should respect advance widths in the way that Draw above does.
     * 
     * Glyphs should be drawn in logical content order.
     * 
     * UNLIKE Draw above, this cannot be used to render substrings that start or
     * end inside a ligature.
     */
    void DrawToPath(gfxContext *aContext, gfxPoint aPt,
                    PRUint32 aStart, PRUint32 aLength,
                    PropertyProvider *aBreakProvider,
                    gfxFloat *aAdvanceWidth);

    /**
     * Computes the ReflowMetrics for a substring.
     * Uses GetSpacing from aBreakProvider.
     * @param aTightBoundingBox if true, we make the bounding box tight
     */
    Metrics MeasureText(PRUint32 aStart, PRUint32 aLength,
                        PRBool aTightBoundingBox,
                        gfxContext *aRefContextForTightBoundingBox,
                        PropertyProvider *aProvider);

    /**
     * Computes just the advance width for a substring.
     * Uses GetSpacing from aBreakProvider.
     */
    gfxFloat GetAdvanceWidth(PRUint32 aStart, PRUint32 aLength,
                             PropertyProvider *aProvider);

    /**
     * Clear all stored line breaks for the given range (both before and after),
     * and then set the line-break state before aStart to aBreakBefore and
     * after the last cluster to aBreakAfter.
     * 
     * We require that before and after line breaks be consistent. For clusters
     * i and i+1, we require that if there is a break after cluster i, a break
     * will be specified before cluster i+1. This may be temporarily violated
     * (e.g. after reflowing line L and before reflowing line L+1); to handle
     * these temporary violations, we say that there is a break betwen i and i+1
     * if a break is specified after i OR a break is specified before i+1.
     * 
     * This can change textrun geometry! The existence of a linebreak can affect
     * the advance width of the cluster before the break (when kerning) or the
     * geometry of one cluster before the break or any number of clusters
     * after the break. (The one-cluster-before-the-break limit is somewhat
     * arbitrary; if some scripts require breaking it, then we need to
     * alter nsTextFrame::TrimTrailingWhitespace, perhaps drastically becase
     * it could affect the layout of frames before it...)
     * 
     * We return true if glyphs or geometry changed, false otherwise. This
     * function is virtual so that gfxTextRun subclasses can reshape
     * properly.
     * 
     * @param aAdvanceWidthDelta if non-null, returns the change in advance
     * width of the given range.
     */
    virtual PRBool SetLineBreaks(PRUint32 aStart, PRUint32 aLength,
                                 PRBool aLineBreakBefore, PRBool aLineBreakAfter,
                                 gfxFloat *aAdvanceWidthDelta,
                                 gfxContext *aRefContext);

    /**
     * Finds the longest substring that will fit into the given width.
     * Uses GetHyphenationBreaks and GetSpacing from aBreakProvider.
     * Guarantees the following:
     * -- 0 <= result <= aMaxLength
     * -- result is the maximal value of N such that either
     *       N < aMaxLength && line break at N && GetAdvanceWidth(aStart, N) <= aWidth
     *   OR  N < aMaxLength && hyphen break at N && GetAdvanceWidth(aStart, N) + GetHyphenWidth() <= aWidth
     *   OR  N == aMaxLength && GetAdvanceWidth(aStart, N) <= aWidth
     * where GetAdvanceWidth assumes the effect of
     * SetLineBreaks(aStart, N, aLineBreakBefore, N < aMaxLength, aProvider)
     * -- if no such N exists, then result is the smallest N such that
     *       N < aMaxLength && line break at N
     *   OR  N < aMaxLength && hyphen break at N
     *   OR  N == aMaxLength
     *
     * The call has the effect of
     * SetLineBreaks(aStart, result, aLineBreakBefore, result < aMaxLength, aProvider)
     * and the returned metrics and the invariants above reflect this.
     *
     * @param aMaxLength this can be PR_UINT32_MAX, in which case the length used
     * is up to the end of the string
     * @param aLineBreakBefore set to true if and only if there is an actual
     * line break at the start of this string.
     * @param aSuppressInitialBreak if true, then we assume there is no possible
     * linebreak before aStart. If false, then we will check the internal
     * line break opportunity state before deciding whether to return 0 as the
     * character to break before.
     * @param aTrimWhitespace if non-null, then we allow a trailing run of
     * spaces to be trimmed; the width of the space(s) will not be included in
     * the measured string width for comparison with the limit aWidth, and
     * trimmed spaces will not be included in returned metrics. The width
     * of the trimmed spaces will be returned in aTrimWhitespace.
     * Trimmed spaces are still counted in the "characters fit" result.
     * @param aMetrics if non-null, we fill this in for the returned substring.
     * If a hyphenation break was used, the hyphen is NOT included in the returned metrics.
     * @param aTightBoundingBox if true, we make the bounding box in aMetrics tight
     * @param aRefContextForTightBoundingBox a reference context to get the
     * tight bounding box, if aTightBoundingBox is true
     * @param aUsedHyphenation if non-null, records if we selected a hyphenation break
     * @param aLastBreak if non-null and result is aMaxLength, we set this to
     * the maximal N such that
     *       N < aMaxLength && line break at N && GetAdvanceWidth(aStart, N) <= aWidth
     *   OR  N < aMaxLength && hyphen break at N && GetAdvanceWidth(aStart, N) + GetHyphenWidth() <= aWidth
     * or PR_UINT32_MAX if no such N exists, where GetAdvanceWidth assumes
     * the effect of
     * SetLineBreaks(aStart, N, aLineBreakBefore, N < aMaxLength, aProvider)
     * 
     * Note that negative advance widths are possible especially if negative
     * spacing is provided.
     */
    PRUint32 BreakAndMeasureText(PRUint32 aStart, PRUint32 aMaxLength,
                                 PRBool aLineBreakBefore, gfxFloat aWidth,
                                 PropertyProvider *aProvider,
                                 PRBool aSuppressInitialBreak,
                                 gfxFloat *aTrimWhitespace,
                                 Metrics *aMetrics, PRBool aTightBoundingBox,
                                 gfxContext *aRefContextForTightBoundingBox,
                                 PRBool *aUsedHyphenation,
                                 PRUint32 *aLastBreak);

    /**
     * Update the reference context.
     * XXX this is a hack. New text frame does not call this. Use only
     * temporarily for old text frame.
     */
    void SetContext(gfxContext *aContext) {}

    // Utility getters

    PRBool IsRightToLeft() const { return (mFlags & gfxTextRunFactory::TEXT_IS_RTL) != 0; }
    gfxFloat GetDirection() const { return (mFlags & gfxTextRunFactory::TEXT_IS_RTL) ? -1.0 : 1.0; }
    void *GetUserData() const { return mUserData; }
    void SetUserData(void *aUserData) { mUserData = aUserData; }
    PRUint32 GetFlags() const { return mFlags; }
    void SetFlagBits(PRUint32 aFlags) {
      NS_ASSERTION(!(aFlags & ~gfxTextRunFactory::SETTABLE_FLAGS),
                   "Only user flags should be mutable");
      mFlags |= aFlags;
    }
    void ClearFlagBits(PRUint32 aFlags) {
      NS_ASSERTION(!(aFlags & ~gfxTextRunFactory::SETTABLE_FLAGS),
                   "Only user flags should be mutable");
      mFlags &= ~aFlags;
    }
    const gfxSkipChars& GetSkipChars() const { return mSkipChars; }
    PRUint32 GetAppUnitsPerDevUnit() const { return mAppUnitsPerDevUnit; }
    gfxFontGroup *GetFontGroup() const { return mFontGroup; }
    const PRUint8 *GetText8Bit() const
    { return (mFlags & gfxTextRunFactory::TEXT_IS_8BIT) ? mText.mSingle : nsnull; }
    const PRUnichar *GetTextUnicode() const
    { return (mFlags & gfxTextRunFactory::TEXT_IS_8BIT) ? nsnull : mText.mDouble; }
    const void *GetTextAt(PRUint32 aIndex) {
        return (mFlags & gfxTextRunFactory::TEXT_IS_8BIT)
            ? static_cast<const void *>(mText.mSingle + aIndex)
            : static_cast<const void *>(mText.mDouble + aIndex);
    }
    const PRUnichar GetChar(PRUint32 i) const
    { return (mFlags & gfxTextRunFactory::TEXT_IS_8BIT) ? mText.mSingle[i] : mText.mDouble[i]; }
    PRUint32 GetHashCode() const { return mHashCode; }
    void SetHashCode(PRUint32 aHash) { mHashCode = aHash; }

    // Call this, don't call "new gfxTextRun" directly. This does custom
    // allocation and initialization
    static gfxTextRun *Create(const gfxTextRunFactory::Parameters *aParams,
        const void *aText, PRUint32 aLength, gfxFontGroup *aFontGroup, PRUint32 aFlags);

    // Clone this textrun, according to the given parameters. This textrun's
    // glyph data is copied, so the text and length must be the same as this
    // textrun's. If there's a problem, return null. Actual linebreaks will
    // be set as per aParams; there will be no potential linebreaks.
    // If aText is not persistent (aFlags & TEXT_IS_PERSISTENT), the
    // textrun will copy it.
    virtual gfxTextRun *Clone(const gfxTextRunFactory::Parameters *aParams, const void *aText,
                              PRUint32 aLength, gfxFontGroup *aFontGroup, PRUint32 aFlags);

    /**
     * This class records the information associated with a character in the
     * input string. It's optimized for the case where there is one glyph
     * representing that character alone.
     * 
     * A character can have zero or more associated glyphs. Each glyph
     * has an advance width and an x and y offset.
     * A character may be the start of a cluster.
     * A character may be the start of a ligature group.
     * A character can be "missing", indicating that the system is unable
     * to render the character.
     * 
     * All characters in a ligature group conceptually share all the glyphs
     * associated with the characters in a group.
     */
    class CompressedGlyph {
    public:
        CompressedGlyph() { mValue = 0; }

        enum {
            // Indicates that a cluster and ligature group starts at this
            // character; this character has a single glyph with a reasonable
            // advance and zero offsets. A "reasonable" advance
            // is one that fits in the available bits (currently 14) (specified
            // in appunits).
            FLAG_IS_SIMPLE_GLYPH  = 0x80000000U,
            // Indicates that a linebreak is allowed before this character
            FLAG_CAN_BREAK_BEFORE = 0x40000000U,

            // The advance is stored in appunits
            ADVANCE_MASK  = 0x3FFF0000U,
            ADVANCE_SHIFT = 16,

            GLYPH_MASK = 0x0000FFFFU,

            // Non-simple glyphs may or may not have glyph data in the
            // corresponding mDetailedGlyphs entry. They have the following
            // flag bits:

            // When NOT set, indicates that this character corresponds to a
            // missing glyph and should be skipped (or possibly, render the character
            // Unicode value in some special way). If there are glyphs,
            // the mGlyphID is actually the UTF16 character code. The bit is
            // inverted so we can memset the array to zero to indicate all missing.
            FLAG_NOT_MISSING              = 0x01,
            FLAG_NOT_CLUSTER_START        = 0x02,
            FLAG_NOT_LIGATURE_GROUP_START = 0x04,
            FLAG_LOW_SURROGATE            = 0x08,
            
            GLYPH_COUNT_MASK = 0x00FFFF00U,
            GLYPH_COUNT_SHIFT = 8
        };

        // "Simple glyphs" have a simple glyph ID, simple advance and their
        // x and y offsets are zero. Also the glyph extents do not overflow
        // the font-box defined by the font ascent, descent and glyph advance width.
        // These case is optimized to avoid storing DetailedGlyphs.

        // Returns true if the glyph ID aGlyph fits into the compressed representation
        static PRBool IsSimpleGlyphID(PRUint32 aGlyph) {
            return (aGlyph & GLYPH_MASK) == aGlyph;
        }
        // Returns true if the advance aAdvance fits into the compressed representation.
        // aAdvance is in appunits.
        static PRBool IsSimpleAdvance(PRUint32 aAdvance) {
            return (aAdvance & (ADVANCE_MASK >> ADVANCE_SHIFT)) == aAdvance;
        }

        PRBool IsSimpleGlyph() const { return (mValue & FLAG_IS_SIMPLE_GLYPH) != 0; }
        PRUint32 GetSimpleAdvance() const { return (mValue & ADVANCE_MASK) >> ADVANCE_SHIFT; }
        PRUint32 GetSimpleGlyph() const { return mValue & GLYPH_MASK; }

        PRBool IsMissing() const { return (mValue & (FLAG_NOT_MISSING|FLAG_IS_SIMPLE_GLYPH)) == 0; }
        PRBool IsLowSurrogate() const {
            return (mValue & (FLAG_LOW_SURROGATE|FLAG_IS_SIMPLE_GLYPH)) == FLAG_LOW_SURROGATE;
        }
        PRBool IsClusterStart() const {
            return (mValue & FLAG_IS_SIMPLE_GLYPH) || !(mValue & FLAG_NOT_CLUSTER_START);
        }
        PRBool IsLigatureGroupStart() const {
            return (mValue & FLAG_IS_SIMPLE_GLYPH) || !(mValue & FLAG_NOT_LIGATURE_GROUP_START);
        }

        PRBool CanBreakBefore() const { return (mValue & FLAG_CAN_BREAK_BEFORE) != 0; }
        // Returns FLAG_CAN_BREAK_BEFORE if the setting changed, 0 otherwise
        PRUint32 SetCanBreakBefore(PRBool aCanBreakBefore) {
            NS_ASSERTION(aCanBreakBefore == PR_FALSE || aCanBreakBefore == PR_TRUE,
                         "Bogus break-before value!");
            PRUint32 breakMask = aCanBreakBefore*FLAG_CAN_BREAK_BEFORE;
            PRUint32 toggle = breakMask ^ (mValue & FLAG_CAN_BREAK_BEFORE);
            mValue ^= toggle;
            return toggle;
        }

        CompressedGlyph& SetSimpleGlyph(PRUint32 aAdvanceAppUnits, PRUint32 aGlyph) {
            NS_ASSERTION(IsSimpleAdvance(aAdvanceAppUnits), "Advance overflow");
            NS_ASSERTION(IsSimpleGlyphID(aGlyph), "Glyph overflow");
            mValue = (mValue & FLAG_CAN_BREAK_BEFORE) | FLAG_IS_SIMPLE_GLYPH |
                (aAdvanceAppUnits << ADVANCE_SHIFT) | aGlyph;
            return *this;
        }
        CompressedGlyph& SetComplex(PRBool aClusterStart, PRBool aLigatureStart,
                PRUint32 aGlyphCount) {
            mValue = (mValue & FLAG_CAN_BREAK_BEFORE) | FLAG_NOT_MISSING |
                (aClusterStart ? 0 : FLAG_NOT_CLUSTER_START) |
                (aLigatureStart ? 0 : FLAG_NOT_LIGATURE_GROUP_START) |
                (aGlyphCount << GLYPH_COUNT_SHIFT);
            return *this;
        }
        /**
         * Missing glyphs are treated as cluster and ligature group starts.
         */
        CompressedGlyph& SetMissing(PRUint32 aGlyphCount) {
            mValue = (mValue & FLAG_CAN_BREAK_BEFORE) |
                (aGlyphCount << GLYPH_COUNT_SHIFT);
            return *this;
        }
        /**
         * Low surrogates don't have any glyphs and are not the start of
         * a cluster or ligature group.
         */
        CompressedGlyph& SetLowSurrogate() {
            mValue = (mValue & FLAG_CAN_BREAK_BEFORE) | FLAG_NOT_MISSING |
                FLAG_LOW_SURROGATE;
            return *this;
        }
        PRUint32 GetGlyphCount() const {
            NS_ASSERTION(!IsSimpleGlyph(), "Expected non-simple-glyph");
            return (mValue & GLYPH_COUNT_MASK) >> GLYPH_COUNT_SHIFT;
        }

    private:
        PRUint32 mValue;
    };

    /**
     * When the glyphs for a character don't fit into a CompressedGlyph record
     * in SimpleGlyph format, we use an array of DetailedGlyphs instead.
     */
    struct DetailedGlyph {
        /** The glyphID, or the Unicode character
         * if this is a missing glyph */
        PRUint32 mGlyphID;
        /** The advance, x-offset and y-offset of the glyph, in appunits
         *  mAdvance is in the text direction (RTL or LTR)
         *  mXOffset is always from left to right
         *  mYOffset is always from bottom to top */   
        PRInt32  mAdvance;
        float    mXOffset, mYOffset;
    };

    // The text is divided into GlyphRuns as necessary
    struct GlyphRun {
        nsRefPtr<gfxFont> mFont;   // never null
        PRUint32          mCharacterOffset; // into original UTF16 string
    };

    class THEBES_API GlyphRunIterator {
    public:
        GlyphRunIterator(gfxTextRun *aTextRun, PRUint32 aStart, PRUint32 aLength)
          : mTextRun(aTextRun), mStartOffset(aStart), mEndOffset(aStart + aLength) {
            mNextIndex = mTextRun->FindFirstGlyphRunContaining(aStart);
        }
        PRBool NextRun();
        GlyphRun *GetGlyphRun() { return mGlyphRun; }
        PRUint32 GetStringStart() { return mStringStart; }
        PRUint32 GetStringEnd() { return mStringEnd; }
    private:
        gfxTextRun *mTextRun;
        GlyphRun   *mGlyphRun;
        PRUint32    mStringStart;
        PRUint32    mStringEnd;
        PRUint32    mNextIndex;
        PRUint32    mStartOffset;
        PRUint32    mEndOffset;
    };

    class GlyphRunOffsetComparator {
    public:
        PRBool Equals(const GlyphRun& a,
                      const GlyphRun& b) const
        {
            return a.mCharacterOffset == b.mCharacterOffset;
        }

        PRBool LessThan(const GlyphRun& a,
                        const GlyphRun& b) const
        {
            return a.mCharacterOffset < b.mCharacterOffset;
        }
    };

    friend class GlyphRunIterator;
    friend class FontSelector;

    // API for setting up the textrun glyphs. Should only be called by
    // things that construct textruns.
    /**
     * Record every character that is the second half of a surrogate pair.
     * This should be called after creating a Unicode textrun.
     */
    void RecordSurrogates(const PRUnichar *aString);
    /**
     * We've found a run of text that should use a particular font. Call this
     * only during initialization when font substitution has been computed.
     * Call it before setting up the glyphs for the characters in this run;
     * SetMissingGlyph requires that the correct glyphrun be installed.
     *
     * If aForceNewRun, a new glyph run will be added, even if the
     * previously added run uses the same font.  If glyph runs are
     * added out of strictly increasing aStartCharIndex order (via
     * force), then SortGlyphRuns must be called after all glyph runs
     * are added before any further operations are performed with this
     * TextRun.
     */
    nsresult AddGlyphRun(gfxFont *aFont, PRUint32 aStartCharIndex, PRBool aForceNewRun = PR_FALSE);
    void ResetGlyphRuns() { mGlyphRuns.Clear(); }
    void SortGlyphRuns();

    // Call the following glyph-setters during initialization or during reshaping
    // only. It is OK to overwrite existing data for a character.
    /**
     * Set the glyph data for a character. aGlyphs may be null if aGlyph is a
     * simple glyph or has no associated glyphs. If non-null the data is copied,
     * the caller retains ownership.
     */
    void SetSimpleGlyph(PRUint32 aCharIndex, CompressedGlyph aGlyph) {
        NS_ASSERTION(aGlyph.IsSimpleGlyph(), "Should be a simple glyph here");
        if (mCharacterGlyphs) {
            mCharacterGlyphs[aCharIndex] = aGlyph;
        }
        if (mDetailedGlyphs) {
            mDetailedGlyphs[aCharIndex] = nsnull;
        }
    }
    void SetGlyphs(PRUint32 aCharIndex, CompressedGlyph aGlyph,
                   const DetailedGlyph *aGlyphs);
    void SetMissingGlyph(PRUint32 aCharIndex, PRUint32 aUnicodeChar);
    void SetSpaceGlyph(gfxFont *aFont, gfxContext *aContext, PRUint32 aCharIndex);
    
    /**
     * Prefetch all the glyph extents needed to ensure that Measure calls
     * on this textrun with aTightBoundingBox false will succeed. Note
     * that some glyph extents might not be fetched due to OOM or other
     * errors.
     */
    void FetchGlyphExtents(gfxContext *aRefContext);

    // API for access to the raw glyph data, needed by gfxFont::Draw
    // and gfxFont::GetBoundingBox
    const CompressedGlyph *GetCharacterGlyphs() { return mCharacterGlyphs; }
    const DetailedGlyph *GetDetailedGlyphs(PRUint32 aCharIndex) {
        return mDetailedGlyphs ? mDetailedGlyphs[aCharIndex].get() : nsnull;
    }
    PRBool HasDetailedGlyphs() { return mDetailedGlyphs.get() != nsnull; }
    PRUint32 CountMissingGlyphs();
    const GlyphRun *GetGlyphRuns(PRUint32 *aNumGlyphRuns) {
        *aNumGlyphRuns = mGlyphRuns.Length();
        return mGlyphRuns.Elements();
    }
    // Returns the index of the GlyphRun containing the given offset.
    // Returns mGlyphRuns.Length() when aOffset is mCharacterCount.
    PRUint32 FindFirstGlyphRunContaining(PRUint32 aOffset);
    // Copy glyph data for a range of characters from aSource to this
    // textrun. If aStealData is true then we actually steal the glyph data,
    // setting the data in aSource to "missing". aDest should be in the last
    // glyphrun.
    virtual void CopyGlyphDataFrom(gfxTextRun *aSource, PRUint32 aStart,
                                   PRUint32 aLength, PRUint32 aDest,
                                   PRBool aStealData);

    nsExpirationState *GetExpirationState() { return &mExpirationState; }

    struct LigatureData {
        // textrun offsets of the start and end of the containing ligature
        PRUint32 mLigatureStart;
        PRUint32 mLigatureEnd;
        // appunits advance to the start of the ligature part within the ligature;
        // never includes any spacing
        gfxFloat mPartAdvance;
        // appunits width of the ligature part; includes before-spacing
        // when the part is at the start of the ligature, and after-spacing
        // when the part is as the end of the ligature
        gfxFloat mPartWidth;
        
        PRPackedBool mPartIsStartOfLigature;
        PRPackedBool mPartIsEndOfLigature;
    };

#ifdef DEBUG
    // number of entries referencing this textrun in the gfxTextRunWordCache
    PRUint32 mCachedWords;
    
    void Dump(FILE* aOutput);
#endif

protected:
    // Allocates extra space for the CompressedGlyph array and the text
    // (if needed)
    void *operator new(size_t aSize, PRUint32 aLength, PRUint32 aFlags);

    /**
     * Initializes the textrun to blank.
     * @param aObjectSize the size of the object; this lets us fine
     * where our CompressedGlyph array and string have been allocated
     */
    gfxTextRun(const gfxTextRunFactory::Parameters *aParams, const void *aText,
               PRUint32 aLength, gfxFontGroup *aFontGroup, PRUint32 aFlags,
               PRUint32 aObjectSize);

private:
    // **** general helpers **** 

    // Allocate aCount DetailedGlyphs for the given index
    DetailedGlyph *AllocateDetailedGlyphs(PRUint32 aCharIndex, PRUint32 aCount);

    // Spacing for characters outside the range aSpacingStart/aSpacingEnd
    // is assumed to be zero; such characters are not passed to aProvider.
    // This is useful to protect aProvider from being passed character indices
    // it is not currently able to handle.
    PRBool GetAdjustedSpacingArray(PRUint32 aStart, PRUint32 aEnd,
                                   PropertyProvider *aProvider,
                                   PRUint32 aSpacingStart, PRUint32 aSpacingEnd,
                                   nsTArray<PropertyProvider::Spacing> *aSpacing);

    //  **** ligature helpers ****
    // (Platforms do the actual ligaturization, but we need to do a bunch of stuff
    // to handle requests that begin or end inside a ligature)

    // if aProvider is null then mBeforeSpacing and mAfterSpacing are set to zero
    LigatureData ComputeLigatureData(PRUint32 aPartStart, PRUint32 aPartEnd,
                                     PropertyProvider *aProvider);
    gfxFloat ComputePartialLigatureWidth(PRUint32 aPartStart, PRUint32 aPartEnd,
                                         PropertyProvider *aProvider);
    void DrawPartialLigature(gfxFont *aFont, gfxContext *aCtx, PRUint32 aStart,
                             PRUint32 aEnd, const gfxRect *aDirtyRect, gfxPoint *aPt,
                             PropertyProvider *aProvider);
    // Advance aStart to the start of the nearest ligature; back up aEnd
    // to the nearest ligature end; may result in *aStart == *aEnd
    void ShrinkToLigatureBoundaries(PRUint32 *aStart, PRUint32 *aEnd);
    // result in appunits
    gfxFloat GetPartialLigatureWidth(PRUint32 aStart, PRUint32 aEnd, PropertyProvider *aProvider);
    void AccumulatePartialLigatureMetrics(gfxFont *aFont,
                                          PRUint32 aStart, PRUint32 aEnd, PRBool aTight,
                                          gfxContext *aRefContext,
                                          PropertyProvider *aProvider,
                                          Metrics *aMetrics);

    // **** measurement helper ****
    void AccumulateMetricsForRun(gfxFont *aFont, PRUint32 aStart,
                                 PRUint32 aEnd, PRBool aTight,
                                 gfxContext *aRefContext,
                                 PropertyProvider *aProvider,
                                 PRUint32 aSpacingStart, PRUint32 aSpacingEnd,
                                 Metrics *aMetrics);

    // **** drawing helper ****
    void DrawGlyphs(gfxFont *aFont, gfxContext *aContext, PRBool aDrawToPath,
                    gfxPoint *aPt, PRUint32 aStart, PRUint32 aEnd,
                    PropertyProvider *aProvider,
                    PRUint32 aSpacingStart, PRUint32 aSpacingEnd);

    // All our glyph data is in logical order, not visual.
    // mCharacterGlyphs is allocated fused with this object. We need a pointer
    // to it because gfxTextRun subclasses exist with extra fields, so we don't
    // know where it starts without a virtual method call or an explicit pointer.
    CompressedGlyph*                               mCharacterGlyphs;
    nsAutoArrayPtr<nsAutoArrayPtr<DetailedGlyph> > mDetailedGlyphs; // only non-null if needed
    // XXX this should be changed to a GlyphRun plus a maybe-null GlyphRun*,
    // for smaller size especially in the super-common one-glyphrun case
    nsAutoTArray<GlyphRun,1>                       mGlyphRuns;
    // When TEXT_IS_8BIT is set, we use mSingle, otherwise we use mDouble.
    // When TEXT_IS_PERSISTENT is set, we don't own the text, otherwise we
    // own the text. When we own the text, it's allocated fused with this
    // object, so it need not be deleted.
    // This text is not null-terminated.
    union {
        const PRUint8   *mSingle;
        const PRUnichar *mDouble;
    } mText;
    void             *mUserData;
    gfxFontGroup     *mFontGroup; // addrefed
    gfxSkipChars      mSkipChars;
    nsExpirationState mExpirationState;
    PRUint32          mAppUnitsPerDevUnit;
    PRUint32          mFlags;
    PRUint32          mCharacterCount;
    PRUint32          mHashCode;
};

class THEBES_API gfxFontGroup : public gfxTextRunFactory {
public:
    gfxFontGroup(const nsAString& aFamilies, const gfxFontStyle *aStyle);

    virtual ~gfxFontGroup() {
        mFonts.Clear();
    }

    virtual gfxFont *GetFontAt(PRInt32 i) {
        return static_cast<gfxFont*>(mFonts[i]);
    }
    virtual PRUint32 FontListLength() const {
        return mFonts.Length();
    }

    PRBool Equals(const gfxFontGroup& other) const {
        return mFamilies.Equals(other.mFamilies) &&
            mStyle.Equals(other.mStyle);
    }

    const gfxFontStyle *GetStyle() const { return &mStyle; }

    virtual gfxFontGroup *Copy(const gfxFontStyle *aStyle) = 0;

    /**
     * The listed characters should not be passed in to MakeTextRun and should
     * be treated as invisible and zero-width.
     */
    static PRBool IsInvalidChar(PRUnichar ch) {
        if (ch >= 32) {
            return ch == 0x0085/*NEL*/ ||
                ((ch & 0xFF00) == 0x2000 /* Unicode control character */ &&
                 (ch == 0x200B/*ZWSP*/ || ch == 0x2028/*LSEP*/ || ch == 0x2029/*PSEP*/ ||
                  IS_BIDI_CONTROL_CHAR(ch)));
        }
        // We could just blacklist all control characters, but it seems better
        // to only blacklist the ones we know cause problems for native font
        // engines.
        return ch == 0x0B || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\f' ||
            (ch >= 0x1c && ch <= 0x1f);
    }

    /**
     * Make a textrun for an empty string. This is fast; if you call it,
     * don't bother caching the result.
     */
    gfxTextRun *MakeEmptyTextRun(const Parameters *aParams, PRUint32 aFlags);
    /**
     * Make a textrun for a single ASCII space. This is fast; if you call it,
     * don't bother caching the result.
     */
    gfxTextRun *MakeSpaceTextRun(const Parameters *aParams, PRUint32 aFlags);

    /**
     * Make a textrun for a given string.
     * If aText is not persistent (aFlags & TEXT_IS_PERSISTENT), the
     * textrun will copy it.
     * This calls FetchGlyphExtents on the textrun.
     */
    virtual gfxTextRun *MakeTextRun(const PRUnichar *aString, PRUint32 aLength,
                                    const Parameters *aParams, PRUint32 aFlags) = 0;
    /**
     * Make a textrun for a given string.
     * If aText is not persistent (aFlags & TEXT_IS_PERSISTENT), the
     * textrun will copy it.
     * This calls FetchGlyphExtents on the textrun.
     */
    virtual gfxTextRun *MakeTextRun(const PRUint8 *aString, PRUint32 aLength,
                                    const Parameters *aParams, PRUint32 aFlags) = 0;

    /* helper function for splitting font families on commas and
     * calling a function for each family to fill the mFonts array
     */
    typedef PRBool (*FontCreationCallback) (const nsAString& aName,
                                            const nsACString& aGenericName,
                                            void *closure);
    static PRBool ForEachFont(const nsAString& aFamilies,
                              const nsACString& aLangGroup,
                              FontCreationCallback fc,
                              void *closure);
    PRBool ForEachFont(FontCreationCallback fc, void *closure);

    const nsString& GetFamilies() { return mFamilies; }

protected:
    nsString mFamilies;
    gfxFontStyle mStyle;
    nsTArray< nsRefPtr<gfxFont> > mFonts;

    /* If aResolveGeneric is true, then CSS/Gecko generic family names are
     * replaced with preferred fonts.
     *
     * If aResolveFontName is true then fc() is called only for existing fonts
     * and with actual font names.  If false then fc() is called with each
     * family name in aFamilies (after resolving CSS/Gecko generic family names
     * if aResolveGeneric).
     */
    static PRBool ForEachFontInternal(const nsAString& aFamilies,
                                      const nsACString& aLangGroup,
                                      PRBool aResolveGeneric,
                                      PRBool aResolveFontName,
                                      FontCreationCallback fc,
                                      void *closure);

    static PRBool FontResolverProc(const nsAString& aName, void *aClosure);
};
#endif
