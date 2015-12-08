
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "strdata.h"
#include "chamfile.h"
#include "filespec.h"

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



StringData::StringData( U32 resourceID, void* hInst, int res_type ){

  //This version should be initialized with
  //code like this:
  //  StringData strdata( CHAMSTRINGS );
 
  GetData     = FromResource;
  hResourceData = (HGLOBAL*)NULL;

  HRSRC hResInfo = FindResource((HINSTANCE)hInst,
                                    MAKEINTRESOURCE(resourceID),
                                    MAKEINTRESOURCE(res_type) );

  hResourceData  =  LoadResource((HINSTANCE)hInst, hResInfo );
}

StringData::StringData(const FileSpec& filespec ){

  // This version should be initialized with code like this:
  //      StringData strdata( "strings.wx" )

  GetData = FromFile;

  TCI_ASSERT(filespec.IsValid());
  if (filespec.IsValid())                // we have a non-empty name string
    ResourceFile  =  TCI_NEW( ChamFile(filespec, ChamFile::CHAM_FILE_READ | ChamFile::CHAM_FILE_BINARY) );
}


StringData::~StringData(){

  if (GetData == FromFile) {
    if ( ResourceFile != (ChamFile *)NULL ){
      delete ResourceFile;
    }
  }
 else {
// not needed anymore!?    FreeResource( *hResourceData );
//    delete hResourceData;
    hResourceData  =  (HGLOBAL*)NULL;
  }
}

// Utility to load bytes from the string file
TCI_BOOL StringData::LoadBytes( U32 fileoffset,U32 limit,
                               TCI_BOOL stopatnull,
                               U8* dest) {

  if (GetData == FromFile) {
    if (ResourceFile == (ChamFile*)NULL)
      return FALSE;
    if ( ResourceFile->Goto( fileoffset ) != ChamFile::CFR_OK ) {
      return FALSE;
    }

    U32 res;
    TCI_VERIFY(ResourceFile->ReadBytes(dest, limit, res) == ChamFile::CFR_OK);
    return TRUE;
  }
  else {
    if (hResourceData == (HGLOBAL*)NULL)
      return FALSE;
    U8* text  = (U8*)LockResource( hResourceData );
    TCI_ASSERT(text);

    U32 i = (U32) fileoffset;
    for( U32 j=0; j<limit; j++ ) {
      dest[j] = text[i+j];
        //if( j >= ignorebytes && stopatnull && !destbuffer[j] ) break;
        //Force ignorebytes = 1. This may one day be changed in StringTable,
        //in which case it must also be changed here.
      if( stopatnull && j >= 1 && !dest[j] ) break;
    }

    UnlockResource(hResourceData);
    return TRUE;
  }
}


//********************** End StringData Class ******************

