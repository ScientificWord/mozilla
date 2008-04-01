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

/*
 * Implementation of DOM Core's nsIDOMDocumentType node.
 */

#ifndef nsDOMDocumentType_h___
#define nsDOMDocumentType_h___

#include "nsCOMPtr.h"
#include "nsIDOMDocumentType.h"
#include "nsIContent.h"
#include "nsGenericDOMDataNode.h"
#include "nsString.h"

// XXX DocumentType is currently implemented by inheriting the generic
// CharacterData object, even though DocumentType is not character
// data. This is done simply for convenience and should be changed if
// this restricts what should be done for character data.

class nsDOMDocumentType : public nsGenericDOMDataNode,
                          public nsIDOMDocumentType
{
public:
  nsDOMDocumentType(nsINodeInfo* aNodeInfo,
                    nsIAtom *aName,
                    nsIDOMNamedNodeMap *aEntities,
                    nsIDOMNamedNodeMap *aNotations,
                    const nsAString& aPublicId,
                    const nsAString& aSystemId,
                    const nsAString& aInternalSubset);

  virtual ~nsDOMDocumentType();

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIDOMNode
  NS_IMPL_NSIDOMNODE_USING_GENERIC_DOM_DATA

  // nsIDOMDocumentType
  NS_DECL_NSIDOMDOCUMENTTYPE

  // nsIContent overrides
  virtual PRBool IsNodeOfType(PRUint32 aFlags) const;
  virtual const nsTextFragment* GetText();
  virtual nsresult BindToTree(nsIDocument *aDocument, nsIContent *aParent,
                              nsIContent *aBindingParent,
                              PRBool aCompileEventHandlers);


protected:
  nsCOMPtr<nsIAtom> mName;
  nsCOMPtr<nsIDOMNamedNodeMap> mEntities;
  nsCOMPtr<nsIDOMNamedNodeMap> mNotations;
  nsString mPublicId;
  nsString mSystemId;
  nsString mInternalSubset;
};

nsresult
NS_NewDOMDocumentType(nsIDOMDocumentType** aDocType,
                      nsNodeInfoManager *aOwnerDoc,
                      nsIPrincipal *aPrincipal,
                      nsIAtom *aName,
                      nsIDOMNamedNodeMap *aEntities,
                      nsIDOMNamedNodeMap *aNotations,
                      const nsAString& aPublicId,
                      const nsAString& aSystemId,
                      const nsAString& aInternalSubset);

#endif // nsDOMDocument_h___
