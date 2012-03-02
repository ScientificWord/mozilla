var data;
var msg;

function escapeString(s)
{
  var newS = s.replace("&", "&amp;","g");
  newS = newS.replace('"', '&quot;', "g");
  newS = newS.replace('<', '&lt;', 'g');
  return newS;
}

function repeatChar( theChar, theCol)
{
  if (--theCol <= 0)
    return "";

  for (var i = 2; i < theCol; i += i)
    theChar += theChar;

  return theChar + theChar.slice(0, theCol - theChar.length);
}


function startup()
{
  data = window.arguments[0];
  var msg = data.msg;
  var splitMsg = msg.split("\n");
  var i, line;
  var parent = document.getElementById("theparent");
  for (i = 0; i < splitMsg.length; i++)
  {
    if (i === 1) continue;
    line = document.createElement("label");
    line.setAttribute("value", splitMsg[i]);
    if ( i > 1) {
      line.setAttribute("class", "code");
    }
    parent.appendChild(line);
  }
}

function onAccept()
{
  data.result = true;
  return true;
}

function onCancel()
{
  data.result = false;
  return true;
}