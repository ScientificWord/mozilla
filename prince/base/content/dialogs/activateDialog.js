#include ../productname.inc
/// This comment fixes a syntax highlighting glitch on my laptop -- BBM

var req;
// var server='licensing.mackichan.com';
var server='licensing.mackichan.com';



function startUp() {
	var editorElement = msiGetActiveEditorElement();
	var editor = msiGetEditor(editorElement);
	var hostid = editor.mAppUtils.hostid;
	document.getElementById('computerid').value = hostid;
}

function writeStringAsFile( str, file )
{
  var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
  fos.init(file, -1, -1, false);
  var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
    .createInstance(Components.interfaces.nsIConverterOutputStream);
  os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
  os.writeString(str);
  os.close();
  fos.close();
}

function writeLicense(licenseText)
{
	var match;
	var dsprops = Components.classes['@mozilla.org/file/directory_service;1'].getService(Components.interfaces.nsIProperties);
  var licenseFile = dsprops.get('ProfD', Components.interfaces.nsILocalFile);
  if (!licenseFile) return false;
  licenseFile.append("license.lic");
  if (licenseFile.exists())
  	licenseFile.remove(false);
  writeStringAsFile(licenseText, licenseFile);
  return true;
}

function accept() {
	var acceptMessage = "Your license has been installed. Thank you for supporting ";
	acceptMessage = acceptMessage +
#ifdef PROD_SW
	"Scientific Word.";
#endif
#ifdef PROD_SNB
	"Scientific Notebook.";
#endif
#ifdef PROD_SWP
	"Scientific WorkPlace.";
#endif


	var transferComplete = function() {
		if (this.responseText.length === 0 || !(/LICENSE mackichn[ a-z]+6\.0/.test(this.responseText)))
		{
			if (/your site license administrator/.test(this.responseText)) {
				match = /Your\slicense.*the site\./.exec(this.responseText);
				if (match && match.length >= 1) {
				  AlertWithTitle("Home Use License", match[0]);
				  return;
				}

			}
			AlertWithTitle("Activation Failure","Failed to get license: "+this.responseText);
		}
		else {
			writeLicense(this.responseText);
			AlertWithTitle("Activation Success", acceptMessage);
			editor.mAppUtils.reset();
		}
	};

	var transferFailed = function() {
		AlertWithTitle("Activation Failure","Unable to connect to licensing server");
	};

	var transferCanceled = function() {
		 AlertWithTitle("Activation Failure","Getting a license was cancelled");
	};
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
 		sPhone: document.getElementById('phone').value,
 		sFirstName: document.getElementById('name').value,
 		sFamilyName: ""
 	};
 	var querystring = ['sSerial=' + query.sSerial,
 		'sProductName=' + query.sProductName,
 		'sComputerID=' + query.sComputerID,
 		'sVersion=' + query.sVersion,
 		'sEMail=' + query.sEMail,
 		'sPhone=' + query.sPhone,
 		'sFirstName=' + query.sFirstName,
 		'sFamilyName=' + query.sFamilyName
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
	return true;
}


