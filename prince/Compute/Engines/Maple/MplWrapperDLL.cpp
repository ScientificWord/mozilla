// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.

// MplWrapperDLL.cpp : Defines the entry point for the DLL application.
//

#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#ifdef TESTING
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include "../../CmpTypes.h"
#include "../../MRequest.h"
#include "../../MResult.h"
#include "../fltutils.h"

#define ENGWRAPDLL_EXPORTS
#include "../ewrapDLL.h"
#undef ENGWRAPDLL_EXPORTS

#include "MplWrap.h"

#ifdef TESTING
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

LONG clients;

MapleWrapper *maple_wrapper;

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call) {

  case DLL_PROCESS_ATTACH:
#ifdef TESTING
    // detect and report memory leaks
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    if (InterlockedIncrement(&clients) > 1) {
      InterlockedDecrement(&clients);
      return false;
    }
    break;

  case DLL_PROCESS_DETACH:
    delete maple_wrapper;
    InterlockedDecrement(&clients);
    break;

  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    return false;
  }

  return true;
}

ENGWRAPDLL_API int __cdecl iLoadStrsAndDLL(const char *engpath,
                                           const char *libpath,
                                           void *ID_dBase, void *NOM_dBase)
{
  maple_wrapper = new MapleWrapper(ID_dBase, NOM_dBase);
  return maple_wrapper->iLoadStrsAndDLL(engpath, libpath);
}

ENGWRAPDLL_API int __cdecl iInitialize(const char *libpath,
                                       const char *champath,
                                       const char *vcampath)
{
  return maple_wrapper->iInitialize(libpath, champath);
}

ENGWRAPDLL_API int __cdecl SetEngineState(int attr_ID, const char *new_value)
{
  return maple_wrapper->SetEngineState(attr_ID, new_value);
}

ENGWRAPDLL_API void *__cdecl iProcessRequest(void *msr, void *mr)
{
  return (void *)maple_wrapper->iProcessRequest((MathServiceRequest *) msr,
                                                (MathResult *) mr);
}

ENGWRAPDLL_API void __cdecl iDisposeSList(void *slist)
{
  maple_wrapper->iDisposeSList((SEMANTICS_NODE *) slist);
}

ENGWRAPDLL_API int __cdecl iGetEngStrPtr(int str_ID, char **engstr,
                                         int *engstr_ln)
{
  return maple_wrapper->iGetEngStrPtr(str_ID, engstr, engstr_ln);
}

ENGWRAPDLL_API int __cdecl iExecute(const char *cmd)
{
  return maple_wrapper->iExecute(cmd);
}

ENGWRAPDLL_API void __cdecl iCleanup()
{
  maple_wrapper->iCleanupAndUnloadDLL();
}

ENGWRAPDLL_API void __cdecl iReleaseClient(long client_ID, char *def_list)
{
  maple_wrapper->iReleaseClient(client_ID, def_list);
}

