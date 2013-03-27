// Copyright (c) 2005 MacKichan Software, Inc.  All Rights Reserved.
// put the plot data from the graph object (window.arguments[0]) in
// the right places in the dialog. 

function plotStartup() { 
  var plotno = window.arguments[0].getGraphAttribute ("plotnumber");
  var dim    = window.arguments[0].getGraphAttribute ("Dimension");
  var ptype  = window.arguments[0].getPlotValue ("PlotType", plotno);
  var animate= window.arguments[0].getPlotValue ("Animate", plotno);
  var alist  = window.arguments[0].plotAttributeList();                    
  for (var i=0; i<alist.length; i++) { 
    if (document.getElementById(alist[i])) {  
      switch (alist[i]) {
        case "DiscAdjust":
          var oldval = window.arguments[0].getPlotValue (alist[i], plotno); 
          radioGroupSetCurrent(alist[i], oldval);  
          break;
        case "IncludeLines":                   
        case "IncludePoints":
        case "AnimateVisBefore":
        case "AnimateVisAfter":
          var oldval = window.arguments[0].getPlotValue (alist[i], plotno);   
          if (oldval == "false") 
            document.getElementById(alist[i]).checked = false;            
          else if (oldval == "true")
            document.getElementById(alist[i]).checked = true;            
          break;
        case "AISubIntervals":            
//        case "AnimateStart":            
//        case "AnimateEnd": 
//        case "AnimateFPS":
          var oldval = window.arguments[0].getPlotValue (alist[i], plotno);   
          document.getElementById(alist[i]).value = oldval;            
          break;
        default:
          populatePopupMenu (alist[i], window.arguments[0].getPlotValue (alist[i], plotno));
          break;
	  }
    } else if (alist[i] == "LineColor") {
      var oldval = window.arguments[0].getPlotValue (alist[i], plotno);   
      var colorstr = "background-color: " + oldval;
//***      document.getElementById("linecolorbutton").setAttribute("style", colorstr);
    }
  }
  
  if (ptype == "approximateIntegral") {
    document.getElementById("twoD").collapsed = true;            
    document.getElementById("threeD").collapsed = true;            
    document.getElementById("approxIntegral").collapsed = false;            
  } else if (dim[0] == "2") {
    document.getElementById("twoD").collapsed = false;            
    document.getElementById("threeD").collapsed = true;            
    document.getElementById("approxIntegral").collapsed = true;            
  } else if (dim[0] == "3") {
    document.getElementById("twoD").collapsed = true;            
    document.getElementById("threeD").collapsed = false;            
    document.getElementById("approxIntegral").collapsed = true;            
  }   
  if (animate == "true") {
    document.getElementById("animation").collapsed = false;            
  }
                                                            
}


function plotCancel(){
  window.arguments[0].setPlotAttribute("returnvalue", false);
  DOMGListRemove (window.arguments[1], window.arguments[2]);
  return false;
}

// extract the value from each of the plot attributes and save
function plotOK () {
  window.arguments[0].setPlotAttribute("returnvalue", true);
  DOMGListRemove (window.arguments[1], window.arguments[2]);

  // grab anything that's in the plot attribute list
  var alist = window.arguments[0].plotAttributeList();                                
  var plotno = window.arguments[0].getGraphAttribute("plotnumber");
  for (var i=0; i<alist.length; i++) {   
    if (document.getElementById(alist[i])) {   
      switch (alist[i]) {
        case "DiscAdjust":                         
          var newval = document.getElementById(alist[i]).selectedItem.value;
          var oldval = window.arguments[0].getPlotValue (alist[i], plotno);  
          if (newval != oldval) {                                        
            window.arguments[0].setPlotAttribute (alist[i], plotno, newval);
          }  
          break;
        case "IncludePoints":
        case "IncludeLines":
        case "AnimateVisBefore":
        case "AnimateVisAfter":
          var newval = document.getElementById(alist[i]).checked ? "true" : "false";
          var oldval = window.arguments[0].getPlotValue (alist[i], plotno);  
          if (newval != oldval) {                                        
            window.arguments[0].setPlotAttribute (alist[i], plotno, newval);
          }
          break;
                                                                          
        case "AISubIntervals":
//        case "AnimateStart":
//        case "AnimateEnd":
//        case "AnimateFPS":
          var newval = parseInt(document.getElementById(alist[i]).value, 10);
          var oldval = window.arguments[0].getPlotValue (alist[i], plotno);   
          if (isNaN(newval))
            newval = oldval;
          if (newval != oldval) {                                        
            window.arguments[0].setPlotAttribute (alist[i], plotno, newval);
          }  
          break;
        default:
          var newval = document.getElementById(alist[i]).value;            
          if ((newval != "") && (newval != "undefined")) {                 
            var oldval = window.arguments[0].getPlotValue (alist[i], plotno);                        
            if (newval != oldval) {                                        
               window.arguments[0].setPlotAttribute (alist[i], plotno, newval);
            }                                                              
          }                                                                
          break;
      }                                                                   
	}
  }
  return true;
}

function GetPlotColor (attributeName)
{
  // Don't allow a blank color, i.e., using the "default"
  var colorObj = { NoDefault:true, Type:"", TextColor:0, PageColor:0, Cancel:false };
  var plotno   = window.arguments[0].getGraphAttribute("plotnumber");
  var oldcolor = window.arguments[0].getPlotValue (attributeName, plotno);                        
  if (oldcolor != "") {
     colorObj.TextColor = oldcolor;
     colorObj.PageColor = oldcolor;
  }
  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable", "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;
  else {
		msiGetEditor(editorElement).incrementModificationCount(1);
	}
  var color = colorObj.TextColor;
//  dump ("SMR in GetPlotColor setting the color to " + color + "\n");
  window.arguments[0].setPlotAttribute (attributeName, plotno, color);

}

