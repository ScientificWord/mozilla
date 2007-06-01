// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

#include "msiMapleEngineWrapper.h"
#include "MplWrap.h"

#include "nsMemory.h"
#include "nsEmbedString.h"

#include "../../CmpTypes.h"
#include "../../MRequest.h"
#include "../../MResult.h"
#include "../fltutils.h"
#include <stdio.h>
#include "nsILocalFile.h"


NS_IMPL_ISUPPORTS1(msiMapleEngineWrapper, msiIEngineWrapper)

msiMapleEngineWrapper::msiMapleEngineWrapper() : Maple_wrapper(0)
{
}

msiMapleEngineWrapper::~msiMapleEngineWrapper()
{
  delete Maple_wrapper;
}

/* long loadStrsAndDLL (in string engpath, in string libpath, in voidPtr ID_dbase, in voidPtr NOM_dbase); */
NS_IMETHODIMP msiMapleEngineWrapper::LoadStrsAndDLL(nsILocalFile *engpath, nsILocalFile *libpath, void * ID_dbase, void * NOM_dbase, PRInt32 *_retval)
{
  Maple_wrapper = new MapleWrapper(ID_dbase, NOM_dbase);
  *_retval = Maple_wrapper->iLoadStrsAndDLL(engpath, libpath);
  return NS_OK;
}

/* long initialize (in string libpath, in string champath, in string vcampath); */
NS_IMETHODIMP msiMapleEngineWrapper::Initialize(nsILocalFile *libpath, nsILocalFile *champath, nsILocalFile *vcampath, PRInt32 *_retval)
{
  *_retval = Maple_wrapper->iInitialize(libpath, champath);
  return NS_OK;
}

/* voidPtr ProcessRequest (in voidPtr msr, in voidPtr mr); */
NS_IMETHODIMP msiMapleEngineWrapper::ProcessRequest(void * msr, void * mr, void * *_retval)
{
  *_retval = (void *)Maple_wrapper->iProcessRequest((MathServiceRequest *) msr,
                                                (MathResult *) mr);
  return NS_OK;
}

/* void disposeSList (in voidPtr slist); */
NS_IMETHODIMP msiMapleEngineWrapper::DisposeSList(void * slist)
{
  Maple_wrapper->iDisposeSList((SEMANTICS_NODE *) slist);
  return NS_OK;
}

/* long setEngineState (in long attr_ID, in string new_value); */
NS_IMETHODIMP msiMapleEngineWrapper::SetEngineState(PRInt32 attr_ID, const char *new_value, PRInt32 *_retval)
{
  *_retval = Maple_wrapper->SetEngineState(attr_ID, new_value);
  return NS_OK;
}

/* long getEngStrPtr (in long str_ID, out string engstr, out long engstr_ln); */
NS_IMETHODIMP msiMapleEngineWrapper::GetEngStrPtr(PRInt32 str_ID, char **engstr, PRInt32 *engstr_ln, PRInt32 *_retval)
{
  *_retval = Maple_wrapper->iGetEngStrPtr(str_ID, engstr, engstr_ln);
  return NS_OK;
}

/* long execute (in string cmd); */
NS_IMETHODIMP msiMapleEngineWrapper::Execute(const char *cmd, PRInt32 *_retval)
{
  *_retval = Maple_wrapper->iExecute(cmd);
  return NS_OK;
}

/* void cleanup (); */
NS_IMETHODIMP msiMapleEngineWrapper::Cleanup()
{
  Maple_wrapper->iCleanupAndUnloadDLL();
  return NS_OK;
}

/* void releaseClient (in long client_ID, in string def_list); */
NS_IMETHODIMP msiMapleEngineWrapper::ReleaseClient(PRInt32 client_ID, const char *def_list)
{
  Maple_wrapper->iReleaseClient(client_ID, def_list);
  return NS_OK;
}
