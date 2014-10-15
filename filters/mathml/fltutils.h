
#ifndef FilterUtils_h
#define FilterUtils_h

#include "flttypes.h"
#include <stdlib.h>
#include <stdio.h>
//jcs #include <io.h>


#define ZID_STR_LIM     24

/*  As we parse LaTeX, a data structure called a "Parse Tree"
is generated. Each node in this tree is a "TNODE" struct
as defined below.
  Note that this one struct is used for two proposes:

  1. Each object that is represented explicitly by a token,
     or command, or an environment in the LaTeX file is represented
     in the parse tree by a TNODE.
  2. Many implicit higher level container objects are also
     represented in the parse tree by a TNODE.

  Example:  $x+\frac 12$

  Each node has a tag of the form <n.n.n>, identifying the object
  represented by the node.

<1.2.0>$ <-> <3.1.24>x <-> <3.13.60>+ <-> <5.1.0>\frac <-> <2.2.0>$
											  \
										      <5.1.1> <-> <5.1.2>
												 \			 \
									            <3.3.2>1    <3.3.3>2

  There are 7 LaTeX tokens in the examples - note that "\frac" is one
  token.  In the parse tree, a 2-node sublist appears below the \frac
  node.  These nodes represent the implicit parts of a fraction,
  numerator and denominator.  Sublists representing the explicit
  contents of these parts appear below.
*/

typedef struct tagANOMALY_REC {	// Struct to 
  tagANOMALY_REC* next;
  U16 ilk;
  U8* atext;
  U32 off1;
  U32 off2;
} ANOMALY_REC;

#define ANOMALY_HYPERREF (1000) // a \hyperref in math
#define ANOMALY_NOTE     (1001) // eg \footnote in math
#define ANOMALY_FRAME    (1002) // a \frame in math
#define ANOMALY_VSPACE   (1003) // \vspace, \medskip, ...
#define ANOMALY_TAG      (1004) // tag, notag, tag*
#define ANOMALY_LEFTOVER (1005)
#define ANOMALY_AUTONUMBERING (1010)
#define ANOMALY_NONUMBERING (1011)



typedef struct tagATTRIB_REC {	// Struct to define an attribute node.
  tagATTRIB_REC*  next;			//  The TNODE struct defined below
  tagATTRIB_REC*  prev;			//  can carry a list of attributes.
  U16             attr_ID;
  U8*             attr_nom;
  U16             e_val;
  U8*             z_val;
  U16             attr_vtype;
} ATTRIB_REC;


typedef struct tagCVLINE_REC {	// Struct to one line of a LaTeX
  tagCVLINE_REC*  next;			//  comment or verbatim environment.
  tagCVLINE_REC*  prev;			
  U16             whatever;
  U8*             cvline;
} CVLINE_REC;


// struct to carry info for a message to be written to the log file

typedef struct tagLOG_MSG_REC {
  tagLOG_MSG_REC*   next;
  U32 src_lineno;
  U32 dst_lineno;
  U8  ztoken[ 32 ];
  U8* msg;
} LOG_MSG_REC;

// Operator forms

#define	OF_PREFIX         1
#define	OF_INFIX          2
#define	OF_POSTFIX        3
//#define	OF_PREorIN	  4
//#define	OF_PREorPOST	  5
//#define	OF_INorPOST	  6
//#define	OF_PREorINorPOST  7


#define DETAILS_form		1
#define DETAILS_precedence	2
#define DETAILS_delimited_group 3
#define DETAILS_function_status 4
#define DETAILS_unit_state      5
#define DETAILS_is_expression   6
#define DETAILS_is_differential 7
#define DETAILS_bigop_status    8
#define DETAILS_integral_num    9
#define DETAILS_is_func_arg     10
#define DETAILS_space_width     11
#define DETAILS_style           12
#define DETAILS_is_geometry     13
#define DETAILS_is_Greek        14
#define DETAILS_bigl_size       15
#define DETAILS_TeX_atom_ilk    16
#define DETAILS_last_id         16

typedef struct tagDETAILS {
  I16 form;
  I16 precedence;
  I16 delimited_group;
  I16 function_status;
  I16 unit_state;
  I16 is_expression;
  I16 is_differential;
  I16 bigop_status;
  I16 integral_num;
  I16 is_func_arg;
  I16 space_width;
  I16 style;
  I16 is_geometry;
  I16 is_Greek;
  I16 bigl_size;
  I16 TeX_atom_ilk;
} DETAILS;


typedef struct tagMATH_CONTEXT {
  I16 chapter_num;
  I16 section_num;
  I16 subsection_num;
  I16 etcetra;
} MATH_CONTEXT;


typedef struct tagTNODE {
  tagTNODE*     next;
  tagTNODE*     prev;
  U32           src_offset1;
  U32           src_offset2;
  U32           src_linenum;
  U8            zuID[ ZID_STR_LIM ];
  U8            cuID[ 16 ];
  U8            src_tok[ 32 ];
  U16           status;
  tagTNODE*     parts;
  tagTNODE*     contents;
  tagTNODE*     sublist_owner;
  U8*           var_value;
  U8*           var_value2;
  U16           v_len;
  U16           v_type;
  ATTRIB_REC*   attrib_list;
  LOG_MSG_REC*  msg_list;
  DETAILS*      details;
  CVLINE_REC*   cv_list;
} TNODE;


// We put data from the ini file into the following struct
//  and pass it down to objects requiring the info.

typedef struct tagUSERPREFS {
  I16 showpreface;
  U16 mml_version;
  U16 output_mode;
  U8* Preface;

  U8* start_display_math;           //<mml:math class="displayedmathml" mode="display" display="block">
  U8* start_inline_math;            //<mml:math class="inlinemathml" display="inline">
  U8* namespace_prefix;             // "mml:",
  U8* mstyle_attrs;                 // "",
  U8* mathname_attrs;               // "mathcolor=\"gray\"",
  U8* unitname_attrs;               // "mathcolor=\"green\"",
  U8* text_in_math_attrs;           // "mathcolor=\"black\"",
  U8* link_attrs;                   // "mathcolor=\"green\"",
  U8* renderer_baselines;           // "true",
  U8* eqn_tags_to_mlabeledtr;       // "false",
  U8* eqn_nums_to_mlabeledtr;       // "false",
  U8* entity_mode;                  // "0",
  U8* lr_spacing_mode;              // "1",
  U8* lr_spacing_mode_in_scripts;   // "1",
  U8* eqn_nums_format;              // "(%theequation%)",
  U8* long_arrows_are_stretched;    // "false",
  U8* indent_increment;             // "2",
  U8* adjust_output_for_IE_spacing_bug; // "true",
  U8* end_math;                     // <mml:mtext class="fix-IE-bug">&ZeroWidthSpace;</mml:mtext></mml:math>

} USERPREFS;


#define	TT_CONTEXT_START      1
#define	TT_CONTEXT_END        2
#define TT_SYMBOL             3
#define TT_GENERIC            4
#define TT_BUCKET_START       5
#define TT_BUCKET_END         6
#define TT_TEMPLATE_LITERAL   7
#define TT_START_REQPARAM     8
#define TT_END_REQPARAM       9
#define TT_START_OPTPARAM     10
#define TT_END_OPTPARAM       11
#define TT_LIST_ITEM_START    12
#define TT_LIST_CONTINUE      13
#define TT_VAR_VALUE          14
#define TT_VAR_VALUEA         15

#define TT_S_ELEM_HEADER      16
#define TT_S_ELEM_ENDER	      17
#define TT_A_ELEM_HEADER      18
#define TT_A_ELEM_ENDER	      19
#define TT_ATOM_BODY	      20
#define TT_REQ_ELEM	      21

#define TT_ALIGNGROUP	      22


typedef struct tagTILE { // Struct to carry translated fragments
  tagTILE*   	next;	    //  from "Translator" to "PageManager".
  tagTILE*   	prev;	    //  Used to facilitate line breaking
  U16         ilk;	    //  and pagination of output.
  U8*         zval;
  LOG_MSG_REC*  msg_list;
} TILE;


// return values for the Parser

#define PR_ERROR        0
#define PR_COMPLETE     1
#define PR_INCOMPLETE   2

#define TOKENIZER_MML	1
#define TOKENIZER_XML   2
#define TOKENIZER_TEX   3

#define FT_MATHML	1
#define FT_NBLATEX      2

#define ST_MI	        1
#define ST_MN           2
#define ST_MO           3

#define COPF_PREFIX     1
#define COPF_INFIX      2
#define COPF_POSTFIX    3

#define IM_STANDARD	1
#define IM_VERBATIM	2
#define IM_PREAMBLE	3
#define IM_NONLATEX	4
#define IM_TEXGLUE   	5
#define IM_TEXDIMEN 	6
#define IM_SPECIAL	7
#define IM_SPECSTR	8
#define IM_NUMBER	9
#define IM_BOOLEAN	10
#define IM_LBRACE	11
#define IM_DFLOAT	12
#define IM_HYPHENATION	13



#define TCMD_Sb		    30
#define TCMD_Sp		    31
#define TCMD_DisplayedMath  34
#define TCMD_subscript      50
#define TCMD_superscript    51

#define TCMD_DollarMath	    77

// Productions for the LaTeX environment "eqnarray"

#define TENV_eqnarray       121
#define TENV_eqnarraystar   122
#define TENV_align          123
#define TENV_alignstar      124
#define TENV_alignat        125
#define TENV_alignatstar    126
#define TENV_xalignat       127
#define TENV_xalignatstar   128
#define TENV_xxalignat      129
#define TENV_xxalignatstar  130
#define TENV_gather         131
#define TENV_gatherstar     132
#define TENV_multline       133
#define TENV_multlinestar   134

// these are MATH only environments

#define TENV_cases          140
#define TENV_split          141
#define TENV_gathered       142
#define TENV_aligned        143
#define TENV_alignedat      144

#define TCMD_U		    298
#define TCMD_UNICODE        299
#define TCMD_symbol	    465
#define TCMD_TEXTsymbol     466

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


#define	TR_Mbf			  82
#define	TR_Mbs			  91
#define	TR_Mit		      	  86
#define	TR_Mnf            	  85
#define	TR_Mrm			  81
#define	TR_Msf            	  83
#define	TR_Mtt            	  84
#define	TR_Mcal           	  80
#define	TR_MBbb           	  87
#define	TR_Mfrak          	  88
#define	TR_group           	  6
#define	TR_Mpmb           	  89
#define	TR_displaystyle  	  600
#define	TR_textstyle     	  601
#define	TR_scriptstyle  	  602
#define	TR_scriptscriptstyle      603


#define	TR_Tup           	  450
#define	TR_Tit		      	  451
#define	TR_Tsl           	  452
#define	TR_Tsc           	  453
#define	TR_Tmd		     	  454
#define	TR_Tbf			  455
#define	TR_Trm                    456
#define	TR_Tsf                    457
#define	TR_Ttt                    458
#define	TR_em		          459
#define	TR_Tnormal                460
#define	TR_Tcircled               461


#define	TR_tiny                   480
#define	TR_scriptsize             481
#define	TR_footnotesize           482
#define	TR_small                  483
#define	TR_normalsize             484
#define	TR_large                  485
#define	TR_Large                  486
#define	TR_LARGE                  487
#define	TR_huge                   488
#define	TR_Huge                   489


void 	  FUSetDumpFile( char* dump_file_spec );
TCI_BOOL  FUOpenDumpFile();
void	  FUCloseDumpFile();
void 	  DumpLine( const char* line );

void 	  JBMLine( const char* line );

#define CONTEXT_NOM_LIM     16

void 	  DumpTNode( TNODE* t_node,U16 indent,U16 grammar_ID );
void 	  DumpTList( TNODE* t_list,U16 indent,U16 grammar_ID );

TNODE* 	MakeTNode( U32 s_off,U32 e_off,U32 line_no,U8* zuID );


void 	DisposeTNode( TNODE* del );
void 	DisposeTList( TNODE* t_list );
void 	DisposeMsgs( LOG_MSG_REC* msg_list );
TNODE*  JoinTLists( TNODE* list,TNODE* newtail );

#define INVALID_LIST_POS  30000

TNODE* 	FindObject( TNODE* parts_list,U8* zID,U16 list_pos );

void 	UidsTozuID( U16 uobjtype,U16 usubtype,U16 uID,U8* dest );
void 	GetUids( U8* idstr,U16& uobjtype,U16& usubtype,U16& uID );

void 	StrReplace( char* line,char* tok,char* sub );

void 	CheckTNODETallies();

CVLINE_REC* MakeCVNode( U16 whatever,U8* cv_line,U16 cv_ln  );
CVLINE_REC* JoinCVLists( CVLINE_REC* cv_list,CVLINE_REC* new_tail );
void 	    DisposeCVLines( CVLINE_REC* cv_list );

U8*     AppendBytesToBuffer( U8* buffer,U32& buf_lim,
                      U32& buf_i,U8* new_bytes,U32 n_bytes );

ATTRIB_REC* MakeATTRIBNode( U8* attrib_name,	// =  AttrIDToName( attr_ID,0 );
			    U16 attr_ID,U16 e_ID,
			    U8* svalue,U16 vln,
			    U16 val_type );
void      DisposeAttribs( ATTRIB_REC* alist );

TILE*     AppendTILEs( TILE* tile_list,TILE* new_tail );
TILE*     MakeTILE( U16 ilk,U8* zval );
void 	  DumpTILEs( TILE* tile_list );
TILE*     JoinTILELists( TILE* rv,TILE* new_frag );
void 	  DisposeTILEs( TILE* tiles );

bool      DelinkTNode( TNODE* elem );
void      DetachTList( TNODE* elem );
TNODE*    CopyTList( TNODE* list );
TNODE*    CopyTNode( TNODE* node );

void      HLinesToBucket( TNODE* LaTeX_array,TCI_BOOL is_tabular );
void      ColsToList( TNODE* LaTeX_array );
TNODE*    MakeColsEntry( TNODE* batch,U16 pos );
TNODE*    RemoveStarCmds( TNODE* cols_list );
TNODE*    ExpandStarCmd( TNODE* star_cmd );

TNODE*    MATRIXtoExternalFormat( TNODE* MATRIX_node );
TNODE*    MATRIXtocases( TNODE* MATRIX_node );
TNODE*    EQNtoeqnarray( TNODE* EQN_node,U16& usubtype );
void      EQNtoEquation( TNODE* EQNROW,U16 uID,TNODE* rv );

TNODE*    MATRIXcellToArray( CELL_STRUCT* CELL_rover,
			     U16 column_no,
			     TNODE* COLS_reqparam,
			     U16* global_vrules,
			     U16* global_lcrs,
			     U16 dest_subtype );
TNODE*    CountHLineTokens( TNODE* hline_rover,U16& tally );
TNODE*    CellColsToMultiColumn( CELL_STRUCT* mc_data,
				 U16 column_no );

CELL_STRUCT*  MakeCellStructs( TNODE* CELL_entries_list,
			       U16* global_vrules,
			       U16* global_lcrs,
			       U16 ncols );
void      AdjustCellStructs( CELL_STRUCT* list,
				 U16 n_columns );
void      DisposeCellStructs( CELL_STRUCT* list );
void      DumpCellStructs( CELL_STRUCT* list );

bool      ColsMatch( CELL_STRUCT* CELL_rover,
		     U16 column_no,
		     TNODE* global_cols,
		     U16* global_vrules,
		     U16* global_lcrs );
bool      SameAtText( TNODE* cell_at,TNODE* global_at );
bool      SamePWidth( TNODE* cell_p, U16 column_no, TNODE* global_cols );
U16       EQNenumIDtouID( U16 enum_ID, U8* env_nom, U16& LaTeX_uID );
TNODE*    ProcessEqnRD( TNODE* RD_entry_node, TCI_BOOL& is_intertext );

U32       TextAccentedCharToUnicode( U16 accent_uID,char base_char );

#endif


