Components.utils.import("resource://app/modules/unitHandler.jsm");

var frameUnitHandler;
var sides;
var gFrameTab;
var scale;
var scaledWidthDefault; 
var scaledHeightDefault;
var scaledHeight;
var scaledWidth; 
var Dg;
var position;
var marginAtt = "margin";
var paddingAtt = "padding";
var borderAtt = "border";
var widthAtt = "width";
var heightAtt = "height";
var metrics = {margin:{}, border: {}, padding: {}};
var role;
var gConstrainWidth = 0;
var gConstrainHeight = 0;
var gActualWidth = 0;
var gActualHeight = 0;
var gDefaultPlacement = "";

function setHasNaturalSize(istrue)
// images frequently have a natural size
{
  var bcaster = document.getElementById("hasNaturalSize"); 
	if (istrue) 
	{
		bcaster.removeAttribute("hidden");
	}
	else
	{
		bcaster.setAttribute("hidden", "true");
	}
}

function setCanRotate(istrue)
{
  var rotationbox = document.getElementById("rotate");
  if (!istrue)
  {
    rotationbox.setAttribute("hidden", "true");
  }
  else
  {
    rotationbox.removeAttribute("hidden");
  }
}

function updateMetrics()
{
	// this function takes care of the mapping in the cases where there are constraints on margins, padding, etc.
  var sub;
	if (gFrameModeImage)
	{
		sub = metrics.margin;
		sub.top    = sub.bottom = Dg.marginInput.top.value;
		switch(position)
		{
			case 0: sub.left = sub.right = 0; // centered
				break;
			case 1: sub.right = Number(Dg.marginInput.left.value); 	// on the left
							sub.left  = -Number(Dg.marginInput.right.value);
			  break;
			case 2: sub.right = -Number(Dg.marginInput.right.value);
							sub.left  = Number(Dg.marginInput.left.value);
				break;
		}
	  sub = metrics.padding;
		sub.top    = sub.right  = sub.bottom = sub.left   = Dg.paddingInput.left.value;
	  sub = metrics.border;
	  sub.top    = sub.right  = sub.bottom = sub.left   = Dg.borderInput.left.value;
	  metrics.innermargin = metrics.outermargin = 0;  
	  metrics.unit = frameUnitHandler.currentUnit;		            
	}
	else
	{
		sub = metrics.margin;		
	  sub.top =    Dg.marginInput.top.value;
		sub.right =  Dg.marginInput.right.value; 
		sub.bottom = Dg.marginInput.bottom.value; 
		sub.left =   Dg.marginInput.left.value;
		if (position == 0)
		{
			sub.right = sub.left = 0;
		}
		sub = metrics.padding;
	  sub.top =    Dg.paddingInput.top.value; 
		sub.right =  Dg.paddingInput.right.value; 
		sub.bottom = Dg.paddingInput.bottom.value; 
		sub.left =   Dg.paddingInput.left.value;
		sub = metrics.border;
	  sub.top =    Dg.borderInput.top.value; 
		sub.right =  Dg.borderInput.right.value; 
		sub.bottom = Dg.borderInput.bottom.value; 
		sub.left =   Dg.borderInput.left.value;
	  metrics.innermargin = metrics.outermargin = 0;  
	  metrics.unit = frameUnitHandler.currentUnit;		            
	}
}

function initFrameTab(dg, element, newElement, contentsElement)
{
  var i;
  var len;
  var j;
  var len2;
  var values;
  var width = 0;
  Dg = dg;
  if (gFrameModeImage)
  {
    widthAtt = "imageWidth";
    heightAtt = "imageHeight";
  }
  frameUnitHandler = new UnitHandler();
  sides = ["Top", "Right", "Bottom", "Left"]; // do not localize -- visible to code only
  gFrameTab={};
  scale = 0.25;
  scaledWidthDefault = 50; 
  scaledHeightDefault = 60;
  scaledHeight = scaledHeightDefault;
  if (!gConstrainHeight)
    gConstrainHeight = scaledHeightDefault;
  scaledWidth = scaledWidthDefault; 
  if (!gConstrainWidth)
    gConstrainWidth = scaledWidthDefault;
  position = 0;  // left = 1, right = 2, neither = 0
  //var unit;

  dg.editorElement = msiGetParentEditorElementForDialog(window);
  dg.editor = msiGetEditor(dg.editorElement);
  if (!dg.editor) {
    window.close();
    return null;
  }
  // For convenience, map dialog elements to an object
//  currentFrame = element;
  dg.widthInput           = document.getElementById("frameWidthInput");
  dg.heightInput          = document.getElementById("frameHeightInput");
  dg.autoHeightCheck      = document.getElementById("autoHeight");
  dg.autoWidthCheck       = document.getElementById("autoWidth");
  dg.autoWidthLabel       = document.getElementById("autoWidthLabel");
  dg.frameUnitMenulist    = document.getElementById("frameUnitMenulist");
  dg.unitList             = document.getElementById("unitList");
  dg.sizeRadioGroup       = document.getElementById("sizeRadio");
  dg.actual               = document.getElementById( "actual" );
  dg.iconic               = document.getElementById( "iconic" );
  dg.custom               = document.getElementById( "custom" );
  dg.constrainCheckbox    = document.getElementById( "constrainCheckbox" );
  dg.marginInput          = {left:   document.getElementById("marginLeftInput"),
                             right: document.getElementById("marginRightInput"),
                             top:   document.getElementById("marginTopInput"),
                             bottom: document.getElementById("marginBottomInput")};
  dg.borderInput          = {left:  document.getElementById("borderLeftInput"),
                             right: document.getElementById("borderRightInput"),
                             top:   document.getElementById("borderTopInput"),
                             bottom: document.getElementById("borderBottomInput")};
  dg.paddingInput         = {left:  document.getElementById("paddingLeftInput"),
                             right: document.getElementById("paddingRightInput"),
                             top:   document.getElementById("paddingTopInput"),
                             bottom: document.getElementById("paddingBottomInput")};
  dg.colorWell            = document.getElementById("colorWell");
  dg.bgcolorWell          = document.getElementById("bgcolorWell");
  dg.textAlignment				= document.getElementById("textAlignment");
  dg.rotationList					= document.getElementById("rotationList");
  dg.placementRadioGroup  = document.getElementById("placementRadioGroup");
  dg.placeForceHereCheck  = document.getElementById("placeForceHereCheck");
  dg.placeHereCheck       = document.getElementById("placeHereCheck");
  dg.placeFloatsCheck     = document.getElementById("placeFloatsCheck");
  dg.placeTopCheck        = document.getElementById("placeTopCheck");
  dg.placeBottomCheck     = document.getElementById("placeBottomCheck");
  dg.herePlacementRadioGroup  = document.getElementById("herePlacementRadioGroup");
  dg.OkButton             = document.documentElement.getButton("accept");
  var fieldList = [];
  var attrs = ["margin","border","padding"];
  for (i=0, len = sides.length; i<len; i++)
  {
    for (j = 0, len2 = attrs.length; j < len2; j++)
    {
      fieldList.push(dg[attrs[j]+"Input"][sides[i].toLowerCase()]);
    }
  }
  fieldList.push(dg.heightInput);
  fieldList.push(dg.widthInput);
  frameUnitHandler.setEditFieldList(fieldList);
	frameUnitHandler.initCurrentUnit(dg.frameUnitMenulist.value);
// The defaults for the document are set by the XUL document, modified by persist attributes. If there is
// no pre-existing frame object, the dg is set to go.
  var placeLocation, placementStr, pos;
  if (!newElement)
  {   // we need to initialize the dg from the frame element
    if (!contentsElement)
      contentsElement = element;

    frameUnitHandler.setCurrentUnit(contentsElement.getAttribute("units"));
    
    var width = 0;
    var widthStr = "";
    if (contentsElement.hasAttribute(widthAtt))
      width = frameUnitHandler.getValueFromString( contentsElement.getAttribute(widthAtt) );
    else
    {
      widthStr = msiGetHTMLOrCSSStyleValue(dg.editorElement, contentsElement, widthAtt, "width");
      if (widthStr)
        width = frameUnitHandler.getValueFromString( widthStr, "px" );
    }
    var height = 0;
    var heightStr = "";
    if (contentsElement.hasAttribute(heightAtt))
      height = frameUnitHandler.getValueFromString( contentsElement.getAttribute(heightAtt) );
    else
    {
      heightStr = msiGetHTMLOrCSSStyleValue(dg.editorElement, contentsElement, heightAtt, "height");
      if (heightStr)
        height = frameUnitHandler.getValueFromString( heightStr, "px" );
    }
    if (!gConstrainWidth || !gConstrainHeight)
      setConstrainDimensions(frameUnitHandler.getValueAs(width, "px"), frameUnitHandler.getValueAs(height,"px"));
    setWidthAndHeight(width, height, null);

    try
		{
      for (i = 0; i < dg.frameUnitMenulist.itemCount; i++)
  		{
        if (dg.frameUnitMenulist.getItemAtIndex(i).value === frameUnitHandler.currentUnit)
        {
          dg.frameUnitMenulist.selectedIndex = i;
          break;
        }
  		}	
		}
		catch(e)
    {
      msidump(e.message);
    }
    if (gFrameModeImage) 
		{
		  placement = element.getAttribute("placement");
			if (placement == "L")
			{
				position = 1;  // left = 1, right = 2, neither = 0
			}
			else if (placement == "R")
			{
				position = 2;
			}
			var overhang = element.getAttribute("overhang");
			if (overhang == null) overhang = 0;
			dg.marginInput.right.value = overhang;
			var sidemargin = element.getAttribute("sidemargin");
			if (sidemargin == null) sidemargin = 0;
			dg.marginInput.left.value = sidemargin;
			var topmargin = element.getAttribute("topmargin");
			if (topmargin == null) topmargin = 0;
			dg.marginInput.top.value = topmargin;
			var borderwidth = contentsElement.getAttribute("borderw");
			if (borderwidth == null) borderwidth = 0;
			dg.borderInput.left.value = borderwidth;
			var padding = contentsElement.getAttribute("padding");
			if (padding == null) padding = 0;
			dg.paddingInput.left.value = padding;
		}  
		else
		{
	    values = [0,0,0,0];
	    if (element.hasAttribute(marginAtt))
	      { values = parseLengths(element.getAttribute(marginAtt));}
	    for (i = 0; i<4; i++)
	      { dg.marginInput[sides[i].toLowerCase()].value = values[i];}
	    values = [0,0,0,0];
	    if (contentsElement.hasAttribute("borderw"))
	      { values = parseLengths(contentsElement.getAttribute("borderw"));}
	    for (i = 0; i<4; i++)
	      { dg.borderInput[sides[i].toLowerCase()].value = values[i];}
	    values = [0,0,0,0];
	    if (contentsElement.hasAttribute(paddingAtt))
	      { values = parseLengths(contentsElement.getAttribute(paddingAtt));}
	    for (i = 0; i<4; i++)
	      { dg.paddingInput[sides[i].toLowerCase()].value = values[i];}
    }
		placeLocation = element.getAttribute("placeLocation");
    if (!placeLocation)
      placeLocation = "";
    placementStr = element.getAttribute("placement");
    if (!placementStr)
      placementStr = "";
    pos = element.getAttribute("pos");
    if (!pos)
      pos = "";

    var theColor;
    if (contentsElement.hasAttribute("border-color"))
    {
      theColor = hexcolor(contentsElement.getAttribute("border-color"));
      setColorInDialog("colorWell", theColor);
    }
    if (contentsElement.hasAttribute("background-color"))
    {
      theColor = hexcolor(contentsElement.getAttribute("background-color"));
      setColorInDialog("bgcolorWell", theColor);
    }
  }
  else if (gDefaultPlacement.length)
  {
    var defPlacementArray = gDefaultPlacement.split(",");
    if (defPlacementArray.length)
    {
      pos = TrimString(defPlacementArray[0]);
      if (defPlacementArray.length > 1)
      {
        placeLocation = TrimString(defPlacementArray[1]);
        if (defPlacementArray.length > 2)
          placementStr = TrimString(defPlacementArray[2]);
      }
    }
  }

  if (!newElement || gDefaultPlacement.length)
  {
    try
    {  dg.placeForceHereCheck.checked = (placeLocation.search("H") != -1);
      dg.placeHereCheck.checked = (placeLocation.search("h") != -1);
      dg.placeFloatsCheck.checked = (placeLocation.search("p") != -1);
      dg.placeTopCheck.checked = (placeLocation.search("t") != -1);
      dg.placeBottomCheck.checked = (placeLocation.search("b") != -1);

      dg.herePlacementRadioGroup.value = placementStr;
      if (!dg.herePlacementRadioGroup.value || !dg.herePlacementRadioGroup.value.length)
        dg.herePlacementRadioGroupValue = "full";  //as in the default below
      switch (dg.herePlacementRadioGroup.value) {
        case "L": dg.herePlacementRadioGroup.selectedIndex = 0;
                  break;
        case "R": dg.herePlacementRadioGroup.selectedIndex = 1;
                  break;
        case "I": dg.herePlacementRadioGroup.selectedIndex = 2;
                  break;
        case "O": dg.herePlacementRadioGroup.selectedIndex = 3;
                  break;
        default:  dg.herePlacementRadioGroup.selectedIndex = 4;
      }
      
      dg.placementRadioGroup.selectedIndex = (pos == "inline")?0:(pos == "display")?1:(pos == "float")?2:-1;
    }
    catch(e)
    {
      msidump(e.message);
    }
  }

  var placement = 0;
  var placementLetter = document.getElementById("herePlacementRadioGroup").value;
  if (/l|i/i.test(placementLetter)) placement=1;
  else if (/r|o/i.test(placementLetter)) placement = 2;
  gFrameTab = dg;
  setAlignment(placement);
  enableHere(dg.herePlacementRadioGroup);
  enableFloating();
  updateDiagram(marginAtt);
  updateDiagram(borderAtt);
  updateDiagram(paddingAtt);
  var broadcaster = document.getElementById("role-image");
  var hiddenAttr = broadcaster.getAttribute("hidden");
  if ( hiddenAttr == true )
    role = "textframe";
  else role = "image";
	updateMetrics();
  return dg;
  
}

function setNewUnit(element)
{
  frameUnitHandler.setCurrentUnit(element.value);
}

function initFrameSizePanel()
{
  document.getElementById("hasNaturalSize").removeAttribute("hidden");
}

function initUnitList(unitPopUp)
{
  var elements = unitPopUp.getElementsByTagName("menuitem");
  var i, len;
  for (i=0, len = elements.length; i<len; i++)
  {
    elements[i].label = frameUnitHandler.getDisplayString(elements[i].value);
  }
}
  
function parseLengths( str ) 
{
  var values = str.split(" ");
  var length = values.length;
  if (length < 1) return [];
  // check to see if there is any empty string at the end.
  if (/^\s*$/.test(values[length-1]))
  {
    length--;
    values.pop();
  }
  switch (length)  // notice there are no breaks. Fall-through is intentional
  {
    case 1: values.push(values[0]);
    case 2: values.push(values[0]); 
    case 3: values.push(values[1]);
    default:
  }
  return values;
}

function update( anId )
{
  //parse the id
  if (anId.length == 0) return;
  var regexp=/(.*)(Left|Right|Top|Bottom)(.*)/;
  var result = regexp.exec(anId);
//  switch (result[2]) { // notice that there are no breaks in this switch statement -- intentional
//    case "Left":  document.getElementById(result[1]+"Left"+result[3]).value = document.getElementById(anId).value;
//    case "Right": document.getElementById(result[1]+"Right"+result[3]).value = document.getElementById(anId).value;
//    case "Top":   document.getElementById(result[1]+"Top"+result[3]).value = document.getElementById(anId).value;
//    case "Bottom": document.getElementById(result[1]+"Bottom"+result[3]).value = document.getElementById(anId).value;
//  }
  if (gFrameModeImage)
 	{
		if (result[1] == "border" || result[1] == "padding")
		{
			extendInput( result[1] );
		}
	}
  updateDiagram( result[1] );
}

function extendInput( anId )
{
  var input = "Input";
  var val = document.getElementById(anId+"Left"+input).value;
  document.getElementById(anId+"Right"+input).value = Math.max(0,val);
  document.getElementById(anId+"Top"+input).value = Math.max(0,val);
  document.getElementById(anId+"Bottom"+input).value = Math.max(0,val);
  updateDiagram( anId );
}

function toPixels( x )
{
  return Math.round(.4999 + frameUnitHandler.getValueAs(x,"px")*scale);
}

var color;
function getColorAndUpdate(id)
{
  var colorWell; 
	colorWell = document.getElementById(id);
  if (!colorWell) return;

  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"Rule", TextColor:color, PageColor:0, Cancel:false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);

  // User canceled the gFrameTab
  if (colorObj.Cancel)
    return;

  setColorInDialog(id, colorObj.TextColor);
}

function setColorInDialog(id, color)
{
  setColorWell(id, color); 
  if (id == "colorWell") 
	  setStyleAttributeByID("frame","border-color",color);
  else
	  setContentBGColor(color);
}

// come up with a four part attribute giving the four parts of the margin or padding or border, etc. using the same rules as CSS
function getCompositeMeasurement(attribute, unit, showUnit)
{
  var i;
  var values = [];
  for (i = 0; i<4; i++)
  { 
    values.push( Math.max(0,frameUnitHandler.getValueAs(Number(document.getElementById(attribute + sides[i] + "Input").value ),unit)));
  }
  if (values[1] == values[3])
  {
		values.splice(3,1);
		if (values[0] == values[2])
		{
			values.splice(2,1);
			if (values[0] == values[1]) values.splice(1,1);			
		}
	}
  var val;
  if (showUnit) {val = values.join(unit+" ")+unit;}
  else { val = values.join(" ");}
  return val;
}

// The equivalent for when we want just one of the attributes. "which" is "Top", "Right", "Bottom", or Left"
function getSingleMeasurement(attribute, which, unit, showUnit)
{
  var i = -1;
  var value;
  for (var j=0; j<4; j++)
  {
    if (sides[j]==which)
    {
      i = j;
      break;
    }
  }
  if (i < 0) return;
  value = Math.max(0,frameUnitHandler.getValueAs(Number(document.getElementById(attribute + sides[i] + "Input").value ),unit));
  if (showUnit) value = value + unit;
  return value;
}

//attribute = margin, border, padding; 
function updateDiagram( attribute )
{
  var i;
  var values = [];
  updateMetrics();
   for (i = 0; i<4; i++)
     { values.push( Math.max(0,toPixels(metrics[attribute][sides[i].toLowerCase()]  )));}
   if (values[1] == values[3])
   {
     values.splice(3,1);
     if (values[0] == values[2]) 
     {
       values.splice(2,1);
       if (values[0] == values[1]) values.splice(1,1);
     }
   }
  var val = values.join("px ")+"px";
  if (attribute=="border")  // add border color and border width
  {
    var bgcolor = Dg.colorWell.getAttribute("style");
    var arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
    removeStyleAttributeFamilyOnNode(document.getElementById("frame"), "border");
    var style = document.getElementById("frame").getAttribute("style");
    style += " border-width: "+val+"; border-color: " + arr[1]+"; border-style: solid;";
    document.getElementById("frame").setAttribute("style", style);
  }
  else
  { 
		setStyleAttributeByID("frame", attribute, val );
	}
  bgcolor = Dg.bgcolorWell.getAttribute("style");
  arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
  setContentBGColor(arr[1]);
  redrawDiagram();
}

function redrawDiagram()
{
  // space flowing around the diagram is 150 - (lmargin + rmargin + lborder + lpadding + rborder + rpadding + imageWidth)*scale
  var hmargin  = toPixels(Number(metrics.margin.left)) + toPixels(Number(metrics.margin.right));
  var hborder  = toPixels(Number(metrics.border.left)) + toPixels(Number(metrics.border.right));
  var hpadding = toPixels(Number(metrics.padding.left)) + toPixels(Number(metrics.padding.right));
  var vborder  = toPixels(Number(metrics.border.top)) + toPixels(Number(metrics.border.bottom));
  var vpadding = toPixels(Number(metrics.padding.top)) + toPixels(Number(metrics.padding.bottom));
  var vmargin  = toPixels(Number(metrics.margin.top)) + toPixels(Number(metrics.margin.bottom));
  switch(position)
  { // the total width is 210 px -- 60 for the left margin, 150 for the page
    case 1: document.getElementById("leftspacer").setAttribute("width", Math.min(60, Number(60 + toPixels(metrics.margin.left))) + "px"); 
            document.getElementById("leftpage").setAttribute("width", "0px");    
            document.getElementById("frame").setAttribute("width", scaledWidth+hborder+"px");
            document.getElementById("frame").setAttribute("height", scaledHeight+vborder+"px");
            document.getElementById("content").setAttribute("width", scaledWidth +"px");
            document.getElementById("content").setAttribute("height", scaledHeight +"px");
            document.getElementById("rightpage").setAttribute("width", 150 - (scaledWidth + hmargin+hborder) + "px");  
           // document.getElementById("rightspace").setAttribute("width", Math.max(0, - scaledWidth - (hmargin+hborder)) + "px");  
            document.getElementById("leftspace").setAttribute("width", "0px");
            break;
    case 2: document.getElementById("leftspacer").setAttribute("width", Number(60 + Math.min(0,150 - scaledWidth - (hmargin + hborder))) + "px"); 
            document.getElementById("leftpage").setAttribute("width", Math.min(150,150 - scaledWidth - (hmargin + hborder)) + "px");    
            document.getElementById("frame").setAttribute("width", scaledWidth+hborder +"px");
            document.getElementById("frame").setAttribute("height", (scaledHeight || 20)+vborder +"px");
            document.getElementById("content").setAttribute("width", scaledWidth+hborder +"px");
            document.getElementById("content").setAttribute("height", (scaledHeight || 20) +"px");
            document.getElementById("rightpage").setAttribute("width", "0px");  
            document.getElementById("rightspace").setAttribute("width", "0px");
            document.getElementById("leftspace").setAttribute("width", Math.max(0, - scaledWidth - (hmargin + hborder)) + "px");  
            break;
    default:document.getElementById("leftspacer").setAttribute("width", Number(135 - (scaledWidth + hborder)/2) + "px");
            document.getElementById("leftpage").setAttribute("width", "0px");    
            document.getElementById("frame").setAttribute("width", scaledWidth+"px");
            document.getElementById("frame").setAttribute("height", scaledHeight+"px");
            document.getElementById("content").setAttribute("width", scaledWidth+"px");
            document.getElementById("content").setAttribute("height", scaledHeight+"px");
            document.getElementById("rightpage").setAttribute("width", "0px");  
            document.getElementById("rightspace").setAttribute("width", "0px");
            document.getElementById("leftspace").setAttribute("width", "0px");
            break;
  }  	
}

function setStyleAttributeOnNode( node, att, value, editor)
{
  var style="";
  removeStyleAttributeFamilyOnNode( node, att, editor);
  if (node.hasAttribute("style")) style = node.getAttribute("style");
  style.replace("null","");
  style = style + " " + att +": " + value + "; ";
  if (editor)
    msiEditorEnsureElementAttribute(node, "style", style, editor);
  else
    node.setAttribute("style",style);
}

function removeStyleAttributeFamilyOnNode( node, att, editor)
{
  var style="";
  if (node.hasAttribute("style")) style = node.getAttribute("style");
  style.replace("null","");
  var re = new RegExp("^|[^-]"+att + "[-a-zA-Z]*:[^;]*;","g");
  if (re.test(style))
  {
    style = style.replace(re, "");
    if (editor)
      msiEditorEnsureElementAttribute(node, "style", style, editor);
    else
      node.setAttribute("style",style);
  }
}

function setStyleAttributeByID( id, att, value)
{
  setStyleAttributeOnNode(document.getElementById(id), att, value);
}

function enableHere(radiogroup )
{
  var broadcaster = document.getElementById("herePlacement");
  var theValue = "true";
  var position;
  if (!radiogroup) radiogroup = document.getElementById("herePlacementRadioGroup");
  if (document.getElementById('placeHereCheck').checked)
  {
    theValue = "false";
    position = radiogroup.selectedItem.value;
    setAlignment((position==="L" || position==="I")?1:((position==="R"||position=="O")?2:0));
  }
  else
  {
    setAlignment(0);
  }
  broadcaster.setAttribute("disabled",theValue);
//  document.getElementById('herePlacementRadioGroup').value;
  updateDiagram("margin");
}

function enableFloating( )
{
  var broadcaster = document.getElementById("floatingPlacement");
  var theValue = "true";
  if (document.getElementById('float').selected)
 	{
		theValue = "false";
	  broadcaster.setAttribute("disabled",theValue);
	//  if (theValue=="true") document.getElementById("herePlacement").setAttribute("disabled","true");
	//  else 
	  enableHere();
	}
	else if (document.getElementById('display').selected)
	{
		setAlignment(0);
	  updateDiagram("margin");
	} 
	else if (document.getElementById('inline').selected)
		updateDiagram("margin");
}

/************************************/
function handleChar(event, id)
{
  var element = event.originalTarget;
  if ((event.keyCode != event.DOM_VK_UP) && (event.keyCode != event.DOM_VK_DOWN))
    updateTextNumber(element, id, event);
  update(id);
  event.preventDefault();
}

function geomHandleChar(event, id, tbid)
{
  handleChar(event, id, tbid);
//  geomInputChar(event, id, tbid);
}

function unitRound( size, unit )
{
  var places;
  if (!unit) unit = frameUnitHandler.getCurrentUnit();
  // round off to a number of decimal places appropriate for the units
  switch (unit) {
    case "mm" : places = 10;
      break;
    case "cm" : places = 100;
      break;
    case "pt" : places = 10;
      break;
    case "in" : places = 100;
      break;
    default   : places = 100;
  }
  return Math.round(size*places)/places;
}

function setConstrainDimensions(width, height)
{
  gConstrainWidth = width;
  gConstrainHeight = height;
}

function setWidthAndHeight(width, height, event)
{
  if (Number(width) > 0)
  {
    Dg.widthInput.value = width;
    Dg.autoWidthCheck.checked = false;
  }
  else
    Dg.autoWidthCheck.checked = true;
  if (Number(height) > 0)
  {
    Dg.heightInput.value = height;
    Dg.autoHeightCheck.checked = false;
  }
  else
    Dg.autoHeightCheck.checked = true;
  if (Dg.autoHeightCheck.checked && !Dg.autoWidthCheck.checked)
    constrainProportions( "frameWidthInput", "frameHeightInput", event );
  else if (!Dg.autoHeightCheck.checked && Dg.autoWidthCheck.checked)
    constrainProportions( "frameHeightInput", "frameWidthInput", event );
	if (Dg.autoHeightCheck.checked) Dg.heightInput.value = 0;
	if ((Dg.autoWidthCheck.getAttribute("style")!=="visibility: hidden;") && Dg.autoWidthCheck.checked) Dg.widthInput.value = 0;
}

function setContentSize(width, height)  
// width and height are the size of the image in pixels
{
  scaledWidth = Math.round(scale*width);
  if (scaledWidth == 0) scaledWidth = 40;
//  gConstrainWidth = scaledWidth;
  scaledHeight = Math.round(scale*height);
  if (scaledHeight == 0) scaledHeight = 60;
//  gConstrainHeight = scaledHeight;
  setStyleAttributeByID("content", "width", scaledWidth + "px");
  setStyleAttributeByID("content", "height", scaledHeight + "px");
  updateDiagram("margin");
}

function setContentBGColor(color)
{
	setStyleAttributeByID("content", "background-color", color);
}
  
// alignment = 1 for left, 2 for right, 0 for neither
function setAlignment(alignment ) 
{
  position = alignment;
  if (position ==1|| position ==2)
  {
    gFrameTab.marginInput.left.removeAttribute("disabled");
    gFrameTab.marginInput.right.removeAttribute("disabled");
  }
  else
  {
    gFrameTab.marginInput.left.setAttribute("disabled", "true");
    gFrameTab.marginInput.left.setAttribute("value", "0.00");
    gFrameTab.marginInput.right.setAttribute("disabled", "true");
    gFrameTab.marginInput.right.setAttribute("value", "0.00");
  }
}

function setTextValueAttributes()
{
	// copies textbox.value to the value attribute so that persist works.
	var arr1 = ["margin","border","padding"];
	var arr2 = ["Left","Right","Top","Bottom"];
	var arr3 = ["frameWidth","frameHeight"];
	var i,j,k, textbox;
	for (i=0; i< arr1.length; i++)
	{
		for (j=0; j < arr2.length; j++)
		{
			textbox = document.getElementById(arr1[i]+arr2[j]+"Input");
			if (textbox.value != null) textbox.setAttribute("value",textbox.value);
		}
	}
	for (k=0; k < arr3.length; k++)
	{
		textbox = document.getElementById(arr3[k]+"Input");
		if (textbox.value != null) textbox.setAttribute("value",textbox.value);
	}
}

function isValid()
{
  if (!(gFrameTab.widthInput.value > 0))
  {
    AlertWithTitle("Layout error", "Width must be positive");
    return false;
  }
  return true;
}

function setFrameAttributes(frameNode, contentsNode)
{
  var rot;
  var editor = msiGetEditor(Dg.editorElement);
	setTextValueAttributes();
	metrics.unit = frameUnitHandler.currentUnit;
  if (metrics.unit == "px") // switch to pts
  {
    var el = {value: "pt"};
    setNewUnit(el);
    metrics.unit = "pt";
  }
  msiEditorEnsureElementAttribute(frameNode, "units",metrics.unit, editor);
  if (contentsNode) {
    msiEditorEnsureElementAttribute(contentsNode, "units",metrics.unit, editor);
  }
  if (!contentsNode)
    contentsNode = frameNode;

  msiEditorEnsureElementAttribute(contentsNode, "msi_resize","true", editor);
  rot = gFrameTab.rotationList.value;
	if (rot ==="rot0")
	{
    msiEditorEnsureElementAttribute(contentsNode, "rotation", null, editor);  //this will remove the "rotation" attribute
	}
	else
	{
    msiEditorEnsureElementAttribute(contentsNode, "rotation", rot, editor);
	}
  //frameNode.setAttribute("req", "ragged2e");
  msiRequirePackage(gFrameTab.editorElement, "ragged2e", null);
  if (gFrameModeImage) {
    var sidemargin = getSingleMeasurement("margin", "Left", metrics.unit, false);
    msiEditorEnsureElementAttribute(frameNode, "sidemargin", sidemargin, editor);
    var topmargin = getSingleMeasurement("margin", "Top", metrics.unit, false);
    msiEditorEnsureElementAttribute(frameNode, "topmargin", topmargin, editor);
    var overhang = getSingleMeasurement("margin", "Right", metrics.unit, false);
     msiEditorEnsureElementAttribute(frameNode, "overhang", overhang, editor);
		var marginArray = [];
		marginArray[0] = frameUnitHandler.getValueAs(topmargin,"px");
		marginArray[2] = marginArray[0];
		if (position == 1) //left
		{
			if (overhang < 0) marginArray[3] = -frameUnitHandler.getValueAs(overhang,"px");
			else
			{
				marginArray[3] = 0;
			}
			marginArray[1] = frameUnitHandler.getValueAs(sidemargin,"px");
		}
		else if (position == 2) // right
		{
			if (overhang < 0) marginArray[1] = -frameUnitHandler.getValueAs(overhang,"px");
			else
			{
				marginArray[1] = 0;
			}
			marginArray[3] = frameUnitHandler.getValueAs(sidemargin,"px");
		}
		else
		{
			marginArray[1] = marginArray[3] = 0;
		}
		setStyleAttributeOnNode(frameNode, "margin", marginArray.join("px ")+"px", editor);
  }
  else 
	{
    var style = getCompositeMeasurement("margin","px", true);
    setStyleAttributeOnNode(frameNode, "margin", style, editor);
    msiEditorEnsureElementAttribute(frameNode, "margin", getCompositeMeasurement("margin", metrics.unit, false), editor);
	}  
  if (gFrameModeImage) {
    var borderwidth = getSingleMeasurement(borderAtt, "Left", metrics.unit, false);
    msiEditorEnsureElementAttribute(contentsNode, "borderw", borderwidth, editor);
  }
  else {
    msiEditorEnsureElementAttribute(contentsNode, "borderw", getCompositeMeasurement("border",metrics.unit, false), editor);
  }
  if (gFrameModeImage) {
    var padding = getSingleMeasurement("padding", "Left", metrics.unit, false);
    msiEditorEnsureElementAttribute(contentsNode, "padding", padding, editor);
  }
  else{
    msiEditorEnsureElementAttribute(contentsNode, "padding", getCompositeMeasurement("padding",metrics.unit, false), editor);
  }
  if (gFrameTab.autoHeightCheck.checked)
  {
    if (gFrameModeImage){
      msiEditorEnsureElementAttribute(contentsNode, heightAtt, null, editor);
    }
    else{
      msiEditorEnsureElementAttribute(contentsNode, heightAtt, "0", editor);
    }
  }
  else{
    msiEditorEnsureElementAttribute(contentsNode, heightAtt, gFrameTab.heightInput.value, editor);
  }
  if ((gFrameTab.autoWidthCheck.getAttribute("style")!=="visibility: hidden;") && gFrameTab.autoWidthCheck.checked)
  {
    if (gFrameModeImage){
      msiEditorEnsureElementAttribute(contentsNode, widthAtt, null, editor);
    }
    else{
      msiEditorEnsureElementAttribute(contentsNode, widthAtt, "0", editor);
      contentsNode.setAttribute(widthAtt,0);
    }
  }
  else{
    msiEditorEnsureElementAttribute(contentsNode, widthAtt, gFrameTab.widthInput.value, editor);
    contentsNode.setAttribute(widthAtt,gFrameTab.widthInput.value);
  }
  var pos = document.getElementById("placementRadioGroup").selectedItem;
  var posid = pos.getAttribute("id") || "";
  msiEditorEnsureElementAttribute(frameNode, "pos", posid, editor);
  var bgcolor = gFrameTab.colorWell.getAttribute("style");
  var arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
  var theColor = (arr && arr.length > 1) ? arr[1] : "";
  setStyleAttributeOnNode(contentsNode, "border-color", theColor, editor);
  msiEditorEnsureElementAttribute(contentsNode, "border-color", hexcolor(theColor), editor);
  bgcolor = gFrameTab.bgcolorWell.getAttribute("style");
  arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
  theColor = (arr && arr.length > 1) ? arr[1] : "";
  setStyleAttributeOnNode(contentsNode, "background-color", theColor, editor);
  msiEditorEnsureElementAttribute(contentsNode, "background-color", hexcolor(theColor), editor);
	msiRequirePackage(gFrameTab.editorElement, "xcolor", "");
  msiEditorEnsureElementAttribute(frameNode, "textalignment", gFrameTab.textAlignment.value, editor);
	setStyleAttributeOnNode(frameNode, "text-align", gFrameTab.textAlignment.value, editor)
//RWA - The display attribute should be set by a CSS rule rather than on the individual item's style. (So that, for instance,
//      the override for graphics with captions will take effect. See baselatex.css.)
//  if (document.getElementById("inline").selected)
//    setStyleAttributeOnNode(frameNode, "display", "inline-block");
//  else setStyleAttributeOnNode(frameNode, "display", "block");
  // some experimentation here.
  if (posid !=='inline') {
     msiRequirePackage(gFrameTab.editorElement, "boxedminipage", "");
  }
  if (posid === "float")
  {
    var placeLocation="";
    var isHere = false;
    var needsWrapfig = false;
    if (gFrameTab.placeForceHereCheck.checked) {
      placeLocation += "H";
      isHere = true;
    } else if (gFrameTab.placeHereCheck.checked) {
      placeLocation += "h";
      isHere = true;
    } else if (gFrameTab.placeFloatsCheck.checked) {
      placeLocation += "p";
    } else if (gFrameTab.placeTopCheck.checked) { 
      placeLocation += "t";
    } else if (gFrameTab.placeBottomCheck.checked) {
      placeLocation += "b";
    }
    msiEditorEnsureElementAttribute(frameNode, "placeLocation", placeLocation, editor);
    if (isHere)
    {
      var floatparam = document.getElementById("herePlacementRadioGroup").selectedItem.value;
      if (floatparam != "full") {
        msiRequirePackage(gFrameTab.editorElement, "wrapfig","");
      }
      var floatshort = floatparam.slice(0,1);
      msiEditorEnsureElementAttribute(frameNode, "placement",floatshort, editor);
      needsWrapfig = true;
      if (floatparam == "I" || floatparam == "L") floatparam = "left";
      else if (floatparam == "O" || floatparam=="R") floatparam = "right";
			else {
        floatparam = "none";
        needWrapfig = true;
      }
      setStyleAttributeOnNode(frameNode, "float", floatparam, editor);
	    if (floatparam == "right") side = "Right";
	    else if (floatparam == "left") side = "Left";
			else side = null;
	    if (needsWrapfig) {
	      msiEditorEnsureElementAttribute(frameNode, "req", "wrapfig", "");  
	    }
      if (!gFrameModeImage)
	    {
	      if (side){
          msiEditorEnsureElementAttribute(frameNode, "overhang", 0 - getSingleMeasurement("margin", side, metrics.unit, false), editor);
        }
	    }
    }
  }
  else 
  {
    removeStyleAttributeFamilyOnNode(frameNode, "float", editor);
		var fp = document.getElementById("herePlacementRadioGroup").value;
    if (posid == "display" || (posid == "float" && (float==null || float=="full")))
    {
      setStyleAttributeOnNode(frameNode, "margin-left","auto", editor);
      setStyleAttributeOnNode(frameNode, "margin-right","auto", editor);
    }
  }
  // now set measurements in the style for frameNode
  if (gFrameModeImage)
	{
		style = getSingleMeasurement("padding","Left", "px", true);
	  setStyleAttributeOnNode(contentsNode, "padding", style, editor);
	  style = getSingleMeasurement("border","Left","px", true);
	  setStyleAttributeOnNode(contentsNode, "border-width", style, editor);
	}
	else
	{
		style = getCompositeMeasurement("padding","px", true);
	  setStyleAttributeOnNode(contentsNode, "padding", style, editor);
	  style = getCompositeMeasurement("border","px", true);
	  setStyleAttributeOnNode(contentsNode, "border-width", style, editor);
	}
  if (contentsNode.hasAttribute(heightAtt) && Number(contentsNode.getAttribute(heightAtt))!= 0 )
    setStyleAttributeOnNode(contentsNode, "height", frameUnitHandler.getValueAs(contentsNode.getAttribute(heightAtt),"px") + "px", editor);
  else removeStyleAttributeFamilyOnNode(contentsNode, "height", editor);
  if (contentsNode.hasAttribute(widthAtt) && Number(contentsNode.getAttribute(widthAtt))!= 0)
    setStyleAttributeOnNode(contentsNode, "width", frameUnitHandler.getValueAs(contentsNode.getAttribute(widthAtt),"px") + "px", editor);
  else removeStyleAttributeFamilyOnNode(contentsNode, "width", editor);
  if (style != "0px")
    setStyleAttributeOnNode( contentsNode, "border-style", "solid", editor );
}

function frameHeightChanged(input, event)
{
  if (input.value > 0)
  {
     scaledHeight = toPixels(input.value);
  }
  else scaledHeight = scaledHeightDefault;
  setStyleAttributeByID("content", "height", scaledHeight + "px");
  constrainProportions( "frameHeightInput", "frameWidthInput", event );
  redrawDiagram();
  if (input.value == 0)
  {
    if (input.id === "frameHeightInput"){
      Dg.autoHeightCheck.checked = true;
    }
  }
  else
  {
    if (input.id === "frameHeightInput"){
      Dg.autoHeightCheck.checked = false;
    }
  }
}

function frameWidthChanged(input, event)
{
  if (input.value > 0) scaledWidth = toPixels(input.value);
    else scaledWidth = scaledWidthDefault;
  setStyleAttributeByID("content", "width", scaledWidth + "px");
  constrainProportions( "frameWidthInput", "frameHeightInput", event );
  redrawDiagram();
}

function setDisabled(checkbox,id)
{
	var textbox = document.getElementById(id);
	if (checkbox.checked)
	{
		textbox.setAttribute("disabled","true");
	}
	else
	{
		textbox.removeAttribute("disabled");
	}
}

function checkAutoDimens(checkBox, textboxId)
{
  if (checkBox.checked && !Dg.constrainCheckbox.checked){
    Dg.constrainCheckbox.checked = true;
  }
  Dg.frameWidthInput.value = "0";
  setDisabled(this, textboxId);
}

function ToggleConstrain()
{
  // If just turned on, save the current width and height as basis for constrain ratio
  // Thus clicking on/off lets user say "Use these values as aspect ration"
  if (!gFrameModeImage && Dg.constrainCheckbox.checked && !Dg.constrainCheckbox.disabled)
//     && (gDialog.widthUnitsMenulist.selectedIndex == 0)
//     && (gDialog.heightUnitsMenulist.selectedIndex == 0))
  {
    gConstrainWidth = frameUnitHandler.getValueAs(Number(TrimString(Dg.widthInput.value)), "px");
    gConstrainHeight = frameUnitHandler.getValueAs(Number(TrimString(Dg.heightInput.value)), "px");
  }
}

function constrainProportions( srcID, destID, event )
{
  var srcElement = document.getElementById(srcID);
  if (!srcElement)
    return;
  updateTextNumber(srcElement, srcID, event);
  var destElement = document.getElementById(destID);
  if (!destElement)
    return;

  // always force an integer (whether we are constraining or not)
//  forceInteger(srcID);

  if (gFrameModeImage)
  {
    if (gActualWidth && gActualHeight &&
        (Dg.constrainCheckbox.checked && !Dg.constrainCheckbox.disabled))
    {
  //  // double-check that neither width nor height is in percent mode; bail if so!
  //  if ( (gDialog.widthUnitsMenulist.selectedIndex != 0)
  //     || (gDialog.heightUnitsMenulist.selectedIndex != 0) )
  //    return;

    // This always uses the actual width and height ratios
    // which is kind of funky if you change one number without the constrain
    // and then turn constrain on and change a number
    // I prefer the old strategy (below) but I can see some merit to this solution
      if (srcID == "frameWidthInput")
        destElement.value = unitRound( srcElement.value * gActualHeight / gActualWidth );
      else
        destElement.value = unitRound( srcElement.value * gActualWidth / gActualHeight );
    }
  }
  else  //not a graphic - use the other strategy, as there's no natural width
  {
    // With this strategy, the width and height ratio
    //   can be reset to whatever the user entered.
    if (srcID == "frameWidthInput")
      destElement.value = unitRound( srcElement.value * gConstrainHeight / gConstrainWidth );
    else
      destElement.value = unitRound( srcElement.value * gConstrainWidth / gConstrainHeight );
  }
  setContentSize(frameUnitHandler.getValueAs(Dg.widthInput.value,"px"), frameUnitHandler.getValueAs(Dg.heightInput.value,"px"));
}

function doDimensionEnabling()
{
  // Enabled only if "Custom" is selected
  var enable = (Dg.custom.selected);

  // BUG 74145: After input field is disabled,
  //   setting it enabled causes blinking caret to appear
  //   even though focus isn't set to it.
  SetElementEnabledById( "frameHeightInput", enable );
  SetElementEnabledById( "frameHeightLabel", enable );

  SetElementEnabledById( "frameWidthInput", enable );
  SetElementEnabledById( "frameWidthLabel", enable);

  SetElementEnabledById( "unitList", enable );

  var constrainEnable = enable ;
//         && ( gDialog.widthUnitsMenulist.selectedIndex == 0 )
//         && ( gDialog.heightUnitsMenulist.selectedIndex == 0 );

  SetElementEnabledById( "constrainCheckbox", constrainEnable );

}

function setActualSize()
{
  if (gActualWidth && gActualHeight)
  {
    frameTabDlg.widthInput.value = frameUnitHandler.getValueOf(gActualWidth,"px");
    frameTabDlg.heightInput.value = frameUnitHandler.getValueOf(gActualHeight,"px");
  }
  else if (gConstrainWidth && gConstrainHeight)
  {
    frameTabDlg.widthInput.value = frameUnitHandler.getValueOf(gConstrainWidth,"px");
    frameTabDlg.heightInput.value = frameUnitHandler.getValueOf(gConstrainHeight,"px");
  }
//  frameTabDlg.unitList.selectedIndex = 0;
  doDimensionEnabling();
}
