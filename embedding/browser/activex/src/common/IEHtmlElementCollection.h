/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *   Adam Lock <adamlock@netscape.com>
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
#ifndef IEHTMLNODECOLLECTION_H
#define IEHTMLNODECOLLECTION_H

#include "nsIDOMHTMLCollection.h"
#include "nsIDOMNodeList.h"

#include "IEHtmlNode.h"

class CIEHtmlElement;

class CIEHtmlElementCollection :
    public CNode,
    public IDispatchImpl<IHTMLElementCollection, &IID_IHTMLElementCollection, &LIBID_MSHTML>
{
private:
    // Hold a DOM node list
    nsCOMPtr<nsIDOMNodeList> mDOMNodeList;
    // Or hold a static collection
    IDispatch  **mNodeList;
    PRUint32     mNodeListCount;
    PRUint32     mNodeListCapacity;

public:
    CIEHtmlElementCollection();

protected:
    virtual ~CIEHtmlElementCollection();
    virtual HRESULT FindOrCreateIEElement(nsIDOMNode* domNode, IHTMLElement** pIHtmlElement);

public:
    // Adds a node to the collection
    virtual HRESULT AddNode(IDispatch *pNode);

    virtual HRESULT PopulateFromDOMHTMLCollection(nsIDOMHTMLCollection *pNodeList);

    // Populates the collection with items from the DOM node
    virtual HRESULT PopulateFromDOMNode(nsIDOMNode *pIDOMNode, BOOL bRecurseChildren);

    // Helper method creates a collection from a parent node
    static HRESULT CreateFromParentNode(CNode *pParentNode, BOOL bRecurseChildren, CIEHtmlElementCollection **pInstance);

    // Helper method creates a collection from the specified HTML collection
    static HRESULT CreateFromDOMHTMLCollection(CNode *pParentNode, nsIDOMHTMLCollection *pNodeList, CIEHtmlElementCollection **pInstance);


BEGIN_COM_MAP(CIEHtmlElementCollection)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IHTMLElementCollection)
END_COM_MAP()

    // IHTMLElementCollection methods
    virtual HRESULT STDMETHODCALLTYPE toString(BSTR __RPC_FAR *String);
    virtual HRESULT STDMETHODCALLTYPE put_length(long v);
    virtual HRESULT STDMETHODCALLTYPE get_length(long __RPC_FAR *p);
    virtual HRESULT STDMETHODCALLTYPE get__newEnum(IUnknown __RPC_FAR *__RPC_FAR *p);
    virtual HRESULT STDMETHODCALLTYPE item(VARIANT name, VARIANT index, IDispatch __RPC_FAR *__RPC_FAR *pdisp);
    virtual HRESULT STDMETHODCALLTYPE tags(VARIANT tagName, IDispatch __RPC_FAR *__RPC_FAR *pdisp);
};

typedef CComObject<CIEHtmlElementCollection> CIEHtmlElementCollectionInstance;

#endif