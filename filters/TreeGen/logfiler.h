
#ifndef LogFiler_h
#define LogFiler_h

/*  Throughout the filtering process, circumstances may arise where
  the filter cannot continue (bad source LaTeX) or cannot produce
  correct Nemeth code (LaTeX and Nemeth aren't ono-to-one).
  The filter notifies the user by writing messages to a log file.
  Each run of the filter has an associated log file.
  This object manages the log file.
    Message processing is a bit complex, especially during the Nemeth
  generating phase.  After a message is produced, it may be necessary
  to pass it along thru the processing until final output is scripted.
  That way we can tell the user where the questionable output is
  located.
    Note that a new LogFiler is created for each filtering task.
  The actual text of all messages is located in this module.
*/


#include "fltutils.h"
#include <stdio.h>

// Clients of this object refer to messages using the following IDs.
//   Identifiers for source error messages

#define BAD_TMPL_SYNTAX         1
#define UNKNOWN_TMPL_ELEMENT    2
#define MISSING_TMPL_LITERAL    3
#define UNDEFINED_TOKEN         4
#define MISSING_RUN_STARTER     5
#define UNEXPECTED_ENDOFRUN     6
#define UNEXPECTED_ENDOFINPUT   7
#define MISSING_BUCKET          8
#define MISSING_VAR             9
#define MISSING_LIST_COUNTER    10
#define PARAM_EXPECTED          11
#define BAD_PARAM_LIST          12
#define MISSING_RUN_ENDER       13
#define UNEXPECTED_ENDOFGROUP   14

// Identifiers for output phase error messages

#define MSG_ID1			1
#define MSG_ID2			2
#define MSG_ID3			3
#define MSG_ID4			4
#define MSG_ID5			5
#define MSG_ID6			6
#define MSG_ID7			7
#define MSG_ID8			8
#define MSG_ID9			9
#define MSG_ID10		10
#define MSG_ID11		11
#define MSG_ID12		12
#define MSG_ID13		13
#define MSG_ID14		14
#define MSG_ID15		15
#define MSG_ID16		16
#define MSG_ID17		17
#define MSG_ID18		18
#define MSG_ID19		19
#define MSG_ID20		20
#define MSG_ID21		21
#define MSG_ID22		22
#define MSG_ID23		23
#define MSG_ID24		24
#define MSG_ID25		25
#define MSG_ID26		26
#define MSG_ID27		27
#define MSG_ID28		28
#define MSG_ID29		29
#define MSG_ID30		30
#define MSG_ID31		31
#define MSG_ID32		32
#define MSG_ID33		33
#define MSG_ID34		34
#define MSG_ID35		35
#define MSG_ID36		36
#define MSG_ID37		37
#define MSG_ID38		38
#define MSG_ID39		39
#define MSG_ID40		40
#define MSG_ID41		41
#define MSG_ID42		42
#define MSG_ID43		43

#define SUFFIX_ID1		1
#define SUFFIX_ID2	    2

class LogFiler {

public:
  LogFiler( const char* log_file_spec );
  ~LogFiler();

  void          LogSrcError( U16 msgID,U8* zhint,U8* bad_src,
  									U16 bad_off,U32 linenum );

  LOG_MSG_REC*  AppendLogMsg( LOG_MSG_REC* msg_list,
							  U16 msg_ID,U16 suffix_ID,
								U8* src_tok,U32 src_line );
  LOG_MSG_REC*  AppendMsgs( LOG_MSG_REC* src_list,
								   LOG_MSG_REC* targ_list );
  void          MsgToLog( LOG_MSG_REC* msg,U32 dstfile_line_num );

  U16           GetLogMsgCount();

private:

  void          LineToLogFile( char* line,char* logfile_spec );
  char*		    IDtoSrcMsg( U16 msgID );
  LOG_MSG_REC*  MakeLogMsg( U8* intro,U8* msg,U8* suffix,
  									U8* src_tok,U32 src_line );
  U8* 			GetMsgStr( U16 msg_ID );
  U8* 			GetSuffixStr( U16 suffix_ID );

  char logfile_spec[256];

  U16 src_logmsg_count;
  U16 dst_logmsg_count;

  U16 n_msgs_generated;		// diagnostics
  U16 n_msgs_delivered;
};

#endif
