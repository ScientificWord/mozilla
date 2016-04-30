
function onLoad() {
  var versionField = document.getElementById("versionField");
  var brandShortNameEl = document.getElementById("brandShortName");
  versionField.value = brandShortNameEl.firstChild.nodeValue + ' ' + navigator.vendorSub + ' (6.0.14 04/29/2016)';
  var CopyRight = document.getElementById("CopyRight");
  var currentTime = new Date();
  var year = currentTime.getFullYear();
  CopyRight.value = '©' + ' ' + year + '. All rights reserved.'
}

