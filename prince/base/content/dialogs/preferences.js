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
  ComputeUserSettingsStartup();
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




function setIndex(item,idx) {
  document.getElementById(item).selectedIndex = idx;
}

function setChecked(item,val) {
  document.getElementById(item).setAttribute("checked", val == 1 ? "true" : "false");
}

function setValue(item,val) {
  document.getElementById(item).value = val;
}



var data;

function ComputeUserSettingsStartup(){
  data = window.arguments[0];

  setValue("digits", data.sig_digits.toString());
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
  setIndex("expe", data.expe ? 1 : 0);
}

function getIndex(item) {
  return document.getElementById(item).selectedIndex;
}

function getChecked(item) {
  return document.getElementById(item).getAttribute("checked") == "true" ? 1 : 0;
}

function getValue(item) {
  return document.getElementById(item).value;
}

function onAccept(){
   data.sig_digits = getValue("digits");
   data.lower = getValue("lower");
   data.upper = getValue("upper");
   data.primesasn = getValue("primesasn");
   data.mixednum = getChecked("mixednum");
   data.trigargs = getChecked("trigargs");

   data.usearc = 1 - getChecked("usearc");  // ?
   data.loge = getChecked("logs");
   data.dotderiv = getChecked("dots");

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
  

}



