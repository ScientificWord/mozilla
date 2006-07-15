/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ex: set tabstop=8 softtabstop=2 shiftwidth=2 expandtab: */
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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brian Stell <bstell@netscape.com>
 *   Louie Zhao  <louie.zhao@sun.com>
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
#ifndef nsFontFreeType_h__
#define nsFontFreeType_h__

#include "gfx-config.h"
#include "nsFontMetricsGTK.h"
#include "nsFreeType.h"

#if (!defined(MOZ_ENABLE_FREETYPE2))
class nsFreeTypeFont : public nsFontGTK {
public:
  static nsFreeTypeFont *NewFont(nsITrueTypeFontCatalogEntry*,
                                 PRUint16, const char *);
};
#else

class nsFreeTypeFont : public nsFontGTK
{
public:

  nsFreeTypeFont();
  nsFreeTypeFont(nsITrueTypeFontCatalogEntry *, PRUint16, const char *);
  virtual ~nsFreeTypeFont(void);
  static nsFreeTypeFont *NewFont(nsITrueTypeFontCatalogEntry*,
                                 PRUint16, const char *);

  void LoadFont(void);

  virtual GdkFont* GetGDKFont(void);
  virtual PRBool   GetGDKFontIs10646(void);
  virtual PRBool   IsFreeTypeFont(void);

  virtual gint GetWidth(const PRUnichar* aString, PRUint32 aLength);
  virtual gint DrawString(nsRenderingContextGTK* aContext,
                          nsDrawingSurfaceGTK* aSurface, nscoord aX,
                          nscoord aY, const PRUnichar* aString,
                          PRUint32 aLength);
#ifdef MOZ_MATHML
  virtual nsresult GetBoundingMetrics(const PRUnichar*   aString,
                                      PRUint32           aLength,
                                      nsBoundingMetrics& aBoundingMetrics);
#endif
  virtual nsresult doGetBoundingMetrics(const PRUnichar*   aString,
                                        PRUint32 aLength,
                                        PRInt32* aLeftBearing,
                                        PRInt32* aRightBearing,
                                        PRInt32* aAscent,
                                        PRInt32* aDescent,
                                        PRInt32* aWidth);

  virtual PRUint32 Convert(const PRUnichar* aSrc, PRUint32 aSrcLen,
                           PRUnichar* aDest, PRUint32 aDestLen);

  FT_Face getFTFace();
  int     ascent();
  int     descent();
  PRBool  getXHeight(unsigned long &val);
  int     max_ascent();
  int     max_descent();
  int     max_width();
  PRBool  superscript_y(long &val);
  PRBool  subscript_y(long &val);
  PRBool  underlinePosition(long &val);
  PRBool  underline_thickness(unsigned long &val);

  FT_Error FaceRequester(FT_Face* aface);
  static void FreeGlobals();

  static PRUint8 sLinearWeightTable[256];

protected:
  XImage *GetXImage(PRUint32 width, PRUint32 height);
  nsITrueTypeFontCatalogEntry *mFaceID;
  PRUint16        mPixelSize;
  FTC_Image_Desc  mImageDesc;
  nsCOMPtr<nsIFreeType2> mFt2;
};

void WeightTableInitCorrection(PRUint8*, PRUint8, double);

#endif
#endif

