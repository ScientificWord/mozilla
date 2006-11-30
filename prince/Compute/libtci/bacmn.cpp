// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "bytearry.h"
#include "../CmpTypes.h"
#include <stdlib.h>
#include <string.h>

void* ByteArray::m_mempool = NULL;

ByteArray::ByteArray() : m_handle(NULL), m_bytecount(0), 
  m_allocsize(0), m_lockedForWriting(false), m_readOnlyLockCount(0)
{}

ByteArray::ByteArray(U32 allocsize) : m_handle(NULL), m_bytecount(0), 
  m_allocsize(0), m_lockedForWriting(false), m_readOnlyLockCount(0)
{
  if (allocsize > 0)
    ReSize(allocsize, BA_none);
}

ByteArray::ByteArray(const ByteArray& b) : m_handle(NULL), m_bytecount(0), 
  m_allocsize(0), m_lockedForWriting(false), m_readOnlyLockCount(0)
{
  TCI_ASSERT(!b.LockedFor(BA_ReadAccess));
  if (!b.LockedFor(BA_ReadAccess))
  {
    const U8* q = b.ReadOnlyLock();
    AddBytes(q, b.GetByteCount(), BA_none);
    b.ReadOnlyUnlock();
  }
}

ByteArray::ByteArray(const TCICHAR* str) : m_handle(NULL), m_bytecount(0), 
  m_allocsize(0), m_lockedForWriting(false), m_readOnlyLockCount(0)
{
  U32 len = str ? strlen(str) : 0;
  AddBytes(reinterpret_cast<const U8*>(str), len*sizeof(TCICHAR), BA_none);
}

ByteArray::~ByteArray()
{
  if (m_handle) 
  {
    TCI_ASSERT(!LockedFor(BA_ReadAccess | BA_WriteAccess));
    ByteArray::FreeMemory(m_handle);
    m_handle = 0;
  }
}

bool ByteArray::operator==(const ByteArray& b)
{
  bool rv(false);
  TCI_ASSERT(!LockedFor(BA_ReadAccess) && !b.LockedFor(BA_ReadAccess));
  if (!LockedFor(BA_ReadAccess) && !b.LockedFor(BA_ReadAccess) && (b.GetByteCount() == GetByteCount()))
  {
    const U8* p = ReadOnlyLock();
    const U8* q = b.ReadOnlyLock();
    if (p && q)
      rv =  !memcmp(p, q, GetByteCount());
    b.ReadOnlyUnlock();
    ReadOnlyUnlock();
  }
  return rv;
}

void ByteArray::AddBytes(const U8* pData, U32 count)
{
  AddBytes(pData, count, BA_double);
}

ByteArray& ByteArray::operator+=( const ByteArray& b ) 
{
  TCI_ASSERT(!b.LockedFor(BA_ReadAccess));
  AddBytes((const U8*)(b.GetDataPtr()), b.GetByteCount(), BA_double);
  return *this;
}

ByteArray& ByteArray::operator+=(const TCICHAR* str) 
{
  U32 len = str ? strlen(str) : 0;
  AddBytes(reinterpret_cast<const U8*>(str), len*sizeof(TCICHAR), BA_double);
  return *this;
}

ByteArray& ByteArray::operator+=(const U8 u8) 
{
  AddBytes(&u8, 1, BA_double);
  return *this;
}

void ByteArray::AddString(const TCICHAR* str)  // adds null terminator to data.
{
  U32 len = str ? strlen(str)+1 : 0;
  AddBytes(reinterpret_cast<const U8*>(str), len*sizeof(TCICHAR), BA_double);
}

ByteArray& ByteArray::operator=(const TCICHAR* str) 
{
  U32 len = str ? strlen(str) : 0;
  AddBytes(reinterpret_cast<const U8*>(str), len*sizeof(TCICHAR), BA_reset);
  return *this;
}

ByteArray& ByteArray::operator=( const U8 u8 ) 
{
  AddBytes(&u8, 1, BA_reset);
  return *this;
}

ByteArray& ByteArray::operator=( const ByteArray& b ) 
{
  TCI_ASSERT(!b.LockedFor(BA_ReadAccess));
  AddBytes((const U8*)b.GetDataPtr(), b.GetByteCount(), BA_reset);
  return *this;
}

bool ByteArray::ReSize(U32 thesize)
{
  return ReSize(thesize, BA_none);
}

bool ByteArray::SetByteCount(U32 newcount) 
{
  bool rv(false);
  m_bytecount =  (newcount <= m_allocsize) ? newcount : m_allocsize;
  rv = (newcount <= m_allocsize);
  return rv;
}

U8* ByteArray::Lock()
{
  U8* rv = NULL;
  TCI_ASSERT(m_allocsize > 0 && m_handle != NULL);
  TCI_ASSERT(!LockedFor(BA_WriteAccess));
  if (!LockedFor(BA_WriteAccess))
  {
    m_lockedForWriting = true;
    rv = GetDataPtr();
  }
  return rv;
}

const U8* ByteArray::ReadOnlyLock() const
{
  const U8* rv = NULL;
  TCI_ASSERT(m_allocsize > 0 && m_handle != NULL);
  TCI_ASSERT(!LockedFor(BA_ReadAccess));
  if (!LockedFor(BA_ReadAccess))
  {
    m_readOnlyLockCount += 1;
    rv = (const U8*)GetDataPtr();
  }
  return rv;
}


void ByteArray::Unlock()
{
  m_lockedForWriting = false;
}

void ByteArray::ReadOnlyUnlock() const
{
  if (m_readOnlyLockCount > 0)
    m_readOnlyLockCount -= 1;
}


//
// PRIVATE functions
//

U8*  ByteArray::GetDataPtr() const
{
  return static_cast<U8*>(m_handle);
}

void ByteArray::AddBytes(const U8* pData, U32 count, U32 flags)
{
  TCI_ASSERT(!LockedFor(BA_WriteAccess));
  if (!LockedFor(BA_WriteAccess))
  {
    if (flags & BA_reset)
      m_bytecount = 0;
    if (pData && count)
    {
      U32 limit = m_bytecount + count;
      if (limit >= m_allocsize)
        ReSize(limit, flags);
      U8* p = Lock() + m_bytecount;
      memcpy(p, pData, count);
      Unlock();
      m_bytecount = limit;
    }
  }
}

bool ByteArray::ReplaceBytes(U32 nOffset, U32 nNumBytes, const U8* pNewBytes,
                                  U32 nNumNewBytes)
{
  if (LockedFor(BA_WriteAccess))
  {
    TCI_ASSERT(false);
    return false;
  }
  bool rv = false;
  if (nOffset <= m_bytecount)
  {
    if (nOffset + nNumBytes > m_bytecount)
      nNumBytes = m_bytecount - nOffset;
    int nDelta = (int)nNumNewBytes - (int)nNumBytes;
    if (m_bytecount + nDelta >= m_allocsize)
      ReSize(m_bytecount + nDelta);
    U8* pOldMem = Lock() + nOffset;
    if (nDelta && (nOffset + nNumBytes < m_bytecount))
    {
      U8* pOldTail = pOldMem + nNumBytes;
      memmove(pOldTail + nDelta, pOldTail, m_bytecount - nOffset - nNumBytes);
    }
    memcpy(pOldMem, pNewBytes, nNumNewBytes);
    rv = true;
    Unlock();
  }
  return rv;
}
                                  
U32 ByteArray::CalculateAllocSize(U32 thesize, U32 flags)
{
  U32 newsize = (thesize > m_allocsize) ? thesize : m_allocsize;
  if (flags & BA_double)
    newsize *=2;
  return newsize;
}


bool ByteArray::ReSize(U32 thesize, U32 flags)
{
  TCI_ASSERT(!LockedFor(BA_WriteAccess));
  bool rv(false);
  if (!LockedFor(BA_WriteAccess)) 
  {
    U32 size = CalculateAllocSize(thesize, flags);
    if (m_handle == NULL)
    { 
       ByteArray::AllocMemory(&m_handle, size);
       TCI_ASSERT(m_handle);
       m_allocsize = (m_handle != NULL) ? size : 0;
    }
    else
    {  
      //Handle must already have been nonzero
      if (size > m_allocsize)
      {
        void* newh = ByteArray::ReallocMemory(m_handle, size);
        m_handle =  newh;
        m_allocsize = (m_handle != NULL) ? size : 0;
      }
    }
    TCI_ASSERT(m_handle);
    rv = (m_handle != NULL);
  }
  return rv;
}

bool ByteArray::LockedFor(U32 mode)  const 
{
  bool rv(false);
  if (m_lockedForWriting)
    rv = true;
  if (!rv && (m_readOnlyLockCount > 0) && (mode == BA_WriteAccess))
    rv = true;
  if (!rv && (m_readOnlyLockCount > 0) && (mode == (BA_WriteAccess|BA_ReadAccess)))
    rv = true;
  return rv;
}


bool ByteArray::InitHeap()
{
  return true;
}

bool ByteArray::AllocMemory(void** handle, U32 size)
{
  if (!size)
    return false;
  *handle =  malloc(size);
  TCI_ASSERT(*handle);
  return *handle != NULL;
}

void* ByteArray::ReallocMemory(void* handle, U32 size)
{
  void* newHandle = realloc(handle, size);
  TCI_ASSERT(newHandle);
  return newHandle;
}

void ByteArray::FreeMemory(void* handle)
{
  if (handle)
    free(handle);
}
