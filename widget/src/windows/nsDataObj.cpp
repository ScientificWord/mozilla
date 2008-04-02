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
 *   Sean Echevarria <Sean@Beatnik.com>
 *   Blake Ross <blaker@netscape.com>
 *   Brodie Thiesfield <brofield@jellycan.com>
 *   Masayuki Nakano <masayuki@d-toybox.com>
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
#include <shlobj.h>

#include "nsDataObj.h"
#include "nsClipboard.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsVoidArray.h"
#include "nsITransferable.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "IENUMFE.H"
#include "nsCOMPtr.h"
#include "nsIComponentManager.h"
#include "nsPrimitiveHelpers.h"
#include "nsXPIDLString.h"
#include "nsIImage.h"
#include "nsImageClipboard.h"
#include "nsIDirectoryService.h"
#include "nsILocalFile.h"
#include "nsDirectoryServiceDefs.h"
#include "prprf.h"
#include "nsCRT.h"
#include "nsPrintfCString.h"
#include "nsIStringBundle.h"
#include "nsEscape.h"
#include "nsIURL.h"
#include "nsNetUtil.h"
#include "nsXPCOMStrings.h"

// XXX for older version of PSDK where IAsyncOperation and related stuff is not available
// but this should be removed when parocles config is updated
// IAsyncOperation interface GUID
#ifndef __IAsyncOperation_INTERFACE_DEFINED__
  const IID IID_IAsyncOperation = {0x3D8B0590, 0xF691, 0x11d2, {0x8E, 0xA9, 0x00, 0x60, 0x97, 0xDF, 0x5B, 0xD4}};
#endif

#if 0
#define PRNTDEBUG(_x) printf(_x);
#define PRNTDEBUG2(_x1, _x2) printf(_x1, _x2);
#define PRNTDEBUG3(_x1, _x2, _x3) printf(_x1, _x2, _x3);
#else
#define PRNTDEBUG(_x) // printf(_x);
#define PRNTDEBUG2(_x1, _x2) // printf(_x1, _x2);
#define PRNTDEBUG3(_x1, _x2, _x3) // printf(_x1, _x2, _x3);
#endif

//-----------------------------------------------------------------------------
// CStream implementation
nsDataObj::CStream::CStream() : mRefCount(1)
{
}

//-----------------------------------------------------------------------------
nsDataObj::CStream::~CStream()
{
}

//-----------------------------------------------------------------------------
// helper - initializes the stream
nsresult nsDataObj::CStream::Init(nsIURI *pSourceURI)
{
  nsresult rv;
  rv = NS_NewChannel(getter_AddRefs(mChannel), pSourceURI);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = mChannel->Open(getter_AddRefs(mInputStream));
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

//-----------------------------------------------------------------------------
// IUnknown
STDMETHODIMP nsDataObj::CStream::QueryInterface(REFIID refiid, void** ppvResult)
{
  *ppvResult = NULL;
  if (IID_IUnknown == refiid ||
      refiid == IID_IStream)

  {
    *ppvResult = this;
  }

  if (NULL != *ppvResult)
  {
    ((LPUNKNOWN)*ppvResult)->AddRef();
    return S_OK;
  }

  return ResultFromScode(E_NOINTERFACE);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) nsDataObj::CStream::AddRef(void)
{
  return ++mRefCount;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG) nsDataObj::CStream::Release(void)
{
  ULONG nCount = --mRefCount;
  if (nCount == 0)
  {
    delete this;
    return (ULONG)0;
  }

  return mRefCount;
}

//-----------------------------------------------------------------------------
// IStream
STDMETHODIMP nsDataObj::CStream::Clone(IStream** ppStream)
{
  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::Commit(DWORD dwFrags)
{
  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::CopyTo(IStream* pDestStream,
                                        ULARGE_INTEGER nBytesToCopy,
                                        ULARGE_INTEGER* nBytesRead,
                                        ULARGE_INTEGER* nBytesWritten)
{
  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::LockRegion(ULARGE_INTEGER nStart,
                                            ULARGE_INTEGER nBytes,
                                            DWORD dwFlags)
{
  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::Read(void* pvBuffer,
                                      ULONG nBytesToRead,
                                      ULONG* nBytesRead)
{
  NS_ENSURE_TRUE(mInputStream, E_FAIL);

  nsresult rv;
  PRUint32 read = 0;
  rv = mInputStream->Read((char*)pvBuffer, nBytesToRead, &read);
  *nBytesRead = read;
  NS_ENSURE_SUCCESS(rv, S_FALSE);

  return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::Revert(void)
{
  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::Seek(LARGE_INTEGER nMove,
                                      DWORD dwOrigin,
                                      ULARGE_INTEGER* nNewPos)
{
  if (nNewPos == NULL)
    return STG_E_INVALIDPOINTER;

  if (nMove.LowPart == 0 && nMove.HighPart == 0 &&
      (dwOrigin == STREAM_SEEK_SET || dwOrigin == STREAM_SEEK_CUR)) { 
    nNewPos->LowPart = 0;
    nNewPos->HighPart = 0;
    return S_OK;
  }

  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::SetSize(ULARGE_INTEGER nNewSize)
{
  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::Stat(STATSTG* statstg, DWORD dwFlags)
{
  if (statstg == NULL)
    return STG_E_INVALIDPOINTER;

  if (!mChannel)
    return E_FAIL;

  memset((void*)statstg, 0, sizeof(STATSTG));

  if (dwFlags != STATFLAG_NONAME) 
  {
    nsCOMPtr<nsIURI> sourceURI;
    if (NS_FAILED(mChannel->GetURI(getter_AddRefs(sourceURI)))) {
      return E_FAIL;
    }

    nsCAutoString strFileName;
    nsCOMPtr<nsIURL> sourceURL = do_QueryInterface(sourceURI);
    sourceURL->GetFileName(strFileName);

    if (strFileName.IsEmpty())
      return E_FAIL;

    NS_UnescapeURL(strFileName);
    NS_ConvertUTF8toUTF16 wideFileName(strFileName);

    PRUint32 nMaxNameLength = (wideFileName.Length()*2) + 2;
    void * retBuf = CoTaskMemAlloc(nMaxNameLength); // freed by caller
    if (!retBuf) 
      return STG_E_INSUFFICIENTMEMORY;

    ZeroMemory(retBuf, nMaxNameLength);
    memcpy(retBuf, wideFileName.get(), wideFileName.Length()*2);
    statstg->pwcsName = (LPOLESTR)retBuf;
  }

  SYSTEMTIME st;
  FILETIME ft;

  statstg->type = STGTY_STREAM;

  GetSystemTime(&st);
  SystemTimeToFileTime((const SYSTEMTIME*)&st, (LPFILETIME)&statstg->mtime);
  statstg->ctime = statstg->atime = statstg->mtime;

  PRInt32 nLength = 0;
  if (mChannel)
    mChannel->GetContentLength(&nLength);

  if (nLength < 0) 
    nLength = 0;

  statstg->cbSize.LowPart = (DWORD)nLength;
  statstg->grfMode = STGM_READ;
  statstg->grfLocksSupported = LOCK_ONLYONCE;
  statstg->clsid = CLSID_NULL;

  return S_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::UnlockRegion(ULARGE_INTEGER nStart,
                                              ULARGE_INTEGER nBytes,
                                              DWORD dwFlags)
{
  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP nsDataObj::CStream::Write(const void* pvBuffer,
                                       ULONG nBytesToRead,
                                       ULONG* nBytesRead)
{
  return E_NOTIMPL;
}

//-----------------------------------------------------------------------------
HRESULT nsDataObj::CreateStream(IStream **outStream)
{
  NS_ENSURE_TRUE(outStream, E_INVALIDARG);

  nsresult rv = NS_ERROR_FAILURE;
  nsAutoString wideFileName;
  nsCOMPtr<nsIURI> sourceURI;

  rv = GetDownloadDetails(getter_AddRefs(sourceURI),
                          wideFileName);
  NS_ENSURE_SUCCESS(rv, rv);

  nsDataObj::CStream *pStream = new nsDataObj::CStream();
  NS_ENSURE_TRUE(pStream, E_OUTOFMEMORY);

  rv = pStream->Init(sourceURI);
  if (NS_FAILED(rv))
  {
    pStream->Release();
    return E_FAIL;
  }
  *outStream = pStream;

  return S_OK;
}

ULONG nsDataObj::g_cRef = 0;

EXTERN_C GUID CDECL CLSID_nsDataObj =
	{ 0x1bba7640, 0xdf52, 0x11cf, { 0x82, 0x7b, 0, 0xa0, 0x24, 0x3a, 0xe5, 0x05 } };

/* 
 * deliberately not using MAX_PATH. This is because on platforms < XP
 * a file created with a long filename may be mishandled by the shell
 * resulting in it not being able to be deleted or moved. 
 * See bug 250392 for more details.
 */
#define NS_MAX_FILEDESCRIPTOR 128 + 1

/*
 * Class nsDataObj
 */

//-----------------------------------------------------
// construction 
//-----------------------------------------------------
nsDataObj::nsDataObj(nsIURI * uri)
: m_cRef(0), mTransferable(nsnull)
{
  mDataFlavors    = new nsVoidArray();
  m_enumFE        = new CEnumFormatEtc(32);
 
  m_enumFE->AddRef();

  if (uri) {

    // A URI was obtained, so pass this through to the DataObject
    // so it can create a SourceURL for CF_HTML flavour
    uri->GetSpec(mSourceURL);
  }

  mIsAsyncMode = FALSE;
  mIsInOperation = FALSE;
}
//-----------------------------------------------------
// destruction
//-----------------------------------------------------
nsDataObj::~nsDataObj()
{
  NS_IF_RELEASE(mTransferable);
  PRInt32 i;
  for (i=0;i<mDataFlavors->Count();i++) {
    nsCAutoString* df = reinterpret_cast<nsCAutoString *>(mDataFlavors->ElementAt(i));
    delete df;
  }

  delete mDataFlavors;

  m_cRef = 0;
  m_enumFE->Release();

  // Free arbitrary system formats
  for (PRUint32 idx = 0; idx < mDataEntryList.Length(); idx++) {
      CoTaskMemFree(mDataEntryList[idx]->fe.ptd);
      ReleaseStgMedium(&mDataEntryList[idx]->stgm);
      CoTaskMemFree(mDataEntryList[idx]);
  }
}


//-----------------------------------------------------
// IUnknown interface methods - see inknown.h for documentation
//-----------------------------------------------------
STDMETHODIMP nsDataObj::QueryInterface(REFIID riid, void** ppv)
{
	*ppv=NULL;

	if ( (IID_IUnknown == riid) || (IID_IDataObject	== riid) ) {
		*ppv = this;
		AddRef();
		return S_OK;
  } else if (IID_IAsyncOperation == riid) {
    *ppv = static_cast<IAsyncOperation*>(this);
    AddRef();
    return S_OK;
  }

	return ResultFromScode(E_NOINTERFACE);
}

//-----------------------------------------------------
STDMETHODIMP_(ULONG) nsDataObj::AddRef()
{
	++g_cRef;
	++m_cRef;
	NS_LOG_ADDREF(this, m_cRef, "nsDataObj", sizeof(*this));
  //PRNTDEBUG3("nsDataObj::AddRef  >>>>>>>>>>>>>>>>>> %d on %p\n", (m_cRef+1), this);
	return m_cRef;
}


//-----------------------------------------------------
STDMETHODIMP_(ULONG) nsDataObj::Release()
{
  //PRNTDEBUG3("nsDataObj::Release >>>>>>>>>>>>>>>>>> %d on %p\n", (m_cRef-1), this);
	if (0 < g_cRef)
		--g_cRef;

	--m_cRef;
	NS_LOG_RELEASE(this, m_cRef, "nsDataObj");
	if (0 != m_cRef)
		return m_cRef;

	delete this;

	return 0;
}

//-----------------------------------------------------
BOOL nsDataObj::FormatsMatch(const FORMATETC& source, const FORMATETC& target) const
{
	if ((source.cfFormat == target.cfFormat) &&
		 (source.dwAspect  & target.dwAspect)  &&
		 (source.tymed     & target.tymed))       {
		return TRUE;
	} else {
		return FALSE;
	}
}

//-----------------------------------------------------
// IDataObject methods
//-----------------------------------------------------
STDMETHODIMP nsDataObj::GetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
  PRNTDEBUG("nsDataObj::GetData\n");
  PRNTDEBUG3("  format: %d  Text: %d\n", pFE->cfFormat, CF_TEXT);
  if ( !mTransferable )
	  return ResultFromScode(DATA_E_FORMATETC);

  PRUint32 dfInx = 0;

  static CLIPFORMAT fileDescriptorFlavorA = ::RegisterClipboardFormat( CFSTR_FILEDESCRIPTORA ); 
  static CLIPFORMAT fileDescriptorFlavorW = ::RegisterClipboardFormat( CFSTR_FILEDESCRIPTORW ); 
  static CLIPFORMAT uniformResourceLocatorA = ::RegisterClipboardFormat( CFSTR_INETURLA );
  static CLIPFORMAT uniformResourceLocatorW = ::RegisterClipboardFormat( CFSTR_INETURLW );
#ifndef WINCE
  static CLIPFORMAT fileFlavor = ::RegisterClipboardFormat( CFSTR_FILECONTENTS ); 
  static CLIPFORMAT PreferredDropEffect = ::RegisterClipboardFormat( CFSTR_PREFERREDDROPEFFECT );
#endif

  // Arbitrary system formats
  LPDATAENTRY pde;
  HRESULT hres = FindFORMATETC(pFE, &pde, FALSE);
  if (SUCCEEDED(hres)) {
      return AddRefStgMedium(&pde->stgm, pSTM, FALSE);
  }

  // Firefox internal formats
  ULONG count;
  FORMATETC fe;
  m_enumFE->Reset();
  while (NOERROR == m_enumFE->Next(1, &fe, &count)) {
    nsCAutoString * df = reinterpret_cast<nsCAutoString*>(mDataFlavors->SafeElementAt(dfInx));
    if ( df ) {
      if (FormatsMatch(fe, *pFE)) {
        pSTM->pUnkForRelease = NULL;        // caller is responsible for deleting this data
        CLIPFORMAT format = pFE->cfFormat;
        switch(format) {

        // Someone is asking for plain or unicode text
        case CF_TEXT:
        case CF_UNICODETEXT:
        return GetText(*df, *pFE, *pSTM);

        // Someone is asking for an image
        case CF_DIB:
          return GetDib(*df, *pFE, *pSTM);
                                              
        // ... not yet implemented ...
        //case CF_BITMAP:
        //  return GetBitmap(*pFE, *pSTM);
        //case CF_METAFILEPICT:
        //  return GetMetafilePict(*pFE, *pSTM);
            
        default:
          if ( format == fileDescriptorFlavorA )
            return GetFileDescriptor ( *pFE, *pSTM, PR_FALSE );
          if ( format == fileDescriptorFlavorW )
            return GetFileDescriptor ( *pFE, *pSTM, PR_TRUE);
          if ( format == uniformResourceLocatorA )
            return GetUniformResourceLocator( *pFE, *pSTM, PR_FALSE);
          if ( format == uniformResourceLocatorW )
            return GetUniformResourceLocator( *pFE, *pSTM, PR_TRUE);
#ifndef WINCE
          if ( format == fileFlavor )
            return GetFileContents ( *pFE, *pSTM );
          if ( format == PreferredDropEffect )
            return GetPreferredDropEffect( *pFE, *pSTM );
#endif
          PRNTDEBUG2("***** nsDataObj::GetData - Unknown format %u\n", format);
          return GetText(*df, *pFE, *pSTM);
        } //switch
      } // if
    }
    dfInx++;
  } // while

  return ResultFromScode(DATA_E_FORMATETC);
}

//-----------------------------------------------------
STDMETHODIMP nsDataObj::GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM)
{
  PRNTDEBUG("nsDataObj::GetDataHere\n");
		return ResultFromScode(E_FAIL);
}


//-----------------------------------------------------
// Other objects querying to see if we support a 
// particular format
//-----------------------------------------------------
STDMETHODIMP nsDataObj::QueryGetData(LPFORMATETC pFE)
{
  PRNTDEBUG("nsDataObj::QueryGetData  ");
  PRNTDEBUG3("format: %d  Text: %d\n", pFE->cfFormat, CF_TEXT);

  // Arbitrary system formats
  LPDATAENTRY pde;
  if (SUCCEEDED(FindFORMATETC(pFE, &pde, FALSE)))
    return S_OK;

  // Firefox internal formats
  ULONG count;
  FORMATETC fe;
  m_enumFE->Reset();
  while (NOERROR == m_enumFE->Next(1, &fe, &count)) {
    if (fe.cfFormat == pFE->cfFormat) {
      return S_OK;
    }
  }
  
  PRNTDEBUG2("***** nsDataObj::QueryGetData - Unknown format %d\n", pFE->cfFormat);
	return ResultFromScode(E_FAIL);
}

//-----------------------------------------------------
STDMETHODIMP nsDataObj::GetCanonicalFormatEtc
	 (LPFORMATETC pFEIn, LPFORMATETC pFEOut)
{
  PRNTDEBUG("nsDataObj::GetCanonicalFormatEtc\n");
		return ResultFromScode(E_FAIL);
}

HGLOBAL nsDataObj::GlobalClone(HGLOBAL hglobIn)
{
  HGLOBAL hglobOut = NULL;

  LPVOID pvIn = GlobalLock(hglobIn);
  if (pvIn) {
    SIZE_T cb = GlobalSize(hglobIn);
    HGLOBAL hglobOut = GlobalAlloc(GMEM_FIXED, cb);
    if (hglobOut) {
      CopyMemory(hglobOut, pvIn, cb);
    }
    GlobalUnlock(hglobIn);
  }
  return hglobOut;
}

IUnknown* nsDataObj::GetCanonicalIUnknown(IUnknown *punk)
{
  IUnknown *punkCanonical;
  if (punk && SUCCEEDED(punk->QueryInterface(IID_IUnknown,
                                             (LPVOID*)&punkCanonical))) {
    punkCanonical->Release();
  } else {
    punkCanonical = punk;
  }
  return punkCanonical;
}

//-----------------------------------------------------
STDMETHODIMP nsDataObj::SetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease)
{
  PRNTDEBUG("nsDataObj::SetData\n");
#ifndef WINCE
  static CLIPFORMAT PerformedDropEffect = ::RegisterClipboardFormat( CFSTR_PERFORMEDDROPEFFECT );  

  if (pFE && pFE->cfFormat == PerformedDropEffect) {
    // The drop operation has completed.  Delete the temp file if it exists.
    if (mCachedTempFile) {
      mCachedTempFile->Remove(PR_FALSE);
      mCachedTempFile = NULL;
    }
  }
#endif

  // Store arbitrary system formats
  LPDATAENTRY pde;
  HRESULT hres = FindFORMATETC(pFE, &pde, TRUE); // add
  if (SUCCEEDED(hres)) {
    if (pde->stgm.tymed) {
      ReleaseStgMedium(&pde->stgm);
      ZeroMemory(&pde->stgm, sizeof(STGMEDIUM));
    }

    if (fRelease) {
      pde->stgm = *pSTM;
      hres = S_OK;
    } else {
      hres = AddRefStgMedium(pSTM, &pde->stgm, TRUE);
    }
    pde->fe.tymed = pde->stgm.tymed;

    // Break circular reference loop (see msdn)
    if (GetCanonicalIUnknown(pde->stgm.pUnkForRelease) ==
        GetCanonicalIUnknown(static_cast<IDataObject*>(this))) {
      pde->stgm.pUnkForRelease->Release();
      pde->stgm.pUnkForRelease = NULL;
    }
    return hres;
  }

  if (fRelease)
    ReleaseStgMedium(pSTM);

  return ResultFromScode(S_OK);
}

HRESULT
nsDataObj::FindFORMATETC(FORMATETC *pfe, LPDATAENTRY *ppde, BOOL fAdd)
{
  *ppde = NULL;

  if (pfe->ptd != NULL) return DV_E_DVTARGETDEVICE;

  // See if it's in our list
  for (PRUint32 idx = 0; idx < mDataEntryList.Length(); idx++) {
    if (mDataEntryList[idx]->fe.cfFormat == pfe->cfFormat &&
        mDataEntryList[idx]->fe.dwAspect == pfe->dwAspect &&
        mDataEntryList[idx]->fe.lindex == pfe->lindex) {
      if (fAdd || (mDataEntryList[idx]->fe.tymed & pfe->tymed)) {
        *ppde = mDataEntryList[idx];
        return S_OK;
      } else {
        return DV_E_TYMED;
      }
    }
  }

  if (!fAdd)
    return DV_E_FORMATETC;

  LPDATAENTRY pde = (LPDATAENTRY)CoTaskMemAlloc(sizeof(DATAENTRY));
  if (pde) {
    pde->fe = *pfe;
    *ppde = pde;
    ZeroMemory(&pde->stgm, sizeof(STGMEDIUM));

    m_enumFE->AddFE(pfe);
    mDataEntryList.AppendElement(pde);

    return S_OK;
  } else {
    return E_OUTOFMEMORY;
  }
}

HRESULT
nsDataObj::AddRefStgMedium(STGMEDIUM *pstgmIn, STGMEDIUM *pstgmOut, BOOL fCopyIn)
{
  HRESULT hres = S_OK;
  STGMEDIUM stgmOut = *pstgmIn;

  if (pstgmIn->pUnkForRelease == NULL &&
      !(pstgmIn->tymed & (TYMED_ISTREAM | TYMED_ISTORAGE))) {
    if (fCopyIn) {
      // Object needs to be cloned
      if (pstgmIn->tymed == TYMED_HGLOBAL) {
        stgmOut.hGlobal = GlobalClone(pstgmIn->hGlobal);
        if (!stgmOut.hGlobal) {
          hres = E_OUTOFMEMORY;
        }
      } else {
        hres = DV_E_TYMED;
      }
    } else {
      stgmOut.pUnkForRelease = static_cast<IDataObject*>(this);
    }
  }

  if (SUCCEEDED(hres)) {
    switch (stgmOut.tymed) {
    case TYMED_ISTREAM:
      stgmOut.pstm->AddRef();
      break;
    case TYMED_ISTORAGE:
      stgmOut.pstg->AddRef();
      break;
    }
    if (stgmOut.pUnkForRelease) {
      stgmOut.pUnkForRelease->AddRef();
    }
    *pstgmOut = stgmOut;
  }

  return hres;
}

//-----------------------------------------------------
STDMETHODIMP nsDataObj::EnumFormatEtc(DWORD dwDir, LPENUMFORMATETC *ppEnum)
{
  PRNTDEBUG("nsDataObj::EnumFormatEtc\n");

  switch (dwDir) {
    case DATADIR_GET: {
       m_enumFE->Clone(ppEnum);
    } break;
    case DATADIR_SET:
        *ppEnum=NULL;
        break;
    default:
        *ppEnum=NULL;
        break;
  } // switch

  if (NULL == *ppEnum)
    return ResultFromScode(E_FAIL);

  // Clone already AddRefed the result so don't addref it again.
  return NOERROR;
}

//-----------------------------------------------------
STDMETHODIMP nsDataObj::DAdvise(LPFORMATETC pFE, DWORD dwFlags,
										            LPADVISESINK pIAdviseSink, DWORD* pdwConn)
{
  PRNTDEBUG("nsDataObj::DAdvise\n");
	return ResultFromScode(E_FAIL);
}


//-----------------------------------------------------
STDMETHODIMP nsDataObj::DUnadvise(DWORD dwConn)
{
  PRNTDEBUG("nsDataObj::DUnadvise\n");
	return ResultFromScode(E_FAIL);
}

//-----------------------------------------------------
STDMETHODIMP nsDataObj::EnumDAdvise(LPENUMSTATDATA *ppEnum)
{
  PRNTDEBUG("nsDataObj::EnumDAdvise\n");
	return ResultFromScode(E_FAIL);
}

// IAsyncOperation methods
STDMETHODIMP nsDataObj::EndOperation(HRESULT hResult,
                                     IBindCtx *pbcReserved,
                                     DWORD dwEffects)
{
  mIsInOperation = FALSE;
  Release();
  return S_OK;
}

STDMETHODIMP nsDataObj::GetAsyncMode(BOOL *pfIsOpAsync)
{
  if (!pfIsOpAsync)
    return E_FAIL;

  *pfIsOpAsync = mIsAsyncMode;

  return S_OK;
}

STDMETHODIMP nsDataObj::InOperation(BOOL *pfInAsyncOp)
{
  if (!pfInAsyncOp)
    return E_FAIL;

  *pfInAsyncOp = mIsInOperation;

  return S_OK;
}

STDMETHODIMP nsDataObj::SetAsyncMode(BOOL fDoOpAsync)
{
  mIsAsyncMode = fDoOpAsync;
  return S_OK;
}

STDMETHODIMP nsDataObj::StartOperation(IBindCtx *pbcReserved)
{
  mIsInOperation = TRUE;
  return S_OK;
}

//-----------------------------------------------------
// other methods
//-----------------------------------------------------
ULONG nsDataObj::GetCumRefCount()
{
	return g_cRef;
}

//-----------------------------------------------------
ULONG nsDataObj::GetRefCount() const
{
	return m_cRef;
}

//-----------------------------------------------------
// GetData and SetData helper functions
//-----------------------------------------------------
HRESULT nsDataObj::AddSetFormat(FORMATETC& aFE)
{
  PRNTDEBUG("nsDataObj::AddSetFormat\n");
	return ResultFromScode(S_OK);
}

//-----------------------------------------------------
HRESULT nsDataObj::AddGetFormat(FORMATETC& aFE)
{
  PRNTDEBUG("nsDataObj::AddGetFormat\n");
	return ResultFromScode(S_OK);
}

//-----------------------------------------------------
HRESULT 
nsDataObj::GetBitmap ( const nsACString& , FORMATETC&, STGMEDIUM& )
{
  PRNTDEBUG("nsDataObj::GetBitmap\n");
	return ResultFromScode(E_NOTIMPL);
}


//
// GetDIB
//
// Someone is asking for a bitmap. The data in the transferable will be a straight
// nsIImage, so just QI it.
//
HRESULT 
nsDataObj :: GetDib ( const nsACString& inFlavor, FORMATETC &, STGMEDIUM & aSTG )
{
  PRNTDEBUG("nsDataObj::GetDib\n");
  ULONG result = E_FAIL;
#ifndef WINCE  
  
  PRUint32 len = 0;
  nsCOMPtr<nsISupports> genericDataWrapper;
  mTransferable->GetTransferData(PromiseFlatCString(inFlavor).get(), getter_AddRefs(genericDataWrapper), &len);
  nsCOMPtr<nsIImage> image ( do_QueryInterface(genericDataWrapper) );
  if ( !image ) {
    // In the 0.9.4 timeframe, I had some embedding clients put the nsIImage directly into the
    // transferable. Newer code, however, wraps the nsIImage in a nsISupportsInterfacePointer.
    // We should be backwards compatibile with code already out in the field. If we can't find
    // the image directly out of the transferable,  unwrap the image from its wrapper.
    nsCOMPtr<nsISupportsInterfacePointer> ptr(do_QueryInterface(genericDataWrapper));
    if ( ptr )
      ptr->GetData(getter_AddRefs(image));
  }
  
  if ( image ) {
    // use a the helper class to build up a bitmap. We now own the bits,
    // and pass them back to the OS in |aSTG|.
    nsImageToClipboard converter ( image );
    HANDLE bits = nsnull;
    nsresult rv = converter.GetPicture ( &bits );
    if ( NS_SUCCEEDED(rv) && bits ) {
      aSTG.hGlobal = bits;
      aSTG.tymed = TYMED_HGLOBAL;
      result = S_OK;
    }
  } // if we have an image
  else  
    NS_WARNING ( "Definitely not an image on clipboard" );

#endif
	return ResultFromScode(result);
}



//
// GetFileDescriptor
//

HRESULT 
nsDataObj :: GetFileDescriptor ( FORMATETC& aFE, STGMEDIUM& aSTG, PRBool aIsUnicode )
{
  HRESULT res = S_OK;
  
  // How we handle this depends on if we're dealing with an internet
  // shortcut, since those are done under the covers.
  if (IsFlavourPresent(kFilePromiseMime) ||
      IsFlavourPresent(kFileMime))
  {
    if (aIsUnicode)
      return GetFileDescriptor_IStreamW(aFE, aSTG);
    else
      return GetFileDescriptor_IStreamA(aFE, aSTG);
  }
  else if (IsFlavourPresent(kURLMime))
  {
    if ( aIsUnicode )
      res = GetFileDescriptorInternetShortcutW ( aFE, aSTG );
    else
      res = GetFileDescriptorInternetShortcutA ( aFE, aSTG );
  }
  else
    NS_WARNING ( "Not yet implemented\n" );
  
	return res;
	
} // GetFileDescriptor


//
HRESULT 
nsDataObj :: GetFileContents ( FORMATETC& aFE, STGMEDIUM& aSTG )
{
  HRESULT res = S_OK;
  
  // How we handle this depends on if we're dealing with an internet
  // shortcut, since those are done under the covers.
  if (IsFlavourPresent(kFilePromiseMime) ||
      IsFlavourPresent(kFileMime))
    return GetFileContents_IStream(aFE, aSTG);
  else if (IsFlavourPresent(kURLMime))
    return GetFileContentsInternetShortcut ( aFE, aSTG );
  else
    NS_WARNING ( "Not yet implemented\n" );

	return res;
	
} // GetFileContents

// 
// Given a unicode string, we ensure that it contains only characters which are valid within
// the file system. Remove all forbidden characters from the name, and completely disallow 
// any title that starts with a forbidden name and extension (e.g. "nul" is invalid, but 
// "nul." and "nul.txt" are also invalid and will cause problems).
//
// It would seem that this is more functionality suited to being in nsILocalFile.
//
static void
MangleTextToValidFilename(nsString & aText)
{
  static const char* forbiddenNames[] = {
    "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", 
    "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
    "CON", "PRN", "AUX", "NUL", "CLOCK$"
  };

  aText.StripChars(FILE_PATH_SEPARATOR  FILE_ILLEGAL_CHARACTERS);
  aText.CompressWhitespace(PR_TRUE, PR_TRUE);
  PRUint32 nameLen;
  for (size_t n = 0; n < NS_ARRAY_LENGTH(forbiddenNames); ++n) {
    nameLen = (PRUint32) strlen(forbiddenNames[n]);
    if (aText.EqualsIgnoreCase(forbiddenNames[n], nameLen)) {
      // invalid name is either the entire string, or a prefix with a period
      if (aText.Length() == nameLen || aText.CharAt(nameLen) == PRUnichar('.')) {
        aText.Truncate();
        break;
      }
    }
  }
}

// 
// Given a unicode string, convert it down to a valid local charset filename
// with the supplied extension. This ensures that we do not cut MBCS characters
// in the middle.
//
// It would seem that this is more functionality suited to being in nsILocalFile.
//
static PRBool
CreateFilenameFromTextA(nsString & aText, const char * aExtension, 
                         char * aFilename, PRUint32 aFilenameLen)
{
  // ensure that the supplied name doesn't have invalid characters. If 
  // a valid mangled filename couldn't be created then it will leave the
  // text empty.
  MangleTextToValidFilename(aText);
  if (aText.IsEmpty())
    return PR_FALSE;

  // repeatably call WideCharToMultiByte as long as the title doesn't fit in the buffer 
  // available to us. Continually reduce the length of the source title until the MBCS
  // version will fit in the buffer with room for the supplied extension. Doing it this
  // way ensures that even in MBCS environments there will be a valid MBCS filename of
  // the correct length.
  int maxUsableFilenameLen = aFilenameLen - strlen(aExtension) - 1; // space for ext + null byte
  int currLen, textLen = (int) NS_MIN(aText.Length(), aFilenameLen);
  char defaultChar = '_';
  do {
    currLen = WideCharToMultiByte(CP_ACP, 
      WC_COMPOSITECHECK|WC_DEFAULTCHAR,
      aText.get(), textLen--, aFilename, maxUsableFilenameLen, &defaultChar, NULL);
  }
  while (currLen == 0 && textLen > 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER);
  if (currLen > 0 && textLen > 0) {
    strcpy(&aFilename[currLen], aExtension);
    return PR_TRUE;
  }
  else {
    // empty names aren't permitted
    return PR_FALSE;
  }
}

static PRBool
CreateFilenameFromTextW(nsString & aText, const wchar_t * aExtension, 
                         wchar_t * aFilename, PRUint32 aFilenameLen)
{
  // ensure that the supplied name doesn't have invalid characters. If 
  // a valid mangled filename couldn't be created then it will leave the
  // text empty.
  MangleTextToValidFilename(aText);
  if (aText.IsEmpty())
    return PR_FALSE;

  const int extensionLen = wcslen(aExtension);
  if (aText.Length() + extensionLen + 1 > aFilenameLen)
    aText.Truncate(aFilenameLen - extensionLen - 1);
  wcscpy(&aFilename[0], aText.get());
  wcscpy(&aFilename[aText.Length()], aExtension);
  return PR_TRUE;
}

#define PAGEINFO_PROPERTIES "chrome://navigator/locale/pageInfo.properties"

static PRBool
GetLocalizedString(const PRUnichar * aName, nsXPIDLString & aString)
{
  nsresult rv;
  nsCOMPtr<nsIStringBundleService> stringService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
  if (NS_FAILED(rv)) 
    return PR_FALSE;

  nsCOMPtr<nsIStringBundle> stringBundle;
  rv = stringService->CreateBundle(PAGEINFO_PROPERTIES, getter_AddRefs(stringBundle));
  if (NS_FAILED(rv))
    return PR_FALSE;

  rv = stringBundle->GetStringFromName(aName, getter_Copies(aString));
  return NS_SUCCEEDED(rv);
}

//
// GetFileDescriptorInternetShortcut
//
// Create the special format for an internet shortcut and build up the data
// structures the shell is expecting.
//
HRESULT
nsDataObj :: GetFileDescriptorInternetShortcutA ( FORMATETC& aFE, STGMEDIUM& aSTG )
{
#ifdef WINCE
  return E_FAIL;
#else
  // get the title of the shortcut
  nsAutoString title;
  if ( NS_FAILED(ExtractShortcutTitle(title)) )
    return E_OUTOFMEMORY;

  HGLOBAL fileGroupDescHandle = ::GlobalAlloc(GMEM_ZEROINIT|GMEM_SHARE,sizeof(FILEGROUPDESCRIPTORA));
  if (!fileGroupDescHandle)
    return E_OUTOFMEMORY;

  LPFILEGROUPDESCRIPTORA fileGroupDescA = reinterpret_cast<LPFILEGROUPDESCRIPTORA>(::GlobalLock(fileGroupDescHandle));
  if (!fileGroupDescA) {
    ::GlobalFree(fileGroupDescHandle);
    return E_OUTOFMEMORY;
  }

  // get a valid filename in the following order: 1) from the page title, 
  // 2) localized string for an untitled page, 3) just use "Untitled.URL"
  if (!CreateFilenameFromTextA(title, ".URL", 
                               fileGroupDescA->fgd[0].cFileName, NS_MAX_FILEDESCRIPTOR)) {
    nsXPIDLString untitled;
    if (!GetLocalizedString(NS_LITERAL_STRING("noPageTitle").get(), untitled) ||
        !CreateFilenameFromTextA(untitled, ".URL", 
                                 fileGroupDescA->fgd[0].cFileName, NS_MAX_FILEDESCRIPTOR)) {
      strcpy(fileGroupDescA->fgd[0].cFileName, "Untitled.URL");
    }
  }

  // one file in the file block
  fileGroupDescA->cItems = 1;
  fileGroupDescA->fgd[0].dwFlags = FD_LINKUI;

  ::GlobalUnlock( fileGroupDescHandle );
  aSTG.hGlobal = fileGroupDescHandle;
  aSTG.tymed = TYMED_HGLOBAL;

  return S_OK;
#endif
} // GetFileDescriptorInternetShortcutA

HRESULT
nsDataObj :: GetFileDescriptorInternetShortcutW ( FORMATETC& aFE, STGMEDIUM& aSTG )
{
#ifdef WINCE
  return E_FAIL;
#else
  // get the title of the shortcut
  nsAutoString title;
  if ( NS_FAILED(ExtractShortcutTitle(title)) )
    return E_OUTOFMEMORY;

  HGLOBAL fileGroupDescHandle = ::GlobalAlloc(GMEM_ZEROINIT|GMEM_SHARE,sizeof(FILEGROUPDESCRIPTORW));
  if (!fileGroupDescHandle)
    return E_OUTOFMEMORY;

  LPFILEGROUPDESCRIPTORW fileGroupDescW = reinterpret_cast<LPFILEGROUPDESCRIPTORW>(::GlobalLock(fileGroupDescHandle));
  if (!fileGroupDescW) {
    ::GlobalFree(fileGroupDescHandle);
    return E_OUTOFMEMORY;
  }

  // get a valid filename in the following order: 1) from the page title, 
  // 2) localized string for an untitled page, 3) just use "Untitled.URL"
  if (!CreateFilenameFromTextW(title, NS_LITERAL_STRING(".URL").get(), 
                               fileGroupDescW->fgd[0].cFileName, NS_MAX_FILEDESCRIPTOR)) {
    nsXPIDLString untitled;
    if (!GetLocalizedString(NS_LITERAL_STRING("noPageTitle").get(), untitled) ||
        !CreateFilenameFromTextW(untitled, NS_LITERAL_STRING(".URL").get(), 
                                 fileGroupDescW->fgd[0].cFileName, NS_MAX_FILEDESCRIPTOR)) {
      wcscpy(fileGroupDescW->fgd[0].cFileName, NS_LITERAL_STRING("Untitled.URL").get());
    }
  }

  // one file in the file block
  fileGroupDescW->cItems = 1;
  fileGroupDescW->fgd[0].dwFlags = FD_LINKUI;

  ::GlobalUnlock( fileGroupDescHandle );
  aSTG.hGlobal = fileGroupDescHandle;
  aSTG.tymed = TYMED_HGLOBAL;

  return S_OK;
#endif
} // GetFileDescriptorInternetShortcutW


//
// GetFileContentsInternetShortcut
//
// Create the special format for an internet shortcut and build up the data
// structures the shell is expecting.
//
HRESULT
nsDataObj :: GetFileContentsInternetShortcut ( FORMATETC& aFE, STGMEDIUM& aSTG )
{
#ifdef WINCE
  return E_FAIL;
#else
  nsAutoString url;
  if ( NS_FAILED(ExtractShortcutURL(url)) )
    return E_OUTOFMEMORY;

  // will need to change if we ever support iDNS
  nsCAutoString asciiUrl;
  LossyCopyUTF16toASCII(url, asciiUrl);
    
  static const char* shortcutFormatStr = "[InternetShortcut]\r\nURL=%s\r\n";
  static const int formatLen = strlen(shortcutFormatStr) - 2; // don't include %s in the len
  const int totalLen = formatLen + asciiUrl.Length(); // we don't want a null character on the end

  // create a global memory area and build up the file contents w/in it
  HGLOBAL hGlobalMemory = ::GlobalAlloc(GMEM_SHARE, totalLen);
  if ( !hGlobalMemory )
    return E_OUTOFMEMORY;

  char* contents = reinterpret_cast<char*>(::GlobalLock(hGlobalMemory));
  if ( !contents ) {
    ::GlobalFree( hGlobalMemory );
    return E_OUTOFMEMORY;
  }
    
  //NOTE: we intentionally use the Microsoft version of snprintf here because it does NOT null 
  // terminate strings which reach the maximum size of the buffer. Since we know that the 
  // formatted length here is totalLen, this call to _snprintf will format the string into 
  // the buffer without appending the null character.
  _snprintf( contents, totalLen, shortcutFormatStr, asciiUrl.get() );
    
  ::GlobalUnlock(hGlobalMemory);
  aSTG.hGlobal = hGlobalMemory;
  aSTG.tymed = TYMED_HGLOBAL;

  return S_OK;
#endif  
} // GetFileContentsInternetShortcut

// check if specified flavour is present in the transferable
PRBool nsDataObj :: IsFlavourPresent(const char *inFlavour)
{
  PRBool retval = PR_FALSE;
  NS_ENSURE_TRUE(mTransferable, PR_FALSE);
  
  // get the list of flavors available in the transferable
  nsCOMPtr<nsISupportsArray> flavorList;
  mTransferable->FlavorsTransferableCanExport(getter_AddRefs(flavorList));
  NS_ENSURE_TRUE(flavorList, PR_FALSE);

  // try to find requested flavour
  PRUint32 cnt;
  flavorList->Count(&cnt);
  for (PRUint32 i = 0; i < cnt; ++i) {
    nsCOMPtr<nsISupports> genericFlavor;
    flavorList->GetElementAt (i, getter_AddRefs(genericFlavor));
    nsCOMPtr<nsISupportsCString> currentFlavor (do_QueryInterface(genericFlavor));
    if (currentFlavor) {
      nsCAutoString flavorStr;
      currentFlavor->GetData(flavorStr);
      if (flavorStr.Equals(inFlavour)) {
        retval = PR_TRUE;         // found it!
        break;
      }
    }
  } // for each flavor

  return retval;
}

HRESULT nsDataObj::GetPreferredDropEffect ( FORMATETC& aFE, STGMEDIUM& aSTG )
{
  HRESULT res = S_OK;
  aSTG.tymed = TYMED_HGLOBAL;
  aSTG.pUnkForRelease = NULL;    
  HGLOBAL hGlobalMemory = NULL;
  hGlobalMemory = ::GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
  if (hGlobalMemory) {
    DWORD* pdw = (DWORD*) GlobalLock(hGlobalMemory);
    // The PreferredDropEffect clipboard format is only registered if a drag/drop
    // of an image happens from Mozilla to the desktop.  We want its value
    // to be DROPEFFECT_MOVE in that case so that the file is moved from the
    // temporary location, not copied.
    // This value should, ideally, be set on the data object via SetData() but 
    // our IDataObject implementation doesn't implement SetData.  It adds data
    // to the data object lazily only when the drop target asks for it.
    *pdw = (DWORD) DROPEFFECT_MOVE;
    GlobalUnlock(hGlobalMemory);
  }
  else {
    res = E_OUTOFMEMORY;
  }
  aSTG.hGlobal = hGlobalMemory;
  return res;
}

//-----------------------------------------------------
HRESULT nsDataObj::GetText(const nsACString & aDataFlavor, FORMATETC& aFE, STGMEDIUM& aSTG)
{
  void* data = NULL;
  PRUint32   len;
  
  // if someone asks for text/plain, look up text/unicode instead in the transferable.
  const char* flavorStr;
  const nsPromiseFlatCString& flat = PromiseFlatCString(aDataFlavor);
  if ( aDataFlavor.Equals("text/plain") )
    flavorStr = kUnicodeMime;
  else
    flavorStr = flat.get();

  // NOTE: CreateDataFromPrimitive creates new memory, that needs to be deleted
  nsCOMPtr<nsISupports> genericDataWrapper;
  mTransferable->GetTransferData(flavorStr, getter_AddRefs(genericDataWrapper), &len);
  if ( !len )
    return ResultFromScode(E_FAIL);
  nsPrimitiveHelpers::CreateDataFromPrimitive ( flavorStr, genericDataWrapper, &data, len );
  if ( !data )
    return ResultFromScode(E_FAIL);

  HGLOBAL     hGlobalMemory = NULL;

  aSTG.tymed          = TYMED_HGLOBAL;
  aSTG.pUnkForRelease = NULL;

  // We play games under the hood and advertise flavors that we know we
  // can support, only they require a bit of conversion or munging of the data.
  // Do that here.
  //
  // The transferable gives us data that is null-terminated, but this isn't reflected in
  // the |len| parameter. Windoze apps expect this null to be there so bump our data buffer
  // by the appropriate size to account for the null (one char for CF_TEXT, one PRUnichar for
  // CF_UNICODETEXT).
  DWORD allocLen = (DWORD)len;
  if ( aFE.cfFormat == CF_TEXT ) {
    // Someone is asking for text/plain; convert the unicode (assuming it's present)
    // to text with the correct platform encoding.
    char* plainTextData = nsnull;
    PRUnichar* castedUnicode = reinterpret_cast<PRUnichar*>(data);
    PRInt32 plainTextLen = 0;
    nsPrimitiveHelpers::ConvertUnicodeToPlatformPlainText ( castedUnicode, len / 2, &plainTextData, &plainTextLen );
   
    // replace the unicode data with our plaintext data. Recall that |plainTextLen| doesn't include
    // the null in the length.
    nsMemory::Free(data);
    if ( plainTextData ) {
      data = plainTextData;
      allocLen = plainTextLen + sizeof(char);
    }
    else {
      NS_WARNING ( "Oh no, couldn't convert unicode to plain text" );
      return ResultFromScode(S_OK);
    }
  }
  else if ( aFE.cfFormat == nsClipboard::CF_HTML ) {
    // Someone is asking for win32's HTML flavor. Convert our html fragment
    // from unicode to UTF-8 then put it into a format specified by msft.
    NS_ConvertUTF16toUTF8 converter ( reinterpret_cast<PRUnichar*>(data) );
    char* utf8HTML = nsnull;
    nsresult rv = BuildPlatformHTML ( converter.get(), &utf8HTML );      // null terminates
    
    nsMemory::Free(data);
    if ( NS_SUCCEEDED(rv) && utf8HTML ) {
      // replace the unicode data with our HTML data. Don't forget the null.
      data = utf8HTML;
      allocLen = strlen(utf8HTML) + sizeof(char);
    }
    else {
      NS_WARNING ( "Oh no, couldn't convert to HTML" );
      return ResultFromScode(S_OK);
    }
  }
  else {
    // we assume that any data that isn't caught above is unicode. This may
    // be an erroneous assumption, but is true so far.
    allocLen += sizeof(PRUnichar);
  }

  hGlobalMemory = (HGLOBAL)GlobalAlloc(GMEM_MOVEABLE, allocLen);

  // Copy text to Global Memory Area
  if ( hGlobalMemory ) {
    char* dest = reinterpret_cast<char*>(GlobalLock(hGlobalMemory));
    char* source = reinterpret_cast<char*>(data);
    memcpy ( dest, source, allocLen );                         // copies the null as well
    GlobalUnlock(hGlobalMemory);
  }
  aSTG.hGlobal = hGlobalMemory;

  // Now, delete the memory that was created by CreateDataFromPrimitive (or our text/plain data)
  nsMemory::Free(data);

  return ResultFromScode(S_OK);
}

//-----------------------------------------------------
HRESULT nsDataObj::GetMetafilePict(FORMATETC&, STGMEDIUM&)
{
	return ResultFromScode(E_NOTIMPL);
}

//-----------------------------------------------------
HRESULT nsDataObj::SetBitmap(FORMATETC&, STGMEDIUM&)
{
	return ResultFromScode(E_NOTIMPL);
}

//-----------------------------------------------------
HRESULT nsDataObj::SetDib(FORMATETC&, STGMEDIUM&)
{
	return ResultFromScode(E_FAIL);
}

//-----------------------------------------------------
HRESULT nsDataObj::SetText  (FORMATETC& aFE, STGMEDIUM& aSTG)
{
  if (aFE.cfFormat == CF_TEXT && aFE.tymed ==  TYMED_HGLOBAL) {
		HGLOBAL hString = (HGLOBAL)aSTG.hGlobal;
		if(hString == NULL)
			return(FALSE);

		// get a pointer to the actual bytes
		char *  pString = (char *) GlobalLock(hString);    
		if(!pString)
			return(FALSE);

		GlobalUnlock(hString);
    nsAutoString str; str.AssignWithConversion(pString);

  }
	return ResultFromScode(E_FAIL);
}

//-----------------------------------------------------
HRESULT nsDataObj::SetMetafilePict(FORMATETC&, STGMEDIUM&)
{
	return ResultFromScode(E_FAIL);
}



//-----------------------------------------------------
//-----------------------------------------------------
CLSID nsDataObj::GetClassID() const
{
	return CLSID_nsDataObj;
}

//-----------------------------------------------------
// Registers a the DataFlavor/FE pair
//-----------------------------------------------------
void nsDataObj::AddDataFlavor(const char* aDataFlavor, LPFORMATETC aFE)
{
  // These two lists are the mapping to and from data flavors and FEs
  // Later, OLE will tell us it's needs a certain type of FORMATETC (text, unicode, etc)
  // so we will look up data flavor that corresponds to the FE
  // and then ask the transferable for that type of data

  // Just ignore the CF_HDROP here
  // all file drags are now hangled by CFSTR_FileContents format
#ifndef WINCE
  if (aFE->cfFormat == CF_HDROP) {
    return;
  }  
  else 
#endif
  {
    mDataFlavors->AppendElement(new nsCAutoString(aDataFlavor));
    m_enumFE->AddFE(aFE);
  }
}

//-----------------------------------------------------
// Sets the transferable object
//-----------------------------------------------------
void nsDataObj::SetTransferable(nsITransferable * aTransferable)
{
    NS_IF_RELEASE(mTransferable);

  mTransferable = aTransferable;
  if (nsnull == mTransferable) {
    return;
  }

  NS_ADDREF(mTransferable);

  return;
}


//
// ExtractURL
//
// Roots around in the transferable for the appropriate flavor that indicates
// a url and pulls out the url portion of the data. Used mostly for creating
// internet shortcuts on the desktop. The url flavor is of the format:
//
//   <url> <linefeed> <page title>
//
nsresult
nsDataObj :: ExtractShortcutURL ( nsString & outURL )
{
  NS_ASSERTION ( mTransferable, "We don't have a good transferable" );
  nsresult rv = NS_ERROR_FAILURE;
  
  PRUint32 len = 0;
  nsCOMPtr<nsISupports> genericURL;
  if ( NS_SUCCEEDED(mTransferable->GetTransferData(kURLMime, getter_AddRefs(genericURL), &len)) ) {
    nsCOMPtr<nsISupportsString> urlObject ( do_QueryInterface(genericURL) );
    if ( urlObject ) {
      nsAutoString url;
      urlObject->GetData ( url );
      outURL = url;

      // find the first linefeed in the data, that's where the url ends. trunc the 
      // result string at that point.
      PRInt32 lineIndex = outURL.FindChar ( '\n' );
      NS_ASSERTION ( lineIndex > 0, "Format for url flavor is <url> <linefeed> <page title>" );
      if ( lineIndex > 0 ) {
        outURL.Truncate ( lineIndex );
        rv = NS_OK;    
      }
    }
  } else if ( NS_SUCCEEDED(mTransferable->GetTransferData(kURLDataMime, getter_AddRefs(genericURL), &len)) ||
              NS_SUCCEEDED(mTransferable->GetTransferData(kURLPrivateMime, getter_AddRefs(genericURL), &len)) ) {
    nsCOMPtr<nsISupportsString> urlObject ( do_QueryInterface(genericURL) );
    if ( urlObject ) {
      nsAutoString url;
      urlObject->GetData ( url );
      outURL = url;

      rv = NS_OK;    
    }

  }  // if found flavor
  
  return rv;

} // ExtractShortcutURL


//
// ExtractShortcutTitle
//
// Roots around in the transferable for the appropriate flavor that indicates
// a url and pulls out the title portion of the data. Used mostly for creating
// internet shortcuts on the desktop. The url flavor is of the format:
//
//   <url> <linefeed> <page title>
//
nsresult
nsDataObj :: ExtractShortcutTitle ( nsString & outTitle )
{
  NS_ASSERTION ( mTransferable, "We'd don't have a good transferable" );
  nsresult rv = NS_ERROR_FAILURE;
  
  PRUint32 len = 0;
  nsCOMPtr<nsISupports> genericURL;
  if ( NS_SUCCEEDED(mTransferable->GetTransferData(kURLMime, getter_AddRefs(genericURL), &len)) ) {
    nsCOMPtr<nsISupportsString> urlObject ( do_QueryInterface(genericURL) );
    if ( urlObject ) {
      nsAutoString url;
      urlObject->GetData ( url );

      // find the first linefeed in the data, that's where the url ends. we want
      // everything after that linefeed. FindChar() returns -1 if we can't find
      PRInt32 lineIndex = url.FindChar ( '\n' );
      NS_ASSERTION ( lineIndex != -1, "Format for url flavor is <url> <linefeed> <page title>" );
      if ( lineIndex != -1 ) {
        url.Mid ( outTitle, lineIndex + 1, (len/2) - (lineIndex + 1) );
        rv = NS_OK;    
      }
    }
  } // if found flavor
  
  return rv;

} // ExtractShortcutTitle


//
// BuildPlatformHTML
//
// Munge our HTML data to win32's CF_HTML spec. Basically, put the requisite
// header information on it. This will null terminate |outPlatformHTML|. See
//  http://msdn.microsoft.com/workshop/networking/clipboard/htmlclipboard.asp
// for details.
//
// We assume that |inOurHTML| is already a fragment (ie, doesn't have <HTML>
// or <BODY> tags). We'll wrap the fragment with them to make other apps
// happy.
//
nsresult 
nsDataObj :: BuildPlatformHTML ( const char* inOurHTML, char** outPlatformHTML ) 
{
  *outPlatformHTML = nsnull;

  nsDependentCString inHTMLString(inOurHTML);
  const char* const numPlaceholder  = "00000000";
  const char* const startHTMLPrefix = "Version:0.9\r\nStartHTML:";
  const char* const endHTMLPrefix   = "\r\nEndHTML:";
  const char* const startFragPrefix = "\r\nStartFragment:";
  const char* const endFragPrefix   = "\r\nEndFragment:";
  const char* const startSourceURLPrefix = "\r\nSourceURL:";
  const char* const endFragTrailer  = "\r\n";

  // Do we already have mSourceURL from a drag?
  if (mSourceURL.IsEmpty()) {
    nsAutoString url;
    ExtractShortcutURL(url);

    AppendUTF16toUTF8(url, mSourceURL);
  }

  const PRInt32 kSourceURLLength    = mSourceURL.Length();
  const PRInt32 kNumberLength       = strlen(numPlaceholder);

  const PRInt32 kTotalHeaderLen     = strlen(startHTMLPrefix) +
                                      strlen(endHTMLPrefix) +
                                      strlen(startFragPrefix) + 
                                      strlen(endFragPrefix) + 
                                      strlen(endFragTrailer) +
                                      (kSourceURLLength > 0 ? strlen(startSourceURLPrefix) : 0) +
                                      kSourceURLLength +
                                      (4 * kNumberLength);

  NS_NAMED_LITERAL_CSTRING(htmlHeaderString, "<html><body>\r\n");

  NS_NAMED_LITERAL_CSTRING(fragmentHeaderString, "<!--StartFragment-->");

  nsDependentCString trailingString(
      "<!--EndFragment-->\r\n"
      "</body>\r\n"
      "</html>");

  // calculate the offsets
  PRInt32 startHTMLOffset = kTotalHeaderLen;
  PRInt32 startFragOffset = startHTMLOffset
                              + htmlHeaderString.Length()
			      + fragmentHeaderString.Length();

  PRInt32 endFragOffset   = startFragOffset
                              + inHTMLString.Length();

  PRInt32 endHTMLOffset   = endFragOffset
                              + trailingString.Length();

  // now build the final version
  nsCString clipboardString;
  clipboardString.SetCapacity(endHTMLOffset);

  clipboardString.Append(startHTMLPrefix);
  clipboardString.Append(nsPrintfCString("%08u", startHTMLOffset));

  clipboardString.Append(endHTMLPrefix);  
  clipboardString.Append(nsPrintfCString("%08u", endHTMLOffset));

  clipboardString.Append(startFragPrefix);
  clipboardString.Append(nsPrintfCString("%08u", startFragOffset));

  clipboardString.Append(endFragPrefix);
  clipboardString.Append(nsPrintfCString("%08u", endFragOffset));

  if (kSourceURLLength > 0) {
    clipboardString.Append(startSourceURLPrefix);
    clipboardString.Append(mSourceURL);
  }

  clipboardString.Append(endFragTrailer);

  clipboardString.Append(htmlHeaderString);
  clipboardString.Append(fragmentHeaderString);
  clipboardString.Append(inHTMLString);
  clipboardString.Append(trailingString);

  *outPlatformHTML = ToNewCString(clipboardString);
  if (!*outPlatformHTML)
    return NS_ERROR_OUT_OF_MEMORY;

  return NS_OK;
}

HRESULT 
nsDataObj :: GetUniformResourceLocator( FORMATETC& aFE, STGMEDIUM& aSTG, PRBool aIsUnicode )
{
  HRESULT res = S_OK;
  if (IsFlavourPresent(kURLMime)) {
    if ( aIsUnicode )
      res = ExtractUniformResourceLocatorW( aFE, aSTG );
    else
      res = ExtractUniformResourceLocatorA( aFE, aSTG );
  }
  else
    NS_WARNING ("Not yet implemented\n");
  return res;
}

HRESULT
nsDataObj::ExtractUniformResourceLocatorA(FORMATETC& aFE, STGMEDIUM& aSTG )
{
  HRESULT result = S_OK;

  nsAutoString url;
  if (NS_FAILED(ExtractShortcutURL(url)))
    return E_OUTOFMEMORY;

  NS_LossyConvertUTF16toASCII asciiUrl(url);
  const int totalLen = asciiUrl.Length() + 1;
  HGLOBAL hGlobalMemory = GlobalAlloc(GMEM_ZEROINIT|GMEM_SHARE, totalLen);
  if (!hGlobalMemory)
    return E_OUTOFMEMORY;

  char* contents = reinterpret_cast<char*>(GlobalLock(hGlobalMemory));
  if (!contents) {
    GlobalFree(hGlobalMemory);
    return E_OUTOFMEMORY;
  }

  strcpy(contents, asciiUrl.get());
  GlobalUnlock(hGlobalMemory);
  aSTG.hGlobal = hGlobalMemory;
  aSTG.tymed = TYMED_HGLOBAL;

  return result;
}

HRESULT
nsDataObj::ExtractUniformResourceLocatorW(FORMATETC& aFE, STGMEDIUM& aSTG )
{
  HRESULT result = S_OK;

  nsAutoString url;
  if (NS_FAILED(ExtractShortcutURL(url)))
    return E_OUTOFMEMORY;

  const int totalLen = (url.Length() + 1) * sizeof(PRUnichar);
  HGLOBAL hGlobalMemory = GlobalAlloc(GMEM_ZEROINIT|GMEM_SHARE, totalLen);
  if (!hGlobalMemory)
    return E_OUTOFMEMORY;

  wchar_t* contents = reinterpret_cast<wchar_t*>(GlobalLock(hGlobalMemory));
  if (!contents) {
    GlobalFree(hGlobalMemory);
    return E_OUTOFMEMORY;
  }

  wcscpy(contents, url.get());
  GlobalUnlock(hGlobalMemory);
  aSTG.hGlobal = hGlobalMemory;
  aSTG.tymed = TYMED_HGLOBAL;

  return result;
}


// Gets the filename from the kFilePromiseURLMime flavour
nsresult nsDataObj::GetDownloadDetails(nsIURI **aSourceURI,
                                       nsAString &aFilename)
{
  NS_ENSURE_TRUE(mTransferable, NS_ERROR_FAILURE);

  // get the URI from the kFilePromiseURLMime flavor
  nsCOMPtr<nsISupports> urlPrimitive;
  PRUint32 dataSize = 0;
  mTransferable->GetTransferData(kFilePromiseURLMime, getter_AddRefs(urlPrimitive), &dataSize);
  nsCOMPtr<nsISupportsString> srcUrlPrimitive = do_QueryInterface(urlPrimitive);
  NS_ENSURE_TRUE(srcUrlPrimitive, NS_ERROR_FAILURE);
  
  // Get data for flavor
  // The format of the data is (URLSTRING\nFILENAME)
  nsAutoString strData;
  srcUrlPrimitive->GetData(strData);
  if (strData.IsEmpty())
    return NS_ERROR_FAILURE;

  // Now figure if there is a "\n" delimiter in the data string.
  // If there is, the string after the "\n" is a filename.
  // If there is no delimiter then just get the filename from the url.
  nsCAutoString strFileName;
  nsCOMPtr<nsIURI> sourceURI;
  // New line char is used as a delimiter (hardcoded)
  PRInt32 nPos = strData.FindChar('\n');
  // Store source uri
  NS_NewURI(aSourceURI, Substring(strData, 0, nPos));
  if (nPos != -1) {
    // if there is delimiter
    CopyUTF16toUTF8(Substring(strData, nPos + 1, strData.Length()), strFileName);
  } else {
    // no filename was supplied - try to get it from a URL
    nsCOMPtr<nsIURL> sourceURL = do_QueryInterface(*aSourceURI);
    sourceURL->GetFileName(strFileName);
  }
  // check for an error; the URL must point to a file
  if (strFileName.IsEmpty())
    return NS_ERROR_FAILURE;

  NS_UnescapeURL(strFileName);
  NS_ConvertUTF8toUTF16 wideFileName(strFileName);

  // make the name safe for the filesystem
  MangleTextToValidFilename(wideFileName);

  aFilename = wideFileName;

  return NS_OK;
}

HRESULT nsDataObj::GetFileDescriptor_IStreamA(FORMATETC& aFE, STGMEDIUM& aSTG)
{
  HGLOBAL fileGroupDescHandle = ::GlobalAlloc(GMEM_ZEROINIT|GMEM_SHARE,sizeof(FILEGROUPDESCRIPTORW));
  NS_ENSURE_TRUE(fileGroupDescHandle, E_OUTOFMEMORY);

  LPFILEGROUPDESCRIPTORA fileGroupDescA = reinterpret_cast<LPFILEGROUPDESCRIPTORA>(GlobalLock(fileGroupDescHandle));
  if (!fileGroupDescA) {
    ::GlobalFree(fileGroupDescHandle);
    return E_OUTOFMEMORY;
  }

  nsAutoString wideFileName;
  nsresult rv;
  nsCOMPtr<nsIURI> sourceURI;
  rv = GetDownloadDetails(getter_AddRefs(sourceURI),
                          wideFileName);
  if (NS_FAILED(rv))
  {
    ::GlobalFree(fileGroupDescHandle);
    return E_FAIL;
  }

  nsCAutoString nativeFileName;
  NS_UTF16ToCString(wideFileName, NS_CSTRING_ENCODING_NATIVE_FILESYSTEM, nativeFileName);
  
  strncpy(fileGroupDescA->fgd[0].cFileName, nativeFileName.get(), NS_MAX_FILEDESCRIPTOR - 1);
  fileGroupDescA->fgd[0].cFileName[NS_MAX_FILEDESCRIPTOR - 1] = '\0';

  // one file in the file block
  fileGroupDescA->cItems = 1;
  fileGroupDescA->fgd[0].dwFlags = FD_PROGRESSUI;

  GlobalUnlock( fileGroupDescHandle );
  aSTG.hGlobal = fileGroupDescHandle;
  aSTG.tymed = TYMED_HGLOBAL;

  return S_OK;
}

HRESULT nsDataObj::GetFileDescriptor_IStreamW(FORMATETC& aFE, STGMEDIUM& aSTG)
{
  HGLOBAL fileGroupDescHandle = ::GlobalAlloc(GMEM_ZEROINIT|GMEM_SHARE,sizeof(FILEGROUPDESCRIPTORW));
  NS_ENSURE_TRUE(fileGroupDescHandle, E_OUTOFMEMORY);

  LPFILEGROUPDESCRIPTORW fileGroupDescW = reinterpret_cast<LPFILEGROUPDESCRIPTORW>(GlobalLock(fileGroupDescHandle));
  if (!fileGroupDescW) {
    ::GlobalFree(fileGroupDescHandle);
    return E_OUTOFMEMORY;
  }

  nsAutoString wideFileName;
  nsresult rv;
  nsCOMPtr<nsIURI> sourceURI;
  rv = GetDownloadDetails(getter_AddRefs(sourceURI),
                          wideFileName);
  if (NS_FAILED(rv))
  {
    ::GlobalFree(fileGroupDescHandle);
    return E_FAIL;
  }

  wcsncpy(fileGroupDescW->fgd[0].cFileName, wideFileName.get(), NS_MAX_FILEDESCRIPTOR - 1);
  fileGroupDescW->fgd[0].cFileName[NS_MAX_FILEDESCRIPTOR - 1] = '\0';
  // one file in the file block
  fileGroupDescW->cItems = 1;
  fileGroupDescW->fgd[0].dwFlags = FD_PROGRESSUI;

  GlobalUnlock(fileGroupDescHandle);
  aSTG.hGlobal = fileGroupDescHandle;
  aSTG.tymed = TYMED_HGLOBAL;

  return S_OK;
}

HRESULT nsDataObj::GetFileContents_IStream(FORMATETC& aFE, STGMEDIUM& aSTG)
{
  IStream *pStream = NULL;

  nsDataObj::CreateStream(&pStream);
  NS_ENSURE_TRUE(pStream, E_FAIL);

  aSTG.tymed = TYMED_ISTREAM;
  aSTG.pstm = pStream;
  aSTG.pUnkForRelease = pStream;

  return S_OK;
}
