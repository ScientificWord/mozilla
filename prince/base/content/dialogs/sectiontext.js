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
  {
    initialStr="<dialogbase xmlns='http://www.w3.org/1999/xhtml'></dialogbase>";
  }
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
  var theNode = null;
  while (theNodes && theNodes.length>0) {
    theNode = theNodes[0];
    theNodes = theNode.getElementsByTagName("dialogbase");
  }  // this is a workaround for a bug which causes <dialogbase> to be nested in a chain.
  // The better solution is to keep this from happening  
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


function onAccept()
{
  stashTitlePrototype();
  SaveWindowLocation();
  close();
  return (false);
}

function onCancel() {
  close();
  return(true);
}

