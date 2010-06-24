var node;
var keys;

function dumpln(s)
{
  dump(s+"\n");
}

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
  node = activeEditor.getSelectedElement("a");
  if (node)
  {
    if (node.tagName != "a") alert("Wrong node passed to indexentry.js!");
    if (node.hasAttribute("name")) 
      document.getElementById("keylist").value = node.getAttribute("name");
  }
  initKeyList();
}


function tagConflicts()
{
  var val = document.getElementById("keylist").value;
  var i;
  for (i = 0; i<keys.length; i++)
  {
    if (val==keys[i])
    { 
      document.getElementById("uniquekeywarning").hidden = false;
      return true;
    }
  }
  document.getElementById("uniquekeywarning").hidden = true;
  return false;
}

function onAccept()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var domdoc = editor.document;
  var val = document.getElementById("keylist").value;
  if (tagConflicts()) return false;
  var newnode = !node; 
  if (newnode) 
  {
    node = domdoc.createElement("a");
  }
  node.setAttribute("name", val);
  node.setAttribute("key", val);
  if (newnode) editor.insertElementAtSelection(node, true);
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
  keys = keyString.split(/\s+/);
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
  ACSA.resetArray("key");
  for (i=0, len=keys.length; i<len; i++)
  {
    if (keys[i].length > 0) 
      ACSA.addString("key",keys[i]);
  }
  dump("Keys are : "+keys.join()+"\n");    
}
