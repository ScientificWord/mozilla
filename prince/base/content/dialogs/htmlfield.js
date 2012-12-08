var gHTML;
var gName;

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
}



function onAccept()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var domdoc = editor.document;

  gHTML  = document.getElementById("htmlbuttonTextbox");
  gName  = document.getElementById("name");

  var htmlNode;
  var newnode = !gHTML;
  dump("\nhtmlfield onAccept()"); 
  if (newnode) 
  {
    dump("\nhtmlfield onAccept() newnode");
    htmlNode = domdoc.createElement("span");
  
    if (gName.value.length > 0) 
      htmlNode.setAttribute("name", gName.value)
    else 
      htmlNode.removeAttribute("name");
  
    editor.insertElementAtSelection(htmlNode, true);
  }

  return true;
}


function onCancel()
{
  return true;
}


