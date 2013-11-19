// Copyright (c) 2010-13 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
 //   AlertWithTitle("Error", "No editor found in typesetBibItem Startup! Closing dialog window...");
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.key = "";  
  gDialog.bibLabel = ""; 

  data.node = getSelectionParentByTag(editor, "bibitem");
  if (data.node) {
    var node = getChildByTagName(data.node, "bibkey");
    if (node) gDialog.key = node.textContent;
    node = getChildByTagName(data.node,"biblabel");
    if (node) gDialog.bibLabel = node.innerHTML;
  }

  InitDialog();

//  document.getElementById("keysListbox").focus();
  msiSetInitialDialogFocus(document.getElementById("keysAutoCompleteBox"));

  SetWindowLocation();
}

function InitDialog()
{
//  fillKeysListbox();
  
  gDialog.markerList = new msiBibItemKeyMarkerList(window);
  gDialog.markerList.setUpTextBoxControl(document.getElementById("keysAutoCompleteBox"));
//  var keyString = gDialog.markerList.getIndexString();
//  document.getElementById("keysAutoCompleteBox").setAttribute("autocompletesearchparam", keyString);
  document.getElementById("keysAutoCompleteBox").value = gDialog.key;
  
//  var theStringSource = "<bold>" + gDialog.remark + "</bold>";
  var theStringSource = gDialog.bibLabel;
  var editorControl = document.getElementById("labelEditControl");
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
  var editorElement = msiGetParentEditorElementForDialog(window);
  var keyList = document.getElementById("keysAutoCompleteBox");
  gDialog.key = document.getElementById("keysAutoCompleteBox").value;
  data.key = gDialog.key;
//  if (findInArray(keyList, gDialog.key) < 0)
//    keyList.push(gDialog.key);

  var labelControl = document.getElementById("labelEditControl");
  var labelContentFilter = new msiDialogEditorContentFilter(labelControl);
  gDialog.bibLabel = labelContentFilter.getDocumentFragmentString();

//  var serializer = new XMLSerializer();
//  gDialog.remark = serializer.serializeToString(editorControl.contentDocument.documentElement);
////  gDialog.remark = document.getElementById("remarkTextbox").value;

  if (data.bibLabel != gDialog.bibLabel)
  {
    data.bBibLabelChanged = true;
    data.bibLabel = gDialog.bibLabel;
  }

//  var listBox = document.getElementById('keysListbox');
//  listBox.addEventListener('ValueChange', checkDisableControls, false);

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (data.node)
  {
    var node = getChildByTagName(data.node, "bibkey");
    if (node) node.textContent = gDialog.key;
    else {
      node = editor.createElement("bibkey", data.node, 0);
      node.textContent = gDialog.key;
    }
    node = getChildByTagName(data.node, "biblabel");
    if (node) node.innerHTML = gDialog.bibLabel;
    else {
      node = editor.createElement("biblabel", data.node, 0);
      node.innerHTML = gDialog.bibLabel;
    }
  }
  else if ("paragraphNode" in data)
  {
    if (!theWindow || !("msiDoTagBibItem" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (theWindow && ("msiDoTagBibItem" in theWindow))
      theWindow.msiDoTagBibItem(data, data.paragraphNode, editorElement);
  }

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
  if (document.getElementById("keysAutoCompleteBox").value.length > 0)
    document.documentElement.getButton('accept').removeAttribute("disabled");
  else
    document.documentElement.getButton('accept').setAttribute("disabled", "true");
}

function keyTextboxChanged()
{
  var keyList = document.getElementById("keysAutoCompleteBox");
  var newKey = keyList.value;
  if (gDialog.key != newKey)
  {
    if (newKey.length && keyList.controller && (keyList.controller.searchStatus == 4))  //found a match!
    {
      msiPostDialogMessage("dlgErrors.bibmarkerInUse", {bibmarkerString : keyList.value});
      keyList.select();
      keyList.focus();
      return;
    }
    gDialog.markerList.changeString(gDialog.key, newKey);
    gDialog.key = newKey;
  }

  checkDisableControls();
}

//function fillKeysListbox()
//{
//  var theListbox = document.getElementById("keysListbox");
//  //Clear the listbox
//  var theItems = theListbox.getElementsByTagName("menuitem");
//  var itemCount = theItems.length;
//  for (var i = itemCount - 1; i >= 0; --i)
//    theListbox.removeChild(theItems[i]);
//
//  for (var i = 0; i < gDialog.keyList.length; ++i)
//  {
//    theListbox.appendItem(gDialog.keyList[i], gDialog.keyList[i]);
//    //include the label also as the "value", to allow use of the "value" property of the menulist parent.
//  }
////  if (selItem != null)
////    theListbox.selectItem(selItem);
//}


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