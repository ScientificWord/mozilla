
#include "tokizer.h"
#include "textoken.h"
#include "fltutils.h"

#include <string.h>
#include <stdlib.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/*	This tokenizer was generalized in Dec. 1998, to handle specialized
tokenizing within various LaTeX scopes - for example, in comments,
verbatims, the argument of \special{}, etc.
  Basically, TeXtokenizer now has a SetInputMode function.
When Parser goes into a new scope (calls Parser::PushContext),
Tokizer::SetInputMode is called.  It calls Tokenizer::SetInputMode.
Each mode has it's own specialized "LocateToken" function.
*/

TeXtokenizer::TeXtokenizer( Tokizer* the_tokenizer ) {
//JBMLine( "TeXtokenizer::ctor\n" );

  tokenizer   =  the_tokenizer;
  input_mode  =  IM_STANDARD;
  last_token_output[0]  =  0;  

  U16 si  =  0;                         // intialize the array
  while ( si < OWNED_SPACE_LIMIT ) {    // that holds the "positions"
    owned_space_locs[si]  =  0L;        // of owned spaces
    si++;
  }

  si_index  =  0;
}


TeXtokenizer::~TeXtokenizer() {
//JBMLine( "TeXtokenizer::dtor\n" );
}

// Function to span the next token in the buffer - zcurr_env is not used.
// "end_ptr" points to the first byte beyond the data being processed.
// Cases handled:
//   \name              \ followed by a run of letters  rv = len
//   \~                 \ followed by a non-letter      rv = 2
//   [                  everything else                 rv = 1

// Used only in "LocateToken"

I16 TeXtokenizer::GetTokenLen( U8* zcurr_env,
									U8* tok_ptr,U8* end_ptr ) {

  U16 token_len =  1;

  switch ( *tok_ptr ) {

    case '\\' :  {
      char* ptr =  (char*)tok_ptr+1;
      if ( ( *ptr>='A' && *ptr<='Z' ) || ( *ptr>='a' && *ptr<='z' ) ) {
        while ( ( *ptr>='A' && *ptr<='Z' ) || ( *ptr>='a' && *ptr<='z' ) ) {
          token_len++;
          ptr++;
        }

        if ( *ptr == '*' ) {
		  if ( IsStarForm(tok_ptr,token_len) )
		    token_len++;
        
        } else if ( token_len == 5 ) {
          if ( !strncmp((char*)tok_ptr,"\\char",5) ) {
            while ( *ptr>='0' && *ptr<='9' ) {
              token_len++;
              ptr++;
            }
          }
        }

      } else {
        token_len =  2;
        if ( *ptr == '\\' && *(ptr+1) == '*' )
          token_len++;
      }
    }
    break;

    case '{'  :  {
      if ( *(tok_ptr+1) == '}' )
        token_len =  2;
    }
    break;

    case '$'  :  {
      if ( *(tok_ptr+1) == '$' )
        token_len =  2;
    }
    break;

    default :  break;
  }

  TCI_ASSERT( token_len <= (end_ptr - tok_ptr) );

  if ( token_len <= (end_ptr - tok_ptr) )
    return token_len;
  else
    return 0;
}


// We've encountered something like "\blah*".  Function to decide
// if this matches a known LaTeX * command.

TCI_BOOL TeXtokenizer::IsStarForm( U8* tok_ptr,U16 token_len ) {

  TCI_BOOL rv =  FALSE;
  switch ( token_len ) {
    case 4  : {
      if      ( !strncmp((char*)tok_ptr,"\\tag",4) )
        rv  =  TRUE;
	}
	break;
    case 5  : {
      if      ( !strncmp((char*)tok_ptr,"\\part",5) )
        rv  =  TRUE;
	}
	break;
    case 7  : {
      if      ( !strncmp((char*)tok_ptr,"\\hspace",7) )
        rv  =  TRUE;
      else if ( !strncmp((char*)tok_ptr,"\\vspace",7) )
        rv  =  TRUE;
// \TCItag*<uID5.702.0>!\TCItag!REQPARAM(5.702.1,TEXT)
      else if ( !strncmp((char*)tok_ptr,"\\TCItag",7) )
        rv  =  TRUE;
	}
	break;
    case 8  : {
      if      ( !strncmp((char*)tok_ptr,"\\chapter",8) )
        rv  =  TRUE;
      else if ( !strncmp((char*)tok_ptr,"\\mathnom",8) )
        rv  =  TRUE;
      else if ( !strncmp((char*)tok_ptr,"\\section",8) )
        rv  =  TRUE;
	}
	break;
    case 10 : {
      if      ( !strncmp((char*)tok_ptr,"\\paragraph",10) )
        rv  =  TRUE;
	}
	break;
    case 11 : {
      if      ( !strncmp((char*)tok_ptr,"\\subsection",11) )
        rv  =  TRUE;
      else if ( !strncmp((char*)tok_ptr,"\\newtheorem",11) )
        rv  =  TRUE;
	}
	break;
    case 13 : {
      if      ( !strncmp((char*)tok_ptr,"\\subparagraph",13) )
        rv  =  TRUE;
      else if ( !strncmp((char*)tok_ptr,"\\operatorname",13) )
        rv  =  TRUE;
	}
	break;
    case 14 : {
      if      ( !strncmp((char*)tok_ptr,"\\subsubsection",14) )
        rv  =  TRUE;
	}
	break;
    case 16 : {
      if      ( !strncmp((char*)tok_ptr,"\\enlargethispage",16) )
        rv  =  TRUE;
      else if ( !strncmp((char*)tok_ptr,"\\includegraphics",16) )
        rv  =  TRUE;
	}
	break;
    case 20 : {
      if      ( !strncmp((char*)tok_ptr,"\\DeclareMathOperator",20) )
        rv  =  TRUE;
	}
	break;
    default :  break;
  }			// switch on token length

  return rv;
}


// Locate the token that starts at "src".
//  end_ptr points to the first byte beyond the data being processed.
// Return values are:
//	0 - hit end of data in the input buffer.
//  1 - hit bytes that don't consistute a token.
//  2 - found a valid token.

U16 TeXtokenizer::LocateToken( U8* zcurr_env,U32 src_off,
                                U8* src,U8* end_ptr,
                                 U16& s_off,U16& e_off,
                                  TCI_BOOL& is_word,
                                   TCI_BOOL& is_white,
                                     TCI_BOOL& is_comment,
                                	  TCI_BOOL& is_par,
                                	   TCI_BOOL& is_chdata,
                                	    TCI_BOOL in_comment ) {
/*
char vvv[80];
U16 i =  0;
U8* p =  src;
while ( i<20 && p<end_ptr ) {
  if ( *p == '\n' )
    vvv[i] =  ' ';
  else
    vvv[i] =  *p;
  i++;
  p++;
}
vvv[i] =  0;
char zzz[128];
sprintf( zzz,"LocateToken, mode=%d, src=%s\n",input_mode,vvv );
JBMLine( zzz );
*/

  if ( src >= end_ptr ) return 0;	// assumed end of data

  U16 rv    =  0;

  U8 save   =  *end_ptr;        // temporary 0 terminator for string funcs
  *end_ptr  =  0;

  s_off       =  0;             // these are  out only
  e_off       =  0;   			//   relative to "src".
  is_word     =  FALSE;
  is_white    =  FALSE;
  is_comment  =  FALSE;
  is_par      =  FALSE;
  is_chdata   =  FALSE;

// depending on input_mode, we select a token-locating function

  TCI_BOOL done =  TRUE;
  switch ( input_mode ) {
    case IM_PREAMBLE  :
      rv  =  LocatePreambleToken( src,end_ptr,s_off,e_off,
      								is_comment );
    break;
    case IM_NONLATEX  :
      rv  =  LocateNonLaTeXToken( src,end_ptr,s_off,e_off,
      								is_chdata,is_comment );
    break;
    case IM_SPECIAL   :
      rv  =  LocateSpecialToken( src,end_ptr,s_off,e_off,
      								is_comment );
    break;
    case IM_SPECSTR   :
      rv  =  LocateSpecStrToken( src,end_ptr,s_off,e_off,
      								is_chdata,is_comment );
    break;
    case IM_NUMBER    :
      rv  =  LocateNumberToken( src,end_ptr,s_off,e_off,
      								is_chdata,is_comment );
    break;
    case IM_BOOLEAN   :
      rv  =  LocateBooleanToken( src,end_ptr,s_off,e_off,
      								is_comment );
    break;
    case IM_LBRACE    :
      rv  =  LocateLBraceToken( src,end_ptr,s_off,e_off,
      								is_chdata,is_comment );
    break;
    case IM_DFLOAT    :
      rv  =  LocateDFloatToken( src,end_ptr,s_off,e_off,
      								is_chdata,is_comment );
    break;
    case IM_HYPHENATION  :
      rv  =  LocateHyphenationToken( src,end_ptr,s_off,e_off,
      								is_chdata,is_comment );
    break;
    case IM_TEXDIMEN  :
      rv  =  LocateDimenToken( src,end_ptr,s_off,e_off,
      								is_chdata,is_comment );
    break;
    case IM_TEXGLUE  :
      rv  =  LocateGlueToken( src,end_ptr,s_off,e_off,
      								is_chdata,is_comment );
    break;
    case IM_VERBATIM  :
      rv  =  LocateVerbatimToken( src,end_ptr,src_off,s_off,e_off,
      								is_chdata );
    break;
    case IM_PASSTHRU  :
      rv  =  LocatePassthruToken( src, end_ptr, s_off, e_off, is_chdata, is_comment);
    break;
	default :
      done  =  FALSE;
    break;
  }		// switch( input_mode)

  if ( done ) {
    *end_ptr  =  save;
/*
char zzz[128];
sprintf( zzz,"LocateToken returns, mode=%d, tlen=%d,off=%lu\n",
									input_mode,e_off-s_off,src_off );
JBMLine( zzz );
*/
	return rv;
  }


  TCI_BOOL in_TEXT  =  FALSE;
  if ( !strcmp((char*)zcurr_env,"TEXT") )
    in_TEXT =  TRUE;

  U8* rover =  src;

// At this point we're not in VERBATIM.
// span white space - count eoln's, ie. '\n' 

  U16 eoln_tally  =  0;					// In LaTeX, the semantics of 
  U16 white_tally =  0;					//  white space is tricky.  In
  while ( *rover && *rover <= ' ' ) {	//  MATH, all white space has
	if ( *rover == '\n' ) {				//  no meaning - it is used to
      eoln_tally++;						//  end tokens or to help with
	  *rover  =  ' ';					//  readability of the LaTeX.
    }									//  In TEXT, runs of whitspace
    rover++;							//  general count for a single
    s_off++;							//  space in the output.  EOLN
    white_tally++;						//  is just another space.
  }

// In TEXT, a run of white space with more than 1 eoln indicates
//  a blank line.  We return, indicating a "\par" was found.
//  The caller, who knows more about current context, may choose
//  to discard this token.

  if ( in_TEXT && eoln_tally ) {
    if ( eoln_tally > 1 || in_comment ) {
      e_off  =  s_off;
      s_off  =  0;
      *end_ptr  =  save;
      is_par    =  TRUE;
	  return 2;
    }
  }

// We're thru the whitespace - check for end of input buffer.
  
  if ( *rover == 0 && white_tally ) {	// end of input buffer
//JBMLine( "LocateTok::restoring eoln\n" );
    *(rover-1)	=  '\n';
    s_off   =  0;
    e_off   =  0;
    *end_ptr  =  save;
    return 0;
  }

// Check for a comment - only in TEXT or MATH ( not NON_LaTeX )
// Note that we DON'T coelesce consecutive comment line here.

  if ( *rover == '%' && input_mode == IM_STANDARD ) {
    e_off =  s_off;
    while ( *rover && *rover != '\n' ) {
      rover++;
      e_off++;
    }
    if ( *rover == '\n' ) {
      rover++;
      e_off++;
    }
    *end_ptr    =  save;
    is_comment  =  TRUE;
/*
char zzz[80];
sprintf( zzz,"COMMENT, s=%d, e=%d\n",s_off,e_off );
JBMLine( zzz );
*/
	return 2;
  }

  if ( in_TEXT ) {

    TCI_BOOL prev_token_owns_space;
    if ( white_tally )
      prev_token_owns_space =  IsOwnedSpace( src_off );

    if ( white_tally && !prev_token_owns_space ) {      // white space token
      e_off     =  s_off;
      s_off     =  0;
      is_white  =  TRUE;
      rv        =  2;

//JBMLine( "Make space\n" );

    } else {          // the token is a word of text or \cmd in TEXT

      char ch =  *rover;
	  if ( ch ) {
	    if        ( input_mode == IM_STANDARD ) {
          if ( strchr("${}[]~&\\",ch) ) {        // \cmd in text
            I16 tl  =  GetTokenLen( zcurr_env,rover,end_ptr );
            e_off   =  s_off + tl;
            rv  =  2;
          } else {		// a non-\cmd - starts with a letter, a word
            e_off =  s_off;
            char ch;
            while ( ch = *rover ) {
			  if ( ch == '\n' ) {
				char* nr  =  (char*)rover;
				U16 eoln_count  =  0;
			    while ( *nr && *nr <= ' ' ) {
			      if ( *nr <= '\n' )  eoln_count++;
				  nr++;
				}

		// We may have come to the end of the input buffer here!

				if ( *nr == 0 && !tokenizer->IsEOF() ) {
                  s_off   =  0;
                  e_off   =  0;
                  *end_ptr  =  save;
                  return 0;
                }

				if ( eoln_count > 1 )
				  break;
				else
				  *rover  =  ' ';
			  }
              if ( strchr("%${}[]~&\\",ch) ) break;
              rover++;
              e_off++;
            }			// while ( ch = *rover )
            is_word =  TRUE;
            rv  =  2;
		  }		// word(s) in TEXT

		} else
		  TCI_ASSERT(0);

	  }		// if ( ch )

    }   // the token is a word of text or \cmd in text

  } else if ( *rover >= 33 && *rover <= 126 ) {       // in MATH

  // Note that all "white_space" is ignored in MATH

    I16 tl  =  GetTokenLen( zcurr_env,rover,end_ptr );
    e_off =  s_off + tl;
    rv    =  2;

  } else {    // ERROR - bad byte in MATH

  }

  *end_ptr  =  save;

  if ( rv == 2 && input_mode == IM_STANDARD ) {
    if ( in_TEXT ) {

  // In TEXT, the white space after a token may be part of the token,
  //  ie. owned space.  We decide this issue here.

      char end_ch =  src[ e_off ];
      if ( end_ch <= ' ' ) {
        TCI_BOOL this_token_owns_space =  FALSE;
        if        ( src[s_off] == '\\' ) {
          char ch =  src[ s_off+1 ];
          if      ( ch>='A' && ch<='Z' )
            this_token_owns_space =  TRUE;
          else if ( ch>='a' && ch<='z' )
            this_token_owns_space =  TRUE;
          else if ( ch=='[' || ch==']' )
            this_token_owns_space =  TRUE;

        } else if ( src[s_off] == '}' ) {
      //      this_token_owns_space =  TRUE;
        } else if ( src[s_off] == ']' ) {
          this_token_owns_space =  TRUE;
        }

        if ( this_token_owns_space )
          SetPrevTokenOwnsSpace( src_off+e_off,TRUE );
      }
    }           // in TEXT
  }     // token found

  return rv;
}



// Search the array of known "owned spaces" backwards.


TCI_BOOL TeXtokenizer::IsOwnedSpace( U32 src_off ) {

  TCI_BOOL rv  =  FALSE;

  U16 si;

  if ( si_index )
    si  =  si_index - 1;
  else
    si  =  OWNED_SPACE_LIMIT - 1;

  while ( si != si_index ) {
    if ( src_off == owned_space_locs[si] ) {
      rv    =  TRUE;
      break;
    }
    if ( si )                           // We are searching backwarks
      si--;                             // thru a circular queue,
    else                                // starting at the last value
      si  =  OWNED_SPACE_LIMIT - 1;     // that has been saved.
  }

/*
char zzz[80];
sprintf( zzz,"IsOwnedSpace (%lu,%lu,%lu) returns %d\n",
owned_space_locs[0],owned_space_locs[1],owned_space_locs[2],
rv );
JBMLine( zzz );
*/

  return rv;
}


void TeXtokenizer::SetPrevTokenOwnsSpace( U32 src_off,TCI_BOOL new_val ) {

/*
char zzz[80];
if ( new_val ) {
sprintf( zzz,"SetOwnedSpace at %lu\n",src_off );
JBMLine( zzz );
}
*/

  TCI_BOOL found =  FALSE;
  U16 si;                               // we search to see if new_val
  if ( si_index )                       // is in the owned space array
    si  =  si_index - 1;
  else
    si  =  OWNED_SPACE_LIMIT - 1;

  while ( si != si_index ) {
    if ( src_off == owned_space_locs[si] ) {
      found =  TRUE;
      break;
    }
    if ( si )                           // We are searching backwarks
      si--;                             // thru a circular queue,
    else                                // starting at the last value
      si  =  OWNED_SPACE_LIMIT - 1;     // that has been saved.
  }

  if ( !found ) {
    owned_space_locs[si_index]  =  src_off;
    si_index++;
    if ( si_index == OWNED_SPACE_LIMIT )
      si_index  =  0;
  }
}


void TeXtokenizer::Reset() {

//JBMLine( "Reset\n" );

  last_token_output[0]  =  0;  

  U16 si  =  0;                                 // intialize the array
  while ( si < OWNED_SPACE_LIMIT ) {            // that holds the "positions"
    owned_space_locs[si]  =  0L;                // of owned spaces
    si++;
  }

  si_index  =  0;
}

/*
#define SYM_DUMMY         "dummy"
#define SYM_TYPE          "type"
#define SYM_WIDTH         "width"
#define SYM_HEIGHT        "height"
#define SYM_DEPTH         "depth"
#define SYM_FUNCTION      "function"
#define SYM_RADIUS        "radius"
#define SYM_LANGUAGE      "language"
#define SYM_PHI           "phi"
#define SYM_THETA         "theta"
#define SYM_XMIN          "xmin"
#define SYM_XMAX          "xmax"
#define SYM_YMIN          "ymin"
#define SYM_YMAX          "ymax"
#define SYM_ZMIN          "zmin"
#define SYM_ZMAX          "zmax"
#define SYM_XVIEWMIN      "xviewmin"
#define SYM_XVIEWMAX      "xviewmax"
#define SYM_YVIEWMIN      "yviewmin"
#define SYM_YVIEWMAX      "yviewmax"
#define SYM_ZVIEWMIN      "zviewmin"
#define SYM_ZVIEWMAX      "zviewmax"
#define SYM_XRANGE        "xrange"
#define SYM_YRANGE        "yrange"
#define SYM_ZRANGE        "zrange"
#define SYM_VIEWSET       "viewset"
#define SYM_RANGESET      "rangeset"
#define SYM_RECOMP        "recompute"
#define SYM_XIS           "xis" 
#define SYM_YIS           "yis"
#define SYM_ZIS           "zis"
#define SYM_VAR1          "var1name" 
#define SYM_VAR2          "var2name"
#define SYM_VAR3          "var3name"
#define SYM_PLOTTYPE      "plottype"
#define SYM_PLOTTICKS     "plotticks"
#define SYM_PLOTTICKDISABLE "plottickdisable"
#define SYM_LABELOVERRIDES "labeloverrides"
#define SYM_NUMPTS        "numpoints"
#define SYM_GRID1         "num-x-gridlines"
#define SYM_GRID2         "num-y-gridlines"
#define SYM_GRID3         "num-z-gridlines"
#define SYM_TICK1         "num-x-ticks"
#define SYM_TICK2         "num-y-ticks"
#define SYM_TICK3         "num-z-ticks"
#define SYM_XLABEL        "x-label"
#define SYM_YLABEL        "y-label"
#define SYM_ZLABEL        "z-label"
#define SYM_CONFX         "numpts-xgridline"
#define SYM_CONFY         "numpts-ygridline"
#define SYM_BOXTYPE       "boxtype" 
#define SYM_NUMBOXES      "numboxes"
#define SYM_PLOTSTYLE     "plotstyle"
#define SYM_LIGHTING      "lighting"
#define SYM_AXESSTYLE     "axesstyle"
#define SYM_PLOTSHADING   "plotshading"
#define SYM_LINECOLOR     "linecolor"          
#define SYM_LINESTYLE     "linestyle"          
#define SYM_LINETHICK     "linethickness"      
#define SYM_PTSTYLE       "pointstyle"           
#define SYM_ISPTPLOT      "pointplot"           
#define SYM_CONSTRAINED   "constrained"
#define SYM_DISPLAYFLAGS  "display"
#define SYM_FITFRAME      "fit-to-frame"
#define SYM_MAINTASP      "maintain-aspect-ratio"
#define SYM_OLDSNAPSHOT   "plot-snapshots"
#define SYM_SNAPSHOT      "plot_snapshots"
#define SYM_OLDVALIDFILE  "valid-file"
#define SYM_VALIDFILE     "valid_file"
#define SYM_CROPLEFT      "cropleft"
#define SYM_CROPTOP       "croptop"
#define SYM_CROPRIGHT     "cropright"
#define SYM_CROPBOTTOM    "cropbottom"
#define SYM_RECTLEFT      "rectleft"
#define SYM_RECTTOP       "recttop"
#define SYM_RECTRIGHT     "rectright"
#define SYM_RECTBOTTOM    "rectbottom"
#define SYM_FILENAME      "filename"
#define SYM_FILEPROP      "file-properties"
#define SYM_TEMPFNAME     "tempfilename"
#define SYM_TFILEPROP     "tempfile-properties"
#define SYM_OWIDTH        "original-width"
#define SYM_OHEIGHT       "original-height"
#define SYM_OLD_ALIAS     "file-alias"
#define SYM_ALIAS         "file_alias"
#define SYM_OLDT_ALIAS    "temp-alias"
#define SYM_T_ALIAS       "temp_alias"
#define SYM_DISCONT       "discont"
#define SYM_GRAPHICSHP    "GRAPHICSHP"
#define SYM_GRAPHICSPS    "GRAPHICSPS"
#define SYM_GRFILENAME    "FILENAME"
#define SYM_LITERAL       "literal"
#define SYM_GREYSCALE     "greyscale"


#define    T_UNKNOWN       0
#define    T_STRING        1
#define    T_DIMENSION     2
#define    T_NUMBER        3
#define    T_DFLOAT        4
#define    T_BOOLEAN       5 
#define    T_EOP           6
#define    T_SEMICOLON     7
#define    T_RBRACE        8
#define    T_LBRACE        9
#define    T_NAME         10
#define    T_ASSIGN       11

  {SYM_DUMMY,         ASGN_DUMMY   ,       T_UNKNOWN},
  {SYM_TYPE,          ASGN_TYPE    ,       T_STRING},
  {SYM_WIDTH,         ASGN_WIDTH   ,       T_DIMENSION},
  {SYM_HEIGHT,        ASGN_HEIGHT  ,       T_DIMENSION},
  {SYM_DEPTH,         ASGN_DEPTH,          T_DIMENSION},
  {SYM_FUNCTION,      ASGN_FUNCTION,       T_STRING},
  {SYM_RADIUS,        ASGN_RADIUS,         T_STRING},
  {SYM_LANGUAGE,      ASGN_LANGUAGE,       T_STRING},
  {SYM_PHI,           ASGN_PHI,            T_NUMBER},
  {SYM_THETA,         ASGN_THETA,          T_NUMBER},
  {SYM_XMIN,          ASGN_XMIN,           T_STRING},
  {SYM_XMAX,          ASGN_XMAX,           T_STRING},
  {SYM_YMIN,          ASGN_YMIN,           T_STRING},
  {SYM_YMAX,          ASGN_YMAX,           T_STRING},
  {SYM_ZMIN,          ASGN_ZMIN,           T_STRING},
  {SYM_ZMAX,          ASGN_ZMAX,           T_STRING},
  {SYM_XVIEWMIN,      ASGN_XVIEWMIN,       T_STRING},
  {SYM_XVIEWMAX,      ASGN_XVIEWMAX,       T_STRING},
  {SYM_YVIEWMIN,      ASGN_YVIEWMIN,       T_STRING},
  {SYM_YVIEWMAX,      ASGN_YVIEWMAX,       T_STRING},
  {SYM_ZVIEWMIN,      ASGN_ZVIEWMIN,       T_STRING},
  {SYM_ZVIEWMAX,      ASGN_ZVIEWMAX,       T_STRING},
  {SYM_XRANGE,        ASGN_XRANGE,         T_STRING},
  {SYM_YRANGE,        ASGN_YRANGE,         T_STRING},
  {SYM_ZRANGE,        ASGN_ZRANGE,         T_STRING},
  {SYM_VIEWSET,       ASGN_VIEWSET,        T_STRING},
  {SYM_RANGESET,      ASGN_RANGESET,       T_STRING},
  {SYM_RECOMP,        ASGN_RECOMP,         T_BOOLEAN},
  {SYM_XIS,           ASGN_XIS,            T_STRING},
  {SYM_YIS,           ASGN_YIS,            T_STRING},
  {SYM_ZIS,           ASGN_ZIS,            T_STRING},
  {SYM_VAR1,          ASGN_VAR1,           T_STRING},
  {SYM_VAR2,          ASGN_VAR2,           T_STRING},
  {SYM_VAR3,          ASGN_VAR3,           T_STRING},
  {SYM_PLOTTYPE,      ASGN_PLOTTYPE,       T_NUMBER},
  {SYM_PLOTTICKS,     ASGN_PLOTTICKS,      T_NUMBER},
  {SYM_PLOTTICKDISABLE, ASGN_PLOTTICKDISABLE, T_BOOLEAN},
  {SYM_LABELOVERRIDES,ASGN_LABELOVERRIDES, T_NUMBER},
  {SYM_NUMPTS,        ASGN_NUMPTS,         T_NUMBER},
  {SYM_GRID1,         ASGN_GRID1,          T_NUMBER},
  {SYM_GRID2,         ASGN_GRID2,          T_NUMBER},
  {SYM_GRID3,         ASGN_GRID3,          T_NUMBER},
  {SYM_TICK1,         ASGN_TICK1,          T_NUMBER},
  {SYM_TICK2,         ASGN_TICK2,          T_NUMBER},
  {SYM_TICK3,         ASGN_TICK3,          T_NUMBER},
  {SYM_XLABEL,        ASGN_XLABEL,         T_STRING},
  {SYM_YLABEL,        ASGN_YLABEL,         T_STRING},
  {SYM_ZLABEL,        ASGN_ZLABEL,         T_STRING},
  {SYM_CONFX,         ASGN_CONFX,          T_NUMBER},
  {SYM_CONFY,         ASGN_CONFY,          T_NUMBER},
  {SYM_BOXTYPE,       ASGN_BOXTYPE,        T_NUMBER},
  {SYM_NUMBOXES,      ASGN_NUMBOXES,       T_NUMBER},
  {SYM_PLOTSTYLE,     ASGN_PLOTSTYLE,      T_STRING},
  {SYM_LIGHTING,      ASGN_LIGHTING,       T_NUMBER},
  {SYM_AXESSTYLE,     ASGN_AXESSTYLE,      T_STRING},
  {SYM_PLOTSHADING,   ASGN_PLOTSHADING,    T_STRING},
  {SYM_LINECOLOR,     ASGN_LINECOLOR,      T_STRING},
  {SYM_LINESTYLE,     ASGN_LINESTYLE,      T_NUMBER},
  {SYM_LINETHICK,     ASGN_LINETHICK,      T_NUMBER},
  {SYM_PTSTYLE,       ASGN_PTSTYLE,        T_STRING},
  {SYM_ISPTPLOT,      ASGN_ISPTPLOT,       T_BOOLEAN},
  {SYM_CONSTRAINED,   ASGN_CONSTRAINED,    T_BOOLEAN},
  {SYM_DISPLAYFLAGS,  ASGN_DISPLAYFLAGS,   T_STRING},
  {SYM_FITFRAME,      ASGN_FITTOFRAME,     T_BOOLEAN},
  {SYM_MAINTASP,      ASGN_MAINTAINASPECT, T_BOOLEAN},
  {SYM_SNAPSHOT,      ASGN_SNAPSHOT,       T_BOOLEAN},
  {SYM_OLDSNAPSHOT,   ASGN_OLDSNAPSHOT,    T_BOOLEAN},
  {SYM_VALIDFILE,     ASGN_VALIDFILE,      T_STRING},
  {SYM_OLDVALIDFILE,  ASGN_OLDVALIDFILE,   T_STRING},
  {SYM_CROPLEFT,      ASGN_CROPLEFT,       T_STRING},
  {SYM_CROPTOP,       ASGN_CROPTOP,        T_STRING},
  {SYM_CROPRIGHT,     ASGN_CROPRIGHT,      T_STRING},
  {SYM_CROPBOTTOM,    ASGN_CROPBOTTOM,     T_STRING},
  {SYM_RECTLEFT,      ASGN_RECTLEFT,       T_STRING},
  {SYM_RECTTOP,       ASGN_RECTTOP,        T_STRING},
  {SYM_RECTRIGHT,     ASGN_RECTRIGHT,      T_STRING},
  {SYM_RECTBOTTOM,    ASGN_RECTBOTTOM,     T_STRING},
  {SYM_FILENAME,      ASGN_FILENAME,       T_STRING},
  {SYM_FILEPROP,      ASGN_FILEPROP,       T_STRING},
  {SYM_TEMPFNAME,     ASGN_TEMPFNAME,      T_STRING},
  {SYM_TFILEPROP,     ASGN_TFILEPROP,      T_STRING},
  {SYM_OWIDTH,        ASGN_OWIDTH,         T_DIMENSION},
  {SYM_OHEIGHT,       ASGN_OHEIGHT,        T_DIMENSION},
  {SYM_OLD_ALIAS,     ASGN_OLDALIAS,       T_STRING},
  {SYM_ALIAS,         ASGN_ALIAS,          T_STRING},
  {SYM_T_ALIAS,       ASGN_T_ALIAS,        T_STRING},
  {SYM_OLDT_ALIAS,    ASGN_OLDT_ALIAS,     T_STRING},
  {SYM_DISCONT,       ASGN_DISCONT,        T_BOOLEAN},
  {SYM_GRAPHICSHP,    ASGN_GRAPHICSHP,     T_LBRACE},
  {SYM_GRAPHICSPS,    ASGN_GRAPHICSPS,     T_LBRACE},
  {SYM_GRFILENAME,    ASGN_GRFILENAME,     T_LBRACE},
  {SYM_LITERAL,       ASGN_LITERAL,        T_STRING},
  {SYM_GREYSCALE,     ASGN_GREYSCALE,      T_DFLOAT},
*/

/*
Within a \special, we need to identify the following tokens:
{LaTeX white space}"{"
{LaTeX white space}"%LaTeX comment"
{LaTeX white space}"field name"
{LaTeX white space}";"
{LaTeX white space}"}"

Note that the various types of special field values have
  their own token spaces and tokenizers.
*/

U16 TeXtokenizer::LocateSpecialToken( U8* src,U8* end_ptr,
										U16& s_off,U16& e_off,
                                   		    TCI_BOOL& is_comment ) {
//JBMLine( "LocateSpecialToken\n" );

  U16 rv    =  0;
  U8* rover =  src;

// White space in front of a special token is ignored.

  while ( *rover && *rover <= ' ' ) {
    rover++;
    s_off++;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
	return 0;
  }

  if ( *rover == '%' ) {	// comments can occur - I think
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;
    e_off++;
	is_comment  =  TRUE;
	return 2;
  }

  char ch =  *rover;
  if ( strchr("{;}",ch) ) {
    e_off =  s_off + 1;
	rv    =  2;
  } else {						// hit a field name

TCI_ASSERT( (ch>='A' && ch<='Z') || (ch>='a' && ch<='z') );

    e_off =  s_off;
    char* enders  =  "{\\;:='\" \n";
	while ( TRUE ) {
	  if ( *rover==0 ) {			// hit end of data
	    rv  =  2;
        break;
      } else if ( strchr(enders,*rover) ) {	// hit end of token
	    rv  =  2;
        break;
	  } else {
char ch =  *rover;
TCI_ASSERT( (ch>='A' && ch<='Z') || (ch>='a' && ch<='z')
			 || (ch>='0' && ch<='9') || ch=='-' || ch=='_' );
		e_off++;
		rover++;
	  }
    }		// loop thru bytes in field name

  }		// field name clause

  return rv;
}

/*
We're looking for the (string) value of a \special field.
Within context SPECSTR, we need to identify the following tokens:
{LaTeX white space : =}"%LaTeX comment"
{LaTeX white space : =}"dquoted matter"
{LaTeX white space : =}'squoted matter'
{LaTeX white space : =}\TEXUX
{LaTeX white space : =}";"
*/

U16 TeXtokenizer::LocateSpecStrToken( U8* src,U8* end_ptr,
										U16& s_off,U16& e_off,
                                   		  TCI_BOOL& is_chdata,
                                   		    TCI_BOOL& is_comment ) {
//JBMLine( "LocateSpecStrToken\n" );

  U16 rv    =  0;
  U8* rover =  src;

// White space and assignment operators in front
//  of a SpecialString token are ignored.

  while ( *rover ) {
    if ( *rover <= ' ' || *rover == ':' || *rover == '=' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
	return 0;
  }

  if ( *rover == '%' ) {	// comments can occur?
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  char ch =  *rover;

// Here we're in a string, "Like this" or 'this' or \TEXUX

  TCI_ASSERT( ch=='\'' || ch=='"' || ch==';' || ch=='\\' );

  if ( ch==';' ) {		// this is an ender for "T_STRING";
    e_off =  s_off + 1;
    rv    =  2;

  } else if ( ch=='\\' ) {		// \\TEXUX
    I16 tl  =  GetTokenLen( NULL,rover,end_ptr );
    e_off   =  s_off + tl;
    if ( !strncmp((char*)rover,"\\TEXUX",6) )
      rv  =  2;
	else
      TCI_ASSERT(0);

  } else {				// delimited string clause
    is_chdata =  TRUE;
	//s_off++;				// this excludes the delimiting byte
    e_off =  s_off + 1;
  	rover++;
    char ender  =  ch;
	while (TRUE) {
      if ( *rover == '\\' ) {			// backslash escape char
		e_off +=  2;
		rover +=  2;
      } else if ( ender == *rover ) {	// end of quoted data
	    e_off++;			// include the right delimiting byte
	    rv  =  2;
        break;
	  } else if ( *rover==0 ) {		 	// end of input buffer data
	    return 0;
	  } else {							// part of quoted data
	    if ( *rover == '\n' ) {
          if ( ender == '"' )
		    *rover  =  ' ';
		}
		e_off++;
		rover++;
	  }
    }		// while(TRUE)

  }		// delimited string clause

  return rv;
}


U16 TeXtokenizer::LocateNumberToken( U8* src,U8* end_ptr,
										U16& s_off,U16& e_off,
                                   		  TCI_BOOL& is_chdata,
                                   		    TCI_BOOL& is_comment ) {
//JBMLine( "LocateNumberToken\n" );

  U16 rv    =  0;
  U8* rover =  src;

// White space and assignment operators in front
//  of a number token are ignored.

  while ( *rover ) {
    if ( *rover <= ' ' || *rover == ':' || *rover == '=' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
	return 0;
  }

  if ( *rover == '%' ) {	// comments can occur?
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  char ch =  *rover;

TCI_ASSERT( (ch >= '0' && ch <= '9') || ch == ';' || ch == '-' || ch == '+' );

  if ( ch == ';' ) {
    e_off =  s_off + 1;
    rv    =  2;
  } else {
    is_chdata =  TRUE;
    e_off =  s_off;
    if ( ch == '-' || ch == '+' ) {
	  e_off++;
	  rover++;
	}

    if ( *rover >= '0' && *rover <= '9' )
      rv    =  2;
	else
      TCI_ASSERT( 0 );

    while ( *rover >= '0' && *rover <= '9' ) {
	  e_off++;
	  rover++;
    }
  }

  return rv;
}


U16 TeXtokenizer::LocateBooleanToken( U8* src,U8* end_ptr,
										U16& s_off,U16& e_off,
                               		      TCI_BOOL& is_comment ) {
//JBMLine( "LocateBooleanToken\n" );

  U16 rv    =  0;
  U8* rover =  src;

// White space in front of a Boolean token is ignored

  while ( *rover ) {
    if ( *rover <= ' ' || *rover == ':' || *rover == '=' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
	return 0;
  }

  if ( *rover == '%' ) {	// comments can occur?
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  char ch =  *rover;
    
TCI_ASSERT( ch=='T' || ch=='F' || ch==';' );

  if ( ch == ';' ) {
    e_off =  s_off + 1;
    rv    =  2;
  } else if ( !strncmp((char*)rover,"TRUE",4) ) {
    e_off =  s_off + 4;
    rv    =  2;
  } else if ( !strncmp((char*)rover,"FALSE",5) ) {
    e_off =  s_off + 5;
  	rv    =  2;
  } else {
	TCI_ASSERT(0);
  	rv    =  1;
  }

  return rv;
}


U16 TeXtokenizer::LocateLBraceToken( U8* src,U8* end_ptr,
										U16& s_off,U16& e_off,
                                   		  TCI_BOOL& is_chdata,
                                   		    TCI_BOOL& is_comment ) {
//JBMLine( "LocateLBraceToken\n" );

  U16 rv    =  0;
  U8* rover =  src;

// White space in front of an LBrace is ignored?

  while ( *rover ) {
    if ( *rover <= ' ' || *rover == ':' || *rover == '=' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {			// end of data in input buffer
    s_off  =  0;
	return 0;
  }

  if ( *rover == '%' ) {		// comments can occur?
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  char ch =  *rover;

// I didn't know the syntax of LBrace at the time of implementation

  TCI_ASSERT(0);
  rv =  1;

  return rv;
}


U16 TeXtokenizer::LocateDFloatToken( U8* src,U8* end_ptr,
    									U16& s_off,U16& e_off,
                                   		  TCI_BOOL& is_chdata,
                                    	    TCI_BOOL& is_comment ) {
//JBMLine( "LocateDFloatToken\n" );

  U16 rv    =  0;
  U8* rover =  src;

// White space in front of a DFloat token is ignored?

  while ( *rover ) {
    if ( *rover <= ' ' || *rover == ':' || *rover == '=' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
	return 0;
  }

  if ( *rover == '%' ) {	// comments can occur?
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;				// include the '\n' in the comment
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  char ch =  *rover;

// I didn't know the syntax of DFloat at the time of implementation

  TCI_ASSERT(0);
  rv  =  1;

  return rv;
}

/*
\hyphenation{ac-cent-ed-sym-bol add-to-counter add-to-length align-at
  aligned-at allow-dis-play-breaks ams-art ams-cd ams-intsm ams-la-tex
  amsl-doc ams-symb ams-texdf ams-text ams-xtra bmatrix bold-sym-bol
  cen-ter-tags curr-addr eqn-ar-ray idots-int int-lim-its latex
  make-title med-space mr-abbrev neg-med-space neg-thick-space
  neg-thin-space new-en-vi-ron-ment new-theo-rem no-ams-fonts
  no-int-lim-its no-name-lim-its over-left-arrow over-left-right-arrow
  over-right-arrow pmatrix ps-ams-fonts qed-sym-bol set-length side-set
  small-er subj-class tbinom the-equa-tion theo-rem theo-rem-style
  thick-space thin-space un-der-left-arrow un-der-left-right-arrow
  un-der-right-arrow use-pack-age var-inj-lim var-proj-lim vmatrix
  xalign-at xx-align-at}
*/

U16 TeXtokenizer::LocateHyphenationToken( U8* src,U8* end_ptr,
    										U16& s_off,U16& e_off,
                                   		  	  TCI_BOOL& is_chdata,
                                    	    	TCI_BOOL& is_comment ) {
//JBMLine( "LocateHyphenationToken\n" );

  U16 rv    =  0;
  U8* rover =  src;

// White space in front of a Hyphenation token is ignored.

  while ( *rover ) {
    if ( *rover <= ' ' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
	return 0;
  }

  if ( *rover == '%' ) {	// comments can occur?
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;				// include the '\n' in the comment
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  char ch =  *rover;

  if        ( ch == '{' || ch == '}' ) {
    e_off =  s_off + 1;
	rv  =  2;				// success

  } else if ( (ch >= 'a' && ch <= 'z') || (ch>='A' && ch<='Z') ) {
    is_chdata =  TRUE;
    e_off =  s_off;

    while( ch = *rover ) {
      if ( (ch >= 'a' && ch <= 'z') || (ch>='A' && ch<='Z') ) {
	    e_off++;
	    rover++;
      } else if ( ch == '-' ) {
	    e_off++;
	    rover++;
      } else
	    break;
    }
	rv  =  2;				// success

  } else {
	TCI_ASSERT(0);
	e_off++;
    rv  =  1;
  }

  return rv;
}


U16 TeXtokenizer::LocatePreambleToken( U8* src,U8* end_ptr,
    									U16& s_off,U16& e_off,
                                    	  TCI_BOOL& is_comment ) {
//JBMLine( "LocatePreambleToken\n" );

  U16 rv    =  0;		// assumes end of data in input buffer
  U8* rover =  src;

// White space in front of a PREAMBLE token is ignored.

  while ( *rover ) {
    if ( *rover <= ' ' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
	TCI_ASSERT(0);
    s_off  =  0;
	return 0;
  }


  if ( *rover == '%' ) {	// LaTeX comments can occur
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;				// include the '\n' in the comment
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  char ch =  *rover;

  if ( ch == '\\' ) {    				// \cmd in preamble
    I16 tl  =  GetTokenLen( NULL,rover,end_ptr );
    e_off   =  s_off + tl;
    rv  =  2;
  } else if ( strchr("{}[]",ch) ) {		// grouping token in preamble
    if ( ch == '{' && *(rover+1) == '}' ) {
      e_off   =  s_off + 2;
      rv  =  2;
	} else {
      e_off   =  s_off + 1;
      rv  =  2;
	}
  } else if ( !strncmp((char*)rover,"document",8) ) {
    e_off   =  s_off + 8;
    rv  =  2;
  } else if ( !strncmp((char*)rover,"tcilatex",8) ) {
    TCI_ASSERT(0);
    e_off   =  s_off + 8;
    rv  =  2;
  } else {
    e_off   =  s_off + 1;
    rv  =  1;
TCI_ASSERT(0);
  }		// word(s) in TEXT

  return rv;
}


// TeX glues occur in required and optional command parameters,
//  and as values in \special{}.  Tokens that bound glues are
//  { } [ ] and ;
//  Names of LaTeX variables, like \baselineskip, can occur
//  in glues.

U16 TeXtokenizer::LocateGlueToken(  U8* src,U8* end_ptr,
    								  U16& s_off,U16& e_off,
                                   		TCI_BOOL& is_chdata,
                                    	  TCI_BOOL& is_comment ) {
//JBMLine( "LocateGlueToken\n" );

  U16 rv    =  0;			// assumed end of data in buffer
  U8* rover =  src;

// White space in front of a Glue token is ignored?

  while ( *rover ) {
    if ( *rover <= ' ' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
	return 0;
  }

// glues generally occur in \cmd arguments - therefore comments

  if ( *rover == '%' ) {	// LaTeX comments can occur
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;				// include the '\n' in the comment
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  if        ( strchr("[]{};+-",*rover) ) {
    e_off =  s_off + 1;
	rv  =  2;

  } else if ( strchr("0123456789.",*rover) ) {
    is_chdata =  TRUE;
    e_off =  s_off;

    while( strchr("0123456789.",*rover) ) {
	  e_off++;
	  rover++;
    }
    rv  =  2;

  } else if ( *rover=='\\' ) {

    I16 tl  =  GetTokenLen( NULL,rover,end_ptr );
    e_off   =  s_off + tl;
    rv  =  2;

  } else if ( *rover >= 'a' && *rover <= 'z' ) {
    e_off =  s_off;

    while( *rover >= 'a' && *rover <= 'z' ) {
	  e_off++;
	  rover++;
    }
    rv  =  2;

  } else {
	TCI_ASSERT(0);
	e_off++;
    rv  =  1;
  }

  return rv;
}


// TeX dimens occur in required and optional command parameters,
//  and as values in \special{}.  Tokens that bound dimens are
//  { } [ ] and ;  The actual dimen is reported as chdata, but
//  we restrict the syntax here so that only a signed number
//  followed by a unit abbreviation is accepted.
//  Names of LaTeX variables, like \baselineskip, can occur
//  where a dimen required.

U16 TeXtokenizer::LocateDimenToken( U8* src,U8* end_ptr,
                                    U16& s_off,U16& e_off,
                                   		TCI_BOOL& is_chdata,
                                    	  TCI_BOOL& is_comment ) {

  U16 rv    =  0;			// assumed end of data in buffer
  U8* rover =  src;

// White space in front of a Dimen token is ignored?
  while ( *rover ) {
    if ( *rover <= ' ' ) {
      rover++;
      s_off++;
    } else
      break;
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
    return 0;
  }

// dimens generally occur in \cmd arguments - therefore comments

  if ( *rover == '%' ) {	// LaTeX comments can occur
    e_off =  s_off;
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;				// include the '\n' in the comment
    e_off++;
    is_comment  =  TRUE;
    return 2;
  }

  if        ( strchr("[]{};",*rover) ) {	// dimen delimiters
    e_off =  s_off + 1;
    rv  =  2;

  } else if ( strchr("+-0123456789.",*rover) ) {
    is_chdata =  TRUE;
    e_off =  s_off;

    if ( *rover=='+' || *rover=='-' ) {
      e_off++;
      rover++;
      if ( *rover=='\\' ) {
        I16 tl  =  GetTokenLen( NULL,rover,end_ptr );
        e_off   +=  tl;
      }
    }

    bool has_digits =  false;
    while ( strchr("0123456789.",*rover) ) {
      if ( *rover>='0' && *rover<='9' )
        has_digits =  true;
      e_off++;
      rover++;
    }

    if ( has_digits ) {			// there must be unit
      while ( *rover ) {
        if ( *rover <= ' ' ) {
          rover++;
          s_off++;
        } else
          break;
      }
      if ( *rover == 0 ) {		// end of data in input buffer
        s_off  =  0;
        return 0;
      }

      if ( *rover=='\\' ) {
// Note that we are treating this LaTeX token as chdata
//  It is not subject to lookup - typically it's an underlying var.

        I16 tl  =  GetTokenLen( NULL,rover,end_ptr );
        e_off   +=  tl;

      } else {
        U16 bytesdone =  ParseTeXUnit( rover );
        e_off +=  bytesdone;
        rover +=  bytesdone;
      }
    }

    rv  =  2;

  } else if ( *rover=='\\' ) {

// Note that we are treating this LaTeX token as chdata
//  It is not subject to lookup - typically it's an underlying var.

    is_chdata =  TRUE;
    I16 tl  =  GetTokenLen( NULL,rover,end_ptr );
    e_off   =  s_off + tl;
    rv  =  2;

  } else {
    TCI_ASSERT(0);
    e_off++;
    rv  =  1;
  }

  return rv;
}



U16 TeXtokenizer::LocateVerbatimToken( U8* src,U8* end_ptr,
										U32 src_off,
    									U16& s_off,U16& e_off,
    									TCI_BOOL& is_chdata ) {
//JBMLine( "LocateVerbatimToken\n" );

  U8* rover =  src;

// We don't really tokenize in VERBATIM mode.  We locate all
//  bytes from the current point to the end of the current line,
//  and treat that chuck as a "token". 

// Any white space after "\begin{verbatim}" that is on the current
//  line is not part of the verbatim data.  We step over it here.

  if ( IsOwnedSpace(src_off) ) {
    while ( *rover && *rover <= ' ' && *rover != '\n' ) {
      rover++;
      s_off++;
    }
    if ( *rover == '\n' ) {	// eoln with possible space in front
      rover++;
      s_off++;
    }
  }

  if ( *rover == 0 ) {		// end of data in input buffer
    s_off  =  0;
	return 0;
  }

// span to next eoln

  e_off =  s_off;
  while ( *rover && *rover != '\n' ) {
    rover++;
    e_off++;
  }
  TCI_ASSERT( *rover == '\n' );
  
// Note that '\n' is being included in the token!

  e_off++;
  is_chdata =  TRUE;

  return 2;
}


// Certain params to LaTeX \cmds contain data bytes that don't
//  represent pre-defined tokens.  However, comments and
//  eolns that mean space may occur.

U16 TeXtokenizer::LocateNonLaTeXToken( U8* src,U8* end_ptr,
    									                 U16& s_off, U16& e_off,
                                   		 TCI_BOOL& is_chdata,
                                    	    TCI_BOOL& is_comment ) {
//JBMLine( "LocateNonLaTeXToken\n" );

  U16 rv    =  0;		// assumed end of input buffer data
  U8* rover =  src;

// White space in front of NONLATEX token is ignored?

  while ( *rover ) {
    if ( *rover <= ' ' ) {
      rover++;
      s_off++;
	} else
	  break;
  }

  if ( *rover == 0 ) {
    s_off  =  0;
	return rv;			// end of data
  }

  e_off =  s_off;
  if ( *rover == '%' ) {	// LaTeX comments can occur
    while ( *rover != '\n' ) {
      rover++;
      e_off++;
    }
    rover++;				// include the '\n' in the comment
    e_off++;
    is_comment  =  TRUE;
    return 2;			// comment token
  }

// Here we only tokenize "{" and "}" - all other bytes are
//  gathered in pseudo "words".
//  '\n' is NOT converted to ' '.	????

  if ( strchr("{}[]",*rover) ) {
    e_off++;
    rv  =  2;					// token
  }	else {
    is_chdata =  TRUE;
    rv  =  2;
    while ( TRUE ) {
      char ch  =  *rover;
	  if ( ch==0 ) {			// end of data in input buffer
        s_off =  0;
	    return 0;
	  }
	  if        ( ch == '\\' ) {	// escape - step over next byte
        rover++;
        e_off++;
      } else if ( strchr("%{}[]",*rover) )
        break;

      rover++;
      e_off++;
    }			// while ( TRUE )

  }		// chdata clause

  return rv;
}


U16 TeXtokenizer::LocatePassthruToken( U8* src, U8* end_ptr,
    									                 U16& s_off, U16& e_off,
                                   		  TCI_BOOL& is_chdata,
                                    	    TCI_BOOL& is_comment ) {
    U16 rv    =  0;		// assumed end of input buffer data
    U8* rover =  src;
    
    if ( strchr("{}[]",*rover) ) {
       e_off++;
       rv  =  2;					// token
       return rv;
    }

    is_chdata = true;
    is_comment = false;
    int depth = 0;
    while (*rover){
      if ( *rover == '{' ) {
        ++depth;
      } else if (*rover == '}' ){
        --depth;
        if (depth == -1)
          break;
      } else if (*rover == '\\'){
         ++rover;
      }
      ++rover;
    }
    s_off = 0;
    e_off = rover - src;
    return 2;
}



U16 TeXtokenizer::ParseTeXUnit( U8* p_unit ) {

// pt<uID14.2.1>
// in<uID14.2.2>
// mm<uID14.2.3>
// cm<uID14.2.4>
// ex<uID14.2.5>
// em<uID14.2.6>
// pc<uID14.2.7>

  U8* ptr =  p_unit;

  if ( *ptr=='p' || *ptr=='i' || *ptr=='m' || *ptr=='c' || *ptr=='e' ) {
    ptr++;
    if ( *ptr=='t' || *ptr=='n' || *ptr=='m' || *ptr=='x' || *ptr=='c' ) {
      ptr++;
    } else
      TCI_ASSERT(0);
  } else
    TCI_ASSERT(0);

  return ptr - p_unit;
}

