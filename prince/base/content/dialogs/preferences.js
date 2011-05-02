function setIndex(item,idx) {
  if (document.getElementById(item)==null) dump("Bad id: "+item+"\n");
  document.getElementById(item).selectedIndex = idx;
}

function setChecked(item,val) {
  if (document.getElementById(item)==null) dump("Bad id: "+item+"\n");
  document.getElementById(item).setAttribute("checked", val == 1 ? "true" : "false");
}

function setValue(item,val) {
  if (document.getElementById(item)==null) dump("Bad id: "+item+"\n");
  document.getElementById(item).value = val;
}


function getIndex(item) {
  if (document.getElementById(item)==null) dump("Bad id: "+item+"\n");
  return document.getElementById(item).selectedIndex;
}

function getChecked(item) {
  if (document.getElementById(item)==null) dump("Bad id: "+item+"\n");
  return document.getElementById(item).getAttribute("checked") == "true" ? 1 : 0;
}

function getValue(item) {
  if (document.getElementById(item)==null) dump("Bad id: "+item+"\n");
  return document.getElementById(item).value;
}


var data;

function initialize()
{
  var url;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  var dir =dsprops.get("resource:app", Components.interfaces.nsIFile);
  data = window.arguments[0];
  dir.append("shells");
  url = msiFileURLFromFile(dir);
  var tree = document.getElementById("dir-tree");
  tree.setAttribute("ref", url.spec);
  tree.currentIndex = 0;
  showShellsInDir(tree);

  var principal = document.getElementById("principal");
  principal.setAttribute("checked", data.principal.toString());

  var special = document.getElementById("special");
  special.setAttribute("checked", data.special.toString());

  var logSent = document.getElementById("logSent");
  logSent.setAttribute("checked", data.logSent.toString());

  var engSent = document.getElementById("engSent");
  engSent.setAttribute("checked", data.engSent.toString());

  var engReceived = document.getElementById("engReceived");
  engReceived.setAttribute("checked", data.engReceived.toString());

  var logReceived = document.getElementById("logReceived");
  logReceived.setAttribute("checked", data.logReceived.toString());
  
  
  setValue("degree",data.degree.toString());
  setValue("digitsRendered", data.digitsRendered.toString());
  setValue("digitsUsed", data.digitsUsed.toString());
  setValue("lower", data.lower.toString());
  setValue("upper", data.upper.toString());
  setValue("primesasn", data.primesasn.toString());

  setChecked("mixednum", data.mixednum);
  setChecked("trigargs", data.trigargs);
  setChecked("usearc", 1 - data.usearc);
  setChecked("logs", data.loge);
  setChecked("dots", data.dotderiv);
  setChecked("bar", data.barconj);
  setChecked("i_imaginary", data.i_imaginary);
  setChecked("j_imaginary", data.j_imaginary);
  setChecked("e_exp", data.e_exp);
  setChecked("primederiv", data.primederiv);
  
  setIndex("matrix_delim", data.matrix_delim);

  setIndex("derivformat", data.derivformat);
  setIndex("imagi", data.imaginaryi);
  setIndex("diffD", data.diffD);
  setIndex("diffd", data.diffd);
  setIndex("expe", data.expe);
  // This shouldn't be necessary. Why are these dialog buttons hidden?
  var buttons = new Object();
  var kids = document.childNodes;
  var thisdialog=kids[3];
  buttons.accept = document.getAnonymousElementByAttribute(thisdialog, "dlgtype", "accept");
  buttons.cancel = document.getAnonymousElementByAttribute(thisdialog, "dlgtype", "cancel");
  buttons.accept.hidden=false;
  buttons.accept.removeAttribute("disabled");
  buttons.cancel.hidden=false;
}
//  ComputeUserSettingsStartup();

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

function onCancel () {
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

   data.digitsUsed = getValue("digitsUsed");
   data.digitsRendered = getValue("digitsRendered");
   data.degree = getValue("degree");

   data.lower = getValue("lower");
   data.upper = getValue("upper");
   data.primesasn = getValue("primesasn");
   data.mixednum = getChecked("mixednum");
   data.trigargs = getChecked("trigargs");

   data.usearc = 1 - getChecked("usearc");  // ?
   data.loge = getChecked("logs");
   data.dotderiv = getChecked("dots");
   data.principal = getChecked("principal");
   data.special = getChecked("special");

   data.barconj = getChecked("bar");

   data.i_imaginary = getChecked("i_imaginary");
   data.j_imaginary = getChecked("j_imaginary");


   data.e_exp = setChecked("e_exp");
   data.primederiv = getChecked("primederiv");
  
   data.matrix_delim = getIndex("matrix_delim");
   
   data.derivformat = getIndex("derivformat");
   data.imaginaryi  = getIndex("imagi");
   data.diffD = getIndex("diffD");
   data.diffd = getIndex("diffd");
   data.expe  = (getIndex("expe") != 0);

   
   msiComputeLogger.LogMMLSent(data.logSent);
   msiComputeLogger.LogMMLReceived(data.logReceived);
   msiComputeLogger.LogEngSent(data.engSent);
   msiComputeLogger.LogEngReceived(data.engReceived);

}



