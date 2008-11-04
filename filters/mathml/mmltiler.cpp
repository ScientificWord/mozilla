
/*
#define TT_S_ELEM_HEADER	  16
#define TT_S_ELEM_ENDER		  17
#define TT_A_ELEM_HEADER	  18
#define TT_A_ELEM_ENDER		  19
#define TT_ATOM_BODY		  20
#define TT_REQ_ELEM		      21
*/

/*
  Written September 98, JBM.  Object to translate a MathML parse tree
into a linear linked list of "tiles".  The final step in generating
MathML output from the filter is to arrange these tiles in lines
of output.
*/

#include "mmltiler.h"
#include "treegen.h"
#include "logfiler.h"
#include "grammar.h"
#include "tmpliter.h"

#include <string.h>
#include <stdlib.h>
//#include <bytearry.h>


#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



// Note the caller has constructed the destination grammar, and the
//  logfile manager.  Given a destination grammar, we can get names
//  and productions (ie templates) from uIDs.

MMLTiler::MMLTiler( Grammar* dest_mml_grammar ) {

  d_mml_grammar =  dest_mml_grammar;

  use_unicodes 	=  FALSE;

//;; output entities as unicodes
//<uID12.1.13>0  OR  1  OR  2

  U8* dest_zname;			// pointers returned by lookups
  U8* d_template;			// in d_mml_grammar
  if ( d_mml_grammar->GetGrammarDataFromUID( (U8*)"12.1.13",
                (U8*)"MMLCONTEXT",&dest_zname,&d_template ) ) {
    if ( d_template && *d_template ) {
      if ( *d_template != '0' )
        use_unicodes  =  TRUE;
	}
  }

  n_space[0]  =  0; 
  context_sp  =  0;                 // zero the context stack
}


MMLTiler::~MMLTiler() {

  TCI_ASSERT( context_sp == 0 );
}


// Configuration data comes from d_mml_grammar
//  only when MMLTiler is part of a testjig.
// When MMLTiler is a component of the DLL used during
//  Export to HTML, config data is passed in thru
//  the following 2 functions.

// MMLTiler sets up tags, so it needs the namespace prefix.
// Many MML TNODEs processed by MMLTiler carry both named
// entities and unicode entities.  Hence we need to know
// which from is required in the output.

void MMLTiler::SetNameSpace( U8* name_space ) {

  n_space[0]  =  0;

  if ( name_space ) {
    if ( strlen((char*)name_space) < 32 )
      strcpy( (char*)n_space,(char*)name_space );
    else
      TCI_ASSERT(0);
  } else {
    U8* dest_zname;			// pointers returned by lookups
    U8* d_template;			// in d_mml_grammar
    if ( d_mml_grammar->GetGrammarDataFromUID( (U8*)"12.1.2",
                (U8*)"MMLCONTEXT",&dest_zname,&d_template ) ) {

      if ( d_template )
        strcpy( (char*)n_space,(char*)d_template );
    }
  }
}


void MMLTiler::SetEntityMode( U8* entity_mode ) {

  if ( entity_mode ) {
    U16 tmp =  atoi( (char*)entity_mode );
    use_unicodes  =  (tmp>0) ? TRUE : FALSE;
  } else {
  }
}


/*
  The following function takes as input a parse tree in MathML form,
and generates the "output tiles" from this parse tree.
  It sets up a context and calls ListToTiles, which is recursive -
processing TNODEs in one list may make recursive calls to ListToTiles.
*/
                
TILE* MMLTiler::MMLtreeToTiles( TNODE* parse_tree,U16 lim ) {

  TILE* rv  =  NULL;

  TCI_BOOL omit =  FALSE;
  if ( parse_tree ) {
    if ( !parse_tree->next
	&&   !parse_tree->var_value
	&&   !parse_tree->var_value2
	&&   !parse_tree->parts
	&&   !parse_tree->contents )
    omit  =  TRUE;
  } else
    omit  =  TRUE;
// We may want prolog and epilog nodes
// The initial "context" for translation may be in question
//  not in this case - always MATH


  if ( parse_tree && !omit ) {
    U8 zcontext[ CONTEXT_NOM_LIM ];
    d_mml_grammar->GetDefaultContext( zcontext );

    if ( zcontext[0] )  PushContext( zcontext );
	    U16 error_code;
      rv  =  ListToTiles( parse_tree,lim,error_code );
    if ( zcontext[0] )  PopContext();
  }

  return rv;
}


/*
  Callers of "ListToTiles" generally push a context, make the call
    and then pop the context.
  This function traverses a list of TNODE's, catenating the nodes'
    translations to a list of "tiles" that is returned.
*/

TILE* MMLTiler::ListToTiles( TNODE* mml_obj_list,U16 lim,
										                      U16& error_code ) {

//DumpTList( mml_obj_list,NULL,0,0,TOKENIZER_MML );

  TILE* rv  =  NULL;		// variables used to build returned list
  TILE* tail;
  TILE* new_frag;

  U8* context =  context_sp ? context_stack[context_sp-1] : (U8*)NULL;

  U8* dest_zname;			// pointers returned by lookups
  U8* d_template;			// in d_mml_grammar

  error_code    =  0;
  TNODE* rover  =  mml_obj_list;
  while ( rover ) { // loop thru first level objects in "mml_obj_list"

    new_frag  =  NULL;

    if ( strncmp((char*)rover->zuID,"0.0.0",5) ) {      // an object node

      U16 uobjtype,usubtype,uID;
      GetUids( rover->zuID,uobjtype,usubtype,uID );
        
      switch ( uobjtype ) {

        case 1  : {   //Context Start
		      TCI_ASSERT(0);
/*
          U8* tc  =  d_mml_grammar->GetContextName( rover->zuID );
          if ( tc ) {
            PushContext( tc );
            context =  context_stack[ context_sp-1 ];
            if ( dest_zname && *dest_zname )
			        new_frag  =  TokenToTile( rover,dest_zname,TT_CONTEXT_START );
          } else {
            // no such context in d_mml_grammar?  
          }
*/
        }
        break;

        case 2  : {   //Context End
		      TCI_ASSERT(0);
/*
          U8* tc  =  d_mml_grammar->GetContextName( rover->zuID );
          if ( tc ) {
            PopContext();
            context =  ( context_sp > 0 ) ? context_stack[ context_sp-1 ] : (U8*)NULL;
            if ( dest_zname && *dest_zname )
		          new_frag  =  TokenToTile( rover,dest_zname,TT_CONTEXT_END );
          } else {
            // no such context in d_mml_grammar?  
          }
*/
        }
        break;

        case 3  : {   //Symbol

/*
mi<uID3.201.1>
mn<uID3.202.1>
mo<uID3.203.1>
mtext<uID3.204.1>
mspace<uID3.205.1>
ms<uID3.206.1>

or an entity
*/
          if ( d_mml_grammar->GetGrammarDataFromUID( rover->zuID,
                              context,&dest_zname,&d_template ) ) {

            if ( usubtype>=201 && usubtype<=206 ) {
			        new_frag  =  AtomicElementToTiles( rover,dest_zname,
			  										                      error_code );
			      } else {
			        TCI_ASSERT(0);
			        new_frag  =  EntityToTiles( rover,dest_zname,
			  										                error_code );
			      }

          } else {
		        TCI_ASSERT(0);
		    // TeX symbol not defined in MathML
		      }

	      }
        break;

		    case  9 : {
          if ( d_mml_grammar->GetGrammarDataFromUID( rover->zuID,
                              context,&dest_zname,&d_template ) ) {
            U8 zender[32];
            strcpy( (char*)zender,"<" );
            strcat( (char*)zender,(char*)n_space );
            strcat( (char*)zender,(char*)dest_zname );
            strcat( (char*)zender," />" );
            new_frag  =  MakeTILE( TT_ALIGNGROUP,zender );
	        } else {
		        TCI_ASSERT(0);
		      }
	      }
        break;

		    case 70 : {
		      TCI_ASSERT(0);
	      }
        break;

        default : {
          if ( d_mml_grammar->GetGrammarDataFromUID( rover->zuID,
                              context,&dest_zname,&d_template ) ) {

            if ( d_template && *d_template ) {
              U16 zt_len  =  strlen( (char*)d_template );
              new_frag  =  StructuredObjectToTiles( d_template,zt_len,
              										rover,error_code );

            } else if ( dest_zname && *dest_zname ) {

			        TCI_ASSERT(0);
			        new_frag  =  TokenToTile( rover,dest_zname,TT_GENERIC );

            } else {      // this symbol not available in d_mml_grammar
			        TCI_ASSERT(0);
            }

          } else
		        TCI_ASSERT(0);

        }
        break;

      }       // switch ( uobjtype )

    } else {    // node uID is "0.0.0"

    //UNCLASSIFIED bytes from the original source
      //dest_ba +=  rover->var_value;

	  TCI_ASSERT(0);
    }

    if ( new_frag ) {
	  if ( rv ) {		// append to end of existing list
	    tail->next      =  new_frag;
		new_frag->prev  =  tail;
	  } else
	    rv  =  new_frag;

	  tail  =  new_frag;
	  while ( tail->next )
	    tail  =  tail->next;
    }

    if ( lim == 1 )
	  break;
	else
      rover =  rover->next;

  }     // while loop thru first level objects in "mml_obj_list"


  return rv;
}


// In this implementation, a "context" or "environment" is just a zname
//  that is known to the grammar.  In general it should be a struct
//  or an object.  "Contexts" can have many attributes, including stores
//  of definitions that apply only in that instance of the context.

void MMLTiler::PushContext( U8* new_context ) { 

  if ( context_sp < CONTEXT_STACK_LIM ) {
    char* env =  TCI_NEW( char[ strlen((char*)new_context) + 1 ] );
    strcpy( env,(char*)new_context );
    context_stack[context_sp++] =  (U8*)env;
  } else
    TCI_ASSERT( 0 );
}


void MMLTiler::PopContext() {

  if ( context_sp > 0 ) {
    context_sp--;
    delete context_stack[context_sp];
    context_stack[context_sp] =  (U8*)NULL;;
  } else
    TCI_ASSERT( 0 );
}


/*
    When "ListToTiles()" encounters a non-atomic or structured object,
  the following function is called.  The object template is traversed.
  As template elements are encountered, their translations are appended
  to a list of tiles.

  template elements:

    TMPL_ELEMENT_LITERAL
    TMPL_ELEMENT_BUCKET
    TMPL_ELEMENT_reqELEMENT
    TMPL_ELEMENT_LIST
    TMPL_ELEMENT_ALTERNATION

    TMPL_SYNTAX_ERROR
    TMPL_ELEMENT_UNKNOWN

    TMPL_ELEMENT_VARIABLE

;General Layout Schemata

/mrow<uID5.750.0>
mrow<uID5.750.1>!mrow!BUCKET(5.750.2,MATH,,,/mrow)!/mrow!
/mfrac<uID5.1.3>
mfrac<uID5.1.0>!mfrac!reqELEMENT(5.1.1)reqELEMENT(5.1.2)!/mfrac!
/msqrt<uID5.60.3>
msqrt<uID5.60.0>!msqrt!BUCKET(5.60.2,MATH,,,/msqrt)!/msqrt!
/mroot<uID5.61.3>
mroot<uID5.61.0>!mroot!reqELEMENT(5.61.2)reqELEMENT(5.61.1)!/mroot!
/mstyle<uID5.600.3>
mstyle<uID5.600.0>!mstyle!BUCKET(5.600.2,MATH,,,/mstyle)!/mstyle!
/merror<uID5.601.3>
merror<uID5.601.0>!merror!BUCKET(5.601.2,MATH,,,/merror)!/merror!
/mpadded<uID5.602.3>
mpadded<uID5.602.0>!mpadded!BUCKET(5.602.2,MATH,,,/mpadded)!/mpadded!
/mphantom<uID5.603.3>
mphantom<uID5.603.0>!mphantom!BUCKET(5.603.2,MATH,,,/mphantom)!/mphantom!
/mfenced<uID5.70.3>
mfenced<uID5.70.0>!mfenced!_FENCEDITEMS_!/mfenced!
_FENCEDITEMS_LIST(5.70.11,reqELEMENT(5.70.10),5.70.12,,/mfenced,)


;Script and Limit Schemata

/msub<uID5.50.4>
msub<uID5.50.2>!msub!_SUBBASE__SUBSCRIPT_!/msub!
_SUBBASE_reqELEMENT(5.50.3)
_SUBSCRIPT_reqELEMENT(5.50.1)

/msup<uID5.51.4>
msup<uID5.51.2>!msup!reqELEMENT(5.51.3)_SUPERSCRIPT_!/msup!
_SUPERSCRIPT_reqELEMENT(5.51.1)

/msubsup<uID5.52.6>
msubsup<uID5.52.2>!msubsup!reqELEMENT(5.52.3)_SSSUB__SSSUPER_!/msubsup!
_SSSUB_reqELEMENT(5.52.4)
_SSSUPER_reqELEMENT(5.52.5)

/munder<uID5.53.4>
munder<uID5.53.2>!munder!reqELEMENT(5.53.3)_UNDERSCRIPT_!/munder!
_UNDERSCRIPT_reqELEMENT(5.53.1)

/mover<uID5.54.4>
mover<uID5.54.2>!mover!reqELEMENT(5.54.3)_OVERSCRIPT_!/mover!
_OVERSCRIPT_reqELEMENT(5.54.1)

/munderover<uID5.55.6>
munderover<uID5.55.2>!munderover!reqELEMENT(5.55.3)_UOSUB__UOSUPER_!/munderover!
_UOSUB_reqELEMENT(5.55.4)
_UOSUPER_reqELEMENT(5.55.5)

/mprescripts/<uID5.56.15>
mprescripts/<uID5.56.10>LIST(5.56.11,_LEFTSUBSUPPAIR_,5.56.12,,/mmultiscripts,)
_LEFTSUBSUPPAIR_reqELEMENT(5.56.13)reqELEMENT(5.56.14)

/mmultiscripts<uID5.56.11>
mmultiscripts<uID5.56.2>!mmultiscripts!reqELEMENT(5.56.3)_RIGHTSCRIPTS__LEFTSCRIPTS_!/mmultiscripts!
_RIGHTSCRIPTS_LIST(5.56.6,_SUBSUPPAIR_,5.56.7,,mprescripts/|/mmultiscripts,)
_SUBSUPPAIR_reqELEMENT(5.56.8)reqELEMENT(5.56.9)
_LEFTSCRIPTS_IF(MATH,?mprescripts/?,5.56.10)ifEND


;Tables and Matrices

/mtd<uID5.35.15>
mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd)!/mtd!

/mtr<uID5.35.14>
mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
_LISTOFCELLS_LIST(5.35.11,_MTCELL_,5.35.12,,/mtr|/mtable,)
_MTCELL_IF(MATH,?mtd?,5.35.14)ELSE(_SINGLEELEMCELL_)ifEND
_SINGLEELEMCELL_reqELEMENT(5.35.8)

/mtable<uID5.35.14>
mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
_LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)
_MTROW_IF(MATH,?mtr?,5.35.13)ELSE(_SINGLECELLROW_)ifEND
_SINGLECELLROW_reqELEMENT(5.35.8)

optoptpreqp
*/

TILE* MMLTiler::StructuredObjectToTiles( U8* ztemplate,
											U16 tmpl_len,
                                              TNODE* obj_node,
                                                U16& error_code ) {

  TILE* rv  =  NULL;		// variables used to build returned list
  TILE* tail;
  TILE* new_frag;

  U8 literal[64];
  literal[0]  =  0;

  TNODE* parts_list   =  obj_node->parts;
  TmplIterater* tmpl  =  TCI_NEW( TmplIterater(d_mml_grammar,
  									ztemplate,tmpl_len,logfiler) );

  U16 error_flag;
  U16 tmpl_elm_type;
  while ( tmpl_elm_type = tmpl->GetNextElement() ) {

    new_frag    =  NULL;
    error_flag  =  0;
    
    switch ( tmpl_elm_type ) {

      case TMPL_SYNTAX_ERROR      : {
        error_flag  =  BAD_TMPL_SYNTAX;
      }
      break;

      case TMPL_ELEMENT_UNKNOWN   : {       // Error logged by GetNextElement
        error_flag  =  UNKNOWN_TMPL_ELEMENT;
      }
      break;

      case TMPL_ELEMENT_BUCKET    : {       // located in parts_list
        U8* pID   =  tmpl->GetField( TFIELD_ID );
        U8* pENV  =  tmpl->GetField( TFIELD_env );

        TNODE* p_list =  FindObject( parts_list,pID,INVALID_LIST_POS );
        if ( *pENV ) PushContext( pENV );
		if ( p_list ) {
          U16 t_len;
          U8* tok   =  tmpl->GetField( TFIELD_start,t_len );
          if ( t_len ) {
			new_frag  =  BytesToTile( tok,t_len,TT_BUCKET_START );
			TCI_ASSERT(0);
		  }
          TILE* ct  =  ListToTiles( p_list->contents,0,error_code );
		  new_frag  =  AppendTILEs( new_frag,ct );

          tok   =  tmpl->GetField( TFIELD_end,t_len );
          if ( t_len ) {
			TILE* tbe =  BytesToTile( tok,t_len,TT_BUCKET_END );
  		    new_frag  =  AppendTILEs( new_frag,tbe );
			TCI_ASSERT(0);
		  }
		}
        if ( *pENV ) PopContext();
      }
      break;

      case TMPL_ELEMENT_LITERAL   : {       // copy it to literal
		if ( literal[0] == 0 ) {
	      U16 t_len;
          U8* tok   =  tmpl->GetField( TFIELD_lit,t_len );
		  strncpy( (char*)literal,(char*)tok+1,t_len-2 );
		  literal[ t_len-2 ]  =  0;
		}
      }
      break;

      case TMPL_ELEMENT_IF        : {

    // Loop thru "alternation" objects in current tmpl element
    // If found, we put a node for the object - if object has parts
    //  do the parts list.

        TCI_BOOL done  =  FALSE;
        U16 alter_elm_type  =  TMPL_ELEMENT_IF;
        while ( !done && alter_elm_type != TMPL_ELEMENT_ifEND ) {

          if ( alter_elm_type == TMPL_ELEMENT_ELSE ) {

			U16 tmpl_len;
            U8* ztemplate =  tmpl->GetField( TFIELD_else,tmpl_len );
            new_frag  =  StructuredObjectToTiles( ztemplate,tmpl_len,
                                              obj_node,error_code );
			done  =  TRUE;

          } else {

        // Search for the object called for by this "alternation"

            U8* zID   =  tmpl->GetField( TFIELD_ID );
            TNODE* on =  FindObject( parts_list,zID,INVALID_LIST_POS );

            if ( on ) {   // found
              TNODE* save =  on->next;
              on->next    =  (TNODE*)NULL;
              U8* zcontext  =  tmpl->GetField( TFIELD_env );
              if ( zcontext && zcontext[0] )  PushContext( zcontext );
                new_frag  =  ListToTiles( on,0,error_code );
              if ( zcontext && zcontext[0] )  PopContext();
              on->next    =  save;

              done  =  TRUE;
            }
		  }

          alter_elm_type  =  tmpl->GetNextElement();

        }       // while ( !done );

        if ( alter_elm_type != TMPL_ELEMENT_ifEND )
          if ( !tmpl->EndAlternation() )
            error_flag  =  999;

      }
      break;

      case TMPL_ELEMENT_reqELEMENT  :  {
        U8* pID   =  tmpl->GetField( TFIELD_ID );
        U8* pENV  =  tmpl->GetField( TFIELD_env );
        TNODE* p_list =  FindObject( parts_list,pID,INVALID_LIST_POS );
        if ( *pENV )
          PushContext( pENV );

        new_frag  =  ListToTiles( p_list->contents,0,error_code );

/*
		U16 elem_count  =  CountElements( new_frag );
		if ( elem_count>1 ) {
		  TILE* tbs =  BytesToTile( (U8*)"<mrow>",6,TT_S_ELEM_HEADER );
  		  new_frag  =  AppendTILEs( tbs,new_frag );

		  TILE* tbe =  BytesToTile( (U8*)"</mrow>",7,TT_S_ELEM_ENDER );
  		  new_frag  =  AppendTILEs( new_frag,tbe );
        }
*/

        if ( *pENV )
          PopContext();
      }
      break;

      case TMPL_ELEMENT_LIST      : {       // 

/*
\Sb<uID5.30.0>!\Sb!!{!LIST(5.30.1,BUCKET(5.30.3,MATH,{,},),\CELL,,},5.30.2)
*/

        U8* pID     =  tmpl->GetField( TFIELD_ID );
        TNODE* list_node  =  FindObject( parts_list,pID,INVALID_LIST_POS );

        U16 start_len;
        U8* pstart  =  tmpl->GetField( TFIELD_start,start_len );
        U16 cont_len;
        U8* pcont   =  tmpl->GetField( TFIELD_list_cont,cont_len );

        if ( list_node->parts ) {
          TNODE* items  =  list_node->parts;
          U16 list_pos  =  0;
          while ( TRUE ) {
            TNODE* list =  FindObject( items,pID,list_pos );

            if ( list ) {         // we have another item
              if ( start_len ) {
			    TCI_ASSERT(0);
                //d_mml_grammar->OutputBytes( pstart,start_len,dest_ba );
                new_frag  =  AppendTILEs( new_frag,
                	BytesToTile(pstart,start_len,TT_LIST_ITEM_START) );
              } else if ( list_pos > 0 && cont_len ) {
			    TCI_ASSERT(0);
                //d_mml_grammar->OutputBytes( pcont,cont_len,dest_ba );
                new_frag  =  AppendTILEs( new_frag,
                	BytesToTile(pcont,cont_len,TT_LIST_CONTINUE) );
			  }
            } else {              // end the list
              break;
            }

            U16 nest_len;
            U8* pnest   =  tmpl->GetField( TFIELD_nest,nest_len );
            new_frag  =  AppendTILEs( new_frag,
            	StructuredObjectToTiles(pnest,nest_len,list,error_code) );

            list_pos++;

          }       // while loop thru list items

        }	// if ( list_node->parts )

      }
      break;

      case TMPL_ELEMENT_VARIABLE  : {       // 
		TCI_ASSERT(0);

/*
TNODE* shit =  parts_list;
while ( shit ) {
JBMLine( (char*)shit->zuID );
JBMLine( "\n" );
shit = shit->next;
}
*/

        U8* pID     =  tmpl->GetField( TFIELD_ID );
        TNODE* n    =  FindObject( parts_list,pID,INVALID_LIST_POS );

        if ( n ) {
          U16 var_type  =  n->v_type;
          if        ( var_type == 'l' || var_type == 'n' ) {
            //d_mml_grammar->OutputBytes( n->var_value,n->v_len,dest_ba );
            new_frag  =  BytesToTile( n->var_value,n->v_len,TT_VAR_VALUE );

          } else if ( var_type == 'a' ) {

            if ( n->var_value ) {
/*
    char zzz[80];
    sprintf( zzz,"VAR value = %s\n",n->var_value );
    JBMLine( zzz );
*/
              U8* context =  tmpl->GetField( TFIELD_env );
              U8* dest_zname;
              U8* dest_ztemplate;
              if ( d_mml_grammar->GetGrammarDataFromUID( n->var_value,
                                  context,&dest_zname,&dest_ztemplate ) ) {
                U16 nln =  strlen( (char*)dest_zname );
                //d_mml_grammar->OutputBytes( dest_zname,nln,dest_ba );
                new_frag  =  BytesToTile( dest_zname,nln,TT_VAR_VALUEA );
              } else {
              }
            }

          } else {
/*
char zzz[80];
sprintf( zzz,"Transltr::UNKNOWN VAR type, %d\n",var_type );
JBMLine( zzz );
*/
	        TCI_ASSERT(0);
          }

        } else {
/*
char zzz[80];
sprintf( zzz,"Transltr::VAR not found, %s\n",pID );
JBMLine( zzz );
*/
	      TCI_ASSERT(0);
        }

      }
      break;

      default :
        TCI_ASSERT( 0 );
      break;

    }           //  switch ( tmpl_info.curr_atom_type )

    if ( error_flag )
      break;  

    if ( new_frag ) {
	  if ( rv ) {		// append to end of existing list
	    tail->next      =  new_frag;
		new_frag->prev  =  tail;
	  } else
	    rv  =  new_frag;

	  tail  =  new_frag;
	  while ( tail->next )
	    tail  =  tail->next;
    }

  }     // while loop thru template elements

  if ( literal[0] )
    rv  =  StartEndStructuredElement( rv,literal,obj_node );

  delete tmpl;

  return rv;
}



TILE* MMLTiler::TokenToTile( TNODE* src_node,
								U8* grammar_info,
									U16 token_type ) {
  TILE* rv  =  NULL;

  //d_mml_grammar->OutputToken( rover,dest_zname,dest_ba );

  rv  =  MakeTILE( token_type,grammar_info );
  rv->msg_list  =  src_node->msg_list;
  src_node->msg_list  =  NULL;

  return rv;
}


TILE* MMLTiler::AtomToTile( TNODE* src_node,
									U8* grammar_info,
										U16 token_type ) {
  TILE* rv  =  NULL;

  //d_mml_grammar->OutputAtom( rover,dest_zname,dest_ba );

  rv  =  MakeTILE( token_type,grammar_info );
  rv->msg_list  =  src_node->msg_list;
  src_node->msg_list  =  NULL;

  return rv;
}


TILE* MMLTiler::BytesToTile( U8* zbytes,U16 zln,
										U16 token_type ) {

  TILE* rv  =  NULL;

  //d_mml_grammar->OutputBytes( tok,t_len,dest_ba );

  U8 ztoken[80];
  U16 ti  =  0;
  while ( ti<zln ) {
    ztoken[ti] =  *zbytes;
    zbytes++;
    ti++;  
  }
  ztoken[zln]  =  0;

  rv  =  MakeTILE( token_type,ztoken );

  return rv;
}


TILE* MMLTiler::AtomicElementToTiles( TNODE* mml_atom_elem_node,
										    U8* dest_zname,
										    U16& error_code ) {

  TILE* rv  =  NULL;

  U8 zheader[1024];
  strcpy( (char*)zheader,"<" );
  strcat( (char*)zheader,(char*)n_space );
  strcat( (char*)zheader,(char*)dest_zname );
  AttrsToHeader( mml_atom_elem_node->attrib_list,(U8*)zheader );

  if ( mml_atom_elem_node->var_value ) {
    strcat( (char*)zheader,">" );
    rv  =  MakeTILE( TT_A_ELEM_HEADER,zheader );

    U8* chdata  =  mml_atom_elem_node->var_value;
    if ( use_unicodes && mml_atom_elem_node->var_value2 )
	  chdata  =  mml_atom_elem_node->var_value2;

    TILE* body  =  MakeTILE( TT_ATOM_BODY,chdata );

    U8 zender[32];
    strcpy( (char*)zender,"</" );
    strcat( (char*)zender,(char*)n_space );
    strcat( (char*)zender,(char*)dest_zname );
    strcat( (char*)zender,">" );
    TILE* ender =  MakeTILE( TT_A_ELEM_ENDER,zender );

    if ( body ) {
      rv->next    =  body;
	  body->prev  =  rv;
	  body->next  =  ender;
	  ender->prev =  body;
    } else {
      rv->next    =  ender;
	  ender->prev =  rv;
    }

  } else {
    //strcat( (char*)zheader,(char*)n_space );
    strcat( (char*)zheader," />" );
    rv  =  MakeTILE( TT_A_ELEM_HEADER,zheader );
  }

  rv->msg_list  =  mml_atom_elem_node->msg_list;
  mml_atom_elem_node->msg_list  =  NULL;

  return rv;
}


TILE* MMLTiler::EntityToTiles( TNODE* mml_entity_node,
									U8* dest_zname,
										U16& error_code ) {

  TILE* rv  =  NULL;

  return rv;
}


void MMLTiler::AttrsToHeader( ATTRIB_REC* attrib_list,
											U8* zheader ) {

  while ( attrib_list ) {
    strcat( (char*)zheader," " );
    strcat( (char*)zheader,(char*)attrib_list->attr_nom );
    strcat( (char*)zheader,"=\"" );
    strcat( (char*)zheader,(char*)attrib_list->z_val );
    strcat( (char*)zheader,"\"" );

    attrib_list =  attrib_list->next;
  }
}


TILE* MMLTiler::StartEndStructuredElement( TILE* body,
										  U8* elem_name,
										TNODE* mml_src_node ) {

  TILE* rv  =  NULL;

  U8 zheader[1024];
  strcpy( (char*)zheader,"<" );
  strcat( (char*)zheader,(char*)n_space );
  strcat( (char*)zheader,(char*)elem_name );
  AttrsToHeader( mml_src_node->attrib_list,(U8*)zheader );
  strcat( (char*)zheader,">" );
  rv  =  MakeTILE( TT_S_ELEM_HEADER,zheader );
  rv->msg_list  =  mml_src_node->msg_list;
  mml_src_node->msg_list  =  NULL;

  U8 zender[32];
  strcpy( (char*)zender,"</" );
  strcat( (char*)zender,(char*)n_space );
  strcat( (char*)zender,(char*)elem_name );
  strcat( (char*)zender,">" );
  TILE* ender =  MakeTILE( TT_S_ELEM_ENDER,zender );

  if ( body ) {
    rv->next    =  body;
	body->prev  =  rv;
	while ( body->next ) body =  body->next;
	body->next  =  ender;
	ender->prev =  body;
  } else {
    rv->next    =  ender;
	ender->prev =  rv;
  }

  return rv;
}


void MMLTiler::SetLogFiler( LogFiler* lf ) {

  logfiler  =  lf;
}
