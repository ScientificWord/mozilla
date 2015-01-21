 
 
function onLoad() {
  var versionField = document.getElementById("versionField");
  var brandShortNameEl = document.getElementById("brandShortName");          
  versionField.value = brandShortNameEl.firstChild.nodeValue + ' ' + navigator.vendorSub + ' (' + navigator.productSub + ')';
  var CopyRight = document.getElementById("CopyRight");
  var currentTime = new Date();
  var year = currentTime.getFullYear();
  CopyRight.value = '©' + ' ' + year + '. All rights reserved.'
}

