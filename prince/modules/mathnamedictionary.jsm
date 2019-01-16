var EXPORTED_SYMBOLS = ["mathnames"];

Components.utils.import('resource://app/modules/pathutils.jsm');

var mathnames = {
  bInitialized: false,
  bAutocompleteInitialized: false,
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
    let ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray;1"].getService();
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

function getUserResourceFile( name, resdirname )
{
  var dsprops, userAreaFile, resdir, file, basedir;
  dsprops = Components.classes["@mozilla.org/file/directory_service;1"].getService(Components.interfaces.nsIProperties);
  basedir =dsprops.get("ProfD", Components.interfaces.nsIFile);
  userAreaFile = basedir.clone();
  userAreaFile.append(name);
  if (!userAreaFile.exists())
  { // copy from resource area
    resdir = dsprops.get("resource:app", Components.interfaces.nsIFile);
    if (resdir) resdir.append("res");
    if (resdir) {
      file = resdir.clone();
      if (resdirname && resdirname.length > 0) file.append(resdirname);
      file.append(name);
      try {
        if (file.exists()) file.copyTo(basedir,"");
      }
      catch(e) {
        dump("failed to copy: "+e.toString());
      }
    }
    userAreaFile = file.clone();
  }
  return userAreaFile;
}


