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

function onAccept()
{
  window.arguments[0].filename = document.getElementById("dircontents").value;
  return true;
}


function onCancel()
{
  window.arguments[0].filename = null;
  return true;
}
