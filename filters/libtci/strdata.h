#ifndef STRDATA_H
#define STRDATA_H

//NOTE: Only the header file is maintained in this (General) directory.
//      Each EDGE must provide its own implementation.

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif


//Resource type for string data resources.
#define STRFILE 256

class ByteArray;
class ChamFile;
class FileSpec;

class StringData {

public:
  StringData( U32 resourceID, void* hInst = 0, int res_type = STRFILE );
  StringData(const FileSpec& filespec );
  ~StringData();

  TCI_BOOL DataAvailable(void) {if ((GetData==FromResource && hResourceData!=NULL)||
                                    (GetData==FromFile     && ResourceFile!=NULL))
                                  return TRUE;
                                else
                                  return FALSE;}


  TCI_BOOL LoadBytes(U32 offset, U32 limit, TCI_BOOL stopatnull,
                        U8* destba);

private:
  enum DataSource {FromResource,FromFile};

  enum DataSource    GetData;
  union {
    void*     hResourceData;
    ChamFile* ResourceFile;
  };
};

#endif

