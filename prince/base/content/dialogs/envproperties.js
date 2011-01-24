var envData = new Object();
//var structNode;

function capitalize( str )
{
  return str.slice(0,1).toLocaleUpperCase()+str.slice(1);
}



function startup()
{
//  envData = window.arguments[0];
  envData.envNode = window.arguments[0];
  if (!envData.envNode)
    return;

  envData.leadInType = "auto";
  var tagName = msiGetBaseNodeName(envData.envNode);
//  var firstPara = envData.envNode.firstChild;
//  while (firstPara && firstPara.nodeType != Node.ELEMENT_NODE)
//    firstPara = firstPara.nextSibling;
//  envData.firstParaNode = firstPara;  //save for later
//  dump("envproperties startup()\n");

  document.title = capitalize(document.title.replace("##", tagName));
  var leadInEditor = document.getElementById("customLeadInEdit");
  try {
//  var editorElement = msiGetParentEditorElementForDialog(window);  
    var children;
    // save the starting state
    if (envData.envNode)
      children = msiNavigationUtils.getSignificantContents(envData.envNode);
//    for (var ix = 0; !envData.customLeadInNode && (ix < children.length); ++ix)
//    {
    if (msiGetBaseNodeName(children[0]) == "envLeadIn")   //NOTE - if the lead-in isn't the first significant child, we've got trouble anyway!
      envData.customLeadInNode = children[0];
//    }
    envData.customLeadInStr = "";
    if (envData.customLeadInNode)
    {
      envData.leadInType = "custom";
      var serializer = new XMLSerializer();
      var leadInKids = msiNavigationUtils.getSignificantContents(envData.customLeadInNode);
      for (var jx = 0; jx < leadInKids.length; ++jx)
        envData.customLeadInStr += serializer.serializeToString(leadInKids[jx]);
    }
    var editorInitializer = new msiEditorArrayInitializer();
    editorInitializer.addEditorInfo(leadInEditor, envData.customLeadInStr, true);
    editorInitializer.doInitialize();

//    if (envData.envNode.hasAttribute("nonum"))
//      document.getElementById("numberedCheckbox").checked = false;
  }
  catch(e) {
    dump("Error in envproperties.js, startup: "+e.message+"\n");
  }
  
  envData.bUnnumbered = false;
  var numAttrStr = envData.envNode.getAttribute("numbering");
  if (numAttrStr && numAttrStr == "none")
    envData.bUnnumbered = true;

  document.getElementById("leadInRadioGroup").value = envData.leadInType;
  var bEnableCustom = (envData.leadInType == "custom");
  msiEnableEditorControl(leadInEditor, bEnableCustom);
  document.getElementById("customLeadInLabel").disabled = !bEnableCustom;
  document.getElementById("numberingCheckbox").checked = !envData.bUnnumbered;
}

function updateControls()
{
  var leadInType = document.getElementById("leadInRadioGroup").value;
  var bEnableCustom = (leadInType == "custom");
  var leadInEditor = document.getElementById("customLeadInEdit");
  msiEnableEditorControl(leadInEditor, bEnableCustom);
  document.getElementById("customLeadInLabel").disabled = !bEnableCustom;
}

function onCancel() {
  return true;   
}

function onAccept()
{
  var reviseData = new Object();
  reviseData.leadInType = document.getElementById("leadInRadioGroup").value;
  reviseData.bUnnumbered = !(document.getElementById("numberingCheckbox").checked);

//  var firstPara = envData.firstParaNode;
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
    if (!theWindow || !("doReviseEnvironmentNode" in theWindow))
      theWindow = msiGetTopLevelWindow();
    if (parentEditor && theWindow)
      theWindow.doReviseEnvironmentNode(parentEditor, envData, reviseData);
  }
  catch(exc) {dump("In envproperties dialog onAccept(), exception: [" + exc + "].\n");}
  return true;    
}
