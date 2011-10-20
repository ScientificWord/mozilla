Components.utils.import("resource://app/modules/computelogger.jsm");


function myDump(aMessage) {
  var consoleService = Components.classes["@mozilla.org/consoleservice;1"]
                                 .getService(Components.interfaces.nsIConsoleService);
  consoleService.logStringMessage(aMessage);
}


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
  setTypesetFilePrefTestboxes();
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
    var listItem; 
    var listbox = document.getElementById("dircontents");
    while (listbox.itemCount > 0) listbox.removeItemAt(0);
    while (items.hasMoreElements()) {
      var item = items.getNext().QueryInterface(Components.interfaces.nsIFile);
      if (item.isFile() && regexp.test(item.leafName))
      {
        name = item.leafName.replace(regexp,'');
        listItem = listbox.appendItem(name, item.path);
      }
    }
    listbox.selectedIndex =0; 
  }
  catch(e) {
    myDump(e.toString());
  }
}

function onShellSelect()
{
  var i;
  var filename;
  filename = document.getElementById("dircontents").value;
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  var shelldir = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
  shelldir.append("shells");
  var shelldirs = shelldir.path.split(/\//);
  var filepathdirs = filename.split(/\//);
  for (i = 0; i < shelldirs.length; i++)
  {
    if (shelldirs[i] != filepathdirs[i])
    {
      throw("Non-shell path");
    } 
  }
  var relpath = filepathdirs.slice(shelldirs.length).join("/");
  var pref = document.getElementById("defaultshell");
  pref.value = relpath;
}


function onCancel () {
  return true;
}


function onAccept(){
  try {
    document.getElementById("prefGeneral").writePreferences(true);
    document.getElementById("prefEdit").writePreferences(true);
    writeComputePreferences();
    document.getElementById("prefPlots").writePreferences(true);
    getTypesetFilePrefs();
    document.getElementById("prefTypesetting").writePreferences(true);
  }
  catch(e) {
    myDump(e.toString());
  }

}

function writeComputePreferences()
{
  document.getElementById("prefComp").writePreferences(true);
  // the compute preferences have to be sent to the engine as well as saved
  var prefId;
  var pref;
  for (prefId in prefMapper)
  {
    pref = document.getElementById(prefId);
    onComputeSettingChange(pref, true);    
  }  
  for (prefId in prefEngineMapper)
  {
    pref = document.getElementById(prefId);
    onComputeSettingChange(pref, true);    
  }  
  for (prefId in prefLogMapper)
  {
    pref = document.getElementById(prefId);
    onComputeSettingChange(pref, true);    
  }   
}

var prefMapper;
var prefEngineMapper;
var prefLogMapper;

function initializePrefMappersIfNeeded()
{ // map between pref names used by the engine and those used in SWP
  if (prefMapper == null)
  {  
    prefMapper = new Object();
    prefMapper.mfencedpref = "use_mfenced";
    prefMapper.digitsRenderedpref = "Sig_digits_rendered";
    prefMapper.lowerthresholdpref = "SciNote_lower_thresh";
    prefMapper.upperthresholdpref = "SciNote_upper_thresh";
    prefMapper.trigargspref = "Parens_on_trigargs";
    prefMapper.imagipref = "Output_imaginaryi";
    prefMapper.diffDpref = "Output_diffD_uppercase";
    prefMapper.diffdpref = "Output_diffd_lowercase";
    prefMapper.expepref = "Output_Euler_e";
    prefMapper.matrix_delimpref = "Default_matrix_delims";
    prefMapper.usearcpref    = "Output_InvTrigFuncs_1";
    prefMapper.mixednumpref = "Output_Mixed_Numbers";
    prefMapper.derivformatpref = "Default_derivative_format";
    prefMapper.primesasnpref = "Primes_as_n_thresh";
    prefMapper.primederivpref = "Prime_means_derivative";

    prefMapper.logepref = "log_is_base_e";
    prefMapper.dotderivativepref = "Dot_derivative";
    prefMapper.barconjpref = "Overbar_conjugate";
    prefMapper.i_imaginarypref = "Input_i_Imaginary";
    prefMapper.j_imaginarypref = "Input_j_Imaginary";
    prefMapper.e_exppref = "Input_e_Euler";
  }
  if (prefEngineMapper == null)
  {  
    prefEngineMapper = new Object();
    prefEngineMapper.digitsusedpref  = "Digits";
    prefEngineMapper.degreepref      = "MaxDegree";
    prefEngineMapper.principalpref   = "PvalOnly"
    prefEngineMapper.specialpref     = "IgnoreSCases";
  }
  if (prefLogMapper == null)
  {
    prefLogMapper = new Object();
    prefLogMapper.logSentpref     = "LogMMLSent";
    prefLogMapper.logReceivedpref = "LogMMLReceived";
    prefLogMapper.engSentpref     = "LogEngSent";
    prefLogMapper.engReceivedpref = "LogEngReceived";
  } 
}

function onComputeSettingChange(pref, force)
{
  if (force || pref.instantApply)
  try
  {
    var currEngine = GetCurrentEngine();
    var engineName;
    var prefId = pref.id;
    var val = pref.value;
    initializePrefMappersIfNeeded();
    try
    {  
      var mappedPref;
      if (mappedPref = prefMapper[prefId])
      {
        myDump("Setting user pref " + mappedPref + " to " + val +"\n");
        currEngine.setUserPref(currEngine[mappedPref], val); 
      }
      else if (mappedPref = prefEngineMapper[prefId])
      {
        myDump("Setting engine attribute " + mappedPref + " to " + val + "\n");
        currEngine.setEngineAttr(currEngine[mappedPref], val);      
      }
      else if (mappedPref = prefLogMapper[prefId])
      {
        myDump("Setting logger pref " + mappedPref + " to " + val + "\n");
        msiComputeLogger[mappedPref](val);
      }
      else
      {
        myDump("Invalid prefId: "+prefId+"\n");
      }
    }
    catch(e)
    {
      myDump(e.message+", prefId = "+prefId+"\n");
    }
  } 
  catch(e)
  {
    myDump(e.message+"\n");
  } 
}

function onBrowseBibTeXExecutable()
{
  var filePicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  filePicker.init(window, msiGetDialogString("filePicker.selectBibStyleDir"), Components.interfaces.nsIFilePicker.modeOpen);
  filePicker.appendFilters(Components.interfaces.nsIFilePicker.filterApps);
  var thePref = document.getElementById("bibTeXExecutable");
  setFilePrefFromTextbox(thePref);
  if (thePref.value && thePref.value.path && thePref.value.path.length)
  {
    filePicker.defaultString = thePref.value.leafName;
    filePicker.displayDirectory = thePref.value.parent;
  }
  var res = filePicker.show();
  if (res == Components.interfaces.nsIFilePicker.returnOK)
  {
    thePref.value = filePicker.file;
//    thePref.updateElements();
//    document.getElementById("bibTeXExecutableTextbox").value = gDialog.bibTeXExe.path;
  }
}

function onBrowseBibTeXDatabaseDir()
{
  var dirPicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  dirPicker.init(window, msiGetDialogString("filePicker.selectBibDBDir"), Components.interfaces.nsIFilePicker.modeGetFolder);
  var thePref = document.getElementById("bibTeXDatabaseDir");
  setFilePrefFromTextbox(thePref);
  if (thePref.value && thePref.value.path && thePref.value.path.length)
    dirPicker.displayDirectory = thePref.value;
  var res = dirPicker.show();
  if (res == Components.interfaces.nsIFilePicker.returnOK)
  {
    thePref.value = dirPicker.file;
//    thePref.updateElements();
//    document.getElementById("bibTeXDatabaseDirTextbox").value = gDialog.bibTeXDBDir.path;
  }
}

function onBrowseBibTeXStyleDir()
{
  var dirPicker = Components.classes["@mozilla.org/filepicker;1"].createInstance(Components.interfaces.nsIFilePicker);
  dirPicker.init(window, msiGetDialogString("filePicker.selectBibDBDir"), Components.interfaces.nsIFilePicker.modeGetFolder);
  var thePref = document.getElementById("bibTeXStyleDir");
  setFilePrefFromTextbox(thePref);
  if (thePref.value && thePref.value.path && thePref.value.path.length)
    dirPicker.displayDirectory = thePref.value;
  var res = dirPicker.show();
  if (res == Components.interfaces.nsIFilePicker.returnOK)
  {
    thePref.value = dirPicker.file;
//    thePref.updateElements();
//    document.getElementById("bibTeXDatabaseDirTextbox").value = gDialog.bibTeXDBDir.path;
  }
}

function setTextboxValue(aPref)
{
  var theTextbox;
  switch(aPref.id)
  {
    case "bibTeXExecutable":
      theTextbox = document.getElementById("bibTeXExecutableTextbox");
    break;
    case "bibTeXDatabaseDir":
      theTextbox = document.getElementById("bibTeXDatabaseDirTextbox");
    break;
    case "bibTeXStyleDir":
      theTextbox = document.getElementById("bibTeXStyleDirTextbox");
    break;
    default:
    break;
  }
  if (theTextbox)
    theTextbox.value = (aPref.value && ("path" in aPref.value)) ? aPref.value.path : "";
}

function setFilePrefFromTextbox(aPref)
{
  var theTextbox;
  switch(aPref.id)
  {
    case "bibTeXExecutable":
      theTextbox = document.getElementById("bibTeXExecutableTextbox");
    break;
    case "bibTeXDatabaseDir":
      theTextbox = document.getElementById("bibTeXDatabaseDirTextbox");
    break;
    case "bibTeXStyleDir":
      theTextbox = document.getElementById("bibTeXStyleDirTextbox");
    break;
    default:
    break;
  }
  if (theTextbox && theTextbox.value)
  {
    try { thePref.value.initWithPath(theTextbox.value); }
    catch(exc) {dump("Error in preferences.js setFilePrefFromTextbox for pref [" + aPref.id + "]: [" + exc + "].\n");} 
  }
}

function setTypesetFilePrefTestboxes()
{
  setTextboxValue(document.getElementById("bibTeXExecutable"));
  setTextboxValue(document.getElementById("bibTeXDatabaseDir"));
  setTextboxValue(document.getElementById("bibTeXStyleDir"));
}

function getTypesetFilePrefs()
{
  setFilePrefFromTextbox(document.getElementById("bibTeXExecutable"));
  setFilePrefFromTextbox(document.getElementById("bibTeXDatabaseDir"));
  setFilePrefFromTextbox(document.getElementById("bibTeXStyleDir"));
}
