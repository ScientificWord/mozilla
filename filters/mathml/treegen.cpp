
#include "treegen.h"
#include "fltutils.h"
#include "logfiler.h"
#include "grammar.h"
#include "tokizer.h"
#include "texparse.h"
#include "nthmstor.h"
#include "OutWrap.h"


//#include "tci_new.h"
#include <string.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//ctor

TreeGenerator::TreeGenerator( U16 output_ilk,
                                const char* LaTeX_gmr_fspec,
                                const char* XML_gmr_fspec,
                                const char* MATH_gmr_fspec ) {

  output_type =  output_ilk;

// Construct the objects that persist over filtering runs.

  FILE* fp  =  fopen( LaTeX_gmr_fspec,"r" );
  if ( fp ) {
    s_tex_grammar =  TCI_NEW( Grammar( fp,FALSE ) );
    fclose( fp );
  }

  tokizer       =  NULL;
  theorem_store =  NULL;
  logfiler      =  NULL;
  p_userprefs   =  NULL;

  d_xml_grammar =  NULL;
  if ( XML_gmr_fspec && XML_gmr_fspec[0] ) {
    FILE* fp  =  fopen( XML_gmr_fspec,"r" );
    if ( fp ) {
      d_xml_grammar =  TCI_NEW( Grammar( fp,TRUE ) );
      fclose( fp );
    }
  }

  s_math_grammar =  NULL;
  if ( MATH_gmr_fspec && MATH_gmr_fspec[0] ) {
    FILE* fp  =  fopen( MATH_gmr_fspec,"r" );
    if ( fp ) {
      s_math_grammar =  TCI_NEW( Grammar( fp,FALSE ) );
      fclose( fp );
    }
  }

  d_math_grammar =  NULL;
  if ( MATH_gmr_fspec && MATH_gmr_fspec[0] ) {
    FILE* fp  =  fopen( MATH_gmr_fspec,"r" );
    if ( fp ) {
      d_math_grammar =  TCI_NEW( Grammar( fp,TRUE ) );
      fclose( fp );
    }
  }

  TeXparser     =  NULL;
  srcfile       =  NULL;
  curr_src_off  =  0;

  eoln	  =  "\n";
  do_xml  =  1;
  entity_mode  =  1;
  pLaTeX2MATH  =  NULL;
}


TreeGenerator::~TreeGenerator() {

  CheckTNODETallies();		// diagnostics in FLTUTILS

  if ( s_tex_grammar )      delete s_tex_grammar;
  if ( tokizer )            delete tokizer;
  if ( theorem_store )      delete theorem_store;
  if ( TeXparser )			delete TeXparser;
  if ( srcfile ) {
    fclose( srcfile );
	  srcfile =  NULL;
  }

  if ( s_math_grammar )     delete s_math_grammar;
  if ( d_math_grammar )     delete d_math_grammar;
  if ( pLaTeX2MATH )        delete pLaTeX2MATH;
}


// A public function to translate between SciNotebook LaTeX and
// MathType when both source and destination are ByteArrays.
// Often the source for this translation is the Clipboard.
// Note the caller of this function must do some setup work.
//  Certain files, like the log, must be opened
//  and TreeGenerator->Reset needs to be called.

TNODE* TreeGenerator::ParseBytes( U8* context,U8* src_ba,
                               	    TCI_BOOL is_clipbrd,
                                   		U16& nlog_msgs ) {

  TNODE* rv =  NULL;

  U16 new_log_msgs;
  if ( logfiler )
    new_log_msgs  =  logfiler->GetLogMsgCount();

/*
  theorem_store->??
*/
  //LaTeXObjCounter* ba_countman  =  
  //      TCI_NEW( LaTeXObjCounter() );

  TeXParser* TeX_ba_parser  =
    	TCI_NEW( TeXParser( s_tex_grammar,tokizer,NULL,logfiler ) );

  TNODE* parse_tree =  TeX_ba_parser->Parse( context,src_ba,is_clipbrd );
  if ( parse_tree ) {

	if ( FUOpenDumpFile() ) {
	  FUCloseDumpFile();
      DumpLine( "\nDumping Source Parse Tree - from Bytes\n\n" );
	  U16 tokenizerID =  s_tex_grammar->GetTokenizerID();
      DumpTList( parse_tree,0,tokenizerID );
      DumpLine( "\n\n" );
	}

    rv  =  parse_tree;
  }

  if ( logfiler )
    nlog_msgs  =  logfiler->GetLogMsgCount() - new_log_msgs;
  else
    nlog_msgs =  0;

  delete TeX_ba_parser;

  return rv;
}


void* TreeGenerator::BufferToDestTree( U8* src_LaTeX,
                               		TCI_BOOL is_clipbrd,
                                   	U16& nlog_msgs,
                                    ANOMALY_REC* anomalies,
                                    U8& eqn_option ) {

// Generate a "LaTeX" parse tree from src_LaTeX
  TNODE* TeX_tree =  ParseBytes( NULL,src_LaTeX,is_clipbrd,nlog_msgs );

// Generate a "MTEF" parse tree from TeX_tree
  void* temp  =  ConvertParseTree( TeX_tree,FALSE,anomalies,eqn_option );

  DisposeTList( TeX_tree );

  return temp;
}


void TreeGenerator::SetNewFile( const char* new_srcfile_spec,
							    int make_xml,U16 e_mode,U16& result,
							    const char** context_zstrs,
							    int* context_ints ) {

  do_xml  =  make_xml;
  entity_mode =  e_mode;

  if ( srcfile ) {			// close previous source file
    fclose( srcfile );
	  srcfile =  NULL;
  }

  if ( theorem_store )
    delete theorem_store;
  theorem_store =  TCI_NEW( NewObjectStore(s_tex_grammar) );

  if ( new_srcfile_spec && new_srcfile_spec[0] ) {
    srcfile =  fopen( new_srcfile_spec,"r" );
	if ( srcfile )
  	  tokizer =  TCI_NEW( Tokizer(s_tex_grammar,
  	  					    theorem_store,srcfile) );
	else
	  TCI_ASSERT(0);
  }

  curr_src_off  =  0L;;
  batch_count   =  0L;

  if ( TeXparser )			// we create a new parser for each file
    delete TeXparser;

  if ( srcfile ) {
    TeXparser  =  TCI_NEW( TeXParser( s_tex_grammar,tokizer,srcfile,logfiler ) );
	result  =  0;
  } else {				// null srcfile?
    TeXparser  =  NULL;
	result  =  1;
  }

  if ( pLaTeX2MATH )
    delete pLaTeX2MATH;

  pLaTeX2MATH  =  TCI_NEW( OutTreeGenerator( output_type,this,
                            NULL,
                            s_math_grammar,
  							d_math_grammar,
                            logfiler,
                            p_userprefs,
                            context_zstrs,
                            context_ints ) );

}



void TreeGenerator::OnStartNewFile( int make_xml,U16 e_mode,U16& result,
							        const char** context_zstrs,
                                    int* context_ints ) {

  do_xml  =  make_xml;
  entity_mode =  e_mode;

  if ( srcfile ) {			// close previous source file
    fclose( srcfile );
	  srcfile =  NULL;
  }

  if ( theorem_store )
    delete theorem_store;
  theorem_store =  TCI_NEW( NewObjectStore(s_tex_grammar) );

  tokizer =  TCI_NEW( Tokizer(s_tex_grammar,theorem_store,srcfile) );

  curr_src_off  =  0L;;
  batch_count   =  0L;

  if ( TeXparser )			// we create a new parser for each file
    delete TeXparser;
  TeXparser  =  TCI_NEW( TeXParser( s_tex_grammar,tokizer,srcfile,logfiler ) );

  if ( pLaTeX2MATH )
    delete pLaTeX2MATH;
  pLaTeX2MATH  =  TCI_NEW( OutTreeGenerator( output_type,this,
                            NULL,
                            s_math_grammar,
  							d_math_grammar,
                            logfiler,
                            p_userprefs,
                            context_zstrs,
                            context_ints ) );

  result  =  0;
}


TNODE* TreeGenerator::ParsePreamble( U16& result ) {

  TNODE* rv =  NULL;

// If there is a preamble in the LaTeX we're converting,
//  it must be parsed and processed.  Macros, newtheorems,
//  newenvironments, counter settings, etc. must be stored.

  U32 bytesdone;
  TNODE* LaTeX_preamble =  TeXparser->ParsePreamble( bytesdone,result );
  curr_src_off  =  bytesdone;

  if ( result == 0 ) {	// success
    ProcessPreamble( LaTeX_preamble );
/*
	  if ( do_xml ) {
	    if ( LaTeX_preamble ) {
        rv  =  (TNODE*)ConvertParseTree( LaTeX_preamble,TRUE,NULL );
        DisposeTList( LaTeX_preamble );
	    }		// if ( preamble )

	  } else
*/
      rv  =  LaTeX_preamble;

  }	// if ( result == 0 )

  return rv;
}


// 

TNODE* TreeGenerator::ParseBatch( U16& result,U16& log_tally ) {

  TNODE* rv =  NULL;

  result  =  PR_INCOMPLETE;

  U16 log_msgs_before,log_msgs_after;
  if ( logfiler )
    log_msgs_before =  logfiler->GetLogMsgCount();

  U16 parse_result;
  U32 save_src_off  =  curr_src_off;
  TNODE* src_parse_tree =  TeXparser->ParseBatch( curr_src_off,
  												parse_result );

  if ( logfiler ) {
    log_msgs_after  =  logfiler->GetLogMsgCount();
    log_tally =  log_msgs_after - log_msgs_before;
  } else
    log_tally =  0;

  if ( parse_result == PR_ERROR ) {       // possible error exit
    result  =  2;

    return rv;		// this is NULL
  }

  if ( src_parse_tree ) {			// parsing of source para succeeded

	if ( FUOpenDumpFile() ) {
      DumpLine( "\nDumping Source Parse Tree\n\n" );
      U16 tokenizerID =  s_tex_grammar->GetTokenizerID();
      DumpTList( src_parse_tree,0,tokenizerID );
      DumpLine( "\n\n" );
	    FUCloseDumpFile();
	}

    if ( do_xml ) {
      U8 eqn_option;
      void* temp  =  ConvertParseTree( src_parse_tree,FALSE,NULL,eqn_option );
      rv  =  (TNODE*)temp;
      DisposeTList( src_parse_tree );
	} else
      rv  =  src_parse_tree;

    batch_count++;
  }

  if ( parse_result == PR_COMPLETE ) {
    result  =  PR_COMPLETE;
  }

  return rv;
}


// Build an XML tree that is equivalent to our LaTeX tree.

void* TreeGenerator::ConvertParseTree( TNODE* nb_parse_tree,
                                    TCI_BOOL is_preamble,
                                    ANOMALY_REC* anomalies,
                                    U8& eqn_option ) {

  void* dst_parse_tree  =  NULL;

  if ( nb_parse_tree ) {	  
    U16 result;
    dst_parse_tree =	pLaTeX2MATH->CreateDestTree( nb_parse_tree,
                                    eqn_option,anomalies,result );
  }

  if ( dst_parse_tree && is_preamble ) {
//  pLaTeX2XML->FixTCIDataInPreamble( (TNODE*)dst_parse_tree );
//  pLaTeX2XML->RemovePatchComment( (TNODE*)dst_parse_tree );
  }

  if ( FUOpenDumpFile() ) {
    DumpLine( "\nDumping re-built XML Parse Tree\n\n" );
    DumpLine( "Source Line       Contents\n" );
    U16 tokenizerID   =  d_xml_grammar->GetTokenizerID();
//    DumpTList( dst_parse_tree,0,tokenizerID );
    DumpLine( "\n\n" );
	  FUCloseDumpFile();
  }

  return dst_parse_tree;
}


void TreeGenerator::ClearTree( TNODE* tree ) {

  DisposeTList( tree );
}


// The dialog app has a static control that displays a chunk
//  of the source.  This function appends to the string
//  of source shown to the user by the app.

#define MAX_SOURCE_SAMPLE_LEN 1024

TCI_BOOL TreeGenerator::AppendSrc( U32 src_off,char* srcstr,
										TeXParser* parser ) {

  TCI_BOOL rv  =  FALSE;

  char* end_of_line =  "\n";

  U16 out_msg_ln  =  strlen( srcstr );
  if ( out_msg_ln < MAX_SOURCE_SAMPLE_LEN - 24 ) {
    rv  =  TRUE;
    if ( out_msg_ln ) {     // add paragraph sep
      strcpy( srcstr+out_msg_ln,end_of_line );
      out_msg_ln  +=  strlen( end_of_line );
      //strcpy( srcstr+out_msg_ln,end_of_line );
      //out_msg_ln  +=  strlen( end_of_line );
    }

    U8* tmp =  parser->GetSrcPtr( src_off );
    U16 bytes_to_copy =  strlen( (char*)tmp );

    U16 si  =  0;
    U16 di  =  out_msg_ln;
    while ( si < bytes_to_copy && di < MAX_SOURCE_SAMPLE_LEN - 2 ) {
      char ch =  tmp[si++];
      srcstr[di++]  =  ch;
      if ( ch == '&' )
        srcstr[di++]  =  ch;
    }
    srcstr[ di ] =  0;

  }       //  if ( out_msg_ln < MAX_SOURCE_SAMPLE_LEN - 24 )

  return rv;
}


void TreeGenerator::Reset( USERPREFS* userprefs,
						    LogFiler* new_logfiler,
							char* dump_file_spec ) {

// USERPREFS is a way to pass some configuration info
//  into the dll - it's not used yet.

  if ( userprefs )
    p_userprefs =  userprefs;

// When the TreeGenerator starts {another} run, certain counters
//  must be reset. ( page, chapter, section, etc.)


  logfiler  =  NULL;
  if ( new_logfiler )
    logfiler  =  new_logfiler;

// Pass the FILE pointer down to FLTUTILS where it is kept.
// May want to encapsulate all dump file management
// in a new DumpFiler object.

  if ( dump_file_spec )
    FUSetDumpFile( dump_file_spec );
}



// If there is a preamble in the LaTeX we're converting,
//  it must be parsed and processed.  Macros, newtheorems,
//  newenvironments, counter settings, etc. must be stored.

// We examine the preable TNODEs for newcommands and newtheorems

void TreeGenerator::ProcessPreamble( TNODE* tlist ) {

  if ( FUOpenDumpFile() ) {
    DumpLine( "\nDumping Source Parse Tree - Preamble\n\n" );
  	U16 tokenizer_ID  =  s_tex_grammar->GetTokenizerID();
    DumpTList( tlist,0,tokenizer_ID );
    DumpLine( "\n\n" );
    FUCloseDumpFile();
  }


  TNODE* rover  =  tlist;
  while ( rover ) {

  // recurse into { group } found in preamble

    if        ( !strcmp("6.1.0",(char*)rover->zuID) ) {
      tokizer->EnterExitGroup( TRUE );
        ProcessPreamble( rover->parts->contents );
      tokizer->EnterExitGroup( FALSE );

  // handle \newcommand
	} else if ( !strcmp((char*)rover->zuID,"5.99.0") ) {
      ProcessNewCommand( rover );

  // handle \DeclareMathOperator
	} else if ( !strcmp((char*)rover->zuID,"5.95.1") ) {
      ProcessNewMathOp( rover,false );

  // handle \DeclareMathOperator*
	} else if ( !strcmp((char*)rover->zuID,"5.96.1") ) {
      ProcessNewMathOp( rover,true );

  // handle \newtheorem and \newtheorem*
    } else if ( !strcmp((char*)rover->zuID,"5.155.1")
    ||          !strcmp((char*)rover->zuID,"5.160.1") ) {
      ProcessNewTheorem( rover );

    } else if ( !strcmp((char*)rover->zuID,"5.156.1") ) {
	// \theoremstyle<uID5.156.1>!{!VAR(5.156.2,TEXT,n,,})!}!
      U8* new_val =  rover->parts->var_value;
      tokizer->SetTheoremStyleAttr( TA_style,new_val );

    } else if ( !strcmp((char*)rover->zuID,"5.157.1") ) {
	// \theorembodyfont<uID5.157.1>!{!VAR(5.157.2,TEXT,n,,})!}!
      U8* new_val =  rover->parts->var_value;
      tokizer->SetTheoremStyleAttr( TA_bodyfont,new_val );

    } else if ( !strcmp((char*)rover->zuID,"5.158.1") ) {
	// \theoremheaderfont<uID5.158.1>!{!VAR(5.158.2,TEXT,n,,})!}!
      U8* new_val =  rover->parts->var_value;
      tokizer->SetTheoremStyleAttr( TA_headerfont,new_val );

    } else if ( !strcmp((char*)rover->zuID,"5.154.0") ) {
      ProcessNewEnvironment( rover );

  // handle \setcounter
	} else if ( !strcmp((char*)rover->zuID,"5.355.0") ) {
      theorem_store->ProcessCounterCmd( rover );

    }

    rover =  rover->next;
  }			// loop thru preamble TNODES

}


//  <5.155.1>\newtheorem{lemon}[theorem]{Don't Suck}[chapter]
//    <5.155.2>lemon
//    <5.155.3>[theorem]
//      <888.0.0>theorem
//    <5.155.4>Don't Suck
//    <5.155.5>[chapter]
//      <888.0.0>chapter
/*
\newtheorem<uID5.155.1>!\newtheorem!
REQPARAM(5.155.2,NONLATEX)  env_nom
OPTPARAM(5.155.3,NONLATEX)
REQPARAM(5.155.4,NONLATEX)  typeset caption
OPTPARAM(5.155.5,NONLATEX)
\newtheorem*<uID5.160.1>!\newtheorem*!
REQPARAM(5.160.2,NONLATEX)
REQPARAM(5.160.4,NONLATEX)
*/

void TreeGenerator::ProcessNewTheorem( TNODE* nt_node ) {

  U8* env_nom   =  NULL;
  U8* caption   =  NULL;
  U8* nums_like =  NULL;
  U8* within    =  NULL;
  //U16 scope     =  0;           // \newtheorems are global

  U16 uobjtype,usubtype,uID;
  TNODE* p_rover  =  nt_node->parts;
  while ( p_rover ) {
 	GetUids( p_rover->zuID,uobjtype,usubtype,uID );
    if        ( uID == 2 ) {
      TNODE* contents =  p_rover->contents;
      if ( contents )
        env_nom =  contents->var_value;

    } else if ( uID == 3 ) {
      TNODE* contents =  p_rover->contents;
      if ( contents )
        nums_like =  contents->var_value;

    } else if ( uID == 4 ) {
      TNODE* contents =  p_rover->contents;
      if ( contents )
        caption =  contents->var_value;

    } else if ( uID == 5 ) {
      TNODE* contents =  p_rover->contents;
      if ( contents )
        within  =  contents->var_value;
    }  

    p_rover =  p_rover->next;  
  }			// while ( p_rover )

  TCI_BOOL is_starred =  (usubtype == 160) ? TRUE : FALSE;
  tokizer->AddNewTheorem( 1,env_nom,caption,
  							nums_like,within,is_starred );

// Set up a counter for this newtheorem type, if required

  if ( usubtype==155 && nums_like==NULL )
    theorem_store->CreateCounter( env_nom,0,within );
}


//  <5.99.0>\newcommand{\gnaw}[2]{{\em gnu\/}$(#1;#2)$}
//    <5.99.1>\gnaw
//    <5.99.2>[2]
//      <888.0.0>2
//    <5.99.3>{\em gnu\/}$(#1;#2)$
//\newcommand<uID5.99.0>!\newcommand!
//  !{!NONTEXBUCKET(5.99.1,TEXT,{,})!}!
//  OPTPARAM(5.99.2,TEXT)
//  !{!NONTEXBUCKET(5.99.3,TEXT,{,})!}!

void TreeGenerator::ProcessNewCommand( TNODE* nt_node ) {

  U8* nom     =  NULL;
  U8* def     =  NULL;
  U16 p_count =  0;
  U16 scope   =  0;

  TNODE* p_rover  =  nt_node->parts;
  while ( p_rover ) {
    TNODE* contents =  p_rover->contents;
    if ( contents ) {
      if      ( !strcmp((char*)p_rover->zuID,"5.99.1") )
        nom   =  contents->var_value;
      else if ( !strcmp((char*)p_rover->zuID,"5.99.2") )
        p_count =  atoi( (char*)contents->var_value );
      else if ( !strcmp((char*)p_rover->zuID,"5.99.3") )
        def   =  contents->var_value;
    }  
    p_rover =  p_rover->next;  
  }

  tokizer->AddMacro( nom,def,p_count,scope );
}


//\DeclareMathOperator<uID5.95.1>!\DeclareMathOperator!
//  REQPARAM(5.95.2,NONLATEX)REQPARAM(5.95.3,NONLATEX)
//\DeclareMathOperator*<uID5.96.1>!\DeclareMathOperator*!
//  REQPARAM(5.96.2,NONLATEX)REQPARAM(5.96.3,NONLATEX)

void TreeGenerator::ProcessNewMathOp( TNODE* nt_node,bool is_starred ) {

  U8* nom     =  NULL;
  U8* def     =  NULL;
  U16 scope   =  0;

  TNODE* p_rover  =  nt_node->parts;
  while ( p_rover ) {
    U16 uobj,usub,uID;
 	GetUids( p_rover->zuID,uobj,usub,uID );

    TNODE* contents =  p_rover->contents;
    if ( contents && uobj==5 ) {
      if      ( uID==2 )
        nom   =  contents->var_value;
      else if ( uID==3 )
        def   =  contents->var_value;
    }  
    p_rover =  p_rover->next;  
  }

  tokizer->AddMathOp( nom,def,is_starred,scope );
}


/*
\newenvironment<uID5.154.0>
REQPARAM(5.154.1,NONLATEX) envname
OPTPARAM(5.154.2,NONLATEX) nargs
OPTPARAM(5.154.3,NONLATEX) default-val-optional-arg
REQPARAM(5.154.4,NONLATEX) begdef
REQPARAM(5.154.5,NONLATEX) enddef
*/

void TreeGenerator::ProcessNewEnvironment( TNODE* nt_node ) {

  U8* env_nom   =  NULL;
  U8* n_args    =  NULL;
  U8* opt_def   =  NULL;
  U8* begdef    =  NULL;
  U8* enddef    =  NULL;
  //U16 scope     =  0;           // \newtheorems are global

  TNODE* p_rover  =  nt_node->parts;
  while ( p_rover ) {
    if        ( !strcmp((char*)p_rover->zuID,"5.154.1") ) {
      TNODE* contents =  p_rover->contents;
      if ( contents )
        env_nom =  contents->var_value;

    } else if ( !strcmp((char*)p_rover->zuID,"5.154.2") ) {

      TNODE* contents =  p_rover->contents;
      if ( contents )
        n_args  =  contents->var_value;

    } else if ( !strcmp((char*)p_rover->zuID,"5.154.3") ) {

      TNODE* contents =  p_rover->contents;
      if ( contents ) {
        opt_def   =  contents->var_value;
      }

    } else if ( !strcmp((char*)p_rover->zuID,"5.154.4") ) {
      TNODE* contents =  p_rover->contents;
      if ( contents )
        begdef =  contents->var_value;

    } else if ( !strcmp((char*)p_rover->zuID,"5.154.5") ) {
      TNODE* contents =  p_rover->contents;
      if ( contents )
        enddef  =  contents->var_value;
    }  

    p_rover =  p_rover->next;  
  }			// while ( p_rover )

  tokizer->AddNewEnviron( 1,env_nom,n_args,opt_def,begdef,enddef );
}


Grammar* TreeGenerator::GetMathGrammar( bool get_src ) {

  if ( get_src ) 
    return s_math_grammar;
  else
    return d_math_grammar;
}



/* THE REMAINDER OF THIS FILE IS COMMENTED

// Function to translate a file using lparser.dll
// Call filter->FInitialize first.

U16 Filter::TranslateFile( const char* srcfile_spec,
                            char* dstfile_spec,
                              U16 src_type,U16 dst_type,
                                char* srcstr,char* eoln,int do_xml ) {

  TCI_BOOL do_html  =  (output_mode == 3) ? TRUE : FALSE;

  U16 rv  =  1;         // no errors logged (yet)

  if ( srcstr )
    srcstr[0]   =  0;         // This is a zstring that the user sees
  TCI_BOOL show_src =  TRUE;  //  as static text in our dialog app.
  U32 src_show_off  =  9999L; // Invalid starting value

  U16 result;
  parser->SetNewFile( srcfile_spec,do_xml,1,result,NULL );

  if ( result != 0 ) {
    return 2;
  }

// open a new XML dest file

  U32 dest_file_lineno  =  0L;

  FILE* dstfile =  NULL;
  if ( do_xml && srcfile_spec && srcfile_spec[0] ) {
	char dst_spec[256];
    strcpy( dst_spec,srcfile_spec );
    U16 ln  =  strlen( dst_spec );
    if ( ln>4 )
      if ( !stricmp(dst_spec+ln-4,".tex") )
	    dst_spec[ln-4]  =  0;
    if ( do_html ) {
//	release
//  strcat( dst_spec,".html" );
//

// hardwired dst dir

	  U16 ln  =  strlen( srcfile_spec );
	  if ( ln>4 ) {
	    if ( !stricmp(srcfile_spec+ln-4,".tex") )
		  ln  =  ln - 4;
		while ( ln
		&&      srcfile_spec[ln] != '/'
		&&      srcfile_spec[ln] != '\\'
		&&      srcfile_spec[ln] != ':' )
		  ln--;
		if ( ln ) ln++;
	    strcpy( dst_spec,"E:/tci/swtools/mathml/webeq/" );
	    U16 dln   =  strlen( dst_spec );
		char ch;
		while ( (ch = srcfile_spec[ln]) && (ch != '.') ) {
		  dst_spec[dln++] =  ch;
		  ln++;
		}
	    strcpy( dst_spec+dln,".html" );
// end hardwired dst dir

	  } else {
	    TCI_ASSERT(0);
	    strcpy( dst_spec,"E:/tci/swtools/mathml/webeq/test.html" );
	  }



	} else {
	  strcat( dst_spec,".xml" );
	}
    dstfile =  fopen( dst_spec,"wt" );
  }

  U16 indent  =  0;

  TNODE* preamble =  parser->ParsePreamble( result );
  if ( result == 0 ) {

	if ( do_xml ) {
	  if ( !do_html ) {
	    TCI_BOOL start_tag  =  TRUE;
        XMLTagToDstfile( (U8*)"xml version=\"1.0\"",start_tag,  // <?xml...?>
      					indent,dstfile,dest_file_lineno );

// Added preface for Mazilla
dest_file_lineno++;
fputs(
"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"mathml.dtd\">\n",
dstfile );
dest_file_lineno++;
fputs(
"<?xml-stylesheet type=\"text/xsl\" href=\"snb.xsl\"?>\n",
dstfile );
// End Added preface for Mazilla

        XMLTagToDstfile( (U8*)"DOCUMENT",start_tag,			  // <document>
      					indent,dstfile,dest_file_lineno );
	    if ( preamble ) {
          XMLTreeToDstfile( preamble,(U8*)"PREAMBLE",
        				indent,dstfile,dest_file_lineno );
          //DisposeTList( preamble );
	    }		// if ( preamble )

      } else {
        HTMLPrefaceToDstfile( dstfile,dest_file_lineno );
	  }

	}		// if do_xml

  } else {
	TCI_ASSERT(0);
    if ( dstfile ) fclose( dstfile );
    if ( preamble )
      parser->ClearTree( preamble );

    return 2;
  }


  if ( preamble )
    parser->ClearTree( preamble );

  U32 batch_count =  0;

  U16 log_entry_counter =  0;
  U16 parse_result  =  PR_INCOMPLETE;
  while ( parse_result == PR_INCOMPLETE ) { // loop thru "paras" of src

    U16 nlog_entries;
    TNODE* src_parse_tree =  parser->ParseBatch( parse_result,
    											nlog_entries );
    log_entry_counter +=  nlog_entries;

    if ( parse_result == PR_ERROR ) {       // possible error exit
      rv  =  2;

      TCI_BOOL start_tag  =  FALSE;
	  if ( do_xml ) {
	    if ( !do_html ) {
          if ( batch_count > 0 )
            XMLTagToDstfile( (U8*)"BODY",start_tag,
          				indent,dstfile,dest_file_lineno );	// end body
          XMLTagToDstfile( (U8*)"DOCUMENT",start_tag,
        				indent,dstfile,dest_file_lineno );	// end document
        } else {
	      HTMLEpilogToDstfile( dstfile,dest_file_lineno );
		}

	  }

      break;
    }

    if ( !src_parse_tree ) {
	  break;

    } else {			// parsing of source para succeeded

  // Write this batch to dstfile.xml

      if ( do_xml ) {
        if ( batch_count==0 ) {
	      if ( !do_html ) {
            TCI_BOOL start_tag  =  TRUE;				// <body>
            XMLTagToDstfile( (U8*)"BODY",start_tag,
          					indent,dstfile,dest_file_lineno );
          }
        }
        XMLTreeToDstfile( src_parse_tree,NULL,
        					indent,dstfile,dest_file_lineno );
	  }

      batch_count++;
      parser->ClearTree( src_parse_tree );

    }       //  if ( obj_list ) - from parser

  }     // while loop thru para blocks of source

  TCI_BOOL start_tag  =  FALSE;
  if ( do_xml ) {
    if ( !do_html ) {
      if ( batch_count > 0 )
        XMLTagToDstfile( (U8*)"BODY",start_tag,
      					indent,dstfile,dest_file_lineno );	// end body
      XMLTagToDstfile( (U8*)"DOCUMENT",start_tag,
    					indent,dstfile,dest_file_lineno );	// end document
	} else {
	  HTMLEpilogToDstfile( dstfile,dest_file_lineno );
	}
  }

  if ( log_entry_counter )
    rv  =  2;
  if ( logfiler->GetLogMsgCount() )	// some messages were logged
    rv  =  2;

  if ( dstfile )
    fclose( dstfile );

  return rv;
}


////////////////////////////////////////////////////////////////////

// XML output to file, line breaking

U8* Filter::OutputXML( TILE* tiles,FILE* dstfile,
						char* eoln,U16 line_lim,
						  U16& indent,
							U32& dest_file_lineno ) {

  U8* rv  =  NULL;

  char* line  =  TCI_NEW( char[32767] );
  U16 li  =  0;

// initial indent is passed in!

  if ( indent )				// indent new line
	while ( li<indent ) line[li++]  =  ' ';

  char* eoln_seq  =  "\n";

  TCI_BOOL first_tile =  TRUE;
  TILE* rover =  tiles;
  while ( rover ) {

    switch ( rover->ilk ) {

      case TT_A_ELEM_HEADER	  :			// tiles that start
	  case TT_S_ELEM_HEADER	  : {		//  a new line
	    if ( li && !first_tile ) {		
    	  strcat( line,eoln_seq );
          dest_file_lineno++;			// output current line
          fputs( line,dstfile );
		  li =  0;				  		// indent next line
		  while ( li<indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );

        if ( rover->ilk == TT_S_ELEM_HEADER )
		  indent  +=  2;
	  }
	  break;

      case TT_ATOM_BODY		  : {
	    //line[li++]  =  ' ';		no space after headtag
        if ( rover->zval ) {
// Implement line breaking here!
		  U16 vlen  =  strlen( (char*)rover->zval );
		  if ( vlen+li < 32767 )
            strcpy( line+li,(char*)rover->zval );
		  else
		    TCI_ASSERT(0);
		}
        li  =  strlen( line );
	  }
	  break;

      case TT_A_ELEM_ENDER	  : {
	    //line[li++]  =  ' ';		no space before tailtag
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
	  }
	  break;

      case TT_S_ELEM_ENDER	  : {
	    if ( li ) {
	      if ( !first_tile ) {		// output current line
    	    strcat( line,eoln_seq );
            dest_file_lineno++;
            fputs( line,dstfile );
		  }
		  indent -=  2;
		  li =  0;				  	// indent next line
		  while ( li<indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
	  }
	  break;

      case TT_ALIGNGROUP	: {
	    if ( li && !first_tile ) {		// output current line
    	  strcat( line,eoln_seq );
          dest_file_lineno++;
          fputs( line,dstfile );
		  li =  0;				  		// indent next line
		  while ( li<indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
	  }
	  break;

	  default :
	    TCI_ASSERT(0);
	  break;
	}

  // log any msgs carried by this xml_tile

	LOG_MSG_REC* msg_rover  =  rover->msg_list;
	while ( msg_rover ) {
      logfiler-> MsgToLog( msg_rover,dest_file_lineno );
	  msg_rover =  msg_rover->next;
    }
	DisposeMsgs( rover->msg_list );
	rover->msg_list =  NULL;

    first_tile  =  FALSE;
    rover =  rover->next;
  }

  if ( li ) {
	strcat( line,eoln_seq );
    dest_file_lineno++;
    fputs( line,dstfile );
  }

  delete line;
  return rv;
}

////////////////////////////////////////////////////////////////////

// XML output to rendering function

void Filter::RenderXML( TILE* tiles,
						  U16 start_indent,
						    U16 indent_increment,
  							  char* eoln_seq,
							    ANOMALY_REC* anomaly_list,
								  U8* src_LaTeX,
						            FILTRENDERTILE renderfunc ) {

  U32 dest_lineno =  0;

  char* line  =  TCI_NEW( char[32768] );
  U16 li  =  0;

// send anomaly reports

  ANOMALY_REC* a_rover  =  anomaly_list;
  while ( a_rover ) {
    if ( a_rover->ilk ) {
	  line[0] =  0;
      if      ( a_rover->atext )
	    strcpy( line,(char*)a_rover->atext );
	  else if ( a_rover->off1 ) {
	    U32 ln  =  a_rover->off2 - a_rover->off1;
	    if ( ln < 32768 ) {
	      strncpy( line,(char*)src_LaTeX+a_rover->off1,ln );
	     line[ln] =  0;
	    }
	  }
      (*renderfunc)( a_rover->ilk,line );
	}

    a_rover =  a_rover->next;
  }


// ambiant indent is passed in!
  U16 curr_indent =  start_indent;

// indent the first line, if necessary
  if ( curr_indent )
	while ( li<curr_indent ) line[li++]  =  ' ';


  TCI_BOOL first_tile =  TRUE;
  TILE* rover =  tiles;
  while ( rover ) {				// loop thru tiles

    switch ( rover->ilk ) {

      case TT_A_ELEM_HEADER	:			// tiles that start
	  case TT_S_ELEM_HEADER	: {			//  a new line
	    if ( li && !first_tile ) {		
    	  if ( eoln_seq ) strcat( line,eoln_seq );
          dest_lineno++;				// output current line
          (*renderfunc)( rover->ilk,line );
		  li =  0;				  		// indent next line
		  while ( li<curr_indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );

        if ( rover->ilk == TT_S_ELEM_HEADER )
		  curr_indent +=  indent_increment;
	  }
	  break;

      case TT_ATOM_BODY		: {
	    //line[li++]  =  ' ';		no space after headtag
        if ( rover->zval ) {
// Implement line breaking here!
		  U16 vlen  =  strlen( (char*)rover->zval );
		  if ( vlen+li < 32767 )
            strcpy( line+li,(char*)rover->zval );
		  else
		    TCI_ASSERT(0);
		}
        li  =  strlen( line );
        //if ( rover->zval ) {
		//  (*renderfunc)( 2,(char*)rover->zval );
        //}
	  }
	  break;

      case TT_A_ELEM_ENDER	: {
	    //line[li++]  =  ' ';		no space before tailtag
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
        //(*renderfunc)( 3,(char*)rover->zval );
  	  }
  	  break;

      case TT_S_ELEM_ENDER	: {
	    if ( li ) {
	      if ( !first_tile ) {		// output current line
    	    if ( eoln_seq ) strcat( line,eoln_seq );
            dest_lineno++;
            (*renderfunc)( 4,line );
		  }
		  curr_indent -=  indent_increment;
		  li =  0;				  	// indent next line
		  while ( li<curr_indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
	  }
	  break;

      case TT_ALIGNGROUP	: {
	    if ( li && !first_tile ) {		// output current line
    	  if ( eoln_seq ) strcat( line,eoln_seq );
          dest_lineno++;
          (*renderfunc)( 5,line );
		  li =  0;				  		// indent next line
		  while ( li<curr_indent ) line[li++]  =  ' ';
		}
        strcpy( line+li,(char*)rover->zval );
        li  =  strlen( line );
  	  }
	  break;

	  default :
	    TCI_ASSERT(0);
	  break;
    }

    first_tile  =  FALSE;
    rover =  rover->next;
  }

  if ( li ) {
    if ( eoln_seq ) strcat( line,eoln_seq );
    dest_lineno++;
    (*renderfunc)( 5,line );
  }
  delete line;

}


void Filter::XMLTreeToDstfile( TNODE* xml_tree,
								U8* container_element,
							   	U16& indent,FILE* dstfile,
								U32& dest_file_lineno ) {

  //if ( !xml_tree ) return;

  TILE* starter  =  NULL;
  TILE* ender    =  NULL;
  if ( container_element ) {
    TCI_BOOL start_tag  =  TRUE;
    starter =  xml_tiler->XMLTagToTile( container_element,
    									start_tag,&ender );
  }

  TILE* tiles  =  xml_tiler->XMLTreeToTiles( xml_tree );

//DumpTiles( tiles );

  if ( starter ) {
	starter->next =  tiles;
	if ( tiles )
	  tiles->prev =  starter;
	tiles =  starter;
  }
  if ( ender ) {
    TILE* rover =  tiles;
	while ( rover->next )
	  rover =  rover->next;
	rover->next =  ender;
	ender->prev =  rover;
  }

//DumpTiles( tiles );

  char* eoln  =  "\n";
  U8* outstr  =  OutputXML( tiles,dstfile,eoln,
  								70,indent,dest_file_lineno );

  DisposeTILEs( (TILE*)tiles );
}


void Filter::XMLTreeToRenderer( TNODE* xml_tree,
  								  ANOMALY_REC* anomaly_list,
								    U8* src_LaTeX,
									  FILTRENDERTILE renderfunc ) {

  TILE* tiles  =  xml_tiler->XMLTreeToTiles( xml_tree );

  U16 start_indent      =  0;
  U16 indent_increment  =  2;
  char* eoln_seq        =  NULL;
  RenderXML( tiles,start_indent,indent_increment,
				  eoln_seq,anomaly_list,src_LaTeX,renderfunc );

  DisposeTILEs( (TILE*)tiles );
}


void Filter::XMLTagToDstfile( U8* element_tag,TCI_BOOL start_tag,
								U16& indent,FILE* dstfile,
									U32& dest_file_lineno ) {

  TILE* starter  =  NULL;
  TILE* ender    =  NULL;

  starter  =  xml_tiler->XMLTagToTile( element_tag,
  								start_tag,&ender );

  char* eoln  =  "\n";

  U8* outstr;
  if ( start_tag && starter )
    outstr  =  OutputXML( starter,dstfile,eoln,
  								70,indent,dest_file_lineno );
  if ( !start_tag && ender )
    outstr  =  OutputXML( ender,dstfile,eoln,
  								70,indent,dest_file_lineno );
  if ( starter )
    DisposeTILEs( (TILE*)starter );
  if ( ender )
    DisposeTILEs( (TILE*)ender );
}


////////////////////////////////////////////////////////////////////


void Filter::HTMLPrefaceToDstfile( FILE* dstfile,
									U32& dest_file_lineno ) {

char* pre_lines[] = {
"  <!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"\n",
"           \"http://www.w3.org/TR/REC-html40/loose.dtd\">\n",
"<HTML>\n",
"<META NAME=\"GENERATOR\" CONTENT=\"TtM Unregistered 2.58\">\n",
"<head>\n",
"<title> Sample SWP MathML Output</title>\n",
"\n",
"</head>\n",
"<body>\n",
"<xml:namespace prefix=\"m\" />\n",
"<style> m\\:math {behavior: url(mml.htc)} </style>\n",
"<script for=\"webeq\" event=\"webeqResize(lbl)\" src=\"webeqResize.js\"></script>\n",
NULL
};

  U16 i =  0;
  while ( pre_lines[i] ) {
    dest_file_lineno++;
    fputs( pre_lines[i],dstfile );
	i++;
  }

}


void Filter::HTMLEpilogToDstfile( FILE* dstfile,
									U32& dest_file_lineno ) {

char* post_lines[] = {
"</body>\n",
"</HTML>\n",
NULL
};

  U16 i =  0;
  while ( post_lines[i] ) {
    dest_file_lineno++;
    fputs( post_lines[i],dstfile );
	i++;
  }

}

*/
