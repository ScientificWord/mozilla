 
 
function onLoad() {
  var versionField = document.getElementById("versionField");
  var hidversionField = document.getElementById("hidversionField");
  var brandShortNameEl = document.getElementById("brandShortName");          
  hidversionField.value = brandShortNameEl.firstChild.nodeValue + ' ' + navigator.vendorSub + ' (' + navigator.productSub + ')';
  versionField.value = hidversionField.value;
  versionField.width = hidversionField.width;
  document.getElementById("lic").checked = false;
  document.getElementById("cred").checked = false;
}


var creditsURL="chrome://prince/content/credits.xhtml";
var licensesURL="chrome://prince/content/licenses.xhtml";


function showCredits()
{
  if (document.getElementById("cred").checked)
  {
    document.getElementById("aboutframe").setAttribute("src",creditsURL);
    document.getElementById("lic").checked = false;
    document.getElementById("aboutdeck").setAttribute("selectedIndex","1");
  }
  else 
    document.getElementById("aboutdeck").setAttribute("selectedIndex","0");
}


function showLicenses()
{
  if (document.getElementById("lic").checked)
  {
    document.getElementById("aboutframe").setAttribute("src",licensesURL);
    document.getElementById("cred").checked = false;
    document.getElementById("aboutdeck").setAttribute("selectedIndex","1");
  }
  else 
    document.getElementById("aboutdeck").setAttribute("selectedIndex","0");
}
