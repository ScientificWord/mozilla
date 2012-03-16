// Copyright (c) 2011 MacKichan Software, Inc.  All Rights Reserved.

//var gDialog = new Object();
var importDataIn;

var theTimer = Components.classes["@mozilla.org/timer;1"]
                    .createInstance(Components.interfaces.nsITimer);

var dlgTimerCallback = 
{  
  statusNone : 0,
  statusRunning : 1,
  statusSuccess : 2,
  statusFailed : 3,
  
  mTimer : null,
  timercopy : null,
  timerCount : 0,
//  mImpData : null,
//  importFinished : false,
//  texImportFinished : false,
  importStatus : this.statusNone,
  texImportStatus : this.statusNone,

  notify : function(timer)
  {
    if (!this.timercopy)
      this.timercopy = timer;
    if ( (this.importStatus != this.statusRunning) && (this.texImportStatus != this.statusRunning) )
    {
      timer.cancel();
      return;
    }
    this.checkLoadingComplete();
    ++this.timerCount;  //May want to display time elapsed in dialog
    if (!(this.timerCount % 8))
      this.checkLogFileStatus();
//    dump("In graphics convert dialog timerCallback, timer count is [" + this.timerCount + "].\n");
    this.disableDialogControls();
    if ((this.importStatus != this.statusRunning) && (this.texImportStatus != this.statusRunning))
    {
      this.stopTimer();
      dump("In msiGraphicsConversionDlg.notify; calling stopTimer()\n");
      this.closeTheDialog();
      return true;
    }
  },
  stopTimer : function()
  {
    if (this.timercopy)
      this.timercopy.cancel();
//    if (this.mTimer && this.mTimer != this.timercopy)
//      this.mTimer.cancel();
  },
  
  setData : function(impData, theWindow, theDocument)
  {
    this.importProcess = impData.mImportProcess;
    this.texImportProcess = impData.mTexImportProcess;
    this.importTargFile = impData.mImportTargFile;
    this.texImportTargFile = impData.mTexImportTargFile;
    this.importSentinelFile = impData.mImportSentinelFile;
    this.texSentinelFile = impData.mTexSentinelFile;
    this.importLogFile = impData.mImportLogFile;
    this.texImportLogFile = impData.mTexImportLogFile;
    this.importStatus = impData.mImportStatus;
    this.texImportStatus = impData.mTexImportStatus;
    this.importDataInOut = impData;
    this.ourWindow = theWindow;
    this.ourDocument = theDocument;
  },

  copyDataOut : function(impData)
  {
    impData.mImportStatus = this.importStatus;
    impData.mTexImportStatus = this.texImportStatus;
  },

  forceEnd : function(mode)
  {
    if (mode == "tex")
    {
      if (this.texImportStatus == this.statusRunning)
      {
        this.texImportProcess.kill();
        this.texImportStatus = this.statusFailed;
      }
    }
    else
    {
      if (this.importStatus == this.statusRunning)
      {
        this.importProcess.kill();
        this.importStatus = this.statusFailed;
      }
    }
  },

  checkLogFileStatus : function()
  {
    var errorRE = /(error)|(fail)/i;
    var logUrl;
    if (this.texImportStatus == this.statusRunning)
    {
      if (this.texImportLogFile && this.texImportLogFile.exists())
      {
        var logStr;
        try
        {
          logUrl = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService).newFileURI(this.texImportLogFile);
          logStr = getFileAsString(logUrl.spec);
        } catch(ex) {dump("Exception in msiGraphicsConversionDlg.js, checkLogFileStatus: " + ex + "\n");}
        if (logStr)
        {
          if (errorRE.exec(logStr))
            this.texImportStatus = this.statusFailed;
        }
        else
          dump("Couldn't read logfile [" + logUrl + "], from nsILocalFile [" + this.texImportLogFile.path + "]!\n");
      }
    }
    if (this.importStatus == this.statusRunning)
    {
      if (this.importLogFile && this.importLogFile.exists())
      {
        var logStr;
        try
        {
          logUrl = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService).newFileURI(this.importLogFile);  
          logStr = getFileAsString(logUrl.spec);
        } catch(ex) {dump("Exception in msiGraphicsConversionDlg.js, checkLogFileStatus: " + ex + "\n");}
        if (logStr)
        {
          if (errorRE.exec(logStr))
            this.importStatus = this.statusFailed;
        }
        else
          dump("Couldn't read logfile " + logUrl + "], from nsILocalFile [" + this.importLogFile.path + "]!\n");
      }
    }
  },

  closeTheDialog : function()
  {
    this.forceEnd("import");
    this.forceEnd("tex");
//    if (theTimer)
//      theTimer.cancel();
//    theTimer = null;
    this.stopTimer();
//    Components.utils.reportError("in onCancel\n");
    this.copyDataOut(this.importDataInOut);
    this.ourWindow.SaveWindowLocation();
//    top.document.commandDispatcher.focusedWindow.focus();  
    this.ourWindow.close();
  },

  checkLoadingComplete : function()
  {
    if (this.importStatus == this.statusRunning)
    {
      if (this.importTargFile.exists())
        this.importStatus = this.statusSuccess;
      else if (this.importSentinelFile && this.importSentinelFile.exists())
        this.importStatus = this.statusFailed;
    }
    if (this.texImportStatus == this.statusRunning)
    {
      if (this.texImportTargFile.exists())
        this.texImportStatus = this.statusSuccess;
      else if (this.texSentinelFile && this.texSentinelFile.exists())
        this.texImportStatus = this.statusFailed;
    }
    var loggingStrs = ["none", "running", "success", "failed"];
//    dump("In msiGraphicsConversionDlg.checkLoadingComplete; importStatus is [" + loggingStrs[this.importStatus] + "] and texImportStatus is [" + loggingStrs[this.texImportStatus] + "].\n");
    
    return false;
  },

  disableDialogControls : function()
  {
    var stopButton = this.ourDocument.getElementById("stopImportButton");
    var texStopButton = this.ourDocument.getElementById("stopImportTeXButton");
    if (this.importStatus != this.statusRunning)
      stopButton.disabled = true;
    if (this.texImportStatus != this.statusRunning)
      texStopButton.disabled = true;
  }

};

function disableControls(importRunning, texImportRunning)
{
  if (!importRunning)
    gDialog.importControls[1].disabled = true;
  if (!texImportRunning)
    gDialog.texImportControls[1].disabled = true;
}

function forceEndLoad(control)
{
  if (control.id == "stopImportTeXButton")
  {
    dlgTimerCallback.forceEnd("tex");
  }
  else
  {
    dlgTimerCallback.forceEnd("import");
  }
  dlgTimerCallback.checkLoadingComplete();
}


function Startup()
{
  gDialog.importControls = [document.getElementById("importLoadingText"), document.getElementById("stopImportButton")];
  gDialog.texImportControls = [document.getElementById("texImportLoadingText"), document.getElementById("stopImportTeXButton")];

  importDataIn = window.arguments[0];
  var ctrlText = "";
  if (importDataIn.mImportProcess)
  {
    ctrlText = gDialog.importControls[0].textContent;
    ctrlText = ctrlText.replace("--file--", importDataIn.mSourceFile.leafName);
    gDialog.importControls[0].textContent = ctrlText;
  }
  else
    showHideControls(gDialog.importControls, false);
  if (importDataIn.mTexImportProcess)
  {
    ctrlText = gDialog.texImportControls[0].textContent;
    ctrlText = ctrlText.replace("--file--", importDataIn.mSourceFile.leafName);
    gDialog.texImportControls[0].textContent = ctrlText;
  }
  else
    showHideControls(gDialog.texImportControls, false);

  //Components.utils.reportError("in Init\n");
  // set up the first pass
  dlgTimerCallback.setData(importDataIn, window, document);
  dlgTimerCallback.mTimer = theTimer;
  theTimer.initWithCallback( dlgTimerCallback, 500, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
}

function onAccept()
{
  doEndDialog();
  return true;
}

function doEndDialog()
{
//  dump("In msiGraphicsConversionDlg.doEndDialog\n");
//  if (importData.mImportProcess)
  forceEndLoad(gDialog.importControls[1]);
//  if (importData.mTexImportProcess)
  forceEndLoad(gDialog.texImportControls[1]);
//  if (theTimer)
//    theTimer.cancel();
//  theTimer = null;
  dlgTimerCallback.stopTimer();
//  Components.utils.reportError("in onCancel\n");
  dlgTimerCallback.copyDataOut(importDataIn);
  SaveWindowLocation();
  top.document.commandDispatcher.focusedWindow.focus();  
  window.close();
//  dump("In msiGraphicsConversionDlg.doEndDialog, after window.close call\n");
  return true;
}