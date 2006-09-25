

#include "nsIGenericFactory.h"

#include "nsStringAPI.h"
#include "msiArrowStateService.h"


NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(msiArrowStateService, msiArrowStateService::GetInstance)

static const nsModuleComponentInfo gComponents[] = {
  { 
    "Arrow state Service",
    MSI_ARROWSTATE_SERVICE_CID,
    MSI_ARROWSTATE_SERVICE_CONTRACTID,
    msiArrowStateServiceConstructor
  }
};

NS_IMPL_NSGETMODULE(msiArrowStateComponent, gComponents)  

