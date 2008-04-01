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

#ifndef nsPropertyTable_h_
#define nsPropertyTable_h_

#include "nscore.h"

class nsIAtom;
typedef unsigned long PtrBits;

typedef void
(*NSPropertyFunc)(void           *aObject,
                  nsIAtom        *aPropertyName,
                  void           *aPropertyValue,
                  void           *aData);

/**
 * Callback type for property destructors.  |aObject| is the object
 * the property is being removed for, |aPropertyName| is the property
 * being removed, |aPropertyValue| is the value of the property, and |aData|
 * is the opaque destructor data that was passed to SetProperty().
 **/
typedef NSPropertyFunc NSPropertyDtorFunc;
class nsINode;
class nsIFrame;

class nsPropertyOwner
{
public:
  nsPropertyOwner(const nsPropertyOwner& aOther) : mObject(aOther.mObject) {}

  // These are the types of objects that can own properties. No object should
  // inherit more then one of these classes.
  // To add support for more types just add to this list.
  nsPropertyOwner(const nsINode* aObject) : mObject(aObject) {}
  nsPropertyOwner(const nsIFrame* aObject) : mObject(aObject) {}

  operator const void*() { return mObject; }
  const void* get() { return mObject; }

private:
  const void* mObject;
};

// Categories of properties
// 0 is global.
#define DOM_USER_DATA         1
#define DOM_USER_DATA_HANDLER 2

class nsPropertyTable
{
 public:
  /**
   * Get the value of the property |aPropertyName| for node |aObject|.
   * |aResult|, if supplied, is filled in with a return status code.
   **/
  void* GetProperty(nsPropertyOwner aObject,
                    nsIAtom    *aPropertyName,
                    nsresult   *aResult = nsnull)
  {
    return GetPropertyInternal(aObject, 0, aPropertyName, PR_FALSE, aResult);
  }

  void* GetProperty(nsPropertyOwner aObject,
                    PRUint16    aCategory,
                    nsIAtom    *aPropertyName,
                    nsresult   *aResult = nsnull)
  {
    return GetPropertyInternal(aObject, aCategory, aPropertyName, PR_FALSE,
                               aResult);
  }

  /**
   * Set the value of the property |aPropertyName| in the global category to
   * |aPropertyValue| for node |aObject|.  |aDtor| is a destructor for the
   * property value to be called if the property is removed.  It can be null
   * if no destructor is required.  |aDtorData| is an optional pointer to an
   * opaque context to be passed to the property destructor.  Note that the
   * destructor is global for each property name regardless of node; it is an
   * error to set a given property with a different destructor than was used
   * before (this will return NS_ERROR_INVALID_ARG). If aOldValue is non-null
   * it will contain the old value after the function returns (the destructor
   * for the old value will not be run in that case). If |aTransfer| is PR_TRUE
   * the property will be transfered to the new table when the property table
   * for |aObject| changes (currently the tables for nodes are owned by their
   * ownerDocument, so if the ownerDocument for a node changes, its property
   * table changes too). If |aTransfer| is PR_FALSE the property will just be
   * deleted instead.
   */
  NS_HIDDEN_(nsresult) SetProperty(nsPropertyOwner     aObject,
                                   nsIAtom            *aPropertyName,
                                   void               *aPropertyValue,
                                   NSPropertyDtorFunc  aDtor,
                                   void               *aDtorData,
                                   PRBool              aTransfer = PR_FALSE,
                                   void              **aOldValue = nsnull)
  {
    return SetPropertyInternal(aObject, 0, aPropertyName, aPropertyValue,
                               aDtor, aDtorData, aTransfer, aOldValue);
  }

  /**
   * Set the value of the property |aPropertyName| in the category |aCategory|
   * to |aPropertyValue| for node |aObject|.  |aDtor| is a destructor for the
   * property value to be called if the property is removed.  It can be null
   * if no destructor is required.  |aDtorData| is an optional pointer to an
   * opaque context to be passed to the property destructor.  Note that the
   * destructor is global for  each property name regardless of node; it is an
   * error to set a given property with a different destructor than was used
   * before (this will return NS_ERROR_INVALID_ARG). If aOldValue is non-null
   * it will contain the old value after the function returns (the destructor
   * for the old value will not be run in that case). If aTransfer is PR_TRUE
   * the property will be transfered to the new table when the property table
   * for |aObject| changes (currently the tables for nodes are owned by their
   * ownerDocument, so if the ownerDocument for a node changes, its property
   * table changes too). If |aTransfer| is PR_FALSE the property will just be
   * deleted instead.
   */
  NS_HIDDEN_(nsresult) SetProperty(nsPropertyOwner     aObject,
                                   PRUint16            aCategory,
                                   nsIAtom            *aPropertyName,
                                   void               *aPropertyValue,
                                   NSPropertyDtorFunc  aDtor,
                                   void               *aDtorData,
                                   PRBool              aTransfer = PR_FALSE,
                                   void              **aOldValue = nsnull)
  {
    return SetPropertyInternal(aObject, aCategory, aPropertyName,
                               aPropertyValue, aDtor, aDtorData, aTransfer,
                               aOldValue);
  }

  /**
   * Delete the property |aPropertyName| in the global category for object
   * |aObject|. The property's destructor function will be called.
   */
  NS_HIDDEN_(nsresult) DeleteProperty(nsPropertyOwner aObject,
                                      nsIAtom    *aPropertyName)
  {
    return DeleteProperty(aObject, 0, aPropertyName);
  }

  /**
   * Delete the property |aPropertyName| in category |aCategory| for object
   * |aObject|. The property's destructor function will be called.
   */
  NS_HIDDEN_(nsresult) DeleteProperty(nsPropertyOwner aObject,
                                      PRUint16    aCategory,
                                      nsIAtom    *aPropertyName);

  /**
   * Unset the property |aPropertyName| in the global category for object
   * |aObject|, but do not call the property's destructor function.  The
   * property value is returned.
   */
  void* UnsetProperty(nsPropertyOwner aObject,
                      nsIAtom    *aPropertyName,
                      nsresult   *aStatus = nsnull)
  {
    return GetPropertyInternal(aObject, 0, aPropertyName, PR_TRUE, aStatus);
  }

  /**
   * Unset the property |aPropertyName| in category |aCategory| for object
   * |aObject|, but do not call the property's destructor function.  The
   * property value is returned.
   */
  void* UnsetProperty(nsPropertyOwner aObject,
                      PRUint16    aCategory,
                      nsIAtom    *aPropertyName,
                      nsresult   *aStatus = nsnull)
  {
    return GetPropertyInternal(aObject, aCategory, aPropertyName, PR_TRUE,
                               aStatus);
  }

  /**
   * Deletes all of the properties for object |aObject|, calling the
   * destructor function for each property.
   */
  NS_HIDDEN_(void) DeleteAllPropertiesFor(nsPropertyOwner aObject);

  /**
   * Deletes all of the properties in category |aCategory| for object |aObject|,
   * calling the destructor function for each property.
   */
  NS_HIDDEN_(void) DeleteAllPropertiesFor(nsPropertyOwner aObject,
                                          PRUint16 aCategory);

  /**
   * Transfers all properties for object |aObject| that were set with the
   * |aTransfer| argument as PR_TRUE to |aTable|. Deletes the other properties
   * for object |aObject|, calling the destructor function for each property.
   * If transfering a property fails, this deletes all the properties for
   * object |aObject|.
   */
  NS_HIDDEN_(nsresult)
    TransferOrDeleteAllPropertiesFor(nsPropertyOwner aObject,
                                     nsPropertyTable *aOtherTable);

  /**
   * Enumerate the properties in category |aCategory| for object |aObject|.
   * For every property |aCallback| will be called with as arguments |aObject|,
   * the property name, the property value and |aData|.
   */
  NS_HIDDEN_(void) Enumerate(nsPropertyOwner aObject, PRUint16 aCategory,
                             NSPropertyFunc aCallback, void *aData);

  /**
   * Deletes all of the properties for all objects in the property
   * table, calling the destructor function for each property.
   */
  NS_HIDDEN_(void) DeleteAllProperties();
  
  ~nsPropertyTable() {
    DeleteAllProperties();
  }

  /**
   * Function useable as destructor function for property data that is
   * XPCOM objects. The function will call NS_IF_RELASE on the value
   * to destroy it.
   */
  static void SupportsDtorFunc(void *aObject, nsIAtom *aPropertyName,
                               void *aPropertyValue, void *aData);

  class PropertyList;

 private:
  NS_HIDDEN_(void) DestroyPropertyList();
  NS_HIDDEN_(PropertyList*) GetPropertyListFor(PRUint16 aCategory,
                                               nsIAtom *aPropertyName) const;
  NS_HIDDEN_(void*) GetPropertyInternal(nsPropertyOwner aObject,
                                        PRUint16    aCategory,
                                        nsIAtom    *aPropertyName,
                                        PRBool      aRemove,
                                        nsresult   *aStatus);
  NS_HIDDEN_(nsresult) SetPropertyInternal(nsPropertyOwner     aObject,
                                           PRUint16            aCategory,
                                           nsIAtom            *aPropertyName,
                                           void               *aPropertyValue,
                                           NSPropertyDtorFunc  aDtor,
                                           void               *aDtorData,
                                           PRBool              aTransfer,
                                           void              **aOldValue);

  PropertyList *mPropertyList;
};
#endif
