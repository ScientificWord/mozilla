var structureData;
var element;

function capitalize( str )
{
  return str.slice(0,1).toLocaleUpperCase()+str.slice(1);
}



function startup()
{
  structureData = window.arguments[0];
  dump("structureproperties startup()\n");
  document.title = capitalize(document.title.replace("##", structureData.tagName));
  var str;
  str = capitalize(document.getElementById("textboxlabel").getAttribute("value"));
  document.getElementById("textboxlabel").setAttribute("value",
    capitalize(str.replace("##", structureData.tagName)));
  str = document.getElementById("numberedCheckbox").getAttribute("label");
  document.getElementById("numberedCheckbox").setAttribute("label",
    capitalize(str.replace("##", structureData.tagName)));
  try {
    var editorElement = msiGetParentEditorElementForDialog(window);  
    var mixed = msiGetSelectionContainer(editorElement);
    if (!mixed) return;
    element = mixed.node;
    if (!element) return;
    while (element && element.localName != structureData.tagName) 
      element = element.parentNode;
    var filename;
    if (element)
    {
      // save the starting state
      if (element.hasAttribute("open")) structureData.open = element.getAttribute("open")=="true"?true:false;
      if (element.hasAttribute("subdoc"))
      {
        document.getElementById("subdocCheckbox").checked = true;
        filename = element.getAttribute("subdoc");
        structureData.subdoc = filename;
        if (!(filename && filename.length > 0))
          filename = generateFilename(element);
        //validate here??
        document.getElementById("textboxlabel").disabled = false;
        document.getElementById("subdocFileName").disabled = false;
        document.getElementById("subdocFileName").value = filename;
      } else document.getElementById("subdocCheckbox").checked = false;
      if (element.hasAttribute("nonum"))
        document.getElementById("numberedCheckbox").checked = false;
    }
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
  var sectitle = element.firstChild;
  while (sectitle && sectitle.nodeType != Node.ELEMENT_NODE) sectitle = sectitle.nextSibling;
  if (sectitle)
  { 
    // if it is currently written to disk, and this is changing, or the file name is changing, read it back in
    if (!structureData.open && structureData.subdoc && 
      (!element.hasAttribute("subdoc") || element.getAttribute("subdoc") != structureData.subdoc))
      sectitle.getFromDisk(structureData.subdoc);
    if (element.hasAttribute("subdoc") && element.getAttribute("open") == "false")  // then we need to write the contents to disk.
    {                                                                   
      sectitle.saveToDisk(element.getAttribute("subdoc"));
    }
    if (document.getElementById("subdocCheckbox").checked)
      element.setAttribute("subdoc",document.getElementById("subdocFileName").value);
    else 
      element.removeAttribute("subdoc");
    if (!document.getElementById("numberedCheckbox").checked)
      element.setAttribute("nonum","true");
    else
      element.removeAttribute("nonum"); 
    
  }
  return true;    
}