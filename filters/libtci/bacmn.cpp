#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "bytearry.h"
#include <tchar.h>
#ifndef NO_SMRTHEAP
  #if defined(TESTING) && !defined(NO_SMARTHEAP_DEBUG)
    #define MEM_DEBUG
  #endif
  #include <smrtheap.h>
#else
  #include <stdlib.h>
#endif

void* ByteArray::m_mempool = NULL;

ByteArray::ByteArray() : m_bytecount(0), m_handle(NULL) ,
  m_allocsize(0), m_lockedForWriting(FALSE), m_readOnlyLockCount(0)
{}

ByteArray::ByteArray(U32 allocsize) : m_bytecount(0), m_handle(NULL) ,
  m_allocsize(0), m_lockedForWriting(FALSE), m_readOnlyLockCount(0)
{
  if (allocsize > 0)
    ReSize(allocsize, BA_none);
}

ByteArray::ByteArray(const ByteArray& b) : m_bytecount(0), m_handle(NULL) ,
  m_allocsize(0), m_lockedForWriting(FALSE), m_readOnlyLockCount(0)
{
  TCI_ASSERT(!b.LockedFor(BA_ReadAccess));
  if (!b.LockedFor(BA_ReadAccess))
  {
    const U8* q = b.ReadOnlyLock();
    AddBytes(q, b.GetByteCount(), BA_none);
    b.ReadOnlyUnlock();
  }
}

ByteArray::ByteArray(const TCICHAR* str) : m_bytecount(0), m_handle(NULL) ,
  m_allocsize(0), m_lockedForWriting(FALSE), m_readOnlyLockCount(0)
{
  U32 len = str ? _tcslen(str) : 0;
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

TCI_BOOL ByteArray::operator==(const ByteArray& b)
{
  TCI_BOOL rv(FALSE);
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
  U32 len = str ? _tcslen(str) : 0;
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
  U32 len = str ? _tcslen(str)+1 : 0;
  AddBytes(reinterpret_cast<const U8*>(str), len*sizeof(TCICHAR), BA_double);
}

ByteArray& ByteArray::operator=(const TCICHAR* str) 
{
  U32 len = str ? _tcslen(str) : 0;
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

TCI_BOOL ByteArray::ReSize(U32 thesize)
{
  return ReSize(thesize, BA_none);
}

TCI_BOOL ByteArray::SetByteCount(U32 newcount) 
{
  TCI_BOOL rv(FALSE);
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
    m_lockedForWriting = TRUE;
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
  m_lockedForWriting = FALSE;
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
        TCI_VERIFY(ReSize(limit, flags));
      U8* p = Lock() + m_bytecount;
      memcpy(p, pData, count);
      Unlock();
      m_bytecount = limit;
    }
  }
}

TCI_BOOL ByteArray::ReplaceBytes(U32 nOffset, U32 nNumBytes, const U8* pNewBytes,
                                  U32 nNumNewBytes)
{
  if (LockedFor(BA_WriteAccess))
  {
    TCI_ASSERT(FALSE);
    return FALSE;
  }
  TCI_BOOL rv = FALSE;
  if (nOffset <= m_bytecount)
  {
    if (nOffset + nNumBytes > m_bytecount)
      nNumBytes = m_bytecount - nOffset;
    int nDelta = (int)nNumNewBytes - (int)nNumBytes;
    if (m_bytecount + nDelta >= m_allocsize)
      TCI_VERIFY(ReSize(m_bytecount + nDelta));
    U8* pOldMem = Lock() + nOffset;
    if (nDelta && (nOffset + nNumBytes < m_bytecount))
    {
      U8* pOldTail = pOldMem + nNumBytes;
      memmove(pOldTail + nDelta, pOldTail, m_bytecount - nOffset - nNumBytes);
    }
    memcpy(pOldMem, pNewBytes, nNumNewBytes);
    rv = TRUE;
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


TCI_BOOL ByteArray::ReSize(U32 thesize, U32 flags)
{
  TCI_ASSERT(!LockedFor(BA_WriteAccess));
  TCI_BOOL rv(FALSE);
  if (!LockedFor(BA_WriteAccess)) 
  {
    U32 size = CalculateAllocSize(thesize, flags);
    if (m_handle == NULL)
    { 
       TCI_VERIFY(ByteArray::AllocMemory(&m_handle, size));
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

TCI_BOOL ByteArray::LockedFor(U32 mode)  const 
{
  TCI_BOOL rv(FALSE);
  if (m_lockedForWriting)
    rv = TRUE;
  if (!rv && (m_readOnlyLockCount > 0) && (mode == BA_WriteAccess))
    rv = TRUE;
  if (!rv && (m_readOnlyLockCount > 0) && (mode == (BA_WriteAccess|BA_ReadAccess)))
    rv = TRUE;
  return rv;
}

#ifndef NO_SMRTHEAP

#define MEM_FLAGS MEM_MOVEABLE

TCI_BOOL ByteArray::InitHeap()
{
  if (m_mempool == NULL)
  {
    m_mempool =  MemPoolInit( MEM_POOL_DEFAULT );
    TCI_ASSERT(m_mempool);
  }
  #ifndef _SHI_dbgMacros
  if (ByteArray::m_mempool)
  {
    // this call will go away in the non-testing version
    dbgMemPoolSetCheckFrequency((MEM_POOL)m_mempool, 1000);
  }
  #endif
  return m_mempool != NULL;
}

TCI_BOOL ByteArray::AllocMemory(void** h, U32 size)
{
  if (!size)
    return FALSE;

  if (!m_mempool && !ByteArray::InitHeap())
    return FALSE;

  *h =  MemAllocPtr((MEM_POOL)m_mempool, size, 0);
  TCI_ASSERT(*h);
  return *h != NULL;
}

void* ByteArray::ReallocMemory(void* handle, U32 size)
{
  void* newHandle = MemReAllocPtr(handle, size, 0);
  TCI_ASSERT(newHandle);
  return newHandle;
}

void ByteArray::FreeMemory(void* handle)
{
  if (handle) 
  {
    TCI_ASSERT(MemCheckPtr((MEM_POOL)m_mempool, handle)==MEM_POINTER_OK );
    MemFreePtr(handle);
    handle = NULL;
  }
}

#else

TCI_BOOL ByteArray::InitHeap()
{
  return TRUE;
}

TCI_BOOL ByteArray::AllocMemory(void** handle, U32 size)
{
  if (!size)
    return FALSE;
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

#endif // #ifndef NO_SMRTHEAP
