// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMencloseCaret_h___
#define msiMencloseCaret_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMContainerCaret.h"

class nsIPresShell;
struct nsRect;

class msiMencloseCaret : public msiMContainerCaret
{
public:
  msiMencloseCaret(nsIDOMNode* mathmlNode, PRUint32 offset);
  msiMencloseCaret(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  // msiIMathmlCaret overrides
  NS_IMETHOD GetNodeAndOffsetFromMouseEvent(nsIEditor *editor, 
                                            nsIPresShell *presShell, 
                                            PRUint32 flags, 
                                            nsIDOMMouseEvent * mouseEvent,
                                            nsIDOMNode **node, 
                                            PRUint32 *offset);
                                            
  NS_IMETHOD AdjustNodeAndOffsetFromMouseEvent(nsIEditor *editor, nsIPresShell *presShell,
                                               PRUint32 flags, 
                                               nsIDOMMouseEvent *mouseEvent, 
                                               nsIDOMNode **node, 
                                               PRUint32 *offset);
                                       
  NS_IMETHOD AdjustSelectionPoint(nsIEditor *editor, PRBool leftSelPoint, 
                                  nsIDOMNode** selectionNode, PRUint32 * selectionOffset,
                                  msiIMathMLCaret** parentCaret);
  NS_IMETHOD SetupDeletionTransactions(nsIEditor * editor,
                                       PRUint32 startOffset,
                                       PRUint32 endOffset,
                                       nsIDOMNode * start,
                                       nsIDOMNode * end,
                                       nsIArray ** transactionList);


  NS_IMETHOD CaretObjectLeft(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset);
  NS_IMETHOD CaretObjectRight(nsIEditor *editor, PRUint32 flags, nsIDOMNode ** node, PRUint32 *offset);
  
  // end msiIMathmlCaret overrides
  
protected:
void  GetThresholds(const nsRect &encloseRect, 
                    const nsRect &firstKidRect, const nsRect& lastKidRect,
                    PRInt32 &left, PRInt32 & right);
  
};

#endif // msiMencloseCaret_h___
