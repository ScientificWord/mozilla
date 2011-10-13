/*
 * Copyright (c) 2008 Samuel Gross.
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
Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function findPluginWithId(window, plugin_id) {
  var embeds = window.document.embeds;
  
  if (embeds.length == 1) {
    if (embeds[0].wrappedJSObject.plugin_id == plugin_id) {
      return window;
    }
  }
  
  var frames = window.frames;
  for (var i = 0; i < frames.length; ++i) {
    var found = findPluginWithId(frames[i], plugin_id);
    if (found) {
      return found;
    }
  }

  return null;
}

function getWindowForPluginId(plugin_id) {
  var console = Components.classes["@mozilla.org/consoleservice;1"].
     getService(Components.interfaces.nsIConsoleService);

  var wm = Components.classes["@mozilla.org/appshell/window-mediator;1"].
      getService(Components.interfaces.nsIWindowMediator);
  var enumerator = wm.getEnumerator("navigator:browser");

  while(enumerator.hasMoreElements()) {
    var chromeWindow = enumerator.getNext();
    var browsers = chromeWindow.gBrowser.browsers;

    for (var i = 0; i < browsers.length; ++i) {
      var window = browsers[i].contentWindow;
      var found = findPluginWithId(window, plugin_id);
      if (found) {
        return {window:found, chromeWindow:chromeWindow};
      }
    }
  }

  return null;
}

function getChromeWindowForPluginId(plugin_id) {
  for (var i = 0; i < plugins.length; i++) {
    if (plugins[i].plugin_id == plugin_id) {
      return plugins[i].chromeWindow;
    }
  }
  return null;
}

function getTabBrowserForWindow(plugin_id) {
  var chromeWindow = getChromeWindowForPluginId(plugin_id);
  return chromeWindow && chromeWindow.gBrowser;
}

function getPluginShim(chromeWindow) {
  var window = chromeWindow.gBrowser.contentWindow;

  for (var i = 0; i < plugins.length; i++) {
    if (plugins[i].window == chromeWindow.gBrowser.contentWindow) {
      return plugins[i].plugin;
    }
  }

  return null;
}

var plugins = [];

function PDFService() {
};

PDFService.prototype = {
  classDescription: "Firefox PDF Plugin Service",
  classID: Components.ID("{862827b0-98ac-4b68-a0c9-4ccd8ff35a02}"),
  contractID: "@sgross.mit.edu/pdfservice;1",
  QueryInterface: XPCOMUtils.generateQI([Components.interfaces.PDFService]),
  
  AdvanceTab: function(plugin_id, offset) {
    var browser = getTabBrowserForWindow(plugin_id);
    browser.mTabContainer.advanceSelectedTab(offset, true);
  },
  
  GoHistory: function(plugin_id, dir) {
    var browser = getTabBrowserForWindow(plugin_id);
    if (dir == -1) {
      browser.goBack();
    } else {
      browser.goForward();
    }
  },
  
  Init: function(plugin_id, plugin) {
    var obj = getWindowForPluginId(plugin_id);
    
    if (obj == null) {
      return;
    }
    
    var window = obj.window;
    var chromeWindow = obj.chromeWindow;
    
    plugins.push({plugin_id:plugin_id, plugin:plugin, window:window, chromeWindow:chromeWindow});

    if (!chromeWindow.edu_mit_sgross_pdfplugin_swizzled) {
      var browser = chromeWindow.gBrowser;
      browser._fastFind = new FastFindShim(chromeWindow, browser._fastFind);

      // also 'swizzle' the close function on the find bar to cause the elem to regain focus
      var findBar = chromeWindow.gFindBar;
      findBar.toggleHighlight = createFindbarToggleHighlight(chromeWindow, findBar.toggleHighlight);
      findBar._setHighlightTimeout = createSetHighlightTimeout(chromeWindow, findBar._setHighlightTimeout);
      
      // route zoom commands to the plugin
      var FullZoom = chromeWindow.FullZoom;
      FullZoom.reduce = createZoom(chromeWindow, FullZoom.reduce, -1);
      FullZoom.reset = createZoom(chromeWindow, FullZoom.reset, 0);
      FullZoom.enlarge = createZoom(chromeWindow, FullZoom.enlarge, 1);
      
      chromeWindow.goDoCommand = createGoDoCommand(chromeWindow, chromeWindow.goDoCommand);

      chromeWindow.edu_mit_sgross_pdfplugin_swizzled = true;
    }
  },
  
  CleanUp: function(plugin_id) {
    for (var i = 0; i < plugins.length; i++) {
      var obj = plugins[i];
      if (obj.plugin_id == plugin_id) {
        plugins = plugins.slice(0, i).concat(plugins.slice(i+1, plugins.length));
        return;
      }
    }
  },
  
  FindPrevious: function(plugin_id) {
      var chromeWindow = getChromeWindowForPluginId(plugin_id);
      chromeWindow.gFindBar.onFindAgainCommand(true);
  },
  
  Save: function(plugin_id, url) {
    var chromeWindow = getChromeWindowForPluginId(plugin_id);
    chromeWindow.internalSave(url, null, null, null, "application/pdf", false,
               null, null, null, null);

  }
}

/**
 * Wraps a nsITypeAheadFind object to route find queries to the plugin when appropriate
 */
function FastFindShim(chromeWindow, fastfind) {
  this.fastfind = fastfind;
  this.chromeWindow = chromeWindow;
  for (let prop in fastfind) {
   if (fastfind.__lookupGetter__(prop)) {
     this.__defineGetter__(prop, createDelegateGetter(chromeWindow, this.fastfind, prop));
   }
   if (fastfind.__lookupSetter__(prop)) {
     this.__defineSetter__(prop, createDelegateSetter(chromeWindow, this.fastfind, prop));
   }
  }
}

/**
 * Delegates gets to underlying fastfind.
 */
function createDelegateGetter(chromeWindow, fastfind, prop) {
  if (prop == 'searchString') {
    return function() {
      // TODO: improve condition based on last search type (PDF versus HTML)
      return getPluginShim(chromeWindow) ? this.lastSearchString : fastfind[prop];
    }
  }
  return function() {
    return fastfind[prop];
  }
}

function createDelegateSetter(chromeWindow, fastfind, prop) {
  return function(val) {
    return fastfind[prop] = val;
  }
}

/**
 * Delegate all other methods to wrapped fastfind object.
 */
FastFindShim.prototype.__noSuchMethod__ = function(id, args) {
  if (id == 'find' || id == 'findAgain') {
    var plugin = getPluginShim(this.chromeWindow);
    if (plugin) {
      var str, reverse;
      if (id == 'find') {
        str = args[0];
        reverse = false;
      } else { // findAgain
        str = this.lastSearchString;
        reverse = args[0];
      }
      this.lastSearchString = str;
      // find must take place before findAll because find cancels any current searches
      var ret = plugin.Find(str, this.fastfind.caseSensitive, !reverse);
      var findBar = this.chromeWindow.gFindBar;
      if (findBar.getElement("highlight").checked) {
        plugin.FindAll(str, this.fastfind.caseSensitive);
      }
      return ret;
    }
  }
  return this.fastfind[id].apply(this.fastfind, args);
}

/**
 * Creates a function that calls toggleHighlight on the plugin.
 */
function createFindbarToggleHighlight(chromeWindow, orig) {
  return function(shouldHighlight) {
    var plugin = getPluginShim(chromeWindow);
    if (plugin) {
      if (shouldHighlight) {
        var word = this._findField.value;
        // TODO: better choice for determining case sensitivity?
        var caseSensitive = this._shouldBeCaseSensitive(word);
        plugin.FindAll(word, caseSensitive);
      } else {
        plugin.RemoveHighlights();
      }
    } else {
      orig.call(this, shouldHighlight);
    }
  };
}

/**
 * Creates a function that only sets the highlight timeout when 
 * the plugin is not present.
 */
function createSetHighlightTimeout(chromeWindow, orig) {
  return function() {
    if (!getPluginShim(chromeWindow)) {
      orig.call(this);
    }
  };
}

function createZoom(chromeWindow, orig, arg) {
  return function() {
    var plugin = getPluginShim(chromeWindow);
    if (plugin) {
      plugin.Zoom(arg);
    } else {
      orig.call(this);
    }
  }
}

function createGoDoCommand(chromeWindow, orig) {
  return function(cmd) {
    var plugin;
    if (cmd == 'cmd_copy' && (plugin = getPluginShim(chromeWindow))) {
      plugin.Copy();
    } else {
      return orig.call(this, cmd);
    }
  }
}

var components = [PDFService];
function NSGetModule(compMgr, fileSpec) {
  return XPCOMUtils.generateModule(components);
}