var EXPORTED_SYMBOLS = ["UnitHandler"];


function UnitHandler()
{
  this.supportedUnits = ["mm", "cm", "in", "pt", "bp", "pc", "px", "pct"];
  this.unitSize =  // in mm
    {mm: 1,
     cm: 10,
     "in": 25.4,
     pt: 0.3514598,
     bp: 0.3527778,
     pc: 2.54,
     px: 0.2654833,	// we say 96 pixels/inch. It varies but portability requires a fixed value
     pct: null };

  this.currentUnit = null;

  this.getValueAs = function( value, unit )  // given a measurement that is 'value' in the current unit, provide it in the new unit.
  { 
    if (!(unit in this.unitSize)){ return null;}
    if (unit === this.currentUnit) { return value; }
    return (value/this.unitSize[unit])*this.unitSize[this.currentUnit];
  };

  this.setCurrentUnit = function( unit ) // returns the previous value
  {
    var prev = this.currentUnit;
    if (!(unit in this.unitSize)) 
    {
      this.currentUnit = null;
      return prev;
    }
    this.currentUnit = unit;
    return prev;
  };
   
  this.getCurrentUnit = function()
  {
    return this.currentUnit;
  };

  this.convertAll = function(newUnit, valueArray )
  {
    var factor = this.unitSize[this.currentUnit]/this.unitSize[newUnit];
    for (var i = 0, len = valueArray.length; i < len; i++)
      { valueArray[i] *= factor; }
  };

  this.getDisplayString = function(theUnit)
  {
    if (!(theUnit in this.unitSize)){ return null;}
    if (!this.mStringBundle)
    {
      try {
        var strBundleService = Components.classes["@mozilla.org/intl/stringbundle;1"].getService(); 
        strBundleService = strBundleService.QueryInterface(Components.interfaces.nsIStringBundleService);
        this.mStringBundle = strBundleService.createBundle("chrome://prince/locale/msiDialogs.properties"); 
      } catch (ex) {dump("Problem in initializing string bundle in unitHandler.getDisplayString: exception is [" + ex + "].\n");}
    }
    if (this.mStringBundle)
    {
      var unitsPrefix = "units.";
      try
      {
        return this.mStringBundle.GetStringFromName(unitsPrefix + theUnit);
      } catch (e) {dump("Problem in msiUnitsList.getDisplayString for unit [" + theUnit + "]: exception is [" + e + "].\n");}
    }
    return null;
  };

  this.getNumberAndUnitFromString = function(valueStr)
  {
    var unitsStr = this.supportedUnits.join("|");
    var ourRegExp = new RegExp("(\\-?\\d*\\.?\\d*).*(" + unitsStr + ")");
    var matchArray = ourRegExp.exec(valueStr);
    if (matchArray)
    {
      var retVal = {};
      retVal.number = Number(matchArray[1]);
      retVal.unit = matchArray[2];
      return retVal;
    }
    return null;
  };

  this.compareUnitStrings = function(value1, value2)
  {
    var firstValue = this.getNumberAndUnitFromString(value1);
    var secondValue = this.getNumberAndUnitFromString(value2);
    if ((firstValue === null) || (secondValue === null))
    {
      dump("Problem in msiUnitsList.compareUnitStrings - trying to compare unrecognized units!\n");
      return NaN;
    }
    // convert both to mm
    var firstMm = value1.number *  this.unitSize[value1.unit];
    var secondMm = value1.number * this.unitSize[value2.unit];
    if (firstMm < secondMm)
      { return -1; }
    else if (firstMm > secondMm)
      { return 1;}
    else
      { return 0; }
  };

}

