/*
 * Copyright (c) 2009 Samuel Gross.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

const Cc = Components.classes;
const Ci = Components.interfaces;

const PREF_DISABLED_PLUGIN_TYPES = "plugin.disable_full_page_plugin_for_types";
const PDF_MIME_TYPE = "application/pdf";
const PROFILE_AFTER_CHANGE = "profile-after-change";


// This file is executed when the user first installs the PDF plugin extenion.
//
// We remove preferences for handling PDF files that would interfere with the
// plugin. For example, the user may have set PDF files to save without asking.
//
// NOTE: This does not overwrite preferences set after the plugin is installed.
// The user can still disable or uninstall the plugin through the add-ons
// window, or change the file handling preferences through the "Applications"
// preferences tab.


function NSGetModule() {
  return {
    log: function(s) {
      var logger = Cc["@mozilla.org/consoleservice;1"].
          getService(Ci.nsIConsoleService);
  
      logger.logStringMessage('' + s);
    },

    registerSelf: function(compMgr, location, loaderStr, type) {
      this.log('NSGetModule');

      this.observerSvc = Cc["@mozilla.org/observer-service;1"].
          getService(Ci.nsIObserverService);

      // We need to wait until the profile is loaded in order to modify
      // user and mime-type preferences.
      this.observerSvc.addObserver(this, PROFILE_AFTER_CHANGE, false);
    },

    observe: function(subject, topic, data) {
      this.observerSvc.removeObserver(this, PROFILE_AFTER_CHANGE);

      try {
        this.enablePdfType();
      } catch (e) {
        this.log("Error enabling PDF type: " + e);
      }

      try {
        this.removePdfHandler();
      } catch (e) {
        this.log("Error enabling PDF type: " + e);
      }
    },

    /** Enables the PDF plugin. */
    enablePdfType: function() {
      var prefSvc = Cc["@mozilla.org/preferences-service;1"].
              getService(Ci.nsIPrefBranch);

      if (prefSvc.prefHasUserValue(PREF_DISABLED_PLUGIN_TYPES)) {
        var types = prefSvc.getCharPref(PREF_DISABLED_PLUGIN_TYPES);

        if (types) {
          var filtered = types.split(",").filter(function(v) v != PDF_MIME_TYPE);

          prefSvc.setCharPref(PREF_DISABLED_PLUGIN_TYPES, filtered.join(","));
        }
      }
    },

    /** Removes any existing handler for the application/pdf mime-type. */
    removePdfHandler: function() {
      var handlerSvc = Cc["@mozilla.org/uriloader/handler-service;1"].
          getService(Ci.nsIHandlerService);
    
      var mimeSvc = Cc["@mozilla.org/mime;1"].
          getService(Ci.nsIMIMEService);

      let mimeInfo = mimeSvc.getFromTypeAndExtension(PDF_MIME_TYPE, null);

      if (mimeInfo && handlerSvc.exists(mimeInfo)) {
        handlerSvc.remove(mimeInfo);
      }
    }
  }
}

