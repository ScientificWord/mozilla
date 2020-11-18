"use strict";
Components.utils.import("resource://app/modules/unitHandler.jsm");


var frameUnitHandler;
var sides;
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
var metrics = {margin:{
    left: 0,
    right: 0,
    top: 0,
    bottom: 0}, 
  border: {
    left: 0,
    right: 0,
    top: 0,
    bottom: 0}, 
  padding: {
    left: 0,
    right: 0,
    top: 0,
    bottom: 0
  }
};
var role;
var gConstrainWidth = 0;
var gConstrainHeight = 0;
var gActualWidth = 0;
var gActualHeight = 0;
var gDefaultPlacement = "inline";
var gDefaultInlineOffset = "";
var hasNaturalSize;
var gFrameModeImage;
var gFrameModeText;
var gCaptionLoc;
var editorElement = msiGetActiveEditorElement();
var editor = msiGetEditor(editorElement);


var gSizeState;

function SizeState(dialog) {
  this.dialog = dialog;
}


SizeState.prototype = {
  dialog: null,
  isCustomSize: false,
  preserveAspectRatio: false,
  width: 0,
  autoWidth: false,
  height: 0,
  autoHeight: false,
  actualSize: {
    width: 0,
    height: 0,
    aspectRatio: true,
    unit: ""
  },
  sizeUnit: null,

  selectActualSize: function() {
    this.isCustomSize = false;
    // this.preserveAspectRatio = false;
    this.width = this.actualSize.width;
    this.height = this.actualSize.height;
    this.update(this.dialog);
  },

  selectCustomSize: function() {
    this.isCustomSize = true;
    this.update(this.dialog);
    if (this.preserveAspectRatio) this.selectAutoHeight();
  },

  setPreserveAspectRatio: function(val) {
    this.preserveAspectRatio = val;
    this.update(this.dialog);
  },

  selectAutoHeight: function() {
    this.update(this.dialog);
  },

  selectAutoWidth: function() {
    this.update(this.dialog);
  },


  applySizes: function() {  
    // now for the values
    if (this.dialog.unitList) this.dialog.unitList.value = this.sizeUnit;
    if (this.dialog.frameWidthInput) {
      this.dialog.frameWidthInput.value = this.width;
    } 
    if (this.dialog.frameHeightInput) this.dialog.frameHeightInput.value =  this.height;
    // if (this.autoWidth) {
    //   this.dialog.autoDims.selectedItem = this.dialog.autoWidth;
    // }
    // else {
    //   if (this.dialog.autoDims) this.dialog.autoDims.selectedItem = this.dialog.autoHeight;
    // }
    if (this.dialog.constrainCheckbox) this.dialog.constrainCheckbox.checked = this.preserveAspectRatio;
    if (this.dialog.sizeRadio) this.dialog.sizeRadio.value = (this.isCustomSize)?"custom":"actual";
    // if (this.dialog.sizeRadio) this.dialog.sizeRadio.value = (this.isCustomSize)?"custom":"actual";
  },

  computeDerivedQuantities: function() {
    if (this.actualSize.width && this.actualSize.height && this.actualSize.width > 0) {
      this.actualSize.aspectRatio = this.actualSize.width/this.actualSize.height;
    }
    else {
      this.actualSize.aspectRatio = null;
    }
  },

  computeInferredDimensions: function() {
    var unitHandler = new UnitHandler(editor);
    if (!this.isCustomSize &&
      this.actualSize.width != null && this.actualSize.height != null)
    {
      unitHandler.initCurrentUnit(this.actualSize.unit);
      if (this.width == 0) this.width = unitHandler.getValueAs(this.actualSize.width, this.sizeUnit);
      if (this.height == 0) this.height = unitHandler.getValueAs(this.actualSize.height, this.sizeUnit);
    }
    if (this.preserveAspectRatio && this.isCustomSize) {
      if (this.autoHeight) {
        this.height = this.width / this.actualSize.aspectRatio;
      }
      if (this.autoWidth && this.actualSize.aspectRatio > 0) {
        this.width = this.height * this.actualSize.aspectRatio;
      }
    }
  },

  computeConstraints: function() {
  },

  update: function() {
    if (!this.dialog)
      return;
    this.computeDerivedQuantities();
    this.computeConstraints();
    this.computeInferredDimensions();
    this.applySizes();
  }
}

function setCanRotate(istrue)
{
  var rotationbox = document.getElementById("rotate");
  if (istrue)
  {
    rotationbox.removeAttribute("hidden");
  }
  else
  {
    rotationbox.setAttribute("hidden", "true");
  }
}

function setFrameSizeFromExisting(dg, wrapperNode)
{
  var border = 0;
  var padding = 0;
  var width;
  var height;
  var actualwidth;
  var actualheight;
  var aspectRatio;
  var objectnode;
  if (!wrapperNode) return;
  try {
 
    gSizeState.sizeUnit = gSizeState.actualSize.unit = wrapperNode.getAttribute("units");
    msidump("gSizeState.sizeUnit: "+gSizeState.sizeUnit);
    gSizeState.width = frameUnitHandler.getValueOf(wrapperNode.getAttribute("width"), gSizeState.sizeUnit);
    msidump("gSizeState.width: "+gSizeState.width);
    gSizeState.height = frameUnitHandler.getValueOf(wrapperNode.getAttribute("height"), gSizeState.sizeUnit);
    msidump("gSizeState.height: "+gSizeState.height);
    gSizeState.preserveAspectRatio = (wrapperNode.getAttribute("aspect") === "true");
    msidump("gSizeState.preserveAspectRatio: "+gSizeState.preserveAspectRatio);
    actualwidth = frameUnitHandler.getValueOf(wrapperNode.getAttribute("naturalwidth"), gSizeState.sizeUnit);
    msidump("naturalwidth: "+actualwidth);
    actualheight = frameUnitHandler.getValueOf(wrapperNode.getAttribute("naturalheight"), gSizeState.sizeUnit);
    msidump("naturalheight: "+actualheight);
    if (Number(actualheight) > 0) aspectRatio = actualwidth/actualheight;
    gSizeState.actualSize.aspectRatio = aspectRatio;
    msidump("gSizeState.actualSize.aspectRatio: "+gSizeState.actualSize.aspectRatio);
    gSizeState.actualSize.width = actualwidth;
    msidump("gSizeState.actualSize.width: "+gSizeState.actualSize.width);
    gSizeState.actualSize.height = actualheight;  
    msidump("gSizeState.actualSize.height: "+gSizeState.actualSize.height);

    gSizeState.update(dg);
  }
  catch(e) {
    msidump(e.message);
  }
}

function updateMetrics()
{
  // this function takes care of the mapping in the cases where there are constraints on margins, padding, etc.
  var sub;
  {
    sub = metrics.margin;
    sub.top    = sub.bottom = Dg.marginInput.top.value;
    switch(position)
    {
      case 0: sub.left = sub.right = 0; // centered
        break;
      case 1: sub.right = Number(Dg.marginInput.left.value);  // on the left
              sub.left  = -Number(Dg.marginInput.right.value);
        break;
      case 2: sub.right = -Number(Dg.marginInput.right.value);
              sub.left  = Number(Dg.marginInput.left.value);
        break;
    }
    metrics.margin = sub;
    sub = metrics.padding;
    sub.top    = sub.right  = sub.bottom = sub.left   = Dg.paddingInput.left.value;
    metrics.padding = sub;
    sub = metrics.border;
    sub.top    = sub.right  = sub.bottom = sub.left   = Dg.borderInput.left.value;
    metrics.border = sub;
    metrics.innermargin = metrics.outermargin = 0;
    gSizeState.sizeUnit = frameUnitHandler.currentUnit;
  }
}

function rescaleMetrics(width, height) // width and height in current unit.
{
  var totalWidth = Number(width) + Number(metrics.margin.left) + Number(metrics.margin.right) + Number(metrics.padding.left) +
    Number(metrics.padding.right) + Number(metrics.border.left) + Number(metrics.border.right);
  var totalHeight = Number(height) + Number(metrics.margin.top) + Number(metrics.margin.bottom) + Number(metrics.padding.top) +
    Number(metrics.padding.bottom) + Number(metrics.border.top) + Number(metrics.border.bottom);
  if (totalWidth === 0 || totalHeight === 0) return;
  var oldScale = scale;
  if (toPixels(totalWidth) > 100) {
    scale = scale * (100/toPixels(totalWidth));
  }
  if (toPixels(totalHeight) > 64) {
    scale = scale * (64/toPixels(totalHeight));
  }
  if (toPixels(totalWidth) <50 && toPixels(totalHeight) < 32)
    scale = Math.min (scale*(100/toPixels(totalWidth)), scale*(64/toPixels(totalHeight)));
  if (scale > 0.25) scale = 0.25;
  if (oldScale != scale) {
    scaledWidth = toPixels(Dg.naturalwidth.value);
    scaledHeight = toPixels(Dg.naturalheight.value);
  }
}

function checkMenuItem( item, checkit ) {
  if (item === null) return;
  if (checkit) {
    item.setAttribute("checked", true);
    document.getElementById("floatlistNone").removeAttribute('checked');
  }
  else item.removeAttribute("checked");
}

function initUnitHandler()
{
  // this is so some functions can use setFrameAttributes without having called initFrameTab.
  frameUnitHandler = new UnitHandler(editor);
}
function posCodeToLocationValue ( posCode ) {
  var ret = '';
  if (posCode === 'inline' || posCode === 'center' || posCode === 'floating'  || posCode === 'left' ||
    posCode === 'right' || posCode === 'inside' || posCode === 'outside' ||
    posCode === "L" || posCode === "R" || posCode === "I" || posCode === "O") ret = posCode;
  else if (posCode === 'display') ret = 'center';
  return ret;
}


function initFrameTab(dg, element, newElement,  contentsElement)
{
  var i, v;
  var len;
  var j;
  var len2;
  var values;
  var width;
  var height;
  var pos;
  var ret;
  var currUnit;
  var isFloat;
//  var currUnit;
  var placement;
  Dg = dg;
  gSizeState = new SizeState(dg);

  initUnitHandler();
  currUnit = element.getAttribute("units");
  frameUnitHandler.initCurrentUnit(currUnit);
  gConstrainHeight = element.getAttribute("aspect"); 
  gConstrainWidth = element.getAttribute("aspect");  
  gFrameModeImage = (element.getAttribute("frametype") === "image");
  gFrameModeText = (element.getAttribute("frametype") === "textframe");

  sides = ["Top", "Right", "Bottom", "Left"]; // do not localize -- visible to code only
  scale = 0.25;
  scaledWidthDefault = 50;
  scaledHeightDefault = 60;
  scaledHeight = scaledHeightDefault;
  scaledWidth = scaledWidthDefault;

// BBM: the following is bogus. gConstrainXXX is a boolean
  if (gFrameModeImage) {
    if (!gConstrainHeight)
      gConstrainHeight = scaledHeightDefault;
    if (!gConstrainWidth)
      gConstrainWidth = scaledWidthDefault;
  }

  pos = element.getAttribute("pos");
  position = 0;  // left = 1, right = 2, display/center = 3, inline = 0
  if (pos === "L" || pos === "I") position = 1;
  else if (pos === "R" || pos === "O") position = 2;
  else if (pos === 'display' || pos === 'center') position = 3;

  dg.editorElement = msiGetParentEditorElementForDialog(window);
  dg.editor = msiGetEditor(dg.editorElement);
  if (!dg.editor) {
    window.close();
    return null;
  }
  // For convenience, map dialog elements to an object
//  currentFrame = element;
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
  dg.frameUnitMenulist    = document.getElementById("frameUnitMenulist"); // ??
  dg.colorWell            = document.getElementById("colorWell");
  dg.bgcolorWell          = document.getElementById("bgcolorWell");
  dg.textAlignment        = document.getElementById("textAlignment");
  dg.rotationList         = document.getElementById("rotationList");
  dg.locationList         = document.getElementById("locationList");
  dg.frameInlineOffsetInput    = document.getElementById("frameInlineOffsetInput");
  dg.floatList            = document.getElementById("floatList");
  dg.OkButton             = document.documentElement.getButton("accept");
  dg.naturalWidth         = document.getElementById( "truewidth" );
  dg.naturalHeight        = document.getElementById( "trueheight" );
  dg.frameHeightInput     = document.getElementById("frameHeightInput");
  dg.frameWidthInput      = document.getElementById("frameWidthInput");
  dg.autoDims             = document.getElementById("autoDims");
  dg.autoHeight           = document.getElementById("autoHeight");
  dg.autoWidth            = document.getElementById("autoWidth");
  dg.captionLocation      = document.getElementById("captionLocation");
  dg.floatlistNone        = document.getElementById("floatlistNone");
  dg.ltxfloat_forceHere   = document.getElementById("ltxfloat_forceHere");
  dg.ltxfloat_here        = document.getElementById("ltxfloat_here");
  dg.ltxfloat_pageOfFloats= document.getElementById("ltxfloat_pageOfFloats");
  dg.ltxfloat_topPage     = document.getElementById("ltxfloat_topPage");
  dg.ltxfloat_bottomPage  = document.getElementById("ltxfloat_bottomPage");

  dg.unitList             = document.getElementById("unitList");          // ??
  dg.sizeRadioGroup       = document.getElementById("sizeRadio");
  dg.actual               = document.getElementById( "actual" );
  dg.custom               = document.getElementById( "custom" );
  dg.constrainCheckbox    = document.getElementById( "constrainCheckbox" );
  dg.sizeRadio            = document.getElementById( "sizeRadio" );
  dg.keyEntryTextbox             = document.getElementById( "keyEntryTextbox" );
  gSizeState.sizeUnit = currUnit;

  var fieldList = [];
  var attrs = ["margin","border","padding"];
  var name;
  var side;
  var b;
  try {
    for (i=0, len = sides.length; i<len; i++)
    {
      side = sides[i].toLowerCase();
      for (j = 0, len2 = attrs.length; j < len2; j++)
      {
        name = attrs[j]+"Input";
        if (dg[name] && dg[name][side])
          fieldList.push(dg[name][side]);
      }
    }
  }
  catch(e) {
    msidump(e.message);
  }
  if (dg.frameHeightInput) fieldList.push(dg.frameHeightInput);
  if (dg.frameWidthInput) fieldList.push(dg.frameWidthInput);
  if (dg.frameInlineOffsetInput) fieldList.push(dg.frameInlineOffsetInput);
  if (dg.naturalwidth) fieldList.push(dg.naturalwidth);
  if (dg.naturalheight) fieldList.push(dg.naturalheight);
  frameUnitHandler.setEditFieldList(fieldList);
// The defaults for the image frame are set by the input frame, or the one just generated using the
// preferences.
  pos = element.getAttribute("pos");
  if (pos != null) ret = posCodeToLocationValue(pos);
  if (pos != null && ret.length > 0 && dg.locationList) dg.locationList.selectedItem = document.getElementById(ret);
  if ((pos === "inline") && element.hasAttribute("inlineOffset"))
    inlineOffset = element.getAttribute("inlineOffset");
  isFloat = (pos != null) && pos === 'floating';
  v = element.getAttribute("ltxfloat");
  if (isFloat && v && v.length > 0) {
    b = (v.search("H") >= 0);
    checkMenuItem(document.getElementById("ltxfloat_forceHere"), b);
    b = (v.search("h") >= 0);
    checkMenuItem(document.getElementById("ltxfloat_here"), b);
    b = (v.search("p") >= 0);
    checkMenuItem(document.getElementById("ltxfloat_pageOfFloats"), b);
    b = (v.search("t") >= 0);
    checkMenuItem(document.getElementById("ltxfloat_topPage"), b);
    b = (v.search("b") >= 0);
    checkMenuItem(document.getElementById("ltxfloat_bottomPage"), b);
  }
  if (!isFloat) checkMenuItem(document.getElementById("floatlistNone"), true);
  v =  element.getAttribute("borderw");
  if (v != null && dg.borderInput && dg.borderInput.left && dg.borderInput.right && dg.borderInput.top && dg.borderInput.bottom)
    dg.borderInput.left.value = dg.borderInput.right.value = dg.borderInput.bottom.value = dg.borderInput.top.value = v;
  v =  element.getAttribute("sidemargin");
  if (v != null && dg.marginInput) {
    if (dg.marginInput.left) dg.marginInput.left.value = v;
    if (dg.marginInput.right) dg.marginInput.right.value = v;
  }
  v =  element.getAttribute("topmargin");
  if (v != null && dg.marginInput && dg.marginInput.top && dg.marginInput.bottom) dg.marginInput.top.value = dg.marginInput.bottom.value = v;
  v =  element.getAttribute("padding");
  if (v != null && dg.paddingInput && dg.paddingInput.left && dg.paddingInput.right) dg.paddingInput.left.value = dg.paddingInput.right.value = dg.paddingInput.top.value = dg.paddingInput.bottom .value = v;
  v =  element.getAttribute("background-color");
  if (v != null && dg.bgcolorWell) dg.bgcolorWell.setAttribute("style","background-color: " + v + ";");
  v =  element.getAttribute("border-color");
  if (v != null && dg.colorWell) dg.colorWell.setAttribute("style","background-color: " + v + ";");

  var floatLocation, posStr;
  var inlineOffset = 0;
  try {
      if (! contentsElement)
         contentsElement = element;
      setFrameSizeFromExisting(dg, element);
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

      var overhang = element.getAttribute("overhang");
      if (overhang === null) overhang = 0;
      // ??
      dg.marginInput.right.value = overhang;

      var theColor;
      if (element.hasAttribute("border-color"))
      {
        theColor = hexcolor(element.getAttribute("border-color"));
        setColorInDialog("colorWell", theColor);
      }
      if (element.hasAttribute("background-color"))
      {
        theColor = hexcolor(element.getAttribute("background-color"));
        setColorInDialog("bgcolorWell", theColor);
      }
      var key = "";
      var captionNodes = element.getElementsByTagName("imagecaption");
      var captionNode;
      if (captionNodes && captionNodes.length > 0) captionNode = captionNodes[0];
      initKeyEntryOverlay(dg.editor, captionNode)
      if (captionNode) {
        if (captionNode.hasAttribute("key"))
          key = captionNode.getAttribute("key");
        else if (captionNode.hasAttribute("id"))
          key = captionNode.getAttribute("id");
        dg.keyEntryTextbox.value = key;
        gOriginalKey = key;
      }
      var captionLoc = element.getAttribute("captionloc") || "none";
      dg.captionLocation.value = captionLoc;
      gCaptionLoc = captionLoc;
    }

  catch(e) {
    dump(e.message);
  }

  try {
    setAlignment(placement);

  // The following will figure out what is enabled in the current state.
    locationChanged();
    floatPropertyChanged();
    captionPropertyChanged();
    doDimensionEnabling();
    updateMetrics();
    updateDiagram(marginAtt);
    updateDiagram(borderAtt);
    updateDiagram(paddingAtt);
    var broadcaster = document.getElementById("role-image");
    var hiddenAttr = broadcaster.getAttribute("hidden");
    if ( hiddenAttr === true )
      role = "textframe";
    else role = "image"; 
    return dg;
  }
  catch(e) {
    msidump(e.message);
  }
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
  if (anId.length === 0) return;
  var regexp=/(.*)(Left|Right|Top|Bottom)(.*)/;
  var result = regexp.exec(anId);
  if (result[1] == "border" || result[1] == "padding")
  {
    extendInput( result[1] );
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
  return Math.round(0.4999 + frameUnitHandler.getValueAs(x,"px")*scale);
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
  // User canceled the dialog
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
    if (document.getElementById(attribute + sides[i] + "Input"))
      values.push( Math.max(0,frameUnitHandler.getValueAs(Number(document.getElementById(attribute + sides[i] + "Input").value ),unit)));
    else values.push(0);
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
  if (document.getElementById(attribute + sides[i] + "Input")) {
    value = Math.max(0,frameUnitHandler.getValueAs(Number(document.getElementById(attribute + sides[i] + "Input").value ),unit));
    if (showUnit) value = value + unit;
  }
  return value;
}

//attribute = margin, border, padding;
function updateDiagram( attribute )
{
  var i;
  var values = [];
  updateMetrics();
  rescaleMetrics(Dg.naturalWidth.value,Dg.naturalHeight.value);
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
  var bgcolor = Dg.bgcolorWell.getAttribute("style");
  var arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
  if (attribute=="border")  // add border color and border width
  {
    removeStyleAttributeFamilyOnNode(document.getElementById("frame"), "border");
    var style = document.getElementById("frame").getAttribute("style");
    style += " border-width: "+val+"; border-color: " + arr[1]+"; border-style: solid;";
    document.getElementById("frame").setAttribute("style", style);
  }
  else
  {
    setStyleAttributeByID("frame", attribute, val );
  }
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


// called when one of these is clicked: inline, display, float

function locationChanged()
{
  var floatBroadcaster = document.getElementById("floatEnabled");
  var inlineOffsetBroadcaster = document.getElementById("inlineOffsetEnabled");
  var bEnableWrapfig = true;
  var bEnableFloats = false;
  var currentLocation = document.getElementById("locationList").value;
  if (floatBroadcaster) {
    if (currentLocation === "floating") {
      if (document.getElementById("captionLocation").value !=='none')
        document.getElementById("keyEnabled").removeAttribute("disabled");
      floatBroadcaster.removeAttribute("disabled");
      if (Dg.floatList.selectedItem === "floatlistNone")
        Dg.floatList.selectedItem = "ltxfloat_here";
    } else {
      floatBroadcaster.setAttribute("disabled", "true");
      document.getElementById("keyEnabled").setAttribute("disabled", "true");
    }
  }
  if (currentLocation === "inline")
    inlineOffsetBroadcaster.removeAttribute("disabled");
  else
    inlineOffsetBroadcaster.setAttribute("disabled", "true");
}

function enableFloatOptions(radiogroup)
{
  return;
}

function enableFloating( )
{
  return;
}

/************************************/
function handleChar(event, id)
{
  var element = event.originalTarget;
  if ((event.keyCode != event.DOM_VK_UP) && (event.keyCode != event.DOM_VK_DOWN))
    updateTextNumber(element, id, event);
  update(id);
  if ((event.keyCode === event.DOM_VK_UP) || (event.keyCode === event.DOM_VK_DOWN))
    event.preventDefault();
}

function geomHandleChar(event, id, tbid)
{
  handleChar(event, id, tbid);
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
  gSizeState.actualSize.width = width;
  gSizeState.actualSize.height = height;
  gSizeState.computeDerivedQuantities();
}

function setWidthAndHeight(width, height, event)
{
  gSizeState.width = width;
  gSizeState.height = height;
  gSizeState.computeConstraints();
}

function setContentSize(width, height)
// width and height are the size of the image in pixels
{
  scaledWidth = Math.round(scale*width);
  if (scaledWidth === 0) scaledWidth = 40;
//  gConstrainWidth = scaledWidth;
  scaledHeight = Math.round(scale*height);
  if (scaledHeight === 0) scaledHeight = 60;
//  gConstrainHeight = scaledHeight;
  setStyleAttributeByID("content", "width", scaledWidth + "px");
  setStyleAttributeByID("content", "height", scaledHeight + "px");
  updateDiagram("margin");
}

function setContentBGColor(color)
{
  setStyleAttributeByID("content", "background-color", color);
}

// alignment = 1 for left, 2 for right, 3 for display/center, 0 for inline
function setAlignment(alignment )
{
  position = alignment;
  if (position ==1 || position ==2)
  {
    Dg.marginInput.left.removeAttribute("disabled");
    Dg.marginInput.right.removeAttribute("disabled");
  }
  else if (position == 3)
  {
    Dg.marginInput.left.setAttribute("disabled", "true");
    Dg.marginInput.left.setAttribute("value", "0.00");
    Dg.marginInput.right.setAttribute("disabled", "true");
    Dg.marginInput.right.setAttribute("value", "0.00");
  }
  else 
  {
    Dg.marginInput.left.removeAttribute("disabled");
    Dg.marginInput.right.removeAttribute("disabled");
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
      if (textbox && textbox.value != null) textbox.setAttribute("value",textbox.value);
    }
  }
  for (k=0; k < arr3.length; k++)
  {
    textbox = document.getElementById(arr3[k]+"Input");
    if (!textbox) { // these input fields don't exist, so our work here is donw.
      return false;
    }
    if (textbox && textbox.value != null) textbox.setAttribute("value",textbox.value);
  }
  return true;
}

function isValid()
{
  return true;
}

function isEnabled(element)
{
  if (!element) return false;
  return(!element.hasAttribute("disabled"));
}

/*If there is a frame around the contents, then frameNode is the msiFrame.
If there is no frame, then frameNode == contentsNode, and attributes set on either one will
be set on contentsNode.
*/
function setFrameAttributes(frameNode, contentsNode, editor, dimsonly)
/*when dimsonly, put only the dimensions in the style attribute of contentsNode.
this is the case for images in an msiframe
*/
{
  var rotation;
  var frametype;
  var w, h;
  var rowCountObj = { value: 0 };
  var colCountObj = { value: 0 };
  var editorElement;
  var widthInPx;
  var heightInPx;

  try {  
    if (editor == null) {
      editorElement = msiGetParentEditorElementForDialog(window);
      editor = msiGetEditor(editorElement);
    }

    msiEditorEnsureElementAttribute(frameNode, "units",gSizeState.sizeUnit, editor);

    msiEditorEnsureElementAttribute(contentsNode, "msi_resize","true", editor);
    if (Dg.rotationList) {
      rotation = Dg.rotationList.value;
      if (rotation ==="rot0")
      {
        msiEditorEnsureElementAttribute(contentsNode, "rotation", null, editor);
         //this will remove the "rotation" attribute
      }
      else
      {
        msiEditorEnsureElementAttribute(contentsNode, "rotation", rotation, editor);
      }
    }
    msiRequirePackage(Dg.editorElement, "ragged2e", null);

    var inlineOffsetNum = getInlineOffset("px");  //returns 0 unless inline position is chosen!
    var sidemargin = getSingleMeasurement("margin", "Left", gSizeState.sizeUnit, false);
    msiEditorEnsureElementAttribute(frameNode, "sidemargin", sidemargin, editor);
    var topmargin = getSingleMeasurement("margin", "Top", gSizeState.sizeUnit, false);
    msiEditorEnsureElementAttribute(frameNode, "topmargin", topmargin, editor);
    var overhang = getSingleMeasurement("margin", "Right", gSizeState.sizeUnit, false);
     msiEditorEnsureElementAttribute(frameNode, "overhang", overhang, editor);
    var marginArray = [];
    // marginArray[0] = frameUnitHandler.getValueAs(topmargin,"px");
    marginArray[0] = topmargin;
    marginArray[2] = marginArray[0];
    if (inlineOffsetNum > 0)
      marginArray[2] += inlineOffsetNum;
    if (position == 1) //left
    {
      if (overhang < 0) marginArray[3] = overhang;
      else
      {
        marginArray[3] = 0;
      }
      marginArray[1] = sidemargin;
    }
    else if (position == 2) // right
    {
      if (overhang < 0) marginArray[1] = -overhang;
      else
      {
        marginArray[1] = 0;
      }
      marginArray[3] = sidemargin;
    }
    else
    {
      marginArray[1] = marginArray[3] = 0;
    }
    if (gFrameModeImage || gFrameModeText) {
      var borderwidth = getSingleMeasurement(borderAtt, "Left", gSizeState.sizeUnit, false);
      msiEditorEnsureElementAttribute(frameNode, "borderw", borderwidth, editor);
    }
    else {
      msiEditorEnsureElementAttribute(frameNode, "borderw", getCompositeMeasurement("border",gSizeState.sizeUnit, false), editor);
    }
    if (gFrameModeImage || gFrameModeText) {
      var padding = getSingleMeasurement("padding", "Left", gSizeState.sizeUnit, false);
      msiEditorEnsureElementAttribute(frameNode, "padding", padding, editor);
    }
    else {
      msiEditorEnsureElementAttribute(frameNode, "padding", getCompositeMeasurement("padding",gSizeState.sizeUnit, false), editor);
    }

    var captionLoc = document.getElementById("captionLocation").value;
    if (captionLoc === "none") {
      frameNode.removeAttribute("captionloc");
      // removeStyleAttributeFamilyOnNode(frameNode, "caption-side");
    } else {
      frameNode.setAttribute("captionloc", captionLoc);
      // setStyleAttributeOnNode(frameNode, "caption-side", captionLoc);
    }

    var posItem = null;
    var posid;
    if (isEnabled(document.getElementById("locationList")))
    {
      if (document.getElementById("locationList"))
        posItem = document.getElementById("locationList").selectedItem;
      posid = (posItem && posItem.getAttribute("id")) || "";
      msiEditorEnsureElementAttribute(frameNode, "pos", posid, editor);
    }

    var bgcolor = Dg.colorWell.getAttribute("style");
    var arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
    var theColor = (arr && arr.length > 1) ? arr[1] : "";
    // setStyleAttributeOnNode(frameNode, "border-color", theColor, editor);
    msiEditorEnsureElementAttribute(frameNode, "border-color", hexcolor(theColor), editor);
    bgcolor = Dg.bgcolorWell.getAttribute("style");
    arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
    theColor = (arr && arr.length > 1) ? arr[1] : "";
    // setStyleAttributeOnNode(frameNode, "background-color", theColor, editor);
    msiEditorEnsureElementAttribute(frameNode, "background-color", hexcolor(theColor), editor);
    msiRequirePackage(Dg.editorElement, "xcolor", "");
    msiEditorEnsureElementAttribute(frameNode, "textalignment", Dg.textAlignment.value, editor);
    // currently we don't need boxedminipage for display (centeredfw)
    if (posid && posid !== "inline") {
       msiRequirePackage(Dg.editorElement, "boxedminipage", "");
    }
    else
    {
      if (isEnabled(Dg.frameInlineOffsetInput) && inlineOffsetNum !== 0)
      {
        inlineOffset = Number(Dg.frameInlineOffsetInput.value);
        var inlineOffsetStr = String(inlineOffset);
        msiEditorEnsureElementAttribute(frameNode, "inlineOffset", inlineOffsetStr, editor);
        inlineOffsetStr = String(-inlineOffset) + frameUnitHandler.currentUnit;
      }
      else
      {
        msiEditorEnsureElementAttribute(frameNode, "inlineOffset", null, editor);
      }
    }
    var needsWrapfig = false;
    if (isEnabled(document.getElementById("floatList")) && document.getElementById("floatList").value !== "")
    {
      var floatPosition="";
      var isHere = false;
      needsWrapfig = false;
      if (Dg.ltxfloat_forceHere && Dg.ltxfloat_forceHere.hasAttribute("checked")) {
        floatPosition += "H";
        isHere = true;
      }
      if (Dg.ltxfloat_here && Dg.ltxfloat_here.hasAttribute("checked")) {
        floatPosition += "h";
        isHere = true;
      }
      if (Dg.ltxfloat_pageOfFloats && Dg.ltxfloat_pageOfFloats.hasAttribute("checked")) {
        floatPosition += "p";
      }
      if (Dg.ltxfloat_topPage &&  Dg.ltxfloat_topPage.hasAttribute("checked")) {
        floatPosition += "t";
      }
      if (Dg.ltxfloat_bottomPage && Dg.ltxfloat_bottomPage.hasAttribute("checked")) {
        floatPosition += "b";
      }
      msiEditorEnsureElementAttribute(frameNode, "ltxfloat", floatPosition, editor);
      if (floatPosition.length > 0) {
        msiRequirePackage(Dg.editorElement, "float","");
      }
    } else
      frameNode.removeAttribute("ltxfloat");
    if (isEnabled(Dg.locationList))
    {
      var locationParam = Dg.locationList.value;
      if ((locationParam === "inside") ||
          (locationParam === "outside") ||
          (locationParam === "left") ||
          (locationParam === "right") ||
		  (locationParam === "I") ||
		  (locationParam === "O") ||
		  (locationParam === "L") ||
		  (locationParam === "R") )
      {
        // (gp) Changed to not need additional conditional
        needsWrapfig = true;
        msiRequirePackage(Dg.editorElement, "wrapfig","");
        msiEditorEnsureElementAttribute(frameNode, "req", "wrapfig", "");
      }
    }
    frameNode.setAttribute("width", gSizeState.width);
    frameNode.setAttribute("height", gSizeState.height);
    widthInPx = Math.round(frameUnitHandler.getValueAs(gSizeState.width,'px'));
    heightInPx = Math.round(frameUnitHandler.getValueAs(gSizeState.height,'px'));
    contentsNode.setAttribute('width', widthInPx);
    contentsNode.setAttribute('height', heightInPx);
    contentsNode.setAttribute("aspect", gSizeState.preserveAspectRatio ? "true" : "false");
    frameNode.setAttribute("aspect", gSizeState.preserveAspectRatio ? "true" : "false");
    editor.getFrameStyleFromAttributes(frameNode);
    setStyleAttributeOnNode(contentsNode, 'width', widthInPx + 'px', editor);
    setStyleAttributeOnNode(contentsNode, 'height', heightInPx + 'px', editor);
  }
  catch(e) {
    dump(e.message);
  }
}

// When the height changes and the aspect ratio is preserved, we have to change the width, and vice versa. The next two variables prevent potentially endless
// cycling.

var changingHeight = false;
var changingWidth = false;

function approxEqual(a, b) { // a and b are measurements in the current units. They are approximately equal if |a-b| < 2 pixels.
  return ((Math.abs(Number(frameUnitHandler.getValueAs(a, 'px') - Number(frameUnitHandler.getValueAs(b, 'px'))))) < 2);
}

function setActualSizeFromSizes(gSizeState) {
  if (approxEqual(gSizeState.width, gSizeState.actualSize.width) && approxEqual(gSizeState.height, gSizeState.actualSize.height)) {
    setActualSize(true);
  } else {
    setActualSize(false);
  }
}


function frameHeightChanged(input, event)
{
  var newWidth;
  if (changingWidth) return;
  changingHeight = true;
  if (input.value > 0)
  {
     scaledHeight = toPixels(input.value);
  }
  else scaledHeight = scaledHeightDefault;
  setStyleAttributeByID("content", "height", scaledHeight + "px");
  gSizeState.height = input.value;
  if (gSizeState.preserveAspectRatio) {
    if (gSizeState.actualSize.aspectRatio > 0) {
      newWidth = gSizeState.height*gSizeState.actualSize.aspectRatio;
      if (newWidth > 0) {
        scaledWidth = toPixels(newWidth);
      } else scaledWidth = scaledWidthDefault;
      setStyleAttributeByID("content", "width", scaledWidth + "px");
      gSizeState.width = newWidth;
    }
  }
  setActualSizeFromSizes(gSizeState);
  gSizeState.update();
  redrawDiagram();
  changingHeight = false;
}

function frameWidthChanged(input, event)
{
  var newHeight;
  if (changingHeight) return;
  changingWidth = true;
  if (input.value > 0) scaledWidth = toPixels(input.value);
    else scaledWidth = scaledWidthDefault;
  setStyleAttributeByID("content", "width", scaledWidth + "px");
  gSizeState.width = input.value;
  if (gSizeState.preserveAspectRatio) {
    if (gSizeState.actualSize.aspectRatio > 0) {
      newHeight = gSizeState.width/gSizeState.actualSize.aspectRatio;
      if (newHeight > 0) {
        scaledHeight = toPixels(newHeight);
      } else scaledHeight = scaledHeightDefault;
      setStyleAttributeByID("content", "height", scaledHeight + "px");
      gSizeState.height = newHeight;
    }
  }
  setActualSizeFromSizes(gSizeState);
  gSizeState.update(Dg);
  redrawDiagram();
  changingWidth = false;
}

function getInlineOffset(whichUnit)
{
  if (!whichUnit)
    whichUnit = frameUnitHandler.currentUnit;
  if (Dg.locationList.value === "inline")
    return Dg.frameInlineOffsetInput.value;
  return 0;
}

function setDisabled(checkbox,id)
{
  return;
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

function checkAutoDimens(radio, textboxId)
{
  gSizeState.update(Dg);
}

function ToggleConstrain()
{
  // If just turned on, save the current width and height as basis for constrain ratio
  gSizeState.preserveAspectRatio = !gSizeState.preserveAspectRatio;
  gSizeState.update(Dg);
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

  if (gFrameModeImage)
  {
    if (gActualWidth && gActualHeight && Dg.constrainCheckbox &&
        ((Dg.constrainCheckbox.checked && !Dg.constrainCheckbox.disabled) || destElement.hasAttribute("disabled")))
    {
    // This always uses the actual width and height ratios
    // which is kind of funky if you change one number without the constrain
    // and then turn constrain on and change a number
    // I prefer the old strategy (below) but I can see some merit to this solution
      if (srcID === "frameWidthInput")
        destElement.value = unitRound( srcElement.value * gActualHeight / gActualWidth );
      else
        destElement.value = unitRound( srcElement.value * gActualWidth / gActualHeight );
    }
  }
  else  if (gConstrainWidth>0 && gConstrainHeight>0)
    //not a graphic - use the other strategy, as there's no natural width
  {
    // With this strategy, the width and height ratio
    //   can be reset to whatever the user entered.
    // if (srcID === "frameWidthInput")
    //   destElement.value = unitRound( srcElement.value * gConstrainHeight / gConstrainWidth );
    // else
    //   destElement.value = unitRound( srcElement.value * gConstrainWidth / gConstrainHeight );
  }
  setContentSize(Dg.frameWidthInput.value, Dg.frameHeightInput.value);
}

function doDimensionEnabling()
{
    // Enabled only if "Custom" is selected, or actualsize is not enabled
    try{
	// jcs - do we ever have a "custom" element?
	var enable = (document.getElementById("custom") &&
		      document.getElementById("custom").selected) || !hasNaturalSize;
	if (enable) {
	    document.getElementById("customSize").removeAttribute("disabled");
	}
	else
	    document.getElementById("customSize").setAttribute("disabled", "true");
	
	SetElementEnabledById( "unitList", enable );
	var constrainEnable = enable;
	SetElementEnabledById( "constrainCheckbox", constrainEnable );
    }catch(e){
	msidump(e.message);
    }

}

function setActualSize(val)
{
  gSizeState.isCustomSize = !val;
  gSizeState.update(Dg);
}


function floatPropertyChanged() {
  var floatList = document.getElementById("floatList");
  var none = floatList.value === "";
  var menuitems = floatList.getElementsByTagName("menuitem");
  var i;
  var item;
  if (none) {
    for (i = 0; i < menuitems.length; i++)
    {
      item = menuitems.item(i);
      if (item.value !== "") {
        item.removeAttribute("checked");
      }
    }
    document.getElementById("locationEnabled").removeAttribute("disabled");
  }
  else {
    document.getElementById("floatlistNone").removeAttribute("checked");
  }
}

function captionPropertyChanged() {
  var captionLoc = document.getElementById("captionLocation").value;
//  if (tagConflicts()) return;
  if (captionLoc === "none") {
    document.getElementById("keyEnabled").setAttribute("disabled", "true");
  } else {
    document.getElementById("keyEnabled").removeAttribute("disabled");
  }
}
