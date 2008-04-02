/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 sw=2 et tw=80: */
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
 *   Travis Bogard <travis@netscape.com>
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

#include "nsGlobalWindow.h"
#include "nsIScriptSecurityManager.h"
#include "nsIScriptContext.h"
#include "nsIDocShell.h"
#include "nsIDocShellLoadInfo.h"
#include "nsIWebNavigation.h"
#include "nsCDefaultURIFixup.h"
#include "nsIURIFixup.h"
#include "nsIURL.h"
#include "nsIJARURI.h"
#include "nsIIOService.h"
#include "nsIServiceManager.h"
#include "nsNetUtil.h"
#include "plstr.h"
#include "prprf.h"
#include "prmem.h"
#include "nsCOMPtr.h"
#include "nsEscape.h"
#include "nsJSUtils.h"
#include "nsIDOMWindow.h"
#include "nsIDOMDocument.h"
#include "nsIDocument.h"
#include "nsIPresShell.h"
#include "nsPresContext.h"
#include "nsIJSContextStack.h"
#include "nsXPIDLString.h"
#include "nsDOMError.h"
#include "nsDOMClassInfo.h"
#include "nsCRT.h"
#include "nsIProtocolHandler.h"
#include "nsReadableUtils.h"
#include "nsITextToSubURI.h"
#include "nsContentUtils.h"

static nsresult
GetContextFromStack(nsIJSContextStack *aStack, JSContext **aContext)
{
  nsCOMPtr<nsIJSContextStackIterator>
    iterator(do_CreateInstance("@mozilla.org/js/xpc/ContextStackIterator;1"));
  NS_ENSURE_TRUE(iterator, NS_ERROR_FAILURE);

  nsresult rv = iterator->Reset(aStack);
  NS_ENSURE_SUCCESS(rv, rv);

  PRBool done;
  while (NS_SUCCEEDED(iterator->Done(&done)) && !done) {
    rv = iterator->Prev(aContext);
    NS_ASSERTION(NS_SUCCEEDED(rv), "Broken iterator implementation");

    // Consider a null context the end of the line.
    if (!*aContext) {
      break;
    }

    if (nsJSUtils::GetDynamicScriptContext(*aContext)) {
      return NS_OK;
    }
  }

  *aContext = nsnull;

  return NS_OK;
}

static nsresult
GetDocumentCharacterSetForURI(const nsAString& aHref, nsACString& aCharset)
{
  aCharset.Truncate();

  nsresult rv;

  nsCOMPtr<nsIJSContextStack> stack(do_GetService("@mozilla.org/js/xpc/ContextStack;1", &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  JSContext *cx;

  rv = GetContextFromStack(stack, &cx);
  NS_ENSURE_SUCCESS(rv, rv);

  if (cx) {
    nsCOMPtr<nsIDOMWindow> window =
      do_QueryInterface(nsJSUtils::GetDynamicScriptGlobal(cx));
    NS_ENSURE_TRUE(window, NS_ERROR_FAILURE);

    nsCOMPtr<nsIDOMDocument> domDoc;
    rv = window->GetDocument(getter_AddRefs(domDoc));
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIDocument> doc(do_QueryInterface(domDoc));

    if (doc) {
      aCharset = doc->GetDocumentCharacterSet();
    }
  }

  return NS_OK;
}

nsLocation::nsLocation(nsIDocShell *aDocShell)
{
  mDocShell = do_GetWeakReference(aDocShell);
}

nsLocation::~nsLocation()
{
}


// QueryInterface implementation for nsLocation
NS_INTERFACE_MAP_BEGIN(nsLocation)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNSLocation)
  NS_INTERFACE_MAP_ENTRY(nsIDOMLocation)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMLocation)
  NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(Location)
NS_INTERFACE_MAP_END


NS_IMPL_ADDREF(nsLocation)
NS_IMPL_RELEASE(nsLocation)

void
nsLocation::SetDocShell(nsIDocShell *aDocShell)
{
   mDocShell = do_GetWeakReference(aDocShell);
}

nsIDocShell *
nsLocation::GetDocShell()
{
  nsCOMPtr<nsIDocShell> docshell(do_QueryReferent(mDocShell));
  return docshell;
}

nsresult
nsLocation::CheckURL(nsIURI* aURI, nsIDocShellLoadInfo** aLoadInfo)
{
  *aLoadInfo = nsnull;

  nsCOMPtr<nsIDocShell> docShell(do_QueryReferent(mDocShell));
  if (!docShell) {
    return NS_ERROR_NOT_AVAILABLE;
  }

  nsresult result;
  // Get JSContext from stack.
  nsCOMPtr<nsIJSContextStack>
    stack(do_GetService("@mozilla.org/js/xpc/ContextStack;1", &result));

  if (NS_FAILED(result))
    return NS_ERROR_FAILURE;

  JSContext *cx;

  if (NS_FAILED(GetContextFromStack(stack, &cx)))
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsISupports> owner;
  nsCOMPtr<nsIURI> sourceURI;

  if (cx) {
    // No cx means that there's no JS running, or at least no JS that
    // was run through code that properly pushed a context onto the
    // context stack (as all code that runs JS off of web pages
    // does). We won't bother with security checks in this case, but
    // we need to create the loadinfo etc.

    // Get security manager.
    nsCOMPtr<nsIScriptSecurityManager>
      secMan(do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &result));

    if (NS_FAILED(result))
      return NS_ERROR_FAILURE;

    // Check to see if URI is allowed.
    result = secMan->CheckLoadURIFromScript(cx, aURI);

    if (NS_FAILED(result))
      return result;

    // Now get the principal to use when loading the URI
    nsCOMPtr<nsIPrincipal> principal;
    if (NS_FAILED(secMan->GetSubjectPrincipal(getter_AddRefs(principal))) ||
        !principal)
      return NS_ERROR_FAILURE;
    owner = do_QueryInterface(principal);
    principal->GetURI(getter_AddRefs(sourceURI));
  }

  // Create load info
  nsCOMPtr<nsIDocShellLoadInfo> loadInfo;
  docShell->CreateLoadInfo(getter_AddRefs(loadInfo));
  NS_ENSURE_TRUE(loadInfo, NS_ERROR_FAILURE);

  loadInfo->SetOwner(owner);

  // now set the referrer on the loadinfo
  if (sourceURI) {
    loadInfo->SetReferrer(sourceURI);
  }

  loadInfo.swap(*aLoadInfo);

  return NS_OK;
}

// Walk up the docshell hierarchy and find a usable base URI. Basically 
// anything that would allow a relative uri.

nsresult
nsLocation::FindUsableBaseURI(nsIURI * aBaseURI, nsIDocShell * aParent,
                              nsIURI ** aUsableURI)
{
  if (!aBaseURI || !aParent)
    return NS_ERROR_FAILURE;
  NS_ENSURE_ARG_POINTER(aUsableURI);

  *aUsableURI = nsnull;
  nsresult rv = NS_OK;    
  nsCOMPtr<nsIDocShell> parentDS = aParent;
  nsCOMPtr<nsIURI> baseURI = aBaseURI;
  nsCOMPtr<nsIIOService> ioService =
    do_GetService(NS_IOSERVICE_CONTRACTID, &rv);

  while(NS_SUCCEEDED(rv) && baseURI) {
    // Check if the current base uri supports relative uris.
    // We make this check by looking at the protocol flags of
    // the protocol handler. If the protocol flags has URI_NORELATIVE,
    // it means that the base uri does not support relative uris.
    nsCAutoString scheme;
    baseURI->GetScheme(scheme);
    nsCOMPtr<nsIProtocolHandler> protocolHandler;
    // Get the protocol handler for the base uri.
    ioService->GetProtocolHandler(scheme.get(), getter_AddRefs(protocolHandler));
    if (!protocolHandler)
      return NS_ERROR_FAILURE;
    PRUint32 pFlags; // Is there a default value for the protocol flags?
    protocolHandler->GetProtocolFlags(&pFlags);
    if (!(pFlags & nsIProtocolHandler::URI_NORELATIVE)) {
      *aUsableURI = baseURI;
      NS_ADDREF(*aUsableURI);
      return NS_OK;
    }

    // Get the same type parent docshell
    nsCOMPtr<nsIDocShellTreeItem> docShellAsTreeItem(do_QueryInterface(parentDS));
    if (!docShellAsTreeItem)
      return NS_ERROR_FAILURE;
    nsCOMPtr<nsIDocShellTreeItem> parentDSTreeItem;
    docShellAsTreeItem->GetSameTypeParent(getter_AddRefs(parentDSTreeItem));      
    nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(parentDSTreeItem));

    // Get the parent docshell's uri
    if (webNav) {
      rv = webNav->GetCurrentURI(getter_AddRefs(baseURI));
      parentDS = do_QueryInterface(parentDSTreeItem);
    }
    else
      return NS_ERROR_FAILURE;
  }  // while 

  return rv;
}


nsresult
nsLocation::GetURI(nsIURI** aURI, PRBool aGetInnermostURI)
{
  *aURI = nsnull;

  nsresult rv;
  nsCOMPtr<nsIDocShell> docShell(do_QueryReferent(mDocShell));
  nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(docShell, &rv));
  if (NS_FAILED(rv)) {
    return rv;
  }

  nsCOMPtr<nsIURI> uri;
  rv = webNav->GetCurrentURI(getter_AddRefs(uri));
  NS_ENSURE_SUCCESS(rv, rv);

  // It is valid for docshell to return a null URI. Don't try to fixup
  // if this happens.
  if (!uri) {
    return NS_OK;
  }

  if (aGetInnermostURI) {
    nsCOMPtr<nsIJARURI> jarURI(do_QueryInterface(uri));
    while (jarURI) {
      jarURI->GetJARFile(getter_AddRefs(uri));
      jarURI = do_QueryInterface(uri);
    }
  }

  NS_ASSERTION(uri, "nsJARURI screwed up?");

  nsCOMPtr<nsIURIFixup> urifixup(do_GetService(NS_URIFIXUP_CONTRACTID, &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  return urifixup->CreateExposableURI(uri, aURI);
}

nsresult
nsLocation::GetWritableURI(nsIURI** aURI)
{
  *aURI = nsnull;

  nsCOMPtr<nsIURI> uri;

  nsresult rv = GetURI(getter_AddRefs(uri));
  if (NS_FAILED(rv) || !uri) {
    return rv;
  }

  return uri->Clone(aURI);
}

nsresult
nsLocation::SetURI(nsIURI* aURI, PRBool aReplace)
{
  nsCOMPtr<nsIDocShell> docShell(do_QueryReferent(mDocShell));
  if (docShell) {
    nsCOMPtr<nsIDocShellLoadInfo> loadInfo;
    nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(docShell));

    if(NS_FAILED(CheckURL(aURI, getter_AddRefs(loadInfo))))
      return NS_ERROR_FAILURE;

    if (aReplace) {
      loadInfo->SetLoadType(nsIDocShellLoadInfo::loadStopContentAndReplace);
    } else {
      loadInfo->SetLoadType(nsIDocShellLoadInfo::loadStopContent);
    }

    return docShell->LoadURI(aURI, loadInfo,
                             nsIWebNavigation::LOAD_FLAGS_NONE, PR_TRUE);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsLocation::GetHash(nsAString& aHash)
{
  aHash.SetLength(0);

  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetURI(getter_AddRefs(uri));

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));

  if (url) {
    nsCAutoString ref;
    nsAutoString unicodeRef;

    rv = url->GetRef(ref);
    if (NS_SUCCEEDED(rv)) {
      nsCOMPtr<nsITextToSubURI> textToSubURI(
          do_GetService(NS_ITEXTTOSUBURI_CONTRACTID, &rv));

      if (NS_SUCCEEDED(rv)) {
        nsCAutoString charset;
        url->GetOriginCharset(charset);
        
        rv = textToSubURI->UnEscapeURIForUI(charset, ref, unicodeRef);
      }
      
      if (NS_FAILED(rv)) {
        // Oh, well.  No intl here!
        NS_UnescapeURL(ref);
        CopyASCIItoUTF16(ref, unicodeRef);
        rv = NS_OK;
      }
    }

    if (NS_SUCCEEDED(rv) && !unicodeRef.IsEmpty()) {
      aHash.Assign(PRUnichar('#'));
      aHash.Append(unicodeRef);
    }
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::SetHash(const nsAString& aHash)
{
  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetWritableURI(getter_AddRefs(uri));

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));
  if (url) {
    rv = url->SetRef(NS_ConvertUTF16toUTF8(aHash));
    if (NS_SUCCEEDED(rv)) {
      SetURI(url);
    }
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::GetHost(nsAString& aHost)
{
  aHost.Truncate();

  nsCOMPtr<nsIURI> uri;
  nsresult result;

  result = GetURI(getter_AddRefs(uri), PR_TRUE);

  if (uri) {
    nsCAutoString hostport;

    result = uri->GetHostPort(hostport);

    if (NS_SUCCEEDED(result)) {
      AppendUTF8toUTF16(hostport, aHost);
    }
  }

  return result;
}

NS_IMETHODIMP
nsLocation::SetHost(const nsAString& aHost)
{
  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetWritableURI(getter_AddRefs(uri));

  if (uri) {
    rv = uri->SetHostPort(NS_ConvertUTF16toUTF8(aHost));
    if (NS_SUCCEEDED(rv)) {
      SetURI(uri);
    }
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::GetHostname(nsAString& aHostname)
{
  aHostname.Truncate();

  nsCOMPtr<nsIURI> uri;
  nsresult result;

  result = GetURI(getter_AddRefs(uri), PR_TRUE);

  if (uri) {
    nsCAutoString host;

    result = uri->GetHost(host);

    if (NS_SUCCEEDED(result)) {
      AppendUTF8toUTF16(host, aHostname);
    }
  }

  return result;
}

NS_IMETHODIMP
nsLocation::SetHostname(const nsAString& aHostname)
{
  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetWritableURI(getter_AddRefs(uri));

  if (uri) {
    rv = uri->SetHost(NS_ConvertUTF16toUTF8(aHostname));
    if (NS_SUCCEEDED(rv)) {
      SetURI(uri);
    }
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::GetHref(nsAString& aHref)
{
  aHref.Truncate();

  nsCOMPtr<nsIURI> uri;
  nsresult result;

  result = GetURI(getter_AddRefs(uri));

  if (uri) {
    nsCAutoString uriString;

    result = uri->GetSpec(uriString);

    if (NS_SUCCEEDED(result)) {
      AppendUTF8toUTF16(uriString, aHref);
    }
  }

  return result;
}

NS_IMETHODIMP
nsLocation::SetHref(const nsAString& aHref)
{
  nsAutoString oldHref;
  nsresult rv = NS_OK;

  // Get JSContext from stack.
  nsCOMPtr<nsIJSContextStack>
    stack(do_GetService("@mozilla.org/js/xpc/ContextStack;1", &rv));

  if (NS_FAILED(rv))
    return NS_ERROR_FAILURE;

  JSContext *cx;

  if (NS_FAILED(GetContextFromStack(stack, &cx)))
    return NS_ERROR_FAILURE;

  if (cx) {
    rv = SetHrefWithContext(cx, aHref, PR_FALSE);
  } else {
    rv = GetHref(oldHref);

    if (NS_SUCCEEDED(rv)) {
      nsCOMPtr<nsIURI> oldUri;

      rv = NS_NewURI(getter_AddRefs(oldUri), oldHref);

      if (oldUri) {
        rv = SetHrefWithBase(aHref, oldUri, PR_FALSE);
      }
    }
  }

  return rv;
}

nsresult
nsLocation::SetHrefWithContext(JSContext* cx, const nsAString& aHref,
                               PRBool aReplace)
{
  nsCOMPtr<nsIURI> base;

  // Get the source of the caller
  nsresult result = GetSourceBaseURL(cx, getter_AddRefs(base));

  if (NS_FAILED(result)) {
    return result;
  }

  return SetHrefWithBase(aHref, base, aReplace);
}

nsresult
nsLocation::SetHrefWithBase(const nsAString& aHref, nsIURI* aBase,
                            PRBool aReplace)
{
  nsresult result;
  nsCOMPtr<nsIURI> newUri, baseURI;

  nsCOMPtr<nsIDocShell> docShell(do_QueryReferent(mDocShell));

  // Try to make sure the base url is something that will be useful. 
  result = FindUsableBaseURI(aBase,  docShell, getter_AddRefs(baseURI));
  if (!baseURI)  {
    // If nothing useful was found, just use what you have.
    baseURI = aBase;
  }

  nsCAutoString docCharset;
  if (NS_SUCCEEDED(GetDocumentCharacterSetForURI(aHref, docCharset)))
    result = NS_NewURI(getter_AddRefs(newUri), aHref, docCharset.get(), baseURI);
  else
    result = NS_NewURI(getter_AddRefs(newUri), aHref, nsnull, baseURI);

  if (newUri) {
    /* Check with the scriptContext if it is currently processing a script tag.
     * If so, this must be a <script> tag with a location.href in it.
     * we want to do a replace load, in such a situation. 
     * In other cases, for example if a event handler or a JS timer
     * had a location.href in it, we want to do a normal load,
     * so that the new url will be appended to Session History.
     * This solution is tricky. Hopefully it isn't going to bite
     * anywhere else. This is part of solution for bug # 39938, 72197
     * 
     */
    PRBool inScriptTag=PR_FALSE;
    // Get JSContext from stack.
    nsCOMPtr<nsIJSContextStack> stack(do_GetService("@mozilla.org/js/xpc/ContextStack;1", &result));

    if (stack) {
      JSContext *cx;

      result = GetContextFromStack(stack, &cx);
      if (cx) {
        nsIScriptContext *scriptContext =
          nsJSUtils::GetDynamicScriptContext(cx);

        if (scriptContext) {
          if (scriptContext->GetProcessingScriptTag()) {
            // Now check to make sure that the script is running in our window,
            // since we only want to replace if the location is set by a
            // <script> tag in the same window.  See bug 178729.
            nsCOMPtr<nsIScriptGlobalObject> ourGlobal(do_GetInterface(docShell));
            inScriptTag = (ourGlobal == scriptContext->GetGlobalObject());
          }
        }  
      } //cx
    }  // stack

    return SetURI(newUri, aReplace || inScriptTag);
  }

  return result;
}

NS_IMETHODIMP
nsLocation::GetPathname(nsAString& aPathname)
{
  aPathname.Truncate();

  nsCOMPtr<nsIURI> uri;
  nsresult result = NS_OK;

  result = GetURI(getter_AddRefs(uri));

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));
  if (url) {
    nsCAutoString file;

    result = url->GetFilePath(file);

    if (NS_SUCCEEDED(result)) {
      AppendUTF8toUTF16(file, aPathname);
    }
  }

  return result;
}

NS_IMETHODIMP
nsLocation::SetPathname(const nsAString& aPathname)
{
  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetWritableURI(getter_AddRefs(uri));

  if (uri) {
    rv = uri->SetPath(NS_ConvertUTF16toUTF8(aPathname));
    if (NS_SUCCEEDED(rv)) {
      SetURI(uri);
    }
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::GetPort(nsAString& aPort)
{
  aPort.SetLength(0);

  nsCOMPtr<nsIURI> uri;
  nsresult result = NS_OK;

  result = GetURI(getter_AddRefs(uri), PR_TRUE);

  if (uri) {
    PRInt32 port;
    result = uri->GetPort(&port);

    if (NS_SUCCEEDED(result) && -1 != port) {
      nsAutoString portStr;
      portStr.AppendInt(port);
      aPort.Append(portStr);
    }

    // Don't propagate this exception to caller
    result = NS_OK;
  }

  return result;
}

NS_IMETHODIMP
nsLocation::SetPort(const nsAString& aPort)
{
  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetWritableURI(getter_AddRefs(uri));

  if (uri) {
    // perhaps use nsReadingIterators at some point?
    NS_ConvertUTF16toUTF8 portStr(aPort);
    const char *buf = portStr.get();
    PRInt32 port = -1;

    if (buf) {
      if (*buf == ':') {
        port = atol(buf+1);
      }
      else {
        port = atol(buf);
      }
    }

    rv = uri->SetPort(port);
    if (NS_SUCCEEDED(rv)) {
      SetURI(uri);
    }
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::GetProtocol(nsAString& aProtocol)
{
  aProtocol.SetLength(0);

  nsCOMPtr<nsIURI> uri;
  nsresult result = NS_OK;

  result = GetURI(getter_AddRefs(uri));

  if (uri) {
    nsCAutoString protocol;

    result = uri->GetScheme(protocol);

    if (NS_SUCCEEDED(result)) {
      CopyASCIItoUTF16(protocol, aProtocol);
      aProtocol.Append(PRUnichar(':'));
    }
  }

  return result;
}

NS_IMETHODIMP
nsLocation::SetProtocol(const nsAString& aProtocol)
{
  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetWritableURI(getter_AddRefs(uri));

  if (uri) {
    rv = uri->SetScheme(NS_ConvertUTF16toUTF8(aProtocol));
    if (NS_SUCCEEDED(rv)) {
      SetURI(uri);
    }
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::GetSearch(nsAString& aSearch)
{
  aSearch.SetLength(0);

  nsCOMPtr<nsIURI> uri;
  nsresult result = NS_OK;

  result = GetURI(getter_AddRefs(uri));

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));

  if (url) {
    nsCAutoString search;

    result = url->GetQuery(search);

    if (NS_SUCCEEDED(result) && !search.IsEmpty()) {
      aSearch.Assign(PRUnichar('?'));
      AppendUTF8toUTF16(search, aSearch);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsLocation::SetSearch(const nsAString& aSearch)
{
  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetWritableURI(getter_AddRefs(uri));

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));
  if (url) {
    rv = url->SetQuery(NS_ConvertUTF16toUTF8(aSearch));
    if (NS_SUCCEEDED(rv)) {
      SetURI(uri);
    }
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::Reload(PRBool aForceget)
{
  nsresult rv;
  nsCOMPtr<nsIDocShell> docShell(do_QueryReferent(mDocShell));
  nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(docShell));

  if (webNav) {
    PRUint32 reloadFlags = nsIWebNavigation::LOAD_FLAGS_NONE;

    if (aForceget) {
      reloadFlags = nsIWebNavigation::LOAD_FLAGS_BYPASS_CACHE | 
                    nsIWebNavigation::LOAD_FLAGS_BYPASS_PROXY;
    }
    rv = webNav->Reload(reloadFlags);
    if (rv == NS_BINDING_ABORTED) {
      // This happens when we attempt to reload a POST result and the user says
      // no at the "do you want to reload?" prompt.  Don't propagate this one
      // back to callers.
      rv = NS_OK;
    }
  } else {
    rv = NS_ERROR_FAILURE;
  }

  return rv;
}

NS_IMETHODIMP
nsLocation::Reload()
{
  nsAXPCNativeCallContext *ncc = nsnull;
  nsresult rv = nsContentUtils::XPConnect()->
    GetCurrentNativeCallContext(&ncc);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!ncc)
    return NS_ERROR_NOT_AVAILABLE;

  nsCOMPtr<nsIDocShell> docShell(do_QueryReferent(mDocShell));
  nsCOMPtr<nsPIDOMWindow> window(do_GetInterface(docShell));

  if (window && window->IsHandlingResizeEvent()) {
    // location.reload() was called on a window that is handling a
    // resize event. Sites do this since Netscape 4.x needed it, but
    // we don't, and it's a horrible experience for nothing. In stead
    // of reloading the page, just clear style data and reflow the
    // page since some sites may use this trick to work around gecko
    // reflow bugs, and this should have the same effect.

    nsCOMPtr<nsIDocument> doc(do_QueryInterface(window->GetExtantDocument()));

    nsIPresShell *shell;
    nsPresContext *pcx;
    if (doc && (shell = doc->GetPrimaryShell()) &&
        (pcx = shell->GetPresContext())) {
      pcx->RebuildAllStyleData(NS_STYLE_HINT_REFLOW);
    }

    return NS_OK;
  }

  PRBool force_get = PR_FALSE;

  PRUint32 argc;

  ncc->GetArgc(&argc);

  if (argc > 0) {
    jsval *argv = nsnull;

    ncc->GetArgvPtr(&argv);
    NS_ENSURE_TRUE(argv, NS_ERROR_UNEXPECTED);

    JSContext *cx = nsnull;

    rv = ncc->GetJSContext(&cx);
    NS_ENSURE_SUCCESS(rv, rv);

    JSAutoRequest ar(cx);
    JS_ValueToBoolean(cx, argv[0], &force_get);
  }

  return Reload(force_get);
}

NS_IMETHODIMP
nsLocation::Replace(const nsAString& aUrl)
{
  nsresult rv = NS_OK;

  // Get JSContext from stack.
  nsCOMPtr<nsIJSContextStack>
  stack(do_GetService("@mozilla.org/js/xpc/ContextStack;1"));

  if (stack) {
    JSContext *cx;

    rv = GetContextFromStack(stack, &cx);
    NS_ENSURE_SUCCESS(rv, rv);
    if (cx) {
      return SetHrefWithContext(cx, aUrl, PR_TRUE);
    }
  }

  nsAutoString oldHref;

  rv = GetHref(oldHref);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIURI> oldUri;

  rv = NS_NewURI(getter_AddRefs(oldUri), oldHref);
  NS_ENSURE_SUCCESS(rv, rv);

  return SetHrefWithBase(aUrl, oldUri, PR_TRUE);
}

NS_IMETHODIMP
nsLocation::Assign(const nsAString& aUrl)
{
  nsAutoString oldHref;
  nsresult result = NS_OK;

  result = GetHref(oldHref);

  if (NS_SUCCEEDED(result)) {
    nsCOMPtr<nsIURI> oldUri;

    result = NS_NewURI(getter_AddRefs(oldUri), oldHref);

    if (oldUri) {
      result = SetHrefWithBase(aUrl, oldUri, PR_FALSE);
    }
  }

  return result;
}

NS_IMETHODIMP
nsLocation::ToString(nsAString& aReturn)
{
  return GetHref(aReturn);
}

nsresult
nsLocation::GetSourceDocument(JSContext* cx, nsIDocument** aDocument)
{
  // XXX Code duplicated from nsHTMLDocument
  // XXX Tom said this reminded him of the "Six Degrees of
  // Kevin Bacon" game. We try to get from here to there using
  // whatever connections possible. The problem is that this
  // could break if any of the connections along the way change.
  // I wish there were a better way.

  nsresult rv = NS_ERROR_FAILURE;

  // We need to use the dynamically scoped global and assume that the
  // current JSContext is a DOM context with a nsIScriptGlobalObject so
  // that we can get the url of the caller.
  // XXX This will fail on non-DOM contexts :(

  nsCOMPtr<nsIDOMWindow> window =
    do_QueryInterface(nsJSUtils::GetDynamicScriptGlobal(cx), &rv);

  if (window) {
    nsCOMPtr<nsIDOMDocument> domDoc;
    rv = window->GetDocument(getter_AddRefs(domDoc));
    if (domDoc) {
      return CallQueryInterface(domDoc, aDocument);
    }
  } else {
    *aDocument = nsnull;
  }

  return rv;
}

nsresult
nsLocation::GetSourceBaseURL(JSContext* cx, nsIURI** sourceURL)
{
  nsCOMPtr<nsIDocument> doc;
  nsresult rv = GetSourceDocument(cx, getter_AddRefs(doc));
  if (doc) {
    NS_IF_ADDREF(*sourceURL = doc->GetBaseURI());
  } else {
    *sourceURL = nsnull;
  }

  return rv;
}
