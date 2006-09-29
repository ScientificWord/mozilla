// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiBigOpCaret_h___
#define msiBigOpCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "msiMCaretBase.h"
#include "msiIBigOpInfo.h"

class nsIPresShell;
class nsIFrame;
struct nsRect;

class msiBigOperatorCaret : public msiMCaretBase
{
public:
  msiBigOperatorCaret(nsIDOMNode* mathmlNode, PRUint32 offset, nsCOMPtr<msiIBigOpInfo> & bigOpInfo);

  //override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
protected:
  nsresult GetFramesAndRects(const nsIFrame * script, 
                             nsIFrame ** base, nsIFrame ** script1, nsIFrame ** script2,
                             nsRect& sRect, nsRect& bRect, nsRect& s1Rect, nsRect& s2Rect,
                             PRBool & isAboveBelow);
                             
  void GetAtRightThresholds(const nsRect& sRect, const nsRect& bRect, const nsRect& s1Rect, nsRect& s2Rect,
                           PRInt32& left, PRInt32& right);
                           
  void GetAboveBelowThresholds(const nsRect& sRect, const nsRect& bRect, const nsRect& s1Rect, nsRect& s2Rect,
                               PRInt32& above, PRBool& aboveSet, PRInt32& below, PRBool& belowSet);
                           
  
protected:
  nsCOMPtr<msiIBigOpInfo> m_bigOpInfo;                             
  
};

#endif // msiBigOpCaret_h___
