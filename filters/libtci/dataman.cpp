
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "dataman.h"
#include "filespec.h"
#include "tcistrin.h"
#include "chamfile.h"
#include "namestor.h"
#include "filesys.h"
#include "strutil.h"
#include "bytearry.h"

#include "stdlib.h"
#include "stdio.h"

#define DM_MAX_LINE   512
#define INVALID_POS   -1

// Construct a DataMan, given the name of a data file.
//
//  Data files are ASCII files with the format of a Windows .INI file
//
//
//

DataMan::DataMan( const FileSpec& inifile_name, TCI_BOOL casematters)
  : inifile_spec(inifile_name), cf(NULL), lock_count(0), section_names(NULL),
    curr_sectionID(-1), curr_keyID(-1), curr_subkeyID(-1), timestamp(-1),
    lock_mode(DM_NOTLOCKED), case_sensitive(casematters)
{
}


DataMan::~DataMan() {

  delete  section_names;
}


TCI_BOOL DataMan::LockFile( U32 mode ) {

  if ( lock_count ) {
    TCI_ASSERT(mode==DM_READONLY || lock_mode==DM_READWRITE);    // multiple locks must be readonly
    if (mode!=DM_READONLY && lock_mode!=DM_READWRITE) {
      return FALSE;
    } else {
      lock_count++;
      return TRUE;
    }
  }

  TCI_BOOL rv  =  FALSE;

  U32 fmode =  ChamFile::CHAM_FILE_BINARY;
  fmode |=  mode==DM_READWRITE ? ChamFile::CHAM_FILE_WRITE : ChamFile::CHAM_FILE_READ;

  FileSpec fs(inifile_spec);
  int filesize =  FileSys::Size(fs);
  // This tests for data overflow...beyond 2Gig
  TCI_ASSERT(filesize+4096 > 0);
  cf =  TCI_NEW( ChamFile(fs, fmode, filesize+4096) );

  if ( (mode==DM_READONLY && cf->Readable())
    || (mode==DM_READWRITE && cf->Writeable())  ) {

    int filetime =  cf->FileTime();
    if ((filetime != timestamp) || !section_names)
      StoreSectionLocs();

    lock_count  =  1;
    lock_mode =  mode;
    rv  =  TRUE;
  } else {
    delete cf;
    cf =  NULL;
  }

  return rv;
}


void DataMan::UnlockFile() {

  if (!lock_count)
    TCI_ASSERT(FALSE);
  else
    lock_count--;

  if ( !lock_count && cf ) {
    timestamp =  cf->FileTime();
    delete cf;
    cf =  NULL;
    lock_mode =  DM_NOTLOCKED;
  }
}


TCI_BOOL DataMan::GetFileString( int sectionID,int subsectionID,
                                    int keyID,int subkeyID,int slot,
                                        U8* dest,U32 limit,U32& len ) {

  //TCI_ASSERT( FALSE );    // just to be obnoxious
  TCIString res;
  len =  0;
  TCI_BOOL rv =  GetFileString(sectionID, subsectionID, keyID, subkeyID, slot, res);
  if ( rv ) {
    U32 n =  CHAMmin(U32(limit-1), U32(res.GetLength()));
    strncpy((char*)dest, res, n);
    dest[n] =  0;
    len =  n;
  }

  return rv;
}


TCI_BOOL DataMan::GetFileString( int sectionID,int subsectionID,
                                    int keyID,int subkeyID,int slot,
                                        TCIString& str ) {

  TCI_BOOL local_lock =  FALSE;
  if ( !lock_count ) {
    local_lock =  LockFile( DM_READONLY );
    if ( !local_lock )
      return FALSE;
  }

  TCI_BOOL rv  =  FALSE;
  if ( SectionToBuffer(sectionID,subsectionID) && KeyToBuffer(keyID,subkeyID) ) {
    rv =  ReadLine( slot, str );
    str.TrimRight();
  }

  if ( local_lock )
    UnlockFile();

  return rv;    
}


TCI_BOOL DataMan::GetFileString( const TCIString& sectionName, const TCIString& keyName, TCIString& buffer )
{
  TCI_BOOL local_lock =  FALSE;
  if ( !lock_count ) 
  {
    local_lock =  LockFile( DM_READONLY );
    if ( !local_lock )
      return FALSE;
  }

  SetKeyString( ID_SECTION, sectionName );
  SetKeyString( ID_KEY, keyName );

  TCI_BOOL rv =  ReadLine( -1, buffer );
  if (rv)
    buffer.TrimRight();
  else
    buffer.Empty();

  if ( local_lock )
    UnlockFile();

  return rv;    
}

TCI_BOOL DataMan::GetFileString( const U8* section, const U8* key, U8* dest, U32 limit, U32& len ) {

  TCI_BOOL local_lock =  FALSE;
  if ( !lock_count ) {
    local_lock =  LockFile( DM_READONLY );
    if ( !local_lock )
      return FALSE;
  }

  SetKeyString( ID_SECTION, section );
  SetKeyString( ID_KEY, key );

  TCIString res;
  len =  0;
  TCI_BOOL rv =  ReadLine( -1, res );
  res.TrimRight();
  if ( rv ) {
    U32 n =  CHAMmin(U32(limit-1), U32(res.GetLength()));
    strncpy((char*)dest, res, n);
    dest[n] =  0;
    len =  n;
  }

  if ( local_lock )
    UnlockFile();

  return rv;    
}


TCI_BOOL DataMan::GetFileInt( int sectionID,int subsectionID,
                                  int keyID,int subkeyID,int slot,
                                      long& val ) {

  TCI_BOOL rv  =  FALSE;

  TCIString res;
  if ( GetFileString(sectionID,subsectionID, keyID,subkeyID,slot,res) ) {
    val =  StrUtil::StringInt(res);
    rv  =  TRUE;
  }

  return rv;
}


TCI_BOOL DataMan::GetFileInt( const U8* section, const U8* key, long& val ) {

  TCI_BOOL rv  =  FALSE;

  U8  dest[64];
  U32 len;
  if ( GetFileString( section, key, dest, 64, len) ) {
    val =  atol( (char *)dest );
    rv  =  TRUE;
  }

  return rv;
}


TCI_BOOL DataMan::PutFileString( int sectionID,int subsectionID,
                                  int keyID,int subkeyID,int slot,const U8* src ) {

  TCIString data(src);
  return PutFileString(sectionID, subsectionID, keyID, subkeyID, slot, data);
}


TCI_BOOL DataMan::PutFileString( int sectionID,int subsectionID,
                                  int keyID,int subkeyID,int slot, const TCIString& src ) {

  // check for write permission
  TCI_ASSERT( !cf || (cf->GetOpenMode()&ChamFile::CHAM_FILE_WRITE) );
  if ( lock_count && !(cf->GetOpenMode()&ChamFile::CHAM_FILE_WRITE) )
    return FALSE;

  TCI_BOOL local_lock =  FALSE;
  if ( !lock_count ) {
    local_lock =  LockFile( DM_READWRITE );
    if ( !local_lock )
      return FALSE;
  }

  TCI_BOOL rv  =  FALSE;
  if ( SectionToBuffer(sectionID,subsectionID) && KeyToBuffer(keyID,subkeyID) )
    rv =  WriteLine( slot, src );

  if ( local_lock )
    UnlockFile();

  return rv;
}

TCI_BOOL DataMan::PutFileInt( int sectionID,int subsectionID,
                                  int keyID,int subkeyID,int slot, long src ) {

  TCIString data = StrUtil::IntString(src);
  return PutFileString(sectionID, subsectionID, keyID, subkeyID, slot, data);
}

TCI_BOOL DataMan::PutFileString( const U8* section, const U8* key, const U8* src ) {

  // check for write permission
  TCI_ASSERT( !cf || (cf->GetOpenMode()&ChamFile::CHAM_FILE_WRITE) );
  if ( lock_count && !(cf->GetOpenMode()&ChamFile::CHAM_FILE_WRITE) )
    return FALSE;

  TCI_BOOL local_lock =  FALSE;
  if ( !lock_count ) {
    local_lock =  LockFile( DM_READWRITE );
    if ( !local_lock )
      return FALSE;
  }

  TCI_BOOL rv;
  if ( key == NULL ) {
    TCI_ASSERT(src == NULL);
    rv = DeleteSection(section);
  } else {
    SetKeyString( ID_SECTION, section );
    SetKeyString( ID_KEY, key );
    TCIString data(src);
    rv  =  WriteLine( -1, data );
  }
  if ( local_lock )
    UnlockFile();

  return rv;
}

TCI_BOOL DataMan::PutFileInt( const U8* section, const U8* key, long src ) {

  char numberString[30];
  sprintf(numberString, "%d", src);
  return PutFileString( section, key, (U8 *)numberString );
}
    

TCI_BOOL DataMan::GetFileSection( int sectionID,int subsectionID,
                                                    ByteArray& dest ) {

  TCI_BOOL rv  =  FALSE;

  TCI_BOOL local_lock  =  FALSE;
  if ( !lock_count ) {
// 94.0811 by ron - don't want to set local_lock unless we're successful in locking file!
    if (LockFile( DM_READONLY ))
      local_lock  =  TRUE;
// 94.0811 by ron - if we can't lock file, should delete chamfile and get out!
    else
      return FALSE;
  }

  if ( SectionToBuffer(sectionID,subsectionID) ) {
    int section_off =  LocateSection();
    if ( section_off == INVALID_POS ) {
      if ( local_lock )
        UnlockFile();
      return FALSE;
    }

    rv =  TRUE;
    TCIString line;
    line.GetBuffer( 256 );
    U32 nfile_lines =  0;
    U32 nout_lines  =  0;
    TCI_VERIFY( cf->Goto( section_off ) == ChamFile::CFR_OK );

    while (cf->ReadString(line)==ChamFile::CFR_OK) {

      line.TrimLeft();  // trim leading spaces
      if ( line.IsEmpty() || line[0]==';' ) {   // empty or comment line

      } else if ( line[0] == '[' ) {

        if ( nfile_lines ) break;                 // end of section

      } else {

        line.TrimRight();
        dest.AddString((const TCICHAR*)line);
        nout_lines++;
      }

      nfile_lines++;
    }   // loop thru section lines

    dest +=  (U8)0;
    if (!nout_lines)
      dest +=  (U8)0;

  }   // if ( section_located )

  if ( local_lock ) UnlockFile();

  return rv;
}


void DataMan::SetKeyString( int whichkey, const TCIString& newkeystr ) {

  if ( whichkey==ID_SECTION ) {
    zSection =  newkeystr;
    curr_sectionID =  -2;
    curr_subsectionID =  -1;

  } else if ( whichkey==ID_KEY ) {
    zKey =  newkeystr;
    curr_keyID =  -2;
    curr_subkeyID =  -1;

  } else {
    TCI_ASSERT( FALSE );
  }
}



TCI_BOOL DataMan::ReplaceSection(U32 pos, const ByteArray& ba, U32 oldlen)
{
   const unsigned char* src = ba.ReadOnlyLock();
   TCIString str;

   for (U32 i = 0; i < ba.GetByteCount(); ++i){
	   str += *(src + i);
   }
   ba.ReadOnlyUnlock();
   return  WriteIniLine(str, pos, oldlen ) ;
}


TCI_BOOL DataMan::ReplaceFilterSection(const char* newsection)
{
   if (!LockFile(DM_READWRITE))
     return FALSE;
   
   U32 pos = GetFirstSectionPos(); // pos is a NAME_REC*
   ByteArray ba(2046);
   TCIString sectionName;
   while (GetNextSection(pos, sectionName, ba)) {
       if (sectionName == "FILTER" ){
		   int off0 = LocateSection();
		   GetNextSection(pos, sectionName, ba);		   
		   int off1 = LocateSection();
           WriteIniLine(newsection, off0, off1-off0 );
		   break;
	   }
   }
          //long oldlen = ba.GetByteCount();
   //       ba = cst -> filtersection.c_str();
      
   //		  // Start replace after the [Filter] + eoln  
   //		  startpos = startpos + sectionName.GetLength() + 4; 
   //       cst -> dm -> ReplaceSection(startpos, ba, oldlen);
   //       wroteFilterSec = true;
   //		  break;
   //    }
   //}
   //if (!wroteFilterSec){
      // The cst didn't have a filter section
   //   long oldlen = 0;
   //   ba = "[FILTER]\r\n";
   //   ba += cst -> filtersection.c_str();

	  // Put filter section at top of file
   //   cst -> dm -> ReplaceSection(0, ba, oldlen);

   //}
   UnlockFile();

   return TRUE;
}




// START PRIVATE FUNCTIONS*********************


// Translate from section IDs to string, results are buffered in private variables

TCI_BOOL DataMan::SectionToBuffer( int sectionID,int subsectionID ) {

  TCI_BOOL rv =  TRUE;
  if ( sectionID != curr_sectionID || subsectionID != curr_subsectionID ) {
    zSection.Empty();
    TCI_VERIFY( StrUtil::LoadStringA(zSection, sectionID) );
    if ( !zSection.IsEmpty() ) {
      rv =  TRUE;
      if ( subsectionID != -1 ) {
        TCIString zSubSection = StrUtil::IntString(subsectionID);
        zSection +=  zSubSection;
      }
      curr_sectionID    =  sectionID;
      curr_subsectionID =  subsectionID;
    }
  }

  return rv;
}


// Translate from key IDs to string, results are buffered in private variables

TCI_BOOL DataMan::KeyToBuffer( int keyID,int subkeyID ) {

  TCI_BOOL rv =  TRUE;
  if ( keyID != curr_keyID || subkeyID != curr_subkeyID ) {
    zKey.Empty();
    rv = StrUtil::LoadStringA(zKey, keyID);
    TCI_ASSERT( rv );
    if ( rv ) {
      if ( subkeyID != -1 ) {
        TCIString zSubKey = StrUtil::IntString(subkeyID);
        zKey +=  zSubKey;
      }

      curr_keyID    =  keyID;
      curr_subkeyID =  subkeyID;
    }
  }

  return rv;
}


int DataMan::DFLocate(int targslot, TCIString& str, U32& len) {

  len =  0;
  int off =  0;

  int tally = 0;
  int i =  0;
  while (tally<targslot) {
    if ( (i=str.Find(',',off)) == -1) {  // no more commas
      // make sure we add the commas before the 'line end'
      str.TrimRight();
      str +=  ',';
      i =  str.GetLength()-1;
      str +=  eoln;
    }
    tally++;    // we found another comma
    if (tally==targslot) {
      len =  i - off;
      break;
    }
    off =  i+1;
  }

  return off;
}


int DataMan::LocateSection() {

  U32 theinfo;
  return section_names->GetName(zSection,zSection.GetLength(),0,theinfo) ? (int)theinfo : INVALID_POS;
}

int DataMan::GoToSection(const TCIString& sectionName)
{
  SetKeyString(ID_SECTION, sectionName);
  return LocateSection();
}

TCI_BOOL DataMan::LocateLine( TCIString& line, int curr_pos, int& line_pos, int& empty_pos ) {

  TCI_ASSERT(cf);
  if ( !cf )
    return FALSE;

  empty_pos =  -1;
  TCI_BOOL found =  FALSE;

  int line_num  =  0;
  int last_empty_line =  0;
  int key_len =  zKey.GetLength();
  int eoln_len =  eoln.GetLength();

  while (cf->ReadString(line)==ChamFile::CFR_OK) {

    TCI_ASSERT( line_num || line[0]=='[' );

    int len =  line.GetLength();
    if ( len <= eoln_len ) {
      if ( line_num > last_empty_line+1 )
        empty_pos =  curr_pos;
      last_empty_line =  line_num;

    } else if ( !line.CompareNoCase(zKey, key_len) ) {
      TCICHAR ch =  line[key_len];
      if ( ch=='=' || ch<=' ' ) {
        found =  TRUE;
        line_pos =  curr_pos;
        break;
      }
    } else if ( line_num && line[0] == '[' ) {     // end of section - key not found
      if ( empty_pos == -1 )
        empty_pos =  curr_pos;
      break;
    }
    line_num++;
    curr_pos +=  len;
  }

  return found;
}


TCI_BOOL DataMan::WriteIniLine( const TCIString& line, int line_pos, U32 old_ln ) {

// 94.0811 by ron - if we don't have "cf", get out!
  TCI_ASSERT(cf);
  if (!cf)
    return FALSE;

//
//  Read the tail of the file into a buffer
//  Write the line, then write the buffer
//  Finally, fix up the offsets in section_names
//

  TCI_BOOL rv  =  FALSE;

  U32 fsize =  cf->FileSize();
  U32 tail_pos  =  line_pos + old_ln;
  U32 tailbytes =  0;

  U32 new_ln =  line.GetLength();
  U32 rbytes =  0;
  TCIString tail_buff;
  if ( new_ln != old_ln ) {
    tailbytes =  fsize - tail_pos;
    if ( cf->Goto(tail_pos)==ChamFile::CFR_OK && tailbytes>0 ) {
      TCI_VERIFY( cf->ReadBytes( tail_buff, tailbytes, rbytes )==ChamFile::CFR_OK );
      TCI_ASSERT( rbytes==tailbytes );
    }
  }

  if ( cf->Goto(line_pos)==ChamFile::CFR_OK ) {
    if ( new_ln ) {
      TCI_VERIFY(cf->WriteBytes(line, new_ln, rbytes) == ChamFile::CFR_OK );
      TCI_ASSERT( rbytes==new_ln );
    }
    if ( tailbytes>0 ) {
      TCI_VERIFY( cf->WriteBytes(tail_buff, tailbytes, rbytes) == ChamFile::CFR_OK );
      TCI_ASSERT( rbytes==tailbytes );
    }

    if ( new_ln < old_ln )
      cf->ChangeSize( line_pos+new_ln+tailbytes );

    rv  =  TRUE;    //cf->Flush();
  }

  UpdateSectionOffsets(line_pos, new_ln, old_ln);

  return rv;
}

void DataMan::UpdateSectionOffsets(int line_pos, U32 new_ln, U32 old_ln)
{
  if ( new_ln != old_ln ) {
    int delta =  new_ln - old_ln;
    U32 curr  =  0L;
    U8  dname[DM_MAX_KEY_LEN];
    U32 dlen;
    U32 dtype;
    U32 dinfo;

    while ( (curr=section_names->GetNext(curr,dname,dlen,dtype,dinfo))  !=  0 ) {
      if (dinfo > (U32)line_pos) {
        int tmp =  (int)dinfo + delta;
        section_names->PutDataWithName( dname,dlen,0,(U32)tmp );
      }
    }
  }
}

// Locate and store the file positions of the inifile sections

void DataMan::StoreSectionLocs() {

// 94.0811 by ron - if we don't have "cf", get out!
  if (!cf)
    return;

  TCI_BOOL new_store =  FALSE;
// 94.0831 by Andy
  // always regenerate the namestore when reloading the file
  // this will prevent sciword from thinking sections still
  // exist in the ini file when they have been removed by an
  // external process
  delete section_names;
  // NOTE: hash table sizes are much more efficient if their 
  //       table size is a prime number; thus we use 31.
  section_names =  TCI_NEW(NameStore(31, FALSE));
  NameStore::NS_MODE	mode = case_sensitive ? NameStore::NSM_CASE_SENSITIVE : NameStore::NSM_CASE_INSENSITIVE;
  section_names->SetCaseSensitivity(mode);
  new_store =  TRUE;

  TCIString line;
  line.GetBuffer( 256 );
  int curr_pos =  cf->GetCurPos();

  while (cf->ReadString(line)==ChamFile::CFR_OK) {
    if ( line[0] == '[' ) {
      int ep =  line.Find(']');
      TCI_ASSERT(ep>0);
      if ( ep>0 ) {
        
        if ( new_store )
          section_names->PushName( ((const U8 *)line)+1, ep-1, 0, curr_pos, FALSE );
        else
          section_names->PutDataWithName( ((const U8 *)line)+1, ep-1, 0, curr_pos );

        if ( eoln.IsEmpty() ) {       // save the line end sequence
          ep++;
          while ( line[ep] >= ' ' ) ep++;
          eoln =  line.Right(line.GetLength()-ep);
        }
      }
    }
    curr_pos  =  cf->GetCurPos();
  }
}


//TCI_BOOL DataMan::ReadLine( int slot, U8* dest, U32 limit, U32& len, int* sect_pos, int* line_pos ) {
TCI_BOOL DataMan::ReadLine( int slot, TCIString& str, int* sect_pos, int* line_pos ) {

  TCI_BOOL rv =  FALSE;

  if ( line_pos )  *line_pos =  INVALID_POS;

  int section_off =  LocateSection();
  if ( sect_pos )  *sect_pos =  section_off;
  if ( section_off == INVALID_POS )
    return FALSE;

  TCIString line;
  int line_off;
  int empty_off;
  TCI_VERIFY( cf->Goto( section_off ) == ChamFile::CFR_OK );
  if ( LocateLine(line,section_off,line_off,empty_off) ) {
    int eqp =  line.Find('=');
    TCI_ASSERT(eqp>0);
    if ( eqp<=0 )
      return FALSE;

    eqp++;
    line =  line.Right(line.GetLength() - eqp);
    int off =  0;
    U32 len =  line.GetLength();
    if ( slot!=-1 ) {
      off =  DFLocate( slot, line, len );     // this sets len
      TCI_ASSERT( off>=0 );
    }
    if ( off>=0 ) {
      str =  line.Mid(off, len);
      rv  =  TRUE;
    }
    if ( line_pos )  *line_pos =  line_off;
  } else if ( line_pos )
    *line_pos =  empty_off;

  return rv;
}


TCI_BOOL DataMan::WriteLine( int slot, const TCIString& src ) {

  int section_off, line_off=0;
  U32 oldlen=0;

  TCIString oldline;
  TCIString mysrc = src;

  TCI_BOOL hasZSection = ReadLine( -1, oldline, &section_off, &line_off );
  if (hasZSection) {
    oldlen =  oldline.GetLength();
    if ( slot>=0 ) {
      U32 len = 0;
      int off =  DFLocate(slot, oldline, len);
      TCI_ASSERT( off>=0 );

      // remove 'line end' sequence
      oldline.TrimRight();

      TCIString tail =  oldline.Mid(off+len);
      oldline.SetDataLength(off);
      oldline +=  mysrc + tail;
      mysrc =  oldline;
    }
  } else {    // readline failed
    if ( mysrc.IsEmpty() )
      return TRUE;

    TCI_ASSERT( !oldlen );

    if ( line_off == INVALID_POS )
      line_off =  cf->FileSize();

    if ( section_off == INVALID_POS ) {   // unable to locate section

      TCI_ASSERT(line_off == (int)cf->FileSize());
      TCI_VERIFY( cf->Goto(line_off) == ChamFile::CFR_OK );

      oldline  =  eoln +  '[' + zSection + ']' + eoln;

      U32 rbytes;
      TCI_VERIFY(cf->WriteBytes(oldline, rbytes) == ChamFile::CFR_OK );
      // put section name in namestore
      int data =  line_off + eoln.GetLength();
      section_names->PushName( zSection, zSection.GetLength(), 0, data, FALSE );
      // advance file pointer
      line_off +=  oldline.GetLength();
    }
  }

  if ( !mysrc.IsEmpty() ) {
    if (!hasZSection && slot > 1) {
      TCIString commas(',', slot-1);
      oldline =  zKey + '=' + commas + mysrc + eoln;
    } else
      oldline =  zKey + '=' + mysrc + eoln;
  } else {
    oldline.Empty();
  }

  // write the new data
  TCI_ASSERT(line_off <= (int)cf->FileSize());
  TCI_BOOL rv =  FALSE;
  if ( line_off >= (int)cf->FileSize() ) {
    TCI_VERIFY( cf->Goto(cf->FileSize()) == ChamFile::CFR_OK );
    if ( !oldline.IsEmpty() ) {
      U32 rbytes;
      TCI_VERIFY(cf->WriteBytes(oldline, rbytes) == ChamFile::CFR_OK );
      TCI_ASSERT(rbytes == (U32)oldline.GetLength());
    }
    rv =  TRUE;
  } else {
    // correct oldlen -  needs keylen+1 added to it
    if ( oldlen )
      oldlen +=  zKey.GetLength()+1;
    rv =  WriteIniLine( oldline, line_off, oldlen );
  }

  return rv;
}


#if TCIENV(CHAMNOWIN)
void DataMan::Dump() {

}
#endif

//
// Terminology: 
//  .ini file 'Sections' have 'Entries' that consists of 
//  a 'Key' followed by an equal sign followed by a 'String'.
//
// Example:
//   [Section]        ; /* A section    */
//   Key1=String1     ; /* First entry  */
//   Key2=String2     ; /* Second entry */
//
// The following function will delete the specified entry.
// That is, it will delete a line form the .ini file.
//
TCI_BOOL DataMan::DeleteEntry(int sectionID,int subsectionID,
                              int keyID,int subkeyID)
{
  TCI_ASSERT(!cf || (cf->GetOpenMode() & ChamFile::CHAM_FILE_WRITE));
  if (lock_count && !(cf->GetOpenMode() & ChamFile::CHAM_FILE_WRITE))
    return FALSE;

  TCI_BOOL local_lock = FALSE;
  if (!lock_count) 
  {
    local_lock = LockFile(DM_READWRITE);
    if (!local_lock)
      return FALSE;
  }

  TCI_BOOL rv = TRUE;
  if (SectionToBuffer(sectionID, subsectionID) && KeyToBuffer(keyID, subkeyID))
  {
    int section_off = LocateSection();

    if (section_off != INVALID_POS)
    {
      TCIString line;
      int line_off;
      int empty_off;

      TCI_VERIFY(cf->Goto(section_off) == ChamFile::CFR_OK);
 
      if (LocateLine(line, section_off, line_off, empty_off)) 
        rv = WriteIniLine(EmptyTCIString, line_off, line.GetLength());  
    }
  }

  if (local_lock)
    UnlockFile();

  return rv;
}


//
// Terminology: 
//  .ini file 'Sections' have 'Entries' that consists of 
//  a 'Key' followed by an equal sign followed by a 'String'.
//
// Example:
//   [Section]        ; /* A section    */
//   Key1=String1     ; /* First entry  */
//   Key2=String2     ; /* Second entry */
//
// The following function will delete the specified section.
// That is, it will delete all lines until the next section 
// in the ini file.
//
TCI_BOOL DataMan::DeleteSection(const U8* section)
{
  if ( section == NULL )
    return FALSE;

  // check for write permission
  TCI_ASSERT(!cf || (cf->GetOpenMode() & ChamFile::CHAM_FILE_WRITE));
  if (lock_count && !(cf->GetOpenMode() & ChamFile::CHAM_FILE_WRITE))
    return FALSE;

  TCI_BOOL local_lock = FALSE;
  if (!lock_count) 
  {
    local_lock = LockFile(DM_READWRITE);
    if (!local_lock)
      return FALSE;
  }

  TCI_BOOL rv =  FALSE;
  SetKeyString( ID_SECTION, section );
  int section_off = LocateSection();

  if (section_off != INVALID_POS)
  {
    rv = TRUE;
    TCIString line;

	// jump to section if file
    TCI_VERIFY(cf->Goto(section_off) == ChamFile::CFR_OK);
    
    // read section 
    cf->ReadString(line); //skips this section head

    while (cf->ReadString(line)==ChamFile::CFR_OK && line[0] != '[');

    if ( cf->GetCurPos() == (int)cf->FileSize() ) {  // End of file
      TCI_VERIFY( cf->ChangeSize(cf->FileSize() - (cf->FileSize() - section_off)) );
      section_names->DeleteName(section, strlen((char*)section), 0);
    } else {
      // jump back to the beginning of the section
      int nextsec_off = cf->GetCurPos() - line.GetLength();
      TCI_VERIFY(cf->Goto(nextsec_off) == ChamFile::CFR_OK);

      // Reads the rest of the file.
      U32 rbytes =  0;
      I32 tailbytes = cf->FileSize() - nextsec_off;
      TCIString tail_buff;
      TCI_VERIFY( cf->ReadBytes( tail_buff, tailbytes, rbytes )==ChamFile::CFR_OK );
      TCI_ASSERT(rbytes == (U32)tailbytes);

      //reset the file position
      TCI_VERIFY(cf->Goto(section_off) == ChamFile::CFR_OK);
      if ( tailbytes ) {
        TCI_VERIFY( cf->WriteBytes(tail_buff, rbytes) == ChamFile::CFR_OK );
        TCI_ASSERT(rbytes == (U32)tailbytes);
        TCI_VERIFY( cf->ChangeSize(cf->FileSize() - (nextsec_off - section_off)) );
      }
      section_names->DeleteName(section, strlen((char*)section), 0);
      UpdateSectionOffsets(section_off, 0, nextsec_off - section_off);
    }
  }
  if (local_lock)
    UnlockFile();

  return rv;
}


///////////////////////////////////////////////////////////////////////////
//  GetNextSection will return the postion, section name, and whole section
// to pos, sectionName, and ba
TCI_BOOL DataMan::GetNextSection( U32& pos, TCIString& sectionName, ByteArray& ba)
{
  U32 dlen, dtype;
  U32 dinfo;
  U8  sectName[DM_MAX_KEY_LEN];

  // section_names == 0 occurs e.g. if the inifile was empty. I don't know if 
  // there's another way to check this condition.
  if (section_names == 0){
    return FALSE;
  }
  if ((pos = section_names->GetNext(pos, sectName, dlen, dtype, dinfo)) != 0) {
    sectName[dlen] = '\0';
    sectionName = sectName;
    SetKeyString(ID_SECTION, sectionName);
    GetFileSection(-2, -1, ba);
    return TRUE;
  } else {
    return FALSE;
  }
}

//This version will return only the section name and updated position. It can be used
//in conjunction with GetNextEntry() to iterate through a data file.
TCI_BOOL DataMan::GetNextSectionNameAndOffset( U32& pos, TCIString& sectionName, U32& nOffset )
{
  if (!section_names)
    return FALSE;

  TCI_BOOL bFoundOne = FALSE;
  U32 dlen, dtype;
  U32 dinfo;
  U8  sectName[DM_MAX_KEY_LEN];

  if ((pos = section_names->GetNext(pos, sectName, dlen, dtype, dinfo)) != 0)
  {
    sectName[dlen] = '\0';
    sectionName = sectName;
    SetKeyString( ID_SECTION, sectionName );
    nOffset = dinfo;
    bFoundOne = TRUE;
  }
  else
  {
    sectionName.Empty();
    nOffset = 0;
  }
  return bFoundOne;
}

TCI_BOOL DataMan::GetNextEntry( const TCIString& sectionName, U32& line_pos ,TCIString& keyStr, 
                                       TCIString& valueStr )
{
  if (!LockFile(DM_READONLY))
  {
    TCI_ASSERT(FALSE);
    return FALSE;
  }

  int curr_pos = line_pos;
  if (cf->Goto(curr_pos)!= ChamFile::CFR_OK)
  {
    TCI_ASSERT(FALSE);
    return FALSE;
  }

  TCI_BOOL foundOne = FALSE;
  int len  =  0;
  int eoln_len =  eoln.GetLength();
  TCIString sepChars("= "); //anything else?
  TCIString theLine;

  while (cf->ReadString(theLine)==ChamFile::CFR_OK) {
    len = theLine.GetLength();
    curr_pos += len;
    if ( theLine.GetLength() <= eoln_len || theLine.GetAt(0)==';') {
      continue;
    }
    else if ( theLine[0] == '[' )     // end of section? - no more keys if so
    {
      TCI_BOOL bEndOfSection = TRUE;
      int nCloseBracket = theLine.ReverseFind(']');
      if (nCloseBracket > 0)
      {
        theLine = theLine.Mid(1, nCloseBracket - 1);
        theLine.Trim();
        bEndOfSection = (theLine != sectionName);
      }
      if (bEndOfSection)
        break;
      else
        continue;
    }

    theLine.TrimLeft();
    if (!theLine.IsEmpty())
    {
      int nSep = theLine.FindOneOf(sepChars);
      if (nSep > 0)
      {
        keyStr = theLine.Left(nSep);
        valueStr = theLine.Mid(nSep+1);
        keyStr.Trim();
        valueStr.Trim();
        foundOne =  TRUE;
        line_pos =  curr_pos;
        break;
      }
    } 
  }

  UnlockFile();
  return foundOne;
}

