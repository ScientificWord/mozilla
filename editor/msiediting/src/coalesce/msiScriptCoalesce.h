// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiScriptCoalesce_h___
#define msiScriptCoalesce_h___

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiMCoalesceBase.h"

class nsIArray;
class nsITransaction;


class msiScriptCoalesce : public msiMCoalesceBase
{
public:
  msiScriptCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset, PRUint32 mathmlType);
  virtual ~msiScriptCoalesce();

  //msiIMathMLCoalesce
  NS_DECL_MSIIMATHMLCOALESCE
protected:  
  nsresult CoalesceLeft(nsIEditor * editor, nsIDOMNode * node, nsIArray** coalesced);
  nsresult CoalesceRight(nsIEditor * editor, nsIDOMNode * node, nsIArray** coalesced);
  nsresult CoalesceLeft(nsIEditor * editor, nsIDOMNode * node, nsITransaction** coalesced);
  nsresult CoalesceRight(nsIEditor * editor, nsIDOMNode * node, nsITransaction** coalesced);
  void DetermineScriptShiftAttributes(nsIDOMNode * node, nsAString & subShift, 
                                      nsAString & supShift);
  
private:
  PRUint32 m_maxOffset;
  
};

#endif // msiScriptCoalesce_h___
