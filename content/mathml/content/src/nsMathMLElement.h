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
 * The Original Code is MathML DOM code.
 *
 * The Initial Developer of the Original Code is
 * mozilla.org.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Vlad Sukhoy <vladimir.sukhoy@gmail.com> (original developer)
 *    Daniel Kraft <d@domob.eu> (nsMathMLElement patch, attachment 262925) 
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

#ifndef nsMathMLElement_h
#define nsMathMLElement_h

#include "nsMappedAttributeElement.h"
#include "nsIDOMElement.h"

class nsCSSValue;

typedef nsMappedAttributeElement nsMathMLElementBase;

/*
 * The base class for MathML elements.
 */
class nsMathMLElement : public nsMathMLElementBase
                      , public nsIDOMElement
{
public:
  nsMathMLElement(nsINodeInfo* aNodeInfo)
    : nsMathMLElementBase(aNodeInfo), mIncrementScriptLevel(PR_FALSE)
  {}

  // Implementation of nsISupports is inherited from nsMathMLElementBase
  NS_DECL_ISUPPORTS_INHERITED

  // Forward implementations of parent interfaces of nsMathMLElement to 
  // our base class

  // Since we override one of the nsIDOMNode method, we can't use the following line:
//  NS_FORWARD_NSIDOMNODE(nsMathMLElementBase::)
//  Instead we do this:
  NS_IMETHOD GetNodeName(nsAString & aNodeName) { return nsMathMLElementBase::GetNodeName(aNodeName); } \
  NS_IMETHOD GetNodeValue(nsAString & aNodeValue) { return nsMathMLElementBase::GetNodeValue(aNodeValue); } \
  NS_IMETHOD SetNodeValue(const nsAString & aNodeValue) { return nsMathMLElementBase::SetNodeValue(aNodeValue); } \
  NS_IMETHOD GetNodeType(PRUint16 *aNodeType) { return nsMathMLElementBase::GetNodeType(aNodeType); } \
  NS_IMETHOD GetParentNode(nsIDOMNode * *aParentNode) { return nsMathMLElementBase::GetParentNode(aParentNode); } \
  NS_IMETHOD GetChildNodes(nsIDOMNodeList * *aChildNodes) { return nsMathMLElementBase::GetChildNodes(aChildNodes); } \
  NS_IMETHOD GetFirstChild(nsIDOMNode * *aFirstChild) { return nsMathMLElementBase::GetFirstChild(aFirstChild); } \
  NS_IMETHOD GetLastChild(nsIDOMNode * *aLastChild) { return nsMathMLElementBase::GetLastChild(aLastChild); } \
  NS_IMETHOD GetPreviousSibling(nsIDOMNode * *aPreviousSibling) { return nsMathMLElementBase::GetPreviousSibling(aPreviousSibling); } \
  NS_IMETHOD GetNextSibling(nsIDOMNode * *aNextSibling) { return nsMathMLElementBase::GetNextSibling(aNextSibling); } \
  NS_IMETHOD GetAttributes(nsIDOMNamedNodeMap * *aAttributes) { return nsMathMLElementBase::GetAttributes(aAttributes); } \
  NS_IMETHOD GetOwnerDocument(nsIDOMDocument * *aOwnerDocument) { return nsMathMLElementBase::GetOwnerDocument(aOwnerDocument); } \
  NS_IMETHOD InsertBefore(nsIDOMNode *newChild, nsIDOMNode *refChild, nsIDOMNode **_retval) { return nsMathMLElementBase::InsertBefore(newChild, refChild, _retval); } \
  NS_IMETHOD ReplaceChild(nsIDOMNode *newChild, nsIDOMNode *oldChild, nsIDOMNode **_retval) { return nsMathMLElementBase::ReplaceChild(newChild, oldChild, _retval); } \
  NS_IMETHOD RemoveChild(nsIDOMNode *oldChild, nsIDOMNode **_retval) { return nsMathMLElementBase::RemoveChild(oldChild, _retval); } \
  NS_IMETHOD AppendChild(nsIDOMNode *newChild, nsIDOMNode **_retval) { return nsMathMLElementBase::AppendChild(newChild, _retval); } \
  NS_IMETHOD HasChildNodes(PRBool *_retval) { return nsMathMLElementBase::HasChildNodes(_retval); } \
  NS_IMETHOD CloneNode(PRBool deep, nsIDOMNode **_retval) { return nsMathMLElementBase::CloneNode(deep, _retval); } \
  NS_IMETHOD Normalize(void) { return nsMathMLElementBase::Normalize(); } \
  NS_IMETHOD IsSupported(const nsAString & feature, const nsAString & version, PRBool *_retval) { return nsMathMLElementBase::IsSupported(feature, version, _retval); } \
  NS_IMETHOD GetNamespaceURI(nsAString & aNamespaceURI) { return nsMathMLElementBase::GetNamespaceURI(aNamespaceURI); } \
  NS_IMETHOD GetPrefix(nsAString & aPrefix) { return nsMathMLElementBase::GetPrefix(aPrefix); } \
  NS_IMETHOD SetPrefix(const nsAString & aPrefix) { return nsMathMLElementBase::SetPrefix(aPrefix); } \
  NS_IMETHOD GetLocalName(nsAString & aLocalName) { return nsMathMLElementBase::GetLocalName(aLocalName); } \
  NS_IMETHOD HasAttributes(PRBool *_retval) { return nsMathMLElementBase::HasAttributes(_retval); } 

  NS_IMETHOD GetLastTraversalChild(nsIDOMNode * *aLastTraversalChild);

  NS_FORWARD_NSIDOMELEMENT(nsMathMLElementBase::)

  nsresult BindToTree(nsIDocument* aDocument, nsIContent* aParent,
                      nsIContent* aBindingParent,
                      PRBool aCompileEventHandlers);

  virtual PRBool ParseAttribute(PRInt32 aNamespaceID,
                                nsIAtom* aAttribute,
                                const nsAString& aValue,
                                nsAttrValue& aResult);

  NS_IMETHOD_(PRBool) IsAttributeMapped(const nsIAtom* aAttribute) const;
  virtual nsMapRuleToAttributesFunc GetAttributeMappingFunction() const;

  enum {
    PARSE_ALLOW_UNITLESS = 0x01, // unitless 0 will be turned into 0px
    PARSE_ALLOW_NEGATIVE = 0x02
  };
  static PRBool ParseNumericValue(const nsString& aString,
                                  nsCSSValue&     aCSSValue,
                                  PRUint32        aFlags);

  static void MapMathMLAttributesInto(const nsMappedAttributes* aAttributes, 
                                      nsRuleData* aRuleData);
  
  nsresult Clone(nsINodeInfo*, nsINode**) const;
  virtual PRInt32 IntrinsicState() const;
  virtual PRBool IsNodeOfType(PRUint32 aFlags) const;

  // Set during reflow as necessary. Does a style change notification,
  // aNotify must be true.
  void SetIncrementScriptLevel(PRBool aIncrementScriptLevel, PRBool aNotify);
  PRBool GetIncrementScriptLevel() const {
    return mIncrementScriptLevel;
  }

private:
  PRPackedBool mIncrementScriptLevel;
};

#endif // nsMathMLElement_h
