
var data;
function startUp()
{
  data = window.arguments[0];
  if (data.type)
  {
    document.getElementById("note.names").value = data.type;
    document.getElementById("hidenote").checked = data.hide;
  }
  else data.type = document.getElementById("note.names").value;
  if (data.noteNode == null)
		document.getElementById("hidenote").disabled = true;
  checkEnable();
}

  
function checkEnable() {
  if (document.getElementById("note.names").value == "footnote")
    document.getElementById("optionsButton").disabled = false;
  else
    document.getElementById("optionsButton").disabled = true;
}

function launchOptionsDialog() {
  var optionsData = {markOrText : "markAndText"};
  if ("markOrText" in data)
    optionsData.markOrText = data.markOrText;
  if ("footnoteNumber" in data)
    optionsData.overrideNumber = Number(data.footnoteNumber);

  window.openDialog("chrome://prince/content/NoteOptionsDialog.xul", "noteoptions", "chrome,close,titlebar,modal,resizable", optionsData);
  if (!optionsData.Cancel)
  {
    if ("overrideNumber" in optionsData)
      data.footnoteNumber = optionsData.overrideNumber;
    else if ("footnoteNumber" in data)
      delete data.footnoteNumber;
    data.markOrText = optionsData.markOrText;
  }
}

function onOK() {
  data.type = document.getElementById("note.names").value;
  data.hide = document.getElementById("hidenote").checked;

  try
  {
    var editorElement = msiGetParentEditorElementForDialog(window);
    var parentEditor = msiGetEditor(editorElement);
    var theWindow = window.opener;
    if (!theWindow || !("msiReviseNote" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (parentEditor && theWindow)
      theWindow.msiInsertOrReviseNote(data.noteNode, editorElement, data);
  }
  catch(exc) {dump("Exception in onOK of Note dialog: [" + exc + "].\n");}

  close();
  return (false);
}

function onCancel() {
  data.Cancel = true;
  close();
  return(true);
}

