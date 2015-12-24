 // Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.
 "use strict";

var gBodyElement;
var emptyElementStr=" ";
var data;

function fillUsedPackagesList(strResult, objArray) {
  var obj;
  var lineArray=[];
  var fieldArray=[];
  var pkg;
  lineArray = strResult.split(/\n/);
  var i, j;
  var addobj;
  var optstring;
  var optarray;
  var F = function (x) { return x.pkg===fieldArray[1];};
  for (i = 0; i < lineArray.length; i++) {
    fieldArray = lineArray[i].split(/;\s*/);
    if (fieldArray.length < 4) break;
    var o = objArray.filter(F)[0];
    if (o) {
      obj = o;
      addobj = false;
      obj={usedby: [], pkg: "", opt: [], pri: NaN};
      obj.pkg = fieldArray[1];
    } else {
      addobj = true;
      obj={usedby: [], pkg: "", opt: [], pri: NaN};
      obj.pkg = fieldArray[1];
    }
    if (fieldArray[2].length > 0) {
      optstring = fieldArray[2];
      optarray = optstring.split(/\s*,\s*/,"g");
      if (optstring.length > 0 && optarray.length === 0) {
        obj.opt.push(optstring);
      }
      else for (j = 0; j < optarray.length; j++) {
        obj.opt.push(optarray[j]);
      }
    }
    if (fieldArray[3] && !isNaN(Number(fieldArray[3])))
    {
      if (isNaN(obj.pri)) {
        obj.pri = fieldArray[3];
      }
      else {
        obj.pri = Math.min(obj.pri, Number(fieldArray[3]));
      }
    }
    obj.usedby.push(fieldArray[0]);
    if (addobj) objArray.push(obj);
  }
}

function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor) {
    window.close();
    return;
  }
  var princedoc = editorElement.contentDocument;

  doSetOKCancel(onAccept, onCancel);
  var xsltPath = "chrome://prince/content/optionspackages.xsl";
  var xslString;
  var strResult = "";
  var xsltProcessor = new XSLTProcessor();
  var request = new XMLHttpRequest();
  var attlist, colist, i, str;
  request.open("GET", xsltPath, false); 
  request.send(null);
  // print the name of the root element or error message
  var xslString = request.responseText;

  try{
    var parser = new DOMParser();
    var doc = parser.parseFromString(xslString, "text/xml");
    xsltProcessor.importStylesheet(doc);
    var newDoc = xsltProcessor.transformToDocument(princedoc);
    strResult = newDoc.documentElement.textContent || "";
  }
  catch (e)
  {
    msidump(e.message);
  }
  // strResult is a series of lines, each of which looks like
  // <tag>, <required package>, <options>, <priority>
  var packageList=[];
  var docClassElement;
  fillUsedPackagesList(strResult, packageList);
  data = window.arguments[0];
  data.Cancel = false;
  docClassElement = princedoc.documentElement.getElementsByTagName("documentclass")[0];
  gDialog.docClassName = docClassElement.getAttribute("class");
  gDialog.docClassOptions = [];
  colist = princedoc.documentElement.getElementsByTagName("colist");
  if (colist && colist.length >= 1) {
    attlist = colist[0].attributes;
    str = "";
    for (i = 0; i < attlist.length; i++) {
      att = attlist[i];
      if (att.value && att.value.length > 0) {
        gDialog.docClassOptions.push(att.value);
      }
    }
  }
  getDefaultPriorities(packageList);
  gDialog.packages = packageList;
  gDialog.packagesAdded = [];
  gDialog.packages.sort(comparePackagesByPriority);

  gDialog.globalLaTeXData = { packages : ReadInPackagesData("chrome://user/content/classes.pkg", gDialog.docClassName),
                              classOptions : ReadInOptionData("chrome://user/content/classes.opt", [gDialog.docClassName])[gDialog.docClassName] };
  gDialog.globalLaTeXData.packageOptions = ReadInOptionData("chrome://user/content/packgs.opt", gDialog.globalLaTeXData.packages);

  gDialog.packagesInUseListbox = document.getElementById("packagesInUseListbox");
  gDialog.currentPackageDescription = document.getElementById("currentPackageDescription");
  gDialog.descCaptionTemplate = document.getElementById("currentPackageDescriptionCaption").label;

  InitDialog();
//  document.getElementById("modifyButton").focus();
  msiSetInitialDialogFocus(document.getElementById("modifyButton"));

  SetWindowLocation();
}

function getDefaultPriorities(pklist) { 
  // get default priorities for those package declarations that don't override the priority
  // we save both the given priority (if given) and the final priority, since we need to know whether the user overrode the priority.
  var i, length, n, pkg, priDoc;
  length = pklist.length;
  priDoc = loadDefaultPriDefinitions();
  for (i=0; i < length; i++) {
    pkg = pklist[i];
    if (pkg) {
      n = Number(pkg.pri);
      if (n && !isNaN(n)) {
        pkg.finalpri = n;
      }
      else {
        n = getPriorityOf(pkg.pkg, priDoc);
        if (n && !isNaN(n)) {
          pkg.finalpri = n;
        }
      }
    }
  }
}

function loadDefaultPriDefinitions() {
  var request = new XMLHttpRequest();
  request.open("GET", "chrome://prnc2ltx/content/packages.xml", false); 
  request.send(null);
  return request.responseXML;
}

function getPriorityOf(name, xmldoc) {
  var names = xmldoc.getElementsByTagName("package");
  var i, length;
  length = names.length;
  var record;
  for (i = 0; i < length; i++) {
    record = names[i];
    if (record.getAttribute("name") === name) {
      return (record.getAttribute("pri"));
    } 
  }
  return null;
}


function InitDialog()
{
  setDocumentClassDescription(gDialog.docClassName);
  fillPackagesListbox(gDialog.packages);
  if (gDialog.packagesInUseListbox.getRowCount() > 0)
  {
    var selItem = gDialog.packagesInUseListbox.getItemAtIndex(0);
    gDialog.packagesInUseListbox.selectItem(selItem);
    selectPackage(selItem.label);
  }
  else
    setPackageDescriptionLine(null);
  document.getElementById("optionsDescriptionBox").value=gDialog.docClassOptions.join(",");
  checkDisabledControls();

//  checkInaccessibleAcceleratorKeys(document.documentElement);

//  gDialog.tabOrderArray = new Array( gDialog.positionSpecGroup,
//                                       document.documentElement.getButton("accept"),
//                                       document.documentElement.getButton("cancel") );
  
  document.documentElement.getButton("accept").setAttribute("default", true);
}

function getOptionNameMap()
{
  var obj = new Object();
  obj["orientation"] = "pgorient";
  obj["paper size"] = "papersize";
  obj["print side"] = "sides";
  obj["quality"] = "qual";
  obj["columns"] = "columns";
  obj["title page"] = "titlepage";
  obj["body text point size"] = "textsize";
  obj["equation numbering"] = "eqnnopos";
  obj["displayed equations"] = "eqnpos";
  obj["bibliography style"] = "bibstyle";
  return obj;
}

function addOptionNames(arr, docclass)
{ // we use the classes.opt file to map from an option name to the attribute name
  var i, n, name, counter;
  var returnArray=[];
  var request = new XMLHttpRequest();
  request.open("GET", "chrome://user/content/classes.opt", false); 
  request.overrideMimeType("text/plain");
  request.send(null);
  // print the name of the root element or error message
  var optfile = request.responseText;
  var regex1 = new RegExp("\\["+docclass+"\\]([^\\[]*)"); // selects the [classname] block
  var match = regex1.exec(optfile);
  if (1 < match.length) {
    optfile = match[1];
  } else {
     return null; // document class not found
  }
   
  var optNameMap = getOptionNameMap();
  counter=0;
  for (i = 0; i < arr.length; i++) {
    var regex2 = new RegExp("(^\\d+).*,\\s*"+arr[i]+"\\s*$","m");
    match = regex2.exec(optfile);
    if (match && match.length > 1) {
      n = match[1];
      // now look for header line: "n=<attribute name>"
    }
    var regex3 = new RegExp("^"+n+"=(.*)\s*$","m");
    match = regex3.exec(optfile);
    if (match && match.length > 1) {
      nm = (match[1].toLowerCase());
      if (optNameMap[nm]) {
        nm = optNameMap[nm];
      }
      else {
        nm = "option" + counter++;
      }
      returnArray.push(nm+"="+arr[i]); 
    }
  }
  return returnArray;
}

function onAccept()
{
  modifyPackagePriorities();
  var bPackagesAreReordered = false;
  data.packages = new Array();
  for (var i = 0; i < gDialog.packages.length; ++i)
  {
    data.packages[i] = new Object();
    data.packages[i].packageName = gDialog.packages[i].pkg;
    if (gDialog.packages[i].opt)
      data.packages[i].packageOptions = gDialog.packages[i].opt.join(",");
    if ("pri" in gDialog.packages[i])
      data.packages[i].packagePriority = gDialog.packages[i].pri;
  }
  data.docClassOptions = addOptionNames(gDialog.docClassOptions, gDialog.docClassName);

  var parentEditorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (!theWindow || !("reviseLaTeXPackagesAndOptions" in theWindow))
    theWindow = msiGetTopLevelWindow();

  theWindow.reviseLaTeXPackagesAndOptions(parentEditorElement, data);
  SaveWindowLocation();
  return true;
}

function onCancel()
{
  data.Cancel = true;
}

function doAccept()
{
  document.documentElement.getButton('accept').oncommand();
}


function setDocumentClassDescription(className)
{
  var docClassControl = document.getElementById('documentClassDescription');
  var initialClassString = docClassControl.value;
  var docClassString = initialClassString.replace(docClassControl.getAttribute('replaceTemplate'), className);
  docClassControl.value = docClassString;
}

function goNative()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var packagesData = new Object();
  packagesData.packages = gDialog.packages;
  window.openDialog("chrome://prince/content/typesetNativePackages.xul", "nativepackages", "chrome,close,titlebar,resizable,modal", packagesData);
  if (!packagesData.Cancel)
  {
    gDialog.packages = packagesData.packages;
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
}

function doModifyDialog()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var classOptionData = new Object();
  classOptionData.isPackage = false;
  classOptionData.theName = gDialog.docClassName;
  classOptionData.optionSet = gDialog.globalLaTeXData.classOptions;
//  gDialog.selectedOptionsStr = data.optionsStr;

  classOptionData.options = gDialog.docClassOptions;
  window.openDialog("chrome://prince/content/typesetStyleOptions.xul", "styleoptions", "chrome,close,titlebar,resizable,modal", classOptionData);
  if (!classOptionData.Cancel)
  {
    gDialog.docClassOptions = classOptionData.options;
    document.getElementById("optionsDescriptionBox").value=gDialog.docClassOptions.join(",");
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
}

function movePackage(bUp)
{
  var currIndex = gDialog.packagesInUseListbox.selectedIndex;
  if (currIndex < 0)
    return;
  var packageName = gDialog.packagesInUseListbox.getSelectedItem(0).label;
  var valueStr = gDialog.packagesInUseListbox.getSelectedItem(0).value;
  var isHighlighted = (gDialog.packagesInUseListbox.getSelectedItem(0).getAttribute("class")==="highlight");
  var desiredIndex = currIndex + 1;
  if (bUp)
    desiredIndex = currIndex - 1;
  if (desiredIndex >= 0 && desiredIndex < gDialog.packagesInUseListbox.getRowCount())
  {
    gDialog.packagesInUseListbox.removeItemAt(currIndex);
    var newItem = gDialog.packagesInUseListbox.insertItemAt(desiredIndex, packageName, valueStr);
    if (isHighlighted) newItem.setAttribute("class","highlight")
    gDialog.packagesInUseListbox.selectItem(newItem);
  }
  if (packageName && packageName.length > 0)
  {
    currIndex = findPackageByName(gDialog.packages, packageName);
    if (currIndex >= 0)
    {
      desiredIndex = currIndex + 1;
      if (bUp)
        desiredIndex = currIndex - 1;
      if (desiredIndex >= 0 && desiredIndex < gDialog.packages.length)
      {
        var packagesItems = gDialog.packages.splice(currIndex, 1);
        gDialog.packages.splice(desiredIndex, 0, packagesItems[0]);
      }
    }
  }
  checkDisabledControls();
}

function addPackage()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var addPackageData = new Object();
  addPackageData.packages = gDialog.packages;
  addPackageData.docClassName = gDialog.docClassName;
  addPackageData.packagesAvailable = gDialog.globalLaTeXData.packages;
  window.openDialog("chrome://prince/content/typesetAddPackage.xul", "addpackage", "chrome,close,resizable,titlebar,modal", addPackageData);
  if (!addPackageData.Cancel)
  {
    var newPackage = addPackageData.newPackage;
    if ( (newPackage.length > 0) && (findPackageByName(gDialog.packages, newPackage) < 0) )  //if we're actually adding anything
    {
      var newPackage = {};
      newPackage.pkg = addPackageData.newPackage;
      gDialog.packagesAdded.push(newPackage);
      gDialog.packages.push(newPackage);
      gDialog.packagesInUseListbox.selectItem( gDialog.packagesInUseListbox.appendItem(addPackageData.newPackage, '') );
      selectPackage(newPackage.pkg);
      checkDisabledControls();
      msiGetEditor(editorElement).incrementModificationCount(1);
    }
  }
}

function removeCurrentlySelectedPackage()
{
  var selItem = gDialog.packagesInUseListbox.getSelectedItem(0);
  removePackage(selItem);
  checkDisabledControls();
}

function removePackage(packageListItem)
{
  var packageName = packageListItem.label;
  var whichPackage = findPackageByName(gDialog.packages, packageName);
  gDialog.packages.splice(whichPackage, 1);
  whichPackage = gDialog.packagesInUseListbox.getIndexOfItem(packageListItem);
  gDialog.packagesInUseListbox.removeItemAt(whichPackage);
  whichPackage = findPackageByName(gDialog.packagesAdded, packageName);
  if (whichPackage >= 0)
    gDialog.packagesAdded.splice(whichPackage, 1);
}

function modifyCurrentlySelectedPackage()
{
  var selItem = gDialog.packagesInUseListbox.getSelectedItem(0);
  modifyPackage(selItem.label);
}

function modifyPackage(packageName)
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var whichPackage = findPackageByName(gDialog.packages, packageName);
  var packageData = new Object();
  packageData.isPackage = true;
  packageData.options = null;
  if (whichPackage >= 0)
  {
    packageData.options = gDialog.packages[whichPackage].opt;
    packageData.theName = gDialog.packages[whichPackage].pkg;
    packageData.optionSet = gDialog.globalLaTeXData.packageOptions[packageName];
  }

  window.openDialog("chrome://prince/content/typesetStyleOptions.xul", "styleoptions", "chrome,close,titlebar,resizable,modal", packageData);
  if (!packageData.Cancel)
  {
    gDialog.packages[whichPackage].opt = packageData.options;
    setPackageDescriptionLine(gDialog.packages[whichPackage]);
    msiGetEditor(editorElement).incrementModificationCount(1);
  }
}


function fillPackagesListbox(packageList)
{
  var item;
  var pkgObj;
  for (var i = 0; i < packageList.length; ++i)
  {
    pkgObj = packageList[i];
    item = gDialog.packagesInUseListbox.appendItem(pkgObj.pkg, pkgObj.opt);
    if (pkgObj.usedby.indexOf("usepackage") === -1)
      item.setAttribute("class","highlight");
  }
}


function checkDisabledControls()
{
  var numPackages = gDialog.packagesInUseListbox.getRowCount();
  var selectedIndex = gDialog.packagesInUseListbox.selectedIndex;
  if (selectedIndex < 0)
  {
    document.getElementById('moveUpButton').setAttribute('disabled','true');
    document.getElementById('moveDownButton').setAttribute('disabled','true');
    document.getElementById('removePackageButton').setAttribute('disabled','true');
    document.getElementById('modifyPackageButton').setAttribute('disabled','true');
  }
  else
  {
    document.getElementById('removePackageButton').setAttribute('disabled','false');
    document.getElementById('modifyPackageButton').setAttribute('disabled','false');
    if (selectedIndex >0)
      document.getElementById('moveUpButton').setAttribute('disabled','false');
    else
      document.getElementById('moveUpButton').setAttribute('disabled','true');
    if (selectedIndex < numPackages)
      document.getElementById('moveDownButton').setAttribute('disabled','false');
    else
      document.getElementById('moveDownButton').setAttribute('disabled','true');
  }
}

function onSelectPackage()
{
  var pkgName = null;
  var selItem = gDialog.packagesInUseListbox.getSelectedItem(0);
  if (selItem)
    pkgName = selItem.label;
  selectPackage(pkgName);
}

function selectPackage(packageName)
{
  var whichPackage = -1;
  var thePackage = null;
  if (packageName)
    whichPackage = findPackageByName(gDialog.packages, packageName);
  if (whichPackage >= 0)
    thePackage = gDialog.packages[whichPackage];
  if ( (packageName!=null) && (thePackage == null) )
    alert("Package " + packageName + " not found in package list!");
  setPackageDescriptionLine(thePackage);
  checkDisabledControls();
}

function setPackageDescriptionLine(thePackage)
{
  if (thePackage == null)
  {
    document.getElementById("currentPackageDescriptionCaption").label = "";
    gDialog.currentPackageDescription.value = "";
  }
  else
  {
    var caption = document.getElementById("currentPackageDescriptionCaption");
    caption.label = gDialog.descCaptionTemplate.replace(caption.getAttribute("replaceTemplate"), thePackage.pkg);
    if (!thePackage.opt) {
      thePackage.opt = [];
    }
    gDialog.currentPackageDescription.value = thePackage.opt.join();
  }
}

function comparePackagesByPriority(a,b)
{
  var aPriority = 100;
  var bPriority = 100;
  if (a.pri && !isNaN(a.finalpri))
    aPriority = a.finalpri;
  if (b.finalpri && !isNaN(b.finalpri))
    bPriority = b.finalpri;
  if (aPriority < bPriority)
    return -1; 
  if (aPriority > bPriority)
    return 1;
  return 0;
}

//The algorithm here is intended to do the following:
//  (i) Reflect any changes the user made to ordering in priority settings;
// (ii) Leave existing priorities as near unchanged as possible.
//The algorithm in brief:
//  (i) Check the original priority numbers against position to see who's been moved; basically use the same set of numbers
//      but assigned to different packages if necessary (exception: if packages have the same priority but one has been
//      moved past the others that should be reflected by adding or subtracting the smallest possible number);
// (ii) Added packages at the top of the list will then force topmost numbers down a bit;
//(iii) Added packages at the bottom of the list may not receive priorities unless we know they've been moved past each other. If
//        they do, they may force the bottom numbers up a bit;
// (iv) Finally, the remaining (necessarily added) packages in the middle of the list will get interpolated.

function modifyPackagePriorities()
{
  var newPriorities = getModifiedPriorityList();
  for (var ix = 0; ix < gDialog.packagesInUseListbox.getRowCount(); ++ix)
  {
    whichPackage = findPackageByName(gDialog.packages, gDialog.packagesInUseListbox.getItemAtIndex(ix).label);
    if (whichPackage >= 0)
      gDialog.packages[whichPackage].pri = newPriorities[ix];
  }
}

function getModifiedPriorityList()
{
  var nPackages = gDialog.packagesInUseListbox.getRowCount();
  var packageArray = [];
  var priorityValues = [];
  var origPackages = [];
  var revisedPriorities = [];
  var newPackages = [];
  var thePackage;
  var whichPackage;
  for (var ix = 0; ix < nPackages; ++ix)
  {
    whichPackage = findPackageByName(gDialog.packages, gDialog.packagesInUseListbox.getItemAtIndex(ix).label);
    thePackage = gDialog.packages[whichPackage];
    packageArray.push(thePackage);
    if (thePackage != null && "pri" in thePackage)
    {
      priorityValues.push(thePackage.pri);
      origPackages.push(thePackage);
    }
    else
      newPackages.push(thePackage);
    revisedPriorities[ix] = 0;
  }

  function sortTopFirst(a,b)
  { return ( (a < b) ? 1 : ( (a > b) ? -1 : 0 ) ) };

  //Return 1 if a is expected to come after b in the list; this is true if a was added later than b,
  // or if both were present originally but with no priority and a is alphabetically after b.
  function compareUnPrioritizedPackages(a,b)  
  { var nCompare = 0;
    var aOrder = gDialog.packagesAdded.indexOf(a);
    var bOrder = gDialog.packagesAdded.indexOf(b);
    if (aOrder>=0 && bOrder>=0)
      nCompare = (aOrder > bOrder ? 1 : (aOrder < bOrder ? -1 : 0));
    else if (aOrder >= 0)
      nCompare = 1;
    else if (bOrder >= 0)
      nCompare = -1;
    if (!nCompare)
      nCompare = a.packageName.toLowerCase().localeCompare(b.packageName.toLowerCase());
  }

  priorityValues.sort( sortTopFirst );
  origPackages.sort( comparePackagesByPriority );
  newPackages.sort( compareUnPrioritizedPackages );

  var maxPriority = 200;
  var minPriority = 5;
  var currPri = 0;
//  var priAdjustment = 0;
  var interCount = [];
  var lastVal = maxPriority;
  var aPackage = null;
  var anIndex = -1;
  var jx;
  for (ix = 0; ix < nPackages; ++ix)
  {
    aPackage = packageArray[ix];
    anIndex = origPackages.indexOf(aPackage);
    if (anIndex >= 0)
    {
      interCount[currPri] = 0;
      revisedPriorities[ix] = priorityValues[currPri];
      //Now adjust near ties
      for (jx = ix-1; jx >= 0; --jx)
      {
        if ( (revisedPriorities[jx] > 0) && (revisedPriorities[jx] <= revisedPriorities[ix]) )
        {
          if ( origPackages.indexOf( packageArray[jx] ) > anIndex )    //So it comes before us now but was originally after us
            revisedPriorities[ix] = revisedPriorities[jx] - 1;
          else if ( packageArray[jx].priority - aPackage.priority > 0 ) //Or it really got a higher priority and we shouldn't end up even with it
            revisedPriorities[ix] = revisedPriorities[jx] - 1;
          else
            revisedPriorities[ix] = revisedPriorities[jx];
          break;
        }
      }
      priorityValues[currPri++] = revisedPriorities[ix];
    }
  }
  interCount[currPri] = 0;
  var currGaps = [];
  var currGap = 0;
  for (jx = 0; jx < priorityValues.length; ++jx)
  {
    if (jx == 0)
      currGaps[currGap++] = maxPriority - priorityValues[0];
//    else if (sortedValues[jx] + 1 > sortedValues[jx-1])  //don't want to count a "gap" for two which are yoked together, either the same or different only by 1
    else
      currGaps[currGap++] = priorityValues[jx-1] - priorityValues[jx];
  }
//    else if (jx == sortedValues.length - 1)
  currGaps[currGap] = priorityValues[priorityValues.length - 1] - minPriority;

  //Now count up the ones which need to be inserted betweeen the others
  currGap = 0;
  for (ix = 0; ix < nPackages; ++ix)
  {
    aPackage = packageArray[ix];
    if (origPackages.indexOf(aPackage) < 0)
    {
      if (!interCount[currGap])
        interCount[currGap] = 2;  //Need to ensure that there is a gap of at least 2 to insert this one into
      else if ( newPackages.indexOf(aPackage) > newPackages.indexOf( packageArray[ix-1] ) )  //In this case, one has been moved and they shouldn't use the same value.
        ++interCount[currGap];
    }
    else
    {
      ++currGap;
    }
  }
  if (interCount[0] > 1)
    --interCount[0];  //Don't need an extra spot at the bottom or top
  if (interCount[interCount.length - 1] > 1)
    --interCount[interCount.length - 1];  //Don't need an extra spot at the bottom or top

  function adjustGaps(currentGaps, neededGaps, limits)
  {
    var excess = [];
    var totalExcess = 0;
    var positiveExcess = 0;
    var resultGaps = [];
    var bNeedChanges = false;
    for (var ii = 0; ii < neededGaps.length; ++ii)
    {
      excess[ii] = currentGaps[ii] - neededGaps[ii];
      totalExcess += excess[ii];
      if (excess[ii] > 0)
        positiveExcess += excess[ii];
      else if (excess[ii] < 0)
        bNeedChanges = true;
    }
    if (!bNeedChanges)
    {
      resultGaps = resultGaps.concat(currentGaps);
      return resultGaps;
    }

    if (totalExcess <= 0)  //Should really never happen! There aren't enough values to fit everything in as needed.
    {
      if (limits.mMin > 1)
      {
        totalExcess += limits.mMin - 1;
        if ( (currentGaps[ currentGaps.length - 1 ] + limits.mMin - 1) > 0)
          positiveExcess += (currentGaps[ currentGaps.length - 1 ] + limits.mMin - 1);
        currentGaps[ currentGaps.length - 1 ] += limits.mMin - 1;
        excess[currentGaps.length - 1] += limits.mMin - 1;
        limits.mMin = 1;
      }
      if (totalExcess <= 5)  //insist on 5? why?
      {
        limits.mMax += (5-totalExcess);  //raise it
        if (currentGaps[0] + 5 - totalExcess > 0)
          positiveExcess += currentGaps[0] + 5 - totalExcess;
        currentGaps[0] += (5-totalExcess);
        excess[0] += 5 - totalExcess;
        totalExcess = 5;
      }
    }

    for (ii = 0; ii < currentGaps.length; ++ii)
    {
      if (excess[ii] > 0)
        resultGaps[ii] = Math.floor( (excess[ii] / positiveExcess) * totalExcess );
      else
        resultGaps[ii] = 0;
    }
    //Now adjust for roundoff and pass this array back.
    return resultGaps;
  }

  var extrema = { mMin : minPriority, mMax : maxPriority };
  var revisedGaps = adjustGaps(currGaps, interCount, extrema);

  minPriority = extrema.mMin;
  maxPriority = extrema.mMax;
  currGap = 0;
  var offsetCount = 0;
  var currVal = maxPriority;
  for (ix = 0; ix < nPackages; ++ix)
  {
    aPackage = packageArray[ix];
    if (origPackages.indexOf(aPackage) >= 0)  //Start by resetting the ones which were positive
    {
      currVal = revisedPriorities[ix] = currVal - revisedGaps[currGap++];
      offsetCount = 0;
    }
    else
    {
      anIndex = newPackages.indexOf(aPackage);
//      if ( !offsetCount || (valArray[ix-1] < valArray[ix]) )
      if ( !offsetCount || (newPackages.indexOf(packageArray[ix-1]) > anIndex) )  //Has been moved, can't share a value
        ++offsetCount;
      revisedPriorities[ix] = currVal - Math.floor( (offsetCount) * revisedGaps[currGap] / interCount[currGap] );
    }
  }

  return revisedPriorities;
}


function doTestDialog()
{
  modifyPackagePriorities();
  var bPackagesAreReordered = false;
  data.packages = new Array();
  for (var i = 0; i < gDialog.packages.length; ++i)
  {
    data.packages[i] = new Object();
    data.packages[i].packageName = gDialog.packages[i].pkg;
    data.packages[i].packageOptions = gDialog.packages[i].opt.join(",");
    if ("pri" in gDialog.packages[i])
      data.packages[i].packagePriority = gDialog.packages[i].pri;
  }
  data.docClassOptions = gDialog.docClassOptions;

  var parentEditorElement = msiGetParentEditorElementForDialog(window);
  var theWindow = window.opener;
  if (!theWindow || !("reviseLaTeXPackagesAndOptions" in theWindow))
    theWindow = msiGetTopLevelWindow();

  theWindow.reviseLaTeXPackagesAndOptions(parentEditorElement, data);
}

function doTestResetPriorities()
{
  var revisedArray = getModifiedPriorityList();
  var numStrArray = [];
//  var numbersStr = document.getElementById("resetPriorityTestBox").value;
//  var numStrArray = numbersStr.split(",");
//  var valArray = [];
//  for (var ii = 0; ii < numStrArray.length; ++ii)
//    valArray[ii] = Number(numStrArray[ii]);
//  var revisedArray = resetPrioritiesTest(valArray);
  for (var ii = 0; ii < revisedArray.length; ++ii)
    numStrArray[ii] = String(revisedArray[ii]);
  document.getElementById("resetPriorityResultBox").value = numStrArray.join(",");
}

function resetPrioritiesTest(valArray)
{
  var sortedValues = [];
  var revisedValues = [];
  for (var ix = 0; ix < valArray.length; ++ix)
  {
    if (valArray[ix] >= 1)
    {
      sortedValues.push( Math.floor(valArray[ix]) );
    }
    revisedValues[ix] = 0;
  }

  function sortTopFirst(a,b)
  { return ( (a < b) ? 1 : ( (a > b) ? -1 : 0 ) ) };

//Should we create an array by index of the order these started in?
  sortedValues.sort( sortTopFirst );
  var firstGaps = [];
  var maxPriority = 200;
  var minPriority = 5;
  var currPri = 0;
//  var priAdjustment = 0;
  var interCount = [];
  var lastVal = maxPriority;
  for (ix = 0; ix < valArray.length; ++ix)
  {
    if (valArray[ix] >= 1)
    {
      interCount[currPri] = 0;
      revisedValues[ix] = sortedValues[currPri];
      for (var jx = ix-1; jx >= 0; --jx)
      {
        if ( (revisedValues[jx] > 0) && (revisedValues[jx] <= revisedValues[ix]) )
        {
          if ( (valArray[jx] < valArray[ix]) || (valArray[jx] - valArray[ix] > 1) )
            revisedValues[ix] = revisedValues[jx] - 1;
          else
            revisedValues[ix] = revisedValues[jx];
          break;
        }
      }
//      revisedValues[ix] += priAdjustment;
      sortedValues[currPri++] = revisedValues[ix];
    }
  }
  interCount[currPri] = 0;
  var currGaps = [];
  var currGap = 0;
  for (var jx = 0; jx < sortedValues.length; ++jx)
  {
    if (jx == 0)
      currGaps[currGap++] = maxPriority - sortedValues[0];
//    else if (sortedValues[jx] + 1 > sortedValues[jx-1])  //don't want to count a "gap" for two which are yoked together, either the same or different only by 1
    else
      currGaps[currGap++] = sortedValues[jx-1] - sortedValues[jx];
  }
//    else if (jx == sortedValues.length - 1)
  currGaps[currGap] = sortedValues[sortedValues.length - 1] - minPriority;
  //Now count up the ones which need to be inserted betweeen the others
  currGap = 0;
  for (ix = 0; ix < valArray.length; ++ix)
  {
    if (valArray[ix] < 1)  //We want to include the ones which had priority 0
    {
      if (!interCount[currGap])
        interCount[currGap] = 2;  //Need to ensure that there is a gap of at least 2 to insert this one into
      else if (valArray[ix] > valArray[ix-1])  //In this case, one has been moved and they shouldn't use the same value.
            ++interCount[currGap];
    }
    else
    {
      ++currGap;
    }
  }

  function adjustGaps(currentGaps, neededGaps, limits)
  {
    var excess = [];
    var totalExcess = 0;
    var positiveExcess = 0;
    var resultGaps = [];
    var bNeedChanges = false;
    for (var ii = 0; ii < neededGaps.length; ++ii)
    {
      excess[ii] = currentGaps[ii] - neededGaps[ii];
      totalExcess += excess[ii];
      if (excess[ii] > 0)
        positiveExcess += excess[ii];
      else if (excess[ii] < 0)
        bNeedChanges = true;
    }
    if (!bNeedChanges)
    {
      resultGaps = resultGaps.concat(currentGaps);
      return resultGaps;
    }

    if (totalExcess <= 0)  //Should really never happen! There aren't enough values to fit everything in as needed.
    {
      if (limits.mMin > 1)
      {
        totalExcess += limits.mMin - 1;
        if ( (currentGaps[ currentGaps.length - 1 ] + limits.mMin - 1) > 0)
          positiveExcess += (currentGaps[ currentGaps.length - 1 ] + limits.mMin - 1);
        currentGaps[ currentGaps.length - 1 ] += limits.mMin - 1;
        excess[currentGaps.length - 1] += limits.mMin - 1;
        limits.mMin = 1;
      }
      if (totalExcess <= 5)  //insist on 5? why?
      {
        limits.mMax += (5-totalExcess);  //raise it
        if (currentGaps[0] + 5 - totalExcess > 0)
          positiveExcess += currentGaps[0] + 5 - totalExcess;
        currentGaps[0] += (5-totalExcess);
        excess[0] += 5 - totalExcess;
        totalExcess = 5;
      }
    }

    for (ii = 0; ii < currentGaps.length; ++ii)
    {
      if (excess[ii] > 0)
        resultGaps[ii] = Math.floor( (excess[ii] / positiveExcess) * totalExcess );
      else
        resultGaps[ii] = 0;
    }
    //Now adjust for roundoff and pass this array back.
    return resultGaps;
  }

  var extrema = { mMin : minPriority, mMax : maxPriority };
  var revisedGaps = adjustGaps(currGaps, interCount, extrema);

  minPriority = extrema.mMin;
  maxPriority = extrema.mMax;
  currGap = 0;
  var offsetCount = 0;
  var currVal = maxPriority;
  for (ix = 0; ix < valArray.length; ++ix)
  {
    if (valArray[ix] >= 1)  //Start by resetting the ones which were positive
    {
      currVal = revisedValues[ix] = currVal - revisedGaps[currGap++];
      offsetCount = 0;
    }
    else
    {
      if ( !offsetCount || (valArray[ix-1] < valArray[ix]) )
        ++offsetCount;
      revisedValues[ix] = currVal - Math.floor( (offsetCount) * revisedGaps[currGap] / interCount[currGap] );
    }
  }

  return revisedValues;
}

//Option data look like:
//1=Body text point size
//1.1=10pt - default,
//1.2=11pt,11pt
//1.3=12pt,12pt

var optionHeadingRE = /^\s*([0-9]+)\s*=\s*([^,]*)(\,.*)?/;
//var optionRE = /^\s*([0-9]+)\.([0-9]+)\s*=([^\,]+)(\s*\-\s*default)?\,([^\s]*)$/
var optionRE = /^\s*([0-9]+)\.([0-9]+)\s*=([^\,]+)(\,.*)?$/;
var choiceDefaultRE = /\s*([^\s].*)(\s+\-\s+default)/;      //Why doesn't this work?
var sectionRE = /^\s*\[([^\]]+)\]/;
var stripSpacesRE = /^\s*([^\s].*[^\s])\s*$/;

function ReadInPackagesData(packagesFile, className)
{
  var theLines = GetLinesFromFile(packagesFile);
  var pkgList = [];
  var sectionData = {mTitle : "", mStart : 0, mEnd : 0};
  var found = null;
  while (FindNextBracketDelimitedSection(theLines, sectionData))
  {
    if (sectionData.mTitle == className)
      break;
    sectionData.mStart = sectionData.mEnd + 1;
  }
  if (sectionData.mTitle == className)
  {
    for (var ix = sectionData.mStart+1; ix <= sectionData.mEnd; ++ix)
    {
      found = stripSpacesRE.exec(theLines[ix]);
      if (found && found[1] && found[1].length)
        pkgList.push(found[1]);
    }
  }
  return pkgList;
}

function GetLinesFromFile(aFileUrl)
{
//  var fileHandler = msiGetFileProtocolHandler();
//  var theFile = fileHandler.getFileFromURLSpec(aFileUrl);
//  // open an input stream from file
//  var istream = Components.classes["@mozilla.org/network/file-input-stream;1"].
//                          createInstance(Components.interfaces.nsIFileInputStream);
//  istream.init(theFile, 1, 0, false);
//  istream.QueryInterface(Components.interfaces.nsILineInputStream);

  var theText = "";
//  var req = Components.classes["@mozilla.org/xmlextras/xmlhttprequest;1"]  
//                     .createInstance(Components.interfaces.nsIXMLHttpRequest);  
//  req.onprogress = onProgress;  
//  req.onload = onLoad;  
//  req.onerror = onError;  
  var req = new XMLHttpRequest();
  req.open("GET", aFileUrl, false);  
  req.overrideMimeType("text/plain");
  req.send(null);
  if (req.status == 0)
    theText = req.responseText;
  // read lines into array
  var lines = theText.split("\n");
  return lines;
}

function ReadInOptionData(optionsFile, sectionArray)
{
  var theLines = GetLinesFromFile(optionsFile);
  var optionList = {};
  var foundSections = 0;
  var sectionData = {mTitle : "", mStart : 0, mEnd : 0};
  while (FindNextBracketDelimitedSection(theLines, sectionData))
  {
    if (sectionArray.indexOf(sectionData.mTitle) >= 0)
    {
      optionList[sectionData.mTitle] = ReadInAnOptionSection(theLines, sectionData);
      ++foundSections;
      if (foundSections == sectionArray.length)
        break;
    }
    sectionData.mStart = sectionData.mEnd + 1;
  }
  return optionList;
}

function FindOptionByNumber(optionSection, optionNum)
{
  for (var whichOption in optionSection)
  {
    if ( (optionSection[whichOption] != null) && (optionSection[whichOption].mNumber == optionNum) )
      return optionSection[whichOption];
  }
  return null;
}

function ReadInAnOptionSection(lineList, sectionData)
{
  var foundHeader = null, foundChoice = null, foundLine = null;
  var optionSectionData = {};
  var currOption = null;
  var choiceName, choiceData;
  var bIsDefault = false;
  for (var ix = sectionData.mStart + 1; ix <= sectionData.mEnd; ++ix)
  {
    if (lineList[ix].charAt(0) == ";")
      continue;
    foundLine = stripSpacesRE.exec(lineList[ix]);
    if (!foundLine || !foundLine[1] || !foundLine[1].length)
      continue;
    if (foundHeader = optionHeadingRE.exec(foundLine[1]))
    {
      if (foundHeader[2] in optionSectionData)
        dump("Trouble in typesetOptionsAndPackages.js, ReadInAnOptionSection; option [" + foundHeader[2] + "] appears twice in section [" + sectionData.mTitle + "]!\n");
      else
        optionSectionData[foundHeader[2]] = {mNumber : Number(foundHeader[1]), mChoices : {} };//Index it by option name string
      currOption = optionSectionData[foundHeader[2]];
    }
    else if (foundChoice = optionRE.exec(foundLine[1]))
    {
      if (currOption && (currOption.mNumber != foundChoice[1]) )
        currOption = FindOptionByNumber(optionSectionData, foundChoice[1]);
      if (currOption)
      {
        choiceName = foundChoice[3];
        choiceData = null;
        if (foundChoice[4])
          choiceData = foundChoice[4].substr( foundChoice[4].indexOf(",") + 1 );  //Everything following the comma, or everything if it's not there??

//        if (foundDefault = choiceDefaultRE.exec(choiceName))
//        {
//          choiceName = foundDefault[1];
//          bIsDefault = true;
//        }
        currOption.mChoices[choiceName] = {mChoiceNumber : foundChoice[2], mChoiceData : choiceData};
      }
    }
  }
  return optionSectionData;
}

function FindNextBracketDelimitedSection(lineList, sectionData)
{
  var found = null, foundLine = null;
  var foundStart = false;
  var foundEnd = false;
  for (var ix = sectionData.mStart; ix < lineList.length; ++ix)
  {
    if (lineList[ix].charAt(0) == ";")  //skip comment lines!
      continue;
    foundLine = stripSpacesRE.exec(lineList[ix]);
    if (!foundLine || !foundLine[1] || !foundLine[1].length)
      continue;
    if (found = sectionRE.exec(foundLine[1]))
    {
      if (!foundStart)
      {
        sectionData.mStart = ix;
        sectionData.mTitle = found[1];
        foundStart = true;
      }
      else
      {
        foundEnd = true;
        sectionData.mEnd = ix-1;
        break;
      }
    }
  }
  if (!foundEnd)
      sectionData.mEnd = ix-1;
  return foundStart;
}
