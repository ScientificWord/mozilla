// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiEditingManager_h__
#define msiEditingManager_h__

#include "nsCOMPtr.h"
#include "msiIEditingManager.h"
#include "msiBigOpInfoImp.h"

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

friend msiBigOpInfoImp::msiBigOpInfoImp(nsIDOMNode * mathmlNode, PRUint32 offset);


};

#endif // msiEditingManager_h__
