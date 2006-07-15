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
* JSObjectView --------------------------------------------
*  The Tree View class for the JS Object Viewer.
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
* REQUIRED IMPORTS:
*   chrome://inspector/content/jsutil/xpcom/XPCU.js
****************************************************************/

//////////// global variables /////////////////////

//////////// global constants ////////////////////

////////////////////////////////////////////////////////////////////////////
//// class JSObjectView

function JSObjectView(aObject)
{
  this.mJSObject = aObject;
}

JSObjectView.prototype = 
{
  ////////////////////////////////////////////////////////////////////////////
  //// Initialization
  
  ////////////////////////////////////////////////////////////////////////////
  //// interface nsITreeView

  mTree: null,
  mProperties: [],
  mLevels: [],
  mOrder: [],
  
  get rowCount() {
    return this.mObjects.length;
  },
  
/*  get selection() {

  },

  set selection(aVal) {

  },
  
*/  
  getRowProperties: function(aIndex, aProperties)
  {
  },
  
  getCellProperties: function(aIndex, aCol, aProperties)
  {
  },
  
  getColumnProperties: function(aCol, aProperties)
  {
  },

  isContainer: function(aIndex)
  {
  },
  
  isContainerOpen: function(aIndex)
  {
  },
  
  isContainerEmpty: function(aIndex)
  {
  },
  
  isSeparator: function(aIndex)
  {
  },

  isSorted: function()
  {
    return false;
  },

  getParentIndex: function(aRowIndex)
  {
    return 1;
  },
  
  hasNextSibling: function(aRowIndex, aAfterIndex)
  {
    return false;
  },
  
  getLevel: function(aIndex)
  {
  },

  getImageSrc: function(aRow, aCol)
  {
  },

  getProgressMode: function(aRow, aCol)
  {
  },

  getCellValue: function(aRow, aCol)
  {
  },

  getCellText: function(aRow, aCol)
  {
    var object = null;
  
    switch (aCol.id) {
      case "olrCol1":
        return 1;
        break;
      case "olrCol2":
        return 2;
        break;    
    };
  },
  
  setTree: function(aTree)
  {
    this.mTree = aTree;
  },
  
  toggleOpenState: function(aIndex)
  {
  },
  
  cycleHeader: function(aCol)
  {
  },
  
  selectionChanged: function()
  {
  },
  
  cycleCell: function(aRow, aCol)
  {
  },
  
  isEditable: function(aRow, aCol)
  {
  },

  isSelectable: function(aRow, aCol)
  {
  },
  
  setCellValue: function(aRow, aCol, aValue)
  {
  },

  setCellText: function(aRow, aCol, aValue)
  {
  },
  
  performAction: function(aAction)
  {
  },
  
  performActionOnRow: function(aAction, aRow)
  {
  },
  
  performActionOnCell: function(aAction, aRow, aCol)
  {
  },
  
  ////////////////////////////////////////////////////////////////////////////
  //// jsObjectView
  

  
};