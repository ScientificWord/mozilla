/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 *   robert@ocallahan.org
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

#ifndef NSTEXTRUNTRANSFORMATIONS_H_
#define NSTEXTRUNTRANSFORMATIONS_H_

#include "gfxFont.h"

class nsTransformedTextRun;
class nsStyleContext;

class nsTransformingTextRunFactory {
public:
  virtual ~nsTransformingTextRunFactory() {}

  // Default 8-bit path just transforms to Unicode and takes that path
  gfxTextRun* MakeTextRun(const PRUint8* aString, PRUint32 aLength,
                          const gfxFontGroup::Parameters* aParams,
                          gfxFontGroup* aFontGroup, PRUint32 aFlags,
                          nsStyleContext** aStyles, PRBool aOwnsFactory = PR_TRUE);
  gfxTextRun* MakeTextRun(const PRUnichar* aString, PRUint32 aLength,
                          const gfxFontGroup::Parameters* aParams,
                          gfxFontGroup* aFontGroup, PRUint32 aFlags,
                          nsStyleContext** aStyles, PRBool aOwnsFactory = PR_TRUE);

  virtual void RebuildTextRun(nsTransformedTextRun* aTextRun, gfxContext* aRefContext) = 0;
};

/**
 * Builds textruns that render their text using a font-variant (i.e.,
 * smallcaps).
 */
class nsFontVariantTextRunFactory : public nsTransformingTextRunFactory {
public:
  virtual void RebuildTextRun(nsTransformedTextRun* aTextRun, gfxContext* aRefContext);
};

/**
 * Builds textruns that transform the text in some way (e.g., capitalize)
 * and then render the text using some other textrun implementation.
 */
class nsCaseTransformTextRunFactory : public nsTransformingTextRunFactory {
public:
  // We could add an optimization here so that when there is no inner
  // factory, no title-case conversion, and no upper-casing of SZLIG, we override
  // MakeTextRun (after making it virtual in the superclass) and have it
  // just convert the string to uppercase or lowercase and create the textrun
  // via the fontgroup.
  
  // Takes ownership of aInnerTransformTextRunFactory
  nsCaseTransformTextRunFactory(nsTransformingTextRunFactory* aInnerTransformingTextRunFactory,
                                PRBool aAllUppercase = PR_FALSE)
    : mInnerTransformingTextRunFactory(aInnerTransformingTextRunFactory),
      mAllUppercase(aAllUppercase) {}

  virtual void RebuildTextRun(nsTransformedTextRun* aTextRun, gfxContext* aRefContext);

protected:
  nsAutoPtr<nsTransformingTextRunFactory> mInnerTransformingTextRunFactory;
  PRPackedBool                            mAllUppercase;
};

/**
 * So that we can reshape as necessary, we store enough information
 * to fully rebuild the textrun contents.
 */
class nsTransformedTextRun : public gfxTextRun {
public:
  static nsTransformedTextRun *Create(const gfxTextRunFactory::Parameters* aParams,
                                      nsTransformingTextRunFactory* aFactory,
                                      gfxFontGroup* aFontGroup,
                                      const PRUnichar* aString, PRUint32 aLength,
                                      const PRUint32 aFlags, nsStyleContext** aStyles,
                                      PRBool aOwnsFactory);

  ~nsTransformedTextRun() {
    if (mOwnsFactory) {
      delete mFactory;
    }
  }
  
  virtual void SetCapitalization(PRUint32 aStart, PRUint32 aLength,
                                 PRPackedBool* aCapitalization,
                                 gfxContext* aRefContext);
  virtual PRBool SetPotentialLineBreaks(PRUint32 aStart, PRUint32 aLength,
                                        PRPackedBool* aBreakBefore,
                                        gfxContext* aRefContext);
  virtual PRBool SetLineBreaks(PRUint32 aStart, PRUint32 aLength,
                               PRBool aLineBreakBefore, PRBool aLineBreakAfter,
                               gfxFloat* aAdvanceWidthDelta,
                               gfxContext* aRefContext);

  nsTransformingTextRunFactory       *mFactory;
  nsTArray<PRUint32>                  mLineBreaks;
  nsTArray<nsRefPtr<nsStyleContext> > mStyles;
  nsTArray<PRPackedBool>              mCapitalize;
  PRPackedBool                        mOwnsFactory;

private:
  nsTransformedTextRun(const gfxTextRunFactory::Parameters* aParams,
                       nsTransformingTextRunFactory* aFactory,
                       gfxFontGroup* aFontGroup,
                       const PRUnichar* aString, PRUint32 aLength,
                       const PRUint32 aFlags, nsStyleContext** aStyles,
                       PRBool aOwnsFactory)
    : gfxTextRun(aParams, aString, aLength, aFontGroup, aFlags, sizeof(nsTransformedTextRun)),
      mFactory(aFactory), mOwnsFactory(aOwnsFactory)
  {
    PRUint32 i;
    for (i = 0; i < aLength; ++i) {
      mStyles.AppendElement(aStyles[i]);
    }
    for (i = 0; i < aParams->mInitialBreakCount; ++i) {
      mLineBreaks.AppendElement(aParams->mInitialBreaks[i]);
    }
  }  
};

#endif /*NSTEXTRUNTRANSFORMATIONS_H_*/
