// Copyright (c) 2010-13 MacKichan Software, Inc.  All Rights Reserved.

var data;

function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var node;
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
  if (!data.node) data.node = editor.createNode("bibitem", data.paragraphNode, data.offset);
  if (data.node) {
    node = getChildByTagName(data.node, "bibkey");
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

function handleEmptyBibItem(editor, node)
{
  if (!node) return false;
  var pnode = node.parentNode;
  var bibparent;
  var offset;
  if (!(pnode) || pnode.tagName !== "bibliography") return false;
  var bibitems = msiNavigationUtils.getChildrenByTagName(pnode, "bibitem");
  if (!bibitems) return false;
  if (bibitems.length <= 1) {
    bibparent = pnode.parentNode;
    offset = msiNavigationUtils.offsetInParent(pnode);
    editor.deleteNode(pnode);
    editor.selection.collapse(bibparent, offset);
    // BBM: maybe should check that bibparent is a paragraph type.
  }
  else if (bibitems[bibitems.length - 1] == node) {
    bibparent = pnode.parentNode;
    offset = msiNavigationUtils.offsetInParent(pnode);
    offset = offset + 1;
    editor.deleteNode(node);
    editor.selection.collapse(bibparent, offset);
  }
  SaveWindowLocation();
  return true;
}


function onAccept()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var keyList = document.getElementById("keysAutoCompleteBox");
  gDialog.key = document.getElementById("keysAutoCompleteBox").value;
  data.key = gDialog.key;
  if (/^\s*$/.test(data.key)) {
    // User has entered an empty (by our definition) bib entry.
    // If it is the last one, we delete it and move the cursor past the <bibliography>
    // If it is the only one, we remove the <bibliography>
    if (handleEmptyBibItem(editor, data.node)) return true;
  }

  var labelControl = document.getElementById("labelEditControl");
  var labelContentFilter = new msiDialogEditorContentFilter(labelControl);
  gDialog.bibLabel = labelContentFilter.getDocumentFragmentString();

  if (data.bibLabel != gDialog.bibLabel)
  {
    data.bBibLabelChanged = true;
    data.bibLabel = gDialog.bibLabel;
  }

//  var listBox = document.getElementById('keysListbox');
//  listBox.addEventListener('ValueChange', checkDisableControls, false);

  var theWindow = window.opener;
  if (data.node)
  {
    var node;
    var node2;
    node = getChildByTagName(data.node, "bodyText");
    if (!node) 
    {
      node = editor.createNode("bodyText", data.node, -1);
    }
    editor.selection.collapse(node,0);

    node2 = getChildByTagName(data.node, "bibkey");
    if (node2) node2.textContent = gDialog.key;
    else {
      node2 = editor.createNode("bibkey", data.node, 0);
      node2.textContent = gDialog.key;
    }
    node2 = getChildByTagName(data.node, "biblabel");
    if (node2) node2.innerHTML = gDialog.bibLabel;
    else {
      node2 = editor.createNode("biblabel", data.node, 0);
      node2.innerHTML = gDialog.bibLabel;
    }
  }
  else if ("paragraphNode" in data)
  {
    if (editor.tagListManager.getClassOfTag(data.paragraphNode.tagName, null) !== "listparenttag")
    {
      if (!theWindow || !("msiDoTagBibItem" in theWindow))
        theWindow = msiGetTopLevelWindow();
      if (theWindow && ("msiDoTagBibItem" in theWindow))
        theWindow.msiDoTagBibItem(data, data.paragraphNode, editorElement);
    }

  }
  editor.selection.collapse(node,0);
  msiUpdateStructToolbar(editorElement);
  SaveWindowLocation();
  return true;
}

function onCancel()
{  
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  gDialog.key = document.getElementById("keysAutoCompleteBox").value;
  data.key = gDialog.key;
  if (/^\s*$/.test(data.key)) {
    // User has entered an empty (by our definition) bib entry.
    // If it is the last one, we delete it and move the cursor past the <bibliography>
    // If it is the only one, we remove the <bibliography>
    if (handleEmptyBibItem(editor, data.node)) return true;
  }

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