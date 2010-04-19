
var msiTabBrowser = function(editor) {
  if (editor == undefined) throw new Error("Tabbed browser constructor needs an editor.");
  this.browsers = [];
  this.browsers.push(editor);
  this.mEditor = editor;
  this.selectedBrowser = editor;
  this.currentURI = (function() { try{ return editor.currentDocument.baseURIObject;}
                                  catch(e){}
                                  return null; })();
};

msiTabBrowser.prototype.getBrowserIndexForDocument = function( doc ) {
  try {if (doc == this.mEditor.currentDocument) return 0;}
  catch(e){}
  return -1;
};

msiTabBrowser.prototype.getBrowserForDocument = function( doc ) {
  try {if (doc == this.mEditor.currentDocument) return this.mEditor;}
  catch(e){}
  return null;
};

msiTabBrowser.prototype.getBrowser = function() {
  return this.mEditor;
};

msiTabBrowser.prototype.addProgressListener = function(listener, notifytype) {
  try {this.mEditor.editingSession.addProgressListener(listener, notifytype);}
  catch (e) {}
};

msiTabBrowser.prototype.removeProgressListener = function(listener) {
  try {this.mEditor.editingSession.removeProgressListener(listener);}
  catch (e) {}
};

