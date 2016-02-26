var EXPORTED_SYMBOLS = ["UnitHandler" ];


function UnitHandler( editor )
{
  this.supportedUnits = ["mm", "cm", "in", "pt", "bp", "pc", "px", "pct"];
  var pixelsPerInch;
  if (editor) pixelsPerInch =  editor.cssPixelsPerInch;
  if (!pixelsPerInch) pixelsPerInch = 96;
  this.units =  // in mm
    {mm: {size: 1, increment: .1, places: 1 },
     cm: {size: 10, increment: .1, places: 2 },
     "in": {size: 25.4, increment: .1, places: 2 },
     pt: {size: 0.3514598, increment: .1, places: 1 },
     bp: {size: 0.3527778, increment: .1, places: 1 },
     pc: {size: 2.54, increment: .1, places: 1 },
     px: {size: 25.4/pixelsPerInch, increment: 1, places: 0 },
     pct: {size: null, increment: .1, places: 1 },
     "%": {size: null, increment: .1, places: 1 }
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
  this.callback = function(unit) {
                  };

  this.setUnitChangeCallback = function( unitCallback ) {
    this.callback = unitCallback;
  }

  this.getValueAs = function( value, unit )  // given a measurement that is 'value' in the current unit, provide it in the new unit.
  {
    if (!(unit in this.units)){ return null;}
    if (unit === this.currentUnit) { return value; }
    try {
      return (value/this.units[unit].size)*this.units[this.currentUnit].size;
    }
    catch(e) {
      return 0;
    }
  };

  this.getValueOf = function( value, unit )  // given a measurement that is 'value' in the unit "unit"
    // return it in the current unit.
  {
    if (!(unit in this.units)){ return null;}
    if (unit === this.currentUnit) { return value; }
    try {
      return value*this.units[unit].size/this.units[this.currentUnit].size;
    }
    catch(e) {
      return 0;
    }
  };

  this.getValueFromString = function( aString, defaultUnit )
  {
    var valuePair = this.getNumberAndUnitFromString(aString);
    if (!defaultUnit)
      defaultUnit = this.currentUnit;
    if (!valuePair)
      valuePair = {number : Number(aString), unit : defaultUnit};
    return this.getValueOf( valuePair.number, valuePair.unit );
  };

  this.getValueStringAs = function( value, unit )
    //returns same as this.getValueOf together with the unit
  {
    var theVal = this.getValueAs(value, unit);
    return (String(theVal) + unit);
  };

  this.getValueString = function( value )
    //returns value in current unit together with the unit
  {
    return this.getValueStringAs( value, this.currentUnit );
  };
  var len;
  this.setCurrentUnit = function( unit ) // returns the previous value
  {
    var prev = this.currentUnit;
    if (!(unit in this.units))
    {
      return prev;
    }
    var factor = this.units[this.currentUnit].size/this.units[unit].size;
    var limAttr;
    var i;
    var len = this.editFieldList.length;
    for ( i = 0; i < len; i++)
    {
      this.editFieldList[i].setAttribute("increment", this.units[unit].increment);
      this.editFieldList[i].setAttribute("decimalplaces", this.units[unit].places);
      limAttr = this.editFieldList[i].getAttribute("min");
      if (limAttr && (Number(limAttr) != Number.NaN))
        this.editFieldList[i].setAttribute("min", String(Number(limAttr) * factor));
      limAttr = this.editFieldList[i].getAttribute("max");
      if (limAttr && (Number(limAttr) != Number.NaN))
        this.editFieldList[i].setAttribute("max", String(Number(limAttr) * factor));
      this.editFieldList[i].value *= factor;
    }
    this.currentUnit = unit;
    this.callback(unit);
    return prev;
  };

  this.initCurrentUnit = function(unit) // this is for setting the initial value -- no conversions
  {
    var len = this.editFieldList.length;
    var i;
    if (!unit) return;
    for (i = 0;  i < len; i++)
    {
      this.editFieldList[i].setAttribute("increment", this.units[unit].increment);
      this.editFieldList[i].setAttribute("decimalplaces", this.units[unit].places);
      this.editFieldList[i].value *= 1; // force a redisplay
    }
    this.currentUnit = unit;
    this.callback(unit);
  }

  this.getCurrentUnit = function()
  {
    return this.currentUnit;
  };

  this.convertAll = function(newUnit, valueArray )
  {
    var factor = this.units[this.currentUnit].size/this.units[newUnit].size;
    var i;
    var len = valueArray.length;
    for (i = 0; i < len; i++)
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
      } catch (e) {throw("Problem in msiUnitHandler.getDisplayString for unit [" + theUnit + "]: exception is [" + e + "].\n");}
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
      throw("Problem in msiUnitsList.compareUnitStrings - trying to compare unrecognized units!\n");      return NaN;
    }
    // convert both to mm
    var firstMm = firstValue.number *  this.units[firstValue.unit].size;
    var secondMm = secondValue.number * this.units[secondValue.unit].size;
    if (firstMm < secondMm)
      { return -1; }
    else if (firstMm > secondMm)
      { return 1;}
    else
      { return 0; }
  };

  this.buildUnitMenu = function(menulist, initialValue)
  {
    var x;
    var unit;
    var index;
    for (x=0; x < this.supportedUnits.length; x++)
    {
      unit = this.supportedUnits[x];
      menulist.appendItem(this.getDisplayString(unit), unit, "");
      if (unit==initialValue) index=x;
    }
    menulist.value = initialValue;
  }
}

