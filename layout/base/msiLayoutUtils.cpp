#include "msiLayoutUtils.h"

#include  "nsLayoutUtils.h"
#include  "nsIFrame.h"
#include "nsIPrivateDOMEvent.h"
#include "nsGUIEvent.h"
#include "nsHTMLReflowState.h"  //for NS_UNCONSTRAINEDSIZE
#include "nsFrame.h"

msiLayoutUtils::msiLayoutUtils()
{
  NS_ASSERTION( NS_UNCONSTRAINEDSIZE==NS_MAXSIZE, "NS_UNCONSTRAINEDSIZE NS_MAXSIZE");
}

msiLayoutUtils::~msiLayoutUtils()
{
}

NS_IMPL_ISUPPORTS1(msiLayoutUtils, msiILayoutUtils)

nsresult MSI_NewLayoutUtils(msiILayoutUtils** aInstancePtrResult)
{
  NS_PRECONDITION(aInstancePtrResult, "null ptr");
  
  msiLayoutUtils* _msiLayoutUtils = new msiLayoutUtils();
  if (nsnull == _msiLayoutUtils)
      return NS_ERROR_OUT_OF_MEMORY;
      
  return _msiLayoutUtils->QueryInterface(NS_GET_IID(msiILayoutUtils), (void**) aInstancePtrResult);
}
