const NS_IPCSERVICE_CONTRACTID  = "@mozilla.org/process/ipc-service;1";
const NS_IPCBUFFER_CONTRACTID   = "@mozilla.org/process/ipc-buffer;1";
const NS_PIPECONSOLE_CONTRACTID = "@mozilla.org/process/pipe-console;1";
const NS_PIPETRANSPORT_CONTRACTID= "@mozilla.org/process/pipe-transport;1";
const NS_PROCESSINFO_CONTRACTID = "@mozilla.org/xpcom/process-info;1";

var gPipeConsole;

function goAboutDialog() {
  window.openDialog("chrome://prince/content/aboutDialog.xul", GetString("About"), "modal,chrome,resizable=yes");
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
//         This is the <filename>__files directory in the directory containing the output file.
//       The output <filename>.sci file.                                    
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    msiSaveFilePickerDirectory(fp, "tex");
    var filename = fp.file.leafName.substring(0,fp.file.leafName.lastIndexOf("."));
    var infile =  "\""+fp.file.path+"\"";
    dump("Open Tex: " + infile+"\n");

// Get the directory for the result from the preferences, or default to the SWP Docs directory
    var docdir;
    try
    {
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
#ifdef XP_WIN
        dirkey = "Pers";
#else
#ifdef XP_MACOSX
        dirkey = "UsrDocs";
#else
        dirkey = "Home";
#endif
#endif
      // if we can't find the one in the prefs, get the default
      docdir = dsprops.get(dirkey, Components.interfaces.nsILocalFile);
      if (!docdir.exists()) docdir.create(1,0755);
      // Choose one of the three following lines depending on the app
	  // BBM Replace this code by using a string from a branding file
      docdir.append("SWP Docs");
      // docdir.append("SW Docs");
      // docdir.append("SNB Docs");
      if (!docdir.exists()) docdir.create(1,0755);
      dump("default document directory is "+docdir.path+"\n");
    }

    var outfile = docdir.clone();
    var outdir = docdir.clone();
    outfile.append(filename+".sci");
    outdir.append(filename + "_files");
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
         dump(ex+"\n");
    }      
//  TODO BBM todo: we may need to run a merge program to bring in processing instructions for specifying tag property files
    
    msiEditPage("file:///" + outfile.path.replace(/\\/g,"/"), window, false);
  }                       
}

function documentAsTeXFile( document, xslSheet, outTeXfile )
{
    dump("\nDocument as TeXFile\n");

    var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);

    var exefile = dsprops.get("resource:app", Components.interfaces.nsIFile);
    var stylefile = exefile.clone();
    exefile.append("Transform.exe");
    var bareleaf = document.documentURI;
    bareleaf = unescape(bareleaf);
    var index =  bareleaf.lastIndexOf(".");
    if (index > 0) bareleaf = bareleaf.substr(0,index);
    index = bareleaf.lastIndexOf("/");
    if (index > 0) bareleaf = bareleaf.substr(index+1);

    var outfile = msiAuxDirFromDocPath(document.documentURI);
    var outTeX = outfile.clone();
    outfile.append("temp");
    // clean out the temp directory
    outfile.remove(true);
    outfile.create(1, 0755);
    outfile.append(bareleaf + ".xml");
    if (outTeXfile == null)
    {
      outTeXfile = outTeX;
      outTeXfile.append("tex");
      if (!outTeXfile.exists()) outTeXfile.create(1, 0755);
      outTeXfile.append(bareleaf + ".tex");
    }
        
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

    stylefile.append("res");
    stylefile.append("xsl");
    stylefile.append(xslSheet);
    var xslPath = stylefile.path;
    var outfilePath = outfile.path;
    var outfileTeXPath = outTeXfile.path;
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
      dump("\nUnable to export TeX:\n");
      dump(ex+"\n");
    }      

}
//{
//  var str = "";
//  if (!document) return str;
//  if (xslSheetPath.length == 0) return str;
//  var xsltProcessor = new XSLTProcessor();
//  var myXMLHTTPRequest = new XMLHttpRequest();
//  myXMLHTTPRequest.open("GET", xslSheetPath, false);
//  myXMLHTTPRequest.send(null);
//
//  var xslStylesheet = myXMLHTTPRequest.responseXML;
//  xsltProcessor.importStylesheet(xslStylesheet);
//  var newDoc = xsltProcessor.transformToDocument(document);
//  str = newDoc.documentElement.textContent;
//  dump("\n"+str);
//  return str;
//}

//function documentAsTeXFile( document, xslSheet, outputFile )
//{
//  if (outputFile && outputFile.path.length > 0) 
//  {
//    var str = documentAsTeX(document, xslSheet );
//    if (outputFile.exists()) 
//    outputFile.remove(false);
//    outputFile.create(0, 0755);
//    var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
//    fos.init(outputFile, -1, -1, false);
//    var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
//      .createInstance(Components.interfaces.nsIConverterOutputStream);
//    os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
//    os.writeString(str);
//    os.close();
////   fos.close();
//  }
// }


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
   try 
   {
     fp.show();
   }
   catch (ex) 
   {
     dump("filePicker threw an exception\n");
   }
   documentAsTeXFile(editor.document, "latex.xsl", fp.file );
}

/* ==== */
/* = 
compileTeXFile:
  pdftex -- a boolean to determine whether to produce pdf (as opposed to dvi)
  infileLeaf -- the name of the input TeX file without '.tex' or the initial part of the path 
  infilePath -- the full name of the input TeX file, including the path and 'tex'
  outputDir -- the directory in which to put the resulting file
  passCount -- the number of passes with LaTeX needed
  
  returns -- a boolean to indicate whether the expected file appears where it is supposed to
  
 = */
/* ==== */

function compileTeXFile( pdftex, infileLeaf, infilePath, outputDir, passCount )
{
  // the following requires that the pdflatex program (or a hard link to it) be in xpi-stage/prince/TeX/bin/pdflatex 
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var exefile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
  if (pdftex)
    exefile.append("pdflatex.cmd");
  else
    exefile.append("tex.cmd");
  dump("\nexecutable file: "+exefile.path+"\n");
  try 
  {
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(exefile);
    var args = ["-output-directory", outputDir, infileLeaf, passCount];
    theProcess.run(true, args, args.length);
//    var data=new Object();
//    data.pipeconsole = Components.classes["@mozilla.org/process/pipe-console;1"].createInstance(Components.interfaces.nsIPipeConsole);
//    data.pipeconsole.open(500,80,false);
//    data.pipetransport = Components.classes["@mozilla.org/process/pipe-transport;1"].createInstance(Components.interfaces.nsIPipeTransport);
//    var args = ["-output-directory", outputDir, infileLeaf, passCount];
//    data.pipetransport.init(exefile.path, args, args.length, "", 0, 2000, "", true, false, data.pipeconsole );
//    data.pipetransport.loggingEnabled = true;
//    data.pipetransport.asyncRead(data.pipeconsole, null, 0, -1, 0);
//    data.stdin = data.pipetransport.openOutputStream(0,-1,0);
//    data.clean = false;
//    window.openDialog("chrome://prince/content/msiConsole.xul", "_blank", "chrome,close,titlebar,modal", data);
//    data.pipetransport.join();
//    // we need to kill the process
//    
  } 
  catch (ex) {
    dump("\nUnable to run TeX:\n");
    dump(ex+"\n");
    return false;
  }
  // check for a dvi or pdf file
  var outfileLeaf = infileLeaf;
  if (pdftex)
    outfileLeaf += ".pdf";
  else
    outfileLeaf += ".dvi";
  dump("\nOutputleaf="+outfileLeaf+"\n");
  var outputfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  outputfile.initWithPath( outputDir );
  outputfile.append(outfileLeaf);
  dump("\nFinal output filename: "+outputfile.path+"\n");
  return true;//outputfile.exists();
}


function printTeX( pdftex, preview )
{
  try {
    var editor = GetCurrentEditor();
    if (!editor) return;
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
    var extendedoutleaf = outleaf;
    outputfile = outputfile.parent;
    outputfile.append(outleaf + "_files");
    if (!outputfile.exists()) outputfile.create(1, 0755);
    outputfile.append("tex");
    // remove and create the tex directory to clean it out
    outputfile.remove(true);
    outputfile.create(1, 0755);
    var dvipdffile = outputfile.clone();
    var dvipdffileroot = outputfile.clone();
    dvipdffile.append(outleaf+ (pdftex?".pdf":".dvi"));
    if (dvipdffile.exists()) 
    {
      document.getElementById("preview-frame").loadURI("about:blank");
      try {
        dvipdffile.remove(false);
      }
      catch(e) {
        var n = 0;
        while (dvipdffile.exists())
        {
          dvipdffile = dvipdffileroot.clone();
          extendedoutleaf = outleaf + "_" + n++;
          dvipdffile.append(extendedoutleaf + (pdftex?".pdf":".dvi"));
        }
      }
    }
    outputfile.append(extendedoutleaf+".tex");
    if (outputfile.exists()) outputfile.remove(false);
    
    dump("\TeX file="+outputfile.path + "\n");
    dump("DVI/PDF file is " + dvipdffile.path + "\n"); 
//    var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
//    fos.init(outputfile, -1, -1, false);
//    var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
//      .createInstance(Components.interfaces.nsIConverterOutputStream);
//    os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
//    os.writeString(str);
//    os.close();
  //   fos.close();
    documentAsTeXFile(editor.document, "latex.xsl", outputfile );
    if (compileTeXFile(pdftex, extendedoutleaf, outputfile.path, dvipdffile.parent.path, 1))
    {
      if (!dvipdffile.exists())
      {
        AlertWithTitle("TeX Error", "Click on View/TeX Log to see the log file");
        goDoCommand("cmd_showTeXLog");
      }
      dump("outputfile to be launched: "+dvipdffile.path+"\n");
      if (preview)
      {
        document.getElementById("preview-frame").loadURI(dvipdffile.path);
        // Switch to the preview pane (third in the deck)
        goDoCommand("cmd_PreviewMode"); 
      }     
    }
    else
      dump("\nRunning TeX failed to create a file!\n");
   }
   catch(e) {
     dump(e+"\n");
   }
}

function previewTeX(pdftex)
{
  printTeX(pdftex, true);
};

function compileTeX(pdftex)
{
  try
  {
    var editor = GetCurrentEditor();
    if (!editor) return;
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
    var dvipdffile = outputfile.clone();
    outputfile.append(outleaf+".tex");
    if (outputfile.exists()) outputfile.remove(false);
    dvipdffile.append(outleaf+ (pdftex?".pdf":".dvi"));
    if (dvipdffile.exists()) dvipdffile.remove(false);
    
    dump("TeX file="+outputfile.path)+"\n";  
    var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
    fp.init(window, "Save "+(pdftex?"PDF":"DVI")+" file", msIFilePicker.modeSave);

    fp.appendFilter("Compiled files","*.pdf; *.dvi");
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
    documentAsTeXFile(editor.document, "latex.xsl", outputfile );
    if (!outputfile.exists())
    {
      AlertWithTitle("XSLT Error", "Need to offer to show log");
      goDoCommand("cmd_showXSLTLog");
    } else
    {
      if (compileTeXFile(pdftex, outleaf, outputfile.path, dvipdffile.parent.path, 1))
      {
        if (!dvipdffile.exists())
        {
          AlertWithTitle("TeX Error", "Need to offer to show log");
          goDoCommand("cmd_showTeXLog");
        }
        else
        // move the output result to the place indicated by fp.
          dvipdffile.move(fp.file.parent, fp.file.leafName); 
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