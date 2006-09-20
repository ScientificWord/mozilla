// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "nsCOMPtr.h"
#include "nsIDOMNode.h"
#include "msiBigOpInfoImp.h"
#include "msiEditingManager.h"
#include "nsString.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMElement.h"
#include "msiEditingAtoms.h"
#include "msiIMathMLEditingBC.h"
#include "nsIDOMText.h"
#include "nsIDOMCharacterData.h"

msiBigOpInfoImp::msiBigOpInfoImp(nsIDOMNode* mathmlNode, PRUint32 offset) :
m_rawMathmlNode(mathmlNode), m_rawOffset(offset), m_flags(BOFlag_none),
m_scriptType(msiIMathMLEditingBC::MATHML_UNKNOWN)
{
  nsAutoString localName;
  nsresult res(NS_OK);
  if (mathmlNode)
    mathmlNode->GetLocalName(localName);
  if (msiEditingManager::GetBigOpNodes(mathmlNode, localName, m_mo, m_mstyle, m_script, m_scriptType) && m_mo)
  {
    //TODO --- should we only allow the fixed set of symbol from the dialog? 
    //TODO --- should big ops have form ="prefix" set? 
    nsCOMPtr<nsIDOMElement> moElement(do_QueryInterface(m_mo));
    if (moElement)
    {
      nsAutoString value, atomString;
      msiEditingAtoms::msiLimitPlacement->ToString(atomString);
      moElement->GetAttribute(atomString,  value);
      if (msiEditingAtoms::msiLimitsAtRight->Equals(value))
        m_flags |= BOFlag_limits_right;
      else
      {
        if (msiEditingAtoms::msiLimitsAboveBelow->Equals(value))
          m_flags |= BOFlag_limits_above_below;
        else
        {
          value.Truncate(0);
          msiEditingAtoms::movablelimits->ToString(atomString);
          moElement->GetAttribute(atomString,  value);
          if (msiEditingAtoms::msifalse->Equals(value))
            m_flags |= BOFlag_limits_above_below;
        }
      }  
    }
    if (m_mstyle)
    {
      nsCOMPtr<nsIDOMElement> styleElement(do_QueryInterface(m_mstyle));
      if (styleElement)
      {
        nsAutoString value, displaystyle;
        msiEditingAtoms::displaystyle->ToString(displaystyle);
        res = styleElement->GetAttribute(displaystyle,  value);
        if (NS_SUCCEEDED(res) && (msiEditingAtoms::msitrue->Equals(value) || 
                                  msiEditingAtoms::msifalse->Equals(value)))
        {
          if (msiEditingAtoms::msitrue->Equals(value))
            m_flags |= BOFlag_displaystyle; 
          else
            m_flags |= BOFlag_textstyle; 
          nsCOMPtr<nsIDOMNamedNodeMap> attrMap;
          m_mstyle->GetAttributes(getter_AddRefs(attrMap));
          PRUint32 numAttr(0);
          if (attrMap)
            attrMap->GetLength(&numAttr);
          if (numAttr > 1)
          {
            //TODO -- do we care about other attributes which may be set -- which may conflict?
          }
        }
      }
    }
    if (m_script && (m_script != mathmlNode))
        m_offset = offset == 0 ? 0 : 1;
    else
      m_offset = offset;
  }
}

msiBigOpInfoImp::~msiBigOpInfoImp()
{
}

NS_IMETHODIMP msiBigOpInfoImp::GetRawMathmlNode(nsIDOMNode * *rawMathmlNode)
{
  *rawMathmlNode = m_rawMathmlNode;
  NS_ADDREF(*rawMathmlNode); 
  return NS_OK; 
}

NS_IMETHODIMP msiBigOpInfoImp::GetRawOffset(PRUint32 *rawOffset)
{
   *rawOffset = m_rawOffset;
   return NS_OK;
}

NS_IMETHODIMP msiBigOpInfoImp::GetMo(nsIDOMNode * *mo)
{
  if (m_mo)
  {
    *mo = m_mo;
    NS_ADDREF(*mo); 
    return NS_OK;
  }
  else
   return NS_ERROR_NULL_POINTER;  
}

NS_IMETHODIMP msiBigOpInfoImp::GetMstyle(nsIDOMNode * *mstyle)
{
  if (m_mstyle)
  {
    *mstyle = m_mstyle;
    NS_ADDREF(*mstyle); 
  }
  else
    *mstyle = nsnull;
  return NS_OK;
}

NS_IMETHODIMP msiBigOpInfoImp::GetScript(nsIDOMNode * *script)
{
  if (m_script)
  {
    *script = m_script;
    NS_ADDREF(*script); 
  }
  else
    *script = nsnull;
  return NS_OK;
}

NS_IMETHODIMP msiBigOpInfoImp::GetMathmlNode(nsIDOMNode * *bigOp)
{
  nsresult res(NS_ERROR_NULL_POINTER);
  *bigOp = nsnull;
  if (m_script)
    *bigOp = m_script;
  else if (m_mstyle) 
    *bigOp = m_mstyle;
  else if (m_mo) 
    *bigOp = m_mo;
  if (*bigOp)
  {
    NS_ADDREF(*bigOp);
    res = NS_OK;
  }
   return res;
}

NS_IMETHODIMP msiBigOpInfoImp::GetOffset(PRUint32 *offset)
{
  *offset = m_offset;
  return NS_OK;
}

NS_IMETHODIMP msiBigOpInfoImp::GetFlags(PRUint32 *flags) 
{
  *flags = m_flags;
  return NS_OK;
}

NS_IMETHODIMP msiBigOpInfoImp::GetScriptType(PRUint32 *scriptType) 
{
  *scriptType = m_scriptType;
  return NS_OK;
}

NS_IMETHODIMP msiBigOpInfoImp::GetUseSubSupLimits(PRBool *useSubSupLimits)
{
  *useSubSupLimits = (m_scriptType == msiIMathMLEditingBC::MATHML_MSUB || 
                      m_scriptType == msiIMathMLEditingBC::MATHML_MSUP || 
                      m_scriptType == msiIMathMLEditingBC::MATHML_MSUBSUP);
  if (!(*useSubSupLimits))
    *useSubSupLimits = (m_flags & BOFlag_limits_right);
  if (!(*useSubSupLimits) && !(m_flags & BOFlag_limits_above_below) && m_mo)
  {
    nsCOMPtr<nsIDOMNode> child;
    m_mo->GetFirstChild(getter_AddRefs(child));
    if (child)
    {
      nsCOMPtr<nsIDOMText> text(do_QueryInterface(child));
      if (text)
      {
         nsAutoString moData;
         nsAutoString integral(0x222B);
         nsCOMPtr<nsIDOMCharacterData> characterdata(do_QueryInterface(text));
         if (characterdata)
            characterdata->GetData(moData);
         *useSubSupLimits = moData.Equals(integral);
      }
    }
    
  }
  return NS_OK;
}


NS_IMPL_ISUPPORTS1(msiBigOpInfoImp, msiIBigOpInfo)

nsresult
MSI_NewBigOpInfoImp(nsIDOMNode* rawMathmlNode, PRUint32 rawOffset, msiIBigOpInfo** aResult)
{
  msiBigOpInfoImp * boi = new msiBigOpInfoImp(rawMathmlNode, rawOffset);
  if (!boi) 
    return NS_ERROR_OUT_OF_MEMORY;
  *aResult = NS_STATIC_CAST(msiIBigOpInfo*, boi);
  NS_ADDREF(*aResult);
  return NS_OK;
}

