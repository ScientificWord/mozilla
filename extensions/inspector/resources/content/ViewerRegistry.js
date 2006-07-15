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
* ViewerRegistry -----------------------------------------------
*  The central registry where information about all installed
*  viewers is kept.  
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
* REQUIRED IMPORTS:
*   chrome://inspector/content/jsutil/xpcom/XPCU.js
*   chrome://inspector/content/jsutil/rdf/RDFU.js
****************************************************************/

//////////// global variables /////////////////////

//////////// global constants ////////////////////

const kViewerURLPrefix = "chrome://inspector/content/viewers/";
const kViewerRegURL  = "chrome://inspector/content/res/viewer-registry.rdf";

////////////////////////////////////////////////////////////////////////////
//// class ViewerRegistry

function ViewerRegistry() // implements inIViewerRegistry
{
  this.mViewerHash = {};
}

ViewerRegistry.prototype = 
{
  ////////////////////////////////////////////////////////////////////////////
  //// interface inIViewerRegistry

  // not yet formalized...
  
  ////////////////////////////////////////////////////////////////////////////
  //// Initialization

  mDS: null,
  mObserver: null,
  mViewerDS: null,
  mViewerHash: null,
  mFilters: null,
  
  get url() { return this.mURL; },

  //// Loading Methods

  load: function(aURL, aObserver)
  {
    this.mURL = aURL;
    this.mObserver = aObserver;
    RDFU.loadDataSource(aURL, new ViewerRegistryLoadObserver(this));
  },

  onError: function(aStatus, aErrorMsg)
  {
    this.mObserver.onViewerRegistryLoadError(aStatus, aErrorMsg);
  },

  onLoad: function(aDS)
  {
    this.mDS = aDS;
    this.prepareRegistry();
    this.mObserver.onViewerRegistryLoad();
  },

  prepareRegistry: function()
  {
    this.mViewerDS = RDFArray.fromContainer(this.mDS, "inspector:viewers", kInspectorNSURI);
    
    // create and cache the filter functions
    var js, fn;
    this.mFilters = [];
    for (var i = 0; i < this.mViewerDS.length; ++i) {
      js = this.getEntryProperty(i, "filter");
      try {
        fn = new Function("object", js);
      } catch (ex) {
        fn = new Function("return false");
        debug("### ERROR - Syntax error in filter for viewer \"" + this.getEntryProperty(i, "description") + "\"\n");
      }
      this.mFilters.push(fn);
    }
  },

  ///////////////////////////////////////////////////////////////////////////
  // Returns the absolute url where the xul file for a viewer can be found.
  //
  // @param long aIndex - the index of the entry representing the viewer
  // @return wstring - the fully cannonized url
  ///////////////////////////////////////////////////////////////////////////
  getEntryURL: function(aIndex)
  {
    var uid = this.getEntryProperty(aIndex, "uid");
    return kViewerURLPrefix + uid + "/" + uid + ".xul";
  },

  //// Lookup Methods

  ///////////////////////////////////////////////////////////////////////////
  // Searches the viewer registry for all viewers that can view a particular
  // object.
  //
  // @param Object aObject - the object being searched against
  // @param String aPanelId - the id of the panel requesting viewers
  // @return nsIRDFResource[] - array of entries in the viewer registry
  ///////////////////////////////////////////////////////////////////////////
  findViewersForObject: function(aObject, aPanelId)
  {
    // check each entry in the registry
    var len = this.mViewerDS.length;
    var entry;
    var urls = [];
    for (var i = 0; i < len; ++i) {
      if (this.getEntryProperty(i, "panels").indexOf(aPanelId) == -1) {
        continue;
      }
      if (this.objectMatchesEntry(aObject, i)) {
        if (this.getEntryProperty(i, "important")) {
          urls.unshift(i); 
        } else {
          urls.push(i);
        }
      }
    }

    return urls;
  },

  ///////////////////////////////////////////////////////////////////////////
  // Determines if an object is eligible to be viewed by a particular viewer.
  //
  // @param Object aObject - the object being checked for eligibility
  // @param long aIndex - the index of the entry
  // @return boolean - true if object can be viewed
  ///////////////////////////////////////////////////////////////////////////
  objectMatchesEntry: function(aObject, aIndex)
  {
    return this.mFilters[aIndex](aObject);
  },

  ///////////////////////////////////////////////////////////////////////////
  // Notifies the registry that a viewer has been instantiated, and that
  // it corresponds to a particular entry in the viewer registry.
  //
  // @param 
  ///////////////////////////////////////////////////////////////////////////
  cacheViewer: function(aViewer, aIndex)
  {
    var uid = this.getEntryProperty(aIndex, "uid");
    this.mViewerHash[uid] = { viewer: aViewer, entry: aIndex };
  },

  uncacheViewer: function(aViewer)
  {
    delete this.mViewerHash[aViewer.uid];
  },
  
  // for previously loaded viewers only
  getViewerByUID: function(aUID)
  {
    return this.mViewerHash[aUID].viewer;
  },

  // for previously loaded viewers only
  getEntryForViewer: function(aViewer)
  {
    return this.mViewerHash[aViewer.uid].entry;
  },

  // for previously loaded viewers only
  getEntryByUID: function(aUID)
  {
    return this.mViewerHash[aUID].aIndex;
  },

  getEntryProperty: function(aIndex, aProp)
  {
    return this.mViewerDS.get(aIndex, aProp);
  },

  getEntryCount: function()
  {
    return this.mViewerDS.length;
  },

  ////////////////////////////////////////////////////////////////////////////
  //// Viewer Registration

  addNewEntry: function(aUID, aDescription, aFilter)
  {
  },

  removeEntry: function(aIndex)
  {
  },

  saveRegistry: function()
  {
  }

};

////////////////////////////////////////////////////////////////////////////
//// Listener Objects

function ViewerRegistryLoadObserver(aTarget) 
{
  this.mTarget = aTarget;
}

ViewerRegistryLoadObserver.prototype = {
  mTarget: null,

  onError: function(aStatus, aErrorMsg) 
  {
    this.mTarget.onError(aStatus, aErrorMsg);
  },

  onDataSourceReady: function(aDS) 
  {
    this.mTarget.onLoad(aDS);
  }
};
