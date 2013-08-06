
// Copyright (c) 2010 MacKichan Software, Inc.  All Rights Reserved.

//const xhtmlns  = "http://www.w3.org/1999/xhtml";

//var refnode;
var editorElement;
var gKey;
var gReftype;
var bIsRevise;
var data;
var gDialog = new Object();

// dialog initialization code
function Startup()
{
  editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }
  
  data = window.arguments[0];
  bIsRevise = false;
  data.Cancel = false;
  gDialog.key = data.key;
  gDialog.refType = data.refType;
  gDialog.vario.checked = data.vario;
  if (data && ("reviseData" in data))
    setDataFromReviseData(data.reviseData);

  gKey   = document.getElementById("keylist");
  gReftype = document.getElementById("reftype");
//  refnode = getSelectionParentByTag(editor, "xref");
// Ref buttons looks like:
// <ref key="somekey" reftype="page/obj"/>  
//  initKeyList();
//  msiSetInitialDialogFocus(gKey);
  gDialog.markerList = new msiKeyMarkerList(window);
  gDialog.markerList.setUpTextBoxControl(gKey);
//  var autocompleteKeyString = gDialog.markerList.getIndexString();
//  gKey.setAttribute("autocompletesearchparam", autocompleteKeyString);

  gKey.value = gDialog.key;
  gReftype.value = gDialog.refType;
  SetWindowLocation();
}

function setDataFromReviseData(reviseData)
{
//  isNewNode = !(refnode);
  var refnode = reviseData.getReferenceNode();
  if (refnode)
  {
    if (refnode.hasAttribute("key")) 
      gDialog.key = refnode.getAttribute("key");
    else if (refnode.hasAttribute("href"))
      gDialog.key = refnode.getAttribute("href");
    if (refnode.hasAttribute("reftype")) 
      gDialog.refType = refnode.getAttribute("reftype");
    if (refnode.getAttribute("req")=="varioref")
      gDialog.vario.checked = true;
    else gDialog.vario.checked = false;
    bIsRevise = true;
  }
}

function onAccept()
{
  data.refType = gReftype.value;
  data.key = gKey.value;
  data.vario = document.getElementById("varioref").checked;
  try
  {
    var theWindow = window.opener;
    if (bIsRevise)
    {
      var xrefNode = data.reviseData.getReferenceNode();
      if (!theWindow || !("doReviseCrossReference" in theWindow))
        theWindow = msiGetTopLevelWindow();
      if (theWindow && ("doReviseCrossReference" in theWindow))
        theWindow.doReviseCrossReference(editorElement, xrefNode, data);
    }
    else
    {
      if (!theWindow || !("doInsertCrossReference" in theWindow))
        theWindow = msiGetTopLevelWindow();
      if (theWindow && ("doInsertCrossReference" in theWindow))
        theWindow.doInsertCrossReference(editorElement, data);
    }
    SaveWindowLocation();
  }
  catch(e) { dump("In xref dialog, exception: "+e.message+"\n"); }
  
  return true;
}

function onCancel()
{
  data.Cancel = true;
  return true;
}


//var xsltSheet="<?xml version='1.0'?><xsl:stylesheet version='1.1' xmlns:xsl='http://www.w3.org/1999/XSL/Transform' xmlns:html='http://www.w3.org/1999/xhtml' ><xsl:output method='text' encoding='UTF-8'/> <xsl:template match='/'>  <xsl:apply-templates select='//*[@key]'/></xsl:template><xsl:template match='//*[@key]'>   <xsl:value-of select='@key'/><xsl:text> </xsl:text></xsl:template> </xsl:stylesheet>";
//
//function initKeyList()
//{
//  var editorElement = msiGetActiveEditorElement();
//  var editor;
//  if (editorElement) editor = msiGetEditor(editorElement);
//  var parser = new DOMParser();
//  var dom = parser.parseFromString(xsltSheet, "text/xml");
//  dump(dom.documentElement.nodeName == "parsererror" ? "error while parsing" + dom.documentElement.textContents : dom.documentElement.nodeName);
//  var processor = new XSLTProcessor();
//  processor.importStylesheet(dom.documentElement);
//  var newDoc;
//  if (editor) newDoc = processor.transformToDocument(editor.document, document);
//  dump(newDoc.documentElement.localName+"\n");
//  var keyString = newDoc.documentElement.textContent;
//  var keys = keyString.split(/\s+/);
//  var i;
//  var len;
//  keys.sort();
//  var lastkey = "";
//  for (i=keys.length-1; i >= 0; i--)
//  {
//    if (keys[i] == "" || keys[i] == lastkey) keys.splice(i,1);
//    else lastkey = keys[i];
//  }  
//  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//  ACSA.resetArray("key");
//  for (i=0, len=keys.length; i<len; i++)
//  {
//    if (keys[i].length > 0) 
//      ACSA.addString("key",keys[i]);
//  }
//  dump("Keys are : "+keys.join()+"\n");    
//}
