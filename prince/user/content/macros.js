// user.js is a JavaScript file that is loaded into Scientific WorkPlace, Scientific Word, and Scientific Notebook.
//
// It is intended as a repository for user-defined JavaScript that might be called through keyboard macros or autosubsitute
//

// Some utility function for the use of macros and autosubstitute


function deleteSelection()
{
  msiGoDoCommand('cmd_delete');
}

function insertMathSymbol( s, delSelection )
{
  if (delSelection) deleteSelection();
  doParamCommand('cmd_MSIsymbolCmd',s);
}

function insertMathunit( unit, delSelection )
{
  if (delSelection) deleteSelection();
  var editorElement = document.getElementById("content-frame");
  insertmathunit(unit, editorElement);
}

function insertMathname( name, delSelection )
{
  if (delSelection) deleteSelection();
  var editorElement = document.getElementById("content-frame");
  doInsertMathName(name, editorElement);
}

function insertTag( name, delSelection )
{
  var editorElement = document.getElementById("content-frame");
  var editor = msiGetEditor(editorElement);
  var HTMLEditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
  if (delSelection) deleteSelection();
  HTMLEditor.insertHTML("<"+name+"/>");
}
  

function insertText ( textString )
{
  var editorElement = document.getElementById("content-frame");
  var editor = msiGetEditor(editorElement);
  var plaintextEditor = editor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
  plaintextEditor.insertText( textString);
}

function checkSpelling(delSelection)
{
  if (delSelection) deleteSelection(); 
  msiGoDoCommand('cmd_spelling');
}

function toggleTextTag( tagname, delSelection )
{
  if (delSelection) deleteSelection();
  msiDoStatefulCommand('cmd_texttag', tagname );
}

function insertParaTag( tagname, delSelection )
{
  if (delSelection) deleteSelection();
  msiDoStatefulCommand('cmd_paratag', tagname );
}

function insertSectionTag( tagname, delSelection )
{
  if (delSelection) deleteSelection();
  msiDoStatefulCommand('cmd_structtag', tagname );
}

function yell ( textString, delSelection )
{
  if (delSelection) deleteSelection();
  alert(textString);
}

function insertFragmentOrMacro( name, delSelection )
{
  if (delSelection) deleteSelection();
  onMacroOrFragmentEntered( name );
}
  
function previewPDF(delSelection)
{
  if (delSelection) deleteSelection();
  printTeX(true,true);
}

function previewDVI(delSelection)
{
  if (delSelection) deleteSelection();
  printTeX(false,true);
}

     
// The following was added by BBM for diagnostic purposes. It should not be in the release version
// If the selection is collapsed and in a math object, this will dump the math object and show
// the location of the selection point.

function dumpMath()
{
  var editorElement = document.getElementById("content-frame");
  var editor = msiGetEditor(editorElement);
  var HTMLEditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
  var rootnode = HTMLEditor.getSelectionContainer();
  while (rootnode && rootnode.localName != "math") rootnode = rootnode.parentNode;
  if (!rootnode)
  { 
    dump("Failed to find math node\n");
    return;
  }
  var sel = HTMLEditor.selection;
  var selNode = sel.anchorNode;
  var selOffset = sel.anchorOffset;
  var indent = 0;
  dump("Selection: node="+selNode.nodeType == Node.TEXT_NODE?"text":selNode.localName+", offset="+selOffset+"\n");
  dumpNodeMarkingSel(rootnode, selNode, selOffset, indent);
}


function doIndent( k )
{
  for (j = 0; j < k; j++) dump("  ");
}


function dumpNodeMarkingSel(node, selnode, offset, indent)
{
  var len = node.childNodes.length;
  if (node.nodeType == Node.ELEMENT_NODE)
  {
    doIndent(indent);
    dump("<"+((node.localName!="null")?node.localName:"text")+"> \n");
    for (var i = 0; i < len; i++)
    {    
      if (node==selnode && i==offset)
      {
        for (var j = 0; j<= indent; j++) dump("**"); 
        dump("<cursor>\n");
      }
      dumpNodeMarkingSel(node.childNodes[i],selnode,offset, indent+1);
    }
    if (node==selnode && offset==len) 
    {
      for (var j = 0; j<= indent; j++) dump("**"); 
      dump("<cursor>\n");
    }
    doIndent(indent);
    dump("</"+node.localName+">\n");
  }
  else if (node.nodeType == Node.TEXT_NODE)
  {
    if (node==selnode)
    {
      doIndent(indent);
      dump(node.nodeValue.slice(0,offset));
      dump("<cursor>");
      dump(node.nodeValue.slice(offset)+"\n");
    }
    else 
    {
      var s = node.nodeValue;
      var t = s.replace(/^\s*/,'');
      var r = t.replace(/\s*$/,'');
      if (r.length>0)
      {
        doIndent(indent);
        dump(r+'\n');
      }
    }  
  }
}   
  

function softSave()
{
  msiGoDoCommand('cmd_softSave');
}

