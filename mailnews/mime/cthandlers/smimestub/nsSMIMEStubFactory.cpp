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

#include "nsIGenericFactory.h"

/* Include all of the interfaces our factory can generate components for */
#include "nsSMIMEStub.h"
#include "nsMimeContentTypeHandler.h"

////////////////////////////////////////////////////////////////////////
// Define the contructor function for the CID
//
// What this does is defines a function nsMimeContentTypeHandlerConstructor
// which we will specific in the nsModuleComponentInfo table. This function will
// be used by the generic factory to create an instance.
//
// NOTE: This creates an instance by using the default constructor
//
//NS_GENERIC_FACTORY_CONSTRUCTOR(nsMimeContentTypeHandler)
extern "C" MimeObjectClass *
MIME_SMimeCreateContentTypeHandlerClass(const char *content_type, 
                                        contentTypeHandlerInitStruct *initStruct);

static NS_IMETHODIMP
nsSMimeMimeContentTypeHandlerConstructor(nsISupports *aOuter,
                                         REFNSIID aIID,
                                         void **aResult)
{
  nsresult rv;
  nsMimeContentTypeHandler *inst = nsnull;

  if (NULL == aResult) {
    rv = NS_ERROR_NULL_POINTER;
    return rv;
  }
  *aResult = NULL;
  if (NULL != aOuter) {
    rv = NS_ERROR_NO_AGGREGATION;
    return rv;
  }
  inst = new nsMimeContentTypeHandler(SMIME_CONTENT_TYPE, 
                                      &MIME_SMimeCreateContentTypeHandlerClass);
  if (inst == NULL) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  NS_ADDREF(inst);
  rv = inst->QueryInterface(aIID,aResult);
  NS_RELEASE(inst);

  return rv;
}

////////////////////////////////////////////////////////////////////////
// Define a table of CIDs implemented by this module along with other
// information like the function to create an instance, contractid, and
// class name.
//
static const nsModuleComponentInfo components[] =
{
  { "MIME SMIMEStubed Mail Handler", NS_SMIME_CONTENT_TYPE_HANDLER_CID, "@mozilla.org/mimecth;1?type=application/x-pkcs7-mime",
    nsSMimeMimeContentTypeHandlerConstructor, },

  { "MIME SMIMEStubed Mail Handler", NS_SMIME_CONTENT_TYPE_HANDLER_CID, "@mozilla.org/mimecth;1?type=application/pkcs7-mime",
     nsSMimeMimeContentTypeHandlerConstructor, }


};

////////////////////////////////////////////////////////////////////////
// Implement the NSGetModule() exported function for your module
// and the entire implementation of the module object.
//
NS_IMPL_NSGETMODULE(nsSMIMEModule, components)
