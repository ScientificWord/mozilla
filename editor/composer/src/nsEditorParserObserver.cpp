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
 *   steve clark <buster@netscape.com>
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

#include "nsServiceManagerUtils.h"
#include "nsIParserService.h"
#include "nsEditorParserObserver.h"

NS_IMPL_ADDREF(nsEditorParserObserver)
NS_IMPL_RELEASE(nsEditorParserObserver)

NS_INTERFACE_MAP_BEGIN(nsEditorParserObserver)
      NS_INTERFACE_MAP_ENTRY(nsIElementObserver)
      NS_INTERFACE_MAP_ENTRY(nsIObserver)
      NS_INTERFACE_MAP_ENTRY(nsISupportsWeakReference)
      NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIElementObserver)
NS_INTERFACE_MAP_END

nsEditorParserObserver::nsEditorParserObserver()
: mBadTagFound(PR_FALSE)
{
}

nsEditorParserObserver::~nsEditorParserObserver()
{
}

NS_IMETHODIMP nsEditorParserObserver::Notify(
                     PRUint32 aDocumentID, 
                     const PRUnichar* aTag, 
                     PRUint32 numOfAttributes, 
                     const PRUnichar* nameArray[], 
                     const PRUnichar* valueArray[])
{
  // XXX based on aTag...
  Notify();
  return NS_OK;
}

NS_IMETHODIMP nsEditorParserObserver::Notify(
                     PRUint32 aDocumentID, 
                     eHTMLTags aTag, 
                     PRUint32 numOfAttributes, 
                     const PRUnichar* nameArray[], 
                     const PRUnichar* valueArray[])
{
  if (eHTMLTag_frameset == aTag)
  {
    Notify();
    return NS_OK;
  }
  else
    return NS_ERROR_ILLEGAL_VALUE;
}
NS_IMETHODIMP nsEditorParserObserver::Notify(nsISupports* aWebShell, 
                                             nsISupports* aChannel, 
                                             const PRUnichar* aTag, 
                                             const nsStringArray* aKeys, 
                                             const nsStringArray* aValues,
                                             const PRUint32 aFlags)
{
  Notify();
  return NS_OK;
}

NS_IMETHODIMP nsEditorParserObserver::Observe(nsISupports*, const char*, const PRUnichar*)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

void nsEditorParserObserver::Notify()
{
  mBadTagFound = PR_TRUE;
}

NS_IMETHODIMP nsEditorParserObserver::Start(eHTMLTags* aWatchTags) 
{
  nsresult res = NS_OK;
  
  nsCOMPtr<nsIParserService> parserService(do_GetService("@mozilla.org/parser/parser-service;1"));
    
  if (!parserService) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  res = parserService->RegisterObserver(this,
                                        NS_LITERAL_STRING("text/html"),
                                        aWatchTags);
  return res;
}

NS_IMETHODIMP nsEditorParserObserver::End() 
{
  nsresult res = NS_OK;
  nsCOMPtr<nsIParserService> parserService(do_GetService("@mozilla.org/parser/parser-service;1"));

  if (!parserService) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
   
  res = parserService->UnregisterObserver(this,
                                          NS_LITERAL_STRING("text/html"));
  return res;
}

NS_IMETHODIMP nsEditorParserObserver::GetBadTagFound(PRBool *aFound)
{
  NS_ENSURE_ARG_POINTER(aFound);
  *aFound = mBadTagFound;
  return NS_OK; 
}



