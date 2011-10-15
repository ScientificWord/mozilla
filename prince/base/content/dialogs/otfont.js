Components.utils.import("resource://app/modules/fontlist.jsm"); 
//Components.utils.import("resource://app/modules/pathutils.jsm"); 
Components.utils.import("resource://app/modules/unitHandler.jsm");

var fontnode;
var fontsize;
var fontcolor;
var leadingnode;
var data = { ruleColor: "#000000" };
var unitHandler;

function startup()
{
//  var editorElement = msiGetParentEditorElementForDialog(window);
//  if (!editorElement)
//  {
//    throw("No editor in otfont.OnAccept!");
//  }
//  var editor = msiGetEditor(editorElement);  
	unitHandler = new UnitHandler();
  try{
	initializeUnitHandler(unitHandler);
  var menuObject = { menulist: []};
  menuObject.menulist = document.getElementById("otfontlist");
  addOTFontsToMenu(menuObject);
}
catch(e) {
	dump(e.message);
}
//	fontnode = getSelectionParentByTag(editor, "otfont");
//  initializeFontName(fontnode);
//	fontsize = getSelectionParentByTag(editor, "fontsize"); // always in pts
//  initializeFontSize(fontsize);
//	fontcolor = getSelectionParentByTag(editor, "fontcolor");
//  initializeColorData(fontcolor);
//	leadingnode = getSelectionParentByTag(editor, "leading");
//  initializeLeading(leadingnode);
}

function initializeUnitHandler(unithandler)
{
	var fieldlist = [];
	fieldlist.push(document.getElementById("otfont.fontsize"));
	fieldlist.push(document.getElementById("leading"));
	unithandler.setEditFieldList(fieldlist);
	unithandler.initCurrentUnit("pt")
}

//function initializeFontName(node)
//{
//  var menuObject = { };
//	var initial;
//  initializeFontFamilyList(false);
//  if (node)
//  {
//    initial = node.getAttribute("fontname");
//    if (initial) 
//		{
//			document.getElementById("otfontlist").value = initial;
//		}
//  }	
//}
//
//function initializeFontSize(node)
//{
//	var size;
//	if (node)
//	{
//		size = node.getAttribute("size");
//		if (leading)
//		{
//			document.getElementById("size").value = size;
//		}		
//	}
//}
//
//function initializeColorData(node)
//{
//	var color;
//	if (node)
//	{
//		color = node.getAttribute("color");
//		if (color)
//		{
//			setColorWell("colorWell", color);
//		}		
//	}
//}
//
//function initializeLeading(node)
//{
//	var leading;
//	if (node)
//	{
//		leading = node.getAttribute("val");
//		if (leading)
//		{
//			document.getElementById("leading").value = leading;
//		}		
//	}
//}

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
	
  var theWindow = window.opener;
  if (!theWindow || !("msiEditorSetTextProperty" in theWindow))
  {
    theWindow = msiGetTopLevelWindow();
  }
//  theWindow.msiRequirePackage(editorElement, "xltxtra", null);
  var hasRealData = (!!fontname || !!color || !!fontsize || !!leading);
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

function onCancel()
{
  return true;
}

