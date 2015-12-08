
#ifndef FILESYS_H
#define FILESYS_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

#ifndef STD_TIME_H
  #include <time.h>
#endif
#ifndef TCISTRIN_H
  #include "tcistrin.h"
#endif
#include <vector>

class FileSpec;
class FileIterator;
class ByteArray;


typedef TCI_BOOL (*CreateLongTempCallback)(const FileSpec& filename, void* data);


struct FileSys{
  enum FReturn {
    FILE_EXISTS, FILE_NOT_FOUND, PATH_NOT_FOUND, DRIVE_NOT_FOUND,
    ACCESS_DENIED, BAD_FILE_SPEC, BAD_NETPATH
  };
  
  enum GFData { GFD_None, GFD_Time, GFD_Size };
  

  static  FReturn   InterpretSysError(const FileSpec& fspec);
  static  TCI_BOOL  Exists(const FileSpec&);  
  static  FReturn   Exists(const FileSpec&, U32 mode);
  static  TCI_BOOL  IsDirectory(const FileSpec&);
  static  TCI_BOOL  IsFile(const FileSpec&,FReturn* pError = NULL);
  static  TCI_BOOL  IsOnFixedDrive(const FileSpec&);
  static  TCI_BOOL  IsOnRemoteDrive(const FileSpec&);
  static  TCI_BOOL  IsOnRemovableDrive(const FileSpec&);
  static  TCI_BOOL  IsOnCDRom(const FileSpec&);
  static  TCI_BOOL  IsOnWritableDirectory(const FileSpec&, TCI_BOOL allow_remote);
  static  int       GetDrvType(const FileSpec&);
  static  U32       GetRemoteNameInfoStruct(const FileSpec& fspec,ByteArray& buff);
  static  TCI_BOOL  SearchRemoteContainerForFile(FileSpec&);
  static  int       GetFileData(const FileSpec& fspec, GFData type);
  static  int       Size(const FileSpec&);
  static  time_t    Time(const FileSpec&);
  static  TCI_BOOL  SetTime(const FileSpec&, int);
  static  time_t    CompareDateTime(const FileSpec&, const FileSpec&);
  static  TCI_BOOL  Unique(const FileSpec& fspec, void* data);
  
  static  U32       GetFreeSpaceOnDrive(const FileSpec&);
  static  TCI_BOOL  ConvertToUNCName(FileSpec& fspec);
  static  TCIString GetURIFileString(const FileSpec& filePath, const FileSpec& basePath);
  static  TCI_BOOL  Delete(const FileSpec&);
  static  TCI_BOOL  DeleteAll(const FileSpec&);
  static  TCI_BOOL  Copy(const FileSpec&, const FileSpec&);
  static  TCI_BOOL  Rename(const FileSpec&, const FileSpec&);
  static  TCI_BOOL  Touch(const FileSpec&, TCI_BOOL force = FALSE);
  static  TCI_BOOL  Touch(const FileSpec&, const FileSpec&, TCI_BOOL force = FALSE);  //force to touch read only files
  static  TCI_BOOL  SetMode(const FileSpec&, U32 mode);
  static  TCI_BOOL  HasMode(const FileSpec&, U32 mode);
  static  TCI_BOOL  CreateDir(const FileSpec&);
  static  TCI_BOOL  ExistsSameBase(const FileSpec&);
  static  TCIString GenerateFileKey(int nSeqNumber);
  static  FileSpec  CreateTemp(const FileSpec& path, const TCIString& prefix, const TCIString& ext, TCI_BOOL unique = TRUE);
  static  FileSpec  CreateLongTemp(const FileSpec& path, const TCIString& prefix, 
                                    const TCIString& ext, 
                                      CreateLongTempCallback pConditionsCallback,
                                        void* callbackData);
  static  void      ClearTemp(const FileSpec& path, const TCIString& prefix);
  static  void      FindAllFiles(std::vector<FileSpec>& fileList, const TCIString& baseDir, const TCIString& pattern, 
                                  TCI_BOOL bRecurse, TCI_BOOL bIncludeDirectories);
};

#endif
