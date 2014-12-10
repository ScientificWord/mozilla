
function accept ()
{
	return true;
}

function onLoad()
{
	var bLicensed = isLicensed();
	var daysleft = licenseTimeRemaining();
	var permanent = (daysleft === "permanent");
	var expstr = "This license ";
	var prodname = document.getElementById('prodname').textContent;
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
		document.getElementById('licensed').setAttribute('hidden', true);
	  document.getElementById('unlicensed').removeAttribute('hidden');
	  if (daysleft && daysleft < 0) {
	  	document.getElementById('expiredon').value =
	  	'The license for this computer expired on '+licenseExpDate()+'.';
	  	document.getElementById('expired').removeAttribute('hidden');
	  }
	  else {
	  	document.getElementById('expired').setAttribute('hidden', true);

	  }
	}
}