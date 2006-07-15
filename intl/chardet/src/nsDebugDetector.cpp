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

#include "nsDebugDetector.h"
#include "pratom.h"
#include "nsCharDetDll.h"


#define REPORT_CHARSET "ISO-8859-7"
#define REPORT_CONFIDENT  eSureAnswer

//--------------------------------------------------------------------
nsDebugDetector::nsDebugDetector( nsDebugDetectorSel aSel)
{
  mSel = aSel;
  mBlks = 0;
  mObserver = nsnull;
  mStop = PR_FALSE;
}
//--------------------------------------------------------------------
nsDebugDetector::~nsDebugDetector()
{
}
//--------------------------------------------------------------------
NS_IMETHODIMP nsDebugDetector::Init(nsICharsetDetectionObserver* aObserver)
{
  NS_ASSERTION(mObserver == nsnull , "Init twice");
  if(nsnull == aObserver)
     return NS_ERROR_ILLEGAL_VALUE;

  mObserver = aObserver;
  return NS_OK;
}
//--------------------------------------------------------------------

NS_IMETHODIMP nsDebugDetector::DoIt(const char* aBytesArray, PRUint32 aLen, PRBool* oDontFeedMe)
{
  NS_ASSERTION(mObserver != nsnull , "have not init yet");
  NS_ASSERTION(mStop == PR_FALSE , "don't call DoIt if we return PR_TRUE in oDontFeedMe");

  if((nsnull == aBytesArray) || (nsnull == oDontFeedMe))
     return NS_ERROR_ILLEGAL_VALUE;

  mBlks++;
  if((k1stBlk == mSel) && (1 == mBlks)) {
     *oDontFeedMe = mStop = PR_TRUE;
     Report();
  } else if((k2ndBlk == mSel) && (2 == mBlks)) {
     *oDontFeedMe = mStop = PR_TRUE;
     Report();
  } else {
     *oDontFeedMe = mStop = PR_FALSE;
  }
   
  return NS_OK;
}

//--------------------------------------------------------------------
NS_IMETHODIMP nsDebugDetector::Done()
{
  NS_ASSERTION(mObserver != nsnull , "have not init yet");
  if(klastBlk == mSel)
     Report();
  return NS_OK;
}
//--------------------------------------------------------------------
void nsDebugDetector::Report()
{
  mObserver->Notify( REPORT_CHARSET, REPORT_CONFIDENT);
}


NS_IMPL_ISUPPORTS1(nsDebugDetector, nsICharsetDetector)

