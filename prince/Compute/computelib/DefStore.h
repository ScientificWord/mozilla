// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef DEFSTORE_H
#define DEFSTORE_H

#include "CmpTypes.h"
#include "Engines/fltutils.h"

class DefStore {
public:

  DefStore( DefStore* parent_store,U32 client_handle );
  ~DefStore();

  void      SetParentDefStore( DefStore* parent_store );

  void      PushDefInfo( U32 engine_ID,const char* def_name,U32 def_type,
                        const char* markup,const char* arg_list,
                        U32 n_sub_args,
                        const char* ASCII_src,const U16* WIDE_src );
  DefInfo*  GetDefInfo( U32 engine_ID,const char* canonical_ID );
  void      RemoveDef( U32 engine_ID,const char* targ_canon_nom );

  char*     GetDefList( U32 engine_ID );
  void      ClearDefs( U32 engine_ID );

  const DefInfo*  GetNextDef( U32 engine_ID,const DefInfo* curr_def );

  int         SetPref( U32 pref_ID,const char* new_value );
  const char* GetPref( U32 pref_ID,int no_inherit );
  int         RemovePref( U32 pref_ID );

private:
  DefInfo* GetLocalDefInfo( U32 engine_ID,const char* targ_canon_nom );
  DefInfo* DeleteDefInfo( U32 engine_ID,const char* targ_canon_nom );

  DefStore* parent_ds;
  U32 client_ID;
  DefInfo*   def_list;
  PARAM_REC* prefs_list;
};

#endif

