
var gProcessor;


function jump()
{
  var tree=document.getElementById('toc-tree');
  var col=tree.columns.getFirstColumn();
  if (tree.currentIndex >= 0)
  {
    var editorElement = msiGetActiveEditorElement();
    var editor;
    if (editorElement) editor = msiGetEditor(editorElement);
    var element = editor.document.getElementById(tree.view.getCellValue(tree.currentIndex,col));
    if (element) 
    {
      if (element.hasAttribute("type")&&(element.getAttribute("type")=="footnote")) element.setAttribute("hide","false");
      editor.selectElement(element);
      // if element is a footnote, make sure it is displayed
      var selectionController = editor.selectionController;
      selectionController.scrollSelectionIntoView(selectionController.SELECTION_NORMAL, 
        selectionController.SELECTION_ANCHOR_REGION, true);
      msiResetStructToolbar(editorElement);
      editorElement.focus();
    }
  }
  //event.preventDefault();
}

function truefunc(x) {return x.indexOf("(")==0?false:true;}  // don't copy tags that start with "("
// they aren't real tags and they break the XSL code.
 
function buildTOC()
{
//  BBM todo: build the above from the taglist manager list of structure tags
  var editorElement = msiGetActiveEditorElement();
  var editor;
  var tagarray, fulltagarray;
  if (editorElement) editor = msiGetEditor(editorElement);
  var theTagManager = editor ? editor.tagListManager : null;
  var taglist;
  if (!theTagManager) return;
  // get non-structure tags that we want listed
  var otherTags = document.getElementById("taglist").getAttribute('taglist');
  var tagArr = otherTags.split(" ");
  var otherTagArray = tagArr;
  var doLOF, doLOT, doTOC, doTag;
  doTOC = (document.getElementById("TOC").getAttribute('checked')=='true');
  doLOF = (document.getElementById("LOF").getAttribute('checked')=='true');
  doLOT = (document.getElementById("LOT").getAttribute('checked')=='true');
  doTag = (document.getElementById("Tag").getAttribute('checked')=='true');
	if (!(doTOC || doLOF || doLOT || doTag)) return;
  if (doLOF)
    otherTagArray.concat("object");
  if (doLOT)
    otherTagArray.concat("table");

  // get structure tags
  taglist = theTagManager.getTagsInClass('structtag',',',false); 
  fulltagarray = taglist.split(',');
  if (doTOC)
  {
    tagarray = fulltagarray.filter(truefunc);
  }
  else
  {
    tagarray = [];
  }
  var i;
 
  var xpath="html:xxxx|html:"+ tagarray.join("|html:");
  var currentTree = document.getElementById("toc-tree");
  if (currentTree) currentTree.parentNode.removeChild(currentTree);
  ensureAllStructureTagsHaveIds(editor.document, fulltagarray);
  if (!gProcessor) gProcessor = new XSLTProcessor();
  else gProcessor.reset();
  var req = new XMLHttpRequest();
  req.open("GET", "chrome://prince/content/toc.xsl", false); 
  req.send(null);
  // print the name of the root element or error message
  var stylestring = req.responseText;
  var re = /##sectiontags##/g;
  stylestring = stylestring.replace(re,xpath);
  var re = /##othertags##/g;
  stylestring = stylestring.replace(re,"|html:"+tagArr.join("|html:"));
  re = /##LOF##/g;
  stylestring = stylestring.replace(re, ""+(doLOF?"html:object":"html:xxxobj"));
  re = /##LOT##/g;
  stylestring = stylestring.replace(re, ""+(doLOT?"html:table":"html:xxxtable"));
  re = /##TAG##/g;
  doTag=true;
  stylestring = stylestring.replace(re, ""+(doTag?"html:"+tagArr.join("|html:"):"html:xxx"));

  // dump( stylestring+"\n");

  var parser = new DOMParser();
  var dom = parser.parseFromString(stylestring, "text/xml");
  // dump(dom.documentElement.nodeName == "parsererror" ? "error while parsing" : dom.documentElement.nodeName);
  try {gProcessor.importStylesheet(dom.documentElement);}
  catch(e){
    // dump("Error importing xsl sheet: "+e.message+"\n");
  }
  var newFragment;
  if (editor) newFragment = gProcessor.transformToFragment(editor.document, document);
  document.getElementById("table-of-contents").appendChild(newFragment);
  setTOCLevel(document.getElementById("toc-level-scale").value);
}

// //validates given XPath, returns nsXPathExpression if valid, null otherwise
//function getValidXpath(anXpath, aContextNode, aDefaultNS){
//    if (!anXpath || anXpath=='/' || anXpath=='.') return;
//    
//    // if (!xpathEvaluator) 
//    xpathEvaluator = new XPathEvaluator(); //lazy creation of global evaluator
//
//    try {
//        var nsResolver;
//        if (aContextNode || aDefaultNS){
//            var originalResolver = xpathEvaluator.createNSResolver(aContextNode);
//            //nsResolver=originalResolver;
//            //var defNs=findDefautNS(aContextNode);
//            if (aDefaultNS){
//                nsResolver = function lookupNamespaceURI(aPrefix) {
//                        //log('prefix:'+aPrefix);
//                        if (aPrefix=='default') {
//                            return aDefaultNS;
//                        }
//                        return originalResolver.lookupNamespaceURI(aPrefix); 
//                    }
//            }
//            else nsResolver = originalResolver;
//            //log("foo:"+nsResolver.lookupNamespaceURI("foo"));
//            //log("bar:"+nsResolver.lookupNamespaceURI("bar"));
//            //log("alias:"+nsResolver.lookupNamespaceURI("alias"));
//        }
//        else log("no context node");
//        //log('expression:'+anXpath+'   '+aContextNode+ '    res:'+nsResolver);
//        return xpathEvaluator.createExpression(anXpath, nsResolver);
//    }
//    catch(ex) {
//        //log(ex);  //swallow any exception
//        return null
//    }
//}

function ensureAllStructureTagsHaveIds(doc, tagarray)
{
////  var xpathExpression = "//default:chapter | //default:section | //default:subsection | //default:subsubsection | //default:paragraph | //default:appendix";
//  var aDefaultNS = doc.defaultView.expatState.getDefaultNS;
//  var xpath = getValidXPath(xpathExpression, doc.documentElement, aDefaultNS);
//  if (!xpath) return;
//  var nsResolver = doc.createNSResolver(doc.documentElement);
//  var iterator = doc.evaluate(xpath, doc, nsResolver, XPathResult.UNORDERED_NODE_ITERATOR_TYPE, null );
//
//  try {
//    var thisNode = iterator.iterateNext();
//  
//    while (thisNode) {
//      alert( thisNode.firstChild.textContent );
//      thisNode = iterator.iterateNext();
//    }	
//  }
//  catch (e) {
//    dump( 'Error: Document tree modified during iteration ' + e );
//  }
  var i, j;
  var list; 
  var idstring;             
  var n = 3141592;
  var prefix = 'tsid_';//temp structure id
  var regexp= /^(314|tsid_)/;
  var otherTags = document.getElementById("taglist").getAttribute('taglist');
  var tagArr = otherTags.split(" ");
  var extendedarray = tagarray.concat('object','table', tagArr);
  
  for (i=0; i<extendedarray.length; i++)
  {
    list=doc.getElementsByTagName(extendedarray[i]);
    for (j=0; j<list.length; j++)
    {                          
      if (list[j].hasAttribute("id"))
      {
        idstring =list[j].getAttribute("id");
        if (regexp.test(idstring)) list[j].setAttribute("id",prefix+(n++).toString());
      }
      else
      {
        list[j].setAttribute("id", prefix + (n++).toString());
      }
    }
  }
}                    

function setTOCLevel(level)
{
  var view = document.getElementById('toc-tree').view;
  var i;
  for (i=0; i<view.rowCount; i++)
  {
    if (view.isContainerOpen(i) != (view.getLevel(i) < level))
      view.toggleOpenState(i); 
  }
}

function toggleChecked(item)
{
  item.setAttribute("checked", (item.getAttribute("checked")=="true"?"false":"true"));
}