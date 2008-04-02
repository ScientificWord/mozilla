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
 * The Original Code is mozilla.org code for the DOM Inspector.
 *
 * The Initial Developer of the Original Code is
 *   Shawn Wilsher <me@shawnwilsher.com>
 * 
 * Portions created by the Initial Developer are Copyright (C) 2006
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

///////////////////////////////////////////////////////////////////////////////
//// Global Variables

var dialog;

///////////////////////////////////////////////////////////////////////////////
//// Initialization/Destruction

window.addEventListener("load", DomNodeDialog_initialize, false);

function DomNodeDialog_initialize()
{
  dialog = new DomNodeDialog();
  dialog.initialize();
}

///////////////////////////////////////////////////////////////////////////////
//// class DomNodeDialog

function DomNodeDialog()
{
  this.mData  = window.arguments[0];
  this.mTitle = window.arguments[1];
  this.mDoc   = window.arguments[2];

  this.nodeName  = document.getElementById("tx_nodeName");
  this.nodeValue = document.getElementById("tx_nodeValue");
  this.namespace = document.getElementById("tx_namespace");
  this.menulist  = document.getElementById("ml_namespace");
}

DomNodeDialog.prototype =
{
 /**
  * This function initializes the content of the dialog.
  */
  initialize: function initialize()
  {
    document.title = this.mTitle;
    var menuitems  = this.menulist.firstChild.childNodes;
    var defaultNS  = document.getElementById("mi_namespace");
    var customNS   = document.getElementById("mi_custom");
    var accept     = document.documentElement.getButton("accept");

    accept.disabled        = this.mData.name == null;
    this.nodeName.value    = this.mData.name || "";
    this.nodeName.disabled = this.mData.name != null;
    this.nodeValue.value   = this.mData.value || "";
    this.menulist.disabled = !this.enableNamespaces();
    defaultNS.value        = this.mDoc.documentElement.namespaceURI;
    customNS.value         = this.mData.namespaceURI;
    this.menulist.value    = this.mData.namespaceURI;

    this.toggleNamespace();
  },

 /**
  * The function that is called on accept.  Sets data.
  */
  accept: function accept()
  {
    this.mData.name         = this.nodeName.value;
    this.mData.value        = this.nodeValue.value;
    this.mData.namespaceURI = this.namespace.value;
    this.mData.accepted     = true;
    return true;
  },

 /**
  * toggleNamespace toggles the namespace textbox based on the namespace menu.
  */
  toggleNamespace: function toggleNamespace()
  {
    dialog.namespace.disabled = dialog.menulist.selectedItem.id != "mi_custom";
    dialog.namespace.value    = dialog.menulist.value;
  },

 /**
  * enableNamespaces determines if the document accepts namespaces or not
  *
  * @return True if the document can have namespaced attributes, false
  *           otherwise.
  */
  enableNamespaces: function enableNamespaces()
  {
    return this.mDoc.contentType != "text/html";
  },

 /**
  * toggleAccept enables/disables the Accept button when there is/isn't an
  *   attribute name.
  */
  toggleAccept: function toggleAccept()
  {
    document.documentElement.getButton("accept").disabled = 
      dialog.nodeName.value == "";
  }
};
