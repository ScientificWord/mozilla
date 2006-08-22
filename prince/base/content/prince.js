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
      editPage(fp.fileURL.spec, window, false);
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

function appendResult(result,sep,math) {
  GetCurrentEditor().insertHTMLWithContext(
      result.replace(fullmath,fullmath+sep),
      "", "", "", null,
      math, math.childNodes.length, false );
  coalescemath();
}

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

