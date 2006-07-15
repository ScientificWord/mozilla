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

#include "nsDOMDocumentType.h"
#include "nsDOMAttributeMap.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsLayoutAtoms.h"
#include "nsCOMPtr.h"
#include "nsContentUtils.h"
#include "nsDOMString.h"
#include "nsIDOM3Node.h"
#include "nsNodeInfoManager.h"
#include "nsLayoutAtoms.h"
#include "nsIDocument.h"

nsresult
NS_NewDOMDocumentType(nsIDOMDocumentType** aDocType,
                      nsNodeInfoManager *aNodeInfoManager,
                      nsIPrincipal *aPrincipal,
                      nsIAtom *aName,
                      nsIDOMNamedNodeMap *aEntities,
                      nsIDOMNamedNodeMap *aNotations,
                      const nsAString& aPublicId,
                      const nsAString& aSystemId,
                      const nsAString& aInternalSubset)
{
  NS_PRECONDITION(aNodeInfoManager || aPrincipal,
                  "Must have a principal if no nodeinfo manager.");
  NS_ENSURE_ARG_POINTER(aDocType);
  NS_ENSURE_ARG_POINTER(aName);

  nsresult rv;

  nsRefPtr<nsNodeInfoManager> nimgr;
  if (aNodeInfoManager) {
    nimgr = aNodeInfoManager;
  }
  else {
    nimgr = new nsNodeInfoManager();
    NS_ENSURE_TRUE(nimgr, NS_ERROR_OUT_OF_MEMORY);
    
    rv = nimgr->Init(nsnull);
    NS_ENSURE_SUCCESS(rv, rv);

    nimgr->SetDocumentPrincipal(aPrincipal);
  }

  nsCOMPtr<nsINodeInfo> ni;
  rv = nimgr->GetNodeInfo(nsLayoutAtoms::documentTypeNodeName, nsnull,
                          kNameSpaceID_None, getter_AddRefs(ni));
  NS_ENSURE_SUCCESS(rv, rv);

  *aDocType = new nsDOMDocumentType(ni, aName, aEntities, aNotations,
                                    aPublicId, aSystemId, aInternalSubset);
  if (!*aDocType) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  NS_ADDREF(*aDocType);

  return NS_OK;
}

nsDOMDocumentType::nsDOMDocumentType(nsINodeInfo *aNodeInfo,
                                     nsIAtom *aName,
                                     nsIDOMNamedNodeMap *aEntities,
                                     nsIDOMNamedNodeMap *aNotations,
                                     const nsAString& aPublicId,
                                     const nsAString& aSystemId,
                                     const nsAString& aInternalSubset) :
  nsGenericDOMDataNode(aNodeInfo),
  mName(aName),
  mEntities(aEntities),
  mNotations(aNotations),
  mPublicId(aPublicId),
  mSystemId(aSystemId),
  mInternalSubset(aInternalSubset)
{
}

nsDOMDocumentType::~nsDOMDocumentType()
{
}


// QueryInterface implementation for nsDOMDocumentType
NS_INTERFACE_MAP_BEGIN(nsDOMDocumentType)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNode)
  NS_INTERFACE_MAP_ENTRY(nsIDOMDocumentType)
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(DocumentType)
NS_INTERFACE_MAP_END_INHERITING(nsGenericDOMDataNode)


NS_IMPL_ADDREF_INHERITED(nsDOMDocumentType, nsGenericDOMDataNode)
NS_IMPL_RELEASE_INHERITED(nsDOMDocumentType, nsGenericDOMDataNode)


NS_IMETHODIMP    
nsDOMDocumentType::GetName(nsAString& aName)
{
  return mName->ToString(aName);
}

NS_IMETHODIMP    
nsDOMDocumentType::GetEntities(nsIDOMNamedNodeMap** aEntities)
{
  NS_ENSURE_ARG_POINTER(aEntities);

  *aEntities = mEntities;

  NS_IF_ADDREF(*aEntities);

  return NS_OK;
}

NS_IMETHODIMP    
nsDOMDocumentType::GetNotations(nsIDOMNamedNodeMap** aNotations)
{
  NS_ENSURE_ARG_POINTER(aNotations);

  *aNotations = mNotations;

  NS_IF_ADDREF(*aNotations);

  return NS_OK;
}

NS_IMETHODIMP
nsDOMDocumentType::GetPublicId(nsAString& aPublicId)
{
  aPublicId = mPublicId;

  return NS_OK;
}

NS_IMETHODIMP
nsDOMDocumentType::GetSystemId(nsAString& aSystemId)
{
  aSystemId = mSystemId;

  return NS_OK;
}

NS_IMETHODIMP
nsDOMDocumentType::GetInternalSubset(nsAString& aInternalSubset)
{
  // XXX: null string
  aInternalSubset = mInternalSubset;

  return NS_OK;
}

NS_IMETHODIMP
nsDOMDocumentType::GetNodeName(nsAString& aNodeName)
{
  return mName->ToString(aNodeName);
}

NS_IMETHODIMP
nsDOMDocumentType::GetNodeValue(nsAString& aNodeValue)
{
  SetDOMStringToNull(aNodeValue);

  return NS_OK;
}

NS_IMETHODIMP
nsDOMDocumentType::SetNodeValue(const nsAString& aNodeValue)
{
  return NS_OK;
}

NS_IMETHODIMP
nsDOMDocumentType::GetNodeType(PRUint16* aNodeType)
{
  *aNodeType = nsIDOMNode::DOCUMENT_TYPE_NODE;

  return NS_OK;
}

nsGenericDOMDataNode*
nsDOMDocumentType::Clone(nsINodeInfo *aNodeInfo, PRBool aCloneText) const
{
  return new nsDOMDocumentType(aNodeInfo, mName, mEntities, mNotations,
                               mPublicId, mSystemId, mInternalSubset);
}
