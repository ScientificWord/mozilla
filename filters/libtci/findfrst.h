#ifndef FINDFRST_H
#define FINDFRST_H


#ifndef CHMTYPES_H
  #include "chmtypes.h"
#endif


//  Find First -- File operations
//

class TCIString;

class FindFiles {
public:
  FindFiles();
  ~FindFiles();
  //* FindFirst() and FindNext() are a simple implementation  
  //* of the findfirst() and findnext() functions available for
  //* Borland C++ and MS Visual C++ compilers.
  TCI_BOOL  FindFirst(const TCIString& path, TCIString& filename);
  TCI_BOOL  FindNext(TCIString& filename);
  I32 GetFileTime(const TCIString& filename);
  void  EndFind(void);
private:
  void* m_findData;
  void* m_findHandle;

};

#endif
