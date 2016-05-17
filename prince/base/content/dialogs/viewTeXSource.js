
var editor;

function onLoadViewSource() 
{
  editor = ace.edit("texeditor");
  editor.setTheme("ace/theme/dawn");
  editor.getSession().setMode("ace/mode/latex");
  editor.setShowPrintMargin(false);
  editor.setValue(getTextFileAsString(window.arguments[0]), -1);
  editor.getSession().setUseWrapMode("free");
  editor.setReadOnly(true);
 }

