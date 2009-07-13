
// This module holds the FILE* for the .log file.
//  It generates and scripts all messages written to the log file.

#include "logfiler.h"
#include "fltutils.h"

#include <string.h>
#include <cstdio>
#include <stdlib.h>

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ctor

LogFiler::LogFiler( const char* log_file_spec ) {
//JBMLine( "LogFiler_ctor\n" );

  if ( log_file_spec )
    strcpy( logfile_spec,log_file_spec );
  else
    logfile_spec[0] =  0;

// We count the number of messages generated in a filter run.
//  At the end of the run, a message box is displayed if any
//  messages were logged.

  src_logmsg_count  =  0;
  dst_logmsg_count  =  0;

// During program development, we keep track of messages for the log
//  file as they are generated and as they are delivered.  If the
//  numbers don't match at the end, we need to account for the
//  missing messages.  They can go missing in functions that neglect
//  to pass them along as layouts are processed.

  n_msgs_generated  =  0;
  n_msgs_delivered  =  0;
}

// dtor

LogFiler::~LogFiler() {
//JBMLine( "LogFiler_dtor\n" );

  TCI_ASSERT( n_msgs_generated == n_msgs_delivered );
}

// Lookup table for messages generated while parsing source LaTeX

const char* LogFiler::IDtoSrcMsg( U16 msgID ) {

  const char* msg;
  switch ( msgID ) {
    case BAD_TMPL_SYNTAX        :
      msg =  "Bad syntax for a template element";		break;
    case UNKNOWN_TMPL_ELEMENT   :
      msg =  "Unknown template element encountered";		break;
    case MISSING_TMPL_LITERAL   :
      msg =  "Literal expected";				break;
    case UNDEFINED_TOKEN        :
      msg =  "Undefined token encountered";			break;
    case MISSING_RUN_STARTER    :
      msg =  "Run starting literal expected";		        break;
    case UNEXPECTED_ENDOFRUN    :
      msg =  "Unexpected end of delimited run";			break;
    case UNEXPECTED_ENDOFINPUT  :
      msg =  "Unexpected end of input";				break;
    case MISSING_BUCKET         :
      msg =  "Missing bucket object";				break;
    case MISSING_VAR            :
      msg =  "Missing variable object";			        break;
    case MISSING_LIST_COUNTER   :
      msg =  "Missing list counter object";			break;
    case PARAM_EXPECTED         :
      msg =  "Parameter expected";			        break;
    case BAD_PARAM_LIST         :
      msg =  "Bad parameter list";				break;
    case UNEXPECTED_ENDOFGROUP  :
      msg =  "Unexpected token";			        break;
    default :
      msg =  (char*)NULL;				        break;
  }             //  switch ( msgID )

  return msg;
}

#define ZLINE_LEN       128

// During the parsing of LaTeX source, we have encountered
//  an error condition.  The following function forms
//  the message to be logged, and writes it logfile.
// Sometimes we don't have a source line number to report.
//  Hence, if "linenum" is 0, it is not displayed.

void LogFiler::LogSrcError( U16 msgID,U8* zhint,
							U8* bad_src,U16 bad_off,U32 linenum ) {
//JBMLine( "DLL_LogFiler::LogSrcError\n" );

  char* buff  =  TCI_NEW( char[512] );
  const char* msg   =  IDtoSrcMsg( msgID );

  if ( msg ) {
    if ( zhint ) {
	  if ( linenum )
        sprintf( buff,"\nline %lu, %s: %s\n",linenum,msg,zhint );
	  else
        sprintf( buff,"\n%s: %s\n",msg,zhint );
    } else {
	  if ( linenum )
        sprintf( buff,"\nline %lu, %s\n",linenum,msg );
	  else
        sprintf( buff,"\n%s\n",msg );
    }
    if ( logfile_spec[0] ) {
      src_logmsg_count++;
      LineToLogFile( buff,logfile_spec );
    } else {
	  TCI_ASSERT(0);
      DumpLine( buff );
	}

    if ( bad_src ) {    
      U16 bad_src_len =  strlen( (char*)bad_src );

      char line[ZLINE_LEN];
      U16 li    =  0;
      U16 s_off =  0;
      U16 s_lim =  bad_off;
      if ( bad_off>60 ) {
        s_off =  bad_off-40;
        s_lim =  40;
        strcpy( line,"..." );
        li  =  3;
      }
      strncpy( line+li,(char*)bad_src+s_off,s_lim );
      U16 cut_off =  li + s_lim;
      line[ cut_off ]   =  '\n';
      line[ cut_off+1 ] =  0;
      if ( logfile_spec[0] ) {
        LineToLogFile( line,logfile_spec );
      } else
        DumpLine( line );

      if ( bad_src_len > bad_off ) {
        memset( line,' ',cut_off );
      
        U16 tail_len  =  bad_src_len - bad_off;
        if ( cut_off + tail_len + 2 > ZLINE_LEN )
          tail_len  =  ZLINE_LEN - cut_off - 2;
      
        strncpy( line+cut_off,(char*)bad_src+bad_off,tail_len );
        cut_off +=  tail_len;
        line[ cut_off++ ] =  '\n';
        line[ cut_off   ] =  0;
        if ( logfile_spec[0] ) {
          LineToLogFile( line,logfile_spec );
        } else
          DumpLine( line );
      }

    }   //  if ( bad_src )

  }     //  if ( msg )

  delete buff;
}


// Form an output phase message and write it in the log file.

void LogFiler::MsgToLog( LOG_MSG_REC* msg_rec,
								U32 dstfile_line_num ) {

  char line[256];

  char* src =  (char*)msg_rec->msg;

  if ( src ) {          // we have text for the message
    strcpy( line,src );
    char number[32];
    char* target;

  // Put the name of the source TeX token in the message

    if ( msg_rec->ztoken ) {
      target  =  "%SRC_TOKN";
      while ( strstr(line,target) )
        StrReplace( line,target,(char*)msg_rec->ztoken );
    }

  // Put the source TeX line number in the message

  // jcs ultoa not a standard function:  ultoa( msg_rec->src_lineno,number,10 );
   #ifdef _WINDOWS
    _snprintf(number,  10, "%ld",  msg_rec->src_lineno);
   #else
    snprintf(number,  10, "%ld",  msg_rec->src_lineno);
   #endif

    target  =  "%SRC_LINE";
    while ( strstr(line,target) )
      StrReplace( line,target,number );

  // Put the approximate out file line number in the message

    U32 dst_line_num  =  dstfile_line_num + 1;

/*
char xxx[80];
sprintf( xxx,"dst_line_num=%lu, msg:dst_lineno=%lu, dstfile_line_num=%lu\n",
                        dst_line_num,msg->dst_lineno,dstfile_line_num );
JBMLine( xxx );
*/

    
  // jcs ultoa not a standard function:  ultoa( dst_line_num,number,10 );
    #ifdef _WINDOWS
    _snprintf(number,  10, "%ld",  dst_line_num);
    #else
    snprintf(number,  10, "%ld",  msg_rec->src_lineno);
    #endif


    target  =  "%DST_LINE";
    while ( strstr(line,target) )
      StrReplace( line,target,number );

    strcat( line,"\n" );

    if ( logfile_spec[0] ) {
      dst_logmsg_count++;
      
	  LineToLogFile( line,logfile_spec );
    } else
      DumpLine( (char*)line );

  }             // if ( src )

  n_msgs_delivered++;
}

// Function to create a message record, Nemeth generation phase.

LOG_MSG_REC* LogFiler::MakeLogMsg( U8* intro,U8* msg,U8* suffix,
											U8* src_tok,U32 src_line ) {

  LOG_MSG_REC* node =  (LOG_MSG_REC*)TCI_NEW( char[ sizeof(LOG_MSG_REC) ] );
  node->next  =  NULL;
  node->src_lineno  =  src_line;
  node->dst_lineno  =  0L;
  if ( src_tok )
    strcpy( (char*)node->ztoken,(char*)src_tok );
  else
    node->ztoken[0] =  0;
  if ( msg ) {
    U16 zln =  strlen( (char*)msg ) + 1;
    if ( intro )
      zln  +=  strlen( (char*)intro );
    if ( suffix )
      zln  +=  strlen( (char*)suffix );
    U8* tmp =  (U8*)TCI_NEW( char[ zln ] );
	*tmp  =  0;
    if ( intro )
      strcpy( (char*)tmp,(char*)intro );
    strcat( (char*)tmp,(char*)msg );
    if ( suffix )
      strcat( (char*)tmp,(char*)suffix );
    node->msg =  tmp;
  } else
    node->msg =  NULL;

  n_msgs_generated++;

  return node;
}

// Report the number of messages logged in the current LogFiler

U16 LogFiler::GetLogMsgCount() {

  return src_logmsg_count + dst_logmsg_count;
}


// Append a list of msgs to the end of a second list

LOG_MSG_REC* LogFiler::AppendMsgs( LOG_MSG_REC* list_to_add,
  								   	  LOG_MSG_REC* targ_list ) {

  LOG_MSG_REC* rv;

  if ( targ_list ) {			// locate last node in targ_list
	rv  =  targ_list;
    LOG_MSG_REC* rover  =  targ_list;
    while ( rover->next )
      rover =  rover->next;
    rover->next =  list_to_add;
  } else {
	rv  =  list_to_add;
  }

  return rv;
}


// Lookup table for messages generated during output phase.

U8* LogFiler::GetMsgStr( U16 msg_ID ) {

  U8* rv  =  NULL;

  switch ( msg_ID ) {

    case MSG_ID1	:
      rv  =  (U8*)"Multiline lower limit/subscript not implemented";
	break;
    case MSG_ID2	:
      rv  =  (U8*)"Multiline upper limit/superscript not implemented";
	break;
    case MSG_ID3	:
      rv  =  (U8*)"MathML for TeX big operator unknown";
	break;
    case MSG_ID4	:
      rv  =  (U8*)"No MathML entity for LaTeX symbol";
	break;
    case MSG_ID5	:
      rv  =  (U8*)"Lookup failed, MathML.gmr, fence object";
	break;
    case MSG_ID6	:
      rv  =  (U8*)"MathML not yet implemented for LaTeX decoration";
	break;
    case MSG_ID7	:
      rv  =  (U8*)"";
	break;
    case MSG_ID8	:
      rv  =  (U8*)"";
	break;
    case MSG_ID9	:
      rv  =  (U8*)"";
	break;
    case MSG_ID10	:
      rv  =  (U8*)"";
	break;
    case MSG_ID11	:
      rv  =  (U8*)"";
	break;
    case MSG_ID12	:
      rv  =  (U8*)"";
	break;
    case MSG_ID13	:
      rv  =  (U8*)"";
	break;
    case MSG_ID14	:
      rv  =  (U8*)"";
	break;
    case MSG_ID15	:
      rv  =  (U8*)"";
	break;
    case MSG_ID16	:
      rv  =  (U8*)"";
	break;
    case MSG_ID17	:
      rv  =  (U8*)"";
	break;
    case MSG_ID18	:
      rv  =  (U8*)"";
	break;
    case MSG_ID19	:
      rv  =  (U8*)"";
	break;
    case MSG_ID20	:
      rv  =  (U8*)"";
	break;
    case MSG_ID21	:
      rv  =  (U8*)"";
	break;
    case MSG_ID22	:
      rv  =  (U8*)"";
	break;
    case MSG_ID23	:
      rv  =  (U8*)"";
	break;
    case MSG_ID24	:
      rv  =  (U8*)"";
	break;
    case MSG_ID25	:
      rv  =  (U8*)"";
	break;
    case MSG_ID26	:
      rv  =  (U8*)"";
	break;
    case MSG_ID27	:
      rv  =  (U8*)"";
    break;
    case MSG_ID28  :
      rv  =  (U8*)"";
    break;
    case MSG_ID29  :
      rv  =  (U8*)"";
    break;
    case MSG_ID30  :
      rv  =  (U8*)"";
    break;
    case MSG_ID31  :
      rv  =  (U8*)"";
    break;
    case MSG_ID32  :
      rv  =  (U8*)"";
    break;
    case MSG_ID33  :
      rv  =  (U8*)"";
    break;
    case MSG_ID34  :
      rv  =  (U8*)"";
    break;
    case MSG_ID35  :
      rv  =  (U8*)"";
    break;
    case MSG_ID36 :
      rv  =  (U8*)"";
    break;
    case MSG_ID37  :
      rv  =  (U8*)"";
    break;
    case MSG_ID38  :
      rv  =  (U8*)"";
    break;
    case MSG_ID39  :
      rv  =  (U8*)"";
    break;
    case MSG_ID40  :
      rv  =  (U8*)"No LaTeX symbol for MathML operator";
    break;
    case MSG_ID41 :
      rv  =  (U8*)"No LaTeX for MathML operator";
    break;
    case MSG_ID42 :
      rv  =  (U8*)"";
    break;
    case MSG_ID43 :
      rv  =  (U8*)"";
    break;

	default :  break;
  }

  return rv;
}


U8* LogFiler::GetSuffixStr( U16 suffix_ID ) {

  U8* rv  =  NULL;

  switch ( suffix_ID ) {
    case SUFFIX_ID1	:
      rv  =  (U8*)" (out~%DST_LINE).";								break;
    case SUFFIX_ID2	:
      rv  = (U8*)": %SRC_TOKN (tex@%SRC_LINE,out~%DST_LINE ).";		break;
    default :  break;
  }

  return rv;
}

// Generate an output phase message record, and append it to a list.

LOG_MSG_REC* LogFiler::AppendLogMsg( LOG_MSG_REC* msg_list,
									  U16 msg_ID,U16 suffix_ID,
										U8* src_tok,U32 src_line ) {

  U8* msg     =  GetMsgStr( msg_ID );
  U8* suffix  =  GetSuffixStr( suffix_ID );;

  LOG_MSG_REC* new_msg  =  MakeLogMsg( NULL,msg,suffix,src_tok,src_line );
  return AppendMsgs( new_msg,msg_list );
}


void LogFiler::LineToLogFile( char* line,char* logfile_spec ) {
//JBMLine( "DLL_LogFiler::LineToLogFile\n" );

  FILE* fp =  fopen( logfile_spec,"a" );
  if ( fp ) {
    fputs( line,fp );
    fclose( fp );
  } else {
//JBMLine( "  a open failed\n" );
    FILE* fp =  fopen( logfile_spec,"wt" );
    if ( fp ) {
      fputs( line,fp );
      fclose( fp );
    } else {
//JBMLine( "  wt open failed\n" );
    }
  }
}

