/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 *   Alec Flett <alecf@netscape.com>
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

const mediatorContractId = "@mozilla.org/appshell/window-mediator;1";
const nsIWebBrowserChrome = Components.interfaces.nsIWebBrowserChrome;

function nsBrowserContentListener(toplevelWindow, contentWindow)
{
    // this one is not as easy as you would hope.
    // need to convert toplevelWindow to an XPConnected object, instead
    // of a DOM-based object, to be able to QI() it to nsIXULWindow

    this.init(toplevelWindow, contentWindow);
}

/* implements nsIURIContentListener */

nsBrowserContentListener.prototype =
{
    init: function(toplevelWindow, contentWindow)
    {
        this.toplevelWindow = toplevelWindow;
        this.contentWindow = contentWindow;

        // hook up the whole parent chain thing
        var windowDocShell = this.convertWindowToDocShell(toplevelWindow);
        if (windowDocShell) {
            windowDocshell
              .QueryInterface(Components.interfaces.nsIInterfaceRequestor)
              .getInterface(Components.interfaces.nsIURIContentListener)
              .parentContentListener = this;
        }

        var registerWindow = false;
        try {
          var treeItem = contentWindow.docShell.QueryInterface(Components.interfaces.nsIDocShellTreeItem);
          var treeOwner = treeItem.treeOwner;
          var interfaceRequestor = treeOwner.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
          var webBrowserChrome = interfaceRequestor.getInterface(nsIWebBrowserChrome);
          if (webBrowserChrome)
          {
            var chromeFlags = webBrowserChrome.chromeFlags;
            var res = chromeFlags & nsIWebBrowserChrome.CHROME_ALL;
            var res2 = chromeFlags & nsIWebBrowserChrome.CHROME_DEFAULT;
            if ( res == nsIWebBrowserChrome.CHROME_ALL || res2 == nsIWebBrowserChrome.CHROME_DEFAULT)
            {
              registerWindow = true;
            }
         }
       } catch (ex) {}

        // register ourselves
       if (registerWindow)
       {
        var uriLoader = Components.classes["@mozilla.org/uriloader;1"].getService(Components.interfaces.nsIURILoader);
        uriLoader.registerContentListener(this);
       }
    },
    close: function()
    {
        this.contentWindow = null;
        var uriLoader = Components.classes["@mozilla.org/uriloader;1"].getService(Components.interfaces.nsIURILoader);

        uriLoader.unRegisterContentListener(this);
    },
    QueryInterface: function(iid)
    {
        if (iid.equals(Components.interfaces.nsIURIContentListener) ||
            iid.equals(Components.interfaces.nsISupportsWeakReference) ||
            iid.equals(Components.interfaces.nsISupports))
            return this;

        Components.returnCode = Components.results.NS_ERROR_NO_INTERFACE;
        return null;
    },
    onStartURIOpen: function(uri)
    {
        // ignore and don't abort
        return false;
    },

    doContent: function(contentType, isContentPreferred, request, contentHandler)
    {
        // forward the doContent to our content area webshell
        var docShell = this.contentWindow.docShell;
        var contentListener;
        try {
            contentListener =
                docShell.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
                .getInterface(Components.interfaces.nsIURIContentListener);
        } catch (ex) {
            dump(ex);
        }

        if (!contentListener) return false;

        return contentListener.doContent(contentType, isContentPreferred, request, contentHandler);

    },

    isPreferred: function(contentType, desiredContentType)
    {
        try {
            var webNavInfo = 
              Components.classes["@mozilla.org/webnavigation-info;1"]
                        .getService(Components.interfaces.nsIWebNavigationInfo);
            return webNavInfo.isTypeSupported(contentType, null);
        } catch (e) {
            // XXX propagate failures other than "NS_ERROR_NOT_AVAILABLE"?
            // This seems to never get called, so not like it matters....
            return false;
        }
    },
    canHandleContent: function(contentType, isContentPreferred, desiredContentType)
    {
        var docShell = this.contentWindow.docShell;
        var contentListener;
        try {
            contentListener =
                docShell.QueryInterface(Components.interfaces.nsIInterfaceRequestor).getInterface(Components.interfaces.nsIURIContentListener);
        } catch (ex) {
            dump(ex);
        }
        if (!contentListener) return false;

        return contentListener.canHandleContent(contentType, isContentPreferred, desiredContentType);
    },
    convertWindowToDocShell: function(win) {
        // don't know how to do this
        return null;
    },
    loadCookie: null,
    parentContentListener: null
}
