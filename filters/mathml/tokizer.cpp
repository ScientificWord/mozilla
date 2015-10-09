
/*
  A Tokizer provides one main service to a Parser.
    Parsers call Tokizer::GetTNODE to get tokens from the source stream.
    If the token has a template, a pointer to the template is passed back.

  Note that Tokizers DO NOT hold any environment/context/state info about
    a particular parse or translation.  These issues are the domain of
    the calling Parser or Translater.

  In this implementation, a Grammar extracts the id of the "tokenizer"
    that it is to use from the .gmr file used in its creation.  A 
    Tokenizer object is created - it implements LocateToken function(s).

  This Tokizer owns a "MacroStore" object.  When a Parser using the
    tokizer encounters a "\newcommand", the defining data for the
    macro is passed to tokizer's MacroStore.  Thus the underlining
    Grammar(.gmr) is supplemented at parse time with file specific
    macros.  After this tokizer builds a TNODE in GetTNODE, if the
    token is not defined in the .gmr file, the MacroStore is checked
    for a definition of the token.  If the token turns out to be a
    macro call, Tokizer replaces the call with its expansion.
    Thus Parser does not see the macro call - just its expansion.
*/
  
#include "tokizer.h"
#include "grammar.h"
#include "textoken.h"
#include "mcrostor.h"
#include "nthmstor.h"
#include "fltutils.h"

#include <string.h>
#include <stdlib.h>

#define LINELIMIT         32767

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/* A "Grammar" stores a collection of definitions of grammar elements.
    It implements functions that look up these definitions.  For Parsing,
    the lookup is hashed on the literal that identifies an instance of
    the grammar element.  For Translating, the elements are hashed on the
    uID of the element - in this case the client is going from a uID
    back to an element in a grammar.
*/

Tokizer::Tokizer( Grammar* TeX_grammar,NewObjectStore* obj_store,
															FILE* sf ) {
//JBMLine( "Tokizer::ctor\n" );

  s_grammar     =  TeX_grammar;

  tokenizerID   =  TOKENIZER_TEX;
  tokenizer     =  (TeXtokenizer*)NULL;
  input_mode    =  IM_STANDARD;		  // standard or verbatim

  macro_store   =  TCI_NEW( MacroStore() );
  nobject_store =  obj_store;

  buf_start_offset  =  0L;
  buf_lim       =  0L;
  buf_ptr       =  NULL;
  curr_line_num =  0;
  linenum_list  =  NULL;
  has_preamble  =  FALSE;

  srcfile   =  NULL;
  BA_input  =  FALSE;

  comment_buffer  =  (TNODE*)NULL;

  if ( tokenizerID == TOKENIZER_TEX ) {
//LogLine( "Constructing TeX tokenizer\n" );
    tokenizer   =  TCI_NEW( TeXtokenizer( this) );
  }

  buf_ptr   =  NULL;

  buf_start_offset  =  0L;
  buf_lim   =  0L;
  is_eof    =  FALSE;

  curr_line_num =  0;
  linenum_list  =  (LINENUM_REC*)NULL;
  has_preamble  =  FALSE;

  srcfile   =  sf;


  // WARNING: the following call loads the first batch from the file
  //  into the global file buffer.

  if ( sf )
    is_latex  =  IsLaTeXFile( sf );

  if ( tokenizer )
    tokenizer->Reset();

}


Tokizer::~Tokizer() {
//JBMLine( "Tokizer_dtor\n" );

  if ( macro_store )
    delete macro_store;

  
  if ( tokenizer )
    delete tokenizer;

  if ( buf_ptr ) delete buf_ptr;

  LINENUM_REC* rover  =  linenum_list;
  while ( rover ) {
    LINENUM_REC* del  =  rover;
    rover =  rover->next;
    delete del;
  }

  if ( comment_buffer ) {
//JBMLine( "Dispose comment_buffer\n" );
    DisposeTNode( comment_buffer );
    comment_buffer  =  NULL;
  }
}




// env_list_subset  =  "math0text00"
// env_list_superset=  "junk0text0math00"

TCI_BOOL Tokizer::IsSubset( char* env_list_subset,char* env_list_superset ) {

  while ( *env_list_subset ) {
    char* test_names  =  env_list_superset;

    while ( *test_names ) {
      if ( !strcmp(env_list_subset,test_names) )
        break;
      else
        test_names  +=  strlen(test_names) + 1;
    }
    if ( *test_names )          // found
      env_list_subset +=  strlen(env_list_subset) + 1;
    else
      break;
  }

  return (*env_list_subset) ? FALSE : TRUE;
}



TNODE* Tokizer::GetTNODE( U8* zcurr_env,U32 src_off,U8** ztmpl ) {
/*
char zzz[80];
sprintf( zzz,"Tokizer::GetTNODE, env=%s, src_off=%lu\n",zcurr_env,src_off );
JBMLine( zzz );
*/

  TNODE* rv =  (TNODE*)NULL;
  error_flag  =  0;

  TCI_ASSERT( buf_ptr );        // verify that we have an input buffer

  U8  tok_buffer[128];
  U16 compound_token_count =  0;
  U32 the_s_off;
  U32 the_e_off;
  U32 cum_width     =  0;
  U32 comment_width =  0;

// This is where I handle compound tokens like "\begin {eqnarray*}".
// tokenizer->LocateToken returns "\begin", " ", "{", "eqnarray*", and "}".
// Notebook.gmr defines "\begin{eqnarray*}" as a single compound token.
// The following loop catenates atoms from tokenizer into the larger token.

  do {
    U8* ptr;
    U8* end_ptr;

  // We handle comments and input buffer reloading here

    TCI_BOOL in_comment =  FALSE;
    while ( TRUE ) {
      ptr     =  buf_ptr + ( src_off + cum_width + comment_width - buf_start_offset );
      end_ptr =  buf_ptr + ( buf_lim - buf_start_offset );

      rv  =  GetToken( zcurr_env,src_off+cum_width+comment_width,
      						ptr,end_ptr,ztmpl,in_comment );

      if ( !rv && !BA_input ) {       // try to append more data to input buffer
        //U16 line_count  =  LoadSrcPara( is_latex,0L );
        //if ( line_count ) {
        if ( ReplenishInputBuffer(src_off) ) {
          ptr     =  buf_ptr + ( src_off + cum_width + comment_width - buf_start_offset );
          end_ptr =  buf_ptr + ( buf_lim - buf_start_offset );
          rv      =  GetToken( zcurr_env,src_off+cum_width+comment_width,
       								ptr,end_ptr,ztmpl,in_comment );
        }
      }		// input buffer reloading loop

      in_comment  =  FALSE;
      if ( rv ) {
 	    U16 uobjtype,usubtype,uID;
 	    GetUids( rv->zuID,uobjtype,usubtype,uID );
		if ( uobjtype == 777 ) {
/*
char zzz[256];
sprintf( zzz,"Hit a comment,*%s*,s_off=%lu\n",
(char*)rv->cv_list->cvline,src_off );
JBMLine( zzz );
*/
          comment_width +=  rv->src_offset2;
          AppendComments( rv );
          in_comment  =  TRUE;
		  rv  =  NULL;
		} else
		  break;
	  } else
		break;
    }	// while loop over comment lines and input buffer reloading

  // Here we have acquired the token at position "src_off" - it may be NULL
  //  if this position marks the end of data.  The token may be from source
  //  or the expansion of a macro.  Note that a comment is NOT a token -
  //  we may have accumulated comment(s) in the comment_buffer as a side
  //  affect of acquiring the token.

    if ( rv ) {         // we've got the requested token
      U16 uobjtype,usubtype,uID;
      GetUids( rv->zuID,uobjtype,usubtype,uID );

      if        ( compound_token_count ) {  // We're processing a multi-token
        compound_token_count++;
        if ( compound_token_count==2 ) {        // check for  "{"
          if ( uobjtype!=6 || usubtype!=1 || uID!=0 ) {
            TCI_ASSERT( 0 );
            DisposeTNode( rv );
            return NULL;
          }
        }

      // Append this token to "tok_buffer"

        if ( uobjtype!=9 ) {    // we exclude spacing tokens here
          U16 tln =  rv->src_offset2 - rv->src_offset1;

          U16 b_used  =  strlen( (char*)tok_buffer );
          if ( b_used + tln >= 126 ) {
            TCI_ASSERT( 0 );
            DisposeTNode( rv );
            return NULL;
          }

          strncat( (char*)tok_buffer,(char*)ptr + rv->src_offset1,tln );
/*
JBMLine( "Appending to compound token\n" );
JBMLine( (char*)tok_buffer );
JBMLine( "\n" );
*/
        }

      // Does this atom end our compound token?

        if ( uobjtype==6 && usubtype==1 && uID==1 ) {   // this is "}"

       // build the TNODE for the compound token and break

          U8* zuID;
          I16 tl  =  strlen( (char*)tok_buffer );
          if ( !GGDFromNameAndAttrs(tok_buffer,tl,NULL,zcurr_env,&zuID,ztmpl) )
            zuID  =  (U8*)"999.0.0";    // this object is not yet defined
          the_e_off =  cum_width + comment_width + rv->src_offset2;

          DisposeTNode( rv );
          rv  =  MakeTNode( the_s_off,the_e_off,0L,zuID );
          tokenizer->SetPrevTokenOwnsSpace( src_off+the_e_off,TRUE );
          break;

        } else {        // continue

          cum_width +=  rv->src_offset2;
          DisposeTNode( rv );
        }

      } else if ( uobjtype==20 ) {      // start compound token

        U16 tln =  rv->src_offset2 - rv->src_offset1;
        memcpy( (char*)tok_buffer,(char*)ptr + rv->src_offset1,tln );
        tok_buffer[tln] =  0;
        compound_token_count++;
/*
JBMLine( "Start compound token\n" );
JBMLine( (char*)tok_buffer );
JBMLine( "\n" );
*/
        the_s_off   =  comment_width + rv->src_offset1;
        cum_width   +=  rv->src_offset2;
        DisposeTNode( rv );

      } else {          // single token exits here - normal case

        U16 tln   =  rv->src_offset2 - rv->src_offset1;
        if ( tln>31 ) tln =  31;
        memcpy( (char*)tok_buffer,(char*)ptr + rv->src_offset1,tln );
        tok_buffer[tln]   =  0;

        if ( comment_width ) {			// step over comments
          rv->src_offset1 +=  comment_width;
          rv->src_offset2 +=  comment_width;
		}
/*
JBMLine( "Normal token\n" );
JBMLine( (char*)tok_buffer );
JBMLine( "\n" );
*/
        break;
      }

    } else              // rv is NULL - end of data
      break;  

  } while ( TRUE );


// we expand macros in the following clause

  if ( rv ) {                                       // we have a token
        
    if ( !strcmp((char*)rv->zuID,"999.0.0") ) {     // not defined in .gmr

    // the token found is not defined in the source .gmr - it may be
    //  an instance of \newcommand{\token}{def}.

      bool do_dispose =  false;
      U16 n_params;
      U8* macro_def =  macro_store->GetMacroDef( tok_buffer,n_params );

      if ( !macro_def ) {   // token not instance of \newcommand
	                        // might be instance of \DeclareMathOp
        bool is_starred;
        U8* visual_def  =  macro_store->GetMathOpDef( tok_buffer,is_starred );
      
        if ( visual_def ) {     // token is instance of user mathop
          n_params    =  0;
          macro_def   =  MakeMathNomTmpl( tok_buffer,visual_def,is_starred );
		  do_dispose  =  true;
		}
	  }		// clause to detect mathnom instances
      
      if ( macro_def ) {        // this token starts a macro call
        TCI_BOOL error =  FALSE;
        
        U32 macro_start_off =  src_off + rv->src_offset1;
        U32 macro_slen;
        U8* macro_sub;

        if ( n_params ) {       // parameters are required
          U32 locs[32];
          memset( locs,0,32*sizeof(U32) );

          U32 param_start =  src_off + rv->src_offset2;
          U32 param_slen;
          if ( LocateMacroParams(zcurr_env,param_start,n_params,locs,param_slen) ) {
            macro_slen  =  rv->src_offset2 - rv->src_offset1 + param_slen;
            macro_sub   =  BuildMacroInstance( macro_def,n_params,locs );
		    do_dispose  =  true;
          } else {      // ERROR - bad params to macro call
            TCI_ASSERT( 0 );
            error =  TRUE;
          }

        } else {		// there are no params

          macro_slen  =  rv->src_offset2 - rv->src_offset1;
          macro_sub   =  macro_def;

        // if there is a ' ' after the call ("\gn "), this space is part
        //  of the call.

          U32 offset  =  macro_start_off + macro_slen;

          if ( offset >= buf_start_offset && offset < buf_lim ) {
            U8* ptr =  buf_ptr + offset - buf_start_offset;
            while ( *ptr == ' ' ) {
              macro_slen++;  
              ptr++;  
            }
          }

        }		// end no parms clause

      // Next plug in the macro_sub and start token-extraction

        if ( !error ) {

        // In the source buffer, replace the macro call with its value

          MacroReplace( macro_start_off,macro_slen,macro_sub );
          if ( do_dispose ) delete macro_sub;

          DisposeTList( rv );
          return GetTNODE( zcurr_env,macro_start_off,ztmpl );
        }

      } else {      // not instance of \newcommand or \DeclareMathOp

      // the token found is not defined in the source .gmr AND it's not
      //  a macro call - it may be a newtheorem call.

        U8* theorem_zuID;
        U8* theorem_tmpl;

        U16 ln  =  strlen( (char*)tok_buffer );
        if ( GetObjectDataFromName(zcurr_env,tok_buffer,ln,
        					&theorem_zuID,&theorem_tmpl) ) {

        // Here we patch up the info returned by this function,
        //  so that the caller sees a well-defined object.

          strcpy( (char*)rv->zuID,(char*)theorem_zuID );
          *ztmpl  =  theorem_tmpl;
        }  

      }         // newtheorem clause

    }       // 999.0.0 - unknown token

  }


  if ( rv ) {           // assign the input file line number
    U32 src_pos     =  src_off + rv->src_offset1;
    rv->src_linenum =  GetLineNum( src_pos );
/*
char zzz[80];
sprintf( zzz,"s=%lu, e=%lu\n",rv->src_offset1,rv->src_offset2 );
JBMLine( zzz );
*/

    strcpy( (char*)rv->src_tok,(char*)tok_buffer );
/*
JBMLine( (char*)rv->zuID );
JBMLine( "\n" );
JBMLine( (char*)rv->src_tok );
JBMLine( "\n" );
*/
  }

  return rv;
}






/*

// Functions to add bytes to the d_grammar stream.
//  We must address the issue of token separators.  The tokenizer
//  needs to be queried for a separating literal

void Tokizer::OutputBytes( U8* src,U16 sln,ByteArray& dest_ba ) {

  tokenizer->OutputBytes( src,sln,dest_ba );
}


void Tokizer::OutputToken( TNODE* obj,U8* ztoken,ByteArray& dest_ba ) {

  tokenizer->OutputToken( obj,ztoken,dest_ba );
}


void Tokizer::OutputAtom( TNODE* a_node,U8* ztoken,ByteArray& dest_ba ) {

  tokenizer->OutputAtom( a_node,ztoken,dest_ba );
}

*/

// Load the input buffer from a zstring

void Tokizer::SetSrcBuffer( U8* z_src ) {

  TCI_ASSERT( buf_ptr == 0 );

  buf_lim =  strlen( (char*)z_src );
  buf_ptr =  (U8*)TCI_NEW(char[buf_lim+1] );
  memcpy( (char*)buf_ptr,(char*)z_src,buf_lim );
  buf_ptr[buf_lim]  =  0;
/*
JBMLine( "SetSrcBuffer\n" );
JBMLine( (char*)buf_ptr );
JBMLine( "\n" );
*/
  BA_input  =  TRUE;
}


TCI_BOOL Tokizer::ReplenishInputBuffer( U32 src_off ) {
/*
char zzz[80];
sprintf( zzz,"ReplenishInputBuffer at offset=%ld\n",src_off );
JBMLine( zzz );
*/
  ClearInputBuffer( src_off );
  U32 lines_loaded  =  LoadSrcLines( is_latex,100 );

  return lines_loaded > 0 ? TRUE : FALSE;
}


// At certain points a Parser can interupt parsing - generally at the start
//  of any level 0 object.  At such times the parse tree generated so far
//  can be processed thru to final output.
// The following function (which is not really necessary) allows a parser
//  to notify its grammar of a good time to update its file buffer.

void Tokizer::ClearInputBuffer( U32 src_off ) {
/*
char zzz[80];
sprintf( zzz,"ClearInputBuffer to offset=%ld\n",src_off );
JBMLine( zzz );
*/
  I32 bytes_processed =  src_off - buf_start_offset;
  I32 bytes_to_do     =  buf_lim - src_off;

  if ( bytes_to_do > 0 )
    memmove( (char*)buf_ptr,(char*)buf_ptr + bytes_processed,bytes_to_do );

  buf_start_offset  =  src_off;
  buf_lim   =  src_off + bytes_to_do;

  LINENUM_REC* rover  =  linenum_list;
  linenum_list =  NULL;
  while ( rover ) {
    if ( rover->offset < buf_start_offset ) {
      LINENUM_REC* del  =  rover;
      rover =  rover->next;
      if ( rover && rover->offset >= buf_start_offset ) {
        linenum_list =  del;
	    break;
	  } else
        delete del;
    } else {
      linenum_list =  rover;
      break;
    }
  }

}


// Load the input buffer from a file.  If param "line_limit" is not 0,
//  we quit after the total number of lines read from the file reaches
//  "line_limit" - used to stop at the end of the preamble and break
//  the source file into batches.

// We read each line into a line buffer (32K), convert eoln sequence
//  to"\n", and append the line to a bytearray.  After accumulating
//  the requested no. of lines, a new input buffer is allocated.
//  The contents of the  previous input buffer and the bytearray
//  are contenated in the new buffer.

U32 Tokizer::LoadSrcLines( TCI_BOOL is_latex,U32 line_limit ) {
/*
char zzz[80];
sprintf( zzz,"Tokizer::LoadSrcLines, num_lines=%ld\n",line_limit );
JBMLine( zzz );
*/

  if ( is_eof ) return 0L;

  U32 rv  =  0L;			// no. of lines loaded

// We accumulate the lines in a temporary buffer

  U32 fb_limit    =  8192;
  U8* file_buffer =  (U8*)TCI_NEW( char[fb_limit] );
  U32 fbi =  0L;

  U8* linebuf =  (U8*)TCI_NEW( char[LINELIMIT] );
  U32 bytes_loaded  =  0L;

  while ( TRUE ) {  	// loop thru lines of source file

    char* fs_res  =  fgets( (char*)linebuf,LINELIMIT,srcfile );
	if ( !fs_res ) {
	  is_eof  =  TRUE;
	  break;
	}

    curr_line_num++;		// srcfile line number!!

    if ( is_latex ) {
      if ( !strncmp((char*)linebuf,"\\end{document}",14) ) {
	    is_eof  =  TRUE;
        break;
	  }
	}

	rv++;

    U16 ln  =  strlen( (char*)linebuf );

  // convert line end bytes to '\n'

    if ( ln >= 2 ) {
	  if        ( linebuf[ln-2] == '\r' ) {
	    linebuf[ln-2] =  '\n';
	    linebuf[ln-1] =   0;
		ln--;
	  } else if ( linebuf[ln-2] == '\n' ) {
	    linebuf[ln-1] =   0;
		ln--;
	  }
	}
    if ( ln >= 1 &&	linebuf[ln-1] == '\r' )
	  linebuf[ln-1] =   '\n';

  // All non-comment lines have a LINENUM_REC
  //  l_data->offset is relative to the start of the file - absolute!

    if ( linebuf[0] != '%' ) {
      LINENUM_REC* l_data =  (LINENUM_REC*)TCI_NEW(char[sizeof(LINENUM_REC)] );
      l_data->next        =  NULL;
      l_data->offset      =  bytes_loaded + buf_lim;
      l_data->line_num    =  curr_line_num;			// 1 based count!

      if ( linenum_list )
        linenum_last->next  =  l_data;
      else
        linenum_list  =  l_data;

      linenum_last  =  l_data;
    }

    file_buffer =  AppendBytesToBuffer( file_buffer,fb_limit,
                                              fbi,linebuf,ln );
    bytes_loaded  +=  ln;

    if ( line_limit && rv == line_limit )
      break;
  }     // loop thru file lines

// move data from src_ba to buffer

  U32 nbytes  =  fbi;
  TCI_ASSERT( bytes_loaded == nbytes );
  if ( nbytes ) {
    U32 curr_bytes  =  buf_lim - buf_start_offset;
/*
char zzz[80];
sprintf( zzz,"  curr bytes=%ld, new bytes=%ld\n",curr_bytes,nbytes );
JBMLine( zzz );
*/
    U8* tmp =  (U8*)TCI_NEW(char[ curr_bytes+nbytes+1 ] );
    if ( buf_ptr ) {
	  if ( curr_bytes )
        memcpy( (char*)tmp,(char*)buf_ptr,curr_bytes );
      delete buf_ptr;
    }
    memcpy( (char*)tmp+curr_bytes,(char*)file_buffer,nbytes );

    tmp[ curr_bytes+nbytes ]  =  0;
    buf_ptr =  tmp;
    buf_lim +=  nbytes;

/*
JBMLine( "LoadSrcLines::Buffer Dump\n" );
JBMLine( (char*)tmp );
JBMLine( "*\n\n" );
*/
  }		// if ( nbytes )

  delete file_buffer;
  delete linebuf;

  return rv;
}


TCI_BOOL Tokizer::IsLaTeXFile( FILE* sf ) {

  TCI_BOOL rv  =  FALSE;

  char* linebuf =  TCI_NEW( char[LINELIMIT] );
  while ( fgets(linebuf,LINELIMIT,sf) ) {    // loop thru lines of source file
    curr_line_num++;
    if ( !strncmp(linebuf,"\\begin{document}",16) ) {
      rv  =  TRUE;
      break;
    }
  }
  delete linebuf;

  fseek( sf,0,SEEK_SET );
  U32 begin_line_num =  curr_line_num;
  curr_line_num =  0;

  if ( rv ) {   // we've found \begin{document}

//JBMLine( "Tokizer calls LoadSrcPara for preamble\n" );

    U32 line_count  =  LoadSrcLines( TRUE,begin_line_num );
    has_preamble    =  TRUE;

  } else {
    U32 line_count  =  LoadSrcLines( FALSE,100 );
  }

  return rv;
}



U8* Tokizer::GetSrcPtr( U32 offset ) {

/*
char zzz[120];
sprintf( zzz,"Enter Tokizer::GetSrcPtr, off=%ld, s=%ld, e=%ld\n",
                offset,buf_start_offset, buf_lim );
JBMLine( zzz );
*/

  if ( offset >= buf_start_offset && offset < buf_lim )
    return buf_ptr + offset - buf_start_offset;
  else {
TCI_ASSERT( offset != 0L );
    return buf_ptr;
  }
}


U32 Tokizer::GetLineNum( U32 src_offset ) {

  U32 rv  =  0;

  LINENUM_REC* rover  =  linenum_list;
  while ( rover ) {
    if ( rover->offset > src_offset ) break;
    else {
      rv  =  rover->line_num;
      rover =  rover->next;
    }
  }

  return rv;
}


// When parsing source, we may encounter an unknown token or other error.
//  The caller knows the absolute offset of the problem in the buffer.
//  The following function copies the line in question from the buffer.
//  Generally, it is needed to give a message to the user.

U8* Tokizer::GetBufferedLine( U32 token_offset,
                                U32& line_start_off,U32& src_lineno ) {

  U8* rv  =  NULL;

  if ( token_offset >= buf_start_offset && token_offset < buf_lim ) {

    LINENUM_REC* prev   =  NULL;
    LINENUM_REC* rover  =  linenum_list;
    while ( rover && rover->offset <= token_offset ) {
      prev  =  rover;
      rover =  rover->next;
    }

   if ( prev ) {
    U32 l_start =  prev->offset;
    U32 l_end   =  rover ? rover->offset : buf_lim;
    if ( l_end > l_start ) {
      U16 ln  =  l_end - l_start;  
      rv  =  (U8*)TCI_NEW(char[ ln+1 ] );
      U8* lp  =  buf_ptr + l_start - buf_start_offset;
      memcpy( (char*)rv,(char*)lp,ln );
      rv[ln]  =  0;
      line_start_off  =  l_start;
      src_lineno      =  prev->line_num;
     }
   }

  }     // token is in buffer

  return rv;
}


// Here we call tokenizer->LocateToken and contruct TNODEs.
// The "token" located in the source buffer may not be pre-defined
// in the src grammar.  It may be a chunk of chdata.  In this case
// we must generate a uID here.

TNODE* Tokizer::GetToken( U8* zcurr_env,U32 src_off,
                              U8* ptr,U8* end_ptr,U8** ztmpl,
                                	TCI_BOOL in_comment ) {

  TNODE* rv =  NULL;
  *ztmpl    =  NULL;

  U16      s_off;       // reference params to tokenizer->LocateToken
  U16      e_off;
  TCI_BOOL is_word;
  TCI_BOOL is_white;
  TCI_BOOL is_comment;
  TCI_BOOL is_par;
  TCI_BOOL is_chdata;

  U16 tokizer_result  =  tokenizer->LocateToken(zcurr_env,
  										src_off,ptr,end_ptr,
                                		s_off,e_off,
                                		is_word,is_white,
                                		is_comment,is_par,is_chdata,
                                		in_comment );

  U16 uobjtype  =  0;
  U16 usubtype  =  0;
  U16 uID   =  0;

  U8 zuID[64];
  U8* sg_zuID   =  NULL;
  if        ( tokizer_result == 0 ) {

  // end of data currently in the input buffer - return NULL

  } else if ( tokizer_result == 1 ) {

  // we have data but it's NOT a token in the current context
    
	TCI_ASSERT(0);
    uobjtype  =  999;

  } else if ( tokizer_result == 2 ) {		// token located OK
    I16   tl  =  e_off - s_off;

  // Some tokens are IDed using flags set in LocateToken

	if        ( is_word ) {
      uobjtype  =  888;

    } else if ( is_white ) {
      uobjtype  =  9;
      usubtype  =  1;
	  uID =  1;

    } else if ( is_comment ) {
      uobjtype  =  777;

    } else if ( is_par ) {
      uobjtype  =  9;
      usubtype  =  2;

    } else if ( is_chdata ) {

	  switch ( input_mode ) {
	    case  IM_VERBATIM : {
          uobjtype  =  555;
		}
		break;
	    case  IM_SPECIAL  : {
          uobjtype  =  888;
          usubtype  =  1;
		}
		break;
	    case  IM_SPECSTR  : {
		  if        ( *(ptr+s_off) == '\'' ) {
            uobjtype  =  888;
            usubtype  =  2;
		  } else if ( *(ptr+s_off) == '"' ) {
            uobjtype  =  888;
            usubtype  =  3;
		  } else
		    TCI_ASSERT(0);
		}
		break;
	    case  IM_NUMBER   : {
          uobjtype  =  888;
          usubtype  =  4;
		}
		break;
	    case  IM_LBRACE   : {
          uobjtype  =  888;
          usubtype  =  5;
		}
		break;
	    case  IM_DFLOAT   : {
          uobjtype  =  888;
          usubtype  =  6;
		}
		break;
	    case  IM_TEXDIMEN : {
          uobjtype  =  888;
          usubtype  =  7;
		}
		break;
	    case  IM_NONLATEX : {
          uobjtype  =  888;
          usubtype  =  8;
		}
		break;
	    case  IM_HYPHENATION : {
          uobjtype  =  888;
          usubtype  =  9;
		}
		break;
	    case  IM_TEXGLUE  : {
          uobjtype  =  888;
          usubtype  =  10;
		}
		break;
    case  IM_PASSTHRU : {
          uobjtype  =  888;
          usubtype  =  11;
		}
		break;

		default :
		  TCI_ASSERT(0);
		break;
	  }						// switch ( input_mode )

    } else if ( tl == 2 && *(ptr+s_off) == '\\' && *(ptr+s_off+1) == '\n' ) {
      uobjtype  =  9;
      usubtype  =  1;
	  uID =  2;

    } else if ( tl == 2 && *(ptr+s_off) == '\\' && *(ptr+s_off+1) == '\r' ) {
      uobjtype  =  9;
      usubtype  =  1;
	  uID =  2;

  // Most tokens are pre-defined in the .gmr file

    } else if ( GGDFromNameAndAttrs(ptr+s_off,tl,
   								NULL,zcurr_env,&sg_zuID,ztmpl) ) {
/*
char zzz[512];
sprintf( zzz,"  FOUND, zuID=%s, tmpl=%s\n",sg_zuID,*ztmpl );
JBMLine( zzz );
*/
 	  strcpy( (char*)zuID,(char*)sg_zuID );

    } else {    // lookup failed - this object is not yet defined
/*
char zzz[80];
sprintf( zzz,"TOKEN NOT FOUND, env=%s, *",zcurr_env );
JBMLine( zzz );
U16 zi =  0;
while ( zi<tl && zi<75 ) { zzz[zi] =  *(ptr+s_off+zi); zi++; }  
zzz[zi++] = '*';
zzz[zi++] = '\n';
zzz[zi] = 0;
JBMLine( zzz );
*/
      uobjtype  =  999;
    }

  }     // Token located OK

  if ( uobjtype || sg_zuID ) {

    if ( sg_zuID == NULL )
      UidsTozuID( uobjtype,usubtype,uID,zuID );

    rv  =  MakeTNode( s_off,e_off,0L,zuID );

    if ( is_comment || ( input_mode==IM_VERBATIM && is_chdata ) ) {
      U16 lln =  e_off - s_off;
      if ( is_comment && lln > 1 ) {
	    if ( *(ptr+e_off-1) == '\n' )
          lln--;
	  }
      CVLINE_REC* tmp =  MakeCVNode( 0,ptr+s_off,lln  );
/*
JBMLine( (char*)tmp->cvline );
JBMLine( "\n" );
*/
      rv->cv_list =  JoinCVLists( rv->cv_list,tmp );

    } else if ( is_word || is_chdata ) {

  // if the token is a run of TEXT
  //  we put the actual text in the var_value field

      if ( uobjtype==888 && (usubtype==2 || usubtype==3) ) {
		s_off++;  e_off--;		// exclude delimiters
	  }
      U16 vln   =  e_off - s_off;
      U8* tmp   =  (U8*)TCI_NEW( char[ vln+1 ] );

	  char* p =  (char*)ptr + s_off;
	  U16 si  =  0;
	  U16 di  =  0;
	  while ( di < vln ) {
	    if ( *p == '\n' || *p == '\r' || *p == ' ' ) {
          if ( usubtype==0 ) {
		    if ( si && tmp[si-1]==' ' ) {
			// do nothing - consecutive spaces
		    } else
	          tmp[si++] =  ' ';

		  } else
	        tmp[si++] =  ' ';
		} else
	      tmp[si++] =  *p;
		di++;
		p++;
	  }					// loop thru src bytes
      tmp[si]  =  0;
/*
JBMLine( "Building word token *" );
JBMLine( (char*)tmp );
JBMLine( "*\n" );
*/
      rv->var_value =  tmp;
      rv->v_len =  si;

	}		// if ( chdata )

  }     // if ( zuID )

  return rv;
}


/*
  When we encounter a macro call, Tokizer replaces it in the source buffer
    with it's expansion.  The list of source line numbers is also updated.
*/

void Tokizer::MacroReplace( U32 macro_start_off,U32 macro_slen,
                                                        U8* instance ) {

  I32 expanded_len  =  strlen( (char*)instance );
  I32 nhead_bytes   =  macro_start_off - buf_start_offset;
  I32 ntail_bytes   =  buf_lim - ( macro_start_off + macro_slen );
  I32 delta =  expanded_len - macro_slen;

  if ( expanded_len <= macro_slen ) {   // don't need a bigger source buffer

    memcpy( (char*)buf_ptr + nhead_bytes,(char*)instance,expanded_len );

    if ( expanded_len < macro_slen ) {
      char* dest  =  (char*)buf_ptr + macro_start_off - buf_start_offset + expanded_len;
      char* src   =  (char*)buf_ptr + macro_start_off - buf_start_offset + macro_slen;
      memmove( dest,src,ntail_bytes+1 );
    }

  } else {      // allocate a larger buffer

    I32 new_size  =  nhead_bytes + expanded_len + ntail_bytes;

    U8* new_buf =  (U8*)TCI_NEW(char[ new_size+1 ] );

    memcpy( (char*)new_buf,(char*)buf_ptr,nhead_bytes );
    memcpy( (char*)new_buf + nhead_bytes,(char*)instance,expanded_len );

    char* src   =  (char*)buf_ptr + macro_start_off - buf_start_offset + macro_slen;
    memcpy( (char*)new_buf + nhead_bytes + expanded_len,src,ntail_bytes+1 );

    delete buf_ptr;
    buf_ptr =  new_buf;
  }

  if ( delta ) {
    buf_lim +=  delta;

    LINENUM_REC* rover  =  linenum_list;
    while ( rover ) {
      if ( rover->offset > macro_start_off )
        rover->offset =  rover->offset + delta;
      rover =  rover->next;
    }
  }

}


/*
          // Build a copy of the expanded macro call
          //  with param place holders ( #1 ) replaced by values.

          U16 nbytes  =  strlen( (char*)macro_def );
          U8* macro_sub =  (U8*)TCI_NEW(char[nbytes+1] );
          strcpy( (char*)macro_sub,(char*)macro_tmpl );
*/

void Tokizer::AddMacro( U8* nom,U8* def,U16 p_count,U16 scope ) {
/*
char zzz[256];
sprintf( zzz,"Tokizer::AddMacro, nom=%s, def=%s, p_count=%d,scope=%d\n",
nom,def,p_count,scope );
JBMLine( zzz );
*/

  if ( macro_store )
    macro_store->AddMacro( nom,def,p_count,scope );
}


void Tokizer::AddMathOp( U8* nom,U8* def,bool is_starred,U16 scope ) {
/*
char zzz[256];
sprintf( zzz,"Tokizer::AddMathOp, nom=%s, def=%s, is_starred=%d,scope=%d\n",
nom,def,is_starred,scope );
JBMLine( zzz );
*/

  if ( macro_store )
    macro_store->AddMathOp( nom,def,is_starred,scope );
}


// We have encountered a macro call in the source buffer.  The definition
//  of this macro mandates that it has params.
//  definition  \newcommand{\good}[3]{{#1}$({#2};{#3})$}
//
//  call        \good{\em gnus\/}{x}{54}
//                    ^         ^ ^^ ^ ^
// This function locates the positions of the parameter values.


TCI_BOOL Tokizer::LocateMacroParams( U8* pENV,U32 src_off,U16 n_params,
                                              U32* locs,U32& param_slen ) {

  TCI_BOOL rv  =  TRUE;
  error_flag  =  0;

  U32 param_start =  src_off;
  U16 param_num   =  0;
  while ( param_num < n_params ) {      // loop thru the parameters

  // read the token(s) in this parameter value bucket

    U16 nest_level  =  0;
    TCI_BOOL done    =  FALSE;
    TCI_BOOL first   =  TRUE;
    while ( !done ) {

      U8* ztmpl;              // a dummy
      TNODE* nt =  GetTNODE( pENV,src_off,&ztmpl );

      if ( nt ) {
        U16 t_len =  nt->src_offset2 - nt->src_offset1;
        src_off   +=  nt->src_offset1;

        if ( first ) {

          if ( !strcmp((char*)nt->zuID,"6.1.0") ) {        // "{"
            src_off +=  t_len;
            locs[ 2*param_num]  =  src_off;
            nest_level++;
            first =  FALSE;

          } else if ( !strcmp((char*)nt->zuID,"9.1.10") ) {     // "{}"
            locs[ 2*param_num]      =  src_off + 1;
            locs[ 2*param_num + 1]  =  src_off + 1;
            src_off +=  t_len;
            DisposeTNode( nt );
            break;

          } else {                      // this is an ERROR condition
            rv  =  FALSE;
            param_num =  n_params;
            error_flag  =  1;           // Parameter expected
            DisposeTNode( nt );
            break;
          }

        } else if ( !strcmp((char*)nt->zuID,"6.1.0") ) {        // "{"
          src_off +=  t_len;
          if ( !nest_level )
            locs[ 2*param_num]  =  src_off;
          nest_level++;

        } else if ( !strcmp((char*)nt->zuID,"6.1.1") ) {        // "}"

          if ( nest_level > 1 ) {
            nest_level--;
          } else {                      // hit end of param value
            locs[ 2*param_num + 1]  =  src_off;
            done  =  TRUE;
          }
          src_off +=  t_len;

        } else {        // step over non-semantically-relevent token

          src_off +=  t_len;
        }

        DisposeTNode( nt );

      } else {          // unexpected end of input

        rv  =  FALSE;
        param_num =  n_params;
        error_flag  =  2;
        break;
      }

    }           // while loop thru tokens in TVERBATIM


    param_num++;

  }     // loop thru parameters

  param_slen  =  src_off - param_start;

  return rv;
}


// We have a macro definition that contains parameters markers.
//  \newcommand{\good}[3]{{#1}$({#2};{#3})$}
//
// There is a call to this macro in the source buffer.
//  \good{\em gnus\/}{x}{54}
//        ^         ^ ^^ ^ ^
// The positions (in the source buffer) of the parameter values have been
//  recorded in "locs".
//
// The following function builds a zstr that holds the value of this macro
//  call after it is expanded and the params have been substituted.

          
U8* Tokizer::BuildMacroInstance( U8* macro_def,U16 n_params,U32* locs ) {

  U8* rv  =  NULL;

  U32 ba_limit  =  512;
  U8* ba_ptr  =  (U8*)TCI_NEW( char[ba_limit] );
  U32 ba_i  =  0;

  U8  lit_buf[256];     // buffer to hold literals as we go thru the macro def
  U16 lbi =  0;

  U8* ptr =  macro_def;
  while ( *ptr ) {      // loop thru bytes in the macro definition

    if        ( *ptr == '\\' ) {        
                                        // the TeX escape char handled here.
      lit_buf[ lbi ]  =  *ptr;          // We move the character modified
      lbi++;                            // into lit_buf along with the
      ptr++;                            // escape char.
      if ( *ptr ) {
        lit_buf[ lbi ]  =  *ptr;
        lbi++;
        ptr++;
      }

    } else if ( *ptr == '#' ) {         // possible parameter marker

      ptr++;

      if ( *ptr >= '0' && *ptr <= '9' ) {       // parameter ID 1-9

        if ( lbi ) {                    // add preceding literal bytes
          lit_buf[ lbi ]  =  0;         //  to ba.
          ba_ptr  =  AppendBytesToBuffer( ba_ptr,ba_limit,
                                           ba_i,lit_buf,lbi );
          lbi =  0;
        }

        U16 p_ID  =  *ptr - '1';        // add the param value to ba.
        ptr++;
        U32 p_loc =  locs[ 2*p_ID ];
        if ( p_loc >= buf_start_offset && p_loc < buf_lim ) {
          U8* param_val =  buf_ptr + p_loc - buf_start_offset;

          U32 end_loc =  locs[ 2*p_ID+1 ];
          if ( end_loc >= buf_start_offset && end_loc < buf_lim ) {
            U8* end_p =  buf_ptr + end_loc - buf_start_offset;
            U8 save =  *end_p;
            *end_p  =  0;
            //ba  +=  param_val;
			U16 zp_ln =  strlen( (char*)param_val );
            ba_ptr  =  AppendBytesToBuffer( ba_ptr,ba_limit,ba_i,
												param_val,zp_ln );
            *end_p  =  save;

          } else {
            TCI_ASSERT( 0 );
			delete ba_ptr;
            return rv;
          }

        } else {

          TCI_ASSERT( 0 );
		  delete ba_ptr;
          return rv;
        }

      } else {          // I'm treating this '#' as a literal here!

        lit_buf[ lbi ]  =  '#';
        lbi++;

      }

    } else {            // All else is a literal - add to lit_buf

      lit_buf[ lbi ]  =  *ptr;
      lbi++;
      ptr++;
    }

  }     // while loop thru macro definition

  // add the tail of the macro value to ba

  if ( lbi ) {
    lit_buf[ lbi ]  =  0;
    ba_ptr  =  AppendBytesToBuffer( ba_ptr,ba_limit,ba_i,lit_buf,lbi );
  }

// Transfer the completely expanded macro value to "rv"

  rv  =  (U8*)TCI_NEW(char[ ba_i+1 ] );
  strcpy( (char*)rv,(char*)ba_ptr );
  delete ba_ptr;

/*
JBMLine( "BuildMacroInstance rv=\n" );
JBMLine( (char*)rv );
JBMLine( "\n" );
*/

  return rv;
}


void Tokizer::AddNewTheorem( U16 scope,U8* env_nom,
								U8* print_nom,U8* n_like,U8* in,
									TCI_BOOL isstarred ) {
/*
char zzz[256];
sprintf( zzz,"Tokizer::AddNewTheorem, env_nom=%s, print_nom=%s\n",
env_nom,print_nom );
JBMLine( zzz );
*/

  if ( nobject_store )
    nobject_store->AddNewTheorem( scope,env_nom,print_nom,
    									n_like,in,isstarred );
}


void Tokizer::AddNewEnviron( U16 scope,U8* env_nom,
								U8* n_args,U8* opt_def,
									U8* begdef,U8* enddef ) {
/*
char zzz[256];
sprintf( zzz,"Tokizer::AddNewEnviron, env_nom=%s, print_nom=%s\n",
env_nom,print_nom );
JBMLine( zzz );
*/

  if ( nobject_store )
    nobject_store->AddNewEnviron( scope,env_nom,n_args,opt_def,begdef,enddef );
}


// Tokizer has a store of newtheorem definitions.  This function checks
//  that store to see if "token" is defined there.  If token does begin
//  or end a defined newtheorem, the function returns a zuID and a template
//  ( Tokizer production ) to the caller.

TCI_BOOL Tokizer::GetObjectDataFromName( U8* zcurr_env,
											U8* token,U16 ln,
                                              U8** d_zuID,
                                                U8** d_ztemplate ) {
//JBMLine( "Tokizer::GetObjectDataFromName\n" );

  TCI_BOOL rv  =  FALSE;

  if ( strcmp((char*)zcurr_env,"TEXT")
  &&   strcmp((char*)zcurr_env,"MATH") )
    return rv;

  if ( nobject_store ) {

  // "token" is not defined in the source .gmr AND it's not
  //  a macro call - it may be an instance of newtheorem or newenv.

    U8  env_nom[80];
    U16 n_off;
    U16 cmd_type  =  0;
    if        ( ln>8 && !strncmp((char*)token,"\\begin{",7) ) {
      cmd_type  =  1;
      n_off =  7;
    } else if ( ln>6 && !strncmp((char*)token,"\\end{",5) ) {
      cmd_type  =  2;
      n_off =  5;
    } else if ( ln>4 && !strncmp((char*)token,"\\the",4) ) {
      cmd_type  =  3;
      n_off =  4;
    }

    if ( cmd_type ) {
    
  // token = "\begin{...}" or "\end{...}" or "\thepage"

      U16 nln =  ln - n_off;
      TCI_ASSERT( nln < 80 );
      strncpy( (char*)env_nom,(char*)token+n_off,nln );
      env_nom[nln]  =  0;

      U8* ptr =  (U8*)strchr( (char*)env_nom,'}' );
      if ( ptr )
        *ptr  =  0;
      else if ( cmd_type != 3 )
        TCI_ASSERT(0);
/*
JBMLine( "env_nom=" );
JBMLine( (char*)env_nom );
JBMLine( "\n" );
*/

      if ( cmd_type == 3 ) {
        if ( nobject_store->HasCounter(env_nom) ) {
          *d_zuID =  (U8*)"5.366.0";		// \thepage<uID5.366.0>
          *d_ztemplate  =  NULL;
          rv  =  TRUE;
		} else
		  TCI_ASSERT(0);

	  } else {
        TCI_BOOL is_theorem =  TRUE;
        U8* object_tmpl =  nobject_store->GetTheoremInfo( env_nom,TRI_tmpl );
	    if ( !object_tmpl ) {
          object_tmpl =  nobject_store->GetEnvironInfo( env_nom,ERI_tmpl );
          is_theorem  =  FALSE;
	    }

        if ( object_tmpl ) {    	// this token starts/ends a new env
/*
if ( !is_theorem ) {
JBMLine( "object_tmpl=" );
JBMLine( (char*)object_tmpl );
JBMLine( "\n" );
}
*/
          if        ( cmd_type==1 ) {     // begin
		    if ( is_theorem )
              *d_zuID =  (U8*)"5.329.0";
		    else
              *d_zuID =  (U8*)"5.529.0";
            *d_ztemplate  =  object_tmpl;
            rv  =  TRUE;

          } else if ( cmd_type==2 ) {     // end
		    if ( is_theorem )
              *d_zuID =  (U8*)"5.329.1";
		    else
              *d_zuID =  (U8*)"5.529.11";
            *d_ztemplate  =  NULL;
            rv  =  TRUE;

          } else
            TCI_ASSERT(0);
        }             // if ( theorem_tmpl )

	  }			// newtheorem or newenvironment clause

    }       // if ( cmd_type )

  }     // if ( nobject_store )

  return rv;
}


void Tokizer::SetObjectStore( NewObjectStore* new_object_store ) {
//JBMLine( "Tokizer sets NewObjectStore\n" );

  nobject_store =  new_object_store;
}

/*
// ">  <mn name1="val1" name2="val2" >
//		  ^							 ^
//		 src					  src+ln

ATTRIB_REC* Tokizer::MakeATTRIBList( U8* src,U16 ln ) {

  ATTRIB_REC* rv  =  NULL;
  ATTRIB_REC* tail;

  U8 save =  *(src+ln);			// 0 terminator for str functions
  *(src+ln) =  0;

  U16 bytesdone =  0;
  while ( bytesdone < ln ) {

    U16 advance =  0;
    ATTRIB_REC* na  =  GetNextATTRIB( src+bytesdone,ln-bytesdone,advance );
    if ( na ) {

//char zzz[256];
//sprintf( zzz,"aname=%s, avalue=%s\n",na->attr_name,na->attr_val );
//JBMLine( zzz );

	  bytesdone +=  advance;
	  if ( rv ) {
		na->prev    =  tail;
		tail->next  =  na;
      } else 
		rv  =  na;
	  tail  =  na;
    } else
	  break;
  }

  *(src+ln) =  save;
  return rv;
}

// src must be 0 terminated
// " name  =  'value'  name1..."
ATTRIB_REC* Tokizer::GetNextATTRIB( U8* src,U16 ln,U16& bytesdone ) {

  ATTRIB_REC* rv  =  NULL;

  U8* ptr =  src;
  while ( *ptr && *ptr <= ' ' ) ptr++;

  if ( *ptr && *ptr>='a' && *ptr<='z' ) {	// at name start
	U8* name  =  ptr;
	U16 nln   =  0;
    while ( *ptr && *ptr>='a' && *ptr<='z' ) { nln++; ptr++; }
    while ( *ptr && *ptr <= ' ' ) ptr++;

    if ( *ptr && *ptr=='=' ) {				// at =
	  ptr++;
      while ( *ptr && *ptr <= ' ' ) ptr++;
      if ( *ptr && (*ptr=='"' || *ptr=='\'') ) {	// at start of value
		U8 ender  =  *ptr;
		ptr++;
		U8* svalue  =  ptr;
		U16 vln =  0;
        while ( *ptr && *ptr!=ender ) { vln++; ptr++; }
        if ( *ptr )
          rv  =  MakeATTRIBNode( name,nln,svalue,vln,0 );
      }
    }
  }

  bytesdone =  ptr - src + 1;

  return rv;
}
*/


TCI_BOOL Tokizer::AtomicElement( U8* zuID ) {

  TCI_BOOL rv  =  FALSE;

  U16 uobjtype,usubtype,uID;
  GetUids( zuID,uobjtype,usubtype,uID );
  if ( uobjtype==3 )
    if ( usubtype==201
    ||   usubtype==202
    ||   usubtype==203
    ||   usubtype==204
    ||   usubtype==206 )
	  rv  =  TRUE;
  return rv;
}

// <mi> <malignmark/> x </mi>
//	^					   ^

U8*	Tokizer::GetVarValue( U8* tok_ptr,U16 lim,U16& vln ) {

  U8* rv  =  NULL;
  vln =  0;

  U8 save =  *(tok_ptr+lim);
  *(tok_ptr+lim)  =  0;

  char* p1  =  strchr((char*)tok_ptr,'>');
  if ( p1 ) {
	p1++;
    char* p2  =  strstr( p1,"</" );
    if ( p2 ) {
      p2--;
	  while ( *p1<=' ' && p1<p2 ) p1++;
	  while ( *p2<=' ' && p2>p1 ) p2--;
	  if ( p2 >= p1 ) {
	    vln =  p2-p1+1;
        rv  =  (U8*)TCI_NEW( char[ vln+1 ] );
        strncpy( (char*)rv,(char*)p1,vln );
        rv[vln] =  0;
      }
    }
  }			//  if ( p1 == '>')

  *(tok_ptr+lim)  =  save;
/*
if ( rv) {
JBMLine( "GetVarValue returns " );
JBMLine( (char*)rv );
JBMLine( "\n" );
}
*/
  return rv;
}


/* obj_def format is as follows:
+<uID3.13.204,in>infix,28,lspace=".22222em" rspace=".22222em"
*/

TCI_BOOL Tokizer::IsFormMatch( U8* req_form,U8* obj_def ) {

  TCI_BOOL rv  =  TRUE;
  if ( req_form ) {
    U16 zln =  strlen( (char*)req_form );
    if ( strncmp((char*)obj_def,(char*)req_form,zln) )
	  rv  =  FALSE;
  }

  return rv;
}


U16 Tokizer::SetInputMode( U16 new_mode ) {
/*
char zzz[80];
sprintf( zzz,"Tokizer::SetMode, old=%d, new=%d\n",input_mode,new_mode );
JBMLine( zzz );
*/

  U16 rv  =  input_mode;
  input_mode =  new_mode;
  tokenizer->SetInputMode( new_mode );
  return rv;
}


TNODE* Tokizer::GetComments() {

  TNODE* rv =  comment_buffer;
  comment_buffer  =  NULL;
  return rv;
}

// In the process of acquiring a token, comments that occur in front
//  of the token ( and possibly within a compound token ) may be 
//  encountered.  Each such comment is placed in a TNODE.  These
//  TNODEs are passed off to the following function, which buffers
//  the comment.  The consumer of tokens, Parser, asks for the
//  buffered comments at points where it is convenient to handle them.

void Tokizer::AppendComments( TNODE* new_comment ) {

  // If comment_buffer is empty, and new_comment is empty
  //  we just dispose new_comment.  LaTeX no-space line end trick.

  if ( !comment_buffer && new_comment ) {

    TCI_BOOL is_empty =  TRUE;
    CVLINE_REC* cv_rover  =  new_comment->cv_list;
	while ( cv_rover && is_empty ) {
      U8* cline =  cv_rover->cvline;
	  if ( cline ) {
	    if ( *cline == '%' ) {
		  cline++;
		  while ( *cline ) {
		    if ( *cline > ' ' ) {
			  is_empty  =  FALSE;
			  break;
			}
			cline++;
		  }
        } else {
		  TCI_ASSERT(0);
		}
	  }		// if ( cline )

	  cv_rover  =  cv_rover->next;
	}		// while ( cv_rover && is_empty )

	if ( is_empty ) {
	  DisposeTNode( new_comment );
	  return;
	}
  }

  if ( comment_buffer ) {
    if ( new_comment ) {
	  comment_buffer->cv_list =  JoinCVLists( comment_buffer->cv_list,
	     										new_comment->cv_list );
	  new_comment->cv_list  =  NULL;
      DisposeTNode( new_comment );
	}

  } else
    comment_buffer  =  new_comment;
}


void Tokizer::SetTheoremStyleAttr( U16 which_attr,U8* new_val ) {

  if ( nobject_store )
    nobject_store->SetTheoremStyleAttr( which_attr,new_val );
}

void Tokizer::EnterExitGroup( TCI_BOOL do_enter ) {

  if ( nobject_store )
    nobject_store->EnterExitGroup( do_enter );
}

// We've located a token and input_mode == IM_TEXMETRIC.
//  We assign a uID to the token here.

U8* Tokizer::GetTeXMetricuID( U8* ptr,U16 tln,U8** ztmpl ) {

  U8* rv  =  NULL;
  char ch =  *ptr;

  TCI_BOOL is_delimiter =  FALSE;
  TCI_BOOL is_operator  =  FALSE;
  TCI_BOOL is_number  =  FALSE;
  TCI_BOOL is_unit    =  FALSE;
  TCI_BOOL is_name    =  FALSE;
  TCI_BOOL is_comment =  FALSE;
  TCI_BOOL is_true    =  FALSE;

  if        ( strchr("[]{};",ch) ) {
    is_delimiter =  TRUE;
  } else if ( strchr("0123456789.",ch) ) {
    is_number    =  TRUE;
  } else if ( strchr("+-",ch) ) {
    if ( tln > 1 )	is_number    =  TRUE;
	else			is_operator  =  TRUE;
  } else if ( ch=='\\' ) {
    is_name  =  TRUE;
  } else if ( ch=='%' ) {
    is_comment  =  TRUE;
  } else if ( ch=='t' ) {
    is_true  =  TRUE;
  } else if ( strchr("ceipm",ch) ) {
    if ( tln == 2 ) is_unit =  TRUE;
	else			TCI_ASSERT(0);
  }

  if ( is_delimiter || is_operator ) {
    U8* zcurr_env =  (U8*)"MATH";
    U8* zuID;
    if ( GGDFromNameAndAttrs(ptr,tln,NULL,
   										zcurr_env,&zuID,ztmpl) ) {
      rv  =  zuID;
	} else
      TCI_ASSERT(0);
  } else if ( is_number ) {
    rv  =  (U8*)"50.1.1";
  } else if ( is_unit ) {
    rv  =  (U8*)"50.1.2";
  } else if ( is_name ) {
    rv  =  (U8*)"50.1.3";
  } else if ( is_true ) {
    rv  =  (U8*)"50.1.4";
  } else if ( is_comment ) {
    rv  =  (U8*)"777.0.0";
  } else
    TCI_ASSERT(0);

  return rv;
}


void Tokizer::SaveInputBuffer() {

  // Save variables that constitute clear input buffer state

  sv_buf_ptr		  =  buf_ptr;
  sv_buf_start_offset =  buf_start_offset;
  sv_buf_lim		  =  buf_lim;
  sv_curr_line_num	  =  curr_line_num;
  sv_linenum_list	  =  linenum_list;
  sv_linenum_last	  =  linenum_last;
  sv_input_mode		  =  input_mode;
  sv_comment_buffer	  =  comment_buffer;

  // Clear variables that constitute clear input buffer state

  buf_ptr		    =  NULL;
  buf_start_offset  =  0;
  buf_lim		    =  0;
  curr_line_num	    =  0;
  linenum_list	    =  NULL;
  linenum_last	    =  NULL;
  input_mode		=  0;
  comment_buffer	=  NULL;
}


void Tokizer::RestoreInputBuffer() {

// Dispose variables that constitute current input buffer state

  if ( buf_ptr )
    delete buf_ptr;

  LINENUM_REC* rover  =  linenum_list;
  while ( rover ) {
    LINENUM_REC* del  =  rover;
    rover =  rover->next;
    delete del;
  }

  if ( comment_buffer ) 
    DisposeTNode( comment_buffer );

// Restore previous input buffer state

  buf_ptr		    =  sv_buf_ptr;
  buf_start_offset  =  sv_buf_start_offset;
  buf_lim		    =  sv_buf_lim;
  curr_line_num	    =  sv_curr_line_num;
  linenum_list	    =  sv_linenum_list;
  linenum_last	    =  sv_linenum_last;
  input_mode		=  sv_input_mode;
  comment_buffer	=  sv_comment_buffer;

  BA_input  =  FALSE;
}



TCI_BOOL Tokizer::GGDFromNameAndAttrs( U8* token,U16 ln,
										U8* zform,
                                          U8* zcurr_env,
                                            U8** d_zuID,
                                              U8** d_ztemplate ) {

  TCI_BOOL rv  =  FALSE;

  rv  =  s_grammar->GetGrammarDataFromNameAndAttrs( token,ln,
							zform,zcurr_env,d_zuID,d_ztemplate );

  if ( !rv && nobject_store ) {
    rv  =  GetObjectDataFromName( zcurr_env,token,ln,d_zuID,d_ztemplate );
  }

  return  rv;
}


// \mathnom{\cmd}{visual_def}

U8* Tokizer::MakeMathNomTmpl( U8* token,U8* visual_def,bool is_starred ) {

  U16 tlen  =  strlen( (char*)token );
  U16 dlen  =  strlen( (char*)visual_def );

  U8* rv  =  (U8*)TCI_NEW( char[tlen+dlen+14] );

// \mathnom{ tok_buffer }{ visual_def }

  strcpy( (char*)rv,"\\mathnom" );
  if ( is_starred )
    strcat( (char*)rv,"*" );
  strcat( (char*)rv,"{" );
  strcat( (char*)rv,(char*)token );
  strcat( (char*)rv,"}{" );
  strcat( (char*)rv,(char*)visual_def );
  strcat( (char*)rv,"}" );

  return  rv;
}

