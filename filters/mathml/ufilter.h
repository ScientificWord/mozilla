#ifndef MMLFilter_h
#define MMLFilter_h
																																																								
/*
  The object that constructs an MMLFilter first creates a TreeGenerator.

  It is the TreeGenerator that uses gmr files to build a source
  parse tree, and, from that a destination parse tree.

  After the dest parse tree is generated, it is passed to this object.
  A tiler creates a linear list of tiles from the tree.
  Finally this list goes to the output routine.

  Note that this particular Filter handles only the math runs
  from a LaTeX document.
*/

//#include <windows.h>
#include "fltutils.h"


class LogFiler;
class TreeGenerator;
class MMLTiler;

typedef void CALLBACK;

typedef bool (* FILTRENDERTILE)(U32 type, const char* data);


class MMLFilter {

public:
  MMLFilter( TreeGenerator* lparser );
  ~MMLFilter();

  void  FInitialize( USERPREFS* userprefs,
  						const char* new_log_file,
  						char* name_space,
						U16 starting_indent,
						char* dump_file_spec );

  U16   TranslateFile( const char* srcfile_spec,
                        char* dstfile_spec,
                            char* srcstr,char* eoln );

  U16   TranslateBuffer( const char* srcbuffer, 
                            FILTRENDERTILE renderfunc );


private:

  void  MMLTreeToRenderer( TNODE* xml_tree,ANOMALY_REC* anomalies,
	   						U8* src_LaTeX,FILTRENDERTILE renderfunc );

  void  RenderMML( TILE* tiles,U16 start_indent,
					        U16 indent_increment,char* eoln_seq,
						    ANOMALY_REC* anomalies,U8* src_LaTeX,
					        FILTRENDERTILE renderfunc );


  void  MMLTreeToDstfile( TNODE* xml_tree,
                            FILE* dstfile,
                            U16 indent_increment,
				   	        char* eoln_seq );

  LogFiler*       logfiler;
  TreeGenerator*  converter;
  MMLTiler*       mml_tiler;
  U16             start_indent;
  U16             indent_inc;
};

#endif

