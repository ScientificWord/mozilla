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
var widthAtt = "ltx_width";
var heightAtt = "ltx_height";
var metrics = {margin:{}, border: {}, padding: {}};
var role;
var gConstrainWidth = 0;
var gConstrainHeight = 0;
var gActualWidth = 0;
var gActualHeight = 0;
var gDefaultPlacement = "center";
var gDefaultInlineOffset = "";
var hasNaturalSize;
var gCaptionLoc;

var sizeState = null;

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
  enabledState: {
    isActualSize: true,
    preserveAspectRatio: true,
    width: true,
    autoWidth: false,
    height: true,
    autoHeight: false
  },
  actualSize: {
    width: null,
    height: null,
    aspectRatio: null,
    unit: ''
  },
  sizeUnit: null,

  selectActualSize: function() {
    var unitHandler = new UnitHandler();
    unitHandler.initCurrentUnit('px');
    this.isCustomSize = false;
    this.preserveAspectRatio = false;
    this.width = unitHandler.getValueAs(this.actualSize.width, this.sizeUnit);
    this.height = unitHandler.getValueAs(this.actualSize.height, this.sizeUnit);
    this.update(this.dialog);
  },

  selectCustomSize: function() {
    this.isCustomSize = true;
    this.update(this.dialog);
  },

  setPreserveAspectRatio: function(val) {
    this.preserveAspectRatio = val;
    if (val) {
      this.enabledState.autoWidth = this.enabledState.autoHeight = true;
      this.autoWidth = this.dialog.autoWidth.selected;
      this.autoHeight = this.dialog.autoHeight.selected;
      this.enabledState.width = !this.autoWidth;
      this.enabledState.height = !this.autoHeight;
    }
    else {
      this.enabledState.autoWidth = this.enabledState.autoHeight = false;
      this.enabledState.width = this.enabledState.height = true;
    }
    this.update(this.dialog);
  },

  selectAutoHeight: function() {
    this.autoHeight = true;
    this.autoWidth = false;
    this.enabledState.width = true;
    this.enabledState.height = false;
    this.update(this.dialog);
  },

  selectAutoWidth: function() {
    this.autoHeight = false;
    this.autoWidth = true;
    this.enabledState.width = false;
    this.enabledState.height = true;
    this.update(this.dialog);
  },


  applySizes: function() {  // dg is a dialog
    this.dialog.frameWidthInput.disabled = !this.enabledState.width;
    this.dialog.frameHeightInput.disabled = !this.enabledState.height;
    this.dialog.autoWidth.disabled = !this.enabledState.autoWidth;
    this.dialog.autoHeight.disabled = !this.enabledState.autoHeight;
    this.dialog.constrainCheckbox.disabled = !this.enabledState.preserveAspectRatio;
    //this.dialog.actual.disabled = !this.enabledState.isActualSize;

    // now for the values
    this.dialog.unitList.value = this.sizeUnit;
    this.dialog.frameWidthInput.value = this.width;
    this.dialog.frameHeightInput.value = this.height;
    if (this.autoWidth) {
      this.dialog.autoDims.selectedItem = this.dialog.autoWidth;
    }
    else {
      this.dialog.autoDims.selectedItem = this.dialog.autoHeight;
    }
    this.dialog.constrainCheckbox.checked = this.preserveAspectRatio;
    this.dialog.sizeRadio.value = this.isCustomSize?"custom":"actual";
  },

  computeDerivedQuantities: function() {
    if (this.actualSize.width && this.actualSize.height && this.actualSize.width > 0) {
      this.actualSize.aspectRatio = this.actualSize.height/this.actualSize.width;
    }
    else {
      this.actualSize.aspectRatio = null;
    }
  },

  computeInferredDimensions: function() {
    var unitHandler = new UnitHandler();
    if (!this.isCustomSize &&
      this.actualSize.width != null && this.actualSize.height != null)
    {
      unitHandler.initCurrentUnit(this.actualSize.unit);
      this.width = unitHandler.getValueAs(this.actualSize.width, this.sizeUnit);
      this.height = unitHandler.getValueAs(this.actualSize.height, this.sizeUnit);
      this.enabledState.height = this.enabledState.width = false;
    }
    if (this.preserveAspectRatio && this.isCustomSize) {
      if (this.autoHeight) {
        this.height = this.width * this.actualSize.aspectRatio;
      }
      if (this.autoWidth && this.actualSize.aspectRatio > 0) {
        this.width = this.height/this.actualSize.aspectRatio;
      }
    }
  },

  computeConstraints: function() {
    this.enabledState.preserveAspectRatio = (this.actualSize.aspectRatio != null) &&
      this.isCustomSize;
    this.enabledState.autoWidth = this.enabledState.autoHeight = (this.actualSize.aspectRatio != null) &&
      this.preserveAspectRatio;
    this.enabledState.height = (this.isCustomSize) && (!this.enabledState.autoHeight
      || !this.autoHeight);
    this.enabledState.width = (this.isCustomSize) && (!this.enabledState.autoWidth
      || !this.autoWidth);
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


function setHasNaturalSize()
// images frequently have a natural size
{
  var bcaster = document.getElementById("hasNaturalSize");
  if (!bcaster) {
    hasNaturalSize = false;
    return;
  }
  hasNaturalSize = (sizeState.actualSize.width != null) && (sizeState.actualSize.height != null);
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

function setFrameSizeFromExisting(dg, wrapperNode, contentsNode)
{
  var border = 0, padding = 0;
  var node;
  var graphspec;
  var unitHandler;
  if (!wrapperNode) return;
  // Cope with different capitalization in plots.
  if (wrapperNode.nodeName === "msiframe") {
    sizeState.width = wrapperNode.getAttribute(widthAtt);
    sizeState.height = wrapperNode.getAttribute(heightAtt);
  }
  sizeState.sizeUnit = contentsNode.getAttribute("units") || wrapperNode.getAttribute("units") || wrapperNode.getAttribute("Units");

  if (wrapperNode.parentNode.nodeName ==='graph') {
    graphspec = wrapperNode.parentNode.firstChild;
    sizeState.actualSize.width = graphspec.getAttribute('Width');
    sizeState.actualSize.height = graphspec.getAttribute('Height');
    sizeState.actualSize.unit = graphspec.getAttribute('Units');
    unitHandler = new UnitHandler;
    unitHandler.initCurrentUnit(sizeState.actualSize.unit);
    sizeState.width = unitHandler.getValueAs(sizeState.actualSize.width, sizeState.sizeUnit);
    sizeState.height = unitHandler.getValueAs(sizeState.actualSize.height, sizeState.sizeUnit);
  }
  else {
    if (!sizeState.width) {
      width = contentsNode.getAttribute(widthAtt) ||
        wrapperNode.getAttribute("width") || wrapperNode.getAttribute("Width") || wrapperNode.getAttribute(widthAtt);
    }
    if (!sizeState.height) {
      height= contentsNode.getAttribute(heightAtt) ||
        wrapperNode.getAttribute("height") || wrapperNode.getAttribute("Height") || wrapperNode.getAttribute(heightAtt);
    }

    // For existing nodes, we have the actual sizes in the document.
    if (!sizeState.actualSize.width) {
      sizeState.actualSize.width = contentsNode.getAttribute('naturalWidth') || wrapperNode.getAttribute('natualWidth');
    }
    if (!sizeState.actualSize.height) {
      sizeState.actualSize.height = contentsNode.getAttribute('naturalHeight') || wrapperNode.getAttribute('natualHeight');
    }
  }
  sizeState.update(dg);
  setHasNaturalSize();
}

function updateMetrics()
{
  // this function takes care of the mapping in the cases where there are constraints on margins, padding, etc.
  var sub;
//  if (gFrameModeImage)
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
    metrics.unit = frameUnitHandler.currentUnit;
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
    scaledWidth = toPixels(Dg.truewidth.value);
    scaledHeight = toPixels(Dg.trueheight.value);
  }
}

function checkMenuItem( item, checkit ) {
  if (item === null) return;
  if (checkit) {
    item.setAttribute("checked", true);
  }
  else item.removeAttribute("checked");
}

function initUnitHandler()
{
  // this is so some functions can use setFrameAttributes without having called initFrameTab.
  frameUnitHandler = new UnitHandler();
}

function initFrameTab(dg, element, newElement,  contentsElement)
{
  var i, v;
  var len;
  var j;
  var len2;
  var values;
  var width = 0;
  var prefUnit;
  var currUnit;
  var placement;
  Dg = dg;
  initUnitHandler();
  var prefBranch = GetPrefs();
  var isFloat = false;
  var prefprefix;
  if (contentsElement && contentsElement.getAttribute("type") && contentsElement.getAttribute("type").indexOf("mupad") >= 0)
    prefprefix = "swp.graph.";
  else
    prefprefix = "swp.graphics.";

  prefUnit = prefBranch.getCharPref(prefprefix + "units");
  if (newElement) {
    currUnit = prefUnit;
  }
  else {
    currUnit = element.getAttribute("units");
  }
  frameUnitHandler.initCurrentUnit(currUnit);

  sides = ["Top", "Right", "Bottom", "Left"]; // do not localize -- visible to code only
  scale = 0.25;
  scaledWidthDefault = 50;
  scaledHeightDefault = 60;
  scaledHeight = scaledHeightDefault;
  scaledWidth = scaledWidthDefault;

  if (gFrameModeImage) {
    if (!gConstrainHeight)
      gConstrainHeight = scaledHeightDefault;
    if (!gConstrainWidth)
      gConstrainWidth = scaledWidthDefault;
  }
  position = 0;  // left = 1, right = 2, neither = 0

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
  dg.truewidth            = document.getElementById( "truewidth" );
  dg.trueheight           = document.getElementById( "trueheight" );
  dg.frameHeightInput     = document.getElementById("frameHeightInput");
  dg.frameWidthInput      = document.getElementById("frameWidthInput");
  dg.autoDims             = document.getElementById("autoDims");
  dg.autoHeight           = document.getElementById("autoHeight");
  dg.autoWidth            = document.getElementById("autoWidth");
  dg.captionLocation      = document.getElementById( "captionLocation");
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
  sizeState = new SizeState(dg);
  sizeState.sizeUnit = currUnit;

  var fieldList = [];
  var attrs = ["margin","border","padding"];
  var name;
  var side;
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
  if (dg.truewidth) fieldList.push(dg.truewidth);
  if (dg.trueheight) fieldList.push(dg.trueheight);
  frameUnitHandler.setEditFieldList(fieldList);
// The defaults for the document are set by the XUL document, modified by persist attributes.
// These are then overwritten by preference settings, if they exist
  v = null;
  v = (!newElement && element.getAttribute("placement") || prefBranch.getCharPref(prefprefix + "placement"));
  if (v != null && dg.locationList) dg.locationList.value = v;
  if (!newElement) {
    v = element.getAttribute("placeLocation");
    isFloat = (v != null) && v.length > 0;
  }
  else {
    v = prefBranch.getBoolPref(prefprefix + "floatlocation.forcehere");
    if (v != null) {
      checkMenuItem(document.getElementById("placement_forceHere"),v);
      isFloat  = isFloat | v;
    }
    v = prefBranch.getBoolPref(prefprefix + "floatlocation.here");
    if (v != null) {
      checkMenuItem(document.getElementById("placement_here"),v);
      isFloat  = isFloat | v;
    }
    v = prefBranch.getBoolPref(prefprefix + "floatlocation.pagefloats");
    if (v != null) {
      checkMenuItem(document.getElementById("placement_pageOfFloats"),v);
      isFloat  = isFloat | v;
    }
    v = prefBranch.getBoolPref(prefprefix + "floatlocation.toppage");
    if (v != null) {
      checkMenuItem(document.getElementById("placement_topPage"),v);
      isFloat  = isFloat | v;
    }
    v = prefBranch.getBoolPref(prefprefix + "floatlocation.bottompage");
    if (v != null) {
      checkMenuItem(document.getElementById("placement_bottomPage"),v);
      isFloat  = isFloat | v;
    }
  }
  if (!isFloat) checkMenuItem(document.getElementById("floatlistNone"), true);
  v = prefBranch.getCharPref(prefprefix + "floatplacement");
//  if (v != null) dg.wrapOptionRadioGroup.value = frameUnitHandler.getValueOf(v, prefUnit)
  v = null;
  v = ((!newElement && element.getAttribute("borderw")) || prefBranch.getCharPref(prefprefix + "border"));
  if (v != null && dg.borderInput && dg.borderInput.left && dg.borderInput.right && dg.borderInput.top && dg.borderInput.bottom)
    dg.borderInput.left.value = dg.borderInput.right.value = dg.borderInput.bottom.value = frameUnitHandler.getValueOf(v, prefUnit);
  v = (!newElement && element.getAttribute("sidemargin") || prefBranch.getCharPref(prefprefix + "hmargin"));
  if (v != null && dg.marginInput && dg.marginInput.left && dg.marginInput.right ) dg.marginInput.left.value = dg.marginInput.right.value = frameUnitHandler.getValueOf(v, prefUnit);
  v = null;
  v = (!newElement && element.getAttribute("topmargin") ||  prefBranch.getCharPref(prefprefix + "vmargin"));
  if (v != null && dg.marginInput && dg.marginInput.top && dg.marginInput.bottom) dg.marginInput.top.value = dg.marginInput.bottom.value = frameUnitHandler.getValueOf(v, prefUnit);
  v = null;
  v = (!newElement && element.getAttribute("padding") || prefBranch.getCharPref(prefprefix + "paddingwidth"));
  if (v != null && dg.paddingInput && dg.paddingInput.left && dg.paddingInput.right) dg.paddingInput.left.value = dg.paddingInput.right.value = dg.paddingInput.top.value = dg.paddingInput.bottom .value= frameUnitHandler.getValueOf(v, prefUnit);
  v = null;
  v = (!newElement && element.getAttribute("background-color") || prefBranch.getCharPref(prefprefix + "bgcolor"));
  if (v != null && dg.bgcolorWell) dg.bgcolorWell.setAttribute("style","background-color: " + v + ";");
  v = null;
  v = (!newElement && element.getAttribute("border-color") || prefBranch.getCharPref(prefprefix + "bordercolor"));
  if (v != null && dg.colorWell) dg.colorWell.setAttribute("style","background-color: " + v + ";");

  var floatLocation, posStr, pos;
  var inlineOffset = 0;
  try {
    if (!newElement)
    {   // we need to initialize the dg from the frame element
      if (! contentsElement)
         contentsElement = element;
      setFrameSizeFromExisting(dg, element, contentsElement);
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

      pos = element.getAttribute("pos");
      if (!pos)
        pos = "center";
      dg.locationList.value = pos;
      if ((pos=="inline") && element.hasAttribute("inlineOffset"))
        inlineOffset = frameUnitHandler.getValueFromString( element.getAttribute("inlineOffset"), frameUnitHandler.currentUnit );

      // if (gFrameModeImage)
      {
        if (pos === "L")
        {
          position = 1;  // left = 1, right = 2, neither = 0
        }
        else if (pos === "R")
        {
          position = 2;
        }
        var overhang = element.getAttribute("overhang");
        if (overhang === null) overhang = 0;
        dg.marginInput.right.value = overhang;
        // var sidemargin = element.getAttribute("sidemargin");
        // if (sidemargin == null) sidemargin = 0;
        // dg.marginInput.left.value = sidemargin;
        // var topmargin = element.getAttribute("topmargin");
        // if (topmargin == null) topmargin = 0;
        // dg.marginInput.top.value = topmargin;
        // var borderwidth = element.getAttribute("borderw");
        // if (borderwidth == null) borderwidth = 0;
        // dg.borderInput.left.value = borderwidth;
        // var padding = element.getAttribute("padding");
        // if (padding == null) padding = 0;
        // dg.paddingInput.left.value = padding;
      }
      floatLocation = element.getAttribute("ltxfloat");
      if (!floatLocation || floatLocation === "")
        floatLocation = "";
      dg.floatList.value = floatLocation;

      isFloat = false;
      v = (floatLocation.indexOf("H") >= 0);
      checkMenuItem(document.getElementById("ltxfloat_forceHere"),v);
      // if (v && !isFloat) dg.floatList.label = document.getElementById("ltxfloat_forceHere").getAttribute("label");
      isFloat  = isFloat | v;

      v = (floatLocation.indexOf("h") >= 0);
      checkMenuItem(document.getElementById("ltxfloat_here"),v);
      // if (v && !isFloat) dg.floatList.label = document.getElementById("ltxfloat_here").getAttribute("label");
      isFloat  = isFloat | v;

      v = (floatLocation.indexOf("p") >= 0);
      checkMenuItem(document.getElementById("ltxfloat_pageOfFloats"),v);
      // if (v && !isFloat) dg.floatList.label = document.getElementById("ltxfloat_pageOfFloats").getAttribute("label");
      isFloat  = isFloat | v;

      v = (floatLocation.indexOf("t") >= 0);
      checkMenuItem(document.getElementById("ltxfloat_topPage"),v);
      // if (v && !isFloat) dg.floatList.selectedIndex = 4;(document.getElementById("ltxfloat_topPage"));
      isFloat  = isFloat | v;

      v = (floatLocation.indexOf("b") >= 0);
      checkMenuItem(document.getElementById("ltxfloat_bottomPage"),v);
      // if (v && !isFloat) dg.floatList.label = document.getElementById("ltxfloat_bottomPage").getAttribute("label");
      isFloat  = isFloat | v;

      checkMenuItem(document.getElementById("floatlistNone"), !isFloat);
  //
  //    if (!isFloat) dg.floatList.label = document.getElementById("floatlistNone").getAttribute("label");

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
      var captionNodes = element.getElementsByTagName("caption");
      var captionNode;
      if (captionNodes && captionNodes.length > 0) captionNode = captionNodes[0];
      if (captionNode) {
        if (captionNode.hasAttribute("key"))
          key = captionNode.getAttribute("key");
        else if (captionNode.hasAttribute("id"))
          key = captionNode.getAttribute("id");
        dg.keyInput.value = key;
        gOriginalKey = key;
      }
      var captionLoc = element.getAttribute("captionloc") || "none";
      dg.captionLocation.value = captionLoc;
      gCaptionLoc = captionLoc;
    }
    else  //so it is a new element
    {
      if (gDefaultPlacement.length)
      {
        var defPlacementArray = gDefaultPlacement.split(",");
        if (defPlacementArray.length)
        {
          pos = TrimString(defPlacementArray[0]);
          if (defPlacementArray.length > 1)
          {
            floatLocation = TrimString(defPlacementArray[1]);
            if (defPlacementArray.length > 2)
              floatLocation = TrimString(defPlacementArray[2]);
          }
        }
      }
      if (gDefaultInlineOffset.length)
        inlineOffset = frameUnitHandler.getValueFromString( gDefaultInlineOffset );
    }
  }
  catch(e) {
    //msidump(e.message);
  }

  try {
    if (!newElement || gDefaultPlacement.length)
    {
      try
      {
        if (dg.locationList) {
          dg.locationList.value = pos;
        }
        if (pos == "inline")
        {
          dg.frameInlineOffsetInput.value= inlineOffset;
        }
      }
      catch(e)
      {
        msidump(e.message);
      }
    }

    if (dg.locationList) {
      placement = 0;
      var placementLetter = document.getElementById("locationList").value;
      if (/l|i/i.test(placementLetter)) placement=1;
      else if (/r|o/i.test(placementLetter)) placement = 2;
  //    enableFloatOptions(dg.wrapOptionRadioGroup);
    }
    Dg = dg;
    setAlignment(placement);
  //  enableFloatOptions(dg.wrapOptionRadioGroup);
  //  enableFloating();

  // The following will figure out what is enabled in the current state.
    locationChanged();
    floatPropertyChanged();
  //  TablePropertyChanged('TableBaselineRadioGroup');
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

  // User canceled the Dg
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
  rescaleMetrics(Dg.truewidth.value,Dg.trueheight.value);
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
  if (currentLocation === "floating") {
    floatBroadcaster.removeAttribute("disabled");
    if (Dg.floatList.selectedItem === "floatlistNone")
      Dg.floatList.selectedItem = "ltxfloat_here";
  } else {
    floatBroadcaster.setAttribute("disabled", "true");
  }
  if (currentLocation === "inline")
    inlineOffsetBroadcaster.removeAttribute("disabled");
  else
    inlineOffsetBroadcaster.setAttribute("disabled", "true");
}

function enableFloatOptions(radiogroup)
{
  return;
  // var broadcaster = document.getElementById("wrapOption");
  // var theValue = "true";
  // var position;
  // if (!radiogroup)
  //   radiogroup = document.getElementById("wrapOptionRadioGroup");
  //
  // if (document.getElementById("placeHereCheck") && document.getElementById('placeHereCheck').checked && radiogroup)
  // {
  //   theValue = "false";
  //   position = radiogroup.selectedItem.value;
  //   setAlignment((position==="L" || position==="I")?1:((position==="R"||position=="O")?2:0));
  // }
  // else
  // {
  //   setAlignment(0);
  // }
  // broadcaster.setAttribute("disabled",theValue);
  // updateDiagram("margin");
}

function enableFloating( )
{
  return;
  // var broadcaster = document.getElementById("floatingPlacement");
  // var theValue = "true";
  // var bEnableInlineOffset = false;
  // if (document.getElementById('float') && document.getElementById('float').selected)
  // {
  //   theValue = "false";
  //   broadcaster.setAttribute("disabled",theValue);
  //   enableFloatOptions();
  // }
  // else if (document.getElementById('display').selected)
  // {
  //   setAlignment(0);
  //   updateDiagram("margin");
  // }
  // else if (document.getElementById('inline').selected)
  // {
  //   updateDiagram("margin");
  //   bEnableInlineOffset = true;
  // }
  // showDisableControlsByID(["frameInlineOffsetLabel","frameInlineOffsetInput"], bEnableInlineOffset);
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
  // gConstrainWidth = width;
  // gConstrainHeight = height;
  sizeState.actualSize.width = width;
  sizeState.actualSize.height = height;
  sizeState.computeDerivedQuantities();
}

function setWidthAndHeight(width, height, event)
{
  sizeState.width = width;
  sizeState.height = height;
  sizeState.computeConstraints();


  // Dg.frameWidthInput.value = width;
  // Dg.frameHeightInput.value = height;
  // if (Dg.autoHeightCheck.checked && !Dg.autoWidthCheck.checked)
  //   constrainProportions( "frameWidthInput", "frameHeightInput", event );
  // else if (!Dg.autoHeightCheck.checked && Dg.autoWidthCheck.checked)
  //   constrainProportions( "frameHeightInput", "frameWidthInput", event );
}

function setContentSize(width, height)
// width and height are the size of the image in pixels
{
  Dg.truewidth.value = frameUnitHandler.getValueOf(width,"px");
  Dg.trueheight.value = frameUnitHandler.getValueOf(height,"px");

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

// alignment = 1 for left, 2 for right, 0 for neither
function setAlignment(alignment )
{
  position = alignment;
  if (position ==1|| position ==2)
  {
    Dg.marginInput.left.removeAttribute("disabled");
    Dg.marginInput.right.removeAttribute("disabled");
  }
  else
  {
    Dg.marginInput.left.setAttribute("disabled", "true");
    Dg.marginInput.left.setAttribute("value", "0.00");
    Dg.marginInput.right.setAttribute("disabled", "true");
    Dg.marginInput.right.setAttribute("value", "0.00");
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
    if (textbox && textbox.value != null) textbox.setAttribute("value",textbox.value);
  }
}

function isValid()
{
  // if (!(Dg.frameWidthInput.value > 0))
  // {
  //   AlertWithTitle("Layout error", "Width must be positive");
  //   return false;
  // }
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
  var rot;
  // if (frameNode.tagName == "msiframe") dimsonly = true;
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

  msiEditorEnsureElementAttribute(contentsNode, "msi_resize","true", editor);
  if (Dg.rotationList) {
    rot = Dg.rotationList.value;
    if (rot ==="rot0")
    {
      msiEditorEnsureElementAttribute(contentsNode, "rotation", null, editor);
       //this will remove the "rotation" attribute
    }
    else
    {
      msiEditorEnsureElementAttribute(contentsNode, "rotation", rot, editor);
    }
  }
  msiRequirePackage(Dg.editorElement, "ragged2e", null);

  var inlineOffsetNum = getInlineOffset("px");  //returns 0 unless inline position is chosen!
  var sidemargin = getSingleMeasurement("margin", "Left", metrics.unit, false);
  msiEditorEnsureElementAttribute(frameNode, "sidemargin", sidemargin, editor);
  var topmargin = getSingleMeasurement("margin", "Top", metrics.unit, false);
  msiEditorEnsureElementAttribute(frameNode, "topmargin", topmargin, editor);
  var overhang = getSingleMeasurement("margin", "Right", metrics.unit, false);
   msiEditorEnsureElementAttribute(frameNode, "overhang", overhang, editor);
  var marginArray = [];
  marginArray[0] = frameUnitHandler.getValueAs(topmargin,"px");
  marginArray[2] = marginArray[0];
  if (inlineOffsetNum > 0)
    marginArray[2] += inlineOffsetNum;
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
  // gFrameModeImage = true;
  if (gFrameModeImage) {
    var borderwidth = getSingleMeasurement(borderAtt, "Left", metrics.unit, false);
    msiEditorEnsureElementAttribute(frameNode, "borderw", borderwidth, editor);
  }
  else {
    msiEditorEnsureElementAttribute(frameNode, "borderw", getCompositeMeasurement("border",metrics.unit, false), editor);
  }
  if (gFrameModeImage) {
    var padding = getSingleMeasurement("padding", "Left", metrics.unit, false);
    msiEditorEnsureElementAttribute(frameNode, "padding", padding, editor);
  }
  else {
    msiEditorEnsureElementAttribute(frameNode, "padding", getCompositeMeasurement("padding",metrics.unit, false), editor);
  }
  if (Dg.autoHeightCheck && Dg.autoHeightCheck.checked)
  {
    // if (gFrameModeImage){
      msiEditorEnsureElementAttribute(contentsNode, heightAtt, null, editor);
    // }
    // else{
    //   msiEditorEnsureElementAttribute(contentsNode, heightAtt, "0", editor);
    // }
  }
  else{
    if (Dg.frameHeightInput)
      msiEditorEnsureElementAttribute(contentsNode, heightAtt, Dg.frameHeightInput.value, editor);
  }
  if ((Dg.autoWidthCheck && Dg.autoWidthCheck.getAttribute("style")!=="visibility: hidden;") && Dg.autoWidthCheck.checked)
  {
    // if (gFrameModeImage){
      msiEditorEnsureElementAttribute(contentsNode, widthAtt, null, editor);
    // }
    // else{
    //   msiEditorEnsureElementAttribute(contentsNode, widthAtt, "0", editor);
    //   contentsNode.setAttribute(widthAtt,0);
    // }
  }
  else{
    if (Dg.frameWidthInput) {
      msiEditorEnsureElementAttribute(contentsNode, widthAtt, Dg.frameWidthInput.value, editor);
      contentsNode.setAttribute(widthAtt,Dg.frameWidthInput.value);
    }
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
  setStyleAttributeOnNode(frameNode, "border-color", theColor, editor);
  msiEditorEnsureElementAttribute(frameNode, "border-color", hexcolor(theColor), editor);
  bgcolor = Dg.bgcolorWell.getAttribute("style");
  arr = bgcolor.match(/background-color\s*:([a-zA-Z\ \,0-9\(\)]+)\s*;\s*/,"");
  theColor = (arr && arr.length > 1) ? arr[1] : "";
  setStyleAttributeOnNode(frameNode, "background-color", theColor, editor);
  msiEditorEnsureElementAttribute(frameNode, "background-color", hexcolor(theColor), editor);
  msiRequirePackage(Dg.editorElement, "xcolor", "");
  msiEditorEnsureElementAttribute(frameNode, "textalignment", Dg.textAlignment.value, editor);
  setStyleAttributeOnNode(frameNode, "text-align", Dg.textAlignment.value, editor);
//RWA - The display attribute should be set by a CSS rule rather than on the individual item's style. (So that, for instance,
//      the override for graphics with captions will take effect. See baselatex.css.)
  if (posid && posid !=='inline') {
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
      setStyleAttributeOnNode(frameNode, "position", "relative", editor);
      setStyleAttributeOnNode(frameNode, "bottom", inlineOffsetStr, editor);
    }
    else
    {
      msiEditorEnsureElementAttribute(frameNode, "inlineOffset", null, editor);
      removeStyleAttributeFamilyOnNode(frameNode, "position", editor);
      removeStyleAttributeFamilyOnNode(frameNode, "bottom", editor);
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
  } else
    frameNode.removeAttribute("ltxfloat");
  if (isEnabled(Dg.locationList))
  {
    var locationParam = Dg.locationList.value;
    if ((locationParam === "inside") ||
        (locationParam === "outside") ||
        (locationParam === "left") ||
        (locationParam === "right") )
    {
      msiRequirePackage(Dg.editorElement, "wrapfig","");
    }
    // if (locationParam.indexOf("H") >= 0) {
    //   msiRequirePackage(Dg.editorElement,"float","");
    // }
    //var locationShort = locationParam.slice(0,1);
    //msiEditorEnsureElementAttribute(frameNode, "pos", locationShort, editor);
    // needsWrapfig = true;
    // if (locationParam == "I" || locationParam == "L") locationParam = "left";
    // else if (locationParam == "O" || locationParam=="R") locationParam = "right";
    // else {
    //   locationParam = "none";
    //   needsWrapfig = true;
    // }
    var needsWrapfig = true;
    if (locationParam === "right" || locationParam === "inside")
       setStyleAttributeOnNode(frameNode, "float", "right", editor);
    else if (locationParam === "left" || locationParam === "outside")
       setStyleAttributeOnNode(frameNode, "float", "left", editor);
    else {
       removeStyleAttributeFamilyOnNode(frameNode, "float", editor);
       needsWrapfig = false;
    }

    //setStyleAttributeOnNode(frameNode, "float", locationParam, editor);
    if (needsWrapfig) {
      msiEditorEnsureElementAttribute(frameNode, "req", "wrapfig", "");
    }
    if (!gFrameModeImage)
    {
      if (locationParam == "right") side = "Right";
      else if (locationParam == "left") side = "Left";
      else side = null;

      if (side){
        msiEditorEnsureElementAttribute(frameNode, "overhang", 0 - getSingleMeasurement("margin", side, metrics.unit, false), editor);
      }
    }
  }
  else
  {
    removeStyleAttributeFamilyOnNode(frameNode, "float", editor);
    if (posid == "ll_center")
    {
      setStyleAttributeOnNode(frameNode, "margin-left","auto", editor);
      setStyleAttributeOnNode(frameNode, "margin-right","auto", editor);
    }
  }
  // now set measurements in the style for frameNode
  if (gFrameModeImage)
  {
    style = getSingleMeasurement("padding","Left", "px", true);
    setStyleAttributeOnNode(frameNode, "padding", style, editor);
    style = getSingleMeasurement("border","Left","px", true);
    setStyleAttributeOnNode(frameNode, "border-width", style, editor);
  }
  else
  {
    style = getCompositeMeasurement("padding","px", true);
    setStyleAttributeOnNode(frameNode, "padding", style, editor);
    style = getCompositeMeasurement("border","px", true);
    setStyleAttributeOnNode(frameNode, "border-width", style, editor);
  }
  if (contentsNode.hasAttribute(heightAtt) && Number(contentsNode.getAttribute(heightAtt))!== 0 ) {
    setStyleAttributeOnNode(contentsNode, "height", frameUnitHandler.getValueAs(contentsNode.getAttribute(heightAtt),"px") + "px", editor);
  }
  else {
    removeStyleAttributeFamilyOnNode(contentsNode, "height", editor);
  }
  // if (frameNode.hasAttribute(heightAtt) && Number(frameNode.getAttribute(heightAtt))!== 0 ) {
  if ( document.getElementById("autoHeight") && !(document.getElementById("autoHeight").checked) && Number(frameNode.getAttribute(heightAtt))!== 0 ) {
    setStyleAttributeOnNode(frameNode, "height", frameUnitHandler.getValueAs(frameNode.getAttribute(heightAtt),"px") + "px", editor);
  }
  else {
    removeStyleAttributeFamilyOnNode(frameNode, "height", editor);
  }
  if (contentsNode.hasAttribute(widthAtt) && Number(contentsNode.getAttribute(widthAtt))!== 0) {
    setStyleAttributeOnNode(contentsNode, "width", frameUnitHandler.getValueAs(contentsNode.getAttribute(widthAtt),"px") + "px", editor);
  }
  else {
    removeStyleAttributeFamilyOnNode(contentsNode, "width", editor);
  }
  if (frameNode.hasAttribute(widthAtt) && Number(frameNode.getAttribute(widthAtt))!== 0) {
    setStyleAttributeOnNode(frameNode, "width", frameUnitHandler.getValueAs(frameNode.getAttribute(widthAtt),"px") + "px", editor);
  }
  else {
    removeStyleAttributeFamilyOnNode(frameNode, "width", editor);
  }
  if (style !== "0px")
    setStyleAttributeOnNode( frameNode, "border-style", "solid", editor );
}

function frameHeightChanged(input, event)
{
  if (input.value > 0)
  {
     scaledHeight = toPixels(input.value);
  }
  else scaledHeight = scaledHeightDefault;
  setStyleAttributeByID("content", "height", scaledHeight + "px");
  sizeState.height = input.value;
  redrawDiagram();
  sizeState.update();
}

function frameWidthChanged(input, event)
{
  if (input.value > 0) scaledWidth = toPixels(input.value);
    else scaledWidth = scaledWidthDefault;
  setStyleAttributeByID("content", "width", scaledWidth + "px");
  sizeState.width = input.value;
  sizeState.update(Dg);
  redrawDiagram();
}

function getInlineOffset(whichUnit)
{
  if (!whichUnit)
    whichUnit = frameUnitHandler.currentUnit;
  if (Dg.locationList.value === 'inline')
    return frameUnitHandler.getValueAs(Dg.frameInlineOffsetInput.value, whichUnit);
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
  if (radio.id === 'autoWidth') {
    sizeState.autoWidth = radio.selected;
    sizeState.autoHeight = !radio.selected;
    sizeState.enabledState.width = !radio.selected;
    sizeState.enabledState.height = radio.selected;
  }
  else {
    sizeState.autoHeight = radio.selected;
    sizeState.autoWidth = !radio.selected;
    sizeState.enabledState.height = !radio.selected;
    sizeState.enabledState.width = radio.selected;
  }
  sizeState.update(Dg);
}

function ToggleConstrain()
{
  // If just turned on, save the current width and height as basis for constrain ratio
  sizeState.preserveAspectRatio = !sizeState.preserveAspectRatio;
  sizeState.update(Dg);
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
  //  // double-check that neither width nor height is in percent mode; bail if so!
  //  if ( (gDialog.widthUnitsMenulist.selectedIndex !== 0)
  //     || (gDialog.heightUnitsMenulist.selectedIndex !== 0) )
  //    return;

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
  setContentSize(frameUnitHandler.getValueAs(Dg.frameWidthInput.value,"px"), frameUnitHandler.getValueAs(Dg.frameHeightInput.value,"px"));
}

function doDimensionEnabling()
{
  return;
  // Enabled only if "Custom" is selected, or actualsize is not enabled
  var enable = (document.getElementById("custom").selected) || !hasNaturalSize;
  if (enable) {
    document.getElementById("customSize").removeAttribute("disabled");
  }
  else
    document.getElementById("customSize").setAttribute("disabled", "true");

  // BUG 74145: After input field is disabled,
  //   setting it enabled causes blinking caret to appear
  //   even though focus isn't set to it.
  // SetElementEnabledById( "frameHeightInput", enable );
  // SetElementEnabledById( "frameHeightLabel", enable );

  // SetElementEnabledById( "frameWidthInput", enable );
  // SetElementEnabledById( "frameWidthLabel", enable);

  SetElementEnabledById( "unitList", enable );

  var constrainEnable = enable;
//         && ( gDialog.widthUnitsMenulist.selectedIndex === 0 )
//         && ( gDialog.heightUnitsMenulist.selectedIndex === 0 );

  SetElementEnabledById( "constrainCheckbox", constrainEnable );

}

function setActualSize(val)
{
  sizeState.isCustomSize = !val;
  sizeState.update(Dg);
//   var width, height;
//   if (gActualWidth && gActualHeight)
//   {
//     width = Dg.frameWidthInput.value = frameUnitHandler.getValueOf(gActualWidth,"px");
//     height = Dg.frameHeightInput.value = frameUnitHandler.getValueOf(gActualHeight,"px");
//   }
//   else if (gConstrainWidth && gConstrainHeight)
//   {
//     width = Dg.frameWidthInput.value = frameUnitHandler.getValueOf(gConstrainWidth,"px");
//     height = Dg.frameHeightInput.value = frameUnitHandler.getValueOf(gConstrainHeight,"px");
//   }
// //  Dg.unitList.selectedIndex = 0;
//   doDimensionEnabling();
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
    //document.getElementById("locationEnabled").setAttribute("disabled", "true");
    document.getElementById("floatlistNone").removeAttribute("checked");
  }
}

function captionPropertyChanged() {
  var captionLoc = document.getElementById("captionLocation").value;
  if (captionLoc === "none") {
    document.getElementById("keyEnabled").setAttribute("disabled", "true");
  } else {
    document.getElementById("keyEnabled").removeAttribute("disabled");
  }
}
