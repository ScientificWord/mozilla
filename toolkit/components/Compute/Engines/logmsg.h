#ifndef LOGMSG_H
#define LOGMSG_H

enum LogMsgID
{
  MSG_UNSUPPORTED_NUMBER,
  MSG_UNSUPPORTED_OPERATOR,
  MSG_UNDEFINED_FUNCTION,
  MSG_UNDEFINED_INVERSE,
  MSG_UNDEFINED_LIMFUNC,
  MSG_TEXT_IN_MATH
};

// struct to carry info for a message to be written to the log file
struct LOG_MSG_REC
{
  LOG_MSG_REC *next;
  char *msg;
};


void DisposeMsgs(LOG_MSG_REC* msg_list);
void RecordMsg(LOG_MSG_REC* &msg_list, LogMsgID id, const char* token);


#endif