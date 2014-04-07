
//  Written June 98, JBM, and updated fall of 2000.
//  This object inputs Notebook/WorkPlace LaTeX MATH parse trees
//  and generates corresponding MathML trees as output.
//  Notebook.gmr -> MathML.gmr

#include "ltex2mml.h"
#include "grammar.h"
#include "fltutils.h"
#include "mmlutils.h"
#include "logfiler.h"
#include "treegen.h"

#include <string.h>
#include <stdlib.h>


#define	MML_IDENTIFIER	1
#define	MML_NUMBER		2
#define	MML_OPERATOR	3
#define	MML_TEXT		4
#define	MML_SPACE		5

#define	TeX_ATOM_ORD	1
#define	TeX_ATOM_OP 	2
#define	TeX_ATOM_BIN	3
#define	TeX_ATOM_REL	4
#define	TeX_ATOM_OPEN   5
#define	TeX_ATOM_CLOSE  6
#define	TeX_ATOM_PUNCT  7
#define	TeX_ATOM_INNER  8


#define	MF_UNKNOWN      0
#define	MF_GEOMETRY     1
#define	MF_MATRIX_ALG   2


#define OPF_multiform   0
#define OPF_prefix      1
#define OPF_infix       2
#define OPF_postfix     3


char* uIDToString(U16 usub);

const char *zop_forms[] = {
"multiform",
"prefix",
"infix",
"postfix",
""
};

const char* zmtext  =  "3.204.1";
const char* zmspace =  "3.205.1";

#define UNDEFINED_DETAIL    32000


#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


U8* context_math  =  (U8*)"MATH";

// LaTeX2MMLTree is constructed once by the object "LaTeX2XMLTree".
// The destination Grammar is MathML.gmr
// ctor

LaTeX2MMLTree::LaTeX2MMLTree( TreeGenerator* parent,
                              Grammar* src_mml_grammar,
  							  Grammar* dest_mml_grammar,
  							  LogFiler* logger,
							  USERPREFS* preferences,
							  const char** upref_zstrs ) {

  lparser       =  parent;
  s_mml_grammar =  src_mml_grammar;		// keyed by name
  d_mml_grammar =  dest_mml_grammar;	// keyed by uID
  logfiler      =  logger;
// The "in_display" flag is kept current.  We use it to implement
//  some aspects of displays IFF the renderer (TeX Explorer)
//  doesn't implement <mstyle displaystyle="true">
  in_display    =  FALSE;
  script_level  =  0;
  do_trig_args  =  TRUE;		// hard-coded flag for now


// MML identifiers that are to be interpreted as function names.
//  Should be passed in.

  funcs_list  =  (IDENTIFIER_NODE*)NULL;
  funcs_list  =  AddIdentifier( funcs_list,(U8*)"f" );
  funcs_list  =  AddIdentifier( funcs_list,(U8*)"g" );
  funcs_list  =  AddIdentifier( funcs_list,(U8*)"h" );
  funcs_list  =  AddIdentifier( funcs_list,(U8*)"&Gamma;" );
  funcs_list  =  AddIdentifier( funcs_list,(U8*)"&Fscr;" ); // Fourier
  funcs_list  =  AddIdentifier( funcs_list,(U8*)"&Lscr;" ); // Lagrangian

  vars_list   =  (IDENTIFIER_NODE*)NULL;
  vars_list   =  AddIdentifier( vars_list,(U8*)"&theta;" );

  theequation =  0;
  thesection  =  1;
  thechapter  =  1;

  p_userprefs =  preferences;

  p_anomalies =  NULL;	// passed in with each tree to be translated

  math_field_ID =  MF_UNKNOWN;

  renderer_implements_displays    =  TRUE;
  renderer_implements_baselining  =  TRUE;
  renderer_implements_mlabeledtr  =  TRUE;
  do_equation_numbers             =  TRUE;
  output_entities_as_unicodes     =  0;
  stretch_for_long_arrows         =  TRUE;
  mo_spacing_mode =  2;         // 1 - don't script,
  spacing_mode_in_scripts =  2; // 2 - get from MathML.gmr
                                // 3 - use TeX algorithm

  zMMLStyleAttrs      =  NULL;    // mathcolor="red" mathbackground="transparent"
  zMMLFunctionAttrs   =  NULL;    // mathcolor="gray"
  zMMLUnitAttrs       =  NULL;    // mathcolor="green"
  zMMLTextAttrs       =  NULL;    // mathcolor="black"
  zMMLHyperLinkAttrs  =  NULL;    // mathcolor="gray"


// There are 2 ways to configure some aspects of our MathML output.
//  Callers can pass data in as a array of zstrs, "upref_zstrs".
//  OR the data can be read from MathML.gmr.
// Most of the config data is used by this object or by MMLtiler.

  if ( upref_zstrs ) {

    if ( p_userprefs )
      mml_version =  p_userprefs->mml_version;


// namespace used to qualify MathML tokens <uID12.1.2> slot 2
// mml:  used by MMLtiler

//<uID12.1.5>mathcolor="red" mathbackground="transparent" <uID12.1.5> slot 3
// current value is empty
    U16 zln =  upref_zstrs[4] ? strlen( upref_zstrs[4] ) : 0;
    if ( zln ) {
	  zMMLStyleAttrs =  TCI_NEW( char[zln+1] );
      strcpy( zMMLStyleAttrs,upref_zstrs[4] );
    }

// Attributes on an <mi> to indicate a function name <uID12.1.6>
// mathcolor="gray"
    zln =  upref_zstrs[5] ? strlen( upref_zstrs[5] ) : 0;
    if ( zln ) {
	  zMMLFunctionAttrs =  TCI_NEW( char[zln+1] );
      strcpy( zMMLFunctionAttrs,upref_zstrs[5] );
    }

// Attributes on an <mtext> or <mi> to indicate a unit name <uID12.1.7>
// mathcolor="green"
    zln =  upref_zstrs[6] ? strlen( upref_zstrs[6] ) : 0;
    if ( zln ) {
	  zMMLUnitAttrs =  TCI_NEW( char[zln+1] );
      strcpy( zMMLUnitAttrs,upref_zstrs[6] );
    }

// Attributes on an <mtext> embedded in MATH - EmbeddedText <uID12.1.8>
// mathcolor="black"
    zln =  upref_zstrs[7] ? strlen( upref_zstrs[7] ) : 0;
    if ( zln ) {
	  zMMLTextAttrs =  TCI_NEW( char[zln+1] );
      strcpy( zMMLTextAttrs,upref_zstrs[7] );
    }

// Attributes on an <mtext> for hyper objects <uID12.1.9>
// mathcolor="green"
    zln =  upref_zstrs[8] ? strlen( upref_zstrs[8] ) : 0;
    if ( zln ) {
	  zMMLHyperLinkAttrs =  TCI_NEW( char[zln+1] );
      strcpy( zMMLHyperLinkAttrs,upref_zstrs[8] );
    }

// renderer implements Baselining flag <uID12.1.10>
// true
    const char* d_template;
    if ( upref_zstrs[9] ) {
      d_template  =   upref_zstrs[9];
      zln =  strlen( d_template );
      if ( d_template[0] == 'f' || d_template[0] == 'F' )
        renderer_implements_baselining  =  FALSE;
    }
// output equation tags flag ( uses <mlabeledtr> ) <uID12.1.11>
// false
    if ( upref_zstrs[10] ) {
      d_template  =   upref_zstrs[10];
      zln =  strlen( d_template );
      if ( d_template[0] == 'f' || d_template[0] == 'F' )
        renderer_implements_mlabeledtr  =  FALSE;
    }
// output equation numbers flag ( uses <mlabeledtr> ) <uID12.1.12>
// false
    if ( upref_zstrs[11] ) {
      d_template  =   upref_zstrs[11];
      if ( d_template[0] == 'f' || d_template[0] == 'F' )
        do_equation_numbers =  FALSE;
    }
// output entities as unicodes, 0 - &alpha;, 1 - &#945;, 2 - &#x3B1; <uID12.1.13>
// 0
    if ( upref_zstrs[12] ) {
      d_template  =   upref_zstrs[12];
      output_entities_as_unicodes =  atoi( (char*)d_template );
    }
// control lspace and rspace on <mo>s, 1-don't script, 2-get from MathML.gmr, 3-TeX <uID12.1.14>
// 1
    if ( upref_zstrs[13] ) {
      d_template  =   upref_zstrs[13];
      mo_spacing_mode =  atoi( (char*)d_template );
    }
// control lspace and rspace on <mo>s in scripts, 1-don't script, 2-get from MathML.gmr, 3-TeX <uID12.1.15>
// 1
    if ( upref_zstrs[14] ) {
      d_template  =   upref_zstrs[14];
      spacing_mode_in_scripts =  atoi( (char*)d_template );
    }
// format for equation numbers - side attribute comes from 5.701.4 <uID12.1.16> (%thechapter%.%thesection%.%theequation%) <uID12.1.16>
// (%theequation%)

// format string for equation numbers
    if ( upref_zstrs[15] ) {
      d_template  =   upref_zstrs[15];
    }

// script long arrows as stretched versions of short arrows <uID12.1.17>
// false
    if ( upref_zstrs[16] ) {
      d_template  =   upref_zstrs[16];
      if ( d_template[0] == 'f' || d_template[0] == 'F' )
        stretch_for_long_arrows  =  FALSE;
    }
// indent increment for nested MML output <uID12.1.18>
// 2

  } else {

// Configure our MathML output based on data
//  from MathML.gmr :: MMLCONTEXT section

    U16 id  =  1;
    while ( TRUE ) {
      U8 zuID[32];
      UidsTozuID( 12,1,id,(U8*)zuID );

      U8* dest_zname;
      U8* d_template;
      if ( d_mml_grammar->GetGrammarDataFromUID(
					               zuID,(U8*)"MMLCONTEXT",
   							       &dest_zname,&d_template) ) {
        if ( d_template ) {
          U16 zln =  strlen( (char*)d_template );
          switch (id ) {
            case  1 :
		      mml_version =  atoi( (char*)d_template );
            break;
            case  2 : // mml token prefix - "mml:"
            // MMLTiler uses this data
            break;
            case  3 :
              if ( d_template[0] == 'f' || d_template[0] == 'F' )
                renderer_implements_displays  =  FALSE;
            break;
            case  4 :
            // not used
            break;
            case  5 :
              if ( zln ) {
	            zMMLStyleAttrs =  TCI_NEW( char[zln+1] );
                strcpy( zMMLStyleAttrs,(char*)d_template );
              }
            break;
            case  6 :   //
              if ( zln ) {
	            zMMLFunctionAttrs =  TCI_NEW( char[zln+1] );
                strcpy( zMMLFunctionAttrs,(char*)d_template );
              }
            break;
            case  7 :   //
              if ( zln ) {
	            zMMLUnitAttrs =  TCI_NEW( char[zln+1] );
                strcpy( zMMLUnitAttrs,(char*)d_template );
              }
            break;
            case  8 :   // 
              if ( zln ) {
	            zMMLTextAttrs =  TCI_NEW( char[zln+1] );
                strcpy( zMMLTextAttrs,(char*)d_template );
              }
            break;
            case  9 :   // 
              if ( zln ) {
	              zMMLHyperLinkAttrs =  TCI_NEW( char[zln+1] );
                strcpy( zMMLHyperLinkAttrs,(char*)d_template );
              }
            break;
            case 10 :
              if ( d_template[0] == 'f' || d_template[0] == 'F' )
                renderer_implements_baselining  =  FALSE;
            break;
            case 11 :
              if ( d_template[0] == 'f' || d_template[0] == 'F' )
                renderer_implements_mlabeledtr  =  FALSE;
            break;
            case 12 :
              if ( d_template[0] == 'f' || d_template[0] == 'F' )
                do_equation_numbers =  FALSE;
            break;
            case 13 :
              output_entities_as_unicodes =  atoi( (char*)d_template );
            break;
            case 14 :
		      mo_spacing_mode =  atoi( (char*)d_template );
            break;
            case 15 :
		      spacing_mode_in_scripts =  atoi( (char*)d_template );
            break;
            case 16 :
// format string for equation numbers
            break;
            case 17 :
              if ( d_template[0] == 'f' || d_template[0] == 'F' )
                stretch_for_long_arrows  =  FALSE;
            break;
            case 18 :
            break;

            default :
              TCI_ASSERT(0);
            break;
          }

        }   // if ( d_template )

      } else    // Normal loop exit - out of data
	    break;

      id++;
	}         // Loop thru MathML.gmr :: MMLCONTEXT

  }

// Load entities for "InvisibleTimes" and "InvisibleComma".
//&InvisibleComma;<uID3.17.200>infix,2,U02063,separator="true"
//&InvisibleTimes;<uID3.17.201>infix,39,U02062

  entity_ic[0]  =  0;
  entity_ic_unicode[0]  =  0;
  AppendEntityToBuffer( (U8*)"3.17.200",entity_ic,entity_ic_unicode );
  entity_it[0]  =  0;
  entity_it_unicode[0]  =  0;
  AppendEntityToBuffer( (U8*)"3.17.201",entity_it,entity_it_unicode );
}



LaTeX2MMLTree::~LaTeX2MMLTree() {

  DisposeIdentifierList( funcs_list );
  DisposeIdentifierList( vars_list );
  if ( zMMLStyleAttrs )
    delete zMMLStyleAttrs;
  if ( zMMLFunctionAttrs )
    delete zMMLFunctionAttrs;
  if ( zMMLUnitAttrs )
    delete zMMLUnitAttrs;
  if ( zMMLTextAttrs )
    delete zMMLTextAttrs;
  if ( zMMLHyperLinkAttrs )
    delete zMMLHyperLinkAttrs;
}


// Convert a NBLaTeX parse tree into an equivalent MML parse tree.
// At present, the starting "LaTeX_parse_tree" is the root of a
//  tree from the parse of $Math, $$Math, or an eqnarray.

TNODE* LaTeX2MMLTree::NBLaTeXTreeToMML( TNODE* LaTeX_parse_tree,
									      MATH_CONTEXT& mc,
  										  ANOMALY_REC* anomalies,
			  							  U16& error_code ) {

  
  TNODE* mml_rv =  NULL;
  error_code  =  0;
  p_anomalies =  anomalies;		// list head is passed in.


  TNODE* mml_tree  =  TranslateMathObject( LaTeX_parse_tree );

  p_anomalies =  NULL;	// the caller now owns this list.

// Add "context" attributes, mathcolor, etc.

  if ( mml_tree ) {

    if ( zMMLStyleAttrs ) {
// mstyle<uID5.600.0>!mstyle!BUCKET(5.600.2,MATH,,,/mstyle,)!/mstyle!
      U16 uobjtype,usubtype,uID;
      GetUids( mml_tree->zuID,uobjtype,usubtype,uID );
      if ( uobjtype != 5 || usubtype != 600 || uID != 0 ) {
        mml_tree  =  FixImpliedMRow( mml_tree );
        mml_tree  =  CreateElemWithBucketAndContents( 5,600,0,2,mml_tree );
        SetDetailNum( mml_tree,DETAILS_style,2 );
      }

      SetMMLAttribs( mml_tree,(U8*)zMMLStyleAttrs );
	}

    mml_rv =  MMLlistToMRow( mml_tree );
  }


  return mml_rv;
}


// "math_container_obj" is a LaTeX tree ( Notebook.gmr )
//  from the parse of $Math, $$Math, or an eqnarray.
// Note that both TCI internal format and external LaTeX
//	as scripted by SWP are accepted as sources.

TNODE* LaTeX2MMLTree::TranslateMathObject( TNODE* math_container_obj ) {
  //JBMLine("\nTranslateMathObject");
  TNODE* mml_rv =  NULL;

  U16 uobjtype,usubtype,uID;
  GetUids( math_container_obj->zuID,uobjtype,usubtype,uID );

  if ( uobjtype == 5 ) {

    TNODE* out_of_flow_list =  NULL;

    switch ( usubtype ) {
	  case  TCMD_DollarMath  : 		// $x$
		if ( uID == 0 ) {
          TNODE* local_oof_list =  NULL;
          script_level  =  0;
          
          TNODE* mml_cont =  TranslateTeXDollarMath( math_container_obj,
								                    &local_oof_list );
          
          script_level  =  0;

          U16 vspace_context  =  renderer_implements_baselining ? 2 : 1;
        // 1 - pass vspace up to caller
        // 2 - nest mml_cont in <mpadded>
          mml_cont  =  HandleOutOfFlowObjects( mml_cont,
    						&local_oof_list,&out_of_flow_list,vspace_context );

          mml_rv =  MMLlistToMRow( mml_cont );
          mml_rv->src_linenum =  math_container_obj->src_linenum;
		}

// We have translated the contents of the TeX inline.
//  It remains to nest it in an <mstyle> marked for "inline".

        if ( mml_rv ) {
          mml_rv  =  FixImpliedMRow( mml_rv );
/*  If all inline math is to be stylized, get attributes from MathML.gmr
          mml_rv  =  CreateElemWithBucketAndContents( 5,600,0,2,mml_rv );
          SetDetailNum( mml_rv,DETAILS_style,2 );
          SetNodeAttrib( mml_rv,(U8*)"displaystyle",(U8*)"false" );
          SetNodeAttrib( mml_rv,(U8*)"scriptlevel",(U8*)"0" );
*/
        }
	  break;

	  case  TCMD_DisplayedMath  : 	// \begin{mathdisplay}, etc.
	    if ( uID==1 || uID==11 || uID==21 || uID==31 || uID==41 ) {
          script_level  =  0;
		      in_display  =  TRUE;
          mml_rv  =  TranslateTeXDisplay( math_container_obj, uID, &out_of_flow_list );
		      in_display  =  FALSE;
          script_level  =  0;

          if (out_of_flow_list){
            // move leftover labels to anomaly list
            TNODE* lis = out_of_flow_list;
          
            if (strcmp((const char*) lis->src_tok, "\\label") == 0 ) {
               U8* marker =  lis->parts->contents->var_value;
               RecordAnomaly( 1005, NULL, lis->src_offset1, lis->src_offset2 );
               out_of_flow_list = lis->next;
            }
          }

// We have translated the contents of the TeX display.  It remains
//  to nest it in an <mstyle> marked for "display".  There are other
//  issues to be considered, like labels and tags (numbering in general).
//	The only provision for tagging in MathML is <mlabeledtr>
//  within <mtable>.

          if ( mml_rv ) {
            mml_rv  =  FixImpliedMRow( mml_rv );
/*  If all displayed math is to be stylized, get attributes from MathML.gmr
            mml_rv  =  CreateElemWithBucketAndContents( 5,600,0,2,mml_rv );
            SetDetailNum( mml_rv,DETAILS_style,2 );
            SetNodeAttrib( mml_rv,(U8*)"displaystyle",(U8*)"true" );
            SetNodeAttrib( mml_rv,(U8*)"scriptlevel",(U8*)"0" );
*/
		  }
	    }
	  break;

// \EQN<uID5.39.0>!\EQN!_ENUMID__NCOLS__MLLABEL__MATHLETTERS__EQNROWS_
	  case  39  :
	    if ( uID==0 ) {
// Utility to generate external tree from internal version.
          TNODE* eqnarray =  EQNtoeqnarray( math_container_obj,
                                                usubtype );

          U16 uobj,usub,id;
          GetUids( eqnarray->zuID,uobj,usub,id );
          if ( usub>=140 && usub<=143 ) {
// These LaTeX envs differ from other eqnarray-related envs.
// They occur only nested in MATH (displays).
// Internally, SWP makes the following translations
//   \begin{equation*}\begin{split}  <--> \EQN{6}...
            TNODE* local_oof_list =  NULL;
		    if ( usub == TENV_cases ) {
              mml_rv  =  LaTeXCases2MML( eqnarray,&local_oof_list );
            } else {
              mml_rv  =  NestedTeXEqnArray2MML( eqnarray,
                                        &local_oof_list,usub );
            }
            if ( mml_rv ) {
              mml_rv  =  FixImpliedMRow( mml_rv );
/*  If all displayed math is to be stylized, get attributes from MathML.gmr
              mml_rv  =  CreateElemWithBucketAndContents( 5,600,0,2,mml_rv );
              SetDetailNum( mml_rv,DETAILS_style,2 );
              SetNodeAttrib( mml_rv,(U8*)"displaystyle",(U8*)"true" );
              SetNodeAttrib( mml_rv,(U8*)"scriptlevel",(U8*)"0" );
*/
		    }
          } else
            mml_rv  =  TranslateMathObject( eqnarray );

          DisposeTList( eqnarray );
		}
	  break;

	  case TENV_eqnarray  :
	  case TENV_eqnarraystar  :
	  case 123  :		// TENV_align          123
	  case 124  :		// \begin{align*}<uID5.124.0>!
	  case 125  :		// TENV_alignat        125
	  case 126  :		// TENV_alignatstar    126
	  case 127  :		// TENV_xalignat       127
	  case 128  :		// TENV_xalignatstar   128
	  case 129  :		// TENV_xxalignat      129
	  case 130  :		// TENV_xxalignatstar  130
	  case 131  :		// TENV_gather         131
	  case 132  :		// TENV_gatherstar     132
	  case 133  :		// TENV_multline       133
	  case 134  : 	// TENV_multlinestar   134
    //JBMLine("\nTranslateMathObject A");
		if ( uID == 0 ) {
          script_level  =  0;
		      in_display  =  TRUE;
          mml_rv  =  TranslateTeXEqnArray( math_container_obj,
   							                &out_of_flow_list,usubtype );
          mml_rv  =  AddEQNAttribs( mml_rv,usubtype );
		  in_display  =  FALSE;
          script_level  =  0;
		}
    
	  break;

	  case 135  :
	  case 136  :
	  case 137  :
	  case 138  :
	  case 139  :
	    if ( uID == 0 )	{	// these don't exist
		    TCI_ASSERT(0);
      }
	    break;

	  case 140  :		    // TENV_cases          140
	  case 141  :		    // TENV_split          141
	  case 142  :		    // TENV_gathered       142
	  case 143  : 	        // TENV_aligned        143
	    if ( uID == 0 )	{	// these are MATH only
		  TCI_ASSERT(0);	// not level 1 objects
	    break;				// must be in $'s, $$'s
    }

    default : {
	    TCI_ASSERT(0);	// un-expected object at level 1.
    }
	  break;
	}

    TCI_ASSERT( out_of_flow_list == NULL );

  } else	// if ( uobjtype == 5 )
    TCI_ASSERT(0);

  //JBMLine("\nEnd TranslateMathObject");
  return mml_rv;
}


// This is the recursive function called
//  to process each LaTeX MATH contents list, ie MATH bucket.
// We accumulate all out-of-flow objects (labels,vspace,struts,comments)
//  encountered in the bucket, and pass them back to the caller
//  thru the "out_of_flow_list" parameter.
// Caller MUST handle the out_of_flow objects.

TNODE* LaTeX2MMLTree::TranslateMathList( 
                            TNODE* LaTeX_list,
  										      TCI_BOOL do_bindings,
 								            MATH_CONTEXT_INFO* m_context,
                            U16& tex_nodes_done,
                            U16& error_code,
                            TNODE** out_of_flow_list ) {


  tex_nodes_done  =  0;		// first level LaTeX nodes processed
  error_code      =  0;

  TCI_ASSERT( *out_of_flow_list == NULL );

// Warning! nodes in LaTeX parse tree are modified here.

  UnicodesToSymbols( LaTeX_list );

  TNODE* MML_rv   =  NULL;

  TNODE* oof_list =  NULL;	// local out_of_flow_list
  TNODE* curr_mml_node;
  TNODE* last_mml_node  =  NULL;

  TNODE* rover  =  LaTeX_list;
  while ( rover ) {    		// Loop thru first level objects
    						//  in source "LaTeX_parse_tree"
    curr_mml_node   =  NULL;
    U16 local_nodes_done  =  1;	// assumed - set if otherwise
    TNODE* save_next  =  NULL;
	  TCI_BOOL use_save_next  =  FALSE;

    U16 uobjtype,usubtype,uID;
    GetUids( rover->zuID,uobjtype,usubtype,uID );
    
    switch ( uobjtype ) {       // switch on primary object type
	  case 0  :
      case 1  :	  // Context Start
      case 2  :   // Context End
        TCI_ASSERT( 0 );
      break;

      case 3  :   // LaTeX Symbol
        if ( usubtype==17 && uID==106 ) {	// &
// maligngroup<uID9.20.0>
// mmltiler has special handling for <uID9.d.d> objects
  		  curr_mml_node =  MakeTNode( 0L,0L,0L,(U8*)"9.20.0" );
// mark this node as pure whitespace
          SetDetailNum( curr_mml_node,DETAILS_space_width,0 );
        } else
          curr_mml_node =  MathSymbolToMML( rover,LaTeX_list,
							            &oof_list,local_nodes_done );
      break;

      case 4  :   // Accent - math OR a bold node
        if ( usubtype==3 )
// \BF<uID4.3.1>!\BF!!{!VAR(4.3.50,MATH,a,,})!}!
          curr_mml_node =  BoldSymbolToMML( rover,local_nodes_done );
        else
          curr_mml_node =  MathAccToMML( rover,uID,&oof_list );
      break;

      case 5 : {
        if ( usubtype>=550 && usubtype<=560 && uID==0 ) {
          use_save_next =  TRUE;
          save_next   =  rover->next;
          HyperObj2MML( &oof_list,rover,usubtype,TRUE,NULL );
          local_nodes_done  =  1;
        } else if ( usubtype==700 && uID==0 ) {
	        use_save_next =  TRUE;
          save_next   =  rover->next;
          oof_list  =  MoveNodeToList( oof_list,rover );
          local_nodes_done  =  1;
		   } else {
	        use_save_next =  TRUE;
          save_next =  rover->next;
          curr_mml_node =  MathStructureToMML( rover,&oof_list,
          					m_context,local_nodes_done,error_code );
          U16 count =  1;
          while ( save_next && count < local_nodes_done ) {
            count++;
            save_next =  save_next->next;
          }  
        }  
      }
      break;

	  case   6  :		// a { group } in MATH
	    if ( usubtype==1 && uID==0 ) {
// {<uID6.1.0>BUCKET(6.1.2,INHERIT,{,},,)
		  if ( rover->parts && rover->parts->contents ) {
			TNODE* TeX_cont =  rover->parts->contents;
			U8 switch_nom[64];
            U16 switch_tag  =  GetMathRunTagFromTeXSwitch(
       								    TeX_cont,switch_nom );
			if ( switch_tag ) {
    // {\large x = 1}
			  curr_mml_node =  TaggedMath2MML( rover,TeX_cont,
	  		  					          switch_tag,switch_nom,
	  		  							  &oof_list );
			} else {

	// Here we're just dropping the tag.
	// We translate the contents, leaving all binding for later.

			  TCI_BOOL do_bindings  =  FALSE;
        U16 tex_nodes_done,error_code;
			  TNODE* local_oof_list =  NULL;
        TNODE* mml_cont =  TranslateMathList( TeX_cont,
							        do_bindings,NULL,tex_nodes_done,
							        error_code,&local_oof_list );
	      if ( !mml_cont ) {
		       TCI_ASSERT(0);
        }
			  if ( local_oof_list ) {
			    if ( oof_list ) {
				  TNODE* tail =  oof_list;
				  while ( tail->next )
				    tail  =  tail->next;
				  tail->next  =  local_oof_list;
				  local_oof_list->prev  =  tail;
				} else
				  oof_list  =  local_oof_list;
			  }
              curr_mml_node =  mml_cont;
			}
		  }
		} else {
		  TCI_ASSERT(0);
    }
	  break;

      case 7  :   // Big (prefix) operator - integral, summation, etc.
        curr_mml_node =  BigOp2MML( rover,local_nodes_done,&oof_list );
      break;

      case 8  :   // Function name, "\limfunc", "\func", etc.
        curr_mml_node =  Function2MML( rover,usubtype,uID,
							              local_nodes_done,&oof_list );
      break;

      case 9  :   // White space
        if ( usubtype == 3 ) {		// vertical space
	      use_save_next =  TRUE;
          save_next =  rover->next;
          oof_list  =  MoveNodeToList( oof_list,rover );
       	  local_nodes_done  =  1;
		} else                    // all non-vertical spacing elements
          curr_mml_node =  LaTeXHSpacing2MML( rover,usubtype,uID,
                                            local_nodes_done );
      break;

      case 777  : // LaTeX comment - possibly %TCIMACRO{}
        curr_mml_node =  MathComment2MML( rover,local_nodes_done );
      break;

      default :
		//TCI_ASSERT(0);
      break;

    }       // switch ( uobjtype )

// Append the "curr_mml_node" to the MML_rv list

    if ( curr_mml_node ) {
      if ( !MML_rv )
        MML_rv  =  curr_mml_node;
      else {
        last_mml_node->next =  curr_mml_node;
        curr_mml_node->prev =  last_mml_node;
	  }

      last_mml_node =  curr_mml_node;
      while ( last_mml_node->next )
        last_mml_node =  last_mml_node->next;
    }		// if ( curr_mml_node )

// Advance TeX list pointer, rover

    if ( use_save_next ) {
      rover =  save_next;
	} else {
      U16 count =  0;
      while ( rover && count < local_nodes_done ) {
        count++;
        rover =  rover->next;
      }  
    }  

    tex_nodes_done  +=  local_nodes_done;

  }     // while loop thru first level objects on "obj_list"


// MML_rv now points to a MathML list that has been generated
//  from the source LaTeX contents list.  This MathML
//  list needs many fix-ups - insert invisible (ie. implied)
//  operators, handle operator precedence, etc.

  if ( do_bindings && MML_rv )
    MML_rv  =  FinishMMLBindings( MML_rv );

  if ( oof_list ) {
    if ( *out_of_flow_list ) {
      TNODE* tail =  *out_of_flow_list;
	    while ( tail->next )
	      tail  =  tail->next;
	    tail->next  =  oof_list;
    } else
      *out_of_flow_list =  oof_list;
  }
  return MML_rv;
}


// "tex_sym_node" represents something like "-".  The next node
//  may represent something like "=".  In this case we may want
//  to coelesce the two nodes into a single MML operator.

// Note that coelescing consecutive LaTeX operator symbols
//  is controlled by data in "MathML.gmr".  We catenate the
//  LaTeX tokens for the operators and perform a 'lookup by name'
//  in s_mml_grammar.  If the lookup succeeds we script
//  a compound operator in MML.

// Sample line from MathML.gmr -
// <=<uID3.21.12>&lt;=,infix,26,

TCI_BOOL LaTeX2MMLTree::IsMultiTeXSymOp( TNODE* tex_sym_node,
											U8** zop_info,
											U16& advance ) {

  TCI_BOOL rv =  FALSE;
  *zop_info   =  NULL;
  advance =  0;

  U8* gmr_section =  (U8*)"MULTISYMOPS";
  U8* zform   =  NULL;
  U8* op_zuID;

  U8 TeX_token_zstr[80];
  TeX_token_zstr[0]  =  0;
  U16 advance1,advance2,advance3;
  if ( GetLaTeXOpSymbol(tex_sym_node,TeX_token_zstr,advance1) ) {
// Got first mml op
    TNODE* nn =  tex_sym_node->next;

    if ( GetLaTeXOpSymbol(nn,TeX_token_zstr,advance2) ) {
// Got second mml op
      U16 save_ln =  strlen( (char*)TeX_token_zstr );
      TNODE* nnn  =  nn->next;
	  if ( advance2==2 )
        nnn  =  nnn->next;
	  TCI_BOOL is_triple =  FALSE;
      if ( GetLaTeXOpSymbol(nnn,TeX_token_zstr,advance3) ) {
// Got third mml op
        U16 nln   =  strlen( (char*)TeX_token_zstr );
        if ( s_mml_grammar->GetGrammarDataFromNameAndAttrs(
    							(U8*)TeX_token_zstr,nln,zform,
                                gmr_section,&op_zuID,zop_info) ) {
// Lookup succeeds for triple, as in "..."
          advance =  advance1 + advance2 + advance3;
          is_triple =  TRUE;
          rv  =  TRUE;
		}
	  }
	  if ( !is_triple ) {
        TeX_token_zstr[save_ln] =  0;
        if ( s_mml_grammar->GetGrammarDataFromNameAndAttrs(
   							(U8*)TeX_token_zstr,save_ln,zform,
                                gmr_section,&op_zuID,zop_info) ) {
// Lookup succeeds for double, as in "++"
          advance =  advance1 + advance2;
          rv  =  TRUE;
		} else {
          // jcs if ( !strncmp((char*)TeX_token_zstr,"\\not",4) )
          // jcs   TCI_ASSERT(0);                                                          
		}

	  }

	}		// if ( GetLaTeXOpSymbol(nn,TeX_token_zstr,advance2) )

  }// jcs  else
   // jcs  TCI_ASSERT(0);

  return rv;
}



// Utility to catenate the LaTeX symbol for an operator
//  to a string of operator symbols - "<=..."

TCI_BOOL LaTeX2MMLTree::GetLaTeXOpSymbol( TNODE* tex_node,
									  	    U8* zTeX_tokens,
										    U16& nodes_done ) {

  TCI_BOOL rv =  FALSE;
  U16 local_advance =  0;
  if ( tex_node ) {
// Step over narrow white space.
    U16 objclass,subclass,id;
    GetUids( tex_node->zuID,objclass,subclass,id );
    if ( objclass==9 && tex_node->next ) {	// space
	  TCI_BOOL omit_space =  FALSE;
      if        ( subclass==1 ) {
	    if ( id==3 || id==6 || id==8 || id==10 )
	      omit_space  =  TRUE;
      } else if ( subclass==5 ) {
	    if ( id==3 )
	      omit_space  =  TRUE;
	  }
	  if ( omit_space ) {
        tex_node  =  tex_node->next;
        local_advance++;
	  } else
        tex_node  =  NULL;
    }

// If we're looking at an operator, get it's TeX symbol.

    if ( tex_node ) {
      TCI_BOOL forces_geometry;
      U16 ilk =  ClassifyTeXSymbol( tex_node,NULL,
                                    forces_geometry,NULL );
	  if ( ilk == MML_NUMBER ) {
        U16 obj,sub,id;
        GetUids( tex_node->zuID,obj,sub,id );
        if ( obj==3 && sub==17 && id==88 )	// "."
	      ilk =  MML_OPERATOR;
	  }
	  if ( ilk == MML_OPERATOR ) {
        strcat( (char*)zTeX_tokens,(char*)tex_node->src_tok );
        local_advance++;
        rv  =  TRUE;
      }		// if ( ilk == MML_OPERATOR )
    }	  // if ( tex_node )
  }	    // if ( tex_node )

  nodes_done =  rv ? local_advance : 0;

  return rv;
}


// Given a LaTeX symbol TNODE (ie. <uID3.?.?>),
//  generate an MML <mi>, <mn>, <mo>, <mtext>, or <mspace> TNODE.

TNODE* LaTeX2MMLTree::MathSymbolToMML( TNODE* tex_symbol_node,
                                        TNODE* LaTeX_list,
										TNODE** out_of_flow_list,
				  						U16& tex_nodes_done ) {

  tex_nodes_done  =  0;

  TNODE* mml_rv   =  MakeTNode( 0L,0L,tex_symbol_node->src_linenum,NULL );

// Does the LaTeX symbol represent (the start) of a number,
//  (the start) of an operator,
//  (the start) or an identifier?

  TCI_BOOL forces_geometry;
  U16 symbol_mml_ilk  =  ClassifyTeXSymbol( tex_symbol_node,
                                LaTeX_list,forces_geometry,mml_rv );

  U16 mml_elem_ID =  0;
  if ( symbol_mml_ilk == MML_IDENTIFIER ) {
    DisposeTNode( mml_rv );
    mml_rv  =  GetIdentifier( tex_symbol_node,
                        forces_geometry,tex_nodes_done );
    SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );
  } else if ( symbol_mml_ilk == MML_NUMBER ) {
	mml_elem_ID =  202;		// <mn>
    GetNumberStr( tex_symbol_node,out_of_flow_list,
    						mml_rv,tex_nodes_done );

  } else if ( symbol_mml_ilk == MML_OPERATOR ) {
	mml_elem_ID =  203;		// <mo>
    SetMMLOpNode( tex_symbol_node,tex_nodes_done,mml_rv );

  } else if ( symbol_mml_ilk == MML_TEXT ) {

  } else if ( symbol_mml_ilk == MML_SPACE ) {
	TCI_ASSERT(0);

  } else {
// When a LaTeX symbol can't be categorized as an
//  <mo>, <mi>, or <mn> we translate it to <mtext>
    symbol_mml_ilk  =  MML_TEXT;
  }

  if ( symbol_mml_ilk == MML_TEXT ) {
    mml_elem_ID =  204;		// <mtext>

    U8 entity_buffer[128];
    U8 unicode_buffer[128];
    entity_buffer[0]  =  0;
    unicode_buffer[0] =  0;
    AppendEntityToBuffer( tex_symbol_node->zuID,entity_buffer,
                                              unicode_buffer );
    if ( entity_buffer[0] ) {
      SetChData( mml_rv,entity_buffer,unicode_buffer );
    } else {
	  TCI_ASSERT(0);
      SetChData( mml_rv,(U8*)"???",NULL );
	}
    tex_nodes_done  =  1;
  }

// Set the zuID ( <mi>, <mn>, <mo>, <mtext> ) of the returned TNODE

  if ( mml_elem_ID ) {
	U8 zuID[32];
    UidsTozuID( 3,mml_elem_ID,1,(U8*)zuID );
    strcpy( (char*)mml_rv->zuID,(char*)zuID );
  }

  return mml_rv;
}



TNODE* LaTeX2MMLTree::TEXBUTTON2MML(TNODE* texb) {
  U8* texb_name = NULL;
  U8* texb_content = NULL;
  TNODE* mml_cdata = NULL;

  if ( texb && texb->parts ) {
    
    // Get the name
    TNODE* name_bucket =  FindObject( texb->parts, (U8*)"5.418.2", INVALID_LIST_POS );
    if ( name_bucket && name_bucket->contents ) {
       texb_name = name_bucket -> contents -> var_value;
    }

    // Get the tex
    TNODE* tex_bucket =  FindObject( texb->parts, (U8*)"5.418.3", INVALID_LIST_POS );
    if ( tex_bucket && tex_bucket->contents ) {
       texb_content = tex_bucket -> contents -> var_value;
       
       // need to make a <!CDATA ...
       mml_cdata = MakeTNode( 0L, 0L, 0L, (U8*)"4.418.5" );
       char* buf = new char[1+ strlen((const char*)texb_content) + strlen("<![CDATA[]]>")];
       strcpy(buf, "<![CDATA[");
       strcpy(buf + strlen("<![CDATA["), (const char*)texb_content);
       strcpy(buf +  strlen("<![CDATA[") + strlen((const char*)texb_content), "]]>"); 
       SetChData( mml_cdata, (U8*)buf, NULL ); 
    }
    // put the name and tex into a mml object

    TNODE* mml_rv = CreateElemWithBucketAndContents( 5, 418, 1, 4, mml_cdata );
    SetNodeAttrib( mml_rv, (U8*)"enc", (U8*)"1" );
    SetNodeAttrib( mml_rv, (U8*)"name", texb_name );

    return mml_rv;
   

  }
    
  return NULL;
}

// MML_parse_list has been generated by a one-to-one mapping
//  of LaTeX objects to their MathML counterparts. 
// Here we embellish the raw MathML by identifying operators
//  and adding form and precedence details.

TNODE* LaTeX2MMLTree::AddOperatorInfo( TNODE* MML_parse_list ) {

  TNODE* rv =  MML_parse_list;

// We traverse the mml list, adding form and precedence
//  information to all <mo> nodes.

  TCI_BOOL in_script  =  GetScriptStatus();

  TNODE* MML_rover  =  MML_parse_list;
  while ( MML_rover ) {
    U16 uobjtype,usubtype,uID;
    GetUids( MML_rover->zuID,uobjtype,usubtype,uID );

    if ( uobjtype==3 ) {	// non-structured

      switch ( usubtype ) {
        case 201  :  			//EID_mi	  mi<uID3.201.1>
		break;
        case 202  :  			//EID_mn	  mn<uID3.202.1>
		break;
        case 203  :  			//EID_mo	  mo<uID3.203.1>
          if ( !MML_rover->details
          ||   MML_rover->details->form==UNDEFINED_DETAIL )
            SetMultiformOpAttrs( MML_rover,MML_rover );

          if ( in_script ) {
            if ( spacing_mode_in_scripts == 3 )
              SetTeXOperatorSpacing( MML_rover,MML_rover,in_script );
          }	else {
            if ( mo_spacing_mode == 3 )   // 3 - emulate TeX spacing
              SetTeXOperatorSpacing( MML_rover,MML_rover,in_script );
          }

		break;
        case 204  :  			//EID_mtext	  mtext<uID3.204.1>
		break;
        case 205  :  			//EID_mspace  mspace<uID3.205.1>
		break;
        case 206  :  			//EID_ms	    ms<uID3.206.1>
		break;
		default   :
		  TCI_ASSERT(0);
		break;
	  }

    } else if ( uobjtype==5 ) {	// structured

// Here's where "embellished operators" are handled.

      if ( usubtype==750 && uID==1 ) {	// mrow
        if ( MML_rover->parts && MML_rover->parts->contents ) {
          TNODE* cont =  MML_rover->parts->contents;
          U16 uobj,usub,id;
          GetUids( cont->zuID,uobj,usub,id );
          if ( uobj==5 && usub==600 && id==0 ) {	// mstyle
            if ( cont->parts && cont->parts->contents ) {
              TNODE* cont2  =  cont->parts->contents;
              U16 uobjtype,usubtype,uID;
              GetUids( cont2->zuID,uobjtype,usubtype,uID );
              if ( uobjtype==3 && usubtype==203 ) {
                SetMultiformOpAttrs( MML_rover,cont2 );
                if ( in_script ) {
                  if ( spacing_mode_in_scripts == 3 )
                    SetTeXOperatorSpacing( MML_rover,cont2,in_script );
                }	else {
                  if ( mo_spacing_mode == 3 )   // 3 - emulate TeX spacing
                    SetTeXOperatorSpacing( MML_rover,cont2,in_script );
                }
              }

			}
		  }		// mstyle clause
		}

	  }	else if ( usubtype>=50 && usubtype<=55 && uID==2 ) {

      // possible embellished operator

// msub<uID5.50.2>reqELEMENT(5.50.3)
// msup<uID5.51.2>reqELEMENT(5.51.3)
// msubsup<uID5.52.2>reqELEMENT(5.52.3)
// munder<uID5.53.2>reqELEMENT(5.53.3)
// mover<uID5.54.2>reqELEMENT(5.54.3)
// munderover<uID5.55.2>reqELEMENT(5.55.3)

        if ( MML_rover->parts && MML_rover->parts->contents ) {
          TNODE* cont =  MML_rover->parts->contents;
          U16 uobj,usub,id;
          GetUids( cont->zuID,uobj,usub,id );
          if ( uobj==3 && usub==203 ) {
            if ( !MML_rover->details
            ||   MML_rover->details->form == UNDEFINED_DETAIL )
              SetMultiformOpAttrs( MML_rover,cont );

            if ( in_script ) {
              if ( spacing_mode_in_scripts == 3 )
                SetTeXOperatorSpacing( MML_rover,cont,in_script );
            } else {
              if ( mo_spacing_mode == 3 )   // 3 - emulate TeX spacing
                SetTeXOperatorSpacing( MML_rover,cont,in_script );
            }

		  }
		}
	  }

	}	// uobj == 5 clause

    MML_rover =  MML_rover->next;
  }		// loop thru mml list

  return rv;
}


// The form of a multiform op like + isn't determined
//  during the initial translation to MML.  It's done,
//  in a later pass, when more context is available.
// If the operator is embellished, MML_rover is the node
//  containing op_node.
// Otherwise, both args are the same - the op_node.

void LaTeX2MMLTree::SetMultiformOpAttrs( TNODE* MML_rover,
										    TNODE* op_node ) {

  U8* opname  =  op_node->var_value;
  U16 nln =  strlen( (char*)opname );

  if ( !strncmp((char*)opname,"TeX",3) )
    return;

// Some operators have more than 1 form - prefix, infix, or postfix.
// We need the "form" of an overloaded operator for grammar lookups.

  TCI_BOOL has_left_operand;
  TCI_BOOL has_right_operand;
  I16 left_space;
  I16 right_space;
  CheckForOperands( MML_rover,has_left_operand,has_right_operand,
  									left_space,right_space );

  U16 form_ID;
  U16 precedence;

  U16 forms_for_op;
  if ( IsMultiFormOp(opname,forms_for_op) ) {
    TCI_ASSERT(forms_for_op);

    if ( has_left_operand && has_right_operand ) {

      if      ( forms_for_op & 2 )
        form_ID =  OPF_infix;
      else if ( forms_for_op & 4 )
        form_ID =  OPF_prefix;
      else if ( forms_for_op & 1 )
        form_ID =  OPF_postfix;

      if ( opname[0] == '!' )
        form_ID =  OPF_postfix;

    } else if ( has_right_operand ) {

      if      ( forms_for_op & 4 )
        form_ID =  OPF_prefix;
      else if ( forms_for_op & 2 )
        form_ID =  OPF_infix;
      else if ( forms_for_op & 1 )
        form_ID  =  OPF_postfix;

    } else if ( has_left_operand ) {

      if      ( forms_for_op & 1 )
        form_ID =  OPF_postfix;
      else if ( forms_for_op & 2 )
        form_ID =  OPF_infix;
      else if ( forms_for_op & 4 )

        form_ID =  OPF_prefix;

    } else {

      if      ( forms_for_op & 2 )
        form_ID =  OPF_infix;
      else if ( forms_for_op & 4 )
        form_ID =  OPF_prefix;
      else if ( forms_for_op & 1 )
        form_ID =  OPF_postfix;
    }

    U8* op_zuID;		// for return info from lookups
    U8* op_zinfo;
    if ( s_mml_grammar->GetGrammarDataFromNameAndAttrs(
   						opname,nln,(U8*)zop_forms[form_ID],
                        (U8*)"MULTIFORMOPS",&op_zuID,&op_zinfo) ) {

// +<uID3.13.60>prefix,47,U0002B,lspace="mediummathspace" rspace="0"
// +<uID3.13.60>infix,28,U0002B,lspace="mediummathspace" rspace="mediummathspace"

// +<uID3.13.60>postfix,64,U0002B,lspace="0" rspace="mediummathspace"

	  SetNodeAttrib( op_node,(U8*)"form",(U8*)zop_forms[form_ID] );
	  OP_GRAMMAR_INFO op_record;
	  if ( op_zinfo && *op_zinfo ) {

        if ( output_entities_as_unicodes ) {
          U8 unicode_buffer[128];
          if ( GetUnicodeEntity(op_zinfo,unicode_buffer) )
            SetChData( op_node,NULL,unicode_buffer );
        }

        GetAttribsFromGammarInfo( op_zinfo,op_record );
        if ( op_record.attr_list ) {
		  MergeMOAttribs( op_node,op_record.attr_list );
          DisposeAttribs( op_record.attr_list );
          op_record.attr_list =  NULL;
        }

	  } else
	    TCI_ASSERT(0);

      precedence  =  op_record.precedence;
      SetDetailNum( MML_rover,DETAILS_form,form_ID );
      SetDetailNum( MML_rover,DETAILS_precedence,precedence );
	  if ( form_ID==OPF_prefix && precedence==54 )
        SetDetailNum( MML_rover,DETAILS_is_differential,1 );

	} else {
	  TCI_ASSERT(0);
      form_ID  =  0;
	}

  } else {		// Not a multi-form operator

    if ( op_node->details
    &&   op_node->details->form != UNDEFINED_DETAIL ) {
      form_ID  =  op_node->details->form;
	  precedence  =  op_node->details->precedence;
	  if ( MML_rover != op_node ) {
        SetDetailNum( MML_rover,DETAILS_form,form_ID );
        SetDetailNum( MML_rover,DETAILS_precedence,precedence );
	    if ( form_ID==OPF_prefix && precedence==54 )
          SetDetailNum( MML_rover,DETAILS_is_differential,1 );
	  }

	} else
	  TCI_ASSERT(0);

  }

// the following as a diagnostic

  if        ( form_ID==OPF_prefix ) {
	if ( precedence > 1 )
	  if ( MML_rover->next ) {
        TCI_ASSERT( has_right_operand );
    }
  } else if ( form_ID==OPF_infix ) {
    if ( has_left_operand + has_right_operand == 1 ) {
      TCI_ASSERT( 0 );
    }
  } else if ( form_ID==OPF_postfix ) {
	if ( precedence > 1 )
	  if ( MML_rover->prev ) {
        TCI_ASSERT( has_left_operand );
    }
  }

}



// Many of the narrow white space objects that occur in LaTeX MATH,
//  like "\," should be added to the left or right space attributes
//  of an adjacent <mo>.  That's done here.
// All LaTeX whitespace objects generate TNODEs which have 
//  details->space_width set during initial translation.

TNODE* LaTeX2MMLTree::AbsorbMSpaces( TNODE* MML_list ) {

  TNODE* rv =  MML_list;

// \ <uID9.1.2>      			width = "mediummathspace";
// \,<uID9.1.6>	thin      		width = "thinmathspace";
// \;<uID9.1.7>	thick     		width = "thickmathspace";
// \/<uID9.1.8>	italic correct	width = "veryverythinmathspace";
// \!<uID9.1.9>	negative thin   width = "negativethinmathspace";
//;3.205.1 <mspace width="thinmathspace "/>

  TNODE* rover  =  MML_list;
  while ( rover ) {           // loop thru list of MML nodes
    TCI_BOOL try_to_absorb  =  FALSE;
    if ( rover->details
    &&   rover->details->space_width != UNDEFINED_DETAIL ) {
      I16 em_teenths  =  rover->details->space_width;
// Arbitrary cutoff here - spaces wider than 5/18's em aren't absorbed.
      if ( -3 <= em_teenths && em_teenths <= 5 && em_teenths != 0 ) {
        U16 uobj,usub,id;
        GetUids( rover->zuID,uobj,usub,id );
        if ( uobj == 9 && usub == 20 && id == 0 ) {
        // maligngroup<uID9.20.0>
        } else {
          try_to_absorb  =  TRUE;
          if ( rover->attrib_list ) {
            U8* buffer;
            if ( LocateAttribVal(rover->attrib_list,
								(U8*)"linebreak",&buffer) )
              try_to_absorb  =  FALSE;
          }
        }
      }
    }
    if ( try_to_absorb ) {
	  TNODE* parent_op  =  NULL;
	  TCI_BOOL right_side;
	  if ( rover->prev ) {        // look for <mo> on the left
        U16 uobjtype,usubtype,uID;
        GetUids( rover->prev->zuID,uobjtype,usubtype,uID );
        if ( uobjtype==3 && usubtype==203 && uID==1 ) { // mo<uID3.203.1>
	      parent_op =  rover->prev;
	      right_side  =  TRUE;
		}
	  }
	  if ( !parent_op && rover->next ) {
        U16 uobjtype,usubtype,uID;
        GetUids( rover->next->zuID,uobjtype,usubtype,uID );
        if ( uobjtype==3 && usubtype==203 && uID==1 ) { // mo<uID3.203.1>
	      parent_op =  rover->next;
	      right_side  =  FALSE;
		}
	  }

      rover =  rover->next;
	  if ( parent_op )
		rv  =  AbsorbMMLSpaceNode( rv,parent_op,right_side );

	} else
      rover =  rover->next;
  }

  return rv;
}


TNODE* LaTeX2MMLTree::AbsorbMMLSpaceNode( TNODE* head,TNODE* op_node,
									  	    TCI_BOOL space_on_right ) {

  TNODE* rv =  head;

  TNODE* space_node   =  space_on_right ? op_node->next : op_node->prev;
  TNODE* left_anchor  =  space_on_right ? op_node : space_node->prev;
  TNODE* right_anchor =  space_on_right ? space_node->next : op_node;

  TNODE* parent   =  space_node->sublist_owner;
  double delta_width  =  space_node->details->space_width;

  if ( space_node == head )
    rv  =  op_node;
  if ( parent )
    parent->contents  =  op_node;

  space_node->next  =  NULL;
  space_node->prev  =  NULL;
  DisposeTNode( space_node );

  if ( left_anchor )
    left_anchor->next   =  right_anchor;
  if ( right_anchor )
    right_anchor->prev  =  left_anchor;

  double curr_op_space =  0.0;

  TCI_BOOL in_script  =  GetScriptStatus();

  U16 curr_spacing_mode =  in_script ? spacing_mode_in_scripts : mo_spacing_mode;

  TCI_BOOL use_default_spacing  =  FALSE;
  if ( curr_spacing_mode == 1 ) { // 1 - don't script
    use_default_spacing  =  TRUE;
  } else {
    U8* attr_nom    =  space_on_right ? (U8*)"rspace" : (U8*)"lspace";
    ATTRIB_REC* ar  =  LocateAttribRec( op_node,attr_nom );
    if ( ar ) {
// NEED TO EXTRACT EXISTING SPACE HERE! NOT YET IMPLEMENTED
   // curr_op_space =  GetSpaceInMUs(ar);
      use_default_spacing  =  TRUE;
    } else
      use_default_spacing  =  TRUE;
  }
  
  if ( use_default_spacing ) {
    if ( op_node->details && op_node->details->precedence != UNDEFINED_DETAIL ) {

// LaTeX operators get 3/18, 4/18, or 5/18 em's depending on precedence.
// The space tweek that we are absorbing is extra.

      U16 prec  =  op_node->details->precedence;
      if      ( prec < 20 )
        curr_op_space  =  5.0;
      else if ( prec < 30 )
        curr_op_space  =  4.0;
      else
        curr_op_space  =  3.0;
    } else
      curr_op_space  =  4.0;
  }

  double ems_val  =  (curr_op_space + delta_width) / 18.0;

  U8 zattr_val[80];
	sprintf( (char*)zattr_val,"%fem",ems_val );
  if ( zattr_val[0] ) {
    U16 zln =  strlen( (char*)zattr_val );
    if ( space_on_right )
      SetNodeAttrib( op_node,(U8*)"rspace",zattr_val );
    else
      SetNodeAttrib( op_node,(U8*)"lspace",zattr_val );
  }

  return rv;
}


// Many LaTeX constructs are parsed to uobjtype = 8 nodes.
// ( See NoteBook.gmr ) \func{}, \limfunc{}, \sin, etc.
// Some of these LaTeX objects should be treated as prefix
//  operators in MathML.	\lim for example?
// At the time of writing this code, I am NOT clear as to
//  which uobjtype 8's should be treated as functions
//  and which should be treated as prefix operators.
// "mod" is handled as a special case.

/* A note about "mod".
Lamport handles "mod" correctly, offering "\bmod" for "5 mod 3 = 2",
and "\pmod" for "2x+1\equiv 7(mod 13)".

We auto-recognize "mod" and script "\func{mod}".
If the Math Name dialog is used to define mod as an operator,
we sometimes script "\func{mod}", but generally script "\limfunc{mod}"

We need to fix our interface to support generation
of well-formed MathML regarding expressions containing mod.
*/

TNODE* LaTeX2MMLTree::Function2MML( TNODE* src_tex_func,
								    U16 usubtype,U16 uID,
									U16& tex_nodes_done,
									TNODE** out_of_flow_list ) {

// uID8.1.?		\func
// uID8.2.?   \sin, etc.
// uID8.3.?		\limfunc
// uID8.4.?		\lim, etc.

  TNODE* mml_rv =  NULL;

  if ( usubtype==1 || usubtype==3 ) {	// \func OR \limfunc
    U8 zuID[32];
    UidsTozuID( 8,usubtype,2,(U8*)zuID );
    TNODE* name_bucket  =  FindObject( src_tex_func->parts,
									    (U8*)zuID,INVALID_LIST_POS );
	if ( name_bucket ) {
      U8* func_name =  GetFuncName( name_bucket->contents );
	  if ( !strcmp((char*)func_name,"mod") ) {
        mml_rv  =  MakeTNode( 0L,0L,src_tex_func->src_linenum,
                                     (U8*)"3.203.1" );	// <mo>
        SetChData( mml_rv,func_name,NULL );

	    if ( src_tex_func->prev ) {
	      SetNodeAttrib( mml_rv,(U8*)"form",(U8*)zop_forms[OPF_infix] );
        //SetNodeAttrib( mml_rv,(U8*)"lspace",(U8*)".33333em" );
          SetMOSpacing( mml_rv,TRUE,(U8*)"0.27777em" );
          SetMOSpacing( mml_rv,FALSE,(U8*)"0.27777em" );
          SetDetailNum( mml_rv,DETAILS_form,2 );
          SetDetailNum( mml_rv,DETAILS_precedence,27 );
          SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_BIN );
	    } else {
	      SetNodeAttrib( mml_rv,(U8*)"form",(U8*)zop_forms[OPF_prefix] );
          SetDetailNum( mml_rv,DETAILS_form,1 );
          SetDetailNum( mml_rv,DETAILS_precedence,46 );
          SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_OP );
	    }
        if ( zMMLFunctionAttrs )
          SetMMLAttribs( mml_rv,(U8*)zMMLFunctionAttrs );
	    return mml_rv;
	  }
	}	// clause to handle "mod"
  }

  if ( usubtype==1			// \func{myfunc}
  ||   usubtype==2 ) {		// built-in, no limits

// \lim, \max, etc. are considered to be operators

    if ( usubtype==2 && FuncIsMMLOperator(2,uID) ) {
      mml_rv =  MakeTNode( 0L,0L,src_tex_func->src_linenum,
                                     (U8*)"3.203.1" );	// <mo>

	  SetNodeAttrib( mml_rv,(U8*)"form",(U8*)zop_forms[OPF_prefix] );
      SetDetailNum( mml_rv,DETAILS_form,1 );
      SetDetailNum( mml_rv,DETAILS_precedence,29 );
	} else {
      mml_rv =  MakeTNode( 0L,0L,src_tex_func->src_linenum,
                                     (U8*)"3.201.1" );	// <mi>
      SetDetailNum( mml_rv,DETAILS_function_status,usubtype );
	}

// Get the name of this TeX function

    if        ( usubtype==1 ) {		// user defined \func
      TNODE* name_bucket  =  FindObject( src_tex_func->parts,
									(U8*)"8.1.2",INVALID_LIST_POS );
      U8* func_name =  GetFuncName( name_bucket->contents );
      SetChData( mml_rv,func_name,NULL );

    } else if ( usubtype==2 ) {		// standard function like \sin
      U8* f_nom =  src_tex_func->src_tok + 1;
      SetChData( mml_rv,f_nom,NULL );
    }

    if ( zMMLFunctionAttrs )
      SetMMLAttribs( mml_rv,(U8*)zMMLFunctionAttrs );

// Some functions take limits - we call "LimFunc2MML"

  } else if ( usubtype==3			// \limfunc
  ||          usubtype==4 ) {		// built-in with limits

    U8* func_name =  NULL;
    if ( usubtype==3 ) {
// \limfunc<uID8.3.1>!\limfunc!REQPARAM(8.3.2,NONLATEX)_LIMPLACE__FIRSTLIM__SECONDLIM_
	  TNODE* name_bucket  =  FindObject( src_tex_func->parts,
                                (U8*)"8.3.2",INVALID_LIST_POS );
      func_name =  GetFuncName( name_bucket->contents );
	} else {
	  U16 zln   =  strlen( (char*)src_tex_func->src_tok );
	  func_name =  (U8*)TCI_NEW( char[zln] );
	  strcpy( (char*)func_name,(char*)src_tex_func->src_tok + 1 );
    }

    mml_rv  =  LimFunc2MML( src_tex_func,func_name,
                                usubtype,uID,out_of_flow_list );
    tex_nodes_done  =  1;
  }

  SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_OP );
  return mml_rv;
}



// Function to decide if a LaTeX TNODE represents
//  - (the start of) an MML identifier.
//  - (the start of) a number.
//  - (the start of) an operator.
// Note that this classification may be context sensitive
//  or may be dictated by info in MathML.gmr

U16 LaTeX2MMLTree::ClassifyTeXSymbol( TNODE* tex_sym_node,
                                        TNODE* LaTeX_list,
                                        TCI_BOOL& forces_geometry,
										TNODE* mml_node ) {

  U16 rv  =  0;
  forces_geometry =  FALSE;

  TCI_BOOL is_ellipsis    =  FALSE;
  TCI_BOOL check_mml_gmr  =  FALSE;
  I16 TeX_atom_ilk  =  TeX_ATOM_ORD;
//	TeX_ATOM_ORD	  1   TeX_ATOM_OP 		2   TeX_ATOM_BIN		3
//	TeX_ATOM_REL	  4   TeX_ATOM_OPEN   5   TeX_ATOM_CLOSE  6
//	TeX_ATOM_PUNCT  7   TeX_ATOM_INNER  8

  U16 objclass,subclass,id;
  GetUids( tex_sym_node->zuID,objclass,subclass,id );

  if ( objclass != 3 )	// We handle only class 3 objects here.
    return rv;

  switch ( subclass ) {
    case    1 :     // a<uID3.1.1> ... z<uID3.1.26>
	  if ( id==4 && IsDifferentiald(tex_sym_node,LaTeX_list) ) {
	    rv  =  MML_OPERATOR;
        if ( mml_node )
          SetDetailNum( mml_node,DETAILS_is_differential,1 );
	  } else
	    rv  =  MML_IDENTIFIER;
	break;
    case    2 :     // A<uID3.2.1> ... Z<uID3.2.26>
	  if ( id==4 && IsDifferentialD(tex_sym_node,LaTeX_list) ) {
	    rv  =  MML_OPERATOR;
        if ( mml_node ) {
          SetDetailNum( mml_node,DETAILS_is_differential,1 );
          SetDetailNum( tex_sym_node,DETAILS_is_differential,1 );
	    }
	  } else
	    rv  =  MML_IDENTIFIER; 
	break;

    case    3 :  	// 0<uID3.3.1> ... 9<uID3.3.10>
      rv  =  MML_NUMBER;
	break;

    case    4 :
    case    5 :
    case    6 :
    case    7 :
	  TCI_ASSERT(0);  // no LaTeX nodes have uID3.4.x, etc.
    break;

    case    8 :
//	;European Latin symbols
//	\NG<uID3.8.1>
//	\ng<uID3.8.2>
	  rv  =  MML_IDENTIFIER; 
    break;

    case    9 : // the following can't occur in MATH
//  \DH<uID3.9.1>
//  \dh<uID3.9.2>
//  \textexclamdown<uID3.9.3>
//  \aa<uID3.9.4>
//  \guillemotleft<uID3.9.5>
//  \guillemotright<uID3.9.6>
//  \TH<uID3.9.7>
//  \th<uID3.9.8>
//  \textquestiondown<uID3.9.9>
	  TCI_ASSERT(0);
    break;

    case   10 : 		// \alpha<uID3.10.1> ... \straightphi<uID3.10.47>
	  if ( id==34 ) {  	// \Delta<uID3.10.34>
        rv  =  ClassDeltaFromContext( tex_sym_node ); 
        if ( !rv )
          check_mml_gmr  =  TRUE;
	  } else
	    rv  =  MML_IDENTIFIER; 
	break;

    case   11 : 		// \not<uID3.11.1>
      if ( id==1 )
	    rv  =  MML_OPERATOR;
      else
        TCI_ASSERT(0);
	break;

    case   12 : 		// arrow symbols
      //check_mml_gmr  =  TRUE;
      TeX_atom_ilk  =  TeX_ATOM_REL;
	  rv  =  MML_OPERATOR;
	break;

    case  13  : {		// binops	\pm<uID3.13.1>, etc.
      //check_mml_gmr  =  TRUE;
	  rv  =  MML_OPERATOR;

// In certain contexts, binops are part of implied groupings
//  that translate to MML objects that aren't <mo>'s

	  if        ( id==4 ) {
	    if ( tex_sym_node->next )
          if ( IsImpliedEllipsis(tex_sym_node) )
            is_ellipsis  =  TRUE;

	  } else if ( id==39 || id==40 ) {
	  // \bigtriangleup<uID3.13.39>	t-norm
	  // \bigtriangledown<uID3.13.40>	t-conorm
        if ( tex_sym_node->next ) {
	      TNODE* nn =  tex_sym_node->next;
	      if ( IsTeXThinOrThickSpace(nn) && nn->next )
	        nn  =  nn->next;
          U16 uobj,usub,uID;
          GetUids( nn->zuID,uobj,usub,uID );
	      if ( uobj==3 ) {
	        if        ( usub==14 ) {
	          if ( uID==107		// <<uID3.14.107>
			  ||   uID==108		// =<uID3.14.108>
			  ||   uID==109 )	// ><uID3.14.109>
	            rv  =  MML_IDENTIFIER; 
		    } else if ( usub==17 ) {
			  if ( uID==90 )	// :<uID3.17.90>
	            rv  =  MML_IDENTIFIER; 
			}
		  }		// if ( uobj==3 )

	    } else
	      rv  =  MML_IDENTIFIER; 

	  }		// else if ( id==39 || id==40 )

	  if ( rv ==  MML_OPERATOR )
        TeX_atom_ilk  =  TeX_ATOM_BIN;
	}
	break;

    case  14  : {		// binrels
      //check_mml_gmr  =  TRUE;
      TeX_atom_ilk  =  TeX_ATOM_REL;
	    rv  =  MML_OPERATOR;
	  }
	  break;

    case  15  : {		// corners or delimiters
      //check_mml_gmr  =  TRUE;
      switch ( id ) {
        case 1  :  // \lfloor<uID3.15.1>
        case 3  :  // \lceil<uID3.15.3>
        case 5  :  // \langle<uID3.15.5>
        case 7  :  // \ulcorner<uID3.15.7>
        case 8  :  // \llcorner<uID3.15.8>
        case 11 :  // \leftdoublebracket<uID3.15.11>
        case 21 :  // \lmoustache<uID3.15.21>
        case 23 :  // \lgroup<uID3.15.23>
        case 30 :  // \guilsinglleft<uID3.15.30>
          TeX_atom_ilk  =  TeX_ATOM_OPEN;
        break;
        case 2  :  // \rfloor<uID3.15.2>
        case 4  :  // \rceil<uID3.15.4>
        case 6  :  // \rangle<uID3.15.6>
        case 9  :  // \urcorner<uID3.15.9>
        case 10 :  // \lrcorner<uID3.15.10>
        case 12 :  // \rightdoublebracket<uID3.15.12>
        case 20 :  // \rmoustache<uID3.15.20>
        case 22 :  // \rgroup<uID3.15.22>
        case 31 :  // \guilsinglright<uID3.15.31>
          TeX_atom_ilk  =  TeX_ATOM_CLOSE;
        break;

        case 24 :  // \arrowvert<uID3.15.24>
        case 25 :  // \Arrowvert<uID3.15.25>
        case 26 :  // \bracevert<uID3.15.26>
        break;

        default :
          TCI_ASSERT(0);        break;
      }
	    rv  =  MML_OPERATOR;
    }
	  break;

    case  16  : {		// Miscellaneous symbols
	    switch ( id ) {
        case  1 : // \AE<uID3.16.1>
        case  2 : // \OE<uID3.16.2>
        case  3 : // \ae<uID3.16.3>
        case  4 : // \oe<uID3.16.4>
        case  5 : // \L<uID3.16.5>
        case  6 : // \l<uID3.16.6>
        case  7 : // \O<uID3.16.7>
        case  8 : // \o<uID3.16.8>
        case  9 : // \i<uID3.16.9>
        case 10 : // \j<uID3.16.10>
        case 11 : // \ss<uID3.16.11>
        case 14 : // \AA<uID3.16.14>
	        rv  =  MML_IDENTIFIER; 
		    break;
        case 12 : // \questiondown<uID3.16.12>
        case 13 : // \exclamdown<uID3.16.13>
	        rv  =  MML_OPERATOR;
		    break;
		    default :
		      TCI_ASSERT(0);
		    break;
	    }

	  }
	  break;

    case  17  : {

      switch ( id ) {
        case  1  :		// \ldots<uID3.17.1>
        case  2  :		// \cdots<uID3.17.2>
        case  3  :		// \vdots<uID3.17.3>
        case  4  :		// \ddots<uID3.17.4>
	        rv  =  MML_IDENTIFIER;
		    break;

        //case  3  :		// \vdots<uID3.17.3>
        //case  4  :		// \ddots<uID3.17.4>
	      //  rv  =  MML_TEXT;
		    //break;

        case 14  :		// \partial<uID3.17.14>
		      rv  =  MML_OPERATOR;
          if ( mml_node )
            SetDetailNum( mml_node,DETAILS_is_differential,1 );
        break;

        case 29 :
          rv  =  MML_OPERATOR;
        break;

        case 30  :		// \angle<uID3.17.30>
        case 31  :    // \triangle<uID3.17.31>
        case 38  :    // \square<uID3.17.38>
        case 44  :    // \measuredangle<uID3.17.44>
        case 45  :    // \sphericalangle<uID3.17.45>
	        rv  =  MML_IDENTIFIER;
          math_field_ID =  MF_GEOMETRY;
          forces_geometry =  TRUE;
		    break;

        case 87  : 		// ,<uID3.17.87>
        case 92  : 		// ?<uID3.17.92>
	      if ( !tex_sym_node->next )
	          rv  =  MML_TEXT;
	      else
            check_mml_gmr =  TRUE;
	    break;

        case 88  : {		// .<uID3.17.88>
	      if ( tex_sym_node->next ) {
            rv  =  MML_OPERATOR;
            U16 uobj,usub,id;
            TNODE* next_right =  tex_sym_node->next;
            while ( next_right ) {
              GetUids( next_right->zuID,uobj,usub,id );
              if ( uobj==3 && usub==3 && id>=1 && id<=10 ) {  // digit
                rv  =  MML_NUMBER;
                break;
              } else if ( uobj==9 && usub==1 && id==6 ) {     // thinspace
                next_right  =  next_right->next;
              } else if ( uobj==9 && usub==5 && id==1 ) {     // allowbreak
                next_right  =  next_right->next;
              } else if ( uobj==9 && usub==3 ) {  // vertical spacing element
                next_right  =  next_right->next;
              } else
                break;
            }

            if ( rv != MML_NUMBER ) {
              if ( IsImpliedEllipsis(tex_sym_node) )
                is_ellipsis  =  TRUE;
              else if ( !next_right )
                rv  =  MML_TEXT;
            }

	      } else
            rv  =  MML_TEXT;
	    }
	    break;

        case 120 :		// \textvisiblespace<uID3.17.120>
          TCI_ASSERT(0);
		    break;

  // currency symbols

        case  63 :		// \pounds<uID3.17.63>
        case  64 :		// \cents<uID3.17.64>
        case  65 :		// \yen<uID3.17.65>
        case 108 :		// \$<uID3.17.108>
        case 140 :		// \textcurrency<uID3.17.140>
        case 141 :		// \texteuro<uID3.17.141>
        case 142 :		// \textlira<uID3.17.142>
        case 143 :		// \textfranc<uID3.17.143>
        case 144 :		// \textpeseta<uID3.17.144>
          check_mml_gmr =  TRUE;
        break;

        case 95  :
          rv  =  MML_OPERATOR;
          TeX_atom_ilk  =  TeX_ATOM_BIN;
        break;

        case 151 :   // FScr -- fourier
          rv = MML_IDENTIFIER;
        break;

        default :
          check_mml_gmr =  TRUE;
        break;
	    }
	  }
	  break;

    case  18  :		// negrels
      TeX_atom_ilk  =  TeX_ATOM_REL;
      rv  =  MML_OPERATOR;
	  break;

    default   :
	    TCI_ASSERT(0);
	  break;
  }

  if        ( is_ellipsis ) {
	  rv  =  MML_IDENTIFIER; 

  } else if ( check_mml_gmr ) {
    rv  =  MML_IDENTIFIER; 
    U8* dest_zname;
    U8* d_template;
    if ( d_mml_grammar->GetGrammarDataFromUID(tex_sym_node->zuID,
						context_math,&dest_zname,&d_template) ) {
      if ( d_template && *d_template ) {
	    U16 id  =  OPF_multiform;
		while ( id <= OPF_postfix ) {
	      if ( strstr((char*)d_template,zop_forms[id]) ) {
            rv  =  MML_OPERATOR;
			break;
		  }
		  id++;
		}
	  }
    } else
	  TCI_ASSERT(0);
  }

  if ( mml_node && TeX_atom_ilk )
    SetDetailNum( mml_node,DETAILS_TeX_atom_ilk,TeX_atom_ilk );

  return rv;
}



// We've hit a digit, or ".digit".
// A run of TeX symbols often coelesce into a single <mn>.

void LaTeX2MMLTree::GetNumberStr( TNODE* TeX_num_node,
									TNODE** out_of_flow_list,
  									TNODE* mn_node,
								  	U16& advance ) {

  advance =  0;

  U8 zvar_value[5000];		// storage for <mn>'s chdata
  zvar_value[0] =  0;

  U8 zvar_value2[5000];
  zvar_value2[0] =  0;

  U16 objclass,subclass,id;
  char zname[2];

  TCI_BOOL got_decimal =  FALSE;
  TCI_BOOL is_digit;
  TCI_BOOL is_decimal;
  TCI_BOOL is_comma;

  TNODE* t_rover  =  TeX_num_node;
  while ( t_rover ) {       // loop thru LaTeX nodes
	zname[0]    =  0;
    is_digit    =  FALSE;
    is_decimal  =  FALSE;
    is_comma    =  FALSE;

    GetUids( t_rover->zuID,objclass,subclass,id );
    if        ( objclass==3 && subclass==3 && id>=1 && id<=10 ) {
      is_digit  =  TRUE;

      U8 hex_digits[32];
      hex_digits[0] =  0;
	  U16 hln =  0;
	  if ( id==1 && t_rover->next ) {	// hit "0..."
        U16 uobj,usub,uID;
        GetUids( t_rover->next->zuID,uobj,usub,uID );
        if ( uobj==3 && usub==1 && uID==24 ) {	// 0x...
		  TNODE* rover  =  t_rover->next->next;
		  while ( rover ) {
		    U8 h_digit;
		    if ( h_digit = GetHexDigit(rover) ) {
              hex_digits[hln++] =  h_digit;
		      rover =  rover->next;
			} else
			  break;
		  }
		}		// 0x...
	  }

	  if ( hln ) {
	    hex_digits[hln] =  0;
        advance +=  hln + 2;
        strcat( (char*)zvar_value,"0x" );
        strcat( (char*)zvar_value,(char*)hex_digits );
        strcat( (char*)zvar_value2,"0x" );
        strcat( (char*)zvar_value2,(char*)hex_digits );
        is_digit  =  FALSE;
        break;
	  }

    } else if ( objclass==3 && subclass==17 && id==88 ) {
  // a period or decimal point

      if ( got_decimal )
        break;
      else {
	    if ( t_rover->next ) {
	// Look for ..
          U16 uobj,usub,uID;
          GetUids( t_rover->next->zuID,uobj,usub,uID );
          if ( uobj==3 && usub==17 && uID==88 )
            break;

		}
        is_decimal  =  TRUE;
        got_decimal =  TRUE;
      }

    } else if ( objclass==3 && subclass==17 && id==87 ) {
  // a comma, in or ending a number

      TCI_BOOL quit   =  TRUE;
      if ( InsideOfList(TeX_num_node) ) {

      } else if ( t_rover->next && !got_decimal ) {
        TNODE* right1 =  t_rover->next;
        GetUids( right1->zuID,objclass,subclass,id );
        if ( objclass==3 && subclass==3 && id>=1 && id<=10 ) {
          if ( right1->next ) {
            TNODE* right2 =  right1->next;
            GetUids( right2->zuID,objclass,subclass,id );
            if ( objclass==3 && subclass==3 && id>=1 && id<=10 ) {
              if ( right2->next ) {
                TNODE* right3 =  right2->next;
                GetUids( right3->zuID,objclass,subclass,id );
                if ( objclass==3 && subclass==3 && id>=1 && id<=10 ) {
                  quit      =  FALSE;
                  is_comma  =  TRUE;
                }
              }
            }
          }
        }
      }		// if ( t_rover->next )

      if ( quit ) break;

    } else if ( objclass==9 && subclass==1 && id==6 ) {
	// \,<uID9.1.6>

      if ( NumberContinues(t_rover,got_decimal) ) {
        AppendEntityToBuffer( (U8*)"9.1.6",zvar_value,zvar_value2 );
      } else
	    break;

    } else if ( objclass==9 && subclass==5 && id==1 ) {
	// \allowbreak<uID9.5.1>  

    // Do nothing - throw away this space object \allowbreak

    } else if ( objclass==9 && subclass==3 ) {	// vertical space

      if ( NumberContinues(t_rover,got_decimal) ) {
	    TCI_ASSERT( t_rover->prev );
	    t_rover =  t_rover->prev;
        *out_of_flow_list =  MoveNodeToList( *out_of_flow_list,t_rover->next );
        advance--;    
	  } else
	    break;

    } else
      break;

    advance++;    

    if      ( is_decimal )
      zname[0]  =  '.';
    else if ( is_comma )
      zname[0]  =  ',';
    else if ( is_digit )
      zname[0]  =  '0' + id - 1;

    if ( zname[0] ) {
      zname[1]  =  0;
      strcat( (char*)zvar_value,(char*)zname );
      strcat( (char*)zvar_value2,(char*)zname );
    }

    t_rover =  t_rover->next;

  }       // while ( t_rover )


  if ( zvar_value[0] )
    SetChData( mn_node,(U8*)zvar_value,(U8*)zvar_value2 );

  TCI_ASSERT( advance );
}



TCI_BOOL LaTeX2MMLTree::NumberContinues( TNODE* rover,
											TCI_BOOL got_decimal ) {

  TCI_BOOL rv =  FALSE;

  if ( rover->next ) {
    TNODE* right1 =  rover->next;
    U16 uobj,usub,uid;
    GetUids( right1->zuID,uobj,usub,uid );
    if ( uobj==9 && usub==5 && uid==1 ) {	// \allowbreak
      if ( right1->next )
        GetUids( right1->next->zuID,uobj,usub,uid );
    }
    if      ( uobj==3 && usub==3 && uid>=1 && uid<=10 )
      rv  =  TRUE;
    else if ( uobj==3 && usub==17 && uid==88 ) {
      if ( !got_decimal )
        rv  =  TRUE;
    }
  }

  return rv;
}


// After <mo> nodes are created in the initial translation,
//  we set their "form" attributes.  We need to know what
//  operands occur around multi-form ops like "+" or "-".

void LaTeX2MMLTree::CheckForOperands( TNODE* MML_op_node,
								        TCI_BOOL& has_left_operand,
								        TCI_BOOL& has_right_operand,
								        I16& left_space,
								        I16& right_space ) {

  U16 n_right_spaces;
  U16 n_left_spaces;
  U16 r_nodes_spanned;
  U16 l_nodes_spanned;
  U16 curr_precedence =  0;
  has_right_operand =  OperandExists( MML_op_node,TRUE,
  								        curr_precedence,n_right_spaces,
										r_nodes_spanned,right_space );
  has_left_operand  =  OperandExists( MML_op_node,FALSE,
  								        curr_precedence,n_left_spaces,
										l_nodes_spanned,left_space );
}



U16 LaTeX2MMLTree::GetCompoundOpForm( TNODE* tex_sym_node,
                                             U8 c1,U8 c2 ) {

  U16 rv  =  COPF_PREFIX;

// only 3 cases here, ++, -- and ..   (PREFIX or POSTFIX)
// Need to implement this - check nodes on left and right

  TNODE* nnn  =  NULL;
  if ( tex_sym_node && tex_sym_node->next && tex_sym_node->next->next )
    nnn  =  tex_sym_node->next->next;

  if ( nnn ) {
    if ( c1=='.' && c2=='.' ) {
      TCI_BOOL forces_geometry;
      U16 ilk =  ClassifyTeXSymbol( nnn,NULL,forces_geometry,NULL );
	    if ( ilk == MML_NUMBER )
        rv  =  COPF_INFIX;
	    else
        rv  =  COPF_POSTFIX;
	  } else
      rv  =  COPF_PREFIX;

  } else if ( tex_sym_node->prev ) {
    rv  =  COPF_POSTFIX;
  } else {
    rv  =  COPF_PREFIX;
	  TCI_ASSERT(0);
  }

  return rv;
}


// "tex_symbol_node" has been found to represent an LaTeX atom
//   that corresponds to a MathML identifier - <mi>  ie. \alpha
//   or start a run of LaTeX nodes that maps to an identier.

TNODE* LaTeX2MMLTree::GetIdentifier( TNODE* tex_symbol_node,
  									    TCI_BOOL forces_geometry,
  									    U16& tex_nodes_done ) {

  LOG_MSG_REC* msg_list =  NULL;

  U8 zvar_value[256];
  zvar_value[0]   =  0;

  U8 zvar_value2[256];
  zvar_value2[0]  =  0;

  U8* dest_zname;
  U8* d_template  =  NULL;

  U16 objclass,subclass,uid;
  GetUids( tex_symbol_node->zuID,objclass,subclass,uid );

  if ( subclass==1 || subclass==2 ) {   // a..z OR a..Z
    strcpy( (char*)zvar_value,(char*)tex_symbol_node->src_tok );
    strcpy( (char*)zvar_value2,(char*)tex_symbol_node->src_tok );
    tex_nodes_done++;

  } else if ( objclass==3 && subclass==13 && uid==4 ) {	// \cdot

    if ( IsImpliedEllipsis(tex_symbol_node) ) {
//    strcpy( (char*)zvar_value,"&ctdot;" );
// &ctdot;<uID3.17.2>,U022EF
      AppendEntityToBuffer( (U8*)"3.17.2",zvar_value,zvar_value2 );
      tex_nodes_done  +=  3;
    } else {
      tex_nodes_done++;
      TCI_ASSERT(0);
    }

  } else if ( objclass==3 && subclass==17 && uid==88 ) {
    if ( IsImpliedEllipsis(tex_symbol_node) ) {	// ...
//    strcpy( (char*)zvar_value,"&hellip;" );
// &hellip;<uID3.17.1>,U02026
      AppendEntityToBuffer( (U8*)"3.17.1",zvar_value,zvar_value2 );
      tex_nodes_done  +=  3;
    } else {
      tex_nodes_done++;
      TCI_ASSERT(0);
    }

  } else {	// Does this LaTeX symbol correspond to a MathML entity?

    U16 zln   =  strlen( (char*)zvar_value );
    U16 zln2  =  strlen( (char*)zvar_value2 );
    if ( d_mml_grammar->GetGrammarDataFromUID(tex_symbol_node->zuID,
                       context_math,&dest_zname,&d_template) ) {
      tex_nodes_done++;
      if ( dest_zname && *dest_zname )
        strcpy( (char*)zvar_value+zln,(char*)dest_zname );

      GetUnicodeEntity( d_template,zvar_value2+zln2 );

    } else {	//char* msg =  "No MathML entity for LaTeX symbol";
      tex_nodes_done++;
      msg_list  =  logfiler->AppendLogMsg( msg_list,
    								        MSG_ID4,SUFFIX_ID2,
         							        tex_symbol_node->src_tok,
         							        tex_symbol_node->src_linenum );
      TCI_ASSERT(0);
    }
  }


  TNODE* mml_rv =  MakeTNode( 0L,0L,tex_symbol_node->src_linenum,
                                (U8*)"3.201.1" );
  if ( !zvar_value[0] )
    strcpy( (char*)zvar_value,"??" );
  SetChData( mml_rv,zvar_value,zvar_value2 );

  if ( d_template ) {
    char* p_colon =  strchr( (char*)d_template,':' );
    if ( p_colon ) {
      ATTRIB_REC* ar  =  ExtractAttrs( (char*)p_colon+1 );
      if ( ar ) {
        MergeMOAttribs( mml_rv,ar );
        DisposeAttribs( ar );
      }
    }
  }

  if ( forces_geometry )    // \triangle, \square \etc.
    SetDetailNum( mml_rv,DETAILS_is_geometry,1 );
  if ( subclass == 10 )    // \triangle, \square \etc.
    SetDetailNum( mml_rv,DETAILS_is_Greek,1 );


  if ( msg_list )
    mml_rv->msg_list =  msg_list;

  return mml_rv;
}


// Translate a LaTeX schematum ( objtype 5 ) to MathML.
// The only caller is "TranslateMathList".

TNODE* LaTeX2MMLTree::MathStructureToMML( TNODE* obj_node,
									TNODE** out_of_flow_list,
 									MATH_CONTEXT_INFO* m_context,
                                    U16& tex_nodes_done,
                                    U16& error_code ) {

// These are all class 5 objects - structures with buckets

  TNODE* mml_rv =  NULL;
  tex_nodes_done  =  1;   // assumed, set if otherwise

  U16 objclass,subclass,id;
  GetUids( obj_node->zuID,objclass,subclass,id );

  if        ( subclass == 12 && id >= 15 ) {	// \mbox{TEXT}, etc.
    mml_rv  =  MBox2MML( obj_node,out_of_flow_list,id );
    tex_nodes_done  =  1;

// decorations
  } else if ( subclass == 12 ) {
    mml_rv  =  Decoration2MML( obj_node,out_of_flow_list,subclass,id );
    tex_nodes_done  =  1;

// decorated extensible arrows
  } else if ( subclass == 16 ) {
    mml_rv  =  XArrow2MML( obj_node,out_of_flow_list,id );
    tex_nodes_done  =  1;

// labels
  } else if ( subclass >= 10 && subclass <= 28 ) {
    mml_rv  =  OverOrUnder2MML( obj_node,subclass,id,out_of_flow_list );
    tex_nodes_done  =  1;

  } else {

// all other MATH schemata
    switch ( subclass ) {
  // fraction forms that don't include built-in delimiters
      case   1  :       // frac   - default line thickness (auto size)
      case   2  :       // tfrac  -       "        (small - textstyle)
      case   3  :       // dfrac  -       "     (large - displaystyle)
      case   4  :       // QABOVE - thick line (auto size)
      case   5  :       // QTABOVE-       "     ( small )
      case   6  :       // QDABOVE-       "     ( large )
      case   7  :       // QATOP  - no line (auto size)
      case   8  :       // QTATOP -     "   ( small )
      case   9  :       // QDATOP -     "   ( large )
  // AMS continued fractions - size remains normal at all levels of nesting
      case 113  :       // cfrac[DIMEN]{num}{denom}
      case 114  :       // lcfrac
      case 115  : {     // rcfrac
  //    mml_rv  =  Fraction2MML( obj_node,subclass,out_of_flow_list );
        mml_rv  =  GenFrac2MML( obj_node,subclass,out_of_flow_list );
        tex_nodes_done  =  1;
      }
      break;

      case 32 :         // \substack{...\\...}
      case 717: {				// \begin{subarray}...
  // multi-line script object
        mml_rv  =  ScriptStack2MML( obj_node,subclass,out_of_flow_list );
        tex_nodes_done  =  1;
      }
      break;

      case 35 : { 			// \begin{array}
	// We make some modifications to the source parse tree.
        HLinesToBucket( obj_node,FALSE );
        ColsToList( obj_node );
        mml_rv  =  Array2MML( obj_node,out_of_flow_list );
        SetMTableAttribs( mml_rv,FALSE,obj_node );
        tex_nodes_done  =  1;
      }
      break;

	  case  36  : 	    // \MATRIX - TCI internal format
	  case  37  : {	    // \TABLE - TCI internal format
	//  we make an equivalent tree for \begin{array} or \begin{tabular}
        TNODE* array  =  MATRIXtoExternalFormat( obj_node );

        U16 uobj,usub,uid;
        GetUids( array->zuID,uobj,usub,uid );

        if        ( usub == 140 ) {
          mml_rv  =  LaTeXCases2MML( array,out_of_flow_list );
        } else if ( usub >= 710 && usub <= 716 ) {
          HLinesToBucket( array,FALSE );
        // These LaTeX schemata don't have a {cols} arg
          mml_rv  =  Matrix2MML( array,out_of_flow_list,usub );
        } else {
          ColsToList( array );
          if ( subclass == 36 ) {
            HLinesToBucket( array,FALSE );
            mml_rv  =  Array2MML( array,out_of_flow_list );
            SetMTableAttribs( mml_rv,FALSE,array );
          } else {
            HLinesToBucket( array,TRUE );
            mml_rv  =  Tabular2MML( array,out_of_flow_list,usub );
          }
        }

		TNODE* parent =  obj_node->sublist_owner;
		TNODE* left_anchor  =  obj_node->prev;
		TNODE* right_anchor =  obj_node->next;

		obj_node->prev  =  NULL;
		obj_node->next  =  NULL;
        DisposeTList( obj_node );

		if ( parent )
		  parent->contents  =  array;
		if ( left_anchor ) {
		  left_anchor->next =  array;
		  array->prev =  left_anchor;
		}
		if ( right_anchor )	{
		  right_anchor->prev  =  array;
		  array->next =  right_anchor;
		}
        tex_nodes_done  =  1;
      }
      break;

// \EQN<uID5.39.0>!\EQN!_ENUMID__NCOLS__MLLABEL__MATHLETTERS__EQNROWS_
	  case  39  : {
// Here we have a nested EQN!
		if ( id==0 ) {
		  TNODE* parent =  obj_node->sublist_owner;
		  TNODE* left_anchor  =  obj_node->prev;
		  TNODE* right_anchor =  obj_node->next;

          TNODE* eqnarray =  EQNtoeqnarray( obj_node,subclass );
		  in_display  =  TRUE;
          mml_rv  =  TranslateTeXEqnArray( eqnarray,out_of_flow_list,
          												      subclass );
          mml_rv  =  AddEQNAttribs( mml_rv,subclass );
		  in_display  =  FALSE;

		  obj_node->prev  =  NULL;
		  obj_node->next  =  NULL;
          DisposeTList( obj_node );

		  if ( parent )       parent->contents    =  eqnarray;
		  if ( left_anchor )  left_anchor->next   =  eqnarray;
		  if ( right_anchor ) right_anchor->prev  =  eqnarray;

          tex_nodes_done  =  1;
		}
	  }
	  break;

      case TCMD_superscript :   // "^" - superscript
      case TCMD_subscript   :   // "_" - subscript
      case TCMD_Sp          :   // "\Sp" - superscript stack - internal?
      case TCMD_Sb          : { // "\Sb" - subscript stack - internal?
        mml_rv  =  Script2PsuedoMML( obj_node,m_context,out_of_flow_list );
        tex_nodes_done  =  1;
      }
      break;

      case 56 :
        if ( id == 1 ) {
          mml_rv  =  MarkingCmd2MML( obj_node );
          tex_nodes_done  =  1;
        } else
          TCI_ASSERT(0);
      break;

      case 60 :
      case 61 : {
        mml_rv  =  Radical2MML( obj_node,subclass,out_of_flow_list );
        tex_nodes_done  =  1;
      }
      break;

      case 70 : {		    // LaTeX fence object, \leftx.. \rightx delimited
        mml_rv  =  LaTeXFence2MML( obj_node,out_of_flow_list );
        tex_nodes_done  =  1;
      }
      break;

      case 71 :         // \big<uID5.71.1>!\big!VAR(5.71.2,FENCE,a,,)
      case 72 :         // \Big<uID5.72.1>!\Big!VAR(5.71.2,FENCE,a,,)
      case 73 :         // \bigg<uID5.73.1>!\bigg!VAR(5.71.2,FENCE,a,,)
      case 74 : {       // \Bigg<uID5.74.1>!\Bigg!VAR(5.71.2,FENCE,a,,)
        mml_rv  =  BiglmrToMML( obj_node,subclass,id );
        tex_nodes_done  =  1;
      }
      break;

// tagged MATH
      case 80  :        // \mathcal
      case 81  :        // \mathrm
      case 82  :        // \mathbf
      case 83  :        // \mathsf
      case 84  :        // 
      case 85  :        // 
      case 86  :        // 
      case 87  :        // 
      case 88  :        // \pmb
      case 89  :
      case 91  :
      case 96  :        // \boldsymbol{}
      case 459 : {      // \emph{}
        U8 zuID[16];
	    U16 uID =  ( subclass == 459 ) ? 2 : 1;
        UidsTozuID( 5,subclass,uID,zuID );
        TNODE* bucket =  FindObject( obj_node->parts,
                                (U8*)zuID,INVALID_LIST_POS );
	    if ( bucket && bucket->contents ) {
	      U8 switch_nom[64];
          U8* tag_nom =  QTRuSubIDtoName( subclass );
	      if ( tag_nom )
	        strcpy( (char*)switch_nom,(char*)tag_nom );
          mml_rv  =  TaggedMath2MML( obj_node,bucket->contents,
      							        subclass,switch_nom,
      							        out_of_flow_list );
	    } else
	      TCI_ASSERT(0);
        tex_nodes_done  =  1;
      }
      break;

// text runs in MATH
      case 90   :       // \text  <uID5.9 0.0>!\text!REQPARAM(5.90.1,TEXT)
      case 450  :       // \textup<uID5.450.1>!\textup!REQPARAM(5.450.2,TEXT)
      case 451  :       // \textit<uID5.451.1>
      case 452  :       // \textsl<uID5.452.1>
      case 453  :       // \textsc<uID5.453.1>
      case 454  :       // \textmd<uID5.454.1>
      case 455  :       // \textbf<uID5.455.1>
      case 456  :       // \textrm<uID5.456.1>
      case 457  :       // \textsf<uID5.457.1>
      case 458  :       // \texttt<uID5.458.1>
//    case 459  :       // \emph  <uID5.459.1>!\emph!REQPARAM(5.459.2,INHERIT)
      case 460  : {     // \textnormal<uID5.460.1>!\textnormal!REQPARAM(5.460.2,TEXT)
        U16 bucketID  =  (subclass==90) ? 1 : 2;
        U8 zuID[16];
        UidsTozuID( 5,subclass,bucketID,zuID );
        TNODE* bucket =  FindObject( obj_node->parts,
  							        (U8*)zuID,INVALID_LIST_POS );
	    if ( bucket && bucket->contents )
          mml_rv  =  TaggedText2MML( bucket->contents,
                                out_of_flow_list,
                                NULL,subclass );
	    else
	      TCI_ASSERT(0);
          tex_nodes_done  =  1;
	    }
	    break;

  // Fraction forms that include built-in delimiters - binomials

      case 101  :       // binom    - auto  - no line       ()
      case 102  :       // tbinom   - small - no line       ()
      case 103  :       // dbinom   - large - no line       ()
  // Our macros for binomials with user specified delimiters
      case 104  :       // QATOPD   - auto  - no line       {} etc.
      case 105  :       // QTATOPD  - small - no line       [] etc.
      case 106  :       // QDATOPD  - large - no line       || etc.
      case 107  :       // QOVERD   - auto  - normal line   <> etc.
      case 108  :       // QTOVERD  - small - normal line   \\ etc.
      case 109  :       // QDOVERD  - large - normal line   // etc.
      case 110  :       // QABOVED  - auto  - thick line    {} etc.
      case 111  :       // QTABOVED - small - thick line    {} etc.
      case 112  :       // QDABOVED - large - thick line    {} etc.
  // AMS delimited fractions - same args as our QABOVED's
  //  case 110  :       // \fracwithdelims<uID5.110.0>
  //  case 111  :       // \tfracwithdelims<uID5.111.0>
  //  case 112  :       // \dfracwithdelims<uID5.112.0>
  // AMS generalized fraction
      case 116  :       // genfrac{|}{|}{DIMEN}{size-behavior}{num}{denom}
        mml_rv  =  GenFrac2MML( obj_node,subclass,out_of_flow_list );
        tex_nodes_done  =  1;
      break;

	  case 135  :       // not used in NoteBook.gmr
	  case 136  :       //           "
	  case 137  :       //           "
	  case 138  :       //           "
	  case 139  :       //           "
	    TCI_ASSERT(0);
	  break;

      case 124            :   // align*
	    TCI_ASSERT(0);
	  break;

	  case TENV_cases     :
	    if ( id == 0 ) {
          mml_rv  =  LaTeXCases2MML( obj_node,out_of_flow_list );
          tex_nodes_done  =  1;
	    }
	  break;

	  case TENV_split     :
	  case TENV_gathered  :
      case TENV_aligned   :
      case TENV_alignedat :
	    if ( id == 0 ) {
          mml_rv  =  NestedTeXEqnArray2MML( obj_node,
                                out_of_flow_list,subclass );
          tex_nodes_done  =  1;
	    }
	  break;

      case 180  :       // \up<uID5.180.0>
      case 181  :       // \it<uID5.181.0>
      case 182  :       // \sl<uID5.182.0>
      case 183  :       // \sc<uID5.183.0>
      case 184  :       // \md<uID5.184.0>
      case 185  :       // \bf<uID5.185.0>
      case 186  :       // \rm<uID5.186.0>
      case 187  :       // \sf<uID5.187.0>
      case 188  :       // \tt<uID5.188.0>
      case 189  :       // \em<uID5.189.0>
	  break;

	  case 298  : 	// \U<uID5.298.0>!\U!REQPARAM(5.298.2,NONLATEX)
	  case 299  : {	// \UNICODE<uID5.299.0>!\UNICODE!REQPARAM(5.299.2,NONLATEX)
      //TCI_ASSERT(0);    // Should never get here - \U{}s converted to symbols earlier
        mml_rv  =  Unicode2MML( obj_node );
        tex_nodes_done  =  1;
	  }
	  break;

      case 350 :
      case 351 :
      case 352 :         // \pagebreak
      case 353 :
      case 354 : {
        //DoPagingCmd( obj_node );
        tex_nodes_done  =  1;
      }
      break;

// \QTR<uID5.400.0>!\QTR!REQPARAM(5.400.1,NONLATEX)REQPARAM(5.400.2,INHERIT)
      case 400 : {
        TNODE* ilk  =  FindObject( obj_node->parts,
  						              (U8*)"5.400.1",INVALID_LIST_POS );
        U8* run_name  =  ilk->contents->var_value;
        U16 usub_ID   =  GetQTRuSubID( run_name,TRUE );
        TNODE* bucket =  FindObject( obj_node->parts,
  						              (U8*)"5.400.2",INVALID_LIST_POS );
        if ( bucket && bucket->contents ) {
          // Note that QTR in MATH contains MATH!!!
	        TNODE* TeX_cont =  bucket->contents;
		      if ( usub_ID == TR_group ) {
		        U8 switch_nom[64];
            U16 switch_tag  =  GetMathRunTagFromTeXSwitch( TeX_cont, switch_nom );
			      if ( switch_tag )
			        mml_rv =  TaggedMath2MML( obj_node, TeX_cont, switch_tag, switch_nom, out_of_flow_list );
			      else {
			        TCI_BOOL do_bindings  =  FALSE;
              U16 tex_nodes_done,error_code;
			        TNODE* local_oof_list =  NULL;
              TNODE* mml_cont =  TranslateMathList( TeX_cont,
			  		                       do_bindings,NULL,tex_nodes_done,
			  		                       error_code,&local_oof_list );
	            if ( !mml_cont ) {		// <mtext>
                mml_cont  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
                SetChData( mml_cont,(U8*)"???",NULL );
	            }
              mml_cont  =  HandleOutOfFlowObjects( mml_cont,
           	  				      &local_oof_list,out_of_flow_list,3 );
              mml_rv =  mml_cont;
			      }

		      } else	// not a {\bb group}
            mml_rv  =  TaggedMath2MML( obj_node,TeX_cont,usub_ID,
           							    NULL,out_of_flow_list );

		} else	// empty \QTR
		  TCI_ASSERT(0);

        tex_nodes_done  =  1;
      }
      break;

// \QTO<uID5.401.0>!\QTO!REQPARAM(5.401.1,NONLATEX)REQPARAM(5.401.2,INHERIT)
      case 401 : {
	    TNODE* m_bucket =  FindObject( obj_node->parts,
					                (U8*)"5.401.2",INVALID_LIST_POS );
		if ( m_bucket && m_bucket->contents ) {
		  TNODE* TeX_cont =  m_bucket->contents;
          U16 tex_nodes_done,error_code;
  		  TCI_BOOL do_bindings  =  TRUE;
		  TNODE* local_oof_list =  NULL;
          mml_rv  =  TranslateMathList( TeX_cont,do_bindings,NULL,
									tex_nodes_done,error_code,
									&local_oof_list );
          mml_rv  =  HandleOutOfFlowObjects( mml_rv,&local_oof_list,
        							            out_of_flow_list,3 );
		}
        tex_nodes_done  =  1;
      }
      break;

      case 409  : 			// \FRAME in MATH
        RecordAnomaly( 1002,NULL,obj_node->src_offset1,
        						            obj_node->src_offset2 );
        tex_nodes_done  =  1;
      break;

      case 414  :			  // \nonumber
        tex_nodes_done  =  1;
      break;

      case 417  :			  // \typeout
      break;

      case 419  :			  // \FORMULA<uID5.419.1>
        mml_rv  =  FORMULA2MML( obj_node,out_of_flow_list );
        tex_nodes_done  =  1;
      break;


// \TeXButton<uID5.418.1>!\TeXButton!
//	REQPARAM(5.418.2,NONLATEX)REQPARAM(5.418.3,NONLATEX)
     case 418  :			// \TeXButton
        mml_rv = TEXBUTTON2MML(obj_node);
     break;
// \HTMLButton<uID5.428.1>!\HTMLButton!
//	REQPARAM(5.428.2,NONLATEX)REQPARAM(5.428.3,NONLATEX)
      case 428  :			// \HTMLButton
	    TCI_ASSERT(0);
      break;

      case 420  :			  // \footnotemark
      case 421  :			  // \footnote
      case 422  :			  // \footnotetext
      case 423  :			  // \endnote
      case 424  :			  // \marginpar
      case 426  : 			// \FOOTNOTE
        RecordAnomaly( 1001,NULL,obj_node->src_offset1,
        				            obj_node->src_offset2 );
        tex_nodes_done  =  1;
      break;

// \CustomNote<uID5.425.0>
// OPTPARAM(5.425.2,TEXT)
// REQPARAM(5.425.3,NONLATEX)
// REQPARAM(5.425.5,TEXT)

      case 425  : { 		// \CustomNote
        RecordAnomaly( 1001,NULL,obj_node->src_offset1,
        				            obj_node->src_offset2 );
        tex_nodes_done  =  1;

	    TNODE* t_bucket =  FindObject( obj_node->parts,
			                (U8*)"5.425.2",INVALID_LIST_POS );
		if ( t_bucket && t_bucket->contents ) {
          TNODE* local_oof_list =  NULL;
	      mml_rv =  TextInMath2MML( t_bucket->contents,
			    	                &local_oof_list,FALSE,FALSE );
	      if ( local_oof_list )
            DisposeTList( local_oof_list );
		}
      }
      break;


      case 461  :			  // \textcircled{}
      break;

      case 465  :			  // \symbol{94}
        mml_rv  =  CmdSymbol2MML( obj_node );
        tex_nodes_done  =  1;
      break;

      case 471  :			  // 
//\hyperref<uID5.550.0>!\hyperref!
// OPTPARAM(5.550.5,NONLATEX)
// REQPARAM(5.550.6,TEXT)_OLDARGS_
// _OLDARGS_IF(INHERIT,?{}?,_OLDHYPERREFARGS_)elseIF(?{?,_OLDHYPERREFARGS_)ifEND
// _OLDHYPERREFARGS_REQPARAM(5.471.2,TEXT)REQPARAM(5.471.3,TEXT)REQPARAM(5.471.4,NONLATEX)
        TCI_ASSERT(0);	// Should never get here
      break;


      case 476  :       // \msipassthru<uID5.476.0>!\msipassthru!REQPARAM(5.476.1,NONLATEX)
        mml_rv  =  PassThru2MML( obj_node, out_of_flow_list );
        break;
      case 480  :	      // \tiny<uID5.480.0>
      case 481  :	      // \scriptsize<uID5.481.0>
      case 482  :	      // \footnotesize<uID5.482.0>
      case 483  :	      // \small<uID5.483.0>
      case 484  :	      // \normalsize<uID5.484.0>
      case 485  :	      // \large<uID5.485.0>
      case 486  :	      // \Large<uID5.486.0>
      case 487  :	      // \LARGE<uID5.487.0>
      case 488  :	      // \huge<uID5.488.0>
      case 489  :	      // \Huge<uID5.489.0>
        tex_nodes_done  =  1;
      break;

      case 490  :       // \begin{tabular}
      case 491  : {			// \begin{tabular*}
        HLinesToBucket( obj_node,TRUE );
        ColsToList( obj_node );
        mml_rv  =  Tabular2MML( obj_node,out_of_flow_list,subclass );
        tex_nodes_done  =  1;
      } 
      break;

	  case 550  :		    // \hyperref<uID5.550.0>
	  case 551  :		    // \hyperlink<uID5.551.0>
	  case 552  :		    // \hypertarget<uID5.552.0>
	  case 553  :		    // \href<uID5.553.0>
	  case 554  :		    // \HREFTOLABEL<uID5.554.0>
	  case 555  :		    // \HYPERLINK<uID5.555.0>
	  case 556  :		    // \HYPERTARGET<uID5.556.0>
	  case 557  : {		  // \HREF<uID5.557.0>
        TCI_ASSERT(0);	// Should never get here	
        tex_nodes_done  =  1;
      } 
      break;

      case 600  :       // \displaystyle<uID5.600.1>
      case 601  :       // \textstyle<uID5.601.1>
      case 602  :       // \scriptstyle<uID5.602.1>
      case 603  :       // \scriptscriptstyle<uID5.603.1>
      break;

// \label<uID5.700.0>!\label!REQPARAM(5.700.1,NONLATEX)
// \tag<uID5.701.0>!\tag!REQPARAM(5.701.1,TEXT)
// \tag*<uID5.702.0>!\tag*!REQPARAM(5.702.1,TEXT)
// \notag<uID5.703.0>
// \ref<uID5.704.0>!\ref!REQPARAM(5.704.1,NONLATEX)
// \pageref<uID5.705.0>!\pageref!REQPARAM(5.705.1,NONLATEX)
// ;; Note that we map to \tag{} here!
// \TCItag<uID5.701.0>!\TCItag!REQPARAM(5.701.1,TEXT)
      case 472  :
//\QTSN<uID5.472.0>!\QTSN!REQPARAM(5.472.1,NONLATEX)REQPARAM(5.472.2,TEXT)
      case 700  : 			// \label
      case 704  : 			// \ref
      case 705  : {			// \pageref
        RecordAnomaly( 1001,NULL,obj_node->src_offset1,
      					            obj_node->src_offset2 );
        tex_nodes_done  =  1;
      } 
      break;

      case 701  : 		// \tag
      case 702  : 		// \tag*
      case 703  : {     // \notag
      RecordAnomaly( 1004,NULL,obj_node->src_offset1,
     						            obj_node->src_offset2 );
        tex_nodes_done  =  1;
      } 
      break;


      case 710  : 			// \matrix
      case 711  : 			// \smallmatrix
      case 712  : 			// \pmatrix
      case 713  : 			// \bmatrix
      case 714  : 			// \vmatrix
      case 715  : 			// \Vmatrix
      case 716  : {			// \Bmatrix
        HLinesToBucket( obj_node,FALSE );
    // These LaTeX schemata don't have a {cols} arg
        mml_rv  =  Matrix2MML( obj_node,out_of_flow_list,subclass );
        tex_nodes_done  =  1;
      } 
      break;

      case 800  :
	    mml_rv  =  Rule2MML( obj_node,out_of_flow_list );
      break;

      case 802  :
        tex_nodes_done  =  0;
        mml_rv  =  UnitInMath2MML( obj_node,out_of_flow_list,
                                    FALSE,tex_nodes_done );
      break;

      default :
        TCI_ASSERT(0);
      break;

    }		// switch ( subclass )

  }

  return mml_rv;
}               //  MathStructureToMML



// \fbox<uID5.12.13>!\fbox!REQPARAM(5.15.1,TEXT)    - loose box
// \frame<uID5.12.14>!\frame!REQPARAM(5.15.1,TEXT)	- tight box

TNODE* LaTeX2MMLTree::FBox2MML( TNODE* tex_fbox_node,
							  	              TNODE** out_of_flow_list,
								                U16 uID ) {

  DumpTList( tex_fbox_node, 0, TOKENIZER_TEX );

  TNODE* mml_rv =  NULL;

  TNODE* tbucket  =  FindObject( tex_fbox_node->parts, (U8*)"5.15.1", INVALID_LIST_POS );
  if ( tbucket ) {
    TNODE* local_oof_list =  NULL;
	  TNODE* contents =  TextInMath2MML( tbucket->contents, &local_oof_list,FALSE,FALSE );
    if ( !contents )
      contents  =  MakeSmallmspace();

	  if ( local_oof_list ) {
      contents  =  Struts2MML( contents,local_oof_list );
      local_oof_list  =  DisposeOutOfFlowList( local_oof_list,5,800 );
      contents  =  Labels2MML( contents,local_oof_list );
	    // We may want to pass this back to the caller in the future.
      DisposeTList( local_oof_list );
	  }
    contents  =  FixImpliedMRow( contents );

    TNODE* cell;
    // mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd)!/mtd!
    
    cell  =  CreateElemWithBucketAndContents( 5,57,1,0, contents);
    //contents -> sublist_owner = cell;

    mml_rv = cell;
//      cell  =  CreateElemWithBucketAndContents( 5,35,14,8,contents );
//  
//      // Put the cell under a cell list item node
//  
//  	  TNODE* list_node  =  MakeTNode( 0,0,0,(U8*)"5.35.11:0" );
//  	  list_node->parts  =  cell;
//  	  cell->sublist_owner =  list_node;
//  
//      TNODE* mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );
//  	  mtr->parts->parts =  list_node;
//  	  list_node->sublist_owner =  mtr->parts;

//      // Put the current row under a node in the row list
//  
//  	  TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)"5.35.9:0" );
//  	  row_list_node->parts  =  mtr;
//  	  mtr->sublist_owner  =  row_list_node;
//  
//      // mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
//      // _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)
//  
//      mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
//      SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_INNER );
//      //  SetNodeAttrib( rv,(U8*)"align",(U8*)"bottom" );
//      mml_rv->parts->parts  =  row_list_node;
//      row_list_node->sublist_owner =  mml_rv->parts;
//  
//      SetNodeAttrib( mml_rv,(U8*)"frame",(U8*)"solid" );
//  	  if ( uID==13 )
//          SetNodeAttrib( mml_rv,(U8*)"framespacing",(U8*)"0.8em 1.0ex" );
  }

  return mml_rv;
}


// in TEXT and MATH
// \mbox<uID5.12.15>!\mbox!REQPARAM(5.15.1,TEXT)
// A boxed (unbreakable) run of text.


TNODE* LaTeX2MMLTree::MBox2MML( TNODE* tex_box_node,
							  	              TNODE** out_of_flow_list,
								                U16 uID ) {

  TNODE* mml_rv =  NULL;

  TNODE* tbucket  =  FindObject( tex_box_node->parts,
				  				              (U8*)"5.15.1",INVALID_LIST_POS );
  TNODE* mbucket  =  FindObject( tex_box_node->parts,
				  				              (U8*)"5.15.2",INVALID_LIST_POS );
  TNODE* local_oof_list =  NULL;
	TNODE* contents =  NULL;
  if        ( tbucket && tbucket->contents ) {
    TCI_BOOL no_line_breaks =  TRUE;
	  contents =  TextInMath2MML( tbucket->contents,
                          &local_oof_list,no_line_breaks,FALSE );
    if ( !contents )
      contents  =  MakeSmallmspace();
	  if ( local_oof_list ) {
      contents  =  Struts2MML( contents,local_oof_list );
      local_oof_list  =  DisposeOutOfFlowList( local_oof_list,5,800 );
      contents  =  Labels2MML( contents,local_oof_list );
	// We may want to pass this back to the caller in the future.
      DisposeTList( local_oof_list );
	  }

  } else if ( mbucket && mbucket->contents ) {
    U16 tex_nodes_done,error_code;
  	TCI_BOOL do_bindings  =  TRUE;
    contents  =  TranslateMathList( mbucket->contents,do_bindings,
									              NULL,tex_nodes_done,error_code,
									              &local_oof_list );
    contents  =  HandleOutOfFlowObjects( contents,&local_oof_list,
          									            out_of_flow_list,3 );
  } else
    TCI_ASSERT(0);

	if ( contents->next ) // mrow<uID5.750.1>!mrow!BUCKET(5.750.2,
    mml_rv  =  CreateElemWithBucketAndContents( 5,750,1,2,contents );
	else
    mml_rv  =  contents;

  return mml_rv;
}


/*
TNODE* LaTeX2MMLTree::Fraction2MML( TNODE* tex_frac_node,
							  		    U16 subclass,TNODE** out_of_flow_list ) {

  TNODE* rv =  NULL;

// For our purposes, "display" is turned off in fractions.
  TCI_BOOL save_in_display_flag =  in_display;
  in_display  =  FALSE;

// mfrac<uID5.1.0>!mfrac!reqELEMENT(5.1.1)reqELEMENT(5.1.2)!/mfrac!
  TNODE* mml_frac =  MakeTNode( 0L,0L,0L,(U8*)"5.1.0" );

  U16 size_ilk  =  0;		// auto
  if      ( subclass==2 || subclass==5 || subclass==8 )
    size_ilk    =  1;		// small	- tfrac
  else if ( subclass==3 || subclass==6 || subclass==9 )
    size_ilk    =  2;		// large	- dfrac

  U16 line_ilk  =  0;		// normal
  if      ( subclass==4 || subclass==5 || subclass==6 )
    line_ilk    =  1;		// thick
  else if ( subclass==7 || subclass==8 || subclass==9 )
    line_ilk    =  2;		// none

  if ( line_ilk ) {
	  U8* attr_val  =  ( line_ilk==1 ) ? (U8*)"2" : (U8*)"0";
    SetNodeAttrib( mml_frac,(U8*)"linethickness",attr_val );
  }

  TCI_BOOL is_differential_op =  FALSE;
  TCI_BOOL is_unit  =  FALSE;

// Locate and translate the numerator
  TNODE* numerator  =  FindObject( tex_frac_node->parts,
									              (U8*)"5.1.1",INVALID_LIST_POS );
  TCI_ASSERT( numerator );
  TCI_BOOL do_bindings  =  TRUE;
  U16 tex_nodes_done,loc_error;
  TNODE* local_oof_list =  NULL;
  TNODE* mml_num_cont =  TranslateMathList( numerator->contents,
							                  do_bindings,NULL,tex_nodes_done,
							                  loc_error,&local_oof_list );
// Put the numerator into rv
// mfrac<uID5.1.0>!mfrac!reqELEMENT(5.1.1)reqELEMENT(5.1.2)!/mfrac!
  if ( !mml_num_cont )
    mml_num_cont  =  MakeSmallmspace();
  else if ( mml_num_cont->next )
    mml_num_cont  =  MMLlistToMRow( mml_num_cont );
  else {
    if ( mml_num_cont->details
    &&   mml_num_cont->details->is_differential != UNDEFINED_DETAIL
    &&   mml_num_cont->details->form == 1 )
	    is_differential_op  =  TRUE;
    if ( mml_num_cont->details
    &&   mml_num_cont->details->unit_state == 1 )
	    is_unit =  TRUE;
  }
  mml_num_cont  =  HandleOutOfFlowObjects( mml_num_cont,
				                  &local_oof_list,out_of_flow_list,3 );

// Put the numerator into mml_frac
  TNODE* part1  =  CreateBucketWithContents( 5,1,1,mml_num_cont );
  mml_frac->parts =  part1;
  part1->sublist_owner  =  mml_frac;

// Locate and translate the denominator
  TNODE* denominator  =  FindObject( tex_frac_node->parts,
  									            (U8*)"5.1.2",INVALID_LIST_POS );
  TNODE* mml_den_cont =  TranslateMathList( denominator->contents,
									              do_bindings,NULL,tex_nodes_done,
									              loc_error,&local_oof_list );
  if ( !mml_den_cont )
    mml_den_cont  =  MakeSmallmspace();
  else if ( mml_den_cont->next )
    mml_den_cont  =  MMLlistToMRow( mml_den_cont );
  mml_den_cont  =  HandleOutOfFlowObjects( mml_den_cont,
							              &local_oof_list,out_of_flow_list,3 );

// Put the denominator into rv
  TNODE* part2  =  CreateBucketWithContents( 5,1,2,mml_den_cont );
  part1->next   =  part2;
  part2->prev   =  part1;

  if ( size_ilk ) {	// it's not auto
// mstyle<uID5.600.0>!mstyle!BUCKET(5.600.2,MATH,,,/mstyle,)!/mstyle!
    TNODE* mstyle =  CreateElemWithBucketAndContents( 5,600,0,2,mml_frac );
    if        ( size_ilk==1 ) {		// tfrac - \textstyle
      SetNodeAttrib( mstyle,(U8*)"displaystyle",(U8*)"false" );
      SetNodeAttrib( mstyle,(U8*)"scriptlevel",(U8*)"1" );
    } else if ( size_ilk==2 ) {		// dfrac - \displaystyle
      SetNodeAttrib( mstyle,(U8*)"displaystyle",(U8*)"true" );
      SetNodeAttrib( mstyle,(U8*)"scriptlevel",(U8*)"0" );
	  }

    SetDetailNum( mstyle,DETAILS_style,1 );
    rv  =  CreateElemWithBucketAndContents( 5,750,1,2,mstyle );
  } else
    rv  =  mml_frac;

  if ( rv ) {
	  if ( is_differential_op ) {
      SetDetailNum( rv,DETAILS_is_differential,1 );
      SetDetailNum( rv,DETAILS_form,1 );
      SetDetailNum( rv,DETAILS_precedence,54 );
	  } else if ( is_unit ) {
      SetDetailNum( rv,DETAILS_unit_state,1 );
	  } else
      SetDetailNum( rv,DETAILS_is_expression,1 );
  }

  in_display  =  save_in_display_flag;
  return rv;
}
*/


/*
\begin{array}<uID5.35.0>!\begin{array}!_AXALIGN__ACOLS__AROWS_!\end{array}!
_AXALIGN_OPTPARAM(5.35.2,MEXTALIGN)
_ACOLS_REQPARAM(5.35.3,COLS)
_AROWS_LIST(5.35.9,_ALINE_,5.35.10,,\end{array},\\|\\*)
_ALINE__HRULES__ALINECELLS_
_ALINECELLS_LIST(5.35.11,_ACELL_,5.35.12,,\\|\\*|\end{array},&)
_ACELL_IF(TEXT,?\multicolumn?,5.412.0)elseIF(?\hdotsfor?,5.431.0)ELSE(_ARRBUCKET_)ifEND
_ARRBUCKET_BUCKET(5.35.8,MATH,,,&|\\|\\*|\end{array},)
;; 5.35.13 used in nb_xml.gmr - HRULES mapped to a bucket
*/

TNODE* LaTeX2MMLTree::Array2MML( TNODE* tex_array_node,
								TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  TNODE* row_list  =  FindObject( tex_array_node->parts,
   							    (U8*)"5.35.9",INVALID_LIST_POS );

  U16 nRows =  0;
  if ( row_list && row_list->parts ) {
// Count the parts, ie rows, under row_list
    TNODE* row_rover  =  row_list->parts;
    while ( row_rover ) {
      if ( row_rover->next )
        nRows++;
      else {        // this is the last row - it may be empty
        TNODE* q  =  FindObject( row_rover->parts,
   							    (U8*)"5.35.11",INVALID_LIST_POS );
        if ( q ) {
          TNODE* qq =  FindObject( q->parts,(U8*)"5.35.11",0 );
          if ( qq ) {
            if ( qq->next )
              nRows++;
            else {
              TNODE* qqq  =  FindObject( qq->parts,(U8*)"5.35.8",
                								INVALID_LIST_POS );
              if ( qqq && qqq->contents )
                nRows++;
            }
          }
        }		// if ( q )
      }
      row_rover =  row_rover->next;
    }

  } else {    // ERROR - no row list or no rows

  }


  MATH_CONTEXT_INFO context;
  context.row_number  =  0;

// Generate a list of <mtr>'s for the rows in the matrix

  TNODE* mtr_head   =  NULL;
  TNODE* mtr_tail;

  TNODE* row_rover  =  row_list->parts;
  U16 row_count =  0;
  while ( row_count < nRows ) {     // loop down thru rows
    context.row_number++;
    context.column_number  =  0;

    if ( row_rover && row_rover->parts ) {
      TNODE* mtd_head =  NULL;
      TNODE* mtd_tail;
// _ALINECELLS_LIST(5.35.11,_ACELL_,5.35.12,,\\|\end{array},&)
      TNODE* cell_list  =  FindObject( row_rover->parts,
   							        (U8*)"5.35.11",INVALID_LIST_POS );

      TNODE* TeX_EOL  =  FindObject( row_rover->parts,(U8*)"9.5.8",
                                        INVALID_LIST_POS );
      if ( !TeX_EOL )
        TeX_EOL =  FindObject( row_rover->parts,(U8*)"9.5.9",
                                        INVALID_LIST_POS );

	    if ( cell_list && cell_list->parts ) {
        TNODE* cell_rover =  cell_list->parts;    // head of list of "cells"

        U16 col_count =  0;
        while ( cell_rover ) {     // loop across columns in a row
          context.column_number++;

  // Translate current cell to MathML
       	  U16 mcol_align    =  0;
       	  U16 cols_spanned  =  0;
          TNODE* local_oof_list =  NULL;
          TNODE* contents =  NULL;
		  if ( cell_rover->parts ) {
            TNODE* bucket =  cell_rover->parts;
            U16 uobj,usub,uID;
            GetUids( bucket->zuID,uobj,usub,uID );
            if ( uobj==5 && usub==412 ) {
              contents  =  Multicolumn2MML( bucket,&local_oof_list,
 								     TRUE,mcol_align,cols_spanned );
            } else if ( uobj==5 && usub==431 ) {
              contents  =  Hdotsfor2MML( bucket,
 								     TRUE,cols_spanned );
	        } else {
              TCI_BOOL do_bindings  =  TRUE;
              U16 tex_nodes_done,error_code;
              contents =  TranslateMathList( bucket->contents,
										do_bindings,&context,
										tex_nodes_done,error_code,
										&local_oof_list );
            }
          }

          if ( !contents )
            contents  =  MakeSmallmspace();
          U16 vspace_context  =  2;   //We handle vspace in array cells
          contents  =  HandleOutOfFlowObjects( contents,
							          &local_oof_list,out_of_flow_list,
							          vspace_context );

          if ( TeX_EOL ) {
            contents  =  EOLSpace2MML( contents,TeX_EOL );
            TeX_EOL =  NULL;
          }

  // mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd)!/mtd!
          contents  =  FixImpliedMRow( contents );
          TNODE* cell  =  CreateElemWithBucketAndContents( 5,35,
											        14,8,contents );
          SetMTRAttribs( cell,mcol_align,cols_spanned );

  // Put the current cell under a cell list item node
          U8 zlistID[32];
          UidsTozuID( 5,35,11,(U8*)zlistID );
          U16 zln =  strlen( (char*)zlistID );
          zlistID[zln]  =  ':';

          // JCS non-standard: itoa( col_count,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1, "%d", col_count);
          
          TNODE* list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
          list_node->parts  =  cell;
          cell->sublist_owner =  list_node;

  // Add this list item to the list we're building
          if ( !mtd_head )
            mtd_head =  list_node;
          else {
            mtd_tail->next  =  list_node;
  	        list_node->prev =  mtd_tail;
          }
          mtd_tail  =  list_node;

  // Advance to next source cell
          cell_rover  =  cell_rover->next;
          col_count++;

          if ( cols_spanned > 1 )
            context.column_number +=  cols_spanned - 1;

        }		// loop across columns in a row

      }   // if ( cell_list && cell_list->parts )

  // Add current row to the list of rows we're building

/*
/mtr<uID5.35.16>
mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
_LISTOFCELLS_LIST(5.35.11,_MTCELL_,5.35.12,,/mtr|/mtable,)
_MTCELL_IF(MATH,?mtd?,5.35.14)ifEND
*/

      TNODE* mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );
	  mtr->parts->parts =  mtd_head;
	  mtd_head->sublist_owner =  mtr->parts;

  // Put the current row under a node in the row list

	  U8 zlistID[32];
      UidsTozuID( 5,35,9,(U8*)zlistID );
	  U16 zln =  strlen( (char*)zlistID );
	  zlistID[zln]  =  ':';
          // JCS non-standard: itoa( row_count,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1, "%d", row_count);
	  TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	  row_list_node->parts  =  mtr;
	  mtr->sublist_owner  =  row_list_node;

      if ( !mtr_head )
        mtr_head  =  row_list_node;
      else {
        mtr_tail->next =  row_list_node;
	    row_list_node->prev =  mtr_tail;
	  }
      mtr_tail  =  row_list_node;

  // Advance to the next row in the LaTeX source tree
      row_rover =  row_rover->next;
	}

    row_count++;

  }		// while ( row_count < nRows )


// mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
// _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)

  mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
  SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_INNER );
//  SetNodeAttrib( mml_rv,(U8*)"align",(U8*)"bottom" );
  mml_rv->parts->parts  =  mtr_head;
  if ( mtr_head )
    mtr_head->sublist_owner =  mml_rv->parts;

  return mml_rv;
}


/*
\frac + 24 variants

\genfrac<uID5.116.0>!\genfrac!
REQPARAM(5.1.4,FENCE)     - the left delimiter
REQPARAM(5.1.5,FENCE)     - the right delimiter
REQPARAM(5.1.3,DIMEN)     - the line thickness, 0 for binomials, empty for default line thickness
REQPARAM(5.1.6,NONLATEX)  - empty-auto, 0-displaystyle, 1-textstyle, 2-scriptstyle, 3-scriptscriptstyle
REQPARAM(5.1.1,MATH)      - the numerator
REQPARAM(5.1.2,MATH)      - the denominator
*/

TNODE* LaTeX2MMLTree::GenFrac2MML( TNODE* TeX_frac_node,
					        U16 subclass,TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  U16 size_ilk;
  switch ( subclass ) {
// auto size - context determines size
    case 1    :           // \frac   - auto size - normal line
    case 4    :           // \QABOVE - auto size - thick line
    case 7    :           // \QATOP  - auto size - no line
    case 101  :           // \binom  - auto size - no line - ()
    case 104  :           // \QATOPD - auto size - no line - fence args
    case 107  :           // \QOVERD - auto size - normal line - fence args
    case 110  :           // \QABOVED- auto size - thick line - fence args
      size_ilk  =  100; // auto
    break;

// always displaystyle - num and denom normal size
    case 3    :           // \dfrac   - large
    case 6    :           // \QDABOVE - large
    case 9    :           // \QDATOP  - large
    case 103  :           // \dbinom  - large
    case 106  :           // \QDATOPD - large
    case 109  :           // \QDOVERD - large - normal line   // etc.
    case 112  :           // \QDABOVED- large - thick line    {} etc.
// continued fractions are always displaystyle
    case 113  :           // \cfrac
    case 114  :           // \lcfrac
    case 115  :           // \rcfrac
      size_ilk  =  0;		// large
    break;

// always textstyle
    case 2    :           // \tfrac   - small
    case 5    :           // \QTABOVE - small
    case 8    :           // \QTATOP  - small
    case 102  :           // \tbinom  - small
    case 105  :           // \QTATOPD - small
    case 108  :           // \QTOVERD - small - normal line   \\ etc.
    case 111  :
      size_ilk  =  1;		// small
    break;

    case 116  : {         // \genfrac{(}{]}{dimen}{empty,0-3}{num}{denom}
      TNODE* sizing_attr_node =  FindObject( TeX_frac_node->parts,
									        (U8*)"5.1.6",INVALID_LIST_POS );
      if ( sizing_attr_node ) {
        if ( sizing_attr_node->contents && sizing_attr_node->contents->var_value )
          size_ilk  =  atoi( (char*)sizing_attr_node->contents->var_value );
        else
          size_ilk  =  100;		// auto
      } else
        TCI_ASSERT(0);
    }
    break;

    default :  TCI_ASSERT(0);  break;
  }

// manage script level in numerator and denominator

  TCI_BOOL save_in_display_flag =  in_display;
  U16 save_script_level   =  script_level;

  if        ( size_ilk == 0 ) {     // displaystyle
    script_level  =  0;
  } else if ( size_ilk == 1 ) {     // textstyle
    script_level  =  1;
  } else if ( size_ilk == 100 ) {   // auto
    if ( !in_display )
      script_level++;
  }
  in_display  =  FALSE;


// mfrac<uID5.1.0>!mfrac!reqELEMENT(5.1.1)reqELEMENT(5.1.2)!/mfrac!
  TNODE* mml_frac =  MakeTNode( 0L,0L,0L,(U8*)"5.1.0" );

  TCI_BOOL is_differential_op =  FALSE;
  TCI_BOOL is_unit  =  FALSE;

// Locate and translate the numerator
  TNODE* numerator  =  FindObject( TeX_frac_node->parts,
									(U8*)"5.1.1",INVALID_LIST_POS );
  TCI_ASSERT( numerator );
  TCI_BOOL do_bindings  =  TRUE;
  U16 tex_nodes_done,loc_error;
  TNODE* local_oof_list =  NULL;
  TNODE* mml_num_cont =  TranslateMathList( numerator->contents,
								    do_bindings,NULL,tex_nodes_done,
								    loc_error,&local_oof_list );

  if ( !mml_num_cont )          // numerator is empty
    mml_num_cont  =  MakeSmallmspace();
  else if ( mml_num_cont->next )// numerator has more than 1 element
    mml_num_cont  =  MMLlistToMRow( mml_num_cont );
  else {                        // numerator has exactly 1 element
    if ( mml_num_cont->details
    &&   mml_num_cont->details->is_differential != UNDEFINED_DETAIL
    &&   mml_num_cont->details->form == 1 )
	  is_differential_op  =  TRUE;
    if ( mml_num_cont->details
    &&   mml_num_cont->details->unit_state == 1 )
	  is_unit =  TRUE;
  }
  mml_num_cont  =  HandleOutOfFlowObjects( mml_num_cont,
	  	                  &local_oof_list,out_of_flow_list,3 );

// Put the numerator into mml_frac
  TNODE* part1  =  CreateBucketWithContents(5,1,1,mml_num_cont );
  mml_frac->parts =  part1;
  part1->sublist_owner  =  mml_frac;


// Locate and translate the denominator
  TNODE* denominator  =  FindObject( TeX_frac_node->parts,
                                (U8*)"5.1.2",INVALID_LIST_POS );
  TNODE* mml_den_cont =  TranslateMathList( denominator->contents,
									              do_bindings,NULL,tex_nodes_done,
									              loc_error,&local_oof_list );
  if ( !mml_den_cont )
    mml_den_cont  =  MakeSmallmspace();
  else if ( mml_den_cont->next )
    mml_den_cont  =  MMLlistToMRow( mml_den_cont );
  mml_den_cont  =  HandleOutOfFlowObjects( mml_den_cont,
							              &local_oof_list,out_of_flow_list,3 );

// Put the denominator into mml_frac
  TNODE* part2  =  CreateBucketWithContents(5,1,2,mml_den_cont );
  part1->next   =  part2;
  part2->prev   =  part1;


// Line thickness

  TCI_BOOL use_default_thickness  =  FALSE;
  U8 thickness_attr_val[80];

  switch ( subclass ) {
// default line
    case 1   :           // \frac   - default line
    case 2   :           // \tfrac
    case 3   :           // \dfrac
    case 107 :           // \QOVERD
    case 108 :           // \QTOVERD
    case 109 :           // \QDOVERD
      use_default_thickness =  TRUE;
    break;
// no line
    case 7   :           // \QATOP  - no line
    case 8   :           // \QTATOP
    case 9   :           // \QDATOP
    case 101 :           // \binom
    case 102 :           // \tbinom
    case 103 :           // \dbinom
    case 104 :           // \QATOPD
    case 105 :           // \QTATOPD
    case 106 :           // \QDATOPD
      strcpy( (char*)thickness_attr_val,"0" );
    break;
// thick line
    case 4   :           // \QABOVE - thick line
    case 5   :           // \QTABOVE
    case 6   :           // \QDABOVE
    case 110 :           // \QABOVED
    case 111 :           // \QTABOVED
    case 112 :           // \QDABOVED
//    strcpy( (char*)thickness_attr_val,"1pt" );
      strcpy( (char*)thickness_attr_val,"thick" );
    break;
// if line thickness is not specified, use default line
    case 113 :           // \cfrac<uID5.113.0>
    case 114 :           // \lcfrac<uID5.114.0>
    case 115 :           // \rcfrac<uID5.115.0>
    case 116 : {  // specified line
      TNODE* dimen  =  FindObject( TeX_frac_node->parts,
									(U8*)"5.1.3",INVALID_LIST_POS );
      if ( dimen ) {
        if ( dimen->contents ) {
          strcpy( (char*)thickness_attr_val,(char*)dimen->contents->var_value );
        } else    // here the thickness arg is require and was empty
          use_default_thickness =  TRUE;
      } else    // here the thickness arg is optional and was not specified
        use_default_thickness =  TRUE;
    }
    break;

    default  :  TCI_ASSERT(0);  break;
  }

  if ( !use_default_thickness )
    SetNodeAttrib( mml_frac,(U8*)"linethickness",thickness_attr_val );

// Set numerator alignment for aligned continued fractions

  if ( subclass == 114 )        // \lcfrac<uID5.114.0>
    SetNodeAttrib( mml_frac,(U8*)"numalign",(U8*)"left" );
  if ( subclass == 115 )        // \rcfrac<uID5.115.0>
    SetNodeAttrib( mml_frac,(U8*)"numalign",(U8*)"right" );


// Left side fence - NULL or (, or specified in the markup

  U8* l_uID =  ( subclass<=9 ) ? NULL : (U8*)"10.4.3";
  if ( subclass >= 113 && subclass <= 115 ) // \cfrac
    l_uID =  NULL;
  TNODE* l_fence  =  FindObject( TeX_frac_node->parts,
									(U8*)"5.1.4",INVALID_LIST_POS );
  if ( l_fence ) {
    if ( l_fence->contents )
      l_uID =  l_fence->contents->zuID;
    else
      l_uID =  NULL;
  }

// mo for left delimiter

  TNODE* left_mo;
  if ( l_uID ) {
    U8 mml_lfence_nom[64];
    U8 lunicode_nom[64];
    U8 mml_lfence_attrs[128];				// (<uID10.4.3>
    TeXDelimToMMLOp( l_uID,1,mml_lfence_nom,lunicode_nom,mml_lfence_attrs );

    left_mo =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );
    SetChData( left_mo,mml_lfence_nom,lunicode_nom );
// On screen, this delimiter is stretchy, in TeX it has a few discreet values.
    SetAttrsForFenceMO( left_mo,6,TRUE );
  } else
    left_mo  =  NULL;


// Right side fence - NULL or ), or specified in the markup

  U8* r_uID =  ( subclass<=9 ) ? NULL : (U8*)"10.4.4";
  if ( subclass >= 113 && subclass <= 115 ) // \cfrac
    r_uID =  NULL;
  TNODE* r_fence  =  FindObject( TeX_frac_node->parts,
									(U8*)"5.1.5",INVALID_LIST_POS );
  if ( r_fence ) {
    if ( r_fence->contents )
      r_uID =  r_fence->contents->zuID;
    else
      r_uID =  NULL;
  }

// mo for right delimiter

  TNODE* right_mo  =  NULL;
  if ( r_uID ) {
    U8 mml_rfence_nom[64];
    U8 runicode_nom[64];
    U8 mml_rfence_attrs[128];				// )<uID10.4.4>
    TeXDelimToMMLOp( r_uID,3,mml_rfence_nom,runicode_nom,mml_rfence_attrs );
    if ( mml_rfence_nom[0] != '.' ) {
      right_mo  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );
      SetChData( right_mo,mml_rfence_nom,runicode_nom );
// On screen, this delimiter is stretchy, in TeX it has a few discreet values.
      SetAttrsForFenceMO( right_mo,6,FALSE );
    }
  }

// join left + body + right

  TNODE* body =  mml_frac;
  if ( size_ilk != 100 ) {	// it's not auto
// mstyle<uID5.600.0>!mstyle!BUCKET(5.600.2,MATH,,,/mstyle,)!/mstyle!
    TNODE* mstyle =  CreateElemWithBucketAndContents( 5,600,0,2,mml_frac );
    if        ( size_ilk==0 ) {		// dbinom - \displaystyle
      SetNodeAttrib( mstyle,(U8*)"displaystyle",(U8*)"true" );
      SetNodeAttrib( mstyle,(U8*)"scriptlevel",(U8*)"0" );
    } else if ( size_ilk==1 ) {		// tbinom - \textstyle
      SetNodeAttrib( mstyle,(U8*)"displaystyle",(U8*)"false" );
      SetNodeAttrib( mstyle,(U8*)"scriptlevel",(U8*)"0" );
    } else if ( size_ilk==2 ) {		// 
      SetNodeAttrib( mstyle,(U8*)"displaystyle",(U8*)"false" );
      SetNodeAttrib( mstyle,(U8*)"scriptlevel",(U8*)"1" );
    } else if ( size_ilk==3 ) {		// 
      SetNodeAttrib( mstyle,(U8*)"displaystyle",(U8*)"false" );
      SetNodeAttrib( mstyle,(U8*)"scriptlevel",(U8*)"2" );
	  }

    SetDetailNum( mstyle,DETAILS_style,1 );
    body  =  CreateElemWithBucketAndContents( 5,750,1,2,mstyle );
  }

  if ( left_mo || right_mo ) {  // nest in an mrow

    TNODE* start;
    TNODE* tail;
    if ( left_mo ) {
	    start =  left_mo;
	    left_mo->next  =  body;
	    if ( body )
	      body->prev  =  left_mo;
    } else
	    start =  body;

    if ( start ) {
	    tail  =  start;
	    while ( tail->next )
	      tail  =  tail->next;
    } else {
	    start =  right_mo;
	    tail  =  right_mo;
    }

    if ( right_mo && right_mo != tail ) {
      tail->next  =  right_mo;
      right_mo->prev =  tail;
    }

    mml_rv  =  CreateElemWithBucketAndContents( 5,750,1,2,start );
  } else
    mml_rv  =  body;


  if ( mml_rv ) {
    SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_INNER );
	if ( is_differential_op ) {
      SetDetailNum( mml_rv,DETAILS_is_differential,1 );
      SetDetailNum( mml_rv,DETAILS_form,1 );
      SetDetailNum( mml_rv,DETAILS_precedence,54 );
	} else if ( is_unit ) {
      SetDetailNum( mml_rv,DETAILS_unit_state,1 );
	} else
      SetDetailNum( mml_rv,DETAILS_is_expression,1 );
  }

  in_display    =  save_in_display_flag;
  script_level  =  save_script_level;

  return mml_rv;
}


/*
LaTex
\hat<uID4.1.1>!\hat!REQPARAM(4.1.50,MATH)
..
..
\ddddot<uID4.1.13>!\ddddot!REQPARAM(4.1.50,MATH)

Map to high precedence postfix operators in MML

/mover<uID5.54.1>		  base			  accent
mover<uID5.54.2>!mover!reqELEMENT(5.54.3)reqELEMENT(5.54.5)
!/mover!

/munder<uID5.53.1>
munder<uID5.53.2>!munder!reqELEMENT(5.53.3)_UNDERSCRIPT_!/munder!
_UNDERSCRIPT_reqELEMENT(5.53.4)
*/

TNODE* LaTeX2MMLTree::MathAccToMML( TNODE* tex_acc_node,
                                    U16 tex_acc_uID,
                                    TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  U8 zuID[16];
  UidsTozuID( 4,1,50,(U8*)zuID );
  TNODE* tex_base_bucket =  FindObject( tex_acc_node->parts,
								(U8*)zuID,INVALID_LIST_POS );

// Translate the contents of tex_base_bucket

  TNODE* mml_bc =  NULL;
  if ( tex_base_bucket && tex_base_bucket->contents ) {
    TCI_BOOL do_bindings  =  TRUE;
    U16 tex_nodes_done,error_code;
    TNODE* local_oof_list =  NULL;
    mml_bc  =  TranslateMathList( tex_base_bucket->contents,
                           		do_bindings,NULL,tex_nodes_done,
                           		error_code,&local_oof_list );
    mml_bc  =  HandleOutOfFlowObjects( mml_bc,
				        &local_oof_list,out_of_flow_list,3 );
  } else {
  	TCI_ASSERT(0);
    mml_bc  =  MakeSmallmspace();
  }

// mover<uID5.54.2>!mover!reqELEMENT(5.54.3)_OVERSCRIPT_
  mml_rv  =  CreateElemWithBucketAndContents( 5,54,2,3,mml_bc );

// this accent may embellish an operator!!

  if ( mml_bc
  &&   mml_bc->details
  &&   mml_bc->details->form != UNDEFINED_DETAIL ) {
    I16 val =  GetDetailNum( mml_bc,DETAILS_form );
    SetDetailNum( mml_rv,DETAILS_form,val );
    val =  GetDetailNum( mml_bc,DETAILS_precedence );
	if ( val != UNDEFINED_DETAIL )
      SetDetailNum( mml_rv,DETAILS_precedence,val );
  } else {
    SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );
  }


  TNODE* mml_acc_op =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );// <mo>

  U8* op_nom;
  U8* op_info;
  if ( d_mml_grammar->GetGrammarDataFromUID(tex_acc_node->zuID,
                                context_math,&op_nom,&op_info) ) {
    U8 entity_buffer[128];
    entity_buffer[0]  =  0;
    if ( output_entities_as_unicodes )
      if ( op_info && *op_info )
        GetUnicodeEntity( op_info,entity_buffer );

    if ( op_nom && *op_nom )
      SetChData( mml_acc_op,op_nom,entity_buffer );

	OP_GRAMMAR_INFO op_record;
	if ( op_info && *op_info ) {
      GetAttribsFromGammarInfo( op_info,op_record );
      U16 form_ID =  op_record.form;
	  if ( form_ID >= OPF_prefix && form_ID <= OPF_postfix ) {
	    SetNodeAttrib( mml_acc_op,(U8*)"form",(U8*)zop_forms[form_ID] );
	    ATTRIB_REC* tail  =  mml_acc_op->attrib_list;
	    while ( tail->next )
	      tail  =  tail->next;
	    tail->next  =  op_record.attr_list;
	  } else
	    mml_acc_op->attrib_list =  op_record.attr_list;
	  op_record.attr_list =  NULL;
	} else
	  TCI_ASSERT(0);

  } else
    TCI_ASSERT(0);

  TNODE* part2  =  CreateBucketWithContents( 5,54,5,mml_acc_op );

  TNODE* part1  =  mml_rv->parts;
  part1->next   =  part2;
  part2->prev   =  part1;

  SetNodeAttrib( mml_rv,(U8*)"accent",(U8*)"true" );
  return mml_rv;
}


/*
_LIMPLACE_IF(BIGOPPLACES,?\nolimits?,15.2.2)elseIF(?\limits?,15.2.1)ifEND
_FIRSTLIM_IF(MATH,?_?,5.50.1)elseIF(?\Sb?,5.30.0)elseIF(?^?,5.51.1)elseIF(?\Sp?,5.31.0)ifEND
_SECONDLIM_IF(MATH,?^?,5.51.1)elseIF(?\Sp?,5.31.0)elseIF(?_?,5.50.1)elseIF(?\Sb?,5.30.0)ifEND

;Big operators - auto size
\int<uID7.1.1,a>!\int!_LIMPLACE__FIRSTLIM__SECONDLIM_
;Big operators - large size
\dint<uID7.1.1,l>!\dint!_LIMPLACE__LOWERLIM__UPPERLIM_
;Big operators - small size
\tint<uID7.1.1,s>!\tint!_LIMPLACE__LOWERLIM__UPPERLIM_
*/

TNODE* LaTeX2MMLTree::BigOp2MML( TNODE* tex_bigop_node,
						   			U16& tex_nodes_done,
						   			TNODE** out_of_flow_list ) {

  TNODE* rv =  NULL;

  tex_nodes_done  =  1;		// All LaTeX bigops are parsed
  U16 n_integrals =  0;		//  to 1 node with various parts

  U16 uobjtype,usubtype,tex_uID;
  GetUids( tex_bigop_node->zuID,uobjtype,usubtype,tex_uID );
  switch ( tex_uID ) {
    case  1 :	 //\int<uID7.1.1,a>
	  n_integrals =  1;	  break;
    case  2 :	 //\iint<uID7.1.2,a>
	  n_integrals =  2;	  break;
    case  3 :	 //\iiint<uID7.1.3,a>
	  n_integrals =  3;	  break;
    case  4 :	 //\iiiint<uID7.1.4,a>
	  n_integrals =  4;	  break;
    case  5 :	 //\idotsint<uID7.1.5,a>
      n_integrals =  5;	  break;
    case  6 :	 //\oint<uID7.1.6,a>
	  n_integrals =  1;	  break;
	default :
	break;
  }

// Determine the bigop's size attribute.  \int<uID7.1.1,a>
//  'a' == auto, 'l' == large, 's' == small

  U16 uID_ln  =  strlen( (char*)tex_bigop_node->zuID );
  U8 tex_size_attr  =  tex_bigop_node->zuID[ uID_ln-1 ];
  if ( tex_size_attr != 'a'
  &&   tex_size_attr != 'l'
  &&   tex_size_attr != 's' ){
    TCI_ASSERT( 0 );
  }

// Determine the TeX limit placement attribute of this bigop

  TNODE* op_parts =  tex_bigop_node->parts;
  TCI_BOOL tex_limits     =  FALSE;
  TCI_BOOL tex_nolimits   =  FALSE;
  TCI_BOOL tex_autolimits =  FALSE;
  if      ( FindObject(op_parts,(U8*)"15.2.1",INVALID_LIST_POS) )
    tex_limits     =  TRUE;
  else if ( FindObject(op_parts,(U8*)"15.2.2",INVALID_LIST_POS) )
    tex_nolimits   =  TRUE;
  else
    tex_autolimits =  TRUE;   // the default

// If tex_autolimits, limits are under/over in a display, - <munderover>
//  and at the right for inline math, except for integrals.
// If tex_limits, limit are always under/over			  - <munderover>
// If tex_nolimits, limit are always at the right         - <msubsup>

  TCI_BOOL use_subsup;
  if      ( tex_limits )
    use_subsup  =  FALSE;
  else if ( tex_nolimits )
    use_subsup  =  TRUE;
  else if ( tex_autolimits ) {
    use_subsup  =  n_integrals ? TRUE : FALSE;
  }

// Locate the TeX lower limit.

  TNODE* mml_ll   =  NULL;
  TNODE* tex_sub  =  FindObject( op_parts,(U8*)"5.50.1",
										    INVALID_LIST_POS );
  if ( tex_sub ) {	// single line lower limit
    if ( tex_sub->parts && tex_sub->parts->contents ) {
      TNODE* tex_ll_cont  =  tex_sub->parts->contents;
      TCI_BOOL do_bindings  =  TRUE;
      U16 tex_nodes_done,loc_error;
      TNODE* local_oof_list =  NULL;
      mml_ll =  TranslateMathList( tex_ll_cont,do_bindings,NULL,
						tex_nodes_done,loc_error,&local_oof_list );
      if ( !mml_ll )
        mml_ll  =  MakeSmallmspace();
      else if ( mml_ll->next )
        mml_ll  =  MMLlistToMRow( mml_ll );
      mml_ll  =  HandleOutOfFlowObjects( mml_ll,
					    &local_oof_list,out_of_flow_list,3 );
	}

  } else {  // We may have a multiline lower limit
    TNODE* tex_sb =  FindObject( op_parts,(U8*)"5.30.0",
											INVALID_LIST_POS );
    if ( tex_sb )
      mml_ll  =  SbAndSp2MML( tex_sb,out_of_flow_list );
  }

// Locate the TeX upper limit.

  TNODE* mml_ul   =  NULL;
  TNODE* tex_sup  =  FindObject( op_parts,(U8*)"5.51.1",
											INVALID_LIST_POS );
  if ( tex_sup ) {	// single line upper limit
    if ( tex_sup->parts && tex_sup->parts->contents ) {
      TNODE* tex_ul_cont  =  tex_sup->parts->contents;
      TCI_BOOL do_bindings  =  TRUE;
      U16 tex_nodes_done,loc_error;
      TNODE* local_oof_list =  NULL;
      mml_ul =  TranslateMathList( tex_ul_cont,do_bindings,NULL,
						    tex_nodes_done,loc_error,&local_oof_list );
      if ( !mml_ul )
        mml_ul  =  MakeSmallmspace();
      else if ( mml_ul->next )
        mml_ul  =  MMLlistToMRow( mml_ul );
      mml_ul  =  HandleOutOfFlowObjects( mml_ul,
     						&local_oof_list,out_of_flow_list,3 );
	}

  } else {	// We may have a multiline upper limit
    TNODE* tex_sp =  FindObject( op_parts,(U8*)"5.31.0",
											INVALID_LIST_POS );
    if ( tex_sp )
      mml_ul  =  SbAndSp2MML( tex_sp,out_of_flow_list );
  }

// Return <mo>,<munder>,<over>,<munderover>,<msub>,<msup>,or <msubsup>
// The first component, the base, in all cases is an <mo>.

  U16 precedence;
  TNODE* base_op  =  NULL;

/* I'm not using special code to script translation of \idotsint
  if ( IsIDotsInt(tex_bigop_node) ) {
    base_op =  IDotsInt2MML();
    precedence  =  31;

// We look up the mml operator entity that matches our TeX bigop
  } else {
*/
    base_op  =  MakeTNode( 0L,0L,tex_bigop_node->src_linenum,
                                  		(U8*)"3.203.1" );	// <mo>
    precedence  =  SetMMLBigOpEntity( tex_bigop_node,base_op );
    SetNodeAttrib( base_op,(U8*)"form",(U8*)zop_forms[OPF_prefix] );

	if ( mml_ll || mml_ul ) {
	  if ( !use_subsup ) {
	    U8* attr_nom  =  (U8*)"movablelimits";
	    //U8* attr_nom  =  (U8*)"moveablelimits";
	    U8* attr_val  =  NULL;
        if      ( tex_autolimits ) {
          if ( renderer_implements_displays ) {
   		    attr_val  =  (U8*)"true";
          } else {
            if ( in_display )
   		      attr_val  =  (U8*)"false";
            else
   		      attr_val  =  (U8*)"true";
          }

        } else if ( tex_limits )
   		    attr_val  =  (U8*)"false";
	    if ( attr_val )
          SetNodeAttrib( base_op,attr_nom,attr_val );
	  }
	}

    base_op =  SetBigOpSize( base_op,tex_size_attr );

    SetDetailNum( base_op,DETAILS_form,1 );
    SetDetailNum( base_op,DETAILS_precedence,precedence );
/*
  }
*/

/*
msub<uID5.50.2>!msub!_SUBBASE_reqELEMENT(5.50.3)_SUBSCRIPT_reqELEMENT(5.50.4)
msup<uID5.51.2>!msup!reqELEMENT(5.51.3)_SUPERSCRIPT_reqELEMENT(5.51.5)
msubsup<uID5.52.2>!msubsup!reqELEMENT(5.52.3)_SSSUB_reqELEMENT(5.52.4)_SSSUPER_reqELEMENT(5.52.5)

munder<uID5.53.2>!munder!reqELEMENT(5.53.3)_UNDERSCRIPT_reqELEMENT(5.53.4)

mover<uID5.54.2>!mover!reqELEMENT(5.54.3)_OVERSCRIPT_reqELEMENT(5.54.5)
munderover<uID5.55.2>!munderover!reqELEMENT(5.55.3)_UOSUB__UOSUPER_!/munderover!
_UOSUB_reqELEMENT(5.55.4)
_UOSUPER_reqELEMENT(5.55.5)
*/

  TNODE* mml_big_op =  NULL;

  if ( mml_ll || mml_ul ) {	// we have limit(s)

    U16 mml_selem_type;
    U16 mml_ll_uID  =  0;
    U16 mml_ul_uID  =  0;
    if ( use_subsup ) {		// under and/or over
      if        ( mml_ll && mml_ul ) {
        mml_selem_type  =  52;	// <msubsup>
        mml_ll_uID  =  4;
        mml_ul_uID  =  5;
      } else if ( mml_ll ) {
        mml_selem_type  =  50;	// <msub>
        mml_ll_uID  =  4;
	  } else {
        mml_selem_type  =  51;	// <msup>
        mml_ul_uID  =  5;
	  }
	} else {					// sub and/or super
      if        ( mml_ll && mml_ul ) {
        mml_selem_type  =  55;	// <munderover>
        mml_ll_uID  =  4;
        mml_ul_uID  =  5;
      } else if ( mml_ll ) {
        mml_selem_type  =  53;	// <munder>
        mml_ll_uID  =  4;
	  } else {
        mml_selem_type  =  54;	// <mover>
        mml_ul_uID  =  5;
	  }
	}

// construct mml_big_op - structured element

    mml_big_op  =  CreateElemWithBucketAndContents(
			  					5,mml_selem_type,2,3,base_op );
	TNODE* parts_tail =  mml_big_op->parts;

  // construct lower limit bucket
	if ( mml_ll ) {
      TNODE* next_part  =  CreateBucketWithContents(
							5,mml_selem_type,mml_ll_uID,mml_ll );
      parts_tail->next  =  next_part;
      next_part->prev   =  parts_tail;
	  parts_tail  =  next_part;
	}

  // construct upper limit bucket
	if ( mml_ul ) {
      TNODE* next_part  =  CreateBucketWithContents(
							5,mml_selem_type,mml_ul_uID,mml_ul );
      parts_tail->next  =  next_part;
      next_part->prev   =  parts_tail;
	}

  } else		// there are no limits
	mml_big_op  =  base_op;


  rv  =  mml_big_op;


  I16 form  =  1;
  if ( rv ) {		// a TeX BigOp, \int, \sum
    SetDetailNum( rv,DETAILS_form,form );
    SetDetailNum( rv,DETAILS_precedence,precedence );
    SetDetailNum( rv,DETAILS_bigop_status,1 );
    SetDetailNum( rv,DETAILS_TeX_atom_ilk,TeX_ATOM_OP );
	if ( n_integrals )
      SetDetailNum( rv,DETAILS_integral_num,n_integrals );
  }

  return rv;
}


// _<uID5.50.0>!_!REQPARAM(5.50.1,MATH)
// ^<uID5.51.0>!^!REQPARAM(5.51.1,MATH)
// $x\Sb{\CELL{}\CELL{}}$ - internal
// $x_{\substack{ 12 \\ 34}}$ - external

TNODE* LaTeX2MMLTree::Script2PsuedoMML( TNODE* tex_script_node,
									    MATH_CONTEXT_INFO* m_context,
										TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;
  script_level++;

  U8* op_nom  =  NULL;
  U16 objclass,subclass,id;
  GetUids( tex_script_node->zuID,objclass,subclass,id );
  if ( objclass==5 ) {
    if      ( subclass==TCMD_superscript )	// superscript
      op_nom =  (U8*)"TeX^";
    else if ( subclass==TCMD_subscript )	// subscript
      op_nom =  (U8*)"TeX_";
    else if ( subclass==TCMD_Sb )			// "\Sb" - subscript stack
      op_nom =  (U8*)"TeXSb";
    else if ( subclass==TCMD_Sp )			// "\Sp" - superscript stack
      op_nom =  (U8*)"TeXSp";
  }

  if ( op_nom ) {

    if ( subclass==TCMD_Sb || subclass==TCMD_Sp ) {
// multi-line script
      mml_rv  =  MakeTNode( 0L,0L,tex_script_node->src_linenum,
        							      (U8*)"3.201.1" );	// mi<uID3.201.1>
      SetChData( mml_rv,op_nom,NULL );
      TNODE* mtable =  SbAndSp2MML( tex_script_node,out_of_flow_list );
	  if ( mtable ) {
	    mml_rv->next  =  mtable;
	  	mtable->prev  =  mml_rv;
	  }

	} else {		// _ or ^
// single line script
      TNODE* TeX_contents =  tex_script_node->parts->contents;
      if ( TeX_contents ) {
        TNODE* cont =  NULL;

        TCI_BOOL is_degrees =  FALSE;
        if ( subclass==TCMD_superscript
        &&   ScriptIsCirc(TeX_contents) ) {
          is_degrees =  TRUE;

	    } else if ( BaseIsDiffD(tex_script_node) ) {
		  cont  =  TranslateDDsubscript( TeX_contents,out_of_flow_list );

		} else {

          if ( subclass==TCMD_subscript ) {

// It's not clear when a numeric subscript, a_{11},
//  really represents a row,column versus the number eleven

            TCI_BOOL insert_commas  =  TRUE;
            if ( tex_script_node->prev ) {
              U16 uobjtype,usubtype,uID;
              GetUids( tex_script_node->prev->zuID,uobjtype,usubtype,uID );
              if ( uobjtype == 8 )  // a function
                insert_commas  =  FALSE;
            }
            if ( insert_commas )
              cont  =  SubscriptBody2MML( TeX_contents,m_context );
		  }

		  if ( !cont ) {
            TCI_BOOL do_bindings  =  TRUE;
            U16 tex_nodes_done,error_code;
            TNODE* local_oof_list =  NULL;
            cont  =  TranslateMathList( TeX_contents,do_bindings,
   		  		      NULL,tex_nodes_done,error_code,&local_oof_list );
            if ( !cont )
              cont  =  MakeSmallmspace();
            cont  =  HandleOutOfFlowObjects( cont,
          			        &local_oof_list,out_of_flow_list,3 );
		  }
        }

        if ( is_degrees ) {
          mml_rv  =  MakeTNode( 0L,0L,0L,(U8*)"3.201.1" );  // <mi>

		  U8 num_entity[16];
          CreateUnicodeEntity( num_entity,0,0xb0 );

          SetChData( mml_rv,(U8*)"&deg;",num_entity );
          SetNodeAttrib( mml_rv,(U8*)"class",(U8*)"msi_unit" );
          if ( zMMLUnitAttrs )
            SetMMLAttribs( mml_rv,(U8*)zMMLUnitAttrs );

          SetDetailNum( mml_rv,DETAILS_unit_state,1 );
          SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );

		} else {
          if ( !cont )
            cont  =  MakeSmallmspace();

          mml_rv  =  MakeTNode( 0L,0L,tex_script_node->src_linenum,
                                (U8*)"3.201.1" );	// mi<uID3.201.1>
          SetChData( mml_rv,op_nom,NULL );

          TNODE* mrow =   cont->next ? MMLlistToMRow(cont) : cont;
          if ( mrow ) {
            mml_rv->next  =  mrow;
            mrow->prev    =  mml_rv;
          }
        }


	  } else
	    TCI_ASSERT(0);		// empty script?
	}

  } else        // node is not a script?
    TCI_ASSERT(0);

  script_level--;
  return mml_rv;
}


// \sqrt<uID5.60.0>!\sqrt!OPTPARAM(5.60.1,MATH)REQPARAM(5.60.2,MATH)
// \root<uID5.61.0>!\root!REQPARAM(5.61.1,MATH)!\of!REQPARAM(5.61.2,MATH)

// msqrt<uID5.60.0>!msqrt!BUCKET(5.60.2,MATH,,,/msqrt)!/msqrt!
// mroot<uID5.61.0>!mroot!reqELEMENT(5.61.2)reqELEMENT(5.61.1)!/mroot!

TNODE* LaTeX2MMLTree::Radical2MML( TNODE* tex_radical_node,
								            U16 subclass,TNODE** out_of_flow_list ) {

  TNODE* rv =  NULL;
  U8 zuID[16];

// Locate the bucket that contains the power - if it exists
  UidsTozuID( 5,subclass,1,(U8*)zuID );
  TNODE* tex_power_bucket =  FindObject( tex_radical_node->parts,
  									            (U8*)zuID,INVALID_LIST_POS );
// Locate the bucket that contains the base
  UidsTozuID( 5,subclass,2,(U8*)zuID );
  TNODE* tex_base_bucket  =  FindObject( tex_radical_node->parts,
  									            (U8*)zuID,INVALID_LIST_POS );

  TNODE* mml_power_bucket =  NULL;
  if ( tex_power_bucket ) {
    UidsTozuID( 5,61,1,(U8*)zuID );
    mml_power_bucket  =  MakeTNode( 0L,0L,0L,(U8*)zuID );
    UidsTozuID( 5,61,2,(U8*)zuID );
  } else
    UidsTozuID( 5,60,2,(U8*)zuID );
  TNODE* mml_base_bucket  =  MakeTNode( 0L,0L,0L,(U8*)zuID );

// Translate the contents of the tex_base_bucket
  TCI_BOOL do_bindings  =  TRUE;
  U16 tex_nodes_done,error_code;
  TNODE* local_oof_list =  NULL;

  TNODE* bc =  NULL;
  if ( tex_base_bucket && tex_base_bucket->contents ) {
    bc  =  TranslateMathList( tex_base_bucket->contents,do_bindings,
				    NULL,tex_nodes_done,error_code,&local_oof_list );
	  if ( !bc )
      bc  =  MakeSmallmspace();
	  else if ( tex_power_bucket ) {
      if ( bc->next )
	      bc  =  MMLlistToMRow( bc );
	  } else {
// We don't need to wrap a multi-node contents list in an <mrow>
//   if we're generating <msqrt> - it contains an implied <mrow>
      bc  =  FixImpliedMRow( bc );
	  }

    bc  =  HandleOutOfFlowObjects( bc,&local_oof_list,
                                  out_of_flow_list,3 );

  	mml_base_bucket->contents =  bc;
	  bc->sublist_owner =  mml_base_bucket;
  }

// Translate the contents of the tex_power_bucket

  TNODE* pc =  NULL;
  if ( tex_power_bucket ) {
    script_level++;
    pc  =  TranslateMathList( tex_power_bucket->contents,do_bindings,
					NULL,tex_nodes_done,error_code,&local_oof_list );
    script_level--;
	  if ( !pc )
      pc  =  MakeSmallmspace();
	  else if ( pc->next )
	    pc  =  MMLlistToMRow( pc );
    pc  =  HandleOutOfFlowObjects( pc,&local_oof_list,
                                    out_of_flow_list,3 );

	  mml_power_bucket->contents  =  pc;
	  pc->sublist_owner =  mml_power_bucket;

    UidsTozuID( 5,61,0,(U8*)zuID );		// mroot
  } else
    UidsTozuID( 5,60,0,(U8*)zuID );		// msqrt

  rv  =  MakeTNode( 0L,0L,0L,(U8*)zuID );		// mroot or msqrt
  rv->parts =  mml_base_bucket;
  mml_base_bucket->sublist_owner  =  rv;

  if ( mml_power_bucket ) {
    mml_base_bucket->next   =  mml_power_bucket;
	  mml_power_bucket->prev  =  mml_base_bucket;
  }

  SetDetailNum( rv,DETAILS_is_expression,1 );
  SetDetailNum( rv,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );

  return rv;
}

/*
\end{tabular}<uID5.490.99>
\begin{tabular}<uID5.490.0>!\begin{tabular}!_TXALIGN__TCOLS__TROWS_!\end{tabular}!
;;_TSWIDTH_REQPARAM(5.491.1,DIMEN)		no width
_TXALIGN_OPTPARAM(5.490.2,MEXTALIGN)
_TCOLS_REQPARAM(5.490.3,COLS)
_TROWS_LIST(5.490.9,_TLINE_,5.490.10,,\end{tabular},\\)
_TLINE__HRULES__TLINECELLS_
_TLINECELLS_LIST(5.490.11,_TCELL_,5.490.12,,\\|\end{tabular},&)
_HRULES_LIST(5.490.44,_HLINE_,5.490.45,\hline|\cline,,hline)
_HLINE_IF(TEXT,?\hline?,9.9.0)elseIF(?\cline?,5.90.0)ifEND
_TCELL_IF(TEXT,?\multicolumn?,5.412.0)ELSE(_TABBUCKET_)ifEND
_TABBUCKET_BUCKET(5.490.8,TEXT,,,&|\\|\end{tabular},)
;; 5.490.13 used in nb_xml.gmr - HRULES mapped to a bucket

\end{tabular*}<uID5.491.99>
\begin{tabular*}<uID5.491.0>!\begin{tabular*}!_TSWIDTH__TXALIGN__TCOLS__TSROWS_!\end{tabular*}!
_TSWIDTH_REQPARAM(5.490.1,DIMEN)
;;_TXALIGN_OPTPARAM(5.490.2,MEXTALIGN)
*/

TNODE* LaTeX2MMLTree::Tabular2MML( TNODE* tex_tabular_node,
									TNODE** out_of_flow_list,
									U16 usubtype ) {

  TNODE* mml_rv =  NULL;

  TCI_BOOL save_display_flag  =  in_display;
  in_display  =  FALSE;

  U8 zuID[16];

//_TXALIGN_OPTPARAM(5.490.2,MEXTALIGN)
  UidsTozuID( 5,usubtype,2,(U8*)zuID );
  TNODE* ext_align  =  FindObject( tex_tabular_node->parts,
									zuID,INVALID_LIST_POS );

  U16 nRows,nCols;
  TNODE* TeX_row_list;
  TNODE* TeX_cell_list;
  TNODE* TeX_EOL  =  NULL;

//_TROWS_LIST(5.490.9,_TLINE_,5.490.10,,\end{tabular},\\)

  UidsTozuID( 5,usubtype,9,(U8*)zuID );
  TeX_row_list  =  FindObject( tex_tabular_node->parts,
   								    zuID,INVALID_LIST_POS );

  TNODE* TeX_row_rover;
  TNODE* TeX_cell_rover;
  if ( TeX_row_list ) {
    TeX_row_rover =  TeX_row_list->parts;     // head of list of "rows"

// Locate the column list in the first row
// _TLINE__HRULES__TLINECELLS_
// _TLINECELLS_LIST(5.490.11,_TCELL_,5.490.12,,\\|\end{tabular},&)
    UidsTozuID( 5,usubtype,11,(U8*)zuID );
    TeX_cell_list =  FindObject( TeX_row_rover->parts,
   								    zuID,INVALID_LIST_POS );
    TeX_cell_rover  =  TeX_cell_list->parts;    // head of list of "cells"

// Count the parts, ie rows, in TeX_row_rover

    nRows =  0;
    TNODE* row_rover  =  TeX_row_rover;
    while ( row_rover ) {

      if ( row_rover->next ) {
        nRows++;
      } else {        // this is the last row - it may be empty
        UidsTozuID( 5,usubtype,11,(U8*)zuID );
        TNODE* q  =  FindObject( row_rover->parts,
   									zuID,INVALID_LIST_POS );
        if ( q ) {
          TNODE* qq =  FindObject( q->parts,
   									zuID,INVALID_LIST_POS );
          if ( qq ) {
            if ( qq->next )
              nRows++;
            else {
              UidsTozuID( 5,usubtype,8,(U8*)zuID );
              TNODE* qqq  =  FindObject( qq->parts,zuID,
               						   INVALID_LIST_POS );
              if ( qqq && qqq->contents )
                nRows++;
            }
          }
        }		// if ( q )

      }
      row_rover =  row_rover->next;
    }		// loop thru rows

// Count the parts, ie columns, in the first row

    nCols =  0;
    TNODE* col_rover =  TeX_cell_rover;
    while ( col_rover ) {
      nCols++;
      col_rover =  col_rover->next;
    }

  } else    // ERROR - no cell list and no row list
    nRows =  nCols  =  0;

// Generate the list of <mtr>'s from the lines in the tabular

  TNODE* row_first  =  NULL;
  TNODE* row_last;

  U16 row_count =  0;
  while ( row_count < nRows ) {     // loop down thru rows

    TNODE* col_first  =  NULL;
    TNODE* col_last;

    U16 col_count =  0;
//  while ( col_count < nCols ) {   // loop across columns in a row
    while ( TeX_cell_rover ) {      // loop across columns in a row
// Translate current cell to MathML

	  TNODE* local_oof_list =  NULL;
	  TNODE* contents =  NULL;

      U16 mcol_align    =  0;
      U16 cols_spanned  =  0;
	  if ( TeX_cell_rover->parts ) {
        TNODE* bucket =  TeX_cell_rover->parts;
        U16 uobj,usub,uID;
        GetUids( bucket->zuID,uobj,usub,uID );
        if ( uobj==5 && usub==412 ) {
          contents  =  Multicolumn2MML( bucket,&local_oof_list,
 								    FALSE,mcol_align,cols_spanned );
	    } else
	      contents  =  TextInMath2MML( bucket->contents,
	   							    &local_oof_list,FALSE,FALSE );
      }
/*
      if ( local_oof_list ) {
        contents  =  Struts2MML( contents,local_oof_list );
        local_oof_list  =  DisposeOutOfFlowList( local_oof_list,5,800 );
        contents  =  Labels2MML( contents,local_oof_list );
	// We may want to pass this back to the caller in the future.
        DisposeTList( local_oof_list );
	  }
*/
      if ( !contents )
        contents  =  MakeSmallmspace();
      U16 vspace_context  =  2;   //We handle vspace in table cells
      contents  =  HandleOutOfFlowObjects( contents,
      							&local_oof_list,out_of_flow_list,
      							vspace_context );

      if ( TeX_EOL ) {
        contents  =  EOLSpace2MML( contents,TeX_EOL );
        TeX_EOL =  NULL;
      }

// mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd)!/mtd!
      contents  =  FixImpliedMRow( contents );
      TNODE* mtd  =  CreateElemWithBucketAndContents( 5,35,14,8,contents );

      SetMTRAttribs( mtd,mcol_align,cols_spanned );


// Put the current cell under a cell list item node
	  U8 zlistID[32];
      UidsTozuID( 5,35,11,(U8*)zlistID );
	  U16 zln =  strlen( (char*)zlistID );
	  zlistID[zln]  =  ':';
          // JCS non-standard itoa( col_count,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1, "%d", col_count);

	  TNODE* mtd_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	  mtd_list_node->parts  =  mtd;
	  mtd->sublist_owner  =  mtd_list_node;

// Add this list item to the list we're building
      if ( !col_first )
        col_first =  mtd_list_node;
      else {
        col_last->next  =  mtd_list_node;
	    mtd_list_node->prev =  col_last;
	  }
      col_last  =  mtd_list_node;

// Advance to next source cell
      TeX_cell_rover  =  TeX_cell_rover->next;
      col_count++;

    }		// loop across columns in a row

// Add current row to the list of rows we're building

// /mtr<uID5.35.16>
// mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
// _LISTOFCELLS_LIST(5.35.11,_MTCELL_,5.35.12,,/mtr|/mtable,)
// _MTCELL_IF(MATH,?mtd?,5.35.14)ifEND
    TNODE* mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );
	mtr->parts->parts =  col_first;
	col_first->sublist_owner =  mtr->parts;

// Put the current row under a node in the row list
	U8 zlistID[32];
    UidsTozuID( 5,35,9,(U8*)zlistID );
	U16 zln =  strlen( (char*)zlistID );
	zlistID[zln]  =  ':';
        // JCS non-standard itoa( row_count,(char*)zlistID+zln+1,10 );
        sprintf((char*)zlistID+zln+1, "%d", row_count);
	TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	row_list_node->parts  =  mtr;
	mtr->sublist_owner    =  row_list_node;

    if ( !row_first )
      row_first   =  row_list_node;
    else {
      row_last->next  =  row_list_node;
	  row_list_node->prev =  row_last;
	}
    row_last  =  row_list_node;

// Advance to the next row in the LaTeX source tree
    if ( TeX_row_rover ) {
      TeX_row_rover  =  TeX_row_rover->next;
      if ( TeX_row_rover ) {
        UidsTozuID( 5,usubtype,11,(U8*)zuID );
        TeX_cell_list =  FindObject( TeX_row_rover->parts,zuID,
                                        INVALID_LIST_POS );
        if ( TeX_cell_list ) {
          TeX_EOL =  FindObject( TeX_row_rover->parts,(U8*)"9.5.8",
                                        INVALID_LIST_POS );
          if ( !TeX_EOL )
            TeX_EOL =  FindObject( TeX_row_rover->parts,(U8*)"9.5.9",
                                        INVALID_LIST_POS );
          TeX_cell_rover  =  TeX_cell_list->parts;
        } else
          TeX_cell_rover  =  NULL;
      }
    }
    row_count++;
  }			// loop thru rows in LaTeX source

// Create an mtable for mml_rv
// mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
// _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)
  mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
  SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_INNER );
//  SetNodeAttrib( mml_rv,(U8*)"align",(U8*)"bottom" );
  mml_rv->parts->parts  =  row_first;
  row_first->sublist_owner  =  mml_rv->parts;

  SetMTableAttribs( mml_rv,TRUE,tex_tabular_node );

  in_display  =  save_display_flag;

  return mml_rv;
}


//	Source LaTeX Syntax
// \begin{align}<uID5.123.0>!\begin{align}!_ALIGNLINES_!\end{align}!
// _ALIGNLINES_LIST(5.121.9,_ALIGNLINE_,5.121.10,,\end{align},\\)
// _ALIGNLINE_IF(TEXT,?\intertext?,5.330.1)ifEND_ALIGNCELLS_
// _ALIGNCELLS_BUCKET(5.121.4,MATH,,,\\|\end{align},)

TNODE* LaTeX2MMLTree::TranslateTeXEqnArray( TNODE* src_eqn,
									          TNODE** out_of_flow_list,
											  U16 usubtype ) {

  TNODE* mml_rv   =  NULL;

// Given usubtype, get the alignment pattern.
//  ie. for eqnarray, "right center left"

  U8 column_align_vals[64];
  GetEQNAlignVals( usubtype,(U8*)column_align_vals,64 );

// This seems to be the only way to handle extra space between lines

  U8 row_spacing_vals[128];
  row_spacing_vals[0] =  0;

  TCI_BOOL found_spacer =  FALSE;
  TCI_BOOL has_tagged_line   =  FALSE;
  U16 line_numbering_mode =  EqnHasNumberedLines( usubtype );

  TNODE* mtr_head =  NULL;		// we build a list of <mtr>'s
  TNODE* mtr_tail;

// Locate the list of lines in the LaTeX source object

  U8 line_list_zuID[16];	// Same for all flavors of eqnarray
  UidsTozuID( 5,TENV_eqnarray,9,line_list_zuID );
  TNODE* list_of_lines  =  FindObject( src_eqn->parts,
	  							              line_list_zuID,INVALID_LIST_POS );

  U16 src_line_counter  =  0;
  U16 dest_line_counter =  0;
  while ( TRUE ) {     	// loop down thru lines in LaTeX eqnarray
    TNODE* mtd_cont_head  =  NULL;	// we build a list of <mtd>'s

// Find the current line in the list of lines
    TNODE* curr_line  =  FindObject( list_of_lines->parts,
        						            line_list_zuID,src_line_counter );
    if ( !curr_line ) break;	// normal exit - no more lines


    TNODE* TeX_EOL  =  FindObject( curr_line->parts,(U8*)"9.5.8",
                                        INVALID_LIST_POS );
    if ( !TeX_EOL )
      TeX_EOL =  FindObject( curr_line->parts,(U8*)"9.5.9",
                                        INVALID_LIST_POS );

// Process the current line

// \intertext<uID5.330.1>!\intertext!REQPARAM(5.330.2,TEXT)
    TNODE* intertext  =  FindObject( curr_line->parts,
       							            (U8*)"5.330.1",INVALID_LIST_POS );

// Put the current intertext under a node in the lines list

    if ( intertext
    &&   intertext->parts
    &&   intertext->parts->contents ) {
      TNODE* local_oof_list =  NULL;
      TNODE* mtext  =  TextInMath2MML( intertext->parts->contents,
   									    &local_oof_list,FALSE,FALSE );
      if ( !mtext )
        mtext  =  MakeSmallmspace();

      U8 v_space[80];
      strcpy( (char*)v_space,"1.0ex " );

	  if ( local_oof_list ) {
        double extra_depth;
        if ( GetVSpaceFromOOFList(local_oof_list,extra_depth) ) {
          if ( extra_depth != 0.0 ) {
    // the default row spacing is 1.0ex
            sprintf( (char*)v_space,"%fex ",extra_depth + 1.0 );
            found_spacer =  TRUE;
          }
        }

        mtext  =  Struts2MML( mtext,local_oof_list );
        local_oof_list  =  DisposeOutOfFlowList( local_oof_list,5,800 );
        mtext  =  Labels2MML( mtext,local_oof_list );
	// We may want to pass this back to the caller in the future.
        DisposeTList( local_oof_list );
	  }

      mtext =  FixImpliedMRow( mtext );
      TNODE* mtd  =  CreateElemWithBucketAndContents( 5,35,14,8,mtext );
      SetNodeAttrib( mtd,(U8*)"groupalign",(U8*)"center" );

      TNODE* mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );

      if ( strlen((char*)row_spacing_vals) < 120 )
        strcat( (char*)row_spacing_vals,(char*)v_space );

	  U8 zlistID[32];
      UidsTozuID( 5,35,11,(U8*)zlistID );
	  U16 zln =  strlen( (char*)zlistID );
	  zlistID[zln]  =  ':';
          // JCS non-standard itoa( 0,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1, "%d", 0);
	  TNODE* col_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );

	  col_list_node->parts  =  mtd;
	  mtd->sublist_owner  =  col_list_node;

      mtr->parts->parts =  col_list_node;
	  col_list_node->sublist_owner  =  mtr->parts;

// Put the intertext under a node in the lines list

      UidsTozuID( 5,35,9,(U8*)zlistID );
	  zln =  strlen( (char*)zlistID );
	  zlistID[zln]  =  ':';
          // JCS non-standard itoa( dest_line_counter,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1, "%d", dest_line_counter);
	  TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );

	  row_list_node->parts  =  mtr;
	  mtr->sublist_owner  =  row_list_node;

// Append

      if ( !mtr_head )
        mtr_head  =  row_list_node;
      else {
        mtr_tail->next =  row_list_node;
	    row_list_node->prev =  mtr_tail;
	  }
      mtr_tail  =  row_list_node;

      dest_line_counter++;
	}


// end intertext

    TNODE* tag  =  NULL;
    TCI_BOOL add_eqn_number =  FALSE;

    U8 r_space[80];
    strcpy( (char*)r_space,"1.0ex " );

    TNODE* line_bucket  =  FindObject( curr_line->parts,
   							              (U8*)"5.121.4",INVALID_LIST_POS );

    if ( line_bucket ) {
// Look for \TCItag, etc.

      if ( line_bucket->contents ) {
        tag   =  FindObject( line_bucket->contents,(U8*)"5.701.0",
                                        INVALID_LIST_POS );
        if ( !tag )
          tag =  FindObject( line_bucket->contents,(U8*)"5.702.0",
                                        INVALID_LIST_POS );
      }


// If there is no tag, determine if the line is enumerated

      if ( tag==NULL && line_numbering_mode ) {
// \notag<uID5.703.0>
        TNODE* notag  =  FindObject( line_bucket->contents,
                             (U8*)"5.703.0",INVALID_LIST_POS );
// \nonumber<uID5.414.0>
        if ( !notag )
          notag =  FindObject( line_bucket->contents,(U8*)"5.414.0",
                                        INVALID_LIST_POS );
        if ( !notag ) {
          if ( ( usubtype == 133 || usubtype == 134 ) // multline
          &&   dest_line_counter > 0 ) {

          } else {
            theequation++;
            add_eqn_number  =  TRUE;
          }
        }
      }

// translate the line to MML

  	  TCI_BOOL do_bindings  =  TRUE;
      U16 tex_nodes_done,error_code;
      TNODE* local_oof_list =  NULL;
      TNODE* contents =  TranslateMathList( line_bucket->contents,
								              do_bindings,NULL,tex_nodes_done,
								              error_code,&local_oof_list );
// WARNING: At present, TeX Explorer ignores <maligngroup> if it is nested
//  in an <mpadded>, so we can't pad lines in an eqnarray
//    U16 vspace_context  =  2;   // mpadded

      double extra_depth;
      if ( GetVSpaceFromOOFList(local_oof_list,extra_depth) ) {
        if ( extra_depth != 0.0 ) {
    // the default row spacing is 1.0ex
          sprintf( (char*)r_space,"%fex ",extra_depth + 1.0 );
          found_spacer =  TRUE;
        }
      }

      U16 vspace_context  =  3;   // ignore, for now
      contents  =  HandleOutOfFlowObjects( contents,&local_oof_list,
   						                  out_of_flow_list,vspace_context );

      if ( TeX_EOL ) {
        U8 depth[64];
        GetEOLSpace( TeX_EOL,depth );
        if ( depth[0] ) {
          strcpy( (char*)r_space,(char*)depth );
          strcat( (char*)r_space," " );
          found_spacer =  TRUE;
        }
        TeX_EOL =  NULL;
      }

  // Add mml "contents" to the list we're building
      if ( contents )
        mtd_cont_head   =  contents;
	  }


// mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd,)!/mtd!
// maligngroup<uID9.20.0>

	  TNODE* alignmark  =  MakeTNode( 0,0,0,(U8*)"9.20.0" );
// mark node as pure whitespace
    SetDetailNum( alignmark,DETAILS_space_width,0 );
    alignmark->next =  mtd_cont_head;
    if ( mtd_cont_head )
      mtd_cont_head->prev =  alignmark;


    TNODE* mml_cell =  CreateElemWithBucketAndContents( 5,35,14,
                                                    8,alignmark );
    if ( column_align_vals[0] ) {
      if ( ( usubtype == 133 || usubtype == 134 ) // multline
      &&   dest_line_counter == 0 ) {
        SetNodeAttrib( mml_cell,(U8*)"groupalign",(U8*)"right" );
      } else
        SetNodeAttrib( mml_cell,(U8*)"groupalign",column_align_vals );
    }


// Put the current mml_cell under a cell_list_item node

    U8 zlistID[32];
    UidsTozuID( 5,35,11,(U8*)zlistID );
	U16 zln =  strlen( (char*)zlistID );
	zlistID[zln]  =  ':';
        // itoa( 0,(char*)zlistID+zln+1,10 );
        sprintf((char*)zlistID+zln+1, "%d", 0);
	TNODE* list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	list_node->parts  =  mml_cell;
	mml_cell->sublist_owner =  list_node;

// Add current one cell line to the list of lines we're building

/*
/mtr<uID5.35.16>
/mtr<uID5.35.16>
mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
_LISTOFCELLS_LIST(5.35.11,_MTCELL_,5.35.12,,/mtr|/mtable,)
_MTCELL_IF(MATH,?mtd?,5.35.14)ifEND

;MathML 2.0
/mlabeledtr<uID5.35.41>
mlabeledtr<uID5.35.40>!mlabeledtr!_EQNNUMBER__LISTOFCELLS_!/mlabeledtr!
_EQNNUMBER_reqELEMENT(5.35.42)
;_LABELEDLIST_LIST(5.35.11,_TDATA_,5.35.12,,/mtr|/mtable,)
;_TDATA_IF(MATH,?mtd?,5.35.14)ifEND
*/

    TNODE* mtr;
    if ( tag && renderer_implements_mlabeledtr ) {
      has_tagged_line  =  TRUE;
      TNODE* tag_cont =  NULL;
	  if ( tag->parts && tag->parts->contents ) {
        TNODE* local_oof_list =  NULL;
	    tag_cont  =  TextInMath2MML( tag->parts->contents,
	  		    	                &local_oof_list,FALSE,FALSE );
	    if ( local_oof_list )
          DisposeTList( local_oof_list );

// \TCItag<uID5.701.0>!\TCItag!REQPARAM(5.701.1,TEXT)

        U16 objclass,subclass,id;
        GetUids( tag->zuID,objclass,subclass,id );
        if ( subclass==701 ) {
// We need to add ( parens ).
          TNODE* head =  MakeTNode( 0,0,0,(U8*)"3.204.1" );
          SetChData( head,(U8*)"(",NULL );
          TNODE* tail =  MakeTNode( 0,0,0,(U8*)"3.204.1" );
          SetChData( tail,(U8*)")",NULL );
          if ( zMMLTextAttrs ) {
//          SetNodeAttrib( head,(U8*)"mathcolor",(U8*)zMMLTextAttrs );
//          SetNodeAttrib( tail,(U8*)"mathcolor",(U8*)zMMLTextAttrs );
            SetMMLAttribs( head,(U8*)zMMLTextAttrs );
            SetMMLAttribs( tail,(U8*)zMMLTextAttrs );
          }
          if ( tag_cont ) {
            head->next  =  tag_cont;
            tag_cont->prev  =  head;
            TNODE* rover  =  tag_cont;
            while ( rover->next )
              rover =  rover->next;
            rover->next =  tail;
            tail->prev  =  rover;
          } else {
            head->next  =  tail;
            tail->prev  =  head;
          }

          tag_cont  =  MMLlistToMRow( head );
        }
	  }

      mtr  =  CreateElemWithBucketAndContents( 5,35,40,42,tag_cont );
      TNODE* part1  =  mtr->parts;

      TNODE* part2  =  MakeTNode( 0,0,0,(U8*)"5.35.11" );
      part2->parts  =  list_node;
	  list_node->sublist_owner =  part2;

	  part1->next =  part2;
	  part2->prev =  part1;

    } else if ( add_eqn_number && do_equation_numbers ) {
      has_tagged_line  =  TRUE;

      U8 ztag[80];
      //sprintf( (char*)ztag,"(%d.%d)",thesection,theequation );
      FormatCurrEqnNumber( ztag );

      TNODE* tag  =  MakeTNode( 0,0,0,(U8*)"3.204.1" );
      SetChData( tag,ztag,NULL );
      if ( zMMLTextAttrs )
//      SetNodeAttrib( tag,(U8*)"mathcolor",(U8*)zMMLTextAttrs );
        SetMMLAttribs( tag,(U8*)zMMLTextAttrs );
      tag =  MMLlistToMRow( tag );

// <mlabeledtr>

      mtr  =  CreateElemWithBucketAndContents( 5,35,40,42,tag );
      TNODE* part1  =  mtr->parts;

      TNODE* part2  =  MakeTNode( 0,0,0,(U8*)"5.35.11" );
      part2->parts  =  list_node;
	  list_node->sublist_owner =  part2;

	  part1->next =  part2;
	  part2->prev =  part1;

    } else {
      mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );
	  mtr->parts->parts =  list_node;
	  list_node->sublist_owner =  mtr->parts;
    }

    if (out_of_flow_list && *out_of_flow_list) {
       TNODE* lis = *out_of_flow_list;  
       if (strcmp((const char*) lis->src_tok, "\\label") == 0 ) {
         U8* marker =  lis->parts->contents->var_value;
         SetNodeAttrib( mtr, (U8*)"marker", marker );
         SetNodeAttrib( mtr, (U8*)"id", marker );
         *out_of_flow_list = lis->next;
       }
    }
   


    if ( strlen((char*)row_spacing_vals) < 120 )
      strcat( (char*)row_spacing_vals,(char*)r_space );

// Put the current line under a node in the lines list

    UidsTozuID( 5,35,9,(U8*)zlistID );
	zln =  strlen( (char*)zlistID );
	zlistID[zln]  =  ':';
        // itoa( dest_line_counter,(char*)zlistID+zln+1,10 );
        sprintf((char*)zlistID+zln+1, "%d", dest_line_counter);
	TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	row_list_node->parts  =  mtr;
	mtr->sublist_owner  =  row_list_node;

// Append

    if ( !mtr_head )
      mtr_head  =  row_list_node;
    else {
      mtr_tail->next =  row_list_node;
	  row_list_node->prev =  mtr_tail;
	}
    mtr_tail  =  row_list_node;

    src_line_counter++;
    dest_line_counter++;

  }		// loop down thru lines in LaTeX eqnarray

// mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
// _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)

  mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
  SetNodeAttrib( mml_rv, (U8*)"type", (U8*)uIDToString(usubtype));
  if ( found_spacer )
    SetNodeAttrib( mml_rv,(U8*)"rowspacing",row_spacing_vals );

  if ( has_tagged_line )
    SetTableLineTaggingAttrs( mml_rv );

  mml_rv->parts->parts  =  mtr_head;
  mtr_head->sublist_owner =  mml_rv->parts;
  return mml_rv;
}



/*
; math displays - the ending tokens are in the MATH/TEXT region

\[<uID5.34.1>!\[!BUCKET(5.34.2,MATH,,,\],)!\]!

\begin{displaymath}<uID5.34.11>!\begin{displaymath}!_DMBUCKET_!\end{displaymath}!
_DMBUCKET_BUCKET(5.34.12,MATH,,,\end{displaymath},)

\begin{equation*}<uID5.34.21>!\begin{equation*}!_ESBUCKET_!\end{equation*}!
_ESBUCKET_BUCKET(5.34.22,MATH,,,\end{equation*},)

$$<uID5.34.31>BUCKET(5.34.32,MATH,$$,$$,,)
*/

TNODE* LaTeX2MMLTree::TranslateTeXDisplay( TNODE* tex_display_node,
											          U16 id,TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  if ( tex_display_node && tex_display_node->parts ) {

    if ( id==41 )  {     // \begin{equation}
      theequation++;     // increment the equation counter
      // These are auto-numbered, which has to be handled outside in pretex
      RecordAnomaly(1010, NULL, 0, 0);
    }
    if ( id == 21 ) {   // \begin{equation*}
      RecordAnomaly(ANOMALY_NONUMBERING, NULL, 0, 0);
    }

    TNODE* mml_cont =  NULL;
    TNODE* tex_cont =  tex_display_node->parts->contents;
	  if ( tex_cont ) {
	    if ( BucketContainsMath(tex_cont) ) {
        TCI_BOOL do_bindings  =  TRUE;
        U16 tex_nodes_done,error_code;
        TNODE* local_oof_list =  NULL;
        mml_cont  =  TranslateMathList( tex_cont,do_bindings,NULL,
						            tex_nodes_done,error_code,&local_oof_list );
        mml_cont  =  HandleOutOfFlowObjects( mml_cont,
      							    &local_oof_list,out_of_flow_list,2 );
	    } else {          // MATH display contains no math
        U16 ilk   =  1010;
        U8* ztext =  NULL;
        U32 off1  =  0;
        U32 off2  =  0;
        RecordAnomaly( ilk,ztext,off1,off2 );
		    return mml_rv;
	    }
	  }

    if ( id==41 && do_equation_numbers ) {
// Handle equation number here - nest in <mtable> <mlabeledtr> etc.
      mml_rv  =  AddNumberToEquation( mml_cont );
    } else {
      mml_rv  =  MMLlistToMRow( mml_cont );
	    mml_rv->src_linenum =  tex_display_node->src_linenum;
    }

  } else
    TCI_ASSERT(0);

  return mml_rv;
}


// Nest an mml_list (generated by translating a LaTeX math run)
//  in the parts bucket of an mrow.

TNODE* LaTeX2MMLTree::MMLlistToMRow( TNODE* mml_list ) {

// /mrow<uID5.750.0>
// mrow<uID5.750.1>!mrow!BUCKET(5.750.2,MATH,,,/mrow)!/mrow!

  bool do_it  =  true;
  if ( mml_list && !mml_list->next ) {	// a single node
    U16 uobjtype,usubtype,uID;
    GetUids( mml_list->zuID,uobjtype,usubtype,uID );
	  if ( uobjtype==5 && usubtype==750 && uID==1 )
      do_it  =  false;		// mml_list is already an mrow
  }

  TNODE* rv;
  if ( do_it ) {
    rv =  MakeTNode( 0L,0L,0L,(U8*)"5.750.1" );
    TNODE* p  =  MakeTNode( 0L,0L,0L,(U8*)"5.750.2" );
    rv->parts =  p;
    p->sublist_owner  =  rv;
    p->contents =  mml_list;
	  if ( mml_list )
      mml_list->sublist_owner  =  p;
  } else
    rv  =  mml_list;

  return rv;
}


//Sum<uID7.1.?>prefix,29,largeop="true" movablelimits="true" stretchy="true" lspace="0em" rspace=".16666em"

U16 LaTeX2MMLTree::SetMMLBigOpEntity( TNODE* tex_bigop_node,
								  	    TNODE* mml_base_op ) {

  U16 precedence  =  32;

  U8 mml_zuID[32];
  strcpy( (char*)mml_zuID,(char*)tex_bigop_node->zuID );
  char* ptr =  strchr( (char*)mml_zuID,',' );
  if ( ptr )
    *ptr  =  0;

  U8* dest_zname;
  U8* d_template;
  if ( d_mml_grammar->GetGrammarDataFromUID( mml_zuID,
      				context_math,&dest_zname,&d_template ) ) {
	if ( d_template && *d_template ) {
	  char* p =  strchr( (char*)d_template,',' );
	  if ( p )
	    precedence  =  atoi(p+1);
      else
	    TCI_ASSERT(0);

      U8 entity_buffer[128];
      entity_buffer[0]  =  0;
      if ( output_entities_as_unicodes )
        GetUnicodeEntity( d_template,entity_buffer );
      SetChData( mml_base_op,dest_zname,entity_buffer );

    } else
	  TCI_ASSERT(0);

  } else {			// big op lookup failed
	TCI_ASSERT(0);
    SetChData( mml_base_op,(U8*)"unknown",NULL );
  }

  return precedence;
}


/*
\BF<uID4.2.1>!\BF!!{!VAR(3.0.0,MATH,a,,})!}!
\NEG<uID4.3.2>!\NEG!!{!VAR(4.3.51,MATH,a,,})!}!
*/

TNODE* LaTeX2MMLTree::BoldSymbolToMML( TNODE* TeX_node,
											                  U16& advance ) {

  TNODE* mml_rv =  NULL;
  advance =  1;

  if ( TeX_node->parts && TeX_node->parts->contents ) {
    TNODE* TeX_cont =  TeX_node->parts->contents;

    U16 tex_nodes_done,error_code;
  	TCI_BOOL do_bindings  =  TRUE;
		TNODE* local_oof_list =  NULL;
    mml_rv  =  TranslateMathList( TeX_cont,do_bindings,NULL,
									tex_nodes_done,error_code,
									&local_oof_list );
    if ( local_oof_list )
      DisposeTList( local_oof_list );

  } else
    TCI_ASSERT(0);

  return mml_rv;
}


// All math spaces EXPECT vertical spacing elements
//  are handled in the following function.

TNODE* LaTeX2MMLTree::LaTeXHSpacing2MML( TNODE* tex_math_space,
										  U16 usubtype,U16 uID,
										  U16& nodes_done ) {

  TNODE* mml_rv =  NULL;

  // Note that only uobj == 9 LaTeX TNODEs are passed in.

  nodes_done  =  1;       // assumed
  switch ( usubtype ) {
    case  1 :
      mml_rv  =  MathHSpacesToMML( tex_math_space,nodes_done );
	break;
    case  2 :
//  \par<uID9.2.0>
	break;
    case  3 :	// vertical space - handled before this point
      TCI_ASSERT(0);
	break;
    case  5 : {
// \allowbreak<uID9.5.1>  
// \nolinebreak<uID9.5.3>!\nolinebreak!OPTPARAM(9.5.20,NONLATEX)
// \\<uID9.5.8>!\\!OPTPARAM(9.5.23,DIMEN)
// \\*<uID9.5.9>!\\*!OPTPARAM(9.5.24,DIMEN)
	  if ( uID == 1 ) {

/*  I'm blocking \allowbreak for now - allowing it adds more work
      to semantic analysis.

        mml_rv  =  MakeTNode( 0L,0L,tex_math_space->src_linenum,(U8*)zmspace );
        SetNodeAttrib( mml_rv,(U8*)"linebreak",(U8*)"goodbreak" );
        SetDetailNum( mml_rv,DETAILS_space_width,0 );
*/

	  }
	}
	break;
    case  8 :
// \protect<uID9.8.0>
	break;
    case  9 :
// \hline<uID9.9.0>
	break;
	default :
	  TCI_ASSERT(0);
	break;
  }

  return mml_rv;
}


/* TeX's horizontal spacing objects are quite difficult to handle
  in MathML.  Two mml elements can be generated.

<mtext> &Space;&NonBreakingSpace;&ThickSpace; </mtext>

  - Used for wide spaces that would generally not occur inside an expression.
  - Some spacing entities are as follows:
&Space;             00020       one em of space in the current font 
&NonBreakingSpace;  000A0       space that is not a legal breakpoint 
&ZeroWidthSpace;    0200B       space of no width at all 
&VeryThinSpace;     0200A       space of width 1/18 em 
&ThinSpace;         02009       space of width 3/18 em 
&MediumSpace;       02005       space of width 4/18 em 
&ThickSpace;        02009-0200A space of width 5/18 em 

<mspace width="...em" />

  - Used for narrow spaces that might occur within an expression,
    or spaces that have an absolute width, like \hspace{1.0in}

  - The narrowest of these are flag as "absorbable".  These may
    be absorbed into the lspace or rspace of an adjacent operator.

  Note that several LaTeX horizontal spacing elements can occur
  consecutively.  We attempt to coelesce here.
*/

TNODE* LaTeX2MMLTree::MathHSpacesToMML( TNODE* tex_math_space,
										  	U16& nodes_done ) {

  TNODE* mml_rv =  NULL;

  nodes_done  =  0;
  TCI_BOOL no_break   =  FALSE;
  TCI_BOOL zero_space =  FALSE;
  TCI_BOOL has_hspace =  FALSE;
  U16 TeX_fill  =  0;

  U8 mtext_buffer[5000];
  mtext_buffer[0] =  0;

  U8 munic_buffer[5000];
  munic_buffer[0] =  0;

  U8 hspace_buffer[5000];
  hspace_buffer[0]  =  0;

  I16 ems_width =  0;

  TNODE* rover  =  tex_math_space;
  while ( rover ) {

    U16 objclass,subclass,id;
    GetUids( rover->zuID,objclass,subclass,id );

    if ( objclass==9 && subclass==1 ) {   // horizontal spacer

      switch ( id ) {

// spaces that are intrepreted as ending expressions - mtext

        case  2 :   // \ <uID9.1.2>
          ems_width +=  5;
          AppendEntityToBuffer( (U8*)"9.1.7",mtext_buffer,munic_buffer );
        break;
        case  3 :   // ~<uID9.1.3>		  non-breaking
          no_break  =  TRUE;
          ems_width +=  5;
          AppendEntityToBuffer( (U8*)"9.1.7",mtext_buffer,munic_buffer );
        break;
        case  4 :   // \quad<uID9.1.4>	em
          ems_width +=  18;
          AppendEntityToBuffer( (U8*)"9.1.18",mtext_buffer,munic_buffer );
        break;
        case  5 :   // \qquad<uID9.1.5>	2-em
          ems_width +=  36;
          AppendEntityToBuffer( (U8*)"9.1.18",mtext_buffer,munic_buffer );
          AppendEntityToBuffer( (U8*)"9.1.18",mtext_buffer,munic_buffer );
        break;

        case 10 :   // {}<uID9.1.10>	  empty
          zero_space =  TRUE;
          AppendEntityToBuffer( (U8*)"9.1.10",mtext_buffer,munic_buffer );
        break;

// spaces that don't necessarily end expressions - mspace
//   absorbable

        case  6 :   // \,<uID9.1.6>		  thin
          ems_width +=  3;
          AppendEntityToBuffer( (U8*)"9.1.6",mtext_buffer,munic_buffer );
        break;
        case  7 :   // \;<uID9.1.7>		  thick
          ems_width +=  5;
          AppendEntityToBuffer( (U8*)"9.1.7",mtext_buffer,munic_buffer );
        break;
        case  8 :   // \/<uID9.1.8>		  italic correction
          ems_width +=  1;
          AppendEntityToBuffer( (U8*)"9.1.12",mtext_buffer,munic_buffer );
        break;
//   not absorbable
        case  9 :   // \!<uID9.1.9>		  negative thin
          AppendEntityToBuffer( (U8*)"9.1.9",mtext_buffer,munic_buffer );
          ems_width -=  3;
        break;

// \hspace{GLUE}

        case 12 :   // \hspace<uID9.1.12>!\hspace!REQPARAM(9.1.20,GLUE)
        case 13 : { // \hspace*<uID9.1.13>!\hspace*!REQPARAM(9.1.21,GLUE)
	      U8 bucket_zuID[32];
          UidsTozuID( 9,1,id+8,(U8*)bucket_zuID );
          TNODE* glue_bucket =  FindObject( rover->parts,
						              (U8*)bucket_zuID,INVALID_LIST_POS );
          GetValueFromGlue( glue_bucket,(U8*)hspace_buffer,
                                FALSE,ems_width );
          has_hspace =  TRUE;
        }
        break;

// Horizontal fills - stretchy

        case 14 :   // \hfill<uID9.1.14>
          TeX_fill   =  1;
          AppendEntityToBuffer( (U8*)"9.1.18",mtext_buffer,munic_buffer );
          AppendEntityToBuffer( (U8*)"9.1.18",mtext_buffer,munic_buffer );
          ems_width +=  36;
        break;
        case 15 :   // \hrulefill<uID9.1.15>
          TeX_fill   =  2;
          strcat( (char*)mtext_buffer,(char*)"_______" );
          strcat( (char*)munic_buffer,(char*)"_______" );
        break;
        case 16 :   // \dotfill<uID9.1.16>
          TeX_fill   =  3;
          strcat( (char*)mtext_buffer,(char*)"......." );
          strcat( (char*)munic_buffer,(char*)"......." );
        break;

// ??????

        case 17 :   // \@<uID9.1.17>
          ems_width +=  18;
          AppendEntityToBuffer( (U8*)"9.1.18",mtext_buffer,munic_buffer );
        break;

        case 22 :   // \extracolsep<uID9.1.22>!\extracolsep!REQPARAM(9.3.23,GLUE)
          ems_width +=  18;
          AppendEntityToBuffer( (U8*)"9.1.18",mtext_buffer,munic_buffer );
        break;

	    default :
	      TCI_ASSERT(0);
	    break;
      }

      nodes_done++;
      rover =  rover->next;

    } else
      break;

  }   // loop thru source h-spacing nodes

  TCI_ASSERT( nodes_done );


  if        ( TeX_fill ) {          // <mtext>
    mml_rv =  MakeTNode( 0L,0L,tex_math_space->src_linenum,
                                            (U8*)zmtext );
    SetChData( mml_rv,mtext_buffer,munic_buffer );
    if ( TeX_fill == 1 )      // whitespace
      SetDetailNum( mml_rv,DETAILS_space_width,ems_width );

  } else if ( zero_space ) {        // <mtext>

    mml_rv =  MakeTNode( 0L,0L,tex_math_space->src_linenum,
                                            (U8*)zmtext );
    U8 buffer[32];
    buffer[0]   =  0;
    U8 buffer2[32];
    buffer2[0]  =  0;
    AppendEntityToBuffer( (U8*)"9.1.10",buffer,buffer2 );
    SetChData( mml_rv,buffer,buffer2 );
    SetDetailNum( mml_rv,DETAILS_space_width,0 );

  } else if ( ems_width <= 5 ) {    // very narrow - <mspace/>

    U8* named_width_attr =  NULL;

    if      ( ems_width < 0 )       // \!<uID9.1.9>		negative thin
      named_width_attr =  (U8*)"negativethinmathspace";
    else if ( ems_width == 0 )      // \ xxx
      named_width_attr =  (U8*)"veryverythinmathspace";
    else if ( ems_width <= 1 )      // \/<uID9.1.8>		italic correction
      named_width_attr =  (U8*)"veryverythinmathspace";
    else if ( ems_width <= 3 )      // \,<uID9.1.6>		thin
      named_width_attr =  (U8*)"thinmathspace";
    else if ( ems_width <= 4 )      // \;<uID9.1.7>		thick
      named_width_attr =  (U8*)"mediummathspace";
    else
      named_width_attr =  (U8*)"thickmathspace";


    mml_rv  =  MakeTNode( 0L,0L,tex_math_space->src_linenum,(U8*)zmspace );
	U16 zln =  strlen( (char*)named_width_attr );
    mml_rv->attrib_list =  MakeATTRIBNode( (U8*)"width",ELEM_ATTR_width,0,
			  								    named_width_attr,zln,0 );
    if ( no_break )
      SetNodeAttrib( mml_rv,(U8*)"linebreak",(U8*)"nobreak" );

    SetDetailNum( mml_rv,DETAILS_space_width,ems_width );

  } else {            // moderate-to-wide horizontal space
  
    if ( has_hspace ) {         // <mspace>
      mml_rv =  MakeTNode( 0L,0L,tex_math_space->src_linenum,
                                            (U8*)zmspace );
      U16 zln =  strlen( (char*)hspace_buffer );
      mml_rv->attrib_list =  MakeATTRIBNode( (U8*)"width",
			                    ELEM_ATTR_width,0,hspace_buffer,zln,0 );
    } else {                    // <mtext>
      mml_rv =  MakeTNode( 0L,0L,tex_math_space->src_linenum,
                                            (U8*)zmtext );
      SetChData( mml_rv,mtext_buffer,munic_buffer );
    }
    SetDetailNum( mml_rv,DETAILS_space_width,ems_width );

  }

  return mml_rv;
}


U8* LaTeX2MMLTree::GetFuncName( TNODE* nom_bucket_contents ) {

  U16 uobjtype,usubtype,tex_uID;
  GetUids( nom_bucket_contents->zuID,uobjtype,usubtype,tex_uID );

  U8* znom;
  if ( uobjtype==888 && usubtype==8 && tex_uID==0 ) {
	  znom  =  nom_bucket_contents->var_value;
  } else {
    znom  =  (U8*)"Unknown";
    TCI_ASSERT(0);
  }

  U16 zln =  strlen( (char*)znom );

  U8* rv  =  (U8*)TCI_NEW( char[zln+1] );
  strcpy( (char*)rv,(char*)znom );

  return rv;
}

/*
  MathML provides 2 different markups for TeX fences like (x,y).
  <mfenced>         <mo fence="true">(</mo>
    <mi>x</mi>      <mrow>  
    <mi>y</mi>        <mi>x</mi>  
  </mfenced>	        <mo separator="true">,</mo>  
                      <mi>y</mi>  
  					        </mrow>  
                    <mo fence="true">)</mo>  
                      
In the initial implementation, a LaTeX fence is mapped
  to an <mrow> containing <mo>s with attributes for fences.

We may want to generate <mfenced> if 
  1. the delimiters are ()
  2. the contents are letters and/or numbers separated
     by commas

LaTeX - NoteBook.gmr
5.70.0,Fence - parts
  5.70.1,Starter var - left delimiter
  5.70.2,MATH bucket
  5.70.3,Ender   var - right delimiter
*/

TNODE* LaTeX2MMLTree::LaTeXFence2MML( TNODE* tex_fence_node,
										                TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  TNODE* tex_left_delim   =  FindObject( tex_fence_node->parts,
                                (U8*)"5.70.1",INVALID_LIST_POS );
  U8 mml_lfence_nom[64];
  U8 lunicode_nom[64];
  U8 mml_lfence_attrs[128];
  TeXDelimToMMLOp( tex_left_delim->var_value,1,
                        mml_lfence_nom,lunicode_nom,mml_lfence_attrs );
  TNODE* tex_right_delim  =  FindObject( tex_fence_node->parts,
                                (U8*)"5.70.3",INVALID_LIST_POS );
  U8 mml_rfence_nom[64];
  U8 runicode_nom[64];
  U8 mml_rfence_attrs[128];
  TeXDelimToMMLOp( tex_right_delim->var_value,3,
                        mml_rfence_nom,runicode_nom,mml_rfence_attrs );

  TNODE* math_bucket  =  FindObject( tex_fence_node->parts,
                                (U8*)"5.70.2",INVALID_LIST_POS );
  TNODE* TeX_contents;
  if ( math_bucket )
    TeX_contents  =  math_bucket->contents;
  else
    TeX_contents  =  NULL;

  TNODE* mml_body =  NULL;
  if ( TeX_contents ) {
    TCI_BOOL do_bindings  =  TRUE;
    U16 tex_nodes_done,error_code;
    TNODE* local_oof_list =  NULL;
    mml_body  =  TranslateMathList( TeX_contents,do_bindings,NULL,
						            tex_nodes_done,error_code,&local_oof_list );
    mml_body  =  HandleOutOfFlowObjects( mml_body,
    						        &local_oof_list,out_of_flow_list,3 );
  // If there are any operators in mml_body, we nest it.
	  if ( mml_body && DoNestFenceBody(mml_body) )
      mml_body =  CreateElemWithBucketAndContents( 5,750,
           											            1,2,mml_body );
  }

// mo for left delimiter
  TNODE* left_end   =  MakeTNode( 0L,0L,tex_fence_node->src_linenum,
                                (U8*)"3.203.1" );
  SetChData( left_end,mml_lfence_nom,lunicode_nom );
  SetAttrsForFenceMO( left_end,7,TRUE );

// mo for right delimiter
  TNODE* right_end  =  MakeTNode( 0L,0L,tex_fence_node->src_linenum,
                                (U8*)"3.203.1" );
  SetChData( right_end,mml_rfence_nom,runicode_nom );
  SetAttrsForFenceMO( right_end,7,FALSE );

// join left + body + right

  TNODE* start;
  TNODE* tail;
  if ( left_end ) {
    start =  left_end;
    left_end->next  =  mml_body;
    if ( mml_body )
      mml_body->prev  =  left_end;
  } else
    start =  mml_body;

  if ( start ) {
    tail  =  start;
    while ( tail->next )
      tail  =  tail->next;
  } else {
    start =  right_end;
    tail  =  right_end;
  }

  if ( right_end && right_end != tail ) {
    tail->next  =  right_end;
    right_end->prev =  tail;
  }

// Nest in an mrow
  if ( start )
    mml_rv  =  CreateElemWithBucketAndContents( 5,750,1,2,start );

// Set DETAILS, delimited_group
  if ( mml_rv ) {
    SetDetailNum( mml_rv,DETAILS_delimited_group,1 );
    SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_INNER );
  }

  return  mml_rv;
}           // LaTeXFence2MML


/*
// Function to determine if we are going to translate a LaTeX
//  fence as a MathML "mfenced".

void LaTeX2MMLTree::CheckFenceBody( U8* left_uID,U8* right_uID,
									  TNODE* mml_body,
                              		    TCI_BOOL& use_mfenced ) {

  use_mfenced =  FALSE;

  if ( !left_uID || !right_uID ) {
    TCI_ASSERT(0);
    return;
  }
  if ( !mml_body ) {
  // empty
    return;
  }

  U16 uobjtype,usubtype,uID;
  GetUids( left_uID,uobjtype,usubtype,uID );
  if ( uobjtype!=10 || usubtype!=4 ) return;    // not a fence
  //if ( uID != 3 ) return;                       //  not (

  GetUids( right_uID,uobjtype,usubtype,uID );
  if ( uobjtype!=10 || usubtype!=4 ) return;    // not a fence
  //if ( uID != 4 ) return;                       // not )

  TCI_BOOL OK =  TRUE;			// Assume OK

// Traverse the nodes that constitute the contents of the fence.
//  Look for comma separators, etc.

  U16 item_ord  =  0;
  TNODE* rover  =  mml_body;
  while ( rover ) {
    item_ord++;
    GetUids( rover->zuID,uobjtype,usubtype,uID );

    if        ( uobjtype==3 ) {         // atomic object

      if        ( usubtype==201 && uID==1 ) {	// mi
	    if ( (item_ord % 2) == 0 )
          OK =  FALSE;
      } else if ( usubtype==202 && uID==1 ) {	// mn
	    if ( (item_ord % 2) == 0 )
          OK =  FALSE;
      } else if ( usubtype==203 && uID==1 ) {	// mo
	    if ( (item_ord % 2) == 0 ) {
		  if ( rover->v_len != 1 || rover->var_value[0] != ',' )
            OK =  FALSE;
		} else
          OK =  FALSE;
	  }

    } else if ( uobjtype==5 ) {
      OK =  FALSE;

    } else
      OK =  FALSE;

    rover =  rover->next;
  }

  if ( item_ord < 2 )
    use_mfenced =  TRUE;
  else if ( OK ) {
	if ( (item_ord % 2) == 1 )
      use_mfenced =  TRUE;
  }
}
*/


// Given a LaTeX fence "ilk" uID,
//  look up the corresponding MathML operator.

void LaTeX2MMLTree::TeXDelimToMMLOp( U8* TeX_fence_delim_uID,
                                        U16 left_center_right,
									    U8* mml_fence_nom,
									    U8* unicode_nom,
										U8* mml_fence_attrs ) {

  mml_fence_nom[0]    =  0;
  unicode_nom[0]      =  0;
  mml_fence_attrs[0]  =  0;

  if ( TeX_fence_delim_uID ) {
    U8 lookup_uID[32];
    strcpy( (char*)lookup_uID,(char*)TeX_fence_delim_uID );

    U16 uobj,usub,id;
    GetUids( TeX_fence_delim_uID,uobj,usub,id );
    if ( id==20 ||id==21 ) {  // \vert and \Vert
      if      ( left_center_right==1 )
        strcat( (char*)lookup_uID,",l" );
      else if ( left_center_right==3 )
        strcat( (char*)lookup_uID,",r" );
    }

    U8* dest_zname;

    U8* d_template;
    if ( d_mml_grammar->GetGrammarDataFromUID(lookup_uID,
							(U8*)"FENCE",&dest_zname,&d_template ) ) {

	  if ( dest_zname[0] != '.' )	// not the invisible fence
		strcpy( (char*)mml_fence_nom,(char*)dest_zname );

	  if ( d_template && d_template[0] ) {

        if ( usub == 4 ) {
          if ( output_entities_as_unicodes ) {
			unicode_nom[0]  =  0;
            GetUnicodeEntity( d_template,unicode_nom );
          }

        } else {
  		  U16 zln =  strlen( (char*)d_template );
	  	  if ( zln < 128 )
		    strcat( (char*)mml_fence_attrs,(char*)d_template );
        }
	  }

	} else{
	  //TCI_ASSERT(0);
	}
  }		// if ( tex_fence_delim_node )

}


// merror<uID5.601.0>!merror!reqELEMENT(5.601.2)!/merror!

TNODE* LaTeX2MMLTree::CreateMError( const char* err_text,
                                        U32 src_linenum ) {

  TNODE* mml_rv =  MakeTNode( 0L,0L,0L,(U8*)"5.601.0" );

  TNODE* rv_parts =  MakeTNode( 0L,0L,0L,(U8*)"5.601.2" );
  mml_rv->parts =  rv_parts;
  rv_parts->sublist_owner =  mml_rv;

  TNODE* contents =  MakeTNode( 0L,0L,0L,(U8*)zmtext );	// <mtext>
  rv_parts->contents  =  contents;
  contents->sublist_owner =  rv_parts;

  char msg[128];
  sprintf( msg,err_text,src_linenum );
  SetChData( contents,(U8*)msg,NULL );

  return mml_rv;
}


TNODE* LaTeX2MMLTree::CreateElemWithBucketAndContents(
						    U16 uobjtype,U16 usubtype,U16 uID,
							U16 bucketID,TNODE* contents ) {

  U8 zuID[32];
  UidsTozuID( uobjtype,usubtype,uID,(U8*)zuID );
  TNODE* rv =  MakeTNode( 0L,0L,0L,(U8*)zuID );
  UidsTozuID( uobjtype,usubtype,bucketID,(U8*)zuID );
  TNODE* bucket =  MakeTNode( 0L,0L,0L,(U8*)zuID );

  rv->parts =  bucket;
  bucket->sublist_owner =  rv;

  if ( contents ) {
    bucket->contents  =  contents;
  	contents->sublist_owner =  bucket;
  }

  return rv;
}


TNODE* LaTeX2MMLTree::CreateBucketWithContents(
							          U16 uobjtype,U16 usubtype,U16 bucketID,
								        TNODE* contents ) {

  U8 zuID[32];
  UidsTozuID( uobjtype,usubtype,bucketID,(U8*)zuID );
  TNODE* rv =  MakeTNode( 0L,0L,0L,(U8*)zuID );
  if ( contents ) {
    rv->contents  =  contents;
  	contents->sublist_owner =  rv;
  }

  return rv;
}


TNODE* LaTeX2MMLTree::BindByOpPrecedence( TNODE* MML_list,
  										    U16 p_max,U16 p_min ) {

  TNODE* rv =  MML_list;

  U16 lowest_prec;
  U16 highest_prec;
  GetPrecedenceBounds( MML_list,lowest_prec,highest_prec );

  U16 curr_prec =  p_max;

  while ( curr_prec >= p_min
  &&      curr_prec >= lowest_prec ) {	// loop down thru p levels

    TNODE* MML_rover  =  rv;
    while ( MML_rover ) {		// loop thru contents nodes

      TNODE* new_mrow;

// We only look at nodes with form and precedence,
//  operators or embellished operators.

      if ( MML_rover->details
      &&   MML_rover->details->form != UNDEFINED_DETAIL
      &&   MML_rover->details->precedence != UNDEFINED_DETAIL ) {
	    I16 precedence  =  MML_rover->details->precedence;
		I16 form        =  MML_rover->details->form;
		if ( precedence == curr_prec ) {

		  if        ( form==1 ) {		// prefix

            U16 n_right_spaces;
            U16 r_nodes_spanned;
			I16 right_space;
            if ( OperandExists(MML_rover,TRUE,0,n_right_spaces,
								  r_nodes_spanned,right_space) ) {

              rv  =  BindPrefixOP( rv,MML_rover,curr_prec,&new_mrow );
              if ( new_mrow ) {
                MML_rover =  new_mrow;
                SetDetailIsExpr( new_mrow,curr_prec );
                if ( curr_prec==55 || curr_prec==54 ) {	// DD and dd
                  SetDetailNum( new_mrow,DETAILS_is_differential,1 );
                }
              } else {
                if ( MML_rover->next ){
                  TCI_ASSERT(0);
                }
	    	  }
			} else {
              //TCI_ASSERT(0);
			}

		  } else if ( form==2 ) {		// infix

            U16 r_space_nodes,r_operand_nodes;
            U16 l_space_nodes,l_operand_nodes;
		    if ( LocateOperand(MML_rover,TRUE,
	                    r_space_nodes,r_operand_nodes,form,precedence)
		    &&   LocateOperand(MML_rover,FALSE,
	                    l_space_nodes,l_operand_nodes,form,precedence) ) {

			  if ( l_operand_nodes > 1 ) {
			    TCI_ASSERT(0);
			    TNODE* right_anchor =  MML_rover;
			    U16 tally =  l_space_nodes;
			    while ( tally ) {
			      right_anchor  =  right_anchor->prev;
			      tally--;
			    }
			    rv  =  NestNodesInMrow( rv,right_anchor,NULL,FALSE,
			  			                    l_operand_nodes,&new_mrow );
			  }

			  if ( r_operand_nodes > 1 ) {
			    TCI_ASSERT(0);
			    TNODE* left_anchor  =  MML_rover;
			    U16 tally =  r_space_nodes;
			    while ( tally ) {
			      left_anchor =  left_anchor->next;
			      tally--;
			    }
			    rv  =  NestNodesInMrow( rv,left_anchor,NULL,TRUE,
			  				            r_operand_nodes,&new_mrow );
			  }

			  U16 node_to_nest  =  3 + l_space_nodes + r_space_nodes;
			  TNODE* start  =  MML_rover->prev;
			  U16 tally =  l_space_nodes;
			  while ( tally ) {
			    start =  start->prev;
			    tally--;
			  }
			  rv  =  NestNodesInMrow( rv,NULL,start,TRUE,
			  					            node_to_nest,&new_mrow );

			  if ( l_operand_nodes == 1 ) {
			    if ( LeftGroupHasSamePrec(MML_rover,curr_prec,l_space_nodes) )
			      TNODE* tmp  =  ElevateLeftOperand( NULL,MML_rover,l_space_nodes );
			  }

			  if ( new_mrow ) {
			    MML_rover =  new_mrow;
                SetDetailIsExpr( new_mrow,curr_prec );
			  } else
			    TCI_ASSERT(0);

			} else {
			  if ( MML_rover->next || MML_rover->prev ) {
			    //TCI_ASSERT(0);
        }
			}

		  } else if ( form==3 ) {		// postfix

            U16 l_space_nodes,l_operand_nodes;
		    if ( LocateOperand(MML_rover,FALSE,
	                  l_space_nodes,l_operand_nodes,form,precedence) ) {

			  if ( l_operand_nodes > 1 ) {
			    TCI_ASSERT(0);
			    TNODE* right_anchor =  MML_rover;
			    U16 tally =  l_space_nodes;
			    while ( tally ) {
			      right_anchor  =  right_anchor->prev;
			      tally--;
			    }
			    rv  =  NestNodesInMrow( rv,right_anchor,NULL,FALSE,
			  					            l_operand_nodes,&new_mrow );
			  }

			  U16 node_to_nest  =  2 + l_space_nodes;
			  TNODE* start  =  MML_rover->prev;
			  U16 tally =  l_space_nodes;
			  while ( tally ) {
			    start =  start->prev;
			    tally--;
			  }
			  rv  =  NestNodesInMrow( rv,NULL,start,TRUE,
			  				              node_to_nest,&new_mrow );

			  if ( l_operand_nodes == 1 ) {
			    if ( LeftGroupHasSamePrec(MML_rover,curr_prec,l_space_nodes) )
			      TNODE* tmp  =  ElevateLeftOperand( NULL,MML_rover,l_space_nodes );
			  }

			  if ( new_mrow ) {
			    MML_rover =  new_mrow;
                SetDetailsForBoundPostfix( new_mrow,curr_prec );
			  } else
			    TCI_ASSERT(0);

			} else if ( MML_rover->prev ) {
			  TCI_ASSERT(0);
      }

		  } else {
		    TCI_ASSERT(0);
      }

		}     // if ( precedence == curr_prec )

	  }   // if ( MML_rover->details )

	  MML_rover =  MML_rover->next;

	}         // loop thru MML node list

    curr_prec--;
  }		// loop down thru operator precedence levels


  return rv;
}


void LaTeX2MMLTree::InsertAF( TNODE* func_node,TCI_BOOL arg_is_delimited ) {

  TNODE* af_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );	// <mo>

  U8 entity[32];
  entity[0] =  0;
  U8 uni_entity[32];
  uni_entity[0] =  0;
  AppendEntityToBuffer( (U8*)"3.5.6",entity,uni_entity );
  SetChData( af_node,(U8*)entity,(U8*)uni_entity );

  TCI_BOOL in_script  =  GetScriptStatus();
  TCI_BOOL emulate_TeX  =  FALSE;
  if ( in_script ) {
    if ( spacing_mode_in_scripts == 3 )
      emulate_TeX  =  TRUE;
  } else {
    if ( mo_spacing_mode == 3 )
      emulate_TeX  =  TRUE;
  }

  if ( emulate_TeX ) {
    U16 op_ilk;
    if ( func_node->next->details
    &&   func_node->next->details->TeX_atom_ilk!=UNDEFINED_DETAIL )
      op_ilk  =  func_node->next->details->TeX_atom_ilk;
    else {
      op_ilk  =  arg_is_delimited ? TeX_ATOM_INNER : TeX_ATOM_ORD;
    }
    I16 TeX_space_right =  GetTeXSpacing( TeX_ATOM_OP,op_ilk );
    SetMOSpacingFromID( af_node,TeX_space_right,FALSE,in_script );

  } else {
    if ( !arg_is_delimited )
      SetMOSpacing( af_node,FALSE,(U8*)"verythinmathspace" );
  }

  I16 precedence  =  65;
  SetDetailNum( af_node,DETAILS_form,COPF_INFIX );
  SetDetailNum( af_node,DETAILS_precedence,precedence );
  af_node->prev   =  func_node;
  af_node->next   =  func_node->next;

  func_node->next->prev =  af_node;
  func_node->next =  af_node;
}


// After the initial MML list is generated, we pass thru it
//  many more times to improve the MathML.  In one early pass,
//  we insert "ApplyFunction" operators between each
//  function and its arg.

void LaTeX2MMLTree::InsertApplyFunction( TNODE* MML_list ) {

  TCI_BOOL do_second_pass  =  FALSE;

  TNODE* mml_rover  =  MML_list;
  while ( mml_rover ) {	// loop thru level one contents nodes
    TCI_BOOL do_insert  =  FALSE;
    TCI_BOOL is_delimited =  FALSE;
    if ( MMLNodeIsFunction(mml_rover) ) {
      U16 n_space_nodes,n_arg_nodes;

      if ( FunctionHasArg(mml_rover,n_space_nodes,n_arg_nodes,is_delimited) ) {
        do_insert =  TRUE;

	      if ( n_space_nodes+n_arg_nodes > 1 ) {
            // Multiple nodes in the arg, as in "sin 2x"
	          // Here we complete bindings within the arg.
		      TNODE* tail =  mml_rover;
		      U16 tally =  0;
	        while ( tally < n_space_nodes+n_arg_nodes ) {
		        tail  =  tail->next;
		        tally++;
		      }
		      TNODE* right_anchor =  tail->next;
		      tail->next  =  NULL;
          TNODE* arg_nodes  =  mml_rover->next;
          mml_rover->next =  NULL;
          TNODE* new_arg  =  FinishMMLBindings( arg_nodes );
          SetDetailNum( new_arg,DETAILS_is_func_arg,1 );
		      mml_rover->next =  new_arg;
		      new_arg->prev   =  mml_rover;
		      if ( right_anchor ) {
		        TNODE* new_arg_tail =  new_arg;
	          while ( new_arg_tail->next )
		          new_arg_tail  =  new_arg_tail->next;
		        new_arg_tail->next  =  right_anchor;
		        right_anchor->prev  =  new_arg_tail;
		      }
	      }
	    }
	  } else if ( MMLNodeIsOperator(mml_rover) ) {

	  } else {		// possible implicit function, f,g,...
	  U16 args_on_right =  0;
	  if ( mml_rover->next )
	     if ( mml_rover->next->details )
	       if ( mml_rover->next->details->delimited_group != UNDEFINED_DETAIL ) {
	         args_on_right =  IsGroupAFuncArg( mml_rover->next );
           is_delimited  =  TRUE;
         }
         if ( args_on_right ) {
           if ( IsFuncDerivative(mml_rover) ) {
		         do_insert =  TRUE;
	         } else {
             U8 identifier_nom[80];	// We record name of each possible
	           identifier_nom[0] =  0;   //  implicit function, f, myfunc,...
	           TCI_BOOL in_funcs_list  =  FALSE;
             if ( MMLNodeCouldBeFunction(mml_rover,identifier_nom,80) ) {
	             if ( IdentifierInList(funcs_list,identifier_nom) )
		             in_funcs_list =  TRUE;

	             if  ( args_on_right==2 )
		              do_insert =  TRUE;
	             else if ( args_on_right==1 ) {
	               if ( in_funcs_list )
		               do_insert =  TRUE;
			           else {
			             U16 nom_ln  =  strlen( (char*)identifier_nom );
			             if ( nom_ln > 9 ) {
                      char* ptr =  (char*)identifier_nom + nom_ln - 9;
                      if  ( !strcmp(ptr,"^&minus;1") ) {
		                     do_insert =  TRUE;
                      } else {
			                   if ( identifier_nom[nom_ln-3] == '^' )
			                     if ( identifier_nom[nom_ln-2] == '-' )
			                       if ( identifier_nom[nom_ln-1] == '1' )
		                           do_insert =  TRUE;
                      }
			             }
			           }
		           }
               if ( do_insert && !in_funcs_list ) {
                 funcs_list  =  AddIdentifier( funcs_list, identifier_nom );
		             do_second_pass  =  TRUE;
	             }
		         }	// if ( MMLNodeCouldBeFunction(...) )
		     }
  	  }
    }	// clause for implicit functions

    if ( do_insert ) {
	    InsertAF( mml_rover,is_delimited );
	    mml_rover =  mml_rover->next;	// <mo>ApplyF..
	  }

    mml_rover =  mml_rover->next;
  }   // loop thru level one nodes - pass one

  if ( do_second_pass ) {
    TNODE* mml_rover  =  MML_list;
    while ( mml_rover ) {	// loop thru level one contents nodes
	    U16 args_on_right =  0;
      TCI_BOOL is_delimited =  FALSE;
	    if ( mml_rover->next )
	      if ( mml_rover->next->details )
	        if ( mml_rover->next->details->delimited_group != UNDEFINED_DETAIL ) {
	          args_on_right =  IsGroupAFuncArg( mml_rover->next );
            is_delimited  =  TRUE;
          }
	    if ( args_on_right ) {
	      U8 identifier_nom[80];
        if ( MMLNodeCouldBeFunction(mml_rover,identifier_nom,80) ) {
	        TCI_BOOL in_funcs_list  =  FALSE;
	        if ( IdentifierInList(funcs_list,identifier_nom) )
	          in_funcs_list =  TRUE;
	        if ( args_on_right==1 && in_funcs_list ) {
	          InsertAF( mml_rover,is_delimited );
	          mml_rover =  mml_rover->next;	// <mo>ApplyF..
	        }
	      }
	    }		// if ( args_on_right )
      mml_rover =  mml_rover->next;
    }   // loop thru level one nodes
  }   // second pass
}



// In our initial translation of LaTeX -> MML,
//  InvisibleTimes is NOT scripted.  In the following pass
//  we insert InvisibleTimes into the MML_list that
//  we're building. - no bindings here, just node insertion.

void LaTeX2MMLTree::InsertInvisibleTimes( TNODE* MML_list ) {

  TNODE* mml_rover  =  MML_list;
  while ( mml_rover ) {

	TCI_BOOL is_capitol_letter;
    if ( IsMMLNumber(mml_rover) || IsMMLFactor(mml_rover,TRUE,is_capitol_letter) ) {
      mml_rover =  mml_rover->next;

      TCI_BOOL last_was_cap =  is_capitol_letter;
      while ( mml_rover ) {
        U8* op_nom  =  NULL;
        U8* op_uni  =  NULL;
	    U16 op_precedence  =  0;
        if        ( IsQualifier(mml_rover) ) {	// x{x!=0}
          op_nom  =  (U8*)entity_ic;
          op_uni  =  (U8*)entity_ic_unicode;
	      op_precedence  =  2;
        } else if ( IsMMLFactor(mml_rover,TRUE,is_capitol_letter) ) {

          TCI_BOOL do_it  =  TRUE;
          if ( is_capitol_letter && last_was_cap ) {
            if ( math_field_ID == MF_GEOMETRY )
            do_it  =  FALSE;
          }
          if ( do_it ) {
            op_nom  =  (U8*)entity_it;
            op_uni  =  (U8*)entity_it_unicode;
	        op_precedence  =  39;
          }

        } else if ( IsVector(mml_rover) ) {	// 2(x,y,z)
          op_nom  =  (U8*)entity_it;
          op_uni  =  (U8*)entity_it_unicode;
	      op_precedence  =  39;
        } else if ( IsMMLNumber(mml_rover) ) {
	      if ( mml_rover->prev
	      &&   mml_rover->prev->details
	      &&   mml_rover->prev->details->delimited_group != UNDEFINED_DETAIL ) {
            op_nom  =  (U8*)entity_it;
            op_uni  =  (U8*)entity_it_unicode;
	        op_precedence  =  39;
	      } else if ( mml_rover->prev
		  &&   IsMsup(mml_rover->prev) ) {
            op_nom  =  (U8*)entity_it;
            op_uni  =  (U8*)entity_it_unicode;
	        op_precedence  =  39;
		  } else {

// This is tough call, "n5", "T12345"
// The number may be a factor, or it may qualify a variable as a subscript would.

            op_nom  =  (U8*)entity_it;
            op_uni  =  (U8*)entity_it_unicode;
	        op_precedence  =  39;
		    //TCI_ASSERT(0);
		  }
        } else if ( mml_rover->details
		&&          mml_rover->details->space_width != UNDEFINED_DETAIL ) {
		  I16 width_from_details  =  mml_rover->details->space_width;
          if ( -3 <= width_from_details && width_from_details <= 6  )
	        op_precedence  =  39;
        // we don't insert an operator - just advance in inner loop
		}

        if ( IsDegreeMinSec(mml_rover) ) {
          op_nom  =  (U8*)NULL;
          op_precedence =  0;
        }

		if ( op_nom ) {		// insert op node
          TNODE* mo_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );	// <mo>
          SetChData( mo_node,op_nom,op_uni );
          SetDetailNum( mo_node,DETAILS_form,COPF_INFIX );
          SetDetailNum( mo_node,DETAILS_precedence,op_precedence );
		  TNODE* mml_prev =  mml_rover->prev;
		  mo_node->prev   =  mml_prev;
		  mml_prev->next  =  mo_node;
		  mo_node->next   =  mml_rover;
		  mml_rover->prev =  mo_node;
		}

	    if ( op_precedence == 39 ) {    // Advance in inner loop
          last_was_cap =  is_capitol_letter;
          mml_rover =  mml_rover->next;
        } else				// exit inner loop thru factors in 1 term
		  break;	
	  }
	}		// if ( IsNumber || IsFactor )

  // Advance along list - outer loop

    if ( mml_rover )
      mml_rover =  mml_rover->next;
  }			// while ( mml_rover )

}



TNODE* LaTeX2MMLTree::TranslateTeXDollarMath( TNODE* tex_dollar_node,
     							  		            TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  if ( tex_dollar_node && tex_dollar_node->parts ) {

    TNODE* tex_cont =  tex_dollar_node->parts->contents;
	  if ( tex_cont ) {
	    if ( BucketContainsMath(tex_cont) ) {
        TCI_BOOL do_bindings  =  TRUE;
        U16 tex_nodes_done,error_code;
        mml_rv  =  TranslateMathList( tex_cont,do_bindings,
					    NULL,tex_nodes_done,error_code,out_of_flow_list );
	    } else {
        U16 ilk   =  1010;
        U8* ztext =  NULL;
        U32 off1  =  0;
        U32 off2  =  0;
        RecordAnomaly( ilk,ztext,off1,off2 );
		    return mml_rv;
      }
	}
/*
	if ( !mml_rv ) {
// This fudge was added to stop WebEq from crashing.
//  It can't handle an empty <mrow>, or an <mrow>
//  with only whitespace in it.
      mml_rv  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
      SetChData( mml_rv,(U8*)"???",NULL );
	}
*/

  } else
    TCI_ASSERT(0);

  return mml_rv;
}



TCI_BOOL LaTeX2MMLTree::MMLNodeIsFunction( TNODE* mml_node ) {

  TCI_BOOL rv =  FALSE;
  if ( mml_node->details
  &&   mml_node->details->function_status != UNDEFINED_DETAIL ) {
	  U16 func_stat =  mml_node->details->function_status;
	  if ( func_stat ) {
	    rv    =  ( func_stat <= 5 ) ? TRUE : FALSE;
	  }
  }
  return rv;
}


// f(x), sin 2x, g(f(x)), etc.

TCI_BOOL LaTeX2MMLTree::FunctionHasArg( TNODE* mml_func_node,
      									U16& n_space_nodes,
										U16& n_arg_nodes,
										TCI_BOOL& is_delimited ) {

  TCI_BOOL rv   =  FALSE;

  n_space_nodes =  0;
  n_arg_nodes   =  1;	// assumed, set if otherwise
  is_delimited  =  FALSE;

  TNODE* arg    =  mml_func_node->next;

// First we span any space-like nodes

  TCI_BOOL go_on  =  TRUE;
  I16 white_width =  0;
  while ( arg && go_on ) {
    go_on =  FALSE;
    if ( arg->details && arg->details->space_width != UNDEFINED_DETAIL ) {
      white_width +=  arg->details->space_width;
  // this could be <mspace> or <mtext> or <maligngroup>
      if ( -3 <= white_width && white_width <= 9 ) {
        n_space_nodes++;
        arg =  arg->next;
        go_on =  TRUE;
      }
	}
  }

  if ( arg ) {
// We have a candidate argument

// Delimited argument(s)
	if ( arg->details
	&&   arg->details->delimited_group != UNDEFINED_DETAIL ) {
      GROUP_INFO gi;
      GetGroupInfo( arg,gi );
      if ( MMLDelimitersMatch(gi) ) {
        is_delimited  =  TRUE;
	    rv  =  TRUE;
      }
      if ( do_trig_args && FuncTakesTrigArgs(mml_func_node) ) {
        TCI_BOOL is_cap_letter;
        TNODE* mml_rover  =  arg->next;
        while ( mml_rover ) {
          if ( IsMMLFactor(mml_rover,FALSE,is_cap_letter) ) {
			TCI_BOOL keep_going =  TRUE;
	        if ( mml_rover->details
	        &&   mml_rover->details->function_status != UNDEFINED_DETAIL ) {
			  keep_going =  FALSE;
			}
			if ( keep_going ) {
              n_arg_nodes++;
              mml_rover =  mml_rover->next;
			} else
			  break;
		  } else
			break;
		}
	  }

	} else {
// Un-delimited argument

      if ( do_trig_args && FuncTakesTrigArgs(mml_func_node) ) {
// trig-args, argument may be a product
        TCI_BOOL is_cap_letter;
        if ( IsMMLNumber(arg) || IsMMLFactor(arg,FALSE,is_cap_letter) ) {
          TCI_BOOL after_factor =  TRUE;

	      rv  =  TRUE;
          TNODE* mml_rover  =  arg->next;
          while ( mml_rover ) {

            if ( IsMMLFactor(mml_rover,FALSE,is_cap_letter) ) {
			  TCI_BOOL keep_going =  TRUE;
	          if ( mml_rover->details
	          &&   mml_rover->details->function_status !=UNDEFINED_DETAIL ) {
			    keep_going =  FALSE;
			  }
			  if ( keep_going ) {
                after_factor =  TRUE;
                n_arg_nodes++;
                mml_rover =  mml_rover->next;
			  } else
			    break;

            } else if ( after_factor && IsMMLMultOp(mml_rover) ) {
              after_factor =  FALSE;
              n_arg_nodes++;
              mml_rover =  mml_rover->next;

            } else if ( !after_factor && IsMMLNumber(mml_rover) ) {
              after_factor =  TRUE;
              n_arg_nodes++;
              mml_rover =  mml_rover->next;

			} else
			  break;
		  }


	    } else if ( arg->details
	    &&          arg->details->function_status != UNDEFINED_DETAIL ) {
          rv  =  TRUE;
		} else
		  TCI_ASSERT(0);

	  } else {
// argument is single, un-delimited
	    if ( arg->details ) {
          if      ( arg->details->function_status != UNDEFINED_DETAIL )
            rv  =  TRUE;
          else if ( arg->details->unit_state != UNDEFINED_DETAIL )
            rv  =  ( arg->details->unit_state == 1 ) ? FALSE : TRUE;

          if ( arg->details->is_differential != UNDEFINED_DETAIL ) {
          }
          if ( arg->details->bigop_status != UNDEFINED_DETAIL ) {
            rv  =  ( arg->details->bigop_status == 1 ) ? FALSE : TRUE;
          }
          if ( arg->details->integral_num != UNDEFINED_DETAIL ) {
          }
          if ( arg->details->is_expression != UNDEFINED_DETAIL ) {
            rv  =  TRUE;
          }
          if ( arg->details->is_func_arg != UNDEFINED_DETAIL ) {
            TCI_ASSERT(0);
          }
          if ( arg->details->space_width != UNDEFINED_DETAIL ) {
            TCI_ASSERT(0);
          }

	    } else {	// no details
          TCI_BOOL is_cap_letter;
          if ( IsMMLNumber(arg) || IsMMLFactor(arg,FALSE,is_cap_letter) )
		    rv  =  TRUE;
		}
	  }			// argument is single, un-delimited
	}		// Un-delimited arg clause
  }		// if ( arg )

  return rv;
}



TCI_BOOL LaTeX2MMLTree::FuncTakesTrigArgs( TNODE* mml_func_node ) {

  TCI_BOOL rv =  FALSE;

  char* f_nom =  NULL;
  U16 zln;

  U16 uobj,usub,id;
  GetUids( mml_func_node->zuID,uobj,usub,id );

  if        ( uobj == 3 && usub == 201 && id == 1 ) {	// mi<uID3.201.1>
    f_nom =  (char*)mml_func_node->var_value;
    zln   =  mml_func_node->v_len;

  } else if ( uobj == 5 && id == 2 ) {	// mi<uID3.201.1>
/*
msub<uID5.50.2>!msub!reqELEMENT(5.50.3)
msup<uID5.51.2>!msup!reqELEMENT(5.51.3)
msubsup<uID5.52.2>!msubsup!reqELEMENT(5.52.3)
munder<uID5.53.2>!munder!reqELEMENT(5.53.3)
mover<uID5.54.2>!mover!reqELEMENT(5.54.3)
munderover<uID5.55.2>!munderover!reqELEMENT(5.55.3)
*/
    if ( usub>=50 && usub<=55 ) {
      U8 zuID[16];
      UidsTozuID( 5,usub,3,zuID );
      TNODE* base_bucket  =  FindObject( mml_func_node->parts,
	  							(U8*)zuID,INVALID_LIST_POS );
      if ( base_bucket && base_bucket->contents ) {
        TNODE* f_node =  base_bucket->contents;
        U16 uobj,usub,id;
        GetUids( f_node->zuID,uobj,usub,id );
        if ( uobj == 3 && usub == 201 && id == 1 ) {	// mi<uID3.201.1>
          f_nom =  (char*)f_node->var_value;
          zln   =  f_node->v_len;
		} else
		  TCI_ASSERT(0);
	  } else
	    TCI_ASSERT(0);
	} else
	  TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);


  if ( f_nom ) {
    switch ( zln ) {
      case 2  :
        if      ( !strcmp(f_nom,"lg") )	    // \lg<uID8.2.15>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"ln") )	    // \ln<uID8.2.16>
          rv =  TRUE;
	    break;
      case 3  :
        if      ( !strcmp(f_nom,"cos") )	// \cos<uID8.2.5>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"cot") )	// \cot<uID8.2.7>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"csc") )	// \csc<uID8.2.9>
          rv =  TRUE;
//      else if ( !strcmp(f_nom,"lim") )	// \lim<uID8.4.5>
//        rv =  TRUE;
        else if ( !strcmp(f_nom,"log") )	// \log<uID8.2.17>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"sec") )	// \sec<uID8.2.18>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"sin") )	// \sin<uID8.2.19>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"tan") )	// \tan<uID8.2.21>
          rv =  TRUE;
	    break;
      case 4  :
        if      ( !strcmp(f_nom,"cosh") )	// \cosh<uID8.2.6>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"coth") )	// \coth<uID8.2.8>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"sinh") )	// \sinh<uID8.2.20>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"tanh") )	// \tanh<uID8.2.22>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"sech") )	//
          rv =  TRUE;
        else if ( !strcmp(f_nom,"csch") )	//
          rv =  TRUE;
	    break;
      case 6  :
        if      ( !strcmp(f_nom,"arccos") )	// \arccos<uID8.2.1>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arcsin") )	// \arcsin<uID8.2.2>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arctan") )	// \arctan<uID8.2.3>
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arcsec") )	//
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arccsc") )	//
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arccot") )	//
          rv =  TRUE;
	    break;
      case 7  :
        if      ( !strcmp(f_nom,"arccosh") )	//
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arcsinh") )	//
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arctanh") )	//
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arcsech") )	//
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arccsch") )	//
          rv =  TRUE;
        else if ( !strcmp(f_nom,"arccoth") )	//
          rv =  TRUE;
	    break;
	    default :
	    break;
    }
  }

  return rv;
}


// 
// 
// 

void LaTeX2MMLTree::SetAttrsForFenceMO( TNODE* op_node,
                                            U16 subtype,
                                              TCI_BOOL is_left ) {
/*
fracleft<uID10.6.1>  form="prefix"  fence="true" stretchy="true" lspace="0em" rspace="0em"
fracright<uID10.6.2> form="postfix" fence="true" stretchy="true" lspace="0em" rspace="0em"
fenceleft<uID10.7.1> form="prefix"  fence="true" stretchy="true" symmetric="true"
fenceright<uID10.7.2>form="postfix" fence="true" stretchy="true" symmetric="true"
casesleft<uID10.9.1> form="prefix"  fence="true" stretchy="true" lspace="0em" rspace="0em"
*/
/*
  U8* form_val  =  is_left ? (U8*)"prefix" : (U8*)"postfix";
  SetNodeAttrib( op_node,(U8*)"form",form_val );
  SetNodeAttrib( op_node,(U8*)"fence",(U8*)"true" );
  SetNodeAttrib( op_node,(U8*)"stretchy",(U8*)"true" );
  SetNodeAttrib( op_node,(U8*)"lspace",(U8*)"0em" );
  SetNodeAttrib( op_node,(U8*)"rspace",(U8*)"0em" );
*/

  U16 id  =  is_left ? 1 : 2;

	U8 zuID[32];
  UidsTozuID( 10,subtype,id,(U8*)zuID );

  U8* dest_zname;
  U8* d_template;
  if ( d_mml_grammar->GetGrammarDataFromUID(
					                      zuID,(U8*)"FENCE",
   							                &dest_zname,&d_template) ) {
    if ( d_template && *d_template ) {
      ATTRIB_REC* ar  =  ExtractAttrs( (char*)d_template );
      if ( ar ) {
        MergeMOAttribs( op_node,ar );
        DisposeAttribs( ar );
      }
    } else
      TCI_ASSERT(0);
  }

}


// Translate a run LaTeX TEXT (TEXT bucket) to MathML.
// Example - $ x\text{ a run of TEXT }...$
// mtext<uID3.204.1> is the usual MML output
// NOTE: this function may append to "out_of_flow_list"
//   Callers must handle these objects.

TNODE* LaTeX2MMLTree::TextInMath2MML( TNODE* text_contents,
										TNODE** out_of_flow_list,
										TCI_BOOL no_line_breaks,
										TCI_BOOL is_unit ) {

  TNODE* mml_head =  NULL;	    // head of returned list
  TNODE* mml_tail;

  if ( text_contents ) {

// We may want to set class and/or color attributes.

    char* zcolor  =  is_unit ? zMMLUnitAttrs : zMMLTextAttrs;

	U8 ztext[512];  // buffer to accumulate contents of <mtext>
	U16 tln =  0;

	U8 utext[512];  // buffer to accumulate unicode contents of <mtext>
	U16 uln =  0;

    U16 text_count  =  0;
    U16 space_count =  0;
    I16 space_width =  0;

    TNODE* rover  =  text_contents;
	while ( rover ) {		        // loop thru TEXT nodes
	  TNODE* node_to_insert =  NULL;

      TNODE* save_next  =  NULL;
	  TCI_BOOL use_save_next  =  FALSE;

      U16 uobjtype,stype,uID;
      GetUids( rover->zuID,uobjtype,stype,uID );

	  switch ( uobjtype ) {

        case 3 : {			// a non-ascii symbol in TEXT
          ztext[tln] =  0;
          utext[uln] =  0;
          AppendEntityToBuffer( rover->zuID,ztext,utext );
          tln =  strlen( (char*)ztext );
          uln =  strlen( (char*)utext );

          text_count++;
		}
		break;

        case 4 : {			// \accent{}... in TEXT

		  if ( stype == 2 ) {
            TNODE* t_bucket =  FindObject( rover->parts,
  		  			                (U8*)"4.2.50",INVALID_LIST_POS );
            if ( t_bucket ) {

              char ch =  1;     // invalid value

              if ( t_bucket->contents ) {
	            TNODE* TeX_cont =  t_bucket->contents;
                U16 uobj,usub,id;
                GetUids( TeX_cont->zuID,uobj,usub,id );

                if ( uobj==888 && usub==0 && id==0 ) {
                  char vv =  TeX_cont->var_value[0];
                  if        ( vv>='a' && vv<='z' )  {  // small letter
                    ch  =  vv;
                  } else if ( vv>='A' && vv<='Z' )  {  // capital letter
                    ch  =  vv;
                  } else if ( vv=='\\' ) {
                    TCI_ASSERT(0);
                  } else
                    TCI_ASSERT(0);
                } else if ( uobj==3 && usub==16 ) {
                  if      ( id==9 )
                    ch  =  'i';
                  else if ( id==10 )
                    ch  =  'j';
                  else
                    TCI_ASSERT(0);
                } else
                  TCI_ASSERT(0);

              } else
                ch =  0;

              if ( ch != 1 ) {
                U32 unicode =  TextAccentedCharToUnicode( uID,ch );
                if ( !unicode )
                  unicode =  ch;
                U8 buffer[32];
				CreateUnicodeEntity( buffer,0,unicode );

                U16 zln =  strlen( (char*)buffer );
		        strcpy( (char*)ztext+tln,(char*)buffer );
		        tln +=  zln;

		        strcpy( (char*)utext+uln,(char*)buffer );
		        uln +=  zln;

                text_count++;
              }

            } else          // accent has no bucket
              TCI_ASSERT(0);

		  } else        // Accent in TEXT is not a known TEXT accent
            TCI_ASSERT(0);

		}
		break;

        case 5 : {			// \cmd{}{}... in TEXT
// \QTR{}{}
		  if ( stype == 400 ) {
            TNODE* ilk  =  FindObject( rover->parts,
  		  			                (U8*)"5.400.1",INVALID_LIST_POS );
            U8* run_name  =  ilk->contents->var_value;
            U16 usub_ID   =  GetQTRuSubID( run_name,FALSE );
            TNODE* bucket =  FindObject( rover->parts,
  		  				            (U8*)"5.400.2",INVALID_LIST_POS );
            if ( bucket && bucket->contents ) {
	          TNODE* TeX_cont =  bucket->contents;
              node_to_insert  =  TaggedText2MML( TeX_cont,
   									out_of_flow_list,run_name,usub_ID );
		    } else
		      TCI_ASSERT(0);

// text $x$ more text
	      } else if ( stype == TCMD_DollarMath && uID == 0 ) {
            TNODE* local_oof_list =  NULL;
            TNODE* mml_cont  =  TranslateTeXDollarMath( rover,
                                                &local_oof_list );

            U16 vspace_context  =  2;   // script mpadded
            mml_cont  =  HandleOutOfFlowObjects( mml_cont,
    						&local_oof_list,out_of_flow_list,vspace_context );

            node_to_insert =  MMLlistToMRow( mml_cont );
	        node_to_insert->src_linenum =  rover->src_linenum;

	      } else if ( stype == 409 && uID == 0 ) {	// \FRAME<uID5.409.0>!
            RecordAnomaly( 1002,NULL,rover->src_offset1,
          					          rover->src_offset2 );

          } else if ( stype >= 420 && stype <= 426 ) {	// notes
            RecordAnomaly( 1001,NULL,rover->src_offset1,
        							rover->src_offset2 );

// text \textit{italics} more text
          } else if (  (stype >= 450 && stype <= 460)
          ||            stype == 90 ) { // text runs in MATH
            U16 bucketID  =  (stype==90) ? 1 : 2;
            U8 zuID[16];
            UidsTozuID( 5,stype,bucketID,zuID );
            TNODE* bucket =  FindObject( rover->parts,
  							(U8*)zuID,INVALID_LIST_POS );
	        if ( bucket && bucket->contents )
              node_to_insert  =  TaggedText2MML( bucket->contents,
           										    out_of_flow_list,
              										NULL,stype );
	        else
	          TCI_ASSERT(0);

// text \begin{tabular}...\end{tabular} more text
          } else if ( ( stype == 490 || stype == 491 ) && uID == 0 ) {
            HLinesToBucket( rover,TRUE );
            ColsToList( rover );

            node_to_insert  =  Tabular2MML( rover,out_of_flow_list,stype );

// text \frame{ ... } more text
          } else if ( stype == 12 ) {
            if ( uID==8 ) {
// text \underline{...} more text
// I'm handling contents and dropping the underlining for now.
              TNODE* bucket =  FindObject( rover->parts,
  								        (U8*)"5.14.1",INVALID_LIST_POS );
              if ( bucket && bucket->contents ) {
	            TNODE* TeX_cont =  bucket->contents;
                node_to_insert  =  TaggedText2MML( TeX_cont,
               									  out_of_flow_list,
               									  NULL,90 );

// Added Jan. 17, 2003 - JBM - Found a case where this is needed.
// munder<uID5.53.2>!munder!reqELEMENT(5.53.3)reqELEMENT(5.53.4)

  node_to_insert  =  CreateElemWithBucketAndContents( 5,53,2,3,node_to_insert );
  SetDetailNum( node_to_insert,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );
  TNODE* part1    =  node_to_insert->parts;
  TNODE* mo_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );
  SetChData( mo_node,(U8*)"&UnderBar;",(U8*)"&#x0332;" );
  SetNodeAttrib( mo_node,(U8*)"stretchy",(U8*)"true" );
  TNODE* part2  =  CreateBucketWithContents( 5,53,4,mo_node );
  part1->next   =  part2;
  part2->prev   =  part1;

		      } else
		        TCI_ASSERT(0);

			} else if ( uID==13 || uID==14 ) {
              node_to_insert  =  FBox2MML( rover,out_of_flow_list,uID );
			} else
		      TCI_ASSERT(0);

          } else if ( stype>=550 && stype<=560 && uID==0 ) {
	        use_save_next =  TRUE;
            save_next   =  rover->next;
            HyperObj2MML( out_of_flow_list,rover,stype,
                                    FALSE,&node_to_insert );

// text \label{...} more text
          } else if ( stype==700 && uID==0 ) {
	        use_save_next =  TRUE;
            save_next   =  rover->next;
            *out_of_flow_list  =  MoveNodeToList( *out_of_flow_list,rover );

// \QTSN<uID5.472.0>!\QTSN!REQPARAM(5.472.1,NONLATEX)REQPARAM(5.472.2,TEXT)
          } else if ( stype==472 && uID==0 ) {
	        use_save_next =  TRUE;
            save_next   =  rover->next;
            *out_of_flow_list  =  MoveNodeToList( *out_of_flow_list,rover );

          } else if ( stype==800 && uID==0 ) {
// \rule<uID5.800.0>!\rule!OPTPARAM(5.800.1,DIMEN)
//	REQPARAM(5.800.2,DIMEN)REQPARAM(5.800.3,DIMEN)
            if ( RuleIsPureVSpace(rover) ) {
	          use_save_next =  TRUE;
              save_next   =  rover->next;
              *out_of_flow_list  =  MoveNodeToList( *out_of_flow_list,rover );
			} else
              TCI_ASSERT(0);

          } else if ( stype==37 && uID==0 ) {

	        use_save_next =  TRUE;
            save_next   =  rover->next;

// \TABLE<uID5.37.0>!\TABLE!_COLSANDROWS__EXTALIGN__TWIDTH__VR__HR__TCELLS_

	//  we make an equivalent tree for \begin{array} or \begin{tabular}
            TNODE* table  =  MATRIXtoExternalFormat( rover );
            HLinesToBucket( table,TRUE );
            ColsToList( table );

            U16 uobj,usub,uid;
            GetUids( table->zuID,uobj,usub,uid );
            node_to_insert  =  Tabular2MML( table,out_of_flow_list,usub );

		    TNODE* parent =  rover->sublist_owner;
		    TNODE* left_anchor  =  rover->prev;
		    TNODE* right_anchor =  rover->next;

		    rover->prev  =  NULL;
		    rover->next  =  NULL;
            DisposeTList( rover );

		    if ( parent )
		      parent->contents  =  table;
		    if ( left_anchor ) {
		      left_anchor->next =  table;
		      table->prev =  left_anchor;
		    }
		    if ( right_anchor )	{
		      right_anchor->prev  =  table;
		      table->next =  right_anchor;
		    }


          } else if ( stype==298 && uID==0 ) {

  			U32 unicode =  ExtractUNICODE( rover );
            U8 buffer[32];
            CreateUnicodeEntity( buffer,0,unicode );

            U16 zln =  strlen( (char*)buffer );
			if ( tln + zln < 512 ) {
              strcpy( (char*)ztext+tln,(char*)buffer );
              tln +=  zln;
			}
			if ( uln + zln < 512 ) {
              strcpy( (char*)utext+uln,(char*)buffer );
		      uln +=  zln;
			}
            text_count++;

          } else if ( stype==704 && uID==0 ) {
// \ref<uID5.704.0>!\ref!REQPARAM(5.704.1,NONLATEX)
// Bogus implementation here.
		    strcpy( (char*)ztext+tln,"#ref" );
		    tln +=  4;


		    strcpy( (char*)utext+uln,"#ref" );
		    uln +=  4;


            text_count++;

          } else if ( stype==465 && uID==0 ) {
// \symbol<uID5.465.0>!\symbol!REQPARAM(5.465.1,NONLATEX)
// 156   IJ
// 188   ij
            U16 symb_number =  0;
            TNODE* bucket =  FindObject( rover->parts,(U8*)"5.465.1",
   										              INVALID_LIST_POS );
            if ( bucket && bucket->contents ) {		// locate NONLATEX content
              U8* zNONLATEX =  (U8*)"888.8.0";
              TNODE* cont =  FindObject( bucket->contents,zNONLATEX,
   										              INVALID_LIST_POS );
              if ( cont )
	            symb_number =  atoi( (char*)cont->var_value );
            }

            U8 buffer[32];
            buffer[0] =  0;
            if        ( symb_number == 94 ) {
              buffer[0] =  '^';
              buffer[1] =  0;
            } else if ( symb_number == 126 ) {
              buffer[0] =  '~';
              buffer[1] =  0;
            } else if ( symb_number == 156 )
              CreateUnicodeEntity( buffer,0,306 );
            else if ( symb_number == 188 )
              CreateUnicodeEntity( buffer,0,307 );

            if ( buffer[0] ) {
              U16 zln =  strlen( (char*)buffer );
		      strcpy( (char*)ztext+tln,(char*)buffer );
		      tln +=  zln;

		      strcpy( (char*)utext+uln,(char*)buffer );
		      uln +=  zln;

              text_count++;
            }

          } else if ( stype==466 && uID==0 ) {

// \TEXTsymbol<uID5.466.0>!\TEXTsymbol!REQPARAM(5.466.1,NONLATEX)

            if ( rover->parts && rover->parts->contents ) {
              TNODE* cont =  rover->parts->contents;
              if ( cont->v_len ) {

                ztext[tln] =  0;
                utext[uln] =  0;

                char ch =  cont->var_value[0];
                if        ( ch =='<' ) {
// &lt;<uID3.14.107>infix,26,U0003C,
                  AppendEntityToBuffer( (U8*)"3.14.107",ztext,utext );
                } else if ( ch =='>' ) {
// ><uID3.14.109>infix,26,U0003E,lspace="thickmathspace" rspace="thickmathspace"
                  AppendEntityToBuffer( (U8*)"3.14.109",ztext,utext );
                } else if ( ch =='\\' ) {
                  if        ( cont->v_len == 5 ) {  // \vert
// |<uID3.17.101>multiform,111,U0007C
                    AppendEntityToBuffer( (U8*)"3.17.101",ztext,utext );
                  } else if ( cont->v_len == 10 ){  // \backslash
// &bsol;<uID3.17.95>,U0005C
                    AppendEntityToBuffer( (U8*)"3.17.95",ztext,utext );
			      }
                } else
                  TCI_ASSERT(0);

                tln =  strlen( (char*)ztext );
                uln =  strlen( (char*)utext );

                text_count++;

              } else
                TCI_ASSERT(0);
            }

	      } else		// unhandled structure in TEXT
		    TCI_ASSERT(0);

		}
	    break;

	    case   6  :		// a { group } in TEXT
          if ( stype==1 && uID==0 ) {
// {<uID6.1.0>BUCKET(6.1.2,INHERIT,{,},,)
            if ( rover->parts && rover->parts->contents ) {
              TNODE* TeX_cont =  rover->parts->contents;
		      U8 switch_nom[64];
              U16 switch_tag  =  GetMathRunTagFromTeXSwitch(
											       TeX_cont,switch_nom );
			  if ( switch_tag ) {
                node_to_insert  =  TaggedText2MML( TeX_cont,
								out_of_flow_list,switch_nom,switch_tag );
			  } else {

	// Here we're just dropping the tag.
	// We translate the contents.
                TCI_BOOL no_line_breaks =  FALSE;
		        TCI_BOOL is_unit  =  FALSE;
                node_to_insert  =  TextInMath2MML( TeX_cont,
                       out_of_flow_list,no_line_breaks,is_unit );
              }
            }

          } else
            TCI_ASSERT(0);
	    break;

	    case   9  :  {
          if ( stype==1 ) {         // horizontal space within TEXT
            U8 space_str[32];
            space_str[0]  =  0;

            U8 uni_str[32];
            uni_str[0]  =  0;

		    switch ( uID ) {
			  case  1 :   //  <uID9.1.1>
			  case  2 :   // \ <uID9.1.2>
			  case  3 :   // ~<uID9.1.3>
                AppendEntityToBuffer( (U8*)"9.1.17",space_str,uni_str );
                space_count++;
                space_width +=  9;
			  break;
			  case  4 :   // \quad<uID9.1.4>
                AppendEntityToBuffer( (U8*)"9.1.18",space_str,uni_str );
                space_count++;
                space_width +=  18;
			  break;
			  case  5 :   // \qquad<uID9.1.5>
                AppendEntityToBuffer( (U8*)"9.1.18",space_str,uni_str );
                AppendEntityToBuffer( (U8*)"9.1.18",space_str,uni_str );
                space_count++;
                space_width +=  36;
			  break;
			  case  6 :   // \thinspace<uID9.1.6>
			  case  8 :   // \/<uID9.1.8>
			  case  9 :   // \negthinspace<uID9.1.9>
			  case 10 :   // {}<uID9.1.10>
			  case 11 :   // \noindent<uID9.1.11>
			  break;
// \hspace{GLUE}

              case 12 :   // \hspace<uID9.1.12>!\hspace!REQPARAM(9.1.20,GLUE)
              case 13 : { // \hspace*<uID9.1.13>!\hspace*!REQPARAM(9.1.21,GLUE)
                I16 ems_width =  UNDEFINED_DETAIL;
                U8 hspace_buffer[128];
                hspace_buffer[0]  =  0;
	            U8 bucket_zuID[32];
                UidsTozuID( 9,1,uID+8,(U8*)bucket_zuID );
                TNODE* glue_bucket =  FindObject( rover->parts,
							              (U8*)bucket_zuID,INVALID_LIST_POS );
                GetValueFromGlue( glue_bucket,(U8*)hspace_buffer,
                                    FALSE,ems_width );

                node_to_insert =  MakeTNode( 0L,0L,rover->src_linenum,
                                                (U8*)zmspace );

                U16 zln =  strlen( (char*)hspace_buffer );
                node_to_insert->attrib_list =  MakeATTRIBNode( 
                                    (U8*)"width",ELEM_ATTR_width,
			  					            0,hspace_buffer,zln,0 );
                if ( ems_width != UNDEFINED_DETAIL )
                  SetDetailNum( node_to_insert,DETAILS_space_width,ems_width );
              }
              break;

// Horizontal fills - stretchy - bogus implementation here.
// Need to be passed up to line forming process - not likely
// in my lifetime. <maction> -> MML renderer <-> Browser

              case 14 :   // \hfill<uID9.1.14>
                AppendEntityToBuffer( (U8*)"9.1.18",space_str,uni_str );
                AppendEntityToBuffer( (U8*)"9.1.18",space_str,uni_str );
                space_count++;
                space_width +=  36;
              break;
              case 15 :   // \hrulefill<uID9.1.15>
                strcpy( (char*)space_str,"_______" );
                strcpy( (char*)uni_str,"_______" );
              break;
              case 16 :   // \dotfill<uID9.1.16>
                strcpy( (char*)space_str,"......." );
                strcpy( (char*)uni_str,"......." );
              break;

			  case 17 :   // \@<uID9.1.17>
			  default :
			  break;
		    }

		    if ( space_str[0] ) {
			  U16 zln =  strlen( (char*)space_str );
		      if ( tln + zln < 255 ) {
		        strcpy( (char*)ztext+tln,(char*)space_str );
		        tln +=  zln;

                text_count++;

		        if ( uni_str[0] ) {
			      U16 zln =  strlen( (char*)uni_str );
		          if ( uln + zln < 255 ) {
		            strcpy( (char*)utext+uln,(char*)uni_str );
		            uln +=  zln;
			      }
				}

		      } else
		        TCI_ASSERT(0);
		    }


          } else if ( stype==3 ) {	// vertical space
            save_next  =  rover->next;
	        use_save_next  =  TRUE;
            *out_of_flow_list  =  MoveNodeToList( *out_of_flow_list,rover );

		  } else {
		    TCI_ASSERT(0);
/*
			case    :   // \par<uID9.2.0>

			case    :   // \smallskip<uID9.3.1>
			case    :   // \medskip<uID9.3.2>
			case    :   // \bigskip<uID9.3.3>
			case    :   // \strut<uID9.3.4>
			case    :   // \mathstrut<uID9.3.5>
			case    :   // \vspace<uID9.3.6>!\vspace!REQPARAM(9.3.20,GLUE)
			case    :   // \vspace*<uID9.3.7>!\vspace*!REQPARAM(9.3.21,GLUE)

			case    :   // \indent<uID9.4.0>

			case    :   // \allowbreak<uID9.5.1>  
			case    :   // \nolinebreak<uID9.5.3>!\nolinebreak!OPTPARAM(9.5.20,NONLATEX)

			case    :   // \protect<uID9.8.0>
			case    :   // \hline<uID9.9.0>

			case    :   // \vline<uID9.10.0>
			case    :   // \vfill<uID9.14.0>
*/
		  }

		}
		break;

// comment in TEXT
	    case 777  :  {
// For now, I'm only looking for the case where a unicode
//  is hidden in a %TCIMacro{} - used for some Latin chars.
//  It's better to translate the unicode than attempt to handle
//  the expansion - generally true crap.
 		  U32 unicode;
          U16 advance;
          ProcessTextComment( rover,unicode,advance );
  		  if ( unicode ) {
            U8 buffer[32];
            CreateUnicodeEntity( buffer,0,unicode );
            U16 zln =  strlen( (char*)buffer );
		    strcpy( (char*)ztext+tln,(char*)buffer );
		    tln +=  zln;

		    strcpy( (char*)utext+uln,(char*)buffer );
		    uln +=  zln;

            text_count++;
  // Step over nodes in %TCIMacro{} and its expansion.
            while ( advance > 1 ) {
              rover =  rover->next;
              advance--;
            }
  		  } else {   // if ( unicode )
  // Just step over everything else for now.
  // If it's a TCIMacro, we'll hit (and translate) the expansion next.
  		  }
		}
		break;

// words in TEXT
	    case 888  :  {
		  if ( rover->var_value ) {
            U16 i =  0;
            U16 xln =  rover->v_len;
			while ( i < xln ) {
			  U8 ch =  rover->var_value[i++];
			  if ( ch == ' ' ) {      // space
                if ( i > 0 && i < xln-1  ) {    // interior
		          if ( tln+1 < 255 ) {
			        ztext[tln++]  =  ch;

			        utext[uln++]  =  ch;

                    space_count++;
                    space_width +=  9;
                    text_count++;
		          }
                } else if ( tln + 7 < 255 ) {   // ending space
		          ztext[tln]  =  0;
		          utext[uln]  =  0;
                  AppendEntityToBuffer( (U8*)"9.1.17",ztext,utext );
		          tln =  strlen( (char*)ztext );
		          uln =  strlen( (char*)utext );
                  space_count++;
                  space_width +=  9;
                  text_count++;
		        } else
		          TCI_ASSERT(0);  // very long 888

			  } else {                // non-space
		        if ( tln+1 < 255 ) {
			      ztext[tln++]  =  ch;

			      utext[uln++]  =  ch;

                  text_count++;
		        } else
		          TCI_ASSERT(0);  // very long 888
			  }

		    }	// while loop thru letters

		  } else          // 888 node with no data??
		    TCI_ASSERT(0);
		}
		break;

		default   :
		break;

	  }		// switch on uobjtype


      if ( node_to_insert ) {

	// Append any accummulated ztext to the return list

		if ( tln || uln ) {
	      ztext[tln]    =  0;
	      utext[uln]    =  0;
          TNODE* mtext  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
          SetChData( mtext,ztext,utext );

          if ( zcolor )
//          SetNodeAttrib( mtext,(U8*)"mathcolor",(U8*)zcolor );
            SetMMLAttribs( mtext,(U8*)zcolor );

          if ( text_count == space_count )
            SetDetailNum( mtext,DETAILS_space_width,space_width );
          text_count  =  0;
          space_count =  0;


		  if ( !mml_head )
		    mml_head  =  mtext;
		  else {
		    mml_tail->next  =  mtext;
			mtext->prev =  mml_tail;
		  }
		  mml_tail  =  mtext;
	      tln   =  0;
	      uln   =  0;
		}

	// Append the "node_to_insert" to the return list
		if ( !mml_head )
		  mml_head  =  node_to_insert;
		else {
		  mml_tail->next    =  node_to_insert;
		  node_to_insert->prev  =  mml_tail;
		}
		mml_tail  =  node_to_insert;
	  }

  // advance to next node

      if ( use_save_next )
        rover =  save_next;
	  else
        rover =  rover->next;

	}		// loop thru nodes


// Append any remaining ztext to the return list

	if ( tln || uln ) {
	  ztext[tln]    =  0;
	  utext[uln]    =  0;
      TNODE* mtext  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
      if ( zcolor )
//      SetNodeAttrib( mtext,(U8*)"mathcolor",(U8*)zcolor );
        SetMMLAttribs( mtext,(U8*)zcolor );
      SetChData( mtext,ztext,utext );
      if ( text_count == space_count )
        SetDetailNum( mtext,DETAILS_space_width,space_width );

	  if ( !mml_head )
	    mml_head  =  mtext;
	  else {
	    mml_tail->next  =  mtext;
	    mtext->prev =  mml_tail;
	  }
	}		// if ( tln )

  }		// if ( text_contents )


  return mml_head;
}



// The "unit" concept isn't very well defined in our dialect
//  of LaTeX  - \unit{ft}^{2} - for square feet.  I guess
//  that we only tag the letters(s) that compose the unit name.
// It is not clear what should be done with units in MathML.
//  I'm converting unit names to <mtext> for now.
// Second pass, translating \unit{} to <mi>

TNODE* LaTeX2MMLTree::UnitInMath2MML( TNODE* TeX_unit_node,
									  TNODE** out_of_flow_list,
                                      TCI_BOOL nested,
                                      U16& tex_nodes_done ) {

  TNODE* mml_rv =  NULL;
  tex_nodes_done++;       // count the \unit node

// \unit<uID5.802.0>!\unit!!{!BUCKET(5.802.1,MATH,,,},)!}!
  TNODE* bucket =  FindObject( TeX_unit_node->parts,
						        (U8*)"5.802.1",INVALID_LIST_POS );

  if ( bucket && bucket->contents ) {

	U8 z_unit_mtext[256];
    z_unit_mtext[0] =  0;
	U16 tln =  0;
	U8 z_unit_munic[256];
    z_unit_munic[0] =  0;
	U16 uln =  0;

	TCI_BOOL use_save_next;
    TNODE*   save_next;

    TNODE* rover  =  bucket->contents;
    UnicodesToSymbols( rover );     // Remove any \U{} constructs

	while ( rover && tln < 255 ) {	// loop thru letters in \unit{}
	  use_save_next =  FALSE;

      U16 uobjtype,subtype,uID;
      GetUids( rover->zuID,uobjtype,subtype,uID );
	  switch ( uobjtype ) {
		case  3 : {

		  if        ( subtype == 1 )  {		  // small letter
		    z_unit_mtext[tln++]  =  'a' + uID - 1;
		    z_unit_munic[uln++]  =  'a' + uID - 1;
		  } else if ( subtype == 2 )  {		  // capitol letter
		    z_unit_mtext[tln++]  =  'A' + uID - 1;
		    z_unit_munic[uln++]  =  'A' + uID - 1;
		  } else if ( subtype == 10 ) {       // Greek letter
// &Omega;<uID3.10.43>,U003A9
// &mu;<uID3.10.14>,U003BC
            U8* zuID_symbol =  NULL;
			if        ( uID==43 ) {
              z_unit_mtext[tln] =  0;
              z_unit_munic[uln] =  0;
              AppendEntityToBuffer( rover->zuID,z_unit_mtext+tln,z_unit_munic+uln );
              tln =  strlen( (char*)z_unit_mtext );
              uln =  strlen( (char*)z_unit_munic );
            } else if ( uID==14 ) {
              z_unit_mtext[tln] =  0;
              z_unit_munic[uln] =  0;
        	  strcat( (char*)z_unit_mtext,(char*)"&micro;" );
	          if ( output_entities_as_unicodes )
                strcat( (char*)z_unit_munic,(char*)"&#xb5;" );
              tln =  strlen( (char*)z_unit_mtext );
              uln =  strlen( (char*)z_unit_munic );
            } else
              TCI_ASSERT(0);

		  } else if ( subtype == 16 )	{   // Latin (upper ansi) symbols
// &angst;<uID3.16.14>,U0212B
            z_unit_mtext[tln] =  0;
            z_unit_munic[uln] =  0;
            AppendEntityToBuffer( rover->zuID,z_unit_mtext+tln,z_unit_munic+uln );
            tln =  strlen( (char*)z_unit_mtext );
            uln =  strlen( (char*)z_unit_munic );

		  } else if ( subtype == 17 )	{   // miscellaneous
// &deg;<uID3.17.159>,U000B0
// &prime;<uID3.17.160>,U02032
// &Prime;<uID3.17.161>,U02033
// &deg;C<uID3.17.162>,U02103
// &deg;F<uID3.17.163>,U02109
            z_unit_mtext[tln] =  0;
            z_unit_munic[uln] =  0;
            AppendEntityToBuffer( rover->zuID,z_unit_mtext+tln,z_unit_munic+uln );
            tln =  strlen( (char*)z_unit_mtext );
            uln =  strlen( (char*)z_unit_munic );

		  } else          // un-expected subtype
		    TCI_ASSERT(0);
		}
		break;

		case   5  :		// a structure
	      if        ( subtype==90 && uID==0 ) {
	// \text<uID5.90.0>!\text!REQPARAM(5.90.1,TEXT)
            TNODE* tbucket  =  FindObject( rover->parts,
                                (U8*)"5.90.1",INVALID_LIST_POS );
            if ( tbucket && tbucket->contents ) {
	// \text{\AA}%
TCI_ASSERT(0);
/*
              mml_rv  =  TextInMath2MML( tbucket->contents,
                                out_of_flow_list,FALSE,TRUE );
*/
            } else
              TCI_ASSERT(0);

		  } else if ( subtype==51 && uID==1 ) {		// ^
TCI_ASSERT(0);
/*
			TCI_BOOL is_degree;
            mml_rv  =  ScriptedUnit2MML( rover,is_degree );
			if ( is_degree ) {
//&deg;<uID3.17.159>,U000B0
			  z_unit_mtext[tln] =  0;
			  z_unit_munic[uln] =  0;
              AppendEntityToBuffer( (U8*)"3.17.159",z_unit_mtext,z_unit_munic );
              tln =  strlen( (char*)z_unit_mtext );
              uln =  strlen( (char*)z_unit_munic );
	  		}
*/
		  } else if ( subtype >= 298
		  &&          subtype <= 299
		  &&          uID==0 ) {	// \U<uID5.298.0>!\U!REQPARAM(5.298.2,NONLATEX)
TCI_ASSERT(0);  // Shouldn't get here - unicodes all mapped to symbols earlier
		  } else
		    TCI_ASSERT(0);
		break;

		case   6  :		// a { group }
// {<uID6.1.0>BUCKET(6.1.2,INHERIT,{,},,)
// Here's an example  -  {{}^\circ}%
	      if ( subtype==1 && uID==0 ) {
		    if ( rover->parts && rover->parts->contents )
		      rover =  rover->parts->contents;
		  } else
		    TCI_ASSERT(0);
		break;

		case   9  :		// a {}
	      if ( subtype==1 && uID==10 ) {
			// empty group
		  } else if ( subtype == 3 ) {		// vertical space
	        use_save_next =  TRUE;
            save_next   =  rover->next;
            *out_of_flow_list  =  MoveNodeToList( *out_of_flow_list,rover );
		  } else
		    TCI_ASSERT(0);
		break;

		case 777  :		// a comment
// Might want to handle %TCIMACRO{\U{2126}}% here!
		break;

		default :
          //TCI_ASSERT(0);    // un-expected object in a \unit{}
		break;
	  }

	  if ( use_save_next )
        rover =  save_next;
	  else
        rover =  rover->next;

	}       // loop thru objectss in \unit{}


    if ( mml_rv ) {
TCI_ASSERT(0);

    } else {
	  if ( tln ) {
	    z_unit_mtext[tln] =  0;
	    z_unit_munic[uln] =  0;
        U8 buffer[512];
        buffer[0]   =  0;
        U8 buffer2[512];
        buffer2[0]  =  0;

	    strcat( (char*)buffer,(char*)z_unit_mtext );
	    strcat( (char*)buffer2,(char*)z_unit_munic );
        mml_rv  =  MakeTNode( 0L,0L,0L,(U8*)"3.201.1" );  // <mi>
        SetChData( mml_rv,buffer,buffer2 );
        SetNodeAttrib( mml_rv,(U8*)"class",(U8*)"msi_unit" );
		U16 zln =  strlen( (char*)buffer );
		if ( zln==1 )
          SetNodeAttrib( mml_rv,(U8*)"mathvariant",(U8*)"normal" );
      } else
        TCI_ASSERT(0);

    }

// look for \unit{}^{2}

	if ( mml_rv ) {
      TNODE* TeX_next =  TeX_unit_node->next;
      if ( TeX_next ) {
        U16 uobj,usub,id;
        GetUids( TeX_next->zuID,uobj,usub,id );
        if ( uobj==5 && usub==51 && id==1 ) {
          MATH_CONTEXT_INFO m_context;
          TNODE* mml_psuedo =  Script2PsuedoMML( TeX_next,
								&m_context,out_of_flow_list );
          if ( mml_psuedo && mml_psuedo->next ) {
            TNODE* mml_exp  =  mml_psuedo->next;
            mml_psuedo->next  =  NULL;
            mml_exp->prev =  NULL;
            DisposeTNode( mml_psuedo );

            mml_rv  =  CreateElemWithBucketAndContents( 5,51,2,3,mml_rv );
            TNODE* parts  =  mml_rv->parts;
//msup<uID5.51.2>!msup!reqELEMENT(5.51.3)reqELEMENT(5.51.5)!/msup!
            TNODE* next_part  =  CreateBucketWithContents(5,51,5,mml_exp );
            parts->next =  next_part;
            next_part->prev  =  parts;
          }
          tex_nodes_done++;
        }
      }

      U16 local_nodes_done  =  0;
      mml_rv  =  CoelesceFollowingUnits( TeX_unit_node,out_of_flow_list,
                                            mml_rv,local_nodes_done );
	  tex_nodes_done  +=  local_nodes_done;
	}


    if ( mml_rv ) {
	  if ( !mml_rv->var_value && !nested ) {   // not <mi>
        mml_rv  =  FixImpliedMRow( mml_rv );
        mml_rv  =  CreateElemWithBucketAndContents( 5,600,0,2,mml_rv );
      }

/*
;; Attributes on an <mi> that give a destinct rendering for a unit name
<uID12.1.7>mathcolor="green"

MathML.gmr may specify an attribute/value be set on each node that represents a unit.
If an <mi> represents the unit, we put the attrib directly on the <mi>.
Otherwise, we nest in an <mstyle>, then put the attrib on it.
*/

      if ( zMMLUnitAttrs )
        SetMMLAttribs( mml_rv,(U8*)zMMLUnitAttrs );

      SetDetailNum( mml_rv,DETAILS_unit_state,1 );
      SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );
    }

  } else    // \unit{} has no contents
    TCI_ASSERT(0);


  return mml_rv;
}


// Handle compund units, \unit{} / \unit{} OR \unit{} \unit{}
// mi/h  OR  ft lb, etc.

TNODE* LaTeX2MMLTree::CoelesceFollowingUnits( TNODE* TeX_unit_node,
									        TNODE** out_of_flow_list,
											TNODE* MML_rv,
											U16& tex_nodes_done ) {

  TNODE* rv =  MML_rv;

  TNODE* rover  =  TeX_unit_node->next;
  if ( rover ) {
    TCI_BOOL go_on  =  FALSE;
	U8 join[32];
	join[0] =  0;
    U16 join_nodes_done =  0;

	U8 unicode_join[32];
	unicode_join[0] =  0;

    U16 uobjtype,subtype,uID;
    GetUids( rover->zuID,uobjtype,subtype,uID );

// Step over a TeX ^ node (superscript) that may follow a unit.

    if ( uobjtype==5 && subtype==51 && uID==1 ) {
      rover =  rover->next;
      if ( !rover )
        return rv;
      GetUids( rover->zuID,uobjtype,subtype,uID );
    }

// Slash, /, is taken to mean "per" or "divided by" in this context

	if ( uobjtype==3 && subtype==13 && uID==62 ) {	// "/"
	  strcpy( (char*)join,(char*)"/" );
	  if ( output_entities_as_unicodes )
  	    strcpy( (char*)unicode_join,(char*)"/" );
      go_on =  TRUE;
      join_nodes_done++;
      rover =  rover->next;
	} else if ( uobjtype==5 && subtype==802 ) {     // \unit{}
//&middot;<uID3.13.64>infix,40,U000B7
      AppendEntityToBuffer( (U8*)"3.13.64",join,unicode_join );
      go_on =  TRUE;
	}

// Now look for a second \unit

	if ( go_on && rover ) {

      TNODE* n_join =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );	// <mo>
      SetChData( n_join,join,unicode_join );

	  SetNodeAttrib( n_join,(U8*)"form",(U8*)zop_forms[OPF_infix] );
      SetDetailNum( n_join,DETAILS_form,2 );
      SetDetailNum( n_join,DETAILS_precedence,46 );
      SetDetailNum( n_join,DETAILS_TeX_atom_ilk,TeX_ATOM_BIN );

      rv->next  =  n_join;      
      n_join->prev  =  rv;


      U16 uobj,usub,ID;
      GetUids( rover->zuID,uobj,usub,ID );
	  if ( uobj==5 && usub==802 ) {			// another \unit{}
        U16 local_nodes_done  =  0;
        TNODE* unit2  =  UnitInMath2MML( rover,out_of_flow_list,
                                      		TRUE,local_nodes_done );
        if ( unit2 ) {
          tex_nodes_done  +=  join_nodes_done + local_nodes_done;

          TNODE* tail =  rv;
		  while ( tail->next )
		    tail  =  tail->next;

          tail->next  =  unit2;
          unit2->prev =  tail;

          rv  =  MMLlistToMRow( rv );

        }   // if ( unit2 ) was translateable
	  }   // clause for second \unit{}
	}
  }		// if ( node follows unit )

  return rv;
}


// Note that the input here is an MML list.
//  It may level 1 intermediate MML with some
//   psuedo objects ( ie scripts ).

TCI_BOOL LaTeX2MMLTree::OperandExists( TNODE* mml_op_node,
									    TCI_BOOL on_the_right,
										U16 curr_prec,
  										U16& n_space_nodes,
									    U16& nodes_spanned,
									    I16& space_width ) {

  n_space_nodes   =  0;
  nodes_spanned   =  0;
	space_width =  0;

  TNODE* candidate  =  on_the_right ? mml_op_node->next : mml_op_node->prev;

// OperandExists is (sometimes) called before BindScripts.
// If we're looking to the left, there may be a psuedo script
//  between the current operator and it's operand.

  if ( !on_the_right && candidate && candidate->prev ) {
    TNODE* script =  candidate->prev;
    U16 uobj,usub,id;
    GetUids( script->zuID,uobj,usub,id );
	if ( uobj==3 && usub==201 && id==1 ) {	// mi<uID3.201.1>
	  U8* op_nom  =  script->var_value;
	  if ( !strcmp("TeX^",(char*)op_nom)
	  ||   !strcmp("TeXSb",(char*)op_nom)
	  ||   !strcmp("TeXSp",(char*)op_nom)
	  ||   !strcmp("TeX_",(char*)op_nom) )
	    candidate  =  script->prev;
	}
  }

// Now we span any space-like nodes

  TCI_BOOL go_on  =  TRUE;
  I16 white_width =  0;
  while ( candidate && go_on ) {
    go_on =  FALSE;
    if ( candidate->details 
    &&   candidate->details->space_width != UNDEFINED_DETAIL ) {
      white_width +=  candidate->details->space_width;
  // this could be <mspace> or <mtext> or <maligngroup>
      if ( -3 <= white_width && white_width <= 18 ) {
        n_space_nodes++;
        candidate  =  on_the_right ? candidate->next : candidate->prev;
        go_on =  TRUE;
      }
	}
  }


  if ( candidate ) {
    U16 uobjtype,usubtype,uID;
    GetUids( candidate->zuID,uobjtype,usubtype,uID );

    if ( uobjtype==5 && usubtype==70 && uID==0 ) {
// mfenced<uID5.70.0>!mfenced!_FENCEDITEMS_!/mfenced!
      TCI_ASSERT(0);
      nodes_spanned =  1;	// considered to be an operand?

    } else {	// candidate object not a fence

      U16 uobjtype,usubtype,uID;
      GetUids( candidate->zuID,uobjtype,usubtype,uID );
	  if      ( uobjtype == 3 ) {
        switch ( usubtype ) {
          case 201  :			// mi<uID3.201.1>
          case 202  :			// mn<uID3.202.1>
            nodes_spanned =  1;		// considered to be an operand
	      break;
          case 203  : {		    // mo<uID3.203.1>
            U16 form_flags  =  GetOPFormFlags( candidate );
			if ( on_the_right ) {
			  if ( (form_flags & 4) && candidate->next ) {	// prefix op
                U16 r_spaces,r_operand_nodes;
                I16 local_space_width;
                if ( OperandExists(candidate,on_the_right,curr_prec,
              					r_spaces,r_operand_nodes,local_space_width) )
                  nodes_spanned =  1 + r_spaces + r_operand_nodes;
			  }
			} else {	// looking left
			  if ( (form_flags & 1) && candidate->prev ) {	// postfix op
                U16 l_spaces,l_operand_nodes;
                I16 local_space_width;
                if ( OperandExists(candidate,on_the_right,curr_prec,
              					l_spaces,l_operand_nodes,local_space_width) )
                  nodes_spanned =  1 + l_spaces + l_operand_nodes;
			  }
			}
		  }
		  break;

          case 204  : {		// mtext<uID3.204.1>
    // we stepped over NARROW whitespace before getting here!
            if      ( MTextIsWhiteSpace(candidate) )
              TCI_ASSERT(0);
            else if ( MTextIsWord(candidate) )
              nodes_spanned =  1;		// considered to be an operand
		  }
	      break;

          case 205  : { // mspace<uID3.205.1>
    // Here we are looking at <mspace> that returned FALSE
    //  from IsMMLSpaceLikeNode.  Must be quite wide.
		    TCI_ASSERT(0);
		  }
	      break;
          case 206  :		// ms<uID3.206.1>
		        TCI_ASSERT(0);
		  break;
		  default   :
		  break;
	    }

	  } else if ( uobjtype == 5 ) {

        switch ( usubtype ) {
          case 750  :		// mrow<uID5.750.1>
          case   1  :		// mfrac<uID5.1.0>
          case  35  :		// mtable<uID5.35.0>
          case  50  :		// msub<uID5.50.2>
          case  51  :		// msup<uID5.51.2>
          case  52  :		// msubsup<uID5.52.2>
          case  53  :		// munder<uID5.53.2>
          case  54  :		// mover<uID5.54.2>
          case  55  :		// munderover<uID5.55.2>
          case  56  :		// mprescripts/<uID5.56.10>
//        case  56  :		// mmultiscripts<uID5.56.2>
          case  60  :		// msqrt<uID5.60.0>
          case  61  :		// mroot<uID5.61.0>
          case  70  :	{	// mfenced<uID5.70.0>
            nodes_spanned =  1;
          }
          break;

          case 600  :		// mstyle<uID5.600.0>
           if ( candidate->details ) {
             if ( candidate->details->unit_state==1 )
               nodes_spanned =  1;		// considered to be an operand
             else if ( candidate->details->bigop_status==1 ) {
               if ( candidate->next ) {	// prefix op
                 U16 r_spaces,r_operand_nodes;
                 I16 local_space_width;
                 if ( OperandExists(candidate,on_the_right,curr_prec,
						r_spaces,r_operand_nodes,local_space_width) )
                   nodes_spanned =  1 + r_spaces + r_operand_nodes;
               }
             } else
               TCI_ASSERT(0);
           } else
             TCI_ASSERT(0);
          break;
          case 601  :		// merror<uID5.601.0>
          case 602  :		// mpadded<uID5.602.0>
          case 603  :		// mphantom<uID5.603.0>
		    TCI_ASSERT(0);
		  break;

          case 604  :		// maction<uID5.604.0>
		    if ( candidate->details
		    &&   candidate->details->is_expression==1 ) {
              nodes_spanned =  1;		// considered to be an operand
		    } else
		      TCI_ASSERT(0);
		  break;

		      default   :
		        TCI_ASSERT(0);
		      break;
	      }

	    }		// else if ( uobjtype == 5 )

    }	  // un-delimited operand clause

  }		// if ( candidate )

  return nodes_spanned ? TRUE : FALSE;
}


// Nest a run of contents nodes in the parts bucket of <mrow>.
// Tree manipulate function - a bit tricky.
// Return the head of the modified list.

TNODE* LaTeX2MMLTree::NestNodesInMrow( TNODE* list_head,
                                        TNODE* anchor,TNODE* first,
                                        TCI_BOOL on_the_right,
                                        U16 nodes_spanned,
                                        TNODE** p_new_mrow ) {

  *p_new_mrow =  NULL;
  TNODE* rv =  list_head;

  TNODE* ender  =  anchor ? anchor : first;
  U16 i =  anchor ? 0 : 1;
  while ( i < nodes_spanned ) {
    if ( on_the_right )
      ender =  ender->next;
    else
      ender =  ender->prev;
    i++;
  }

  TCI_BOOL new_list_head  =  FALSE;
  TNODE* parent =  NULL;

  TNODE* left_anchor;
  TNODE* right_anchor;
  TNODE* operand_start;
  TNODE* operand_end;

  if ( on_the_right ) {
    left_anchor   =  anchor ? anchor : first->prev;
    right_anchor  =  ender->next;
    operand_start =  anchor ? anchor->next : first;
    operand_end   =  ender;
    if ( first && first->sublist_owner )
      parent  =  first->sublist_owner;
    if ( first && first == list_head )
      new_list_head  =  TRUE;

  } else {		// looking to the left

    TCI_ASSERT( anchor );
    left_anchor   =  ender->prev;
    right_anchor  =  anchor;
    operand_start =  ender;
    operand_end   =  anchor->prev;
    if ( ender->sublist_owner )
      parent  =  ender->sublist_owner;
    if ( !left_anchor )
      new_list_head  =  TRUE;
  }

// cut out the sublist being nested

  if ( left_anchor )
    left_anchor->next =  NULL;
  if ( right_anchor )
    right_anchor->prev  =  NULL;
  operand_start->prev =  NULL;
  operand_end->next   =  NULL;
  if ( parent ) {
    parent->contents  =  NULL;
    operand_start->sublist_owner  =  NULL;
  }

// nest

  TNODE* mrow =  MMLlistToMRow( operand_start );

  if ( mrow ) {
    *p_new_mrow =  mrow;
    if ( left_anchor ) {
      left_anchor->next =  mrow;
	  mrow->prev  =  left_anchor;
	}
    if ( right_anchor ) {
      right_anchor->prev  =  mrow;
      mrow->next   =  right_anchor;
	}
    if ( parent ) {
      parent->contents  =  mrow;
      mrow->sublist_owner =  parent;
	}
    if ( new_list_head )
      rv =  mrow;

  } else {
    TCI_ASSERT(0);
    if ( left_anchor )
      left_anchor->next =  right_anchor;
    if ( right_anchor )
      right_anchor->prev  =  left_anchor;
  }

  return rv;
}


TCI_BOOL LaTeX2MMLTree::MMLNodeCouldStartGroup( TNODE* mml_node,
                                                U8* nom,
                                                U16& biglmr_size ) {

  TCI_BOOL rv =  FALSE;

  U16 uobjtype,usubtype,uID;
  GetUids( mml_node->zuID,uobjtype,usubtype,uID );
  if ( uobjtype==3 && usubtype==203 && uID== 1 ) {	// mo<uID3.203.1>
    U16 precedence;
	  if ( mml_node->details
	  &&   mml_node->details->form != UNDEFINED_DETAIL ) {
      precedence  =  mml_node->details->precedence;
	  } else {
      precedence =  GetPrecedence( mml_node,OPF_prefix );
	    if ( precedence != 1 )
        precedence =  GetPrecedence( mml_node,OPF_postfix );
    }
	  if ( precedence == 1 ) {
      rv =  TRUE;
      U8* f_nom =  mml_node->var_value;
      strcpy( (char*)nom,(char*)f_nom );
	    if ( mml_node->details
	    &&   mml_node->details->bigl_size != UNDEFINED_DETAIL )
        biglmr_size  =  mml_node->details->bigl_size;
      else
        biglmr_size  =  0;
	  }
  }

  return rv;
}


// Utility to locate the "mate" of a fence delimiter,
//  scanning from left to right.

TCI_BOOL LaTeX2MMLTree::LocateMatchingDelim( TNODE* mml_fence_mo,
	  										                    U8* fence_op_nom,
									    	                    U16 biglmr_size,
									    	                    U16& nodes_spanned ) {

  TCI_BOOL rv   =  FALSE;
  nodes_spanned =  0;		// count includes the 2 delimiters

  U8 matching_delim[32];
  U8 secondary_match[32];	// used in locating intervals like (0,1]
  GetMatchingFence( fence_op_nom,matching_delim,secondary_match );

  U16 nest_level  =  1;		// start at first interior node
  U16 node_count  =  1;

  TNODE* mml_rover  =  mml_fence_mo->next;
  while ( mml_rover ) {
    node_count++;
    U16 uobjtype,usubtype,uID;
    GetUids( mml_rover->zuID,uobjtype,usubtype,uID );
    if ( uobjtype==3 && usubtype==203 && uID==1 ) {	// mo - operator
	    char* op_nom  =  (char*)mml_rover->var_value;
	    if ( op_nom ) {

        U16 curr_size =  0;
	      if ( mml_rover->details )
	        if ( mml_rover->details->bigl_size != UNDEFINED_DETAIL )
            curr_size =  mml_rover->details->bigl_size;

        if ( biglmr_size == curr_size ) {
	        if        ( !strcmp(op_nom,(char*)matching_delim) ) {
		        TCI_ASSERT( nest_level );
		        nest_level--;
		        if ( !nest_level ) {
              nodes_spanned =  node_count;
              rv   =  TRUE;
		          break;
		        }
	        } else if ( !strcmp(op_nom,(char*)secondary_match) ) {
		        if ( nest_level == 1 ) {
              nodes_spanned =  node_count;
              rv   =  TRUE;
		          break;
		        }
	        } else if ( !strcmp(op_nom,(char*)fence_op_nom) )
		        nest_level++;
        }

	    }		// if ( op_nom )
	  }	// <mo> clause

	  mml_rover =  mml_rover->next;
  }		// while ( mml_rover )

  return rv;
}


// Given an mml list generated from a LaTeX contents list,
//  complete the transformation to "weell-formed" MathML.

TNODE* LaTeX2MMLTree::FinishMMLBindings( TNODE* mml_list ) {

  if ( mml_list && !mml_list->next )
    return mml_list;

// First we bind any delimited groups, (...), etc.
  TNODE* rv  =  BindDelimitedGroups( mml_list );

// Locate the base for each script, and form
//  <msub>'s, <msup>'s and <msubsup>'s from psuedo
//  scripts generated during initial translation.
  rv  =  BindScripts( rv );

// Set the form and precedence of all operators
  rv  =  AddOperatorInfo( rv );
// Some <mspace> nodes can be absorbed into the lspace
//  and rspace of adjacent <mo>s.
  rv  =  AbsorbMSpaces( rv );

  if ( math_field_ID == MF_GEOMETRY )
    rv  =  BindGeometryObjs( rv );
  rv  =  BindMixedNumbers( rv );

  rv  =  BindUnits( rv );

  rv  =  BindDegMinSec( rv );

// As soon as differential d has been processed, we handle integrals
  rv  =  BindDelimitedIntegrals( rv );	    // \int ... dx

// Nest operands with their operators
//  according to operator precedences.

 rv  =  BindByOpPrecedence( rv,68,66 );
 InsertApplyFunction( rv );				// 65
 rv  =  BindByOpPrecedence( rv,65,54 );

 rv  =  BindByOpPrecedence( rv,53,40 );
  InsertInvisibleTimes( rv );				// 39
  rv  =  AbsorbMSpaces( rv );
  rv  =  BindByOpPrecedence( rv,39,39 );
  rv  =  BindByOpPrecedence( rv,29,29 );
  InsertInvisibleTimes( rv );				// 39


   rv  =  BindByOpPrecedence( rv,39,28 );	// -> + -
   InsertInvisibleTimes( rv );				// 39
   rv  =  BindByOpPrecedence( rv,27,2 );

  return rv;
}
    

// Contents lists may contain delimited groups.
//  ..(  {  } [ ] || || )..  Here we nest such groups, 
//  and make a recursive call to "FinishMMLBindings"
//  on the contents of each group being nested.

TNODE* LaTeX2MMLTree::BindDelimitedGroups( TNODE* MML_list ) {

  TNODE* rv =  MML_list;

  TNODE* rover  =  MML_list;
  while ( rover ) {
    U8 fence_op_nom[32];
    U16 biglmr_size;
    if ( MMLNodeCouldStartGroup(rover,fence_op_nom,biglmr_size) ) {
      U16 node_count  =  0;			// includes delimiting <mo>'s
      if ( LocateMatchingDelim(rover,fence_op_nom,biglmr_size,node_count) ) {

        U8* stretchy_val  =  biglmr_size ? (U8*)"true" : (U8*)"false";
	// Add attributes to the delimiting <mo> nodes
        SetNodeAttrib( rover,(U8*)"form",(U8*)zop_forms[OPF_prefix] );
        SetNodeAttrib( rover,(U8*)"fence",(U8*)"true" );
        if ( biglmr_size ) {
//        SetBiglSizingAttrs( rover,biglmr_size );
        } else
          SetNodeAttrib( rover,(U8*)"stretchy",stretchy_val );

        TNODE* right_delim  =  rover;
        U16 tally =  1;
        while ( tally < node_count ) {
          right_delim =  right_delim->next;
          tally++;
        }
        SetNodeAttrib( right_delim,(U8*)"form",(U8*)zop_forms[OPF_postfix] );
        SetNodeAttrib( right_delim,(U8*)"fence",(U8*)"true" );
        if ( biglmr_size ) {
//        SetBiglSizingAttrs( right_delim,biglmr_size );
        } else
          SetNodeAttrib( right_delim,(U8*)"stretchy",stretchy_val );

	// Nest the delimiters and their contents in an <mrow>
        TNODE* new_mrow;
        rv  =  NestNodesInMrow( rv,NULL,rover,
			  			    	TRUE,node_count,&new_mrow );
        if ( new_mrow ) {
          rover =  new_mrow;	// for ongoing list iteration

	// We mark the <mrow>, indicating that it contains a group
          SetDetailNum( new_mrow,DETAILS_delimited_group,1 );
          SetDetailNum( new_mrow,DETAILS_TeX_atom_ilk,TeX_ATOM_INNER );

    // Now we set up a recursive call to "FinishMMLBindings"
	//  on the contents list that has been nested.

          TNODE* cont =  new_mrow->parts->contents;
          if ( cont ) {
            U16 n_interior_nodes  =  0;
            TNODE* left_anchor    =  cont;
            TNODE* start          =  left_anchor->next;
            TNODE* right_anchor   =  start;
            while ( right_anchor->next ) {
              right_anchor        =  right_anchor->next;
              n_interior_nodes++;
            }
            if ( n_interior_nodes ) {
              TNODE* ender  =  right_anchor->prev;
              ender->next   =  NULL;
              start->prev   =  NULL;

              TNODE* fixed_interior =  FinishMMLBindings( start );
              if ( fixed_interior ) {
                if ( DoNestFenceBody(fixed_interior) )
                  fixed_interior =  CreateElemWithBucketAndContents( 5,750,1,2,
                  											fixed_interior );
                left_anchor->next   =  fixed_interior;
                fixed_interior->prev  =  left_anchor;
                TNODE* fini =  fixed_interior;
                while ( fini->next )	// locate end of fixed interior
                  fini  =  fini->next;
                fini->next  =  right_anchor;
                right_anchor->prev  =  fini;

              } else {
                left_anchor->next   =  right_anchor;
                right_anchor->prev  =  left_anchor;
              }

            }		// if ( n_interior_nodes )
          }		  // if ( cont )

        } else	// if ( new_mrow )
          TCI_ASSERT(0);
      }	  // if ( LocateMatchingDelim(rover,left_to_right,
    }	// if ( MMLNodeCouldStartGroup(rover,fence_op_nom) )

    rover =  rover->next;
  }		// loop thru contents list

  return rv;
}


// Attach "details" information to an mml TNODE.
//  operator form and precedence, etc.
/*	parser\fltutils.h
define DETAILS_form		1
define DETAILS_precedence	2
define DETAILS_delimited_group   3

typedef struct tagDETAILS {
  U16 form;
  U16 precedence;
  U16 delimited_group;
} DETAILS;
*/

void LaTeX2MMLTree::SetDetailNum( TNODE* mml_tnode,
									U16 field_ID,I16 num ) {

TCI_ASSERT( num != UNDEFINED_DETAIL );
  DETAILS* info =  mml_tnode->details;
  if ( !info ) {
    info =  (DETAILS*)TCI_NEW( char[ sizeof(DETAILS) ] );
    mml_tnode->details  =  info;
    info->form            =  UNDEFINED_DETAIL;
    info->precedence      =  UNDEFINED_DETAIL;
    info->delimited_group =  UNDEFINED_DETAIL;
    info->function_status =  UNDEFINED_DETAIL;
    info->unit_state      =  UNDEFINED_DETAIL;
    info->is_expression   =  UNDEFINED_DETAIL;
    info->is_differential =  UNDEFINED_DETAIL;
    info->bigop_status    =  UNDEFINED_DETAIL;
    info->integral_num    =  UNDEFINED_DETAIL;
    info->is_func_arg     =  UNDEFINED_DETAIL;
    info->space_width     =  UNDEFINED_DETAIL;
    info->style           =  UNDEFINED_DETAIL;
    info->is_geometry     =  UNDEFINED_DETAIL;
    info->is_Greek        =  UNDEFINED_DETAIL;
    info->bigl_size       =  UNDEFINED_DETAIL;
    info->TeX_atom_ilk    =  UNDEFINED_DETAIL;
  }

  if      ( field_ID == DETAILS_form )
	  info->form  =  num;
  else if ( field_ID == DETAILS_precedence )
	  info->precedence      =  num;
  else if ( field_ID == DETAILS_delimited_group )
	  info->delimited_group =  num;
  else if ( field_ID == DETAILS_function_status )
	  info->function_status =  num;
  else if ( field_ID == DETAILS_unit_state )
	  info->unit_state      =  num;
  else if ( field_ID == DETAILS_is_expression )
	  info->is_expression   =  num;
  else if ( field_ID == DETAILS_is_differential )
	  info->is_differential =  num;
  else if ( field_ID == DETAILS_bigop_status )
	  info->bigop_status    =  num;
  else if ( field_ID == DETAILS_integral_num )
	  info->integral_num    =  num;
  else if ( field_ID == DETAILS_is_func_arg )
	  info->is_func_arg     =  num;
  else if ( field_ID == DETAILS_space_width )
	  info->space_width     =  num;
  else if ( field_ID == DETAILS_style )
	  info->style           =  num;
  else if ( field_ID == DETAILS_is_geometry )
	  info->is_geometry     =  num;
  else if ( field_ID == DETAILS_is_Greek )
	  info->is_Greek        =  num;
  else if ( field_ID == DETAILS_bigl_size )
	  info->bigl_size       =  num;
  else if ( field_ID == DETAILS_TeX_atom_ilk )
	  info->TeX_atom_ilk    =  num;
  else
    TCI_ASSERT(0);
}


I16 LaTeX2MMLTree::GetDetailNum( TNODE* mml_tnode,
									                  U16 field_ID ) {

  U16 rv  =  0;
  DETAILS* info =  mml_tnode->details;

  if ( info ) {
    if      ( field_ID == DETAILS_form )
	    rv  =  info->form;
    else if ( field_ID == DETAILS_precedence )
	    rv  =  info->precedence;
    else if ( field_ID == DETAILS_delimited_group )
	    rv  =  info->delimited_group;
    else if ( field_ID == DETAILS_function_status )
	    rv  =  info->function_status;
    else if ( field_ID == DETAILS_unit_state )
	    rv  =  info->unit_state;
    else if ( field_ID == DETAILS_is_expression )
	    rv  =  info->is_expression;
    else if ( field_ID == DETAILS_is_differential )
	    rv  =  info->is_differential;
    else if ( field_ID == DETAILS_bigop_status )
	    rv  =  info->bigop_status;
    else if ( field_ID == DETAILS_integral_num )
	    rv  =  info->integral_num;
    else if ( field_ID == DETAILS_is_func_arg )
	    rv  =  info->is_func_arg;
    else if ( field_ID == DETAILS_space_width )
	    rv  =  info->space_width;
    else if ( field_ID == DETAILS_style )
	    rv  =  info->style;
    else if ( field_ID == DETAILS_is_geometry )
	    rv  =  info->is_geometry;
    else if ( field_ID == DETAILS_is_Greek )
	    rv  =  info->is_Greek;
    else if ( field_ID == DETAILS_bigl_size )
	    rv  =  info->bigl_size;
    else if ( field_ID == DETAILS_TeX_atom_ilk )
	    rv  =  info->TeX_atom_ilk;
    else
      TCI_ASSERT(0);
  }

  return rv;
}


void LaTeX2MMLTree::SetDetailzStr( TNODE* mml_tnode,
									U16 field_ID,U8* zstr ) {

  TCI_ASSERT(0);
/*
  DETAILS* info =  mml_tnode->details;
  if ( !info ) {
    info =  (DETAILS*)TCI_NEW( char[ sizeof(DETAILS) ] );
    mml_tnode->details  =  info;
    info->form          =  0;
    info->precedence    =  0;
    info->delimited_group =  0;
	...
  }

  if ( field_ID == ) {
    U16 zln =  strlen( (char*)zstr );
	if ( zln ) {
      U8* tmp =  (U8*)TCI_NEW( char[ zln+1 ] );
      strcpy( (char*)tmp,(char*)zstr );
	  info->  =  tmp;
    }
  }
*/
}


U16 LaTeX2MMLTree::GetOPFormFlags( TNODE* op_node ) {

  U16 rv  =  0;

  if ( op_node->details
  &&   op_node->details->form != UNDEFINED_DETAIL ) {
    if      ( op_node->details->form == OPF_prefix )
      rv  =  4;
    else if ( op_node->details->form == OPF_infix )
      rv  =  2;
    else if ( op_node->details->form == OPF_postfix )
      rv  =  1;
	else
	  TCI_ASSERT(0);
	return rv;
  }

  U8* mml_opname  =  op_node->var_value;
  if ( !strncmp((char*)mml_opname,"TeX",3) ) {
    TCI_ASSERT(0);
  }

  U16 forms_for_op;
  if ( IsMultiFormOp(mml_opname,forms_for_op) ) {
    TCI_ASSERT(forms_for_op);
    rv  =  forms_for_op;
  } else {
    TCI_ASSERT(0);
  }

  return rv;
}


// f(z), R(R<=5), d(a,b), etc.
// We often need to know what's in a group
//  in order to decide what it represents 
//    1. the argument list of a function	f(x,y)
//    2. an operand						    2(x+1)
//    3. parenthetic info					R(R>0)
//    4. an interval						[0,10)

void LaTeX2MMLTree::GetGroupInfo( TNODE* MML_group_node,
					                  GROUP_INFO& gi ) {

  gi.opening_delim[0]   =  0;
  gi.closing_delim[0]   =  0;
  gi.has_mtext		    =  FALSE;
  gi.is_mod		        =  FALSE;
  gi.lowest_precedence  =  999;
  gi.operator_count     =  0;
  gi.separator_count    =  0;
  gi.n_interior_nodes   =  0;
  gi.n_arrays           =  0;

  TNODE* contents_list  =  NULL;
  U16 uobj,usub,ID;
  GetUids( MML_group_node->zuID,uobj,usub,ID );
  if        ( uobj==5 && usub==750 && ID==1 ) {
// mrow<uID5.750.1>!mrow!BUCKET(5.750.2,MATH,,,/mrow,)!/mrow!
    contents_list =  MML_group_node->parts->contents;
  } else if ( uobj==5 && usub==70  && ID==0 ) {
// mfenced<uID5.70.0>!mfenced!_FENCEDITEMS_!/mfenced!
    TCI_ASSERT(0);
  }

  if ( contents_list ) {
    TNODE* body =  contents_list;
  // Extract the left delimiter - if it exists
    if ( IsFenceMO(body) ) {
      strcpy( (char*)gi.opening_delim,(char*)body->var_value );
      body =  body->next;
    }

    TNODE* right_fence_mo =  body;
    if ( right_fence_mo ) {
      if ( IsFenceMO(right_fence_mo) ) {
        body  =  NULL;
      } else {
        while ( right_fence_mo->next )
          right_fence_mo  =  right_fence_mo->next;
      }
      if ( IsFenceMO(right_fence_mo) )
  // Extract the right delimiter
        strcpy( (char*)gi.closing_delim,(char*)right_fence_mo->var_value );
      else
        right_fence_mo  =  NULL;
    }

    if ( body ) {		// there's an interior
      TCI_BOOL nested_contents  =  FALSE;
      if ( !body->next || body->next == right_fence_mo ) {
	// single node in this group
	// check for ([...]) here.
        if ( body->details
        &&   body->details->delimited_group != UNDEFINED_DETAIL ) {
          gi.n_interior_nodes =  1;
	      return;
	    }
        U16 uobj,usub,uID;
        GetUids( body->zuID,uobj,usub,uID );
        if ( uobj==5 && usub==750 && uID==1 ) 	// mrow
          nested_contents =  TRUE;
	  }
	  if ( nested_contents )
        body  =  body->parts->contents;
	  else if ( right_fence_mo )
        right_fence_mo->prev->next  =  NULL;

      GetGroupInsideInfo( body,gi );
	  if ( !nested_contents && right_fence_mo )
        right_fence_mo->prev->next  =  right_fence_mo;
    }	// if ( body )
  }   // if ( contents_list )
}


void LaTeX2MMLTree::GetGroupInsideInfo( TNODE* MML_cont,GROUP_INFO& gi ) {

// Iterate MML_list to see what's in this delimited group.

  TNODE* rover  =  MML_cont;
  while ( rover ) {
    gi.n_interior_nodes++;

    U16 uobjtype,usubtype,uID;
    GetUids( rover->zuID,uobjtype,usubtype,uID );

    switch ( uobjtype ) {
      case  3 :	{

        if ( usubtype==203 && uID==1 ) {	// mo - operator
          gi.operator_count++;

          if ( rover->var_value	&& rover->v_len == 1 )
            if ( rover->var_value[0] == ',' || rover->var_value[0] == ';' )
              gi.separator_count++;

          if ( rover->details
          &&   rover->details->precedence != UNDEFINED_DETAIL )
            if ( rover->details->precedence < gi.lowest_precedence )
              gi.lowest_precedence =  rover->details->precedence;

	        if ( gi.n_interior_nodes==1 && gi.operator_count == 1 )
		      if ( rover->var_value )
		        if ( !strcmp((char*)rover->var_value,"mod") )
                gi.is_mod =  TRUE;

        } else if ( usubtype==201 && uID==1 ) {	// mi<uID3.201.1>
          if ( gi.n_interior_nodes==1 )
            if ( rover->var_value )
              if ( !strcmp((char*)rover->var_value,"mod") )
                gi.is_mod =  TRUE;

        } else if ( usubtype==204 && uID==1 ) {	// mtext<uID3.204.1>
          gi.has_mtext =  TRUE;
	    }
	  }
	  break;

	  case  5 : {
        if        ( usubtype==750 && uID==1 ) {	// mrow
// mrow<uID5.750.1>!mrow!BUCKET(5.750.2,MATH,,,/mrow,)!/mrow!

        } else if ( usubtype >= 35 && usubtype <= 39 ) {	// \begin{array}
          gi.n_arrays++;
        } else if ( usubtype >= 121 && usubtype <= 143 ) {	// \begin{array}
          gi.n_arrays++;
        } else if ( usubtype >= 710 && usubtype <= 716 ) {	// \begin{array}
          gi.n_arrays++;
        } else {		// clause for mrow in our list
	    }
      }
      break;

      default :
      break;
    }

    rover =  rover->next;
  }		// first level loop thru node list

}



TCI_BOOL LaTeX2MMLTree::TeXNodeDenotesFunction( TNODE* TeX_node ) {

  TCI_BOOL rv =  FALSE;

  if ( TeX_node->src_tok ) {
	char ch =  TeX_node->src_tok[0];
	TCI_ASSERT(0);
  }		  // if ( mml_node->var_value )

  return rv;
}


// We handle fences, and operator precedence within fences,
//  in special case code.  If there are any operators inside
//  the fence, they are higher precedence than the delimiting
//  fence operators.  In this case we must nest the interior.

TCI_BOOL LaTeX2MMLTree::DoNestFenceBody( TNODE* fixed_interior ) {

  U16 node_count      =  0;
  U16 operator_count  =  0;
  TNODE* rover  =  fixed_interior;
  while ( rover ) {
    node_count++;
    U16 uobj,usub,ID;
    GetUids( rover->zuID,uobj,usub,ID );
    if      ( uobj==3 && usub==203 && ID==1 ) 	// mo - operator
      operator_count++;
    else if ( rover->details
    &&        rover->details->precedence != UNDEFINED_DETAIL )
      operator_count++;
    rover =  rover->next;
  }		// loop

  if ( operator_count && node_count > 1 )
    return TRUE;
  else
    return FALSE;
}


TCI_BOOL LaTeX2MMLTree::LeftGroupHasSamePrec( TNODE* mml_op_node,
												                        U16 curr_prec,
												                        U16 l_space_nodes ) {

  TCI_BOOL rv   =  FALSE;

  U16 node_count    =  0;
  U16 op_count      =  0;
  U16 prec_op_count =  0;

  TNODE* lefty  =  mml_op_node->prev;
  U16 tally =  l_space_nodes;
  while ( tally && lefty ) {
    lefty =  lefty->prev;
    tally--;
  }

  if ( lefty ) {
    U16 uobj,usub,ID;
    GetUids( lefty->zuID,uobj,usub,ID );
    if ( uobj==5 && usub==750 && ID==1 ) {	// mrow
	    if ( lefty->parts && lefty->parts->contents ) {
        TNODE* rover  =  lefty->parts->contents;
        while ( rover ) {
          node_count++;
          U16 uobj,usub,ID;
          GetUids( rover->zuID,uobj,usub,ID );
          if ( rover->details
          &&   rover->details->precedence != UNDEFINED_DETAIL ) {
              op_count++;
		        if ( rover->details->precedence == curr_prec )
			      if ( rover->details->form == 2
			      ||   rover->details->form == 3 )
			        prec_op_count++;
          }

          rover =  rover->next;
        }
      }
    }		// mrow clause
  }		// if ( lefty )

  if ( op_count )
    if ( op_count == prec_op_count )
//  if ( 2*op_count + 1 == node_count )
      rv  =  TRUE;

  return rv;
}


// 1+2+3+4=10
// handle first  '+' -> (1+2)+3+4=10
// handle second '+' -> ((1+2)+3)+4=10  Elevate -> (1+2+3)+4=10
// handle third  '+' -> ((1+2+3)+4)=10  Elevate -> (1+2+3+4)=10

TNODE* LaTeX2MMLTree::ElevateLeftOperand( TNODE* list_head,
										  TNODE* mml_op_node,
										  U16 l_space_nodes ) {

  TNODE* rv   =  list_head;

  TNODE* right_anchor =  mml_op_node;

  TNODE* mrow =  mml_op_node->prev;
  U16 tally =  l_space_nodes;	// skip over space-like nodes
  while ( tally && mrow ) {
    mrow  =  mrow->prev;
    right_anchor =  right_anchor->prev;
    tally--;
  }

  TNODE* left_anchor  =  mrow->prev;
  TNODE* sub_head     =  mrow->parts->contents;
  TNODE* sub_tail     =  sub_head;
  while ( sub_tail->next )
    sub_tail  =  sub_tail->next;

  TNODE* parent =  NULL;
  if ( mrow->sublist_owner ) {
    parent  =  mrow->sublist_owner;
  }

// Isolate and dispose
  mrow->prev  =  NULL;
  mrow->next  =  NULL;
  mrow->parts->contents =  NULL;
  DisposeTNode( mrow );


  TCI_BOOL new_list_head  =  FALSE;
  if ( !left_anchor ) {
    new_list_head  =  TRUE;
  }

// re-join
  if ( sub_head ) {
    if ( left_anchor ) {
      left_anchor->next =  sub_head;
	  sub_head->prev    =  left_anchor;
	}
    right_anchor->prev  =  sub_tail;
    sub_tail->next      =  right_anchor;
    if ( parent ) {
      parent->contents  =  sub_head;
      sub_head->sublist_owner =  parent;
	} else
      sub_head->sublist_owner =  NULL;
    if ( new_list_head )
      rv =  sub_head;
  } else			// if ( sub_head )
	TCI_ASSERT(0);

  return rv;
}


// When we encounter 'd' in a Math run, we must decide whether
//  or not it represents &Differentiald;
// If it follows an integral, it's probably &dd;

TCI_BOOL LaTeX2MMLTree::LookBackForIntegral( TNODE* tex_sym_node ) {

  TCI_BOOL rv =  FALSE;

  U16 level =  1;
  TNODE* rover  =  tex_sym_node;
  while ( rover ) {
    U16 uobj,usub,ID;
    GetUids( rover->zuID,uobj,usub,ID );
	if ( uobj==7 && usub==1 && ID>=1 && ID<=6 ) {
// \int<uID7.1.1,a>!\int!_LIMPLACE__FIRSTLIM__SECONDLIM_
// ...
// \doint<uID7.1.6,l>!\doint!_LIMPLACE__FIRSTLIM__SECONDLIM_
      rv  =  TRUE;
	  break;
	}

// &dd; may occur in a first level fraction after an integral

    if ( level == 1 && !rover->prev ) {
	  TNODE* parts  =  NULL;
	  if ( rover->sublist_owner ) {		// a parts list
	    parts =  rover->sublist_owner;
		while ( parts->prev )		// locate head of parts list
		  parts =  parts->prev;
	  }
	  if ( parts && parts->sublist_owner ) {
	    TNODE* frac =  parts->sublist_owner;
        U16 uobj,usub,ID;
        GetUids( frac->zuID,uobj,usub,ID );
	    if ( uobj==5 && ID==0 && usub>=1 && usub<=3 ) {
          level =  2;
		  rover =  frac;	// shift ongoing iteration up
		}
	  }
	}

    rover =  rover->prev;
  }		// loop back thru math run to find integral

  return rv;
}


TCI_BOOL LaTeX2MMLTree::IsFenceMO( TNODE* mml_node ) {

  TCI_BOOL rv =  FALSE;

  U16 uobj,usub,uID;
  GetUids( mml_node->zuID,uobj,usub,uID );
  if ( uobj==3 && usub==203 && uID==1 ) {	// <mo>
    ATTRIB_REC* arover  =  mml_node->attrib_list;
    while ( arover ) {
      if ( strcmp((char*)arover->attr_nom,"fence") )
        arover  =  arover->next;
      else {
        if ( !strcmp("true",(char*)arover->z_val) )
          rv  =  TRUE;
        break;
      }
    }		// loop thru attribs
  }		// <mo> clause

  return rv;
}


/*
TEXT ONLY

121 eqnarray 	  - list of lines C & C & C \\	5.121.9,5.121.11,5.121.4
122 eqnarray*	  - 

\begin{align}<uID5.123.0>!\begin{align}!_ALIGNLINES_!\end{align}!

\begin{alignat}<uID5.125.0>!\begin{alignat}!REQPARAM(5.121.8,NONLATEX)_ALIGNATLINES_!\end{alignat}!
\begin{alignat*}<uID5.126.0>!\begin{alignat*}!REQPARAM(5.121.8,NONLATEX)_ALIGNATSLINES_!\end{alignat*}!

\begin{xalignat}<uID5.127.0>!\begin{xalignat}!REQPARAM(5.121.8,NONLATEX)_XALIGNATLINES_!\end{xalignat}!
\begin{xalignat*}<uID5.128.0>!\begin{xalignat*}!REQPARAM(5.121.8,NONLATEX)_XALIGNATSLINES_!\end{xalignat*}!

\begin{xxalignat}<uID5.129.0>!\begin{xxalignat}!REQPARAM(5.121.8,NONLATEX)_XXALIGNATLINES_!\end{xxalignat}!
\begin{xxalignat*}<uID5.130.0>!\begin{xxalignat*}!REQPARAM(5.121.8,NONLATEX)_XXALIGNATSLINES_!\end{xxalignat*}!

\begin{gather}<uID5.131.0>!\begin{gather}!_GATHERLINES_!\end{gather}!
\begin{gather*}<uID5.132.0>!\begin{gather*}!_GATHERSLINES_!\end{gather*}!

\begin{multline}<uID5.133.0>!\begin{multline}!_MULTLINELINES_!\end{multline}!
\begin{multline*}<uID5.134.0>!\begin{multline*}!_MULTLINESLINES_!\end{multline*}!


TEXT and MATH

124 align*    - list of lines   C & C \\	5.121.9,5.121.11,5.121.4
                                ?5.330.1\intertext{}?
                                  & C \\

MATH only

140 cases     - { list of lines C & C \\	5.121.9,5.121.11,5.121.4

141 split     - list of lines   C & C \\	5.121.9,5.121.11,5.121.4
                                ?5.330.1\intertext{}?
                                  & C \\

142 gathered  - [5.121.1,MEXTALIGN]			5.121.9,5.121.11,5.121.4
			  - list of lines 	C \\

143 aligned   - [5.121.1,MEXTALIGN]			5.121.9,5.121.11,5.121.4
			  - list of lines 	C & C \\
                                ?5.330.1\intertext{}?
                                C & C \\
*/

void LaTeX2MMLTree::AddNumbering( TNODE* mrow,
									                TNODE* xml,
									                TCI_BOOL is_eqnarray ) {


  if ( xml && mrow ) {
    if ( is_eqnarray ) {
	    TNODE* x_rover  =  NULL;
      TNODE* x_eqnarray =  FindObject( xml,(U8*)"5.121.0",INVALID_LIST_POS );
	    if ( x_eqnarray && x_eqnarray->parts && x_eqnarray->parts->parts )
	      x_rover  =  x_eqnarray->parts->parts;

      U16 uobj,usub,ID;
      GetUids( mrow->zuID,uobj,usub,ID );
	    if ( uobj==5 && usub==750 && ID==1 ) {
	      mrow  =  mrow->parts->contents;
	    } else {
	      TCI_ASSERT(0);
      }

	    TNODE* m_rover  =  NULL;
      TNODE* m_table  =  FindObject( mrow,(U8*)"5.35.0",INVALID_LIST_POS );
	    if ( m_table && m_table->parts && m_table->parts->parts )
	      m_rover  =  m_table->parts->parts;

	    U16 entry_num =  0;
	    while ( x_rover && m_rover ) {
		    TranscribeAttribs( x_rover,m_rover,entry_num );
	      entry_num++;
		    x_rover =  x_rover->next;
		    m_rover =  m_rover->next;
	    }

	  } else {
	  }
  }

}


// MathML 2.0 supports line numbering for lines in an mtable.

void LaTeX2MMLTree::TranscribeAttribs( TNODE* xml_line,
									    TNODE* mml_line,
									    U16 entry_num ) {

// counter="equation" ordinal="1" label="wave" is-tagged="true"
//  no_number="true"

  if ( mml_version >= 200 ) {

// Get the XML attributes associated with the current line.

    U8 *zcounter,*zordinal,*zlabel,*zis_tagged,*zno_number;
	GetXMLAttrValues( xml_line->attrib_list,&zcounter,
            		&zordinal,&zlabel,&zis_tagged,&zno_number );

	if ( !zordinal ) return;
	if ( zno_number && zno_number[0] == 't' ) return;

/*
mlabeledtr<uID5.35.40>!mlabeledtr!_EQNNUMBER__LABELEDLIST_!/mlabeledtr!
_EQNNUMBER_reqELEMENT(5.35.42)
_LABELEDLIST_LIST(5.35.11,_TDATA_,5.35.12,,/mtr|/mtable,)
_TDATA_IF(MATH,?mtd?,5.35.14)ifEND
*/

  //  <mtr> --> <mlabeledtr>
    TNODE* mtr  =  mml_line->parts;
    strcpy( (char*)mtr->zuID,"5.35.40" );		// mlabeledtr

    TNODE* eqn_tag  =  NULL;
    if ( zordinal ) {
  // Generate the number for this line
      U8 ztext[80];
      ztext[0]  =  '(';
	  strcpy( (char*)(ztext+1),(char*)zordinal );
	  strcat( (char*)ztext,")" );
  // Put the number in an mtext node.
      eqn_tag  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );	// <mtext>
      SetChData( eqn_tag,ztext,NULL );

    } else if ( zis_tagged && zis_tagged[0]=='t' ) {
	  TCI_ASSERT(0);	// not implemented yet

    } else
	  TCI_ASSERT(0);

  // Put the mtext node in the parts bucket of <mlabeledtr>
    TNODE* num_bucket =  CreateBucketWithContents(5,35,42,eqn_tag );

  // Insert num_bucket at head of parts list of <mlabeledtr>.
    TNODE* mtd  =  mtr->parts;

    num_bucket->next  =  mtd;
    mtd->prev   =  num_bucket;
   
    mtr->parts  =  num_bucket;
    num_bucket->sublist_owner =  mtr;

    mtd->sublist_owner =  NULL;

    if ( *zlabel )
      SetNodeAttrib( mtr,(U8*)"id",(U8*)zlabel );
  }

}


// Given the "attrib_list" from a TNODE that represents
//  one entry from the "list-of-lines" of an XML eqnarray,
//  we lookup the attributes that describe labeling
//  and line numbering.

void LaTeX2MMLTree::GetXMLAttrValues( ATTRIB_REC* attrib_list,
           						        U8** zcounter,U8** zordinal,
           						        U8** zlabel,U8** zis_tagged,
            							U8** zno_number ) {

  U8* attr_val;
  *zcounter  =  NULL;
  if ( LocateAttribVal(attrib_list,(U8*)"counter",&attr_val) )
    *zcounter  =  attr_val;
  *zordinal    =  NULL;
  if ( LocateAttribVal(attrib_list,(U8*)"ordinal",&attr_val) )
    *zordinal  =  attr_val;
  *zlabel      =  NULL;
  if ( LocateAttribVal(attrib_list,(U8*)"label",&attr_val) )
    *zlabel    =  attr_val;
  *zis_tagged  =  NULL;
  if ( LocateAttribVal(attrib_list,(U8*)"is-tagged",&attr_val) )
    *zis_tagged  =  attr_val;
  *zno_number    =  NULL;
  if ( LocateAttribVal(attrib_list,(U8*)"no-number",&attr_val) )
    *zno_number  =  attr_val;
}


// Get the value of an attribute from a list of ATTRIB_RECs.

TCI_BOOL LaTeX2MMLTree::LocateAttribVal( ATTRIB_REC* attrib_list,
											U8* targ_attr_name,
											U8** zvalue ) {

  TCI_BOOL rv =  FALSE;

  ATTRIB_REC* arover  =  attrib_list;
  while ( arover ) {
    if ( !strcmp((char*)arover->attr_nom,(char*)targ_attr_name) ) {
      *zvalue =  arover->z_val;
      rv  =  TRUE;
      break;
    }
    arover  =  arover->next;
  }		// loop thru list of ATTIB_RECs

  return rv;
}


// This is a special translation - VERY POOR!
// LaTeX markups for "degree" and "prime(s)" use superscripts.

TNODE* LaTeX2MMLTree::ScriptedUnit2MML( TNODE* script_node,
											  TCI_BOOL& is_degree ) {

  is_degree =  FALSE;
  TNODE* rv =  NULL;

  if ( script_node->parts && script_node->parts->contents ) {
    U16 n_primes  =  0;
	U16 degrees   =  0;

// Iterate the contents of the superscript

	TNODE* s_rover  =  script_node->parts->contents;
	while ( s_rover ) {
      U16 uobj,usub,id;
      GetUids( s_rover->zuID,uobj,usub,id );
	  if      ( uobj==3 && usub==17 && id==23 )
	    n_primes++;
 	  else if ( uobj==3 && usub==13 && id==10 )
	    degrees++;
	  else
	   TCI_ASSERT(0);	// unknown token in superscripted unit

	  s_rover =  s_rover->next;
    }

	if ( degrees == 1 )
      is_degree =  TRUE;
	if ( n_primes )
      rv  =  PrimeAsScript2MML( n_primes );
  }

  return rv;
}


TNODE* LaTeX2MMLTree::PrimeAsScript2MML( U16 n_primes ) {

  TNODE* rv =  NULL;

// The LaTeX scripted \prime algorithm is used here.
// {}^{\prime}

  U8 ztext[256];
  ztext[0]  =  0;

  U8 utext[256];
  utext[0]  =  0;

  U16 i =  0;
  while ( i<n_primes && i<36 ) {
    AppendEntityToBuffer( (U8*)"3.17.160",ztext,utext );
	i++;
  }

  TNODE* mtext  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
  SetChData( mtext,ztext,utext );
  if ( zMMLUnitAttrs )
    SetMMLAttribs( mtext,(U8*)zMMLUnitAttrs );

  TNODE* empty_mtext  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
  U8 buffer[32];
  buffer[0] =  0;
  U8 uni_buffer[32];
  uni_buffer[0] =  0;
  AppendEntityToBuffer( (U8*)"9.1.10",buffer,uni_buffer );
  SetChData( empty_mtext,(U8*)buffer,(U8*)uni_buffer );

// msup<uID5.51.2>!msup!reqELEMENT(5.51.3)_SUPERSCRIPT_!/msup!
// _SUPERSCRIPT_reqELEMENT(5.51.5)

  rv   =  CreateElemWithBucketAndContents( 5,51,2,3,empty_mtext );
  TNODE* sup  =  CreateBucketWithContents( 5,51,5,mtext );

  TNODE* part1  =  rv->parts;
  part1->next   =  sup;
  sup->prev     =  part1;

  return rv;
}


// $ {\bf...}$
// \mathcal   <uID5.80.0>
// \mathrm    <uID5.81.0>		-	May really be \textrm{}
// \mathbf    <uID5.82.0>		-   Probably real tagged MATH
// \mathsf    <uID5.83.0>		-	screwed up by SWP!!
// \mathtt    <uID5.84.0>
// \mathnormal<uID5.85.0>
// \mathit    <uID5.86.0>
// \mathbb    <uID5.87.0> OR \Bbb<uID5.87.0>
// \mathfrak  <uID5.88.0> OR \frak<uID5.88.0>
// \pmb       <uID5.89.0>
// \QTR{}{}   <uID5.400.1> 	-  could be anything!!
// \emph      <uID5.459.1> in MATH

/* values of the mathvariant attribute
normal | bold | italic | bold-italic | double-struck |
bold-fraktur | script | bold-script | fraktur | sans-serif |
bold-sans-serif | sans-serif-italic | sans-serif-bold-italic |
monospace
*/

TNODE* LaTeX2MMLTree::TaggedMath2MML( TNODE* TeX_tag_node,
									    TNODE* TeX_cont,
									    U16 tag_ilk,U8* tag_nom,
									    TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  if ( TeX_cont ) {		// there is something to tag

    U8  attr_str[512];
    attr_str[0] =  0;
    U8* attr_nom2   =  NULL;
    U8* attr_val2   =  NULL;

    TCI_BOOL is_text  =  FALSE;
    TCI_BOOL in_mml_grammar  =  FALSE;

// Get the tag-specific data required to tag our mml.

    U8*	entity_suffix =  NULL;
    U8* mathvariant =  NULL;
    switch ( tag_ilk ) {
// \mathcal<uID5.80.0>!\mathcal!REQPARAM(5.80.1,MATH)
      case 80 :
        entity_suffix   =  (U8*)"scr";
        mathvariant =  (U8*)"script";
	  break;
// \mathrm<uID5.81.0>!\mathrm!REQPARAM(5.81.1,MATH)
      case 81 :
        in_mml_grammar  =  TRUE;
	  break;
// \mathbf<uID5.82.0>!\mathbf!REQPARAM(5.82.1,MATH)
      case TR_Mbf :
        in_mml_grammar  =  TRUE;
        mathvariant =  (U8*)"bold";
	  break;
// \mathsf<uID5.83.0>!\mathsf!REQPARAM(5.83.1,MATH)
      case 83 :
        in_mml_grammar  =  TRUE;
        mathvariant =  (U8*)"sans-serif";
	  break;
// \mathtt<uID5.84.0>!\mathtt!REQPARAM(5.84.1,MATH)
      case 84 :
        in_mml_grammar  =  TRUE;
        mathvariant =  (U8*)"monospace";
	  break;
// \mathnormal<uID5.85.0>!\mathnormal!REQPARAM(5.85.1,MATH)
      case 85 :
        in_mml_grammar  =  TRUE;
        mathvariant =  (U8*)"normal";
	  break;
// \mathit<uID5.86.0>!\mathit!REQPARAM(5.86.1,MATH)
      case 86 :
        in_mml_grammar  =  TRUE;
        mathvariant =  (U8*)"italic";
	  break;
// \mathbb<uID5.87.0>!\mathbb!REQPARAM(5.87.1,MATH)
      case  87  :
        entity_suffix   =  (U8*)"opf";
        mathvariant =  (U8*)"double-struck";
      break;
// \mathfrak<uID5.88.0>!\mathfrak!REQPARAM(5.88.1,MATH)
      case  88  :
		entity_suffix   =  (U8*)"fr";
        mathvariant =  (U8*)"fraktur";
      break;
// \pmb<uID5.89.0>!\pmb!REQPARAM(5.89.1,MATH)
      case  89  :
        in_mml_grammar  =  TRUE;
        TCI_ASSERT(0);
      break;
      case 91 :
      case 96 :
        in_mml_grammar  =  TRUE;
        mathvariant =  (U8*)"bold-italic";
	  break;

// TEXT
      case  TR_Tup      :
      case  TR_Tit		:
      case  TR_Tsl      :
      case  TR_Tsc      :
      case  TR_Tmd		:
      case  TR_Tbf		:
      case  TR_Trm      :
      case  TR_Tsf      :
      case  TR_Ttt      :
      case  TR_em		:
      case  TR_Tnormal  :
      case  TR_Tcircled :
        is_text =  TRUE;
        in_mml_grammar  =  TRUE;
      break;

// Style switches - resolve to size

      case  TR_displaystyle  	  :
        strcpy( (char*)attr_str,"displaystyle=\"true\"" );
        attr_nom2 =  (U8*)"scriptlevel";
        attr_val2 =  (U8*)"0";
      break;
      case  TR_textstyle     	  :
        strcpy( (char*)attr_str,"displaystyle=\"false\"" );
        attr_nom2 =  (U8*)"scriptlevel";
        attr_val2 =  (U8*)"0";
      break;
      case  TR_scriptstyle  	  :
        strcpy( (char*)attr_str,"displaystyle=\"false\"" );
        attr_nom2 =  (U8*)"scriptlevel";
        attr_val2 =  (U8*)"1";
      break;
      case  TR_scriptscriptstyle  :
        strcpy( (char*)attr_str,"displaystyle=\"false\"" );
        attr_nom2 =  (U8*)"scriptlevel";
        attr_val2 =  (U8*)"2";
      break;

      case  TR_group  :
      break;

// Size switches

      case TR_tiny          :   // \tiny<uID5.480.0>
      case TR_scriptsize    :
      case TR_footnotesize  :
      case TR_small         :
      case TR_normalsize    :
      case TR_large         :
      case TR_Large         :
      case TR_LARGE         :
      case TR_huge          :
      case TR_Huge          :
        in_mml_grammar  =  TRUE;
      break;

	  default :
	    TCI_ASSERT(0);
	  break;
    }


    if ( in_mml_grammar ) {
	  U8 zuID[32];
      U16 uID =  is_text ? 1 : 0;
      UidsTozuID( 5,tag_ilk,uID,(U8*)zuID );
      U8* dest_zname;
      U8* d_template;
      if ( d_mml_grammar->GetGrammarDataFromUID(zuID,context_math,
						                &dest_zname,&d_template) ) {
        if ( d_template && d_template[0] )
          strcpy( (char*)attr_str,(char*)d_template );
      }
    }


	TCI_BOOL tag_each_node  =  TRUE;
    if ( attr_str[0] )		// nest in <mstyle>
      if ( !TeX_tag_node->next && !TeX_tag_node->prev )
	    tag_each_node =  FALSE;

	if ( tag_each_node ) {

// The algorithm here is VERY different from all other schemata.
// A mml_list is generated from the input LaTeX contents list,
//  ( the source tagged MATH ), but no bindings are done.
// Instead, each node from the initial pass is isolated,
//  tagged, and appended to mml_rv.
// The caller, MathStructureToMML, passes mml_rv back to his only caller,
//  TranslateMathList.  Here the list is appended to the first-pass
//  mml list that is being built.  Bindings are done later at this level.

      TCI_BOOL do_bindings  =  FALSE;
      U16 tex_nodes_done,loc_error;
      TNODE* local_oof_list =  NULL;
      TNODE* mml_cont =  TranslateMathList( TeX_cont,do_bindings,
					        NULL,tex_nodes_done,loc_error,&local_oof_list );

  // Traverse the first level mml list.
  //  Isolate each node, tag it,
  //  and append the result to the final return list, mml_rv.

	  TNODE* tail =  NULL;	// for the final return list

      TNODE* mml_rover  =  mml_cont;
	  while ( mml_rover ) {

        TNODE* mml_next =  mml_rover->next;
	    mml_rover->next =  NULL;
	    mml_rover->prev =  NULL;
	    mml_rover->sublist_owner =  NULL;

        TNODE* curr_tagged_node =  NULL;
	    TCI_BOOL nest_in_mstyle =  TRUE;

        U16 uobj,usub,id;
        GetUids( mml_rover->zuID,uobj,usub,id );
	    if ( uobj==3 ) {
          U8 buffer[64];
          buffer[0] =  0;
	      if ( mml_rover->var_value ) {
	        if ( mml_rover->v_len==1 ) {
		      U8 ch =  mml_rover->var_value[0];

		      if ( ch=='{' || ch=='}' || ch=='[' || ch==']'
              ||   ch=='(' || ch==')' || ch=='|' ) {

	            nest_in_mstyle =  FALSE;

              } if ( (ch>='A' && ch<='Z') || (ch>='a' && ch<='z') ) {

                if ( mathvariant ) {
                  if ( mml_version >= 200 ) {
                    SetNodeAttrib( mml_rover,(U8*)"mathvariant",mathvariant );
	                nest_in_mstyle =  FALSE;
			      } else if ( entity_suffix ) {
                    strcpy( (char*)buffer,"&" );
                    strcat( (char*)buffer,(char*)mml_rover->var_value );
			        delete mml_rover->var_value;
			        mml_rover->var_value  =  NULL;
                    strcat( (char*)buffer,(char*)entity_suffix );
                    strcat( (char*)buffer,";" );
                    SetChData( mml_rover,buffer,NULL );
	                nest_in_mstyle =  FALSE;
                    SetNodeAttrib( mml_rover,(U8*)"fontstyle",(U8*)"upright" );
			      }
			    }
		      }


	        } else if ( mml_rover->v_len==4 ) {
	          if ( !strcmp("TeX^",(char*)mml_rover->var_value)
	            ||   !strcmp("TeX_",(char*)mml_rover->var_value) )
	            nest_in_mstyle =  FALSE;

	        } else if ( mml_rover->v_len==5 ) {
	          if ( !strcmp("TeXSb",(char*)mml_rover->var_value)
	            ||   !strcmp("TeXSp",(char*)mml_rover->var_value) )
	            nest_in_mstyle =  FALSE;

	        }


		  }		// if ( entity_suffix )

	    } else
	      nest_in_mstyle =  TRUE;

	    if ( nest_in_mstyle ) {
          if ( attr_str[0] ) {		// nest in <mstyle>
            mml_rover =  FixImpliedMRow( mml_rover );
            TNODE* mstyle =  CreateElemWithBucketAndContents( 5,600,0,2,mml_rover );
            SetDetailNum( mstyle,DETAILS_style,tag_ilk );

//          SetNodeAttrib( mstyle,attr_nom,attr_val );
            SetMMLAttribs( mstyle,attr_str );

            if ( attr_nom2 )
              SetNodeAttrib( mstyle,attr_nom2,attr_val2 );

            curr_tagged_node  =  CreateElemWithBucketAndContents( 5,750,1,2,mstyle );
          } else
	        curr_tagged_node  =  mml_rover;
	      } else
	        curr_tagged_node  =  mml_rover;

  // Append the "tagged" node to output list

          if ( curr_tagged_node ) {
	        if ( !mml_rv )
		      mml_rv  =  curr_tagged_node;
		    else {
		      tail->next  =  curr_tagged_node;
		      curr_tagged_node->prev  =  tail;
		    }
		    tail  =  curr_tagged_node;
	      }

	      mml_rover =  mml_next;
	    }		// loop thru first-pass mml nodes


      //mml_cont  =  HandleOutOfFlowObjects( mml_cont,
      //  					    &local_oof_list,out_of_flow_list,2 );

        if ( local_oof_list ) {
          local_oof_list  =  DisposeOutOfFlowList( local_oof_list,0,0 );
        }


	  } else {

// Here we are handling an isolated run of MATH
//  that will be nested within an <mstyle> element.
// We translate fully - with all bindings completed
//  before nesting the output.

      TCI_BOOL do_bindings  =  TRUE;
      U16 tex_nodes_done,loc_error;
      TNODE* local_oof_list =  NULL;
      TNODE* mml_cont =  TranslateMathList( TeX_cont,do_bindings,
                    NULL,tex_nodes_done,loc_error,&local_oof_list );
      mml_cont  =  HandleOutOfFlowObjects( mml_cont,
      						      &local_oof_list,out_of_flow_list,2 );

      if ( attr_str[0] ) {		// nest in <mstyle>
        mml_cont  =  FixImpliedMRow( mml_cont );
        TNODE* mstyle =  CreateElemWithBucketAndContents( 5,600,0,2,mml_cont );
        SetDetailNum( mstyle,DETAILS_style,tag_ilk );
      //SetNodeAttrib( mstyle,attr_nom,attr_val );
        SetMMLAttribs( mstyle,attr_str );
        if ( attr_nom2 )
          SetNodeAttrib( mstyle,attr_nom2,attr_val2 );

        mml_rv  =  CreateElemWithBucketAndContents( 5,750,1,2,mstyle );
      } else
		TCI_ASSERT(0);
	}

  }	  // if ( TeX_cont )

  return mml_rv;
}



// Translate a TEXT bucket to MathML.
// If the bucket is tagged, nest in <mstyle>

TNODE* LaTeX2MMLTree::TaggedText2MML( TNODE* TeX_cont,
										TNODE** out_of_flow_list,
										U8* tag_nom,U16 subclass ) {

  TNODE* mml_rv =  NULL;

  if ( TeX_cont ) {
    TCI_BOOL no_line_breaks =  FALSE;
	TCI_BOOL is_unit  =  FALSE;
    mml_rv  =  TextInMath2MML( TeX_cont,out_of_flow_list,
                                no_line_breaks,is_unit );

// If we have a subclass, we lookup an attrib name/value in MathML.gmr
//   This handles known tags - textbf, textsl, etc.

    U8 attr_str[512];
    attr_str[0] =  0;

    if ( subclass ) {
      if ( subclass != 90 ) {   // \text{}
	    U8 zuID[32];
        UidsTozuID( 5,subclass,1,(U8*)zuID );
        U8* dest_zname;
        U8* d_template;
        if ( d_mml_grammar->GetGrammarDataFromUID(
					                    zuID,context_math,
   							            &dest_zname,&d_template) ) {
          if ( d_template && d_template[0] )
            strcpy( (char*)attr_str,(char*)d_template );
        } else
          TCI_ASSERT(0);
      }

    } else if ( tag_nom && *tag_nom ) {

// If we have a tag name and no subclass, we lookup
//  an attrib name/value in a special section of MathML.gmr.
//  This handles cst defined tags - MenuDialog, etc.

      U16 nln =  strlen( (char*)tag_nom );
      U8* mml_zuID;
      U8* tag_zinfo;
      if ( s_mml_grammar->GetGrammarDataFromNameAndAttrs(
                                tag_nom,nln,NULL,(U8*)"TEXTTAGS",
                                &mml_zuID,&tag_zinfo) ) {
        if ( tag_zinfo && *tag_zinfo ) {
// MenuDialog<uID14.1.1>fontweight,bold
          if ( strlen((char*)tag_zinfo) < 512 ) {
            strcpy( (char*)attr_str,(char*)tag_zinfo );
          }
        }
      } else    // tag name not found in MathML.gmr, "TEXTTAGS"
        TCI_ASSERT(0);
    } else
      TCI_ASSERT(0);

    if ( attr_str[0] ) {		// nest in <mstyle>
      mml_rv  =  FixImpliedMRow( mml_rv );
      mml_rv  =  CreateElemWithBucketAndContents( 5,600,0,2,mml_rv );
      SetDetailNum( mml_rv,DETAILS_style,3 );
      SetMMLAttribs( mml_rv,attr_str );
    }
  }

  return mml_rv;
}


// In LaTeX, scripts are not attached to any base object.
//   The initial translation to MML produces what I call "psuedo script
//   nodes". A psuedo script node indicates that the following
//   node contains the body of a script that is not yet attached.
// The following function removes a psuedo script node, joining
//   the preceding node (base) and the following node (script)
//   in an <msub>, <msub>, or <msubsup>.

TNODE* LaTeX2MMLTree::MakeMMLScript( TNODE* list_head,
                                     TNODE* psuedo_script_node,
                                     U16 precedence,
                                     TNODE** p_new_mscript,
                                     TCI_BOOL missing_left ) {

// First, we check for a double script, typically _{..}^{..}.
// In LaTeX, two consecutive scripts modify the same base
//  and are aligned vertically.
// In this case we need to generate an <msubsup>.

  if ( psuedo_script_node                   // TeX_
  &&   psuedo_script_node->next             // "a"
  &&   psuedo_script_node->next->next ) {   // TeX^

    U8* op_nom  =  psuedo_script_node->var_value;

// 1 - subscript, 2 - superscript

    U16 script_ilk1 =  2;
    if      ( !strcmp("TeX_",(char*)op_nom) )
      script_ilk1 =  1;
    else if ( !strcmp("TeXSb",(char*)op_nom) )
      script_ilk1 =  1;

// Note: we currently script ^{\substack{ 123 \\ 456}}
//                      NOT  \Sp 123 \\ 456 \endSp
//   internal format is      \Sp{\cell{123}\CELL{456}}

    TNODE* nnn  =  psuedo_script_node->next->next;

    U16 uobj,usub,id;
    GetUids( nnn->zuID,uobj,usub,id );
    if ( uobj==3 && usub==201 && id==1 ) {	// mi<uID3.203.1>
      U8* op_nom  =  nnn->var_value;
  	  U16 script_ilk2 =  0;
      if      ( !strcmp("TeX^",(char*)op_nom) )
  	    script_ilk2 =  2;
      else if ( !strcmp("TeX_",(char*)op_nom) )
  	    script_ilk2 =  1;
      else if ( !strcmp("TeXSp",(char*)op_nom) )
  	    script_ilk2 =  2;
      else if ( !strcmp("TeXSb",(char*)op_nom) )
  	    script_ilk2 =  1;

  	  if ( script_ilk2 && script_ilk2 != script_ilk1 )
        return MakeMMLSubSup( list_head,psuedo_script_node,
                                p_new_mscript,missing_left );
    }

  }

  *p_new_mscript =  NULL;
  TNODE* rv =  list_head;

  TNODE* parent =  NULL;
  TCI_BOOL new_list_head  =  FALSE;

  TNODE* left_anchor  =  psuedo_script_node->prev;
  if ( !missing_left )
    left_anchor =  left_anchor->prev;

  TNODE* operand_left   =  missing_left ? NULL : psuedo_script_node->prev;
  TNODE* middle         =  psuedo_script_node;
  TNODE* operand_right  =  middle->next;

  TNODE* right_anchor =  NULL;
  if ( operand_right )
    right_anchor =  operand_right->next;

  if ( missing_left ) {
    if ( middle->sublist_owner )
      parent  =  middle->sublist_owner;
  } else {
    if ( operand_left->sublist_owner )

      parent  =  operand_left->sublist_owner;
  }

  if ( missing_left ) {
    if ( middle == list_head )
      new_list_head  =  TRUE;
  } else {
    if ( operand_left == list_head )
      new_list_head  =  TRUE;
  }

// cut out the sublist being nested

  if ( left_anchor )
    left_anchor->next   =  NULL;
  if ( right_anchor )
    right_anchor->prev  =  NULL;
  if ( operand_left )
    operand_left->prev  =  NULL;
  if ( operand_right )
    operand_right->next =  NULL;
  if ( parent ) {
    parent->contents    =  NULL;
    if ( missing_left )
      middle->sublist_owner  =  NULL;
    else
      operand_left->sublist_owner  =  NULL;
  }

  middle->next  =  NULL;
  middle->prev  =  NULL;
  if ( operand_left )
    operand_left->next  =  NULL;
  if ( operand_right )
    operand_right->prev =  NULL;
  DisposeTNode( middle );
  
/*
msub<uID5.50.2>!msub!_SUBBASE__SUBSCRIPT_!/msub!
_SUBBASE_reqELEMENT(5.50.3)
_SUBSCRIPT_reqELEMENT(5.50.4)

msup<uID5.51.2>!msup!reqELEMENT(5.51.3)_SUPERSCRIPT_!/msup!
_SUPERSCRIPT_reqELEMENT(5.51.5)
*/

  if ( operand_right ) {
    if ( !operand_left )
      operand_left  =  MakeSmallmspace();

    U16 usubtype  =  precedence==67 ? 50 : 51;
    TNODE* mscript  =  CreateElemWithBucketAndContents( 5,usubtype,2,3,operand_left );

    mscript->src_linenum  =  psuedo_script_node->src_linenum;

    U16 script_bucketID  =  usubtype==50 ? 4 : 5;
    TNODE* part1  =  mscript->parts;
    TNODE* part2  =  CreateBucketWithContents(5,usubtype,script_bucketID,operand_right );
    part1->next   =  part2;
    part2->prev   =  part1;

    if ( mscript ) {
      if ( operand_left->details ) {
        if ( operand_left->details->unit_state != UNDEFINED_DETAIL )
          SetDetailNum( mscript,DETAILS_unit_state,
                        operand_left->details->unit_state );
        if ( operand_left->details->function_status != UNDEFINED_DETAIL )
          SetDetailNum( mscript,DETAILS_function_status,
                        operand_left->details->function_status );
        if ( operand_left->details->is_differential != UNDEFINED_DETAIL )
          SetDetailNum( mscript,DETAILS_is_differential,
                        operand_left->details->is_differential );
        if ( operand_left->details->form != UNDEFINED_DETAIL )
          SetDetailNum( mscript,DETAILS_form,1 );
        if ( operand_left->details->precedence != UNDEFINED_DETAIL )
		  // partialD and nabla should be 54
          SetDetailNum( mscript,DETAILS_precedence,55 );
        if ( operand_left->details->TeX_atom_ilk != UNDEFINED_DETAIL )
          SetDetailNum( mscript,DETAILS_TeX_atom_ilk,
                        operand_left->details->TeX_atom_ilk );
      }

      *p_new_mscript =  mscript;
      if ( left_anchor ) {
        left_anchor->next =  mscript;
        mscript->prev     =  left_anchor;
      }
      if ( right_anchor ) {
        right_anchor->prev  =  mscript;
        mscript->next   =  right_anchor;
      }
      if ( parent ) {
        parent->contents  =  mscript;
        mscript->sublist_owner =  parent;
      }
      if ( new_list_head )
        rv =  mscript;

    } else
      TCI_ASSERT(0);

  } else
    TCI_ASSERT(0);

  return rv;
}



TNODE* LaTeX2MMLTree::MakeMMLSubSup( TNODE* list_head,
                                     TNODE* psuedo_script_node,
                                     TNODE** p_new_mscript,
                                     TCI_BOOL missing_left ) {

  U16 script_ilk  =  strcmp("TeX^",(char*)psuedo_script_node->var_value) ? 1 : 2;

  *p_new_mscript  =  NULL;
  TNODE* rv =  list_head;

  TCI_BOOL new_list_head  =  FALSE;
  TNODE* parent =  NULL;

  TNODE* left_anchor  =  psuedo_script_node->prev;
  if ( !missing_left )
    left_anchor =  left_anchor->prev;

  TNODE* base     =  missing_left ? NULL : psuedo_script_node->prev;
  TNODE* script_op1  =  psuedo_script_node;
  TNODE* script1  =  script_op1->next;
  TNODE* script_op2  =  script1->next;
  TNODE* script2  =  script_op2->next;

  TNODE* right_anchor =  NULL;
  if ( script2 )
    right_anchor =  script2->next;

  if ( missing_left ) {
    if ( script_op1->sublist_owner )
      parent  =  script_op1->sublist_owner;
  } else {
    if ( base->sublist_owner )
      parent  =  base->sublist_owner;
  }

  if ( missing_left ) {
    if ( script_op1 == list_head )
      new_list_head  =  TRUE;
  } else {
    if ( base == list_head )
      new_list_head  =  TRUE;
  }

// cut out the sublist being nested

  if ( left_anchor )
    left_anchor->next   =  NULL;
  if ( right_anchor )
    right_anchor->prev  =  NULL;
  if ( base )
    base->prev  =  NULL;
  if ( script2 )
    script2->next   =  NULL;
  if ( parent ) {
    parent->contents    =  NULL;
    if ( missing_left )
      script_op1->sublist_owner  =  NULL;
    else
      base->sublist_owner  =  NULL;
  }

  if ( base )
    base->next  =  NULL;
  script_op1->next  =  NULL;
  script_op1->prev  =  NULL;
  if ( script1 ) {
    script1->prev =  NULL;
    script1->next =  NULL;
  }
  script_op2->next  =  NULL;
  script_op2->prev  =  NULL;
  if ( script2 ) {
    script2->prev =  NULL;
    script2->next =  NULL;
  }
  if ( right_anchor )
    right_anchor->prev  =  NULL;

  DisposeTNode( script_op1 );
  DisposeTNode( script_op2 );
  
  if ( !base ) {
    base  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
    U8 buffer[32];
    buffer[0] =  0;
    U8 uni_buffer[32];
    uni_buffer[0] =  0;
    AppendEntityToBuffer( (U8*)"9.1.10",buffer,uni_buffer );
    SetChData( base,(U8*)buffer,(U8*)uni_buffer );
  }

/*
msubsup<uID5.52.2>!msubsup!reqELEMENT(5.52.3)_SSSUB__SSSUPER_!/msubsup!
_SSSUB_reqELEMENT(5.52.4)
_SSSUPER_reqELEMENT(5.52.5)
*/

  TNODE* mscript  =  CreateElemWithBucketAndContents( 5,52,2,3,base );
  TNODE* part1  =  mscript->parts;
  TNODE* sub    =  ( script_ilk == 1 ) ? script1 : script2;
  TNODE* super  =  ( script_ilk == 1 ) ? script2 : script1;
  TNODE* part2  =  CreateBucketWithContents(5,52,4,sub );
  part1->next   =  part2;
  part2->prev   =  part1;
  TNODE* part3  =  CreateBucketWithContents(5,52,5,super );
  part2->next   =  part3;
  part3->prev   =  part2;

  if ( mscript ) {
    if ( base->details ) {
      if ( base->details->unit_state != UNDEFINED_DETAIL )
        SetDetailNum( mscript,DETAILS_unit_state,
                                base->details->unit_state );
      if ( base->details->function_status != UNDEFINED_DETAIL )
        SetDetailNum( mscript,DETAILS_function_status,
                                base->details->function_status );
      if ( base->details->is_differential != UNDEFINED_DETAIL )
        SetDetailNum( mscript,DETAILS_is_differential,
                                base->details->is_differential );
      if ( base->details->form != UNDEFINED_DETAIL )
        SetDetailNum( mscript,DETAILS_form,base->details->form );
      if ( base->details->precedence != UNDEFINED_DETAIL )
		  // partialD and nabla should be 54
        SetDetailNum( mscript,DETAILS_precedence,
                                base->details->precedence );
      if ( base->details->TeX_atom_ilk != UNDEFINED_DETAIL )
        SetDetailNum( mscript,DETAILS_TeX_atom_ilk,
                        base->details->TeX_atom_ilk );
    }
    *p_new_mscript =  mscript;
    if ( left_anchor ) {
      left_anchor->next =  mscript;
	    mscript->prev     =  left_anchor;
    }
    if ( right_anchor ) {
      right_anchor->prev  =  mscript;
      mscript->next   =  right_anchor;
    }
    if ( parent ) {
      parent->contents  =  mscript;
      mscript->sublist_owner =  parent;
    }
    if ( new_list_head )
      rv =  mscript;

  } else
    TCI_ASSERT(0);

  return rv;
}



TNODE* LaTeX2MMLTree::BindUnits( TNODE* MML_list ) {

  TNODE* mml_rv =  MML_list;

  U16 stopping_prec =  46;	// a bit arbitrary

  TNODE* MML_rover  =  MML_list;
  while ( MML_rover ) {		// loop thru contents nodes

    TNODE* new_mrow;

  // Currently, only attaching units in this pass

    if ( MML_rover->details
    &&   MML_rover->details->unit_state==1 ) {

  	  U16 l_space_nodes,l_operand_nodes;
      I16 local_space_width;
      if ( OperandExists(MML_rover,FALSE,stopping_prec,
                l_space_nodes,l_operand_nodes,local_space_width) ) {

        if ( l_operand_nodes > 1 ) {
          TCI_ASSERT(0);
          TNODE* right_anchor =  MML_rover;
          U16 tally =  l_space_nodes;
          while ( tally ) {
            right_anchor  =  right_anchor->prev;
            tally--;
          }
          mml_rv  =  NestNodesInMrow( mml_rv,right_anchor,NULL,
		  				            FALSE,l_operand_nodes,&new_mrow );
        }

        TNODE* it =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );	// <mo>
        SetChData( it,entity_it,entity_it_unicode );

	    SetNodeAttrib( it,(U8*)"form",(U8*)zop_forms[OPF_infix] );

//&deg;  &prime;  &Prime;
        if ( !NonSpacedUnit(MML_rover) )
          SetNodeAttrib( it,(U8*)"rspace",(U8*)"thickmathspace" );

        SetDetailNum( it,DETAILS_form,2 );
        SetDetailNum( it,DETAILS_precedence,46 );
        SetDetailNum( it,DETAILS_TeX_atom_ilk,TeX_ATOM_BIN );

        TNODE* lefty  =  MML_rover->prev;
        it->next  =  MML_rover;
        it->prev  =  lefty;
        MML_rover->prev =  it;
        lefty->next =  it;        


        U16 node_to_nest  =  3 + l_space_nodes;
        TNODE* start  =  lefty;
        U16 tally =  l_space_nodes;
        while ( tally ) {
          start =  start->prev;
          tally--;
        }

        mml_rv  =  NestNodesInMrow( mml_rv,NULL,start,TRUE,
									node_to_nest,&new_mrow );
	    if ( new_mrow ) {
          SetDetailNum( new_mrow,DETAILS_unit_state,11 );
          MML_rover =  new_mrow;
        } else {
          TCI_ASSERT(0);
        }

	  } else {
        if ( MML_rover->prev )  {
          TCI_ASSERT(0);		// un-attached unit
        }
	  }

    }       // if ( details->unit_state==1 )

    MML_rover =  MML_rover->next;
  }       // loop thru MML node list

  return mml_rv;
}


// $2\frac{5}{8}$

TNODE* LaTeX2MMLTree::BindMixedNumbers( TNODE* MML_list ) {

  TNODE* rv =  MML_list;

  TNODE* MML_rover  =  MML_list;
  while ( MML_rover ) {		// loop thru contents nodes
// Currently, only binding mixed numbers in this pass
	  if ( IsMMLNumber(MML_rover) && MML_rover->next ) {
      TCI_BOOL do_binding =  FALSE;
	    TNODE* frac =  MML_rover->next;

      U16 uobj,usub,id;
      GetUids( frac->zuID,uobj,usub,id );
// mfrac<uID5.1.0>!mfrac!reqELEMENT(5.1.1)reqELEMENT(5.1.2)!/mfrac!
      if ( uobj==5 && usub==1 && id==0 ) {
	      TNODE* num_bucket =  frac->parts;
		    if ( num_bucket && num_bucket->contents ) {
	        TNODE* denom_bucket =  num_bucket->next;
		      if ( denom_bucket && denom_bucket->contents ) {
	        if ( IsMMLNumber(num_bucket->contents)
	        && 	 IsMMLNumber(denom_bucket->contents) )
              do_binding =  TRUE;
		      }
		    }
	    }

      if ( do_binding ) {
        TNODE* new_mrow;
		//TNODE* anchor =  MML_rover->prev;
		    rv  =  NestNodesInMrow( rv,NULL,MML_rover,TRUE,2,&new_mrow );
		    if ( new_mrow ) {
		// may want to add DETAILS_is_number
          SetDetailNum( new_mrow,DETAILS_is_expression,1 );
		      MML_rover =  new_mrow;
		    } else
		      TCI_ASSERT(0);
	    }
    }       // 	if ( IsMMLNumber(MML_rover) && MML_rover->next )

    MML_rover =  MML_rover->next;
  }       // loop thru MML node list

  return rv;
}



TNODE* LaTeX2MMLTree::BindDegMinSec( TNODE* MML_list ) {

  TNODE* rv =  MML_list;

  TNODE* MML_rover  =  MML_list;
  while ( MML_rover ) {		// loop thru contents nodes
    U16 t1 =  GetDegMinSec( MML_rover );
    U16 t2 =  0;
    U16 t3 =  0;
    if ( t1 && MML_rover->next ) {
      t2 =  GetDegMinSec( MML_rover->next );
      if ( MML_rover->next->next )
        t3 =  GetDegMinSec( MML_rover->next->next );

      U16 nodes_to_bind =  0;
      if        ( t1 == 1 ) {     // degree
        if ( t2 == 2 ) {
          nodes_to_bind =  2;
          if ( t3 == 3 )
            nodes_to_bind++;
        } else if ( t2 == 3 )
          nodes_to_bind =  2;
	
      } else if ( t1 == 2 ) {     // minute
        if ( t2 == 3 )
          nodes_to_bind =  2;
      }	

      if ( nodes_to_bind ) {
        TNODE* new_mrow;
		TNODE* anchor =  NULL;
		if ( MML_rover->prev )
		  anchor =  MML_rover->prev;
        rv  =  NestNodesInMrow( rv,anchor,MML_rover,TRUE,nodes_to_bind,&new_mrow );
        if ( new_mrow ) {
		// may want to add DETAILS_is_number
          SetDetailNum( new_mrow,DETAILS_is_expression,1 );
          MML_rover =  new_mrow;
        } else
          TCI_ASSERT(0);
      }

    }     // if ( t1 && MML_rover->next )

    MML_rover =  MML_rover->next;
  }       // loop thru MML node list

  return rv;
}



TCI_BOOL LaTeX2MMLTree::IsVector( TNODE* mml_node ) {

  TCI_BOOL rv =  FALSE;
  if ( mml_node->details ) {
    if ( mml_node->details->delimited_group == 1 ) {
      GROUP_INFO gi;
      GetGroupInfo( mml_node,gi );
      if ( gi.opening_delim[0]=='('
      ||   gi.opening_delim[0]=='{'
      ||   gi.opening_delim[0]=='[' ) {
	    if ( MMLDelimitersMatch(gi) )
	      if ( gi.lowest_precedence == 3 )	// has commas
	        if ( gi.operator_count == gi.separator_count )
		      if ( gi.n_interior_nodes == 2*gi.separator_count + 1 )
                rv =  TRUE;
	    }
	  }
  }

  return rv;
}



TCI_BOOL LaTeX2MMLTree::IsQualifier( TNODE* mml_node ) {

  TCI_BOOL rv =  FALSE;
  if ( mml_node->details ) {
    if ( mml_node->details->delimited_group == 1 ) {
      GROUP_INFO gi;
      GetGroupInfo( mml_node,gi );
      if ( gi.opening_delim[0]=='(' || gi.opening_delim[0]=='{' )
	    if ( MMLDelimitersMatch(gi) )
      if ( gi.lowest_precedence < 28 )	// 28 is + or -
        rv =  TRUE;
	    else {
		    if ( gi.is_mod )
          rv =  TRUE;
	    }
	  }
  }

  return rv;
}


// Function to determine if the delimiters on a group
//  are canonically matched. (a) {a} [a] |a| ||a|| <a> etc. 

TCI_BOOL LaTeX2MMLTree::MMLDelimitersMatch( GROUP_INFO& gi ) {

  TCI_BOOL rv =  FALSE;

  U8* opener  =  gi.opening_delim;
  U8* closer  =  gi.closing_delim;

  if ( !opener[0] || !closer[0] )
    return FALSE;

  if ( opener[0] == '&' && opener[1] == '#' ){
    TCI_ASSERT(0);
  }

  U16 zln_left  =  strlen( (char*)opener );
  U16 zln_right =  strlen( (char*)closer );

  if ( zln_left == 1 ) {
    if ( zln_right == 1 ) {
	    U8 l_char =  opener[0];
	    U8 r_char =  closer[0];
	    if        ( l_char=='(' ) {
	      if ( r_char==')' ) rv =  TRUE;
	    } else if ( l_char=='[' ) {
	      if ( r_char==']' ) rv =  TRUE;
	    } else if ( l_char=='{' ) {
	      if ( r_char=='}' ) rv =  TRUE;
	    } else if ( l_char=='|' ) {
	      if ( r_char=='|' ) rv =  TRUE;
	    } else if ( l_char=='\\' ) {
	      if ( r_char=='\\' ) rv =  TRUE;
	    } else if ( l_char=='/' ) {
	      if ( r_char=='/' ) rv =  TRUE;
	    } else
	      TCI_ASSERT(0);
	  }

  }	else if ( zln_left == 2 ) {
    if ( zln_right == 2 ) {
	    if ( opener[0] == '|' && closer[0] == '|' )
	      if ( opener[1] == '|' && closer[1] == '|' )
		      rv  =  TRUE;
	  }

  }	else if ( zln_left == 7 ) {
    if ( !strcmp((char*)opener,"&lceil;") )
      if ( !strcmp((char*)closer,"&rceil;") )
		    rv  =  TRUE;

    if ( !strcmp((char*)opener,"&Slash;") )
      if ( !strcmp((char*)closer,"&Slash;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 8 ) {
    if ( !strcmp((char*)opener,"&langle;") )
      if ( !strcmp((char*)closer,"&rangle;") )
		    rv  =  TRUE;

    if ( !strcmp((char*)opener,"&lfloor;") )
      if ( !strcmp((char*)closer,"&rfloor;") )
		    rv  =  TRUE;

    if ( !strcmp((char*)opener,"&Verbar;") )
      if ( !strcmp((char*)closer,"&Verbar;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 9 ) {
    if ( !strcmp((char*)opener,"&UpArrow;") )
      if ( !strcmp((char*)closer,"&UpArrow;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 11 ) {
    if ( !strcmp((char*)opener,"&LeftFloor;") )
      if ( !strcmp((char*)closer,"&RightFloor;") )
		    rv  =  TRUE;

    if ( !strcmp((char*)opener,"&Backslash;") )
      if ( !strcmp((char*)closer,"&Backslash;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 13 ) {
    if ( !strcmp((char*)opener,"&LeftCeiling;") )
      if ( !strcmp((char*)closer,"&RightCeiling;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 14 ) {
    if ( !strcmp((char*)opener,"&LeftSkeleton;") )
      if ( !strcmp((char*)closer,"&RightSkeleton;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 15 ) {
    if ( !strcmp((char*)opener,"&DoubleUpArrow;") )
      if ( !strcmp((char*)closer,"&DoubleUpArrow;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 16 ) {
    if ( !strcmp((char*)opener,"&OpenCurlyQuote;") )
      if ( !strcmp((char*)closer,"&CloseCurlyQuote;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 17 ) {
    if ( !strcmp((char*)opener,"&DoubleDownArrow;") )
      if ( !strcmp((char*)closer,"&DoubleDownArrow;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 18 ) {
    if ( !strcmp((char*)opener,"&LeftAngleBracket;") )
      if ( !strcmp((char*)closer,"&RightAngleBracket;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 19 ) {
    if ( !strcmp((char*)opener,"&LeftBracketingBar;") )
      if ( !strcmp((char*)closer,"&RightBracketingBar;") )
		    rv  =  TRUE;

    if ( !strcmp((char*)opener,"&LeftDoubleBracket;") )
      if ( !strcmp((char*)closer,"&RightDoubleBracket;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 22 ) {
    if ( !strcmp((char*)opener,"&OpenCurlyDoubleQuote;") )
      if ( !strcmp((char*)closer,"&CloseCurlyDoubleQuote;") )
		    rv  =  TRUE;

  }	else if ( zln_left == 25 ) {
    if ( !strcmp((char*)opener,"&LeftDoubleBracketingBar;") )
      if ( !strcmp((char*)closer,"&RightDoubleBracketingBar;") )
		    rv  =  TRUE;

  } else
    TCI_ASSERT(0);

  return rv;
}



TCI_BOOL LaTeX2MMLTree::IsMMLFactor( TNODE* mml_node,
                                    TCI_BOOL include_differential,
									TCI_BOOL& is_capitol_letter ) {

  TCI_BOOL rv =  FALSE;
  is_capitol_letter =  FALSE;

  TCI_BOOL look_inside  =  FALSE;
  if ( mml_node->details ) {
    if ( mml_node->details->form != UNDEFINED_DETAIL ) {	// an operator
      rv =  FALSE;

    } else if ( mml_node->details->delimited_group == 1 ) {
      GROUP_INFO gi;
      GetGroupInfo( mml_node,gi );

      if ( !gi.has_mtext ) {
        if ( MMLDelimitersMatch(gi) ) {
          rv =  TRUE;
          if ( gi.lowest_precedence < 28 )	// 28 is + or -
            rv =  FALSE;
  	    }
  	  }

// if GroupHasMatchedDelimiter && 

    } else if ( mml_node->details->is_expression != UNDEFINED_DETAIL ) {
      rv =  TRUE;

    } else if ( mml_node->details->bigop_status != UNDEFINED_DETAIL ) {
      rv  =  ( mml_node->details->bigop_status == 1 ) ? FALSE : TRUE;

    } else if ( mml_node->details->function_status != UNDEFINED_DETAIL ) {
      rv =  ( mml_node->details->function_status > 10 ) ? TRUE : FALSE;

    } else if ( mml_node->details->is_differential != UNDEFINED_DETAIL ) {
      rv =  include_differential ? TRUE : FALSE;

    } else if ( mml_node->details->integral_num != UNDEFINED_DETAIL ) {
      rv =  (mml_node->details->integral_num == 10) ? TRUE : FALSE;

    } else if ( mml_node->details->unit_state != UNDEFINED_DETAIL ) {
      rv =  ( mml_node->details->unit_state > 10 ) ? TRUE : FALSE;

    } else if ( mml_node->details->space_width != UNDEFINED_DETAIL ) {
      rv =  FALSE;

    } else if ( mml_node->details->style != UNDEFINED_DETAIL ) {
      look_inside =  TRUE;

    } else if ( mml_node->details->is_geometry != UNDEFINED_DETAIL ) {
      rv =  FALSE;

    } else if ( mml_node->details->is_Greek != UNDEFINED_DETAIL ) {
      rv =  TRUE;

    } else if ( mml_node->details->TeX_atom_ilk != UNDEFINED_DETAIL ) {
      look_inside =  TRUE;

    } else
      TCI_ASSERT(0);

  }	else
    look_inside =  TRUE;

  if ( look_inside )
    rv  =  IsMMLFactorFromInternals( mml_node,include_differential,
                                        is_capitol_letter );

  return rv;
}



TCI_BOOL LaTeX2MMLTree::IsMMLFactorFromInternals( TNODE* mml_node,
									                  TCI_BOOL include_differential,
									                  TCI_BOOL& is_capitol_letter ) {

  TCI_BOOL rv =  FALSE;
  is_capitol_letter =  FALSE;
      
  U16 uobjtype,stype,uID;
  GetUids( mml_node->zuID,uobjtype,stype,uID );

  if ( uobjtype == 3 ) {

    TCI_ASSERT( uID == 1 );

    switch ( stype ) {
      case 201  :       // mi<uID3.201.1>
        rv =  TRUE;
        if ( mml_node->v_len == 1 ) {
	        char ch =  mml_node->var_value[0];
	        if ( ch >= 'A' && ch <= 'Z' )
            is_capitol_letter =  TRUE;
        }
      break;
      case 202  :		// mn<uID3.202.1>
        rv =  FALSE;
      break;
      case 203  :		// mo<uID3.203.1>
        rv =  FALSE;
      break;
      case 204  :		// mtext<uID3.204.1>
        //if ( mml_node->next )
        //  rv  =  GetIlk( mml_node->next );
      break;
      case 205  :		// mspace<uID3.205.1>
      case 206  :		// ms<uID3.206.1>
      break;
      default   :
        TCI_ASSERT(0);
      break;
    }

  } else if ( uobjtype == 5 ) {		// a structure or bucket

    U8* bucket_ID =  NULL;

    switch ( stype ) {
      case 750  : {	//mrow<uID5.750.1>	  BUCKET(5.750.2,MATH,,,/mrow,)
		if ( mml_node->details ) {
		} else {	// must look inside
		  //TCI_ASSERT(0);
          TNODE* bucket =  FindObject( mml_node->parts,
								(U8*)"5.750.2",INVALID_LIST_POS );
          if ( bucket && bucket->contents ) {
            TCI_BOOL is_cap_letter;
            rv  =  IsMMLFactor( bucket->contents,include_differential,
                                    is_cap_letter );
          }
		}
      }
      break;
      case   1  : {	//mfrac<uID5.1.0>	  reqELEMENT(5.1.1)reqELEMENT(5.1.2)
        rv =  TRUE;
      }
      break;
      case  60  : {	//msqrt<uID5.60.0>	  BUCKET(5.60.2,MATH,,,/msqrt,)
      }
      break;
      case  61  : {	//mroot<uID5.61.0>	  reqELEMENT(5.61.2)reqELEMENT(5.61.1)
        rv =  TRUE;
      }
      break;
      case 600  : {	//mstyle<uID5.600.0>  BUCKET(5.600.2,MATH,,,/mstyle,)
    // need to look in bucket to decide
        TNODE* bucket =  FindObject( mml_node->parts,
								              (U8*)"5.600.2",INVALID_LIST_POS );
        if ( bucket && bucket->contents ) {
          TCI_BOOL is_cap_letter;
          rv  =  IsMMLFactor( bucket->contents,include_differential,
                                    is_cap_letter );
        }
      }
      break;
      case 601  : {	//merror<uID5.601.0>  BUCKET(5.601.2,MATH,,,/merror,)
      }
      break;
      case 602  : {	//mpadded<uID5.602.0> BUCKET(5.602.2,MATH,,,/mpadded,)
      }
      break;
      case 603  : {	//mphantom<uID5.603.0>BUCKET(5.603.2,MATH,,,/mphantom,)
      }
      break;
      case  70  : {	//mfenced<uID5.70.0>  _FENCEDITEMS_
    // need to look inside here
        rv =  TRUE;
      }
      break;

      case  50  : 	//msub<uID5.50.2>!msub!_SUBBASE__SUBSCRIPT_!/msub!
        bucket_ID =  (U8*)"5.50.3";
      case  51  : 	//msup<uID5.51.2>!msup!reqELEMENT(5.51.3)_SUPERSCRIPT_!/msup!
	      if ( !bucket_ID )
          bucket_ID =  (U8*)"5.51.3";
      case  52  : 	//msubsup<uID5.52.2>!msubsup!reqELEMENT(5.52.3)_SSSUB__SSSUPER_!/msubsup!
	      if ( !bucket_ID )
          bucket_ID =  (U8*)"5.52.3";
      case  53  : 	//munder<uID5.53.2>!munder!reqELEMENT(5.53.3)_UNDERSCRIPT_!/munder!
	      if ( !bucket_ID )
          bucket_ID =  (U8*)"5.53.3";
      case  54  :   //mover<uID5.54.2>!mover!reqELEMENT(5.54.3)_OVERSCRIPT_!/mover!
	      if ( !bucket_ID )
          bucket_ID =  (U8*)"5.54.3";
      case  55  : {	//munderover<uID5.55.2>!munderover!reqELEMENT(5.55.3)_UOSUB__UOSUPER_!/munderover!
	      if ( !bucket_ID )
          bucket_ID =  (U8*)"5.55.3";
        TNODE* bucket =  FindObject( mml_node->parts,
								              bucket_ID,INVALID_LIST_POS );
        if ( bucket && bucket->contents ) {
          TCI_BOOL is_cap_letter;
          rv  =  IsMMLFactor( bucket->contents,include_differential,
                                is_cap_letter );
        }
        rv =  TRUE;
      }
      break;
      case  56  : {	//mprescripts/<uID5.56.10>LIST(5.56.11,_LEFTSUBSUPPAIR_,5.56.12,,/mmultiscripts,)
        rv =  TRUE;
      }

      break;
    //case  56  : {	//mmultiscripts<uID5.56.2>!mmultiscripts!reqELEMENT(5.56.3)_RIGHTSCRIPTS__LEFTSCRIPTS_!/mmultiscripts!
    //case 602  : {	//mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd,)!/mtd!
    //case 602  : {	//mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
      case  35  : {	//mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
        rv =  TRUE;
      }
      break;

      default   :
        TCI_ASSERT(0);
      break;
    }

  } else if ( uobjtype == 9 ) {		// space

  } else	// if ( uobjtype == 3 )
    TCI_ASSERT(0);

  return rv;
}



TCI_BOOL LaTeX2MMLTree::IsMMLNumber( TNODE* mml_node ) {

  TCI_BOOL rv =  FALSE;
      
  U16 uobjtype,stype,uID;
  GetUids( mml_node->zuID,uobjtype,stype,uID );

  if ( uobjtype == 5 ) {		// a structure or bucket
    U8* bucket_ID =  NULL;

    switch ( stype ) {
      case   1  : {	//mfrac<uID5.1.0>	  reqELEMENT(5.1.1)reqELEMENT(5.1.2)
      }
      break;
      case  60  : {	//msqrt<uID5.60.0>	  BUCKET(5.60.2,MATH,,,/msqrt,)
      }
      break;
      case  61  : {	//mroot<uID5.61.0>	  reqELEMENT(5.61.2)reqELEMENT(5.61.1)
      }
      break;
      case 600  : {	//mstyle<uID5.600.0>  BUCKET(5.600.2,MATH,,,/mstyle,)
    // need to look in bucket to decide
        TNODE* bucket =  FindObject( mml_node->parts,
								(U8*)"5.600.2",INVALID_LIST_POS );
        if ( bucket && bucket->contents )
          rv  =  IsMMLNumber( bucket->contents );
      }
      break;
      case  70  : {	//mfenced<uID5.70.0>  _FENCEDITEMS_
    // need to look inside here
      }
      break;

      case  50  : 	//msub<uID5.50.2>!msub!_SUBBASE__SUBSCRIPT_!/msub!
        bucket_ID =  (U8*)"5.50.3";
      case  51  : 	//msup<uID5.51.2>!msup!reqELEMENT(5.51.3)_SUPERSCRIPT_!/msup!
	    if ( !bucket_ID )
          bucket_ID =  (U8*)"5.51.3";
      case  52  : 	//msubsup<uID5.52.2>!msubsup!reqELEMENT(5.52.3)_SSSUB__SSSUPER_!/msubsup!
	    if ( !bucket_ID )
          bucket_ID =  (U8*)"5.52.3";
      case  53  : 	//munder<uID5.53.2>!munder!reqELEMENT(5.53.3)_UNDERSCRIPT_!/munder!
	    if ( !bucket_ID )
          bucket_ID =  (U8*)"5.53.3";
      case  54  :   //mover<uID5.54.2>!mover!reqELEMENT(5.54.3)_OVERSCRIPT_!/mover!
	    if ( !bucket_ID )
          bucket_ID =  (U8*)"5.54.3";
      case  55  : {	//munderover<uID5.55.2>!munderover!reqELEMENT(5.55.3)_UOSUB__UOSUPER_!/munderover!
	    if ( !bucket_ID )
          bucket_ID =  (U8*)"5.55.3";
        TNODE* bucket =  FindObject( mml_node->parts,
								bucket_ID,INVALID_LIST_POS );
        if ( bucket && bucket->contents )
          rv  =  IsMMLNumber( bucket->contents );
      }
      break;
      case  56  : {	//mprescripts/<uID5.56.10>LIST(5.56.11,_LEFTSUBSUPPAIR_,5.56.12,,/mmultiscripts,)
      }
      break;

    //case  56  : {	//mmultiscripts<uID5.56.2>!mmultiscripts!reqELEMENT(5.56.3)_RIGHTSCRIPTS__LEFTSCRIPTS_!/mmultiscripts!
    //case 602  : {	//mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd,)!/mtd!
    //case 602  : {	//mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!

      case  35  : {	//mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
      }
      break;

      default   :
      break;
    }

  } else if ( uobjtype == 3 ) {

    TCI_ASSERT( uID == 1 );

    switch ( stype ) {
      case 201  :		// mi<uID3.201.1>
      break;
      case 202  :		// mn<uID3.202.1>
        rv  =  TRUE;
      break;
      break;
      default   :
      break;
    }

  }

  return rv;
}



TCI_BOOL LaTeX2MMLTree::IsMMLMultOp( TNODE* mml_node ) {

  TCI_BOOL rv =  FALSE;
      
  U16 uobjtype,stype,uID;
  GetUids( mml_node->zuID,uobjtype,stype,uID );

  if ( uobjtype == 5 ) {		// a structure or bucket

  } else if ( uobjtype == 3 ) {

    TCI_ASSERT( uID == 1 );

    switch ( stype ) {
      case 201  :		// mi<uID3.201.1>
      case 202  :		// mn<uID3.202.1>
      break;
      case 203  : {		// mn<uID3.202.1>
        DETAILS* info =  mml_node->details;
        if ( info 
        && info->form == 2
        && info->precedence >= 40 )
          rv  =  TRUE;
      }
      break;
      default   :
      break;
    }

  }

  return rv;
}



void LaTeX2MMLTree::SetDetailIsExpr( TNODE* mrow,
									                      I16 precedence ) {

  U16 detail_id   =  DETAILS_is_expression;
  U16 detail_val  =  1;


  SetDetailNum( mrow,detail_id,detail_val );
}


// \int ... dx

TNODE* LaTeX2MMLTree::BindDelimitedIntegrals( TNODE* MML_list ) {

  TNODE* rv =  MML_list;

  TNODE* new_mrow;
  TNODE* rover  =  MML_list;
  while ( rover ) {
    if ( rover->details
    &&   rover->details->integral_num != UNDEFINED_DETAIL ) {
      rv  =  BindIntegral( rv,rover,&new_mrow );
	  if ( new_mrow )
	    rover =  new_mrow;
	}
    rover =  rover->next;
  }		// loop thru contents list to dx

  return rv;
}


TNODE* LaTeX2MMLTree::BindIntegral( TNODE* list_head,
									TNODE* MML_integral_node,
									TNODE** pnew_mrow ) {

  TNODE* rv   =  list_head;
  *pnew_mrow  =  NULL;

  U16 n_integrals =  MML_integral_node->details->integral_num;

  TCI_BOOL is_delimited =  FALSE;
  TNODE* dd_node  =  NULL;

  U16 node_count  =  1;
  TCI_BOOL done   =  FALSE;
  TNODE* i_rover  =  MML_integral_node;
  while ( i_rover && !done ) {
    i_rover =  i_rover->next;
    node_count++;

    if ( !i_rover ) break;

  // Handle nested integrals recursively

    if ( i_rover->details 
    &&   i_rover->details->integral_num != UNDEFINED_DETAIL ) {
	  TNODE* new_mrow;
      TNODE* tmp  =  BindIntegral( NULL,i_rover,&new_mrow );
	  if ( new_mrow )
	    i_rover =  new_mrow;

	} else {	// non-nested integral clause

      U16 uobj,usub,id;
      GetUids( i_rover->zuID,uobj,usub,id );
      switch ( uobj ) {
        case  3  : {
          if        ( usub == 201 ) {	// mi<uID3.201.1>

          } else if ( usub == 203 ) {	// mo<uID3.203.1>

		    if ( i_rover->details ) {
		      U16 precedence  =  i_rover->details->precedence;

		      if ( i_rover->details->is_differential != UNDEFINED_DETAIL ) {
                dd_node =  i_rover;

                n_integrals--;
			    while ( n_integrals ) {
			      if ( i_rover->next ) {
			        if ( i_rover->next->details
		            &&   i_rover->next->details->is_differential != UNDEFINED_DETAIL ) {
			          i_rover =  i_rover->next;
			          n_integrals--;
                      node_count++;
				    } else {
				      i_rover =  i_rover->next;
                      node_count++;
					}
				// What about ellipsis?
				  } else
				    break;
			    }
                is_delimited =  TRUE;
			    done  =  TRUE;

			    if ( i_rover->next ) {
				  i_rover =  i_rover->next;
                  node_count++;
				}

		      } else if ( precedence && precedence < 27 )
			    done  =  TRUE;
		    } else
		      TCI_ASSERT(0);

          } else if ( usub == 204 )     // mtext<uID3.204.1>
            done   =  TRUE;
        }
        break;

        case  5  : {
          if ( usub == 750 ) {	// mrow<uID5.750.1>
		    if ( i_rover->details
		    &&   i_rover->details->is_differential != UNDEFINED_DETAIL ) { // differential d
              dd_node =  i_rover;

              n_integrals--;
			  while ( n_integrals ) {
			    if ( i_rover->next
			     &&	 i_rover->next->details
		        &&   i_rover->next->details->is_differential != UNDEFINED_DETAIL ) {
			      i_rover =  i_rover->next;
				  n_integrals--;
                  node_count++;
				// What about ellipsis?
				} else
				  break;
			  }
              is_delimited =  TRUE;
			  done  =  TRUE;
		    }
          }		// if ( usub == 750 )
        }
        break;

        default :
        break;
      }

    }	// non-nested integral clause

  }	  // loop thru contents list to dx

// At this point, we've spanned \int ... dx

  if ( is_delimited ) {		// we nest
    TNODE* new_mrow;
    TNODE* anchor   =  MML_integral_node->prev;
    rv  =  NestNodesInMrow( list_head,anchor,MML_integral_node,
		  			    	TRUE,node_count,&new_mrow );
	if ( new_mrow ) {
	  *pnew_mrow  =  new_mrow;


  // We mark the <mrow>, indicating that it contains a group
      SetDetailNum( new_mrow,DETAILS_integral_num,10 );

  // Now we set up a recursive call to "FinishMMLBindings"
  //  on the contents list that has been nested.

	  TNODE* cont =  new_mrow->parts->contents;
	  if ( cont ) {
	    TNODE* left_anchor    =  cont;
	    TNODE* start          =  left_anchor->next;
	    TNODE* right_anchor   =  start;
		//while ( right_anchor != dd_node ) {
		//  n_interior_nodes++;
		//  right_anchor  =  right_anchor->next;
		//}

	    U16 n_interior_nodes  =  1;
		while ( right_anchor != i_rover ) {
		  n_interior_nodes++;
		  right_anchor  =  right_anchor->next;
		}

		if ( n_interior_nodes ) {
	      TNODE* ender  =  i_rover;
		  ender->next   =  NULL;

	      TNODE* fixed_interior =  FinishMMLBindings( start );
		  if ( fixed_interior ) {
		    if ( DoNestFenceBody(fixed_interior) )
              fixed_interior =  CreateElemWithBucketAndContents( 5,750,1,2,
              											fixed_interior );
	        left_anchor->next   =  fixed_interior;
	        fixed_interior->prev  =  left_anchor;
		  } else {
	        left_anchor->next   =  right_anchor;
	        right_anchor->prev  =  left_anchor;
		  }

		} else		// if ( n_interior_nodes )
	      TCI_ASSERT(0);

	  }	else	  // if ( cont )
	    TCI_ASSERT(0);

	} else	// if ( new_mrow )
	  TCI_ASSERT(0);

  }	  // if ( is_delimited )

  return rv;
}


TNODE* LaTeX2MMLTree::BindScripts( TNODE* MML_list ) {

  TNODE* mml_rv =  MML_list;

  U16 precedence;

  TNODE* MML_rover  =  MML_list;
  while ( MML_rover ) {		// loop thru contents nodes
    U16 uobj,usub,id;
    GetUids( MML_rover->zuID,uobj,usub,id );

	TCI_BOOL is_script_op =  FALSE;
	if ( uobj==3 && usub==201 && id==1 ) {		// mi<uID3.201.1>
      U8* op_nom  =  MML_rover->var_value;
      if ( op_nom ) {
        if        ( !strcmp("TeX^",(char*)op_nom) ) {
          is_script_op  =  TRUE;
          precedence    =  58;	// this is arbitrary
        } else if ( !strcmp("TeXSp",(char*)op_nom) ) {
          is_script_op  =  TRUE;
          precedence    =  58;	// this is arbitrary
        } else if ( !strcmp("TeX_",(char*)op_nom) ) {
          is_script_op  =  TRUE;
          precedence    =  67;	// this is arbitrary
        } else if ( !strcmp("TeXSb",(char*)op_nom) ) {
          is_script_op  =  TRUE;
          precedence    =  67;	// this is arbitrary
        }
      }
	}

	if ( is_script_op ) {			// build the MML
	  TCI_BOOL no_left  =  FALSE;
      TNODE* new_mrow;
  	  U16 l_nodes_spanned;
	  if ( LocateScriptBase(MML_rover,l_nodes_spanned) ) {
	    if ( l_nodes_spanned > 1 )
	      mml_rv  =  NestNodesInMrow( mml_rv,MML_rover,NULL,FALSE,
                                        l_nodes_spanned,&new_mrow );
	// If there are more than 2 nodes being nested,
	//  we may want to call "TranslateMathList"
	//  in order to complete any required bindings.
	  } else      // no base found for script
	    no_left  =  TRUE;




	  TNODE* the_next =  MML_rover->next;
	  mml_rv  =  MakeMMLScript( mml_rv,MML_rover,precedence,
                                        &new_mrow,no_left );

	  if ( new_mrow ) {

	    if ( new_mrow->details
	    &&   new_mrow->details->form != UNDEFINED_DETAIL ) {	// an operator
// DETAILS_is_differential 7
	    } else {
          SetDetailNum( new_mrow,DETAILS_is_expression,1 );
	    }
	    MML_rover =  new_mrow;

	  } else {
	    TCI_ASSERT(0);
	    MML_rover =  the_next;
	  }





	} else  // if ( is_script_op )
	  MML_rover =  MML_rover->next;

  }     // loop thru MML node list

  return mml_rv;
}


// We will probably want to refine the following function.
// At present
//  ( any single non-text node )  - returns 2
//  ( a,a,... )                   - returns 2
//  etc.

U16 LaTeX2MMLTree::IsGroupAFuncArg( TNODE* mrow_group ) {

  U16 certainty =  0;

  if ( mrow_group ) {
    GROUP_INFO gi;
    GetGroupInfo( mrow_group,gi );
    if ( gi.opening_delim[0] == '('
    &&   gi.closing_delim[0] == ')'
    &&   !gi.has_mtext ) {

      if ( gi.n_interior_nodes == 1 ) {
        if ( gi.n_arrays == 0 )
          certainty =  2;

      } else {	// multiple interior nodes, (...)
        if        ( gi.lowest_precedence == 999 ) { // no operators
          certainty =  0;

        } else if ( gi.separator_count ) {	// has ,s or ;s
          if ( gi.operator_count == gi.separator_count )
            if ( gi.n_interior_nodes == 2*gi.separator_count +1 )
              certainty =  2;

        } else if ( gi.lowest_precedence < 28 ) {	// 28 -> +,-
          certainty =  0;

        } else
          certainty =  1;

      }			// ( multi-nodes ) clause

    }		//  ( no text )

  }		// if ( mrow_group )

  return  certainty;
}


// ^<uID5.51.1>!^!REQPARAM(5.51.2,MATH)
// \circ<uID3.13.10>

TCI_BOOL LaTeX2MMLTree::TeXScriptIsUnit( TNODE* TeX_script_node ) {

  TCI_BOOL rv =  FALSE;
  TNODE* scripted_stuff =  TeX_script_node->parts->contents;
  if ( scripted_stuff ) {
    U16 uobj,usub,id;
    GetUids( scripted_stuff->zuID,uobj,usub,id );
	if ( uobj==3 && usub==13 && id==10 )
	  if ( !scripted_stuff->next )
	    rv =  TRUE;
  }
  return  rv;
}


void LaTeX2MMLTree::SetDetailsForBoundPostfix( TNODE* mrow,I16 curr_prec ) {

  TCI_BOOL done =  FALSE;

  TNODE* operand  =  mrow->parts->contents;
  if ( operand ) {
    if ( operand
    &&   operand->details 
    &&   operand->details->function_status != UNDEFINED_DETAIL ) {
      SetDetailNum( mrow,DETAILS_function_status,
      			operand->details->function_status );
      done  =  TRUE;
    }
  }

  if ( !done )
    SetDetailNum( mrow,DETAILS_is_expression,1 );
}


TCI_BOOL LaTeX2MMLTree::LocateScriptBase( TNODE* psuedo_script,
											  U16& nodes_spanned ) {

  TCI_BOOL rv   =  FALSE;
  nodes_spanned =  0;

  TNODE* rover  =   psuedo_script->prev;
  while ( rover ) {
    rv  =  TRUE;
    nodes_spanned++;

// We may be looking at an un-bound postfix operator
//  like \prime or factorial (!).  These are considered
//  to be part of the script's base.

	if ( !rover->details
	||    rover->details->form != 3 )
      break;

    rover =  rover->prev;
  }

  return rv;
}


/* We bind operators to their operands according to operator
  precedence, often handling ranges of precedence in a single
  call like
  rv  =  BindByOpPrecedence( mml_list,max=39,min=2 );

  In order to reduce passes at precedence levels that aren't
  represented in the list, we first called the following function.
*/

void LaTeX2MMLTree::GetPrecedenceBounds( TNODE* MML_list,
											                      U16& lowest,
												                    U16& highest ) {

  lowest   =  999;	// 0 < actual values < 100
  highest  =  0;

  TNODE* MML_rover  =  MML_list;
  while ( MML_rover ) {		// loop thru contents nodes
	  U16 cp  =  0;
	  if ( MML_rover->details
	  &&   MML_rover->details->precedence != UNDEFINED_DETAIL ) {
	    cp  =  MML_rover->details->precedence;
	  } else {
	// this clause is entirely diagnostic
      U16 uobjtype,usubtype,uID;
      GetUids( MML_rover->zuID,uobjtype,usubtype,uID );
	    if ( uobjtype==3 && usubtype==203 && uID==1 ){		// <mo>
		    TCI_ASSERT(0);
      }
	  }

	  if ( cp ) {
	    if ( cp < lowest )
	      lowest   =  cp;
	    if ( cp > highest && cp != 999 )
	      highest  =  cp;
	  }

    MML_rover  =  MML_rover->next;
  }		// loop thru MML list

}


// A small number of \U{0xdddd} commands occur in our internal and
//  external formats.  These all should have lines in d_mml_grammar.
// Generally, we need to know the entity name associated with
//  the unicode and whether it is an identifier or operator.
// We can use the unicode number directly if
//  "output_entities_as_unicodes" is true.

TNODE* LaTeX2MMLTree::Unicode2MML( TNODE* obj_node ) {

  TNODE* mml_rv   =  NULL;

  U32 unicode =  ExtractUNICODE( obj_node );
  if ( unicode ) {

    U8 alias[64];
    alias[0]  =  0;

    U8 entity[64];

// script the numeric entity - ie unicode

    entity[0] =  '&';
    entity[1] =  '#';
    //ultoa( unicode,(char*)entity+2,10 );
    sprintf((char*)entity+2, "%d", unicode);
    strcat( (char*)entity,";" );


    U16 ilk =  201;

    U32 div =  unicode / 0x10000;
    U32 mod =  unicode % 0x10000;
    U8 zuID[32];
    UidsTozuID( 11,div,mod,(U8*)zuID );
    U8* dest_zname;
    U8* d_template;
    if ( d_mml_grammar->GetGrammarDataFromUID(zuID,
					context_math,&dest_zname,&d_template) ) {
      if ( !output_entities_as_unicodes )
        strcpy( (char*)entity,(char*)dest_zname );
      if ( d_template && *d_template ) {
        if      ( *d_template == 'i' )	// mi<uID3.201.1>
          ilk =  201;
        else if ( *d_template == 'n' )	// mn<uID3.202.1>
          ilk =  202;
        else if ( *d_template == 'o' )	// mo<uID3.203.1>
          ilk =  203;
        else if ( *d_template == '3' ) {	// an alias

          TNODE* tmp  =  MakeTNode( 0L,0L,0L,d_template );
          TCI_BOOL forces_geometry;
          U16 mml_ilk =  ClassifyTeXSymbol( tmp,NULL,forces_geometry,NULL );
          if      ( mml_ilk == MML_IDENTIFIER )
            ilk =  201;
          else if ( mml_ilk == MML_NUMBER	)
            ilk =  202;
          else if ( mml_ilk == MML_OPERATOR )
            ilk =  203;
          else
            TCI_ASSERT(0);

          DisposeTNode( tmp);
          U16 zln =  strlen( (char*)d_template );
          if ( zln < 64 )
            strcpy( (char*)alias,(char*)d_template );
          else
            TCI_ASSERT(0);

        } else
          TCI_ASSERT(0);
      }
    }

    UidsTozuID( 3,ilk,1,(U8*)zuID );
    mml_rv   =  MakeTNode( 0L,0L,0L,(U8*)zuID );
    SetChData( mml_rv,entity,NULL );

    if ( ilk == 201 || ilk == 202 ) {
      SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );

    } else if ( ilk == 203 ) {	// operator

// we need to set details here!

      if ( alias[0] ) {
        U8* mml_opname;
        U8* mml_op_info;
        if ( d_mml_grammar->GetGrammarDataFromUID(alias,
                    context_math,&mml_opname,&mml_op_info) ) {
          if ( mml_op_info && *mml_op_info ) {
            OP_GRAMMAR_INFO op_record;
            GetAttribsFromGammarInfo( mml_op_info,op_record );

            U16 forms_for_op;
            if ( IsMultiFormOp(mml_opname,forms_for_op) ) {
              mml_rv->attrib_list =  op_record.attr_list;
              op_record.attr_list =  NULL;
            } else {
// Here the operator has only one form - we have it's mml info.
              U16 form_ID =  op_record.form;
              if ( form_ID >= OPF_prefix && form_ID <= OPF_postfix )
                SetNodeAttrib( mml_rv,(U8*)"form",(U8*)zop_forms[form_ID] );
              else
                TCI_ASSERT(0);

              if ( op_record.attr_list ) {
                MergeMOAttribs( mml_rv,op_record.attr_list );
                DisposeAttribs( op_record.attr_list );
                op_record.attr_list =  NULL;
              }

              SetDetailNum( mml_rv,DETAILS_form,form_ID );
              SetDetailNum( mml_rv,DETAILS_precedence,op_record.precedence );
              if ( form_ID==OPF_prefix && op_record.precedence==54 )
                SetDetailNum( mml_rv,DETAILS_is_differential,1 );
            }

          }

        }    // alias lookup clause

      }   // if ( alias[0] )

    }   // if ( ilk == 203 ) 	// operator

  } else            // if ( unicode )
    TCI_ASSERT(0);

  return mml_rv;
}


// \U<uID5.298.0>!\U!REQPARAM(5.298.2,NONLATEX)
// \UNICODE<uID5.299.0>!\UNICODE!OPTPARAM(5.299.1,NONLATEX)REQPARAM(5.299.2,NONLATEX)

U32 LaTeX2MMLTree::ExtractUNICODE( TNODE* tex_uni_node ) {

  U32 rv  =  0;

  U16 uobj,usub,uID;
  GetUids( tex_uni_node->zuID,uobj,usub,uID );		// 5.298.2
  U8 bucket_uID[32];								// OR 5.299.2
  UidsTozuID( uobj,usub,2,(U8*)bucket_uID );

  TNODE* bucket =  FindObject( tex_uni_node->parts,(U8*)bucket_uID,
      										INVALID_LIST_POS );
  if ( bucket ) {		// locate NONLATEX content

    U8* zNONLATEX =  (U8*)"888.8.0";

    TNODE* cont =  FindObject( bucket->contents,zNONLATEX,
      										INVALID_LIST_POS );
    if ( cont ) {
      U32 dec_val =  0L;
      U8* p =  cont->var_value;
      if ( *p=='0' && *(p+1)=='x' ) p +=  2;	// 0x
      char ch;
      while ( ch = *p ) {						    // TCI software
        U32 inc;								        // scripts and
        if      ( ch>='0' && ch<='9' )	// expects
          inc =  ch - '0';						  // hex digits
        else if ( ch>='A' && ch<='F' )	// here.
          inc =  ch - 'A' + 10;
        else if ( ch>='a' && ch<='f' )  // We script
          inc =  ch - 'a' + 10;					// decimal unicodes
        else									          // in our XML
          TCI_ASSERT(0);
        dec_val =  dec_val * 16L + inc;
        p++;
      }

      rv  =  dec_val;

    } else				// 	if ( cont )
      TCI_ASSERT(0);

  } else
    TCI_ASSERT(0);

  return rv;
}


TCI_BOOL LaTeX2MMLTree::IsTeXVariable( TNODE* TeX_node ) {

  TCI_BOOL rv =  FALSE;

  if ( TeX_node ) {
    U16 uobjtype,usubtype,uID;
    GetUids( TeX_node->zuID,uobjtype,usubtype,uID );
    if ( uobjtype == 3 ) {
      if        ( usubtype == 1 ) {		// a..z
        char letter =  0;
        letter =  uID + 'a' - 1;
        if ( letter >= 'r' && letter <= 'z' )
          rv =  TRUE;
      } else if ( usubtype == 10 ) {	// alpha, beta,...
// \theta<uID3.10.9>
// \vartheta<uID3.10.10>
        if ( uID == 9 || uID == 10 )
          rv =  TRUE;
      }
    }   // if ( uobjtype == 3 )
  }

  return rv;
}


U8 LaTeX2MMLTree::GetHexDigit( TNODE* TeX_node ) {

  U8 rv =  0;
  if ( TeX_node ) {
    U16 uobjtype,usubtype,uID;
    GetUids( TeX_node->zuID,uobjtype,usubtype,uID );
	  if ( uobjtype == 3 ) {
	    if        ( usubtype == 1 ) {	// small letter
	      if ( uID >= 1 && uID <= 6 )
		      rv  =  'a' + uID - 1;
	    } else if ( usubtype == 2 ) {	// capitol letter
	      if ( uID >= 1 && uID <= 6 )
		    rv  =  'A' + uID - 1;
	    } else if ( usubtype == 3 ) {	// decimal digit
		    rv  =  '0' + uID - 1;
	    }
    }
  }
  return rv;
}


// \substack<uID5.32.0>
// !\substack!!{!LIST(5.32.9,BUCKET(5.32.8,MATH,,,\\|},),5.32.10,,},\\)!}!
// \begin{subarray}<uID5.717.0>!\begin{subarray}!_SAXALIGN__SACOLS__SAROWS_!\end{subarray}!
// _SAXALIGN_OPTPARAM(5.717.2,MEXTALIGN)
// _SACOLS_REQPARAM(5.717.3,COLS)

TNODE* LaTeX2MMLTree::ScriptStack2MML( TNODE* scriptstack_node,
                                    U16 subclass,
                                    TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  U8 list_uID[32];
  UidsTozuID( 5,subclass,9,(U8*)list_uID );

  TNODE* row_list =  FindObject( scriptstack_node->parts,
                                (U8*)list_uID,INVALID_LIST_POS );
  TNODE* row_rover  =  NULL;
  if ( row_list )
    row_rover =  row_list->parts;

// Generate a list of <mtr>'s

  TNODE* mtr_head =  NULL;
  TNODE* tail;

  U16 row_count =  0;
  while ( row_rover ) {     // loop down thru rows
// Translate current line to MathML
    TNODE* line_contents  =  NULL;
	  if ( row_rover->parts && row_rover->parts->contents ) {
	    TNODE* cont =  row_rover->parts->contents;
      U16 tex_nodes_done,error_code;
  	  TCI_BOOL do_bindings  =  TRUE;
      TNODE* local_oof_list =  NULL;
      line_contents =  TranslateMathList( cont,do_bindings,
                   	NULL,tex_nodes_done,error_code,&local_oof_list );
      U16 vspace_mode =  2;
      line_contents =  HandleOutOfFlowObjects( line_contents,
      							    &local_oof_list,out_of_flow_list,vspace_mode );
	  } else
	    TCI_ASSERT(0);

// mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd)!/mtd!
    line_contents =  FixImpliedMRow( line_contents );
    TNODE* cell =  CreateElemWithBucketAndContents( 5,35,14,8,line_contents );

// Put the current cell under a cell list item node
	  TNODE* list_node  =  MakeTNode( 0,0,0,(U8*)"5.35.11:0" );
	  list_node->parts  =  cell;
	  cell->sublist_owner =  list_node;

// Add current row to the list of rows we're building

// mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
// _LISTOFCELLS_LIST(5.35.11,_MTCELL_,5.35.12,,/mtr|/mtable,)
// _MTCELL_IF(MATH,?mtd?,5.35.14)ifEND

    TNODE* mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );
	  mtr->parts->parts =  list_node;
	  list_node->sublist_owner =  mtr->parts;

// Put the current row under a node in the row list
	  U8 zlistID[32];
    UidsTozuID( 5,35,9,(U8*)zlistID );
	  U16 zln =  strlen( (char*)zlistID );
	  zlistID[zln]  =  ':';
          // itoa( row_count,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1, "%d", row_count);
	  TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	  row_list_node->parts  =  mtr;
	  mtr->sublist_owner  =  row_list_node;

    if ( !mtr_head )
      mtr_head  =  row_list_node;
    else {
      tail->next  =  row_list_node;
	    row_list_node->prev =  tail;
	  }
    tail  =  row_list_node;

// Advance to the next row in the LaTeX source tree

    row_count++;
    row_rover =  row_rover->next;
  }

// mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
// _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)

  mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
  mml_rv->parts->parts  =  mtr_head;
  mtr_head->sublist_owner =  mml_rv->parts;

  SetNodeAttrib( mml_rv,(U8*)"rowspacing",(U8*)"0" );
  SetNodeAttrib( mml_rv,(U8*)"columnspacing",(U8*)"0" );

// Increment scriptlevel??
//  SetNodeAttrib( mml_rv,(U8*)"scriptlevel",(U8*)"+1" );

  if ( subclass == 717 ) {    // \begin{subarray}{l|c|r}
    TNODE* cols_bucket  =  FindObject( scriptstack_node->parts,
                                (U8*)"5.717.3",INVALID_LIST_POS );
	  if ( cols_bucket && cols_bucket->contents ) {
      U16 uobj,usub,id;
      GetUids( cols_bucket->contents->zuID,uobj,usub,id );
      if ( uobj==16 && usub==4 ) {
        if      ( id==1 )
          SetNodeAttrib( mml_rv,(U8*)"columnalign",(U8*)"left" );
        else if ( id==2 )
          SetNodeAttrib( mml_rv,(U8*)"columnalign",(U8*)"center" );
        else if ( id==3 )
	        SetNodeAttrib( mml_rv,(U8*)"columnalign",(U8*)"right" );
        else
          TCI_ASSERT(0);
      } else
        TCI_ASSERT(0);
	  } else
      TCI_ASSERT(0);
  }

  return mml_rv;
}



TNODE* LaTeX2MMLTree::MakeSmallmspace() {

  TNODE* mml_rv =  MakeTNode( 0L,0L,0L,(U8*)zmspace );

  U8 entity_buffer[32];
  strcpy( (char*)entity_buffer,(char*)"veryverythinmathspace" );

  U16 zln =  strlen( (char*)entity_buffer );
  mml_rv->attrib_list =  MakeATTRIBNode( (U8*)"width",
                                        ELEM_ATTR_width,
                                        0,	//U16 e_ID,
                                        entity_buffer,zln,0 );
  return mml_rv;
}



IDENTIFIER_NODE* LaTeX2MMLTree::AddIdentifier( IDENTIFIER_NODE* i_list,
                                                  U8* identifier_nom ) {

  IDENTIFIER_NODE* rv =  i_list;

  if ( identifier_nom && identifier_nom[0] ) {
    if ( !IdentifierInList(i_list,identifier_nom) ) {
      IDENTIFIER_NODE* node =  TCI_NEW( IDENTIFIER_NODE );
	    node->next  =  NULL;
      U16 zln     =  strlen( (char*)identifier_nom );
      char* tmp   =  TCI_NEW( char[zln+1] );
      strcpy( tmp,(char*)identifier_nom );
	    node->zidentifier =  (U8*)tmp;

	    if ( i_list ) {
	      while ( i_list->next )
	        i_list  =  i_list->next;
	      i_list->next  =  node;
	    } else
	      rv  =  node;
	  }
  }

  return rv;
}


void LaTeX2MMLTree::DisposeIdentifierList( IDENTIFIER_NODE* i_list ) {

  IDENTIFIER_NODE* rover  =  i_list;
  while ( rover ) {
    IDENTIFIER_NODE* del  =  rover;
    rover =  rover->next;
	  if ( del->zidentifier ) {
//JBMLine( (char*)del->zidentifier );
//JBMLine( "\n" );
	    delete( del->zidentifier );
	  }
	  delete( del );
  }
}


TCI_BOOL LaTeX2MMLTree::IdentifierInList( IDENTIFIER_NODE* i_list,
										                    U8* identifier_zstr ) {

  TCI_BOOL rv =  FALSE;

  IDENTIFIER_NODE* rover  =  i_list;
  while ( rover ) {
    if ( !strcmp((char*)identifier_zstr,(char*)rover->zidentifier) ) {
	    rv  =  TRUE;
	    break;
	  }
    rover =  rover->next;
  }

  return rv;
}


TCI_BOOL LaTeX2MMLTree::ScriptIsCirc( TNODE* TeX_contents ) {

  TCI_BOOL rv =  FALSE;

  if ( TeX_contents && !TeX_contents->next ) {
    U16 uobjtype,usubtype,uID;
    GetUids( TeX_contents->zuID,uobjtype,usubtype,uID );
    if ( uobjtype == 3 && usubtype == 13 && uID == 10 )
      rv  =  TRUE;
  }

  return rv;
}


// We need to know if "_{xx}" follows a DifferentialD.

TCI_BOOL LaTeX2MMLTree::BaseIsDiffD( TNODE* tex_script_node ) {

  TCI_BOOL rv =  FALSE;
  if ( tex_script_node->prev ) {
    TNODE* tex_base_node  =  tex_script_node->prev;
	if ( tex_base_node->details
	&&   tex_base_node->details->is_differential != UNDEFINED_DETAIL )
      rv  =  TRUE;
  }
  return rv;
}


// The subscript bucket in "D_{xx}" requires special translation.
//  "D_{x^{r}y^{n-r}}"

TNODE* LaTeX2MMLTree::TranslateDDsubscript( TNODE* TeX_subscript_contents,
										    TNODE** out_of_flow_list ) {

  TCI_BOOL do_bindings  =  FALSE;
  U16 tex_nodes_done,loc_error;
  TNODE* local_oof_list =  NULL;
  TNODE* mml_rv =  TranslateMathList( TeX_subscript_contents,do_bindings,
						NULL,tex_nodes_done,loc_error,&local_oof_list );
  mml_rv =  HandleOutOfFlowObjects( mml_rv,
  						          &local_oof_list,out_of_flow_list,2 );
  if ( mml_rv ) {
    mml_rv  =  BindScripts( mml_rv );

// It remains to insert invisible commas between the mml nodes.

    TNODE* rover  =  mml_rv;
    while ( rover && rover->next ) {
      TNODE* mo_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );	// <mo>
      SetChData( mo_node,(U8*)entity_ic,(U8*)entity_ic_unicode );
      SetDetailNum( mo_node,DETAILS_form,COPF_INFIX );
      SetDetailNum( mo_node,DETAILS_precedence,2 );

      mo_node->prev   =  rover;
      mo_node->next   =  rover->next;
      rover->next->prev =  mo_node;
      rover->next =  mo_node;

      rover =  mo_node->next;
    }
  }

  return mml_rv;
}


// \,<uID9.1.6> OR \;<uID9.1.7>

TCI_BOOL LaTeX2MMLTree::IsTeXThinOrThickSpace( TNODE* LaTeX_node ) {

  TCI_BOOL rv =  FALSE;
  if ( LaTeX_node ) {
    U16 uobj,usub,uID;
    GetUids( LaTeX_node->zuID,uobj,usub,uID );
	if ( uobj == 9 && usub==1 ) {
      if ( uID==6 || uID==7 )     // \, OR \;
        rv  =  TRUE;
	}
  }

  return rv;
}



// The following is only partially implemented.

/*
Name              values                                                    default 
align             (top | bottom | center | baseline | axis) [ rownumber ]   axis 
rowalign          (top | bottom | center | baseline | axis) +               baseline 
columnalign       (left | center | right) +                                 center 
groupalign        group-alignment-list-list                                 {left} 
alignmentscope    (true | false) +                                          true 
columnwidth       ( auto | number h-unit | namedspace | fit ) +             auto 
width             auto | number h-unit                                      auto 
rowspacing        ( number v-unit ) +                                       1.0ex 
columnspacing     ( number h-unit | namedspace ) +                          0.8em 
rowlines          (none | solid | dashed) +                                 none 
columnlines       (none | solid | dashed) +                                 none 
frame             none | solid | dashed                                     none 
framespacing      (number h-unit | namedspace) (number v-unit | namedspace) 0.4em 0.5ex 
equalrows         true | false                                              false 
equalcolumns      true | false                                              false 
displaystyle      true | false                                              false 
side              left | right | leftoverlap | rightoverlap                 right 
minlabelspacing   number h-unit                                             0.8em 
*/

void LaTeX2MMLTree::SetMTableAttribs( TNODE* mtable,TCI_BOOL is_tabular,
								        TNODE* tex_tabularORarray_node ) {

// External vertical alignment - [t|b], "center" if not specified.
// Note that the default in LaTeX is "center"
//  but the default in MathML is "axis" (not even a LaTeX option).
// Hence, to get a literal translation of the LaTeX,
//  we must ALWAYS script <mtable align="center|top|bottom"..

  U8* zalign_val  =  (U8*)"axis";

// _AXALIGN_OPTPARAM(5.35.2,MEXTALIGN)
// _TXALIGN_OPTPARAM(5.490.2,MEXTALIGN)

  U16 usubtype  =  is_tabular ? 490 : 35;
	U8 ext_align_zuID[32];
  UidsTozuID( 5,usubtype,2,(U8*)ext_align_zuID );

  TNODE* TeX_ext_align  =  FindObject( tex_tabularORarray_node->parts,
                                (U8*)ext_align_zuID,INVALID_LIST_POS );
  if ( TeX_ext_align && TeX_ext_align->contents ) {
// align  (top | bottom | center | baseline | axis) [ rownumber ]   axis
    U16 uobj,usub,uID;
    GetUids( TeX_ext_align->contents->zuID,uobj,usub,uID );
    if ( uobj==11 && usub==4 ) {
      if      ( uID== 1 )
        zalign_val  =  (U8*)"top";
      else if ( uID== 3 )
        zalign_val  =  (U8*)"bottom";
    } else
      TCI_ASSERT(0);
  }

// Next, look at hlines

// NOTE : MathML doesn't implement much in the way of rules in tables.
//  At the outer boundaries, you can have a complete frame, or no framing.
//  Options for interior rules are "solid | dashed | none".
//  There is nothing like \cline[]. All rules are table level attributes.

  U16 top_lines,bottom_lines;
  U8* rowlines =  GetMtableRowLines( tex_tabularORarray_node,
                                is_tabular,top_lines,bottom_lines );

// Next, look at the TeX {cols}

  TNODE* TeX_cols =  FindObject( tex_tabularORarray_node->parts,
                                (U8*)"5.490.111",INVALID_LIST_POS );

  U8* columnlines   =  NULL;
  U8* c_align_attrs =  NULL;
  U16 left_lines,right_lines;
  if ( TeX_cols )
    GetMtableColumnAttrs( TeX_cols,&columnlines,&c_align_attrs,
						 	                  left_lines,right_lines );
  else {
    left_lines  =  0;
    right_lines =  0;
  }

// Attach attributes to <mtable> node

  SetNodeAttrib( mtable,(U8*)"align",zalign_val );

  if ( top_lines + bottom_lines + left_lines + right_lines > 1 )
    SetNodeAttrib( mtable,(U8*)"frame",(U8*)"solid" );

  if ( rowlines ) {
    SetNodeAttrib( mtable,(U8*)"rowlines",rowlines );
    delete rowlines;
  }

  if ( columnlines ) {
    SetNodeAttrib( mtable,(U8*)"columnlines",columnlines );
    delete columnlines;
  }
  if ( c_align_attrs ) {
    SetNodeAttrib( mtable,(U8*)"columnalign",c_align_attrs );
    delete c_align_attrs;
  }

}


// During the "InsertApplyFunction" pass, we take a look
//  at any node that is followed by a delimited group
//  that might represent a function argument list.
// If the node could represent a function, we record
//  it's "name" in our list of known functions.

TCI_BOOL LaTeX2MMLTree::MMLNodeCouldBeFunction( TNODE* mml_node,
									        U8* function_nom,U16 limit ) {

  TCI_BOOL rv =  FALSE;
  if ( mml_node && function_nom ) {
    function_nom[0] =  0;

    U16 uobj,usub,uID;
    GetUids( mml_node->zuID,uobj,usub,uID );
    if        ( uobj == 3 ) {		// atomic object
      if ( usub==201 && uID==1 )      		// mi<uID3.201.1>
        AppendToFnom( function_nom,limit,mml_node->var_value );
	} else if ( uobj == 5 ) {		// structured object
      if ( usub==750 && uID==1 ) {	// <mrow>
        if ( mml_node->details
        &&   mml_node->details->function_status != UNDEFINED_DETAIL )
          TCI_ASSERT(0);
        else {
          TNODE* cont =  mml_node->parts->contents;
          ExtractFuncNameFromMrow( function_nom,limit,cont );
        }
      } else if ( usub>=50 && usub<=55 && uID==2 ) {	// scripted, etc.
        GetScriptedFuncName( mml_node,usub,function_nom,limit );
      } else {
      }
    }

    if ( function_nom[0] )
      rv  =  TRUE;
  }		// if ( mml_node && function_nom )

  return rv;
}


void LaTeX2MMLTree::AppendToFnom( U8* zdest,U16 limit,U8* zstr ) {

  if ( zdest && zstr ) {
    U16 off =  strlen( (char*)zdest );
    U16 zln =  strlen( (char*)zstr );
    if ( off+zln < limit-1 )
	    strcat( (char*)zdest,(char*)zstr );

	  else
	    TCI_ASSERT(0);
  }
}


void LaTeX2MMLTree::ExtractFuncNameFromMrow( U8* func_nom,U16 limit,
		                                	      TNODE* mrow_cont_list ) {

  func_nom[0]  =  0;
  TCI_BOOL ok =  TRUE;

// Step into an msytle element, if necessary.
// mstyle<uID5.600.0>!mstyle!BUCKET(5.600.2,MATH,,,/mstyle,)!/mstyle!

  U16 uobj,usub,id;
  GetUids( mrow_cont_list->zuID,uobj,usub,id );
  if ( uobj==5 && usub==600 && id==0 ) {
	  if ( mrow_cont_list->details
	  &&	 mrow_cont_list->details->style != UNDEFINED_DETAIL ) {
      U8 style_nom[32];
	    StyleIDtoStr( mrow_cont_list->details->style,style_nom );
      AppendToFnom( func_nom,limit,style_nom );
	  } else {
	    TCI_ASSERT(0);
      AppendToFnom( func_nom,limit,(U8*)"(stylized)" );
	  }
	  mrow_cont_list =  mrow_cont_list->parts->contents;
  }

// Check for mfrac
// mfrac<uID5.1.0>!mfrac!reqELEMENT(5.1.1)

  U16 uobjtype,usubtype,uID;
  GetUids( mrow_cont_list->zuID,uobjtype,usubtype,uID );
  if ( uobjtype==5 && usubtype==1 && uID==0 ) {
	  TNODE* numerator_cont =  mrow_cont_list->parts->contents;
    U16 uobj,usub,id;
    GetUids( numerator_cont->zuID,uobj,usub,id );
    if ( uobj==5 && usub==750 && id==1 ) {		// <mrow>
	    TNODE* mrow_cont  =  numerator_cont->parts->contents;
      GetUids( mrow_cont->zuID,uobj,usub,id );
      if ( uobj==3 && usub==203 && uID==1 ) {	// mo<uID3.203.1>
	      U8* op_name =  mrow_cont->var_value;
		    if ( !strcmp((char*)op_name,"&PartialD;") ) {
		      if ( mrow_cont->next ) {
	          TNODE* mi_func  =  mrow_cont->next;
            U16 uobj,usub,id;
            GetUids( mi_func->zuID,uobj,usub,id );
            if ( uobj==3 && usub==203 && uID==1 ) {	// mi<uID3.201.1>
              func_nom[0]  =  0;
              AppendToFnom( func_nom,limit,mi_func->var_value );
		        }      // <mi> clause
		      }
	      }     // "&Partial;" clause
	    }    // <mo> clause
	  }	// <mrow> clause

  } else {	    // start non-frac clause

    U16 tally =  0;
    TNODE* cont_rover =  mrow_cont_list;
    while ( cont_rover ) {
      tally++;
      U16 uobj,usub,id;
      GetUids( cont_rover->zuID,uobj,usub,id );
      if        ( uobj==3 && usub==201 && id==1 ) {	// <mi>
        if ( tally == 1 )
          AppendToFnom( func_nom,limit,cont_rover->var_value );
	      else {
	        ok  =  FALSE;
		      break;
	      }
	    } else if ( uobj==3 && usub==203 && id==1 ) {	// <mo>
	      if ( cont_rover->v_len==1
	      &&   cont_rover->var_value[0] == '\'' )
          AppendToFnom( func_nom,limit,(U8*)"'" );
	      else {
	        ok  =  FALSE;
	        break;
	      }
	    } else {
	      ok  =  FALSE;
	      break;
	    }

      cont_rover  =  cont_rover->next;
    }
  }
	
  if ( !ok )
    func_nom[0]  =  0;
}


// Note that we handle x_{a_1} here.

void LaTeX2MMLTree::GetScriptedFuncName( TNODE* mml_node,U16 stype,
									                  U8* identifier_nom,U16 limit ) {

  U8 zuID[32];
  UidsTozuID( 5,stype,3,(U8*)zuID );
  TNODE* base_bucket =  FindObject( mml_node->parts,
  										              zuID,INVALID_LIST_POS );
  UidsTozuID( 5,stype,4,(U8*)zuID );
  TNODE* sub_bucket  =  FindObject( mml_node->parts,
  										              zuID,INVALID_LIST_POS );
  UidsTozuID( 5,stype,5,(U8*)zuID );
  TNODE* sup_bucket  =  FindObject( mml_node->parts,
  										              zuID,INVALID_LIST_POS );

  U8 sub_join[2];		// I'm using '_' and '^' for <msub> and <msup>
  U8 sup_join[2];		//      ';' and ':' for <munder> and <mover>
  sub_join[0] =  ( stype>=50 && stype<=52 ) ? '_' : ';';
  sup_join[0] =  ( stype>=50 && stype<=52 ) ? '^' : ':';
  sub_join[1] =  0;
  sup_join[1] =  0;

  if ( base_bucket && base_bucket->contents ) {
    U16 uobj,usub,uID;
    GetUids( base_bucket->contents->zuID,uobj,usub,uID );
    if ( uobj==3 && usub==201 && uID==1 ) {		// <mi>
      AppendToFnom( identifier_nom,limit,base_bucket->contents->var_value );

      if ( sub_bucket && sub_bucket->contents ) {
        TNODE* cont =  sub_bucket->contents;
        AppendToFnom( identifier_nom,limit,sub_join );
        U16 uobj,usub,uID;
        GetUids( cont->zuID,uobj,usub,uID );
        if        ( uobj==3 && usub==201 && uID==1 ) {		// <mi>
          AppendToFnom( identifier_nom,limit,cont->var_value );
        } else if ( uobj==3 && usub==202 && uID==1 ) {		// <mn>
          AppendToFnom( identifier_nom,limit,cont->var_value );
        } else if ( uobj==5 && usub>=50 && usub<=55 && uID==2 ) {	// <msub>
          GetScriptedFuncName( cont,usub,identifier_nom,limit );
        } else if ( uobj==5 && usub==750 && uID==1 ) {		// <mrow>
          TNODE* mrow_bucket  =  FindObject( cont->parts,
  								(U8*)"5.750.2",INVALID_LIST_POS );
          if ( mrow_bucket && mrow_bucket->contents ) {
            TNODE* cont =  mrow_bucket->contents;
  	        while ( cont ) {	// loop thru <mrow>
              U16 uobj,usub,uID;
              GetUids( cont->zuID,uobj,usub,uID );
              if      ( uobj==3 && usub==201 && uID==1 )	// <mi>
                AppendToFnom( identifier_nom,limit,cont->var_value );
              else if ( uobj==3 && usub==202 && uID==1 )	// <mn>
                AppendToFnom( identifier_nom,limit,cont->var_value );
  	          cont  =  cont->next;
  	        }
  	      }
        } // jcs else		// un-handled object in script
          // jcs TCI_ASSERT(0);
      }

	    if ( sup_bucket && sup_bucket->contents ) {
	      TNODE* cont =  sup_bucket->contents;
        AppendToFnom( identifier_nom,limit,sup_join );
        U16 uobj,usub,uID;
        GetUids( cont->zuID,uobj,usub,uID );
        if        ( uobj==3 && usub==201 && uID==1 ) {	// <mi>
          AppendToFnom( identifier_nom,limit,cont->var_value );
        } else if ( uobj==3 && usub==202 && uID==1 ) {	// <mn>
          AppendToFnom( identifier_nom,limit,cont->var_value );
        } else if ( uobj==3 && usub==203 && uID==1 ) {  // <mo>
          AppendToFnom( identifier_nom,limit,cont->var_value );
        } else if ( uobj==5 && usub==750 && uID==1 ) {
	        TNODE* c_rover  =  cont->parts->contents;
	  	    while ( c_rover ) {
            U16 uobj,usub,uID;
            GetUids( c_rover->zuID,uobj,usub,uID );
            if (  ( uobj==3 && usub==201 && uID==1 ) 
            ||    ( uobj==3 && usub==202 && uID==1 )
            ||    ( uobj==3 && usub==203 && uID==1 ) )
              AppendToFnom( identifier_nom,limit,c_rover->var_value );
	  	      c_rover =  c_rover->next;
	  	    }
	      } else
	        TCI_ASSERT(0);
	    }
	  }	//  if ( uobj==3 && usub==201 && uID==1 )
  }	// if ( base_bucket with contents )
}


// Called by BindByOpPrecedence, once for each precedence level.
// All higher level bindings are completed at each call.

TCI_BOOL LaTeX2MMLTree::LocateOperand( TNODE* mml_op_node,
									    TCI_BOOL on_the_right,
  										U16& n_space_nodes,
									    U16& n_operand_nodes,
  										U16 form,
									    U16 precedence ) {

  n_space_nodes   =  0;
  n_operand_nodes =  0;

  TNODE* candidate  =  on_the_right ? mml_op_node->next : mml_op_node->prev;

// Span any space-like nodes

  I16 max_white_width =  18;
  if ( mml_op_node->details ) {
    U16 form  =  mml_op_node->details->form;
    U16 precedence  =  mml_op_node->details->precedence;
    if ( form==2 && precedence < 5 )
      max_white_width =  36;
  }


  TCI_BOOL go_on  =  TRUE;
  I16 white_width =  0;
  while ( candidate && go_on ) {
    go_on =  FALSE;
    if ( candidate->details 
    &&   candidate->details->space_width != UNDEFINED_DETAIL ) {
      white_width +=  candidate->details->space_width;
  // this could be <mspace> or <mtext> or <maligngroup>
      if ( -3 <= white_width && white_width <= max_white_width ) {
        n_space_nodes++;
        candidate  =  on_the_right ? candidate->next : candidate->prev;
        go_on =  TRUE;
      }
    }
  }


  if ( candidate ) {

    U16 uobjtype,usubtype,uID;
    GetUids( candidate->zuID,uobjtype,usubtype,uID );
	if        ( uobjtype == 3 ) {
      switch ( usubtype ) {
        case 201  :			// mi<uID3.201.1>
        case 202  :			// mn<uID3.202.1>
          n_operand_nodes =  1;		// considered to be an operand
	    break;
        case 203  : {		// mo<uID3.203.1>
          n_operand_nodes =  1;		// considered to be an operand
		}
		break;

        case 204  : 		// mtext<uID3.204.1>
    // we stepped over NARROW whitespace before getting here!
          if ( MTextIsWhiteSpace(candidate) )
		    TCI_ASSERT(0);
		  else
            n_operand_nodes =  1;		// considered to be an operand
	    break;

        case 205  : 	// mspace<uID3.205.1>
        // we stepped over mspace before getting here!
		  TCI_ASSERT(0);
	    break;
        case 206  :		// ms<uID3.206.1>
		  TCI_ASSERT(0);
		break;
		default   :
		break;
	  }

	} else if ( uobjtype == 5 ) {		// schemata

      switch ( usubtype ) {
        case 750  :		// mrow<uID5.750.1>
        case   1  :		// mfrac<uID5.1.0>
        case  35  :		// mtable<uID5.35.0>
        case  50  :		// msub<uID5.50.2>
        case  51  :		// msup<uID5.51.2>
        case  52  :		// msubsup<uID5.52.2>
        case  53  :		// munder<uID5.53.2>
        case  54  :		// mover<uID5.54.2>
        case  55  :		// munderover<uID5.55.2>
        case  56  :		// mprescripts/<uID5.56.10>
//      case  56  :		// mmultiscripts<uID5.56.2>
        case  60  :		// msqrt<uID5.60.0>
        case  61  :		// mroot<uID5.61.0>
          n_operand_nodes =  1;
	    break;

        case  70  :	{	// mfenced<uID5.70.0>
          TCI_ASSERT(0);
          n_operand_nodes =  1;
	    }
	    break;

        case 600  :		// mstyle<uID5.600.0>
          if ( candidate->details ) {
            if ( candidate->details->unit_state==1 )
              n_operand_nodes =  1;		// considered to be an operand
            else if ( candidate->details->bigop_status==1 )
              n_operand_nodes =  1;		// considered to be an operand
            else
	          TCI_ASSERT(0);
          } else
	        TCI_ASSERT(0);
	    break;
        case 601  :		// merror<uID5.601.0>
        case 602  :		// mpadded<uID5.602.0>
        case 603  :		// mphantom<uID5.603.0>
	      TCI_ASSERT(0);
	    break;

        case 604  :		// maction<uID5.604.0>
		  if ( candidate->details
		  &&   candidate->details->is_expression==1 ) {
            n_operand_nodes =  1;		// considered to be an operand
		  } else
		    TCI_ASSERT(0);
		break;

	    default   :
	      TCI_ASSERT(0);
	    break;
	  }
	}		// clause for uobjtype == 5 

  }		// if ( candidate )

  return n_operand_nodes ? TRUE : FALSE;
}


//\QTR{bf}{B}		\mathbf{B} 		    \mathbf{B}		
//\QTR{bf}{over } 	\textbf{over }	    \mathbf{over }
//\QTR{bf}{over } 	{\bf over }		    \mathbf{over }
//\QTR{em}{I hope } \emph{I hope }	    \emph{I hope }
//\QTR{em}{The end.}{\em The end.}	    \emph{The end.}
//\QTR{it}{D}		\mathit{D} 		    \mathit{D}
//\QTR{it}{quick } 	\textit{quick }	    \mathit{quick }
//\QTR{it}{quick } 	{\it quick }	    \mathit{quick }
//\QTR{md}{jumps } 	\textmd{jumps }    	\QTR{md}{jumps }
//\QTR{nf}{c}		\mathnormal{c} 		\mathnormal{c}	
//\QTR{nf}{a and }  \textnormal{a and }	\mathnormal{a and }
//\QTR{rm}{a}		\mathrm{a} 		    \mathrm{a}		
//\QTR{rm}{the } 	  {\rm the }		\mathrm{the }
//\QTR{sc}{fox } 	\textsc{fox }	    \QTR{sc}{fox }
//\QTR{sc}{fox } 	  {\sc fox }		\QTR{sc}{fox }
//\QTR{sf}{b}		\mathsf{b} 		    \mathsf{b}
//\QTR{sf}{lazy } 	\textsf{lazy }	    \mathsf{lazy }
//\QTR{sf}{lazy } 	{\sf lazy }		    \mathsf{lazy }
//\QTR{sl}{brown } 	\textsl{brown }	    \QTR{sl}{brown }
//\QTR{sl}{brown } 	{\sl brown }	    \QTR{sl}{brown }
//\QTR{tt}{C}		\mathtt{C} 		    \mathtt{C}
//\QTR{tt}{dog.} 	\texttt{dog.}	    \mathtt{dog.}
//\QTR{tt}{dog. } 	{\tt dog. }		    \mathtt{dog. }
//\QTR{up}{The } 	\textup{The }	    \QTR{up}{The } 			
//\QTR{cal}{A}		\mathcal{A} 	    \mathcal{A}
//\QTR{Bbb}{E}		\Bbb{E}			    \mathbb{E} 		
//\QTR{Bbb}{F}		\mathbb{F}		    \mathbb{F} 		
//\QTR{frak}{G}		\frak{G}		    \mathfrak{G} 	
//\QTR{frak}{H}		\mathfrak{H}	    \mathfrak{H} 	

//\QTR{group}{\QTO{md}{\md }jumps }{\md jumps }
//\QTR{group}{\QTO{up}{\up }The }{\up The }

//\QTR{group}{\QTO{displaystyle}     {\displaystyle}x}	        {\displaystyle x}
//\QTR{group}{\QTO{textstyle}        {\textstyle}y} 			{\textstyle y}
//\QTR{group}{\QTO{scriptstyle}      {\scriptstyle}z} 	        {\scriptstyle z}
//\QTR{group}{\QTO{scriptscriptstyle}{\scriptscriptstyle}123}	{\scriptscriptstyle 123}

U16 LaTeX2MMLTree::GetQTRuSubID( U8* run_name,TCI_BOOL in_math ) {

  U16 rv  =  0;

  U16 zln =  strlen( (char*)run_name );
  switch ( zln ) {
    case 2  :
	  if      ( !strcmp((char*)run_name,"bf") )
	    rv  =  in_math ? TR_Mbf : TR_Tbf;
	  else if ( !strcmp((char*)run_name,"bs") )
	    rv  =  TR_Mbs;
	  else if ( !strcmp((char*)run_name,"em") )
	    rv  =  TR_em;
	  else if ( !strcmp((char*)run_name,"it") )
	    rv  =  in_math ? TR_Mit : TR_Tit;
	  else if ( !strcmp((char*)run_name,"md") )
	    rv  =  TR_Tmd;
	  else if ( !strcmp((char*)run_name,"nf") )
	    rv  =  TR_Mnf;
	  else if ( !strcmp((char*)run_name,"rm") )
	    rv  =   in_math ? TR_Mrm : TR_Trm;
	  else if ( !strcmp((char*)run_name,"sc") )
	    rv  =  TR_Tsc;
	  else if ( !strcmp((char*)run_name,"sf") )
	    rv  =  in_math ? TR_Msf : TR_Tsf;
	  else if ( !strcmp((char*)run_name,"sl") )
	    rv  =  TR_Tsl;
	  else if ( !strcmp((char*)run_name,"tt") )
	    rv  =   in_math ? TR_Mtt : TR_Ttt;
	  else if ( !strcmp((char*)run_name,"up") )
	    rv  =  TR_Tup;
    break;

    case 3  :
	  if      ( !strcmp((char*)run_name,"cal") )
	    rv  =  TR_Mcal;
	  else if ( !strcmp((char*)run_name,"Bbb") )
	    rv  =  TR_MBbb;
    break;

    case 4  :
	  if      ( !strcmp((char*)run_name,"frak") )
	    rv  =  TR_Mfrak;
	  else if ( !strcmp((char*)run_name,"Huge") )
	    rv  =  TR_Huge;
	  else if ( !strcmp((char*)run_name,"huge") )
	    rv  =  TR_huge;
	  else if ( !strcmp((char*)run_name,"tiny") )
	    rv  =  TR_tiny;
    break;

    case 5  :
	  if      ( !strcmp((char*)run_name,"group") )
	    rv  =  TR_group;
	  else if ( !strcmp((char*)run_name,"LARGE") )
	    rv  =  TR_LARGE;
	  else if ( !strcmp((char*)run_name,"Large") )
	    rv  =  TR_Large;
	  else if ( !strcmp((char*)run_name,"large") )
	    rv  =  TR_large;
	  else if ( !strcmp((char*)run_name,"small") )
	    rv  =  TR_small;
    break;

    case 9  :
	  if      ( !strcmp((char*)run_name,"textstyle") )
        rv =  TR_textstyle;
    break;

    case 10 :
	  if      ( !strcmp((char*)run_name,"normalsize") )
	    rv  =  TR_normalsize;
	  else if ( !strcmp((char*)run_name,"scriptsize") )
	    rv  =  TR_scriptsize;
    break;

    case 11 :
	  if      ( !strcmp((char*)run_name,"scriptstyle") )
        rv =  TR_scriptstyle;
    break;

    case 12 :
	  if      ( !strcmp((char*)run_name,"footnotesize") )
	    rv  =  TR_footnotesize;
	  else if ( !strcmp((char*)run_name,"displaystyle") )
        rv =  TR_displaystyle;
    break;

    case 17 :
	  if      ( !strcmp((char*)run_name,"scriptscriptstyle") )
        rv =  TR_scriptscriptstyle;
    break;

    default :	// could be anything from file.cst
// UserInput
// MenuDialog
// KeyName
// NoteLeadin
// TutorialText
      TCI_ASSERT(0);
    break;
  }

  return rv;
}



U8* LaTeX2MMLTree::QTRuSubIDtoName( U16 sub_ID ) {

  U8* rv  =  NULL;

  switch ( sub_ID ) {
    case TR_Mbf     :  rv =  (U8*)"bf";     break;
    case TR_Mbs     :  rv =  (U8*)"bs";     break;
    case TR_em      :  rv =  (U8*)"em";     break;
    case TR_Mit     :  rv =  (U8*)"it";     break;
    case TR_Tmd     :  rv =  (U8*)"md";     break;
    case TR_Mnf     :  rv =  (U8*)"nf";     break;
    case TR_Mrm     :  rv =  (U8*)"rm";     break;
    case TR_Tsc     :  rv =  (U8*)"sc";     break;
    case TR_Msf     :  rv =  (U8*)"sf";     break;
    case TR_Tsl     :  rv =  (U8*)"sl";     break;
    case TR_Mtt     :  rv =  (U8*)"tt";     break;
    case TR_Tup     :  rv =  (U8*)"up";     break;
    case TR_Mcal    :  rv =  (U8*)"cal";    break;
    case TR_MBbb    :  rv =  (U8*)"Bbb";    break;
    case TR_Mpmb    :  rv =  (U8*)"pmb";    break;
    case TR_Mfrak   :  rv =  (U8*)"frak";   break;
    case TR_group   :  rv =  (U8*)"group";  break;

    case TR_tiny                :  rv =  (U8*)"tiny";           break;
    case TR_scriptsize          :  rv =  (U8*)"scriptsize";     break;
    case TR_footnotesize        :  rv =  (U8*)"footnotesize";   break;
    case TR_small               :  rv =  (U8*)"small";          break;
    case TR_normalsize          :  rv =  (U8*)"normalsize";     break;
    case TR_large               :  rv =  (U8*)"large";          break;
    case TR_Large               :  rv =  (U8*)"Large";          break;
    case TR_LARGE               :  rv =  (U8*)"LARGE";          break;
    case TR_huge                :  rv =  (U8*)"huge";           break;
    case TR_Huge                :  rv =  (U8*)"Huge";           break;

    case TR_displaystyle        :  rv =  (U8*)"displaystyle";       break;
    case TR_textstyle           :  rv =  (U8*)"textstyle";		    break;
    case TR_scriptstyle         :  rv =  (U8*)"scriptstyle";		break;
    case TR_scriptscriptstyle   :  rv =  (U8*)"scriptscriptstyle";  break;

    default :
      TCI_ASSERT(0);
    break;
  }

  return rv;
}


void LaTeX2MMLTree::GetUnicodeForBoldSymbol( TNODE* mml_rover,
                                                U8* unicode ) {

  unicode[0]  =  0;

  if ( mml_rover->var_value && mml_rover->v_len==1 ) {

	U32 offset  =  100L;
    U8 ch =  mml_rover->var_value[0];
	if        ( ch>='A' && ch<='Z' ) {
	  offset  =  ch - 'A';
	} else if ( ch>='a' && ch<='z' ) {
	  offset  =  ch - 'a' + 26;
	} else
	  TCI_ASSERT(0);

	if ( offset != 100L ) {
	  U32 the_unicode =  0x1D400 + offset;
	  unicode[0]  =  '#';
	  //unicode[1]  =  'x';
	  //ultoa( the_unicode,(char*)unicode+1,10 );
	  sprintf((char*)unicode+1, "%d", the_unicode);
	}

  } else
    TCI_ASSERT(0);

}


// We've hit a {group} in MATH - does it contain a switch?
// Associate TeX switches with \taggingcmd{}s.
// ie.  map {\bf blah} to \mathbf{blah}, etc.

U16 LaTeX2MMLTree::GetMathRunTagFromTeXSwitch( TNODE* TeX_cont,
                                                U8* switch_nom ) {

  U16 rv  =  0;
  switch_nom[0] =  0;

// Here we iterate a TeX contents list, looking for the first
//  switch \cmd, ie. \bf, etc.

  TNODE* rover  =  TeX_cont;
  while ( rover && !rv ) {

    U16 uobjtype,usubtype,uID;
    GetUids( rover->zuID,uobjtype,usubtype,uID );
    if ( uobjtype==5 ) {	// all switches are of type 5

      switch ( usubtype ) {

  // font related switches

        case 180  :  // \up<uID5.180.0>		\textup<uID5.450.1>
          rv  =  TR_Tup;    break;
        case 181  :  // \it<uID5.181.0>		\textit<uID5.451.1>
          rv  =  TR_Mit;    break;
        case 182  :  // \sl<uID5.182.0>		\textsl<uID5.452.1>
          rv  =  TR_Tsl;    break;
        case 183  :  // \sc<uID5.183.0>		\textsc<uID5.453.1>
          rv  =  TR_Tsc;    break;
        case 184  :  // \md<uID5.184.0>		\textmd<uID5.454.1>
          rv  =  TR_Tmd;    break;
        case 185  :  // \bf<uID5.185.0>		\textbf<uID5.455.1>
          rv  =  TR_Mbf;    break;
        case 186  :  // \rm<uID5.186.0>		\textrm<uID5.456.1>
          rv  =  TR_Mrm;    break;
        case 187  :  // \sf<uID5.187.0>		\textsf<uID5.457.1>
          rv  =  TR_Msf;    break;
        case 188  :  // \tt<uID5.188.0>		\texttt<uID5.458.1>
          rv  =  TR_Mtt;    break;
        case 189  :  // \em<uID5.189.0>		\emph<uID5.459.1>
          rv  =  TR_em;     break;

  // \QTO

        case 401  :  {  // \QTO<uID5.401.0> can encapsulate a switch
	        TNODE* nom_bucket =  FindObject( rover->parts,
                                (U8*)"5.401.1",INVALID_LIST_POS );
          if ( nom_bucket ) {
            rv  =  GetQTRuSubID( nom_bucket->contents->var_value,TRUE );
            if ( rv ) 
              strcpy( (char*)switch_nom,
            		(char*)nom_bucket->contents->var_value );
          } else
            TCI_ASSERT(0);
        }
        break;

  // size switches

        case 480  :  // \tiny<uID5.480.0>
          rv  =  TR_tiny;             break;        
        case 481  :  // \scriptsize<uID5.481.0>
          rv  =  TR_scriptsize;       break;
        case 482  :  // \footnotesize<uID5.482.0>
          rv  =  TR_footnotesize;     break;
        case 483  :  // \small<uID5.483.0>
          rv  =  TR_small;            break;
        case 484  :  // \normalsize<uID5.484.0>
          rv  =  TR_normalsize;       break;
        case 485  :  // \large<uID5.485.0>
          rv  =  TR_large;            break;
        case 486  :  // \Large<uID5.486.0>
          rv  =  TR_Large;            break;
        case 487  :  // \LARGE<uID5.487.0>
          rv  =  TR_LARGE;            break;
        case 488  :  // \huge<uID5.488.0>
          rv  =  TR_huge;             break;
        case 489  :  // \Huge<uID5.489.0>
          rv  =  TR_Huge;             break;

  // style switches

        case 600  :  // \displaystyle<uID5.600.1>
          rv  =  TR_displaystyle;			break;
        case 601  :  // \textstyle<uID5.601.1>
          rv  =  TR_textstyle;			  break;
        case 602  :  // \scriptstyle<uID5.602.1>
          rv  =  TR_scriptstyle;			break;
        case 603  :  // \scriptscriptstyle<uID5.603.1>
          rv  =  TR_scriptscriptstyle;break;

        default   :
        break;
      }

    }		// if ( uobjtype==5 )

    rover =  rover->next;
  }	// loop thru TeX_cont;


  if ( rv && switch_nom[0] == 0 ) {
    U8* tmp =  QTRuSubIDtoName( rv );
    if ( tmp )
      strcpy( (char*)switch_nom,(char*)tmp );
  }


  return rv;
}


// We're at ".", and we're looking for "..."

TCI_BOOL LaTeX2MMLTree::IsImpliedEllipsis( TNODE* TeX_dot_node ) {

  TCI_BOOL rv =  FALSE;

  if ( TeX_dot_node->next && TeX_dot_node->next->next ) {
	  U8* zuID  =  TeX_dot_node->zuID;
	  U8* n_zuID  =  TeX_dot_node->next->zuID;
	  U8* nn_zuID   =  TeX_dot_node->next->next->zuID;
    if ( !strcmp((char*)zuID,(char*)n_zuID) )
      if ( !strcmp((char*)zuID,(char*)nn_zuID) )
        rv  =  TRUE;
  }

  return rv;
}



// Handle a_{ij}, m_{12}, etc.
// Each letter or digit represents a separate <mi> or <mn>,
//   which we separate with invisible commas.

TNODE* LaTeX2MMLTree::SubscriptBody2MML( TNODE* TeX_sub_contents,
								        MATH_CONTEXT_INFO* m_context ) {

  TNODE* mml_rv =  NULL;

// Count digits and letters in the subscript

  TCI_BOOL  do_it =  TRUE;

  U16 n_sletters  =  0;
  U16 n_cletters  =  0;
  U16 n_digits    =  0;

  TNODE* rover  =  TeX_sub_contents;
  while ( rover ) {
    U16 uobj,usub,id;
    GetUids( rover->zuID,uobj,usub,id );
    if ( uobj==3 ) {
      if        ( usub==1 )	{   // small letter
        n_sletters++;
      } else if ( usub==2 ) {   // cap letter
        n_cletters++;
      } else if ( usub==3 ) {   // digit
        n_digits++;
      } else {
        do_it =  FALSE;
        break;
      }
    } else {
      do_it =  FALSE;
      break;
    }
    rover =  rover->next;
  }

  if ( do_it ) {    // subscript is all digits and/or letters
    TNODE* tail =  NULL;

    U8 v_v[2];
    v_v[1]  =  0;

    TNODE* rover  =  TeX_sub_contents;
    while ( rover ) {
      TNODE* new_node =  NULL;
      U16 uobj,usub,id;
      GetUids( rover->zuID,uobj,usub,id );
      if        ( uobj==3 && usub==1  ) {	// small letter
        new_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.201.1" );	// <mi>
        v_v[0]  =  'a' + id - 1;
        SetChData( new_node,v_v,NULL );
      } else if ( uobj==3 && usub==2 ) {	// cap letter
        new_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.201.1" );	// <mi>
        v_v[0]  =  'A' + id - 1;
        SetChData( new_node,v_v,NULL );
      } else if ( uobj==3 && usub==3 ) {	// digit
        new_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.202.1" );	// <mn>
        v_v[0]  =  '0' + id - 1;
        SetChData( new_node,v_v,NULL );
      }

      if ( new_node ) {
        if ( mml_rv ) {
          TNODE* mo_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );	// <mo>
          SetChData( mo_node,(U8*)entity_ic,(U8*)entity_ic_unicode );
          SetDetailNum( mo_node,DETAILS_form,COPF_INFIX );
          SetDetailNum( mo_node,DETAILS_precedence,2 );
          tail->next    =  mo_node;
          mo_node->prev =  tail;
          tail  =  mo_node;

          tail->next  =  new_node;
          new_node->prev  =  tail;
        } else
          mml_rv  =  new_node;

        tail  =  new_node;
      }

      rover =  rover->next;
    }

  }	else {	// if ( do_it )
// a_{n-1}  J_{n}_{1}  K_{mathbf{u}}
//  TCI_ASSERT(0);
  }

  return mml_rv;
}



// The following function decides whether some LaTeX objects
//  represent "operators" or "functions".

TCI_BOOL LaTeX2MMLTree::FuncIsMMLOperator( U16 subtype,U16 uID ) {

  TCI_BOOL rv =  FALSE;

  if ( subtype==2 ) {	// LaTeX built-in, no limits
    switch ( uID ) {
      case 27 :	// \bmod<uID8.2.27>
      case  1 :	// \arccos<uID8.2.1>
      case  2 :	// \arcsin<uID8.2.2>
      case  3 :	// \arctan<uID8.2.3>
      case  4 :	// \arg<uID8.2.4>
      case  5 :	// \cos<uID8.2.5>
      case  6 :	// \cosh<uID8.2.6>
      case  7 :	// \cot<uID8.2.7>
      case  8 :	// \coth<uID8.2.8>
      case  9 :	// \csc<uID8.2.9>
      case 10 :	// \deg<uID8.2.10>
      case 11 :	// \dim<uID8.2.11>
      case 12 :	// \exp<uID8.2.12>
      case 13 :	// \hom<uID8.2.13>
      case 14 :	// \ker<uID8.2.14>
      case 15 :	// \lg<uID8.2.15>
      case 16 :	// \ln<uID8.2.16>
      case 17 :	// \log<uID8.2.17>
      case 18 :	// \sec<uID8.2.18>
      case 19 :	// \sin<uID8.2.19>
      case 20 :	// \sinh<uID8.2.20>
      case 21 :	// \tan<uID8.2.21>
      case 22 :	// \tanh<uID8.2.22>
	    rv  =  FALSE;			break;
	  default :
	    TCI_ASSERT(0);
	  break;
    }

  } else if ( subtype==4 ) {	// LaTeX built-in, with limits

    switch ( uID ) {
      case  1 :	// \det<uID8.4.1>!\det!_LIMPLACE__FIRSTLIM__SECONDLIM_
      case 10 :	// \Pr<uID8.4.10>
	    rv  =  FALSE;			break;

      case  2 :	// \gcd<uID8.4.2>
      case  3 :	// \inf<uID8.4.3>
      case  4 :	// \injlim<uID8.4.4>
      case  5 :	// \lim<uID8.4.5>
      case  6 :	// \liminf<uID8.4.6>
      case  7 :	// \limsup<uID8.4.7>
      case  8 :	// \max<uID8.4.8>
      case  9 :	// \min<uID8.4.9>
      case 11 :	// \projlim<uID8.4.11>
      case 12 :	// \sup<uID8.4.12>
      case 13 :	// \varinjlim<uID8.4.13>
      case 14 :	// \varliminf<uID8.4.14>
      case 15 :	// \varlimsup<uID8.4.15>
      case 16 :	// \varprojlim<uID8.4.16>
	    rv  =  TRUE;			break;

	  default :
	    TCI_ASSERT(0);
	  break;
    }

  } else
    TCI_ASSERT(0);

  return rv;
}


// \limfunc<uID8.3.1>!\limfunc!REQPARAM(8.3.2,NONLATEX)
//  _LIMPLACE__FIRSTLIM__SECONDLIM_

TNODE* LaTeX2MMLTree::LimFunc2MML( TNODE* tex_limfunc_node,
				  			  		U8* limfunc_name,
					  			  	U16 usubtype,U16 uID,
					  			  	TNODE** out_of_flow_list ) {

  TNODE* rv =  NULL;

// Determine the TeX limit placement attribute of this limfunc

  TNODE* lfunc_parts  =  tex_limfunc_node->parts;
  TCI_BOOL tex_limits     =  FALSE;
  TCI_BOOL tex_nolimits   =  FALSE;
  TCI_BOOL tex_autolimits =  FALSE;
  if      ( FindObject(lfunc_parts,(U8*)"15.2.1",INVALID_LIST_POS) )
    tex_limits     =  TRUE;
  else if ( FindObject(lfunc_parts,(U8*)"15.2.2",INVALID_LIST_POS) )
    tex_nolimits   =  TRUE;
  else
    tex_autolimits =  TRUE;

// Special handling for \varlimsup, etc.

  TCI_BOOL is_varlim  =  FALSE;
  if ( usubtype==4 && uID >= 13 && uID <= 16 )
    is_varlim  =  TRUE;

// if tex_limits, limits are always under/over		- <munderover>
// if tex_nolimits, limits are always at the right  - <msubsup>
// if tex_autolimits, limits are under/over         - <munderover>
//   in a display, and at the right for inline math.

  TNODE* mml_ll =  NULL;

// Locate the TeX lower limit.
  TNODE* tex_sub  =  FindObject( lfunc_parts,(U8*)"5.50.1",	// _
									INVALID_LIST_POS );
  if ( tex_sub ) {
    if ( tex_sub->parts ) {
      TNODE* tex_ll_cont  =  tex_sub->parts->contents;
      TCI_BOOL do_bindings  =  TRUE;
      U16 tex_nodes_done,loc_error;
      TNODE* local_oof_list =  NULL;
      mml_ll =  TranslateMathList( tex_ll_cont,do_bindings,
				NULL,tex_nodes_done,loc_error,&local_oof_list );
      if ( !mml_ll )
        mml_ll  =  MakeSmallmspace();
      else if ( mml_ll->next )
        mml_ll  =  MMLlistToMRow( mml_ll );
      mml_ll =  HandleOutOfFlowObjects( mml_ll,
      					        &local_oof_list,out_of_flow_list,2 );
	}
  } else {  // We may have a multiline lower limit
    TNODE* tex_sb =  FindObject( lfunc_parts,(U8*)"5.30.0",	// \Sb
									INVALID_LIST_POS );
    if ( tex_sb )
      mml_ll  =  SbAndSp2MML( tex_sb,out_of_flow_list );
  }

  TNODE* mml_ul =  NULL;

// Locate the TeX upper limit.
  TNODE* tex_sup  =  FindObject( lfunc_parts,(U8*)"5.51.1",
								    INVALID_LIST_POS );
  if ( tex_sup ) {
    if ( tex_sup->parts ) {
      TNODE* tex_ul_cont  =  tex_sup->parts->contents;
      TCI_BOOL do_bindings  =  TRUE;
      U16 tex_nodes_done,loc_error;
      TNODE* local_oof_list =  NULL;
      mml_ul =  TranslateMathList( tex_ul_cont,do_bindings,
				NULL,tex_nodes_done,loc_error,&local_oof_list );
      if ( !mml_ul )
        mml_ul  =  MakeSmallmspace();
      else if ( mml_ul->next )
        mml_ul  =  MMLlistToMRow( mml_ul );
      mml_ul =  HandleOutOfFlowObjects( mml_ul,
   						          &local_oof_list,out_of_flow_list,2 );
    }
  } else {  // we may have a multiline upper limit
    TNODE* tex_sp =  FindObject( lfunc_parts,(U8*)"5.31.0",
									INVALID_LIST_POS );
    if ( tex_sp )
      mml_ul  =  SbAndSp2MML( tex_sp,out_of_flow_list );
  }

// If we have limits, return <msub>, <msup> or <msubsup>,
//   <munder>, <mover> or <munderover>
//   with the first arg, the base, an <mo>.
// If there are no limits, it's hard to decide whether
//   our limfunc represents a prefix operator,
//   a function, or a variable.

  TCI_BOOL is_function  =  FALSE;
  TCI_BOOL is_variable  =  FALSE;
  if ( !mml_ll && !mml_ul ) {	// we have NO limit(s)

	if        ( usubtype==4 && FuncIsMMLOperator(usubtype,uID) ) {
// gcd, etc.
	} else if ( usubtype==3 && IsKnownOperator(limfunc_name) ) {
// div, grad, curl
	} else if ( is_varlim ) {
// it's an operator
	} else if ( IdentifierInList(vars_list,limfunc_name) ) {
      is_variable =  TRUE;
	} else {
      TNODE* nn =  tex_limfunc_node->next;
	  if ( nn ) {
        U16 objclass,subclass,id;
        GetUids( nn->zuID,objclass,subclass,id );
        if ( objclass==3 ) {
          TCI_BOOL forces_geometry;
          U16 ilk =  ClassifyTeXSymbol( nn,NULL,forces_geometry,NULL );
	      if ( ilk == MML_OPERATOR ) {
  		    TNODE* mml_op_node  =  MakeTNode( 0L,0L,0L,(U8*)NULL );
		    U16 tex_nodes_spanned;
            SetMMLOpNode( nn,tex_nodes_spanned,mml_op_node );
            U16 form  =  GetDetailNum( mml_op_node,DETAILS_form );
            if      ( form == OPF_infix )
	          is_variable =  TRUE;
            else if ( form == OPF_prefix ) {
              if ( nn->src_tok[0] == '(' )			  
                is_function =  TRUE;
            }
            DisposeTNode( mml_op_node );
		  }

        } else if ( objclass==5 && subclass==70 && id==0 ) {
          if ( nn->parts && nn->parts->var_value ) {
            if ( !strcmp((char*)nn->parts->var_value,"10.4.3") ) {
                is_function =  TRUE;
            }
          }
        }

	  } else
	    is_variable =  TRUE;
	}

  }

  if        ( is_function ) {
    rv =  MakeTNode( 0L,0L,tex_limfunc_node->src_linenum,
                                     (U8*)"3.201.1" );	// <mi>
    SetDetailNum( rv,DETAILS_function_status,1 );
    SetChData( rv,limfunc_name,NULL );
    if ( zMMLFunctionAttrs )
      SetMMLAttribs( rv,(U8*)zMMLFunctionAttrs );

  } else if ( is_variable ) {
    vars_list =  AddIdentifier( vars_list,limfunc_name );
    rv =  MakeTNode( 0L,0L,tex_limfunc_node->src_linenum,
                                     (U8*)"3.201.1" );	// <mi>
    rv->var_value  =  limfunc_name;
    rv->v_len      =  strlen( (char*)limfunc_name );
    if ( zMMLFunctionAttrs )
      SetMMLAttribs( rv,(U8*)zMMLFunctionAttrs );
    SetDetailNum( rv,DETAILS_is_expression,1 );

  } else {			// prefix operator clause

    TNODE* base_op =  NULL;
    base_op  =  MakeTNode( 0L,0L,tex_limfunc_node->src_linenum,
                                  		(U8*)"3.203.1" );	// <mo>

    base_op->var_value  =  limfunc_name;
    base_op->v_len      =  strlen( (char*)limfunc_name );
    if ( zMMLFunctionAttrs )
      SetMMLAttribs( base_op,(U8*)zMMLFunctionAttrs );

	if ( is_varlim ) {
// \varlimsup, etc.
      SetChData( base_op,(U8*)"lim",NULL );
      base_op =  DecorateVarLim( base_op,uID );
	} else
      SetNodeAttrib( base_op,(U8*)"form",(U8*)zop_forms[OPF_prefix] );


    if ( mml_ll || mml_ul ) {	// we have limit(s)
      U16 mml_selem_type;
      U16 mml_ll_uID  =  0;
      U16 mml_ul_uID  =  0;
      if ( tex_limits || tex_autolimits ) {	// under and/or over

        if ( is_varlim ) {

          if ( mml_ll && mml_ul ) {
            mml_selem_type  =  in_display ? 55 : 52;	// <munderover> : <msubsup>
            mml_ll_uID  =  4;
            mml_ul_uID  =  5;
          } else if ( mml_ll ) {
            mml_selem_type  =  in_display ? 53 : 50;	// <munder> : <msub>
            mml_ll_uID  =  4;
	      } else {
            mml_selem_type  =  in_display ? 54 : 51;	// <mover> : <msup>
            mml_ul_uID  =  5;
	      }

        } else {
          if ( mml_ll && mml_ul ) {
            mml_selem_type  =  55;	// <munderover>
            mml_ll_uID  =  4;
            mml_ul_uID  =  5;
          } else if ( mml_ll ) {
            mml_selem_type  =  53;	// <munder>
            mml_ll_uID  =  4;
	      } else {
            mml_selem_type  =  54;	// <mover>
            mml_ul_uID  =  5;
	      }

	      U8* attr_nom  =  (U8*)"movablelimits";

          if ( tex_limits ) {	// under and/or over
            SetNodeAttrib( base_op,attr_nom,(U8*)"false" );
          } else {    // auto_limits
            if ( renderer_implements_displays )
              SetNodeAttrib( base_op,attr_nom,(U8*)"true" );
            else {
              if ( in_display )
                SetNodeAttrib( base_op,attr_nom,(U8*)"false" );
              else
                SetNodeAttrib( base_op,attr_nom,(U8*)"true" );
            }
          }
        }

	  } else {						// \nolimits - sub and/or sup
        if ( mml_ll && mml_ul ) {
          mml_selem_type  =  52;	// <msubsup>
          mml_ll_uID  =  4;
          mml_ul_uID  =  5;
        } else if ( mml_ll ) {
          mml_selem_type  =  50;	// <msub>
          mml_ll_uID  =  4;
	    } else {
          mml_selem_type  =  51;	// <msup>
          mml_ul_uID  =  5;
	    }
	  }



// Construct rv
      rv  =  CreateElemWithBucketAndContents( 5,mml_selem_type,2,
			  									3,base_op );
	  TNODE* parts_tail =  rv->parts;
      if ( mml_ll ) {		// construct lower limit bucket
        TNODE* next_part  =  CreateBucketWithContents(
							    5,mml_selem_type,mml_ll_uID,mml_ll );
        parts_tail->next  =  next_part;
        next_part->prev   =  parts_tail;
	      parts_tail  =  next_part;
	  }
      if ( mml_ul ) {		// construct upper limit bucket
        TNODE* next_part  =  CreateBucketWithContents(
							    5,mml_selem_type,mml_ul_uID,mml_ul );
        parts_tail->next  =  next_part;
        next_part->prev   =  parts_tail;
	  }

    } else		// there are no limits
	  rv  =  base_op;

    SetDetailNum( rv,DETAILS_form,1 );
    SetDetailNum( rv,DETAILS_precedence,29 );
  }		// prefix operator clause
   
  return rv;
}



/*
\Sb<uID5.30.0>!\Sb!IF(MATH,?{?,6.1.0)ELSE(_SBEXTERNAL_)ifEND
_SBEXTERNAL_LIST(5.30.1,BUCKET(5.30.3,MATH,,,\\|\endSb,),5.30.2,,\endSb,\\)!\endSb!

\Sp<uID5.31.0>!\Sp!IF(MATH,?{?,6.1.0)ELSE(_SPEXTERNAL_)ifEND
_SPEXTERNAL_LIST(5.31.1,BUCKET(5.31.3,MATH,,,\\|\endSp,),5.31.2,,\endSp,\\)!\endSp!

We generate an <mtable> with 0 row and column spacing.
*/

TNODE* LaTeX2MMLTree::SbAndSp2MML( TNODE* SborSp_node,
									                  TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  U16 uobjtype,usubtype,uID;
  GetUids( SborSp_node->zuID,uobjtype,usubtype,uID );
	U8 list_zuID[32];
  UidsTozuID( 5,usubtype,1,(U8*)list_zuID );

  TNODE* line_list  =  FindObject( SborSp_node->parts,
   								              (U8*)list_zuID,INVALID_LIST_POS );

  TNODE* first_entry  =  NULL;
  if ( line_list ) {
    first_entry  =  line_list->parts;
  } else {
    TNODE* sb_group =  FindObject( SborSp_node->parts,
   								              (U8*)"6.1.0",INVALID_LIST_POS );
  // \Sb{\CELL{}\CELL{}...}
    if ( sb_group && sb_group->parts && sb_group->parts->contents )
      first_entry   =  sb_group->parts->contents;
  }

  if ( first_entry ) {
// Generate a list of <mtr>'s
    TNODE* head   =  NULL;
    TNODE* tail;

    U16 row_count =  0;
    TNODE* cell_rover =  first_entry;
    while ( cell_rover ) {     // loop down thru rows
      TNODE* line_contents  =  NULL;
      U16 uobj,usub,id;
      GetUids( cell_rover->zuID,uobj,usub,id );
      if ( (uobj==6 && usub==1  && id==0) 		// {group}
      ||   (uobj==5 && usub==usubtype && id==1) ) {
  // Translate current line to MathML
	      TNODE* cont =  cell_rover->parts->contents;
        U16 tex_nodes_done,error_code;
  	    TCI_BOOL do_bindings  =  TRUE;
        TNODE* local_oof_list =  NULL;
        line_contents =  TranslateMathList( cont,do_bindings,
               		  NULL,tex_nodes_done,error_code,&local_oof_list );

        U16 vspace_context  =  2;
        // 1 - pass vspace up to caller
        // 2 - nest mml_cont in <mpadded>

        line_contents =  HandleOutOfFlowObjects( line_contents,
        						&local_oof_list,out_of_flow_list,vspace_context );
  // mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd)!/mtd!
        line_contents =  FixImpliedMRow( line_contents );
        TNODE* cell =  CreateElemWithBucketAndContents( 5,35,14,8,
        											line_contents );

  // Put the current cell under a cell list item node
	      TNODE* list_node  =  MakeTNode( 0,0,0,(U8*)"5.35.11:0" );
	      list_node->parts  =  cell;
	      cell->sublist_owner =  list_node;

  // Add current row to the list of rows we're building
	// mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
	// _LISTOFCELLS_LIST(5.35.11,_MTCELL_,5.35.12,,/mtr|/mtable,)
	// _MTCELL_IF(MATH,?mtd?,5.35.14)ifEND

        TNODE* mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );
	      mtr->parts->parts =  list_node;
	      list_node->sublist_owner =  mtr->parts;

  // Put the current row under a node in the row list
	      U8 zlistID[32];
        UidsTozuID( 5,35,9,(U8*)zlistID );
	      U16 zln =  strlen( (char*)zlistID );
	      zlistID[zln]  =  ':';
              // itoa( row_count,(char*)zlistID+zln+1,10 );
              sprintf((char*)zlistID+zln+1, "%d", row_count);
	      TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	      row_list_node->parts  =  mtr;
	      mtr->sublist_owner  =  row_list_node;

        if ( !head )
          head  =  row_list_node;
        else {
          tail->next =  row_list_node;
	        row_list_node->prev =  tail;
	      }
        tail  =  row_list_node;

  // Advance to the next row in the LaTeX source tree
        row_count++;

      } else if ( uobj==5 && usub==39 && id==5 ) {

// \CELL<uID5.39.5> - Nothing to do here.

	    } else
        TCI_ASSERT(0);

      cell_rover =  cell_rover->next;
    }

// mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
// _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)

    mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
    SetNodeAttrib( mml_rv,(U8*)"rowspacing",(U8*)"0" );
    SetNodeAttrib( mml_rv,(U8*)"columnspacing",(U8*)"0" );

    mml_rv->parts->parts  =  head;
    head->sublist_owner =  mml_rv->parts;

  } else
    TCI_ASSERT(0);

  return mml_rv;
}



TNODE* LaTeX2MMLTree::BindPrefixOP( TNODE* head,TNODE* MML_rover,
									              U16 curr_prec,TNODE** new_mrow ) {

  TNODE* rv =  head;
  *new_mrow =  NULL;

  TCI_BOOL is_cap_letter;
  if ( curr_prec==29
  &&   MML_rover->next
  &&   MML_rover->next->next
  &&   ( IsMMLNumber(MML_rover->next) || IsMMLFactor(MML_rover->next,FALSE,is_cap_letter) )
  &&   MML_rover->next->next->details
  &&   MML_rover->next->next->details->form==1 ) {
    rv  =  BindPrefixOP( head,MML_rover->next->next,
								curr_prec,new_mrow );

    TNODE* mo_node  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );	// <mo>
    SetChData( mo_node,(U8*)entity_it,(U8*)entity_it_unicode );
    SetDetailNum( mo_node,DETAILS_form,COPF_INFIX );
    SetDetailNum( mo_node,DETAILS_precedence,39 );
	  TNODE* prev =  MML_rover->next;
	  TNODE* next =  MML_rover->next->next;
	  mo_node->prev =  prev;
	  prev->next  =  mo_node;
	  mo_node->next =  next;
	  next->prev  =  mo_node;

    TNODE* anchor =  MML_rover;
    TNODE* first  =  MML_rover->next;
	  rv  =  NestNodesInMrow( head,anchor,first,
							  TRUE,3,new_mrow );

	  if ( new_mrow )
      SetDetailIsExpr( *new_mrow,curr_prec );
	  else {
	    TCI_ASSERT(0);
    }

  } else
  if ( MML_rover->next
  &&   MML_rover->next->details
  &&   MML_rover->next->details->form==1 )
    rv  =  BindPrefixOP( head,MML_rover->next,
								curr_prec,new_mrow );


  U16 n_space_nodes;
  U16 n_operand_nodes;
  if ( LocateOperand(MML_rover,TRUE,
  		n_space_nodes,n_operand_nodes,1,curr_prec) ) {
    if ( n_operand_nodes > 1 ) {
      TCI_ASSERT(0);
      TNODE* left_anchor  =  MML_rover;
  	  U16 tally =  n_space_nodes;
  	  while ( tally ) {
  	    left_anchor =  left_anchor->next;
  	    tally--;
  	  }
  	  rv  =  NestNodesInMrow( rv,left_anchor,NULL,TRUE,
  							                n_operand_nodes,new_mrow );
    }
    rv  =  NestNodesInMrow( rv,NULL,MML_rover,TRUE,
  							                2+n_space_nodes,new_mrow );

  } else if ( MML_rover->next ) {
    TCI_ASSERT(0);
  }

  return rv;
}



// There is a section in MathML.gmr for multiformops.

TCI_BOOL LaTeX2MMLTree::IsMultiFormOp( U8* mml_op_name,U16& forms ) {

  TCI_BOOL rv  =  FALSE;
  forms =  0;

  U16 op_ln =  strlen( (char*)mml_op_name );

  U8* zform =  (U8*)"multiform";
  U8* op_zuID;
  U8* op_ztemplate;
  if ( s_mml_grammar->GetGrammarDataFromNameAndAttrs(
					mml_op_name,op_ln,zform,(U8*)"MULTIFORMOPS",
                               		&op_zuID,&op_ztemplate) ) {
	if ( op_ztemplate ) {
	  char* ptr =  strchr( (char*)op_ztemplate,',' );
	  if ( ptr ) {
	    ptr++;
	    if ( ptr[0] == '1' )	// prefix
	      forms +=  4;
	    if ( ptr[1] == '1' )	// infix
	      forms +=  2;
	    if ( ptr[2] == '1' )	// postfix
	      forms +=  1;
	    rv  =  TRUE;
	  } else
	    TCI_ASSERT(0);
	} else
	  TCI_ASSERT(0);
  }

  return rv;
}


// During "InsertApplyFunction" we may encounter mfrac
//  followed by a group that may represent a delimited func arg.
//  df/dx(1.2)

TCI_BOOL LaTeX2MMLTree::IsFuncDerivative( TNODE* mml_node ) {

  TCI_BOOL rv =  FALSE;

  U16 uobj,usub,id;
  GetUids( mml_node->zuID,uobj,usub,id );
// mfrac<uID5.1.0>!mfrac!reqELEMENT(5.1.1)reqELEMENT(5.1.2)!/mfrac!
  if ( uobj==5 && usub==1 && id==0 ) {	// <mfrac>
    TNODE* denom  =  FindObject( mml_node->parts,
								(U8*)"5.1.2",INVALID_LIST_POS );
    if ( denom && denom->contents ) {
      TNODE* cont =  denom->contents;
      U16 uobj,usub,id;
      GetUids( cont->zuID,uobj,usub,id );
      if ( uobj==5 && usub==750 && id==1 ) {	// <mrow>
		    TNODE* mrow =  cont;
		    if ( mrow->parts && mrow->parts->contents ) {
		      TNODE* cont2  =  mrow->parts->contents;
		      if ( cont2->details
		      &&   cont2->details->is_differential != UNDEFINED_DETAIL )
		        if ( cont2->next )
		          rv  =  TRUE;
		    }
	    }
	  }

	  if ( rv ) {			// denominator is "d something"
	    rv  =  FALSE;
      TNODE* numer  =  FindObject( mml_node->parts,
								(U8*)"5.1.1",INVALID_LIST_POS );
      if ( numer && numer->contents ) {
        TNODE* cont =  numer->contents;
        U16 uobj,usub,id;
        GetUids( cont->zuID,uobj,usub,id );
        if ( uobj==5 && usub==750 && id==1 ) {	// <mrow>
		      TNODE* mrow =  cont;
		      if ( mrow->parts && mrow->parts->contents ) {
		        TNODE* cont2  =  mrow->parts->contents;
		        if ( cont2->details
		        &&   cont2->details->is_differential != UNDEFINED_DETAIL )
		          if ( cont2->next )
		            rv  =  TRUE;
		      }
		    }
	    }
	  }

  }	else if ( uobj==5 && usub==750 && id==1 ) {	// <mrow>
    TNODE* mbucket  =  FindObject( mml_node->parts,
								(U8*)"5.750.2",INVALID_LIST_POS );
    if ( mbucket && mbucket->contents && !mbucket->contents->next )
      return IsFuncDerivative( mbucket->contents );

  }	else if ( uobj==5 && usub==600 && id==0 ) {	// <mstyle>
    TNODE* mbucket  =  FindObject( mml_node->parts,
								(U8*)"5.600.2",INVALID_LIST_POS );
    if ( mbucket && mbucket->contents && !mbucket->contents->next )
      return IsFuncDerivative( mbucket->contents );

  } else if ( uobj==5 && usub==51 && id==2 ) { // <msup>
//msup<uID5.51.2>!msup!reqELEMENT(5.51.3)reqELEMENT(5.51.5)!/msup!
    TNODE* base =  FindObject( mml_node->parts,
								(U8*)"5.51.3",INVALID_LIST_POS );
    TNODE* exp  =  FindObject( mml_node->parts,
								(U8*)"5.51.5",INVALID_LIST_POS );
    if ( base
    &&   base->contents
    &&   MMLNodeIsFunction(base->contents) ) {
      rv  =  TRUE;
    } else if ( exp && exp->contents ) {
      TNODE* cont =  exp->contents;
      U16 uobj,usub,id;
      GetUids( cont->zuID,uobj,usub,id );
      if ( uobj==3 && usub==203 && id==1 ) {	// <mo>
        if ( cont->var_value ) {
          char* ptr =  strstr( (char*)cont->var_value,"&prime;" );
		  if ( ptr )
            rv  =  TRUE;
        }
	  }
    }
  }

  return rv;
}


// Handle any \hline's from \begin{tabular}.
// We pass back a zstr "solid none solid..." which becomes
//  the value an mtable's "rowlines" attribute.
// top_lines and bottom_lines are used in deciding
//  whether or not to frame the mtable.


U8* LaTeX2MMLTree::GetMtableRowLines( TNODE* TeX_tabularORarray_node,
										TCI_BOOL is_tabular,
										U16& top_lines,
										U16& bottom_lines ) {

  U8* rv  =  NULL;
  top_lines     =  0;
  bottom_lines  =  0;

  U16 usubtype  =  is_tabular ? 490 : 35;

  U8 cells_list_zuID[32];
  UidsTozuID( 5,usubtype,11,(U8*)cells_list_zuID );

  U8 rows_list_zuID[32];
  UidsTozuID( 5,usubtype,9,(U8*)rows_list_zuID );
  TNODE* TeX_row_list  =  FindObject( TeX_tabularORarray_node->parts,
                                rows_list_zuID,INVALID_LIST_POS );

  U8 buffer[512];
  U16 bi  =   0;

  TCI_BOOL has_lines  =  FALSE;

  U16 row_count =  0;
  TNODE* row_rover  =  TeX_row_list->parts;
  while ( row_rover && bi<500 ) {
    row_count++;
    TNODE* hline_bucket =  FindObject( row_rover->parts,
   							(U8*)"5.490.13",INVALID_LIST_POS );
    TNODE* cell_list    =  FindObject( row_rover->parts,
   							cells_list_zuID,INVALID_LIST_POS );

	  if        ( row_count == 1 ) {  // top of table
	    if ( hline_bucket )
          top_lines++;

	  } else if ( cell_list ) {       // interior

// This is rather crude.  I'm not looking
//  at the contents of hline_bucket, just it's existance.

	    if ( hline_bucket ) {
          has_lines  =  TRUE;
          if ( bi )
            buffer[bi++]  =  ' ';
          strcpy( (char*)buffer+bi,"solid" );
	      bi  +=  5;
	    } else {
          if ( bi )
            buffer[bi++]  =  ' ';
          strcpy( (char*)buffer+bi,"none" );
	      bi  +=  4;
	    }

	  } else {						// bottom of table

// If there \hline's at the bottom of a table,
//  we add row with no cell_list to carry them.

	    if ( hline_bucket )
          bottom_lines++;
	  }

    row_rover =  row_rover->next;
  }

  if ( !bottom_lines && bi ) {
    if ( bi )
      buffer[bi++]  =  ' ';
    strcpy( (char*)buffer+bi,"none" );
    bi  +=  4;
  }

  if ( has_lines ) {
	rv  =  (U8*)TCI_NEW( char[bi+1] );
	strcpy( (char*)rv,(char*)buffer );
  }

  return rv;
}


// \begin{array}[t]{||r|cl|}...
// Map the "cols" arg to mtable column attributes.

void LaTeX2MMLTree::GetMtableColumnAttrs( TNODE* TeX_cols_list,
										                    U8** columnlines,
										                    U8** c_align_attrs,
										                    U16& left_lines,
										                    U16& right_lines ) {

  *columnlines    =  NULL;
  *c_align_attrs  =  NULL;
  left_lines  =  0;
  right_lines =  0;

  if ( TeX_cols_list && TeX_cols_list->parts ) {

    TCI_BOOL has_lines  =  FALSE;
	  U8 vline_buffer[512];       // "solid none solid"
	  U16 ri  =  0;

    TCI_BOOL has_aligns =  FALSE;
	  U8 a_buffer[512];           // "left center right"
	  U16 ai  =  0;

    U16 column_count  =  0;
    TNODE* crover =  TeX_cols_list->parts;
	  while ( crover ) {		// loop thru columns
      column_count++;

	    if ( crover->parts && crover->parts->contents ) {
	      U8* c_alignment =  NULL;
		    U16 l_verts =  0;
		    U16 r_verts =  0;

        TNODE* cont_rover =  crover->parts->contents;
		    while ( cont_rover ) {	// loop thru cols for current column
          U16 uobj,usub,id;
          GetUids( cont_rover->zuID,uobj,usub,id );
          if ( uobj==16 && usub==4 ) {

            switch ( id ) {
              case  1  :
	              c_alignment =  (U8*)"left";
                has_aligns  =  TRUE;
              break;
              case  2  :
	              c_alignment =  (U8*)"center";
              break;
              case  3  :
	              c_alignment =  (U8*)"right";
                has_aligns  =  TRUE;
              break;
              case  4  :
                if ( c_alignment )
		              r_verts++;
                else
                  l_verts++;
              break;
// @<uID16.4.5>!@!BUCKET(16.4.50,TEXT,{,},,)
// ;;p<uID16.4.6>!p!BUCKET(16.4.51,TEXT,{,},,)
// p<uID16.4.6>!p!REQPARAM(16.4.51,DIMEN)
// *<uID16.4.7>!*!REQPARAM(16.4.52,NONLATEX)REQPARAM(16.4.53,COLS)
              default :
			          TCI_ASSERT(0);
              break;
			      }

		      } else {
		        TCI_ASSERT(0);
// {<uID6.1.0>
// }<uID6.1.1>
// ,<uID3.17.87>
// [<uID3.17.94>
// ]<uID3.17.96>
// 0<uID3.3.1>
// 1<uID3.3.2>
// 2<uID3.3.3>
// 3<uID3.3.4>
// 4<uID3.3.5>
// 5<uID3.3.6>
// 6<uID3.3.7>
// 7<uID3.3.8>
// 8<uID3.3.9>
// 9<uID3.3.10>
		      }

	        cont_rover  =  cont_rover->next;
		    }						// loop thru cols for current column

	      if ( c_alignment ) {
          if ( ai < 500 ) {
            if ( ai )
              a_buffer[ai++]  =  ' ';
	          strcpy( (char*)a_buffer+ai,(char*)c_alignment );
		      }
	        ai  +=  strlen( (char*)c_alignment );
		    } else
		      TCI_ASSERT(0);

        if ( column_count == 1 && l_verts )
          left_lines++;

		    if ( !crover->next ) {		// last column
		      if ( r_verts )
            right_lines++;
		    } else {
          if ( ri < 500 ) {
            if ( ri )
              vline_buffer[ri++]  =  ' ';
	    const char* line_attr =  r_verts ? "solid" : "none";
	    strcpy( (char*)vline_buffer+ri,line_attr );
	    ri  +=  strlen( line_attr );
	  }
          if ( r_verts )
            has_lines =  TRUE;
		    }

	    }		// if ( crover->parts && crover->parts->contents )

	    crover  =  crover->next;
	  }		// loop thru columns


    if ( has_lines ) {
	    U8* rv  =  (U8*)TCI_NEW( char[ri+1] );
	    strcpy( (char*)rv,(char*)vline_buffer );
      *columnlines    =  rv;
	  }
    if ( has_aligns ) {
	    U8* rv  =  (U8*)TCI_NEW( char[ai+1] );
	    strcpy( (char*)rv,(char*)a_buffer );
      *c_align_attrs  =  rv;
	  }

  }		// if ( TeX_cols_list && TeX_cols_list->parts )
}



TCI_BOOL LaTeX2MMLTree::IsMsup( TNODE* MML_node ) {

  TCI_BOOL rv =  FALSE;
  if ( MML_node ) {
    U16 uobj,usub,uID;			// msup<uID5.51.2>
    GetUids( MML_node->zuID,uobj,usub,uID );
	  if ( uobj==5 && usub==51 && uID==2 )
      rv  =  TRUE;
  }

  return rv;
}



// Some LaTeX nodes are extracted from contents lists
//  and placed on "out-of-flow" lists for processing later.

TNODE* LaTeX2MMLTree::MoveNodeToList( TNODE* list,TNODE* TeX_node ) {

  TNODE* owner  =  TeX_node->sublist_owner;
  TNODE* prev   =  TeX_node->prev;
  TNODE* next   =  TeX_node->next;

  TeX_node->sublist_owner =  NULL;
  TeX_node->prev  =  NULL;

  TeX_node->next  =  list;	// Notice - goes to head
  if ( list )
    list->prev =  TeX_node;

  if ( owner )
	owner->contents =  next;
  if ( next ) {
    next->prev  =  prev;
    if ( owner )
      next->sublist_owner  =  owner;
  }
  if ( prev )
    prev->next  =  next;

  return TeX_node;
}


// "\idotsint" must be identified for special translation.

TCI_BOOL LaTeX2MMLTree::IsIDotsInt( TNODE* tex_bigop_node ) {

  U16 uobj,usub,uID;
  GetUids( tex_bigop_node->zuID,uobj,usub,uID );
  if ( uobj==7 && usub==1 && uID==5 )
    return TRUE;
   else
    return FALSE;
}


// There is no MathML equivalent for TeX's "\idotsint".
// I'm generating \int...\int for now.

TNODE* LaTeX2MMLTree::IDotsInt2MML() {

//&int;<uID7.1.1>prefix,31,U0222B,largeop="true" stretchy="true"  lspace="0em" rspace="0em"
  U8 ent_int[80];
  U8 ent_int_unicode[80];
  ent_int[0]  =  0;
  ent_int_unicode[0]  =  0;
  AppendEntityToBuffer( (U8*)"7.1.1",ent_int,ent_int_unicode );

//&hellip;<uID3.17.1>,U02026
  U8 ent_dots[80];
  U8 ent_dots_unicode[80];
  ent_dots[0]  =  0;
  ent_dots_unicode[0]  =  0;
  AppendEntityToBuffer( (U8*)"3.17.1",ent_dots,ent_dots_unicode );

  TNODE* cont =  MakeTNode( 0L,0L,0L,(U8*)"3.202.1" );
  SetChData( cont,ent_int,ent_int_unicode );

  TNODE* cn	  =  MakeTNode( 0L,0L,0L,(U8*)"3.201.1" );
  SetChData( cn,ent_dots,ent_dots_unicode );
  cont->next  =  cn;
  cn->prev    =  cont;

  TNODE* cnn  =  MakeTNode( 0L,0L,0L,(U8*)"3.202.1" );
  SetChData( cnn,(U8*)ent_int,ent_int_unicode  );
  cn->next    =  cnn;
  cnn->prev   =  cn;

// Note that the caller adds operator-related details
//  to the <mrow> returned here.
  return  CreateElemWithBucketAndContents( 5,750,1,2,cont );
}


// goto's - anomalies in the present implementation.

void LaTeX2MMLTree::HyperObj2MML( TNODE** out_of_flow_list,
									TNODE* TeX_hyper_obj,
							  		U16 subclass,TCI_BOOL in_math,
							  		TNODE** node_to_insert ) {

  U16 anomaly_ID  =  0;
  U8* zuID_tbucket  =  NULL;
  switch ( subclass ) {
    case  550 :				// \hyperref<uID5.550.0>
//\hyperref<uID5.550.0>!\hyperref!OPTPARAM(5.550.5,NONLATEX)REQPARAM(5.550.6,INHERIT)_OLDARGS_
      zuID_tbucket  =  (U8*)"5.550.6";
      anomaly_ID  =  1000;
	  break;

    case  551 :				// \hyperlink<uID5.551.0>
//\hyperlink<uID5.551.0>!\hyperlink!REQPARAM(5.551.1,NONLATEX)REQPARAM(5.551.2,INHERIT)
      zuID_tbucket  =  (U8*)"5.551.2";
      anomaly_ID  =  1000;
	  break;

    case  552 :				// \hypertarget<uID5.552.0>
//\hypertarget<uID5.552.0>!\hypertarget!REQPARAM(5.552.1,NONLATEX)REQPARAM(5.552.2,INHERIT)
      zuID_tbucket  =  (U8*)"5.552.2";
      *out_of_flow_list  =  MoveNodeToList( *out_of_flow_list,TeX_hyper_obj );
	  break;

    case  553 :				// \href<uID5.553.0>
      anomaly_ID  =  1000;
	  break;

    case  554 :				// \HREFTOLABEL<uID5.554.0> internal
// \HREFTOLABEL<uID5.554.0>!\HREFTOLABEL!REQPARAM(5.554.1,NONLATEX)REQPARAM(5.554.2,TEXT)
      zuID_tbucket  =  (U8*)"5.554.2";
      anomaly_ID  =  1000;
	  break;

    case  555 :				// \HYPERLINK<uID5.555.0> internal
//\HYPERLINK<uID5.555.0>!\HYPERLINK!REQPARAM(5.555.1,NONLATEX)REQPARAM(5.555.2,TEXT)
      zuID_tbucket  =  (U8*)"5.555.2";
      anomaly_ID  =  1000;
	  break;

    case  556 :				// \HYPERTARGET<uID5.556.0>
//\HYPERTARGET<uID5.556.0>!\HYPERTARGET!REQPARAM(5.556.1,TEXT)REQPARAM(5.556.2,NONLATEX)
      zuID_tbucket  =  (U8*)"5.556.1";
      *out_of_flow_list  =  MoveNodeToList( *out_of_flow_list,TeX_hyper_obj );
	  break;

    case  557 :				// \HREF<uID5.557.0> internal
// \HREF<uID5.557.0>
// REQPARAM(5.557.1,NONLATEX)
// REQPARAM(5.557.2,NONLATEX)
// REQPARAM(5.557.3,TEXT)
      zuID_tbucket  =  (U8*)"5.557.3";
      anomaly_ID  =  1000;
	  break;

    case  558 :				// \HYPERREF<uID5.558.0>
// REQPARAM(5.558.1,NONLATEX)
// REQPARAM(5.558.2,NONLATEX)
// REQPARAM(5.558.3,NONLATEX)
// REQPARAM(5.558.4,TEXT)
      zuID_tbucket  =  (U8*)"5.558.4";
      anomaly_ID  =  1000;
	  break;

    case  559 :				// \MSIHYPERREF<uID5.559.0>
// REQPARAM(5.559.1,TEXT)     - screen
// REQPARAM(5.559.2,TEXT)     - left
// REQPARAM(5.559.3,TEXT)     - right
// REQPARAM(5.559.4,NONLATEX) - uri
// REQPARAM(5.559.5,NONLATEX) - marker
      zuID_tbucket  =  (U8*)"5.559.1";
      anomaly_ID  =  1000;
	  break;

    case  560 :				// \HYPERDEF<uID5.560.0>
// REQPARAM(5.560.1,NONLATEX)
// REQPARAM(5.560.2,NONLATEX)
// REQPARAM(5.560.3,INHERIT)
//    zuID_tbucket  =  (U8*)"5.560.3";
      anomaly_ID  =  1000;
	  break;

    default :
	    TCI_ASSERT(0);
	  break;
  }

  if ( node_to_insert && zuID_tbucket ) {
    TNODE* tbucket  =  FindObject( TeX_hyper_obj->parts,
	  						                zuID_tbucket,INVALID_LIST_POS );
	  if ( tbucket && tbucket->contents ) {
      TCI_BOOL no_line_breaks =  TRUE;
	    TNODE* mml_text =  TextInMath2MML( tbucket->contents,
  								out_of_flow_list,no_line_breaks,FALSE );
      if ( mml_text ) {
        U16 uobj,usub,id;
        GetUids( mml_text->zuID,uobj,usub,id );

	      if ( uobj==5 && usub==750 && id==1 ) {  // mrow<uID5.750.1>
          mml_text  =  FixImpliedMRow( mml_text );
          mml_text  =  CreateElemWithBucketAndContents( 5,600,0,2,mml_text );
	      }

        if ( zMMLHyperLinkAttrs )
//        SetNodeAttrib( mml_text,(U8*)"mathcolor",(U8*)zMMLHyperLinkAttrs );
          SetMMLAttribs( mml_text,(U8*)zMMLHyperLinkAttrs );
      }
      *node_to_insert =  mml_text;

	  } else
	    TCI_ASSERT(0);
  }

  if ( anomaly_ID )
    RecordAnomaly( anomaly_ID,NULL,TeX_hyper_obj->src_offset1,
       							            TeX_hyper_obj->src_offset2 );
}



TCI_BOOL LaTeX2MMLTree::MMLNodeIsOperator( TNODE* MML_node ) {

  TCI_BOOL rv =  FALSE;

  if ( MML_node ) {
    U16 uobjtype,usubtype,uID;
    GetUids( MML_node->zuID,uobjtype,usubtype,uID );
    if        ( uobjtype==3 ) {	// non-structured
      if ( usubtype==203 )
        rv =  TRUE;
    } else if ( uobjtype==5 ) {	// structured
// Here's where "embellished operators" are handled.
      if ( usubtype==750 && uID==1 ) {	// mrow
        if ( MML_node->parts && MML_node->parts->contents ) {
          TNODE* cont =  MML_node->parts->contents;
          U16 uobj,usub,id;
          GetUids( cont->zuID,uobj,usub,id );
          if ( uobj==5 && usub==600 && id==0 ) {	// mstyle
            if ( cont->parts && cont->parts->contents ) {
              TNODE* cont2  =  cont->parts->contents;
              GetUids( cont2->zuID,uobj,usub,id );
              if ( uobj==3 && usub==203 )
                rv =  TRUE;
			      }
		      }		// mstyle clause
		    } else
		      TCI_ASSERT(0);

	    }	else if ( usubtype>=50 && usubtype<=55 && uID==2 ) {
        if ( MML_node->parts && MML_node->parts->contents ) {
          TNODE* cont =  MML_node->parts->contents;
          U16 uobj,usub,id;
          GetUids( cont->zuID,uobj,usub,id );
          if ( uobj==3 && usub==203 )
            rv =  TRUE;
		    } else
		      TCI_ASSERT(0);
	    }

	  }	// uobjtype == 5 clause
  }		// if ( MML_node )

  return rv;
}


// We look for "d" used as "differential d", as in "\int xdx"

TCI_BOOL LaTeX2MMLTree::IsDifferentiald( TNODE* tex_d_node,
                                            TNODE* LaTeX_list ) {

  TCI_BOOL rv =  FALSE;

  TNODE* next_right =  tex_d_node->next;
  if ( next_right ) {
    U16 uobj,usub,id;
    GetUids( next_right->zuID,uobj,usub,id );
	  if ( uobj==5 && usub==51 && id==1 )	  // ^<uID5.51.1>
	    next_right  =  next_right->next;
	  rv  =  IsTeXVariable( next_right );
  }

// catch \frac{df}{dx} or \frac{d}{dy}

  if ( !rv && tex_d_node->sublist_owner ) {
    TNODE* mbucket  =  tex_d_node->sublist_owner;
    U16 uobj,usub,id;
    GetUids( mbucket->zuID,uobj,usub,id );
	  if ( uobj==5 && usub==1 && id==1 )	{	// numerator of a fraction
      TNODE* denom  =  FindObject( mbucket,(U8*)"5.1.2",
                                    INVALID_LIST_POS );
	    if ( denom && denom->contents ) {
        TNODE* d_cont =  denom->contents;
        U16 uobj,usub,id;
        GetUids( d_cont->zuID,uobj,usub,id );
        if ( uobj==3 && usub==1 && id==4 )  // d<uID3.1.4>
	        rv  =  IsTeXVariable( d_cont->next );
	    }
	  }
  }

  if ( !rv && LaTeX_list ) {
	  if ( TeXListHasIntegral(LaTeX_list,tex_d_node) ) {
      if ( next_right ) {
        U16 uobj,usub,id;
        GetUids( next_right->zuID,uobj,usub,id );
        if ( uobj == 3 ) {
          if ( usub == 1 || usub == 2 || usub == 10 ) {
            rv  =  TRUE;
          }
        }
      }
	  }
  }

  return rv;
}


TCI_BOOL LaTeX2MMLTree::IsDifferentialD( TNODE* tex_D_node,
                                            TNODE* LaTeX_list ) {

  TCI_BOOL rv =  FALSE;

  TNODE* next_right =  tex_D_node->next;
  if ( next_right ) {
    U16 uobjtype,usubtype,uID;
    GetUids( next_right->zuID,uobjtype,usubtype,uID );
// _<uID5.50.1>!_!REQPARAM(5.50.2,MATH)
	  if ( uobjtype==5 &&	usubtype==50 && uID==1 ) {
	    TNODE* mbucket  =  FindObject( next_right->parts,
                                (U8*)"5.50.2",INVALID_LIST_POS );
	    TNODE* cont =  mbucket->contents;
	    while ( cont ) {
	      if ( IsTeXVariable(cont) ) {
		      rv  =  TRUE;
		      if ( cont->next )
		        if ( IsPowerForm(cont->next) )
		          cont  =  cont->next;
		    } else {
		      rv  =  FALSE;
		      break;
		    }
		    cont  =  cont->next;
	    }		// while ( cont )
	  }
  }		// if ( next_right )

  return rv;
}


TCI_BOOL LaTeX2MMLTree::IsPowerForm( TNODE* tex_node ) {

  TCI_BOOL rv =  FALSE;

  U16 uobjtype,usubtype,uID;
  GetUids( tex_node->zuID,uobjtype,usubtype,uID );
// ^<uID5.51.1>!^!REQPARAM(5.51.2,MATH)
  if ( uobjtype==5 && usubtype==51 && uID==1 ) {
	  TNODE* mbucket  =  FindObject( tex_node->parts,
							(U8*)"5.51.2",INVALID_LIST_POS );
	  TNODE* cont =  mbucket->contents;
	  if ( cont ) {
      //U16 uobj,usub,id;
      //GetUids( cont->zuID,uobj,usub,id );
      //if ( uobj==3 && usub==3 )
	    rv  =  TRUE; 
	  }
  }

  return rv;
}


/*
;Decorations - MATH ONLY
\overline<uID5.12.1>!\overline!REQPARAM(5.13.1,MATH)
\overleftarrow<uID5.12.2>!\overleftarrow!REQPARAM(5.13.1,MATH)
\overrightarrow<uID5.12.3>!\overrightarrow!REQPARAM(5.13.1,MATH)
\overleftrightarrow<uID5.12.4>!\overleftrightarrow!REQPARAM(5.13.1,MATH)
\overbrace<uID5.12.5>!\overbrace!REQPARAM(5.13.1,MATH)
\widehat<uID5.12.6>!\widehat!REQPARAM(5.13.1,MATH)
\widetilde<uID5.12.7>!\widetilde!REQPARAM(5.13.1,MATH)
\underline<uID5.12.8>!\underline!REQPARAM(5.14.1,INHERIT)
\underleftarrow<uID5.12.9>!\underleftarrow!REQPARAM(5.13.1,MATH)
\underrightarrow<uID5.12.10>!\underrightarrow!REQPARAM(5.13.1,MATH)
\underleftrightarrow<uID5.12.11>!\underleftrightarrow!REQPARAM(5.13.1,MATH)
\underbrace<uID5.12.12>!\underbrace!REQPARAM(5.13.1,MATH)
*/

TNODE* LaTeX2MMLTree::Decoration2MML( TNODE* obj_node,
										TNODE** out_of_flow_list,
										U16 TeX_subclass,
										U16 tex_uID ) {

  TNODE* mml_rv =  NULL;

  if ( tex_uID==13 || tex_uID==14 )
    return FBox2MML( obj_node,out_of_flow_list,tex_uID );

  if ( tex_uID>=1 && tex_uID<=4 )
    if ( ContainsAllCaps(obj_node) )
      math_field_ID =  MF_GEOMETRY;

// Translate the decoration to MathML.
// The decoration is a stretchy operator

  TNODE* mml_script_contents  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );

  U8 mml_op_zuID[32];
  UidsTozuID( 3,25,tex_uID,(U8*)mml_op_zuID );
  U8* op_nom;
  U8* op_info;
  if ( d_mml_grammar->GetGrammarDataFromUID(mml_op_zuID,
								context_math,&op_nom,&op_info) ) {

// &OverBar;<uID3.25.1>postfix,70,U000AF,stretchy="true" accent="true"

    if ( op_nom && *op_nom )
      SetChData( mml_script_contents,op_nom,NULL );
	else
	  TCI_ASSERT(0);

	OP_GRAMMAR_INFO op_record;
	if ( op_info && *op_info ) {
      if ( output_entities_as_unicodes ) {
        U8 unicode_buffer[128];
        if ( GetUnicodeEntity(op_info,unicode_buffer) )
          SetChData( mml_script_contents,NULL,unicode_buffer );
	  }
      GetAttribsFromGammarInfo( op_info,op_record );
      if ( op_record.attr_list ) {
	    TCI_ASSERT( mml_script_contents->attrib_list == NULL );
	    mml_script_contents->attrib_list  =  op_record.attr_list;
		op_record.attr_list =  NULL;
	  }
	}

  } else
    TCI_ASSERT(0);

// Translate the object being decorated to MathML.

  TNODE* mml_base_contents  =  NULL;

  U8* bucket_zuID =  (tex_uID==8) ? (U8*)"5.14.1" : (U8*)"5.13.1";
  TNODE* base_bucket =  FindObject( obj_node->parts,
									bucket_zuID,INVALID_LIST_POS );
  if ( base_bucket ) {
	  TCI_BOOL do_bindings  =  TRUE;
    U16 tex_nodes_done,error_code;
    TNODE* local_oof_list =  NULL;
    mml_base_contents =  TranslateMathList( base_bucket->contents,
			do_bindings,NULL,tex_nodes_done,error_code,&local_oof_list );
    mml_base_contents =  HandleOutOfFlowObjects( mml_base_contents,
    						        &local_oof_list,out_of_flow_list,2 );
  } else
	  TCI_ASSERT(0);


  TNODE* b_cont =  mml_base_contents;
  if ( mml_base_contents ) {
    if ( mml_base_contents->next )
      b_cont  =  MMLlistToMRow( mml_base_contents );
  } else
    b_cont  =  CreateElemWithBucketAndContents( 5,750,1,2,NULL );

  if ( !mml_script_contents )
// mrow<uID5.750.1>!mrow!BUCKET(5.750.2,MATH,,,/mrow,)!/mrow!
    mml_script_contents =  CreateElemWithBucketAndContents( 5,750,1,2,NULL );

// Compose the mml return object

  U16 usubtype  =  tex_uID>7 ? 53 : 54;
  mml_rv  =  CreateElemWithBucketAndContents( 5,usubtype,2,3,b_cont );
  SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );
  TNODE* part1  =  mml_rv->parts;

  U16 script_bucketID  =  usubtype==53 ? 4 : 5;
  TNODE* s_cont =  mml_script_contents;
  if ( mml_script_contents && mml_script_contents->next )
    s_cont  =  MMLlistToMRow( mml_script_contents );
  TNODE* part2  =  CreateBucketWithContents(5,usubtype,script_bucketID,s_cont );

  part1->next   =  part2;
  part2->prev   =  part1;

  return mml_rv;
}


/*
\overset<uID5.10.0>!\overset!REQPARAM(5.10.1,MATH)REQPARAM(5.10.2,MATH)
\underset<uID5.11.0>!\underset!REQPARAM(5.11.1,MATH)REQPARAM(5.11.2,MATH)
\stackover<uID5.26.0>!\stackover!REQPARAM(5.26.1,MATH)REQPARAM(5.26.2,MATH)
\stackunder<uID5.27.0>!\stackunder!REQPARAM(5.27.1,MATH)REQPARAM(5.27.2,MATH)
; the first param of stackrel is small type and goes on top
\stackrel<uID5.28.0>!\stackrel!REQPARAM(5.28.1,MATH)REQPARAM(5.28.2,MATH)
*/

TNODE* LaTeX2MMLTree::OverOrUnder2MML( TNODE* obj_node,
						   			            U16 TeX_subclass,U16 tex_uID,
					   			              TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  U16 mode  =  0;			// 1 = <mover>, 2 = <munder>
  if ( TeX_subclass==10 || TeX_subclass==26 || TeX_subclass==28 )
    mode  =  1;				// <mover>
  else if ( TeX_subclass==11 || TeX_subclass==27 )
    mode  =  2;				// <munder>
  else
	TCI_ASSERT(0);

// Translate the decoration to MathML.

  TNODE* mml_script_contents  =  NULL;
  U8 bucket_zuID[32];
  UidsTozuID( 5,TeX_subclass,1,(U8*)bucket_zuID );

  TNODE* script_bucket  =  FindObject( obj_node->parts,
                            (U8*)bucket_zuID,INVALID_LIST_POS );
  if ( script_bucket ) {
	  TCI_BOOL do_bindings  =  TRUE;
    U16 tex_nodes_done,error_code;
    TNODE* local_oof_list =  NULL;
    script_level++;
    mml_script_contents =  TranslateMathList( script_bucket->contents,
                                do_bindings,NULL,tex_nodes_done,
                                error_code,&local_oof_list );
    script_level--;
    mml_script_contents =  HandleOutOfFlowObjects( mml_script_contents,
    								            &local_oof_list,out_of_flow_list,2 );
  } else
    TCI_ASSERT(0);

// Translate the object being decorated to MathML.

  TNODE* mml_base_contents  =  NULL;
  UidsTozuID( 5,TeX_subclass,2,(U8*)bucket_zuID );

  TNODE* base_bucket =  FindObject( obj_node->parts,
  							(U8*)bucket_zuID,INVALID_LIST_POS );
  if ( base_bucket ) {
	  TCI_BOOL do_bindings  =  TRUE;
    U16 tex_nodes_done,error_code;
    TNODE* local_oof_list =  NULL;
    mml_base_contents =  TranslateMathList( base_bucket->contents,
				do_bindings,NULL,tex_nodes_done,error_code,&local_oof_list );
    mml_base_contents =  HandleOutOfFlowObjects( mml_base_contents,
    								        &local_oof_list,out_of_flow_list,2 );
  } else
	  TCI_ASSERT(0);


  TNODE* b_cont =  mml_base_contents;
  if ( mml_base_contents ) {
    if ( mml_base_contents->next )
      b_cont  =  MMLlistToMRow( mml_base_contents );
  } else
    b_cont  =  CreateElemWithBucketAndContents( 5,750,1,2,NULL );


  if ( !mml_script_contents )
// mrow<uID5.750.1>!mrow!BUCKET(5.750.2,MATH,,,/mrow,)!/mrow!
    mml_script_contents =  CreateElemWithBucketAndContents( 5,750,1,2,NULL );

// Compose the mml return object

  U16 usubtype  =  mode==1 ? 54 : 53;
  mml_rv  =  CreateElemWithBucketAndContents( 5,usubtype,2,3,b_cont );
  SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_ORD );
  TNODE* part1  =  mml_rv->parts;

  U16 script_bucketID  =  usubtype==53 ? 4 : 5;
  TNODE* s_cont =  mml_script_contents;
  if ( mml_script_contents && mml_script_contents->next )
    s_cont  =  MMLlistToMRow( mml_script_contents );
  TNODE* part2  =  CreateBucketWithContents(5,usubtype,script_bucketID,s_cont );

  part1->next   =  part2;
  part2->prev   =  part1;

  return mml_rv;
}


// Translate LaTeX \symbol{94}, \symbol{126}
//  mml comes from MathML.gmr, <uID3.5.94>, etc.

TNODE* LaTeX2MMLTree::CmdSymbol2MML( TNODE* obj_node ) {

  TNODE* mml_rv   =  NULL;

// \symbol<uID5.465.0>!\symbol!REQPARAM(5.465.1,NONLATEX)
  TNODE* bucket =  FindObject( obj_node->parts,(U8*)"5.465.1",
      										              INVALID_LIST_POS );
  if ( bucket && bucket->contents ) {		// locate NONLATEX content
    U8* zNONLATEX =  (U8*)"888.8.0";
    TNODE* cont =  FindObject( bucket->contents,zNONLATEX,
      										              INVALID_LIST_POS );
    if ( cont ) {
	  U16 code  =  atoi( (char*)cont->var_value );
      U8 zuID[32];
      UidsTozuID( 3,5,code,(U8*)zuID );
      U8* dest_zname;
      U8* d_template;
      if ( d_mml_grammar->GetGrammarDataFromUID(zuID,
					        context_math,&dest_zname,&d_template) ) {
        mml_rv   =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );
        SetChData( mml_rv,dest_zname,NULL );
	  } else
	    TCI_ASSERT(0);

	} else
	  TCI_ASSERT(0);

  } else
	  TCI_ASSERT(0);

  return mml_rv;
}


// After <mo>'s are created, but before the forms of multiforms
//  are set, we look for implied fences, (whatever).
// We are looking for <mo>s with prefix or postfix forms
//  with precedence 1.

U16 LaTeX2MMLTree::GetPrecedence( TNODE* mml_node,U16 op_form_ID ) {

  U16 rv  =  0;

  U8* opname  =  mml_node->var_value;
  U16 nln =  strlen( (char*)opname );
  U8* zform   =  (U8*)zop_forms[op_form_ID];

  U8* gmr_section =  context_math;
  U16 forms_for_op;
  if ( IsMultiFormOp(opname,forms_for_op) )
    gmr_section =  (U8*)"MULTIFORMOPS";

  U8* zuID;
  U8* zproduction;
  if ( s_mml_grammar->GetGrammarDataFromNameAndAttrs(opname,
    									nln,zform,gmr_section,
                                		&zuID,&zproduction) ) {
	if ( zproduction ) {
	  char* p =  strchr( (char*)zproduction,',' );
	  if ( p )
	    rv  =  atoi(p+1);
	} else
      TCI_ASSERT(0);
  }

  return rv;
}


// Given a fence delimiting symbol, return it's mate(s).
//  "(" has mates  ")" and "]"
// The secondary mate is used for intervals, as in (0,1].
// Note that the "mate(s)" are defined
//  in the CLOSERS section of MathML.gmr

void LaTeX2MMLTree::GetMatchingFence( U8* left_delim,
  									     U8* matching_delim,
  									     U8* secondary_match ) {

  matching_delim[0]   =  0;
  secondary_match[0]  =  0;

// Use left_delim to look up any possible CLOSERs in MathML.gmr

  U16 nln =  strlen( (char*)left_delim );
  U8* closer_zuID;
  U8* closer_zinfo;
  if ( s_mml_grammar->GetGrammarDataFromNameAndAttrs(
   						left_delim,nln,NULL,(U8*)"CLOSERS",
                            &closer_zuID,&closer_zinfo) ) {
    if ( closer_zinfo && *closer_zinfo ) {
// (<uID13.1.1> mate=")" secondary="]"
	    char* pm  =  strstr( (char*)closer_zinfo,"mate" );
	    if ( pm ) {
		    char* p1  =  strchr( pm,'"' );
		    if ( p1 ) {
		      p1++;
		      char* p2  =  strchr( p1,'"' );
		      if ( p2 ) {
		        U16 zln =  p2 - p1;
		        strncpy( (char*)matching_delim,p1,zln );
		        matching_delim[zln] =  0;
		      }
		    }
	    }
	    char* ps  =  strstr( (char*)closer_zinfo,"secondary" );
	    if ( ps ) {
		    char* p1  =  strchr( ps,'"' );
		    if ( p1 ) {
		      p1++;
		      char* p2  =  strchr( p1,'"' );
		      if ( p2 ) {
		        U16 zln =  p2 - p1;
		        strncpy( (char*)secondary_match,p1,zln );
		        secondary_match[zln] =  0;
		      }
		    }
	    }

	  } else		// missing CLOSERS info??
	    TCI_ASSERT(0);
  }		// if ( lookup succeeded )

}


// ||<uID3.21.13>prefix,1
// int<uID7.1.1>prefix,31,U0222B,largeop="true" stretchy="true"  lspace="0em"
// format is as follows
// form_name,precedence_number,Unicode_hex,att1_nom="att1_val" att2_nom="att2_val"...

void LaTeX2MMLTree::GetAttribsFromGammarInfo( U8* zinfo,
								                OP_GRAMMAR_INFO& op_record ) {

  op_record.form        =  0;
  op_record.precedence  =  0;
  op_record.unicode     =  NULL;
  op_record.attr_list   =  NULL;

  char* sptr  =  (char*)zinfo;
  char* eptr;
  U16 state =  0;
  do {
    eptr  =  strchr( sptr,',' );
	  if ( !eptr )
	    eptr  =  (char*)zinfo + strlen( (char*)zinfo );
	  if ( eptr >= sptr ) {
	    state++;
	    if        ( state==1 ) {		// multiform,prefix,infix,postfix
		    U16 ln  =  eptr - sptr;
		    if ( ln > 0 && ln < 8 ) {
		      char buffer[16];
		      strncpy( buffer,sptr,ln );
		      buffer[ln]  =  0;
		      U16 id  =  OPF_prefix;
		      while ( id <= OPF_postfix ) {
            if ( !strcmp(zop_forms[id],buffer) ) {
		          op_record.form  =  id;
			        break;
			      } else
			        id++;
		      }
	        TCI_ASSERT( op_record.form );
		    }

	    } else if ( state==2 ) {		// 30,
		    U16 ln  =  eptr - sptr;
		    if ( ln > 0 ) {
		      op_record.precedence  =  atoi( sptr );
	        //TCI_ASSERT( op_record.precedence );
		    }
	    } else if ( state==3 ) {		// U0dddd,
	  // currently not used
	    } else if ( state==4 ) {		// lspace="..."
		    if ( eptr > sptr )
          op_record.attr_list =  ExtractAttrs( sptr );
	    } else
	      TCI_ASSERT(0);

	// Advance to the next "," separated field

      if ( *eptr )
        sptr  =  eptr + 1;
	    else
	      break;

	  } else
	    break;

  } while ( TRUE );

}


// largeop="true" stretchy="true"  lspace="0em" rspace="0em"
     
ATTRIB_REC* LaTeX2MMLTree::ExtractAttrs( char* zattrs ) {

  ATTRIB_REC* head  =  NULL;	// We build an "attrib_list"
  ATTRIB_REC* tail;

  if ( zattrs ) {
    TCI_BOOL done   =  FALSE;
    char* anom_ptr  =  zattrs;
    while ( !done ) {
  // Span spaces
      while ( *anom_ptr && *anom_ptr <= ' ' ) anom_ptr++;
      if ( *anom_ptr ) {
        char* p_equal  =  strchr( anom_ptr,'=' );
        if ( p_equal ) {
          char* left_delim  =  strchr( p_equal+1,'"' );
          if ( left_delim ) {
            left_delim++;
            char* right_delim  =  strchr( left_delim,'"' );
            if ( right_delim ) {
              while ( p_equal > anom_ptr && *(p_equal-1) <= ' ' )
                p_equal--; 
              char znom[80];
              U16 nom_ln  =  p_equal - anom_ptr;
              if ( nom_ln && nom_ln < 80 ) {
                strncpy( znom,anom_ptr,nom_ln );
                znom[nom_ln]  =  0;
                U16 val_ln  =  right_delim - left_delim;
                ATTRIB_REC* arp   =  MakeATTRIBNode( (U8*)znom,7,7,
                                        (U8*)left_delim,val_ln,0 );
                if ( !head )
                  head  =  arp;
                else
                  tail->next  =  arp;
                tail  =  arp;
              } else    // attrib zname exceeds 80 bytes??
                TCI_ASSERT(0);
              anom_ptr =  right_delim + 1;
            } else			// no ending "
              done  =  TRUE;
          } else			// no starting "
	          done  =  TRUE;
        } else		// no = 
          done  =  TRUE;
      } else		// nothing after spaces
        done  =  TRUE;
    }			// while ( !done )
  }

  return head;
}


// Add (or reset) an ATTRIB_REC* to the attrib_list
//  of a TNODE that represents a MathML object.

void LaTeX2MMLTree::SetNodeAttrib( TNODE* mml_node,
									U8* a_nom,U8* a_val ) {

  if ( a_nom && a_val && a_nom[0] ) {
    U16 vln =  strlen( (char*)a_val );

// Search for this attrib in the node's current attrib list.

    ATTRIB_REC* tail;
    TCI_BOOL new_attrib =  TRUE;
    ATTRIB_REC* rover   =  mml_node->attrib_list;
    while ( rover ) {
      if ( !strcmp((char*)rover->attr_nom,(char*)a_nom) ) {
  // overwrite the existing attribute value
	    if ( rover->z_val )
	      delete rover->z_val;
	    char* tmp =  TCI_NEW( char[vln+1] );
        strcpy( tmp,(char*)a_val );
	    rover->z_val  =  (U8*)tmp;
        new_attrib    =  FALSE;
        break;
	  }
      tail  =  rover;
      rover =  rover->next;
    }

    if ( new_attrib ) {		// Not a reset
      ATTRIB_REC* arp =  MakeATTRIBNode( a_nom,7,0,a_val,vln,0 );
      if ( mml_node->attrib_list ) {
        tail->next =  arp;
        arp->prev  =  tail;
      } else
        mml_node->attrib_list  =  arp;
    }

  } else
    TCI_ASSERT(0);

}


// MathML 2.0 re-defined the markup of some operators -
//  in particular, the long versions of several arrows.
// We still script the old form if we're generating 1.0 output.
// The old productions are kept in the "DEPOPS" section of
//  MathML.gmr

TCI_BOOL LaTeX2MMLTree::IsDeprecatedOp( TNODE* tex_sym_node,
									U8** name,U8** production ) {

  TCI_BOOL rv =  FALSE;
  if ( mml_version < 200 || !stretch_for_long_arrows )
    if ( d_mml_grammar->GetGrammarDataFromUID(tex_sym_node->zuID,
						  		(U8*)"DEPOPS",name,production) )
      rv  =  TRUE;

  return rv;
}


// An <mo> TNODE that has some attributes from its beginnings
//  may get more from MathML.gmr when it's form is set.
// If a given attr already has a value, we reset the value.
// Otherwise, we append a new attribute/value.

void LaTeX2MMLTree::MergeMOAttribs( TNODE* mml_op_node,
								ATTRIB_REC* attr_list ) {

  if ( attr_list ) {
    ATTRIB_REC* a_rover =  attr_list;
    while ( a_rover ) {
      U8* attr_nom  =  a_rover->attr_nom;
      U8* attr_val  =  a_rover->z_val;
      if      ( !strcmp((char*)attr_nom,"lspace") )
        SetMOSpacing( mml_op_node,TRUE,attr_val );
      else if ( !strcmp((char*)attr_nom,"rspace") )
        SetMOSpacing( mml_op_node,FALSE,attr_val );
      else
        SetNodeAttrib( mml_op_node,attr_nom,attr_val );
      a_rover =  a_rover->next;
    }
  }
}


/* Note:: We always specify the mml "name" here - ie. the contents
 of the returned <mo>.  However, we only specify attributes and
 details for operators that have only one "form".  For multi-form
 operators, like +, -, etc., we decide the form (and specify it
 and other details) from context in a later pass, "AddOperatorInfo".
*/

void LaTeX2MMLTree::SetMMLOpNode( TNODE* tex_op_node,
									U16& tex_nodes_spanned,
									TNODE* mml_rv ) {

  U8 mml_opname[256];		// Buffer for the contents of the
  mml_opname[0] =  0;		//  MathML TNODE we're generating.

  U8 mml_uniname[256];
  mml_uniname[0]  =  0;

// There may be several consecutive LaTeX TNODEs that should
//  be mapped to a single MathML <mo>.  Things like ++, --, >=, etc.

  U8* mml_op_info =  NULL;

  U8* production  =  NULL;

  TCI_BOOL is_single_token_op =  TRUE;
  if ( mml_rv->details
  &&   mml_rv->details->is_differential != UNDEFINED_DETAIL ) {

//  SetDetailNum( mml_rv,DETAILS_is_differential,1 );

  } else if ( IsMultiTeXSymOp(tex_op_node,&production,tex_nodes_spanned) ) {

// -=<uID3.21.1>-=,infix,12,,lspace="thinmathspace" rspace="thinmathspace"

    is_single_token_op  =  FALSE;
    if ( production && *production ) {
	  U8* p_comma =  (U8*)strchr( (char*)production,',' );
	  if ( p_comma ) {
        U16 nln =  p_comma - production;
	    strncpy( (char*)mml_opname,(char*)production,nln );
	    mml_opname[nln] =  0;
	    mml_op_info =  p_comma + 1;

	    U8* p_comma2  =  (U8*)strchr( (char*)mml_op_info,',' );
	    if ( p_comma2 ) {
	      if ( output_entities_as_unicodes ) {
            U16 nln =  p_comma2 - mml_op_info;
	        strncpy( (char*)mml_uniname,(char*)mml_op_info,nln );
	        mml_uniname[nln] =  0;
		  }
	      mml_op_info =  p_comma2 + 1;

	    } else
          TCI_ASSERT(0);

	  } else
        TCI_ASSERT(0);
	} else
	  TCI_ASSERT(0);
  }

  if ( is_single_token_op ) {
    tex_nodes_spanned =  1;
    U8* dest_zname;

    if ( !stretch_for_long_arrows 
    &&   IsDeprecatedOp(tex_op_node,&dest_zname,&mml_op_info) ) {
      // these are &xlarr; etc.
      strcpy( (char*)mml_opname,(char*)dest_zname );
      if ( output_entities_as_unicodes )
        GetUnicodeEntity( mml_op_info,mml_uniname );

	} else if ( d_mml_grammar->GetGrammarDataFromUID(tex_op_node->zuID,
    							  context_math,&dest_zname,&mml_op_info) ) {
      strcpy( (char*)mml_opname,(char*)dest_zname );
      if ( output_entities_as_unicodes )
        GetUnicodeEntity( mml_op_info,mml_uniname );

	} else {
	  TCI_ASSERT(0);
      strcpy( (char*)mml_opname,(char*)tex_op_node->src_tok );
      strcat( (char*)mml_opname,"??" );
	}
  }

// Here we have the MML name of the operator.

  TCI_ASSERT( mml_opname[0] );
  SetChData( mml_rv,mml_opname,mml_uniname );

  if ( mml_op_info && *mml_op_info ) {
    OP_GRAMMAR_INFO op_record;
    GetAttribsFromGammarInfo( mml_op_info,op_record );

    U16 forms_for_op;
    if ( IsMultiFormOp(mml_opname,forms_for_op) ) {

      if ( op_record.attr_list ) {
        DisposeAttribs( op_record.attr_list );
        op_record.attr_list =  NULL;
      }

	} else {
// Here the operator has only one form - we have it's mml info.
      U16 form_ID =  op_record.form;
	  if ( form_ID >= OPF_prefix && form_ID <= OPF_postfix )
	    SetNodeAttrib( mml_rv,(U8*)"form",(U8*)zop_forms[form_ID] );
	  else
	    TCI_ASSERT(0);
      if ( op_record.attr_list ) {
        MergeMOAttribs( mml_rv,op_record.attr_list );
        DisposeAttribs( op_record.attr_list );
        op_record.attr_list =  NULL;
      }

      SetDetailNum( mml_rv,DETAILS_form,form_ID );
      SetDetailNum( mml_rv,DETAILS_precedence,op_record.precedence );
    }
  }

}  // SetMMLOpNode



// When an <mstyle> node is generated, we set a "style ID"
//  in the details to indicate the intent of the tagging.
// Most <mstyle> nodes change size only - no semantic change
//  is intended.  However, <mstyle> may change font/face,
//  giving the tagged object a different meaning.
// The following function generates a qualifying string
//  for a function name that is tagged.

void LaTeX2MMLTree::StyleIDtoStr( U16 ID,U8* style_nom ) {

  style_nom[0]  =  0;

  char* nom =  NULL;
  switch ( ID ) {
    case 1  :
	  // scriptlevel
	  break;
    case 2  :
	  // displaystyle
	  break;
    case 80 :	// \mathcal<uID5.80.0>
      nom =  "mathcal ";
	  break;
    case 81 :	// \mathrm<uID5.81.0>
      nom =  "mathrm ";
	  break;
    case 82 :	// \mathbf<uID5.82.0>
      nom =  "mathbf ";
	  break;
    case 83 :	// \mathsf<uID5.83.0>
      nom =  "mathsf ";
    break;
    case 84 :	// \mathtt<uID5.84.0>
      nom =  "mathtt ";
	  break;
    case 85 :	// \mathnormal<uID5.85.0>
      nom =  "mathnormal ";
	  break;
    case 86 :	// \mathit<uID5.86.0>
      nom =  "mathit ";
	  break;
    case 87  :	// \mathbb<uID5.87.0>
      nom =  "mathbb ";
    break;
    case 88  :	// \mathfrak<uID5.88.0>
      nom =  "mathfrak ";
    break;
    case 89  :	// \pmb<uID5.89.0>!
      TCI_ASSERT(0);
    break;
    case  TR_Tup  :
      // attr_nom  =  (U8*)"fontstyle";
      // attr_val  =  (U8*)"upright";
    break;
    case  TR_Tmd	:
      //  attr_nom  =  (U8*)"fontweight";
      //  attr_val  =  (U8*)"medium";
    break;
    case  TR_Tsc  :
      //  attr_nom  =  (U8*)"fontstyle";
      //  attr_val  =  (U8*)"slanted";
    break;
    case  TR_Tsl  :
      //  attr_nom  =  (U8*)"fontstyle";
      //  attr_val  =  (U8*)"slanted";
    break;
    case  TR_em   :
      //  attr_nom  =  (U8*)"fontweight";
      //  attr_val  =  (U8*)"bold";
    break;
    case  TR_group  :
    break;

	  default :
	    TCI_ASSERT(0);
	  break;
  }

  if ( nom )
    strcpy( (char*)style_nom,nom );
}



TNODE* LaTeX2MMLTree::PassThru2MML( TNODE* TeX_node, TNODE** out_of_flow_list ) 
{
  //JBMLine("PassThru\n");
  TNODE* node =  FindObject(TeX_node->parts, (U8*)"5.476.1",INVALID_LIST_POS);
  const char* theText =  (const char*)(node->contents->var_value);
  
  //JBMLine(theText);

  char* buf = new char[1+strlen(theText)];
  strcpy(buf, theText);
  // jcs May want this in cdata?
  //char* buf = new char[strlen(theText) + strlen("<![CDATA[]]>")];
  //strcpy(buf, "<![CDATA[");
  //strcpy(buf + strlen("<![CDATA["), theText);
  //strcpy(buf +  strlen("<![CDATA[") + strlen(theText), "]]>");
  
  TNODE* mml  =  MakeTNode( 0L,0L,0L,(U8*)"0.0.0"); 
  SetChData( mml, (U8*)buf, NULL );

  TNODE* rv  =  CreateElemWithBucketAndContents( 5,476,0,1,mml );
  //JBMLine("\nEnd PassThru\n");

  return rv;
}


// \rule's that make a mark can't be translated to MML.
//  However, in our .tex files most \rule's are used
//  to increase the size of a bounding box (ie. struts). 
//  We can translate these to <mpadded> elements.

TNODE* LaTeX2MMLTree::Rule2MML( TNODE* TeX_rule_node,
									              TNODE** out_of_flow_list ) {

  TNODE* rv =  NULL;

  if ( RuleIsPureVSpace(TeX_rule_node) ) {
    *out_of_flow_list  =  MoveNodeToList( *out_of_flow_list,TeX_rule_node );

  } else
    TCI_ASSERT(0);

  return rv;
}


// TeX \rule's are often used to force an increase in ascent
//  or descent of the bounding box for a run of MATH.

TCI_BOOL LaTeX2MMLTree::RuleIsPureVSpace( TNODE* TeX_rule_node ) {

  TCI_BOOL rv =  FALSE;

  TNODE* wp =  FindObject( TeX_rule_node->parts,
							(U8*)"5.800.2",INVALID_LIST_POS );
  if ( wp && wp->contents ) {
    double rule_width  =  0.0;
    rule_width  =  atof( (char*)wp->contents->src_tok );
    if ( -0.0001 < rule_width && rule_width < 0.0001 )
      rv =  TRUE;
  }

  return rv;
}


// In TeX, \rule[dimen]{0}{dimen} ( a rule with 0 width )
//  can be used to pad a run of MATH with extra whitespace
//  above and/or below.

void LaTeX2MMLTree::GetVSpaceFromRule( TNODE* TeX_rule_node,
										                    U8* depth,U8* height ) {

// \rule<uID5.800.0>!\rule!
//	OPTPARAM(5.800.1,DIMEN) - offset from baseline
//	REQPARAM(5.800.2,DIMEN) - rule horizontal extent
//	REQPARAM(5.800.3,DIMEN) - rule vertical extent

  TNODE* offset =  FindObject( TeX_rule_node->parts,
							              (U8*)"5.800.1",INVALID_LIST_POS );
  TNODE* hp     =  FindObject( TeX_rule_node->parts,
							              (U8*)"5.800.3",INVALID_LIST_POS );

  U16 offset_type =  0;
  double rule_offset =  0.0;		// vertical offset from baseline.
  if ( offset && offset->contents ) {
    char* p_dimen =  (char*)offset->contents->src_tok;
    rule_offset =  atof( p_dimen );
    offset_type =  TeXUnitToID( p_dimen );
  }
  U16 height_type =  0;
  double rule_height =  0.0;
  if ( hp && hp->contents ) {
    char* p_dimen =  (char*)hp->contents->src_tok;
    rule_height =  atof( p_dimen );
    height_type =  TeXUnitToID( p_dimen );
  }

  if ( offset_type && offset_type != height_type )
    rule_offset =  ConvertTeXDimen( rule_offset,offset_type,height_type );

  double rule_ascent  =  rule_offset + rule_height;
  double rule_descent =  -rule_offset;

  U16 mml_dest_type =  GetMMLdimenType( height_type );
  if ( rule_ascent < -0.00001 || rule_ascent > 0.00001 ) {
    rule_ascent =  ConvertTeXDimen( rule_ascent,height_type,mml_dest_type );
	  sprintf( (char*)height,"%f",rule_ascent );
    AttachUnit( (char*)height,mml_dest_type );
  } else
    height[0] =  0;

  if ( rule_descent < -0.00001 || rule_descent > 0.00001 ) {
    rule_descent  =  ConvertTeXDimen( rule_descent,height_type,mml_dest_type );
	  sprintf( (char*)depth,"%f",rule_descent );
    AttachUnit( (char*)depth,mml_dest_type );
  } else
    depth[0] =  0;
}


// Convert a TeX dimen expressed in some TeX unit
//  to it's equivalent in another TeX unit.
// There are some restrictions on the conversions allowed.
// We convert to a common unit first (I chose "mils") and
//  then convert the common value to dest_type.
// Warning: for generality, conversions between absolute
//  and font relative units are provided.  This is BOGUS.

double LaTeX2MMLTree::ConvertTeXDimen( double s,U16 src_type,
												                      U16 dest_type ) {

  double rv =  0.0;

  if ( src_type == dest_type ) {
    rv  =  s;

  } else {

// Warnings re absolute <-> relative conversions

    if      ( src_type == 6 || dest_type == 6 ) {    // ex
	    TCI_ASSERT(0);	// Can't convert "ex"s into anything else
    } else if ( src_type == 5 && dest_type != 9 ) {    // em
	    TCI_ASSERT(0);  // Can't convert "em"s or "mu"s into anything else
    } else if ( src_type == 9 && dest_type != 5 ) {    // mu->em
	    TCI_ASSERT(0);  // Can't convert "em"s or "mu"s into anything else
    }

    double numerator;
    double denominator;

    U16 i =  0;
	  while ( i < 2 ) {
      U16 the_type  =  (i==0) ? src_type : dest_type;
	    switch ( the_type ) {
	      case 1  :				// bp
          numerator   =  1000.0;
          denominator =  72.0;
	      break;
	      case 2  :				// cc
          numerator   =  2845.0;
          denominator =  4.531 * 72.27;
	      break;
	      case 3  :				// cm
          numerator   =  28450.0;
          denominator =  72.27;
	      break;
	      case 4  :				// dd
          numerator   =  2845.0;
          denominator =  0.376 * 72.27;
	      break;
	      case 7  :				// in
          numerator   =  1000.0;
          denominator =  1.0;
	      break;
	      case 8  :				// mm
          numerator   =  2845.0;
          denominator =  72.27;
	      break;
	      case 10 :				// pc
          numerator   =  12000.0;
          denominator =  72.27;
	      break;
	      case 11 :				// pt
          numerator   =  1000.0;
          denominator =  72.27;
	      break;
	      case 12 :				// sp
          numerator   =  1000.0;
          denominator =  65536.0 * 72.27;
	      break;

// font relative
	      case 5  :				// em
          numerator   =  18000.0;
          denominator =  120.0;
	      break;
	      case 6  :				// ex
          numerator   =  9000.0;
          denominator =  120.0;
	      break;
	      case 9  :				// mu
          numerator   =  1000.0;
          denominator =  120.0;
	      break;

	      default :
	        TCI_ASSERT(0);
          numerator   =  1.0;
          denominator =  1.0;
	      break;
	    }

      if ( i==0 )
        rv  =  ( s * numerator ) / denominator;
	    else
        rv  =  ( rv * denominator ) / numerator;

	    i++;
	  }

  }

  return rv;
}


static char* z_TeX_units =  "bp cc cm dd em ex in mm mu pc pt sp ";
//	          ids ->				 1  2  3  4	 5  6  7  8	 9	10 11 12

U16 LaTeX2MMLTree::TeXUnitToID( char* p_dimen ) {

  U16 rv  =  0;

// Locate the unit in p_dimen.
// p_dimen  =  "0.25in" , "12pt" , "2.35mm" , etc.
//					^		 ^			^

  int unit_offset  =  strcspn( p_dimen,"bcdeimps" );
  if ( unit_offset >= 0 && unit_offset < strlen(p_dimen) ) {
	  char z_unit[4];
	  z_unit[0] =  p_dimen[unit_offset];
	  z_unit[1] =  p_dimen[unit_offset+1];
	  z_unit[2] =  ' ';
	  z_unit[3] =  0;
    char* p =  strstr( z_TeX_units,z_unit );
    if ( p ) {
      U16 offset  =  p - z_TeX_units;
	    rv  =  ( offset / 3 ) + 1;
    } else
      TCI_ASSERT( 0 );	// unknown TeX unit?

  } else
    TCI_ASSERT( 0 );

  return rv;
}


void LaTeX2MMLTree::AttachUnit( char* mml_dimen,U16 unit_id ) {

  if ( mml_dimen && unit_id && unit_id <= 12 ) {
    U16 offset  =  ( unit_id - 1 ) * 3;
	  U16 zln =  strlen( mml_dimen );
	  mml_dimen[zln++]  =  z_TeX_units[offset++];
	  mml_dimen[zln++]  =  z_TeX_units[offset];
	  mml_dimen[zln]    =  0;
  }

}


// Given a TeX unit, we select a unit for our MML output.
// This is a bit arbitrary - I don't know just what units
//  are available in MML.  This info needs to come from
//  mathml.gmr

U16 LaTeX2MMLTree::GetMMLdimenType( U16 TeX_dimen_type ) {

  U16 rv  =  0;

  switch ( TeX_dimen_type ) {
    case 1  : rv  =  7;		break;		// Big point,   bp -> in
    case 2  : rv  =  8;		break;		// Cicero,      cc -> mm
    case 4  : rv  =  8;		break;		// Didot point, dd -> mm

    case 3  : rv  =  3;		break;		// cm ->  not changed
    case 5  : rv  =  5;		break;		// em -> 	  "
    case 6  : rv  =  6;		break;		// ex -> 	  "
    case 7  : rv  =  7;		break;		// in -> 	  "
    case 8  : rv  =  8;		break;		// mm -> 	  "
    case 11 : rv  =  11;	break;		// pt -> 	  "

    case 9  : rv  =  5;		break;		// Math unit,   mu -> em
    case 10 : rv  =  11;	break;		// Pica,        pc -> pt
    case 12 : rv  =  11;	break;		// Scaled point,sp -> pt
    default :
      TCI_ASSERT(0);
    break;
  }

  return rv;
}


TNODE* LaTeX2MMLTree::DisposeOutOfFlowList( TNODE* out_of_flow_list,
												                      U16 id1,U16 id2 ) {


  TNODE* head   =  NULL;
  TNODE* tail;
  TNODE* rover  =  out_of_flow_list;
  while ( rover ) {
    U16 uobjtype,usubtype,uID;
    GetUids( rover->zuID,uobjtype,usubtype,uID );
	  TCI_BOOL do_dispose =  FALSE;
    if      ( !id1  )
	    do_dispose =  TRUE;
    else if ( uobjtype==id1 && usubtype==id2 )
	    do_dispose =  TRUE;
    if ( do_dispose ) {
	    TNODE* del  =  rover;
	    rover =  rover->next;
	    del->next   =  NULL;
	    if ( rover )
	      rover->prev =  NULL;
	    DisposeTNode( del );
	  } else {
	    if ( head ) {
	      tail->next  =  rover;
		    rover->prev =  tail;
	    } else
	      head  =  rover;
	    tail  = rover;
	    rover =  rover->next;
	    tail->next  =  NULL;
	    if ( rover )
	      rover->prev =  NULL;
	  }
  }

  return head;
}


// When nodes that generate vspace are encountered in a LaTeX
//  contents list, they are set aside until mml generation
//  has been completed.  MML_rv and the out_of_flow_list
//  are passed to the following function, where MML_rv
//  may be wrapped in an <mpadded>.

TNODE* LaTeX2MMLTree::VSpace2MML( TNODE* MML_rv,
									TNODE* out_of_flow_list,
									U16 vspace_context ) {

  TNODE* rv =  NULL;

  if ( MML_rv ) {
	  U8 depth[64];
	  U8 height[64];
	  depth[0]  =  0;
	  height[0] =  0;
    double skip_multiplier =  0.0;

    TNODE* rover  =  out_of_flow_list;
    while ( rover ) {
      U16 uobjtype,usubtype,uID;
      GetUids( rover->zuID,uobjtype,usubtype,uID );

      if ( uobjtype==9 && usubtype==3 ) {
	      switch ( uID ) {

	        case 1 :		// \smallskip<uID9.3.1> 0.25 * \baselineskip
	        case 2 :		// \medskip<uID9.3.2>	0.50 * \baselineskip
	        case 3 :		// \bigskip<uID9.3.3>	1.00 * \baselineskip
            if        ( vspace_context == 1 ) {
              RecordAnomaly( 1003,NULL,rover->src_offset1,
        						            rover->src_offset2 );
            } else if ( vspace_context == 2 ) {
	            if      ( uID==1 )
                skip_multiplier +=  1.0;
	            else if ( uID==2 )
                skip_multiplier +=  2.0;
	            else if ( uID==3 )
                skip_multiplier +=  4.0;
            } else if ( vspace_context == 3 ) {
              // ignore - that's what LaTeX does.
            } else
	            TCI_ASSERT(0);
	        break;


	        case 4 :		// \strut<uID9.3.4>
	        case 5 :		// \mathstrut<uID9.3.5>  // not implemented
	          strcpy( (char*)depth,"+3.5pt" );
	          strcpy( (char*)height,"+8.5pt" );
	        break;

	        break;
	        case 6 :		// \vspace<uID9.3.6>!\vspace!REQPARAM(9.3.20,GLUE)
	        case 7 :		// \vspace*<uID9.3.7>!\vspace*!REQPARAM(9.3.21,GLUE)
            if        ( vspace_context == 1 ) {
              RecordAnomaly( 1003,NULL,rover->src_offset1,
        						            rover->src_offset2 );
            } else if ( vspace_context == 2 ) {
// \vspace<uID9.3.6>!\vspace!REQPARAM(9.3.20,GLUE)
// \vspace*<uID9.3.7>!\vspace*!REQPARAM(9.3.21,GLUE)
              I16 ems_value;
	            U8 bucket_zuID[32];
              UidsTozuID( 9,3,uID+14,(U8*)bucket_zuID );
              TNODE* glue_bucket =  FindObject( rover->parts,
							              (U8*)bucket_zuID,INVALID_LIST_POS );
              GetValueFromGlue( glue_bucket,(U8*)depth,TRUE,ems_value );
            } else if ( vspace_context == 3 ) {
              // ignore - that's what LaTeX does.
            } else
	            TCI_ASSERT(0);
	        break;
	        default :
	          TCI_ASSERT(0);
	        break;
	      }

	    }

	    rover =  rover->next;
    }

    if ( skip_multiplier ) {
      TCI_ASSERT( depth[0] == 0 );
      double extra_depth  = skip_multiplier * 0.5;
	    sprintf( (char*)depth,"+%fex",extra_depth );
    }

    if ( depth[0] || height[0] ) {
// mpadded<uID5.602.0>!mpadded!BUCKET(5.602.2,MATH,,,/mpadded,)!/mpadded!
      MML_rv  =  FixImpliedMRow( MML_rv );
      rv  =  CreateElemWithBucketAndContents( 5,602,0,2,MML_rv );

      if ( depth[0] )
        SetNodeAttrib( rv,(U8*)"depth",depth );
      if ( height[0] )
        SetNodeAttrib( rv,(U8*)"height",height );

	  } else
      rv  =  MML_rv;

  } else 	// MATH content list has only vspace in it.
    RecordAnomaly( 1003,NULL,out_of_flow_list->src_offset1,
        						out_of_flow_list->src_offset2 );

  return rv;
}


// \label{} commands are mapped to "id" attributes
//   on a containing MML element.

TNODE* LaTeX2MMLTree::Labels2MML( TNODE* MML_rv,
									                TNODE* label_list ) {

  TNODE* rv =  NULL;

  if ( !MML_rv || MML_rv->next ) {
// if !MML_rv, we may want to generate something non-empty
//  as contents for the following <mrow>.
    rv  =  CreateElemWithBucketAndContents( 5,750,1,2,MML_rv );
  } else
	  rv  =  MML_rv;

  while ( label_list ) {
// \label      <uID5.700.0>...REQPARAM(5.700.1,NONLATEX)
// \hypertarget<uID5.552.0>...REQPARAM(5.552.1,NONLATEX)
// \HYPERTARGET<uID5.556.0>...REQPARAM(5.556.2,NONLATEX)
// \QTSN<uID5.472.0>!\QTSN!REQPARAM(5.472.1,NONLATEX)REQPARAM(5.472.2,TEXT)
    U16 uobjtype,usubtype,uID;
    GetUids( label_list->zuID,uobjtype,usubtype,uID );
	  U8* zuID_name_bucket  =  NULL;
    if ( uobjtype==5 && uID==0 ) {
      if      ( usubtype == 700 )
	      zuID_name_bucket  =  (U8*)"5.700.1";
      else if ( usubtype == 552 )
	      zuID_name_bucket  =  (U8*)"5.552.1";
      else if ( usubtype == 556 )
	      zuID_name_bucket  =  (U8*)"5.556.2";
      else if ( usubtype == 472 )
	      zuID_name_bucket  =  (U8*)"5.472.1";
      else
        TCI_ASSERT(0);
	  }


	  if ( zuID_name_bucket ) {
      TNODE* l_name_bucket  =  FindObject( label_list->parts,
                                zuID_name_bucket,INVALID_LIST_POS );
      if ( l_name_bucket && l_name_bucket->contents) {
        U8* attr_val  =  l_name_bucket->contents->var_value;
        SetNodeAttrib( rv,(U8*)"key",attr_val );
	    } else
	      TCI_ASSERT(0);
	  }


    label_list  =  label_list->next;
// If there's another label, wrap the current rv
//  in another <mrow> and label this new wrapper <mrow>.
    if ( label_list )
      rv  =  CreateElemWithBucketAndContents( 5,750,1,2,rv );
  }		// loop thru label_list

  return rv;
}


// The DLL version of LaTeX2MMLTree can pass a list
//  of "anomaly" lines back to it's caller.
// Schemata like \FRAME, \footnote, etc. that cannot
//  be handled in MathML are passed back by this mechanism.

void LaTeX2MMLTree::RecordAnomaly( U16 ilk,U8* ztext,
									                  U32 off1,U32 off2 ) {

  if ( p_anomalies ) {		// We're reporting anomalies - DLL
  	ANOMALY_REC* new_anomaly;
    if ( p_anomalies->ilk ) {	// first node is already taken.
  	  new_anomaly  =  TCI_NEW( ANOMALY_REC );
	    new_anomaly->next =  NULL;
  // Append to end of the current list.
  	  ANOMALY_REC* rover  =  p_anomalies;
	    while ( rover->next )
	      rover =  rover->next;
	    rover->next =  new_anomaly;
	  } else
  	  new_anomaly =  p_anomalies;

  // Fill out ANOMALY_REC
  	new_anomaly->ilk  =  ilk;		// an id
  	new_anomaly->atext  =  ztext ? ztext : NULL;
  	new_anomaly->off1 =  off1;		// src offsets
  	new_anomaly->off2 =  off2;
  }
}


// When nodes that generate struts are encountered
//  in a LaTeX contents list, they are set aside until
//  mml generation has been completed.  MML_rv and the
//  out_of_flow_list are passed to the following function,
//  where MML_rv may be wrapped in an <mpadded>.

TNODE* LaTeX2MMLTree::Struts2MML( TNODE* MML_rv,
									              TNODE* out_of_flow_list ) {

  TNODE* rv =  NULL;

/*		diagnostic to test all the unit conversions
char src[64];
char dst[64];
char output[128];
double s  =  1.0;
for ( U16 i=1; i<13; i++ ) {
  for ( U16 j=1; j<13; j++ ) {

    U16 dst_type  =  GetMMLdimenType( j );
    double d  =  ConvertTeXDimen( s,i,dst_type );
	  sprintf( src,"%f",s );
    AttachUnit( src,i );
	  sprintf( dst,"%f",d );
    AttachUnit( dst,dst_type );

    sprintf( output,"%s,%s",src,dst );
    JBMLine( output );
    JBMLine( "\n" );
  }
}
*/


// \vspace<uID9.3.6>!\vspace!REQPARAM(9.3.20,GLUE)
// \vspace*<uID9.3.7>!\vspace*!REQPARAM(9.3.21,GLUE)

  if ( MML_rv ) {
	  U8 depth[64];
	  U8 height[64];
	  depth[0]  =  0;
	  height[0] =  0;

    TNODE* rover  =  out_of_flow_list;
    while ( rover ) {
      U16 uobjtype,usubtype,uID;
      GetUids( rover->zuID,uobjtype,usubtype,uID );
	    if      ( uobjtype==5 && usubtype==800 )
		    GetVSpaceFromRule( rover,depth,height );

	    rover =  rover->next;
    }	// loop thru out_of_flow_list

    if ( depth[0] || height[0] ) {
// mpadded<uID5.602.0>!mpadded!BUCKET(5.602.2,MATH,,,/mpadded,)!/mpadded!
      MML_rv  =  FixImpliedMRow( MML_rv );
      rv  =  CreateElemWithBucketAndContents( 5,602,0,2,MML_rv );
      if ( depth[0] )
        SetNodeAttrib( rv,(U8*)"depth",depth );
      if ( height[0] )
        SetNodeAttrib( rv,(U8*)"height",height );
	  } else
      rv  =  MML_rv;

  } else 	// MATH content list has only vspace in it.
    RecordAnomaly( 1003,NULL,out_of_flow_list->src_offset1,
       						out_of_flow_list->src_offset2 );

  return rv;
}



// Handle any "out-of-flow" objects.

TNODE* LaTeX2MMLTree::HandleOutOfFlowObjects( TNODE* MML_list,
											TNODE** local_oof_list,
											TNODE** out_of_flow_list,
											U16 vspace_context ) {

  TNODE* MML_rv =  MML_list;

  TNODE* the_list =  *local_oof_list;
  if ( the_list ) {
    MML_rv  =  Struts2MML( MML_rv,the_list );
    the_list  =  DisposeOutOfFlowList( the_list,5,800 );
  }
  if ( the_list ) {
    MML_rv  =  VSpace2MML( MML_rv,the_list,vspace_context );
    the_list  =  DisposeOutOfFlowList( the_list,9,3 );
  }
  if ( the_list ) {
    //MML_rv  =  Labels2MML( MML_rv,the_list );
    //move \labels to  out_of_flow_list
    TNODE* r =  the_list;
    while (r) {
      TNODE* nxt = r->next;
      U16 uobjtype, usubtype, uID;
      GetUids( r->zuID, uobjtype, usubtype, uID );
      if (usubtype == 700){
        if ( out_of_flow_list != NULL ) {
          r->next = *out_of_flow_list;
        } else {
          r->next = NULL;
        }
        *out_of_flow_list = r;
      }
      r = nxt;
    }
    //the_list  =  DisposeOutOfFlowList( the_list,0,0 );
  }

  *local_oof_list =  the_list;
  //TCI_ASSERT( *local_oof_list == NULL );

// append anything left on the_list to "the_list".
  return MML_rv;
}


/*
\begin{cases}<uID5.140.0>!\begin{cases}!_CASESLINES_!\end{cases}!
_CASESLINES_LIST(5.140.9,_CASESLINE_,5.140.10,,\end{cases},\\)
_CASESLINE__CASESLBUCKET_!&!_CASESRBUCKET_
_CASESLBUCKET_BUCKET(5.140.5,MATH,,,&,\\|\end{cases})
_CASESRBUCKET_BUCKET(5.140.7,MATH,,,\\|\end{cases},)

LaTeX's "cases" environment is more like "array" than "eqnarray".
Each line has 2 MATH buckets, each containing an independent expression.
*/

TNODE* LaTeX2MMLTree::LaTeXCases2MML( TNODE* src_cases,
									TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

// Locate the list of lines in the LaTeX source object

  U8 line_list_zuID[16];
  UidsTozuID( 5,140,9,line_list_zuID );
  TNODE* list_of_lines  =  FindObject( src_cases->parts,
                                line_list_zuID,INVALID_LIST_POS );

  TNODE* mml_rows_head  =  NULL;		// list of rows we're building
  TNODE* mml_rows_tail;

  U16 src_line_counter  =  0;
  while ( TRUE ) {     // loop down thru lines
// Find the current line in the list of lines
    TNODE* curr_line  =  FindObject( list_of_lines->parts,
                                line_list_zuID,src_line_counter );
    if ( !curr_line ) break;	// normal exit - no more lines

// Generate a list of <mtd>'s for the 2 cells in this line.

    TNODE* mml_cols_head =  NULL;
    TNODE* mml_cols_tail;

// Find the left cell in the current line.
    TNODE* TeX_cell =  FindObject( curr_line->parts,
                                (U8*)"5.140.5",INVALID_LIST_POS );

    U16 col_count =  0;
    while ( col_count < 2 ) {   // loop across columns in a line

// Translate current cell to MathML

	    TCI_BOOL do_bindings  =  TRUE;
      U16 tex_nodes_done,error_code;
	    TNODE* local_oof_list =  NULL;
      TNODE* mml_contents =  TranslateMathList( TeX_cell->contents,
                                        do_bindings,NULL,
                                        tex_nodes_done,error_code,
                                        &local_oof_list );
      if ( !mml_contents )
        mml_contents  =  MakeSmallmspace();
      mml_contents  =  HandleOutOfFlowObjects( mml_contents,
							            &local_oof_list,out_of_flow_list,2 );

      TNODE* mml_cell;
  // mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd)!/mtd!
      mml_contents  =  FixImpliedMRow( mml_contents );
      mml_cell  =  CreateElemWithBucketAndContents( 5,35,14,8,
      												mml_contents );
  // Put the current mml_cell under a cell list item node
	    U8 zlistID[32];
      UidsTozuID( 5,35,11,(U8*)zlistID );
	    U16 zln =  strlen( (char*)zlistID );
	    zlistID[zln]  =  ':';
            // itoa( col_count,(char*)zlistID+zln+1,10 );
            sprintf((char*)zlistID+zln+1, "%d", col_count);
	    TNODE* list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	    list_node->parts  =  mml_cell;
	    mml_cell->sublist_owner =  list_node;

  // Add this list item to the list we're building
      if ( !mml_cols_head )
        mml_cols_head =  list_node;
      else {
        mml_cols_tail->next  =  list_node;
		    list_node->prev =  mml_cols_tail;
	    }
      mml_cols_tail  =  list_node;

  // Find the right cell in the current line.
      TeX_cell =  FindObject( curr_line->parts,
                            (U8*)"5.140.7",INVALID_LIST_POS );
      col_count++;

    }	// loop across columns in a line

// Add current row to the list of rows we're building

/*
/mtr<uID5.35.16>
mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
_LISTOFCELLS_LIST(5.35.11,_MTCELL_,5.35.12,,/mtr|/mtable,)
_MTCELL_IF(MATH,?mtd?,5.35.14)ifEND
*/

    TNODE* mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );
	  mtr->parts->parts =  mml_cols_head;
	  mml_cols_head->sublist_owner =  mtr->parts;

// Put the current row under a node in the row list

	  U8 zlistID[32];
    UidsTozuID( 5,35,9,(U8*)zlistID );
	  U16 zln =  strlen( (char*)zlistID );
	  zlistID[zln]  =  ':';
	  // itoa( src_line_counter,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1, "%d", src_line_counter);

	  TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	  row_list_node->parts  =  mtr;
	  mtr->sublist_owner  =  row_list_node;
    if ( !mml_rows_head )
      mml_rows_head  =  row_list_node;
    else {
      mml_rows_tail->next =  row_list_node;
	    row_list_node->prev =  mml_rows_tail;
	  }
    mml_rows_tail  =  row_list_node;

    src_line_counter++;
  }

// mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
// _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)

  mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
//  SetNodeAttrib( rv,(U8*)"align",(U8*)"bottom" );
  mml_rv->parts->parts  =  mml_rows_head;
  mml_rows_head->sublist_owner =  mml_rv->parts;


  if ( mml_rv ) {
    SetNodeAttrib( mml_rv,(U8*)"columnalign",(U8*)"left" );
//  SetNodeAttrib( mml_rv,(U8*)"align",(U8*)"bottom" );

// mo for left delimiting "{"
    TNODE* left_end =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );
    SetChData( left_end,(U8*)"{",NULL );
    SetAttrsForFenceMO( left_end,9,TRUE );

    left_end->next  =  mml_rv;
	  mml_rv->prev    =  left_end;

    mml_rv  =  MMLlistToMRow( left_end );
    SetDetailNum( mml_rv,DETAILS_is_expression,1 );
    SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_INNER );
  }

  return mml_rv;
}


// Note: We handle external format here - \begin{matrix}, etc.
// The internal form - \MATRIX[m]... - is converted to an equivalent
// external parse tree, then the following function is called.

TNODE* LaTeX2MMLTree::Matrix2MML( TNODE* tex_matrix_node,
								    TNODE** out_of_flow_list,
								    U16 subtype ) {

  TNODE* mml_rv =  Array2MML( tex_matrix_node,out_of_flow_list );

// "align" and "frame" and "rowlines" and "columnlines" and "columnalign"
  SetMTableAttribs( mml_rv,FALSE,tex_matrix_node );

// These LaTeX constructs have NO {cols} arg - all columns are centered
  SetNodeAttrib( mml_rv,(U8*)"columnalign",(U8*)"center" );

// All except matrix and smallmatrix have built-in delimiters

  if ( subtype >= 712 && subtype <= 716 ) {

/* from MathML.gmr, context MATH
(<uID5.712.0,l>U0xxxx,form="prefix"   fence="true" stretchy="true" symmetric="true" lspace="0em" rspace="0em"
)<uID5.712.0,r>U0xxxx,form="postfix"  fence="true" stretchy="true" symmetric="true" lspace="0em" rspace="0em"
.
.
{<uID5.716.0,l>U0xxxx,form="prefix"   fence="true" stretchy="true" symmetric="true" lspace="0em" rspace="0em"
}<uID5.716.0,r>U0xxxx,form="postfix"  fence="true" stretchy="true" symmetric="true" lspace="0em" rspace="0em"
*/

    U8 lookup_zuID[32];
    strcpy( (char*)lookup_zuID,(char*)tex_matrix_node->zuID );
    strcat( (char*)lookup_zuID,",l" );

    U8* delim_entity;
    U8* delim_attrs;
    if ( d_mml_grammar->GetGrammarDataFromUID(lookup_zuID,
                    context_math,&delim_entity,&delim_attrs ) ) {
      if ( delim_entity && delim_entity[0] ) {
// mo for left delimiting fence
        TNODE* left_end =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );
        SetChData( left_end,delim_entity,NULL );
        if ( delim_attrs && *delim_attrs ) {
          if ( output_entities_as_unicodes ) {
            U8 unicode_buffer[128];
            if ( GetUnicodeEntity(delim_attrs,unicode_buffer) )
              SetChData( left_end,NULL,unicode_buffer );
          }
		  U8* p_comma =  (U8*)strchr( (char*)delim_attrs,',' );
          if ( p_comma ) {
            ATTRIB_REC* ar  =  ExtractAttrs( (char*)p_comma+1 );
            if ( ar ) {
              MergeMOAttribs( left_end,ar );
              DisposeAttribs( ar );
            }
          }
        }

        left_end->next  =  mml_rv;
	    mml_rv->prev    =  left_end;

        U16 zln =  strlen( (char*)lookup_zuID );
        lookup_zuID[zln-1]  =  'r';
        if ( d_mml_grammar->GetGrammarDataFromUID(lookup_zuID,
                    context_math,&delim_entity,&delim_attrs ) ) {
          if ( delim_entity && delim_entity[0] ) {
// mo for right delimiting fence
            TNODE* right_end  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );
            SetChData( right_end,delim_entity,NULL );
            if ( delim_attrs && *delim_attrs ) {
              if ( output_entities_as_unicodes ) {
                U8 unicode_buffer[128];
                if ( GetUnicodeEntity(delim_attrs,unicode_buffer) )
                  SetChData( right_end,NULL,unicode_buffer );
              }
		      U8* p_comma =  (U8*)strchr( (char*)delim_attrs,',' );
              if ( p_comma ) {
                ATTRIB_REC* ar  =  ExtractAttrs( (char*)p_comma+1 );
                if ( ar ) {
                  MergeMOAttribs( right_end,ar );
                  DisposeAttribs( ar );
                }
              }
            }

            mml_rv->next    =  right_end;
	        right_end->prev =  mml_rv;
	      }
        }

        mml_rv  =  MMLlistToMRow( left_end );
      }

	} else
	  TCI_ASSERT(0);

  }   // if ( subtype >= 712 && subtype <= 716 )

  if ( subtype == 711 ) {  // \smallmatrix - increment scriptlevel
    mml_rv =  FixImpliedMRow( mml_rv );
    mml_rv =  CreateElemWithBucketAndContents( 5,600,0,2,mml_rv );
    SetNodeAttrib( mml_rv,(U8*)"scriptlevel",(U8*)"+1" );
  }

  SetDetailNum( mml_rv,DETAILS_is_expression,1 );
  SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_INNER );

  return mml_rv;
}


// \multicolumn<uID5.412.0>!\multicolumn!
//   REQPARAM(5.412.1,TEXT)
//   REQPARAM(5.412.2,COLS)
//   REQPARAM(5.412.3,INHERIT)

TNODE* LaTeX2MMLTree::Multicolumn2MML( TNODE* tex_node,
									TNODE** out_of_flow_list,
									TCI_BOOL in_math,
									U16& mcol_align,
									U16& cols_spanned ) {

  TNODE* mml_rv =  NULL;

  if ( tex_node && tex_node->parts ) {

// Get number of columns spanned

    TNODE* ncols_bucket =  FindObject( tex_node->parts,
					          (U8*)"5.412.1",INVALID_LIST_POS );
    if ( ncols_bucket && ncols_bucket->contents ) {
      TNODE* text =  ncols_bucket->contents;
      if ( text->var_value )
	    cols_spanned  =  atoi( (char*)text->var_value );
	}

// Get specified column alignment

    TNODE* cols_bucket =  FindObject( tex_node->parts,
					          (U8*)"5.412.2",INVALID_LIST_POS );
	if ( cols_bucket && cols_bucket->contents ) {
      TNODE* c_rover  =  cols_bucket->contents;
      U16 uobjtype,usubtype,id;
      GetUids( c_rover->zuID,uobjtype,usubtype,id );
      if ( uobjtype==16 && usubtype==4 )
        mcol_align  =  id;
	}

    TNODE* bucket =  FindObject( tex_node->parts,
					          (U8*)"5.412.3",INVALID_LIST_POS );
	if ( bucket && bucket->contents ) {
	  TNODE* local_oof_list =  NULL;
      TNODE* contents =  NULL;
      if ( in_math ) {
  // Translate current cell to MathML
	    TCI_BOOL do_bindings  =  TRUE;
        U16 tex_nodes_done,error_code;
        contents =  TranslateMathList( bucket->contents,
							            do_bindings,NULL,
							            tex_nodes_done,
							            error_code,
							            &local_oof_list );
	  } else {
	    contents  =  TextInMath2MML( bucket->contents,
	  					    	&local_oof_list,FALSE,FALSE );
	  }
      if ( !contents )
        contents  =  MakeSmallmspace();
      mml_rv  =  HandleOutOfFlowObjects( contents,
   						      &local_oof_list,out_of_flow_list,2 );
	}		// if ( bucket && bucket->contents )

  }

  return mml_rv;
}


// \hdotsfor<uID5.431.0>
// OPTPARAM(5.431.1,NONLATEX) - spacing multiplier
// REQPARAM(5.431.2,NONLATEX) - number of columns spanned

TNODE* LaTeX2MMLTree::Hdotsfor2MML( TNODE* tex_node,
									TCI_BOOL in_math,
									U16& cols_spanned ) {

  TNODE* mml_rv =  NULL;

  if ( tex_node && tex_node->parts ) {
// Get number of columns spanned

    TNODE* ncols_bucket =  FindObject( tex_node->parts,
							    (U8*)"5.431.2",INVALID_LIST_POS );
	if ( ncols_bucket && ncols_bucket->contents ) {
      TNODE* text =  ncols_bucket->contents;
      if ( text->var_value )
		cols_spanned  =  atoi( (char*)text->var_value );
	}

// Get spacing multiplier
    TNODE* mult_bucket  =  FindObject( tex_node->parts,
							    (U8*)"5.431.1",INVALID_LIST_POS );
	if ( mult_bucket && mult_bucket->contents ) {

    }

    mml_rv  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
    SetChData( mml_rv,(U8*)"...",NULL );
  }

  return mml_rv;
}


// Occasionally, LaTeX MATH buckets contain no math, like displays
//  that are used to center something that isn't really math.
// Currently, I don't generate MML in such cases.
//  The caller needs to handle the translation.

TCI_BOOL LaTeX2MMLTree::BucketContainsMath( TNODE* contents ) {

  TCI_BOOL rv =  TRUE;		// assumed

// Objects that are specifically NOT math.
  U16 n_spaces    =  0;			// I'm tracking more things
  U16 n_tabulars  =  0;			// than needed by the logic
  U16 n_text_runs =  0;			// currently employed here.
// Everything else is considered to be math.
  U16 n_math_objects  =  0;

// iterate first level contents list
  TNODE* rover  =  contents;
  while ( rover && !n_math_objects ) {
    U16 uobj,usub,uID;
    GetUids( rover->zuID,uobj,usub,uID );
	  if ( uobj==5 ) {			// LaTeX schemata
	    if      ( usub==90  && uID==0 )
  // \text{...}
	      n_text_runs++;
	    else if ( usub>=450 && usub<=461 )
  // \textup{...} .. \textcircled{...}
	      n_text_runs++;
	    else if ( usub==490 || usub==491 )
  // \begin{tabular}<uID5.490.0>  \begin{tabular*}<uID5.491.0>
	      n_tabulars++;
	    else if ( usub==37 )
  // \TABLE<uID5.37.0>!\TABLE!_COLSANDROWS__EXTALIGN__TWIDTH_...
	      n_tabulars++;
	    else						// all other schemata
	      n_math_objects++;
	  } else if ( uobj==9 ) {		// LaTeX spaces
  // \smallskip<uID9.3.1>
	    if ( usub==1 || usub==3 )
	      n_spaces++;
	    else
	      n_math_objects++;
	  } else						// everything else
	    n_math_objects++;

	  rover =  rover->next;
  }		// loop thru MATH bucket

  if ( n_math_objects == 0
  &&   n_text_runs    == 0
  &&   n_tabulars     == 0 ) {	// Run contains no math!!
	  //TCI_ASSERT(0);
	  rv  =  FALSE;
  }

  return rv;
}



// In LaTeX, \vspace{GLUE} can be used to typeset
//  more or less whitespace below a run of MATH.
//
// Note that the arg here is GLUE.  This code doesn't handle all
//  the generality of TeX's glue.  The syntax handled here is
//  [+|-]digit(s)[.digit(s)]TeX_unit
//  6pt
//  -1.5in
//  etc.
//
// Note that handling \vspace*{} ( space that is never removed - 
//  not even when the object falls at the bottom of a page )
//  and \vspace{} here is FUNDAMENTLY INCORRECT if the cmd
//  occurs in the first level of MATH.  Such objects belong
//  to the line in which they are typeset.  As with all stretchy
//  objects, they MUST be handed up to the caller,
//  and on up to the process managing layout.

void LaTeX2MMLTree::GetValueFromGlue( TNODE* glue_bucket,
                                U8* val,TCI_BOOL is_relative,
                                I16& val_in_math_units ) {

// Locate key nodes in the contents of the GLUE bucket

  TCI_BOOL is_minus =  FALSE;
  TNODE* val_node   =  NULL;
  TNODE* unit_node  =  NULL;
  TNODE* glue_crover  =  glue_bucket->contents;
  while ( glue_crover ) {
    U16 uobjtype,usubtype,id;
    GetUids( glue_crover->zuID,uobjtype,usubtype,id );

    if      ( uobjtype==888 && usubtype==10 && id==0 )
      val_node  =  glue_crover;
    else if ( uobjtype==3 && usubtype==13 ) {
      if      ( id==60 )
        is_minus  =  FALSE;
      else if ( id==61 )
        is_minus  =  TRUE;
      else
        TCI_ASSERT(0);
    } else if ( uobjtype==14 && usubtype==2 )
      unit_node =  glue_crover;
    else
      TCI_ASSERT(0);

    glue_crover =  glue_crover->next;
  }

  double magnitude  =  0.0;
  if ( val_node ) {
    magnitude =  atof( (char*)val_node->src_tok );
    if ( is_minus )
      magnitude =  -magnitude;
  }

  U16 TeX_unit_ID =  0;
  if ( unit_node )
    TeX_unit_ID =  TeXUnitToID( (char*)unit_node->src_tok );
  else
    TCI_ASSERT(0);

  U16 mml_dest_units  =  GetMMLdimenType( TeX_unit_ID );
  if ( magnitude < -0.00001 || magnitude > 0.00001 ) {

// z_TeX_units =  "bp cc cm dd em ex in mm mu pc pt sp ";
//                 1  2  3  4  5  6  7  8  9

    double tmp  =  0.0;
    if      ( TeX_unit_ID == 5 )    // em's
      tmp =  magnitude * 18.0;
    else if ( TeX_unit_ID == 9 )    // math units
      tmp =  magnitude;
    else if ( TeX_unit_ID == 6 )    // ex's
      tmp =  9.0 * magnitude;     // bogus
    else {
      tmp =  ConvertTeXDimen( magnitude,TeX_unit_ID,7 );
      tmp =  tmp * 120.0;         // bogus
    }
    tmp +=  0.5;
	  sprintf( (char*)val,"%f",tmp );
    val_in_math_units =  atoi( (char*)val );


    magnitude =  ConvertTeXDimen( magnitude,TeX_unit_ID,mml_dest_units );
    if ( is_minus )
	    sprintf( (char*)val,"%f",magnitude );
    else {
      if ( is_relative ) {
        val[0] =  '+';
        sprintf( (char*)val+1,"%f",magnitude );
      } else
	      sprintf( (char*)val,"%f",magnitude );
    }
    AttachUnit( (char*)val,mml_dest_units );

  } else {
    val[0]  =  0;
    val_in_math_units =  0;
  }

/*
char buffer[256];
sprintf( (char*)buffer,"%f, rv = %s, mathunits = %d\n",magnitude,val,val_in_math_units );
JBMLine( (char*)buffer );
*/

}


//\FORMULA<uID5.419.1>!\FORMULA!
//  REQPARAM(5.419.2,MATH)      - the formula
//  REQPARAM(5.419.3,MATH)      - the result (this is displayed)
//  REQPARAM(5.419.4,NONLATEX)  - the computation, evaluate, etc.

TNODE* LaTeX2MMLTree::FORMULA2MML( TNODE* TeX_FORMULA_node,
									                  TNODE** out_of_flow_list ) {

  TNODE* mml_rv =  NULL;

  if ( TeX_FORMULA_node && TeX_FORMULA_node->parts ) {

    TNODE* mml_formula  =  NULL;
    TNODE* formula_bucket =  FindObject( TeX_FORMULA_node->parts,
									              (U8*)"5.419.2",INVALID_LIST_POS );
    if ( formula_bucket && formula_bucket->contents ) {
      U16 tex_nodes_done,error_code;
  	  TCI_BOOL do_bindings  =  TRUE;  // the formula component
		  TNODE* local_oof_list =  NULL;  // is a complete expression
      mml_formula =  TranslateMathList( formula_bucket->contents,
                                    do_bindings,NULL,
									                  tex_nodes_done,error_code,
									                  &local_oof_list );
// There shouldn't be any struts, labels, etc. in the formula.
//  It's not rendered in the LaTeX output.
	    if ( local_oof_list )
        DisposeTList( local_oof_list );
    }

    TNODE* mml_result =  NULL;
    TNODE* result_bucket  =  FindObject( TeX_FORMULA_node->parts,
									              (U8*)"5.419.3",INVALID_LIST_POS );
    if ( result_bucket && result_bucket->contents ) {
      U16 tex_nodes_done,error_code;
  	  TCI_BOOL do_bindings  =  TRUE;
		  TNODE* local_oof_list =  NULL;
      mml_result  =  TranslateMathList( result_bucket->contents,
                                    do_bindings,NULL,
									                  tex_nodes_done,error_code,
									                  &local_oof_list );
// Generally, the result component of a \FORMULA should be
//  generated by an engine.  It shouldn't contain any out_of_flows.
      mml_result  =  HandleOutOfFlowObjects( mml_result,
       									     &local_oof_list,out_of_flow_list,3 );
    }


// Put the "formula" and it's "result" in the parts of an <maction>
// maction<uID5.604.0>!maction!reqELEMENT(5.604.1)reqELEMENT(5.604.2)!/maction!

    mml_rv  =  CreateElemWithBucketAndContents( 5,604,0,1,mml_formula );
	  TNODE* parts_tail =  mml_rv->parts;

// append the "result" to the parts of the returned <maction> 

    TNODE* next_part  =  CreateBucketWithContents(5,604,2,mml_result );
    parts_tail->next  =  next_part;
    next_part->prev   =  parts_tail;
	  parts_tail  =  next_part;

    SetNodeAttrib( mml_rv, (U8*)"actiontype", (U8*)"toggle" );
    SetNodeAttrib( mml_rv, (U8*)"selection", (U8*)"2" );

  } else {
    TCI_ASSERT(0);
  }

  if ( mml_rv )   
    SetDetailNum( mml_rv, DETAILS_is_expression, 1 );


  return mml_rv;
}



// Handle semantic comments in MATH - only found in external format.

TNODE* LaTeX2MMLTree::MathComment2MML( TNODE* TeX_comment_node,
												                        U16 &advance ) {

  TNODE* mml_rv =  NULL;

  advance =  1;

// Handle %TCIMACRO{...
// 		    %............
// 		    %...........}
//        %BeginExpansion

  U8* tcimacro_data =  TCIMacroToBuffer( TeX_comment_node );

  if ( tcimacro_data ) {  // the comment is a %TCIMACRO

// We generate mml for both the hidden internal form AND
//  the exposed external LaTeX form of the object.
// At present, I am scripting mml for the external object only,
//  EXCEPT when the %TCIMACRO contains \FORMULA.

		TNODE* MML_expansion;
    TNODE* hidden_internal_markup =  GetTCIMacroParts( tcimacro_data,
                                            TeX_comment_node,
								                            &MML_expansion,advance );
	  delete tcimacro_data;

    TCI_BOOL done =  FALSE;
    if ( hidden_internal_markup ) {
      TNODE* formula  =  FindObject( hidden_internal_markup,
					                      (U8*)"5.419.1",INVALID_LIST_POS );
      if ( formula ) {
        TCI_BOOL do_bindings  =  TRUE;
        U16 tex_nodes_done,error_code;
        TNODE* local_oof_list =  NULL;
        mml_rv =  TranslateMathList( hidden_internal_markup,
                     		do_bindings,NULL,tex_nodes_done,
                         		error_code,&local_oof_list );
	      if ( local_oof_list )
          DisposeTList( local_oof_list );
        done  =  TRUE;
        if ( MML_expansion )
          DisposeTList( MML_expansion );
      } else {

      }

      DisposeTList( hidden_internal_markup );
    }

    if ( !done )
      mml_rv  =  MML_expansion;

  } else {		// Not a %TCIMACRO comment
  // Nothing implemented re any non-TCIMACRO comments, for now.
  // Just discarding here.
  }

  return mml_rv;
}


// A comment in TEXT may contain %TCIMacro{} - only found in external format.
// %TCIMACRO{...
// %............
// %...........}
// %BeginExpansion
// Some LaTeX TEXT 
// %EndExpansion

void LaTeX2MMLTree::ProcessTextComment( TNODE* TeX_comment_node,
												                  U32& unicode,
												                    U16& advance ) {

  unicode =  0;
  advance =  1;

  U8* tcimacro_data =  TCIMacroToBuffer( TeX_comment_node );

  if ( tcimacro_data ) {  // this comment is a %TCIMACRO
    TNODE* hidden_internal_markup =  GetTCIMacroParts( tcimacro_data,
                                     TeX_comment_node,NULL,advance );
    if ( hidden_internal_markup ) {
// At present, I'm just looking for a \U{}.
      U16 uobjtype,usubtype,id;
      GetUids( hidden_internal_markup->zuID,uobjtype,usubtype,id );
      if ( uobjtype==5 && usubtype==298 && id==0 )
        unicode =  ExtractUNICODE( hidden_internal_markup );
      else
        TCI_ASSERT(0);
      DisposeTList( hidden_internal_markup );
    }
	  delete tcimacro_data;

  } else {		// Not a %TCIMACRO comment
// Nothing implemented re any non-TCIMACRO comments, for now.
// Just discarding here.
  }

}



U8* LaTeX2MMLTree::TCIMacroToBuffer( TNODE* comment_node ) {

  U8* rv  =  NULL;

  U8* TCIMACRO  =  (U8*)"%TCIMACRO";

  U32 buf_lim;
  U32 buf_i;
                     
  TCI_BOOL macro_found  =  FALSE;
  CVLINE_REC* cv_rover  =  comment_node->cv_list;
  while ( cv_rover ) {
	  if ( cv_rover->cvline ) {
      if ( !macro_found ) {
        if ( !strncmp((char*)cv_rover->cvline,(char*)TCIMACRO,9) ) {
          macro_found  =  TRUE;
          buf_lim =  1024;
		      rv  =  (U8*)TCI_NEW( char[buf_lim] );
          buf_i =  0;
	      }
	    }

      if ( macro_found ) {
        U8* new_bytes =  cv_rover->cvline;
        U32 n_bytes   =  strlen( (char*)new_bytes );
		    U32 save_i  =  buf_i;
        rv  =  AppendBytesToBuffer( rv,buf_lim,buf_i,
        								            new_bytes,n_bytes );
		    if ( save_i ) {
		      TCI_ASSERT( rv[save_i] == '%' );
		      rv[save_i] = '\n';
	      }
	    }

	  }		// if ( cv_rover->cvline )

	  cv_rover  =  cv_rover->next;
  }		// while ( cv_rover )

  return rv;
}


// Locate the components of a TCIMacro{}/expansion structure.
// lparser is called to produce a parse tree for the hidden
// internal data contained %TCIMacro{hidden internal data}.

TNODE* LaTeX2MMLTree::GetTCIMacroParts( U8* zTCIMacro,
										                    TNODE* comment_node,
										                    TNODE** MML_expansion,
										  	                U16& advance ) {

  if ( MML_expansion )
    *MML_expansion  =  NULL;
  advance   =  1;

  TNODE* hidden_LaTeX =  NULL;    // our return value


  TCI_BOOL hasBeginExpansion  =  FALSE;
  TCI_BOOL isTeXButton  =  FALSE;


  U16 s_off,e_off;
  if ( LocateBraces(zTCIMacro,s_off,e_off) ) {

  // Advance to contents of %TCIMACRO{..contents..}

    char* ptr =  (char*)zTCIMacro + s_off + 1;
	  while ( *ptr ) {
	    if ( *ptr <= ' ' ) ptr++;
	    else break;
	  }

    if ( !strncmp(ptr,"\\TeXButton{",11) )
      isTeXButton =  TRUE;

    if ( strstr((char*)zTCIMacro+e_off,"BeginExpansion") )
      hasBeginExpansion   =  TRUE;

// Copy TCIMACRO contents to a buffer

	  *(zTCIMacro+e_off)  =  0;   // NULL terminate

    U8* data  =  (U8*)TCI_NEW( char[e_off-s_off] );
    strcpy( (char*)data,ptr );

// Build LaTeX parse tree for the body of the TCIMACRO

	  U8* local_context   =  (U8*)"MATH";
    TCI_BOOL is_clipbrd =  FALSE;
    U16 nlog_msgs;
    hidden_LaTeX =  lparser->ParseBytes( local_context,
    										            data,is_clipbrd,nlog_msgs );

    delete data;

  } else				// LocateBraces failed!
    TCI_ASSERT( 0 );

// There is an object boundary issue here.
// %TCIMacro{\FORMULA{...}}
//			 \HTML{...}
//			 \UNICODE{...}
//			 \TeXButton{...}
//			 .
//			 .
// %BeginExpansion
// some visible LaTeX
// %EndExpansion
//
// Generally, a %TCIMacro gives two markups for a single,
//  complete LaTeX/WorkPlace object.  In this case, we want
//  to map to a single XML element with the visible LaTeX
//  expansion moved to a sub-element.
// However, this may not be the case when %TCIMacro contains
//  \TeXButton{...}
//  The expansion may contain something like "\begin{minipage}"
//  It would be inappropriate to shift part a LaTeX object
//  to a sub-element.  Hence we don't subordinate the expansion
//  in the \TeXButton{} case.


  if ( isTeXButton ) {
/*
    rv->attrib_list =  attrman->AppendAttrIDval( rv->attrib_list,
		AID_isTeXButton,AVAL_true,0 );
*/

  } else if ( hasBeginExpansion ) {

	  U16 local_advance;
    TNODE* ender  =  LocateEndExpansion( comment_node,local_advance );

	  TCI_ASSERT( ender );

    if ( local_advance > 1 ) {

      TNODE* save =  ender;
		  if ( ender )
	      ender->prev->next =  NULL;

      if ( MML_expansion ) {
        TCI_BOOL do_bindings  =  TRUE;
        U16 tex_nodes_done,error_code;
        TNODE* local_oof_list =  NULL;
        TNODE* cont =  TranslateMathList( comment_node->next,
                           		do_bindings,NULL,tex_nodes_done,
                           		error_code,&local_oof_list );
      //cont  =  HandleOutOfFlowObjects( cont,
			//	                &local_oof_list,out_of_flow_list,3 );

        if ( cont )
          *MML_expansion  =  cont;
      }

	    if ( ender ) {
	      ender->prev->next =  save;
        advance =  local_advance + 1;
	    } else 	// no ender
        advance =  local_advance;

	  }		// if ( local_advance > 1 )

  }		//  if ( hasBeginExpansion )

  return hidden_LaTeX;
}



//  Comment ->	TNODE -> ...  -> Comment
//    %BeginExpansion			   %EndExpansion

TNODE* LaTeX2MMLTree::LocateEndExpansion( TNODE* start_node,
	                                        	U16& advance ) {

  advance =  0;

// Note that "start_node" contains %BeginExpansion

  TNODE* trover =  start_node;
  while ( trover ) {		// loop thru LaTeX tnodes
    U16 uobjtype,usubtype,uID;
    GetUids( trover->zuID,uobjtype,usubtype,uID );
    if ( uobjtype == 777 && usubtype == 0 ) {   // LaTeX comment
      CVLINE_REC* cv_rover  =  trover->cv_list;
      while ( cv_rover ) {	// loop thru comment lines
	      if ( cv_rover->cvline ) {
          if ( !strcmp((char*)cv_rover->cvline,"%EndExpansion") )
		        return trover;
	      }		// if ( cv_rover->cvline )
	      cv_rover  =  cv_rover->next;
      }		// loop thru lines of one comment node

	  }		// comment node processing

    trover  =  trover->next;
    advance++;
  }				// loop thru LaTeX nodes looking for
				//  %EndExpansion
  return NULL;
}



//Function to locate the delimiting braces { and } in stuff like
//  TCIDATA{PageSetup=72,72,72,72,0}

TCI_BOOL LaTeX2MMLTree::LocateBraces( U8* TCIData,
                                U16& s_off,U16& e_off ) {

  TCI_BOOL rv =  FALSE;

  I16 level =  0;
  U8 ch;
  U8* ptr =  TCIData;
  while ( ch = *ptr ) {
    if        ( ch=='\\') {
	    ptr++;
    } else if ( ch=='{' ) {
	    if ( level==0 )
	      s_off  =  ptr - TCIData;
	    level++;
    } else if ( ch=='}' ) {
	    TCI_ASSERT( level>0 );
	    level--;
	    if ( level==0 ) {
	      rv  =  TRUE;
	      e_off  =  ptr - TCIData;
		    break;
	    }
	  }
	  ptr++;
  }

  return rv;
}



void LaTeX2MMLTree::SetMTRAttribs( TNODE* mtr_node,
                           U16 alignment,U16 ncols_spanned ) {


  if ( ncols_spanned > 1 ) {
    U8 zval[32];
    // JCS non-standard: itoa( ncols_spanned,(char*)zval,10 );
    sprintf((char*)zval, "%d", ncols_spanned);
    SetNodeAttrib( mtr_node,(U8*)"columnspan",(U8*)zval );
  }
  if ( alignment && alignment<=3 ) {
    U8* zval;
    if      ( alignment==1 ) zval =  (U8*)"left";
    else if ( alignment==2 ) zval =  (U8*)"center";
    else if ( alignment==3 ) zval =  (U8*)"right";
    SetNodeAttrib( mtr_node,(U8*)"columnalign",zval );
  }
}



// EqnArrays are composed of a series of consecutive lines,
//  each containing alignment markers, "&", used by LaTeX
//  renderers to align particular elements from line to line.
//          x &=& 1
//       2x+1 &=& 123
//  right  center  left     // for an eqnarray environment
//      ^    ^     ^
// The groups being shifted to achieve the desired alignment
//  move left or right, or are centered, depending eqnarray_ilk.
//
// <mtd groupalign="right center left">
//   <maligngroup/> x <maligngroup/> = <maligngroup/> 1 
// </mtd>

// Returns the group alignment pattern for each eqnarray variant.

// WARNING! In TeX Explorer, groupalign="right" means that
//  alignment occurs at the right end of the group - the aligned
//  group goes left of the alignment point.

void LaTeX2MMLTree::GetEQNAlignVals( U16 eqnarray_ilk,
                                        U8* align_vals,
                                            U16 lim ) {

  align_vals[0] =  0;

  char* zleft   =  "left";
  char* zcenter =  "center";
  char* zright  =  "right";

  char* alignments[8];      // only need 6 here

  U16 group_counter  =  0;
  switch ( eqnarray_ilk ) {

    case  39  :     // internal format should be converted
      TCI_ASSERT(0);//   to external before we get here.
    break;

// non-nested eqnarrays

    case TENV_eqnarray  :
	  case TENV_eqnarraystar  :
      alignments[group_counter++] =  zright;
      alignments[group_counter++] =  zcenter;
      alignments[group_counter++] =  zleft;
    break;
	  case 123  :		      // TENV_align
	  case 124  :		      // \begin{align*}<uID5.124.0>!
      alignments[group_counter++] =  zright;
      alignments[group_counter++] =  zleft;
    break;

	  case 125  :		      // TENV_alignat
	  case 126  :		      // TENV_alignatstar
	  case 127  :		      // TENV_xalignat
	  case 128  :		      // TENV_xalignatstar
	  case 129  :		      // TENV_xxalignat
	  case 130  :		      // TENV_xxalignatstar
      alignments[group_counter++] =  zright;
      alignments[group_counter++] =  zleft;
      alignments[group_counter++] =  zright;
      alignments[group_counter++] =  zleft;
      alignments[group_counter++] =  zright;
      alignments[group_counter++] =  zleft;
    break;

	  case 131  :		      // TENV_gather
	  case 132  :		      // TENV_gatherstar
      alignments[group_counter++] =  zcenter;
    break;
	  case 133  :		      // TENV_multline
	  case 134  : 	      // TENV_multlinestar
      alignments[group_counter++] =  zcenter;
    break;

// nested eqnarrays - occur in MATH
//  case 124            :
	  case 140            :   // cases
      alignments[group_counter++] =  zleft;
      alignments[group_counter++] =  zleft;
    break;
	  case TENV_split     :
      alignments[group_counter++] =  zright;
      alignments[group_counter++] =  zleft;
    break;
	  case TENV_gathered  :
      alignments[group_counter++] =  zcenter;
    break;
    case TENV_aligned   :
      alignments[group_counter++] =  zright;
      alignments[group_counter++] =  zleft;
    break;

    case TENV_alignedat :
      alignments[group_counter++] =  zcenter;
    break;

    default :
      TCI_ASSERT(0);
    break;
  }

// Generate the string of align values to be returned

  U16 di    =  0;
  U16 slot  =  0;
  while ( slot < group_counter ) {
    U16 delta =  strlen( alignments[slot] );
    if ( di + 1 + delta < lim ) {
      if ( di )
        align_vals[ di++ ]  =  ' ';
      strcpy( (char*)align_vals+di,alignments[slot] );
      di  +=  strlen( alignments[slot] );
    }
    slot++;
  }

}



TNODE* LaTeX2MMLTree::AddEQNAttribs( TNODE* mml_eqn_node,
                                            U16 usubtype ) {

  TNODE* mml_rv =  mml_eqn_node;
  if ( mml_rv ) {
    if ( usubtype == 140 ) {	// cases
      TCI_ASSERT(0);
    } else if ( usubtype >= TENV_eqnarray
    &&          usubtype <= 134 ) {	// TENV_multlinestar   134
      mml_rv  =  FixImpliedMRow( mml_rv );
      mml_rv  =  CreateElemWithBucketAndContents( 5,600,0,2,mml_rv );
      SetNodeAttrib( mml_rv,(U8*)"displaystyle",(U8*)"true" );
      SetDetailNum( mml_rv,DETAILS_style,2 );
    } else {
// 5.141. is split, 5.142. is gathered, 5.143. is aligned
// these are MATH only environments - always occur within MATH
	  }

  } // if ( mml_rv )

  return mml_rv;
}


// TeX Explorer, August 01, doesn't implement "displaystyle".
// Lots of problems here.  With TeX Explorer, \sum, etc.
//  are NOT aligned vertically on the math axis unless
//  they are made "stretchy".

TNODE* LaTeX2MMLTree::SetBigOpSize( TNODE* base_op,
                                        U8 tex_size_attr ) {

  TNODE* mml_rv =  base_op;

// auto-size<uID7.2.1>largeop="true"
// display-size<uID7.2.2>largeop="false" stretchy="true" maxsize="1.4" minsize="1.4"
// text-size<uID7.2.3>largeop="false" stretchy="false"

  U8* zuID;
  if      ( tex_size_attr == 'l' )
    zuID  =  (U8*)"7.2.2";
  else if ( tex_size_attr == 's' )
    zuID  =  (U8*)"7.2.3";
  else if ( tex_size_attr == 'a' ) {
    if ( renderer_implements_displays ) {
      zuID  =  (U8*)"7.2.1";
// It's not clear that the action above is enough to handle
//   auto sized BigOps even if the renderer implements "displaystyle".
//   math axis alignment may no be handled by the renderer.
    } else {
      if ( in_display )
        zuID  =  (U8*)"7.2.2";
      else
        zuID  =  (U8*)"7.2.3";
    }
  }


  U8* dest_zname;
  U8* d_template;
  if ( d_mml_grammar->GetGrammarDataFromUID(
					                      zuID,context_math,
   							                &dest_zname,&d_template) ) {
    if ( d_template && *d_template )
      SetMMLAttribs( base_op,d_template );
    else
      TCI_ASSERT(0);
  }

  return mml_rv;
}



// Does an <mtext> contain only spaces?

TCI_BOOL LaTeX2MMLTree::MTextIsWhiteSpace( TNODE* mtext ) {

  TCI_BOOL rv =  FALSE;

  if ( mtext && mtext->details )
    if ( mtext->details->space_width != UNDEFINED_DETAIL )
      rv  =  TRUE;

  return rv;
}



TCI_BOOL LaTeX2MMLTree::MTextIsWord( TNODE* mtext ) {

  TCI_BOOL rv =  FALSE;

  if ( mtext && mtext->var_value ) {
    rv  =  TRUE;
    U8* ptr =  mtext->var_value;
	U8 ch;
	while ( ch = *ptr ) {
	  if        ( ch>='0' && ch<='9' ) {
	  } else if ( ch>='A' && ch<='Z' ) {
	  } else if ( ch>='a' && ch<='z' ) {
	  } else if ( ch==' ' ) {
	  } else {
        rv  =  FALSE;
		break;
	  }
	  ptr++;
    }
  }

  return rv;
}



TCI_BOOL LaTeX2MMLTree::ContainsAllCaps( TNODE* TeX_decoration_node ) {

  U16 n_nodes =  0;
  U16 n_caps  =  0;

  TNODE* math_bucket  =  FindObject( TeX_decoration_node->parts,
									(U8*)"5.13.1",INVALID_LIST_POS );

  if ( math_bucket && math_bucket->contents ) {
    TNODE* rover  =  math_bucket->contents;
    while ( rover ) {
      n_nodes++;
      TCI_BOOL go_on  =  FALSE;
      U16 uobjtype,usubtype,uID;
      GetUids( rover->zuID,uobjtype,usubtype,uID );
      if        ( uobjtype==3 && usubtype==2 ) {
        go_on  =  TRUE;
      } else if ( uobjtype==5 && usubtype==51 && uID==2 ) {  // <msup>
//  check for "A^{\prime}" here
        if ( IsPrimedPoint(rover) )
          go_on  =  TRUE;
      }

      if ( go_on ) {
        n_caps++;
        rover =  rover->next;
      } else
        break;
    }


  } else
    TCI_ASSERT(0);

  if ( n_nodes > 1 && n_nodes == n_caps )
    return TRUE;
  else
    return FALSE;
}



// bind runs like  &ang; A B C

TNODE* LaTeX2MMLTree::BindGeometryObjs( TNODE* MML_list ) {

  TNODE* mml_rv =  MML_list;


  TNODE* MML_rover  =  MML_list;
  while ( MML_rover ) {		// loop thru contents nodes

    U16 objclass,subclass,id;
    GetUids( MML_rover->zuID,objclass,subclass,id );

    TCI_BOOL try_coelesce =  FALSE;
    if ( MML_rover->details
    &&   MML_rover->details->is_geometry == 1 )   // \triangle
      try_coelesce  =  TRUE;
    else if   ( objclass==3 && subclass==201 && id==1 ) {
      U16 zln =  MML_rover->v_len;
      if ( zln==1 ) {
          U8 ch =  MML_rover->var_value[0];
          if ( ch>='A' && ch<='Z' )
            try_coelesce  =  TRUE;
      }
    } else if ( objclass==5 && subclass==50 && id==2 ) {  // <msub>
      try_coelesce  =  IsSubscriptedPoint( MML_rover );
    } else if ( objclass==5 && subclass==51 && id==2 ) {  // <msup>
      try_coelesce  =  IsPrimedPoint( MML_rover );
    }

    if ( try_coelesce ) {
  	  U16 node_to_nest;
	    if ( SpanGeometricObj(MML_rover,node_to_nest) ) {
        TNODE* new_mrow;
		    mml_rv  =  NestNodesInMrow( mml_rv,NULL,MML_rover,TRUE,
                                node_to_nest,&new_mrow );
		    if ( new_mrow ) {
          SetDetailNum( new_mrow,DETAILS_is_expression,1 );
		      MML_rover =  new_mrow;
		    } else
		      TCI_ASSERT(0);
	    }
    }     // if ( try_coelesce )


    MML_rover =  MML_rover->next;
  }       // loop thru MML node list

  return mml_rv;
}



TCI_BOOL LaTeX2MMLTree::SpanGeometricObj( TNODE* MML_rover,
                                            U16& nodes_to_nest ) {

  nodes_to_nest =  1;

  if ( MML_rover->next
  &&   MML_rover->next->details
  &&   MML_rover->next->details->delimited_group != UNDEFINED_DETAIL ) {

    U16 uobj,usub,uid;
    GetUids( MML_rover->zuID,uobj,usub,uid );
    if ( uobj==3 && usub==201 && uid==1 ) { // <mi>
      U16 zln =  MML_rover->v_len;
      if ( zln==1 ) {
        U8 ch =  MML_rover->var_value[0];
        if ( ch == 'P' ) {
          GROUP_INFO gi;
          GetGroupInfo( MML_rover->next,gi );
          if ( gi.opening_delim[0] == '(' )
          if ( gi.closing_delim[0] == ')' )
          if ( gi.separator_count ) // has ,s or ;s
          if ( gi.operator_count == gi.separator_count )
          if ( gi.n_interior_nodes == 2*gi.separator_count +1 ) {
            nodes_to_nest++;
            return TRUE;
          }
        }
      }
    }
  }


  TNODE* rover  =  MML_rover->next;

  while ( rover ) {
    TCI_BOOL do_join  =  FALSE;

    U16 objclass,subclass,id;
    GetUids( rover->zuID,objclass,subclass,id );

    if ( rover->details
    &&   rover->details->is_Greek == 1 ) {
      do_join  =  TRUE;

    } else if ( objclass==3 && subclass==201 && id==1 ) { // <mi>
      U16 zln =  rover->v_len;
      if ( zln==1 ) {
        U8 ch =  rover->var_value[0];
        if      ( ch>='a' && ch<='z' )
          do_join  =  TRUE;
        else if ( ch>='A' && ch<='Z' )
          do_join  =  TRUE;
      }

    } else if ( objclass==3 && subclass==202 && id==1 ) { // <mn>
      do_join  =  TRUE;

    } else if ( objclass==5 && subclass==50 && id==2 ) {  // <msub>
//  check for "P_{1}" here
      do_join  =  IsSubscriptedPoint( rover );

    } else if ( objclass==5 && subclass==51 && id==2 ) {  // <msup>
//  check for "A^{\prime}" here
      do_join  =  IsPrimedPoint( rover );
    }

    if ( do_join ) {
      nodes_to_nest++;
      rover =  rover->next;
    } else
      break;

  }   // while ( rover )


  return ( nodes_to_nest > 1 ) ? TRUE : FALSE;
}


// msup<uID5.51.2>!msup!reqELEMENT(5.51.3)_SUPERSCRIPT_!/msup!
// _SUPERSCRIPT_reqELEMENT(5.51.5)

TCI_BOOL LaTeX2MMLTree::IsPrimedPoint( TNODE* rover ) {

  TCI_BOOL rv =  FALSE;

  TNODE* base =  FindObject( rover->parts,(U8*)"5.51.3",
									    INVALID_LIST_POS );
  if ( base && base->contents ) {
    TNODE* bc =  base->contents;
    if ( bc && !bc->next ) {

      TCI_BOOL check_super  =  FALSE;
      if ( bc->details
      &&   bc->details->is_Greek == 1 ) {
        check_super =  TRUE;
      } else {
        U16 uobj,usub,uid;
        GetUids( bc->zuID,uobj,usub,uid );
        if ( uobj==3 && usub==201 && uid==1 ) { // <mi>
          U16 zln =  bc->v_len;
          if ( zln==1 ) {
            U8 ch =  bc->var_value[0];
            if      ( ch>='a' && ch<='z' )
              check_super  =  TRUE;
            else if ( ch>='A' && ch<='Z' )
              check_super  =  TRUE;
          }
        }
      }

      if ( check_super ) {
        TNODE* super  =  FindObject( rover->parts,
									(U8*)"5.51.5",INVALID_LIST_POS );
        if ( super && super->contents ) {
          TNODE* sc =  super->contents;
          if ( sc && !sc->next ) {
            if ( sc->details )
              if ( sc->details->form == OPF_postfix )
                if ( sc->details->precedence > 60 )
                  rv  =  TRUE;
          }
        }
      }   // if ( check_super )

    }

  }     // if ( base ... )

  return rv;
}     // msup clause



// /msub<uID5.50.1>
// msub<uID5.50.2>!msub!_SUBBASE__SUBSCRIPT_!/msub!
// _SUBBASE_reqELEMENT(5.50.3)
// _SUBSCRIPT_reqELEMENT(5.50.4)

TCI_BOOL LaTeX2MMLTree::IsSubscriptedPoint( TNODE* msub ) {

  TCI_BOOL rv =  FALSE;

  TNODE* base =  FindObject( msub->parts,(U8*)"5.50.3",
									                      INVALID_LIST_POS );
  if ( base && base->contents ) {
    TNODE* bc =  base->contents;
    if ( bc && !bc->next ) {

      TCI_BOOL check_sub  =  FALSE;
      if ( bc->details
      &&   bc->details->is_Greek == 1 ) {
//      check_sub =  TRUE;
      } else {
        U16 uobj,usub,uid;
        GetUids( bc->zuID,uobj,usub,uid );
        if ( uobj==3 && usub==201 && uid==1 ) { // <mi>
          U16 zln =  bc->v_len;
          if ( zln==1 ) {
            U8 ch =  bc->var_value[0];
            if      ( ch>='a' && ch<='z' ) {
//            check_sub  =  TRUE;
            } else if ( ch>='A' && ch<='Z' )
              check_sub  =  TRUE;
          }
        }
      }

      if ( check_sub ) {
        TNODE* sub  =  FindObject( msub->parts,
									          (U8*)"5.50.4",INVALID_LIST_POS );
        if ( sub && sub->contents ) {
          TNODE* sc =  sub->contents;
          if ( sc && !sc->next ) {
            U16 uobj,usub,uid;
            GetUids( sc->zuID,uobj,usub,uid );
            if ( uobj==3 && usub==202 && uid==1 ) // <mn>
              rv  =  TRUE;
          }
        }
      }   // if ( check_sub )

    }

  }     // if ( base ... )

  return rv;
}



U16 LaTeX2MMLTree::ClassDeltaFromContext( TNODE* tex_sym_node ) {

  U16 rv  =  0;

  TNODE* nn =  tex_sym_node->next;
  TNODE* pn =  tex_sym_node->prev;

  if ( !nn ) {
    rv  =  MML_IDENTIFIER;
  } else if ( !pn ) {
    U16 uobj,usub,uid;
    GetUids( nn->zuID,uobj,usub,uid );
    if ( uobj==3 && usub==14 )
      rv  =  MML_IDENTIFIER;
  }

  return rv;
}



// Some lines in eqnarrays call for extra whitespace below.
//  \smallskip, \medskip, \bigskip, \vspace{GLUE}
// If such v-space is found in the oof list from processing
//  a MATH bucket, we return it's total value in "ex"s.

TCI_BOOL LaTeX2MMLTree::GetVSpaceFromOOFList( TNODE* TeX_oof_list,
                                    double& extra_depth ) {

  TCI_BOOL rv =  FALSE;
  extra_depth =  0.0;

  double skip_depth     =  0.0;   // smallskip, medskip, bigskip
  I16 vs_in_math_units  =  0;     // for \vspace{GLUE}

  TNODE* rover  =  TeX_oof_list;
  while ( rover ) {             // loop thru the out-of-flow list
    U16 uobjtype,usubtype,uID;
    GetUids( rover->zuID,uobjtype,usubtype,uID );
    if ( uobjtype==9 && usubtype==3 ) {   // v-space objects
      switch ( uID ) {
        case 1 :		// \smallskip<uID9.3.1> 0.25 * \baselineskip
          skip_depth +=  0.5;     // in "ex"s
        break;
        case 2 :		// \medskip<uID9.3.2>	  0.50 * \baselineskip
          skip_depth +=  1.0;
        break;
        case 3 :		// \bigskip<uID9.3.3>	  1.00 * \baselineskip
          skip_depth +=  2.0;
        break;

        case 4 :		// \strut<uID9.3.4>
        case 5 :		// \mathstrut<uID9.3.5>  // not implemented
//        strcpy( (char*)depth,"+3.5pt" );
//        strcpy( (char*)height,"+8.5pt" );
        break;

        break;
        case 6 :		// \vspace<uID9.3.6>!\vspace!REQPARAM(9.3.20,GLUE)
        case 7 : {	// \vspace*<uID9.3.7>!\vspace*!REQPARAM(9.3.21,GLUE)
          U8 depth[64];
          depth[0]  =  0;
          I16 mu_value;
          U8 bucket_zuID[32];
          UidsTozuID( 9,3,uID+14,(U8*)bucket_zuID );
          TNODE* glue_bucket =  FindObject( rover->parts,
						              (U8*)bucket_zuID,INVALID_LIST_POS );
          TCI_BOOL is_relative  =  FALSE;
          GetValueFromGlue( glue_bucket,(U8*)depth,is_relative,mu_value );
          vs_in_math_units  +=  mu_value;
        }
        break;
        default :
          TCI_ASSERT(0);
        break;
      }

    }

    rover =  rover->next;
  }

// I'm just adding all v-space together here.

  if ( skip_depth != 0.0 || vs_in_math_units ) {
    double math_units =  vs_in_math_units;
    extra_depth  =  skip_depth + ( math_units / 9.0 );
    rv  =  TRUE;
  }

  return rv;
}


// Construct embellished operators for \varlimsup, etc.

TNODE* LaTeX2MMLTree::DecorateVarLim( TNODE* mo,U16 TeX_varlim_uID ) {

  TNODE* mml_rv =  NULL;

// munder<uID5.53.2>!munder!reqELEMENT(5.53.3)reqELEMENT(5.53.4)
// mover <uID5.54.2>!mover! reqELEMENT(5.54.3)reqELEMENT(5.54.5)

  U16 usub  =  0;
  U16 decor_bucket_id;
  U16 mml_decor_uID;
  if        ( TeX_varlim_uID==13 ) {   // \varinjlim - rightarrow under lim
// &rarr;<uID3.25.10>postfix,70,,stretchy="true"
    usub  =  53;
    decor_bucket_id  =  4;
    mml_decor_uID =  10;

  } else if ( TeX_varlim_uID==14 ) {   // \varliminf - bar under lim
// &UnderBar;<uID3.25.8>postfix,70,U00332,stretchy="true"
    usub  =  53;
    decor_bucket_id  =  4;
    mml_decor_uID =  8;

  } else if ( TeX_varlim_uID==15 ) {   // \varlimsup - bar over lim
// &OverBar;<uID3.25.1>postfix,70,U000AF,stretchy="true" accent="true"
    usub  =  54;
    decor_bucket_id  =  5;
    mml_decor_uID =  1;

  } else if ( TeX_varlim_uID==16 ) {   // \varprojlim - leftarrow under lim
// &larr;<uID3.25.9>postfix,70,,stretchy="true"
    usub  =  53;
    decor_bucket_id  =  4;
    mml_decor_uID =  9;

  } else
    TCI_ASSERT(0);

  if ( usub ) {   // munder OR mover

// Change from mo to mi, and set color.

    strcpy( (char*)mo->zuID,"3.201.1" );

//  If we leave "lim" as an <mi>, set the following attr
//  SetNodeAttrib( mo,(U8*)"movablelimits",(U8*)"false" );

    if ( zMMLFunctionAttrs )
      SetMMLAttribs( mo,(U8*)zMMLFunctionAttrs );

// We return an <munder> OR an <mover>

    mml_rv  =  CreateElemWithBucketAndContents( 5,usub,2,3,mo );

// Create the decoration - a stretchy operator

    TNODE* decor  =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );

    U8 mml_op_zuID[32];
    UidsTozuID( 3,25,mml_decor_uID,(U8*)mml_op_zuID );
    U8* op_nom;
    U8* op_info;
    if ( d_mml_grammar->GetGrammarDataFromUID(mml_op_zuID,
								context_math,&op_nom,&op_info) ) {
	  U8 entity_buffer[80];
	  entity_buffer[0]  =  0;
      if ( output_entities_as_unicodes )
        if ( op_info && *op_info )
          GetUnicodeEntity( op_info,entity_buffer );

      if ( op_nom && *op_nom )
        SetChData( decor,op_nom,entity_buffer );

      OP_GRAMMAR_INFO op_record;
	  if ( op_info && *op_info ) {
        GetAttribsFromGammarInfo( op_info,op_record );
        if ( op_record.attr_list ) {
	      TCI_ASSERT( decor->attrib_list == NULL );
	      decor->attrib_list  =  op_record.attr_list;
		  op_record.attr_list =  NULL;
	    }
	  }
    } else
      TCI_ASSERT(0);

// The decoration is rendered a bit closer if it's an accent

    SetNodeAttrib( decor,(U8*)"accent",(U8*)"true" );

// Link the decoration to mml_rv

    TNODE* part2  =  CreateBucketWithContents( 5,usub,decor_bucket_id,decor );

    TNODE* part1  =  mml_rv->parts;
    part1->next   =  part2;
    part2->prev   =  part1;
  }

  return mml_rv;
}


// \correctchoice<uID5.56.1>
// \correctchoice<uID5.56.1>&emsp;&check;

TNODE* LaTeX2MMLTree::MarkingCmd2MML( TNODE* obj_node ) {

  TNODE* mml_rv =  NULL;

  U8* dest_zname;
  U8* d_template;
  if ( d_mml_grammar->GetGrammarDataFromUID(
                        obj_node->zuID,context_math,
                        &dest_zname,&d_template) ) {
    if ( d_template && *d_template )
      mml_rv  =  MakeTNode( 0L,0L,0L,(U8*)zmtext );
    SetChData( mml_rv,d_template,NULL );
  }

  return mml_rv;
}


// arrays and tabulars can contain lines like "a & b \\[5pt]"
// Here we translate the extra space that TeX adds below the line.
// The contents of one of the cells in the mml line
//  is padded below.

TNODE* LaTeX2MMLTree::EOLSpace2MML( TNODE* mml_cell_contents,
									    TNODE* TeX_EOL ) {

  TNODE* rv =  mml_cell_contents;

  if ( mml_cell_contents && TeX_EOL && TeX_EOL->parts ) {
// \\<uID9.5.8>!\\!OPTPARAM(9.5.23,DIMEN)
// \\*<uID9.5.9>!\\*!OPTPARAM(9.5.24,DIMEN)
    U16 uobjtype,usubtype,uID;
    GetUids( TeX_EOL->zuID,uobjtype,usubtype,uID );
    if ( uobjtype==9 && usubtype==5 && uID>=8 && uID<=9 ) {
	    U8 bucket_zuID[32];
      UidsTozuID( 9,5,uID+15,(U8*)bucket_zuID );
      TNODE* dimen  =  FindObject( TeX_EOL->parts,
							    (U8*)bucket_zuID,INVALID_LIST_POS );
      if ( dimen && dimen->contents ) {
        char* dimen_str =  (char*)dimen->contents->src_tok;
        double eol_depth  =  atof( dimen_str );
        if ( eol_depth > 0.00001 ) {
          U16 TeX_unit_ID   =  TeXUnitToID( dimen_str );
          U16 mml_dest_unit =  GetMMLdimenType( TeX_unit_ID );
          eol_depth  =  ConvertTeXDimen( eol_depth,TeX_unit_ID,
                                              mml_dest_unit );
	      U8 depth[64];
	      sprintf( (char*)depth,"+%f",eol_depth );
          AttachUnit( (char*)depth,mml_dest_unit );
// mpadded<uID5.602.0>!mpadded!BUCKET(5.602.2,MATH,,,/mpadded,)!/mpadded!
          mml_cell_contents =  FixImpliedMRow( mml_cell_contents );
          rv  =  CreateElemWithBucketAndContents( 5,602,0,2,
                                         mml_cell_contents );
          SetNodeAttrib( rv,(U8*)"depth",depth );
        }
      }

    } else
      TCI_ASSERT(0);
	}

  return rv;
}


// Get the extra depth below an eqnarray line ending with \\[5pt].

void LaTeX2MMLTree::GetEOLSpace( TNODE* TeX_EOL,U8* depth ) {

  depth[0]  =  0;

  U16 uobjtype,usubtype,uID;
  GetUids( TeX_EOL->zuID,uobjtype,usubtype,uID );

// \\<uID9.5.8>!\\!OPTPARAM(9.5.23,DIMEN)
// \\*<uID9.5.9>!\\*!OPTPARAM(9.5.24,DIMEN)

  if ( uobjtype==9 && usubtype==5 && uID>=8 && uID<=9 ) {
	  U8 bucket_zuID[32];
    UidsTozuID( 9,5,uID+15,(U8*)bucket_zuID );
    TNODE* dimen  =  FindObject( TeX_EOL->parts,
							              (U8*)bucket_zuID,INVALID_LIST_POS );
    if ( dimen && dimen->contents ) {
      char* dimen_str   =  (char*)dimen->contents->var_value;
      double eol_depth  =  atof( dimen_str );
      if ( eol_depth > 0.00001 ) {
        U16 TeX_unit_ID =  TeXUnitToID( dimen_str );
        U16 MML_unit_ID =  6;     // ex
        eol_depth  =  ConvertTeXDimen( eol_depth,TeX_unit_ID,MML_unit_ID );
	      sprintf( (char*)depth,"%f",eol_depth + 1.0 );
        AttachUnit( (char*)depth,MML_unit_ID );
      }
    }
  } else
    TCI_ASSERT(0);
}


// To add an equation number to a displayed equation,
//  we nest it in a <mtable>, and use <mlabeledtr>.

TNODE* LaTeX2MMLTree::AddNumberToEquation( TNODE* mml_equation ) {

  TNODE* mml_rv =  NULL;

// mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd)!/mtd!
  mml_equation  =  FixImpliedMRow( mml_equation );
  TNODE* mtd  =  CreateElemWithBucketAndContents( 5,35,14,8,mml_equation );

// Put the mtd under a cell list item node

	TNODE* list_node  =  MakeTNode( 0,0,0,(U8*)"5.35.11:0" );
	list_node->parts  =  mtd;
	mtd->sublist_owner  =  list_node;

// Form the text for our equation number - need to add ( parens ).

  U8 ztag[80];
  //sprintf( (char*)ztag,"(%d.%d)",thesection,theequation );
  FormatCurrEqnNumber( ztag );

  TNODE* tag  =  MakeTNode( 0,0,0,(U8*)"3.204.1" );
  SetChData( tag,ztag,NULL );
  if ( zMMLTextAttrs )
//  SetNodeAttrib( tag,(U8*)"mathcolor",(U8*)zMMLTextAttrs );
    SetMMLAttribs( tag,(U8*)zMMLTextAttrs );
  tag =  MMLlistToMRow( tag );

// <mlabeledtr>

  TNODE* mlabeledtr =  CreateElemWithBucketAndContents( 5,35,40,42,tag );
  TNODE* part1  =  mlabeledtr->parts;

  TNODE* part2  =  MakeTNode( 0,0,0,(U8*)"5.35.11" );
  part2->parts  =  list_node;
	list_node->sublist_owner =  part2;

	part1->next =  part2;
	part2->prev =  part1;

// Put the mlabeledtr under a node in the row list

  TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)"5.35.9:0" );
  row_list_node->parts  =  mlabeledtr;
  mlabeledtr->sublist_owner  =  row_list_node;

// mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
// _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)

  mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
  mml_rv->parts->parts  =  row_list_node;
  row_list_node->sublist_owner =  mml_rv->parts;

  SetTableLineTaggingAttrs( mml_rv );

  return mml_rv;
}

char* uIDToString(U16 usub){
  switch (usub) {
     case TENV_eqnarray  : return "eqnarray";
	   case 123            : return "align";
	   case 125            : return "alignat";
	   case 127            : return "xalignat";
	   case 131            : return "gather";
	   case 133            : return "multline";
	   case 124            : return "align*";
	   case 126            : return "alignat*";
	   case 128            : return "xalignat*";
	   case 130            : return "xxalignat*";
	   case 132            : return "gather*";
	   case 134            : return "multline*";
	   case TENV_eqnarraystar  :  return "eqnarray*";
	   case 129            : return "xxalignat";
	   case TENV_split     : return "split";
	   case TENV_gathered  : return "gathered";
     case TENV_aligned   : return "alinged";
     case TENV_alignedat : return "alignedat";
   }
   return "unknown";
}
// Does an environment for markup of a multiline MATH display
//  automatically call for numbering of its lines using "theequation"?
// Generally, lines in first level, un-starred eqnarrays are
//  enumerated.  Constructs that are starred or intended to be nested
//  are NOT enumerated.

U16 LaTeX2MMLTree::EqnHasNumberedLines( U16 usub ) {

  U16 rv =  0;

  switch ( usub ) {

//TEXT
	case TENV_eqnarray  :
	case 123  :		// TENV_align          123
	case 125  :		// TENV_alignat        125
	case 127  :		// TENV_xalignat       127
	case 131  :		// TENV_gather         131
      rv =  1;      // all lines are numbered
    break;
	case 133  :		// TENV_multline       133
      rv =  2;      // the first line is numbered
    break;

	case 124  :		// \begin{align*}<uID5.124.0>!
	case 126  :		// TENV_alignatstar    126
	case 128  :		// TENV_xalignatstar   128
	case 130  :		// TENV_xxalignatstar  130
	case 132  :		// TENV_gatherstar     132
	case 134  : 	// TENV_multlinestar   134

	case TENV_eqnarraystar  :
	case 129  :		// TENV_xxalignat      129
      rv =  0;      // no lines are numbered
    break;

//MATH
//  case 124            :
	case TENV_split     :
	case TENV_gathered  :
    case TENV_aligned   :
    case TENV_alignedat :
      rv =  0;
    break;

    default :
      TCI_ASSERT(0);
    break;
  }

  return rv;
}


// Add attribs to a table with labelled rows.
// Side and min offset distance are specified for labels.

void LaTeX2MMLTree::SetTableLineTaggingAttrs( TNODE* mml_rv ) {

// side<uID5.701.4>right
// minlabelspacing<uID5.701.5>1.0in
  U8* dest_zname;
  U8* d_template;
  if ( d_mml_grammar->GetGrammarDataFromUID(
    (U8*)"5.701.4",context_math,&dest_zname,&d_template) ) {
    if ( dest_zname && dest_zname[0]
    &&   d_template && d_template[0] )
      SetNodeAttrib( mml_rv,dest_zname,d_template );
  }
  if ( d_mml_grammar->GetGrammarDataFromUID(
    (U8*)"5.701.5",context_math,&dest_zname,&d_template) ) {
    if ( dest_zname && dest_zname[0]
    &&   d_template && d_template[0] )
      SetNodeAttrib( mml_rv,dest_zname,d_template );
  }
}


// Given a LaTeX symbol's zuID, generate chdata for MML output.

void LaTeX2MMLTree::AppendEntityToBuffer( U8* zuID,U8* entity_buffer,
                                              U8* unicode_buffer ) {

  U16 zln   =  strlen( (char*)entity_buffer );
  U16 zln2  =  strlen( (char*)unicode_buffer );
  U8* dest_zname;
  U8* d_template;
  if ( d_mml_grammar->GetGrammarDataFromUID(zuID,context_math,
                                        &dest_zname,&d_template) ) {
    if ( dest_zname && *dest_zname )
      strcpy( (char*)entity_buffer+zln,(char*)dest_zname );

    if ( unicode_buffer )
      if ( output_entities_as_unicodes )
        if ( d_template && *d_template )
          GetUnicodeEntity( d_template,unicode_buffer+zln2 );
  } else {
    TCI_ASSERT(0);
    strcpy( (char*)entity_buffer+zln,"?" );
  }
}


// Function to generate a unicode, &#dddd;, from the data given
//  in MathML.gmr on the right side of a line that describes a symbol.
// &DD;<uID3.2.4>prefix,55,U02145
// &AElig;<uID3.16.1>,U000C6


/*  TeX's stretchy delimiters - "\big("
\big<uID5.71.1>!\big!VAR(5.71.2,FENCE,a,,)
                , \Big<uID5.72.1>,  \bigg<uID5.73.1>,  \Bigg<uID5.74.1>

\bigl<uID5.71.3>, \Bigl<uID5.72.3>, \biggl<uID5.73.3>, \Biggl<uID5.74.3>
\bigm<uID5.71.5>, \Bigm<uID5.72.5>, \biggm<uID5.73.5>, \Biggm<uID5.74.5>
\bigr<uID5.71.7>, \Bigr<uID5.72.7>, \biggr<uID5.73.7>, \Biggr<uID5.74.7>
*/

TNODE* LaTeX2MMLTree::BiglmrToMML( TNODE* tex_bigl_node,
                                    U16 subclass,U16 id ) {

// This LaTeX construct is translated to an <mo>.
  TNODE* mml_rv  =  MakeTNode( 0L,0L,tex_bigl_node->src_linenum,
                                (U8*)"3.203.1" );

  U16 left_mid_right;       // some kind of fence
  if      ( id==1 )             // ??
    left_mid_right =  2;
  else if ( id==3 )             // a left side delimiter
    left_mid_right =  1;
  else if ( id==5 )             // mid
    left_mid_right =  2;
  else if ( id==7 )             // right side
    left_mid_right =  3;
  else
    TCI_ASSERT(0);

  TNODE* delim_VAR  =  FindObject( tex_bigl_node->parts,
                            (U8*)"5.71.2",INVALID_LIST_POS );
  U8 mml_delim_entity[64];
  U8 unicode_entity[64];
  U8 dummy[128];
  TeXDelimToMMLOp( delim_VAR->var_value,left_mid_right,
                            mml_delim_entity,unicode_entity,dummy );
  SetChData( mml_rv,mml_delim_entity,unicode_entity );

  U16 big_size  =  subclass-70;
  SetBiglSizingAttrs( mml_rv,big_size );

// If the LaTeX markup contains correctly used l-m-r variants,
//  we can set form and precedence here in initial translation.

  I16 form_ID     =  0;
  I16 precedence  =  1;
  I16 TeX_atom_ilk  =  0;
  if        ( id==3 ) {     // a left-side delimiter
    form_ID =  OPF_prefix;
    precedence  =  1;
    TeX_atom_ilk  =  TeX_ATOM_OPEN;
  } else if ( id==5 ) {     // a mid-type delimiter
    form_ID =  OPF_infix;
    precedence  =  2;
    TeX_atom_ilk  =  TeX_ATOM_BIN;
  } else if ( id==7 ) {     // a right-side delimiter
    form_ID =  OPF_postfix;
    precedence  =  1;
    TeX_atom_ilk  =  TeX_ATOM_CLOSE;
  }

  if ( form_ID ) {
    SetDetailNum( mml_rv,DETAILS_form,form_ID );
    SetDetailNum( mml_rv,DETAILS_precedence,precedence );
  }

  if ( TeX_atom_ilk )
    SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_atom_ilk );

// We mark this node for consideration when implicit fences are done.
// big - 1, Big - 2, bigg - 3, Bigg - 4

  SetDetailNum( mml_rv,DETAILS_bigl_size,big_size );

// Note that the translation is not completed here in most cases.
// The LaTeX node is probably part of a matched pair used as a fence.
// More attributes will be added when the fence is located.

  return  mml_rv;
}


// The attributes used to script the size of a TeX's \bigl(, \Bigm|,
//  \biggr], etc. are taken from the "BIGLMR" section of MathML.gmr

void LaTeX2MMLTree::SetBiglSizingAttrs( TNODE* mml_delim_op,U16 biglmr_size ) {

	U8 zuID[16];
  UidsTozuID( 15,biglmr_size,0,(U8*)zuID );
  U8* dest_zname;
  U8* d_template;
  if ( d_mml_grammar->GetGrammarDataFromUID(
					            zuID,(U8*)"BIGLMR",
   							    &dest_zname,&d_template) ) {
// \big<uID15.1.0>  stretchy="true" minsize="1.2" maxsize="1.2"
// \Big<uID15.2.0>  stretchy="true" minsize="1.8" maxsize="1.8"
    if ( d_template && *d_template )
      SetMMLAttribs( mml_delim_op,d_template );

  } else    // lookup in MathML.gmr failed
    TCI_ASSERT(0);
}



// \xleftarrow<uID5.16.1>!\xleftarrow!OPTPARAM(5.17.1,MATH)REQPARAM(5.17.2,MATH)
// \xrightarrow<uID5.16.2>!\xrightarrow!OPTPARAM(5.17.1,MATH)REQPARAM(5.17.2,MATH)

TNODE* LaTeX2MMLTree::XArrow2MML( TNODE* obj_node,
                                    TNODE** out_of_flow_list,
                                    U16 tex_uID ) {

  TNODE* mml_rv =  NULL;

// Translate the xarrow to MathML - a stretchy operator
//  that we will embed in an <munderover> - embellished operator.

  TNODE* mo =  MakeTNode( 0L,0L,0L,(U8*)"3.203.1" );

// &larr;<uID3.24.1>infix,25,U02190,stretchy="true"
// &rarr;<uID3.24.2>infix,25,U02192,stretchy="true"

  OP_GRAMMAR_INFO op_record;
  op_record.form        =  OPF_infix;
  op_record.precedence  =  25;

  U8 mml_op_zuID[32];
  UidsTozuID( 3,24,tex_uID,(U8*)mml_op_zuID );
  U8* op_nom;
  U8* op_info;
  if ( d_mml_grammar->GetGrammarDataFromUID(mml_op_zuID,
                                context_math,&op_nom,&op_info) ) {

    U8 entity_buffer[128];
    entity_buffer[0]  =  0;
    if ( output_entities_as_unicodes )
      GetUnicodeEntity( op_info,entity_buffer );
    SetChData( mo,op_nom,entity_buffer );

	if ( op_info && *op_info ) {
      GetAttribsFromGammarInfo( op_info,op_record );
      if ( op_record.attr_list ) {
        mo->attrib_list  =  op_record.attr_list;
        op_record.attr_list =  NULL;
      }
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  SetNodeAttrib( mo,(U8*)"form",(U8*)zop_forms[op_record.form] );

// Translate the mbucket under the xarrow.

  TNODE* under_contents  =  NULL;

  TNODE* under_bucket =  FindObject( obj_node->parts,
                                (U8*)"5.17.1",INVALID_LIST_POS );
  if ( under_bucket ) {
    TCI_BOOL do_bindings  =  TRUE;
    U16 tex_nodes_done,error_code;
    TNODE* local_oof_list =  NULL;
    script_level++;
    under_contents =  TranslateMathList( under_bucket->contents,
                                do_bindings,NULL,tex_nodes_done,
                                error_code,&local_oof_list );
    script_level--;
    under_contents =  HandleOutOfFlowObjects( under_contents,
    								        &local_oof_list,out_of_flow_list,2 );
  } else
    under_contents =  MakeSmallmspace();

// Translate the mbucket over the xarrow.

  TNODE* over_contents  =  NULL;

  TNODE* over_bucket  =  FindObject( obj_node->parts,
                                (U8*)"5.17.2",INVALID_LIST_POS );
  if ( over_bucket ) {
    TCI_BOOL do_bindings  =  TRUE;
    U16 tex_nodes_done,error_code;
    TNODE* local_oof_list =  NULL;
    script_level++;
    over_contents  =  TranslateMathList( over_bucket->contents,
                                do_bindings,NULL,tex_nodes_done,
                                error_code,&local_oof_list );
    script_level--;
    over_contents  =  HandleOutOfFlowObjects( over_contents,
    								        &local_oof_list,out_of_flow_list,2 );
  } else
    over_contents  =  MakeSmallmspace();

// Construct mml_rv - an <munderover>

// munderover<uID5.55.2>!munderover!reqELEMENT(5.55.3)_UOSUB__UOSUPER_!/munderover!
// _UOSUB_reqELEMENT(5.55.4)
// _UOSUPER_reqELEMENT(5.55.5)

  mml_rv  =  CreateElemWithBucketAndContents( 5,55,2,3,mo );

  TNODE* part1  =  mml_rv->parts;
  if ( under_contents->next )
    under_contents  =  MMLlistToMRow( under_contents );
  TNODE* part2  =  CreateBucketWithContents( 5,55,4,under_contents );
  part1->next   =  part2;
  part2->prev   =  part1;

  if ( over_contents->next )
    over_contents   =  MMLlistToMRow( over_contents );
  TNODE* part3  =  CreateBucketWithContents( 5,55,5,over_contents );
  part2->next   =  part3;
  part3->prev   =  part2;

// mml_rv is an embellished operator - set operator Details

  SetDetailNum( mml_rv,DETAILS_form,op_record.form );
  SetDetailNum( mml_rv,DETAILS_precedence,op_record.precedence );
  SetDetailNum( mml_rv,DETAILS_TeX_atom_ilk,TeX_ATOM_REL );

  return mml_rv;
}



TCI_BOOL LaTeX2MMLTree::TeXListHasIntegral( TNODE* LaTeX_list,
                                            TNODE* tex_d_node ) {

  TCI_BOOL rv =  FALSE;

  TNODE* rover  =  LaTeX_list;
  while ( rover && (rover != tex_d_node) ) {
    U16 uobj,usub,id;
    GetUids( rover->zuID,uobj,usub,id );
	  if ( uobj==7 && usub==1 && id<=6 ) {
      rv =  TRUE;
      break;
	  }
    rover =  rover->next;
  }

  return rv;
}


// attr_str is the right side (template) from a MathML.gmr line.
// attr_str -> mathcolor="red" mathbackground="transparent"

void LaTeX2MMLTree::SetMMLAttribs( TNODE* mml_node,U8* attr_str ) {

  if ( attr_str && *attr_str ) {
    ATTRIB_REC* ar  =  ExtractAttrs( (char*)attr_str );
    if ( ar ) {
      ATTRIB_REC* arover  =  ar;
      while ( arover ) {
        if ( arover->attr_nom && arover->z_val )
          SetNodeAttrib( mml_node,arover->attr_nom,arover->z_val );
        arover  =  arover->next;
      }
      DisposeAttribs( ar );
    }
  }
}


/*
\begin{split}<uID5.141.0>!\begin{split}!_SPLITLINES_!\end{split}!
_SPLITLINES_LIST(5.141.9,_SPLITLINE_,5.141.10,,\end{split},\\|\\*)
_SPLITLINE_IF(TEXT,?\intertext?,5.330.1)ifEND_SPLITCELLS_
_SPLITCELLS_BUCKET(5.141.4,MATH,,,\\|\\*|\end{split},)

\begin{gathered}<uID5.142.0>!\begin{gathered}!OPTPARAM(5.121.1,MEXTALIGN)_GATHEREDLINES_!\end{gathered}!
_GATHEREDLINES_LIST(5.142.9,_GATHEREDLINE_,5.142.10,,\end{gathered},\\|\\*)
_GATHEREDLINE_BUCKET(5.142.4,MATH,,,\\|\\*|\end{gathered},)

\begin{aligned}<uID5.143.0>!\begin{aligned}!OPTPARAM(5.121.1,MEXTALIGN)_ALIGNEDLINES_!\end{aligned}!
_ALIGNEDLINES_LIST(5.143.9,_ALIGNEDLINE_,5.143.10,,\end{aligned},\\|\\*)
_ALIGNEDLINE_IF(TEXT,?\intertext?,5.330.1)ifEND_ALIGNEDCELLS_
_ALIGNEDCELLS_BUCKET(5.143.4,MATH,,,\\|\\*|\end{aligned},)
*/


TNODE* LaTeX2MMLTree::NestedTeXEqnArray2MML( TNODE* src_eqn,
									        TNODE** out_of_flow_list,
											U16 usubtype ) {

  TNODE* mml_rv =  NULL;

// Given usubtype, get the alignment pattern.
//  ie. for eqnarray, "right center left"

  U8 column_align_vals[64];
  GetEQNAlignVals( usubtype,(U8*)column_align_vals,64 );

// This seems to be the only way to handle extra space between lines

  U8 row_spacing_vals[128];
  row_spacing_vals[0] =  0;

  TCI_BOOL found_spacer =  FALSE;


  TNODE* mtr_head =  NULL;		  // we build a list of <mtr>'s
  TNODE* mtr_tail;

  U8 lbucket_uID[16];
  UidsTozuID( 5,121,4,lbucket_uID );

// Locate the list of lines in the LaTeX source object

  U8 line_list_zuID[16];	      // 
  UidsTozuID( 5,121,9,line_list_zuID );
  TNODE* list_of_lines  =  FindObject( src_eqn->parts,
	  							line_list_zuID,INVALID_LIST_POS );

  U16 src_line_counter  =  0;
  U16 dest_line_counter =  0;
  while ( TRUE ) {     	// loop down thru lines in LaTeX environment

    TNODE* mtd_cont_head  =  NULL;	// we build a list of <mtd>'s

// Find the current line in the list of lines
    TNODE* curr_line  =  FindObject( list_of_lines->parts,
        						line_list_zuID,src_line_counter );
    if ( !curr_line ) break;	// normal exit - no more lines


    TNODE* TeX_EOL  =  FindObject( curr_line->parts,(U8*)"9.5.8",
                                        INVALID_LIST_POS );
    if ( !TeX_EOL )
      TeX_EOL =  FindObject( curr_line->parts,(U8*)"9.5.9",
                                        INVALID_LIST_POS );

// Process the current line

// \intertext<uID5.330.1>!\intertext!REQPARAM(5.330.2,TEXT)
    TNODE* intertext  =  FindObject( curr_line->parts,
       							(U8*)"5.330.1",INVALID_LIST_POS );

// Put the current intertext under a node in the lines list

    if ( intertext
    &&   intertext->parts
    &&   intertext->parts->contents ) {
      TNODE* local_oof_list =  NULL;
      TNODE* mtext  =  TextInMath2MML( intertext->parts->contents,
   								&local_oof_list,FALSE,FALSE );
      if ( !mtext )
        mtext  =  MakeSmallmspace();

      U8 v_space[80];
      strcpy( (char*)v_space,"1.0ex " );

	  if ( local_oof_list ) {
        double extra_depth;
        if ( GetVSpaceFromOOFList(local_oof_list,extra_depth) ) {
          if ( extra_depth != 0.0 ) {
    // the default row spacing is 1.0ex
            sprintf( (char*)v_space,"%fex ",extra_depth + 1.0 );
            found_spacer =  TRUE;
          }
        }

        mtext  =  Struts2MML( mtext,local_oof_list );
        local_oof_list  =  DisposeOutOfFlowList( local_oof_list,5,800 );
        mtext  =  Labels2MML( mtext,local_oof_list );
	// We may want to pass this back to the caller in the future.
        DisposeTList( local_oof_list );
	  }

      mtext =  FixImpliedMRow( mtext );
      TNODE* mtd  =  CreateElemWithBucketAndContents( 5,35,14,8,mtext );
      SetNodeAttrib( mtd,(U8*)"groupalign",(U8*)"center" );

      TNODE* mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );

      if ( strlen((char*)row_spacing_vals) < 120 )
        strcat( (char*)row_spacing_vals,(char*)v_space );

	  U8 zlistID[32];
      UidsTozuID( 5,35,11,(U8*)zlistID );
	  U16 zln =  strlen( (char*)zlistID );
	  zlistID[zln]  =  ':';
	  // JCS non-standard: itoa( 0,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1,"%d", 0);

	  TNODE* col_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );

	  col_list_node->parts  =  mtd;
	  mtd->sublist_owner  =  col_list_node;

      mtr->parts->parts =  col_list_node;
	  col_list_node->sublist_owner  =  mtr->parts;

// Put the intertext under a node in the lines list

      UidsTozuID( 5,35,9,(U8*)zlistID );
	  zln =  strlen( (char*)zlistID );
	  zlistID[zln]  =  ':';
	  // JCS non-standard: itoa( dest_line_counter,(char*)zlistID+zln+1,10 );
          sprintf((char*)zlistID+zln+1, "%d", dest_line_counter);
	  TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );

	  row_list_node->parts  =  mtr;
	  mtr->sublist_owner  =  row_list_node;

// Append

      if ( !mtr_head )
        mtr_head  =  row_list_node;
      else {
        mtr_tail->next =  row_list_node;
	    row_list_node->prev =  mtr_tail;
	  }
      mtr_tail  =  row_list_node;

      dest_line_counter++;
	}

// end intertext

    TNODE* tag  =  NULL;
    TCI_BOOL add_eqn_number =  FALSE;

    U8 r_space[80];
    strcpy( (char*)r_space,"1.0ex " );

    TNODE* line_bucket  =  FindObject( curr_line->parts,
   							    lbucket_uID,INVALID_LIST_POS );
    if ( line_bucket ) {

// Look for \TCItag, etc.

      if ( line_bucket->contents ) {
        tag   =  FindObject( line_bucket->contents,
                                (U8*)"5.701.0",INVALID_LIST_POS );
        if ( !tag )
          tag =  FindObject( line_bucket->contents,(U8*)"5.702.0",
                                        INVALID_LIST_POS );
      }


// translate the line to MML

  	  TCI_BOOL do_bindings  =  TRUE;
      U16 tex_nodes_done,error_code;
      TNODE* local_oof_list =  NULL;
      TNODE* contents =  TranslateMathList( line_bucket->contents,
								              do_bindings,NULL,tex_nodes_done,
								              error_code,&local_oof_list );

// WARNING: At present, TeX Explorer ignores <maligngroup> if it is nested
//  in an <mpadded>, so we can't pad lines in an eqnarray
//    U16 vspace_context  =  2;   // mpadded

      double extra_depth;
      if ( GetVSpaceFromOOFList(local_oof_list,extra_depth) ) {
        if ( extra_depth != 0.0 ) {
    // the default row spacing is 1.0ex
          sprintf( (char*)r_space,"%fex ",extra_depth + 1.0 );
          found_spacer =  TRUE;
        }
      }

      U16 vspace_context  =  3;   // ignore, for now
      contents  =  HandleOutOfFlowObjects( contents,&local_oof_list,
   						                  out_of_flow_list,vspace_context );

      if ( TeX_EOL ) {
        U8 depth[64];
        GetEOLSpace( TeX_EOL,depth );
        if ( depth[0] ) {
          strcpy( (char*)r_space,(char*)depth );
          strcat( (char*)r_space," " );
          found_spacer =  TRUE;
        }
        TeX_EOL =  NULL;
      }

  // Add mml "contents" to the list we're building
      if ( contents )
        mtd_cont_head   =  contents;
	}


// mtd<uID5.35.14>!mtd!BUCKET(5.35.8,MATH,,,/mtd,)!/mtd!
// maligngroup<uID9.20.0>

	TNODE* alignmark  =  MakeTNode( 0,0,0,(U8*)"9.20.0" );
// mark node as pure whitespace
    SetDetailNum( alignmark,DETAILS_space_width,0 );
    alignmark->next =  mtd_cont_head;
    if ( mtd_cont_head )
      mtd_cont_head->prev =  alignmark;


    TNODE* mml_cell =  CreateElemWithBucketAndContents( 5,35,14,
                                                    8,alignmark );
    if ( column_align_vals[0] ) {
      if ( ( usubtype == 133 || usubtype == 134 ) // multline
      &&   dest_line_counter == 0 ) {
        SetNodeAttrib( mml_cell,(U8*)"groupalign",(U8*)"right" );
      } else
        SetNodeAttrib( mml_cell,(U8*)"groupalign",column_align_vals );
    }


// Put the current mml_cell under a cell_list_item node

	U8 zlistID[32];
    UidsTozuID( 5,35,11,(U8*)zlistID );
	U16 zln =  strlen( (char*)zlistID );
	zlistID[zln]  =  ':';
	// JCS non-standard: itoa( 0,(char*)zlistID+zln+1,10 );
        sprintf((char*)zlistID+zln+1,"%d", 0);

	TNODE* list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	list_node->parts  =  mml_cell;
	mml_cell->sublist_owner =  list_node;

// Add current one cell line to the list of lines we're building

/*
/mtr<uID5.35.16>
/mtr<uID5.35.16>
mtr<uID5.35.13>!mtr!_LISTOFCELLS_!/mtr!
_LISTOFCELLS_LIST(5.35.11,_MTCELL_,5.35.12,,/mtr|/mtable,)
_MTCELL_IF(MATH,?mtd?,5.35.14)ifEND

;MathML 2.0
/mlabeledtr<uID5.35.41>
mlabeledtr<uID5.35.40>!mlabeledtr!_EQNNUMBER__LISTOFCELLS_!/mlabeledtr!
_EQNNUMBER_reqELEMENT(5.35.42)
;_LABELEDLIST_LIST(5.35.11,_TDATA_,5.35.12,,/mtr|/mtable,)
;_TDATA_IF(MATH,?mtd?,5.35.14)ifEND
*/

    TNODE* mtr;
    if        ( tag && renderer_implements_mlabeledtr ) {

    } else if ( add_eqn_number && do_equation_numbers ) {

    } else {
      mtr  =  CreateElemWithBucketAndContents( 5,35,13,11,NULL );
	    mtr->parts->parts =  list_node;
	    list_node->sublist_owner =  mtr->parts;
    }


    if ( strlen((char*)row_spacing_vals) < 120 )
      strcat( (char*)row_spacing_vals,(char*)r_space );

// Put the current line under a node in the lines list

    UidsTozuID( 5,35,9,(U8*)zlistID );
	zln =  strlen( (char*)zlistID );
	zlistID[zln]  =  ':';
	// JCS non-standard: itoa( dest_line_counter,(char*)zlistID+zln+1,10 );
        sprintf((char*)zlistID+zln+1, "%d", dest_line_counter);
	TNODE* row_list_node  =  MakeTNode( 0,0,0,(U8*)zlistID );
	row_list_node->parts  =  mtr;
	mtr->sublist_owner  =  row_list_node;

// Append

    if ( !mtr_head )
      mtr_head  =  row_list_node;
    else {
      mtr_tail->next =  row_list_node;
	  row_list_node->prev =  mtr_tail;
	}
    mtr_tail  =  row_list_node;

    src_line_counter++;
    dest_line_counter++;

  }		// loop down thru lines in LaTeX eqnarray


// mtable<uID5.35.0>!mtable!_LISTOFROWS_!/mtable!
// _LISTOFROWS_LIST(5.35.9,_MTROW_,5.35.10,,/mtable,)

  mml_rv  =  CreateElemWithBucketAndContents( 5,35,0,9,NULL );
  if ( found_spacer )
    SetNodeAttrib( mml_rv,(U8*)"rowspacing",row_spacing_vals );

  TNODE* TeX_ext_align  =  FindObject( src_eqn->parts,(U8*)"5.121.1",
                                        INVALID_LIST_POS );
  if ( TeX_ext_align && TeX_ext_align->contents ) {
// align  (top | bottom | center | baseline | axis) [ rownumber ]   axis
    U16 uobj,usub,uID;
    GetUids( TeX_ext_align->contents->zuID,uobj,usub,uID );
    if ( uobj==11 && usub==4 ) {
      if      ( uID== 1 )
        SetNodeAttrib( mml_rv,(U8*)"align",(U8*)"top" );
      else if ( uID== 3 )
        SetNodeAttrib( mml_rv,(U8*)"align",(U8*)"bottom" );
    } else
      TCI_ASSERT(0);
  }


  mml_rv->parts->parts  =  mtr_head;
  mtr_head->sublist_owner =  mml_rv->parts;

  return mml_rv;
}


// If the operator is embellished,
//  MML_rover is the node containing op_node.
// Otherwise, both args are the same - the op_node.

void LaTeX2MMLTree::SetTeXOperatorSpacing( TNODE* MML_rover,
										                        TNODE* op_node,
										                        TCI_BOOL in_script ) {

  U8* opname  =  op_node->var_value;
  U16 nln =  strlen( (char*)opname );

  if ( !strncmp((char*)opname,"TeX",3) )
    return;

  I16 op_ilk  =  0;
  if ( MML_rover->details
  &&   MML_rover->details->TeX_atom_ilk!=UNDEFINED_DETAIL )
    op_ilk  =  MML_rover->details->TeX_atom_ilk;
  else if ( op_node->details
  &&   op_node->details->TeX_atom_ilk!=UNDEFINED_DETAIL ) {
    op_ilk  =  op_node->details->TeX_atom_ilk;
    TCI_ASSERT(0);
  }

  I16 op_form =  OPF_infix;
  if ( op_node->details
  &&   op_node->details->form!=UNDEFINED_DETAIL )
    op_form =  op_node->details->form;

  if ( op_ilk ) {

    U8 zattr_val[80];

  // Script lspace

    if ( op_form == OPF_postfix ) {

      SetMOSpacing( op_node,TRUE,(U8*)"0em" );

    } else {

      I16 left_ilk  =  0;

      TNODE* next_left  =  MML_rover->prev;
      while ( next_left && MTextIsWhiteSpace(next_left) )
        next_left  =  next_left->prev;

      if ( next_left ) {
        if ( next_left->details
        &&   next_left->details->TeX_atom_ilk!=UNDEFINED_DETAIL )
          left_ilk  =  next_left->details->TeX_atom_ilk;
        else {
          TCI_ASSERT(0);
          left_ilk  =  TeX_ATOM_ORD;
        }
        if ( left_ilk ) {
          I16 TeX_space_left  =  GetTeXSpacing( left_ilk,op_ilk );
          SetMOSpacingFromID( op_node,TeX_space_left,TRUE,in_script );
        }
      }
    }

  // Script rspace

    if ( op_form == OPF_prefix ) {

      SetMOSpacing( op_node,FALSE,(U8*)"0em" );

    } else {

      I16 right_ilk =  0;

      TNODE* next_right =  MML_rover->next;
      while ( next_right && MTextIsWhiteSpace(next_right) )
        next_right  =  next_right->next;

      if ( next_right ) {
        if ( next_right->details
        &&   next_right->details->TeX_atom_ilk!=UNDEFINED_DETAIL )
          right_ilk =  next_right->details->TeX_atom_ilk;
        else {
          TCI_ASSERT(0);
          right_ilk =  TeX_ATOM_ORD;
        }
        if ( right_ilk ) {
          I16 TeX_space_right =  GetTeXSpacing( op_ilk,right_ilk );
          SetMOSpacingFromID( op_node,TeX_space_right,FALSE,in_script );
        }
      }
    }

  } else
    TCI_ASSERT(0);

}


// The following data comes from "The TeXBook", page 170.
// A negative value means that the space is NOT typeset in scripts.
// 99 means undefined.

I16 LaTeX2MMLTree::GetTeXSpacing( U16 left_ilk,U16 right_ilk ) {

  I16 rv  =  0;

  switch ( left_ilk ) {
    case TeX_ATOM_ORD   :
      switch ( right_ilk ) {
        case TeX_ATOM_ORD   :   rv  =  0;   break;
        case TeX_ATOM_OP    :   rv  =  1;   break;
        case TeX_ATOM_BIN   :   rv  = -2;   break;
        case TeX_ATOM_REL   :   rv  = -3;   break;
        case TeX_ATOM_OPEN  :   rv  =  0;   break;
        case TeX_ATOM_CLOSE :   rv  =  0;   break;
        case TeX_ATOM_PUNCT :   rv  =  0;   break;
        case TeX_ATOM_INNER :   rv  = -1;   break;
        default :          TCI_ASSERT(0);   break;
      }
      break;
    case TeX_ATOM_OP    :
      switch ( right_ilk ) {
        case TeX_ATOM_ORD   :   rv  =  1;   break;
        case TeX_ATOM_OP    :   rv  =  1;   break;
        case TeX_ATOM_BIN   :   rv  = 99;   break;
        case TeX_ATOM_REL   :   rv  = -3;   break;
        case TeX_ATOM_OPEN  :   rv  =  0;   break;
        case TeX_ATOM_CLOSE :   rv  =  0;   break;
        case TeX_ATOM_PUNCT :   rv  =  0;   break;
        case TeX_ATOM_INNER :   rv  = -1;   break;
        default :          TCI_ASSERT(0);   break;
      }
      break;
    case TeX_ATOM_BIN   :
      switch ( right_ilk ) {
        case TeX_ATOM_ORD   :   rv  = -2;   break;
        case TeX_ATOM_OP    :   rv  = -2;   break;
        case TeX_ATOM_BIN   :   rv  = 99;   break;
        case TeX_ATOM_REL   :   rv  = 99;   break;
        case TeX_ATOM_OPEN  :   rv  = -2;   break;
        case TeX_ATOM_CLOSE :   rv  = 99;   break;
        case TeX_ATOM_PUNCT :   rv  = 99;   break;
        case TeX_ATOM_INNER :   rv  = -2;   break;
        default :          TCI_ASSERT(0);   break;
      }
      break;
    case TeX_ATOM_REL   :
      switch ( right_ilk ) {
        case TeX_ATOM_ORD   :   rv  = -3;   break;
        case TeX_ATOM_OP    :   rv  = -3;   break;
        case TeX_ATOM_BIN   :   rv  = 99;   break;
        case TeX_ATOM_REL   :   rv  =  0;   break;
        case TeX_ATOM_OPEN  :   rv  = -3;   break;
        case TeX_ATOM_CLOSE :   rv  =  0;   break;
        case TeX_ATOM_PUNCT :   rv  =  0;   break;
        case TeX_ATOM_INNER :   rv  = -3;   break;
        default :          TCI_ASSERT(0);   break;
      }
      break;
    case TeX_ATOM_OPEN  :
      switch ( right_ilk ) {
        case TeX_ATOM_ORD   :   rv  =  0;   break;
        case TeX_ATOM_OP    :   rv  =  0;   break;
        case TeX_ATOM_BIN   :   rv  = 99;   break;
        case TeX_ATOM_REL   :   rv  =  0;   break;
        case TeX_ATOM_OPEN  :   rv  =  0;   break;
        case TeX_ATOM_CLOSE :   rv  =  0;   break;
        case TeX_ATOM_PUNCT :   rv  =  0;   break;
        case TeX_ATOM_INNER :   rv  =  0;   break;
        default :          TCI_ASSERT(0);   break;
      }
      break;
    case TeX_ATOM_CLOSE :
      switch ( right_ilk ) {
        case TeX_ATOM_ORD   :   rv  =  0;   break;
        case TeX_ATOM_OP    :   rv  =  1;   break;
        case TeX_ATOM_BIN   :   rv  = -2;   break;
        case TeX_ATOM_REL   :   rv  = -3;   break;
        case TeX_ATOM_OPEN  :   rv  =  0;   break;
        case TeX_ATOM_CLOSE :   rv  =  0;   break;
        case TeX_ATOM_PUNCT :   rv  =  0;   break;
        case TeX_ATOM_INNER :   rv  = -1;   break;
        default :          TCI_ASSERT(0);   break;
      }
      break;
    case TeX_ATOM_PUNCT :
      switch ( right_ilk ) {
        case TeX_ATOM_ORD   :   rv  = -1;   break;
        case TeX_ATOM_OP    :   rv  = -1;   break;
        case TeX_ATOM_BIN   :   rv  = 99;   break;
        case TeX_ATOM_REL   :   rv  = -1;   break;
        case TeX_ATOM_OPEN  :   rv  = -1;   break;
        case TeX_ATOM_CLOSE :   rv  = -1;   break;
        case TeX_ATOM_PUNCT :   rv  = -1;   break;
        case TeX_ATOM_INNER :   rv  = -1;   break;
        default :          TCI_ASSERT(0);   break;
      }
      break;
    case TeX_ATOM_INNER :
      switch ( right_ilk ) {
        case TeX_ATOM_ORD   :   rv  = -1;   break;
        case TeX_ATOM_OP    :   rv  =  1;   break;
        case TeX_ATOM_BIN   :   rv  = -2;   break;
        case TeX_ATOM_REL   :   rv  = -3;   break;
        case TeX_ATOM_OPEN  :   rv  = -1;   break;
        case TeX_ATOM_CLOSE :   rv  =  0;   break;
        case TeX_ATOM_PUNCT :   rv  = -1;   break;
        case TeX_ATOM_INNER :   rv  = -1;   break;
        default :          TCI_ASSERT(0);   break;
      }
      break;
    default :
      TCI_ASSERT(0);
    break;
  }

  return rv;
}



void LaTeX2MMLTree::SetMOSpacing( TNODE* mo,TCI_BOOL is_left,U8* zval ) {

  if ( mo ) {
    TCI_BOOL in_script  =  GetScriptStatus();

    TCI_BOOL script_nothing =  FALSE;
    TCI_BOOL script_zval    =  FALSE;
    TCI_BOOL script_zero    =  FALSE;
    if ( in_script ) {
      if      ( spacing_mode_in_scripts == 1 )  // 1 - don't script
        script_nothing =  TRUE;
      else if ( spacing_mode_in_scripts == 2 )  // 2 - get from MathML.gmr
        script_zval    =  TRUE;
      else if ( spacing_mode_in_scripts == 3 )  // 3 - Emulate TeX
        script_zval    =  TRUE;
      else if ( spacing_mode_in_scripts == 4 )  // 4 - script 0
        script_zero    =  TRUE;
      else
        TCI_ASSERT(0);
    } else {
      if      ( mo_spacing_mode == 1 )  // 1 - don't script,
        script_nothing =  TRUE;
      else if ( mo_spacing_mode == 2 )  // 2 - get from MathML.gmr
        script_zval    =  TRUE;
      else if ( mo_spacing_mode == 3 )  // 3 - Emulate TeX
        script_zval    =  TRUE;
      else
        TCI_ASSERT(0);
    }

    TCI_BOOL done =  FALSE;

    U8* op_name =  mo->var_value;
    if ( op_name ) {
      U16 nln   =  strlen( (char*)op_name );
      U8* op_zuID;
      U8* op_zinfo;
      if ( s_mml_grammar->GetGrammarDataFromNameAndAttrs(
     						    op_name,nln,NULL,(U8*)"OPSPACING",
                                &op_zuID,&op_zinfo) ) {
// The "OPSPACING" section of MathML.gmr contain last resort
//  overrides for operator spacing. - mod, etc.
        if ( op_zinfo && *op_zinfo ) {
          OP_GRAMMAR_INFO op_record;
          GetAttribsFromGammarInfo( op_zinfo,op_record );
          if ( op_record.attr_list ) {

            ATTRIB_REC* a_rover =  op_record.attr_list;
            while ( a_rover ) {
              U8* attr_nom  =  a_rover->attr_nom;
              U8* attr_val  =  a_rover->z_val;
              if ( !strcmp((char*)attr_nom,"lspace")
              ||   !strcmp((char*)attr_nom,"rspace") )
                SetNodeAttrib( mo,attr_nom,attr_val );
              a_rover =  a_rover->next;
            }

            DisposeAttribs( op_record.attr_list );
            op_record.attr_list =  NULL;
          }
        }

        done  =  TRUE;
      }
    }
    
    if ( !done ) {

      U8* attr_nom  =  is_left ? (U8*)"lspace" : (U8*)"rspace";
      if        ( script_nothing ) {
    // Do nothing
        ATTRIB_REC* ar  =  LocateAttribRec( mo,attr_nom );
        if ( ar ) {
          TCI_ASSERT(0);
        }
      } else if ( script_zval ) {
        SetNodeAttrib( mo,attr_nom,zval );
      } else if ( script_zero ) {
        SetNodeAttrib( mo,attr_nom,(U8*)"0.0em" );
      }
    }

  }   // if ( mo )
}



ATTRIB_REC* LaTeX2MMLTree::LocateAttribRec( TNODE* op_node,
                                            U8* targ_attr_nom ) {

  ATTRIB_REC* rv  =  NULL;

  if ( op_node && op_node->attrib_list ) {
    ATTRIB_REC* a_rover =  op_node->attrib_list;
    while ( a_rover ) {
      if ( !strcmp((char*)targ_attr_nom,(char*)a_rover->attr_nom) ) {
        rv  =  a_rover;
        break;
      } else
        a_rover =  a_rover->next;
    }
  }

  return rv;
}



TCI_BOOL LaTeX2MMLTree::GetScriptStatus() {

  TCI_BOOL rv  =  FALSE;

  if ( in_display ) {
    if ( script_level > 1 )
      rv  =  TRUE;
  } else {
    if ( script_level > 0 )
      rv  =  TRUE;
  }

  return rv;
}


//TeX documents the size of the spaces that it inserts between math atoms
// as follows:
//  0 - no extra space inserted
//  1 - thin space
//  2 - medium space
//  3 - thick space
//  * - atoms don't occur next to each other

void LaTeX2MMLTree::SetMOSpacingFromID( TNODE* op_node,I16 TeX_space_ID,
                                    TCI_BOOL is_left,TCI_BOOL in_script ) {

  if ( TeX_space_ID != 99 ) {
    U8 zattr_val[80];
    double mus  =  0.0;
    if ( TeX_space_ID < 0 )
      TeX_space_ID =  in_script ? 0 : (-1*TeX_space_ID);
    if ( TeX_space_ID > 0 )
      mus =  TeX_space_ID + 2.0;
	  sprintf( (char*)zattr_val,"%fem",mus / 18.0 );
    SetMOSpacing( op_node,is_left,zattr_val );
  }
}


// Re-script \U{} nodes as symbol nodes.
// Called by TranslateMathList and UnitInMath2MML.
// Removing \U{}s early simplifies ongoing processing.
// NOTE: \U{2124} might be mapped to \mathbb{Z}

void LaTeX2MMLTree::UnicodesToSymbols( TNODE* LaTeX_list ) {

  TNODE* rover  =  LaTeX_list;
  while ( rover ) {    		// Loop thru first level objects
// \U<uID5.298.0>!\U!REQPARAM(5.298.2,NONLATEX)
// \UNICODE<uID5.299.0>!\UNICODE!OPTPARAM(5.299.1,NONLATEX)REQPARAM(5.299.2,NONLATEX)
    U16 uobjtype,usubtype,uID;
    GetUids( rover->zuID,uobjtype,usubtype,uID );
    if ( uobjtype==5 && usubtype>=298 && usubtype<=299 ) {
      U32 unicode =  ExtractUNICODE( rover );
      if ( unicode ) {
// uIDs starting with <uID11. map directly to unicodes - see how below.
// The production part of these lines is another uID. The second
//  uID is that of the symbolic (named) version of the same character.
// &cent;<uID11.0.162>3.17.64   ->  &cent;<uID3.17.64>,U000A2
// &deg;<uID11.0.176>3.17.159   ->  &deg;<uID3.17.159>,U000B0
// &Copf;<uID11.0.8450>3.17.154 ->  &Copf;<uID3.17.154>,U02102
// etc.
        U32 div =  unicode / 0x10000;
        U32 mod =  unicode % 0x10000;
        U8 zuID[32];
        UidsTozuID( 11, div, mod, (U8*)zuID );
        U8* dest_zname;
        U8* d_template;
        if ( d_mml_grammar->GetGrammarDataFromUID(zuID,
					          context_math,&dest_zname,&d_template) ) {
          if ( d_template && *d_template=='3' ) {
            strcpy( (char*)rover->zuID,(char*)d_template );
            TNODE* parts  =  rover->parts;
            rover->parts  =  NULL;
            parts->sublist_owner  =  NULL;
            DisposeTList( parts );
          } else
            TCI_ASSERT(0);
		} else{
          //TCI_ASSERT(0);
		}
      } else    // if ( unicode )
        TCI_ASSERT(0);

    }   // \U{} clause

    rover =  rover->next;
  }   // loop thru MATH contents list
}



//<uID12.1.4>    (%thechapter%.%thesection%.%theequation%)

void LaTeX2MMLTree::FormatCurrEqnNumber( U8* ztag ) {

  ztag[0] =  0;

  U8* dest_zname;
  U8* d_template;
  if ( d_mml_grammar->GetGrammarDataFromUID(
	                      (U8*)"12.1.16",(U8*)"MMLCONTEXT",
						                &dest_zname,&d_template) ) {
    if ( d_template && *d_template ) {
      U16 tag_len =  0;

      char* ptr =  (char*)d_template;
      while ( *ptr  ) {
        char* p_token =  strchr( ptr,'%' );
        if ( p_token ) {
          U16 tok_len =  0;
          I16 thecounter;
          if        ( !strncmp(p_token,"%thechapter%",12) ) {
            tok_len =  12;
            thecounter  =  thechapter;
          } else if ( !strncmp(p_token,"%thesection%",12) ) {
            tok_len =  12;
            thecounter  =  thesection;
          } else if ( !strncmp(p_token,"%theequation%",13) ) {
            tok_len =  13;
            thecounter  =  theequation;
          } else {
            ztag[ tag_len++ ] =  '%';
            ptr++;
          }

          if ( tok_len ) {
            while ( ptr < p_token ) {
              ztag[ tag_len++ ] =  *ptr;
              ptr++;              
            }
            ptr +=  tok_len;

            // JCS non-standard: itoa( thecounter,(char*)ztag+tag_len,10 );
            sprintf((char*)ztag+tag_len, "%d", thecounter);
            tag_len =  strlen( (char*)ztag );
          }

        } else {
          strcpy( (char*)ztag+tag_len,ptr );
          break;
        }

      }     // loop thru d_template

    }

  } else
    TCI_ASSERT(0);

}


// Several MathML schemata ( msqrt, mtd, mstyle, mpadded, etc. )
//  take a single mrow as their contents.  This first level
//  nested mrow need not be scripted - it's implied.
// The arg to the following function is a MathML list that's
//  about to be nested in a construct that takes an implied mrow.
// Currently, I'm striping an outer level mrow here.

TNODE* LaTeX2MMLTree::FixImpliedMRow( TNODE* mml_node ) {

  TNODE* rv =  mml_node;

  if ( mml_node && !mml_node->next ) {  // test for single node
    U16 uobjtype,usubtype,uID;
    GetUids( mml_node->zuID,uobjtype,usubtype,uID );
    if ( uobjtype==5 && usubtype==750 && uID==1 ) { // mrow
      if ( !mml_node->attrib_list ) {
        if ( mml_node->parts && mml_node->parts->contents ) {
          rv  =  mml_node->parts->contents;
          mml_node->parts->contents =  NULL;
        } else {
          //TCI_ASSERT(0);
          rv  =  NULL;
        }
        DisposeTList( mml_node );
      }
    }
  }

  return rv;
}


TCI_BOOL LaTeX2MMLTree::GetUnicodeEntity( U8* d_template,
                                            U8* entity_buffer ) {

  TCI_BOOL rv =  FALSE;

  if ( d_template && *d_template ) {    // right side data exists
    char* p =  strstr( (char*)d_template,",U" );
    if      ( p )
      p +=  2;                    // step over ",U"
    else if ( *d_template=='U' )
      p =  (char*)d_template + 1; // step over "U"

    U16 offset  =  0;                                  
    while ( p ) {
      U32 unicode =  0;
      U8 ch;
      while ( ch = *p  ) {
        if        ( ch>='0' && ch<='9' ) {
          unicode =  16 * unicode + (ch-'0');
        } else if ( ch>='A' && ch<='F' ) {
          unicode =  16 * unicode + (ch-'A'+10);
        } else if ( ch>='a' && ch<='f' ) {
          unicode =  16 * unicode + (ch-'a'+10);
        } else
          break;
        p++;
      }
      if ( unicode ) {
        rv  =  TRUE;
        if ( ch==0 || ch==',' || ch==':' ) {
          CreateUnicodeEntity( entity_buffer,offset,unicode );
          break;
        } else if ( ch=='\'' ) {
          CreateUnicodeEntity( entity_buffer,offset,unicode );
          offset  =  strlen( (char*)entity_buffer );
          p +=  2;              // step over "'U"
        } else {
		  TCI_ASSERT(0);
          break;
		}
	  } else
	    break;
    }

  }

  return rv;
}


void LaTeX2MMLTree::CreateUnicodeEntity( U8* buffer,U16 offset,
                                                    U32 unicode ) {

  if      ( output_entities_as_unicodes == 1 )
    sprintf( (char*)buffer+offset,"&#%u;",unicode );
  else
    sprintf( (char*)buffer+offset,"&#x%x;",unicode );
}


// MML TNODEs are generated to carry the MathML translations
//  of LaTeX TNODEs from the parse phase.  The following 
//  function attaches "chdata" to these MML TNODEs.
//  (This chdata generally comes from MathML.gmr)
// The chdata consists of MathML &entities; and possibly
//  &#unicode; versions IF unicodes are required in the 
//  final output.

void LaTeX2MMLTree::SetChData( TNODE* node,U8* ent_val,U8* uni_val ) {

  if ( ent_val ) {
    if ( node->var_value )
      delete node->var_value;
    U16 zln   =  strlen( (char*)ent_val );
    char* tmp =  TCI_NEW( char[zln+1] );
    strcpy( tmp,(char*)ent_val );
    node->var_value =  (U8*)tmp;
    node->v_len     =  zln;
  }
  if ( output_entities_as_unicodes && uni_val ) {
    if ( node->var_value2 )
      delete node->var_value2;
    U16 zln   =  strlen( (char*)uni_val );
    char* tmp =  TCI_NEW( char[zln+1] );
    strcpy( tmp,(char*)uni_val );
    node->var_value2  =  (U8*)tmp;
  }
}


TCI_BOOL LaTeX2MMLTree::InsideOfList( TNODE* node ) {

  TCI_BOOL rv =  FALSE;
  if ( node ) {
    TNODE* first  =  node;
	while ( first->prev )
      first =  first->prev;    
    if ( first && first->sublist_owner ) {
      TNODE* parent  =  first->sublist_owner;
      U16 uobjtype,usubtype,uID;
      GetUids( parent->zuID,uobjtype,usubtype,uID );
      if ( uobjtype==5 && usubtype==70 && uID==2 )
        rv  =  TRUE;
    }
// more as needed
  }

  return rv;
}


// Certain units - &deg;  &prime;  &Prime;
//  aren't separated from the element that they qualify

TCI_BOOL LaTeX2MMLTree::NonSpacedUnit( TNODE* MML_unit_mi ) {
  
  TCI_BOOL rv =  FALSE;
  if ( MML_unit_mi ) {
    if ( MML_unit_mi->var_value ) {
      char* ent =  (char*)MML_unit_mi->var_value;
	  if ( !strcmp(ent,"&deg;")
	  ||   !strcmp(ent,"&prime;")
	  ||   !strcmp(ent,"&Prime;") )
        rv  =  TRUE;
    }
  }
  return rv;
}


TCI_BOOL LaTeX2MMLTree::IsKnownOperator( U8* limfunc_name ) {

  TCI_BOOL rv =  FALSE;

  if ( limfunc_name ) {
    U16 ln  =  strlen( (char*)limfunc_name );
    switch ( ln ) {
      case 3 :
        if      ( !strcmp((char*)limfunc_name,"div") )
          rv  =  TRUE;
        else if ( !strcmp((char*)limfunc_name,"lcm") )
          rv  =  TRUE;
	  break;
      case 4 :
        if      ( !strcmp((char*)limfunc_name,"curl") )
          rv  =  TRUE;
        else if ( !strcmp((char*)limfunc_name,"grad") )
          rv  =  TRUE;
	  break;
      default :
	  break;
    }
  }

  return rv;
}


U16 LaTeX2MMLTree::GetDegMinSec( TNODE* MML_node ) {

  U16 rv  =  0;

  if ( MML_node ) {
    U16 uobjtype,usubtype,uID;
    GetUids( MML_node->zuID,uobjtype,usubtype,uID );

    TNODE* c2 =  NULL;
    if ( usubtype==750 && uID==1
    &&   MML_node->parts 
    &&   MML_node->parts->contents ) {
      c2 =  MML_node->parts->contents;
	  if ( c2 ) c2  =  c2->next;
	  if ( c2 ) c2  =  c2->next;
    }
    if ( !c2
    &&   uobjtype==5 && usubtype==51 && uID==2
    &&   MML_node->parts 
    &&   MML_node->parts->next
    &&   MML_node->parts->next->contents ) {
      c2 =  MML_node->parts->next->contents;
    }

    if ( c2
    &&   c2->var_value ) {
      if      ( !strcmp((char*)c2->var_value,"&deg;") )     // degree
        rv  =  1;
      else if ( !strcmp((char*)c2->var_value,"&prime;") )   // minute
        rv  =  2;
      else if ( !strcmp((char*)c2->var_value,"&Prime;") )   // second
        rv  =  3;
    }
  }

  return rv;
}


// The following function is called from InsertInvisibleTimes
//  to block the insertion of &InvisibleTimes; within 30 deg 15" 5'

TCI_BOOL LaTeX2MMLTree::IsDegreeMinSec( TNODE* mml_rover ) {

  TCI_BOOL rv =  FALSE;

  if ( mml_rover
  &&   mml_rover->prev ) {
    U16 t1 =  GetDegMinSec( mml_rover->prev );
    if ( t1 ) {
      U16 t2 =  GetDegMinSec( mml_rover );
      if        ( t1 == 1 ) {     // degree
        if ( t2 >= 2 )
          rv =  TRUE;
      } else if ( t1 == 2 ) {     // minute
        if ( t2 == 3 )
          rv =  TRUE;
      }	
    }
  }

  return rv;
}


