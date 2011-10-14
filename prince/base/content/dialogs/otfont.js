Components.utils.import("resource://app/modules/fontlist.jsm"); 
Components.utils.import("resource://app/modules/pathutils.jsm"); 
Components.utils.import("resource://app/modules/unitHandler.jsm");

var node;
var data = { ruleColor: "#000000" };

function startup()
{
  if (window.arguments && window.arguments.length > 0)
	{
	  node = window.arguments[0];
	}
  initializeFontName(node);
  initializeFontSize(node);
  initializeColorData(node);
  initializeLeading(node);
  initializeUnits(node);
}

function initializeFontName(node)
{
  var menuObject = { };
	var initial;
  initializeFontFamilyList(false);
  menuObject.menulist = document.getElementById("otfontlist");
  addOTFontsToMenu(menuObject);
  if (node)
  {
    initial = node.getAttribute("fontname");
    if (initial) 
		{
			document.getElementById("otfontlist").value = initial;
		}
  }	
}

function initializeFontSize(node)
{
	var size;
	if (node)
	{
		size = node.getAttribute("size");
		if (leading)
		{
			document.getElementById("size").value = size;
		}		
	}
}

function initializeColorData(node)
{
	var color;
	if (node)
	{
		color = node.getAttribute("color");
		if (color)
		{
			setColorWell("colorWell", color);
		}		
	}
}

function initializeLeading(node)
{
	var leading;
	if (node)
	{
		leading = node.getAttribute("leading");
		if (leading)
		{
			document.getElementById("leading").value = leading;
		}		
	}
}

function initializeUnits(node)
{
	var unit;
	if (node)
	{
		unit = node.getAttribute("units");
		if (unit)
		{
			document.getElementsById("otfont.units");
		}
	}
}

function getColorAndUpdate()
{
  var colorWell = document.getElementById("colorWell");
  if (!colorWell) return;
  var color;

  var colorObj = { NoDefault: false, Type: "Rule", TextColor: color, PageColor: 0, Cancel: false };
  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);
  if (colorObj.Cancel)
    return;
  color = colorObj.TextColor;
  setColorWell("colorWell", color); 
}




function onAccept()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
   if (!editorElement)
   {
     throw("No editor in otfont.OnAccept!");
   }
  var fontname = getFontName();
	var leading = getLeading();
	var color = getColorFromColorPicker();
	var unit = getUnits();
	var fontsize = getFontSize();
	alert(fontname+","+leading+","+color+","+unit+","+fontsize);
	
//  if (node)
//  {
//    node.setAttribute("fontname", fontname);
//    node.setAttribute("style","font-family: "+fontname+";");
//  }
//  else
//  {
//	  var theWindow = window.opener;
//	  if (!theWindow || !("msiEditorSetTextProperty" in theWindow))
//    {
//	    theWindow = msiGetTopLevelWindow();
//    }
//    theWindow.msiRequirePackage(editorElement, "xltxtra", null);
//    theWindow.msiEditorSetTextProperty(editorElement, "otfont", "fontname", fontname);
//  }
  editorElement.contentWindow.focus();
}

function getFontName()
{
  var list = document.getElementById("otfontlist");
	if (list.selectedIndex  > 0)
  {
    return list.value;
  }
  return null;
}

function getFontSize() 
{
	var size = document.getElementById("otfont.fontsize").value;
	return size;
}

function getLeading() 
{
	var leading = document.getElementById("leading").value;
	return leading;
}

function getUnits() 
{
	var units = document.getElementById("otfont.units").value;
	return units;
}

function getColorFromColorPicker()
{
	var color = getColor("colorWell");
	return color;
}

function onCancel()
{
  return true;
}

