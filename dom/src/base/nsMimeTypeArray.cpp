/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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
 * ***** END LICENSE BLOCK ***** */

#include "nsMimeTypeArray.h"
#include "nsContentUtils.h"
#include "nsIScriptGlobalObject.h"
#include "nsIDOMNavigator.h"
#include "nsIDOMPluginArray.h"
#include "nsIDOMPlugin.h"
#include "nsDOMClassInfo.h"
#include "nsIMIMEService.h"
#include "nsIMIMEInfo.h"
#include "nsIFile.h"


nsMimeTypeArray::nsMimeTypeArray(nsIDOMNavigator* navigator)
{
  mNavigator = navigator;
  mMimeTypeCount = 0;
  mMimeTypeArray = nsnull;
}

nsMimeTypeArray::~nsMimeTypeArray()
{
  Clear();
}


// QueryInterface implementation for nsMimeTypeArray
NS_INTERFACE_MAP_BEGIN(nsMimeTypeArray)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMimeTypeArray)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(MimeTypeArray)
NS_INTERFACE_MAP_END


NS_IMPL_ADDREF(nsMimeTypeArray)
NS_IMPL_RELEASE(nsMimeTypeArray)


NS_IMETHODIMP
nsMimeTypeArray::GetLength(PRUint32* aLength)
{
  if (mMimeTypeArray == nsnull) {
    nsresult rv = GetMimeTypes();
    if (rv != NS_OK)
      return rv;
  }
  *aLength = mMimeTypeCount;
  return NS_OK;
}

NS_IMETHODIMP
nsMimeTypeArray::Item(PRUint32 aIndex, nsIDOMMimeType** aReturn)
{
  if (mMimeTypeArray == nsnull) {
    nsresult rv = GetMimeTypes();
    if (rv != NS_OK)
      return rv;
  }
  if (aIndex < mMimeTypeCount) {
    *aReturn = mMimeTypeArray[aIndex];
    NS_IF_ADDREF(*aReturn);
    return NS_OK;
  }
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsMimeTypeArray::NamedItem(const nsAString& aName, nsIDOMMimeType** aReturn)
{
  NS_ENSURE_ARG_POINTER(aReturn);
  *aReturn = nsnull;

  if (mMimeTypeArray == nsnull) {
    nsresult rv = GetMimeTypes();
    if (rv != NS_OK)
      return rv;
  }

  nsAutoString type;

  for (PRUint32 i = 0; i < mMimeTypeCount; i++) {
    nsIDOMMimeType *mtype = mMimeTypeArray[i];

    mtype->GetType(type);

    if (type.Equals(aName)) {
      *aReturn = mtype;

      NS_ADDREF(*aReturn);

      return NS_OK;
    }
  }

  // Now let's check with the MIME service.
  nsCOMPtr<nsIMIMEService> mimeSrv = do_GetService("@mozilla.org/mime;1");
  if (mimeSrv) {
    nsCOMPtr<nsIMIMEInfo> mimeInfo;
    mimeSrv->GetFromTypeAndExtension(NS_ConvertUTF16toUTF8(aName), EmptyCString(),
                                     getter_AddRefs(mimeInfo));
    if (mimeInfo) {
      // Now we check whether we can really claim to support this type
      nsHandlerInfoAction action = nsIHandlerInfo::saveToDisk;
      mimeInfo->GetPreferredAction(&action);
      if (action != nsIMIMEInfo::handleInternally) {
        PRBool hasHelper = PR_FALSE;
        mimeInfo->GetHasDefaultHandler(&hasHelper);
        if (!hasHelper) {
          nsCOMPtr<nsIHandlerApp> helper;
          mimeInfo->GetPreferredApplicationHandler(getter_AddRefs(helper));
          if (!helper) {
            // mime info from the OS may not have a PreferredApplicationHandler
            // so just check for an empty default description
            nsAutoString defaultDescription;
            mimeInfo->GetDefaultDescription(defaultDescription);
            if (defaultDescription.IsEmpty()) {
              // no support; just leave
              return NS_OK;
            }
          }
        }
      }

      // If we got here, we support this type!  Say so.
      nsCOMPtr<nsIDOMMimeType> helper = new nsHelperMimeType(aName);
      if (!helper) {
        return NS_ERROR_OUT_OF_MEMORY;
      }
      nsCOMPtr<nsIDOMMimeType> entry = new nsMimeType(nsnull, helper);
      if (!entry) {
        return NS_ERROR_OUT_OF_MEMORY;
      }

      entry.swap(*aReturn);
    }
  }

  return NS_OK;
}

void  nsMimeTypeArray::Clear()
{
  if (mMimeTypeArray != nsnull) {
    for (PRUint32 i = 0; i < mMimeTypeCount; i++) {
      NS_IF_RELEASE(mMimeTypeArray[i]);
    }
    delete[] mMimeTypeArray;
    mMimeTypeArray = nsnull;
  }
  mMimeTypeCount = 0;
}

nsresult nsMimeTypeArray::Refresh()
{
  Clear();
  return GetMimeTypes();
}

nsresult nsMimeTypeArray::GetMimeTypes()
{
  NS_PRECONDITION(!mMimeTypeArray && mMimeTypeCount==0,
                      "already initialized");

  nsIDOMPluginArray* pluginArray = nsnull;
  nsresult rv = mNavigator->GetPlugins(&pluginArray);
  if (rv == NS_OK) {
    // count up all possible MimeTypes, and collect them here. Later,
    // we'll remove duplicates.
    mMimeTypeCount = 0;
    PRUint32 pluginCount = 0;
    rv = pluginArray->GetLength(&pluginCount);
    if (rv == NS_OK) {
      PRUint32 i;
      for (i = 0; i < pluginCount; i++) {
        nsCOMPtr<nsIDOMPlugin> plugin;
        if (NS_SUCCEEDED(pluginArray->Item(i, getter_AddRefs(plugin))) &&
            plugin) {
          PRUint32 mimeTypeCount = 0;
          if (plugin->GetLength(&mimeTypeCount) == NS_OK)
            mMimeTypeCount += mimeTypeCount;
        }
      }
      // now we know how many there are, start gathering them.
      mMimeTypeArray = new nsIDOMMimeType*[mMimeTypeCount];
      if (mMimeTypeArray == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
      PRUint32 mimeTypeIndex = 0;
      PRUint32 k;
      for (k = 0; k < pluginCount; k++) {
        nsIDOMPlugin* plugin = nsnull;
        if (pluginArray->Item(k, &plugin) == NS_OK) {
          PRUint32 mimeTypeCount = 0;
          if (plugin->GetLength(&mimeTypeCount) == NS_OK) {
            for (PRUint32 j = 0; j < mimeTypeCount; j++)
              plugin->Item(j, &mMimeTypeArray[mimeTypeIndex++]);
          }
          NS_RELEASE(plugin);
        }
      }
    }
    NS_RELEASE(pluginArray);
  }
  return rv;
}

nsMimeType::nsMimeType(nsIDOMPlugin* aPlugin, nsIDOMMimeType* aMimeType)
{
  mPlugin = aPlugin;
  mMimeType = aMimeType;
}

nsMimeType::~nsMimeType()
{
}


// QueryInterface implementation for nsMimeType
NS_INTERFACE_MAP_BEGIN(nsMimeType)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMimeType)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(MimeType)
NS_INTERFACE_MAP_END


NS_IMPL_ADDREF(nsMimeType)
NS_IMPL_RELEASE(nsMimeType)


NS_IMETHODIMP
nsMimeType::GetDescription(nsAString& aDescription)
{
  return mMimeType->GetDescription(aDescription);
}

NS_IMETHODIMP
nsMimeType::GetEnabledPlugin(nsIDOMPlugin** aEnabledPlugin)
{
  nsAutoString type;
  GetType(type);

  PRBool disabled = PR_FALSE;

  if (type.Length() == 1 && type.First() == '*') {
    // Check if the default plugin is disabled.
    disabled = nsContentUtils::GetBoolPref("plugin.default_plugin_disabled");
  }

  *aEnabledPlugin = disabled ? nsnull : mPlugin;

  NS_IF_ADDREF(*aEnabledPlugin);

  return NS_OK;
}

NS_IMETHODIMP
nsMimeType::GetSuffixes(nsAString& aSuffixes)
{
  return mMimeType->GetSuffixes(aSuffixes);
}

NS_IMETHODIMP
nsMimeType::GetType(nsAString& aType)
{
  return mMimeType->GetType(aType);
}

// QueryInterface implementation for nsHelperMimeType
NS_IMPL_ISUPPORTS1(nsHelperMimeType, nsIDOMMimeType)

NS_IMETHODIMP
nsHelperMimeType::GetDescription(nsAString& aDescription)
{
  aDescription.Truncate();
  return NS_OK;
}

NS_IMETHODIMP
nsHelperMimeType::GetEnabledPlugin(nsIDOMPlugin** aEnabledPlugin)
{
  *aEnabledPlugin = nsnull;
  return NS_OK;
}

NS_IMETHODIMP
nsHelperMimeType::GetSuffixes(nsAString& aSuffixes)
{
  aSuffixes.Truncate();
  return NS_OK;
}

NS_IMETHODIMP
nsHelperMimeType::GetType(nsAString& aType)
{
  aType = mType;
  return NS_OK;
}

