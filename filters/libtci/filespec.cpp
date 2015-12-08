
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "filespec.h"
#include "tcistrin.h"

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

static TCI_BOOL CharOKinFileName( U32 targ_mode,char ch );

PathAtom* PathAtom::NewNext(PAType typ, U32 n)
{ 
  type = typ; 
  off = n; 
  next = TCI_NEW(PathAtom); 
  return next; 
}



FileSpec::FileSpec(const TCIString& fname)
  : name(fname), atoms(0), baseAtoms(NULL)
{
  atoms =  ConvertName();
  AtomToName();
}


FileSpec::FileSpec(const FileSpec& fspec)
  : name(fspec.name), atoms(0), basePath(fspec.basePath)
{
  atoms =  ConvertName();
  baseAtoms = CopyPathAtoms(fspec.baseAtoms);
}


FileSpec::FileSpec(const FileSpec& path1, const FileSpec& path2)
  : baseAtoms(NULL)
{

  FileSpec t1(path1);
  TCI_ASSERT(t1.IsAbsolute());
  if (t1.IsRelative())
    t1 = t1.FullPath();       // Make it absolute (sort of)
  t1.MakePath();
  t1.Concat(path2);

  name  =  t1.name;
  atoms =  ConvertName();
}


FileSpec::~FileSpec()
{
  DelAtoms(atoms);
  atoms =  NULL;

  DelAtoms(baseAtoms);
  baseAtoms = NULL;
}


PathAtom* FileSpec::CopyPathAtoms(PathAtom* src)
{
  if (!src)
    return NULL;

  PathAtom* root_dst;
  PathAtom** locative = &root_dst;
  while (src) {
    *locative = TCI_NEW(PathAtom(*src));
    locative = &(**locative).next;
    src = src->next;
  }

  return root_dst;
}


// New version with better Macintosh path handling.
const FileSpec& FileSpec::Concat(const FileSpec& right)
{
  PathAtom* leftAtoms =  atoms;
  PathAtom* rightAtoms =  right.atoms;

  // Take care of the trivial cases
  if (right.IsEmpty())
    return *this;

  if (this->IsEmpty() || rightAtoms->type==PA_Root || rightAtoms->type==PA_Device) {
    DelAtoms(atoms);
    name =  right.name;
    atoms =  CopyPathAtoms(right.atoms);
    return *this;
  }
  

  // Count number of PA_Down atoms in the left path.
  // 'last' is node before the PA_End node.
  PathAtom* scan = leftAtoms;
  PathAtom* last = NULL;
  int downs = 0;
  while (scan) {
    if (scan->type == PA_Down)
        ++downs;
    if (scan->type != PA_End)
        last = scan;
    scan = scan->next;
  }

  // Add path separator if needed.
  if (last  &&  last->type == PA_Name) {
    ++downs;
    // add path separator
    PathAtom* node = TCI_NEW(PathAtom);
    node->type = PA_Down;
    node->off = name.GetLength();
    name += (TCICHAR)CHAM_PATHSEP;
    // link it in
    node->next = last->next;
    last->next = node;
    last = node;
    // fix PA_End offset
    scan = last->next;
    if (scan)
      scan->off = name.GetLength();
  }

  // Skip leading PA_Stay atom in the right path.
  if (rightAtoms->type == PA_Stay)
    rightAtoms = rightAtoms->next;

  // Skip leading PA_NoOp atom in the right path.
  if (rightAtoms->type == PA_NoOp)
    rightAtoms = rightAtoms->next;
    
  // Count the number of leading PA_Ups in the right path.
  int ups = 0;
  while (rightAtoms && rightAtoms->type == PA_Up) {
    ++ups;
    rightAtoms = rightAtoms->next;
    if (rightAtoms && rightAtoms->type == PA_NoOp)
      rightAtoms =  rightAtoms->next;
  }

  // Determine last node to keep in the left path.
  int depth = downs - ups;
  TCI_ASSERT(depth >= 0);

  if (depth <= 0) {
    //sls This seems to be wrong.  If left ends in ../, we are
    //    throwing it away.
    name = right.name.Mid(rightAtoms->off);
    DelAtoms(atoms);
    atoms = ConvertName();
    AtomToName();
    return *this;
  }

  PathAtom* keep = leftAtoms;
  while (keep) {
    if (keep->type == PA_Down)
      if (0 == --depth) break;
    keep = keep->next;
  }

  // Truncate left path
  name.Truncate(keep->next->off);

  // Concatenate path strings
  name += right.name.Mid(rightAtoms->off);

  DelAtoms(atoms);
  atoms =  ConvertName();
  AtomToName(); // this is needed to fix path separators if they were originally different

  return *this;
}



const FileSpec& FileSpec::operator=(const FileSpec& fs)
{
  if (this == &fs)
    return *this;

  DelAtoms(atoms);
  name =  fs.name;
  atoms =  ConvertName();

  DelAtoms(baseAtoms);
  baseAtoms = CopyPathAtoms(fs.baseAtoms);
  basePath = fs.basePath;

  return *this;
}


const FileSpec& FileSpec::operator=(const TCIString& fname)
{
  DelAtoms(atoms);
  name =  fname;
  atoms =  ConvertName();
  AtomToName();
  return *this;
}

////////   Path Manipulation   ////////////////////
void FileSpec::Empty() {

  name.Empty();
  if (atoms) {
    PathAtom* pAtom;
    do {
      pAtom =  atoms;
      atoms =  pAtom->next;
      delete pAtom;
    } while (atoms);
  }
}


FileSpec FileSpec::FullPath() const {

  TCIString dest;
  LPTSTR destbuf  =  dest.GetBuffer(CHAM_MAXPATH);
  LPTSTR lpszDummy;
  if (!GetFullPathName(name, CHAM_MAXPATH, destbuf, &lpszDummy))
  {
    TCI_ASSERT(FALSE);
    dest =  name;
  }
  dest.ReleaseBuffer(-1);

  return FileSpec(dest);
}


FileSpec FileSpec::GetName(TCI_BOOL noextension) const {

  if (!atoms)             // empty
    return FileSpec();

  I32 namelen,nameloc,extlen,extloc;
  LocateNameAndExtension(nameloc,namelen,extloc,extlen);
  FileSpec rv;     //empty
  if (nameloc >= 0)
    rv = FileSpec(name.Mid(nameloc, (noextension ? namelen : namelen+extlen)));
  return rv;
}


FileSpec FileSpec::GetDir() const {

  PathAtom* pAtom =  atoms;
  U32 count =  1;
  I32 last_down = -1;
  while (pAtom) {
    if (pAtom->type==PA_Down || pAtom->type==PA_Up || pAtom->type==PA_Stay || pAtom->type==PA_NoOp)
      last_down =  count;
    pAtom =  pAtom->next;
    count++;
  }

  return Convert(CF_CURR_OS, last_down);
}


I32 FileSpec::GetDirLength() const {

  PathAtom* pAtom =  atoms;
  I32 rv =  -1;
  while (pAtom) {
    if (pAtom->type==PA_Down || pAtom->type==PA_Up || pAtom->type==PA_Stay || pAtom->type==PA_NoOp)
      rv =  pAtom->off;
    pAtom =  pAtom->next;
  }

  return rv;
}

FileSpec FileSpec::SWGetShortPathName() const {
  TCIString strShortName;
	if (::GetShortPathName(name,strShortName.GetBuffer(CHAM_MAXPATH), CHAM_MAXPATH) == 0)
	{
		// rare failure case (especially on not-so-modern file systems)
		strShortName = name;
	}
	strShortName.ReleaseBuffer();
  return FileSpec(strShortName);
}


FileSpec FileSpec::SWGetLongPathName() const {

  HANDLE fhand;
  WIN32_FIND_DATA fdata;
  TCIString dest;
  const TCICHAR* buff =  name;
  FileSpec ret;

  PathAtom* pAtom =  atoms;
  while (pAtom) {           // skip over beginning stuff
    if (pAtom->type == PA_Name || pAtom->type == PA_Extension ||
        pAtom->type == PA_ExtMark ||pAtom->type == PA_End) {
      dest = TCIString(buff,pAtom->off);
      ret = dest;
      break;
    }
    pAtom = pAtom->next;
  }
  while (pAtom) {           // expand all the names
    if (pAtom->type == PA_Name || pAtom->type == PA_Extension ||
        pAtom->type == PA_ExtMark) {
      do {
        pAtom = pAtom->next;
      } while (pAtom && pAtom->type != PA_Down && pAtom->type != PA_End ); /* UP,STAY? */
      dest = TCIString(buff, pAtom->off);
      fhand = FindFirstFile(dest,&fdata);
      if (fhand == INVALID_HANDLE_VALUE)
        return ret;
      else {
        ret += fdata.cFileName;
        FindClose(fhand);
      }
    } else {
      pAtom =  pAtom->next;       
    }
  }

  return ret;
}


// Returns the directory depth for absolute paths.  For relative paths,
// an absolute path is first constructed from the base path and then the
// directory depth is determined.  Files that have no path or are relative
// without a base, a FALSE is returned.
TCI_BOOL FileSpec::GetDirDepth(I32& depth) const
{
  PathAtom* pAtom =  atoms;
  depth = -1;
  while (pAtom) {
    if (pAtom->type == PA_Down) ++depth;
    if (pAtom->type == PA_Up) --depth;
    pAtom =  pAtom->next;       
  }
  if (depth < 0)
    return FALSE;
  else
    return TRUE;
}

// Returns the directory that is "upLevels" up from the current file path.
FileSpec FileSpec::GetParentDir(I32 upLevels) const
{
  I32 depth;
  if (GetDirDepth(depth)) {
    depth = (depth - upLevels) + 1;
    if (depth > 0) {
      I32 count = 0;
      PathAtom* pAtom = atoms;
      while (pAtom) {
        ++count;
        if (pAtom->type == PA_Down) {
          if (--depth == 0)
            return Convert(CF_CURR_OS, count);
        }
        if (pAtom->type == PA_Up) ++depth;
        pAtom =  pAtom->next;
      }
    }
  }
  return FileSpec();
}


// Returns TRUE if file path is valid and relative.
TCI_BOOL FileSpec::IsRelative() const
{
  if (IsEmpty())
    return FALSE;

  PAType type = atoms->type;
  if ((type == PA_Name) || (type == PA_Up) || (type == PA_Stay))
    return TRUE;

  TCI_BOOL rv =  FALSE;
  PathAtom* pAtom = atoms;
  while (pAtom && !rv) {
    rv =  pAtom->type == PA_Up;
    pAtom =  pAtom->next;
  }

  return rv;
}


// Returns TRUE if file path is valid and absolute.
TCI_BOOL FileSpec::IsAbsolute() const
{
  if (IsEmpty())
    return TRUE;

  PAType type = atoms->type;
  TCI_BOOL rv =  (type == PA_Device) || (type == PA_Root) || (type == PA_URL);
  PathAtom* pAtom = atoms;
  while (pAtom && rv) {
    rv =  pAtom->type != PA_Up;
    pAtom =  pAtom->next;
  }

  return rv;
}

// The base path is used to convert relative paths to absolute
// (and vice versa).  This procedure accepts only absolute paths.
// It will return FALSE if the new base path is not valid or absolute.
TCI_BOOL FileSpec::SetBasePath(const FileSpec& fs)
{
  if (fs.IsValid() && fs.IsAbsolute()) {
    basePath = fs.name;
    DelAtoms(baseAtoms);
    baseAtoms = CopyPathAtoms(fs.atoms);
    return TRUE;
  }
  else
    return FALSE;
}

// Returns the current base path.
FileSpec FileSpec::GetBasePath() const
{
  return FileSpec(basePath);
}


// Helper function for comparing atoms
TCI_BOOL FileSpec::EqualAtom(PathAtom* p1, const TCIString* n1,
                             PathAtom* p2, const TCIString* n2) const
{
  TCI_BOOL rv;
  if (p1 && p2 && p1->type == p2->type) {
    if (p1->type == PA_Device  ||  p1->type == PA_Name) {
      U32 len1 =  p1->next->off - p1->off;
      U32 len2 =  p2->next->off - p2->off;
      TCIString one(n1->Mid(p1->off, len1));
      TCIString two(n2->Mid(p2->off, len2));
      #ifdef CASE_SENSITIVE
        rv = (0 == one.Compare(two));
      #else
        rv = (0 == one.CompareNoCase(two));
      #endif
    }
    else
      rv = TRUE;
  }
  else
    rv = FALSE;

  return rv;
}
  
// Returns TRUE if the file path is in or below the base path directory.
TCI_BOOL FileSpec::IsBelowBasePath() const
{
  if (!baseAtoms || name.IsEmpty())
    return FALSE;

  PathAtom* p1 = baseAtoms;
  const TCIString* n1 = &basePath;

  PathAtom* p2;
  const TCIString* n2; 

  FileSpec fs;
  if (IsRelative()) {
    // Convert relative path to absolute for accurate comparison
    fs = basePath;
    fs.Concat(*this);
    p2 = fs.atoms;
    n2 = &fs.name;
  }
  else {
    p2 = atoms;
    n2 = &name;
  }

  // Compare atoms until they are different
  while (p1 && p2 && EqualAtom(p1, n1, p2, n2)) {
    p1 = p1->next;
    p2 = p2->next;
  }

  TCI_BOOL rv;
  if ((p1 == NULL) || (p1->type == PA_End))
    rv = TRUE;
  else if (p2->type == PA_End && (p1->type == PA_Down)
           && (p1->next) && (p1->next->type == PA_End))
    rv = TRUE;
  else
    rv = FALSE;

  return rv;
}


// Make file path absolute by concatenating the relative
// file path onto base path.
TCI_BOOL FileSpec::MakeAbsolute()
{
  if (IsEmpty())
    return FALSE;

  TCI_BOOL rv =  !IsRelative();
  if (!rv) {
    if (atoms->type != PA_Device && atoms->type != PA_Root && baseAtoms) {
      FileSpec fs(basePath);
      fs.Concat(*this);

      DelAtoms(atoms);
      name =  fs.name;
      atoms =  ConvertName();

      rv =  MakeAbsolute();   // to take care of 
    } else if (atoms->type == PA_Device || atoms->type == PA_Root) {
      PathAtom* prevprevAtom;
      PathAtom* prevAtom;
      PathAtom* pAtom;
      TCIString temp;
      while (TRUE) {
        prevprevAtom =  NULL;
        prevAtom =  NULL;
        pAtom =  atoms;
        while (pAtom && pAtom->type != PA_Up) {
          prevprevAtom =  prevAtom;
          prevAtom =  pAtom;
          pAtom =  pAtom->next;
        }
        if (!pAtom)
          break;      // no more PA_Up

        if (!prevprevAtom || !pAtom->next) {
          TCI_ASSERT(FALSE);
          break;      // Uh-oh
        }

        temp =  name.Left(prevprevAtom->off) + name.Mid(pAtom->next->off);
        prevprevAtom->next =  pAtom->next;
        delete prevAtom;
        delete pAtom;
      }
      if (!temp.IsEmpty()) {
        DelAtoms(atoms);
        name =  temp;
        atoms =  ConvertName();
      }
    } else {
      TCI_ASSERT(FALSE);
    }
  }

  return rv;
}


// Make file path relative to base path.
TCI_BOOL FileSpec::MakeRelative()
{
  if (!baseAtoms || IsEmpty())
    return FALSE;

  PathAtom* p1 = baseAtoms;
  TCIString* n1 = &basePath;
  PathAtom* p2 = atoms;
  TCIString* n2 = &name;

  // Compare atoms until they are different
  while ( p1 && p2 && EqualAtom(p1, n1, p2, n2) ) {
    p1 = p1->next;
    p2 = p2->next;
  }

  if (p1 && p2) {
    PAType t1 = p1->type;
    PAType t2 = p2->type;
    // Check for same volumes; we know that p1 (basePath) starts with
    // a PA_Device or PA_Root.  If p2 (name) is not absolute or not the
    // same volume, it will fail below.
    if (t1 == PA_Device  ||  t1 == PA_Root)
      return FALSE;

    if (t2 == PA_End  &&  t1 == PA_Down) {
      // p2 (name) is a path and is missing the trailing PA_Down atom.
      p1 = p1->next;
      p2 = NULL;
    }
  }

  // Count number of downs in base path (after they differ)
  int downs = 0;
  while (p1) {
    if (p1->type == PA_Down)  ++downs;
    p1 = p1->next;
  }

  
  TCIString remain;
  if (p2)
    remain = name.Mid(p2->off);

  name.Truncate(0);
  if (CHAM_OSMODE == CF_DOS) {
    while (downs--)
      name += "..\\";
  }
  else if ( CHAM_OSMODE==CF_INTERNAL || CHAM_OSMODE==CF_UNIX ) {
    while (downs--)
      name += "../";
  }
  else if ( CHAM_OSMODE==CF_MAC ) {
    ++downs;
    while (downs--)
      name += ":";
  }

  name += remain;
  DelAtoms(atoms);
  atoms =  ConvertName();
  return TRUE;
}



TCI_BOOL FileSpec::IsURL() const
{
  if (atoms  &&  atoms->type == PA_URL)
	  return TRUE;

  return FALSE;
}



TCI_BOOL FileSpec::MakePath()
{
  // Check for special case "c: or \\machineName\c" or URL
  PathAtom* pAtom =  atoms;
  PathAtom* lastname =  NULL;
  TCI_BOOL ends_in_down =  FALSE;
  if (pAtom && (pAtom->type==PA_Device || pAtom->type==PA_URL)) {
    pAtom =  pAtom->next;
    if (pAtom && pAtom->type==PA_NoOp) {  // c:
      pAtom =  pAtom->next;
      if (pAtom && pAtom->type==PA_End) {
        lastname = atoms->next;
      }
    } else if (pAtom && pAtom->type == PA_End) { // \\machineName\c
        lastname = atoms;
    }
  }

  if (!lastname) {
    pAtom =  atoms;
    while (pAtom) {
      if (pAtom->type==PA_Down || pAtom->type==PA_Stay) {
        ends_in_down =  TRUE;
      } else if (pAtom->type!=PA_End && pAtom->type!=PA_NoOp) {
        ends_in_down =  FALSE;
        if (pAtom->type==PA_Name)
          lastname =  pAtom;
      }
      pAtom =  pAtom->next;
    }
  }

  TCI_BOOL rv =  ends_in_down;
  if (!rv && lastname) {
    pAtom =  TCI_NEW(PathAtom);
    pAtom->off =  name.GetLength();
    pAtom->type =  PA_Down;
    pAtom->next =  lastname->next;
    lastname->next =  pAtom;
    pAtom =  pAtom->next;
    while (pAtom) {
      pAtom->off++;
      pAtom =  pAtom->next;
    }
    AtomToName();
    rv =  TRUE;
  }

  return rv;
}

TCI_BOOL FileSpec::UnmakePath() {

  TCI_BOOL ends_in_name =  FALSE;
  PathAtom* pAtom =  atoms;
  PathAtom* prevAtom =  NULL;
  PathAtom* lastdown =  NULL;
  while (pAtom) {
    if (pAtom->type==PA_Name) {
      ends_in_name =  TRUE;
    } else if (pAtom->type!=PA_End && pAtom->type!=PA_NoOp) {
      ends_in_name =  FALSE;
      if (pAtom->type==PA_Down)
        lastdown =  prevAtom;
    }
    prevAtom =  pAtom;
    pAtom =  pAtom->next;
  }
  TCI_BOOL rv =  ends_in_name;
  if (!rv && lastdown) {
    int inc =  lastdown->next->off - lastdown->next->next->off;
    pAtom =  lastdown->next;
    lastdown->next =  pAtom->next;
    delete pAtom;
    pAtom =  lastdown->next;
    while (pAtom) {
      pAtom->off +=  inc;
      pAtom =  pAtom->next;
    }
    AtomToName();
    rv =  TRUE;
  }

  return rv;
}


// code copied from above function; should probably be merged
TCI_BOOL FileSpec::IsPath() const {

  TCI_BOOL ends_in_name =  FALSE;
  PathAtom* pAtom =  atoms;
  PathAtom* prevAtom =  NULL;
  PathAtom* lastdown =  NULL;
  while (pAtom) {
    if (pAtom->type==PA_Name) {
      ends_in_name =  TRUE;
    } else if (pAtom->type!=PA_End && pAtom->type!=PA_NoOp) {
      ends_in_name =  FALSE;
      if (pAtom->type==PA_Down)
        lastdown =  prevAtom;
    }
    prevAtom =  pAtom;
    pAtom =  pAtom->next;
  }

  return (!ends_in_name);
}


TCI_BOOL FileSpec::SetDriveSameAs(const FileSpec& fs)
{
//first we ensure that the source has a device to copy:
  PathAtom* pDrive = fs.atoms;
  PathAtom* pSrcNext = (PathAtom*)NULL;
  I32 drivelen(0);
  while (pDrive && pDrive->type!=PA_Device)
    pDrive =  pDrive->next;
  if (!pDrive)
    return FALSE;

  //now decide what we need to replace (if anything)
  I32 offset(0);
  I32 olddrivelen(0);
  I32 remainder(0);
  PathAtom* pOldDrive = atoms;
//  PathAtom* pOldDown = (PathAtom*)NULL;
  PathAtom* pPrev = (PathAtom*)NULL;
  PathAtom* pNext = (PathAtom*)NULL;
  while (pOldDrive && pOldDrive->type!=PA_Device) {
    pPrev = pOldDrive;
    pOldDrive = pOldDrive->next;
  }
//  pNext = pOldDrive ? pOldDrive->next : atoms;
  if (!pOldDrive) {  
  //in this case we'll copy the device name together with following atoms (e.g. ":\")
    pNext = atoms;
    while (pNext && pNext->type!=PA_Name)
      pNext = pNext->next;
  } else { //in this case we'll just plug the new device name in where the old one was
    pNext = pOldDrive->next;
    while(pNext && pNext->type!=PA_Down)
      pNext = pNext->next;
    if (!pNext)
      return(FALSE);
    offset = pOldDrive->off;
  }

//  TCI_ASSERT(pNext);  //insist on a name here somewhere?
  olddrivelen = pNext ? pNext->off : name.GetLength();
  olddrivelen -= offset;
  if (!pOldDrive)
    pPrev = (PathAtom*)NULL;

  pSrcNext = pDrive->next;
  if (!pOldDrive) {
    //this is the case where we want to actually want to copy 
    while (pSrcNext && pSrcNext->type!=PA_Name && pSrcNext->type!=PA_End)
      pSrcNext=pSrcNext->next;
  } else {
    while(pSrcNext && pSrcNext->type!=PA_Down && pSrcNext->type!=PA_End)
      pSrcNext = pSrcNext->next;
    if (!pSrcNext)
      return(FALSE);
  }
  drivelen = pSrcNext ? pSrcNext->off : fs.name.GetLength();
  drivelen -= pDrive->off;

  remainder = name.GetLength() - offset - olddrivelen;

  //now prepare the new atoms and string:
  TCIString replace;
  if (offset)
    replace += name.Left(offset);
  replace += fs.name.Mid(pDrive->off,drivelen);
//  PathAtom* pNewDrive = TCI_NEW(PathAtom);

  PathAtom* pNewDrive = TCI_NEW(PathAtom(*pDrive));
  pNewDrive->off = offset;
  I32 delta = offset - pDrive->off;
  if (pPrev)
    pPrev->next = pNewDrive;
  else
    atoms = pNewDrive;
  PathAtom* pNew = pPrev = pNewDrive;
  for (PathAtom* pTemp=pDrive->next;pTemp && pTemp!=pSrcNext;pTemp=pTemp->next) {
    pNew = TCI_NEW(PathAtom(*pTemp));
    pNew->off += delta;
    pPrev->next = pNew;
    pPrev = pNew;
  }

  pNew->next = pNext;
  delta = replace.GetLength() - pNext->off;
  replace += name.Mid(offset+olddrivelen,remainder);
  name = replace;
  
  for (PathAtom* pAtom=pNext;pAtom;pAtom=pAtom->next)
    pAtom->off += delta;

  PathAtom* pTemp=pOldDrive;
  while (pTemp && pTemp!=pNext) {
    PathAtom* pDelete = pTemp;
    pTemp = pTemp->next;
    delete pDelete;
  }

  AtomToName();

  return TRUE;
}


void FileSpec::SetExtension(const FileSpec& fs)
{
  if (IsEmpty()) {
    *this =  fs;
    return;
  }

  PathAtom* pAtom =  atoms;
  PathAtom* pLastDown =  NULL;
  while (pAtom) {
    if (pAtom->type==PA_Down || pAtom->type==PA_Stay || pAtom->type==PA_NoOp)
      pLastDown =  pAtom;
    else if (pAtom->type==PA_Device && pAtom->next && pAtom->next->type==PA_NoOp) {
      pAtom = pAtom->next;
      pLastDown =  pAtom;
    }
    pAtom =  pAtom->next;
  }

  if (!pLastDown)
    pLastDown =  atoms;   // assume no path information
  else
    pLastDown =  pLastDown->next;

  TCI_ASSERT(pLastDown);
  TCI_ASSERT(pLastDown->type==PA_Name);
  if (pLastDown->type==PA_Name) {
    U32 len =  pLastDown->next->off - pLastDown->off;
    TCIString temp(name.Mid(pLastDown->off, len));
    int off =  temp.ReverseFind('.');
    if (off >= 0)
      name.SetDataLength(pLastDown->off + off);
    if (fs.name.IsEmpty())
      name.ReleaseBuffer(name.GetLength());
    else
      name +=  fs.name;
    DelAtoms(atoms);                         // re-parse as we may have added/deleted atoms
    atoms =  ConvertName();
  }
}


void FileSpec::SetExtension(const TCIString& s) 
{
  // We allow an empty string to clear the extension.
  // Otherwise the string has to start with a dot
  TCI_ASSERT(s.GetLength() == 0 || s[0] == '.');
  SetExtension(FileSpec(s));
}


//a private utility function:
void FileSpec::LocateNameAndExtension(I32& nameloc,I32& namelen,I32& extloc,I32& extlen) const {
  nameloc = extloc = -1;
  namelen = extlen = 0;

  if (!atoms)                 // empty
    return;

  PathAtom* pAtom =  atoms;
  PathAtom* pLastDown = NULL;
  PathAtom* pLastName = NULL;
  PathAtom* pExt =  NULL;  //in case there's an extension atom
  while (pAtom) {
    switch(pAtom->type) {
      case PA_Down:
        pLastDown =  pAtom;       
      break;
      case PA_Name:
        pLastName =  pAtom;       
      break;
      case PA_Device:
      case PA_Stay:
      case PA_Up:
        if (pAtom->next && pAtom->next->type==PA_NoOp)
          pLastDown =  pAtom->next;
      break;
      case PA_ExtMark:
        pExt = pAtom;             
      break;
      default:
      break;
    }
    pAtom =  pAtom->next;
  }

  if (!pLastName)
    pLastName = pLastDown ? pLastDown->next : atoms;
  TCI_ASSERT(pLastName);

  if (pLastName && pLastName->type==PA_Name) {
    nameloc = pLastName->off;
    namelen =  pLastName->next->off - pLastName->off;
  }

  if (pExt) {
    extloc = pExt->off;

    extlen = (pExt->next && pExt->next->next) ?
                       pExt->next->next->off - pExt->off
                     : pExt->next->off - pExt->off;
  } else if (nameloc >= 0) {
    TCIString temp = name.Mid(nameloc, namelen);
    extloc = temp.ReverseFind('.');
    if (extloc >= 0) {
      extlen = namelen - extloc;
      namelen = extloc;
      extloc += nameloc;
    }
  } 
}

FileSpec FileSpec::GetExtension() const {

  if (!atoms)                  // empty
    return FileSpec();
  PathAtom* pAtom =  atoms;
  PathAtom* pLastDown =  NULL;
  while (pAtom) {
    switch (pAtom->type)
    {
    case PA_Root:
    case PA_Down:
    case PA_ExtMark:
      pLastDown =  pAtom;
      break;
    case PA_Device:
    case PA_Up:
    case PA_Stay:
      if (pAtom->next && pAtom->next->type==PA_NoOp) {
        pAtom = pAtom->next;
        pLastDown =  pAtom;
      }
      break;
    default:
      break;
    }
    pAtom =  pAtom->next;
  }

  if (!pLastDown)
    pLastDown =  atoms;   // assume no path information
  else if (pLastDown->type!=PA_ExtMark)
    pLastDown =  pLastDown->next;

  TCI_ASSERT(pLastDown);
  int off;
  TCIString temp;
  if (pLastDown->type==PA_ExtMark) {
    U32 len = (pLastDown->next && pLastDown->next->next) ?
                       pLastDown->next->next->off - pLastDown->off
                     : pLastDown->next->off - pLastDown->off;
    temp = name.Mid(pLastDown->off, len);
    off = 0;
  } else if (pLastDown->type==PA_Name) {
    U32 len =  pLastDown->next->off - pLastDown->off;
    temp = name.Mid(pLastDown->off, len);
    off  = temp.ReverseFind('.');
  } else
    off = -1;
  if ( off > 0)
    return FileSpec(temp.Mid(off));
  else if (off == 0)
    return FileSpec(temp);
  else 
   return FileSpec();
}

// According to Windows, this should return the path to the root
// directory on the drive.
FileSpec FileSpec::GetDrive() const {

  FileSpec rv;
  PathAtom* pAtom =  atoms;
  if (!pAtom || pAtom->type!=PA_Device)
    return rv;

  if (!pAtom->next) {
    TCI_ASSERT(FALSE);
    return rv;
  }

  // Get drive name.
  if (pAtom->next->type==PA_NoOp) {  // Usual case
    TCI_ASSERT(pAtom->next->next);
    rv = name.Mid(pAtom->off, pAtom->next->next->off - pAtom->off);
  } else {                           // UNC   case
    rv = name.Mid(pAtom->off, pAtom->next->off - pAtom->off);
  }
  rv.MakePath();
  return rv;
}


TCIString FileSpec::ConvertPath(U32 mode) const
{
  TCIString dest;
  dest.GetBuffer(CHAM_MAXPATH);
  const TCICHAR* buff =  name;

  PathAtom* pAtom =  atoms;
  PathAtom* pPrevAtom =  0;

  if (mode==CF_CURR_OS) {
    mode = CHAM_OSMODE;
  }

  if (IsURL())
    mode = CF_URL;

  const TCICHAR* data;
  U32 len;
  while (pAtom) {

    data =  NULL;
    len =  0;

    switch (pAtom->type) {

      case PA_Device : {
        TCI_ASSERT(pAtom->next);
        if (mode==CF_DOS || mode==CF_INTERNAL || mode==CF_MAC) {
          data =  buff + pAtom->off;            // see extra colon character below
          len =  pAtom->next->off - pAtom->off;
        } else {
          TCI_ASSERT(FALSE);
        }
      }
      break;

      case PA_URL : {
        TCI_ASSERT(mode == CF_URL);
          data =  buff + pAtom->off;            // see extra colon character below
          len =  pAtom->next->off - pAtom->off;
      }
      break;

      case PA_Up     : {
        if      ( mode==CF_DOS )
          data  =  "..\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX || mode==CF_URL ) 
          data  =  "../";
        else if ( mode==CF_MAC )
          data =  (pPrevAtom && pPrevAtom->type!=PA_Up ? "::" : ":");
      }
      break;

      case PA_Down   : {
        if      ( mode==CF_DOS )
          data  =  "\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX || mode==CF_URL) 
          data  =  "/";
        else if ( mode==CF_MAC ) 
          data  =  ":";
      }
      break;

      case PA_Stay   : {
        if      ( mode==CF_DOS )
          data  =  ".\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX || mode==CF_URL) 
          data  =  "./";
        else if ( mode==CF_MAC ) 
          data  =  ":";
      }
      break;

      case PA_ExtMark: {
        data  =  ".";
      }
      break;
      
      case PA_Extension:
      case PA_UrlArg:
      case PA_Name: {
        TCI_ASSERT(pAtom->next);
        data =  buff + pAtom->off;
        len =  pAtom->next->off - pAtom->off;
      }
      break;

      case PA_End: 
        //if ( di<limit ) zdest[di++] =  0;
      break;

      case PA_Root: 
        if ( mode==CF_UNIX || mode==CF_INTERNAL || mode==CF_URL) {
          data  =  "/";
        } else if ( mode==CF_DOS ) {
          data  =  "\\";
        } else {
          // error          
          TCI_ASSERT( FALSE );
        }
      break;

      case PA_NoOp   :    break;
      default : break;
    }

    if ( data ) {
      if (!len)
        dest +=  data;
      else {
        TCIString datastr(data,len);
        if (pAtom->type==PA_Device && mode!=CF_MAC) {
          if (len==1)
            datastr += ':';    // always append colon to device name
          else if (mode==CF_INTERNAL)
            datastr.Replace("\\","/");
        } 
        dest +=  datastr;
      }
    }
    
    pPrevAtom =  pAtom;
    pAtom     =  pAtom->next;
  }

  return dest;
}


I32 FileSpec::ComparePath(const FileSpec& fspec) const {

  PathAtom* p1 =  atoms;
  PathAtom* p2 =  fspec.atoms;

  enum PAType last_type =  PA_NoOp;
  I32 rv =  0;
  while (!rv && p1 && p2) {
    if (p1->type != p2->type) {
      if (p1->type == PA_End) {
        // we ignore trailing path separators
        if (p2->type != PA_Down || p2->next->type != PA_End)
          rv =  -1;   // p1 < p2
      } else if (p2->type == PA_End) {
        // we ignore trailing path separators
        if (p1->type != PA_Down || p1->next->type != PA_End)
          rv =  1;    // p1 > p2
      } else
          rv =  2;    // not comparable, really
      break;
    }
    switch (p1->type) {
      case PA_Device:
      case PA_Extension:
      case PA_UrlArg:
      case PA_Name: {
        U32 len1 =  p1->next->off - p1->off;
        U32 len2 =  p2->next->off - p2->off;
        TCIString one(name.Mid(p1->off, len1));
        TCIString two(fspec.name.Mid(p2->off, len2));
      #ifdef CASE_SENSITIVE
        rv =  one.Compare(two);
      #else
        rv =  one.CompareNoCase(two);
      #endif
      }
      break;

      case PA_URL: {
        U32 len1 =  p1->next->off - p1->off;
        U32 len2 =  p2->next->off - p2->off;
        TCIString one(name.Mid(p1->off, len1));
        TCIString two(fspec.name.Mid(p2->off, len2));
        rv =  one.CompareNoCase(two);
      }
      break;
      
      default:
      break;  
    }
    if (!rv) {
      last_type =  p1->type;
      p1 =  p1->next;
      p2 =  p2->next;
    }
  }

  if ((!p1 || !p2) && (p1 || p2)) {
    if (!p1)
      rv =  -1;
    else
      rv =  1;
  }

  return rv;
}


////////   Miscellaneous   ////////////////////

// attempts to abbreviate name to < maxlen characters in a logical manner
// returns FALSE if can't do so
// if bAtLeastName then dest is always set, even if too long
//
// it is expected that this is an absolute path and not a directory

TCI_BOOL FileSpec::AbbrevName(TCIString& dest, I32 maxlen, TCI_BOOL bAtLeastName) const {

  TCI_BOOL rv =  FALSE;

  dest.Empty();
  if (name.GetLength() < maxlen) {
    dest =  name;
    rv =  TRUE;
  } else {
    U32 mode =  CHAM_OSMODE;

    I32 name_len =  0;
    // get the size of the name
    PathAtom* pAtom =  atoms;
    PathAtom* nameAtom =  NULL;
    while (pAtom) {
      if ( pAtom->type==PA_Name ) {
        name_len =  pAtom->next->off - pAtom->off;
        nameAtom =  pAtom;
      }
      pAtom =  pAtom->next;
    }
    TCI_ASSERT(name_len);
    TCI_ASSERT(nameAtom);
    if (name_len == 0 || nameAtom == NULL)
      return FALSE;  // pretty much internal error
    if (name_len >= maxlen) {
      if (bAtLeastName)
        dest = name.Mid(nameAtom->off, name_len);
      return FALSE;
    }

    pAtom = atoms;
    while (pAtom) {

      switch (pAtom->type ) {

        case PA_URL : {
          mode = CF_URL;
          I32 len =  pAtom->next->off - pAtom->off;
          if (len+name_len+5 <= maxlen) {     //   URL/.../lastname  -  5 = 3*.+2*/ 
            dest +=  name.Mid(pAtom->off, len);
            dest +=  CHAM_PATHMODE;
          } else if (name_len+4 <= maxlen) {
            dest +=  "...";
            dest +=  CHAM_PATHMODE;
          }
        }
        break;

        case PA_Device : {
          if (mode==CF_DOS || mode==CF_MAC) {
            I32 len =  pAtom->next->off - pAtom->off;
            if (len+name_len+5 <= maxlen) {     //   c:...\lastname  -  5 = 3*.+1*:+1*\ 
              dest +=  name.Mid(pAtom->off, len);
              if (len==1 && mode != CF_MAC)
                dest +=  ":";
              dest +=  CHAM_PATHMODE;
            } else if (name_len+4 <= maxlen) {  // long UNC names get here
              dest +=  "...";
              dest +=  CHAM_PATHMODE;
            }
          } else {
            TCI_ASSERT(FALSE);
          }
        }
        break;

        case PA_Up:
          // relative path ....
          TCI_ASSERT(FALSE);//  error
        break;

        case PA_Name : {
          if (pAtom==nameAtom) {
            if (dest.GetLength()+name_len <= maxlen)
              dest +=  name.Mid(pAtom->off, name_len);
            else
              dest =  name.Mid(pAtom->off, name_len);
            rv =  TRUE;
            pAtom =  NULL;  // to get out of loop
            break;
          }
          I32 len =  pAtom->next->off - pAtom->off;
          if ((len+name_len+dest.GetLength()+6) <= maxlen) {     //   \namehere\...\lastname  -  6 = 3*\+3*.
            dest +=  name.Mid(pAtom->off, len);
            dest +=  CHAM_PATHMODE;
          } else if ((name_len+dest.GetLength()+4) <= maxlen) {
            dest +=  "...";
            dest +=  CHAM_PATHMODE;
          }
        }
        break;

        case PA_End   : 
          //
        break;

        case PA_Root :
          if (mode==CF_UNIX || mode==CF_INTERNAL) {
            dest +=  "/";
          } else if (mode==CF_DOS) {
            // assuming full path!
            I32 len =  pAtom->next->off - pAtom->off;
            if ((name_len+len+4) <= maxlen) {     // \...\name is possible
              dest +=  name.Mid(pAtom->off, len);
            }
          } else {
            // error          
            TCI_ASSERT( FALSE );
          }
        break;

        case PA_NoOp   :    break;
        case PA_ExtMark :    break;
        case PA_Extension     :    break;
        case PA_UrlArg: break;
        default : break;
      }
      if (pAtom)
        pAtom =  pAtom->next;
    }
  }

  return rv;
}


TCI_BOOL FileSpec::IsUNCName() const {
  TCI_BOOL rv = FALSE;
  PathAtom* pAtom =  atoms;
  while (pAtom && pAtom->type!=PA_Device)
    pAtom =  pAtom->next;
  if (pAtom) {
    TCI_ASSERT(pAtom->next);
    U32 len = pAtom->next ? pAtom->next->off - pAtom->off : name.GetLength() - pAtom->off;
    if (len > 1) {
      const TCICHAR* pName = name;
      rv = (pName[pAtom->off]=='\\' && pName[pAtom->off + 1]=='\\')
           || (pName[pAtom->off]=='/' && pName[pAtom->off + 1]=='/');
    }
  }
  return rv;
}


/////////////////// Private functions ///////////////////
void FileSpec::DelAtoms(PathAtom* pAtom)
{
  PathAtom* temp;
  while (pAtom) {
    temp =  pAtom;
    pAtom =  temp->next;
    delete temp;
  }
}


void FileSpec::AtomToName() {

  TCIString dest;
  LPTSTR destbuf =  dest.GetBuffer(CHAMmax(2*name.GetLength(),CHAM_MAXPATH));

  const TCICHAR* buff =  name;

  PathAtom* pAtom =  atoms;
  PathAtom* pPrevAtom =  0;

  U32 mode;
  if (IsURL())
    mode = CF_URL;
  else
    mode = CHAM_OSMODE;

  const TCICHAR* data;
  U32 len;
  while (pAtom) {

    data =  NULL;
    len =  0;

    switch ( pAtom->type ) {

      case PA_Device : {
        TCI_ASSERT(pAtom->next);
        if (mode==CF_DOS || mode==CF_INTERNAL  || mode==CF_MAC) {  // see extra colon below
          data =  buff + pAtom->off;
          len =  pAtom->next->off - pAtom->off;
        } else {
          TCI_ASSERT(FALSE);
        }
      }
      break;

      case PA_URL : {
        TCI_ASSERT(mode == CF_URL);
        data =  buff + pAtom->off;
        len =  pAtom->next->off - pAtom->off;
      }
      break;

      case PA_Up     : {
        if      ( mode==CF_DOS )
          data  =  "..\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX || mode==CF_URL) 
          data  =  "../";
        else if ( mode==CF_MAC )
          data =  (!pPrevAtom || pPrevAtom->type!=PA_Up ? "::" : ":");
      }
      break;

      case PA_Down   : {
        if      ( mode==CF_DOS )
          data  =  "\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX || mode==CF_URL) 
          data  =  "/";
        else if ( mode==CF_MAC ) 
          data  =  ":";
      }
      break;

      case PA_Stay   : {
        if      ( mode==CF_DOS )
          data  =  ".\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX || mode==CF_URL) 
          data  =  "./";
        else if ( mode==CF_MAC ) 
          data  =  ":";
      }
      break;

      case PA_ExtMark: {
        data  =  ".";
      }
      break;
      
      case PA_Extension:
      case PA_UrlArg:
      case PA_Name: {
        TCI_ASSERT(pAtom->next);
        data =  buff + pAtom->off;
        len =  pAtom->next->off - pAtom->off;
      }
      break;

      case PA_End: 
      break;

      case PA_Root: 
        if ( mode==CF_UNIX || mode==CF_INTERNAL || mode==CF_URL) {
          data  =  "/";
        } else if ( mode==CF_DOS ) {
          data  =  "\\";
        } else {
          // error? 
          TCI_ASSERT( FALSE );
        }
      break;

      case PA_NoOp   :    break;
      default        :    break;
    }

    if ( data ) {
      if (!len)
        dest +=  data;
      else {
        memcpy(destbuf+dest.GetLength(), data, len);
        dest.SetDataLength(dest.GetLength() + len);
        destbuf[dest.GetLength()] =  0;
      }
      if (pAtom->type==PA_Device && len==1 && mode!=CF_MAC)  // always append colon to device names
        dest += ':';
    }
    
    if (pAtom->type != PA_NoOp)
      pPrevAtom =  pAtom;
    pAtom =  pAtom->next;
  }
  dest.ReleaseBuffer(-1);   // compute length

  name =  dest;
}


//
// num is the number of atoms to include in the output
// if num < 0 then return empty filespec
// if num = 0 then convert all atoms
FileSpec FileSpec::Convert(U32 mode, I32 num) const {

  if (num<0)
    return FileSpec();
  
  TCIString dest;

  LPTSTR destbuf =  dest.GetBuffer(CHAMmax(2*name.GetLength(),CHAM_MAXPATH));

  const TCICHAR* buff =  name;

  PathAtom* pAtom =  atoms;
  PathAtom* pPrevAtom =  0;

  if (mode==CF_CURR_OS) {
    mode = CHAM_OSMODE;
  }

  if (IsURL())
    mode = CF_URL;

  const TCICHAR* data;
  U32 len;
  I32 count =  0;
  while (pAtom) {

    if (num && count>=num)
      break;

    data =  NULL;
    len =  0;

    switch ( pAtom->type ) {

      case PA_Device : {
        TCI_ASSERT(pAtom->next);
        if (mode==CF_DOS || mode==CF_INTERNAL || mode==CF_MAC) {    // see extra colon below
          data =  buff + pAtom->off;
          len =  pAtom->next->off - pAtom->off;
        } else {
          TCI_ASSERT(FALSE);
        }
      }
      break;

      case PA_URL : {
        TCI_ASSERT(mode == CF_URL);
        data =  buff + pAtom->off;
        len =  pAtom->next->off - pAtom->off;
      }
      break;

      case PA_Up     : {
        if      ( mode==CF_DOS )
          data  =  "..\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX || mode==CF_URL) 
          data  =  "../";
        else if ( mode==CF_MAC )
          data =  (!pPrevAtom || pPrevAtom->type!=PA_Up ? "::" : ":");
      }
      break;

      case PA_Down   : {
        if      ( mode==CF_DOS )
          data  =  "\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX || mode==CF_URL ) 
          data  =  "/";
        else if ( mode==CF_MAC ) 
          data  =  ":";
      }
      break;

      case PA_Stay   : {
        if      ( mode==CF_DOS )
          data  =  ".\\";
        else if ( mode==CF_INTERNAL || mode==CF_UNIX  || mode==CF_URL) 
          data  =  "./";
        else if ( mode==CF_MAC ) // this only works for first token
          data  =  ":";
      }
      break;

      case PA_ExtMark: {
        data  =  ".";
      }
      break;
      
      case PA_Extension:
      case PA_UrlArg:
      case PA_Name: {
        TCI_ASSERT(pAtom->next);
        data =  buff + pAtom->off;
        len =  pAtom->next->off - pAtom->off;
      }
      break;

      case PA_End: 
      break;

      case PA_Root: 
        if ( mode==CF_UNIX || mode==CF_INTERNAL || mode==CF_URL ) {
          data  =  "/";
        } else if ( mode==CF_DOS ) {
          data  =  "\\";
        } else {
          // error          
          TCI_ASSERT( FALSE );
        }
      break;

      case PA_NoOp   :    break;
      default : break;
    }

    if ( data ) {
      if (!len)
        dest +=  data;
      else {
        U32 prevlen = dest.GetLength();
        memcpy(destbuf+prevlen, data, len);
        dest.SetDataLength(dest.GetLength() + len);
        destbuf[dest.GetLength()] =  0;
        if (pAtom->type==PA_Device && mode!=CF_MAC) {  
          if (len==1)
            dest +=  ':';     // always append colon to device names
          else if (mode==CF_INTERNAL)
            dest.Replace("\\","/",prevlen);
        }
      }
    }
    
    pPrevAtom =  pAtom;
    pAtom =  pAtom->next;
    count++;
  }
  dest.ReleaseBuffer(-1);   // compute length

  return FileSpec(dest);
}


PathAtom* FileSpec::ConvertName()
{
// Declare and initialize g_URLToken once for speed.
//Made local to function to avoid linking problems with "global" static objects
  static const TCIString g_URLToken("://");
  static const TCIString g_pseudoURL1("mailto:");
  static const TCIString g_pseudoURL2("news:");

  int len =  name.GetLength();
  if (!len  ||  !name[0])
    return 0;

  //
  // check initial characters
  //
  int pos;
  int si  =  0;
  TCICHAR ch =  name[si];
    
  PathAtom* pPathAtom = TCI_NEW(PathAtom);
  PathAtom* pCurrAtom = pPathAtom;

  if ( ch =='.' ) {  // DOS or UNIX - "..\..\nnn"  OR  ".\nnn"  OR  "."  OR  ".ext"

    if (si+1<len && name[si+1]!='.' && name[si+1]!='/' && name[si+1]!='\\') {  // this must be an extension?
      pCurrAtom =  pCurrAtom->NewNext(PA_ExtMark, si);
      si++;
      pCurrAtom =  pCurrAtom->NewNext(PA_Extension, si);
      while( si<len && CharOKinFileName(CF_CURR_OS, name[si]))
        si++;
      TCI_ASSERT(si==len);
    } else {
      while ( ch=='.' || ch=='/' || ch=='\\' ) {
        PAType typ =  PA_NoOp;
        U32 inc =  1;
        if (ch=='.') {
          if (si+1 < len && name[si+1] == '.') {
            typ =  PA_Up;
            inc =  2;
          } else {
            typ = PA_Stay;
            inc = 1;
          }
        }
        pCurrAtom =  pCurrAtom->NewNext(typ, si);
        si +=  inc;
        ch =  si<len ? name[si] : 0;
      }
    }
  }
  
  else if ( ch ==':' ) {                  // MAC - ":::nnn:mmm"
    if (si+1 >= len || name[si+1] != ':') {                // ":nnn" is equivalent to ".\nnn
      pCurrAtom =  pCurrAtom->NewNext(PA_Stay, si);
      si++;
    } else {
      pCurrAtom =  pCurrAtom->NewNext(PA_Up, si);         // 1st :: counts as 1 up
      si += 2;
      while ( si < len && name[si] == ':' ) {             // remaining : count as 1 up each
        pCurrAtom =  pCurrAtom->NewNext(PA_Up, si);
        si++;
      }
    }
  }
  
  else if ( ch == '/' && (si+1>=len || name[si+1]!='/') ) {  // UNIX root directory, if not UNC name
    pCurrAtom =  pCurrAtom->NewNext(PA_Root, si);
    si++;
  }
  
  else if ( ch == '\\' || (ch == '/' && name[si+1] == '/') ) {            // DOS root directory or UNC name
    TCI_ASSERT(si == 0);
    if (name[si+1] == '\\' || ch == '/' ) {    // UNC name
      TCICHAR* namebuff = name.GetBuffer(0);
      namebuff[si] = '\\';        
      namebuff[si+1] = '\\';      //force internal storage of the DOS form
      pCurrAtom =  pCurrAtom->NewNext(PA_Device, si);
      si +=  2;
      int cnt =  0;
      while (si < len) {
        if (name[si]=='\\' || name[si]=='/') {
          namebuff[si] = '\\';   //force internal storage of the DOS form
          cnt++;
          if (cnt >= 2)
            break;
        }
     	  si++;
      }
      if ( si<len && (name[si]=='\\'|| name[si]=='/')) {
        pCurrAtom =  pCurrAtom->NewNext(PA_Down, si);
        si++;
      }
    } else {
      pCurrAtom =  pCurrAtom->NewNext(PA_Root, si);
      si++;
    }
  }
  

  else if ( isalpha(ch) && len > 1 && name[1] ==':' ) {   // DOS device - "C:"
                                                            // Warning:  could also be Mac volume
    pCurrAtom =  pCurrAtom->NewNext(PA_Device, si);
    si++;

    TCI_ASSERT( name[si]==':' );
    pCurrAtom =  pCurrAtom->NewNext(PA_NoOp, si);
    si++;

    if ( si<len && (name[si]=='/' || name[si]=='\\') ) {
      pCurrAtom =  pCurrAtom->NewNext(PA_Down, si);
      si++;
    }
  }


  // Common Internet Scheme Syntax:
  //   <scheme>://<user>:<password>@<host>:<port>/<url-path>?<searchpat>
  //
  // Look for  ://  and store the entire token up to /<url-path> 
  // as a PA_URL type.
  else if ((pos = name.Find(g_URLToken)) != -1) { // URL
    pCurrAtom =  pCurrAtom->NewNext(PA_URL, si);

    pos = name.Find('/', pos+3);
    if (pos == -1)
      pos = name.Find('\\', pos+3); // accept DOS style path sep.

    if (pos == -1)
      si = len;
    else {
      si = pos;
      pCurrAtom =  pCurrAtom->NewNext(PA_Down, si);
      ++si;
    }
  }


  // Local Scheme Syntax:
  //   mailto:<stuff> | news:<stuff>
  //
  // Store the entire token up to <stuff> as a PA_URL type.
  else if (name.Find(g_pseudoURL1) != -1 || name.Find(g_pseudoURL2) != -1) { // pseudo-URL
    pCurrAtom =  pCurrAtom->NewNext(PA_URL, si);

    pos = name.Find(':');
    TCI_ASSERT(pos != -1);

    si = pos+1;
  }


  else if ( isalpha(ch) && (name.Find(':') != -1) ) { //  Mac volume names

    pCurrAtom =  pCurrAtom->NewNext(PA_Device, si);
    si +=  name.Find(':');

    TCI_ASSERT(name[si]==':');
    if ( si+1<len && (name[si+1]=='/' || name[si+1]=='\\') ) {   // Oops, this is Mac on DOS
      pCurrAtom =  pCurrAtom->NewNext(PA_NoOp, si);
      si++;

      pCurrAtom =  pCurrAtom->NewNext(PA_Down, si);
      si++;
    } else {                                                    // Std Mac format
      pCurrAtom =  pCurrAtom->NewNext(PA_Down, si);
      si++;
    }
  }


  //
  // check remaining characters
  //
  U32 state =  0, targ_mode;
  if (CHAM_OSMODE == CF_DOS)
    targ_mode = CF_DOS;
  else
    targ_mode = CF_CURR_OS;
  while (si<len && (ch=name[si]) != 0) {
    if ( ch == '?' ) {      // if URL, everything from ? on is arg
      pCurrAtom = pCurrAtom->NewNext(PA_UrlArg, si);
      si = len;
      break;
    }
    else if ( state==0 ) {
      if (ch == '.' && name[si+1] == '.') {
        pCurrAtom =  pCurrAtom->NewNext(PA_Up, si);
        si +=  2;
        TCI_ASSERT(name[si] == '\\' || name[si] == '/');
      } else if (ch == ':') {
        pCurrAtom =  pCurrAtom->NewNext(PA_Up, si);
      } else if ( CharOKinFileName(targ_mode,ch) ) {
        pCurrAtom =  pCurrAtom->NewNext(PA_Name, si);
        state++;
      } else {
        //error, bad character in path name.  Will be handled gracefully by caller!
        TCI_ASSERT( FALSE );
      }
    } else if ( ch==':' || ch=='/' || ch=='\\' ) {
      pCurrAtom =  pCurrAtom->NewNext(PA_Down, si);
      state =  0;
    }
    si++;
  }

  pCurrAtom->type =  PA_End;
  pCurrAtom->off =  si;

  return pPathAtom;
}




static TCI_BOOL CharOKinFileName( U32 targ_mode,char ch ) {

  char* excludes;
  if (targ_mode == CF_CURR_OS)
    excludes  =  " |\"<>+[=];:,*?";
  else // if (targ_mode == CF_DOS)
    excludes  =  "|\"<>:*?";
  char* ptr =  strchr(excludes, ch);
  return  ptr ? FALSE : TRUE;
}


