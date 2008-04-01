/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set tw=80 expandtab softtabstop=2 ts=2 sw=2: */
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
 *   Mats Palmgren <mats.palmgren@bredband.net>
 *   Daniel Kraft <d@domob.eu>
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

#include "nsMappedAttributeElement.h"
#include "nsIDocument.h"

nsresult
nsMappedAttributeElement::BindToTree(nsIDocument* aDocument, nsIContent* aParent,
                                     nsIContent* aBindingParent,
                                     PRBool aCompileEventHandlers)
{
  nsresult rv = nsMappedAttributeElementBase::BindToTree(aDocument, aParent,
                                                         aBindingParent,
                                                         aCompileEventHandlers);
  NS_ENSURE_SUCCESS(rv, rv);

  if (aDocument) {
    // If we're in a document now, let our mapped attrs know what their new
    // sheet is.
    nsHTMLStyleSheet* sheet = aDocument->GetAttributeStyleSheet();
    if (sheet) {
      mAttrsAndChildren.SetMappedAttrStyleSheet(sheet);
    }
  }

  return rv;
}

nsresult
nsMappedAttributeElement::WalkContentStyleRules(nsRuleWalker* aRuleWalker)
{
  mAttrsAndChildren.WalkMappedAttributeStyleRules(aRuleWalker);
  return NS_OK;
}

PRBool
nsMappedAttributeElement::SetMappedAttribute(nsIDocument* aDocument,
                                             nsIAtom* aName,
                                             nsAttrValue& aValue,
                                             nsresult* aRetval)
{
  NS_PRECONDITION(aDocument == GetCurrentDoc(), "Unexpected document");
  nsHTMLStyleSheet* sheet = aDocument ?
    aDocument->GetAttributeStyleSheet() : nsnull;

  *aRetval = mAttrsAndChildren.SetAndTakeMappedAttr(aName, aValue,
                                                    this, sheet);
  return PR_TRUE;
}

nsMapRuleToAttributesFunc
nsMappedAttributeElement::GetAttributeMappingFunction() const
{
  return &MapNoAttributesInto;
}

void
nsMappedAttributeElement::MapNoAttributesInto(const nsMappedAttributes* aAttributes,
                                              nsRuleData* aData)
{
}
