// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

// This object emulates the DLL interface defined in CompDLL/ComputeDLL.h
// Sort of hokey.

#include "CmpTypes.h"
#include "MRequest.h"
#include "nsILocalFile.h"

class ComputeDLL
{
public:
  static int InitCompDLL();
  static void TermCompDLL();

  static U32 InstallEngine(U32 trans_ID, nsILocalFile *install_script,
                             int *result_code);
  static void UninstallEngine(U32 eng_ID);

  static const char *GetNextCommand(U32 engine_ID, U32 * curr_cmd_ID);

  static U32 GetClientHandle(U32 parent_handle);
  static void ReleaseClientHandle(U32 c_handle);

  static U32 CreateTransaction(U32 c_handle, const U16 * w_mml,
                                 U32 eng_ID, U32 cmd_ID);
  static void ReleaseTransaction(U32 trans_ID);

  static int ExecuteTransaction(U32 trans_ID);

  static int  AddParam(U32 trans_ID, U32 param_ID, U32 param_type,
                       const char *ascii_str);
  static int  AddWideParam(U32 trans_ID, U32 param_ID, U32 param_type,
                           const U16 * wide_str);

  static const U16 * GetPtrWIDEresult(U32 trans_ID, int str_ID);
  static const char * GetPtrASCIIresult(U32 trans_ID, int str_ID);

  static const U16 * GetPtrWIDEresultPart(U32 trans_ID, U32 part,
                                           int index);
  static const char * GetPtrASCIIresultPart(U32 trans_ID, U32 part,
                                            int index);

  static DefStore* GetDefStore(U32 client_ID);

  static const void * GetNextDef(U32 client_ID, U32 engine_ID,
                                 const void *curr_def);
  static void  ClearDefs(U32 client_ID, U32 engine_ID);

  static int  SetEngineStateAttr(U32 engine_ID, int attr_ID,
                                 const char *s_val);
  static const char * GetEngineStateAttr(U32 engine_ID, int attr_ID);

  static int  SetUserPref(U32 client_ID, U32 pref_ID,
                          const char *ascii_str);
  static int  SetWideUserPref(U32 client_ID, U32 pref_ID,
                              const U16 * wide_str);
  static const char * GetUserPref(U32 client_ID, U32 pref_ID,
                                  int no_inherit);
  static const U16 * GetWideUserPref(U32 client_ID, U32 pref_ID,
                                      int no_inherit);
private:
  static bool CheckParam(MathServiceRequest * msr, U32 p_ID, U32 expected_type);
  static int ValidateParams(U32 cmd_ID, MathServiceRequest * msr);
};
