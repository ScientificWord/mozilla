#ifndef DEFINFO_H
#define DEFINFO_H

#include "iCmpTypes.h"

// Node for list of definitions
#define DT_NONE 0
#define DT_VARIABLE 1
#define DT_FUNCTION 2
#define DT_MUPNAME 3

class DefInfo
{
  public:
    DefInfo* next;
    U32 owner_ID;
    U32 engine_ID;
    char* canonical_name;
    U32 def_type;
    char* src_markup;
    char* arg_list;
    U32 n_subscripted_args;
    char* ASCII_src;
    U16* WIDE_src;
};


#endif

