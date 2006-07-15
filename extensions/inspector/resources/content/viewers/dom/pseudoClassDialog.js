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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Joe Hewitt <hewitt@netscape.com> (original author)
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

/***************************************************************
* PseudoClassDialog --------------------------------------------
*  A dialog for choosing the pseudo-classes that should be 
*  imitated on the selected element.
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
* REQUIRED IMPORTS:
****************************************************************/

//////////// global variables /////////////////////

var dialog;

//////////// global constants ////////////////////

const gCheckBoxIds = {
  cbxStateHover: 4,
  cbxStateActive: 1,
  cbxStateFocus: 2
};
/////////////////////////////////////////////////

window.addEventListener("load", PseudoClassDialog_initialize, false);

function PseudoClassDialog_initialize()
{
  dialog = new PseudoClassDialog();
  dialog.initialize();
}

////////////////////////////////////////////////////////////////////////////
//// class PseudoClassDialog

function PseudoClassDialog() 
{
  this.mOpener = window.opener.viewer;
  this.mSubject = window.arguments[0];

  this.mDOMUtils = XPCU.getService("@mozilla.org/inspector/dom-utils;1", "inIDOMUtils");
}

PseudoClassDialog.prototype = 
{
  
  initialize: function()
  {
    var state = this.mDOMUtils.getContentState(this.mSubject);
    
    for (var key in gCheckBoxIds) {
      if (gCheckBoxIds[key] & state) {
        var cbx = document.getElementById(key);
        cbx.setAttribute("checked", "true");
      }
    }
  },
  
  onOk: function()
  {
    var el = this.mSubject;
    var root = el.ownerDocument.documentElement;
    
    for (var key in gCheckBoxIds) {
      var cbx = document.getElementById(key);
      if (cbx.checked) 
        this.mDOMUtils.setContentState(el, gCheckBoxIds[key]);
      else
        this.mDOMUtils.setContentState(root, gCheckBoxIds[key]);
    }
  }
  
};

