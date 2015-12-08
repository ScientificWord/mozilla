
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "tcistrin.h"
#include "strutil.h"
#include "tcidebug.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define NOGDICAPMASKS     
#define NOVIRTUALKEYCODES 
#define NOWINMESSAGES     
#define NOWINSTYLES       
#define NOSYSMETRICS      
#define NOMENUS           
#define NOICONS           
#define NOKEYSTATES       
#define NOSYSCOMMANDS     
#define NORASTEROPS       
#define NOSHOWWINDOW      
#define OEMRESOURCE       
#define NOATOM            
#define NOCLIPBOARD       
#define NOCOLOR           
#define NOCTLMGR          
#define NODRAWTEXT        
#define NOGDI             
#define NOKERNEL          
//#define NOUSER            
//#define NONLS             
#define NOMB              
#define NOMEMMGR          
#define NOMETAFILE        
#define NOMINMAX          
#define NOMSG             
#define NOOPENFILE        
#define NOSCROLL          
#define NOSERVICE         
#define NOSOUND           
#define NOTEXTMETRIC      
#define NOWH              
#define NOWINOFFSETS      
#define NOCOMM            
#define NOKANJI           
#define NOHELP            
#define NOPROFILER        
#define NODEFERWINDOWPOS  
#define NOMCX             
#include <windows.h>

extern void * GetInstanceHandle();


int StrUtil::UniStrCmp(const U16* s1, const U16* s2)
{
  int i = 0;
  while (TRUE)
  {
    if       (s1[i] < s2[i])            return -1;
    else if  (s1[i] > s2[i])            return 1; 
    else if  (s1[i] == 0 && s2[i] == 0) return 0; 
    else i++; 
  }     
}

int StrUtil::UniStrNCmp(const U16* s1, const U16* s2, U32 c)
{
  U32 i = 0;
  while (TRUE)
  {
    if       (i == c)                   return  0;
    else if  (s1[i] < s2[i])            return -1;
    else if  (s1[i] > s2[i])            return  1; 
    else if  (s1[i] == 0 && s2[i] == 0) return  0; 
    else i++; 
  }
}

int StrUtil::UniStrICmp(const U16* s1, const U16* s2)
{
  int i = 0;
  U16 u1, u2;
  while (TRUE)
  {
    u1 = UniToUpper(s1[i]); 
    u2 = UniToUpper(s2[i]);
    if       (u1 < u2)            return -1;
    else if  (u1 > u2)            return 1; 
    else if  (u1 == 0 && u2 == 0) return 0; 
    else i++; 
  }     

}

int StrUtil::UniStrNICmp(const U16* s1, const U16* s2, U32 c)
{
  U32 i = 0;
  U16 u1, u2;
  while (TRUE)
  {
    if       (i == c)             return  0;
    u1 = UniToUpper(s1[i]); 
    u2 = UniToUpper(s2[i]);
    if  (u1 < u2)            return -1;
    else if  (u1 > u2)            return 1; 
    else if  (u1 == 0 && u2 == 0) return 0; 
    else i++; 
  }     

}

// UniToUpper and UniToLower should work through unicode 0x017f
int   StrUtil::UniToUpper(U16 uni)
{
  if (uni < 128)
    return toupper(uni);
  else if ( 0xe0 <= uni &&  uni<= 0xfe && uni != 0xf7)
    return uni - 32;
  else if ( uni == 0xff)
    return 0x0178;
  else if ( 0x0101 <= uni &&  uni<= 0xfe && uni != 0x0137 && (uni %2 == 1)/*odd*/)
    return uni - 1;
  else if ( 0x013a <= uni &&  uni<= 0xfe && uni != 0x0148 && (uni %2 == 0)/*even*/)
    return uni - 1;
  else if ( 0x014b <= uni &&  uni<= 0xfe && uni != 0x0177 && (uni %2 == 1)/*odd*/)
    return uni - 1;
  else if ( uni ==0x017a ||  uni == 0x17c && uni == 0x017e)
    return uni - 1;
  else
    return uni; 
}

int   UniToLower(U16 uni)
{
  if (uni < 128)
    return tolower(uni);
  else if ( 0xc0 <= uni &&  uni<= 0xde && uni != 0xd7)
    return uni + 32;
  else if ( 0x0100 <= uni &&  uni<= 0xfe && uni != 0x0136 && (uni %2 == 0)/*even*/)
    return uni +- 1;
  else if ( 0x0139 <= uni &&  uni<= 0xfe && uni != 0x0147 && (uni %2 == 1)/*odd*/)
    return uni + 1;
  else if ( 0x014a <= uni &&  uni<= 0xfe && uni != 0x0176 && (uni %2 == 0)/*even*/)
    return uni + 1;
  else if ( uni ==0x0179 ||  uni == 0x17b && uni == 0x017d)
    return uni + 1;
  else if (uni == 0x0178)
    return 0xff;
  else
    return uni; 
}

int  StrUtil::U8ToU16(const U8* src, int srcBytes, U16* dest, int numUniChars)
{
  int rv(0);
  if (src && srcBytes > 0 && dest && numUniChars > 0)
  while (rv < srcBytes && rv < numUniChars-1)
  {
    dest[rv] = src[rv];
    rv += 1;
  }
  dest[rv] = 0;
  return rv;
}


TCI_BOOL StrUtil::LoadStringA(TCIString& str, U32 id) 
{
  int nSize = 256;
  int rv =  ::LoadString((HINSTANCE)GetInstanceHandle(), id, str.GetBuffer(nSize), nSize);
  str.ReleaseBuffer();
  while (nSize - rv <= 2)
  {
    nSize *= 2;
    rv = ::LoadString((HINSTANCE)GetInstanceHandle(), id, str.GetBuffer(nSize), nSize);
    str.ReleaseBuffer();
  }

  return (rv > 0);
} 


TCI_BOOL StrUtil::LoadStringResource(int resid,TCICHAR** strings,int maxnum)
{
  HINSTANCE hInst =  (HINSTANCE)GetInstanceHandle();
  HRSRC hrsrc  =
      ::FindResource(hInst, MAKEINTRESOURCE(resid), RT_RCDATA);
  HGLOBAL hglob  =  ::LoadResource(hInst, hrsrc);
  TCICHAR* q  =  (TCICHAR*)::LockResource( hglob );
  if (q == NULL || strings==NULL)
    return FALSE;
  for (int ix=0; ix<maxnum && *q != 0; ix++) {
    int n = strlen(q) + 1;
    strings[ix] = TCI_NEW(TCICHAR[n]);
      strcpy(strings[ix],q);
      strings[ix][n-1] = 0;
    q += n;
  }
  ::FreeResource( hglob );
  return TRUE;
}


// Given an RCDATA and an ID, 

void StrUtil::AttrID2CanonicalName( U32 res_ID,U16 targ_ID,
                                            TCIString& nom ) {

  HINSTANCE hInst =  (HINSTANCE)GetInstanceHandle();
  HRSRC hrsrc  =
      ::FindResource( hInst,MAKEINTRESOURCE(res_ID),RT_RCDATA );
  HGLOBAL hglob  =  ::LoadResource( hInst,hrsrc );
  char* q  =  (char*)::LockResource( hglob );

  int nWordSize =  sizeof(WORD)/sizeof(char);
  while ( q && *q ) {
// canonical0ID...
// ^         ^
    char* save  =  q;
    q   +=  strlen(q) + 1;
    WORD* p =  (WORD*)q;
    if ( *p == targ_ID ) {
      nom =  (const U8*)save;
      break;
    } else
      q +=  nWordSize;
  }             // while ( q && *q )

  ::FreeResource( hglob );
}


U32 StrUtil::CanonicalName2AttrID( U32 res_ID, const TCIString& targ ) {

  U32 rv  =  0;

  HINSTANCE hInst =  (HINSTANCE)GetInstanceHandle();
  HRSRC hrsrc  =
      ::FindResource( hInst,MAKEINTRESOURCE(res_ID),RT_RCDATA );
  HGLOBAL hglob  =  ::LoadResource( hInst,hrsrc );

  char* q  =  (char*)LockResource( hglob );

  TCIString nom;
  int nWordSize =  sizeof(WORD)/sizeof(char);
  while ( q && *q ) {
// canonical0ID...
// ^         ^
    nom =  (const U8*)q;    // canonical name
    q   +=  strlen(q) + 1;
    if ( nom == targ ) {
      WORD* p =  (WORD*)q;  // ID
      rv  =  *p;
      break;
    } else
      q +=  nWordSize;
  }             // while ( q && *q )

  ::FreeResource( hglob );
  return rv;
}



TCI_BOOL StrUtil::LoadExternalResourceString(TCIString& str,U32 id,void* handle)
{
  TCI_BOOL rv = FALSE;
  I32 limit = 256;
  I32 numchars = 0;
  while (!rv && limit < 30000) {
      numchars = ::LoadString((HINSTANCE)handle, id, str.GetBuffer(limit), limit-1);
      if (numchars>=limit-1)
        limit = limit<<1;
      else
        rv = TRUE;
  }
  if (rv)
    str.ReleaseBuffer(numchars+1);
  else
    str.Empty();
  return rv;
}

TCIString StrUtil::GetResourceString(U32 id)
{
  TCIString str;
  LoadStringA(str, id);
  return str;
}


void StrUtil::TCIStringToBuffer(const TCIString& src, TCICHAR*& pBuffer, int& bufflen)
{
  int len = src.GetLength();
  if ( len > bufflen) {
    delete[] pBuffer;
    pBuffer = TCI_NEW(TCICHAR[len+1]);
  }
  memcpy( pBuffer, (const TCICHAR*)src, len * sizeof(TCICHAR) );
}


TCIString StrUtil::IntString(int number)
{
  TCICHAR number_string[30];

  sprintf( number_string, "%d", number );
  return number_string;
}


TCIString StrUtil::HexString(int number, int digits, TCI_BOOL use0x )
{
  TCICHAR number_string[30];
  sprintf(number_string, "%#0*x", digits+2, number );
  TCIString rv(number_string);
  if (!use0x) 
    rv = rv.Mid(2);
  return rv;
}


int StrUtil::StringIntOrHex(const TCIString &string) {
  int pos = string.Find("0x");
  if (pos < 0) {
    return(StrUtil::StringInt(string));
  } else {
    return (int)StrUtil::StringHexInt(string);
  }
}


int StrUtil::StringInt(const TCIString &string)
{
  int firstDigit = string.FindOneOf("-+0123456789");
  TCIString writeableCopy = string;
  TCICHAR* buf = writeableCopy.GetBuffer(0);
  return firstDigit != -1 ? _tciatoi(&buf[firstDigit]) : 0;
}


U32 StrUtil::StringHexInt(const TCIString &string)
{
  int pos = string.Find("0x");
  if (pos < 0) { // did not find 0x still assume hex format
    pos = string.FindOneOf("0123456789ABCDEFabcdef");
    if (pos >= 0) 
      pos -= 1; //pos point to character before first hex digit
    else
      return 0;
  } else {
    pos +=  1;    //pos point to character before first hex digit
  }
  int n =  pos;
  while (n < string.GetLength()-1 && isxdigit(string[n+1]))
  {
    n++;
  }

  U32 rv =  0;
  unsigned int mult =  1;
  for (n; n>pos; n--)
  {
    int zz =  string[n] - '0';
    if (zz > 9) {
      int ch =  tolower(string[n]);
      TCI_ASSERT(ch >= 'a' && ch <= 'f');
      zz =  10 + ch - 'a';
    }
    rv +=  zz * mult;
    mult *=  16;
  }

  return(rv);
}



TCIString StrUtil::UnsignedIntString(unsigned int number)
{
  TCICHAR number_string[30];
  sprintf( number_string, "%u", number );
  return number_string;
}


TCIString StrUtil::BasedNumberString(int number, U16 base, TCICHAR* charset, int lim, TCI_BOOL bLeftPadded)
{
  if (!base)
    return TCIString("");

  if (base > 62)
    base = 62;
  TCI_BOOL delcharset = FALSE;
  if (!charset) 
  {
    delcharset = TRUE;
    charset = TCI_NEW(TCICHAR[base]);
    TCICHAR nextchar = '0';
    for (U16 ii=0;ii<10 && ii<base;ii++)
      charset[ii] = nextchar++;
    nextchar = 'A';
    for (U16 ii=10;ii<36 && ii<base;ii++)
      charset[ii] = nextchar++;
    nextchar = 'a';
    for (U16 ii=36;ii<62 && ii<base;ii++)
      charset[ii] = nextchar++;
  }

  if (lim<=0)
  {
    bLeftPadded = FALSE;  //doesn't make sense to pad if no size passed in
    lim = 32;    //way too much room...
  }
  TCI_BOOL isneg = FALSE;
  if (number < 0) 
  {
    isneg = TRUE;
    number = -number;
  }
  
  TCIString temp;
//  TCICHAR* ptr = temp.GetBuffer(lim + (isneg ? 1 : 0));
  TCICHAR* ptr = temp.GetBuffer(lim);
  lim--;  //to get indexing right inside loop
//  if (isneg)
//    *ptr++ = '-';
  U16 m = 0;
  int i = 0;
  for (i=lim; i>=0; i--)
  {
    m = number % base;
    number = number/base;
    ptr[i] = charset[m];
    if (!bLeftPadded && !number)
      break;
  }
  if (i < 0)
    i = 0;
  temp.ReleaseBuffer(lim+1);
//  if (i > 0) 
//  {
//    for (U16 j=0; j<i; j++)
//      ptr[j] = ptr[j + lim - 1 - i];
//    temp.ReleaseBuffer(i + 1 + isneg ? 1 : 0);
//  }

  TCIString retStr;
  if (isneg)
    retStr += '-';
  retStr += temp.Mid(i);

  if (delcharset)
    delete charset;
  return retStr; 
}

TCIString StrUtil::RomanNumeralString(int number,TCI_BOOL capitalize)
{
  TCIString temp;
  TCICHAR* ptr = temp.GetBuffer(64);  //should be plenty of room
  U16 disp = capitalize ? 0 : U16('a' - 'A');
  U16 offset = 0;
  if (number < 0) {
    TCI_ASSERT(FALSE);  //should we ever allow negative roman numerals? i think NOT...
    ptr[offset++] = '-';  //but in case we did...
  }
  if (number >= 1000) {
    TCI_ASSERT(number < 50000);  //otherwise we may not fit?
    int n =  number / 1000;
    for (int i = 0; i<n; i++)
      ptr[offset++] = 'M';
    number %=  1000;
  }
  if (number >= 900) {
    ptr[offset++] = 'C' + disp;
    ptr[offset++] = 'M' + disp;
    number -=  900;
  }
  if (number >= 500) {
    ptr[offset++] = 'D' + disp;
    number -=  500;
  }
  if (number >= 400) {
    ptr[offset++] = 'C' + disp;
    ptr[offset++] = 'D' + disp;
    number -=  400;
  }
  if (number >= 100) {
    int n =  number / 100;
    for (int i = 0; i<n; i++)
      ptr[offset++] = 'C' + disp;
    number %=  100;
  }
  if (number >= 90) {
    ptr[offset++] = 'X' + disp;
    ptr[offset++] = 'C' + disp;
    number -=  90;
  }
  if (number >= 50) {
    ptr[offset++] = 'L' + disp;
    number -=  50;
  }
  if (number >= 40) {
    ptr[offset++] = 'X' + disp;
    ptr[offset++] = 'L' + disp;
    number -=  40;
  }
  if (number >= 10) {
    int n =  number / 10;
    for (int i = 0; i<n; i++)
      ptr[offset++] = 'X' + disp;
    number %=  10;
  }
  if (number >= 9) {
    ptr[offset++] = 'I' + disp;
    ptr[offset++] = 'X' + disp;
    number -=  9;
  }
  if (number >= 5) {
    ptr[offset++] = 'V' + disp;
    number -=  5;
  }
  if (number >= 4) {
    ptr[offset++] = 'I' + disp;
    ptr[offset++] = 'V' + disp;
    number -=  4;
  }
  if (number >= 1) {
    for (int i=0; i<number; i++)
      ptr[offset++] = 'I' + disp;
  }
  temp.ReleaseBuffer(offset);
  return temp;
}

TCIString StrUtil::AlphaNumberString(int number,TCI_BOOL capitalize)
{
  TCIString temp;
  if (number==0) {
    temp.Empty();
    return TCIString("");
  }

  TCI_BOOL isneg = FALSE;
  if (number < 0) {
    TCI_ASSERT(FALSE);  //should we allow this?
    isneg = TRUE;     //okay, i guess
    number = -number;
  }
  char base =  capitalize ? 'A' : 'a';
  number--;    // make zero based
  char ch = number % 26 + base;
  number = number / 26 + 1;  //number of repetitions
  if (number >= 100) { //arbitrary limit
    TCI_ASSERT(FALSE);
    number = 100;
  }
  TCICHAR* ptr = temp.GetBuffer(number + (isneg ? 1 : 0));
  if (isneg)
    *(ptr++) = '-';
  for (U16 ii=0;ii<number;ii++)
    *(ptr++) = ch;
  temp.ReleaseBuffer(-1);
  return temp;
}


TCIString StrUtil::CharString(TCICHAR ch)
{
  TCIString temp(ch,1);
  return temp;
}

double StrUtil::StringToDouble(const TCICHAR* numStr)
{
  TCIString theStr(numStr);
  int nStart = theStr.FindOneOf("0123456789.+-");
  return (nStart >= 0) ? _tciatof(((const TCICHAR*)theStr) + nStart) : 0.0;
}

TCIString StrUtil::DoubleToString(double theVal)
{
  return Format("%g", theVal);
}

TCIString StrUtil::SelectSubstringUsingIndex(const TCIString& theStr, 
                                                   int nIndex, TCICHAR sep)
{
  TCIString rv;
  int nStart = (!theStr.IsEmpty() && theStr.GetAt(0) == sep) ? 1 : 0;
  int nEnd = theStr.Find(sep, nStart);
  int ix =0;
  for (ix = 0; nEnd >= 0 && ix < nIndex; ix++)
  {
    nStart = nEnd + 1;
    nEnd = theStr.Find(sep, nStart);
  }
  if (ix == nIndex && nStart>=0)  //found it!
  {
    rv = nEnd>=nStart ? theStr.Mid(nStart, nEnd - nStart) : theStr.Mid(nStart);
    rv.Trim();
  }
  return rv;
}


void StrUtil::substringEnumerator::CheckInit()
{
  if (m_targetStr.GetLength() > 0 && m_targetStr.GetAt(0) == m_separator) 
    m_nOffset = 1;
}


StrUtil::substringEnumerator StrUtil::StartEnumSubstrings(const TCIString& theStr, TCICHAR sep)
{
  return substringEnumerator(theStr, sep);
}

TCI_BOOL StrUtil::GetNextSubstring(StrUtil::substringEnumerator& subEnum, TCIString& nextStr)
{
  if (subEnum.m_targetStr.GetLength() <= subEnum.m_nOffset)
  {
    nextStr.Empty();
    return FALSE;
  }

  int nStart = subEnum.m_nOffset;
  int nEnd = subEnum.m_targetStr.Find(subEnum.m_separator, nStart);
  if (nEnd>=nStart)  //found it!
    nextStr = subEnum.m_targetStr.Mid(nStart, nEnd - nStart);
  else
  {
    nextStr = subEnum.m_targetStr.Mid(nStart);
    nEnd = subEnum.m_targetStr.GetLength();
  }  
  nextStr.Trim();
  subEnum.m_nOffset = nEnd + 1;
  return TRUE;
}

//The purpose of this function is to allow quick conversion of a TCIString to a
//LPCTSTR suitable for passing to system functions. In particular, a NULL TCIString
//reference is converted to a NULL string pointer.
const TCICHAR* StrUtil::ConvertTCIStringToSysArg(const TCIString& str)
{
  if (&str == NULL)
    return (const TCICHAR*)NULL;
  else
    return (const TCICHAR*)str;
}

// TCIString formatting
/////////////////////////////////////////////////////////////////////////////

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

// formatting (using wsprintf style formatting)
TCIString StrUtil::Format(const TCICHAR* lpszFormat, ...)
{
  va_list argList;
  va_start(argList, lpszFormat);
  TCIString str(FormatUsingArgList(lpszFormat, argList));
  va_end(argList);

  return str;
}


TCIString StrUtil::GetFormattedResourceString(U32 id,...)
{
  TCIString fmt;
  va_list argList;
  va_start(argList, id);
  TCICHAR lpszFormat[257];
  U32 rv =  (U32)::LoadString((HINSTANCE)GetInstanceHandle(), id, lpszFormat, 256);
  if (rv > 0)
    fmt = FormatUsingArgList(lpszFormat, argList);
  va_end(argList);
  return fmt;
}


inline TCICHAR *_tcistrinc(const TCICHAR *cp)
{ 
  return (TCICHAR *)(cp + 1);
}

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

static
int EstimateFormatLength(const TCICHAR *lpszFormat, va_list argList)
{
  TCI_TRACE("CALLED: Format(const TCICHAR *, ...)");

    // make a guess at the maximum length of the resulting string
    int nMaxLen = 0;
    for (const TCICHAR *lpsz = lpszFormat; *lpsz != '\0'; lpsz = _tcistrinc(lpsz))
    {
        // handle '%' character, but watch out for '%%'
        if (*lpsz != '%' || *(lpsz = _tcistrinc(lpsz)) == '%')
        {
            nMaxLen += 1;
            continue;
        }

        int nItemLen = 0;

        // handle '%' character with format
        int nWidth = 0;
        for (; *lpsz != '\0'; lpsz = _tcistrinc(lpsz))
        {
            // check for valid flags
            if (*lpsz == '#')
                nMaxLen += 2;   // for '0x'
            else if (*lpsz == '*')
                nWidth = va_arg(argList, int);
            else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
                *lpsz == ' ')
                ;
            else // hit non-flag character
                break;
        }
        // get width and skip it
        if (nWidth == 0)
        {
            // width indicated by
            nWidth = _tciatoi(lpsz);
            for (; *lpsz != '\0' && _tciisdigit(*lpsz); lpsz = _tcistrinc(lpsz))
                ;
        }
        TCI_ASSERT(nWidth >= 0);

        int nPrecision = 0;
        if (*lpsz == '.')
        {
            // skip past '.' separator (width.precision)
            lpsz = _tcistrinc(lpsz);

            // get precision and skip it
            if (*lpsz == '*')
            {
                nPrecision = va_arg(argList, int);
                lpsz = _tcistrinc(lpsz);
            }
            else
            {
                nPrecision = _tciatoi(lpsz);
                for (; *lpsz != '\0' && _tciisdigit(*lpsz); lpsz = _tcistrinc(lpsz))
                    ;
            }
            TCI_ASSERT(nPrecision >= 0);
        }

        // should be on type modifier or specifier
        int nModifier = 0;
        switch (*lpsz)
        {
        // modifiers that affect size
        case 'h':
            nModifier = FORCE_ANSI;
            lpsz = _tcistrinc(lpsz);
            break;
        case 'l':
            nModifier = FORCE_UNICODE;
            lpsz = _tcistrinc(lpsz);
            break;

        // modifiers that do not affect size
        case 'F':
        case 'N':
        case 'L':
            lpsz = _tcistrinc(lpsz);
            break;
        }

        // now should be on specifier
        switch (*lpsz | nModifier)
        {
        // single characters
        case 'c':
        case 'C':
            nItemLen = 2;
            va_arg(argList, TCICHAR);
            break;
        case 'c'|FORCE_ANSI:
        case 'C'|FORCE_ANSI:
            nItemLen = 2;
            va_arg(argList, char);
            break;
        case 'c'|FORCE_UNICODE:
        case 'C'|FORCE_UNICODE:
            nItemLen = 2;
            va_arg(argList, TCIWCHAR);
            break;

        // strings
        case 's':
        case 'S':
            nItemLen = _tcistrlen(va_arg(argList, const TCICHAR *));
            nItemLen = CHAMmax(1, nItemLen);
            break;
        case 's'|FORCE_ANSI:
        case 'S'|FORCE_ANSI:
            nItemLen = _tcistrlen(va_arg(argList, const char *));
            nItemLen = CHAMmax(1, nItemLen);
            break;

#if 0
#ifndef _MAC
        case 's'|FORCE_UNICODE:
        case 'S'|FORCE_UNICODE:
            nItemLen = wcslen(va_arg(argList, const TCIWCHAR *));
            nItemLen = CHAMmax(1, nItemLen);
            break;
#endif
#endif
        }

        // adjust nItemLen for strings
        if (nItemLen != 0)
        {
            nItemLen = CHAMmax(nItemLen, nWidth);
            if (nPrecision != 0)
                nItemLen = CHAMmin(nItemLen, nPrecision);
        }
        else
        {
            switch (*lpsz)
            {
            // integers
            case 'd':
            case 'i':
            case 'u':
            case 'x':
            case 'X':
            case 'o':
                va_arg(argList, int);
                nItemLen = 32;
                nItemLen = CHAMmax(nItemLen, nWidth+nPrecision);
                break;

            case 'e':
            case 'f':
            case 'g':
            case 'G':
                va_arg(argList, double);
                nItemLen = 32;
                nItemLen = CHAMmax(nItemLen, nWidth+nPrecision);
                break;

            case 'p':
                va_arg(argList, void*);
                nItemLen = 32;
                nItemLen = CHAMmax(nItemLen, nWidth+nPrecision);
                break;

            // no output
            case 'n':
                va_arg(argList, int*);
                break;

            default:
                TCI_ASSERT(FALSE);  // unknown formatting option
            }
        }

        // adjust nMaxLen for output nItemLen
        nMaxLen += nItemLen;
    }

  TCI_TRACE1("Format - nMaxLen:%d", nMaxLen);

  return nMaxLen;
}

TCIString StrUtil::FormatUsingArgList(const TCICHAR* format, va_list args)
{
  int len =  EstimateFormatLength(format, args);
  TCIString string_buf;
  TCICHAR* buff =  string_buf.GetBuffer(len);
  TCI_VERIFY(vsprintf(buff, format, args) <= len);
  string_buf.ReleaseBuffer();

  return string_buf;
}


// Hex encode and decode


char HexToChar(char c0, char c1) 
{
   char digit;
   digit = (c0 >= 'A' ? ((c0 & 0xdf) - 'A')+10 : (c0 - '0'));
   digit *= 16;
   digit += (c1 >= 'A' ? ((c1 & 0xdf) - 'A')+10 : (c1 - '0'));
   return(digit);
}


TCIString CharToHex(char ch)
{
   TCIString hex;
   int c = ch;
   if (ch < 0){
      c = 256+ch;
   }

   int digit = c / 16;
   if (digit < 10){
      hex += '0' + digit;
   }else{
      hex += 'A' - 10 + digit;
   }
   digit = c % 16;
   if (digit < 10){
      hex += '0' + digit;
   }else{
      hex += 'A' - 10 + digit;
   }
   return hex;
}



TCIString StrUtil::MakeHex(const TCIString& in)
{
   TCIString out;
   for (int i = 0; i < in.GetLength(); ++i){
      out += CharToHex(in[i]);
   }
   return out;

}

TCIString StrUtil::UndoHex(const TCIString& in)
{
     if (in.GetLength() == 0) 
         return "";

     TCIString out;
     for (int i = 0; i < in.GetLength()-1; i += 2){
       out += HexToChar(in[i], in[i+1]);
     }
     return out;

}

int  StrUtil::MultiByteToUnicode(const char* mb, int numbytes, U16* uc, int  numUchars, U32 codepage)
{
  if (codepage == 0 )
    codepage = GetACP();
  return MultiByteToWideChar(codepage, 0, (LPCSTR)mb, numbytes, uc, numUchars);
}

U32 StrUtil:: GetANSICodepage()
{
  return (U32)GetACP();
}

TCI_BOOL StrUtil::IsCodepageDoublebyte(U32 cp)
{
  return (cp == 932 || cp == 936 || cp == 949 || cp == 950 || cp == 1361);
} 

TCI_BOOL StrUtil::IsValidCodepage(U32 codepage)
{
  //ljh 11/04 -- a codepage is valid when if is installed on the current system.
  return IsValidCodePage(codepage);
}

TCI_BOOL StrUtil::IsDBCSLeadByte(U8 c, U32 codepage) 
{
  if (codepage == 0 )
    codepage = StrUtil:: GetANSICodepage();
  return (IsDBCSLeadByteEx(codepage, c) != 0); 
}

TCIString StrUtil::FormatLastError(U32 err) 
{
  LPVOID lpMsgBuf;

  if (err == 0)
    err = GetLastError();
  if (err == 0)  // no error to format so return empty string
    return EmptyTCIString;
    
  FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
    NULL,
    err,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
  );

  TCIString out(reinterpret_cast<const TCICHAR*>(lpMsgBuf));
  // Free the buffer.
  LocalFree( lpMsgBuf );

  return out;
}


void StrUtil::stringWithParamsFormatter::normalizeParamName( TCIString& paramName )
{
  paramName.Trim();   //should we strip surrounding whitespace? assuming yes.
  if (paramName[0] == '<' && paramName[paramName.GetLength() - 1] == '>')
    paramName = paramName.Mid( 1, paramName.GetLength() - 2 );
}


//A macro that's handy below:
#define FMT_ADD_TO_OUTPUT_STR( outputStr, currFmtStr, intSpecifiers, nCurrIntParam, theParam ) switch ((nCurrIntParam))\
        {\
          case 0:  (outputStr) += Format( (currFmtStr), (theParam) );    break;\
          case 1:  (outputStr) += Format( (currFmtStr), (intSpecifiers)[0], (theParam) );     break;\
          case 2:  (outputStr) += Format( (currFmtStr), (intSpecifiers)[0], (intSpecifiers)[1], (theParam) );  break;\
          default:       TCI_ASSERT(FALSE);\
          case 3:  (outputStr) += Format( (currFmtStr), (intSpecifiers)[0], (intSpecifiers)[1], (intSpecifiers)[2], (theParam) );  break;\
        }

TCIString StrUtil::stringWithParamsFormatter::format(const TCICHAR* formatString, ...) const
{
  TCIString formatStr(formatString);
  substringEnumerator subEnum = StartEnumSubstrings( formatStr, m_separator );
  TCIString simpleFormatStr;
  TCIString outputStr;
  TCI_VERIFY( GetNextSubstring( subEnum, simpleFormatStr ));

  va_list argList;
  va_start(argList, formatString);

//  stringSubsParam::paramType currType = stringSubsParam::unknownType;
  TCIString nextParam;
//  stringSubsParam* pParams = TCI_NEW( stringSubsParam[nParams] );
//  stringSubsParam currParam;

  TCI_BOOL bOkay = TRUE;
  int intSpecifiers[3] = {0,0,0};
  int nCurrIntParam = 0;
  int nCurrOffset = 0, nCurrStart = 0;  //offsets in "simpleFormatStr"
  const TCICHAR* pInitStr = (const TCICHAR*)simpleFormatStr;

  for (const TCICHAR *lpsz = (const TCICHAR*)simpleFormatStr; bOkay && *lpsz != '\0'; lpsz = _tcistrinc(lpsz))
  {
//    if (nCurrParam == nParams - 1)  //filled up our buffer!
//    {
//      nParams += m_startMaxParams;
//      stringSubsParam* pNewParams = TCI_NEW( stringSubsParam[nParams] );
//      for (int ix = 0; ix < nCurrParam; ++ix)
//        pNewParams[ix] == pParams[ix];
//      delete pParams;
//      pParams = pNewParams;
//    }
    bOkay = TRUE;
    nCurrIntParam = 0;

    // handle '%' character, but watch out for '%%'
    if (*lpsz != '%' || *(lpsz = _tcistrinc(lpsz)) == '%')
        continue;

    // handle '%' character with format
    TCI_BOOL bGoOn = FALSE, bWidthFromArgs = FALSE;
    for (; !bGoOn && *lpsz != '\0'; lpsz = _tcistrinc(lpsz))
    {

      // check for valid flags
      switch(*lpsz)
      {
        case '*':
          bOkay = GetNextSubstring( subEnum, nextParam );
          if (bOkay && nextParam.GetLength())
          {
            normalizeParamName( nextParam );
//            TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::intType ) );
            TCI_VERIFY( (*m_getParams.m_pGetInt)( intSpecifiers[nCurrIntParam++], nextParam, m_callbackData) );
          }
          else
//            pParams[nCurrParam++].setFromInt( va_arg(argList, int) );
            intSpecifiers[nCurrIntParam++] = va_arg( argList, int );
          bWidthFromArgs = TRUE;
        break;
        case '#':
        case '-':
        case '+':
        case '0':
        case ' ':
        break;
        default:
          bGoOn = TRUE;
        break;
      }
      if (bGoOn)
        break;
    }

    if (!bWidthFromArgs)  //skip width field if present
    {
      while( *lpsz != '\0' && _tciisdigit(*lpsz) )
        lpsz = _tcistrinc(lpsz);
    }

    int nPrecision = 0;
    if (*lpsz == '.')  //we have "precision" field
    {
      // skip past '.' separator (width.precision)
      lpsz = _tcistrinc(lpsz);

      // skip precision
      if (*lpsz == '*')
      {
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (bOkay && nextParam.GetLength())
        {
          normalizeParamName( nextParam );
//          TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::intType ) );
          TCI_VERIFY( (*m_getParams.m_pGetInt)( intSpecifiers[nCurrIntParam++], nextParam, m_callbackData) );
        }
        else
//          pParams[nCurrParam++].setFromInt( va_arg(argList, int) );
          intSpecifiers[nCurrIntParam++] = va_arg( argList, int );
        lpsz = _tcistrinc(lpsz);
      }
      else
      {
        while ( *lpsz != '\0' && _tciisdigit(*lpsz) )
          lpsz = _tcistrinc(lpsz);
      }
    }

    // should be on type modifier or specifier
    int nModifier = 0;
    switch (*lpsz)
    {
      // modifiers that affect size
      case 'h':
        nModifier = FORCE_ANSI;
        lpsz = _tcistrinc(lpsz);
      break;
      case 'l':
        nModifier = FORCE_UNICODE;
        lpsz = _tcistrinc(lpsz);
      break;

      // modifiers that do not affect size
      case 'F':
      case 'N':
      case 'L':
        lpsz = _tcistrinc(lpsz);
      break;
      default:
      break;
    }

    // now should be on specifier
    switch (*lpsz | nModifier)
    {
      // single characters
      case 'c':
      case 'C':
      case 'c'|FORCE_ANSI:
      case 'C'|FORCE_ANSI:
      {
        TCICHAR theChar = 0;
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (bOkay && nextParam.GetLength())
        {
          normalizeParamName( nextParam );
          TCI_VERIFY( (*m_getParams.m_pGetChar)( theChar, nextParam, m_callbackData ) );
//          TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::charType ) );
        }
        else
          theChar = va_arg( argList, TCICHAR );
//          pParams[nCurrParam++].setFromChar( va_arg(argList, TCICHAR) );
        nCurrOffset = lpsz - pInitStr;
        TCIString currFmtStr( simpleFormatStr.Mid( nCurrStart, nCurrOffset + 1 - nCurrStart ) );
        FMT_ADD_TO_OUTPUT_STR( outputStr, currFmtStr, intSpecifiers, nCurrIntParam, theChar )
        nCurrStart = nCurrOffset + 1;
//        switch (nCurrIntParam)
//        {
//          case 0:  outputStr += Format( currFmtStr, theChar );    break;
//          case 1:  outputStr += Format( currFmtStr, intSpecifiers[0], theChar );     break;
//          case 2:  outputStr += Format( currFmtStr, intSpecifiers[0], intSpecifiers[1], theChar );  break;
//          default:       TCI_ASSERT(FALSE);  //then fallthrough
//          case 3:  outputStr += Format( currFmtStr, intSpecifiers[0], intSpecifiers[1], intSpecifiers[2], theChar );  break;
//        }
      }
      break;
      case 'c'|FORCE_UNICODE:
      case 'C'|FORCE_UNICODE:
      {
        TCIWCHAR theWChar = 0;
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (bOkay && nextParam.GetLength())
        {
          normalizeParamName( nextParam );
          TCI_VERIFY( (*m_getParams.m_pGetWChar)( theWChar, nextParam, m_callbackData ) );
//          TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::wcharType ) );
        }
        else
//          pParams[nCurrParam++].setFromWChar( va_arg(argList, TCIWCHAR) );
          theWChar = va_arg( argList, TCIWCHAR );
        nCurrOffset = lpsz - pInitStr;
        TCIString currFmtStr( simpleFormatStr.Mid( nCurrStart, nCurrOffset + 1 - nCurrStart ) );
        FMT_ADD_TO_OUTPUT_STR( outputStr, currFmtStr, intSpecifiers, nCurrIntParam, theWChar )
        nCurrStart = nCurrOffset + 1;
      }
      break;

      // strings
      case 's':
      case 'S':
      case 's'|FORCE_ANSI:
      case 'S'|FORCE_ANSI:
      {
        TCIString theStr;
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (bOkay && nextParam.GetLength())
        {
          normalizeParamName( nextParam );
          TCI_VERIFY( getStringVal( m_getParams.m_pGetString, theStr, nextParam, m_callbackData ) );
//          TCI_VERIFY( (*m_getParams.m_pGetString)( theStr, nextParam, m_callbackData ) );
//          TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::stringType ) );
        }
        else
//          pParams[nCurrParam++].setFromString( TCIString( va_arg(argList, const TCICHAR*) ) );
          theStr = va_arg( argList, const TCICHAR* );
        nCurrOffset = lpsz - pInitStr;
        TCIString currFmtStr( simpleFormatStr.Mid( nCurrStart, nCurrOffset + 1 - nCurrStart ) );
        FMT_ADD_TO_OUTPUT_STR( outputStr, currFmtStr, intSpecifiers, nCurrIntParam, (const TCICHAR*)theStr )
        nCurrStart = nCurrOffset + 1;
      }
      break;

      // integers
      case 'd':
      case 'i':
      case 'o':
      {
        int theInt = 0;
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (bOkay && nextParam.GetLength())
        {
          normalizeParamName( nextParam );
          TCI_VERIFY( (*m_getParams.m_pGetInt)( theInt, nextParam, m_callbackData ) );
//          TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::intType ) );
        }
        else
//          pParams[nCurrParam++].setFromInt( va_arg(argList, int) );
          theInt = va_arg( argList, int );
        nCurrOffset = lpsz - pInitStr;
        TCIString currFmtStr( simpleFormatStr.Mid( nCurrStart, nCurrOffset + 1 - nCurrStart ) );
        FMT_ADD_TO_OUTPUT_STR( outputStr, currFmtStr, intSpecifiers, nCurrIntParam, theInt )
        nCurrStart = nCurrOffset + 1;
      }
      break;

      case 'u':
      case 'x':
      case 'X':
      {
        U32 theUInt = 0;
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (bOkay && nextParam.GetLength())
        {
          normalizeParamName( nextParam );
          TCI_VERIFY( (*m_getParams.m_pGetUInt)( theUInt, nextParam, m_callbackData ) );
//          TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::intType ) );
        }
        else
//          pParams[nCurrParam++].setFromInt( va_arg(argList, int) );
          theUInt = va_arg( argList, U32 );
        nCurrOffset = lpsz - pInitStr;
        TCIString currFmtStr( simpleFormatStr.Mid( nCurrStart, nCurrOffset + 1 - nCurrStart ) );
        FMT_ADD_TO_OUTPUT_STR( outputStr, currFmtStr, intSpecifiers, nCurrIntParam, theUInt )
        nCurrStart = nCurrOffset + 1;
      }
      break;

      case 'e':
      case 'f':
      case 'g':
      case 'G':
      {
        double theDouble = 0.0;
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (bOkay && nextParam.GetLength())
        {
          normalizeParamName( nextParam );
          TCI_VERIFY( (*m_getParams.m_pGetDouble)( theDouble, nextParam, m_callbackData ) );
//          TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::dblType ) );
        }
        else
//          pParams[nCurrParam++].setFromDouble( va_arg(argList, double) );
          theDouble = va_arg( argList, double );
        nCurrOffset = lpsz - pInitStr;
        TCIString currFmtStr( simpleFormatStr.Mid( nCurrStart, nCurrOffset + 1 - nCurrStart ) );
        FMT_ADD_TO_OUTPUT_STR( outputStr, currFmtStr, intSpecifiers, nCurrIntParam, theDouble )
        nCurrStart = nCurrOffset + 1;
      }
      break;

      case 'p':
      {
        void* thePtr = 0;
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (bOkay && nextParam.GetLength())
        {
          normalizeParamName( nextParam );
          TCI_VERIFY( (*m_getParams.m_pGetPointer)( thePtr, nextParam, m_callbackData ) );
//          TCI_VERIFY( setValueFromCallback( pParams[nCurrParam++], nextParam, stringSubsParam::ptrType ) );
        }
        else
//          pParams[nCurrParam++].setFromPtr( va_arg(argList, void*) );
          thePtr = va_arg( argList, void* );
        nCurrOffset = lpsz - pInitStr;
        TCIString currFmtStr( simpleFormatStr.Mid( nCurrStart, nCurrOffset + 1 - nCurrStart ) );
        FMT_ADD_TO_OUTPUT_STR( outputStr, currFmtStr, intSpecifiers, nCurrIntParam, thePtr )
        nCurrStart = nCurrOffset + 1;
      }
      break;

      // no output
      case 'n':
        bOkay = GetNextSubstring( subEnum, nextParam );
        if (!bOkay || !nextParam.GetLength())
          va_arg(argList, int*);
        nCurrOffset = lpsz - pInitStr;
        nCurrStart = nCurrOffset + 1;
      break;

      default:
        TCI_ASSERT(FALSE);  // unknown formatting option
      break;
    }
    TCI_ASSERT(bOkay);
  }

  va_end(argList);
  outputStr += simpleFormatStr.Mid( nCurrStart );

  return outputStr;

}

TCI_BOOL StrUtil::stringWithParamsFormatter::getStringVal( subsParamGetString pGetString, TCIString& theStr, const TCIString& paramName, void* pData )
{
  int nTheLen = 0;
  TCI_VERIFY( !(*pGetString)( (TCICHAR*)NULL, nTheLen, paramName, pData ) );
  TCI_ASSERT( nTheLen > 0);
  ++nTheLen;  //make room for null terminator
  TCICHAR* pBuff = theStr.GetBuffer( nTheLen );
  TCI_BOOL rv = (*pGetString)( pBuff, nTheLen, paramName, pData );
  TCI_ASSERT(rv);
  return rv;
}
