var thmData = new Object();
//var structNode;

function capitalize( str )
{
  return str.slice(0,1).toLocaleUpperCase()+str.slice(1);
}



function startup()
{
//  thmData = window.arguments[0];
  thmData = window.arguments[0];
  if (!thmData.envNode)
    return;

  thmData.leadInType = "auto";
  var tagName = msiGetBaseNodeName(thmData.envNode);
//  var firstPara = thmData.envNode.firstChild;
//  while (firstPara && firstPara.nodeType != Node.ELEMENT_NODE)
//    firstPara = firstPara.nextSibling;
//  thmData.firstParaNode = firstPara;  //save for later
//  dump("envproperties startup()\n");

  document.title = capitalize(document.title.replace("##", tagName));
  var fixLabels = ["numberAllRadio", "unnumberAllRadio", "unnumberThisRadio", "styleLabel"];
  var fixControl;
  for (var ii = 0; ii < fixLabels.length; ++ii)
  {
    fixControl = document.getElementById(fixLabels[ii]);
    fixControl.label = capitalize(fixControl.label.replace("##", tagName));
  }
  var leadInEditor = document.getElementById("customLeadInEdit");
  try {
//  var editorElement = msiGetParentEditorElementForDialog(window);  
    var children;
    // save the starting state
    if (thmData.envNode)
      children = msiNavigationUtils.getSignificantContents(thmData.envNode);
//    for (var ix = 0; !thmData.customLeadInNode && (ix < children.length); ++ix)
//    {
    if (msiGetBaseNodeName(children[0]) == "envLeadIn")   //NOTE - if the lead-in isn't the first significant child, we've got trouble anyway!
      thmData.customLeadInNode = children[0];
//    }
    thmData.customLeadInStr = "";
    if (thmData.customLeadInNode)
    {
      thmData.leadInType = "custom";
      var serializer = new XMLSerializer();
      var leadInKids = msiNavigationUtils.getSignificantContents(thmData.customLeadInNode);
      for (var jx = 0; jx < leadInKids.length; ++jx)
        thmData.customLeadInStr += serializer.serializeToString(leadInKids[jx]);
    }
    var editorInitializer = new msiEditorArrayInitializer();
    editorInitializer.addEditorInfo(leadInEditor, thmData.customLeadInStr, true);
    editorInitializer.doInitialize();

//    if (thmData.envNode.hasAttribute("nonum"))
//      document.getElementById("numberedCheckbox").checked = false;
  }
  catch(e) {
    dump("Error in thmproperties.js, startup: "+e.message+"\n");
  }
  
  thmData.numbering = tagName;
  var numAttrStr = thmData.envNode.getAttribute("numbering");
  if (numAttrStr)
    thmData.numbering = numAttrStr;
  if (!("defaultNumbering" in thmData))
    thmData.defaultNumbering = numAttrStr;
  if (thmData.defaultNumbering != "none")
  {
    if (thmData.numbering == "none")
      document.getElementById("numberingRadioGroup").value = "unnumberThis";
    else
      document.getElementById("numberingRadioGroup").value = "numberAll";
  }
  else
    document.getElementById("numberingRadioGroup").value = "unnumberAll";

  var styleAttrStr = "plain";
  if ( ("theoremstyle" in thmData) && (thmData.theoremstyle.length) )
    styleAttrStr = thmData.theoremstyle;
  if (styleAttrStr && styleAttrStr.length)
    document.getElementById("styleRadioGroup").value = styleAttrStr;

  document.getElementById("leadInRadioGroup").value = thmData.leadInType;
  var bEnableCustom = (thmData.leadInType == "custom");
  msiEnableEditorControl(leadInEditor, bEnableCustom);
  document.getElementById("customLeadInLabel").disabled = !bEnableCustom;
}

function updateControls()
{
  var leadInType = document.getElementById("leadInRadioGroup").value;
  var bEnableCustom = (leadInType == "custom");
  var leadInEditor = document.getElementById("customLeadInEdit");
  msiEnableEditorControl(leadInEditor, bEnableCustom);
  document.getElementById("customLeadInLabel").disabled = !bEnableCustom;
  document.getElementById("unnumberThisRadio").disabled = (document.getElementById("numberingRadioGroup").value == "unnumberAll");
}

function onCancel() {
  return true;   
}

function onAccept()
{
  var reviseData = new Object();
  reviseData.leadInType = document.getElementById("leadInRadioGroup").value;

  var tagName = msiGetBaseNodeName(thmData.envNode);
  var theVal = document.getElementById("numberingRadioGroup").value;
  var defNumbering = thmData.defaultNumbering;
  if (defNumbering == "none")
  {
    if ("defaultTheoremEnvNumbering" in thmData)
      defNumbering = thmData.defaultTheoremEnvNumbering;
    if (!defNumbering.length || (defNumbering == "self"))
      defNumbering = tagName;
  }
  if (theVal == "numberAll")
    reviseData.numbering = reviseData.defaultNumbering = defNumbering;
  else
  {
    reviseData.numbering = "none";  //reflects that we don't allow "number only this one"
    reviseData.defaultNumbering = (theVal == "unnumberAll") ? "none" : defNumbering;
  }
  reviseData.theoremstyle = document.getElementById("styleRadioGroup").value;

//  var firstPara = thmData.firstParaNode;
//  while (firstPara && firstPara.nodeType != Node.ELEMENT_NODE)
//    firstPara = firstPara.nextSibling;
  var leadInEditor = document.getElementById("customLeadInEdit");
  var editorElement = msiGetParentEditorElementForDialog(window);
//  msiDumpWithID("In structure properties dialog, parent editorElement's id is [@].\n", editorElement);
  var parentEditor = msiGetEditor(editorElement);
  try
  {
    reviseData.customLeadInStr = "";
    if (reviseData.leadInType == "custom")
    { 
      var leadInContentFilter = new msiDialogEditorContentFilter(leadInEditor);
      reviseData.customLeadInStr = leadInContentFilter.getDocumentFragmentString();
//      dump("In env properties dialog, custom lead-in is [" + reviseData.customLeadIn + "].\n");
    }
  
    var theWindow = window.opener;
    if (!theWindow || !("doReviseTheoremNode" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (parentEditor && theWindow)
      theWindow.doReviseTheoremNode(parentEditor, thmData, reviseData);
  }
  catch(exc) {dump("In thmproperties dialog onAccept(), exception: [" + exc + "].\n");}
  return true;    
}
