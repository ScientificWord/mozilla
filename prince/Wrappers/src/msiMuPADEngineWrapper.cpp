// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//#include "MuWrap.h"
#include "msiMuPADEngineWrapper.h"
#include "nsIGenericFactory.h"


#include "MRequest.h"
#include "MResult.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <locale.h>
#include <time.h>
#include "mupkernel.h"

#include "xpcom-config.h"
#include "nsCOMPtr.h"
#include "nsILocalFile.h"


NS_IMPL_ISUPPORTS1(msiMuPADEngineWrapper, msiIEngineWrapper)

NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(msiMuPADEngineWrapper, msiMuPADEngineWrapper::GetInstance)


msiMuPADEngineWrapper* s_wrapper;

/* static */ 
msiMuPADEngineWrapper* msiMuPADEngineWrapper::GetInstance()
{
  if (!s_wrapper) {
    s_wrapper = new msiMuPADEngineWrapper();
    if (!s_wrapper)
      return nsnull;

    NS_ADDREF(s_wrapper);   // addref the global
  }

  NS_ADDREF(s_wrapper);   // addref the return result
  return s_wrapper;
}



msiMuPADEngineWrapper::msiMuPADEngineWrapper() : MuPAD_wrapper(0)
{
}

msiMuPADEngineWrapper::~msiMuPADEngineWrapper()
{
  delete MuPAD_wrapper;
}

/* long loadStrsAndDLL (in string engpath, in string libpath, in voidPtr ID_dbase, in voidPtr NOM_dbase); */
NS_IMETHODIMP msiMuPADEngineWrapper::LoadStrsAndDLL(nsILocalFile *engpath, nsILocalFile *libpath, nsILocalFile *modpath, void * ID_dbase, void * NOM_dbase, PRInt32 *_retval)
{
  MuPAD_wrapper = new MuPADWrapper(ID_dbase, NOM_dbase);
  *_retval = MuPAD_wrapper->iLoadStrsAndDLL(engpath, libpath, modpath);
  return NS_OK;
}

/* long initialize (in string libpath, in string champath, in string vcampath); */
NS_IMETHODIMP msiMuPADEngineWrapper::Initialize(nsILocalFile *libpath, nsILocalFile *champath, nsILocalFile *vcampath, PRInt32 *_retval)
{
  *_retval = MuPAD_wrapper->iInitialize(libpath, champath, vcampath);
  return NS_OK;
}

/* voidPtr ProcessRequest (in voidPtr msr, in voidPtr mr); */
NS_IMETHODIMP msiMuPADEngineWrapper::ProcessRequest(void * msr, void * mr, void * *_retval)
{
  *_retval = (void *)MuPAD_wrapper->iProcessRequest((MathServiceRequest *) msr,
                                                (MathResult *) mr);
  return NS_OK;
}

/* void disposeSList (in voidPtr slist); */
NS_IMETHODIMP msiMuPADEngineWrapper::DisposeSList(void * slist)
{
  MuPAD_wrapper->iDisposeSList((SEMANTICS_NODE *) slist);
  return NS_OK;
}

/* long setEngineState (in long attr_ID, in string new_value); */
NS_IMETHODIMP msiMuPADEngineWrapper::SetEngineState(PRInt32 attr_ID, const char *new_value, PRInt32 *_retval)
{
  *_retval = MuPAD_wrapper->SetEngineState(attr_ID, new_value);
  return NS_OK;
}

/* long getEngStrPtr (in long str_ID, out string engstr, out long engstr_ln); */
NS_IMETHODIMP msiMuPADEngineWrapper::GetEngStrPtr(PRInt32 str_ID, char **engstr, PRInt32 *engstr_ln, PRInt32 *_retval)
{
  *_retval = MuPAD_wrapper->iGetEngStrPtr(str_ID, const_cast<const char **>(engstr), engstr_ln);
  return NS_OK;
}

/* long execute (in string cmd); */
NS_IMETHODIMP msiMuPADEngineWrapper::Execute(const char *cmd, PRInt32 *_retval)
{
  *_retval = MuPAD_wrapper->iExecute(cmd);
  return NS_OK;
}

/* void cleanup (); */
NS_IMETHODIMP msiMuPADEngineWrapper::Cleanup()
{
  MuPAD_wrapper->iCleanupAndUnloadDLL();
  return NS_OK;
}

/* void releaseClient (in long client_ID, in string def_list); */
NS_IMETHODIMP msiMuPADEngineWrapper::ReleaseClient(PRInt32 client_ID, const char *def_list)
{
  MuPAD_wrapper->iReleaseClient(client_ID, def_list);
  return NS_OK;
}



static const nsModuleComponentInfo gComponents[] =
{
  { "MuPADEngineWrapper",
    MSI_MUPADENGINEWRAPPER_CID ,
    MSI_MUPADENGINEWRAPPER_CONTRACTID,
    msiMuPADEngineWrapperConstructor }
};

NS_IMPL_NSGETMODULE(wrappers, gComponents)
