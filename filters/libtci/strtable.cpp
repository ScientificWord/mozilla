
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "strtable.h"
#include "strdata.h"
#include "bytearry.h"
#include "tcistrin.h"

#define INVALID  0xFFFF
#define HASH_INCREMENT 79


// unused slots in the hashtables should contain INVALID

// hash table structure
typedef struct tableentry {
  U16 id;
  U16 offset;
} hashtable;


void ConvertBytes( ByteArray* ba,U32 numitems ) {

  U16 temp;
  U8*  byteptr     =  ba->Lock();
  U32 offset  =  0;
  U16* wordptr  =  (U16*)byteptr;

  for ( U32 i=0; i<numitems; i++ ) {
    temp  =  byteptr[offset]*256 + byteptr[offset+1];
    offset += 2;
    wordptr[i]  =  temp;
  }

  ba->Unlock();
}



U32 MakePrime( U32 n ) {
  U32 i;
  U32 m = n;
  while( TRUE ) {
    for ( i=2; i<257; i++ ) {
      if( m % i == 0 ) {
        m++;
        break;
      } else if( i * i > m ) return m;
    }
  }
}


// constructor from a file name - file contains hashtables and strings
StringTable::StringTable( StringData* sdata ) {

  idtostrtable  =  (ByteArray *)NULL;
  strtoidtable  =  (ByteArray *)NULL;
  stringdata  = sdata;

  //ByteArray* strdata =  TCI_NEW( ByteArray(6) );
  U8 buff[6];
  TCI_BOOL b =  sdata->LoadBytes(0L,6,FALSE,buff);
  TCI_ASSERT(b);
  if (b) {
    table1size  =  buff[0] * 256 + buff[1];
    table2size  =  buff[2] * 256 + buff[3];
    ignorebytes =  buff[4] * 256 + buff[5];
    hash_increment  =  HASH_INCREMENT;

    //Note: Force ignorebytes = 1. If this ever changes, then
    //      it must also be changed in all of the StringData
    //      implementations.
    TCI_ASSERT(ignorebytes == 1);
    ignorebytes = 1;

    table1offset = 6L;
    idtostrtable  =  TCI_NEW( ByteArray(4*table1size) );
    table2offset = table1offset + 4*table1size;
    U32 fileoffset  =  table1offset;
    U8* ptr =  idtostrtable->Lock();
    if (sdata->LoadBytes(fileoffset, 4*table1size, FALSE, ptr)) {
      idtostrtable->Unlock();
      ConvertBytes(idtostrtable, 2*table1size);
      fileoffset  +=  4*table1size;
      strtoidtable  =  TCI_NEW( ByteArray(4*table2size));
      ptr =  strtoidtable->Lock();
      if (sdata->LoadBytes(fileoffset, 4*table2size, FALSE, ptr)) {
        strtoidtable->Unlock();
        ConvertBytes(strtoidtable, 2*table2size);
      } else {
        TCI_ASSERT(FALSE);
        strtoidtable->Unlock();
      }
    } else {
      TCI_ASSERT(FALSE);
      idtostrtable->Unlock();
    }
  }
}


StringTable:: ~StringTable() {

  delete idtostrtable;
  delete strtoidtable;
  //We don't delete stringdata as it is not created here.
}


U32 StringTable::GetString(U32 strnum, TCIString& str, U32& kind)
{
  U32 rv  =  0;
  str.Empty();

  hashtable * table = (hashtable *) idtostrtable->Lock();
  U32 startindex  =  strnum % table1size;
  U32 index =  startindex;
  do {
    if (table[index].id == strnum) {          // entry found
      U32 strloc  =  table[index].offset;
      U8 buff[128];
      if (stringdata->LoadBytes(strloc, 128, TRUE, buff)) {
        if( ignorebytes == 1 )
          kind =  buff[0];
        else
          kind =  buff[1]*256 + buff[0];
        TCICHAR* ptr =  str.GetBuffer(128);
        TCI_ASSERT(sizeof(TCICHAR) == 1);
        U32 ii = 0;
        for (U32 jj=ignorebytes; buff[jj] && jj<128; jj++) 
          ptr[ii++] =  buff[jj];
        ptr[ii] = 0;
        str.ReleaseBuffer(-1);
        rv =  ii;
      }
      break;
    } else if ( table[index].id == INVALID ) {
      break; // unused hashtable slot
    } else {                    // advance to next slot
      index +=  hash_increment;
      if (index >= table1size)
        index -= table1size;
    }
  } while (index != startindex);
  idtostrtable->Unlock();
  return rv;
}

//Given an ID, retrieve the associated string.
//  Caller MUST provide storage for returned string!
U32 StringTable::GetString( U32 strnum, U8* str, U32 limit, U32& kind ) {

  U32 rv =  0;
  str[0] =  0;

  hashtable * table = (hashtable *) idtostrtable->Lock();
  U32 startindex  =  strnum % table1size;
  U32 index =  startindex;
  do {
    if ( table[index].id == strnum ) {          // entry found
      U32 strloc  =  table[index].offset;
      U8* ptr =  TCI_NEW(U8[limit]);
      if (stringdata->LoadBytes(strloc, limit, TRUE, ptr)) {
        if( ignorebytes == 1 )
          kind = ptr[0];
        else
          kind = ptr[1] * 256 + ptr[0];
        U32 jj = ignorebytes;
        U32 ii = 0;
        for (; ptr[jj] && ii<limit-1; jj++) 
          str[ii++] = ptr[jj];
        str[ii] = 0;
        rv  =  ii;
      }
      delete ptr;
      break;
    } else if ( table[index].id == INVALID ) {
      break; // unused hashtable slot
    } else {                    // advance to next slot
      index +=  hash_increment;
      if (index >= table1size)
        index -= table1size;
    }
  } while ( index != startindex );
  idtostrtable->Unlock();
  return rv;
}


U32 StringTable::GetKind( U32 strnum ) {

  U32 rv = 0;

  // look up in the id to string table
  hashtable * table = (hashtable *) idtostrtable->Lock();
  U32 startindex  =  strnum % table1size;
  U32 index =  startindex;
  do {
    if ( table[index].id == strnum ) {          // entry found
      U32 strloc  =  table[index].offset;
      U8 buff[128];
      if (stringdata->LoadBytes(strloc, 128, TRUE, buff)) {
        if( ignorebytes == 1 )
          rv = buff[0];
        else
          rv = buff[1] * 256 + buff[0];
      }
      break;
    } else if ( table[index].id == INVALID ) {
      break; // unused hashtable slot
    } else {                    // advance to next slot
      index+=hash_increment;
      if ( index >= table1size ) index -= table1size;
    }
  } while ( index != startindex );
  idtostrtable->Unlock();
  return rv;
}

//Given an string, retrieve the associated ID
U32 StringTable::GetID( const U8 * str, U32 length, U32& kind ) {

  U32 startindex  =  Hash(str, length);
  U32 rv = 0;

  hashtable* table =  (hashtable*)strtoidtable->Lock();
  U32 index =  startindex;

  U8 buff[128];
  do {
    if (table[index].id == INVALID)
      break;

    U32 strloc  =  table[index].offset;
    TCI_BOOL b =  stringdata->LoadBytes(strloc, 128, TRUE, buff);
    TCI_ASSERT(b);
    if (!b)
      break;

    if (strlen((char*)buff+ignorebytes) != length) {
      //nothing
    } else if (strncmp((char*)str,(char*)buff+ignorebytes, length)==0) {
      if( ignorebytes == 1 )
        kind = buff[0];
      else
        kind =  buff[1] * 256 + buff[0];
      rv =  table[index].id;
      break;
    }

    index +=  hash_increment;
    if (index >= table2size)
      index -= table2size;
  } while (index != startindex);

  strtoidtable->Unlock();
  return rv;
}


U32 StringTable::GetID(const TCIString& str, U32& kind ) {
  U32 length =  str.GetLength();
  U32 startindex  =  Hash(str, length);
  U32 rv = 0;

  hashtable* table =  (hashtable*)strtoidtable->Lock();
  U32 index =  startindex;

  U8 buff[128];
  do {
    if (table[index].id == INVALID)
      break;

    U32 strloc  =  table[index].offset;
    TCI_BOOL b =  stringdata->LoadBytes(strloc, 128, TRUE, buff);
    TCI_ASSERT(b);
    if (!b)
      break;

    if (strlen((char*)buff+ignorebytes) != length) {
      //nothing
    } else if (str.Compare((char*)buff+ignorebytes, length)==0) {
      if( ignorebytes == 1 )
        kind = buff[0];
      else
        kind =  buff[1] * 256 + buff[0];
      rv =  table[index].id;
      break;
    }

    index +=  hash_increment;
    if (index >= table2size)
      index -= table2size;
  } while (index != startindex);

  strtoidtable->Unlock();
  return rv;
}


//Given an string, hash the SOB  (Sweet Old Bob)
U32 StringTable::Hash(const U8* str, U32 length)
{
  if (!length)
    return 0;

  U16 rv = 0;
  U16 next, j=0;
  while (j<length-1) {
    next =  (str[j]<<7) + str[j+1];
    rv ^=  next>>(j&0x3);
    j +=  2;
  }
  if (j<length)
    rv ^=  str[j]<<(j&0x3);

  TCI_ASSERT(table2size);
  return (rv % table2size);
}


