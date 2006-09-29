// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiMrowBoundFenceCaret_h___
#define msiMrowBoundFenceCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCaretBase.h"

class nsIPresShell;
struct nsRect;

class msiMrowBoundFenceCaret : public msiMCaretBase
{
public:
  msiMrowBoundFenceCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
  //msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET

protected:
  void GetThresholds(const nsRect &frameRect, 
                     const nsRect &openRect, const nsRect &closeRect, 
                     PRInt32 &left, PRInt32 & right);
protected:
};

#endif // msiMrowBoundFenceCaret_h___
