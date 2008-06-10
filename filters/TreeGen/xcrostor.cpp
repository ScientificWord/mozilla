
#include "mcrostor.h"
#include "fltutils.h"

#include "tci_new.h"
#include <string.h>
#include <stdlib.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MacroStore::MacroStore() {
//JBMLine( "MacroStore::ctor\n" );

  macros  =  (MACRO_REC*)NULL;
  mathops =  (MATHOP_REC*)NULL;
  scope_handle  =  0;
}


MacroStore::~MacroStore() {

  ClearScope( 0 );
}


U8* MacroStore::GetMacroDef( U8* token,U16& n_params ) {

  U8* rv  =  NULL;

  MACRO_REC*  rover =  macros;
  while ( rover ) {
    if ( !strcmp((char*)token,(char*)rover->name) ) {
      rv  =  rover->def;
      n_params  =  rover->param_count;
      break;
    } else
      rover =  rover->next;
  }

  return rv;
}


U8* MacroStore::GetMathOpDef( U8* token,bool& is_starred ) {

  U8* rv  =  NULL;

  MATHOP_REC*  rover =  mathops;
  while ( rover ) {
    if ( !strcmp((char*)token,(char*)rover->name) ) {
      rv  =  rover->def;
      is_starred  =  rover->is_starred;
      break;
    } else
      rover =  rover->next;
  }

  return rv;
}


U16 MacroStore::GetNewScope() {

  scope_handle++;
  return scope_handle;
}


void MacroStore::ClearScope( U16 scope ) {

  if ( scope == 0 ) {
    MACRO_REC*  del;
    MACRO_REC*  rover =  macros;
    while ( rover ) {
      del   =  rover;
      rover =  rover->next;
      if ( del->name ) delete del->name;
      if ( del->def )  delete del->def;
      delete del;
    }
    macros  =  NULL;

    MATHOP_REC*  mo_del;
    MATHOP_REC*  mo_rover =  mathops;
    while ( mo_rover ) {
      mo_del   =  mo_rover;
      mo_rover =  mo_rover->next;
      if ( mo_del->name ) delete mo_del->name;
      if ( mo_del->def )  delete mo_del->def;
      delete mo_del;
    }
    mathops =  NULL;

  } else {
TCI_ASSERT( 0 );
  }
}


void MacroStore::AddMacro( U8* nom,U8* def,U16 p_count,U16 scope ) {

  MACRO_REC* n_macro  =  (MACRO_REC*)TCI_NEW(char[sizeof(MACRO_REC)] );

  U16 nom_len =  strlen( (char*)nom );
  U8* tmp =  (U8*)TCI_NEW(char[ nom_len+1 ] );
  strcpy( (char*)tmp,(char*)nom );
  n_macro->name =  tmp;

  U16 def_len =  strlen( (char*)def );
  tmp =  (U8*)TCI_NEW(char[ def_len+1 ] );
  strcpy( (char*)tmp,(char*)def );
  n_macro->def  =  tmp;

  n_macro->param_count  =  p_count;
  n_macro->scope  =  scope;

  // Add at head of list

  n_macro->next =  macros;
  macros  =  n_macro;
  
}


void MacroStore::AddMathOp( U8* nom,U8* def,bool starred,U16 scope ) {

  MATHOP_REC* n_mathop  =  (MATHOP_REC*)TCI_NEW(char[sizeof(MATHOP_REC)] );

  U16 nom_len =  strlen( (char*)nom );
  U8* tmp =  (U8*)TCI_NEW(char[ nom_len+1 ] );
  strcpy( (char*)tmp,(char*)nom );
  n_mathop->name  =  tmp;

  U16 def_len =  strlen( (char*)def );
  tmp =  (U8*)TCI_NEW(char[ def_len+1 ] );
  strcpy( (char*)tmp,(char*)def );
  n_mathop->def =  tmp;

  n_mathop->is_starred  =  starred;
  n_mathop->scope =  scope;

  // Add at head of list

  n_mathop->next =  mathops;
  mathops =  n_mathop;
}

