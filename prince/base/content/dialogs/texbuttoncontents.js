// Copyright (c) 2010 MacKichan Software, Inc.  All Rights Reserved.

//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var texnode;
var editor;
var gIsEnc;
var gName;
var gReq;  
var gOpt;  
var gTeX;  
var isNewNode;

// dialog initialization code
function Startup()
{
  var childnode;
  var editorElement = msiGetParentEditorElementForDialog(window);
  editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }
  texnode = editor.getSelectedElement("texb");
  gIsEnc = document.getElementById("enc");
  gName  = document.getElementById("name");
  gReq   = document.getElementById("req");
  gOpt   = document.getElementById("opt");
  gTeX   = document.getElementById("texbuttonTextbox");
  isNewNode = !(texnode);
  if (texnode) {
    if (texnode.hasAttribute("enc")) gIsEnc.checked = texnode.getAttribute("enc") == 1;
    if (texnode.hasAttribute("name")) gName.value = texnode.getAttribute("name");
    if (texnode.hasAttribute("req")) gReq.value = texnode.getAttribute("req");
    if (texnode.hasAttribute("opt")) gOpt.value = texnode.getAttribute("opt");
    childnode = texnode.firstChild;
    while (childnode && childnode.nodeType != Node.CDATA_SECTION_NODE) {
      childnode = childnode.nextSibling;
    }
    if (childnode) 
      gTeX.value = childnode.textContent;
  }
  else texnode = editor.document.createElement("texb");

// TeX buttons looks like:
// <texb enc="0/1" name=" " req = " " opt = " ">,<![CDATA[[texbutton contents]]></texb>  

//  document.getElementById("texbuttonTextbox").focus();
  msiSetInitialDialogFocus(gTeX);
  SetWindowLocation();
}

function onAccept()
{
  try {
    while (texnode.firstChild)
    {
      texnode.removeChild(texnode.firstChild);
    }
    var newCData = editor.document.createCDATASection(gTeX.value);
    texnode.appendChild(newCData);
    if (gIsEnc.checked) texnode.setAttribute("enc", "1")
    else texnode.removeAttribute("enc");
    if (gName.value.length > 0) texnode.setAttribute("name", gName.value)
    else texnode.removeAttribute("name");
    if (gReq.value.length > 0) texnode.setAttribute("req", gReq.value)
    else texnode.removeAttribute("req");
    if (gOpt.value.length > 0) texnode.setAttribute("opt", gOpt.value)
    else texnode.removeAttribute("opt");

    SaveWindowLocation();
  }
  catch(e) {
    dump("Exception: "+e.message+"\n");
  }
  if (isNewNode) editor.insertElementAtSelection(texnode, true);
  return true;
}

function onCancel()
{
  return true;
}

