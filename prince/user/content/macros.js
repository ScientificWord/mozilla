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

function insertMathunit( unit )
{
  deleteSelection();
  var editorElement = document.getElementById("content-frame");
  insertmathunit(unit, editorElement);
}

function insertMathname( name )
{
  deleteSelection();
  var editorElement = document.getElementById("content-frame");
  doInsertMathName(name, editorElement);
}

function insertTag( name )
{
  var editorElement = document.getElementById("content-frame");
  var editor = msiGetEditor(editorElement);
  var HTMLEditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
  deleteSelection(); 
  HTMLEditor.insertHTML("<"+name+"/>");
}
  

function insertText ( textString )
{
  var editorElement = document.getElementById("content-frame");
  var editor = msiGetEditor(editorElement);
  var plaintextEditor = editor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
  plaintextEditor.insertText( textString);
}

function checkSpelling()
{
  deleteSelection(); msiGoDoCommand('cmd_spelling');
}

function toggleTextTag( tagname )
{
  deleteSelection(); msiDoStatefulCommand('cmd_texttag', tagname );
}

function insertParaTag( tagname )
{
  deleteSelection(); msiDoStatefulCommand('cmd_paratag', tagname );
}

function insertSectionTag( tagname )
{
  deleteSelection(); msiDoStatefulCommand('cmd_structtag', tagname );
}

function yell ( textString )
{
  deleteSelection(); alert(textString);
}

function insertFragmentOrMacro( name )
{
  deleteSelection(); onMacroOrFragmentEntered( name );
}
  
function previewPDF()
{
  deleteSelection(); printTeX(true,true);
}

function previewDVI()
{
  deleteSelection(); printTeX(false,true);
}

