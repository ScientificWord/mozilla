
#ifndef TeXParser_h
#define TeXParser_h
																																																								
/*
  Class TeXParser  -  This object manages the parsing of source LaTeX.
It produces a parse tree, composed of TNODEs.  Note that this object
breaks the source (file) into "batches".  Currently, TeXParser ends
a batch when a paragraph end is encountered and the context stack
is empty.
  The basic action of the TeXParser is to use Grammar productions and

a stream of tokens (TNODEs) to build a parse tree.
*/

#include "flttypes.h"
#include "fltutils.h"

#define CONTEXT_STACK_LIM      16

// struct to hold grammar info for an instance of a LIST production

typedef struct tagLIST_INFO {
  U16   count;
  U16   s_off;
  U16   e_off;
  char* c_lit;
  U16   c_lit_ln;
  char* e_lit;
  U16   e_lit_ln;
} LIST_INFO;

#define LIST_NEST_LIM   4

typedef struct tagCONTEXT_STACK_REC {
  U8  c_name[ CONTEXT_NOM_LIM ];
  U8* c_start_token;
  U8* c_end_token;
  U8* c_error_stops;
  U16 inputmode;
} CONTEXT_STACK_REC;

typedef struct tagTOKEN_INFO {
  tagTOKEN_INFO* next;
  U8*    ztoken;
  U8*    ztuID;
} TOKEN_INFO;


class Grammar;
class Tokizer;
class LogFiler;
class TmplIterater;

class TeXParser {

public:
  TeXParser( Grammar* the_grammar,Tokizer* s_tokizer,
  						FILE* src,LogFiler* the_logger );
  ~TeXParser();

  TNODE* ParsePreamble( U32& preamble_len,U16& error_code );
  TNODE* Parse( U8* context,U8* src_ba,TCI_BOOL is_clipbrd );
  TNODE* ParseBatch( U32& start_off,U16& result );
  U8*    GetSrcPtr( U32 offset );

private:
  TNODE*      RunToList( U32 beg_off,U32& end_off,U16 max_tokens,
                            U32& bytesdone,TCI_BOOL exclude_ender,
                                U16& error_flag,TCI_BOOL is_primary );
  TNODE*      TemplateToList( U8* ztemplate,U16 tmpl_len,
                                U32 src_off,
                                    U32& bytesdone,U16& error_flag );

  LIST_INFO*  MakeListInfo( U8* ztemplate,U16 tmpl_off,
  												U16 tmpl_atom_len );
  I16         LocateOption( U8* opt_list,U16 opt_list_len,
                                        U8* rover,I16& option_len );

  void        SyntaxError();

  void        PushContext( U8* new_env,U8* start_toks,
                             U8* end_toks,U8* error_stops,
                               U16 inputmode );
  void        PopContext();

  TNODE*      AddToEndOfList( TNODE* head,TNODE* newnode );
  TCI_BOOL    ProcessTemplateLiteral( U8* lit_ptr,U16 lit_len,
                                     TNODE* tnode,U8* zcurr_env );
  TNODE*      ProcessTemplateList( TmplIterater* tmpl,
                                      U32 src_off,
                                        U32& bytesdone,U16& error_flag );

  TCI_BOOL    TokenMatchesuID( U8* opt_ptr,U16 opt_len,
  									TNODE* tnode,U8* zcurr_env );

  TOKEN_INFO* MakeTokenList( U8* token_names,U8* zcurr_env );
  void        ClearTokenList( TOKEN_INFO* list );
  TCI_BOOL    IsTokenInList( U8* ztok_uID,TOKEN_INFO* list );

  TNODE*      DoReqParam( TmplIterater* tmpl,U8* context,
							U32& src_off,U16& error_flag );
  TNODE*      DoOptParam( TmplIterater* tmpl,U8* context,
								U32& src_off,U16& error_flag );
  TNODE*      DoNonTeXBucket( TmplIterater* tmpl,U8* context,
								U8* nest_toks,U8* end_toks,
								TCI_BOOL include_endtok,
								U32& src_off,U16& error_flag );

  void        LinkContentsToParent( TNODE* parent,TNODE* contents,
												U8* context );
  U8*         FixNonLaTeX( U8* raw );
  TNODE*      TrimWhite( TNODE* rlist );

  CONTEXT_STACK_REC context_stack[CONTEXT_STACK_LIM];
  U16               context_sp;
  TCI_BOOL          is_clipboard;

  Grammar*      s_grammar;
  Tokizer*      tokizer;
  LogFiler* 	logfiler;
};

#endif

