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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Daniel Veditz <dveditz@netscape.com>
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

#include "nsString.h"
#include "nsStubNotifier.h"

extern PRInt32 gInstallStatus;

nsStubListener::nsStubListener( pfnXPIProgress aProgress )
    : m_progress(aProgress)
{
}

nsStubListener::~nsStubListener()
{}

NS_IMPL_ISUPPORTS1(nsStubListener, nsIXPIListener)


NS_IMETHODIMP
nsStubListener::OnInstallStart(const PRUnichar *URL)
{
    // we're not interested in this one
    return NS_OK;
}

NS_IMETHODIMP
nsStubListener::OnPackageNameSet(const PRUnichar *URL, const PRUnichar* UIPackageName, const PRUnichar* aVersion)
{
    // we're not interested in this one
    return NS_OK;
}

NS_IMETHODIMP
nsStubListener::OnItemScheduled(const PRUnichar* message )
{
    if (m_progress)
      {
        m_progress( NS_LossyConvertUTF16toASCII(message).get(), 0, 0 );
      }
    return NS_OK;
}

NS_IMETHODIMP
nsStubListener::OnFinalizeProgress(const PRUnichar* message, PRInt32 itemNum, PRInt32 totNum )
{
    if (m_progress)
      {
        m_progress( NS_LossyConvertUTF16toASCII(message).get(), itemNum, totNum );
      }
    return NS_OK;
}

NS_IMETHODIMP
nsStubListener::OnInstallDone(const PRUnichar *URL, PRInt32 status)
{
//    if (m_final)
//        m_final( nsCAutoString(URL), status );
    gInstallStatus = status;
    return NS_OK;
}

NS_IMETHODIMP
nsStubListener::OnLogComment(const PRUnichar* comment)
{
    // we're not interested in this one
    return NS_OK;
}
