// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef iCMPTYPES_H
#define iCMPTYPES_H

typedef unsigned short U16;
typedef unsigned long U32;

// NULL
#undef NULL
#define NULL  0

// Node for list of definitions
#define DT_NONE 0
#define DT_VARIABLE 1
#define DT_FUNCTION 2

typedef struct tagDefInfo
{
  tagDefInfo *next;
  U32 owner_ID;
  U32 engine_ID;
  char *canonical_name;
  U32 def_type;
  char *src_markup;
  char *arg_list;
  U32 n_subscripted_args;
  char *ASCII_src;
  U16 *WIDE_src;
} DefInfo;

#endif
