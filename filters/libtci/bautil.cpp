#include "bautil.h"
#include "tcistrin.h"
#include "bytearry.h"

#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include <memory.h>

TCI_BOOL BAUtil::GetNextString(const ByteArray& ba, U32& offset, TCIString& next) 
{
  TCI_BOOL rv(FALSE);
  next.Empty();
  U32 bytecount = ba.GetByteCount();
  if (offset < bytecount)
  {
    U32 start(offset);
    const U8* ptr = ba.ReadOnlyLock();
    if (ptr != NULL)
    {
      if (ptr[start] != 0)
      {
        while (offset < bytecount && ptr[offset] != 0)
          offset++;
        if (start < offset)
        {
          rv = TRUE;
          next = TCIString(ptr+start, offset-start);
        }
      }
      if (offset < bytecount && ptr[offset] == 0)
        offset++;
    }
    ba.ReadOnlyUnlock();
  }
  return rv;
}


TCI_BOOL BAUtil::GetNextLine(const ByteArray& ba, U32& nOffset, TCIString& nextLine) 
{
  TCI_BOOL rv(FALSE);
  nextLine.Empty();
  U32 bytecount = ba.GetByteCount();
  if (nOffset < bytecount)
  {
    U32 start(nOffset);
    const U8* ptr = ba.ReadOnlyLock();
    if (ptr != NULL)
    {
      if (ptr[start] >= 0x20)
      {
        while ((nOffset < bytecount) && (ptr[nOffset] >= 0x20))
          nOffset++;
        if (start < nOffset)
        {
          rv = TRUE;
          nextLine = TCIString(ptr+start, nOffset-start);
        }
      }
      if (nOffset < bytecount && ptr[nOffset] < 0x20)
      {
        if (nOffset+1 < bytecount && ptr[nOffset]=='\r' && ptr[nOffset+1]=='\n')
          nOffset += 2;
        else  
          nOffset++;
        rv = TRUE;  
      }    
        
    }
    ba.ReadOnlyUnlock();
  }
  return rv;
}


TCI_BOOL BAUtil::ReplaceLine(ByteArray& ba, U32 nLineOffset, const TCIString& newLine)
{
  U32 nNextLineOffset(nLineOffset);
  TCIString oldLine;
  GetNextLine(ba, nNextLineOffset, oldLine);
  TCIString realNewLine(newLine);
  int nLen = realNewLine.GetLength();
  if (nLen >= 1 && realNewLine[nLen - 1] < 0x20)
  {
    if (nLen < 2 || realNewLine[nLen - 2] != '\r' || realNewLine[nLen - 1] != '\n')
    {
      realNewLine = realNewLine.Left(nLen - 1) + "\r\n";
    }
  }
  else
    realNewLine += "\r\n";
  return ba.ReplaceBytes(nLineOffset, nNextLineOffset - nLineOffset, (const U8*)realNewLine, realNewLine.GetLength());
}

TCI_BOOL BAUtil::GetNextParagraph(const ByteArray& data, U32& offset, ByteArray& paragraph, 
                                  TCI_BOOL& bHasHeader, TCI_BOOL multipara_tag)
{
  paragraph.SetByteCount(0);
  U32 bytecount = data.GetByteCount();
  const U8* ptr =  data.ReadOnlyLock();
  if (bytecount && ptr)
  {
    if (offset+2 < bytecount && (ptr[offset]=='\n' && ptr[offset+1]=='\t' && ptr[offset+2]=='\n'))
    {
      offset += 3;
      bHasHeader = TRUE;
    }

    U32 start = offset;
    while (offset+2 < bytecount && !(ptr[offset]=='\n' && ptr[offset+1]=='\t' && ptr[offset+2]=='\n'))
      offset++;
    if (offset+2 >= bytecount)
      offset = bytecount;
    if (offset > start)
    {
      if (multipara_tag && bHasHeader)
        start -=  3;
      paragraph.AddBytes(ptr+start, offset - start);
    }
  }
  data.ReadOnlyUnlock();
  return (paragraph.GetByteCount() > 0);
}

TCI_BOOL BAUtil::HasString(const ByteArray& data, const TCICHAR* str) 
{
  TCI_BOOL rv(FALSE);
  if (str != NULL)
  {
    U32 len = strlen(str);
    U32 bytecount = data.GetByteCount();
    const TCICHAR* p  =  (const TCICHAR*)data.ReadOnlyLock();
    if (bytecount > 0 && p != NULL)
    {
      U32 offset(0);
      while (offset < bytecount && p[offset]) 
      {
        if (strncmp(p+offset, str, len) == 0) //match
        {
          rv = TRUE;
          break;
        }
        offset += strlen(p+offset) + 1;
      }
    }
    data.ReadOnlyUnlock();
  }
  return rv;
}

// return index of first matched string, else -1
I32 BAUtil::FindString( const ByteArray& ba, const TCICHAR* str )
{
  if (!str || !*str)  // vacuous
    return 0;
  U32 len = strlen(str);
  if (ba.GetByteCount() < len)
    return -1;

  I32 rv = -1;
  const U8* p = ba.ReadOnlyLock();
  for (U32 start = 0; start < ba.GetByteCount() - len; ++start)
    if (strncmp( str, reinterpret_cast<const TCICHAR*>(p+start), len ) == 0) {
      rv = start;
      break;
    }

  ba.ReadOnlyUnlock();
  return rv;
}

TCI_BOOL BAUtil::StartsWithString( const ByteArray& ba, const TCICHAR* str )
{
  if (!str || !*str)  // vacuous
    return TRUE;
  U32 len = strlen(str);
  if (ba.GetByteCount() < len)
    return FALSE;

  TCI_BOOL rv = FALSE;
  const U8* p = ba.ReadOnlyLock();
  if (strncmp( str, reinterpret_cast<const TCICHAR*>(p), len ) == 0)
    rv = TRUE;
  ba.ReadOnlyUnlock();
  return rv;
}

TCI_BOOL BAUtil::MatchesString( const ByteArray& ba, const TCICHAR* str )
{
  if (!str)
    return ba.GetByteCount() == 0;
  U32 len = strlen(str);
  if (ba.GetByteCount() != len)
    return FALSE;

  TCI_BOOL rv = FALSE;
  const U8* p = ba.ReadOnlyLock();
  if (strncmp( str, reinterpret_cast<const TCICHAR*>(p), len ) == 0)
    rv = TRUE;
  ba.ReadOnlyUnlock();
  return rv;
}

I32 BAUtil::FindChar( const ByteArray& ba, const TCICHAR ch )
{
  I32 rv = -1;
  if (ba.GetByteCount() == 0)
    return rv;

  const U8* p = ba.ReadOnlyLock();
  const U8* dest = reinterpret_cast<const U8*>( memchr( p, ch, ba.GetByteCount() ) );
  if (dest != NULL)
    rv = dest - p;
  ba.ReadOnlyUnlock();

  return rv;
}
