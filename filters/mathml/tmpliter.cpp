
/*
        Implementation of "Template Iterater"

   I refer to grammar productions as "Templates". An example follows.

\matrix<uID5.35.0>!\matrix!IF(MATH,?\format?,5.36.0)ifEND_ROWS_!\endmatrix!
_ROWS_LIST(5.35.9,_COLUMNS_,5.35.10,,\endmatrix,\\)
_COLUMNS_LIST(5.35.11,_CELL_,5.35.12,,\\|\endmatrix,&)
_CELL_BUCKET(5.35.8,MATH,,,&|\\|\endmatrix)

     In parsing, arbitrating and translating we iterate thru templates,
   taking action(s) at each template element.  This object retrieves
   consecutive elements of a template for a client.

  Elements with names that start and end with _ are macros. (ie. _ROWS_)
These macros are not seen by clients of this object - they are
expanded internally.  Clients are given the elements that result
from this expansion.

*/

#include "tmpliter.h"
#include "grammar.h"
#include "logfiler.h"
#include "fltutils.h"


#include <string.h>
#include <stdlib.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Construct a "Template Iterater" from a template string and its length.
//   A Grammar is used to look up the definitions of template variables.

TmplIterater::TmplIterater( Grammar* TeX_grammar,U8* tmpl,U16 tln,
											LogFiler* the_logger ) {

  s_grammar =  TeX_grammar;
  logfiler  =  the_logger;

  // we keep a copy of the template's definition 

  ztmpl   =  (U8*)TCI_NEW( char[ tln+1 ] );
  if ( tmpl )
    strncpy( (char*)ztmpl,(char*)tmpl,tln );
  ztmpl[ tln ]  =  0;

/*
JBMLine( "TmplIter_ctor for::\n" );
JBMLine( (char*)ztmpl );
JBMLine( "\n" );
*/

  curr_element_ID[0]  =  0;
  curr_element_env[0] =  0;

  curr_element_off1   =  0;      // set iteration state to the beginning
  curr_element_off2   =  0;

  // This object is recursive.  Templates can contain "variables" which
  //  name nested templates.  When the current element in this template
  //  iterater is a variable, another template iterater is constructed
  //  from the template string that defines the variable.  The following
  //  variable holds a pointer to this nested TmplIterater.

  macro_iter =  (TmplIterater*)NULL;
}


TmplIterater::~TmplIterater() {

  if ( macro_iter )
    delete macro_iter;
  delete ztmpl;
}


/*
    This is a utility for Parser, Arbitrator and Translater.
  It is used to iterate thru the "elements" of this template.

  Elements of type TMPL_ELEMENT_MACRO are handled entirely within
  this object.  When encountered, they are expanded and the template
  elements from the variable's definition are returned.

  Returns the "type" constant for the template element encountered.

TMPL_ELEMENT_END, TMPL_ELEMENT_UNKNOWN,
TMPL_ELEMENT_LITERAL,TMPL_ELEMENT_REQPARAM,TMPL_ELEMENT_OPTPARAM,
TMPL_ELEMENT_LIST,TMPL_ELEMENT_BUCKET,TMPL_ELEMENT_VARIABLE,
TMPL_ELEMENT_IF,TMPL_ELEMENT_elseIF,TMPL_ELEMENT_ELSE,TMPL_ELEMENT_ifEND,
TMPL_ELEMENT_TVERBATIM,
TMPL_ELEMENT_NOTTEXBUCKET,
TMPL_SYNTAX_ERROR

TMPL_ELEMENT_MACRO - This must never be returned to a client!!
*/

/*
  switch ( *ptr ) {   // the first letter of an element determines it's type
    case '!'  : 		// literal
    case 'I'  : 		// IF
    case 'e'  :         // elseIF
    case 'E'  :         // ELSE
    case 'i'  :         // ifEND
    case 'B'  : 		// BUCKET
    case 'L'  : 		// LIST
    case 'O'  : 		// OPTPARAM
    case 'R'  : 		// REQPARAM
    case 'T'  :         // TVERBATIM(5.99.3,TEXT,{,})
    case 'V'  :         // VAR(0.0.0,MATH,n,dflt,})
    case 'r'  : 		// reqELEMENT
    case '_'  :         // we've hit a macro
    default   :         // This is an error condition
  }       // switch on first letter of template element
*/

U16 TmplIterater::GetNextElement() {

//JBMLine( "Calling tmpl->GetNextElement()\n" );

  if ( macro_iter ) {    // the client get next element from a macro
    U16 rv  =  macro_iter->GetNextElement();
    if ( rv==TMPL_ELEMENT_END ) {       // end of macro
      delete macro_iter;
      macro_iter =  NULL;
    } else
      return rv;
  }

  if ( curr_element_off2 >= strlen((char*)ztmpl) )
    return TMPL_ELEMENT_END;

  // We assume a syntax error.  Only when we have succeeded in parsing
  //  the next element are the element type ( and rv ) set.

  U16 rv    =  TMPL_SYNTAX_ERROR;
  char* ep;
  char* ptr =  (char*)ztmpl + curr_element_off2;

  switch ( *ptr ) {   // the first letter of an element determines it's type

    case '!'  :  {
      curr_element_ID[0]  =  0;
      curr_element_env[0] =  0;
      ep  =  strchr( ptr+1,'!' );
      if ( ep )
        rv  =  TMPL_ELEMENT_LITERAL;
    }
    break;

    case 'I'  :  {

      //IF(MATH,?\format?,5.67.999)
      //  1    2         3        4

      char* p1  =  strchr( ptr,'(' );
      if ( !p1 ) break;
      char* p2  =  strchr( p1,',' );        // extract env
      if ( !p2 ) break;                         
      U16 zln   =  p2 - p1 - 1;
      strncpy( (char*)curr_element_env,(char*)p1+1,zln );
      curr_element_env[ zln ] =  0;

      p2++;
      char* p3  =  strchr( p2,',' );        // extract condition
      if ( !p3 ) break;                         
      condition_ptr =  (U8*)p2;
      condition_len =  p3 - p2;

      p3++;
      ep  =  strchr( p3,')' );              // extract ID
      if ( ep ) {
        U16 zln   =  ep - p3;
        strncpy( (char*)curr_element_ID,(char*)p3,zln );
        curr_element_ID[ zln ]  =  0;
        rv  =  TMPL_ELEMENT_IF;
      }
    }
    break;

    case 'e'  :  {                      // elseIF
      char* p1  =  strchr( ptr,'(' );
      if ( !p1 ) break;
      char* p2  =  strchr( p1,',' );
      if ( !p2 ) break;
      condition_ptr =  (U8*)p1 + 1;
      condition_len =  p2 - p1 - 1;

      p2++;
      ep  =  strchr( p2,')' );              // extract ID
      if ( ep ) {
        U16 zln   =  ep - p2;
        strncpy( (char*)curr_element_ID,(char*)p2,zln );
        curr_element_ID[ zln ]  =  0;
        rv  =  TMPL_ELEMENT_elseIF;
      }
    }
    break;

    case 'E'  :  {                      // ELSE
      curr_element_ID[0]  =  0;
      char* p1  =  strchr( ptr,'(' );
      if ( !p1 ) break;
      ep  =  strchr( p1,')' );
      if ( ep ) {
        condition_ptr =  (U8*)p1 + 1;   // we overload condition to hold
        condition_len =  ep - p1 - 1;   //  the body of the else clause
        rv  =  TMPL_ELEMENT_ELSE;
      }
    }
    break;

    case 'i'  :  {                      //ifEND
      curr_element_ID[0]  =  0;
      curr_element_env[0] =  0;
      ep  =  ptr + 4;
      rv  =  TMPL_ELEMENT_ifEND;
    }
    break;

    case 'B'  :  {

    //BUCKET(0.0.0,MATH,l_delim_tok,r_delim_tok,stop_tok_list,error_stops)
    //      1     2    3           4           5             6			 7

      char* p1  =  strchr( ptr,'(' );       // extract uID
      if ( !p1 ) break;
      char* p2  =  strchr( p1,',' );
      if ( !p2 ) break;
      U16 zln   =  p2 - p1 - 1;
      strncpy( (char*)curr_element_ID,(char*)p1+1,zln );
      curr_element_ID[ zln ] =  0;

      p2++;
      char* p3  =  strchr( p2,',' );        // extract env
      if ( !p3 ) break;
      zln       =  p3 - p2;
      strncpy( (char*)curr_element_env,(char*)p2,zln );
      curr_element_env[ zln ] =  0;

      p3++;
      char* p4  =  strchr( p3,',' );        // extract left delimiting token
      if ( !p4 ) break;
      start_lit_ptr =  (U8*)p3;
      start_lit_len =  p4 - p3;

      p4++;
      char* p5  =  strchr( p4,',' );        // extract right delimiting token
      if ( !p5 ) break;
      end_lit_ptr   =  (U8*)p4;
      end_lit_len   =  p5 - p4;

      p5++;
      char* p6  =  strchr( p5,',' );        // extract stoppers list
      if ( !p6 ) break;
      stopper_ptr   =  (U8*)p5;
      stopper_len   =  p6 - p5;

      p6++;
      ep  =  strchr( p6,')' );              // extract error stop token(s)
      if ( ep ) {
        list_etok_ptr   =  (U8*)p6;
        list_etok_len   =  ep - p6;
        rv  =  TMPL_ELEMENT_BUCKET;
      }
    }
    break;

    case 'L'  :  {

      //LIST(5.30.1,ITEM_TMPL,0.0.0,start_tok,end_tok_list,continue_tok)
      //    1      2         3     4         5            6             7

      curr_element_env[0] =  0;

      char* p1  =  strchr( ptr,'(' );           // extract uID
      if ( !p1 ) break;
      char* p2  =  strchr( p1,',' );
      if ( !p2 ) break;
      U16 zln   =  p2 - p1 - 1;
      strncpy( (char*)curr_element_ID,(char*)p1+1,zln );
      curr_element_ID[ zln ] =  0;

      p2++;                                     // item template
      char* p3  =  p2 + GetTmplLen( (U8*)p2 );
      if ( p3 == p2 ) break;
      nest_tmpl_ptr =  (U8*)p2;
      nest_tmpl_len =  p3 - p2;

      p3++;                                     // counter ID
      char* p4  =  strchr( p3,',' );
      if ( !p4 ) break;
      list_counter_ptr  =  (U8*)p3;
      list_counter_len  =  p4 - p3;

      p4++;                                     // start token
      char* p5  =  strchr( p4,',' );
      if ( !p5 ) break;
      start_lit_ptr =  (U8*)p4;
      start_lit_len =  p5 - p4;

      p5++;                                     // end token
      char* p6  =  strchr( p5,',' );
      if ( !p6 ) break;
      list_etok_ptr =  (U8*)p5;
      list_etok_len =  p6 - p5;

      p6++;                                     // continuation token
      ep  =  strchr( p6,')' );
      if ( ep ) {
        list_ctok_ptr =  (U8*)p6;
        list_ctok_len =  ep - p6;
        rv  =  TMPL_ELEMENT_LIST;
      }
    }
    break;

    case 'O'  :  
    case 'R'  :  {
      char* p1  =  strchr( ptr,'(' );
      if ( !p1 ) break;
      char* p2  =  strchr( p1,',' );
      if ( !p2 ) break;
      U16 zln   =  p2 - p1 - 1;
      strncpy( (char*)curr_element_ID,(char*)p1+1,zln );
      curr_element_ID[ zln ] =  0;

      ep  =  strchr( p2,')' );
      if ( ep ) {
        zln =  ep - p2 - 1;
        strncpy( (char*)curr_element_env,p2+1,zln );
        curr_element_env[ zln ] =  0;
        if ( *ptr == 'R' )
          rv  =  TMPL_ELEMENT_REQPARAM;
        else
          rv  =  TMPL_ELEMENT_OPTPARAM;
      }
    }
    break;

    case 'T'  :  {
      //TVERBATIM(5.99.3,TEXT,F)
      //         1      2    3 4

      char* p1  =  strchr( ptr,'(' );       // extract uID
      if ( !p1 ) break;
      char* p2  =  strchr( p1,',' );
      if ( !p2 ) break;
      U16 zln   =  p2 - p1 - 1;
      strncpy( (char*)curr_element_ID,(char*)p1+1,zln );
      curr_element_ID[ zln ] =  0;

      p2++;
      char* p3  =  strchr( p2,',' );        // extract env
      if ( !p3 ) break;
      zln       =  p3 - p2;
      strncpy( (char*)curr_element_env,(char*)p2,zln );
      curr_element_env[ zln ] =  0;

      p3++;
      ep  =  strchr( p3,')' );              // extract verb flag
      if ( ep ) {
        condition_ptr   =  (U8*)p3;
        condition_len   =  ep - p3;
        rv  =  TMPL_ELEMENT_TVERBATIM;
      }
    }
    break;

    case 'N'  :  {
      //NONTEXBUCKET(5.99.3,TEXT,{,})
      //            1      2    3 4 5

      char* p1  =  strchr( ptr,'(' );       // extract uID
      if ( !p1 ) break;
      char* p2  =  strchr( p1,',' );
      if ( !p2 ) break;
      U16 zln   =  p2 - p1 - 1;
      strncpy( (char*)curr_element_ID,(char*)p1+1,zln );
      curr_element_ID[ zln ] =  0;

      p2++;
      char* p3  =  strchr( p2,',' );        // extract env
      if ( !p3 ) break;
      zln       =  p3 - p2;
      strncpy( (char*)curr_element_env,(char*)p2,zln );
      curr_element_env[ zln ] =  0;

      p3++;
      char* p4  =  strchr( p3,',' );        // extract nesting tokens
      if ( !p4 ) break;
      list_ctok_ptr   =  (U8*)p3;
      list_ctok_len   =  p4 - p3;

      p4++;
      ep  =  strchr( p4,')' );              // extract stoppers list
      if ( ep ) {
        stopper_ptr   =  (U8*)p4;
        stopper_len   =  ep - p4;
        rv  =  TMPL_ELEMENT_NONTEXBUCKET;
      }
    }
    break;

    case 'V'  :  {

      //VAR(0.0.0,MATH,n,dflt,})
      //   1     2    3 4    5 6

      char* p1  =  strchr( ptr,'(' );       // extract uID
      if ( !p1 ) break;
      char* p2  =  strchr( p1,',' );
      if ( !p2 ) break;
      U16 zln   =  p2 - p1 - 1;
      strncpy( (char*)curr_element_ID,(char*)p1+1,zln );
      curr_element_ID[ zln ] =  0;

      p2++;
      char* p3  =  strchr( p2,',' );        // extract env
      if ( !p3 ) break;
      zln       =  p3 - p2;
      strncpy( (char*)curr_element_env,(char*)p2,zln );
      curr_element_env[ zln ] =  0;

      p3++;  
      char* p4  =  strchr( p3,',' );
      if ( !p4 ) break;
      variable_ptr  =  (U8*)p3;

      p4++;
      char* p5  =  strchr( p4,',' );
      if ( !p5 ) break;
      dflt_ptr  =  (U8*)p4;
      dflt_len  =  p5 - p4;

      p5++;
      ep  =  strchr( p5,')' );              // extract stoppers list
      if ( ep ) {
        stopper_ptr   =  (U8*)p5;
        stopper_len   =  ep - p5;
        rv  =  TMPL_ELEMENT_VARIABLE;
      }
    }
    break;

    case 'r'  :  {
      char* p1  =  strchr( ptr,'(' );
      if ( !p1 ) break;
      ep  =  strchr( p1,')' );
      if ( ep ) {
        U16 zln   =  ep - p1 - 1;
        strncpy( (char*)curr_element_ID,(char*)p1+1,zln );
        curr_element_ID[ zln ] =  0;
        strcpy( (char*)curr_element_env,"MATH" );
        rv  =  TMPL_ELEMENT_reqELEMENT;
      }
    }
    break;

    case '_'  :  {              // we've hit a macro
      curr_element_ID[0]  =  0;
      curr_element_env[0] =  0;
      curr_element_type   =  TMPL_ELEMENT_MACRO;

      U8* macro_nom =  (U8*)ptr;            //..._ABC_...
      U16 nom_len =  GetTmplLen( macro_nom );

      curr_element_off1 =  curr_element_off2;
      curr_element_off2 +=  nom_len;

      TmplIterater* ti  =  NULL;
      U16 next_el =  TMPL_ELEMENT_END;
      U8* ztemplate;
      if ( s_grammar->GetMacroDef(macro_nom,nom_len,&ztemplate) && ztemplate ) {

/*
JBMLine( "Macro lookup\n" );
JBMLine( (char*)ztemplate );
JBMLine( "\n" );
*/

        U16 tmpl_len  =  strlen( (char*)ztemplate );
        ti  =  TCI_NEW( TmplIterater(s_grammar,ztemplate,tmpl_len,logfiler) );
        next_el =  ti->GetNextElement();
      }

      if ( next_el == TMPL_ELEMENT_END ){   // macro undefined or empty
        if ( ti ) delete ti;
        return GetNextElement();
      } else {                              // install the macro
        macro_iter =  ti;
        return  next_el;
      }

    }
    break;

    default : {         // This is an error condition
      curr_element_ID[0]  =  0;
      curr_element_env[0] =  0;
      logfiler->LogSrcError( UNKNOWN_TMPL_ELEMENT,NULL,ztmpl,curr_element_off2,0 );
      rv  =  TMPL_ELEMENT_UNKNOWN;
    }
    break;

  }       // switch on first letter of template element

  curr_element_type  =  rv;
  curr_element_off1  =  curr_element_off2;
  if (  rv == TMPL_SYNTAX_ERROR ) {
    logfiler->LogSrcError( BAD_TMPL_SYNTAX,NULL,ztmpl,curr_element_off1,0 );
  } else
    curr_element_off2 =  (U8*)ep - ztmpl + 1;

/*
U16 eln =  curr_element_off2 - curr_element_off1;
U8* tmp =  (U8*)TCI_NEW(char[ eln+1 ] );
strncpy( (char*)tmp,(char*)ztmpl+curr_element_off1,eln );
tmp[eln] = 0;
JBMLine( "NEXT TMPL ELM::\n" );
JBMLine( (char*)tmp );
JBMLine( "\n" );
delete tmp;
*/

  return rv;
}


// As template elements are located, their attributes are also found.
//  Clients call the "GetField" functions to get these attributes.

U8* TmplIterater::GetField( U16 whichfield ) {

  if ( macro_iter )
    return macro_iter->GetField( whichfield );

  U8* rv;
  switch ( whichfield ) {

    case TFIELD_ID  :
      rv  =  curr_element_ID;
    break;

    case TFIELD_env :
      rv  =  curr_element_env;
    break;

    default :
      rv  =  (U8*)NULL;
    break;
  }

  return rv;
}


U8* TmplIterater::GetField( U16 whichfield,U16& ln ) {

  if ( macro_iter )
    return macro_iter->GetField( whichfield,ln );

  U8* rv;
  switch ( whichfield ) {

    case TFIELD_lit       :
      rv  =  ztmpl + curr_element_off1;
      ln  =  curr_element_off2 - curr_element_off1;
    break;

    case TFIELD_nest      :
      rv  =  nest_tmpl_ptr;
      ln  =  nest_tmpl_len;
    break;

    case TFIELD_start     :
      rv  =  start_lit_ptr;
      ln  =  start_lit_len;
    break;

    case TFIELD_end       :
      rv  =  end_lit_ptr;
      ln  =  end_lit_len;
    break;

    case TFIELD_stopper   :
      rv  =  stopper_ptr;
      ln  =  stopper_len;
    break;

    case TFIELD_list_cont :
      rv  =  list_ctok_ptr;             // list continuation literal token
      ln  =  list_ctok_len;             
    break;

    case TFIELD_list_end  :
      rv  =  list_etok_ptr;             // list terminating literal token
      ln  =  list_etok_len;
    break;

    case TFIELD_list_counter  :
      rv  =  list_counter_ptr;
      ln  =  list_counter_len;
    break;

    case TFIELD_else      :
    case TFIELD_condition :
      rv  =  condition_ptr;
      ln  =  condition_len;
    break;

    case TFIELD_var_type  :
      rv  =  variable_ptr;
      ln  =  1;
    break;

    case TFIELD_dflt      :
      rv  =  dflt_ptr;
      ln  =  dflt_len;
    break;

    default :
      rv  =  (U8*)NULL;
      ln  =  0;
    break;
  }

  return rv;
}


U16 TmplIterater::GetTmplLen( U8* src ) {

  U16 rv  =  0;

  if ( *src == '_' ) {          // variable = "_BLAH_"

    U8* ptr =  (U8*)strchr( (char*)src+1,'_' );
    if ( ptr ) 
      rv  =  ptr - src + 1;

  } else {                      // element  = "ELEMENT( ... )

    U8 ch;
    U8* ptr   =  src;

    U16 level =  0;
    while ( ch = *ptr ) {
      if ( ch=='(' )
        level++;
      else if ( ch==')' ) {
        level--;
        if ( level==0 ) {
          rv  =  ptr - src + 1;
          break;
        }
      }

      ptr++;
    }
  }

  return rv;
}


U8* TmplIterater::MakeTokenList( U16 whichfield ) {

  U16 len;
  U8* toks;
  if ( macro_iter )
    toks  =  macro_iter->GetField( whichfield,len );
  else
    toks  =  GetField( whichfield,len );

  if ( toks && len )
    return  MakeTokenList( toks,len );
  else
    return  (U8*)NULL;
}


// Templates may contain sets of tokens that are joined by "or", ie "|".
//   For example, "&|\\|\endmatrix"
// The following utility generates a catenated strings version
//                 &0\\0\endmatrix00

U8* TmplIterater::MakeTokenList( U8* toks,U16 len ) {

  U16 ret_ln  =  len + 2;
  U8* rv      =  (U8*)TCI_NEW( char[ ret_ln ] );
  strncpy( (char*)rv,(char*)toks,len );
  rv[ len ]   =  0;
  rv[ len+1 ] =  0;
  U8* fix =  rv;
  while ( *fix ) {
    if ( *fix == '|' ) *fix =  0;
    fix++;
  }

  return rv;
}


// When a client encounters an IF it typically loops thru the elements
//  that follow until some condition is met.  The client then needs
//  to skip the rest of the "alternatives" and get to the end of the IF. 
//  The following function is provided for this purpose.

TCI_BOOL TmplIterater::EndAlternation() {

  U16 if_level  =  1;       // We handle nested IFs - actually not allowed
  U16 alter_elm_type;
  do {
    alter_elm_type  =  GetNextElement();
    if        ( alter_elm_type == TMPL_ELEMENT_IF ) {
      if_level++;

    } else if ( alter_elm_type == TMPL_ELEMENT_ifEND ) {
      if_level--;
      if ( if_level == 0 )  return TRUE;  

    } else if ( alter_elm_type == TMPL_ELEMENT_elseIF ) {

    } else if ( alter_elm_type == TMPL_ELEMENT_ELSE ) {

    } else if ( alter_elm_type == TMPL_ELEMENT_END ) {  // ERROR

JBMLine( "EndAlternation::Unexpected end of tmpl\n" );
      return FALSE;

    } else {                                            // ERROR
char zzz[80];
sprintf( zzz,"EndAlternation::Unexpected tmpl element found, %d\n",alter_elm_type );
JBMLine( zzz );
      return FALSE;
    }

  } while ( TRUE );

  return FALSE;
}

