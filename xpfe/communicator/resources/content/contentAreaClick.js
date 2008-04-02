/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 *   Alec Flett      <alecf@netscape.com>
 *   Ben Goodger     <ben@netscape.com>
 *   Mike Pinkerton  <pinkerton@netscape.com>
 *   Blake Ross      <blakeross@telocity.com>
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

/*
 * - [ Dependencies ] ---------------------------------------------------------
 *  utilityOverlay.js:
 *    - gatherTextUnder
 *    - startScrolling
 */

  var pref = null;
  pref = Components.classes["@mozilla.org/preferences-service;1"]
                   .getService(Components.interfaces.nsIPrefBranch);

  // Prefill a single text field
  function prefillTextBox(target) {
    // obtain values to be used for prefilling
    var walletService = Components.classes["@mozilla.org/wallet/wallet-service;1"].getService(Components.interfaces.nsIWalletService);
    var value = walletService.WALLET_PrefillOneElement(window.content, target);
    if (value) {

      // result is a linear sequence of values, each preceded by a separator character
      // convert linear sequence of values into an array of values
      var separator = value[0];
      var valueList = value.substring(1, value.length).split(separator);

      target.value = valueList[0];
/*
 * Following code is a replacement for above line.  In the case of multiple values, it
 * presents the user with a dialog containing a list from which he can select the value
 * he wants.  However it is being commented out for now because of several problems, namely
 *
 *   1. There is no easy way to put localizable strings for the title and message of
 *      the dialog without introducing a .properties file which currently doesn't exist
 *   2. Using blank title and messages as shown below have a problem because a zero-length
 *      title is being displayed as some garbage characters (which is why the code below
 *      has a title of " " instead of "").  This is a separate bug which will have to be
 *      investigated further.
 *   3. The current wallet tables present alternate values for items such as shipping
 *      address -- namely billing address and home address.  Up until now, these alternate
 *      values have never been a problem because the preferred value is always first and is
 *      all the user sees when doing a prefill.  However now he will be presented with a
 *      list showing all these values and asking him to pick one, even though the wallet
 *      code was clearly able to determine that he meant shipping address and not billing
 *      address.
 *   4. There is a relatively long delay before the dialog come up whereas values are
 *      filled in quickly when no dialog is involved.
 *
 * Once this feature is checked in, a separate bug will be opened asking that the above
 * problems be examined and this dialog turned on
 
      if (valueList.length == 1) {
        // only one value, use it for prefilling
        target.value = valueList[0];
      } else {

        // more than one value, have user select the one he wants
        var promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService();
        promptService = promptService.QueryInterface(Components.interfaces.nsIPromptService);
        var position = {};
        var title = " ";
        var message = "";
        var ok =
          promptService.select
            (window, title, message, valueList.length, valueList, position)
        if (ok) {
          target.value = valueList[position.value];
        }
      }

 * End of commented out code
 */
    }
  }
  
  function hrefAndLinkNodeForClickEvent(event)
  {
    var target = event.target;
    var href = "";
    var linkNode = null;

    var isKeyPress = (event.type == "keypress");

    if ( target instanceof HTMLAnchorElement ||
         target instanceof HTMLAreaElement   ||
         target instanceof HTMLLinkElement ) {
      if (target.hasAttribute("href")) 
        linkNode = target;
    }
    else if ( target instanceof HTMLInputElement
            && (event.target.type == "text") // text field
            && !isKeyPress       // not a key event
            && event.detail == 2 // double click
            && event.button == 0 // left mouse button
            && event.target.value.length == 0 // no text has been entered
            && "@mozilla.org/wallet/wallet-service;1" in Components.classes // wallet is available
    ) {
      prefillTextBox(target); // prefill the empty text field if possible
    }
    else {
      linkNode = event.originalTarget;
      while (linkNode && !(linkNode instanceof HTMLAnchorElement))
        linkNode = linkNode.parentNode;
      // <a> cannot be nested.  So if we find an anchor without an
      // href, there is no useful <a> around the target
      if (linkNode && !linkNode.hasAttribute("href"))
        linkNode = null;
    }

    if (linkNode) {
      href = linkNode.href;
    } else {
      // Try simple XLink
      linkNode = target;
      while (linkNode) {
        if (linkNode.nodeType == Node.ELEMENT_NODE) {
          href = linkNode.getAttributeNS("http://www.w3.org/1999/xlink", "href");
          break;
        }
        linkNode = linkNode.parentNode;
      }
      if (href) {
        href = makeURLAbsolute(linkNode.baseURI, href);
      }
    }

    return href ? {href: href, linkNode: linkNode} : null;
  }

  // Called whenever the user clicks in the content area,
  // except when left-clicking on links (special case)
  // should always return true for click to go through
  function contentAreaClick(event) 
  {
    if (!event.isTrusted || event.getPreventDefault()) {
      return true;
    }

    var isKeyPress = (event.type == "keypress");
    var ceParams = hrefAndLinkNodeForClickEvent(event);
    if (ceParams) {
      var href = ceParams.href;
      if (isKeyPress) {
        openNewTabWith(href, true, event.shiftKey);
        event.stopPropagation();
      }
      else {
        handleLinkClick(event, href, ceParams.linkNode);
        // if in mailnews block the link left click if we determine
        // that this URL is phishy (i.e. a potential email scam) 
        if ("gMessengerBundle" in this && !event.button)
          return !isPhishingURL(ceParams.linkNode, false, href);
      }
      return true;
    }

    if (pref && !isKeyPress && event.button == 1 &&
        pref.getBoolPref("middlemouse.contentLoadURL")) {
      if (middleMousePaste(event)) {
        event.stopPropagation();
      }
    }
    return true;
  }

  function contentAreaMouseDown(event)
  {
    if (event.button == 1 && (event.target != event.currentTarget)
        && !hrefAndLinkNodeForClickEvent(event)
        && !isAutoscrollBlocker(event.originalTarget)) {
      startScrolling(event);
      return false;
    }
    return true;
  }

  function isAutoscrollBlocker(node)
  {
    if (!pref)
      return false;
    
    if (pref.getBoolPref("middlemouse.contentLoadURL"))
      return true;
    
    if (!pref.getBoolPref("middlemouse.paste"))
      return false;
    
    if (node.ownerDocument.designMode == "on")
      return true;
    
    while (node) {
      if (node instanceof HTMLTextAreaElement ||
          (node instanceof HTMLInputElement &&
           (node.type == "text" || node.type == "password")))
        return true;
      
      node = node.parentNode;
    }
    return false;
  }

  function openNewTabOrWindow(event, href, sendReferrer)
  {
    // should we open it in a new tab?
    if (pref && pref.getBoolPref("browser.tabs.opentabfor.middleclick")) {
      openNewTabWith(href, sendReferrer, event.shiftKey);
      event.stopPropagation();
      return true;
    }

    // should we open it in a new window?
    if (pref && pref.getBoolPref("middlemouse.openNewWindow")) {
      openNewWindowWith(href, sendReferrer);
      event.stopPropagation();
      return true;
    }

    // let someone else deal with it
    return false;
  }

  function handleLinkClick(event, href, linkNode)
  {
    // Make sure we are allowed to open this URL
    urlSecurityCheck(href, document);

    switch (event.button) {                                   
      case 0:                                                         // if left button clicked
        if (event.metaKey || event.ctrlKey) {                         // and meta or ctrl are down
          if (openNewTabOrWindow(event, href, true))
            return true;
        } 
        var saveModifier = true;
        if (pref) {
          try {
            saveModifier = pref.getBoolPref("ui.key.saveLink.shift");
          }
          catch(ex) {            
          }
        }
        saveModifier = saveModifier ? event.shiftKey : event.altKey;
          
        if (saveModifier) {                                           // if saveModifier is down
          saveURL(href, linkNode ? gatherTextUnder(linkNode) : "",
                  "SaveLinkTitle", false, getReferrer(document));
          return true;
        }
        if (event.altKey)                                             // if alt is down
          return true;                                                // do nothing
        return false;
      case 1:                                                         // if middle button clicked
        if (openNewTabOrWindow(event, href, true))
          return true;
        break;
    }
    return false;
  }

  function middleMousePaste( event )
  {
    var url = readFromClipboard();
    if (!url)
      return false;
    addToUrlbarHistory(url);
    url = getShortcutOrURI(url);

    // On ctrl-middleclick, open in new window or tab.  Do not send referrer.
    if (event.ctrlKey) {
      // fix up our pasted URI in case it is malformed.
      const nsIURIFixup = Components.interfaces.nsIURIFixup;
      if (!gURIFixup)
        gURIFixup = Components.classes["@mozilla.org/docshell/urifixup;1"]
                              .getService(nsIURIFixup);

      url = gURIFixup.createFixupURI(url, nsIURIFixup.FIXUP_FLAGS_MAKE_ALTERNATE_URI).spec;

      return openNewTabOrWindow(event, url, false);
    }

    // If ctrl wasn't down, then just load the url in the targeted win/tab.
    var browser = getBrowser();
    var tab = event.originalTarget;
    if (tab.localName == "tab" &&
        tab.parentNode == browser.mTabContainer) {
      tab.linkedBrowser.userTypedValue = url;
      if (tab == browser.mCurrentTab && url != "about:blank") {
          gURLBar.value = url;
      }
      tab.linkedBrowser.loadURI(url);
      if (event.shiftKey != (pref && pref.getBoolPref("browser.tabs.loadInBackground")))
        browser.selectedTab = tab;
    }
    else if (event.target == browser) {
      tab = browser.addTab(url);
      if (event.shiftKey != (pref && pref.getBoolPref("browser.tabs.loadInBackground")))
        browser.selectedTab = tab;
    }
    else {
      if (url != "about:blank") {
        gURLBar.value = url;
      }
      loadURI(url);
    }

    event.stopPropagation();
    return true;
  }

  function addToUrlbarHistory(aUrlToAdd)
  {
    // Remove leading and trailing spaces first
    aUrlToAdd = aUrlToAdd.replace(/^\s+/, '').replace(/\s+$/, '');

    if (!aUrlToAdd)
      return;
    if (aUrlToAdd.search(/[\x00-\x1F]/) != -1) // don't store bad URLs
      return;

    if (!gRDF)
      gRDF = Components.classes["@mozilla.org/rdf/rdf-service;1"]
                       .getService(Components.interfaces.nsIRDFService);
 
    if (!gGlobalHistory)
      gGlobalHistory = Components.classes["@mozilla.org/browser/global-history;2"]
                                 .getService(Components.interfaces.nsIBrowserHistory);

    if (!gURIFixup)
      gURIFixup = Components.classes["@mozilla.org/docshell/urifixup;1"]
                            .getService(Components.interfaces.nsIURIFixup);
    if (!gLocalStore)
      gLocalStore = gRDF.GetDataSource("rdf:local-store");

    if (!gRDFC)
      gRDFC = Components.classes["@mozilla.org/rdf/container-utils;1"]
                        .getService(Components.interfaces.nsIRDFContainerUtils);

    var entries = gRDFC.MakeSeq(gLocalStore, gRDF.GetResource("nc:urlbar-history"));
    if (!entries)
      return;
    var elements = entries.GetElements();
    if (!elements)
      return;
    var index = 0;

    var urlToCompare = aUrlToAdd.toUpperCase();
    while(elements.hasMoreElements()) {
      var entry = elements.getNext();
      if (!entry) continue;

      index ++;
      try {
        entry = entry.QueryInterface(Components.interfaces.nsIRDFLiteral);
      } catch(ex) {
        // XXXbar not an nsIRDFLiteral for some reason. see 90337.
        continue;
      }

      if (urlToCompare == entry.Value.toUpperCase()) {
        // URL already present in the database
        // Remove it from its current position.
        // It is inserted to the top after the while loop.
        entries.RemoveElementAt(index, true);
        break;
      }
    }   // while

    // Otherwise, we've got a new URL in town. Add it!

    try {
      var url = getShortcutOrURI(aUrlToAdd);
      var fixedUpURI = gURIFixup.createFixupURI(url, 0);
      if (!fixedUpURI.schemeIs("data"))
        gGlobalHistory.markPageAsTyped(fixedUpURI);
    }
    catch(ex) {
    }

    // Put the value as it was typed by the user in to RDF
    // Insert it to the beginning of the list.
    var entryToAdd = gRDF.GetLiteral(aUrlToAdd);
    entries.InsertElementAt(entryToAdd, 1, true);

    // Remove any expired history items so that we don't let
    // this grow without bound.
    for (index = entries.GetCount(); index > MAX_URLBAR_HISTORY_ITEMS; --index) {
        entries.RemoveElementAt(index, true);
    }  // for
  }

  function makeURLAbsolute(base, url)
  {
    // Construct nsIURL.
    var ioService = Components.classes["@mozilla.org/network/io-service;1"]
                              .getService(Components.interfaces.nsIIOService);
    var baseURI  = ioService.newURI(base, null, null);

    return ioService.newURI(baseURI.resolve(url), null, null).spec;
  }
