/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
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
# The Original Code is the Extension Manager.
#
# The Initial Developer of the Original Code is mozilla.org
# Portions created by the Initial Developer are Copyright (C) 2008
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#   Dave Townsend <dtownsend@oxymoronical.com>
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
*/

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

const PREF_GETADDONS_BROWSEADDONS        = "extensions.getAddons.browseAddons";
const PREF_GETADDONS_BROWSERECOMMENDED   = "extensions.getAddons.recommended.browseURL";
const PREF_GETADDONS_GETRECOMMENDED      = "extensions.getAddons.recommended.url";
const PREF_GETADDONS_BROWSESEARCHRESULTS = "extensions.getAddons.search.browseURL";
const PREF_GETADDONS_GETSEARCHRESULTS    = "extensions.getAddons.search.url";

const XMLURI_PARSE_ERROR  = "http://www.mozilla.org/newlayout/xml/parsererror.xml"

function AddonSearchResult() {
}

AddonSearchResult.prototype = {
  id: null,
  name: null,
  version: null,
  summary: null,
  description: null,
  rating: null,
  iconURL: null,
  thumbnailURL: null,
  homepageURL: null,
  eula: null,
  type: null,
  xpiURL: null,
  xpiHash: null,

  QueryInterface: XPCOMUtils.generateQI([Ci.nsIAddonSearchResult])
}

function AddonRepository() {
}

AddonRepository.prototype = {
  // The current set of results
  _addons: [],

  // Whether we are currently searching or not
  _searching: false,

  // Is this a search for recommended add-ons
  _recommended: false,

  // XHR associated with the current request
  _request: null,

  // How many times in succession has a search yielded no new results. Stops
  // us trying to find more results indefinately
  _emptyCount: null,

  // Callback object to notify on completion
  _callback: null,

  // The uri to pull results from
  _retrieveURI: null,

  // Maximum number of results to return
  _maxResults: null,

  get homepageURL() {
    return Components.classes["@mozilla.org/toolkit/URLFormatterService;1"]
                     .getService(Components.interfaces.nsIURLFormatter)
                     .formatURLPref(PREF_GETADDONS_BROWSEADDONS);
  },

  get isSearching() {
    return this._searching;
  },

  getRecommendedURL: function() {
    var urlf = Components.classes["@mozilla.org/toolkit/URLFormatterService;1"]
                         .getService(Components.interfaces.nsIURLFormatter);

    return urlf.formatURLPref(PREF_GETADDONS_BROWSERECOMMENDED);
  },

  getSearchURL: function(aSearchTerms) {
    var prefs = Components.classes["@mozilla.org/preferences-service;1"]
                          .getService(Components.interfaces.nsIPrefBranch);
    var urlf = Components.classes["@mozilla.org/toolkit/URLFormatterService;1"]
                         .getService(Components.interfaces.nsIURLFormatter);

    var url = prefs.getCharPref(PREF_GETADDONS_BROWSESEARCHRESULTS);
    url = url.replace(/%TERMS%/g, encodeURIComponent(aSearchTerms));
    return urlf.formatURL(url);
  },

  cancelSearch: function() {
    this._searching = false;
    if (this._request) {
      this._request.abort();
      this._request = null;
    }
  },

  retrieveRecommendedAddons: function(aMaxResults, aCallback) {
    if (this._searching)
      return;

    this._searching = true;
    this._addons = [];
    this._callback = aCallback;
    this._recommended = true;
    this._emptyCount = 0;
    this._maxResults = aMaxResults;

    var urlf = Components.classes["@mozilla.org/toolkit/URLFormatterService;1"]
                         .getService(Components.interfaces.nsIURLFormatter);

    this._retrieveURI = urlf.formatURLPref(PREF_GETADDONS_GETRECOMMENDED);
    this._loadList();
  },

  searchAddons: function(aSearchTerms, aMaxResults, aCallback) {
    if (this._searching)
      return;

    this._searching = true;
    this._addons = [];
    this._callback = aCallback;
    this._recommended = false;
    this._emptyCount = 0;
    this._maxResults = aMaxResults;

    var prefs = Components.classes["@mozilla.org/preferences-service;1"]
                          .getService(Components.interfaces.nsIPrefBranch);
    var urlf = Components.classes["@mozilla.org/toolkit/URLFormatterService;1"]
                         .getService(Components.interfaces.nsIURLFormatter);

    this._retrieveURI = prefs.getCharPref(PREF_GETADDONS_GETSEARCHRESULTS);
    this._retrieveURI = this._retrieveURI.replace(/%TERMS%/g, encodeURIComponent(aSearchTerms));
    this._retrieveURI = urlf.formatURL(this._retrieveURI);
    this._loadList();
  },

  // Posts results to the callback
  _reportSuccess: function(aCount) {
    this._searching = false;
    this._request = null;
    this._callback.searchSucceeded(this._addons, this._addons.length,
                                   this._recommended ? -1 : aCount);
  },

  // Notifies the callback of a failure
  _reportFailure: function(aEvent) {
    this._searching = false;
    this._request = null;
    this._callback.searchFailed();
  },

  // Parses an add-on entry from an <addon> element
  _parseAddon: function(element) {
    var em = Cc["@mozilla.org/extensions/manager;1"].
             getService(Ci.nsIExtensionManager);
    var app = Cc["@mozilla.org/xre/app-info;1"].
              getService(Ci.nsIXULAppInfo).
              QueryInterface(Ci.nsIXULRuntime);

    var guid = element.getElementsByTagName("guid");
    if (guid.length != 1)
      return;

    // Ignore add-ons already seen in the results
    for (var i = 0; i < this._addons.length; i++)
      if (this._addons[i].id == guid[0].textContent)
        return;

    // Ignore installed add-ons
    if (em.getItemForID(guid[0].textContent) != null)
      return;

    // Ignore sandboxed add-ons
    var status = element.getElementsByTagName("status");
    // The status element has a unique id for each status type. 4 is Public.
    if (status.length != 1 || status[0].getAttribute("id") != 4)
      return;

    // Ignore add-ons not compatible with this OS
    var compatible = false;
    var os = element.getElementsByTagName("compatible_os");
    var i = 0;
    while (i < os.length && !compatible) {
      if (os[i].textContent == "ALL" || os[i].textContent == app.OS) {
        compatible = true;
        break;
      }
      i++;
    }
    if (!compatible)
      return;

    // Ignore add-ons not compatible with this Application
    compatible = false;
    var tags = element.getElementsByTagName("compatible_applications");
    if (tags.length != 1)
      return;
    var vc = Cc["@mozilla.org/xpcom/version-comparator;1"].
             getService(Ci.nsIVersionComparator);
    var apps = tags[0].getElementsByTagName("name");
    var i = 0;
    while (i < apps.length) {
      if (apps[i].textContent.toLowerCase() == app.name.toLowerCase()) {
        var minversion = apps[i].parentNode.getElementsByTagName("min_version")[0].textContent;
        var maxversion = apps[i].parentNode.getElementsByTagName("max_version")[0].textContent;
        if ((vc.compare(minversion, app.version) > 0) ||
            (vc.compare(app.version, maxversion) > 0))
          return;
        compatible = true;
        break;
      }
      i++;
    }
    if (!compatible)
      return;

    var addon = new AddonSearchResult();
    addon.id = guid[0].textContent;
    addon.rating = -1;
    var node = element.firstChild;
    while (node) {
      if (node instanceof Ci.nsIDOMElement) {
        switch (node.localName) {
          case "name":
          case "version":
          case "summary":
          case "description":
          case "eula":
            addon[node.localName] = node.textContent;
            break;
          case "rating":
            if (node.textContent.length > 0)
              addon.rating = parseInt(node.textContent);
            break;
          case "thumbnail":
            addon.thumbnailURL = node.textContent;
            break;
          case "icon":
            addon.iconURL = node.textContent;
            break;
          case "learnmore":
            addon.homepageURL = node.textContent;
            break;
          case "type":
            // The type element has an id attribute that is the id from AMO's
            // database. This doesn't match our type values to perform a mapping
            if (node.getAttribute("id") == 2)
              addon.type = Ci.nsIUpdateItem.TYPE_THEME;
            else
              addon.type = Ci.nsIUpdateItem.TYPE_EXTENSION;
            break;
          case "install":
            addon.xpiURL = node.textContent;
            if (node.hasAttribute("hash"))
              addon.xpiHash = node.getAttribute("hash");
            break;
        }
      }
      node = node.nextSibling;
    }

    this._addons.push(addon);
  },

  // Called when a single request has completed, parses out any add-ons and
  // either notifies the callback or does a new request for more results
  _listLoaded: function(aEvent) {
    var request = aEvent.target;
    var responseXML = request.responseXML;

    if (!responseXML || responseXML.documentElement.namespaceURI == XMLURI_PARSE_ERROR ||
        (request.status != 200 && request.status != 0)) {
      this._reportFailure();
      return;
    }
    var elements = responseXML.documentElement.getElementsByTagName("addon");
    var oldcount = this._addons.length;
    for (var i = 0; i < elements.length; i++) {
      this._parseAddon(elements[i]);

      var prefs = Components.classes["@mozilla.org/preferences-service;1"]
                            .getService(Components.interfaces.nsIPrefBranch);
      if (this._addons.length == this._maxResults) {
        this._reportSuccess(elements.length);
        return;
      }
    }

    // We didn't find any new add-ons this pass
    if (oldcount == this._addons.length)
      this._emptyCount++;
    else
      this._emptyCount = 0;

    /* We should keep trying to find new recommended add-ons, but bail if we
       don't get a new result for a few passes. */
    if (this._recommended && this._emptyCount < 5)
      this._loadList();
    else
      this._reportSuccess(elements.length);
  },

  // Performs a new request for results
  _loadList: function() {
    this._request = Cc["@mozilla.org/xmlextras/xmlhttprequest;1"].
                    createInstance(Ci.nsIXMLHttpRequest);
    this._request.open("GET", this._retrieveURI, true);
    this._request.overrideMimeType("text/xml");

    var self = this;
    this._request.onerror = function(event) { self._reportFailure(event); };
    this._request.onload = function(event) { self._listLoaded(event); };
    this._request.send(null);
  },

  classDescription: "Addon Repository",
  contractID: "@mozilla.org/extensions/addon-repository;1",
  classID: Components.ID("{8eaaf524-7d6d-4f7d-ae8b-9277b324008d}"),
  QueryInterface: XPCOMUtils.generateQI([Ci.nsIAddonRepository])
}

function NSGetModule(aCompMgr, aFileSpec) {
  return XPCOMUtils.generateModule([AddonRepository]);
}
