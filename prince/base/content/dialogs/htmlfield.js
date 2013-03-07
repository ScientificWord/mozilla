var node;

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
  node = getSelectionParentByTag(activeEditor,"htmlfield");
  if (node) {
     document.getElementById("htmlbuttonTextbox").value = node.innerHTML;
     document.getElementById("name").value = node.getAttribute("name");
  }
}



function onAccept()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var domdoc = editor.document;
  var gHTML;
  var gName;

  gHTML  = document.getElementById("htmlbuttonTextbox");
  gName  = document.getElementById("name");

  var htmlNode;
  dump("\nhtmlfield onAccept()"); 
  if (node == null) 
  {
    dump("\nhtmlfield onAccept() newnode");
    htmlNode = domdoc.createElement("htmlfield");
    editor.insertElementAtSelection(htmlNode, true);
  }
  else htmlNode = node;
  
  if (gName.value.length > 0) 
    htmlNode.setAttribute("name", gName.value)
  else 
    htmlNode.removeAttribute("name");
  htmlNode.innerHTML = gHTML.value;
  
  return true;
}


function onCancel()
{
  return true;
}


