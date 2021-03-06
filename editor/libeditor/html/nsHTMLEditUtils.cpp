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
 * and other provisions required by the GPL or the LGPL. If you do not
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "nsHTMLEditUtils.h"
#include "nsTextEditUtils.h"

#include "nsString.h"
#include "nsUnicharUtils.h"
#include "nsEditor.h"
#include "nsEditProperty.h"
#include "nsIAtom.h"
#include "nsGkAtoms.h"
#include "nsIDOMNode.h"
#include "nsIContent.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMHTMLAnchorElement.h"
#include "nsHTMLTags.h"
#include "nsIDOMElement.h"
#include "nsIDOMDocumentTraversal.h"
#include "nsIDOMNodeFilter.h"
#include "nsIDOMTreeWalker.h"
#include "nsIEnumerator.h"
#include "msiITagListManager.h"
#include "msiUtils.h"


nsString strMathMLNs = NS_LITERAL_STRING("http://www.w3.org/1998/Math/MathML");

///////////////////////////////////////////////////////////////////////////
//
PRBool
nsHTMLEditUtils::IsBig(nsIDOMNode *node)
{
  return nsEditor::NodeIsType(node, nsEditProperty::big);
}


///////////////////////////////////////////////////////////////////////////
// IsInlineStyle true if node is an inline style
//
PRBool
nsHTMLEditUtils::IsInlineStyle(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsInlineStyle");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsInlineStyle");
  nsAutoString tagName;
  nodeAtom->ToString(tagName);
  PRBool isTextTag;
  manager->GetTagInClass (NS_LITERAL_STRING("texttag"), tagName, nsnull, &isTextTag);
  if (isTextTag) return PR_TRUE;
  return (nodeAtom == nsEditProperty::b)
      || (nodeAtom == nsEditProperty::i)
      || (nodeAtom == nsEditProperty::u)
      || (nodeAtom == nsEditProperty::tt)
      || (nodeAtom == nsEditProperty::s)
      || (nodeAtom == nsEditProperty::strike)
      || (nodeAtom == nsEditProperty::big)
      || (nodeAtom == nsEditProperty::small)
      || (nodeAtom == nsEditProperty::blink)
      || (nodeAtom == nsEditProperty::sub)
      || (nodeAtom == nsEditProperty::sup)
      || (nodeAtom == nsEditProperty::font);
}

///////////////////////////////////////////////////////////////////////////
// IsFormatNode true if node is a format node  
// we count para nodes as format nodes
//
PRBool
nsHTMLEditUtils::IsFormatNode(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsFormatNode");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsFormatNode");
  nsAutoString tagName;
  nodeAtom->ToString(tagName);
  PRBool isParaTag;
  manager->GetTagInClass (NS_LITERAL_STRING("paratag"), tagName, nsnull, &isParaTag);
  if (isParaTag) return PR_TRUE;
  PRBool isFrontMTag;
  manager->GetTagInClass (NS_LITERAL_STRING("frontmtag"), tagName, nsnull, &isFrontMTag);
  if (isFrontMTag) return PR_TRUE;
  return (nodeAtom == nsEditProperty::p)
      || (nodeAtom == nsEditProperty::pre)
      || (nodeAtom == nsEditProperty::h1)
      || (nodeAtom == nsEditProperty::h2)
      || (nodeAtom == nsEditProperty::h3)
      || (nodeAtom == nsEditProperty::h4)
      || (nodeAtom == nsEditProperty::h5)
      || (nodeAtom == nsEditProperty::h6)
      || (nodeAtom == nsEditProperty::address);
}

///////////////////////////////////////////////////////////////////////////
// IsNodeThatCanOutdent true if node is a list, list item, or blockquote
//
PRBool
nsHTMLEditUtils::IsNodeThatCanOutdent(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsNodeThatCanOutdent");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);

  if (!nodeAtom)
    return PR_FALSE;

  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsNodeThatCanOutdent");
  nsAutoString tagName;
  nodeAtom->ToString(tagName);
  PRBool isListTag;
  manager->GetTagInClass (NS_LITERAL_STRING("listtag"), tagName, nsnull, &isListTag);
  if (isListTag) return PR_TRUE;
  return (nodeAtom == nsEditProperty::ul)
      || (nodeAtom == nsEditProperty::ol)
      || (nodeAtom == nsEditProperty::dl)
      || (nodeAtom == nsEditProperty::li)
      || (nodeAtom == nsEditProperty::dd)
      || (nodeAtom == nsEditProperty::dt)
      || (nodeAtom == nsEditProperty::blockquote);
}

///////////////////////////////////////////////////////////////////////////
//
PRBool
nsHTMLEditUtils::IsSmall(nsIDOMNode *node)
{
  return nsEditor::NodeIsType(node, nsEditProperty::small);
}


/********************************************************
 *  helper methods from nsHTMLEditRules
 ********************************************************/

///////////////////////////////////////////////////////////////////////////
// IsHeader: true if node an html header
//
PRBool
nsHTMLEditUtils::IsHeader(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsHeader");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsHeader");
  // we could look for a paragraph tag that is the first child of a section tag -- TODO if needed
  return (nodeAtom == nsEditProperty::h1)
      || (nodeAtom == nsEditProperty::h2)
      || (nodeAtom == nsEditProperty::h3)
      || (nodeAtom == nsEditProperty::h4)
      || (nodeAtom == nsEditProperty::h5)
      || (nodeAtom == nsEditProperty::h6);
}


///////////////////////////////////////////////////////////////////////////
// IsParagraph: true if node an html paragraph
//
PRBool
nsHTMLEditUtils::IsParagraph(nsIDOMNode *node, msiITagListManager * manager)
{
  return nsEditor::NodeIsTypeString(node, NS_LITERAL_STRING("paratag"), manager);
}


///////////////////////////////////////////////////////////////////////////
// IsHR: true if node an horizontal rule
//
PRBool
nsHTMLEditUtils::IsHR(nsIDOMNode *node, msiITagListManager * manager)
{
// How to handle this?  TODO
  return nsEditor::NodeIsType(node, nsEditProperty::hr);
}


///////////////////////////////////////////////////////////////////////////
// IsListItem: true if node an html list item
//
PRBool
nsHTMLEditUtils::IsListItem(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsListItem");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils:IsListItem:");
  PRBool isListTag;
  nsAutoString tagName;
  nodeAtom->ToString(tagName);
  manager->GetTagInClass (NS_LITERAL_STRING("listtag"), tagName, nsnull, &isListTag);
  if (isListTag) // list tag can be a list or a listitem. Check that we have a listitem.
  {
    nsAutoString htmlparent;
    manager->GetStringPropertyForTag(tagName, nsnull, NS_LITERAL_STRING("htmllistparent"), htmlparent);
    if (htmlparent.Length() > 0) return PR_TRUE;
  }
  return  ((nodeAtom == nsEditProperty::li)
      || (nodeAtom == nsEditProperty::dd)
      || (nodeAtom == nsEditProperty::dt));
}


///////////////////////////////////////////////////////////////////////////
// IsTableElement: true if node an html table, td, tr, ...
//
PRBool
nsHTMLEditUtils::IsTableElement(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null node passed to nsHTMLEditor::IsTableElement");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsTableElement");
  return (nodeAtom == nsEditProperty::table)
    || (nodeAtom == nsEditProperty::tr)
    || (nodeAtom == nsEditProperty::td)
    || (nodeAtom == nsEditProperty::th)
    || (nodeAtom == nsGkAtoms::mtr_)
    || (nodeAtom == nsGkAtoms::mtable_)
    || (nodeAtom == nsGkAtoms::mtd_)
    || (nodeAtom == nsEditProperty::thead)
    || (nodeAtom == nsEditProperty::tfoot)
    || (nodeAtom == nsEditProperty::tbody)
    || (nodeAtom == nsEditProperty::caption)
    || (nodeAtom == nsEditProperty::caption2);
// BBM: do we want to add the mtable elements?
}

///////////////////////////////////////////////////////////////////////////
// IsTableElementButNotTable: true if node an html td, tr, ... (doesn't include table)
//
PRBool
nsHTMLEditUtils::IsTableElementButNotTable(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null node passed to nsHTMLEditor::IsTableElementButNotTable");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsTableElementButNotTable");
  return (nodeAtom == nsEditProperty::tr)
      || (nodeAtom == nsEditProperty::td)
      || (nodeAtom == nsEditProperty::th)
	    || (nodeAtom == nsGkAtoms::mtr_)
	    || (nodeAtom == nsGkAtoms::mtd_)
      || (nodeAtom == nsEditProperty::thead)
      || (nodeAtom == nsEditProperty::tfoot)
      || (nodeAtom == nsEditProperty::tbody)
      || (nodeAtom == nsEditProperty::caption)
      || (nodeAtom == nsEditProperty::caption2);
}

///////////////////////////////////////////////////////////////////////////
// IsTable: true if node an html table
//
PRBool
nsHTMLEditUtils::IsTable(nsIDOMNode *node, msiITagListManager * manager)
{
	PRBool retVal;
	retVal =  nsEditor::NodeIsType(node, nsEditProperty::table, manager);
	if (!retVal)
	{
		retVal =  nsEditor::NodeIsType(node, nsGkAtoms::mtable_, manager);
	}
	return retVal;
}

///////////////////////////////////////////////////////////////////////////
// IsTableRow: true if node an html tr
//
PRBool
nsHTMLEditUtils::IsTableRow(nsIDOMNode *node, msiITagListManager * manager)
{
	PRBool retVal;
	retVal = nsEditor::NodeIsType(node, nsEditProperty::tr, manager);
	if (!retVal)
	{
		retVal = nsEditor::NodeIsType(node, nsGkAtoms::mtr_, manager);
	}
	return retVal;
}


///////////////////////////////////////////////////////////////////////////
// IsTableCell: true if node an html td or th
//
PRBool
nsHTMLEditUtils::IsTableCell(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsTableCell");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsTableCell");
  return (nodeAtom == nsEditProperty::td) || (nodeAtom == nsGkAtoms::mtd_)
      || (nodeAtom == nsEditProperty::th);
}


///////////////////////////////////////////////////////////////////////////
// IsTableCell: true if node an html td or th
//
PRBool
nsHTMLEditUtils::IsTableCellOrCaption(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsTableCell");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsTableCell");
  return (nodeAtom == nsEditProperty::td) || (nodeAtom == nsGkAtoms::mtd_)
      || (nodeAtom == nsEditProperty::th)
      || (nodeAtom == nsEditProperty::caption);
}


///////////////////////////////////////////////////////////////////////////
// IsList: true if node an html list
//
PRBool
nsHTMLEditUtils::IsList(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsList");
  if (!node) return PR_FALSE;
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsList");
  if (!nodeAtom) return PR_FALSE;
  PRBool isListTag;
  nsAutoString tagName;
  nodeAtom->ToString(tagName);
  manager->GetTagInClass (NS_LITERAL_STRING("listtag"), tagName, nsnull, &isListTag);
  if (isListTag) // list tag can be a list or a listitem. Check that we have a list.
  {
    nsAutoString htmllist;
    manager->GetStringPropertyForTag(tagName, nsnull, NS_LITERAL_STRING("htmllist"), htmllist);
    if (htmllist.Length() > 0) return PR_TRUE;
  }
  return (nodeAtom == nsEditProperty::ul)
      || (nodeAtom == nsEditProperty::ol)
      || (nodeAtom == nsEditProperty::dl);
}


///////////////////////////////////////////////////////////////////////////
// IsOrderedList: true if node an html ordered list
//
PRBool
nsHTMLEditUtils::IsOrderedList(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsOrderedList");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsOrderedList");
  PRBool isListTag;
  nsAutoString tagName;
  nodeAtom->ToString(tagName);
  manager->GetTagInClass (NS_LITERAL_STRING("listtag"), tagName, nsnull, &isListTag);
  if (isListTag) // list tag can be a list or a listitem. Check that we have a list.
  {
    nsAutoString htmllist;
    manager->GetStringPropertyForTag(tagName, nsnull, NS_LITERAL_STRING("htmllist"), htmllist);
    if (htmllist.EqualsLiteral("ol")) return PR_TRUE;
  }
  return nsEditor::NodeIsType(node, nsEditProperty::ol);
}


///////////////////////////////////////////////////////////////////////////
// IsUnorderedList: true if node an html unordered list
//
PRBool
nsHTMLEditUtils::IsUnorderedList(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsOrderedList");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsOrderedList");
  PRBool isListTag;
  nsAutoString tagName;
  nodeAtom->ToString(tagName);
  manager->GetTagInClass (NS_LITERAL_STRING("listtag"), tagName, nsnull, &isListTag);
  if (isListTag) // list tag can be a list or a listitem. Check that we have a list.
  {
    nsAutoString htmllist;
    manager->GetStringPropertyForTag(tagName, nsnull, NS_LITERAL_STRING("htmllist"), htmllist);
    if (htmllist.EqualsLiteral("ul")) return PR_TRUE;
  }
  return nsEditor::NodeIsType(node, nsEditProperty::ul);
}


///////////////////////////////////////////////////////////////////////////
// IsBlockquote: true if node an html blockquote node
//
PRBool
nsHTMLEditUtils::IsBlockquote(nsIDOMNode *node, msiITagListManager * manager)
{
  return nsEditor::NodeIsType(node, nsEditProperty::blockquote);
}


///////////////////////////////////////////////////////////////////////////
// IsPre: true if node an html pre node
//
PRBool
nsHTMLEditUtils::IsPre(nsIDOMNode *node, msiITagListManager * manager)
{
  nsAutoString tagName;
  nsCOMPtr<nsIDOMElement> element = do_QueryInterface(node);
  if (element)
    element->GetTagName(tagName);
  return (nsEditor::NodeIsType(node, nsEditProperty::pre) || tagName.EqualsLiteral("verbatim"));
}


///////////////////////////////////////////////////////////////////////////
// IsAddress: true if node an html address node
//
PRBool
nsHTMLEditUtils::IsAddress(nsIDOMNode *node, msiITagListManager * manager)
{
  return nsEditor::NodeIsType(node, nsEditProperty::address);
}


///////////////////////////////////////////////////////////////////////////
// IsImage: true if node an html image node
//
PRBool
nsHTMLEditUtils::IsImage(nsIDOMNode *node, msiITagListManager * manager)
{
  return nsEditor::NodeIsType(node, nsEditProperty::img);
}

PRBool
nsHTMLEditUtils::IsLink(nsIDOMNode *aNode, msiITagListManager * manager)
{
  if (!aNode) return PR_FALSE;
  nsCOMPtr<nsIDOMHTMLAnchorElement> anchor = do_QueryInterface(aNode);
  if (anchor)
  {
    nsAutoString tmpText;
    if (NS_SUCCEEDED(anchor->GetHref(tmpText)) && !tmpText.IsEmpty())
      return PR_TRUE;
  }
  return PR_FALSE;
}

PRBool
nsHTMLEditUtils::IsNamedAnchor(nsIDOMNode *aNode, msiITagListManager * manager)
{
  if (!aNode) return PR_FALSE;
  nsCOMPtr<nsIDOMHTMLAnchorElement> anchor = do_QueryInterface(aNode);
  if (anchor)
  {
    nsAutoString tmpText;
    if (NS_SUCCEEDED(anchor->GetName(tmpText)) && !tmpText.IsEmpty())
      return PR_TRUE;
  }
  return PR_FALSE;
}


///////////////////////////////////////////////////////////////////////////
// IsDiv: true if node an html div node
//
PRBool
nsHTMLEditUtils::IsDiv(nsIDOMNode *node, msiITagListManager * manager)
{
  return nsEditor::NodeIsType(node, nsEditProperty::div);
}


///////////////////////////////////////////////////////////////////////////
// IsMozDiv: true if node an html div node with type = _moz
//
PRBool
nsHTMLEditUtils::IsMozDiv(nsIDOMNode *node, msiITagListManager * manager)
{
  if (IsDiv(node, manager) && nsTextEditUtils::HasMozAttr(node)) return PR_TRUE;
  return PR_FALSE;
}



///////////////////////////////////////////////////////////////////////////
// IsMailCite: true if node an html blockquote with type=cite
//
PRBool
nsHTMLEditUtils::IsMailCite(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null parent passed to nsHTMLEditUtils::IsMailCite");
  nsCOMPtr<nsIDOMElement> elem = do_QueryInterface(node);
  if (!elem) return PR_FALSE;
  nsAutoString attrName (NS_LITERAL_STRING("type"));

  // don't ask me why, but our html mailcites are id'd by "type=cite"...
  nsAutoString attrVal;
  nsresult res = elem->GetAttribute(attrName, attrVal);
//  ToLowerCase(attrVal);
  if (NS_SUCCEEDED(res))
  {
    if (attrVal.EqualsLiteral("cite"))
      return PR_TRUE;
  }

  // ... but our plaintext mailcites by "_moz_quote=true".  go figure.
  attrName.AssignLiteral("_moz_quote");
  res = elem->GetAttribute(attrName, attrVal);
  if (NS_SUCCEEDED(res))
  {
//    ToLowerCase(attrVal);
    if (attrVal.EqualsLiteral("true"))
      return PR_TRUE;
  }

  return PR_FALSE;
}


///////////////////////////////////////////////////////////////////////////
// IsFormWidget: true if node is a form widget of some kind
//
PRBool
nsHTMLEditUtils::IsFormWidget(nsIDOMNode *node, msiITagListManager * manager)
{
  NS_PRECONDITION(node, "null node passed to nsHTMLEditUtils::IsFormWidget");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(node);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::IsFormWidget");
  return (nodeAtom == nsEditProperty::textarea)
      || (nodeAtom == nsEditProperty::select)
      || (nodeAtom == nsEditProperty::button)
      || (nodeAtom == nsEditProperty::input);
}

PRBool
nsHTMLEditUtils::SupportsAlignAttr(nsIDOMNode * aNode, msiITagListManager * manager)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::SupportsAlignAttr");
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(aNode);
  NS_PRECONDITION(nodeAtom, "node with null tag passed to nsHTMLEditUtils::SupportsAlignAttr");
  nsAutoString tagName;
  nodeAtom->ToString(tagName);
  nsAutoString tagClass;
  manager->GetRealClassOfTag (tagName, nsnull, tagClass);
  if (tagClass.EqualsLiteral("frontmtag")||tagClass.EqualsLiteral("structtag")||tagClass.EqualsLiteral("paratag")||tagClass.EqualsLiteral("structtag")||tagClass.EqualsLiteral("listtag")||tagClass.EqualsLiteral("envtag"))
    return PR_TRUE;
  return (nodeAtom == nsEditProperty::hr)
      || (nodeAtom == nsEditProperty::table)
      || (nodeAtom == nsEditProperty::tbody)
      || (nodeAtom == nsEditProperty::tfoot)
      || (nodeAtom == nsEditProperty::thead)
      || (nodeAtom == nsEditProperty::tr)
      || (nodeAtom == nsEditProperty::td)
      || (nodeAtom == nsEditProperty::th)
      || (nodeAtom == nsEditProperty::div)
      || (nodeAtom == nsEditProperty::p)
      || (nodeAtom == nsEditProperty::h1)
      || (nodeAtom == nsEditProperty::h2)
      || (nodeAtom == nsEditProperty::h3)
      || (nodeAtom == nsEditProperty::h4)
      || (nodeAtom == nsEditProperty::h5)
      || (nodeAtom == nsEditProperty::h6);
}
PRBool
nsHTMLEditUtils::IsNodeType(nsIDOMNode *aNode, nsString strClassName, msiITagListManager * manager)
{
  PRBool found;
  nsCOMPtr<nsIAtom> nodeAtom = nsEditor::GetTag(aNode);
  if (!nodeAtom) return NS_ERROR_FAILURE;
  nsString strTagName;
  nodeAtom->ToString(strTagName);
  // to do: put in namespace atom
  manager->GetTagInClass(strClassName, strTagName, nsnull , &found);
  return found;
}


  //added for XML editing
PRBool
nsHTMLEditUtils::IsTextNode(nsIDOMNode *aNode, msiITagListManager * manager)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsTextNode");
  return IsNodeType(aNode, NS_LITERAL_STRING("texttag"), manager);
}


PRBool
nsHTMLEditUtils::IsParaNode(nsIDOMNode *aNode, msiITagListManager * manager)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsParaNode");
  return IsNodeType(aNode, NS_LITERAL_STRING("paratag"), manager);
}

PRBool
nsHTMLEditUtils::IsStructNode(nsIDOMNode *aNode, msiITagListManager * manager)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsStructNode");
  return IsNodeType(aNode, NS_LITERAL_STRING("structtag"), manager);
}

PRBool
nsHTMLEditUtils::IsListNode(nsIDOMNode *aNode, msiITagListManager * manager)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsListNode");
  return IsNodeType(aNode, NS_LITERAL_STRING("listtag"), manager);
}

PRBool
nsHTMLEditUtils::IsEnvNode(nsIDOMNode *aNode, msiITagListManager * manager)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsEnvNode");
  return IsNodeType(aNode, NS_LITERAL_STRING("envtag"), manager);
}


PRBool
nsHTMLEditUtils::IsFrontMNode(nsIDOMNode *aNode, msiITagListManager * manager)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsFrontMNode");
  return IsNodeType(aNode, NS_LITERAL_STRING("frontmtag"), manager);
}


PRBool
nsHTMLEditUtils::IsOtherNode(nsIDOMNode *aNode, msiITagListManager * manager)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsOtherNode");
  return IsNodeType(aNode, NS_LITERAL_STRING("othertag"), manager);
}

PRBool nsHTMLEditUtils::IsMath(nsIDOMNode *aNode)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsMath");
  nsAutoString tagNamespace;
  PRUint16 nodeType;
  aNode->GetNodeType(&nodeType);
  nsCOMPtr<nsIDOMNode> theNode;
  if (nodeType == nsIDOMNode::TEXT_NODE)
    aNode->GetParentNode(getter_AddRefs(theNode));
  else
    theNode = aNode;
  if (!theNode) return PR_FALSE;
  theNode->GetNamespaceURI(tagNamespace);
  return tagNamespace.Equals(strMathMLNs);

}

PRBool nsHTMLEditUtils::IsMathNode(nsIDOMNode *aNode)
{
  NS_PRECONDITION(aNode, "null node passed to nsHTMLEditUtils::IsMathNode");
  nsAutoString tagNamespace;
  nsAutoString tagName;
  nsCOMPtr<nsIDOMElement> element;
  element = do_QueryInterface(aNode);
  if (!element) return PR_FALSE;
  element->GetTagName(tagName);
  element->GetNamespaceURI(tagNamespace);
  return (tagName.EqualsLiteral("math") && tagNamespace.Equals(strMathMLNs));

}

// We use bitmasks to test containment of elements. Elements are marked to be
// in certain groups by setting the mGroup member of the nsElementInfo struct
// to the corresponding GROUP_ values (OR'ed together). Similarly, elements are
// marked to allow containment of certain groups by setting the
// mCanContainGroups member of the nsElementInfo struct to the corresponding
// GROUP_ values (OR'ed together).
// Testing containment then simply consists of checking whether the
// mCanContainGroups bitmask of an element and the mGroup bitmask of a
// potential child overlap.

#define GROUP_NONE             0

// body, head, html
#define GROUP_TOPLEVEL         (1 << 1)

// base, isindex, link, meta, script, style, title
#define GROUP_HEAD_CONTENT     (1 << 2)

// b, big, i, s, small, strike, tt, u
#define GROUP_FONTSTYLE        (1 << 3)

// abbr, acronym, cite, code, del, dfn, em, ins, kbd, samp, strong, var
#define GROUP_PHRASE           (1 << 4)

// a, applet, basefont, bdo, br, font, iframe, img, map, object, q, script,
// span, sub, sup
#define GROUP_SPECIAL          (1 << 5)

// button, form, input, label, select, textarea
#define GROUP_FORMCONTROL      (1 << 6)

// address, applet, blockquote, button, center, del, dir, div, dl, fieldset,
// form, h1, h2, h3, h4, h5, h6, hr, iframe, ins, isindex, map, menu, noframes,
// noscript, object, ol, p, pre, table, ul
#define GROUP_BLOCK            (1 << 7)

// frame, frameset
#define GROUP_FRAME            (1 << 8)

// col, tbody
#define GROUP_TABLE_CONTENT    (1 << 9)

// tr
#define GROUP_TBODY_CONTENT    (1 << 10)

// td, th
#define GROUP_TR_CONTENT       (1 << 11)

// col
#define GROUP_COLGROUP_CONTENT (1 << 12)

// param
#define GROUP_OBJECT_CONTENT   (1 << 13)

// li
#define GROUP_LI               (1 << 14)

// area
#define GROUP_MAP_CONTENT      (1 << 15)

// optgroup, option
#define GROUP_SELECT_CONTENT   (1 << 16)

// option
#define GROUP_OPTGROUP_CONTENT (1 << 17)

// dd, dt
#define GROUP_DL_CONTENT       (1 << 18)

// p
#define GROUP_P                (1 << 19)

// text, whitespace, newline, comment
#define GROUP_LEAF             (1 << 20)

// XXX This is because the editor does sublists illegally.
// ol, ul
#define GROUP_OL_UL            (1 << 21)

#define GROUP_INLINE_ELEMENT \
  (GROUP_FONTSTYLE | GROUP_PHRASE | GROUP_SPECIAL | GROUP_FORMCONTROL | \
   GROUP_LEAF)

#define GROUP_FLOW_ELEMENT (GROUP_INLINE_ELEMENT | GROUP_BLOCK)

struct nsElementInfo
{
#ifdef DEBUG
  eHTMLTags mTag;
#endif
  PRUint32 mGroup;
  PRUint32 mCanContainGroups;
  PRPackedBool mIsContainer;
  PRPackedBool mCanContainSelf;
};

#ifdef DEBUG
#define ELEM(_tag, _isContainer, _canContainSelf, _group, _canContainGroups) \
  { eHTMLTag_##_tag, _group, _canContainGroups, _isContainer, _canContainSelf }
#else
#define ELEM(_tag, _isContainer, _canContainSelf, _group, _canContainGroups) \
  { _group, _canContainGroups, _isContainer, _canContainSelf }
#endif

static const nsElementInfo kElements[eHTMLTag_userdefined] = {
  ELEM(a, PR_TRUE, PR_FALSE, GROUP_SPECIAL, GROUP_INLINE_ELEMENT),
  ELEM(abbr, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(acronym, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(address, PR_TRUE, PR_TRUE, GROUP_BLOCK,
       GROUP_INLINE_ELEMENT | GROUP_P),
  ELEM(applet, PR_TRUE, PR_TRUE, GROUP_SPECIAL | GROUP_BLOCK,
       GROUP_FLOW_ELEMENT | GROUP_OBJECT_CONTENT),
  ELEM(area, PR_FALSE, PR_FALSE, GROUP_MAP_CONTENT, GROUP_NONE),
  ELEM(b, PR_TRUE, PR_TRUE, GROUP_FONTSTYLE, GROUP_INLINE_ELEMENT),
  ELEM(base, PR_FALSE, PR_FALSE, GROUP_HEAD_CONTENT, GROUP_NONE),
  ELEM(basefont, PR_FALSE, PR_FALSE, GROUP_SPECIAL, GROUP_NONE),
  ELEM(bdo, PR_TRUE, PR_TRUE, GROUP_SPECIAL, GROUP_INLINE_ELEMENT),
  ELEM(bgsound, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(big, PR_TRUE, PR_TRUE, GROUP_FONTSTYLE, GROUP_INLINE_ELEMENT),
  ELEM(blink, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(blockquote, PR_TRUE, PR_TRUE, GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(body, PR_TRUE, PR_TRUE, GROUP_TOPLEVEL, GROUP_FLOW_ELEMENT),
  ELEM(br, PR_FALSE, PR_FALSE, GROUP_SPECIAL, GROUP_NONE),
  ELEM(button, PR_TRUE, PR_TRUE, GROUP_FORMCONTROL | GROUP_BLOCK,
       GROUP_FLOW_ELEMENT),
  ELEM(canvas, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(caption, PR_TRUE, PR_TRUE, GROUP_NONE, GROUP_INLINE_ELEMENT),
  ELEM(center, PR_TRUE, PR_TRUE, GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(cite, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(code, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(col, PR_FALSE, PR_FALSE, GROUP_TABLE_CONTENT | GROUP_COLGROUP_CONTENT,
       GROUP_NONE),
  ELEM(colgroup, PR_TRUE, PR_FALSE, GROUP_NONE, GROUP_COLGROUP_CONTENT),
  ELEM(dd, PR_TRUE, PR_FALSE, GROUP_DL_CONTENT, GROUP_FLOW_ELEMENT),
  ELEM(del, PR_TRUE, PR_TRUE, GROUP_PHRASE | GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(dfn, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(dir, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_LI),
  ELEM(div, PR_TRUE, PR_TRUE, GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(dl, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_DL_CONTENT),
  ELEM(dt, PR_TRUE, PR_TRUE, GROUP_DL_CONTENT, GROUP_INLINE_ELEMENT),
  ELEM(em, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(embed, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(fieldset, PR_TRUE, PR_TRUE, GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(font, PR_TRUE, PR_TRUE, GROUP_SPECIAL, GROUP_INLINE_ELEMENT),
  ELEM(form, PR_TRUE, PR_TRUE, GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(frame, PR_FALSE, PR_FALSE, GROUP_FRAME, GROUP_NONE),
  ELEM(frameset, PR_TRUE, PR_TRUE, GROUP_FRAME, GROUP_FRAME),
  ELEM(h1, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_INLINE_ELEMENT),
  ELEM(h2, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_INLINE_ELEMENT),
  ELEM(h3, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_INLINE_ELEMENT),
  ELEM(h4, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_INLINE_ELEMENT),
  ELEM(h5, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_INLINE_ELEMENT),
  ELEM(h6, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_INLINE_ELEMENT),
  ELEM(head, PR_TRUE, PR_FALSE, GROUP_TOPLEVEL, GROUP_HEAD_CONTENT),
  ELEM(hr, PR_FALSE, PR_FALSE, GROUP_BLOCK, GROUP_NONE),
  ELEM(html, PR_TRUE, PR_FALSE, GROUP_TOPLEVEL, GROUP_TOPLEVEL),
  ELEM(i, PR_TRUE, PR_TRUE, GROUP_FONTSTYLE, GROUP_INLINE_ELEMENT),
  ELEM(iframe, PR_TRUE, PR_TRUE, GROUP_SPECIAL | GROUP_BLOCK,
       GROUP_FLOW_ELEMENT),
  ELEM(image, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(img, PR_FALSE, PR_FALSE, GROUP_SPECIAL, GROUP_NONE),
  ELEM(input, PR_FALSE, PR_FALSE, GROUP_FORMCONTROL, GROUP_NONE),
  ELEM(ins, PR_TRUE, PR_TRUE, GROUP_PHRASE | GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(isindex, PR_FALSE, PR_FALSE, GROUP_BLOCK | GROUP_HEAD_CONTENT,
       GROUP_NONE),
  ELEM(kbd, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(keygen, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(label, PR_TRUE, PR_FALSE, GROUP_FORMCONTROL, GROUP_INLINE_ELEMENT),
  ELEM(legend, PR_TRUE, PR_TRUE, GROUP_NONE, GROUP_INLINE_ELEMENT),
  ELEM(li, PR_TRUE, PR_FALSE, GROUP_LI, GROUP_FLOW_ELEMENT),
  ELEM(link, PR_FALSE, PR_FALSE, GROUP_HEAD_CONTENT, GROUP_NONE),
  ELEM(listing, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(map, PR_TRUE, PR_TRUE, GROUP_SPECIAL, GROUP_BLOCK | GROUP_MAP_CONTENT),
  ELEM(marquee, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(menu, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_LI),
  ELEM(meta, PR_FALSE, PR_FALSE, GROUP_HEAD_CONTENT, GROUP_NONE),
  ELEM(multicol, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(nobr, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(noembed, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(noframes, PR_TRUE, PR_TRUE, GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(noscript, PR_TRUE, PR_TRUE, GROUP_BLOCK, GROUP_FLOW_ELEMENT),
  ELEM(object, PR_TRUE, PR_TRUE, GROUP_SPECIAL | GROUP_BLOCK,
       GROUP_FLOW_ELEMENT | GROUP_OBJECT_CONTENT),
  // XXX Can contain self and ul because editor does sublists illegally.
  ELEM(ol, PR_TRUE, PR_TRUE, GROUP_BLOCK | GROUP_OL_UL,
       GROUP_LI | GROUP_OL_UL),
  ELEM(optgroup, PR_TRUE, PR_FALSE, GROUP_SELECT_CONTENT,
       GROUP_OPTGROUP_CONTENT),
  ELEM(option, PR_TRUE, PR_FALSE,
       GROUP_SELECT_CONTENT | GROUP_OPTGROUP_CONTENT, GROUP_LEAF),
  ELEM(p, PR_TRUE, PR_FALSE, GROUP_BLOCK | GROUP_P, GROUP_INLINE_ELEMENT),
  ELEM(param, PR_FALSE, PR_FALSE, GROUP_OBJECT_CONTENT, GROUP_NONE),
  ELEM(plaintext, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(pre, PR_TRUE, PR_TRUE, GROUP_BLOCK, GROUP_INLINE_ELEMENT),
  ELEM(q, PR_TRUE, PR_TRUE, GROUP_SPECIAL, GROUP_INLINE_ELEMENT),
  ELEM(s, PR_TRUE, PR_TRUE, GROUP_FONTSTYLE, GROUP_INLINE_ELEMENT),
  ELEM(samp, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(script, PR_TRUE, PR_FALSE, GROUP_HEAD_CONTENT | GROUP_SPECIAL,
       GROUP_LEAF),
  ELEM(select, PR_TRUE, PR_FALSE, GROUP_FORMCONTROL, GROUP_SELECT_CONTENT),
  ELEM(small, PR_TRUE, PR_TRUE, GROUP_FONTSTYLE, GROUP_INLINE_ELEMENT),
  ELEM(spacer, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(span, PR_TRUE, PR_TRUE, GROUP_SPECIAL, GROUP_INLINE_ELEMENT),
  ELEM(strike, PR_TRUE, PR_TRUE, GROUP_FONTSTYLE, GROUP_INLINE_ELEMENT),
  ELEM(strong, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(style, PR_TRUE, PR_FALSE, GROUP_HEAD_CONTENT, GROUP_LEAF),
  ELEM(sub, PR_TRUE, PR_TRUE, GROUP_SPECIAL, GROUP_INLINE_ELEMENT),
  ELEM(sup, PR_TRUE, PR_TRUE, GROUP_SPECIAL, GROUP_INLINE_ELEMENT),
  ELEM(table, PR_TRUE, PR_FALSE, GROUP_BLOCK, GROUP_TABLE_CONTENT),
  ELEM(tbody, PR_TRUE, PR_FALSE, GROUP_TABLE_CONTENT, GROUP_TBODY_CONTENT),
  ELEM(td, PR_TRUE, PR_FALSE, GROUP_TR_CONTENT, GROUP_FLOW_ELEMENT),
  ELEM(textarea, PR_TRUE, PR_FALSE, GROUP_FORMCONTROL, GROUP_LEAF),
  ELEM(tfoot, PR_TRUE, PR_FALSE, GROUP_NONE, GROUP_TBODY_CONTENT),
  ELEM(th, PR_TRUE, PR_FALSE, GROUP_TR_CONTENT, GROUP_FLOW_ELEMENT),
  ELEM(thead, PR_TRUE, PR_FALSE, GROUP_NONE, GROUP_TBODY_CONTENT),
  ELEM(title, PR_TRUE, PR_FALSE, GROUP_HEAD_CONTENT, GROUP_LEAF),
  ELEM(tr, PR_TRUE, PR_FALSE, GROUP_TBODY_CONTENT, GROUP_TR_CONTENT),
  ELEM(tt, PR_TRUE, PR_TRUE, GROUP_FONTSTYLE, GROUP_INLINE_ELEMENT),
  ELEM(u, PR_TRUE, PR_TRUE, GROUP_FONTSTYLE, GROUP_INLINE_ELEMENT),
  // XXX Can contain self and ol because editor does sublists illegally.
  ELEM(ul, PR_TRUE, PR_TRUE, GROUP_BLOCK | GROUP_OL_UL,
       GROUP_LI | GROUP_OL_UL),
  ELEM(var, PR_TRUE, PR_TRUE, GROUP_PHRASE, GROUP_INLINE_ELEMENT),
  ELEM(wbr, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(xmp, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),

  // These aren't elements.
  ELEM(text, PR_FALSE, PR_FALSE, GROUP_LEAF, GROUP_NONE),
  ELEM(whitespace, PR_FALSE, PR_FALSE, GROUP_LEAF, GROUP_NONE),
  ELEM(newline, PR_FALSE, PR_FALSE, GROUP_LEAF, GROUP_NONE),
  ELEM(comment, PR_FALSE, PR_FALSE, GROUP_LEAF, GROUP_NONE),
  ELEM(entity, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(doctypeDecl, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(markupDecl, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),
  ELEM(instruction, PR_FALSE, PR_FALSE, GROUP_NONE, GROUP_NONE),

  ELEM(userdefined, PR_TRUE, PR_FALSE, GROUP_NONE, GROUP_FLOW_ELEMENT)
};

PRBool
nsHTMLEditUtils::CanContain(PRInt32 aParent, PRInt32 aChild, msiITagListManager * manager)
{
  NS_ASSERTION(aParent > eHTMLTag_unknown && aParent <= eHTMLTag_userdefined,
               "aParent out of range!");
  NS_ASSERTION(aChild > eHTMLTag_unknown && aChild <= eHTMLTag_userdefined,
               "aChild out of range!");

#ifdef DEBUG
  static PRBool checked = PR_FALSE;
  if (!checked) {
    checked = PR_TRUE;
    PRInt32 i;
    for (i = 1; i <= eHTMLTag_userdefined; ++i) {
      NS_ASSERTION(kElements[i - 1].mTag == i,
                   "You need to update kElements (missing tags).");
    }
  }
#endif

  // Special-case button.
  if (aParent == eHTMLTag_button) {
    static const eHTMLTags kButtonExcludeKids[] = {
      eHTMLTag_a,
      eHTMLTag_fieldset,
      eHTMLTag_form,
      eHTMLTag_iframe,
      eHTMLTag_input,
      eHTMLTag_isindex,
      eHTMLTag_select,
      eHTMLTag_textarea
    };

    PRUint32 j;
    for (j = 0; j < NS_ARRAY_LENGTH(kButtonExcludeKids); ++j) {
      if (kButtonExcludeKids[j] == aChild) {
        return PR_FALSE;
      }
    }
  }

  // Deprecated elements.
  if (aChild == eHTMLTag_bgsound || aChild == eHTMLTag_keygen) {
    return PR_FALSE;
  }

  // Bug #67007, dont strip userdefined tags.
  if (aChild == eHTMLTag_userdefined) {
    return PR_TRUE;
  }

  const nsElementInfo& parent = kElements[aParent - 1];
  if (aParent == aChild) {
    return parent.mCanContainSelf;
  }

  const nsElementInfo& child = kElements[aChild - 1];
  return (parent.mCanContainGroups & child.mGroup) != 0;
}

PRBool
nsHTMLEditUtils::IsContainer(PRInt32 aTag, msiITagListManager * manager)
{
  NS_ASSERTION(aTag > eHTMLTag_unknown && aTag <= eHTMLTag_userdefined,
               "aTag out of range!");

  return kElements[aTag - 1].mIsContainer;
}

void
nsHTMLEditUtils::MatchingFence( nsIDOMElement * element, nsIDOMElement ** other )
{
  // if node is a fence, return the paired one.
  nsAutoString form;
  element->GetAttribute(NS_LITERAL_STRING("form"), form);
  if (form.EqualsLiteral("prefix") || form.EqualsLiteral("postfix"))
  {
    nsCOMPtr<nsIDOMTreeWalker> tw;
    nsresult res;
    nsCOMPtr<nsIDOMNode> mathNode;
    nsCOMPtr<nsIDOMNode> node;
    nsCOMPtr<nsIDOMElement> elt;
    nsAutoString nodeName;
    nsCOMPtr<nsIDOMDocument> doc;
    *other = nsnull;
    res = element->GetOwnerDocument(getter_AddRefs(doc));
    res = msiUtils::GetMathParent(element, mathNode);
    if (!mathNode) return;
    nsCOMPtr<nsIDOMDocumentTraversal> trav = do_QueryInterface(doc, &res);
    res = trav->CreateTreeWalker(mathNode,
         nsIDOMNodeFilter::SHOW_ELEMENT,
         nsnull, PR_FALSE, getter_AddRefs(tw));
    tw->SetCurrentNode(element);
    // Now the tree is set up for examination
    PRInt32 counter = 1;
    nsAutoString tempform;
    if (form.EqualsLiteral("postfix"))
    {
      tw->PreviousNode(getter_AddRefs(node));
      while (counter > 0 && node) {
        elt = do_QueryInterface(node);
        elt->GetNodeName(nodeName);
        if (nodeName.EqualsLiteral("mo"))
        {
          elt->GetAttribute(NS_LITERAL_STRING("form"), tempform);
          if (tempform.EqualsLiteral("prefix")) counter--;
          else if (tempform.EqualsLiteral("postfix")) counter++;
          if (counter == 0) // found matching postfix
          {
            *other = elt;
            NS_ADDREF(*other);
            return;
          }
        }
        if (counter > 0) tw->PreviousNode(getter_AddRefs(node));
      }
    }
    else if (form.EqualsLiteral("prefix")) // same thing in reverse
    {
      tw->NextNode(getter_AddRefs(node));
      while (counter > 0 && node) {
        elt = do_QueryInterface(node);
        elt->GetNodeName(nodeName);
        if (nodeName.EqualsLiteral("mo"))
        {
          elt->GetAttribute(NS_LITERAL_STRING("form"), tempform);
          if (tempform.EqualsLiteral("prefix")) counter++;
          else if (tempform.EqualsLiteral("postfix")) counter--;
          if (counter == 0) // found matching postfix
          {  
            *other = elt;
            NS_ADDREF(*other);
            return;
          }
        }
        if (counter > 0) tw->NextNode(getter_AddRefs(node));
      }
    }
  }
}
