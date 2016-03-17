Components.utils.import("resource://app/modules/graphicsConverter.jsm");
Components.utils.import("resource://app/modules/pathutils.jsm");

#include productname.inc

// const NS_IPCSERVICE_CONTRACTID  = "@mozilla.org/process/ipc-service;1";
// const NS_IPCBUFFER_CONTRACTID   = "@mozilla.org/process/ipc-buffer;1";
// const NS_PIPECONSOLE_CONTRACTID = "@mozilla.org/process/pipe-console;1";
// const NS_PIPETRANSPORT_CONTRACTID= "@mozilla.org/process/pipe-transport;1";
// const NS_PROCESSINFO_CONTRACTID = "@mozilla.org/xpcom/process-info;1";

//const fullmath = '<math xmlns="http://www.w3.org/1998/Math/MathML">';

Components.utils.import("resource://app/modules/macroArrays.jsm");
Components.utils.import("resource://app/modules/os.jsm");
var currPDFfileLeaf = "main.pdf"; // this is the leafname of the last pdf file generated.

function princeStartUp()
{
  // take out parts of the UI not needed on the Mac
  // protect these statements because some of these buttons don't exist in SNB
  var os = getOS(window);
  var button;
  var menuitem;
  if ('osx' == os)
  {
    if (button = document.getElementById("printPreviewButton")) button.hidden=true;
    if (button = document.getElementById("printPreviewMenuItem")) button.hidden=true;
  }
  else
  {
    if (button = document.getElementById("printPreviewButtonMac")) button.hidden = true;
    if (button = document.getElementById("printPreviewMenuItemMac")) button.hidden=true;
  }
  var prefs = GetPrefs();
  var wantdebug = prefs.getBoolPref("swp.debugtools");
  if (wantdebug)
  {
    if (menuitem = document.getElementById("venkmanName")) menuitem.hidden = false;
    if (menuitem = document.getElementById("menu_inspector")) menuitem.hidden = false;
  }

  msiEditorOnLoad();
}

// Some temporary functions to test how the program behaves when not licensed

function goAboutDialog() {
  window.openDialog("chrome://prince/content/aboutDialog.xul", "about", "modal,chrome,resizable=yes");
}

function getBrowser()
{
  alert("Get Browser!");
}

function getPPBrowser()
{
  return document.getElementById("preview-browser");
}


function GetCurrentEditor() {
  var editor;
  try {
	  var editorElement = msiGetActiveEditorElement();
	  editor = msiGetEditor(editorElement);
//    editor instanceof Components.interfaces.nsIPlaintextEditor;
//    editor instanceof Components.interfaces.nsIHTMLEditor;
  } catch (e) {
		throw ("Failure in GetCurrentEditor: \n" + e.message);
	}
  return editor;
}



function GetCurrentEditorElement() {
  var editorElement = msiGetActiveEditorElement();
  return editorElementƒ;
}

function doQuit() {
  var cancel = false;
  // call ShutdownAllEditors on all editing windows
  var wm=Components.classes["@mozilla.org/appshell/window-mediator;1"].getService();
  wm=wm.QueryInterface(Components.interfaces.nsIWindowMediator);
  var wlist=wm.getEnumerator(null);
  while (!cancel && wlist.hasMoreElements())
  {
    var w=wlist.getNext();
    if (w && ("ShutdownAllEditors" in w)) cancel |= w.ShutdownAllEditors();
  }
  if (!cancel)
  {
    var appStartup = Components.classes["@mozilla.org/toolkit/app-startup;1"].
                       getService(Components.interfaces.nsIAppStartup);

    appStartup.quit(Components.interfaces.nsIAppStartup.eAttemptQuit);
    return true;
  }
  return false;
}

/////////////////////////////////////////////////
// our connection to the computation code
//var compsample;
//var compengine;
//
//function GetCurrentEngine() {
//  if (!compsample) {
//    compsample = Components.classes["@mackichan.com/simplecomputeengine;2"].getService(Components.interfaces.msiISimpleComputeEngine);
//    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
//    var gmrfile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
//    gmrfile.append("mupInstall.gmr");
//    try {
//      compsample.startup(gmrfile.path);
//      compengine = 2;
//    } catch(e) {
//      var msg_key;
//      if (e.result == Components.results.NS_ERROR_NOT_AVAILABLE)
//        msg_key = "Error.notavailable";
//      else if (e.result == Components.results.NS_ERROR_FILE_NOT_FOUND)
//        msg_Key = "Error.notfound";
//      else if (e.result == Components.results.NS_ERROR_NOT_INITIALIZED)
//        msg_key = "Error.notinitialized";
//      else if (e.result == Components.results.NS_ERROR_FAILURE)
//        msg_key = "Error.failure";
//      else
//        throw e;
//      AlertWithTitle("Error.title", msg_key);
//  } }
//  return compsample;
//}


// jcs // return node on RHS of =, if structure is that simple
// jcs function GetRHS(math)
// jcs {
// jcs   var ch = last_child(math);
// jcs   while (ch) {
// jcs     if (ch.nodeType == Node.ELEMENT_NODE && ch.localName == "mo") {
// jcs       var op = ch.firstChild;
// jcs       if (op.nodeType == Node.TEXT_NODE && op.data == "=") {
// jcs         var m = node_after(ch);
// jcs         if (m)
// jcs           return m;
// jcs     } }
// jcs     ch = node_before(ch);
// jcs   }
// jcs   return math;
// jcs }

// function appendResult(result,sep,math) {
//   GetCurrentEditor().insertHTMLWithContext(
//       result.replace(fullmath,fullmath+sep),
//       "", "", "", null,
//       math, math.childNodes.length, false );
//   coalescemath();
// }

function focusOn(id) {
  var node = document.getElementById(id);
  if (node) node.focus();
}

function runFixup(math)
{
  try {
    var out = GetCurrentEngine().perform(math,GetCurrentEngine().Fixup);
    return out;
  }
	catch(e) {
		throw("Failure in RunFixup():\n"+ e.message);
  }
  return math;
}

function GetFixedMath(math)
{
  return runFixup(GetMathAsString(math));
}


//The function coalescemath is defined in computeOverlay.js

// form a single run of math and put caret on end
function coalescemath() {
  var editor = GetCurrentEditor();
  var selection = editor.selection;
  if (selection) {
    var f = selection.focusNode;
    var element = findmathparent(f);
    if (!element) {
      f = f.previousSibling;
      element = findmathparent(f);
      if (!element) {
	      msidump("focus not in math!\n");
        return;
      }
    }
    var last = node_before(element);
    if (!last || last.localName != "math") {
      msidump("previous is not math!\n");
      return;
    }
    var ch = element.firstChild;  // move children to previous math element
    var nextch;
    while (ch) {
      nextch = ch.nextSibling;
      last.appendChild(element.removeChild(ch));
      ch = nextch;
    }
    element.parentNode.removeChild(element);  // now empty

    editor.setCaretAfterElement(last_child(last));
  }
}



function findtagparent(node,tag) {
  if (!node || node.localName == tag)
    return node;
  else
    return findtagparent(node.parentNode,tag);
}

function findmathparent(node) {
  return findtagparent(node,"math");
  // really, should check namespace, too
}


// The following function is in ComputeUtils.js
// function GetMathAsString(math)
// {
//   var ser = new XMLSerializer();
//   var mathstr = ser.serializeToString(math);
//   if (math.localName != "math")
//     mathstr = "<math>" + mathstr + "</math>";
//   // risky string surgery, but it works in simple cases
//   mathstr = mathstr.replace(/ _moz_dirty=\"\"/g,"");
//   mathstr = mathstr.replace(/\<mi\/\>/g,"");
//   // the following namespace problems happen with inserted computation results...need a better solution
//   mathstr = mathstr.replace(/ xmlns:a0=\"http:\/\/www.w3.org\/1998\/Math\/MathML\"/g,"");
//   mathstr = mathstr.replace(/\<a0:/g,"\<");
//   mathstr = mathstr.replace(/\<\/a0:/g,"\<\/");
//   return mathstr;
// }

//////////////////////////////////////////////////////////////////
function is_all_ws( nod )
{
  return !(/[^\t\n\r ]/.test(nod.data));
}

function is_str_ws( s )
{
  return !(/[^\t\n\r ]/.test(s));
}

function is_ignorable( nod )
{
  return ( nod.nodeType == 8) || // A comment node
         ( (nod.nodeType == 3) && is_all_ws(nod) ); // a text node, all ws
}

function node_before( sib )
{
  while ((sib = sib.previousSibling)) {
    if (!is_ignorable(sib)) return sib;
  }
  return null;
}

function node_after( sib )
{
  while ((sib = sib.nextSibling)) {
    if (!is_ignorable(sib)) return sib;
  }
  return null;
}

function last_child( par )
{
  var res=par.lastChild;
  while (res) {
    if (!is_ignorable(res)) return res;
    res = res.previousSibling;
  }
  return null;
}

function first_child( par )
{
  var res=par.firstChild;
  while (res) {
    if (!is_ignorable(res)) return res;
    res = res.nextSibling;
  }
  return null;
}

function count_children( par )
{
  var res = 0;
  var c=par.firstChild;
  while (c) {
    if (!is_ignorable(c)) res++;
    c = c.nextSibling;
  }
  return res;
}

// quickly check if this file is a 5.5 subdocument. If so, there will be a line like
// %TCIDATA{LaTeXparent=0,0,masterfile.tex} in it.
function useMasterDocIfNeeded(file, strContents) {
  var regexp = /%TCIDATA{LaTeXparent=[0-9, ]*,([^}]*)}/;
  var match;
  var newfile;
  if (strContents && (strContents.length > 0)) {
    match = regexp.exec(strContents);
    if (match && match.length > 1 && match[1].length > 0) {
      newfile = file.parent;
      newfile.append(match[1]);
      return newfile;
    }
  }
  return file;
}
/*
     Prepare to run pretex.exe. We need to send it some directories:                                       //
      The input directory gives the location of the .cls and .tex files that pretex reads to               //
        determine how to translate the TeX. This is usually prince/ptdata.                                 //
      The MathML conversion directory. This is where the DLL used to convert math and its associated .gmr  //
        files are. This is usually resource://app.                                                                  //
      The input .tex file.                                                                                 //
      The output directory where the auxiliary files that are generated (such as .css, etc.) go.           //
      The output <filename>.sci file.
      */

function openTeX()
{
  var filename, infile, docdir, prefs, prefdir, defdocdirstring, dirkey, file;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
  var url;
  var fileURL;
  var str;
  fp.init(window, GetString("OpenTeXFile"), msIFilePicker.modeOpen);
  fp.appendFilter(GetString("TeXFiles"), "*.tex; *.ltx; *.shl");
  fp.appendFilters(msIFilePicker.filterXML);
  msiSetFilePickerDirectory(fp, "tex");
  try {
    fp.show();
  }
  catch (ex) {
  }
  if (fp.file && (fp.file.path.length > 0)) {
    msiSaveFilePickerDirectory(fp, "tex");
    url = msiGetIOService().newFileURI(fp.file);
    fileURL = url.spec;
    str = getTextFileAsString(fileURL);
    file = useMasterDocIfNeeded(fp.file, str);
    if (file !== fp.file) {
      url = msiGetIOService().newFileURI(file);
      fileURL = url.spec;
      str = getTextFileAsString(fileURL);
    }
    var pTeX = false;
    if (pTeX) file = convertToUTFIfNecessary(file, str);
    filename = file.leafName.substring(0,file.leafName.lastIndexOf("."));
    infile =  "\"" + file.path + "\"";
// Get the directory for the result from the preferences, or default to the SWPDocs directory
    try
    {
      prefs = GetPrefs();
      prefdir = prefs.getCharPref("swp.prefDocumentDir");
      if (prefdir && prefdir.length > 0) {
        docdirname = prefdir;
        // msidump("swp.prefDocumentDir is ", prefdir + "\n");
        docdir = Components.classes["@mozilla.org/file/local;1"].
            createInstance(Components.interfaces.nsILocalFile);
        docdir.initWithPath(docdirname);
        if (!valueOf(docdir.exists()))
          docdir.create(1, 0755);
      }
    }
    catch (e)
    {
      if (getOS(window) == "win")
        dirkey = "Pers";
      else
      if (getOS(window) =="osx")
        dirkey = "UsrDocs";
      else
        dirkey = "Home";
      // if we can't find the one in the prefs, get the default
      docdir = dsprops.get(dirkey, Components.interfaces.nsILocalFile);
      if (!docdir.exists()) docdir.create(1,0755);
      if (!prefdir || prefdir.length === 0) {
        prefdir = GetString("DefaultDocDir");
      }
      defdocdirstring = prefdir;
      docdir.append(defdocdirstring);
      if (!docdir.exists()) docdir.create(1,0755);
      msidump("default document directory is "+docdir.path+"\n");
    }

    var outdir = docdir.clone();
    outdir.append(filename + "_work");
    var outfile = outdir.clone();
    outfile.append("main.xhtml");
    try {
      if (outdir.exists()) outdir.remove(true);
      outdir.create(1 , 0755);
    }
    catch(e){
      3;
    }
    var css = outdir.clone();
    css.append("css");
    css.create(1 , 0755);
    var graphics = outdir.clone();
    graphics.append("graphics");
    graphics.create(1 , 0755);
    var plots = outdir.clone();
    plots.append("plots");
    plots.create(1 , 0755);
    if (outfile.exists()) outfile.remove(false);
    var mmldir = dsprops.get("resource:app", Components.interfaces.nsIFile);
    var exefile=dsprops.get("resource:app", Components.interfaces.nsIFile);
    var os = getOS(window);
    if (os == "win") {
      exefile.append("pretex.exe");
    } else {
      exefile.append("pretex");
    }

    var dataDir = dsprops.get("resource:app", Components.interfaces.nsIFile);
    dataDir.append("ptdata");
    msidump("\n\nExe="+exefile.path);
    msidump("\noutdir=\""+outdir.path);
    msidump("\noutfile=\""+outfile.path);
    msidump("\ninfile=\""+file.path);
    msidump("\ndataDir=\""+dataDir.path);
    msidump("\nmmldir=\""+mmldir.path+"\n");
    msidump("\nargs =['-i', "+dataDir.path+", '-f', 'latex2xml.tex', '-o', "+outdir.path+", '-m',"+ mmldir.path+", "+file.path+", "+outfile.path);

    // run pretex.exe
    dump("+++ Running");
    try
    {
	var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
	theProcess.init(exefile);
	var args =['-i', dataDir.path, '-f', 'latex2xml.tex', '-o', outdir.path, '-m', mmldir.path, file.path, outfile.path];
	
	dump("+++" + args);
      theProcess.run(true, args, args.length);
    }
    catch (ex)
    {
       //   msidump("\nUnable to open TeX:\n");
		     // msidump("\nexe  = "  + exefile);
       //   msidump("\narg paths = " + dataDir.path + "\n   " + file.path + "\n    " + outfile.path + "\n     " + outdir.path);
       //   msidump(ex+"\n");
    }

//  TODO BBM todo: we may need to run a merge program to bring in processing instructions for specifying tag property files
    if ((outfile) && (outfile.path.length > 0))
    {
      if (!outfile.exists()) finalThrow(cmdFailString('Import TeX'),'Conversion program did not produce a file');
      var xsltProcessor = setupInputXSLTproc();
      var parser = new DOMParser();
      var xmlDoc = document.implementation.createDocument("", "", null);
      xmlDoc.async = false;
      var url = msiFileURLFromFile(outfile);
      var request = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance();
      request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
      request.open("GET", url.spec, false);
      request.send(null);
      var res = request.responseXML;
      var newDoc = xsltProcessor.transformToDocument(res);

      var outputStream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
      var fileMode = 0x22;  //MODE_WRONLY|MODE_TRUNCATE - rewrite the file
      var permissions = 0x777; //all permissions for everybody?
      outputStream.init(outfile, fileMode, permissions, 0);
      var serializer = new XMLSerializer();
      serializer.serializeToStream(newDoc, outputStream, "utf-8");
      outputStream.close();

      msiEditPage(url, window, false, false);
    }
  }
}

function openTeXbyName(fn)
{
    var filename, infile, docdir, prefs, prefdir, defdocdirstring, dirkey, file;
    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
    var inputfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
    var outputfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);

    inputfile.initWithPath( fn ); // outputfile now points to our document file
     
    inputfile = useMasterDocIfNeeded(inputfile);

    msidump("Input file is " + inputfile);
    filename = inputfile.leafName.substring(0,inputfile.leafName.lastIndexOf("."));

    var outdir = inputfile.parent;
    outdir.append(filename + "_work");
    try {
	msidump("Output dir is " + outdir);
	if (outdir.exists()){
	    msidump("Output dir exists");
	    outdir.remove(true);
	    msidump("Output dir removed")
	}
	outdir.create(1 , 0755);
	msidump("Output dir created");
    }
    catch(e){
	msidump("Could not remove output directory " + outdir);
	msidump(e);
    }
    
    
    var outfile = outdir.clone();
    outfile.append("main.xhtml");
    msidump("Output file is " + outfile);
    var css = outdir.clone();
    css.append("css");
    css.create(1 , 0755);
    var graphics = outdir.clone();
    graphics.append("graphics");
    graphics.create(1 , 0755);
    var plots = outdir.clone();
    plots.append("plots");
    plots.create(1 , 0755);
    var mmldir = dsprops.get("resource:app", Components.interfaces.nsIFile);
    var exefile=dsprops.get("resource:app", Components.interfaces.nsIFile);
    var os = getOS(window);
    if (os == "win") {
      exefile.append("pretex.exe");
    } else {
      exefile.append("pretex");
    }

    var dataDir = dsprops.get("resource:app", Components.interfaces.nsIFile);
    dataDir.append("ptdata");
    msidump("\n\nExe="+exefile.path);
    msidump("\noutdir=\""+outdir.path);
    msidump("\noutfile=\""+outfile.path);
    msidump("\ninfile=\""+inputfile.path);
    msidump("\ndataDir=\""+dataDir.path);
    msidump("\nmmldir=\""+mmldir.path+"\n");
    msidump("\nargs =['-i', "+dataDir.path+", '-f', 'latex2xml.tex', '-o', "+outdir.path+", '-m',"+ mmldir.path+", "+inputfile.path+", "+outfile.path);

    // run pretex.exe

    try
    {
	msidump("Running pretex");
	var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
	theProcess.init(exefile);
	var args =['-i', dataDir.path, '-f', 'latex2xml.tex', '-o', outdir.path, '-m', mmldir.path, inputfile.path, outfile.path];
	
	msidump("+++" + args);
        theProcess.run(true, args, args.length);
    }
    catch (ex)
    {
	msidump("Could not run pretex. " + ex);
    }
    if ((outfile) && (outfile.path.length > 0))
    {
	if (!outfile.exists())
	    finalThrow(cmdFailString('Import TeX'),
		       'Conversion program did not produce a file');
    }
    var xsltProcessor = setupInputXSLTproc();
    var parser = new DOMParser();
    var xmlDoc = document.implementation.createDocument("", "", null);
    xmlDoc.async = false;
    var url = msiFileURLFromFile(outfile);
    var request = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"].createInstance();
    request.QueryInterface(Components.interfaces.nsIXMLHttpRequest);
    request.open("GET", url.spec, false);
    request.send(null);
    var res = request.responseXML;
    var newDoc = xsltProcessor.transformToDocument(res);
    var outputStream = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
    var fileMode = 0x22;  //MODE_WRONLY|MODE_TRUNCATE - rewrite the file
    var permissions = 0x777; //all permissions for everybody?
    outputStream.init(outfile, fileMode, permissions, 0);
    var serializer = new XMLSerializer();
    serializer.serializeToStream(newDoc, outputStream, "utf-8");
    outputStream.close();

    //msiEditPage(url, window, false, false);
    var editorElement = msiGetTopLevelEditorElement();
    var outTeXfile = outdir.append("main.tex");
    var compileInfo = new Object();
    documentAsTeXFile( editorElement, newDoc, outTeXfile, compileInfo );

    
  }



function runTests()
{
    openTeXbyName("C:\\Users\\Jon\\tester\\docs\\checkout.tex");
}


function setupInputXSLTproc()
{
  var xslFileURL = "chrome://ptprince/content/ptprince.xsl";
  var xsltStr = getXSLAsString(xslFileURL);
  var xsltProcessor = new XSLTProcessor();

  try
  {
    var parser = new DOMParser();
    var xslDoc = parser.parseFromString(xsltStr, "text/xml");
    xsltProcessor.importStylesheet(xslDoc);
  }
  catch(e)
  { msidump("error: " + e + "\n"); }

  return xsltProcessor;
}


#define INTERNAL_XSLT

// documentAsTeXFile writes the document out as TeX.
// Returns true if the TeX file was created.
// outTeXfile is an nsILocalFile. If it is null, create main.tex in the document's 'tex' directory.

function documentAsTeXFile( editor, document, outTeXfile, compileInfo )
{
  msidump("\nDocument as TeXFile\n");
  if (!document) return false;
  // determine the compiler
  var compiler = "pdflatex";
  // Determine which compiler to use
  var texprogNode;
  var texprogNodes = document.getElementsByTagName("texprogram");
  if (texprogNodes.length > 0)
  {
    texprogNode = texprogNodes[0];
    if (texprogNode.hasAttribute("prog")) compiler = texprogNode.getAttribute("prog");
  }

  var xslfiles = processingInstructionsList(document, "sw-xslt", false);
  //  if nothing returned, use the default xlt
  if (xslfiles.length < 1) xslfiles = ["latex.xsl"];
  var xslSheet=xslfiles[0];
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var documentPath = document.documentURI;
  var docurl = msiURIFromString(documentPath);
  var workingDir;
	var parentDir;
  var outTeX;
  var isDefaultLocation = false;
  workingDir = msiFileFromFileURL(docurl);
  var bareleaf = workingDir.leafName; // This is the leaf of the document.
  workingDir = workingDir.parent;
  if (outTeXfile == null  || outTeXfile.path.length == 0)
  {
    isDefaultLocation = true;
		outTeXdir = workingDir.clone();
    outTeXdir.append("tex");
    // try {
    //   outTeXdir.remove(true);
    // }
    // catch(e) {
    //   AlertWithTitle("Error", "Unable to delete working TeX directory.\nClose any applications that may have the tex or pdf files open.");
    //   return false;
    // }
    if (!outTeXdir.exists()) outTeXdir.create(1, 0755);
    outTeXfile = outTeXdir;
    outTeXfile.append(bareleaf.replace(/\.xhtml$/,"") + ".tex");
  }
  var outfileTeXPath = outTeXfile.path;
  var stylefile;
  var xslPath = "chrome://prnc2ltx/content/"+xslSheet;
  var str = documentToTeXString(document, xslPath);
  var pTeX = false;
  if (pTeX) {
    try {
      var charset = 'Shift_JIS';
      var unicodeConverter = Components
           .classes["@mozilla.org/intl/scriptableunicodeconverter"]
           .createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
      unicodeConverter.charset = charset;
      str = unicodeConverter.ConvertFromUnicode(str);
      str + unicodeConverter.Finish();
    } catch(ex) {
        return null; 
    }   
  }
  compileInfo.runMakeIndex = /\\printindex/.test(str);
  compileInfo.runBibTeX = /\\bibliography/.test(str);
  compileInfo.passCount = 1;
  var runcount = 1;
  if (RegExp(/\\tableofcontents|\\listoffigures|\\listoftables|\\includemovie/).test(str)) runcount = 3;
  if (compileInfo.passCount < runcount) compileInfo.passCount = runcount;
  if (RegExp(/\\ref|\\xref|\\pageref|\\vxref|\\vref|\\vpageref|\\cite/).test(str)) runcount = 2;
  if (compileInfo.passCount < runcount) compileInfo.passCount = runcount;
  var minpasses = 1;
  var index;
  var minpassElements = document.getElementsByTagName("button");
  if (minpassElements.length > 0) {
    for (index = 0; index < minpassElements.length; index++) {
      if (minpassElements[index].hasAttribute('minpasses')) {
        minpasses = minpassElements[index].getAttribute('minpasses');
        break;
      }
    }
  }

  if (compileInfo.runMakeIndex || compileInfo.runBibTeX) {
    if (compileInfo.passCount < 3) compileInfo.passCount = 3;
  }
  if (compileInfo.passCount < minpasses) compileInfo.passCount = minpasses;


//  msidump("\n"+str);
  if (!str || str.length < 3) return false;
  // if isDefaultLocation is false, the graphics path in the TeX is incorrect. We need to adjust this.
  if (!isDefaultLocation)
	{
		// save graphics, gcache, tcache and plot directories
    var prefix = outTeXfile.leafName.replace(/\.tex$/i,"_");
		var specialDirs = ["plots","graphics","tcache","gcache"];
		var k;
		for (k = 0; k < specialDirs.length; k++)
		{
      str = str.replace("{../"+specialDirs[k]+"/", "{"+prefix+specialDirs[k]+"/", "g");
		}
	}

  if (outTeXfile.exists())
    outTeXfile.remove(false);
  outTeXfile.create(0, 0755);
  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(outTeXfile, -1, -1, false);
  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
    .createInstance(Components.interfaces.nsIConverterOutputStream);
  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
  os.writeString(str);
  os.close();
  fos.close();
  if (outTeXfile.exists() && !isDefaultLocation)
	{
		// save graphics, gcache, tcache and plot directories
		var parentDir = outTeXfile.parent.clone();
		var s,d; // source and destination directories.
		for (k = 0; k < specialDirs.length; k++)
		{
			s = workingDir.clone();
			s.append(specialDirs[k]);
			d = parentDir.clone();
			d.append(prefix+specialDirs[k]);
			if (s.exists())
			{
				// copy files from s to d
				var entries = s.directoryEntries;
				if (entries.hasMoreElements())
				{
				  if (!d.exists())
					  d.create(1,0755);
  				while(entries.hasMoreElements())
  				{
  				  var entry = entries.getNext();
  				  entry.QueryInterface(Components.interfaces.nsIFile);
  				  entry.copyTo(d,"");
  				}
  			}
			}
		}
	}
	return outTeXfile.exists();
}

function currentFileName()
{
//  var docUrl = GetDocumentUrl();
  var editorElement = msiGetTopLevelEditorElement();
  var docUrl = msiGetEditorURL(editorElement);
  var filename = "";
  if (docUrl && !IsUrlAboutBlank(docUrl))
    filename = GetFilename(docUrl);
  return filename;
}



function exportTeX()
{
  if (!okToPrint()) {
   finalThrow(cmdFailString("exporttotex"), "Exporting TeX for a modified document is not allowed since this program is not licensed.")
   return;
  }

  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor) return;
#ifndef PROD_SW
  rebuildSnapshots(editor.document);
#endif
  uri = editor.document.documentURI;
  checkPackageDependenciesForEditor(editor);
  var file = msiFileFromFileURL(msiURIFromString(uri));
  var fileName = file.parent.leafName.replace(/_work$/i, ".tex");
  fileName = fileName.replace(" ", "_");
  var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
  fp.init(window, "Export TeX File as...", msIFilePicker.modeSave);
  fp.defaultExtension = "tex";
  fp.defaultString=fileName;
  fp.appendFilter("TeX file",".tex");
  var compileInfo = new Object();
  var dialogResult = fp.show();

  if (dialogResult != msIFilePicker.returnCancel)
    if (!documentAsTeXFile(editor, editor.document, fp.file, compileInfo ))
      throw("TeX file not created");
}


function exportToWeb()
{
  if (!okToPrint()) {
   finalThrow(cmdFailString("exporttoweb"), "Exporting to the web for a modified document is not allowed since this program is not licensed.")
   return;
  }
  if (currentFileName().length < 0) return;
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor) return;

   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
   fp.init(window, "Export to Web", msIFilePicker.modeSave);
   fp.appendFilter("Complete zip file ", "*.zip");
   fp.appendFilter("Zip file with web refs for CSS", "*.zip");
   fp.appendFilter("Zip file with MathJax", "*.zip");
   fp.appendFilter("Zip file with web refs for CSS and MathJax", "*.zip");
     try
   {
     var dialogResult = fp.show();
     if (dialogResult != msIFilePicker.returnCancel)
       if (!saveforweb(editor.document, fp.filterIndex, fp.file ))
         AlertWithTitle("Export", "Web file not created.");
   }
   catch (ex)
   {
     msidump("filePicker threw an exception in exportToWeb: "+ex.message+"\n");
   }
}


/* ==== */
/* =
compileTeXFile:
  compiler -- a string, either 'pdflatex' or 'xelatex', giving which compiler to use.
  infileLeaf -- the name of the input TeX file without '.tex' or the initial part of the path
  infilePath -- the full name of the input TeX file, including the path and 'tex'
  outputDir -- the directory in which to put the resulting file
  compileInfo -- an object for storing the required # of passes, whether makeindex needs
       to be called, etc.

  returns -- a boolean to indicate whether the expected file appears where it is supposed to

 = */
/* ==== */

// function setBibTeXRunArgs(passData)
// {
//   var bibTeXDBaseDir = GetLocalFilePref("swp.bibtex.dir");
//   var bibTeXDPath;
//   var bibtexData;
//   if (bibTeXDBaseDir)
//     bibtexData = [passData.args[0],"-d", bibTeXDBaseDir.path ];
//   else
//     bibtexData = [passData.args[0]];
//   return bibtexData;
// }

function removeOldPDFFiles(outputDir)
{
  var thedir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  thedir.initWithPath( outputDir );
  var items = thedir.directoryEntries;
  while (items.hasMoreElements()) {
    var item = items.getNext().QueryInterface(Components.interfaces.nsIFile);
    if (item.isFile() && item.leafName.indexOf("SWP") === 0)
    {
      try {
        item.remove(false);
      }
      catch (e) {
        // do nothing
      }
    }
  }
}

function compileTeXFile( compiler, infileLeaf, infilePath, outputDir, compileInfo, callbackFn )
{
  // the following requires that the pdflatex program (or a hard link to it) be in TeX/bin/pdflatex
  if (!okToPrint()) {
    finalThrow(cmdFailString("compiletex"), "Compiling a modified TeX file is not permitted since this program is not licensed.");
    return false;
  }
  var passData;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var exefile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
  var indexexe = exefile.clone();
  var bibtexexe = exefile.clone();
  var msitexpaths = exefile.clone();
  var extension;
  // A word on file names. Essentially what we want to do is to compile main.tex to main.pdf.
  // Unfortunately, the Acrobat plugin keeps a lock on the file it is displaying in the preview pane, so
  // compiling to main.pdf will frequently fail.
  //
  // The strategy: always compile to main.pdf, and then try renaming it to SWP.pdf, or SWP0.pdf, or SWP1.pdf, or ...
  // The final leafname is returned in compileInfo, for use of the routines that display the pdf file and stored in the global
  // currPDFfileLeaf, where it is used to display the pdf when changes have not been made to the document.
  //
  passData = new Object;
  var os = getOS(window);
  if (os == "win") extension = "cmd";
  else extension = "bash";
  exefile.append(compiler+"."+extension);
  indexexe.append("makeindex."+extension);
  bibtexexe.append("runbibtex." + extension);
  msitexpaths.append("MSITeX." + extension);
  removeOldPDFFiles(outputDir);
  passData.file = exefile;
  passData.indexexe = indexexe;
  passData.bibtexexe = bibtexexe;
  passData.outputDir = outputDir;
  passData.args = [msitexpaths.path, outputDir, infileLeaf, "x", "x"];
  passData.passCount = compileInfo.passCount;
  passData.runMakeIndex = compileInfo.runMakeIndex;
  passData.runBibTeX = compileInfo.runBibTeX;
  // if (passData.runBibTeX)
  //   passData.bibtexArgs = setBibTeXRunArgs(passData);

  var i;
  // initForSilentCompile(passData, compileInfo, false, callbackFn);
  window.openDialog("chrome://prince/content/passes.xul","about", "chrome,resizable=yes,alwaysRaised=yes",
    passData, compileInfo, true, callbackFn);
//    There was some commented code here for using the pipe-console object from the enigmail project. We are not
//    using it in 6.0, and XulRunner is getting a better implementation, which we will use later.

}


function printPDFFile(infile)
{
  // the following requires that the printpdf batch file (or a hard link to it) be in xpi-stage/prince/TeX/bin/printpdf.cmd
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var exefile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
  exefile.append("printpdf.cmd");
  msidump("\nexecutable file: "+exefile.path+"\n");
  try
  {
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(exefile);
    var args = [infile.path];
    theProcess.run(true, args, args.length);
  }
  catch (ex) {
    msidump("\nUnable to run Acrobat: "+ex.message+"\n");
    return false;
  }
}

// compileDocument compiles the current document of the current editor; it converts it to TeX and then PDF.
// Returns true if everything succeeded.
function compileDocument(preview)
{
  if (!okToPrint()) {
    finalThrow(cmdFailString("compiletex"), "Compiling a modified TeX file is not permitted since this program is not licensed.");
    return false;
  }

  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (!editor) return null;
#ifndef PROD_SW
  rebuildSnapshots(editor.document);
#endif
  var compiler = "pdflatex";
  // Determine which compiler to use
  var texprogNode;
  var texprogNodes = editor.document.getElementsByTagName("texprogram");
  if (texprogNodes.length > 0)
  {
    texprogNode = texprogNodes[0];
    if (texprogNode.hasAttribute("prog")) compiler = texprogNode.getAttribute("prog");
  }
  checkPackageDependenciesForEditor(editor);
  var pdfViewer = document.getElementById("preview-frame");
  if (pdfViewer && (pdfViewer.src != "about:blank"))
    pdfViewer.loadURI("about:blank");   // this releases the currently displayed pdf preview.
  msidump("pdfModCount = "+editorElement.pdfModCount+", modCount is ");

  editorElement.pdfModCount = editor.getModificationCount();
  msidump(editorElement.pdfModCount+"\n");
  try {
    var docUrl = msiGetEditorURL(editorElement);
    var docPath = GetFilepath(docUrl);
    var workingDir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
    var outputfile;
      // for Windows
#ifdef XP_WIN32
      docPath = docPath.replace("/","\\","g");
#endif
    workingDir.initWithPath( docPath ); // workingDir now points to our document file
    workingDir = workingDir.parent; // and now it points to the working directory
    outputfile = workingDir.clone();
    outputfile.append("tex");
    // remove and create the tex directory to clean it out
    try {
      if (outputfile.exists()) outputfile.remove(true);
//      if (outputfile.exists()) AlertWithTitle("Locked file","the tex directory was not deleted");
      outputfile.create(1, 0755);
    }
    catch(e) {} //
    var pdffile = outputfile.clone();
    outputfile.append("main.tex");
    try  {
      if (outputfile.exists()) outputfile.remove(false);
    }
    catch(e){}

    msidump("TeX file="+outputfile.path + "\n");
//    msidump("PDF file is " + pdffile.path + "\n");
    var compileInfo = new Object();  // an object to hold pass counts and whether makeindex needs to run.
    if (documentAsTeXFile(editor, editor.document, null, compileInfo ))
      compileTeXFile(compiler, "main", outputfile.path, pdffile.path, compileInfo, preview ? previewPDFFile : printPDFFile);
    else
    {
      AlertWithTitle("XSLT Error", "Unable to create a TeX file");
      return null;
    }
  }
  catch(e)
  {
    msidump("compileDocument failed: "+e.message+"\n");
    return null;
  }
}


function printTeX(preview )
{
  if (!okToPrint()) {
    finalThrow(cmdFailString("printtex"), "Compiling a modified TeX file is not permitted since this program is not licensed.");
    return false;
  }
  try {
    compileDocument(preview);
  }
  catch(e) {
    msidump("printTeX failed: "+e.message+"\n");
  }
}

function previewTeX()
{
  if (!okToPrint()) {
    finalThrow(cmdFailString("printtex"), "Compiling a modified TeX file is not permitted since this program is not licensed.");
    return false;
  }
  printTeX(true);
};


function compileTeX(compiler)
{
  try
  {
    var editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    if (!editor) return;

    document.getElementById("preview-frame").loadURI("about:blank");
  // now save this TeX string and run TeX on it.
    checkPackageDependenciesForEditor(editor);
    var docUrl = msiGetEditorURL(editorElement);
    var docPath = GetFilepath(docUrl);
    var outputfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  // for Windows
#ifdef XP_WIN32
    docPath = docPath.replace("/","\\","g");
#endif
    outputfile.initWithPath( docPath ); // outputfile now points to our document file
    var outleaf = outputfile.leafName;
    outleaf = outleaf.substr(0, outleaf.lastIndexOf("."));
    outputfile = outputfile.parent;
    outputfile.append(outleaf + "_files");
    if (!outputfile.exists()) outputfile.create(1, 0755);
    outputfile.append("tex");
    if (!outputfile.exists()) outputfile.create(1, 0755);
    var pdffile = outputfile.clone();
    outputfile.append(outleaf+".tex");
    if (outputfile.exists()) outputfile.remove(false);
    pdffile.append(outleaf + ".pdf");
    if (pdffile.exists()) pdffile.remove(false);

    msidump("TeX file="+outputfile.path)+"\n";
    var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
    fp.init(window, "Save PDF file", msIFilePicker.modeSave);

    fp.appendFilter("Compiled files","*.pdf");
    fp.appendFilters(msIFilePicker.filterAll);

    try
    {
      fp.show();
      // need to handle cancel (uncaught exception at present)
    }
    catch (ex)
    {
      msidump("filePicker threw an exception\n");
      return;
    }

    var compileInfo = new Object();
    documentAsTeXFile(editor, editor.document, null, compileInfo);
    if (!outputfile.exists())
    {
      AlertWithTitle("XSLT Error", "Unable to generate TeX file");
    } else
    {
      compileTeXFile(compiler, outleaf, outputfile.path, pdffile.parent.path, compileInfo, function (pdffile) {
        if (!pdffile.exists())
        {
          AlertWithTitle("TeX Error", "No PDF file was created because of errors");
          msiGoDoCommand("cmd_showTeXErrors");
        }
        else
        // move the output result to the place indicated by fp.
          pdffile.move(fp.file.parent, fp.file.leafName);
      });
    }
  }
  catch(e) {
    msidump(e+"\n");
    return;
  }
}

function copyToExternalLocation( pdffile ) {
// 
  var destdirectory = pdffile.parent.parent.parent;
  var leafname = pdffile.parent.parent.leafName;
  var n = 2;
  leafname = leafname.replace(/_work$/,'');
  var newfile = destdirectory.clone();
  newfile.append(leafname+'.pdf');
  try {
    if (newfile.exists()) {
      newfile.remove(false);
    }
  }
  catch (e) {
    while (newfile.exists()) {
      newfile = newfile.parent;
      newfile.append(leafname + '_' + n + '.pdf');
      n++;
    }
  }
  pdffile.copyTo(destdirectory, newfile.leafName);
}

function previewPDFFile( pdffile ) {
  previewOrPrintPDFFile( pdffile, true );
}

function printPDFFile( pdffile ) {
  previewOrPrintPDFFile( pdffile, false );
}




function previewOrPrintPDFFile( pdffile, preview ) {
  if (!pdffile.exists())
  {
    AlertWithTitle("TeX Error", "Unable to create a PDF file.");
    msiGoDoCommand("cmd_showTeXErrors");
  }
  else
  {
    copyToExternalLocation(pdffile);
    currPDFfileLeaf = pdffile.leafName;
    pdffile = copyFileToTmp( pdffile );

    if (preview)
    {
    // get prefs for viewing pdf files
      var prefs = GetPrefs();
      var pdfAction = prefs.getCharPref("swp.prefPDFPath");
      if (pdfAction == "default")
      {
        try {
          document.getElementById("preview-frame").loadURI(msiFileURLStringFromFile(pdffile));
        }
        catch (e) {
          return;
        }
        // Switch to the preview pane (third in the deck)
        msiGoDoCommand("cmd_PreviewMode");
      }
      else
      {
        var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
        var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
        var extension;
        var exefile;
        var arr = new Array();
        if (pdfAction == "launch")
        {
          var os = getOS(window);
          if (os == "win")
          {
            extension = "cmd";
          }
          else
          {
            extension = "bash";
          }
          exefile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
          exefile.append("shell."+ extension);
          theProcess.init(exefile);
          arr = [pdffile.parent.path, pdffile.leafName];
          theProcess.run(false, arr, arr.length);
        }
        else // pdfAction == complete path to viewer
        {
          exefile = Components.classes["@mozilla.org/file/local;1"].
                  createInstance(Components.interfaces.nsILocalFile);
          exefile.initWithPath(pdfAction);
          arr=[pdffile.path];
          theProcess.init(exefile);
          theProcess.run(false, arr, arr.length);
        }
      }
    }
    else
      printPDFFile(pdffile);
  }
}

// |forceOpen| is a bool that indicates that the sidebar should be forced open.  In other words
// the toggle won't be allowed to close the sidebar.
function toggleSidebar(aCommandID, forceOpen) {

  var sidebarBox = document.getElementById("sidebar-box");
  if (!aCommandID)
    aCommandID = sidebarBox.getAttribute("sidebarcommand");

  var elt = document.getElementById(aCommandID);  // elt is a broadcaster object
  var sidebar = document.getElementById("sidebar"); // a deck
  var sidebarTitle = document.getElementById("sidebar-title-box");
  var sidebarSplitter = document.getElementById("sidebar-splitter");

  if (!forceOpen && elt.getAttribute("checked") == "true") {
    elt.removeAttribute("checked");
    sidebarBox.setAttribute("sidebarcommand", "");
    sidebarTitle.setAttribute("label", "");
    sidebarBox.hidden = true;
    sidebarSplitter.hidden = true;
    content.focus();
    return;
  }

  var elts = document.getElementsByAttribute("group", "sidebar");
  for (var i = 0; i < elts.length; ++i)
    elts[i].removeAttribute("checked");

  elt.setAttribute("checked", "true");;

  if (sidebarBox.hidden) {
    sidebarBox.hidden = false;
    sidebarSplitter.hidden = false;
  }

  var url = elt.getAttribute("sidebarurl");
  var title = elt.getAttribute("sidebartitle");
  if (!title)
    title = elt.getAttribute("label");
  var i;
  var deckpaneltoshow;
  var nodelist = sidebar.childNodes;
  for (i = 0; i < nodelist.length; i++)
    if (nodelist.item(i).id == url)
      sidebar.selectedIndex = i;
  sidebarBox.setAttribute("sidebarcommand", elt.id);
  sidebarTitle.setAttribute("label", title);
}

function showSidebar(sidebarnum, checkbox, forceOpen)
{
  var sidebar=document.getElementById("sidebar"+sidebarnum);
  if (sidebar == null) return;
  var splitter=document.getElementById("splitter"+sidebarnum);
  if (splitter == null) return;
  if (checkbox.getAttribute("checked") === "true") {
    // show the sidebar
    sidebar.removeAttribute("hidden");
    // splitter.removeAttribute("hidden");
  }
  else
  {
    sidebar.setAttribute("hidden", "true");
    // splitter.setAttribute("hidden", "true");
  }
}

function setStatusBarVisibility()
{
  var currentstate;
  currentstate=document.getElementById("viewstatusbar").getAttribute("checked");
  var statusbar = document.getElementById("status-bar");
  if (currentstate=="true")
  {
    statusbar.setAttribute("hidden","false");
  }
  else statusbar.setAttribute("hidden","true");
}

function switchFocus(elem)
{
  var mediator = Components.classes["@mozilla.org/rdf/datasource;1?name=window-mediator"].getService();
  mediator.QueryInterface(Components.interfaces.nsIWindowDataSource);

  var resource = elem.getAttribute('id');
  var switchwindow = mediator.getWindowForResource(resource);

  if (switchwindow){
    switchwindow.focus();
  }
}
const nsIWindowMediator = Components.interfaces.nsIWindowMediator;

function toOpenWindow( aWindow )
{
  try {
    // Try to focus the previously focused window e.g. message compose body
    aWindow.document.commandDispatcher.focusedWindow.focus();
  } catch (e) {
    // e.g. full-page plugin or non-XUL document; just raise the top window
    aWindow.focus();
  }
}

function toOpenWindowByType( inType, uri )
{
  // Recently opened one.
  if (uri in window)
    return;

  var windowManager = Components.classes['@mozilla.org/appshell/window-mediator;1'].getService(nsIWindowMediator);

  var topWindow = windowManager.getMostRecentWindow( inType );

  if ( topWindow )
    toOpenWindow( topWindow );
  else
  {
    function newWindowLoaded(event) {
      delete window[uri];
    }
    window[uri] = window.openDialog(uri, "", "all,dialog=no");
    window[uri].addEventListener("load", newWindowLoaded, false);
  }
}

function documentToTeXString(document, xslPath)
{
  if (!okToPrint()) {
    finalThrow(cmdFailString("totexstring"), "Compiling a modified TeX file is not permitted since this program is not licensed.");
    return false;
  }
  var xsltString = "";
  var strResult = "";
  var editorElement = msiGetActiveEditorElement();
  editor = msiGetEditor(editorElement);
  xsltString = getXSLAsString(xslPath);
  var compiler = "pdflatex";
  // Determine which compiler to use
  var texprogNode;
  var texprogNodes = document.getElementsByTagName("texprogram");
  if (texprogNodes.length > 0)
  {
    texprogNode = texprogNodes[0];
    if (texprogNode.hasAttribute("prog")) compiler = texprogNode.getAttribute("prog");
  }


  var xsltProcessor = new XSLTProcessor();

  try{
    var parser = new DOMParser();
    var doc = parser.parseFromString(xsltString, "text/xml");
    xsltProcessor.importStylesheet(doc);
    var newDoc = xsltProcessor.transformToDocument(document);
    strResult = newDoc.documentElement.textContent || "";

    strResult=strResult.replace(/(\s*)\n(\s*)/g,'\n');
    strResult=strResult.replace(/\n{2,}/g, '\n');
    strResult=strResult.replace(/\n*(\\MsiNewline)\n*/g, '\\MsiNewline');
    strResult=strResult.replace(/(\\MsiNewline)+/g, "\n");
    strResult=strResult.replace(/(\\MsiBlankline)+/g, "\n\n");


//     while (strResult.search(/\n[ \t]+/) >= 0)
// 		  strResult = strResult.replace(/\n[ \t]+/,"\n","g");
//     while (strResult.search(/\n\n/) >= 0)
//       strResult = strResult.replace(/\n\n/,"\n","g");
// 	  while (strResult.search(/\\par[ \t]*\n/) >= 0)
// 		  strResult = strResult.replace(/\\par[ \t]*\n/,"\n\n", "g");
//
//     while (strResult.search(/\\par[ \t\n]+/) >= 0)
// 		  strResult = strResult.replace(/\\par[ \t\n]+/,"\n\n", "g");
//     while (strResult.search(/\\msipar[ \t\n]+/) >= 0)
// 		  strResult = strResult.replace(/\\msipar([ \t\n]+)/,"\\par$1", "g");
		//while (strResult.search(/\\par/) >= 0)
	//  strResult = strResult.replace(/\\par/,"\n\n", "g");    {
      strResult = editor.filterCharsForLaTeX(compiler !== 'pdflatex', strResult);

  }
  catch(e){
    msidump("error: "+e.message+"\n\n");
  //  msidump(resultString);
  }
  return strResult;
}


function openBrowser(url) {
  var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var extension;
  var exefile;
  var arr = [];
  arr.push('.');
  var os = getOS(window);
  if (os == "win")
  {
    extension = "cmd";
    arr.push('start');
  }
  else
  {
    extension = "bash";
    // arr.push('open');
  }
  exefile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
  exefile.append("shell."+ extension);
  theProcess.init(exefile);
  arr.push(url);
  theProcess.run(false, arr, arr.length);
}



function msidumpDocument()
{
  var ed= GetCurrentEditor();
  var root = ed.document.documentElement;
  ed.debugmsidumpContent(root);
}
