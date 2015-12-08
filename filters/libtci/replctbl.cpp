
#ifdef TESTING
  #undef THIS_FILE
  static char THIS_FILE[] = __FILE__;
#endif

#include "replctbl.h"

/**************************NameReplaceTable functions**********************/
ReplNameRec::ReplNameRec( const TCIString& oldnm,const TCIString& newnm )
           : newnext(0),oldnext(0),oldname(oldnm),newname(newnm) {
}

NameReplaceTable::NameReplaceTable(){
  for (int i=0;i<8;i++){
    newchains[i] = (ReplNameRec*)NULL;
    oldchains[i] = (ReplNameRec*)NULL;
  }
}

NameReplaceTable::~NameReplaceTable(){
  for (int i=0;i<8;i++){
    ReplNameRec* rec = oldchains[i];
    ReplNameRec* nextrec = (ReplNameRec*)NULL;
    while (rec){
      nextrec = rec->oldnext;
      delete rec;
      rec = nextrec;
    }
  }
}

//note that a return of FALSE from this function simply means name already in table?
TCI_BOOL NameReplaceTable::Add( const TCIString& oldnm,const TCIString& newnm ){
  TCI_BOOL rv = FALSE;
  U16 index = Hash(oldnm);
  TCI_ASSERT( index<8 );
  ReplNameRec* rec = (ReplNameRec*)NULL;
  ReplNameRec* nextrec = oldchains[index];
  ReplNameRec* newrec = (ReplNameRec*)NULL;
  I16 compresult = nextrec ? 
      oldnm.CompareNoCase(nextrec->oldname) : -1;
  while (nextrec && compresult>0){
    rec = nextrec;
    nextrec = rec->oldnext;
    compresult = nextrec ? 
        oldnm.CompareNoCase(nextrec->oldname) : -1;
  }
  if (compresult<0){  //we go after "rec" and before "nextrec"
    newrec = TCI_NEW(ReplNameRec(oldnm,newnm));
    newrec->oldnext = nextrec;
    if (rec)
      rec->oldnext = newrec;
    else {
//      TCI_ASSERT( !chains[index] );
      newrec->oldnext = oldchains[index];
      oldchains[index] = newrec;
    }
    rv = TRUE;   //new record created
  }
  if (rv){    //link into "newchains" list (ordered by newname)
    TCI_ASSERT(newrec);
    index = Hash(newnm);
    rec = (ReplNameRec*)NULL;
    nextrec = newchains[index];
    compresult = nextrec ? 
      newnm.CompareNoCase(nextrec->newname) : -1;
    while (nextrec && compresult>0){
      rec = nextrec;
      nextrec = rec->newnext;
      compresult = nextrec ? 
          newnm.CompareNoCase(nextrec->newname) : -1;
    }
    //Assert not yet in newchains list; we go after "rec" and before "nextrec"
    TCI_ASSERT(compresult<0); 
    newrec->newnext = nextrec;
    if (rec)
      rec->newnext = newrec;
    else {
  //      TCI_ASSERT( !chains[index] );
      newrec->newnext = newchains[index];
      newchains[index] = newrec;
    }
  }
  return rv;
}

TCI_BOOL NameReplaceTable::IsEmpty(){
  TCI_BOOL rv = TRUE;
  for (int i=0;i<8;i++){
    if (oldchains[i]){
      rv = FALSE;
      break;
    }
  }
  return rv;
}

U16 NameReplaceTable::Hash( const TCIString& name ){
  U16 res = 0;
  TCICHAR ch(0);
  int len = name.GetLength();
  for (int i=0;i<len;i++){
    ch = name[i]; 
    if (ch == 0) 
      break;
    res += U16(ch);
  }
  res = res % 8;
  return res;
}

//this is the "look-up" function
TCIString* NameReplaceTable::GetNewName( const TCIString& name ){
  TCIString* rv = (TCIString*)NULL;
  U16 index = Hash(name);
  ReplNameRec* rec = oldchains[index];
  while (rec && !rv){
    if (!rec->oldname.CompareNoCase(name))
      rv = &rec->newname;
    else
      rec = rec->oldnext;
  }
  if (!rv){   //now search newnames
    rec = newchains[index];
    while (rec && !rv){
      if (!rec->newname.CompareNoCase(name))
        rv = &rec->newname;
      else
        rec = rec->newnext;
    }
  }
  return rv;
}


