// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include <windows.h>
//#include "stdafx.h"
#include "OLEutils.h"
#include "unknwn.h"

#include <tcistrin.h>
//#include <tcitime.h>

//#include "app/SciWord.h"

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern CSciwordApp    *SWApp;

/////////////////////////////////////////////////////////////////////////////
// TCIStringToCLSID
// Convert a string representation of a CLSID (including surrounding {}) into the CLSID

bool SWOleUtils::TCIStringToCLSID(const TCIString & guidstr, CLSID * guid)
{
  bool rv = false;
  HRESULT hr;

  int guidstrlen = strlen(guidstr);
  LPWSTR wguidstr = NULL;
  if (TCIStringToWideStr(guidstr, wguidstr)) {
    hr = CLSIDFromString(wguidstr, guid);
    if (hr == NOERROR)
      rv = true;
  }

  delete[] wguidstr;
  return rv;
}

bool SWOleUtils::TCIStringToWideStr(const TCIString & str,
                                        WCHAR * &pWideStr)
{
  int strlen = str.GetLength();
  pWideStr = TCI_NEW(WCHAR[strlen + 1]);
  int res = MultiByteToWideChar(GetACP(), 0, str, -1, pWideStr, strlen + 1);
  return (res > 0);
}

TCIString SWOleUtils::WideStrToTCIString(const WCHAR * pWideStr)
{
  int strlen = wcslen(pWideStr);
  strlen *= 2;                  //double it for "good measure"?
  TCIString rv;
  TCICHAR *strBuff = rv.GetBuffer(strlen + 1);
  int res =
    WideCharToMultiByte(GetACP(), 0, pWideStr, -1, strBuff, strlen + 1,
                        NULL, NULL);
  strBuff[res] = 0;
  rv.ReleaseBuffer(res + 1);
  return rv;
}

bool SWOleUtils::CLSIDRegistered(const CLSID * guid)
{
  bool rv = false;
  HRESULT hr;

  LPWSTR wprogstr = NULL;
  hr = ProgIDFromCLSID(*guid, &wprogstr);
  if (hr == S_OK) {
    rv = true;
    // free the memory
    IMalloc *SysMalloc = NULL;
    hr = CoGetMalloc(1, &SysMalloc);
    if (hr == S_OK) {
      SysMalloc->Free(wprogstr);
      SysMalloc->Release();
    }
  }

  return rv;
}

/*
SWVariant::varError SWOleUtils::TCIStringToBSTR(const TCIString& str, BSTR& bstr)
{
  WCHAR* wStr = NULL;
  TCIStringToWideStr(str,wStr);
  bstr = ::SysAllocString(wStr);
  delete wStr;
  return bstr==NULL ? SWVariant::memError : SWVariant::noError;
}

SWVariant::varError SWOleUtils::BSTRToTCIString(const BSTR& bstr, TCIString& str)
{
  WCHAR* wStr = (WCHAR*)bstr;
  str = WideStrToTCIString(wStr);
  return SWVariant::noError;
}

SWVariant::varError SWOleUtils::TCITimeToVariantTime(const TCITime& ourTime, double& varTime)
{
  SYSTEMTIME sysTime;
  ourTime.GetSYSTEMTIME(sysTime);
  ::SystemTimeToVariantTime(&sysTime, &varTime);
  return SWVariant::noError;
}

SWVariant::varError SWOleUtils::VariantTimeToTCITime(const double varTime, TCITime& ourTime )
{
  SYSTEMTIME sysTime;
  ::VariantTimeToSystemTime( varTime, &sysTime );
  ourTime = TCITime(sysTime);
  return SWVariant::noError;
}
*/
/*
SWVariant::varError SWOleUtils::TCIStringToCURRENCY(const TCIString& src, CURRENCY& result)
{
  SWVariant::varError rv = SWVariant::noError;
  WCHAR* wStr = NULL;
  TCIStringToWideStr(src, wStr);
  HRESULT sc = VarCyFromStr(wStr, LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE, &result);
  switch(sc)
  {
    case DISP_E_BADVARTYPE:
    case DISP_E_OVERFLOW:
    case DISP_E_TYPEMISMATCH:
    case E_INVALIDARG:   
      rv = SWVariant::typeError;   break;

    case E_OUTOFMEMORY:   rv = SWVariant::ptrMemError;   break;
    default:    break;
  }
  return rv;
}

SWVariant::varError SWOleUtils::CURRENCYToTCIString(const CURRENCY& currency, TCIString& str)
{
  SWVariant::varError rv = SWVariant::noError;
  BSTR* pBStr = NULL;
  HRESULT sc = ::VarBstrFromCy(currency, LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE, 
                               pBStr);
  switch(sc)
  {
    case DISP_E_BADVARTYPE:
    case DISP_E_OVERFLOW:
    case DISP_E_TYPEMISMATCH:
    case E_INVALIDARG:   
      rv = SWVariant::typeError;   break;

    case E_OUTOFMEMORY:   rv = SWVariant::ptrMemError;   break;

    default:
      if (pBStr)
        str = WideStrToTCIString((WCHAR*)(*pBStr));
    break;
  }
  return rv;
}
*/
/*
VARTYPE SWOleUtils::SWTypeToVarType(SWVariant::varType vType)
{
  VARTYPE rv = VT_EMPTY;
  switch(vType)
  {
    case SWVariant::v_u8:               rv = VT_UI1;         break;
    case SWVariant::v_i16:              rv = VT_I2;          break;
    case SWVariant::v_comResult:        rv = VT_ERROR;       break;
    case SWVariant::v_i32:              rv = VT_I4;          break;
    case SWVariant::v_flt:              rv = VT_R4;          break;
    case SWVariant::v_dbl:              rv = VT_R8;          break;
    case SWVariant::v_bool:             rv = VT_BOOL;        break;
    case SWVariant::v_date:             rv = VT_DATE;        break;
    case SWVariant::v_currency:         rv = VT_CY;          break;
    case SWVariant::v_tciStr:           rv = VT_BSTR;        break;
    case SWVariant::v_wstr:             rv = VT_BSTR;        break;
    case SWVariant::v_varPtr:           rv = VT_BYREF|VT_VARIANT;     break;

    case SWVariant::v_voidPtr:          rv = VT_BYREF;       break;
    case SWVariant::v_iUnknown:         rv = VT_UNKNOWN;     break;
    case SWVariant::v_iDispatch:        rv = VT_DISPATCH;    break;
    case SWVariant::v_byteArray:        rv = VT_BYREF;       break;  //voidPtr!

    default:                                      break;
  }
  return rv;
}

SWVariant::varError SWOleUtils::HResultToVarErr(HRESULT hr)
{
  SWVariant::varError rv = SWVariant::noError;
  switch(hr)
  {
    case DISP_E_BADINDEX:  rv = SWVariant::arraySizeError;   break;
    case E_INVALIDARG:     rv = SWVariant::typeError;        break;
    case E_OUTOFMEMORY:    rv = SWVariant::memError;         break;
    case S_OK:
    default:                                                 break;
  }
  return rv;
}

*/
