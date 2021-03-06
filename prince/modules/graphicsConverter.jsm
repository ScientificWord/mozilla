var EXPORTED_SYMBOLS = ["graphicsConverter", "graphicsTimerHandler", "graphicsMultipleTimerHandler"];

Components.utils.import("resource://app/modules/os.jsm");
Components.utils.import("resource://app/modules/pathutils.jsm");
Components.utils.import("resource://app/modules/unitHandler.jsm");

const nsIFile = Components.interfaces.nsIFile;
const nsIINIParser = Components.interfaces.nsIINIParser;
const nsIINIParserFactory = Components.interfaces.nsIINIParserFactory;
const nsILocalFile = Components.interfaces.nsILocalFile;
const nsISupports = Components.interfaces.nsISupports;


function x_setStyleAttributeOnNode(node, attr, value) {
  var style = node.getAttribute('style');
  if (style.indexOf(attr+':') < 0) {
    style += '; ' + attr + ': ' + value;
  }
  node.setAttribute('style', style);
}

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
  handler: null,
  inkscapeScriptState: 0,
  inkscapeFileList: null,
  inkscapeBatching: false,

  init: function(aWindow, baseDirIn, product) {
    this.handler = new UnitHandler(null);
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
    this.product = product;
    // This next line will probably need to be removed when the utilities directory is reorganized.
    // this.converterDir.append("bin");
  },

// pathForOS takes a string path and converts all slashes to forward or backward slashes depending on the OS.
// Use this for paths going out to the OS, as, for example, when sending file parameters to graphics conversions.

  pathForOS: function(path) {
    if (this.OS === 'win')
      path = path.replace(/\//g, '\\');
    else
      path = path.replace(/\\/g, '/'); // '
    return path;
  }, 

// pathForBrowser guarantees that all slashes are forward slashes, as required by most browsers and SWP.
// Use this when the path goes into a documents, as a data attributer for an <object>, for example.
  pathForBrowser: function(path) {
    return path.replace(/\\/g, '/');
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
          // if (this.OS === "win") {
          //   returnPath = "graphics\\" + leaf;
          // }
          // else {
            returnPath = "graphics/" + leaf;
          // }

          if (graphicsFile.path !== destFile.path) {
            try {
              if (destFile.exists()) destFile.remove(false);
            }
            catch(e) {}
            try {
              graphicsFile.copyTo(destDir, leaf);
            }
            catch(e) {}
          }
        }
        copiedFile = this.baseDir.clone();
        copiedFile.append("graphics");
        copiedFile.append(leaf);

        if (command != "") {
          if (width && height) {      
            returnPath = this.execCommands(copiedFile, command, width, height);
          }
          else {
            returnPath = this.execCommands(copiedFile, command);
          }
        }
      }
      catch(e) {
        //msidump(e.message);
      }
    }
    return this.pathForBrowser(returnPath);
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
    var paramOffset = 0;
    var toolsDir = this.converterDir.clone();
    var pathsfile = this.codeDir.clone(); 
    var leaf = 'MSITeX.' + (this.OS==='win'?'cmd':'bash');
    pathsfile.append(leaf); // corresponds to $M in graphicsConversions.ini
    var regex0 = /\\/;  // look for slash or backslash
    var regex1 = /\//;
    var dollar1;
    var dollar2; // these correspond to $1 and $2 in graphicsConversions.ini
    var theProcess;
    var utilityFile;
    var programFile;
    var extension;
    var paramArray;
    var paramArray1 = [];
    // var paramArray2 = [];
    var param;
    var temp;
//    var bRunSynchronously = true;
    var resolutionParameter = null;
    var returnPath; // the path of the file to display, relative to baseDir
    returnPath = 'graphics/' + graphicsFile.leafName;
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
        else paramArray=[];
        ithCommand = commandlist[i];
        ithCommand = ithCommand.replace(/^\s*/,'');  // aka 'trim'
        ithCommand = ithCommand.replace(/\s*$/,'');
        commandparts = commandlist[i].split(',');
        progname = commandparts[0];
        theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
        // use progname unchanged if it contains a '/', otherwise assume it is in the utilties dir.
        if (!regex0.test(progname) && !regex1.test(progname))
        {
          programFile = toolsDir.clone();
          programFile.append(progname);
        }
        else if (regex0.test(progname) || regex1.test(progname)) {
          programFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
          programFile.initWithPath(progname);
        }

        theProcess.init(programFile);
        // build parameters
        // bRunSynchronously = (i+1) < commandlist.length;  // run all commands but the last synchronously
        if (width && height) {
          resolutionParameter = this.setResolutionParameters(graphicsFile, width, height );
        }
        // We skip over the program name in building the parameters, so we start j at 1, not 0
        for (j = 1; j < commandparts.length; j++) {
          param = commandparts[j];
       //   if (j==0 && param != "sam2p") continue;
          param = param.replace('$M', pathsfile.path);
          param = param.replace("$1", dollar1);
          if (regex2.test(param)) {
            param = param.replace("$2", dollar2);
            param = param.replace(/^\s*/,'');  // aka 'trim'
            param = param.replace(/\s*$/,'');

            if (!/tcache/.test(param)) {
              returnPath = param.replace(/\"/g, '');
            } 
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
            paramArray.push(this.pathForOS(param));
          // }
        }
        if (resolutionParameter) {
          paramArray.push(resolutionParameter);
        }
        theProcess.run(true, paramArray, paramArray.length);
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
    if (!newWidth || !newHeight || newWidth===0 || newHeight === 0 || isNaN(newWidth) || isNaN(newHeight)) return null;
    var originalUrl = graphicsFile.path;
    var pathParts = originalUrl.split('.');
    var extension = pathParts[pathParts.length-1];
    if (!extension || extension.length === 0) extension = pathParts[pathParts.length-2];
    if (!extension || extension.length === 0) return;
    extension = extension.toLowerCase();
    var origDimension = null;
    var resolutionParameter;
    var intermediate;
    var w, h;
    var pixelsPerInch;
    if (origDimension && origDimension.width > 0) {
      // origDimension is in big points, 'bp'
      this.handler.setCurrentUnit('px');
      w = this.handler.getValueOf(origDimension.width, 'bp');
      h = this.handler.getValueOf(origDimension.height, 'bp');
      pixelsPerInch = this.handler.getValueOf(1, 'in');
      resolutionParameter = "-r"+Math.round(pixelsPerInch*(w/origDimension.width))+"x"+
        Math.round(pixelsPerInch*(w/origDimension.width));
      return resolutionParameter;
    }
    return null;
  },

  readSizeFromSVGFile: function(svgFile)
  {
    var theText = this.getFileAsString(svgFile);
    var dimensions = {width: 0, height: 0, unit: 'px'};
    var regexw = /width\s*=\"\s*(\d+)([a-z]*)\"/;
    var regexh = /height\s*=\"\s*(\d+)([a-z]*)\"/;
    var match = regexw.exec(theText);
    if (match) {
      dimensions.width = match[1];
      if (match.length > 2) dimensions.unit = match[2];
    }
    match = regexh.exec(theText);
    if (match) {
      dimensions.height = match[1];
      if (match.length > 2) dimensions.unit = match[2];
    }
    return dimensions;

  },

  readSizeFromEPSFile: function(epsFile)
  {
    var theText = this.getFileAsString(epsFile);
    var dimensions = {width: 0, height: 0, unit: 'pt'};
    var regex = /%%BoundingBox:\s*(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/;
    var match = regex.exec(theText);
    if (match){
      dimensions.width = match[3] - match[1];
      dimensions.height = match[4] - match[2];
    }
    return dimensions;

  },

//   readSizeFromPDFFile: function(pdfFile)
//   {
//     var theText = this.getFileAsString(pdfFile);

// //    var BBRE = /\/BBox\s*\[([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s+([0-9\.]+)\s*\]/;
//     var mediaBoxRE = /\/MediaBox\s+\[\s+([.0-9]+)\s+([.0-9]+)\s+([.0-9]+)\s+([.0-9]+)/;

//     // function checkBoundingBox( objString )
//     // {
//     //   var res = null;
//     //   var bbArray = BBRE.exec(objString);
//     //   if (bbArray && bbArray[1] && bbArray[2])
//     //   {
//     //     if (bbArray[3] && bbArray[4])
//     //     {
//     //       res = {wdth : Math.round(9.6 * (Number(bbArray[3]) - Number(bbArray[1]))),
//     //              ht : Math.round(9.6 * (Number(bbArray[4]) - Number(bbArray[2]))) };   //9.6 here is 96 pts-per-inch/10 since these are given in tenths of an inch
//     //     }
//     //     else  //this shouldn't happen, though
//     //     {
//     //       res = {wdth : Math.round(9.6 * Number(bbArray[1])),
//     //              ht : Math.round(9.6 * Number(bbArray[2])) };   //9.6 here is 96 pts- 
//     //   }
//     //   return res;
//     // }

//     function checkMediaBox( objString )
//     {
//       var res = null;
//       var dimsArray = mediaBoxRE.exec(objString);
//       if (dimsArray && dimsArray.length > 4)
//       {
//           res = { width : Math.round(Number(dimsArray[3]) - Number(dimsArray[1])),
//                   height : Math.round(Number(dimsArray[4]) - Number(dimsArray[2])),
//                   unit: 'pt' };
//       }
//       return res;
    
//     return checkMediaBox(theText);
//   },

  readSizeFromVectorFile: function(vecGraphics, extension) {
 
     if (extension === 'eps')
       return this.readSizeFromEPSFile(vecGraphics);
     if (extension === 'ps')
       return this.readSizeFromEPSFile(vecGraphics);
     if (extension === 'svg')
       return this.readSizeFromSVGFile(vecGraphics);
     return null;
   },

   setInitialWidthAndHeight: function setInitialWidthAndHeight(objectnode) {
     // This function looks at the naturalwidth and naturalheight, whether
     // the prefs initial width and height are supposed to be used, and sorts
     // it out to set usedims and the width and height.
     var useWidth = false;
     var useHeight = false;
     var autoWidth, autoHeight; // booleans, all
     var prefs = null;
     var prefWidth, prefHeight, prefUnit;
     var theUnits;
     var naturalheight, naturalwidth;
     var framenode;
     var unitHandler = new UnitHandler(null);

     var usedims; // = useWidth + 2*useHeight where usexxx is 0 or 1
     // get as much as we can out of the objectnode. It may have been through
     // this function before.
     var prefService = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefService);

     if (prefService)
       prefs = prefService.getBranch(null);


    try {
      framenode = objectnode.parentNode;
      theUnits = objectnode.getAttribute('units');
      if (framenode == null || framenode.nodeName != 'msiframe') {
        framenode = objectnode;
      }
      if (theUnits == null) theUnits =framenode.getAttribute('units');
      if (theUnits == null) finalThrow('No units given in object node');
      unitHandler.initCurrentUnit(theUnits);
      if (framenode.hasAttribute('naturalwidth') && framenode.hasAttribute('naturalheight')) {
        naturalheight = framenode.getAttribute('naturalheight');
        naturalwidth = framenode.getAttribute('naturalwidth');        
      } else {
        naturalheight = unitHandler.getValueOf(objectnode.offsetHeight,'px');
        naturalwidth = unitHandler.getValueOf(objectnode.offsetWidth, 'px');        
      }
      objectnode.setAttribute('naturalwidth',naturalwidth);
      objectnode.setAttribute('naturalheight',naturalheight);
      framenode.setAttribute('naturalwidth',naturalwidth);
      framenode.setAttribute('naturalheight',naturalheight);

      width = naturalwidth;
      height = naturalheight;

      usedims = framenode.getAttribute('usedims');
      if (usedims) {
       if (usedims > 1) {
         useHeight = true;
         usedims = usedims - 2;
       }
       if (usedims > 0) {
         useWidth = true;
       }
      } else {
       if (prefs == null) prefs = GetPrefs();
       useWidth = prefs.getBoolPref('swp.graphics.usedefaultwidth');
       useHeight = prefs.getBoolPref('swp.graphics.usedefaultheight');
       usedims = 0;
       // encoding both useWidth and useHeight in a small integer
       if (useHeight) usedims = 2;
       if (useWidth) usedims++;
      }
      if (useWidth || useHeight) {
       prefUnit=prefs.getCharPref('swp.graphics.units'); 
      }
      prefWidth = prefs.getCharPref('swp.graphics.hsize');
      prefHeight = prefs.getCharPref('swp.graphics.vsize');
      if (useWidth) {
        width = unitHandler.getValueOf(prefWidth, prefUnit);
        if (useHeight) {
          height = unitHandler.getValueOf(prefHeight, prefUnit);
        } else {
          height = Number(width)*(Number(naturalheight)/Number(naturalwidth));
        }
      } else if (useHeight) {  // useWidth is already false
        height = unitHandler.getValueOf(prefHeight, prefUnit); 
        width = Number(height) * (Number(naturalwidth)/Number(naturalheight));  // use pref ht, compute width
      } // else use neither pref, so leave height and width alone.
      // Now save the results in the object node
      objectnode.setAttribute('usedims',usedims);
      framenode.setAttribute('usedims',usedims);
      objectnode.setAttribute('width',Math.round(Number(unitHandler.getValueAs(width,'px'))));
      framenode.setAttribute('width',width);
      objectnode.setAttribute('height',Math.round(Number(unitHandler.getValueAs(height,'px'))));
      framenode.setAttribute('height',height);
      x_setStyleAttributeOnNode(objectnode, 'height', height+theUnits); 
      x_setStyleAttributeOnNode(objectnode, 'width', width+theUnits); 
      x_setStyleAttributeOnNode(framenode, 'height', height+theUnits);
      x_setStyleAttributeOnNode(framenode, 'width', width+theUnits);
    }
    catch(e) {
      msidump(e.message);
    }
  },




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
  // getBatchFile: function(theOS) {
  //   var extension = "";
  //   if (theOS == "win") extension = "cmd";
  //   else extension = "bash";
  //   var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  //   var templateFile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
  //   templateFile.append("rungfxconv." + extension);
  //   return templateFile;
  // },

  getConvertibleFileTypes: function(aWindow) {
    return;
    // var os = getOS(aWindow);
    // var cmdFile = this.getBatchFile(os);
    // var cmdContents = this.getFileAsString(cmdFile);
    // var theLines = cmdContents.split(/\n/);
    // var fileTypes = [];
    // var inFileTypeList = false;

    // var remarkStr = "^\\s*REM\\s+";
    // if (os != "win")
    //   remarkStr = "^\\s*#\\s*";
    // var remarkRE = new RegExp(remarkStr);
    // var startFlagRE = new RegExp(remarkStr + "BEGIN\\s+EXTENSION\\s+LIST");
    // var endFlagRE = new RegExp(remarkStr + "END\\s+EXTENSION\\s+LIST");
    // var theLine;

    // for (var ix = 0; ix < theLines.length; ++ix) {
    //   if (!inFileTypeList) {
    //     if (startFlagRE.test(theLines[ix]))
    //       inFileTypeList = true;
    //   } else if (endFlagRE.test(theLines[ix]))
    //     inFileTypeList = false;
    //   else if (remarkRE.test(theLines[ix])) {
    //     theLine = theLines[ix].replace(remarkRE, "");
    //     theLine = this.trimStr(theLine);
    //     fileTypes = fileTypes.concat(theLine.split(/\s+/));
    //   }
    // }
    // return fileTypes;
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

  allDerivedGraphicsExist: function(documentDir, graphicFile, objElement) 
  {
    // The following code is a kludge because we wanted all information about graphics conversion procedures to be in graphicsConversions.ini. It's not that bad, though,
    // because the following tests depend on the graphics type, but not on the conversion programs.
    var extension = getExtension(graphicFile.path).toLowerCase();
    var extensionRE = /\.([^\.]+)$/;
    var bareLeaf = graphicFile.leafName.replace(extensionRE,'');
    var testFile = documentDir.clone();
    switch (extension) {
      case 'wmf':
      case 'emf':
        testFile.append('graphics');
        testFile.append(graphicFile.leafName);
        if (testFile.exists()) {
          // testFile = testFile.parent;
          // testFile.append(bareLeaf + '.eps');
          // if (testFile.exists()) {
          //   if (this.OS !== 'win') {
          //     objElement.setAttribute("copiedSrcUrl", 'graphics/' + bareLeaf + '.eps');
          //   }              
          testFile = testFile.parent.parent;
          testFile.append('gcache');
          testFile.append(bareLeaf + '.png');
          if (testFile.exists()) {
            objElement.setAttribute('data','gcache/'+bareLeaf+'.png');
            objElement.setAttribute('src','gcache/'+bareLeaf+'.png');
            if (this.product === 'snb') return true;
            else 
            {
              testFile = testFile.parent.parent;
              testFile.append('tcache');
              testFile.append(bareLeaf + '.pdf');
              if (testFile.exists()) return true;
            }
          }
          
        }
        break;

      case 'pdf':
        testFile.append('graphics');
        testFile.append(graphicFile.leafName);
        if (testFile.exists()) {
          testFile = testFile.parent.parent;
          testFile.append('gcache');
          testFile.append(bareLeaf + '.png');
          if (testFile.exists()) {
            objElement.setAttribute('data','gcache/'+bareLeaf+'.png');
            objElement.setAttribute('src','gcache/'+bareLeaf+'.png');
            return true;
          }
        }
        break;

      case 'eps':
      case 'ps':
        testFile.append('graphics');
        testFile.append(graphicFile.leafName);
        if (testFile.exists()) {
          testFile = testFile.parent.parent;
          testFile.append('gcache');
          testFile.append(bareLeaf + '.png');
          if (testFile.exists()) {
            objElement.setAttribute('data','gcache/'+bareLeaf+'.png');
            objElement.setAttribute('src','gcache/'+bareLeaf+'.png');
            if (this.product === 'snb') return true;
            else 
            {
              testFile = testFile.parent.parent;
              testFile.append('tcache');
              testFile.append(bareLeaf + '.pdf');
              if (testFile.exists()) return true;
            }
          }
        }
        break;

      case 'svg':
        testFile.append('graphics');
        testFile.append(graphicFile.leafName);
        if (testFile.exists()) {
          testFile = testFile.parent.parent;
          testFile.append('gcache');
          testFile.append(bareLeaf + '.png');
          if (testFile.exists()) {
            objElement.setAttribute('data','gcache/'+bareLeaf+'.png');
            objElement.setAttribute('src','gcache/'+bareLeaf+'.png');
            if (this.product === 'snb') return true;
            else 
            {
              testFile = testFile.parent.parent;
              testFile.append('tcache');
              testFile.append(bareLeaf + '.pdf');
              if (testFile.exists()) return true;
            }
          }
        }
        break;

      case 'gif':
      case 'tif':
      case 'tiff':
      case 'bmp':
        testFile.append('graphics');
        testFile.append(graphicFile.leafName);
        if (testFile.exists()) {
          testFile = testFile.parent.parent;
          testFile.append('gcache');
          testFile.append(bareLeaf + '.png');
          if (testFile.exists()) {
            objElement.setAttribute('data','gcache/'+bareLeaf+'.png');
            objElement.setAttribute('src','gcache/'+bareLeaf+'.png');
            return true;
          }
        }
        break;

      default:
        testFile.append('graphics');
        testFile.append(graphicFile.leafName);
        objElement.setAttribute('data','graphics/'+bareLeaf+'.'+extension);
        objElement.setAttribute('src','graphics/'+bareLeaf+'.'+extension);
        if (testFile.exists()) return true;
        break;
    }
    return false;
  },
  
  imageLoaded: function()
  {
    var datafile;
    var data;
    var basename;
    var pathparts;
    var ext;
    var frame = this.parentNode;
    var dimensions = null;
    var objectnode = this;
    var graphicDir;
    var docUrlString = this.ownerDocument.documentURI;
    var docurl = msiURIFromString(docUrlString);
    var unithandler = new UnitHandler();
    var extensionRE = /\.([^\.]+)$/;

    graphicDir = msiFileFromFileURL(docurl);
    graphicDir = graphicDir.parent;

    objectnode.setAttribute("msi_resize", true);
    if (objectnode.hasAttribute('width') && objectnode.hasAttribute('height')) return;
    graphicsConverter.setInitialWidthAndHeight(objectnode);
    // objectnode.removeEventListener("load", this, true);
    // ensureTypesetGraphicForElement put in a value for the data attribute. We will get that file.
    // data = objectnode.getAttribute('data');
    // data = data.replace('\\','/');
    // pathparts = data.split('/');
    // datafile = graphicDir.clone();
    // datafile.append(pathparts[0]);
    // datafile.append(pathparts[1]);
    // ext = extensionRE.exec(data)[1];

    // dimensions = graphicsConverter.readSizeFromVectorFile(datafile, ext);

    // if (ext === 'wmf' || ext === 'emf') {
    //   datafile = datafile.parent.parent;
    //   datafile.append('tcache');
    //   datafile.append(leafname.replace(ext,'pdf'));
    //   dimensions == graphicsConverter.readSizeFromVectorFile(datafile, 'pdf');
    // }
    // frame.setAttribute('naturalwidth', unithandler.getValueOf(dimensions.width, dimensions.unit));
    // frame.setAttribute('naturalheight', unithandler.getValueOf(dimensions.height, dimensions.unit));

 
  },



  //Note: documentDir should be an nsILocalFile
  //We require that the 'copiedSrcUrl' be set. Copy the original file before calling this.
  ensureTypesetGraphicForElement: function(objElement, documentDir) {
    var attrValStr;
    var inkscapeCmd;
    var pixWidth, pixHeight;
    // var frame;
    var unitHandler = new UnitHandler();
    var ext;
    var basename;
    var graphicsFile; // the file in the graphics directory. Given by 'copiedSrcUrl' attribute
    var gcacheFile; // the file used for screen representation, if graphicsFile is not suitable
    var tcacheFile; // the file used by TeX, if neither of the other two is suitable.
    var extensionRE = /\.([^\.]+)$/;
    var importName = null;
    var pngName;
    var pngFile;
    var pdfName;
    var pdfFile;
    var leafname;
    var tempname;
    var arr, arr2;



    if (objElement.getAttribute("msigraph") === "true")
      return null;
    // frame = objElement.parentNode;
    // if (!(frame && frame.nodeName === 'msiframe')) frame = null;

    var gfxFileStr = objElement.getAttribute("copiedSrcUrl");
    if (!gfxFileStr || gfxFileStr.length == 0) return null; 
    ext = getExtension(gfxFileStr);

    if (this.OS === "win") {
      gfxFileStr = documentDir.path + '\\' + gfxFileStr;
      gfxFileStr = gfxFileStr.replace("/", "\\"); // fix the separator after 'graphics'
    } else {
      gfxFileStr = documentDir.path + '/' + gfxFileStr;
    }

    graphicsFile = msiFileFromAbsolutePath(gfxFileStr);
    leafname = graphicsFile.leafName;
    if (!graphicsFile.exists()) return null;

    if (this.allDerivedGraphicsExist(documentDir, graphicsFile, objElement)) {
      tempname = objElement.data;
      arr = tempname.split('/');
      arr2 = [arr[arr.length - 2], arr[arr.length -1]];
      return arr2.join('/');
    }
    if ((ext === 'wmf' || ext === 'emf' || ext === 'svg') && this.inkscapeBatching) {
      // we will be using inkscape. It is much faster if we batch the operations. this.inkscapeBatching
      // has been set if we are converting all graphics in a newly imported document. 
      if (this.inkscapeScriptState === 0) { // do initialization
        this.inkscapeScriptState = 1;
        this.inkscapeFileList = '';
      } 
      leafname = leafname.replace(/\.[ew]mf$/,'');
      pngName = leafname + '.png';
      pngFile = documentDir.clone();
      pngFile.append('gcache');
      pngFile.append(pngName);
      this.assureSubdir("gcache");
      inkscapeCmd = '-e "'+ pngFile.path +'" "' + graphicsFile.path + '"';
      this.inkscapeFileList += inkscapeCmd + '\n';

      pdfName = leafname + '.pdf';
      pdfFile = documentDir.clone();
      pdfFile.append('tcache');
      pdfFile.append(pdfName);
      this.assureSubdir("tcache");
      inkscapeCmd = '-A "'+ pdfFile.path +'" "' + graphicsFile.path + '"';
      this.inkscapeFileList += inkscapeCmd + '\n';
    } else 
      importName = this.copyAndConvert(graphicsFile, true); //, theWidth, theHeight);
    if (importName) {
      if (objElement.nodeName === 'object') objElement.setAttribute("data", importName);
      else objElement.setAttribute("src", importName);
    }


    // var theUnits = objElement.getAttribute("units");
    // if ((!theUnits || !theUnits.length) && frameframeframe != null) {
    //   theUnits = frame.getAttribute("units");
    // }
    // if (!theUnits || !theUnits.length) {
    //   theUnits = "cm";
    //   frame.setAttribute("units", theUnits);
    //   objElement.setAttribute("units", theUnits);
    // }
    // unitHandler.initCurrentUnit(theUnits);
    return importName;
  },

  ensureTypesetGraphicsForDocument: function(aDocument) {
    var docURI = msiURIFromString(aDocument.documentURI);
    var filePath = msiPathFromFileURL(docURI);
    var timer;
    var documentDir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
    documentDir.initWithPath(filePath);
    documentDir = documentDir.parent;

    var objList = aDocument.getElementsByTagName("object");
    var imgList = aDocument.getElementsByTagName("img");
    var pngPath;
    if (objList.length>0 || imgList.length>0)
    {
      this.beginBatchProcessing();
      for (ii = 0; ii < imgList.length; ++ii) {
        this.ensureTypesetGraphicForElement(imgList[ii], documentDir);
      }      
      for (var ii = 0; ii < objList.length; ++ii) {
        this.ensureTypesetGraphicForElement(objList[ii], documentDir);
      }
      this.endBatchProcessing(aDocument);
      for (ii = 0; ii < objList.length; ++ii) {
        copiedSrcUrl = objList[ii].getAttribute('copiedSrcUrl')
        if (/[ew]mf$/.test(copiedSrcUrl)) {
          copiedSrcUrl = copiedSrcUrl.replace('graphics', 'gcache');
          copiedSrcUrl = copiedSrcUrl.replace(/[ew]mf$/, 'png');
          objList[ii].setAttribute('data', copiedSrcUrl);
          objList[ii].setAttribute('src', copiedSrcUrl);
        }
      }
    }
  },


  beginBatchProcessing: function () {
    this.inkscapeBatching = true;
    this.inkscapeScriptState = 0; // file not yet created.
  },

  endBatchProcessing: function (theDoc) {
    var urlstring;
    var filepath; 
    var file;
    var batchfile;
    var strm;
    var quit = 'quit\n';
    var theProcess;
    var pathsfile;
    var leaf;
    var ext;
    var args;
    var copiedSrcUrl;
    var filelist = 'wmflist';
    if (this.OS == "win")
      ext = 'cmd';
    else
      ext = 'bash';

    if (this.inkscapeScriptState === 1) {
      file = this.baseDir.clone();

      try
      {
        file.append(filelist);
        file.QueryInterface(Components.interfaces.nsIFile);
        if( file.exists() == true ) file.remove( false );
        strm = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
        strm.QueryInterface(Components.interfaces.nsIOutputStream);
        strm.QueryInterface(Components.interfaces.nsISeekableStream);
        strm.init( file, 0x04 | 0x08, 0700, 0 );
        strm.write( this.inkscapeFileList, this.inkscapeFileList.length );
        strm.write(quit, quit.length);
        strm.flush();
        strm.close();
        pathsfile = this.codeDir.clone(); 
        leaf = 'MSITeX.' + ext;
        pathsfile.append(leaf); // corresponds to $M in graphicsConversions.ini
        args = [pathsfile.path, this.baseDir.path];
        batchfile = this.codeDir.clone(); 
        batchfile.append('inkscapebatch.' + ext); 
        theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
        theProcess.init(batchfile);
        theProcess.run(true, args, args.length);
      }
      catch(ex)
      {
        dump(ex.message);
      } 
    }
    this.inkscapeScriptState = 0;
    this.inkscapeFileList = '';
    this.inkscapeBatching = false;
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
