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

#ifndef nsDocShellTreeOwner_h__
#define nsDocShellTreeOwner_h__

// Helper Classes
#include "nsCOMPtr.h"
#include "nsString.h"

// Interfaces Needed
#include "nsIBaseWindow.h"
#include "nsIDocShellTreeOwner.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIWebBrowserChrome.h"
#include "nsIDOMMouseListener.h"
#include "nsIDOMDocument.h"
#include "nsIDOMEventTarget.h"
#include "nsIEmbeddingSiteWindow.h"
#include "nsIWebProgressListener.h"
#include "nsWeakReference.h"
#include "nsIDOMKeyListener.h"
#include "nsIDOMMouseMotionListener.h"
#include "nsIDOMContextMenuListener.h"
#include "nsITimer.h"
#include "nsIPrompt.h"
#include "nsIAuthPrompt.h"
#include "nsITooltipListener.h"
#include "nsITooltipTextProvider.h"
#include "nsCTooltipTextProvider.h"
#include "nsIDragDropHandler.h"
#include "nsPIDOMEventTarget.h"
#include "nsCommandHandler.h"

class nsWebBrowser;
class ChromeTooltipListener;
class ChromeContextMenuListener;

// {6D10C180-6888-11d4-952B-0020183BF181}
#define NS_ICDOCSHELLTREEOWNER_IID \
{ 0x6d10c180, 0x6888, 0x11d4, { 0x95, 0x2b, 0x0, 0x20, 0x18, 0x3b, 0xf1, 0x81 } }

/*
 * This is a fake 'hidden' interface that nsDocShellTreeOwner implements.
 * Classes such as nsCommandHandler can QI for this interface to be
 * sure that they're dealing with a valid nsDocShellTreeOwner and not some
 * other object that implements nsIDocShellTreeOwner.
 */
class nsICDocShellTreeOwner : public nsISupports
{
public:
    NS_DECLARE_STATIC_IID_ACCESSOR(NS_ICDOCSHELLTREEOWNER_IID)
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsICDocShellTreeOwner,
                              NS_ICDOCSHELLTREEOWNER_IID)

class nsDocShellTreeOwner : public nsIDocShellTreeOwner,
                            public nsIBaseWindow,
                            public nsIInterfaceRequestor,
                            public nsIWebProgressListener,
                            public nsICDocShellTreeOwner,
                            public nsSupportsWeakReference
{
friend class nsWebBrowser;
friend class nsCommandHandler;

public:
    NS_DECL_ISUPPORTS

    NS_DECL_NSIBASEWINDOW
    NS_DECL_NSIDOCSHELLTREEOWNER
    NS_DECL_NSIINTERFACEREQUESTOR
    NS_DECL_NSIWEBPROGRESSLISTENER

protected:
    nsDocShellTreeOwner();
    virtual ~nsDocShellTreeOwner();

    void WebBrowser(nsWebBrowser* aWebBrowser);
    
    nsWebBrowser* WebBrowser();
    NS_IMETHOD SetTreeOwner(nsIDocShellTreeOwner* aTreeOwner);
    NS_IMETHOD SetWebBrowserChrome(nsIWebBrowserChrome* aWebBrowserChrome);

    NS_IMETHOD AddChromeListeners();
    NS_IMETHOD RemoveChromeListeners();

    nsresult   FindChildWithName(const PRUnichar *aName, 
                 PRBool aRecurse, nsIDocShellTreeItem* aRequestor,
                 nsIDocShellTreeItem* aOriginalRequestor,
                 nsIDocShellTreeItem **aFoundItem);
    nsresult   FindItemWithNameAcrossWindows(const PRUnichar* aName,
                 nsIDocShellTreeItem* aRequestor,
                 nsIDocShellTreeItem* aOriginalRequestor,
                 nsIDocShellTreeItem **aFoundItem);

    void       EnsurePrompter();
    void       EnsureAuthPrompter();

    void AddToWatcher();
    void RemoveFromWatcher();

    // These helper functions return the correct instances of the requested
    // interfaces.  If the object passed to SetWebBrowserChrome() implements
    // nsISupportsWeakReference, then these functions call QueryReferent on
    // that object.  Otherwise, they return an addrefed pointer.  If the
    // WebBrowserChrome object doesn't exist, they return nsnull.
    already_AddRefed<nsIWebBrowserChrome>     GetWebBrowserChrome();
    already_AddRefed<nsIEmbeddingSiteWindow>  GetOwnerWin();
    already_AddRefed<nsIInterfaceRequestor>   GetOwnerRequestor();

protected:

   // Weak References
   nsWebBrowser*           mWebBrowser;
   nsIDocShellTreeOwner*   mTreeOwner;
   nsIDocShellTreeItem*    mPrimaryContentShell; 

   nsIWebBrowserChrome*    mWebBrowserChrome;
   nsIEmbeddingSiteWindow* mOwnerWin;
   nsIInterfaceRequestor*  mOwnerRequestor;

   nsWeakPtr               mWebBrowserChromeWeak;   // nsIWebBrowserChrome

    // the objects that listen for chrome events like context menus and tooltips. 
    // They are separate objects to avoid circular references between |this|
    // and the DOM. These are strong, owning refs.
   ChromeTooltipListener*         mChromeTooltipListener;
   ChromeContextMenuListener*     mChromeContextMenuListener;
   nsCOMPtr<nsIDragDropHandler>   mChromeDragHandler;

   nsCOMPtr<nsIPrompt>     mPrompter;
   nsCOMPtr<nsIAuthPrompt> mAuthPrompter;
};


//
// class ChromeTooltipListener
//
// The class that listens to the chrome events and tells the embedding
// chrome to show tooltips, as appropriate. Handles registering itself
// with the DOM with AddChromeListeners() and removing itself with
// RemoveChromeListeners().
//
class ChromeTooltipListener : public nsIDOMMouseListener,
                                public nsIDOMKeyListener,
                                public nsIDOMMouseMotionListener
{
public:
  NS_DECL_ISUPPORTS
  
  ChromeTooltipListener ( nsWebBrowser* inBrowser, nsIWebBrowserChrome* inChrome ) ;
  virtual ~ChromeTooltipListener ( ) ;

    // nsIDOMMouseListener
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent) {	return NS_OK; }
  NS_IMETHOD MouseDown(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseUp(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseClick(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseDblClick(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseOver(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD MouseOut(nsIDOMEvent* aMouseEvent);

    // nsIDOMMouseMotionListener
  NS_IMETHOD MouseMove(nsIDOMEvent* aMouseEvent);
  NS_IMETHOD DragMove(nsIDOMEvent* aMouseEvent) { return NS_OK; }

    // nsIDOMKeyListener
  NS_IMETHOD KeyDown(nsIDOMEvent* aKeyEvent) ;
  NS_IMETHOD KeyUp(nsIDOMEvent* aKeyEvent) ;
  NS_IMETHOD KeyPress(nsIDOMEvent* aKeyEvent) ;

    // Add/remove the relevant listeners, based on what interfaces
    // the embedding chrome implements.
  NS_IMETHOD AddChromeListeners();
  NS_IMETHOD RemoveChromeListeners();

private:

    // various delays for tooltips
  enum {
    kTooltipAutoHideTime = 5000,       // 5000ms = 5 seconds
    kTooltipShowTime = 500             // 500ms = 0.5 seconds
  };

  NS_IMETHOD AddTooltipListener();
  NS_IMETHOD RemoveTooltipListener();

  NS_IMETHOD ShowTooltip ( PRInt32 inXCoords, PRInt32 inYCoords, const nsAString & inTipText ) ;
  NS_IMETHOD HideTooltip ( ) ;

  nsWebBrowser* mWebBrowser;
  nsCOMPtr<nsPIDOMEventTarget> mEventTarget;
  nsCOMPtr<nsITooltipTextProvider> mTooltipTextProvider;
  
    // This must be a strong ref in order to make sure we can hide the tooltip
    // if the window goes away while we're displaying one. If we don't hold
    // a strong ref, the chrome might have been disposed of before we get a chance
    // to tell it, and no one would ever tell us of that fact.
  nsCOMPtr<nsIWebBrowserChrome> mWebBrowserChrome;

  PRPackedBool mTooltipListenerInstalled;

  nsCOMPtr<nsITimer> mTooltipTimer;
  static void sTooltipCallback ( nsITimer* aTimer, void* aListener ) ;
  PRInt32 mMouseClientX, mMouseClientY;       // mouse coordinates for last mousemove event we saw
  PRInt32 mMouseScreenX, mMouseScreenY;       // mouse coordinates for tooltip event
  PRBool mShowingTooltip;

    // a timer for auto-hiding the tooltip after a certain delay
  nsCOMPtr<nsITimer> mAutoHideTimer;
  static void sAutoHideCallback ( nsITimer* aTimer, void* aListener ) ;
  void CreateAutoHideTimer ( ) ;

    // The node hovered over that fired the timer. This may turn into the node that
    // triggered the tooltip, but only if the timer ever gets around to firing.
    // This is a strong reference, because the tooltip content can be destroyed while we're
    // waiting for the tooltip to pup up, and we need to detect that.
    // It's set only when the tooltip timer is created and launched. The timer must
    // either fire or be cancelled (or possibly released?), and we release this
    // reference in each of those cases. So we don't leak.
  nsCOMPtr<nsIDOMNode> mPossibleTooltipNode;

}; // ChromeTooltipListener


//
// class ChromeContextMenuListener
//
// The class that listens to the chrome events and tells the embedding
// chrome to show context menus, as appropriate. Handles registering itself
// with the DOM with AddChromeListeners() and removing itself with
// RemoveChromeListeners().
//
class ChromeContextMenuListener : public nsIDOMContextMenuListener
{
public:
  NS_DECL_ISUPPORTS
  
  ChromeContextMenuListener ( nsWebBrowser* inBrowser, nsIWebBrowserChrome* inChrome ) ;
  virtual ~ChromeContextMenuListener ( ) ;

  // nsIDOMContextMenuListener
  NS_IMETHOD HandleEvent(nsIDOMEvent* aEvent) {	return NS_OK; }
  NS_IMETHOD ContextMenu ( nsIDOMEvent* aEvent );

  // Add/remove the relevant listeners, based on what interfaces
  // the embedding chrome implements.
  NS_IMETHOD AddChromeListeners();
  NS_IMETHOD RemoveChromeListeners();

private:

  NS_IMETHOD AddContextMenuListener();
  NS_IMETHOD RemoveContextMenuListener();

  PRPackedBool mContextMenuListenerInstalled;

  nsWebBrowser* mWebBrowser;
  nsCOMPtr<nsPIDOMEventTarget> mEventTarget;
  nsCOMPtr<nsIWebBrowserChrome> mWebBrowserChrome;

}; // class ChromeContextMenuListener



#endif /* nsDocShellTreeOwner_h__ */












