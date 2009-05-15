// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.


//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

// dialog initialization code
function Startup()
{
//  var editor = GetCurrentEditor();
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    AlertWithTitle("Error", "No editor found in typesetManualCitation Startup! Closing dialog window...");
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.keyList = data.keyList;
  gDialog.key = data.key;  //a string
  gDialog.remark = data.remark;  //this should become arbitrary markup - a Document Fragment perhaps?

  InitDialog();

//  document.getElementById("keysListbox").focus();
  msiSetInitialDialogFocus(document.getElementById("keysListbox"));

  SetWindowLocation();
}

function InitDialog()
{
  fillKeysListbox();
  document.getElementById("keysListbox").value = gDialog.key;
  
  if (!gDialog.remark || gDialog.remark.length == 0)
    gDialog.remark = "Is this bold?";
  var theStringSource = "<bold>" + gDialog.remark + "</bold>";
  var editorControl = document.getElementById("remarkEditControl");
  msiInitializeEditorForElement(editorControl, theStringSource);
  
//  attachEditorToTextbox(document.getElementById("remarkTextbox"), theStringSource)
  //This next will have to be replaced by code to set up the MathML editor for the Remark field
//  document.getElementById("remarkTextbox").value = gDialog.remark;
  checkDisableControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  gDialog.key = document.getElementById("keysListbox").value;
  if (findInArray(gDialog.keyList, gDialog.key) < 0)
    gDialog.keyList.push(gDialog.key);

  var editorControl = document.getElementById("remarkEditControl");
  var serializer = new XMLSerializer();
  gDialog.remark = serializer.serializeToString(editorControl.contentDocument.documentElement);
//  gDialog.remark = document.getElementById("remarkTextbox").value;

  data.key = gDialog.key;
  data.keyList = gDialog.keyList;
  data.remark = gDialog.remark;

  var listBox = document.getElementById('keysListbox');
  listBox.addEventListener('ValueChange', checkDisableControls, false);

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var theWindow = window.opener;
  if (!theWindow || !("updateEditorBibItemList" in theWindow))
    theWindow = msiGetTopLevelWindow();
  if (editor && theWindow)
    theWindow.updateEditorBibItemList(editor, data.keyList);

  var dd =editorControl.contentDocument;
  var elts = dd.getElementsByTagName("sw:dialogbase");
  var elt = elts.item(0);
  var str1 = serializer.serializeToString(elt);
  var str = "<citation>" + str1 + "</citation>";
  
  // .getElementsByTagName("sw:dialogbase")
  insertXMLAtCursor(editor, str, true, false);

  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}

function checkDisableControls(event)
{
//  var selDBase = document.getElementById("databaseFileListbox").selectedItem;
  if (document.getElementById("keysListbox").value.length > 0)
    document.documentElement.getButton('accept').removeAttribute("disabled");
  else
    document.documentElement.getButton('accept').setAttribute("disabled", "true");
}


function fillKeysListbox()
{
  var theListbox = document.getElementById("keysListbox");
  //Clear the listbox
  var theItems = theListbox.getElementsByTagName("menuitem");
  var itemCount = theItems.length;
  for (var i = itemCount - 1; i >= 0; --i)
    theListbox.removeChild(theItems[i]);

  for (var i = 0; i < gDialog.keyList.length; ++i)
  {
    theListbox.appendItem(gDialog.keyList[i], gDialog.keyList[i]);
    //include the label also as the "value", to allow use of the "value" property of the menulist parent.
  }
//  if (selItem != null)
//    theListbox.selectItem(selItem);
}


//function attachEditorToTextbox(theTextbox, theStringSource)
//{
//  var theParent = theTextbox.parentNode;
//  var theEditor = document.createElement("editor");
//  var newID = theTextbox.getAttribute("id") + "edit";
//  theEditor.setAttribute("flex", "1");
//  theEditor.setAttribute("type", "content-primary");
//  theEditor.setAttribute("editortype", "html");
//  theEditor.setAttribute("id", newID);
//  var theWidth = theTextbox.width;
//  var theHeight = theTextbox.height;
//  theEditor.width = theWidth;
//  theEditor.height = theHeight;
//  theParent.replaceChild(theEditor, theTextbox);
//  theEditor = document.getElementById(newID);
//  if (!theEditor)
//    alert("Document reports not finding editor by ID " + newID);
//  else if (theStringSource && theStringSource.length > 0)
//  {
//    theEditor.makeEditable("html");
//    var editor = theEditor.getEditor(theEditor.contentWindow);
//    if (editor)
//      editor.insertHTML(theStringSource);
//    else if (!theEditor.contentWindow)
//      alert("theEditor.contentWindow was null!");
//    else
//      alert("getEditor returned null but contentWindow was okay!");
//  }
//}