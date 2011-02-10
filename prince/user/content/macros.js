// user.js is a JavaScript file that is loaded into Scientific WorkPlace, Scientific Word, and Scientific Notebook.
//
// It is intended as a repository for user-defined JavaScript that might be called through keyboard macros or autosubsitute
//

// Some utility function for the use of macros and autosubstitute


function deleteSelection()
{
  msiGoDoCommand('cmd_delete');
}

function getCurrentEditorElement()
{
  var theEditorElement = msiGetActiveEditorElement();
  return theEditorElement;
}


function insertMathSymbol( s, delSelection )
{
  if (delSelection) deleteSelection();
  doParamCommand('cmd_MSIsymbolCmd',s);
}

function insertMathunit( unit, delSelection )
{
  if (delSelection) deleteSelection();
  var editorElement = getCurrentEditorElement();
  insertmathunit(unit, editorElement);
}

function insertMathname( name, delSelection )
{
  dump("\n ** insert math name");
  if (delSelection) deleteSelection();
  dump("\n ** selection deleted");
  var editorElement = getCurrentEditorElement();
  dump("\n ** editorElement = " + editorElement);
  doInsertMathName(name, editorElement);
  dump("\n ** did insert math name\n");
}

function insertTag( name, delSelection )
{
  var editorElement = getCurrentEditorElement();
  var editor = msiGetEditor(editorElement);
  var HTMLEditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
  if (delSelection) deleteSelection();
  HTMLEditor.insertHTML("<"+name+"/>");
}
  

function insertText ( textString )
{
  var editorElement = getCurrentEditorElement();
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

function insertListItem( tagname, delSelection )
{
  if (delSelection) deleteSelection();
  msiDoStatefulCommand('cmd_paratag', tagname );
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

function insertFootnote(delSelection)
{
  if (delSelection) deleteSelection();
  var editorElement = getCurrentEditorElement();
  msiNote(null, editorElement, 'footnote', false);
}
     
function insertMarginNote(delSelection)
{
  if (delSelection) deleteSelection();
  var editorElement = getCurrentEditorElement();
  msiNote(null, editorElement, 'marginnote',false);
}
     
// The following was added by BBM for diagnostic purposes. It should not be in the release version
// If the selection is collapsed and in a math object, this will dump the math object and show
// the location of the selection point.

function dumpMath()
{
  var editorElement = getCurrentEditorElement();
  var editor = msiGetEditor(editorElement);
  var HTMLEditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
  var rootnode = HTMLEditor.getSelectionContainer();
  while (rootnode && rootnode.localName != "math" && editor.tagListManager.getTagInClass("paratags",rootnode.localName,null)) rootnode = rootnode.parentNode;
  if (!rootnode)
  { 
    dump("Failed to find math or paragraph node\n");
    return;
  }
  var sel = HTMLEditor.selection;
  var selNode = sel.anchorNode;
  var selOffset = sel.anchorOffset;
  var focNode = sel.focusNode;
  var focOffset = sel.focusOffset;
  var indent = 0;
  dump(selNode.toString() + " " + focNode.toString()+"\n");
  dump("Selection: selNode="+selNode.nodeType == Node.TEXT_NODE?"text":selNode.localName+", offset="+selOffset+"\n");
  dump("           focusNode="+focNode.nodeType == Node.TEXT_NODE?"text":focNode.localName+", offset="+focOffset+"\n");
  dumpNodeMarkingSel(rootnode, selNode, selOffset, focNode, focOffset, indent);
}


function doIndent( k )
{
  for (j = 0; j < k; j++) dump("  ");
}


function dumpNodeMarkingSel(node, selnode, seloffset, focnode, focoffset, indent)
{
  var len = node.childNodes.length;
  if (node.nodeType == Node.ELEMENT_NODE)
  {
    doIndent(indent);
    dump("<"+node.localName+"> \n");
    for (var i = 0; i < len; i++)
    {    
      if (node==selnode && i==seloffset)
      {
        for (var j = 0; j<= indent; j++) dump("**"); 
        dump("<selection anchor>\n");
      }
      if (node==focnode && i==focoffset)
      {
        for (var j = 0; j<= indent; j++) dump("**"); 
        dump("<selection focus>\n");
      }
      dumpNodeMarkingSel(node.childNodes[i],selnode,seloffset, focnode, focoffset, indent+1);
    }
    if (node==selnode && seloffset==len) 
    {
      for (var j = 0; j<= indent; j++) dump("**"); 
      dump("<selection anchor>\n");
    }
    if (node==focnode && focoffset==len) 
    {
      for (var j = 0; j<= indent; j++) dump("**"); 
      dump("<selection focus>\n");
    }
    doIndent(indent);
    dump("</"+node.localName+">\n");
  }
  else if (node.nodeType == Node.TEXT_NODE)
  {
    if (node==selnode)
    {
      doIndent(indent);
      dump(node.nodeValue.slice(0,seloffset));
      dump("<selection anchor>");
      dump(node.nodeValue.slice(seloffset)+"\n");
    }
    else {
      if (node==focnode)
      {
        doIndent(indent);
        dump(node.nodeValue.slice(0,focoffset));
        dump("<selection focus>");
        dump(node.nodeValue.slice(focoffset)+"\n");
      }
      else {
        var s = node.nodeValue;
        var t = s.replace(/^\s*/,'');
        var r = t.replace(/\s*$/,'');
        if (r.length>0)
        {
          doIndent(indent);
          dump(r+'\n');
        }
        else dump("whitespace node\n");
      }
    }  
  }
}   
  

function softSave(delSelection)
{
  if (delSelection) deleteSelection();
  msiGoDoCommand('cmd_softSave');
}

