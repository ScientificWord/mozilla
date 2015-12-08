
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "filesys.h"
#include "filespec.h"
#include "chamfile.h"
#include "bytearry.h"
#include "tcitime.h"
#include "strutil.h"
#include "findfrst.h"

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
#define NOUSER            
#define NONLS             
#define NOMB              
#define NOMEMMGR          
#define NOMETAFILE        
#define NOMINMAX          
#define NOMSG             
#define NOOPENFILE        
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
#include <io.h>
#include <sys/stat.h>


TCI_BOOL FileSys::Exists(const FileSpec& fspec)
{
   if (fspec.IsURL()) {
    #ifdef INET_ON
      // ChamFile constructor starts downloading document if it
      // exists.  It returns from the constructor after the
      // header packet is received from the server.
      ChamFile* fp = TCI_NEW(ChamFile(fspec, ChamFile::CHAM_FILE_READ));
      TCI_BOOL  bv = fp->Readable();
      delete fp;    // This aborts the downloading
      return bv;
    #else
      // Should not test existence of URLs without internet support
      TCI_ASSERT(FALSE);  
      return FALSE;
    #endif
  }

  TCI_BOOL rv  =  FALSE;
  if (!fspec.IsEmpty()) {
    TCIString fileName = fspec.GetFullPath();
    TCICHAR* buf =  fileName.GetBuffer(CHAM_MAXPATH);
    int len =  fileName.GetLength() - 1;
    TCICHAR ch =  buf[len];
    if (ch == '/' || ch == '\\' || ch == ':')
      buf[len] =  0;
    struct stat fs;
    rv =  stat(buf, &fs)==0;
    fileName.ReleaseBuffer();
  }

  return rv;
}


FileSys::FReturn FileSys::InterpretSysError(const FileSpec& fspec) {
  U32 res =  GetLastError();
  FileSys::FReturn rv = FILE_NOT_FOUND;

  switch( res ) {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_NOT_READY:         rv =  FILE_NOT_FOUND;   break;
    case ERROR_INVALID_DRIVE:     rv =  DRIVE_NOT_FOUND;  break;
    case ERROR_SHARING_VIOLATION:
    case ERROR_LOCK_VIOLATION:
    case ERROR_ACCESS_DENIED:     rv =  ACCESS_DENIED;    break;
    case ERROR_BAD_NETPATH:       rv =  BAD_NETPATH;      break;

    case ERROR_PATH_NOT_FOUND: {  
      //this error is returned also if a drive letter is completely invalid; check this
      rv =  PATH_NOT_FOUND;
      FileSpec drivespec(fspec.GetDrive());
      res = GetFileAttributes((const TCICHAR*)drivespec);
      if (res==-1) {
        if (GetLastError() == ERROR_BAD_NETPATH)
          rv = BAD_NETPATH;
        else
          rv = DRIVE_NOT_FOUND;
      }
    }
    break;

    default:
      TCI_ASSERT( FALSE );
    break;
  }
  return rv;
}

FileSys::FReturn FileSys::Exists(const FileSpec& fspec, U32 mode){

  if (fspec.IsURL()) {
    #ifdef INET_ON
      // ChamFile constructor starts downloading document if it
      // exists.  It returns from the constructor after the
      // header packet is received from the server.
      ChamFile* fp = TCI_NEW(ChamFile(fspec, ChamFile::CHAM_FILE_READ));
      TCI_BOOL bv = fp->Readable();
      delete fp;

      FReturn rv;
      if (bv) {
        if (mode & ChamFile::CHAM_FILE_WRITE)
          rv = ACCESS_DENIED;
        else
          rv = FILE_EXISTS;
      }else{
        rv = FILE_NOT_FOUND;
      }

      return rv;
    #else
      // Should not test existence of URLs without internet support
      TCI_ASSERT(FALSE);  
      return FILE_NOT_FOUND;
    #endif
  }


  FReturn rv =   FILE_EXISTS;
  TCIString fileName = fspec.GetFullPath();
  DWORD   res =  GetFileAttributes((const char*)fileName);
  if ( res==-1 ) {
    // file access denied - find out why
    rv = InterpretSysError(fspec);
  } else if ( (mode&ChamFile::CHAM_FILE_WRITE) && (res&FILE_ATTRIBUTE_READONLY) ) {
    rv =  ACCESS_DENIED;
  }  
  return rv;
}



TCI_BOOL FileSys::IsDirectory(const FileSpec& fspec) {
  if (!fspec.IsValid() || fspec.IsURL())
    return FALSE;

  DWORD res =  GetFileAttributes(fspec.GetFullPath());
  return (res!=-1) && (res & FILE_ATTRIBUTE_DIRECTORY);
}

TCI_BOOL FileSys::IsFile(const FileSpec& fspec,FileSys::FReturn* pError){
  if (fspec.IsURL())
    return TRUE;

  DWORD res =  GetFileAttributes(fspec.GetFullPath());
  if (res==-1) {
    if (pError)
      *pError = InterpretSysError(fspec);
    return FALSE;
  } else 
    return ((res & FILE_ATTRIBUTE_DIRECTORY)==0);
}

int FileSys::GetDrvType(const FileSpec& fspec)
{
  TCIString drive = fspec.GetDrive();
  if (drive.IsEmpty())
    return -1;
  int drvtype =  ::GetDriveType(drive);
  return drvtype;
}


TCI_BOOL FileSys::IsOnFixedDrive(const FileSpec& fspec)
{
  return (GetDrvType(fspec) == DRIVE_FIXED);
}

TCI_BOOL FileSys::IsOnRemoteDrive(const FileSpec& fspec)
{
  if (fspec.IsURL() || fspec.IsUNCName()) 
  {
    return TRUE;
  }
  else
    return (GetDrvType(fspec) == DRIVE_REMOTE);
}


TCI_BOOL FileSys::IsOnRemovableDrive(const FileSpec& fs)
{
  return (GetDrvType(fs) == DRIVE_REMOVABLE);
}


TCI_BOOL FileSys::IsOnCDRom(const FileSpec& fspec)
{
  return (GetDrvType(fspec) == DRIVE_CDROM);
}

TCI_BOOL FileSys::IsOnWritableDirectory(const FileSpec& fspec, TCI_BOOL allow_remote)
{
  if (fspec.IsURL())
  {
    return FALSE;
  }

  int typ =  GetDrvType(fspec);
  TCI_BOOL rv =  typ == DRIVE_FIXED || typ == DRIVE_RAMDISK || typ == DRIVE_REMOVABLE;
  if (!rv && allow_remote && typ == DRIVE_REMOTE)
    rv =  TRUE;
  if (rv) {
    // attemp to create a file on the directory
    TCI_ASSERT(!(fspec.GetDir().IsEmpty()));
    FileSpec trial(fspec.GetDir());
    trial.MakePath();
    TCIString str(trial.GetFullPath());
    TCICHAR buff[10];
    memset(buff, 0, sizeof(buff));
    int pos =  0;
    TCICHAR ch = 'a';
    HANDLE fh;
    SECURITY_ATTRIBUTES sa;
    sa.nLength =  sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor =  NULL;
    sa.bInheritHandle =  FALSE;
    do
    {
      if (!isalpha(ch)) {
        buff[pos] = '_';
        pos++;
        ch =  'a';
      }
      buff[pos] =  ch++;

      TCIString f(str + buff);
      DWORD res =  GetFileAttributes((const TCICHAR*)f);
      if (res != 0xFFFFFFFF)
      {
        // file exists -- try something else for a file name
        fh =  0;
        continue;
      }
      else
      {
        fh =  CreateFile(f, GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_NEW	, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fh == INVALID_HANDLE_VALUE) {
          res =  GetLastError();
          if (res == ERROR_FILE_EXISTS || res == ERROR_ALREADY_EXISTS)
            fh =  0;
        } else {
          // created a file -- directory must be writable
          CloseHandle(fh);
          DeleteFile(f);
          break;
        }
      }
    } while (pos<9 && fh != INVALID_HANDLE_VALUE);
    rv =  fh != INVALID_HANDLE_VALUE;
  }

  return(rv);
}

              
U32 FileSys::GetRemoteNameInfoStruct(const FileSpec& fspec,ByteArray& buff) {
  U32 remoteinfosize(1024);
  buff.ReSize(remoteinfosize);
  U8* unamebuff = buff.Lock();
  U32 res = WNetGetUniversalName(fspec.GetFullPath(), REMOTE_NAME_INFO_LEVEL,unamebuff,&remoteinfosize);
  if (res==ERROR_MORE_DATA) {
    buff.Unlock();
    buff.ReSize(remoteinfosize);
    unamebuff = buff.Lock();
    res = WNetGetUniversalName(fspec.GetFullPath(), REMOTE_NAME_INFO_LEVEL,unamebuff,&remoteinfosize);
  }
  buff.Unlock();
  buff.SetByteCount(remoteinfosize);
  return res;
}


//this function is intended to search shared drives belonging to a remote (but NOT internet!) server.
//The problem being addressed is the impossibility of resolving an "absolute" file reference in a 
//document located on a remote machine.
TCI_BOOL FileSys::SearchRemoteContainerForFile(FileSpec& fspec) {
  TCI_BOOL rv = FALSE;
  if (!fspec.IsURL() && IsOnRemoteDrive(fspec)) {
    TCIString udrive;
    TCI_BOOL wasUNC = fspec.IsUNCName();
    U32 res = NO_ERROR;
    FileSpec olddrive(fspec.GetDrive());
    if (wasUNC) {
      udrive = fspec.GetDrive();
    } else {
      ByteArray unameba;
      res = FileSys::GetRemoteNameInfoStruct(fspec,unameba);
      if (res==NO_ERROR) {
        const U8* unamebuff = unameba.Lock();
        udrive = (TCICHAR*)((REMOTE_NAME_INFO*)unamebuff)->lpConnectionName;
        unameba.Unlock();
      }
    }  //if !wasUNC
    if (udrive.GetLength() > 0) {
      int parentlen = -1;
      TCICHAR ch = udrive[parentlen+1];
      while (parentlen < udrive.GetLength() && (ch=='\\' || ch=='/')) {
        parentlen++;
        ch = udrive[parentlen+1];
      }
      parentlen = udrive.Find('\\',parentlen+1);
      if (parentlen<=0)
        parentlen = udrive.Find('/',parentlen+1);
      TCIString remoteparent(parentlen<=0 ? udrive : udrive.Left(parentlen));
      // typedef struct _NETRESOURCE {  // nr 
      //DWORD  dwScope; 
      //DWORD  dwType; 
      //DWORD  dwDisplayType; 
      //DWORD  dwUsage; 
      //LPTSTR lpLocalName; 
      //LPTSTR lpRemoteName; 
      //LPTSTR lpComment; 
      //LPTSTR lpProvider; 
      //} NETRESOURCE;
      TCICHAR* remoteparentbuff = remoteparent.GetBuffer(0); 
      NETRESOURCE netParent = { RESOURCE_GLOBALNET,RESOURCETYPE_DISK,RESOURCEDISPLAYTYPE_GENERIC,
                                  RESOURCEUSAGE_CONTAINER,NULL,remoteparentbuff,NULL,NULL };

      HANDLE enumHandle = NULL;
      res = WNetOpenEnum(RESOURCE_GLOBALNET,RESOURCETYPE_DISK,0,&netParent,&enumHandle);
      if (res==NO_ERROR) {
        U32 numentries = 1;
        U32 netresourcesize = 256;
        ByteArray netresba(netresourcesize);
        U8* netresourcebuff = netresba.Lock();
        while (!rv) {
          res = WNetEnumResource(enumHandle,&numentries,netresourcebuff,&netresourcesize);
          if (res==ERROR_NO_MORE_ITEMS)
            break;
          if (res==ERROR_MORE_DATA) {
            netresba.Unlock();
            netresba.ReSize(netresourcesize);
            netresourcebuff = netresba.Lock();
            res = WNetEnumResource(enumHandle,&numentries,netresourcebuff,&netresourcesize);
          }
          NETRESOURCE* netResource = (NETRESOURCE*)netresourcebuff;
          FileSpec trial(netResource->lpRemoteName);
          fspec.SetDriveSameAs(trial);
          rv = Exists(fspec);
        }
        res = WNetCloseEnum(enumHandle);
        if (!rv)
          fspec.SetDriveSameAs(olddrive);
        netresba.Unlock();
      }
    }
  }

  return rv;
}


int FileSys::GetFileData(const FileSpec& fspec, GFData type)
{
  int rv =  -1;

  if (fspec.IsURL()) {
    #ifdef INET_ON
      // ChamFile constructor starts downloading document if it
      // exists.  It returns from the constructor after the
      // header packet is received from the server.
      ChamFile* fp = TCI_NEW(ChamFile(fspec, ChamFile::CHAM_FILE_READ));
      TCI_BOOL bv = fp->Readable();
      if (bv) {
        if (type==GFD_Time)
          rv = fp->FileTime();
        else if (type==GFD_Size)
          rv = fp->FileSize();
        else
          TCI_ASSERT(FALSE);
      }

      delete fp;    // This aborts the downloading
      return rv;
    #else
      // Should not be doing this without internet support
      TCI_ASSERT(FALSE);  
      return rv;
    #endif
  }

  SECURITY_ATTRIBUTES sa;
  sa.nLength =  sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor =  NULL;
  sa.bInheritHandle =  FALSE;
  HANDLE fh =  CreateFile(fspec.GetFullPath() , GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( fh != INVALID_HANDLE_VALUE ) {
    if (type == GFD_Time) {
      FILETIME lastwrite;
      TCI_VERIFY( ::GetFileTime(fh, NULL, NULL, &lastwrite) );
      TCITime tTime(lastwrite);
      rv = (I32)tTime.GetTime();
    } else if (type == GFD_Size) {
      rv =  GetFileSize(fh, NULL);
    }
    TCI_VERIFY( CloseHandle( fh ) );
  }
  return rv;
}


int FileSys::Size(const FileSpec& fspec) {

  return GetFileData(fspec, GFD_Size);
}


time_t FileSys::Time(const FileSpec& fspec){

  int rv = GetFileData(fspec, GFD_Time);
  if (rv == -1)  //Error! Try FindNextFile instead:
  {
    FindFiles fileSrch;
    rv = (time_t)fileSrch.GetFileTime(fspec.GetFullPath());
  }
  return (time_t)rv;
}


TCI_BOOL FileSys::SetTime(const FileSpec& fspec, int newtime){

  TCI_BOOL rv =  FALSE;

  if (fspec.IsURL())
    return rv;

  SECURITY_ATTRIBUTES sa;
  sa.nLength =  sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor =  NULL;
  sa.bInheritHandle =  FALSE;
  HANDLE fh =  CreateFile(fspec.GetFullPath(), GENERIC_WRITE, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( fh != INVALID_HANDLE_VALUE ) {
    TCITime tTime(newtime);
    FILETIME ftime;
    tTime.GetFILETIME(ftime);
    TCI_VERIFY(rv =  (SetFileTime(fh, NULL, NULL, &ftime) != 0));
    TCI_VERIFY(CloseHandle(fh));
  }

  return(rv);
}


TCI_BOOL FileSys::Touch(const FileSpec& fspec, TCI_BOOL force){

  TCI_BOOL rv =  FALSE;
  if (fspec.IsURL())
    return rv;
  TCI_BOOL change_mode =  FALSE;
  if (force && !HasMode(fspec, ChamFile::CHAM_FILE_WRITE)) {
    change_mode =  TRUE;
    SetMode(fspec, ChamFile::CHAM_FILE_WRITE);
  }

  SECURITY_ATTRIBUTES sa;
  sa.nLength =  sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor =  NULL;
  sa.bInheritHandle =  FALSE;
  HANDLE fh =  CreateFile(fspec.GetFullPath(), GENERIC_WRITE, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( fh != INVALID_HANDLE_VALUE ) {
    SYSTEMTIME stime;
    GetSystemTime(&stime);
    FILETIME ftime;
    SystemTimeToFileTime(&stime, &ftime);
    rv =  (SetFileTime(fh, NULL, NULL, &ftime) != 0);
    TCI_ASSERT(rv);
    TCI_VERIFY(CloseHandle(fh));
  }

  if (change_mode)
    SetMode(fspec, ChamFile::CHAM_FILE_READ);

  return(rv);
}




time_t FileSys::CompareDateTime(const FileSpec& fspec1, const FileSpec& fspec2){

  time_t dt1 =  Time(fspec1);
  time_t dt2 =  Time(fspec2);
  return (dt2<0 ? dt2 : dt1 - dt2);
}


////////   File Manipulation   ////////////////////
TCI_BOOL FileSys::Delete(const FileSpec& fspec) {

  if (fspec.IsURL()) {
    return TRUE;  //don't want to do anything here!
  } 
  
  DWORD res =  GetFileAttributes(fspec.GetFullPath());
  if (res==-1) {
    FReturn fret = InterpretSysError(fspec);
    return (fret == FILE_NOT_FOUND);
  } else if ((res & FILE_ATTRIBUTE_DIRECTORY)!=0) {
    //it's a directory; try RemoveDirectory()
    TCI_BOOL rv =  (RemoveDirectory(fspec.GetFullPath()) != 0);
    if (!rv) {
      DWORD res =  GetLastError();
      rv =  (res == ERROR_FILE_NOT_FOUND);
    }
    return rv;
  } else {
    //it exists and is a file
    TCI_BOOL rv = (DeleteFile(fspec.GetFullPath()) != 0);
    if (!rv) {
      DWORD res =  GetLastError();
      rv =  res == ERROR_FILE_NOT_FOUND;
    }
    return rv;
  }

}


TCI_BOOL FileSys::DeleteAll(const FileSpec& fspec)
{
  TCI_BOOL rv =  TRUE;
  FileSpec path(fspec.GetDir());

  TCIString f(fspec.GetFullPath());
  int off =  f.ReverseFind('.');
  if (off >= 0)
    f.SetDataLength(+ off);
  f +=  ".*";
  FindFiles findFiles;
  TCIString file;
  if (findFiles.FindFirst(f, file)) {
    do {
      FileSpec g(path.GetFullPath());
      g += file;
      rv =  rv && Delete(g);
    } while (findFiles.FindNext(file));
  } else {
    rv =  FALSE;
  }
  findFiles.EndFind();

  return rv;
}

TCI_BOOL FileSys::ExistsSameBase(const FileSpec& fspec)
{
  TCI_BOOL rv =  TRUE;
  FileSpec path(fspec.GetDir());

  TCIString f(fspec.GetFullPath());
  int off =  f.ReverseFind('.');
  if (off >= 0)
    f.SetDataLength(+ off);
  f +=  ".*";
  
  TCIString buf;
  FindFiles findFiles;
  rv = findFiles.FindFirst(f, buf); 
  findFiles.EndFind();
  return rv;
}
   

#define CHAM_BLOCK_COPY ChamFile::CF_MIN_BUFFER_SIZE

TCI_BOOL FileSys::Copy(const FileSpec& source, const FileSpec& dest) {

  if (!source.GetFullPath().CompareNoCase(dest.GetFullPath()) )
    return TRUE;

#ifndef INET_ON
  // Should not copy URLs without internet support
  if (source.IsURL()  || dest.IsURL()) {
    TCI_ASSERT(FALSE);  
    return FALSE;
  }
#endif

  TCI_BOOL rv = FALSE;
  if (!source.IsURL()) {
    // Use FALSE to allow Windows to overwrite an existing file,
    // TRUE to prevent destination file from being overwritten.

    // CopyFile can fail on Win95 if there are sharing complications!
    rv = CopyFile(source.GetFullPath(), dest.GetFullPath(), FALSE) != 0;
  }
  
  if (!rv) {
#ifdef TESTING
    U32 winErr = GetLastError();
#endif
    // source file is an URL or CopyFile failed
    ChamFile* fpSrc = NULL;
    ChamFile* fpDest = NULL;
    fpSrc = TCI_NEW(ChamFile(source, ChamFile::CHAM_FILE_READ));
    rv = fpSrc->Readable();
    if (rv) {
      fpDest = TCI_NEW(ChamFile(dest, ChamFile::CHAM_FILE_WRITE | ChamFile::CHAM_FILE_CREATE));
      rv = fpDest->Writeable();
    }
    if (rv) {
      I32 offset = 0;
      TCICHAR* buff = TCI_NEW(TCICHAR[CHAM_BLOCK_COPY]);
      U32 bytes1, bytes2, to_be_read;
      do {
        to_be_read = CHAMmin(CHAM_BLOCK_COPY,fpSrc->FileSize()-offset);
        if (to_be_read==0) {
          bytes1 = bytes2 = 0;
        } else {
          if (ChamFile::CFR_OK==fpSrc->RawReadBytes(offset, buff, to_be_read, bytes1)
               && ChamFile::CFR_OK==fpDest->RawWriteBytes(offset, buff, bytes1, bytes2))
            offset += bytes1;
          else  //set up error condition to ensure leaving loop and returning FALSE! rwa, 1-28-00
          {
            bytes1 = to_be_read;
            bytes2 = 0;
          }
        }
      } while (bytes1 && (bytes1 == bytes2));

      rv = (bytes1 == bytes2);
      delete[] buff;
    }

    delete fpSrc;
    delete fpDest;
  }

  return rv;
}


TCI_BOOL FileSys::Rename(const FileSpec& fspec, const FileSpec& dest){

  if (fspec.IsURL())
    return FALSE;
    
#if TCIENV(CHAMWIN32)
  TCI_BOOL rv =  MoveFile(fspec.GetFullPath(), dest.GetFullPath()) ? TRUE : FALSE;
  //TCI_BOOL rv =  MoveFileEx(name, dest.name, MOVEFILE_COPY_ALLOWED);
# ifdef TESTING
  if (!rv) 
  {
    DWORD res =  GetLastError();
    TCI_ASSERT(res != NO_ERROR);
  }
# endif
  return(rv);
#else
  //
  // This is basically an implementation of rename, since
  // rename will not move files across filesystems on
  // different devices. The code below will do that.
  //
  int retval = FALSE;
  FILE *newfp, *oldfp;
  if ( newfp = fopen(dest.GetFullPath(), "w+") ) {
    if ( oldfp = fopen(fspec.GetFullPath(), "r") ) {
      char* buf =  TCI_NEW(char[CHAM_BLOCK_COPY]);
      size_t res;
      while ((res=fread(buf, 1, CHAM_BLOCK_COPY, oldfp)) != 0)
        fwrite(buf, 1, res, newfp);
      retval = TRUE;
      fclose(oldfp);
      unlink(fspec.GetFullPath());
      delete [] buf;
    } else {
      TCI_ASSERT( FALSE );
    }
    fclose(newfp);
  } else {
    TCI_ASSERT( FALSE );
  }

  return (retval != 0);
#endif
}


TCI_BOOL FileSys::SetMode(const FileSpec& fspec, U32 mode) {

  TCI_ASSERT(!fspec.IsURL());
  int amode =  0;
  if (mode & ChamFile::CHAM_FILE_READ)    amode |=  S_IREAD;
  if (mode & ChamFile::CHAM_FILE_WRITE)   amode |=  S_IWRITE;
  return chmod(fspec.GetFullPath(), amode) == 0;
}

TCI_BOOL FileSys::HasMode(const FileSpec& fspec, U32 mode){

  TCI_ASSERT((mode == ChamFile::CHAM_FILE_READ) || (mode == ChamFile::CHAM_FILE_WRITE));
  if (fspec.IsEmpty())
    return(FALSE);
  if (fspec.IsURL())
    return FALSE;

  DWORD attrs =  GetFileAttributes(fspec.GetFullPath());
  if (mode & ChamFile::CHAM_FILE_READ)
    return((attrs & FILE_ATTRIBUTE_NORMAL) != 0);

  TCI_ASSERT(mode & ChamFile::CHAM_FILE_WRITE);
  return((attrs & FILE_ATTRIBUTE_READONLY) == 0);
}


TCIString FileSys::GenerateFileKey(int nSeqNumber)
{
  //Encode the current time in seconds since January 1, 1970 as a
  //6 digit base 36 number.
//  retStr.GetBuffer(8);
  U32 t = (U32)time(NULL);
  TCIString retStr = StrUtil::BasedNumberString(t, 36, NULL, 6, TRUE);
//  U16 m;
//  for (int i=0; i<6; i++)
//  {
//    m = U16(t%36);
//    t = t/36;
//    retStr.SetAt(5-i, Base36Digit(m));
//  }
//
////  //Encode the result of rand() as a base 36 number and use the last
////  //two digits as the last two characters of the key.
////
////  int r = rand();
////  for (i=0; i<2; i++){
////    m = r%36;
////    r = r/36;
////    key.key[7-i] = Base36Digit(m);
////  }

  retStr += StrUtil::BasedNumberString(nSeqNumber, 36, NULL, 2, TRUE);
//  retStr.SetAt(6, Base36Digit(nSeqNumber/36));
//  retStr.SetAt(7, Base36Digit(nSeqNumber%36));

  return retStr;
}

FileSpec FileSys::CreateTemp(const FileSpec& path, const TCIString& prefix,
                                const TCIString& ext, TCI_BOOL unique)
{
  TCI_ASSERT(prefix.GetLength()<5);
  int prefix_len =  CHAMmin(4, prefix.GetLength());

  TCIString usePrefix(prefix.Left(prefix_len));
  CreateLongTempCallback pConditionsCallback = NULL;
   if (unique)
     pConditionsCallback = &FileSys::Unique;
  return CreateLongTemp(path, usePrefix, ext, pConditionsCallback, NULL);
}

FileSpec FileSys::CreateLongTemp(const FileSpec& path, const TCIString& prefix,
                                   const TCIString& ext, 
                                     CreateLongTempCallback pConditionsCallback,
                                      void* callbackData)
{
  FileSpec f(path);
  if (f.IsEmpty()) {
    TCI_ASSERT(FALSE);  //annoying
  }

  f.MakePath();

  TCIString fname(f + prefix + "%04d" + ext);
  FileSpec rv;
  int i =  0;
  while (i<10000)
  {
    TCIString temp(StrUtil::Format(fname, i));
    FileSpec fs(temp);
    if (!Exists(fs) && ((pConditionsCallback == NULL)|| (*pConditionsCallback)(fs, callbackData)))
    {
      rv =  fs;
      break;
    }
    i++;
  }

  return rv;
}


void FileSys::ClearTemp(const FileSpec& path, const TCIString& prefix)
{
  FileSpec f(path);
  if (f.IsEmpty()) {
    TCI_ASSERT(FALSE);
  }

  f.MakePath();

  TCI_ASSERT(prefix.GetLength()<5);
  int prefix_len =  CHAMmin(4, prefix.GetLength());

  TCIString fname(f.GetFullPath() + prefix.Left(prefix_len) + "*");
  fname += ".*";
  TCIString buf;
  FindFiles findFiles;
  if (findFiles.FindFirst(fname, buf)) {
    do {
      FileSpec g(f.GetFullPath());
      g += buf;
      TCIString x = g.GetExtension();
      x.MakeLower();      // doesn't change g
      if ( x==".tex" || x==".aux" || x==".dvi" || x==".log" ||
           x==".bbl" || x==".blg" || x==".glo" || x==".out" || // .out are generated by hyperref package
           x==".idx" || x==".ind" || x==".lof" || x==".lot" || x==".toc" ||
           x.Mid(1,2)=="aa" || // CIndex files
           x==".tmp" || x==".usr" || x==".bdx"  // other database files
        )
        TCI_VERIFY(Delete(g));
    } while (findFiles.FindNext(buf));
  }
  findFiles.EndFind();
}

//ljh 03/03  callbackData is not used here, however Unique is used as callback function
// in CreateTemp. The callback signature has this parameter.
TCI_BOOL FileSys::Unique(const FileSpec& fspec, void* callbackData)
{
  FileSpec f(fspec);
  f.SetExtension(EmptyTCIString);
  TCIString fname = f.GetFullPath();
  fname +=  ".*";   // because '*' is not a valid file name character
  TCIString buf;
  FindFiles findFiles;
  TCI_BOOL rv =  !findFiles.FindFirst(fname, buf);
  findFiles.EndFind();

  return(rv);
}

// returns kbytes on drive. Theoretically, could overflow a U32.

U32 FileSys::GetFreeSpaceOnDrive(const FileSpec& fspec) 
{
  TCIString drive = fspec.GetDrive();

  if (drive.IsEmpty())
    return 0;

  DWORD sectorsPerCluster = 0;	   // Sectors per cluster 
  DWORD bytesPerSector = 0;	       // Bytes per sector 
  DWORD numberOfFreeClusters = 0;	 // Number of free clusters  
  DWORD totalNumberOfClusters = 0; 

  BOOL rv = FALSE;

  rv = ::GetDiskFreeSpace(drive, &sectorsPerCluster,
                          &bytesPerSector, &numberOfFreeClusters,
                          &totalNumberOfClusters);

  if (!rv)
    return 0;

  U32 ans;
  if (bytesPerSector * sectorsPerCluster >= 1024)
  {
    U32 kbytesPerCluster = (bytesPerSector * sectorsPerCluster) >> 10;
    ans = numberOfFreeClusters * kbytesPerCluster;
  }
  else
    ans = (numberOfFreeClusters * bytesPerSector * sectorsPerCluster) >> 10;

  if (ans == 0)  // there might have been an overflow problem...
    ans = numberOfFreeClusters;
  return ans;
}



TCI_BOOL FileSys::ConvertToUNCName(FileSpec& fspec) {
  TCI_BOOL rv = !fspec.IsURL();

  if (rv && !fspec.IsUNCName()) {
    U32 namesize = 256;
    ByteArray ba(namesize);
    U8* unamebuff = ba.Lock();
    U32 res = WNetGetUniversalName(fspec.GetFullPath(), UNIVERSAL_NAME_INFO_LEVEL,unamebuff,&namesize);
    if (res==ERROR_MORE_DATA) {
      if (namesize == 256) {  //this is a kludge to handle MicroSoft's failure to return a reasonable ERROR!!!
        TCIString univnamestr;
        TCICHAR* univnamebuff = univnamestr.GetBuffer(namesize);
        FileSpec drivspec(fspec.GetDrive());
        drivspec.UnmakePath();
        res = WNetGetConnection(drivspec.GetFullPath(),univnamebuff,&namesize);
        if (res==ERROR_MORE_DATA && namesize > 256) {
          univnamebuff = univnamestr.GetBuffer(namesize);
          res = WNetGetConnection(drivspec.GetFullPath(),univnamebuff,&namesize);
        }
        if (res==NO_ERROR || res==ERROR_CONNECTION_UNAVAIL) {
          univnamestr.ReleaseBuffer();
          drivspec = univnamestr;
          fspec.SetDriveSameAs(drivspec);
        }
      } else {
        ba.Unlock();
        ba.ReSize(namesize);
        unamebuff = ba.Lock();
        res = WNetGetUniversalName(fspec.GetFullPath(), UNIVERSAL_NAME_INFO_LEVEL,unamebuff,&namesize);
        if (res==NO_ERROR || res==ERROR_CONNECTION_UNAVAIL)
          fspec = FileSpec(((UNIVERSAL_NAME_INFO*)unamebuff)->lpUniversalName);
      }
    } else if (res==NO_ERROR || res==ERROR_CONNECTION_UNAVAIL)
      fspec = FileSpec(((UNIVERSAL_NAME_INFO*)unamebuff)->lpUniversalName);
    rv = fspec.IsUNCName();
    ba.Unlock();
  }
  return rv;
}


//Convert a file path to a form suitable for use in an HTML document (for instance);
//in particular, if not relative or an URL, should be of the "file://domainName/driveName/path"
//form.
TCIString FileSys::GetURIFileString(const FileSpec& filePath, const FileSpec& basePath)
{
  FileSpec ourPath(filePath);
  if (FileSys::IsDirectory(ourPath))
    ourPath.MakePath();
  TCIString rv(ourPath.ConvertPath(CF_INTERNAL));
  if (filePath.IsAbsolute() && !filePath.IsURL())
  {
    FileSpec baseDir(basePath);
	if (!FileSys::IsDirectory(baseDir))
	  baseDir = baseDir.GetDir();
	baseDir.MakePath();
    ourPath.SetBasePath(baseDir);
    if (ourPath.MakeRelative())
    {
      rv = ourPath.ConvertPath(CF_INTERNAL);
    }
    else
    {
      //The actual formatting here is: ("file://%s/%s", hostName, fileName).
      //For our purposes, however, it's most easily done as:
      if (filePath.IsUNCName())
	    rv = StrUtil::Format("file:%s",	(const TCICHAR*)rv);
      else
        rv = StrUtil::Format("file:///%s", (const TCICHAR*)rv);
    }
  }
  return rv;
}

// Create all directories in FileSpec. Return TRUE if already existed or could create.
// May need to create parent directories.
// May come as file name or directory name.
TCI_BOOL FileSys::CreateDir(const FileSpec& fspec)
{
  if (!fspec.IsValid() || fspec.IsURL()) {
    TCI_ASSERT(FALSE);
    return FALSE;
  }
  if (fspec == fspec.GetDrive())  // i.e. fspec.IsDrive()
    return IsDirectory(fspec);

  FileSpec fullFS = fspec.FullPath();
  if (!fullFS.IsPath())
    fullFS.MakePath();

  if (!IsDirectory(fullFS))
  {
    fullFS.UnmakePath();
    if (!IsDirectory(fullFS.GetDir()) && !CreateDir(fullFS.GetDir()))
      return FALSE;
  }

  const TCICHAR* buff =  fullFS.GetFullPath();
  
  DWORD attr = GetFileAttributes(buff);
  if (attr == -1 || !(attr & FILE_ATTRIBUTE_DIRECTORY))
  {
    SECURITY_ATTRIBUTES sa;
    sa.nLength =  sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor =  NULL;
    sa.bInheritHandle =  FALSE;

    if (!::CreateDirectory(buff, &sa))
      return FALSE;
  }
  return TRUE;
}


TCI_BOOL FileSys::Touch(const FileSpec& fspec, const FileSpec& fs, TCI_BOOL force) {

  TCI_BOOL rv =  FALSE;
  if (fspec.IsURL())
    return rv;
  TCI_BOOL change_mode =  FALSE;
  if (force && !HasMode(fspec, ChamFile::CHAM_FILE_WRITE)) {
    change_mode =  TRUE;
    SetMode(fspec, ChamFile::CHAM_FILE_WRITE);
  }

#if TCIENV(CHAMWIN32)
  SECURITY_ATTRIBUTES sa;
  sa.nLength =  sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor =  NULL;
  sa.bInheritHandle =  FALSE;
  HANDLE fh =  CreateFile(fs.GetFullPath(), GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( fh != INVALID_HANDLE_VALUE ) {
    FILETIME ftime;
    if (!GetFileTime(fh, NULL, NULL, &ftime)) {
      TCI_ASSERT(FALSE);
      SYSTEMTIME stime;
      GetSystemTime(&stime);
      SystemTimeToFileTime(&stime, &ftime);
    }
    TCI_VERIFY(CloseHandle(fh));
    fh =  CreateFile(fspec.GetFullPath(), GENERIC_WRITE, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if (fh != INVALID_HANDLE_VALUE) {
		rv =  SetFileTime(fh, NULL, NULL, &ftime) ? TRUE :FALSE;
      TCI_ASSERT(rv);
    }
    TCI_VERIFY(CloseHandle(fh));
  }
#else
  TCI_ASSERT(FALSE);    // what to do here???
#endif

  if (change_mode)
    SetMode(fspec, ChamFile::CHAM_FILE_READ);

  return(rv);
}

void FileSys::FindAllFiles(std::vector<FileSpec>& fileList, const TCIString& baseDir, const TCIString& pattern, 
                               TCI_BOOL bRecurse, TCI_BOOL bIncludeDirectories)
{
  FileSpec dirSpec(baseDir);
  dirSpec.MakePath();
  FindFiles findFiles;

  TCIString buf,theFile;
  TCIString searchName(dirSpec.GetFullPath());
  searchName += pattern;

  TCI_BOOL searchDone = !findFiles.FindFirst(searchName, theFile);
  while (!searchDone)
  {
    FileSpec fSpec(dirSpec, theFile);
    if ( bIncludeDirectories || !FileSys::IsDirectory(fSpec) )
      fileList.push_back(fSpec);
    searchDone = !findFiles.FindNext(theFile);
  }
  findFiles.EndFind();
    
  if (bRecurse)  //search subdirectories
  {
    searchName = dirSpec.GetFullPath();
    searchName += "*";
    if (findFiles.FindFirst(searchName,buf))
    {
      do
      {
        if (buf!="." && buf!="..")
        {
          FileSpec subDir(dirSpec,buf);
          if (FileSys::IsDirectory(subDir))
          {
            FindAllFiles(fileList,subDir.GetFullPath(),pattern,
                               bRecurse, bIncludeDirectories);
          }
        }
      }
      while (findFiles.FindNext(buf) && !buf.IsEmpty());
    }
  }
}
