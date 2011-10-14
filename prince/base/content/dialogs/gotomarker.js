var gEditorElement;
var keyMarkerList;
var keyMarkerListbox;

function initialize()
{
  gEditorElement = msiGetParentEditorElementForDialog(window);
  if (!gEditorElement)
  {
    dump("Failed to get parent editor in gotomarker.js!\n");
    window.close();
    return;
  }
  keyMarkerList = new msiKeyMarkerList(window);
  keyMarkerListbox = document.getElementById("markerList");
  keyMarkerList.setUpTextBoxControl(keyMarkerListbox);
}

function doEnabling()
{
  var bDisable = true;
  if (keyMarkerListbox.value && (keyMarkerListbox.value.length > 0) && keyMarkerListbox.controller && 
         (keyMarkerListbox.controller.searchStatus == nsIAutoCompleteController.STATUS_COMPLETE_MATCH))  //matches an existing key
    bDisable = false;
  var dialogNode = document.getElementById("gotomarkerdialog");
  dialogNode.getButton("accept").disabled = bDisable;
}

function onAccept()
{
  var theMarker = keyMarkerListbox.value;
  theMarker = TrimString(theMarker);
  try {
    msiGoToMarker(gEditorElement, theMarker, true);
  }
  catch(e) {
    dump("In gotomarker.js onAccept, error [" + e.message + "]\n");
  }
}

function onCancel()
{
}
