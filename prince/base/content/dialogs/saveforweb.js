
function saveforweb( doc, usedirectory, dir )
{
  try {
    var doc2 = document.implementation.createDocument ('http://www.w3.org/1999/xhtml', 'html', null);
    var node;
    node = doc2.importNode(doc.documentElement,true);
  // Perhaps wasteful of memory, but we don't want to disturb our current document
    var cssdir = dir.clone();
    var xbldir = dir.clone();
    cssdir.append('css');
    xbldir.append('xbl');
    if (!cssdir.exists()) cssdir.create(1, 0755);
    if (!xbldir.exists()) xbldir.create(1, 0755);
    var SSs = doc.styleSheets; 
    var i;
    var pi;
    var href;
    var hrefModified, newHRef, oldHRef;
    var root = doc2.getElementsByTagName('html')[0];
    var re = new RegExp("resource://app/res/","");
    for (i = 0; i <SSs.length; i++)
    {
      oldHRef = SSs[i].href;
      if (re.test(oldHRef)) 
      {
        hrefModified = true;
        newHRef = oldHRef.replace(re,"");
      }
      else newHRef = oldHRef;
      var pi = doc2.createProcessingInstruction("xml-stylesheet", "href='"+newHRef+"'", "type='text/css'");
      root.parentNode.insertBefore(pi,root);
      handleStyleSheet(oldHRef, newHRef );  
    }
  }
  catch(e) {
    dump("Error in saveforweb: "+e.message+"\n");
  }
}  

function handleStyleSheet (oldHRef, newHRef)
{
  // if the parameters are different, copy the style sheet to newHRef (relative to dir)
  // while it is open, look for @import and -moz-binding commands that may need to be changed.
  dump("handleStyleSheet("+oldHRef+", "+newHRef+");\n");
}