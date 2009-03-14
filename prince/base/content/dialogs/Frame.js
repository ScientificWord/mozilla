var frmElement = {};
var newElement = true;
var gd;
var editor;

Components.utils.import("resource://app/modules/unitHandler.jsm");

var data;
var scale= .25; /*scale of reduced diagram*/

function startUp()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  editor = msiGetEditor(editorElement);
  if (!editor)
  {
    window.close();
    return;
  }
  if ("arguments" in window && window.arguments[0])
  {
    data = window.arguments[0];
    if (data.element && ("localName" in data.element))
    { 
      if (data.element.localName != "msiframe")
      {
        // we allow for the element passed in to be an immediate child of a msiframe element
        if (data.element.parentNode.localName == "msiframe")
          data.element = data.element.parentNode;
        else data.element = null;
      }
    } else data.element = null;
    try {
      frmElement = data.element;}
    catch(e) {}
  }
  if (data.element == null) frmElement=editor.createElementWithDefaults("msiframe");
     
  gd = new Object();
  gd = initFrameTab(gd, frmElement, data.newElement);
}

function onOK() {
  setFrameAttributes(frmElement);
  data.element = frmElement.cloneNode(true);
  return(true);
}

function onCancel() {
  close();
  data.newElement = false;
  return(true);
}


