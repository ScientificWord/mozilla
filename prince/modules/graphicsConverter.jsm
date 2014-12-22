var EXPORTED_SYMBOLS = ["graphicsConverter", "graphicsTimerHandler", "graphicsMultipleTimerHandler"];

Components.utils.import("resource://app/modules/os.jsm");
Components.utils.import("resource://app/modules/pathutils.jsm");
Components.utils.import("resource://app/modules/unitHandler.jsm");

const nsIFile = Components.interfaces.nsIFile;
const nsIINIParser = Components.interfaces.nsIINIParser;
const nsIINIParserFactory = Components.interfaces.nsIINIParserFactory;
const nsILocalFile = Components.interfaces.nsILocalFile;
const nsISupports = Components.interfaces.nsISupports;

function createINIParser(aFile) {
    return Components.manager.
      getClassObjectByContractID("@mozilla.org/xpcom/ini-parser-factory;1",
      nsIINIParserFactory).createINIParser(aFile);
}

var graphicsConverter = {
  nativeGraphicTypes: ["png", "gif", "jpg", "jpeg", "pdf", "svg", "xvc", "xvz"],
  typesetGraphicTypes: ["eps", "pdf", "png", "jpg", "svg"],
  baseDir: null,
  converterDir: null,
  iniParser: null,
  codeDir: null,
  OS: "",
  handler: new UnitHandler(),


  init: function(aWindow, baseDirIn) {
    this.OS = getOS(aWindow);
    this.baseDir = baseDirIn;
    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
    this.codeDir = dsprops.get("CurProcD", Components.interfaces.nsIFile);
    var iniFile = this.codeDir.clone();
    this.converterDir = this.codeDir.clone();
    iniFile.append("graphicsConversions.ini");
    this.iniParser = createINIParser(iniFile);
    this.converterDir.append("utilities");
    this.handler.initCurrentUnit('bp');
    // This next line will probably need to be removed when the utilities directory is reorganized.
    // this.converterDir.append("bin");
  },

  copyAndConvert: function(graphicsFile,  copyToGraphics, width, height) {
    var baseName;
    var extension;
    var leaf = graphicsFile.leafName;
    var chunks = leaf.split('.');
    var command;
    var destDir;
    var destFile;
    var returnPath;
    var copiedFile;
    extension = chunks[chunks.length - 1].toLowerCase();
    chunks.length = chunks.length - 1;
    baseName = chunks.join('.');
    try {
      command = this.iniParser.getString("Converters", extension);
    }
    catch(e) {
      command = null;
    }
    if (command != null) {
      try {
        if (copyToGraphics) {
          this.assureSubdir("graphics");
          destDir = this.baseDir.clone();
          destDir.append("graphics");
          destFile = destDir.clone();
          destFile.append(leaf);
          returnPath = "graphics/" + leaf;
          if (graphicsFile.path !== destFile.path) {
            if (destFile.exists()) destFile.remove(false);
            graphicsFile.copyTo(destDir, leaf);
          }
        }
        copiedFile = this.baseDir.clone();
        copiedFile.append("graphics");
        copiedFile.append(leaf);

        if (command != "") {
          returnPath = this.execCommands(copiedFile, command, width, height);
        }
      }
      catch(e) {
        msidump(e.message);
      }
    }
    return returnPath;
  },

  assureSubdir: function(leafname) {
    var theDir = this.baseDir.clone();
    theDir.append(leafname);
    if (!theDir.exists()) {
      theDir.create(1, 0777);
    }
  },

  execCommands: function(graphicsFile, command, width, height) {
    var commandlist = command.split(';');
    var progname;
    var commandparts;
    var toolsDir;
    var paramOffset = 0;
    toolsDir = this.converterDir.clone();
    var regex0 = /\\/;  // look for slash or backslash
    var regex1 = /\//;
    var dollar1;
    var dollar2; // these correspond to $1 and $2 in graphicsConversions.ini
    var theProcess;
    var utilityFile;
    var extension;
    var paramArray;
    var paramArray1 = [];
    var paramArray2 = [];
    var param;
    var bRunSynchronously = true;
    var resolutionParameter = null;
    var returnPath; // the path of the file to display, relative to baseDir
    var pathParts = graphicsFile.path.split('.');
    if (regex0.test(pathParts[pathParts.length - 1]) || regex1.test(pathParts[pathParts.length - 1])) { // there is a '/' past the last '.', so there is no useful extension -- shouldn't happen
      return; 
    }
    if (pathParts.length > 1) 
    {
      extension = pathParts[pathParts.length-1];
      pathParts.length = pathParts.length - 1; //delete the extension
    }
    try {
      dollar1 = pathParts.join('.');
      dollar2 = graphicsFile.leafName;
      var leafParts = dollar2.split('.');
      if (leafParts.length > 1) leafParts.length = leafParts.length - 1;
      dollar2 = leafParts.join('.');
      var i;
      var j;
      var ithCommand;
      var regex2=/\$2/;
      for (i = 0; i < commandlist.length; i++) {
        if (i === 0) paramArray=paramArray1;
        else paramArray=paramArray2;
        ithCommand = commandlist[i];
        ithCommand = ithCommand.replace(/^\s*/,'');  // aka 'trim'
        ithCommand = ithCommand.replace(/\s*$/,'');
        commandparts = commandlist[i].split(',');
        progname = commandparts[0];
        if (this.OS === "win") progname += ".exe";
        theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
        // use progname unchanged if it contains a '/', otherwise assume it is in the utilties dir.
        if (commandparts[0]=="sam2p") {
          utilityFile = toolsDir.clone();
          utilityFile = utilityFile.parent;
          utilityFile.append("shell"+(this.OS==="win"?".cmd":".bash"));
        }
        else if (regex0.test(progname) || regex1.test(progname)) {
          utilityFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
          utilityFile.initWithPath(progname);
        }
        else {
          utilityFile = toolsDir.clone();
          utilityFile.append(progname);
        }
        theProcess.init(utilityFile);
        // build parameters
        // bRunSynchronously = (i+1) < commandlist.length;  // run all commands but the last synchronously
        if (width && height) {
          resolutionParameter = this.setResolutionParameters(graphicsFile, width, height );
        }
        for (j = 0; j < commandparts.length; j++) {
          param = commandparts[j];
          if (j==0 && param != "sam2p") continue;
          param = param.replace("$1", dollar1);
          if (regex2.test(param)) {
            param = param.replace("$2", dollar2);
            param = param.replace(/^\s*/,'');  // aka 'trim'
            param = param.replace(/\s*$/,'');

            returnPath = param.replace(/\"/g, '');
            if (/tcache/.test(param)) this.assureSubdir('tcache');
            if (/gcache/.test(param)) this.assureSubdir('gcache');
            if (/graphics/.test(param)) this.assureSubdir('graphics');
            if (this.OS === "win") {
              param = this.baseDir.path + '\\' + param;  
            } 
            else {
              param = this.baseDir.path + '/' + param;                
            }
            param = param.replace(/\"/g,'');
          }
          param = param.replace(/\"/g,'');
          // if (param.length > 0) {
            paramArray.push(param);
          // }
        }
        if (resolutionParameter) {
          paramArray.unshift(resolutionParameter);
        }
        theProcess.run(bRunSynchronously, paramArray, paramArray.length);
      }
    }
    catch(e) {
      AlertWithTitle("Dump", e.message, null);
    }
    return returnPath.replace("\\","/", 'g');
  },

  // A function to call when an existing graphics object changes dimensions, requiring
  // a new preview file optimized for those dimensions.
  setResolutionParameters: function(graphicsFile, newWidth, newHeight) {
    // should we be saving the old dimensions in the node attributes, to skip this 
    // function when possible?
    // newWidth and newHeight are in pixels?
    var originalUrl = graphicsFile.path;
    var pathParts = originalUrl.split('.');
    var extension = pathParts[pathParts.length-1];
    if (!extension || extension.length === 0) extension = pathParts[pathParts.length-2];
    if (!extension || extension.length === 0) return;
    var origDimension = null;
    var resolutionParameter;
    var intermediate;
    var w, h; 
    var pixelsPerInch;

    switch (extension) {
      case "eps": origDimension = this.readSizeFromEPSFile(graphicsFile);
        break;
      case "pdf": origDimension = this.readSizeFromPDFFile(graphicsFile);
        break;
      default:
        return null;
        break;
    }
    if (origDimension && origDimension.width > 0) {
      // origDimension is in big points, 'bp'
      this.handler.setCurrentUnit('px');
      w = this.handler.getValueOf(origDimension.width, 'bp');
      h = this.handler.getValueOf(origDimension.height, 'bp');
      pixelsPerInch = this.handler.getValueOf(1, 'in');
      resolutionParameter = "-l:gs=-r"+Math.round((pixelsPerInch*newWidth)/w)+"x"+
        Math.round((pixelsPerInch*newHeight)/h);
      return resolutionParameter;
    }
    return null;
  },


  readSizeFromEPSFile: function(epsFile)
  {
    var theText = this.getFileAsString(epsFile);
    var dimensions = {width: 0, height: 0};
    var regex = /%%BoundingBox: (\d+) (\d+) (\d+) (\d+)/;
    var match = regex.exec(theText);
    dimensions.width = match[3] - match[1];
    dimensions.height = match[4] - match[2];
    return dimensions;

  },

  readSizeFromPDFFile: function(pdfFile)
  {
    var theText = this.getFileAsString(pdfFile);
    var dimensions = {width: 0, height: 0};

  //  var wdthRE = /(\/Width)\s+([0-9]+)/i;
  //  var htRE = /(\/Height\s+([0-9]+)/i;
    var BBRE = /\/BBox\s*\[([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s*\]/;
    var mediaBoxRE = /\/MediaBox\s*\[([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s*\]/;

    function checkBoundingBox( objString )
    {
      var res = null;
      var bbArray = BBRE.exec(objString);
      if (bbArray && bbArray[1] && bbArray[2])
      {
        if (bbArray[3] && bbArray[4])
        {
          res = {wdth : Math.round(9.6 * (Number(bbArray[3]) - Number(bbArray[1]))),
                 ht : Math.round(9.6 * (Number(bbArray[4]) - Number(bbArray[2]))) };   //9.6 here is 96 pts-per-inch/10 since these are given in tenths of an inch
        }
        else  //this shouldn't happen, though
        {
          res = {wdth : Math.round(9.6 * Number(bbArray[1])),
                 ht : Math.round(9.6 * Number(bbArray[2])) };   //9.6 here is 96 pts-per-inch/10 since these are given in tenths of an inch
        }
      }
      return res;
    }

    function checkMediaBox( objString )
    {
      var res = null;
      var dimsArray = mediaBoxRE.exec(objString);
      if (dimsArray && dimsArray[1] && dimsArray[2])
      {
        if (dimsArray[3] && dimsArray[4])
        {
          res = { wdth : Math.round(Number(dimsArray[3]) - Number(dimsArray[1])),
                  ht : Math.round(Number(dimsArray[4]) - Number(dimsArray[2])) };
        }
        else  //this shouldn't happen, though
        {
          res = { wdth : Math.round(Number(dimsArray[1])),
                  ht : Math.round(Number(dimsArray[2])) };
        }
      }
      return res;
    }

    var htWdth = null;
    var pdfStart = theText.indexOf("%PDF");
    if (pdfStart < 0)
      return;
    var xObj = theText.indexOf("/XObject", pdfStart);
    var objEnd = pdfStart;
    while (!htWdth && (xObj > 0))  //look for /XObject spec first
    {
      objEnd = theText.indexOf("endobj", xObj);
      htWdth = checkBoundingBox( theText.substring(xObj, objEnd) );
      if (!htWdth)
        xObj = theText.indexOf("/XObject", objEnd);
    }
    if (!htWdth)
    {
      var pageDesc = theText.indexOf("/Page", pdfStart);
      objEnd = pdfStart;
      while (!htWdth && (pageDesc > 0))  //look for /XObject spec first
      {
        objEnd = theText.indexOf("endobj", pageDesc);
        htWdth = checkMediaBox( theText.substring(pageDesc, objEnd) );
        if (!htWdth)
          pageDesc = theText.indexOf("/Page", objEnd);
      }
    }

    if (htWdth)
    {
      var bReset = false;
      // htWdth fields are in Adobe Big Points
      dimensions.width  = htWdth.wdth; 
      dimensions.height = htWdth.ht;
      dimensions.unit = 'bp';
      // bReset = true;
      // if (frameTabDlg.actual.selected || bReset)
      //   setActualOrDefaultSize();

      // SetSizeWidgets( Math.round(this.handler.getValueAs(frameTabDlg.widthInput.value,"px")), 
      //                 Math.round(this.handler.getValueAs(frameTabDlg.heightInput.value,"px")) );
    }
    return dimensions;
  },

  // getFileAsString: function(fileUrl) {
  //   var req = new XMLHttpRequest();
  //   req.overrideMimeType("text/plain");
  //   req.open('GET', fileUrl, false);
  //   req.send(null);
  //   if (req.status === 0)
  //     return req.responseText;
  //   else
  //     return null;
  // },


  getGraphicsFilterDirPath: function(filterName) {
    if (!this.imageMagickDir || !this.uniconvertorDir || !this.wmf2epsDir) {
      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
      var utilsDir = dsprops.get("CurProcD", Components.interfaces.nsIFile);
      //      utilsDir = utilsDir.parent;
      utilsDir.append("utilities");
      this.uniconvertorDir = this.imageMagickDir = this.wmf2epsDir = utilsDir;
      //this.wmf2epsDir = utilsDir.clone();
      //this.wmf2epsDir.append("wmf2epsc");
      //if (!this.wmf2epsDir.exists())
      //  this.wmf2epsDir = null;
      //      dump("Path to UniConvertor and ImageMagick is " + utilsDir.path + "\n");
    }
    switch (filterName) {
      case "ImageMagick":
        return this.uniconvertorDir.path;
        break;
      case "UniConvertor":
        return this.imageMagickDir.path;
        break;
      case "wmf2eps":
        if (this.wmf2epsDir) {
          return this.wmf2epsDir.path;
        }
        break;
      default:
        return "";
        break;
    }
  },

  formParamArray: function(graphicsInFile, graphicsOutDir, mode, theOS) {
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

  getTargetFilesForImport: function(graphicsInFile, graphicsOutDir, mode, aWindow) {
    var theExtensions = this.getTargetFileExtensions(graphicsInFile, graphicsOutDir, mode, aWindow);
    var graphicsOutFileList = [];
    var nameAndExtension = this.splitExtension(graphicsInFile.leafName);
    for (var kk = 0; kk < theExtensions.length; ++kk) {
      var outFile = graphicsOutDir.clone();
      outFile.append(nameAndExtension.name + "." + theExtensions[kk]);
      graphicsOutFileList.push(outFile);
    }
    return graphicsOutFileList;
  },

  getTargetFileExtensions: function(graphicsInFile, graphicsOutDir, mode, aWindow) {
    var os = getOS(aWindow);
    var filterCommandFile = this.getBatchFile(os);
    var infoMode = (mode == "tex" ? "outtex" : "out");
    var paramArray = this.formParamArray(graphicsInFile, graphicsOutDir, infoMode, os);
    var outInfoFile = graphicsOutDir.clone();
    outInfoFile.append(graphicsInFile.leafName + "info.txt");
    var infoProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    //    paramArray.push(">" + outInfoFile.path);
    paramArray.push(outInfoFile.path);
    //    paramArray.push(">C:\\temp\\logGfxCommands.txt");  //for debugging only!!
    infoProcess.init(filterCommandFile);
    //    dump("In getTargetFileExtension, running infoProcess in mode " + mode + " for file " + graphicsInFile.leafName + ".\n");
    infoProcess.run(true, paramArray, paramArray.length);

    //    dump("In getTargetFileExtension mode " + mode + " trying to get file contents for file [" + outInfoFile.path + "].\n");
    var extensionLines = this.getFileAsString(outInfoFile);
    var extensions = extensionLines.split("\n");
    var extensionList = [];
    var extension;
    for (var jj = 0; jj < extensions.length; ++jj) {
      extension = extensions[jj].replace(/^\s*/, "");
      extension = extension.replace(/\s*$/, "");
      if (extension.length > 0)
        extensionList.push(extension);
    }
    //    dump("Got extension back from file " + outInfoFile.path + "; it's " + extension + ".\n");
    return extensionList;
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
  doGraphicsImport: function(graphicsInFile, graphicsOutDir, mode, aWindow, callbackObject, bReturnAllFiles) {
    //    var graphicsOutFileList = [];
    //    var os = getOS(aWindow);
    //    var extensionList = this.getTargetFileExtensions(graphicsInFile, graphicsOutDir, mode, aWindow);
    //    if (!extensionList || !extensionList.length)
    //    {
    //      return graphicsOutFileList;
    //    }
    //
    //    var nameAndExtension = this.splitExtension(graphicsInFile.leafName);
    //    var extension = extensionList[extensionList.length - 1];
    //    var graphicsOutFile = graphicsOutDir.clone();
    //    graphicsOutFile.append(nameAndExtension.name + "." + extension);

    var graphicsOutFileList = this.getTargetFilesForImport(graphicsInFile, graphicsOutDir, mode, aWindow);
    var graphicsOutFile = null;
    if (graphicsOutFileList.length > 0) {
      graphicsOutFile = graphicsOutFileList[graphicsOutFileList.length - 1];
      this.doImportGraphicsToTarget(graphicsInFile, graphicsOutFile, mode, aWindow, callbackObject);
    }

    //    dump("Returning from doGraphicsImport with return [" + graphicsOutFile.path + "].\n");
    if (bReturnAllFiles)
      return graphicsOutFileList;

    return graphicsOutFile;

    //    for (var kk = 0; kk < extensionList.length - 1; ++kk)
    //    {
    //      var outFile = graphicsOutDir.clone();
    //      outFile.append(nameAndExtension.name + "." + extensionList[kk]);
    //      graphicsOutFileList.push( outFile );
    //    }
    //    graphicsOutFileList.push(graphicsOutFile);
    //    return graphicsOutFileList;
  },

  //This function should only be called following a call to getTargetFilesForImport above!
  //It does no checking that the input file can be converted to the graphicsOutFile indicated.
  doImportGraphicsToTarget: function(graphicsInFile, graphicsOutFile, mode, aWindow, callbackObject) {
    var os = getOS(aWindow);
    var bRunSynchronously = false;
    if (!callbackObject)
      bRunSynchronously = true;
    var filterCommandFile = this.getBatchFile(os);
    var graphicsOutDir = graphicsOutFile.parent;
    var paramArray = this.formParamArray(graphicsInFile, graphicsOutDir, mode, os);
    //    dump("In doGraphicsImport, initializing import process for file [" + graphicsInFile.path + "] in mode " + mode + " .\n");
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(filterCommandFile);
    //  var outLogFile = importTimerHandler.getImportLogFile(mode);
    //  theProcess.run(false, [">" + outLogFile + " 2>&1"], 1);
    //    dump("In doGraphicsImport, running import process in mode " + mode + " for file " + graphicsInFile.leafName + ".\n");
    theProcess.run(bRunSynchronously, paramArray, paramArray.length);
    if (!bRunSynchronously) {
      var theTimer = Components.classes["@mozilla.org/timer;1"].createInstance(Components.interfaces.nsITimer);
      callbackObject.startLoading(graphicsInFile, graphicsOutFile, theProcess, mode, theTimer);
      //      dump("In doGraphicsImport, starting timer for file " + graphicsInFile.leafName + ".\n");
      theTimer.initWithCallback(callbackObject, 200, Components.interfaces.nsITimer.TYPE_REPEATING_SLACK);
    }
  },

  //Calling this function with an array of objects of the form {mFile : nsIFile, mOutDirectory : nsIFile, mMode : modeString} to convert will 
  //  return a graphicsMultipleTimerHandler (see bottom of file). This handler will record successes and failures for
  //  each conversion process; after mSecTimeLimit number of milliseconds it will stop any unfinished processes.
  //  It can initially be queried to get the target files by calling getTargetFile(sourceFile, mode).
  //  You should then manage it somehow - probably with a timer of its own rather than a simple loop? It can be asked to
  //  stopAll(), or queried as to the status of loading. In particular, calling .isFinished() on it will let you know
  //  whether there are pending conversions.
  doMultipleGraphicsImport: function(graphicsInFileArray, aWindow, mSecTimeLimit) {
    if (!mSecTimeLimit)
      mSecTimeLimit = 300000; //5 minutes?
    var multiCallbackHandler = new graphicsMultipleTimerHandler();
    var theTimerHandler;
    var theImportTimer;
    for (var ii = 0; ii < graphicsInFileArray.length; ++ii) {
      theTimerHandler = new graphicsTimerHandler(mSecTimeLimit, multiCallbackHandler, ii + 1);
      multiCallbackHandler.addTimerHandler(theTimerHandler);
      this.doGraphicsImport(graphicsInFileArray[ii].mFile, graphicsInFileArray[ii].mOutDirectory, graphicsInFileArray[ii].mMode, aWindow, theTimerHandler, false);
    }
    return multiCallbackHandler;
  },

  getFileAsString: function(aFile) //Takes an nsIFile
  {
    var fileAsUrl = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService).newFileURI(aFile);
    var req = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance();
    req.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    if (!req) {
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
  getBatchFile: function(theOS) {
    var extension = "";
    if (theOS == "win") extension = "cmd";
    else extension = "bash";
    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
    var templateFile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
    templateFile.append("rungfxconv." + extension);
    return templateFile;
  },

  getConvertibleFileTypes: function(aWindow) {
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

    for (var ix = 0; ix < theLines.length; ++ix) {
      if (!inFileTypeList) {
        if (startFlagRE.test(theLines[ix]))
          inFileTypeList = true;
      } else if (endFlagRE.test(theLines[ix]))
        inFileTypeList = false;
      else if (remarkRE.test(theLines[ix])) {
        theLine = theLines[ix].replace(remarkRE, "");
        theLine = this.trimStr(theLine);
        fileTypes = fileTypes.concat(theLine.split(/\s+/));
      }
    }
    return fileTypes;
  },

  trimStr: function(aStr) {
    aStr = aStr.replace(/^\s+/, "");
    return aStr.replace(/\s+$/, "");
  },

  splitExtension: function(fileName) {
    var retName = fileName;
    var retExt = "";
    var splitAt = fileName.lastIndexOf(".");
    if (splitAt >= 0) {
      retName = fileName.substr(0, splitAt);
      retExt = fileName.substr(splitAt + 1);
    }
    return {
      name: retName,
      ext: retExt
    };
  },

  isDisplayableGraphicFile: function(fileName) {
    var pathAndExt = this.splitExtension(fileName);
    return this.nativeGraphicTypes.indexOf(pathAndExt.ext);
  },

  //baseDir is an nsIFile
  //return an nsILocalFile
  resolveRelativeFilePath: function(baseDir, relPathStr) {
    var baseURI, resolvedURI, resolvedURIStr, filePath;
    var outFile = null;
    try {
      baseURI = msiFileURLFromFile(baseDir);
      resolvedURIStr = baseURI.resolve(relPathStr);
      resolvedURI = msiURIFromString(resolvedURIStr);
      filePath = msiPathFromFileURL(resolvedURI);
      outFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
      outFile.initWithPath(filePath);
    } catch (ex) {
      dump("Exception in graphicsConverter.resolveRelativeFilePath: " + ex + "\n");
    }
    return outFile;
  },

  canUseFormatForTypeset: function(extension) {
    return (this.typesetGraphicTypes.indexOf(extension.toLowerCase()) >= 0);
  },

  //Note: documentDir should be an nsILocalFile
  ensureTypesetGraphicForElement: function(objElement, documentDir, aWindow, callbackObject) {
    dump("In graphicsConverter.ensureTypesetGraphicForElement, 1\n");
    if (objElement.getAttribute("msigraph") == "true")
      return false;
    var gfxFileStr = objElement.getAttribute("data");
    if (!gfxFileStr || !gfxFileStr.length)
      gfxFileStr = objElement.getAttribute("src");
    if (!gfxFileStr || !gfxFileStr.length) {
      dump("Error in graphicsConverter.ensureTypesetGraphicForElement! No data or src for " + objElement.nodeName + " element.\n");
      return false;
    }
    if (!this.isDisplayableGraphicFile(gfxFileStr))
      return false;

    var theUnits = objElement.getAttribute("units");
    if (!theUnits || !theUnits.length)
      theUnits = "in";
    var theWidth = objElement.getAttribute("imageWidth") || objElement.getAttribute("width") || objElement.getAttribute("naturalWidth");
    var theHeight = objElement.getAttribute("imageHeight") || objElement.getAttribute("height") || objElement.getAttribute("naturalHeight");
    if (!theWidth || !theHeight) {
      var pixWidth, pixHeight;
      try {
        if (!theWidth) {
          pixWidth = objElement.offsetWidth;
          this.handler.initCurrentUnit(theUnits);
          var attrValStr = String(this.handler.getValueOf(pixWidth, "px"));
          objElement.setAttribute("imageWidth", attrValStr);
          objElement.setAttribute("naturalWidth", attrValStr);
        }
        if (!theHeight) {
          pixHeight = objElement.offsetHeight;
          attrValStr = String(this.handler.getValueOf(pixHeight, "px"));
          objElement.setAttribute("imageHeight", attrValStr);
          objElement.setAttribute("naturalHeight", attrValStr);
        }
      } catch (exc) {
        dump("In graphicsConverter.ensureTypesetGraphicForElement, error [" + exc + "] trying to get offsetWidth or offsetHeight for graphic [" + gfxFileStr + "]\n");
      }
    }

    var typesetFile = objElement.getAttribute("typesetSource");
    if (typesetFile && typesetFile.length) {
      var typesetNSFile = this.resolveRelativeFilePath(documentDir, typesetFile);
      if (typesetNSFile && typsetNSFile.exists())
        return false;
    }
    dump("In graphicsConverter.ensureTypesetGraphicForElement, 2\n");
    var gfxFile = this.resolveRelativeFilePath(documentDir, gfxFileStr);
    dump("In graphicsConverter.ensureTypesetGraphicForElement, 3\n");
    gfxFileStr = gfxFile.leafName;
    var nameAndExt = this.splitExtension(gfxFileStr);
    if (this.canUseFormatForTypeset(nameAndExt.ext))
      return false;

    var bChanged = false;
    var graphicsDirNames = ["tcache", "graphics", "gcache"];
    var targetDir, dirEntries;
    var aFile, fName, fNameAndExt;
    for (var jj = 0; jj < graphicsDirNames.length; ++jj) {
      dump("In graphicsConverter.ensureTypesetGraphicForElement, " + String(jj + 4) + "\n");
      targetDir = documentDir.clone();
      targetDir.append(graphicsDirNames[jj]);
      if (!targetDir.exists())
        continue;
      dirEntries = targetDir.directoryEntries;
      while (dirEntries.hasMoreElements()) {
        aFile = dirEntries.getNext().QueryInterface(Components.interfaces.nsILocalFile);
        fNameAndExt = this.splitExtension(aFile.leafName);
        if ((fNameAndExt.name == nameAndExt.name) && this.canUseFormatForTypeset(nameAndExt.ext))
          return false; //no action needed
      }
    }

    //if we get here, we need to generate a typesettable graphic file
    targetDir = documentDir.clone();
    targetDir.append("tcache");
    if (!targetDir.exists())
      targetDir.create(1, 0755);
    dump("In graphicsConverter.ensureTypesetGraphicForElement, 7\n");
    var targExtArray = this.getTargetFileExtensions(gfxFile, targetDir, "tex", aWindow);
    var targetFile;
    dump("In graphicsConverter.ensureTypesetGraphicForElement, 8\n");
    if (targExtArray && targExtArray.length) {
      fName = nameAndExt.name + targExtArray[targExtArray.length - 1];
      targetFile = targetDir.clone();
      targetFile.append(fName);
      this.doImportGraphicsToTarget(gfxFile, targetFile, "tex", aWindow, callbackObject);
      var newGraphic = documentDir.clone();
      newGraphic.append("tcache");
      newGraphic.append(nameAndExt.name + '.png');
                       
      var newGraphicPath = msiMakeUrlRelativeTo(newGraphic, documentDir);

      if (objElement.hasAttribute("src")) 
         objElement.removeAttribute("src");
      objElement.setAttribute("src", newGraphicPath);
      if (objElement.hasAttribute("data")) 
         objElement.removeAttribute("data");
      objElement.setAttribute("data", newGraphicPath);
      bChanged = true;
      objElement.setAttribute("-moz_dirty","true");
    }
    dump("In graphicsConverter.ensureTypesetGraphicForElement, 9\n");
    return bChanged;
  },

  ensureTypesetGraphicsForDocument: function(aDocument, aWindow) {
    dump("In graphicsConverter.ensureTypesetGraphicsForDocument, 1\n");
    var docURI = msiURIFromString(aDocument.documentURI);
    var filePath = msiPathFromFileURL(docURI);
    var documentDir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
    //    dump("In graphicsConverter.ensureTypesetGraphicsForDocument, 2\n  with documentURI=[" + aDocument.documentURI + "]\n");
    documentDir.initWithPath(filePath);
    dump("In graphicsConverter.ensureTypesetGraphicsForDocument, 3\n");
    documentDir = documentDir.parent;

    var objList = aDocument.getElementsByTagName("object");
    var imgList = aDocument.getElementsByTagName("img");
    var multiCallbackHandler = new graphicsMultipleTimerHandler();
    var timerHandler;
    var bChanged = false;

    for (var ii = 0; ii < objList.length; ++ii) {
      timerHandler = multiCallbackHandler.addNewTimerHandler(60000); //set time limit of 60 seconds for conversion to finish?
      bChanged = this.ensureTypesetGraphicForElement(objList[ii], documentDir, aWindow, null) || bChanged;
    }
    for (ii = 0; ii < imgList.length; ++ii) {
      timerHandler = multiCallbackHandler.addNewTimerHandler(60000); //set time limit of 60 seconds for conversion to finish?
      bChanged = this.ensureTypesetGraphicForElement(imgList[ii], documentDir, aWindow, timerHandler) || bChanged;
    }
    return (bChanged ? multiCallbackHandler : null);
  }
};

var graphicsTimerCallbackBase = {
  statusNone: 0,
  statusRunning: 1,
  statusSuccess: 2,
  statusFailed: 3,
  maxGraphicsLoadTime: 1200000, //20 minutes - longest we'll allow waiting for a conversion?

  statusStrings: ["none", "running", "success", "failed"],

  timer: null,
  sourceFile: null,
  importTargFile: null,
  importProcess: null,
  importStatus: 0,
  timerCount: 0,
  timerInterval: 200,
  timerCountLimit: 25,
  endCallback: null,
  errorString: "",
  userData: null,
  endCallbackObject: null,
  timerStopped: false,
  mode: "import",

  notify: function(aTimer) {
    if (aTimer)
      this.timer = aTimer;
    this.checkStatus();

    if (this.importStatus == this.statusRunning) {
      ++this.timerCount;
      //      var exitVal = "[" + (this.importProcess ? this.importProcess.exitValue : "none");
      //      if (!(this.timerCount & 1))
      //        dump("In graphics loading timerCallback for file [" + this.importTargFile.path + ", timer count is [" + this.timerCount + "], status is " + this.statusStrings[this.importStatus] + ", timer limit is " + this.timerCountLimit + ".\n");
      if (this.timerCount < this.timerCountLimit) //keep waiting
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

  onEndTimer: function() {
    if (this.endCallbackObject) {
      this.endCallbackObject.terminalCallback(this.sourceFile, this.importTargFile, this.importStatus, this.errorString, this.userData);
    }
  },

  reset: function() {
    this.cleanUp();
    this.timerCount = 0;
    this.sourceFile = null;
    this.importProcess = null;
    this.importStatus = this.statusNone;
  },

  cleanUp: function() {
    if (this.importTargFile) {
      var sentFile = this.getFailureSentinelFile();
      if (sentFile.exists())
        sentFile.remove(false);
      this.importTargFile = null;
      var logFile = this.getImportLogFile();
      if (logFile.exists())
        logFile.remove(false);
    }
  },

  startLoading: function(srcFile, targFile, process, mode, aTimer) {
    this.timerCount = 0;
    this.timer = aTimer;
    this.sourceFile = srcFile;
    this.importTargFile = targFile;
    this.importProcess = process;
    this.importStatus = this.statusRunning;
    this.mode = mode;
    //    dump("In timerhandler for target file " + targFile.path + "; failure sentinel is " + this.getFailureSentinelFile().path + ".\n");
  },

  stop: function() {
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

  checkStatus: function() {
    if (this.importStatus == this.statusRunning) {
      if (this.importTargFile.exists())
        this.importStatus = this.statusSuccess;
      else if (!this.importProcess || this.processFailed())
        this.importStatus = this.statusFailed;
    }
  },

  didNotSucceed: function() {
    return (this.importStatus != this.statusSuccess);
  },

  checkLogFileStatus: function() {
    var errorRE = /(error)|(fail)/i;
    if (this.importStatus == this.statusRunning) {
      if (this.getImportLogFile() && this.getImportLogFile().exists()) {
        var logStr;
        //        dump("In timerHandler.checkLogFileStatus, found file [" + this.getImportLogFile().path + "] exists.\n");
        try {
          logStr = this.getImportLogInfo();
        } catch (ex) {
          //          dump("In graphicsTimerHandler.checkLogFileStatus, exception: " + ex + "\n");
        }
        if (logStr) {
          var bItFailed = false;
          if (errorRE.exec(logStr)) {
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

  checkFinalStatus: function() {
    this.checkStatus();
    if (this.importStatus == this.statusRunning)
      this.checkLogFileStatus();
  },

  isLoading: function() {
    this.checkStatus();
    return (this.importStatus == this.statusRunning);
  },

  getFailureSentinelFile: function() {
    var sentinelFile = this.importTargFile.clone();
    if (!sentinelFile || !sentinelFile.path)
      return null;
    var leaf = sentinelFile.leafName;
    leaf += ".txt";
    sentinelFile = sentinelFile.parent;
    sentinelFile.append(leaf);
    return sentinelFile;
  },

  getImportLogFile: function() {
    var logFile = this.importTargFile.clone();
    if (!logFile || !logFile.path)
      return null;
    var leaf = logFile.leafName;
    leaf += ".log";
    logFile = logFile.parent;
    logFile.append(leaf);
    return logFile;
  },

  processFailed: function() {
    if (this.importStatus == this.statusRunning) {
      var sentFile = this.getFailureSentinelFile();
      var sentPath = sentFile ? sentFile.path : "";
      //      dump("In graphicsTimerHandler.processFailed, sentinel file [" + sentPath + (sentFile.exists() ? "] exists.\n" : "] doesn't exist.\n"));
      if (sentFile && sentFile.exists())
        this.importStatus = this.statusFailed;
    }
    //    dump("In graphicsTimerHandler.processFailed, status is " + this.statusStrings[this.importStatus] + ".\n");
    return (this.importStatus == this.statusFailed);
  },

  getImportLogInfo: function() {
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

function graphicsTimerHandler(millisecondsTimeLimit, terminateCallbackObject, data) {
  //  this.timercopy = null;
  //  this.sourceFile = null;
  //  this.importTargFile = null;
  //  this.importProcess = null;
  //  this.importStatus = graphicsTimerCallbackBase.statusNone;
  //  this.timerCount = 0;
  //  this.timerInterval = 200;
  if (millisecondsTimeLimit == 0)
    millisecondsTimeLimit = this.maxGraphicsLoadTime;
  this.timerCountLimit = (millisecondsTimeLimit / this.timerInterval);
  this.endCallbackObject = terminateCallbackObject;
  if (data)
    this.userData = data;

}

graphicsTimerHandler.prototype = graphicsTimerCallbackBase;

var graphicsMultipleTimerBase = {
  mTimerHandlers: [],

  terminalCallback: function(sourceFile, importTargFile, importStatus, errorString, userData) {
    var ix = 0;
    if (userData)
      ix = userData;
    else
      ix = 1 + this.findByTargFile(importTargFile);
    if (ix > 0)
      this.mTimerHandlers[ix - 1].mbFinished = true;
  },

  isFinished: function() {
    var bDone = true;
    for (var ix = 0; bDone && (ix < this.mTimerHandlers.length); ++ix) {
      if (!this.mTimerHandlers[ix].mbFinished)
        bDone = false;
    }
    //    dump("In graphicsMultipleTimerHandler.isFinished; returning " + (bDone ? "true\n" : "false\n"));
    return bDone;
  },

  addTimerHandler: function(aTimerHandler) {
    this.mTimerHandlers.push({
      mHandler: aTimerHandler,
      mbFinished: false
    });
    return this.mTimerHandlers.length; //returning the index of the new timer
  },

  addNewTimerHandler: function(mSecTimeLimit) {
    if (!mSecTimeLimit)
      mSecTimeLimit = 300000; //5 minutes outside limit?
    var aTimerHandler = new graphicsTimerHandler(mSecTimeLimit, this, this.mTimerHandlers.length + 1);
    this.addTimerHandler(aTimerHandler);
    return aTimerHandler;
  },

  stopAll: function() {
    for (var ix = 0; ix < this.mTimerHandlers.length; ++ix) {
      if (this.mTimerHandlers[ix].mHandler)
        this.mTimerHandlers[ix].mHandler.stop();
    }
  },

  getStatus: function(aTargFile) {
    var ix = this.findByTargFile(aTargFile);
    if (ix > 0 && this.mTimerHandlers[ix].mHandler)
      return this.mTimerHandlers[ix].mHandler.importStatus;
    return graphicsTimerCallbackBase.statusNone;
  },

  getActiveCount: function() {
    var rv = 0;
    for (var ix = 0; ix < this.mTimerHandlers.length; ++ix) {
      if (this.mTimerHandlers[ix] && this.mTimerHandlers[ix].mHandler && this.mTimerHandlers[ix].mHandler.isLoading())
      ++rv;
    }
    return rv;
  },

  findByTargFile: function(targFile) {
    var index = -1;
    for (var ix = 0;
      (index < 0) && (ix < this.mTimerHandlers.length); ++ix) {
      if (this.mTimerHandlers[ix].mHandler.importTargFile.path == targFile.path)
        index = ix;
    }
    return index;
  },

  getTargetFile: function(srcFile, mode) {
    var targFile = null;
    for (var ix = 0;
      (!targFile) && (ix < this.mTimerHandlers.length); ++ix) {
      if ((this.mTimerHandlers[ix].mHandler.sourceFile.path == srcFile.path) && (this.mTimerHandlers[ix].mHandler.mode == mode))
        targFile = this.mTimerHandlers[ix].mHandler.importTargFile;
    }
    return targFile;
  }
};

function graphicsMultipleTimerHandler() {
  this.mTimerHandlers = [];
}

graphicsMultipleTimerHandler.prototype = graphicsMultipleTimerBase;


function msiMakeUrlRelativeTo(inputFile, baseDir) {
   var inputURL = msiFileURLFromFile(inputFile);
   var baseURL = msiFileURLFromFile(baseDir);
 
   var basePath = baseURL.path;
   var urlPath = inputURL.path;

   // Get base filename before we start chopping up the basePath
   var baseFilename = baseURL.fileName;
   
   var doCaseInsensitive = (this.graphicsConverter.OS = 'win');
   if (doCaseInsensitive)
     basePath = basePath.toLowerCase();
 
    
   // Both url and base paths now begin with "/"
   // Look for shared dirs starting after that
   urlPath = urlPath.slice(1);
   basePath = basePath.slice(1);
   var firstDirTest = true;
   var nextBaseSlash = 0;
   var done = false;
   // Remove all matching subdirs common to both base and input urls
   do {
     nextBaseSlash = basePath.indexOf('/');
     var nextUrlSlash = urlPath.indexOf('/');
     if (nextUrlSlash === -1) {
       // We're done matching and all dirs in url
       // what's left is the filename
       done = true;
       // Remove filename for named anchors in the same file
       if (nextBaseSlash === -1 && baseFilename) {
         var anchorIndex = urlPath.indexOf('#');
         if (anchorIndex > 0) {
           var urlFilename = doCaseInsensitive ? urlPath.toLowerCase() : urlPath;
           if (urlFilename.indexOf(baseFilename) === 0)
             urlPath = urlPath.slice(anchorIndex);
         }
       }
     } else if (nextBaseSlash >= 0) {
       // Test for matching subdir
       var baseDir = basePath.slice(0, nextBaseSlash);
       var urlDir = urlPath.slice(0, nextUrlSlash);
       if (doCaseInsensitive)
         urlDir = urlDir.toLowerCase();
       if (urlDir === baseDir) {
         // Remove matching dir+"/" from each path
         //  and continue to next dir
         basePath = basePath.slice(nextBaseSlash + 1);
         urlPath = urlPath.slice(nextUrlSlash + 1);
       } else {
         // No match, we're done
         done = true;
         // Be sure we are on the same local drive or volume
         //   (the first "dir" in the path) because we can't
         //   relativize to different drives/volumes.
         // UNIX doesn't have volumes, so we must not do this else
         //  the first directory will be misinterpreted as a volume name
         if (firstDirTest && baseScheme === 'file' && os !== 'osx')
           return inputUrl;
       }
     } else
       // No more base dirs left, we're done
       done = true;
     firstDirTest = false;
   } while (!done);
   // Add "../" for each dir left in basePath
   while (nextBaseSlash > 0) {
     urlPath = '../' + urlPath;
     nextBaseSlash = basePath.indexOf('/', nextBaseSlash + 1);
   }
   return urlPath;
}


// function GetFilename(urlspec)
// {
//   if (!urlspec || IsUrlAboutBlank(urlspec))
//     return "";
// 
//   var IOService = GetIOService();
//   if (!IOService)
//     return "";
// 
//   var filename;
// 
//   try {
//     var uri = IOService.newURI(urlspec, null, null);
//     if (uri)
//     {
//       var url = uri.QueryInterface(Components.interfaces.nsIURL);
//       if (url)
//         filename = url.fileName;
//     }
//   } catch (e) {}
// 
//   return filename ? filename : "";
// }
