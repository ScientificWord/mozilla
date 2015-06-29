#ifndef __msiAppUtils_h__
#define __msiAppUtils_h__

#include "license.h"
#include "msiIAppUtils.h"

class msiAppUtils : public msiIAppUtils {
  public:
    NS_DECL_ISUPPORTS
    NS_DECL_MSIIAPPUTILS

    msiAppUtils();

    // NS_IMETHOD LicensedApp(PRUint32 appNum, PRBool *_retval);
    // NS_IMETHOD GetLicensedUntil(nsACString & aLicensedUntil);
    // NS_IMETHOD GetHostid(nsACString & aHostid);
    // NS_IMETHOD Hello(PRUint32 appNum);
    // NS_IMETHOD Goodbye(void);
    static PRBool rlm_compute_ok ();
    static PRBool rlm_tex_ok();
    static PRBool rlm_save_ok ();
    static char * getProd();

  private:
    ~msiAppUtils();
    static PRUint32 licensedProd;
    static char * pchProdName;
    static char * pchExpDate;
    static RLM_HANDLE rh;
    static RLM_LICENSE lic;
};

#endif
