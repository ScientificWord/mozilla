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

#ifndef nsHTMLEditUtils_h__
#define nsHTMLEditUtils_h__

#include "prtypes.h"  // for PRBool
#include "nsError.h"  // for nsresult
class nsIEditor;
class nsIDOMNode;
class nsIDOMElement;
class nsString;
class msiITagListManager;

class nsHTMLEditUtils
{
public:
  // from nsTextEditRules:
  static PRBool IsBig(nsIDOMNode *aNode);
  static PRBool IsSmall(nsIDOMNode *aNode);

  // from nsHTMLEditRules:
  static PRBool IsInlineStyle(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsFormatNode(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsNodeThatCanOutdent(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsHeader(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsParagraph(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsHR(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsListItem(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsTable(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsTableRow(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsTableElement(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsTableElementButNotTable(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsTableCell(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsTableCellOrCaption(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsList(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsOrderedList(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsUnorderedList(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsBlockquote(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsPre(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsAddress(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsAnchor(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsImage(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsLink(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsNamedAnchor(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsDiv(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsMozDiv(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsMailCite(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsFormWidget(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool SupportsAlignAttr(nsIDOMNode *aNode, msiITagListManager * manager);
  //added for XML editing
  static PRBool IsNodeType(nsIDOMNode *aNode, nsString strClassName, msiITagListManager * manager);
  static PRBool IsTextNode(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsParaNode(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsListNode(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsEnvNode(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsStructNode(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsFrontMNode(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsOtherNode(nsIDOMNode *aNode, msiITagListManager * manager);
  static PRBool IsMath(nsIDOMNode *aNode);
  static PRBool IsMathNode(nsIDOMNode *aNode);
  static PRBool CanContain(PRInt32 aParent, PRInt32 aChild, msiITagListManager * manager);
  static PRBool IsContainer(PRInt32 aTag, msiITagListManager * manager);
  static void MatchingFence(nsIDOMElement *aNode, nsIDOMElement ** other);
};

#endif /* nsHTMLEditUtils_h__ */

