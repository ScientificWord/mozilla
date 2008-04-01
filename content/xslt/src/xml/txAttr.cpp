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
 * The Original Code is TransforMiiX XSLT processor code.
 *
 * The Initial Developer of the Original Code is
 * The MITRE Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

// Tom Kneeland (3/29/99)
//
//  Implementation of the Document Object Model Level 1 Core
//    Implementation of the Attr class
//

#include "txDOM.h"
#include "txAtoms.h"
#include "txXMLUtils.h"

Attr::Attr(nsIAtom *aPrefix, nsIAtom *aLocalName, PRInt32 aNamespaceID,
           Element *aOwnerElement, const nsAString &aValue) :
    NodeDefinition(Node::ATTRIBUTE_NODE, aLocalName, aValue,
                   aOwnerElement->getOwnerDocument()),
    mPrefix(aPrefix),
    mNamespaceID(aNamespaceID),
    mOwnerElement(aOwnerElement)
{
}

//
//Not implemented anymore, return null as an error.
//
Node* Attr::appendChild(Node* newChild)
{
  NS_ASSERTION(0, "not implemented");
  return nsnull;
}

nsresult
Attr::getNodeName(nsAString& aName) const
{
  if (mPrefix) {
    mPrefix->ToString(aName);
    aName.Append(PRUnichar(':'));
  }
  else {
    aName.Truncate();
  }

  const char* ASCIIAtom;
  mLocalName->GetUTF8String(&ASCIIAtom);
  AppendUTF8toUTF16(ASCIIAtom, aName);

  return NS_OK;
}

//
//Return the attributes local (unprefixed) name atom.
//
MBool Attr::getLocalName(nsIAtom** aLocalName)
{
  if (!aLocalName)
    return MB_FALSE;
  *aLocalName = mLocalName;
  NS_ADDREF(*aLocalName);
  return MB_TRUE;
}

//
//Return the namespace the attribute belongs to. If the attribute doesn't
//have a prefix it doesn't belong to any namespace per the namespace spec,
//and is handled in the constructor.
//
PRInt32 Attr::getNamespaceID()
{
  return mNamespaceID;
}

//
//Return the attributes owning element
//
Node* Attr::getXPathParent()
{
  return mOwnerElement;
}
