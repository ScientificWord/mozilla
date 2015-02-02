#include "nsXPCOMGlue.h"
#include "nsCOMPtr.h"
#include "nsILocalFile.h"
#include "nsDirectoryService.h"
#include "msiAppUtils.h"



/* Header file */

/* Implementation file */
NS_IMPL_ISUPPORTS1(msiAppUtils, msiIAppUtils)

msiAppUtils::~msiAppUtils()
{
  Goodbye();
}

msiAppUtils::msiAppUtils()
{
  Hello();
}

/* [noscript] readonly attribute PRInt32 licensedApp; */
NS_IMETHODIMP msiAppUtils::GetLicensedApp(PRInt32 *aLicensedApp)
{
    *aLicensedApp = mLicensedApp;
    return NS_OK;
}

/* readonly attribute DOMString licensedUntil; */
NS_IMETHODIMP msiAppUtils::GetLicensedUntil(nsAString & aLicensedUntil)
{
    aLicensedUntil = mLicensedUntil;
    return NS_OK;
}

/* void hello (); */
NS_IMETHODIMP msiAppUtils::Hello()
{
    int stat;
    char *product;
    int count = 1;
    const char * utf8Path;
    const char * ver = "6.0";
    nsString path;
    RLM_LICENSE lic = NULL;
    nsCOMPtr<nsIProperties> fileLocator(do_GetService("@mozilla.org/file/directory_service;1"));
    nsCOMPtr<nsILocalFile> licFile;
    fileLocator->Get("resource:app", NS_GET_IID(nsIFile), getter_AddRefs(licFile));
    licFile->GetPath(path);

    utf8Path = ToNewUTF8String(path);


    mrh = rlm_init(utf8Path, utf8Path, (char *) NULL);
    stat = rlm_stat(mrh);
    if (stat)
    {
      char errstring[RLM_ERRSTRING_MAX];

      (void) printf("Error initializing license system\n");
      (void) printf("%s\n", rlm_errstring((RLM_LICENSE) NULL, mrh, 
                  errstring));
    }
    else
    {
      mLic = rlm_checkout(mrh, product, ver, count);
    }    
    return NS_OK;
}

/* void goodbye (); */
NS_IMETHODIMP msiAppUtils::Goodbye()
{
  rlm_close(mrh);
  return NS_OK;
}

