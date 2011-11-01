var frmElement = {};
var newElement = true;
var gd;
var editor;
var msiframe;
var isNewNode;
var gFrameModeImage = true;

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
  msiframe = null;
  if (window.arguments.length > 0) msiframe = window.arguments[0];
  isNewNode = !(msiframe);
  gd = new Object();
  setHasNaturalSize(false);
  gd = initFrameTab(gd, msiframe, isNewNode);
// we don't want heavy-weight frames inline
  document.getElementById('inline').hidden = true;
//  initFrameSizePanel(); // needed when the user can set the size
}


function onOK() {
  editor.beginTransaction();
	if (isNewNode) 
	{
		try 
		{
			msiframe = editor.document.createElement("msiframe");
			var para = editor.document.createElement("bodyText");
			msiframe.appendChild(para);
			var br = editor.document.createElement("br");
			para.appendChild(br);
			editor.insertElementAtSelection(msiframe, true);
		}
		catch(e)
		{
			dump(e.message+"\n");
		}
	}
  setFrameAttributes(msiframe);
	editor.endTransaction();
	if (isNewNode)
	{
		editor.selection.collapse(para,0);
	}
  return(true);
}

function onCancel() {
  close();
  return(true);
}


