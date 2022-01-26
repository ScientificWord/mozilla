#include "nsIGenericFactory.h"
#include "nsIClassInfoImpl.h"

#include "msiMapleEngineWrapper.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(msiMapleEngineWrapper)

static NS_METHOD msiMapleEngineWrapperRegistrationProc(nsIComponentManager *aCompMgr,
                                                        nsIFile *aPath,
                                                        const char *registryLocation,
                                                        const char *componentType,
                                                        const nsModuleComponentInfo *info)
{
    return NS_OK;
}

static NS_METHOD msiMapleEngineWrapperUnregistrationProc(nsIComponentManager *aCompMgr,
                                                          nsIFile *aPath,
                                                          const char *registryLocation,
                                                          const nsModuleComponentInfo *info)
{
    return NS_OK;
}

NS_DECL_CLASSINFO(msiMapleEngineWrapperImpl)

static const nsModuleComponentInfo components[] =
{
  { "Maple Engine Wrapper Component",
    MSI_MAPLEENGINEWRAPPER_CID,
    MSI_MAPLEENGINEWRAPPER_CONTRACTID,
    msiMapleEngineWrapperConstructor,
    msiMapleEngineWrapperRegistrationProc,
    msiMapleEngineWrapperUnregistrationProc
  }
};

NS_IMPL_NSGETMODULE(msiIEngineWrapper, components)

#if NEED_TO_DEBUG_MACROS
NSDEPENDENT_LIBS(msiMapleEngineWrapperModule)                            
NS_MODULEINFO NSMODULEINFO(msiMapleEngineWrapperModule) = {              
    NS_MODULEINFO_VERSION,                                                
    ("msiMapleEngineWrapperModule"),                                     
    (components),                                                         
    (sizeof(components) / sizeof(components[0])),                         
    (nsnull),                                                             
    (nsnull),                                                             
    (NSDEPENDENT_LIBS_NAME(msiMapleEngineWrapperModule))                 
};                                                                        

extern "C" NS_EXPORT nsresult                                                 
NSGetModule(nsIComponentManager *servMgr,                                     
            nsIFile* location,                                                
            nsIModule** result)                                               
{                                                                             
    return NS_NewGenericModule2(&(NSMODULEINFO(msiMapleEngineWrapperModule)), result);                            \
}
#endif

