if (!eu) var eu = {};
if (!eu.sevendiamonds) eu.sevendiamonds = {};
if (!eu.sevendiamonds.askDictionary) eu.sevendiamonds.askDictionary = {};

eu.sevendiamonds.askDictionary = {
    /**
	 * Load the extension, register event listener etc.
	 */
	load: function() {           
		//Components.utils.reportError(document.getElementById('contentAreaContextMenu'));
		document.getElementById('contentAreaContextMenu').addEventListener('focus',
	            function(event) {
					Components.utils.reportError("contentAreaContextMenu focus");
	                Components.utils.reportError(event);
	            }, false);
	},

	ask: function() {
		var selectionString = getBrowserSelection();
		window.location.href = "dict:///" + selectionString;
	},
	
	checkSelection: function() {
		var selectionString = getBrowserSelection();
		if (selectionString != null && selectionString.trim().length > 0) {
			console("has selection" + selectionString);
		} else {
			console("no selection");
		}
	},
}