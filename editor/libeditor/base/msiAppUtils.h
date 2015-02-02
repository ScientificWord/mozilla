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

protected:
  PRInt32 mLicensedApp;
  nsString mLicensedUntil;
  RLM_HANDLE mrh;
  RLM_LICENSE mLic;
};

#endif