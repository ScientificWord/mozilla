
/* Given the name of a .gmr file, the ctor must load its contents
    into a set of internal stores.  Public functions on a Grammar
    object dispense this data to clients - Parsers and Translaters.

  Note that Grammars DO NOT hold any environment/context/state info about
    a particular parse or translation.  These issues are the domain of
    the calling Parser or Translater.

  In this implementation, a Grammar extracts the id of the "tokenizer"
    that it is to use from the .gmr file used in its creation.

*/
  
#include "grammar.h"
#include "fltutils.h"

#include "tci_new.h"
#include <string.h>
#include <stdlib.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Each hash table has a list of env names - a catenation of zstrs.
//  We check that the table identified by "tableID" is valid for "zcurr_env".

TCI_BOOL Grammar::EnvOK( U8* zcurr_env,HASH_TABLE* table ) {

  TCI_BOOL rv  =  FALSE;

  char* env_nom =  (char*)table->zenv_names;

  if ( env_nom ) {


    if ( !strcmp((char*)zcurr_env,"GENERIC") ) {
      rv  =  TRUE;

    } else {

      while ( *env_nom ) {
        if ( !strcmp(env_nom,(char*)zcurr_env) ) {    // found
          rv  =  TRUE;
          break;
        } else
          env_nom +=  strlen(env_nom) + 1;
      }
    }

  } else {

//JBMLine( "Table slot has NO NAMES\n" );

  }

  return rv;
}


// fline = "\blah<uID2.10.43>remaining bytes are the template
//               ^   ^       ^
//               0   1       2

void Grammar::LocateOffsets( char* fline,I16* offsets ) {

  char* ptr =  strstr( fline,"<uID" );
  if ( ptr ) {

    offsets[0]  =  ptr - fline;
    offsets[1]  =  offsets[0] + 4;

    char* p2    =  ptr;
    while ( *p2 && *p2!='>' ) p2++;

    TCI_ASSERT( *p2 == '>' );
    offsets[2]  =  p2 - fline + 1;

  } else {
    TCI_ASSERT( 0 );
  }
}



// <START_CONTEXT_TEXT,1.1.0>
//                ^

void Grammar::GetContextNom( char* nom,char* dest,
                                U16& the_usubtype,U16& the_uID ) {

  while ( *nom != '>' && *nom != ',' ) {
    *dest   =  *nom;
    dest++;
    nom++;
  }
  *dest =  0;

  if ( *nom == ',' ) {
    nom =  strchr( nom,'.' ); 
    nom++;
    the_usubtype  =  atoi( nom );
    nom =  strchr( nom,'.' ); 
    nom++;
    the_uID       =  atoi( nom );
  }
}

// Utility to put info that identifies a context encountered during
//   the read of .gmr file into contextIDs array.

void Grammar::AddContext( char* new_env_nom,char* env_list,
                                U16 the_usubtype,U16 the_uID ) {

  while ( *env_list ) {
    if ( !strcmp(env_list,new_env_nom) ) return;
    env_list +=  strlen(env_list) + 1;
  }
  strcpy( env_list,new_env_nom );
  env_list +=  strlen(new_env_nom) + 1;
  *env_list =  0;


  U16 slot  =  0;
  while ( contextIDs[slot].context_uID || contextIDs[slot].context_usubtype )
    slot++;
  TCI_ASSERT( slot < NUM_CONTEXTS );

  strcpy( (char*)contextIDs[slot].context_nom,new_env_nom );
  contextIDs[slot].context_usubtype =  the_usubtype;
  contextIDs[slot].context_uID      =  the_uID;

/*
char zzz[80];
sprintf( zzz,"Added context=%s,%d.%d, slot=%d\n",
                        new_env_nom,the_usubtype,the_uID,slot );
JBMLine( zzz );
*/
}


void Grammar::RemoveContextName( char* del_nom,char* env_list ) {

  char* delp  =  (char*)NULL;
  char* endp  =  env_list;

  while ( *endp ) {
    if ( !strcmp(endp,del_nom) ) delp =  endp;
    endp  +=  strlen(endp) + 1;
  }

  if ( delp ) {
    char* rover =  delp + strlen(delp) + 1;
    while ( rover <= endp ) {
      *delp =  *rover;
      delp++;
      rover++;
    }
  }

}


HASH_TABLE* Grammar::FindTableForEnv( char* env_list ) {

  if ( *env_list == 0 )
    return h_tables->next;

  HASH_TABLE* rv  =  NULL;

  HASH_TABLE* rover =  h_tables;
  while ( rover ) {
    char* env_names =  (char*)rover->zenv_names;
    if ( env_names ) {
      if ( IsSubset(env_names,env_list) && IsSubset(env_list,env_names) ) {
        rv  =  rover;
        break;
      }
    }
    rover =  rover->next;
  }

  if ( !rv ) {

    char* tp  =  env_list;
    while ( *tp ) tp  +=  strlen(tp) + 1;

    U16 zln =  tp - env_list + 1;
    char* envs  =  TCI_NEW(char[zln] );
    memcpy( envs,env_list,zln );

/*
char zzz[80];
JBMLine( "Assigning envs\n" );
char* sp = envs;
while ( *sp ) {
sprintf( zzz,"%s\n",sp );
JBMLine( zzz );
sp  +=  strlen(sp) + 1;
}
*/

    rv  =  MakeNextHNode();
    rv->zenv_names =  (U8*)envs;
  }

  return rv;
}


// env_list_subset  =  "math0text00"
// env_list_superset=  "junk0text0math00"

TCI_BOOL Grammar::IsSubset( char* env_list_subset,char* env_list_superset ) {

  while ( *env_list_subset ) {
    char* test_names  =  env_list_superset;

    while ( *test_names ) {
      if ( !strcmp(env_list_subset,test_names) )
        break;
      else
        test_names  +=  strlen(test_names) + 1;
    }
    if ( *test_names )          // found
      env_list_subset +=  strlen(env_list_subset) + 1;
    else
      break;
  }

  return (*env_list_subset) ? FALSE : TRUE;
}



I16 Grammar::GetObjID( U16 curr_env,U8* obj_ptr,U16 token_len ) {

  I16 rv  =  -1;


  return rv;
}



// lit_list = !blah|blah blah|!
//             ^    ^         ^
//                start
// returns  0 if the "empty" literal is in the list
// returns -1 if there are no more literals in the list
// caller advances rover by the returned value + 1 between calls

I16 Grammar::GetLitOptionLen( U8* lit_list,U16 lit_list_len,U8* start ) {

  I16 rv;

  U8* end_ptr =  lit_list + lit_list_len;
  U8 end_char =  *lit_list;

  if ( start >= end_ptr ) {
    rv  =  -1;                          // end of list

  } else if ( *start == '|' || *start == end_char ) {
    rv  =  0;                           // empty literal

  } else {

    char stop_chars[4];
    stop_chars[0] =  end_char;
    strcpy( stop_chars+1,"|\\" );

    U8* ptr   =  start;
    do {
      U16 off =  strcspn( (char*)ptr,stop_chars );
      ptr +=  off;

      if      ( *ptr == '\\' )
        ptr   +=  2;
      else if ( *ptr == '|' || *ptr == end_char ) {
        rv =  ptr - start;
        break;
      } else {
        TCI_ASSERT( 0 );
        break;
      }
  
    } while ( ptr < end_ptr );

  }

  return rv;
}



HASH_TABLE* Grammar::MakeNextHNode() {

  HASH_TABLE* rv  =  (HASH_TABLE*)TCI_NEW(char[sizeof(HASH_TABLE)] );

  rv->next        =  (HASH_TABLE*)NULL;
  rv->zenv_names  =  (U8*)NULL;       // context names
  rv->slot_count  =  0;               // number of records in array
  rv->records     =  (HASH_REC*)NULL; // pointer to array of records

  // add this node to the "h_tables" list

  if ( h_tables ) {
    HASH_TABLE* rover =  h_tables;
    while ( rover->next ) rover =  rover->next;
    rover->next =  rv;
  } else
    h_tables  =  rv;

  return rv;
}


//zzzzzz

#define TOKENIZER_TEX     1
#define LINELIMIT         32767

/* A "Grammar" stores a collection of definitions of grammar elements.
    It implements functions that look up these definitions.  For Parsing,
    the lookup is hashed on the literal that identifies an instance of
    the grammar element.  For Translating, the elements are hashed on the
    uID of the element - in this case the client is going from a uID
    back to an element in a grammar.
*/

Grammar::Grammar( FILE* grammar_file,TCI_BOOL key_on_uids ) {
//JBMLine( "Grammar::ctor\n" );

  hashed_on_uids  =  key_on_uids;

  // Clear the list of hash tables for named grammar objects.
  //   Every set of names has its own hash table, and a list of "context"
  //   names in which the objects are allowed.
  //   Examples of contexts are "text" and "math" and the arguments of
  //   some special commands like format, which takes \l \c \r tokens only.

  h_tables  =  (HASH_TABLE*)NULL;       // list of "hash table" nodes

  // the first node is reserved for "_DEFS_"

  MakeNextHNode();

  // the second node is reserved for the un-named context

  MakeNextHNode();

  // For grammars that have contexts, there may be a "default context".
  //  "text" is the name of the default context in LaTeX.

  default_context[0]  =  0;

  // Store for the names and uIDs of the contexts in this grammar

  for ( U16 slot=0; slot<NUM_CONTEXTS; slot++ ) {   // text,math,\format,etc
    contextIDs[slot].context_nom[0]   =  0;
    contextIDs[slot].context_usubtype =  0;
    contextIDs[slot].context_uID      =  0;
  }

  // The following call reads in the grammar file.  As lines are read
  //   they are examined for environment switches.  Nodes are created
  //   in "h_tables" for sets of names encountered and are filled out
  //   re "zenv_names" and "slot_count"s.

  LINE_REC* line_list =  FileToLineList( grammar_file );

  // Allocate arrays of hash slots for each "hash table" node.
  // The "slot_count" field gives the number of items to be placed
  //   in each hash table.

  HASH_TABLE* rover =  h_tables;
  while ( rover ) {
    if ( rover->slot_count ) {
      //U16 n_slots =  ( 4 * rover->slot_count ) / 3;
      U16 n_slots =  ( 3 * rover->slot_count ) / 2;
      n_slots++;                // assure at least 1 slot remains empty

      HASH_REC* h_array =  (HASH_REC*)TCI_NEW(char[n_slots * sizeof(HASH_REC)] );
      for ( U16 si=0; si<n_slots; si++ ) {
        h_array[si].zname     =  (U8*)NULL;
//      h_array[si].zuID[16];
        h_array[si].ztemplate =  (U8*)NULL;
      }
      rover->records    =  h_array;
      rover->slot_count =  n_slots;
    }
    rover =  rover->next;
  }

  // Move data from "line_list" to the hash tables

  while ( line_list ) {

    FileLineToHashTables( (char*)line_list->zline,line_list->table_node );

    LINE_REC* del =  line_list;
    line_list =  line_list->next;
    delete del->zline;
    delete del;
  }

  last_table_hit  =  (HASH_TABLE*)NULL;   // last hash table in which an object was located

// Use the following to dump this grammar

  //Dump( (FILE*)NULL  );   // this goes to "\jbm"
}


Grammar::~Grammar() {
//JBMLine( "Grammar_dtor\n" );

  //if ( macro_store )
    //delete macro_store;

  HASH_TABLE* del;
  
  while ( h_tables ) {
    if ( h_tables->zenv_names )
      delete h_tables->zenv_names;

    U16 lim_slot  =  h_tables->slot_count;
    if ( lim_slot ) {
      HASH_REC* slot_array =  h_tables->records;
      U16 slot  =  0;
      while ( slot < lim_slot ) {
        if ( slot_array[slot].zname )     delete slot_array[slot].zname;
        if ( slot_array[slot].ztemplate ) delete slot_array[slot].ztemplate;
        slot++;
      }
      delete slot_array;
    }

    del =  h_tables;
    h_tables  =  h_tables->next;
    delete del;
  }             // while loop thru list of "hash table" nodes

}


void Grammar::Dump( FILE* df ) {

  char zzz[256];

  sprintf( zzz,"\nDumping Grammar hash tables:\n" );
  if ( df ) fputs( zzz,df );
  else      JBMLine( zzz );

  U16 tableID =  0;
  HASH_TABLE* curr_table  =  h_tables;
  while ( curr_table ) {

    U16 lim_slot  =  curr_table->slot_count;

    sprintf( zzz,"\nTable id = %d,Item count = %d\n",tableID,lim_slot );
    if ( df ) fputs( zzz,df );
    else      JBMLine( zzz );

    if ( lim_slot ) {
      if ( curr_table->zenv_names ) {
        sprintf( zzz,"Contexts(s):\n" );
        if ( df ) fputs( zzz,df );
        else      JBMLine( zzz );
        char* envs  =  (char*)curr_table->zenv_names;
        while ( *envs ) {
          sprintf( zzz,"Context=%s\n",envs );
          if ( df ) fputs( zzz,df );
          else      JBMLine( zzz );
          envs  +=  strlen(envs) + 1;
        }
      } else {
        sprintf( zzz,"Context: Default\n" );
        if ( df ) fputs( zzz,df );
        else      JBMLine( zzz );
      }

      HASH_REC* rover =  curr_table->records;
      U16 slot  =  0;
      while ( slot < lim_slot ) {
        if ( rover[slot].zname ) {
		  U16 hash  =  HashzNom( rover[slot].zname ) % lim_slot;
          sprintf( zzz,"%d(%d),%s,%s,%s\n",slot,hash,
                  rover[slot].zname,rover[slot].zuID,rover[slot].ztemplate );
        } else
          sprintf( zzz,"0\n" );

        if ( df ) fputs( zzz,df );
        else      JBMLine( zzz );
        slot++;
      }
    }           //  if ( lim_slot )

    tableID++;
    curr_table  =  curr_table->next;
  }     // while loop

}


void Grammar::GetDefaultContext( U8* dest ) {

  strcpy( (char*)dest,(char*)default_context );
}


// The zuID param is something like "1.2.xx"
//  The class ID, 1, identifies a "context start" object.
//  The subclass ID, 2, identfies the context to which we are switching.

U8* Grammar::GetContextName( U8* zuID ) {

U16 classID =  atoi( (char*)zuID );
TCI_ASSERT( classID == 1 || classID == 2 );

  char* zp  =  strchr( (char*)zuID,'.' );
  U16 contextID =  atoi( zp+1 );

  U8* rv  =  (U8*)NULL;

  for ( U16 slot=0; slot<NUM_CONTEXTS; slot++ ) {
    if ( contextIDs[slot].context_usubtype == contextID ) {
      rv =  (U8*)&contextIDs[slot].context_nom;
      break;
    }
  }

  return rv;
}


TNODE* Grammar::GetContextNode( U8* zcontext_nom,TCI_BOOL is_start ) {

  U16 usubtype  =  0;
  U16 uID       =  0;

  for ( U16 slot=0; slot<NUM_CONTEXTS; slot++ ) {
    if ( !strcmp( (char*)contextIDs[slot].context_nom,(char*)zcontext_nom ) ) {
      usubtype  =  contextIDs[slot].context_usubtype;
      uID       =  contextIDs[slot].context_uID;
      break;
    }
  }

  TNODE* rv =  (TNODE*)NULL;
  if ( usubtype || uID ) {
    U16 objtyp  =  is_start ? 1 : 2;

    U8 zuID[ ZID_STR_LIM ];
    UidsTozuID( objtyp,usubtype,uID,zuID );

    rv  =  MakeTNode( 0,0,0,zuID );

/*
char zzz[80];
sprintf( zzz,"Grammar::GetContextNode, nom=%s, subtype=%d, uID=%d\n",
                zcontext_nom,usubtype,uID );
JBMLine( zzz );
*/

  } else {
/*
char zzz[80];
sprintf( zzz,"Grammar::GetContextNode failed, nom=%s\n",zcontext_nom );
JBMLine( zzz );
*/
  }
  return rv;
}


// Given a uID, look up it's definition in the destination grammar

TCI_BOOL Grammar::GetGrammarDataFromUID( U8* zuID,U8* zcurr_env,
                                          U8** dest_zname,
                                          U8** dest_ztemplate ) {
/*
JBMLine( "GetGrammarDataFromUID::\n" );
char zzz[80];
sprintf( zzz,"  zuID=%s***env=%s\n",zuID,zcurr_env );
JBMLine( zzz );
*/

  TCI_BOOL rv  =  FALSE;
  if ( !hashed_on_uids ) {
    TCI_ASSERT( 0 );
    return rv;
  }

  HASH_TABLE* start_table =  last_table_hit ? last_table_hit : h_tables->next;
  HASH_TABLE* curr_table  =  start_table;

  U16 hash    =  HashzuID( zuID );


  do {                  // loop thru the hash tables

    if ( EnvOK(zcurr_env,curr_table) ) {

      U16 slot_limit =  curr_table->slot_count;
      if ( slot_limit ) {
        U16 curr_slot =  hash % slot_limit;
        HASH_REC* h_array =  curr_table->records;

        do {              // loop down this array

          if ( !h_array[curr_slot].zname ) break;

          U8* zcurr_uID =  h_array[curr_slot].zuID;

          //if ( *zcurr_uID == 0 ) break;

          if ( strcmp( (char*)zuID,(char*)zcurr_uID ) ) {
            curr_slot++;
            if ( curr_slot == slot_limit ) curr_slot =  0;
          } else {
            *dest_zname     =  h_array[curr_slot].zname;
            *dest_ztemplate =  h_array[curr_slot].ztemplate;
            last_table_hit  =  curr_table;
            rv  =  TRUE;
          }
        } while ( !rv );

      }           // table in use

    }           // if ( EnvOK(tableID) )

    curr_table  =  curr_table->next;
    if ( !curr_table )  curr_table  =  h_tables->next;
    
  } while ( !rv && curr_table != start_table );

/*
if ( !rv ) {
char zzz[80];
sprintf( zzz,"FromuID FAILS, %s\n", zuID );
JBMLine( zzz );
}
*/

  return  rv;
}


TCI_BOOL Grammar::GetGrammarDataFromNameAndAttrs( U8* token,U16 ln,
												  U8* zform,
                                                  U8* zcurr_env,
                                                  U8** d_zuID,
                                                  U8** d_ztemplate ) {

  TCI_BOOL rv  =  FALSE;
  if ( hashed_on_uids ) {
    TCI_ASSERT( 0 );
    return rv;
  }

  // Make a local copy of the token we're looking for
/*
char zzz[512];
sprintf( zzz,"Grammar token=%s, ln=%d\n",token,ln );
JBMLine( zzz );
*/
  char ztoken[ TOK_LEN_LIM ];
//  TCI_ASSERT( ln+1 < TOK_LEN_LIM );
  if ( ln+1 < TOK_LEN_LIM ) {
    strncpy( ztoken,(char*)token,ln );
    ztoken[ln]  =  0;
  } else
    return rv;

  HASH_TABLE* start_table =  last_table_hit ? last_table_hit : h_tables->next;
  HASH_TABLE* curr_table  =  start_table;

  U32 hash    =  HashzNom( (U8*)ztoken );

  do {                  // loop thru the hash tables
/*
char zzz[80];
sprintf( zzz,"curr_env=%s, token=%s\n",zcurr_env,ztoken );
JBMLine( zzz );
*/
    if ( EnvOK(zcurr_env,curr_table) ) {

//JBMLine( "EnvOK\n" );

      U16 slot_limit =  curr_table->slot_count;
      if ( slot_limit ) {

//JBMLine( "Searching an occupied table\n" );

        U16 curr_slot =  hash % slot_limit;
        HASH_REC* h_array =  curr_table->records;

        do {              // loop down this array

          char* zcurr_nom =  (char*)h_array[curr_slot].zname;

          if ( !zcurr_nom ) {	// lookup failed, empty table entry
//JBMLine( "Failed\n" );
            break;
          }
/*
sprintf( zzz,"n1=%s,n2=%s\n",ztoken,zcurr_nom );
JBMLine( zzz );
*/
          if ( strcmp(ztoken,zcurr_nom) ) {
            curr_slot++;
            if ( curr_slot == slot_limit ) curr_slot =  0;

          } else {		// name match found

		// In MathML, there may be several definitions for one entity or symbol.
		//  ie '+' has 3 entries with differing form attributes (infix, prefix
		//  and postfix).
		// The caller of GetGrammarDataFromNameAndAttrs must set the zform
		//  param in order to get the required entry.
		//    ie zform  =  "prefix";
		//

			TCI_BOOL found =  FALSE;
			if ( zform ) {
              U8* obj_def =  h_array[curr_slot].ztemplate;
			  if ( obj_def ) {
                found =  IsFormMatch( zform,obj_def );
  			  }
			} else
			  found =  TRUE;

			if ( found ) {
              *d_zuID         =  h_array[curr_slot].zuID;
              *d_ztemplate    =  h_array[curr_slot].ztemplate;
              last_table_hit  =  curr_table;
              rv  =  TRUE;
			} else {
              curr_slot++;
              if ( curr_slot == slot_limit ) curr_slot =  0;
			}
          }
        } while ( !rv );

      }           // table in use

    }           // if ( EnvOK(zcurr_env,curr_table) )

    curr_table  =  curr_table->next;
    if ( !curr_table )  curr_table  =  h_tables->next;
    
  } while ( !rv && curr_table != start_table );


  return  rv;
}


TCI_BOOL Grammar::GetMacroDef( U8* v_nom,U16 v_len,U8** ztemplate ) {

  TCI_BOOL rv  =  FALSE;

  // Make a local copy of the token we're looking for

  char ztoken[ TOK_LEN_LIM ];
  TCI_ASSERT( v_len+1 < TOK_LEN_LIM );
  strncpy( ztoken,(char*)v_nom,v_len );
  ztoken[v_len] =  0;

  // Macros definitions are kept in the first node in "h_tables"

  U32 hash    =  HashzNom( (U8*)ztoken );

  U16 slot_limit  =  h_tables->slot_count;
  if ( slot_limit ) {
    U16 curr_slot =  hash % slot_limit;
    HASH_REC* h_array =  h_tables->records;

    do {              // loop down this array

      char* zcurr_nom =  (char*)h_array[curr_slot].zname;

      if ( !zcurr_nom ) break;
/*
sprintf( zzz,"n1=%s,n2=%s\n",ztoken,zcurr_nom );
JBMLine( zzz );
*/
      if ( strcmp(ztoken,zcurr_nom) ) {
        curr_slot++;
        if ( curr_slot == slot_limit ) curr_slot =  0;
      } else {
        *ztemplate  =  h_array[curr_slot].ztemplate;
        rv  =  TRUE;
      }
    } while ( !rv );

  }           // table in use

  return  rv;
}


// private functions


// The following call reads in the grammar file.  As lines are read
//   they are examined for context switches.  "h_tables" is filled
//   re "zenv_names" and "slot_count"s.


LINE_REC* Grammar::FileToLineList( FILE* grammar_file ) {

  char* t_ptr;

  LINE_REC* head  =  (LINE_REC*)NULL;
  LINE_REC* tail;
  LINE_REC* new_node;

  HASH_TABLE* curr_table  =  NULL;  // this identifies the table for each line

  char env_list[256];
  env_list[0] =  0;

  U16 env_usubtype;
  U16 env_uID;

  char fline[256];
  while ( fgets(fline,256,grammar_file) ) {   // loop thru lines of grammar

    U16 ln  =  strlen( fline );               // remove trailing white chars
    while ( ln ) {
      if ( fline[ln-1] <= ' ' ) {
        ln--;
        fline[ln] =  0;
      } else
        break;
    }

    if ( ln ) {          // process line

      if ( fline[0] == ';' 
      && ( fline[1] != '<' || fline[2] != 'u') ) { 

       // comment line

      } else if ( t_ptr = strstr(fline,"<TOKENIZER_TEX>") ) {
        tokenizerID =  TOKENIZER_TEX;

      } else if ( t_ptr = strstr(fline,"<TOKENIZER_MML>") ) {
        tokenizerID =  TOKENIZER_MML;
		//TCI_ASSERT(0);

      } else if ( t_ptr = strstr(fline,"<DEFAULT_CONTEXT_") ) {

        GetContextNom( t_ptr+17,(char*)default_context,env_usubtype,env_uID ); 

      } else if ( t_ptr = strstr(fline,"<START_CONTEXT_") ) {

        char new_env_nom[ CONTEXT_NOM_LIM ];
        GetContextNom( t_ptr+15,new_env_nom,env_usubtype,env_uID ); 
        AddContext( new_env_nom,env_list,env_usubtype,env_uID );
        curr_table  =  FindTableForEnv( env_list );

      } else if ( t_ptr = strstr(fline,"<END_CONTEXT_") ) {

        char new_env_nom[ CONTEXT_NOM_LIM ];
        GetContextNom( t_ptr+13,new_env_nom,env_usubtype,env_uID ); 
        RemoveContextName( new_env_nom,env_list );
        curr_table  =  FindTableForEnv( env_list );

      } else {          // put this line on the list

        new_node  =  (LINE_REC*)TCI_NEW(char[sizeof(LINE_REC)] );
        new_node->next  =  (LINE_REC*)NULL;
        new_node->zline =  (U8*)TCI_NEW(char[ ln+1 ] );
        strcpy( (char*)new_node->zline,fline );

/*
JBMLine( (char*)new_node->zline );
JBMLine( "\n" );
*/
        TCI_BOOL is_macro  =  FALSE;
        if ( fline[0] == '_' )
          if ( !strstr(fline,"<uID") )
            is_macro  =  TRUE;

        if ( is_macro ) {
          new_node->table_node    =  h_tables;
          h_tables->slot_count    =  h_tables->slot_count + 1;
        } else {
          new_node->table_node    =  curr_table;
          curr_table->slot_count  =  curr_table->slot_count + 1;
        }

        if ( !head ) head =  new_node;
        else tail->next   =  new_node;
        tail  =  new_node;
      }

    }                   // process line

  }             // loop thru file lines

  return head;
}


// The following function uses the class variable "hashed_on_uids"
//  to decide which field keys each record.
// fline =  "\alpha<uID3.1.1>"
// fline =  "_BLAH_etc."

void Grammar::FileLineToHashTables( char* fline,HASH_TABLE* t_node ) {

  // Locate the fields in this line

  I16 offsets[4];
  if ( t_node == h_tables ) {           // first table reserved for "_DEFS_"
    char* p =  strchr( fline+1,'_' );
    offsets[0]  =  offsets[2] =  p - fline + 1;
  } else
    LocateOffsets( fline,offsets );


  // fline = "\blah<uID2.10.43>remaining bytes are the template
  //               ^   ^       ^
  //               0   1       2
  // fline = "_blah_remaining bytes are the template
  //                ^
  //                0
  //                2

  // Set up the name of this syntax element in a heap zstring

  U16 zln   =  offsets[0];
  U8* znom  =  (U8*)TCI_NEW(char[zln+1] );
  strncpy( (char*)znom,fline,zln );
  znom[zln] =  0;

  // Find a slot for the element in the hash table

  HASH_REC* h_array  =  t_node->records;
  U16 slot_lim  =  t_node->slot_count;

/*
char zzz[80];
sprintf( zzz,"slot_lim = %d\n",slot_lim );
JBMLine( zzz );
*/

  U16 slot;
  if ( hashed_on_uids && t_node != h_tables ) {
    U16 hash  =  HashzuID( (U8*)fline+offsets[1] );
    slot  =  hash % slot_lim;
/*
sprintf( zzz,"slot = %d\n",slot );
JBMLine( zzz );
*/
  } else
    slot  =  HashzNom( znom ) % slot_lim;

  while ( h_array[slot].zname ) {       // slot is already occupied
    slot++;
    if ( slot == slot_lim ) slot  =  0;
  }

  // Assign field values for the hash table entry

  h_array[slot].zname     =  znom;
  if ( t_node == h_tables )
    h_array[slot].zuID[ 0 ] =  0;
  else {
    U16 zuID_len  =  offsets[2] - offsets[1] - 1;
    strncpy( (char*)h_array[slot].zuID,fline+offsets[1],zuID_len );
    h_array[slot].zuID[ zuID_len ]  =  0;
  }

  // extract the template

  char* ptr =  fline + offsets[2];
  if ( *ptr >= ' ' ) {
    zln =  strlen( ptr );
    U8* ztempl  =  (U8*)TCI_NEW(char[zln+1] );
    strcpy( (char*)ztempl,ptr );
    h_array[slot].ztemplate =  ztempl;
  }
}


U32 Grammar::HashzNom( U8* znom ) {

  U32 rv  =  0L;

  U32 fudge =  31L;
  while ( *znom ) {
    rv  +=  (*znom - 31) * fudge;
    znom++;
	if ( fudge == 7L )
	  fudge =  31L;
	else
	  fudge =  fudge - 12L;
  }

  return rv;
}


U16 Grammar::HashzuID( U8* zuID ) {

  U8* ptr   =  zuID;
  U32 temp  =  0L;
  U16 state =  0;
  U16 shift =  0;

  while ( state<3 ) {

    while ( *ptr>='0' &&  *ptr<='9' ) {
      temp  +=  ( *ptr << shift );
      shift++;
      ptr++;
    }

    ptr++;
    state++;
  }

/*
char zzz[80];
sprintf( zzz,"Hash,%s = %lu\n",zuID,temp );
JBMLine( zzz );
*/
  return temp;
}


TCI_BOOL Grammar::IsFormMatch( U8* req_form,U8* obj_def ) {

  TCI_BOOL rv  =  TRUE;
  if ( req_form ) {
    U16 zln =  strlen( (char*)req_form );
    if ( strncmp((char*)obj_def,(char*)req_form,zln) )
	  rv  =  FALSE;
  }

  return rv;
}


