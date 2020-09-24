#include ../productname.inc
/// This comment fixes a syntax highlighting glitch on my laptop -- BBM

var req;
var server='licensing.mackichan.com';



function startUp() {
	var editorElement = msiGetActiveEditorElement();
	var editor = msiGetEditor(editorElement);
	var hostid = editor.mAppUtils.hostid;
	document.getElementById('computerid').value = hostid;
}

function accept() {
	var acceptMessage = document.getElementById('thankyou').getAttribute('value');
	var prodnum = 0;
	acceptMessage = acceptMessage + document.getElementById('productname').getAttribute('value') + '.';



	var transferComplete = function() {
		if (this.responseText.length === 0 || !(/LICENSE mackichn[ a-z]+6\.0/.test(this.responseText)))
		{
			if (/your site license administrator/.test(this.responseText)) {
				match = /Your\slicense.*the site\./.exec(this.responseText);
				if (match && match.length >= 1) {
				  AlertWithTitle(document.getElementById('homeuse').getAttribute('value'), match[0]);
				  return;
				}
			}
			AlertWithTitle(document.getElementById('failureheader').getAttribute('value'),document.getElementById('failurelicense').getAttribute('value')+this.responseText);
		}
		else {
			writeLicense(this.responseText);
			AlertWithTitle(document.getElementById('successtitle').getAttribute('value'), acceptMessage);
			editor.mAppUtils.reset();
			editor.mAppUtils.licensedApp(prodnum);
		}
	};

	var transferFailed = function() {
		AlertWithTitle(document.getElementById('failureheader').getAttribute('value'),document.getElementById('failureserver').getAttribute('value'));
	};

	var transferCanceled = function() {
		 AlertWithTitle(document.getElementById('failureheader'),document.getElementById('failurecancelled').getAttribute('value'));
	};
	var editorElement = msiGetActiveEditorElement();
	var editor = msiGetEditor(editorElement);
	var hostid = editor.mAppUtils.hostid;
	var url = 'http://' + server + '/licensing.net/licensedispenser.asmx/getlicense';
	var query = { sSerial: document.getElementById('serial').value,
		sProductName: document.getElementById('productname').value,
 		sComputerID: hostid,
 		sVersion: "6.0",
 		sEmail: document.getElementById('email').value,
 		sPhone: document.getElementById('phone').value,
 		sFirstName: document.getElementById('name').value,
 		sFamilyName: "",
 		sSubVersion: "6.0.10"
 	};
 	var querystring = ['sSerial=' + query.sSerial,
 		'sProductName=' + query.sProductName,
 		'sComputerID=' + query.sComputerID,
 		'sVersion=' + query.sVersion,
 		'sEMail=' + query.sEmail,
 		'sPhone=' + query.sPhone,
 		'sFirstName=' + query.sFirstName,
 		'sFamilyName=' + query.sFamilyName,
 		'sSubVersion=' + query.sSubVersion
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
		finalT;
	}
	return true;
}


