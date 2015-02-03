#ifndef __msiAppUtils_h__
#define __msiAppUtils_h__

#include "license.h"
#include "msiIAppUtils.h"

class msiAppUtils : public msiIAppUtils
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MSIIAPPUTILS

  msiAppUtils();

private:
  ~msiAppUtils();
  static PRUint32 licensedProd;
  static char * pchProdName;
  static char * pchExpDate;
  static RLM_HANDLE rh;
  static RLM_LICENSE lic;
};

#endif