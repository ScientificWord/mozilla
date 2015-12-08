
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "filespec.h"
#include "browse.h"
#include "tcistrin.h"
#include "strutil.h"

#define BIGBUFFER_SIZE 16384


CBrowseButton::CBrowseButton()
{
  m_filenameEdit = NULL;
  m_filter = m_defFilter = 0;
  m_openFileDialog = TRUE;
  m_flags = 0;
  f_list    =  NULL;    // a list of selected files
}


CBrowseButton::~CBrowseButton()
{
  DisposeFList();
}


void CBrowseButton::DisposeFList()
{
  FSPEC_NODE* del;                  // CBrowseButton now owns a list of
  FSPEC_NODE* rover =  f_list;      // selected files - to support
  while ( rover ) {                 // OFN_ALLOWMULTISELECT in the flags
    del   =  rover;                 // set in Initialize.
    rover =  rover->next;           // Here we dispose this list.
    if ( del->fs )                  //
      delete del->fs;               //
    delete del;                     //
  }                                 //
  f_list  =  NULL;                  //
}


void CBrowseButton::Initialize(const FileSpec& filename,
                             CEdit *filenameEdit,  // Can be NULL
                             TCI_BOOL openFileDialog,
                             DWORD flags,
                             SFilterMap* filtMap, U32 defFilter, U32 filter)
{
  DisposeFList();

  m_filename       =  filename;
  m_filenameEdit   =  filenameEdit;
  m_openFileDialog =  openFileDialog;
  m_flags          =  flags;
  m_filtMap        =  filtMap;
  m_filter         =  filter;
  m_defFilter      =  defFilter;

  PutFileName(m_filename);
}


const FileSpec& CBrowseButton::GetFileName()
{
  if (m_filenameEdit) {
    CString file;
    m_filenameEdit->GetWindowText(file);
    m_filename = (const TCICHAR *)file;
  }
  return m_filename;
}
  
// Function to retrieve selected files when OFN_ALLOWMULTISELECT is set
// Caller must set "pos = 0" to start calling loop.

FileSpec* CBrowseButton::GetNextFileName( U32& pos )
{
  U32 tally =  0;
  FSPEC_NODE* rover =  f_list;
  while ( tally < pos && rover ) {
    tally++;
    rover =  rover->next;
  }

  if ( rover && tally==pos ) {
    pos++;
    return rover->fs;   // we are returning a pointer to a FileSpec here.
  } else
    return NULL;
}


void CBrowseButton::PutFileName(const FileSpec& filename)
{
  m_filename = filename;
  if (m_filenameEdit)
    m_filenameEdit->SetWindowText(m_filename);
}


void CBrowseButton::OnButtonBrowse()
{
  FileSpec filename(GetFileName());
  if (IDOK == DoBrowse(m_openFileDialog, m_filtMap, m_defFilter, m_filter, filename, m_flags))
    PutFileName(filename);
}

int CBrowseButton::Browse()
{
  FileSpec filename(GetFileName());
  int rv = DoBrowse(m_openFileDialog, m_filtMap, m_defFilter, m_filter, filename, m_flags);
  if (IDOK == rv)
    PutFileName(filename);
  return rv;
}



int CBrowseButton::DoBrowse(TCI_BOOL bOpenFileDialog,
            SFilterMap* filterMap, U32 defFilter, U32 filter,
            FileSpec& fileName, DWORD dwFlags) 
{
  // Determine default file type
  TCIString defExt, fileType;
  int i, m;
  for (i = 0; (m = filterMap[i].mask) != 0; ++i) {
    if (m & defFilter)
      StrUtil::LoadString(fileType,filterMap[i].resourceID);
  }

  // Determine default extension from the filter string
  int pos1 = fileType.Find("|*.", 0);
  if (pos1 != -1) {
    pos1 += 3;   // now points to start of extension
    int pos2 = fileType.Find('|', pos1);
    if (pos2 != -1) {
      defExt = fileType.Mid(pos1, pos2-pos1);
      if (defExt[0] == '*')
        defExt = "";
    }
  }

  // Add remaining filters
  TCIString str;
  filter &= ~defFilter;
  for (i = 0; (m = filterMap[i].mask) != 0; ++i)
    if (m & filter) {
      StrUtil::LoadString(str, filterMap[i].resourceID);
      fileType += str;
    }

  // terminate list of file types
  fileType += "|";  

  // Call common dialog
  CFileDialog dlg(bOpenFileDialog, defExt, fileName, dwFlags, fileType);
  if (!(dwFlags & OFN_SHOWHELP))
    dlg.m_ofn.Flags &= ~OFN_SHOWHELP;
  char bigbuffer[BIGBUFFER_SIZE];
  strcpy( bigbuffer,(char*)dlg.m_ofn.lpstrFile );
  dlg.m_ofn.lpstrFile =  bigbuffer;
  dlg.m_ofn.nMaxFile  =  BIGBUFFER_SIZE;

  int rv = dlg.DoModal();

  if (IDOK == rv) {
    if ( m_flags & OFN_ALLOWMULTISELECT ) {

      POSITION pos  =  dlg.GetStartPosition();
	  int tally =  0;
      while ( pos ) {
        TCIString f_nom  =  dlg.GetNextPathName( pos );
/*
char zzz[80];
int sln =  f_nom.GetLength();
sprintf( zzz,"pos=%d, sln=%d\n",tally,sln );
JBMLine( zzz );
tally++;
*/
        if ( !f_nom.IsEmpty() ) {
          FSPEC_NODE* node  =  TCI_NEW(FSPEC_NODE);
          node->fs    =  TCI_NEW( FileSpec(f_nom) );
          node->next  =  f_list;
          f_list  =  node;
        } else
          break;
      }                 // loop thru selections

      if ( f_list )
        fileName =  *(f_list->fs);

    } else {

      // Convert full path to human readable form
      FileSpec fs((const TCICHAR *)dlg.GetPathName());
      fileName = fs;

      // Add extension if not already one.  I think we should use nFilterIndex
      // to find out which filter extension was being used.     -ZZZCLG

      //nFilterIndex:
      //Specifies an index into the buffer pointed to by lpstrFilter. The system uses the index value 
      //to obtain a pair of strings to use as the initial filter description and filter pattern for the dialog 
      //box. The first pair of strings has an index value of 1. When the user closes the dialog box, the 
      //system copies the index of the selected filter strings into this location. If nFilterIndex is 
      //zero, the custom filter is used. If nFilterIndex is zero and lpstrCustomFilter is NULL, the 
      //system uses the first filter in the buffer identified by lpstrFilter. If all three members are 
      //zero or NULL, the system does not use any filters and does not show any files in the file list 
      //control of the dialog box.
    }
  }

  return rv;
}


// Returns FALSE if any characters are in the filename.
TCI_BOOL CBrowseButton::IsEmpty()
{
  if (m_filenameEdit) {
    return (0 == m_filenameEdit->GetWindowTextLength());
  }
  return m_filename.IsEmpty();
}
