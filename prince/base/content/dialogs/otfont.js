Components.utils.import("resource://app/modules/fontlist.jsm"); 
//Components.utils.import("resource://app/modules/pathutils.jsm"); 
Components.utils.import("resource://app/modules/unitHandler.jsm");

var texnode;
var data = { ruleColor: "#000000" };
var unitHandler;

function startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  if (!editorElement)
  {
    throw("No editor in otfont.OnAccept!");
  }
  var editor = msiGetEditor(editorElement);  
	unitHandler = new UnitHandler();
	initializeUnitHandler(unitHandler);
  var menuObject = { menulist: []};
  menuObject.menulist = document.getElementById("otfontlist");
  addOTFontsToMenu(menuObject);
  texnode = getSelectionParentByTag(editor, "rawTeX");
  if (texnode) {
		document.getElementById("rawtex").value = texnode.getAttribute("tex");
	}
}

function initializeUnitHandler(unithandler)
{
	var fieldlist = [];
	fieldlist.push(document.getElementById("otfont.fontsize"));
	fieldlist.push(document.getElementById("leading"));
	unithandler.setEditFieldList(fieldlist);
	unithandler.initCurrentUnit("pt")
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
  colorWell.setAttribute("color",color);
}

function onAccept()
{
  try{
	var editorElement = msiGetParentEditorElementForDialog(window);
   if (!editorElement)
   {
     throw("No editor in otfont.OnAccept!");
   }
	var editor = msiGetEditor(editorElement);
  
  var fontname = getFontName();
	var leading = unitHandler.getValueAs(getLeading(), "pt");
	var color = getColorFromColorPicker();
	var fontsize = unitHandler.getValueAs(getFontSize(), "pt");
	var rawtex = getRawTeX();
	
  var theWindow = window.opener;
  if (!theWindow || !("msiEditorSetTextProperty" in theWindow))
  {
    theWindow = msiGetTopLevelWindow();
  }
//  theWindow.msiRequirePackage(editorElement, "xltxtra", null);
  var hasRealData = (!!fontname || !!color || !!fontsize || !!leading || rawtex);
  if (hasRealData){
		editor.beginTransaction();
		if (fontname) theWindow.msiEditorSetTextProperty(editorElement, "otfont", "fontname", fontname);
		if (color) theWindow.msiEditorSetTextProperty(editorElement, "fontcolor", "color", color);
		if (fontsize) 
		{
			var fontsizedata;
			if (leading) fontsizedata = fontsize+"/"+leading+" pt";
			else fontsizedata = fontsize+"/"+fontsize;
			theWindow.msiEditorSetTextProperty(editorElement, "fontsize", "size", fontsizedata);
		}
		else if (leading) theWindow.msiEditorSetTextProperty(editorElement, "leading", "val", leading+"pt");
		if (rawtex) {
			if (texnode) texnode.setAttribute("tex", rawtex);
			else theWindow.msiEditorSetTextProperty(editorElement, "rawTeX", "tex", rawtex);
		}
		editor.endTransaction();
	}
  editorElement.contentWindow.focus();
}
catch(e){
	dump(e.message);
}
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
	if (Number(size) == 0)
	{
		size = null;
	}
	return size;
}

function getLeading() 
{
	var leading = document.getElementById("leading").value;
	if (Number(leading) == 0)
	{
		leading = null;
	}
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
  if (!color || color=="") color = null;	
	return color;
}

function getRawTeX()
{
	var tex = document.getElementById("rawtex").value;
	if (!tex || tex.length == 0) tex = null;
	return tex;
}

function onCancel()
{
  return true;
}

