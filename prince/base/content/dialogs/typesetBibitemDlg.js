// Copyright (c) 2010 MacKichan Software, Inc.  All Rights Reserved.


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
    AlertWithTitle("Error", "No editor found in typesetBibItem Startup! Closing dialog window...");
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.key = "";  //a string
  gDialog.bibLabel = "";  //this should become arbitrary markup - a Document Fragment perhaps?
  if ("reviseData" in data)
  {
    setDataFromReviseData(data.reviseData)
    //set window title too!
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
  var keyString = gDialog.markerList.getIndexString();
  document.getElementById("keysAutoCompleteBox").setAttribute("autocompletesearchparam", keyString);
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

function setDataFromReviseData(reviseData)
{
  var bibItemNode = reviseData.getReferenceNode();
  gDialog.key = bibItemNode.getAttribute("bibitemkey");
  gDialog.bibLabel = "";
  var labels = msiNavigationUtils.getSignificantContents(bibItemNode);
  for (ix = 0; ix < labels.length; ++ix)
  {
    if (msiGetBaseNodeName(labels[ix]) == "biblabel")
    {
      var serializer = new XMLSerializer();
      var labelKids = msiNavigationUtils.getSignificantContents(labels[ix]);
      for (var jx = 0; jx < labelKids.length; ++jx)
        gDialog.bibLabel += serializer.serializeToString(labelKids[jx]);
      break;
    }
  }
}

function onAccept()
{
  var keyList = document.getElementById("keysAutoCompleteBox");
  gDialog.key = document.getElementById("keysAutoCompleteBox").value;
//  if (findInArray(keyList, gDialog.key) < 0)
//    keyList.push(gDialog.key);

  var labelControl = document.getElementById("labelEditControl");
  var labelContentFilter = new msiDialogEditorContentFilter(labelControl);
  gDialog.bibLabel = labelContentFilter.getDocumentFragmentString();

//  var serializer = new XMLSerializer();
//  gDialog.remark = serializer.serializeToString(editorControl.contentDocument.documentElement);
////  gDialog.remark = document.getElementById("remarkTextbox").value;

  data.key = gDialog.key;
  if (data.bibLabel != gDialog.bibLabel)
  {
    data.bBibLabelChanged = true;
    data.bibLabel = gDialog.bibLabel;
  }

//  var listBox = document.getElementById('keysListbox');
//  listBox.addEventListener('ValueChange', checkDisableControls, false);

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if ("reviseData" in data)
  {
    var bibitemNode = data.reviseData.getReferenceNode();
    if (!theWindow || !("doReviseManualBibItem" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (theWindow && ("doReviseManualBibItem" in theWindow))
      theWindow.doReviseManualBibItem(editorElement, bibitemNode, data);
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