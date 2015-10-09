
#ifndef TeXtokenizer_h
#define TeXtokenizer_h

#include "flttypes.h"

/* This object is constructed by a "SciNotebook grammar object".
  It's main function is to locate LaTeX tokens at some point
  in a buffer that is passed in by the Grammar that owns it.

  Note that LaTeX comments are NOT made into tokens - they are
  thrown away by this tokenizer. (The line end sequence at end
  of a LaTeX comment is part of the comment, not a semantically 
  relevent white space token.)

  The most complicated aspect of this tokenizer is the handling
  of white space after tokens in TEXT mode.  Sometimes this white
  space denotes a token, sometimes it just terminates the previous
  token.  When white space after a TEXT token is known to act only
  as a terminator for the previous token, it is referred to as
  "owned space" and this tokenizer does not report it as a token.
  A circular list of the locations of "owned spaces" is kept by
  this object.  This list allows the tokenizer to look ahead or
  back a short distance for tokens.  However, it prevents the
  tokenizer from being used for long look aheads - which are not
  necessary for parsing LaTeX.
*/

#define OWNED_SPACE_LIMIT     8

class Tokizer;

class TeXtokenizer {

public:

  TeXtokenizer( Tokizer* the_tokenizer );
  ~TeXtokenizer();

  // Functions for Tokizer's - used in parsing

  U16       LocateToken( U8* zcurr_env,U32 src_off,
                          U8* src,U8* end_ptr,
                           U16& s_off,U16& e_off,
                            TCI_BOOL& is_word,
                             TCI_BOOL& is_white,
                               TCI_BOOL& is_comment,
                                TCI_BOOL& is_par,
                                 TCI_BOOL& is_chdata,
                                  TCI_BOOL in_comment );
  void      SetPrevTokenOwnsSpace( U32 src_off,TCI_BOOL new_val );


  // Function to allow handling of another run of input

  void      Reset();
  void      SetInputMode( U16 new_mode ) { input_mode =  new_mode; }

private:
  I16       GetTokenLen( U8* zcurr_env,U8* ptr,U8* end_ptr );
  TCI_BOOL  IsOwnedSpace( U32 src_off );

  TCI_BOOL  IsStarForm( U8* tok_ptr,U16 token_len );

  U16 LocateSpecialToken( U8* src,U8* end_ptr,
							U16& s_off,U16& e_off,
	  						  TCI_BOOL& is_comment );
  U16 LocateSpecStrToken( U8* src,U8* end_ptr,
	  						U16& s_off,U16& e_off,
							  TCI_BOOL& is_chdata,
								TCI_BOOL& is_comment );
  U16 LocateNumberToken( U8* src,U8* end_ptr,
	 						U16& s_off,U16& e_off,
							  TCI_BOOL& is_chdata,
								TCI_BOOL& is_comment );
  U16 LocateBooleanToken( U8* src,U8* end_ptr,
	 						U16& s_off,U16& e_off,
								TCI_BOOL& is_comment );
  U16 LocateLBraceToken( U8* src,U8* end_ptr,
	 						U16& s_off,U16& e_off,
							  TCI_BOOL& is_chdata,
								TCI_BOOL& is_comment );
  U16 LocateDFloatToken( U8* src,U8* end_ptr,
	 						U16& s_off,U16& e_off,
							  TCI_BOOL& is_chdata,
								TCI_BOOL& is_comment );
  U16 LocateHyphenationToken( U8* src,U8* end_ptr,
      							U16& s_off,U16& e_off,
                       		  	  TCI_BOOL& is_chdata,
                           	    	TCI_BOOL& is_comment );
  U16 LocatePreambleToken( U8* src,U8* end_ptr,
	 						U16& s_off,U16& e_off,
								TCI_BOOL& is_comment );
  U16 LocateGlueToken( U8* src,U8* end_ptr,
	 						U16& s_off,U16& e_off,
							  TCI_BOOL& is_chdata,
								TCI_BOOL& is_comment );
  U16 LocateDimenToken( U8* src,U8* end_ptr,
	 						U16& s_off,U16& e_off,
							  TCI_BOOL& is_chdata,
								TCI_BOOL& is_comment );
  U16 LocateVerbatimToken( U8* src,U8* end_ptr,
	  						U32 src_off,
      							U16& s_off,U16& e_off,
      								TCI_BOOL& is_chdata );
  U16 LocateNonLaTeXToken( U8* src,U8* end_ptr,
      						U16& s_off,U16& e_off,
							  TCI_BOOL& is_chdata,
                           	    TCI_BOOL& is_comment );

  U16 LocatePassthruToken( U8* src,U8* end_ptr,
      						U16& s_off,U16& e_off,
							  TCI_BOOL& is_chdata,
                           	    TCI_BOOL& is_comment );


  U16 ParseTeXUnit( U8* p_unit );

  U8        last_token_output[64];  
  U32       owned_space_locs[OWNED_SPACE_LIMIT];
  U16       si_index;

  Tokizer*  tokenizer;
  U16       input_mode;
};

#endif
