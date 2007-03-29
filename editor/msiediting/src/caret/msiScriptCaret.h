// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiScriptCaret_h___
#define msiScriptCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class nsIPresShell;
class nsIFrame;
struct nsRect;


class msiScriptCaret : public msiMCaretBase
{
public:
  msiScriptCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  
  // override msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
  
protected:
  nsresult GetFramesAndRects(const nsIFrame * script, 
                             nsIFrame ** base, nsIFrame ** script1, nsIFrame ** script2,
                             nsRect & sRect, nsRect &bRect, nsRect& s1Rect, nsRect& s2Rect);
                             
  void GetThresholds(const nsRect &sRect, 
                     const nsRect &bRect, const nsRect& s1Rect, nsRect& s2Rect,
                     PRInt32 &left, PRInt32& midLf, PRInt32& midRt, PRInt32 & right);
};

#endif // msiScriptCaret_h___
