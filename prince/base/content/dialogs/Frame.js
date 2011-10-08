var frmElement = {};
var newElement = true;
var gd;
var editor;
var msiframe;
var isNewNode;
var gFrameModeImage = false;

Components.utils.import("resource://app/modules/unitHandler.jsm");

var msiframe;
var scale= .25; /*scale of reduced diagram*/
var editor;

function startUp()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  editor = msiGetEditor(editorElement);
  if (!editor)
  {
    window.close();
    return;
  }
  msiframe = getSelectionParentByTag(editor, "msiframe");
  isNewNode = !(msiframe);
//  if (isNewNode)
//  {
//    msiframe = editor.createElementWithDefaults("msiframe");
//    var para = editor.createNode("para", msiframe, 0);
//    var br = editor.createNode("br", para, 0);
//  }   
  gd = new Object();
  setHasNaturalSize(false);
  gd = initFrameTab(gd, msiframe, isNewNode);
//  initFrameSizePanel(); // needed when the user can set the size
}


function onOK() {
  editor.beginTransaction();
	if (isNewNode) 
	{
		try 
		{
			msiframe = editor.createElementWithDefaults("msiframe");
			editor.insertElementAtSelection(msiframe, true);
			var para = editor.createNode("bodyText", msiframe, 0);
			editor.createNode("br", para, 0);		
		}
		catch(e)
		{
			dump(e.message+"\n");
		}
	}
  setFrameAttributes(msiframe);
//  var namespace = new Object();                      
	editor.endTransaction();
  return(true);
}

function onCancel() {
  close();
  return(true);
}


