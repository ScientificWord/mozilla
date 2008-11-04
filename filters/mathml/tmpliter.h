
#ifndef TmplIterater_h
#define TmplIterater_h
																																																								
/*
   If a grammar element has structure, it is defined by a template.
This template is a zstring consisting of a catenation of
"template elements".

As an example, the following template defines the TeX construct "\Sb":

!\Sb!LIST(5.30.1,BUCKET(5.30.2,MATH,,,\\|\endSb),,\\,\endSb)!\endSb!

  It consists of a LITERAL followed by a LIST followed by a LITERAL.

  Some "template elements" are structured.

  The template element "LIST" has the following structure:
  LIST(#1,#2,#3,#4,#5)
        #1 - the string that names (identifies) the list
        #2 - the template for each item in the list - a nested template
        #3 - optional start token for each item in the list 
        #4 - the continuation token for the list
        #5 - the stop token(s) for the list

  The template element "BUCKET" has the following structure:
  BUCKET(#1,#2,#3,#4,#5)
        #1 - the string that names (identifies) the bucket
        #2 - the name of the context valid for the bucket's contents
        #3 - optional start token for this construct
        #4 - optional end token for this construct
        #5 - the stop token(s) for the bucket

A "TmplIterater" object is constructed from a template zstring.
Parsers and Translaters use TmplIteraters to step thru templates
encountered during parses and translations.
*/

#include "fltutils.h"


#define TMPL_ELEMENT_END            0
#define TMPL_ELEMENT_UNKNOWN        1
#define TMPL_ELEMENT_LITERAL        2
#define TMPL_ELEMENT_REQPARAM       3
#define TMPL_ELEMENT_OPTPARAM       4
#define TMPL_ELEMENT_LIST           5
#define TMPL_ELEMENT_BUCKET         6
#define TMPL_ELEMENT_VARIABLE       7
#define TMPL_ELEMENT_IF             8
#define TMPL_ELEMENT_elseIF         9
#define TMPL_ELEMENT_ELSE           10
#define TMPL_ELEMENT_ifEND          11
#define TMPL_ELEMENT_TVERBATIM      12
#define TMPL_ELEMENT_reqELEMENT     13
#define TMPL_ELEMENT_NONTEXBUCKET	14

#define TMPL_ELEMENT_MACRO          100
#define TMPL_SYNTAX_ERROR           9999


#define TFIELD_ID           1
#define TFIELD_env          2
#define TFIELD_lit          3
#define TFIELD_nest         4
#define TFIELD_start        5
#define TFIELD_end          6
#define TFIELD_stopper      7
#define TFIELD_list_cont    8
#define TFIELD_list_end     9
#define TFIELD_list_counter 10
#define TFIELD_condition    11
#define TFIELD_var_type     12
#define TFIELD_dflt         13
#define TFIELD_else         14


class LogFiler;
class Grammar;

class TmplIterater {

public:
  TmplIterater( Grammar* TeX_grammar,U8* tmpl,U16 tln,
  									LogFiler* new_logger ) ;
  ~TmplIterater();

  U16       GetNextElement();
  U8*       GetField( U16 whichfield );
  U8*       GetField( U16 whichfield,U16& ln );
  TCI_BOOL  EndAlternation();

  U8*       MakeTokenList( U16 whichfield );
  U8*       MakeTokenList( U8* toks,U16 len );

private:

  U16       GetTmplLen( U8* src );

  Grammar*  s_grammar;

  U8*   ztmpl;

  //    variables for the current element during an iteration

  U16   curr_element_off1;
  U16   curr_element_off2;
  U16   curr_element_type;
  U8    curr_element_ID[ ZID_STR_LIM ];
  U8    curr_element_env[CONTEXT_NOM_LIM];
  U8    else_element_ID[ ZID_STR_LIM ];
  U8    else_element_env[CONTEXT_NOM_LIM];

  //    variables for fields within the current element

  U8*   nest_tmpl_ptr;      // LISTs have a nested template
  U16   nest_tmpl_len;

  U8*   start_lit_ptr;      // literal token that starts BUCKET or LIST item
  U16   start_lit_len;

  U8*   end_lit_ptr;        // literal token that ends a BUCKET
  U16   end_lit_len;

  U8*   stopper_ptr;        // literal(s) beyond a BUCKET
  U16   stopper_len;

  U8*   list_ctok_ptr;      // list continuation literal token
  U16   list_ctok_len;

  U8*   list_etok_ptr;      // list terminating literal token(s)
  U16   list_etok_len;

  U8*   list_counter_ptr;
  U16   list_counter_len;

  U8*   condition_ptr;      // 
  U16   condition_len;

  U8*   variable_ptr;       // variable type

  U8*   dflt_ptr;           // variable default value
  U16   dflt_len;

  TmplIterater* macro_iter;

  LogFiler*		logfiler;
};

#endif

