
var data;
function startUp()
{
  data = window.arguments[0];
  if (data.type)
  {
    document.getElementById("note.names").value = data.type;
  }
  else data.type = document.getElementById("note.names").value;
}

  
function onOK() {
  data.type = document.getElementById("note.names").value;
  close();
  return (false);
}

function onCancel() {
  close();
  return(true);
}

