// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiMleaf_h___
#define msiMleaf_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMInsertionBase.h"

class msiMleaf : public msiMInsertionBase
{
public:
  msiMleaf(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  
  //override msiIMathMLInsertion
  NS_DECL_MSIIMATHMLINSERTION

protected: 
// Setup the pass off of an insertion to the parent node. The mathmlEditing parameter
// will contain the msiIMathMLInsertion interface for the parent. 
virtual nsresult SetupPassToParent(nsIEditor * editor,
                                  nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                  PRUint32 & flags);

// Setup the pass off of an insertion to the parent node. The mathmlEditing parameter
// will contain the msiIMathMLInsertion interface for the parent.
// In this case, the math leaf will be split into two leaves left and right base on 
// m_offset. 
  virtual nsresult SetupPassToParent(nsIEditor * editor,
                                     nsCOMPtr<msiIMathMLInsertion> & mathmlEditing,
                                     nsCOMPtr<nsIDOMNode> & left,
                                     nsCOMPtr<nsIDOMNode> & right,
                                     PRUint32 & flags);
// Split m_mathmlNodeinto two nodes at m_offset
  nsresult Split(nsCOMPtr<nsIDOMNode> & left,
                 nsCOMPtr<nsIDOMNode> & right);
                                   
protected:
  PRUint32 m_length;                                   
};

#endif // msiMleaf_h___
