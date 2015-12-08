
// CLASS STRUTILS

// StrUtils is a collection of usefule but non-primitive operations on 
// class TCIString.


#ifndef STRUTIL_H
#define STRUTIL_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

#include <stdarg.h>

class TCIString;

struct StrUtil
{
  static int   UniStrLen(const U16* s);
  static U16 * UniStrNCpy( U16* s1, const U16* s2, U32 c);
  static U16 * UniStrCpy( U16* s1, const U16* s2);
  static int   UniStrCmp(const U16* s1, const U16* s2);
  static int   UniStrNCmp(const U16* s1, const U16* s2, U32 c);
  static int   UniStrICmp(const U16* s1, const U16* s2);
  static int   UniStrNICmp(const U16* s1, const U16* s2, U32 c);
  static int   UniToUpper(U16 uni);
  static int   UniToLower(U16 uni);
  static int   U8ToU16(const U8* src, int srcBytes, U16* dest, int numUniChars);
  
  static TCI_BOOL  LoadStringA(TCIString& str, U32 id);                 // Renamed to LoadString
//  static TCIString GetLoadString(U32 id,...);   
  static TCI_BOOL LoadStringResource(int resid,TCICHAR** strings,int maxnum);

  static void AttrID2CanonicalName( U32 res_ID,U16 targ_ID,
                                        TCIString& nom );
  static U32  CanonicalName2AttrID( U32 res_ID, const TCIString& targ );

  static TCI_BOOL LoadExternalResourceString(TCIString& str,U32 id,void* handle);
  static TCIString GetResourceString(U32 id); // Load a string and return it
  static void      TCIStringToBuffer(const TCIString& src,TCICHAR*& pBuffer,int& bufflen);
           
  static int       StringInt(const TCIString &string);    // Convert to an int  assumes string represents int in decimal format
  static U32       StringHexInt(const TCIString &string); // Convert to an int assumes string represents int in hex format
  static int       StringIntOrHex(const TCIString &string); // Convert to an int
  static TCIString IntString(int number);               // Convert from an int and assign.
  static TCIString HexString(int number, int digits, TCI_BOOL use0x = TRUE);   // Convert from an int and assign.
  static TCIString UnsignedIntString(unsigned int number);
  static TCIString BasedNumberString(int number, U16 base, TCICHAR* charset=NULL, int lim=0, TCI_BOOL bLeftPadded=FALSE);
  static TCIString RomanNumeralString(int number,TCI_BOOL capitalize=FALSE);
  static TCIString AlphaNumberString(int number,TCI_BOOL capitalize=FALSE);
  static TCIString CharString(TCICHAR ch);               // Convert from a char and assign.
  static const TCICHAR* ConvertTCIStringToSysArg(const TCIString& str);
  static double StringToDouble(const TCICHAR* numStr);
  static TCIString DoubleToString(double theVal);
       
  // simple formatting
  static TCIString Format(const TCICHAR* lpszFormat, ...);
  static TCIString GetFormattedResourceString(U32 id,...);   // Load a string, format if necessary, return it
  static TCIString FormatUsingArgList(const TCICHAR* lpszFormat, va_list args);

  // Hex encoding and decoding
  static TCIString MakeHex(const TCIString& in);
  static TCIString UndoHex(const TCIString& in);

   static U32 GetANSICodepage();
   static TCI_BOOL IsCodepageDoublebyte(U32 codepage); 
   static TCI_BOOL IsValidCodepage(U32 codepage); 
   static TCI_BOOL IsDBCSLeadByte(U8 c, U32 codepage); 
   static int MultiByteToUnicode(const char* mb, int numbytes, U16* uc, int numUchars, U32 codepage);

  // formatting of Windows error codes
  static TCIString FormatLastError( U32 err );

  //support for delimiter-separated multiple strings
  class substringEnumerator
  {
    public:
    substringEnumerator(const TCIString& targetStr, TCICHAR sep = '|') 
      : m_targetStr(targetStr), m_nOffset(0), m_separator(sep)
      { CheckInit(); }
    void CheckInit();

    int m_nOffset;
    TCICHAR m_separator;
    const TCIString& m_targetStr;
  };

  static TCIString SelectSubstringUsingIndex(const TCIString& theStr, 
                                                  int nIndex, TCICHAR sep);
  static substringEnumerator StartEnumSubstrings(const TCIString& theStr, TCICHAR sep);
  static TCI_BOOL GetNextSubstring(substringEnumerator& subEnum, TCIString& nextStr);
  //just put this here so callers wouldn't be confused by its absence (there's nothing to do):
  static void CloseEnumSubStrings(substringEnumerator& subEnum) {} 
  
  
//The following functionality (stringSubsCallbacks) is used much like a normal StrUtil::Format(). The
//  difference is that you can provide key names indicating how to fill in the parameters (together
//  with callback functions for retrieving values from the names). Thus for instance:
//    stringWithParamsFormatter formatter.format("%s.%s|<filename>|<fileextension>");
//Paramters passed directly can be used by leaving a blank keyname, as in:
//  formatter.Format("%s.%s||<fileextension>", "swp0001");
//The number of keynames (empty or not) separated by '|' (or an alternate separator if you wish) MUST 
//be the same as the number of substitutions indicated in the string (the number of % format specifications,
//  plus any of the width or precision specifications indicated with a '*').
//The "stringSubsParamType" enumeration is provided so that the caller may use a single
//  callback for all types if desired, with simple pass-throughs which add the type as a parameter.
  
  enum stringSubsParamType { unknownType, stringType, intType, uintType, charType, dblType,
                              ptrType, wcharType };
  
//  typedef TCI_BOOL (*subsParamGetString)(TCIString& paramVal, const TCIString& paramName, void* pData);
  typedef TCI_BOOL (*subsParamGetString)(TCICHAR* pBuff, int& nLen, const TCIString& paramName, void* pData);
  typedef TCI_BOOL (*subsParamGetInt)(int& paramVal, const TCIString& paramName, void* pData);
  typedef TCI_BOOL (*subsParamGetUInt)(U32& paramVal, const TCIString& paramName, void* pData);
  typedef TCI_BOOL (*subsParamGetChar)(TCICHAR& paramVal, const TCIString& paramName, void* pData);
  typedef TCI_BOOL (*subsParamGetDouble)(double& paramVal, const TCIString& paramName, void* pData);
  typedef TCI_BOOL (*subsParamGetPointer)(void*& paramVal, const TCIString& paramName, void* pData);
  typedef TCI_BOOL (*subsParamGetWChar)(TCIWCHAR& paramVal, const TCIString& paramName, void* pData);
//The next one could be added at some point to allow callers to set a single callback. Static passthrough
//  functions could be added to serve as the other callbacks, with the pData parameter being
//  filled in as "this", and then each passthrough setting the appropriate type and pData. But not today.
//  typedef TCI_BOOL (*subsParamGetParam)(void* pParam, stringSubsParamType type, const TCIString& paramName, void* pData);
  
  class stringSubsCallbacks
  {
  public:
    stringSubsCallbacks() : m_pGetString(0), m_pGetInt(0), m_pGetUInt(0),
                            m_pGetChar(0), m_pGetDouble(0), m_pGetPointer(0) {}
    stringSubsCallbacks(subsParamGetString pGetString, subsParamGetInt pGetInt, subsParamGetUInt pGetUInt,
                        subsParamGetChar pGetChar, subsParamGetDouble pGetDouble, 
                        subsParamGetPointer pGetPointer, subsParamGetWChar pGetWChar)
                        : m_pGetString(pGetString), m_pGetInt(pGetInt), m_pGetUInt(pGetUInt),
                          m_pGetChar(pGetChar), m_pGetDouble(pGetDouble), 
                          m_pGetPointer(pGetPointer), m_pGetWChar(pGetWChar) {}
    stringSubsCallbacks(const stringSubsCallbacks& src)
                        : m_pGetString(src.m_pGetString), m_pGetInt(src.m_pGetInt), m_pGetUInt(src.m_pGetUInt),
                          m_pGetChar(src.m_pGetChar), m_pGetDouble(src.m_pGetDouble), 
                          m_pGetPointer(src.m_pGetPointer), m_pGetWChar(src.m_pGetWChar) {}
    stringSubsCallbacks& operator=(const stringSubsCallbacks& src)
    { m_pGetString = src.m_pGetString;
      m_pGetInt = src.m_pGetInt;
      m_pGetUInt = src.m_pGetUInt;
      m_pGetChar = src.m_pGetChar;
      m_pGetDouble = src.m_pGetDouble;
      m_pGetPointer = src.m_pGetPointer;
      m_pGetWChar = src.m_pGetWChar;
      return *this;
    }
                          
    subsParamGetString m_pGetString;
    subsParamGetInt m_pGetInt;
    subsParamGetUInt m_pGetUInt;
    subsParamGetChar m_pGetChar;
    subsParamGetDouble m_pGetDouble;
    subsParamGetPointer m_pGetPointer;
    subsParamGetWChar m_pGetWChar;
  };
  class stringWithParamsFormatter
  {
  public:
    enum {m_startMaxParams = 10};
    
    stringWithParamsFormatter() : m_separator('|'), m_callbackData(0) {}
    stringWithParamsFormatter(stringSubsCallbacks& callbacks, void* callbackData, TCICHAR sep = '|')
                                : m_separator(sep), m_callbackData(callbackData),
                                  m_getParams(callbacks) {}
    stringWithParamsFormatter( const stringWithParamsFormatter& src )
                                : m_separator(src.m_separator), m_callbackData(src.m_callbackData),
                                  m_getParams(src.m_getParams) {}
    
    stringWithParamsFormatter& operator=(const stringWithParamsFormatter& src)
    { m_separator = src.m_separator;
      m_callbackData = src.m_callbackData;
      m_getParams = src.m_getParams;
      return *this;
    }  
    
    void setCallbacks(subsParamGetString pGetString, subsParamGetInt pGetInt, subsParamGetUInt pGetUInt,
                        subsParamGetChar pGetChar, subsParamGetDouble pGetDouble, 
                        subsParamGetPointer pGetPointer, subsParamGetWChar pGetWChar)
    { m_getParams = stringSubsCallbacks(pGetString, pGetInt, pGetUInt, 
                                 pGetChar, pGetDouble, pGetPointer, pGetWChar);
    }                             
    void setCallbacks(const stringSubsCallbacks& callbacks)
    { m_getParams = callbacks; }
    void setCallbackData(void* callbackData)
    { m_callbackData = callbackData; }
    void setSeparatorChar( TCICHAR newSep )
    { m_separator = newSep; }
    TCIString format(const TCICHAR* formatStr, ...) const;
  private:
    static void normalizeParamName( TCIString& paramName );  //strip surrounding "<>" (if present?)
    static TCI_BOOL getStringVal( subsParamGetString pGetString, TCIString& theStr, const TCIString& paramName, void* pData );

    TCICHAR m_separator;
    stringSubsCallbacks m_getParams;
    void* m_callbackData;
  };
  
};


#endif