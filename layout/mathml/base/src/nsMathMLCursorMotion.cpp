#include "nsMathMLCursorMotion.h"

// nsISupports
// =============================================================================

NS_IMPL_ISUPPORTS1(nsMathMLCursorMotion, nsIMathMLCursorMotion)
 
/* We need implementations of the four cursor movement helper functions, but these have to be stubs
 * since the real implementation has to have access to the tree structure of frames and, ironically, 
 * nsIMathMLFrame is not a subclass of nsIFrame.  
 */

NS_IMETHODIMP 
nsMathMLCursorMotion::MoveOutToRight(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32 *aOutOffset, PRInt32 count, PRInt32 *_retval)
{
  printf("MoveOutToRight implementation fell through to nsMathMLCursorMotion\n");
  *_retval = count;
  return NS_OK;
}

NS_IMETHODIMP 
nsMathMLCursorMotion::MoveOutToLeft(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32 *aOutOffset, PRInt32 count, PRInt32 *_retval)
{
  printf("MoveOutToLeft implementation fell through to nsMathMLCursorMotion\n");
  *_retval = count;
  return NS_OK;
}

NS_IMETHODIMP 
nsMathMLCursorMotion::EnterFromLeft(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32 count, PRInt32 *_retval)
{
  printf("EnterFromLeft implementation fell through to nsMathMLCursorMotion\n");
  *_retval = count;
  return NS_OK;
}

NS_IMETHODIMP 
nsMathMLCursorMotion::EnterFromRight(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32 count, PRInt32 *_retval)
{
  printf("EnterFromRight implementation fell through to nsMathMLCursorMotion\n");
  *_retval = count;
  return NS_OK;
}

