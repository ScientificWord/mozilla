// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMCaretBase_h___
#define msiMCaretBase_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiIMathMLCaret.h"
#include "msiMEditingBase.h"

class nsIPresShell;
class nsIDOMEvent;
class nsIFrame;
struct nsPoint;

class msiMCaretBase : public msiMEditingBase, 
                      public msiIMathMLCaret
{
public:
  msiMCaretBase(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  virtual ~msiMCaretBase();
  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED
  
  //msiIMathMLCaret
  NS_DECL_MSIIMATHMLCARET
  
  // Mouse Util functions
  static nsresult GetPrimaryFrameForNode(nsIPresShell * presShell, nsIDOMNode * node, nsIFrame ** frame);
  static nsresult GetNodeFromFrame(nsIFrame* frame, nsCOMPtr<nsIDOMNode> & node);
  // End Mouse Util functions
  
  // Utility Selection functions
  nsresult GetSelectionPoint(nsIEditor * editor, PRBool leftSelPoint,
                             nsIDOMNode * startNode, PRUint32 startOffset,
                             nsCOMPtr<nsIDOMNode> & splitNode, PRUint32 & splitOffset);
  // End Utility Selection functions
};

#endif // msiMCaretBase_h___
