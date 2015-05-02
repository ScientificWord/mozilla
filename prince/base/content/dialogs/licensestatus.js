#include ../productname.inc
/// This comment fixes a syntax highlighting glitch on my laptop -- BBM


function startUp() {
	var isLicensed = window.arguments[0];
	var days = window.arguments[1];
	var verbiage1;
	var product = document.getElementById('prod').textContent;
	if (isLicensed) {
		verbiage1 = document.getElementById('verb1').textContent;
	}
	else {
		verbiage1 = document.getElementById('verb1a').textContent;
		document.getElementById('subtitle').hidden=true;
	}

	verbiage1 = verbiage1.replace("**days**", days.toString());
	verbiage1 = verbiage1.replace("**prod**", product);
	document.getElementById('verb1').textContent = verbiage1;
}

