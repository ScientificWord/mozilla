
#ifndef Grammar_h
#define Grammar_h
																																																								
/*
  Class Grammar  - A server for Parser.
This object holds a .gmr file in a hash table.  All objects that
require grammar productions get them thru a Grammar object.
*/

#include "fltutils.h"

#include <stdlib.h>


typedef struct tagHASH_REC {
  U8*   zname;
  U8    zuID[16];
  U8*   ztemplate;
} HASH_REC;

typedef struct tagHASH_TABLE {
  tagHASH_TABLE*  next;
  U8*             zenv_names;
  U16             slot_count;
  HASH_REC*       records;
} HASH_TABLE;

typedef struct tagLINE_REC {
  tagLINE_REC*  next;
  U8*           zline;
  HASH_TABLE*   table_node;
} LINE_REC;


#define TOK_LEN_LIM     40

typedef struct tagCONTEXT_ID {
  U8  context_nom[ CONTEXT_NOM_LIM ];
  U16 context_usubtype;
  U16 context_uID;
} CONTEXT_ID;


#define NUM_CONTEXTS    20
#define UNKNOWN_CLASS   999


class Grammar {

public:
  Grammar( FILE* grammar_file,TCI_BOOL key_on_uids );
  ~Grammar();

  void          Dump( FILE* df );
  void          GetDefaultContext( U8* dest );
  U8*           GetContextName( U8* zuID );

  TNODE*        GetContextNode( U8* zcontext_nom,TCI_BOOL is_start );

  TCI_BOOL      GetGrammarDataFromUID(  U8* zuID,U8* zcurr_env,
                                            U8** dest_zname,
                                                U8** dest_ztemplate );

  TCI_BOOL      GetGrammarDataFromNameAndAttrs( U8* token,U16 ln,
												U8* zform,
                                            	U8* zcurr_env,
                                                U8**  d_zuID,
                                                U8**  d_ztemplate );

  TCI_BOOL      GetMacroDef( U8* v_nom,U16 v_len,U8** ztemplate );
  U16           GetTokenizerID() { return tokenizerID; }

  I16           GetLitOptionLen( U8* lit_list,U16 lit_list_len,U8* start );
/*
  TCI_BOOL      GetQualifiersDef( U8* zuID,U8** zqual_defs );


  void          AddMacro( U8* nom,U8* def,U16 p_count,U16 scope );
  void          AddNewTheorem( U16 scope,U8* nom,U8* def,
  								U8* n_like,U8* in,TCI_BOOL isstarred );
  void          AddNewEnviron( U16 scope,U8* env_nom,
								U8* n_args,U8* begdef,U8* enddef );
*/

private:

  LINE_REC*    FileToLineList( FILE* grammar_file );
  void         FileLineToHashTables( char* fline,HASH_TABLE* t_node );
  U32          HashzNom( U8* znom );
  U16          HashzuID( U8* zuID );
  TCI_BOOL     EnvOK( U8* zcurr_env,HASH_TABLE* table );
  void         LocateOffsets( char* fline,I16* offsets );

  void         GetContextNom( char* nom,char* dest,
                               U16& the_usubtype,U16& the_uID );
  void         AddContext( char* new_env_nom,char* env_list,
                               U16 the_usubtype,U16 the_uID );
  void         RemoveContextName( char* del_nom,char* env_list );
  HASH_TABLE*  FindTableForEnv( char* env_list );
  TCI_BOOL     IsSubset( char* env_list_subset,char* env_list_superset );

  I16           GetObjID( U16 curr_env,U8* obj_ptr,U16 token_len );

  HASH_TABLE*   MakeNextHNode();

  U16           GetTmplLen( U8* src );

/*
  ATTRIB_REC*   MakeATTRIBList( U8* src,U16 ln );
  ATTRIB_REC*   GetNextATTRIB( U8* src,U16 ln,U16& bytesdone );
  TCI_BOOL 	    AtomicElement( U8* zuID );
  U8*           GetVarValue( U8* tok_ptr,U16 lim,U16& vln );
  U8*           GetTeXMetricuID( U8* ptr,U16 tln,U8** ztmpl );
*/
  TCI_BOOL      IsFormMatch( U8* req_form,U8* obj_def );

  TCI_BOOL      hashed_on_uids;

  HASH_TABLE*   h_tables;
  HASH_TABLE*   last_table_hit;

  U8            default_context[ CONTEXT_NOM_LIM ];

  CONTEXT_ID    contextIDs[NUM_CONTEXTS];

  U16   tokenizerID;
};

#endif

