"use strict";
Components.utils.import("resource://app/modules/computelogger.jsm");
Components.utils.import("resource://app/modules/unitHandler.jsm");


var placementIdsGraphics = {prefID : "defaultGraphicsPlacement", placementRadio : "placementRadioGroup",
                            hereRadioGroup : "herePlacementRadioGroup", placeForceHereCheckbox : "placeForceHereCheck",
                            placeHereCheckbox : "placeHereCheck", placeFloatsCheckbox : "placeFloatsCheck",
                            placeTopCheckbox : "placeTopCheck", placeBottomCheckbox : "placeBottomCheck"};
var placementIdsPlot = {prefID : "graph.placement", placementRadio : "plotPlacementRadioGroup",
                        hereRadioGroup : "plotHerePlacementRadioGroup", placeForceHereCheckbox : "plotPlaceForceHereCheck",
                        placeHereCheckbox : "plotPlaceHereCheck", placeFloatsCheckbox : "plotPlaceFloatsCheck",
                        placeTopCheckbox : "plotPlaceTopCheck", placeBottomCheckbox : "plotPlaceBottomCheck"};
var currPlotType;

//function myDump(aMessage) {
//  var consoleService = Components.classes["@mozilla.org/consoleservice;1"]
//                                 .getService(Components.interfaces.nsIConsoleService);
//  consoleService.logStringMessage(aMessage);
//}

var chromedoc;

function initialize()
{
  var url;
//  chromedoc = window.arguments[0]["chromeDoc"];
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  var dir =dsprops.get("resource:app", Components.interfaces.nsIFile);
  dir.append("shells");
  url = msiFileURLFromFile(dir);
  var tree = document.getElementById("dir-tree");
  tree.setAttribute("ref", url.spec);
  tree.currentIndex = 0;
  showShellsInDir(tree);
  setGraphicLayoutPreferences("graphics");
  setGraphicLayoutPreferences("plot");
  initPlotItemPreferences();
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
    dump(e.toString());
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
    writeGraphicLayoutPreferences("graphics");
    writeGraphicLayoutPreferences("plot");
    document.getElementById("prefGeneral").writePreferences(true);
    document.getElementById("prefEdit").writePreferences(true);
    writeComputePreferences();
    document.getElementById("prefPlots").writePreferences(true);
    getTypesetFilePrefs();
    document.getElementById("prefTypesetting").writePreferences(true);
  }
  catch(e) {
    dump(e.toString());
  }
}

function skinChanged()
{
  var chromedoc = window.arguments[0]["chromeDoc"];
//  chromedoc.getElementById("EditorToolbox").customizeDone(true);
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
  storePlotItemPreferences();
}

function setGraphicLayoutPreferences(whichPrefs)
{
  var pref;
  var color;
  switch(whichPrefs)
  {
    case "graphics":
      setGraphicPlacementPreferences(placementIdsGraphics);
      pref = document.getElementById("defaultGraphicsSizeUnits");
      initUnitsControl( document.getElementById("graphicsUnitsList"), pref,
                          [document.getElementById("graphicsWidth"), document.getElementById("graphicsHeight"),
                           document.getElementById("graphicsInlineOffset")] );
      initColorWell("graphics.padding.CW");
      initColorWell("graphics.border.CW");
    break;
    case "plot":
//      setGraphicPlacementPreferences(placementIdsPlot);
      pref = document.getElementById("graph.defaultUnits");
      initUnitsControl( document.getElementById("plotUnitsList"), pref,
                          [document.getElementById("plotWidth"), document.getElementById("plotHeight")]);
      initColorWell("graph.padding.CW");
      initColorWell("graph.border.CW");
      initColorWell("plotCW");
    break;
  }
}

function initColorWell(id)
{
  try
  {
    var colorwell = document.getElementById(id);
    var pref = document.getElementById(colorwell.getAttribute("preference"));
    if (!pref) return;
    var color = pref.valueFromPreferences;
    setColorWell(id, color);
    colorwell.setAttribute("color",color);
  }
  catch(e)
  {
    var m = e.message;
  }
}

function writeColorWell(id)
{
  try
  {
    var colorwell = document.getElementById(id);
    var pref = document.getElementById(colorwell.getAttribute("preference"));
    if (!pref) return;
    pref.valueFromPreferences = colorwell.getAttribute("color");
  }
  catch(e)
  {
    var m = e.message;
  }
  
}


function writeGraphicLayoutPreferences(whichPrefs)
{
  var pref, elem;
  switch(whichPrefs)
  {
    case "graphics":
      writeGraphicPlacementPreferences(placementIdsGraphics);
      //The following are necessary since when changing the units, the unit handler changes the values, but
      //this doesn't fire the needed event.
      pref = document.getElementById("defaultGraphicsHSize");
      pref.value = document.getElementById("graphicsWidth").value;
      pref = document.getElementById("defaultGraphicsVSize");
      pref.value = document.getElementById("graphicsHeight").value;
      pref = document.getElementById("defaultGraphicsSizeUnits");
      pref.value = document.getElementById("graphicsUnitsList").value;
      pref = document.getElementById("defaultGraphicsInlineOffset");
      pref.value = document.getElementById("graphicsInlineOffset").value;
    break;
    case "plot":
      writeGraphicPlacementPreferences(placementIdsPlot);
      //The following are necessary since when changing the units, the unit handler changes the values, but
      //this doesn't fire the needed event.
      elem = document.getElementById("plotWidth");
      pref = document.getElementById(elem.getAttribute("preference"));
      pref.value = elem.value;
      elem = document.getElementById("plotHeight");
      pref = document.getElementById(elem.getAttribute("preference"));
      pref.value = elem.value;
//      pref = document.getElementById("defaultGraphSizeUnits");
//      pref.value = document.getElementById("plotUnitsList").value;
      writeColorWell("graph.padding.CW");
      writeColorWell("graph.border.CW");
      writeColorWell("plotCW");
    break;
  }
}

function setGraphicPlacementPreferences(whichIDs)
{
  if (!whichIDs)
    whichIDs = placementIdsGraphics;
  var pref = document.getElementById(whichIDs.prefID);
  var pos = "inline";
  var placeLocation = "";
  var placement = "";
  if (pref.value && pref.value.length)
  {
    var defPlacementArray = pref.value.split(",");
    if (defPlacementArray.length)
    {
      pos = TrimString(defPlacementArray[0]);
      if (defPlacementArray.length > 1)
      {
        placeLocation = TrimString(defPlacementArray[1]);
        if (defPlacementArray.length > 2)
          placement = TrimString(defPlacementArray[2]);
      }
    }
  }

  if (pos.length)
    document.getElementById(whichIDs.placementRadio).value = pos;
  if (placement.length)
    document.getElementById(whichIDs.hereRadioGroup).value = placement;
  for (var ii = 0; ii < placeLocation.length; ++ii)
  {
    switch(placeLocation[ii])
    {
      case "H":      document.getElementById(whichIDs.placeForceHereCheckbox).checked = true;  break;
      case "h":      document.getElementById(whichIDs.placeHereCheckbox).checked = true;       break;
      case "p":      document.getElementById(whichIDs.placeFloatsCheckbox).checked = true;     break;
      case "t":      document.getElementById(whichIDs.placeTopCheckbox).checked = true;        break;
      case "b":      document.getElementById(whichIDs.placeBottomCheckbox).checked = true;     break;
      default:                                                                       break;
    }
  }
}

function writeGraphicPlacementPreferences(whichIDs)
{
  if (!whichIDs)
    whichIDs = placementIdsGraphics;
  var pos = document.getElementById(whichIDs.placementRadio).value;
  var placeLocation = "";
  if (document.getElementById(whichIDs.placeForceHereCheckbox).checked)   placeLocation += "H";
  if (document.getElementById(whichIDs.placeHereCheckbox).checked)        placeLocation += "h";
  if (document.getElementById(whichIDs.placeFloatsCheckbox).checked)      placeLocation += "p";
  if (document.getElementById(whichIDs.placeTopCheckbox).checked)         placeLocation += "t";
  if (document.getElementById(whichIDs.placeBottomCheckbox).checked)      placeLocation += "b";
  var placement = document.getElementById(whichIDs.hereRadioGroup).value;

  var pref = document.getElementById(whichIDs.prefID);
  pref.value = pos + "," + placeLocation + "," + placement;
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
        dump("Setting user pref " + mappedPref + " to " + val +"\n");
        if (mappedPref === "Default_matrix_delims") {
          if (val === "matrix_brackets") val = 1;
          else if (val === "matrix_parens") val = 2;
          else if (val === "matrix_braces") val = 3;
          else (val = 0); 
        }
        if (mappedPref === "Output_imaginaryi") {
          if (val === "imagi_i") val = 0;
          else if (val === "imagi_j") val = 1;
          else (val = 2); 
        }
        currEngine.setUserPref(currEngine[mappedPref], val); 
      }
      else if (mappedPref = prefEngineMapper[prefId])
      {
        dump("Setting engine attribute " + mappedPref + " to " + val + "\n");
        currEngine.setEngineAttr(currEngine[mappedPref], val);      
      }
      else if (mappedPref = prefLogMapper[prefId])
      {
        dump("Setting logger pref " + mappedPref + " to " + val + "\n");
        msiComputeLogger[mappedPref](val);
      }
      else
      {
        dump("Invalid prefId: "+prefId+"\n");
      }
    }
    catch(e)
    {
      dump(e.message+", prefId = "+prefId+"\n");
    }
  } 
  catch(e)
  {
    dump(e.message+"\n");
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
    try { aPref.value.initWithPath(theTextbox.value); }
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

function initUnitsControl(unitBox, pref, controlArray)
{
  unitBox.unitsHandler = new UnitHandler();
  unitBox.unitsHandler.setEditFieldList(controlArray);

  var currUnit = "pt";
  if (pref.value && pref.value.length)
    currUnit = pref.value;
  else if (unitBox.value)
    currUnit = unitBox.value;
  unitBox.unitsHandler.initCurrentUnit(currUnit);
}

function onChangeUnits(unitBox)
{
  if (unitBox.unitsHandler)
    unitBox.unitsHandler.setCurrentUnit(unitBox.value);
}


function getColorAndUpdate(id)
{
  var colorWell = document.getElementById(id);
  if (!colorWell) return;
  var color;

  var colorObj = { NoDefault: false, Type: "Rule", TextColor: color, PageColor: 0, Cancel: false };
  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);
  if (colorObj.Cancel)
    return;
  color = colorObj.TextColor;
  setColorWell(id,color);
  colorWell.setAttribute("color",color);
  // next line for immediate writing only 
  writeColorWell(id);
}

//***************************Plot Item tab**************************//
var plotItemIds = ["plotLineColorWell", "plotDirectionalShading", "plotBaseColorWell", "plotSecondColorWell",
                   "plotLineStyle", "plotLineThickness", "plotPointMarker", "plotFillPattern",
                   "plotSurfaceStyle", "plotSurfaceMesh", "plotAISubIntervals", "plotAIMethod",
                   "plotAIInfo", "plotPtssampTubeRadius", "plotPtssampConfHorizontal",
                   "plotPtssampConfVertical"];  //(All other than the edit ones, which require special treatment)

var plotColorWells = ["plotLineColorWell", "plotBaseColorWell", "plotSecondColorWell"];

var plotVarEditControls = ["plotVar1StartEdit", "plotVar1EndEdit", "plotVar2StartEdit", "plotVar2EndEdit",
                    "plotVar3StartEdit", "plotVar3EndEdit", "plotVar4StartEdit", "plotVar4EndEdit",
                    "xrangelow", "xrangehigh", "yrangelow", "yrangehigh", "zrangelow", "zrangehigh"];

var plotVarControls = ["plotPtssamp1", "plotPtssamp2", "plotPtssamp3", "plotPtssamp4"];

var plotVarEditsReady = [];

function initPlotItemPreferences()
{
  addNonRootPlotPrefs();
  currPlotType = document.getElementById("plotTypeList").value;
  initPlotItemEditors();
  setPlotItemBroadcasters();
  setPlotItemPreferences();
}

var bPlotEditsReady = false;

var minMaxDocumentObserverBase = {
  observe: function (aSubject, aTopic, aData)
  {
    if (aTopic === "obs_documentCreated")
    {
      plotVarEditsReady.push(this.ctrlID);
      bPlotEditsReady = checkPlotVarEditsReady();
    }
  }
};

function minMaxDocumentObserver(ctrl)
{
  this.ctrlID = ctrl.id;
}

minMaxDocumentObserver.prototype = minMaxDocumentObserverBase;

function checkPlotVarEditsReady()
{
  var isReady = true;
  for (var anEditorId in plotVarEditControls)
  {
    if (plotVarEditsReady.indexOf(anEditorId) < 0)
    {
      isReady = false;
      break;
    }
  }
  if (!isReady)
    msidump("Edits not yet ready in IntervalsAndAnimations dialog!\n");
  return isReady;
}

function initPlotItemEditors()
{
  //Initialize editor controls
  var fallbackVals = [-6, 6, -6, 6, -6, 6, 0, 10];
  var editorElement, prefElement, theStringSource, key;
  var editorInitializer = new msiEditorArrayInitializer();
  for (var ii = 0; ii < plotVarEditControls.length; ++ii)
  {
    theStringSource = "";
    editorElement = document.getElementById(plotVarEditControls[ii]);
    try
    {
      prefElement = document.getElementById(editorElement.getAttribute("preference"));
      theStringSource = prefElement.value;
    } catch(ex) { 
      dump("Exception trying to initialize editor " + plotVarEditControls[ii] + " in initPlotItemPreferences().\n");
    }
    if (!theStringSource.length)
    {
      key = getBasePlotPrefKeyName(prefElement.getAttribute(name));
      theStringSource = getPlotDefaultValue(null, null, key);
      if (!theStringSource.length)
        theStringSource = GetNumAsMathML(fallbackVals[ii]);
    }
    editorElement.mInitialDocObserver = [{mCommand : "obs_documentCreated", mObserver : minMaxDocumentObserver(editorElement)}];
    editorInitializer.addEditorInfo(editorElement, theStringSource, true);
  }
  editorInitializer.doInitialize();
}

function addNonRootPlotPrefs()
{
  var refPref, prefId;
  var prefService = Components.classes["@mozilla.org/preferences-service;1"]
                              .getService(Components.interfaces.nsIPrefService);
  var plotBranch = prefService.getBranch("swp.plot.");
  var childCount = {value : 0};
  var children = plotBranch.getChildList("", childCount);
  for (var jj = 0; jj < children.length; ++jj)
  {
    prefId = children[jj];
    prefId = prefId.substr( prefId.lastIndexOf(".") + 1);
    prefId = "plot." + prefId;
    refPref = document.getElementById(prefId);
    if ( refPref && (prefId != ("plot." + children[jj])) )
      prefElement = insertNewPrefElement(refPref, "swp.plot." + children[jj]);
  }
}

function setPlotItemBroadcasters()
{
  var plotTypeObj = getPlotDimAndPlottype(currPlotType);
  var numvars, dim, ptype;
  if (currPlotType == "any")
  {
    dim = 3;
    numvars = 4;
    ptype = "any";
  }
  else
  {
    numvars = plotVarsNeeded(plotTypeObj.dim, plotTypeObj.plotType, true);
    dim = currPlotType.dim;
    ptype = currPlotType.plotType;
  }
  document.getElementById("plot.1Var").collapsed = (numvars < 1);
  document.getElementById("plot.2Vars").collapsed = (numvars < 2);
  document.getElementById("plot.3Vars").collapsed = (numvars < 3);
  document.getElementById("plot.4Vars").collapsed = (numvars < 4);

  var showFillColors = 0;
  var showLineColors = 1;
  var bUseMesh = false;
  var bUseDirShading = false;
  if (dim == 3)
  {
    document.getElementById("plot.threeDim").collapsed = false;
    document.getElementById("plot.colorAlphaEnabled").setAttribute("hasAlpha", "true");
    bUseDirShading = true;
    bUseMesh = true;
    showFillColors = 2;
    showLineColors = 0;
    switch(ptype)
    {
      case "curve":
        showLineColors = 1;
        showFillColors = 0;
        bUseMesh = false;
      break;
      case "vectorField":
      case "gradient":
        bUseMesh = false;
      break;
      case "any":
        showLineColors = 1;
      break;
    }
  } else {
    document.getElementById("plot.threeDim").collapsed = true;
    document.getElementById("plot.colorAlphaEnabled").setAttribute("hasAlpha", "false");
    switch(plotTypeObj.plotType)
    {
      case "inequality":            showFillColors = 2;   break;
      case "conformal":             showLineColors = 2;   break;
      case "approximateIntegral":
        switch(aiMethod)
        {
          case "LeftRight":
          case "LowerUpper":
          case "LowerUpperAbs":
            showFillColors = 2;
          break;
          default:
            showFillColors = 1;
          break;
        }
      break;
    }
  }
//  document.getElementById("tubeOrLinePts").setAttribute( "selectedIndex", ((currPlotType ==="tube") ? 1 : 0) );
  document.getElementById("plot.enableLinesAndPoints").collapsed = (showLineColors < 1);
  document.getElementById("plot.use1LineColor").collapsed = (showLineColors < 1);
  document.getElementById("plot.use2LineColors").collapsed = (showLineColors < 2);
  document.getElementById("plot.use1FillColor").collapsed = (showFillColors < 1);
  document.getElementById("plot.use2FillColors").collapsed = (showFillColors < 2);
  document.getElementById("plot.useDirectionalShading").collapsed = !bUseDirShading;
  document.getElementById("plot.useBaseColor").collapsed = (showFillColors < 1) && (showLineColors < 2);
  document.getElementById("plot.useMesh").collapsed = !bUseMesh;
  document.getElementById("plot.approxIntPlot").collapsed = (plotTypeObj.plotType !== "approximateIntegral");
  document.getElementById("plot.useAreaFill").collapsed = ((plotTypeObj.plotType != "approximateIntegral") && (plotTypeObj.plotType !== "inequality"));
  document.getElementById("plot.enableConformal").collapsed = (plotTypeObj.plotType != "conformal");
  document.getElementById("plot.useDiscAdjust").collapsed = ((plotTypeObj.plotType != "rectangular") &&
                                (plotTypeObj.plotType != "parametric") && (plotTypeObj.plotType != "implicit"));
}                                

function onChangePlotType()
{
  storePlotItemPreferences();
  currPlotType = document.getElementById("plotTypeList").value;
  setPlotItemBroadcasters();
  setPlotItemPreferences();
}

//<spacer id= hasAlpha="false" preference="plot.LineColor" observes="plot.colorAlphaEnabled" class="color-well"/>
//<menulist id= preference="plot.DirectionalShading" observes="plot.useDirectionalShading">
//<spacer id= preference="plot.BaseColor" hasAlpha="false" observes="plot.colorAlphaEnabled" class="color-well"/>
//<spacer id= preference="plot.SecondaryColor" hasAlpha="false" observes="plot.colorAlphaEnabled" class="color-well" />
//<menulist id= preference="plot.LineStyle">
//<menulist id= preference="plot.LineThickness">
//<menulist id= preference="plot.PointSymbol">
//<menulist id= preference="plot.FillPattern">
//<menulist id= preference="plot.SurfaceStyle">
//<menulist id= preference="plot.SurfaceMesh">
//<textbox id= type="number" class="narrow" preference="plot.AISubIntervals" increment="1" decimalplaces="0" min="1" max="999" />
//<menulist id= preference="plot.AIMethod" style="min-width:80pts">
//<menulist id= preference="plot.AIInfo" style="min-width:80pts">
//  var editControls = [];
//<textbox id= type="number" preference="plot.var1Pts" class="narrow" />
//<textbox id= type="number" preference="plot.var2Pts" class="narrow" />
//<textbox id= type="number" preference="plot.var3Pts" class="narrow" />
//<textbox id= type="number" preference="plot.var4Pts" class="narrow" />
//<textbox id= preference="plot.TubeRadialPoints" type="number" class="narrow" />

//Following called for initialization, or in response to change in plot type menulist.
function setPlotItemPreferences()
{
  var element, refPref, prefElement, prefId, prefName;
  var plotTypeObj = getPlotDimAndPlottype(currPlotType);
  var prefix = getPlotPrefPrefix(currPlotType);
  for (var ii = 0; ii < plotItemIds.length; ++ii)
  {
    element = document.getElementById(plotItemIds[ii]);
    if (elementDisabledOrHidden(element))
      continue;
    prefId = element.getAttribute("preference");
    prefId = prefix + prefId.substr( prefId.lastIndexOf(".") + 1 );
    element.setAttribute("preference", prefId);
    prefElement = document.getElementById(prefId);
    if (!prefElement)
    {
      refPref = getReferencePlotItemPreference(prefId);
      if (refPref)
        prefElement = insertNewPrefElement(refPref, prefId);
    }
//    putValueToControl(element, theVal);
  }
  for (var jj = 0; jj < plotColorWells.length; ++jj)
    setPlotColorWell(plotColorWells[jj]);
  try { setPlotItemIntervalControls(); }
  catch(ex) {}
}

function setPlotItemIntervalControls()
{
  var varNames = ["X", "Y", "Z"];
  var numvars, prefIdBase, startElement, endElement, ptsElement, refPref;
  var startPrefId, endPrefId, ptsPrefId, prefElement;
  var plotTypeObj = getPlotDimAndPlottype(currPlotType);
  var prefix = getPlotPrefPrefix(currPlotType);
  if (currPlotType == "any")
    numvars = 4;
  else
    numvars = plotVarsNeeded(plotTypeObj.dim, plotTypeObj.plotType, true);
  for (var jj = 1; jj <= numvars; ++jj)
  {
    startElement = document.getElementById("plotVar" + jj + "StartEdit");          //plotVar1StartEdit
    endElement = document.getElementById("plotVar" + jj + "EndEdit");              //plotVar1EndEdit
    ptsElement = document.getElementById("plotPtssamp" + jj);
    if ( !elementDisabledOrHidden(startElement) && !elementDisabledOrHidden(endElement))
    {
      if (jj == numvars)
      {
        prefIdBase = prefix + "Anim";
        document.getElementById("plotVar" + jj).textContent = getPlotIntervalVarName(plotTypeObj.plotType, "Anim");
      }
      else
      {
        prefIdBase = prefix + varNames[jj-1];
        document.getElementById("plotVar" + jj).textContent = getPlotIntervalVarName(plotTypeObj.plotType, varNames[jj-1]);
      }
      
      startPrefId = prefIdBase + "Min";
      endPrefId = prefIdBase + "Max";
      ptsPrefId = prefIdBase + "Pts";
      startElement.setAttribute("preference", startPrefId);
      prefElement = document.getElementById(startPrefId);
      if (!prefElement)
      {
        refPref = getReferencePlotItemPreference(startPrefId);
        if (refPref)
          prefElement = insertNewPrefElement(refPref, startPrefId);
      }
      putMathMLExpressionToControl(startElement, prefElement.value);

      endElement.setAttribute("preference", endPrefId);
      prefElement = document.getElementById(endPrefId);
      if (!prefElement)
      {
        refPref = getReferencePlotItemPreference(endPrefId);
        if (refPref)
          prefElement = insertNewPrefElement(refPref, endPrefId);
      }
      putMathMLExpressionToControl(endElement, prefElement.value);

      if (ptsElement)
      {
        ptsElement.setAttribute("preference", ptsPrefId);
        prefElement = document.getElementById(ptsPrefId);
        if (!prefElement)
        {
          refPref = getReferencePlotItemPreference(ptsPrefId);
          if (refPref)
            prefElement = insertNewPrefElement(refPref, ptsPrefId);
        }
      }
    }
  }
}

function storePlotItemPreferences()
{
  var ii, element, prefStr, refPref, prefElement, value, badVal;
  var prefix = getPlotPrefPrefix(currPlotType);
  var serializer = new XMLSerializer();
  for (ii = 0; ii < plotVarEditControls.length; ++ii)
  {
    element = document.getElementById(plotVarEditControls[ii]);
    if (elementDisabledOrHidden(element))
      continue;
    prefElement = document.getElementById(element.getAttribute("preference"));
    if (prefElement)
    {
      if (!isEmptyMathEditControl(element))
        prefElement.value = getMathMLExpressionFromControl(element, serializer);
    }
  }
  for (var jj = 0; jj < plotColorWells.length; ++jj)
    storePlotColorWell(plotColorWells[jj]);
  for (ii = 0; ii < plotItemIds.length; ++ii)
  {
    badVal = false;
    element = document.getElementById(plotItemIds[ii]);
    if (elementDisabledOrHidden(element))
      continue;
    prefStr = element.getAttribute("preference");
    prefElement = document.getElementById(prefStr);
    refPref = getReferencePlotItemPreference(prefStr);
    value = getValueFromControl(element);
    badVal = isBadPlotPrefValue(refPref, value);
    if ( (prefElement.getAttribute("msi-temp") == "true") && (refPref != prefElement) 
             && (badVal || (prefElement.value == refPref.value)) )
      prefElement.parentNode.removeChild(prefElement);
    else if (badVal)
      prefElement.value = prefElement.defaultValue;
  }
}

function isBadPlotPrefValue(prefElem, value)
{
  if (!value.length)
  {
    //Can have a switch on prefElem.name if some prefs can have a legal empty value, but I don't know of one...
    return true;
  }
  return false;
}

function getPlotDimAndPlottype(plotTypeDescription)
{
  switch(plotTypeDescription)
  {
    case "2d-rectangular":              return {dim : 2, plotType : "rectangular"};
    case "3d-rectangular":              return {dim : 3, plotType : "rectangular"};
    case "2d-explicitList":             return {dim : 2, plotType : "explicitList"};
    case "3d-explicitList":             return {dim : 3, plotType : "explicitList"};
    case "polar":                       return {dim : 2, plotType : "polar"};
    case "2d-implicit":                 return {dim : 2, plotType : "implicit"};
    case "3d-implicit":                 return {dim : 3, plotType : "implicit"};
    case "inequality":                  return {dim : 2, plotType : "inequality"};
    case "2d-parametric":               return {dim : 2, plotType : "parametric"};
    case "3d-parametric":               return {dim : 3, plotType : "parametric"};
    case "conformal":                   return {dim : 2, plotType : "conformal"};
    case "2d-gradient":                 return {dim : 2, plotType : "gradient"};
    case "3d-gradient":                 return {dim : 3, plotType : "gradient"};
    case "2d-vectorField":              return {dim : 2, plotType : "vectorField"};
    case "3d-vectorField":              return {dim : 3, plotType : "vectorField"};
    case "ode":                         return {dim : 2, plotType : "ode"};
    case "approximateIntegral":         return {dim : 2, plotType : "approximateIntegral"};
    case "curve":                       return {dim : 3, plotType : "curve"};
    case "cylindrical":                 return {dim : 3, plotType : "cylindrical"};
    case "spherical":                   return {dim : 3, plotType : "spherical"};
    case "tube":                        return {dim : 3, plotType : "tube"};
    case "any":                         ;
    default:                            return {dim : null, plotType : null};
  }
}

function getPlotPrefPrefix(plotTypeDescription)
{
  var prefix = "plot.";
  var plotTypeObj = getPlotDimAndPlottype(plotTypeDescription);
  if (plotTypeObj.dim)
  {
    prefix += plotTypeObj.dim + "d.";
    if (plotTypeObj.plotType)
      prefix += plotTypeObj.plotType + ".";
  }
  return prefix;
}

function getReferencePlotItemPreference(prefStr)
{
  var prefixStr = prefStr.substr(0, prefStr.lastIndexOf("."));
  var attribStr = prefStr.substr( prefStr.lastIndexOf(".") ); //but including the last "."
  var refElement;
  do {
    prefixStr = prefixStr.substr( 0, prefixStr.lastIndexOf("."));
    refElement = document.getElementById(prefixStr + attribStr);
  } while (!refElement && (prefixStr.length > 4));  //"4" is the length of "plot", the beginning sequence for all these
  return refElement;
}

//This should be called with something like "plot.2d.explicitPlot.LineColor" as prefId and the existing <preference>
//  with id "plot.LineColor" as refPref.
function insertNewPrefElement(refPref, prefId)
{
  var prefElement, val, prefsElement;
  try
  {
    prefsElement = document.getElementById("plotPrefs");
    prefElement = document.createElementNS(XUL_NS, "preference");
    var prefix = prefId.substr(0, prefId.lastIndexOf(".") + 1);
    prefElement.setAttribute("id", prefId);
    prefElement.setAttribute("msi-temp", "true");  //This tags the preference to be deleted if it doesn't get set by the user.
    prefsElement.insertBefore(prefElement, refPref.nextSibling);
    var prefName = refPref.name.replace("plot.", prefix);
    prefElement.name = prefName;
    prefElement.type = refPref.type;
    val = prefElement.valueFromPreferences;
    if (val)
      prefElement.value = val;
    else
      prefElement.value = refPref.value;
  } catch(ex) {dump("In preferences.js, insertNewprefElement(), exception: " + ex + "\n"); prefElement = null;}
  return prefElement;
}

function getPlotIntervalVarName(plotType, baseVarName)
{
  var compBundle = document.getElementById("computeBundle");
  var rv = "";
  var prefixStr;
  if (!plotType)
    prefixStr = "";
  else
  {
    switch(plotType)
    {
      case "curve":   prefixStr = "parametric";      break;
      default:        prefixStr = plotType;          break;
    }
  }

  var initialStr = "Intervals.";
  try
  {
    rv = compBundle.getString(initialStr + prefixStr + baseVarName + "Var");
  }
  catch(exc)
  {
    try
    {
      rv = compBundle.getString(initialStr + baseVarName + "Var");
    }
    catch(ex)
    {
      msidump("Problem in preferences.js, getPlotIntervalVarName; unable to get string for " + initialStr + prefixStr + baseVarName + "Var.\n");
      rv = "";
    }
  }
  return rv;
}

function setPlotColorWell(colorId)
{
  var colorWell = document.getElementById(colorId);
  var pref = document.getElementById(colorWell.getAttribute("preference"));
  if (pref)
    putValueToControl(colorWell, pref.value);
}

function storePlotColorWell(colorId)
{
  var theVal;
  var colorWell = document.getElementById(colorId);
  var pref = document.getElementById(colorWell.getAttribute("preference"));
  if (pref)
  {
    theVal = getValueFromControl(colorWell);
    if (theVal.length)
      pref.value = theVal;
  }
}

function elementDisabledOrHidden(anElement)
{
  var node = anElement;
  var rv = false;
  while (node && !rv && (node.id != "PlotItems"))
  {
    rv = (node.disabled || node.hidden || node.collapsed);
    node = node.parentNode;
  }
  return rv;
}
//function writeAPlotPref(dim, plottype, prefID, value)
//{
////What about "ConfHorizontalPts", "ConfVerticalPts"?
//  var thePref = document.getElementById(prefID);
//  var key = thePref.getAttribute("name");
//  var basekey = key.substr( key.lastIndexOf(".") );
//  var oldval = getPlotDefaultValue(dim, plotType, key);
//  if (oldval != value)
//}
//
//function getBasePlotPrefKeyName(prefKey)
//{
//  return prefKey.substr( prefKey.lastIndexOf(".") + 1 );
//}
//
//function getBestStoredPlotPrefKey(dim, plotType, key)
//{
//  var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
//  var basePrefix = ["swp.plot"];
//  var nLastDot = key.lastIndexOf(".");
//  var subkey = key.substr( nLastDot + 1 );
//  var prefType, value, currPrefix;
//  for (; !value && (nLastDot >= basePrefix.length); nLastDot = currPrefix.lastIndexOf("."))
//  {
//    currPrefix = currPrefix.substr(0, nLastDot);
//    currKey = currPrefix + "." + subkey;
//    try 
//    {
//      var prefType = prefs.getPrefType(currKey);
//      if (prefType == prefs.PREF_STRING)
//        value = prefs.getCharPref(currKey);
//      else if (prefType == prefs.PREF_INT)
//      {
//        value = String(prefs.getIntPref(currKey));
//      }
//      else if (prefType == prefs.PREF_BOOL)
//      {
//        value = prefs.getBoolPref(currKey);
//        if (value)
//          value = "true";
//        else
//          value = "false";
//      }
//    } catch(ex) {}
//  }
//  return currKey;
//}

