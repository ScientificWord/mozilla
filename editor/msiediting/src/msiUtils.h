// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiUtils_h___
#define msiUtils_h___

#include "nsIEditor.h"
#include "nsIDOMElement.h"
#include "nsIDOMNode.h"
#include "nsIDOMMouseEvent.h"
#include "nsISelection.h"
#include "nsINameSpaceManager.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIArray.h"
#include "nsEvent.h"
#include "nsPoint.h"
#include "nsIMutableArray.h"
#include "msiIMathMLInsertion.h"
#include "msiIMathMLCaret.h"
#include "msiIMathMLEditingBC.h"
#include "msiILayoutUtils.h"


class msiUtils
{
public:
  static void Initalize();
  static PRInt32  GetMathMLNodeTypeFromCharacter(PRUint32 character);
  
  static nsresult GetMathMLEditingBC(nsIEditor *editor,
                                     nsIDOMNode * node,
                                     PRUint32   offset,
                                     bool       clean,
                                     nsCOMPtr<msiIMathMLEditingBC> & editingBC);
                                     
  static nsresult GetMathMLInsertionInterface(nsIEditor *editor,
                                              nsIDOMNode * node,
                                              PRUint32   offset,
                                              nsCOMPtr<msiIMathMLInsertion> & msiEditing);

  static nsresult SetupPassOffInsertToParent(nsIEditor * editor,
                                             nsIDOMNode * child,
                                             PRBool incrementOffset,
                                             nsCOMPtr<msiIMathMLInsertion> & msiEditing);
                                             
  static nsresult GetMathMLCaretInterface(nsIEditor *editor,
                                          nsIDOMNode * node,
                                          PRUint32   offset,
                                          nsCOMPtr<msiIMathMLCaret> & msiEditing);

  static nsresult SetupPassOffCaretToParent(nsIEditor * editor,
                                            nsIDOMNode * child,
                                            PRBool incrementOffset,
                                            nsCOMPtr<msiIMathMLCaret> & msiEditing);
                                            
  static nsresult CreateMathElement(nsIEditor *editor, 
                                    PRBool isDisplay,
                                    PRBool markCaret,
                                    PRUint32 & flags,
                                    nsCOMPtr<nsIDOMElement> & mathElement);
 
  static nsresult CreateMathMLLeafElement(nsIEditor *editor, 
                                          const nsAString & text,
                                          PRUint32 caretPos,
                                          PRUint32 & flags,
                                          nsCOMPtr<nsIDOMElement> & mathmlElement);
                                          
  static nsresult CreateMathMLLeafElement(nsIEditor *editor,
                                          PRUint32 character,
                                          PRUint32 tagType, 
                                          PRUint32 caretPos,
                                          PRUint32 & flags,
                                          nsCOMPtr<nsIDOMElement> & mathmlElement);
                                          
  static nsresult CreateMathMLLeafElement(nsIEditor *editor,
                                          const nsAString & text,
                                          PRUint32 tagType, 
                                          PRUint32 caretPos,
                                          PRUint32 & flags,
                                          nsCOMPtr<nsIDOMElement> & mathmlElement);
                                          
  static nsresult CreateInputbox(nsIEditor *editor, 
                                 PRBool nestInRow,
                                 PRBool markCaret,
                                 PRUint32 & flags,
                                 nsCOMPtr<nsIDOMElement> & mathmlElement);
                                 
  static nsresult CreateMathOperator(nsIEditor * editor,
                                     const nsAString & text,
                                     PRUint32 caretPos,
                                     PRUint32 & flags,
                                     PRUint32 attribflags,
                                     const nsAString & lspace,                                 
                                     const nsAString & rspace,                                 
                                     const nsAString & minsize,                                 
                                     const nsAString & maxsize,
                                     nsCOMPtr<nsIDOMElement> & mathmlElement);
                                 
  static nsresult CreateMSubOrMSup(nsIEditor *editor,
                                   PRBool isSup,
                                   nsIDOMNode * base,
                                   nsIDOMNode * script,
                                   PRBool scriptInRow,
                                   PRBool markCaret,
                                   PRUint32 & flags,
                                   const nsAString & scriptShift,                                 
                                   nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateMSubSup(nsIEditor * editor,
                                nsIDOMNode * base,
                                nsIDOMNode * subscript,
                                nsIDOMNode * supscript,
                                PRBool subscriptInRow,
                                PRBool supscriptInRow,
                                PRBool markCaret,
                                PRUint32 & flags,
                                const nsAString & subscriptShift,                                 
                                const nsAString & supscriptShift,                                 
                                nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateMunderOrMover(nsIEditor *editor,
                                      PRBool isOver,
                                      nsIDOMNode * base,
                                      nsIDOMNode * script,
                                      PRBool scriptInRow,
                                      PRBool createInputBox,
                                      PRBool markCaret,
                                      PRUint32 & flags,
                                      const nsAString & isAccent,
                                      nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateMunderover(nsIEditor * editor,
                                   nsIDOMNode * base,
                                   nsIDOMNode * underscript,
                                   nsIDOMNode * overscript,
                                   PRBool underscriptInRow,
                                   PRBool overscriptInRow,
                                   PRBool createInputBox,
                                   PRBool markCaret,
                                   PRUint32 & flags,
                                   const nsAString & underIsAccent,
                                   const nsAString & overIsAccent,
                                   nsCOMPtr<nsIDOMElement> & mathmlElement);
 
  static nsresult CreateMenclose(nsIEditor *editor,
                                 const nsAString & around,
                                 const nsAString & typeAttr,
                                 nsIDOMNode * child,
                                 PRBool makeInputBox,
                                 PRBool markCaret,
                                 PRUint32 & flags,
                                 nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateMsqrt(nsIEditor *editor,
                              nsIDOMNode * child,
                              PRBool createInputBox,
                              PRBool markCaret,
                              PRUint32 & flags,
                              nsCOMPtr<nsIDOMElement> & mathmlElement);
                              
  static nsresult CreateMroot(nsIEditor *editor,
                              nsIDOMNode * radicand,
                              nsIDOMNode * index,
                              PRBool createInputBox,
                              PRBool markCaret,
                              PRUint32 & flags,
                              nsCOMPtr<nsIDOMElement> & mathmlElement);
                              
  static nsresult CreateMfrac(nsIEditor * editor,
                              nsIDOMNode * num,
                              nsIDOMNode * denom,
                              PRBool createInputBox,
                              PRBool markCaret,
                              PRUint32 & flags,
                              const nsAString & lineThickness,
                              PRUint32 & attrFlags,
                              nsCOMPtr<nsIDOMElement> & mathmlElement);
                              
  static nsresult CreateBinomial(nsIEditor * editor,
                                 nsIDOMNode * num,
                                 nsIDOMNode * denom,
                                 PRBool createInputBox,
                                 PRBool markCaret,
                                 PRUint32 & flags,
                                 const nsAString & opening,
                                 const nsAString & closing,
                                 const nsAString & lineThickness,
                                 PRUint32 & attrFlags,
                                 nsCOMPtr<nsIDOMElement> & mathmlElement);
                              
  static nsresult CreateMathname(nsIEditor * editor,
                                 const nsAString & name,
                                 PRUint32 & flags,
                                 PRBool isUnit,
                                 nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateEngineFunction(nsIEditor * editor,
                                       const nsAString & name,
                                       PRUint32 & flags,
                                       nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateMRowFence(nsIEditor * editor,
                                  nsIDOMNode * child,
                                  PRBool createInputBox,
                                  const nsAString & open,
                                  const nsAString & close,
                                  PRBool markCaret,
                                  PRUint32 & flags,
                                  PRUint32 & attrFlags,
                                  nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateMRow(nsIEditor *editor,
                             nsIDOMNode * child,
                             nsCOMPtr<nsIDOMElement> & mathmlElement);
                             
  static nsresult CreateMRow(nsIEditor *editor,
                             nsIArray * nodeArray,
                             nsCOMPtr<nsIDOMElement> & mathmlElement);
                             
  static nsresult CreateMtd(nsIEditor *editor,
                            PRBool markCaret,
                            PRUint32 & flags,
                            nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateMtr(nsIEditor * editor,
                            PRUint32 numCols,
                            PRBool isLabeledTr, // label not in col count
                            PRBool markCaret,
                            PRUint32 & flags,
                            nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CreateMtable(nsIEditor * editor,
                               PRUint32 numRows,
                               PRUint32 numCols,
                               const nsAString & rowSignature,
                               PRBool markCaret,
                               PRUint32 & flags,
                               nsCOMPtr<nsIDOMElement> & mathmlElement,
                               const nsAString & delim);
                               
  static nsresult CreateDecoration(nsIEditor * editor,
                                   nsIDOMNode * child,
                                   const nsAString & above,
                                   const nsAString & below,
                                   const nsAString & aroundNotation,
                                   const nsAString & aroundType,
                                   PRBool createInputBox,
                                   PRBool markCaret,
                                   PRUint32 & flags,
                                   nsCOMPtr<nsIDOMElement> & mathmlElement);
                               
  static nsresult CreateContainer(nsIEditor *editor,
                                  const nsAString & elementName,
                                  nsIDOMNode * cloneMyKids,
                                  nsCOMPtr<nsIDOMElement> & mathmlElement);

  static nsresult CloneNode(nsIDOMNode * node,
                            nsCOMPtr <nsIDOMNode> & clone);

  static nsresult CloneChildNode(nsIDOMNode * parent,
                                 PRUint32 indexOfChild,
                                 nsCOMPtr <nsIDOMNode> & clone);

  static nsresult WrapNodeInMStyle(nsIEditor * editor,
                                   nsIDOMNode * node,
                                   nsCOMPtr<nsIDOMElement> & mathmlElement);
                              
  static nsresult GetRightMostCaretPosition(nsIEditor * editor,
                                            nsIDOMNode * node,
                                            PRUint32 & position);
                                          
  static nsresult GetNumberofChildren(nsIDOMNode * node,
                                      PRUint32 & number);
                                          
  static nsresult GetIndexOfChildInParent(nsIDOMNode * child, 
                                          PRUint32 & index);
                                          
  static nsresult GetNonWhitespaceChildren(nsIDOMNode * parent,
                                           nsCOMPtr<nsIArray> & children);
                                          
  static nsresult GetChildNode(nsIDOMNode * parent,
                               PRUint32 indexOfChild,
                               nsCOMPtr<nsIDOMNode> & child);                                          
                               
  static nsresult RemoveChildNode(nsIDOMNode * parent,
                                  PRUint32 indexOfChild,
                                  nsCOMPtr<nsIDOMNode> & removedChild);


  static nsresult RemoveIndexedChildNode(nsIEditor * editor,
                                   nsIDOMNode * parent,
                                   PRUint32 indexOfChild,
                                   nsCOMPtr<nsIDOMNode> & removedChild);

                                  
  static nsresult ReplaceChildNode(nsIDOMNode * parent,
                                   PRUint32 indexOfChild,
                                   nsIDOMNode * newChild,
                                   nsCOMPtr<nsIDOMNode> & replacedChild);
                                   
  static nsresult InsertChildren(nsIDOMNode * parent, 
                                 PRUint32 offset, 
                                 nsIArray * newChildList);

  static nsresult ClearCaretPositionMark(nsIEditor * editor, 
                                         nsIDOMNode * node,
                                         PRBool clearAll);
                                         
  static PRBool NodeHasCaretMark(nsIDOMNode * node,
                                 PRUint32 & pos,
                                 PRBool & caretOnText);

  static nsresult MarkCaretPosition(nsIEditor * editor,
                                    nsIDOMNode * node,
                                    PRUint32 pos,
                                    PRUint32 & flags,
                                    PRBool caretOnText,
                                    PRBool overwrite);
                 
  static nsresult doSetCaretPosition(nsIEditor * editor,
                                     nsISelection * selection,
                                     nsIDOMNode * rootnode);

  static nsresult doSetCaretPosition(nsISelection * selection,
                                     nsIDOMNode * node,
                                     PRUint32 offset);
                                     
  static nsresult GetNSEventFromMouseEvent(nsIDOMMouseEvent * mouseEvent, nsEvent ** nsEvent);
  
  static nsresult GetScreenPointFromMouseEvent(nsIDOMMouseEvent * mouseEvent, nsPoint & point);                                     
                                     
  static PRBool IsWhitespace(nsIDOMNode * node);
                                       
  static PRBool IsInputbox(nsIEditor * editor,
                           nsIDOMNode * node);       
                                                 
  static PRBool IsInputbox(nsISupports * isupports);

  static PRBool hasMMLType(nsIEditor * editor,	nsIDOMNode * node, 	unsigned short mmlType);

  
  static PRBool IsMrow(nsIEditor* editor,
                       nsIDOMNode * node);       
                                                 
  static PRBool IsMrow(nsISupports * isupports);
  
  static PRBool IsMleaf(nsIEditor* editor,
                        nsIDOMNode * node,
                        PRBool allowInputbox);       
                                                 
  static PRBool IsMleaf(nsISupports* isupports,
                        PRBool allowInputbox);

  static PRBool IsEmpty(nsIDOMNode* pNode);
  


                        
  static nsresult GetMathmlNodeType(nsIEditor * editor,
                                    nsIDOMNode * node,
                                    PRUint32 & nodetype);  
                                    
  static PRUint32 GetMathmlNodeType(nsISupports * isupports);
  
  static nsresult GetMathTagParent(nsIDOMNode * node,
                                   nsIAtom * elementAtom,
                                   nsCOMPtr<nsIDOMNode> & tagParent);

  static nsresult GetMathParent(nsIDOMNode * node,
                                nsCOMPtr<nsIDOMNode> & mathParent);

  static nsresult GetTableCell(nsIEditor* editor,
                               nsIDOMNode * node,
                               nsCOMPtr<nsIDOMNode> & mtdCell);
                        
  static nsresult CreateMathMLElement(nsIEditor* editor, nsIAtom * type, 
                                      nsCOMPtr<nsIDOMElement> & mmlElement);
                                      
  static nsresult AddToNodeList(nsIArray* nodeList, 
                                nsIArray * addToFront, 
                                nsIArray * addToEnd, 
                                nsCOMPtr<nsIArray> & pArray);
                                
  static nsresult AppendToMutableList(nsCOMPtr<nsIMutableArray> & mutableList, 
                                      nsCOMPtr<nsIArray> & tobeAdded);
                                
  static nsresult RemoveNodesFromList(nsIArray * nodeList, 
                                      PRUint32 index,
                                      PRUint32 count,
                                      nsCOMPtr<nsIArray> & pArray);
                                      
  static nsresult GetMathmlNodeFromCaretInterface(nsCOMPtr<msiIMathMLCaret> & caret,
                                                  nsCOMPtr<nsIDOMNode> & mathmlNode);
                                                  
  static nsresult GetOffsetFromCaretInterface(nsCOMPtr<msiIMathMLCaret> & caret,
                                              PRUint32 & offset);
                                              
  static nsresult ComparePoints(nsIEditor * editor,
                                nsIDOMNode * node1, PRUint32 offset1,
                                nsIDOMNode * node2, PRUint32 offset2,
                                PRInt32 & comparison);

  static nsresult SplitNode(nsIDOMNode * node, PRUint32 offset,
                            PRBool emptyOK,
                            nsCOMPtr<nsIDOMNode> & left, 
                            nsCOMPtr<nsIDOMNode> & right);

//TODO - how should this be determined -- user preference??
  enum {MROW_PURGE_NONE = 0, MROW_PURGE_BOUNDARY, MROW_PURGE_ALL};
  static PRUint32  GetMrowPurgeMode();
                                      
};                  

#endif // msiUtils_h___
