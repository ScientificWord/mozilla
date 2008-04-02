/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the Mozilla browser.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications, Inc.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Travis Bogard <travis@netscape.com>
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

#ifndef nsWebBrowser_h__
#define nsWebBrowser_h__

// Local Includes
#include "nsDocShellTreeOwner.h"

// Core Includes
#include "nsCOMPtr.h"

// Interfaces needed
#include "nsCWebBrowser.h"
#include "nsIBaseWindow.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIDocShellTreeNode.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIScrollable.h"
#include "nsISHistory.h"
#include "nsITextScroll.h"
#include "nsIWidget.h"
#include "nsIWebProgress.h"
#include "nsISecureBrowserUI.h"
#include "nsIWebBrowser.h"
#include "nsIWebNavigation.h"
#include "nsIWebBrowserSetup.h"
#include "nsIWebBrowserPersist.h"
#include "nsIWebBrowserFocus.h"
#include "nsIWebBrowserStream.h"
#include "nsIWindowWatcher.h"
#include "nsIPrintSettings.h"
#include "nsEmbedStream.h"

#include "nsVoidArray.h"
#include "nsWeakPtr.h"

class nsIContentViewerFile;

class nsWebBrowserInitInfo
{
public:
   //nsIBaseWindow Stuff
   PRInt32                 x;
   PRInt32                 y;
   PRInt32                 cx;
   PRInt32                 cy;
   PRBool                  visible;
   nsCOMPtr<nsISHistory>   sessionHistory;
   nsString                name;
};

class nsWebBrowserListenerState
{
public:
    PRBool Equals(nsIWeakReference *aListener, const nsIID& aID) {
        if (mWeakPtr.get() == aListener && mID.Equals(aID)) return PR_TRUE;
        return PR_FALSE;
    }

    nsWeakPtr mWeakPtr;
    nsIID mID;
};

//  {F1EAC761-87E9-11d3-AF80-00A024FFC08C} - 
#define NS_WEBBROWSER_CID \
{0xf1eac761, 0x87e9, 0x11d3, { 0xaf, 0x80, 0x00, 0xa0, 0x24, 0xff, 0xc0, 0x8c }}


class nsWebBrowser : public nsIWebBrowser,
                     public nsIWebNavigation,
                     public nsIWebBrowserSetup,
                     public nsIDocShellTreeItem,
                     public nsIBaseWindow,
                     public nsIScrollable, 
                     public nsITextScroll, 
                     public nsIInterfaceRequestor,
                     public nsIWebBrowserPersist,
                     public nsIWebBrowserFocus,
                     public nsIWebProgressListener,
                     public nsIWebBrowserStream,
                     public nsSupportsWeakReference
{
friend class nsDocShellTreeOwner;
public:
    nsWebBrowser();

    NS_DECL_ISUPPORTS

    NS_DECL_NSIBASEWINDOW
    NS_DECL_NSIDOCSHELLTREEITEM
    NS_DECL_NSIDOCSHELLTREENODE
    NS_DECL_NSIINTERFACEREQUESTOR
    NS_DECL_NSISCROLLABLE   
    NS_DECL_NSITEXTSCROLL
    NS_DECL_NSIWEBBROWSER
    NS_DECL_NSIWEBNAVIGATION
    NS_DECL_NSIWEBBROWSERSETUP
    NS_DECL_NSIWEBBROWSERPERSIST
    NS_DECL_NSICANCELABLE
    NS_DECL_NSIWEBBROWSERFOCUS
    NS_DECL_NSIWEBBROWSERSTREAM
    NS_DECL_NSIWEBPROGRESSLISTENER

protected:
    virtual ~nsWebBrowser();
    NS_IMETHOD InternalDestroy();

    NS_IMETHOD SetDocShell(nsIDocShell* aDocShell);
    NS_IMETHOD EnsureDocShellTreeOwner();
    NS_IMETHOD GetPrimaryContentWindow(nsIDOMWindowInternal **aDomWindow);
    NS_IMETHOD BindListener(nsISupports *aListener, const nsIID& aIID);
    NS_IMETHOD UnBindListener(nsISupports *aListener, const nsIID& aIID);
    NS_IMETHOD EnableGlobalHistory(PRBool aEnable);

    static nsEventStatus PR_CALLBACK HandleEvent(nsGUIEvent *aEvent);

protected:
   nsDocShellTreeOwner*       mDocShellTreeOwner;
   nsCOMPtr<nsIDocShell>      mDocShell;
   nsCOMPtr<nsIInterfaceRequestor> mDocShellAsReq;
   nsCOMPtr<nsIBaseWindow>    mDocShellAsWin;
   nsCOMPtr<nsIDocShellTreeItem> mDocShellAsItem;
   nsCOMPtr<nsIWebNavigation> mDocShellAsNav;
   nsCOMPtr<nsIScrollable>    mDocShellAsScrollable;
   nsCOMPtr<nsITextScroll>    mDocShellAsTextScroll;
   nsCOMPtr<nsIWidget>        mInternalWidget;
   nsCOMPtr<nsIWindowWatcher> mWWatch;
   nsWebBrowserInitInfo*      mInitInfo;
   PRUint32                   mContentType;
   PRPackedBool               mActivating;
   PRPackedBool               mShouldEnableHistory;
   nativeWindow               mParentNativeWindow;
   nsIWebProgressListener    *mProgressListener;
   nsCOMPtr<nsIWebProgress>      mWebProgress;

   nsCOMPtr<nsIPrintSettings> mPrintSettings;

   // cached background color
   nscolor                       mBackgroundColor;

   // persistence object
   nsCOMPtr<nsIWebBrowserPersist> mPersist;
   PRUint32                       mPersistCurrentState;
   PRUint32                       mPersistResult;
   PRUint32                       mPersistFlags;

   // stream
   nsEmbedStream                 *mStream;
   nsCOMPtr<nsISupports>          mStreamGuard;

   //Weak Reference interfaces...
   nsIWidget*                 mParentWidget;
   nsVoidArray *              mListenerArray;
};

#endif /* nsWebBrowser_h__ */


