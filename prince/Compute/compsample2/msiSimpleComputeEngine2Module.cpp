// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

#include "nsIGenericFactory.h"
#include "nsIClassInfoImpl.h"

#include "msiSimpleComputeEngine2.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(msiSimpleComputeEngine2)

static NS_METHOD msiSimpleComputeEngine2RegistrationProc(nsIComponentManager *aCompMgr,
                                                        nsIFile *aPath,
                                                        const char *registryLocation,
                                                        const char *componentType,
                                                        const nsModuleComponentInfo *info)
{
    return NS_OK;
}

static NS_METHOD msiSimpleComputeEngine2UnregistrationProc(nsIComponentManager *aCompMgr,
                                                          nsIFile *aPath,
                                                          const char *registryLocation,
                                                          const nsModuleComponentInfo *info)
{
    return NS_OK;
}

NS_DECL_CLASSINFO(msiSimpleComputeEngine2)

static const nsModuleComponentInfo components[] =
{
  { "Second Simple Compute Engine Component",
    MSI_SIMPLECOMPUTEENGINE2_CID,
    MSI_SIMPLECOMPUTEENGINE2_CONTRACTID,
    msiSimpleComputeEngine2Constructor,
    msiSimpleComputeEngine2RegistrationProc,
    msiSimpleComputeEngine2UnregistrationProc
  }
};

NS_IMPL_NSGETMODULE(msiISimpleComputeModule2, components)

#if NEED_TO_DEBUG_MACROS
NSDEPENDENT_LIBS(msiSimpleComputeEngineModule2)                            
NS_MODULEINFO NSMODULEINFO(msiSimpleComputeEngine2Module) = {              
    NS_MODULEINFO_VERSION,                                                
    ("msiSimpleComputeEngineModule2"),                                     
    (components),                                                         
    (sizeof(components) / sizeof(components[0])),                         
    (nsnull),                                                             
    (nsnull),                                                             
    (NSDEPENDENT_LIBS_NAME(msiSimpleComputeEngine2Module))                 
};                                                                        

extern "C" NS_EXPORT nsresult                                                 
NSGetModule(nsIComponentManager *servMgr,                                     
            nsIFile* location,                                                
            nsIModule** result)                                               
{                                                                             
    return NS_NewGenericModule2(&(NSMODULEINFO(msiSimpleComputeEngine2Module)), result);                            \
}
#endif

