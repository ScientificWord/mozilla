
var currentpara;
var totalparas;
var currentmode;
var doc;
var editor;
var editorelement;

function resolver(prefix) {
  var ns = {
    'html' : 'http://www.w3.org/1999/xhtml',
    'mml'  : 'http://www.w3.org/1998/Math/MathML',
    'sw'   : 'http://www.sciword.com/namespaces/sciword'
  }
  return ns[prefix] || 'http://www.w3.org/1999/xhtml';
} 

function initialize()
{
  editor = window.arguments[0];
  var context = window.arguments[1];
  editorelement = window.arguments[2];
  var xpathresult, xpath;
  if (!editor) return;
  doc = editor.document;
  try {
    xpath = getTagsXPath(editor, "paratag");
    xpath = xpath.replace('html:','preceding::html:','g');
    xpath = "count("+xpath+")";
    xpathresult = doc.evaluate(xpath, context, resolver, XPathResult.NUMBER_TYPE, null );
    currentpara = 1+ xpathresult.numberValue;
    xpath = xpath.replace('preceding','following','g');
    xpathresult = doc.evaluate(xpath, context, resolver, XPathResult.NUMBER_TYPE, null );
    totalparas = xpathresult.numberValue + currentpara;
    currentmode = "";
    document.getElementById("current").value = currentpara;
    document.getElementById("total").value = totalparas;
    setmode();
    if (currentmode == "abs") {
      document.getElementById("paragraphnumber").value=currentpara;
    }
    else {
      document.getElementById("paragraphnumber").value=0;
    }
  }
  catch(e) {
    dump("gotoparagraph.initialized failed: "+e);
  }
}

function onAccept()
{
  try {
    var n = document.getElementById('paragraphnumber').value;
    var xpath;
    if (currentmode=="rel") n = Number(n) + Number(currentpara);
    xpath = getTagsXPath(editor, "paratag");
    xpath = xpath.replace("html:","//html:","g");
    dump(xpath+"\n");
    var xpathresult = doc.evaluate(xpath, doc, resolver, XPathResult.ORDERED_NODE_SNAPSHOT_TYPE, null );
    dump(xpathresult.snapshotLength+" patterns found, n is " + n +"\n");

    var node = xpathresult.snapshotItem(n-1); //snapshot list is 0-based??
    if (node) 
    {
      dump("Found node = "+node.localName+"\n");
      gotoFirstNonspaceInElement(editor,node);
      editorelement.focus();
    }
  }
  catch(e) {
    alert("onAccept, error = " + e.message);
  }
}

function onCancel()
{
}

function mode()
{
  // return the mode, absolute or relative
  var radiogroup = document.getElementById("mode");
  currentmode = radiogroup.value;
  return currentmode;
}

function setmode() // called after the radiogroup has changed.
{
  var savecurrent, numberbox, temp;
  savecurrent = currentmode;
  currentmode = mode();
  if (savecurrent != currentmode)
  {
    numberbox = document.getElementById("paragraphnumber");
    if (currentmode == "abs") // going from relative to abs mode
    {
      numberbox.setAttribute("max", totalparas);
      numberbox.setAttribute("min", 1); 
      numberbox.value+= Number(currentpara);
      document.getElementById("abslabel").removeAttribute("hidden");
      document.getElementById("rellabel").setAttribute("hidden","true");
    }
    else
    {
      numberbox.setAttribute("max", totalparas - currentpara);
      numberbox.setAttribute("min", 1 - currentpara); 
      numberbox.value -= currentpara;
      document.getElementById("rellabel").removeAttribute("hidden");
      document.getElementById("abslabel").setAttribute("hidden","true");
    }
  }
}
