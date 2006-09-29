// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiUnderAndOrOverCaret_h___
#define msiUnderAndOrOverCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "msiMCaretBase.h"

class nsIPresShell;
class nsIFrame;
struct nsRect;

class msiUnderAndOrOverCaret : public msiMCaretBase
{
public:
  msiUnderAndOrOverCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);

  //override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
protected:
  nsresult GetFramesAndRects(const nsIFrame * underOver, 
                             nsIFrame ** base, nsIFrame ** script1, nsIFrame ** script2,
                             nsRect& sRect, nsRect& bRect, nsRect& s1Rect, nsRect& s2Rect);
                           
  void GetAboveBelowThresholds(const nsRect& uoRect, const nsRect& bRect, const nsRect& s1Rect, nsRect& s2Rect,
                               PRInt32& above, PRBool& aboveSet, PRInt32& below, PRBool& belowSet);
};

#endif // msiUnderAndOrOverCaret_h___
