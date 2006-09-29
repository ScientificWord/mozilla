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

NS_IMETHODIMP 
msiLayoutUtils::GetOffsetIntoTextFromEvent(nsIFrame *textFrame, nsIDOMEvent *domEvent, PRUint32 *offset)
{
  if (!textFrame || !domEvent)
    return NS_ERROR_FAILURE;
  nsresult res(NS_ERROR_FAILURE);
  nsEvent* nsEvent = nsnull;
  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(domEvent));
  if (privateEvent)
     res = privateEvent->GetInternalNSEvent(&nsEvent);
  if (NS_SUCCEEDED(res) && nsEvent && (textFrame->GetType() == nsLayoutAtoms::textFrame))
  {
    res = NS_ERROR_FAILURE;
    nsPoint pt = nsLayoutUtils::GetEventCoordinatesRelativeTo(nsEvent, textFrame);
    if (pt.x != NS_UNCONSTRAINEDSIZE)
    {
      nsIFrame::ContentOffsets offsets = textFrame->GetContentOffsetsFromPoint(pt);
      if (offsets.offset >=0)
      {
        *offset = offsets.offset;
        res = NS_OK;
      }  
    }    
  }
  else
    res = NS_ERROR_FAILURE;
  return res;  
}

nsresult MSI_NewLayoutUtils(msiILayoutUtils** aInstancePtrResult)
{
  NS_PRECONDITION(aInstancePtrResult, "null ptr");
  
  msiLayoutUtils* _msiLayoutUtils = new msiLayoutUtils();
  if (nsnull == _msiLayoutUtils)
      return NS_ERROR_OUT_OF_MEMORY;
      
  return _msiLayoutUtils->QueryInterface(NS_GET_IID(msiILayoutUtils), (void**) aInstancePtrResult);
}
