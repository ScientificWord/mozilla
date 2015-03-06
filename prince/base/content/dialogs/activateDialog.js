#include ../productname.inc

var req;
// var server='licensing.mackichan.com';
var server='192.168.17.200:8090';



function startUp() {
	var editorElement = msiGetActiveEditorElement();
	var editor = msiGetEditor(editorElement);
	var hostid = editor.mAppUtils.hostid;
	document.getElementById('computerid').value = hostid;
}

function accept() {
	var transferComplete = function() {
		alert(this.responseText);
	};

	var transferFailed = function() {
		alert("Failed to get license");
	};

	var transferCanceled = function() {
		 alert("Getting a license was cancelled");
	};
	debugger;
	var editorElement = msiGetActiveEditorElement();
	var editor = msiGetEditor(editorElement);
	var hostid = editor.mAppUtils.hostid;
	var url = 'http://' + server + '/licensing.net/licensedispenser.asmx/getlicense';
	var query = { sSerial: document.getElementById('serial').value,
		sProductName:
#ifdef PROD_SW
 "Scientific Word",
#endif
#ifdef PROD_SNB
 "Scientific Notebook",
#endif
#ifdef PROD_SWP
 "Scientific WorkPlace",
#endif
 		sComputerID: hostid,
 		sVersion: "6.0",
 		sEmail: document.getElementById('email').value,
 		sPhone: document.getElementById('phone').value
 	};
 	var querystring = ['sSerial=' + query.sSerial,
 		'sProductName=' + query.sProductName,
 		'sComputerID=' + query.sComputerID,
 		'sVersion=' + query.sVersion,
 		'sEmail=' + query.sEmail,
 		'sPhone=' + query.sPhone
 		].join('\&');
	try {
		const {XMLHttpRequest} = Components.classes["@mozilla.org/appshell/appShellService;1"]
	                                     .getService(Components.interfaces.nsIAppShellService)
	                                     .hiddenDOMWindow;
		req = new XMLHttpRequest();
		req.addEventListener("load", transferComplete, true);
		req.addEventListener("error", transferFailed, true);
		req.open('GET', url + '?' + querystring, false);
		req.send(querystring);
	}
	catch(e) {
		dump( e.message);
	}
	return 0;
}


