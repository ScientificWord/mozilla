/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 *   Lars Knoll <knoll@kde.org>
 *   Zack Rusin <zack@kde.org>
 *   John C. Griggs <jcgriggs@sympatico.ca>
 *   Jean Claude Batista <jcb@macadamian.com>
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

#ifndef nsFontMetricsQt_h__
#define nsFontMetricsQt_h__

#include "nsIFontMetrics.h"
#include "nsIFontEnumerator.h"
#include "nsIDeviceContext.h"
#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsICharRepresentable.h"
#include "nsICharsetConverterManager.h"
#include "nsVoidArray.h"
#include "nsFont.h"

#include <qfont.h>
#include <qfontmetrics.h>

class nsFont;
class nsString;
class nsRenderingContextQt;
class nsDrawingSurfaceQt;
class nsFontMetricsQt;
class nsFontQtUserDefined;
class QString;
class QFontInfo;
class QFontDatabase;

#undef FONT_HAS_GLYPH
#define FONT_HAS_GLYPH(map,char) IS_REPRESENTABLE(map,char)

typedef struct nsFontCharSetInfo nsFontCharSetInfo;

class nsFontQt
{
public:
    nsFontQt(const nsFont &afont, nsIAtom *aLangGroup, nsIDeviceContext *acontext);
    ~nsFontQt() {}
    NS_DECL_AND_IMPL_ZEROING_OPERATOR_NEW

    bool SupportsChar(PRUnichar c) { return QFontMetrics(font).inFont(QChar(c)); }

    QFont font;

    nsIDeviceContext *mDeviceContext;
    nsCOMPtr<nsIAtom> mLangGroup;

    nscoord          mLeading;
    nscoord          mEmHeight;
    nscoord          mEmAscent;
    nscoord          mEmDescent;
    nscoord          mMaxHeight;
    nscoord          mMaxAscent;
    nscoord          mMaxDescent;
    nscoord          mMaxAdvance;
    nscoord          mAveCharWidth;
    nscoord          mXHeight;
    nscoord          mSuperscriptOffset;
    nscoord          mSubscriptOffset;
    nscoord          mStrikeoutSize;
    nscoord          mStrikeoutOffset;
    nscoord          mUnderlineSize;
    nscoord          mUnderlineOffset;
    nscoord          mSpaceWidth;

    PRUint16         mPixelSize;
    PRUint16         mWeight;

    void RealizeFont();
};

class nsFontMetricsQt : public nsIFontMetrics
{
public:
    nsFontMetricsQt();
    virtual ~nsFontMetricsQt();

    NS_DECL_AND_IMPL_ZEROING_OPERATOR_NEW

    NS_DECL_ISUPPORTS

    NS_IMETHOD  Init(const nsFont& aFont, nsIAtom* aLangGroup,
                     nsIDeviceContext* aContext);
    NS_IMETHOD  Destroy();

    NS_IMETHOD  GetXHeight(nscoord& aResult);
    NS_IMETHOD  GetSuperscriptOffset(nscoord& aResult);
    NS_IMETHOD  GetSubscriptOffset(nscoord& aResult);
    NS_IMETHOD  GetStrikeout(nscoord& aOffset, nscoord& aSize);
    NS_IMETHOD  GetUnderline(nscoord& aOffset, nscoord& aSize);

    NS_IMETHOD  GetHeight(nscoord &aHeight);
    NS_IMETHOD  GetNormalLineHeight(nscoord &aHeight);
    NS_IMETHOD  GetLeading(nscoord &aLeading);
    NS_IMETHOD  GetEmHeight(nscoord &aHeight);
    NS_IMETHOD  GetEmAscent(nscoord &aAscent);
    NS_IMETHOD  GetEmDescent(nscoord &aDescent);
    NS_IMETHOD  GetMaxHeight(nscoord &aHeight);
    NS_IMETHOD  GetMaxAscent(nscoord &aAscent);
    NS_IMETHOD  GetMaxDescent(nscoord &aDescent);
    NS_IMETHOD  GetMaxAdvance(nscoord &aAdvance);
    NS_IMETHOD  GetAveCharWidth(nscoord &aAveCharWidth);
    NS_IMETHOD  GetLangGroup(nsIAtom** aLangGroup);
    NS_IMETHOD  GetFontHandle(nsFontHandle &aHandle);

    NS_IMETHOD  GetSpaceWidth(nscoord &aSpaceWidth);
    // No known string length limits on Qt
    virtual PRInt32 GetMaxStringLength() { return PR_INT32_MAX; }

    nsFontQt    *qFont;

#if 0
    nsFontQt*  FindFont(PRUnichar aChar);
    nsFontQt*  FindUserDefinedFont(PRUnichar aChar);
    nsFontQt*  FindLangGroupPrefFont(nsIAtom *aLangGroup,PRUnichar aChar);
    nsFontQt*  FindLocalFont(PRUnichar aChar);
    nsFontQt*  FindGenericFont(PRUnichar aChar);
    nsFontQt*  FindGlobalFont(PRUnichar aChar);
    nsFontQt*  FindSubstituteFont(PRUnichar aChar);

    nsFontQt*  LookUpFontPref(nsCAutoString &aName,PRUnichar aChar);
    nsFontQt*  LoadFont(QString &aName,PRUnichar aChar);
    nsFontQt*  LoadFont(QString &aName,const QString &aCharSet,
                        PRUnichar aChar);
    QFont*     LoadQFont(QString &aName);

    static nsresult FamilyExists(const nsString& aFontName);

    PRUint16    mLoadedFontsAlloc;
    PRUint16    mLoadedFontsCount;

    nsFontQt               *mSubstituteFont;
    nsFontQtUserDefined    *mUserDefinedFont;

    nsCOMPtr<nsIAtom> mLangGroup;
    nsCStringArray    mFonts;
    PRInt32           mFontsIndex;
    nsVoidArray       mFontIsGeneric;
    nsCAutoString     mDefaultFont;
    nsCString         *mGeneric;
    nsCAutoString     mUserDefined;

    PRUint8 mTriedAllGenerics;
    PRUint8 mIsUserDefined;

    static QFontDatabase *GetQFontDB();

protected:
    void RealizeFont();

    nsIDeviceContext *mDeviceContext;
    nsFont           *mFont;
    nsFontQt         *mWesternFont;

    QString          *mQStyle;

    QIntDict<char>   mCharSubst;


    static QFontDatabase    *mQFontDB;
#endif
};

class nsFontEnumeratorQt : public nsIFontEnumerator
{
public:
  nsFontEnumeratorQt();
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFONTENUMERATOR
};

#endif
