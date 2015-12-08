#ifndef REPLCTBL_H
#define REPLCTBL_H

#ifndef CHAMDEFS_H
  #include "chamdefs.h"
#endif

#ifndef TCISTRIN_H
  #include "tcistrin.h"
#endif

class ReplNameRec{
  public:
  ReplNameRec( const TCIString& oldnm,const TCIString& newnm );
  ~ReplNameRec() {}
//data
  TCIString oldname;
  TCIString newname;
  ReplNameRec* newnext;
  ReplNameRec* oldnext;
};

class NameReplaceTable{
public:
  NameReplaceTable();
  ~NameReplaceTable();

  TCI_BOOL Add( const TCIString& oldname,const TCIString& newname );
  TCI_BOOL IsEmpty();
  U16 Hash( const TCIString& name );
  TCIString* GetNewName( const TCIString& name );
private:
//data
  ReplNameRec* newchains[8]; 
  ReplNameRec* oldchains[8]; 
};

#endif