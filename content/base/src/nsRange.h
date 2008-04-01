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
 * Implementation of the DOM nsIDOMRange object.
 */

#ifndef nsRange_h___
#define nsRange_h___

#include "nsIRange.h"
#include "nsIDOMRange.h"
#include "nsIRangeUtils.h"
#include "nsIDOMNSRange.h"
#include "nsCOMPtr.h"
#include "nsIDOMDocumentFragment.h"
#include "nsIContent.h"
#include "nsIDOMNode.h"
#include "prmon.h"
#include "nsStubMutationObserver.h"

class nsVoidArray;

// -------------------------------------------------------------------------------

class nsRangeUtils : public nsIRangeUtils
{
public:
  NS_DECL_ISUPPORTS

  // nsIRangeUtils interface
  NS_IMETHOD_(PRInt32) ComparePoints(nsIDOMNode* aParent1, PRInt32 aOffset1,
                                     nsIDOMNode* aParent2, PRInt32 aOffset2);
                               
  NS_IMETHOD CompareNodeToRange(nsIContent* aNode, 
                                nsIDOMRange* aRange,
                                PRBool *outNodeBefore,
                                PRBool *outNodeAfter);
};

// -------------------------------------------------------------------------------

class nsRange : public nsIRange,
                public nsIDOMRange,
                public nsIDOMNSRange,
                public nsStubMutationObserver
{
public:
  nsRange()
  {
  }
  virtual ~nsRange();

  NS_DECL_ISUPPORTS

  // nsIDOMRange interface
  NS_DECL_NSIDOMRANGE

  // nsIDOMNSRange interface
  NS_DECL_NSIDOMNSRANGE
  
  // nsIRange interface
  virtual nsINode* GetCommonAncestor();
  virtual void Reset();
  
  // nsIMutationObserver methods
  virtual void CharacterDataChanged(nsIDocument* aDocument,
                                    nsIContent* aContent,
                                    CharacterDataChangeInfo* aChangeInfo);
  virtual void ContentInserted(nsIDocument* aDocument,
                               nsIContent* aContainer,
                               nsIContent* aChild,
                               PRInt32 aIndexInContainer);
  virtual void ContentRemoved(nsIDocument* aDocument,
                              nsIContent* aContainer,
                              nsIContent* aChild,
                              PRInt32 aIndexInContainer);
  virtual void NodeWillBeDestroyed(const nsINode* aNode);
  virtual void ParentChainChanged(nsIContent *aContent);

private:
  // no copy's or assigns
  nsRange(const nsRange&);
  nsRange& operator=(const nsRange&);

  nsINode* IsValidBoundary(nsINode* aNode);
 
public:
/******************************************************************************
 *  Utility routine to detect if a content node starts before a range and/or 
 *  ends after a range.  If neither it is contained inside the range.
 *  
 *  XXX - callers responsibility to ensure node in same doc as range!
 *
 *****************************************************************************/
  static nsresult CompareNodeToRange(nsIContent* aNode, nsIDOMRange* aRange,
                                     PRBool *outNodeBefore,
                                     PRBool *outNodeAfter);
  static nsresult CompareNodeToRange(nsIContent* aNode, nsIRange* aRange,
                                     PRBool *outNodeBefore,
                                     PRBool *outNodeAfter);

protected:
  void DoSetRange(nsINode* aStartN, PRInt32 aStartOffset,
                  nsINode* aEndN, PRInt32 aEndOffset,
                  nsINode* aRoot);
};

// Make a new nsIDOMRange object
nsresult NS_NewRange(nsIDOMRange** aInstancePtrResult);

// Make a new nsIRangeUtils object
nsresult NS_NewRangeUtils(nsIRangeUtils** aInstancePtrResult);

#endif /* nsRange_h___ */
