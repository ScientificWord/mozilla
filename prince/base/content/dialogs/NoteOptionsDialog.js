var numberingControls = ["footnoteNumberLabel", "footnoteNumberTextbox"];
var optionsData;

function startup()
{
  optionsData = window.arguments[0];

  document.getElementById("footnoteMarkOrTextRadioGroup").value = optionsData.markOrText;
  var bOverrideNumber = (optionsData.overrideNumber != null);
  document.getElementById("overrideAutoNumberingCheckbox").checked = bOverrideNumber;
  var numToDisplay = 1;
  if (bOverrideNumber)
    numToDisplay = optionsData.overrideNumber;
  document.getElementById("footnoteNumberTextbox").valueNumber = numToDisplay;
  checkEnable();
}

function checkEnable()
{
  var bOverrideNumber = document.getElementById("overrideAutoNumberingCheckbox").checked;
  enableControlsByID(numberingControls, bOverrideNumber);
}

function onCancel() {
  optionsData.Cancel = true;
  return true;   
}

function onAccept()
{
  optionsData.markOrText = document.getElementById("footnoteMarkOrTextRadioGroup").value;
  var bOverrideNumber = document.getElementById("overrideAutoNumberingCheckbox").checked;
  if (bOverrideNumber)
    optionsData.overrideNumber = document.getElementById("footnoteNumberTextbox").valueNumber;
  else if ("overrideNumber" in optionsData)
    delete optionsData.overrideNumber;

  return true;    
}
