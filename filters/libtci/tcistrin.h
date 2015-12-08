#ifndef   TCISTRING_H
#define   TCISTRING_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif
#ifndef TCI_NEW_H
  #include "TCI_new.h"
#endif

#ifndef STD_STRING_H
  #include <string.h>
  #define STD_STRING_H
#endif

#ifndef STD_STDARG_H
  #include <stdarg.h>
  #define STD_STDARG_H
#endif

//#define ASSERT_HYPERACTIVE_STR


class TCIString
{
 // MEMBER DATA
 private:
   // note: an extra character is always allocated for the terminating zero-byte
   int m_nDataLength;            // does not include terminating 0
   int m_nAllocLength;           // does not include terminating 0
   TCICHAR *m_pchData;       // actual string (zero terminated)
   #ifdef ASSERT_HYPERACTIVE_STR
     int m_nAllocCount;    // JBM a diagnostic only!!
   #endif

 // PUBLIC INTERFACE
 public:
   // CONSTRUCTION

   TCIString();
   TCIString(const TCIString &);        
   TCIString(TCICHAR, int nLength);    // remove default value for type security
   TCIString(const TCICHAR *);         
   TCIString(const TCICHAR *, int nLength);         
   TCIString(TCIWCHAR *);
   TCIString(const U8 *lpsz);
   TCIString(const U8 *, int nLength);
   ~TCIString();
   
   
   // ASSIGNMENT
   const TCIString &operator = (const TCIString &stringSrc);
   const TCIString &operator = (const TCICHAR *lpsz);
   const TCIString &operator = (const TCIWCHAR *lpsz);
   inline const TCIString &operator = (const U8 *lpsz);
   
   // CONVERSION
   inline operator const TCICHAR *() const;
   inline operator const U8 *() const;  // Convert to a U8 *
   
   // ACCESS AND MANIPULATION

   inline int      GetLength() const;
   inline int      GetAllocLength() const;  
   inline TCI_BOOL IsEmpty() const;
          void     Empty();  
   inline TCICHAR  GetAt(int nIndex) const;
   inline void     SetAt(int nIndex, TCICHAR ch) ;
   inline TCICHAR  operator[] (int nIndex) const;




   // Concatenation
   const TCIString &operator += (TCICHAR ch);
   const TCIString &operator += (const TCICHAR *lpsz);
   const TCIString &operator += (const TCIString &string);
   inline void AppendChars(const TCICHAR *lpch,int numbytes);
   

   //
   // The friend operators should take non-const (TCICHAR *)
   // so that calls with unnamed string pointers work, like:
   // TCIString str; str + "unnamed pointer";
   //
   // So leave the friend operators de-constified!
   //
   friend TCIString operator + (const TCIString &string1, const TCIString &string2);
   friend TCIString operator + (const TCIString &string, const TCICHAR *lpsz);
   friend TCIString operator + (const TCICHAR *lpsz, const TCIString &string);
   friend TCIString operator + (TCICHAR ch, const TCIString &string);
   friend TCIString operator + (const TCIString &string, TCICHAR ch);

   // Comparison
   inline int Compare(const TCICHAR *lpsz) const;
   inline int Compare(const TCICHAR *lpsz, int count) const;
   inline int CompareNoCase(const TCICHAR *lpsz) const;
   inline int CompareNoCase(const TCICHAR *lpsz, int count) const;
   // Collate is often slower than Compare but is MBSC/Unicode
   // aware as well as locale-sensitive with respect to sort order.
   inline int Collate(const TCICHAR *lpsz) const;
   inline friend TCI_BOOL operator == (const TCIString &s1,const TCIString &s2);
   inline friend TCI_BOOL operator == (const TCIString &s1, const TCICHAR *s2);
   inline friend TCI_BOOL operator == (const TCICHAR *s1, const TCIString &s2);
   inline friend TCI_BOOL operator != (const TCIString &s1,const TCIString &s2);
   inline friend TCI_BOOL operator != (const TCICHAR *s1, const TCIString &s2);
   inline friend TCI_BOOL operator != (const TCIString& s1, const TCICHAR* s2);
   inline friend TCI_BOOL operator <  (const TCIString &s1, const TCIString &s2);
   inline friend TCI_BOOL operator <  (const TCIString &s1, const TCICHAR *s2);
   inline friend TCI_BOOL operator <  (const TCICHAR *s1, const TCIString &s2);
   inline friend TCI_BOOL operator >  (const TCIString &s1, const TCIString &s2);
   inline friend TCI_BOOL operator >  (const TCIString &s1, const TCICHAR *s2);
   inline friend TCI_BOOL operator >  (const TCICHAR *s1, const TCIString &s2);
   inline friend TCI_BOOL operator <= (const TCIString &s1,const TCIString &s2);
   inline friend TCI_BOOL operator <= (const TCIString &s1, const TCICHAR *s2);
   inline friend TCI_BOOL operator <= (const TCICHAR *s1, const TCIString &s2);
   inline friend TCI_BOOL operator >= (const TCIString &s1, const TCIString &s2);
   inline friend TCI_BOOL operator >= (const TCIString &s1, const TCICHAR *s2);
   inline friend TCI_BOOL operator >= (const TCICHAR *s1, const TCIString &s2);
   

   //
   // Simple sub-string extraction.
   //
   TCIString Mid(int nFirst, int nCount) const;
   TCIString Mid(int nFirst) const;
   TCIString Left(int nCount) const;
   TCIString Right(int nCount) const;
   TCIString SpanIncluding(const TCICHAR *lpszCharSet) const;
   TCIString SpanExcluding(const TCICHAR *lpszCharSet) const;

   // upper/lower/reverse conversion

   inline void MakeUpper();
   inline void MakeLower();
   inline void MakeReverse();

   // trimming whitespace (either side)
   void TrimRight();
   void TrimLeft();
   inline void Trim();

   // searching (return starting index, or -1 if not found)
   // look for a single character match
   int Find(TCICHAR ch, int startPos = 0) const;           // like "C" strchr
   int FindNoCase(TCICHAR ch, int startPos = 0) const;
   int Find(const TCICHAR *sub, int startPos = 0) const;   // like "C" strstr
   int FindNoCase(const TCICHAR *sub, int startPos = 0) const;
   int ReverseFind(TCICHAR ch) const;                      // like "C" strrchr
   int FindOneOf(const TCICHAR *charSet, int startPos = 0) const;  // like "C" strcspn
   int NotMember(const TCICHAR *charSet, int startPos = 0) const;  // like "C" strspn
   TCI_BOOL Contains(const TCICHAR *charSet) const;
   TCI_BOOL ContainedIn(const TCICHAR *charSet) const;

   // replace functions
   int Replace(const TCICHAR *pat, const TCICHAR *repl, int startPos = 0);
   inline int Delete(const TCICHAR *pat, int startPos = 0);

   // Access to string implementation buffer as "C" character array
   TCICHAR *GetBuffer(int nMinBufLength);
   void ReleaseBuffer(int nNewLength = -1);
   TCICHAR *GetBufferSetLength(int nNewLength);
   void FreeExtra();

   void Truncate(int newLen) { TCI_ASSERT(newLen <= m_nDataLength); m_nDataLength = newLen; m_pchData[m_nDataLength] = 0; }
   void SetDataLength(int len) { m_nDataLength = len; }

private:
   // implementation helpers
   void HelpCreate(const TCICHAR *lpsz);
  
   void Init();
   void AllocCopy(TCIString &dest, int nCopyLen, int nCopyIndex) const;
   void AllocBuffer(int nLen);
   void AssignCopy(int nSrcLen, const TCICHAR *lpszSrcData);
   void ConcatCopy(int nSrc1Len, const TCICHAR *lpszSrc1Data, 
                   int nSrc2Len, const TCICHAR *lpszSrc2Data);
   void ConcatInPlace(int nSrcLen, const TCICHAR *lpszSrcData);
};



extern const TCIString EmptyTCIString;   

int TCIString::GetLength() const {return m_nDataLength;}

int TCIString::GetAllocLength() const{
   return m_nAllocLength;
}


TCI_BOOL TCIString::IsEmpty() const{return m_nDataLength == 0;}

TCICHAR TCIString::GetAt(int nIndex) const
{
  TCI_ASSERT(nIndex >= 0);
  TCI_ASSERT(nIndex < m_nDataLength);
  return m_pchData[nIndex];
}

TCICHAR TCIString::operator[] (int nIndex) const
{
  // same as GetAt
  TCI_ASSERT(nIndex >= 0);
  TCI_ASSERT(nIndex < m_nDataLength);
  return m_pchData[nIndex];
}

void TCIString::SetAt(int nIndex, TCICHAR ch)
{
  TCI_ASSERT(nIndex >= 0);
  TCI_ASSERT(nIndex < m_nDataLength);
  TCI_ASSERT(ch != 0);
  m_pchData[nIndex] = ch;
}

int TCIString::Delete(const TCICHAR *pat, int startPos){
   return Replace(pat, "", startPos);
}

const TCIString& TCIString::operator = (const U8 *lpsz)
{
  *this = (TCICHAR *) lpsz;
  return *this;
}


TCIString::operator const TCICHAR *() const                // Convert to a TCICHAR *
{
  return m_pchData;
}

TCIString::operator const U8 *() const                     // Convert to a U8 *
{
  return (U8 *) m_pchData;                // We need to get rid of this.
}





#define _tcistrlen   strlen
#define _tcistrcmp   strcmp
#define _tcistricmp  stricmp
#define _tcistrncmp  strncmp
#define _tcistrnicmp _strnicmp
#define _tcistrcoll  strcoll
#define _tcistrchr   strchr
#define _tcistrrchr  strrchr
#define _tcistrstr   strstr
#define _tcistrpbrk  strpbrk
#define _tcistrspn   strspn
#define _tcistrcspn  strcspn
#define _tciatoi     atoi
#define _tciisdigit  isdigit
//#define _tciisspace isspace - this call appears to screw up characters above 0x80? ron
#define _tcivsprintf vsprintf
#define _tciatoi     atoi
#define _tciatof     atof


//
// Defined in tcistrin.cc
// On PCs these functions are not standard.
// They only seem to be defined in VC++ library.
// 
char *_tcistrupr(char *s);
char *_tcistrlwr(char *s);
char *_tcistrrev(char *s);

void TCIString::MakeUpper(){_tcistrupr(m_pchData);}
void TCIString::MakeLower(){_tcistrlwr(m_pchData);}
void TCIString::MakeReverse(){_tcistrrev(m_pchData);}
int TCIString::Compare(const TCICHAR *lpsz) const{ 
  return _tcistrcmp(m_pchData, lpsz); 
}

int TCIString::Compare(const TCICHAR *lpsz, int count) const{
  return _tcistrncmp(m_pchData, lpsz, count);
}

int TCIString::CompareNoCase(const TCICHAR *lpsz) const{
  return _tcistricmp(m_pchData, lpsz);
}

int TCIString::CompareNoCase(const TCICHAR *lpsz, int count) const{
  return _tcistrnicmp(m_pchData, lpsz, count);
}

int TCIString::Collate(const TCICHAR *lpsz) const{
  return _tcistrcoll(m_pchData, lpsz);  // locale sensitive
}

void TCIString::Trim() { 
  TrimLeft(); TrimRight(); 
}

void TCIString::AppendChars(const TCICHAR *lpch,int numbytes){ 
  ConcatInPlace(numbytes,lpch);
}


     
TCI_BOOL operator == (const TCIString &s1, const TCIString &s2){
    return s1.Compare(s2) == 0;
}

TCI_BOOL operator == (const TCIString &s1, const TCICHAR *s2){
    return s1.Compare(s2) == 0;
}

TCI_BOOL operator == (const TCICHAR *s1, const TCIString &s2){
    return s2.Compare(s1) == 0;
}

TCI_BOOL operator != (const TCIString &s1, const TCIString &s2){
    return s1.Compare(s2) != 0;
}

TCI_BOOL operator != (const TCIString &s1, const TCICHAR *s2){
    return s1.Compare(s2) != 0;
}

TCI_BOOL operator != (const TCICHAR* s1, const TCIString& s2){
    return s2.Compare(s1) != 0;
}

TCI_BOOL operator < (const TCIString &s1,  const TCIString &s2){
    return s1.Compare(s2) < 0;
}

TCI_BOOL operator < (const TCIString &s1, const TCICHAR *s2){
    return s1.Compare(s2) < 0;
}

TCI_BOOL operator < (const TCICHAR *s1, const TCIString &s2) {
    return s2.Compare(s1) > 0;
}

TCI_BOOL operator > (const TCIString &s1, const TCIString &s2) {
    return s1.Compare(s2) > 0;
}

TCI_BOOL operator > (const TCIString &s1, const TCICHAR *s2) {
    return s1.Compare(s2) > 0;
}

TCI_BOOL operator > (const TCICHAR *s1, const TCIString &s2) {
    return s2.Compare(s1) < 0;
}

TCI_BOOL operator <= (const TCIString &s1,  const TCIString &s2) {
    return s1.Compare(s2) <= 0;
}

TCI_BOOL operator <= (const TCIString &s1, const TCICHAR *s2){
    return s1.Compare(s2) <= 0;
}

TCI_BOOL operator <= (const TCICHAR *s1, const TCIString &s2){
    return s2.Compare(s1) >= 0;
}

TCI_BOOL operator >= (const TCIString &s1, const TCIString &s2) {
    return s1.Compare(s2) >= 0;
}

TCI_BOOL operator >= (const TCIString &s1, const TCICHAR *s2) {
    return s1.Compare(s2) >= 0;
}

TCI_BOOL operator >= (const TCICHAR *s1, const TCIString &s2) {
    return s2.Compare(s1) <= 0;
}


#endif
