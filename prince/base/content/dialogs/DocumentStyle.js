

function startup()
{
// get a list of the style sheets used in the document and
// get a list of the tag definitions
  var docStyle = window.arguments[0];
  var editor;
  var editordoc;
  var cssList;
  var xmlList;
  var styleSheetList;
  var arr;
  var possibleDups;
  var i;
  var uri;
  if (!docStyle.edElement) return;  //BBM think this through
  editor = msiGetEditor(docStyle.edElement);
  editordoc = editor.document;
  cssList =  document.getElementById("csslist");
  xmlList =  document.getElementById("xmllist");
  styleSheetList = editordoc.styleSheets;
  arr = [];
  possibleDups=[];
  // build an array of stylesheet names. 
  // we want to remove duplicates, but the later instances are more significant,
  // so we remove by going back to front.
  for (i = styleSheetList.length - 1; i>= 0;  i--)
  {
    uri = styleSheetList[i].href; 
    if (possibleDups.indexOf(uri) === -1)
    {
      arr.push(uri);
      possibleDups.push(uri);
    }
  }
  for (i = arr.length -1; i>=0; i--)
    cssList.appendItem(arr[i]);

  var tagdefsArr;
  tagdefsArr = processingInstructionsList(editordoc, "sw-tagdefs");
  for (i = 0; i < tagdefsArr.length; i++)
      xmlList.appendItem(tagdefsArr[i]);
}

function onAccept()
{
}

function onCancel()
{
}