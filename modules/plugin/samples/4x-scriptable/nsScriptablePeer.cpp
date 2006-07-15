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

// ==============================
// ! Scriptability related code !
// ==============================
//

/////////////////////////////////////////////////////
//
// This file implements the nsScriptablePeer object
// The native methods of this class are supposed to
// be callable from JavaScript
//
#include "plugin.h"
#include "acmeIScriptObject.h"
#include "npapi.h"

static NS_DEFINE_IID(kI4xScriptablePluginIID, NS_I4XSCRIPTABLEPLUGIN_IID);
static NS_DEFINE_IID(kIClassInfoIID, NS_ICLASSINFO_IID);
static NS_DEFINE_IID(kISupportsIID, NS_ISUPPORTS_IID);

nsScriptablePeer::nsScriptablePeer(CPlugin* aPlugin)
{
  mRefCnt = 0;
  mPlugin = aPlugin;
  mWindow = nsnull;
}

nsScriptablePeer::~nsScriptablePeer()
{
}

// AddRef, Release and QueryInterface are common methods and must 
// be implemented for any interface
NS_IMETHODIMP_(nsrefcnt) nsScriptablePeer::AddRef() 
{ 
  ++mRefCnt; 
  return mRefCnt; 
} 

NS_IMETHODIMP_(nsrefcnt) nsScriptablePeer::Release() 
{ 
  --mRefCnt; 
  if (mRefCnt == 0) { 
    delete this;
    return 0; 
  } 
  return mRefCnt; 
} 

// here nsScriptablePeer should return three interfaces it can be asked for by their iid's
// static casts are necessary to ensure that correct pointer is returned
NS_IMETHODIMP nsScriptablePeer::QueryInterface(const nsIID& aIID, void** aInstancePtr) 
{ 
  if(!aInstancePtr) 
    return NS_ERROR_NULL_POINTER; 

  if(aIID.Equals(kI4xScriptablePluginIID)) {
    *aInstancePtr = static_cast<nsI4xScriptablePlugin*>(this); 
    AddRef();
    return NS_OK;
  }

  if(aIID.Equals(kIClassInfoIID)) {
    *aInstancePtr = static_cast<nsIClassInfo*>(this); 
    AddRef();
    return NS_OK;
  }

  if(aIID.Equals(kISupportsIID)) {
    *aInstancePtr = static_cast<nsISupports*>(static_cast<nsI4xScriptablePlugin*>(this)); 
    AddRef();
    return NS_OK;
  }

  return NS_NOINTERFACE; 
}

//
// the following two methods will be callable from JavaScript
//
NS_IMETHODIMP nsScriptablePeer::ShowVersion()
{
  if (mPlugin)
    mPlugin->showVersion();

  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::Clear()
{
  if (mPlugin)
    mPlugin->clear();

  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::GetVersion(char * *aVersion)
{
  if (mPlugin)
    mPlugin->getVersion(aVersion);
  return NS_OK;
}

NS_IMETHODIMP nsScriptablePeer::SetWindow(acmeIScriptObject *window)
{
  NS_IF_ADDREF(window);
  NS_IF_RELEASE(mWindow);
  mWindow = window;
  
  // evaluate a JavaScript expression.
  acmeIScriptObject* result;
  nsresult rv = window->Evaluate("Math.PI", &result);
  if (NS_SUCCEEDED(rv) && result) {
      double value;
      result->ToNumber(&value);
      NS_RELEASE(result);
  }

  // read the current window's location.
  acmeIScriptObject* location = nsnull;
  rv = window->GetProperty("location", &location);
  if (NS_SUCCEEDED(rv) && location) {
    char* locationStr = NULL;
    rv = location->ToString(&locationStr);
    if (NS_SUCCEEDED(rv) && locationStr) {
      NPN_MemFree(locationStr);
    }
    NS_RELEASE(location);
  }
  
  return NS_OK;
}
