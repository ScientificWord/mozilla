/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * ex: set tabstop=8 softtabstop=2 shiftwidth=2 expandtab:
 *
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef nsFreeType_h__
#define nsFreeType_h__

#include "gfx-config.h"
#if (defined(MOZ_ENABLE_FREETYPE2))

#include "nspr.h"
#include "nsHashtable.h"
#include "nsICharsetConverterManager.h"
#include "nsIFontCatalogService.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_IMAGE_H
#include FT_TRUETYPE_TABLES_H
#include "nsIFreeType2.h"

typedef struct FT_FaceRec_*  FT_Face;

typedef struct {
  const char   *mFontFileName;
  time_t        mMTime;
  PRUint32      mFlags;
  const char   *mFontType;
  int           mFaceIndex;
  int           mNumFaces;
  const char   *mFamilyName;
  const char   *mStyleName;
  FT_UShort     mWeight;
  FT_UShort     mWidth;
  int           mNumGlyphs;
  int           mNumUsableGlyphs;
  FT_Long       mFaceFlags;
  FT_Long       mStyleFlags;
  FT_Long       mCodePageRange1;
  FT_Long       mCodePageRange2;
  char          mVendorID[5];
  const char   *mFoundryName;
  int           mNumEmbeddedBitmaps;
  int          *mEmbeddedBitmapHeights;
  PRUint16     *mCCMap;       // compressed char map
} nsFontCatalogEntry;

#define FCE_FLAGS_ISVALID    0x01
#define FCE_FLAGS_UNICODE    0x02
#define FCE_FLAGS_SYMBOL     0x04
#define FCE_FLAGS_SURROGATE  0x08
#define FREE_IF(x) if(x) free((void*)x)

typedef struct {
  const char             *mConverterName;
  PRUint8                 mCmapPlatformID;
  PRUint8                 mCmapEncoding;
  nsIUnicodeEncoder*      mConverter;
} nsTTFontEncoderInfo;

typedef struct nsTTFontFamilyEncoderInfo {
  const char             *mFamilyName;
  nsTTFontEncoderInfo    *mEncodingInfo;
} nsTTFontFamilyEncoderInfo;

typedef struct {
  unsigned long bit;
  const char *charsetName;
} nsulCodePageRangeCharSetName;

//
// the FreeType2 function type declarations
//
typedef FT_Error (*FT_Done_Face_t)(FT_Face);
typedef FT_Error (*FT_Done_FreeType_t)(FT_Library);
typedef FT_Error (*FT_Done_Glyph_t)(FT_Glyph);
typedef FT_Error (*FT_Get_Char_Index_t)(FT_Face, FT_ULong);
typedef FT_Error (*FT_Get_Glyph_t)(FT_GlyphSlot, FT_Glyph*);
typedef void*    (*FT_Get_Sfnt_Table_t)(FT_Face, FT_Sfnt_Tag);
typedef FT_Error (*FT_Glyph_Get_CBox_t)(FT_Glyph, FT_UInt, FT_BBox*);
typedef FT_Error (*FT_Init_FreeType_t)(FT_Library*);
typedef FT_Error (*FT_Load_Glyph_t)(FT_Face, FT_UInt, FT_Int);
typedef FT_Error (*FT_Outline_Decompose_t)
                      (FT_Outline*, const FT_Outline_Funcs*, void*);
typedef FT_Error (*FT_New_Face_t)(FT_Library, const char*, FT_Long, FT_Face*);
typedef FT_Error (*FT_Set_Charmap_t)(FT_Face face, FT_CharMap  charmap);
typedef FT_Error (*FTC_Image_Cache_Lookup_t)
                      (FTC_Image_Cache, FTC_Image_Desc*, FT_UInt, FT_Glyph*);
typedef FT_Error (*FTC_Manager_Lookup_Size_t)
                      (FTC_Manager, FTC_Font, FT_Face*, FT_Size*);
typedef FT_Error (*FTC_Manager_Done_t)(FTC_Manager);
typedef FT_Error (*FTC_Manager_New_t)(FT_Library, FT_UInt, FT_UInt, FT_ULong,
                       FTC_Face_Requester, FT_Pointer, FTC_Manager*);
typedef FT_Error (*FTC_Image_Cache_New_t)(FTC_Manager, FTC_Image_Cache*);
// #ifdef MOZ_SVG
typedef FT_Error (*FT_Glyph_Transform_t)(FT_Glyph, FT_Matrix*, FT_Vector*);
typedef FT_Error (*FT_Get_Kerning_t)
                      (FT_Face, FT_UInt, FT_UInt, FT_UInt, FT_Vector*);
typedef FT_Error (*FT_Glyph_Copy_t)(FT_Glyph, FT_Glyph*);
typedef FT_Error (*FT_Glyph_To_Bitmap_t)
                      (FT_Glyph*, FT_Render_Mode, FT_Vector*, FT_Bool);
// #endif

typedef FT_ULong (*FT_Get_First_Char_t)(FT_Face, FT_UInt*);
typedef FT_ULong (*FT_Get_Next_Char_t)(FT_Face, FT_ULong, FT_UInt*);

class nsFreeTypeFace;

nsFreeTypeFace * nsFreeTypeGetFaceID(nsFontCatalogEntry *aFce);

typedef struct {
  const char *FuncName;
  long  FuncOffset;
  const PRBool Required;
} FtFuncList;

// class nsFreeType class definition
class nsFreeType2 : nsIFreeType2 {
  NS_DECL_ISUPPORTS

public:
  void FreeGlobals();
  nsresult Init();
  virtual ~nsFreeType2();

  NS_DECL_NSIFREETYPE2

  // these belong in nsFT2FontCatalog
  static PRUint16*   GetCCMap(nsFontCatalogEntry *aFce);
  static const char* GetRange1CharSetName(unsigned long aBit);
  static const char* GetRange2CharSetName(unsigned long aBit);
  static nsTTFontFamilyEncoderInfo* GetCustomEncoderInfo(const char *);

protected:
  // run time loaded function pointers
  FT_Done_Face_t            nsFT_Done_Face;
  FT_Done_FreeType_t        nsFT_Done_FreeType;
  FT_Done_Glyph_t           nsFT_Done_Glyph;
  FT_Get_Char_Index_t       nsFT_Get_Char_Index;
  FT_Get_Glyph_t            nsFT_Get_Glyph;
  FT_Get_Sfnt_Table_t       nsFT_Get_Sfnt_Table;
  FT_Glyph_Get_CBox_t       nsFT_Glyph_Get_CBox;
  FT_Init_FreeType_t        nsFT_Init_FreeType;
  FT_Load_Glyph_t           nsFT_Load_Glyph;
  FT_New_Face_t             nsFT_New_Face;
  FT_Outline_Decompose_t    nsFT_Outline_Decompose;
  FT_Set_Charmap_t          nsFT_Set_Charmap;
  FTC_Image_Cache_Lookup_t  nsFTC_Image_Cache_Lookup;
  FTC_Manager_Lookup_Size_t nsFTC_Manager_Lookup_Size;
  FTC_Manager_Done_t        nsFTC_Manager_Done;
  FTC_Manager_New_t         nsFTC_Manager_New;
  FTC_Image_Cache_New_t     nsFTC_Image_Cache_New;
// #ifdef MOZ_SVG
  FT_Glyph_Transform_t      nsFT_Glyph_Transform;
  FT_Get_Kerning_t          nsFT_Get_Kerning;
  FT_Glyph_Copy_t           nsFT_Glyph_Copy;
  FT_Glyph_To_Bitmap_t      nsFT_Glyph_To_Bitmap;
// #endif
  FT_Get_First_Char_t       nsFT_Get_First_Char;
  FT_Get_Next_Char_t        nsFT_Get_Next_Char;

  // this array needs to be big enough to hold all the function pointers
  // plus one extra for the null at the end
// #ifdef MOZ_SVG
  static FtFuncList FtFuncs[24];
// #else
//  static FtFuncList FtFuncs[20];
// #endif
  
protected:
  PRBool mEnableFreeType2;
  char*  mFreeType2SharedLibraryName;

public:
  // these belong in the nsFontFreeType code
  static PRBool  gFreeType2Autohinted;
  static PRBool  gFreeType2Unhinted;
  static PRUint8 gAATTDarkTextMinValue;
  static double  gAATTDarkTextGain;
  static PRInt32 gAntiAliasMinimum;
  static PRInt32 gEmbeddedBitmapMaximumHeight;
  static PRBool  gHasExtFunc;

protected:
  void ClearGlobals();
  void ClearFunctions();
  PRBool InitLibrary();
  PRBool LoadSharedLib();
  void UnloadSharedLib();

  // this belongs in nsFT2FontCatalog
  static nsICharsetConverterManager* GetCharSetManager();

  PRLibrary      *mSharedLib;
  FT_Library      mFreeTypeLibrary;
  FTC_Manager     mFTCacheManager;
  FTC_Image_Cache mImageCache;

  static nsHashtable   *sFontFamilies;
  static nsHashtable   *sRange1CharSetNames;
  static nsHashtable   *sRange2CharSetNames;
  static nsICharsetConverterManager* sCharSetManager;
};

/* this simple record is used to model a given `installed' face */
class nsFreeTypeFace : public nsITrueTypeFontCatalogEntry {
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSITRUETYPEFONTCATALOGENTRY
  
  nsFreeTypeFace();
  virtual ~nsFreeTypeFace();
  virtual nsresult Init(nsFontCatalogEntry *aFce);
  /* additional members */
  NS_IMETHODIMP GetFontCatalogType(PRUint16 *aFontCatalogType);

  static PRBool FreeFace(nsHashKey* aKey, void* aData, void* aClosure);
  const char *GetFilename()
                  { return mFce->mFontFileName; }
  int *GetEmbeddedBitmapHeights()
                  { return mFce->mEmbeddedBitmapHeights; } ;
  int GetFaceIndex()
                  { return mFce->mFaceIndex; }
  int GetNumEmbeddedBitmaps()
                  { return mFce->mNumEmbeddedBitmaps; } ;
  PRUint16 *GetCCMap();
  nsFontCatalogEntry* GetFce() { return mFce; };

  PRBool mHasExtendFuncs;

protected:
  nsFontCatalogEntry *mFce;
  PRUint16           *mCCMap;

};

#endif /* MOZ_ENABLE_FREETYPE2 */

/*
 * Defines for the TrueType codepage bits.
 * Used as a hint for the languages supported in a TrueType font.
 */

/*
 * ulCodePageRange1
 */
#define TT_OS2_CPR1_LATIN1       (0x00000001) /* Latin 1                     */
#define TT_OS2_CPR1_LATIN2       (0x00000002) /* Latin 2: Eastern Europe     */
#define TT_OS2_CPR1_CYRILLIC     (0x00000004) /* Cyrillic                    */
#define TT_OS2_CPR1_GREEK        (0x00000008) /* Greek                       */
#define TT_OS2_CPR1_TURKISH      (0x00000010) /* Turkish                     */
#define TT_OS2_CPR1_HEBREW       (0x00000020) /* Hebrew                      */
#define TT_OS2_CPR1_ARABIC       (0x00000040) /* Arabic                      */
#define TT_OS2_CPR1_BALTIC       (0x00000080) /* Windows Baltic              */
#define TT_OS2_CPR1_VIETNAMESE   (0x00000100) /* Vietnamese                  */
                                 /* 9-15     Reserved for Alternate ANSI     */
#define TT_OS2_CPR1_THAI         (0x00010000) /* Thai                        */
#define TT_OS2_CPR1_JAPANESE     (0x00020000) /* JIS/Japan                   */
#define TT_OS2_CPR1_CHINESE_SIMP (0x00040000) /* Chinese: Simplified         */
#define TT_OS2_CPR1_KO_WANSUNG   (0x00080000) /* Korean Wansung              */
#define TT_OS2_CPR1_CHINESE_TRAD (0x00100000) /* Chinese: Traditional        */
#define TT_OS2_CPR1_KO_JOHAB     (0x00200000) /* Korean Johab                */
                                 /* 22-28    Reserved for Alternate ANSI&OEM */
#define TT_OS2_CPR1_MAC_ROMAN    (0x20000000) /* Mac (US Roman)              */
#define TT_OS2_CPR1_OEM          (0x40000000) /* OEM Character Set           */
#define TT_OS2_CPR1_SYMBOL       (0x80000000) /* Symbol Character Set        */

/*
 * ulCodePageRange2
 */                              /* 32-47    Reserved for OEM                */
#define TT_OS2_CPR2_GREEK        (0x00010000) /* IBM Greek                   */
#define TT_OS2_CPR2_RUSSIAN      (0x00020000) /* MS-DOS Russian              */
#define TT_OS2_CPR2_NORDIC       (0x00040000) /* MS-DOS Nordic               */
#define TT_OS2_CPR2_ARABIC       (0x00080000) /* Arabic                      */
#define TT_OS2_CPR2_CA_FRENCH    (0x00100000) /* MS-DOS Canadian French      */
#define TT_OS2_CPR2_HEBREW       (0x00200000) /* Hebrew                      */
#define TT_OS2_CPR2_ICELANDIC    (0x00400000) /* MS-DOS Icelandic            */
#define TT_OS2_CPR2_PORTUGESE    (0x00800000) /* MS-DOS Portuguese           */
#define TT_OS2_CPR2_TURKISH      (0x01000000) /* IBM Turkish                 */
#define TT_OS2_CPR2_CYRILLIC     (0x02000000)/*IBM Cyrillic; primarily Russian*/
#define TT_OS2_CPR2_LATIN2       (0x04000000) /* Latin 2                     */
#define TT_OS2_CPR2_BALTIC       (0x08000000) /* MS-DOS Baltic               */
#define TT_OS2_CPR2_GREEK_437G   (0x10000000) /* Greek; former 437 G         */
#define TT_OS2_CPR2_ARABIC_708   (0x20000000) /* Arabic; ASMO 708            */
#define TT_OS2_CPR2_WE_LATIN1    (0x40000000) /* WE/Latin 1                  */
#define TT_OS2_CPR2_US           (0x80000000) /* US                          */

#endif
