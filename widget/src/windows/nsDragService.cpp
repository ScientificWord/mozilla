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
 *   Mike Pinkerton (pinkerton@netscape.com)
 *   Mark Hammond (MarkH@ActiveState.com)
 *   David Gardiner <david.gardiner@unisa.edu.au>
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

#include <ole2.h>
#include <oleidl.h>
#include <shlobj.h>
#include <shlwapi.h>

// shellapi.h is needed to build with WIN32_LEAN_AND_MEAN
#include <shellapi.h>

#include "nsDragService.h"
#include "nsITransferable.h"
#include "nsDataObj.h"

#include "nsWidgetsCID.h"
#include "nsNativeDragTarget.h"
#include "nsNativeDragSource.h"
#include "nsClipboard.h"
#include "nsISupportsArray.h"
#include "nsIDocument.h"
#include "nsDataObjCollection.h"

#include "nsAutoPtr.h"

#include "nsString.h"
#include "nsEscape.h"
#include "nsISupportsPrimitives.h"
#include "nsNetUtil.h"
#include "nsIURL.h"
#include "nsCWebBrowserPersist.h"
#include "nsToolkit.h"
#include "nsCRT.h"
#include "nsDirectoryServiceDefs.h"
#include "nsUnicharUtils.h"
#include "gfxASurface.h"
#include "gfxContext.h"
#include "nsMathUtils.h"

//-------------------------------------------------------------------------
//
// DragService constructor
//
//-------------------------------------------------------------------------
nsDragService::nsDragService()
  : mNativeDragSrc(nsnull), mNativeDragTarget(nsnull), mDataObject(nsnull)
{
}

//-------------------------------------------------------------------------
//
// DragService destructor
//
//-------------------------------------------------------------------------
nsDragService::~nsDragService()
{
  NS_IF_RELEASE(mNativeDragSrc);
  NS_IF_RELEASE(mNativeDragTarget);
  NS_IF_RELEASE(mDataObject);
}

PRBool
nsDragService::CreateDragImage(nsIDOMNode *aDOMNode,
                               nsIScriptableRegion *aRegion,
                               SHDRAGIMAGE *psdi)
{
  if (!psdi)
    return PR_FALSE;

  memset(psdi, 0, sizeof(SHDRAGIMAGE));
  if (!aDOMNode) 
    return PR_FALSE;

  // Prepare the drag image
  nsRect dragRect;
  nsRefPtr<gfxASurface> surface;
  nsPresContext* pc;
  DrawDrag(aDOMNode, aRegion,
           mScreenX, mScreenY,
           &dragRect, getter_AddRefs(surface), &pc);
  if (!surface)
    return PR_FALSE;

  PRUint32 bmWidth = dragRect.width, bmHeight = dragRect.height;

  if (bmWidth == 0 || bmHeight == 0)
    return PR_FALSE;

  psdi->crColorKey = CLR_NONE;

  nsRefPtr<gfxImageSurface> imgSurface = new gfxImageSurface(
    gfxIntSize(bmWidth, bmHeight), 
    gfxImageSurface::ImageFormatARGB32);
  if (!imgSurface)
    return PR_FALSE;

  nsRefPtr<gfxContext> context = new gfxContext(imgSurface);
  if (!context)
    return PR_FALSE;

  context->SetOperator(gfxContext::OPERATOR_SOURCE);
  context->SetSource(surface);
  context->Paint();

  BITMAPV5HEADER bmih;
  memset((void*)&bmih, 0, sizeof(BITMAPV5HEADER));
  bmih.bV5Size        = sizeof(BITMAPV5HEADER);
  bmih.bV5Width       = bmWidth;
  bmih.bV5Height      = -bmHeight; // flip vertical
  bmih.bV5Planes      = 1;
  bmih.bV5BitCount    = 32;
  bmih.bV5Compression = BI_BITFIELDS;
  bmih.bV5RedMask     = 0x00FF0000;
  bmih.bV5GreenMask   = 0x0000FF00;
  bmih.bV5BlueMask    = 0x000000FF;
  bmih.bV5AlphaMask   = 0xFF000000;

  HDC hdcSrc = CreateCompatibleDC(NULL);
  void *lpBits = NULL;
  if (hdcSrc) {
    psdi->hbmpDragImage = 
    ::CreateDIBSection(hdcSrc, (BITMAPINFO*)&bmih, DIB_RGB_COLORS,
                       (void**)&lpBits, NULL, 0);
    if (psdi->hbmpDragImage && lpBits) {
      memcpy(lpBits,imgSurface->Data(),(bmWidth*bmHeight*4));
    }

    psdi->sizeDragImage.cx = bmWidth;
    psdi->sizeDragImage.cy = bmHeight;

    // Mouse position in center
    if (mScreenX == -1 || mScreenY == -1) {
      psdi->ptOffset.x = (PRUint32)((float)bmWidth/2.0f);
      psdi->ptOffset.y = (PRUint32)((float)bmHeight/2.0f);
    } else {
      PRInt32 sx = mScreenX, sy = mScreenY;
      ConvertToUnscaledDevPixels(pc, &sx, &sy);
      psdi->ptOffset.x = sx - dragRect.x;
      psdi->ptOffset.y = sy - dragRect.y;
    }

    DeleteDC(hdcSrc);
  }

  return psdi->hbmpDragImage != NULL;
}

//-------------------------------------------------------------------------
NS_IMETHODIMP
nsDragService::InvokeDragSession(nsIDOMNode *aDOMNode,
                                 nsISupportsArray *anArrayTransferables,
                                 nsIScriptableRegion *aRegion,
                                 PRUint32 aActionType)
{
  nsBaseDragService::InvokeDragSession(aDOMNode, anArrayTransferables, aRegion,
                                       aActionType);
  nsresult rv;

  // Try and get source URI of the items that are being dragged
  nsIURI *uri = nsnull;

  nsCOMPtr<nsIDocument> doc(do_QueryInterface(mSourceDocument));
  if (doc) {
    uri = doc->GetDocumentURI();
  }

  PRUint32 numItemsToDrag = 0;
  rv = anArrayTransferables->Count(&numItemsToDrag);
  if (!numItemsToDrag)
    return NS_ERROR_FAILURE;

  // The clipboard class contains some static utility methods that we
  // can use to create an IDataObject from the transferable

  // if we're dragging more than one item, we need to create a
  // "collection" object to fake out the OS. This collection contains
  // one |IDataObject| for each transerable. If there is just the one
  // (most cases), only pass around the native |IDataObject|.
  nsRefPtr<IDataObject> itemToDrag;
  if (numItemsToDrag > 1) {
    nsDataObjCollection * dataObjCollection = new nsDataObjCollection();
    if (!dataObjCollection)
      return NS_ERROR_OUT_OF_MEMORY;
    itemToDrag = dataObjCollection;
    for (PRUint32 i=0; i<numItemsToDrag; ++i) {
      nsCOMPtr<nsISupports> supports;
      anArrayTransferables->GetElementAt(i, getter_AddRefs(supports));
      nsCOMPtr<nsITransferable> trans(do_QueryInterface(supports));
      if (trans) {
        nsRefPtr<IDataObject> dataObj;
        rv = nsClipboard::CreateNativeDataObject(trans,
                                                 getter_AddRefs(dataObj), uri);
        NS_ENSURE_SUCCESS(rv, rv);

        dataObjCollection->AddDataObject(dataObj);
      }
    }
  } // if dragging multiple items
  else {
    nsCOMPtr<nsISupports> supports;
    anArrayTransferables->GetElementAt(0, getter_AddRefs(supports));
    nsCOMPtr<nsITransferable> trans(do_QueryInterface(supports));
    if (trans) {
      rv = nsClipboard::CreateNativeDataObject(trans,
                                               getter_AddRefs(itemToDrag),
                                               uri);
      NS_ENSURE_SUCCESS(rv, rv);
    }
  } // else dragging a single object

  // Create a drag image if support is available
  IDragSourceHelper *pdsh;
  if (SUCCEEDED(CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER,
                                 IID_IDragSourceHelper, (void**)&pdsh))) {
    SHDRAGIMAGE sdi;
    if (CreateDragImage(aDOMNode, aRegion, &sdi)) {
      if (FAILED(pdsh->InitializeFromBitmap(&sdi, itemToDrag)))
        DeleteObject(sdi.hbmpDragImage);
    }
    pdsh->Release();
  }

  // Kick off the native drag session
  return StartInvokingDragSession(itemToDrag, aActionType);
}

//-------------------------------------------------------------------------
NS_IMETHODIMP
nsDragService::StartInvokingDragSession(IDataObject * aDataObj,
                                        PRUint32 aActionType)
{
  // To do the drag we need to create an object that
  // implements the IDataObject interface (for OLE)
  NS_IF_RELEASE(mNativeDragSrc);
  mNativeDragSrc = (IDropSource *)new nsNativeDragSource();
  if (!mNativeDragSrc)
    return NS_ERROR_OUT_OF_MEMORY;

  mNativeDragSrc->AddRef();

  // Now figure out what the native drag effect should be
  DWORD dropRes;
  DWORD effects = DROPEFFECT_SCROLL;
  if (aActionType & DRAGDROP_ACTION_COPY) {
    effects |= DROPEFFECT_COPY;
  }
  if (aActionType & DRAGDROP_ACTION_MOVE) {
    effects |= DROPEFFECT_MOVE;
  }
  if (aActionType & DRAGDROP_ACTION_LINK) {
    effects |= DROPEFFECT_LINK;
  }

  // XXX not sure why we bother to cache this, it can change during
  // the drag
  mDragAction = aActionType;
  mDoingDrag  = PR_TRUE;

  // Start dragging
  StartDragSession();

  // check shell32.dll version and do async drag if it is >= 5.0
  PRUint64 lShellVersion = GetShellVersion();
  IAsyncOperation *pAsyncOp = NULL;
  PRBool isAsyncAvailable = LL_UCMP(lShellVersion, >=, LL_INIT(5, 0));
  if (isAsyncAvailable)
  {
    // do async drag
    if (SUCCEEDED(aDataObj->QueryInterface(IID_IAsyncOperation,
                                          (void**)&pAsyncOp)))
      pAsyncOp->SetAsyncMode(TRUE);
  }

  // Call the native D&D method
  HRESULT res = ::DoDragDrop(aDataObj, mNativeDragSrc, effects, &dropRes);

  if (isAsyncAvailable)
  {
    // if dragging async
    // check for async operation
    BOOL isAsync = FALSE;
    if (pAsyncOp)
    {
      pAsyncOp->InOperation(&isAsync);
      if (!isAsync)
        aDataObj->Release();
    }
  }

  // We're done dragging
  EndDragSession(PR_TRUE);

  // For some drag/drop interactions, IDataObject::SetData doesn't get
  // called with a CFSTR_PERFORMEDDROPEFFECT format and the
  // intermediate file (if it was created) isn't deleted.  See
  // http://bugzilla.mozilla.org/show_bug.cgi?id=203847#c4 for a
  // detailed description of the different cases.  Now that we know
  // that the drag/drop operation has ended, call SetData() so that
  // the intermediate file is deleted.
  static CLIPFORMAT PerformedDropEffect =
    ::RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);

  FORMATETC fmte =
    {
      (CLIPFORMAT)PerformedDropEffect,
      NULL,
      DVASPECT_CONTENT,
      -1,
      TYMED_NULL
    };

  STGMEDIUM medium;
  medium.tymed = TYMED_NULL;
  medium.pUnkForRelease = NULL;
  aDataObj->SetData(&fmte, &medium, FALSE);

  mDoingDrag = PR_FALSE;

  return DRAGDROP_S_DROP == res ? NS_OK : NS_ERROR_FAILURE;
}

//-------------------------------------------------------------------------
// Make Sure we have the right kind of object
nsDataObjCollection*
nsDragService::GetDataObjCollection(IDataObject* aDataObj)
{
  nsDataObjCollection * dataObjCol = nsnull;
  if (aDataObj) {
    nsIDataObjCollection* dataObj;
    if (aDataObj->QueryInterface(IID_IDataObjCollection,
                                 (void**)&dataObj) == S_OK) {
      dataObjCol = static_cast<nsDataObjCollection*>(aDataObj);
      dataObj->Release();
    }
  }

  return dataObjCol;
}

//-------------------------------------------------------------------------
NS_IMETHODIMP
nsDragService::GetNumDropItems(PRUint32 * aNumItems)
{
  if (!mDataObject) {
    *aNumItems = 0;
    return NS_OK;
  }

  if (IsCollectionObject(mDataObject)) {
    nsDataObjCollection * dataObjCol = GetDataObjCollection(mDataObject);
    if (dataObjCol)
      *aNumItems = dataObjCol->GetNumDataObjects();
  }
  else {
    // Next check if we have a file drop. Return the number of files in
    // the file drop as the number of items we have, pretending like we
    // actually have > 1 drag item.
    FORMATETC fe2;
    SET_FORMATETC(fe2, CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL);
    if (mDataObject->QueryGetData(&fe2) == S_OK) {
      STGMEDIUM stm;
      if (mDataObject->GetData(&fe2, &stm) == S_OK) {
        HDROP hdrop = (HDROP)GlobalLock(stm.hGlobal);
        *aNumItems = ::DragQueryFileW(hdrop, 0xFFFFFFFF, NULL, 0);
        ::GlobalUnlock(stm.hGlobal);
        ::ReleaseStgMedium(&stm);
      }
    }
    else
      *aNumItems = 1;
  }

  return NS_OK;
}

//-------------------------------------------------------------------------
NS_IMETHODIMP
nsDragService::GetData(nsITransferable * aTransferable, PRUint32 anItem)
{
  // This typcially happens on a drop, the target would be asking
  // for it's transferable to be filled in
  // Use a static clipboard utility method for this
  if (!mDataObject)
    return NS_ERROR_FAILURE;

  nsresult dataFound = NS_ERROR_FAILURE;

  if (IsCollectionObject(mDataObject)) {
    // multiple items, use |anItem| as an index into our collection
    nsDataObjCollection * dataObjCol = GetDataObjCollection(mDataObject);
    PRUint32 cnt = dataObjCol->GetNumDataObjects();
    if (anItem >= 0 && anItem < cnt) {
      IDataObject * dataObj = dataObjCol->GetDataObjectAt(anItem);
      dataFound = nsClipboard::GetDataFromDataObject(dataObj, 0, nsnull,
                                                     aTransferable);
    }
    else
      NS_WARNING("Index out of range!");
  }
  else {
    // If they are asking for item "0", we can just get it...
    if (anItem == 0) {
       dataFound = nsClipboard::GetDataFromDataObject(mDataObject, anItem,
                                                      nsnull, aTransferable);
    } else {
      // It better be a file drop, or else non-zero indexes are invalid!
      FORMATETC fe2;
      SET_FORMATETC(fe2, CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL);
      if (mDataObject->QueryGetData(&fe2) == S_OK)
        dataFound = nsClipboard::GetDataFromDataObject(mDataObject, anItem,
                                                       nsnull, aTransferable);
      else
        NS_WARNING("Reqesting non-zero index, but clipboard data is not a collection!");
    }
  }
  return dataFound;
}

//---------------------------------------------------------
NS_IMETHODIMP
nsDragService::SetIDataObject(IDataObject * aDataObj)
{
  // When the native drag starts the DragService gets
  // the IDataObject that is being dragged
  NS_IF_RELEASE(mDataObject);
  mDataObject = aDataObj;
  NS_IF_ADDREF(mDataObject);

  return NS_OK;
}

//-------------------------------------------------------------------------
NS_IMETHODIMP
nsDragService::IsDataFlavorSupported(const char *aDataFlavor, PRBool *_retval)
{
  if (!aDataFlavor || !mDataObject || !_retval)
    return NS_ERROR_FAILURE;

#ifdef NS_DEBUG
  if (strcmp(aDataFlavor, kTextMime) == 0)
    NS_WARNING("DO NOT USE THE text/plain DATA FLAVOR ANY MORE. USE text/unicode INSTEAD");
#endif

  *_retval = PR_FALSE;

  FORMATETC fe;
  UINT format = 0;

  if (IsCollectionObject(mDataObject)) {
    // We know we have one of our special collection objects.
    format = nsClipboard::GetFormat(aDataFlavor);
    SET_FORMATETC(fe, format, 0, DVASPECT_CONTENT, -1,
                  TYMED_HGLOBAL | TYMED_FILE | TYMED_GDI);

    // See if any one of the IDataObjects in the collection supports
    // this data type
    nsDataObjCollection* dataObjCol = GetDataObjCollection(mDataObject);
    if (dataObjCol) {
      PRUint32 cnt = dataObjCol->GetNumDataObjects();
      for (PRUint32 i=0;i<cnt;++i) {
        IDataObject * dataObj = dataObjCol->GetDataObjectAt(i);
        if (S_OK == dataObj->QueryGetData(&fe))
          *_retval = PR_TRUE;             // found it!
      }
    }
  } // if special collection object
  else {
    // Ok, so we have a single object. Check to see if has the correct
    // data type. Since this can come from an outside app, we also
    // need to see if we need to perform text->unicode conversion if
    // the client asked for unicode and it wasn't available.
    format = nsClipboard::GetFormat(aDataFlavor);
    SET_FORMATETC(fe, format, 0, DVASPECT_CONTENT, -1,
                  TYMED_HGLOBAL | TYMED_FILE | TYMED_GDI);
    if (mDataObject->QueryGetData(&fe) == S_OK)
      *_retval = PR_TRUE;                 // found it!
    else {
      // We haven't found the exact flavor the client asked for, but
      // maybe we can still find it from something else that's on the
      // clipboard
      if (strcmp(aDataFlavor, kUnicodeMime) == 0) {
        // client asked for unicode and it wasn't present, check if we
        // have CF_TEXT.  We'll handle the actual data substitution in
        // the data object.
        format = nsClipboard::GetFormat(kTextMime);
        SET_FORMATETC(fe, format, 0, DVASPECT_CONTENT, -1,
                      TYMED_HGLOBAL | TYMED_FILE | TYMED_GDI);
        if (mDataObject->QueryGetData(&fe) == S_OK)
          *_retval = PR_TRUE;                 // found it!
      }
      else if (strcmp(aDataFlavor, kURLMime) == 0) {
        // client asked for a url and it wasn't present, but if we
        // have a file, then we have a URL to give them (the path, or
        // the internal URL if an InternetShortcut).
        format = nsClipboard::GetFormat(kFileMime);
        SET_FORMATETC(fe, format, 0, DVASPECT_CONTENT, -1,
                      TYMED_HGLOBAL | TYMED_FILE | TYMED_GDI);
        if (mDataObject->QueryGetData(&fe) == S_OK)
          *_retval = PR_TRUE;                 // found it!
      }
    } // else try again
  }

  return NS_OK;
}


//
// IsCollectionObject
//
// Determine if this is a single |IDataObject| or one of our private
// collection objects. We know the difference because our collection
// object will respond to supporting the private |MULTI_MIME| format.
//
PRBool
nsDragService::IsCollectionObject(IDataObject* inDataObj)
{
  PRBool isCollection = PR_FALSE;

  // setup the format object to ask for the MULTI_MIME format. We only
  // need to do this once
  static UINT sFormat = 0;
  static FORMATETC sFE;
  if (!sFormat) {
    sFormat = nsClipboard::GetFormat(MULTI_MIME);
    SET_FORMATETC(sFE, sFormat, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL);
  }

  // ask the object if it supports it. If yes, we have a collection
  // object
  if (inDataObj->QueryGetData(&sFE) == S_OK)
    isCollection = PR_TRUE;

  return isCollection;

} // IsCollectionObject


//
// EndDragSession
//
// Override the default to make sure that we release the data object
// when the drag ends. It seems that OLE doesn't like to let apps quit
// w/out crashing when we're still holding onto their data
//
NS_IMETHODIMP
nsDragService::EndDragSession(PRBool aDoneDrag)
{
  nsBaseDragService::EndDragSession(aDoneDrag);
  NS_IF_RELEASE(mDataObject);

  return NS_OK;
}

// Gets shell version as packed 64 bit int
PRUint64 nsDragService::GetShellVersion()
{
  PRUint64 lVersion = LL_INIT(0, 0);
  PRUint64 lMinor = lVersion;

  // shell32.dll should be loaded already, so we ae not actually loading the library here
  PRLibrary *libShell = PR_LoadLibrary("shell32.dll");
  if (libShell == NULL)
    return lVersion;

  do
  {
    DLLGETVERSIONPROC versionProc = NULL;
    versionProc = (DLLGETVERSIONPROC)PR_FindFunctionSymbol(libShell, "DllGetVersion");
    if (versionProc == NULL)
      break;

    DLLVERSIONINFO versionInfo;
    ::ZeroMemory(&versionInfo, sizeof(DLLVERSIONINFO));
    versionInfo.cbSize = sizeof(DLLVERSIONINFO);
    if (FAILED(versionProc(&versionInfo)))
      break;

    // why is this?
    LL_UI2L(lVersion, versionInfo.dwMajorVersion);
    LL_SHL(lVersion, lVersion, 32);
    LL_UI2L(lMinor, versionInfo.dwMinorVersion);
    LL_OR2(lVersion, lMinor);
  } while (false);

  PR_UnloadLibrary(libShell);
  libShell = NULL;

  return lVersion;
}
