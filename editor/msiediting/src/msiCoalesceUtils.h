// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiCoalesceUtils_h___
#define msiCoalesceUtils_h___

#include "nsIEditor.h"
#include "nsIDOMNode.h"
#include "nsCOMPtr.h"
#include "nsIArray.h"
#include "nsIMutableArray.h"
#include "nsITransaction.h"
#include "msiIMathMLCoalesce.h"
class msiCoalesceUtils
{
public:

static nsresult GetMathMLCoalesceInterface(nsIEditor *editor,
                                           nsIDOMNode * node,
                                           PRUint32   offset,
                                           nsCOMPtr<msiIMathMLCoalesce> & msiEditing);

static void SetCoalesceSwitch(nsCOMPtr<nsIDOMNode> & node, PRBool start);

static void SetCoalesceSwitch(nsIArray * inArray, PRUint32 index, PRBool start);

static nsresult CoalesceArray(nsIEditor * editor,
                              nsIArray  * inputArray,
                              nsCOMPtr<nsIArray>  & coalescedArray);

static PRBool Coalesce(nsIEditor * editor,
                       nsIDOMNode * node1,  
                       nsIDOMNode * node2, 
                       nsCOMPtr<nsIArray> & coalesced);

static nsresult PrepareForCoalesce(nsIEditor * editor,
                                   nsIDOMNode * node,
                                   PRUint32 offset,
                                   PRUint32 pfcFlags,
                                   nsCOMPtr<nsIArray> & beforeOffset,                              
                                   nsCOMPtr<nsIArray> & afterOffset);                              
                              
static nsresult PrepareForCoalesceFromRight(nsIEditor * editor,
                                            nsIDOMNode * node,
                                            PRUint32 pfcFlags,
                                            nsCOMPtr<nsIArray> & preparedArray);                              
static nsresult PrepareForCoalesceFromLeft(nsIEditor * editor,
                                           nsIDOMNode * node,
                                           PRUint32 pfcFlags,
                                           nsCOMPtr<nsIArray> & preparedArray);                              
                                           
//Transaction based coalescing
                                           
static PRBool Coalesce(nsIEditor * editor,
                       nsIDOMNode * node1,  
                       nsIDOMNode * node2, 
                       nsCOMPtr<nsITransaction> & txn);
                                           
                              
// Should only be called when caretpos is known to be associated to node.
static nsresult ForceCaretPositionMark(nsIDOMNode * node, 
                                       PRUint32 pos, 
                                       PRBool caretOnText);
// Should only be called when caretpos is known to be associated to node.
static nsresult ForceRemovalOfCaretMark(nsIDOMNode * node);
                              
                       
};                  

#endif // msiCoalesceUtils_h___
