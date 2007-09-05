// user.js is a JavaScript file that is loaded into Scientific WorkPlace, Scientific Word, and Scientific Notebook.
//
// It is intended as a repository for user-defined JavaScript that might be called through keyboard macros or autosubsitute
//

// Some utility function for the use of macros and autosubstitute


function deleteSelection()
{
  msiGoDoCommand('cmd_delete');
}

function insertMathSymbol( s )
{
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

function checkSpelling(delSelection, delSelection)
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

