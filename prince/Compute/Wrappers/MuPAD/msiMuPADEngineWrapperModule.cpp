#include "nsIGenericFactory.h"
#include "nsIClassInfoImpl.h"

#include "msiMuPADEngineWrapper.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(msiMuPADEngineWrapper)

static NS_METHOD msiMuPADEngineWrapperRegistrationProc(nsIComponentManager *aCompMgr,
                                                        nsIFile *aPath,
                                                        const char *registryLocation,
                                                        const char *componentType,
                                                        const nsModuleComponentInfo *info)
{
    return NS_OK;
}

static NS_METHOD msiMuPADEngineWrapperUnregistrationProc(nsIComponentManager *aCompMgr,
                                                          nsIFile *aPath,
                                                          const char *registryLocation,
                                                          const nsModuleComponentInfo *info)
{
    return NS_OK;
}

NS_DECL_CLASSINFO(msiMuPADEngineWrapper)

static const nsModuleComponentInfo components[] =
{
  { "MuPAD Engine Wrapper Component",
    MSI_MUPADENGINEWRAPPER_CID,
    MSI_MUPADENGINEWRAPPER_CONTRACTID,
    msiMuPADEngineWrapperConstructor,
    msiMuPADEngineWrapperRegistrationProc,
    msiMuPADEngineWrapperUnregistrationProc
  }
};

NS_IMPL_NSGETMODULE(msiIEngineWrapper, components)

#if NEED_TO_DEBUG_MACROS
NSDEPENDENT_LIBS(msiMuPADEngineWrapperModule)                            
NS_MODULEINFO NSMODULEINFO(msiMuPADEngineWrapperModule) = {              
    NS_MODULEINFO_VERSION,                                                
    ("msiMuPADEngineWrapperModule"),                                     
    (components),                                                         
    (sizeof(components) / sizeof(components[0])),                         
    (nsnull),                                                             
    (nsnull),                                                             
    (NSDEPENDENT_LIBS_NAME(msiMuPADEngineWrapperModule))                 
};                                                                        

extern "C" NS_EXPORT nsresult                                                 
NSGetModule(nsIComponentManager *servMgr,                                     
            nsIFile* location,                                                
            nsIModule** result)                                               
{                                                                             
    return NS_NewGenericModule2(&(NSMODULEINFO(msiMuPADEngineWrapperModule)), result);                            \
}
#endif

