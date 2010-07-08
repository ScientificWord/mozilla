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
  var initialStr;
  if (titleformat[seclevel].proto) initialStr = titleformat[seclevel].proto;
  if (initialStr) {
    var re = /<titleprototype[^>]*>/;
    var s = initialStr.replace(re, "");
    var re2 = /<\/titleprototype>/;
    initialStr = s.replace(re2,"");
  }
  if (!(initialStr && initialStr.length > 0))
    initialStr="<dialogbase><br/></dialogbase>";
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
  else return null;
//  else 
//  {
//    theNodes = doc.getElementsByTagName("para");
//    if (theNodes) theNode = theNodes[0]; 
//  }
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
  try {
    var sourceNode = getBaseNode();
    if (!sourceNode) return;
    var ser = new XMLSerializer();
    var xmlcode = ser.serializeToString(sourceNode);
    titleformat[seclevel].proto = xmlcode;
    titleformat.refresh(titleformat.destNode);
  }
  catch(e) {
    dump("***** saving '"+titleformat[seclevel]+"'\n"+e.message);
  }
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

