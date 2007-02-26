// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMfracCaret_h___
#define msiMfracCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class nsIPresShell;
class nsIFrame;
struct nsRect;

class msiMfracCaret : public msiMCaretBase
{
public:
  msiMfracCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
  //msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
                                         
protected:

nsresult  GetFramesAndRects(const nsIFrame * frac, 
                            nsIFrame ** numer, nsIFrame ** denom,
                            nsRect & fRect, nsRect & nRect, nsRect & dRect);
void GetThresholds(const nsRect &fRect, 
                  const nsRect &nRect, const nsRect& dRect,
                  PRInt32 &left, PRInt32 & right);
};

#endif // msiMfracCaret_h___
