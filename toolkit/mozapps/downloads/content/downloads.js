# -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is Mozilla.org Code.
#
# The Initial Developer of the Original Code is
# Netscape Communications Corporation.
# Portions created by the Initial Developer are Copyright (C) 2001
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Blake Ross <blakeross@telocity.com> (Original Author)
#   Ben Goodger <ben@bengoodger.com> (v2.0)
#   Dan Mosedale <dmose@mozilla.org>
#   Fredrik Holmqvist <thesuckiestemail@yahoo.se>
#   Josh Aas <josh@mozilla.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

///////////////////////////////////////////////////////////////////////////////
// Globals

const kObserverServiceProgID = "@mozilla.org/observer-service;1";
const NC_NS = "http://home.netscape.com/NC-rdf#";
const PREF_BDM_CLOSEWHENDONE = "browser.download.manager.closeWhenDone";
const PREF_BDM_ALERTONEXEOPEN = "browser.download.manager.alertOnEXEOpen";

var gDownloadManager  = null;
var gDownloadListener = null;
var gDownloadsView    = null;
var gUserInterfered   = false;
var gActiveDownloads  = [];

// This variable exists because for XPInstalls, we don't want to close the 
// download manager until the XPInstallManager sends the DIALOG_CLOSE status
// message. Setting this variable to false when the downloads window is 
// opened by the xpinstall manager prevents the window from being closed after
// each download completes (because xpinstall downloads are done sequentially, 
// not concurrently) 
var gCanAutoClose   = true;

// If the user has interacted with the window in a significant way, we should
// not auto-close the window. Tough UI decisions about what is "significant."
var gUserInteracted = false;

///////////////////////////////////////////////////////////////////////////////
// Utility Functions 
function setRDFProperty(aID, aProperty, aValue)
{
  var rdf = Components.classes["@mozilla.org/rdf/rdf-service;1"].getService(Components.interfaces.nsIRDFService);

  var db = gDownloadManager.datasource;
  var propertyArc = rdf.GetResource(NC_NS + aProperty);
  
  var res = rdf.GetResource(aID);
  var node = db.GetTarget(res, propertyArc, true);
  if (node)
    db.Change(res, propertyArc, node, rdf.GetLiteral(aValue));
  else
    db.Assert(res, propertyArc, rdf.GetLiteral(aValue), true);
}

function getRDFProperty(aID, aProperty)
{
  var rdf = Components.classes["@mozilla.org/rdf/rdf-service;1"].getService(Components.interfaces.nsIRDFService);

  var db = gDownloadManager.datasource;
  var propertyArc = rdf.GetResource(NC_NS + aProperty);
  
  var res = rdf.GetResource(aID);
  var node = db.GetTarget(res, propertyArc, true);
  if (!node) return "";
  try {
    node = node.QueryInterface(Components.interfaces.nsIRDFLiteral);
    return node.Value;
  }
  catch (e) {
    try {
      node = node.QueryInterface(Components.interfaces.nsIRDFInt);
      return node.Value;
    }
    catch (e) {
      node = node.QueryInterface(Components.interfaces.nsIRDFResource);
      return node.Value;
    }
  }
  return "";
}

function fireEventForElement(aElement, aEventType)
{
  var e = document.createEvent("Events");
  e.initEvent("download-" + aEventType, true, true);
  
  aElement.dispatchEvent(e);
}

///////////////////////////////////////////////////////////////////////////////
// Start/Stop Observers
function downloadCompleted(aDownload) 
{
  // Wrap this in try...catch since this can be called while shutting down... 
  // it doesn't really matter if it fails then since well.. we're shutting down
  // and there's no UI to update!
  try {
    var rdf = Components.classes["@mozilla.org/rdf/rdf-service;1"].getService(Components.interfaces.nsIRDFService);
    var rdfc = Components.classes["@mozilla.org/rdf/container;1"].createInstance(Components.interfaces.nsIRDFContainer);

    var db = gDownloadManager.datasource;
    
    rdfc.Init(db, rdf.GetResource("NC:DownloadsRoot"));

    var id = aDownload.targetFile.path;
    
    // getTypeFromFile fails if it can't find a type for this file. Handle this gracefully.
    try {
      // Refresh the icon, so that executable icons are shown.
      var mimeService = Components.classes["@mozilla.org/uriloader/external-helper-app-service;1"].getService(Components.interfaces.nsIMIMEService);
      var contentType = mimeService.getTypeFromFile(aDownload.targetFile);

      var listItem = document.getElementById(id);
      var oldImage = listItem.getAttribute("image");
      // I tack on the content-type here as a hack to bypass the cache which seems
      // to be interfering despite the fact the image has 'validate="always"' set
      // on it. 
      listItem.setAttribute("image", oldImage + "&contentType=" + contentType);
    } catch (e) {
    }
    
    var dlRes = rdf.GetUnicodeResource(id);
  
    var insertIndex = gDownloadManager.activeDownloadCount + 1;
    // Don't bother inserting the item into the same place!
    if (insertIndex != rdfc.IndexOf(dlRes)) {
      rdfc.RemoveElement(dlRes, true);
      if (insertIndex == rdfc.GetCount() || insertIndex < 1) 
        rdfc.AppendElement(dlRes);
      else
        rdfc.InsertElementAt(dlRes, insertIndex, true);      
    }
        
    // Remove the download from our book-keeping list and if the count
    // falls to zero, update the title here, since we won't be getting
    // any more progress notifications in which to do it. 
    for (var i = 0; i < gActiveDownloads.length; ++i) {
      if (gActiveDownloads[i] == aDownload) {
        gActiveDownloads.splice(i, 1);
        break;
      }
    }

    gDownloadViewController.onCommandUpdate();

    if (gActiveDownloads.length == 0)
      document.title = document.documentElement.getAttribute("statictitle");
  }
  catch (e) {
  }
}

function autoClose(aDownload)
{
  if (gDownloadManager.activeDownloadCount == 0) {
    // For the moment, just use the simple heuristic that if this window was
    // opened by the download process, rather than by the user, it should auto-close
    // if the pref is set that way. If the user opened it themselves, it should
    // not close until they explicitly close it.
    var pref = Components.classes["@mozilla.org/preferences-service;1"]
                        .getService(Components.interfaces.nsIPrefBranch);
    var autoClose = pref.getBoolPref(PREF_BDM_CLOSEWHENDONE)
    if (autoClose && (!window.opener || window.opener.location.href == window.location.href) &&
        gCanAutoClose && !gUserInteracted)
      gCloseDownloadManager();
  }
}

// This function can be overwritten by extensions that wish to place the Download Window in
// another part of the UI, such as in a tab or a sidebar panel. 
function gCloseDownloadManager()
{
  window.close();
}

var gDownloadObserver = {
  observe: function (aSubject, aTopic, aState) 
  {
    switch (aTopic) {
    case "dl-done":
      var dl = aSubject.QueryInterface(Components.interfaces.nsIDownload);
      downloadCompleted(dl);
      
      autoClose(dl);      
      break;
    case "dl-failed":
    case "dl-cancel":
      var dl = aSubject.QueryInterface(Components.interfaces.nsIDownload);
      downloadCompleted(dl);
      break;
    case "dl-start":
      // whenever a new download starts, it is added to the top, so switch the
      // view to it
      gDownloadsView.selectedIndex = 0;

      // Add this download to the percentage average tally
      var dl = aSubject.QueryInterface(Components.interfaces.nsIDownload);
      gActiveDownloads.push(dl);

      break;
    case "xpinstall-download-started":
      var windowArgs = aSubject.QueryInterface(Components.interfaces.nsISupportsArray);
      var params = windowArgs.QueryElementAt(0, Components.interfaces.nsISupportsInterfacePointer);
      params = params.data.QueryInterface(Components.interfaces.nsIDialogParamBlock);
      var installObserver = windowArgs.QueryElementAt(1, Components.interfaces.nsISupportsInterfacePointer);
      installObserver = installObserver.data.QueryInterface(Components.interfaces.nsIObserver);
      XPInstallDownloadManager.addDownloads(params, installObserver);
      break;  
    case "xpinstall-dialog-close":
      if ("gDownloadManager" in window) {
        var mgr = gDownloadManager.QueryInterface(Components.interfaces.nsIXPInstallManagerUI);
        gCanAutoClose = mgr.hasActiveXPIOperations;
        autoClose();
      }
      break;          
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
// Download Event Handlers

function onDownloadCancel(aEvent)
{
  var selectedIndex = gDownloadsView.selectedIndex;

  gDownloadManager.cancelDownload(aEvent.target.id);

  setRDFProperty(aEvent.target.id, "DownloadAnimated", "false");

  // XXXben - 
  // If we got here because we resumed the download, we weren't using a temp file
  // because we used saveURL instead. (this is because the proper download mechanism
  // employed by the helper app service isn't fully accessible yet... should be fixed...
  // talk to bz...)
  // the upshot is we have to delete the file if it exists. 
  var f = getLocalFileFromNativePathOrUrl(aEvent.target.id);

  if (f.exists()) 
    f.remove(false);

  gDownloadViewController.onCommandUpdate();

  // now reset the richlistbox
  gDownloadsView.clearSelection();
  var rowCount = gDownloadsView.getRowCount();
  if (selectedIndex >= rowCount)
    gDownloadsView.selectedIndex = rowCount - 1;
  else
    gDownloadsView.selectedIndex = selectedIndex;
}

function onDownloadPause(aEvent)
{
  var selectedIndex = gDownloadsView.selectedIndex;

  var uri = aEvent.target.id;
  gDownloadManager.pauseDownload(uri);
  setRDFProperty(uri, "DownloadStatus", aEvent.target.getAttribute("status-internal"));
  setRDFProperty(uri, "ProgressPercent", aEvent.target.getAttribute("progress"));

  // now reset the richlistbox
  gDownloadsView.clearSelection();
  var rowCount = gDownloadsView.getRowCount();
  if (selectedIndex >= rowCount)
    gDownloadsView.selectedIndex = rowCount - 1;
  else
    gDownloadsView.selectedIndex = selectedIndex;
}

function onDownloadResume(aEvent)
{
  var selectedIndex = gDownloadsView.selectedIndex;

  gDownloadManager.resumeDownload(aEvent.target.id);

  // now reset the richlistbox
  gDownloadsView.clearSelection();
  var rowCount = gDownloadsView.getRowCount();
  if (selectedIndex >= rowCount)
    gDownloadsView.selectedIndex = rowCount - 1;
  else
    gDownloadsView.selectedIndex = selectedIndex;
}

function onDownloadRemove(aEvent)
{
  if (aEvent.target.removable) {
    var selectedIndex = gDownloadsView.selectedIndex;
    gDownloadManager.removeDownload(aEvent.target.id);
    gDownloadViewController.onCommandUpdate();
  }
}

function onDownloadShow(aEvent)
{
  var f = getLocalFileFromNativePathOrUrl(aEvent.target.id);

  if (f.exists()) {
    try {
      f.reveal();
    } catch (ex) {
      // if reveal failed for some reason (eg on unix it's not currently
      // implemented), send the file: URL window rooted at the parent to 
      // the OS handler for that protocol
      var parent = f.parent;
      if (parent) {
        openExternal(parent);
      }
    }
  }
  else {
    var brandStrings = document.getElementById("brandStrings");
    var appName = brandStrings.getString("brandShortName");
  
    var strings = document.getElementById("downloadStrings");
    var name = aEvent.target.getAttribute("target");
    var message = strings.getFormattedString("fileDoesNotExistError", [name, appName]);
    var title = strings.getFormattedString("fileDoesNotExistShowTitle", [name]);

    var promptSvc = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);
    promptSvc.alert(window, title, message);
  }
}

function onDownloadOpen(aEvent)
{
  if (aEvent.type == "dblclick" && aEvent.button != 0)
    return;
  var download = aEvent.target;
  if (download.localName == "download") {
    if (download.openable) {
      var f = getLocalFileFromNativePathOrUrl(aEvent.target.id);
      if (f.exists()) {
        if (f.isExecutable()) {
          var dontAsk = false;
          var pref = Components.classes["@mozilla.org/preferences-service;1"]
                              .getService(Components.interfaces.nsIPrefBranch);
          try {
            dontAsk = !pref.getBoolPref(PREF_BDM_ALERTONEXEOPEN);
          }
          catch (e) { }
          
          if (!dontAsk) {
            var strings = document.getElementById("downloadStrings");
            var name = aEvent.target.getAttribute("target");
            var message = strings.getFormattedString("fileExecutableSecurityWarning", [name, name]);

            var title = strings.getString("fileExecutableSecurityWarningTitle");
            var dontAsk = strings.getString("fileExecutableSecurityWarningDontAsk");

            var promptSvc = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);
            var checkbox = { value: false };
            var open = promptSvc.confirmCheck(window, title, message, dontAsk, checkbox);
            
            if (!open) 
              return;
            else
              pref.setBoolPref(PREF_BDM_ALERTONEXEOPEN, !checkbox.value);              
          }        
        }
        try {
          f.launch();
        } catch (ex) {
          // if launch fails, try sending it through the system's external
          // file: URL handler
          openExternal(f);
        }
      }
      else {
        var brandStrings = document.getElementById("brandStrings");
        var appName = brandStrings.getString("brandShortName");
      
        var strings = document.getElementById("downloadStrings");
        var name = aEvent.target.getAttribute("target");
        var message = strings.getFormattedString("fileDoesNotExistError", [name, appName]);

        var title = strings.getFormattedString("fileDoesNotExistOpenTitle", [name]);

        var promptSvc = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService(Components.interfaces.nsIPromptService);
        promptSvc.alert(window, title, message);
      }
    }
    else if(download.canceledOrFailed) {
      // If the user canceled this download, double clicking tries again. 
      fireEventForElement(download, "retry")
    }
  }
}

function onDownloadOpenWith(aEvent)
{
}

function onDownloadProperties(aEvent)
{
  gUserInteracted = true;
  window.openDialog("chrome://mozapps/content/downloads/downloadProperties.xul",
                    "_blank", "modal,centerscreen,chrome,resizable=no", aEvent.target.id);
}

function onDownloadAnimated(aEvent)
{
  gDownloadViewController.onCommandUpdate();    

  setRDFProperty(aEvent.target.id, "DownloadAnimated", "true");
}

function onDownloadRetry(aEvent)
{
  var download = aEvent.target;
  if (download.localName == "download") {
    var src = getRDFProperty(download.id, "URL");
    var f = getLocalFileFromNativePathOrUrl(aEvent.target.id);
    saveURL(src, f, null, true, true);
  }
  
  gDownloadViewController.onCommandUpdate();

  // retry always places the item in the first spot
  gDownloadsView.selectedIndex = 0;
}

// This is called by the progress listener. We don't actually use the event
// system here to minimize time wastage. 
var gLastComputedMean = -1;
var gLastActiveDownloads = 0;
function onUpdateProgress()
{
  var numActiveDownloads = gActiveDownloads.length;
  if (numActiveDownloads == 0) {
    document.title = document.documentElement.getAttribute("statictitle");
    gLastComputedMean = -1;
    return;
  }
    
  var mean = 0;
  var base = 0;
  var dl = null;
  for (var i = 0; i < numActiveDownloads; ++i) {
    dl = gActiveDownloads[i];

    // gActiveDownloads is screwed so it's possible 
    // to have more files than we're really downloading.
    // The good news is that those files have size==0.
    // Same with files with unknown size. Their size==0.
    if (dl.percentComplete < 100 && dl.size > 0) {
      mean += dl.amountTransferred;
      base += dl.size;
    }
  }

  // we're not downloading anything at the moment,
  // but we already downloaded something.
  if (base == 0) {
    mean = 100;
  } else {
    mean = Math.floor((mean / base) * 100);
  }

  if (mean != gLastComputedMean || gLastActiveDownloads != numActiveDownloads) {
    gLastComputedMean = mean;
    gLastActiveDownloads = numActiveDownloads;
    
    var strings = document.getElementById("downloadStrings");
    
    var title;
    if (numActiveDownloads > 1)
      title = strings.getFormattedString("downloadsTitleMultiple", [mean, numActiveDownloads]);
    else
      title = strings.getFormattedString("downloadsTitle", [mean]);

    document.title = title;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Startup, Shutdown
function Startup() 
{
  gDownloadsView = document.getElementById("downloadView");

  const dlmgrContractID = "@mozilla.org/download-manager;1";
  const dlmgrIID = Components.interfaces.nsIDownloadManager;
  gDownloadManager = Components.classes[dlmgrContractID].getService(dlmgrIID);
  
  // The DownloadProgressListener (DownloadProgressListener.js) handles progress
  // notifications. 
  var downloadStrings = document.getElementById("downloadStrings");
  gDownloadListener = new DownloadProgressListener(document, downloadStrings);
  gDownloadManager.listener = gDownloadListener;

  // The active downloads list is created by the front end only when the download starts,  
  // so we need to pre-populate it with any downloads that were already going. 
  var activeDownloads = gDownloadManager.activeDownloads;
  var count = activeDownloads.Count();
  for (var i = 0; i < count; ++i)
    gActiveDownloads.push(activeDownloads.QueryElementAt(i, Components.interfaces.nsIDownload));

  // Handlers for events generated by the UI (downloads, list view)
  gDownloadsView.addEventListener("download-cancel",      onDownloadCancel,     false);
  gDownloadsView.addEventListener("download-pause",       onDownloadPause,      false);
  gDownloadsView.addEventListener("download-resume",      onDownloadResume,     false);
  gDownloadsView.addEventListener("download-remove",      onDownloadRemove,     false);
  gDownloadsView.addEventListener("download-show",        onDownloadShow,       false);
  gDownloadsView.addEventListener("download-open",        onDownloadOpen,       false);
  gDownloadsView.addEventListener("download-retry",       onDownloadRetry,      false);
  gDownloadsView.addEventListener("download-animated",    onDownloadAnimated,   false);
  gDownloadsView.addEventListener("download-properties",  onDownloadProperties, false);
  gDownloadsView.addEventListener("dblclick",             onDownloadOpen,       false);
  
  // Set up AutoDownload display area
  initAutoDownloadDisplay();
  var pbi = Components.classes["@mozilla.org/preferences-service;1"]
                      .getService(Components.interfaces.nsIPrefBranch2);
  pbi.addObserver("browser.download.", gDownloadPrefObserver, false);
  
  // Handlers for events generated by the Download Manager (download events)
  var observerService = Components.classes[kObserverServiceProgID]
                                  .getService(Components.interfaces.nsIObserverService);
  observerService.addObserver(gDownloadObserver, "dl-done",   false);
  observerService.addObserver(gDownloadObserver, "dl-cancel", false);
  observerService.addObserver(gDownloadObserver, "dl-failed", false);  
  observerService.addObserver(gDownloadObserver, "dl-start",  false);  
  observerService.addObserver(gDownloadObserver, "xpinstall-download-started", false);  
  observerService.addObserver(gDownloadObserver, "xpinstall-dialog-close",     false);
  
  // Now look and see if we're being opened by XPInstall
  if ("arguments" in window) {
    try {
      var params = window.arguments[0].QueryInterface(Components.interfaces.nsIDialogParamBlock);
      var installObserver = window.arguments[1].QueryInterface(Components.interfaces.nsIObserver);
      XPInstallDownloadManager.addDownloads(params, installObserver);
      var mgr = gDownloadManager.QueryInterface(Components.interfaces.nsIXPInstallManagerUI);
      gCanAutoClose = mgr.hasActiveXPIOperations;
    }
    catch (e) { }
  }

  // This is for the "Clean Up" button, which requires there to be
  // non-active downloads before it can be enabled. 
  gDownloadsView.controllers.appendController(gDownloadViewController);

  // Finally, update the UI. 
  gDownloadsView.database.AddDataSource(gDownloadManager.datasource);
  gDownloadsView.builder.rebuild();
  
  // downloads can finish before Startup() does, so check if the window should close
  autoClose();
}

function Shutdown() 
{
  gDownloadManager.listener = null;

  // Assert the current progress for all the downloads in case the window is reopened
  gDownloadManager.saveState();

  var pbi = Components.classes["@mozilla.org/preferences-service;1"]
                      .getService(Components.interfaces.nsIPrefBranch2);
  pbi.removeObserver("browser.download.", gDownloadPrefObserver);

  var observerService = Components.classes[kObserverServiceProgID]
                                  .getService(Components.interfaces.nsIObserverService);
  observerService.removeObserver(gDownloadObserver, "dl-done");
  observerService.removeObserver(gDownloadObserver, "dl-cancel");
  observerService.removeObserver(gDownloadObserver, "dl-failed");  
  observerService.removeObserver(gDownloadObserver, "dl-start");  
  observerService.removeObserver(gDownloadObserver, "xpinstall-download-started");  
  observerService.removeObserver(gDownloadObserver, "xpinstall-dialog-close");
}

///////////////////////////////////////////////////////////////////////////////
// XPInstall

var XPInstallDownloadManager = {
  addDownloads: function (aParams, aObserver)
  {
    var numXPInstallItems = aParams.GetInt(1);
    
    var fileLocator = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
    var tempDir = fileLocator.get("TmpD", Components.interfaces.nsIFile);

    var mimeService = Components.classes["@mozilla.org/uriloader/external-helper-app-service;1"].getService(Components.interfaces.nsIMIMEService);

    var IOService = Components.classes["@mozilla.org/network/io-service;1"]
                              .getService(Components.interfaces.nsIIOService);

    var xpinstallManager = gDownloadManager.QueryInterface(Components.interfaces.nsIXPInstallManagerUI);

    var xpiString = "";

    for (var i = 0; i < numXPInstallItems;) {
      // Pretty Name
      var displayName = aParams.GetString(i++);
      
      // URI
      var uri = IOService.newURI(aParams.GetString(i++), null, null);

      var iconURL = aParams.GetString(i++);
      
      // Local File Target
      var url = uri.QueryInterface(Components.interfaces.nsIURL);
      var localTarget = tempDir.clone();
      localTarget.append(url.fileName);
      
      xpiString += localTarget.path + ",";
      
      // MIME Info
      var mimeInfo = null;
      try {
        mimeInfo = mimeService.getFromTypeAndExtension(null, url.fileExtension);
      }
      catch (e) { }
      
      if (!iconURL) 
        iconURL = "chrome://mozapps/skin/xpinstall/xpinstallItemGeneric.png";
      
      var targetUrl = makeFileURI(localTarget);
      var download = gDownloadManager.addDownload(Components.interfaces.nsIXPInstallManagerUI.DOWNLOAD_TYPE_INSTALL, 
                                                  uri, targetUrl, displayName, iconURL, mimeInfo, 0, null);
      
      // Advance the enumerator
      var certName = aParams.GetString(i++);
    }

    var observerService = Components.classes[kObserverServiceProgID]
                                    .getService(Components.interfaces.nsIObserverService);
    observerService.notifyObservers(xpinstallManager.xpiProgress, "xpinstall-progress", "open");  
  }
}

///////////////////////////////////////////////////////////////////////////////
// View Context Menus
var gContextMenus = [ 
  ["menuitem_pause", "menuitem_cancel", "menuseparator_properties", "menuitem_properties"],
  ["menuitem_open", "menuitem_show", "menuitem_remove", "menuseparator_properties", "menuitem_properties"],
  ["menuitem_retry", "menuitem_remove", "menuseparator_properties", "menuitem_properties"],
  ["menuitem_retry", "menuitem_remove", "menuseparator_properties", "menuitem_properties"],
  ["menuitem_resume", "menuitem_cancel", "menuseparator_properties", "menuitem_properties"]
];

function buildContextMenu(aEvent)
{
  if (aEvent.target.id != "downloadContextMenu")
    return false;
    
  var popup = document.getElementById("downloadContextMenu");
  while (popup.hasChildNodes())
    popup.removeChild(popup.firstChild);
  
  if (gDownloadsView.selectedItem) {
    var idx = parseInt(gDownloadsView.selectedItem.getAttribute("state"));
    if (idx < 0)
      idx = 0;
    
    var menus = gContextMenus[idx];
    for (var i = 0; i < menus.length; ++i)
      popup.appendChild(document.getElementById(menus[i]).cloneNode(true));
    
    return true;
  }
  
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// Drag and Drop

var gDownloadDNDObserver =
{
  onDragOver: function (aEvent, aFlavour, aDragSession)
  {
    aDragSession.canDrop = true;
  },
  
  onDrop: function(aEvent, aXferData, aDragSession)
  {
    var split = aXferData.data.split("\n");
    var url = split[0];
    if (url != aXferData.data) {  //do nothing, not a valid URL
      var name = split[1];
      saveURL(url, name, null, true, true);
    }
  },
  _flavourSet: null,  
  getSupportedFlavours: function ()
  {
    if (!this._flavourSet) {
      this._flavourSet = new FlavourSet();
      this._flavourSet.appendFlavour("text/x-moz-url");
      this._flavourSet.appendFlavour("text/unicode");
    }
    return this._flavourSet;
  }
}

///////////////////////////////////////////////////////////////////////////////
// Command Updating and Command Handlers

var gDownloadViewController = {
  supportsCommand: function (aCommand)
  {
    return aCommand == "cmd_cleanUp";
  },
  
  isCommandEnabled: function (aCommand)
  {
    if (aCommand == "cmd_cleanUp") 
      return gDownloadManager.canCleanUp;
    return false;
  },
  
  doCommand: function (aCommand)
  {
    if (aCommand == "cmd_cleanUp" && this.isCommandEnabled(aCommand)) {
      gDownloadManager.cleanUp();
      this.onCommandUpdate();
    }
  },  
  
  onCommandUpdate: function ()
  {
    var command = "cmd_cleanUp";
    var enabled = this.isCommandEnabled(command);
    goSetCommandEnabled(command, enabled);
  }
};

function onDownloadShowInfo()
{
  if (gDownloadsView.selectedItem)
    fireEventForElement(gDownloadsView.selectedItem, "properties");
}

function initAutoDownloadDisplay()
{
  var pref = Components.classes["@mozilla.org/preferences-service;1"]
                       .getService(Components.interfaces.nsIPrefBranch);

  var autodownload = pref.getBoolPref("browser.download.useDownloadDir");  
  if (autodownload) {
    var autodownloadInfo = document.getElementById("autodownloadInfo");
    autodownloadInfo.hidden = false;
    var autodownloadSpring = document.getElementById("autodownloadSpring");
    autodownloadSpring.hidden = true; 

    function getSpecialFolderKey(aFolderType) 
    {
    if (aFolderType == "Desktop")
      return "Desk";

    if (aFolderType != "Downloads")
      throw "ASSERTION FAILED: folder type should be 'Desktop' or 'Downloads'";

#ifdef XP_WIN
    return "Pers";
#else
#ifdef XP_MACOSX
    return "UsrDocs";
#else
    return "Home";
#endif
#endif
    }
    
    function getDownloadsFolder(aFolder)
    {
      var fileLocator = Components.classes["@mozilla.org/file/directory_service;1"]
                                  .getService(Components.interfaces.nsIProperties);
      var dir = fileLocator.get(getSpecialFolderKey(aFolder), Components.interfaces.nsILocalFile);
      
      var bundle = Components.classes["@mozilla.org/intl/stringbundle;1"]
                              .getService(Components.interfaces.nsIStringBundleService);
      bundle = bundle.createBundle("chrome://mozapps/locale/downloads/unknownContentType.properties");

      var description = bundle.GetStringFromName("myDownloads");
      if (aFolder != "Desktop")
        dir.append(description);
        
      return dir;
    }

    var displayName = null;
    var folder;
    switch (pref.getIntPref("browser.download.folderList")) {
    case 0:
      folder = getDownloadsFolder("Desktop");
      displayName = document.getElementById("downloadStrings").getString("displayNameDesktop");
      break;
    case 1:
      folder = getDownloadsFolder("Downloads");
      break;
    case 2: 
      folder = pref.getComplexValue("browser.download.dir", Components.interfaces.nsILocalFile);
      break;
    }

    if (folder) {    
      var ioServ = Components.classes["@mozilla.org/network/io-service;1"]
                              .getService(Components.interfaces.nsIIOService);
      var fph = ioServ.getProtocolHandler("file").QueryInterface(Components.interfaces.nsIFileProtocolHandler);
      var mozIconURI = "moz-icon://" +  fph.getURLSpecFromFile(folder) + "?size=16";
      var folderIcon = document.getElementById("saveToFolder")
      folderIcon.image = mozIconURI;
      
      var folderName = document.getElementById("saveToFolder");
      folderName.label = displayName || folder.leafName;
      folderName.setAttribute("path", folder.path);
    }
  }
  else {
    var autodownloadInfo = document.getElementById("autodownloadInfo");
    autodownloadInfo.hidden = true;
    var autodownloadSpring = document.getElementById("autodownloadSpring");
    autodownloadSpring.hidden = false; 
  }
}

var gDownloadPrefObserver = {
  observe: function (aSubject, aTopic, aData)
  {
    if (aTopic == "nsPref:changed") {
      switch(aData) {
      case "browser.download.folderList":
      case "browser.download.useDownloadDir":
      case "browser.download.dir":
        initAutoDownloadDisplay();
      }
    }
  }
};

function onDownloadShowFolder()
{
  var folderName = document.getElementById("saveToFolder");
  var dir = getLocalFileFromNativePathOrUrl(folderName.getAttribute("path"));
  if (!dir.exists())
   dir.create(Components.interfaces.nsIFile.DIRECTORY_TYPE, 0755);

  try {
    dir.reveal();
  } catch (ex) {
    // if nsILocalFile::Reveal failed (eg it currently just returns an
    // error on unix), just open the folder in a browser window
    openExternal(dir);
  }
}

function openExternal(aFile)
{
  var uri = Components.classes["@mozilla.org/network/io-service;1"]
                      .getService(Components.interfaces.nsIIOService)
                      .newFileURI(aFile);

  var protocolSvc = 
      Components.classes["@mozilla.org/uriloader/external-protocol-service;1"]
                .getService(Components.interfaces.nsIExternalProtocolService);

  protocolSvc.loadUrl(uri);

  return;
}

// we should be using real URLs all the time, but until 
// bug 239948 is fully fixed, this will do...
function getLocalFileFromNativePathOrUrl(aPathOrUrl)
{
  if (aPathOrUrl.substring(0,7) == "file://") {

    // if this is a URL, get the file from that
    ioSvc = Components.classes["@mozilla.org/network/io-service;1"]
      .getService(Components.interfaces.nsIIOService);

    // XXX it's possible that using a null char-set here is bad
    const fileUrl = ioSvc.newURI(aPathOrUrl, null, null).
      QueryInterface(Components.interfaces.nsIFileURL);
    return fileUrl.file.clone().
      QueryInterface(Components.interfaces.nsILocalFile);

  } else {

    // if it's a pathname, create the nsILocalFile directly
    var f = Components.classes["@mozilla.org/file/local;1"].
      createInstance(Components.interfaces.nsILocalFile);
    f.initWithPath(aPathOrUrl);

    return f;
  }
}
