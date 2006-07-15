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
 * The Original Code is Mozilla MathML Project.
 *
 * The Initial Developer of the Original Code is
 * The University Of Queensland.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 *   Shyjan Mahamud <mahamud@cs.cmu.edu>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsCOMPtr.h"
#include "nsFrame.h"
#include "nsPresContext.h"
#include "nsUnitConversion.h"
#include "nsStyleContext.h"
#include "nsStyleConsts.h"
#include "nsString.h"
#include "nsUnicharUtils.h"
#include "nsIRenderingContext.h"
#include "nsIFontMetrics.h"

#include "nsIPrefBranch.h"
#include "nsIPrefService.h"
#include "nsISupportsPrimitives.h"
#include "nsIComponentManager.h"
#include "nsIPersistentProperties2.h"
#include "nsIServiceManager.h"
#include "nsIObserverService.h"
#include "nsIObserver.h"
#include "nsNetUtil.h"

#include "nsILookAndFeel.h"
#include "nsIDeviceContext.h"
#include "nsCSSRendering.h"
#include "prprf.h"         // For PR_snprintf()

#include "nsIDOMWindow.h"
#include "nsINonBlockingAlertService.h"
#include "nsIWindowWatcher.h"
#include "nsIStringBundle.h"
#include "nsDoubleHashtable.h"
#include "nsDisplayList.h"

#include "nsMathMLOperators.h"
#include "nsMathMLChar.h"

//#define SHOW_BORDERS 1
//#define NOISY_SEARCH 1

// -----------------------------------------------------------------------------------
static const PRUnichar   kSpaceCh   = PRUnichar(' ');
static const nsGlyphCode kNullGlyph = {0, 0};

// -----------------------------------------------------------------------------------
// nsGlyphTable is a class that provides an interface for accessing glyphs
// of stretchy chars. It acts like a table that stores the variants of bigger
// sizes (if any) and the partial glyphs needed to build extensible symbols.
// An instance of nsGlyphTable is associated to one primary font. Extra glyphs
// can be taken in other additional fonts when stretching certain characters.
// These supplementary fonts are referred to as "external" fonts to the table.
//
// A char for which nsGlyphTable::Has(aChar) is true means that the table
// contains some glyphs (bigger and/or partial) that can be used to render
// the char. Bigger sizes (if any) of the char can then be retrieved with
// BigOf(aSize). Partial glyphs can be retrieved with TopOf(), GlueOf(), etc.
//
// A table consists of "nsGlyphCode"s which are viewed either as Unicode
// points or as direct glyph indices, depending on the type of the table.
// XXX The latter is not yet supported.

// General format of MathFont Property Files from which glyph data are retrieved:
// -----------------------------------------------------------------------------------
// Each font should have its set of glyph data. For example, the glyph data for
// the "Symbol" font and the "MT Extra" font are in "mathfontSymbol.properties"
// and "mathfontMTExtra.properties", respectively. The mathfont property file is a
// set of all the stretchy MathML characters that can be rendered with that font
// using larger and/or partial glyphs. The entry of each stretchy character in the
// mathfont property file gives, in that order, the 4 partial glyphs: Top (or Left),
// Middle, Bottom (or Right), Glue; and the variants of bigger sizes (if any).
// A position that is not relevant to a particular character is indicated there
// with the UNICODE REPLACEMENT CHARACTER 0xFFFD.
// Characters that need to be built recursively from other characters are said
// to be composite. For example, chars like over/underbrace in CMEX10 have to
// be built from two half stretchy chars and joined in the middle (TeXbook, p.225).
// Such chars are handled in a special manner by the nsMathMLChar class, which allows
// several (2 or more) child chars to be composed in order to render another char.
// To specify such chars, their list of glyphs in the property file should be given
// as space-separated segments of glyphs. Each segment gives the 4 partial
// glyphs with which to build the child char that will be joined with its other
// siblings. In this code, when this situation happens (see the detailed description
// of Stretch() below), the original char (referred to as "parent") creates a
// singly-linked list of child chars, asking them to stretch in an equally divided
// space. The nsGlyphTable embeds the necessary logic to guarantee correctness in a
// recursive stretch (and in the use of TopOf(), GlueOf(), etc) on these child chars.
// -----------------------------------------------------------------------------------

#define NS_TABLE_TYPE_UNICODE       0
#define NS_TABLE_TYPE_GLYPH_INDEX   1

#define NS_TABLE_STATE_ERROR       -1
#define NS_TABLE_STATE_EMPTY        0
#define NS_TABLE_STATE_READY        1

// Hook to resolve common assignments to the PUA
static nsCOMPtr<nsIPersistentProperties> gPUAProperties; 

// helper to check if a font is installed
static PRBool
CheckFontExistence(nsPresContext* aPresContext, const nsString& aFontName)
{
  PRBool aliased;
  nsAutoString localName;
  nsIDeviceContext *deviceContext = aPresContext->DeviceContext();
  deviceContext->GetLocalFontName(aFontName, localName, aliased);
  PRBool rv = (aliased || (NS_OK == deviceContext->CheckFontExistence(localName)));
  // (see bug 35824 for comments about the aliased localName)
  return rv;
}

// alert the user if some of the needed MathML fonts are not installed.
// it is non-modal (i.e., it doesn't wait for input from the user)
static void
AlertMissingFonts(nsString& aMissingFonts)
{
  nsCOMPtr<nsIStringBundleService> sbs(do_GetService(NS_STRINGBUNDLE_CONTRACTID));
  if (!sbs)
    return;

  nsCOMPtr<nsIStringBundle> sb;
  sbs->CreateBundle("resource://gre/res/fonts/mathfont.properties", getter_AddRefs(sb));
  if (!sb)
    return;

  nsXPIDLString title, message;
  const PRUnichar* strings[] = { aMissingFonts.get() };
  sb->GetStringFromName(NS_LITERAL_STRING("mathfont_missing_dialog_title").get(), getter_Copies(title));
  sb->FormatStringFromName(NS_LITERAL_STRING("mathfont_missing_dialog_message").get(),
                           strings, 1, getter_Copies(message));

  nsCOMPtr<nsIWindowWatcher> wwatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID));
  if (!wwatch)
    return;

  nsCOMPtr<nsIDOMWindow> parent;
  wwatch->GetActiveWindow(getter_AddRefs(parent));
  nsresult rv;
  nsCOMPtr<nsINonBlockingAlertService> prompter =
    do_GetService("@mozilla.org/embedcomp/nbalert-service;1", &rv);

  if (prompter && parent) {
    prompter->ShowNonBlockingAlert(parent, title.get(), message.get());
  }
}

// helper to trim off comments from data in a MathFont Property File
static void
Clean(nsString& aValue)
{
  // chop the trailing # comment portion if any ...
  PRInt32 comment = aValue.RFindChar('#');
  if (comment > 0) aValue.Truncate(comment);
  aValue.CompressWhitespace();
}

// helper to load a MathFont Property File
static nsresult
LoadProperties(const nsString& aName,
               nsCOMPtr<nsIPersistentProperties>& aProperties)
{
  nsAutoString uriStr;
  uriStr.AssignLiteral("resource://gre/res/fonts/mathfont");
  uriStr.Append(aName);
  uriStr.StripWhitespace(); // that may come from aName
  uriStr.AppendLiteral(".properties");
  return NS_LoadPersistentPropertiesFromURISpec(getter_AddRefs(aProperties), 
                                                NS_ConvertUTF16toUTF8(uriStr));
}

// helper to get the stretchy direction of a char
static nsStretchDirection
GetStretchyDirection(PRUnichar aChar)
{
  PRInt32 k = nsMathMLOperators::FindStretchyOperator(aChar);
  return (k == kNotFound)
    ? NS_STRETCH_DIRECTION_UNSUPPORTED
    : nsMathMLOperators::GetStretchyDirectionAt(k);
}

// -----------------------------------------------------------------------------------

class nsGlyphTable {
public:
  nsGlyphTable(const nsString& aPrimaryFontName)
  {
    MOZ_COUNT_CTOR(nsGlyphTable);
    mFontName.AppendString(aPrimaryFontName);
    mType = NS_TABLE_TYPE_UNICODE;
    mState = NS_TABLE_STATE_EMPTY;
    mCharCache = 0;
  }

  ~nsGlyphTable() // not a virtual destructor: this class is not intended to be subclassed
  {
    MOZ_COUNT_DTOR(nsGlyphTable);
  }

  void GetPrimaryFontName(nsString& aPrimaryFontName) {
    mFontName.StringAt(0, aPrimaryFontName);
  }

  // True if this table contains some glyphs (variants and/or parts)
  // or contains child chars that can be used to render this char
  PRBool Has(nsPresContext* aPresContext, nsMathMLChar* aChar);
  PRBool Has(nsPresContext* aPresContext, PRUnichar aChar);

  // True if this table contains variants of larger sizes to render this char
  PRBool HasVariantsOf(nsPresContext* aPresContext, nsMathMLChar* aChar);
  PRBool HasVariantsOf(nsPresContext* aPresContext, PRUnichar aChar);

  // True if this table contains parts (or composite parts) to render this char
  PRBool HasPartsOf(nsPresContext* aPresContext, nsMathMLChar* aChar);
  PRBool HasPartsOf(nsPresContext* aPresContext, PRUnichar aChar);

  // True if aChar is to be assembled from other child chars in this table
  PRBool IsComposite(nsPresContext* aPresContext, nsMathMLChar* aChar);

  // The number of child chars to assemble in order to render aChar
  PRInt32 ChildCountOf(nsPresContext* aPresContext, nsMathMLChar* aChar);

  // Getters for the parts
  nsGlyphCode TopOf(nsPresContext* aPresContext, nsMathMLChar* aChar) {
    return ElementAt(aPresContext, aChar, 0);
  }
  nsGlyphCode MiddleOf(nsPresContext* aPresContext, nsMathMLChar* aChar) {
    return ElementAt(aPresContext, aChar, 1);
  }
  nsGlyphCode BottomOf(nsPresContext* aPresContext, nsMathMLChar* aChar) {
    return ElementAt(aPresContext, aChar, 2);
  }
  nsGlyphCode GlueOf(nsPresContext* aPresContext, nsMathMLChar* aChar) {
    return ElementAt(aPresContext, aChar, 3);
  }
  nsGlyphCode BigOf(nsPresContext* aPresContext, nsMathMLChar* aChar, PRInt32 aSize) {
    return ElementAt(aPresContext, aChar, 4 + aSize);
  }
  nsGlyphCode LeftOf(nsPresContext* aPresContext, nsMathMLChar* aChar) {
    return ElementAt(aPresContext, aChar, 0);
  }
  nsGlyphCode RightOf(nsPresContext* aPresContext, nsMathMLChar* aChar) {
    return ElementAt(aPresContext, aChar, 2);
  }

  // use these to measure/render a glyph that comes from this table
  nsresult
  GetBoundingMetrics(nsIRenderingContext& aRenderingContext,
                     nsFont&              aFont,
                     nsGlyphCode&         aGlyphCode,
                     nsBoundingMetrics&   aBoundingMetrics);
  void
  DrawGlyph(nsIRenderingContext& aRenderingContext,
            nsFont&              aFont,
            nsGlyphCode&         aGlyphCode,
            nscoord              aX,
            nscoord              aY,
            nsRect*              aClipRect = nsnull);

private:
  char   GetAnnotation(nsMathMLChar* aChar, PRInt32 aPosition);
  nsGlyphCode ElementAt(nsPresContext* aPresContext, nsMathMLChar* aChar, PRUint32 aPosition);

  // The type is either NS_TABLE_TYPE_UNICODE or NS_TABLE_TYPE_GLYPH_INDEX
  PRInt32 mType;    
                           
  // mFontName[0] is the primary font associated to this table. The others 
  // are possible "external" fonts for glyphs not in the primary font
  // but which are needed to stretch certain characters in the table
  nsStringArray mFontName; 
                               
  // Tri-state variable for error/empty/ready
  PRInt32 mState;

  // The set of glyph data in this table, as provided by the MathFont Property File
  nsCOMPtr<nsIPersistentProperties> mGlyphProperties;

  // For speedy re-use, we always cache the last data used in the table.
  // mCharCache is the Unicode point of the last char that was queried in this
  // table. mGlyphCache is a buffer containing the glyph data associated to
  // that char. For a property line 'key = value' in the MathFont Property File,
  // mCharCache will retain the 'key' -- which is a Unicode point, while mGlyphCache
  // will retain the 'value', which is a consecutive list of nsGlyphCodes, i.e.,
  // the pairs of 'code@font' needed by the char -- in which 'code@0' can be specified
  // without the optional '@0'. However, to ease subsequent processing, mGlyphCache
  // excludes the '@' symbol and explicitly inserts all optional '0' that indicates
  // the primary font identifier. Specifically therefore, the k-th glyph is
  // characterized by :
  // 1) mGlyphCache[2*k] : its Unicode point (or glyph index -- depending on mType),
  // 2) mGlyphCache[2*k+1] : the numeric identifier of the font where it comes from.
  // A font identifier of '0' means the default primary font associated to this
  // table. Other digits map to the "external" fonts that may have been specified
  // in the MathFont Property File.
  nsString  mGlyphCache;
  PRUnichar mCharCache;
};

char
nsGlyphTable::GetAnnotation(nsMathMLChar* aChar, PRInt32 aPosition)
{
  NS_ASSERTION(aChar->mDirection == NS_STRETCH_DIRECTION_VERTICAL ||
               aChar->mDirection == NS_STRETCH_DIRECTION_HORIZONTAL,
               "invalid call");
  static const char kVertical[]   = "TMBG";
  static const char kHorizontal[] = "LMRG";
  if (aPosition >= 4) {
    // return an ASCII digit for the size=0,1,2,...
    return '0' + aPosition - 4;
  }
  return (aChar->mDirection == NS_STRETCH_DIRECTION_VERTICAL) ?
      kVertical[aPosition] :
      kHorizontal[aPosition];
}

nsGlyphCode
nsGlyphTable::ElementAt(nsPresContext* aPresContext, nsMathMLChar* aChar, PRUint32 aPosition)
{
  if (mState == NS_TABLE_STATE_ERROR) return kNullGlyph;
  // Load glyph properties if this is the first time we have been here
  if (mState == NS_TABLE_STATE_EMPTY) {
    if (!CheckFontExistence(aPresContext, *mFontName[0])) {
      mState = NS_TABLE_STATE_ERROR;
      return kNullGlyph;
    }
    nsresult rv = LoadProperties(*mFontName[0], mGlyphProperties);
#ifdef NS_DEBUG
    nsCAutoString uriStr;
    uriStr.AssignLiteral("resource://gre/res/fonts/mathfont");
    LossyAppendUTF16toASCII(*mFontName[0], uriStr);
    uriStr.StripWhitespace(); // that may come from mFontName
    uriStr.AppendLiteral(".properties");
    printf("Loading %s ... %s\n",
            uriStr.get(),
            (NS_FAILED(rv)) ? "Failed" : "Done");
#endif
    if (NS_FAILED(rv)) {
      mState = NS_TABLE_STATE_ERROR; // never waste time with this table again
      return kNullGlyph;
    }
    mState = NS_TABLE_STATE_READY;

    // see if there are external fonts needed for certain chars in this table
    nsCAutoString key;
    nsAutoString value;
    for (PRInt32 i = 1; ; i++) {
      key.AssignLiteral("external.");
      key.AppendInt(i, 10);
      rv = mGlyphProperties->GetStringProperty(key, value);
      if (NS_FAILED(rv)) break;
      Clean(value);
      mFontName.AppendString(value); // i.e., mFontName[i] holds this font name
    }
  }

  // If aChar is a child char to be used by a parent composite char, make
  // sure that it is really attached to this table
  if (aChar->mParent && (aChar->mGlyphTable != this)) return kNullGlyph;

  // Update our cache if it is not associated to this character
  PRUnichar uchar = aChar->mData[0];
  if (mCharCache != uchar) {
    // The key in the property file is interpreted as ASCII and kept
    // as such ...
    char key[10]; PR_snprintf(key, sizeof(key), "\\u%04X", uchar);
    nsAutoString value;
    nsresult rv = mGlyphProperties->GetStringProperty(nsDependentCString(key), value);
    if (NS_FAILED(rv)) return kNullGlyph;
    Clean(value);
    // See if this char uses external fonts; e.g., if the 2nd glyph is taken from the
    // external font '1', the property line looks like \uNNNN = \uNNNN\uNNNN@1\uNNNN.
    // This is where mGlyphCache is pre-processed to explicitly store all glyph codes
    // as combined pairs of 'code@font', excluding the '@' separator. This means that
    // mGlyphCache[2*k] will later be rendered with mFontName[mGlyphCache[2*k+1]-'0']
    // Note: font identifier is internally an ASCII digit to avoid the null char issue
    nsAutoString buffer, puaValue;
    char puaKey[10];
    PRInt32 length = value.Length();
    for (PRInt32 i = 0, j = 0; i < length; i++, j++) {
      PRUnichar code = value[i];
      PRUnichar font = PRUnichar('0');
      // see if we are at the beginning of a child char
      if (code == kSpaceCh) {
        // reset the annotation indicator to be 0 for the next code point
        j = -1;
      }
      // see if this code point is an *indirect reference* to
      // the PUA, and lookup "key.[TLMBRG1-9]" in the PUA
      else if (code == PRUnichar(0xF8FF)) {
        PR_snprintf(puaKey, sizeof(puaKey), "%s.%c", key, GetAnnotation(aChar, j));
        rv = gPUAProperties->GetStringProperty(nsDependentCString(puaKey), puaValue);
        if (NS_FAILED(rv) || puaValue.IsEmpty()) return kNullGlyph;
        code = puaValue[0];
      }
      // see if this code point is a *direct reference* to
      // the PUA, and lookup "code.[TLMBRG1-9]" in the PUA
      else if ((i+2 < length) && (value[i+1] == PRUnichar('.'))) {
        i += 2;
        // safe cast of value[i], it's ascii
        PR_snprintf(puaKey, sizeof(puaKey), "\\u%04X.%c", code, char(value[i]));
        rv = gPUAProperties->GetStringProperty(nsDependentCString(puaKey), puaValue);
        if (NS_FAILED(rv) || puaValue.IsEmpty()) return kNullGlyph;
        code = puaValue[0];
      }
      // see if an external font is needed for the code point
      if ((i+2 < length) && (value[i+1] == PRUnichar('@')) &&
          (value[i+2] >= PRUnichar('0')) && (value[i+2] <= PRUnichar('9'))) {
        i += 2;
        font = value[i];
        // The char cannot be handled if this font is not installed
        nsAutoString fontName;
        mFontName.StringAt(font-'0', fontName);
        if (!fontName.Length() || !CheckFontExistence(aPresContext, fontName)) {
          return kNullGlyph;
        }
      }
      buffer.Append(code);
      buffer.Append(font);
    }
    // update our cache with the new settings
    mGlyphCache.Assign(buffer);
    mCharCache = uchar;
  }

  // If aChar is a composite char, only its children are allowed
  // to use its glyphs in this table, i.e., the parent char itself
  // is disabled and cannot be stretched directly with these glyphs.
  // This guarantees a coherent behavior in Stretch().
  if (!aChar->mParent && (kNotFound != mGlyphCache.FindChar(kSpaceCh))) {
    return kNullGlyph;
  }

  // If aChar is a child char, the index of the glyph is relative to
  // the offset of the list of glyphs corresponding to the child char
  PRUint32 offset = 0;
  PRUint32 length = mGlyphCache.Length();
  if (aChar->mParent) {
    nsMathMLChar* child = aChar->mParent->mSibling;
    while (child && (child != aChar)) {
      offset += 5; // skip the 4 partial glyphs + the whitespace separator
      child = child->mSibling;
    }
    length = 2*(offset + 4); // stay confined in the 4 partial glyphs of this child
  }
  PRUint32 index = 2*(offset + aPosition); // 2* is to account for the code@font pairs
  if (index+1 >= length) return kNullGlyph;
  nsGlyphCode ch;
  ch.code = mGlyphCache.CharAt(index);
  ch.font = mGlyphCache.CharAt(index + 1) - '0'; // the ASCII trick is kept internal...
  return (ch == PRUnichar(0xFFFD)) ? kNullGlyph : ch;
}

PRBool
nsGlyphTable::IsComposite(nsPresContext* aPresContext, nsMathMLChar* aChar)
{
  // there is only one level of recursion in our model. a child
  // cannot be composite because it cannot have its own children
  if (aChar->mParent) return PR_FALSE;
  // shortcut to sync the cache with this char...
  mCharCache = 0; mGlyphCache.Truncate(); ElementAt(aPresContext, aChar, 0);
  // the cache remained empty if the char wasn't found in this table
  if (8 >= mGlyphCache.Length()) return PR_FALSE;
  // the lists of glyphs of a composite char are space-separated
  return (kSpaceCh == mGlyphCache.CharAt(8));
}

PRInt32
nsGlyphTable::ChildCountOf(nsPresContext* aPresContext, nsMathMLChar* aChar)
{
  // this will sync the cache as well ...
  if (!IsComposite(aPresContext, aChar)) return 0;
  // the lists of glyphs of a composite char are space-separated
  return 1 + mGlyphCache.CountChar(kSpaceCh);
}

PRBool
nsGlyphTable::Has(nsPresContext* aPresContext, nsMathMLChar* aChar)
{
  return HasVariantsOf(aPresContext, aChar) || HasPartsOf(aPresContext, aChar);
}

PRBool
nsGlyphTable::Has(nsPresContext* aPresContext, PRUnichar aChar)
{
  nsMathMLChar tmp;
  tmp.mData = aChar;
  tmp.mDirection = GetStretchyDirection(aChar);
  return (tmp.mDirection == NS_STRETCH_DIRECTION_UNSUPPORTED)
    ? PR_FALSE
    : Has(aPresContext, &tmp);
}

PRBool
nsGlyphTable::HasVariantsOf(nsPresContext* aPresContext, nsMathMLChar* aChar)
{
  return BigOf(aPresContext, aChar, 1) != 0;
}

PRBool
nsGlyphTable::HasVariantsOf(nsPresContext* aPresContext, PRUnichar aChar)
{
  nsMathMLChar tmp;
  tmp.mData = aChar;
  tmp.mDirection = GetStretchyDirection(aChar);
  return (tmp.mDirection == NS_STRETCH_DIRECTION_UNSUPPORTED)
    ? PR_FALSE
    : HasVariantsOf(aPresContext, &tmp);
}

PRBool
nsGlyphTable::HasPartsOf(nsPresContext* aPresContext, nsMathMLChar* aChar)
{
  return  GlueOf(aPresContext, aChar) || TopOf(aPresContext, aChar) ||
          BottomOf(aPresContext, aChar) || MiddleOf(aPresContext, aChar) ||
          IsComposite(aPresContext, aChar);
}

PRBool
nsGlyphTable::HasPartsOf(nsPresContext* aPresContext, PRUnichar aChar)
{
  nsMathMLChar tmp;
  tmp.mData = aChar;
  tmp.mDirection = GetStretchyDirection(aChar);
  return (tmp.mDirection == NS_STRETCH_DIRECTION_UNSUPPORTED)
    ? PR_FALSE
    : HasPartsOf(aPresContext, &tmp);
}

// Get the bounding box of a glyph.
// Our primary font is assumed to be the current font in the rendering context
nsresult
nsGlyphTable::GetBoundingMetrics(nsIRenderingContext& aRenderingContext,
                                 nsFont&              aFont,
                                 nsGlyphCode&         aGlyphCode,
                                 nsBoundingMetrics&   aBoundingMetrics)
{
  nsresult rv;
  if (aGlyphCode.font) {
    // glyph not associated to our primary font, it comes from an external font
    mFontName.StringAt(aGlyphCode.font, aFont.name);
    aRenderingContext.SetFont(aFont, nsnull);
  }

  //if (mType == NS_TABLE_TYPE_UNICODE)
    rv = aRenderingContext.GetBoundingMetrics((PRUnichar*)&aGlyphCode.code, PRUint32(1), aBoundingMetrics);
  //else mType == NS_TABLE_TYPE_GLYPH_INDEX
  //return NS_ERROR_NOT_IMPLEMENTED;
  //rv = aRenderingContext.GetBoundingMetricsI((PRUint16*)&aGlyphCode.code, PRUint32(1), aBoundingMetrics);

  if (aGlyphCode.font) {
    // restore our primary font in the rendering context
    mFontName.StringAt(0, aFont.name);
    aRenderingContext.SetFont(aFont, nsnull);
  }
  return rv;
}

// Draw a glyph in a clipped area so that we don't have hairy chars pending outside
// Our primary font is assumed to be the current font in the rendering context
void
nsGlyphTable::DrawGlyph(nsIRenderingContext& aRenderingContext,
                        nsFont&              aFont,
                        nsGlyphCode&         aGlyphCode,
                        nscoord              aX,
                        nscoord              aY,
                        nsRect*              aClipRect)
{
  if (aClipRect) {
    aRenderingContext.PushState();
    aRenderingContext.SetClipRect(*aClipRect, nsClipCombine_kIntersect);
  }
  if (aGlyphCode.font) {
    // glyph not associated to our primary font, it comes from an external font
    mFontName.StringAt(aGlyphCode.font, aFont.name);
    aRenderingContext.SetFont(aFont, nsnull);
  }

  //if (mType == NS_TABLE_TYPE_UNICODE)
    aRenderingContext.DrawString((PRUnichar*)&aGlyphCode.code, PRUint32(1), aX, aY);
  //else
  //NS_ASSERTION(0, "Error *** Not yet implemented");
  //aRenderingContext.DrawStringI((PRUint16*)&aGlyphCode.code, PRUint32(1), aX, aY);

  if (aGlyphCode.font) {
    // restore our primary font in the rendering context
    mFontName.StringAt(0, aFont.name);
    aRenderingContext.SetFont(aFont, nsnull);
  }
  if (aClipRect)
    aRenderingContext.PopState();
}

// class to map a Unicode point to a string (used to store the list of
// fonts preferred for the base size of certain characters, i.e., when
// stretching doesn't happen with a char, we use its preferred base fonts)
class nsBaseFontEntry : public PLDHashInt32Entry
{
public:
  nsBaseFontEntry(const void* aKey) : PLDHashInt32Entry(aKey) { }
  ~nsBaseFontEntry() { };

  nsString mFontFamily; // a font-family list a-la CSS
};

DECL_DHASH_WRAPPER(nsBaseFontHashtable, nsBaseFontEntry, PRInt32)
DHASH_WRAPPER(nsBaseFontHashtable, nsBaseFontEntry, PRInt32)

// -----------------------------------------------------------------------------------
// This is the list of all the applicable glyph tables.
// We will maintain a single global instance that will only reveal those
// glyph tables that are associated to fonts currently installed on the
// user' system. The class is an XPCOM shutdown observer to allow us to
// free its allocated data at shutdown

class nsGlyphTableList : public nsIObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER

  // Hashtable to cache the preferred fonts of some chars at their base size
  static nsBaseFontHashtable* gBaseFonts;

  // These are placeholders used to cache the indices (in mTableList) of
  // the preferred extension tables for the particular chars.
  // Each stretchy char can have a preferred ordered list of fonts to
  // be used for its parts, and/or another preferred ordered list of 
  // fonts to be used for its variants of larger sizes.
  // Several levels of indirection are used to store this information.
  // The stretchy chars are collated in an array in nsMathMLOperators.
  // If 'index' is the rank of a stretchy char in that array, then
  // mTableList[gParts[index]] is the first preferred table to be used for
  // the parts of that stretchy char, mTableList[gParts[index]+1] is the
  // second table, etc. The same reasoning applies with gVariants[index].
  static PRInt32* gParts;
  static PRInt32* gVariants;

  nsGlyphTableList()
  {
    MOZ_COUNT_CTOR(nsGlyphTableList);
    mDefaultCount = 0;
  }

  virtual ~nsGlyphTableList()
  {
    MOZ_COUNT_DTOR(nsGlyphTableList);
  }

  nsresult Initialize();
  nsresult Finalize();

  nsGlyphTable* TableAt(PRInt32 aIndex) {
    return NS_STATIC_CAST(nsGlyphTable*, mTableList.ElementAt(aIndex));
  }
  PRInt32 Count(PRBool aEverything = PR_FALSE) {
    return (aEverything) ? mTableList.Count() : mDefaultCount;
  }

  nsGlyphTable* AdditionalTableAt(PRInt32 aIndex) {
    return NS_STATIC_CAST(nsGlyphTable*, mAdditionalTableList.ElementAt(aIndex));
  }
  PRInt32 AdditionalCount() {
    return mAdditionalTableList.Count();
  }

  PRBool AppendTable(nsGlyphTable* aGlyphTable) {
    return mTableList.AppendElement(aGlyphTable);
  }

  // Add a glyph table in the list, return the new table that was added
  nsGlyphTable*
  AddGlyphTable(const nsString& aPrimaryFontName);
  nsGlyphTable*
  AddAdditionalGlyphTable(const nsString& aPrimaryFontName);

  // Find a glyph table in the list that has a glyph for the given char
  nsGlyphTable*
  GetGlyphTableFor(nsPresContext* aPresContext,
                   nsMathMLChar*   aChar);

  // Find the subset of glyph tables that are applicable to the given char,
  // knowing that the stretchy style context of the char has the given font.
  nsresult
  GetListFor(nsPresContext* aPresContext,
             nsMathMLChar*   aChar,
             nsFont*         aFont,
             nsVoidArray*    aGlyphTableList);

  // Retrieve the subset of preferred glyph tables that start at the given index
  // Return the number of installed fonts that are retrieved or 0 if none is found.
  // If at least one font is found, the preferred fonts become active and
  // take precedence (i.e., whatever was in the existing aGlyphTableList is
  // cleared). But if it turns out that no preferred font is actually installed,
  // the code behaves as if no preferred font was specified at all (i.e., whatever
  // was in aGlyphTableList is retained).
  nsresult
  GetPreferredListAt(nsPresContext* aPresContext,
                     PRInt32         aStartingIndex, 
                     nsVoidArray*    aGlyphTableList,
                     PRInt32*        aCount);

private:
  // Ordered list of glyph tables subdivided in several null-separated segments.
  // The first segment contains mDefaultCount entries which are the default
  // fonts as provided in the mathfont.properties file. The remainder of the
  // list is used to store the preferred tables for the particular chars
  // as explained above.
  PRInt32     mDefaultCount;
  nsVoidArray mTableList;
  // Users can prefer certain fonts for a character, but without wanting those
  // fonts to be used for other characters. mAdditionalTableList is a list of
  // preferred fonts that are not meant to be used as a default sharable list by
  // all characters. Note that mTableList[0..mDefaultCount-1] and mAdditionalTableList
  // are kept mutually exclusive since there is no need to load the same table twice.
  nsVoidArray mAdditionalTableList; 
};

NS_IMPL_ISUPPORTS1(nsGlyphTableList, nsIObserver)

// -----------------------------------------------------------------------------------
// Here is the global list of applicable glyph tables that we will be using
static nsGlyphTableList* gGlyphTableList = nsnull;
nsBaseFontHashtable* nsGlyphTableList::gBaseFonts = nsnull;
PRInt32* nsGlyphTableList::gParts = nsnull;
PRInt32* nsGlyphTableList::gVariants = nsnull;

static PRBool gInitialized = PR_FALSE;

// XPCOM shutdown observer
NS_IMETHODIMP
nsGlyphTableList::Observe(nsISupports*     aSubject,
                          const char* aTopic,
                          const PRUnichar* someData)
{
  Finalize();
  // destroy the PUA
  gPUAProperties = nsnull;
  return NS_OK;
}

// Add an observer to XPCOM shutdown so that we can free our data at shutdown
nsresult
nsGlyphTableList::Initialize()
{
  nsresult rv = NS_OK;
  nsCOMPtr<nsIObserverService> obs = 
           do_GetService("@mozilla.org/observer-service;1", &rv);
  if (NS_SUCCEEDED(rv)) {
    rv = obs->AddObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID, PR_FALSE);
  }
  return rv;
}

// Remove our observer and free the memory that were allocated for us
nsresult
nsGlyphTableList::Finalize()
{
  // Remove our observer from the observer service
  nsresult rv = NS_OK;
  nsCOMPtr<nsIObserverService> obs = 
           do_GetService("@mozilla.org/observer-service;1", &rv);
  if (NS_SUCCEEDED(rv)) {
    rv = obs->RemoveObserver(this, NS_XPCOM_SHUTDOWN_OBSERVER_ID);
  }
  // delete the glyph tables
  PRInt32 i;
  for (i = Count() - 1; i >= 0; i--) {
    nsGlyphTable* glyphTable = TableAt(i);
    delete glyphTable;
  }
  for (i = AdditionalCount() - 1; i >= 0; i--) {
    nsGlyphTable* glyphTable = AdditionalTableAt(i);
    delete glyphTable;
  }
  // delete the other variables
  delete gBaseFonts;
  delete [] gParts;
  delete [] gVariants;
  gParts = gVariants = nsnull;
  gInitialized = PR_FALSE;
  // our oneself will be destroyed when our |Release| is called by the observer
  return rv;
}

nsGlyphTable*
nsGlyphTableList::AddGlyphTable(const nsString& aPrimaryFontName)
{
  // allocate a table to be deleted at shutdown
  nsGlyphTable* glyphTable = new nsGlyphTable(aPrimaryFontName);
  if (!glyphTable) return nsnull;
  mTableList.AppendElement(glyphTable);
  mDefaultCount++;
  return glyphTable;
}

nsGlyphTable*
nsGlyphTableList::AddAdditionalGlyphTable(const nsString& aPrimaryFontName)
{
  // allocate a table to be deleted at shutdown
  nsGlyphTable* glyphTable = new nsGlyphTable(aPrimaryFontName);
  if (!glyphTable) return nsnull;
  mAdditionalTableList.AppendElement(glyphTable);
  return glyphTable;
}

nsGlyphTable*
nsGlyphTableList::GetGlyphTableFor(nsPresContext* aPresContext, 
                                   nsMathMLChar*   aChar)
{
  PRInt32 i;
  for (i = 0; i < Count(); i++) {
    nsGlyphTable* glyphTable = TableAt(i);
    if (glyphTable->Has(aPresContext, aChar)) {
      return glyphTable;
    }
  }
  for (i = 0; i < AdditionalCount(); i++) {
    nsGlyphTable* glyphTable = AdditionalTableAt(i);
    if (glyphTable->Has(aPresContext, aChar)) {
      return glyphTable;
    }
  }
  return nsnull;
}

struct StretchyFontEnumContext {
  nsPresContext* mPresContext;
  nsMathMLChar*   mChar;
  nsVoidArray*    mGlyphTableList;
};

// check if the current font is associated to a known glyph table, if so the
// glyph table is added to the list of tables that can be used for the char
static PRBool
StretchyFontEnumCallback(const nsString& aFamily, PRBool aGeneric, void *aData)
{
  if (aGeneric) return PR_FALSE; // stop now
  StretchyFontEnumContext* context = (StretchyFontEnumContext*)aData;
  nsPresContext* currPresContext = context->mPresContext;
  nsMathMLChar* currChar = context->mChar;
  nsVoidArray* currList = context->mGlyphTableList;
  // check if the current font is associated to a known glyph table
  for (PRInt32 i = 0; i < gGlyphTableList->Count(); i++) {
    nsGlyphTable* glyphTable = gGlyphTableList->TableAt(i);
    nsAutoString fontName;
    glyphTable->GetPrimaryFontName(fontName);
    if (fontName.Equals(aFamily, nsCaseInsensitiveStringComparator()) &&
        glyphTable->Has(currPresContext, currChar)) {
      currList->AppendElement(glyphTable); // the table is retained
      return PR_TRUE; // don't stop
    }
  }
  return PR_TRUE; // don't stop
}

nsresult
nsGlyphTableList::GetListFor(nsPresContext* aPresContext,
                             nsMathMLChar*   aChar,
                             nsFont*         aFont,
                             nsVoidArray*    aGlyphTableList)
{
  // @see the documentation of -moz-math-stretchy in mathml.css
  // for how this work
  aGlyphTableList->Clear();
  PRBool useDocumentFonts =
    aPresContext->GetCachedBoolPref(kPresContext_UseDocumentFonts);

  // Check to honor the pref("browser.display.use_document_fonts", 0)
  // Only include fonts from CSS if the pref to disallow authors' fonts isn't set
  if (useDocumentFonts) {
    // Convert the list of fonts in aFont (from -moz-math-stretchy)
    // to an ordered list of corresponding glyph extension tables
    StretchyFontEnumContext context = {aPresContext, aChar, aGlyphTableList};
    aFont->EnumerateFamilies(StretchyFontEnumCallback, &context);
  }
  if (!aGlyphTableList->Count()) {
    // No font was retained, fallback to our default tables
    PRInt32 count = Count();
    for (PRInt32 i = 0; i < count; i++) {
      nsGlyphTable* glyphTable = TableAt(i);
      if (glyphTable->Has(aPresContext, aChar)) {
        aGlyphTableList->AppendElement(glyphTable);
      }
    }
  }
  return NS_OK;
}

nsresult
nsGlyphTableList::GetPreferredListAt(nsPresContext* aPresContext,
                                     PRInt32         aStartingIndex, 
                                     nsVoidArray*    aGlyphTableList,
                                     PRInt32*        aCount)
{
  *aCount = 0;
  if (aStartingIndex == kNotFound) {
    return NS_OK;
  }
  nsAutoString fontName;
  PRInt32 index = aStartingIndex;
  NS_ASSERTION(index < Count(PR_TRUE), "invalid call");
  nsGlyphTable* glyphTable = TableAt(index);
  while (glyphTable) {
    glyphTable->GetPrimaryFontName(fontName);
    if (CheckFontExistence(aPresContext, fontName)) {
#ifdef NOISY_SEARCH
      printf("Found preferreed font %s\n",
             NS_LossyConvertUTF16toASCII(fontName).get());
#endif
      if (index == aStartingIndex) {
        // At least one font is found, clear aGlyphTableList
        aGlyphTableList->Clear();
      }
      aGlyphTableList->AppendElement(glyphTable);
      ++*aCount;
    }
    glyphTable = TableAt(++index);
  } 
  // XXX append other tables if UseDocumentFonts is set?
  return NS_OK;
}

// -----------------------------------------------------------------------------------

struct PreferredFontEnumContext {
  PRInt32   mCharIndex;
  PRBool    mIsFontForParts;
  PRInt32   mFontCount;
};

// mark a glyph table as a preferred table that can be used for a char
static PRBool
PreferredFontEnumCallback(const nsString& aFamily, PRBool aGeneric, void *aData)
{
  PRInt32 i;
  nsAutoString fontName;
  nsGlyphTable* glyphTable = nsnull;
  PreferredFontEnumContext* context = (PreferredFontEnumContext*)aData;
  // see if the table already exists in mTableList[0..mDefaultCount-1]
  PRBool found = PR_FALSE;
  PRInt32 count = gGlyphTableList->Count();
  for (i = 0; i < count; i++) {
    glyphTable = gGlyphTableList->TableAt(i);
    glyphTable->GetPrimaryFontName(fontName);
    if (fontName.Equals(aFamily, nsCaseInsensitiveStringComparator())) {
      found = PR_TRUE;
      break;
    }
  }
  if (!found) {
    // the table wasn't found in the default sharable list,
    // see if it exists in the additional list
    count = gGlyphTableList->AdditionalCount();
    for (i = 0; i < count; i++) {
      glyphTable = gGlyphTableList->AdditionalTableAt(i);
      glyphTable->GetPrimaryFontName(fontName);
      if (fontName.Equals(aFamily, nsCaseInsensitiveStringComparator())) {
        found = PR_TRUE;
        break;
      }
    }
    if (!found) {
      // the table wasn't found in the additional list either, add it now
      glyphTable = gGlyphTableList->AddAdditionalGlyphTable(aFamily);
      if (!glyphTable)
        return PR_FALSE; // stop in low-memory situations
    }
  }

  // Add the table to the list of preferred extension tables for this char
  if (!context->mFontCount) {
    // this is the first font to be retained, remember
    // the starting index where the first glyphTable was appended
    PRInt32 startingIndex = gGlyphTableList->Count(PR_TRUE);
    if (context->mIsFontForParts) {
      NS_ASSERTION(nsGlyphTableList::gParts[context->mCharIndex] == -1,
                   "remove duplicate property in mathfont.properties");
      nsGlyphTableList::gParts[context->mCharIndex] = startingIndex;
    }
    else {
      NS_ASSERTION(nsGlyphTableList::gVariants[context->mCharIndex] == -1,
                   "remove duplicate property in mathfont.properties");
      nsGlyphTableList::gVariants[context->mCharIndex] = startingIndex;
    }
  }

  gGlyphTableList->AppendTable(glyphTable);
  ++context->mFontCount;

  return PR_TRUE; // don't stop
}

// retrieve a pref value set by the user
static PRBool
GetPrefValue(nsIPrefBranch* aPrefBranch, const char* aPrefKey, nsString& aPrefValue)
{
  aPrefValue.Truncate();
  if (aPrefBranch) {
    nsCOMPtr<nsISupportsString> prefString;
    aPrefBranch->GetComplexValue(aPrefKey,
                                 NS_GET_IID(nsISupportsString),
                                 getter_AddRefs(prefString));
    if (prefString) {
      prefString->GetData(aPrefValue);
    }
  }
  return !aPrefValue.IsEmpty();
}

// Store the list of preferred extension fonts for a char
static void
SetPreferredFonts(const char* aKey, nsString& aFamilyList)
{
  NS_ASSERTION(30 < strlen(aKey), "invalid call");

  // expected key:
  // "font.mathfont-family.\uNNNN.base"     -- fonts for the base size
  // "font.mathfont-family.\uNNNN.parts"    -- fonts for partial glyphs
  // "font.mathfont-family.\uNNNN.variants" -- fonts for larger glyphs
  PRInt32 error = 0;
  // 22 is to skip "font.mathfont-family.\\u";
  PRUnichar uchar = nsCAutoString(aKey + 22).ToInteger(&error, 16);
  if (error) return;
  // 27 is to skip "font.mathfont-family.\\uNNNN"
  const char* extension = aKey + 27;

#ifdef DEBUG_rbs
  printf("Setting preferred fonts for \\u%04X%s: %s\n", uchar, extension,
         NS_LossyConvertUTF16toASCII(aFamilyList).get());
#endif

  if (!strcmp(extension, ".base")) {
    // fonts to be used for the base size of the char (i.e., no stretching)
    nsBaseFontEntry* entry = nsGlyphTableList::gBaseFonts->AddEntry(uchar);
    if (entry) {
      entry->mFontFamily = aFamilyList;
    }
    return;
  }

  PRBool isFontForParts;
  if (!strcmp(extension, ".parts"))
    isFontForParts = PR_TRUE;
  else if (!strcmp(extension, ".variants"))
    isFontForParts = PR_FALSE;
  else return; // input is not applicable

  // Ensure that this is a valid stretchy operator
  PRInt32 k = nsMathMLOperators::FindStretchyOperator(uchar);
  if (k != kNotFound) {
    // We just want to iterate over the font-family list using the
    // callback mechanism that nsFont has...
    nsFont font(aFamilyList, 0, 0, 0, 0, 0);
    PreferredFontEnumContext context = {k, isFontForParts, 0};
    font.EnumerateFamilies(PreferredFontEnumCallback, &context);
    if (context.mFontCount) { // at least one font was retained
      // Append a null separator
      gGlyphTableList->AppendTable(nsnull);
    }
  }
}

struct MathFontEnumContext {
  nsPresContext* mPresContext;
  nsString*       mMissingFamilyList;
};

static PRBool
MathFontEnumCallback(const nsString& aFamily, PRBool aGeneric, void *aData)
{
  // check if the font is missing
  MathFontEnumContext* context = (MathFontEnumContext*)aData;
  nsPresContext* presContext = context->mPresContext;
  nsString* missingFamilyList = context->mMissingFamilyList;
  if (!CheckFontExistence(presContext, aFamily)) {
//#ifndef _WIN32
   // XXX In principle, the mathfont-family list in the mathfont.properties file
   // is customizable depending on the platform. For now, this is here since there
   // is no need to alert Linux users about TrueType fonts specific to Windows.
   if (aFamily.LowerCaseEqualsLiteral("mt extra"))
     return PR_TRUE; // continue to try other fonts
//#endif
    if (!missingFamilyList->IsEmpty()) {
      missingFamilyList->AppendLiteral(", ");
    }
    missingFamilyList->Append(aFamily);
  }

  if (!gGlyphTableList->AddGlyphTable(aFamily))
    return PR_FALSE; // stop in low-memory situations
  return PR_TRUE; // don't stop
}

static nsresult
InitGlobals(nsPresContext* aPresContext)
{
  NS_ASSERTION(!gInitialized, "Error -- already initialized");
  gInitialized = PR_TRUE;
  PRUint32 count = nsMathMLOperators::CountStretchyOperator();
  if (!count) {
    // nothing to stretch, so why bother...
    return NS_OK;
  }

  // Allocate the placeholders for the preferred parts and variants
  nsresult rv = NS_ERROR_OUT_OF_MEMORY;
  gGlyphTableList = new nsGlyphTableList();
  nsGlyphTableList::gBaseFonts = new nsBaseFontHashtable();
  if (gGlyphTableList && nsGlyphTableList::gBaseFonts) {
    nsGlyphTableList::gParts = new PRInt32[count];
    nsGlyphTableList::gVariants = new PRInt32[count];
    if (nsGlyphTableList::gParts && nsGlyphTableList::gVariants) {
      rv = gGlyphTableList->Initialize();
    }
  }
  if (NS_FAILED(rv)) {
    delete gGlyphTableList;
    delete nsGlyphTableList::gBaseFonts;
    delete [] nsGlyphTableList::gParts;
    delete [] nsGlyphTableList::gVariants;
    gGlyphTableList = nsnull;
    nsGlyphTableList::gBaseFonts = nsnull;
    nsGlyphTableList::gParts = nsnull;
    nsGlyphTableList::gVariants = nsnull;
    return rv;
  }
  /*
  else
    The gGlyphTableList has been successfully registered as a shutdown observer.
    It will be deleted at shutdown, even if a failure happens below.
  */

  PRUint32 i;
  for (i = 0; i < count; i++) {
    nsGlyphTableList::gParts[i] = kNotFound; // i.e., -1
    nsGlyphTableList::gVariants[i] = kNotFound; // i.e., -1
  }
  nsGlyphTableList::gBaseFonts->Init(5);

  nsCAutoString key;
  nsAutoString value;
  nsCOMPtr<nsIPersistentProperties> mathfontProp;
  nsCOMPtr<nsIPrefBranch> prefBranch(do_GetService(NS_PREFSERVICE_CONTRACTID));

  // Add the math fonts in the gGlyphTableList in order of preference ...
  // Note: we only load font-names at this stage. The actual glyph tables will
  // be loaded lazily (see nsGlyphTable::ElementAt()).

  // Load the "mathfont.properties" file
  value.Truncate();
  rv = LoadProperties(value, mathfontProp);
  if (NS_FAILED(rv)) return rv;

  // Load the "mathfontPUA.properties" file
  value.AssignLiteral("PUA");
  rv = LoadProperties(value, gPUAProperties);
  if (NS_FAILED(rv)) return rv;

  // Get the default list of mathfonts to be used for stretchy characters
  nsFont font(nsnull, 0, 0, 0, 0, 0);
  NS_NAMED_LITERAL_CSTRING(defaultKey, "font.mathfont-family");
  if (!GetPrefValue(prefBranch, defaultKey.get(), font.name)) {
    // fallback to the internal default list
    rv = mathfontProp->GetStringProperty(defaultKey, font.name);
    if (NS_FAILED(rv)) return rv;
  }

  // Parse the font list and append an entry for each family to gGlyphTableList
  nsAutoString missingFamilyList;
  MathFontEnumContext context = {aPresContext, &missingFamilyList};
  font.EnumerateFamilies(MathFontEnumCallback, &context);
  // Append a null separator
  gGlyphTableList->AppendTable(nsnull);

  // alert the user if some of the expected fonts are missing
  if (!missingFamilyList.IsEmpty()) {
    AlertMissingFonts(missingFamilyList);
  }

  // Let the particular characters have their preferred fonts

  // First, look the prefs of the user
  char **allKey = nsnull;
  prefBranch->GetChildList("font.mathfont-family.", &count, &allKey);    
  for (i = 0; i < count; ++i) {
#ifdef DEBUG_rbs
    GetPrefValue(prefBranch, allKey[i], value);
    printf("Found user pref %s: %s\n", allKey[i],
           NS_LossyConvertUTF16toASCII(value).get());
#endif
    if ((30 < strlen(allKey[i])) && 
        GetPrefValue(prefBranch, allKey[i], value)) {
      SetPreferredFonts(allKey[i], value);
    }
  }
  NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(count, allKey);

  // Next, look our internal settings
  nsCOMPtr<nsISimpleEnumerator> iterator;
  if (NS_SUCCEEDED(mathfontProp->Enumerate(getter_AddRefs(iterator)))) {
    PRBool more;
    while ((NS_SUCCEEDED(iterator->HasMoreElements(&more))) && more) {
      nsCOMPtr<nsIPropertyElement> element;
      if (NS_SUCCEEDED(iterator->GetNext(getter_AddRefs(element)))) {
        if (NS_SUCCEEDED(element->GetKey(key))) {
          if ((30 < key.Length()) && 
              (0 == key.Find("font.mathfont-family.\\u")) &&
              !GetPrefValue(prefBranch, key.get(), value) && // priority to user
              NS_SUCCEEDED(element->GetValue(value))) {
            Clean(value);
            SetPreferredFonts(key.get(), value);
          }
        }
      }
    }
  }
  return rv;
}

// helper to override CSS and set the default font-family list to be used
// for the base size of a particular character (i.e., in the situation where
// stretching doesn't happen).
static void
SetBaseFamily(PRUnichar aChar, nsFont& aFont)
{
  if (!nsGlyphTableList::gBaseFonts) return;
  nsBaseFontEntry* entry = nsGlyphTableList::gBaseFonts->GetEntry(aChar);
  if (entry) {
    aFont.name.Assign(entry->mFontFamily);
  }
}

// -----------------------------------------------------------------------------------
// And now the implementation of nsMathMLChar

nsStyleContext*
nsMathMLChar::GetStyleContext() const
{
  NS_ASSERTION(!mParent, "invalid call - not allowed for child chars");
  NS_ASSERTION(mStyleContext, "chars shoud always have style context");
  return mStyleContext;
  return NS_OK;
}

void
nsMathMLChar::SetStyleContext(nsStyleContext* aStyleContext)
{
  NS_ASSERTION(!mParent, "invalid call - not allowed for child chars");
  NS_PRECONDITION(aStyleContext, "null ptr");
  if (aStyleContext != mStyleContext) {
    if (mStyleContext)
      mStyleContext->Release();
    if (aStyleContext) {
      mStyleContext = aStyleContext;
      aStyleContext->AddRef();

      // Sync the pointers of child chars.
      nsMathMLChar* child = mSibling;
      while (child) {
        child->mStyleContext = mStyleContext;
        child = child->mSibling;
      }
    }
  }
}

void
nsMathMLChar::SetData(nsPresContext* aPresContext,
                      nsString&       aData)
{
  NS_ASSERTION(!mParent, "invalid call - not allowed for child chars");
  if (!gInitialized) {
    InitGlobals(aPresContext);
  }
  mData = aData;
  // some assumptions until proven otherwise
  // note that mGlyph is not initialized
  mOperator = -1;
  mDirection = NS_STRETCH_DIRECTION_UNSUPPORTED;
  mBoundingMetrics.Clear();
  mGlyphTable = nsnull;
  // check if stretching is applicable ...
  if (gGlyphTableList && (1 == mData.Length())) {
    mOperator = nsMathMLOperators::FindStretchyOperator(mData[0]);
    if (mOperator >= 0) {
      mDirection = nsMathMLOperators::GetStretchyDirectionAt(mOperator);
      // default tentative table (not the one that is necessarily going to be used)
      mGlyphTable = gGlyphTableList->GetGlyphTableFor(aPresContext, this);
      // commom case: we won't bother with the stretching if there is
      // no glyph table for us...
      if (!mGlyphTable) {
        // never try to stretch this operator again
        nsMathMLOperators::DisableStretchyOperatorAt(mOperator);
        mDirection = NS_STRETCH_DIRECTION_UNSUPPORTED;
        mOperator = -1;
      }
    }
  }
}

// -----------------------------------------------------------------------------------
/*
 The Stretch:
 @param aContainerSize - suggested size for the stretched char
 @param aDesiredStretchSize - OUT parameter. The desired size
 after stretching. If no stretching is done, the output will
 simply give the base size.

 How it works?
 Summary:-
 The Stretch() method first looks for a glyph of appropriate
 size; If a glyph is found, it is cached by this object and
 its size is returned in aDesiredStretchSize. The cached
 glyph will then be used at the painting stage.
 If no glyph of appropriate size is found, a search is made
 to see if the char can be built by parts.

 Details:-
 A character gets stretched through the following pipeline :

 1) If the base size of the char is sufficient to cover the
    container' size, we use that. If not, it will still be
    used as a fallback if the other stages in the pipeline fail.
    Issues :
    a) The base size, the parts and the variants of a char can
       be in different fonts. For eg., the base size for '(' should
       come from a normal ascii font if CMEX10 is used, since CMEX10
       only contains the stretched versions. Hence, there are two
       style contexts in use throughout the process. The leaf style
       context of the char holds fonts with which to try to stretch
       the char. The parent style context of the char contains fonts
       for normal rendering. So the parent context is the one used
       to get the initial base size at the start of the pipeline.
    b) For operators that can be largeop's in display mode,
       we will skip the base size even if it fits, so that
       the next stage in the pipeline is given a chance to find
       a largeop variant. If the next stage fails, we fallback
       to the base size.

 2) We search for the first larger variant of the char that fits the
    container' size. We search fonts for larger variants in the order
    specified in the list of stretchy fonts held by the leaf style
    context (from -moz-math-stretchy in mathml.css).
    Issues :
    a) the largeop and display settings determine the starting
       size when we do the above search, regardless of whether
       smaller variants already fit the container' size.
    b) if it is a largeopOnly request (i.e., a displaystyle operator
       with largeop=true and stretchy=false), we break after finding
       the first starting variant, regardless of whether that
       variant fits the container's size.

 3) If a variant of appropriate size wasn't found, we see if the char
    can be built by parts. We search the ordered list of stretchy fonts
    for the first font which has parts that fit the container' size.
    Issues:
    a) Certain chars like over/underbrace in CMEX10 have to be built
       from two half stretchy chars and joined in the middle. Such
       chars are handled in a special manner. When this situation is
       detected, the initial char (referred to as "parent") creates a
       singly-linked list of child chars, asking them to stretch in
       a divided space. A convention is used in the setup of
       nsGlyphTable to express that a composite parent char can be built
       from child chars.
    b) There are some chars that have no middle and glue glyphs. For
       such chars, the parts need to be joined using the rule.
       By convention (TeXbook p.225), the descent of the parts is
       zero while their ascent gives the thickness of the rule that
       should be used to join them.

 Of note:
 When the pipeline completes successfully, the desired size of the
 stretched char can actually be slighthly larger or smaller than
 aContainerSize. But it is the responsibility of the caller to
 account for the spacing when setting aContainerSize, and to leave
 any extra margin when placing the stretched char.
*/
// -----------------------------------------------------------------------------------


// plain TeX settings (TeXbook p.152)
#define NS_MATHML_DELIMITER_FACTOR      0.901f
#define NS_MATHML_DELIMITER_SHORTFALL   NSFloatPointsToTwips(5.0f)

static PRBool
IsSizeOK(nscoord a, nscoord b, PRUint32 aHint)
{
  // Normal: True if 'a' is around +/-10% of the target 'b' (10% is
  // 1-DelimiterFactor). This often gives a chance to the base size to
  // win, especially in the context of <mfenced> without tall elements
  // or in sloppy markups without protective <mrow></mrow>
  PRBool isNormal =
    (aHint & NS_STRETCH_NORMAL)
    && PRBool(float(PR_ABS(a - b))
              < (1.0f - NS_MATHML_DELIMITER_FACTOR) * float(b));
  // Nearer: True if 'a' is around max{ +/-10% of 'b' , 'b' - 5pt },
  // as documented in The TeXbook, Ch.17, p.152.
  PRBool isNearer = PR_FALSE;
  if (aHint & (NS_STRETCH_NEARER | NS_STRETCH_LARGEOP)) {
    float c = PR_MAX(float(b) * NS_MATHML_DELIMITER_FACTOR,
                     float(b) - NS_MATHML_DELIMITER_SHORTFALL);
    isNearer = PRBool(float(PR_ABS(b - a)) <= (float(b) - c));
  }
  // Smaller: Mainly for transitory use, to compare two candidate
  // choices
  PRBool isSmaller =
    (aHint & NS_STRETCH_SMALLER)
    && PRBool((float(a) >= (NS_MATHML_DELIMITER_FACTOR * float(b)))
              && (a <= b));
  // Larger: Critical to the sqrt code to ensure that the radical
  // size is tall enough
  PRBool isLarger =
    (aHint & (NS_STRETCH_LARGER | NS_STRETCH_LARGEOP))
    && PRBool(a >= b);
  return (isNormal || isSmaller || isNearer || isLarger);
}

static PRBool
IsSizeBetter(nscoord a, nscoord olda, nscoord b, PRUint32 aHint)
{
  if (0 == olda) return PR_TRUE;
  if (PR_ABS(a - b) < PR_ABS(olda - b)) {
    if (aHint & (NS_STRETCH_NORMAL | NS_STRETCH_NEARER))
      return PR_TRUE;
    if (aHint & NS_STRETCH_SMALLER)
      return PRBool(a < olda);
    if (aHint & (NS_STRETCH_LARGER | NS_STRETCH_LARGEOP))
      return PRBool(a > olda);
  }
  return PR_FALSE;
}

// We want to place the glyphs even when they don't fit at their
// full extent, i.e., we may clip to tolerate a small amount of
// overlap between the parts. This is important to cater for fonts
// with long glues.
static nscoord
ComputeSizeFromParts(nsGlyphCode* aGlyphs,
                     nscoord*     aSizes,
                     nscoord      aTargetSize,
                     PRUint32     aHint)
{
  enum {first, middle, last, glue};
  float flex[] = {0.901f, 0.901f, 0.901f};
  // refine the flexibility depending on whether some parts can be left out
  if (aGlyphs[glue] == aGlyphs[middle]) flex[middle] = 0.0f;
  if (aGlyphs[glue] == aGlyphs[first]) flex[first] = 0.0f;
  if (aGlyphs[glue] == aGlyphs[last]) flex[last] = 0.0f;

  // get the minimum allowable size
  nscoord computedSize = nscoord(flex[first] * aSizes[first] +
                                 flex[middle] * aSizes[middle] +
                                 flex[last] * aSizes[last]);

  if (computedSize <= aTargetSize) {
    // if we can afford more room, the default is to fill-up the target area
    return aTargetSize;
  }
  if (IsSizeOK(computedSize, aTargetSize, aHint)) {
    // settle with the size, and let Paint() do the rest
    return computedSize;
  }
  // reject these parts
  return 0;
}

// Put aFamily in the first position of aFont to guarantee that our
// desired font is the one that the GFX font sub-system will use
inline void
SetFirstFamily(nsFont& aFont, const nsString& aFamily)
{
  // overwrite the old value of font-family:
  aFont.name.Assign(aFamily);
}

nsresult
nsMathMLChar::Stretch(nsPresContext*      aPresContext,
                      nsIRenderingContext& aRenderingContext,
                      nsStretchDirection   aStretchDirection,
                      nsBoundingMetrics&   aContainerSize,
                      nsBoundingMetrics&   aDesiredStretchSize,
                      PRUint32             aStretchHint)
{
  nsresult rv = NS_OK;
  nsStretchDirection direction = aStretchDirection;

  // if we have been called before, and we didn't actually stretch, our
  // direction may have been set to NS_STRETCH_DIRECTION_UNSUPPORTED.
  // So first set our direction back to its instrinsic value
  if (mOperator >= 0) {
    // mOperator is initialized in SetData() and remains unchanged
    mDirection = nsMathMLOperators::GetStretchyDirectionAt(mOperator);
  }

  // if no specified direction, attempt to stretch in our preferred direction
  if (direction == NS_STRETCH_DIRECTION_DEFAULT) {
    direction = mDirection;
  }

  // Set default font and get the default bounding metrics
  // mStyleContext is a leaf context used only when stretching happens.
  // For the base size, the default font should come from the parent context
  nsAutoString fontName;
  nsFont theFont(mStyleContext->GetParent()->GetStyleFont()->mFont);

  // Override with specific fonts if applicable for this character
  PRUnichar uchar = mData[0];
  SetBaseFamily(uchar, theFont);
  aRenderingContext.SetFont(theFont, nsnull);
  rv = aRenderingContext.GetBoundingMetrics(mData.get(),
                                            PRUint32(mData.Length()),
                                            mBoundingMetrics);
  if (NS_FAILED(rv)) {
    NS_WARNING("GetBoundingMetrics failed");
    // ensure that the char later behaves like a normal char
    // (will be reset back to its intrinsic value in case of dynamic updates)
    mDirection = NS_STRETCH_DIRECTION_UNSUPPORTED;
    return rv;
  }

  // set the default desired metrics in case stretching doesn't happen
  aDesiredStretchSize = mBoundingMetrics;

  // quick return if there is nothing special about this char
  if (!mGlyphTable || (mDirection != direction)) {
    // ensure that the char later behaves like a normal char
    // (will be reset back to its intrinsic value in case of dynamic updates)
    mDirection = NS_STRETCH_DIRECTION_UNSUPPORTED;
    return NS_OK;
  }

  // see if this is a particular largeop or largeopOnly request
  PRBool largeop = (NS_STRETCH_LARGEOP & aStretchHint) != 0;
  PRBool largeopOnly = (NS_STRETCH_LARGEOP == aStretchHint); // (==, not mask!)

  ////////////////////////////////////////////////////////////////////////////////////
  // 1. Check the common situations where stretching is not actually needed
  ////////////////////////////////////////////////////////////////////////////////////

  nscoord targetSize, charSize;
  PRBool isVertical = (direction == NS_STRETCH_DIRECTION_VERTICAL);
  if (isVertical) {
    charSize = aDesiredStretchSize.ascent + aDesiredStretchSize.descent;
    targetSize = aContainerSize.ascent + aContainerSize.descent;
  }
  else {
    charSize = aDesiredStretchSize.width;
    targetSize = aContainerSize.width;
  }
  // if we are not a largeop in display mode, return if size fits
  if ((targetSize <= 0) || 
      (!largeop && ((isVertical && charSize >= targetSize) ||
                     IsSizeOK(charSize, targetSize, aStretchHint)))) {
    // ensure that the char later behaves like a normal char
    // (will be reset back to its intrinsic value in case of dynamic updates)
    mDirection = NS_STRETCH_DIRECTION_UNSUPPORTED;
    return NS_OK;
  }

  ////////////////////////////////////////////////////////////////////////////////////
  // 2. Try to search if there is a glyph of appropriate size
  ////////////////////////////////////////////////////////////////////////////////////

  PRInt32 size;
  nsGlyphTable* glyphTable;
  nsBoundingMetrics bm;
  nsGlyphCode startingGlyph = {uchar, 0}; // code@font
  nsGlyphCode ch;

  // this will be the best glyph that we encounter during the search...
  nsGlyphCode bestGlyph = startingGlyph;
  nsGlyphTable* bestGlyphTable = mGlyphTable;
  nsBoundingMetrics bestbm = mBoundingMetrics;

  // use our stretchy style context now that stretching is in progress
  theFont = mStyleContext->GetStyleFont()->mFont;

  // initialize the search list for this char
  PRBool alreadyCSS = PR_FALSE;
  nsAutoVoidArray tableList;
  // see if there are user-specified preferred tables for the variants of this char
  PRInt32 count, t = nsGlyphTableList::gVariants[mOperator];
  gGlyphTableList->GetPreferredListAt(aPresContext, t, &tableList, &count);
  if (!count) {
    // get a list that attempts to honor the css font-family
    gGlyphTableList->GetListFor(aPresContext, this, &theFont, &tableList);
    alreadyCSS = PR_TRUE;
  }

#ifdef NOISY_SEARCH
  printf("Searching in %d fonts for a glyph of appropriate size for: 0x%04X:%c\n",
          tableList.Count(), uchar, uchar&0x00FF);
#endif

  count = tableList.Count();
  for (t = 0; t < count; t++) {
    // see if this table has a glyph that matches the container
    glyphTable = NS_STATIC_CAST(nsGlyphTable*, tableList.ElementAt(t));
    // figure out the starting size : if this is a largeop, start at 2 else 1
    size = 1; // size=0 is the char at its normal size
    if (largeop && glyphTable->BigOf(aPresContext, this, 2)) {
      size = 2;
    }
    glyphTable->GetPrimaryFontName(fontName);
    SetFirstFamily(theFont, fontName);
    aRenderingContext.SetFont(theFont, nsnull);
#ifdef NOISY_SEARCH
    printf("  searching in %s ...\n",
           NS_LossyConvertUTF16toASCII(fontName).get());
#endif
    ch = glyphTable->BigOf(aPresContext, this, size++);
    while (ch) {
      NS_ASSERTION(ch != uchar, "glyph table incorrectly set -- duplicate found");
      rv = glyphTable->GetBoundingMetrics(aRenderingContext, theFont, ch, bm);
      if (NS_SUCCEEDED(rv)) {
        charSize = (isVertical)
                 ? bm.ascent + bm.descent
                 : bm.rightBearing - bm.leftBearing;
        // always break when largeopOnly is set
        if (largeopOnly || IsSizeOK(charSize, targetSize, aStretchHint)) {
#ifdef NOISY_SEARCH
          printf("    size:%d OK!\n", size-1);
#endif
          bestbm = bm;
          bestGlyphTable = glyphTable;
          bestGlyph = ch;
          goto done; // get out...
        }
        nscoord oldSize = (isVertical)
                        ? bestbm.ascent + bestbm.descent
                        : bestbm.rightBearing - bestbm.leftBearing;
        if (IsSizeBetter(charSize, oldSize, targetSize, aStretchHint)) {
          bestGlyphTable = glyphTable;
          bestGlyph = ch;
          bestbm = bm;
#ifdef NOISY_SEARCH
          printf("    size:%d Current best\n", size-1);
        }
        else {
          printf("    size:%d Rejected!\n", size-1);
#endif
        }
      }
      // if largeopOnly is set, break now
      if (largeopOnly) break;
      ch = glyphTable->BigOf(aPresContext, this, size++);
    }
  }
  if (largeopOnly) goto done; // the user doesn't want to stretch

  ////////////////////////////////////////////////////////////////////////////////////
  // Build by parts. If no glyph of appropriate size was found, see if we can
  // build the char by parts. If there are preferred tables, they are used. Otherwise,
  // search for the first table with suitable parts for this char
  ////////////////////////////////////////////////////////////////////////////////////

  // see if there are user-specified preferred tables for the parts of this char
  t = nsGlyphTableList::gParts[mOperator];
  gGlyphTableList->GetPreferredListAt(aPresContext, t, &tableList, &count);
  if (!count && !alreadyCSS) {
    // we didn't do this earlier... so we need to do it here:
    // get a list that attempts to honor the css font-family
    gGlyphTableList->GetListFor(aPresContext, this, &theFont, &tableList);
  }

#ifdef NOISY_SEARCH
  printf("Searching in %d fonts for the first font with suitable parts for: 0x%04X:%c\n",
          tableList.Count(), uchar, uchar&0x00FF);
#endif

  count = tableList.Count();
  for (t = 0; t < count; t++) {
    glyphTable = NS_STATIC_CAST(nsGlyphTable*, tableList.ElementAt(t));
    if (!glyphTable->HasPartsOf(aPresContext, this)) continue; // to next table

    // See if this is a composite character //////////////////////////////////////////
    if (glyphTable->IsComposite(aPresContext, this)) {
      // let the child chars do the job
      nsBoundingMetrics compositeSize;
      rv = ComposeChildren(aPresContext, aRenderingContext, glyphTable,
                           aContainerSize, compositeSize, aStretchHint);
#ifdef NOISY_SEARCH
      printf("    Composing %d chars in font %s %s!\n",
             glyphTable->ChildCountOf(aPresContext, this),
             NS_LossyConvertUTF16toASCII(fontName).get(),
             NS_SUCCEEDED(rv)? "OK" : "Rejected");
#endif
      if (NS_FAILED(rv)) continue; // to next table

      // all went well, painting will be delegated from now on to children
      mGlyph = kNullGlyph; // this will tell paint to build by parts
      mGlyphTable = glyphTable;
      mBoundingMetrics = compositeSize;
      aDesiredStretchSize = compositeSize;
      return NS_OK; // get out ...
    }

    // See if the parts of this table fit in the desired space ///////////////////////
    glyphTable->GetPrimaryFontName(fontName);
    SetFirstFamily(theFont, fontName);
    aRenderingContext.SetFont(theFont, nsnull);
    // Compute the bounding metrics of all partial glyphs
    PRInt32 i;
    nsGlyphCode chdata[4];
    nsBoundingMetrics bmdata[4];
    nscoord computedSize, sizedata[4];
    nsGlyphCode glue = glyphTable->GlueOf(aPresContext, this);
    for (i = 0; i < 4; i++) {
      switch (i) {
        case 0: ch = glyphTable->TopOf(aPresContext, this);    break;
        case 1: ch = glyphTable->MiddleOf(aPresContext, this); break;
        case 2: ch = glyphTable->BottomOf(aPresContext, this); break;
        case 3: ch = glue;                                     break;
      }
      // empty slots are filled with the glue if it is not null
      if (!ch) ch = glue;
      if (!ch) { // glue is null, set bounding metrics to 0
        bm.Clear();
      }
      else {
        rv = glyphTable->GetBoundingMetrics(aRenderingContext, theFont, ch, bm);
        if (NS_FAILED(rv)) {
          // stop if we failed to compute the bounding metrics of a part.
          NS_WARNING("GetBoundingMetrics failed");
          break;
        }
      }
      chdata[i] = ch;
      bmdata[i] = bm;
      sizedata[i] = (isVertical)
                  ? bm.ascent + bm.descent
                  : bm.rightBearing - bm.leftBearing;
    }
    if (NS_FAILED(rv)) continue; // to next table

    // Build by parts if we have successfully computed the
    // bounding metrics of all parts.
    computedSize = ComputeSizeFromParts(chdata, sizedata, targetSize, aStretchHint);
#ifdef NOISY_SEARCH
    printf("    Font %s %s!\n",
           NS_LossyConvertUTF16toASCII(fontName).get(),
           (computedSize) ? "OK" : "Rejected");
#endif
    if (!computedSize) continue; // to next table

    // the computed size is suitable for the available space...
    // now is the time to compute and cache our bounding metrics
    if (isVertical) {
      nscoord lbearing = bmdata[0].leftBearing;
      nscoord rbearing = bmdata[0].rightBearing;
      nscoord width = bmdata[0].width;
      for (i = 1; i < 4; i++) {
        bm = bmdata[i];
        if (width < bm.width) width = bm.width;
        if (lbearing > bm.leftBearing) lbearing = bm.leftBearing;
        if (rbearing < bm.rightBearing) rbearing = bm.rightBearing;
      }
      bestbm.width = width;
      bestbm.ascent = bmdata[0].ascent; // Yes top, so that it works with TeX sqrt!
      bestbm.descent = computedSize - bestbm.ascent;
      bestbm.leftBearing = lbearing;
      bestbm.rightBearing = rbearing;
    }
    else {
      nscoord ascent = bmdata[0].ascent;
      nscoord descent = bmdata[0].descent;
      for (i = 1; i < 4; i++) {
        bm = bmdata[i];
        if (ascent < bm.ascent) ascent = bm.ascent;
        if (descent < bm.descent) descent = bm.descent;
      }
      bestbm.width = computedSize;
      bestbm.ascent = ascent;
      bestbm.descent = descent;
      bestbm.leftBearing = 0;
      bestbm.rightBearing = computedSize;
    }
    // reset
    bestGlyph = kNullGlyph; // this will tell paint to build by parts
    bestGlyphTable = glyphTable;
    goto done; // get out...
  }
#ifdef NOISY_SEARCH
  printf("    No font with suitable parts found\n");
#endif
  // if sum of parts doesn't fit in the space... we will use a single
  // glyph -- the base size or the best glyph encountered during the search

done:
  if (bestGlyph == startingGlyph) { // nothing happened
    // ensure that the char behaves like a normal char
    // (will be reset back to its intrinsic value in case of dynamic updates)
    mDirection = NS_STRETCH_DIRECTION_UNSUPPORTED;
  }
  else {
    // will stretch
    mGlyph = bestGlyph; // note that this can be null to tell paint to build by parts
    mGlyphTable = bestGlyphTable;
    mBoundingMetrics = bestbm;
    aDesiredStretchSize = bestbm;
  }
  return NS_OK;
}

nsresult
nsMathMLChar::ComposeChildren(nsPresContext*      aPresContext,
                              nsIRenderingContext& aRenderingContext,
                              nsGlyphTable*        aGlyphTable,
                              nsBoundingMetrics&   aContainerSize,
                              nsBoundingMetrics&   aCompositeSize,
                              PRUint32             aStretchHint)
{
  PRInt32 i = 0;
  nsMathMLChar* child;
  PRInt32 count = aGlyphTable->ChildCountOf(aPresContext, this);
  NS_ASSERTION(count, "something is wrong somewhere");
  if (!count) return NS_ERROR_FAILURE;
  // if we haven't been here before, create the linked list of children now
  // otherwise, use what we have, adding more children as needed or deleting the extra
  nsMathMLChar* last = this;
  while ((i < count) && last->mSibling) {
    i++;
    last = last->mSibling;
  }
  while (i < count) {
    child = new nsMathMLChar(this);
    if (!child) {
      if (mSibling) delete mSibling; // don't leave a dangling list ...
      mSibling = nsnull;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    last->mSibling = child;
    last = child;
    i++;
  }
  if (last->mSibling) {
    delete last->mSibling;
    last->mSibling = nsnull;
  }
  // let children stretch in an equal space
  nsBoundingMetrics splitSize(aContainerSize);
  if (NS_STRETCH_DIRECTION_HORIZONTAL == mDirection)
    splitSize.width /= count;
  else {
    splitSize.ascent = ((splitSize.ascent + splitSize.descent) / count) / 2;
    splitSize.descent = splitSize.ascent;
  }
  nscoord dx = 0, dy = 0;
  for (i = 0, child = mSibling; child; child = child->mSibling, i++) {
    // child chars should just inherit our values - which may change between calls...
    child->mData = mData;
    child->mOperator = mOperator;
    child->mDirection = mDirection;
    child->mStyleContext = mStyleContext;
    child->mGlyphTable = aGlyphTable; // the child is associated to this table
    // there goes the Stretch() ...
    nsBoundingMetrics childSize;
    nsresult rv = child->Stretch(aPresContext, aRenderingContext, mDirection,
                                 splitSize, childSize, aStretchHint);
    // check if something went wrong or the child couldn't fit in the alloted space
    if (NS_FAILED(rv) || (NS_STRETCH_DIRECTION_UNSUPPORTED == child->mDirection)) {
      delete mSibling; // don't leave a dangling list behind ...
      mSibling = nsnull;
      return NS_ERROR_FAILURE;
    }
    child->SetRect(nsRect(dx, dy, childSize.width, childSize.ascent+childSize.descent));
    if (0 == i)
      aCompositeSize = childSize;
    else {
      if (NS_STRETCH_DIRECTION_HORIZONTAL == mDirection)
        aCompositeSize += childSize;
      else {
        aCompositeSize.descent += childSize.ascent + childSize.descent;
        if (aCompositeSize.leftBearing > childSize.leftBearing)
          aCompositeSize.leftBearing = childSize.leftBearing;
        if (aCompositeSize.rightBearing < childSize.rightBearing)
          aCompositeSize.rightBearing = childSize.rightBearing;
      }
    }
    if (NS_STRETCH_DIRECTION_HORIZONTAL == mDirection)
      dx += childSize.width;
    else
      dy += childSize.ascent + childSize.descent;
  }
  return NS_OK;
}

class nsDisplayMathMLSelectionRect : public nsDisplayItem {
public:
  nsDisplayMathMLSelectionRect(nsIFrame* aFrame, const nsRect& aRect)
    : nsDisplayItem(aFrame), mRect(aRect) {
    MOZ_COUNT_CTOR(nsDisplayMathMLSelectionRect);
  }
#ifdef NS_BUILD_REFCNT_LOGGING
  virtual ~nsDisplayMathMLSelectionRect() {
    MOZ_COUNT_DTOR(nsDisplayMathMLSelectionRect);
  }
#endif

  virtual void Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect);
  NS_DISPLAY_DECL_NAME("MathMLSelectionRect")
private:
  nsRect    mRect;
};

void nsDisplayMathMLSelectionRect::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect)
{
  // get color to use for selection from the look&feel object
  nscolor bgColor = NS_RGB(0, 0, 0);
  mFrame->GetPresContext()->LookAndFeel()->
      GetColor(nsILookAndFeel::eColor_TextSelectBackground, bgColor);
  aCtx->SetColor(bgColor);
  aCtx->FillRect(mRect + aBuilder->ToReferenceFrame(mFrame));
}

class nsDisplayMathMLCharBackground : public nsDisplayItem {
public:
  nsDisplayMathMLCharBackground(nsIFrame* aFrame, const nsRect& aRect,
      nsStyleContext* aStyleContext)
    : nsDisplayItem(aFrame), mStyleContext(aStyleContext), mRect(aRect) {
    MOZ_COUNT_CTOR(nsDisplayMathMLCharBackground);
  }
#ifdef NS_BUILD_REFCNT_LOGGING
  virtual ~nsDisplayMathMLCharBackground() {
    MOZ_COUNT_DTOR(nsDisplayMathMLCharBackground);
  }
#endif

  virtual void Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect);
  NS_DISPLAY_DECL_NAME("MathMLCharBackground")
private:
  nsStyleContext* mStyleContext;
  nsRect          mRect;
};

void nsDisplayMathMLCharBackground::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect)
{
  const nsStyleBorder* border = mStyleContext->GetStyleBorder();
  const nsStylePadding* padding = mStyleContext->GetStylePadding();
  const nsStyleBackground* backg = mStyleContext->GetStyleBackground();
  nsCSSRendering::PaintBackgroundWithSC(mFrame->GetPresContext(), *aCtx, mFrame,
                                        aDirtyRect,
                                        mRect + aBuilder->ToReferenceFrame(mFrame),
                                        *backg, *border, *padding,
                                        PR_TRUE);
}

class nsDisplayMathMLCharForeground : public nsDisplayItem {
public:
  nsDisplayMathMLCharForeground(nsIFrame* aFrame, nsMathMLChar* aChar,
				PRBool aIsSelected)
    : nsDisplayItem(aFrame), mChar(aChar), mIsSelected(aIsSelected) {
    MOZ_COUNT_CTOR(nsDisplayMathMLCharForeground);
  }
#ifdef NS_BUILD_REFCNT_LOGGING
  virtual ~nsDisplayMathMLCharForeground() {
    MOZ_COUNT_DTOR(nsDisplayMathMLCharForeground);
  }
#endif

  virtual void Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect);
  NS_DISPLAY_DECL_NAME("MathMLCharForeground")
private:
  nsMathMLChar* mChar;
  PRPackedBool  mIsSelected;
};

void nsDisplayMathMLCharForeground::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect)
{
  mChar->PaintForeground(mFrame->GetPresContext(), *aCtx,
                         aBuilder->ToReferenceFrame(mFrame), mIsSelected);
}

#ifdef NS_DEBUG
class nsDisplayMathMLCharDebug : public nsDisplayItem {
public:
  nsDisplayMathMLCharDebug(nsIFrame* aFrame, const nsRect& aRect)
    : nsDisplayItem(aFrame), mRect(aRect) {
    MOZ_COUNT_CTOR(nsDisplayMathMLCharDebug);
  }
#ifdef NS_BUILD_REFCNT_LOGGING
  virtual ~nsDisplayMathMLCharDebug() {
    MOZ_COUNT_DTOR(nsDisplayMathMLCharDebug);
  }
#endif

  virtual void Paint(nsDisplayListBuilder* aBuilder, nsIRenderingContext* aCtx,
     const nsRect& aDirtyRect);
  NS_DISPLAY_DECL_NAME("MathMLCharDebug")
private:
  nsRect    mRect;
};

void nsDisplayMathMLCharDebug::Paint(nsDisplayListBuilder* aBuilder,
     nsIRenderingContext* aCtx, const nsRect& aDirtyRect)
{
  // for visual debug
  PRIntn skipSides = 0;
  nsPresContext* presContext = mFrame->GetPresContext();
  const nsStyleBorder* border = mFrame->GetStyleBorder();
  nsStyleContext* styleContext = mFrame->GetStyleContext();
  nsRect rect = mRect + aBuilder->ToReferenceFrame(mFrame);
  nsCSSRendering::PaintBorder(presContext, *aCtx, mFrame,
                              aDirtyRect, rect, *border, styleContext, skipSides);
  nsCSSRendering::PaintOutline(presContext, *aCtx, mFrame,
                               aDirtyRect, rect, *border,
                               *mFrame->GetStyleOutline(), styleContext, 0);
}
#endif


nsresult
nsMathMLChar::Display(nsDisplayListBuilder*   aBuilder,
                      nsIFrame*               aForFrame,
                      const nsDisplayListSet& aLists,
                      const nsRect*           aSelectedRect)
{
  nsresult rv = NS_OK;
  nsStyleContext* parentContext = mStyleContext->GetParent();
  nsStyleContext* styleContext = mStyleContext;

  if (NS_STRETCH_DIRECTION_UNSUPPORTED == mDirection) {
    // normal drawing if there is nothing special about this char
    // Set default context to the parent context
    styleContext = parentContext;
  }

  if (!styleContext->GetStyleVisibility()->IsVisible())
    return NS_OK;

  // if the leaf style context that we use for stretchy chars has a background
  // color we use it -- this feature is mostly used for testing and debugging
  // purposes. Normally, users will set the background on the container frame.
  // paint the selection background -- beware MathML frames overlap a lot
  if (aSelectedRect && !aSelectedRect->IsEmpty()) {
    rv = aLists.BorderBackground()->AppendNewToTop(new (aBuilder)
        nsDisplayMathMLSelectionRect(aForFrame, *aSelectedRect));
    NS_ENSURE_SUCCESS(rv, rv);
  }
  else if (mRect.width && mRect.height) {
    const nsStyleBackground* backg = styleContext->GetStyleBackground();
    if (styleContext != parentContext &&
        0 == (backg->mBackgroundFlags & NS_STYLE_BG_COLOR_TRANSPARENT)) {
      rv = aLists.BorderBackground()->AppendNewToTop(new (aBuilder)
          nsDisplayMathMLCharBackground(aForFrame, mRect, styleContext));
      NS_ENSURE_SUCCESS(rv, rv);
    }

    //else
    //  our container frame will take care of painting its background
    //  nsCSSRendering::PaintBackground(aPresContext, aRenderingContext, aForFrame,
    //                                  aDirtyRect, rect, *border, *padding, PR_TRUE);

#if defined(NS_DEBUG) && defined(SHOW_BOUNDING_BOX)
    // for visual debug
    rv = aLists.BorderBackground()->AppendToTop(new (aBuilder)
        nsDisplayMathMLCharDebug(aForFrame, mRect));
    NS_ENSURE_SUCCESS(rv, rv);
#endif
  }
  return aLists.Content()->AppendNewToTop(new (aBuilder)
        nsDisplayMathMLCharForeground(aForFrame, this,
                                      aSelectedRect && !aSelectedRect->IsEmpty()));
}

void
nsMathMLChar::PaintForeground(nsPresContext* aPresContext,
                              nsIRenderingContext& aRenderingContext,
                              nsPoint aPt,
                              PRBool aIsSelected)
{
  nsStyleContext* parentContext = mStyleContext->GetParent();
  nsStyleContext* styleContext = mStyleContext;

  if (NS_STRETCH_DIRECTION_UNSUPPORTED == mDirection) {
    // normal drawing if there is nothing special about this char
    // Set default context to the parent context
    styleContext = parentContext;
  }

  // Set color ...
  nscolor fgColor = styleContext->GetStyleColor()->mColor;
  if (aIsSelected) {
    // get color to use for selection from the look&feel object
    aPresContext->LookAndFeel()->
      GetColor(nsILookAndFeel::eColor_TextSelectForeground, fgColor);
  }
  aRenderingContext.SetColor(fgColor);

  nsAutoString fontName;
  nsFont theFont(styleContext->GetStyleFont()->mFont);

  if (NS_STRETCH_DIRECTION_UNSUPPORTED == mDirection) {
    // normal drawing if there is nothing special about this char ...
    // Set the default font and grab some metrics to adjust the placements ...
    PRUint32 len = PRUint32(mData.Length());
    if (1 == len) {
      SetBaseFamily(mData[0], theFont);
    }
    aRenderingContext.SetFont(theFont, nsnull);
//printf("Painting %04X like a normal char\n", mData[0]);
//aRenderingContext.SetColor(NS_RGB(255,0,0));
    aRenderingContext.DrawString(mData.get(), len, mRect.x + aPt.x,
                                 mRect.y + aPt.y + mBoundingMetrics.ascent);
  }
  else {
    // Set the stretchy font and grab some metrics to adjust the placements ...
    mGlyphTable->GetPrimaryFontName(fontName);
    SetFirstFamily(theFont, fontName);
    aRenderingContext.SetFont(theFont, nsnull);
    // if there is a glyph of appropriate size, paint that glyph
    if (mGlyph) {
//printf("Painting %04X with a glyph of appropriate size\n", mData[0]);
//aRenderingContext.SetColor(NS_RGB(0,0,255));
      mGlyphTable->DrawGlyph(aRenderingContext, theFont, mGlyph,
                             mRect.x + aPt.x,
                             mRect.y + aPt.y + mBoundingMetrics.ascent);
    }
    else { // paint by parts
      // see if this is a composite char and let children paint themselves
      if (!mParent && mSibling) { // only a "root" having child chars can enter here
        for (nsMathMLChar* child = mSibling; child; child = child->mSibling) {
//if (!mStyleContext->Equals(child->mStyleContext))
//  printf("char contexts are out of sync\n");
          child->PaintForeground(aPresContext, aRenderingContext, aPt,
                                 aIsSelected);
        }
        return; // that's all folks
       }
//aRenderingContext.SetColor(NS_RGB(0,255,0));
      nsRect r = mRect + aPt;
      if (NS_STRETCH_DIRECTION_VERTICAL == mDirection)
        PaintVertically(aPresContext, aRenderingContext, theFont, styleContext,
                        mGlyphTable, this, r);
      else if (NS_STRETCH_DIRECTION_HORIZONTAL == mDirection)
        PaintHorizontally(aPresContext, aRenderingContext, theFont, styleContext,
                          mGlyphTable, this, r);
    }
  }
}

/* =================================================================================
  And now the helper routines that actually do the job of painting the char by parts
 */

// paint a stretchy char by assembling glyphs vertically
nsresult
nsMathMLChar::PaintVertically(nsPresContext*      aPresContext,
                              nsIRenderingContext& aRenderingContext,
                              nsFont&              aFont,
                              nsStyleContext*      aStyleContext,
                              nsGlyphTable*        aGlyphTable,
                              nsMathMLChar*        aChar,
                              nsRect&              aRect)
{
  nsresult rv = NS_OK;
  nsRect clipRect;
  nscoord dx, dy;

  nscoord onePixel = aPresContext->IntScaledPixelsToTwips(1);

  // get metrics data to be re-used later
  PRInt32 i;
  nsGlyphCode ch, chdata[4];
  nsBoundingMetrics bm, bmdata[4];
  nscoord stride = 0, offset[3], start[3], end[3];
  nscoord width = aRect.width;
  nsGlyphCode glue = aGlyphTable->GlueOf(aPresContext, aChar);
  for (i = 0; i < 4; i++) {
    switch (i) {
      case 0: ch = aGlyphTable->TopOf(aPresContext, aChar);    break;
      case 1: ch = aGlyphTable->MiddleOf(aPresContext, aChar); break;
      case 2: ch = aGlyphTable->BottomOf(aPresContext, aChar); break;
      case 3: ch = glue;                                       break;
    }
    // empty slots are filled with the glue if it is not null
    if (!ch) ch = glue;
    if (!ch) {
      bm.Clear();  // glue is null, set bounding metrics to 0
    }
    else {
      rv = aGlyphTable->GetBoundingMetrics(aRenderingContext, aFont, ch, bm);
      if (NS_FAILED(rv)) {
        NS_WARNING("GetBoundingMetrics failed");
        return rv;
      }
      if (width < bm.rightBearing) width =  bm.rightBearing;
    }
    chdata[i] = ch;
    bmdata[i] = bm;
  }
  dx = aRect.x;
  for (i = 0; i < 3; i++) {
    ch = chdata[i];
    bm = bmdata[i];
    if (0 == i) { // top
      dy = aRect.y + bm.ascent;
    }
    else if (1 == i) { // middle
      dy = aRect.y + bm.ascent + (aRect.height - (bm.ascent + bm.descent))/2;
    }
    else { // bottom
      dy = aRect.y + aRect.height - bm.descent;
    }
    // abcissa passed to DrawString
    offset[i] = dy;
    // *exact* abcissa where the *top-most* pixel of the glyph is painted
    start[i] = dy - bm.ascent;
    // *exact* abcissa where the *bottom-most* pixel of the glyph is painted
    end[i] = dy + bm.descent; // end = start + height
  }

  /////////////////////////////////////
  // draw top, middle, bottom
  for (i = 0; i < 3; i++) {
    ch = chdata[i];
    // glue can be null, and other parts could have been set to glue
    if (ch) {
#ifdef SHOW_BORDERS
      // bounding box of the part
      aRenderingContext.SetColor(NS_RGB(0,0,0));
      aRenderingContext.DrawRect(nsRect(dx,start[i],width+30*(i+1),end[i]-start[i]));
#endif
      dy = offset[i];
      if (0 == i) { // top
        clipRect.SetRect(dx, aRect.y, width, aRect.height);
      }
      else if (1 == i) { // middle
        clipRect.SetRect(dx, end[0], width, start[2]-end[0]);
      }
      else { // bottom
        clipRect.SetRect(dx, start[2], width, end[2]-start[2]);
      }
      if (!clipRect.IsEmpty()) {
        clipRect.Inflate(onePixel, onePixel);
        aGlyphTable->DrawGlyph(aRenderingContext, aFont, ch, dx, dy, &clipRect);
      }
    }
  }

  ///////////////
  // fill the gap between top and middle, and between middle and bottom.
  if (!glue) { // null glue : draw a rule
    // figure out the dimensions of the rule to be drawn :
    // set lbearing to rightmost lbearing among the two current successive parts.
    // set rbearing to leftmost rbearing among the two current successive parts.
    // this not only satisfies the convention used for over/underbraces
    // in TeX, but also takes care of broken fonts like the stretchy integral
    // in Symbol for small font sizes in unix.
    nscoord lbearing, rbearing;
    PRInt32 first = 0, last = 2;
    if (chdata[1]) { // middle part exists
      last = 1;
    }
    while (last <= 2) {
      if (chdata[last]) {
        lbearing = bmdata[last].leftBearing;
        rbearing = bmdata[last].rightBearing;
        if (chdata[first]) {
          if (lbearing < bmdata[first].leftBearing)
            lbearing = bmdata[first].leftBearing;
          if (rbearing > bmdata[first].rightBearing)
            rbearing = bmdata[first].rightBearing;
        }
      }
      else if (chdata[first]) {
        lbearing = bmdata[first].leftBearing;
        rbearing = bmdata[first].rightBearing;
      }
      else {
        NS_ERROR("Cannot stretch - All parts missing");
        return NS_ERROR_UNEXPECTED;
      }
      // paint the rule between the parts
      nsRect rule(aRect.x + lbearing, end[first] - onePixel,
                  rbearing - lbearing, start[last] - end[first] + 2*onePixel);
      if (!rule.IsEmpty())
        aRenderingContext.FillRect(rule);
      first = last;
      last++;
    }
  }
  else { // glue is present
    nscoord overlap;
    nsCOMPtr<nsIFontMetrics> fm;
    aRenderingContext.GetFontMetrics(*getter_AddRefs(fm));
    nsMathMLFrame::GetRuleThickness(fm, overlap);
    overlap = 2 * PR_MAX(overlap, onePixel);
    while (overlap > onePixel && bmdata[3].ascent + bmdata[3].descent <= 2*overlap)
      overlap -= onePixel;

    // to protect against gaps, pretend the glue is smaller than 
    // it says to allow a small overlap when adjoining it
    bmdata[3].ascent -= overlap;
    bmdata[3].descent -= overlap;

    for (i = 0; i < 2; i++) {
      PRInt32 count = 0;
      dy = offset[i];
      clipRect.SetRect(dx, end[i], width, start[i+1]-end[i]);
      clipRect.Inflate(overlap, overlap);
#ifdef SHOW_BORDERS
      // exact area to fill
      aRenderingContext.SetColor(NS_RGB(255,0,0));
      aRenderingContext.DrawRect(clipRect);
#endif
      aRenderingContext.PushState();
      aRenderingContext.SetClipRect(clipRect, nsClipCombine_kIntersect);
      bm = bmdata[i];
      while (dy + bm.descent < start[i+1]) {
        if (count++ < 2) {
          stride = bm.descent;
          bm = bmdata[3]; // glue
          stride += bm.ascent;
        }
        // defensive code against odd things such as a smallish TextZoom...
        NS_ASSERTION(1000 != count, "something is probably wrong somewhere");
        if (stride < onePixel || 1000 == count) return NS_ERROR_UNEXPECTED;
        dy += stride;
        aGlyphTable->DrawGlyph(aRenderingContext, aFont, glue, dx, dy);
      }
      aRenderingContext.PopState();
#ifdef SHOW_BORDERS
      // last glyph that may cross past its boundary and collide with the next
      nscoord height = bm.ascent + bm.descent;
      aRenderingContext.SetColor(NS_RGB(0,255,0));
      aRenderingContext.DrawRect(nsRect(dx, dy-bm.ascent, width, height));
#endif
    }
  }
  return NS_OK;
}

// paint a stretchy char by assembling glyphs horizontally
nsresult
nsMathMLChar::PaintHorizontally(nsPresContext*      aPresContext,
                                nsIRenderingContext& aRenderingContext,
                                nsFont&              aFont,
                                nsStyleContext*      aStyleContext,
                                nsGlyphTable*        aGlyphTable,
                                nsMathMLChar*        aChar,
                                nsRect&              aRect)
{
  nsresult rv = NS_OK;
  nsRect clipRect;
  nscoord dx, dy;

  nscoord onePixel = aPresContext->IntScaledPixelsToTwips(1);

  // get metrics data to be re-used later
  PRInt32 i;
  nsGlyphCode ch, chdata[4];
  nsBoundingMetrics bm, bmdata[4];
  nscoord stride = 0, offset[3], start[3], end[3];
  dy = aRect.y;
  nsGlyphCode glue = aGlyphTable->GlueOf(aPresContext, aChar);
  for (i = 0; i < 4; i++) {
    switch (i) {
      case 0: ch = aGlyphTable->LeftOf(aPresContext, aChar);   break;
      case 1: ch = aGlyphTable->MiddleOf(aPresContext, aChar); break;
      case 2: ch = aGlyphTable->RightOf(aPresContext, aChar);  break;
      case 3: ch = glue;                                       break;
    }
    // empty slots are filled with the glue if it is not null
    if (!ch) ch = glue;
    if (!ch) {
      bm.Clear();  // glue is null, set bounding metrics to 0
    }
    else {
      rv = aGlyphTable->GetBoundingMetrics(aRenderingContext, aFont, ch, bm);
      if (NS_FAILED(rv)) {
        NS_WARNING("GetBoundingMetrics failed");
        return rv;
      }
      if (dy < aRect.y + bm.ascent) {
        dy = aRect.y + bm.ascent;
      }
    }
    chdata[i] = ch;
    bmdata[i] = bm;
  }
  for (i = 0; i < 3; i++) {
    ch = chdata[i];
    bm = bmdata[i];
    if (0 == i) { // left
      dx = aRect.x - bm.leftBearing;
    }
    else if (1 == i) { // middle
      dx = aRect.x + (aRect.width - bm.width)/2;
    }
    else { // right
      dx = aRect.x + aRect.width - bm.rightBearing;
    }
    // abcissa that DrawString used
    offset[i] = dx;
    // *exact* abcissa where the *left-most* pixel of the glyph is painted
    start[i] = dx + bm.leftBearing;
    // *exact* abcissa where the *right-most* pixel of the glyph is painted
    end[i] = dx + bm.rightBearing; // note: end = start + width
  }

  ///////////////////////////
  // draw left, middle, right
  for (i = 0; i < 3; i++) {
    ch = chdata[i];
    // glue can be null, and other parts could have been set to glue
    if (ch) {
#ifdef SHOW_BORDERS
      aRenderingContext.SetColor(NS_RGB(255,0,0));
      aRenderingContext.DrawRect(nsRect(start[i], dy - bmdata[i].ascent,
                                 end[i] - start[i], bmdata[i].ascent + bmdata[i].descent));
#endif
      dx = offset[i];
      if (0 == i) { // left
        clipRect.SetRect(dx, aRect.y, aRect.width, aRect.height);
      }
      else if (1 == i) { // middle
        clipRect.SetRect(end[0], aRect.y, start[2]-end[0], aRect.height);
      }
      else { // right
        clipRect.SetRect(start[2], aRect.y, end[2]-start[2], aRect.height);
      }
      if (!clipRect.IsEmpty()) {
        clipRect.Inflate(onePixel, onePixel);
        aGlyphTable->DrawGlyph(aRenderingContext, aFont, ch, dx, dy, &clipRect);
      }
    }
  }

  ////////////////
  // fill the gap between left and middle, and between middle and right.
  if (!glue) { // null glue : draw a rule
    // figure out the dimensions of the rule to be drawn :
    // set ascent to lowest ascent among the two current successive parts.
    // set descent to highest descent among the two current successive parts.
    // this satisfies the convention used for over/underbraces, and helps
    // fix broken fonts.
    nscoord ascent, descent;
    PRInt32 first = 0, last = 2;
    if (chdata[1]) { // middle part exists
      last = 1;
    }
    while (last <= 2) {
      if (chdata[last]) {
        ascent = bmdata[last].ascent;
        descent = bmdata[last].descent;
        if (chdata[first]) {
          if (ascent > bmdata[first].ascent)
            ascent = bmdata[first].ascent;
          if (descent > bmdata[first].descent)
            descent = bmdata[first].descent;
        }
      }
      else if (chdata[first]) {
        ascent = bmdata[first].ascent;
        descent = bmdata[first].descent;
      }
      else {
        NS_ERROR("Cannot stretch - All parts missing");
        return NS_ERROR_UNEXPECTED;
      }
      // paint the rule between the parts
      nsRect rule(end[first] - onePixel, dy - ascent,
                  start[last] - end[first] + 2*onePixel, ascent + descent);
      if (!rule.IsEmpty())
        aRenderingContext.FillRect(rule);
      first = last;
      last++;
    }
  }
  else { // glue is present
    nscoord overlap;
    nsCOMPtr<nsIFontMetrics> fm;
    aRenderingContext.GetFontMetrics(*getter_AddRefs(fm));
    nsMathMLFrame::GetRuleThickness(fm, overlap);
    overlap = 2 * PR_MAX(overlap, onePixel);
    while (overlap > onePixel && bmdata[3].rightBearing - bmdata[3].leftBearing <= 2*overlap)
      overlap -= onePixel;

    // to protect against gaps, pretend the glue is smaller than 
    // it says to allow a small overlap when adjoining it
    bmdata[3].leftBearing += overlap;
    bmdata[3].rightBearing -= overlap;

    for (i = 0; i < 2; i++) {
      PRInt32 count = 0;
      dx = offset[i];
      clipRect.SetRect(end[i], aRect.y, start[i+1]-end[i], aRect.height);
      clipRect.Inflate(overlap, overlap);
#ifdef SHOW_BORDERS
      // rectangles in-between that are to be filled
      aRenderingContext.SetColor(NS_RGB(255,0,0));
      aRenderingContext.DrawRect(clipRect);
#endif
      aRenderingContext.PushState();
      aRenderingContext.SetClipRect(clipRect, nsClipCombine_kIntersect);
      bm = bmdata[i];
      while (dx + bm.rightBearing < start[i+1]) {
        if (count++ < 2) {
          stride = bm.rightBearing;
          bm = bmdata[3]; // glue
          stride -= bm.leftBearing;
        }
        // defensive code against odd things such as a smallish TextZoom...
        NS_ASSERTION(1000 != count, "something is probably wrong somewhere");
        if (stride < onePixel || 1000 == count) return NS_ERROR_UNEXPECTED;
        dx += stride;
        aGlyphTable->DrawGlyph(aRenderingContext, aFont, glue, dx, dy);
      }
      aRenderingContext.PopState();
#ifdef SHOW_BORDERS
      // last glyph that may cross past its boundary and collide with the next
      nscoord width = bm.rightBearing - bm.leftBearing;
      aRenderingContext.SetColor(NS_RGB(0,255,0));
      aRenderingContext.DrawRect(nsRect(dx + bm.leftBearing, aRect.y, width, aRect.height));
#endif
    }
  }
  return NS_OK;
}
