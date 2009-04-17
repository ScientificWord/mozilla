var editordoc;
var editor;
var regex = /^resource:\/\/app/;
function appendlistitem(list, itemstring)
{
  var item = list.appendItem(itemstring);
  item.crop = "start";
  if (regex.test(itemstring)) // from the resource area, hence read-only
  {
    item.setAttribute("class","readonly");
  }
}
  
function startup()
{
// get a list of the style sheets used in the document and
// get a list of the tag definitions
  var docStyle = window.arguments[0];
  var cssList;
  var scriptList;
  var xmlList;
  var styleSheetList;
  var arr;
  var i;
  var uri;
  if (!docStyle.edElement) return;  //BBM think this through
  editor = msiGetEditor(docStyle.edElement);
  editordoc = editor.document;
  cssList =  document.getElementById("csslist");
  scriptList = document.getElementById("scriptlist");
  xmlList =  document.getElementById("xmllist");
  styleSheetList = editordoc.styleSheets;
  arr = [];
  // build an array of stylesheet names. 
  // we want to remove duplicates, but the later instances are more significant,
  // so we remove by going back to front.
  for (i = styleSheetList.length - 1; i>= 0;  i--)
  {
    uri = styleSheetList[i].href; 
  //relativize the names when we can
    if ("file" == GetScheme(uri)) uri = msiMakeRelativeUrl(uri);
    if (arr.indexOf(uri) === -1)
      arr.push(uri);
  }
  for (i = arr.length -1; i>=0; i--)
    appendlistitem(cssList, arr[i]);
  var tagdefsArr;
  arr=[];
  tagdefsArr = processingInstructionsList(editordoc, "sw-tagdefs");
  for (i = tagdefsArr.length - 1; i>= 0;  i--)
  {
    uri = tagdefsArr[i].href; 
  //relativize the names when we can
    if ("file" == GetScheme(uri)) uri = msiMakeRelativeUrl(uri);
    if (arr.indexOf(uri) === -1)
      arr.push(uri);
  }
  for (i = arr.length -1; i>=0; i--)
    appendlistitem(xmlList, arr[i]);
  var scripts = editordoc.getElementsByTagName("script");
  var scriptLength = scripts.length;
  for (i = 0; i< scriptLength; i++)
    appendlistitem(scriptList, scripts[i].getAttribute('src'));
}


function moveup(id)
{
  var i, j, item, listbox, items, newItem, isSelected;
  var isSelected;
  listbox=document.getElementById(id);
  if (!listbox) return;
  i = listbox.currentIndex;
  items=listbox.selectedItems;
  isSelected = items.indexOf(listbox.getItemAtIndex(i))>=0;
  if (i > 0)
  {
    item = listbox.removeItemAt(i);
    newItem = listbox.insertItemAt(i-1, item.getAttribute("label"), item.getAttribute("value"));
    if ("readonly" == item.getAttribute("class"))
      newItem.setAttribute("class", "readonly");
    listboxlistbox.currentIndex = i-1;
    if (isSelected) listbox.addItemToSelection(listbox.getItemAtIndex(i-1));
    listbox.focus();
  }
}

function movedown(id)
{
  var i, j, item, listbox, items, isSelected, newItem;
  listbox=document.getElementById(id);
  if (!listbox) return;
  i = listbox.currentIndex;
  items=listbox.selectedItems;
  isSelected = items.indexOf(listbox.getItemAtIndex(i))>=0;
  if (i < listbox.itemCount-1)
  {
    item = listbox.removeItemAt(i);
    newItem = listbox.insertItemAt(i+1, item.getAttribute("label"), item.getAttribute("value"));
    if ("readonly" == item.getAttribute("class"))
      newItem.setAttribute("class", "readonly");
    listbox.currentIndex = i+1;
    if (isSelected) listbox.addItemToSelection(listbox.getItemAtIndex(i+1));
    listbox.focus();
  }
}


function remove(id)
{
  var i, j, item, items, listbox;
  listbox=document.getElementById(id);
  if (!listbox) return;
  items = listbox.selectedItems;
  if (items && items.length > 0)
  {
    for (i = items.length -1; i >= 0; i--)
    { 
      item = listbox.removeItemAt(listbox.getIndexOfItem(items[i]));
    }
    listbox.focus();
  }
}

function add(listboxid, textboxid, extension)
{
  var newurl, listbox, textbox, docurl, docfilepath, docfile, selectedfile, selectedfilepath;
  listbox = document.getElementById(listboxid);
  textbox = document.getElementById(textboxid);
  if (!(listboxid && textboxid)) return;
  newurl = textbox.value;
// this is all we do when the url is "resource://app/... but in other cases we need to create
// a css or xml directory in the document working directory and copy the file into it.
  if (!newurl || newurl.length <= 5) ret8urn;
  switch (GetScheme(newurl))
  {
    case "file": break;
    case "resource":
      appendlistitem(listbox, newurl);
      return;
    case "http":
      appendlistitem(listbox, newurl);
      return;
    case "chrome":
      appendlistitem(listbox, newurl);
      return;
    default: return;
  }
  docurl = editordoc.URL;
  if (docurl)
  {
    docfilepath = GetFilepath(docurl);
    docfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  // for Windows
#ifdef XP_WIN32
    docfilepath = docfilepath.replace("/","\\","g");
#endif
    docfile.initWithPath( docfilepath ); // docfile now points to our document file
    docfile = docfile.parent; // docfile is now the working directory
    docfile.append(extension);
    if (!docfile.exists()) docfile.create(nsIFile::DIRECTORY_TYPE,0755);
    // now open the existing file and copy it.
    selectedfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
    selectedfilepath = GetFilepath(newurl)// for Windows
#ifdef XP_WIN32
    selectedfilepath = selectedfilepath.replace("/","\\","g");
#endif
    selectedfile.initWithPath( selectedfilepath ); // selectedfile now points to the selected file
    if (!selectedfile.exists()) return; //BBM need some feedback to the user here
    // check to see if a file with the destination name already exists.
    appendlistitem(listbox, extension+"/"+selectedfile.leafName);
    selectedfile.copyTo(docfile, "");
  }
}


function browse( extension )// extension is 'css', 'js', 'xml', 'xsl' or something for tex styles
{
  const msIFilePicker = Components.interfaces.nsIFilePicker;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
  var ucExtension = extension.toUpperCase();
  fp.init(window, GetString("Open"+ucExtension+"File"), msIFilePicker.modeOpen);     
  fp.appendFilter(GetString(ucExtension + "Files"), "*."+extension);
//  fp.appendFilters(msIFilePicker.filterXML)
  var basedir =dsprops.get("resource:app", Components.interfaces.nsIFile);
  var dispDir = basedir.clone();
  dispDir.append("res");
  dispDir.append(extension==="xml"?"tagdefs":extension);
  fp.displayDirectory = dispDir;
  

  try {
    fp.show();
    // need to handle cancel (uncaught exception at present) 
  }
  catch (ex) {
    dump("filePicker.chooseInputFile threw an exception\n");
    dump(e+"\n");
    
  }
  var chosenURL;
  if (basedir.contains(fp.file, true)) // we need to change the uri to use "resource://app/"
  {
    chosenURL = "resource://app";
    var dirarray =[];
    var f = fp.file.clone();
    while (basedir.contains(f, true))
    {
      dirarray.push(f.leafName);
      f = f.parent;
    }
    while (dirarray.length > 0) chosenURL += "/" + dirarray.pop();
  }
  else 
  {
    chosenURL = "file://" + fp.file.target;
    chosenURL = chosenURL.replace("\\","/","g");
  }
  dump("a\n");
  document.getElementById(extension+"file").value = chosenURL;
  dump("b\n");
}


function onAccept()
{
  // save css PI's
  // out with the old. Can't use editordoc.styleSheets because it is read-only
  deleteProcessingInstructions(editordoc,"xml-stylesheet");
  var xmlList;
  var i;
  var listbox = document.getElementById("csslist");
  for (i=0; i<listbox.itemCount; i++)
  {
    addProcessingInstruction(editordoc, "xml-stylesheet", listbox.getItemAtIndex(i).label, "text/css");
  }
  // save tagdefs PI's
  // out with the old.
  deleteProcessingInstructions(editordoc,"sw-tagdefs");
  xmlList =  document.getElementById("xmllist");
  for (i=xmlList.itemCount-1; i>=0; i--)
  {
    addProcessingInstruction(editordoc, "sw-tagdefs", xmlList.getItemAtIndex(i).label,"text/xml");
  }
  // still need to do something for the other tabs
} 

function onCancel()
{
  return true;
}