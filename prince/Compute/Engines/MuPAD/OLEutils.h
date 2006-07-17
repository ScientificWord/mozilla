// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef OLEUTILS_H
#define OLEUTILS_H

#ifndef CHMTYPES_H
# include <chmtypes.h>
#endif
#ifndef SWOLE_H
//# include <swole.h>
#endif

// requires Windows types
class TCIString;

class SWOleUtils
{
public:
  static bool TCIStringToCLSID(const TCIString& guidstr, CLSID* guid);
  static bool TCIStringToWideStr(const TCIString& str, WCHAR*& pWideStr);
  static TCIString WideStrToTCIString(const WCHAR* pWideStr);
  static bool CLSIDRegistered(const CLSID* guid);
//  static SWVariant::varError TCIStringToBSTR(const TCIString& str, BSTR& bstr);
//  static SWVariant::varError BSTRToTCIString(const BSTR& bstr, TCIString& str);
//  static SWVariant::varError TCITimeToVariantTime(const TCITime& ourTime, double& varTime);
//  static SWVariant::varError VariantTimeToTCITime(const double varTime, TCITime& ourTime );
//  static SWVariant::varError TCIStringToCURRENCY(const TCIString& src, CURRENCY& result);
//  static SWVariant::varError CURRENCYToTCIString(const CURRENCY& currency, TCIString& str);
//  static VARTYPE SWTypeToVarType(SWVariant::varType v);
//  static SWVariant::varError HResultToVarErr(HRESULT hr);
};

#endif

