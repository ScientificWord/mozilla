// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

#ifndef PREFSTORE_H
#define PREFSTORE_H

#include "CmpTypes.h"

class PrefsStore {
public:
  PrefsStore();
  ~PrefsStore();

  int         SetPref( U32 pref_ID, const char* new_value );
  const char* GetPref( U32 pref_ID );

private:
  char* prefs[CLPF_last];
};

#endif

