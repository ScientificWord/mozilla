
#ifndef Tokizer_h
#define Tokizer_h
																																																								
/*
  Class Tokizer  - 
  For each filtering task the source, generally a file, is passed
to the source instance of this object.  During parsing, tokens
are read from the source and passed out as TNODEs.
*/

#include "chmtypes.h"
#include "fltutils.h"


typedef struct tagLINENUM_REC {
  tagLINENUM_REC*  next;
  U32 offset;
  U32 line_num;
} LINENUM_REC;

class TeXtokenizer;
class MacroStore;
class NewObjectStore;
class Grammar;

class Tokizer {

public:
  Tokizer( Grammar* TeX_grammar,NewObjectStore* obj_store,
  												FILE* srcfile );
  ~Tokizer();

  void        SaveInputBuffer();
  void          SetSrcBuffer( U8* z_src );
  void        RestoreInputBuffer();

  void          ClearInputBuffer( U32 src_off );
  TCI_BOOL      ReplenishInputBuffer( U32 src_off );
  TCI_BOOL      IsEOF() { return is_eof; }

  U8*           GetSrcPtr( U32 offset );
  U32           GetStartOffset() { return buf_start_offset; }


  TNODE*        GetTNODE( U8* zcurr_env,U32 src_off,U8** ztmpl );
  TNODE*        GetContextNode( U8* zcontext_nom,TCI_BOOL is_start );

  TCI_BOOL      HasEnvs();


  U8*           GetBufferedLine( U32 token_offset,
                                    U32& line_start_off,U32& src_lineno );

  void          AddMacro( U8* nom,U8* def,U16 p_count,U16 scope );
  void          AddMathOp( U8* nom,U8* def,bool is_starred,U16 scope );
  void          AddNewTheorem( U16 scope,U8* nom,U8* def,
  								U8* n_like,U8* in,TCI_BOOL isstarred );
  void          AddNewEnviron( U16 scope,U8* env_nom,U8* n_args,
								U8* opt_def,U8* begdef,U8* enddef );
  TCI_BOOL      HasPreamble() { return has_preamble; }

  U16           GetTokenError() { return error_flag; }

  void          SetObjectStore( NewObjectStore* new_object_store );

  U16           GetTokenizerID() { return tokenizerID; }

  U16           SetInputMode( U16 new_mode );

  TNODE*        GetComments();

  void          SetTheoremStyleAttr( U16 which_attr,U8* new_val );
  void          EnterExitGroup( TCI_BOOL do_enter );

  TCI_BOOL      GGDFromNameAndAttrs( U8* token,U16 ln,
										U8* zform,
                                          U8* zcurr_env,
                                            U8** d_zuID,
                                              U8** d_ztemplate );

private:

  void         LocateOffsets( char* fline,I16* offsets );

  void         GetContextNom( char* nom,char* dest,
                               U16& the_usubtype,U16& the_uID );
  void         AddContext( char* new_env_nom,char* env_list,
                               U16 the_usubtype,U16 the_uID );
  void         RemoveContextName( char* del_nom,char* env_list );

  TCI_BOOL     IsSubset( char* env_list_subset,char* env_list_superset );

  I16           GetObjID( U16 curr_env,U8* obj_ptr,U16 token_len );


  U16           GetTmplLen( U8* src );

  U32           LoadSrcLines( TCI_BOOL is_latex,U32 line_limit );
  TCI_BOOL      IsLaTeXFile( FILE* sf );
  U32           GetLineNum( U32 src_offset );

  TNODE*        GetToken( U8* zcurr_env,U32 src_off,
  							U8* ptr,U8* end_ptr,U8** ztmpl,
                               	TCI_BOOL in_comment );

  void          MacroReplace( U32 macro_start_off,U32 macro_slen,
                                                        U8* instance );

  TCI_BOOL      LocateMacroParams( U8* pENV,U32 src_off,U16 n_params,
                                             U32* locs,U32& param_slen );

  U8*           BuildMacroInstance( U8* macro_def,U16 n_params,U32* locs );

  TCI_BOOL      GetObjectDataFromName( U8* zcurr_env,
  										U8* token,U16 ln,
                                         	U8** d_zuID,
                                            U8** d_ztemplate );
  ATTRIB_REC*   MakeATTRIBList( U8* src,U16 ln );
  ATTRIB_REC*   GetNextATTRIB( U8* src,U16 ln,U16& bytesdone );

  TCI_BOOL 	    AtomicElement( U8* zuID );
  U8*           GetVarValue( U8* tok_ptr,U16 lim,U16& vln );

  TCI_BOOL      IsFormMatch( U8* req_form,U8* obj_def );

  void          AppendComments( TNODE* new_comment );

  U8*           GetTeXMetricuID( U8* ptr,U16 tln,U8** ztmpl );
  U8*           MakeMathNomTmpl( U8* token,U8* visual_def,
  											bool is_starred );

  Grammar*      s_grammar;

  U16           tokenizerID;      // records the ID of the "tokenizer" to use
  TeXtokenizer* tokenizer;

  FILE*       srcfile;
  TCI_BOOL      is_latex;
  TCI_BOOL      has_preamble;
  TCI_BOOL      is_eof;

  U8*         buf_ptr;
  U32           buf_start_offset;
  U32           buf_lim;
  U32           curr_line_num;
  LINENUM_REC*  linenum_list;
  LINENUM_REC*  linenum_last;
  U16           input_mode;		  // standard, verbatim, etc.
  TNODE*        comment_buffer;

  TCI_BOOL    BA_input;

  U8*         sv_buf_ptr;
  U32           sv_buf_start_offset;
  U32           sv_buf_lim;
  U32           sv_curr_line_num;
  LINENUM_REC*  sv_linenum_list;
  LINENUM_REC*  sv_linenum_last;
  U16           sv_input_mode;		  // standard, verbatim, etc.
  TNODE*        sv_comment_buffer;


  MacroStore*      macro_store;
  NewObjectStore*  nobject_store;
  U16           error_flag;

};

#endif

