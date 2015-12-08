
#include "tcistrin.h"
#include "tcidebug.h"

#include <ctype.h>  // for islower, ...
#include <limits.h> // for INT_MAX
#include <string.h>



#ifdef ASSERT_HYPERACTIVE_STR
  #define ALLOC_WARNING_THRESHOLD  5
#endif

#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif


// For an empty string, m_???Data will point here
// (note: avoids a lot of NULL pointer tests when we call standard
//  C runtime libraries
// Warning:  accidentally changing this variable is possible

static TCICHAR ChNil = '\0';


void SafeDelete(TCICHAR *lpch)
{
  if (lpch != &ChNil)
    delete [] lpch;
}

int SafeStrlen(const TCICHAR *lpsz)
{
  return (lpsz == NULL) ? 0 : _tcistrlen(lpsz);
}


#ifdef TESTING
static TCI_BOOL IsValidTCIString(const TCICHAR *lpsz)
{
  return lpsz != 0;
}
#endif


inline TCI_BOOL _tciisspace(TCICHAR cp)
{
  return (cp==0x20 || (cp>=9 && cp<=0x0d));
}


inline TCICHAR *_tcistrinc(const TCICHAR *cp)
{ 
  return (TCICHAR *)(cp + 1);
}


int  EstimateLen(TCIString*, const TCICHAR *lpszFormat, va_list argList);



char *_tcistrupr(char *s)
{
  for (register char *p = s; *p; p++)
    if (islower(*p)) *p = toupper(*p);
  return s;
}

char *_tcistrlwr(char *s)
{
  for (register char *p = s; *p; p++)
    if (isupper(*p)) *p = tolower(*p);
  return s;
}

char *_tcistrrev(char *s)
{
  register int len = strlen(s);
  for (register int i = 0; i < len/2; i++)
  {
    register char tmp = s[i];
    s[i] = s[len - i - 1];
    s[len - i - 1] = tmp;
  }
  return s;
}



void TCIString::HelpCreate(const TCICHAR *lpsz)
{
  int nLen;

  if ((nLen = SafeStrlen(lpsz)) == 0)
    Init();
  else
  {
#ifdef ASSERT_HYPERACTIVE_STR
    m_nAllocCount =  0;
#endif
    AllocBuffer(nLen);
    memcpy(m_pchData, lpsz, nLen * sizeof(TCICHAR));
  }
}


TCIString::TCIString(const U8 *lpsz)
{
  HelpCreate((const TCICHAR *) lpsz);
}


TCIString::TCIString(const U8 * lpch, int nLength)
{
  if (nLength == 0)
    Init();
  else
  {
    TCI_ASSERT(lpch!=0);
#ifdef ASSERT_HYPERACTIVE_STR
    m_nAllocCount = 0;
#endif
    AllocBuffer(nLength);
    memcpy(m_pchData, lpch, nLength*sizeof(TCICHAR));
    m_pchData[nLength] = '\0';
  }

}



// for passing empty strings
const TCIString EmptyTCIString;


void TCIString::Init()
{
  m_nDataLength = m_nAllocLength = 0;
#ifdef ASSERT_HYPERACTIVE_STR
  m_nAllocCount = 0;
#endif
  m_pchData = &ChNil;
}


//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

TCIString::TCIString()
{
  Init();
}

TCIString::TCIString(const TCIString &stringSrc)
{
  // if constructing a TCIString from another TCIString, we make a copy of
  // the
  // original string data to enforce value semantics (i.e. each string
  // gets a copy of its own

#ifdef ASSERT_HYPERACTIVE_STR
  m_nAllocCount = 0;
#endif
  stringSrc.AllocCopy(*this, stringSrc.m_nDataLength, 0);
}

void TCIString::AllocBuffer(int nLen)
 // always allocate one extra character for '\0' termination
 // assumes [optimistically] that data length will equal allocation length
{
  TCI_ASSERT(nLen >= 0);
  TCI_ASSERT(nLen <= INT_MAX - 1);  // max size (enough room for 1 extra)

  if (nLen == 0)
  {
    Init();
  }
  else
  {
    //m_pchData = new TCICHAR[nLen + 1]; // may throw an exception
    m_pchData =  TCI_NEW( TCICHAR[nLen + 1] ); // may throw an exception
    m_pchData[nLen] = '\0';
    m_nDataLength = nLen;
    m_nAllocLength = nLen;

#ifdef ASSERT_HYPERACTIVE_STR
    m_nAllocCount++;
    TCI_ASSERT( m_nAllocCount<ALLOC_WARNING_THRESHOLD );
    if ( m_nAllocCount >= ALLOC_WARNING_THRESHOLD )
      m_nAllocCount =  0;
#endif

  }
}

void TCIString::Empty()
{
  SafeDelete(m_pchData);
  Init();
  TCI_ASSERT(m_nDataLength == 0);
  TCI_ASSERT(m_nAllocLength == 0);
}

TCIString::~TCIString()
 // free any attached data
{
  SafeDelete(m_pchData);
}

//////////////////////////////////////////////////////////////////////////////
// Helpers for the rest of the implementation
//////////////////////////////////////////////////////////////////////////////

void TCIString::AllocCopy(TCIString &dest, int nCopyLen, int nCopyIndex) const
{
  // will clone the data attached to this string
  // allocating 'nExtraLen' characters
  // Places results in uninitialized string 'dest'
  // Will copy the part or all of original data to start of new string


  if (nCopyLen == 0)
    dest.Init();
  else
  {
    dest.AllocBuffer(nCopyLen);
    memcpy(dest.m_pchData, &m_pchData[nCopyIndex], nCopyLen * sizeof(TCICHAR));
  }
}

//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction
//////////////////////////////////////////////////////////////////////////////

TCIString::TCIString(const TCICHAR *lpsz)
{
  int nLen;

  if ((nLen = SafeStrlen(lpsz)) == 0)
    Init();
  else
  {
#ifdef ASSERT_HYPERACTIVE_STR
    m_nAllocCount = 0;
#endif
    AllocBuffer(nLen);
    memcpy(m_pchData, lpsz, nLen * sizeof(TCICHAR));
  }
}


//////////////////////////////////////////////////////////////////////////////
// Assignment operators
//  All assign a new value to the string
//      (a) first see if the buffer is big enough
//      (b) if enough room, copy on top of old buffer, set size and type
//      (c) otherwise free old string data, and create a new one
//
//  All routines return the new string (but as a 'const TCIString&' so that
//      assigning it again will cause a copy, eg: s1 = s2 = "hi there".
//
//////////////////////////////////////////////////////////////////////////////

void TCIString::AssignCopy(int nSrcLen, const TCICHAR *lpszSrcData)
{
  // check if it will fit
  if (nSrcLen > m_nAllocLength)
  {
    // it won't fit, allocate another one
    Empty();
    AllocBuffer(nSrcLen);
  }
  if (nSrcLen != 0)
    memcpy(m_pchData, lpszSrcData, nSrcLen * sizeof(TCICHAR));
  m_nDataLength = nSrcLen;
  m_pchData[nSrcLen] = '\0';
}


const TCIString &TCIString::operator = (const TCIString &stringSrc)
{
  AssignCopy(stringSrc.m_nDataLength, stringSrc.m_pchData);
  return *this;
}

const TCIString &TCIString::operator = (const char *lpsz)
{
  TCI_ASSERT(lpsz == NULL || IsValidTCIString(lpsz));
  AssignCopy(SafeStrlen(lpsz), lpsz);
  return *this;
}

/////////////////////////////////////////////////////////////////////////////
// Special conversion assignment
/////////////////////////////////////////////////////////////////////////////

#ifdef _UNICODE
const TCIString &TCIString::operator = (const char * lpsz)
{
  int nSrcLen = lpsz != NULL ? _tcistrlenA(lpsz) : 0;

  // check if it will fit
  if (nSrcLen > m_nAllocLength)
  {
    // it won't fit, allocate another one
    Empty();
    AllocBuffer(nSrcLen);
  }
  if (nSrcLen != 0)
    _mbstowcsz(m_pchData, lpsz, nSrcLen + 1);
  m_nDataLength = nSrcLen;
  m_pchData[nSrcLen] = '\0';
  return *this;
}

#else //!_UNICODE

#if 0
const TCIString &TCIString::operator = (TCIWCHAR *lpsz)
{
  int nSrcLen = lpsz != NULL ? wcslen(lpsz) : 0;

  // check if it will fit
  if (nSrcLen > m_nAllocLength)
  {
    // it won't fit, allocate another one
    Empty();
    AllocBuffer(nSrcLen);
  }
  if (nSrcLen != 0)
    TCI_ASSERT(FALSE);
  // Need a function:
  // _wcstombsz(m_pchData, lpsz, nSrcLen+1);
  m_nDataLength = nSrcLen;
  m_pchData[nSrcLen] = '\0';
  return *this;
}
#endif

#endif	//!_UNICODE

//////////////////////////////////////////////////////////////////////////////
// Concatenation
//////////////////////////////////////////////////////////////////////////////

// NOTE: "operator+" is done as friend functions for simplicity
//      There are three variants:
//          TCIString + TCIString
// and for ? = TCICHAR, const TCICHAR *
//          TCIString + ?
//          ? + TCIString

void TCIString::ConcatCopy(int nSrc1Len, const TCICHAR *lpszSrc1Data,
                           int nSrc2Len, const TCICHAR *lpszSrc2Data)
{
  // -- master concatenation routine
  // Concatenate two sources
  // -- assume that 'this' is a new TCIString object

  int nNewLen = nSrc1Len + nSrc2Len;

  AllocBuffer(nNewLen);
  memcpy(m_pchData, lpszSrc1Data, nSrc1Len * sizeof(TCICHAR));
  memcpy(&m_pchData[nSrc1Len], lpszSrc2Data, nSrc2Len * sizeof(TCICHAR));
}

TCIString operator + (const TCIString &string1, const TCIString &string2)
{
  TCI_TRACE("CALLED: operator + (const TCIString &, const TCIString &)");

  TCIString s;

  s.ConcatCopy(string1.m_nDataLength, string1.m_pchData,
               string2.m_nDataLength, string2.m_pchData);
  return s;
}


TCIString operator + (const TCIString &string, const TCICHAR *lpsz)
{
  TCI_TRACE("CALLED: operator + (const TCIString &, const TCICHAR *)");
  TCI_ASSERT(lpsz == NULL || IsValidTCIString(lpsz));

  TCIString s;
  s.ConcatCopy(string.m_nDataLength, string.m_pchData, SafeStrlen(lpsz), lpsz);
  return s;
}

TCIString operator + (const TCICHAR *lpsz, const TCIString &string)
{
  TCI_TRACE("CALLED: operator + (const TCICHAR *, const TCIString &)");

  TCI_ASSERT(lpsz == NULL || IsValidTCIString(lpsz));
  TCIString s;

  s.ConcatCopy(SafeStrlen(lpsz), lpsz, string.m_nDataLength, string.m_pchData);
  return s;
}

TCIString operator + (TCICHAR ch, const TCIString &string)
{
  TCI_TRACE("CALLED: operator + (TCICHAR, const TCIString &)");

  TCIString s;
  s.ConcatCopy(1, &ch, string.m_nDataLength, string.m_pchData);
  return s;
}

TCIString operator + (const TCIString &string, TCICHAR ch)
{
  TCI_TRACE("CALLED: operator + (const TCIString &, TCICHAR)");

  TCIString s;
  s.ConcatCopy(string.m_nDataLength, string.m_pchData, 1, &ch);
  return s;
}


//////////////////////////////////////////////////////////////////////////////
// Concatenate in place
//////////////////////////////////////////////////////////////////////////////

void TCIString::ConcatInPlace(int nSrcLen, const TCICHAR *lpszSrcData)
{
  // -- the main routine for += operators

  if (nSrcLen <= 0)
    return;

  // if the buffer is too small, or we have a width mis-match, just
  // allocate a new buffer (slow but sure)
  if (m_nDataLength + nSrcLen > m_nAllocLength)
  {
    // we have to grow the buffer, use the Concat in place routine
    TCICHAR *lpszOldData = m_pchData;

    ConcatCopy(m_nDataLength, lpszOldData, nSrcLen, lpszSrcData);
    TCI_ASSERT(lpszOldData != NULL);
    SafeDelete(lpszOldData);
  }
  else
  {
    // fast concatenation when buffer big enough
    memcpy(&m_pchData[m_nDataLength], lpszSrcData, nSrcLen * sizeof(TCICHAR));
    m_nDataLength += nSrcLen;
  }
  TCI_ASSERT(m_nDataLength <= m_nAllocLength);
  m_pchData[m_nDataLength] = '\0';
}

const TCIString &TCIString::operator += (const TCICHAR *lpsz)
{
  TCI_TRACE("CALLED: operator += (const TCICHAR *)");

  TCI_ASSERT(lpsz == NULL || IsValidTCIString(lpsz));
  ConcatInPlace(SafeStrlen(lpsz), lpsz);
  return *this;
}

const TCIString &TCIString::operator += (const TCIString &string)
{
  TCI_TRACE("CALLED: operator += (const TCIString &)");

  ConcatInPlace(string.m_nDataLength, string.m_pchData);
  return *this;
}

const TCIString &TCIString::operator += (TCICHAR ch)
{
  TCI_TRACE("CALLED: operator += (TCICHAR)");

  ConcatInPlace(1, &ch);
  return *this;
}


///////////////////////////////////////////////////////////////////////////////
// Advanced direct buffer access
///////////////////////////////////////////////////////////////////////////////

TCICHAR *TCIString::GetBuffer(int nMinBufLength)
{
  TCI_ASSERT(nMinBufLength >= 0);

  if (nMinBufLength > m_nAllocLength)
  {
    // we have to grow the buffer
    TCICHAR *lpszOldData = m_pchData;
    int nOldLen = m_nDataLength;// AllocBuffer will tromp it

    AllocBuffer(nMinBufLength);
    memcpy(m_pchData, lpszOldData, nOldLen * sizeof(TCICHAR));
    m_nDataLength = nOldLen;
    m_pchData[m_nDataLength] = '\0';

    SafeDelete(lpszOldData);
  }

  // return a pointer to the character storage for this string
  TCI_ASSERT(m_pchData != NULL);
  return m_pchData;
}

void TCIString::ReleaseBuffer(int nNewLength)
{
  if (nNewLength == -1)
    nNewLength = strlen(m_pchData); // zero ter_tciminated

  TCI_ASSERT(nNewLength <= m_nAllocLength);
  m_nDataLength = nNewLength;
  m_pchData[m_nDataLength] = '\0';
}

TCICHAR *TCIString::GetBufferSetLength(int nNewLength)
{
  TCI_ASSERT(nNewLength >= 0);

  GetBuffer(nNewLength);
  m_nDataLength = nNewLength;
  m_pchData[m_nDataLength] = '\0';
  return m_pchData;
}

void TCIString::FreeExtra()
{
  TCI_ASSERT(m_nDataLength <= m_nAllocLength);
  if (m_nDataLength != m_nAllocLength)
  {
    TCICHAR *lpszOldData = m_pchData;

    AllocBuffer(m_nDataLength);
    memcpy(m_pchData, lpszOldData, m_nDataLength * sizeof(TCICHAR));
    TCI_ASSERT(m_pchData[m_nDataLength] == '\0');
    SafeDelete(lpszOldData);
  }
  TCI_ASSERT(m_pchData != NULL);
}

//////////////////////////////////////////////////////////////////////////////
// Very simple sub-string extraction
//////////////////////////////////////////////////////////////////////////////

TCIString TCIString::Mid(int nFirst) const
{
  return Mid(nFirst, m_nDataLength - nFirst);
}

TCIString TCIString::Mid(int nFirst, int nCount) const
{
  TCI_ASSERT(nFirst >= 0);
  TCI_ASSERT(nCount >= 0);

  // out-of-bounds requests return sensible things
  if (nFirst < 0)
    nFirst = 0;
  if (nCount < 0)
    nCount = 0;

  if (nFirst + nCount > m_nDataLength)
    nCount = m_nDataLength - nFirst;
  if (nFirst > m_nDataLength)
    nCount = 0;

  TCIString dest;
  AllocCopy(dest, nCount, nFirst);
  return dest;
}

TCIString TCIString::Right(int nCount) const
{
  TCI_ASSERT(nCount >= 0);

  if (nCount > m_nDataLength)
    nCount = m_nDataLength;
  else if (nCount < 0)
    nCount = 0;             // let's be defensive

  TCIString dest;
  AllocCopy(dest, nCount, m_nDataLength-nCount);
  return dest;
}

TCIString TCIString::Left(int nCount) const
{
  TCI_ASSERT(nCount >= 0);

  if (nCount > m_nDataLength)
    nCount = m_nDataLength;
  else if (nCount < 0)
    nCount = 0;             // let's be defensive

  TCIString dest;
  AllocCopy(dest, nCount, 0);
  return dest;
}

// strspn equivalent
TCIString TCIString::SpanIncluding(const TCICHAR *lpszCharSet) const
{
  TCI_ASSERT(IsValidTCIString(lpszCharSet));
  return Left(_tcistrspn(m_pchData, lpszCharSet));
}

// strcspn equivalent
TCIString TCIString::SpanExcluding(const TCICHAR *lpszCharSet) const
{
  TCI_ASSERT(IsValidTCIString(lpszCharSet));
  return Left(_tcistrcspn(m_pchData, lpszCharSet));
}

//////////////////////////////////////////////////////////////////////////////
// Find functions.
//////////////////////////////////////////////////////////////////////////////

int TCIString::Find(TCICHAR ch, int startPos) const
{
  if (startPos && startPos < 0 && startPos >= SafeStrlen(m_pchData))
    return -1;

  // find first single character
  TCICHAR *lpsz = _tcistrchr(&m_pchData[startPos], ch);

  // return -1 if not found and index otherwise
  return (lpsz == NULL) ? -1 : (int) (lpsz - m_pchData);
}

int TCIString::FindNoCase(TCICHAR ch, int startPos) const
{
  if (startPos && startPos < 0 && startPos >= SafeStrlen(m_pchData))
    return -1;

  // find first single character
  int pos2 = -1;
  TCICHAR* lpsz  = _tcistrchr(&m_pchData[startPos], ch);
  int pos1 = (lpsz == NULL) ? -1 : static_cast<int>(lpsz - m_pchData);
  if (isalpha(ch))  // check opposite case
  {
    TCICHAR newch =  isupper(ch) ? tolower(ch) : toupper(ch);
    lpsz =  _tcistrchr(&m_pchData[startPos], newch);
    pos2 = (lpsz == NULL) ? -1 : static_cast<int>(lpsz - m_pchData);
  }

  // return -1 if not found and index otherwise
  if (CHAMmin(pos1,pos2) >= 0)
    return(CHAMmin(pos1,pos2));
  else
    return(CHAMmax(pos1,pos2));
}

int TCIString::ReverseFind(TCICHAR ch) const
{
  // find last single character
  TCICHAR *lpsz = _tcistrrchr(m_pchData, ch);

  // return -1 if not found, distance from beginning otherwise
  return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

// find a sub-string (like strstr)
int TCIString::Find(const TCICHAR *sub, int startPos) const
{
  TCI_ASSERT(IsValidTCIString(sub));

  if (startPos && (startPos < 0 || startPos >= SafeStrlen(m_pchData)))
    return -1;

  // find first matching substring
  TCICHAR *lpsz = _tcistrstr(&m_pchData[startPos], sub);

  // return -1 for not found, distance from beginning otherwise
  return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

// find a sub-string (like strstr)
int TCIString::FindNoCase(const TCICHAR *sub, int startPos) const
{
  TCI_ASSERT(IsValidTCIString(sub));

  int sublen =  _tcistrlen(sub);
  if (startPos && (startPos < 0 || startPos+sublen >= SafeStrlen(m_pchData)))
    return -1;

  // try and find no case
  TCICHAR ch =  sub[0];
  int pos =  FindNoCase(ch, startPos);
  int rv =  -1;
  while ((pos >= startPos) && (pos+sublen <= m_nDataLength))
  {
    if (!_tcistrnicmp(&m_pchData[pos], sub, sublen))
	  {
		  rv =  pos;
        break;
	  }
    pos =  FindNoCase(ch, pos+1);
  }
  return rv;
}

int TCIString::FindOneOf(const TCICHAR *lpszCharSet, int startPos) const
{
  if (startPos && startPos < 0 && startPos >= SafeStrlen(m_pchData))
    return -1;

  TCI_ASSERT(IsValidTCIString(lpszCharSet));
  TCICHAR *lpsz = _tcistrpbrk(m_pchData + startPos, lpszCharSet);

  return (lpsz == NULL) ? -1 : (int) (lpsz - m_pchData);
}

//
// Replaces every occurrence of 'pat' with 'repl'.
// The matching is case sensitive.
//


int TCIString::Replace(const TCICHAR *pat, const TCICHAR *repl, int startPos)
{
  TCI_ASSERT(IsValidTCIString(pat));
  TCI_ASSERT(IsValidTCIString(repl));

  if (startPos && startPos < 0 && startPos >= SafeStrlen(m_pchData))
    return 0;

  int lenPat = strlen(pat);

  if (!lenPat)
    return 0;

  int lenRepl = strlen(repl);
  int sizeRepl = lenRepl * sizeof(TCICHAR);
  int count = 0;

  if (lenRepl == lenPat) {
    TCICHAR* next = &m_pchData[startPos];
    while ((next = _tcistrstr(next, pat)) != NULL) {
      memcpy(next, repl, sizeRepl);
      next += lenPat;
      ++count;
    }
    return count;
  }
  else {
    // determine the size of buffer needed
    TCICHAR* next = &m_pchData[startPos];
    while ((next = _tcistrstr(next, pat)) != NULL) {
      next += lenPat;
      ++count;
    }
    
    if (count == 0)
      return 0;

    // Alloc new destination buffer
    TCICHAR* srcBase = m_pchData;
    int lenSrc = m_nDataLength;
    int need = m_nDataLength + (lenRepl - lenPat) * count;
    AllocBuffer(need);
    TCICHAR* dstBase = m_pchData;

    // Copy characters before startPos
    TCICHAR* src;
    TCICHAR* dst;
    if (startPos) {
      memcpy(dstBase, srcBase, startPos * sizeof(TCICHAR));
      src = srcBase + startPos;
      dst = dstBase + startPos; 
    }
    else {
      src = srcBase;
      dst = dstBase;
    }

    // Replace occurrences of pattern while copying the rest
    while ((next = _tcistrstr(src, pat)) != NULL) {
      int noElements = next - src;
      memcpy(dst, src, noElements * sizeof(TCICHAR));
      dst += noElements;
      memcpy(dst, repl, sizeRepl);
      dst += lenRepl;
      src = next + lenPat;
    }
    memcpy(dst, src, (lenSrc-(src-srcBase)) * sizeof(TCICHAR));

    SafeDelete(srcBase);
    return count;
  }
}
    



//
// The following returns the zero-based index of the first character
// in this string that is not contained in the specified character set.
//
int TCIString::NotMember(const TCICHAR *lpszCharSet, int startPos) const
{
  if (startPos && startPos < 0 && startPos >= SafeStrlen(m_pchData))
    return -1;

  TCI_ASSERT(IsValidTCIString(lpszCharSet));
  int index = _tcistrspn(m_pchData + startPos, lpszCharSet) + startPos;
  return (index < SafeStrlen(m_pchData)) ? index : -1;
}

TCI_BOOL TCIString::Contains(const TCICHAR *lpszCharSet) const
{
  TCI_ASSERT(IsValidTCIString(lpszCharSet));
  return !(_tcistrlen(lpszCharSet) > _tcistrspn(lpszCharSet, m_pchData));
}

TCI_BOOL TCIString::ContainedIn(const TCICHAR *lpszCharSet) const
{
  TCI_ASSERT(IsValidTCIString(lpszCharSet));
  return !((size_t)SafeStrlen(m_pchData) > _tcistrspn(m_pchData, lpszCharSet));
}



void TCIString::TrimRight()
{
	// find beginning of trailing spaces by starting at beginning (DBCS aware)
	TCICHAR *lpsz = m_pchData;
	TCICHAR *lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (_tciisspace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
			lpszLast = NULL;
		lpsz = _tcistrinc(lpsz);
	}

	if (lpszLast != NULL)
	{
		// truncate at trailing space start
		*lpszLast = '\0';
		m_nDataLength = lpszLast - m_pchData;
	}
}

void TCIString::TrimLeft()
{
	// find first non-space character
	const TCICHAR *lpsz = m_pchData;
	while (_tciisspace(*lpsz))
		lpsz = _tcistrinc(lpsz);

	// fix up data and length
	int nDataLength = m_nDataLength - (lpsz - m_pchData);
	memmove(m_pchData, lpsz, nDataLength + 1);
	m_nDataLength = nDataLength;
}


///////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// More sophisticated construction

TCIString::TCIString(TCICHAR ch, int nLength)
{
  if (nLength < 1)
  {
    // return empty string if invalid repeat count
    Init();
  }
  else
  {
#ifdef ASSERT_HYPERACTIVE_STR
    m_nAllocCount = 0;
#endif
    AllocBuffer(nLength);
    memset(m_pchData, ch, nLength);
  }
}

TCIString::TCIString(const TCICHAR* lpch, int nLength)
{
  if (nLength <= 0 || lpch == NULL || *lpch == 0)
    Init();
  else
  {
    TCI_ASSERT(IsValidTCIString(lpch));
#ifdef ASSERT_HYPERACTIVE_STR
    m_nAllocCount = 0;
#endif
    AllocBuffer(nLength);
    memcpy(m_pchData, lpch, m_nDataLength*sizeof(TCICHAR));
    m_pchData[m_nDataLength] = '\0';
    m_nDataLength = CHAMmin(m_nDataLength,SafeStrlen(m_pchData));
  }
}
