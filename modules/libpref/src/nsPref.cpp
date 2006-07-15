/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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
 * ***** END LICENSE BLOCK *****
 *
 * This Original Code has been modified by IBM Corporation.
 * Modifications made by IBM described herein are
 * Copyright (c) International Business Machines
 * Corporation, 2000
 *
 * Modifications to Mozilla code or documentation
 * identified per MPL Section 3.3
 *
 * Date         Modified by     Description of modification
 * 03/27/2000   IBM Corp.       Added PR_CALLBACK for Optlink
 *                               use in OS2
 */

#include "nsIPref.h"
#include "nsIPrefBranch.h"
#include "nsIPrefBranchInternal.h"
#include "nsIFactory.h"
#include "nsIComponentManager.h"
#include "nsIObserver.h"
#include "nsCOMPtr.h"
#include "nsMemory.h"
#include "prefapi.h"

#ifndef MOZ_NO_XPCOM_OBSOLETE
#include "nsIFileSpec.h"
#endif

#include "nsString.h"
#include "nsILocalFile.h"
#include "nsIPrefBranch.h"
#include "nsIPrefLocalizedString.h"
#include "nsISecurityPref.h"
#include "nsIPrefService.h"
#include "nsISupportsPrimitives.h"
#include "nsWeakReference.h"

#include "nsIModule.h"
#include "nsIGenericFactory.h"

#include "plstr.h"
#include "prmem.h"
#include "prprf.h"


class nsPref : public nsIPref,
               public nsIPrefService,
               public nsIObserver,
               public nsIPrefBranchInternal,
               public nsISecurityPref,
               public nsSupportsWeakReference
{
public:
  static nsPref *GetInstance();

  /* Use xpidl-generated macro to declare everything required by nsIPref */
  NS_DECL_ISUPPORTS
  NS_DECL_NSIPREFBRANCH
  NS_DECL_NSIPREFBRANCH2
  NS_DECL_NSISECURITYPREF
  NS_DECL_NSIOBSERVER
  NS_FORWARD_NSIPREFSERVICE(mPrefService->)

  NS_IMETHOD CopyCharPref(const char *pref, char ** return_buf);

  NS_IMETHODIMP GetDefaultBoolPref(const char *pref, PRBool *_retval)
                  { return mDefaultBranch->GetBoolPref(pref, _retval); }
  NS_IMETHODIMP CopyDefaultCharPref(const char *pref, char **_retval)
                  { return mDefaultBranch->GetCharPref(pref, _retval); }
  NS_IMETHODIMP GetDefaultIntPref(const char *pref, PRInt32 *_retval)
                  { return mDefaultBranch->GetIntPref(pref, _retval); }
  NS_IMETHODIMP SetDefaultBoolPref(const char *pref, PRBool value)
                  { return mDefaultBranch->SetBoolPref(pref, value); }
  NS_IMETHODIMP SetDefaultCharPref(const char *pref, const char *value)
                  { return mDefaultBranch->SetCharPref(pref, value); }
  NS_IMETHODIMP SetDefaultIntPref(const char *pref, PRInt32 value)
                  { return mDefaultBranch->SetIntPref(pref, value); }

  NS_IMETHOD CopyUnicharPref(const char *pref, PRUnichar **_retval);
  NS_IMETHOD CopyDefaultUnicharPref(const char *pref, PRUnichar **_retval);
  NS_IMETHOD SetUnicharPref(const char *pref, const PRUnichar *value);
  NS_IMETHOD SetDefaultUnicharPref(const char *pref, const PRUnichar *value);
  NS_IMETHOD GetLocalizedUnicharPref(const char *pref, PRUnichar **_retval);
  NS_IMETHOD GetDefaultLocalizedUnicharPref(const char *pref, PRUnichar **_retval);

  NS_IMETHOD GetFilePref(const char *pref, nsIFileSpec **_retval);
  NS_IMETHOD SetFilePref(const char *pref, nsIFileSpec *value, PRBool setDefault);
  NS_IMETHOD GetFileXPref(const char *pref, nsILocalFile **_retval);
  NS_IMETHOD SetFileXPref(const char *pref, nsILocalFile *value);

  NS_IMETHOD RegisterCallback(const char *domain, PrefChangedFunc callback, void * closure);
  NS_IMETHOD UnregisterCallback(const char *domain, PrefChangedFunc callback, void * closure);
  NS_IMETHOD EnumerateChildren(const char *parent, PrefEnumerationFunc callback, void * data); 

protected:
  nsPref();
  virtual ~nsPref();

  static nsPref *gInstance;

private:
  nsCOMPtr<nsIPrefService> mPrefService;
  nsCOMPtr<nsIPrefBranch>  mDefaultBranch;
};

nsPref* nsPref::gInstance = NULL;
static PRInt32 g_InstanceCount = 0;


NS_IMPL_THREADSAFE_ISUPPORTS8(nsPref,
                              nsIPref,
                              nsIPrefService,
                              nsIObserver,
                              nsIPrefBranch,
                              nsIPrefBranch2,
                              nsIPrefBranchInternal,
                              nsISecurityPref,
                              nsISupportsWeakReference)

//----------------------------------------------------------------------------------------
nsPref::nsPref()
//----------------------------------------------------------------------------------------
{
  PR_AtomicIncrement(&g_InstanceCount);

  mPrefService = do_GetService(NS_PREFSERVICE_CONTRACTID);
  NS_ASSERTION(mPrefService, "Preference Service failed to start up!!");

  if (mPrefService)
    mPrefService->GetDefaultBranch("", getter_AddRefs(mDefaultBranch));
}

//----------------------------------------------------------------------------------------
nsPref::~nsPref()
//----------------------------------------------------------------------------------------
{
  PR_AtomicDecrement(&g_InstanceCount);
  gInstance = NULL;
}

/*
 * Implementations to pass branch calls through the PrefService.
 */

NS_IMETHODIMP nsPref::GetRoot(char * *aRoot)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetRoot(aRoot);
  return rv;
}

NS_IMETHODIMP nsPref::GetPrefType(const char *aPrefName, PRInt32 *_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetPrefType(aPrefName, _retval);
  return rv;
}

NS_IMETHODIMP nsPref::CopyCharPref(const char *pref, char ** return_buf)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetCharPref(pref, return_buf);
  return rv;
}

NS_IMETHODIMP nsPref::GetBoolPref(const char *aPrefName, PRBool *_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetBoolPref(aPrefName, _retval);
  return rv;
}

NS_IMETHODIMP nsPref::SetBoolPref(const char *aPrefName, PRInt32 aValue)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->SetBoolPref(aPrefName, aValue);
  return rv;
}

NS_IMETHODIMP nsPref::GetCharPref(const char *aPrefName, char **_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetCharPref(aPrefName, _retval);
  return rv;
}

NS_IMETHODIMP nsPref::SetCharPref(const char *aPrefName, const char *aValue)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->SetCharPref(aPrefName, aValue);
  return rv;
}

NS_IMETHODIMP nsPref::GetIntPref(const char *aPrefName, PRInt32 *_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetIntPref(aPrefName, _retval);
  return rv;
}

NS_IMETHODIMP nsPref::SetIntPref(const char *aPrefName, PRInt32 aValue)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->SetIntPref(aPrefName, aValue);
  return rv;
}

NS_IMETHODIMP nsPref::GetComplexValue(const char *aPrefName, const nsIID & aType, void * *aValue)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetComplexValue(aPrefName, aType, aValue);
  return rv;
}

NS_IMETHODIMP nsPref::SetComplexValue(const char *aPrefName, const nsIID & aType, nsISupports *aValue)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->SetComplexValue(aPrefName, aType, aValue);
  return rv;
}

NS_IMETHODIMP nsPref::ClearUserPref(const char *aPrefName)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->ClearUserPref(aPrefName);
  return rv;
}

NS_IMETHODIMP nsPref::LockPref(const char *aPrefName)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->LockPref(aPrefName);
  return rv;
}

NS_IMETHODIMP nsPref::PrefIsLocked(const char *aPrefName, PRBool *_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->PrefIsLocked(aPrefName, _retval);
  return rv;
}

NS_IMETHODIMP nsPref::PrefHasUserValue(const char *aPrefName, PRBool *_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->PrefHasUserValue(aPrefName, _retval);
  return rv;
}

NS_IMETHODIMP nsPref::UnlockPref(const char *aPrefName)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->UnlockPref(aPrefName);
  return rv;
}

NS_IMETHODIMP nsPref::ResetBranch(const char *aStartingAt)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->ResetBranch(aStartingAt);
  return rv;
}

NS_IMETHODIMP nsPref::DeleteBranch(const char *aStartingAt)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->DeleteBranch(aStartingAt);
  return rv;
}

NS_IMETHODIMP nsPref::GetChildList(const char *aStartingAt, PRUint32 *aCount, char ***aChildArray)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetChildList(aStartingAt, aCount, aChildArray);
  return rv;
}

NS_IMETHODIMP nsPref::AddObserver(const char *aDomain, nsIObserver *aObserver, PRBool aHoldWeak)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch2> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->AddObserver(aDomain, aObserver, aHoldWeak);
  return rv;
}

NS_IMETHODIMP nsPref::RemoveObserver(const char *aDomain, nsIObserver *aObserver)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch2> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->RemoveObserver(aDomain, aObserver);
  return rv;
}

NS_IMETHODIMP nsPref::Observe(nsISupports *aSubject, const char *aTopic, const PRUnichar *someData)
{
  nsresult rv;

  nsCOMPtr<nsIObserver> observer = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = observer->Observe(aSubject, aTopic, someData);
  return rv;
}


/*
 * Some temporary support for deprecated functions.
 */

/*
 * Items replaced by Get/SetComplexValue
 */

NS_IMETHODIMP nsPref::CopyUnicharPref(const char *pref, PRUnichar **_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsISupportsString> theString;
    rv = prefBranch->GetComplexValue(pref, NS_GET_IID(nsISupportsString),
                                     getter_AddRefs(theString));
    if (NS_FAILED(rv))
      return rv;

    return theString->ToString(_retval);
  }
  return rv;
}

NS_IMETHODIMP nsPref::CopyDefaultUnicharPref(const char *pref, PRUnichar **_retval)
{
  nsresult rv;
  nsCOMPtr<nsISupportsString> theString;

  rv = mDefaultBranch->GetComplexValue(pref, NS_GET_IID(nsISupportsString),
                                       getter_AddRefs(theString));
  if (NS_FAILED(rv))
    return rv;

  return theString->ToString(_retval);
}

NS_IMETHODIMP nsPref::SetUnicharPref(const char *pref, const PRUnichar *value)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsISupportsString> theString = do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv)) {
      theString->SetData(nsDependentString(value));
      rv = prefBranch->SetComplexValue(pref, NS_GET_IID(nsISupportsString), theString);
    }
  }
  return rv;
}

NS_IMETHODIMP nsPref::SetDefaultUnicharPref(const char *pref, const PRUnichar *value)
{
  nsresult rv;

  nsCOMPtr<nsISupportsString> theString = do_CreateInstance(NS_SUPPORTS_STRING_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv)) {
    theString->SetData(nsDependentString(value));
    rv = mDefaultBranch->SetComplexValue(pref, NS_GET_IID(nsISupportsString), theString);
  }
  return rv;
}

NS_IMETHODIMP nsPref::GetLocalizedUnicharPref(const char *pref, PRUnichar **_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIPrefLocalizedString> theString;
    rv = prefBranch->GetComplexValue(pref, NS_GET_IID(nsIPrefLocalizedString),
                                       getter_AddRefs(theString));
    if (NS_SUCCEEDED(rv)) {
      rv = theString->ToString(_retval);
    }
  }
  return rv;
}

NS_IMETHODIMP nsPref::GetDefaultLocalizedUnicharPref(const char *pref, PRUnichar **_retval)
{
  nsresult rv;
  nsCOMPtr<nsIPrefLocalizedString> theString;

  rv = mDefaultBranch->GetComplexValue(pref, NS_GET_IID(nsIPrefLocalizedString),
                                       getter_AddRefs(theString));
  if (NS_SUCCEEDED(rv)) {
    rv = theString->ToString(_retval);
  }
  
  return rv;
}

NS_IMETHODIMP nsPref::GetFilePref(const char *pref, nsIFileSpec **_retval)
{
#ifdef MOZ_NO_XPCOM_OBSOLETE
  return NS_ERROR_NOT_IMPLEMENTED;
#else
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetComplexValue(pref, NS_GET_IID(nsIFileSpec), (void **)_retval);
  return rv;
#endif
}

NS_IMETHODIMP nsPref::SetFilePref(const char *pref, nsIFileSpec *value, PRBool setDefault)
{
#ifdef MOZ_NO_XPCOM_OBSOLETE
  return NS_ERROR_NOT_IMPLEMENTED;
#else
  nsresult  rv;

  if (setDefault) {
    rv = mDefaultBranch->SetComplexValue(pref, NS_GET_IID(nsIFileSpec), value);
  } else {
    nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
    if (NS_SUCCEEDED(rv))
      rv = prefBranch->SetComplexValue(pref, NS_GET_IID(nsIFileSpec), value);
  }
    return rv;
#endif
}

NS_IMETHODIMP nsPref::GetFileXPref(const char *pref, nsILocalFile **_retval)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->GetComplexValue(pref, NS_GET_IID(nsILocalFile), (void **)_retval);
  return rv;
}

NS_IMETHODIMP nsPref::SetFileXPref(const char *pref, nsILocalFile *value)
{
  nsresult rv;

  NS_ENSURE_ARG_POINTER(value);
  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_SUCCEEDED(rv))
    rv = prefBranch->SetComplexValue(pref, NS_GET_IID(nsILocalFile), value);
  return rv;
}


/*
 * Callbacks
 */

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::RegisterCallback( const char* domain,
                    PrefChangedFunc callback, 
                    void* instance_data )
//----------------------------------------------------------------------------------------
{
    PREF_RegisterCallback(domain, callback, instance_data);
    return NS_OK;
}

//----------------------------------------------------------------------------------------
NS_IMETHODIMP nsPref::UnregisterCallback( const char* domain,
                        PrefChangedFunc callback, 
                        void* instance_data )
//----------------------------------------------------------------------------------------
{
  return PREF_UnregisterCallback(domain, callback, instance_data);
}

/*
 * Preference enumeration
 */

NS_IMETHODIMP nsPref::EnumerateChildren(const char *parent, PrefEnumerationFunc callback, void *arg) 
{
  PRUint32 theCount;
  PRUint32 i;
  nsresult rv;
  char     **childArray;
  char     *prefName;

  rv = GetChildList(parent, &theCount, &childArray);
  if (NS_SUCCEEDED(rv)) {
    // now that we've built up the list, run the callback on all the matching elements
    for (i = 0; i < theCount; i++) {
      prefName = (char *)childArray[i];
      (*callback)((char*)prefName, arg);
    }

    // now release all the memory
    NS_FREE_XPCOM_ALLOCATED_POINTER_ARRAY(theCount, childArray);
  }

  return NS_OK;
}

/*
 * Pref access without security check - these are here
 * to support nsScriptSecurityManager.
 * These functions are part of nsISecurityPref, not nsIPref.
 * **PLEASE** do not call these functions from elsewhere
 */
NS_IMETHODIMP nsPref::SecurityGetBoolPref(const char *pref, PRBool * return_val)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsISecurityPref> securityPref = do_QueryInterface(prefBranch, &rv);
  if (NS_SUCCEEDED(rv))
    rv = securityPref->SecurityGetBoolPref(pref, return_val);
  return rv;
}

NS_IMETHODIMP nsPref::SecuritySetBoolPref(const char *pref, PRBool value)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsISecurityPref> securityPref = do_QueryInterface(prefBranch, &rv);
  if (NS_SUCCEEDED(rv))
    rv = securityPref->SecuritySetBoolPref(pref, value);
  return rv;
}

NS_IMETHODIMP nsPref::SecurityGetCharPref(const char *pref, char ** return_buf)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsISecurityPref> securityPref = do_QueryInterface(prefBranch, &rv);
  if (NS_SUCCEEDED(rv))
    rv = securityPref->SecurityGetCharPref(pref, return_buf);
  return rv;
}

NS_IMETHODIMP nsPref::SecuritySetCharPref(const char *pref, const char* value)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsISecurityPref> securityPref = do_QueryInterface(prefBranch, &rv);
  if (NS_SUCCEEDED(rv))
    rv = securityPref->SecuritySetCharPref(pref, value);
  return rv;
}

NS_IMETHODIMP nsPref::SecurityGetIntPref(const char *pref, PRInt32 * return_val)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsISecurityPref> securityPref = do_QueryInterface(prefBranch, &rv);
  if (NS_SUCCEEDED(rv))
    rv = securityPref->SecurityGetIntPref(pref, return_val);
  return rv;
}

NS_IMETHODIMP nsPref::SecuritySetIntPref(const char *pref, PRInt32 value)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsISecurityPref> securityPref = do_QueryInterface(prefBranch, &rv);
  if (NS_SUCCEEDED(rv))
    rv = securityPref->SecuritySetIntPref(pref, value);
  return rv;
}

NS_IMETHODIMP nsPref::SecurityClearUserPref(const char *pref_name)
{
  nsresult rv;

  nsCOMPtr<nsIPrefBranch> prefBranch = do_QueryInterface(mPrefService, &rv);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsISecurityPref> securityPref = do_QueryInterface(prefBranch, &rv);
  if (NS_SUCCEEDED(rv))
    rv = securityPref->SecurityClearUserPref(pref_name);
  return rv;
}


//----------------------------------------------------------------------------------------
nsPref* nsPref::GetInstance()
//----------------------------------------------------------------------------------------
{
  if (!gInstance)
  {
    NS_NEWXPCOM(gInstance, nsPref);
  }
  return gInstance;
} // nsPref::GetInstance


//----------------------------------------------------------------------------------------
// Functions used to create new instances of a given object by the
// generic factory.


////////////////////////////////////////////////////////////////////////
// Hand implement the GenericFactory constructor macro so I can make it
// non static. This is simply to keep us from having to make an nsPref.h
// file because we are trying to remove this object, not add to it.
//

NS_IMETHODIMP nsPrefConstructor(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  nsresult rv;
    
  if (NULL == aResult) {
    rv = NS_ERROR_NULL_POINTER;
    return rv;
  }
  *aResult = NULL;
  if (NULL != aOuter) {
    rv = NS_ERROR_NO_AGGREGATION;
    return rv;
  }

  nsPref *inst = nsPref::GetInstance();

  if (NULL == inst) {
    rv = NS_ERROR_OUT_OF_MEMORY;
    return rv;
  }
  NS_ADDREF(inst);
  rv = inst->QueryInterface(aIID, aResult);
  NS_RELEASE(inst);

  return rv;
}
