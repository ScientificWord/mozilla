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
 * The Original Code is XTF unit test components.
 *
 * The Initial Developer of the Original Code is
 * Alexander J. Vincent <ajvincent@gmail.com>.
 * Portions created by the Initial Developer are Copyright (C) 2007
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

// Utility function.
function NOT_IMPLEMENTED() {
  throw Components.results.NS_ERROR_NOT_IMPLEMENTED;
}

const C_i = Components.interfaces;

const nsIXTFElementFactory        = C_i.nsIXTFElementFactory;
const nsIXTFElement               = C_i.nsIXTFElement;
const nsIXTFPrivate               = C_i.nsIXTFPrivate;
const nsIXTFAttributeHandler      = C_i.nsIXTFAttributeHandler;
const mozIJSSubScriptLoader       = C_i.mozIJSSubScriptLoader;
const nsIProgrammingLanguage      = C_i.nsIProgrammingLanguage;
const nsIClassInfo                = C_i.nsIClassInfo;
const nsIComponentRegistrar       = C_i.nsIComponentRegistrar;
const nsIFactory                  = C_i.nsIFactory;
const nsIModule                   = C_i.nsIModule;
const nsISupports                 = C_i.nsISupports;

const xtfClass = "@mozilla.org/xtf/element-factory;1?namespace=";

/**
 * Wrap a JavaScript object for passing to components code.
 */
function ObjectWrapper(object) {
  this.wrappedJSObject = object;
}

/* <foo:element xmlns:foo="xtf-tests;foo"> */
const FooInner = {
  bar: {
    testpassed: true
  },

  handle_default: {
    testpassed: false
  }
}

function FooElement(aLocalName) {
  this._wrapper = null;

  // nsIXTFPrivate
  this.inner = new ObjectWrapper(FooInner[aLocalName]);
}
FooElement.prototype =
{
  // nsIXTFElement
  onCreated: function onCreated(aWrapper) {
    this._wrapper = aWrapper;
    aWrapper.notificationMask = 0;
  },

  // nsIXTFElement
  onDestroyed: function onDestroyed() {
  },

  // nsIXTFElement
  isAttributeHandler: false,

  // nsIXTFElement
  getScriptingInterfaces: function getScriptingInterfaces(aCount) {
    var interfaces = [];
    aCount.value = interfaces.length;
    return interfaces;
  },

  // nsIXTFElement
  willChangeDocument: NOT_IMPLEMENTED,
  documentChanged: NOT_IMPLEMENTED,
  willChangeParent: NOT_IMPLEMENTED,
  parentChanged: NOT_IMPLEMENTED,
  willInsertChild: NOT_IMPLEMENTED,
  childInserted: NOT_IMPLEMENTED,
  willAppendChild: NOT_IMPLEMENTED,
  childAppended: NOT_IMPLEMENTED,
  willRemoveChild: NOT_IMPLEMENTED,
  childRemoved: NOT_IMPLEMENTED,
  willSetAttribute: NOT_IMPLEMENTED,
  attributeSet: NOT_IMPLEMENTED,
  willRemoveAttribute: NOT_IMPLEMENTED,
  attributeRemoved: NOT_IMPLEMENTED,

  beginAddingChildren: NOT_IMPLEMENTED,
  doneAddingChildren: NOT_IMPLEMENTED,

  handleDefault: NOT_IMPLEMENTED,
  cloneState: NOT_IMPLEMENTED,

  get accesskeyNode() {
    return null;
  },
  performAccesskey: NOT_IMPLEMENTED,

  // nsISupports
  QueryInterface: function QueryInterface(aIID) {
    if (aIID.equals(nsIXTFPrivate) ||
        aIID.equals(nsIXTFElement) ||
        aIID.equals(nsISupports))
      return this;

    throw Components.results.NS_ERROR_NO_INTERFACE;
    return null;
  }
};

function FooElementFactory() {}
FooElementFactory.prototype =
{
  // nsIXTFElementFactory
  createElement: function createElement(aLocalName) {
    var rv = null;
    switch (aLocalName) {
      case "bar":
        rv = new FooElement(aLocalName);
        rv.handleDefault = function handleDefault(aEvent) {
          this.inner.wrappedJSObject.testpassed = false;
        }
        break;

      case "handle_default":
        var rv = new FooElement(aLocalName);
        rv.onCreated = function onCreated(aWrapper) {
          this._wrapper = aWrapper;
          aWrapper.notificationMask = nsIXTFElement.NOTIFY_HANDLE_DEFAULT;
        }
        rv.handleDefault = function handleDefault(aEvent) {
          this.inner.wrappedJSObject.testpassed = true;
        }
        break;
    }

    return rv ? rv.QueryInterface(nsIXTFElement) : null;
  },

  // nsISupports
  QueryInterface: function QueryInterface(aIID) {
    if (aIID.equals(nsIXTFElementFactory) ||
        aIID.equals(nsISupports))
      return this;

    throw Components.results.NS_ERROR_NO_INTERFACE;
    return null;
  }
};

const FooComponentFactory = {
  // nsIFactory
  createInstance: function createInstance(outer, iid) {
    if (outer != null)
      throw Components.results.NS_ERROR_NO_AGGREGATION;

    return (new FooElementFactory()).QueryInterface(iid);
  },

  // nsISupports
  QueryInterface: function QueryInterface(aIID) {
    if (aIID.equals(nsIFactory) ||
       (aIID.equals(nsISupports)))
      return this;

    throw Components.results.NS_ERROR_NO_INTERFACE;
    return null;
  }
};

const fooClassID    = Components.ID("{f367b65d-6b7f-4a7f-9a4b-8bde0ff4ef10}");
const fooClassDesc  = "XTF unit test: Foo element factory";
const fooContractID = xtfClass + "xtf-tests;foo";

/* </foo:element> */

const Module = {
  // nsIModule
  registerSelf: function registerSelf(compMgr, fileSpec, location, type) {
    compReg = compMgr.QueryInterface(nsIComponentRegistrar);

    compReg.registerFactoryLocation(fooClassID,
                                    fooClassDesc,
                                    fooContractID,
                                    fileSpec,
                                    location,
                                    type);
  },

  // nsIModule
  getClassObject: function getClassObject(compMgr, aCID, aIID) {
    if (!aIID.equals(nsIFactory)) {
      throw Components.results.NS_ERROR_NOT_IMPLEMENTED;
    }
    if (aCID.equals(fooClassID)) {
      return FooComponentFactory;
    }

    throw Components.results.NS_ERROR_NO_INTERFACE;
  },

  // nsIModule
  canUnload: function canUnload() {
    return true;
  },

  // nsISupports
  QueryInterface: function QueryInterface(aIID) {
    if (aIID.equals(nsIModule) ||
       (aIID.equals(nsISupports)))
      return this;

    throw Components.results.NS_ERROR_NO_INTERFACE;
    return null;
  }
};

function NSGetModule(compMgr, fileSpec) {
  return Module;
}
