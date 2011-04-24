
function initialize()
{
  var url;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  var dir =dsprops.get("resource:app", Components.interfaces.nsIFile);
  dir.append("shells");
  url = msiFileURLFromFile(dir);
  var tree = document.getElementById("dir-tree");
  tree.setAttribute("ref", url.spec);
  tree.currentIndex = 0;
  showShellsInDir(tree);
  // This shouldn't be necessary. Why are these dialog buttons hidden?
  var buttons = new Object();
  var kids = document.childNodes;
  var thisdialog=kids[3];
  buttons.accept = document.getAnonymousElementByAttribute(thisdialog, "dlgtype", "accept");
  buttons.cancel = document.getAnonymousElementByAttribute(thisdialog, "dlgtype", "cancel");
  buttons.accept.hidden=false;
  buttons.accept.removeAttribute("disabled");
  buttons.cancel.hidden=false;
//  ComputeUserSettingsStartup();
}                           

function showShellsInDir(tree)
{
  var regexp = /\.sci$/i;
  var namecol = tree.columns.getNamedColumn('Name');
  var i = tree.currentIndex;
  var leafname = tree.view.getCellText( i,namecol);
  while (tree.view.getParentIndex(i) >= 0)
  {           
    i = tree.view.getParentIndex(i);
    leafname = tree.view.getCellText(i,namecol)+ "/" + leafname;
  }
  var directory;
  var dirurl = msiURIFromString(tree.getAttribute("ref"));
  try {
    directory = msiFileFromFileURL(dirurl);
    directory.append(leafname);
    var items = directory.directoryEntries;
    var name;  
    var listbox = document.getElementById("dircontents");
    while (listbox.itemCount > 0) listbox.removeItemAt(0);
    while (items.hasMoreElements()) {
      var item = items.getNext().QueryInterface(Components.interfaces.nsIFile);
      if (item.isFile() && regexp.test(item.leafName))
      {
        name = item.leafName.replace(regexp,'');
        listbox.appendItem(name, item.path);
      }
    }
    listbox.selectedIndex =0; 
  }
  catch(e) {
    dump(e.toString());
  }
}
function UserSettingsStartup(){
}

function onCancel()
{
  return true;
}

function onAccept(){
  try {
    document.getElementById("prefGeneral").writePreferences(true);
    document.getElementById("prefEdit").writePreferences(true);
    document.getElementById("prefComp").writePreferences(true);
    document.getElementById("prefPlots").writePreferences(true);
    document.getElementById("prefTypesetting").writePreferences(true);
  }
  catch(e) {
    dump(e.toString());
  }
//  document.getElementById("prefAdvanced").writePreferences(true);

//   data.digitsUsed = getValue("digitsUsed");
//   data.digitsRendered = getValue("digitsRendered");
//   data.degree = getValue("degree");
//
//   data.lower = getValue("lower");
//   data.upper = getValue("upper");
//   data.primesasn = getValue("primesasn");
//   data.mixednum = getChecked("mixednum");
//   data.trigargs = getChecked("trigargs");
//
//   data.usearc = 1 - getChecked("usearc");  // ?
//   data.loge = getChecked("logs");
//   data.dotderiv = getChecked("dots");
//   data.principal = getChecked("principal");
//   data.special = getChecked("special");
//
//   data.barconj = getChecked("bar");
//
//   data.i_imaginary = getChecked("i_imaginary");
//   data.j_imaginary = getChecked("j_imaginary");
//
//
//   data.e_exp = setChecked("e_exp");
//   data.primederiv = getChecked("primederiv");
//  
//   data.matrix_delim = getIndex("matrix_delim");
//   
//   data.derivformat = getIndex("derivformat");
//   data.imaginaryi  = getIndex("imagi");
//   data.diffD = getIndex("diffD");
//   data.diffd = getIndex("diffd");
//   data.expe  = (getIndex("expe") != 0);
//
//   
//   msiComputeLogger.LogMMLSent(data.logSent);
//   msiComputeLogger.LogMMLReceived(data.logReceived);
//   msiComputeLogger.LogEngSent(data.engSent);
//   msiComputeLogger.LogEngReceived(data.engReceived);
//
}



