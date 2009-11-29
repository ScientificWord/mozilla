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
  data = window.arguments[0];
  if (data != null)
  {
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
  initFrameSizePanel(); // needed when the user can set the size
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


