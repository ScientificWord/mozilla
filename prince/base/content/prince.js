function goAboutDialog() {
  window.openDialog("chrome://prince/content/aboutDialog.xul", "About", "modal,chrome,resizable=yes");
}

function doOpen() {
  var fp = Components.classes["@mozilla.org/filepicker;1"].
             createInstance(Components.interfaces.nsIFilePicker);
  fp.init(window, "Open HTML File", Components.interfaces.nsIFilePicker.modeOpen);
  fp.appendFilters(Components.interfaces.nsIFilePicker.filterHTML);
  fp.appendFilters(Components.interfaces.nsIFilePicker.filterXML);
  fp.appendFilter("XHTML Files","*.xhtml; *.xht");
  fp.appendFilters(Components.interfaces.nsIFilePicker.filterText);
  fp.appendFilters(Components.interfaces.nsIFilePicker.filterAll);

  try {
    fp.show();
  }
  catch (ex) {
    dump("filePicker.show() threw an exception\n");
  }

  if (fp.file && fp.file.path.length > 0) {
    dump("Ready to edit page: " + fp.fileURL.spec +"\n");
    try {
      msiEditPage(fp.fileURL.spec, window, false);
    } catch (e) { dump(" EditorLoadUrl failed: "+e+"\n"); }
} }


function doNew() {
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  var fp = Components.classes["@mozilla.org/filepicker;1"].
             createInstance(Components.interfaces.nsIFilePicker);
  fp.defaultExtension = ".shl";
  var dir1 =dsprops.get("resource:app", Components.interfaces.nsIFile);
  dir1.append("shells");;
  fp.displayDirectory = dir1;
  fp.init(window, "Open Shell File", Components.interfaces.nsIFilePicker.modeOpen);
  fp.appendFilter("Shell Files","*.xhtml; *.xht; *.shl");
  fp.appendFilters(Components.interfaces.nsIFilePicker.filterText);
  fp.appendFilters(Components.interfaces.nsIFilePicker.filterAll);

  try {
    fp.show();
  }
  catch (ex) {
    dump("filePicker.show() threw an exception\n");
  }

  if (fp.file && fp.file.path.length > 0) {
    dump("Ready to edit shell: " + fp.fileURL.spec +"\n");
    try {
      msiEditPage(fp.fileURL.spec, window, false);
//      GetCurrentEditorElement().webNavigation.loadURI(fp.fileURL.spec,
//               Components.interfaces.nsIWebNavigation.LOAD_FLAGS_BYPASS_CACHE,
//               null, null, null);
    } catch (e) { dump(" EditorLoadUrl failed: "+e+"\n"); }
} }

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

    if (editorList.item(0))
      return editorList.item(0);

    tmpWindow = tmpWindow.opener;
  } while (tmpWindow);

  return null;
}

function doQuit() {
  var appStartup = Components.classes["@mozilla.org/toolkit/app-startup;1"].
                     getService(Components.interfaces.nsIAppStartup);

  appStartup.quit(Components.interfaces.nsIAppStartup.eAttemptQuit);
  return true;
}

/////////////////////////////////////////////////
// our connection to the computation code
var compsample;
var compengine;

function GetCurrentEngine() {
  if (!compsample) {
    compsample = Components.classes["@mackichan.com/simplecomputeengine;2"].getService(Components.interfaces.msiISimpleComputeEngine);
    try {
      compsample.startup("mupInstall.gmr");
      compengine = 2;
    } catch(e) {
      var msg_key;
      if (e.result == Components.results.NS_ERROR_NOT_AVAILABLE)
        msg_key = "Error.notavailable";
      else if (e.result == Components.results.NS_ERROR_FILE_NOT_FOUND)
        msg_Key = "Error.notfound";
      else if (e.result == Components.results.NS_ERROR_NOT_INITIALIZED)
        msg_key = "Error.notinitialized";
      else if (e.result == Components.results.NS_ERROR_FAILURE)
        msg_key = "Error.failure";
      else
        throw e;
      AlertWithTitle("Error.title", msg_key);
  } }
  return compsample;
}

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
    dump("runFixup(): "+e);
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
    dump("doEvalComputation(): " + e);
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
  fp.init(window, "Open TeX File", msIFilePicker.modeOpen);     // BBM todo -- use properties file here for the strings
  fp.appendFilter("TeX files", "*.tex; *.ltx; *.shl");
  fp.appendFilters(msIFilePicker.filterXML)

 // SetFilePickerDirectory(fp, "tex");


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
//       The output directory where the auxiliary files that are generated (such as .css, etc.) go.           //
//         This is a generated directory in the temp directory. It will be taken out of the temp directory    //
//         once the user saves the document.                                                                  //
//       The MathML conversion directory. This is where the DLL used to convert math and its associated .gmr  //
//         files are. This is usuall prince.                                                                  //
//       The input .tex file.                                                                                 //
//       The output .xhtml file, which is usually in the output directory.                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    msiSaveFilePickerDirectory(fp, "tex");
    var filename = fp.file.leafName.substring(0,fp.file.leafName.lastIndexOf("."));
    var infile =  "\""+fp.file.path+"\"";
    dump("Open Tex: " + infile);
    var outfile = dsprops.get("TmpD", Components.interfaces.nsIFile);
    outfile.append(filename);
    var outdir = outfile.clone();
    if (!outdir.exists()) outdir.create(1 , 0755);
    outfile.append("doc.xhtml");
    var css = outdir.clone();
    css.append("my.css");
    if (css.exists()) css.remove(false); 
    css.create(0 , 0755);
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
         dump(ex);
    }      
//  TODO BBM todo: we may need to run a merge program to bring in processing instructions for specifying tag property files
    
    msiEditPage("file:///" + outfile.path.replace(/\\/g,"/"), window, true);
  }                       
}

function documentAsTeX( document, xslSheetPath )
{
  var str = "";
  if (!document) return str;
  if (xslSheetPath.length == 0) return str;
  var xsltProcessor = new XSLTProcessor();
  var myXMLHTTPRequest = new XMLHttpRequest();
  myXMLHTTPRequest.open("GET", xslSheetPath, false);
  myXMLHTTPRequest.send(null);

  var xslStylesheet = myXMLHTTPRequest.responseXML;
  xsltProcessor.importStylesheet(xslStylesheet);
  var newDoc = xsltProcessor.transformToDocument(document);
  str = newDoc.documentElement.textContent;
  dump("\n"+str);
  return str;
}

function documentAsTeXFile( document, xslSheetPath, outputFile )
{
  if (outputFile && outputFile.path.length > 0) 
  {
    var str = documentAsTeX(document, xslSheetPath );
    if (outputFile.exists()) 
    outputFile.remove(false);
    outputFile.create(0, 0755);
    var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
    fos.init(outputFile, -1, -1, false);
    if (str.length > fos.write(str, str.length))
    {
      dump("Wrote fewer bytes than expected!\n");
   }
   fos.close();
  }
}


function currentFileName()
{
//  var docUrl = GetDocumentUrl();
  var editorElement = msiGetTopLevelEditorElement();
  var docUrl = msiGetEditorURL(editorElement);
  dump('\nThis doc url = ' + docUrl);
  var filename = "";
  if (docUrl && !IsUrlAboutBlank(docUrl))
    filename = GetFilename(docUrl);
  // BBM todo: we should check to see if the parent is a .swd directory, and if so, use that name  
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
   fp.appendFilters(msIFilePicker.filterText);
   fp.appendFilter("TeX files", "*.tex; *.ltx");
   fp.defaultExtension = ".tex";
   try 
   {
     fp.show();
   }
   catch (ex) 
   {
     dump("filePicker threw an exception\n");
   }
   documentAsTeXFile(editor.document, "chrome://prnc2ltx/content/latex.xsl", fp.file );
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
//  exefile.append("TeX"); exefile.append("bin"); 
  if (pdftex)
    exefile.append("pdflatex.cmd");
  else
    exefile.append("tex.cmd");
// the following is an egreqious hack. How do we find the path we need in general.
//  var execpath = "/usr/local/teTeX/bin/i386-apple-darwin-current/pdflatex";
//  var exefile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
//  exefile.initWithPath( execpath );
  dump("\nexecutable file: "+exefile.path+"\n");
  // BBM todo: On Mac OSX we want to put up a shell for possible interaction.
  try 
  {
    var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theProcess.init(exefile);
    var args = ["-output-directory", outputDir, infilePath];
    for (var i = 0; i < passCount; i++)
    {
      // BBM todo: We need to build the dialog box to display the number of passes
      theProcess.run(true, args, args.length);
    } 
  } 
  catch (ex) {
    dump("\nUnable to run TeX:\n");
    dump(ex);
    return false;
  }
  // check for a dvi or pdf file
  var outfileLeaf = infileLeaf;
  if (pdftex)
    outfileLeaf += ".pdf";
  else
    outfileLeaf += ".dvi";
  dump("\nOutputleaf="+outfileLeaf);
  var outputfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  outputfile.initWithPath( outputDir );
  outputfile.append(outfileLeaf);
  dump("\nFinal output filename: "+outputfile.path+"\n");
  return true;//outputfile.exists();
}


function printTeX( pdftex )
{
  var editor = GetCurrentEditor();
  if (!editor) return;
  var str = documentAsTeX(editor.document, "chrome://prnc2ltx/content/latex.xsl" );
// now save this TeX string and run TeX on it.  
// BBM todo: We want the directory name above the file.
  var editorElement = msiGetTopLevelEditorElement();
  var docUrl = msiGetEditorURL(editorElement);
//  var docUrl = GetDocumentUrl();
  var scheme, filebase;
  if (docUrl && !IsUrlAboutBlank(docUrl))
  {
    scheme = GetScheme(docUrl);
    filebase = GetFilename(docUrl);
  }
  var filename = filebase.substring(0,filebase.lastIndexOf(".")) + ".tex";
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var texfile = dsprops.get("TmpD", Components.interfaces.nsILocalFile);
  texfile.append(filename);
  texfile.createUnique(0, 0755);
  // capture the file name that was constructed
  var texfileLeaf = texfile.leafName;
  texfileLeaf = texfileLeaf.substring(0, texfileLeaf.lastIndexOf("."));
  var outputfile = dsprops.get("TmpD", Components.interfaces.nsILocalFile);
  dump("\ntexfile="+texfile.path);  
  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(texfile, -1, -1, false);
  if (str.length > fos.write(str, str.length))
  {
    dump("Wrote fewer bytes than expected!\n");
  }
  fos.close();
  if (compileTeXFile(pdftex, texfileLeaf, texfile.path, outputfile.path, 1))
  {
    if (pdftex)
      texfileLeaf += ".pdf";
    else
      texfileLeaf += ".dvi";
    outputfile.append(texfileLeaf);
    dump("outputfile to be launched: "+outputfile.path+"\n");
    outputfile.launch();
  }
  else
    dump("\nRunning TeX failed to create a file!");
}

function previewTeX(pdftex)
{
  printTeX(pdftex);
};

function compileTeX(pdftex)
{
  var editorElement = msiGetTopLevelEditorElement();
  var docUrl = msiGetEditorURL(editorElement);
//  var docUrl = GetDocumentUrl();
  dump('\nThis doc url = ' + docUrl);
  var scheme, filename;
  if (docUrl && !IsUrlAboutBlank(docUrl))
  {
    scheme = GetScheme(docUrl);
    filename = GetFilename(docUrl);
  }
  dump('\nThis doc = ' + filename);

  if (filename.length < 0)
     return;     
  var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
  fp.init(window, "Save "+(pdftex?"PDF":"DVI")+" file", msIFilePicker.modeSave);

  // BBM todo: we need to set up file pickers for pdf and dvi files 
  fp.appendFilters(msIFilePicker.filterAll);

  try 
  {
    fp.show();
    // need to handle cancel (uncaught exception at present) 
  }
  catch (ex) 
  {
    dump("filePicker threw an exception\n");
  }
  
  if (fp.file && fp.file.path.length > 0) 
  {
     var exportfile =  fp.file.path;
    	dump("\nCompiled file: " + exportfile);
  }
  var editor = GetCurrentEditor();
  if (!editor) return;
  var texfile = dsprops.get("TmpD", Components.interfaces.nsILocalFile);
  texfile.append("temp.tex");
  texfile.createUnique(0, 0755);
  // capture the file name that was constructed
  var texfileLeaf = texfile.leafName;
  texfileLeaf = texfileLeaf.substring(0, texfileLeaf.lastIndexOf("."));
  var outputDir = dsprops.get("TmpD", Components.interfaces.nsILocalFile);
 
  documentAsTeXFile(editor.document, "chrome://prnc2ltx/content/latex.xsl", texfile );
  if (compileTeXFile( pdftex, texfileLeaf, texfile.path, outputDir.path, 1 ))
  {
    if (pdftex)
      texfileLeaf += ".pdf";
    else
      texfileLeaf += ".dvi";
    outputDir.append(texfileLeaf);  
    // move the output result to the place indicated by fp.
    outputDir.move(fp.file.parent, fp.file.leafName);  
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
