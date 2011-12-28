var markerNode;
var dialogMarkerList;

function startup()
{
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
  if (markerNode)
  {
    if (markerNode.hasAttribute("name")) 
      document.getElementById("keylist").value = markerNode.getAttribute("name");
    else if (markerNode.hasAttribute("key"))
      document.getElementById("keylist").value = markerNode.getAttribute("key");
    else if (markerNode.hasAttribute("id"))
      document.getElementById("keylist").value = markerNode.getAttribute("id");
  }
  initKeyList();
}


function tagConflicts()
{
  var keyList = document.getElementById("keylist");
  if (keyList.controller && (keyList.controller.searchStatus == nsIAutoCompleteController.STATUS_COMPLETE_MATCH))  //"new" entry matches an existing key
  {
    document.getElementById("uniquekeywarning").hidden = false;
//      msiPostDialogMessage("dlgErrors.markerInUse", {markerString : gDialog.keyInput.value});
    keyList.focus();
    return true;
  }

//  var val = document.getElementById("keylist").value;
//  var i;
//  for (i = 0; i < keys.length; i++)
//  {
//    if (val==keys[i])
//    { 
//      document.getElementById("uniquekeywarning").hidden = false;
//      return true;
//    }
//  }
  document.getElementById("uniquekeywarning").hidden = true;
  return false;
}

function onAccept()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var domdoc = editor.document;
  var val = document.getElementById("keylist").value;
  if (tagConflicts()) return false;
  var newnode = !markerNode; 
  if (newnode) 
  {
    markerNode = domdoc.createElement("a");
  }
  markerNode.setAttribute("name", val);
  markerNode.setAttribute("key", val);
  markerNode.setAttribute("id", val);
  if (newnode) editor.insertElementAtSelection(markerNode, true);
  return true;
}


function onCancel()
{
  return true;
}


function initKeyList()
{
  dialogMarkerList = new msiKeyMarkerList(window);
  dialogMarkerList.setUpTextBoxControl(document.getElementById("keylist"));
}
