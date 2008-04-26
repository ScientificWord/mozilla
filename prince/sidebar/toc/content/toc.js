
var gProcessor;
var xpath="html:chapter|html:section|html:subsection|html:subsubsection|html:paragraph|html:appendix";
var tagarray=["chapter","section","subsection","subsubsection","paragraph","appendix"];


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
      msiResetStructToolbar(editorElement);
      var selectionController = editor.selectionController;
      selectionController.scrollSelectionIntoView(selectionController.SELECTION_NORMAL, selectionController.SELECTION_ANCHOR_REGION, false);
      editorElement.focus();
    }
  }
}

function buildTOC()
{
//  BBM todo: build the above from the taglist manager list of structure tags
  var editorElement = msiGetActiveEditorElement();
  var editor;
  if (editorElement) editor = msiGetEditor(editorElement);
  var currentTree = document.getElementById("toc-tree");
  if (currentTree) currentTree.parentNode.removeChild(currentTree);
  ensureAllStructureTagsHaveIds(editor.document);
  if (!gProcessor) gProcessor = new XSLTProcessor();
  else gProcessor.reset();
  var req = new XMLHttpRequest();
  req.open("GET", "chrome://prince/content/toc.xsl", false); 
  req.send(null);
  // print the name of the root element or error message
  var stylestring = req.responseText;
  var re = /##sectiontags##/g;
  stylestring = stylestring.replace(re,xpath);
  var parser = new DOMParser();
  var dom = parser.parseFromString(stylestring, "text/xml");
  dump(dom.documentElement.nodeName == "parsererror" ? "error while parsing" : dom.documentElement.nodeName);
  gProcessor.importStylesheet(dom.documentElement);
  var newFragment;
  if (editor) newFragment = gProcessor.transformToFragment(editor.document, document);
  document.getElementById("table-of-contents").appendChild(newFragment);
  setTOCLevel(document.getElementById("toc-level-scale").value);
}

function ensureAllStructureTagsHaveIds(doc)
{
//  var xpathExpression = ".//chapter | .//section | .//subsection | .//subsubsection | .//paragraph | .//appendix";
//  var nsResolver = doc.createNSResolver(doc.documentElement);
//  var iterator = doc.evaluate(xpathExpression, doc, nsResolver, XPathResult.UNORDERED_NODE_ITERATOR_TYPE, null );
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
  var list;              ;
  var n = 3141592
  for (i=0; i<tagarray.length; i++)
  {
    list=doc.getElementsByTagName(tagarray[i]);
    for (j=0; j<list.length; j++)
    {
      if (!list[j].hasAttribute("id"))
        list[j].setAttribute("id", (n++).toString());
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

