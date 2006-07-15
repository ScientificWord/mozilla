/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1999
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
#include "nsICharsetAlias.h"
#include "nsXMLEncodingObserver.h"
#include "nsIXMLEncodingService.h"
#include "nsIElementObserver.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"
#include "nsISupports.h"
#include "nsCRT.h"
#include "nsIParser.h"
#include "pratom.h"
#include "nsCharDetDll.h"
#include "nsIServiceManager.h"
#include "nsObserverBase.h"
#include "nsWeakReference.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"

static NS_DEFINE_CID(kCharsetAliasCID, NS_CHARSETALIAS_CID);

static const eHTMLTags gTags[] = 
{ eHTMLTag_instruction,
  eHTMLTag_unknown
};

//-------------------------------------------------------------------------
nsXMLEncodingObserver::nsXMLEncodingObserver()
{
  bXMLEncodingObserverStarted = PR_FALSE;
}
//-------------------------------------------------------------------------
nsXMLEncodingObserver::~nsXMLEncodingObserver()
{
  // call to end the ObserverService
  if (bXMLEncodingObserverStarted == PR_TRUE) {
    End();
  }
}

//-------------------------------------------------------------------------
NS_IMPL_ADDREF ( nsXMLEncodingObserver )
NS_IMPL_RELEASE ( nsXMLEncodingObserver )

// Use the new scheme
NS_IMPL_QUERY_INTERFACE4(nsXMLEncodingObserver, 
                         nsIElementObserver, 
                         nsIObserver, 
                         nsIXMLEncodingService, 
                         nsISupportsWeakReference)

//-------------------------------------------------------------------------
NS_IMETHODIMP nsXMLEncodingObserver::Notify(
                     PRUint32 aDocumentID, 
                     const PRUnichar* aTag, 
                     PRUint32 numOfAttributes, 
                     const PRUnichar* nameArray[], 
                     const PRUnichar* valueArray[])
{
    if(!nsDependentString(aTag).LowerCaseEqualsLiteral("?xml")) 
        return NS_ERROR_ILLEGAL_VALUE;
    else
        return Notify(aDocumentID, numOfAttributes, nameArray, valueArray);
}
//-------------------------------------------------------------------------
NS_IMETHODIMP nsXMLEncodingObserver::Notify(
                     PRUint32 aDocumentID, 
                     eHTMLTags aTag, 
                     PRUint32 numOfAttributes, 
                     const PRUnichar* nameArray[], 
                     const PRUnichar* valueArray[])
{
    if(eHTMLTag_meta != aTag) 
        return NS_ERROR_ILLEGAL_VALUE;
    else 
        return Notify(aDocumentID, numOfAttributes, nameArray, valueArray);
}
//-------------------------------------------------------------------------
NS_IMETHODIMP nsXMLEncodingObserver::Notify(
                     PRUint32 aDocumentID, 
                     PRUint32 numOfAttributes, 
                     const PRUnichar* nameArray[], 
                     const PRUnichar* valueArray[])
{

    nsresult res = NS_OK;
    PRUint32 i;

    if(numOfAttributes >= 3)
    {
      PRBool bGotCurrentCharset=PR_FALSE;
      PRBool bGotCurrentCharsetSource = PR_FALSE;
      PRBool bGotEncoding = PR_FALSE;

      nsCAutoString currentCharset(NS_LITERAL_CSTRING("unknown"));
      nsAutoString charsetSourceStr(NS_LITERAL_STRING("unknown"));
      nsCAutoString encoding(NS_LITERAL_CSTRING("unknown"));

      for(i=0; i < numOfAttributes; i++) 
      {
         if(0==nsCRT::strcmp(nameArray[i], NS_LITERAL_STRING("charset").get())) 
         {
           bGotCurrentCharset = PR_TRUE;
           LossyCopyUTF16toASCII(nsDependentString(valueArray[i]), currentCharset);
         } else if(0==nsCRT::strcmp(nameArray[i], NS_LITERAL_STRING("charsetSource").get())) {
           bGotCurrentCharsetSource = PR_TRUE;
           charsetSourceStr = valueArray[i];
         } else if(nsDependentString(nameArray[i]).LowerCaseEqualsLiteral("encoding")) { 
           bGotEncoding = PR_TRUE;
           LossyCopyUTF16toASCII(nsDependentString(valueArray[i]), encoding);
         }
      }

      // if we cannot find currentCharset or currentCharsetSource
      // return error.
      if( ! (bGotCurrentCharset && bGotCurrentCharsetSource))
      {
         return NS_ERROR_ILLEGAL_VALUE;
      }

      PRInt32 err;
      PRInt32 charsetSourceInt = charsetSourceStr.ToInteger(&err);

      // if we cannot convert the string into PRInt32, return error
      if(NS_FAILED(err))
         return NS_ERROR_ILLEGAL_VALUE;

      PRInt32 currentCharsetSource = charsetSourceInt;

      if(kCharsetFromMetaTag > currentCharsetSource)
      {
           if(! encoding.Equals(currentCharset)) 
           {
              nsCOMPtr<nsICharsetAlias> calias = do_GetService(kCharsetAliasCID, &res);
               if(NS_SUCCEEDED(res) && (nsnull != calias) ) 
               {
                    PRBool same = PR_FALSE;
                    res = calias->Equals( encoding, currentCharset, &same);
                    if(NS_SUCCEEDED(res) && (! same))
                    {
                          nsCAutoString preferred;
                          res = calias->GetPreferred(encoding,
                                                     preferred);
                          if(NS_SUCCEEDED(res))
                          {
                            res = NotifyWebShell(0,0, preferred.get(), kCharsetFromMetaTag );
                            return res;
                          } // if check for GetPreferred
                    } // if check res for Equals
                } // if check res for GetService
            } // if Equals
       } // if 
    } // if 

    return NS_OK;
}

//-------------------------------------------------------------------------
NS_IMETHODIMP nsXMLEncodingObserver::Observe(nsISupports*, const char*, const PRUnichar*) 
{
    return NS_ERROR_NOT_IMPLEMENTED;
}
//-------------------------------------------------------------------------
NS_IMETHODIMP nsXMLEncodingObserver::Start() 
{
    nsresult res = NS_OK;

    if (bXMLEncodingObserverStarted == PR_TRUE) 
      return res;

    nsCOMPtr<nsIObserverService> anObserverService = do_GetService("@mozilla.org/observer-service;1", &res);

    if (NS_SUCCEEDED(res)) {
      res = anObserverService->AddObserver(this, "xmlparser", PR_TRUE);

      bXMLEncodingObserverStarted = PR_TRUE;
    }

    return res;
}
//-------------------------------------------------------------------------
NS_IMETHODIMP nsXMLEncodingObserver::End() 
{
    nsresult res = NS_OK;
    
    if (bXMLEncodingObserverStarted == PR_FALSE) 
      return res;

    nsCOMPtr<nsIObserverService> anObserverService = do_GetService("@mozilla.org/observer-service;1", &res);
    if (NS_SUCCEEDED(res)) {
      res = anObserverService->RemoveObserver(this, "xmlparser");

      bXMLEncodingObserverStarted = PR_FALSE;
    }

    return res;
}

