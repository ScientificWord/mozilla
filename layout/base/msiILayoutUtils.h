#ifndef msiILayoutUtils_h__
#define msiILayoutUtils_h__

#include "nsISupports.h"

class nsIFrame;
class nsIDOMEvent;
struct nsRect;
struct nsPoint;

#define MSI_ILAYOUTUTILS_IID_STR "e4e51a3e-e446-49d6-878e-770b1957869b"

#define MSI_ILAYOUTUTILS_IID \
  {0xe4e51a3e, 0xe446, 0x49d6, \
    { 0x87, 0x8e, 0x77, 0x0b, 0x19, 0x57, 0x86, 0x9b }}

class msiILayoutUtils : public nsISupports {
 public: 

  NS_DECLARE_STATIC_IID_ACCESSOR(MSI_ILAYOUTUTILS_IID)
};

  NS_DEFINE_STATIC_IID_ACCESSOR(msiILayoutUtils, MSI_ILAYOUTUTILS_IID)

#endif /* msiILayoutUtils_h__ */
