// Copyright (c) 2011 MacKichan Software, Inc.  All Rights Reserved.

//var gDialog = new Object();
var importDataIn;

Components.utils.import("resource://app/modules/graphicsConverter.jsm");

//var theTimer = Components.classes["@mozilla.org/timer;1"]
//                    .createInstance(Components.interfaces.nsITimer);

var dlgTimerCallbackObject = 
{  
  mImportHandler : null,
  mTexHandler : null,
  importProcess : null,
  texImportProcess : null,
  importTargFile : null,
  texImportTargFile : null,
//  importStatus : this.statusNone,
//  texImportStatus : this.statusNone,

//  notify : function(timer)
//  {
//    if (!this.timercopy)
//      this.timercopy = timer;
//    if ( (this.importStatus != this.statusRunning) && (this.texImportStatus != this.statusRunning) )
//    {
//      timer.cancel();
//      return;
//    }
//    this.checkLoadingComplete();
//    ++this.timerCount;  //May want to display time elapsed in dialog
//    if (!(this.timerCount % 8))
//      this.checkLogFileStatus();
////    dump("In graphics convert dialog timerCallback, timer count is [" + this.timerCount + "].\n");
//    this.disableDialogControls();
//    if ((this.importStatus != this.statusRunning) && (this.texImportStatus != this.statusRunning))
//    {
//      this.stopTimer();
//      dump("In msiGraphicsConversionDlg.notify; calling stopTimer()\n");
//      this.closeTheDialog();
//      return true;
//    }
//  },

  stopTimer : function()
  {
    if (this.mImportHandler)
      this.mImportHandler.stop();
    if (this.mTexHandler)
      this.mTexHandler.stop();
  },
  
  init : function(impData, theWindow, theDocument)
  {
//    if (impData.mImportProcess)
//    {
//      this.importProcess = impData.mImportProcess;
//      this.mImportHandler = new graphicsTimerHandler(graphicsTimerHandler.maxGraphicsLoadTime, this);
//      this.importTargFile = impData.mImportTargFile;
//      var importTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
//      importTimer.initWithCallback( this.mImportHandler, 200, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
//      this.mImportHandler.startLoading(impData.mSourceFile, impData.mImportTargFile, impData.mImportProcess);
//    }
//    if (impData.mTexImportProcess)
//    {
//      this.texImportProcess = impData.mTexImportProcess;
//      this.mTexHandler = new graphicsTimerHandler(graphicsTimerHandler.maxGraphicsLoadTime, this);
//      this.texImportTargFile = impData.mTexImportTargFile;
//      var texImportTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
//      texImportTimer.initWithCallback( this.mTexHandler, 200, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
//      this.mTexHandler.startLoading(impData.mSourceFile, impData.mTexImportTargFile, impData.mTexImportProcess);
//    }
//    this.importSentinelFile = impData.mImportSentinelFile;
//    this.texSentinelFile = impData.mTexSentinelFile;
//    this.importLogFile = impData.mImportLogFile;
//    this.texImportLogFile = impData.mTexImportLogFile;
//    this.importStatus = impData.mImportStatus;
//    this.texImportStatus = impData.mTexImportStatus;
    this.importDataInOut = impData;
    this.ourWindow = theWindow;
    this.ourDocument = theDocument;
  },

  copyDataOut : function(impData)
  {
    impData.mImportStatus = this.importStatus;
    impData.mTexImportStatus = this.texImportStatus;
  },

  terminalCallback : function(sourceFile, targFile, importStatus, errorString)
  {
    this.checkLoadingComplete();
    this.disableDialogControls();
    if (!this.isLoading())
    {
      this.stopTimer();
//      dump("In msiGraphicsConversionDlg.notify; calling stopTimer()\n");
      this.closeTheDialog();
      return true;
    }
  },

  forceEnd : function(mode)
  {
    if (mode == "tex")
    {
      if (this.mTexHandler && this.mTexHandler.isLoading())
      {
        this.mTexHandler.stop();
        this.texImportStatus = this.mTexHandler.importStatus;
      }
    }
    else
    {
      if (this.mImportHandler && this.mImportHandler.isLoading())
      {
        this.mImportHandler.stop();
        this.importStatus = this.mImportHandler.importStatus;
      }
    }
  },

//  checkLogFileStatus : function()
//  {
//    var errorRE = /(error)|(fail)/i;
//    var logUrl;
//    if (this.texImportStatus == this.statusRunning)
//    {
//      if (this.texImportLogFile && this.texImportLogFile.exists())
//      {
//        var logStr;
//        try
//        {
//          logUrl = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService).newFileURI(this.texImportLogFile);
//          logStr = getFileAsString(logUrl.spec);
//        } catch(ex) {dump("Exception in msiGraphicsConversionDlg.js, checkLogFileStatus: " + ex + "\n");}
//        if (logStr)
//        {
//          if (errorRE.exec(logStr))
//            this.texImportStatus = this.statusFailed;
//        }
//        else
//          dump("Couldn't read logfile [" + logUrl + "], from nsILocalFile [" + this.texImportLogFile.path + "]!\n");
//      }
//    }
//    if (this.importStatus == this.statusRunning)
//    {
//      if (this.importLogFile && this.importLogFile.exists())
//      {
//        var logStr;
//        try
//        {
//          logUrl = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService).newFileURI(this.importLogFile);  
//          logStr = getFileAsString(logUrl.spec);
//        } catch(ex) {dump("Exception in msiGraphicsConversionDlg.js, checkLogFileStatus: " + ex + "\n");}
//        if (logStr)
//        {
//          if (errorRE.exec(logStr))
//            this.importStatus = this.statusFailed;
//        }
//        else
//          dump("Couldn't read logfile " + logUrl + "], from nsILocalFile [" + this.importLogFile.path + "]!\n");
//      }
//    }
//  },

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
    if (this.mImportHandler)
    {
      this.mImportHandler.checkStatus();
    }
    if (this.mTexHandler)
    {
      this.mTexHandler.checkStatus();
    }
//    var loggingStrs = ["none", "running", "success", "failed"];
//    dump("In msiGraphicsConversionDlg.checkLoadingComplete; importStatus is [" + loggingStrs[this.importStatus] + "] and texImportStatus is [" + loggingStrs[this.texImportStatus] + "].\n");
    
    return false;
  },

  isLoading : function()
  {
    return ( (this.mImportHandler && this.mImportHandler.isLoading()) 
             || (this.mTexHandler && this.mTexHandler.isLoading()) );
  },

  disableDialogControls : function()
  {
    var stopButton = this.ourDocument.getElementById("stopImportButton");
    var texStopButton = this.ourDocument.getElementById("stopImportTeXButton");
    if (!this.mImportHandler || ! this.mImportHandler.isLoading())
      stopButton.disabled = true;
    if (!this.mTexHandler || ! this.mTexHandler.isLoading())
      texStopButton.disabled = true;
  }

};

//function disableControls(importRunning, texImportRunning)
//{
//  if (!importRunning)
//    gDialog.importControls[1].disabled = true;
//  if (!texImportRunning)
//    gDialog.texImportControls[1].disabled = true;
//}

function forceEndLoad(control)
{
  if (control.id == "stopImportTeXButton")
  {
    dlgTimerCallbackObject.forceEnd("tex");
  }
  else
  {
    dlgTimerCallbackObject.forceEnd("import");
  }
  dlgTimerCallbackObject.checkLoadingComplete();
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
//  dlgTimerCallback = new graphicsTimerSimpleCallback(millisecondsTimeLimit, terminateCallbackObject);

  dlgTimerCallbackObject.init(importDataIn, window, document);
  var timerHandler, importTimer;
  if (importDataIn.mImportProcess)
  {
    timerHandler = new graphicsTimerHandler(0, dlgTimerCallbackObject);
    importTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
    timerHandler.startLoading(importDataIn.mSourceFile, importDataIn.mImportTargFile, importDataIn.mImportProcess);
    dlgTimerCallbackObject.mImportHandler = timerHandler;
    importTimer.initWithCallback( timerHandler, 200, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
  }
  if (importDataIn.mTexImportProcess)
  {
    timerHandler = new graphicsTimerHandler(0, dlgTimerCallbackObject);
    importTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
    timerHandler.startLoading(importDataIn.mSourceFile, importDataIn.mTexImportTargFile, importDataIn.mTexImportProcess);
    dlgTimerCallbackObject.mTexHandler = timerHandler;
    importTimer.initWithCallback( timerHandler, 200, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
  }

//  dlgTimerCallback.mTimer = theTimer;
//  theTimer.initWithCallback( dlgTimerCallback, 500, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
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
//  forceEndLoad(gDialog.importControls[1]);
  dlgTimerCallbackObject.forceEnd("import");
//  if (importData.mTexImportProcess)
//  forceEndLoad(gDialog.texImportControls[1]);
  dlgTimerCallbackObject.forceEnd("tex");
//  if (theTimer)
//    theTimer.cancel();
//  theTimer = null;
  dlgTimerCallbackObject.stopTimer();
//  Components.utils.reportError("in onCancel\n");
  dlgTimerCallbackObject.copyDataOut(importDataIn);
  SaveWindowLocation();
  top.document.commandDispatcher.focusedWindow.focus();  
  window.close();
//  dump("In msiGraphicsConversionDlg.doEndDialog, after window.close call\n");
  return true;
}