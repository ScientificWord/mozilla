// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef BYTEARRY_H
#define BYTEARRY_H

#include "../CmpTypes.h"
#define TCICHAR  char
#define TCIWCHAR  short
typedef unsigned char U8;

class ByteArray 
{
  public:

  ByteArray();
  explicit ByteArray(U32 thesize);
  ByteArray(const ByteArray&);
  explicit ByteArray(const TCICHAR*);
  ~ByteArray();


  bool    operator==(const ByteArray&);
 
  void        AddBytes(const U8* pData, U32 count);
 
  ByteArray&  operator=(const ByteArray&);
  ByteArray&  operator=(const TCICHAR*);
  ByteArray&  operator=(const U8);
  
  ByteArray&  operator+=(const ByteArray&);
  ByteArray&  operator+=(const TCICHAR*);
  ByteArray&  operator+=(const U8);
  void        AddString(const TCICHAR* str); //ljh 9/99 will include NULL terminator

  bool        ReSize(U32 newsize);
  
  bool        SetByteCount(U32 newcount);
  inline U32      GetByteCount() const {return m_bytecount;}
  
  U8*             Lock();
  void            Unlock();
  const U8*       ReadOnlyLock() const;
  void            ReadOnlyUnlock() const;

  bool        ReplaceBytes(U32 nOffset, U32 nNumBytes, const U8* pNewBytes,
                                  U32 nNumNewBytes);

  private:
  
  void            AddBytes(const U8* pData, U32 count, U32 flags);
  U32             CalculateAllocSize(U32 size, U32 flags);
  bool        ReSize(U32 newsize, U32 flags);
  bool        LockedFor(U32 access) const;
  U8*             GetDataPtr() const; 
  // Memory system specific
  static bool AllocMemory(void** handle, U32 size);
  static void*    ReallocMemory(void* handle, U32 size);
  static void     FreeMemory(void* handle);
  static bool InitHeap(); //ljh 9/99 heap agent only


  //Data
  private:
  void*            m_handle;
  U32              m_bytecount;
  U32              m_allocsize;
  mutable bool m_lockedForWriting;
  mutable int      m_readOnlyLockCount;
  static void*     m_mempool;  // heap agent only

  enum ClassFlags 
  { 
    BA_none        = 0,
    BA_reset       = 1,
    BA_double      = 2,
    BA_ReadAccess  = 4, 
    BA_WriteAccess = 8
  };
};
#endif

