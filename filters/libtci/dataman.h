#ifndef DATAMAN_H
#define DATAMAN_H

/*  A cross-platform IniFile Manager.

*/


#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

#ifndef FILESPEC_H
  #include "filespec.h"
#endif

#ifndef TCISTRIN_H
  #include "tcistrin.h"
#endif

#define DM_MAX_KEY_LEN    128

#define DM_NOTLOCKED    0
#define DM_READONLY     1
#define DM_READWRITE    2

#define ID_SECTION      2
#define ID_KEY          3


class ChamFile;
class NameStore;
class ByteArray;

class DataMan {

public:

  DataMan(const FileSpec& inifile_name, TCI_BOOL casematters = FALSE);
  ~DataMan();

  TCI_BOOL  LockFile(U32 mode);
  void      UnlockFile();
  U32       GetLockMode() {return lock_mode;}

  TCI_BOOL    DeleteEntry( int sectionID,int subsectionID,
                             int keyID,int subkeyID);

  TCI_BOOL    DeleteSection(const U8* section);

  TCI_BOOL    GetFileString( int sectionID,int subsectionID,
                               int keyID,int subkeyID,int slot,
                                 U8* dest,U32 limit,U32& len );
  TCI_BOOL    GetFileString( int sectionID,int subsectionID,
                               int keyID,int subkeyID,int slot,
                                 TCIString& str );

  TCI_BOOL    GetFileInt( int sectionID,int subsectionID,
                            int keyID,int subkeyID,int slot,
                              long& val ); 

  TCI_BOOL    PutFileString( int sectionID,int subsectionID,
                               int keyID,int subkeyID,int slot,
                                 const U8* new_val );
  TCI_BOOL    PutFileString( int sectionID,int subsectionID,
                               int keyID,int subkeyID,int slot,
                                 const TCIString& new_val );
  TCI_BOOL    PutFileInt( int sectionID,int subsectionID,
                            int keyID,int subkeyID,int slot,
                              long new_val );

// String versions
  TCI_BOOL    GetFileString( const U8* section, const U8* key, U8* buff, U32 buffmax, U32& len );
  TCI_BOOL	  GetFileString( const TCIString& sectionName, const TCIString& keyName, TCIString& buffer );
  TCI_BOOL    GetFileInt( const U8* section, const U8* key, long& val ); 
  TCI_BOOL    PutFileString( const U8* section, const U8* key, const U8* new_val );
  TCI_BOOL    PutFileInt( const U8* section, const U8* key, long new_val );

  TCI_BOOL    ReplaceSection(U32 pos, const ByteArray& ba, U32 oldlen);     
  TCI_BOOL    ReplaceFilterSection(const char* str);     

  U32         GetFirstSectionPos() { return 0;} 
  TCI_BOOL    GetFileSection( int sectionID,int subsectionID,ByteArray& dest );
  TCI_BOOL    GetNextSection( U32& pos, TCIString& sectionName, ByteArray& ba); 
  TCI_BOOL    GetNextSectionNameAndOffset( U32& pos, TCIString& sectionName, U32& nOffset );
  TCI_BOOL    GetNextEntry( const TCIString& sectionName, U32& line_pos, 
                                   TCIString& keyStr, TCIString& valueStr );
  int         GoToSection(const TCIString& sectionName);
  void        SetKeyString( int whichkey, const TCIString& newkeystr );
  TCI_BOOL    IsCaseSensitive() const {return case_sensitive;}

  // for diagnostic messages
  const TCICHAR* GetCurrKeyString() const { return zKey; }
  const TCICHAR* GetCurrSecString() const { return zSection; }


private:

  TCI_BOOL    SectionToBuffer( int sectionID,int subsectionID );
  TCI_BOOL    KeyToBuffer( int keyID,int subkeyID );
  int         DFLocate(int targslot, TCIString& str, U32& len);
  TCI_BOOL    WriteLine( int slot, const TCIString& buff );

  TCI_BOOL    LocateLine( TCIString&, int curr_pos, int& line_pos, int& empty_pos );
  TCI_BOOL    ReadLine( int slot, TCIString&, int* sec_pos=0, int* line_pos=0 );

  int         LocateSection();
  void        UpdateSectionOffsets(int line_pos, U32 new_ln, U32 old_ln);

  TCI_BOOL    WriteIniLine( const TCIString&, int line_pos, U32 old_ln );
  void        StoreSectionLocs();

  TCI_BOOL    case_sensitive;
  FileSpec    inifile_spec;
  ChamFile*   cf;
  U32         lock_count;
  U32         lock_mode;
  TCIString   eoln;
  int         timestamp;

  NameStore*  section_names;

  TCIString   zSection;
  int         curr_sectionID;
  int         curr_subsectionID;

  TCIString   zKey;
  int         curr_keyID;
  int         curr_subkeyID;
};

#endif

