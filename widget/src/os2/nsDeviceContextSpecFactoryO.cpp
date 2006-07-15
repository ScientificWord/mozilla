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
 *   John Fairhurst <john_fairhurst@iname.com>
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

#include "nsDeviceContextSpecFactoryO.h"
#include "nsDeviceContextSpecOS2.h"
#include "nsComponentManagerUtils.h"
#include "nsWidgetsCID.h"

nsDeviceContextSpecFactoryOS2 :: nsDeviceContextSpecFactoryOS2()
{
}

nsDeviceContextSpecFactoryOS2 :: ~nsDeviceContextSpecFactoryOS2()
{
}

NS_IMPL_ISUPPORTS1(nsDeviceContextSpecFactoryOS2, nsIDeviceContextSpecFactory)

NS_IMETHODIMP nsDeviceContextSpecFactoryOS2 :: Init(void)
{
  return NS_OK;
}

//XXX this method needs to do what the API says...

NS_IMETHODIMP nsDeviceContextSpecFactoryOS2 :: CreateDeviceContextSpec(nsIWidget *aWidget,
                                                                       nsIPrintSettings* aPrintSettings,
                                                                       nsIDeviceContextSpec *&aNewSpec,
                                                                       PRBool aIsPrintPreview)
{
     nsresult	rv;
     static NS_DEFINE_CID(kDeviceContextSpecCID, NS_DEVICE_CONTEXT_SPEC_CID);
     nsCOMPtr<nsIDeviceContextSpec> devSpec = do_CreateInstance(kDeviceContextSpecCID, &rv);

	if (NS_SUCCEEDED(rv)) {
	  rv = ((nsDeviceContextSpecOS2 *)devSpec.get())->Init(aPrintSettings, aIsPrintPreview);
       if (NS_SUCCEEDED(rv)) {
	    aNewSpec = devSpec;
         NS_ADDREF(aNewSpec);
	  }
	}
	return rv;
}
