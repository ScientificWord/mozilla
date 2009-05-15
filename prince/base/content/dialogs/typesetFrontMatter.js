// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//const mmlns    = "http://www.w3.org/1998/Math/MathML";
//const xhtmlns  = "http://www.w3.org/1999/xhtml";

var data;

// dialog initialization code
function Startup()
{
//  var editor = GetCurrentEditor();
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }

  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  data.Cancel = false;
  gDialog.frontMatterText = data.frontMatterText;

  InitDialog();

//  document.getElementById("editControl").focus();
  msiSetInitialDialogFocus(document.getElementById("editControl"));

  SetWindowLocation();
}

function InitDialog()
{
  var editorControl = document.getElementById("editControl");
  var commandManager = editorControl.commandManager;
//  commandManager.addCommandObserver(editorDocumentObserver, "obs_documentCreated");
  var theFirstText = gDialog.frontMatterText;
  if (!theFirstText.length)
    theFirstText = "<span style='font-weight: bold;'>Some bold text?</span>";
  theFirstText = "<para>" + theFirstText + "</para>";
  msiInitializeEditorForElement(editorControl, theFirstText);

//  var editorControlTwo = document.getElementById("editControlTwo");
//  var commandManagerTwo = editorControlTwo.commandManager;
////  commandManagerTwo.addCommandObserver(editorDocumentObserverTwo, "obs_documentCreated");
////  EditorStartupForEditorElement(editorControlTwo);
//  var theSecondText = "<span style='font-style: italic;'>Some italic text?</span>";
//  msiInitializeEditorForElement(editorControlTwo, theSecondText);
  checkInaccessibleAcceleratorKeys(document.documentElement);

  gDialog.tabOrderArray = new Array( editorControl, /* editorControlTwo, */
                                       document.documentElement.getButton("accept"),
                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}


//var editorDocumentObserver = 
//{
//  observe: function(aSubject, aTopic, aData)
//  { 
//    if (aTopic == "obs_documentCreated")
//    {
//      var editorControl = document.getElementById("editControl");
//      var htmlEditor = editorControl.getHTMLEditor(editorControl.contentWindow);
//      var theText = gDialog.frontMatterText;
//      if (!theText.length)
//        theText = "<span style='font-weight: bold;'>More bold text, added later?</span>";
//      htmlEditor.insertHTML(theText);
//    }  
//  }
//}
//
//var editorDocumentObserverTwo = 
//{
//  observe: function(aSubject, aTopic, aData)
//  { 
//    if (aTopic == "obs_documentCreated")
//    {
//      var editorControl = document.getElementById("editControlTwo");
//      var htmlEditor = editorControl.getHTMLEditor(editorControl.contentWindow);
////      var theText = gDialog.frontMatterText;
////      if (!theText.length)
//      var theText = "<span style='font-style: italic;'>Some italic text?</span>";
//      htmlEditor.insertHTML(theText);
//    }  
//  }
//}

function onAccept()
{
  var editorControl = document.getElementById("editControl");
  var serializer = new XMLSerializer();
  data.frontMatterText = serializer.serializeToString(editorControl.contentDocument.documentElement);

  var editorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (!theWindow || !("insertFrontMatter" in theWindow))
    theWindow = msiGetTopLevelWindow(window);
  try
  {
    AlertWithTitle("Information", "Inserting front matter: \n\n" + data.frontMatterText);
    theWindow.insertFrontMatter(editorElement, data);
  }
  catch(exc) {AlertWithTitle("Error in typesetFrontMatter dialog", "Error in onAccept: " + exc);}

  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}
