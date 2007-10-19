function Startup() {
  var target = window.arguments[0];
  var substitutionStr = "";
  gDialog.bDataModified = false;
  gDialog.bEditorReady = false;

  var editElement = document.getElementById("sectiontitle-frame");
//   var sectiontitleControlObserver = new msiEditorChangeObserver(editElement);
//   var commandBoldObserverData = new Object();
//   commandBoldObserverData.mCommand = "cmd_bold";
//   commandBoldObserverData.mObserver = sectiontitleControlObserver;
//   var commandSetModifiedObserverData = new Object();
//   commandSetModifiedObserverData.mCommand = "cmd_setDocumentModified";
//   commandSetModifiedObserverData.mObserver = sectiontitleControlObserver;
//   var editorDocLoadedObserverData = new Object();
//   editorDocLoadedObserverData.mCommand = "obs_documentCreated";
//   editorDocLoadedObserverData.mObserver = sectiontitleControlObserver;
// 
//   editElement.mInitialDocObserver = [commandBoldObserverData, commandSetModifiedObserverData, editorDocLoadedObserverData];
  msiInitializeEditorForElement(editElement, substitutionStr);
}








function onOK() {
// Copy the contents of the editor to the section header form
  SaveWindowLocation();
  return false;
}

function onCancel() {
  return(true);
}

