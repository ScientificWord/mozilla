
function onLoad() {
  var versionField = document.getElementById("versionField");
  var brandShortNameEl = document.getElementById("brandShortName");
  var dateArray = /(\d{4})(\d{2})(\d{2})/.exec(navigator.productSub);
  versionField.value = brandShortNameEl.firstChild.nodeValue + ' ' + navigator.buildID + ' (' + dateArray.slice(1).join('-') + ')';
  var CopyRight = document.getElementById("CopyRight");
  var currentTime = new Date();
  var year = currentTime.getFullYear();
  CopyRight.value = '©' + ' ' + year + '. All rights reserved.'
}

