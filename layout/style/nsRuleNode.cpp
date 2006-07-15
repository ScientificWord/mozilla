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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Original Author: David W. Hyatt (hyatt@netscape.com)
 *   Daniel Glazman <glazman@netscape.com>
 *   Roger B. Sidje <rbs@maths.uq.edu.au>
 *   Mats Palmgren <mats.palmgren@bredband.net>
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
 * a node in the lexicographic tree of rules that match an element,
 * responsible for converting the rules' information into computed style
 */

#include "nsRuleNode.h"
#include "nscore.h"
#include "nsIServiceManager.h"
#include "nsIDeviceContext.h"
#include "nsILookAndFeel.h"
#include "nsIPresShell.h"
#include "nsIFontMetrics.h"
#include "nsIDocShellTreeItem.h"
#include "nsStyleUtil.h"
#include "nsCSSPseudoElements.h"
#include "nsThemeConstants.h"
#include "nsITheme.h"
#include "pldhash.h"
#include "nsStyleContext.h"
#include "nsStyleSet.h"
#include "nsSize.h"
#include "imgIRequest.h"
#include "nsRuleData.h"
#include "nsILanguageAtomService.h"
#include "nsIStyleRule.h"

/*
 * For storage of an |nsRuleNode|'s children in a linked list.
 */
struct nsRuleList {
  nsRuleNode* mRuleNode;
  nsRuleList* mNext;
  
public:
  nsRuleList(nsRuleNode* aNode, nsRuleList* aNext= nsnull) {
    MOZ_COUNT_CTOR(nsRuleList);
    mRuleNode = aNode;
    mNext = aNext;
  }
 
  ~nsRuleList() {
    MOZ_COUNT_DTOR(nsRuleList);
    mRuleNode->Destroy();
    if (mNext)
      mNext->Destroy(mNext->mRuleNode->mPresContext);
  }

  void* operator new(size_t sz, nsPresContext* aContext) CPP_THROW_NEW {
    return aContext->AllocateFromShell(sz);
  };
  void operator delete(void* aPtr) {} // Does nothing. The arena will free us up when the rule tree
                                      // dies.

  void Destroy(nsPresContext* aContext) {
    this->~nsRuleList();
    aContext->FreeToShell(sizeof(nsRuleList), this);
  }

  // Destroy this node, but not its rule node or the rest of the list.
  nsRuleList* DestroySelf(nsPresContext* aContext) {
    nsRuleList *next = mNext;
    MOZ_COUNT_DTOR(nsRuleList); // hack
    aContext->FreeToShell(sizeof(nsRuleList), this);
    return next;
  }
};

/*
 * For storage of an |nsRuleNode|'s children in a PLDHashTable.
 */

struct ChildrenHashEntry : public PLDHashEntryHdr {
  // key (the rule) is |mRuleNode->GetRule()|
  nsRuleNode *mRuleNode;
};

PR_STATIC_CALLBACK(const void *)
ChildrenHashGetKey(PLDHashTable *table, PLDHashEntryHdr *hdr)
{
  ChildrenHashEntry *entry = NS_STATIC_CAST(ChildrenHashEntry*, hdr);
  return entry->mRuleNode->GetRule();
}

PR_STATIC_CALLBACK(PRBool)
ChildrenHashMatchEntry(PLDHashTable *table, const PLDHashEntryHdr *hdr,
                         const void *key)
{
  const ChildrenHashEntry *entry =
    NS_STATIC_CAST(const ChildrenHashEntry*, hdr);
  return entry->mRuleNode->GetRule() == key;
}

static PLDHashTableOps ChildrenHashOps = {
  // It's probably better to allocate the table itself using malloc and
  // free rather than the pres shell's arena because the table doesn't
  // grow very often and the pres shell's arena doesn't recycle very
  // large size allocations.
  PL_DHashAllocTable,
  PL_DHashFreeTable,
  ChildrenHashGetKey,
  PL_DHashVoidPtrKeyStub,
  ChildrenHashMatchEntry,
  PL_DHashMoveEntryStub,
  PL_DHashClearEntryStub,
  PL_DHashFinalizeStub,
  NULL
};


// EnsureBlockDisplay:
//  - if the display value (argument) is not a block-type
//    then we set it to a valid block display value
//  - For enforcing the floated/positioned element CSS2 rules
static void EnsureBlockDisplay(PRUint8& display)
{
  // see if the display value is already a block
  switch (display) {
  case NS_STYLE_DISPLAY_NONE :
    // never change display:none *ever*
  case NS_STYLE_DISPLAY_TABLE :
  case NS_STYLE_DISPLAY_BLOCK :
  case NS_STYLE_DISPLAY_LIST_ITEM :
    // do not muck with these at all - already blocks
    // This is equivalent to nsStyleDisplay::IsBlockLevel.  (XXX Maybe we
    // should just call that?)
    break;

  case NS_STYLE_DISPLAY_INLINE_TABLE :
    // make inline tables into tables
    display = NS_STYLE_DISPLAY_TABLE;
    break;

  default :
    // make it a block
    display = NS_STYLE_DISPLAY_BLOCK;
  }
}

// XXX This should really be done in the CSS parser.
nsString& Unquote(nsString& aString)
{
  PRUnichar start = aString.First();
  PRUnichar end = aString.Last();

  if ((start == end) && 
      ((start == PRUnichar('\"')) || 
       (start == PRUnichar('\'')))) {
    PRInt32 length = aString.Length();
    aString.Truncate(length - 1);
    aString.Cut(0, 1);
  }
  return aString;
}

nscoord CalcLength(const nsCSSValue& aValue,
                   const nsFont* aFont, 
                   nsStyleContext* aStyleContext,
                   nsPresContext* aPresContext,
                   PRBool& aInherited)
{
  NS_ASSERTION(aValue.IsLengthUnit(), "not a length unit");
  if (aValue.IsFixedLengthUnit()) {
    return aValue.GetLengthTwips();
  }
  nsCSSUnit unit = aValue.GetUnit();
  if (unit == eCSSUnit_Pixel) {
    return NSFloatPixelsToTwips(aValue.GetFloatValue(),
                                aPresContext->ScaledPixelsToTwips());
  }
  // Common code for all units other than pixels:
  aInherited = PR_TRUE;
  const nsFont* font;
  if (aStyleContext) {
    font = &aStyleContext->GetStyleFont()->mFont;
  } else {
    font = aFont;
  }
  switch (unit) {
    case eCSSUnit_EM:
    case eCSSUnit_Char: {
      return NSToCoordRound(aValue.GetFloatValue() * (float)font->size);
      // XXX scale against font metrics height instead?
    }
    case eCSSUnit_EN: {
      return NSToCoordRound((aValue.GetFloatValue() * (float)font->size) / 2.0f);
    }
    case eCSSUnit_XHeight: {
      nsCOMPtr<nsIFontMetrics> fm = aPresContext->GetMetricsFor(*font);
      nscoord xHeight;
      fm->GetXHeight(xHeight);
      return NSToCoordRound(aValue.GetFloatValue() * (float)xHeight);
    }
    case eCSSUnit_CapHeight: {
      NS_NOTYETIMPLEMENTED("cap height unit");
      nscoord capHeight = ((font->size / 3) * 2); // XXX HACK!
      return NSToCoordRound(aValue.GetFloatValue() * (float)capHeight);
    }
    default:
      NS_NOTREACHED("unexpected unit");
      break;
  }
  return 0;
}

#define SETCOORD_NORMAL       0x01   // N
#define SETCOORD_AUTO         0x02   // A
#define SETCOORD_INHERIT      0x04   // H
#define SETCOORD_PERCENT      0x08   // P
#define SETCOORD_FACTOR       0x10   // F
#define SETCOORD_LENGTH       0x20   // L
#define SETCOORD_INTEGER      0x40   // I
#define SETCOORD_ENUMERATED   0x80   // E

#define SETCOORD_LP     (SETCOORD_LENGTH | SETCOORD_PERCENT)
#define SETCOORD_LH     (SETCOORD_LENGTH | SETCOORD_INHERIT)
#define SETCOORD_AH     (SETCOORD_AUTO | SETCOORD_INHERIT)
#define SETCOORD_LAH    (SETCOORD_AUTO | SETCOORD_LENGTH | SETCOORD_INHERIT)
#define SETCOORD_LPH    (SETCOORD_LP | SETCOORD_INHERIT)
#define SETCOORD_LPAH   (SETCOORD_LP | SETCOORD_AH)
#define SETCOORD_LPEH   (SETCOORD_LP | SETCOORD_ENUMERATED | SETCOORD_INHERIT)
#define SETCOORD_LE     (SETCOORD_LENGTH | SETCOORD_ENUMERATED)
#define SETCOORD_LEH    (SETCOORD_LE | SETCOORD_INHERIT)
#define SETCOORD_IA     (SETCOORD_INTEGER | SETCOORD_AUTO)
#define SETCOORD_LAE    (SETCOORD_LENGTH | SETCOORD_AUTO | SETCOORD_ENUMERATED)

// changes aCoord iff it returns PR_TRUE
static PRBool SetCoord(const nsCSSValue& aValue, nsStyleCoord& aCoord, 
                       const nsStyleCoord& aParentCoord,
                       PRInt32 aMask, nsStyleContext* aStyleContext,
                       nsPresContext* aPresContext, PRBool& aInherited)
{
  PRBool  result = PR_TRUE;
  if (aValue.GetUnit() == eCSSUnit_Null) {
    result = PR_FALSE;
  }
  else if (((aMask & SETCOORD_LENGTH) != 0) && 
           (aValue.GetUnit() == eCSSUnit_Char)) {
    aCoord.SetIntValue(NSToIntFloor(aValue.GetFloatValue()), eStyleUnit_Chars);
  } 
  else if (((aMask & SETCOORD_LENGTH) != 0) && 
           aValue.IsLengthUnit()) {
    aCoord.SetCoordValue(CalcLength(aValue, nsnull, aStyleContext, aPresContext, aInherited));
  } 
  else if (((aMask & SETCOORD_PERCENT) != 0) && 
           (aValue.GetUnit() == eCSSUnit_Percent)) {
    aCoord.SetPercentValue(aValue.GetPercentValue());
  } 
  else if (((aMask & SETCOORD_INTEGER) != 0) && 
           (aValue.GetUnit() == eCSSUnit_Integer)) {
    aCoord.SetIntValue(aValue.GetIntValue(), eStyleUnit_Integer);
  } 
  else if (((aMask & SETCOORD_ENUMERATED) != 0) && 
           (aValue.GetUnit() == eCSSUnit_Enumerated)) {
    aCoord.SetIntValue(aValue.GetIntValue(), eStyleUnit_Enumerated);
  } 
  else if (((aMask & SETCOORD_AUTO) != 0) && 
           (aValue.GetUnit() == eCSSUnit_Auto)) {
    aCoord.SetAutoValue();
  } 
  else if (((aMask & SETCOORD_INHERIT) != 0) && 
           (aValue.GetUnit() == eCSSUnit_Inherit)) {
    aCoord = aParentCoord;  // just inherit value from parent
    aInherited = PR_TRUE;
  }
  else if (((aMask & SETCOORD_NORMAL) != 0) && 
           (aValue.GetUnit() == eCSSUnit_Normal)) {
    aCoord.SetNormalValue();
  }
  else if (((aMask & SETCOORD_FACTOR) != 0) && 
           (aValue.GetUnit() == eCSSUnit_Number)) {
    aCoord.SetFactorValue(aValue.GetFloatValue());
  }
  else {
    result = PR_FALSE;  // didn't set anything
  }
  return result;
}

static PRBool SetColor(const nsCSSValue& aValue, const nscolor aParentColor, 
                       nsPresContext* aPresContext, nsStyleContext *aContext,
                       nscolor& aResult, PRBool& aInherited)
{
  PRBool  result = PR_FALSE;
  nsCSSUnit unit = aValue.GetUnit();

  if (eCSSUnit_Color == unit) {
    aResult = aValue.GetColorValue();
    result = PR_TRUE;
  }
  else if (eCSSUnit_String == unit) {
    nsAutoString  value;
    aValue.GetStringValue(value);
    nscolor rgba;
    if (NS_ColorNameToRGB(value, &rgba)) {
      aResult = rgba;
      result = PR_TRUE;
    }
  }
  else if (eCSSUnit_Integer == unit) {
    PRInt32 intValue = aValue.GetIntValue();
    if (0 <= intValue) {
      nsILookAndFeel* look = aPresContext->LookAndFeel();
      nsILookAndFeel::nsColorID colorID = (nsILookAndFeel::nsColorID) intValue;
      if (NS_SUCCEEDED(look->GetColor(colorID, aResult))) {
        result = PR_TRUE;
      }
    }
    else {
      switch (intValue) {
        case NS_COLOR_MOZ_HYPERLINKTEXT:
          aResult = aPresContext->DefaultLinkColor();
          break;
        case NS_COLOR_MOZ_VISITEDHYPERLINKTEXT:
          aResult = aPresContext->DefaultVisitedLinkColor();
          break;
        case NS_COLOR_MOZ_ACTIVEHYPERLINKTEXT:
          aResult = aPresContext->DefaultActiveLinkColor();
          break;
        case NS_COLOR_CURRENTCOLOR:
          // The data computed from this can't be shared in the rule tree 
          // because they could be used on a node with a different color
          aInherited = PR_TRUE;
          aResult = aContext->GetStyleColor()->mColor;
          break;
        default:
          NS_NOTREACHED("Should never have an unknown negative colorID.");
          break;
      }
      result = PR_TRUE;
    }
  }
  else if (eCSSUnit_Inherit == unit) {
    aResult = aParentColor;
    result = PR_TRUE;
    aInherited = PR_TRUE;
  }
  return result;
}

// Overloaded new operator. Initializes the memory to 0 and relies on an arena
// (which comes from the presShell) to perform the allocation.
void* 
nsRuleNode::operator new(size_t sz, nsPresContext* aPresContext) CPP_THROW_NEW
{
  // Check the recycle list first.
  return aPresContext->AllocateFromShell(sz);
}

// Overridden to prevent the global delete from being called, since the memory
// came out of an nsIArena instead of the global delete operator's heap.
void 
nsRuleNode::Destroy()
{
  // Destroy ourselves.
  this->~nsRuleNode();
  
  // Don't let the memory be freed, since it will be recycled
  // instead. Don't call the global operator delete.
  mPresContext->FreeToShell(sizeof(nsRuleNode), this);
}

nsRuleNode* nsRuleNode::CreateRootNode(nsPresContext* aPresContext)
{
  return new (aPresContext) nsRuleNode(aPresContext, nsnull, nsnull);
}

nsILanguageAtomService* nsRuleNode::gLangService = nsnull;

nsRuleNode::nsRuleNode(nsPresContext* aContext, nsIStyleRule* aRule, nsRuleNode* aParent)
  : mPresContext(aContext),
    mParent(aParent),
    mRule(aRule),
    mChildrenTaggedPtr(nsnull),
    mDependentBits(0),
    mNoneBits(0)
{
  MOZ_COUNT_CTOR(nsRuleNode);
  NS_IF_ADDREF(mRule);
}

PR_STATIC_CALLBACK(PLDHashOperator)
DeleteRuleNodeChildren(PLDHashTable *table, PLDHashEntryHdr *hdr,
                       PRUint32 number, void *arg)
{
  ChildrenHashEntry *entry = NS_STATIC_CAST(ChildrenHashEntry*, hdr);
  entry->mRuleNode->Destroy();
  return PL_DHASH_NEXT;
}

nsRuleNode::~nsRuleNode()
{
  MOZ_COUNT_DTOR(nsRuleNode);
  if (mStyleData.mResetData || mStyleData.mInheritedData)
    mStyleData.Destroy(0, mPresContext);
  if (ChildrenAreHashed()) {
    PLDHashTable *children = ChildrenHash();
    PL_DHashTableEnumerate(children, DeleteRuleNodeChildren, nsnull);
    PL_DHashTableDestroy(children);
  } else if (HaveChildren())
    ChildrenList()->Destroy(mPresContext);
  NS_IF_RELEASE(mRule);
}

nsresult 
nsRuleNode::Transition(nsIStyleRule* aRule, nsRuleNode** aResult)
{
  nsRuleNode* next = nsnull;

  if (HaveChildren() && !ChildrenAreHashed()) {
    PRInt32 numKids = 0;
    nsRuleList* curr = ChildrenList();
    while (curr && curr->mRuleNode->mRule != aRule) {
      curr = curr->mNext;
      ++numKids;
    }
    if (curr)
      next = curr->mRuleNode;
    else if (numKids >= kMaxChildrenInList)
      ConvertChildrenToHash();
  }

  if (ChildrenAreHashed()) {
    ChildrenHashEntry *entry = NS_STATIC_CAST(ChildrenHashEntry*,
        PL_DHashTableOperate(ChildrenHash(), aRule, PL_DHASH_ADD));
    if (!entry) {
      *aResult = nsnull;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    if (entry->mRuleNode)
      next = entry->mRuleNode;
    else {
      next = entry->mRuleNode =
          new (mPresContext) nsRuleNode(mPresContext, aRule, this);
      if (!next) {
        PL_DHashTableRawRemove(ChildrenHash(), entry);
        *aResult = nsnull;
        return NS_ERROR_OUT_OF_MEMORY;
      }
    }
  } else if (!next) {
    // Create the new entry in our list.
    next = new (mPresContext) nsRuleNode(mPresContext, aRule, this);
    if (!next) {
      *aResult = nsnull;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    nsRuleList* newChildrenList = new (mPresContext) nsRuleList(next, ChildrenList());
    if (NS_UNLIKELY(!newChildrenList)) {
      next->Destroy();
      *aResult = nsnull;
      return NS_ERROR_OUT_OF_MEMORY;
    }
    SetChildrenList(newChildrenList);
  }
  
  *aResult = next;
  return NS_OK;
}

void
nsRuleNode::ConvertChildrenToHash()
{
  NS_ASSERTION(!ChildrenAreHashed() && HaveChildren(),
               "must have a non-empty list of children");
  PLDHashTable *hash = PL_NewDHashTable(&ChildrenHashOps, nsnull,
                                        sizeof(ChildrenHashEntry),
                                        kMaxChildrenInList * 4);
  if (!hash)
    return;
  for (nsRuleList* curr = ChildrenList(); curr;
       curr = curr->DestroySelf(mPresContext)) {
    // This will never fail because of the initial size we gave the table.
    ChildrenHashEntry *entry = NS_STATIC_CAST(ChildrenHashEntry*,
        PL_DHashTableOperate(hash, curr->mRuleNode->mRule, PL_DHASH_ADD));
    NS_ASSERTION(!entry->mRuleNode, "duplicate entries in list");
    entry->mRuleNode = curr->mRuleNode;
  }
  SetChildrenHash(hash);
}

PR_STATIC_CALLBACK(PLDHashOperator)
ClearStyleDataHelper(PLDHashTable *table, PLDHashEntryHdr *hdr,
                               PRUint32 number, void *arg)
{
  ChildrenHashEntry *entry = NS_STATIC_CAST(ChildrenHashEntry*, hdr);
  entry->mRuleNode->ClearStyleData();
  return PL_DHASH_NEXT;
}

nsresult
nsRuleNode::ClearStyleData()
{
  // Blow away all data stored at this node.
  if (mStyleData.mResetData || mStyleData.mInheritedData)
    mStyleData.Destroy(0, mPresContext);

  mNoneBits &= ~NS_STYLE_INHERIT_MASK;
  mDependentBits &= ~NS_STYLE_INHERIT_MASK;

  if (ChildrenAreHashed())
    PL_DHashTableEnumerate(ChildrenHash(),
                           ClearStyleDataHelper, nsnull);
  else
    for (nsRuleList* curr = ChildrenList(); curr; curr = curr->mNext)
      curr->mRuleNode->ClearStyleData();

  return NS_OK;
}

inline void
nsRuleNode::PropagateNoneBit(PRUint32 aBit, nsRuleNode* aHighestNode)
{
  nsRuleNode* curr = this;
  for (;;) {
    NS_ASSERTION(!(curr->mNoneBits & aBit), "propagating too far");
    curr->mNoneBits |= aBit;
    if (curr == aHighestNode)
      break;
    curr = curr->mParent;
  }
}

inline void
nsRuleNode::PropagateDependentBit(PRUint32 aBit, nsRuleNode* aHighestNode)
{
  for (nsRuleNode* curr = this; curr != aHighestNode; curr = curr->mParent) {
    if (curr->mDependentBits & aBit) {
#ifdef DEBUG
      while (curr != aHighestNode) {
        NS_ASSERTION(curr->mDependentBits & aBit, "bit not set");
        curr = curr->mParent;
      }
#endif
      break;
    }

    curr->mDependentBits |= aBit;
  }
}

/*
 * The following "Check" functions are used for determining what type of
 * sharing can be used for the data on this rule node.  MORE HERE...
 */

/* the information for a property (or in some cases, a rect group of
   properties) */

struct PropertyCheckData {
  size_t offset;
  nsCSSType type;
};

/*
 * a callback function that that can revise the result of
 * CheckSpecifiedProperties before finishing; aResult is the current
 * result, and it returns the revised one.
 */
typedef nsRuleNode::RuleDetail
  (* PR_CALLBACK CheckCallbackFn)(const nsRuleDataStruct& aData,
                                  nsRuleNode::RuleDetail aResult);

/* the information for all the properties in a style struct */
struct StructCheckData {
  const PropertyCheckData* props;
  PRInt32 nprops;
  CheckCallbackFn callback;
};


/**
 * @param aValue the value being examined
 * @param aSpecifiedCount to be incremented by one if the value is specified
 * @param aInherited to be incremented by one if the value is set to inherit
 */
inline void
ExamineCSSValue(const nsCSSValue& aValue,
                PRUint32& aSpecifiedCount, PRUint32& aInheritedCount)
{
  if (aValue.GetUnit() != eCSSUnit_Null) {
    ++aSpecifiedCount;
    if (aValue.GetUnit() == eCSSUnit_Inherit) {
      ++aInheritedCount;
    }
  }
}

static void
ExamineCSSValuePair(const nsCSSValuePair* aValuePair,
                    PRUint32& aSpecifiedCount, PRUint32& aInheritedCount)
{
  NS_PRECONDITION(aValuePair, "Must have a value pair");
  
  ExamineCSSValue(aValuePair->mXValue, aSpecifiedCount, aInheritedCount);
  ExamineCSSValue(aValuePair->mYValue, aSpecifiedCount, aInheritedCount);
}

static void
ExamineCSSRect(const nsCSSRect* aRect,
               PRUint32& aSpecifiedCount, PRUint32& aInheritedCount)
{
  NS_PRECONDITION(aRect, "Must have a rect");

  NS_FOR_CSS_SIDES(side) {
    ExamineCSSValue(aRect->*(nsCSSRect::sides[side]),
                    aSpecifiedCount, aInheritedCount);
  }
}

PR_STATIC_CALLBACK(nsRuleNode::RuleDetail)
CheckFontCallback(const nsRuleDataStruct& aData,
                  nsRuleNode::RuleDetail aResult)
{
  const nsRuleDataFont& fontData =
      NS_STATIC_CAST(const nsRuleDataFont&, aData);
  if (eCSSUnit_Enumerated == fontData.mFamily.GetUnit()) {
    // A special case. We treat this as a fully specified font,
    // since no other font props are legal with a system font.
    NS_ASSERTION(aResult == nsRuleNode::eRulePartialReset ||
                 aResult == nsRuleNode::eRuleFullReset ||
                 aResult == nsRuleNode::eRulePartialMixed ||
                 aResult == nsRuleNode::eRuleFullMixed,
                 "we know we already have a reset-counted property");
    PRInt32 family = fontData.mFamily.GetIntValue();
    if ((family == NS_STYLE_FONT_CAPTION) ||
        (family == NS_STYLE_FONT_ICON) ||
        (family == NS_STYLE_FONT_MENU) ||
        (family == NS_STYLE_FONT_MESSAGE_BOX) ||
        (family == NS_STYLE_FONT_SMALL_CAPTION) ||
        (family == NS_STYLE_FONT_STATUS_BAR) ||
        (family == NS_STYLE_FONT_WINDOW) ||
        (family == NS_STYLE_FONT_DOCUMENT) ||
        (family == NS_STYLE_FONT_WORKSPACE) ||
        (family == NS_STYLE_FONT_DESKTOP) ||
        (family == NS_STYLE_FONT_INFO) ||
        (family == NS_STYLE_FONT_DIALOG) ||
        (family == NS_STYLE_FONT_BUTTON) ||
        (family == NS_STYLE_FONT_PULL_DOWN_MENU) ||
        (family == NS_STYLE_FONT_LIST) ||
        (family == NS_STYLE_FONT_FIELD)) {
      // promote partial to full since we're fully specified
      if (aResult == nsRuleNode::eRulePartialMixed ||
          aResult == nsRuleNode::eRuleFullMixed) {
        aResult = nsRuleNode::eRuleFullMixed;
      } else {
        aResult = nsRuleNode::eRuleFullReset;
      }
    }
  }

  // em, ex, and percentage values for font size require inheritance
  if ((fontData.mSize.IsRelativeLengthUnit() &&
       fontData.mSize.GetUnit() != eCSSUnit_Pixel) ||
      fontData.mSize.GetUnit() == eCSSUnit_Percent) {
    NS_ASSERTION(aResult == nsRuleNode::eRulePartialReset ||
                 aResult == nsRuleNode::eRuleFullReset ||
                 aResult == nsRuleNode::eRulePartialMixed ||
                 aResult == nsRuleNode::eRuleFullMixed,
                 "we know we already have a reset-counted property");
    // promote reset to mixed since we have something inherited
    if (aResult == nsRuleNode::eRulePartialReset)
      aResult = nsRuleNode::eRulePartialMixed;
    else if (aResult == nsRuleNode::eRuleFullReset)
      aResult = nsRuleNode::eRuleFullMixed;
  }

  return aResult;
}

PR_STATIC_CALLBACK(nsRuleNode::RuleDetail)
CheckColorCallback(const nsRuleDataStruct& aData,
                   nsRuleNode::RuleDetail aResult)
{
  const nsRuleDataColor& colorData =
      NS_STATIC_CAST(const nsRuleDataColor&, aData);

  // currentColor values for color require inheritance
  if (colorData.mColor.GetUnit() == eCSSUnit_Integer && 
      colorData.mColor.GetIntValue() == NS_COLOR_CURRENTCOLOR) {
    NS_ASSERTION(aResult == nsRuleNode::eRuleFullReset,
                 "we should already be counted as full-reset");
    aResult = nsRuleNode::eRuleFullInherited;
  }

  return aResult;
}


// for nsCSSPropList.h, so we get information on things in the style
// structs but not nsCSS*
#define CSS_PROP_INCLUDE_NOT_CSS

static const PropertyCheckData FontCheckProperties[] = {
#define CSS_PROP_FONT(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_FONT
};

static const PropertyCheckData DisplayCheckProperties[] = {
#define CSS_PROP_DISPLAY(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_DISPLAY
};

static const PropertyCheckData VisibilityCheckProperties[] = {
#define CSS_PROP_VISIBILITY(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_VISIBILITY
};

static const PropertyCheckData MarginCheckProperties[] = {
#define CSS_PROP_MARGIN(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_MARGIN
};

static const PropertyCheckData BorderCheckProperties[] = {
#define CSS_PROP_BORDER(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_BORDER
};

static const PropertyCheckData PaddingCheckProperties[] = {
#define CSS_PROP_PADDING(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_PADDING
};

static const PropertyCheckData OutlineCheckProperties[] = {
#define CSS_PROP_OUTLINE(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_OUTLINE
};

static const PropertyCheckData ListCheckProperties[] = {
#define CSS_PROP_LIST(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_LIST
};

static const PropertyCheckData ColorCheckProperties[] = {
#define CSS_PROP_COLOR(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_COLOR
};

static const PropertyCheckData BackgroundCheckProperties[] = {
#define CSS_PROP_BACKGROUND(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_BACKGROUND
};

static const PropertyCheckData PositionCheckProperties[] = {
#define CSS_PROP_POSITION(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_POSITION
};

static const PropertyCheckData TableCheckProperties[] = {
#define CSS_PROP_TABLE(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_TABLE
};

static const PropertyCheckData TableBorderCheckProperties[] = {
#define CSS_PROP_TABLEBORDER(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_TABLEBORDER
};

static const PropertyCheckData ContentCheckProperties[] = {
#define CSS_PROP_CONTENT(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_CONTENT
};

static const PropertyCheckData QuotesCheckProperties[] = {
#define CSS_PROP_QUOTES(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_QUOTES
};

static const PropertyCheckData TextCheckProperties[] = {
#define CSS_PROP_TEXT(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_TEXT
};

static const PropertyCheckData TextResetCheckProperties[] = {
#define CSS_PROP_TEXTRESET(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_TEXTRESET
};

static const PropertyCheckData UserInterfaceCheckProperties[] = {
#define CSS_PROP_USERINTERFACE(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_USERINTERFACE
};

static const PropertyCheckData UIResetCheckProperties[] = {
#define CSS_PROP_UIRESET(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_UIRESET
};

static const PropertyCheckData XULCheckProperties[] = {
#define CSS_PROP_XUL(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_XUL
};

#ifdef MOZ_SVG
static const PropertyCheckData SVGCheckProperties[] = {
#define CSS_PROP_SVG(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_SVG
};

static const PropertyCheckData SVGResetCheckProperties[] = {
#define CSS_PROP_SVGRESET(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_SVGRESET
};  
#endif

static const PropertyCheckData ColumnCheckProperties[] = {
#define CSS_PROP_COLUMN(name_, id_, method_, datastruct_, member_, type_, kwtable_) \
  { offsetof(nsRuleData##datastruct_, member_), type_ },
#include "nsCSSPropList.h"
#undef CSS_PROP_COLUMN
};

#undef CSS_PROP_INCLUDE_NOT_CSS
  
static const StructCheckData gCheckProperties[] = {

#define STYLE_STRUCT(name, checkdata_cb, ctor_args) \
  {name##CheckProperties, \
   sizeof(name##CheckProperties)/sizeof(PropertyCheckData), \
   checkdata_cb},
#include "nsStyleStructList.h"
#undef STYLE_STRUCT
  {nsnull, 0, nsnull}

};



// XXXldb Taking the address of a reference is evil.

inline const nsCSSValue&
ValueAtOffset(const nsRuleDataStruct& aRuleDataStruct, size_t aOffset)
{
  return * NS_REINTERPRET_CAST(const nsCSSValue*,
                     NS_REINTERPRET_CAST(const char*, &aRuleDataStruct) + aOffset);
}

inline const nsCSSRect*
RectAtOffset(const nsRuleDataStruct& aRuleDataStruct, size_t aOffset)
{
  return NS_REINTERPRET_CAST(const nsCSSRect*,
                     NS_REINTERPRET_CAST(const char*, &aRuleDataStruct) + aOffset);
}

inline const nsCSSValuePair*
ValuePairAtOffset(const nsRuleDataStruct& aRuleDataStruct, size_t aOffset)
{
  return NS_REINTERPRET_CAST(const nsCSSValuePair*,
                     NS_REINTERPRET_CAST(const char*, &aRuleDataStruct) + aOffset);
}

inline const nsCSSValueList*
ValueListAtOffset(const nsRuleDataStruct& aRuleDataStruct, size_t aOffset)
{
  return * NS_REINTERPRET_CAST(const nsCSSValueList*const*,
                     NS_REINTERPRET_CAST(const char*, &aRuleDataStruct) + aOffset);
}

inline const nsCSSValueList**
ValueListArrayAtOffset(const nsRuleDataStruct& aRuleDataStruct, size_t aOffset)
{
  return * NS_REINTERPRET_CAST(const nsCSSValueList**const*,
                     NS_REINTERPRET_CAST(const char*, &aRuleDataStruct) + aOffset);
}

inline const nsCSSCounterData*
CounterDataAtOffset(const nsRuleDataStruct& aRuleDataStruct, size_t aOffset)
{
  return * NS_REINTERPRET_CAST(const nsCSSCounterData*const*,
                     NS_REINTERPRET_CAST(const char*, &aRuleDataStruct) + aOffset);
}

inline const nsCSSQuotes*
QuotesAtOffset(const nsRuleDataStruct& aRuleDataStruct, size_t aOffset)
{
  return * NS_REINTERPRET_CAST(const nsCSSQuotes*const*,
                     NS_REINTERPRET_CAST(const char*, &aRuleDataStruct) + aOffset);
}

inline nsRuleNode::RuleDetail
nsRuleNode::CheckSpecifiedProperties(const nsStyleStructID aSID,
                                     const nsRuleDataStruct& aRuleDataStruct)
{
  const StructCheckData *structData = gCheckProperties + aSID;

  // Build a count of the:
  PRUint32 total = 0,      // total number of props in the struct
           specified = 0,  // number that were specified for this node
           inherited = 0;  // number that were 'inherit' (and not
                           //   eCSSUnit_Inherit) for this node

  for (const PropertyCheckData *prop = structData->props,
                           *prop_end = prop + structData->nprops;
       prop != prop_end;
       ++prop)
    switch (prop->type) {

      case eCSSType_Value:
        ++total;
        ExamineCSSValue(ValueAtOffset(aRuleDataStruct, prop->offset),
                        specified, inherited);
        break;

      case eCSSType_Rect:
        total += 4;
        ExamineCSSRect(RectAtOffset(aRuleDataStruct, prop->offset),
                       specified, inherited);
        break;

      case eCSSType_ValuePair:
        total += 2;
        ExamineCSSValuePair(ValuePairAtOffset(aRuleDataStruct, prop->offset),
                            specified, inherited);
        break;
        
      case eCSSType_ValueList:
        {
          ++total;
          const nsCSSValueList* valueList =
              ValueListAtOffset(aRuleDataStruct, prop->offset);
          if (valueList) {
            ++specified;
            if (eCSSUnit_Inherit == valueList->mValue.GetUnit()) {
              ++inherited;
            }
          }
        }
        break;

      case eCSSType_CounterData:
        {
          ++total;
          const nsCSSCounterData* counterData =
              CounterDataAtOffset(aRuleDataStruct, prop->offset);
          if (counterData) {
            ++specified;
            if (eCSSUnit_Inherit == counterData->mCounter.GetUnit()) {
              ++inherited;
            }
          }
        }
        break;

      case eCSSType_Quotes:
        {
          ++total;
          const nsCSSQuotes* quotes =
              QuotesAtOffset(aRuleDataStruct, prop->offset);
          if (quotes) {
            ++specified;
            if (eCSSUnit_Inherit == quotes->mOpen.GetUnit()) {
              ++inherited;
            }
          }
        }
        break;

      default:
        NS_NOTREACHED("unknown type");
        break;

    }

#if 0
  printf("CheckSpecifiedProperties: SID=%d total=%d spec=%d inh=%d.\n",
         aSID, total, specified, inherited);
#endif

  /*
   * Return the most specific information we can: prefer None or Full
   * over Partial, and Reset or Inherited over Mixed, since we can
   * optimize based on the edge cases and not the in-between cases.
   */
  nsRuleNode::RuleDetail result;
  if (inherited == total)
    result = eRuleFullInherited;
  else if (specified == total) {
    if (inherited == 0)
      result = eRuleFullReset;
    else
      result = eRuleFullMixed;
  } else if (specified == 0)
    result = eRuleNone;
  else if (specified == inherited)
    result = eRulePartialInherited;
  else if (inherited == 0)
    result = eRulePartialReset;
  else
    result = eRulePartialMixed;

  if (structData->callback) {
    result = (*structData->callback)(aRuleDataStruct, result);
  }

  return result;
}

const nsStyleStruct*
nsRuleNode::GetDisplayData(nsStyleContext* aContext)
{
  nsRuleDataDisplay displayData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Display, mPresContext, aContext);
  ruleData.mDisplayData = &displayData;

  return WalkRuleTree(eStyleStruct_Display, aContext, &ruleData, &displayData);
}

const nsStyleStruct*
nsRuleNode::GetVisibilityData(nsStyleContext* aContext)
{
  nsRuleDataDisplay displayData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Visibility, mPresContext, aContext);
  ruleData.mDisplayData = &displayData;

  return WalkRuleTree(eStyleStruct_Visibility, aContext, &ruleData, &displayData);
}

const nsStyleStruct*
nsRuleNode::GetTextData(nsStyleContext* aContext)
{
  nsRuleDataText textData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Text, mPresContext, aContext);
  ruleData.mTextData = &textData;

  return WalkRuleTree(eStyleStruct_Text, aContext, &ruleData, &textData);
}

const nsStyleStruct*
nsRuleNode::GetTextResetData(nsStyleContext* aContext)
{
  nsRuleDataText textData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_TextReset, mPresContext, aContext);
  ruleData.mTextData = &textData;

  const nsStyleStruct* res = WalkRuleTree(eStyleStruct_TextReset, aContext, &ruleData, &textData);
  textData.mTextShadow = nsnull; // We are sharing with some style rule.  It really owns the data.
  return res;
}

const nsStyleStruct*
nsRuleNode::GetUserInterfaceData(nsStyleContext* aContext)
{
  nsRuleDataUserInterface uiData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_UserInterface, mPresContext, aContext);
  ruleData.mUserInterfaceData = &uiData;

  const nsStyleStruct* res = WalkRuleTree(eStyleStruct_UserInterface, aContext, &ruleData, &uiData);
  uiData.mCursor = nsnull; // We are sharing with some style rule.  It really owns the data.
  return res;
}

const nsStyleStruct*
nsRuleNode::GetUIResetData(nsStyleContext* aContext)
{
  nsRuleDataUserInterface uiData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_UIReset, mPresContext, aContext);
  ruleData.mUserInterfaceData = &uiData;

  const nsStyleStruct* res = WalkRuleTree(eStyleStruct_UIReset, aContext, &ruleData, &uiData);
  return res;
}

const nsStyleStruct*
nsRuleNode::GetFontData(nsStyleContext* aContext)
{
  nsRuleDataFont fontData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Font, mPresContext, aContext);
  ruleData.mFontData = &fontData;

  return WalkRuleTree(eStyleStruct_Font, aContext, &ruleData, &fontData);
}

const nsStyleStruct*
nsRuleNode::GetColorData(nsStyleContext* aContext)
{
  nsRuleDataColor colorData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Color, mPresContext, aContext);
  ruleData.mColorData = &colorData;

  return WalkRuleTree(eStyleStruct_Color, aContext, &ruleData, &colorData);
}

const nsStyleStruct*
nsRuleNode::GetBackgroundData(nsStyleContext* aContext)
{
  nsRuleDataColor colorData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Background, mPresContext, aContext);
  ruleData.mColorData = &colorData;

  return WalkRuleTree(eStyleStruct_Background, aContext, &ruleData, &colorData);
}

const nsStyleStruct*
nsRuleNode::GetMarginData(nsStyleContext* aContext)
{
  nsRuleDataMargin marginData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Margin, mPresContext, aContext);
  ruleData.mMarginData = &marginData;

  return WalkRuleTree(eStyleStruct_Margin, aContext, &ruleData, &marginData);
}

const nsStyleStruct*
nsRuleNode::GetBorderData(nsStyleContext* aContext)
{
  nsRuleDataMargin marginData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Border, mPresContext, aContext);
  ruleData.mMarginData = &marginData;

  return WalkRuleTree(eStyleStruct_Border, aContext, &ruleData, &marginData);
}

const nsStyleStruct*
nsRuleNode::GetPaddingData(nsStyleContext* aContext)
{
  nsRuleDataMargin marginData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Padding, mPresContext, aContext);
  ruleData.mMarginData = &marginData;

  return WalkRuleTree(eStyleStruct_Padding, aContext, &ruleData, &marginData);
}

const nsStyleStruct*
nsRuleNode::GetOutlineData(nsStyleContext* aContext)
{
  nsRuleDataMargin marginData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Outline, mPresContext, aContext);
  ruleData.mMarginData = &marginData;

  return WalkRuleTree(eStyleStruct_Outline, aContext, &ruleData, &marginData);
}

const nsStyleStruct*
nsRuleNode::GetListData(nsStyleContext* aContext)
{
  nsRuleDataList listData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_List, mPresContext, aContext);
  ruleData.mListData = &listData;

  return WalkRuleTree(eStyleStruct_List, aContext, &ruleData, &listData);
}

const nsStyleStruct*
nsRuleNode::GetPositionData(nsStyleContext* aContext)
{
  nsRuleDataPosition posData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Position, mPresContext, aContext);
  ruleData.mPositionData = &posData;

  return WalkRuleTree(eStyleStruct_Position, aContext, &ruleData, &posData);
}

const nsStyleStruct*
nsRuleNode::GetTableData(nsStyleContext* aContext)
{
  nsRuleDataTable tableData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Table, mPresContext, aContext);
  ruleData.mTableData = &tableData;

  return WalkRuleTree(eStyleStruct_Table, aContext, &ruleData, &tableData);
}

const nsStyleStruct*
nsRuleNode::GetTableBorderData(nsStyleContext* aContext)
{
  nsRuleDataTable tableData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_TableBorder, mPresContext, aContext);
  ruleData.mTableData = &tableData;

  return WalkRuleTree(eStyleStruct_TableBorder, aContext, &ruleData, &tableData);
}

const nsStyleStruct*
nsRuleNode::GetContentData(nsStyleContext* aContext)
{
  nsRuleDataContent contentData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Content, mPresContext, aContext);
  ruleData.mContentData = &contentData;

  const nsStyleStruct* res = WalkRuleTree(eStyleStruct_Content, aContext, &ruleData, &contentData);
  contentData.mCounterIncrement = contentData.mCounterReset = nsnull;
  contentData.mContent = nsnull; // We are sharing with some style rule.  It really owns the data.
  return res;
}

const nsStyleStruct*
nsRuleNode::GetQuotesData(nsStyleContext* aContext)
{
  nsRuleDataContent contentData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Quotes, mPresContext, aContext);
  ruleData.mContentData = &contentData;

  const nsStyleStruct* res = WalkRuleTree(eStyleStruct_Quotes, aContext, &ruleData, &contentData);
  contentData.mQuotes = nsnull; // We are sharing with some style rule.  It really owns the data.
  return res;
}

const nsStyleStruct*
nsRuleNode::GetXULData(nsStyleContext* aContext)
{
  nsRuleDataXUL xulData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_XUL, mPresContext, aContext);
  ruleData.mXULData = &xulData;

  return WalkRuleTree(eStyleStruct_XUL, aContext, &ruleData, &xulData);
}

const nsStyleStruct*
nsRuleNode::GetColumnData(nsStyleContext* aContext)
{
  nsRuleDataColumn columnData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_Column, mPresContext, aContext);
  ruleData.mColumnData = &columnData;

  return WalkRuleTree(eStyleStruct_Column, aContext, &ruleData, &columnData);
}

#ifdef MOZ_SVG
const nsStyleStruct*
nsRuleNode::GetSVGData(nsStyleContext* aContext)
{
  nsRuleDataSVG svgData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_SVG, mPresContext, aContext);
  ruleData.mSVGData = &svgData;

  const nsStyleStruct *res = WalkRuleTree(eStyleStruct_SVG, aContext, &ruleData, &svgData);
  svgData.mStrokeDasharray = nsnull; // We are sharing with some style rule.  It really owns the data.
  return res;
}

const nsStyleStruct*
nsRuleNode::GetSVGResetData(nsStyleContext* aContext)
{
  nsRuleDataSVG svgData; // Declare a struct with null CSS values.
  nsRuleData ruleData(eStyleStruct_SVGReset, mPresContext, aContext);
  ruleData.mSVGData = &svgData;

  return WalkRuleTree(eStyleStruct_SVGReset, aContext, &ruleData, &svgData);
}
#endif

const nsStyleStruct*
nsRuleNode::WalkRuleTree(const nsStyleStructID aSID,
                         nsStyleContext* aContext, 
                         nsRuleData* aRuleData,
                         nsRuleDataStruct* aSpecificData)
{
  // We start at the most specific rule in the tree.  
  nsStyleStruct* startStruct = nsnull;
  
  nsRuleNode* ruleNode = this;
  nsRuleNode* highestNode = nsnull; // The highest node in the rule tree
                                    // that has the same properties
                                    // specified for struct |aSID| as
                                    // |this| does.
  nsRuleNode* rootNode = this; // After the loop below, this will be the
                               // highest node that we've walked without
                               // finding cached data on the rule tree.
                               // If we don't find any cached data, it
                               // will be the root.  (XXX misnamed)
  RuleDetail detail = eRuleNone;
  PRUint32 bit = nsCachedStyleData::GetBitForSID(aSID);

  while (ruleNode) {
    // See if this rule node has cached the fact that the remaining
    // nodes along this path specify no data whatsoever.
    if (ruleNode->mNoneBits & bit)
      break;

    // If the dependent bit is set on a rule node for this struct, that
    // means its rule won't have any information to add, so skip it.
    // XXXldb I don't understand why we need to check |detail| here, but
    // we do.
    if (detail == eRuleNone)
      while (ruleNode->mDependentBits & bit) {
        NS_ASSERTION(ruleNode->mStyleData.GetStyleData(aSID) == nsnull,
                     "dependent bit with cached data makes no sense");
        // Climb up to the next rule in the tree (a less specific rule).
        rootNode = ruleNode;
        ruleNode = ruleNode->mParent;
        NS_ASSERTION(!(ruleNode->mNoneBits & bit), "can't have both bits set");
      }

    // Check for cached data after the inner loop above -- otherwise
    // we'll miss it.
    startStruct = ruleNode->mStyleData.GetStyleData(aSID);
    if (startStruct)
      break; // We found a rule with fully specified data.  We don't
             // need to go up the tree any further, since the remainder
             // of this branch has already been computed.

    // Ask the rule to fill in the properties that it specifies.
    nsIStyleRule *rule = ruleNode->mRule;
    if (rule)
      rule->MapRuleInfoInto(aRuleData);

    // Now we check to see how many properties have been specified by
    // the rules we've examined so far.
    RuleDetail oldDetail = detail;
    detail = CheckSpecifiedProperties(aSID, *aSpecificData);
  
    if (oldDetail == eRuleNone && detail != eRuleNone)
      highestNode = ruleNode;

    if (detail == eRuleFullReset ||
        detail == eRuleFullMixed ||
        detail == eRuleFullInherited)
      break; // We don't need to examine any more rules.  All properties
             // have been fully specified.

    // Climb up to the next rule in the tree (a less specific rule).
    rootNode = ruleNode;
    ruleNode = ruleNode->mParent;
  }

  PRBool isReset = nsCachedStyleData::IsReset(aSID);
  if (!highestNode)
    highestNode = rootNode;

  if (!aRuleData->mCanStoreInRuleTree)
    detail = eRulePartialMixed; // Treat as though some data is specified to avoid
                                // the optimizations and force data computation.

  if (detail == eRuleNone && startStruct && !aRuleData->mPostResolveCallback) {
    // We specified absolutely no rule information, but a parent rule in the tree
    // specified all the rule information.  We set a bit along the branch from our
    // node in the tree to the node that specified the data that tells nodes on that
    // branch that they never need to examine their rules for this particular struct type
    // ever again.
    PropagateDependentBit(bit, ruleNode);
    return startStruct;
  }
  else if (!startStruct && ((!isReset && (detail == eRuleNone || detail == eRulePartialInherited)) 
                             || detail == eRuleFullInherited)) {
    // We specified no non-inherited information and neither did any of
    // our parent rules.

    // We set a bit along the branch from the highest node (ruleNode)
    // down to our node (this) indicating that no non-inherited data was
    // specified.  This bit is guaranteed to be set already on the path
    // from the highest node to the root node in the case where
    // (detail == eRuleNone), which is the most common case here.
    // We must check |!isReset| because the Compute*Data functions for
    // reset structs wouldn't handle none bits correctly.
    if (highestNode != this && !isReset)
      PropagateNoneBit(bit, highestNode);
    
    // All information must necessarily be inherited from our parent style context.
    // In the absence of any computed data in the rule tree and with
    // no rules specified that didn't have values of 'inherit', we should check our parent.
    nsStyleContext* parentContext = aContext->GetParent();
    if (parentContext) {
      // We have a parent, and so we should just inherit from the parent.
      // Set the inherit bits on our context.  These bits tell the style context that
      // it never has to go back to the rule tree for data.  Instead the style context tree
      // should be walked to find the data.
      const nsStyleStruct* parentStruct = parentContext->GetStyleData(aSID);
      aContext->AddStyleBit(bit); // makes const_cast OK.
      aContext->SetStyle(aSID, NS_CONST_CAST(nsStyleStruct*, parentStruct));
      return parentStruct;
    }
    else
      // We are the root.  In the case of fonts, the default values just
      // come from the pres context.
      return SetDefaultOnRoot(aSID, aContext);
  }

  // We need to compute the data from the information that the rules specified.
  const nsStyleStruct* res;
#define STYLE_STRUCT_TEST aSID
#define STYLE_STRUCT(name, checkdata_cb, ctor_args)                           \
  res = Compute##name##Data(startStruct, *aSpecificData, aContext,            \
                      highestNode, detail, !aRuleData->mCanStoreInRuleTree);
#include "nsStyleStructList.h"
#undef STYLE_STRUCT
#undef STYLE_STRUCT_TEST

  // If we have a post-resolve callback, handle that now.
  if (aRuleData->mPostResolveCallback && (NS_LIKELY(res != nsnull)))
    (*aRuleData->mPostResolveCallback)((nsStyleStruct*)res, aRuleData);

  // Now return the result.
  return res;
}

static PRBool
IsChrome(nsPresContext* aPresContext)
{
  PRBool isChrome = PR_FALSE;
  nsCOMPtr<nsISupports> container = aPresContext->GetContainer();
  if (container) {
    nsresult result;
    nsCOMPtr<nsIDocShellTreeItem> docShell(do_QueryInterface(container, &result));
    if (NS_SUCCEEDED(result) && docShell) {
      PRInt32 docShellType;
      result = docShell->GetItemType(&docShellType);
      if (NS_SUCCEEDED(result)) {
        isChrome = nsIDocShellTreeItem::typeChrome == docShellType;
      }
    }
  }
  return isChrome;
}

const nsStyleStruct*
nsRuleNode::SetDefaultOnRoot(const nsStyleStructID aSID, nsStyleContext* aContext)
{
  switch (aSID) {
    case eStyleStruct_Font: 
    {
      nsStyleFont* fontData = new (mPresContext) nsStyleFont(mPresContext);
      if (NS_LIKELY(fontData != nsnull)) {
        nscoord minimumFontSize =
          mPresContext->GetCachedIntPref(kPresContext_MinimumFontSize);

        if (minimumFontSize > 0 && !IsChrome(mPresContext)) {
          fontData->mFont.size = PR_MAX(fontData->mSize, minimumFontSize);
        }
        else {
          fontData->mFont.size = fontData->mSize;
        }
        aContext->SetStyle(eStyleStruct_Font, fontData);
      }
      return fontData;
    }
    case eStyleStruct_Display:
    {
      nsStyleDisplay* disp = new (mPresContext) nsStyleDisplay();
      if (NS_LIKELY(disp != nsnull)) {
        aContext->SetStyle(eStyleStruct_Display, disp);
      }
      return disp;
    }
    case eStyleStruct_Visibility:
    {
      nsStyleVisibility* vis = new (mPresContext) nsStyleVisibility(mPresContext);
      if (NS_LIKELY(vis != nsnull)) {
        aContext->SetStyle(eStyleStruct_Visibility, vis);
      }
      return vis;
    }
    case eStyleStruct_Text:
    {
      nsStyleText* text = new (mPresContext) nsStyleText();
      if (NS_LIKELY(text != nsnull)) {
        aContext->SetStyle(eStyleStruct_Text, text);
      }
      return text;
    }
    case eStyleStruct_TextReset:
    {
      nsStyleTextReset* text = new (mPresContext) nsStyleTextReset();
      if (NS_LIKELY(text != nsnull)) {
        aContext->SetStyle(eStyleStruct_TextReset, text);
      }
      return text;
    }
    case eStyleStruct_Color:
    {
      nsStyleColor* color = new (mPresContext) nsStyleColor(mPresContext);
      if (NS_LIKELY(color != nsnull)) {
        aContext->SetStyle(eStyleStruct_Color, color);
      }
      return color;
    }
    case eStyleStruct_Background:
    {
      nsStyleBackground* bg = new (mPresContext) nsStyleBackground(mPresContext);
      if (NS_LIKELY(bg != nsnull)) {
        aContext->SetStyle(eStyleStruct_Background, bg);
      }
      return bg;
    }
    case eStyleStruct_Margin:
    {
      nsStyleMargin* margin = new (mPresContext) nsStyleMargin();
      if (NS_LIKELY(margin != nsnull)) {
        aContext->SetStyle(eStyleStruct_Margin, margin);
      }
      return margin;
    }
    case eStyleStruct_Border:
    {
      nsStyleBorder* border = new (mPresContext) nsStyleBorder(mPresContext);
      if (NS_LIKELY(border != nsnull)) {
        aContext->SetStyle(eStyleStruct_Border, border);
      }
      return border;
    }
    case eStyleStruct_Padding:
    {
      nsStylePadding* padding = new (mPresContext) nsStylePadding();
      if (NS_LIKELY(padding != nsnull)) {
        aContext->SetStyle(eStyleStruct_Padding, padding);
      }
      return padding;
    }
    case eStyleStruct_Outline:
    {
      nsStyleOutline* outline = new (mPresContext) nsStyleOutline(mPresContext);
      if (NS_LIKELY(outline != nsnull)) {
        aContext->SetStyle(eStyleStruct_Outline, outline);
      }
      return outline;
    }
    case eStyleStruct_List:
    {
      nsStyleList* list = new (mPresContext) nsStyleList();
      if (NS_LIKELY(list != nsnull)) {
        aContext->SetStyle(eStyleStruct_List, list);
      }
      return list;
    }
    case eStyleStruct_Position:
    {
      nsStylePosition* pos = new (mPresContext) nsStylePosition();
      if (NS_LIKELY(pos != nsnull)) {
        aContext->SetStyle(eStyleStruct_Position, pos);
      }
      return pos;
    }
    case eStyleStruct_Table:
    {
      nsStyleTable* table = new (mPresContext) nsStyleTable();
      if (NS_LIKELY(table != nsnull)) {
        aContext->SetStyle(eStyleStruct_Table, table);
      }
      return table;
    }
    case eStyleStruct_TableBorder:
    {
      nsStyleTableBorder* table = new (mPresContext) nsStyleTableBorder(mPresContext);
      if (NS_LIKELY(table != nsnull)) {
        aContext->SetStyle(eStyleStruct_TableBorder, table);
      }
      return table;
    }
    case eStyleStruct_Content:
    {
      nsStyleContent* content = new (mPresContext) nsStyleContent();
      if (NS_LIKELY(content != nsnull)) {
        aContext->SetStyle(eStyleStruct_Content, content);
      }
      return content;
    }
    case eStyleStruct_Quotes:
    {
      nsStyleQuotes* quotes = new (mPresContext) nsStyleQuotes();
      if (NS_LIKELY(quotes != nsnull)) {
        aContext->SetStyle(eStyleStruct_Quotes, quotes);
      }
      return quotes;
    }
    case eStyleStruct_UserInterface:
    {
      nsStyleUserInterface* ui = new (mPresContext) nsStyleUserInterface();
      if (NS_LIKELY(ui != nsnull)) {
        aContext->SetStyle(eStyleStruct_UserInterface, ui);
      }
      return ui;
    }
    case eStyleStruct_UIReset:
    {
      nsStyleUIReset* ui = new (mPresContext) nsStyleUIReset();
      if (NS_LIKELY(ui != nsnull)) {
        aContext->SetStyle(eStyleStruct_UIReset, ui);
      }
      return ui;
    }

    case eStyleStruct_XUL:
    {
      nsStyleXUL* xul = new (mPresContext) nsStyleXUL();
      if (NS_LIKELY(xul != nsnull)) {
        aContext->SetStyle(eStyleStruct_XUL, xul);
      }
      return xul;
    }

    case eStyleStruct_Column:
    {
      nsStyleColumn* column = new (mPresContext) nsStyleColumn();
      if (NS_LIKELY(column != nsnull)) {
        aContext->SetStyle(eStyleStruct_Column, column);
      }
      return column;
    }

#ifdef MOZ_SVG
    case eStyleStruct_SVG:
    {
      nsStyleSVG* svg = new (mPresContext) nsStyleSVG();
      if (NS_LIKELY(svg != nsnull)) {
        aContext->SetStyle(eStyleStruct_SVG, svg);
      }
      return svg;
    }

    case eStyleStruct_SVGReset:
    {
      nsStyleSVGReset* svgReset = new (mPresContext) nsStyleSVGReset();
      if (NS_LIKELY(svgReset != nsnull)) {
        aContext->SetStyle(eStyleStruct_SVGReset, svgReset);
      }
      return svgReset;
    }
#endif
  }
  return nsnull;
}

/*
 * This function handles cascading of *-left or *-right box properties
 * against *-start (which is L for LTR and R for RTL) or *-end (which is
 * R for LTR and L for RTL).
 *
 * Cascading these properties correctly is hard because we need to
 * cascade two properties as one, but which two properties depends on a
 * third property ('direction').  We solve this by treating each of
 * these properties (say, 'margin-start') as a shorthand that sets a
 * property containing the value of the property specified
 * ('margin-start-value') and sets a pair of properties
 * ('margin-left-ltr-source' and 'margin-right-rtl-source') saying which
 * of the properties we use.  Thus, when we want to compute the value of
 * 'margin-left' when 'direction' is 'ltr', we look at the value of
 * 'margin-left-ltr-source', which tells us whether to use the highest
 * 'margin-left' in the cascade or the highest 'margin-start'.
 *
 * Finally, since we can compute the normal (*-left and *-right)
 * properties in a loop, this function works by assuming the computation
 * for those properties has happened as though we have not implemented
 * the logical properties (*-start and *-end).  It is the responsibility
 * of this function to replace the computed values with the values
 * computed from the logical properties when needed.
 */
void
nsRuleNode::AdjustLogicalBoxProp(nsStyleContext* aContext,
                                 const nsCSSValue& aLTRSource,
                                 const nsCSSValue& aRTLSource,
                                 const nsCSSValue& aLTRLogicalValue,
                                 const nsCSSValue& aRTLLogicalValue,
                                 const nsStyleSides& aParentRect,
                                 nsStyleSides& aRect,
                                 PRUint8 aSide,
                                 PRInt32 aMask,
                                 PRBool& aInherited)
{
  PRBool LTRlogical = aLTRSource.GetUnit() == eCSSUnit_Enumerated &&
                      aLTRSource.GetIntValue() == NS_BOXPROP_SOURCE_LOGICAL;
  PRBool RTLlogical = aRTLSource.GetUnit() == eCSSUnit_Enumerated &&
                      aRTLSource.GetIntValue() == NS_BOXPROP_SOURCE_LOGICAL;
  if (LTRlogical || RTLlogical) {
    // We can't cache anything on the rule tree if we use any data from
    // the style context, since data cached in the rule tree could be
    // used with a style context with a different value.
    aInherited = PR_TRUE;
    PRUint8 dir = aContext->GetStyleVisibility()->mDirection;

    nsStyleCoord parentCoord;
    nsStyleCoord coord;
    aParentRect.Get(aSide, parentCoord);
    if (dir == NS_STYLE_DIRECTION_LTR) {
      if (LTRlogical &&
          SetCoord(aLTRLogicalValue, coord, parentCoord, aMask, aContext,
                   mPresContext, aInherited))
        aRect.Set(aSide, coord);
    } else {
      if (RTLlogical &&
          SetCoord(aRTLLogicalValue, coord, parentCoord, aMask, aContext,
                   mPresContext, aInherited))
        aRect.Set(aSide, coord);
    }
  }
}

/**
 * Begin an nsRuleNode::Compute*Data function for an inherited struct.
 *
 * @param type_ The nsStyle* type this function computes.
 * @param ctorargs_ The arguments used for the default nsStyle* constructor.
 * @param data_ Variable (declared here) holding the result of this
 *              function.
 * @param parentdata_ Variable (declared here) holding the parent style
 *                    context's data for this struct.
 * @param rdtype_ The nsCSS* struct type used to compute this struct's data.
 * @param rdata_ Variable (declared here) holding the nsCSS* used here.
 */
#define COMPUTE_START_INHERITED(type_, ctorargs_, data_, parentdata_, rdtype_, rdata_) \
  nsStyleContext* parentContext = aContext->GetParent();                      \
                                                                              \
  const nsRuleData##rdtype_& rdata_ =                                         \
    NS_STATIC_CAST(const nsRuleData##rdtype_&, aData);                        \
  nsStyle##type_* data_ = nsnull;                                             \
  const nsStyle##type_* parentdata_ = nsnull;                                 \
  PRBool inherited = aInherited;                                              \
                                                                              \
  if (parentContext && aRuleDetail != eRuleFullReset)                         \
    parentdata_ = parentContext->GetStyle##type_();                           \
  if (aStartStruct)                                                           \
    /* We only need to compute the delta between this computed data and */    \
    /* our computed data. */                                                  \
    data_ = new (mPresContext)                                                \
            nsStyle##type_(*NS_STATIC_CAST(nsStyle##type_*, aStartStruct));   \
  else {                                                                      \
    /* XXXldb What about eRuleFullInherited?  Which path is faster? */        \
    if (aRuleDetail != eRuleFullMixed && aRuleDetail != eRuleFullReset) {     \
      /* No question. We will have to inherit. Go ahead and init */           \
      /* with inherited vals from parent. */                                  \
      inherited = PR_TRUE;                                                    \
      if (parentdata_)                                                        \
        data_ = new (mPresContext) nsStyle##type_(*parentdata_);              \
      else                                                                    \
        data_ = new (mPresContext) nsStyle##type_ ctorargs_;                  \
    }                                                                         \
    else                                                                      \
      data_ = new (mPresContext) nsStyle##type_ ctorargs_;                    \
  }                                                                           \
                                                                              \
  if (NS_UNLIKELY(!data_))                                                    \
    return nsnull;  /* Out Of Memory */                                       \
  if (!parentdata_)                                                           \
    parentdata_ = data_;

/**
 * Begin an nsRuleNode::Compute*Data function for a reset struct.
 *
 * @param type_ The nsStyle* type this function computes.
 * @param ctorargs_ The arguments used for the default nsStyle* constructor.
 * @param data_ Variable (declared here) holding the result of this
 *              function.
 * @param parentdata_ Variable (declared here) holding the parent style
 *                    context's data for this struct.
 * @param rdtype_ The nsCSS* struct type used to compute this struct's data.
 * @param rdata_ Variable (declared here) holding the nsCSS* used here.
 */
#define COMPUTE_START_RESET(type_, ctorargs_, data_, parentdata_, rdtype_, rdata_) \
  nsStyleContext* parentContext = aContext->GetParent();                      \
                                                                              \
  const nsRuleData##rdtype_& rdata_ =                                         \
    NS_STATIC_CAST(const nsRuleData##rdtype_&, aData);                        \
  nsStyle##type_* data_;                                                      \
  if (aStartStruct)                                                           \
    /* We only need to compute the delta between this computed data and */    \
    /* our computed data. */                                                  \
    data_ = new (mPresContext)                                                \
            nsStyle##type_(*NS_STATIC_CAST(nsStyle##type_*, aStartStruct));   \
  else                                                                        \
    data_ = new (mPresContext) nsStyle##type_ ctorargs_;                      \
                                                                              \
  if (NS_UNLIKELY(!data_))                                                    \
    return nsnull;  /* Out Of Memory */                                       \
                                                                              \
  const nsStyle##type_* parentdata_ = data_;                                  \
  if (parentContext &&                                                        \
      aRuleDetail != eRuleFullReset &&                                        \
      aRuleDetail != eRulePartialReset &&                                     \
      aRuleDetail != eRuleNone)                                               \
    parentdata_ = parentContext->GetStyle##type_();                           \
  PRBool inherited = aInherited;

/**
 * Begin an nsRuleNode::Compute*Data function for an inherited struct.
 *
 * @param type_ The nsStyle* type this function computes.
 * @param data_ Variable holding the result of this function.
 */
#define COMPUTE_END_INHERITED(type_, data_)                                   \
  if (inherited)                                                              \
    /* We inherited, and therefore can't be cached in the rule node.  We */   \
    /* have to be put right on the style context. */                          \
    aContext->SetStyle(eStyleStruct_##type_, data_);                          \
  else {                                                                      \
    /* We were fully specified and can therefore be cached right on the */    \
    /* rule node. */                                                          \
    if (!aHighestNode->mStyleData.mInheritedData) {                           \
      aHighestNode->mStyleData.mInheritedData =                               \
        new (mPresContext) nsInheritedStyleData;                              \
      if (NS_UNLIKELY(!aHighestNode->mStyleData.mInheritedData)) {            \
        data_->Destroy(mPresContext);                                         \
        return nsnull;                                                        \
      }                                                                       \
    }                                                                         \
    aHighestNode->mStyleData.mInheritedData->m##type_##Data = data_;          \
    /* Propagate the bit down. */                                             \
    PropagateDependentBit(NS_STYLE_INHERIT_BIT(type_), aHighestNode);         \
  }                                                                           \
                                                                              \
  return data_;

/**
 * Begin an nsRuleNode::Compute*Data function for a reset struct.
 *
 * @param type_ The nsStyle* type this function computes.
 * @param data_ Variable holding the result of this function.
 */
#define COMPUTE_END_RESET(type_, data_)                                       \
  if (inherited)                                                              \
    /* We inherited, and therefore can't be cached in the rule node.  We */   \
    /* have to be put right on the style context. */                          \
    aContext->SetStyle(eStyleStruct_##type_, data_);                          \
  else {                                                                      \
    /* We were fully specified and can therefore be cached right on the */    \
    /* rule node. */                                                          \
    if (!aHighestNode->mStyleData.mResetData) {                               \
      aHighestNode->mStyleData.mResetData =                                   \
        new (mPresContext) nsResetStyleData;                                  \
      if (NS_UNLIKELY(!aHighestNode->mStyleData.mResetData)) {                \
        data_->Destroy(mPresContext);                                         \
        return nsnull;                                                        \
      }                                                                       \
    }                                                                         \
    aHighestNode->mStyleData.mResetData->m##type_##Data = data_;              \
    /* Propagate the bit down. */                                             \
    PropagateDependentBit(NS_STYLE_INHERIT_BIT(type_), aHighestNode);         \
  }                                                                           \
                                                                              \
  return data_;
  
/* static */ void
nsRuleNode::SetFont(nsPresContext* aPresContext, nsStyleContext* aContext,
                    nscoord aMinFontSize,
                    PRBool aIsGeneric, const nsRuleDataFont& aFontData,
                    const nsFont& aDefaultFont, const nsStyleFont* aParentFont,
                    nsStyleFont* aFont, PRBool& aInherited)
{
  const nsFont* defaultVariableFont =
    aPresContext->GetDefaultFont(kPresContext_DefaultVariableFont_ID);

  // font-family: string list, enum, inherit
  if (eCSSUnit_String == aFontData.mFamily.GetUnit()) {
    // set the correct font if we are using DocumentFonts OR we are overriding for XUL
    // MJA: bug 31816
    if (!aIsGeneric) {
      // only bother appending fallback fonts if this isn't a fallback generic font itself
      if (!aFont->mFont.name.IsEmpty())
        aFont->mFont.name.Append((PRUnichar)',');
      // XXXldb Should this name be quoted?
      aFont->mFont.name.Append(aDefaultFont.name);
    }
    aFont->mFont.familyNameQuirks =
        (aPresContext->CompatibilityMode() == eCompatibility_NavQuirks &&
         aFontData.mFamilyFromHTML);
  }
  else if (eCSSUnit_Enumerated == aFontData.mFamily.GetUnit()) {
    nsSystemFontID sysID;
    switch (aFontData.mFamily.GetIntValue()) {
      // If you add fonts to this list, you need to also patch the list
      // in CheckFontCallback (also in this file).
      case NS_STYLE_FONT_CAPTION:       sysID = eSystemFont_Caption;      break;    // css2
      case NS_STYLE_FONT_ICON:          sysID = eSystemFont_Icon;         break;
      case NS_STYLE_FONT_MENU:          sysID = eSystemFont_Menu;         break;
      case NS_STYLE_FONT_MESSAGE_BOX:   sysID = eSystemFont_MessageBox;   break;
      case NS_STYLE_FONT_SMALL_CAPTION: sysID = eSystemFont_SmallCaption; break;
      case NS_STYLE_FONT_STATUS_BAR:    sysID = eSystemFont_StatusBar;    break;
      case NS_STYLE_FONT_WINDOW:        sysID = eSystemFont_Window;       break;    // css3
      case NS_STYLE_FONT_DOCUMENT:      sysID = eSystemFont_Document;     break;
      case NS_STYLE_FONT_WORKSPACE:     sysID = eSystemFont_Workspace;    break;
      case NS_STYLE_FONT_DESKTOP:       sysID = eSystemFont_Desktop;      break;
      case NS_STYLE_FONT_INFO:          sysID = eSystemFont_Info;         break;
      case NS_STYLE_FONT_DIALOG:        sysID = eSystemFont_Dialog;       break;
      case NS_STYLE_FONT_BUTTON:        sysID = eSystemFont_Button;       break;
      case NS_STYLE_FONT_PULL_DOWN_MENU:sysID = eSystemFont_PullDownMenu; break;
      case NS_STYLE_FONT_LIST:          sysID = eSystemFont_List;         break;
      case NS_STYLE_FONT_FIELD:         sysID = eSystemFont_Field;        break;
    }

    // GetSystemFont sets the font face but not necessarily the size
    aFont->mFont.size = defaultVariableFont->size;

    if (NS_FAILED(aPresContext->DeviceContext()->GetSystemFont(sysID,
                                                             &aFont->mFont))) {
        aFont->mFont.name = defaultVariableFont->name;
    }
    // this becomes our cascading size
    aFont->mSize = aFont->mFont.size =
      nsStyleFont::ZoomText(aPresContext, aFont->mFont.size);

    aFont->mFont.familyNameQuirks = PR_FALSE;

    // XXXldb All of this platform-specific stuff should be in the
    // nsIDeviceContext implementations, not here.

#ifdef XP_WIN
    //
    // As far as I can tell the system default fonts and sizes for
    // on MS-Windows for Buttons, Listboxes/Comboxes and Text Fields are 
    // all pre-determined and cannot be changed by either the control panel 
    // or programmtically.
    //
    switch (sysID) {
      // Fields (text fields)
      // Button and Selects (listboxes/comboboxes)
      //    We use whatever font is defined by the system. Which it appears
      //    (and the assumption is) it is always a proportional font. Then we
      //    always use 2 points smaller than what the browser has defined as
      //    the default proportional font.
      case eSystemFont_Field:
      case eSystemFont_Button:
      case eSystemFont_List:
        // Assumption: system defined font is proportional
        aFont->mSize = nsStyleFont::ZoomText(aPresContext,
             PR_MAX(defaultVariableFont->size - NSIntPointsToTwips(2), 0));
        break;
    }
#endif
  }
  else if (eCSSUnit_Inherit == aFontData.mFamily.GetUnit()) {
    aInherited = PR_TRUE;
    aFont->mFont.name = aParentFont->mFont.name;
    aFont->mFont.familyNameQuirks = aParentFont->mFont.familyNameQuirks;
  }
  else if (eCSSUnit_Initial == aFontData.mFamily.GetUnit()) {
    aFont->mFont.name = aDefaultFont.name;
    aFont->mFont.familyNameQuirks = PR_FALSE;
  }

  // font-style: enum, normal, inherit
  if (eCSSUnit_Enumerated == aFontData.mStyle.GetUnit()) {
    aFont->mFont.style = aFontData.mStyle.GetIntValue();
  }
  else if (eCSSUnit_Normal == aFontData.mStyle.GetUnit()) {
    aFont->mFont.style = NS_STYLE_FONT_STYLE_NORMAL;
  }
  else if (eCSSUnit_Inherit == aFontData.mStyle.GetUnit()) {
    aInherited = PR_TRUE;
    aFont->mFont.style = aParentFont->mFont.style;
  }
  else if (eCSSUnit_Initial == aFontData.mStyle.GetUnit()) {
    aFont->mFont.style = aDefaultFont.style;
  }

  // font-variant: enum, normal, inherit
  if (eCSSUnit_Enumerated == aFontData.mVariant.GetUnit()) {
    aFont->mFont.variant = aFontData.mVariant.GetIntValue();
  }
  else if (eCSSUnit_Normal == aFontData.mVariant.GetUnit()) {
    aFont->mFont.variant = NS_STYLE_FONT_VARIANT_NORMAL;
  }
  else if (eCSSUnit_Inherit == aFontData.mVariant.GetUnit()) {
    aInherited = PR_TRUE;
    aFont->mFont.variant = aParentFont->mFont.variant;
  }
  else if (eCSSUnit_Initial == aFontData.mVariant.GetUnit()) {
    aFont->mFont.variant = aDefaultFont.variant;
  }

  // font-weight: int, enum, normal, inherit
  if (eCSSUnit_Integer == aFontData.mWeight.GetUnit()) {
    aFont->mFont.weight = aFontData.mWeight.GetIntValue();
  }
  else if (eCSSUnit_Enumerated == aFontData.mWeight.GetUnit()) {
    PRInt32 value = aFontData.mWeight.GetIntValue();
    switch (value) {
      case NS_STYLE_FONT_WEIGHT_NORMAL:
      case NS_STYLE_FONT_WEIGHT_BOLD:
        aFont->mFont.weight = value;
        break;
      case NS_STYLE_FONT_WEIGHT_BOLDER:
      case NS_STYLE_FONT_WEIGHT_LIGHTER:
        aInherited = PR_TRUE;
        aFont->mFont.weight = nsStyleUtil::ConstrainFontWeight(aParentFont->mFont.weight + value);
        break;
    }
  }
  else if (eCSSUnit_Normal == aFontData.mWeight.GetUnit()) {
    aFont->mFont.weight = NS_STYLE_FONT_WEIGHT_NORMAL;
  }
  else if (eCSSUnit_Inherit == aFontData.mWeight.GetUnit()) {
    aInherited = PR_TRUE;
    aFont->mFont.weight = aParentFont->mFont.weight;
  }
  else if (eCSSUnit_Initial == aFontData.mWeight.GetUnit()) {
    aFont->mFont.weight = aDefaultFont.weight;
  }

  // font-size: enum, length, percent, inherit
  PRBool zoom = PR_FALSE;
  if (eCSSUnit_Enumerated == aFontData.mSize.GetUnit()) {
    PRInt32 value = aFontData.mSize.GetIntValue();
    PRInt32 scaler = aPresContext->FontScaler();
    float scaleFactor = nsStyleUtil::GetScalingFactor(scaler);

    zoom = PR_TRUE;
    if ((NS_STYLE_FONT_SIZE_XXSMALL <= value) && 
        (value <= NS_STYLE_FONT_SIZE_XXLARGE)) {
      aFont->mSize = nsStyleUtil::CalcFontPointSize(value, (PRInt32)aDefaultFont.size, scaleFactor, aPresContext, eFontSize_CSS);
    }
    else if (NS_STYLE_FONT_SIZE_XXXLARGE == value) {
      // <font size="7"> is not specified in CSS, so we don't use eFontSize_CSS.
      aFont->mSize = nsStyleUtil::CalcFontPointSize(value, (PRInt32)aDefaultFont.size, scaleFactor, aPresContext);
    }
    else if (NS_STYLE_FONT_SIZE_LARGER      == value ||
             NS_STYLE_FONT_SIZE_SMALLER     == value) {

      aInherited = PR_TRUE;

      // Un-zoom so we use the tables correctly.  We'll then rezoom due
      // to the |zoom = PR_TRUE| above.
      nscoord parentSize =
          nsStyleFont::UnZoomText(aPresContext, aParentFont->mSize);

      if (NS_STYLE_FONT_SIZE_LARGER == value) {
        aFont->mSize = nsStyleUtil::FindNextLargerFontSize(parentSize, (PRInt32)aDefaultFont.size,
                                                           scaleFactor, aPresContext, eFontSize_CSS);
        NS_ASSERTION(aFont->mSize > parentSize, "FindNextLargerFontSize failed.");
      } 
      else {
        aFont->mSize = nsStyleUtil::FindNextSmallerFontSize(parentSize, (PRInt32)aDefaultFont.size,
                                                            scaleFactor, aPresContext, eFontSize_CSS);
        NS_ASSERTION(aFont->mSize < parentSize, 
            "FindNextSmallerFontSize failed; this is expected if parentFont size <= 1px");
      }
    } else {
      NS_NOTREACHED("unexpected value");
    }
  }
  else if (aFontData.mSize.IsLengthUnit()) {
    aFont->mSize = CalcLength(aFontData.mSize, &aParentFont->mFont, nsnull, aPresContext, aInherited);
    zoom = aFontData.mSize.IsFixedLengthUnit() ||
           aFontData.mSize.GetUnit() == eCSSUnit_Pixel;
  }
  else if (eCSSUnit_Percent == aFontData.mSize.GetUnit()) {
    aInherited = PR_TRUE;
    aFont->mSize = NSToCoordRound(float(aParentFont->mSize) *
                                  aFontData.mSize.GetPercentValue());
    zoom = PR_FALSE;
  }
  else if (eCSSUnit_Inherit == aFontData.mSize.GetUnit()) {
    aInherited = PR_TRUE;
    aFont->mSize = aParentFont->mSize;
    zoom = PR_FALSE;
  }
  else if (eCSSUnit_Initial == aFontData.mSize.GetUnit()) {
    aFont->mSize = aDefaultFont.size;
    zoom = PR_TRUE;
  }

  // We want to zoom the cascaded size so that em-based measurements,
  // line-heights, etc., work.
  if (zoom)
    aFont->mSize = nsStyleFont::ZoomText(aPresContext, aFont->mSize);

  // enforce the user' specified minimum font-size on the value that we expose
  aFont->mFont.size = PR_MAX(aFont->mSize, aMinFontSize);

  // font-size-adjust: number, none, inherit
  if (eCSSUnit_Number == aFontData.mSizeAdjust.GetUnit()) {
    aFont->mFont.sizeAdjust = aFontData.mSizeAdjust.GetFloatValue();
  }
  else if (eCSSUnit_None == aFontData.mSizeAdjust.GetUnit()) {
    aFont->mFont.sizeAdjust = 0.0f;
  }
  else if (eCSSUnit_Inherit == aFontData.mSizeAdjust.GetUnit()) {
    aInherited = PR_TRUE;
    aFont->mFont.sizeAdjust = aParentFont->mFont.sizeAdjust;
  }
  else if (eCSSUnit_Initial == aFontData.mSizeAdjust.GetUnit()) {
    aFont->mFont.sizeAdjust = 0.0f;
  }
}

// SetGenericFont:
//  - backtrack to an ancestor with the same generic font name (possibly
//    up to the root where default values come from the presentation context)
//  - re-apply cascading rules from there without caching intermediate values
/* static */ void
nsRuleNode::SetGenericFont(nsPresContext* aPresContext,
                           nsStyleContext* aContext,
                           const nsRuleDataFont& aFontData,
                           PRUint8 aGenericFontID, nscoord aMinFontSize,
                           nsStyleFont* aFont)
{
  // walk up the contexts until a context with the desired generic font
  nsAutoVoidArray contextPath;
  nsStyleContext* higherContext = aContext->GetParent();
  while (higherContext) {
    if (higherContext->GetStyleFont()->mFlags & aGenericFontID) {
      // done walking up the higher contexts
      break;
    }
    contextPath.AppendElement(higherContext);
    higherContext = higherContext->GetParent();
  }

  // re-apply the cascading rules, starting from the higher context

  // If we stopped earlier because we reached the root of the style tree,
  // we will start with the default generic font from the presentation
  // context. Otherwise we start with the higher context.
  const nsFont* defaultFont = aPresContext->GetDefaultFont(aGenericFontID);
  nsStyleFont parentFont(*defaultFont);
  parentFont.mSize = parentFont.mFont.size
      = nsStyleFont::ZoomText(aPresContext, parentFont.mSize);
  if (higherContext) {
    const nsStyleFont* tmpFont = higherContext->GetStyleFont();
    parentFont.mFlags = tmpFont->mFlags;
    parentFont.mFont = tmpFont->mFont;
    parentFont.mSize = tmpFont->mSize;
  }
  aFont->mFlags = parentFont.mFlags;
  aFont->mFont = parentFont.mFont;
  aFont->mSize = parentFont.mSize;

  PRBool dummy;
  PRUint32 fontBit = nsCachedStyleData::GetBitForSID(eStyleStruct_Font);
  
  for (PRInt32 i = contextPath.Count() - 1; i >= 0; --i) {
    nsStyleContext* context = (nsStyleContext*)contextPath[i];
    nsRuleDataFont fontData; // Declare a struct with null CSS values.
    nsRuleData ruleData(eStyleStruct_Font, aPresContext, context);
    ruleData.mFontData = &fontData;

    // Trimmed down version of ::WalkRuleTree() to re-apply the style rules
    for (nsRuleNode* ruleNode = context->GetRuleNode(); ruleNode;
         ruleNode = ruleNode->GetParent()) {
      if (ruleNode->mNoneBits & fontBit)
        // no more font rules on this branch, get out
        break;

      nsIStyleRule *rule = ruleNode->GetRule();
      if (rule)
        rule->MapRuleInfoInto(&ruleData);
    }

    // Compute the delta from the information that the rules specified
    fontData.mFamily.Reset(); // avoid unnecessary operations in SetFont()

    nsRuleNode::SetFont(aPresContext, context, aMinFontSize,
                        PR_TRUE, fontData, *defaultFont,
                        &parentFont, aFont, dummy);

    // XXX Not sure if we need to do this here
    // If we have a post-resolve callback, handle that now.
    if (ruleData.mPostResolveCallback)
      (ruleData.mPostResolveCallback)((nsStyleStruct*)aFont, &ruleData);

    parentFont.mFlags = aFont->mFlags;
    parentFont.mFont = aFont->mFont;
    parentFont.mSize = aFont->mSize;
  }

  // Finish off by applying our own rules. In this case, aFontData
  // already has the current cascading information that we want. We
  // can just compute the delta from the parent.
  nsRuleNode::SetFont(aPresContext, aContext, aMinFontSize,
                      PR_TRUE, aFontData, *defaultFont,
                      &parentFont, aFont, dummy);
}

static PRBool ExtractGeneric(const nsString& aFamily, PRBool aGeneric,
                             void *aData)
{
  nsAutoString *data = NS_STATIC_CAST(nsAutoString*, aData);

  if (aGeneric) {
    *data = aFamily;
    return PR_FALSE; // stop enumeration
  }
  return PR_TRUE;
}

const nsStyleStruct* 
nsRuleNode::ComputeFontData(nsStyleStruct* aStartStruct,
                            const nsRuleDataStruct& aData, 
                            nsStyleContext* aContext, 
                            nsRuleNode* aHighestNode,
                            const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_INHERITED(Font, (mPresContext), font, parentFont,
                          Font, fontData)

  // See if there is a minimum font-size constraint to honor
  nscoord minimumFontSize = 
    mPresContext->GetCachedIntPref(kPresContext_MinimumFontSize);

  if (minimumFontSize < 0)
    minimumFontSize = 0;

  PRBool useDocumentFonts =
    mPresContext->GetCachedBoolPref(kPresContext_UseDocumentFonts);

  // See if we are in the chrome
  // We only need to know this to determine if we have to use the
  // document fonts (overriding the useDocumentFonts flag), or to
  // determine if we have to override the minimum font-size constraint.
  if ((!useDocumentFonts || minimumFontSize > 0) && IsChrome(mPresContext)) {
    // if we are not using document fonts, but this is a XUL document,
    // then we use the document fonts anyway
    useDocumentFonts = PR_TRUE;
    minimumFontSize = 0;
  }

  // Figure out if we are a generic font
  PRUint8 generic = kGenericFont_NONE;
  if (eCSSUnit_String == fontData.mFamily.GetUnit()) {
    fontData.mFamily.GetStringValue(font->mFont.name);
    // XXXldb Do we want to extract the generic for this if it's not only a
    // generic?
    nsFont::GetGenericID(font->mFont.name, &generic);

    // If we aren't allowed to use document fonts, then we are only entitled
    // to use the user's default variable-width font and fixed-width font
    if (!useDocumentFonts) {
      // Extract the generic from the specified font family...
      nsAutoString genericName;
      if (!font->mFont.EnumerateFamilies(ExtractGeneric, &genericName)) {
        // The specified font had a generic family.
        font->mFont.name = genericName;
        nsFont::GetGenericID(genericName, &generic);

        // ... and only use it if it's -moz-fixed or monospace
        if (generic != kGenericFont_moz_fixed &&
            generic != kGenericFont_monospace) {
          font->mFont.name.Truncate();
          generic = kGenericFont_NONE;
        }
      } else {
        // The specified font did not have a generic family.
        font->mFont.name.Truncate();
        generic = kGenericFont_NONE;
      }
    }
  }

  // Now compute our font struct
  if (generic == kGenericFont_NONE) {
    // continue the normal processing
    // our default font is the most recent generic font
    // XXXldb Probably should be the serif/sans-serif pref instead.
    const nsFont* defaultFont =
      mPresContext->GetDefaultFont(parentFont->mFlags & NS_STYLE_FONT_FACE_MASK);

    nsRuleNode::SetFont(mPresContext, aContext, minimumFontSize, PR_FALSE,
                        fontData, *defaultFont, parentFont, font, inherited);
  }
  else {
    // re-calculate the font as a generic font
    inherited = PR_TRUE;
    nsRuleNode::SetGenericFont(mPresContext, aContext, fontData, generic,
                               minimumFontSize, font);
  }
  // Set our generic font's bit to inform our descendants
  font->mFlags &= ~NS_STYLE_FONT_FACE_MASK;
  font->mFlags |= generic;

  COMPUTE_END_INHERITED(Font, font)
}

const nsStyleStruct*
nsRuleNode::ComputeTextData(nsStyleStruct* aStartStruct,
                            const nsRuleDataStruct& aData, 
                            nsStyleContext* aContext, 
                            nsRuleNode* aHighestNode,
                            const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_INHERITED(Text, (), text, parentText, Text, textData)

    // letter-spacing: normal, length, inherit
  SetCoord(textData.mLetterSpacing, text->mLetterSpacing, parentText->mLetterSpacing,
           SETCOORD_LH | SETCOORD_NORMAL, aContext, mPresContext, inherited);

  // line-height: normal, number, length, percent, inherit
  if (eCSSUnit_Percent == textData.mLineHeight.GetUnit()) {
    inherited = PR_TRUE;
    // Use |mFont.size| to pick up minimum font size.
    text->mLineHeight.SetCoordValue(
        nscoord(float(aContext->GetStyleFont()->mFont.size) *
                textData.mLineHeight.GetPercentValue()));
  } else {
    SetCoord(textData.mLineHeight, text->mLineHeight, parentText->mLineHeight,
             SETCOORD_LH | SETCOORD_FACTOR | SETCOORD_NORMAL,
             aContext, mPresContext, inherited);
    if (textData.mLineHeight.IsFixedLengthUnit() ||
        textData.mLineHeight.GetUnit() == eCSSUnit_Pixel) {
      nscoord lh = nsStyleFont::ZoomText(mPresContext,
                                         text->mLineHeight.GetCoordValue());
      nscoord minimumFontSize =
        mPresContext->GetCachedIntPref(kPresContext_MinimumFontSize);

      if (minimumFontSize > 0 && !IsChrome(mPresContext)) {
        // If we applied a minimum font size, scale the line height by
        // the same ratio.  (If we *might* have applied a minimum font
        // size, we can't cache in the rule tree.)
        inherited = PR_TRUE;
        const nsStyleFont *font = aContext->GetStyleFont();
        if (font->mSize != 0) {
          lh = nscoord(float(lh) * float(font->mFont.size) / float(font->mSize));
        } else {
          lh = minimumFontSize;
        }
      }
      text->mLineHeight.SetCoordValue(lh);
    }
  }


  // text-align: enum, string, inherit
  if (eCSSUnit_Enumerated == textData.mTextAlign.GetUnit()) {
    text->mTextAlign = textData.mTextAlign.GetIntValue();
  }
  else if (eCSSUnit_String == textData.mTextAlign.GetUnit()) {
    NS_NOTYETIMPLEMENTED("align string");
  }
  else if (eCSSUnit_Inherit == textData.mTextAlign.GetUnit()) {
    inherited = PR_TRUE;
    text->mTextAlign = parentText->mTextAlign;
  }
  else if (eCSSUnit_Initial == textData.mTextAlign.GetUnit())
    text->mTextAlign = NS_STYLE_TEXT_ALIGN_DEFAULT;

  // text-indent: length, percent, inherit
  SetCoord(textData.mTextIndent, text->mTextIndent, parentText->mTextIndent,
           SETCOORD_LPH, aContext, mPresContext, inherited);

  // text-transform: enum, none, inherit
  if (eCSSUnit_Enumerated == textData.mTextTransform.GetUnit()) {
    text->mTextTransform = textData.mTextTransform.GetIntValue();
  }
  else if (eCSSUnit_None == textData.mTextTransform.GetUnit()) {
    text->mTextTransform = NS_STYLE_TEXT_TRANSFORM_NONE;
  }
  else if (eCSSUnit_Inherit == textData.mTextTransform.GetUnit()) {
    inherited = PR_TRUE;
    text->mTextTransform = parentText->mTextTransform;
  }

  // white-space: enum, normal, inherit
  if (eCSSUnit_Enumerated == textData.mWhiteSpace.GetUnit()) {
    text->mWhiteSpace = textData.mWhiteSpace.GetIntValue();
  }
  else if (eCSSUnit_Normal == textData.mWhiteSpace.GetUnit()) {
    text->mWhiteSpace = NS_STYLE_WHITESPACE_NORMAL;
  }
  else if (eCSSUnit_Inherit == textData.mWhiteSpace.GetUnit()) {
    inherited = PR_TRUE;
    text->mWhiteSpace = parentText->mWhiteSpace;
  }

  // word-spacing: normal, length, inherit
  SetCoord(textData.mWordSpacing, text->mWordSpacing, parentText->mWordSpacing,
           SETCOORD_LH | SETCOORD_NORMAL, aContext, mPresContext, inherited);

  COMPUTE_END_INHERITED(Text, text)
}

const nsStyleStruct*
nsRuleNode::ComputeTextResetData(nsStyleStruct* aStartStruct,
                                 const nsRuleDataStruct& aData, 
                                 nsStyleContext* aContext, 
                                 nsRuleNode* aHighestNode,
                                 const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(TextReset, (), text, parentText, Text, textData)
  
  // vertical-align: enum, length, percent, inherit
  SetCoord(textData.mVerticalAlign, text->mVerticalAlign, parentText->mVerticalAlign,
           SETCOORD_LPH | SETCOORD_ENUMERATED, aContext, mPresContext, inherited);

  // text-decoration: none, enum (bit field), inherit
  if (eCSSUnit_Enumerated == textData.mDecoration.GetUnit()) {
    PRInt32 td = textData.mDecoration.GetIntValue();
    text->mTextDecoration = td;
    if (td & NS_STYLE_TEXT_DECORATION_PREF_ANCHORS) {
      PRBool underlineLinks =
        mPresContext->GetCachedBoolPref(kPresContext_UnderlineLinks);
      if (underlineLinks) {
        text->mTextDecoration |= NS_STYLE_TEXT_DECORATION_UNDERLINE;
      }
      else {
        text->mTextDecoration &= ~NS_STYLE_TEXT_DECORATION_UNDERLINE;
      }
    }
  }
  else if (eCSSUnit_None == textData.mDecoration.GetUnit()) {
    text->mTextDecoration = NS_STYLE_TEXT_DECORATION_NONE;
  }
  else if (eCSSUnit_Inherit == textData.mDecoration.GetUnit()) {
    inherited = PR_TRUE;
    text->mTextDecoration = parentText->mTextDecoration;
  }

  // unicode-bidi: enum, normal, inherit
  if (eCSSUnit_Normal == textData.mUnicodeBidi.GetUnit() ) {
    text->mUnicodeBidi = NS_STYLE_UNICODE_BIDI_NORMAL;
  }
  else if (eCSSUnit_Enumerated == textData.mUnicodeBidi.GetUnit() ) {
    text->mUnicodeBidi = textData.mUnicodeBidi.GetIntValue();
  }
  else if (eCSSUnit_Inherit == textData.mUnicodeBidi.GetUnit() ) {
    inherited = PR_TRUE;
    text->mUnicodeBidi = parentText->mUnicodeBidi;
  }

  COMPUTE_END_RESET(TextReset, text)
}

const nsStyleStruct*
nsRuleNode::ComputeUserInterfaceData(nsStyleStruct* aStartStruct,
                                     const nsRuleDataStruct& aData, 
                                     nsStyleContext* aContext, 
                                     nsRuleNode* aHighestNode,
                                     const RuleDetail& aRuleDetail,
                                     PRBool aInherited)
{
  COMPUTE_START_INHERITED(UserInterface, (), ui, parentUI,
                          UserInterface, uiData)

  // cursor: enum, auto, url, inherit
  nsCSSValueList*  list = uiData.mCursor;
  if (nsnull != list) {
    delete [] ui->mCursorArray;
    ui->mCursorArray = nsnull;
    ui->mCursorArrayLength = 0;

    if (eCSSUnit_Inherit == list->mValue.GetUnit()) {
      inherited = PR_TRUE;
      ui->mCursor = parentUI->mCursor;
      ui->CopyCursorArrayFrom(*parentUI);
    }
    else {
      // The parser will never create a list that is *all* URL values --
      // that's invalid.
      PRUint32 arrayLength = 0;
      for (nsCSSValueList *list2 = list;
           list2->mValue.GetUnit() == eCSSUnit_Array; list2 = list2->mNext)
        if (list2->mValue.GetArrayValue()->Item(0).GetImageValue())
          ++arrayLength;

      if (arrayLength != 0) {
        ui->mCursorArray = new nsCursorImage[arrayLength];
        if (ui->mCursorArray) {
          ui->mCursorArrayLength = arrayLength;

          for (nsCursorImage *item = ui->mCursorArray;
               list->mValue.GetUnit() == eCSSUnit_Array;
               list = list->mNext) {
            nsCSSValue::Array *arr = list->mValue.GetArrayValue();
            imgIRequest *req = arr->Item(0).GetImageValue();
            if (req) {
              item->mImage = req;
              if (arr->Item(1).GetUnit() != eCSSUnit_Null) {
                item->mHaveHotspot = PR_TRUE;
                item->mHotspotX = arr->Item(1).GetFloatValue(),
                item->mHotspotY = arr->Item(2).GetFloatValue();
              }
              ++item;
            }
          }
        }
      }

      if (eCSSUnit_Enumerated == list->mValue.GetUnit()) {
        ui->mCursor = list->mValue.GetIntValue();
      }
      else if (eCSSUnit_Auto == list->mValue.GetUnit()) {
        ui->mCursor = NS_STYLE_CURSOR_AUTO;
      }
    }
  }

  // user-input: auto, none, enum, inherit
  if (eCSSUnit_Enumerated == uiData.mUserInput.GetUnit()) {
    ui->mUserInput = uiData.mUserInput.GetIntValue();
  }
  else if (eCSSUnit_Auto == uiData.mUserInput.GetUnit()) {
    ui->mUserInput = NS_STYLE_USER_INPUT_AUTO;
  }
  else if (eCSSUnit_None == uiData.mUserInput.GetUnit()) {
    ui->mUserInput = NS_STYLE_USER_INPUT_NONE;
  }
  else if (eCSSUnit_Inherit == uiData.mUserInput.GetUnit()) {
    inherited = PR_TRUE;
    ui->mUserInput = parentUI->mUserInput;
  }

  // user-modify: enum, inherit
  if (eCSSUnit_Enumerated == uiData.mUserModify.GetUnit()) {
    ui->mUserModify = uiData.mUserModify.GetIntValue();
  }
  else if (eCSSUnit_Inherit == uiData.mUserModify.GetUnit()) {
    inherited = PR_TRUE;
    ui->mUserModify = parentUI->mUserModify;
  }

  // user-focus: none, normal, enum, inherit
  if (eCSSUnit_Enumerated == uiData.mUserFocus.GetUnit()) {
    ui->mUserFocus = uiData.mUserFocus.GetIntValue();
  }
  else if (eCSSUnit_None == uiData.mUserFocus.GetUnit()) {
    ui->mUserFocus = NS_STYLE_USER_FOCUS_NONE;
  }
  else if (eCSSUnit_Normal == uiData.mUserFocus.GetUnit()) {
    ui->mUserFocus = NS_STYLE_USER_FOCUS_NORMAL;
  }
  else if (eCSSUnit_Inherit == uiData.mUserFocus.GetUnit()) {
    inherited = PR_TRUE;
    ui->mUserFocus = parentUI->mUserFocus;
  }

  COMPUTE_END_INHERITED(UserInterface, ui)
}

const nsStyleStruct*
nsRuleNode::ComputeUIResetData(nsStyleStruct* aStartStruct,
                               const nsRuleDataStruct& aData, 
                               nsStyleContext* aContext, 
                               nsRuleNode* aHighestNode,
                               const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(UIReset, (), ui, parentUI, UserInterface, uiData)
  
  // user-select: none, enum, inherit
  if (eCSSUnit_Enumerated == uiData.mUserSelect.GetUnit()) {
    ui->mUserSelect = uiData.mUserSelect.GetIntValue();
  }
  else if (eCSSUnit_None == uiData.mUserSelect.GetUnit()) {
    ui->mUserSelect = NS_STYLE_USER_SELECT_NONE;
  }
  else if (eCSSUnit_Inherit == uiData.mUserSelect.GetUnit()) {
    inherited = PR_TRUE;
    ui->mUserSelect = parentUI->mUserSelect;
  }

  // force-broken-image-icons: integer
  if (eCSSUnit_Integer == uiData.mForceBrokenImageIcon.GetUnit()) {
    ui->mForceBrokenImageIcon = uiData.mForceBrokenImageIcon.GetIntValue();
  }
  COMPUTE_END_RESET(UIReset, ui)
}

const nsStyleStruct*
nsRuleNode::ComputeDisplayData(nsStyleStruct* aStartStruct,
                               const nsRuleDataStruct& aData, 
                               nsStyleContext* aContext, 
                               nsRuleNode* aHighestNode,
                               const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Display, (), display, parentDisplay,
                      Display, displayData)
  nsIAtom* pseudoTag = aContext->GetPseudoType();
  PRBool generatedContent = (pseudoTag == nsCSSPseudoElements::before || 
                             pseudoTag == nsCSSPseudoElements::after);
  NS_ASSERTION(!generatedContent || parentContext,
               "Must have parent context for generated content");
  if (parentDisplay == display && generatedContent)
    parentDisplay = parentContext->GetStyleDisplay();

  // opacity: factor, inherit
  if (eCSSUnit_Number == displayData.mOpacity.GetUnit()) {
    display->mOpacity = displayData.mOpacity.GetFloatValue();
    if (display->mOpacity > 1.0f)
      display->mOpacity = 1.0f;
    if (display->mOpacity < 0.0f)
      display->mOpacity = 0.0f;
  }
  else if (eCSSUnit_Inherit == displayData.mOpacity.GetUnit()) {
    inherited = PR_TRUE;
    display->mOpacity = parentDisplay->mOpacity;
  }

  // display: enum, none, inherit
  if (eCSSUnit_Enumerated == displayData.mDisplay.GetUnit()) {
    display->mDisplay = displayData.mDisplay.GetIntValue();
  }
  else if (eCSSUnit_None == displayData.mDisplay.GetUnit()) {
    display->mDisplay = NS_STYLE_DISPLAY_NONE;
  }
  else if (eCSSUnit_Inherit == displayData.mDisplay.GetUnit()) {
    inherited = PR_TRUE;
    display->mDisplay = parentDisplay->mDisplay;
  }

  // appearance: enum, none, inherit
  if (eCSSUnit_Enumerated == displayData.mAppearance.GetUnit()) {
    display->mAppearance = displayData.mAppearance.GetIntValue();
  }
  else if (eCSSUnit_None == displayData.mAppearance.GetUnit()) {
    display->mAppearance = NS_THEME_NONE;
  }
  else if (eCSSUnit_Inherit == displayData.mAppearance.GetUnit()) {
    inherited = PR_TRUE;
    display->mAppearance = parentDisplay->mAppearance;
  }

  // binding: url, none, inherit
  if (eCSSUnit_URL == displayData.mBinding.GetUnit()) {
    display->mBinding = displayData.mBinding.GetURLValue();
  }
  else if (eCSSUnit_None == displayData.mBinding.GetUnit()) {
    display->mBinding = nsnull;
  }
  else if (eCSSUnit_Inherit == displayData.mBinding.GetUnit()) {
    inherited = PR_TRUE;
    display->mBinding = parentDisplay->mBinding;
  }

  // position: enum, inherit
  if (eCSSUnit_Enumerated == displayData.mPosition.GetUnit()) {
    display->mPosition = displayData.mPosition.GetIntValue();
  }
  else if (eCSSUnit_Inherit == displayData.mPosition.GetUnit()) {
    inherited = PR_TRUE;
    display->mPosition = parentDisplay->mPosition;
  }

  // clear: enum, none, inherit
  if (eCSSUnit_Enumerated == displayData.mClear.GetUnit()) {
    display->mBreakType = displayData.mClear.GetIntValue();
  }
  else if (eCSSUnit_None == displayData.mClear.GetUnit()) {
    display->mBreakType = NS_STYLE_CLEAR_NONE;
  }
  else if (eCSSUnit_Inherit == displayData.mClear.GetUnit()) {
    inherited = PR_TRUE;
    display->mBreakType = parentDisplay->mBreakType;
  }

  // temp fix for bug 24000
  if (eCSSUnit_Enumerated == displayData.mBreakBefore.GetUnit()) {
    display->mBreakBefore = (NS_STYLE_PAGE_BREAK_ALWAYS == displayData.mBreakBefore.GetIntValue());
  }
  if (eCSSUnit_Enumerated == displayData.mBreakAfter.GetUnit()) {
    display->mBreakAfter = (NS_STYLE_PAGE_BREAK_ALWAYS == displayData.mBreakAfter.GetIntValue());
  }
  // end temp fix

  // float: enum, none, inherit
  if (eCSSUnit_Enumerated == displayData.mFloat.GetUnit()) {
    display->mFloats = displayData.mFloat.GetIntValue();
  }
  else if (eCSSUnit_None == displayData.mFloat.GetUnit()) {
    display->mFloats = NS_STYLE_FLOAT_NONE;
  }
  else if (eCSSUnit_Inherit == displayData.mFloat.GetUnit()) {
    inherited = PR_TRUE;
    display->mFloats = parentDisplay->mFloats;
  }

  // overflow-x: enum, auto, inherit
  if (eCSSUnit_Enumerated == displayData.mOverflowX.GetUnit()) {
    display->mOverflowX = displayData.mOverflowX.GetIntValue();
  }
  else if (eCSSUnit_Auto == displayData.mOverflowX.GetUnit()) {
    display->mOverflowX = NS_STYLE_OVERFLOW_AUTO;
  }
  else if (eCSSUnit_Inherit == displayData.mOverflowX.GetUnit()) {
    inherited = PR_TRUE;
    display->mOverflowX = parentDisplay->mOverflowX;
  }

  // overflow-y: enum, auto, inherit
  if (eCSSUnit_Enumerated == displayData.mOverflowY.GetUnit()) {
    display->mOverflowY = displayData.mOverflowY.GetIntValue();
  }
  else if (eCSSUnit_Auto == displayData.mOverflowY.GetUnit()) {
    display->mOverflowY = NS_STYLE_OVERFLOW_AUTO;
  }
  else if (eCSSUnit_Inherit == displayData.mOverflowY.GetUnit()) {
    inherited = PR_TRUE;
    display->mOverflowY = parentDisplay->mOverflowY;
  }

  // CSS3 overflow-x and overflow-y require some fixup as well in some
  // cases.  NS_STYLE_OVERFLOW_VISIBLE and NS_STYLE_OVERFLOW_CLIP are
  // meaningful only when used in both dimensions.
  if (display->mOverflowX != display->mOverflowY &&
      (display->mOverflowX == NS_STYLE_OVERFLOW_VISIBLE ||
       display->mOverflowX == NS_STYLE_OVERFLOW_CLIP ||
       display->mOverflowY == NS_STYLE_OVERFLOW_VISIBLE ||
       display->mOverflowY == NS_STYLE_OVERFLOW_CLIP)) {
    // We can't store in the rule tree since a more specific rule might
    // change these conditions.
    inherited = PR_TRUE;

    // NS_STYLE_OVERFLOW_CLIP is a deprecated value, so if it's specified
    // in only one dimension, convert it to NS_STYLE_OVERFLOW_HIDDEN.
    if (display->mOverflowX == NS_STYLE_OVERFLOW_CLIP)
      display->mOverflowX = NS_STYLE_OVERFLOW_HIDDEN;
    if (display->mOverflowY == NS_STYLE_OVERFLOW_CLIP)
      display->mOverflowY = NS_STYLE_OVERFLOW_HIDDEN;

    // If 'visible' is specified but doesn't match the other dimension, it
    // turns into 'auto'.
    if (display->mOverflowX == NS_STYLE_OVERFLOW_VISIBLE)
      display->mOverflowX = NS_STYLE_OVERFLOW_AUTO;
    if (display->mOverflowY == NS_STYLE_OVERFLOW_VISIBLE)
      display->mOverflowY = NS_STYLE_OVERFLOW_AUTO;
  }

  // clip property: length, auto, inherit
  if (eCSSUnit_Inherit == displayData.mClip.mTop.GetUnit()) { // if one is inherit, they all are
    inherited = PR_TRUE;
    display->mClipFlags = parentDisplay->mClipFlags;
    display->mClip = parentDisplay->mClip;
  }
  else {
    PRBool  fullAuto = PR_TRUE;

    display->mClipFlags = 0; // clear it

    if (eCSSUnit_Auto == displayData.mClip.mTop.GetUnit()) {
      display->mClip.y = 0;
      display->mClipFlags |= NS_STYLE_CLIP_TOP_AUTO;
    } 
    else if (displayData.mClip.mTop.IsLengthUnit()) {
      display->mClip.y = CalcLength(displayData.mClip.mTop, nsnull, aContext, mPresContext, inherited);
      fullAuto = PR_FALSE;
    }
    if (eCSSUnit_Auto == displayData.mClip.mBottom.GetUnit()) {
      // Setting to NS_MAXSIZE for the 'auto' case ensures that
      // the clip rect is nonempty. It is important that mClip be
      // nonempty if the actual clip rect could be nonempty.
      display->mClip.height = NS_MAXSIZE;
      display->mClipFlags |= NS_STYLE_CLIP_BOTTOM_AUTO;
    } 
    else if (displayData.mClip.mBottom.IsLengthUnit()) {
      display->mClip.height = CalcLength(displayData.mClip.mBottom, nsnull, aContext, mPresContext, inherited) -
                              display->mClip.y;
      fullAuto = PR_FALSE;
    }
    if (eCSSUnit_Auto == displayData.mClip.mLeft.GetUnit()) {
      display->mClip.x = 0;
      display->mClipFlags |= NS_STYLE_CLIP_LEFT_AUTO;
    } 
    else if (displayData.mClip.mLeft.IsLengthUnit()) {
      display->mClip.x = CalcLength(displayData.mClip.mLeft, nsnull, aContext, mPresContext, inherited);
      fullAuto = PR_FALSE;
    }
    if (eCSSUnit_Auto == displayData.mClip.mRight.GetUnit()) {
      // Setting to NS_MAXSIZE for the 'auto' case ensures that
      // the clip rect is nonempty. It is important that mClip be
      // nonempty if the actual clip rect could be nonempty.
      display->mClip.width = NS_MAXSIZE;
      display->mClipFlags |= NS_STYLE_CLIP_RIGHT_AUTO;
    } 
    else if (displayData.mClip.mRight.IsLengthUnit()) {
      display->mClip.width = CalcLength(displayData.mClip.mRight, nsnull, aContext, mPresContext, inherited) -
                             display->mClip.x;
      fullAuto = PR_FALSE;
    }
    display->mClipFlags &= ~NS_STYLE_CLIP_TYPE_MASK;
    if (fullAuto) {
      display->mClipFlags |= NS_STYLE_CLIP_AUTO;
    }
    else {
      display->mClipFlags |= NS_STYLE_CLIP_RECT;
    }
  }

  // CSS2 specified fixups:
  if (generatedContent) {
    // According to CSS2 section 12.1, :before and :after
    // pseudo-elements must not be positioned or floated (CSS2 12.1) and
    // must be limited to certain display types (depending on the
    // display type of the element to which they are attached).

    if (display->mPosition != NS_STYLE_POSITION_STATIC)
      display->mPosition = NS_STYLE_POSITION_STATIC;
    if (display->mFloats != NS_STYLE_FLOAT_NONE)
      display->mFloats = NS_STYLE_FLOAT_NONE;

    PRUint8 displayValue = display->mDisplay;
    if (displayValue != NS_STYLE_DISPLAY_NONE &&
        displayValue != NS_STYLE_DISPLAY_INLINE) {
      inherited = PR_TRUE;
      if (parentDisplay->IsBlockLevel()) {
        // If the subject of the selector is a block-level element,
        // allowed values are 'none', 'inline', 'block', and 'marker'.
        // If the value of the 'display' has any other value, the
        // pseudo-element will behave as if the value were 'block'.
        if (displayValue != NS_STYLE_DISPLAY_BLOCK &&
            displayValue != NS_STYLE_DISPLAY_MARKER)
          display->mDisplay = NS_STYLE_DISPLAY_BLOCK;
      } else {
        // If the subject of the selector is an inline-level element,
        // allowed values are 'none' and 'inline'. If the value of the
        // 'display' has any other value, the pseudo-element will behave
        // as if the value were 'inline'.
        display->mDisplay = NS_STYLE_DISPLAY_INLINE;
      }
    }
  }
  else if (display->mDisplay != NS_STYLE_DISPLAY_NONE) {
    // CSS2 9.7 specifies display type corrections dealing with 'float'
    // and 'position'.  Since generated content can't be floated or
    // positioned, we can deal with it here.

    // 1) if float is not none, and display is not none, then we must
    // set a block-level 'display' type per CSS2.1 section 9.7.
    if (display->mFloats != NS_STYLE_FLOAT_NONE) {
      EnsureBlockDisplay(display->mDisplay);
      
      // We can't cache the data in the rule tree since if a more specific
      // rule has 'float: none' we'll end up with the wrong 'display'
      // property.
      inherited = PR_TRUE;
    } else if (nsCSSPseudoElements::firstLetter == pseudoTag) {
      // a non-floating first-letter must be inline
      // XXX this fix can go away once bug 103189 is fixed correctly
      display->mDisplay = NS_STYLE_DISPLAY_INLINE;
      
      // We can't cache the data in the rule tree since if a more specific
      // rule has 'float: left' we'll end up with the wrong 'display'
      // property.
      inherited = PR_TRUE;
    }
    
    // 2) if position is 'absolute' or 'fixed' then display must be
    // block-level and float must be 'none'
    if (display->IsAbsolutelyPositioned()) {
      // Backup original display value for calculation of a hypothetical
      // box (CSS2 10.6.4/10.6.5).
      // See nsHTMLReflowState::CalculateHypotheticalBox
      display->mOriginalDisplay = display->mDisplay;
      EnsureBlockDisplay(display->mDisplay);
      display->mFloats = NS_STYLE_FLOAT_NONE;
      
      // We can't cache the data in the rule tree since if a more specific
      // rule has 'position: static' we'll end up with problems with the
      // 'display' and 'float' properties.
      inherited = PR_TRUE;
    }
  }

  COMPUTE_END_RESET(Display, display)
}

const nsStyleStruct*
nsRuleNode::ComputeVisibilityData(nsStyleStruct* aStartStruct,
                                  const nsRuleDataStruct& aData, 
                                  nsStyleContext* aContext, 
                                  nsRuleNode* aHighestNode,
                                  const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_INHERITED(Visibility, (mPresContext),
                          visibility, parentVisibility,
                          Display, displayData)

  // direction: enum, inherit
  if (eCSSUnit_Enumerated == displayData.mDirection.GetUnit()) {
    visibility->mDirection = displayData.mDirection.GetIntValue();
    if (NS_STYLE_DIRECTION_RTL == visibility->mDirection)
      mPresContext->SetBidiEnabled(PR_TRUE);
  }
  else if (eCSSUnit_Inherit == displayData.mDirection.GetUnit()) {
    inherited = PR_TRUE;
    visibility->mDirection = parentVisibility->mDirection;
  }

  // visibility: enum, inherit
  if (eCSSUnit_Enumerated == displayData.mVisibility.GetUnit()) {
    visibility->mVisible = displayData.mVisibility.GetIntValue();
  }
  else if (eCSSUnit_Inherit == displayData.mVisibility.GetUnit()) {
    inherited = PR_TRUE;
    visibility->mVisible = parentVisibility->mVisible;
  }

  // lang: string, inherit
  // this is not a real CSS property, it is a html attribute mapped to CSS struture
  if (eCSSUnit_String == displayData.mLang.GetUnit()) {
    if (!gLangService) {
      CallGetService(NS_LANGUAGEATOMSERVICE_CONTRACTID, &gLangService);
    }

    if (gLangService) {
      nsAutoString lang;
      displayData.mLang.GetStringValue(lang);
      visibility->mLangGroup = gLangService->LookupLanguage(lang);
    }
  } 

  COMPUTE_END_INHERITED(Visibility, visibility)
}

const nsStyleStruct*
nsRuleNode::ComputeColorData(nsStyleStruct* aStartStruct,
                             const nsRuleDataStruct& aData, 
                             nsStyleContext* aContext, 
                             nsRuleNode* aHighestNode,
                             const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_INHERITED(Color, (mPresContext), color, parentColor,
                          Color, colorData)

  // color: color, string, inherit
  // Special case for currentColor.  According to CSS3, setting color to 'currentColor'
  // should behave as if it is inherited
  if (colorData.mColor.GetUnit() == eCSSUnit_Integer && 
      colorData.mColor.GetIntValue() == NS_COLOR_CURRENTCOLOR) {
    color->mColor = parentColor->mColor;
    inherited = PR_TRUE;
  } else {
    SetColor(colorData.mColor, parentColor->mColor, mPresContext, aContext, color->mColor, 
             inherited);
  }

  COMPUTE_END_INHERITED(Color, color)
}

const nsStyleStruct*
nsRuleNode::ComputeBackgroundData(nsStyleStruct* aStartStruct,
                                  const nsRuleDataStruct& aData, 
                                  nsStyleContext* aContext, 
                                  nsRuleNode* aHighestNode,
                                  const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Background, (mPresContext), bg, parentBG,
                      Color, colorData)

  // save parentFlags in case bg == parentBG and we clobber them later
  PRUint8 parentFlags = parentBG->mBackgroundFlags;

  // background-color: color, string, enum (flags), inherit
  if (eCSSUnit_Inherit == colorData.mBackColor.GetUnit()) { // do inherit first, so SetColor doesn't do it
    bg->mBackgroundColor = parentBG->mBackgroundColor;
    bg->mBackgroundFlags &= ~NS_STYLE_BG_COLOR_TRANSPARENT;
    bg->mBackgroundFlags |= (parentFlags & NS_STYLE_BG_COLOR_TRANSPARENT);
    inherited = PR_TRUE;
  }
  else if (SetColor(colorData.mBackColor, parentBG->mBackgroundColor, 
                    mPresContext, aContext, bg->mBackgroundColor, inherited)) {
    bg->mBackgroundFlags &= ~NS_STYLE_BG_COLOR_TRANSPARENT;
    // if not using document colors, we have to use the user's background color
    // instead of any background color other than transparent
    if (!mPresContext->GetCachedBoolPref(kPresContext_UseDocumentColors) &&
        !IsChrome(mPresContext)) {
      bg->mBackgroundColor = mPresContext->DefaultBackgroundColor();
    }
  }
  else if (eCSSUnit_Enumerated == colorData.mBackColor.GetUnit()) {
    //bg->mBackgroundColor = parentBG->mBackgroundColor; XXXwdh crap crap crap!
    bg->mBackgroundFlags |= NS_STYLE_BG_COLOR_TRANSPARENT;
  }

  // background-image: url (stored as image), none, inherit
  if (eCSSUnit_Image == colorData.mBackImage.GetUnit()) {
    bg->mBackgroundImage = colorData.mBackImage.GetImageValue();
  }
  else if (eCSSUnit_None == colorData.mBackImage.GetUnit()) {
    bg->mBackgroundImage = nsnull;
  }
  else if (eCSSUnit_Inherit == colorData.mBackImage.GetUnit()) {
    inherited = PR_TRUE;
    bg->mBackgroundImage = parentBG->mBackgroundImage;
  }

  if (bg->mBackgroundImage) {
    bg->mBackgroundFlags &= ~NS_STYLE_BG_IMAGE_NONE;
  } else {
    bg->mBackgroundFlags |= NS_STYLE_BG_IMAGE_NONE;
  }

  // background-repeat: enum, inherit
  if (eCSSUnit_Enumerated == colorData.mBackRepeat.GetUnit()) {
    bg->mBackgroundRepeat = colorData.mBackRepeat.GetIntValue();
  }
  else if (eCSSUnit_Inherit == colorData.mBackRepeat.GetUnit()) {
    inherited = PR_TRUE;
    bg->mBackgroundRepeat = parentBG->mBackgroundRepeat;
  }

  // background-attachment: enum, inherit
  if (eCSSUnit_Enumerated == colorData.mBackAttachment.GetUnit()) {
    bg->mBackgroundAttachment = colorData.mBackAttachment.GetIntValue();
  }
  else if (eCSSUnit_Inherit == colorData.mBackAttachment.GetUnit()) {
    inherited = PR_TRUE;
    bg->mBackgroundAttachment = parentBG->mBackgroundAttachment;
  }

  // background-clip: enum, inherit, initial
  if (eCSSUnit_Enumerated == colorData.mBackClip.GetUnit()) {
    bg->mBackgroundClip = colorData.mBackClip.GetIntValue();
  }
  else if (eCSSUnit_Inherit == colorData.mBackClip.GetUnit()) {
    bg->mBackgroundClip = parentBG->mBackgroundClip;
  }
  else if (eCSSUnit_Initial == colorData.mBackClip.GetUnit()) {
    bg->mBackgroundClip = NS_STYLE_BG_CLIP_BORDER;
  }

  // background-inline-policy: enum, inherit, initial
  if (eCSSUnit_Enumerated == colorData.mBackInlinePolicy.GetUnit()) {
    bg->mBackgroundInlinePolicy = colorData.mBackInlinePolicy.GetIntValue();
  }
  else if (eCSSUnit_Inherit == colorData.mBackInlinePolicy.GetUnit()) {
    bg->mBackgroundInlinePolicy = parentBG->mBackgroundInlinePolicy;
  }
  else if (eCSSUnit_Initial == colorData.mBackInlinePolicy.GetUnit()) {
    bg->mBackgroundInlinePolicy = NS_STYLE_BG_INLINE_POLICY_CONTINUOUS;
  }

  // background-origin: enum, inherit, initial
  if (eCSSUnit_Enumerated == colorData.mBackOrigin.GetUnit()) {
    bg->mBackgroundOrigin = colorData.mBackOrigin.GetIntValue();
  }
  else if (eCSSUnit_Inherit == colorData.mBackOrigin.GetUnit()) {
    bg->mBackgroundOrigin = parentBG->mBackgroundOrigin;
  }
  else if (eCSSUnit_Initial == colorData.mBackOrigin.GetUnit()) {
    bg->mBackgroundOrigin = NS_STYLE_BG_ORIGIN_PADDING;
  }

  // background-position: enum, length, percent (flags), inherit
  if (eCSSUnit_Percent == colorData.mBackPositionX.GetUnit()) {
    bg->mBackgroundXPosition.mFloat = colorData.mBackPositionX.GetPercentValue();
    bg->mBackgroundFlags |= NS_STYLE_BG_X_POSITION_PERCENT;
    bg->mBackgroundFlags &= ~NS_STYLE_BG_X_POSITION_LENGTH;
  }
  else if (colorData.mBackPositionX.IsLengthUnit()) {
    bg->mBackgroundXPosition.mCoord = CalcLength(colorData.mBackPositionX, nsnull, 
                                                 aContext, mPresContext, inherited);
    bg->mBackgroundFlags |= NS_STYLE_BG_X_POSITION_LENGTH;
    bg->mBackgroundFlags &= ~NS_STYLE_BG_X_POSITION_PERCENT;
  }
  else if (eCSSUnit_Enumerated == colorData.mBackPositionX.GetUnit()) {
    bg->mBackgroundXPosition.mFloat = (float)colorData.mBackPositionX.GetIntValue() / 100.0f;
    bg->mBackgroundFlags |= NS_STYLE_BG_X_POSITION_PERCENT;
    bg->mBackgroundFlags &= ~NS_STYLE_BG_X_POSITION_LENGTH;
  }
  else if (eCSSUnit_Inherit == colorData.mBackPositionX.GetUnit()) {
    inherited = PR_TRUE;
    bg->mBackgroundXPosition = parentBG->mBackgroundXPosition;
    bg->mBackgroundFlags &= ~(NS_STYLE_BG_X_POSITION_LENGTH | NS_STYLE_BG_X_POSITION_PERCENT);
    bg->mBackgroundFlags |= (parentFlags & (NS_STYLE_BG_X_POSITION_LENGTH | NS_STYLE_BG_X_POSITION_PERCENT));
  }

  if (eCSSUnit_Percent == colorData.mBackPositionY.GetUnit()) {
    bg->mBackgroundYPosition.mFloat = colorData.mBackPositionY.GetPercentValue();
    bg->mBackgroundFlags |= NS_STYLE_BG_Y_POSITION_PERCENT;
    bg->mBackgroundFlags &= ~NS_STYLE_BG_Y_POSITION_LENGTH;
  }
  else if (colorData.mBackPositionY.IsLengthUnit()) {
    bg->mBackgroundYPosition.mCoord = CalcLength(colorData.mBackPositionY, nsnull,
                                                 aContext, mPresContext, inherited);
    bg->mBackgroundFlags |= NS_STYLE_BG_Y_POSITION_LENGTH;
    bg->mBackgroundFlags &= ~NS_STYLE_BG_Y_POSITION_PERCENT;
  }
  else if (eCSSUnit_Enumerated == colorData.mBackPositionY.GetUnit()) {
    bg->mBackgroundYPosition.mFloat = (float)colorData.mBackPositionY.GetIntValue() / 100.0f;
    bg->mBackgroundFlags |= NS_STYLE_BG_Y_POSITION_PERCENT;
    bg->mBackgroundFlags &= ~NS_STYLE_BG_Y_POSITION_LENGTH;
  }
  else if (eCSSUnit_Inherit == colorData.mBackPositionY.GetUnit()) {
    inherited = PR_TRUE;
    bg->mBackgroundYPosition = parentBG->mBackgroundYPosition;
    bg->mBackgroundFlags &= ~(NS_STYLE_BG_Y_POSITION_LENGTH | NS_STYLE_BG_Y_POSITION_PERCENT);
    bg->mBackgroundFlags |= (parentFlags & (NS_STYLE_BG_Y_POSITION_LENGTH | NS_STYLE_BG_Y_POSITION_PERCENT));
  }

  COMPUTE_END_RESET(Background, bg)
}

const nsStyleStruct*
nsRuleNode::ComputeMarginData(nsStyleStruct* aStartStruct,
                              const nsRuleDataStruct& aData, 
                              nsStyleContext* aContext, 
                              nsRuleNode* aHighestNode,
                              const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Margin, (), margin, parentMargin, Margin, marginData)

  // margin: length, percent, auto, inherit
  nsStyleCoord  coord;
  nsStyleCoord  parentCoord;
  NS_FOR_CSS_SIDES(side) {
    parentMargin->mMargin.Get(side, parentCoord);
    if (SetCoord(marginData.mMargin.*(nsCSSRect::sides[side]),
                 coord, parentCoord, SETCOORD_LPAH,
                 aContext, mPresContext, inherited)) {
      margin->mMargin.Set(side, coord);
    }
  }

  AdjustLogicalBoxProp(aContext,
                       marginData.mMarginLeftLTRSource,
                       marginData.mMarginLeftRTLSource,
                       marginData.mMarginStart, marginData.mMarginEnd,
                       parentMargin->mMargin, margin->mMargin,
                       NS_SIDE_LEFT, SETCOORD_LPAH, inherited);
  AdjustLogicalBoxProp(aContext,
                       marginData.mMarginRightLTRSource,
                       marginData.mMarginRightRTLSource,
                       marginData.mMarginEnd, marginData.mMarginStart,
                       parentMargin->mMargin, margin->mMargin,
                       NS_SIDE_RIGHT, SETCOORD_LPAH, inherited);

  margin->RecalcData();
  COMPUTE_END_RESET(Margin, margin)
}

const nsStyleStruct* 
nsRuleNode::ComputeBorderData(nsStyleStruct* aStartStruct,
                              const nsRuleDataStruct& aData, 
                              nsStyleContext* aContext, 
                              nsRuleNode* aHighestNode,
                              const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Border, (mPresContext), border, parentBorder,
                      Margin, marginData)

  // border-width, border-*-width: length, enum, inherit
  nsStyleCoord  coord;
  nsStyleCoord  parentCoord;
  { // scope for compilers with broken |for| loop scoping
    NS_FOR_CSS_SIDES(side) {
      const nsCSSValue &value = marginData.mBorderWidth.*(nsCSSRect::sides[side]);
      NS_ASSERTION(eCSSUnit_Percent != value.GetUnit(),
                   "Percentage borders not implemented yet "
                   "If implementing, make sure to fix all consumers of "
                   "nsStyleBorder, the IsPercentageAwareChild method, "
                   "the nsAbsoluteContainingBlock::FrameDependsOnContainer "
                   "method, the "
                   "nsLineLayout::IsPercentageAwareReplacedElement method "
                   "and probably some other places");
      if (eCSSUnit_Enumerated == value.GetUnit()) {
        NS_ASSERTION(value.GetIntValue() == NS_STYLE_BORDER_WIDTH_THIN ||
                     value.GetIntValue() == NS_STYLE_BORDER_WIDTH_MEDIUM ||
                     value.GetIntValue() == NS_STYLE_BORDER_WIDTH_THICK,
                     "Unexpected enum value");
        border->SetBorderWidth(side,
                               (mPresContext->GetBorderWidthTable())[value.GetIntValue()]);
      }
      else if (SetCoord(value, coord, parentCoord, SETCOORD_LENGTH, aContext,
                        mPresContext, inherited)) {
        if (coord.GetUnit() == eStyleUnit_Coord) {
          border->SetBorderWidth(side, coord.GetCoordValue());
        }
#ifdef DEBUG
        else {
          NS_ASSERTION(coord.GetUnit() == eStyleUnit_Chars, "unexpected unit");
          NS_WARNING("Border set in chars; we don't handle that");
        }
#endif        
      }
      else if (eCSSUnit_Inherit == value.GetUnit()) {
        inherited = PR_TRUE;
        border->SetBorderWidth(side, parentBorder->GetBorderWidth(side));
      }
      else if (eCSSUnit_Initial == value.GetUnit()) {
        border->SetBorderWidth(side,
          (mPresContext->GetBorderWidthTable())[NS_STYLE_BORDER_WIDTH_MEDIUM]);
      }
    }
  }

  // border-style, border-*-style: enum, none, inherit
  const nsCSSRect& ourStyle = marginData.mBorderStyle;
  { // scope for compilers with broken |for| loop scoping
    NS_FOR_CSS_SIDES(side) {
      const nsCSSValue &value = ourStyle.*(nsCSSRect::sides[side]);
      nsCSSUnit unit = value.GetUnit();
      if (eCSSUnit_Enumerated == unit) {
        border->SetBorderStyle(side, value.GetIntValue());
      }
      else if (eCSSUnit_None == unit || eCSSUnit_Initial == unit) {
        border->SetBorderStyle(side, NS_STYLE_BORDER_STYLE_NONE);
      }
      else if (eCSSUnit_Inherit == unit) {
        inherited = PR_TRUE;
        border->SetBorderStyle(side, parentBorder->GetBorderStyle(side));
      }
    }
  }

  // -moz-border-*-colors: color, string, enum
  nscolor borderColor;
  nscolor unused = NS_RGB(0,0,0);
  
  { // scope for compilers with broken |for| loop scoping
    NS_FOR_CSS_SIDES(side) {
      nsCSSValueList* list =
          marginData.mBorderColors.*(nsCSSValueListRect::sides[side]);
      if (list) {
        // Some composite border color information has been specified for this
        // border side.
        border->EnsureBorderColors();
        border->ClearBorderColors(side);
        while (list) {
          if (SetColor(list->mValue, unused, mPresContext, aContext, borderColor, inherited))
            border->AppendBorderColor(side, borderColor, PR_FALSE);
          else if (eCSSUnit_Enumerated == list->mValue.GetUnit() &&
                   NS_STYLE_COLOR_TRANSPARENT == list->mValue.GetIntValue())
            border->AppendBorderColor(side, nsnull, PR_TRUE);
          list = list->mNext;
        }
      }
    }
  }

  // border-color, border-*-color: color, string, enum, inherit
  const nsCSSRect& ourBorderColor = marginData.mBorderColor;
  PRBool transparent;
  PRBool foreground;

  { // scope for compilers with broken |for| loop scoping
    NS_FOR_CSS_SIDES(side) {
      const nsCSSValue &value = ourBorderColor.*(nsCSSRect::sides[side]);
      if (eCSSUnit_Inherit == value.GetUnit()) {
        if (parentContext) {
          inherited = PR_TRUE;
          parentBorder->GetBorderColor(side, borderColor,
                                       transparent, foreground);
          if (transparent)
            border->SetBorderTransparent(side);
          else if (foreground) {
            // We want to inherit the color from the parent, not use the
            // color on the element where this chunk of style data will be
            // used.  We can ensure that the data for the parent are fully
            // computed (unlike for the element where this will be used, for
            // which the color could be specified on a more specific rule).
            border->SetBorderColor(side, parentContext->GetStyleColor()->mColor);
          } else
            border->SetBorderColor(side, borderColor);
        } else {
          // We're the root
          border->SetBorderToForeground(side);
        }
      }
      else if (SetColor(value, unused, mPresContext, aContext, borderColor, inherited)) {
        border->SetBorderColor(side, borderColor);
      }
      else if (eCSSUnit_Enumerated == value.GetUnit()) {
        switch (value.GetIntValue()) {
          case NS_STYLE_COLOR_TRANSPARENT:
            border->SetBorderTransparent(side);
            break;
          case NS_STYLE_COLOR_MOZ_USE_TEXT_COLOR:
            border->SetBorderToForeground(side);
            break;
        }
      }
    }
  }

  // -moz-border-radius: length, percent, inherit
  { // scope for compilers with broken |for| loop scoping
    NS_FOR_CSS_SIDES(side) {
      parentBorder->mBorderRadius.Get(side, parentCoord);
      if (SetCoord(marginData.mBorderRadius.*(nsCSSRect::sides[side]), coord,
                   parentCoord, SETCOORD_LPH, aContext, mPresContext,
                   inherited))
        border->mBorderRadius.Set(side, coord);
    }
  }

  // float-edge: enum, inherit
  if (eCSSUnit_Enumerated == marginData.mFloatEdge.GetUnit())
    border->mFloatEdge = marginData.mFloatEdge.GetIntValue();
  else if (eCSSUnit_Inherit == marginData.mFloatEdge.GetUnit()) {
    inherited = PR_TRUE;
    border->mFloatEdge = parentBorder->mFloatEdge;
  }

  COMPUTE_END_RESET(Border, border)
}
  
const nsStyleStruct*
nsRuleNode::ComputePaddingData(nsStyleStruct* aStartStruct,
                               const nsRuleDataStruct& aData, 
                               nsStyleContext* aContext, 
                               nsRuleNode* aHighestNode,
                               const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Padding, (), padding, parentPadding, Margin, marginData)

  // padding: length, percent, inherit
  nsStyleCoord  coord;
  nsStyleCoord  parentCoord;
  NS_FOR_CSS_SIDES(side) {
    parentPadding->mPadding.Get(side, parentCoord);
    if (SetCoord(marginData.mPadding.*(nsCSSRect::sides[side]),
                 coord, parentCoord, SETCOORD_LPH,
                 aContext, mPresContext, inherited)) {
      padding->mPadding.Set(side, coord);
    }
  }

  AdjustLogicalBoxProp(aContext,
                       marginData.mPaddingLeftLTRSource,
                       marginData.mPaddingLeftRTLSource,
                       marginData.mPaddingStart, marginData.mPaddingEnd,
                       parentPadding->mPadding, padding->mPadding,
                       NS_SIDE_LEFT, SETCOORD_LPH, inherited);
  AdjustLogicalBoxProp(aContext,
                       marginData.mPaddingRightLTRSource,
                       marginData.mPaddingRightRTLSource,
                       marginData.mPaddingEnd, marginData.mPaddingStart,
                       parentPadding->mPadding, padding->mPadding,
                       NS_SIDE_RIGHT, SETCOORD_LPH, inherited);

  padding->RecalcData();
  COMPUTE_END_RESET(Padding, padding)
}

const nsStyleStruct*
nsRuleNode::ComputeOutlineData(nsStyleStruct* aStartStruct,
                               const nsRuleDataStruct& aData, 
                               nsStyleContext* aContext, 
                               nsRuleNode* aHighestNode,
                               const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Outline, (mPresContext), outline, parentOutline,
                      Margin, marginData)

  // outline-width: length, enum, inherit
  SetCoord(marginData.mOutlineWidth, outline->mOutlineWidth, parentOutline->mOutlineWidth,
           SETCOORD_LEH, aContext, mPresContext, inherited);

  // outline-offset: length, inherit
  SetCoord(marginData.mOutlineOffset, outline->mOutlineOffset, parentOutline->mOutlineOffset,
           SETCOORD_LH, aContext, mPresContext, inherited);
  

  // outline-color: color, string, enum, inherit
  nscolor outlineColor;
  nscolor unused = NS_RGB(0,0,0);
  if (eCSSUnit_Inherit == marginData.mOutlineColor.GetUnit()) {
    inherited = PR_TRUE;
    if (parentOutline->GetOutlineColor(outlineColor))
      outline->SetOutlineColor(outlineColor);
    else
      outline->SetOutlineInvert();
  }
  else if (SetColor(marginData.mOutlineColor, unused, mPresContext, aContext, outlineColor, inherited))
    outline->SetOutlineColor(outlineColor);
  else if (eCSSUnit_Enumerated == marginData.mOutlineColor.GetUnit())
    outline->SetOutlineInvert();

// -moz-outline-radius: length, percent, inherit
  nsStyleCoord  coord;
  nsStyleCoord  parentCoord;
  { // scope for compilers with broken |for| loop scoping
    NS_FOR_CSS_SIDES(side) {
      parentOutline->mOutlineRadius.Get(side, parentCoord);
      if (SetCoord(marginData.mOutlineRadius.*(nsCSSRect::sides[side]), coord,
                   parentCoord, SETCOORD_LPH, aContext, mPresContext,
                   inherited))
        outline->mOutlineRadius.Set(side, coord);
    }
  }

  // outline-style: auto, enum, none, inherit
  if (eCSSUnit_Enumerated == marginData.mOutlineStyle.GetUnit())
    outline->SetOutlineStyle(marginData.mOutlineStyle.GetIntValue());
  else if (eCSSUnit_None == marginData.mOutlineStyle.GetUnit())
    outline->SetOutlineStyle(NS_STYLE_BORDER_STYLE_NONE);
  else if (eCSSUnit_Auto == marginData.mOutlineStyle.GetUnit()) {
    outline->SetOutlineStyle(NS_STYLE_BORDER_STYLE_AUTO);
  } else if (eCSSUnit_Inherit == marginData.mOutlineStyle.GetUnit()) {
    inherited = PR_TRUE;
    outline->SetOutlineStyle(parentOutline->GetOutlineStyle());
  }

  outline->RecalcData(mPresContext);
  COMPUTE_END_RESET(Outline, outline)
}

const nsStyleStruct* 
nsRuleNode::ComputeListData(nsStyleStruct* aStartStruct,
                            const nsRuleDataStruct& aData, 
                            nsStyleContext* aContext, 
                            nsRuleNode* aHighestNode,
                            const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_INHERITED(List, (), list, parentList, List, listData)

  // list-style-type: enum, none, inherit
  if (eCSSUnit_Enumerated == listData.mType.GetUnit()) {
    list->mListStyleType = listData.mType.GetIntValue();
  }
  else if (eCSSUnit_None == listData.mType.GetUnit()) {
    list->mListStyleType = NS_STYLE_LIST_STYLE_NONE;
  }
  else if (eCSSUnit_Inherit == listData.mType.GetUnit()) {
    inherited = PR_TRUE;
    list->mListStyleType = parentList->mListStyleType;
  }

  // list-style-image: url, none, inherit
  if (eCSSUnit_Image == listData.mImage.GetUnit()) {
    list->mListStyleImage = listData.mImage.GetImageValue();
  }
  else if (eCSSUnit_None == listData.mImage.GetUnit()) {
    list->mListStyleImage = nsnull;
  }
  else if (eCSSUnit_Inherit == listData.mImage.GetUnit()) {
    inherited = PR_TRUE;
    list->mListStyleImage = parentList->mListStyleImage;
  }

  // list-style-position: enum, inherit
  if (eCSSUnit_Enumerated == listData.mPosition.GetUnit()) {
    list->mListStylePosition = listData.mPosition.GetIntValue();
  }
  else if (eCSSUnit_Inherit == listData.mPosition.GetUnit()) {
    inherited = PR_TRUE;
    list->mListStylePosition = parentList->mListStylePosition;
  }

  // image region property: length, auto, inherit
  if (eCSSUnit_Inherit == listData.mImageRegion.mTop.GetUnit()) { // if one is inherit, they all are
    inherited = PR_TRUE;
    list->mImageRegion = parentList->mImageRegion;
  }
  else {
    if (eCSSUnit_Auto == listData.mImageRegion.mTop.GetUnit())
      list->mImageRegion.y = 0;
    else if (listData.mImageRegion.mTop.IsLengthUnit())
      list->mImageRegion.y = CalcLength(listData.mImageRegion.mTop, nsnull, aContext, mPresContext, inherited);
      
    if (eCSSUnit_Auto == listData.mImageRegion.mBottom.GetUnit())
      list->mImageRegion.height = 0;
    else if (listData.mImageRegion.mBottom.IsLengthUnit())
      list->mImageRegion.height = CalcLength(listData.mImageRegion.mBottom, nsnull, aContext, 
                                            mPresContext, inherited) - list->mImageRegion.y;
  
    if (eCSSUnit_Auto == listData.mImageRegion.mLeft.GetUnit())
      list->mImageRegion.x = 0;
    else if (listData.mImageRegion.mLeft.IsLengthUnit())
      list->mImageRegion.x = CalcLength(listData.mImageRegion.mLeft, nsnull, aContext, mPresContext, inherited);
      
    if (eCSSUnit_Auto == listData.mImageRegion.mRight.GetUnit())
      list->mImageRegion.width = 0;
    else if (listData.mImageRegion.mRight.IsLengthUnit())
      list->mImageRegion.width = CalcLength(listData.mImageRegion.mRight, nsnull, aContext, mPresContext, inherited) -
                                list->mImageRegion.x;
  }

  COMPUTE_END_INHERITED(List, list)
}

const nsStyleStruct* 
nsRuleNode::ComputePositionData(nsStyleStruct* aStartStruct,
                                const nsRuleDataStruct& aData, 
                                nsStyleContext* aContext, 
                                nsRuleNode* aHighestNode,
                                const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Position, (), pos, parentPos, Position, posData)

  // box offsets: length, percent, auto, inherit
  nsStyleCoord  coord;
  nsStyleCoord  parentCoord;
  NS_FOR_CSS_SIDES(side) {
    parentPos->mOffset.Get(side, parentCoord);
    if (SetCoord(posData.mOffset.*(nsCSSRect::sides[side]),
                 coord, parentCoord, SETCOORD_LPAH,
                 aContext, mPresContext, inherited)) {
      pos->mOffset.Set(side, coord);
    }
  }

  if (posData.mWidth.GetUnit() == eCSSUnit_Proportional)
    pos->mWidth.SetIntValue((PRInt32)(posData.mWidth.GetFloatValue()), eStyleUnit_Proportional);
  else 
    SetCoord(posData.mWidth, pos->mWidth, parentPos->mWidth,
             SETCOORD_LPAH, aContext, mPresContext, inherited);
  SetCoord(posData.mMinWidth, pos->mMinWidth, parentPos->mMinWidth,
           SETCOORD_LPH, aContext, mPresContext, inherited);
  if (! SetCoord(posData.mMaxWidth, pos->mMaxWidth, parentPos->mMaxWidth,
                 SETCOORD_LPH, aContext, mPresContext, inherited)) {
    if (eCSSUnit_None == posData.mMaxWidth.GetUnit()) {
      pos->mMaxWidth.Reset();
    }
  }

  SetCoord(posData.mHeight, pos->mHeight, parentPos->mHeight,
           SETCOORD_LPAH, aContext, mPresContext, inherited);
  SetCoord(posData.mMinHeight, pos->mMinHeight, parentPos->mMinHeight,
           SETCOORD_LPH, aContext, mPresContext, inherited);
  if (! SetCoord(posData.mMaxHeight, pos->mMaxHeight, parentPos->mMaxHeight,
                 SETCOORD_LPH, aContext, mPresContext, inherited)) {
    if (eCSSUnit_None == posData.mMaxHeight.GetUnit()) {
      pos->mMaxHeight.Reset();
    }
  }

  // box-sizing: enum, inherit
  if (eCSSUnit_Enumerated == posData.mBoxSizing.GetUnit()) {
    pos->mBoxSizing = posData.mBoxSizing.GetIntValue();
  }
  else if (eCSSUnit_Inherit == posData.mBoxSizing.GetUnit()) {
    inherited = PR_TRUE;
    pos->mBoxSizing = parentPos->mBoxSizing;
  }

  // z-index
  if (! SetCoord(posData.mZIndex, pos->mZIndex, parentPos->mZIndex,
                 SETCOORD_IA, aContext, nsnull, inherited)) {
    if (eCSSUnit_Inherit == posData.mZIndex.GetUnit()) {
      // handle inherit, because it's ok to inherit 'auto' here
      inherited = PR_TRUE;
      pos->mZIndex = parentPos->mZIndex;
    }
  }

  COMPUTE_END_RESET(Position, pos)
}

const nsStyleStruct* 
nsRuleNode::ComputeTableData(nsStyleStruct* aStartStruct,
                             const nsRuleDataStruct& aData, 
                             nsStyleContext* aContext, 
                             nsRuleNode* aHighestNode,
                             const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Table, (), table, parentTable, Table, tableData)

  // table-layout: auto, enum, inherit
  if (eCSSUnit_Enumerated == tableData.mLayout.GetUnit())
    table->mLayoutStrategy = tableData.mLayout.GetIntValue();
  else if (eCSSUnit_Auto == tableData.mLayout.GetUnit())
    table->mLayoutStrategy = NS_STYLE_TABLE_LAYOUT_AUTO;
  else if (eCSSUnit_Inherit == tableData.mLayout.GetUnit()) {
    inherited = PR_TRUE;
    table->mLayoutStrategy = parentTable->mLayoutStrategy;
  }

  // rules: enum (not a real CSS prop)
  if (eCSSUnit_Enumerated == tableData.mRules.GetUnit())
    table->mRules = tableData.mRules.GetIntValue();

  // frame: enum (not a real CSS prop)
  if (eCSSUnit_Enumerated == tableData.mFrame.GetUnit())
    table->mFrame = tableData.mFrame.GetIntValue();

  // cols: enum, int (not a real CSS prop)
  if (eCSSUnit_Enumerated == tableData.mCols.GetUnit() ||
      eCSSUnit_Integer == tableData.mCols.GetUnit())
    table->mCols = tableData.mCols.GetIntValue();

  // span: pixels (not a real CSS prop)
  if (eCSSUnit_Enumerated == tableData.mSpan.GetUnit() ||
      eCSSUnit_Integer == tableData.mSpan.GetUnit())
    table->mSpan = tableData.mSpan.GetIntValue();
    
  COMPUTE_END_RESET(Table, table)
}

const nsStyleStruct* 
nsRuleNode::ComputeTableBorderData(nsStyleStruct* aStartStruct,
                                   const nsRuleDataStruct& aData, 
                                   nsStyleContext* aContext, 
                                   nsRuleNode* aHighestNode,
                                   const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_INHERITED(TableBorder, (mPresContext), table, parentTable,
                          Table, tableData)

  // border-collapse: enum, inherit
  if (eCSSUnit_Enumerated == tableData.mBorderCollapse.GetUnit()) {
    table->mBorderCollapse = tableData.mBorderCollapse.GetIntValue();
  }
  else if (eCSSUnit_Inherit == tableData.mBorderCollapse.GetUnit()) {
    inherited = PR_TRUE;
    table->mBorderCollapse = parentTable->mBorderCollapse;
  }

  // border-spacing-x: length, inherit
  SetCoord(tableData.mBorderSpacing.mXValue, table->mBorderSpacingX,
           parentTable->mBorderSpacingX, SETCOORD_LH,
           aContext, mPresContext, inherited);
  // border-spacing-y: length, inherit
  SetCoord(tableData.mBorderSpacing.mYValue, table->mBorderSpacingY,
           parentTable->mBorderSpacingY, SETCOORD_LH,
           aContext, mPresContext, inherited);

  // caption-side: enum, inherit
  if (eCSSUnit_Enumerated == tableData.mCaptionSide.GetUnit()) {
    table->mCaptionSide = tableData.mCaptionSide.GetIntValue();
  }
  else if (eCSSUnit_Inherit == tableData.mCaptionSide.GetUnit()) {
    inherited = PR_TRUE;
    table->mCaptionSide = parentTable->mCaptionSide;
  }

  // empty-cells: enum, inherit
  if (eCSSUnit_Enumerated == tableData.mEmptyCells.GetUnit()) {
    table->mEmptyCells = tableData.mEmptyCells.GetIntValue();
  }
  else if (eCSSUnit_Inherit == tableData.mEmptyCells.GetUnit()) {
    inherited = PR_TRUE;
    table->mEmptyCells = parentTable->mEmptyCells;
  }

  COMPUTE_END_INHERITED(TableBorder, table)
}

const nsStyleStruct* 
nsRuleNode::ComputeContentData(nsStyleStruct* aStartStruct,
                               const nsRuleDataStruct& aData, 
                               nsStyleContext* aContext, 
                               nsRuleNode* aHighestNode,
                               const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Content, (), content, parentContent,
                      Content, contentData)

  // content: [string, url, counter, attr, enum]+, normal, inherit
  PRUint32 count;
  nsAutoString  buffer;
  nsCSSValueList* contentValue = contentData.mContent;
  if (contentValue) {
    if (eCSSUnit_Normal == contentValue->mValue.GetUnit() ||
        eCSSUnit_Initial == contentValue->mValue.GetUnit()) {
      // "normal" and "initial" both mean no content
      content->AllocateContents(0);
    }
    else if (eCSSUnit_Inherit == contentValue->mValue.GetUnit()) {
      inherited = PR_TRUE;
      count = parentContent->ContentCount();
      if (NS_SUCCEEDED(content->AllocateContents(count))) {
        while (0 < count--) {
          content->ContentAt(count) = parentContent->ContentAt(count);
        }
      }
    }
    else {
      count = 0;
      while (contentValue) {
        count++;
        contentValue = contentValue->mNext;
      }
      if (NS_SUCCEEDED(content->AllocateContents(count))) {
        const nsAutoString  nullStr;
        count = 0;
        contentValue = contentData.mContent;
        while (contentValue) {
          const nsCSSValue& value = contentValue->mValue;
          nsCSSUnit unit = value.GetUnit();
          nsStyleContentType type;
          nsStyleContentData &data = content->ContentAt(count++);
          switch (unit) {
            case eCSSUnit_String:   type = eStyleContentType_String;    break;
            case eCSSUnit_Image:    type = eStyleContentType_Image;       break;
            case eCSSUnit_Attr:     type = eStyleContentType_Attr;      break;
            case eCSSUnit_Counter:  type = eStyleContentType_Counter;   break;
            case eCSSUnit_Counters: type = eStyleContentType_Counters;  break;
            case eCSSUnit_Enumerated:
              switch (value.GetIntValue()) {
                case NS_STYLE_CONTENT_OPEN_QUOTE:     
                  type = eStyleContentType_OpenQuote;     break;
                case NS_STYLE_CONTENT_CLOSE_QUOTE:
                  type = eStyleContentType_CloseQuote;    break;
                case NS_STYLE_CONTENT_NO_OPEN_QUOTE:
                  type = eStyleContentType_NoOpenQuote;   break;
                case NS_STYLE_CONTENT_NO_CLOSE_QUOTE:
                  type = eStyleContentType_NoCloseQuote;  break;
                case NS_STYLE_CONTENT_ALT_CONTENT:
                  type = eStyleContentType_AltContent;    break;
                default:
                  NS_ERROR("bad content value");
              }
              break;
            default:
              NS_ERROR("bad content type");
          }
          data.mType = type;
          if (type == eStyleContentType_Image) {
            data.mContent.mImage = value.GetImageValue();
            NS_IF_ADDREF(data.mContent.mImage);
          }
          else if (type <= eStyleContentType_Attr) {
            value.GetStringValue(buffer);
            Unquote(buffer);
            data.mContent.mString = nsCRT::strdup(buffer.get());
          }
          else if (type <= eStyleContentType_Counters) {
            data.mContent.mCounters = value.GetArrayValue();
            data.mContent.mCounters->AddRef();
          }
          else {
            data.mContent.mString = nsnull;
          }
          contentValue = contentValue->mNext;
        }
      } 
    }
  }

  // counter-increment: [string [int]]+, none, inherit
  nsCSSCounterData* ourIncrement = contentData.mCounterIncrement;
  if (ourIncrement) {
    if (eCSSUnit_None == ourIncrement->mCounter.GetUnit() ||
        eCSSUnit_Initial == ourIncrement->mCounter.GetUnit()) {
      content->AllocateCounterIncrements(0);
    }
    else if (eCSSUnit_Inherit == ourIncrement->mCounter.GetUnit()) {
      inherited = PR_TRUE;
      count = parentContent->CounterIncrementCount();
      if (NS_SUCCEEDED(content->AllocateCounterIncrements(count))) {
        while (0 < count--) {
          const nsStyleCounterData *data =
            parentContent->GetCounterIncrementAt(count);
          content->SetCounterIncrementAt(count, data->mCounter, data->mValue);
        }
      }
    }
    else if (eCSSUnit_String == ourIncrement->mCounter.GetUnit()) {
      count = 0;
      while (ourIncrement) {
        count++;
        ourIncrement = ourIncrement->mNext;
      }
      if (NS_SUCCEEDED(content->AllocateCounterIncrements(count))) {
        count = 0;
        ourIncrement = contentData.mCounterIncrement;
        while (ourIncrement) {
          PRInt32 increment;
          if (eCSSUnit_Integer == ourIncrement->mValue.GetUnit()) {
            increment = ourIncrement->mValue.GetIntValue();
          }
          else {
            increment = 1;
          }
          ourIncrement->mCounter.GetStringValue(buffer);
          content->SetCounterIncrementAt(count++, buffer, increment);
          ourIncrement = ourIncrement->mNext;
        }
      }
    }
  }

  // counter-reset: [string [int]]+, none, inherit
  nsCSSCounterData* ourReset = contentData.mCounterReset;
  if (ourReset) {
    if (eCSSUnit_None == ourReset->mCounter.GetUnit() ||
        eCSSUnit_Initial == ourReset->mCounter.GetUnit()) {
      content->AllocateCounterResets(0);
    }
    else if (eCSSUnit_Inherit == ourReset->mCounter.GetUnit()) {
      inherited = PR_TRUE;
      count = parentContent->CounterResetCount();
      if (NS_SUCCEEDED(content->AllocateCounterResets(count))) {
        while (0 < count--) {
          const nsStyleCounterData *data =
            parentContent->GetCounterResetAt(count);
          content->SetCounterResetAt(count, data->mCounter, data->mValue);
        }
      }
    }
    else if (eCSSUnit_String == ourReset->mCounter.GetUnit()) {
      count = 0;
      while (ourReset) {
        count++;
        ourReset = ourReset->mNext;
      }
      if (NS_SUCCEEDED(content->AllocateCounterResets(count))) {
        count = 0;
        ourReset = contentData.mCounterReset;
        while (ourReset) {
          PRInt32 reset;
          if (eCSSUnit_Integer == ourReset->mValue.GetUnit()) {
            reset = ourReset->mValue.GetIntValue();
          }
          else {
            reset = 0;
          }
          ourReset->mCounter.GetStringValue(buffer);
          content->SetCounterResetAt(count++, buffer, reset);
          ourReset = ourReset->mNext;
        }
      }
    }
  }

  // marker-offset: length, auto, inherit
  SetCoord(contentData.mMarkerOffset, content->mMarkerOffset, parentContent->mMarkerOffset,
           SETCOORD_LH | SETCOORD_AUTO, aContext, mPresContext, inherited);
    
  COMPUTE_END_RESET(Content, content)
}

const nsStyleStruct* 
nsRuleNode::ComputeQuotesData(nsStyleStruct* aStartStruct,
                              const nsRuleDataStruct& aData, 
                              nsStyleContext* aContext, 
                              nsRuleNode* aHighestNode,
                              const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_INHERITED(Quotes, (), quotes, parentQuotes,
                          Content, contentData)

  // quotes: [string string]+, none, inherit
  PRUint32 count;
  nsAutoString  buffer;
  nsCSSQuotes* ourQuotes = contentData.mQuotes;
  if (ourQuotes) {
    nsAutoString  closeBuffer;
    if (eCSSUnit_Inherit == ourQuotes->mOpen.GetUnit()) {
      inherited = PR_TRUE;
      count = parentQuotes->QuotesCount();
      if (NS_SUCCEEDED(quotes->AllocateQuotes(count))) {
        while (0 < count--) {
          parentQuotes->GetQuotesAt(count, buffer, closeBuffer);
          quotes->SetQuotesAt(count, buffer, closeBuffer);
        }
      }
    }
    else if (eCSSUnit_None == ourQuotes->mOpen.GetUnit()) {
      quotes->AllocateQuotes(0);
    }
    else if (eCSSUnit_String == ourQuotes->mOpen.GetUnit()) {
      count = 0;
      while (ourQuotes) {
        count++;
        ourQuotes = ourQuotes->mNext;
      }
      if (NS_SUCCEEDED(quotes->AllocateQuotes(count))) {
        count = 0;
        ourQuotes = contentData.mQuotes;
        while (ourQuotes) {
          ourQuotes->mOpen.GetStringValue(buffer);
          ourQuotes->mClose.GetStringValue(closeBuffer);
          Unquote(buffer);
          Unquote(closeBuffer);
          quotes->SetQuotesAt(count++, buffer, closeBuffer);
          ourQuotes = ourQuotes->mNext;
        }
      }
    }
  }

  COMPUTE_END_INHERITED(Quotes, quotes)
}

const nsStyleStruct* 
nsRuleNode::ComputeXULData(nsStyleStruct* aStartStruct,
                           const nsRuleDataStruct& aData, 
                           nsStyleContext* aContext, 
                           nsRuleNode* aHighestNode,
                           const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(XUL, (), xul, parentXUL, XUL, xulData)

  // box-align: enum, inherit
  if (eCSSUnit_Enumerated == xulData.mBoxAlign.GetUnit()) {
    xul->mBoxAlign = xulData.mBoxAlign.GetIntValue();
  }
  else if (eCSSUnit_Inherit == xulData.mBoxAlign.GetUnit()) {
    inherited = PR_TRUE;
    xul->mBoxAlign = parentXUL->mBoxAlign;
  }

  // box-direction: enum, inherit
  if (eCSSUnit_Enumerated == xulData.mBoxDirection.GetUnit()) {
    xul->mBoxDirection = xulData.mBoxDirection.GetIntValue();
  }
  else if (eCSSUnit_Inherit == xulData.mBoxDirection.GetUnit()) {
    inherited = PR_TRUE;
    xul->mBoxDirection = parentXUL->mBoxDirection;
  }

  // box-flex: factor, inherit
  if (eCSSUnit_Number == xulData.mBoxFlex.GetUnit()) {
    xul->mBoxFlex = xulData.mBoxFlex.GetFloatValue();
  }
  else if (eCSSUnit_Inherit == xulData.mBoxOrient.GetUnit()) {
    inherited = PR_TRUE;
    xul->mBoxFlex = parentXUL->mBoxFlex;
  }

  // box-orient: enum, inherit
  if (eCSSUnit_Enumerated == xulData.mBoxOrient.GetUnit()) {
    xul->mBoxOrient = xulData.mBoxOrient.GetIntValue();
  }
  else if (eCSSUnit_Inherit == xulData.mBoxOrient.GetUnit()) {
    inherited = PR_TRUE;
    xul->mBoxOrient = parentXUL->mBoxOrient;
  }

  // box-pack: enum, inherit
  if (eCSSUnit_Enumerated == xulData.mBoxPack.GetUnit()) {
    xul->mBoxPack = xulData.mBoxPack.GetIntValue();
  }
  else if (eCSSUnit_Inherit == xulData.mBoxPack.GetUnit()) {
    inherited = PR_TRUE;
    xul->mBoxPack = parentXUL->mBoxPack;
  }

  // box-ordinal-group: integer
  if (eCSSUnit_Integer == xulData.mBoxOrdinal.GetUnit()) {
    xul->mBoxOrdinal = xulData.mBoxOrdinal.GetIntValue();
  }

  COMPUTE_END_RESET(XUL, xul)
}

const nsStyleStruct* 
nsRuleNode::ComputeColumnData(nsStyleStruct* aStartStruct,
                              const nsRuleDataStruct& aData, 
                              nsStyleContext* aContext, 
                              nsRuleNode* aHighestNode,
                              const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(Column, (), column, parent, Column, columnData)

  // column-width: length, auto, inherit
  SetCoord(columnData.mColumnWidth,
           column->mColumnWidth, parent->mColumnWidth, SETCOORD_LAH,
           aContext, mPresContext, inherited);

  // column-gap: length, percentage, inherit
  SetCoord(columnData.mColumnGap,
           column->mColumnGap, parent->mColumnGap, SETCOORD_LPH,
           aContext, mPresContext, inherited);

  // column-count: auto, integer, inherit
  if (eCSSUnit_Auto == columnData.mColumnCount.GetUnit()) {
    column->mColumnCount = NS_STYLE_COLUMN_COUNT_AUTO;
  } else if (eCSSUnit_Integer == columnData.mColumnCount.GetUnit()) {
    column->mColumnCount = columnData.mColumnCount.GetIntValue();
  } else if (eCSSUnit_Inherit == columnData.mColumnCount.GetUnit()) {
    inherited = PR_TRUE;
    column->mColumnCount = parent->mColumnCount;
  }

  COMPUTE_END_RESET(Column, column)
}

#ifdef MOZ_SVG
static void
SetSVGPaint(const nsCSSValue& aValue, const nsStyleSVGPaint& parentPaint,
            nsPresContext* aPresContext, nsStyleContext *aContext, 
            nsStyleSVGPaint& aResult, PRBool& aInherited)
{
  if (aValue.GetUnit() == eCSSUnit_Inherit) {
    aResult = parentPaint;
    aInherited = PR_TRUE;
  } else if (aValue.GetUnit() == eCSSUnit_None) {
    aResult.mType = eStyleSVGPaintType_None;
  } else if (aValue.GetUnit() == eCSSUnit_URL) {
    aResult.mType = eStyleSVGPaintType_Server;
    aResult.mPaint.mPaintServer = aValue.GetURLValue();
    NS_IF_ADDREF(aResult.mPaint.mPaintServer);
  } else if (SetColor(aValue, parentPaint.mPaint.mColor, aPresContext, aContext, aResult.mPaint.mColor, aInherited)) {
    aResult.mType = eStyleSVGPaintType_Color;
  }
}

static void
SetSVGOpacity(const nsCSSValue& aValue, float parentOpacity, float& opacity, PRBool& aInherited)
{
  if (aValue.GetUnit() == eCSSUnit_Inherit) {
    opacity = parentOpacity;
    aInherited = PR_TRUE;
  }
  else if (aValue.GetUnit() == eCSSUnit_Number) {
    opacity = aValue.GetFloatValue();
    opacity = PR_MAX(opacity, 0.0f);
    opacity = PR_MIN(opacity, 1.0f);
  }
}

const nsStyleStruct* 
nsRuleNode::ComputeSVGData(nsStyleStruct* aStartStruct,
                           const nsRuleDataStruct& aData, 
                           nsStyleContext* aContext, 
                           nsRuleNode* aHighestNode,
                           const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_INHERITED(SVG, (), svg, parentSVG, SVG, SVGData)

  // clip-rule: enum, inherit
  if (eCSSUnit_Enumerated == SVGData.mClipRule.GetUnit()) {
    svg->mClipRule = SVGData.mClipRule.GetIntValue();
  }
  else if (eCSSUnit_Inherit == SVGData.mClipRule.GetUnit()) {
    inherited = PR_TRUE;
    svg->mClipRule = parentSVG->mClipRule;
  }

  // fill: 
  SetSVGPaint(SVGData.mFill, parentSVG->mFill, mPresContext, aContext, svg->mFill, inherited);

  // fill-opacity:
  SetSVGOpacity(SVGData.mFillOpacity, parentSVG->mFillOpacity, svg->mFillOpacity, inherited);

  // fill-rule: enum, inherit
  if (eCSSUnit_Enumerated == SVGData.mFillRule.GetUnit()) {
    svg->mFillRule = SVGData.mFillRule.GetIntValue();
  }
  else if (eCSSUnit_Inherit == SVGData.mFillRule.GetUnit()) {
    inherited = PR_TRUE;
    svg->mFillRule = parentSVG->mFillRule;
  }
  
  // marker-end: url, none, inherit
  if (eCSSUnit_URL == SVGData.mMarkerEnd.GetUnit()) {
    svg->mMarkerEnd = SVGData.mMarkerEnd.GetURLValue();
  } else if (eCSSUnit_None == SVGData.mMarkerEnd.GetUnit()) {
    svg->mMarkerEnd = nsnull;
  } else if (eCSSUnit_Inherit == SVGData.mMarkerEnd.GetUnit()) {
    inherited = PR_TRUE;
    svg->mMarkerEnd = parentSVG->mMarkerEnd;
  }

  // marker-mid: url, none, inherit
  if (eCSSUnit_URL == SVGData.mMarkerMid.GetUnit()) {
    svg->mMarkerMid = SVGData.mMarkerMid.GetURLValue();
  } else if (eCSSUnit_None == SVGData.mMarkerMid.GetUnit()) {
    svg->mMarkerMid = nsnull;
  } else if (eCSSUnit_Inherit == SVGData.mMarkerMid.GetUnit()) {
    inherited = PR_TRUE;
    svg->mMarkerMid = parentSVG->mMarkerMid;
  }

  // marker-start: url, none, inherit
  if (eCSSUnit_URL == SVGData.mMarkerStart.GetUnit()) {
    svg->mMarkerStart = SVGData.mMarkerStart.GetURLValue();
  } else if (eCSSUnit_None == SVGData.mMarkerStart.GetUnit()) {
    svg->mMarkerStart = nsnull;
  } else if (eCSSUnit_Inherit == SVGData.mMarkerStart.GetUnit()) {
    inherited = PR_TRUE;
    svg->mMarkerStart = parentSVG->mMarkerStart;
  }

  // pointer-events: enum, inherit
  if (eCSSUnit_Enumerated == SVGData.mPointerEvents.GetUnit()) {
    svg->mPointerEvents = SVGData.mPointerEvents.GetIntValue();
  } else if (eCSSUnit_None == SVGData.mPointerEvents.GetUnit()) {
    svg->mPointerEvents = NS_STYLE_POINTER_EVENTS_NONE;
  } else if (eCSSUnit_Inherit == SVGData.mPointerEvents.GetUnit()) {
    inherited = PR_TRUE;
    svg->mPointerEvents = parentSVG->mPointerEvents;
  }

  // shape-rendering: enum, auto, inherit
  if (eCSSUnit_Enumerated == SVGData.mShapeRendering.GetUnit()) {
    svg->mShapeRendering = SVGData.mShapeRendering.GetIntValue();
  }
  else if (eCSSUnit_Auto == SVGData.mShapeRendering.GetUnit()) {
    svg->mShapeRendering = NS_STYLE_SHAPE_RENDERING_AUTO;
  }
  else if (eCSSUnit_Inherit == SVGData.mShapeRendering.GetUnit()) {
    inherited = PR_TRUE;
    svg->mShapeRendering = parentSVG->mShapeRendering;
  }

  // stroke: 
  SetSVGPaint(SVGData.mStroke, parentSVG->mStroke, mPresContext, aContext, svg->mStroke, inherited);

  // stroke-dasharray: <dasharray>, none, inherit
  nsCSSValueList *list = SVGData.mStrokeDasharray;
  if (list) {
    if (eCSSUnit_Inherit == list->mValue.GetUnit()) {
      // only do the copy if weren't already set up by the copy constructor
      if (!svg->mStrokeDasharray) {
        inherited = PR_TRUE;
        svg->mStrokeDasharrayLength = parentSVG->mStrokeDasharrayLength;
        if (svg->mStrokeDasharrayLength) {
          svg->mStrokeDasharray = new nsStyleCoord[svg->mStrokeDasharrayLength];
          if (svg->mStrokeDasharray)
            memcpy(svg->mStrokeDasharray,
                   parentSVG->mStrokeDasharray,
                   svg->mStrokeDasharrayLength * sizeof(float));
          else
            svg->mStrokeDasharrayLength = 0;
        }
      }
    } else {
      delete [] svg->mStrokeDasharray;
      svg->mStrokeDasharray = nsnull;
      svg->mStrokeDasharrayLength = 0;
      
      if (eCSSUnit_Initial != list->mValue.GetUnit() &&
          eCSSUnit_None    != list->mValue.GetUnit()) {
        // count number of values
        nsCSSValueList *value = SVGData.mStrokeDasharray;
        while (nsnull != value) {
          ++svg->mStrokeDasharrayLength;
          value = value->mNext;
        }
        
        NS_ASSERTION(svg->mStrokeDasharrayLength != 0, "no dasharray items");
        
        svg->mStrokeDasharray = new nsStyleCoord[svg->mStrokeDasharrayLength];

        if (svg->mStrokeDasharray) {
          value = SVGData.mStrokeDasharray;
          PRUint32 i = 0;
          while (nsnull != value) {
            SetCoord(value->mValue,
                     svg->mStrokeDasharray[i++], nsnull,
                     SETCOORD_LP | SETCOORD_FACTOR,
                     aContext, mPresContext, inherited);
            value = value->mNext;
          }
        } else
          svg->mStrokeDasharrayLength = 0;
      }
    }
  }

  // stroke-dashoffset: <dashoffset>, inherit
  SetCoord(SVGData.mStrokeDashoffset,
           svg->mStrokeDashoffset, parentSVG->mStrokeDashoffset,
           SETCOORD_LPH | SETCOORD_FACTOR,
           aContext, mPresContext, inherited);

  // stroke-linecap: enum, inherit
  if (eCSSUnit_Enumerated == SVGData.mStrokeLinecap.GetUnit()) {
    svg->mStrokeLinecap = SVGData.mStrokeLinecap.GetIntValue();
  }
  else if (eCSSUnit_Inherit == SVGData.mStrokeLinecap.GetUnit()) {
    inherited = PR_TRUE;
    svg->mStrokeLinecap = parentSVG->mStrokeLinecap;
  }

  // stroke-linejoin: enum, inherit
  if (eCSSUnit_Enumerated == SVGData.mStrokeLinejoin.GetUnit()) {
    svg->mStrokeLinejoin = SVGData.mStrokeLinejoin.GetIntValue();
  }
  else if (eCSSUnit_Inherit == SVGData.mStrokeLinejoin.GetUnit()) {
    inherited = PR_TRUE;
    svg->mStrokeLinejoin = parentSVG->mStrokeLinejoin;
  }

  // stroke-miterlimit: <miterlimit>, inherit
  if (eCSSUnit_Number == SVGData.mStrokeMiterlimit.GetUnit()) {
    svg->mStrokeMiterlimit = SVGData.mStrokeMiterlimit.GetFloatValue();
  }
  else if (eCSSUnit_Inherit == SVGData.mStrokeMiterlimit.GetUnit()) {
    svg->mStrokeMiterlimit = parentSVG->mStrokeMiterlimit;
    inherited = PR_TRUE;
  }

  // stroke-opacity:
  SetSVGOpacity(SVGData.mStrokeOpacity, parentSVG->mStrokeOpacity, svg->mStrokeOpacity, inherited);  

  // stroke-width:
  SetCoord(SVGData.mStrokeWidth,
           svg->mStrokeWidth, parentSVG->mStrokeWidth,
           SETCOORD_LPH | SETCOORD_FACTOR,
           aContext, mPresContext, inherited);

  // text-anchor: enum, inherit
  if (eCSSUnit_Enumerated == SVGData.mTextAnchor.GetUnit()) {
    svg->mTextAnchor = SVGData.mTextAnchor.GetIntValue();
  }
  else if (eCSSUnit_Inherit == SVGData.mTextAnchor.GetUnit()) {
    inherited = PR_TRUE;
    svg->mTextAnchor = parentSVG->mTextAnchor;
  }
  
  // text-rendering: enum, auto, inherit
  if (eCSSUnit_Enumerated == SVGData.mTextRendering.GetUnit()) {
    svg->mTextRendering = SVGData.mTextRendering.GetIntValue();
  }
  else if (eCSSUnit_Auto == SVGData.mTextRendering.GetUnit()) {
    svg->mTextRendering = NS_STYLE_TEXT_RENDERING_AUTO;
  }
  else if (eCSSUnit_Inherit == SVGData.mTextRendering.GetUnit()) {
    inherited = PR_TRUE;
    svg->mTextRendering = parentSVG->mTextRendering;
  }

  COMPUTE_END_INHERITED(SVG, svg)
}

const nsStyleStruct* 
nsRuleNode::ComputeSVGResetData(nsStyleStruct* aStartStruct,
                                const nsRuleDataStruct& aData,
                                nsStyleContext* aContext, 
                                nsRuleNode* aHighestNode,
                                const RuleDetail& aRuleDetail, PRBool aInherited)
{
  COMPUTE_START_RESET(SVGReset, (), svgReset, parentSVGReset, SVG, SVGData)

  // stop-color: 
  SetSVGPaint(SVGData.mStopColor, parentSVGReset->mStopColor,
              mPresContext, aContext, svgReset->mStopColor, inherited);

  // clip-path: url, none, inherit
  if (eCSSUnit_URL == SVGData.mClipPath.GetUnit()) {
    svgReset->mClipPath = SVGData.mClipPath.GetURLValue();
  } else if (eCSSUnit_None == SVGData.mClipPath.GetUnit()) {
    svgReset->mClipPath = nsnull;
  } else if (eCSSUnit_Inherit == SVGData.mClipPath.GetUnit()) {
    inherited = PR_TRUE;
    svgReset->mClipPath = parentSVGReset->mClipPath;
  }

  // stop-opacity:
  SetSVGOpacity(SVGData.mStopOpacity, parentSVGReset->mStopOpacity,
                svgReset->mStopOpacity, inherited);

  // dominant-baseline: enum, auto, inherit
  if (eCSSUnit_Enumerated == SVGData.mDominantBaseline.GetUnit()) {
    svgReset->mDominantBaseline = SVGData.mDominantBaseline.GetIntValue();
  }
  else if (eCSSUnit_Auto == SVGData.mDominantBaseline.GetUnit()) {
    svgReset->mDominantBaseline = NS_STYLE_DOMINANT_BASELINE_AUTO;
  }
  else if (eCSSUnit_Inherit == SVGData.mDominantBaseline.GetUnit()) {
    inherited = PR_TRUE;
    svgReset->mDominantBaseline = parentSVGReset->mDominantBaseline;
  }

  // filter: url, none, inherit
  if (eCSSUnit_URL == SVGData.mFilter.GetUnit()) {
    svgReset->mFilter = SVGData.mFilter.GetURLValue();
  } else if (eCSSUnit_None == SVGData.mFilter.GetUnit()) {
    svgReset->mFilter = nsnull;
  } else if (eCSSUnit_Inherit == SVGData.mFilter.GetUnit()) {
    inherited = PR_TRUE;
    svgReset->mFilter = parentSVGReset->mFilter;
  }

  // mask: url, none, inherit
  if (eCSSUnit_URL == SVGData.mMask.GetUnit()) {
    svgReset->mMask = SVGData.mMask.GetURLValue();
  } else if (eCSSUnit_None == SVGData.mMask.GetUnit()) {
    svgReset->mMask = nsnull;
  } else if (eCSSUnit_Inherit == SVGData.mMask.GetUnit()) {
    inherited = PR_TRUE;
    svgReset->mMask = parentSVGReset->mMask;
  }
  
  COMPUTE_END_RESET(SVGReset, svgReset)
}
#endif

inline const nsStyleStruct* 
nsRuleNode::GetParentData(const nsStyleStructID aSID)
{
  NS_PRECONDITION(mDependentBits & nsCachedStyleData::GetBitForSID(aSID),
                  "should be called when node depends on parent data");
  NS_ASSERTION(mStyleData.GetStyleData(aSID) == nsnull,
               "both struct and dependent bits present");
  // Walk up the rule tree from this rule node (towards less specific
  // rules).
  PRUint32 bit = nsCachedStyleData::GetBitForSID(aSID);
  nsRuleNode *ruleNode = mParent;
  while (ruleNode->mDependentBits & bit) {
    NS_ASSERTION(ruleNode->mStyleData.GetStyleData(aSID) == nsnull,
                 "both struct and dependent bits present");
    ruleNode = ruleNode->mParent;
  }

  return ruleNode->mStyleData.GetStyleData(aSID);
}

const nsStyleStruct* 
nsRuleNode::GetStyleData(nsStyleStructID aSID, 
                         nsStyleContext* aContext,
                         PRBool aComputeData)
{
  const nsStyleStruct *data;
  if (mDependentBits & nsCachedStyleData::GetBitForSID(aSID)) {
    // We depend on an ancestor for this struct since the cached struct
    // it has is also appropriate for this rule node.  Just go up the
    // rule tree and return the first cached struct we find.
    data = GetParentData(aSID);
    NS_ASSERTION(data, "dependent bits set but no cached struct present");
    return data;
  }

  data = mStyleData.GetStyleData(aSID);
  if (NS_LIKELY(data != nsnull))
    return data; // We have a fully specified struct. Just return it.

  if (NS_UNLIKELY(!aComputeData))
    return nsnull;

  // Nothing is cached.  We'll have to delve further and examine our rules.
#define STYLE_STRUCT_TEST aSID
#define STYLE_STRUCT(name, checkdata_cb, ctor_args) \
  data = Get##name##Data(aContext);
#include "nsStyleStructList.h"
#undef STYLE_STRUCT
#undef STYLE_STRUCT_TEST

  if (NS_LIKELY(data != nsnull))
    return data;

  NS_NOTREACHED("could not create style struct");
  // To ensure that |GetStyleData| never returns null (even when we're
  // out of memory), we'll get the style set and get a copy of the
  // default values for the given style struct from the set.  Note that
  // this works fine even if |this| is a rule node that has been
  // destroyed (leftover from a previous rule tree) but is somehow still
  // used.
  return mPresContext->PresShell()->StyleSet()->
    DefaultStyleData()->GetStyleData(aSID);
}

void
nsRuleNode::Mark()
{
  for (nsRuleNode *node = this;
       node && !(node->mDependentBits & NS_RULE_NODE_GC_MARK);
       node = node->mParent)
    node->mDependentBits |= NS_RULE_NODE_GC_MARK;
}

PR_STATIC_CALLBACK(PLDHashOperator)
SweepRuleNodeChildren(PLDHashTable *table, PLDHashEntryHdr *hdr,
                      PRUint32 number, void *arg)
{
  ChildrenHashEntry *entry = NS_STATIC_CAST(ChildrenHashEntry*, hdr);
  if (entry->mRuleNode->Sweep())
    return PL_DHASH_REMOVE; // implies NEXT, unless |ed with STOP
  return PL_DHASH_NEXT;
}

PRBool
nsRuleNode::Sweep()
{
  // If we're not marked, then we have to delete ourself.
  // However, we never allow the root node to GC itself, because nsStyleSet
  // wants to hold onto the root node and not worry about re-creating a
  // rule walker if the root node is deleted.
  if (!(mDependentBits & NS_RULE_NODE_GC_MARK) && !IsRoot()) {
    Destroy();
    return PR_TRUE;
  }

  // Clear our mark, for the next time around.
  mDependentBits &= ~NS_RULE_NODE_GC_MARK;

  // Call sweep on the children, since some may not be marked, and
  // remove any deleted children from the child lists.
  if (HaveChildren()) {
    if (ChildrenAreHashed()) {
      PLDHashTable *children = ChildrenHash();
      PL_DHashTableEnumerate(children, SweepRuleNodeChildren, nsnull);
    } else {
      for (nsRuleList **children = ChildrenListPtr(); *children; ) {
        if ((*children)->mRuleNode->Sweep()) {
          // This rule node was destroyed, so remove this entry, and
          // implicitly advance by making *children point to the next
          // entry.
          *children = (*children)->DestroySelf(mPresContext);
        } else {
          // Advance.
          children = &(*children)->mNext;
        }
      }
    }
  }
  return PR_FALSE;
}
