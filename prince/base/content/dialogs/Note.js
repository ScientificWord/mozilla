
var data;
function startUp()
{
  data = window.arguments[0];
  if (data.type)
  {
    document.getElementById("note.names").value = data.type;
    document.getElementById("hidenote").checked = data.hide;
  }
  else data.type = document.getElementById("note.names").value;
}

  
function onOK() {
  data.type = document.getElementById("note.names").value;
  data.hide = document.getElementById("hidenote").checked;
  close();
  return (false);
}

function onCancel() {
  close();
  return(true);
}

