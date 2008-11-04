
#include "fltutils.h"
#include <string.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

U8* zNONLATEX =  (U8*)"888.8.0";
U8* zDIMEN    =  (U8*)"888.7.0";
U8* zGLUE     =  (U8*)"888.10.0";


char  dumpspec[256];
FILE* dumpfile  =  NULL;

// diagnostic variables - we track TNODEs

U16 tnodes_created  =  0;
U16 tnodes_disposed =  0;

void FUSetDumpFile( char* dump_file_spec ) {

  if ( dumpfile ) {
    fclose( dumpfile );
    dumpfile  =  NULL;
  }
  
  dumpspec[0] =  0;

  if ( dump_file_spec ) {
    strcpy( dumpspec,dump_file_spec );
/*
char zzz[256];
sprintf( zzz,"DLL SetDumpFile=%s\n",dump_file_spec );
JBMLine( zzz );
*/
  }
}

TCI_BOOL  FUOpenDumpFile() {

  if ( dumpspec[0] ) {
    dumpfile  =  fopen( dumpspec,"a" );
	if ( dumpfile ) {
//JBMLine( "DLL Opens DumpFile\n" );
	  return TRUE;
    }
  }

//JBMLine( "DLL Fails to open DumpFile\n" );
  return FALSE;
}

void FUCloseDumpFile() {

  if ( dumpfile ) {
    fclose( dumpfile );
    dumpfile  =  NULL;
//JBMLine( "DLL Closes DumpFile\n" );
  }
}

void DumpLine( char* line ) {    // DIAGNOSTIC

  if ( dumpfile ) {
    fputs( line,dumpfile );
    fflush( dumpfile );
  }
}


// Global utility functions

void JBMLine( char* line ) {   // DIAGNOSTIC

#ifdef TESTING
  FILE* jbmfile =  fopen( "\\jbm","a" );
  if ( jbmfile ) {
    fputs( line,jbmfile );
    fclose( jbmfile );
  }
#endif
}


void DumpTNode( TNODE* t_node,U16 indent,U16 grammar_ID ) {

  if ( t_node ) {

// form the indentation string

    char indent_str[128];
    U16 ii  =  0;
    while ( ii<indent && ii<127 ) indent_str[ ii++ ]  =  ' ';
    indent_str[ii] =  0;

/*
char zzz[80];
sprintf( zzz,"off2=%ld, off1=%ld\n",t_node->src_offset2,t_node->src_offset1 );
JLogLine( zzz );
*/

    char zzz[256];
    sprintf( zzz,"%8lu %s<%s>%s\n",
                t_node->src_linenum,
                    indent_str,t_node->zuID,t_node->src_tok );
    DumpLine( zzz );
	if ( t_node->var_value ) {
      DumpLine( (char*)t_node->var_value );
      DumpLine( "\n" );
	}
	if ( t_node->var_value2 ) {
      DumpLine( (char*)t_node->var_value2 );
      DumpLine( "\n" );
	}

    CVLINE_REC* cv_rover  =  t_node->cv_list;
	while ( cv_rover ) {
      DumpLine( (char*)cv_rover->cvline );
      DumpLine( "\n" );
	  cv_rover  =  cv_rover->next;
	}

  } else
    TCI_ASSERT(0);

}


void DumpTList( TNODE* t_list,U16 indent,U16 grammar_ID ) {

  while ( t_list ) {
    DumpTNode( t_list,indent,grammar_ID );
    if ( t_list->parts ) {
//LogLine( "PARTS::\n" );
      DumpTList( t_list->parts,indent+2,grammar_ID );
    }
    if ( t_list->contents ) {
//LogLine( "CONTENTS::\n" );
      DumpTList( t_list->contents,indent+2,grammar_ID );
    }
    t_list  =  t_list->next;
  }

}



// Utility to make a translation node

TNODE* MakeTNode( U32 s_off,U32 e_off,U32 line_no,U8* zuID ) {

  TNODE* rv         =  (TNODE*)TCI_NEW( char[ sizeof(TNODE) ] );
  rv->next          =  (TNODE*)NULL;
  rv->prev          =  (TNODE*)NULL;
  rv->src_offset1   =  s_off;
  rv->src_offset2   =  e_off;
  rv->src_linenum   =  line_no;
  rv->status        =  0;
  if ( zuID ) {
    strcpy( (char*)rv->zuID,(char*)zuID );
  } else
    rv->zuID[0] =  0;

  rv->cuID[0]   =  0;
  rv->src_tok[0]  =  0;

  rv->contents        =  (TNODE*)NULL;
  rv->parts           =  (TNODE*)NULL;
  rv->sublist_owner   =  (TNODE*)NULL;
  rv->var_value       =  (U8*)NULL;
  rv->var_value2      =  (U8*)NULL;
  rv->v_len           =  0;
  rv->v_type          =  0;
  rv->attrib_list	  =  (ATTRIB_REC*)NULL;
  rv->msg_list	      =  (LOG_MSG_REC*)NULL;
  rv->details	      =  (DETAILS*)NULL;
  rv->cv_list	      =  (CVLINE_REC*)NULL;

  tnodes_created++;
  return rv;
}

/*
// Utility to make an attribute node

ATTRIB_REC* MakeATTRIBNode( U16 attr_ID,U16 e_ID,
								U8* svalue,U16 vln,
									U16 val_type ) {

  ATTRIB_REC* rv    =  (ATTRIB_REC*)TCI_NEW( char[ sizeof(ATTRIB_REC) ] );
  rv->next          =  (ATTRIB_REC*)NULL;
  rv->prev          =  (ATTRIB_REC*)NULL;

  rv->attr_ID =  attr_ID;
  rv->e_val   =  e_ID;

  TCI_ASSERT( attr_ID );

  if ( svalue ) {		// we have a STR value for the attribute
    U8* tmp	=  (U8*)TCI_NEW( char[vln+1] );
	strncpy( (char*)tmp,(char*)svalue,vln );
	tmp[vln]  =  0;
	rv->attr_val  =  tmp;
  } else
	rv->attr_val  =  (U8*)NULL;

  rv->attr_vtype  =  val_type;

  return rv;
}
*/

void DisposeAttribs( ATTRIB_REC* alist ) {

  ATTRIB_REC* arover  =  alist;
  while ( arover ) {
    ATTRIB_REC* adel  =  arover;
	  arover  =  arover->next;
    if ( adel->attr_nom ) delete adel->attr_nom;
    if ( adel->z_val    ) delete adel->z_val;
	  delete adel;
  }
}


void DisposeTNode( TNODE* del ) {

  if ( del->var_value )
    delete del->var_value;
  if ( del->var_value2 )
    delete del->var_value2;

  DisposeAttribs( del->attrib_list );

  LOG_MSG_REC* msg_rover  =  del->msg_list;
  while ( msg_rover ) {
    LOG_MSG_REC* msg_del  =  msg_rover;
	msg_rover  =  msg_rover->next;
    if ( msg_del->msg  ) delete msg_del->msg;
	delete msg_del;
  }

  if ( del->details ) delete del->details;

  if ( del->cv_list ) DisposeCVLines( del->cv_list );

  if ( del->parts )
    DisposeTList( del->parts );
  if ( del->contents )
    DisposeTList( del->contents );
  delete del;
  tnodes_disposed++;
}


void DisposeTList( TNODE* t_list ) {

  TNODE* del;
  while ( t_list ) {
    del     =  t_list;
    t_list  =  t_list->next;
    DisposeTNode( del );
  }
}


void UidsTozuID( U16 uobjtype,U16 usubtype,U16 uID,U8* dest ) {

  // jcs -- itoa is nonstandard

  //itoa( uobjtype,(char*)dest,10 );
  sprintf((char*) dest, "%d", uobjtype);

  U16 off =  strlen( (char*)dest );
  dest[ off++ ] =  '.';
  
  // itoa( usubtype,(char*)dest+off,10 );
  sprintf((char*) dest + off, "%d", usubtype);

  off =  strlen( (char*)dest );
  dest[ off++ ] =  '.';

  //itoa( uID,(char*)dest+off,10 );
  sprintf((char*)dest+off, "%d", uID);
}


// idstr = "9.10.102,a,l>
void GetUids( U8* idstr,U16& uobjtype,U16& usubtype,U16& uID ) {

  uobjtype  =  atoi( (char*)idstr );
  usubtype  =  0;
  uID       =  0;

  U8 ch;
  while ( ch = *idstr ) {
    if ( ch == '.' || ch == '>' || ch == ',' ) break;
    idstr++;
  }
  if ( ch == '.' ) {
    idstr++;
    if ( *idstr >= '0' && *idstr <= '9' )
      usubtype  =  atoi( (char*)idstr );

    while ( ch = *idstr ) {
      if ( ch == '.' || ch == '>' || ch == ',' ) break;
      idstr++;
    }
    if ( ch == '.' ) {
      idstr++;
      if ( *idstr >= '0' && *idstr <= '9' )
        uID =  atoi( (const char*)idstr );
    }
  }
}


void StrReplace( char* line,char* tok,char* sub ) {

  char buffer[256];

  char* ptr =  strstr( line,tok );
  if ( ptr ) {
    *ptr  =  0; 
    strcpy( buffer,line );      // head
    strcat( buffer,sub );       // substitution
    ptr +=  strlen( tok );
    strcat( buffer,ptr );       // tail

    strcpy( line,buffer );
  }
}



void CheckTNODETallies() {

  if ( tnodes_created != tnodes_disposed  ) {
//*
char zzz[80];
sprintf( zzz,"Nodes created = %d, nodes disposed = %d\n",
                                tnodes_created,tnodes_disposed );
JBMLine( zzz );
//*/
	  TCI_ASSERT( 0 );
  }

}


// A utility to locate an object node in a parts list

TNODE* FindObject( TNODE* parts_list,U8* zID,U16 list_pos ) {

  TNODE* rv =  (TNODE*)NULL;

  U8 zid[64];
  strcpy( (char*)zid,(char*)zID );
  if ( list_pos != INVALID_LIST_POS ) {
    U16 ln  =  strlen( (char*)zid );
    zid[ ln++ ] =  ':';
    
    // itoa( list_pos,(char*)zid+ln,10 );
    sprintf((char*)zid+ln, "%d", list_pos);
  }

  U16 zln =  strlen( (char*)zid );

/*
char zzz[80];
sprintf( zzz,"FindObject:  seeking=%s\n",zid );
JBMLine( zzz );
*/

  TNODE* rover  =  parts_list;
  while ( rover ) {
/*
sprintf( zzz,"  current=%s\n",rover->zuID );
JBMLine( zzz );
*/
    if ( !strncmp((char*)zid,(char*)rover->zuID,zln) ) {
      rv  =  rover;
      break;
    }

    rover =  rover->next;
  }

//if ( rv ) JBMLine( "FOUND\n" );
//else    JBMLine( "NOT FOUND\n" );

  return rv;
}


void DisposeMsgs( LOG_MSG_REC* msg_list ) {

  LOG_MSG_REC* msg_rover  =  msg_list;
  while ( msg_rover ) {
    LOG_MSG_REC* msg_del  =  msg_rover;
	msg_rover  =  msg_rover->next;
    if ( msg_del->msg  ) delete msg_del->msg;
	delete msg_del;
  }

}


CVLINE_REC* MakeCVNode( U16 whatever,U8* cv_line,U16 cv_ln  ) {

  CVLINE_REC* rv  =  (CVLINE_REC*)TCI_NEW( char[ sizeof(CVLINE_REC) ] );
  rv->next        =  NULL;
  rv->prev        =  NULL;			
  rv->whatever    =  whatever;
  if ( cv_line ) {			// we have a line to store
    U8* tmp	=  (U8*)TCI_NEW( char[cv_ln+1] );
	strncpy( (char*)tmp,(char*)cv_line,cv_ln );
	tmp[cv_ln]  =  0;
	rv->cvline  =  tmp;
  } else
	rv->cvline  =  (U8*)NULL;
  return rv;
}


CVLINE_REC* JoinCVLists( CVLINE_REC* cv_list,CVLINE_REC* new_tail ) {

  CVLINE_REC* rv;
  if ( cv_list ) {
    rv  =  cv_list;
	while ( cv_list->next ) cv_list =  cv_list->next;
	cv_list->next =  new_tail;
	if ( new_tail )
	  new_tail->prev  =  cv_list;
  } else
    rv  =  new_tail;
  return rv;
}


void DisposeCVLines( CVLINE_REC* cv_list ) {

  CVLINE_REC* cv_rover  =  cv_list;
  while ( cv_rover ) {
    CVLINE_REC* cv_del  =  cv_rover;
	cv_rover  =  cv_rover->next;
    if ( cv_del->cvline  ) delete cv_del->cvline;
	delete cv_del;
  }
}


TNODE*  JoinTLists( TNODE* list,TNODE* newtail ) {

  TNODE* rv;
  if ( list ) {
    rv  =  list;
	while ( list->next ) list =  list->next;
	list->next  =  newtail;
	if ( newtail )
	  newtail->prev =  list;

  } else {
    rv  =  newtail;
  }

  return rv;
}

// Removed "ByteArray" from this module.  Added the following function
//  to handle expandable buffers.

U8* AppendBytesToBuffer( U8* buffer,U32& buf_lim,U32& buf_i,
									U8* new_bytes,U32 n_bytes ) {

  U8* rv;

  if ( buf_i+n_bytes+1 >= buf_lim ) {		// expand the buffer
	U32 new_size  =  buf_i + n_bytes + 4096;
    rv  =  (U8*)TCI_NEW( char[new_size] );
	buf_lim =  new_size;
	memcpy( (char*)rv,buffer,buf_i );
	delete buffer;
  } else
    rv  =  buffer;
  
  memcpy( (char*)rv+buf_i,(char*)new_bytes,n_bytes );
  buf_i +=  n_bytes;

  rv[buf_i] =  0;

  return rv;
}


ATTRIB_REC* MakeATTRIBNode( U8* attrib_name,
							  U16 attr_ID,U16 e_ID,
								U8* svalue,U16 vln,
								  U16 val_type ) {

/*
typedef struct tagATTRIB_REC {	// Struct to define an attribute node.
  tagATTRIB_REC*  next;			//  The TNODE struct defined below
  tagATTRIB_REC*  prev;			//  can carry a list of attributes.
  U16             attr_ID;
  U8*             attr_nom;
  U16             e_val;
  U8*             z_val;
  U16             attr_vtype;
} ATTRIB_REC;
*/

  ATTRIB_REC* rv  =  (ATTRIB_REC*)TCI_NEW( char[sizeof(ATTRIB_REC)] );
  rv->next        =  (ATTRIB_REC*)NULL;
  rv->prev        =  (ATTRIB_REC*)NULL;

  TCI_ASSERT( attr_ID );
  if ( attrib_name ) {
	U16 nln =  strlen( (char*)attrib_name );
    U8* tmp	=  (U8*)TCI_NEW( char[nln+1] );
	strcpy( (char*)tmp,(char*)attrib_name );
	rv->attr_nom  =  tmp;
  } else {
    TCI_ASSERT( 0 );
	rv->attr_nom  =  (U8*)NULL;
  }

  rv->attr_ID =  attr_ID;
  rv->e_val   =  e_ID;

  if ( svalue ) {		  	// we have a STR value for the attribute
    U8* tmp	=  (U8*)TCI_NEW( char[vln+1] );
	strncpy( (char*)tmp,(char*)svalue,vln );
	tmp[vln]  =  0;
	rv->z_val =  tmp;
  } else {
    TCI_ASSERT( 0 );
	rv->z_val =  (U8*)NULL;
  }

  rv->attr_vtype  =  val_type;

  return rv;
}


TILE* AppendTILEs( TILE* tile_list,TILE* new_tail ) {

  TILE* rv;

  if ( tile_list ) {
    rv  =  tile_list;
	while ( tile_list->next )
	  tile_list =  tile_list->next;
	tile_list->next =  new_tail;
	if ( new_tail )
	  new_tail->prev  =  tile_list;
  } else
    rv  =  new_tail;

  return rv;
}

   
void DisposeTILEs( TILE* tiles ) {

  if ( tiles ) {

    TILE* rover =  tiles;
	while ( rover->next ) rover =  rover->next;

    while ( rover != tiles ) {
      TILE* del =  rover;
	  rover =  rover->prev;
	  if ( del->zval ) delete del->zval;
	  if ( del->msg_list ) DisposeMsgs( del->msg_list );

	  delete del;
	}

	if ( rover->prev ) rover->prev->next  =  NULL;
	if ( rover->zval ) delete rover->zval;
	delete rover;

  }
}


// Utility to make a tile

TILE* MakeTILE( U16 ilk,U8* zval ) {

  TILE* rv   =  (TILE*)TCI_NEW( char[ sizeof(TILE) ] );
  rv->next      =  (TILE*)NULL;
  rv->prev      =  (TILE*)NULL;
  rv->ilk       =  ilk;
  rv->msg_list  =  NULL;

  if ( zval ) {
    U16 zln   =  strlen( (char*)zval );
    U8* tmp   =  (U8*)TCI_NEW( char[ zln+1 ] );
    strcpy( (char*)tmp,(char*)zval );
    rv->zval  =  tmp;
  } else
    rv->zval  =  NULL;

  return rv;
}


void DumpTILEs( TILE* tile_list ) {

  TILE* rover =  tile_list;
  while ( rover ) {

char zzz[256];  
sprintf( zzz,"%s! ilk=%d\n",rover->zval,rover->ilk );
JBMLine( zzz );
  
    rover =  rover->next;
  }

}


TILE* JoinTILELists( TILE* tlist,TILE* new_tail ) {

  if ( !tlist )
    return new_tail;

  TILE* tile_rover  =  tlist;
  while ( tile_rover->next )
    tile_rover  =  tile_rover->next;

  tile_rover->next  =  new_tail;
  if ( new_tail )
    new_tail->prev  =  tile_rover;

  return tlist;
}


// WARNING - The following function permutes a LaTeX parse tree.
//  In LaTeX, rules ( \hline,\cline{1-2} ) can occur before or
//  after each row in arrays and tabulars.  I don't have a special
//  production in my grammar description language to put these
//  cmds in a bucket - they are currently handled as a LIST of
//  conditional tokens in notebook.gmr.  Here we collect the
//  contents of the entries in this list and put them in a bucket.

void HLinesToBucket( TNODE* LaTeX_AorT,TCI_BOOL is_tabular ) {

  U16 subtype =  is_tabular ? 490 : 35;

// Locate list of lines in array or tabular object - uID is 9

  U8 lines_uID[32];
  UidsTozuID( 5,subtype,9,(U8*)lines_uID );
  TNODE* rows_list_node =  FindObject( LaTeX_AorT->parts,
                                (U8*)lines_uID,INVALID_LIST_POS );
  if ( rows_list_node ) {
    TNODE* row_rover  =  rows_list_node->parts;
    while ( row_rover ) {	// loop thru rows in array or tabular

// Locate list of rules above the current line.
// Note that _HRULES_ is the same list in array and tabular.

      TNODE* hr_list_node =  FindObject( row_rover->parts,
	  							(U8*)"5.490.44",INVALID_LIST_POS );

// The list of rules should always be present - it may be empty.

      U16 rule_count  =  0;
      TNODE* head =  NULL;
      TNODE* tail =  NULL;
      if ( hr_list_node ) {

        TNODE* hrl_rover  =  hr_list_node->parts;
        while ( hrl_rover ) {	// loop thru \hline, \cline{} list
          if ( hrl_rover->parts ) {
            rule_count++;
            TNODE* rule =  hrl_rover->parts;
            DelinkTNode( rule );
            if ( tail ) {
              tail->next  =  rule;
              rule->prev  =  tail;
            } else
              head  =  rule;
            tail  =  rule;
            while ( tail->next )
              tail  =  tail->next;
          } //else
		    //TCI_ASSERT(0);
          hrl_rover =  hrl_rover->next;
        }		// loop thru rule nodes


        if ( head ) {		// we found some rules

  // Make a bucket for the rules - give it a type.

          TNODE* bucket =  MakeTNode( 0L,0L,0L,(U8*)"5.490.13" );
          strcpy( (char*)bucket->cuID,"70.20.0" );
          bucket->contents    =  head;
          head->sublist_owner =  bucket;

  // Insert "bucket" in current line's parts list
  //  at the right of the old list-of-rules.

          if ( hr_list_node->next ) {
            bucket->next  =  hr_list_node->next;
            hr_list_node->next->prev  =  bucket;
          }
          bucket->prev  =  hr_list_node;
          hr_list_node->next  =  bucket;

        }		// if ( head )

  // Remove the old list-of-rules from the current line.

        DelinkTNode( hr_list_node );
        DisposeTNode( hr_list_node );

        if ( !row_rover->next && rule_count ) {

  // Here we are at the last row in this tabular and it has rules.
  // The parser generates a list of columns for this row - it has
  //  a single entry which is empty.  We dispose this empty list.

          U8 columns_list_uID[32];
          UidsTozuID( 5,subtype,11,(U8*)columns_list_uID );
          TNODE* columns_list_node =  FindObject( row_rover->parts,
						(U8*)columns_list_uID,INVALID_LIST_POS );
          if ( columns_list_node ) {
            U16 c_count =  0;
            bool empty  =  true;
            if ( columns_list_node->parts ) {
              TNODE* c_rover  =  columns_list_node->parts;
              while ( c_rover ) {
                c_count++;
                if ( c_rover->parts ) {
                  if ( c_rover->parts->contents )
                    empty =  false;
                } else
                  TCI_ASSERT(0);
                c_rover =  c_rover->next;
              }
            }
            if ( c_count==1 && empty ) {
              DelinkTNode( columns_list_node );
              DisposeTNode( columns_list_node );
            }
          }		// if ( columns_list_node )
        }		// if ( !row_rover->next && rule_count )


      } else {			// rules list for current line missing 
        //TCI_ASSERT(0);
      }

      row_rover =  row_rover->next;
    }			// while ( row_rover )

  } else {		// failed to find list of lines in array
    //TCI_ASSERT(0);
  }
}


/*
\begin{array}<uID5.35.0>!\begin{array}!_AXALIGN__ACOLS__AROWS_!\end{array}!
_ACOLS_REQPARAM(5.35.3,COLS)

\begin{tabular}<uID5.490.0>!\begin{tabular}!_TXALIGN__TCOLS__TROWS_!\end{tabular}!
_TCOLS_REQPARAM(5.490.3,COLS)

_SACOLS_REQPARAM(5.717.3,COLS)

We break the tokens in a colsbucket into a list, each entry
 corresponding to to the cols for one tabular column.
*/

void ColsToList( TNODE* LaTeX_AorT ) {

  U16 objtype,subtype,uID;
  GetUids( LaTeX_AorT->zuID,objtype,subtype,uID );
  if ( subtype == 491 ) subtype =  490;

// Locate the cols bucket of the array or tabular

  U8 cols_uID[32];
  UidsTozuID( 5,subtype,3,(U8*)cols_uID );
  TNODE* cols_bucket =  FindObject( LaTeX_AorT->parts,
                                (U8*)cols_uID,INVALID_LIST_POS );
  if ( cols_bucket ) {

    TNODE* head =  NULL;	// variables for entry list
    TNODE* tail =  NULL;	//  that we are constructing.
	  U16 pos =  0;

    bool is_first_col =  true;
    TNODE* c_rover  =  cols_bucket->contents;
	  DetachTList( c_rover );
	  c_rover =  RemoveStarCmds( c_rover );

    TNODE* batch  =  c_rover;
	  while ( c_rover ) {		// loop thru tokens in old cols bucket

      U16 objtype,subtype,uID;
      GetUids( c_rover->zuID,objtype,subtype,uID );
	    TCI_ASSERT( objtype==16 && subtype==4 );
	    if ( uID == 1 || uID == 2 || uID == 3 || uID == 6 ) {   // l,c,r,p
        if ( is_first_col )
          is_first_col  =  false;
		    else {
	        TNODE* entry  =  MakeColsEntry( batch,pos );
          batch =  c_rover;
          pos++;
	// add the new entry to our list
          if ( tail ) {
            tail->next  =  entry;
            entry->prev =  tail;
          } else
            head  =  entry;
          tail  =  entry;
        }
      }

      c_rover =  c_rover->next;
    }		// while ( c_rover )

  // Handle the last entry

    TNODE* entry  =  MakeColsEntry( batch,pos );
  // add the new entry to our list
    if ( tail ) {
      tail->next  =  entry;
      entry->prev =  tail;
    } else
      head  =  entry;

  // replace old cols bucket with new cols list

    if ( head ) {		// we have a list of entries

  // Make a list node to carry the entries.

      TNODE* list =  MakeTNode( 0L,0L,0L,(U8*)"5.490.111" );
      list->parts =  head;
      head->sublist_owner =  list;

  // Insert "list" in current tabular's parts list
  //  at the right of the old cols_bucket.

      if ( cols_bucket->next ) {
        list->next  =  cols_bucket->next;
        cols_bucket->next->prev =  list;
      }
      list->prev  =  cols_bucket;
      cols_bucket->next =  list;

    }		// if ( head )

  // Remove the old cols_bucket from the tabular's parts.

    DelinkTNode( cols_bucket );
    DisposeTNode( cols_bucket );

  } else {			// cols_bucket not found
    //TCI_ASSERT(0);
  }
}


// Here we make a list entry in the process of breaking
//  the contents of a cols_bucket into a list.
//  |l||cr|
//  |l||		c		r|
//	^			^		^

TNODE* MakeColsEntry( TNODE* batch,U16 pos ) {

// Make the returned cols_list entry node

  U8 cols_entryID[32];
  strcpy( (char*)cols_entryID,"5.490.111:" );
  
  // itoa( pos,(char*)cols_entryID+10,10 );
  sprintf((char*)cols_entryID+10, "%d", pos);
  
  TNODE* rv =  MakeTNode( 0L,0L,0L,(U8*)cols_entryID );

// Make a typed cols bucket and assign it to rv->parts

  TNODE* bucket =  MakeTNode( 0L,0L,0L,(U8*)"5.490.113" );
  strcpy( (char*)bucket->cuID,"70.7.0" );
  rv->parts =  bucket;
  bucket->sublist_owner =  rv;

// Set the contents of bucket

  DetachTList( batch );
  bucket->contents  =  batch;
  batch->sublist_owner  =  bucket;

  return rv;
}


// The contents list of a COLS param can contain *{}{} cmds.
//  ie. *{5}{c||} - this cmd repeats a sequence of cols tokens.
// Before the COLS param of a tabular/array environment is
//  separated into a list with one entry for each column
//  we need to replace these cmds with the required
//  number of copies of their second param.

TNODE* RemoveStarCmds( TNODE* cols_list ) {

// Note that the head of cols_list may be replaced by this function.

  TNODE* rv =  cols_list;

  TNODE* c_rover  =  cols_list;
  while ( c_rover ) {		// loop thru tokens in cols bucket
    U16 objtype,subtype,uID;
    GetUids( c_rover->zuID,objtype,subtype,uID );
	if ( objtype==16 && subtype==4 && uID == 7 ) {    // *{}{}
      TNODE* sub  =  ExpandStarCmd( c_rover );
	  if ( sub ) {
      // link in sub at right of c_rover
        if ( c_rover->next ) {
	      TNODE* tail =  sub;
		  while ( tail->next )
		    tail  =  tail->next;
		  tail->next  =  c_rover->next;
		  c_rover->next->prev =  tail;
		}
		c_rover->next =  sub;
		sub->prev =  c_rover;
	  // delink the node we're replacing
        if ( c_rover->prev )
          DelinkTNode( c_rover );
		else {		// we're replacing the head of the list
		  sub->prev =  NULL;
		  rv  =  sub;
		}
	  // dispose the node we're replacing
		DisposeTNode( c_rover );
		c_rover =  sub;

	  } else {
	    TCI_ASSERT(0);
        c_rover =  c_rover->next;
	  }

	} else
      c_rover =  c_rover->next;

  }		// loop thru cols nodes

  return rv;
}


// LaTeX uses *{10}{c|} in a COLS param as a convenient way
//  to repeat a group of tokens, c| in this case.  We generate
//  the expanded list of token nodes here.
// *<uID16.4.7>!*!REQPARAM(16.4.52,NONLATEX)REQPARAM(16.4.53,COLS)

TNODE* ExpandStarCmd( TNODE* star_cmd ) {

  TNODE* head =  NULL;
  TNODE* tail =  NULL;

  TNODE* nrepeats_node  =  FindObject( star_cmd->parts,
	  						(U8*)"16.4.52",INVALID_LIST_POS );
  U16 nrepeats  =  atoi( (char*)nrepeats_node->contents->var_value );

  TNODE* cols_bucket    =  FindObject( star_cmd->parts,
	  						(U8*)"16.4.53",INVALID_LIST_POS );

  if ( nrepeats && cols_bucket && cols_bucket->contents ) {

    while ( nrepeats ) {
	  TNODE* copy =  CopyTList( cols_bucket->contents );
      if ( tail ) {
	    tail->next  =  copy;
		copy->prev  =  tail;
	  } else
	    head  =  copy;

	  tail  =  copy;
	  while ( tail->next )
	    tail  =  tail->next;

	  nrepeats--;
    }			// while ( nrepeats )

  }		// if ( nrepeats && cols_bucket && cols_bucket->contents )

  return head;
}


// Function to remove a TNODE's links to a tree.
// The delinked node can be moved or disposed by the caller.
// Note that we can't delink the first node of a first
//  level list here.

bool DelinkTNode( TNODE* elem ) {

  bool rv =  true;		// assume all will be OK.

// remove elem from it's present list:  prev <-> elem <-> next

  TNODE* el_prev  =  elem->prev;
  if ( el_prev ) {					// elem has a prev
    el_prev->next =  elem->next;
    if ( elem->next )
      elem->next->prev  =  el_prev;
  } else {							// elem may head a sublist
    TNODE* el_owner =  elem->sublist_owner;
    if ( el_owner ) {
	  if        ( el_owner->parts == elem ) {
	    el_owner->parts =  elem->next;
	    if ( elem->next ) {
	      elem->next->sublist_owner =  el_owner;
	      elem->next->prev  =  NULL;
		}
	  } else if ( el_owner->contents == elem ) {
	    el_owner->contents =  elem->next;
	    if ( elem->next ) {
	      elem->next->sublist_owner =  el_owner;
	      elem->next->prev  =  NULL;
		}
	  } else {
	    TCI_ASSERT(0);
	    rv  =  false;
	  }
    } else {			// No sublist_owner
	  TCI_ASSERT(0);
	  rv  =  false;
	}
  }

  if ( rv ) {
    elem->prev  =  NULL;
    elem->next  =  NULL;
	elem->sublist_owner =  NULL;
  }

  return rv;
}


void DetachTList( TNODE* elem ) {

/*
  TNODE* next_node  =  elem->next;
  if ( next_node ) {					// elem has a next
// remove elem from it's present list:  elem <-/-> next
    next_node->prev =  NULL;
    elem->next  =  NULL;
  }
*/

  TNODE* prev_node  =  elem->prev;
  if ( prev_node ) {					// elem has a prev
// remove elem from it's present list:  prev <-> elem
    prev_node->next =  NULL;
    elem->prev  =  NULL;
  } else {							// elem may head a sublist
    TNODE* owner_node =  elem->sublist_owner;
    if ( owner_node ) {
	    if        ( owner_node->parts == elem ) {
	      owner_node->parts     =  NULL;
	    } else if ( owner_node->contents == elem ) {
	      owner_node->contents  =  NULL;
	    } else
	      TCI_ASSERT(0);
	    elem->sublist_owner =  NULL;
    } //else							// No sublist_owner
	  //TCI_ASSERT(0);
  }

}


// Two (mutually recursive) functions to generate a copy
//  of a list of TNODEs - only data and type info is copied.

TNODE* CopyTList( TNODE* list ) {

  TNODE* head =  NULL;
  TNODE* tail =  NULL;

  while ( list ) {
    TNODE* copy =  CopyTNode( list );
	  if ( tail ) {
	    tail->next  =  copy;
	    copy->prev  =  tail;
	  } else
	    head  =  copy;
	  tail  =  copy;

    list  =  list->next;
  }		// loop thru list of TNODEs

  return head;
}


TNODE* CopyTNode( TNODE* node ) {

  TNODE* rv =  MakeTNode( 0,0,0,node->zuID );
  if ( node->cuID[0] )
    strcpy( (char*)rv->cuID,(char*)node->cuID );

  if ( node->var_value ) {
	U16 new_ln    =  strlen( (char*)node->var_value );
    U8* the_value =  (U8*)TCI_NEW( char[new_ln+1] );
    strcpy( (char*)the_value,(char*)node->var_value );
	rv->var_value =  the_value;
    rv->v_len =  new_ln;
  }

  if ( node->var_value2 ) {
	U16 new_ln    =  strlen( (char*)node->var_value2 );
    U8* the_value =  (U8*)TCI_NEW( char[new_ln+1] );
    strcpy( (char*)the_value,(char*)node->var_value2 );
	rv->var_value2  =  the_value;
    rv->v_len =  new_ln;
  }

  if ( node->parts ) {
    TNODE* parts  =  CopyTList( node->parts );
	rv->parts =  parts;
	parts->sublist_owner  =  rv;
  }

  if ( node->contents ) {
    TNODE* conts  =  CopyTList( node->contents );
	rv->contents  =  conts;
	conts->sublist_owner =  rv;
  }

  return rv;
}


/*
\MATRIX[c]{4,6}{c}\VR{1,,r,,,}{,,r,,,}{,@{--},l,,,}{,,,p{1.25in},,}{1,,,,,}%
\HR{\hline,\hline\hline,\cline{2-3},\hline,\hline,\hline,\hline}%
\CELL[4,1,1,1,,,c,,]{\sin \ln \theta }%
\CELL{}%
\CELL[2,1,,,,,c,,]{\cos }%
\CELL[1,1,1,1,,,c,,p{1.25in}]{}%
\CELL[1,1,1,2,,,c,,]{\tan }%
\CELL[1,1,,,,,r,@{\,\vline\,},]{low}
*/

// NOTE: this function also does \TABLE -> \begin{tabular}
// NOTE: the syntax for \MATRIX and \TABLE are the same,
//  except for an additional OPTPARAM, [width], in \TABLE,
//  and MATH in \MATRIX cells versus TEXT in \TABLE cells.
// The syntax for \begin{array} and \begin{tabular} are the same.
//  \begin{tabular*} has an additional REQPARAM, {width}.

TNODE* MATRIXtoExternalFormat( TNODE* MATRIX_node ) {

// Loop thru the parts of \MATRIX, recording pointers to the major
//  parts needed for the translation to "\begin{array}" as we go.

// \TABLE <uID5.37.0>!\TABLE!      _COLSANDROWS__EXTALIGN__TWIDTH__VR__HR__TCELLS_
// \MATRIX<uID5.36.0>!\MATRIX!_ILK__COLSANDROWS__EXTALIGN_       _MVR__HR__MCELLS_
  TNODE* ilk_optparam =  NULL;// _ILK_OPTPARAM(5.36.34,NONLATEX)  -- MATRIX ONLY
  U16 nrows =  0;             // _COLSANDROWS_REQPARAM(5.36.12,NONLATEX)
  U16 ncols =  0;
  U8* ext_align_uID   =  NULL;// _EXTALIGN_!{!VAR(5.36.1,MEXTALIGN,a,c,})!}!
  TNODE* width        =  NULL;// _TWIDTH_OPTPARAM(5.36.20,DIMEN)  -- TABLE ONLY
  TNODE* VR_list      =  NULL;// _MVR_!\VR!LIST(5.36.2,_COLATTRIBS_,5.36.3,{,\HR,)
  TNODE* HR_reqparam  =  NULL;// _HR_!\HR!REQPARAM(5.36.9,MATH)
  TNODE* CELL_list    =  NULL;// _MCELLS_LIST(5.36.6,_MCELLARGS_,5.36.7,\CELL,,)
							                // _MCELLARGS_OPTPARAM(5.36.33,COLS)REQPARAM(5.36.8,MATH)
                              // _COLATTRIBS_BUCKET(5.36.29,COLS,,},,)

  TNODE* parts_rover  =  MATRIX_node->parts;
  while ( parts_rover ) {
    U16 uobjtype,usubtype,uID;
    GetUids( parts_rover->zuID,uobjtype,usubtype,uID );
	  if        ( uobjtype==5 && usubtype==36 && uID==34  ) { // _ILK_
      ilk_optparam   =  parts_rover;
	  } else if ( uobjtype==5 && usubtype==36 && uID==12 ) {  // _COLSANDROWS_
      TNODE* cont =  FindObject( parts_rover->contents,zNONLATEX,
      											INVALID_LIST_POS );
      TCI_ASSERT( cont );
      char* p =  (char*)cont->var_value;
      ncols   =  atoi( p );
	    p =  strchr( p,',' );
      nrows   =  atoi( p+1 );
	  } else if ( uobjtype==5 && usubtype==36 && uID==1  ) {  // _EXTALIGN_!
      ext_align_uID =  parts_rover->var_value;
	  } else if ( uobjtype==5 && usubtype==36 && uID==20 ) {  // _TWIDTH_
      width   =  parts_rover;
	  } else if ( uobjtype==5 && usubtype==36 && uID==2  ) {  // _MVR_
      VR_list =  parts_rover;
	  } else if ( uobjtype==5 && usubtype==36 && uID==9  ) {  // _HR_
      HR_reqparam =  parts_rover;
	  } else if ( uobjtype==5 && usubtype==36 && uID==6  ) {  // _MCELLS_
      CELL_list   =  parts_rover;
	  } else if ( uobjtype==5 && usubtype==37 && uID==6  ) {  // _TCELLS_
      CELL_list   =  parts_rover;                           
	  } else
	    TCI_ASSERT(0);
    parts_rover =  parts_rover->next;
  }


  U16 rv_subtype  =  0;

  // \begin{cases} -> \MATRIX[c]{..}...
  // We have a special function to handle the cases variant

  if ( ilk_optparam && ilk_optparam->contents ) {
    TNODE* cont =  FindObject( ilk_optparam->contents,zNONLATEX,
                                INVALID_LIST_POS );
    if ( cont && cont->var_value ) {
      char ch =  cont->var_value[0];
      switch ( ch ) {
	      case 'c' :  // \begin{cases}
          return MATRIXtocases( MATRIX_node );
        break;
	      case 'm' :  // \begin{matrix}
          rv_subtype  =  710;   break;
	      case 's' :  // \begin{smallmatrix}
          rv_subtype  =  711;   break;
	      case 'p' :  // \begin{pmatrix}
          rv_subtype  =  712;   break;
	      case 'b' :  // \begin{bmatrix}
          rv_subtype  =  713;   break;
	      case 'v' :  // \begin{vmatrix}
          rv_subtype  =  714;   break;
	      case 'V' :  // \begin{Vmatrix}
          rv_subtype  =  715;   break;
	      case 'B' :  // \begin{Bmatrix}
          rv_subtype  =  716;   break;
        default :
          TCI_ASSERT(0);
        break;
      }

    } else
      TCI_ASSERT(0);
  }


// source
//\MATRIX<uID5.36.0>!\MATRIX!_ILK__COLSANDROWS__EXTALIGN_         _VR__HR__MCELLS_
//\TABLE <uID5.37.0>!\TABLE!      _COLSANDROWS__EXTALIGN__TWIDTH__VR__HR__TCELLS_
// all parts have uIDs 5.36.dd, except for CELL_list,
//  which is 5.37.dd in \TABLE

  U16 src_obj,src_sub,src_ID;
  GetUids( MATRIX_node->zuID,src_obj,src_sub,src_ID );

// destination
//\begin{array}   <uID5.35.0> !\begin{array}!   _AXALIGN__ACOLS__AROWS_!\end{array}!
//\begin{tabular} <uID5.490.0>!\begin{tabular}! _TXALIGN__TCOLS__TROWS_!\end{tabular}!
//\begin{tabular*}<uID5.491.0>!\begin{tabular*}!_TSWIDTH__TXALIGN__TCOLS__TSROWS_!\end{tabular*}!

  U16 dest_subtype  =  (src_sub==36) ? 35 : 490;	// array : tabular
  if ( width ) {
    TCI_ASSERT( src_sub==37 );
    dest_subtype  =  491;							// tabular*
  }

  if ( !rv_subtype )
    rv_subtype  =  dest_subtype;

// We've located the parts of \MATRIX, start building returned array.

  U8 dest_uID[32];
  UidsTozuID( 5,rv_subtype,0,(U8*)dest_uID );
  TNODE* rv =  MakeTNode( 0L,0L,0L,(U8*)dest_uID );	// \begin{whatever}

  dest_subtype  =  (src_sub==36) ? 35 : 490;

  TNODE* parts_head =  NULL;  // pointers for the "parts" list
  TNODE* parts_tail =  NULL;  //  that we're building

  if ( width ) {	// we're handling \TABLE -> \begin{tabular*}{width}
// _TSWIDTH_REQPARAM(5.490.1,DIMEN)
    parts_head    =  MakeTNode( 0L,0L,0L,(U8*)"5.490.1" );
    strcpy( (char*)parts_head->cuID,(char*)width->cuID );
    TNODE* dimen  =  width->contents;
	  if ( dimen ) {
	    DetachTList( dimen );
	    parts_head->contents  =  dimen;
	    dimen->sublist_owner  =  parts_head;
	  }
    parts_tail  =  parts_head;
  }

// The next part of \begin{array} is the optional external
//  alignment parameter - Lamport's "[pos]"
//  \begin{array][t|c|b]{clr}

  if ( ext_align_uID ) {	// _AXALIGN_OPTPARAM(5.35.2,MEXTALIGN)

  // ext_align_uID is t<uID11.4.1>, or c<uID11.4.2>, or b<uID11.4.3>
  // if the MATRIX is centered, we skip the optional arg

    if ( strcmp((char*)ext_align_uID,"11.4.2") ) {
    // make the bucket for the option parameter
      UidsTozuID( 5,dest_subtype,2,(U8*)dest_uID );
      TNODE* xa     =  MakeTNode( 0L,0L,0L,(U8*)dest_uID );
      TNODE* extalign_var   =  MakeTNode( 0L,0L,0L,ext_align_uID );
	    xa->contents  =  extalign_var;
	    extalign_var->sublist_owner =  xa;
	    if ( parts_tail ) {
	      parts_tail->next  =  xa;
		    xa->prev    =  parts_tail;
	    } else
	      parts_head  =  xa;
      parts_tail  =  xa;
	  }
  }

  // the next part of "\begin{array}" is the required "cols" parameter

  TNODE* COLS_reqparam  =  NULL;

  U16* global_vrules  =  (U16*)TCI_NEW( char[(ncols+1) * sizeof(U16)] );
  U16 gvi =  0;
  while ( gvi <= ncols )
    global_vrules[gvi++]  =  0;

  U16* global_lcrs =  (U16*)TCI_NEW( char[(ncols+1) * sizeof(U16)] );
  gvi =  0;
  while ( gvi <= ncols )
    global_lcrs[gvi++]  =  1;

  if ( VR_list ) {		// \VR{1,,r,,,}{,,r,,,}{,@{--},l,,,}{,,,p{1.25in},,}{1,,,,,}

// _MVR_!\VR!LIST(5.36.2,_COLATTRIBS_,5.36.3,{,\HR,)
// _COLATTRIBS_BUCKET(5.37.29,COLS,,},,)			
											  		
// Make the main bucket node 				  		
											  		
    UidsTozuID( 5,dest_subtype,3,(U8*)dest_uID );
    COLS_reqparam  =  MakeTNode( 0L,0L,0L,(U8*)dest_uID ); 		
    strcpy( (char*)COLS_reqparam->cuID,"70.7.0" );	
	  TNODE* COLS_tail  =  NULL;

	  U16 curr_VR =  0;
    TNODE* VR_rover =  VR_list->parts;
	  while ( VR_rover && curr_VR < ncols+1 ) {	// loop thru source VR_list entries

	    U16 state =  0;
	    TNODE* vr_rover =  VR_rover->parts->contents;
	    while ( vr_rover ) {			// loop thru tokens in COLS bucket
        TNODE* COLS_element =  NULL;

        U16 uobj,usub,ID;
        GetUids( vr_rover->zuID,uobj,usub,ID );
	      if ( uobj==3 && usub==17 && ID==87 ) {	  //<5.36.2>	VRs
		      state++;								  //  <5.36.2:0>
		      vr_rover =  vr_rover->next;			  //    <5.37.29>
		    } else {								  //      <3.3.2>1   #l_verts
												  //      <3.17.87>, @{text}
		      switch ( state ) {					  //      <3.17.87>, l|c|r
		        case  0 : {							  //      <16.4.3>r
	            TCI_ASSERT( uobj==3 && usub==3 );	  //      <3.17.87>, p{wd}
												  //      <3.17.87>, dummy1
              U16 n_verts =  ID-1;				  //      <3.17.87>, dummy2
												  //  <5.36.2:1>
              global_vrules[curr_VR]  =  n_verts;

			        TNODE* tail;
			        while ( n_verts ) {
                TNODE* vert =  MakeTNode( 0L,0L,0L,(U8*)"16.4.4" );
			          if ( COLS_element ) {
			            tail->next  =  vert;
				          vert->prev  =  tail;
			          } else
                  COLS_element  =  vert;
			          tail  =  vert;
			          n_verts--;
			        }

		          vr_rover =  vr_rover->next;
		        }
			      break;
			      case  1 : {
              if ( uobj==16 && usub==4 && ID==5 ) {		// @{text}
                COLS_element  =  MakeTNode( 0L,0L,0L,vr_rover->zuID );
			          TNODE* parts  =  vr_rover->parts;
                DelinkTNode( parts );
                COLS_element->parts   =  parts;
			          parts->sublist_owner  =  COLS_element;
    		        vr_rover =  vr_rover->next;
		          } else
		            TCI_ASSERT(0);
		        }
			      break;
			      case  2 : {									// l|c|r
	            TCI_ASSERT( uobj==16 && usub==4 );
              global_lcrs[curr_VR]  =  ID;
              COLS_element  =  MakeTNode( 0L,0L,0L,vr_rover->zuID );
    		      vr_rover =  vr_rover->next;
		        }
			      break;
			      case  3 : {
	            if ( uobj==16 && usub==4 && ID==6 ) {		// p{wd}16.4.6
                COLS_element  =  MakeTNode( 0L,0L,0L,vr_rover->zuID );
			          TNODE* parts  =  vr_rover->parts;
                DelinkTNode( parts );
                COLS_element->parts   =  parts;
			          parts->sublist_owner  =  COLS_element;
    		        vr_rover =  vr_rover->next;
		          } else
		            TCI_ASSERT(0);
		        }
			      break;
			      case  4 :
			      case  5 : {
              TCI_ASSERT(0);
    		      vr_rover =  vr_rover->next;
			      }
			      break;
			      default :
		          TCI_ASSERT(0);
    		      vr_rover =  vr_rover->next;
			      break;
		      }
    // Add the new node to the contents of "COLS_bucket"

          if ( COLS_element ) {
            if ( COLS_tail ) {
	            COLS_tail->next    =  COLS_element;
		          COLS_element->prev =  COLS_tail;
	          } else {
              COLS_reqparam->contents     =  COLS_element;
		          COLS_element->sublist_owner =  COLS_reqparam;
	          }
	          COLS_tail =  COLS_element;
		        while ( COLS_tail->next )
		          COLS_tail =  COLS_tail->next;
	        }
		    }
	    }			// while ( vr_rover )

	    curr_VR++;
	    VR_rover  =  VR_rover->next;
	  }		// loop thru source VR_list entries

	  TCI_ASSERT( curr_VR == ncols+1 );

  // Add the new "COLS_reqparam" to new_parts

    if ( parts_tail ) {
	    parts_tail->next    =  COLS_reqparam;
	    COLS_reqparam->prev =  parts_tail;
	  } else
	    parts_head  =  COLS_reqparam;

	  parts_tail =  COLS_reqparam;

  } else			// \VR not found??
	  TCI_ASSERT(0);

/*	We're mapping TO the following syntax
\begin{array}<uID5.35.0>!\begin{array}!_AXALIGN__ACOLS__AROWS_!\end{array}!
;;_TSWIDTH_REQPARAM(5.491.1,DIMEN)		no width
_AXALIGN_OPTPARAM(5.35.2,MEXTALIGN)
_ACOLS_REQPARAM(5.35.3,COLS)
_AROWS_LIST(5.35.9,_ALINE_,5.35.10,,\end{array},\\)
_ALINE__HRULES__ALINECELLS_
_ALINECELLS_LIST(5.35.11,_ACELL_,5.35.12,,\\|\end{array},&)
_HRULES_LIST(5.490.44,_HLINE_,5.490.45,\hline|\cline,,hline)
_HLINE_IF(TEXT,?\hline?,9.9.0)elseIF(?\cline?,5.90.0)ifEND
_ACELL_IF(TEXT,?\multicolumn?,5.412.0)ELSE(_ARRBUCKET_)ifEND
_ARRBUCKET_BUCKET(5.35.8,MATH,,,&|\\|\end{array},)
*/

// The tree generated by a direct parse of a \TABLE's CELLs list
// is NOT 1-to-1 with the LaTeX that we need to generate.  Here
// we generate a list of CELL_STRUCT's that can be adjusted and
// then used for the LaTeX generation.  The main problem lies
// with the handling of "COLS".

  CELL_STRUCT* CELL_slist =  MakeCellStructs( CELL_list->parts,
                                global_vrules,global_lcrs,ncols );
  AdjustCellStructs( CELL_slist,ncols );

  CELL_STRUCT* CELL_rover =  CELL_slist;
  TNODE* hline_rover  =  HR_reqparam->contents;

  UidsTozuID( 5,dest_subtype,9,(U8*)dest_uID );
  TNODE* rows_list  =  MakeTNode( 0L,0L,0L,(U8*)dest_uID );
  TNODE* rows_tail  =  NULL;

  U8 row_entryID[32];
  strcpy( (char*)row_entryID,(char*)dest_uID );
  strcat( (char*)row_entryID,":" );
  U16 zln =  strlen( (char*)row_entryID );

  U16 curr_row  =  0;
  while ( curr_row <= nrows ) {		  // loop down thru rows
                                    //  in the array being made.
    // Make TNODE for entry in row list

    // itoa( curr_row,(char*)row_entryID+zln,10 );
    sprintf((char*)row_entryID+zln, "%d", curr_row);

    TNODE* rows_entry =  MakeTNode( 0L,0L,0L,row_entryID );

    // Add the new node to the parts of "rows_list"

    if ( rows_tail ) {
	    rows_tail->next   =  rows_entry;
	    rows_entry->prev  =  rows_tail;
	  } else {
      rows_list->parts  =  rows_entry;
	    rows_entry->sublist_owner =  rows_list;
	  }
	  rows_tail =  rows_entry;

  // Now for the parts of our new row list entry.
  //   First, we have a list of \hline's and/or \cline{}'s

    U16 hr_list_counter =  0;

    TNODE* hr_list    =  MakeTNode( 0L,0L,0L,(U8*)"5.490.44" );
	  rows_entry->parts =  hr_list;			// 1st part of entry
	  hr_list->sublist_owner  =  rows_entry;

    TNODE* hr_entry =  MakeTNode( 0L,0L,0L,(U8*)"5.490.44:0" );

	  hr_list->parts  =  hr_entry;
	  hr_entry->sublist_owner =  hr_list;

    TNODE* hr_tail  =  hr_entry;

//  TNODE* hline_rover  =  HR_reqparam->contents;
//  ",,[5pt]\hline,\hline\hline,,\cline\hline,,,"
	
    U16 h_advance;
    TNODE* ending_node  =  CountHLineTokens( hline_rover,h_advance );

	  while ( h_advance ) {		// there are some \hline, \cline nodes

      TCI_BOOL do_it  =  FALSE;

      U16 objtype,subtype,uID;
      GetUids( hline_rover->zuID,objtype,subtype,uID );
      if      ( objtype==9 && subtype== 9  && uID== 0 ) // \hline<uID9.9.0>
        do_it =  TRUE;
      else if ( objtype==5 && subtype==449 && uID== 0 ) // \cline<uID5.449.0>
        do_it =  TRUE;

      if ( do_it ) {

        if ( hr_list_counter ) {
	        U8 hr_entryID[16];						//  <5.490.44:0>	
	        strcpy( (char*)hr_entryID,"5.490.44:" );	//    <9.9.0>\hline	
	        
                // itoa( hr_list_counter,(char*)hr_entryID+9,10 );
		sprintf((char*)hr_entryID+9, "%d", hr_list_counter);
                TNODE* tmp =  MakeTNode( 0L,0L,0L,hr_entryID );
	        strcpy( (char*)tmp->cuID,"70.3.0" );
	        hr_tail->next   =  tmp;
	        tmp->prev =  hr_tail;

          hr_tail =  tmp;
		    }

        DetachTList( hline_rover );
        hr_tail->parts   =  hline_rover;
        hline_rover->sublist_owner =  hr_tail;

	      hr_list_counter++;
        hline_rover =  hline_rover->next;

		  } else {

// Accumulate TeX dimen from \\[50pt] here!

        TNODE* del  =  hline_rover;
        hline_rover =  hline_rover->next;
	      if ( hline_rover )
	        DetachTList( hline_rover );
	      if ( del ) {
	        DetachTList( del );
	        DisposeTNode( del );
	      }

		  }

      h_advance--;

	  }   // while ( h_advance )

    TCI_ASSERT( ending_node == hline_rover );

  // hline_rover points at the "," that ends the current \hlines

	  if ( hline_rover ) {
      TNODE* del  =  hline_rover;
      hline_rover =  hline_rover->next;
	    if ( hline_rover )
	      DetachTList( hline_rover );
	    if ( del ) {
	      DetachTList( del );
	      DisposeTNode( del );
	    }
	  }


    if ( curr_row < nrows ) {

  // Now we build the list of columns

      UidsTozuID( 5,dest_subtype,11,(U8*)dest_uID );
      TNODE* cols_list  =  MakeTNode( 0L,0L,0L,(U8*)dest_uID );
	    hr_list->next     =  cols_list;
	    cols_list->prev   =  hr_list;

// If we have a dimen from \\[50pt], attach as cols_list->next
//    TNODE* dimen  =  MakeTNode( 0L,0L,0L,(U8*)"9.5.8" );
//    cols_list->next   =  dimen;
//    dimen->prev   =  cols_list;


	    U8 col_entryID[32];
	    strcpy( (char*)col_entryID,(char*)dest_uID );
	    strcat( (char*)col_entryID,":" );
	    U16 zln1  =  strlen( (char*)col_entryID );
	
	    TNODE* cols_tail  =  NULL;

	    U16 curr_col  =  0;
	    while ( curr_col < ncols && CELL_rover ) {

              // Make TNODE for entry in column list

	      // itoa( curr_col,(char*)col_entryID+zln1,10 );
              sprintf((char*)col_entryID+zln1, "%d", curr_col);
        TNODE* cols_entry =  MakeTNode( 0L,0L,0L,col_entryID );

      // Add the new node to the parts of "cols_list"

        if ( cols_tail ) {
	        cols_tail->next   =  cols_entry;
		      cols_entry->prev  =  cols_tail;
	      } else {
          cols_list->parts  =  cols_entry;
		      cols_entry->sublist_owner =  cols_list;
	      }
	      cols_tail =  cols_entry;

	      TNODE* cell_bucket  =  MATRIXcellToArray( CELL_rover,curr_col,
	  		COLS_reqparam,global_vrules,global_lcrs,dest_subtype );

	      cols_entry->parts   =  cell_bucket;
	      cell_bucket->sublist_owner  =  cols_entry;

	      curr_col    +=  CELL_rover->ncolumns_spanned;
	      CELL_rover  =  CELL_rover->next;

	    }		// loop thru columns in a row

      curr_row++;

    // We may iterate once more to handle hrules at the bottom
      if ( curr_row == nrows ) {
        U16 h_advance;
        TNODE* ending_node  =  CountHLineTokens( hline_rover,h_advance );
	      if ( h_advance==0 ) break;
	    }

	  } else		// Here we skip the last row when it has
	    break;    //  only hrules - bottom rules.

  }		// while ( curr_row <= nrows ) - loop down thru rows


  DisposeCellStructs( CELL_slist );
  delete global_vrules;
  delete global_lcrs;

  // Append rows_list to new_parts

  if ( parts_tail ) {
	  parts_tail->next  =  rows_list;
	  if ( rows_list )
	    rows_list->prev =  parts_tail;
  } else
    parts_head =  rows_list;

  parts_tail  =  rows_list;

// Append our parts_head list to rv

  rv->parts =  parts_head;
  if ( parts_head )
	  parts_head->sublist_owner  =  rv;

  return rv;
}


/*
; Productions for \EQN
\CELL<uID5.39.5>
\RD<uID5.39.6>

\EQN<uID5.39.0>!\EQN!_ENUMID__NCOLS__MLLABEL__MATHLETTERS__EQNROWS_
_ENUMID_REQPARAM(5.39.1,NONLATEX)
_NCOLS_REQPARAM(5.39.13,NONLATEX)
_MLLABEL_REQPARAM(5.39.15,NONLATEX)
_MATHLETTERS_REQPARAM(5.39.7,NONLATEX)
_EQNROWS_!{!LIST(5.39.2,_EQNROW_,5.39.3,\RD,},)!}!

_EQNROW_!{!_EQNCELLS__INTERTEXT_!}!_LINENUMSTYLE__LINETAG__LINELABEL__LINEEXTRADEPTH_
_EQNCELLS_LIST(5.39.26,_EQNBUCKET_,5.39.27,\CELL,,)
_INTERTEXT_IF(TEXT,?\intertext?,5.330.1)ifEND
_LINENUMSTYLE_REQPARAM(5.39.14,NONLATEX)
_LINETAG_REQPARAM(5.39.16,TEXT)
_LINELABEL_REQPARAM(5.39.17,NONLATEX)
_LINEEXTRADEPTH_REQPARAM(5.39.18,NONLATEX)

_EQNBUCKET_BUCKET(5.39.4,MATH,{,},,)
*/

TNODE* EQNtoeqnarray( TNODE* EQN_node,U16& LaTeX_ilk ) {

// Loop thru the parts of \EQN, recording data needed
//  for the translation to "\begin{eqnarray}" as we go.

  TNODE* ENUMID		  =  NULL;
  TNODE* NCOLS		  =  NULL;
  TNODE* MLLABEL	  =  NULL;
  TNODE* MATHLETTERS  =  NULL;
  TNODE* EQNROWS	  =  NULL;

  TNODE* parts_rover  =  EQN_node->parts;
  while ( parts_rover ) {
    U16 uobjtype,usubtype,uID;
    GetUids( parts_rover->zuID,uobjtype,usubtype,uID );
	  if ( uobjtype==5 && usubtype==39 ) {
	    if      ( uID==1  )
        ENUMID  =  parts_rover;
	    else if ( uID==13 )
        NCOLS	  =  parts_rover;
	    else if ( uID==15 )
        MLLABEL =  parts_rover;
	    else if ( uID==7  )
        MATHLETTERS =  parts_rover;
	    else if ( uID==2  )
        EQNROWS =  parts_rover;
	  } else
      TCI_ASSERT(0);
    parts_rover =  parts_rover->next;
  }

  TNODE* rv =  NULL;
  U16 LaTeX_uID =  0;
  U16 LaTeX_usubtype  =  0;
  if ( ENUMID && ENUMID->contents ) {
	  TNODE* enum_node  =  FindObject( ENUMID->contents,
	  							              zNONLATEX,INVALID_LIST_POS );
    U16 enum_ID     =  atoi( (char*)enum_node->var_value );
    U8 env_nom[32];
    LaTeX_usubtype  =  EQNenumIDtouID( enum_ID,(U8*)env_nom,
    											      LaTeX_uID );
    TCI_ASSERT( LaTeX_usubtype );
    LaTeX_ilk =  LaTeX_usubtype;

// Make the rv node - eqnarray, align, gather, etc.

    U8 eqn_uID[32];
    UidsTozuID( 5,LaTeX_usubtype,LaTeX_uID,(U8*)eqn_uID );
    rv =  MakeTNode( 0L,0L,0L,(U8*)eqn_uID );
	
	  strcpy( (char*)rv->src_tok,"\\begin{" );
	  strcat( (char*)rv->src_tok,(char*)env_nom );
	  strcat( (char*)rv->src_tok,"}" );
  } else
    TCI_ASSERT(0);


  if ( LaTeX_usubtype == TCMD_DisplayedMath ) {
    EQNtoEquation( EQNROWS,LaTeX_uID,rv );
	  return rv;
  }

// \begin{alignat}<uID5.125.0>!\begin{alignat}!REQPARAM(5.121.8,NONLATEX)_ALIGNATLINES_!\end{alignat}!
// _ALIGNATLINES_LIST(5.121.9,_ALIGNATCOLUMNS_,5.121.10,,\end{alignat},\\)
// _ALIGNATCOLUMNS_BUCKET(5.121.4,MATH,,,\\|\end{alignat},)

// \begin{align}<uID5.123.0>!\begin{align}!_ALIGNLINES_!\end{align}!
// _ALIGNLINES_LIST(5.121.9,_ALIGNLINE_,5.121.10,,\end{align},\\)
// _ALIGNLINE_IF(TEXT,?\intertext?,5.330.1)ifEND_ALIGNCELLS_
// _ALIGNCELLS_BUCKET(5.121.4,MATH,,,\\|\end{align},)

  TNODE* parts_tail =  NULL;

  if ( NCOLS && NCOLS->contents ) {

// alignat and it's variants have an extra REQPARAM
// \begin{alignat}{2}
//   x   & = 1  & \qquad y   & = 2 \\
//	 x+y & = 7	& \qquad y-z & = 11

	  TNODE* ncols_node =  FindObject( NCOLS->contents,
  								              zNONLATEX,INVALID_LIST_POS );
    //U16 ncols =  atoi( (char*)ncols_node->var_value );

    if ( LaTeX_usubtype >= 125 && LaTeX_usubtype <= 130 ) {
// Make an rv_parts node for ncols
      TNODE* cols =  MakeTNode( 0L,0L,0L,(U8*)"5.121.8" );
	    strcpy( (char*)cols->cuID,"70.11.0" );
	    TNODE* cont =  NCOLS->contents;
	    DetachTList( cont );
	    cols->contents  =  cont;
	    cont->sublist_owner =  cols; 

      rv->parts   =  cols;
      cols->sublist_owner =  rv;
      parts_tail  =  cols;
	  }
	
  } else
    TCI_ASSERT(0);

// Make the list-of-lines node 

  TNODE* lines_list  =  MakeTNode( 0L,0L,0L,(U8*)"5.121.9" );
  if ( parts_tail ) {
    parts_tail->next =  lines_list;
	  lines_list->prev =  parts_tail;
  } else {
    rv->parts =  lines_list;
    lines_list->sublist_owner =  rv;
  }
  parts_tail  =  lines_list;

// Process the list of RDs

  if ( EQNROWS ) {
	  U16 curr_row    =  0;
	  TNODE* rl_tail  =  NULL;
    TNODE* RD_rover =  EQNROWS->parts;	// 5.39.2:0
	  while ( RD_rover ) {	// loop thru source RD_list entries
  // Make TNODE for entry in rows list
	    U8 rows_entryID[16];
	    strcpy( (char*)rows_entryID,"5.121.9:" );
	    
            // itoa( curr_row,(char*)rows_entryID+8,10 );
            sprintf((char*)rows_entryID+8, "%d", curr_row);
      TNODE* rows_entry =  MakeTNode( 0L,0L,0L,rows_entryID );
  // Add the new node to the parts of "lines_list"
      if ( rl_tail ) {
	      rl_tail->next     =  rows_entry;
		    rows_entry->prev  =  rl_tail;
	    } else {
        lines_list->parts =  rows_entry;
		    rows_entry->sublist_owner =  lines_list;
	    }
	    rl_tail =  rows_entry;

	    TCI_BOOL is_intertext;
	    rows_entry->parts  =  ProcessEqnRD( RD_rover,is_intertext );
	    if ( rows_entry->parts ) {
	      rows_entry->parts->sublist_owner =  rows_entry;
	      if ( is_intertext && RD_rover->next ) {
	        RD_rover  =  RD_rover->next;
	        TNODE* part2  =  ProcessEqnRD( RD_rover,is_intertext );
		      rows_entry->parts->next =  part2;
		      part2->prev   =  rows_entry->parts;
	      }
	    }


	    curr_row++;
	    RD_rover  =  RD_rover->next;
	  }		// loop thru source RD_list entries

  } else
	  TCI_ASSERT(0);

  return rv;
}


/*
\EQN{}{}{}... maps to both eqnarrays and equation

\begin{equation*}<uID5.34.21>!\begin{equation*}!_ESBUCKET_!\end{equation*}!
_ESBUCKET_BUCKET(5.34.22,MATH,,,\end{equation*},)

; the equation environment has a built-in counter
\begin{equation}<uID5.34.41>!\begin{equation}!_EBUCKET_!\end{equation}!
_EBUCKET_BUCKET(5.34.42,MATH,,,\end{equation},)
*/

void EQNtoEquation( TNODE* EQN_RD_list,U16 uID,
                                TNODE* equation_node ) {

  if ( EQN_RD_list ) {
    TNODE* RD_0 =  EQN_RD_list->parts;	// no comments nodes
	  if ( RD_0 ) {
	    TCI_BOOL is_intertext;
	    TNODE* m_bucket =  ProcessEqnRD( RD_0,is_intertext );

  // Set m_bucket zuID for the MATH bucket of equation environment
      U8* bucket_ID =  (uID==42) ? (U8*)"5.34.42" : (U8*)"5.34.22";
      strcpy( (char*)m_bucket->zuID,(char*)bucket_ID );

      equation_node->parts    =  m_bucket;
	    m_bucket->sublist_owner =  equation_node;
	  }

  } else
	  TCI_ASSERT(0);
}


// \MATRIX{}...\CELL[,,,]{MATH}...
// Each \CELL maps to a MATH/TEXT bucket OR an instance of \multicolumn{}{}{}

TNODE* MATRIXcellToArray( CELL_STRUCT* CELL_rover,
                            U16 column_no,
                            TNODE* COLS_reqparam,
                            U16* global_vrules,
                            U16* global_lcrs,
                            U16 dest_subtype ) {

  TNODE* rv =  NULL;

  TNODE* cell_mb  =  CELL_rover->contents_bucket;
  CELL_rover->contents_bucket =  NULL;
  TCI_ASSERT( cell_mb );

// Do we a \multicolumn{}{}{} here?

  if ( CELL_rover->ncolumns_spanned > 1
  ||   !ColsMatch(CELL_rover,column_no,COLS_reqparam,global_vrules,global_lcrs) ) {

  // make the node for \multicolumn{ncolumns}{cols}{MATH bucket}
    rv  =  MakeTNode( 0L,0L,0L,(U8*)"5.412.0" );

  // {ncolumns}
    TNODE* rv_part1 =  MakeTNode( 0L,0L,0L,(U8*)"5.412.1" );
	  strcpy( (char*)rv_part1->cuID,"70.2.0" );	// TEXT
    rv->parts  =  rv_part1;
	  rv_part1->sublist_owner =  rv;
    TNODE* nc_word  =  MakeTNode( 0L,0L,0L,(U8*)"888.0.0" );
	  rv_part1->contents  =  nc_word;
	  nc_word->sublist_owner =  rv_part1;

  // fill in the var_value of node giving # of columns spanned
	  char buff[16];
	  
          //itoa( CELL_rover->ncolumns_spanned,buff,10 );
          sprintf(buff, "%d", CELL_rover->ncolumns_spanned);
	  U16 zln =  strlen( buff );
	  U8* tmp =  (U8*)TCI_NEW( char[zln+1] );
	  strcpy( (char*)tmp,buff );
	  nc_word->var_value  =  tmp;
	  nc_word->v_len  =  zln;

  // {cols}
    TNODE* rv_part2 =  MakeTNode( 0L,0L,0L,(U8*)"5.412.2" );
	  strcpy( (char*)rv_part2->cuID,"70.7.0" );	// COLS
    rv_part1->next  =  rv_part2;
	  rv_part2->prev  =  rv_part1;

  // generate "contents" for cols bucket, and ncolumns_done.
    TNODE* cont   =  CellColsToMultiColumn( CELL_rover,column_no );
	  rv_part2->contents  =  cont;
	  cont->sublist_owner =  rv_part2;

  // {MATH or TEXT bucket}
    TNODE* rv_part3 =  cell_mb;
	  strcpy( (char*)rv_part3->zuID,"5.412.3" );	// MATH or TEXT
    rv_part2->next  =  rv_part3;
	  rv_part3->prev  =  rv_part2;

  } else {
    rv  =  cell_mb;

    U8 dest_uID[32];	// over-write the uID as required
    UidsTozuID( 5,dest_subtype,8,(U8*)dest_uID );
	  strcpy( (char*)rv->zuID,(char*)dest_uID );
  }

  return rv;
}


// \hline,\hline\hline,,,\cline{2-4},...

TNODE* CountHLineTokens( TNODE* hline_rover,U16& tally ) {

  tally =  0;

  TNODE* hlr  =  hline_rover;
  while ( hlr ) {
    U16 objtype,subtype,uID;
    GetUids( hlr->zuID,objtype,subtype,uID );
    if        ( objtype==3 && subtype==17  && uID==87 ) {   // ,<uID3.17.87>
      break;
    } else if ( objtype==9 && subtype== 9  && uID== 0 ) {   // \hline<uID9.9.0>
      tally++;
    } else if ( objtype==5 && subtype==449 && uID== 0 ) {   // \cline<uID5.449.0>
      tally++;
    } else if ( objtype==3 && subtype==13  && uID== 9 ) {   // *<uID3.13.9>
      tally++;
    } else if ( objtype==3 && subtype==17  && uID==94 ) {   // [<uID3.17.94>

// [5pt],
      while ( hlr ) {
        tally++;
        U16 uobj,usub,id;
        GetUids( hlr->zuID,uobj,usub,id );
        if ( uobj==3 && usub==17 && id==96 )   // ]<uID3.17.96>
          break;
        else        
          hlr =  hlr->next;
      }

    } else              // unrecognized token in \HR{}
      TCI_ASSERT(0);

    if ( hlr )
      hlr =  hlr->next;
  }

  return hlr;
}


// Given a CELL_STRUCT,
//  generate cols for \multicolumn{ncolunms}{cols}{body MATH}.

TNODE* CellColsToMultiColumn( CELL_STRUCT* cell_info,
                                U16 column_no ) {

  TNODE* rv =  NULL;

// left verts

  U16 n_val =  cell_info->nleft_verts;
  while ( n_val ) {
    TNODE* vert =  MakeTNode( 0L,0L,0L,(U8*)"16.4.4" );
    rv  =  JoinTLists( rv,vert );
	n_val--;
  }

// lrc

  U8 zuID[32];
  UidsTozuID( 16,4,cell_info->lcr,(U8*)zuID );
  TNODE* lcr  =  MakeTNode( 0L,0L,0L,zuID );
  rv  =  JoinTLists( rv,lcr );

//<16.4.6>p
  if ( cell_info->p_width  ) {
    rv  =  JoinTLists( rv,cell_info->p_width );
    cell_info->p_width =  NULL;
  }

// at_text
  if ( cell_info->at_text ) {
    rv  =  JoinTLists( rv,cell_info->at_text );
    cell_info->at_text =  NULL;
  }

// It remains to add any required right verts

  U16 right_rules =  cell_info->nright_verts;
  while ( right_rules ) {
    TNODE* vert =  MakeTNode( 0L,0L,0L,(U8*)"16.4.4" );
    rv  =  JoinTLists( rv,vert );
	right_rules--;
  }

  return rv;
}


// The optional cols param in a \CELL[cols]{}
//  is used to override/redefine/? the global cols.  It may just
//  mimick the global data - in which case the \CELL should
//  not be mapped to a \multicolumn if it spans only 1 column.
// There is an issue regarding just what part of the global
//  COLS is replaced by a \CELL cols.  In Notebook, any verts
//  to the left of "l,c,r, or p{wd}" may be associated with
//  the cols for that particular column.  In LaTeX, these
//  same verts are considered to be right verts belonging
//  to the previous column.

bool ColsMatch( CELL_STRUCT* CELL_rover,
	 				  U16 column_no,
	 					TNODE* global_cols,
	 					  U16* global_vrules,
	 			            U16* global_lcrs  ) {

  bool rv =  true;		// assume cell cols match global cols

  if        ( CELL_rover->lcr != global_lcrs[column_no] ) {
    rv =  false;
  } else if ( CELL_rover->nright_verts != global_vrules[column_no+1] ) {
    rv =  false;
  } else if ( CELL_rover->nleft_verts != global_vrules[column_no] ) {
//    if ( column_no == 0 )
    rv =  false;
  } else if ( CELL_rover->at_text ) {
    rv =  false;
  } else if ( CELL_rover->p_width ) {
    rv =  SamePWidth( CELL_rover->p_width,column_no,global_cols );
  }

  return rv;

/*

// Locate left side of column_no in global_cols.
// c ||r l |\p{} |
// ^ ^   ^ ^
// 0 1   2 3

  TNODE* g_rover  =  global_cols->contents;
  U16 curr  =  0;
  if ( column_no > 0 ) {
    while ( curr < column_no && g_rover ) {
      U16 objtype,subtype,uID;
      GetUids( g_rover->zuID,objtype,subtype,uID );
	  TCI_ASSERT( objtype==16 && subtype==4 );
	  if ( uID == 1 || uID == 2 || uID == 3 || uID == 6 )
   // l,c,r,p{width}
	    curr++;
      g_rover =  g_rover->next;
    }
  }

  TCI_ASSERT( curr == column_no );

// compare

  while ( g_rover && cell_cols ) {
    U16 g_type,g_sub,g_ID;
    GetUids( g_rover->zuID,g_type,g_sub,g_ID );
	TCI_ASSERT( g_type==16 && g_sub==4 );

    U16 l_type,l_sub,l_ID;
    GetUids( cell_cols->zuID,l_type,l_sub,l_ID );
	TCI_ASSERT( l_type==16 && l_sub==4 );

	if ( l_ID != g_ID ) {
	  rv  =  false;
	  break;

	} else if ( l_ID == 5 ) {             // @{text}
	  if ( !SameAtText(g_rover,cell_cols) ) {
	    rv  =  false;
	    break;
	  }

	} else if ( l_ID == 6 ) {             // p{width}
	  if ( !SamePWidth(g_rover,cell_cols) ) {
	    rv  =  false;
	    break;
	  }

	}

    g_rover   =  g_rover->next;
    cell_cols =  cell_cols->next;
  }			// matching loop

  return rv;
*/
}


bool SameAtText( TNODE* cell_at,
						TNODE* global_at ) {

  bool rv =  false;
  return rv;
}


bool SamePWidth( TNODE* cell_p,
					  U16 column_no,
						TNODE* global_cols ) {

  bool rv =  false;
  return rv;
}


/*
typedef struct tagCELL_STRUCT {	// 
  tagCELL_STRUCT* next;         //  
  U16             ncolumns_spanned;
  U16             nrows_spanned;
  U16             nleft_verts;
  U16             nright_verts;
  U16             lcr;
  TNODE*          at_text;
  TNODE*          p_width;
  TNODE*          contents_bucket;
} CELL_STRUCT;
*/

CELL_STRUCT* MakeCellStructs( TNODE* CELL_entries_list,
									U16* global_vrules,
									  U16* global_lcrs,
									    U16 ncols ) {

  CELL_STRUCT* rv   =  NULL;
  CELL_STRUCT* tail =  NULL;

  U16 uobj,usub,id;
  GetUids( CELL_entries_list->zuID,uobj,usub,id );
  U8* zuID_cell_bucket  =  (usub==36) ? (U8*)"5.36.8"  : (U8*)"5.37.8";
  U8* zuID_cols_bucket  =  (usub==36) ? (U8*)"5.36.33" : (U8*)"5.37.33";

  U16 curr_column_no  =  0;
  TNODE* CELL_entry_rover =  CELL_entries_list;
  while ( CELL_entry_rover ) {

    CELL_STRUCT* curr_struct  =  (CELL_STRUCT*)TCI_NEW( char[sizeof(CELL_STRUCT)] );
	  curr_struct->next     =  NULL;
    curr_struct->nleft_verts  =  0;			// initialize
    curr_struct->nright_verts =  0;
    curr_struct->at_text  =  NULL;
    curr_struct->p_width  =  NULL;
    curr_struct->contents_bucket  =  NULL;

	  if ( !rv )  rv  =  curr_struct;			// append to rv
	  else		tail->next  =  curr_struct;
	  tail  =  curr_struct;

// source - CELL_entry_rover->parts
// _MCELLARGS_OPTPARAM(5.36.33,COLS)REQPARAM(5.36.8,MATH)
// _TCELLARGS_OPTPARAM(5.37.33,COLS)REQPARAM(5.37.8,TEXT)

// Locate \CELL[]{} REQPARAM - move to CELL_STRUCT

    TNODE* cell_bucket  =  FindObject( CELL_entry_rover->parts,
  							zuID_cell_bucket,INVALID_LIST_POS );
    TCI_ASSERT( cell_bucket );
    DelinkTNode( cell_bucket );
    curr_struct->contents_bucket  =  cell_bucket;

// Locate the OPTPARAM of \CELL[cols]{}.

    TNODE* cols_bucket  =  FindObject( CELL_entry_rover->parts,
  							zuID_cols_bucket,INVALID_LIST_POS );
    if ( cols_bucket ) {
      TNODE* cols_data  =  cols_bucket->contents;
      if ( cols_data ) {
        U16 state   =  0;
  // [ncolumns, nrows, nlv, nrv, ?, ?,l|c|r,@{text},p{wd}]
  //    ^        ^      ^    ^   ^  ^   ^      ^     ^
  //    0		 1      2    3   4  5   6      7     8
        TNODE* cols_rover =  cols_data;
        while ( cols_rover ) {		// loop thru "," separated objects

          U16 objtype,subtype,uID;
          GetUids( cols_rover->zuID,objtype,subtype,uID );
          if ( objtype==3 && subtype==17 && uID==87 ) {   // ,<uID3.17.87>
	        state++;
            cols_rover  =  cols_rover->next;

	      } else {		// process nodes in current state

            switch ( state ) { 
              case  0 :		// Here, we should be at a run of digits
    	      case  1 :		// ending with ","
	          case  2 :		//    <3.3.2>1
	          case  3 : {	//    <3.17.87>,
                U16 n_val =  0;
		        while ( true ) {
                  U16 objtype,subtype,uID;
                  GetUids( cols_rover->zuID,objtype,subtype,uID );
			      if ( objtype==3 && subtype==3 ) {	// a digit
			        n_val =  n_val*10 + uID-1;
                    cols_rover  =  cols_rover->next;
			      } else {
                    TCI_ASSERT( objtype==3 && subtype==17 && uID==87 );
			        break;
		          }
		        }		// while (true)

		        if        ( state == 0 ) {
                  curr_struct->ncolumns_spanned =  n_val;
		        } else if ( state == 1 ) {
		          TCI_ASSERT( n_val==1 );
                  curr_struct->nrows_spanned  =  n_val;
		        } else if ( state == 2 ) {
                  curr_struct->nleft_verts    =  n_val;
		        } else if ( state == 3 ) {
                  curr_struct->nright_verts   =  n_val;
		        }
	          }
	          break;

	          case  4 :
	          case  5 : {
		        TCI_ASSERT(0);		// I don't know what is kept here!
				// currently no fields in CELL_STRUCT for this data.
		        while ( true ) {		// skip over - advance to next ","
                  U16 objtype,subtype,uID;
                  GetUids( cols_rover->zuID,objtype,subtype,uID );
			      if ( objtype==3 && subtype==17 && uID==87 )
			        break;
			      else
                    cols_rover  =  cols_rover->next;
		        }
	          }
	          break;

	          case  6 : {		// <16.4.1>l, <16.4.2>c, <16.4.3>r
                if ( objtype==16 && subtype==4 ) {
                  curr_struct->lcr  =  uID;
                } else
                  TCI_ASSERT(0);
                cols_rover  =  cols_rover->next;
	          }
	          break;

	          case  7 : {	//<16.4.5>@
                if ( objtype==16 && subtype==4 && uID==5 ) {
			      TNODE* at   =  cols_rover;
                  cols_rover  =  cols_rover->next;
                  DelinkTNode( at );
                  curr_struct->at_text  =  at;
		        } else
		          TCI_ASSERT(0);
	          }
	          break;

	          case  8 : {	//<16.4.6>p
                if ( objtype==16 && subtype==4 && uID==6 ) {

/* I don't know what to do here.  The commented code below puts
transfers the p{width} to the CELL_STRUCT.
I don't think this valid - p{width}'s in \CELL[cols] are copied
from the \TABLE's COLS param.  I don't know if p{width} could
ever be valid in this context.  I'm discarding it for now.

			      TNODE* p_w  =  cols_rover;
                  cols_rover  =  cols_rover->next;
                  DelinkTNode( p_w );
                  curr_struct->p_width  =  p_w;
*/
                  cols_rover  =  cols_rover->next;
		        } else
		          TCI_ASSERT(0);
	          }
	          break;

	          default :
		        TCI_ASSERT(0);
	          break;
	        }

	      }			// object processing clause

        }		// while ( cols_rover )

        curr_column_no  +=  curr_struct->ncolumns_spanned;

	  } else			// cols_bucket is empty?
	    TCI_ASSERT(0);

    } else {
      curr_struct->ncolumns_spanned =  1;
      curr_struct->nrows_spanned    =  1;
      curr_struct->nleft_verts      =  global_vrules[curr_column_no];
      curr_struct->lcr              =  global_lcrs[curr_column_no];
      curr_column_no++;
      curr_struct->nright_verts     =  global_vrules[curr_column_no];
    }

	if ( curr_column_no == ncols )
	  curr_column_no  =  0;

    CELL_entry_rover  =  CELL_entry_rover->next;
  }			// loop thru entries in list of CELLs

  return rv;
}


void DisposeCellStructs( CELL_STRUCT* list ) {

  while ( list ) {
    CELL_STRUCT* del  =  list;
	list  =  list->next;

    if ( del->at_text ) {
	  TCI_ASSERT(0);
	  DisposeTList( del->at_text );
	}
    if ( del->p_width ) {
	  TCI_ASSERT(0);
	  DisposeTList( del->p_width );
	}
    if ( del->contents_bucket ) {
	  TCI_ASSERT(0);
	  DisposeTList( del->contents_bucket );
	}
    delete del;

  }		// while ( list )
}


// We've extracted all the info in the \CELLs list of a \TABLE.
// Here we try to deal with ambiguities regarding verts.
// Basically we shift left-verts from all cells but the first
//  in a row to right-verts on the previous cell.

void AdjustCellStructs( CELL_STRUCT* list,
												U16 n_columns ) {

//JBMLine( "CELLs before Adjust\n" );
//DumpCellStructs( list );

/*
  U16 curr_column_no =  0;
  U16 next_column_no;

  CELL_STRUCT* c_rover  =  list;
  while ( c_rover ) {
    next_column_no =  curr_column_no + c_rover->ncolumns_spanned;
	CELL_STRUCT* righty =  c_rover->next;

    if ( next_column_no == n_columns ) {	// we're at end of row
      curr_column_no  =  0;
	} else {
      c_rover->nright_verts =  righty->nleft_verts;
      righty->nleft_verts =  0;
      curr_column_no  =  next_column_no;
	}

	c_rover =  c_rover->next;
  }		// while ( list )
*/

//JBMLine( "CELLs after Adjust\n" );
//DumpCellStructs( list );
}


void DumpCellStructs( CELL_STRUCT* list ) {

  char zzz[256];

  U16 curr_column_no  =  0;
  while ( list ) {

sprintf( zzz,
"ncolumns_spanned=%d,\n nrows_spanned=%d,\n nleft_verts=%d,\n nright_verts=%d,\n lcr=%d\n",
list->ncolumns_spanned,list->nrows_spanned,list->nleft_verts,list->nright_verts,list->lcr );
JBMLine( zzz );

    list  =  list->next;
  }
}


// We translate from \EQN{enumID}... to \begin{eqnarray}, \begin{split},

U16 EQNenumIDtouID( U16 enum_ID,U8* env_nom,U16& LaTeX_uID ) {

  U16 rv  =  0;
  LaTeX_uID   =  0;
  char* zstr  =  NULL;
  switch ( enum_ID ) {
    case  0 :
      rv  =  TCMD_DisplayedMath;
      LaTeX_uID  =  41;
      zstr  =  "equation";
    break;	// \begin{equation}<uID5.34.41>
    case  1 : rv  =  121; zstr  =  "eqnarray";	break;	// eqnarray<uID5.121.0>
    case  2 : rv  =  123; zstr  =  "align";	    break;	// align<uID5.123.0>
    case  3 : rv  =  125; zstr  =  "alignat";	  break;  // alignat<uID5.125.0>
    case  4 : rv  =  131; zstr  =  "gather";	  break;	// gather<uID5.131.0>
    case  5 : rv  =  133; zstr  =  "multline";	break;	//  multline<uID5.133.0>
    case  6 :
      rv  =  TCMD_DisplayedMath;
      LaTeX_uID  =  21;
      zstr  =  "equation*";
    break;	// \begin{equation*}<uID5.34.21>
    case  7 : rv  =  122; zstr  =  "eqnarray*"; break;	// eqnarraySTAR<uID5.122.0>
    case  8 : rv  =  124; zstr  =  "align*";	  break;	// alignSTAR<uID5.124.0>
    case  9 : rv  =  126; zstr  =  "alignat*";	break;	// alignatSTAR<uID5.126.0>
    case 10 : rv  =  132; zstr  =  "gather*";	  break;	// gatherSTAR<uID5.132.0>
    case 11 : rv  =  134; zstr  =  "multline*"; break;	// multlineSTAR<uID5.134.0>
    case 12 : rv  =  127; zstr  =  "xalignat";	break;	// xalignat<uID5.127.0>
    case 13 : rv  =  129; zstr  =  "xxalignat"; break;	// xxalignat<uID5.129.0>
    case 14 : rv  =  141; zstr  =  "split";	    break;	// split<uID5.141.0>
    case 15 : rv  =  142; zstr  =  "gathered";	break;	// gathered<uID5.142.0>
    case 16 : rv  =  143; zstr  =  "aligned";	  break;	// aligned<uID5.143.0>
    case 17 : rv  =  144; zstr  =  "alignedat"; break;	// alignedat<uID5.144.0>
    default : break;				                    // xalignatSTAR<uID5.128.0>
  }									                    // xxalignatSTAR<uID5.130.0>
									                    // cases<uID5.140.0>
  if ( zstr )
    strcpy( (char*)env_nom,zstr );

  return rv;
}


// The internal format for an eqnarray is \EQN{}{}{}{}
//  {\RD{ListOfCELLs}...{}} 
// Each cell  generally contains something like "x+1 &=& 7".
// Note that we are rebuilding a LaTeX tree here.

TNODE* ProcessEqnRD( TNODE* RD_entry_node,
                            TCI_BOOL& is_intertext ) {

  TNODE* rv =  NULL;
  is_intertext  =  FALSE;

// Locate the key parts of the RD:0 list entry node
// _EQNROW_!{!_EQNCELLS__INTERTEXT_!}!_LINENUMSTYLE__LINETAG__LINELABEL__LINEEXTRADEPTH_
// _EQNCELLS_LIST(5.39.26,_EQNBUCKET_,5.39.27,\CELL,,)
// _EQNBUCKET_BUCKET(5.39.4,MATH,{,},,)
// _INTERTEXT_IF(TEXT,?\intertext?,5.330.1)ifEND

  TNODE* CELLSLIST      =  NULL;	// a list of \CELL{}s
  TNODE* INTERTEXT      =  NULL;	//   OR \intertext
  TNODE* LINENUMSTYLE   =  NULL;  // a NONLATEX bucket - 0,1,2,3
  TNODE* LINETAG	      =  NULL;	// a TEXT bucket	 - \tag{TEXT}
  TNODE* LINELABEL	    =  NULL;	// a NONLATEX bucket - \label{NONLATEX}
  TNODE* LINEEXTRADEPTH =  NULL;	// a NONLATEX bucket - [1.0in]

  TNODE* parts_rover  =  RD_entry_node->parts;
  while ( parts_rover ) {
    U16 uobjtype,usubtype,uID;
    GetUids( parts_rover->zuID,uobjtype,usubtype,uID );
	  if ( uobjtype==5 && usubtype==39 ) {
	    if      ( uID==26 )
        CELLSLIST       =  parts_rover;
	    else if ( uID==14 )
        LINENUMSTYLE	  =  parts_rover;
	    else if ( uID==16 )
        LINETAG         =  parts_rover;
	    else if ( uID==17 )
        LINELABEL       =  parts_rover;
	    else if ( uID==18 )
        LINEEXTRADEPTH  =  parts_rover;
	  } else if ( uobjtype==5 && usubtype== 330 && uID==1  ) {
      INTERTEXT =  parts_rover;
	  } else
      TCI_ASSERT(0);
    parts_rover =  parts_rover->next;
  }

  if ( CELLSLIST && CELLSLIST->parts ) {

// First we handle possible tag and label objects

    TNODE* tag_node   =  NULL;
    TNODE* label_node =  NULL;

    U16 line_attrs_ID =  0;
    if ( LINENUMSTYLE && LINENUMSTYLE->contents ) {
	    TNODE* attr_node  =  FindObject( LINENUMSTYLE->contents,
                                zNONLATEX,INVALID_LIST_POS );
      line_attrs_ID   =  atoi( (char*)attr_node->var_value );
	    if ( line_attrs_ID==0 ) {
// \notag<uID5.703.0>
        tag_node  =  MakeTNode( 0L,0L,0L,(U8*)"5.703.0" );
      }

    } else
      TCI_ASSERT(0);

    TCI_ASSERT( line_attrs_ID <= 3 );

    if ( LINETAG && LINETAG->contents ) {
/*
_LINETAG_REQPARAM(5.39.16,TEXT)
maps to
\TCItag<uID5.701.0>!\TCItag!REQPARAM(5.701.1,TEXT)
\TCItag*<uID5.702.0>!\TCItag*!REQPARAM(5.702.1,TEXT)
OR
\tag<uID5.701.0>!\tag!REQPARAM(5.701.1,TEXT)
\tag*<uID5.702.0>!\tag*!REQPARAM(5.702.1,TEXT)

\TCItag{} is our invention.  \tag{} is NOT legal
in LaTeX's eqnarray (and some other) environments.
*/
      TCI_ASSERT( line_attrs_ID >= 2 );
      TCI_ASSERT( tag_node==NULL );

      U16 usub  =  ( line_attrs_ID == 3 ) ? 702 : 701;
  U8 zuID[32];
  UidsTozuID( 5,usub,0,(U8*)zuID );
      tag_node    =  MakeTNode( 0L,0L,0L,(U8*)zuID );
  UidsTozuID( 5,usub,1,(U8*)zuID );
      TNODE* buck =  MakeTNode( 0L,0L,0L,(U8*)zuID );

	    strcpy( (char*)buck->cuID,"70.2.0" );
	    tag_node->parts =  buck;
	    buck->sublist_owner =  tag_node;

	    TNODE* cont =  LINETAG->contents;	// a list of TEXT
      DetachTList( cont );
	    buck->contents  =  cont;
    }

    if ( LINELABEL && LINELABEL->contents ) {
	    TNODE* nlx  =  FindObject( LINELABEL->contents,
                                zNONLATEX,INVALID_LIST_POS );
/*
_LINELABEL_REQPARAM(5.39.17,NONLATEX)
maps to
\label<uID5.700.0>!\label!REQPARAM(5.700.1,NONLATEX)
*/
      if ( nlx && nlx->var_value ) {
  // the current line has a label
        label_node  =  MakeTNode( 0L,0L,0L,(U8*)"5.700.0" );
        TNODE* buck =  MakeTNode( 0L,0L,0L,(U8*)"5.700.1" );
	      strcpy( (char*)buck->cuID,"70.11.0" );
	      label_node->parts   =  buck;
	      buck->sublist_owner =  label_node;

        TNODE* cont =  LINELABEL->contents;
        DetachTList( cont );
	      buck->contents  =  cont;
	    }
    }

//   Make the mbucket node for the cell(s) contents in one line.

    rv =  MakeTNode( 0L,0L,0L,(U8*)"5.121.4" );
    strcpy( (char*)rv->cuID,"70.3.0" );

    TNODE* contents_head  =  NULL;
	  TNODE* contents_tail  =  NULL;

	  U16 cell_counter    =  0;
    TNODE* cells_rover  =  NULL;
	  while ( 1 ) {	// loop thru cells in one line

	    cells_rover  =  FindObject( CELLSLIST->parts,
  							CELLSLIST->zuID,cell_counter );
	    if ( cells_rover )
	      cell_counter++;
	    else
	  	  break;

      if ( cell_counter > 1 ) {
        TNODE* amp  =  MakeTNode( 0L,0L,0L,(U8*)"3.17.106" );
	      if ( contents_tail ) {
		      contents_tail->next =  amp;
		      amp->prev =  contents_tail;
		    } else
		      contents_head =  amp;
		    contents_tail =  amp;
	    }

      TNODE* contents_rover =  cells_rover->parts->contents;
      if ( contents_rover ) {
        DetachTList( contents_rover );

	      if ( contents_tail ) {
		      contents_tail->next   =  contents_rover;
		      contents_rover->prev  =  contents_tail;
		    } else
		      contents_head =  contents_rover;
		    contents_tail =  contents_rover;
		    while ( contents_tail->next )
		      contents_tail  =  contents_tail->next;
	    }

	  }	// loop thru cells in one line

  // If we've generated a tag or a label, they are appended
  //  to the contents of the line here.

    if ( tag_node || label_node ) {
	    if ( contents_tail ) {
	      if ( tag_node ) {
		      contents_tail->next  =  tag_node;
		      tag_node->prev  =  contents_tail;
		      contents_tail   =  tag_node;
	      }
	      if ( label_node ) {
		      contents_tail->next  =  label_node;
		      label_node->prev  =  contents_tail;
		    }
	    } else {
	      if ( tag_node )
		      contents_head  =  tag_node;
	      if ( label_node ) {
		      if ( contents_head ) {
		        contents_head->next =  label_node;
		        label_node->prev    =  contents_head;
		      } else
		        contents_head =  label_node;
		    }
	    }
	    tag_node    =  NULL;
	    label_node  =  NULL;
	  }		// if ( tag_node || label_node )


    if ( contents_head ) {
	    rv->contents  =  contents_head;
	    contents_head->sublist_owner =  rv;
	  }

// \\*[1.10in] as the continuation cmd

    if ( LINEEXTRADEPTH && LINEEXTRADEPTH->contents ) {
	    TNODE* dimen_node =  FindObject( LINEEXTRADEPTH->contents,
                                    zNONLATEX,INVALID_LIST_POS );
  // 888.8.0 -> v_v = "[1.0in]" - NONLATEX - a DIMEN in brackets

      if ( dimen_node->var_value ) {
        char* nlx =  (char*)dimen_node->var_value;
        char* pl  =  strchr( nlx,'[' );
        char* pr  =  pl ? strchr( pl,']' ) : NULL;
	      if ( pl && pr ) {		// located [ and ]

  // the current line ending \\[] has a non-empty OPTPARAM
  // \\<uID9.5.8>!\\!OPTPARAM(9.5.23,DIMEN)
  // \\*<uID9.5.9>!\\*!OPTPARAM(9.5.24,DIMEN)

          char* p_star    =  strchr( nlx,'*' );
		      bool is_starred =  ( p_star && p_star < pl ) ? true : false;

	        U16 ln    =  pr - pl;
          U8* dimen =  (U8*)TCI_NEW( char[ln] );
          strncpy( (char*)dimen,(char*)pl+1,ln-1 );
          dimen[ln-1] =  0;

          // allocate node for \\ object
	  const char* eoln_uID  =  is_starred ? "9.5.9" : "9.5.8";
          TNODE* eoln =  MakeTNode( 0L,0L,0L,(U8*)eoln_uID );
	        rv->next    =  eoln;		// link to cells list node
	        eoln->prev  =  rv;

    // allocate node for optparam of \\ object
	  const char* buck_uID  =  is_starred ? "9.5.24" : "9.5.23";
          TNODE* dimem_bucket =  MakeTNode( 0L,0L,0L,(U8*)buck_uID );
	        strcpy( (char*)dimem_bucket->cuID,"70.10.0" );
	        eoln->parts =  dimem_bucket;
	        dimem_bucket->sublist_owner =  eoln;

    // allocate node for contents of OPTPARAM of \\ object
          TNODE* cont =  MakeTNode( 0L,0L,0L,zDIMEN );
	        dimem_bucket->contents  =  cont;	// link contents to bucket
	        cont->sublist_owner =  dimem_bucket;

		      cont->var_value =  dimen;	// install var_value
		      cont->v_len =  ln-1;
	      } else
	        TCI_ASSERT(0);
	    }

    }		// if ( LINEEXTRADEPTH && LINEEXTRADEPTH->contents )


  } else if ( INTERTEXT && INTERTEXT->parts ) {

  //TNODE* group_bucket =  FindObject( INTERTEXT->parts,
  //							(U8*)"6.1.2",INVALID_LIST_POS );
  //if ( group_bucket->contents ) {
  //  TNODE* intertext_node =  FindObject( group_bucket->contents,
  //							(U8*)"5.330.1",INVALID_LIST_POS );
  //}

    is_intertext  =  TRUE;
    rv  =  MakeTNode( 0L,0L,0L,(U8*)"5.330.1" );
    TNODE* part1  =  INTERTEXT->parts;
    DetachTList( part1 );
	  rv->parts =  part1;
	  part1->sublist_owner =  rv;

  } else
	  TCI_ASSERT(0);


  return rv;
}



U32 TextAccentedCharToUnicode( U16 accent_uID,char base_char ) {

  U32 rv  =  0;

  switch ( accent_uID ) {
    case 1  :  // \^<uID4.2.1>!\^!REQPARAM(4.2.50,TEXT)
    {
      switch ( base_char ) {
        case 'A' :  rv  =  0xc2;  break;
        case 'a' :  rv  =  0xe2;  break;
        case 'O' :  rv  =  0xd4;  break;
        case 'o' :  rv  =  0xf4;  break;
        case 'E' :  rv  =  0xca;  break;
        case 'e' :  rv  =  0xea;  break;
        case 'U' :  rv  =  0xdb;  break;
        case 'u' :  rv  =  0xfb;  break;
        case 'I' :  rv  =  0xce;  break;
        case 'i' :  rv  =  0xee;  break;
//Latin Extended A
        case 'H' :  rv  =  0x124;  break;
        case 'h' :  rv  =  0x125;  break;
        case 'J' :  rv  =  0x134;  break;
        case 'j' :  rv  =  0x135;  break;
        case 'W' :  rv  =  0x174;  break;
        case 'w' :  rv  =  0x175;  break;
        case 'Y' :  rv  =  0x176;  break;
        case 'y' :  rv  =  0x177;  break;
        case 'C' :  rv  =  0x108;  break;
        case 'c' :  rv  =  0x109;  break;
        case 'G' :  rv  =  0x11c;  break;
        case 'g' :  rv  =  0x11d;  break;
        case 'S' :  rv  =  0x15c;  break;
        case 's' :  rv  =  0x15d;  break;
        default  :  break;
      }
    }
    break;
    case 2  :  // \v<uID4.2.2>  cup
    {
      switch ( base_char ) {
//Latin Extended A
        case 'C' :  rv  =  0x10c;  break;
        case 'c' :  rv  =  0x10d;  break;
        case 'D' :  rv  =  0x10e;  break;
        case 'd' :  rv  =  0x10f;  break;
        case 'E' :  rv  =  0x11a;  break;
        case 'e' :  rv  =  0x11b;  break;
        case 'L' :  rv  =  0x13d;  break;
        case 'l' :  rv  =  0x13e;  break;
        case 'N' :  rv  =  0x147;  break;
        case 'n' :  rv  =  0x148;  break;
        case 'R' :  rv  =  0x158;  break;
        case 'r' :  rv  =  0x159;  break;
        case 'S' :  rv  =  0x160;  break;
        case 's' :  rv  =  0x161;  break;
        case 'T' :  rv  =  0x164;  break;
        case 't' :  rv  =  0x165;  break;
        case 'Z' :  rv  =  0x17d;  break;
        case 'z' :  rv  =  0x17e;  break;
        default  :  break;
      }
    }
    break;
    case 3  :  // \~<uID4.2.3>
    {
      switch ( base_char ) {
        case 'N' :  rv  =  0xd1;  break;
        case 'n' :  rv  =  0xf1;  break;
        case 'A' :  rv  =  0xc3;  break;
        case 'a' :  rv  =  0xe3;  break;
        case 'O' :  rv  =  0xd5;  break;
        case 'o' :  rv  =  0xf5;  break;
//Latin Extended A
        case 'I' :  rv  =  0x128;  break;
        case 'i' :  rv  =  0x129;  break;
        case 'U' :  rv  =  0x168;  break;
        case 'u' :  rv  =  0x169;  break;
        default  :  break;
      }
    }
    break;
    case 4  :  // \'<uID4.2.4>
    {
      switch ( base_char ) {
        case  0  :  rv  =  0xb4;  break;
        case 'A' :  rv  =  0xc1;  break;
        case 'a' :  rv  =  0xe1;  break;
        case 'O' :  rv  =  0xd3;  break;
        case 'o' :  rv  =  0xf3;  break;
        case 'E' :  rv  =  0xc9;  break;
        case 'e' :  rv  =  0xe9;  break;
        case 'U' :  rv  =  0xda;  break;
        case 'u' :  rv  =  0xfa;  break;
        case 'I' :  rv  =  0xcd;  break;
        case 'i' :  rv  =  0xed;  break;
        case 'Y' :  rv  =  0xdd;  break;
        case 'y' :  rv  =  0xfd;  break;
//Latin Extended A
        case 'N' :  rv  =  0x143;  break;
        case 'n' :  rv  =  0x144;  break;
        case 'R' :  rv  =  0x154;  break;
        case 'r' :  rv  =  0x155;  break;
        case 'C' :  rv  =  0x106;  break;
        case 'c' :  rv  =  0x107;  break;
        case 'L' :  rv  =  0x139;  break;
        case 'l' :  rv  =  0x13a;  break;
        case 'Z' :  rv  =  0x179;  break;
        case 'z' :  rv  =  0x17a;  break;
        case 'S' :  rv  =  0x15a;  break;
        case 's' :  rv  =  0x15b;  break;
        default  :  break;
      }
    }
    break;
    case 5  :  // \`<uID4.2.5>
    {
      switch ( base_char ) {
        case 'A' :  rv  =  0xc0;  break;
        case 'a' :  rv  =  0xe0;  break;
        case 'O' :  rv  =  0xd2;  break;
        case 'o' :  rv  =  0xf2;  break;
        case 'E' :  rv  =  0xc8;  break;
        case 'e' :  rv  =  0xe8;  break;
        case 'U' :  rv  =  0xd9;  break;
        case 'u' :  rv  =  0xf9;  break;
        case 'I' :  rv  =  0xcc;  break;
        case 'i' :  rv  =  0xec;  break;
//Latin Extended A
        default  :  break;
      }
    }
    break;
    case 6  :  // \.<uID4.2.6>  dot above
    {
      switch ( base_char ) {
//Latin Extended A
        case 'G' :  rv  =  0x120;  break;
        case 'I' :  rv  =  0x130;  break;
        case 'g' :  rv  =  0x121;  break;
        case 'E' :  rv  =  0x116;  break;
        case 'e' :  rv  =  0x117;  break;
        case 'C' :  rv  =  0x10a;  break;
        case 'c' :  rv  =  0x10b;  break;
        case 'Z' :  rv  =  0x17b;  break;
        case 'z' :  rv  =  0x17c;  break;
        default  :  break;
      }
    }
    break;
    case 7  :  // \"<uID4.2.7>
    {
      switch ( base_char ) {
        case  0  :  rv  =  0xa8;  break;
        case 'A' :  rv  =  0xc4;  break;
        case 'a' :  rv  =  0xe4;  break;
        case 'O' :  rv  =  0xd6;  break;
        case 'o' :  rv  =  0xf6;  break;
        case 'E' :  rv  =  0xcb;  break;
        case 'e' :  rv  =  0xeb;  break;
        case 'U' :  rv  =  0xdc;  break;
        case 'u' :  rv  =  0xfc;  break;
        case 'I' :  rv  =  0xcf;  break;
        case 'i' :  rv  =  0xef;  break;
        case 'y' :  rv  =  0xff;  break;
//Latin Extended A
        case 'Y' :  rv  =  0x178;  break;
        default  :  break;
      }
    }
    break;
    case 8  :  // \u<uID4.2.8>
    {
      switch ( base_char ) {
//Latin Extended A
        case 'A' :  rv  =  0x102;  break;
        case 'a' :  rv  =  0x103;  break;
        case 'E' :  rv  =  0x114;  break;
        case 'e' :  rv  =  0x115;  break;
        case 'I' :  rv  =  0x12c;  break;
        case 'i' :  rv  =  0x12d;  break;
        case 'O' :  rv  =  0x14e;  break;
        case 'o' :  rv  =  0x14f;  break;
        case 'U' :  rv  =  0x16c;  break;
        case 'u' :  rv  =  0x16d;  break;
        case 'G' :  rv  =  0x11e;  break;
        case 'g' :  rv  =  0x11f;  break;
        default  :  break;
      }
    }
    break;
    case 9  :  // \=<uID4.2.9> overbar accent
    {
      switch ( base_char ) {
        case  0  :  rv  =  0xaf;   break;
//Latin Extended A
        case 'A' :  rv  =  0x100;  break;
        case 'a' :  rv  =  0x101;  break;
        case 'E' :  rv  =  0x112;  break;
        case 'e' :  rv  =  0x113;  break;
        case 'I' :  rv  =  0x12a;  break;
        case 'i' :  rv  =  0x12b;  break;
        case 'O' :  rv  =  0x14c;  break;
        case 'o' :  rv  =  0x14d;  break;
        case 'U' :  rv  =  0x16a;  break;
        case 'u' :  rv  =  0x16b;  break;
        default  :  break;
      }
    }
    break;
    case 10 :  // \vec<uID4.2.10>
    {
      switch ( base_char ) {
//Latin Extended A
        default  :  break;
      }
    }
    break;
    case 11 :  // \dddot<uID4.2.11>
    {
      switch ( base_char ) {
//Latin Extended A
        default  :  break;
      }
    }
    break;
    case 12 :  // \ocirc<uID4.2.12>
    {
      switch ( base_char ) {
//Latin Extended A
        default  :  break;
      }
    }
    break;
    case 13 :  // \c<uID4.2.13>
    {
      switch ( base_char ) {
        case  0  :  rv  =  0xb8;  break;
        case 'C' :  rv  =  0xc7;  break;
        case 'c' :  rv  =  0xe7;  break;
//Latin Extended A
        case 'G' :  rv  =  0x122;  break;
        case 'g' :  rv  =  0x123;  break;
        case 'T' :  rv  =  0x162;  break;
        case 't' :  rv  =  0x163;  break;
        case 'N' :  rv  =  0x145;  break;
        case 'n' :  rv  =  0x145;  break;
        case 'K' :  rv  =  0x136;  break;
        case 'k' :  rv  =  0x137;  break;
        case 'R' :  rv  =  0x156;  break;
        case 'r' :  rv  =  0x157;  break;
        case 'L' :  rv  =  0x13b;  break;
        case 'l' :  rv  =  0x13c;  break;
        case 'S' :  rv  =  0x15e;  break;
        case 's' :  rv  =  0x15f;  break;
        default  :  break;
      }
    }
    break;
    case 14 :  // \H<uID4.2.14>  ''
    {
      switch ( base_char ) {
//Latin Extended A
        case 'O' :  rv  =  0x150;  break;
        case 'o' :  rv  =  0x151;  break;
        case 'U' :  rv  =  0x170;  break;
        case 'u' :  rv  =  0x171;  break;
        default  :  break;
      }
    }
    break;
    case 15 :  // \k<uID4.2.15>
    {
      switch ( base_char ) {
//Latin Extended A
        case 'U' :  rv  =  0x172;  break;
        case 'u' :  rv  =  0x173;  break;
        case 'A' :  rv  =  0x104;  break;
        case 'a' :  rv  =  0x105;  break;
        case 'E' :  rv  =  0x118;  break;
        case 'e' :  rv  =  0x119;  break;
        case 'I' :  rv  =  0x12e;  break;
        case 'i' :  rv  =  0x12f;  break;
        default  :  break;
      }
    }
    break;
    case 16 :  // \r<uID4.2.16>
    {
      switch ( base_char ) {
//Latin Extended A
        case 'U' :  rv  =  0x16e;  break;
        case 'u' :  rv  =  0x16f;  break;
        default  :  break;
      }
    }
    break;
    case 17 :  // \t<uID4.2.17>
    {
      switch ( base_char ) {
//Latin Extended A
        default  :  break;
      }
    }
    break;
    case 18 :  // \d<uID4.2.18>
    {
      switch ( base_char ) {
//Latin Extended A
        default  :  break;
      }
    }
    break;
    case 19 :  // \b<uID4.2.19>
    {
      switch ( base_char ) {
//Latin Extended A
        default  :  break;
      }
    }
    break;
  
    default :
      TCI_ASSERT(0);
    break;
  }

  return rv;
}

/*
Latin1

\`{A} &  \DH  & \`{a}  &  \dh
\U{c0}  \U{d0}  \U{e0}  \U{f0}

\'{A} & \~{N} & \'{a}  & \~{n}
\U{c1}  \U{d1}  \U{e1}  \U{f1}

\^{A} & \`{O} & \^{a}  & \`{o}
\U{c2}  \U{d2}  \U{e2}  \U{f2}

\~{A} & \'{O} & \~{a}  & \'{o}
\U{c3}  \U{d3}  \U{e3}  \U{f3}

\"{A} & \^{O} & \"{a}  & \^{o}
\U{c4}  \U{d4}  \U{e4}  \U{f4}

 \AA  & \~{O} &  \aa   & \~{o}
\U{c5}  \U{d5}  \U{e5}  \U{f5}

 \AE  & \"{O} &  \ae   & \"{o}
\U{c6}  \U{d6}  \U{e6}  \U{f6}

\c{C} & $\times $ & \c{c} & $\div $
\U{c7}  $\times $}  \U{e7}  $\div $}

\`{E} &   \O  & \`{e}  &  \o 
\U{c8}  \U{d8}  \U{e8}  \U{f8}

\'{E} & \`{U} & \'{e}  & \`{u}
\U{c9}  \U{d9}  \U{e9}  \U{f9}

\^{E} & \'{U} & \^{e}  & \'{u}
\U{ca}  \U{da}  \U{ea}  \U{fa}

\"{E} & \^{U} & \"{e}  & \^{u}
\U{cb}  \U{db}  \U{eb}  \U{fb}

\`{I} & \"{U} & \`{\i} & \"{u}
\U{cc}  \U{dc}  \U{ec}  \U{fc}

\'{I} & \'{Y} & \'{\i} & \'{y}
\U{cd}  \U{dd}  \U{ed}  \U{fd}

\^{I} &  \TH  & \^{\i} &  \th
\U{ce}  \U{de}  \U{ee}  \U{fe}

\"{I} &  \ss  & \"{\i} & \"{y}
\U{cf}  \U{df}  \U{ef}  \U{ff}


Latin Extended A

\={A} & \DJ  &  \.{G} & \.{I} & \U{140} & \H{O} & \v{S} & \H{U} \\ 
\U{100} \U{110} \U{120} \U{130} \U{140}   \U{150} \U{160} \U{170}

\={a} & \dj  &  \.{g} & \i  &   \L  &   \H{o} & \v{s} & \H{u} \\ 
\U{101} \U{111} \U{121}    \i   \U{141} \U{151} \U{161} \U{171}

\u{A} & \={E} & \c{G} & \U{132}& \l  &   \OE  &  \c{T} & \k{U} \\ 
\U{102} \U{112} \U{122} \U{132}  \U{142} \U{152} \U{162} \U{172}

\u{a} & \={e} & \c{g} & \U{133}& \'{N} & \oe  &  \c{t} & \k{u} \\ 
\U{103} \U{113} \U{123} \U{133}  \U{143} \U{153} \U{163} \U{173}

\k{A} & \u{E} & \^{H} & \^{J} & \'{n} & \'{R} & \v{T} & \^{W} \\ 
\U{104} \U{114} \U{124} \U{134} \U{144} \U{154} \U{164} \U{174}

\k{a} & \u{e} & \^{h} & \^{\j} & \c{N} & \'{r} & \v{t} & \^{w} \\ 
\U{105} \U{115} \U{125} \^{\j}   \U{145} \U{155} \U{165} \U{175}

\'{C} & \.{E} & \U{126}& \c{K} & \c{n} & \c{R} & \U{166}& \^{Y} \\ 
\U{106} \U{116} \U{126}  \U{136} \U{146} \U{156} \U{166}  \U{176}

\'{c} & \.{e} & \U{127}& \c{k} & \v{N} & \c{r} & \U{167}& \^{y} \\ 
\U{107} \U{117} \U{127}  \U{137} \U{147} \U{157} \U{167}  \U{177}

\^{C} & \k{E} & \~{I} & \U{138} & \v{n} & \v{R} & \~{U} & \"{Y} \\ 
\U{108} \U{118} \U{128} \U{138}   \U{148} \U{158} \U{168} \U{178}

\^{c} & \k{e} & \~{\i} & \'{L} & \U{149}& \v{r} & \~{u} & \'{Z} \\ 
\U{109} \U{119} \U{129}  \U{139} \U{149}  \U{159} \U{169} \U{179}

\.{C} & \v{E} & \={I} & \'{l} & \NG  &  \'{S} & \={U} & \'{z} \\ 
\U{10a} \U{11a} \U{12a} \U{13a} \U{14a} \U{15a} \U{16a} \U{17a}

\.{c} & \v{e} & \={\i} & \c{L} & \ng  &  \'{s} & \={u} & \.{Z} \\ 
\U{10b} \U{11b} \U{12b}  \U{13b} \U{14b} \U{15b} \U{16b} \U{17b}

\v{C} & \^{G} & \u{I} & \c{l} & \={O} & \^{S} & \u{U} & \.{z} \\ 
\U{10c} \U{11c} \U{12c} \U{13c} \U{14c} \U{15c} \U{16c} \U{17c}

\v{c} & \^{g} & \u{\i} & \v{L} & \={o} & \^{s} & \u{u} & \v{Z} \\ 
\U{10d} \U{11d} \U{12d}  \U{13d} \U{14d} \U{15d} \U{16d} \U{17d}

\v{D} & \u{G} & \k{I} & \v{l} & \u{O} & \c{S} & \r{U} & \v{z} \\ 
\U{10e} \U{11e} \U{12e} \U{13e} \U{14e} \U{15e} \U{16e} \U{17e}

\v{d} & \u{g} & \k{i} & \U{13f}& \u{o} & \c{s} & \r{u} & 
\U{10f} \U{11f} \U{12f} \U{13f}  \U{14f} \U{15f} \U{16f}
*/


TNODE* MATRIXtocases( TNODE* MATRIX_node ) {

// Loop thru the parts of \MATRIX, recording pointers to the major
//  parts needed for the translation to "\begin{cases}" as we go.

// \MATRIX<uID5.36.0>!\MATRIX!_ILK__COLSANDROWS__EXTALIGN__VR__HR__MCELLS_

  TNODE* ilk_optparam =  NULL;// _ILK_OPTPARAM(5.36.34,TEXT)
  U16 nrows =  0;			        // _COLSANDROWS_REQPARAM(5.36.12,NONLATEX)
  U16 ncols =  0;			  
  U8* ext_align_uID   =  NULL;// _EXTALIGN_!{!VAR(5.36.1,MEXTALIGN,a,c,})!}!
  TNODE* VR_list      =  NULL;// _VR_!\VR!LIST(5.36.2,_COLATTRIBS_,5.36.3,{,\HR,)
                              // _COLATTRIBS_BUCKET(5.36.29,COLS,,},,)
  TNODE* HR_reqparam  =  NULL;// _HR_!\HR!REQPARAM(5.36.9,MATH)
  TNODE* CELL_list    =  NULL;// _MCELLS_LIST(5.36.6,_MCELLARGS_,5.36.7,\CELL,,)
							                // _MCELLARGS_OPTPARAM(5.36.33,COLS)REQPARAM(5.36.8,MATH)

  TNODE* parts_rover  =  MATRIX_node->parts;
  while ( parts_rover ) {
    U16 uobjtype,usubtype,uID;
    GetUids( parts_rover->zuID,uobjtype,usubtype,uID );
	  if        ( uobjtype==5 && usubtype==36 && uID==34 ) {  // _ILK_
      ilk_optparam  =  parts_rover;
	  } else if ( uobjtype==5 && usubtype==36 && uID==12 ) {	// _COLSANDROWS_
      TNODE* cont =  FindObject( parts_rover->contents,zNONLATEX,
      											INVALID_LIST_POS );
      TCI_ASSERT( cont );
      char* p =  (char*)cont->var_value;
      ncols   =  atoi( p );
      TCI_ASSERT( ncols==2 );
	    p =  strchr( p,',' );
      nrows   =  atoi( p+1 );
	  } else if ( uobjtype==5 && usubtype==36 && uID==1  ) {  // _EXTALIGN_
      ext_align_uID =  parts_rover->var_value;
	  } else if ( uobjtype==5 && usubtype==36 && uID==2  ) {  // _VR_
      VR_list =  parts_rover;
	  } else if ( uobjtype==5 && usubtype==36 && uID==9  ) {  // _HR_
      HR_reqparam =  parts_rover;
	  } else if ( uobjtype==5 && usubtype==36 && uID==6  ) {  // _MCELLS_
      CELL_list   =  parts_rover;
	  } else
	    TCI_ASSERT(0);
    parts_rover =  parts_rover->next;
  }

// source
//\MATRIX<uID5.36.0>!\MATRIX!_ILK__COLSANDROWS__EXTALIGN__VR__HR__MCELLS_
// all parts have uIDs 5.36.dd

  U16 src_obj,src_sub,src_ID;
  GetUids( MATRIX_node->zuID,src_obj,src_sub,src_ID );

// destination
// \begin{cases}<uID5.140.0>!\begin{cases}!_CASESLINES_!\end{cases}!
// _CASESLINES_LIST(5.140.9,_CASESLINE_,5.140.10,,\end{cases},\\|\\*)
// _CASESLINE__CASESLBUCKET_!&!_CASESRBUCKET_
// _CASESLBUCKET_BUCKET(5.140.5,MATH,,,&,\\|\\*|\end{cases})
// _CASESRBUCKET_BUCKET(5.140.7,MATH,,,\\|\\*|\end{cases},)


// We've located the parts of \MATRIX, start building returned cases tree.

  U8 dest_uID[32];
  UidsTozuID( 5,140,0,(U8*)dest_uID );
  TNODE* rv =  MakeTNode( 0L,0L,0L,(U8*)dest_uID );	// \begin{cases}

// \begin{cases} has no optional external
//    alignment parameter - Lamport's "[pos]"

  if ( ext_align_uID ) {	  // _EXTALIGN_!{!VAR(5.36.1,MEXTALIGN,a,c,})!}!
  // ext_align_uID is t<uID11.4.1>, or c<uID11.4.2>, or b<uID11.4.3>
  // For "cases"  the MATRIX must be centered
    if ( strcmp((char*)ext_align_uID,"11.4.2") )
	    TCI_ASSERT(0);
  }

// "\begin{cases}" has no required "cols" parameter

  if ( VR_list ) {		// \VR{1,,r,,,}{,,r,,,}{,@{--},l,,,}{,,,p{1.25in},,}{1,,,,,}

  } else			// \VR not found??
	  TCI_ASSERT(0);

// Make a list node for list of lines within the case schemata

  UidsTozuID( 5,140,9,(U8*)dest_uID );
  TNODE* rows_list  =  MakeTNode( 0L,0L,0L,(U8*)dest_uID );
  TNODE* rows_tail  =  NULL;

  U8 row_entryID[32];
  strcpy( (char*)row_entryID,(char*)dest_uID );
  strcat( (char*)row_entryID,":" );
  U16 zln =  strlen( (char*)row_entryID );

// loop down thru rows in the array being made.

  TNODE* CELL_rover =  CELL_list->parts;

  U16 curr_row  =  0;
  while ( curr_row < nrows ) {
  // Make TNODE for entry in row list

    // itoa( curr_row,(char*)row_entryID+zln,10 );
    sprintf((char*)row_entryID+zln, "%d", curr_row);
    TNODE* rows_entry =  MakeTNode( 0L,0L,0L,row_entryID );

  // Add the new node to the parts of "rows_list"

    if ( rows_tail ) {
	    rows_tail->next   =  rows_entry;
	    rows_entry->prev  =  rows_tail;
	  } else {
      rows_list->parts  =  rows_entry;
	    rows_entry->sublist_owner =  rows_list;
	  }
	  rows_tail =  rows_entry;


    TNODE* p_head =  NULL;
    TNODE* p_tail =  NULL;

    U16 column_no =  0;
    while ( column_no < ncols && CELL_rover ) {

// Locate \CELL[]{} REQPARAM - move to CELL_STRUCT

	    TNODE* cell_bucket  =  FindObject( CELL_rover->parts,
                                (U8*)"5.36.8",INVALID_LIST_POS );
      TCI_ASSERT( cell_bucket );
      DelinkTNode( cell_bucket );

// _CASESLINE__CASESLBUCKET_!&!_CASESRBUCKET_
// _CASESLBUCKET_BUCKET(5.140.5,MATH,,,&,\\|\\*|\end{cases})
// _CASESRBUCKET_BUCKET(5.140.7,MATH,,,\\|\\*|\end{cases},)

      if        ( column_no == 0 ) {
        p_head =  cell_bucket;
        strcpy( (char*)p_head->zuID,"5.140.5" );
      } else if ( column_no == 1 ) {
        p_tail =  cell_bucket;
        strcpy( (char*)p_tail->zuID,"5.140.7" );
	      p_head->next  =  p_tail;
	      p_tail->prev  =  p_head;
      }


	    CELL_rover  =  CELL_rover->next;
      column_no++;

	  }		// loop thru columns in a row


    rows_entry->parts =  p_head;
	  p_head->sublist_owner =  rows_entry;

    curr_row++;

  }		// while ( curr_row <= nrows ) - loop down thru rows


// Attach our rows_list list to rv

  rv->parts =  rows_list;
  if ( rows_list )
	  rows_list->sublist_owner  =  rv;

  return rv;
}

