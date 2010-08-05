// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiEditingManager_h__
#define msiEditingManager_h__

#include "nsCOMPtr.h"
#include "msiIEditingManager.h"
#include "msiBigOpInfoImp.h"
#include "nsTArray.h"


class msiMultiSpanCellInfo {
public:
  msiMultiSpanCellInfo() : m_pNode(0), m_startRow(0), m_startCol(0), m_rowSpan(1), m_colSpan(1) {}
  msiMultiSpanCellInfo(const msiMultiSpanCellInfo& src);

  msiMultiSpanCellInfo& operator=(const msiMultiSpanCellInfo& src);
  PRBool operator==(const msiMultiSpanCellInfo& comp) const
  { return ( (m_pNode == comp.m_pNode) && (m_startRow == comp.m_startRow) && (m_startCol == comp.m_startCol)
      && (m_rowSpan == comp.m_rowSpan) && (m_colSpan == comp.m_colSpan) ); }
  PRBool operator<(const msiMultiSpanCellInfo& comp) const
  { return ( (m_startRow < comp.m_startRow) || ((m_startRow == comp.m_startRow) && (m_startCol < comp.m_startCol)) ); }

  PRBool SetSpansFromCell(nsIDOMNode* aCell);
  void SetStartPos(PRUint32 nRow, PRUint32 nCol) {m_startRow = nRow; m_startCol = nCol;}

  PRUint32 StartRow() const {return m_startRow;}
  PRUint32 StartCol() const {return m_startCol;}
  PRUint32 RowSpan() const {return m_rowSpan;}
  PRUint32 ColSpan() const {return m_colSpan;}
  nsIDOMNode* Node() const {return m_pNode;}

private:
  nsCOMPtr<nsIDOMNode> m_pNode;
  PRUint32 m_startRow, m_startCol;
  PRUint32 m_rowSpan, m_colSpan;
};


/** implementation of a msi editing manager object.
 *
 */
class msiEditingManager : public msiIEditingManager
{
public:

  msiEditingManager();

  ~msiEditingManager();

  NS_DECL_ISUPPORTS
  NS_DECL_MSIIEDITINGMANAGER

//The following are intended to be added to the interface:
static nsresult AddMatrixRows(nsIEditor * editor, nsIDOMNode *aMatrix, PRUint32 insertAt, PRUint32 howMany);
static nsresult AddMatrixColumns(nsIEditor * editor, nsIDOMNode *aMatrix, PRUint32 insertAt, PRUint32 howMany);
static nsresult GetFirstMatrixRow(nsIDOMNode* aMatrix, nsIDOMNode** aRowOut);
static nsresult GetNextMatrixRow(nsIDOMNode* aMatrix, nsIDOMNode* currRow, nsIDOMNode** nextRowOut);
static nsresult GetMatrixSize(nsIDOMNode *aMatrix, PRInt32* aRowCount, PRInt32* aColCount);
static nsresult FindMatrixCell(nsIDOMNode* aMatrix, nsIDOMNode *aCell, PRInt32* whichRow, PRInt32* whichCol);
static nsresult GetCellAt(nsIDOMNode* aMatrix, PRInt32 whichRow, PRInt32 whichCol, nsCOMPtr<nsIDOMNode> *pCell);


protected:

PRBool NodeSupportsMathChild(nsIDOMNode * node, PRBool displayed);

nsresult DetermineParentLeftRight(nsIDOMNode * node,
                                  PRUint32 & offset,
                                  PRUint32 & flags,
                                  nsCOMPtr<nsIDOMNode> & parent,
                                  nsCOMPtr<nsIDOMNode> & left,
                                  nsCOMPtr<nsIDOMNode> & right);

nsresult InsertMathmlElement(nsIEditor * editor,
                             nsISelection * selection, 
                             nsIDOMNode* node, 
                             PRUint32 offset,
                             PRUint32 flags,
                             const nsCOMPtr<nsIDOMElement> & mathmlElement);
                                  
PRUint32 GetMathMLNodeAndTypeFromNode(nsIDOMNode * rawNode, PRUint32 rawOffset, 
                                      nsCOMPtr<nsIDOMNode> & mathmlNode,
                                      PRUint32 & offset); 
                                      
PRUint32 GetMMLNodeTypeFromLocalName(nsIDOMNode * mathmlNode, 
                                     const nsAString & localName); 
PRUint32 GetMRowNodeType(nsIDOMNode * mathmlNode);

PRUint32 GetMoNodeType(nsIDOMNode * mathmlNode);

PRBool IsBigOperator(nsIDOMNode* mathmlNode, const nsAString & localName);
static PRBool GetBigOpNodes(nsIDOMNode* mathmlNode, const nsAString & localName,
                            nsCOMPtr<nsIDOMNode> & mo, nsCOMPtr<nsIDOMNode> & mstyle,
                            nsCOMPtr<nsIDOMNode> & script, PRUint32 & scriptType);

void SetMathmlNodeAndOffsetForMrowFence(const nsAString & localName,
                                        nsCOMPtr<nsIDOMNode> & mathmlNode,
                                        PRUint32 & offset);

PRBool   NodeInMath(nsIDOMNode* node);
nsresult EnsureMathWithSelectionCollapsed(nsIDOMNode * node);

static nsresult GetMatrixInfo(nsIDOMNode *aMatrix, PRInt32& nRow, PRInt32& nCol, 
                                          nsIDOMNode** pFindCell, nsTArray<msiMultiSpanCellInfo> *multiCellArray);

friend msiBigOpInfoImp::msiBigOpInfoImp(nsIDOMNode * mathmlNode, PRUint32 offset);


};

#endif // msiEditingManager_h__
