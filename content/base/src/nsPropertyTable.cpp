/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim:cindent:ts=2:et:sw=2:
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK *****
 *
 * This Original Code has been modified by IBM Corporation. Modifications made by IBM 
 * described herein are Copyright (c) International Business Machines Corporation, 2000.
 * Modifications to Mozilla code or documentation identified per MPL Section 3.3
 *
 * Date             Modified by     Description of modification
 * 04/20/2000       IBM Corp.      OS/2 VisualAge build.
 */

/**
 * nsPropertyTable allows a set of arbitrary key/value pairs to be stored
 * for any number of nodes, in a global hashtable rather than on the nodes
 * themselves.  Nodes can be any type of object; the hashtable keys are
 * nsIAtom pointers, and the values are void pointers.
 */

#include "nsPropertyTable.h"
#include "pldhash.h"
#include "nsContentErrors.h"
#include "nsIAtom.h"

#define CATEGORY_BIT 0x00000001U
#define CATEGORY_MASK ~CATEGORY_BIT

struct PropertyListMapEntry : public PLDHashEntryHdr {
  const void  *key;
  void        *value;
};

//----------------------------------------------------------------------

class nsPropertyTable::PropertyList {
public:
  PropertyList(PRUint32           aCategory,
               nsIAtom*           aName,
               NSPropertyDtorFunc aDtorFunc,
               void*              aDtorData) NS_HIDDEN;
  ~PropertyList() NS_HIDDEN;

  // Removes the property associated with the given object, and destroys
  // the property value
  NS_HIDDEN_(PRBool) DeletePropertyFor(nsPropertyOwner aObject);

  // Destroy all remaining properties (without removing them)
  NS_HIDDEN_(void) Destroy();

  NS_HIDDEN_(PRBool) Equals(PRUint32 aCategory, nsIAtom *aPropertyName)
  {
    PRUint32 category = mBits & CATEGORY_BIT ? mBits & CATEGORY_MASK : 0;

    return mName == aPropertyName && category == aCategory;
  }
  NS_HIDDEN_(PRBool) IsInCategory(PRUint32 aCategory)
  {
    PRUint32 category = mBits & CATEGORY_BIT ? mBits & CATEGORY_MASK : 0;

    return category == aCategory;
  }
  NS_HIDDEN_(void*) GetDtorData()
  {
    return mBits & CATEGORY_BIT ? nsnull : NS_REINTERPRET_CAST(void*, mBits);
  }
  NS_HIDDEN_(void) SetDtorData(void *aDtorData)
  {
    NS_ASSERTION(!(mBits & CATEGORY_BIT),
                 "Can't set both DtorData and category!");
    mBits = NS_REINTERPRET_CAST(PtrBits, aDtorData);
  }

  nsCOMPtr<nsIAtom>  mName;           // property name
  PLDHashTable       mObjectValueMap; // map of object/value pairs
  NSPropertyDtorFunc mDtorFunc;       // property specific value dtor function
  PropertyList*      mNext;

private:
  PtrBits            mBits;           // pointer to pass to dtor or category
};

void
nsPropertyTable::DeleteAllProperties()
{
  while (mPropertyList) {
    PropertyList* tmp = mPropertyList;

    mPropertyList = mPropertyList->mNext;
    tmp->Destroy();
    delete tmp;
  }
}

void
nsPropertyTable::DeleteAllPropertiesFor(nsPropertyOwner aObject)
{
  for (PropertyList* prop = mPropertyList; prop; prop = prop->mNext) {
    prop->DeletePropertyFor(aObject);
  }
}

void
nsPropertyTable::Enumerate(nsPropertyOwner aObject, PRUint32 aCategory,
                           NSPropertyFunc aCallback, void *aData)
{
  PropertyList* prop;
  for (prop = mPropertyList; prop; prop = prop->mNext) {
    if (prop->IsInCategory(aCategory)) {
      PropertyListMapEntry *entry = NS_STATIC_CAST(PropertyListMapEntry*,
          PL_DHashTableOperate(&prop->mObjectValueMap, aObject,
                               PL_DHASH_LOOKUP));
      if (PL_DHASH_ENTRY_IS_BUSY(entry)) {
        aCallback(NS_CONST_CAST(void*, aObject.get()), prop->mName, entry->value,
                  aData);
      }
    }
  }
}

void*
nsPropertyTable::GetPropertyInternal(nsPropertyOwner aObject,
                                     PRUint32    aCategory,
                                     nsIAtom    *aPropertyName,
                                     PRBool      aRemove,
                                     nsresult   *aResult)
{
  NS_PRECONDITION(aPropertyName && aObject, "unexpected null param");
  nsresult rv = NS_PROPTABLE_PROP_NOT_THERE;
  void *propValue = nsnull;

  PropertyList* propertyList = GetPropertyListFor(aCategory, aPropertyName);
  if (propertyList) {
    PropertyListMapEntry *entry = NS_STATIC_CAST(PropertyListMapEntry*,
        PL_DHashTableOperate(&propertyList->mObjectValueMap, aObject,
                             PL_DHASH_LOOKUP));
    if (PL_DHASH_ENTRY_IS_BUSY(entry)) {
      propValue = entry->value;
      if (aRemove) {
        // don't call propertyList->mDtorFunc.  That's the caller's job now.
        PL_DHashTableRawRemove(&propertyList->mObjectValueMap, entry);
      }
      rv = NS_OK;
    }
  }

  if (aResult)
    *aResult = rv;

  return propValue;
}

nsresult
nsPropertyTable::SetPropertyInternal(nsPropertyOwner     aObject,
                                     PRUint32            aCategory,
                                     nsIAtom            *aPropertyName,
                                     void               *aPropertyValue,
                                     NSPropertyDtorFunc  aPropDtorFunc,
                                     void               *aPropDtorData,
                                     void              **aOldValue)
{
  NS_PRECONDITION(aPropertyName && aObject, "unexpected null param");

  PropertyList* propertyList = GetPropertyListFor(aCategory, aPropertyName);

  if (propertyList) {
    // Make sure the dtor function and data match
    if (aPropDtorFunc != propertyList->mDtorFunc ||
        aPropDtorData != propertyList->GetDtorData()) {
      return NS_ERROR_INVALID_ARG;
    }

  } else {
    propertyList = new PropertyList(aCategory, aPropertyName, aPropDtorFunc,
                                    aPropDtorData);
    if (!propertyList || !propertyList->mObjectValueMap.ops) {
      delete propertyList;
      return NS_ERROR_OUT_OF_MEMORY;
    }

    propertyList->mNext = mPropertyList;
    mPropertyList = propertyList;
  }

  // The current property value (if there is one) is replaced and the current
  // value is destroyed
  nsresult result = NS_OK;
  PropertyListMapEntry *entry = NS_STATIC_CAST(PropertyListMapEntry*,
    PL_DHashTableOperate(&propertyList->mObjectValueMap, aObject, PL_DHASH_ADD));
  if (!entry)
    return NS_ERROR_OUT_OF_MEMORY;
  // A NULL entry->key is the sign that the entry has just been allocated
  // for us.  If it's non-NULL then we have an existing entry.
  if (entry->key) {
    if (aOldValue)
      *aOldValue = entry->value;
    else if (propertyList->mDtorFunc)
      propertyList->mDtorFunc(NS_CONST_CAST(void*, entry->key), aPropertyName,
                              entry->value, propertyList->GetDtorData());
    result = NS_PROPTABLE_PROP_OVERWRITTEN;
  }
  else if (aOldValue) {
    *aOldValue = nsnull;
  }
  entry->key = aObject;
  entry->value = aPropertyValue;

  return result;
}

nsresult
nsPropertyTable::DeleteProperty(nsPropertyOwner aObject,
                                PRUint32    aCategory,
                                nsIAtom    *aPropertyName)
{
  NS_PRECONDITION(aPropertyName && aObject, "unexpected null param");

  PropertyList* propertyList = GetPropertyListFor(aCategory, aPropertyName);
  if (propertyList) {
    if (propertyList->DeletePropertyFor(aObject))
      return NS_OK;
  }

  return NS_PROPTABLE_PROP_NOT_THERE;
}

nsPropertyTable::PropertyList*
nsPropertyTable::GetPropertyListFor(PRUint32 aCategory,
                                    nsIAtom* aPropertyName) const
{
  PropertyList* result;

  for (result = mPropertyList; result; result = result->mNext) {
    if (result->Equals(aCategory, aPropertyName)) {
      break;
    }
  }

  return result;
}

//----------------------------------------------------------------------
    
nsPropertyTable::PropertyList::PropertyList(PRUint32            aCategory,
                                            nsIAtom            *aName,
                                            NSPropertyDtorFunc  aDtorFunc,
                                            void               *aDtorData)
  : mName(aName),
    mDtorFunc(aDtorFunc),
    mNext(nsnull),
    mBits(aCategory == 0 ? NS_REINTERPRET_CAST(PtrBits, aDtorData) :
                           aCategory | CATEGORY_BIT)
{
  NS_ASSERTION(!(aCategory & CATEGORY_BIT), "Don't set the reserved bit!");
  NS_ASSERTION(aCategory == 0 || !aDtorData,
               "Can't set both DtorData and category!");

  PL_DHashTableInit(&mObjectValueMap, PL_DHashGetStubOps(), this,
                    sizeof(PropertyListMapEntry), 16);
}

nsPropertyTable::PropertyList::~PropertyList()
{
  PL_DHashTableFinish(&mObjectValueMap);
}


PR_STATIC_CALLBACK(PLDHashOperator)
DestroyPropertyEnumerator(PLDHashTable *table, PLDHashEntryHdr *hdr,
                          PRUint32 number, void *arg)
{
  nsPropertyTable::PropertyList *propList =
      NS_STATIC_CAST(nsPropertyTable::PropertyList*, table->data);
  PropertyListMapEntry* entry = NS_STATIC_CAST(PropertyListMapEntry*, hdr);

  propList->mDtorFunc(NS_CONST_CAST(void*, entry->key), propList->mName,
                      entry->value, propList->GetDtorData());
  return PL_DHASH_NEXT;
}

void
nsPropertyTable::PropertyList::Destroy()
{
  // Enumerate any remaining object/value pairs and destroy the value object
  if (mDtorFunc)
    PL_DHashTableEnumerate(&mObjectValueMap, DestroyPropertyEnumerator,
                           nsnull);
}

PRBool
nsPropertyTable::PropertyList::DeletePropertyFor(nsPropertyOwner aObject)
{
  PropertyListMapEntry *entry = NS_STATIC_CAST(PropertyListMapEntry*,
      PL_DHashTableOperate(&mObjectValueMap, aObject, PL_DHASH_LOOKUP));
  if (!PL_DHASH_ENTRY_IS_BUSY(entry))
    return PR_FALSE;

  if (mDtorFunc)
    mDtorFunc(NS_CONST_CAST(void*, aObject.get()), mName,
              entry->value, GetDtorData());

  PL_DHashTableRawRemove(&mObjectValueMap, entry);

  return PR_TRUE;
}

/* static */
void
nsPropertyTable::SupportsDtorFunc(void *aObject, nsIAtom *aPropertyName,
                                  void *aPropertyValue, void *aData)
{
  nsISupports *propertyValue = NS_STATIC_CAST(nsISupports*, aPropertyValue);
  NS_IF_RELEASE(propertyValue);
}
