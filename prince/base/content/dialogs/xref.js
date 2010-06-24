
// Copyright (c) 2010 MacKichan Software, Inc.  All Rights Reserved.

//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var refnode;
var editor;
var gKey;
var gReftype;
var newNode;

// dialog initialization code
function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }
  gKey   = document.getElementById("key");
  gReftype = document.getElementById("reftype");
  refnode = window.arguments[0];
  newNode = !(refnode);
  if (refnode) {
    if (refnode.hasAttribute("key")) gKey.value = refnode.getAttribute("key");
    if (refnode.hasAttribute("reftype")) gReftype.selectedIndex = 
      (refnode.getAttribute("reftype")=="page") ? 0 : 1;
  }
  else refnode = editor.document.createElement("xref");

// Ref buttons looks like:
// <ref key="somekey" reftype="page/obj"/>  
  initKeyList();
//  msiSetInitialDialogFocus(gKey);
  SetWindowLocation();
}

function dumpln(x) { dump(x + "\n"); }

function onAccept()
{
  try {
    dumpln(refnode.tagName);

    if (gKey.value.length > 0) refnode.setAttribute("key", gKey.value)
    else refnode.removeAttribute("key");
    dumpln("key = " + gKey.value);
    refnode.setAttribute("reftype", (gReftype.selectedIndex == 0)?"page":"obj");
    dumpln("reftype = "+refnode.getAttribute("reftype"));
    SaveWindowLocation();
  }
  catch(e) {
    dump("Exception: "+e.message+"\n");
  }
  dumpln("newNode is "+newNode);
  if (newNode) editor.insertElementAtSelection(refnode, true);
  return true;
}

function onCancel()
{
  return true;
}




var xsltSheet="<?xml version='1.0'?><xsl:stylesheet version='1.1' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:html='http://www.w3.org/1999/xhtml' ><xsl:output method='text' encoding='UTF-8'/> <xsl:template match='/'>  <xsl:apply-templates select='//*[@key]'/></xsl:template><xsl:template match='//*[@key]'>   <xsl:value-of select='@key'/><xsl:text> </xsl:text></xsl:template> </xsl:stylesheet>";

function initKeyList()
{
  var editorElement = msiGetActiveEditorElement();
  var editor;
  if (editorElement) editor = msiGetEditor(editorElement);
  var parser = new DOMParser();
  var dom = parser.parseFromString(xsltSheet, "text/xml");
  dump(dom.documentElement.nodeName == "parsererror" ? "error while parsing" + dom.documentElement.textContents : dom.documentElement.nodeName);
  var processor = new XSLTProcessor();
  processor.importStylesheet(dom.documentElement);
  var newDoc;
  if (editor) newDoc = processor.transformToDocument(editor.document, document);
  dump(newDoc.documentElement.localName+"\n");
  var keyString = newDoc.documentElement.textContent;
  var keys = keyString.split(/\s+/);
  var i;
  var len;
  keys.sort();
  var lastkey = "";
  for (i=keys.length-1; i >= 0; i--)
  {
    if (keys[i] == "" || keys[i] == lastkey) keys.splice(i,1);
    else lastkey = keys[i];
  }  
  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
  ACSA.resetArray("keys");
  for (i=0, len=keys.length; i<len; i++)
  {
    if (keys[i].length > 0) 
      ACSA.addString("keys",keys[i]);
  }
  dump("Keys are : "+keys.join()+"\n");    
}
