var globalElement;
var frElement = null;
var newElement = true;
var editor;

Components.utils.import("resource://app/modules/unitHandler.jsm");

var data;
var scale= .25; /*pixels per pixel in reduced view*/

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
        if (data.element.parentNode.localName == "msiframe")
          data.element = data.element.parentNode;
        else data.element = null;
      }
    } else data.element = null;
    // frElement is now an msiframe, either the node passed in, or its parent, or it is null.
  }

  gDialog = new Object();
  gDialog.widthInput = document.getElementById("frameWidthInput");
  gDialog.heightInput = document.getElementById("frameHeightInput");
  gDialog.autoHeight = document.getElementById("autoHeight");
  gDialog.autoWidth = document.getElementById("autoWidth");
  gDialog.frameUnitMenulist = document.getElementById("frameUnitMenulist");
  if (data.element) 
  {
    globalElement = data.element.cloneNode(true);
    if (globalElement.hasAttribute("width"))
    {
      gDialog.widthInput.value = globalElement.getAttribute("width");
      gDialog.autoWidth.checked = false;
    }
    else gDialog.autoWidth.checked = true;
    if (globalElement.hasAttribute("height"))
    {
      gDialog.heightInput.value = globalElement.getAttribute("height");
      gDialog.autoHeight.checked = false;
    }
    else gDialog.autoHeight.checked = true;
    
    data.newElement = false;
  }
  else globalElement = editor.createElementWithDefaults("msiframe");
  initFrameTab(gDialog, globalElement, data.newElement);
  
  var fmSizeFieldList = [gDialog.widthInput, gDialog.heightInput];
  frameUnitHandler.addEditFieldList(fmSizeFieldList);
  var saveUnit = frameUnitHandler.setCurrentUnit("mm");  // frameUnitHander defined in overlay
  setContentSize((!gDialog.autoHeight.checked)?50:frameUnitHandler.getValueAs(gDialog.heightInput.value,"mm"),
    (!gDialog.autoWidth.checked)?75:frameUnitHandler.getValueAs(gDialog.widthInput.value,"mm"));
  frameUnitHandler.setCurrentUnit(saveUnit);
  for (var i = 0; i<gDialog.frameUnitMenulist.itemCount; i++)
  {
    if (gDialog.frameUnitMenulist.getItemAtIndex(i).value == saveUnit) break;
  }
  if (i<gDialog.frameUnitMenulist.itemCount) gDialog.frameUnitMenulist.selectedItem = i; 
}

function onOK() {
  setFrameAttributes(globalElement);
  var width, height;
  width = gDialog.widthInput.value;
  if (gDialog.autoWidth.checked) width = 0;
  height = gDialog.heightInput.value;
  if (gDialog.autoHeight.checked) height = 0;
  dump("height = "+height+", width = "+width+"\n");
  if (width == 0) {globalElement.removeAttribute("width");}
  else 
  {
    globalElement.setAttribute("width", width);
    dump("width = "+width+"; unit = "+frameUnitHandler.currentUnit);
    setStyleAttributeOnNode(globalElement, "width", frameUnitHandler.getValueAs(width,"px")+"px");
  }
  if (height == 0) 
  {
    globalElement.removeAttribute("height");
  }
  else
  {
    globalElement.setAttribute("height", height);
    setStyleAttributeOnNode(globalElement, "height", frameUnitHandler.getValueAs(height,"px")+"px");
  }
  data.element = globalElement.cloneNode(true);
  return(true);
}

function onCancel() {
  close();
  data.newElement = false;
  return(true);
}


