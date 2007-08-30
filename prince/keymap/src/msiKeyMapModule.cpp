

#include "nsIGenericFactory.h"

#include "nsStringAPI.h"
#include "msiKeyMap.h"


NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(msiKeyMap, msiKeyMap::GetInstance)

static const nsModuleComponentInfo gComponents[] = {
  { 
    "Key Mapping Service",
    MSI_KEYMAP_CID,
    MSI_KEYMAP_CONTRACTID,
    msiKeyMapConstructor
  }
};

NS_IMPL_NSGETMODULE(msiKeyMapComponent, gComponents)  

