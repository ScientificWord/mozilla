var titleformat;
var seclevel;
var units;

function Startup() {
  var initialStr="";
//  if (titleformat[seclevel]) initialStr = titleformat[seclevel];
  var re = /<sw:dialogbase[^>]*>/;
  var s = initialStr.replace(re, "");
  var re2 = /<\/sw:dialogbase>/;
  initialStr = s.replace(re2,"");
  gDialog.bDataModified = false;
  gDialog.bEditorReady = false;

  var editElement = document.getElementById("note-frame");
  msiInitializeEditorForElement(editElement, initialStr);
}
                      
function getBaseNode( )
{
  var sw = "http://www.sciword.com/namespaces/sciword";
  var editElement = document.getElementById("note-frame");
  var doc = editElement.contentDocument;
  var theNodes = doc.getElementsByTagNameNS(sw,"dialogbase");
  var theNode;
  if (theNodes) theNode = theNodes[0]; 
  else 
  {
    theNodes = doc.getElementsByTagNameNS(sw,"para");
    if (theNodes) theNode = theNodes[0]; 
  }
  if (!theNode)
  {
    var bodies = doc.getElementsByTagName("body");
    if (bodies) for ( var i = 0; i < bodies.length; i++)
    {
      theNode = bodies[i];
      if (theNode.nodeType == theNode.ELEMENT_NODE)
        return theNode;
     }
  }
  return theNode;
}

function stashTitlePrototype()
{
  var sourceNode = getBaseNode();
  if (!sourceNode) return;
  var ser = new XMLSerializer();
  var xmlcode = ser.serializeToString(sourceNode);
  dump(xmlcode+"\n");
//  var re = /<sw:dialogbase[^>]*>/;
//  var s = xmlcode.replace(re, "");
//  var re2 = /<\/sw:dialogbase>/;
//  titleformat[seclevel] = s.replace(re2,"");
//  titleformat[seclevel] = xmlcode;
//  titleformat.refresh(titleformat.destNode);
//  dump("***** saving '"+titleformat[seclevel]+"'\n");
// we still need to replace the text of the section title area in the main dialog.
}

function onOK() {
// Copy the contents of the editor to the section header form
  stashTitlePrototype();
  SaveWindowLocation();
  close();
  return (false);
}

function onCancel() {
  close();
  return(true);
}

