#ifndef FILESPEC_H
#define FILESPEC_H

#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif

#ifndef TCISTRIN_H
  #include "tcistrin.h"
#endif

#ifndef STD_TIME_H
  #include <time.h>
  #define STD_TIME_H
#endif


 
#define CF_CURR_OS    0
#define CF_DOS        1
#define CF_UNIX       2
#define CF_MAC        3
#define CF_INTERNAL   4
#define CF_URL        5

#define CHAM_PATHMODE '\\'
#define CHAM_OSMODE   CF_DOS

#define CHAM_PATHSEP '/'

enum PAType {   // "path atom" constants
  PA_NoOp, PA_Root, PA_Device, PA_URL, PA_Up, PA_Down, PA_Stay, PA_Name, PA_ExtMark,
  PA_Extension, PA_UrlArg, PA_End
};

class PathAtom {
public:
  PathAtom() : type(PA_NoOp), off(0), next(NULL) {}
  PathAtom(const PathAtom& p) : off(p.off), type(p.type), next(NULL) {}

  PathAtom* NewNext(PAType typ, U32 n);

  PAType type;
  U32 off;
  PathAtom* next;
};



class FileSpec {
public:
  FileSpec() : atoms(0), baseAtoms(NULL) {}
  FileSpec(const TCIString& fname);
  FileSpec(const FileSpec& fspec);
  FileSpec(const FileSpec& path, const FileSpec& name);
  virtual ~FileSpec();

  const FileSpec& operator=(const FileSpec& fs);
  const FileSpec& operator=(const TCIString& s);
  const FileSpec& operator+=(const FileSpec& fs)
	  { MakePath(); return Concat( fs ); }
  const FileSpec& operator+=(const TCIString& s)
    { MakePath(); return Concat( FileSpec(s) ); }
	TCI_BOOL operator==(const FileSpec& fs) const
	  { return ComparePath( fs ) == 0; }
	TCI_BOOL operator!=(const FileSpec& fs) const
	  { return ComparePath( fs ) != 0; }
  operator const TCICHAR*() const
    { return name; }

/////////////////////////  Path Manipulation  /////////////////////////
  TCI_BOOL  IsValid() const { return !name.IsEmpty(); }
  TCI_BOOL  IsEmpty() const { return name.IsEmpty(); }
	
  void      Empty();

  FileSpec  FullPath() const;
	FileSpec  GetName(TCI_BOOL noextension=FALSE) const;
	
	const TCIString&  GetFullPath() const { return name; }
	FileSpec  GetDir() const;

  // Convert to/from 8.3 DOS names.
  FileSpec  SWGetShortPathName() const;
  FileSpec  SWGetLongPathName()  const;

  // Returns the directory depth for absolute paths.  For relative paths,
  // an absolute path is first constructed from the base path and then the
  // directory depth is determined.  Files that have no path or are relative
  // without a base, a FALSE is returned.
  TCI_BOOL  GetDirDepth(I32& depth) const;

  // Returns the directory that is "upLevels" up from the current file path.
  FileSpec  GetParentDir(I32 upLevels) const;

  // Returns TRUE if file path is valid and relative.
  TCI_BOOL  IsRelative() const;

  // Returns TRUE if file path is valid and absolute.
  TCI_BOOL  IsAbsolute() const;

  // The base path is used to convert relative paths to absolute
  // (and vice versa).  This procedure accepts only absolute paths.
  // It will return FALSE if the newBasePath is not valid or absolute.
  TCI_BOOL  SetBasePath(const FileSpec& newBasePath);

  // Returns the current base path.
  FileSpec  GetBasePath() const;

  // Returns TRUE if the file path is in or below the base path directory.
  TCI_BOOL  IsBelowBasePath() const;

  // Make file path absolute by concatenating the relative
  // file path onto base path.
  TCI_BOOL  MakeAbsolute();

  // Make file path relative to base path.
  TCI_BOOL  MakeRelative();

  // Returns TRUE if file path is valid and an URL
  TCI_BOOL  IsURL() const;

  TCI_BOOL  MakePath();
  TCI_BOOL  UnmakePath();
  TCI_BOOL  IsPath() const;

  TCI_BOOL  SetDriveSameAs(const FileSpec& fs);
  void      SetExtension(const FileSpec&);
  void      SetExtension(const TCIString&);

  FileSpec  GetExtension() const;
  FileSpec  GetDrive() const;
  
  TCIString ConvertPath(U32 targ_mode) const;
  I32       ComparePath(const FileSpec&) const;

  TCI_BOOL  IsUNCName() const;
  
/////////////////////////  Miscellaneous  /////////////////////////
  TCI_BOOL  AbbrevName(TCIString& dest, I32 maxlen, TCI_BOOL bAtLeastName) const;

protected:
  void        AtomToName();
  const FileSpec& FileSpec::Concat(const FileSpec& fs);
	
private:
  // Helper function that returns a copy of atoms
  PathAtom* CopyPathAtoms(PathAtom* src);

  // Helper function for comparing atoms
  TCI_BOOL  EqualAtom(PathAtom* p1, const TCIString* n1,
                      PathAtom* p2, const TCIString* n2) const;

  FileSpec  Convert(U32 targ_mode, I32 num_atoms=0) const;
  PathAtom* ConvertName();
	I32       GetDirLength() const;
  void      DelAtoms(PathAtom*);
  TCI_BOOL  Unique() const;
  void      LocateNameAndExtension(I32& nameloc,I32& namelen,I32& extloc,I32& extlen) const;

  // file path
  PathAtom* atoms;
  TCIString name;

  // base path (for relative paths)
  PathAtom* baseAtoms;
  TCIString basePath;

  friend class ChamFile;
};

#endif //ndef FileSpec_h
