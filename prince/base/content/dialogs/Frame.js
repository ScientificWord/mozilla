var frmElement = {};
var newElement = true;
var gd;
var editor;
var msiframe;
var isNewNode;

Components.utils.import("resource://app/modules/unitHandler.jsm");

var msiframe;
var scale= .25; /*scale of reduced diagram*/
var editor;

function startUp()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  editor = msiGetEditor(editorElement);
  if (!editor)
  {
    window.close();
    return;
  }
  msiframe = editor.getSelectedElement("msiframe");
  isNewNode = !(msiframe);
  if (isNewNode) msiframe = editor.createElementWithDefaults("msiframe");
     
  gd = new Object();
  gd = initFrameTab(gd, msiframe, isNewNode);
  initFrameSizePanel(); // needed when the user can set the size
}


function onOK() {
  setFrameAttributes(msiframe);
  // BBM This is a stub
  
  if (isNewNode) editor.insertElementAtSelection(msiframe, true);
  var para = editor.createNode("para", msiframe, 0);
  var br = editor.createNode("br", para, 0);
  // BBM This new node should go AROUND the selection.
  return(true);
}

function onCancel() {
  close();
  return(true);
}


