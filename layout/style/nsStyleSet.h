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
 *   Daniel Glazman <glazman@netscape.com>
 *   Brian Ryner    <bryner@brianryner.com>
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
 * the container for the style sheets that apply to a presentation, and
 * the internal API that the style system exposes for creating (and
 * potentially re-creating) style contexts
 */

#ifndef nsStyleSet_h_
#define nsStyleSet_h_

#include "nsIStyleRuleProcessor.h"
#include "nsICSSStyleSheet.h"
#include "nsBindingManager.h"
#include "nsRuleNode.h"
#include "nsTArray.h"
#include "nsCOMArray.h"

class nsIURI;

// The style set object is created by the document viewer and ownership is
// then handed off to the PresShell.  Only the PresShell should delete a
// style set.

class nsStyleSet
{
 public:
  nsStyleSet();

  // Initialize the object.  You must check the return code and not use
  // the nsStyleSet if Init() fails.

  nsresult Init(nsPresContext *aPresContext);

  // For getting the cached default data in case we hit out-of-memory.
  // To be used only by nsRuleNode.
  nsCachedStyleData* DefaultStyleData() { return &mDefaultStyleData; }

  // enable / disable the Quirk style sheet
  void EnableQuirkStyleSheet(PRBool aEnable);

  // get a style context for a non-pseudo frame.
  already_AddRefed<nsStyleContext>
  ResolveStyleFor(nsIContent* aContent, nsStyleContext* aParentContext);

  // get a style context from some rules
  already_AddRefed<nsStyleContext>
  ResolveStyleForRules(nsStyleContext* aParentContext, const nsCOMArray<nsIStyleRule> &rules);

  // Get a style context for a non-element (which no rules will match),
  // such as text nodes, placeholder frames, and the nsFirstLetterFrame
  // for everything after the first letter.
  //
  // Perhaps this should go away and we shouldn't even create style
  // contexts for such content nodes.  However, not doing any rule
  // matching for them is a first step.
  already_AddRefed<nsStyleContext>
  ResolveStyleForNonElement(nsStyleContext* aParentContext);

  // get a style context for a pseudo-element (i.e.,
  // |aPseudoTag == nsCOMPtr<nsIAtom>(do_GetAtom(":first-line"))|, in
  // which case aParentContent must be non-null, or an anonymous box, in
  // which case it may be null or non-null.
  already_AddRefed<nsStyleContext>
  ResolvePseudoStyleFor(nsIContent* aParentContent,
                        nsIAtom* aPseudoTag,
                        nsStyleContext* aParentContext,
                        nsICSSPseudoComparator* aComparator = nsnull);

  // This functions just like ResolvePseudoStyleFor except that it will
  // return nsnull if there are no explicit style rules for that
  // pseudo element.  It should be used only for pseudo-elements.
  already_AddRefed<nsStyleContext>
  ProbePseudoStyleFor(nsIContent* aParentContent,
                      nsIAtom* aPseudoTag,
                      nsStyleContext* aParentContext);

  // Begin ignoring style context destruction, to avoid lots of unnecessary
  // work on document teardown.
  void BeginShutdown(nsPresContext* aPresContext);

  // Free all of the data associated with this style set.
  void Shutdown(nsPresContext* aPresContext);

  // Notification that a style context is being destroyed.
  void NotifyStyleContextDestroyed(nsPresContext* aPresContext,
                                   nsStyleContext* aStyleContext);

  // Get a new style context that lives in a different parent
  // The new context will be the same as the old if the new parent is the
  // same as the old parent.
  already_AddRefed<nsStyleContext>
    ReParentStyleContext(nsPresContext* aPresContext,
                         nsStyleContext* aStyleContext,
                         nsStyleContext* aNewParentContext);

  // Test if style is dependent on content state
  nsReStyleHint HasStateDependentStyle(nsPresContext* aPresContext,
                                       nsIContent*     aContent,
                                       PRInt32         aStateMask);

  // Test if style is dependent on the presence of an attribute.
  nsReStyleHint HasAttributeDependentStyle(nsPresContext* aPresContext,
                                           nsIContent*    aContent,
                                           nsIAtom*       aAttribute,
                                           PRInt32        aModType,
                                           PRUint32       aStateMask);

  // APIs for registering objects that can supply additional
  // rules during processing.
  void SetBindingManager(nsBindingManager* aBindingManager)
  {
    mBindingManager = aBindingManager;
  }

  // Free global data at module shutdown
  static void FreeGlobals() { NS_IF_RELEASE(gQuirkURI); }

  // The "origins" of the CSS cascade, from lowest precedence to
  // highest (for non-!important rules).
  enum sheetType {
    eAgentSheet, // CSS
    ePresHintSheet,
    eUserSheet, // CSS
    eHTMLPresHintSheet,
    eDocSheet, // CSS
    eStyleAttrSheet,
    eOverrideSheet, // CSS
    eSheetTypeCount
    // be sure to keep the number of bits in |mDirty| below and in
    // NS_RULE_NODE_LEVEL_MASK updated when changing the number of sheet
    // types
  };

  // APIs to manipulate the style sheet lists.  The sheets in each
  // list are stored with the most significant sheet last.
  nsresult AppendStyleSheet(sheetType aType, nsIStyleSheet *aSheet);
  nsresult PrependStyleSheet(sheetType aType, nsIStyleSheet *aSheet);
  nsresult RemoveStyleSheet(sheetType aType, nsIStyleSheet *aSheet);
  nsresult ReplaceSheets(sheetType aType,
                         const nsCOMArray<nsIStyleSheet> &aNewSheets);

  //Enable/Disable entire author style level (Doc & PresHint levels)
  PRBool GetAuthorStyleDisabled();
  nsresult SetAuthorStyleDisabled(PRBool aStyleDisabled);

  PRInt32 SheetCount(sheetType aType) const {
    return mSheets[aType].Count();
  }

  nsIStyleSheet* StyleSheetAt(sheetType aType, PRInt32 aIndex) const {
    return mSheets[aType].ObjectAt(aIndex);
  }

  nsresult AddDocStyleSheet(nsIStyleSheet* aSheet, nsIDocument* aDocument);

  void     BeginUpdate();
  nsresult EndUpdate();

  // Methods for reconstructing the tree; BeginReconstruct basically moves the
  // old rule tree root and style context roots out of the way,
  // and EndReconstruct destroys the old rule tree when we're done
  nsresult BeginReconstruct();
  // Note: EndReconstruct should not be called if BeginReconstruct fails
  void EndReconstruct();

 private:
  // Not to be implemented
  nsStyleSet(const nsStyleSet& aCopy);
  nsStyleSet& operator=(const nsStyleSet& aCopy);

  // Returns false on out-of-memory.
  PRBool BuildDefaultStyleData(nsPresContext* aPresContext);

  // Update the rule processor list after a change to the style sheet list.
  nsresult GatherRuleProcessors(sheetType aType);

  void AddImportantRules(nsRuleNode* aCurrLevelNode,
                         nsRuleNode* aLastPrevLevelNode);

#ifdef DEBUG
  // Just like AddImportantRules except it doesn't actually add anything; it
  // just asserts that there are no important rules between aCurrLevelNode and
  // aLastPrevLevelNode.
  void AssertNoImportantRules(nsRuleNode* aCurrLevelNode,
                              nsRuleNode* aLastPrevLevelNode);
  
  // Just like AddImportantRules except it doesn't actually add anything; it
  // just asserts that there are no CSS rules between aCurrLevelNode and
  // aLastPrevLevelNode.  Mostly useful for the preshint levels.
  void AssertNoCSSRules(nsRuleNode* aCurrLevelNode,
                        nsRuleNode* aLastPrevLevelNode);
#endif
  
  // Enumerate the rules in a way that cares about the order of the
  // rules.
  void FileRules(nsIStyleRuleProcessor::EnumFunc aCollectorFunc,
                 RuleProcessorData* aData);

  // Enumerate all the rules in a way that doesn't care about the order
  // of the rules and break out if the enumeration is halted.
  void WalkRuleProcessors(nsIStyleRuleProcessor::EnumFunc aFunc,
                          RuleProcessorData* aData);

  already_AddRefed<nsStyleContext> GetContext(nsPresContext* aPresContext,
                                              nsStyleContext* aParentContext,
                                              nsIAtom* aPseudoTag);

  nsPresContext* PresContext() { return mRuleTree->GetPresContext(); }

  static nsIURI  *gQuirkURI;

  // The sheets in each array in mSheets are stored with the most significant
  // sheet last.
  nsCOMArray<nsIStyleSheet> mSheets[eSheetTypeCount];

  nsCOMPtr<nsIStyleRuleProcessor> mRuleProcessors[eSheetTypeCount];

  // cached instance for enabling/disabling
  nsCOMPtr<nsIStyleSheet> mQuirkStyleSheet;

  nsRefPtr<nsBindingManager> mBindingManager;

  // To be used only in case of emergency, such as being out of memory
  // or operating on a deleted rule node.  The latter should never
  // happen, of course.
  nsCachedStyleData mDefaultStyleData;

  nsRuleNode* mRuleTree; // This is the root of our rule tree.  It is a
                         // lexicographic tree of matched rules that style
                         // contexts use to look up properties.
  nsRuleWalker* mRuleWalker; // This is an instance of a rule walker that can
                             // be used to navigate through our tree.

  PRInt32 mDestroyedCount; // used to batch style context GC
  nsTArray<nsStyleContext*> mRoots; // style contexts with no parent

  PRUint16 mBatching;

  nsRuleNode* mOldRuleTree; // Old rule tree; used during tree reconstruction
                            // (See BeginReconstruct and EndReconstruct)

  unsigned mInShutdown : 1;
  unsigned mAuthorStyleDisabled: 1;
  unsigned mDirty : 7;  // one dirty bit is used per sheet type

};

#endif
