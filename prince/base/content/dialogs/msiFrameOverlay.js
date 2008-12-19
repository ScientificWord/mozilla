
function  initFrameTab(gDialog)
{
  gDialog.frameUnitMenulist      = document.getElementById( "frameUnitMenulist");
  gDialog.marginInput = {left:   document.getElementById( "marginLeftInput"),
                         right:  document.getElementById( "marginRightInput"),
                         top:    document.getElementById( "marginTopInput"),
                         bottom: document.getElementById( "marginBottomInput")};
  gDialog.borderInput = {left:   document.getElementById( "borderLeftInput"),
                         right:  document.getElementById( "borderRightInput"),
                         top:    document.getElementById( "borderTopInput"),
                         bottom: document.getElementById( "borderBottomInput")};
  gDialog.paddingInput ={left:   document.getElementById( "paddingLeftInput"),
                         right:  document.getElementById( "paddingRightInput"),
                         top:    document.getElementById( "paddingTopInput"),
                         bottom: document.getElementById( "paddingBottomInput")};
  gDialog.cropInput   = {left:   document.getElementById( "cropLeftInput"),
                         right:  document.getElementById( "cropRightInput"),
                         top:    document.getElementById( "cropTopInput"),
                         bottom: document.getElementById( "cropBottomInput")};
  gDialog.colorWell   = document.getElementById("colorWell");
  gDialog.placementRadioGroup   = document.getElementById("placementRadioGroup");
  gDialog.placeHereCheck        = document.getElementById("placeHereCheck");
  gDialog.placeFloatsCheck      = document.getElementById("placeFloatsCheck");
  gDialog.placeTopCheck         = document.getElementById("placeTopCheck");
  gDialog.placeBottomCheck      = document.getElementById("placeBottomCheck");
  gDialog.herePlacementRadioGroup   = document.getElementById("herePlacementRadioGroup");
}

function extendInput( anId )
{
  //parse the id
  if (anId.length == 0) return;
  var regexp=/(.*)(Left|Right|Top|Bottom)(.*)/;
  var result = regexp.exec(anId);
  switch (result[2]) { // notice that there are no breaks in this switch statement -- intentional
    case "Left":  document.getElementById(result[1]+"Left"+result[3]).value = document.getElementById(anId).value;
    case "Right": document.getElementById(result[1]+"Right"+result[3]).value = document.getElementById(anId).value;
    case "Top":   document.getElementById(result[1]+"Top"+result[3]).value = document.getElementById(anId).value;
    case "Bottom": document.getElementById(result[1]+"Bottom"+result[3]).value = document.getElementById(anId).value;
  }
}

function enableHere( )
{
  var broadcaster = document.getElementById("herePlacement");
  var theValue = "true";
  if (document.getElementById('placeHereCheck').checked) theValue = "false";
  broadcaster.setAttribute("disabled",theValue);
}


function enableFloating( )
{
  var broadcaster = document.getElementById("floatingPlacement");
  var theValue = "true";
  if (document.getElementById('floatingRadio').selected) theValue = "false";
  broadcaster.setAttribute("disabled",theValue);
  if (theValue=="true") document.getElementById("herePlacement").setAttribute("disabled","true");
  else enableHere();
}

