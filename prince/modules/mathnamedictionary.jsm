var EXPORTED_SYMBOLS = ["namesdict"];
Components.utils.import('resource://app/modules/pathutils.jsm');

/*
  The mathnames database contains recognized math names and their properties, such as the name,
  the type (function or operator), limit placement, etc. It is persisted as an xml file, and this 
  module exposes it to the JavaScript code. The information is returned using getNameData which 
  returns an object like
    {
      val: <name>,
      type:
      builtin:
      lp: <limit placement>
      engine: <is engine function?>,
      movableLimits:
      size:
    }
*/

var namesdict = {
  bInitialized: false,
  sourcefile: 'mathnames.xml',
  namelistobjs: [],
  savedlistobjs: [],
  orderedstringlist: {},
  namesdoc: {},

  init: function init() {
    if (this.bInitialized) return;
    let i;
    let L;
    let namefile = getUserResourceFile(this.sourcefile, 'xml');
    if (!namefile)
      dump('No mathnames file: '+ this.sourcefile);
    let path = msiFileURLFromAbsolutePath(namefile.target).spec;
    let req = Components.classes['@mozilla.org/xmlextras/xmlhttprequest;1'].createInstance();
    let ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
    ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
    try {                        
      req.open('GET',path,false);
      req.send(null);
      this.namesdoc = req.responseXML;
      if (!this.namesdoc && req.responseText)
        dump('Bad mathnames file: '+ path);
      let nodelist = this.namesdoc.getElementsByTagName('mathname');
      L = nodelist.length;
      let namelistobj;
      let namenode;
      for (i = 0; i < L; i++) {
        namenode = nodelist[i];
        namelistobj = {
          val: namenode.getAttribute('id'),
          type: namenode.getAttribute('type').charAt(0),
          builtin: namenode.getAttribute('builtIn')==='true',
          lp: 0,
          engine: namenode.getAttribute('enginefunction')
        };
        ACSA.addString('mathnames', namelistobj.val);
        this.namelistobjs.push(namelistobj);
        this.savedlistobjs.push(namelistobj);
      }
      ACSA.sortArray('mathnames');
      this.bInitialized = true;
    }
    catch (e) {
      dump('Bad mathnames file: '+ path);
    }
  },

  getNameData: function getNameData(name) {
    this.init();
// #ifndef FUTURE
    let L = this.namelistobjs.length;
    let i;
    for (i = 0; i < L; i++)
      if (this.namelistobjs[i].val === name)
        return this.namelistobjs[i];
    return null;
//#else
//     return this.namelistobjs.filter(function(x) {x.val===name;});
// #endif
  },

  setNameData: function setNameData(name, type, builtin, lp, engine) {
    this.init();
    let obj = this.getNameData(name);
    if (!obj) {
      obj = {val: name};
      this.namelistobjs.push(obj);   
    } 
    obj.type = type;
    obj.builtin = builtin;
    obj.lp = lp;
    obj.engine = engine;
  },

  getNameStrings: function getNameStrings () {
    this.init();
    return this.orderedstringlist;
  }
};



