
#ifndef MMLTiler_h
#define MMLTiler_h
																																																								
/*
  Class MMLTiler  - Given an MML parse tree, this object translates
the tree into a list of "tiles" that represent the contents
of the parse tree in the destination grammar.  These tiles
are used as input to line and page breaking functions.
*/

#include "flttypes.h"
#include "fltutils.h"

class Grammar;
class LogFiler;

#define CONTEXT_STACK_LIM   32

class MMLTiler {

public:
  MMLTiler( Grammar* dest_grammar );
  ~MMLTiler();

  void      SetLogFiler( LogFiler* lf );
  void      SetNameSpace( U8* name_space );
  void      SetEntityMode( U8* entity_mode );

  TILE*     MMLtreeToTiles( TNODE* parse_tree,U16 lim );

private:
  void      PushContext( U8* new_env );
  void      PopContext();

  TILE*     ListToTiles( TNODE* parse_tree,U16 lim,
  											U16& error_code );
  TILE*     StructuredObjectToTiles( U8* templ,U16 templ_len,
                                      	TNODE* parse_tree,
                                      		U16& error_code );

  TILE*     TokenToTile( TNODE* src_node,U8* grammar_info,
											U16 token_type );
  TILE*     AtomToTile( TNODE* src_node,U8* grammar_info,
											U16 token_type );
  TILE*     BytesToTile( U8* zbytes,U16 zln,U16 token_type );

  TILE*     AtomicElementToTiles( TNODE* mml_atom_elem_node,
  									U8* dest_zname,U16& error_code );
  TILE*     EntityToTiles( TNODE* mml_entity_node,
  									U8* dest_zname,U16& error_code );

  void      AttrsToHeader(  ATTRIB_REC* attrib_list,U8* zheader );

  TILE*     StartEndStructuredElement( TILE* body,
										U8* elem_name,
										  TNODE* mml_src_node );


  Grammar*  d_mml_grammar;
  LogFiler* logfiler;

  U8 n_space[32];
  TCI_BOOL  use_unicodes;

  U8*       context_stack[CONTEXT_STACK_LIM];
  U16       context_sp;
};

#endif

