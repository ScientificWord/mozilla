#ifndef CHAMFILE_H
#define CHAMFILE_H

//  ChamFile -- File operations
//

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif


class TCIString;
class ByteArray;
class FileSpec;

#ifdef INET_ON
class INetTranslator;
#endif

class ChamFile {
// MEMBER DATA
private:
  void* filespec; // A file handle of type HANDLE

  U32 openmode;
  U32 filesize;

  FileSpec* fname;
  long      fp;
  U8*       buffer;
  TCI_BOOL  buffer_dirty;
  int       bufsize;
  int       bp;
  int       bytes_in_buffer;
  U32       nFileError;

// TYPES
public:
  enum ClassConstants{ 
           CF_MIN_BUFFER_SIZE =4096,
           CHAM_FILE_READ     =0x0001,
           CHAM_FILE_WRITE    =0x0002,
           CHAM_FILE_CREATE   =0x0004,
           CHAM_FILE_BINARY   =0x0008,
           CHAM_FILE_EXCLUSIVE=0x0010,
		   CHAM_FILE_APPEND   =0x0020
  };
   
   
   
  
  enum CFReturn {
    CFR_OK, CFR_ReadError, CFR_WriteError, CFR_DiskFull, CFR_DiskError,
    CFR_UserAbort, CFR_BadFormat,CFR_EOF
  };
  
// PUBLIC INTERFACE

public:
  ChamFile(const FileSpec&, U32 mode, I32 buf_size=CF_MIN_BUFFER_SIZE );
  ~ChamFile();

  TCI_BOOL Writeable();
  TCI_BOOL Readable();

  CFReturn  WriteString(const TCIString& str);
  CFReturn  ReadString(U8* str, U32 limit);
  CFReturn  ReadString(TCIString&);

  CFReturn  WriteBytes(const TCIString& src, U32& bytes);
  CFReturn  WriteBytes(const TCIString& src, U32 n, U32& bytes); 
  CFReturn  WriteBytes(const ByteArray& src, U32 offset, U32 num, U32& bytes);
  CFReturn  ReadBytes( U8* str, U32 limit, U32& bytes );
  CFReturn  ReadBytes(TCIString&, U32 numbytes, U32& bytes );

  CFReturn RawReadBytes(I32 offset, TCICHAR* destBuffer, U32 numBytes, U32& bytes);
  CFReturn RawWriteBytes(I32 offset, const TCICHAR* srcBuffer, U32 numBytes, U32& bytes);


  CFReturn  Goto(I32);
  TCI_BOOL  AtEnd();
  TCI_BOOL  Flush();

  TCI_BOOL  Delete();

  U32       FileSize();
  I32       GetCurPos();
  TCI_BOOL  ChangeSize(I32 new_size);
  const FileSpec* GetFName();
  U32       GetOpenMode() { return openmode; }
  I32       FileTime();
  U32       GetFileError() { return nFileError; }
	

  I32       TCISetFilePointer(void* hFile, I32 lDistanceToMove, U32 dwMoveMethod);
  TCI_BOOL  TCIReadFile(void* hFile, void* lpBuffer, U32 nNumberOfBytesToRead, U32 * lpNumberOfBytesRead);
  TCI_BOOL  TCIWriteFile(void* hFile, const void* lpBuffer, U32 nNumberOfBytesToWrite, U32 * lpNumberOfBytesWritten);

// PRIVATE INTERFACE
private:
  void      FillBuffer();
  CFReturn  FlushBuffer( TCI_BOOL mode );
  CFReturn  ToBuffer(const TCIString &source);


#ifdef INET_ON
  ////
  //// Internet Implementation Stuff
  ////
public:
  static void     SetINetTranslator( INetTranslator* p) { InetTranslator = p; }

private:
  TCI_BOOL  m_isURL;     // TRUE if file is an URL

  TCI_BOOL  IntConstruct(const FileSpec& fURL, TCIString& cachename);
  void      IntDestruct();

  static    INetTranslator* InetTranslator;
  
#endif

};

#endif // ndef ChamFile_h
