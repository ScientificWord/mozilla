// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var node;
var newNode = false;
var editor;  
var preamble;

// dialog initialization code
function Startup()
{
//  var editor = GetCurrentEditor();
  var editorElement = msiGetParentEditorElementForDialog(window);
  editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }
  preamble = editor.document.getElementsByTagName('preamble')[0];
  doSetOKCancel(onAccept, onCancel);
  node = window.arguments[0];
  if (!node)
  {
    newNode=true;
    node=editor.createNode('preambleTeX',preamble,1000); 
  }
  var str = node.textContent.replace(/[ \t]*$/,'');
  document.getElementById("preambleTextbox").value = str;
  InitDialog();

  var tb = document.getElementById("preambleTextbox");
  msiSetInitialDialogFocus(document.getElementById("preambleTextbox"));
  tb.selectionEnd = tb.selectionStart = tb.textLength;

  SetWindowLocation();
}

function InitDialog()
{
//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function onAccept()
{
  try {
    var value = document.getElementById("preambleTextbox").value;
    var cdata = node.ownerDocument.createCDATASection(value);
    while (node.firstChild) 
    {
      node.removeChild(node.firstChild);
    }
    node.appendChild(cdata);

    SaveWindowLocation();
    return true;
  }
  catch(e){
    dump("error in onAccept: "+e.message);
  }

}

function onCancel()
{
  return true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}
