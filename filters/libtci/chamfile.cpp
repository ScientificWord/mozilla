
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "chamfile.h"
#include "filespec.h"
#include "tcitime.h"
#include "bytearry.h"
#ifdef INET_ON
  #include "inettran.h"
#endif

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
#define NONLS             
#define NOMB              
#define NOMEMMGR          
#define NOMETAFILE        
#define NOMINMAX          
#define NOMSG             
//#define NOOPENFILE        
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


#ifdef INET_ON
  INetTranslator* ChamFile::InetTranslator = 0;
#endif

#define SEEK(handle,offset,mode)    (TCISetFilePointer(handle, offset, mode))
#define READ(handle,buff,size,res)  (TCIReadFile(handle, (LPSTR)buff, size, &res))
#define WRITE(handle,buff,size,res) (TCIWriteFile(handle, (LPCSTR)buff, size, &res))



ChamFile::ChamFile(const FileSpec& fs, U32 mode, I32 buf_size)
{
  TCIString filename =  fs.GetFullPath();    // will be in CURR_OS form

  filespec  =  NULL;
  openmode  =  0;
  filesize  =  0L;

  fname     =  0;
  fp        =  0L;
  bufsize   =  CF_MIN_BUFFER_SIZE;
  bp        =  0;
  bytes_in_buffer  =  0;
  buffer    =  (U8*)NULL;
  buffer_dirty  =  FALSE;
  nFileError = 0;

#ifdef INET_ON
  // Is it an URL?
  m_isURL = fs.IsURL();

  if (m_isURL) {
    TCI_ASSERT(mode & CHAM_FILE_READ);
    TCI_ASSERT(!(mode & (CHAM_FILE_WRITE|CHAM_FILE_CREATE|CHAM_FILE_APPEND)));

    if (!IntConstruct(fs, filename))    // retrieves cache file name
      return;
  }
#else
  TCI_ASSERT( !fs.IsURL() );
#endif
  // It's a local file

  DWORD acc_mode =  GENERIC_READ;
  if ( mode&CHAM_FILE_WRITE )
    acc_mode |=  GENERIC_WRITE;
  DWORD share_mode =  FILE_SHARE_READ;    // share read privledges
  SECURITY_ATTRIBUTES sa;                   // \  
  sa.nLength =  sizeof(SECURITY_ATTRIBUTES);//  > Ignored on Mac
  sa.lpSecurityDescriptor =  NULL;          // /
  sa.bInheritHandle =  FALSE;               //
  DWORD create_mode =  (mode&CHAM_FILE_CREATE) ? CREATE_ALWAYS 
                      : (mode & CHAM_FILE_APPEND) ? OPEN_ALWAYS
                      : OPEN_EXISTING;
  DWORD attrib =  FILE_ATTRIBUTE_NORMAL;
  HANDLE fh =  CreateFile((LPCSTR)filename, acc_mode, share_mode, &sa, create_mode, attrib, NULL );
  if ( fh != INVALID_HANDLE_VALUE ) {
    fname    =  TCI_NEW(FileSpec(fs));
    filesize =  GetFileSize(fh, NULL);
    filespec =  fh;
    openmode =  mode;
    if ( buf_size > bufsize) 
      bufsize =  buf_size;
    buffer =  TCI_NEW( U8[bufsize] );
	if (mode & CHAM_FILE_APPEND)
	{
	  if (SEEK( filespec, 0, FILE_END ) != -1)
		fp = filesize;
	  else
	    TCI_ASSERT(FALSE);
	}
  }
}


ChamFile::~ChamFile()
{
  if ( buffer ) {
    if ( filespec && (openmode&CHAM_FILE_WRITE) && bp )
      FlushBuffer(0);
    delete buffer;
    buffer =  0;
  }

  if ( filespec ) {
    TCI_VERIFY( CloseHandle( (HANDLE)filespec ) );
  }
  filespec =  NULL;

#ifdef INET_ON
  if (m_isURL)
    IntDestruct();
#endif

  delete fname;
}


TCI_BOOL ChamFile::Writeable() {

  return filespec!=NULL && (openmode&CHAM_FILE_WRITE);
}


TCI_BOOL ChamFile::Readable() {

  return filespec!=NULL && ((openmode&CHAM_FILE_READ) || (openmode&CHAM_FILE_WRITE));
}


ChamFile::CFReturn ChamFile::WriteString(const TCIString& s) {

  CFReturn rv =  CFR_OK;

  // zzzASC - dirty and expensive (in terms of time)
  TCIString str(s);
  str.Replace("\r\n", "\n");
  str.Replace("\r", "\n");

  int pos =  str.Find('\n');
  int prevpos =  0;
  while ((rv == CFR_OK) && (pos >= 0)) {
    if (!buffer) {
      TCI_ASSERT(FALSE);
      return CFR_WriteError;
    } else {
      rv =  ToBuffer(str.Mid(prevpos, pos-prevpos));
      if (rv == CFR_OK)
        rv =  ToBuffer("\r\n");   // what about non DOS OS??? zzzASC
    }
    prevpos =  pos + 1;
    pos =  str.Find('\n', pos+1);
  }

  if (rv==CFR_OK && prevpos<str.GetLength()) {
      rv =  ToBuffer(str.Mid(prevpos));
  }

  return rv;
}


ChamFile::CFReturn ChamFile::WriteBytes(const TCIString& src, U32 n, U32& bytes ) 
{
  U32 srclen =  (U32)src.GetLength();

  if (srclen < n)
  {
    TCI_ASSERT(FALSE);
    bytes = 0;
    return CFR_WriteError;
  }
  else
  {
    if (srclen == n)
      return WriteBytes(src, bytes);
    else // (srclen > n)
    {
      TCIString newSrc = src;
      newSrc.Truncate(n);
      return WriteBytes(newSrc, bytes);
    }
  }
}

ChamFile::CFReturn ChamFile::WriteBytes(const TCIString& src, U32& bytes) {

  CFReturn rv =  CFR_OK;

  if ( buffer==(U8*)NULL ) {
    TCI_ASSERT(FALSE);
    return CFR_WriteError;
  } else {
    rv =  ToBuffer(src);
    bytes =  (rv==CFR_OK ? src.GetLength() : 0);
  }

  return rv;
}


ChamFile::CFReturn ChamFile::WriteBytes(const ByteArray& src, U32 offset, U32 num, U32& bytes ) {

  CFReturn rv =  CFR_OK;

  const U8* dataptr  =  src.ReadOnlyLock();
  dataptr +=  offset;
  U32 len =  src.GetByteCount();
  TCI_ASSERT(offset+num <= len);
  int nbytes =  num;
  if (offset+nbytes > len) {
    nbytes =  len - offset;
    if (nbytes < 0)
      nbytes =  0;
  }
  if (buffer==(U8*)NULL ) {
    TCI_ASSERT(FALSE);
    return CFR_WriteError;
  } else {
    TCIString tmp(dataptr, nbytes);
    rv =  ToBuffer(tmp);
    bytes =  (rv==CFR_OK ? tmp.GetLength() : 0);
  }
  src.ReadOnlyUnlock();

  return rv;
}


ChamFile::CFReturn ChamFile::ReadBytes(U8* destbuffer, U32 numbytes, U32& bytes) {

  bytes =  0;
  CFReturn rv =  CFR_OK;
  if (buffer) {    // we're using a read buffer

    U32 di  =  0;
    do {
      while ( bp<bytes_in_buffer && di<numbytes )
        *(destbuffer+di++)  =  *(buffer+bp++);

      if ( bp>=bytes_in_buffer && di<numbytes )
        FillBuffer();
    } while ( bytes_in_buffer && di<numbytes );
    bytes = di;

    //What are the conditions for returning CFR_EOF??
    if (!di)
      rv =  CFR_EOF;

  } else {
    TCI_ASSERT(FALSE);
    return CFR_ReadError;
  }

  return rv;
}


ChamFile::CFReturn ChamFile::ReadBytes( TCIString& dest, U32 numbytes, U32& bytes ) {

  bytes =  0;
  CFReturn rv =  CFR_OK;
  if ( buffer != (U8*)NULL ) {    // we're using a read buffer

    dest.SetDataLength(0);
    do {

      int len =  CHAMmin((U32)bytes_in_buffer-bp, numbytes);
      if ( len ) {
        dest +=  TCIString(buffer+bp, len);
        numbytes -=  len;
        bytes +=  len;
        bp +=  len;
      }
      if ( numbytes ) // used all the bytes in the buffer
        FillBuffer();

    } while (numbytes && bytes_in_buffer);

    //What are the conditions for returning CFR_EOF??
    if (!bytes)
      rv =  CFR_EOF;

  } else {
    TCI_ASSERT(FALSE);
    return CFR_ReadError;
  }

  return rv;
}


ChamFile::CFReturn ChamFile::Goto( I32 offset ) {

  CFReturn rv =  CFR_DiskError;

  if ( buffer != (U8*)NULL ) {    // we're using a read buffer

    if ( fp<=offset && offset<fp+bytes_in_buffer ) {
      bp =  offset-fp;
      rv =  CFR_OK;
    } else {
      if ( buffer_dirty )
        if ( (rv=FlushBuffer(0)) != CFR_OK )
          return rv;

      TCI_BOOL res =  SEEK( filespec, offset, FILE_BEGIN ) != -1L;
      if ( res ) {
        fp  =  offset;
        bp  =  0;
        bytes_in_buffer =  0;
        rv =  CFR_OK;
      } else {
        TCI_ASSERT(FALSE);
      }
    }

  } else {
    TCI_ASSERT(FALSE);
  }

  return rv;
}


TCI_BOOL ChamFile::AtEnd() {

  TCI_BOOL rv;

  if ( buffer != (U8*)NULL ) {    // we're using a read buffer
    rv  =  (fp+bp) == (I32)filesize;
  } else {  // not using a read buffer
    //Is there a better way than this?
    TCI_ASSERT(FALSE);
    rv = FALSE;
    //DWORD res =  SEEK( filespec, 0, FILE_CURRENT );
    //rv =  res==filesize;
  }

  return rv;
}


TCI_BOOL ChamFile::Delete(){

  TCI_BOOL rv =  FALSE;
  if ( filespec != NULL ) {
	  rv =  CloseHandle((HANDLE)filespec) ? TRUE : FALSE;
    TCI_ASSERT(rv);
    rv =  DeleteFile(static_cast<const TCICHAR*>(*fname)) ? TRUE : FALSE;
    TCI_ASSERT(rv);
  }
  filespec = NULL;

  return rv;
}


TCI_BOOL ChamFile::Flush() {

  TCI_BOOL rv  =  FALSE;
  if ( filespec!=NULL && (openmode&CHAM_FILE_WRITE) ) {
    //you only flush when writing
    if ( buffer!=(U8*)NULL && bytes_in_buffer )
      FlushBuffer(0);
    rv =  FlushFileBuffers((HANDLE)filespec) ? TRUE : FALSE;
  }

  TCI_ASSERT( rv );
  return rv;
}


ChamFile::CFReturn ChamFile::ReadString(U8* s, U32 limit) {

  TCIString readbuf;
  CFReturn rv = ReadString(readbuf);
  if (CFR_OK==rv) {
    TCI_ASSERT(readbuf.GetLength()<(int)limit);
    U32 lim =  CHAMmin(U32(limit-1), U32(readbuf.GetLength()));
    strncpy((char*)s, readbuf, lim);
    s[lim] =  0;
  }
  return rv;
}


ChamFile::CFReturn ChamFile::ReadString(TCIString& s) {

  CFReturn rv =  CFR_OK;
  s.SetDataLength(0);
  if ( buffer != (U8*)NULL ) {          // we're using a read buffer
    U8 ch;
    long di =  bp;

    do {                // loop thru buffer fills

      while ( bp<bytes_in_buffer ) {    // loop thru chars in buffer
        ch  =  *(buffer+bp);
        bp++;

        if ( ch=='\r' || ch=='\n' ) {
          if ( ch=='\r' ) {
            if ( bp>=bytes_in_buffer ) {
              //s +=  TCIString((TCICHAR*)buffer+di, bp-di);
              int l =  s.GetLength();
              TCICHAR* ptr =  s.GetBuffer(l+bp-di+1);
              memcpy(ptr+l, buffer+di, bp-di);
              ptr[l+bp-di] =  0;
              s.ReleaseBuffer(-1);
              s.SetDataLength(l+bp-di);
              FillBuffer();
              di =  bp;
            }
            if (bytes_in_buffer && bp<bytes_in_buffer && (ch=*(buffer+bp))=='\n')
              bp++;
          }
          //s +=  TCIString((TCICHAR*)buffer+di, bp-di);
          int l =  s.GetLength();
          TCICHAR* ptr =  s.GetBuffer(l+bp-di+1);
          memcpy(ptr+l, buffer+di, bp-di);
          ptr[l+bp-di] =  0;
          s.ReleaseBuffer(l+bp-di+1);
          s.SetDataLength(l+bp-di);
          return CFR_OK;
        }
      }

      if (di != bp) {
        //s +=  TCIString((TCICHAR*)buffer+di, bp-di);
        int l =  s.GetLength();
        TCICHAR* ptr =  s.GetBuffer(l+bp-di+1);
        memcpy(ptr+l, buffer+di, bp-di);
        ptr[l+bp-di] =  0;
        s.ReleaseBuffer(l+bp-di+1);
        s.SetDataLength(l+bp-di);
      }
      FillBuffer();
      di =  bp;
    } while (bytes_in_buffer);

	  // Note: May falsely return EOF if the
    //       line-ends have been stripped.
    if (s.IsEmpty() && !bytes_in_buffer)
      rv =  CFR_EOF;    // reached end-of-file

  } else {                        // no buffer
    TCI_ASSERT(FALSE);
    return CFR_ReadError;
  }

  return rv;
}


U32 ChamFile::FileSize()
{
  return filesize;
}


TCI_BOOL ChamFile::ChangeSize( I32 new_size ) {

  TCI_BOOL rv  =  FALSE;

  TCI_VERIFY( SEEK( filespec, new_size, FILE_BEGIN ) != -1L );
  if ( SetEndOfFile( (HANDLE)filespec ) ) {
    if ( buffer!=(U8*)NULL ) {
      if ( new_size <= fp ) {
        bytes_in_buffer =  bp =  0;
        fp =  new_size;
      } else if ( new_size < fp+bytes_in_buffer ) {
        bytes_in_buffer =  new_size - fp;
        if ( bp > bytes_in_buffer )
          bp  =  bytes_in_buffer;
      }
    }
    filesize  =  new_size;
    rv  =  TRUE;
  }

  TCI_VERIFY( SEEK(filespec, fp, FILE_BEGIN) != -1L );
  return rv;
}


I32 ChamFile::GetCurPos() {

  return buffer ? fp+bp : SEEK( filespec, 0L, FILE_CURRENT );
}


const FileSpec* ChamFile::GetFName() {

  return fname;
}


I32 ChamFile::FileTime()
{
  if (filespec==NULL)
    return -1;

  // For URLs, maybe we can get better information from the cache
  
  FILETIME lastwrite;
  TCI_VERIFY( ::GetFileTime( (HANDLE)filespec, NULL, NULL, &lastwrite ) );
  TCITime tTime(lastwrite);
  return( tTime.GetTime() );
}


void ChamFile::FillBuffer() {

  if ( openmode & CHAM_FILE_WRITE )
    if ( buffer_dirty )
      FlushBuffer( 1 );

  if ( SEEK( filespec, fp+bp, FILE_BEGIN ) >= 0 ) {
    DWORD n = 0;
    BOOL res =  READ( filespec, buffer, bufsize, n );
    if (res) {
      bytes_in_buffer =  n;
      fp  +=  bp;
    } else {
#ifdef TESTING
      U32 readerr = GetLastError();
      TCI_ASSERT(FALSE);
#endif
      bytes_in_buffer =  0;
    }
    bp  =  0;
  } else {
    TCI_ASSERT(FALSE);    // seek failed
    bytes_in_buffer =  0;
  }
}


ChamFile::CFReturn ChamFile::FlushBuffer( TCI_BOOL mode ) {

  CFReturn rv =  CFR_OK;
  if ( buffer!=(U8*)NULL && bytes_in_buffer && bp ) {

    if ( SEEK( filespec, fp, FILE_BEGIN ) >= 0 ) {

      DWORD lim =  mode ? bp : bytes_in_buffer;
      DWORD nb;
      BOOL res =  WRITE( filespec, buffer, lim, nb );
      if ( !res ) {
        TCI_ASSERT( FALSE );
        rv =  CFR_WriteError;
      } else if ( nb<lim ) {
        TCI_ASSERT( FALSE );
        rv =  CFR_DiskFull;
      } else {
        fp  +=  lim;
        bp  =  0;
        bytes_in_buffer =  0;
        buffer_dirty  =  FALSE;
        rv =  CFR_OK;
      }
    } else
      rv =  CFR_DiskError;
  }

  return rv;
}

ChamFile::CFReturn ChamFile::ToBuffer(const TCIString &source) {

  CFReturn rv =  CFR_OK;
  U32 numbytes = source.GetLength();

  if ( bp+numbytes > (U32)bufsize )
    if ( (rv=FlushBuffer(1)) != CFR_OK ) return rv;

  if ( numbytes> (U32)bufsize ) {
    DWORD nb =  0;
    BOOL res =  WRITE( filespec, (const char *)source, numbytes, nb );
    if ( !res ) {
      TCI_ASSERT( FALSE );
      rv =  CFR_WriteError;
    } else if ( nb<numbytes ) {
      TCI_ASSERT( FALSE );
      rv =  CFR_DiskFull;
    } else {
      bytes_in_buffer =  bp =  0;
      fp  +=  nb;
      if (fp > (long)filesize) 
        filesize =  (U32)fp;
      rv =  CFR_OK;
    }

  } else {
    memcpy( (char*)(buffer+bp),(const char *)source,numbytes );
    bp  +=  numbytes;
    if ( bp > bytes_in_buffer ) {
      bytes_in_buffer =  bp;
      if ( fp+bp > (long)filesize )
        filesize  =  (U32)fp + bp;
    }
    buffer_dirty  =  TRUE;
    rv =  CFR_OK;
  }
  return rv;
}


// Fast unbuffered read from file at specified offset.
ChamFile::CFReturn ChamFile::RawReadBytes(I32 offset, TCICHAR* destBuffer, U32 numBytes, U32& bytes)
{
  CFReturn rv =  CFR_OK;
  bytes = 0;
  if (SEEK(filespec, offset, FILE_BEGIN) >= 0) {
    fp = offset;
    if ( !READ(filespec, destBuffer, numBytes, bytes) )
      rv = CFR_ReadError;
  }
  else
    rv = CFR_ReadError;
  return rv;
}


// Fast unbuffered write to file at specified offset.
ChamFile::CFReturn ChamFile::RawWriteBytes(I32 offset, const TCICHAR* srcBuffer, U32 numBytes, U32& bytes)
{
  CFReturn rv =  CFR_OK;
  bytes = 0;

  if (SEEK(filespec, offset, FILE_BEGIN) >= 0) {
    BOOL res =  WRITE( filespec, srcBuffer, numBytes, bytes );
    if ( !res ) {
      TCI_ASSERT( FALSE );
      rv =  CFR_WriteError;
    } else if ( bytes < numBytes ) {
      TCI_ASSERT( FALSE );
      rv =  CFR_DiskFull;
    }
  } else
    rv =  CFR_DiskError;
  return rv;
}


#if INET_ON

TCI_BOOL ChamFile::IntConstruct(const FileSpec& urlSpec, TCIString& cachefile)
{
  if (InetTranslator == NULL)
  {
    TCI_ASSERT(FALSE);
    return FALSE;
  }  

  TCI_BOOL rv = InetTranslator->Open(urlSpec,cachefile.GetBuffer(CHAM_MAXPATH));
  cachefile.ReleaseBuffer();
  
  return rv;
}


void ChamFile::IntDestruct()
{
  if (fname && InetTranslator)
    TCI_VERIFY( InetTranslator->Close(*fname) );
}
#endif



I32 ChamFile::TCISetFilePointer(void* hFile, LONG lDistanceToMove, DWORD dwMoveMethod)
{
  return SetFilePointer((HANDLE)hFile, lDistanceToMove, NULL, dwMoveMethod);
}


TCI_BOOL ChamFile::TCIReadFile(void* hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead)
{
 return ReadFile((HANDLE)hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, NULL) ? TRUE : FALSE;
}


TCI_BOOL ChamFile::TCIWriteFile(void* hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten)
{
 return WriteFile((HANDLE)hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, NULL) ? TRUE : FALSE;
}

