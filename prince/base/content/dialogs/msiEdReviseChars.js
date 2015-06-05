//Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

var data;

// dialog initialization code
function Startup()
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  if (!editor)
  {
    window.close();
    return;
  }

  gDialog.AccentAbove = document.getElementById("AccentAbove");
  gDialog.AccentBelow = document.getElementById("AccentBelow");
  gDialog.NegatedCheckbox = document.getElementById("NegatedCheckbox");
  
  doSetOKCancel(onAccept, onCancel);
  data = window.arguments[0];
  if (!data)
    data = new Object();
  if (!data.mNormalizer)
  {
    data.mNormalizer = Components.classes["@mozilla.org/intl/unicodenormalizer;1"]
                               .createInstance(Components.interfaces.nsIUnicodeNormalizer);
  }
  if ("reviseData" in data)
    setDataFromReviseData(data.reviseData);

  initDialog();
  window.mMSIDlgManager = new msiDialogConfigManager(window);
  window.mMSIDlgManager.configureDialog();
  // Set a variable on the opener window so we
  //  can track ownership of close this window with it
//  window.opener.InsertCharWindow = window;
  window.sizeToContent();

  SetWindowLocation();
}

function initDialog()
{
//  makeMSIButtonGroup(gDialog.AccentAboveButtonGroup, false);
//  makeMSIButtonGroup(gDialog.AccentBelowButtonGroup, false);

  var nWhichSel = -1;
  if (("mUpperAccent" in data) && (data.mUpperAccent != null))
    nWhichSel = setSelectionByValue(gDialog.AccentAboveButtonGroup, data.mUpperAccent);
  if (("mLowerAccent" in data) && (data.mLowerAccent != null))
    nWhichSel = setSelectionByValue(gDialog.AccentBelowButtonGroup, data.mLowerAccent);
  if (("mNegated" in data) && data.mNegated)
    gDialog.NegatedCheckbox.checked = true;
  else
    gDialog.NegatedCheckbox.checked = false;
}

function isReviseDialog()
{
  return true;
}

function setDataFromReviseData(reviseData)
{
  if (reviseData.isTextReviseData())
    setDataFromTextReviseData(reviseData);
  else
    setDataFromMathReviseData(reviseData);
}

function setDataFromMathReviseData(reviseData)
{
  var topNode = reviseData.getReferenceNode();
  var textComponents = msiNavigationUtils.treatMathNodeAsAccentedCharacter(topNode);
  var baseText = textComponents.mBaseText;
  data.mUpperAccent = textComponents.mAboveText;
  if (data.mUpperAccent)
    data.mUpperAccent = isRecognizedUpperAccent(data.mUpperAccent); //Convert it to the form (combining vs. stand-alone) we expect
  data.mLowerAccent = textComponents.mBelowText;
  if (data.mLowerAccent)
    data.mLowerAccent = isRecognizedLowerAccent(data.mLowerAccent); //Convert it to the form (combining vs. stand-alone) we expect
  data.mNegated = false;  //Is this always right? The assumption is that negated operators and relations in Math are single characters.
  var decomposed = new Object();
  data.mNormalizer.NormalizeUnicodeNFD(baseText, decomposed);
  if (decomposed.value && decomposed.value.length)
  {
    data.mBaseChar = "";
    for (var ix = decomposed.value.length - 1; ix > 0; --ix)
    {
      if (upperAccent = isRecognizedUpperAccent(decomposed.value[ix]))
      {
        if (data.mUpperAccent)  //we've already identified one - put this one with the base character
          data.mBaseChar = upperAccent + data.mBaseChar;
//          data.mBaseChar = decomposed.value[ix] + data.mBaseChar;
        else
          data.mUpperAccent = decomposed.value[ix];
      }
      else if (lowerAccent = isRecognizedLowerAccent(decomposed.value[ix]))
      {
        if (data.mLowerAccent)  //we've already identified one - put this one with the base character
          data.mBaseChar = lowerAccent + data.mBaseChar;
//          data.mBaseChar = decomposed.value[ix] + data.mBaseChar;
        else
          data.mLowerAccent = lowerAccent;
      }
      else if (strikeOut = isOverstrike(decomposed.value[ix]))
        data.mNegated = true;
    }
    data.mBaseChar = decomposed.value[0] + data.mBaseChar;
  }

}

function setDataFromTextReviseData(reviseData)
{
  var theText = reviseData.getText();
  var decomposed = new Object();
  data.mNormalizer.NormalizeUnicodeNFD(theText, decomposed);
  var upperAccent, lowerAccent, strikeOut;
  if (decomposed.value && decomposed.value.length)
  {
    data.mBaseChar = "";
    for (var ix = decomposed.value.length - 1; ix > 0; --ix)
    {
      if (upperAccent = isRecognizedUpperAccent(decomposed.value[ix]))
      {
        if (data.mUpperAccent)  //we've already identified one - put this one with the base character
          data.mBaseChar = upperAccent + data.mBaseChar;
//          data.mBaseChar = decomposed.value[ix] + data.mBaseChar;
        else
          data.mUpperAccent = decomposed.value[ix];
      }
      else if (lowerAccent = isRecognizedLowerAccent(decomposed.value[ix]))
      {
        if (data.mLowerAccent)  //we've already identified one - put this one with the base character
          data.mBaseChar = lowerAccent + data.mBaseChar;
//          data.mBaseChar = decomposed.value[ix] + data.mBaseChar;
        else
          data.mLowerAccent = lowerAccent;
      }
      else if (strikeOut = isOverstrike(decomposed.value[ix]))
        data.mNegated = true;
    }
    data.mBaseChar = decomposed.value[0] + data.mBaseChar;
  }
}

function getRadioValue(radioParent)
{
  var kids = radioParent.childNodes;
  var i;
  var len = kids.length;
  for ( i=0; i<len; i++) {
    if (kids[i].nodeName ==="msibutton" && kids[i].checked) {
      return kids[i].getAttribute("value");
    }
  }
  return null;
}


function collectDialogData()
{
  data.mUpperAccent = getRadioValue(gDialog.AccentAbove);
  data.mUpperAccentStandAlone = getStandAloneForm(data.mUpperAccent);
  data.mLowerAccent = getRadioValue(gDialog.AccentBelow);
  data.mLowerAccentStandAlone = getStandAloneForm(data.mLowerAccent);
  data.mNegated = gDialog.NegatedCheckbox.checked;
  var theCharStr = data.mBaseChar;
  if (data.mUpperAccent)
  {
    if (msiNavigationUtils.upperAccentCombinesWithCharInMath(data.mUpperAccent))
    {
      theCharStr += data.mUpperAccent;
      data.mUpperAccentStandAlone = null;
    }
  } else data.mUpperAccent="";
  if (data.mLowerAccent) {
    if (msiNavigationUtils.lowerAccentCombinesWithCharInMath(data.mLowerAccent))
    {
    theCharStr += data.mLowerAccent;
    data.mLowerAccentStandAlone = null;
    }
  } else data.mLowerAccent="";
  if (data.mNegated)
    theCharStr += "\u0338";
  var baseComposed = new Object();
  data.mNormalizer.NormalizeUnicodeNFC(theCharStr, baseComposed);
  data.mCompiledBaseChar = baseComposed.value;
  theCharStr = data.mBaseChar + data.mUpperAccent + data.mLowerAccent;
  if (data.mNegated)
    theCharStr += "\u0338";
  var composed = new Object();
  data.mNormalizer.NormalizeUnicodeNFC(theCharStr, composed);
  data.mCompiledText = composed.value;
}

function getStandAloneForm(aUniChar)
{
  var retVal = null;
  switch(aUniChar)
  {
    case "^":
    case "\u0302":
    case "\uFE3F":
      retVal = "^";
    break;
    case "\u02c7":
    case "\u030c":
      retVal =  "\u02c7";
    break;
    case "~":
    case "\u0303":
      retVal = "~";
    break;
    case "\u00b4":
    case "\u0301":
      retVal = "\u00b4";
    break;
    case "\u0060":
    case "\u0300":
      retVal = "\u0060";
    break;
    case "\u02d8":
    case "\u0306":
      retVal = "\u02d8";
    break;
    case "\u00af":
    case "\u0305":
      retVal = "\u00af";
    break;

    case "\u02dd":
    case "\u030b":
      retVal = "\u02dd";
    break;
    case "\u02da":
    case "\u030a":
      retVal = "\u02da";
    break;
    case "\u02d9":
    case "\u0307":
      retVal = "\u02d9";
    break;
    case "\u00a8":
    case "\u0308":
      retVal = "\u00a8";
    break;
    case "\u20db":
      retVal = "\u20db";
    break;
    case "\u20dc":
      retVal = "\u20dc";
    break;
    case "\u20d7":
      retVal = "\u20d7";
    break;

    case "\u00b8":
    case "\u0327":
      retVal = "\u00b8";
    break;
    case "\u02db":
    case "\u0328":
      retVal = "\u02db";
    break;
    case "\u02d3":
    case "\u0323":
      retVal = "\u02d3";
    break;
    case "\u005f":
    case "\u0332":
      retVal = "\u005f";
    break;

    default:
      retVal = aUniChar;  //Just return what was passed in
    break;
  }
  return retVal;
}

function isRecognizedUpperAccent(aUniChar)
{
  var retVal = null;
  switch(aUniChar)
  {
    case "^":
    case "\u0302":
      retVal = "\u0302";
    break;
    case "\u02c7":
    case "\u030c":
      retVal =  "\u030c";
    break;
    case "~":
    case "\u0303":
      retVal = "\u0303";
    break;
    case "\u00b4":
    case "\u0301":
      retVal = "\u0301";
    break;
    case "\u0060":
    case "\u0300":
      retVal = "\u0300";
    break;
    case "\u02d8":
    case "\u0306":
      retVal = "\u0306";
    break;
    case "\u00af":
    case "\u0305":
      retVal = "\u0305";
    break;

    case "\u02dd":
    case "\u030b":
      retVal = "\u030b";
    break;
    case "\u02da":
    case "\u030a":
      retVal = "\u030a";
    break;
    case "\u02d9":
    case "\u0307":
      retVal = "\u0307";
    break;
    case "\u00a8":
    case "\u0308":
      retVal = "\u0308";
    break;
    case "\u20db":
      retVal = "\u20db";
    break;
    case "\u20dc":
      retVal = "\u20dc";
    break;
    case "\u20d7":
      retVal = "\u20d7";
    break;
  }
  return retVal;
}

function isRecognizedLowerAccent(aUniChar)
{
  var retVal = null;
  switch(aUniChar)
  {
    case "\u00b8":
    case "\u0327":
      retVal = "\u0327";
    break;
    case "\u02db":
    case "\u0328":
      retVal = "\u0328";
    break;
    case "\u02d3":
    case "\u0323":
      retVal = "\u0323";
    break;
    case "\u005f":
    case "\u0332":
      retVal = "\u0332";
    break;
  }
  return retVal;
}

function isOverstrike(aUniChar)
{
  return (aUniChar == "\u0338");
}

function onAccept()
{
  var retVal = Apply();
  if (retVal)
    SaveWindowLocation();

  return retVal;
}

function Apply()
{
  // Revise the character
  var editorElement = msiGetParentEditorElementForDialog(window);
  collectDialogData();
    
  var theWindow = window.opener;
  if (isReviseDialog())
  {
    if (!theWindow || !("msiReviseChars" in theWindow))
      theWindow = msiGetTopLevelWindow();
    theWindow.msiReviseChars(data.reviseData, data, editorElement);
    editorElement.focus();
    //theWindow.document.commandDispatcher.focusedWindow.focus();
  }
    
//  try {
//  } catch(e) {}

  // Set persistent attributes to save?
  
  // Don't close the dialog
//  return false;
  return true;
}

//function doFakeApply()
//{
//  var editorElement = msiGetParentEditorElementForDialog(window);
//  collectDialogData();
//    
//  var theWindow = window.opener;
//  if (isReviseDialog())
//  {
//    if (!theWindow || !("msiReviseChars" in theWindow))
//      theWindow = msiGetTopLevelWindow();
//    theWindow.msiReviseChars(data.reviseData, data, editorElement);
//  }
//    
//  return false;
//}

// Don't allow inserting in HTML Source Mode
function onFocus()
{
  var enable = true;
//  if ("gEditorDisplayMode" in window.opener)
  try
  {
    var editorElement = msiGetParentEditorElementForDialog(window);
    enable = !msiIsInHTMLSourceMode(editorElement);
  } catch(exc) {AlertWithTitle("Error in msiEdReviseChars.js", "Error in onFocus: " + exc);}
  SetElementEnabled(document.documentElement.getButton("accept"), enable);
}

function onClose()
{
//  window.opener.InsertCharWindow = null;
  SaveWindowLocation();
  return true;
}

//------------------------------------------------------------------
var initialize = true;


/*------------------------------------------------------------------ Following list retained from Mozilla's original
implementation may yet prove useful (rwa):

// From Unicode 3.0 Page 54. 3.11 Conjoining Jamo Behavior
var SBase = 0xac00;
var LBase = 0x1100;
var VBase = 0x1161;
var TBase = 0x11A7;
var LCount = 19;
var VCount = 21;
var TCount = 28;
var NCount = VCount * TCount;
// End of Unicode 3.0


const upper=[
  // A
  "\u00c0\u00c1\u00c2\u00c3\u00c4\u00c5\u0100\u0102\u0104\u01cd\u01de\u01de\u01e0\u01fa\u0200\u0202\u0226\u1e00\u1ea0\u1ea2\u1ea4\u1ea6\u1ea8\u1eaa\u1eac\u1eae\u1eb0\u1eb2\u1eb4\u1eb6",
  // B
  "\u0181\u0182\u0184\u1e02\u1e04\u1e06",
  // C
  "\u00c7\u0106\u0108\u010a\u010c\u0187\u1e08",
  // D
  "\u010e\u0110\u0189\u018a\u1e0a\u1e0c\u1e0e\u1e10\u1e12",
  // E
  "\u00C8\u00C9\u00CA\u00CB\u0112\u0114\u0116\u0118\u011A\u0204\u0206\u0228\u1e14\u1e16\u1e18\u1e1a\u1e1c\u1eb8\u1eba\u1ebc\u1ebe\u1ec0\u1ec2\u1ec4\u1ec6",
  // F
  "\u1e1e",
  // G
  "\u011c\u011E\u0120\u0122\u01e4\u01e6\u01f4\u1e20",
  // H
  "\u0124\u0126\u021e\u1e22\u1e24\u1e26\u1e28\u1e2a",
  // I
  "\u00CC\u00CD\u00CE\u00CF\u0128\u012a\u012C\u012e\u0130\u0208\u020a\u1e2c\u1e2e\u1ec8\u1eca",
  // J
  "\u0134\u01f0",
  // K
  "\u0136\u0198\u01e8\u1e30\u1e32\u1e34",
  // L
  "\u0139\u013B\u013D\u013F\u0141\u1e36\u1e38\u1e3a\u1e3c",
  // M
  "\u1e3e\u1e40\u1e42",
  // N
  "\u00D1\u0143\u0145\u0147\u014A\u01F8\u1e44\u1e46\u1e48\u1e4a",
  // O
  "\u00D2\u00D3\u00D4\u00D5\u00D6\u014C\u014E\u0150\u01ea\u01ec\u020c\u020e\u022A\u022C\u022E\u0230\u1e4c\u1e4e\u1e50\u1e52\u1ecc\u1ece\u1ed0\u1ed2\u1ed4\u1ed6\u1ed8\u1eda\u1edc\u1ede\u1ee0\u1ee2",
  // P
  "\u1e54\u1e56",
  // No Q
  // R
  "\u0154\u0156\u0158\u0210\u0212\u1e58\u1e5a\u1e5c\u1e5e",
  // S
  "\u015A\u015C\u015E\u0160\u0218\u1e60\u1e62\u1e64\u1e66\u1e68",
  // T
  "\u0162\u0164\u0166\u021A\u1e6a\u1e6c\u1e6e\u1e70",
  // U
  "\u00D9\u00DA\u00DB\u00DC\u0168\u016A\u016C\u016E\u0170\u0172\u0214\u0216\u1e72\u1e74\u1e76\u1e78\u1e7a\u1ee4\u1ee6\u1ee8\u1eea\u1eec\u1eee\u1ef0",
  // V
  "\u1e7c\u1e7e",
  // W
  "\u0174\u1e80\u1e82\u1e84\u1e86\u1e88",
  // X
  "\u1e8a\u1e8c",
  // Y
  "\u00DD\u0176\u0178\u0232\u1e8e\u1ef2\u1ef4\u1ef6\u1ef8",
  // Z
  "\u0179\u017B\u017D\u0224\u1e90\u1e92\u1e94"
];

const lower=[
  // a
  "\u00e0\u00e1\u00e2\u00e3\u00e4\u00e5\u0101\u0103\u0105\u01ce\u01df\u01e1\u01fb\u0201\u0203\u0227\u1e01\u1e9a\u1ea1\u1ea3\u1ea5\u1ea7\u1ea9\u1eab\u1ead\u1eaf\u1eb1\u1eb3\u1eb5\u1eb7",
  // b
  "\u0180\u0183\u0185\u1e03\u1e05\u1e07",
  // c
  "\u00e7\u0107\u0109\u010b\u010d\u0188\u1e09",
  // d
  "\u010f\u0111\u1e0b\u1e0d\u1e0f\u1e11\u1e13",
  // e
  "\u00e8\u00e9\u00ea\u00eb\u0113\u0115\u0117\u0119\u011b\u0205\u0207\u0229\u1e15\u1e17\u1e19\u1e1b\u1e1d\u1eb9\u1ebb\u1ebd\u1ebf\u1ec1\u1ec3\u1ec5\u1ec7",
  // f
  "\u1e1f",
  // g
  "\u011d\u011f\u0121\u0123\u01e5\u01e7\u01f5\u1e21",
  // h
  "\u0125\u0127\u021f\u1e23\u1e25\u1e27\u1e29\u1e2b\u1e96",
  // i
  "\u00ec\u00ed\u00ee\u00ef\u0129\u012b\u012d\u012f\u0131\u01d0\u0209\u020b\u1e2d\u1e2f\u1ec9\u1ecb",
  // j
  "\u0135",
  // k
  "\u0137\u0138\u01e9\u1e31\u1e33\u1e35",
  // l
  "\u013a\u013c\u013e\u0140\u0142\u1e37\u1e39\u1e3b\u1e3d",
  // m
  "\u1e3f\u1e41\u1e43",
  // n
  "\u00f1\u0144\u0146\u0148\u0149\u014b\u01f9\u1e45\u1e47\u1e49\u1e4b",
  // o
  "\u00f2\u00f3\u00f4\u00f5\u00f6\u014d\u014f\u0151\u01d2\u01eb\u01ed\u020d\u020e\u022b\u22d\u022f\u0231\u1e4d\u1e4f\u1e51\u1e53\u1ecd\u1ecf\u1ed1\u1ed3\u1ed5\u1ed7\u1ed9\u1edb\u1edd\u1edf\u1ee1\u1ee3",
  // p
  "\u1e55\u1e57",
  // No q
  // r
  "\u0155\u0157\u0159\u0211\u0213\u1e59\u1e5b\u1e5d\u1e5f",
  // s
  "\u015b\u015d\u015f\u0161\u0219\u1e61\u1e63\u1e65\u1e67\u1e69",
  // t
  "\u0162\u0163\u0165\u0167\u021b\u1e6b\u1e6d\u1e6f\u1e71\u1e97",
  // u
  "\u00f9\u00fa\u00fb\u00fc\u0169\u016b\u016d\u016f\u0171\u0173\u01d4\u01d6\u01d8\u01da\u01dc\u0215\u0217\u1e73\u1e75\u1e77\u1e79\u1e7b\u1ee5\u1ee7\u1ee9\u1eeb\u1eed\u1eef\u1ef1",
  // v
  "\u1e7d\u1e7f",
  // w
  "\u0175\u1e81\u1e83\u1e85\u1e87\u1e89\u1e98",
  // x
  "\u1e8b\u1e8d",
  // y
  "\u00fd\u00ff\u0177\u0233\u1e8f\u1e99\u1ef3\u1ef5\u1ef7\u1ef9",
  // z
  "\u017a\u017c\u017e\u0225\u1e91\u1e93\u1e95"
];


const symbol = "\u00a1\u00a2\u00a3\u00a4\u00a5\u20ac\u00a6\u00a7\u00a8\u00a9\u00aa\u00ab\u00ac\u00ae\u00af\u00b0\u00b1\u00b2\u00b3\u00b4\u00b5\u00b6\u00b7\u00b8\u00b9\u00ba\u00bb\u00bc\u00bd\u00be\u00bf\u00d7\u00f7";

const otherupper = "\u00c6\u00d0\u00d8\u00de\u0132\u0152\u0186\u01c4\u01c5\u01c7\u01c8\u01ca\u01cb\u01F1\u01f2";

const otherlower = "\u00e6\u00f0\u00f8\u00fe\u00df\u0133\u0153\u01c6\u01c9\u01cc\u01f3";

*/
