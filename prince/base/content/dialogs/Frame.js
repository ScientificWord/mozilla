var globalElement;
Components.utils.import("resource://app/modules/unitHandler.jsm");
var fmUnitHandler = new UnitHandler();

var data;
var scale= .25; /*pixels per pixel in reduced view*/

function startUp()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor)
  {
    window.close();
    return;
  }
  gDialog = new Object();
  gDialog.widthInput = document.getElementById("frameWidthInput");
  gDialog.heightInput = document.getElementById("frameHeightInput");
  initFrameTab(gDialog);
  
  var fmSizeFieldList = [gDialog.widthInput, gDialog.heightInput];
  fmUnitHandler.setEditFieldList(fmSizeFieldList);
  fmUnitHandler.initCurrentUnit("px");
  fmUnitHandler.setCurrentUnit("px");
  setContentSize(50,40);
}

function onOK() {
  setFrameAttributes(data);
  // add return data to the data object. 
//  data.units = currentUnit;
//  data.position = document.getElementById('frame.position').value;
//  var s = document.getElementById('width').value;
//  if (s=="auto") data.width = s;
//  else data.width = Number(s);
//  var s = document.getElementById('height').value;
//  if (s=="auto") data.height = s;
//  else data.height = Number(s);
//  data.overhang = Number(document.getElementById('overhang').value);
//  data.topmargin = Number(document.getElementById('marginTop').value);
//  data.sidemargin = Number(document.getElementById('marginSide').value);
//  data.border = Number(document.getElementById('border').value);
//  data.padding = Number(document.getElementById('padding').value);
////  data.color = color;
//  data.textalignment = document.getElementById('textalignment').value;
////   data.borderstyle = document.getElementById('borderstyle').value;
//  data.rotation = document.getElementById('rotation').value;
//  close();
//  return (false);
}

function onCancel() {
  close();
  return(true);
}


