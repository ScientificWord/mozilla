#include "nsXPCOMGlue.h"
#include "nsCOMPtr.h"
#include "nsILocalFile.h"
#include "nsDirectoryService.h"
#include "msiAppUtils.h"

PRUint32 msiAppUtils::licensedProd;
char * msiAppUtils::pchProdName;
char * msiAppUtils::pchExpDate;
RLM_HANDLE msiAppUtils::rh;
RLM_LICENSE msiAppUtils::lic;
PRUint32 syzygy; // obscure name for licensed product number
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
  *aLicensedApp = syzygy;
  return NS_OK;

    // nsCString sProd;
    // PRInt32 index;
    // PRInt32 stat = -1;
    // if (lic) stat = rlm_license_stat(lic);
    // if (stat) {
    //   index = 0;
    // } else {
    //   sProd = nsDependentCString(pchProdName);
    //   if (sProd.EqualsLiteral("snb")) index = 1;
    //   else if (sProd.EqualsLiteral("sw")) index = 2;
    //   else if (sProd.EqualsLiteral("swp")) index = 3;
    //   else index = 0;
    // }
    // *aLicensedApp = index;
    // return NS_OK;
}

/* readonly attribute DOMString licensedUntil; */
NS_IMETHODIMP msiAppUtils::GetLicensedUntil(nsACString & aLicensedUntil)
{
    PRInt32 stat = rlm_license_stat(lic);
    if (!stat) {
      aLicensedUntil = nsDependentCString(pchExpDate);
    }
    else aLicensedUntil = nsDependentCString("unlicensed");
    return NS_OK;
}

/* void hello (); */
NS_IMETHODIMP msiAppUtils::Hello()
{
  int stat;
  char *product = "swp";    
  char *licensedProd;
  PRInt32 count = 1;
  PRInt32 days;
  const char * utf8Path;
  const char * ver = "6.0";
  nsString path;
  PRBool done;
  PRUint32 prodnum;

  if (lic == nsnull || rlm_license_stat(lic) != 0)
  {
    nsCOMPtr<nsIProperties> fileLocator(do_GetService("@mozilla.org/file/directory_service;1"));
    nsCOMPtr<nsILocalFile> licFile;
    fileLocator->Get("resource:app", NS_GET_IID(nsIFile), getter_AddRefs(licFile));
    licFile->Append(NS_LITERAL_STRING("license.lic"));
    licFile->GetPath(path);
    utf8Path = ToNewUTF8String(path);


    rh = rlm_init(utf8Path, (char *)nsnull, (char *) nsnull);
    stat = rlm_stat(rh);
    if (stat)
    {
      char errstring[RLM_ERRSTRING_MAX];

      (void) printf("Error initializing license system\n");
      (void) printf("%s\n", rlm_errstring((RLM_LICENSE) nsnull, rh, 
                  errstring));
    }
    else
    {
      syzygy = 0;
      prodnum = 3;
      done = PR_FALSE;
      while (!done) {
        lic = rlm_checkout(rh, product, ver, count);
        stat = rlm_license_stat(lic);
        if (! stat) {
          printf("License is valid\n");
          days= rlm_license_exp_days(lic);
          pchProdName = rlm_license_product(lic);
          pchExpDate = rlm_license_exp(lic);
          syzygy = prodnum;
          done = PR_TRUE;
        }  
        else {
          if (strcmp(product,"swp") == 0) {
            product = "sw";
            prodnum = 2;
          }
          else if (strcmp(product, "sw") == 0) {
            product = "snb";
            prodnum = 1;
          }
          else {
            printf("Invalid license\n");
            days = -1;
            pchProdName = "";
            pchExpDate = "unlicensed";
            done = PR_TRUE;
          }
        }
      }
    }
  }
  return NS_OK;
}

/* void goodbye (); */
NS_IMETHODIMP msiAppUtils::Goodbye()
{
  rlm_close(rh);
  return NS_OK;
}

