// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

//var target;

function dlgAutoSubsList()
{
  this.names = new Object();
  this.canAdd = function(subsName)
  {
    if (subsName in this.names)
      return false;
    return true;
  };

  this.saveSub = function(theSub, theType, theContext, theData, contextMarkupStr, appearanceList)
  {
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    var context = Components.interfaces.msiIAutosub.CONTEXT_MATHANDTEXT;
    if (theContext == "math")
    {
      context = Components.interfaces.msiIAutosub.CONTEXT_MATHONLY;
      if (contextMarkupStr == null || contextMarkupStr.length == 0)
        contextMarkupStr = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"></math>";
    }
    else if (theContext == "text")
      context = Components.interfaces.msiIAutosub.CONTEXT_TEXTONLY;
    var action = Components.interfaces.msiIAutosub.ACTION_SUBSTITUTE;
    if (theType == "script")
      action = Components.interfaces.msiIAutosub.ACTION_EXECUTE;
    else if (appearanceList != null)
    {
      var serialize = new XMLSerializer();
      theData = "";
      for (var ix = 0; ix < appearanceList.length; ++ix)
        if (!((appearanceList[ix].tagName == "br") && appearanceList[ix].hasAttribute("temp")))
          theData += serialize.serializeToString(appearanceList[ix],'UTF-8');
    }
    var bAdded = false;
    try { bAdded = autosub.addEntry( theSub, context, action, theData, contextMarkupStr, ""); }
    catch(exc) {dump("Error in autoSubstituteDialog.js, in dlgAutoSubsList.saveSub; error is [" + exc + "].\n"); bAdded = false;}
//    dump("In autoSubstituteDialog.js, dlgAutoSubsList.addSub; adding [" + theSub + "], with data: [" + theData + "]; bAdded is [" + bAdded + "].\n");
    if (bAdded)
    {
      this.bModified = true;
      var newObj = new Object();
//      if (theType == "sc")
//      {
        newObj.theContext = "";
        newObj.theInfo = "";
//      }
//      else  --> what could we do here? Should we add controls for specifying "context" and "paste info"??? Not now...
//      {
//      }
      newObj.mathContext = theContext;
      newObj.theData = theData;
      newObj.type = theType;
      this.names[theSub] = newObj;
      var ACSA = msiSearchStringManager.setACSAImpGetService();
//      var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//      ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
      ACSA.addString("autosubstitution", theSub);
    }
  };

  this.synchronizeACSA = function()
  {
    var ACSA = msiSearchStringManager.setACSAImpGetService();
//    var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    for (var aName in this.names)
    {
      ACSA.addString("autosubstitution", aName);
    }
  };

//  this.modifySub = function(theSub, theType, theContext, theData, contextMarkupStr, appearanceList)
//  {
//    if (theSub in this.names)
//      this.removeSub(theSub);
//    this.addSub(theSub, theType, theContext, theData, contextMarkupStr, appearanceList);
//  };

  this.removeSub = function(theSub)
  {
    var bRemoved = false;
    try
    {
      var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
      bRemoved = autosub.removeEntry(theSub);
      var ACSA = msiSearchStringManager.setACSAImpGetService();
//      var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//      ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
      ACSA.deleteString("autosubstitution", theSub);
    }
    catch(exc) {dump("Error in autoSubstituteDialog.js, in dlgAutoSubsList.removeSub; error is [" + exc + "].\n"); bRemoved = false;}
    document.getElementById("keystrokesBox").value = "";
    changePattern("");
    this.bModified = this.bModified || bRemoved;
  };

  this.saveToFile = function()
  {
    if (!this.bModified)
      return;
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    autosub.Save();
  };

  this.enableAutoSubstitution = function(bEnable, context)
  {
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    autosub.enableAutoSubstitution( bEnable, context );
  };

  this.isAutoSubstitutionEnabled = function(bMath)
  {
    var autosub = Components.classes["@mozilla.org/autosubstitute;1"].getService(Components.interfaces.msiIAutosub);
    return autosub.isAutoSubstitutionEnabled( bMath );
  };

}

function msiEditorChangeObserver(editorElement)
{
  this.mEditorElement = editorElement;
  this.observe = function(aSubject, aTopic, aData)
  {
    // Should we allow this even if NOT the focused editor?
//    msiDumpWithID("In autoSubstituteDialog documentCreated observer for editor [@], observing [" + aTopic + "].\n", this.mEditorElement);
    if (!this.mEditorElement.docShell)
    {
      msiDumpWithID("In autoSubstituteDialog documentCreated observer for editor [@], returning as docShell is null.\n", this.mEditorElement);
      return;
    }
    var commandManager = msiGetCommandManager(this.mEditorElement);
    if (commandManager != aSubject)
    {
      msiDumpWithID("In autoSubstituteDialog documentCreated observer for editor [@], observing [" + aTopic + "]; returning, as commandManager doesn't equal aSubject; aSubject is [" + aSubject + "], while commandManager is [" + commandManager + "].\n", this.mEditorElement);
//      if (commandManager != null)
        return;
    }

    switch(aTopic)
    {
      case "cmd_bold":
      case "cmd_setDocumentModified":
      {
        checkSubstitutionControl(this.mEditorElement);
      }
      break;

      case "obs_documentCreated":
      {
        var bIsRealDocument = false;
        var currentURL = msiGetEditorURL(this.mEditorElement);
//        msiDumpWithID("For editor element [@], currentURL is " + currentURL + "].\n", this.mEditorElement);
        if (currentURL != null)
        {
          var fileName = GetFilename(currentURL);
          bIsRealDocument = (fileName != null && fileName.length > 0);
        }
        if (bIsRealDocument)
        {
          if (!gDialog.bEditorReady)
          {
            gDialog.bEditorReady = true;
            gDialog.subsList.synchronizeACSA();  //want to do this after the editor is loaded, since the active StringArray "imp" has changed.
          }
        }
//        else
//          msiDumpWithID("In autoSubstituteDialog documentCreated observer for editor [@], bIsRealDocument is false.\n", this.mEditorElement);
//        setControlsForSubstitution();
      }
      break;
    }
  };
}

function Startup() {
  var target = window.arguments[0];
//  gDialog.nameList = new msiMathNameList();  //see msiEditorUtilities.js
  gDialog.subsList = new dlgAutoSubsList();
  gDialog.bStopNextEnter = false;

  // Now load the autosubs file
  gDialog.subsList.names = msiAutosubstitutionList.createDialogSubstitutionList();
//  if (!createSubstitutionList())
//  {
//    AlertWithTitle("autoSubstituteDialog.js", "createSubstitutionList returned false.");
////    window.close();
////    return;
//  }

  gDialog.bNameOK = false;
  gDialog.bDataNonEmpty = false;
  gDialog.bIsNew = false;
  gDialog.bDataModified = false;
  gDialog.bTypeModified = false;
  gDialog.bContextModified = false;
  gDialog.bEditorReady = false;
  gDialog.bGlobalEnablingChanged = false;

  var substitutionStr = "";
  var currPattern = "";
  if (target != null && ("val" in target) && (target.val.length > 0))
    currPattern = target.val;
  var namesBox = document.getElementById("keystrokesBox");
  namesBox.value = currPattern;
  msiAutosubstitutionList.setUpTextBoxControl(namesBox);
  var theType = "substitution";
  var theContext = "math";
  if ( (currPattern.length > 0) && (currPattern in gDialog.subsList.names) )
  {
    gDialog.bIsNew = false;
    theType = gDialog.subsList.names[currPattern].type;

    if (theType == "substitution")
      substitutionStr = gDialog.subsList.names[currPattern].data;
  }
  else
    gDialog.bIsNew = true;
//  if (theContext == "math")
//  {
//    if (substitutionStr.length == 0)
//      substitutionStr = GetComputeString("Math.emptyForInput");
//    else
//      substitutionStr = "<math>" + substitutionStr + "</math>";
//  }
  document.getElementById("autosubTypeRadioGroup").value = theType;
  document.getElementById("autosubContextRadioGroup").value = theContext;

  var enableText = gDialog.subsList.isAutoSubstitutionEnabled( false );
  document.getElementById("disableSubsInText").checked = !enableText;
  var enableMath = gDialog.subsList.isAutoSubstitutionEnabled( true );
  document.getElementById("disableSubsInMath").checked = !enableMath;
  
  var editElement = document.getElementById("subst-frame");
  var substitutionControlObserver = new msiEditorChangeObserver(editElement);
  var commandBoldObserverData = new Object();
  commandBoldObserverData.mCommand = "cmd_bold";
  commandBoldObserverData.mObserver = substitutionControlObserver;
  var commandSetModifiedObserverData = new Object();
  commandSetModifiedObserverData.mCommand = "cmd_setDocumentModified";
  commandSetModifiedObserverData.mObserver = substitutionControlObserver;
  var editorDocLoadedObserverData = new Object();
  editorDocLoadedObserverData.mCommand = "obs_documentCreated";
  editorDocLoadedObserverData.mObserver = substitutionControlObserver;

  editElement.mInitialDocObserver = [commandBoldObserverData, commandSetModifiedObserverData, editorDocLoadedObserverData];
//  editElement.mbSinglePara = false;

  msiInitializeEditorForElement(editElement, substitutionStr);

//  changePattern(namesBox.value);
//  dump("In autoSubstituteDialog.js, startup is finished.\n");
}

//function createSubstitutionList()
//{
//  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
//  var basedir = dsprops.get("resource:app", Components.interfaces.nsIFile);
//  basedir.append("res");
//  var autosubsFilename = "autosubs.xml";
//  var autosubsFile = basedir;
//  autosubsFile.append("tagdefs");
//  autosubsFile.append(autosubsFilename);                                                               ke
//  var subsDoc = document.implementation.createDocument("", "subs", null);
//  subsDoc.async = false;
//  if (!subsDoc.load("file:///" + autosubsFile.path))
//  {
////    AlertWithTitle("Error in Automatic Substitution Dialog", "Unable to load autosubstitution file \"" + autosubsFile.path + "\"; aborting Auto Substitution dialog.\n");
//    dump("Unable to load autosubstitution file \"" + autosubsFile.path + "\"; aborting Auto Substitution dialog.\n");
//    return false;
//  }
//  
//  var retVal = false;  //until we get something in the list
//  // We need to prebuild these so that the keyboard shortcut works
//  // ACSA = autocomplete string array
//  var ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
//  ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
//  
////  gDialog.subsList = new Object();
//  var rootElementList = subsDoc.getElementsByTagName("subs");
//  dump("In autoSubstituteDialog.js, in createSubstitutionList(), subsDoc loaded, rootElementList has length [" + rootElementList.length + "].\n");
//  var nameNodesList = null;
//  if (rootElementList.length > 0)
//    nameNodesList = rootElementList[0].getElementsByTagName("sub");
//  else
//    nameNodesList = subsDoc.getElementsByTagName("sub");
//  dump("In autoSubstituteDialog.js, in createSubstitutionList(), subsDoc loaded, nameNodesList has length [" + nameNodesList.length + "].\n");
//  if (nameNodesList.length == 0)
//    AlertWithTitle("autoSubstituteDialog.js", "Empty nameNodesList returned.");
//  var thePattern = "";
//  var stringType = "";
//  var newObject = null;
////  var unitRegExp = /insertMathunit\([\'\"]([^\'\"]+)[\'\"]\);/;
////  var mathnameRegExp = /insertMathname\([\'\"]([^\'\"]+)[\'\"]\);/;
//  var nAddedToList = 0;
//  for (var ix = 0; ix < nameNodesList.length; ++ix)
//  {
//    thePattern = nameNodesList[ix].getElementsByTagName("pattern").item(0).textContent;
//    if (thePattern != null && thePattern.length > 0)
//    {
//      newObject = new Object();
//      stringType = nameNodesList[ix].getAttribute("tp");
//      newObject.mathContext = nameNodesList[ix].getAttribute("ctx");
//      newObject.theData = nameNodesList[ix].getElementsByTagName("data").item(0).textContent;
//      if (stringType == "sc")
//      {
//        newObject.theContext = "";
//        newObject.theInfo = "";
//      }
//      else
//      {
//        newObject.theContext = nameNodesList[ix].getElementsByTagName("context").item(0).textContent;
//        newObject.theInfo = nameNodesList[ix].getElementsByTagName("info").item(0).textContent;
//      }
//      if (stringType == "sc")
//      {
////        var unitMatch = newObject.theData.match(unitRegExp);
////        if (unitMatch && unitMatch.length > 1)
////        {
////          newObject.type = "unit";
////          newObject.unitStr = unitMatch[1];
////        }
////        else
////        {
////          var mathnameMatch = newObject.theData.match(mathnameRegExp);
////          if (mathnameMatch && mathnameMatch.length > 1)
////          {
////            newObject.type = "mathName";
////            newObject.mathnameStr = mathnameMatch[1];
////          }
////          else
//        newObject.type = "script";
////        }
//      }
//      else if (stringType == "subst")
//        newObject.type = "substitution";
//      else
//        newObject.type = "";
//
//      if (newObject.type.length > 0)
//      {
//        ACSA.addString("autosub", thePattern);
//        gDialog.subsList.names[thePattern] = newObject;
//        retVal = true;
//        ++nAddedToList;
////        dump("In autoSubstituteDialog.createSubstitutionList, adding pattern [" + thePattern + "] with data [" + newObject.theData + "].\n");
//      }
//    }
//  }
//  ACSA.sortArrays();
//  dump("In autoSubstituteDialog.createSubstitutionList, added " + nAddedToList + " elements to autocomplete array.\n");
//  return retVal;
//}

//enableControls() . If the selected name is already in the list, the label of the AddOrModify button should have been changed to Modify.
//  It will be enabled or not in conjunction with the OK button.
//If not, the Delete button should be disabled.
//We use the dialog variables:
//   gDialog.bNameOK - true if "pattern" field is nonempty; necessary in order to enable almost anything;
//   gDialog.bDataNonEmpty - true if there's something in the data control (either means the substitution editor control or the script text control);
//   gDialog.bIsNew - true if the name (the "pattern") is not currently in the list;
//   gDialog.bDataModified - set when the data control doesn't match the data in the list for the selected pattern;
//   gDialog.bTypeModified - set when the type control doesn't match the type in the list for the selected pattern;
//   gDialog.bContextModified - set when the math context radio selection doesn't match the math context in the list for the selected pattern;
//  Note that the "bCurrentItemModified" variable simply returns (gDialog.bDataModified || gDialog.bTypeModified || gDialog.bContextModified).
function enableControls()
{
  var currName = document.getElementById("keystrokesBox").value;
  var currType = document.getElementById("autosubTypeRadioGroup").value;

  var dumpStr = "For autosubstitution [" + currName + "]; "

//  var bIsNew = (currName != null) && (currName.length > 0) && !(currName in gDialog.subsList.names);
//  var bCanSave = contentsAreNonEmpty(currType);
  var bCurrentItemModified = (gDialog.bDataModified || gDialog.bTypeModified || gDialog.bContextModified);

  var bActionEnabled = gDialog.bDataNonEmpty && gDialog.bNameOK;
  if (!gDialog.bIsNew)
  {
    bActionEnabled = gDialog.bDataNonEmpty && gDialog.bNameOK && bCurrentItemModified;
  }
  if (!bActionEnabled)
    bActionEnabled = gDialog.bGlobalEnablingChanged;

  enableControlsByID(["deleteButton"], (gDialog.bNameOK && !gDialog.bIsNew));

  dumpStr += " canAdd returned [" + gDialog.bIsNew + "]; while gDialog.bDataNonEmpty is [" + gDialog.bDataNonEmpty + "] ";
  dumpStr += " and bCurrentItemModified is [" + bCurrentItemModified + "] and gDialog.bNameOK is [" + gDialog.bNameOK + "].";
  dump(dumpStr + ".\n");

  if (!bActionEnabled && !gDialog.subsList.bModified)
    document.documentElement.getButton("accept").setAttribute("disabled", "true");
  else
    document.documentElement.getButton("accept").removeAttribute("disabled");
}

function getDataFromControl(whichType)
{
  var theValue = null;
  var bMathContext = (document.getElementById("autosubContextRadioGroup").value == "math");
  switch(whichType)
  {
    case "substitution":
    {
      var editElement = document.getElementById("subst-frame");
      if (!editElement.getHTMLEditor(editElement.contentWindow))
        return theValue;
      var docFrag = getEditControlContentNodes(editElement, bMathContext);
      if ( (docFrag != null) && (docFrag.childNodes.length > 0) )
      {
        var serial = new XMLSerializer();
        theValue = "";
        for (var ix = 0; ix < docFrag.childNodes.length; ++ix)
          theValue += serial.serializeToString(docFrag.childNodes[ix]);
      }
    }
    break;
    case "script":
    {
      theValue = document.getElementById("scriptTextbox").value;
    }
    break;
    default:
      return false;
    break;
  }
  return theValue;
}

function changeType(theType)
{
  var currName = document.getElementById("keystrokesBox").value;
  if ((currName != null) && (currName in gDialog.subsList.names))
    gDialog.bTypeModified = (theType != gDialog.subsList.names[currName].type);
  else
    gDialog.bTypeModified = true;
  if (theType == "substitution")
    setControlsForSubstitution();
  else
    setControlsForScript();
//  setControlsToSubstitutionType(theType);
}

function changeContext(theContext)
{
  var currName = document.getElementById("keystrokesBox").value;
  if ((currName != null) && (currName in gDialog.subsList.names))
    gDialog.bContextModified = (theContext != gDialog.subsList.names[currName].mathContext);
  enableControls();
}

function changeGlobalEnabling(bMath)
{
  bDisabled = false;
  if (bMath)
  {
    disabled = document.getElementById("disableSubsInMath").checked;
    if (disabled && gDialog.subsList.isAutoSubstitutionEnabled( true ))
      gDialog.bGlobalEnablingChanged = true;
    else if (!disabled && !gDialog.subsList.isAutoSubstitutionEnabled( true ))
      gDialog.bGlobalEnablingChanged = true;
  }
  else
  {
    disabled = document.getElementById("disableSubsInText").checked;
    if (disabled && gDialog.subsList.isAutoSubstitutionEnabled( false ))
      gDialog.bGlobalEnablingChanged = true;
    else if (!disabled && !gDialog.subsList.isAutoSubstitutionEnabled( false ))
      gDialog.bGlobalEnablingChanged = true;
  }
  enableControls();
}

////Here we hide/show the corresponding substitution/script/mathname/unitname controls.
//function setControlsToSubstitutionType(theType)
//{
//  var typeToShow = "substitution-frame";
//  var labelToShow = document.getElementById("substitutionLabel");
//  switch(theType)
//  {
//    case "substitution":
//    break;
//    case "script":
//      typeToShow = "scriptTextbox";
//      labelToShow = document.getElementById("scriptLabel");
//    break;
//  }
//  var controlIDs = ["substitution-frame", "scriptTextbox"];
//  for (var ix = 0; ix < controlIDs.length; ++ix)
//  {
//    if (controlIDs[ix] == typeToShow)
//      document.getElementById(controlIDs[ix]).display = "normal";
//    else
//      document.getElementById(controlIDs[ix]).display = "none";
//  }
//  var theCaption = document.getElementByID("replacementLabel");
//  theCaption.label = document.getElementByID(labelToShow).value;
//  theCaption.accesskey = document.getElementByID(labelToShow).accesskey;
//
//  //Now try to set the correct data in the displayed control.
//  var autoSubName = document.getElementById("keystrokesBox").value;
//  setSubstitutionControlContents(autoSubName);
//}

//Here we hide/show the corresponding substitution/script/mathname/unitname controls.
function setControlsForSubstitution()
{
  msiEnableEditorControl(document.getElementById("subst-frame"), true);
//  document.getElementById("subst-frame").disabled = false;
  document.getElementById("thedeck").selectedIndex=0;
  
//  var labelToShow = document.getElementById("substitutionLabel");
//  var theCaption = document.getElementById("replacementLabel");
//  theCaption.label = labelToShow.value;
//  theCaption.accesskey = labelToShow.accesskey;
//  theCaption.control = labelToShow.control;

  //Now try to set the correct data in the displayed control.
  var autoSubName = document.getElementById("keystrokesBox").value;
  setSubstitutionControlFromSub(autoSubName);
}

function setControlsForScript()
{
//  document.getElementById("subst-frame").disabled = true;
  msiEnableEditorControl(document.getElementById("subst-frame"), false);
  document.getElementById("thedeck").selectedIndex=1;
//  var labelToShow = document.getElementById("scriptLabel");
//  var theCaption = document.getElementById("replacementLabel");
//  theCaption.label = labelToShow.value;
//  theCaption.accesskey = labelToShow.accesskey;
//  theCaption.control = labelToShow.control;

  //Now try to set the correct data in the displayed control.
  var autoSubName = document.getElementById("keystrokesBox").value;
  setScriptControlFromSub(autoSubName);
}

//function setSubstitutionControlContents(autoSubName)
//{
//  var theType = document.getElementById("autosubTypeRadioGroup").value;
//  switch(theType)
//  {
//    case "substitution":
//      setSubstitutionControlFromSub(autoSubName);
//    break;
//    case "script":
//      setScriptControlFromSub(autoSubName);
//    break;
//  }
//}

function setSubstitutionControlFromSub(autoSubName)
{
  var theEditorElement = document.getElementById("subst-frame");
  var theEditor = null;
  if (theEditorElement.docShell != null)
    theEditor = theEditorElement.getHTMLEditor(theEditorElement.contentWindow);
  if (!gDialog.bEditorReady)
  {
    if (theEditor != null)
      gDialog.bEditorReady = true;
  }
  if (theEditor == null)
  {
    gDialog.bEditorReady = false;
    dump("Calling autoSubstituteDialog.setSubstitutionControlFromSub [" + autoSubName + "]; editor not reported ready yet.\n");
    return;
  }
//  var theEditor = theEditorElement.getHTMLEditor();
  msiDeleteBodyContents(theEditor);
  if (!autoSubName || !(autoSubName in gDialog.subsList.names))
  {
    gDialog.bDataModified = true;
  }
  else
  {
    switch(gDialog.subsList.names[autoSubName].type)
    {
      case "substitution":
      {
//        msiDeleteBodyContents(theEditor);
        var theText = gDialog.subsList.names[autoSubName].theData;
        var contextMarkup = gDialog.subsList.names[autoSubName].theContext;
        if (contextMarkup == null)
          contextMarkup = "";
        if (gDialog.subsList.names[autoSubName].mathContext == "math")
        {
//          if ((theText.indexOf("<math") < 0) && (contextMarkup.indexOf("<math") < 0))
          if (theText.indexOf("<math") < 0)
          {
//            insertinlinemath(theEditorElement);
//            dump("In setsubstitutionControlFromSub(), inserting inline math in editor control.\n");
            theText = "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">" + theText + "</math>";
          }
        }
//        insertXMLAtCursor(theEditor, theText, true, true);
        dump("Calling insertHTMLWithContext to enter text [" + theText + "], with context [" + contextMarkup + "].\n");
//        theEditor.insertHTMLWithContext(theText, contextMarkup, "", "", null, null, 0, true);
        theEditor.insertHTMLWithContext(theText, "", "", "", null, null, 0, true);
      }
      break;
      case "script":
        var currActiveEdElement = msiGetActiveEditorElement();
        msiSetActiveEditor(theEditorElement, false);
        eval(gDialog.subsList.names[autoSubName].theData);
        msiSetActiveEditor(currActiveEdElement, false);
      break;
    }
    gDialog.bDataModified = false;
    theEditor.resetModificationCount();
  }
  enableControls();
}

function setScriptControlFromSub(autoSubName)
{
  var scriptControl = document.getElementById("scriptTextbox");
  if (!autoSubName || !(autoSubName in gDialog.subsList.names))
  {
    scriptControl.value = "";
    gDialog.bDataModified = true;
  }
  else
  {
    switch(gDialog.subsList.names[autoSubName].type)
    {
      case "substitution":
        scriptControl.value = "";
//        ADD FUNCTION TO MACROS.JS TO ALLOW INSERTION OF XML?
//        scriptControl.value = "doInsertXML(" + gDialog.subsList.names[autoSubName].data + ");";
      break;
      case "script":
        scriptControl.value = gDialog.subsList.names[autoSubName].theData;
      break;
    }
    gDialog.bDataModified = false;
  }
  enableControls();
}

//function getMathNameFromScriptString(theString)
//{
//  var mathnameRegExp = /insertMathname\([\'\"]([^\'\"]+)[\'\"]\);/;
//  var mathnameMatch = theString.match(mathnameRegExp);
//  if (mathnameMatch && mathnameMatch.length > 1)
//    return mathnameMatch[1];
//  return "";
//}
//
//function getUnitNameFromScriptString(theString)
//{
//  var unitRegExp = /insertMathunit\([\'\"]([^\'\"]+)[\'\"]\);/;
//  var unitMatch = theString.match(unitRegExp);
//  if (unitMatch && unitMatch.length > 1)
//    return unitMatch[1];
//  return "";
//}

//function checkDataEvent(control, theEvent)
//{
//  var bActionEnabled = nameFieldNonEmpty();
//  var currType = document.getElementById("autosubTypeRadioGroup").value;
//  var theData = getDataFromControl(currType);
//  if (!patternIsNew())
//    bAtionEnabled = bActionEnabled && dataIsNonEmptyAndModified(currType, theData);
//  else
//    bActionEnabled = bActionEnabled && dataIsNonEmpty(currType, theData);
//  enableControlsByID(["addOrModifyButton"], bActionEnabled);
//  if (!bActionEnabled)
//    document.documentElement.getButton("accept").setAttribute("disabled", "true");
//  else
//    document.documentElement.getButton("accept").removeAttribute("disabled");
//}

function checkSubstitutionControl(editControl)
{
  if (!gDialog.bEditorReady)
  {
    dump("Calling autoSubstituteDialog.checkSubstitutionControl; editor not reported ready yet.\n");
    return;
  }
  var editor = null;
  try
  {
    editor = editControl.getHTMLEditor(editControl.contentWindow);
  } catch(exc) {dump("In autoSubstituteDialog.checkSubstitutionControl, getHTMLEditor() returned exception" + exc + "\n"); editor = null;}

  if (!editor)
    return;
  var bModified = editor.documentModified;
  if (gDialog.substContentFilter == null)
    gDialog.substContentFilter = new msiDialogEditorContentFilter(editControl);
  var theContext = document.getElementById("autosubContextRadioGroup").value;
  var bNonEmpty = gDialog.substContentFilter.hasNonEmptyContent( (theContext == "math") );
//  var bNonEmpty = !editor.documentIsEmpty;
  dump("In autoSubstituteDialog.js, checkSubstitutionControl, editor.documentModified returns [" + bModified + "] and editor.documentIsEmpty returns [" + editor.documentIsEmpty + "].\n");
  if (bModified != gDialog.bDataModified || bNonEmpty != gDialog.bDataNonEmpty)
  {
    gDialog.bDataModified = bModified;
    gDialog.bDataNonEmpty = bNonEmpty;
    enableControls();
  }
}

function checkScriptControl(scriptControl)
{
  var currName = document.getElementById("keystrokesBox").value;
  var currData = "";
  var controlData = scriptControl.value;
  if ((currName != null) && (currName in gDialog.subsList.names))
    currData = gDialog.subsList.names[currName].theData;
  var bModified = (!currData.length) || (currData != controlData);
  var bNonEmpty = (controlData != null) && (controlData.length > 0);
  if (bModified != gDialog.bDataModified || bNonEmpty != gDialog.bDataNonEmpty)
  {
    gDialog.bDataModified = bModified;
    gDialog.bDataNonEmpty = bNonEmpty;
    enableControls();
  }
}

function checkKeyPressEvent(control, theEvent)
{
//  var dumpStr = "In checkKeyPressEvent handler, gDialog.bStopNextEnter is [";
//  if (gDialog.bStopNextEnter)
//    dumpStr += "true]; ";
//  else
//    dumpStr += "false]; ";
  if (!theEvent.altKey)
  {
    if (theEvent.keyCode==KeyEvent.DOM_VK_RETURN)
    {
      if (gDialog.bStopNextEnter)
      {
        gDialog.bStopNextEnter = false;
        if (!control)
        {
          dump("Null control in checkKeyPressEvent!\n");
          control = document.getElementById("keystrokesBox");
        }
        control.controller.handleEnter(false);
        theEvent.stopPropagation();
        theEvent.preventDefault();
//        dumpStr += "called theEvent.stopPropagation().\n";
      }
    }
    else //now we're typing into the name field, so we assume we should stop the next enter from accepting the dialog
    {
      gDialog.bStopNextEnter = true;
//      dumpStr += "setting gDialog.bStopNextEnter.\n";
    }
  }
//  dump(dumpStr);
  //Now hopefully continue processing as usual.
}

function changePattern(currPattern)
{
//  var currName = document.getElementById("mathNamesBox").value;
  dump("autoSubstituteDialog.js 2\n");
  var theType = document.getElementById("autosubTypeRadioGroup").value;
  var newType = theType;
  if (currPattern.length > 0)
    gDialog.bNameOK = true;
  else
    gDialog.bNameOK = false;
  var theContext = "math";
  var bWasNew = gDialog.bIsNew;
  if (gDialog.bNameOK && (currPattern in gDialog.subsList.names))
  {
    gDialog.bIsNew = false;
    newType = gDialog.subsList.names[currPattern].type;
    theContext = gDialog.subsList.names[currPattern].mathContext;
  }
  else
    gDialog.bIsNew = true;
  if (theType == null || !theType.length)
    theType = "substitution";
  if (gDialog.bIsNew && !bWasNew)
    newType = "substitution";
  dump("In autoSubstituteDialog.js, changePattern(); new key pattern is [" + currPattern + "], gDialog.bIsNew is [" + gDialog.bIsNew + "], sub type is [" + newType + "], and context is [" + theContext + "].\n");
  dump("autoSubstituteDialog.js 3\n");
  document.getElementById("autosubTypeRadioGroup").value = newType;
  document.getElementById("autosubContextRadioGroup").value = theContext;

  if (theType != newType)
  {
    if (newType == "substitution")
      setControlsForSubstitution(currPattern);
    else
      setControlsForScript(currPattern);
  }
  else
  {
    if (newType == "substitution")
      setSubstitutionControlFromSub(currPattern);
    else
      setScriptControlFromSub(currPattern);
  }
  dump("autoSubstituteDialog.js 4\n");
}

//This function will add the current name to the listbox, and to the local gDialog.subsList.
//Writing to the XML file, and updating the prototype mathNameList, occurs onOK?? Or is this wrong?
function saveCurrentSub()
{
  dump("autoSubstituteDialog.js 0\n");
  var currSub = document.getElementById("keystrokesBox").value;
  var theType = document.getElementById("autosubTypeRadioGroup").value;
  var appearanceListFrag = null;
  var theData = null;
  var theContext = document.getElementById("autosubContextRadioGroup").value;
  if (theType == "substitution")
    appearanceListFrag = getEditControlContentNodes(document.getElementById("subst-frame"), (theContext == "math"));
  else if (theType == "script")
    theData = document.getElementById("scriptTextbox").value;
//  var bEngFuncStr = bEngineFunction ? "true" : "false";
//  var bAutoSubStr = bAutoSubstitute ? "true" : "false";
//  alert("Calling to add name: [" + currName + "], of type [" + theType + "], with bEngineFunction [" + bEngFuncStr + "] and bAutoSubstitute [" + bAutoSubStr + "].\n");
  var contextMarkupStr = "";  //this gets expanded in addSub to <math xmlns=...> if "theContext" is "math".
//  if (gDialog.bIsNew)
  var appearanceList = null;
  if ( (appearanceListFrag != null) && (appearanceListFrag.childNodes.length > 0) )
    appearanceList = appearanceListFrag.childNodes;
  gDialog.subsList.saveSub(currSub, theType, theContext, theData, contextMarkupStr, appearanceList);
//  else
//    gDialog.subsList.modifySub(currSub, theType, theContext, theData, contextMarkupStr, appearanceList);

  dump("autoSubstituteDialog.js 1\n");
  changePattern(currSub);
}

function removeCurrentSub()
{
  var currSub = document.getElementById("keystrokesBox").value;
  gDialog.subsList.removeSub(currSub);
//  gDialog.subsList.updateBaseList();

//  setScriptControlFromSub(currSub);  //this should remove all text in the data controls
  enableControls();
}

function getEditControlContentNodes(editorElement, bMathOnly)
{
  if (gDialog.substContentFilter == null)
    gDialog.substContentFilter = new msiDialogEditorContentFilter(editorElement);
  gDialog.substContentFilter.setMathOnly(bMathOnly);
  var docFrag = gDialog.substContentFilter.getXMLNodesAsDocFragment();
  return docFrag;
//  var doc = editorElement.contentDocument;
////  var target = msiGetRealBodyElement(doc);
//  var target = doc.getElementsByTagName("dialogbase")[0];
//  var returnlist;
//  if (!target) target = doc.getElementsByTagName("para")[0];
//  if (!target)
//    return null;
//  if (bMathOnly)
//  {
//    var mathList = target.getElementsByTagName("math");
//    if (mathList != null && mathList.length > 0)
//      target = mathList[0];
//    else
//      target = null;
//  }
//  if (target != null)
//  {
//    returnlist = target.childNodes;
//    return returnlist;
//  }
//  else
//  
//    return null;
}


function dumpln(s)
{
  dump(s+"\n");
}

function onOK() {
  dumpln("a");
  var bActionEnabled = gDialog.bDataNonEmpty && gDialog.bNameOK;
  if (!gDialog.bIsNew)
    bActionEnabled = gDialog.bDataNonEmpty && gDialog.bNameOK && gDialog.bCurrentItemModified;
  if (bActionEnabled)
    saveCurrentSub();
  try
  {
    gDialog.subsList.saveToFile();
    var disableFlags = 0;
    var enableFlags = 0;
    var disableMath = document.getElementById("disableSubsInMath").checked;
    if (disableMath && gDialog.subsList.isAutoSubstitutionEnabled( true ))
      disableFlags = Components.interfaces.msiIAutosub.CONTEXT_MATHONLY;
    else if ((!disableMath) && !gDialog.subsList.isAutoSubstitutionEnabled( true ))
      enableFlags = Components.interfaces.msiIAutosub.CONTEXT_MATHONLY;
    var disableText = document.getElementById("disableSubsInText").checked;
    if (disableText && gDialog.subsList.isAutoSubstitutionEnabled( false ))
      disableFlags |= Components.interfaces.msiIAutosub.CONTEXT_TEXTONLY;
    else if ((!disableText) && !gDialog.subsList.isAutoSubstitutionEnabled( false ))
      enableFlags |= Components.interfaces.msiIAutosub.CONTEXT_TEXTONLY;
    if (disableFlags != 0)
      gDialog.subsList.enableAutoSubstitution(false, disableFlags);
    if (enableFlags != 0)
      gDialog.subsList.enableAutoSubstitution(true, enableFlags);
  } catch(ex) {dump("Exception in autoSubstituteDialog.js OnOK(); exception is [" + ex + "].\n");}

  SaveWindowLocation();
  return true;
}

function onCancel() {
  return(true);
}

//function ShutdownEditors()
//{
//  ShutdownAllEditors();
//}