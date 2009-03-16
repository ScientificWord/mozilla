var EXPORTED_SYMBOLS = ["UnitHandler"];


function UnitHandler()
{
  this.supportedUnits = ["mm", "cm", "in", "pt", "bp", "pc", "px", "pct"];
  this.units =  // in mm
    {mm: {size: 1, increment: .1, places: 1 },
     cm: {size: 10, increment: .1, places: 2 },
     "in": {size: 25.4, increment: .1, places: 2 },
     pt: {size: 0.3514598, increment: .1, places: 1 },
     bp: {size: 0.3527778, increment: .1, places: 1 },
     pc: {size: 2.54, increment: .1, places: 1 },
     px: {size: 0.2645833, increment: 1, places: 0 },	// we say 96 pixels/inch. It varies but portability requires a fixed value
     pct: {size: null, increment: .1, places: 1 }
  };

  this.editFieldList = [];

  this.addEditFieldList = function (fieldArray)
  {
    this.editFieldList = this.editFieldList.concat(fieldArray);
  }

  this.setEditFieldList = function (fieldArray)
  {
    this.editFieldList = fieldArray;
  }

  this.currentUnit = null;

  this.getValueAs = function( value, unit )  // given a measurement that is 'value' in the current unit, provide it in the new unit.
  { 
    if (!(unit in this.units)){ return null;}
    if (unit === this.currentUnit) { return value; }
    return (value/this.units[unit].size)*this.units[this.currentUnit].size;
  };

  this.setCurrentUnit = function( unit ) // returns the previous value
  {
    var prev = this.currentUnit;
    if (!(unit in this.units)) 
    {
      return prev;
    }
    var factor = this.units[this.currentUnit].size/this.units[unit].size;
    for (var i = 0, len = this.editFieldList.length; i < len; i++)
    {
      this.editFieldList[i].setAttribute("increment", this.units[unit].increment); 
      this.editFieldList[i].setAttribute("decimalplaces", this.units[unit].places); 
      this.editFieldList[i].value *= factor; 
    }
    this.currentUnit = unit;
    return prev;
  };
   
  this.initCurrentUnit = function(unit) // this is for setting the initial value -- no conversions
  {
    if (!unit) return;
    for (var i in this.editFieldList)
    {
      this.editFieldList[i].setAttribute("increment", this.units[unit].increment); 
      this.editFieldList[i].setAttribute("decimalplaces", this.units[unit].places); 
    }
    this.currentUnit = unit;
  }
   
  this.getCurrentUnit = function()
  {
    return this.currentUnit;
  };

  this.convertAll = function(newUnit, valueArray )
  {
    var factor = this.units[this.currentUnit].size/this.units[newUnit].size;
    for (var i = 0, len = valueArray.length; i < len; i++)
      { valueArray[i] *= factor; }
  };

  this.getDisplayString = function(theUnit)
  {
    if (!(theUnit in this.units)){ return null;}
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
    var firstMm = value1.number *  this.units[value1.unit].size;
    var secondMm = value1.number * this.units[value2.unit].size;
    if (firstMm < secondMm)
      { return -1; }
    else if (firstMm > secondMm)
      { return 1;}
    else
      { return 0; }
  };

}

