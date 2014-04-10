// user.js is a JavaScript file that is loaded into Scientific WorkPlace, Scientific Word, and Scientific Notebook.
//
// It is intended as a repository for user-defined JavaScript that might be called through keyboard macros or autosubsitute
//

// Some utility function for the use of macros and autosubstitute


function deleteSelection()
{
  dump("\ndeleteSelection();\n");
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
  dump("\ninsertMathSymbol(" + s + ")");
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
  dump("\ninsertMathname();\n");
  if (delSelection) deleteSelection();
  var editorElement = getCurrentEditorElement();
  doInsertMathName(name, editorElement);
}

function insertMathOperator( name, limitPlacement, size, delSelection)
{
  dump("\ninsertMathOperator(j);\n");
  if (delSelection) deleteSelection();
  var editorElement = getCurrentEditorElement();
  doInsertMathOperator(name, limitPlacement, size, editorElement);
}


function insertTag( name, delSelection )
{
  var editorElement = getCurrentEditorElement();
  var editor = msiGetEditor(editorElement);
  var HTMLEditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
  if (delSelection) deleteSelection();
  var tagclass = editor.tagListManager.getRealClassOfTag(name, null);
  var cmd = "";
  switch (tagclass) {
    case "texttag"  : cmd = "cmd_texttag";
      break;
    case "paratag"  :
    case "listtag"  : cmd = "cmd_paratag";
      break;
    case "structtag":
    case "envtag"   : cmd = "cmd_structtag";
      break;
    default: 
  }
  if (cmd !== "")
    msiDoStatefulCommand(cmd,name);
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

function insertFrontMatterItem( tagname, delSelection)
{
  if (delSelection) deleteSelection();
  msiDoStatefulCommand('cmd_frontmtag', tagname );
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
     

function softSave(delSelection)
{
  if (delSelection) deleteSelection();
  msiGoDoCommand('cmd_softSave');
}

function insertIntegral(delSelection)
{
  dump("\ninsertIntegeral\n");
  if (delSelection) 
    deleteSelection();
  insertMathSymbol("\u222B");
  insertMathSymbol("\u2146"); 
  insertText('x');
  //msiGoDoCommand('cmd_charPrevious');
  //msiGoDoCommand('cmd_charPrevious');
}
