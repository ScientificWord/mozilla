/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License
 * at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and
 * limitations under the License.
 *
 * The Original Code is the Places JS Livemark Service.
 *
 * The Initial Developer of the Original Code is Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Annie Sullivan <annie.sullivan@gmail.com> (C++ author)
 *   Joe Hughes <joe@retrovirus.com>
 *   Vladimir Vukicevic <vladimir@pobox.com>
 *   Masayuki Nakano <masayuki@d-toybox.com>
 *   Robert Sayre <sayrer@gmail.com> (JS port)
 *   Phil Ringnalda <philringnalda@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the
 * terms of either the GNU General Public License Version 2 or later
 * (the "GPL"), or the GNU Lesser General Public License Version 2.1
 * or later (the "LGPL"), in which case the provisions of the GPL or
 * the LGPL are applicable instead of those above. If you wish to
 * allow use of your version of this file only under the terms of
 * either the GPL or the LGPL, and not to allow others to use your
 * version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the
 * notice and other provisions required by the GPL or the LGPL. If you
 * do not delete the provisions above, a recipient may use your
 * version of this file under the terms of any one of the MPL, the GPL
 * or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;

#include ../../url-classifier/content/moz/lang.js
#include ../../url-classifier/content/moz/observer.js
#include ../../url-classifier/content/moz/alarm.js

function LOG(str) {
  dump("*** " + str + "\n");
}

const LS_CLASSID = Components.ID("{dca61eb5-c7cd-4df1-b0fb-d0722baba251}");
const LS_CLASSNAME = "Livemark Service";
const LS_CONTRACTID = "@mozilla.org/browser/livemark-service;2";

const LIVEMARK_TIMEOUT = 15000; // fire every 15 seconds
const PLACES_BUNDLE_URI = "chrome://places/locale/places.properties";
const DEFAULT_LOAD_MSG = "Live Bookmark loading...";
const DEFAULT_FAIL_MSG = "Live Bookmark feed failed to load.";
const LMANNO_FEEDURI = "livemark/feedURI";
const LMANNO_SITEURI = "livemark/siteURI";
const LMANNO_EXPIRATION = "livemark/expiration";
const LMANNO_LOADFAILED = "livemark/loadfailed";

const PS_CONTRACTID = "@mozilla.org/preferences-service;1";
const NH_CONTRACTID = "@mozilla.org/browser/nav-history-service;1";
const AS_CONTRACTID = "@mozilla.org/browser/annotation-service;1";
const OS_CONTRACTID = "@mozilla.org/observer-service;1";
const SB_CONTRACTID = "@mozilla.org/intl/stringbundle;1";
const IO_CONTRACTID = "@mozilla.org/network/io-service;1";
const BMS_CONTRACTID = "@mozilla.org/browser/nav-bookmarks-service;1";
const FAV_CONTRACTID = "@mozilla.org/browser/favicon-service;1";
const LG_CONTRACTID = "@mozilla.org/network/load-group;1";
const FP_CONTRACTID = "@mozilla.org/feed-processor;1";
const SEC_CONTRACTID = "@mozilla.org/scriptsecuritymanager;1";
const IS_CONTRACTID = "@mozilla.org/widget/idleservice;1";
const SEC_FLAGS = Ci.nsIScriptSecurityManager.DISALLOW_INHERIT_PRINCIPAL;
const NS_BINDING_ABORTED = 0x804b0002;

// Check every hour by default
var gExpiration = 3600000;

// Check every 10 minutes on error
const ERROR_EXPIRATION = 600000;

// Don't check when the user is idle for longer than half an hour:
const IDLE_TIMELIMIT = 1800000;

var gIoService = Cc[IO_CONTRACTID].getService(Ci.nsIIOService);
var gStringBundle;
function GetString(name)
{
  try {
    if (!gStringBundle) {
      var bundleService = Cc[SB_CONTRACTID].getService();
      bundleService = bundleService.QueryInterface(Ci.nsIStringBundleService);
      gStringBundle = bundleService.createBundle(PLACES_BUNDLE_URI);
    }

    if (gStringBundle)
      return gStringBundle.GetStringFromName(name);
  } catch (ex) {
    LOG("Exception loading string bundle: " + ex.message);
  }

  return null;
}

function MarkLivemarkLoadFailed(aFolderId) {
  // if it failed before, too, nothing more to do
  var ans = Cc[AS_CONTRACTID].getService(Ci.nsIAnnotationService);
  if (ans.itemHasAnnotation(aFolderId, LMANNO_LOADFAILED))
    return;

  var failedMsg = GetString("bookmarksLivemarkFailed") || DEFAULT_FAIL_MSG;
  var failedURI = gIoService.newURI("about:livemark-failed", null, null);
  var bms = Cc[BMS_CONTRACTID].getService(Ci.nsINavBookmarksService);
  bms.insertBookmark(aFolderId, failedURI, 0, failedMsg);
  ans.setItemAnnotation(aFolderId, LMANNO_LOADFAILED, true, 0,
                        ans.EXPIRE_NEVER);
}

function LivemarkService() {

  try {
    var prefs = Cc[PS_CONTRACTID].getService(Ci.nsIPrefBranch);
    var livemarkRefresh =
      prefs.getIntPref("browser.bookmarks.livemark_refresh_seconds");
    // Reset global expiration variable to reflect hidden pref (in ms)
    // with a lower limit of 1 minute (60000 ms)
    gExpiration = Math.max(livemarkRefresh * 1000, 60000);
  }
  catch (ex) { }

  // [ {folderId:, folderURI:, feedURI:, loadGroup:, locked: } ];
  this._livemarks = [];

  this._loading = GetString("bookmarksLivemarkLoading") || DEFAULT_LOAD_MSG;
  this._observerServiceObserver =
    new G_ObserverServiceObserver('xpcom-shutdown',
                                  BindToObject(this._shutdown, this),
                                  true /*only once*/);

  if (IS_CONTRACTID in Cc)
    this._idleService = Cc[IS_CONTRACTID].getService(Ci.nsIIdleService);

  // this is giving a reentrant getService warning in XPCShell. bug 194568.
  this._ans = Cc[AS_CONTRACTID].getService(Ci.nsIAnnotationService);

  var livemarks = this._ans.getItemsWithAnnotation(LMANNO_FEEDURI, {});
  for (var i = 0; i < livemarks.length; i++) {
    var feedURI =
      gIoService.newURI(
        this._ans.getItemAnnotation(livemarks[i], LMANNO_FEEDURI),
        null, null
      );
    this._pushLivemark(livemarks[i], feedURI);
  }

  this._bms.addObserver(this, false);
}

LivemarkService.prototype = {

  get _bms() {
    if (!this.__bms)
      this.__bms = Cc[BMS_CONTRACTID].getService(Ci.nsINavBookmarksService);
    return this.__bms;
  },

  get _history() {
    if (!this.__history)
      this.__history = Cc[NH_CONTRACTID].getService(Ci.nsINavHistoryService);
    return this.__history;
  },

  _updateTimer: null,
  start: function LS_start() {
    if (this._updateTimer)
      return;
    this._updateTimer = new G_Alarm(BindToObject(this._fireTimer, this),
                                    LIVEMARK_TIMEOUT, true /* repeat */);
  },

  // returns new length of _livemarks
  _pushLivemark: function LS__pushLivemark(folderId, feedURI) {
    return this._livemarks.push({folderId: folderId, feedURI: feedURI});
  },

  _getLivemarkIndex: function LS__getLivemarkIndex(folderId) {
    for (var i=0; i < this._livemarks.length; ++i) {
      if (this._livemarks[i].folderId == folderId)
        return i;
    }
    throw Cr.NS_ERROR_INVALID_ARG;
  },

  _shutdown: function LS__shutdown() {
    // remove bookmarks observer
    this._bms.removeObserver(this);

    for (var livemark in this._livemarks) {
      if (livemark.loadGroup)
        livemark.loadGroup.cancel(NS_BINDING_ABORTED);
    }

    // kill timer
    if (this._updateTimer) {
      this._updateTimer.cancel();
      this._updateTimer = null;
    }
  },

  _fireTimer: function LS__fireTimer() {
    for (var i=0; i < this._livemarks.length; ++i) {
      this._updateLivemarkChildren(i, false);
    }
  },

  deleteLivemarkChildren: function LS_deleteLivemarkChildren(folderId) {
    this._bms.removeFolderChildren(folderId);
  },

  insertLivemarkLoadingItem: function LS_insertLivemarkLoading(bms, livemark) {
    var loadingURI = gIoService.newURI("about:livemark-loading", null, null);
    if (!livemark.loadingId || livemark.loadingId == -1)
      livemark.loadingId = bms.insertBookmark(livemark.folderId, loadingURI,
                                              0, this._loading);
  },

  _updateLivemarkChildren:
  function LS__updateLivemarkChildren(index, forceUpdate) {
    if (this._livemarks[index].locked)
      return;

    var livemark = this._livemarks[index];
    livemark.locked = true;
    try {
      // Check the TTL/expiration on this.  If there isn't one,
      // then we assume it's never been loaded.  We perform this
      // check even when the update is being forced, in case the
      // livemark has somehow never been loaded.
      var exprTime = this._ans.getPageAnnotation(livemark.feedURI,
                                                 LMANNO_EXPIRATION);
      if (!forceUpdate && exprTime > Date.now()) {
        // no need to refresh
        livemark.locked = false;
        return;
      }

      // Check the user idle time. If the user isn't using their computer, don't
      // bother updating - save the internet some bandwidth. If we can't
      // get the idle time, assume the user isn't idle.
      var idleTime = 0;
      try {
        idleTime = this._idleService.idleTime;
      } catch (ex) { /* We don't care */ }
      if (idleTime > IDLE_TIMELIMIT)
      {
        livemark.locked = false;
        return;
      }
    }
    catch (ex) {
      // This livemark has never been loaded, since it has no expire time.
    }

    var loadgroup;
    try {
      // Create a load group for the request.  This will allow us to
      // automatically keep track of redirects, so we can always
      // cancel the channel.
      loadgroup = Cc[LG_CONTRACTID].createInstance(Ci.nsILoadGroup);
      var uriChannel = gIoService.newChannel(livemark.feedURI.spec, null, null);
      uriChannel.loadGroup = loadgroup;
      uriChannel.loadFlags |= Ci.nsIRequest.LOAD_BACKGROUND |
                              Ci.nsIRequest.VALIDATE_ALWAYS;
      var httpChannel = uriChannel.QueryInterface(Ci.nsIHttpChannel);
      httpChannel.requestMethod = "GET";
      httpChannel.setRequestHeader("X-Moz", "livebookmarks", false);

      // Stream the result to the feed parser with this listener
      var listener = new LivemarkLoadListener(livemark);
      this.insertLivemarkLoadingItem(this._bms, livemark);
      httpChannel.notificationCallbacks = listener;
      httpChannel.asyncOpen(listener, null);
    }
    catch (ex) {
      if (livemark.loadingId != -1) {
        this._bms.removeItem(livemark.loadingId);
        livemark.loadingId = -1;
      }
      MarkLivemarkLoadFailed(livemark.folderId);
      livemark.locked = false;
      LOG("exception: " + ex);
      throw ex;
    }
    livemark.loadGroup = loadgroup;
  },

  createLivemark: function LS_createLivemark(folder, name, siteURI,
                                             feedURI, index) {
    // Don't add livemarks to livemarks
    if (this.isLivemark(folder))
      throw Cr.NS_ERROR_INVALID_ARG;
    var livemarkID = this._createFolder(folder, name, siteURI, feedURI, index);

    // kick off http fetch
    this._updateLivemarkChildren(
      this._pushLivemark(livemarkID, feedURI) - 1,
      false
    );

    return livemarkID;
  },

  createLivemarkFolderOnly:
  function LS_createLivemarkFolderOnly(folder, name, siteURI, feedURI, index) {
    var livemarkID = this._createFolder(folder, name, siteURI, feedURI, index);
    this._pushLivemark(livemarkID, feedURI);
    var livemarkIndex = this._getLivemarkIndex(livemarkID);
    var livemark = this._livemarks[livemarkIndex];
    this.insertLivemarkLoadingItem(this._bms, livemark);

    return livemarkID;
  },

  _createFolder:
  function LS__createFolder(folder, name, siteURI, feedURI, index) {
    var livemarkID = this._bms.createFolder(folder, name, index);
    this._bms.setFolderReadonly(livemarkID, true);

    // Add an annotation to map the folder id to the livemark feed URI
    this._ans.setItemAnnotation(livemarkID, LMANNO_FEEDURI, feedURI.spec, 0,
                                this._ans.EXPIRE_NEVER);

    if (siteURI) {
      // Add an annotation to map the folder URI to the livemark site URI
      this._setSiteURISecure(livemarkID, feedURI, siteURI);
    }

    return livemarkID;
  },

  isLivemark: function LS_isLivemark(folder) {
    return this._ans.itemHasAnnotation(folder, LMANNO_FEEDURI);
  },

  _ensureLivemark: function LS__ensureLivemark(container) {
    if (!this.isLivemark(container))
      throw Cr.NS_ERROR_INVALID_ARG;
  },

  getSiteURI: function LS_getSiteURI(container) {
    this._ensureLivemark(container);

    if (this._ans.itemHasAnnotation(container, LMANNO_SITEURI)) {
      var siteURIString =
        this._ans.getItemAnnotation(container, LMANNO_SITEURI);

      return gIoService.newURI(siteURIString, null, null);
    }
    return null;
  },

  setSiteURI: function LS_setSiteURI(container, siteURI) {
    this._ensureLivemark(container);

    if (!siteURI) {
      this._ans.removeItemAnnotation(container, LMANNO_SITEURI);
      return;
    }

    var livemarkIndex = this._getLivemarkIndex(container);
    var livemark = this._livemarks[livemarkIndex];
    this._setSiteURISecure(container, livemark.feedURI, siteURI);
  },

  _setSiteURISecure: function LS__setSiteURISecure(container, feedURI, siteURI) {
    var secMan = Cc[SEC_CONTRACTID].getService(Ci.nsIScriptSecurityManager);
    var feedPrincipal = secMan.getCodebasePrincipal(feedURI);
    try {
      secMan.checkLoadURIWithPrincipal(feedPrincipal, siteURI, SEC_FLAGS);
    } catch (e) {
      return;
    }
    this._ans.setItemAnnotation(container, LMANNO_SITEURI, siteURI.spec,
                                0, this._ans.EXPIRE_NEVER);
  },

  getFeedURI: function LS_getFeedURI(container) {
    if (this._ans.itemHasAnnotation(container, LMANNO_FEEDURI))
      return gIoService.newURI(this._ans.getItemAnnotation(container,
                                                           LMANNO_FEEDURI),
                               null, null);

    return null;
  },

  setFeedURI: function LS_setFeedURI(container, feedURI) {
    if (!feedURI)
      throw Cr.NS_ERROR_INVALID_ARG;

    this._ans.setItemAnnotation(container, LMANNO_FEEDURI, feedURI.spec, 0,
                                this._ans.EXPIRE_NEVER);

    // now update our internal table
    var livemarkIndex = this._getLivemarkIndex(container);
    this._livemarks[livemarkIndex].feedURI = feedURI;
  },

  reloadAllLivemarks: function LS_reloadAllLivemarks() {
    for (var i = 0; i < this._livemarks.length; ++i) {
      this._updateLivemarkChildren(i, true);
    }
  },

  reloadLivemarkFolder: function LS_reloadLivemarkFolder(folderID) {
    var livemarkIndex = this._getLivemarkIndex(folderID);
    this._updateLivemarkChildren(livemarkIndex, true);
  },

  // nsINavBookmarkObserver
  onBeginUpdateBatch: function() { },
  onEndUpdateBatch: function() { },
  onItemAdded: function() { },
  onItemChanged: function() { },
  onItemVisited: function() { },
  onItemMoved: function() { },

  onItemRemoved: function(aItemId, aParentFolder, aIndex) {
    try {
      var livemarkIndex = this._getLivemarkIndex(aItemId);
    }
    catch(ex) {
      // not a livemark
      return;
    }
    var livemark = this._livemarks[livemarkIndex];

    // remove the livemark from the update array
    this._livemarks.splice(livemarkIndex, 1);

    // check if we have more then one livemark for this address
    // if exists we should not delete the annotation since it's still in use
    var stillInUse = false;
    stillInUse = this._livemarks.some(
                 function(mark) { return mark.feedURI.equals(livemark.feedURI) }
                 );
    if (!stillInUse) {
      this._ans.removePageAnnotation(livemark.feedURI, LMANNO_EXPIRATION);
    }

    if (livemark.loadGroup)
      livemark.loadGroup.cancel(NS_BINDING_ABORTED);
  },

  createInstance: function LS_createInstance(outer, iid) {
    if (outer != null)
      throw Cr.NS_ERROR_NO_AGGREGATION;
    return this.QueryInterface(iid);
  },

  QueryInterface: function LS_QueryInterface(iid) {
    if (iid.equals(Ci.nsILivemarkService) ||
        iid.equals(Ci.nsIFactory) ||
        iid.equals(Ci.nsINavBookmarkObserver) ||
        iid.equals(Ci.nsISupports))
      return this;
    throw Cr.NS_ERROR_NOT_IMPLEMENTED;
  }
};

function LivemarkLoadListener(livemark) {
  this._livemark = livemark;
  this._livemark.loadingId = -1;
  this._processor = null;
  this._isAborted = false;
  this._ttl = gExpiration;
  this._ans = Cc[AS_CONTRACTID].getService(Ci.nsIAnnotationService);
}

LivemarkLoadListener.prototype = {

  abort: function LLL_abort() {
    this._isAborted = true;
  },

  get _bms() {
    if (!this.__bms)
      this.__bms = Cc[BMS_CONTRACTID].getService(Ci.nsINavBookmarksService);
    return this.__bms;
  },

  get _history() {
    if (!this.__history)
      this.__history = Cc[NH_CONTRACTID].getService(Ci.nsINavHistoryService);
    return this.__history;
  },

  // called back from handleResult
  runBatched: function LLL_runBatched(aUserData) {
    var result = aUserData.QueryInterface(Ci.nsIFeedResult);

    // We need this to make sure the item links are safe
    var secMan = Cc[SEC_CONTRACTID].getService(Ci.nsIScriptSecurityManager);
    var feedPrincipal = secMan.getCodebasePrincipal(this._livemark.feedURI);

    var lmService = Cc[LS_CONTRACTID].getService(Ci.nsILivemarkService);

    // Enforce well-formedness because the existing code does
    if (!result || !result.doc || result.bozo) {
      if (this._livemark.loadingId != -1) {
        this._bms.removeItem(this._livemark.loadingId);
        this._livemark.loadingId = -1;
      }
      MarkLivemarkLoadFailed(this._livemark.folderId);
      this._ttl = gExpiration;
      throw Cr.NS_ERROR_FAILURE;
    }

    // Clear out any child nodes of the livemark folder, since
    // they're about to be replaced.
    this.deleteLivemarkChildren(this._livemark.folderId);
    this._livemark.loadingId = -1;
    // removeItemAnnotation can safely be used even when the anno isn't set
    this._ans.removeItemAnnotation(this._livemark.folderId, LMANNO_LOADFAILED);
    var feed = result.doc.QueryInterface(Ci.nsIFeed);
    if (feed.link) {
      var oldSiteURI = lmService.getSiteURI(this._livemark.folderId);
      if (!oldSiteURI || !feed.link.equals(oldSiteURI))
        lmService.setSiteURI(this._livemark.folderId, feed.link);
    }
    // Loop through and check for a link and a title
    // as the old code did
    for (var i = 0; i < feed.items.length; ++i) {
      let entry = feed.items.queryElementAt(i, Ci.nsIFeedEntry);
      let href = entry.link;
      if (!href)
        continue;

      let title = entry.title ? entry.title.plainText() : entry.updated;
      if (!title)
        continue;

      try {
        secMan.checkLoadURIWithPrincipal(feedPrincipal, href, SEC_FLAGS);
      }
      catch(ex) {
        continue;
      }

      this.insertLivemarkChild(this._livemark.folderId, href, title);
    }
  },

  /**
   * See nsIFeedResultListener.idl
   */
  handleResult: function LLL_handleResult(result) {
    if (this._isAborted) {
      if (this._livemark.loadingId != -1) {
        this._bms.removeItem(this._livemark.loadingId);
        this._livemark.loadingId = -1;
      }
      MarkLivemarkLoadFailed(this._livemark.folderId);
      this._livemark.locked = false;
      return;
    }
    try {
      // The actual work is done in runBatched, see above.
      this._bms.runInBatchMode(this, result);
    }
    finally {
      this._processor.listener = null;
      this._processor = null;
      this._livemark.locked = false;
    }
  },

  deleteLivemarkChildren: LivemarkService.prototype.deleteLivemarkChildren,

  insertLivemarkChild:
  function LS_insertLivemarkChild(folderId, uri, title) {
    var id = this._bms.insertBookmark(folderId, uri, this._bms.DEFAULT_INDEX,
                                      title);
  },

  /**
   * See nsIStreamListener.idl
   */
  onDataAvailable: function LLL_onDataAvailable(request, context, inputStream,
                                                sourceOffset, count) {
    this._processor.onDataAvailable(request, context, inputStream,
                                    sourceOffset, count);
  },

  /**
   * See nsIRequestObserver.idl
   */
  onStartRequest: function LLL_onStartRequest(request, context) {
    if (this._isAborted)
      throw Cr.NS_ERROR_UNEXPECTED;

    var channel = request.QueryInterface(Ci.nsIChannel);

    // Parse feed data as it comes in
    this._processor = Cc[FP_CONTRACTID].createInstance(Ci.nsIFeedProcessor);
    this._processor.listener = this;
    this._processor.parseAsync(null, channel.URI);

    this._processor.onStartRequest(request, context);
  },

  /**
   * See nsIRequestObserver.idl
   */
  onStopRequest: function LLL_onStopRequest(request, context, status) {
    if (!Components.isSuccessCode(status)) {
      // Something went wrong; try to load again in a bit
      this._setResourceTTL(ERROR_EXPIRATION);
      this._isAborted = true;
      if (this._livemark.loadingId != -1) {
        this._bms.removeItem(this._livemark.loadingId);
        this._livemark.loadingId = -1;
      }
      MarkLivemarkLoadFailed(this._livemark.folderId);
      this._livemark.locked = false;
      return;
    }
    // Set an expiration on the livemark, for reloading the data
    try {
      this._processor.onStopRequest(request, context, status);

      // Calculate a new ttl
      var channel = request.QueryInterface(Ci.nsICachingChannel);
      if (channel) {
        var entryInfo = channel.cacheToken.QueryInterface(Ci.nsICacheEntryInfo);
        if (entryInfo) {
          // nsICacheEntryInfo returns value as seconds,
          // expiresTime stores as ms
          var expiresTime = entryInfo.expirationTime * 1000;
          var nowTime = Date.now();

          // note, expiresTime can be 0, see bug #383538
          if (expiresTime > nowTime) {
            this._setResourceTTL(Math.max((expiresTime - nowTime),
                                 gExpiration));
            return;
          }
        }
      }
    }
    catch (ex) { }
    this._setResourceTTL(this._ttl);
  },

  _setResourceTTL: function LLL__setResourceTTL(milliseconds) {
    var exptime = Date.now() + milliseconds;
    this._ans.setPageAnnotation(this._livemark.feedURI, LMANNO_EXPIRATION,
                                exptime, 0,
                                Ci.nsIAnnotationService.EXPIRE_NEVER);
  },

  /**
   * See nsIBadCertListener2
   */
  notifyCertProblem: function LLL_certProblem(socketInfo, status, targetSite) {
    return true;
  },

  /**
   * See nsISSLErrorListener
   */
  notifySSLError: function LLL_SSLError(socketInfo, error, targetSite) {
    return true;
  },

  /**
   * See nsIInterfaceRequestor
   */
  getInterface: function LLL_getInterface(iid) {
    return this.QueryInterface(iid);
  },

  /**
   * See nsISupports.idl
   */
  QueryInterface: function LLL_QueryInterface(iid) {
    if (iid.equals(Ci.nsIFeedResultListener) ||
        iid.equals(Ci.nsIStreamListener) ||
        iid.equals(Ci.nsIRequestObserver)||
        iid.equals(Ci.nsINavHistoryBatchCallback) ||
        iid.equals(Ci.nsIBadCertListener2) ||
        iid.equals(Ci.nsISSLErrorListener) ||
        iid.equals(Ci.nsIInterfaceRequestor) ||
        iid.equals(Ci.nsISupports))
      return this;
    throw Cr.NS_ERROR_NO_INTERFACE;
  },
}

function GenericComponentFactory(ctor) {
  this._ctor = ctor;
}

GenericComponentFactory.prototype = {

  _ctor: null,

  // nsIFactory
  createInstance: function(outer, iid) {
    if (outer != null)
      throw Cr.NS_ERROR_NO_AGGREGATION;
    return (new this._ctor()).QueryInterface(iid);
  },

  // nsISupports
  QueryInterface: function(iid) {
    if (iid.equals(Ci.nsIFactory) ||
        iid.equals(Ci.nsISupports))
      return this;
    throw Cr.NS_ERROR_NO_INTERFACE;
  },

};

var Module = {
  QueryInterface: function(iid) {
    if (iid.equals(Ci.nsIModule) ||
        iid.equals(Ci.nsISupports))
      return this;

    throw Cr.NS_ERROR_NO_INTERFACE;
  },

  getClassObject: function M_getClassObject(cm, cid, iid) {
    if (!iid.equals(Ci.nsIFactory))
      throw Cr.NS_ERROR_NOT_IMPLEMENTED;
    if (cid.equals(LS_CLASSID))
      return new GenericComponentFactory(LivemarkService);

    throw Cr.NS_ERROR_NO_INTERFACE;
  },

  registerSelf: function(cm, file, location, type) {
    var cr = cm.QueryInterface(Ci.nsIComponentRegistrar);

    cr.registerFactoryLocation(LS_CLASSID, LS_CLASSNAME,
      LS_CONTRACTID, file, location, type);
  },

  unregisterSelf: function M_unregisterSelf(cm, location, type) {
    var cr = cm.QueryInterface(Ci.nsIComponentRegistrar);
    cr.unregisterFactoryLocation(LS_CLASSID, location);
  },

  canUnload: function M_canUnload(cm) {
    return true;
  }
};

function NSGetModule(cm, file) {
  return Module;
}
