const NS_IPCSERVICE_CONTRACTID  = "@mozilla.org/process/ipc-service;1";
const NS_IPCBUFFER_CONTRACTID   = "@mozilla.org/process/ipc-buffer;1";
const NS_PIPECONSOLE_CONTRACTID = "@mozilla.org/process/pipe-console;1";
const NS_PIPETRANSPORT_CONTRACTID= "@mozilla.org/process/pipe-transport;1";
const NS_PROCESSINFO_CONTRACTID = "@mozilla.org/xpcom/process-info;1";

var gPipeConsole;
var currPDFfileLeaf = "main.pdf";
Components.utils.import("resource://app/modules/macroArrays.jsm");

function goAboutDialog() {
  window.openDialog("chrome://prince/content/aboutDialog.xul", "about", "modal,chrome,resizable=yes");
}

function getBrowser()
{
  alert("Get Browser!");
}

function GetCurrentEditor() {
  var editor;
  try {
    var editorElement = GetCurrentEditorElement();
    editor = editorElement.getEditor(editorElement.contentWindow);

    editor instanceof Components.interfaces.nsIPlaintextEditor;
    editor instanceof Components.interfaces.nsIHTMLEditor;
  } catch (e) { dump (e)+"\n"; }
                                                                                                     
  return editor;
}

function GetCurrentEditorElement() {
  var tmpWindow = window;
  
  do {
    var editorList = tmpWindow.document.getElementsByTagName("editor");
    // Doesn't this assume one editor per window?? --BBM
    if (editorList.item(0))
      return editorList.item(0);

    tmpWindow = tmpWindow.opener;
  } while (tmpWindow);

  return null;
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

//const fullmath = '<math xmlns="http://www.w3.org/1998/Math/MathML">';

// return node on RHS of =, if structure is that simple
function GetRHS(math)
{
  var ch = last_child(math);
  while (ch) {
    if (ch.nodeType == Node.ELEMENT_NODE && ch.localName == "mo") {
      var op = ch.firstChild;
      if (op.nodeType == Node.TEXT_NODE && op.data == "=") {
        var m = node_after(ch);
        if (m)
          return m;
    } }
    ch = node_before(ch);
  }
  return math;
}

// function appendResult(result,sep,math) {
//   GetCurrentEditor().insertHTMLWithContext(
//       result.replace(fullmath,fullmath+sep),
//       "", "", "", null,
//       math, math.childNodes.length, false );
//   coalescemath();
// }

function runFixup(math)
{
  try {
    var out = GetCurrentEngine().perform(math,GetCurrentEngine().Fixup);
    return out;
  } catch(e) {
    dump("runFixup(): "+e+"\n");
    return math;
} }

function GetFixedMath(math)
{
  return runFixup(GetMathAsString(math));
}

function doEvalComputation(math,op,joiner,remark) {
  var mathstr = GetFixedMath(GetRHS(math));
  try {
    var out = GetCurrentEngine().perform(mathstr,op);
    appendResult(out,joiner,math);
  } catch (e) {
    dump("doEvalComputation(): " + e+"\n");
} }

function doComputeCommand(cmd) {
  var selection = GetCurrentEditor().selection;
  if (selection) {
    var element = findmathparent(selection.focusNode);
    if (!element) {
	    dump("not in math!\n");
      return;
    }
    var eng = GetCurrentEngine();
    switch (cmd) {
    case "cmd_compute_Evaluate":
      doEvalComputation(element,eng.Evaluate,"<mo>=</mo>","evaluate");
      break;
    case "cmd_compute_EvaluateNumeric":
      doEvalComputation(element,eng.Evaluate_Numerically,"<mo>"+String.fromCharCode(0x2248)+"</mo>","evaluate numeric");
      break;
} } }

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
	      dump("focus not in math!\n");
        return;
      }
    }
    var last = node_before(element);
    if (!last || last.localName != "math") {
      dump("previous is not math!\n");
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
} }

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

function GetMathAsString(math)
{
  var ser = new XMLSerializer();
  var mathstr = ser.serializeToString(math);
  if (math.localName != "math")
    mathstr = "<math>" + mathstr + "</math>";
  // risky string surgery, but it works in simple cases
  mathstr = mathstr.replace(/ _moz_dirty=\"\"/g,"");
  mathstr = mathstr.replace(/\<mi\/\>/g,"");
  // the following namespace problems happen with inserted computation results...need a better solution
  mathstr = mathstr.replace(/ xmlns:a0=\"http:\/\/www.w3.org\/1998\/Math\/MathML\"/g,"");
  mathstr = mathstr.replace(/\<a0:/g,"\<");
  mathstr = mathstr.replace(/\<\/a0:/g,"\<\/");
  return mathstr;
}

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


function openTeX()
{
  dump("Open TeX \n");
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
  fp.init(window, GetString("OpenTeXFile"), msIFilePicker.modeOpen);     
  fp.appendFilter(GetString("TeXFiles"), "*.tex; *.ltx; *.shl");
  fp.appendFilters(msIFilePicker.filterXML)

  msiSetFilePickerDirectory(fp, "tex");


  try {
    fp.show();
    // need to handle cancel (uncaught exception at present) 
  }
  catch (ex) {
    dump("filePicker.chooseInputFile threw an exception\n");
    dump(e+"\n");
    
  }

  // This checks for already open window and activates it... 
  // note that we have to test the native path length
  // since file.URL will be "file:///" if no filename picked (Cancel button used)
  
  if (fp.file && fp.file.path.length > 0) {
   
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//      Prepare to run pretex.exe. We need to send it some directories:                                       //
//       The input directory gives the location of the .cls and .tex files that pretex reads to               //
//         determine how to translate the TeX. This is usually prince/ptdata.                                 //
//       The MathML conversion directory. This is where the DLL used to convert math and its associated .gmr  //
//         files are. This is usually resource://app.                                                                  //
//       The input .tex file.                                                                                 //
//       The output directory where the auxiliary files that are generated (such as .css, etc.) go.           //
//       The output <filename>.sci file.                                    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    msiSaveFilePickerDirectory(fp, "tex");
    var filename = fp.file.leafName.substring(0,fp.file.leafName.lastIndexOf("."));
    var infile =  "\""+fp.file.path+"\"";
    dump("Open Tex: " + infile+"\n");

// Get the directory for the result from the preferences, or default to the SWPDocs directory
    var docdir;
    try
    {
      var prefs = GetPrefs();
      var docdirname = prefs.getCharPref("swp.prefDocumentDir");
      dump("swp.prefDcoumentDir is ", docdirname + "\n");
      docdir = Components.classes["@mozilla.org/file/local;1"].
          createInstance(Components.interfaces.nsILocalFile);
      docdir.initWithPath(docdirname);
      if (!valueOf(docdir.exists()))
        docdir.create(1, 0755);
    }
    catch (e)
    {
      var dirkey;
      if (msiGetOS() == "win")
        dirkey = "Pers";
      else
      if (msiGetOS() =="osx")
        dirkey = "UsrDocs";
      else
        dirkey = "Home";
      // if we can't find the one in the prefs, get the default
      docdir = dsprops.get(dirkey, Components.interfaces.nsILocalFile);
      if (!docdir.exists()) docdir.create(1,0755);
      var defdocdirstring = GetStringPref("swp.prefDocumentDir");
      if (defdocdirstring.length == 0) defdocdirstring = "SWPDocs";
      docdir.append(defdocdirstring);
      if (!docdir.exists()) docdir.create(1,0755);
      dump("default document directory is "+docdir.path+"\n");
    }                                                                

    var outdir = docdir.clone();
    outdir.append(filename + "_work");
    var outfile = outdir.clone();
    outfile.append("main.xhtml");
	  if (outdir.exists()) outdir.remove(true);
    outdir.create(1 , 0755);
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
    exefile.append("pretex.exe");
    var dataDir = dsprops.get("resource:app", Components.interfaces.nsIFile);
    dataDir.append("ptdata");
    dump("\n\nExe="+exefile.path);
    dump("\noutdir=\""+outdir.path);
    dump("\noutfile=\""+outfile.path);
    dump("\ninfile=\""+fp.file.path);
    dump("\ndataDir=\""+dataDir.path);
    dump("\nmmldir=\""+mmldir.path+"\n");
	dump("\nargs =['-i', "+dataDir.path+", '-f', 'latex2xml.tex', '-o', "+outdir.path+", '-m',"+ mmldir.path+", "+fp.file.path+", "+outfile.path);

    // run pretex.exe
    
    try 
    {
      var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
      theProcess.init(exefile);
      var args =['-i', dataDir.path, '-f', 'latex2xml.tex', '-o', outdir.path, '-m', mmldir.path, fp.file.path, outfile.path];
      theProcess.run(true, args, args.length);
    } 
    catch (ex) 
    {
         dump("\nUnable to open TeX:\n");
		     dump("\nexe  = "  + exefile);
         dump("\narg paths = " + dataDir.path + "\n   " + fp.file.path + "\n    " + outfile.path + "\n     " + outdir.path);
         dump(ex+"\n");
    }      
//  TODO BBM todo: we may need to run a merge program to bring in processing instructions for specifying tag property files
    
    msiEditPage("file:///" + outfile.path.replace(/\\/g,"/"), window, false);
  }                       
}

#define INTERNAL_XSLT

// documentAsTeXFile writes the document out as TeX. It uses XSLT transformations from 'xslSheet'.
// Returns true if the TeX file was created.
// outTeXfile is an nsILocalFile. If it is null, create main.tex in the document's 'tex' directory.

function documentAsTeXFile( document, xslSheet, outTeXfile, compileInfo )
{
  dump("\nDocument as TeXFile\n");
  if (!document) return false;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var documentPath = document.documentURI;
  var docurl = msiURIFromString(documentPath);                                      
  var workingDir;
  var outTeX;
  workingDir = msiFileFromFileURL(docurl);
  var bareleaf = workingDir.leafName; // This is the leaf of the document.
  workingDir = workingDir.parent;
  if (outTeXfile == null  || outTeXfile.path.length == 0)
  {
    outTeX = workingDir.clone();
    outTeX.append("tex");
    if (!outTeX.exists()) outTeX.create(1, 0755);
    outTeXfile = outTeX;
    outTeXfile.append(bareleaf + ".tex");
  }
  var outfileTeXPath = outTeXfile.path;
  var stylefile;
  var xslPath;
#ifdef INTERNAL_XSLT
  var xslPath = "chrome://prnc2ltx/content/"+xslSheet;
  var str = documentToTeXString(document, xslPath);
  compileInfo.runMakeIndex = /\\makeindex/.test(str);
  compileInfo.passCount = 1;
  var runcount = 1;
  if (/\\tableofcontents|\\listoffigures|\\listoftables/.test(str)) runcount = 3;
  if (compileInfo.passCount < runcount) compileInfo.passCount = runcount;
  if (/\\xref|\\pageref|\\vxref|\\vpageref/.test(str)) runcount = 2;
  if (compileInfo.passCount < runcount) compileInfo.passCount = runcount;
  var matcharr = /%% *minpasses *= *(\d+)/.exec(str);
  if (matcharr && matcharr.length > 1) 
  {
    runcount = matcharr[1];
    if (compileInfo.passCount < runcount) compileInfo.passCount = runcount;
  }
  if (compileInfo.runMakeIndex) {
    if (compileInfo.passCount < 2) compileInfo.passCount = 2;
  }

//  dump("\n"+str);
  if (!str || str.length < 3) return false;
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
#else
  stylefile = dsprops.get("resource:app", Components.interfaces.nsIFile);
  stylefile.append("res");
  stylefile.append("xsl");
  stylefile.append(xslSheet);
  xslPath = stylefile.path;
  var exefile = dsprops.get("resource:app", Components.interfaces.nsIFile);
  exefile.append("Transform.exe");

  var index =  bareleaf.lastIndexOf(".");
  if (index > 0) bareleaf = bareleaf.substr(0,index); // probably at this time, bareleaf=="main"

  var outfile = workingDir.clone();
  outfile.append("temp");
  // clean out the temp directory
  try {if (outfile.exists()) outfile.remove(true);}
  catch(e){
    dump("deleting temp directory failed: "+e.message+"\n");
  }
  try  {outfile.create(1, 0755);}
  catch(e){
    dump("creating temp directory failed: "+e.message+"\n");
    return false
  }
  outfile.append(bareleaf + ".xml");
      
  dump("\nOutput file = " + outfile.path+"\n");
  var s = new XMLSerializer();
  var str = s.serializeToString(document);


  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(outfile, -1, -1, false);
  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
    .createInstance(Components.interfaces.nsIConverterOutputStream);
  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
  os.writeString(str);
  os.close();
  fos.close();
  var outfilePath = outfile.path;
  while (xslPath.charAt(0) == "/".charAt(0)) xslPath = xslPath.substr(1);
  // for Windows
#ifdef XP_WIN32
  xslPath = xslPath.replace("\\","/","g");
  outfilePath = outfilePath.replace("\\","/","g");
  outfileTeXPath = outfileTeXPath.replace("\\","/","g");
#endif
  try 
  {
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(exefile);
    var args =['-s', outfilePath, '-o', outfileTeXPath, xslPath, ];
    theProcess.run(true, args, args.length);
  } 
  catch (ex) 
  {
    dump("\nUnable to export TeX: "+ex.message+"\n");
    return false;
  }      
#endif
  return outTeXfile.exists();
}

function currentFileName()
{
//  var docUrl = GetDocumentUrl();
  var editorElement = msiGetTopLevelEditorElement();
  var docUrl = msiGetEditorURL(editorElement);
  dump('\nThis doc url = ' + docUrl+"\n");
  var filename = "";
  if (docUrl && !IsUrlAboutBlank(docUrl))
    filename = GetFilename(docUrl);
  return filename;
}


function exportTeX()
{
  dump("\nExport TeX\n");

  if (currentFileName().length < 0) return;
  var editor = GetCurrentEditor();
  if (!editor) return;

   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
   fp.init(window, "Export TeX File", msIFilePicker.modeSave);
   fp.appendFilter("TeX files", "*.tex; *.ltx");
   fp.appendFilters(msIFilePicker.filterText);
   fp.defaultExtension = ".tex";
   compileInfo = new Object();
   try 
   {
     var dialogResult = fp.show();
     if (dialogResult != msIFilePicker.returnCancel)
       if (!documentAsTeXFile(editor.document, "latex.xsl", fp.file, compileInfo ))
         AlertWithTitle("XSLT Error", "TeX file not created");

   }
   catch (ex) 
   {
     dump("filePicker threw an exception in exportTeX: "+ex.message+"\n");
   }

   
}


function exportToWeb()
{
  dump("\nExport to Web\n");

  if (currentFileName().length < 0) return;
  var editor = GetCurrentEditor();
  if (!editor) return;

   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
   fp.init(window, "Export to Web", msIFilePicker.modeSave);
   fp.appendFilter("Compressed zip file", "*.zip");
   fp.appendFilter("Directory", "");
   try 
   {
     var dialogResult = fp.show();
     if (dialogResult != msIFilePicker.returnCancel)
       if (!saveforweb(editor.document, false, fp.file ))
         AlertWithTitle("Export", "Web file not created.");
   }
   catch (ex) 
   {
     dump("filePicker threw an exception in exportToWeb: "+ex.message+"\n");
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

var passData;

function compileTeXFile( compiler, infileLeaf, infilePath, outputDir, compileInfo )
{
  // the following requires that the pdflatex program (or a hard link to it) be in TeX/bin/pdflatex 
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var exefile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
  var indexexe = exefile.clone();
  var extension;
  var compiledFileLeaf = "SWP";
  passData = new Object;
#ifdef XP_WIN32
  extension = "cmd";
#else
  extension = "bash";
#endif
  exefile.append(compiler+"."+extension);
  indexexe.append("makeindex."+extension);
  dump("\nexecutable file: "+exefile.path+"\n");
  passData.file = exefile;
  passData.indexexe = indexexe;
  passData.outputDir = outputDir;
  passData.args = ["-output-directory", outputDir, infileLeaf, compiledFileLeaf];
  passData.passCount = compileInfo.passCount;
  passData.runMakeIndex = compileInfo.runMakeIndex;
  var i;
  dump("Opening dialog\n");
  window.openDialog("chrome://prince/content/passes.xul","about", "chrome,modal=yes,resizable=yes,alwaysRaised=yes",
    passData);
//    There was some commented code here for using the pipe-console object from the enigmail project. We are not 
//    using it in 6.0, and XulRunner is getting a better implementation, which we will use later.
  // check for a dvi or pdf file
  var compiledFileLeaf = "SWP";                                                                               var outfileLeaf = compiledFileLeaf;
  outfileLeaf += ".pdf";

  dump("\nOutputleaf="+outfileLeaf+"\n");
  var tempOutputfile;
  var outputfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  outputfile.initWithPath( passData.outputDir );
  tempOutputfile = outputfile.clone();
  var leaf = "main.pdf";
  outputfile.append(leaf);
  var n = 0;
  dump("Leaf is "+leaf+"\n");
  while (outputfile.exists())
  {
    leaf = "main"+(n++)+".pdf";
    dump("Leaf is "+leaf+"\n");
    outputfile = outputfile.parent;
    outputfile.append(leaf);
  }
  currPDFfileLeaf = leaf;
  tempOutputfile.append("swp.pdf");
//  if (outputfile.exists())
//    outputfile.remove(false);
  if (tempOutputfile.exists())
  {
    tempOutputfile.moveTo(null, leaf);     // BBM: allow for .xdv ??
    dump("\nFinal output filename: "+tempOutputfile.path+"\n");
    return true;
  }
  else return false;    
}


function printPDFFile(infile)
{
  // the following requires that the printpdf batch file (or a hard link to it) be in xpi-stage/prince/TeX/bin/printpdf.cmd 
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var exefile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
  exefile.append("printpdf.cmd");
  dump("\nexecutable file: "+exefile.path+"\n");
  try 
  {
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(exefile);
    var args = [infile.path];
    theProcess.run(true, args, args.length);
  } 
  catch (ex) {
    dump("\nUnable to run Acrobat: "+ex.message+"\n");
    return false;
  }
}

// compileDocument compiles the current document of the current editor; it converts it to TeX and then PDF.
// Returns true if everything succeeded. 
function compileDocument()
{
  var editor = GetCurrentEditor();
  if (!editor) return null;
  var compiler = "pdflatex";
  // Determine which compiler to use
  var texprogNode;
  var texprogNodes = editor.document.getElementsByTagName("texprogram");
  if (texprogNodes.length > 0)
  {
    texprogNode = texprogNodes[0];
    if (texprogNode.hasAttribute("prog")) compiler = texprogNode.getAttribute("prog");
  }
  var editorElement = msiGetTopLevelEditorElement();
  var pdfViewer = document.getElementById("preview-frame");
  if (pdfViewer && (pdfViewer.src != "about:blank"))
    pdfViewer.loadURI("about:blank");   // this releases the currently displayed pdf preview.
  dump("pdfModCount = "+editorElement.pdfModCount+", modCount is ");
  editorElement.pdfModCount = editor.getModificationCount();
  dump(editorElement.pdfModCount+"\n");
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
      if (outputfile.exists()) AlertWithTitle("Locked file","the tex directory was not deleted");
      outputfile.create(1, 0755);
    }
    catch(e) {}; // 
    var pdffile = outputfile.clone();
    var pdffileroot = outputfile.clone();
    outputfile.append("main.tex");
    if (outputfile.exists()) outputfile.remove(false);
    
    dump("\TeX file="+outputfile.path + "\n");
//    dump("DVI/PDF file is " + pdffile.path + "\n"); 
    var compileInfo = new Object();  // an object to hold pass counts and whether makeindex needs to run.
    if (documentAsTeXFile(editor.document, "latex.xsl", outputfile, compileInfo ))
    {
      if (compileTeXFile(compiler, "main", outputfile.path, pdffile.path, compileInfo))
      {
        if (!currPDFfileLeaf) currPDFfileLeaf = "main.pdf";
        dump("currPDFfileLeaf is "+currPDFfileLeaf+"\n");
        pdffile.append(currPDFfileLeaf);
        if (!pdffile.exists())
        {  
          AlertWithTitle("TeX Error", "Unable to create a PDF file.");
          goDoCommand("cmd_showTeXLog");
          return null;
        }
        else 
        {
          dump("outputfile to be launched: "+pdffile.path+"\n");
          return pdffile;
        }
      }
      else return null;
    }
    else 
    {
      AlertWithTitle("XSLT Error", "Unable to create a TeX file");
      return null;
    }
  }
  catch(e)
  {
    dump("compileDocument failed: "+e.message+"\n");
    return null;
  }
}


function printTeX(preview )
{
  try {
    var pdffile = compileDocument();
    if (pdffile)
    {
      if (preview)
      {
        document.getElementById("preview-frame").loadURI(msiFileURLStringFromFile(pdffile));
        // Switch to the preview pane (third in the deck)
        goDoCommand("cmd_PreviewMode"); 
      } 
      else
        printPDFFile(pdffile);
    }
  }
  catch(e) {
    dump("printTeX failed: "+e.message+"\n");
  }
}

function previewTeX()
{
  printTeX(true);
};


function compileTeX(compiler)
{
  try
  {
    var editor = GetCurrentEditor();
    if (!editor) return;

    document.getElementById("preview-frame").loadURI("about:blank");
  // now save this TeX string and run TeX on it.  
    var editorElement = msiGetTopLevelEditorElement();
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
    
    dump("TeX file="+outputfile.path)+"\n";  
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
      dump("filePicker threw an exception\n");
      return;
    }
  
//    var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
//    fos.init(outputfile, -1, -1, false);
//    var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
//      .createInstance(Components.interfaces.nsIConverterOutputStream);
//    os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
//    os.writeString(str);
//    os.close();
  //   fos.close();
    var compileInfo = new Object();
    documentAsTeXFile(editor.document, "latex.xsl", outputfile, compileInfo );
    if (!outputfile.exists())
    {
      AlertWithTitle("XSLT Error", "Unable to generate TeX file");
    } else
    {
      if (compileTeXFile(compiler, outleaf, outputfile.path, pdffile.parent.path, compileInfo))
      {
        if (!pdffile.exists())
        {
          AlertWithTitle("TeX Error", "Need to offer to show log");
          goDoCommand("cmd_showTeXLog");
        }
        else
        // move the output result to the place indicated by fp.
          pdffile.move(fp.file.parent, fp.file.leafName); 
      } 
    }
  }
  catch(e) {
    dump(e+"\n");
    return;
  }
}

function initializeAutoCompleteStringArray()
 
{ 
 dump("===> initializeAutoCompleteStringArray\n");
                            
//   var stringArraySearch = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService(Components.interfaces.nsIAutoCompleteSearchStringArray);
//   stringArraySearch.editor = GetCurrentEditor(); 
}

 
 
 
//In msiEditor.js // handle events on prince-specific elements here, or call the default goDoCommand() 
//In msiEditor.js function goDoPrinceCommand (cmdstr, element) 
//In msiEditor.js {
//In msiEditor.js    if ((element.localName.toLowerCase() == "img") && (element.getAttribute("msigraph") == "true"))
//In msiEditor.js    {
//In msiEditor.js       graphClickEvent(cmdstr);
//In msiEditor.js    }
//In msiEditor.js    else 
//In msiEditor.js    { 
//In msiEditor.js       goDoCommand(cmdstr);  
//In msiEditor.js    }
//In msiEditor.js }


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
  var str = "";
  var resultString = "";
  if (xslPath.length == 0) return resultString;
  var contents = "";
  var leafname = /[^\/]*$/;
  var match;
  var xsltProcessor = new XSLTProcessor();
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var outputfile = dsprops.get("ProfD", Components.interfaces.nsILocalFile);
  var matcharray = leafname.exec(xslPath);
  outputfile.append("comp-"+matcharray[0]);
  // we cache the xsl file as comp-???.xsl in the user's profile
  
  if (outputfile.exists())
  {
    path=msiFileURLFromFile( outputfile );
    var myXMLHTTPRequest = new XMLHttpRequest();
    myXMLHTTPRequest.open("GET", path.spec, false);
    myXMLHTTPRequest.send(null);
    resultString = myXMLHTTPRequest.responseText;
  }
  else {
    var path = xslPath;
    var stylesheetElement=/<xsl:stylesheet[^>]*>/;
    var includeFileRegEx = /<xsl:include\s+href\s*=\s*\"([^\"]*)\"\/>/;
    var myXMLHTTPRequest = new XMLHttpRequest();
    myXMLHTTPRequest.open("GET", path, false);
    myXMLHTTPRequest.send(null);

    str = myXMLHTTPRequest.responseText;
  // a bug in the Mozilla XSLT processor causes problems with included stylesheets, so
  // we do the inclusions ourselves 
    var filesSeen= []; 
    while (match = includeFileRegEx.exec(str))
    {
      resultString += str.slice(0, match.index);
      str = str.slice(match.index);
      str = str.replace(match[0],""); // get rid of the matched pattern. Why does lastIndex not work?
      dump("File "+match[1]+"found at index = "+match.index+ "\n");
      // match[1] should have the file name of the incusion 
      if (filesSeen.indexOf(match[1]) < 0)
      {
        try {
          path = path.replace(leafname,match[1]);
          myXMLHTTPRequest.open("GET", path, false);
          myXMLHTTPRequest.send(null);
          contents = myXMLHTTPRequest.responseText;
          if (contents)
          {
            // strip out the xml and xsl declarations
            contents = contents.replace('<?xml version="1.0"?>','');
            contents = contents.replace("</xsl:stylesheet>","");
            contents = contents.replace(stylesheetElement,"");
          }
          filesSeen.push(match[1]);
        }
        catch(e)
        {
          dump("Reading include file, got "+e.message+"\n");
        }
      }
      else {
        dump("\n\n\n" + str+"\n\n\n");
        dump("Already saw " + match[1] +"\n");
        contents="";
        break;
      }                         
      // now replace the include command with the contents of the file
  //    includeFileRegEx.lastIndex = includeFileRegEx.index;
      str = contents + str;
      // and continue  
    }
    resultString += str;
  }
  // BBM: for debugging purposes, let's write the string out
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var outputfile = dsprops.get("ProfD", Components.interfaces.nsILocalFile);
  outputfile.append("comp-"+matcharray[0]);
  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(outputfile, -1, -1, false);
  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
    .createInstance(Components.interfaces.nsIConverterOutputStream);
  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
  os.writeString(resultString);
  os.close();
 // end of debugging code 
  var parser = new DOMParser();
  var doc = parser.parseFromString(resultString, "text/xml");
    
  try{
    xsltProcessor.importStylesheet(doc);
    var newDoc = xsltProcessor.transformToDocument(document);
    var strResult = newDoc.documentElement.textContent || "";
  }
  catch(e){
    dump("error: "+e.message+"\n\n");
  //  dump(resultString);
  }
  return strResult;
}
