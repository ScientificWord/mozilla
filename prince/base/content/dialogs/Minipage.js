var frmElement = {};
var newElement = true;
var gd;
var gDialog;
var editor;
var msiframe;
var isNewNode;
var gFrameModeTextFrame = true ;
var gFrameModeImage = false ;

var msiframe;
var scale= .25; /*scale of reduced diagram*/
var editor;

function startUp()
{
  var widthregexp = /width\s*\:\s*([0-9]+)([a-z]*)\s*;/;
  var heightregexp = /height\s*\:\s*([0-9]+)([a-z]*)\s*;/;
  var match;
  var width, height;
  var style;
  try {
    var editorElement = msiGetParentEditorElementForDialog(window);
    editor = msiGetEditor(editorElement);
    if (!editor)
    {
      window.close();
      return;
    }
    msiframe = null;
    var i = 0;
    while (i < window.arguments.length) {
      msiframe = window.arguments[i];
      if (msiframe !== null  && msiframe.nodeName === 'msiframe') break;
      i ++ ;
    }
    isNewNode = !(msiframe) || msiframe.nodeName !== 'msiframe';
    if (isNewNode) {
      msiframe = editor.createFrameWithDefaults('textframe', false, null, 0);
      editor.getFrameStyleFromAttributes(msiframe);
    } 
    gDialog = {};

    gd = initFrameTab(gDialog, msiframe, false, null);
    gSizeState.selectCustomSize();
    gSizeState.setPreserveAspectRatio(false);
    document.getElementById('sizeRadio').hidden = true;
  // we don't want heavy-weight frames inline
    document.getElementById('inline').hidden=true;
  //  initFrameSizePanel(); // needed when the user can set the size
  }
  catch(e) {
    throw(e);
  }
}


function onOK() {
  if (!isValid()) return false; // isValid is in msiFrameOverlay.js
  editor.beginTransaction();
	if (isNewNode) 
	{
		try 
		{
			if (editor.selection.isCollapsed)
			{
        msiframe = editor.tagListManager.getNewInstanceOfNode( "msiframe", null, editor.document);
				editor.insertElementAtSelection(msiframe, true);
        editor.createDefaultParagraph(msiframe, 0, true);
        editor.setCursorInNewHTML(msiframe);
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
  setFrameAttributes(msiframe, msiframe, editor, 0);
  var width = document.getElementById("frameWidthInput").value;
  var height = document.getElementById("frameHeightInput").value;
  var units = msiframe.getAttribute('units');

  msiframe.setAttribute('frametype', 'textframe');
  msiframe.setAttribute('width',width);
  msiframe.setAttribute('height', height);

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


