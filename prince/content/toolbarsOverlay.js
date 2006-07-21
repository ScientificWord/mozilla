

function setCompact(aMenuItemId)
{
  if (document.getElementById(aMenuItemId).getAttribute('checked')=='true')
  {
    document.getElementById('SymbolToolbox').setAttribute('iconsize','small');
  }
  else
  {
    document.getElementById('SymbolToolbox').removeAttribute('iconsize');
  }
}



function setDefaultPrinter(aStr, aEvent)
{
  document.getElementById('defaultCompiler').setAttribute('defaultcompiler',aStr);
  aEvent.stopPropagation();
  return true;
}


function setDefault(obj)
{
  if (obj.getAttribute('for')==obj.getAttribute('defaultcompiler')) {
    obj.setAttribute('default','true');
    obj.parentNode.parentNode.setAttribute('oncommand',obj.getAttribute('oncommand'));   // the button is up two.
  }
  else
    obj.setAttribute('default','false');
  x = 3+4;
}

function setAnimation()
{
  if (document.getElementById('gifsAnimatedButton').getAttribute('checkState')=='1') 
    stopAnimation();
  else startAnimation(); 
}         