var EXPORTED_SYMBOLS = ["graphicsConverter", "graphicsTimerHandler", "graphicsMultipleTimerHandler" ];

Components.utils.import("resource://app/modules/os.jsm");

var graphicsConverter =
{
  getGraphicsFilterDirPath : function(filterName)
  {
    if (!this.imageMagickDir || !this.uniconvertorDir || !this.wmf2epsDir)
    {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
      var utilsDir = dsprops.get("CurProcD", Components.interfaces.nsIFile);
//      utilsDir = utilsDir.parent;
      utilsDir.append("utilities");
      this.uniconvertorDir = this.imageMagickDir = utilsDir;
      this.wmf2epsDir = utilsDir.clone();
      this.wmf2epsDir.append("wmf2eps");
      if (!this.wmf2epsDir.exists())
        this.wmf2epsDir = null;
//      dump("Path to UniConvertor and ImageMagick is " + utilsDir.path + "\n");
    }
    switch(filterName)
    {
      case "ImageMagick":
        return this.uniconvertorDir.path;
      break;
      case "UniConvertor":
        return this.imageMagickDir.path;
      break;
      case "wmf2eps":
        if (this.wmf2epsDir)
        {
          return this.wmf2epsDir.path;
          break;
        }
      default:
        return "";
      break;
    }
  },

  formParamArray : function(graphicsInFile, graphicsOutDir, mode, theOS)
  {
    var params = [];
    var nameAndExtension = this.splitExtension(graphicsInFile.path);
    params.push(nameAndExtension.name);

    //Note: Windows and Unix et al are treated differently here - the Unix shell can readily handle converting case of the
    //  parameter (for purpose of matching expected ones), while on the other hand it may need the original case
    //  preserved in order to find the source file. Windows is the opposite on both counts, so the conversion is done here.
    if (theOS == "win")
      params.push(nameAndExtension.ext.toLowerCase());
    else
      params.push(nameAndExtension.ext);
        
    var graphicsOutFile = graphicsOutDir.clone();
    var leafName = graphicsInFile.leafName;
    nameAndExtension = this.splitExtension(leafName);
    graphicsOutFile.append(nameAndExtension.name);
    params.push(graphicsOutFile.path);
    params.push(mode);
    params.push(this.getGraphicsFilterDirPath("ImageMagick"));
    params.push(this.getGraphicsFilterDirPath("UniConvertor"));
    params.push(this.getGraphicsFilterDirPath("wmf2eps"));
    return params;
  },

  getTargetFileExtension : function(graphicsInFile, graphicsOutDir, mode, aWindow)
  {
    var os = getOS(aWindow);
    var filterCommandFile = this.getBatchFile(os);
    var infoMode = (mode == "tex" ? "outtex" : "out");
    var paramArray = this.formParamArray(graphicsInFile, graphicsOutDir, infoMode, os);
    var outInfoFile = graphicsOutDir.clone();
//    var outInfoFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
//    outInfoFile.initWithPath("C:\\temp");
    outInfoFile.append(graphicsInFile.leafName + "info.txt");
    var infoProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
//    paramArray.push(">" + outInfoFile.path);
    paramArray.push(outInfoFile.path);
//    paramArray.push(">C:\\temp\\logGfxCommands.txt");  //for debugging only!!
    infoProcess.init(filterCommandFile);
//    dump("In getTargetFileExtension, running infoProcess in mode " + mode + " for file " + graphicsInFile.leafName + ".\n");
    infoProcess.run(true, paramArray, paramArray.length);

//    dump("In getTargetFileExtension mode " + mode + " trying to get file contents for file [" + outInfoFile.path + "].\n");
    var extension = this.getFileAsString(outInfoFile);
    extension = extension.replace(/^\s*/,"");
    extension = extension.replace(/\s*$/,"");
//    dump("Got extension back from file " + outInfoFile.path + "; it's " + extension + ".\n");
    return extension;
  },

//  filterSourceFile - an nsIFile; the command file generated for this import
//  graphicsInFile - an nsIFile
//  graphicsOutFile - an nsILocalFile
//  mode - a string, either "tex" or "import" (defaults to "import")
//  aWindow = a reference window (typically the ambient variable "window"), mostly to pass to the getOS() function
//  callbackObject = object to be used for timer callbacks; must implement the usual "notify(timer)" function, and
//                   additionally (though it needn't do anything) a startLoading function - 
//                       startLoading(graphicsInFile, graphicsOutFile, theProcess, mode, theTimer)
//                   which will be called to allow the object to store the source and target files, process, mode, and timer
//    NOTE: If callbackObject is null, the conversion will be run as a synchronous process, and will thus block the calling thread.
//  Returns nsIFile representing graphics file being created
  doGraphicsImport : function(graphicsInFile, graphicsOutDir, mode, aWindow, callbackObject)
  {
    var bRunSynchronously = false;
    if (!callbackObject)
      bRunSynchronously = true;
    var os = getOS(aWindow);
    var extension = this.getTargetFileExtension(graphicsInFile, graphicsOutDir, mode, aWindow);

    var filterCommandFile = this.getBatchFile(os);
    var paramArray = this.formParamArray(graphicsInFile, graphicsOutDir, mode, os);
    var graphicsOutFile = graphicsOutDir.clone();
    var nameAndExtension = this.splitExtension(graphicsInFile.leafName);
    graphicsOutFile.append(nameAndExtension.name + "." + extension);
//    dump("In doGraphicsImport, initializing import process for file [" + graphicsInFile.path + "] in mode " + mode + " .\n");
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(filterCommandFile);
  //  var outLogFile = importTimerHandler.getImportLogFile(mode);
  //  theProcess.run(false, [">" + outLogFile + " 2>&1"], 1);
//    dump("In doGraphicsImport, running import process in mode " + mode + " for file " + graphicsInFile.leafName + ".\n");
    theProcess.run(bRunSynchronously, paramArray, paramArray.length);
    if (!bRunSynchronously)
    {
      var theTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
      callbackObject.startLoading(graphicsInFile, graphicsOutFile, theProcess, mode, theTimer);
//      dump("In doGraphicsImport, starting timer for file " + graphicsInFile.leafName + ".\n");
      theTimer.initWithCallback( callbackObject, 200, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
    }
//    dump("Returning from doGraphicsImport with return [" + graphicsOutFile.path + "].\n");
    return graphicsOutFile;
  },

  //Calling this function with an array of objects of the form {mFile : nsIFile, mOutDirectory : nsIFile, mMode : modeString} to convert will 
  //  return a graphicsMultipleTimerHandler (see bottom of file). This handler will record successes and failures for
  //  each conversion process; after mSecTimeLimit number of milliseconds it will stop any unfinished processes.
  //  It can initially be queried to get the target files by calling getTargetFile(sourceFile, mode).
  //  You should then manage it somehow - probably with a timer of its own rather than a simple loop? It can be asked to
  //  stopAll(), or queried as to the status of loading. In particular, calling .isFinished() on it will let you know
  //  whether there are pending conversions.
  doMultipleGraphicsImport : function(graphicsInFileArray, aWindow, mSecTimeLimit)
  {
    if (!mSecTimeLimit)
      mSecTimeLimit = 300000;  //5 minutes?
    var multiCallbackHandler = new graphicsMultipleTimerHandler();
    var theTimerHandler;
    var theImportTimer;
    for (var ii = 0; ii < graphicsInFileArray.length; ++ii)
    {
      theTimerHandler = new graphicsTimerHandler(mSecTimeLimit, multiCallbackHandler, ii+1);
      multiCallbackHandler.addTimerHandler(theTimerHandler);
      this.doGraphicsImport(graphicsInFileArray[ii].mFile, graphicsInFileArray[ii].mOutDirectory, graphicsInFileArray[ii].mMode, aWindow, theTimerHandler);
    }
    return multiCallbackHandler;
  },

  getFileAsString : function( aFile )  //Takes an nsIFile
  {
    var fileAsUrl = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService).newFileURI(aFile);
    var req = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance();
    req.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    if (!req)
    {
//      dump("In graphicsConverter.getFileAsString, req is null!\n");
      return null;
    }
//    var req = new XMLHttpRequest();
    req.overrideMimeType("text/plain");
    req.open('GET', fileAsUrl.spec, false);   
    req.send(null);
//    dump("graphicsConverter.getFileAsString sent XMLHTTPRequest for file [" + aFile.path + "]; status is " + req.status + " and status text is [" + req.statusText + "].\n");
    if (req.status === 0)
      return req.responseText;
    else
      return null;
  },

//Returns an nsIFile
  getBatchFile : function(theOS)
  {
    var extension = "";
    if (theOS == "win") extension = "cmd";
    else extension = "bash";
    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
    var templateFile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
    templateFile.append("rungfxconv." + extension);
    return templateFile;
  },

  getConvertibleFileTypes : function(aWindow)
  {
    var os = getOS(aWindow);
    var cmdFile = this.getBatchFile(os);
    var cmdContents = this.getFileAsString(cmdFile);
    var theLines = cmdContents.split(/\n/);
    var fileTypes = [];
    var inFileTypeList = false;

    var remarkStr = "^\\s*REM\\s+";
    if (os != "win")
      remarkStr = "^\\s*#\\s*";
    var remarkRE = new RegExp(remarkStr);
    var startFlagRE = new RegExp(remarkStr + "BEGIN\\s+EXTENSION\\s+LIST");
    var endFlagRE = new RegExp(remarkStr + "END\\s+EXTENSION\\s+LIST");
    var theLine;

    for (var ix = 0; ix < theLines.length; ++ix)
    {
      if (!inFileTypeList)
      {
       if (startFlagRE.test(theLines[ix]))
         inFileTypeList = true;
      }
      else if (endFlagRE.test(theLines[ix]))
        inFileTypeList = false;
      else if (remarkRE.test(theLines[ix]))
      {
        theLine = theLines[ix].replace(remarkRE, "");
        theLine = this.trimStr(theLine);
        fileTypes = fileTypes.concat( theLine.split(/\s+/) );
      }
    }
    return fileTypes;
  },

  trimStr : function(aStr)
  {
    aStr = aStr.replace(/^\s+/,"");
    return aStr.replace(/\s+$/,"");
  },

  splitExtension : function(fileName)
  {
    var retName = fileName;
    var retExt = "";
    var splitAt = fileName.lastIndexOf(".");
    if (splitAt >= 0)
    {
      retName = fileName.substr(0, splitAt);
      retExt = fileName.substr(splitAt + 1);
    }
    return {name : retName, ext : retExt};
  }
};

var graphicsTimerCallbackBase =
{
  statusNone : 0,
  statusRunning : 1,
  statusSuccess : 2,
  statusFailed : 3,
  maxGraphicsLoadTime : 1200000,  //20 minutes - longest we'll allow waiting for a conversion?

  statusStrings : ["none", "running", "success", "failed"],
    
  timer : null,
  sourceFile : null,
  importTargFile : null,
  importProcess : null,
  importStatus : 0,
  timerCount : 0,
  timerInterval : 200,
  timerCountLimit : 25,
  endCallback : null,
  errorString : "",
  userData : null,
  endCallbackObject : null,
  timerStopped : false,
  mode : "import",

  notify : function(aTimer)
  { 
    if (aTimer)
      this.timer = aTimer;
    this.checkStatus();

    if (this.importStatus == this.statusRunning)
    {
      ++this.timerCount;
//      var exitVal = "[" + (this.importProcess ? this.importProcess.exitValue : "none");
//      if (!(this.timerCount & 1))
//        dump("In graphics loading timerCallback for file [" + this.importTargFile.path + ", timer count is [" + this.timerCount + "], status is " + this.statusStrings[this.importStatus] + ", timer limit is " + this.timerCountLimit + ".\n");
      if (this.timerCount < this.timerCountLimit)  //keep waiting
        return;
      //Otherwise, we fall through and execute the stop() method below

//      if (this.ImportProcess && !this.ImportProcess.isRunning)
//      {
//        stopImportTimer("import");
//      }
//      this.stop();
      
//      return;
    }
     
    //Otherwise, we're finished
    this.stop();
  },

  onEndTimer : function()
  {
    if (this.endCallbackObject)
    {
      this.endCallbackObject.terminalCallback(this.sourceFile, this.importTargFile, this.importStatus, this.errorString, this.userData);
    }
  },

  reset : function()
  {
    this.cleanUp();
    this.timerCount = 0;
    this.sourceFile = null;
    this.importProcess = null;
    this.importStatus = this.statusNone;
  },

  cleanUp : function()
  {
    if (this.importTargFile)
    {
      var sentFile = this.getFailureSentinelFile();
      if (sentFile.exists())
        sentFile.remove(false);
      this.importTargFile = null;
      var logFile = this.getImportLogFile();
      if (logFile.exists())
        logFile.remove(false);
    }
  },

  startLoading : function(srcFile, targFile, process, mode, aTimer)
  {
    this.timerCount = 0;
    this.timer = aTimer;
    this.sourceFile = srcFile;
    this.importTargFile = targFile;
    this.importProcess = process;
    this.importStatus = this.statusRunning;
    this.mode = mode;
//    dump("In timerhandler for target file " + targFile.path + "; failure sentinel is " + this.getFailureSentinelFile().path + ".\n");
  },

  stop : function()
  {
//    if (this.importProcess)
//      this.importProcess.kill();
    if (this.timerStopped)
      return;
    if (this.timer)
      this.timer.cancel();
    this.timerStopped = true;
    this.checkFinalStatus();
    this.onEndTimer();
  },

  checkStatus : function()
  {
    if (this.importStatus == this.statusRunning)
    {
      if (this.importTargFile.exists())
        this.importStatus = this.statusSuccess;
      else if (!this.importProcess || this.processFailed())
        this.importStatus = this.statusFailed;
    }
  },

  didNotSucceed : function()
  {
    return (this.importStatus != this.statusSuccess);
  },

  checkLogFileStatus : function()
  {
    var errorRE = /(error)|(fail)/i;
    if (this.importStatus == this.statusRunning)
    {
      if (this.getImportLogFile() && this.getImportLogFile().exists())
      {
        var logStr;
//        dump("In timerHandler.checkLogFileStatus, found file [" + this.getImportLogFile().path + "] exists.\n");
        try
        {
          logStr = this.getImportLogInfo();
        } catch(ex) 
        {
//          dump("In graphicsTimerHandler.checkLogFileStatus, exception: " + ex + "\n");
        }
        if (logStr)
        {
          var bItFailed = false;
          if (errorRE.exec(logStr))
          {
            this.importStatus = this.statusFailed;
            this.errorString = logStr;
            bItFailed = true;
          }
//          dump("In graphicsTimerHandler.checkLogFileStatus, error string search returned " + (bItFailed ? "true.\n" : "false.\n"));
        }
//       else
//          dump("In graphicsTimerHandler.checkLogFileStatus, getImportLogInfo failed to return a string!\n");
      }
    }
  },

  checkFinalStatus : function()
  {
    this.checkStatus();
    if (this.importStatus == this.statusRunning)
      this.checkLogFileStatus();
  },

  isLoading : function()
  {
    this.checkStatus();
    return (this.importStatus == this.statusRunning);
  },

  getFailureSentinelFile : function()
  {
    var sentinelFile = this.importTargFile.clone();
    if (!sentinelFile || !sentinelFile.path)
      return null;
    var leaf = sentinelFile.leafName;
    leaf += ".txt";
    sentinelFile = sentinelFile.parent;
    sentinelFile.append(leaf);
    return sentinelFile;
  },

  getImportLogFile : function()
  {
    var logFile = this.importTargFile.clone();
    if (!logFile || !logFile.path)
      return null;
    var leaf = logFile.leafName;
    leaf += ".log";
    logFile = logFile.parent;
    logFile.append(leaf);
    return logFile;
  },

  processFailed: function()
  {
    if (this.importStatus == this.statusRunning)
    {
      var sentFile = this.getFailureSentinelFile();
      var sentPath = sentFile ? sentFile.path : "";
//      dump("In graphicsTimerHandler.processFailed, sentinel file [" + sentPath + (sentFile.exists() ? "] exists.\n" : "] doesn't exist.\n"));
      if (sentFile && sentFile.exists())
        this.importStatus = this.statusFailed;
    }
//    dump("In graphicsTimerHandler.processFailed, status is " + this.statusStrings[this.importStatus] + ".\n");
    return (this.importStatus == this.statusFailed);
  },

  getImportLogInfo : function()
  {
    var logFile = this.getImportLogFile();
//    dump("In getImportLogInfo, trying to get contents of log file [" + logFile.path + "]; our target file is [" + this.importTargFile.path + "].\n");
//    if (logFile.exists())
//      dump("  File claims to exist.\n");
//    else
//      dump("  File claims not to exist.\n");
    return graphicsConverter.getFileAsString(logFile);
  }

};

// This timer handler will handle tracking the success or failure of the conversion up to the time limit passed in.
//   Either when the conversion succeeds (or definitively fails) or at the end of the time limit, a function the 
//   terminateCallback object should contain will be called. It will be passed the parameters:
//     endCallback(nsIFile sourceGraphicsFile, nsIFile targetGraphicsFile, importStatus, string errorMessage, data)
//   The importStatus parameter is one of the constants in graphicsTimerCallbackBase, so one of:
//     statusNone(=0), statusRunning(=1), statusSuccess(=2), statusFailed(3).
//   The "data" parameter is just user data to be passed back in the terminalCallback function call.
//   Presumably once we're able to actually kill a process (next generation of Mozilla code), the statusRunning value
//     wouldn't be returned(?).
// Sample usage might be:
//     var callbackHandler = new graphicsTimerHandler(5000, myCallback);
//     graphicsConverter.doGraphicsImport(graphicsInFile, graphicsOutDir, "import", callbackHandler);

function graphicsTimerHandler(millisecondsTimeLimit, terminateCallbackObject, data)
{
//  this.timercopy = null;
//  this.sourceFile = null;
//  this.importTargFile = null;
//  this.importProcess = null;
//  this.importStatus = graphicsTimerCallbackBase.statusNone;
//  this.timerCount = 0;
//  this.timerInterval = 200;
  if (millisecondsTimeLimit == 0)
    millisecondsTimeLimit = this.maxGraphicsLoadTime;
  this.timerCountLimit = (millisecondsTimeLimit/this.timerInterval);
  this.endCallbackObject = terminateCallbackObject;
  if (data)
    this.userData = data;

}

graphicsTimerHandler.prototype = graphicsTimerCallbackBase;

var graphicsMultipleTimerBase = 
{
  mTimerHandlers : [],

  terminalCallback : function(sourceFile, importTargFile, importStatus, errorString, userData)
  {
    var ix = 0;
    if (userData)
      ix = userData;
    else
      ix = 1 + this.findByTargFile(importTargFile);
    if (ix > 0)
      this.mTimerHandlers[ix-1].mbFinished = true;
  },

  isFinished : function()
  {
    var bDone = true;
    for (var ix = 0; bDone && (ix < this.mTimerHandlers.length); ++ix)
    {
      if (!this.mTimerHandlers[ix].mbFinished)
        bDone = false;
    }
//    dump("In graphicsMultipleTimerHandler.isFinished; returning " + (bDone ? "true\n" : "false\n"));
    return bDone;
  },

  addTimerHandler : function(aTimerHandler)
  {
    this.mTimerHandlers.push({mHandler : aTimerHandler, mbFinished : false});
    return this.mTimerHandlers.length;  //returning the index of the new timer
  },

  addNewTimerHandler : function(mSecTimeLimit)
  {
    if (!mSecTimeLimit)
      mSecTimeLimit = 300000;  //5 minutes outside limit?
    var aTimerHandler = new graphicsTimerHandler(mSecTimeLimit, this, this.mTimerHandlers.length + 1);
    this.addTimerHandler(aTimerHandler);
    return aTimerHandler;
  },

  stopAll : function()
  {
    for (var ix = 0; ix < this.mTimerHandlers.length; ++ix)
      this.mTimerHandlers[ix].stop();
  },

  getStatus : function(aTargFile)
  {
    var ix = this.findByTargFile(aTargFile);
    if (ix > 0)
      return this.mTimerHandlers[ix].importStatus;
    return graphicsTimerCallbackBase.statusNone;
  },

  findByTargFile : function(targFile)
  {
    var index = -1;
    for (var ix = 0; (index < 0) && (ix < this.mTimerHandlers.length); ++ix)
    {
      if (this.mTimerHandlers[ix].mHandler.importTargFile.path == targFile.path)
        index = ix;
    }
    return index;
  },

  getTargetFile : function(srcFile, mode)
  {
    var targFile = null;
    for (var ix = 0; (!targFile) && (ix <this.mTimerHandlers.length); ++ix)
    {
      if ( (this.mTimerHandlers[ix].mHandler.sourceFile.path == srcFile.path) && (this.mTimerHandlers[ix].mHandler.mode == mode) )
        targFile = this.mTimerHandlers[ix].mHandler.importTargFile;
    }
    return targFile;
  }
};

function graphicsMultipleTimerHandler()
{
  this.mTimerHandlers = [];
}

graphicsMultipleTimerHandler.prototype = graphicsMultipleTimerBase;
