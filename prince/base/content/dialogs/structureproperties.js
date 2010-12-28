var structureData = new Object();
//var structNode;

function capitalize( str )
{
  return str.slice(0,1).toLocaleUpperCase()+str.slice(1);
}



function startup()
{
//  structureData = window.arguments[0];
  structureData.structNode = window.arguments[0];
  if (!structureData.structNode)
    return;
  var tagName = msiGetBaseNodeName(structureData.structNode);

  var sectitle = structureData.structNode.firstChild;
  while (sectitle && sectitle.nodeType != Node.ELEMENT_NODE)
    sectitle = sectitle.nextSibling;
  structureData.secTitleNode = sectitle;  //save for later
  dump("structureproperties startup()\n");

  document.title = capitalize(document.title.replace("##", tagName));
  var str;
  str = capitalize(document.getElementById("textboxlabel").getAttribute("value"));
  document.getElementById("textboxlabel").setAttribute("value", capitalize(str.replace("##", tagName)));
  str = document.getElementById("numberedCheckbox").getAttribute("label");
  document.getElementById("numberedCheckbox").setAttribute("label", capitalize(str.replace("##", tagName)));

  var shortFormEditor = document.getElementById("sectionShortEdit");
  try {
//  var editorElement = msiGetParentEditorElementForDialog(window);  
//  var mixed = msiGetSelectionContainer(editorElement);
//  if (!mixed) return;
//  element = mixed.node;
//  if (!element) return;
//  while (element && element.localName != structureData.tagName) 
//  element = element.parentNode;
    var filename;
    var children;
    // save the starting state
    if (structureData.structNode.hasAttribute("open"))
      structureData.open = structureData.structNode.getAttribute("open")=="true" ? true : false;
    if (sectitle)
      children = msiNavigationUtils.getSignificantContents(sectitle);
    for (var ix = 0; !structureData.shortFormNode && (ix < children.length); ++ix)
    {
      if (msiGetBaseNodeName(children[ix]) == "shortTitle")
        structureData.shortFormNode = children[ix];
    }
    structureData.shortFormStr = "";
    if (structureData.shortFormNode)
    {
      var serializer = new XMLSerializer();
      var shortKids = msiNavigationUtils.getSignificantContents(structureData.shortFormNode);
      for (var jx = 0; jx < shortKids.length; ++jx)
        structureData.shortFormStr += serializer.serializeToString(shortKids[jx]);
    }
    var editorInitializer = new msiEditorArrayInitializer();
    editorInitializer.addEditorInfo(shortFormEditor, structureData.shortFormStr, true);
    editorInitializer.doInitialize();

    if (structureData.structNode.hasAttribute("subdoc"))
    {
      document.getElementById("subdocCheckbox").checked = true;
      filename = structureData.structNode.getAttribute("subdoc");
      structureData.subdoc = filename;
      if (!(filename && filename.length > 0))
        filename = generateFilename(structureData.structNode);
      //validate here??
      document.getElementById("textboxlabel").disabled = false;
      document.getElementById("subdocFileName").disabled = false;
      document.getElementById("subdocFileName").value = filename;
    }
    else
      document.getElementById("subdocCheckbox").checked = false;
    if (structureData.structNode.hasAttribute("nonum"))
      document.getElementById("numberedCheckbox").checked = false;
  }
  catch(e) {
    dump("Error in structureproperties.js, startup: "+e.message+"\n");
  }    
}

function validateFilename( filename )
{
  var temp = filename.replace(/[\n\t]/,"","g");;
  temp = temp.replace(/[^a-zA-Z_]/, "", "g");
  var ioService = Components.classes['@mozilla.org/network/io-service;1']  
        .getService(Components.interfaces.nsIIOService);  
  var fileHandler = ioService.getProtocolHandler('file')  
          .QueryInterface(Components.interfaces.nsIFileProtocolHandler);  
  var file = fileHandler.getFileFromURLSpec(element.baseURI);  
  file = file.parent; // and now it points to the working directory
  file.append(temp+".xml");
  while (temp.length == 0 || file.exists())   // we add ".nnn" to the end of temp as necessary
  {   // we also have to check for conflicts with other subdoc names that have not been written to the disk
    if (!/\./.test(temp)) temp += ".1";
    else
    {
      var regex = /([a-zA-Z0-9_]+)\.([0-9]+)/;
      var match = regex.exec(temp);
      if (match != null)
      {
        temp = match[1] + "." + (Number(match[2])+1).toString();
      }
      else temp+= ".1";
    }
  }
  return temp;
}     
     
function subdocCheckChanged( cb )
{
  if (cb.checked) 
  {
    document.getElementById("textboxlabel").disabled = false;
    document.getElementById("subdocFileName").disabled = false;
    if (document.getElementById("subdocFileName").value.length == 0)
      document.getElementById("subdocFileName").value = generateFilename(element);
  }
  else
  {
    document.getElementById("textboxlabel").disabled = true;
    document.getElementById("subdocFileName").disabled = true;
  }
}
    
function generateFilename( el )
{
  var child = el.firstChild;
  while (child && child.nodeType != Node.ELEMENT_NODE) child = child.nextSibling;
  var text = child.textContent;
  // build the name from the text content of the first child element (usually the sectiontitle)
  return validateFilename(text);
}    

function filenameChanged(tb)
{
  var fn = tb.value;
  fn = validateFilename(fn);
  if (fn != tb.value)
    tb.value = fn;
}


function onCancel() {
  return true;   
}

function onAccept()
{
  var reviseData = new Object();

  var sectitle = structureData.secTitleNode;
//  while (sectitle && sectitle.nodeType != Node.ELEMENT_NODE)
//    sectitle = sectitle.nextSibling;
  var shortFormEditor = document.getElementById("sectionShortEdit");
  var editorElement = msiGetParentEditorElementForDialog(window);
//  msiDumpWithID("In structure properties dialog, parent editorElement's id is [@].\n", editorElement);
  var parentEditor = msiGetEditor(editorElement);
  try
  {
    if (sectitle)
    { 
      // if it is currently written to disk, and this is changing, or the file name is changing, read it back in
      if (!structureData.open && structureData.subdoc && 
        (!structureData.structNode.hasAttribute("subdoc") || structureData.structNode.getAttribute("subdoc") != structureData.subdoc))
        sectitle.getFromDisk(structureData.subdoc);
      if (structureData.structNode.hasAttribute("subdoc") && structureData.structNode.getAttribute("open") == "false")  // then we need to write the contents to disk.
      {                                                                   
        sectitle.saveToDisk(structureData.structNode.getAttribute("subdoc"));
      }
      reviseData.subDocName = "";
      if (document.getElementById("subdocCheckbox").checked)
        reviseData.subDocName = document.getElementById("subdocFileName").value;

      reviseData.noNumAttr = "";
      if (!document.getElementById("numberedCheckbox").checked)
        reviseData.noNumAttr = "true";

      reviseData.newShortForm = "";
      var shortFormContentFilter = new msiDialogEditorContentFilter(shortFormEditor);
      reviseData.newShortForm = shortFormContentFilter.getDocumentFragmentString();
//      dump("In structure properties dialog, short form title is [" + reviseData.newShortForm + "].\n");
    }
  
    var theWindow = window.opener;
    if (!theWindow || !("doReviseStructureNode" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (parentEditor && theWindow)
      theWindow.doReviseStructureNode(parentEditor, structureData, reviseData);
  }
  catch(exc) {dump("In structureproperties dialog onAccept(), exception: [" + exc + "].\n");}
  return true;    
}
