

var theStringSource = "starting string";

function onLoad() {
	try {
	  var editorElement = document.getElementById("testEdit");
	  var editorInitializer = new msiEditorArrayInitializer();
	  editorElement.mbSinglePara = true;
      // editorElement.mInitialContentListener = invisibleMathOpFilter;  //in plotDlgUtils.js

	  editorInitializer.addEditorInfo(editorElement, theStringSource, true, true);
	  editorInitializer.doInitialize();
		msiSetInitialDialogFocus(editorElement);
		msiSetActiveEditor(editorElement, false);
		msiGetEditor(editorElement).SetTopXULWindow(window);
		msiGetEditor(editorElement).selectAll();
	}
	catch(e) {
		dump('Error: ' + e.message);
	}
  
}