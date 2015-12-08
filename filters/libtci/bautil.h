#ifndef BAUTIL_H
#define BAUTIL_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

class ByteArray;
class TCIString;

struct BAUtil
{
  // the next 3 deal with null-separated lists of strings
  static TCI_BOOL    GetNextString(const ByteArray& ba, U32& offset, TCIString& next);
  static TCI_BOOL    GetNextParagraph(const ByteArray& ba, U32& offset, ByteArray&, TCI_BOOL& bHasHeader, TCI_BOOL multipara_tag = FALSE);
  static TCI_BOOL    HasString(const ByteArray&, const TCICHAR* str );

  // the next 4 work across the whole ByteArray
  static I32         FindString(const ByteArray& ba, const TCICHAR* str );
  static TCI_BOOL    StartsWithString(const ByteArray& ba, const TCICHAR* str );
  static TCI_BOOL    MatchesString(const ByteArray& ba, const TCICHAR* str );  // operator== ?
  static I32         FindChar(const ByteArray& ba, const TCICHAR ch );
  
  //the next two deal with lines separated by non-printable characters (one would hope for "\r\n" or "\n")
  static TCI_BOOL    GetNextLine(const ByteArray& ba, U32& nOffset, TCIString& nextLine);
  static TCI_BOOL    ReplaceLine(ByteArray& ba, U32 nLineOffset, const TCIString& newLine);
};

#endif