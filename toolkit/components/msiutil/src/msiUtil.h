#include "msiIUtil.h"
#include "nsCOMPtr.h"
#include "nsString.h"

class msiUtil : public msiIUtil
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_MSIIUTIL
      
  msiUtil();
  static msiUtil* GetInstance();
private:
  ~msiUtil();
};
