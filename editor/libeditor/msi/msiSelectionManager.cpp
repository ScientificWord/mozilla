#include "msiSelectionManager.h"
#include "nsVoidArray.h"
#include "nsISelection.h"
#include "nsIDOMNode.h"
#include "nsIDOMText.h"
#include "nsIDOMElement.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMAttr.h"
#include "msiEditor.h"
#include "msiIMathMLEditingBC.h"

PRUint32 msiSelectionManager::m_nodeID = 1;

msiSelectionManager::msiSelectionManager(nsISelection * selection, msiEditor * msiEditor)
:  nsSelectionState(), m_msiEditor(msiEditor)
{
  NS_NAMED_LITERAL_STRING(m_attributeName, "msiSelectionManagerID");
  NS_NAMED_LITERAL_STRING(m_surrogateAttributeName, "msiSelectionManagerSurrogateID");
  nsVoidArray  rangeData;
  NormalizeSelection(selection, rangeData);
  InitalizeRangeStore(rangeData);
  Delete(rangeData);
}


nsresult 
msiSelectionManager::NormalizeSelection(nsISelection * selection, nsVoidArray & rangeData)
{
  if (!selection) 
    return NS_ERROR_NULL_POINTER;
  PRInt32 i(0), rangeCount(0);
  selection->GetRangeCount(&rangeCount);
  nsRangeStore *item = nsnull;
  nsresult res(NS_OK);
  //TODO -- Need to write code to process the ranges, removing nested ranges and merging intersecting 
  for (i=0; i<rangeCount && NS_SUCCEEDED(res); i++)
  {
    item = new nsRangeStore;
    if (!item) 
      res = NS_ERROR_FAILURE;  
    nsCOMPtr<nsIDOMRange> range;
    res = selection->GetRangeAt(i, getter_AddRefs(range));
    if (NS_SUCCEEDED(res) && range)
    { 
      item->StoreRange(range);
      SetMathmlSurrogates(item); 
      rangeData.AppendElement(item);
    }
    else
      res = NS_ERROR_FAILURE;  
  }
  return res;
}

nsresult 
msiSelectionManager::InitalizeRangeStore(const nsVoidArray & rangeData)
{
  PRInt32 i, rangeCount(rangeData.Count());
  
  // now store the normalized ranges and set the msiSelManager IDs on the nodes. 
  nsRangeStore * rangeStoreItem = nsnull;
  nsRangeStore * rangeDataItem = nsnull;
  for (i=0; i<rangeCount; i++)
  {
    rangeDataItem = (nsRangeStore*)rangeData.ElementAt(i);
    rangeStoreItem = nsnull;
    if (!rangeDataItem)
      return NS_ERROR_UNEXPECTED;
    SetNodeIDs(rangeDataItem);
    rangeStoreItem = new nsRangeStore(rangeDataItem);
    if (!rangeStoreItem)
      return NS_ERROR_UNEXPECTED;
    mArray.AppendElement(rangeStoreItem);
  }
  return NS_OK;
}

nsresult 
msiSelectionManager::SetNodeIDs(nsRangeStore * rangeItem)
{
  if (!rangeItem)
    return NS_ERROR_FAILURE;
  for (int dummy =0; dummy< 2; dummy++)
  {
    nsCOMPtr<nsIDOMNode> surrogate, node;
    nsCOMPtr<nsIDOMElement> element;
    PRBool useSurrogate(PR_FALSE);
    if (dummy == 0)
    {
      surrogate = rangeItem->startSurrogate;
      node = rangeItem->startNode;
    }
    else
    {
      surrogate = rangeItem->endSurrogate;
      node = rangeItem->endNode;  
    }
    if (surrogate)
    {
      useSurrogate = PR_TRUE;
      element = do_QueryInterface(surrogate);
    }
    else    
      element = do_QueryInterface(node);
    if (element)
    {
      nsAutoString value;
      value.AppendInt(m_nodeID);
      if (useSurrogate)
        element->SetAttribute(m_surrogateAttributeName, value);
      else
        element->SetAttribute(m_attributeName, value);
      m_nodeID +=1;
    }
    else
    {
  #ifdef DEBUG
        nsCOMPtr<nsIDOMText> textNode(do_QueryInterface(node));
        if (textNode && m_msiEditor)
        {
          if (m_msiEditor->NodeInMath(node))
            NS_ASSERTION(PR_FALSE, "YUCK\n.");
          //else
          //ljh -- since mathml editing won't be handling is textnode mozilla's nsRangeUpdater code 
          // should handle updates without the need for the id attribute. -- I hope!
        }
  #endif
    }
  }
  return NS_OK;
}

nsresult 
msiSelectionManager::ClearNodeIDs(nsRangeStore * rangeItem)
{
  if (!rangeItem)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIDOMElement> element;
  if (rangeItem->startSurrogate)
  {   
    element = do_QueryInterface(rangeItem->startSurrogate);
    if (element)
      element->RemoveAttribute(m_surrogateAttributeName);
    element = nsnull;
  }
  if (rangeItem->startNode)
  {   
    element = do_QueryInterface(rangeItem->startNode);
    if (element)
      element->RemoveAttribute(m_attributeName);
    element = nsnull;
  }
  if (rangeItem->endSurrogate)
  {   
    element = do_QueryInterface(rangeItem->endSurrogate);
    if (element)
      element->RemoveAttribute(m_surrogateAttributeName);
    element = nsnull;
  }
  if (rangeItem->endNode)
  {   
    element = do_QueryInterface(rangeItem->endNode);
    if (element)
      element->RemoveAttribute(m_attributeName);
    element = nsnull;
  }
  return NS_OK;
}

PRUint32 msiSelectionManager::RangeCount()
{
  PRUint32 count(0);
  PRInt32 tmp = mArray.Count();
  if (tmp > 0)
    count = tmp;
  return count;  
}

nsresult
msiSelectionManager::GetRange(PRUint32 index, nsCOMPtr<nsIDOMRange> & range)
{
  range = nsnull;
  nsresult res(NS_ERROR_FAILURE);
  PRInt32 count = mArray.Count();
  if (count > 0 && index < NS_STATIC_CAST(PRUint32, count))
  {
    nsRangeStore * item = (nsRangeStore*)mArray.ElementAt(index);
    if (!item)
      return NS_ERROR_FAILURE;
    res = item->GetRange(range);
  }
  return res;
}

nsresult msiSelectionManager::IsRangeCollapsed(PRUint32 index, PRBool &collapsed)
{
  nsCOMPtr<nsIDOMRange> range;
  nsresult res = GetRange(index, range);
  if (NS_SUCCEEDED(res) && range)
    res = range->GetCollapsed(&collapsed);
  else
    res = NS_ERROR_FAILURE;
  return  res;
}


nsRangeStore * msiSelectionManager::GetRangeStoreItem(PRUint32 index)
{
  PRInt32 count = mArray.Count();
  if (count > 0 && index < NS_STATIC_CAST(PRUint32, count))
    return (nsRangeStore*)(mArray.ElementAt(index));
  else
    return nsnull;  
}

void msiSelectionManager::Cleanup()
{
  PRInt32 count = mArray.Count();
  for (PRInt32 index= 0; index < count; index++)
  {
    nsRangeStore * item = (nsRangeStore*)mArray.ElementAt(index);
    if (item)
    {
      ClearNodeIDs(item);
    }
  }
}

void msiSelectionManager::SetMathmlSurrogates(nsRangeStore * rangeItem)
{
  if (!m_msiEditor || !rangeItem)
    return;
  if (rangeItem->startNode)
  {
    nsCOMPtr<nsIDOMText> text(do_QueryInterface(rangeItem->startNode));
    if (text && m_msiEditor->NodeInMath(rangeItem->startNode))
    {
      nsCOMPtr<nsIDOMNode> parent;
      PRUint32 index(msiIMathMLEditingBC::INVALID);
      GetParentAndIndexOfChildInParent(rangeItem->startNode, parent, index);
      if (parent && IS_VALID_NODE_OFFSET(index))
      {
        rangeItem->startSurrogate = parent;
        rangeItem->startSurrogateOffset = index;
      }
    }
  }  
  if (rangeItem->endNode)
  {
    nsCOMPtr<nsIDOMText> text(do_QueryInterface(rangeItem->endNode));
    if (text && m_msiEditor->NodeInMath(rangeItem->endNode))
    {
      nsCOMPtr<nsIDOMNode> parent;
      PRUint32 index(msiIMathMLEditingBC::INVALID);
      GetParentAndIndexOfChildInParent(rangeItem->endNode, parent, index);
      if (parent && IS_VALID_NODE_OFFSET(index))
      {
        rangeItem->endSurrogate = parent;
        rangeItem->endSurrogateOffset = index;
      }
    }
  }  
  return;
} 

nsresult
msiSelectionManager::GetParentAndIndexOfChildInParent(nsIDOMNode * child, 
                                                      nsCOMPtr<nsIDOMNode> & parent, 
                                                      PRUint32 &index)
{
  nsresult res(NS_ERROR_FAILURE);
  if (child)
  {
    child->GetParentNode(getter_AddRefs(parent));
    if (parent)
    {
      nsCOMPtr<nsIDOMNodeList> children;
      parent->GetChildNodes(getter_AddRefs(children));
      if (children) 
      {
        PRUint32 i, count;
        children->GetLength(&count);
        for (i=0; i < count; i++) 
        {
          nsCOMPtr<nsIDOMNode> currChild;
          children->Item(i, getter_AddRefs(currChild));
          if (currChild == child)
          {
            index = i;
            res = NS_OK;
            break;
          }
        }
      }  
    }
  }
  return res;
}

nsresult
msiSelectionManager::Delete(nsVoidArray & rangeData)
{
  // free any items in the array
  nsRangeStore *item;
  PRInt32 count = rangeData.Count();
  for (PRInt32 i = count-1; i >= 0; --i)
  {
    item = (nsRangeStore*)rangeData.ElementAt(i);
    delete item;
  }
  rangeData.Clear();
  return NS_OK;
}
