// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "msiBigOpCoalesce.h"
#include "msiUtils.h"
#include "msiCoalesceUtils.h"
#include "msiRequiredArgument.h"
#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "nsIEditor.h"
#include "nsIMutableArray.h"s
#include "msiEditingAtoms.h"
#include "nsIDOMText.h"
#include "nsIDOMCharacterData.h"
#include "nsIMutableArray.h"
#include "nsComponentManagerUtils.h"

msiBigOpCoalesce::msiBigOpCoalesce(nsIDOMNode* mathmlNode, PRUint32 offset, nsCOMPtr<msiIBigOpInfo> & bigOpInfo)
: msiMCoalesceBase(mathmlNode, offset, MSI_BIGOPERATOR), m_bigOpInfo(bigOpInfo)
{
  NS_ASSERTION(m_bigOpInfo, "Yuck -- m_bigOpInfo is null");
  if (m_bigOpInfo)
  {
    nsCOMPtr<nsIDOMNode> bigOp;
    PRUint32 scriptType(MATHML_UNKNOWN);
    m_bigOpInfo->GetScriptType(&scriptType);
    m_bigOpInfo->GetMathmlNode(getter_AddRefs(bigOp));
    NS_ASSERTION(bigOp == m_mathmlNode, "BigOp Node should equal m_mathmlNode");
    if (scriptType == MATHML_UNKNOWN) // bigOp type == MO or MSTYLE
    {
      nsCOMPtr<nsIDOMNode> mstyle;
      m_bigOpInfo->GetMstyle(getter_AddRefs(mstyle));
      if (mstyle)
        m_maxOffset = 1;
      else // bigOp == MO
      {
        m_maxOffset = 1;
//        nsCOMPtr<nsIDOMNode> child;
//        bigOp->GetFirstChild(getter_AddRefs(child));
//        if (child)
//        {
//          nsCOMPtr<nsIDOMText> text(do_QueryInterface(child));
//          if (text)
//          {
//             nsCOMPtr<nsIDOMCharacterData> characterdata(do_QueryInterface(text));
//             if (characterdata)
//                characterdata->GetLength(&m_maxOffset);
//          }
//        }
      }
    }
    else if (scriptType == MATHML_MUNDEROVER || scriptType == MATHML_MSUBSUP)
      m_maxOffset = 3;
    else  //bigOp type == MSUB, MSUP, MUNDER or MOVER
      m_maxOffset = 2;
  }
}
  
msiBigOpCoalesce::~msiBigOpCoalesce()
{
}

NS_IMETHODIMP
msiBigOpCoalesce::Coalesce(nsIEditor * editor,
                           nsIDOMNode * node,
                           nsIArray ** coalesced)                
{
  nsresult res(NS_ERROR_FAILURE);
  *coalesced = nsnull;
  PRUint32 offset(0);
  PRUint32 mmltype(MATHML_UNKNOWN), scriptType(MATHML_UNKNOWN);
  nsCOMPtr<nsIDOMNode> currSup, currSub, inSup, inSub, newSup, newSub, base;
  nsCOMPtr<nsIDOMNode> bigOp;
  if (m_bigOpInfo)
  {
    m_bigOpInfo->GetOffset(&offset);
    m_bigOpInfo->GetScriptType(&scriptType);
    m_bigOpInfo->GetMathmlNode(getter_AddRefs(bigOp));
  }
  PRBool doCoalesce(offset == m_maxOffset && bigOp);
  if (node)
    res = msiUtils::GetMathmlNodeType(editor, node, mmltype);
  
  doCoalesce = doCoalesce && (mmltype == MATHML_MSUB || mmltype == MATHML_MSUP || mmltype == MATHML_MSUBSUP);
  if (node && editor && m_mathmlNode && doCoalesce)
  {
    if (scriptType ==MATHML_UNKNOWN)
      base = bigOp;
    else
    {
      msiUtils::GetChildNode(bigOp, 0, base);
      if (scriptType == MATHML_MSUB || scriptType == MATHML_MSUBSUP ||
          scriptType == MATHML_MUNDER || scriptType == MATHML_MUNDEROVER)
        res = msiUtils::GetChildNode(bigOp, 1, currSub);
      else if (scriptType == MATHML_MSUP || scriptType == MATHML_MOVER)
        res = msiUtils::GetChildNode(bigOp, 1, currSup);
      if (NS_SUCCEEDED(res) && scriptType == MATHML_MSUBSUP || scriptType == MATHML_MUNDEROVER)
        res = msiUtils::GetChildNode(m_mathmlNode, 2, currSup);
    }  
    if (NS_SUCCEEDED(res) && (mmltype == MATHML_MSUB || mmltype == MATHML_MSUBSUP))
      res = msiUtils::GetChildNode(node, 1, inSub);
    else if (mmltype == MATHML_MSUP)
      res = msiUtils::GetChildNode(node, 1, inSup);
    if (NS_SUCCEEDED(res) && mmltype == MATHML_MSUBSUP)
      res = msiUtils::GetChildNode(node, 2, inSup);
    
      
    if (NS_SUCCEEDED(res) && (currSup || inSup))
      res = msiRequiredArgument::MakeRequiredArgument(editor, currSup, inSup, newSup);
    if (NS_SUCCEEDED(res) && (currSub || inSub))
      res = msiRequiredArgument::MakeRequiredArgument(editor, currSub, inSub, newSub);
    if (NS_SUCCEEDED(res) && base && (newSub || newSup))
    {
      PRUint32 dummyflags(0);
      nsCOMPtr<nsIDOMElement> newElement;
      PRBool useSubSupLimits(PR_FALSE);
      if (m_bigOpInfo)
         m_bigOpInfo->GetUseSubSupLimits(&useSubSupLimits);
      if (useSubSupLimits)
      {
        nsAutoString subShift, supShift;
        DetermineScriptShiftAttributes(node, subShift, supShift);
        if (newSub && newSup)
          res = msiUtils::CreateMSubSup(editor, base, newSub, newSup, PR_FALSE, PR_FALSE, PR_FALSE,
                                        dummyflags, subShift, supShift, newElement); 
        else if (newSub)
          res = msiUtils::CreateMSubOrMSup(editor, PR_FALSE, base, newSub, PR_FALSE, PR_FALSE, 
                                         dummyflags, subShift, newElement);
        else //newSup
          res = msiUtils::CreateMSubOrMSup(editor, PR_TRUE, base, newSup, PR_FALSE, PR_FALSE, 
                                         dummyflags, supShift, newElement);
      }
      else
      {
        nsAutoString emptyString;
        if (newSub && newSup)
          res = msiUtils::CreateMunderover(editor, base, newSub, newSup, PR_FALSE, PR_FALSE, PR_FALSE, PR_TRUE,
                                           dummyflags, emptyString, 
                                           emptyString, newElement);
        else if (newSub)
          res = msiUtils::CreateMunderOrMover(editor, PR_FALSE, base, newSub, PR_FALSE, PR_FALSE, PR_TRUE, 
                                              dummyflags, emptyString, newElement);
        else //newSup
          res = msiUtils::CreateMunderOrMover(editor, PR_TRUE, base, newSup, PR_FALSE, PR_FALSE, PR_TRUE,
                                              dummyflags, emptyString, newElement);
      }
      if (NS_SUCCEEDED(res) && newElement)
      {
        nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
        if (NS_SUCCEEDED(res) && mutableArray)
        {
          res = mutableArray->AppendElement(newElement, PR_FALSE);
          if (NS_SUCCEEDED(res))
          {
            *coalesced = mutableArray;
            NS_ADDREF(*coalesced);
          }
        }
      }  
    }
  }
  return res;
}

NS_IMETHODIMP
msiBigOpCoalesce::CoalesceTxn(nsIEditor * editor,
                              nsIDOMNode * node,
                              nsITransaction ** txn)                
{
  *txn = nsnull;
  return NS_OK;
}


NS_IMETHODIMP
msiBigOpCoalesce::PrepareForCoalesce(nsIEditor * editor,
                                      PRUint32    pfcFlags,
                                      nsIArray ** beforeOffset,                
                                      nsIArray ** afterOffset)                
{
  *beforeOffset = nsnull;
  *afterOffset = nsnull;
  nsresult res(NS_ERROR_FAILURE);
  PRUint32 offset(0);
  nsCOMPtr<nsIDOMNode> bigOp;
  if (m_bigOpInfo)
  {
    m_bigOpInfo->GetMathmlNode(getter_AddRefs(bigOp));
    m_bigOpInfo->GetOffset(&offset);
    if (bigOp)
    {
      nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
      if (NS_SUCCEEDED(res) && m_mathmlNode)
      {
        res = mutableArray->AppendElement(bigOp, PR_FALSE);
        if (NS_SUCCEEDED(res))
        {
          if (offset == 0)
          {
            *afterOffset = mutableArray;
            NS_ADDREF(*afterOffset);
          }
          else  
          {
            *beforeOffset = mutableArray;
            NS_ADDREF(*beforeOffset);
          }
        }
      }    
    }    
  }
  return res;
}


void msiBigOpCoalesce::DetermineScriptShiftAttributes(nsIDOMNode * node, nsAString & subShift, nsAString & supShift)
{
  PRUint32 scriptType(MATHML_UNKNOWN);
  nsCOMPtr<nsIDOMNode> script;
  if (m_bigOpInfo)
  {  
    m_bigOpInfo->GetScriptType(&scriptType);
    if (scriptType == MATHML_MSUB || scriptType == MATHML_MSUP || scriptType == MATHML_MSUBSUP)
      m_bigOpInfo->GetScript(getter_AddRefs(script));
  }
  nsCOMPtr<nsIDOMElement> currElement;
  if (script)
    currElement = do_QueryInterface(m_mathmlNode);
  nsCOMPtr<nsIDOMElement> newElement(do_QueryInterface(node));
  subShift.Truncate(0);
  supShift.Truncate(0);
  nsAutoString subscriptshift, superscriptshift;
  msiEditingAtoms::subscriptshift->ToString(subscriptshift);
  msiEditingAtoms::superscriptshift->ToString(superscriptshift);
  if (currElement)
  {
    currElement->GetAttribute(subscriptshift, subShift);
    currElement->GetAttribute(superscriptshift, supShift);
  }
  if (newElement)
  {
    if (subShift.IsEmpty())
      newElement->GetAttribute(subscriptshift, subShift);
    if (supShift.IsEmpty())
      newElement->GetAttribute(superscriptshift, supShift);
  }
  return;
}  
