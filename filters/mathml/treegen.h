
#ifndef TreeGenerator_h
#define TreeGenerator_h
																																																								
/*
  Class TreeGenerator - This is the main implementation object in the filter.

  TreeGenerator is constructed using a source grammar file spec, Notebook.gmr,
  and one or more destination grammar file specs, like MathType.gmr.
  This object constructs other objects that parse the input into a tree
  and generate the output, also in tree form.
  
  DLLs, like mtfDLL.cpp, construct an instance of this object.
  Test jigs, like MTFLaTeX.cpp, construct an instance of this object.

treeGen   =  new TreeGenerator( 2, fsNBgmr, NULL, fsMTFgmr );


treeGen->Reset( &userprefs,NULL,NULL );
treeGen->OnStartNewFile( do_xml,entity_mode,result,context_zstrs,context_ints );

Filter* filter  =  TCI_NEW( Filter(treeGen) );
filter->FInitialize( &userprefs, log, (const char*)n_space, NULL );

U16 parse_result  =  filter->TranslateBuffer( latex, renderfunc );


Note: This is a stripped down version of TreeGenerator.

  The "TranslateFile" function has been removed.
  Hence, preamble processing is not needed.

*/

#include "fltutils.h"

class LogFiler;
class Grammar;
class Tokizer;
class TeXParser;
class NewObjectStore;
class OutTreeGenerator;


class TreeGenerator {

public  :

  TreeGenerator( U16 output_ilk,
                 const char* notebook_grammar,
                 const char* XML_gmr_fspec,
                 const char* MATH_gmr_fspec );
   
  ~TreeGenerator();

  Grammar*  GetMathGrammar( bool get_src );

  void      Reset( USERPREFS* userprefs,
  					       LogFiler* new_logfiler,
							     char* dump_file_spec );

  void      SetNewFile( const char* new_srcfile_spec,
					        int do_xml,U16 e_mode,U16& result,
							const char** context_zstrs,
							int* context_ints );
  void      OnStartNewFile( int do_xml,U16 e_mode,U16& result,
							const char** context_zstrs,
							int* context_ints );

  TNODE*    ParsePreamble( U16& result );
  TNODE*    ParseBatch( U16& result,U16& nlog_entries );
  TNODE*    ParseBytes( U8* context,U8* src_ba,
  						            TCI_BOOL is_clipbrd,
                          	U16& nlog_msgs );

  void*     BufferToDestTree( U8* src_ba,TCI_BOOL is_clipbrd,
                                U16& nlog_msgs,
                                ANOMALY_REC* anomalies,
                                U8& eqn_option );
  void      ClearTree( TNODE* tree );


private :

  void      ProcessPreamble( TNODE* tlist );
  void      ProcessNewTheorem( TNODE* nt_node );
  void      ProcessNewCommand( TNODE* nt_node );
  void      ProcessNewMathOp( TNODE* nt_node,bool is_starred );
  void      ProcessNewEnvironment( TNODE* nt_node );

  TCI_BOOL  AppendSrc( U32 src_off,char* srcstr,TeXParser* parser );

  void*     ConvertParseTree( TNODE* nb_parse_tree,
                                TCI_BOOL is_preamble,
                                ANOMALY_REC* anomalies,
                                U8& eqn_option );

  USERPREFS*        p_userprefs;
  LogFiler*         logfiler;

  Grammar*          s_tex_grammar;
  Tokizer*			    tokizer;
  NewObjectStore*   theorem_store;

  TeXParser*        TeXparser;
  FILE*             srcfile;
  U32               curr_src_off;

  Grammar*  d_xml_grammar;
  Grammar*  s_math_grammar;
  Grammar*  d_math_grammar;

  const char*     eoln;
  U32       batch_count;

  int       do_xml;
  U16       entity_mode;

  U16       output_type;
  OutTreeGenerator* pLaTeX2MATH;
};

#endif

