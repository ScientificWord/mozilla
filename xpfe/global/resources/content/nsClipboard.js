/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ben Matthew Goodger <ben@netscape.com>
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

/** 
 * nsClipboard - wrapper around nsIClipboard and nsITransferable
 *               that simplifies access to the clipboard. 
 **/ 
var nsClipboard = {
  _CB: null,
  get mClipboard()
    {
      if (!this._CB) 
        {
          const kCBContractID = "@mozilla.org/widget/clipboard;1";
          const kCBIID = Components.interfaces.nsIClipboard;
          this._CB = Components.classes[kCBContractID].getService(kCBIID);
        }
      return this._CB;
    },
    
  currentClipboard: null,
  /** 
   * Array/Object read (Object aFlavourList, long aClipboard, Bool aAnyFlag) ;
   *
   * returns the data in the clipboard
   * 
   * @param FlavourSet aFlavourSet
   *        formatted list of desired flavours
   * @param long aClipboard
   *        the clipboard to read data from (kSelectionClipboard/kGlobalClipboard)
   * @param Bool aAnyFlag
   *        should be false.
   **/
  read: function (aFlavourList, aClipboard, aAnyFlag)
    {
      this.currentClipboard = aClipboard;
      var data = nsTransferable.get(aFlavourList, this.getClipboardTransferable, aAnyFlag);
      return data.first.first;  // only support one item
    },
    
  /**
   * nsISupportsArray getClipboardTransferable (Object aFlavourList) ;
   * 
   * returns a nsISupportsArray of the item on the clipboard
   *
   * @param Object aFlavourList
   *        formatted list of desired flavours.
   **/
  getClipboardTransferable: function (aFlavourList)
    {
      const supportsContractID = "@mozilla.org/supports-array;1";
      const supportsIID = Components.interfaces.nsISupportsArray;
      var supportsArray = Components.classes[supportsContractID].createInstance(supportsIID);
      var trans = nsTransferable.createTransferable();
      for (var flavour in aFlavourList) 
        trans.addDataFlavor(flavour);
      nsClipboard.mClipboard.getData(trans, nsClipboard.currentClipboard)
      supportsArray.AppendElement(trans);
      return supportsArray;
    }
};

