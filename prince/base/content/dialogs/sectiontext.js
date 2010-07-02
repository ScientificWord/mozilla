var titleformat;
var seclevel;
var units;

function Startup() {
  titleformat = window.arguments[0];
  seclevel = window.arguments[1];
  seclevel = seclevel.toLowerCase();
  dump("seclevel = "+seclevel+"\n");
  units = window.arguments[2];
  units = units.toLowerCase();
  dump("units = "+units+"\n");
  var initialStr="<dialogbase><br/></dialogbase>";
  if (titleformat[seclevel]) initialStr = titleformat[seclevel].proto;
//  var re = /<dialogbase[^>]*>/;
//  var s = initialStr.replace(re, "");
//  var re2 = /<\/dialogbase>/;
//  initialStr = s.replace(re2,"");
  gDialog.bDataModified = false;
  gDialog.bEditorReady = false;

  var editElement = document.getElementById("sectiontitle-frame");
  msiInitializeEditorForElement(editElement, initialStr);
}
                      
function getBaseNode( )
{
  var editElement = document.getElementById("sectiontitle-frame");
  var doc = editElement.contentDocument;
  var theNodes = doc.getElementsByTagName("dialogbase");
  var theNode;
  if (theNodes) theNode = theNodes[0]; 
  else 
  {
    theNodes = doc.getElementsByTagName("para");
    if (theNodes) theNode = theNodes[0]; 
  }
//  if (!theNode)
//  {
//    var bodies = doc.getElementsByTagName("body");
//    if (bodies) for ( var i = 0; i < bodies.length; i++)
//    {
//      theNode = bodies[i];
//      if (theNode.nodeType == theNode.ELEMENT_NODE)
//        return theNode;
//     }
//  }
  return theNode;
}

function stashTitlePrototype()
{
  var sourceNode = getBaseNode();
  if (!sourceNode) return;
  var ser = new XMLSerializer();
  var xmlcode = ser.serializeToString(sourceNode);
  titleformat[seclevel].proto = xmlcode;
  titleformat.refresh(titleformat.destNode);
  dump("***** saving '"+titleformat[seclevel]+"'\n");
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

