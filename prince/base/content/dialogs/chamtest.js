function Startup()
{
  window.opener.focus();
}

function fire()
{
//  alert('Ouch!');
  window.opener.focus();
  window.opener.insertMathSymbol('±',false);
  window.opener.document.getElementById('content-frame').focus();
  window.opener.document.getElementById('content-frame').select();
}

function offButtonClick()
{
  window.opener.focus();
}