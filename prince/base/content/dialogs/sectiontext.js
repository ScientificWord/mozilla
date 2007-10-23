var titleformat;
var seclevel;
var units;

function Startup() {
  titleformat = window.arguments[0];
  seclevel = window.arguments[1];
  dump("seclevel = "+seclevel+"\n");
  units = window.arguments[2];
  dump("units = "+units+"\n");
  var substitutionStr = "";
  gDialog.bDataModified = false;
  gDialog.bEditorReady = false;

  var editElement = document.getElementById("sectiontitle-frame");
  msiInitializeEditorForElement(editElement, substitutionStr);
}
                      
function findBasePara( )
{
  var sw = "http://www.sciword.com/namespaces/sciword";
  var editElement = document.getElementById("sectiontitle-frame");
  if (!editElement) return;
  var editor = msiGetEditor(editElement);
  var doc = editor.document;
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
//  var editElement = document.getElementById("sectiontitle-frame");
//  if (!editElement) return;
//  var editor = msiGetEditor(editElement);
  var sectitlenode;
  try {
     sectitlenode = data.sectitle;
  }
  catch(e) {
    sectitlenode = document.createElement("sectitle");
  }
  var titleprotonode = document.createElement("titleprototype");
  sectitlenode.appendChild(titleprotonode);
  var sourceNode = getBaseNode();
  if (!sourceNode) return;
  var node;
  for (i=0; i < sourceNode.childNodes.length; i++)
  {
    node = sourceNode.childNodes[i];
    titleprotonode.appendChild(node.cloneNode(true));
  }
  // now dump to see if we have the right thing. It would be more revealing to use
  // an XML serializer
  dump("\n** " + titleprotonode.textContent + "\n");
  acceptDialog();
}
  
  
  





function onOK() {
// Copy the contents of the editor to the section header form
  stashTitlePrototype();
  SaveWindowLocation();
  return false;
}

function onCancel() {
  return(true);
}

