/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Jan Varga <varga@ku.sk>
 *   H�kan Waara (hwaara@chello.se)
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

 //This file stores variables common to mail windows
var messengerContractID        = "@mozilla.org/messenger;1";
var statusFeedbackContractID   = "@mozilla.org/messenger/statusfeedback;1";
var mailSessionContractID      = "@mozilla.org/messenger/services/session;1";
var secureUIContractID         = "@mozilla.org/secure_browser_ui;1";


var prefContractID             = "@mozilla.org/preferences-service;1";
var msgWindowContractID      = "@mozilla.org/messenger/msgwindow;1";

var messenger;
var pref;
var prefServices;
var statusFeedback;
var messagePaneController;
var msgWindow;

var msgComposeService;
var accountManager;
var RDF;
var msgComposeType;
var msgComposeFormat;

var mailSession;

var gMessengerBundle;
var gBrandBundle;

var datasourceContractIDPrefix = "@mozilla.org/rdf/datasource;1?name=";
var accountManagerDSContractID = datasourceContractIDPrefix + "msgaccountmanager";
var folderDSContractID         = datasourceContractIDPrefix + "mailnewsfolders";

var accountManagerDataSource;
var folderDataSource;

var accountCentralBox = null;
var gSearchBox = null;
var gAccountCentralLoaded = true;
var gFakeAccountPageLoaded = false;
//End progress and Status variables

// for checking if the folder loaded is Draft or Unsent which msg is editable
var gIsEditableMsgFolder = false;
var gOfflineManager;

// cache the last keywords
var gLastKeywords = "";

function OnMailWindowUnload()
{
  RemoveMailOfflineObserver();
  ClearPendingReadTimer();

  var searchSession = GetSearchSession();
  if (searchSession)
  {
    removeGlobalListeners();
    if (gPreQuickSearchView)     //close the cached pre quick search view
      gPreQuickSearchView.close();
  }
  
  var dbview = GetDBView();
  if (dbview) {
    dbview.close(); 
  }

  var mailSession = Components.classes[mailSessionContractID].getService();
  if(mailSession)
  {
    mailSession = mailSession.QueryInterface(Components.interfaces.nsIMsgMailSession);
    if(mailSession)
    {
      mailSession.RemoveFolderListener(folderListener);
    }
  }

  mailSession.RemoveMsgWindow(msgWindow);
  messenger.SetWindow(null, null);

  var msgDS = folderDataSource.QueryInterface(Components.interfaces.nsIMsgRDFDataSource);
  msgDS.window = null;

  msgDS = accountManagerDataSource.QueryInterface(Components.interfaces.nsIMsgRDFDataSource);
  msgDS.window = null;


  msgWindow.closeWindow();
}

function CreateMessenger()
{
  messenger = Components.classes[messengerContractID].createInstance();
  messenger = messenger.QueryInterface(Components.interfaces.nsIMessenger);
}

function CreateMailWindowGlobals()
{
  // get the messenger instance
  CreateMessenger();

  prefServices = Components.classes[prefContractID].getService(Components.interfaces.nsIPrefService);
  pref = prefServices.getBranch(null);

  //Create windows status feedback
  // set the JS implementation of status feedback before creating the c++ one..
  window.MsgStatusFeedback = new nsMsgStatusFeedback();
  // double register the status feedback object as the xul browser window implementation
  window.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
        .getInterface(Components.interfaces.nsIWebNavigation)
        .QueryInterface(Components.interfaces.nsIDocShellTreeItem).treeOwner
        .QueryInterface(Components.interfaces.nsIInterfaceRequestor)
        .getInterface(Components.interfaces.nsIXULWindow)
        .XULBrowserWindow = window.MsgStatusFeedback;

  statusFeedback = Components.classes[statusFeedbackContractID].createInstance();
  statusFeedback = statusFeedback.QueryInterface(Components.interfaces.nsIMsgStatusFeedback);

  /*
    not in use unless we want the lock button back

  // try to create and register ourselves with a security icon...
  var securityIcon = document.getElementById("security-button");
  if (securityIcon) {
    // if the client isn't built with psm enabled then we won't have a secure UI to monitor the lock icon
    // so be sure to wrap this in a try / catch clause...
    try {
      var secureUI;
      // we may not have a secure UI if psm isn't installed!
      if (secureUIContractID in Components.classes) {
        secureUI = Components.classes[secureUIContractID].createInstance();
        if (secureUI) {
          secureUI = secureUI.QueryInterface(Components.interfaces.nsISecureBrowserUI);
          secureUI.init(content, securityIcon);
        }
      }
    }
    catch (ex) {}
  }
  */

  window.MsgWindowCommands = new nsMsgWindowCommands();

  //Create message window object
  msgWindow = Components.classes[msgWindowContractID].createInstance();
  msgWindow = msgWindow.QueryInterface(Components.interfaces.nsIMsgWindow);

  msgComposeService = Components.classes['@mozilla.org/messengercompose;1'].getService();
  msgComposeService = msgComposeService.QueryInterface(Components.interfaces.nsIMsgComposeService);

  mailSession = Components.classes["@mozilla.org/messenger/services/session;1"].getService(Components.interfaces.nsIMsgMailSession);

  accountManager = Components.classes["@mozilla.org/messenger/account-manager;1"].getService(Components.interfaces.nsIMsgAccountManager);

  RDF = Components.classes['@mozilla.org/rdf/rdf-service;1'].getService();
  RDF = RDF.QueryInterface(Components.interfaces.nsIRDFService);

  msgComposeType = Components.interfaces.nsIMsgCompType;
  msgComposeFormat = Components.interfaces.nsIMsgCompFormat;

  gMessengerBundle = document.getElementById("bundle_messenger");
  gBrandBundle = document.getElementById("bundle_brand");

  //Create datasources
  accountManagerDataSource = Components.classes[accountManagerDSContractID].getService();
  folderDataSource         = Components.classes[folderDSContractID].getService();
}

function InitMsgWindow()
{
  msgWindow.messagePaneController = new nsMessagePaneController();
  msgWindow.statusFeedback = statusFeedback;
  msgWindow.msgHeaderSink = messageHeaderSink;
  msgWindow.SetDOMWindow(window);
  mailSession.AddMsgWindow(msgWindow);

  var messagepane = document.getElementById("messagepane");
  messagepane.docShell.allowAuth = false;
  msgWindow.rootDocShell.allowAuth = true; 
}

function messagePaneOnClick(event)
{
  // if this is stand alone mail (no browser)
  // or this isn't a simple left click, do nothing, and let the normal code execute
  if (event.button != 0 || event.metaKey || event.ctrlKey || event.shiftKey || event.altKey)
    return contentAreaClick(event);

  // try to determine the href for what you are clicking on.  
  // for example, it might be "" if you aren't left clicking on a link
  var ceParams = hrefAndLinkNodeForClickEvent(event);
  if (!ceParams)
    return true;
  var href = ceParams.href;

  // we know that http://, https://, ftp://, file://, chrome://, 
  // resource://, about:, and gopher:// (as if), 
  // should load in a browser.  but if we don't have one of those
  // (examples are mailto, imap, news, mailbox, snews, nntp, ldap, 
  // and externally handled schemes like aim)
  // we may or may not want a browser window, in which case we return here
  // and let the normal code handle it
  var needABrowser = /(^http(s)?:|^ftp:|^file:|^gopher:|^chrome:|^resource:|^about:)/i;
  if (href.search(needABrowser) == -1) 
    return true;

  // however, if the protocol should not be loaded internally, then we should
  // not put up a new browser window.  we should just let the usual processing
  // take place.
  try {
    var extProtService = Components.classes["@mozilla.org/uriloader/external-protocol-service;1"].getService();
    extProtService = extProtService.QueryInterface(Components.interfaces.nsIExternalProtocolService);
    var scheme = href.substring(0, href.indexOf(":"));
    if (!extProtService.isExposedProtocol(scheme))
      return true;
  } 
  catch (ex) {} // ignore errors, and just assume that we can proceed.

  // if you get here, the user did a simple left click on a link
  // that we know should be in a browser window.
  // since we are in the message pane, send it to the top most browser window 
  // (or open one) right away, instead of waiting for us to get some data and
  // determine the content type, and then open a browser window
  // we want to preventDefault, so that in
  // nsGenericHTMLElement::HandleDOMEventForAnchors(), we don't try to handle the click again
  event.preventDefault();
  if (isPhishingURL(ceParams.linkNode, false, href))
    return false;

  openTopBrowserWith(href);
  return true;
}

function AddDataSources()
{
  accountManagerDataSource = accountManagerDataSource.QueryInterface(Components.interfaces.nsIRDFDataSource);
  folderDataSource = folderDataSource.QueryInterface(Components.interfaces.nsIRDFDataSource);
  //to move menu item
  SetupMoveCopyMenus('moveMenu', accountManagerDataSource, folderDataSource);

  //to copy menu item
  SetupMoveCopyMenus('copyMenu', accountManagerDataSource, folderDataSource);


  //To FileButton menu
  SetupMoveCopyMenus('button-file', accountManagerDataSource, folderDataSource);

  //To move and copy menus in message pane context
  SetupMoveCopyMenus("messagePaneContext-copyMenu", accountManagerDataSource, folderDataSource);
  SetupMoveCopyMenus("messagePaneContext-moveMenu", accountManagerDataSource, folderDataSource);

  //Add statusFeedback

  var msgDS = folderDataSource.QueryInterface(Components.interfaces.nsIMsgRDFDataSource);
  msgDS.window = msgWindow;

  msgDS = accountManagerDataSource.QueryInterface(Components.interfaces.nsIMsgRDFDataSource);
  msgDS.window = msgWindow;

}

function SetupMoveCopyMenus(menuid, accountManagerDataSource, folderDataSource)
{
  var menu = document.getElementById(menuid);
  if(menu)
  {
    menu.database.AddDataSource(accountManagerDataSource);
    menu.database.AddDataSource(folderDataSource);
    menu.setAttribute('ref', 'msgaccounts:/');
  }
}

// We're going to implement our status feedback for the mail window in JS now.
// the following contains the implementation of our status feedback object

function nsMsgStatusFeedback()
{
}

nsMsgStatusFeedback.prototype =
{
  // global variables for status / feedback information....
  statusTextFld : null,
  statusBar     : null,
  statusPanel   : null,
  throbber      : null,
  stopCmd       : null,
  startTimeoutID : null,
  stopTimeoutID  : null,
  pendingStartRequests : 0,
  meteorsSpinning : false,
  myDefaultStatus : null,

  ensureStatusFields : function()
    {
      if (!this.statusTextFld ) this.statusTextFld = document.getElementById("statusText");
      if (!this.statusBar) this.statusBar = document.getElementById("statusbar-icon");
      if (!this.statusPanel) this.statusPanel = document.getElementById("progress-panel");
      if (!this.throbber)   this.throbber = document.getElementById("navigator-throbber");
      if (!this.stopCmd)   this.stopCmd = document.getElementById("cmd_stop");
    },

  // nsIXULBrowserWindow implementation
  setJSStatus : function(status)
    {
      if (status.length > 0)
        this.showStatusString(status);
    },
  setJSDefaultStatus : function(status)
    {
      if (status.length > 0)
      {
        this.myDefaultStatus = status;
        this.statusTextFld.label = status;
      }
    },
  setOverLink : function(link, context)
    {
      this.ensureStatusFields();
      this.statusTextFld.label = link;
    },
  QueryInterface : function(iid)
    {
      if (iid.equals(Components.interfaces.nsIMsgStatusFeedback) ||
          iid.equals(Components.interfaces.nsIXULBrowserWindow) ||
          iid.equals(Components.interfaces.nsISupports))
        return this;
      throw Components.results.NS_NOINTERFACE;
    },

  // nsIMsgStatusFeedback implementation.
  showStatusString : function(statusText)
    {
      this.ensureStatusFields();
      if ( !statusText.length )
        statusText = this.myDefaultStatus;
      else
        this.myDefaultStatus = "";
      this.statusTextFld.label = statusText;
  },
  _startMeteors : function()
    {
      this.ensureStatusFields();

      this.meteorsSpinning = true;
      this.startTimeoutID = null;

      // Show progress meter
      this.statusPanel.hidden = false;

      // Turn progress meter on.
      this.statusBar.setAttribute("mode","undetermined");

      // start the throbber
      if (this.throbber)
        this.throbber.setAttribute("busy", true);

      //turn on stop button and menu
      if (this.stopCmd)
        this.stopCmd.removeAttribute("disabled");
    },
  startMeteors : function()
    {
      this.pendingStartRequests++;
      // if we don't already have a start meteor timeout pending
      // and the meteors aren't spinning, then kick off a start
      if (!this.startTimeoutID && !this.meteorsSpinning && window.MsgStatusFeedback)
        this.startTimeoutID = setTimeout('window.MsgStatusFeedback._startMeteors();', 500);

      // since we are going to start up the throbber no sense in processing
      // a stop timeout...
      if (this.stopTimeoutID)
      {
        clearTimeout(this.stopTimeoutID);
        this.stopTimeoutID = null;
      }
  },
   _stopMeteors : function()
    {
      if(gTimelineEnabled){
        gTimelineService.stopTimer("FolderLoading");
        gTimelineService.markTimer("FolderLoading");
        gTimelineService.resetTimer("FolderLoading");
      }
      this.ensureStatusFields();
      this.showStatusString(defaultStatus);

      // stop the throbber
      if (this.throbber)
        this.throbber.setAttribute("busy", false);

      // Turn progress meter off.
      this.statusPanel.hidden = true;
      this.statusBar.setAttribute("mode","normal");
      this.statusBar.value = 0;  // be sure to clear the progress bar
      this.statusBar.label = "";
      if (this.stopCmd)
        this.stopCmd.setAttribute("disabled", "true");

      this.meteorsSpinning = false;
      this.stopTimeoutID = null;
    },
   stopMeteors : function()
    {
      if (this.pendingStartRequests > 0)
        this.pendingStartRequests--;

      // if we are going to be starting the meteors, cancel the start
      if (this.pendingStartRequests == 0 && this.startTimeoutID)
      {
        clearTimeout(this.startTimeoutID);
        this.startTimeoutID = null;
      }

      // if we have no more pending starts and we don't have a stop timeout already in progress
      // AND the meteors are currently running then fire a stop timeout to shut them down.
      if (this.pendingStartRequests == 0 && !this.stopTimeoutID)
      {
        if (this.meteorsSpinning && window.MsgStatusFeedback)
          this.stopTimeoutID = setTimeout('window.MsgStatusFeedback._stopMeteors();', 500);
      }
  },
  showProgress : function(percentage)
    {
      this.ensureStatusFields();
      if (percentage >= 0)
      {
        this.statusBar.setAttribute("mode", "normal");
        this.statusBar.value = percentage;
        this.statusBar.label = Math.round(percentage) + "%";
      }
    },
  closeWindow : function(percent)
    {
  }
}


function nsMsgWindowCommands()
{
}

nsMsgWindowCommands.prototype =
{
  QueryInterface : function(iid)
  {
    if (iid.equals(Components.interfaces.nsIMsgWindowCommands) ||
        iid.equals(Components.interfaces.nsISupports))
      return this;
    throw Components.results.NS_NOINTERFACE;
  },
  SelectFolder: function(folderUri)
  {
    SelectFolder(folderUri);
  },
  SelectMessage: function(messageUri)
  {
    SelectMessage(messageUri);
  }
}

function nsMessagePaneController()
{
}

nsMessagePaneController.prototype =
{
  QueryInterface : function(iid)
  {
    if (iid.equals(Components.interfaces.nsIMsgMessagePaneController) ||
        iid.equals(Components.interfaces.nsISupports))
      return this;
    throw Components.results.NS_NOINTERFACE;
  },
  clearMsgPane: function()
  {
    if (gDBView)
      setTitleFromFolder(gDBView.msgFolder,null);
    else
      setTitleFromFolder(null,null);
    ClearMessagePane();
  }
}

function StopUrls()
{
  msgWindow.StopUrls();
}

function loadStartPage()
{
  try
  {
    gMessageNotificationBar.clearMsgNotifications();

    var startpageenabled = pref.getBoolPref("mailnews.start_page.enabled");
    if (startpageenabled)
    {
      var startpage = pref.getComplexValue("mailnews.start_page.url",
                                           Components.interfaces.nsIPrefLocalizedString).data;
      if (startpage)
      {
        // first, clear out the charset setting.
        messenger.setDisplayCharset("");
        GetMessagePaneFrame().location.href = startpage;
        //dump("start message pane with: " + startpage + "\n");
        ClearMessageSelection();
      }
    }
  }
  catch (ex)
  {
    dump("Error loading start page.\n");
    return;
  }
}

// When the ThreadPane is hidden via the displayDeck, we should collapse the
// elements that are only meaningful to the thread pane. When AccountCentral is
// shown via the displayDeck, we need to switch the displayDeck to show the
// accountCentralBox, and load the iframe in the AccountCentral box with
// corresponding page.
function ShowAccountCentral()
{
    try
    {
        document.getElementById("displayDeck").selectedPanel = accountCentralBox;
        var acctCentralPage = pref.getComplexValue("mailnews.account_central_page.url",
                                                   Components.interfaces.nsIPrefLocalizedString).data;
        window.frames["accountCentralPane"].location.href = acctCentralPage;
    }
    catch (ex)
    {
        dump("Error loading AccountCentral page -> " + ex + "\n");
        return;
    }
}

function ShowingAccountCentral()
{
    if (!IsFolderPaneCollapsed())
        GetFolderTree().focus();
    gAccountCentralLoaded = true;
}

function HidingAccountCentral()
{
    gAccountCentralLoaded = false;
}

function ShowThreadPane()
{
    document.getElementById("displayDeck").selectedPanel =
        document.getElementById("threadPaneBox");
}

function ShowingThreadPane()
{
    gSearchBox.collapsed = false;
    var threadPaneSplitter = document.getElementById("threadpane-splitter");
    threadPaneSplitter.collapsed = false;
    if (!threadPaneSplitter.hidden && threadPaneSplitter.getAttribute("state") != "collapsed")
        GetMessagePane().collapsed = false;
    document.getElementById("key_toggleMessagePane").removeAttribute("disabled");
}

function HidingThreadPane()
{
    ClearThreadPane();
    GetUnreadCountElement().hidden = true;
    GetTotalCountElement().hidden = true;
    GetMessagePane().collapsed = true;
    document.getElementById("threadpane-splitter").collapsed = true;
    gSearchBox.collapsed = true;
    document.getElementById("key_toggleMessagePane").setAttribute("disabled", "true");
}

function ObserveDisplayDeckChange(event)
{
    var selectedPanel = document.getElementById("displayDeck").selectedPanel;
    var nowSelected = selectedPanel ? selectedPanel.id : "";

    if (nowSelected == "threadPaneBox")
        ShowingThreadPane();
    else
        HidingThreadPane();
    
    if (nowSelected == "accountCentralBox")
        ShowingAccountCentral();
    else
        HidingAccountCentral();
}

// Given the server, open the twisty and the set the selection
// on inbox of that server. 
// prompt if offline.
function OpenInboxForServer(server)
{
    try {
        ShowThreadPane();
        var inboxFolder = GetInboxFolder(server);
        SelectFolder(inboxFolder.URI);

        if(CheckOnline()) {
            if (server.type != "imap")
                GetMessagesForInboxOnServer(server);
        }
        else if (DoGetNewMailWhenOffline()) {
                GetMessagesForInboxOnServer(server);
        }
    }
    catch (ex) {
        dump("Error opening inbox for server -> " + ex + "\n");
        return;
    }
}

function GetSearchSession()
{
  if (("gSearchSession" in top) && gSearchSession)
    return gSearchSession;
  else
    return null;
}

function SetKeywords(aKeywords) 
{ 
  // we cache the last keywords.  
  // if there is no chagne, we do nothing.
  // most of the time, this will be the case.
  if (aKeywords == gLastKeywords)
    return;

  // these are the UI elements who care about keywords
  var elements = document.getElementsByAttribute("keywordrelated","true");
  var len = elements.length;
  for (var i=0; i<len; i++) {
    var element = elements[i];
    var originalclass = element.getAttribute("originalclass");

    // we use XBL for certain headers.
    // if the element has keywordrelated="true"
    // but no original class, it's an XBL widget
    // so to get the real element, use getAnonymousElementByAttribute()
    if (!originalclass) {
      element = document.getAnonymousElementByAttribute(element, "keywordrelated", "true");
      originalclass = element.getAttribute("originalclass");
    }

    if (aKeywords) {
      if (element.getAttribute("appendoriginalclass") == "true") {
        aKeywords += " " + originalclass;
      }
      element.setAttribute("class", aKeywords);
    }
    else {
      // if no keywords, reset class to the original class
      element.setAttribute("class", originalclass);
    }
  }

  // cache the keywords 
  gLastKeywords = aKeywords;
}

function ShowHideToolBarButtons()
{
  var prefBase = "mail.toolbars.showbutton.";
  var prefBranch = prefServices.getBranch(prefBase);
  var prefCount = {value:0};
  var prefArray = prefBranch.getChildList("" , prefCount);

  if (prefArray && (prefCount.value > 0)) {
    for (var i=0;i < prefCount.value;i++) {
      document.getElementById("button-" + prefArray[i]).hidden = !(prefBranch.getBoolPref(prefArray[i]));
    }
  }
}
  
function AddToolBarPrefListener()
{
  try {
    var pbi = pref.QueryInterface(Components.interfaces.nsIPrefBranch2);
    pbi.addObserver(gMailToolBarPrefListener.domain, gMailToolBarPrefListener, false);
  } catch(ex) {
    dump("Failed to observe prefs: " + ex + "\n");
  }
}

function RemoveToolBarPrefListener()
{
  try {
    var pbi = pref.QueryInterface(Components.interfaces.nsIPrefBranch2);
    pbi.removeObserver(gMailToolBarPrefListener.domain, gMailToolBarPrefListener);
  } catch(ex) {
    dump("Failed to remove pref observer: " + ex + "\n");
  }
}

// Pref listener constants
const gMailToolBarPrefListener =
{
  domain: "mail.toolbars.showbutton",
  observe: function(subject, topic, prefName)
  {
    // verify that we're changing a button pref
    if (topic != "nsPref:changed")
      return;

    document.getElementById("button-" + prefName.substr(this.domain.length+1)).hidden = !(pref.getBoolPref(prefName));
  }
};


