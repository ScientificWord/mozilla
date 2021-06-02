#include "nsMathMLCursorMover.h"

// nsISupports
// =============================================================================

NS_IMPL_ISUPPORTS1(nsMathMLCursorMover, nsIMathMLCursorMover)
 
/* We need implementations of the four cursor movement helper functions, but these have to be stubs
 * since the real implementation has to have access to the tree structure of frames and, ironically, 
 * nsIMathMLFrame is not a subclass of nsIFrame.  
 */

NS_IMETHODIMP 
nsMathMLCursorMover::MoveOutToRight(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32 *aOutOffset, PRInt32 count,
  PRBool* fBailing, PRInt32 *_retval)
{
#ifdef debug_barry
  printf("MoveOutToRight implementation fell through to MathMLCursorMover\n");
#endif  
  *_retval = count;
  return NS_OK;
}

NS_IMETHODIMP 
nsMathMLCursorMover::MoveOutToLeft(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32 *aOutOffset, PRInt32 count,
  PRBool* fBailing, PRInt32 *_retval)
{
#ifdef debug_barry
  printf("MoveOutToLeft implementation fell through to MathMLCursorMover\n");
#endif
  *_retval = count;
  return NS_OK;
}

NS_IMETHODIMP 
nsMathMLCursorMover::EnterFromLeft(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32 *aOutOffset, PRInt32 count,
  PRBool* fBailing, PRInt32 *_retval)
{
#ifdef debug_barry  
  printf("EnterFromLeft implementation fell through to MathMLCursorMover\n");
#endif
  *_retval = count;
  return NS_OK;
}

NS_IMETHODIMP 
nsMathMLCursorMover::EnterFromRight(nsIFrame *leavingFrame, nsIFrame **aOutFrame, PRInt32 *aOutOffset, PRInt32 count,
  PRBool* fBailing, PRInt32 *_retval)
{
#ifdef debug_barry
  printf("EnterFromRight implementation fell through to MathMLCursorMover\n");
#endif
  *_retval = count;
  return NS_OK;
}

