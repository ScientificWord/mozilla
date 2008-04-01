/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is TransforMiiX XSLT processor code.
 *
 * The Initial Developer of the Original Code is
 * The MITRE Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Keith Visco <kvisco@ziplink.net> (Original Author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "txList.h"

  //----------------------------/
 //- Implementation of txList -/
//----------------------------/

/**
 * Default constructor for a txList;
**/

txList::txList() {
   firstItem  = 0;
   lastItem   = 0;
   itemCount  = 0;
} //-- txList;

/**
 * txList destructor, cleans up ListItems, but will not delete the Object
 * references
*/
txList::~txList() {
    clear();
} //-- ~txList

nsresult txList::insert(int index, void* objPtr)
{
    if (index >= itemCount) {
        return insertBefore(objPtr, 0);
    }
    // add inside the list
    ListItem* nextItem = firstItem;
    for (int i = 0; i < index; i++)
        nextItem = nextItem->nextItem;
    return insertBefore(objPtr, nextItem);
} //-- insert

nsresult txList::add(void* objPtr)
{
    return insertBefore(objPtr, 0);
} //-- add

/**
 * Returns the object located at the given index. This may
 * be slow or fast depending on the implementation.
 * Note:
 * Currently this list is implemented via a linked list, so
 * this method will be slow (unless the list only has a couple
 * members) as it will need traverse the links each time
 * @return the object located at the given index
**/
void* txList::get(int index) {

    if (index < 0 || index >= itemCount)
        return 0;

    int c = 0;
    ListItem* item = firstItem;
    while ((c != index) && item) {
        item = item->nextItem;
        ++c;
    }

    if (item)
        return item->objPtr;
    return 0;
} //-- get(int)

txList::ListItem* txList::getFirstItem() {
    return firstItem;
} //-- getFirstItem

txList::ListItem* txList::getLastItem() {
    return lastItem;
} //-- getLastItem

/**
 * Returns the number of items in this txList
**/
PRInt32 List::getLength() {
   return itemCount;
} //-- getLength


/**
 * Inserts the given Object pointer as the item just after refItem.
 * If refItem is a null pointer the Object will be inserted at the
 * beginning of the txList (ie, insert after nothing).
 * This method assumes refItem is a member of this list, and since this
 * is a private method, I feel that's a valid assumption
**/
nsresult txList::insertAfter(void* objPtr, ListItem* refItem)
{
    //-- if refItem == null insert at front
    if (!refItem)
        return insertBefore(objPtr, firstItem);
    return insertBefore(objPtr, refItem->nextItem);
} //-- insertAfter

/**
 * Inserts the given Object pointer as the item just before refItem.
 * If refItem is a null pointer the Object will be inserted at the
 * end of the txList (ie, insert before nothing).
 * This method assumes refItem is a member of this list, and since this
 * is a private method, I feel that's a valid assumption
**/
nsresult txList::insertBefore(void* objPtr, ListItem* refItem)
{
    ListItem* item = new ListItem;
    NS_ENSURE_TRUE(item, NS_ERROR_OUT_OF_MEMORY);

    item->objPtr = objPtr;
    item->nextItem = 0;
    item->prevItem = 0;

    //-- if refItem == null insert at end
    if (!refItem) {
        //-- add to back of list
        if (lastItem) {
            lastItem->nextItem = item;
            item->prevItem = lastItem;
        }
        lastItem = item;
        if (!firstItem)
            firstItem = item;
    }
    else {
        //-- insert before given item
        item->nextItem = refItem;
        item->prevItem = refItem->prevItem;
        refItem->prevItem = item;

        if (item->prevItem)
            item->prevItem->nextItem = item;
        else
            firstItem = item;
    }

    // increase the item count
    ++itemCount;
    
    return NS_OK;
} //-- insertBefore

void* txList::remove(void* objPtr) {
   ListItem* item = firstItem;
   while (item) {
      if (item->objPtr == objPtr) {
         remove(item);
         delete item;
         return objPtr;
      }
      item = item->nextItem;
   }
   // not in list
   return 0;
} //-- remove

txList::ListItem* txList::remove(ListItem* item) {

    if (!item)
        return item;

    //-- adjust the previous item's next pointer
    if (item->prevItem) {
        item->prevItem->nextItem = item->nextItem;
    }
    //-- adjust the next item's previous pointer
    if (item->nextItem) {
        item->nextItem->prevItem = item->prevItem;
    }

    //-- adjust first and last items
    if (item == firstItem)
        firstItem = item->nextItem;
    if (item == lastItem)
        lastItem = item->prevItem;

    //-- decrease Item count
    --itemCount;
    return item;
} //-- remove

void txList::clear()
{
    ListItem* item = firstItem;
    while (item) {
        ListItem* tItem = item;
        item = item->nextItem;
        delete tItem;
    }
    firstItem  = 0;
    lastItem   = 0;
    itemCount  = 0;
}

  //------------------------------------/
 //- Implementation of txListIterator -/
//------------------------------------/


/**
 * Creates a new txListIterator for the given txList
 * @param list, the txList to create an Iterator for
**/
txListIterator::txListIterator(txList* list) {
   this->list   = list;
   currentItem  = 0;
   atEndOfList  = MB_FALSE;
} //-- txListIterator

/**
 * Adds the Object pointer to the txList pointed to by this txListIterator.
 * The Object pointer is inserted as the next item in the txList
 * based on the current position within the txList
 * @param objPtr the Object pointer to add to the list
**/
nsresult txListIterator::addAfter(void* objPtr)
{
    if (currentItem || !atEndOfList)
        return list->insertAfter(objPtr, currentItem);
    return list->insertBefore(objPtr, 0);

} //-- addAfter

/**
 * Adds the Object pointer to the txList pointed to by this txListIterator.
 * The Object pointer is inserted as the previous item in the txList
 * based on the current position within the txList
 * @param objPtr the Object pointer to add to the list
**/
nsresult txListIterator::addBefore(void* objPtr)
{
    if (currentItem || atEndOfList)
        return list->insertBefore(objPtr, currentItem);
    return list->insertAfter(objPtr, 0);

} //-- addBefore

/**
 * Returns true if a successful call to the next() method can be made
 * @return MB_TRUE if a successful call to the next() method can be made,
 * otherwise MB_FALSE
**/
MBool txListIterator::hasNext() {
    MBool hasNext = MB_FALSE;
    if (currentItem)
        hasNext = (currentItem->nextItem != 0);
    else if (!atEndOfList)
        hasNext = (list->firstItem != 0);

    return hasNext;
} //-- hasNext

/**
 * Returns true if a successful call to the previous() method can be made
 * @return MB_TRUE if a successful call to the previous() method can be made,
 * otherwise MB_FALSE
**/
MBool txListIterator::hasPrevious() {
    MBool hasPrevious = MB_FALSE;
    if (currentItem)
        hasPrevious = (currentItem->prevItem != 0);
    else if (atEndOfList)
        hasPrevious = (list->lastItem != 0);

    return hasPrevious;
} //-- hasPrevious

/**
 * Returns the next Object pointer in the list
**/
void* txListIterator::next() {

    void* obj = 0;
    if (currentItem)
        currentItem = currentItem->nextItem;
    else if (!atEndOfList)
        currentItem = list->firstItem;

    if (currentItem)
        obj = currentItem->objPtr;
    else
        atEndOfList = MB_TRUE;

    return obj;
} //-- next

/**
 * Returns the previous Object in the list
**/
void* txListIterator::previous() {

    void* obj = 0;

    if (currentItem)
        currentItem = currentItem->prevItem;
    else if (atEndOfList)
        currentItem = list->lastItem;
    
    if (currentItem)
        obj = currentItem->objPtr;

    atEndOfList = MB_FALSE;

    return obj;
} //-- previous

/**
 * Returns the current Object
**/
void* txListIterator::current() {

    if (currentItem)
        return currentItem->objPtr;

    return 0;
} //-- current

/**
 * Moves the specified number of steps
**/
void* txListIterator::advance(int i) {

    void* obj = 0;

    if (i > 0) {
        if (!currentItem && !atEndOfList) {
            currentItem = list->firstItem;
            --i;
        }
        for (; currentItem && i > 0; i--)
            currentItem = currentItem->nextItem;
        
        atEndOfList = currentItem == 0;
    }
    else if (i < 0) {
        if (!currentItem && atEndOfList) {
            currentItem = list->lastItem;
            ++i;
        }
        for (; currentItem && i < 0; i++)
            currentItem = currentItem->prevItem;

        atEndOfList = MB_FALSE;
    }

    if (currentItem)
        obj = currentItem->objPtr;

    return obj;
} //-- advance

/**
 * Removes the Object last returned by the next() or previous() methods;
 * @return the removed Object pointer
**/
void* txListIterator::remove() {

    void* obj = 0;
    if (currentItem) {
        obj = currentItem->objPtr;
        txList::ListItem* item = currentItem;
        previous(); //-- make previous item the current item
        list->remove(item);
        delete item;
    }
    return obj;
} //-- remove

/**
 * Resets the current location within the txList to the beginning of the txList
**/
void txListIterator::reset() {
   atEndOfList = MB_FALSE;
   currentItem = 0;
} //-- reset

/**
 * Move the iterator to right after the last element
**/
void txListIterator::resetToEnd() {
   atEndOfList = MB_TRUE;
   currentItem = 0;
} //-- moveToEnd
