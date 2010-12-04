 
 
function onLoad() {
  var versionField = document.getElementById("versionField");
  var brandShortNameEl = document.getElementById("brandShortName");          
  versionField.value = brandShortNameEl.firstChild.nodeValue + ' ' + navigator.vendorSub + ' (' + navigator.productSub + ')';
}

