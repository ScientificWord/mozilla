

function setCompact(aMenuItemId)
{
  if (document.getElementById(aMenuItemId).getAttribute('checked')=='true')
  {
    document.getElementById('Symbol-tabbox').setAttribute('iconsize','small');
  }
  else
  {
    document.getElementById('Symbol-tabbox').removeAttribute('iconsize');
  }
}

function setSymbolSize( size )
{
  var tabbox = document.getElementById('Symbol-tabbox');
  var oldsize = tabbox.getAttribute('textsize');
  if (oldsize == size) return;
  var oldmenu = document.getElementById('symbols'+oldsize);
  oldmenu.setAttribute("checked", "false");
  tabbox.setAttribute('textsize', size);
}

function setDefaultPrinter(aStr, aEvent)
{
  document.getElementById('defaultCompiler').setAttribute('defcomp',aStr);
  document.getElementById('defaultCompiler').setAttribute('printCommand','cmd_print'+aStr.substr(0,1).toUpperCase()+aStr.substr(1));
  document.getElementById('defaultCompiler').setAttribute('previewCommand','cmd_preview'+aStr.substr(0,1).toUpperCase()+aStr.substr(1));
  aEvent.stopPropagation();
  return true;
}


function setDefault(obj)
{
  if (obj.getAttribute('for')==obj.getAttribute('defcomp')) {
    obj.setAttribute('default','true');
  }
  else
    obj.setAttribute('default','false');
}

function setDefaultmenu(obj)
{
  if (obj.getAttribute('for')==obj.getAttribute('defcomp')) {
    obj.setAttribute('default','true');
  }
  else
    obj.setAttribute('default','false');
}


function setAnimation()
{
  if (document.getElementById('gifsAnimatedButton').getAttribute('checkState')=='1') 
    msiStopAnimation();
  else msiStartAnimation(); 
}         