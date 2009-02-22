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
  var errObj =document.getElementById("errdisplay").cloneNode(true);
  var matchArray=/(.*)Location:\s*([^L]*)Line\sNumber\s(\d*), Column (\d*):([^\-]*)/.exec(msg);
  
  errObj.setAttribute("msg", escapeString(matchArray[1]));
  errObj.setAttribute("href", matchArray[2]);
  errObj.setAttribute("line", matchArray[3]);
  errObj.setAttribute("col", matchArray[4]);
  errObj.setAttribute("code", escapeString(matchArray[5]));
  errObj.setAttribute("errorDots", repeatChar("&nbsp;",Number(matchArray[4]))); 
  document.getElementById("theparent").replaceChild(errObj,document.getElementById("errdisplay"));
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