
var paragraphData = new Object();


function initialize()
{
  paragraphData.node = window.arguments[0];
  if (!paragraphData.node)
    return;
  assignDialogData(paragraphData.node);
}


function unitIndexFromVal(val)
{
  switch (val) {
    case "pt": return 0;
    case "cm": return 1;
    case "mm": return 2;
    case "in": return 3;
    default: return -1;
  }
}
  
function alignmentIndexFromVal(val)
{
  switch (val) {
    case "left": return 0;
    case "right": return 1;
    case "center": return 2;
    case "normal": return 3;
    default: return -1;
  }
}


function assignDialogData(node)
{
  var val;
  if (val = node.getAttribute("unit"))
    document.getElementById("otfont.units").selectedIndex = unitIndexFromVal(val);
  if (val = node.getAttribute("before"))
    document.getElementById("beforetext").value = val;
  if (val = node.getAttribute("after"))
    document.getElementById("aftertext").value = val;
  if (val = node.getAttribute("first"))
    document.getElementById("firstline").value = val;
  if (val = node.getAttribute("above"))
    document.getElementById("aboveparagraph").value = val;
  if (val = node.getAttribute("below"))
    document.getElementById("belowparagraph").value = val;
  if (val = node.getAttribute("spacing"))
    document.getElementById("linespacing").value = val;
  if (val = node.getAttribute("leading"))
    document.getElementById("leadingval").value = val;
  if (val = node.getAttribute("alignment"))
    document.getElementById("alignment").selectIndex = alignmentIndexFromVal(val);
  if (val = node.getAttribute("bwidth"))
    document.getElementById("borderwidth").value = val;
  if (val = node.getAttribute("sidep"))
    document.getElementById("bordersidepad").value = val;
  if (val = node.getAttribute("tpad"))
    document.getElementById("bordertoppad").value = val;
  if (val = node.getAttribute("bcolor"))
    setColorWell("bordercolorWell",val);
  if (val = node.getAttribute("bgcolor"))
    setColorWell("backgroundcolorWell",val);
  if (val = node.getAttribute("tcolor"))
    setColorWell("textcolorWell",val);
}

function collectDialogData(data)
{
  data.units = document.getElementById("otfont.units").selectedItem.value;
  data.beforetext = document.getElementById("beforetext").value;
  if (Number( data.beforetext) == 0) data.beforetext = null;
  data.aftertext = document.getElementById("aftertext").value;
  if (Number(data.aftertext) == 0) data.aftertext = null;
  data.firstline = document.getElementById("firstline").value;
  if (Number(data.firstline) == 0) data.firstline = null;
  data.aboveparagraph = document.getElementById("aboveparagraph").value;
  if (Number(data.aboveparagraph) == 0) data.aboveparagraph = null;
  data.belowparagraph = document.getElementById("belowparagraph").value;
  if (Number(data.belowparagraph) == 0) data.belowparagraph = null;
  data.linespacing = document.getElementById("linespacing");
  if (data.linespacing) data.linespacing = data.linespacing.value;
  if (data.linespacing == "leading")
  {
    data.leading = document.getElementById("leadingval").value;
    if (Number(data.leading)==0) data.linespacing = "single";
  }
  // now for alignment
  data.align = document.getElementById("alignment").selectedItem.id;
  // ... and border and colors
  data.bwidth = document.getElementById("borderwidth").value;
  if (Number(data.bwidth) == 0) data.bwidth = null;
  data.sidep = document.getElementById("bordersidepad").value;
  if (Number(data.sidep) == 0) data.sidep = null;
  data.tpad = document.getElementById("bordertoppad").value;
  if (Number(data.tpad) == 0) data.tpad = null;
  data.bcolor = getColorWell("bordercolorWell");
  if (data.bcolor == "black") data.bcolor = null;
  data.bgcolor = getColorWell("backgroundcolorWell");
  if (data.bgcolor == "white") data.bcolor = null;
  data.tcolor = getColorWell("textcolorWell");
  if (data.tcolor == "black") data.tcolor = null;
}

function assignAttributesToNode(node, data)
{
  var style = "";
  var unit = data.units;
  if (data.units) node.setAttribute("unit",data.units);
  if (data.beforetext)
  {
    node.setAttribute("before",data.beforetext);
    style += "margin-left: "+data.beforetext+unit+";";
  }
  else node.removeAttribute("before");
  if (data.aftertext)
  {
    node.setAttribute("after",data.aftertext);
    style += "margin-right: "+data.aftertext+unit+";";
  }
  else node.removeAttribute("after");
  if (data.firstline)
  {
    node.setAttribute("first",data.firstline);
    style+="text-indent: "+data.firstline+unit+";";
  }
  else node.removeAttribute("first");
  if (data.aboveparagraph)
  {
    node.setAttribute("above",data.aboveparagraph);
    style+="margin-top: "+data.aboveparagraph+unit+";";
  }
  else node.removeAttribute("above");
  if (data.belowparagraph)
  {
    node.setAttribute("below",data.belowparagraph);
    style+="margin-bottom: "+data.belowparagraph+unit+";";
  }
  else node.removeAttribute("below");
  if (data.linespacing)
  {
    node.setAttribute("spacing",data.linespacing);
    switch (data.linespacing)
    {
      case "single": break;
      case "sesqui":
        style+="line-height: 150%;";
        break;
      case "double":
        style+="line-height: 200%;";
        break;
      case "leading":
      default:
        style+="line-height: "+data.leading+unit+";";
    }
  }
  else node.removeAttribute("spacing");
  if (data.leading)
  {
    node.setAttribute("leading",data.leading);
  }
  else node.removeAttribute("leading");
  if (data.align!="normal")
  {
    node.setAttribute("alignment",data.align);
    style+="text-align: "+data.align+";";
  }
  else node.removeAttribute("alignment");
  if (data.bwidth)
  {
    node.setAttribute("bwidth",data.bwidth);
    style+="border-width: "+data.bwidth+unit+"; border-style: solid; ";
  }
  else node.removeAttribute("bwidth");
  if (data.sidep)
  {
    node.setAttribute("sidep",data.sidep);
    style+="padding-left: "+data.sidep+unit+"; padding-right: "+data.sidep+unit+";";
  }
  else node.removeAttribute("sidep");
  if (data.tpad)
  {
    node.setAttribute("tpad",data.tpad);
    style+="padding-top: "+data.tpad+unit+"; padding-bottom: "+data.tpad+unit+";";
  }
  else node.removeAttribute("tpad");
  if (data.bcolor)
  {
    node.setAttribute("bcolor",data.bcolor);
    style+="border-color: "+data.bcolor+";";
  }
  else node.removeAttribute("bcolor");
  if (data.bgcolor)
  {
    node.setAttribute("bgcolor",data.bgcolor);
    style+="background-color: "+data.bgcolor+";";
  }
  else node.removeAttribute("bgcolor");
  if (data.tcolor)
  {
    node.setAttribute("tcolor",data.tcolor);
    style+="color: "+data.tcolor+";";
  }
  else node.removeAttribute("tcolor");
  node.setAttribute("style", style);
}

function setColorWell(ColorWellID, color)
{
  var colorWell = document.getElementById(ColorWellID);
  if (colorWell)
  {
    if (!color || color == "")
    {
      // Don't set color (use default)
      // Trigger change to not show color swatch
      colorWell.setAttribute("default","true");
      // Style in CSS sets "background-color",
      //   but color won't clear unless we do this:
      colorWell.removeAttribute("style");
    }
    else
    {
      colorWell.removeAttribute("default");
      // Use setAttribute so colorwell can be a XUL element, such as button
      colorWell.setAttribute("style", "background-color:"+color);
    }
  }
}

function getColorAndUpdate(id)
{
  var colorWell = document.getElementById(id);
  if (!colorWell) return;
  var color = getColorWell(id);

  var colorObj = { NoDefault: false, Type: "Rule", TextColor: color, PageColor: 0, Cancel: false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  setColorWell(id, colorObj.TextColor); 
}

function getColorWell(colorWellId)
{
  var colorWell = document.getElementById(colorWellId);
  if (colorWell)
  {
    var style = colorWell.getAttribute("style");
    var ma = style.match(/background-color\s*:([\w\(\),\s]+);/);
    if (ma && ma.length >0) return ma[1];
  }
  return "white";
}


function onAccept()
{
  collectDialogData(paragraphData);
  assignAttributesToNode(paragraphData.node, paragraphData);
  return true;
}

function onCancel()
{
  return true;
}

