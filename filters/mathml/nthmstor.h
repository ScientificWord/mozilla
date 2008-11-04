
#ifndef NewObjectStore_h
#define NewObjectStore_h
																																																								
/*
  Class NewObjectStore  - A server for Parser.
This object holds the definitions of newtheorems and newenvironments
that an individual document may contain.  Notebook documents are a
special case.  The grammar has a set of built-in newtheorems - 
individual files don't contain \newtheorems commands.
This object is sufficiently general in design to handle both
notebook (grammar defined newtheorems) and general LaTeX (document
defined newtheorems).
  The object DOES NOT persist - a NewObjectStore is created for
each file that is filtered.

  In LaTeX, when Parser encounters "\newtheorem", it calls on
NewObjectStore to store the definition.  When Parser is extracting
tokens from source, things like "\begin{my_theorem}" may be
encountered.  Such tokens are not defined in the underlying grammar.
The NewObjectStore is queried.  If "my_theorem" is defined,
Parser calls upon NewObjectStore to fill out TNODEs for
"\begin{my_theorem}" and "\end{my_theorem}".  zuIDs must be
generated for these objects, along with a production like

\begin{my_theorem}<uID5.329.0>
!\begin{my_theorem}!
BUCKET(5.329.2,TEXT,,,\end{my_theorem})
!\end{my_theorem}!

LaTeX2e adds the following commands:

\theoremstyle{style}	style = "plain", "break", "marginbreak",
								"changebreak", "change", "margin"
\theorembodyfont{font-declarations}		- each newtheorem set can
									      declare 1
\theoremheaderfont{font-declarations}	- global ( 1 per doc )
						font-declarations = "\rmfamily"

*/

#include "flttypes.h"
#include "fltutils.h"
#include <stdlib.h>

#define TRI_envname     1
#define TRI_caption     2
#define TRI_nums_like   3
#define TRI_within      4
#define TRI_tmpl        5
#define TRI_style 		6
#define TRI_bodyfont 	7
#define TRI_starred     8

#define NT_SCOPE_GRAMMAR	0
#define NT_SCOPE_DOCUMENT	1

typedef struct tagTHEOREM_REC {
  tagTHEOREM_REC*   next;
  U16               scope;
  U8*               env_name;
  U8*               caption;
  U8*               nums_like;
  U8*               within;
  U8*               tmpl;
  U8*               style;
  U8*               bodyfont;
  TCI_BOOL			is_starred;
} THEOREM_REC;

#define TA_headerfont	1
#define TA_style		2
#define TA_bodyfont		3

typedef struct tagTHEO_STACK_REC {
  U8*               ts_style;
  U8*               ts_bodyfont;
} THEO_STACK_REC;

/*
\newenvironment{envname}[1..9][default-val-opt-param]{begdef}{enddef}
\newenvironment{GrayBox}[1]{shit #1 happens}{\ENDGRAYBOX}
.
\begin{GrayBox}{never}
.
.
\end{GrayBox}
*/

#define ERI_num_args    1
#define ERI_optdef      2
#define ERI_begdef      3
#define ERI_enddef      4
#define ERI_tmpl        5
#define ERI_is_newenv   6

typedef struct tagENVIRON_REC {
  tagENVIRON_REC* next;
  U16             scope;
  U8*             env_name;
  U8*             arg_count;
  U8*             opt_def;
  U8*             begdef;
  U8*             enddef;
  U8*             tmpl;
  TCI_BOOL		  is_starred;
} ENVIRON_REC;


#define LTX_part           1
#define LTX_chapter        2
#define LTX_section        3
#define LTX_subsection     4
#define LTX_subsubsection  5
#define LTX_paragraph      6
#define LTX_subparagraph   7
#define LTX_page           8
#define LTX_equation       9
#define LTX_figure         10
#define LTX_table          11
#define LTX_footnote       12
#define LTX_mpfootnote     13
#define LTX_enumi          14
#define LTX_enumii         15
#define LTX_enumiii        16
#define LTX_enumiv         17
#define LTX_secnumdepth    18
#define LTX_tocdepth       19

   
typedef struct tagSUBCOUNTER_REC {
  tagSUBCOUNTER_REC* next;
  U8  name[32];
  I16 value;
} SUBCOUNTER_REC;

typedef struct tagCOUNTER_INFO {
  tagCOUNTER_INFO* next;
  U8  name[32];
  U16 style;
  I16 value;
  SUBCOUNTER_REC* subcounters;
} COUNTER_INFO;


class Grammar;

class NewObjectStore {

public:
  NewObjectStore( Grammar* s_grammar );
  ~NewObjectStore();

  U8*     GetTheoremInfo( U8* new_theorm,U16 which_info );

  void    AddNewTheorem( U16 scope,U8* env_nom,U8* caption,
  		  					U8* n_like,U8* in,
                           		TCI_BOOL isstarred );

  U8*     GetEnvironInfo( U8* new_env_nom,U16 which_info );

  void    AddNewEnviron( U16 scope,U8* env_nom,U8* n_args,
  		  						U8* opt_def,U8* begdef,U8* enddef );


  void    EnterExitGroup( TCI_BOOL do_enter );
  void    SetTheoremStyleAttr( U16 which_attr,U8* new_val );

  void    ClearTheorems( U16 scope );
  void    ClearEnvirons( U16 scope );

  void    InitCounters();
  void    DisposeCounters();

  void    ResetAllCounters();
  void    ProcessCounterCmd( TNODE* src_node );

  TCI_BOOL  HasCounter( U8* nom );

  I16     GetCounterValue( U16 enumID );
  I16     GetCounterValue( U8* nom );
  void    GetEquationCounterValue( U8* dest );

  U8*     uIDtoName( U16 usubtype,U16 uID );
  U16     uIDtoEnumID( U16 usubtype,U16 uID );
  U8*     EnumIDtoName( U16 enumID );

  void    PushListCounter();
  void    PopListCounter();
  void    IncListCounter();
  I16     GetListCounterVal();

  void    CreateCounter( U8* nom,I16 val,U8* within );

  void    EnterMathLetters();
  void    ExitMathLetters();

private:

  U8*     MakeThmTemplate( U8* nom );
  U8*     MakeEnvTemplate( U8* env_nom,U8* n_args,U8* opt_def );

  U8*     GetTheoremStyleAttr( U16 which_attr );

  THEOREM_REC*  new_theorems;
  ENVIRON_REC*	new_environs;

/*
{									  we keep track of "theoremstyle"
\theorembodyfont{\rmfamilt}			  and "theorembodyfont" on a stack.
.									  As \newtheorem's are encountered,
\newtheorem{Rem}{Remark}			  we get their style and bodyfont
}									  off this stack.
*/

  U16 ts_sp;
  THEO_STACK_REC theo_stack[4];
  U8* headerfont;


  COUNTER_INFO*   FindCounterInfo( U8* nom );
  void            ResetCounter( U8* nom );
  void            SetCounter( U8* nom,I16 new_val );
  void            IncCounter( U8* nom );

  SUBCOUNTER_REC* AddSubCounter( SUBCOUNTER_REC* list,
  										U8* new_name );

  void            Dump();

  U16             GetCaptionOwnerID( TNODE* caption_node );

  COUNTER_INFO* counters;

  I16           list_counter_vals[4];
  U16           list_sp;

};

#endif

