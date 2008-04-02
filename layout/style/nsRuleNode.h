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
 *   L. David Baron <dbaron@dbaron.org>
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

#ifndef nsRuleNode_h___
#define nsRuleNode_h___

#include "nsPresContext.h"
#include "nsStyleStruct.h"

class nsStyleContext;
struct nsRuleList;
struct PLDHashTable;
class nsILanguageAtomService;
struct nsRuleData;
class nsIStyleRule;
struct nsCSSStruct;
// Copy of typedef that's in nsCSSStruct.h, for compilation speed.
typedef nsCSSStruct nsRuleDataStruct;

struct nsRuleDataFont;
class nsCSSValue;
struct nsCSSRect;

struct nsInheritedStyleData
{

#define STYLE_STRUCT_INHERITED(name, checkdata_cb, ctor_args) \
  nsStyle##name * m##name##Data;
#define STYLE_STRUCT_RESET(name, checkdata_cb, ctor_args)

#include "nsStyleStructList.h"

#undef STYLE_STRUCT_INHERITED
#undef STYLE_STRUCT_RESET

  void* operator new(size_t sz, nsPresContext* aContext) CPP_THROW_NEW {
    return aContext->AllocateFromShell(sz);
  }

  void ClearInheritedData(PRUint32 aBits) {
#define STYLE_STRUCT_INHERITED(name, checkdata_cb, ctor_args) \
    if (m##name##Data && (aBits & NS_STYLE_INHERIT_BIT(name))) \
      m##name##Data = nsnull;
#define STYLE_STRUCT_RESET(name, checkdata_cb, ctor_args)

#include "nsStyleStructList.h"

#undef STYLE_STRUCT_INHERITED
#undef STYLE_STRUCT_RESET
  }

  void Destroy(PRUint32 aBits, nsPresContext* aContext) {
#define STYLE_STRUCT_INHERITED(name, checkdata_cb, ctor_args) \
    if (m##name##Data && !(aBits & NS_STYLE_INHERIT_BIT(name))) \
      m##name##Data->Destroy(aContext);
#define STYLE_STRUCT_RESET(name, checkdata_cb, ctor_args)

#include "nsStyleStructList.h"

#undef STYLE_STRUCT_INHERITED
#undef STYLE_STRUCT_RESET

    aContext->FreeToShell(sizeof(nsInheritedStyleData), this);
  }

  nsInheritedStyleData() {
#define STYLE_STRUCT_INHERITED(name, checkdata_cb, ctor_args) \
    m##name##Data = nsnull;
#define STYLE_STRUCT_RESET(name, checkdata_cb, ctor_args)

#include "nsStyleStructList.h"

#undef STYLE_STRUCT_INHERITED
#undef STYLE_STRUCT_RESET

  }
};

struct nsResetStyleData
{
  nsResetStyleData()
  {
#define STYLE_STRUCT_RESET(name, checkdata_cb, ctor_args) \
    m##name##Data = nsnull;
#define STYLE_STRUCT_INHERITED(name, checkdata_cb, ctor_args)

#include "nsStyleStructList.h"

#undef STYLE_STRUCT_RESET
#undef STYLE_STRUCT_INHERITED
  }

  void* operator new(size_t sz, nsPresContext* aContext) CPP_THROW_NEW {
    return aContext->AllocateFromShell(sz);
  }

  void ClearInheritedData(PRUint32 aBits) {
#define STYLE_STRUCT_RESET(name, checkdata_cb, ctor_args) \
    if (m##name##Data && (aBits & NS_STYLE_INHERIT_BIT(name))) \
      m##name##Data = nsnull;
#define STYLE_STRUCT_INHERITED(name, checkdata_cb, ctor_args)

#include "nsStyleStructList.h"

#undef STYLE_STRUCT_RESET
#undef STYLE_STRUCT_INHERITED
  }

  void Destroy(PRUint32 aBits, nsPresContext* aContext) {
#define STYLE_STRUCT_RESET(name, checkdata_cb, ctor_args) \
    if (m##name##Data && !(aBits & NS_STYLE_INHERIT_BIT(name))) \
      m##name##Data->Destroy(aContext);
#define STYLE_STRUCT_INHERITED(name, checkdata_cb, ctor_args)

#include "nsStyleStructList.h"

#undef STYLE_STRUCT_RESET
#undef STYLE_STRUCT_INHERITED

    aContext->FreeToShell(sizeof(nsResetStyleData), this);
  }

#define STYLE_STRUCT_RESET(name, checkdata_cb, ctor_args) \
  nsStyle##name * m##name##Data;
#define STYLE_STRUCT_INHERITED(name, checkdata_cb, ctor_args)

#include "nsStyleStructList.h"

#undef STYLE_STRUCT_RESET
#undef STYLE_STRUCT_INHERITED

};

struct nsCachedStyleData
{
  struct StyleStructInfo {
    ptrdiff_t mCachedStyleDataOffset;
    ptrdiff_t mInheritResetOffset;
    PRBool    mIsReset;
  };

  static StyleStructInfo gInfo[];

  nsInheritedStyleData* mInheritedData;
  nsResetStyleData* mResetData;

  static PRBool IsReset(const nsStyleStructID& aSID) {
    return gInfo[aSID].mIsReset;
  }

  static PRUint32 GetBitForSID(const nsStyleStructID& aSID) {
    return 1 << aSID;
  }

  NS_HIDDEN_(void*) NS_FASTCALL GetStyleData(const nsStyleStructID& aSID) {
    // Each struct is stored at this.m##type##Data->m##name##Data where
    // |type| is either Inherit or Reset, and |name| is the name of the
    // style struct.  The |gInfo| stores the offset of the appropriate
    // m##type##Data for the struct within nsCachedStyleData (|this|)
    // and the offset of the appropriate m##name##Data within the
    // m##type##Data.  Note that if we don't have any reset structs,
    // then mResetData is null, and likewise for mInheritedData.  This
    // saves us from having to go through the long if-else cascade into
    // which most compilers will turn a case statement.

    // NOTE:  nsStyleContext::SetStyle works roughly the same way.

    const StyleStructInfo& info = gInfo[aSID];

    // Get either &mInheritedData or &mResetData.
    char* resetOrInheritSlot = reinterpret_cast<char*>(this) + info.mCachedStyleDataOffset;

    // Get either mInheritedData or mResetData.
    char* resetOrInherit = reinterpret_cast<char*>(*reinterpret_cast<void**>(resetOrInheritSlot));

    void* data = nsnull;
    if (resetOrInherit) {
      // If we have the mInheritedData or mResetData, then we might have
      // the struct, so get it.
      char* dataSlot = resetOrInherit + info.mInheritResetOffset;
      data = *reinterpret_cast<void**>(dataSlot);
    }
    return data;
  }

  // Typesafe and faster versions of the above
  #define STYLE_STRUCT_INHERITED(name_, checkdata_cb_, ctor_args_)       \
    NS_HIDDEN_(nsStyle##name_ *) NS_FASTCALL GetStyle##name_ () {        \
      return mInheritedData ? mInheritedData->m##name_##Data : nsnull;   \
    }
  #define STYLE_STRUCT_RESET(name_, checkdata_cb_, ctor_args_)           \
    NS_HIDDEN_(nsStyle##name_ *) NS_FASTCALL GetStyle##name_ () {        \
      return mResetData ? mResetData->m##name_##Data : nsnull;           \
    }
  #include "nsStyleStructList.h"
  #undef STYLE_STRUCT_RESET
  #undef STYLE_STRUCT_INHERITED

  NS_HIDDEN_(void) ClearInheritedData(PRUint32 aBits) {
    if (mResetData)
      mResetData->ClearInheritedData(aBits);
    if (mInheritedData)
      mInheritedData->ClearInheritedData(aBits);
  }

  NS_HIDDEN_(void) Destroy(PRUint32 aBits, nsPresContext* aContext) {
    if (mResetData)
      mResetData->Destroy(aBits, aContext);
    if (mInheritedData)
      mInheritedData->Destroy(aBits, aContext);
    mResetData = nsnull;
    mInheritedData = nsnull;
  }

  nsCachedStyleData() :mInheritedData(nsnull), mResetData(nsnull) {}
  ~nsCachedStyleData() {}
};

/**
 * nsRuleNode is a node in a lexicographic tree (the "rule tree")
 * indexed by style rules (implementations of nsIStyleRule).
 *
 * The rule tree is owned by the nsStyleSet and is destroyed when the
 * presentation of the document goes away.  It is garbage-collected
 * (using mark-and-sweep garbage collection) during the lifetime of the
 * document (when dynamic changes cause the destruction of enough style
 * contexts).  Rule nodes are marked if they are pointed to by a style
 * context or one of their descendants is.
 *
 * An nsStyleContext, which represents the computed style data for an
 * element, points to an nsRuleNode.  The path from the root of the rule
 * tree to the nsStyleContext's mRuleNode gives the list of the rules
 * matched, from least important in the cascading order to most
 * important in the cascading order.
 *
 * The reason for using a lexicographic tree is that it allows for
 * sharing of style data, which saves both memory (for storing the
 * computed style data) and time (for computing them).  This sharing
 * depends on the computed style data being stored in structs (nsStyle*)
 * that contain only properties that are inherited by default
 * ("inherited structs") or structs that contain only properties that
 * are not inherited by default ("reset structs").  The optimization
 * depends on the normal case being that style rules specify relatively
 * few properties and even that elements generally have relatively few
 * properties specified.  This allows sharing in the following ways:
 *   1. [mainly reset structs] When a style data struct will contain the
 *      same computed value for any elements that match the same set of
 *      rules (common for reset structs), it can be stored on the
 *      nsRuleNode instead of on the nsStyleContext.
 *   2. [only? reset structs] When (1) occurs, and an nsRuleNode doesn't
 *      have any rules that change the values in the struct, the
 *      nsRuleNode can share that struct with its parent nsRuleNode.
 *   3. [mainly inherited structs] When an element doesn't match any
 *      rules that change the value of a property (or, in the edge case,
 *      when all the values specified are 'inherit'), the nsStyleContext
 *      can use the same nsStyle* struct as its parent nsStyleContext.
 *
 * Since the data represented by an nsIStyleRule are immutable, the data
 * represented by an nsRuleNode are also immutable.
 */

class nsRuleNode {
public:
  enum RuleDetail {
    eRuleNone, // No props have been specified at all.
    eRulePartialReset, // At least one prop with a non-"inherit" value
                       // has been specified.  No props have been
                       // specified with an "inherit" value.  At least
                       // one prop remains unspecified.
    eRulePartialMixed, // At least one prop with a non-"inherit" value
                       // has been specified.  Some props may also have
                       // been specified with an "inherit" value.  At
                       // least one prop remains unspecified.
    eRulePartialInherited, // Only props with "inherit" values have
                           // have been specified.  At least one prop
                           // remains unspecified.
    eRuleFullReset, // All props have been specified.  None has an
                    // "inherit" value.
    eRuleFullMixed, // All props have been specified.  At least one has
                    // a non-"inherit" value.
    eRuleFullInherited  // All props have been specified with "inherit"
                        // values.
  };

private:
  nsPresContext* mPresContext; // Our pres context.

  nsRuleNode* mParent; // A pointer to the parent node in the tree.
                       // This enables us to walk backwards from the
                       // most specific rule matched to the least
                       // specific rule (which is the optimal order to
                       // use for lookups of style properties.
  nsIStyleRule* mRule; // [STRONG] A pointer to our specific rule.

  struct Key {
    nsIStyleRule* mRule;
    PRUint8 mLevel;
    PRPackedBool mIsImportantRule;

    Key(nsIStyleRule* aRule, PRUint8 aLevel, PRPackedBool aIsImportantRule)
      : mRule(aRule), mLevel(aLevel), mIsImportantRule(aIsImportantRule)
    {}

    PRBool operator==(const Key& aOther) const
    {
      return mRule == aOther.mRule &&
             mLevel == aOther.mLevel &&
             mIsImportantRule == aOther.mIsImportantRule;
    }

    PRBool operator!=(const Key& aOther) const
    {
      return !(*this == aOther);
    }
  };

  static PR_CALLBACK PLDHashNumber
  ChildrenHashHashKey(PLDHashTable *aTable, const void *aKey);

  static PR_CALLBACK PRBool
  ChildrenHashMatchEntry(PLDHashTable *aTable,
                         const PLDHashEntryHdr *aHdr,
                         const void *aKey);

  static PLDHashTableOps ChildrenHashOps;

  Key GetKey() const {
    return Key(mRule, GetLevel(), IsImportantRule());
  }

  // The children of this node are stored in either a hashtable or list
  // that maps from rules to our nsRuleNode children.  When matching
  // rules, we use this mapping to transition from node to node
  // (constructing new nodes as needed to flesh out the tree).

  void *mChildrenTaggedPtr; // Accessed only through the methods below.

  enum {
    kTypeMask = 0x1,
    kListType = 0x0,
    kHashType = 0x1
  };
  enum {
    // Maximum to have in a list before converting to a hashtable.
    // XXX Need to optimize this.
    kMaxChildrenInList = 32
  };

  PRBool HaveChildren() {
    return mChildrenTaggedPtr != nsnull;
  }
  PRBool ChildrenAreHashed() {
    return (PRWord(mChildrenTaggedPtr) & kTypeMask) == kHashType;
  }
  nsRuleList* ChildrenList() {
    return reinterpret_cast<nsRuleList*>(mChildrenTaggedPtr);
  }
  nsRuleList** ChildrenListPtr() {
    return reinterpret_cast<nsRuleList**>(&mChildrenTaggedPtr);
  }
  PLDHashTable* ChildrenHash() {
    return (PLDHashTable*) (PRWord(mChildrenTaggedPtr) & ~PRWord(kTypeMask));
  }
  void SetChildrenList(nsRuleList *aList) {
    NS_ASSERTION(!(PRWord(aList) & kTypeMask),
                 "pointer not 2-byte aligned");
    mChildrenTaggedPtr = aList;
  }
  void SetChildrenHash(PLDHashTable *aHashtable) {
    NS_ASSERTION(!(PRWord(aHashtable) & kTypeMask),
                 "pointer not 2-byte aligned");
    mChildrenTaggedPtr = (void*)(PRWord(aHashtable) | kHashType);
  }
  void ConvertChildrenToHash();

  nsCachedStyleData mStyleData;   // Any data we cached on the rule node.

  PRUint32 mDependentBits; // Used to cache the fact that we can look up
                           // cached data under a parent rule.

  PRUint32 mNoneBits; // Used to cache the fact that the branch to this
                      // node specifies no non-inherited data for a
                      // given struct type.  (This usually implies that
                      // the entire branch specifies no non-inherited
                      // data, although not necessarily, if a
                      // non-inherited value is overridden by an
                      // explicit 'inherit' value.)  For example, if an
                      // entire rule branch specifies no color
                      // information, then a bit will be set along every
                      // rule node on that branch, so that you can break
                      // out of the rule tree early and just inherit
                      // from the parent style context.  The presence of
                      // this bit means we should just get inherited
                      // data from the parent style context, and it is
                      // never used for reset structs since their
                      // Compute*Data functions don't initialize from
                      // inherited data.

friend struct nsRuleList;

public:
  // Overloaded new operator. Initializes the memory to 0 and relies on an arena
  // (which comes from the presShell) to perform the allocation.
  NS_HIDDEN_(void*) operator new(size_t sz, nsPresContext* aContext) CPP_THROW_NEW;
  NS_HIDDEN_(void) Destroy();
  static NS_HIDDEN_(nsILanguageAtomService*) gLangService;

protected:
  NS_HIDDEN_(void) PropagateDependentBit(PRUint32 aBit,
                                         nsRuleNode* aHighestNode);
  NS_HIDDEN_(void) PropagateNoneBit(PRUint32 aBit, nsRuleNode* aHighestNode);
  
  NS_HIDDEN_(const void*) SetDefaultOnRoot(const nsStyleStructID aSID,
                                                 nsStyleContext* aContext);

  NS_HIDDEN_(const void*)
    WalkRuleTree(const nsStyleStructID aSID, nsStyleContext* aContext, 
                 nsRuleData* aRuleData, nsRuleDataStruct* aSpecificData);

  NS_HIDDEN_(const void*)
    ComputeDisplayData(void* aStartStruct,
                       const nsRuleDataStruct& aData,
                       nsStyleContext* aContext, nsRuleNode* aHighestNode,
                       RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeVisibilityData(void* aStartStruct,
                          const nsRuleDataStruct& aData,
                          nsStyleContext* aContext, nsRuleNode* aHighestNode,
                          RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeFontData(void* aStartStruct,
                    const nsRuleDataStruct& aData,
                    nsStyleContext* aContext, nsRuleNode* aHighestNode,
                    RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeColorData(void* aStartStruct,
                     const nsRuleDataStruct& aData,
                     nsStyleContext* aContext, nsRuleNode* aHighestNode,
                     RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeBackgroundData(void* aStartStruct,
                          const nsRuleDataStruct& aData, 
                          nsStyleContext* aContext, nsRuleNode* aHighestNode,
                          RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeMarginData(void* aStartStruct,
                      const nsRuleDataStruct& aData, 
                      nsStyleContext* aContext, nsRuleNode* aHighestNode,
                      RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeBorderData(void* aStartStruct,
                      const nsRuleDataStruct& aData, 
                      nsStyleContext* aContext, nsRuleNode* aHighestNode,
                      RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputePaddingData(void* aStartStruct,
                       const nsRuleDataStruct& aData, 
                       nsStyleContext* aContext, nsRuleNode* aHighestNode,
                       RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeOutlineData(void* aStartStruct,
                       const nsRuleDataStruct& aData, 
                       nsStyleContext* aContext, nsRuleNode* aHighestNode,
                       RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeListData(void* aStartStruct,
                    const nsRuleDataStruct& aData,
                    nsStyleContext* aContext, nsRuleNode* aHighestNode,
                    RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputePositionData(void* aStartStruct,
                        const nsRuleDataStruct& aData, 
                        nsStyleContext* aContext, nsRuleNode* aHighestNode,
                        RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeTableData(void* aStartStruct,
                     const nsRuleDataStruct& aData, 
                     nsStyleContext* aContext, nsRuleNode* aHighestNode,
                     RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeTableBorderData(void* aStartStruct,
                           const nsRuleDataStruct& aData, 
                           nsStyleContext* aContext, nsRuleNode* aHighestNode,
                           RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeContentData(void* aStartStruct,
                       const nsRuleDataStruct& aData,
                       nsStyleContext* aContext, nsRuleNode* aHighestNode,
                       RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeQuotesData(void* aStartStruct,
                      const nsRuleDataStruct& aData, 
                      nsStyleContext* aContext, nsRuleNode* aHighestNode,
                      RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeTextData(void* aStartStruct,
                    const nsRuleDataStruct& aData, 
                    nsStyleContext* aContext, nsRuleNode* aHighestNode,
                    RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeTextResetData(void* aStartStruct,
                         const nsRuleDataStruct& aData,
                         nsStyleContext* aContext, nsRuleNode* aHighestNode,
                         RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeUserInterfaceData(void* aStartStruct,
                             const nsRuleDataStruct& aData, 
                             nsStyleContext* aContext,
                             nsRuleNode* aHighestNode,
                             RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeUIResetData(void* aStartStruct,
                       const nsRuleDataStruct& aData,
                       nsStyleContext* aContext, nsRuleNode* aHighestNode,
                       RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeXULData(void* aStartStruct,
                   const nsRuleDataStruct& aData, 
                   nsStyleContext* aContext, nsRuleNode* aHighestNode,
                   RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeColumnData(void* aStartStruct,
                      const nsRuleDataStruct& aData,
                      nsStyleContext* aContext, nsRuleNode* aHighestNode,
                      RuleDetail aRuleDetail, PRBool aInherited);

#ifdef MOZ_SVG
  NS_HIDDEN_(const void*)
    ComputeSVGData(void* aStartStruct,
                   const nsRuleDataStruct& aData, 
                   nsStyleContext* aContext, nsRuleNode* aHighestNode,
                   RuleDetail aRuleDetail, PRBool aInherited);

  NS_HIDDEN_(const void*)
    ComputeSVGResetData(void* aStartStruct,
                        const nsRuleDataStruct& aData, 
                        nsStyleContext* aContext, nsRuleNode* aHighestNode,
                        RuleDetail aRuleDetail, PRBool aInherited);
#endif

  // helpers for |ComputeFontData| that need access to |mNoneBits|:
  static NS_HIDDEN_(void) SetFontSize(nsPresContext* aPresContext,
                                      const nsRuleDataFont& aFontData,
                                      const nsStyleFont* aFont,
                                      const nsStyleFont* aParentFont,
                                      nscoord* aSize,
                                      const nsFont& aSystemFont,
                                      nscoord aParentSize,
                                      nscoord aScriptLevelAdjustedParentSize,
                                      PRBool aUsedStartStruct,
                                      PRBool& aInherited);

  static NS_HIDDEN_(void) SetFont(nsPresContext* aPresContext,
                                  nsStyleContext* aContext,
                                  nscoord aMinFontSize,
                                  PRUint8 aGenericFontID,
                                  const nsRuleDataFont& aFontData,
                                  const nsStyleFont* aParentFont,
                                  nsStyleFont* aFont,
                                  PRBool aStartStruct, PRBool& aInherited);

  static NS_HIDDEN_(void) SetGenericFont(nsPresContext* aPresContext,
                                         nsStyleContext* aContext,
                                         PRUint8 aGenericFontID,
                                         nscoord aMinFontSize,
                                         nsStyleFont* aFont);

  NS_HIDDEN_(void) AdjustLogicalBoxProp(nsStyleContext* aContext,
                                        const nsCSSValue& aLTRSource,
                                        const nsCSSValue& aRTLSource,
                                        const nsCSSValue& aLTRLogicalValue,
                                        const nsCSSValue& aRTLLogicalValue,
                                        PRUint8 aSide,
                                        nsCSSRect& aValueRect,
                                        PRBool& aInherited);

  inline RuleDetail CheckSpecifiedProperties(const nsStyleStructID aSID, const nsRuleDataStruct& aRuleDataStruct);

  NS_HIDDEN_(const void*) GetParentData(const nsStyleStructID aSID);
  #define STYLE_STRUCT(name_, checkdata_cb_, ctor_args_)  \
    NS_HIDDEN_(const nsStyle##name_*) GetParent##name_();
  #include "nsStyleStructList.h"
  #undef STYLE_STRUCT  

  NS_HIDDEN_(const void*) GetDisplayData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetVisibilityData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetFontData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetColorData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetBackgroundData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetMarginData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetBorderData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetPaddingData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetOutlineData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetListData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetPositionData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetTableData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetTableBorderData(nsStyleContext* aContext);

  NS_HIDDEN_(const void*) GetContentData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetQuotesData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetTextData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetTextResetData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetUserInterfaceData(nsStyleContext* aContext);

  NS_HIDDEN_(const void*) GetUIResetData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetXULData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetColumnData(nsStyleContext* aContext);
#ifdef MOZ_SVG
  NS_HIDDEN_(const void*) GetSVGData(nsStyleContext* aContext);
  NS_HIDDEN_(const void*) GetSVGResetData(nsStyleContext* aContext);
#endif

private:
  nsRuleNode(nsPresContext* aPresContext, nsRuleNode* aParent,
             nsIStyleRule* aRule, PRUint8 aLevel, PRBool aIsImportant)
    NS_HIDDEN;
  ~nsRuleNode() NS_HIDDEN;

public:
  static NS_HIDDEN_(nsRuleNode*) CreateRootNode(nsPresContext* aPresContext);

  NS_HIDDEN_(nsRuleNode*) Transition(nsIStyleRule* aRule, PRUint8 aLevel,
                                     PRPackedBool aIsImportantRule);
  nsRuleNode* GetParent() const { return mParent; }
  PRBool IsRoot() const { return mParent == nsnull; }

  // These PRUint8s are really nsStyleSet::sheetType values.
  PRUint8 GetLevel() const { 
    NS_ASSERTION(!IsRoot(), "can't call on root");
    return (mDependentBits & NS_RULE_NODE_LEVEL_MASK) >>
             NS_RULE_NODE_LEVEL_SHIFT;
  }
  PRBool IsImportantRule() const {
    NS_ASSERTION(!IsRoot(), "can't call on root");
    return (mDependentBits & NS_RULE_NODE_IS_IMPORTANT) != 0;
  }

  // NOTE:  Does not |AddRef|.
  nsIStyleRule* GetRule() const { return mRule; }
  // NOTE: Does not |AddRef|.
  nsPresContext* GetPresContext() const { return mPresContext; }

  NS_HIDDEN_(const void*) GetStyleData(nsStyleStructID aSID, 
                                       nsStyleContext* aContext,
                                       PRBool aComputeData);

  #define STYLE_STRUCT(name_, checkdata_cb_, ctor_args_)                      \
    NS_HIDDEN_(const nsStyle##name_*)                                         \
      GetStyle##name_(nsStyleContext* aContext,                               \
                      PRBool aComputeData);
  #include "nsStyleStructList.h"
  #undef STYLE_STRUCT  

  /*
   * Garbage collection.  Mark walks up the tree, marking any unmarked
   * ancestors until it reaches a marked one.  Sweep recursively sweeps
   * the children, destroys any that are unmarked, and clears marks,
   * returning true if the node on which it was called was destroyed.
   */
  NS_HIDDEN_(void) Mark();
  NS_HIDDEN_(PRBool) Sweep();

  static PRBool
    HasAuthorSpecifiedBorderOrBackground(nsStyleContext* aStyleContext);
};

#endif
