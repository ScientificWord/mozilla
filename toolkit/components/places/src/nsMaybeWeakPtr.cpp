/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Places code.
 *
 * The Initial Developer of the Original Code is
 * Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brian Ryner <bryner@brianryner.com> (original author)
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

#include "nsMaybeWeakPtr.h"

void*
nsMaybeWeakPtr_base::GetValueAs(const nsIID &iid) const
{
  nsresult rv;
  void *ref;
  if (mPtr) {
    rv = mPtr->QueryInterface(iid, &ref);
    if (NS_SUCCEEDED(rv)) {
      return ref;
    }
  }

  nsCOMPtr<nsIWeakReference> weakRef = do_QueryInterface(mPtr);
  if (weakRef) {
    rv = weakRef->QueryReferent(iid, &ref);
    if (NS_FAILED(rv)) {
      ref = nsnull;
    }
  }
  return ref;
}

/* static */ nsresult
nsMaybeWeakPtrArray_base::AppendWeakElementBase(nsTArray_base *aArray,
                                                nsISupports *aElement,
                                                PRBool aOwnsWeak)
{
  nsCOMPtr<nsISupports> ref;
  if (aOwnsWeak) {
    nsCOMPtr<nsIWeakReference> weakRef;
    weakRef = do_GetWeakReference(aElement);
    reinterpret_cast<nsCOMPtr<nsISupports>*>(&weakRef)->swap(ref);
  } else {
    ref = aElement;
  }

  isupports_type *array = static_cast<isupports_type*>(aArray);
  if (array->IndexOf(ref) != isupports_type::NoIndex) {
    return NS_ERROR_INVALID_ARG; // already present
  }
  if (!array->AppendElement(ref)) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  return NS_OK;
}

/* static */ nsresult
nsMaybeWeakPtrArray_base::RemoveWeakElementBase(nsTArray_base *aArray,
                                                nsISupports *aElement)
{
  isupports_type *array = static_cast<isupports_type*>(aArray);
  PRUint32 index = array->IndexOf(aElement);
  if (index != isupports_type::NoIndex) {
    array->RemoveElementAt(index);
    return NS_OK;
  }

  // Don't use do_GetWeakReference; it should only be called if we know
  // the object supports weak references.
  nsCOMPtr<nsISupportsWeakReference> supWeakRef = do_QueryInterface(aElement);
  NS_ENSURE_TRUE(supWeakRef, NS_ERROR_INVALID_ARG);

  nsCOMPtr<nsIWeakReference> weakRef;
  nsresult rv = supWeakRef->GetWeakReference(getter_AddRefs(weakRef));
  NS_ENSURE_SUCCESS(rv, rv);

  index = array->IndexOf(weakRef);
  if (index == isupports_type::NoIndex) {
    return NS_ERROR_INVALID_ARG;
  }

  array->RemoveElementAt(index);
  return NS_OK;
}
