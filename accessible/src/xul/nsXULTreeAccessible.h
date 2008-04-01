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
 *   Author: Kyle Yuan (kyle.yuan@sun.com)
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
#ifndef __nsXULTreeAccessible_h__
#define __nsXULTreeAccessible_h__

#include "nsITreeBoxObject.h"
#include "nsITreeView.h"
#include "nsITreeColumns.h"
#include "nsXULSelectAccessible.h"
#include "nsIAccessibleTreeCache.h"

/*
 * A class the represents the XUL Tree widget.
 */
const PRUint32 kMaxTreeColumns = 100;
const PRUint32 kDefaultTreeCacheSize = 256;

class nsXULTreeAccessible : public nsXULSelectableAccessible,
                            public nsIAccessibleTreeCache
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIACCESSIBLESELECTABLE
  NS_DECL_NSIACCESSIBLETREECACHE

  nsXULTreeAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell);
  virtual ~nsXULTreeAccessible() {}

  /* ----- nsIAccessible ----- */
  NS_IMETHOD Shutdown();
  NS_IMETHOD GetRole(PRUint32 *_retval);
  NS_IMETHOD GetState(PRUint32 *aState, PRUint32 *aExtraState);
  NS_IMETHOD GetValue(nsAString& _retval);

  NS_IMETHOD GetFirstChild(nsIAccessible **_retval);
  NS_IMETHOD GetLastChild(nsIAccessible **_retval);
  NS_IMETHOD GetChildCount(PRInt32 *_retval);
  NS_IMETHOD GetFocusedChild(nsIAccessible **aFocusedChild);
  NS_IMETHOD GetChildAtPoint(PRInt32 aX, PRInt32 aY,
                             nsIAccessible **aAccessible);

  static void GetTreeBoxObject(nsIDOMNode* aDOMNode, nsITreeBoxObject** aBoxObject);
  static nsresult GetColumnCount(nsITreeBoxObject* aBoxObject, PRInt32 *aCount);

  static PRBool IsColumnHidden(nsITreeColumn *aColumn);
  static already_AddRefed<nsITreeColumn> GetNextVisibleColumn(nsITreeColumn *aColumn);
  static already_AddRefed<nsITreeColumn> GetFirstVisibleColumn(nsITreeBoxObject *aTree);
  static already_AddRefed<nsITreeColumn> GetLastVisibleColumn(nsITreeBoxObject *aTree);

protected:
  nsCOMPtr<nsITreeBoxObject> mTree;
  nsCOMPtr<nsITreeView> mTreeView;
  nsAccessNodeHashtable *mAccessNodeCache;

  NS_IMETHOD ChangeSelection(PRInt32 aIndex, PRUint8 aMethod, PRBool *aSelState);
};

/**
  * Treeitems -- used in Trees
  */
class nsXULTreeitemAccessible : public nsLeafAccessible,
                                public nsPIAccessibleTreeItem
{
public:
  enum { eAction_Click = 0, eAction_Expand = 1 };

  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSPIACCESSIBLETREEITEM

  nsXULTreeitemAccessible(nsIAccessible *aParent, nsIDOMNode *aDOMNode, nsIWeakReference *aShell, PRInt32 aRow, nsITreeColumn* aColumn = nsnull);
  virtual ~nsXULTreeitemAccessible() {}

  NS_IMETHOD Shutdown();

  // nsIAccessible
  NS_IMETHOD GetName(nsAString& _retval);
  NS_IMETHOD GetRole(PRUint32 *_retval);
  NS_IMETHOD GetState(PRUint32 *aState, PRUint32 *aExtraState);
  NS_IMETHOD GetNumActions(PRUint8 *_retval);
  NS_IMETHOD GetActionName(PRUint8 aIndex, nsAString& aName);
  virtual nsresult GetAttributesInternal(nsIPersistentProperties *aAttributes);

  NS_IMETHOD GetParent(nsIAccessible **_retval);
  NS_IMETHOD GetNextSibling(nsIAccessible **_retval);
  NS_IMETHOD GetPreviousSibling(nsIAccessible **_retval);

  NS_IMETHOD DoAction(PRUint8 index);
  NS_IMETHOD GetBounds(PRInt32 *x, PRInt32 *y, PRInt32 *width, PRInt32 *height);
  NS_IMETHOD SetSelected(PRBool aSelect); 
  NS_IMETHOD TakeFocus(void); 

  NS_IMETHOD GetAccessibleRelated(PRUint32 aRelationType, nsIAccessible **aRelated);

  // nsIAccessNode
  NS_IMETHOD GetUniqueID(void **aUniqueID);

  // nsPIAccessNode
  NS_IMETHOD Init();

  // nsAccessNode
  virtual PRBool IsDefunct();

protected:
  PRBool IsExpandable();
  nsCOMPtr<nsITreeBoxObject> mTree;
  nsCOMPtr<nsITreeView> mTreeView;
  PRInt32 mRow;
  nsCOMPtr<nsITreeColumn> mColumn;
  nsString mCachedName;
};

class nsXULTreeColumnsAccessible : public nsXULColumnsAccessible
{
public:
  nsXULTreeColumnsAccessible(nsIDOMNode* aDOMNode, nsIWeakReference* aShell);

  // nsIAccessible
  NS_IMETHOD GetNextSibling(nsIAccessible **aNextSibling);
};

#endif
