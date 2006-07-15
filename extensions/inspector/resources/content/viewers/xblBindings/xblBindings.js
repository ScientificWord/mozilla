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
* XBLBindings --------------------------------------------
*  The viewer for the computed css styles on a DOM element.
* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
* REQUIRED IMPORTS:
*   chrome://inspector/content/jsutil/xpcom/XPCU.js
****************************************************************/

//////////// global variables /////////////////////

var viewer;
var gInitContent = false;

//////////// global constants ////////////////////

const kDOMViewCID = "@mozilla.org/inspector/dom-view;1";
const kXBLNSURI = "http://www.mozilla.org/xbl";

//////////////////////////////////////////////////

window.addEventListener("load", XBLBindings_initialize, false);

function XBLBindings_initialize()
{
  viewer = new XBLBindings();
  viewer.initialize(parent.FrameExchange.receiveData(window));
}

////////////////////////////////////////////////////////////////////////////
//// class XBLBindings

function XBLBindings()
{
  this.mURL = window.location;
  this.mObsMan = new ObserverManager(this);
  this.mDOMUtils = XPCU.getService("@mozilla.org/inspector/dom-utils;1", "inIDOMUtils");

  this.mContentTree = document.getElementById("olContent");
  this.mMethodTree = document.getElementById("olMethods");
  this.mPropTree = document.getElementById("olProps");
  this.mHandlerTree = document.getElementById("olHandlers");
  this.mResourceTree = document.getElementById("olResources");
  this.mFunctionTextbox = document.getElementById("txbFunction");
  
  if (gInitContent)
    this.initContent();
}

XBLBindings.prototype = 
{
  ////////////////////////////////////////////////////////////////////////////
  //// Initialization
  
  mSubject: null,
  mPane: null,
  mContentInit: false,
  
  ////////////////////////////////////////////////////////////////////////////
  //// interface inIViewer

  get uid() { return "xblBindings" },
  get pane() { return this.mPane },

  get subject() { return this.mSubject },
  set subject(aObject) 
  {
    this.mSubject = aObject;
    
    this.populateBindings();
    
    var menulist = document.getElementById("mlBindings");
    this.displayBinding(menulist.value);
        
    this.mObsMan.dispatchEvent("subjectChange", { subject: aObject });
  },

  initialize: function(aPane)
  {
    this.mPane = aPane;

    aPane.notifyViewerReady(this);
  },

  destroy: function()
  {
    // persist all attributes on the panels
    var panes = document.getElementsByTagName("multipanel");
    for (var i = 0; i < panes.length; ++i) {
      InsUtil.persistAll(panes[i].id);
    }

    this.mContentTree.treeBoxObject.view = null;
    this.mMethodTree.treeBoxObject.view = null;
    this.mPropTree.treeBoxObject.view = null;
    this.mHandlerTree.treeBoxObject.view = null;
    this.mResourceTree.treeBoxObject.view = null;    
  },
  
  isCommandEnabled: function(aCommand)
  {
    return false;
  },
  
  getCommand: function(aCommand)
  {
    return null;
  },

  ////////////////////////////////////////////////////////////////////////////
  //// event dispatching

  addObserver: function(aEvent, aObserver) { this.mObsMan.addObserver(aEvent, aObserver); },
  removeObserver: function(aEvent, aObserver) { this.mObsMan.removeObserver(aEvent, aObserver); },

  ////////////////////////////////////////////////////////////////////////////
  //// displaying binding info
  
  initContent: function()
  {
    if (this.mContentInit) return;

    window.setTimeout(function(me) {
      // prepare and attach the content DOM datasource
      me.mContentView = XPCU.createInstance(kDOMViewCID, "inIDOMView");
      me.mContentView.whatToShow &= ~(NodeFilter.SHOW_TEXT);
      me.mContentTree.treeBoxObject.view = me.mContentView;

      me.mContentInit = true;
      if (me.mBinding)
        me.displayContent();
    }, 10, this);
  },

  populateBindings: function()
  {
    var urls = this.mDOMUtils.getBindingURLs(this.mSubject);
    var menulist = document.getElementById("mlBindings");

    menulist.removeAllItems();

    var urlCount = urls.length;
    var i;

    for (i = 0; i < urlCount; ++i) {
      var url = urls.queryElementAt(i, Components.interfaces.nsIURI).spec;
      menulist.appendItem(url, url);
    }
    
    menulist.selectedIndex = 0;
  },
  
  displayBinding: function(aURL)
  {
    if (aURL) {
      var doc = document.implementation.createDocument(null, "", null);
      doc.addEventListener("load", gDocLoadListener, true);
      doc.load(aURL, "application/xml");
    
      this.mBindingDoc = doc;
      this.mBindingURL = aURL;
    } else {
      this.mBindingDoc = null;
      this.mBindingURL = null;
      this.doDisplayBinding();
    }
  },
  
  doDisplayBinding: function()
  {
    if (this.mBindingDoc) {
      var url = this.mBindingURL;
      var poundPt = url.indexOf("#");
      var id = url.substr(poundPt+1);
      var bindings = this.mBindingDoc.getElementsByTagName("binding");
      var binding = null;
      for (var i = 0; i < bindings.length; ++i) {
        if (bindings[i].getAttribute("id") == id) {
          binding = bindings[i];
          break;
        }
      }
      this.mBinding = binding;
    } else {
      this.mBinding = null;
    }
        
    this.displayContent();
    this.displayMethods();
    this.displayProperties();
    this.displayHandlers();
    this.displayResources();

    var ml = document.getElementById("mlBindings");    
    var mps = document.getElementById("mpsBinding");
    if (!this.mBinding) {
      ml.setAttribute("disabled", "true");
      mps.setAttribute("collapsed", "true");
    } else {
      ml.removeAttribute("disabled");
      mps.removeAttribute("collapsed");
    }
  },
  
  displayContent: function()
  {
    if (this.mContentInit && this.mBinding) {
      var list = this.mBinding.getElementsByTagName("content");
      if (list.length)
        this.mContentView.rootNode = list[0];
      else
        this.mContentView.rootNode = null;
      document.getElementById("bxContent").setAttribute("disabled", list.length == 0);
    } 
  },
  
  displayMethods: function()
  {
    var bx = this.getBoxObject(this.mMethodTree);
    bx.view = this.mBinding ? new MethodTreeView(this.mBinding) : null;

    var active = this.mBinding && this.mBinding.getElementsByTagName("method").length > 0;
    document.getElementById("bxMethods").setAttribute("disabled", !active);
  },
  
  displayProperties: function()
  {
    var bx = this.getBoxObject(this.mPropTree);
    bx.view = this.mBinding ? new PropTreeView(this.mBinding) : null;

    var active = this.mBinding && this.mBinding.getElementsByTagName("property").length > 0;
    document.getElementById("bxProps").setAttribute("disabled", !active);

    this.displayProperty(null);
  },

  displayHandlers: function()
  {
    var bx = this.getBoxObject(this.mHandlerTree);
    bx.view = this.mBinding ? new HandlerTreeView(this.mBinding) : null;

    var active = this.mBinding && this.mBinding.getElementsByTagName("handler").length > 0;
    document.getElementById("bxHandlers").setAttribute("disabled", !active);
    this.displayHandler(null);
  },

  displayResources: function()
  {
    var bx = this.getBoxObject(this.mResourceTree);
    bx.view = this.mBinding ? new ResourceTreeView(this.mBinding) : null;

    var active = this.mBinding && this.mBinding.getElementsByTagName("resources").length > 0;
    document.getElementById("bxResources").setAttribute("disabled", !active);
  },

  displayMethod: function(aMethod)
  {
    var body = aMethod.getElementsByTagNameNS(kXBLNSURI, "body")[0];
    if (body) {
      this.mFunctionTextbox.value = this.readDOMText(body);
    }
  },

  displayProperty: function(aProp)
  {
    var getradio = document.getElementById("raPropGetter");
    var setradio = document.getElementById("raPropSetter");
    
    // disable/enable radio buttons
    var hasget = aProp && (aProp.hasAttribute("onget") || aProp.getElementsByTagName("getter").length);
    getradio.disabled = !hasget;
    if (!hasget && getradio.hasAttribute("selected"))
      getradio.removeAttribute("selected");
    var hasset = aProp && (aProp.hasAttribute("onset") || aProp.getElementsByTagName("setter").length);
    setradio.disabled = !hasset;
    if (!hasset && setradio.hasAttribute("selected"))
      setradio.removeAttribute("selected");
    
    // make sure at least one is checked
    if (!setradio.hasAttribute("selected") && !getradio.hasAttribute("selected")) {
      if (!getradio.disabled) 
        getradio.setAttribute("selected", "true");
      else if (!setradio.disabled)
        setradio.setAttribute("selected", "true");
    }
    
    // display text
    var et = getradio.hasAttribute("selected") ? "get" : setradio.hasAttribute("selected") ? "set" : null;
    var text = "";
    if (!et || !aProp) {
      // do nothing
    } else if (aProp.hasAttribute("on"+et))
      text = aProp.getAttribute("on"+et);
    else {
      var kids = aProp.getElementsByTagName(et+"ter")
      if (kids && kids.length) {
        text = this.readDOMText(kids[0]);
      }
    }
    this.mFunctionTextbox.value = text;
  },
  
  displayHandler: function(aHandler)
  {
    var text = "";
    if (aHandler) {
      text = aHandler.hasAttribute("action") ? aHandler.getAttribute("action") : null;
      if (!text)
        text = this.readDOMText(aHandler);
    }
    this.mFunctionTextbox.value = text;
  },
  
  ////////////////////////////////////////////////////////////////////////////
  //// selection
  
  onMethodSelected: function()
  {
    var idx = this.mMethodTree.currentIndex;
    var methods = this.mBinding.getElementsByTagNameNS(kXBLNSURI, "method");
    var method = methods[idx];
    this.displayMethod(method);
  },
    
  onPropSelected: function()
  {
    var idx = this.mPropTree.currentIndex;
    var props = this.mBinding.getElementsByTagNameNS(kXBLNSURI, "property");
    var prop = props[idx];
    this.displayProperty(prop);
  },
    
  onHandlerSelected: function()
  {
    var idx = this.mHandlerTree.currentIndex;
    var handlers = this.mBinding.getElementsByTagNameNS(kXBLNSURI, "handler");
    var handler = handlers[idx];
    this.displayHandler(handler);
  },
    
  ////////////////////////////////////////////////////////////////////////////
  //// misc
  
  clearChildren: function(aEl)
  {
    while (aEl.hasChildNodes())
      aEl.removeChild(aEl.lastChild);
  },
  
  readDOMText: function(aEl)
  {
    var text = aEl.nodeValue;
    for (var i = 0; i < aEl.childNodes.length; ++i) {
      text += this.readDOMText(aEl.childNodes[i]);
    }
    return text;
  },

  getBoxObject: function(aTree)
  {
    return aTree.boxObject.QueryInterface(Components.interfaces.nsITreeBoxObject);
  }
  
};

////////////////////////////////////////////////////////////////////////////
//// MethodTreeView

function MethodTreeView(aBinding)
{
  this.mMethods = aBinding.getElementsByTagNameNS(kXBLNSURI, "method");
  this.mRowCount = this.mMethods ? this.mMethods.length : 0;
}

MethodTreeView.prototype = new inBaseTreeView();

MethodTreeView.prototype.getCellText = 
function(aRow, aCol) 
{
  var method = this.mMethods[aRow];
  if (aCol.id == "olcMethodName") {
    var name = method.getAttribute("name");
    var params = method.getElementsByTagName("parameter");
    var pstr = "";
    for (var i = 0; i < params.length; ++i)
      pstr += (i>0?", ": "") + params[i].getAttribute("name");
    return name + "(" + pstr + ")";
  }
  
  return "";
}

////////////////////////////////////////////////////////////////////////////
//// PropTreeView

function PropTreeView(aBinding)
{
  this.mProps = aBinding.getElementsByTagNameNS(kXBLNSURI, "property");
  this.mRowCount = this.mProps ? this.mProps.length : 0;
}

PropTreeView.prototype = new inBaseTreeView();

PropTreeView.prototype.getCellText = 
function(aRow, aCol) 
{
  var prop = this.mProps[aRow];
  if (aCol.id == "olcPropName") {
    return prop.getAttribute("name");
  }
  
  return "";
}

////////////////////////////////////////////////////////////////////////////
//// HandlerTreeView

function HandlerTreeView(aBinding)
{
  this.mHandlers = aBinding.getElementsByTagNameNS(kXBLNSURI, "handler");
  this.mRowCount = this.mHandlers ? this.mHandlers.length : 0;
}

HandlerTreeView.prototype = new inBaseTreeView();

HandlerTreeView.prototype.getCellText = 
function(aRow, aCol) 
{
  var handler = this.mHandlers[aRow];
  if (aCol.id == "olcHandlerEvent") {
    return handler.getAttribute("event");
  } else if (aCol.id == "olcHandlerPhase") {
    return handler.getAttribute("phase");
  }
  
  return "";
}

////////////////////////////////////////////////////////////////////////////
//// ResourceTreeView

function ResourceTreeView(aBinding)
{
  var res = aBinding.getElementsByTagNameNS(kXBLNSURI, "resources");
  var list = [];
  if (res && res.length) {
    var kids = res[0].childNodes;
    for (var i = 0; i < kids.length; ++i)
      if (kids[i].nodeType == Node.ELEMENT_NODE)
        list.push(kids[i]);
  }
        
  this.mResources = list;
  this.mRowCount = this.mResources ? this.mResources.length : 0;
}

ResourceTreeView.prototype = new inBaseTreeView();

ResourceTreeView.prototype.getCellText = 
function(aRow, aCol) 
{
  var resource = this.mResources[aRow];
  if (aCol.id == "olcResourceType") {
    return resource.localName;
  } else if (aCol.id == "olcResourceSrc") {
    return resource.getAttribute("src");
  }
  
  return "";
}

////////////////////////////////////////////////////////////////////////////
//// event listeners

function gDocLoadListener()
{
  viewer.doDisplayBinding();
}
