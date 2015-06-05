
function accept ()
{
	return true;
}

function getActivationNumber()
{
	var path;
	var myXMLHTTPRequest;
	var resultString;
	var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
	var licenseFile = dsprops.get('ProfD', Components.interfaces.nsILocalFile);
	var regexp = /\d{3}-\D\d{4}(-\d{5}){3}/;
	licenseFile.append('license.lic');
	if (!licenseFile.exists()) {
		licenseFile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
		licenseFile.append('license.lic');
		if (!licenseFile.exists*()) return "";
	}
	path = msiFileURLFromFile(licenseFile);
	myXMLHTTPRequest = new XMLHttpRequest();
	myXMLHTTPRequest.open('GET', path.spec, false);
	myXMLHTTPRequest.send(null);
	resultString = myXMLHTTPRequest.responseText;
	if (regexp.test(resultString)) {
		return (regexp.exec(resultString)[0]);
	}
	return "";
}

function onLoad()
{
	var bLicensed = isLicensed();
	var daysleft = licenseTimeRemaining();
	var permanent = (daysleft === "permanent");
	var expstr = "This license ";
	var prodname = document.getElementById('prodname').textContent;
	var actno = getActivationNumber();
	if (actno && actno.length>5) {
	  document.getElementById('activationnumber').textContent = 'Your activation number is '+actno;
	}
	var prodnameArray = prodname.split(" ");
	if (bLicensed) {
		document.getElementById('licensed').removeAttribute('hidden');
	  document.getElementById('unlicensed').setAttribute('hidden', true);
		if (permanent) {
			expstr += "never expires.";
		} else {
			expstr += "expires in "+daysleft+" days."
		}
		document.getElementById('expiration').value=expstr;
	} else {
		document.getElementById('unlicensed').removeAttribute('hidden');
	  document.getElementById('licensed').setAttribute('hidden', true);
		if (daysleft && daysleft < 0) {
	  	document.getElementById('expiredon').value =
	  	'The license for this computer expired on '+licensedUntil()+'.';
	  	document.getElementById('expired').removeAttribute('hidden');
		}
	  else {
	  	document.getElementById('expired').setAttribute('hidden', true);

	  }
	}
}
