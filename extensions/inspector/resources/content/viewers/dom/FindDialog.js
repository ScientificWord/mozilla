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
* FindDialog ---------------------------------------------------
*  Controls the dialog box used for searching the DOM.
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
* REQUIRED IMPORTS:
*   chrome://inspector/content/jsutil/xpcom/XPCU.js
*   chrome://inspector/content/jsutil/rdf/RDFU.js
****************************************************************/

//////////// global variables /////////////////////

var dialog;

//////////// global constants ////////////////////

//////////////////////////////////////////////////

window.addEventListener("load", FindDialog_initialize, false);

function FindDialog_initialize()
{
  dialog = new FindDialog();
}

////////////////////////////////////////////////////////////////////////////
//// class FindDialog

function FindDialog()
{
  this.setDirection(window.arguments[1] ? window.arguments[1] : "down");
  if (window.arguments[2]) {
    this.setValue(window.arguments[2][0], window.arguments[2][1])
  }
  this.toggleType(window.arguments[0] ? window.arguments[0] : "id");
  this.mOpener = window.opener.viewer;
  
  var txf = document.getElementById("tfText1");
  txf.select();
  txf.focus();
}

FindDialog.prototype = 
{
  mType: null,

  ////////// Properties


  ////////// Methods

  doFind: function()
  {
    var el = document.getElementById("tfText1");
    var dir = document.getElementById("rgDirection").value;
    if (this.mType == "id") {
      this.mOpener.startFind("id", dir, el.value);
    } else if (this.mType == "tag") {
      this.mOpener.startFind("tag", dir, el.value);
    } else if (this.mType == "attr") {
      var el2 = document.getElementById("tfText2");
      this.mOpener.startFind("attr", dir, el.value, el2.value);
    }
  },

  toggleType: function(aType)
  {
    this.mType = aType;

    if (aType == "id") {
      this.showDirection(false);
      this.setLabel1(0);
      this.showRow2(false);
    } else if (aType == "tag") {
      this.showDirection(true);
      this.setLabel1(1);
      this.showRow2(false);
    } else if (aType == "attr") {
      this.showDirection(true);
      this.setLabel1(2);
      this.showRow2(true);
    }

    var rd = document.getElementById("rdType_"+aType.toLowerCase());
    if (rd) {
      var rg = document.getElementById("rgType");
      rg.selectedItem = rd;
    }

  },

  setLabel1: function(aIndex)
  {
    var deck = document.getElementById("rwRow1Text");
    deck.setAttribute("selectedIndex", aIndex);
  },

  showRow2: function(aTruth)
  {
    var row = document.getElementById("rwRow2");
    row.setAttribute("hide", !aTruth);
  },
  
  setDirection: function(aMode)
  {
    var rd = document.getElementById("rdDir_"+aMode.toLowerCase());
    if (rd) {
      var rg = document.getElementById("rgDirection");
      rg.selectedItem = rd;
    }
  },
  
  setValue: function(aValue1, aValue2)
  {
    var txf;
    if (aValue1) {
      txf = document.getElementById("tfText1");
      txf.value = aValue1;
    }
    if (aValue2) {
      txf = document.getElementById("tfText2");
      txf.value = aValue2;
    }
  },
  
  showDirection: function(aTruth)
  {
    document.getElementById("rdDir_up").disabled = !aTruth;
    document.getElementById("rdDir_down").disabled = !aTruth;
  }

};

