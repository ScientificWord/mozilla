// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MuPAD2Tree_H
#define MuPAD2Tree_H

#include "../fltutils.h"

class MuPAD2Tree
{
public:
  MuPAD2Tree();
  ~MuPAD2Tree();

  MNODE *MuPADStr2Tree(const char *src);

private:
  MNODE * GetElementList(const char *z_src, int &advance, MNODE * parent,
                         char *enders);

  MNODE *GetDelimitedList(const char *z_src, int &bytes_done,
                          char ender, char ender2,
                          char *group_ilk, char *bucket_ilk);
  MNODE *GetParenedGroup(const char *z_src, int &bytes_done);
  MNODE *GetNumber(const char *z_src, int &advance);
  MNODE *GetOperator(const char *z_src, U32 op_len);
  MNODE *GetString(const char *z_src, int &advance);

  MNODE *DoPrecedenceBinds(MNODE * src_list);

  MNODE *BindOperators(MNODE * src_tree, int targ_prec, OpIlk op_type);
  MNODE *BindOp(MNODE * op_node, int targ_prec, OpIlk op_type);
  int GetPrecedence(const char *targ_op);
  MNODE *BindUnits(MNODE * op_node);

  char *GetName(const char *z_src, int &bytes_spanned);
  char *GetQuotedString(const char *z_src, int &bytes_spanned);

  bool IsUnit(MNODE * src_tree);
  bool IsOperatorName(const char *nom);

  int src_line_number;
  const char *start_ptr;
};

#endif
