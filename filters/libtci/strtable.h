#ifndef STRTABLE_H
#define STRTABLE_H


/* Some issues related to the StringFiler class :

    It is useful to keep the strings and their ids in an external file.
  The strings in the file can be edited to accomplish language translation.

  We need to decide what runtime resources to devote to the task
  of retrieving data from this file.

  At a minimum, a file name could be held and the file could be opened,
  read and closed for each call - SLOW but holds few resources.

  An intermediate solution might involve holding a file handle and keeping
  the access structures ( perhaps hashtables ) in core, leaving the strings
  themselves in the file to be read for each call.  Perhaps a buffer could
  be used hold most recent retrievals.

  Finally, we might keep the entire file in core at all times.  This would
  speedup the retrievals and free the file handle, but more RAM would be tied
  up.  If virtual memory is involved, this may not be a problem.
*/


#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

class  StringData;
class  ByteArray;
class  TCIString;

class  StringTable {

public:

  StringTable( StringData* );
  ~StringTable();

  U32 GetString(U32 strnum, TCIString& str, U32& kind);
  U32 GetString(U32 strnum, U8 * str, U32 limit, U32& kind);
  U32 GetKind( U32 strnum );
  U32 GetID(const U8 * str, U32 length, U32& kind);
  U32 GetID(const TCIString&, U32& kind);

private:
  
  U32 Hash( const U8 * str, U32 length );

  U32 ignorebytes;
  U32 hash_increment;
  U32 table1size;
  U32 table2size;
  U32 table1offset;
  U32 table2offset;

  ByteArray * idtostrtable;
  ByteArray * strtoidtable;
  StringData * stringdata;
};

#endif

