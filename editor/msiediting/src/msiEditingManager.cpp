// Copyright (c) 2005, MacKichan Software, Inc.  All rights reserved.

#include "nsString.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMAttr.h"
#include "nsIDOMNode.h"
#include "nsIDOMText.h"
#include "nsIDOMCharacterData.h"
#include "nsIArray.h"
#include "nsArrayUtils.h"
#include "nsCOMArray.h"
#include "nsIContent.h"
#include "nsISimpleEnumerator.h"
#include "nsIHTMLEditor.h"
#include "nsIDOMRange.h"
#include "nsEditor.h"

#include "msiEditingManager.h"
#include "msiIMathMLEditor.h"
#include "msiInputbox.h"
#include "msiMaction.h"
#include "msiMaligngroup.h"
#include "msiMalignmark.h"
#include "msiMath.h"
#include "msiMenclose.h"
#include "msiMerror.h"
#include "msiMfenced.h"
#include "msiMfrac.h"
#include "msiMglyph.h"
#include "msiMi.h"
#include "msiMlabeledtr.h"
#include "msiMmultiscripts.h"
#include "msiMn.h"
#include "msiMo.h"
#include "msiMover.h"
#include "msiMpadded.h"
#include "msiMphantom.h"
#include "msiMroot.h"
#include "msiMrow.h"
#include "msiMrowFence.h"
#include "msiMrowBoundFence.h"
#include "msiMs.h"
#include "msiMspace.h"
#include "msiMsqrt.h"
#include "msiMstyle.h"
#include "msiMsub.h"
#include "msiMsubsup.h"
#include "msiMsup.h"
#include "msiMtable.h"
#include "msiMtd.h"
#include "msiMtext.h"
#include "msiMtr.h"
#include "msiMunder.h"
#include "msiMunderover.h"
#include "msiWhitespace.h"
#include "msiBigOperator.h"

#include "msiMCaretBase.h"
#include "msiInputboxCaret.h"
#include "msiMactionCaret.h"
#include "msiMaligngroupCaret.h"
#include "msiMalignmarkCaret.h"
#include "msiMathCaret.h"
#include "msiMencloseCaret.h"
#include "msiMerrorCaret.h"
#include "msiMfencedCaret.h"
#include "msiMfracCaret.h"
#include "msiMglyphCaret.h"
#include "msiMiCaret.h"
#include "msiMlabeledtrCaret.h"
#include "msiMmultiscriptsCaret.h"
#include "msiMnCaret.h"
#include "msiMoCaret.h"
#include "msiMoverCaret.h"
#include "msiMpaddedCaret.h"
#include "msiMphantomCaret.h"
#include "msiMrootCaret.h"
#include "msiMrowCaret.h"
#include "msiMrowFenceCaret.h"
#include "msiMrowBoundFenceCaret.h"
#include "msiMsCaret.h"
#include "msiMspaceCaret.h"
#include "msiMsqrtCaret.h"
#include "msiMstyleCaret.h"
#include "msiMsubCaret.h"
#include "msiMsubsupCaret.h"
#include "msiMsupCaret.h"
#include "msiMtableCaret.h"
#include "msiMtdCaret.h"
#include "msiMtextCaret.h"
#include "msiMtrCaret.h"
#include "msiMunderCaret.h"
#include "msiMunderoverCaret.h"
#include "msiWhitespaceCaret.h"
#include "msiBigOpCaret.h"
#include "msiEditingAtoms.h"

#include  "msiMCoalesceBase.h"
#include  "msiMnCoalesce.h"
#include  "msiMrowCoalesce.h"
#include  "msiMsubCoalesce.h"
#include  "msiMsupCoalesce.h"
#include  "msiMsubsupCoalesce.h"
#include  "msiInputboxCoalesce.h"
#include  "msiBigOpCoalesce.h"
#include  "msiMleafCoalesce.h"

#include "msiUtils.h"
#include "msiNameSpaceUtils.h"
#include "msiIBigOpInfo.h"
#include "jcsDumpNode.h"

NS_IMPL_ISUPPORTS1(msiEditingManager, msiIEditingManager)

msiEditingManager::msiEditingManager()
{
  msiNameSpaceUtils::Initialize();
  msiEditingAtoms::AddRefAtoms();
  msiUtils::Initalize();
}

msiEditingManager::~msiEditingManager()
{
   msiNameSpaceUtils::Shutdown();
}

nsresult MoveRangeTo(nsIEditor* editor, nsIDOMRange * range, nsIDOMNode *node, PRUint32 offset)
{
  nsCOMPtr<nsIArray> arrayOfNodes;
  nsCOMPtr<nsIDOMNode> currentNode; 
  PRUint16 nodeType;
  PRUint32 length;
  nsCOMPtr<nsIHTMLEditor> htmlEditor(do_QueryInterface(editor));
  htmlEditor->NodesInRange(range, getter_AddRefs(arrayOfNodes));
  arrayOfNodes->GetLength(&length);

  for (PRInt32 i = (length-1); i>=0; i--)
  {
    currentNode = do_QueryElementAt(arrayOfNodes, i);
    // put inNode in new parent, outNode
    currentNode->GetNodeType(&nodeType);

    //  jcs -- what is this supposed to do? 
    //  if (nodeType==3) // Means a #text ?
    //    currentNode->GetParentNode(getter_AddRefs(currentNode));

    editor->DeleteNode(currentNode);
    editor->InsertNode(currentNode, node, offset);//past the mo
  }
  return NS_OK;
}

NS_IMETHODIMP
msiEditingManager::GetMathMLEditingBC(nsIDOMNode* rawNode, 
                                      PRUint32    rawOffset,
                                      PRBool clean,
                                      msiIMathMLEditingBC ** editingBC)
                                             
{
  nsresult res(NS_ERROR_FAILURE);
  *editingBC = nsnull;
  if (rawNode)
  {
    res = NS_OK;
    nsCOMPtr<nsIDOMNode> mathmlNode;
    PRUint32 offset(rawOffset);
    PRUint32 mathmlNodeType = GetMathMLNodeAndTypeFromNode(rawNode, rawOffset, mathmlNode, offset);
    if (mathmlNodeType != msiIMathMLEditingBC::MATHML_UNKNOWN && mathmlNode)
    {
      *editingBC = new msiMEditingBase(mathmlNode, offset, clean, mathmlNodeType);
      if (*editingBC == nsnull)
        res = NS_ERROR_OUT_OF_MEMORY;
      else
        NS_ADDREF(*editingBC);
    }
  }
  return res;
}

NS_IMETHODIMP
msiEditingManager::GetMathMLInsertionInterface(nsIDOMNode* rawNode, 
                                              PRUint32    rawOffset,
                                              msiIMathMLInsertion ** mathml)
{
  nsresult res(NS_ERROR_FAILURE);
  *mathml = nsnull;
  if (rawNode)
  {
    res = NS_OK;
    nsCOMPtr<nsIDOMNode> mathmlNode;
    PRUint32 offset(rawOffset);
    PRUint32 mathmlNodeType = GetMathMLNodeAndTypeFromNode(rawNode, rawOffset, mathmlNode, offset);
    if (mathmlNodeType != msiIMathMLEditingBC::MATHML_UNKNOWN && mathmlNode)
    {
      switch (mathmlNodeType)
      {
        case msiIMathMLEditingBC::MATHML_MATH:
         *mathml = new msiMath(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MI:
         *mathml = new msiMi(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MN:
         *mathml = new msiMn(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MO:
         *mathml = new msiMo(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MTEXT:
         *mathml = new msiMtext(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSPACE:
         *mathml = new msiMspace(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MS:
         *mathml = new msiMs(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MGLYPH:
         *mathml = new msiMglyph(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROW:
         *mathml = new msiMrow(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROWFENCE:
         *mathml = new msiMrowFence(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROWBOUNDFENCE:
         *mathml = new msiMrowBoundFence(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MFRAC:
         *mathml = new msiMfrac(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSQRT:
         *mathml = new msiMsqrt(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROOT:
         *mathml = new msiMroot(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSTYLE:
         *mathml = new msiMstyle(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MERROR:
         *mathml = new msiMerror(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MPADDED:
         *mathml = new msiMpadded(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MPHANTOM:
         *mathml = new msiMphantom(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MFENCED:
         *mathml = new msiMfenced(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MENCLOSE:
         *mathml = new msiMenclose(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSUB:
         *mathml = new msiMsub(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSUP:
         *mathml = new msiMsup(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSUBSUP:
         *mathml = new msiMsubsup(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MUNDER:
         *mathml = new msiMunder(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MOVER:
         *mathml = new msiMover(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MUNDEROVER:
         *mathml = new msiMunderover(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MMULTISCRIPTS:
         *mathml = new msiMmultiscripts(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MTABLE:
         *mathml = new msiMtable(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MLABELEDTR:
         *mathml = new msiMlabeledtr(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MTR:
         *mathml = new msiMtr(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MTD:
         *mathml = new msiMtd(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MALIGNGROUP:
         *mathml = new msiMaligngroup(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MALIGNMARK:
         *mathml = new msiMalignmark(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MACTION:
         *mathml = new msiMaction(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MSI_INPUTBOX:
          *mathml = new msiInputbox(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MSI_BIGOPERATOR:
        {
          nsCOMPtr<msiIBigOpInfo> bigOpInfo;
          MSI_NewBigOpInfoImp(mathmlNode, offset, getter_AddRefs(bigOpInfo));
          nsCOMPtr<nsIDOMNode> mmlNode;
          PRUint32 bigOpOffset(msiIMathMLEditingBC::INVALID);
          if (bigOpInfo)
          {
            bigOpInfo->GetMathmlNode(getter_AddRefs(mmlNode)); 
            bigOpInfo->GetOffset(&bigOpOffset); 
          }
          else
          {
            mmlNode = mathmlNode;
            bigOpOffset = offset;
          }
          *mathml = new msiBigOperator(mmlNode, bigOpOffset, bigOpInfo);
        }
        break;
        case msiIMathMLEditingBC::MSI_WHITESPACE:
          *mathml = new msiWhitespace(mathmlNode, offset);
        break;
        
        default:
          res = NS_ERROR_NO_INTERFACE;
        break;
      }  
      if (res == NS_OK && *mathml == nsnull)
        res = NS_ERROR_OUT_OF_MEMORY;
      if (NS_SUCCEEDED(res) && *mathml)
        NS_ADDREF(*mathml);
    }
  }   
  return res;
}


NS_IMETHODIMP
msiEditingManager::SupportsMathMLInsertionInterface(nsIDOMNode* node, PRBool *supportsMathml)
{
  nsresult res(NS_ERROR_FAILURE);
  *supportsMathml = PR_FALSE;
  if (node)
  {
    res = NS_OK;
    nsCOMPtr<nsIDOMNode> dontcare;
    PRUint32 dontcare1(0);
    PRUint32 mathmlNodeType = GetMathMLNodeAndTypeFromNode(node, dontcare1, dontcare, dontcare1);
    if (mathmlNodeType != msiIMathMLEditingBC::MATHML_UNKNOWN)
      *supportsMathml = PR_TRUE;
  }   
  return res;
}

NS_IMETHODIMP
msiEditingManager::GetMathMLCaretInterface(nsIEditor* editor,
                                           nsIDOMNode* rawNode, 
                                           PRUint32    rawOffset,
                                           msiIMathMLCaret ** mathml)
{
  nsresult res(NS_ERROR_FAILURE);
  *mathml = nsnull;
  if (rawNode && editor)
  {
    res = NS_OK;
    nsCOMPtr<nsIDOMNode> mathmlNode;
    PRUint32 offset(rawOffset);
    PRUint32 mathmlNodeType = GetMathMLNodeAndTypeFromNode(rawNode, rawOffset, mathmlNode, offset);
    if (mathmlNodeType != msiIMathMLEditingBC::MATHML_UNKNOWN && mathmlNode)
    {
      switch (mathmlNodeType)
      {
        case msiIMathMLEditingBC::MATHML_MATH:
         *mathml = new msiMathCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MI:
         *mathml = new msiMiCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MN:
         *mathml = new msiMnCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MO:
         *mathml = new msiMoCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MTEXT:
         *mathml = new msiMtextCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSPACE:
         *mathml = new msiMspaceCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MS:
         *mathml = new msiMsCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MGLYPH:
         *mathml = new msiMglyphCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROW:
         *mathml = new msiMrowCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROWFENCE:
         *mathml = new msiMrowFenceCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROWBOUNDFENCE:
         *mathml = new msiMrowBoundFenceCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MFRAC:
         *mathml = new msiMfracCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSQRT:
         *mathml = new msiMsqrtCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROOT:
         *mathml = new msiMrootCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSTYLE:
         *mathml = new msiMstyleCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MERROR:
         *mathml = new msiMerrorCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MPADDED:
         *mathml = new msiMpaddedCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MPHANTOM:
         *mathml = new msiMphantomCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MFENCED:
         *mathml = new msiMfencedCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MENCLOSE:
         *mathml = new msiMencloseCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSUB:
         *mathml = new msiMsubCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSUP:
         *mathml = new msiMsupCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSUBSUP:
         *mathml = new msiMsubsupCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MUNDER:
         *mathml = new msiMunderCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MOVER:
         *mathml = new msiMoverCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MUNDEROVER:
         *mathml = new msiMunderoverCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MMULTISCRIPTS:
         *mathml = new msiMmultiscriptsCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MTABLE:
         *mathml = new msiMtableCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MLABELEDTR:
         *mathml = new msiMlabeledtrCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MTR:
         *mathml = new msiMtrCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MTD:
         *mathml = new msiMtdCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MALIGNGROUP:
         *mathml = new msiMaligngroupCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MALIGNMARK:
         *mathml = new msiMalignmarkCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MACTION:
         *mathml = new msiMactionCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MSI_INPUTBOX:
          *mathml = new msiInputboxCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MSI_WHITESPACE:
          *mathml = new msiWhitespaceCaret(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MSI_BIGOPERATOR:
        {
          nsCOMPtr<msiIBigOpInfo> bigOpInfo;
          MSI_NewBigOpInfoImp(mathmlNode, offset, getter_AddRefs(bigOpInfo));
          nsCOMPtr<nsIDOMNode> mmlNode;
          PRUint32 bigOpOffset(msiIMathMLEditingBC::INVALID);
          if (bigOpInfo)
          {
            bigOpInfo->GetMathmlNode(getter_AddRefs(mmlNode)); 
            bigOpInfo->GetOffset(&bigOpOffset); 
          }
          else
          {
            mmlNode = mathmlNode;
            bigOpOffset = offset;
          }
          *mathml = new msiBigOperatorCaret(mmlNode, bigOpOffset, bigOpInfo);
        }
        break;
        default:
          res = NS_ERROR_NO_INTERFACE;
        break;
      }  
      if (res == NS_OK && *mathml == nsnull)
        res = NS_ERROR_OUT_OF_MEMORY;
      if (NS_SUCCEEDED(res) && *mathml)
        NS_ADDREF(*mathml);
    }
  }   
  return res;
}

NS_IMETHODIMP
msiEditingManager::SupportsMathMLCaretInterface(nsIDOMNode* node, PRBool *supportsMathml)
{
  nsresult res(NS_ERROR_FAILURE);
  *supportsMathml = PR_FALSE;
  if (node)
  {
    res = NS_OK;
    nsCOMPtr<nsIDOMNode> dontcare;
    PRUint32 dummyOffset(0);
    PRUint32 mathmlNodeType = GetMathMLNodeAndTypeFromNode(node, dummyOffset, dontcare, dummyOffset);
    if (mathmlNodeType != msiIMathMLEditingBC::MATHML_UNKNOWN)
      *supportsMathml = PR_TRUE;
  }   
  return res;
}
//////////////////////

NS_IMETHODIMP
msiEditingManager::GetMathMLCoalesceInterface(nsIDOMNode* rawNode, 
                                              PRUint32    rawOffset,
                                              msiIMathMLCoalesce ** mathml)
{
  nsresult res(NS_ERROR_FAILURE);
  *mathml = nsnull;
  if (rawNode)
  {
    res = NS_OK;
    nsCOMPtr<nsIDOMNode> mathmlNode;
    PRUint32 offset(rawOffset);
    PRUint32 mathmlNodeType = GetMathMLNodeAndTypeFromNode(rawNode, rawOffset, mathmlNode, offset);
    if (mathmlNodeType != msiIMathMLEditingBC::MATHML_UNKNOWN && mathmlNode)
    {
      switch (mathmlNodeType)
      {
        case msiIMathMLEditingBC::MATHML_MN:
          *mathml = new msiMnCoalesce(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MROW:
          *mathml = new msiMrowCoalesce(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MI:
        case msiIMathMLEditingBC::MATHML_MO:
          *mathml = new msiMleafCoalesce(mathmlNode, offset, mathmlNodeType);
        break;
        case msiIMathMLEditingBC::MATHML_MATH:
        case msiIMathMLEditingBC::MATHML_MTEXT:
        case msiIMathMLEditingBC::MATHML_MSPACE:
        case msiIMathMLEditingBC::MATHML_MS:
        case msiIMathMLEditingBC::MATHML_MGLYPH:
        case msiIMathMLEditingBC::MATHML_MROWFENCE:
        case msiIMathMLEditingBC::MATHML_MROWBOUNDFENCE:
        case msiIMathMLEditingBC::MATHML_MFRAC:
        case msiIMathMLEditingBC::MATHML_MSQRT:
        case msiIMathMLEditingBC::MATHML_MROOT:
        case msiIMathMLEditingBC::MATHML_MSTYLE:
        case msiIMathMLEditingBC::MATHML_MERROR:
        case msiIMathMLEditingBC::MATHML_MPADDED:
        case msiIMathMLEditingBC::MATHML_MPHANTOM:
        case msiIMathMLEditingBC::MATHML_MFENCED:
        case msiIMathMLEditingBC::MATHML_MENCLOSE:
          *mathml = new msiMCoalesceBase(mathmlNode, offset, mathmlNodeType);
        break;
        case msiIMathMLEditingBC::MATHML_MSUB:
          *mathml = new msiMsubCoalesce(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MSUP:
          *mathml = new msiMsupCoalesce(mathmlNode, offset);
        break;
        
        case msiIMathMLEditingBC::MATHML_MSUBSUP:
          *mathml = new msiMsubsupCoalesce(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MATHML_MUNDER:
        case msiIMathMLEditingBC::MATHML_MOVER:
        case msiIMathMLEditingBC::MATHML_MUNDEROVER:
        case msiIMathMLEditingBC::MATHML_MMULTISCRIPTS:
        case msiIMathMLEditingBC::MATHML_MTABLE:
        case msiIMathMLEditingBC::MATHML_MLABELEDTR:
        case msiIMathMLEditingBC::MATHML_MTR:
        case msiIMathMLEditingBC::MATHML_MTD:
        case msiIMathMLEditingBC::MATHML_MALIGNGROUP:
        case msiIMathMLEditingBC::MATHML_MALIGNMARK:
        case msiIMathMLEditingBC::MATHML_MACTION:
          *mathml = new msiMCoalesceBase(mathmlNode, offset, mathmlNodeType);
        break;
        case msiIMathMLEditingBC::MSI_INPUTBOX:
          *mathml = new msiInputboxCoalesce(mathmlNode, offset);
        break;
        case msiIMathMLEditingBC::MSI_BIGOPERATOR:
        {
          nsCOMPtr<msiIBigOpInfo> bigOpInfo;
          MSI_NewBigOpInfoImp(mathmlNode, offset, getter_AddRefs(bigOpInfo));
          nsCOMPtr<nsIDOMNode> mmlNode;
          PRUint32 bigOpOffset(msiIMathMLEditingBC::INVALID);
          if (bigOpInfo)
          {
            bigOpInfo->GetMathmlNode(getter_AddRefs(mmlNode)); 
            bigOpInfo->GetOffset(&bigOpOffset); 
          }
          else
          {
            mmlNode = mathmlNode;
            bigOpOffset = offset;
          }
          *mathml = new msiBigOpCoalesce(mmlNode, bigOpOffset, bigOpInfo);
        }
        break;
        case msiIMathMLEditingBC::MSI_WHITESPACE:
          *mathml = new msiMCoalesceBase(mathmlNode, offset, mathmlNodeType);
        break;
        default:
          res = NS_ERROR_NO_INTERFACE;
        break;
      }  
      if (res == NS_OK && *mathml == nsnull)
        res = NS_ERROR_OUT_OF_MEMORY;
      if (NS_SUCCEEDED(res) && *mathml)
        NS_ADDREF(*mathml);
    }
  }   
  return res;
}

//ljh TODO -- The inserting of Math into document will need more work
// 1. When is it valid to insert math
// 2. If inserting into a bold span, should the math inherit the bold, via mstyle.
NS_IMETHODIMP
msiEditingManager::InsertMath(nsIEditor * editor,
                              nsISelection * selection, 
                              nsIDOMNode* node, 
                              PRUint32 offset,
                              PRUint32 flags,
                              PRBool   isDisplay)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMNode> mathParent, parent, left, right;
  nsCOMPtr<nsIHTMLEditor> htmlEditor;
  htmlEditor = do_QueryInterface(editor);
  if (!htmlEditor) return NS_ERROR_FAILURE;
  if (node)
    msiUtils::GetMathParent(node, mathParent);
  if (mathParent)
  {
    PRUint16 nodeType;
    nsCOMPtr<nsIDOMNode> theNode;
    node->GetNodeType(&nodeType);
    if (nodeType == nsIDOMNode::TEXT_NODE)
      node->GetParentNode(getter_AddRefs(theNode));
    else
      theNode = node;  
    nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
    GetMathMLInsertionInterface(theNode, offset, getter_AddRefs(mathmlEditing));
    if (mathmlEditing && editor)
    {
      PRBool isFeasible(PR_FALSE);
      PRUint32 inquiryID = isDisplay ? msiIMathMLInsertion::INSERT_DISPLAY : 
                                         msiIMathMLInsertion::INSERT_INLINE_MATH;
      mathmlEditing->Inquiry(editor, inquiryID, &isFeasible);
      if (isFeasible)
        res = mathmlEditing->InsertMath(editor, selection, isDisplay, nsnull, nsnull, flags);
      else
      {
        res = NS_OK;
        //TODO -- message about can't insert object
      }  
    }  
  }
  else // not in math
  {
    nsString strmsidisplay = NS_LITERAL_STRING("msidisplay");
    res = DetermineParentLeftRight(node, offset, flags, parent, left, right);
    if (NS_SUCCEEDED(res) && parent)
    {
      nsCOMPtr<nsIDOMElement> mathElement;
      res = msiUtils::CreateMathElement(editor, isDisplay, PR_TRUE, flags, mathElement);
      nsCOMPtr<nsIDOMNode> mathNode(do_QueryInterface(mathElement));
      if (isDisplay) // make sure we don't insert an msidisplay into an msidisplay
      {
        nsCOMPtr<nsIDOMNode> tempparent = node;
        nsString localName;  
        res = tempparent->GetLocalName(localName);
        if (!localName.EqualsLiteral("body") && !localName.Equals(strmsidisplay))
        {
          res = tempparent->GetParentNode(getter_AddRefs(tempparent));
          res = tempparent->GetLocalName(localName);
          if (!localName.Equals(strmsidisplay))
          {
            res = tempparent->GetParentNode(getter_AddRefs(tempparent));
            res = tempparent->GetLocalName(localName);
          }
        }
        if (!localName.EqualsLiteral("body") && localName.Equals(strmsidisplay)) // parent or grandparent is "msidisplay", change node and offset
        {
          left = nsnull;
          right = nsnull;
          msiUtils::GetIndexOfChildInParent(tempparent, offset);
          offset++;  // move the insertion point just past the msidisplay
          res = tempparent->GetParentNode(getter_AddRefs(parent));
        }
      }
      if (NS_SUCCEEDED(res) && mathNode)
      {
        if (offset >= 0){
          if (left)
            res = editor->ReplaceNode(left, node, parent);
          if (isDisplay)
          {
            nsCOMPtr<nsIDOMElement> msidisplay;
            nsCOMPtr<nsIDOMNode> inserted;
            htmlEditor->CreateElementWithDefaults(strmsidisplay, getter_AddRefs(msidisplay));
            res = msidisplay->AppendChild(mathNode, getter_AddRefs(inserted)); // put node in msidisplay
            // and now put display in place of node
            mathNode = msidisplay;
          }
          res = htmlEditor->InsertNodeAtPoint(mathNode, (nsIDOMNode **)address_of(parent), (PRInt32*)&offset, true);
          if (right) {
            offset++;
            res = htmlEditor->InsertNodeAtPoint(right,(nsIDOMNode **)address_of(parent), (PRInt32*)&offset, true);
          }
          if (NS_SUCCEEDED(res))
            msiUtils::doSetCaretPosition(editor, selection, mathNode);
        }
      }
    }  
  }  
  return res;
}

NS_IMETHODIMP
msiEditingManager::InsertSymbol(nsIEditor * editor,
                                nsISelection * selection, 
                                nsIDOMNode* node, 
                                PRUint32 offset,
                                const nsAString & symbol)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertSymbol");
  nsCOMPtr<nsIHTMLEditor> htmleditor = do_QueryInterface(editor);
  if (editor && selection && node)
  {
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    nsAutoString newSymbol(symbol);

    if (symbol.EqualsLiteral("-")) {
      newSymbol.Assign(0x2212);
    }      
    res = msiUtils::CreateMathMLLeafElement(editor, newSymbol, 1, flags, mathmlElement);
    if (NS_SUCCEEDED(res) && mathmlElement)
    {
      nsAutoString localName;
      nsAutoString parenttaglist;
      nsAutoString tag;
      nsAutoString mathtag;
      PRUint32 len;
      
      nsAutoString strSep(NS_LITERAL_STRING(","));
      mathmlElement->GetLocalName(localName);
      if (localName.EqualsLiteral("mi"))
      {
        nsCOMPtr<msiITagListManager> TagListManager;
        htmleditor->GetTagListManager(getter_AddRefs(TagListManager));        
        // see if one of the math variants applies
        TagListManager->BuildParentTagList();
          // return a comma-separated list of all the tags containing the selection.
        TagListManager->GetParentTagList(strSep, PR_TRUE, PR_FALSE, parenttaglist);
        len = parenttaglist.Length();
        const PRUnichar* cur = parenttaglist.BeginReading();
        const PRUnichar* start = cur;
        const PRUnichar* wordend = cur;
        const PRUnichar* end = parenttaglist.EndReading();

        while (cur < end) {
          if ((wordend == end) || PRUnichar(',') == *wordend) {
            parenttaglist.Mid(tag, cur - start, wordend - cur);
            htmleditor->TranslatePropertyToMath( tag, mathtag );
            if (mathtag.Length()>0)
            {
              editor->SetAttribute(mathmlElement, NS_LITERAL_STRING("mathvariant"), mathtag);
              break;
            }
            if (wordend == end)
              break;
            cur = wordend + 1;
          }
          wordend++;
        }
      }        
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
    } 
  }
  return res;
}      

NS_IMETHODIMP
msiEditingManager::InsertFraction(nsIEditor * editor,
                                  nsISelection * selection, 
                                  nsIDOMNode* node, 
                                  PRUint32 offset,
                                  const nsAString & lineThickness,
                                  PRUint32 attrFlags)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMRange> range;
  selection->GetRangeAt(0, getter_AddRefs(range));
  nsCOMPtr<msiIMathMLEditor> mathmlEditor(do_QueryInterface(editor));
  nsCOMPtr<nsIDOMNode> mathnode;
  nsCOMPtr<nsIDOMNode> anchorNode;
  PRInt32 anchorOffset;
  res = mathmlEditor->RangeInMath(range, getter_AddRefs(mathnode));
  PRBool inMath = (nsnull != mathnode);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertFraction");
  if (editor && selection && node)
  {
    editor->BeginTransaction();
    PRBool bCollapsed(PR_FALSE);
    res = selection->GetIsCollapsed(&bCollapsed);
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateMfrac(editor, nsnull, nsnull, (bCollapsed ||!inMath), PR_TRUE, flags, lineThickness, attrFlags, mathmlElement);
    nsCOMPtr<nsIDOMNode> numerator;
    res = mathmlElement->GetFirstChild(getter_AddRefs(numerator));
    if (!bCollapsed && inMath)
    {
      res = selection->GetAnchorNode(getter_AddRefs(anchorNode));
      res = selection->GetAnchorOffset(&anchorOffset);
      MoveRangeTo(editor, range, numerator, 0);
    } 
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
//        editor->InsertNode(mathmlElement, node, offset);
    if (!bCollapsed && inMath)
    {
      selection->Collapse(anchorNode,anchorOffset+1);
    }
    else
    {
      //selection->Collapse(mathmlElement,1);
    }
    editor->EndTransaction();  
  }
  return res;
}      

NS_IMETHODIMP
msiEditingManager::InsertBinomial(nsIEditor * editor,
                                    nsISelection * selection,
                                    nsIDOMNode * node,
                                    PRUint32 offset,
                                    const nsAString & opening,
                                    const nsAString & closing,
                                    const nsAString & lineThickness,
                                    PRUint32 attrFlags)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMRange> range;
  selection->GetRangeAt(0, getter_AddRefs(range));
  nsCOMPtr<msiIMathMLEditor> mathmlEditor(do_QueryInterface(editor));
  nsCOMPtr<nsIDOMNode> mathnode;
  res = mathmlEditor->RangeInMath(range, getter_AddRefs(mathnode));
  PRBool inMath = (nsnull != mathnode);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertBinomial");
  if (editor && selection && node)
  {
    editor->BeginTransaction();
    PRBool bCollapsed(PR_FALSE);
    nsCOMPtr<nsIHTMLEditor> htmlEditor(do_QueryInterface(editor));
    res = selection->GetIsCollapsed(&bCollapsed);
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateBinomial(editor, nsnull, nsnull, (bCollapsed ||!inMath), PR_TRUE, flags, opening, closing, lineThickness, attrFlags, mathmlElement);
    nsCOMPtr<nsIDOMNode> top;
    res = mathmlElement->GetFirstChild(getter_AddRefs(top));
    if (!bCollapsed && inMath)
    {
      MoveRangeTo(editor, range, top, 0);
    } 
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
//        editor->InsertNode(mathmlElement, node, offset);
    //selection->Collapse(node,offset+1);
    editor->EndTransaction();  
  }
  return res;
}      

NS_IMETHODIMP
msiEditingManager::InsertSqRoot(nsIEditor * editor,
                                nsISelection * selection, 
                                nsIDOMNode* node, 
                                PRUint32 offset)
{
  nsresult res(NS_ERROR_FAILURE);
  //check that we are entirely in one math object
  nsCOMPtr<nsIDOMRange> range;
  selection->GetRangeAt(0, getter_AddRefs(range));
  nsCOMPtr<msiIMathMLEditor> mathmlEditor(do_QueryInterface(editor));
  nsCOMPtr<nsIDOMNode> mathnode;
  res = mathmlEditor->RangeInMath(range, getter_AddRefs(mathnode));
  PRBool inMath = (nsnull != mathnode);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertSqRoot");
  if (editor && selection && node)
  {
    editor->BeginTransaction();
    PRBool bCollapsed(PR_FALSE);
    res = selection->GetIsCollapsed(&bCollapsed);
    nsCOMPtr<nsIDOMElement> mathmlElement;
    nsCOMPtr<nsIDOMElement> radicand;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateMsqrt(editor, nsnull, bCollapsed || !inMath, PR_TRUE, flags, mathmlElement);
    if (!bCollapsed && inMath)
    {
      nsCOMPtr<nsIDOMNode> radNode;
      res = mathmlElement->GetFirstChild(getter_AddRefs(radNode));
      if (!radNode) radicand = mathmlElement;
      else radicand = do_QueryInterface(radNode);
      MoveRangeTo(editor, range, radicand, 0);
    } 
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
//        editor->InsertNode(mathmlElement, node, offset);
    //selection->Collapse(node,offset+1);
    editor->EndTransaction();
  }
  return res;
}   

NS_IMETHODIMP
msiEditingManager::InsertRoot(nsIEditor * editor,
                                nsISelection * selection, 
                                nsIDOMNode* node, 
                                PRUint32 offset)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertSqRoot");
  nsCOMPtr<nsIDOMRange> range;
  selection->GetRangeAt(0, getter_AddRefs(range));
  nsCOMPtr<msiIMathMLEditor> mathmlEditor(do_QueryInterface(editor));
  nsCOMPtr<nsIDOMNode> mathnode;
  res = mathmlEditor->RangeInMath(range, getter_AddRefs(mathnode));
  PRBool inMath = (nsnull != mathnode);
  if (editor && selection && node)
  {
    editor->BeginTransaction();
    PRBool bCollapsed(PR_FALSE);
    res = selection->GetIsCollapsed(&bCollapsed);
    nsCOMPtr<nsIDOMElement> mathmlElement;
    nsCOMPtr<nsIDOMElement> radicand;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateMroot(editor, nsnull, nsnull, bCollapsed || !inMath, PR_TRUE, flags, mathmlElement);
    if (!bCollapsed && inMath)
    {
      nsCOMPtr<nsIDOMNode> radNode;
      res = mathmlElement->GetFirstChild(getter_AddRefs(radNode));
      if (!radNode) radicand = mathmlElement;
      else radicand = do_QueryInterface(radNode);
      MoveRangeTo(editor, range, radicand, 0);
    } 
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
//        editor->InsertNode(mathmlElement, node, offset);
    //selection->Collapse(node,offset+1);
    editor->EndTransaction();
  }
  return res;
}   

NS_IMETHODIMP
msiEditingManager::InsertFence(nsIEditor* editor,
                               nsISelection * selection,
                               nsIDOMNode* node, 
                               PRUint32 offset,
                               const nsAString & open,
                               const nsAString & close)
{
  nsresult res(NS_ERROR_FAILURE);
  //check that we are entirely in one math object
  nsCOMPtr<nsIDOMRange> range;
  selection->GetRangeAt(0, getter_AddRefs(range));
  nsCOMPtr<msiIMathMLEditor> mathmlEditor(do_QueryInterface(editor));
  nsCOMPtr<nsIDOMNode> mathnode;
  res = mathmlEditor->RangeInMath(range, getter_AddRefs(mathnode));
  PRBool inMath = (nsnull != mathnode);
//  if (!inMath) return NS_OK;
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertFence");
  if (editor && selection && node)
  {
    editor->BeginTransaction();
    PRBool bCollapsed(PR_FALSE);
    nsCOMPtr<nsIHTMLEditor> htmlEditor(do_QueryInterface(editor));
    res = selection->GetIsCollapsed(&bCollapsed);
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    PRUint32 attrFlags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateMRowFence(editor, nsnull, bCollapsed, open, close, PR_TRUE, flags, attrFlags, mathmlElement);
    if (!bCollapsed)
    {
      MoveRangeTo(editor, range, mathmlElement, 1);
    } 
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
//        editor->InsertNode(mathmlElement, node, offset);
    //selection->Collapse(node,offset+1);
    editor->EndTransaction();  
  }
  return res;
}                

//TODO Are lim, limsup and such mathnames. How are scripts to be handled
NS_IMETHODIMP
msiEditingManager::InsertMathname(nsIEditor* editor,
                                  nsISelection * selection,
                                  nsIDOMNode* node, 
                                  PRUint32 offset,
                                  const nsAString & mathname)
{

  //printf("\njcs -- msiEditingManager::InsertMathname\n");
  //printf("\nnode\n");
  //DumpNode(node,0, true);
  //printf("\noffset = %d\n", offset);
  //printf("\nselection\n");
  //DumpSelection(selection);


  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertMathname");
  if (editor && selection && node)
  {
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateMathname(editor, mathname, flags, PR_FALSE, mathmlElement);
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
  }
  return res;
}                                                 

NS_IMETHODIMP
msiEditingManager::InsertMathunit(nsIEditor* editor,
                                  nsISelection * selection,
                                  nsIDOMNode* node, 
                                  PRUint32 offset,
                                  const nsAString & mathunit)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertMathname");
  if (editor && selection && node)
  {
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateMathname(editor, mathunit, flags, PR_TRUE, mathmlElement);
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
  }
  return res;
}                                                 

NS_IMETHODIMP
msiEditingManager::InsertEngineFunction(nsIEditor* editor,
                                        nsISelection * selection,
                                        nsIDOMNode* node, 
                                        PRUint32 offset,
                                        const nsAString & name)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertMathname");
  if (editor && selection && node)
  {
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateEngineFunction(editor, name, flags, mathmlElement);
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
  }
  return res;
}                                                 

NS_IMETHODIMP
msiEditingManager::InsertMatrix(nsIEditor * editor,
                                nsISelection * selection,
                                nsIDOMNode * node,
                                PRUint32 offset,
                                PRUint32 rows,
                                PRUint32 cols,
                                const nsAString & rowSignature,
                                const nsAString & delim)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertMatrix");
  if (editor && selection && node)
  {
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateMtable(editor, rows, cols, rowSignature, PR_TRUE, flags, mathmlElement, delim);
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
  }
  return res;
}   

NS_IMETHODIMP
msiEditingManager::InsertOperator(nsIEditor * editor,
                                  nsISelection * selection,
                                  nsIDOMNode * node,
                                  PRUint32 offset,
                                  const nsAString & symbol,
                                  PRUint32 attrFlags,
                                  const nsAString & leftspace,
                                  const nsAString & rightspace,
                                  const nsAString & minsize,
                                  const nsAString & maxsize)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertOperator");
  if (editor && selection && node)
  {
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    PRUint32 caretPos = symbol.Length();
    nsAutoString newSymbol(symbol);
    if (symbol.EqualsLiteral("-"))
    {
      newSymbol.Assign(0x2212);
    }
    res = msiUtils::  CreateMathOperator(editor, newSymbol, caretPos, flags, attrFlags, 
                                       leftspace, rightspace, minsize, maxsize, mathmlElement);
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
  }
  return res;
}

NS_IMETHODIMP
msiEditingManager::InsertScript(nsIEditor * editor,
                                nsISelection * selection,
                                nsIDOMNode * node,
                                PRUint32 offset,
                                PRBool isSup,
                                const nsAString & scriptShift)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertOperator");
  if (editor && selection && node)
  {
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    nsCOMPtr<nsIDOMNode> dummy;
    res = msiUtils::CreateMSubOrMSup(editor, isSup, dummy, dummy, PR_FALSE, PR_TRUE,
                                      flags, scriptShift, mathmlElement);
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
  }
  return res;
}

NS_IMETHODIMP
msiEditingManager::InsertDecoration(nsIEditor* editor,
                                    nsISelection * selection,
                                    nsIDOMNode* node, 
                                    PRUint32 offset,
                                    const nsAString & above,
                                    const nsAString & below,
                                    const nsAString & aroundNotation,
                                    const nsAString & aroundType)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertFence");
  nsCOMPtr<nsIDOMRange> range;
  selection->GetRangeAt(0, getter_AddRefs(range));
  nsCOMPtr<msiIMathMLEditor> mathmlEditor(do_QueryInterface(editor));
  nsCOMPtr<nsIDOMNode> mathnode;
  res = mathmlEditor->RangeInMath(range, getter_AddRefs(mathnode));
  PRBool inMath = (nsnull != mathnode);
  if (editor && selection && node)
  {
    editor->BeginTransaction();
    PRBool bCollapsed(PR_FALSE);
    res = selection->GetIsCollapsed(&bCollapsed);
    nsCOMPtr<nsIDOMElement> mathmlElement;
    PRUint32 flags(msiIMathMLInsertion::FLAGS_NONE);
    res = msiUtils::CreateDecoration(editor, nsnull, above, below, aroundNotation, aroundType, 
                                             bCollapsed || !inMath, PR_TRUE, flags, mathmlElement);
    if (!bCollapsed && inMath)
    {
      nsCOMPtr<nsIDOMNode> base;
      res = mathmlElement->GetFirstChild(getter_AddRefs(base));
      if ( (!above.IsEmpty() || !below.IsEmpty()) && (!aroundNotation.IsEmpty() || !aroundType.IsEmpty()) )
      {
        nsCOMPtr<nsIDOMNode> tempparent = base;
        res = tempparent->GetFirstChild(getter_AddRefs(base));
      }
      MoveRangeTo(editor, range, base, 0);
    } 
    if (NS_SUCCEEDED(res) && mathmlElement)
      res = InsertMathmlElement(editor, selection, node, offset, flags, mathmlElement);
        //editor->InsertNode(mathmlElement, node, offset);
    //selection->Collapse(node,offset+1);
    editor->EndTransaction();
  }
  return res;
}                

nsresult msiEditingManager::DetermineParentLeftRight(nsIDOMNode * node,
                                                     PRUint32 & offset,
                                                     PRUint32 & flags,
                                                     nsCOMPtr<nsIDOMNode> & parent,
                                                     nsCOMPtr<nsIDOMNode> & left,
                                                     nsCOMPtr<nsIDOMNode> & right)
{ 
  nsresult res(NS_ERROR_FAILURE);
  if (node)
  {                                                   
    PRUint16 nodeType;
    node->GetNodeType(&nodeType);
    if (nodeType == nsIDOMNode::TEXT_NODE)
    {
      node->GetParentNode(getter_AddRefs(parent));
      if (parent && NodeSupportsMathChild(parent, PR_FALSE))
      {
        res = NS_OK;
        PRUint32 index(0);
        PRUint32 textlen(0);
        msiUtils::GetIndexOfChildInParent(node, index);
        nsCOMPtr<nsIContent> text(do_QueryInterface(node));
        if (text)
          textlen = text->TextLength();
        if (0 < offset && offset < textlen)
        { 
          msiUtils::CloneNode(node, right);
          msiUtils::CloneNode(node, left);
          nsCOMPtr<nsIDOMCharacterData> lfSideCharData(do_QueryInterface(left));
          nsCOMPtr<nsIDOMCharacterData> rtSideCharData(do_QueryInterface(right));
          if (lfSideCharData && rtSideCharData)
          {
            lfSideCharData->DeleteData(offset, textlen-offset);
            rtSideCharData->DeleteData(0, offset);
          }  
        } 
        if (0 < offset)
          offset = index + 1;
        else (offset = index); 
      }
      else
        parent = nsnull;  
    }
    else if (NodeSupportsMathChild(node, PR_FALSE))
    {
      parent = node;
      left = nsnull;
      right = nsnull;
      res = NS_OK;
    }
  }
  return res;  
}

PRBool msiEditingManager::NodeSupportsMathChild(nsIDOMNode * node, PRBool displayed)
{
  //ljh TODO -- Where can math be inserted  -- Is this supposed to be determined via DTD???
  PRBool rv(PR_FALSE);
  if (node)
  {
    PRUint16 nodeType;
    node->GetNodeType(&nodeType);
    if (nodeType == nsIDOMNode::ELEMENT_NODE)
     rv = PR_TRUE;
  }
  return rv;
}

PRUint32 
msiEditingManager::GetMathMLNodeAndTypeFromNode(nsIDOMNode * rawNode, PRUint32 rawOffset,
                                                nsCOMPtr<nsIDOMNode> & mathmlNode,
                                                PRUint32 & offset)
{
  offset = rawOffset;
  PRUint32 rv(msiIMathMLEditingBC::MATHML_UNKNOWN);
  if (rawNode)
  { 
    nsAutoString localName, nsURI;
    PRUint32 length(0);
    nsCOMPtr<nsIDOMText> text(do_QueryInterface(rawNode));
    nsCOMPtr<nsIDOMNode> parent;
    if (text)
    {
      text->GetParentNode(getter_AddRefs(parent));
      NS_ASSERTION(parent, "Yuck -- Get Parent node failed");
      if (parent)
        parent->GetNamespaceURI(nsURI);
    }
    else
      rawNode->GetNamespaceURI(nsURI);
    PRInt32 nsID(kNameSpaceID_Unknown);
    msiNameSpaceUtils::GetNameSpaceID(nsURI, nsID);
    if (nsID == kNameSpaceID_MathML)
    { 
      if (text)
      { 
        nsCOMPtr<nsIContent> tc(do_QueryInterface(text));
        if (tc && tc->TextIsOnlyWhitespace())
        {
          rv = msiIMathMLEditingBC::MSI_WHITESPACE; 
          mathmlNode = rawNode;
          length = 0;
        }
        else if (parent)
        {
          parent->GetLocalName(localName);
          length = localName.Length();
					if (!localName.EqualsLiteral("mtext"))
          	mathmlNode = parent;
        }
      }
      else
      {
        rawNode->GetLocalName(localName);
        length = localName.Length();
        mathmlNode = rawNode;
      }
      if (length)
        rv = GetMMLNodeTypeFromLocalName(mathmlNode, localName);
      if (rv == msiIMathMLEditingBC::MATHML_MROWBOUNDFENCE ||
          rv == msiIMathMLEditingBC::MATHML_MROWFENCE)
        SetMathmlNodeAndOffsetForMrowFence(localName, mathmlNode, offset);
    }
  }  
  return rv;
}

PRBool msiEditingManager::NodeInMath(nsIDOMNode* node)
{
  PRBool rv(PR_FALSE);
  NS_ASSERTION(node, "Null node passed to NodeInMath");
  if (node)
    SupportsMathMLInsertionInterface(node, &rv);
  return rv;
}

//SLS this doesn't do what the name says...
nsresult msiEditingManager::EnsureMathWithSelectionCollapsed(nsIDOMNode * node)
{
  nsresult res(NS_OK);
  if (!NodeInMath(node))
  {
    //TODO
    res = NS_ERROR_FAILURE;
  }
  return res;
}

NS_IMETHODIMP
msiEditingManager::InsertMathmlElement(nsIEditor * editor,
                                       nsISelection * selection, 
                                       nsIDOMNode* node, 
                                       PRUint32 offset,
                                       PRUint32 flags,
                                       nsIDOMElement* mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(editor && selection && node, "Null editor, selection or node passed to msiEditingManager::InsertFraction");
  if (editor && selection && node && mathmlElement)
  {
    PRBool transacting(PR_FALSE);
    //SLS need to check that selection collapsed?
    nsCOMPtr<msiIMathMLEditor> mathmlEditor(do_QueryInterface(editor));
    nsCOMPtr<nsIDOMNode> mathnode;
    res = mathmlEditor->NodeInMath(node, getter_AddRefs(mathnode));
    if (!mathnode)
    {
      editor->BeginTransaction();
      editor->SaveSelection(selection);
      transacting = PR_TRUE;
      res = InsertMath(editor, selection, node, offset, flags, PR_FALSE);
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIDOMNode> anchorNode;
        res = selection->GetAnchorNode(getter_AddRefs(anchorNode));
        if (NS_SUCCEEDED(res))
        {
          node = anchorNode;
          offset = 1;
        }
      }
    }
    else
    {
      res = NS_OK;
    }
    if (NS_SUCCEEDED(res))
    {
      nsCOMPtr<nsIDOMNode> newMathmlNode;
      newMathmlNode = do_QueryInterface(mathmlElement);
      NS_ASSERTION(newMathmlNode, "Failed to get nsIDOMNode interface from nsIDOMElement");
      if (newMathmlNode)
      {
        nsCOMPtr<msiIMathMLInsertion> mathmlEditing;
        GetMathMLInsertionInterface(node, offset, getter_AddRefs(mathmlEditing));
        if (mathmlEditing)
        {
          if (!transacting)
          {
            editor->BeginTransaction();
            editor->SaveSelection(selection);
            transacting = PR_TRUE;
          }
          res = mathmlEditing->InsertNode(editor, selection, newMathmlNode, flags);
        }  
      }
    }
    if (transacting)
      editor->EndTransaction();

  }
  return res;
}      

PRUint32
msiEditingManager::GetMMLNodeTypeFromLocalName(nsIDOMNode * mathmlNode, 
                                               const nsAString & localName)
{
  PRUint32 rv(msiIMathMLEditingBC::MATHML_UNKNOWN);
  PRUint32 length(localName.Length());
  PRBool checkForBigOp(PR_FALSE);
  if (length)
  {
    switch (length)
    {
      case 2:
        if (msiEditingAtoms::mi->Equals(localName))
        {
          rv = msiIMathMLEditingBC::MATHML_MI;
          nsCOMPtr<nsIDOMNamedNodeMap> attrMap;
          mathmlNode->GetAttributes(getter_AddRefs(attrMap));
          if (attrMap)
          {
            nsCOMPtr<nsIDOMNode> attrNode;
            nsAutoString tempinput;
            msiEditingAtoms::tempinput->ToString(tempinput);
            attrMap->GetNamedItem(tempinput, getter_AddRefs(attrNode));
            if (attrNode)
            {
              nsCOMPtr<nsIDOMAttr> attribute(do_QueryInterface(attrNode));
              if (attribute)
              {
                nsAutoString value;
                attribute->GetValue(value);
	            if (msiEditingAtoms::msitrue->Equals(value))
                  rv = msiIMathMLEditingBC::MSI_INPUTBOX;
              }
            }
          }
        }
        else if (msiEditingAtoms::mn->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MN;
        else if (msiEditingAtoms::mo->Equals(localName))
        {
          rv = GetMoNodeType(mathmlNode);
          if (rv == msiIMathMLEditingBC::MATHML_MO)
            checkForBigOp = PR_TRUE;
        }
        else if (msiEditingAtoms::ms->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MS;
        break;
      case 3:
        if (msiEditingAtoms::mtr->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MTR;
        else if (msiEditingAtoms::mtd->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MTD;
        break;
      case 4:
        if (msiEditingAtoms::math->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MATH;
        else if (msiEditingAtoms::mrow->Equals(localName))
          rv = GetMRowNodeType(mathmlNode);
        else if (msiEditingAtoms::msub->Equals(localName))
        {
          checkForBigOp = PR_TRUE;
          rv = msiIMathMLEditingBC::MATHML_MSUB;
        }
        else if (msiEditingAtoms::msup->Equals(localName))
        {
          checkForBigOp = PR_TRUE;
          rv = msiIMathMLEditingBC::MATHML_MSUP;
        }
        break;
      case 5:
        if (msiEditingAtoms::mfrac->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MFRAC;
        else if (msiEditingAtoms::msqrt->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MSQRT;
        else if (msiEditingAtoms::mroot->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MROOT;
        else if (msiEditingAtoms::mtext->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MTEXT;
        else if (msiEditingAtoms::mover->Equals(localName))
        {
          checkForBigOp = PR_TRUE;
          rv = msiIMathMLEditingBC::MATHML_MOVER;
        }
        break;
      case 6:
        if (msiEditingAtoms::mspace->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MSPACE;
        else if (msiEditingAtoms::mglyph->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MGLYPH;
        else if (msiEditingAtoms::mstyle->Equals(localName))
        {
          checkForBigOp = PR_TRUE;
          rv = msiIMathMLEditingBC::MATHML_MSTYLE;
        }
        else if (msiEditingAtoms::merror->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MERROR;
        else if (msiEditingAtoms::munder->Equals(localName))
        {
          checkForBigOp = PR_TRUE;
          rv = msiIMathMLEditingBC::MATHML_MUNDER;
        }
        else if (msiEditingAtoms::mtable->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MTABLE;
        break;
      case 7:
        if (msiEditingAtoms::maction->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MACTION;
        else if (msiEditingAtoms::mpadded->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MPADDED;
        else if (msiEditingAtoms::mfenced->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MFENCED;
        else if (msiEditingAtoms::msubsup->Equals(localName))
        {
          checkForBigOp = PR_TRUE;
          rv = msiIMathMLEditingBC::MATHML_MSUBSUP;
        }
        break;
      case 8:
        if (msiEditingAtoms::mphantom->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MPHANTOM;
        else if (msiEditingAtoms::menclose->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MENCLOSE;
        break;
      case 10:
        if (msiEditingAtoms::munderover->Equals(localName))
        {
          checkForBigOp = PR_TRUE;
          rv = msiIMathMLEditingBC::MATHML_MUNDEROVER;
        }
        else if (msiEditingAtoms::mlabeledtr->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MLABELEDTR;
        else if (msiEditingAtoms::malignmark->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MALIGNMARK;
        break;
      case 11:
        if (msiEditingAtoms::maligngroup->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MALIGNGROUP;
        break;
      case 13:
        if (msiEditingAtoms::mmultiscripts->Equals(localName))
          rv = msiIMathMLEditingBC::MATHML_MMULTISCRIPTS;
        break;
      default:
          rv = msiIMathMLEditingBC::MATHML_UNKNOWN;
        break;
    }
    if (checkForBigOp && IsBigOperator(mathmlNode, localName))
      rv = msiIMathMLEditingBC::MSI_BIGOPERATOR;
  }
  return rv;
}  

PRUint32 msiEditingManager::GetMRowNodeType(nsIDOMNode* mathmlNode)
{
  if (!mathmlNode)
    return msiIMathMLEditingBC::MATHML_UNKNOWN;
  PRUint32 rv(msiIMathMLEditingBC::MATHML_MROW);
  nsCOMPtr<nsIArray> children;
  PRUint32 numKids(0);
  nsresult res = msiUtils::GetNonWhitespaceChildren(mathmlNode, children);
  if (NS_SUCCEEDED(res) && children)
    res = children->GetLength(&numKids);
  if (NS_SUCCEEDED(res) && numKids >= 3)
  {
    nsCOMPtr<nsIDOMNode> first;
    nsCOMPtr<nsIDOMNode> last;
    children->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(first));
    children->QueryElementAt(numKids-1, NS_GET_IID(nsIDOMNode), getter_AddRefs(last));
    if (first && last)
    {
    
      if (msiNameSpaceUtils::GetNameSpaceID(first) == kNameSpaceID_MathML &&
          msiNameSpaceUtils::GetNameSpaceID(last) == kNameSpaceID_MathML)
      { 
        nsAutoString firstName, lastName;
        first->GetLocalName(firstName);
        last->GetLocalName(lastName);
        if (msiEditingAtoms::mo->Equals(firstName) && msiEditingAtoms::mo->Equals(lastName))
        {
          nsCOMPtr<nsIDOMElement> firstElement(do_QueryInterface(first));
          nsCOMPtr<nsIDOMElement> lastElement(do_QueryInterface(last));
          if (firstElement && lastElement)
          {
            nsAutoString firstVal, lastVal, fence;
            msiEditingAtoms::fence->ToString(fence);
            firstElement->GetAttribute(fence, firstVal);
            lastElement->GetAttribute(fence, lastVal);
            if (msiEditingAtoms::msitrue->Equals(firstVal) && msiEditingAtoms::msitrue->Equals(lastVal))
            {
              nsAutoString form;
              msiEditingAtoms::form->ToString(form);
              firstElement->GetAttribute(form, firstVal);
              lastElement->GetAttribute(form, lastVal);
              if (msiEditingAtoms::prefix->Equals(firstVal) && msiEditingAtoms::postfix->Equals(lastVal))
              {
                nsAutoString msiBoundFence;
                msiEditingAtoms::msiBoundFence->ToString(msiBoundFence);
                firstElement->GetAttribute(msiBoundFence, firstVal);
                lastElement->GetAttribute(msiBoundFence, lastVal);
                if (msiEditingAtoms::msitrue->Equals(firstVal) && msiEditingAtoms::msitrue->Equals(lastVal))
                  rv = msiIMathMLEditingBC::MATHML_MROWBOUNDFENCE;
                else
                  rv = msiIMathMLEditingBC::MATHML_MROWFENCE;
              }
            }
          }
        }
      }  
    }
  }
  return rv;
}

PRBool msiEditingManager::IsBigOperator(nsIDOMNode* mathmlNode, const nsAString & localName) 
{
  PRBool rv(PR_FALSE);
  nsCOMPtr<nsIDOMNode> mo, mstyle, script;
  PRUint32 scriptType(msiIMathMLEditingBC::MATHML_UNKNOWN);
  rv = GetBigOpNodes(mathmlNode, localName, mo, mstyle, script, scriptType);
  return rv; 
}

PRBool msiEditingManager::GetBigOpNodes(nsIDOMNode* mathmlNode, const nsAString & localName,
                                        nsCOMPtr<nsIDOMNode> & mo, nsCOMPtr<nsIDOMNode> & mstyle,
                                        nsCOMPtr<nsIDOMNode> & script, PRUint32 &scriptType) 
{
  PRBool rv(PR_FALSE);
  nsresult res(NS_OK);
  mo = nsnull;
  mstyle = nsnull;
  script = nsnull;
  scriptType = msiIMathMLEditingBC::MATHML_UNKNOWN;
  if (!mathmlNode)
     return rv;
  if (msiEditingAtoms::mo->Equals(localName)         || msiEditingAtoms::mstyle->Equals(localName) ||
      msiEditingAtoms::munder->Equals(localName)     || msiEditingAtoms::mover->Equals(localName)  ||
      msiEditingAtoms::munderover->Equals(localName) || msiEditingAtoms::msub->Equals(localName)   ||
      msiEditingAtoms::msup->Equals(localName)       || msiEditingAtoms::msubsup->Equals(localName) )
  {    
    if (msiEditingAtoms::mo->Equals(localName))
    {
      mo = mathmlNode;
      nsCOMPtr<nsIDOMNode> parent;
      res = mo->GetParentNode(getter_AddRefs(parent));
      if (NS_SUCCEEDED(res) && parent)
      {
        nsAutoString name;
        parent->GetLocalName(name);
        if (msiEditingAtoms::mstyle->Equals(name))
        {
          mstyle = parent;
          parent = nsnull;
          name.Truncate(0);
          res = mstyle->GetParentNode(getter_AddRefs(parent));
          if (NS_SUCCEEDED(res) && parent)
            parent->GetLocalName(name);
        }
        if (msiEditingAtoms::munder->Equals(name)     || msiEditingAtoms::mover->Equals(name) ||
            msiEditingAtoms::munderover->Equals(name) || msiEditingAtoms::msub->Equals(name) ||
            msiEditingAtoms::msup->Equals(name)       || msiEditingAtoms::msubsup->Equals(name))
          script = parent;
      }
    }
    else if (msiEditingAtoms::mstyle->Equals(localName))
    {
      nsCOMPtr<nsIArray> children;
      PRUint32 numKids(0);
      nsresult res = msiUtils::GetNonWhitespaceChildren(mathmlNode, children);
      if (NS_SUCCEEDED(res) && children)
        res = children->GetLength(&numKids);
      if (NS_SUCCEEDED(res) && numKids == 1)
      {
        nsCOMPtr<nsIDOMNode> kid;
        res = children->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(kid));
        if (NS_SUCCEEDED(res) && kid)
        {
          nsAutoString kidName;
          res = kid->GetLocalName(kidName);
          if (NS_SUCCEEDED(res) && msiEditingAtoms::mo->Equals(kidName))
          {
            mo = kid;
            mstyle = mathmlNode;
          }
        }
      }
      if (mstyle && mo)
      {
        nsCOMPtr<nsIDOMNode> parent;
        nsAutoString name;
        res = mstyle->GetParentNode(getter_AddRefs(parent));
        if (NS_SUCCEEDED(res) && parent)
          parent->GetLocalName(name);
        if (msiEditingAtoms::munder->Equals(name)     || msiEditingAtoms::mover->Equals(name) ||
            msiEditingAtoms::munderover->Equals(name) || msiEditingAtoms::msub->Equals(name) ||
            msiEditingAtoms::msup->Equals(name)       || msiEditingAtoms::msubsup->Equals(name))
          script = parent;
      }
    }
    else // localname is one of the scripts
    {
      nsCOMPtr<nsIArray> children;
      PRUint32 numKids(0);
      nsresult res = msiUtils::GetNonWhitespaceChildren(mathmlNode, children);
      if (NS_SUCCEEDED(res) && children)
        res = children->GetLength(&numKids);
      if (NS_SUCCEEDED(res) && numKids <= 3)
      {
        nsCOMPtr<nsIDOMNode> base;
        res = children->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(base));
        if (NS_SUCCEEDED(res) && base)
        {
          nsAutoString name;
          base->GetLocalName(name);
          if (msiEditingAtoms::mo->Equals(name))
          {
            mo = base;
            script = mathmlNode;
          }
          else if (msiEditingAtoms::mstyle->Equals(name))
          {
            nsCOMPtr<nsIArray> children;
            PRUint32 numKids(0);
            nsresult res = msiUtils::GetNonWhitespaceChildren(base, children);
            if (NS_SUCCEEDED(res) && children)
              res = children->GetLength(&numKids);
            if (NS_SUCCEEDED(res) && numKids == 1)
            {
              nsCOMPtr<nsIDOMNode> kid;
              res = children->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(kid));
              if (NS_SUCCEEDED(res) && kid)
              {
                nsAutoString kidName;
                res = kid->GetLocalName(kidName);
                if (NS_SUCCEEDED(res) && msiEditingAtoms::mo->Equals(kidName))
                {
                  mo = kid;
                  mstyle = base;
                  script = mathmlNode;
                }
              }
            }
          }
        }  
      }
    }
    if (mo)
    {
      //TODO --- should we only allow the fixed set of symbol from the dialog? 
      //TODO --- should big ops have form ="prefix" set? 
      nsCOMPtr<nsIDOMElement> moElement(do_QueryInterface(mo));
      if (moElement)
      {
        nsAutoString value, largeop;
        msiEditingAtoms::largeop->ToString(largeop);
        res = moElement->GetAttribute(largeop,  value);
        if (NS_SUCCEEDED(res))
          rv = msiEditingAtoms::msitrue->Equals(value);
      }
      if (rv && mstyle)
      {
        nsCOMPtr<nsIDOMElement> styleElement(do_QueryInterface(mstyle));
        if (styleElement)
        {
          nsAutoString value, displaystyle;
          msiEditingAtoms::displaystyle->ToString(displaystyle);
          res = styleElement->GetAttribute(displaystyle,  value);
          if (NS_SUCCEEDED(res) && (msiEditingAtoms::msitrue->Equals(value) || 
                                    msiEditingAtoms::msifalse->Equals(value)))
          {
            nsCOMPtr<nsIDOMNamedNodeMap> attrMap;
            mstyle->GetAttributes(getter_AddRefs(attrMap));
            PRUint32 numAttr(0);
            if (attrMap)
              attrMap->GetLength(&numAttr);
            if (numAttr > 1)
            {
              //TODO -- do we care about other attributes which may be set -- which may conflict?
            }
          }
          else
          {
            mstyle = nsnull;
            script = nsnull;
          }
        }
      }
      if (script)
      {
        nsAutoString scriptName;
        script->GetLocalName(scriptName);
        if (msiEditingAtoms::munder->Equals(scriptName))
          scriptType = msiIMathMLEditingBC::MATHML_MUNDER;
        else if (msiEditingAtoms::mover->Equals(scriptName))
          scriptType = msiIMathMLEditingBC::MATHML_MOVER;
        else if (msiEditingAtoms::munderover->Equals(scriptName))
          scriptType = msiIMathMLEditingBC::MATHML_MUNDEROVER;
        else if (msiEditingAtoms::msup->Equals(scriptName))
          scriptType = msiIMathMLEditingBC::MATHML_MSUP;
        else if (msiEditingAtoms::msubsup->Equals(scriptName))
          scriptType = msiIMathMLEditingBC::MATHML_MSUBSUP;
        else if (msiEditingAtoms::msub->Equals(scriptName))
          scriptType = msiIMathMLEditingBC::MATHML_MSUB;
      }
    }
    if (!rv)
    {
      mo = nsnull;
      mstyle = nsnull;
      script = nsnull;
    }
  }
  return rv;
}

PRUint32 msiEditingManager::GetMoNodeType(nsIDOMNode * mathmlNode)
{
  PRUint32 mathmlType(msiIMathMLEditingBC::MATHML_MO);
  PRUint32 parentType(msiIMathMLEditingBC::MATHML_UNKNOWN);
  nsCOMPtr<nsIDOMNode> parent;
  if (mathmlNode)
    mathmlNode->GetParentNode(getter_AddRefs(parent));
  if (parent)
  {
    PRUint32 parentType = GetMRowNodeType(parent);
    if (parentType == msiIMathMLEditingBC::MATHML_MROWBOUNDFENCE ||
        parentType == msiIMathMLEditingBC::MATHML_MROWFENCE)
    {
      nsCOMPtr<nsIArray> children;
      PRUint32 numKids(0);
      nsresult res = msiUtils::GetNonWhitespaceChildren(parent, children);
      if (NS_SUCCEEDED(res) && children)
        res = children->GetLength(&numKids);
      if (NS_SUCCEEDED(res) && numKids >= 3)
      {
        nsCOMPtr<nsIDOMNode> first;
        nsCOMPtr<nsIDOMNode> last;
        children->QueryElementAt(0, NS_GET_IID(nsIDOMNode), getter_AddRefs(first));
        children->QueryElementAt(numKids-1, NS_GET_IID(nsIDOMNode), getter_AddRefs(last));
        if (first && last && (first == mathmlNode || last == mathmlNode))
          mathmlType = parentType;   
      }  
    }   
  }
  return mathmlType;    
}

void msiEditingManager::SetMathmlNodeAndOffsetForMrowFence(const nsAString & localName,
                                                           nsCOMPtr<nsIDOMNode> & mathmlNode,
                                                           PRUint32 & offset)
{
  if (msiEditingAtoms::mo->Equals(localName))
  {
    PRUint32 index(0);
    nsCOMPtr<nsIDOMNode> mrowFence;
    mathmlNode->GetParentNode(getter_AddRefs(mrowFence));
    msiUtils::GetIndexOfChildInParent(mathmlNode, index);
    if(offset <= msiIMathMLEditingBC::LAST_VALID)
    {
      if (offset > 0)
        index += 1;
      offset = index;
    }
    mathmlNode = mrowFence;  
  }
}                                                        

nsresult msiEditingManager::AddMatrixRows(nsIEditor * editor, nsIDOMNode *aMatrix, PRUint32 insertAt, PRUint32 howMany)
{
  nsresult retVal(NS_ERROR_FAILURE);
  PRInt32 nRows(0), nCols(0);
  PRBool markCaret(PR_FALSE);  //may want to add as parameter??
  nsTArray<msiMultiSpanCellInfo> matrixInfoArray;
  retVal = GetMatrixInfo(aMatrix, nRows, nCols, nsnull, &matrixInfoArray);
  if (NS_SUCCEEDED(retVal))
  {
    msiMultiSpanCellInfo** ourRowData = new msiMultiSpanCellInfo*[nCols];
    for (PRInt32 ii = 0; ii < nCols; ++ii)
      ourRowData[ii] = nsnull;
    PRUint32 startRow, startCol;
    PRUint32 rowSpan, colSpan;
    for (PRUint32 jj = 0; jj < matrixInfoArray.Length(); ++jj)
    {
      startRow = matrixInfoArray[jj].StartRow();
      rowSpan = matrixInfoArray[jj].RowSpan();
      if ( (startRow < insertAt) && (!rowSpan || (insertAt < startRow + rowSpan)) )
      {
        startCol = matrixInfoArray[jj].StartCol();
        ourRowData[startCol] = new msiMultiSpanCellInfo(matrixInfoArray[jj]);
      }
    }
    //Now we know which cells will be continuation cells; all that's left is to insert the rows and add the non-continuation cells.
    nsCOMPtr<nsIDOMElement> aRow, aCell;
    PRUint32 insertPos = 0;
    nsCOMPtr<nsIDOMNode> prevRow, insertParent;
    nsCOMPtr<nsIDOMNode> nextRow = nsnull;
    nsresult res = NS_OK;
    for (PRUint32 ix = 0; NS_SUCCEEDED(res) && ix <= insertAt; ++ix)
    {
      prevRow = nextRow;
      res = GetNextMatrixRow(aMatrix, prevRow, getter_AddRefs(nextRow));
      if (!nextRow)
        break;
    }
    if (nextRow)
    {
      res = nextRow->GetParentNode(getter_AddRefs(insertParent));
      msiUtils::GetIndexOfChildInParent(nextRow, insertPos);
//      insertPos = editor->GetIndexOf(insertParent, nextRow);
    }
    else
    {
      insertParent = aMatrix;
      res = aMatrix->GetLastChild(getter_AddRefs(prevRow));
      while (msiUtils::IsWhitespace(prevRow))
      {
        nextRow = prevRow;
        res = nextRow->GetPreviousSibling(getter_AddRefs(prevRow));
        if (!NS_SUCCEEDED(res) || !prevRow)
          break;
      }
//      insertPos = editor->GetIndexOf(insertParent, prevRow);
      msiUtils::GetIndexOfChildInParent(prevRow, insertPos);
      insertPos++;  //Want to go after prevRow
    }

    for (PRUint32 ii = 0; ii < howMany; ++ii)
    {
      retVal = msiUtils::CreateMathMLElement(editor, msiEditingAtoms::mtr, aRow);
      if (NS_SUCCEEDED(retVal) && aRow)
      {
        nsAutoString spanAttrStr;
        spanAttrStr.AssignASCII("rowspan");
        nsAutoString rowSpanStr;
        PRUint32 theSpan;
        for (PRInt32 jj = 0; jj < nCols; ++jj)
        {
          if (ourRowData[jj])
          {
            aCell = do_QueryInterface(ourRowData[jj]->Node());
            theSpan = ourRowData[jj]->RowSpan();
            if (theSpan != 0)          //In this case, the row we're inserting is cutting across a multi-row cell - just extend it
            {
              rowSpanStr.Truncate();
              rowSpanStr.AppendInt( ++theSpan );
              editor->SetAttribute(aCell, spanAttrStr, rowSpanStr);
            }
            colSpan = ourRowData[jj]->ColSpan();
            if (!colSpan)
              jj = nCols;
            else
              jj += colSpan - 1;   //Want to get past this multi-col multi-row cell so we don't edit its properties again
          }
          else
          {
            PRBool doMarkCaret(PR_FALSE);
            if (ii == 0 && jj == 0 && markCaret)
              doMarkCaret = PR_TRUE;
            PRUint32 flags = 0;
            retVal = msiUtils::CreateMtd(editor, doMarkCaret, flags, aCell);
            if (NS_SUCCEEDED(retVal) && aCell)
            {
              nsCOMPtr<nsIDOMNode> dontcare; 
              retVal = aRow->AppendChild(aCell, getter_AddRefs(dontcare));
            }
            else
              retVal = NS_ERROR_FAILURE;  
          }
        }
        //Then insert aRow - is it safe to assume that each direct child of <mtable> is an <mtr> or an inferred <mtr> at worst?
        //Can there be <mstyle>??? Should probably manage to deal with this somehow...
        retVal = editor->InsertNode(aRow, insertParent, insertPos++);
      }
    }
    delete[] ourRowData;
  }

  return retVal;
}

nsresult msiEditingManager::AddMatrixColumns(nsIEditor * editor, nsIDOMNode *aMatrix, PRUint32 insertAt, PRUint32 howMany)
{
  nsresult retVal(NS_ERROR_FAILURE);
  PRInt32 nRows(0), nCols(0);
  PRBool markCaret(PR_FALSE);  //may want to add as parameter??
  nsTArray<msiMultiSpanCellInfo> matrixInfoArray;
  retVal = GetMatrixInfo(aMatrix, nRows, nCols, nsnull, &matrixInfoArray);
  PRBool bAtEnd = (insertAt > nCols);
  if (NS_SUCCEEDED(retVal))
  {
    msiMultiSpanCellInfo** ourColData = new msiMultiSpanCellInfo*[nRows];
    for (PRInt32 ii = 0; ii < nRows; ++ii)
      ourColData[ii] = nsnull;
    PRUint32 startRow, startCol;
    PRUint32 rowSpan, colSpan;
    for (PRUint32 ii = 0; ii < matrixInfoArray.Length(); ++ii)
    {
      startCol = matrixInfoArray[ii].StartCol();
      colSpan = matrixInfoArray[ii].ColSpan();
      if ( (startCol < insertAt) && (!colSpan || (insertAt < startCol + colSpan)) )
      {
        startRow = matrixInfoArray[ii].StartRow();
        ourColData[startRow] = new msiMultiSpanCellInfo(matrixInfoArray[ii]);
      }
    }

    //Now we know which cells will be continuation cells; all that's left is to insert the non-continuation cells in each row.
    nsCOMPtr<nsIDOMNode> lastRow = nsnull;
    nsCOMPtr<nsIDOMNode> nextCell = nsnull;
    nsCOMPtr<nsIDOMNode> aCell, prevCell, nextRow;
    nsCOMPtr<nsIDOMElement> asElement;
    nsresult res = NS_OK;
    nsAutoString spanAttrStr;
    spanAttrStr.AssignASCII("colspan");
    nsAutoString colSpanStr;
    PRUint32 theSpan;
    PRUint32 insertPos;

    for (PRInt32 ii = 0; ii < nRows; ++ii)
    {
      res = GetNextMatrixRow(aMatrix, lastRow, getter_AddRefs(nextRow));
      if (ourColData[ii])
      {
        aCell = ourColData[ii]->Node();
        theSpan = ourColData[ii]->ColSpan();
        if (theSpan != 0)          //In this case, the column we're inserting is cutting across a multi-col cell - just extend it
        {
          colSpanStr.Truncate();
          colSpanStr.AppendInt( theSpan + howMany );
          asElement = do_QueryInterface(aCell);
          editor->SetAttribute(asElement, spanAttrStr, colSpanStr);
        }
        rowSpan = ourColData[ii]->RowSpan();
        if (!rowSpan)
          ii = nRows;
        else
          ii += rowSpan - 1;   //Want to get past this multi-col multi-row cell so we don't edit its properties again
      }
      else
      {
        //First find insertPos for this row:
        if (bAtEnd)
        {
          prevCell = nsnull;
          res = nextRow->GetLastChild(getter_AddRefs(prevCell));
          if (NS_SUCCEEDED(res) || !prevCell)
            insertPos = 0;
          else
          {
            while (msiUtils::IsWhitespace(prevCell))
            {
              nextCell = prevCell;
              res = nextCell->GetPreviousSibling(getter_AddRefs(prevCell));
              if (!NS_SUCCEEDED(res) || !prevCell)
                break;
            }
            msiUtils::GetIndexOfChildInParent(prevCell, insertPos);
//            insertPos = editor->GetIndexOf(nextRow, prevCell);
            ++insertPos;
          }
        }
        else if (insertAt > 0)
        {
          res = GetMatrixCellAt(aMatrix, ii+1, insertAt, getter_AddRefs(prevCell));  //Note that we know this isn't a continuation cell
          msiUtils::GetIndexOfChildInParent(prevCell, insertPos);
          ++insertPos;
//          insertPos = editor->GetIndexOf(nextRow, prevCell);
        }
        else
          insertPos = 0;

        for (PRUint32 jj = 0; jj < howMany; ++jj)
        {
          PRBool doMarkCaret(PR_FALSE);
          if (jj == 0 && markCaret)
           doMarkCaret = PR_TRUE;
//        ??  retVal = msiUtils::CreateMathMLElement(editor, msiEditingAtoms::mtd, aCell);
          PRUint32 flags = 0;
          asElement = nsnull;
          retVal = msiUtils::CreateMtd(editor, doMarkCaret, flags, asElement);
          if (NS_SUCCEEDED(retVal) && asElement)
          {
            aCell = do_QueryInterface(asElement);
            nsCOMPtr<nsIDOMNode> dontcare; 
            retVal = editor->InsertNode(aCell, nextRow, insertPos++);
          }
          else
            retVal = NS_ERROR_FAILURE;  
        }
      }
      lastRow = nextRow;
    }
    delete[] ourColData;
  }
  return retVal;
}


nsresult msiEditingManager::GetFirstMatrixRow(nsIDOMNode* aMatrix, nsIDOMNode** pRowOut)
{
  return GetNextMatrixRow(aMatrix, nsnull, pRowOut);
}

nsresult msiEditingManager::GetNextMatrixRow(nsIDOMNode* aMatrix, nsIDOMNode* currRow, nsIDOMNode** pNextRowOut)
{
  nsresult retVal = NS_ERROR_FAILURE;
  if (!pNextRowOut)
  {
    NS_ASSERTION(PR_FALSE, "Null row pointer passed in to msiEditingManager::GetNextMatrixRow!");
    return retVal;
  }
  nsCOMPtr<nsIDOMNode> parent, candidate;
  if (currRow && aMatrix)
  {
    PRBool bIsDescendant = PR_FALSE;
    candidate = currRow;
    do
    {
      candidate->GetParentNode(getter_AddRefs(parent));
      if (aMatrix == parent)
        bIsDescendant = PR_TRUE;
      candidate = parent;
    }
    while (!bIsDescendant && parent);
    if (!bIsDescendant)
    {
      NS_ASSERTION(PR_FALSE, "Matrix row passed in to GetNextMatrixRow not a descendant of the matrix.");
      return retVal;
    }
  }

  *pNextRowOut = nsnull;
  nsCOMPtr<nsIDOMNode> nextSibling;
//  if (currRow)
//    candidate = do_QueryInterface(currRow);
  candidate = currRow;
  nsCOMPtr<nsIDOMNode> matrixNode = aMatrix;
  parent = aMatrix;
  nsCOMPtr<nsIDOMElement> asElt;
  PRBool bDone = PR_FALSE;
  do
  {
    nsresult res = NS_ERROR_FAILURE;
    if (candidate)
      res = candidate->GetNextSibling(getter_AddRefs(nextSibling));
    else if (parent) //Can only happen the first time around, if we're called with a null currRow?
      res = parent->GetFirstChild(getter_AddRefs(nextSibling));
    if (NS_SUCCEEDED(res) && nextSibling)
    {
      candidate = nextSibling;

      nsAutoString nodeName;
      candidate->GetLocalName(nodeName);
      if (!msiUtils::IsWhitespace(candidate))
      {
        if (nodeName.EqualsLiteral("mstyle"))
        {
          nsCOMPtr<nsIArray> styleKids;
          nsCOMPtr<nsIDOMNode> styleKid;
          nsAutoString kidName;
          PRUint32 numStyleKids(0);
          res = msiUtils::GetNonWhitespaceChildren(candidate, styleKids);
          if (NS_SUCCEEDED(res) && styleKids)
          {
            res = styleKids->GetLength(&numStyleKids);
            if (NS_SUCCEEDED(res))
            {
              for (PRUint32 jj = 0; !bDone && (jj < numStyleKids); ++jj)
              {
                styleKids->QueryElementAt(jj, NS_GET_IID(nsIDOMNode), getter_AddRefs(styleKid));
                styleKid->GetLocalName(kidName);
                if (kidName.EqualsLiteral("mtr") || kidName.EqualsLiteral("mlabeldtr"))  //In this case the children of the mstyle are the rows, since we found one that is
                {
//                  children->QueryElementAt(0, NS_GET_IID(nsIDOMElement), getter_AddRefs(nextRowOut));
                  *pNextRowOut = styleKid;
                  NS_ADDREF(*pNextRowOut);
                  retVal = NS_OK;
                  bDone = PR_TRUE;
                }
              }
              if (!bDone)
              {
                *pNextRowOut = candidate;  //the style element itself
                NS_ADDREF(*pNextRowOut);
                retVal = NS_OK;
                bDone = PR_TRUE;
              }
            }
          }
        }
//        else if (nodeName->EqualsLiteral("mtr") || nodeName->EqualsLiteral("mlabeldtr") || nodeName->EqualsLiteral("mtd"))
        else  //We'll treat any non-whitespace non-mstyle node as a row, in accordance with the old MathML spec...
        {
          *pNextRowOut = candidate;
          NS_ADDREF(*pNextRowOut);
          retVal = NS_OK;
          bDone = PR_TRUE;
        }
      }
    }
    else
    {
      res = candidate->GetParentNode(getter_AddRefs(parent));
      if (NS_SUCCEEDED(res) && parent)
      {
        if (parent == matrixNode)  //we're at the end - return null
        {
          retVal = NS_OK;
          *pNextRowOut = nsnull;
          bDone = PR_TRUE;
        }
        else
          candidate = parent;  //Then we go through again looking at parent's next sibling
      }
    }
  }
  while (candidate && !bDone);

  return retVal;
}

nsresult msiEditingManager::GetMatrixSize(nsIDOMNode *aMatrix,
                           PRInt32* aRowCount, PRInt32* aColCount)
{
  PRInt32 rowCount(0), colCount(0);
  nsresult retVal = GetMatrixInfo(aMatrix, rowCount, colCount, nsnull, nsnull);
  if (NS_SUCCEEDED(retVal))
  {
    *aRowCount = rowCount;
    *aColCount = colCount;
  }
  return retVal;
}

nsresult msiEditingManager::FindMatrixCell(nsIDOMNode* aMatrix, nsIDOMNode *aCell, PRInt32* whichRow, PRInt32* whichCol)
{
  PRInt32 nRow(0), nCol(0);
  nsresult retVal = GetMatrixInfo(aMatrix, nRow, nCol, &aCell, nsnull);
  if (NS_SUCCEEDED(retVal))
  {
    *whichRow = nRow;
    *whichCol = nCol;
  }
  return retVal;
}

nsresult msiEditingManager::GetMatrixCellAt(nsIDOMNode* aMatrix, PRInt32 whichRow, PRInt32 whichCol, nsIDOMNode** pCell)
{
  nsresult retVal(NS_ERROR_FAILURE);
  NS_ASSERTION(pCell, "Null cell pointer passed to msiEditingManager::GetMatrixCellAt");
  NS_ASSERTION(whichRow && whichCol, "Bad row or column index passed to msiEditingManager::GetMatrixCellAt");
  nsIDOMNode* pCellNode = nsnull;
  if (pCell && whichRow && whichCol)
    retVal = GetMatrixInfo(aMatrix, whichRow, whichCol, &pCellNode, nsnull);
  if (NS_SUCCEEDED(retVal))
//    *pCell = getter_AddRefs(pCellNode);
    *pCell = pCellNode;
  return retVal;
}


//This is a general-purpose function:
//  If nCol and nRow are nonzero (and pFindCell is nonzero), we should find the cell at the given row and col and return it in pFindCell.
//  If nRow and nCol are zero but *pFindCell is not, we should find the starting row and column containing *pFindCell in nRow and nCol.
//  Otherwise, we return the number of rows and columns in nCol and nRow.
//  Additionally, if multiCellArray is nonzero, we should fill the array with any multi-span cell information.
nsresult msiEditingManager::GetMatrixInfo(nsIDOMNode *aMatrix, PRInt32& nRow, PRInt32& nCol, 
                                          nsIDOMNode** pFindCell, nsTArray<msiMultiSpanCellInfo> *multiCellArray)
{
//  NS_ENSURE_ARG_POINTER(aRowCount);
//  NS_ENSURE_ARG_POINTER(aColCount);
  nsresult res;
  nsCOMPtr<nsIDOMNode> matrix;

  // Get the selected matrix or the matrix enclosing the selection anchor
//  res = GetElementOrParentByTagName(NS_LITERAL_STRING("mtable"), aMatrix, getter_AddRefs(matrix));
  res = msiUtils::GetMathTagParent(aMatrix, msiEditingAtoms::mtable, matrix);
  if (NS_FAILED(res))
    return res;
  if (!matrix)
    return NS_ERROR_FAILURE;

  PRBool bDone(PR_FALSE), bAllUsed(PR_FALSE);
  nsCOMPtr<nsIDOMNode> prevRow, nextRow;
  nsCOMPtr<nsIDOMNode> currCellNode;

  nsCOMPtr<nsIDOMNode> aRow;
  nsAutoString nodeName;
  nsTArray<PRInt32> usedCount;
  PRUint32 nCurrCell(0), nCurrRow(0);
  nsCOMPtr<nsIDOMElement> currCellElt;
  nsAutoString rowSpanStr, colSpanStr;
  rowSpanStr.AssignASCII("rowspan");
  colSpanStr.AssignASCII("columnspan");
  PRInt32 nColSpan, nRowSpan;
  nsresult dontcare;
  msiMultiSpanCellInfo cellInfo;
  PRBool bMultiSpan(PR_FALSE);

  res = GetNextMatrixRow(matrix, prevRow, getter_AddRefs(nextRow));
  while (!bDone && NS_SUCCEEDED(res) && nextRow)
  {
    ++nCurrRow;
    nColSpan = 1;
    nRowSpan = 1;
    nCurrCell = 0;
    nextRow->GetLocalName(nodeName);
    if (nodeName.EqualsLiteral("mtr") || nodeName.EqualsLiteral("mlabeldtr"))
    {
      nsCOMPtr<nsIArray> grandchildren;
      dontcare = msiUtils::GetNonWhitespaceChildren(nextRow, grandchildren);
      PRUint32 numGrandkids(0);
      if (NS_SUCCEEDED(dontcare) && grandchildren)
        dontcare = grandchildren->GetLength(&numGrandkids);
      for (PRUint32 jx = 0; !bDone && jx < numGrandkids; ++jx)
      {
        while (nCurrCell < usedCount.Length() && (usedCount[nCurrCell] > 0) )
          ++nCurrCell;  //Look for an unused cell in this row
        grandchildren->QueryElementAt(jx, NS_GET_IID(nsIDOMElement), getter_AddRefs(currCellElt));
        nColSpan = 1;
        nRowSpan = 1;
        cellInfo.SetStartPos(nCurrRow, nCurrCell+1);  //Note that nCurrRow is 1-based, but nCurrCell is used as index into an array and is thus 0-based
        bMultiSpan = cellInfo.SetSpansFromCell(currCellElt);
        if (bMultiSpan)
        {
          nColSpan = cellInfo.ColSpan();
          nRowSpan = cellInfo.RowSpan();
          if (nRowSpan == 0)
            nRowSpan = 0x100000;  //presumably impossibly large value
          if (nColSpan == 0)
            nColSpan = usedCount.Length() - nCurrCell;  //This should suffice - if a row following this one is to have more cells than this, it had better come after the rowspan of a cell with colspan=0
          if (nColSpan < 1)
            nColSpan = 1;
          if (multiCellArray)
            multiCellArray->AppendElement(cellInfo);
        }
        
        if (pFindCell)
        {
          if (nRow && nCol)  //in this case we're asked to find the cell at this position
          {
            if ( (nCurrRow <= nRow) && (nCurrRow + nRowSpan > nRow) && (nCurrCell < nCol) && (nCurrCell + nColSpan >= nCol) )
            {
              currCellNode = do_QueryInterface(currCellElt);
              *pFindCell = currCellNode;
              NS_ADDREF(*pFindCell);
              bDone = PR_TRUE;
            }
          }
          else if (*pFindCell && (*pFindCell == currCellElt))  //in this case we're asked to find location of this cell
          {
            nRow = nCurrRow;
            nCol = nCurrCell+1;
            bDone = PR_TRUE;
          }
        }

        for (PRInt32 lx = 0; lx < nColSpan; ++lx)
        {
          if (nCurrCell < usedCount.Length())
            usedCount[nCurrCell++] = nRowSpan;
          else
          {
            usedCount.AppendElement(nRowSpan);
            ++nCurrCell;
          }
        }
      }
    }
    else if (nodeName.EqualsLiteral("mtd"))
    {
      while (nCurrCell < usedCount.Length() && (usedCount[nCurrCell] > 0) )
        ++nCurrCell;  //Look for an unused cell in this row
      currCellElt = do_QueryInterface(nextRow);
      if (currCellElt)
        bMultiSpan = cellInfo.SetSpansFromCell(currCellElt);
      cellInfo.SetStartPos(nCurrRow, nCurrCell+1);  //Note that nCurrRow is 1-based, but nCurrCell is used as index into an array and is thus 0-based
      if (bMultiSpan)
      {
        nColSpan = cellInfo.ColSpan();
        nRowSpan = cellInfo.RowSpan();
        if (nRowSpan == 0)
          nRowSpan = 0x100000;  //presumably impossibly large value
        if (nColSpan == 0)
          nColSpan = usedCount.Length() - nCurrCell;  //This should suffice - if a row following this one is to have more cells than this, it had better come after the rowspan of a cell with colspan=0
        if (nColSpan < 1)
          nColSpan = 1;
        if (multiCellArray)
          multiCellArray->AppendElement(cellInfo);
      }

      if (pFindCell)
      {
        if (nRow && nCol)  //in this case we're asked to find the cell at this position
        {
          if ( (nCurrRow <= nRow) && (nCurrRow + nRowSpan > nRow) && (nCurrCell < nCol) && (nCurrCell + nColSpan >= nCol) )
          {
            currCellNode = do_QueryInterface(currCellElt);
            *pFindCell = currCellNode;
            NS_ADDREF(*pFindCell);
            bDone = PR_TRUE;
          }
        }
        else if (*pFindCell && (*pFindCell == currCellElt))  //in this case we're asked to find location of this cell
        {
          nRow = nCurrRow;
          nCol = nCurrCell+1;
          bDone = PR_TRUE;
        }
      }

      for (PRUint32 kx = 0; kx < nRowSpan; ++kx)
      {
        if (nCurrCell < usedCount.Length())
          usedCount[nCurrCell++] = nRowSpan;
        else
        {
          usedCount.AppendElement(nColSpan);
          ++nCurrCell;
        }
      }
    }
    else  //This shouldn't happen as of MathML 2!
    {
      while (nCurrCell < usedCount.Length() && (usedCount[nCurrCell] > 0) )
        ++nCurrCell;  //Look for an unused cell in this row
      currCellElt = do_QueryInterface(nextRow);
      if (pFindCell)
      {
        if (nRow && nCol)  //in this case we're asked to find the cell at this position
        {
          if ( (nCurrRow <= nRow) && (nCurrRow + nRowSpan > nRow) && (nCurrCell < nCol) && (nCurrCell + nColSpan >= nCol) )
          {
            currCellNode = do_QueryInterface(currCellElt);
            *pFindCell = currCellNode;
            NS_ADDREF(*pFindCell);
            bDone = PR_TRUE;
          }
        }
        else if (*pFindCell && (*pFindCell == currCellElt))    //in this case we're asked to find location of this cell
        {
          nRow = nCurrRow;
          nCol = nCurrCell+1;
          bDone = PR_TRUE;
        }
      }
      if (nCurrCell < usedCount.Length())
        usedCount[nCurrCell++] = nRowSpan;
      else
      {
        usedCount.AppendElement(nColSpan);
        ++nCurrCell;
      }
    }

      //Now remove 1 from each "used count" before proceeding to the next row - should only have nonzero entries for continuation (rowspan > 1) cells
    bAllUsed = (usedCount.Length() > 0);
    PRInt32 minVal = 0x100000;
    for (PRUint32 lx = 0; lx < usedCount.Length(); ++lx)
    {
      usedCount[lx] = usedCount[lx] - 1;
      if (usedCount[lx] <= 0)
        bAllUsed = PR_FALSE;
      else if (usedCount[lx] < minVal)
        minVal = usedCount[lx];
    }
    if (bAllUsed) //This probably should never happen, but it might if we're using block matrices - means the whole next row(s) is already occupied
    {
//        ++nCurrRow; 
      nCurrRow += minVal;
      for (PRUint32 lx = 0; lx < usedCount.Length(); ++lx)
      {
        usedCount[lx] = usedCount[lx] - minVal;
      }
      bAllUsed = PR_FALSE;
    }
  
    prevRow = nextRow;
    res = GetNextMatrixRow(matrix, prevRow, getter_AddRefs(nextRow));
  };

  if (!pFindCell)
  {
    nRow = nCurrRow;
    nCol = usedCount.Length();
  }
  return res;
}




//nsresult msiEditingManager::GetMatrixInfo(nsIDOMElement *aMatrix, PRInt32& nRow, PRInt32& nCol, 
//                                          nsIDOMElement** pFindCell, nsTArray<msiMultiSpanCellInfo> *multiCellArray)
//{
//  NS_ENSURE_ARG_POINTER(aRowCount);
//  NS_ENSURE_ARG_POINTER(aColCount);
//  nsresult res;
////  *aRowCount = 0;
////  *aColCount = 0;
//  nsCOMPtr<nsIDOMElement> matrix;
//  // Get the selected matrix or the matrix enclosing the selection anchor
//  res = GetElementOrParentByTagName(NS_LITERAL_STRING("mtable"), aMatrix, getter_AddRefs(matrix));
//  if (NS_FAILED(res)) return res;
//  if (!matrix)         return NS_ERROR_FAILURE;
//
//  nsCOMPtr<nsIArray> children;
//  PRUint32 numKids(0);
//  PRBool bDone(PR_FALSE), bAllUsed(PR_FALSE);
//  nsresult res = msiUtils::GetNonWhitespaceChildren(aMatrix, children);
//  if (NS_SUCCEEDED(res) && children)
//    res = children->GetLength(&numKids);
//  if (NS_SUCCEEDED(res))
//  {
//    msiMultiSpanCellInfo cellInfo;
//    PRBool bMultiSpan(PR_FALSE);
//    nsCOMPtr<nsIDOMNode> aRow;
//    nsAutoString nodeName;
//    nsTArray<PRInt32> usedCount;
//    PRUint32 nCurrCell(0), nCurrRow(0);
//    nsCOMPtr<nsIDomElement> currCellElt;
//    nsAutoString rowSpanStr("rowspan");
//    nsAutoString colSpanStr("columnspan");
//    msiEditingAtoms::fence->ToString(fence);
//    PRInt32 nColSpan, nRowSpan;
//    nsresult dontcare;
//    for (PRUint32 ix = 0; (ix < numKids) && !bDone; ++ix)
//    {
//      ++nCurrRow;
//      nColSpan = 1;
//      nRowSpan = 1;
//      nCurrCell = 0;
//      children->QueryElementAt(ix, NS_GET_IID(nsIDOMNode), getter_AddRefs(aRow));
//      aRow->GetLocalName(nodeName);
//      if (nodeName->EqualsLiteral("mtr") || nodeName->EqualsLiteral("mlabeldtr"))
//      {
//        nsCOMPtr<nsIArray> grandchildren;
//        dontcare = msiUtils::GetNonWhitespaceChildren(aRow, grandchildren);
//        PRUint32 numGrandkids(0);
//        if (NS_SUCCEEDED(dontcare) && grandchildren)
//          dontcare = grandchildren->GetLength(&numGrandkids);
//        for (PRUint32 jx = 0; jx < numGrandkids; ++jx)
//        {
//          while (nCurrCell < usedCount.GetLength() && (usedCount[nCurrCell] > 0) )
//            ++nCurrCell;  //Look for an unused cell in this row
//          grandchilren->QueryElementAt(jx, NS_GET_IID(nsIDOMElement), getter_AddRefs(currCellElt));
//          nColSpan = 1;
//          nRowSpan = 1;
//          cellInfo.SetStartPos(nCurrRow, nCurrCell+1);  //Note that nCurrRow is 1-based, but nCurrCell is used as index into an array and is thus 0-based
//          bMultiSpan = cellInfo.SetSpansFromCell(currCellElt);
//          if (bMultiSpan)
//          {
//            nColSpan = cellInfo.ColSpan();
//            nRowSpan = cellInfo.RowSpan();
//            if (nRowSpan == 0)
//              nRowSpan = 0x100000;  //presumably impossibly large value
//            if (nColSpan == 0)
//              nColSpan = usedCount.GetLength() - nCurrColl;  //This should suffice - if a row following this one is to have more cells than this, it had better come after the rowspan of a cell with colspan=0
//            if (nColSpan < 1)
//              nColSpan = 1;
//            if (multiCellArray)
//              multiCellArray.AppendElement(cellInfo);
//          }
//          if (nRow && nCol && pFindCell)  //in this case we're asked to find the cell at this position
//          {
//            if ( (nCurrRow <= nRow) && (nCurrRow + nRowSpan > nRow) && (nCurrCell < nCol) && (nCurrCell + nColSpan >= nCol) )
//            {
//              *pFindCell = getter_AddRefs(currCellElt);
//              bDone = PR_TRUE;
//            }
//          }
//          else if (pFindCell && *pFindCell && (*pFindCell == currCellElt))  //in this case we're asked to find location of this cell
//          {
//            nRow = nCurrRow;
//            nCol = nCurrCell+1;
//            bDone = PR_TRUE;
//          }
//          for (var lx = 0; lx < nColSpan; ++lx)
//          {
//            if (nCurrCell < usedCount.GetLength())
//              usedCount[nCurrCell++] = nRowSpan;
//            else
//            {
//              usedCount.AppendElement(nRowSpan);
//              ++nCurrCell;
//            }
//          }
//        }
//      }
//      else if (nodeName->EqualsLiteral("mtd"))
//      {
//        while (nCurrCell < usedCount.GetLength() && (usedCount[nCurrCell] > 0) )
//          ++nCurrCell;  //Look for an unused cell in this row
//        currCellElt = do_QueryInterface(aRow);
//        if (currCellElt)
//          bMultiSpan = cellInfo.SetSpansFromCell(currCellElt);
//        cellInfo.SetStartPos(nCurrRow, nCurrCell+1);  //Note that nCurrRow is 1-based, but nCurrCell is used as index into an array and is thus 0-based
//        if (bMultiSpan)
//        {
//          nColSpan = cellInfo.ColSpan();
//          nRowSpan = cellInfo.RowSpan();
//          if (nRowSpan == 0)
//            nRowSpan = 0x100000;  //presumably impossibly large value
//          if (nColSpan == 0)
//            nColSpan = usedCount.GetLength() - nCurrColl;  //This should suffice - if a row following this one is to have more cells than this, it had better come after the rowspan of a cell with colspan=0
//          if (nColSpan < 1)
//            nColSpan = 1;
//          if (multiCellArray)
//            multiCellArray.AppendElement(cellInfo);
//        }
//        if (nRow && nCol)  //in this case we're asked to find the cell at this position
//        {
//          if ( (nCurrRow <= nRow) && (nCurrRow + nRowSpan > nRow) && (nCurrCell < nCol) && (nCurrCell + nColSpan >= nCol) )
//          {
//            *pFindCell = getter_AddRefs(currCellElt);
//            bDone = PR_TRUE;
//          }
//        }
//        else if (pFindCell && *pFindCell && (*pFindCell == currCellElt))  //in this case we're asked to find location of this cell
//        {
//          nRow = nCurrRow;
//          nCol = nCurrCell+1;
//          bDone = PR_TRUE;
//        }
//        for (PRUint32 kx = 0; kx < nRowSpan; ++kx)
//        {
//          if (nCurrCell < usedCount.GetLength())
//            usedCount[nCurrCell++] = nRowSpan;
//          else
//          {
//            usedCount.AppendElement(nColSpan);
//            ++nCurrCell;
//          }
//        }
//      }
//      else  //This shouldn't happen as of MathML 2!
//      {
//        while (nCurrCell < usedCount.GetLength() && (usedCount[nCurrCell] > 0) )
//          ++nCurrCell;  //Look for an unused cell in this row
//        currCellElt = do_QueryInterface(aRow);
//        if (nRow && nCol)  //in this case we're asked to find the cell at this position
//        {
//          if ( (nCurrRow <= nRow) && (nCurrRow + nRowSpan > nRow) && (nCurrCell < nCol) && (nCurrCell + nColSpan >= nCol) )
//          {
//            *pFindCell = getter_AddRefs(currCellElt);
//            bDone = PR_TRUE;
//          }
//        }
//        else if (pFindCell && *pFindCell && (*pFindCell == currCellElt))    //in this case we're asked to find location of this cell
//        {
//          nRow = nCurrRow;
//          nCol = nCurrCell+1;
//          bDone = PR_TRUE;
//        }
//        if (nCurrCell < usedCount.GetLength())
//          usedCount[nCurrCell++] = nRowSpan;
//        else
//        {
//          usedCount.AppendElement(nColSpan);
//          ++nCurrCell;
//        }
//      }
//      //Now remove 1 from each "used count" before proceeding to the next row - should only have nonzero entries for continuation (rowspan > 1) cells
//      bAllUsed = PR_TRUE;
//      do {
//        for (PRUint32 lx = 0; lx < usedCount.GetLength(); ++lx)
//        {
//          usedCount[kx] = usedCount[kx] - 1;
//          if (usedCount[kx] < 0)
//            bAllUsed = PR_FALSE;
//        }
//        if (bAllUsed)
//          ++nCurrRow;  //This probably should never happen, but it might if we're using block matrices - means the whole next row is already occupied
//      } while (bAllUsed);
//    }
//  }
//  nRow = nCurrRow;
//  nCol = usedCount.GetLength();
//  return res;
//}
//

//Utility class msiMultiSpanCellInfo (mostly defined in msiEditingManager.h)
msiMultiSpanCellInfo::msiMultiSpanCellInfo(const msiMultiSpanCellInfo& src)
    : m_startRow(src.m_startRow), m_startCol(src.m_startCol),
      m_rowSpan(src.m_rowSpan), m_colSpan(src.m_colSpan)
{
  m_pNode = src.m_pNode;
}

PRBool msiMultiSpanCellInfo::SetSpansFromCell(nsIDOMNode* aCell)
{
  PRBool retVal = PR_FALSE;
  nsAutoString rowSpanStr, colSpanStr;
  nsCOMPtr<nsIDOMElement> asElement = do_QueryInterface(aCell);
  rowSpanStr.AssignASCII("rowspan");
  colSpanStr.AssignASCII("columnspan");
  PRInt32 err;
  nsAutoString attrVal;
  m_rowSpan = 1;
  m_colSpan = 1;
  if (asElement)
  {
    nsresult dontcare = asElement->GetAttribute(rowSpanStr, attrVal);
    if (NS_SUCCEEDED(dontcare) && attrVal.Length())
      m_rowSpan = attrVal.ToInteger(&err, 10);
    dontcare = asElement->GetAttribute(colSpanStr, attrVal);
    if (NS_SUCCEEDED(dontcare) && attrVal.Length())
      m_colSpan = attrVal.ToInteger(&err, 10);
  }
  return ((m_rowSpan != 1) || (m_colSpan != 1));
}