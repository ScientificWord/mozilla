

function onLoad() {
  var versionField = document.getElementById("versionField");
  var brandShortNameEl = document.getElementById("brandShortName");
  versionField.value = brandShortNameEl.firstChild.nodeValue + ' ' + navigator.vendorSub + ' (6.0.8 11/05/2015)';
  var CopyRight = document.getElementById("CopyRight");
  var currentTime = new Date();
  var year = currentTime.getFullYear();
  CopyRight.value = '©' + ' ' + year + '. All rights reserved.'
}

