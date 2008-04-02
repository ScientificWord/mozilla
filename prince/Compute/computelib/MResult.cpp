// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#include "MResult.h"
#include <string.h>


MathResult::MathResult()
{
  result_code = 0;
  result_string = NULL;
  w_result_string = NULL;
  eng_instr = NULL;
  eng_outstr = NULL;
  eng_errorstr = NULL;
  w_eng_instr = NULL;
  w_eng_outstr = NULL;
  w_eng_errorstr = NULL;

  w_buffer = NULL;
  component_list = NULL;
  msg_list = NULL;

  s_tree = NULL;
  sem_str = NULL;
  w_sem_str = NULL;
}

MathResult::~MathResult()
{
  delete[] result_string;
  delete[] w_result_string;
  delete[] eng_instr;
  delete[] eng_outstr;
  delete[] eng_errorstr;

  delete[] w_eng_instr;
  delete[] w_eng_outstr;
  delete[] w_eng_errorstr;

  delete[] w_buffer;
  DisposeComponentList(component_list);
  DisposeMsgs(msg_list);

  DisposeSList(s_tree);
  delete[] sem_str;
  delete[] w_sem_str;
}

const U16 *MathResult::GetWideResultStr()
{
  if (!w_result_string && result_string) {
    int zlen;
    w_result_string = ASCIItoWide(result_string, zlen);
  }

  return w_result_string;
}

void MathResult::PutResultStr(const char *src, int zlen)
{ 
  delete[] result_string;
  result_string = NULL;
  if (zlen > 0) {
    result_string = new char[zlen + 1];
    strncpy(result_string, src, zlen);
    result_string[zlen] = 0;
  }
}

void MathResult::PutWideResultStr(const U16 * src, int zlen)
{
  delete[] w_result_string;
  w_result_string = NULL;
  if (zlen > 0) {
    w_result_string = new U16[zlen + 1];

    const U16 *p = src;
    int i = 0;
    while (i < zlen) {
      w_result_string[i++] = *p;
      p++;
    }
    w_result_string[zlen] = 0;
  }
}

void MathResult::PutEngInStr(const char *src, int zlen)
{
  delete[] eng_instr;
  eng_instr = NULL;
  if (zlen > 0) {
    eng_instr = new char[zlen + 1];
    strncpy(eng_instr, src, zlen);
    eng_instr[zlen] = 0;
  }
}

void MathResult::PutEngOutStr(const char *src, int zlen)
{
  delete[] eng_outstr;
  eng_outstr = NULL;
  if (zlen > 0) {
    eng_outstr = new char[zlen + 1];
    strncpy(eng_outstr, src, zlen);
    eng_outstr[zlen] = 0;
  }
}

void MathResult::PutEngErrorStr(const char *src, int zlen)
{
  delete[] eng_errorstr;
  eng_errorstr = NULL;
  if (zlen > 0) {
    eng_errorstr = new char[zlen + 1];
    strncpy(eng_errorstr, src, zlen);
    eng_errorstr[zlen] = 0;
  }
}

const U16 *MathResult::GetWideEngInStr()
{
  if (!w_eng_instr && eng_instr) {
    int zlen;
    w_eng_instr = ASCIItoWide(eng_instr, zlen);
  }
  return w_eng_instr;
}

const U16 *MathResult::GetWideEngOutStr()
{
  if (!w_eng_outstr && eng_outstr) {
    int zlen;
    w_eng_outstr = ASCIItoWide(eng_outstr, zlen);
  }
  return w_eng_outstr;
}

const U16 *MathResult::GetWideEngErrorStr()
{
  if (!w_eng_errorstr && eng_errorstr) {
    int zlen;
    w_eng_errorstr = ASCIItoWide(eng_errorstr, zlen);
  }

  return w_eng_errorstr;
}

void MathResult::PutResultComponent(const char *src, U32 component_ID,
                                    int index)
{
  AppendComponent(component_list, component_ID, index, src);
}

const char *MathResult::GetComponent(U32 component_ID, int index)
{
  char *rv = NULL;

  COMPONENT_REC *rover = component_list;
  while (rover) {
    if (rover->component_ID == component_ID && rover->index == index) {
      rv = rover->ztext;
      break;
    }
    rover = rover->next;
  }

  return rv;
}

const U16 *MathResult::GetWideComponent(U32 component_ID, int index)
{
  const char *tmp = GetComponent(component_ID, index);
  if (tmp) {
    int zlen;
    delete[] w_buffer;
    w_buffer = ASCIItoWide(tmp, zlen);
    return w_buffer;
  } else
    return NULL;
}

void MathResult::PutSemanticsTree(SEMANTICS_NODE * sem_tree)
{
  DisposeSList(s_tree);
  s_tree = NULL;
  delete[] sem_str;
  sem_str = NULL;
  delete[] w_sem_str;
  w_sem_str = NULL;

  if (sem_tree)
    s_tree = sem_tree;
}

const char *MathResult::GetSemanticsStr()
{
  if (!sem_str && s_tree)
    sem_str = DumpSNode(s_tree, 0);

  return sem_str;
}

const U16 *MathResult::GetWideSemanticsStr()
{
  if (!w_sem_str) {
    const char *tmp = GetSemanticsStr();
    if (tmp) {
      int zlen;
      w_eng_instr = ASCIItoWide(tmp, zlen);
    }
  }

  return w_sem_str;
}

void AppendComponent(COMPONENT_REC * &curr_list, U32 c_ID, int index,
                       const char *zdata)
{
  COMPONENT_REC *new_rec = new COMPONENT_REC();
  new_rec->next = curr_list;

  new_rec->component_ID = c_ID;
  new_rec->index = index;
  if (zdata) {
    size_t zln = strlen(zdata);
    char *tmp = new char[zln + 1];
    strcpy(tmp, zdata);
    new_rec->ztext = tmp;
  } else {
    new_rec->ztext = NULL;
  }
  curr_list = new_rec;
}

void DisposeComponentList(COMPONENT_REC * c_list)
{
  while (c_list) {
    COMPONENT_REC *del = c_list;
    c_list = c_list->next;
    delete[] del->ztext;
    delete del;
  }
}
