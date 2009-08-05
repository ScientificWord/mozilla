#include "logmsg.h"

#include "cmptypes.h"
#include <cstring>
#include <stdio.h>



// message IDs and strings for development purposes only.
// final versions of strings will go in language dependent resources.

char *eMsgStrs[] = {
  "Unsupported number, %s\n",
  "Unsupported operator, %s\n",
  "Undefined function, %s\n",
  "No inverse defined for function, %s\n",
  "Undefined prefix operator with limits, %s\n",
  "Unexpected text in math, %s\n",
  0
};

// See enum LogMsgID in fltutils.h
LOG_MSG_REC* MakeLogMsg()
{
  LOG_MSG_REC *rv = new LOG_MSG_REC();
  rv->next = NULL;
  rv->msg = NULL;

  return rv;
}

void DisposeMsgs(LOG_MSG_REC * msg_list)
{
  LOG_MSG_REC *msg_rover = msg_list;
  while (msg_rover) {
    LOG_MSG_REC *del = msg_rover;
    msg_rover = msg_rover->next;
    delete[] del->msg;
    delete del;
  }
}

LOG_MSG_REC *AppendLogMsg(LOG_MSG_REC * msg_list, LOG_MSG_REC * new_msg_rec)
{
  if (!msg_list)
    return new_msg_rec;
  else {
    LOG_MSG_REC *rover = msg_list;
    while (rover->next)
      rover = rover->next;
    rover->next = new_msg_rec;
    return msg_list;
  }
}

void RecordMsg(LOG_MSG_REC * &msg_list, LogMsgID id, const char *token)
{
  char *msg_str = eMsgStrs[id];
  size_t zln = strlen(msg_str);
  if (token)
    zln += strlen(token);

  char *buffer = new char[zln];
  if (token)
    sprintf(buffer, msg_str, token);
  else
    strcpy(buffer, msg_str);

  LOG_MSG_REC *new_msg_rec = MakeLogMsg();
  new_msg_rec->msg = buffer;
  msg_list = AppendLogMsg(msg_list, new_msg_rec);
}
