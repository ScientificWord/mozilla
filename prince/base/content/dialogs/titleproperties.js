var shortFormEditor;
var node;
var shortNameNode;

function startup()
{
  node = window.arguments[0];
  var tagName = "title";
  var subNodes = node.getElementsByTagName("shortTitle");
  if (subNodes.length > 0) shortNameNode = subNodes[1];

  document.title = "Title";
  var shortFormStr;
  shortFormEditor = document.getElementById("sectionShortEdit");
  try {
    if (shortNameNode)
    {
      var serializer = new XMLSerializer();
      var shortKids = msiNavigationUtils.getSignificantContents(shortNameNode);
      for (var jx = 0; jx < shortKids.length; ++jx)
        shortFormStr += serializer.serializeToString(shortKids[jx]);
    }
    var editorInitializer = new msiEditorArrayInitializer();
    editorInitializer.addEditorInfo(shortFormEditor, shortFormStr, true);
    editorInitializer.doInitialize();

  }
  catch(e) {
    dump("Error in titleproperties.js, startup: "+e.message+"\n");
  }    
}


function onCancel() {
  return true;   
}

function onAccept()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var parentEditor = msiGetEditor(editorElement);
  var shortFormContentFilter;
  var newShortForm;
  try
  {
    shortFormContentFilter = new msiDialogEditorContentFilter(shortFormEditor);
    newShortForm = shortFormContentFilter.getDocumentFragmentString();
  
    var theWindow = window.opener;
    if (!theWindow)
      theWindow = msiGetTopLevelWindow();
    if (parentEditor && theWindow)
    {
      if (newShortForm && newShortForm.length > 0)
      {
        if (!shortNameNode)
        {
          shortNameNode = parentEditor.document.createElementNS(xhtmlns, "shortTitle");
          parentEditor.insertNode(shortNameNode, node, 0);  //Short form always goes at the start
        }
        for (var ix = shortNameNode.childNodes.length; ix > 0 ; --ix)
          parentEditor.deleteNode(shortNameNode.childNodes[ix-1]);
        parentEditor.insertHTMLWithContext(newShortForm, "", "", "", null, shortNameNode, 0, false);
      }
      else
      {
        if (shortNameNode)
          parentEditor.deleteNode(shortNameNode);
      }
    }
  }
  catch(exc) {
    dump("In titleproperties dialog onAccept(), exception: [" + exc + "].\n");
  }
  return true;    
}
