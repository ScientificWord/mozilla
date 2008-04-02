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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

/*
 * style sheet and style rule processor representing style attributes
 * and some additional overrides
 */

#include "nsIHTMLCSSStyleSheet.h"
#include "nsCRT.h"
#include "nsIAtom.h"
#include "nsIURL.h"
#include "nsCSSPseudoElements.h"
#include "nsIStyleRule.h"
#include "nsIFrame.h"
#include "nsICSSStyleRule.h"
#include "nsIStyleRuleProcessor.h"
#include "nsPresContext.h"
#include "nsIDocument.h"
#include "nsCOMPtr.h"
#include "nsRuleWalker.h"
#include "nsRuleData.h"

/*
 * The CSSFirstLetterRule and CSSFirstLineRule exist so that we can fix
 * up the style data so that we don't have non-default values for the
 * properties that don't apply to :first-letter and :first-line.
 *
 * CSSDisablePropsRule is a common base class for both the
 * CSSFirstLetterRule and CSSFirstLineRule.
 */

class CSSDisablePropsRule : public nsIStyleRule {
public:
  CSSDisablePropsRule();
  virtual ~CSSDisablePropsRule();

  NS_DECL_ISUPPORTS

  // Call this something else so that this class still has pure virtual
  // functions.
  void CommonMapRuleInfoInto(nsRuleData* aRuleData);

#ifdef DEBUG
  NS_IMETHOD List(FILE* out = stdout, PRInt32 aIndent = 0) const;
#endif
protected:
  nsCSSValueList mInheritList;
  nsCSSQuotes mInheritQuotes;
  nsCSSCounterData mNoneCounter;
};

CSSDisablePropsRule::CSSDisablePropsRule()
{
  nsCSSValue none(eCSSUnit_None);
  mNoneCounter.mCounter = none;
  nsCSSValue inherit(eCSSUnit_Inherit);
  mInheritList.mValue = inherit;
  mInheritQuotes.mOpen = inherit;
}

class CSSFirstLineRule : public CSSDisablePropsRule {
public:
  CSSFirstLineRule() {}

  NS_IMETHOD MapRuleInfoInto(nsRuleData* aRuleData);
};

class CSSFirstLetterRule : public CSSDisablePropsRule {
public:
  CSSFirstLetterRule() {}

  NS_IMETHOD MapRuleInfoInto(nsRuleData* aRuleData);
};

CSSDisablePropsRule::~CSSDisablePropsRule()
{
}

NS_IMPL_ISUPPORTS1(CSSDisablePropsRule, nsIStyleRule)

#ifdef DEBUG
NS_IMETHODIMP
CSSDisablePropsRule::List(FILE* out, PRInt32 aIndent) const
{
  return NS_OK;
}
#endif

// -----------------------------------------------------------

/*
 * Note:  These rule mapping functions, unlike practically all others,
 * will overwrite the properties even if they're not |eCSSUnit_Null|.
 * XXX This is only a partial fix for the fact that they should be
 * higher in the cascade (at the very top).  It doesn't work in the case
 * where something higher in the cascade fully specifies the struct.
 *
 * XXX This should be cleaned up once we implement eCSSUnit_Initial
 * throughout.
 */

void
CSSDisablePropsRule::CommonMapRuleInfoInto(nsRuleData* aData)
{
  /*
   * Common code for disabling the properties that apply neither to
   * :first-letter nor to :first-line.
   */

  // Disable 'unicode-bidi'.
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(TextReset)) {
    nsCSSValue normal(eCSSUnit_Normal);
    aData->mTextData->mUnicodeBidi = normal;
  }

  // NOTE: 'text-align', 'text-indent', and 'white-space' should not be
  // handled by the frames so we don't need to bother.

  // Disable everything in the nsRuleDataDisplay struct except 'float'.
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Visibility)) {
    nsCSSValue inherit(eCSSUnit_Inherit);
    aData->mDisplayData->mVisibility = inherit;
    aData->mDisplayData->mDirection = inherit;
  }

  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Display)) {
    nsCSSValue none(eCSSUnit_None);
    aData->mDisplayData->mAppearance = none;

    nsCSSValue autovalue(eCSSUnit_Auto);
    aData->mDisplayData->mClip.mTop = autovalue;
    aData->mDisplayData->mClip.mRight = autovalue;
    aData->mDisplayData->mClip.mBottom = autovalue;
    aData->mDisplayData->mClip.mLeft = autovalue;

    nsCSSValue one(1.0f, eCSSUnit_Number);
    aData->mDisplayData->mOpacity = one;

    nsCSSValue inlinevalue(NS_STYLE_DISPLAY_INLINE, eCSSUnit_Enumerated);
    aData->mDisplayData->mDisplay = inlinevalue;

    aData->mDisplayData->mBinding = none;

    nsCSSValue staticposition(NS_STYLE_POSITION_STATIC, eCSSUnit_Enumerated);
    aData->mDisplayData->mPosition = staticposition;

    nsCSSValue visible(NS_STYLE_OVERFLOW_VISIBLE, eCSSUnit_Enumerated);
    aData->mDisplayData->mOverflowX = visible;
    aData->mDisplayData->mOverflowY = visible;

    aData->mDisplayData->mClear = none;

    // Nobody will care about 'break-before' or 'break-after', since
    // they only apply to blocks (assuming we implement them correctly).
  }

  // NOTE:  We'll never do anything with what's in nsCSSList,
  // nsCSSTable, nsCSSBreaks, nsCSSPage, nsCSSAural, nsCSSXUL, or
  // nsCSSSVG, so don't bother.

  // Disable everything in the position struct.
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Position)) {
    nsCSSValue autovalue(eCSSUnit_Auto);
    nsCSSValue none(eCSSUnit_None);
    nsCSSValue zero(0.0f, eCSSUnit_Point);
    aData->mPositionData->mOffset.mTop = autovalue;
    aData->mPositionData->mOffset.mRight = autovalue;
    aData->mPositionData->mOffset.mBottom = autovalue;
    aData->mPositionData->mOffset.mLeft = autovalue;
    aData->mPositionData->mWidth = autovalue;
    aData->mPositionData->mMinWidth = zero;
    aData->mPositionData->mMaxWidth = none;
    aData->mPositionData->mHeight = autovalue;
    aData->mPositionData->mMinHeight = zero;
    aData->mPositionData->mMaxHeight = none;
    nsCSSValue content(NS_STYLE_BOX_SIZING_CONTENT, eCSSUnit_Enumerated);
    aData->mPositionData->mBoxSizing = content;
    aData->mPositionData->mZIndex = autovalue;
  }

  // Disable everything in the Content struct.
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Content)) {
    // Don't bother resetting 'content'.

    aData->mContentData->mCounterIncrement = &mNoneCounter;
    aData->mContentData->mCounterReset = &mNoneCounter;

    nsCSSValue autovalue(eCSSUnit_Auto);
    aData->mContentData->mMarkerOffset = autovalue;
  }

  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Quotes)) {
    aData->mContentData->mQuotes = &mInheritQuotes;
  }

  // Disable everything in the UserInterface struct.
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(UserInterface)) {
    nsCSSValue inherit(eCSSUnit_Inherit);
    aData->mUserInterfaceData->mUserInput = inherit;
    aData->mUserInterfaceData->mUserModify = inherit;
    aData->mUserInterfaceData->mUserFocus = inherit;
    aData->mUserInterfaceData->mCursor = &mInheritList;
  }

  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(UIReset)) {
    nsCSSValue autovalue(eCSSUnit_Auto);
    nsCSSValue none(eCSSUnit_None);
    // Don't bother with '-moz-force-broken-image-icon' since it's only
    // half a property.
    // Don't bother with '-moz-user-select' because there's no way to
    // specify the initial value.
  }

  // Disable all outline properties.
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Outline)) {
    nsCSSValue none(NS_STYLE_BORDER_STYLE_NONE, eCSSUnit_Enumerated);
    aData->mMarginData->mOutlineStyle = none;
  }

}

NS_IMETHODIMP
CSSFirstLineRule::MapRuleInfoInto(nsRuleData* aData)
{
  /*
   * See CSS2.1 section 5.12.1, which says that the properties that apply
   * to :first-line are: font properties, color properties, background
   * properties, 'word-spacing', 'letter-spacing', 'text-decoration',
   * 'vertical-align', 'text-transform', and 'line-height'.
   *
   * We also allow 'text-shadow', which was listed in CSS2 (where the
   * property existed).
   */

  CommonMapRuleInfoInto(aData);

  // Disable 'float'.
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Display)) {
    nsCSSValue none(eCSSUnit_None);
    aData->mDisplayData->mFloat = none;
  }

  // Disable border properties, margin properties, and padding
  // properties.
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Border)) {
    nsCSSValue none(NS_STYLE_BORDER_STYLE_NONE, eCSSUnit_Enumerated);
    aData->mMarginData->mBorderStyle.mTop = none;
    aData->mMarginData->mBorderStyle.mRight = none;
    aData->mMarginData->mBorderStyle.mBottom = none;
    aData->mMarginData->mBorderStyle.mLeft = none;
  }

  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Margin)) {
    nsCSSValue zero(0.0f, eCSSUnit_Point);
    aData->mMarginData->mMargin.mTop = zero;
    aData->mMarginData->mMargin.mRight = zero;
    aData->mMarginData->mMargin.mBottom = zero;
    aData->mMarginData->mMargin.mLeft = zero;
  }

  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Padding)) {
    nsCSSValue zero(0.0f, eCSSUnit_Point);
    aData->mMarginData->mPadding.mTop = zero;
    aData->mMarginData->mPadding.mRight = zero;
    aData->mMarginData->mPadding.mBottom = zero;
    aData->mMarginData->mPadding.mLeft = zero;
  }

  return NS_OK;
}

NS_IMETHODIMP
CSSFirstLetterRule::MapRuleInfoInto(nsRuleData* aData)
{
  /*
   * See CSS2.1 section 5.12.2, which says that the properties that
   * apply to :first-letter are: font properties, 'text-decoration',
   * 'text-transform', 'letter-spacing', 'word-spacing' (when
   * appropriate), 'line-height', 'float', 'vertical-align' (only if
   * 'float' is 'none'), margin properties, padding properties, border
   * properties, 'color', and background properties.
   */

  CommonMapRuleInfoInto(aData);

  // NOTE:  'vertical-align' is only supposed to be relevant if 'float'
  // is 'none', but we don't do anything with it if 'float' is not none,
  // so we don't need to disable it.

  return NS_OK;
}

// -----------------------------------------------------------

class HTMLCSSStyleSheetImpl : public nsIHTMLCSSStyleSheet,
                              public nsIStyleRuleProcessor {
public:
  HTMLCSSStyleSheetImpl();

  NS_DECL_ISUPPORTS

  // basic style sheet data
  NS_IMETHOD Init(nsIURI* aURL, nsIDocument* aDocument);
  NS_IMETHOD Reset(nsIURI* aURL);
  NS_IMETHOD GetSheetURI(nsIURI** aSheetURL) const;
  NS_IMETHOD GetBaseURI(nsIURI** aBaseURL) const;
  NS_IMETHOD GetTitle(nsString& aTitle) const;
  NS_IMETHOD GetType(nsString& aType) const;
  NS_IMETHOD_(PRBool) UseForMedium(nsPresContext* aPresContext) const;
  NS_IMETHOD_(PRBool) HasRules() const;

  NS_IMETHOD GetApplicable(PRBool& aApplicable) const;
  
  NS_IMETHOD SetEnabled(PRBool aEnabled);

  NS_IMETHOD GetComplete(PRBool& aComplete) const;
  NS_IMETHOD SetComplete();

  // style sheet owner info
  NS_IMETHOD GetParentSheet(nsIStyleSheet*& aParent) const;  // will be null
  NS_IMETHOD GetOwningDocument(nsIDocument*& aDocument) const;
  NS_IMETHOD SetOwningDocument(nsIDocument* aDocument);

  // nsIStyleRuleProcessor api
  NS_IMETHOD RulesMatching(ElementRuleProcessorData* aData);

  NS_IMETHOD RulesMatching(PseudoRuleProcessorData* aData);

  NS_IMETHOD HasStateDependentStyle(StateRuleProcessorData* aData,
                                    nsReStyleHint* aResult);

  NS_IMETHOD HasAttributeDependentStyle(AttributeRuleProcessorData* aData,
                                        nsReStyleHint* aResult);

#ifdef DEBUG
  virtual void List(FILE* out = stdout, PRInt32 aIndent = 0) const;
#endif

private: 
  // These are not supported and are not implemented! 
  HTMLCSSStyleSheetImpl(const HTMLCSSStyleSheetImpl& aCopy); 
  HTMLCSSStyleSheetImpl& operator=(const HTMLCSSStyleSheetImpl& aCopy); 

protected:
  virtual ~HTMLCSSStyleSheetImpl();

protected:
  nsIURI*         mURL;
  nsIDocument*    mDocument;

  CSSFirstLineRule* mFirstLineRule;
  CSSFirstLetterRule* mFirstLetterRule;
};


HTMLCSSStyleSheetImpl::HTMLCSSStyleSheetImpl()
  : nsIHTMLCSSStyleSheet(),
    mRefCnt(0),
    mURL(nsnull),
    mDocument(nsnull),
    mFirstLineRule(nsnull),
    mFirstLetterRule(nsnull)
{
}

HTMLCSSStyleSheetImpl::~HTMLCSSStyleSheetImpl()
{
  NS_RELEASE(mURL);

  NS_IF_RELEASE(mFirstLineRule);
  NS_IF_RELEASE(mFirstLetterRule);
}

NS_IMPL_ISUPPORTS3(HTMLCSSStyleSheetImpl,
                   nsIHTMLCSSStyleSheet,
                   nsIStyleSheet,
                   nsIStyleRuleProcessor)

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::RulesMatching(ElementRuleProcessorData* aData)
{
  nsIContent* content = aData->mContent;
  
  if (content) {
    // just get the one and only style rule from the content's STYLE attribute
    nsICSSStyleRule* rule = content->GetInlineStyleRule();
    if (rule)
      aData->mRuleWalker->Forward(rule);
  }

  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::RulesMatching(PseudoRuleProcessorData* aData)
{
  // We only want to add these rules if there are real :first-letter or
  // :first-line rules that cause a pseudo-element frame to be created.
  // Otherwise the use of ProbePseudoStyleContextFor will prevent frame
  // creation, and adding rules here would cause it.
  if (aData->mRuleWalker->AtRoot())
    return NS_OK;

  nsIAtom* pseudoTag = aData->mPseudoTag;
  if (pseudoTag == nsCSSPseudoElements::firstLine) {
    if (!mFirstLineRule) {
      mFirstLineRule = new CSSFirstLineRule();
      if (!mFirstLineRule)
        return NS_ERROR_OUT_OF_MEMORY;
      NS_ADDREF(mFirstLineRule);
    }
    aData->mRuleWalker->Forward(mFirstLineRule);
  }
  else if (pseudoTag == nsCSSPseudoElements::firstLetter) {
    if (!mFirstLetterRule) {
      mFirstLetterRule = new CSSFirstLetterRule();
      if (!mFirstLetterRule)
        return NS_ERROR_OUT_OF_MEMORY;
      NS_ADDREF(mFirstLetterRule);
    }
    aData->mRuleWalker->Forward(mFirstLetterRule);
  } 
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::Init(nsIURI* aURL, nsIDocument* aDocument)
{
  NS_PRECONDITION(aURL && aDocument, "null ptr");
  if (! aURL || ! aDocument)
    return NS_ERROR_NULL_POINTER;

  if (mURL || mDocument)
    return NS_ERROR_ALREADY_INITIALIZED;

  mDocument = aDocument; // not refcounted!
  mURL = aURL;
  NS_ADDREF(mURL);
  return NS_OK;
}

// Test if style is dependent on content state
NS_IMETHODIMP
HTMLCSSStyleSheetImpl::HasStateDependentStyle(StateRuleProcessorData* aData,
                                              nsReStyleHint* aResult)
{
  *aResult = nsReStyleHint(0);
  return NS_OK;
}

// Test if style is dependent on attribute
NS_IMETHODIMP
HTMLCSSStyleSheetImpl::HasAttributeDependentStyle(AttributeRuleProcessorData* aData,
                                                  nsReStyleHint* aResult)
{
  *aResult = nsReStyleHint(0);
  return NS_OK;
}



NS_IMETHODIMP 
HTMLCSSStyleSheetImpl::Reset(nsIURI* aURL)
{
  NS_IF_RELEASE(mURL);
  mURL = aURL;
  NS_ADDREF(mURL);

  NS_IF_RELEASE(mFirstLineRule);
  NS_IF_RELEASE(mFirstLetterRule);
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetSheetURI(nsIURI** aSheetURL) const
{
  NS_IF_ADDREF(mURL);
  *aSheetURL = mURL;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetBaseURI(nsIURI** aBaseURL) const
{
  NS_IF_ADDREF(mURL);
  *aBaseURL = mURL;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetTitle(nsString& aTitle) const
{
  aTitle.AssignLiteral("Internal HTML/CSS Style Sheet");
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetType(nsString& aType) const
{
  aType.AssignLiteral("text/html");
  return NS_OK;
}

NS_IMETHODIMP_(PRBool)
HTMLCSSStyleSheetImpl::UseForMedium(nsPresContext* aPresContext) const
{
  return PR_TRUE; // works for all media
}

NS_IMETHODIMP_(PRBool)
HTMLCSSStyleSheetImpl::HasRules() const
{
  return PR_TRUE;  // We always have rules, since mFirstLineRule and
                   // mFirstLetterRule are created on request.
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetApplicable(PRBool& aApplicable) const
{
  aApplicable = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::SetEnabled(PRBool aEnabled)
{ // these can't be disabled
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetComplete(PRBool& aComplete) const
{
  aComplete = PR_TRUE;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::SetComplete()
{
  return NS_OK;
}

// style sheet owner info
NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetParentSheet(nsIStyleSheet*& aParent) const
{
  aParent = nsnull;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::GetOwningDocument(nsIDocument*& aDocument) const
{
  NS_IF_ADDREF(mDocument);
  aDocument = mDocument;
  return NS_OK;
}

NS_IMETHODIMP
HTMLCSSStyleSheetImpl::SetOwningDocument(nsIDocument* aDocument)
{
  mDocument = aDocument;
  return NS_OK;
}

#ifdef DEBUG
void HTMLCSSStyleSheetImpl::List(FILE* out, PRInt32 aIndent) const
{
  // Indent
  for (PRInt32 index = aIndent; --index >= 0; ) fputs("  ", out);

  fputs("HTML CSS Style Sheet: ", out);
  nsCAutoString urlSpec;
  mURL->GetSpec(urlSpec);
  if (!urlSpec.IsEmpty()) {
    fputs(urlSpec.get(), out);
  }
  fputs("\n", out);
}
#endif

// XXX For backwards compatibility and convenience
nsresult
NS_NewHTMLCSSStyleSheet(nsIHTMLCSSStyleSheet** aInstancePtrResult,
                        nsIURI* aURL, nsIDocument* aDocument)
{
  nsresult rv;
  nsIHTMLCSSStyleSheet* sheet;
  if (NS_FAILED(rv = NS_NewHTMLCSSStyleSheet(&sheet)))
    return rv;

  if (NS_FAILED(rv = sheet->Init(aURL, aDocument))) {
    NS_RELEASE(sheet);
    return rv;
  }

  *aInstancePtrResult = sheet;
  return NS_OK;
}

nsresult
NS_NewHTMLCSSStyleSheet(nsIHTMLCSSStyleSheet** aInstancePtrResult)
{
  if (aInstancePtrResult == nsnull) {
    return NS_ERROR_NULL_POINTER;
  }

  HTMLCSSStyleSheetImpl*  it = new HTMLCSSStyleSheetImpl();

  if (nsnull == it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  NS_ADDREF(it);
  *aInstancePtrResult = it;
  return NS_OK;
}
