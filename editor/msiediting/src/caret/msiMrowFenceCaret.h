// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiMrowFenceCaret_h___
#define msiMrowFenceCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"

class nsIPresShell;
struct nsRect;

class msiMrowFenceCaret : public msiMContainerCaret
{
public:
  msiMrowFenceCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
  //msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
protected:
  void GetThresholds(nsIPresShell* shell, const nsRect &frameRect, 
                     const nsRect &openRect, const nsRect &closeRect, 
                     PRInt32 &left, PRInt32 & right);

};

#endif // msiMrowFenceCaret_h___
