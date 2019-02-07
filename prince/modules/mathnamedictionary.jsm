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
  There are two main im-memory objects:
    - An array of objects as described just above, and
    - An AutoCompleteStringArray (ACSA) consisting of the strings (the names of the objects above) 
    which is used by the mathNames textbox. This is kept ordered.

*/

var namesdict = {
  bInitialized: false,
  sourcefile: 'mathnames.xml',
  namelistobjs: [],
  orderedstringlist: {},
  namesdoc: {},
  ACSA: null,

  init: function init() {
    if (this.bInitialized) return;
    let i;
    let L;
    let namefile = getUserResourceFile(this.sourcefile, 'xml');
    if (!namefile)
      dump('No mathnames file: '+ this.sourcefile);
    let path = msiFileURLFromAbsolutePath(namefile.target).spec;
    let req = Components.classes['@mozilla.org/xmlextras/xmlhttprequest;1'].createInstance();
    this.ACSA = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService();
    this.ACSA.QueryInterface(Components.interfaces.nsIAutoCompleteSearchStringArray);
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
          val: namenode.getAttribute('val'),
          type: namenode.getAttribute('type').charAt(0),
          builtin: namenode.getAttribute('builtIn')==='true',
          lp: 0,
          engine: namenode.getAttribute('enginefunction')
        };
        this.ACSA.addString('mathnames', namelistobj.val);
        this.namelistobjs.push(namelistobj);
      }
      this.ACSA.sortArray('mathnames');
      this.bInitialized = true;
    }
    catch (e) {
      dump('Bad mathnames file: '+ path);
    }
  },

  addNameToStringList: function(nameData, done) { // done is true after a group of names has been
    // added. It triggers a sort of the string array. nameData can be null for this last call.
    this.init();
    if (nameData) {
      this.ACSA.addString('mathnames', nameData.val);
    }
    if (done) this.ACSA.sortArray('mathnames');
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

  getIndex: function getIndex(name) {
    this.init();
// #ifndef FUTURE
    let L = this.namelistobjs.length;
    let i;
    for (i = 0; i < L; i++)
      if (this.namelistobjs[i].val === name)
        return i;
    return -1;
//#else
//     return this.namelistobjs.filter(function(x) {x.val===name;});
// #endif
  },

  remove: function remove(name) {
    this.init();
    var i = this.getIndex(name);
    this.namelistobjs.splice(i,1);
    this.ACSA.deleteString('mathnames',name);
  },

// setNameData allows entering a new mathname or updating properties of an existing one.
// Pass nulls to those properties that shouldn't be changed.
  setNameData: function setNameData(name, type, builtin, lp, engine) {
    this.init();
    let obj = this.getNameData(name);
    if (!obj) { // if it doesn't exist, create a new one.
      obj = {val: name};  // and add it to the list.
      this.namelistobjs.push(obj); 
      this.addNameToStringList(obj, false);  
    } 
    if (type != null) obj.type = type;
    if (builtin != null) obj.builtin = builtin;
    if (lp != null) obj.lp = lp;
    if (engine != null) obj.engine = engine;
    return obj;
  },

  reset: function() {
    this.bInitialized = false;
  },

  save: function() {
    let i;
    let namefile = getUserResourceFile(this.sourcefile, 'xml');
    let node;
    if (!namefile)
      dump('No mathnames file: '+ this.sourcefile);
    let path = msiFileURLFromAbsolutePath(namefile.target).spec;
    let req = Components.classes['@mozilla.org/xmlextras/xmlhttprequest;1'].createInstance();
    try {                        
      req.open('GET',path,false);
      req.send(null);
      this.namesdoc = req.responseXML;
      if (!this.namesdoc && req.responseText)
        dump('Bad mathnames file: '+ path);
      let nodelist = this.namesdoc.getElementsByTagName('mathnames');
      let L = nodelist.length;
      if (L !== 1) 
        dump('Invalid mathnames file');
      let nameroot = nodelist[0];
      let namenodes = nameroot.getElementsByTagName('mathname');
      L = namenodes.length;
      for (i = L-1; i >= 0 ; i--) {
        nameroot.removeChild(namenodes[i]);
      }
      L = this.namelistobjs.length;
      let obj;
      let longtype;
      let theText;
      let data;
      for (i = 0; i < L; i++) {
        obj = this.namelistobjs[i];
        if (obj.val) {
          node = this.namesdoc.createElement('mathname');
          data = this.namesdoc.createElement('data');
          theText = this.namesdoc.createTextNode("insertMathname('"+obj.val+"');");
          node.setAttribute('val',obj.val);
          switch (obj.type) {
            case 'o' : longtype = 'operator';
              break;
            case 'v' : longtype = 'variable';
              break;
            case 'f' : 
            default :  longtype = 'function';
              break;
          }    
          node.setAttribute('type', longtype);
          if (obj.builtin) node.setAttribute('builtIn', 'true');
          data.appendChild(theText);
          node.appendChild(data);
          nameroot.appendChild(node);
        }
        // add something for limit placement?
        // if (obj.lp) node.setAttribute('lp',obj.lp);
      }
      let serializer = Components.classes["@mozilla.org/xmlextras/xmlserializer;1"].createInstance(Components.interfaces.nsIDOMSerializer);
      let xmlstr = serializer.serializeToString(this.namesdoc);
      xmlstr = xmlstr.replace(/(\n)+/g,'');
      xmlstr = xmlstr.replace('<mathname ', '\n  <mathname ','g');
      xmlstr = xmlstr.replace('<mathnames', '\n<mathnames');
      xmlstr = xmlstr.replace('</mathnames', '\n</mathnames');
      writeStringAsFile(xmlstr, namefile);
    }
    catch (e) {
      dump('Bad mathnames file: '+ path);
    }

    return null;  
  }
};

  function writeStringAsFile( str, file )
  {
    var fos = Components.classes["@mozilla.org/network/file-output-stream;1"].createInstance(Components.interfaces.nsIFileOutputStream);
    fos.init(file, -1, -1, false);
    var os = Components.classes["@mozilla.org/intl/converter-output-stream;1"]
      .createInstance(Components.interfaces.nsIConverterOutputStream);
    os.init(fos, "UTF-8", 4096, "?".charCodeAt(0));
    os.writeString(str);
    os.close();
    fos.close();
  }




