
Components.utils.import("resource://app/modules/msiEditorDefinitions.jsm");

var markerNode;
var dialogMarkerList;

function startup()
{
  validkey = false;
  var editorElement = msiGetParentEditorElementForDialog(window);
  var activeEditor = msiGetEditor(editorElement);
  if (!activeEditor)
  {
    dump("Failed to get active editor!\n");
    window.close();
    return;
  }

  var markerNode;
  if (window.arguments && window.arguments.length)
    markerNode = window.arguments[0];
  if (!markerNode)
    markerNode = getSelectionParentByTag(activeEditor, "a");
  initKeyEntryOverlay(activeEditor, markerNode);
}


function onAccept()
{
  if (!validkey) { 
    validkey = !tagConflicts();
  }
  if (validkey) {
    var editorElement = msiGetParentEditorElementForDialog(window);
    var editor = msiGetEditor(editorElement);
    var domdoc = editor.document;
    var val = document.getElementById("keylist").value;
    var newnode = !markerNode; 
    if (newnode) 
    {
      markerNode = domdoc.createElement("a");
    }
    saveKey(markerNode);
    if (newnode) editor.insertElementAtSelection(markerNode, true);
    return true;
  }
  return false;
}


function onCancel()
{
  return true;
}

