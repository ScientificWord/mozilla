/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
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


#ifndef nsAFMObject_h__
#define nsAFMObject_h__ 


#include "nsIFontMetrics.h"
#include "nsFont.h"
#include "nsString.h"
#include "nsUnitConversion.h"
#include "nsIDeviceContext.h"
#include "nsCRT.h"

class nsDeviceContextPS;


// AFM Key Words
typedef enum
{
  kComment,

  // File structure.
  kStartFontMetrics,
  kEndFontMetrics,
  kStartCompFontMetrics,
  kEndCompFontMetrics,
  kStartDescendent,
  kEndDescendent,
  kStartMasterFontMetrics,
  kEndMasterFontMetrics,

  // Control information.
  kMetricsSets,
  kDescendents,
  kMasters,
  kAxes,

  // Global font information. 
  kFontName,
  kFullName,
  kFamilyName,
  kWeight,
  kFontBBox,
  kVersion,
  kNotice,
  kEncodingScheme,
  kMappingScheme,
  kEscChar,
  kCharacterSet,
  kCharacters,
  kIsBaseFont,
  kVVector,
  kIsFixedV,
  kCapHeight,
  kXHeight,
  kAscender,
  kDescender,
  kWeightVector,
  kBlendDesignPositions,
  kBlendDesignMap,
  kBlendAxisTypes,


  // Writing direction information. 
  kStartDirection,
  kEndDirection,
  kUnderlinePosition,
  kUnderlineThickness,
  kItalicAngle,
  kCharWidth,
  kIsFixedPitch,

  // Individual character metrics. 
  kStartCharMetrics,
  kEndCharMetrics,
  kC,
  kCH,
  kWX,
  kW0X,
  kW1X,
  kWY,
  kW0Y,
  kW1Y,
  kW,
  kW0,
  kW1,
  kVV,
  kN,
  kB,
  kL,

  // Kerning data.
  kStartKernData,
  kEndKernData,
  kStartTrackKern,
  kEndTrackKern,
  kTrackKern,
  kStartKernPairs,
  kEndKernPairs,
  kKP,
  kKPH,
  kKPX,
  kKPY,

  // Composite character data.
  kStartComposites,
  kEndComposites,
  kCC,
  kPCC,

  // Axis information.
  kStartAxis,
  kEndAxis,
  kAxisType,
  kAxisLabel,


  // Master Design Information 
  kStartMaster,
  kEndMaster

} AFMKey;



// Single character infor for AFM character. 
struct AFM_Single_Char_Metrics
{

  PRInt32   mCharacter_Code;      // default charcode (-1 if not encoded) 
  float     mW0x;                 // character width x in writing direction 0, 
  float     mW0y;                 // character width y in writing direction 0
  float     mW1x;                 // character width x in writing direction 1
  float     mW1y;                 // character width y in writing direction 1
  //char      *mName;             // character name ,  not using currently
  //double    mVv_x;              // local VVector x ,  not using currently
  //double    mVv_y;              // local VVector y ,  not using currently

  // character bounding box. 
  float     mLlx;
  float     mLly;
  float     mUrx;
  float     mUry;

  //double num_ligatures;      

  //AFMLigature *ligatures;
};


typedef struct AFM_Single_Char_Metrics  AFMscm;


// Font information which we get from AFM files, this is needed for the PS output
struct fontInformation
{
  double      mFontVersion;
  const char *mFontName;
  const char *mFullName;
  const char *mFamilyName;
  const char *mWeight;
  float       mFontBBox_llx;
  float       mFontBBox_lly;
  float       mFontBBox_urx;
  float       mFontBBox_ury;
  const char *mVersion;
  char       *mNotice;         // DO NOT MAKE "const" (125341)
  const char *mEncodingScheme;
  PRInt32     mMappingScheme;
  PRInt32     mEscChar;
  const char *mCharacterSet;
  PRInt32     mCharacters;
  PRBool      mIsBaseFont;
  float       mVVector_0;
  float       mVVector_1;
  PRBool      mIsFixedV;
  float       mCapHeight;
  float       mXHeight;
  float       mAscender;
  float       mDescender;
  float       mUnderlinePosition;
  float       mUnderlineThickness;

  PRInt32     mNumCharacters;
  AFMscm     *mAFMCharMetrics;
};


typedef struct fontInformation AFMFontInformation;


class nsAFMObject 
{
public:

  /** ---------------------------------------------------
   * Construct and AFMObject
   * @update 2/26/99 dwc
   */
  nsAFMObject();

  /** ---------------------------------------------------
   * delete an AFMObject
   * @update 2/26/99 dwc
   */
 virtual ~nsAFMObject();

  /** ---------------------------------------------------
   *  Initialize an AFM object.
   *      @update 9/30/2003 kherron
   *      @param aFontHeight  Font height in twips
   *      @return VOID
   */
  void    Init(nscoord  aFontHeight);


  /** ---------------------------------------------------
   * Read in and parse and AFM file
   * @update 2/26/99 dwc
   * @param aFontName -- The name of the font we want to read in
   * @return VOID
   */
  PRBool AFM_ReadFile(const nsFont &aFontName);

  /** ---------------------------------------------------
   * Check to see if this font is one of our basic fonts, if so create an AFMObject from it
   * @update 2/26/99 dwc
   * @param aFont -- The font to be constructed (family name, weight, style)
   * @param aPrimaryOnly -- if true, only looking for the first font name in aFontName
   * @return -- the font index into our table if a native font was found, -1 otherwise
   */
  PRInt16 CheckBasicFonts(const nsFont &aFont,PRBool aPrimaryOnly=PR_FALSE);

  /** ---------------------------------------------------
   * Create a substitute Font
   * @update 2/26/99 dwc
   * @param aFont -- The font to create a substitute for (family name, weight, style)
   * @return -- the font index into our table if a native font was found, -1 otherwise
   */
  PRInt16 CreateSubstituteFont(const nsFont &aFont);

  /** ---------------------------------------------------
   * Set the font size, which is used to calculate the distances for the font
   * @update 9/30/2003 kherron
   * @param aFontHeight - The height of the font, in twips.
   * @return VOID
   */
  void    SetFontSize(nscoord  aFontHeight) { mFontHeight = aFontHeight; }


  /** ---------------------------------------------------
   * Calculate the width of a unicharacter string
   * @update 2/26/99 dwc
   * @param aString - The unicharacter string to get the width for
   * @param aWidth - Where the width of the string will be put.
   * @param aLenth - The length of the passed in string
   * @return VOID
   */
  void    GetStringWidth(const PRUnichar *aString,nscoord& aWidth,nscoord aLength);

  /** ---------------------------------------------------
   * Calculate the width of a C style string
   * @update 2/26/99 dwc
   * @param aString - The C string to get the width for
   * @param aWidth - Where the width of the string will be put.
   * @param aLenth - The length of the passed in string
   */
  void    GetStringWidth(const char *aString,nscoord& aWidth,nscoord aLength);

  /** ---------------------------------------------------
   * Write out the AFMFontInformation Header from the currently parsed AFM file, this will write a ASCII file
   * so you can use it as a head if need be.  
   * @update 3/09/99 dwc
   * @param aOutFile -- File to write to
   * @return VOID
   */
  void    WriteFontHeaderInformation(FILE *aOutFile);

  /** ---------------------------------------------------
   * Write out the AFMFontInformation character data from the currently parsed AFM file, this will write a ASCII file
   * so you can use it as a head if need be.  
   * @update 3/09/99 dwc
   * @param aOutFile -- File to write to
   * @return VOID
   */
  void    WriteFontCharInformation(FILE *aOutFile);
protected:

  /** ---------------------------------------------------
   * Get a Keyword in the AFM file being parsed
   * @update 2/26/99 dwc
   * @return VOID
   */
  void    GetKey(AFMKey *aTheKey);

  /** ---------------------------------------------------
   * Get a token from the AFM file
   * @update 2/26/99 dwc
   * @return -- The found token
   */
  PRInt32 GetToken(void);

  /** ---------------------------------------------------
   * For a given token, find the keyword it represents
   * @update 2/26/99 dwc
   * @return -- The key found
   */
  PRInt32 MatchKey(char *aKey);

  /** ---------------------------------------------------
   * Get a line from the currently parsed file
   * @update 2/26/99 dwc
   * @return -- The current line
   */
  PRInt32 GetLine(void);

  /** ---------------------------------------------------
   * Get a string from the currently parsed file
   * @update 2/26/99 dwc
   * @return -- the current string
   */
  char*   GetAFMString (void);

  /** ---------------------------------------------------
   * Get a word from the currently parsed file
   * @update 2/26/99 dwc
   * @return -- a string with the name
   */
  char*   GetAFMName (void); 

  /** ---------------------------------------------------
   * Get an integer from the currently parsed file
   * @update 2/26/99 dwc
   * @return -- the current integer
   */
  void    GetAFMInt (PRInt32 *aInt) {GetToken();*aInt = atoi (mToken);}

  /** ---------------------------------------------------
   * Get a floating point number from the currently parsed file
   * @update 2/26/99 dwc
   * @return -- the current floating point
   */
  void    GetAFMNumber (double *aFloat){GetToken();*aFloat = atof (mToken);}
  void    GetAFMNumber (float  *aFloat){GetToken();*aFloat = atof (mToken);}

  /** ---------------------------------------------------
   * Get a boolean from the currently parsed file
   * @update 2/26/99 dwc
   * @param -- The current boolean found is passed back
   */
  void    GetAFMBool (PRBool *aBool);

  /** ---------------------------------------------------
   * Read in the AFMFontInformation from the currently parsed AFM file
   * @update 2/26/99 dwc
   * @param aFontInfo -- The header structure to read the caracter info from
   * @param aNumCharacters -- The number of characters to look for
   */
  void    ReadCharMetrics (AFMFontInformation *aFontInfo,PRInt32 aNumCharacters);


public:
  AFMFontInformation  *mPSFontInfo;


protected:
  FILE                *mAFMFile;          // this is the AFM file we are parsing.
  char                mToken[256];        // Temporary storage for reading and parsing the file
  nscoord             mFontHeight;        // font height in twips that we are supporting.
                                          // XXX  This should be passed into the GetStringWidth
                                          // so we can have one font family support many sizes

};

#define NUM_KEYS (sizeof (keynames) / sizeof (struct keyname_st) - 1)

/** ---------------------------------------------------
 *  A static structure initialized with the default fonts for postscript
 *  @update 3/12/99 dwc
 *  @member mFontName -- string with the substitute font name
 *  @member mFontInfo -- AFM font information header created with the AFMGen program
 *  @member mCharInfo -- Character information created with the AFMGen program
 *  @member mIndex -- This member field is used when substituting fonts
 */
struct AFM_SubstituteFonts
{
  const char*         mPSName;
  const char*         mFamily;
  PRUint16            mWeight;
  PRUint8             mStyle;
  const AFMFontInformation* mFontInfo;
  const AFMscm*             mCharInfo;
  PRInt32             mIndex;
};

typedef struct AFM_SubstituteFonts  DefFonts;

extern DefFonts gSubstituteFonts[];

// number of supported default fonts
#define NUM_AFM_FONTS 13

#endif /* !nsAFMObject_h__ */

