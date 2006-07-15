/* -*- Mode: Java; tab-width: 4; c-basic-offset: 2; -*-
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
 * The Original Code is mozilla.org Code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

function initPanel() {
  var tag = document.getElementById("panelFrame").getAttribute("tag");
  if (hWalletViewer) {
    hWalletViewer.onpageload(tag)
  } else {
    window.queuedTag = tag;
    window.queuedTagPending = true;
  }
}

var hWalletViewer = null;
var walletViewerInterface = null;
var walletServiceInterface = null;
var bundle = null; // string bundle
var JS_STRINGS_FILE = "chrome://communicator/locale/wallet/WalletEditor.properties";
var schemaToValue = [];
var BREAK = "|";

function onLoad()
{
  hWalletViewer = new nsWalletViewer('panelFrame');

  if (!hWalletViewer)
    throw "failed to create walletviewer";
  else
    hWalletViewer.init();
}

function nsWalletViewer(frame_id)
{
  if (!frame_id) {
    throw "Error: frame_id not supplied!";
  }
  this.contentFrame = frame_id
  this.cancelHandlers = [];
  this.okHandlers = [];

  // set up window
  this.onload();
}

nsWalletViewer.prototype =
  {
    onload:
      function() {
        walletViewerInterface = Components.classes["@mozilla.org/walleteditor/walleteditor-world;1"].createInstance();
        walletViewerInterface = walletViewerInterface.QueryInterface(Components.interfaces.nsIWalletEditor);

        walletServiceInterface = Components.classes['@mozilla.org/wallet/wallet-service;1'];
        walletServiceInterface = walletServiceInterface.getService();
        walletServiceInterface = walletServiceInterface.QueryInterface(Components.interfaces.nsIWalletService);

        bundle = srGetStrBundle(JS_STRINGS_FILE); /* initialize string bundle */

        if (!EncryptionTest()) {
          dump("*** user failed to unlock the database\n");
          return;
        }
        if (!FetchInput()) {
          dump("*** user failed to unlock the database\n");
          return;
        }

        // allow l10n to hide certain panels
        var pref = Components.classes["@mozilla.org/preferences-service;1"]
                             .getService(Components.interfaces.nsIPrefBranch);
        var panel;
        try {
          if (pref.getBoolPref("wallet.namePanel.hide")) {
            panel = document.getElementById("pnameID");
            panel.setAttribute("hidden", "true");
            panel = document.getElementById("snameID");
            panel.setAttribute("hidden", "true");
            panel = document.getElementById("bnameID");
            panel.setAttribute("hidden", "true");
          }
          if (pref.getBoolPref("wallet.addressPanel.hide")) {
            panel = document.getElementById("paddressID");
            panel.setAttribute("hidden", "true");
            panel = document.getElementById("saddressID");
            panel.setAttribute("hidden", "true");
            panel = document.getElementById("baddressID");
            panel.setAttribute("hidden", "true");
          }
          if (pref.getBoolPref("wallet.phonePanel.hide")) {
            panel = document.getElementById("pphoneID");
            panel.setAttribute("hidden", "true");
            panel = document.getElementById("sphoneID");
            panel.setAttribute("hidden", "true");
            panel = document.getElementById("bphoneID");
            panel.setAttribute("hidden", "true");
          }
          if (pref.getBoolPref("wallet.creditPanel.hide")) {
            panel = document.getElementById("pcreditID");
            panel.setAttribute("hidden", "true");
          }
          if (pref.getBoolPref("wallet.employPanel.hide")) {
            panel = document.getElementById("pemployID");
            panel.setAttribute("hidden", "true");
          }
          if (pref.getBoolPref("wallet.miscPanel.hide")) {
            panel = document.getElementById("pmiscID");
            panel.setAttribute("hidden", "true");
          }
        } catch(e) {
          // error -- stop hiding if prefs are missing
        }
      },

      init:
        function() {
          if (window.queuedTagPending) {
            this.onpageload(window.queuedTag);
          }
          this.closeBranches("pnameID");
        },

    onAccept:
        function() {
          for(var i = 0; i < hWalletViewer.okHandlers.length; i++) {
            hWalletViewer.okHandlers[i]();
          }

          var tag = document.getElementById(hWalletViewer.contentFrame).getAttribute("tag");
          hWalletViewer.savePageData(tag);
          hWalletViewer.saveAllData();

        return true;
        },

      onCancel:
        function() {
          for(var i = 0; i < hWalletViewer.cancelHandlers.length; i++) {
            hWalletViewer.cancelHandlers[i]();
          }

        return true;
        },

      registerOKCallbackFunc:
        function(aFunctionReference) {
          this.okHandlers[this.okHandlers.length] = aFunctionReference;
        },

      registerCancelCallbackFunc:
        function(aFunctionReference) {
          this.cancelHandlers[this.cancelHandlers.length] = aFunctionReference;
        },

      saveAllData:
        function() {
          ReturnOutput();
        },

      savePageData:
        function(tag) {
          /* collect the list of menuItem labels */
          var elementIDs;
          var contentFrame = window.frames[this.contentFrame];
          if ("_elementIDs" in contentFrame) { // make sure page finished loading
            elementIDs = contentFrame._elementIDs;
          }
          for(var i = 0; i < elementIDs.length; i++) {
            var values = "";
            var menuList = contentFrame.document.getElementById(elementIDs[i]);
            if (menuList.parentNode.getAttribute("hidden") == "true") {
              continue; /* needed for concatenations only */
            }
            Append(menuList); /* in case current editing has not been stored away */
            var menuPopup = menuList.firstChild;

            /* visit each menuItem */
            for (var menuItem = menuPopup.firstChild;
                 menuItem != menuPopup.lastChild; /* skip empty item at end of list */
                 menuItem = menuItem.nextSibling) {
              values += (menuItem.getAttribute("label") + BREAK);
            }
            schemaToValue[tag+elementIDs[i]] = values;
          }
        },

      switchPage:
        function() {
          var PanelTree = document.getElementById("panelTree");
          if (PanelTree.view.selection.count == 0) return;
          var selectedItem = PanelTree.contentView.getItemAtIndex(PanelTree.currentIndex);

          var oldURL = document.getElementById(this.contentFrame).getAttribute("src");
          var oldTag = document.getElementById(this.contentFrame).getAttribute("tag");

          this.savePageData(oldTag);      // save data from the current page.

          var newURL = selectedItem.firstChild.firstChild.getAttribute("url");
          var newTag = selectedItem.firstChild.firstChild.getAttribute("tag");
          if (newURL != oldURL || newTag != oldTag) {
            document.getElementById(this.contentFrame).setAttribute("src", newURL);
            document.getElementById(this.contentFrame).setAttribute("tag", newTag);
          }
        },

      onpageload:
        function(aPageTag) {
          if ('Startup' in window.frames[ this.contentFrame ]) {
            window.frames[ this.contentFrame ].Startup();
          }

          /* restore the list of menuItem labels */
          var elementIDs = window.frames[this.contentFrame]._elementIDs;
          for(var i = 0; i < elementIDs.length; i++) {
            var menuList = window.frames[this.contentFrame].document.getElementById(elementIDs[i]);
            if (!menuList) {
              dump("*** FIX ME: '_elementIDs' in '" + aPageTag +
                   "' contains a reference to a non-existent element ID '" +
                   elementIDs[i] + "'.\n");
              return;
            }
            var menuPopup = menuList.firstChild;
            if ((aPageTag+elementIDs[i]) in schemaToValue) {

              /* following unhiding is needed for concatenations only */
              var row = menuList.parentNode;
              var rows = row.parentNode;
              var grid = rows.parentNode;
              var groupBox = grid.parentNode;
              groupBox.setAttribute("hidden", "false");
              row.setAttribute("hidden", "false");

              var strings = schemaToValue[aPageTag+elementIDs[i]].split(BREAK);
              for (var j = 0; j<strings.length-1; j++) {
                var menuItem = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul", "menuitem");
                if (menuItem) {
                  menuItem.setAttribute("label", strings[j]);
                  menuItem.setAttribute("len", strings[j].length);
                  menuPopup.insertBefore(menuItem, menuPopup.lastChild);
                }
              }
            };
            menuList.selectedItem = menuPopup.firstChild; // ????? is this temporary ?????
          }
        },

    closeBranches:
      function(aComponentName) {
        var panelChildren = document.getElementById("panelChildren");
        var panelTree = document.getElementById("panelTree");
        for(var i = 0; i < panelChildren.childNodes.length; i++) {
          var currentItem = panelChildren.childNodes[i];
          if (currentItem.id != aComponentName && currentItem.id != "primary") {
            currentItem.removeAttribute("open");
          }
        }
        var openItem = document.getElementById(aComponentName);
        var index = panelTree.contentView.getIndexOfItem(openItem);
        panelTree.view.selection.select(index);
      }

  };

  function Append(thisMenuList) {

    /* Note: we always want a zero-length textbox so the user
     * can start typing in a new value.  So we need to determine
     * if user has started typing into the zero-length textbox
     * in which case it's time to create yet another zero-length
     * one.  We also need to determine if user has removed all
     * text in a textbox in which case that textbox needs to
     * be removed
     */

    /* transfer value from menu list to the selected menu item */
    var thisMenuItem = thisMenuList.selectedItem;
    if (!thisMenuItem) {
      return;
    }
    thisMenuItem.setAttribute('label', thisMenuList.value);

    /* determine previous size of textbox */
    var len = Number(thisMenuItem.getAttribute("len"));

    /* update previous size */
    var newLen = thisMenuItem.getAttribute("label").length;
    thisMenuItem.setAttribute("len", newLen);

    /* obtain parent element */
    var thisMenuPopup = thisMenuItem.parentNode;
    if (!thisMenuPopup) {
      return;
    }

    /* determine if it's time to remove menuItem */
    if (newLen == 0) {
      /* no characters left in text field */
      if (len) {
        /* previously there were some characters, time to remove */
        thisMenuPopup.parentNode.selectedItem = thisMenuPopup.lastChild;
        thisMenuPopup.removeChild(thisMenuItem);
      }
      return;
    }

    /* currently modified entry is not null so put it at head of list */
    if (thisMenuPopup.childNodes.length > 1) {
      thisMenuPopup.removeChild(thisMenuItem);
      thisMenuPopup.insertBefore(thisMenuItem, thisMenuPopup.firstChild);
    }

    /* determine if it's time to add menuItem */
    if (len) {
      /* previously there were some characters and there still are so it's not time to add */
      return;
    }

    /* add menu item */
    var menuItem = document.createElementNS("http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul", "menuitem");
    if (!menuItem) {
      return;
    }
    menuItem.setAttribute("label", "");
    menuItem.setAttribute("len", "0");
    thisMenuPopup.appendChild(menuItem);

    return;
  }

  /* return the wallet output data */
  function ReturnOutput() {
    var schema;
    var output = "OK" + BREAK;
    var value;
    for (schema in schemaToValue) {
      if (schemaToValue[schema] != "") {
        value = schemaToValue[schema].split(BREAK);
        for (var i=0; i<value.length-1; i++) { /* skip empty value at end */
          output += (schema + BREAK + Encrypt(value[i]) + BREAK + BREAK);
        }
      }
    }
    walletViewerInterface.SetValue(output, window);
  }

  /* clear all data */
  function ClearAll() {

    // clear out all values from database
    schemaToValue = [];

    // clear out values on current page
    var elementIDs = window.frames[hWalletViewer.contentFrame]._elementIDs;
    for(var i = 0; i < elementIDs.length; i++) {
      var menuList = window.frames[hWalletViewer.contentFrame].document.getElementById(elementIDs[i]);
      var menuPopup = menuList.firstChild;

      // remove all menuItems except for last one
      while (menuPopup.childNodes.length > 1) {
        menuPopup.removeChild(menuPopup.firstChild);
      }
      menuList.removeAttribute("label");
      menuList.selectedItem = menuPopup.firstChild;
    }
  }

  /* get the wallet input data */
  function FetchInput() {
    /*  get wallet data into a list */
    var list = walletViewerInterface.GetValue();

    /* format of this list is as follows:
     *
     *    BREAK-CHARACTER
     *    schema-name BREAK-CHARACTER
     *    value BREAK-CHARACTER
     *    synonymous-value-1 BREAK-CHARACTER
     *    ...
     *    synonymous-value-n BREAK-CHARACTER
     *
     * and repeat above pattern for each schema name.  Note that if there are more than
     * one distinct values for a particular schema name, the above pattern is repeated
     * for each such distinct value
     */

    /* check for database being unlocked */
    if (list.length == 0) {
      /* user supplied invalid database key */
      window.close(); // ?????
      return false;
    }

    /* parse the list into the schemas and their corresponding values */
    BREAK = list[0];
    var strings = list.split(BREAK);
    var stringsLength = strings.length;
    var schema, value;
    for (var i=1; i<stringsLength; i++) {
      if (strings[i] != "" && strings[i-1] == "") {
        schema = strings[i++];
        value = Decrypt(strings[i]);
        if (!(schema in schemaToValue)) {
          schemaToValue[schema] = [];
        }
        schemaToValue[schema] += (value + BREAK);
      }
    }

    return true;
  }

  /* decrypt a value */
  function Decrypt(crypt) {
    try {
      return walletServiceInterface.WALLET_Decrypt(crypt);
    } catch (ex) {
      return bundle.GetStringFromName("EncryptionFailure");
    }
  }

  /* encrypt a value */
  function Encrypt(text) {
    try {
      return walletServiceInterface.WALLET_Encrypt(text);
    } catch (ex) {
      alert(bundle.GetStringFromName("UnableToUnlockDatabase"));
      return "";
    }
  }

  /* see if user was able to unlock the database */
  function EncryptionTest() {
    try {
      walletServiceInterface.WALLET_Encrypt("dummy");
    } catch (ex) {
      alert(bundle.GetStringFromName("UnableToUnlockDatabase"));
      window.close(); // ?????
      return false;
    }
    return true;
  }

  // called by the help button overlaid from platformDialogOverlay
  function doHelpButton() {
    openHelp("forms_data");
  }
