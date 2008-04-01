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
#include "nsCOMPtr.h"
#include "nsIDOMHTMLFontElement.h"
#include "nsIDOMEventTarget.h"
#include "nsGenericHTMLElement.h"
#include "nsGkAtoms.h"
#include "nsIDeviceContext.h"
#include "nsStyleConsts.h"
#include "nsPresContext.h"
#include "nsMappedAttributes.h"
#include "nsCSSStruct.h"
#include "nsRuleData.h"
#include "nsIDocument.h"

class nsHTMLFontElement : public nsGenericHTMLElement,
                          public nsIDOMHTMLFontElement
{
public:
  nsHTMLFontElement(nsINodeInfo *aNodeInfo);
  virtual ~nsHTMLFontElement();

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIDOMNode
  NS_FORWARD_NSIDOMNODE(nsGenericHTMLElement::)

  // nsIDOMElement
  NS_FORWARD_NSIDOMELEMENT(nsGenericHTMLElement::)

  // nsIDOMHTMLElement
  NS_FORWARD_NSIDOMHTMLELEMENT(nsGenericHTMLElement::)

  // nsIDOMHTMLFontElement
  NS_DECL_NSIDOMHTMLFONTELEMENT

  virtual PRBool ParseAttribute(PRInt32 aNamespaceID,
                                nsIAtom* aAttribute,
                                const nsAString& aValue,
                                nsAttrValue& aResult);
  NS_IMETHOD_(PRBool) IsAttributeMapped(const nsIAtom* aAttribute) const;
  virtual nsMapRuleToAttributesFunc GetAttributeMappingFunction() const;
  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
};


NS_IMPL_NS_NEW_HTML_ELEMENT(Font)


nsHTMLFontElement::nsHTMLFontElement(nsINodeInfo *aNodeInfo)
  : nsGenericHTMLElement(aNodeInfo)
{
}

nsHTMLFontElement::~nsHTMLFontElement()
{
}

NS_IMPL_ADDREF_INHERITED(nsHTMLFontElement, nsGenericElement)
NS_IMPL_RELEASE_INHERITED(nsHTMLFontElement, nsGenericElement)


// QueryInterface implementation for nsHTMLFontElement
NS_HTML_CONTENT_INTERFACE_TABLE_HEAD(nsHTMLFontElement, nsGenericHTMLElement)
  NS_INTERFACE_TABLE_INHERITED1(nsHTMLFontElement, nsIDOMHTMLFontElement)
NS_HTML_CONTENT_INTERFACE_TABLE_TAIL_CLASSINFO(HTMLFontElement)


NS_IMPL_ELEMENT_CLONE(nsHTMLFontElement)


NS_IMPL_STRING_ATTR(nsHTMLFontElement, Color, color)
NS_IMPL_STRING_ATTR(nsHTMLFontElement, Face, face)
NS_IMPL_STRING_ATTR(nsHTMLFontElement, Size, size)

static const nsAttrValue::EnumTable kRelFontSizeTable[] = {
  { "-10", -10 },
  { "-9", -9 },
  { "-8", -8 },
  { "-7", -7 },
  { "-6", -6 },
  { "-5", -5 },
  { "-4", -4 },
  { "-3", -3 },
  { "-2", -2 },
  { "-1", -1 },
  { "-0", 0 },
  { "+0", 0 },
  { "+1", 1 },
  { "+2", 2 },
  { "+3", 3 },
  { "+4", 4 },
  { "+5", 5 },
  { "+6", 6 },
  { "+7", 7 },
  { "+8", 8 },
  { "+9", 9 },
  { "+10", 10 },
  { 0 }
};


PRBool
nsHTMLFontElement::ParseAttribute(PRInt32 aNamespaceID,
                                  nsIAtom* aAttribute,
                                  const nsAString& aValue,
                                  nsAttrValue& aResult)
{
  if (aNamespaceID == kNameSpaceID_None) {
    if (aAttribute == nsGkAtoms::size) {
      nsAutoString tmp(aValue);
      tmp.CompressWhitespace(PR_TRUE, PR_TRUE);
      PRUnichar ch = tmp.IsEmpty() ? 0 : tmp.First();
      if ((ch == '+' || ch == '-') &&
          aResult.ParseEnumValue(aValue, kRelFontSizeTable)) {
        return PR_TRUE;
      }

      return aResult.ParseIntValue(aValue);
    }
    if (aAttribute == nsGkAtoms::pointSize ||
        aAttribute == nsGkAtoms::fontWeight) {
      return aResult.ParseIntValue(aValue);
    }
    if (aAttribute == nsGkAtoms::color) {
      return aResult.ParseColor(aValue, GetOwnerDoc());
    }
  }

  return nsGenericHTMLElement::ParseAttribute(aNamespaceID, aAttribute, aValue,
                                              aResult);
}

static void
MapAttributesIntoRule(const nsMappedAttributes* aAttributes,
                      nsRuleData* aData)
{
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Font)) {
    nsRuleDataFont& font = *(aData->mFontData);
    
    // face: string list
    if (font.mFamily.GetUnit() == eCSSUnit_Null) {
      const nsAttrValue* value = aAttributes->GetAttr(nsGkAtoms::face);
      if (value && value->Type() == nsAttrValue::eString &&
          !value->IsEmptyString()) {
        font.mFamily.SetStringValue(value->GetStringValue(), eCSSUnit_String);
        font.mFamilyFromHTML = PR_TRUE;
      }
    }

    // pointSize: int
    if (font.mSize.GetUnit() == eCSSUnit_Null) {
      const nsAttrValue* value = aAttributes->GetAttr(nsGkAtoms::pointSize);
      if (value && value->Type() == nsAttrValue::eInteger)
        font.mSize.SetFloatValue((float)value->GetIntegerValue(), eCSSUnit_Point);
      else {
        // size: int, enum , 
        value = aAttributes->GetAttr(nsGkAtoms::size);
        if (value) {
          nsAttrValue::ValueType unit = value->Type();
          if (unit == nsAttrValue::eInteger || unit == nsAttrValue::eEnum) { 
            PRInt32 size;
            if (unit == nsAttrValue::eEnum) // int (+/-)
              size = value->GetEnumValue() + 3;  // XXX should be BASEFONT, not three see bug 3875
            else
              size = value->GetIntegerValue();

            size = ((0 < size) ? ((size < 8) ? size : 7) : 1); 
            font.mSize.SetIntValue(size, eCSSUnit_Enumerated);
          }
        }
      }
    }

    // fontWeight: int
    if (font.mWeight.GetUnit() == eCSSUnit_Null) {
      const nsAttrValue* value = aAttributes->GetAttr(nsGkAtoms::fontWeight);
      if (value && value->Type() == nsAttrValue::eInteger) // +/-
        font.mWeight.SetIntValue(value->GetIntegerValue(), eCSSUnit_Integer);
    }
  }
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(Color)) {
    if (aData->mColorData->mColor.GetUnit() == eCSSUnit_Null &&
        aData->mPresContext->UseDocumentColors()) {
      // color: color
      const nsAttrValue* value = aAttributes->GetAttr(nsGkAtoms::color);
      nscolor color;
      if (value && value->GetColorValue(color)) {
        aData->mColorData->mColor.SetColorValue(color);
      }
    }
  }
  if (aData->mSIDs & NS_STYLE_INHERIT_BIT(TextReset)) {
    // Make <a><font color="red">text</font></a> give the text a red underline
    // in quirks mode.  The NS_STYLE_TEXT_DECORATION_OVERRIDE_ALL flag only
    // affects quirks mode rendering.
    const nsAttrValue* value = aAttributes->GetAttr(nsGkAtoms::color);
    nscolor color;
    if (value && value->GetColorValue(color)) {
      nsCSSValue& decoration = aData->mTextData->mDecoration;
      PRInt32 newValue = NS_STYLE_TEXT_DECORATION_OVERRIDE_ALL;
      if (decoration.GetUnit() == eCSSUnit_Enumerated) {
        newValue |= decoration.GetIntValue();
      }
      decoration.SetIntValue(newValue, eCSSUnit_Enumerated);
    }
  }

  nsGenericHTMLElement::MapCommonAttributesInto(aAttributes, aData);
}

NS_IMETHODIMP_(PRBool)
nsHTMLFontElement::IsAttributeMapped(const nsIAtom* aAttribute) const
{
  static const MappedAttributeEntry attributes[] = {
    { &nsGkAtoms::face },
    { &nsGkAtoms::pointSize },
    { &nsGkAtoms::size },
    { &nsGkAtoms::fontWeight },
    { &nsGkAtoms::color },
    { nsnull }
  };

  static const MappedAttributeEntry* const map[] = {
    attributes,
    sCommonAttributeMap,
  };

  return FindAttributeDependence(aAttribute, map, NS_ARRAY_LENGTH(map));
}


nsMapRuleToAttributesFunc
nsHTMLFontElement::GetAttributeMappingFunction() const
{
  return &MapAttributesIntoRule;
}
