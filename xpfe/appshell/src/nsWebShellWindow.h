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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#ifndef nsWebShellWindow_h__
#define nsWebShellWindow_h__

#include "nsGUIEvent.h"
#include "nsIWebProgressListener.h"
#include "nsITimer.h"

// can't use forward class decl's because of template bugs on Solaris 
#include "nsIDOMDocument.h"

#include "nsCOMPtr.h"
#include "nsXULWindow.h"

/* Forward declarations.... */
class nsIURI;
class nsIAppShell;

class nsWebShellWindow : public nsXULWindow,
                         public nsIWebProgressListener
{
public:
  nsWebShellWindow();

  // nsISupports interface...
  NS_DECL_ISUPPORTS_INHERITED

  // nsWebShellWindow methods...
  nsresult Initialize(nsIXULWindow * aParent, nsIAppShell* aShell,
                      nsIURI* aUrl,
                      PRInt32 aInitialWidth, PRInt32 aInitialHeight,
                      PRBool aIsHiddenWindow,
                      nsWidgetInitData& widgetInitData);

  nsresult Toolbar();

  // nsIWebProgressListener
  NS_DECL_NSIWEBPROGRESSLISTENER

  // nsIBaseWindow
  NS_IMETHOD Destroy();

protected:
  
  virtual ~nsWebShellWindow();

  nsCOMPtr<nsIDOMDocument> GetNamedDOMDoc(const nsAString & aWebShellName);

  void                     LoadContentAreas();
  PRBool                   ExecuteCloseHandler();

  static nsEventStatus PR_CALLBACK HandleEvent(nsGUIEvent *aEvent);

  nsCOMPtr<nsITimer>      mSPTimer;
  PRLock *                mSPTimerLock;

  void        SetPersistenceTimer(PRUint32 aDirtyFlags);
  static void FirePersistenceTimer(nsITimer *aTimer, void *aClosure);
};


#endif /* nsWebShellWindow_h__ */
