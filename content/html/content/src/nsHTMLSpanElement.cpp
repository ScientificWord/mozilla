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
#include "nsIDOMHTMLElement.h"
#include "nsIDOMEventTarget.h"
#include "nsGenericHTMLElement.h"
#include "nsGkAtoms.h"
#include "nsStyleConsts.h"
#include "nsPresContext.h"
#include "nsIAtom.h"
#include "nsRuleData.h"

class nsHTMLSpanElement : public nsGenericHTMLElement,
                          public nsIDOMHTMLElement
{
public:
  nsHTMLSpanElement(nsINodeInfo *aNodeInfo);
  virtual ~nsHTMLSpanElement();

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIDOMNode
  NS_FORWARD_NSIDOMNODE(nsGenericHTMLElement::)

  // nsIDOMElement
  NS_FORWARD_NSIDOMELEMENT(nsGenericHTMLElement::)

  // nsIDOMHTMLElement
  NS_FORWARD_NSIDOMHTMLELEMENT(nsGenericHTMLElement::)

  virtual nsresult GetInnerHTML(nsAString& aInnerHTML);
  virtual nsresult SetInnerHTML(const nsAString& aInnerHTML);

  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};


NS_IMPL_NS_NEW_HTML_ELEMENT(Span)


nsHTMLSpanElement::nsHTMLSpanElement(nsINodeInfo *aNodeInfo)
  : nsGenericHTMLElement(aNodeInfo)
{
}

nsHTMLSpanElement::~nsHTMLSpanElement()
{
}


NS_IMPL_ADDREF_INHERITED(nsHTMLSpanElement, nsGenericElement)
NS_IMPL_RELEASE_INHERITED(nsHTMLSpanElement, nsGenericElement)


// QueryInterface implementation for nsHTMLSpanElement
NS_HTML_CONTENT_INTERFACE_TABLE_HEAD(nsHTMLSpanElement, nsGenericHTMLElement)
NS_HTML_CONTENT_INTERFACE_TABLE_TAIL_CLASSINFO(HTMLSpanElement)


NS_IMPL_ELEMENT_CLONE(nsHTMLSpanElement)


nsresult
nsHTMLSpanElement::GetInnerHTML(nsAString& aInnerHTML)
{
  if (mNodeInfo->Equals(nsGkAtoms::xmp) ||
      mNodeInfo->Equals(nsGkAtoms::plaintext)) {
    nsContentUtils::GetNodeTextContent(this, PR_FALSE, aInnerHTML);
    return NS_OK;
  }

  return nsGenericHTMLElement::GetInnerHTML(aInnerHTML);  
}

nsresult
nsHTMLSpanElement::SetInnerHTML(const nsAString& aInnerHTML)
{
  if (mNodeInfo->Equals(nsGkAtoms::xmp) ||
      mNodeInfo->Equals(nsGkAtoms::plaintext)) {
    return nsContentUtils::SetNodeTextContent(this, aInnerHTML, PR_TRUE);
  }

  return nsGenericHTMLElement::SetInnerHTML(aInnerHTML);
}

// ------------------------------------------------------------------

class nsHTMLUnknownElement : public nsHTMLSpanElement
{
public:
  nsHTMLUnknownElement(nsINodeInfo *aNodeInfo);

  NS_IMETHOD QueryInterface(REFNSIID aIID, void** aInstancePtr);
  nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};

NS_INTERFACE_MAP_BEGIN(nsHTMLUnknownElement)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(HTMLUnknownElement)
NS_INTERFACE_MAP_END_INHERITING(nsHTMLSpanElement)

nsHTMLUnknownElement::nsHTMLUnknownElement(nsINodeInfo *aNodeInfo)
  : nsHTMLSpanElement(aNodeInfo)
{
}


NS_IMPL_NS_NEW_HTML_ELEMENT(Unknown)


NS_IMPL_ELEMENT_CLONE(nsHTMLUnknownElement)
