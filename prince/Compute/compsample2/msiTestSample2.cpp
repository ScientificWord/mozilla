#include <stdio.h>

#include "msiISimpleComputeEngine.h"
#include "nsIServiceManager.h"
#include "nsIComponentRegistrar.h"

#ifdef XPCOM_GLUE
#include "nsXPCOMGlue.h"
#include "nsMemory.h"
#else
#include "nsXPIDLString.h"
#endif

#define MSI_SIMPLECOMPUTEENGINE_CONTRACTID "@mackichan.com/simplecomputeengine;2"

int
main(void)
{
    nsresult rv;

#ifdef XPCOM_GLUE
    XPCOMGlueStartup(nsnull);
#endif

    nsCOMPtr<nsIServiceManager> servMan;
    rv = NS_InitXPCOM2(getter_AddRefs(servMan), nsnull, nsnull);
    if (NS_FAILED(rv))
    {
        printf("ERROR: XPCOM intialization error [%x].\n", rv);
        return -1;
    }

    nsCOMPtr<nsIComponentRegistrar> registrar = do_QueryInterface(servMan);
    NS_ASSERTION(registrar, "Null nsIComponentRegistrar");
    registrar->AutoRegister(nsnull);
    
    nsCOMPtr<nsIComponentManager> manager = do_QueryInterface(registrar);
    NS_ASSERTION(registrar, "Null nsIComponentManager");
    
    nsCOMPtr<msiISimpleComputeEngine> mysample;
    rv = manager->CreateInstanceByContractID(MSI_SIMPLECOMPUTEENGINE_CONTRACTID,
                                             nsnull,
                                             NS_GET_IID(msiISimpleComputeEngine),
                                             getter_AddRefs(mysample));
    if (NS_FAILED(rv))
    {
        printf("ERROR: Cannot create instance of component " MSI_SIMPLECOMPUTEENGINE_CONTRACTID " [%x].\n"
               "Debugging hint:\n"
               "\tsetenv NSPR_LOG_MODULES nsComponentManager:5\n"
               "\tsetenv NSPR_LOG_FILE xpcom.log\n"
               "\t./msiTestSample2\n"
               "\t<check the contents for xpcom.log for possible cause of error>.\n",
               rv);
        return -2;
    }

    // Call methods on our sample to test it out.
    rv = mysample->Startup(L"mupInstall.gmr");
    if (NS_FAILED(rv))
    {
        printf("ERROR: Calling msiISimpleComputeEngine::Startup() [%x]\n", rv);
        return -3;
    } else {
      printf(" Startup OK\n");
    }

    PRUnichar* str;
    rv = mysample->Eval(L"<math><mn>1</mn><mo>+</mo><mn>2</mn></math>",&str);
    if (NS_FAILED(rv))
    {
        printf("ERROR: Calling msiISimpleComputeEngine::Eval() [%x]\n", rv);
        return -3;
    } else {
      printf(" Evaluate OK\n");
    }

    rv = mysample->Shutdown();
    if (NS_FAILED(rv))
    {
        printf("ERROR: Calling msiISimpleComputeEngine::Shutdown() [%x]\n", rv);
        return -3;
    } else {
      printf(" Shutdown OK\n");
    }

    servMan = 0;
    registrar = 0;
    manager = 0;
    mysample = 0;
    
    NS_ShutdownXPCOM(nsnull);

#ifdef XPCOM_GLUE
    XPCOMGlueShutdown();
#endif
    return 0;
}
