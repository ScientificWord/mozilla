var cleanupOptions = [];

function startup()
{
  cleanupOptions= window.arguments[0].cleanupOptions;
  
}

function onAccept()
{
  var checkboxes = document.getElementsByTagName("checkbox");
  cleanupOptions = [];
  var i;
  for (i = 0; i< checkboxes.length; i++)
  {
    if (checkboxes[i].checked) cleanupOptions.push(checkboxes[i].id);
  }
  window.arguments[0].cleanupOptions = cleanupOptions;
  return true;
}

function onCancel()
{
  return true;
}