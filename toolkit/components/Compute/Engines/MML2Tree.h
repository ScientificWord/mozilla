// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef MML2Tree_H
#define MML2Tree_H

#include "fltutils.h"
class MNODE;
class ATTRIB_REC;

class MML2Tree
{
public:

  MML2Tree();
  ~MML2Tree();

  MNODE* MMLstr2Tree(const char* src);

private:

  MNODE* GetElementList(const char* z_src, int& advance, MNODE* parent);

  int GetElementHeader(const char *p_header,
                       MNODE * new_node, int &has_no_contents);

  MNODE* GetElement(const char *z_src, int &advance);

  ATTRIB_REC* GetAttribute(const char* p_attr, int &bytesdone);

  int VerifyElementEnder(MNODE * mml_element, const char *needle);

  int src_line_number;
  const char *start_ptr;
};

#endif
