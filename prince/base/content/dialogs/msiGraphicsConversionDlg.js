// Copyright (c) 2011 MacKichan Software, Inc.  All Rights Reserved.

//var gDialog = new Object();
var importData;

var importFinished=false;
var texImportFinished=false;

var theTimer = Components.classes["@mozilla.org/timer;1"]
                    .createInstance(Components.interfaces.nsITimer);

var dlgTimerCallback = 
{  
  timercopy : null,
  timerCount : 0,
  notify : function(timer)
  {
    if (!this.timercopy)
      this.timercopy = timer;
    if (!importData.mImportProcess && !importData.mTexImportProcess)
    {
      timer.cancel();
      return;
    }
    checkLoadingComplete();
    ++this.timerCount;  //May want to display time elapsed in dialog
//    dump("In graphics convert dialog timerCallback, timer count is [" + this.timerCount + "].\n");
  },
  stopTimer : function()
  {
    if (this.timercopy)
      this.timercopy.cancel();
  }
};

function forceEndLoad(control)
{
  if (control.id == "stopImportTeXButton")
  {
    importData.mTexImportProcess.kill();
    importData.mTexImportProcess = null;
  }
  else
  {
    importData.mImportProcess.kill();
    importData.mImportProcess = null;
  }
  checkLoadingComplete();
}

function checkLoadingComplete()
{
  if (!importFinished && importData.mImportProcess)
  {
    importFinished = importData.mImportTargFile.exists();
    if (importFinished)
      gDialog.importControls[1].disabled = true;
  }
  if (!texImportFinished && importData.mTexImportProcess)
  {
    texImportFinished = importData.mTexImportTargFile.exists();
    if (texImportFinished)
      gDialog.texImportControls[1].disabled = true;
  }
//  dump("In msiGraphicsConversionDlg.checkLoadingComplete; importFinished is [" + (importFinished ? "true" : "false") + "] and texImportFinished is [" + (texImportFinished ? "true" : "false") + "].\n");
  if ((importFinished || !importData.mImportProcess) && (texImportFinished || !importData.mTexImportProcess))
  {
    dlgTimerCallback.stopTimer();
//    theTimer.cancel();
    doEndDialog();
    return true;
  }
  return false;
}


function Startup()
{
  gDialog.importControls = [document.getElementById("importLoadingText"), document.getElementById("stopImportButton")];
  gDialog.texImportControls = [document.getElementById("texImportLoadingText"), document.getElementById("stopImportTeXButton")];

  importData = window.arguments[0];
  var ctrlText = "";
  if (importData.mImportProcess)
  {
    ctrlText = gDialog.importControls[0].textContent;
    ctrlText = ctrlText.replace("--file--", importData.mSourceFile.leafName);
    gDialog.importControls[0].textContent = ctrlText;
  }
  else
    showHideControls(gDialog.importControls, false);
  if (importData.mTexImportProcess)
  {
    ctrlText = gDialog.texImportControls[0].textContent;
    ctrlText = ctrlText.replace("--file--", importData.mSourceFile.leafName);
    gDialog.texImportControls[0].textContent = ctrlText;
  }
  else
    showHideControls(gDialog.texImportControls, false);

  //Components.utils.reportError("in Init\n");
  // set up the first pass
  theTimer.initWithCallback( dlgTimerCallback, 250, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
}

function onAccept()
{
  doEndDialog();
  return true;
}

function doEndDialog()
{
//  dump("In msiGraphicsConversionDlg.doEndDialog\n");
  if (importData.mImportProcess)
    forceEndLoad(gDialog.importControls[1]);
  if (importData.mTexImportProcess)
    forceEndLoad(gDialog.texImportControls[1]);
//  if (theTimer)
//    theTimer.cancel();
//  theTimer = null;
  dlgTimerCallback.stopTimer();
//  Components.utils.reportError("in onCancel\n");
  SaveWindowLocation();
  top.document.commandDispatcher.focusedWindow.focus();  
  window.close();
//  dump("In msiGraphicsConversionDlg.doEndDialog, after window.close call\n");
  return true;
}