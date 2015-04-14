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
  var newNode = (node == null);
  var error;

  gHTML  = document.getElementById("htmlbuttonTextbox");
  gName  = document.getElementById("name");

  var htmlElement;
  dump("\nhtmlfield onAccept()");
  if (newNode)
  {
    dump("\nhtmlfield onAccept() newnode");
    htmlElement = domdoc.createElement("htmlfield");
  }
  else {
    htmlElement = node;
    editor.deleteNode(node);
  }

  if (gName.value.length > 0)
    htmlElement.setAttribute("name", gName.value)
  else
    htmlElement.removeAttribute("name");
  try {
    htmlElement.innerHTML = gHTML.value;
    editor.insertElementAtSelection(htmlElement, true);
  }
  catch(e) {
    finalThrow(cmdFailString('HTML field'), 'XHTML appears to be invalid');
  }
  return true;
}


function onCancel()
{
  return true;
}


