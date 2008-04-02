// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MRESULT_H
#define MRESULT_H

#include "CmpTypes.h"
#include "fltutils.h"

// used to be an overloaded PARAM_REC
typedef struct tagCOMPONENT_REC
{                               // Struct to carry a result component
  tagCOMPONENT_REC *next;
  U32 component_ID;
  int index;
  char *ztext;
} COMPONENT_REC;

void AppendComponent(COMPONENT_REC * &curr_list, U32 c_ID, int index,
                       const char *zdata);
void DisposeComponentList(COMPONENT_REC * c_list);

class MathResult
{
public:
  MathResult();
  ~MathResult();

  void PutResultCode(int code)
  {
    result_code = code;
  }
  int GetResultCode()
  {
    return result_code;
  }

  void PutResultStr(const char *src, int zlen);
  const char *GetResultStr()
  {
    return result_string;
  }

  void PutWideResultStr(const U16 * src, int zlen);
  const U16 *GetWideResultStr();

  void PutEngInStr(const char *src, int zlen);
  const char *GetEngInStr()
  {
    return eng_instr;
  }
  const U16 *GetWideEngInStr();

  void PutEngOutStr(const char *src, int zlen);
  const char *GetEngOutStr()
  {
    return eng_outstr;
  }
  const U16 *GetWideEngOutStr();

  void PutEngErrorStr(const char *src, int zlen);
  const char *GetEngErrorStr()
  {
    return eng_errorstr;
  }
  const U16 *GetWideEngErrorStr();

  void AttachMsgs(LOG_MSG_REC * new_msgs)
  {
    TCI_ASSERT(!msg_list);
    DisposeMsgs(msg_list);
    msg_list = new_msgs;
  }
  LOG_MSG_REC *GetMsgs()
  {
    return msg_list;
  }

  void PutResultComponent(const char *src, U32 component_ID, int index);
  const char *GetComponent(U32 component_ID, int index);
  const U16 *GetWideComponent(U32 component_ID, int index);

  void PutSemanticsTree(SEMANTICS_NODE * sem_tree);
  SEMANTICS_NODE *GetSemanticsTree()
  {
    return s_tree;
  }

  const char *GetSemanticsStr();
  const U16 *GetWideSemanticsStr();

private:
  int result_code;

  // Note that this object "owns" the following heap arrays
  // It allocates (and MUST delete) the following 4 variables.
  char *result_string;
  U16 *w_result_string;
  char *eng_instr;
  char *eng_outstr;
  char *eng_errorstr;
  char *sem_str;
  U16 *w_eng_instr;
  U16 *w_eng_outstr;
  U16 *w_eng_errorstr;
  U16 *w_sem_str;

  U16 *w_buffer;
  COMPONENT_REC *component_list;

  SEMANTICS_NODE *s_tree;
  LOG_MSG_REC *msg_list;
};

#endif
