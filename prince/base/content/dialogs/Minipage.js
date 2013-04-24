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
  try {
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
    gd = initFrameTab(gd, msiframe, isNewNode, null);
  // we don't want heavy-weight frames inline
    document.getElementById('inline').hidden=true;
    // no natural size ==> autowidth not possible
    gd.autoWidthCheck.setAttribute("style", "visibility:hidden;");
    gd.autoWidthLabel.setAttribute("style", "visibility:hidden;");
  //  initFrameSizePanel(); // needed when the user can set the size
  }
  catch(e) {
    throw(e);
  }
}


function onOK() {
  if (!isValid()) return false; // isValid is in msiFrameOverlay.j s 
  editor.beginTransaction();
	if (isNewNode) 
	{
		try 
		{
			if (editor.selection.isCollapsed)
			{
				msiframe = editor.document.createElement("msiframe");
				var para = editor.document.createElement("bodyText");
				msiframe.appendChild(para);
				var br = editor.document.createElement("br");
				para.appendChild(br);
				editor.insertElementAtSelection(msiframe, true);
			}
			else
			{
				editor.setStructureTag("msiframe");
				msiframe = getSelectionParentByTag(editor, "msiframe");
			}
		}
		catch(e)
		{
			dump(e.message+"\n");
		}
	}
  setFrameAttributes(msiframe, msiframe, editor);
	editor.endTransaction();
//	if (isNewNode)
//	{
//		editor.selection.collapse(para,0);
//	}
  return(true);
}

function onCancel() {
  close();
  return(true);
}


