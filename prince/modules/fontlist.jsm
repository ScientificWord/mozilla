var EXPORTED_SYMBOLS = ["jQuery", "$"];

loader = Components.classes["@mozilla.org/moz/javasubscript-loader;1")
  .getService(Components.interfaces.mozIJSSubScriptLoader);

loader.loadSubScript("resource://app/modules/jQuery-1.4.3.min.js" ,null);
 
