
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
      editor.selectElement(element);
      var selectionController = editor.selectionController;
      selectionController.scrollSelectionIntoView(selectionController.SELECTION_NORMAL, 
        selectionController.SELECTION_ANCHOR_REGION, true);
      msiResetStructToolbar(editorElement);
      editorElement.focus();
    }
  }
}

function truefunc(x) {return true;}

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
  taglist = theTagManager.getTagsInClass('structtag',',',false); 
  fulltagarray = taglist.split(',');
  if (document.getElementById("TOC").hasAttribute('checked'))
  {
    tagarray = fulltagarray.filter(truefunc);
  }
  else
  {
    tagarray = [];
  }
  var otherstest="";
  var doLOF = document.getElementById("LOF").hasAttribute('checked');
  if (doLOF)
  {
    otherstest = ".//html:img";
  }
  var doLOT = document.getElementById("LOT").hasAttribute('checked');
  if (doLOT)
  {
    otherstest += ((otherstest.length?"|":"") + ".//html:table");
  }
  var i, length;
  var xpath="html:xxxx";
  length = tagarray.length;
  for (i=0; i< length; i++)
  {
    xpath += "|html:" + tagarray[i];
  }
  if ((xpath.length > 0) && (otherstest.length > 0)) otherstest += "|"; 
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
  re = /##otherstest##/g;
  stylestring = stylestring.replace(re,otherstest);
  re = /##LOF##/g
  stylestring = stylestring.replace(re, ""+(doLOF?"html:img":"html:xxximg"));
  re = /##LOT##/g
  stylestring = stylestring.replace(re, ""+(doLOT?"html:table":"html:xxxtable"));
  var parser = new DOMParser();
  var dom = parser.parseFromString(stylestring, "text/xml");
  dump(dom.documentElement.nodeName == "parsererror" ? "error while parsing" : dom.documentElement.nodeName);
  gProcessor.importStylesheet(dom.documentElement);
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
  var extendedarray = tagarray.concat('img','table');
  
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

