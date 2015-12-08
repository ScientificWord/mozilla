#ifndef BROWSE_H
#define BROWSE_H


#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

#ifndef FILESPEC_H
  #include "filespec.h"
#endif

#include <afxwin.h>
#include <afxext.h>


// The SFilterMap structure maps the desired file type into the resource
// ID of the filter string for the current platform.
//
// Mask bits are defined by the caller. An array of these is assumed,
// the last entry should be {0,0}.  For example:
//
//   SFilterMap filterMap[] = {
//   #ifdef _MAC
//       {FILTER_TEX, IDS_MAC_FILTER_TEX},
//       {FILTER_MSG, IDS_MAC_FILTER_MSG},
//       {FILTER_TXT, IDS_MAC_FILTER_TXT},
//       {FILTER_ANY, IDS_MAC_FILTER_ANY},
//   #else
//       {FILTER_TEX, IDS_WIN_FILTER_TEX},
//       {FILTER_MSG, IDS_WIN_FILTER_MSG},
//       {FILTER_TXT, IDS_WIN_FILTER_TXT},
//       {FILTER_ANY, IDS_WIN_FILTER_ANY},
//   #endif
//       {0, 0}
//   };
//
// and IDS_WIN_FILTER_TEX should be something like "TeX Files (*.tex)|*.tex||"
// in your string resources.
//

struct SFilterMap {
  U32   mask;
  UINT  resourceID;
};

typedef struct tagFSPEC_NODE {
  tagFSPEC_NODE* next;
  FileSpec* fs;
} FSPEC_NODE;



int DoBrowse(TCI_BOOL bOpenFileDialog,
             SFilterMap* filterMap, U32 defFilter, U32 filter,
             FileSpec& fileName, DWORD dwFlags); 


class CBrowseButton
{
public:
  CBrowseButton();
  ~CBrowseButton();
  void Initialize(const FileSpec& filename,
                   CEdit *filenameEdit,   // Can be NULL
                   TCI_BOOL openFileDialog,
                   DWORD flags,     // Same as Flags field of OPENFILENAME struct (OFN_*)
                   SFilterMap* filtMap, U32 defFilter, U32 filter);
  

public:
  void OnButtonBrowse();
  int Browse();
  const FileSpec& GetFileName();
  FileSpec* GetNextFileName( U32& pos );
  void PutFileName(const FileSpec& filename);
  
  // Returns FALSE if any characters are in the filename.
  TCI_BOOL IsEmpty();

private:
  int   DoBrowse( TCI_BOOL bOpenFileDialog,
                        SFilterMap* filterMap, U32 defFilter, U32 filter,
                                FileSpec& fileName, DWORD dwFlags ); 
  void        DisposeFList();
  
  CEdit*      m_filenameEdit;
  FileSpec    m_filename;
  FSPEC_NODE* f_list;
  SFilterMap* m_filtMap;       
  U32         m_defFilter;    // Default filter (determines def. extension)
  U32         m_filter;       // Other filters
  TCI_BOOL    m_openFileDialog;
  DWORD       m_flags;
};


#endif
