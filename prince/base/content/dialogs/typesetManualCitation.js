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
  if ("reviseData" in data)
  {
    setDataFromReviseData(reviseData)
    //set window title too!
  }
  gDialog.keyList = [""];
  gDialog.key = data.key;
  gDialog.remark = data.remark;

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
  var theStringSource = gDialog.remark;
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

function setDataFromReviseData(reviseData)
{
  var citeNode = reviseData.getReferenceNode();
  var theKeys = citeNode.getAttribute("bibcitekey");
  data.keyList = theKeys.split(",");
  for (var ix = 0; ix < data.keyList.length; ++ix)
    data.keyList = TrimString(data.keyList);
  data.key = data.keyList[0];  //a string
  data.remark = "";
  var remarks = msiNavigationUtils.getSignificantContents(citeNode);
  for (ix = 0; ix < remarks.length; ++ix)
  {
    if (msiGetBaseNodeName(remarks[ix]) == "biblabel")
    {
      var serializer = new XMLSerializer();
      var remarkNodes = msiNavigationUtils.getSignificantContents(remarks[ix]);
      for (var jx = 0; jx < remarkNodes.length; ++jx)
        data.remark += serializer.serializeToString(remarkNodes[jx]);
      break;
    }
  }
}

function onAccept()
{
  gDialog.key = document.getElementById("keysAutoCompleteBox").value;
  if (findInArray(gDialog.keyList, gDialog.key) < 0)
    gDialog.keyList.push(gDialog.key);


  var remarkControl = document.getElementById("remarkEditControl");
  var remarkContentFilter = new msiDialogEditorContentFilter(remarkControl);
  gDialog.remark = remarkContentFilter.getDocumentFragmentString();

//  var serializer = new XMLSerializer();
//  gDialog.remark = serializer.serializeToString(editorControl.contentDocument.documentElement);
////  gDialog.remark = document.getElementById("remarkTextbox").value;

  data.key = gDialog.key;
  data.keyList = gDialog.keyList;
  if (data.remark != gDialog.remark)
  {
    data.bRemarkChanged = true;
    data.remark = gDialog.remark;
  }

//  var listBox = document.getElementById('keysListbox');
//  listBox.addEventListener('ValueChange', checkDisableControls, false);

  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var theWindow = window.opener;
  if ("reviseData" in data)
  {
    if (!theWindow || !("doReviseManualCitation" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (theWindow && ("doReviseManualCitation" in theWindow))
      theWindow.doReviseManualCitation(editorElement, data.reviseData, data);
  }
  else
  {
    if (!theWindow || !("doInsertManualCitation" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (theWindow && ("doInsertManualCitation" in theWindow))
      theWindow.doInsertManualCitation(editorElement, data);
  }

//  var dd =editorControl.contentDocument;
//  var elts = dd.getElementsByTagName("dialogbase");
//  var elt = elts.item(0);
//  var str1 = serializer.serializeToString(elt);
//  var str = "<citation>" + str1 + "</citation>";
//  
//  // .getElementsByTagName("sw:dialogbase")
//  insertXMLAtCursor(editor, str, true, false);

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