var msiSearchUtils = 
{
//the following list needs fleshing out a lot!!
  isNonDefaultAttr : function(attrName, attrValue, parentNode)
  {
    var returnVal = true;
    switch(attrName)
    {
      case '_moz-dirty':
      case '_moz_dirty':
      case '-moz-math-font-style':
        returnVal = false;
      break;
      case "limitPlacement":
        if (attrValue == "auto")
          returnVal = false;
      break;
      default:
      break;
    }
    return returnVal;
  },

  isEmptyInputBox : function(aNode)
  {
    if (msiGetBaseNodeName(aNode) == "mi")
    {
    	if (aNode.hasAttribute("tempinput") && (aNode.getAttribute("tempinput") == "true") )
        return true;
    }
    return false;
  },

  isEmptyElement : function(aNode)
  {
    if (this.isEmptyInputBox(aNode))
      return true;

    var nodeName = msiGetBaseNodeName(aNode);
  	var childNodes = msiNavigationUtils.getSignificantContents(aNode);
    if (this.isMRowLike(nodeName) && (childNodes.length > 1))
      return false;
    
  	if (nodeName in this.mathChildPositions)
    {
      for (var ii = 1; (ii <= childNodes.length) && (this.namedChildFromPosition(nodeName, ii) != ""); ++ii)
      {
        if (!this.isEmptyInputBox(childNodes[ii-1]))
          return false;
      }
      return true;
    }

    if (childNodes.length > 1)
      return false;
    if ( (childNodes == null) || (childNodes.length == 0) || this.isEmptyInputBox(childNodes[0]))
      return true;
    return false;
  },

  isMRowLike: function(nodeName)  //includes <msqrt>, <mtd>, <mphantom> etc. as well as <mrow> and <mstyle>
  {
    switch(nodeName)
    {
      case "math":
      case "mrow":
      case "mstyle":
      case "msqrt":
      case "mtd":
      case "mphantom":
      case "mpadded":
      case "menclose":
      case "merror":
        return true;
      break;

      default:
        return false;
      break;
    }
  },

  mathChildPositions :
  {
  	msub : {base : 1, subscript : 2},
  	munder : {base : 1, subscript : 2},
  	msup : {base : 1, superscript : 2},
  	mover : {base : 1, superscript : 2},
  	msubsup : {base : 1, subscript : 2, superscript : 3},
  	munderover : {base : 1, subscript : 2, superscript : 3},
    mroot : {radicand : 1, root : 2},
    msqrt : {radicand : 1},	//how to indicate "all children"?
    mfrac : {numerator : 1, denominator : 2},
  },
    
  positionFromNamedChild : function(nodeName, childName)
  {
  	var retVal = 0;
    if (nodeName in this.mathChildPositions)
    {
      if (childName in this.mathChildPositions[nodeName])
        retVal = this.mathChildPositions[nodeName][childName];
    }
    return retVal;
  },
  
  namedChildFromPosition : function(nodeName, position)
  {
  	var retVal = "";
  	if (nodeName in this.mathChildPositions)
    {
      for (var aChild in this.mathChildPositions[nodeName])
      {
      	if (this.mathChildPositions[nodeName][aChild] == position)
        {
          retVal = aChild;
          break;
        }  
      }
    }
    return retVal;
  },
  
  //"nWhichChild" is to be passed in 0-based (as obtained from childNodes list); the string to be constructed
  //will be 1-based for XPath.
  
  getMatchPositionForChild : function(sourceNodeName, targNodeName, nWhichChild)
  {
  	var nPosition = 0;
    var childName = this.namedChildFromPosition(sourceNodeName, nWhichChild + 1);
    if (childName.length > 0)
      nPosition = this.positionFromNamedChild(targNodeName, childName);
    
    if (nPosition > 0)
    {
      if (this.isMRowLike(targNodeName))
      	return "*";
      return nPosition.toString();  
//      return "./*[" + nTargChild + "][self::";
    }  
    return "";  
  },

  getTypeOfNodeSearch : function(aNode, anEditor)
  {
    var retType = "find";
    switch(msiGetBaseNodeName(aNode))
    {
      case "math":
      case "mphantom":
        retType = "container";
      break;

      case "mrow":
      case "#document-fragment":
        if (!msiNavigationUtils.isFence(aNode))
          retType = "anonContainer";
      break;

      case "mstyle":
        retType = "styleAttribs";
      break;

      default:
        if (msiNavigationUtils.isDefaultParaTag(aNode, anEditor))
          retType = "anonContainer";
        else if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
          retType = "text";
        else
        {
          var tagType = msiNavigationUtils.getTagClass(aNode, anEditor);
          switch(tagType)
          {
            case "structtag":
            case "texttag":
            case "paratag":
            case "envtag":
            case "listtag":
              retType = "container";
            break;

            case "othertag":
            default:
            break;
          }
        } 
      break;
    }
    return retType;
  }

};

var XPathStringFormatterBase = 
{
//  templatePatterns : {
//      replaceableExpressionRE : new RegExp("!-([^\\s\\-]+)-!", "g"),
//      followingNodesExpr : "[following::{!-targetExpression-!}]",
//      mathNodeExpr : "./mi|./mn|./mrow|./msqrt|./mroot|./mstyle|./mfrac|./mo",  //and what else?
//      hasOneChild : "[(./*[1]) and not(./*[2])]",
//      sqRootExpr : "[!-axisName--!::msqrt|(//mroot[(!_mathNodeExpr_!)[2][(@tempinput="true") or (string()="2")]])"
//    },  
//
//  bStringsInitialized : false,
//
//  initStrings : function()
//  {
//    for (var aPattern in this.templatePatterns)
//    {
//      var newString = this.templatePatterns[aPattern];
//      var oldString = "";
//      while (oldString != newString)
//      {
//        oldString = newString;
//        newString = formatXPathString(oldString, "");
//      }
//      this.templatePatterns[aPattern] = newString;
//      this.bStringsInitialized = true;
//    }
//  },
//
//  getMatchingNodeInfoStrings : function(baseNode)
//  {
//    var retData = new Array();
//    var baseName = msiGetBaseNodeName(baseNode);
//    switch(baseName)
//    {
//      case "mroot":
//        //We want to put the part that comes either after "axis::" at the outer level or inside the "[]" at a nested level?
//        var firstSearchString = "mroot[./*[1]!-childExpr1-!][./*[2]!-childExpr2-!]";
//        var secondSearchString = "msqrt[.//!-childExpr1Contents-!]";
//      break;
//
//      case "msqrt":
//        var firstSearchString = "msqrt[.//!-Contents-!]";
//        var secondSearchString = "mroot[./*[2]
//      break;
//    }
//  },

  getMatchingNodeInfo : function(baseNodeName, flags)
  {
    var baseData = new Object();
    baseData.theName = baseNodeName;
    var retData = new Array(baseData);

    switch(baseNodeName)
    {
      case "mroot":
        var secondData = new Object();
        secondData.theName = "msqrt";
//        secondData.contentsMatch = new Array([1,*]);   //means the first child of mroot should be matched by any children of msqrt, and the second shouldn't be matched
//        retData.push(secondData);
//        Bad syntax!
//        Actually, this is all wrong. <mroot> is one of the elements which can have an arbitrary number of children - "inferred mrow"...
//        How do we deal with this? Trying to generate a string like:
//          [.//msqrt[.//content-expr1][.//content-expr2]...[.//content-exprN]]
      break;

      case "msqrt":
        var secondData = new Object();
        secondData.theName = "mroot";
//        secondData.contentsMatch = new Array([1,1]);
        secondData.additionalTest = "[(./mi|./mn|./mrow|./msqrt|./mroot|./mstyle|./mfrac|./mo)[2][(@tempinput=\"true\") or (string()=\"2\")]]";
        retData.push(secondData);
      break;

      case "msup":
      case "msub":
      case "munder":
      case "mover":
        var secondData = new Object();
        secondData.theName = "msubsup";
//        if (baseNodeName == "msup" || baseNodeName == "mover")
//          secondData.contentsMatch = new Array([1,1],[2,3]);
//        else
//          secondData.contentsMatch = new Array([1,1],[2,2]);
        retData.push(secondData);
        var thirdData = new Object();
        if (baseNodeName == "msup")
          thirdData.theName = "mover";
        else if (baseNodeName == "msub")
          thirdData.theName = "munder";
        else if (baseNodeName == "mover")
          thirdData.theName = "msup";
        else if (baseNodeName == "munder")
          thirdData.theName = "msub";
//        thirdData.contentsMatch = new Array([1,1],[2,2]);
        retData.push(thirdData);
        var fourthData = new Object();
        fourthData.theName = "munderover";
//        if (baseNodeName == "msup" || baseNodeName == "mover")
//          fourthData.contentsMatch = new Array([1,1],[2,3]);
//        else
//          fourthData.contentsMatch = new Array([1,1],[2,2]);
        retData.push(fourthData);
      break;

      case "msubsup":
      case "munderover":
        var secondData = new Object();
        secondData.theName = "msub";
//        secondData.contentsMatch = new Array([1,1],[2,2]);
        retData.push(secondData);
        var thirdData = new Object();
        thirdData.theName = "msup";
//        thirdData.contentsMatch = new Array([1,1],[3,2]);
        retData.push(thirdData);
        var fourthData = new Object();
        if (baseNodeName == "msubsup")
          fourthData.theName = "munderover";
        else
          fourthData.theName = "msubsup";
//        fourthData.contentsMatch = new Array([1,1],[2,2],[3,3]);
        retData.push(fourthData);
        var fifthData = new Object();
        fifthData.theName = "munder";
//        fifthData.contentsMatch = new Array([1,1],[2,2]);
        retData.push(fifthData);
        var sixthData = new Object();
        sixthData.theName = "mover";
//        sixthData.contentsMatch = new Array([1,1],[3,2]);
        retData.push(sixthData);
      break;

      case "mrow":
      case "mstyle":
      case "math":
        retData[0].theName = "*";
//        var secondData = new Object();
//        secondData.theName = "mrow";
//        retData.push(secondData);
      break;

      case "mo":
        //Should worry about "embellished operators", but the "approximate match" algorithm intended for XPath means
        //they'd be picked up equally anyway, since we'll be looking for things like ".//mo[...]"
      break;

      case "mi":
      case "mn":
        //probably don't want to match these to anything other than themselves
      break;
    }

    return retData;
  }
  
};

function XPathFormatter(targetNode, flags)
{
  this.mNode = targetNode;
  this.mFlags = flags;
  this.mMatchInfo = XPathStringFormatterBase.getMatchingNodeInfo( msiGetBaseNodeName(this.mNode), this.mFlags);

  this.isOuterLevel = function()
  {
    return (this.mFlags) && (this.mFlags.indexOf("n") < 0);  //"n" for "nested"?
  };

  this.formatNodeTest = function(templateString)
  {
    function doReplacement(bigMatch, subMatch)
    {
      if ( (subMatch!=null) && (subMatch.length > 1) && (subMatch in this.templatePatterns) )
        return this.templatePatterns[subMatch];
      return bigMatch;
    }
    return templateString.replace(this.replaceableExpressionRE, doReplacement);
  };

  this.prepareAttributeTest = function()
  {
    var attrStr = "";
    var attrs = this.mNode.attributes;
    if (attrs != null)
    {
      for (var ix = 0; ix < attrs.length; ++ix)
      {
        var theAttr = attrs.item(ix);
        if (msiSearchUtils.isNonDefaultAttr(theAttr.name, theAttr.value, this.mNode))
          attrStr += "[@" + theAttr.name + "=" + theAttr.value + "]";
      }
    }
    return attrStr;
  };

  this.getSearchableContentNodes = function(aNode)
  {
    var retArray = new Array();
    var theContents = msiNavigationUtils.getSignificantContents(aNode);
    if (!msiSearchUtils.isMRowLike(msiGetBaseNodeName(aNode)))
      return theContents;

    for (var jx = 0; jx < theContents.length; ++jx)
    {
      switch(msiGetBaseNodeName(theContents[jx]))
      {
        case "mrow":
        case "mstyle":
          retArray = retArray.concat(this.getSearchableContentNodes(theContents[jx]));
        break;

        default:
          retArray.push(theContents[jx]);
        break;
      }
    }
    return retArray;
  };

//NOTE: It's assumed that this will only be called in the presence of an existing test node set, so that it's all to be put
//  inside "[]" brackets and evaluated as a condition. If we encounter something like an <mrow> in the open, it'll be up to 
//  the calling code to deal with the true outer-level search string. (So this should happen in the generic prepareXPathStringForNode
//  code at the latest.)
  this.prepareContentsTest = function(targNodeName)
  {
    var contentsStr = "";
//    var theMatchInfo = this.mMatchInfo[matchInfoIndex];
	  var ourBaseName = msiGetBaseNodeName(this.mNode);
    var childFlags = this.mFlags + "n";
    var theContents = this.getSearchableContentNodes(this.mNode);
    if (msiSearchUtils.isEmptyElement(this.mNode))
      return contentsStr;

    if (msiSearchUtils.isMRowLike(ourBaseName))  //includes <msqrt>, <mtd>, <mphantom> etc. as well as <mrow> and <mstyle>
    {
      var startNode = 0;
      var endNode = theContents.length; //or length-1??
      var currSelector = ".";
      if (targNodeName == "mroot" && ourBaseName == "msqrt")
      	currSelector = "./*[1]";
      
      if (msiNavigationUtils.isFence(this.mNode))
      {
        var openingFormatter = new XPathFormatter(theContents[0], childFlags);
        contentsStr += "[" + currSelector + "//*[1][" + openingFormatter.prepareXPathStringForNode("self::");
        var closingFormatter = new XPathFormatter(theContents[endNode], childFlags);
        contentsStr += "]/../*[position()=last()][" + closingFormatter.prepareXPathStringForNode("self::") + "]";
        ++startNode;
        --endNode;
      }
      for (var jx = startNode; jx < endNode; ++jx)
      {
        if (!msiSearchUtils.isEmptyInputBox(theContents[jx]))
        {
          var contentFormatter = new XPathFormatter(theContents[jx], childFlags);
          contentsStr += "[" + contentFormatter.prepareXPathStringForNode("descendant-or-self::") + "]";
        }
      }
    }
    else  //a template or leaf?
    {
      switch(ourBaseName)
      {
        case "mi":
        case "mo":
        case "mn":
          if (!msiSearchUtils.isEmptyInputBox(this.mNode))
            contentsStr = "[text()=\"" + msiNavigationUtils.getLeafNodeText(this.mNode) + "\"]";
        break;

        default:
          var theContents = msiNavigationUtils.getSignificantContents(this.mNode);
          for (var jx = 0; jx < theContents.length; ++jx)
          {
            //START HERE! do test for empty content, as in an input box. Strangely, we don't seem to have any such test already available.
          	if (!msiSearchUtils.isEmptyInputBox(theContents[jx]))
            {
              var matchPosition = msiSearchUtils.getMatchPositionForChild(ourBaseName, targNodeName, jx);
              if (matchPosition != "0")
              {
                if ( (matchPosition == "*") || msiSearchUtils.isMRowLike(targNodeName))
                {
                  contentsStr += "[" + contentFormatter.prepareXPathStringForNode("descendant-or-self::") + "]";
                }  
                else  
                {
                  contentsStr += "[./*";
                  contentsStr += "[" + matchPosition + "]";
                  var contentFormatter = new XPathFormatter(theContents[jx], childFlags);
//                  contentsStr += "[" + contentFormatter.prepareXPathStringForNode("self::") + "]]";
                  contentsStr += "[" + contentFormatter.prepareXPathStringForNode("descendant-or-self::") + "]]";
                }  
              }
            }  
          }
        break;
      }
    }

    return contentsStr;
  };

//Explanation for the following method:
//The idea is to prepare a string which tests for the current node. A search axis should be specified somewhere; then we're 
//trying to put either "axis::<node specification>" or "[axis::<node specification>]" into the path.
//When we go to add a contents specification, it's a little trickier. If our search node is <msup> then we want something like
// "msup[./*[1][first child specification]][./*[2][second child specification]]" or we can match 
// "msubsup[./*[1][first child specification]][./*[3][second child specification]]" etc.
//If our search node is an <mrow>, the matching should just look like "*[.//first child specification][.//second child spec]...".
//Thus, the meaning of "prepareXPathStringForNode" should be "prepare an expression which can be plugged in for the specification
// strings mentioned above". In fact, we'll assume a "prefix string" like for instance ".//" will be passed in which should be
// prepended to each of our possible match strings.

  this.prepareXPathStringForNode = function(prefixStr)
  {
    var retStr = "";
    var bOuterLevel = this.isOuterLevel();
    //Prepare attribute test if non-default attribute values set on search node
    var attrTest = this.prepareAttributeTest();
    //Prepare content tests if necessary
    
//We don't concern ourselves with the "[]" part of this, unless we're at the outer level. Each node,
// when preparing its "contents" match string should deal with that part. What we put out should begin with the name
// of a matching node, unless we have multiple matches in which case we are responsible for the surrounding "()".
//However (!) to formulate the pieces of the "or" correctly, we do have to have a "prefix" string passed in. So we
//    if (!bOuterLevel)
//    {
//      retStr += "[";
//    }
	  var ourBaseName = msiGetBaseNodeName(this.mNode);
    if (bOuterLevel && ((ourBaseName == "mrow") || (ourBaseName == "mstyle") || (ourBaseName == "math")) )
    {
      var theContents = this.getSearchableContentNodes(this.mNode);
      if (theContents.length > 0)
      {
        var childFlags = this.mFlags + "n";
        var childPrefixStr = "following::";
        var contentFormatter = new XPathFormatter(theContents[0], childFlags);
        retStr = contentFormatter.prepareXPathStringForNode(prefixStr);
        for (var jj = 1; jj < theContents.length; ++jj)
        {
          contentFormatter = new XPathFormatter(theContents[jj], childFlags);
          retStr += "[" + contentFormatter.prepareXPathStringForNode(childPrefixStr) + "]";
        }
      }
    }

    if (this.mMatchInfo.length > 1)
      retStr += "(";
    for (var ix = 0; ix < this.mMatchInfo.length; ++ix)
    {
      if (ix > 0)
      {
        if (bOuterLevel)
          retStr += "|";
        else
          retStr += ") or (";
      }

      retStr += prefixStr;
//      retStr += this.mMatchInfo[ix].theName;
      retStr += "*";
      if (this.mMatchInfo[ix].theName != "*")
        retStr += "[local-name()=\"" + this.mMatchInfo[ix].theName + "\"]";
      if ("additionalTest" in this.mMatchInfo[ix])
        retStr += this.mMatchInfo[ix].additionalTest;
      if (attrTest.length > 0)
        retStr += attrTest;
      var contentsTest = this.prepareContentsTest(this.mMatchInfo[ix].theName);
      if (contentsTest.length > 0)
        retStr += contentsTest;
    }
//    if (!bOuterLevel)
//    {
    if (this.mMatchInfo.length > 1)
      retStr += ")";
//      retStr += "]";
//    }
    dump( "in XPathFormatter.prepareXPathStringForNode, for node {" + msiGetBaseNodeName(this.mNode) + "} with prefix string {" + prefixStr + "}; returning {" + retStr + "}.\n" );
    return retStr;
  };

}

XPathFormatter.prototype = XPathStringFormatterBase;

function msiSearchManager(targEditorElement, searchDocFragment, searchFlags)
{
  this.LongTextLimit = 30;  //Just a wild guess
  this.mTargetEditor = msiGetEditor(targEditorElement);
  this.mSearchDocFragment = searchDocFragment;
  if (targEditorElement == null)
    dump("In msiFindUtils.js, in msiSearchManager initializer, targEditorElement is null!\n");
  else
    msiDumpWithID("In msiFindUtils.js, in msiSearchManager initializer, targEditorElement is [@].\n", targEditorElement);
  this.mTargetDocument = this.mTargetEditor.document;
  this.mSearchFlags = searchFlags;
//  this.mSearchRange = searchRange;
//  this.mAmbientTags = new Object();
  this.mXPathSearchNode = null;  //We want to establish which node we want the returning nodesets to target.
  this.mXPathSearchString = null;
  this.mTextSearchString = null;  //If this is filled in, it means we're going with a preliminary FindService search instead of XPath.
  this.mTargetNode = null;  //Here we store the node identified as our target to find. It's likely not to be the root of the whole search
                            //  expression, so we save it to work from later when doing a more thorough test of a match.

  this.isCaseInsensitive = function()
  {
    return (this.mSearchFlags.indexOf("i") >= 0);
  };

  this.setCaseInsensitive = function(bDoSet)
  {
    if (this.isCaseInsensitive() != bDoSet)
    {
      if (bDoSet)
        this.mSearchFlags += "i";
      else
        this.mSearchFlags = this.mSearchFlags.replace("i", "", "gi");
    }
  };

  this.isNonDefaultNode = function(aNode)
  {
    switch(aNode.nodeName)
    {
      case "dialogbase":
      case "sw:dialogbase":
        return false;
      break;

      default:
        if (msiNavigationUtils.isDefaultParaTag(aNode, this.mTargetEditor))
          return false;
      break;
    }
    return true;
  };


  //The following "search types" are being used here:
  //  "find" - This means the node is an immediate candidate to search for. A priori, this is any tag, unless we "know better".
  //  "container" - This means that the node is to be found in the search, BUT it can't necessarily be looked for directly. The
  //                most important cases are nodes which provide an environment of some sort (such as <math> or a paragraph or 
  //                environment tag, so that a match may take place entirely within them. The key is to look for the contents 
  //                first; if they're found, we check ancestors for these tags.
  //  "anonContainer" - This means the node's contents are the real objects of interest, and the node need not be present at all in
  //                    a match. The key example is <mrow> (except for those which are really fences); though we treat <mstyle> 
  //                    similarly for most purposes, it's in some sense closer to the "container" class above, but is handled below.
  //  "styleAttribs": - This is a node - and <mstyle> is the main or only one - which need not appear in a match or in the ancestor
  //                    chain of a match at all, but which imposes attributes which should be looked for in the ancestor chain.
  //  "text" : - This pretty much means that it's a text node as usual. The point is that as a search type, it means we'll use
  //             the FindService instead of XPath at all.  
  //
  //Given these definitions, what's the strategy?
  //  1. Take the first (top-level) node. If its type is "find", it's our target. HOWEVER, if it has siblings they may (should) 
  //     be added to the target, as qualifiers: 
  //                       //mo[text()='sin'][following::mi[text()='x']]
  //  2. If the type is "container" or "styleAttribs", we want to simply "mark" it for later and move on to examine its contents.
  //     (How do we store this information? Recall, though, that this is occurring within a recursive function, so maybe storage
  //     isn't an issue at all. This should probably just be dealt with when control returns from the function call applied to the contents.
  //  3. If the type is "text", we really want to establish some sort of length test - doing a text search for "a" is a bad idea, but
  //     regardless of what other structure may be present in the search expression, doing a search for three hundred words is
  //     best done by the FindService. Now, all the cases in between....
  //  4. If the type is "anonContainer", we want to simply pass on to the child nodes. The exception here may be that if we want to
  //     look for a sequence of (roughly-speaking, sibling) nodes, it may be most useful to simply return the container as the target.
  //     On the other hand, the only containing node in the search expression may be the DocFragment, which wouldn't be awfully useful.
  //     So I judge it's better to use an array of nodes as the "search target"? Or perhaps just to record all the ones that "aren't first"
  //     in an array of "conditions".
  //What's left to determine is what happens if - e.g. - we drill down into the contents of a node and find only a "text" node there to search
  //  for, then bubble back up and proceed to the next sibling where we find something more interesting. Or we find first a grandchild node
  //  of class "find" and then want to apply as a condition a node we find as a great-uncle. Does all of this successfully get subsumed
  //  by "following::"?
  //
  //The plan emerges:
  //  1. If (ourNode is of type "find"), return it and we're done.
  //  2. Otherwise call the function recursively on each of our children, but not stopping when a success is found. Rather, after a
  //     "find" node is encountered. But isn't this already what we were doing???? Yes, of course, since the XPath generation itself
  //     is recursive.
  //So - what? We want to choose a node to search for, and then of course both its contents (as in the originally envisioned recursion)
  //  and its siblings will appear as "conditions". The siblings were probably always going to appear in this way, really, by virtue
  //  of the original recursive code. What truly remains is just the determination of A "find" node, including possible renegotiation
  //  if a better one is found later (okay, not renegotiation but just overriding), and keeping track of what ancestor
  //  nodes and attributes are required. But even this is too severe a requirement for the initial XPath search and should just be
  //  handled at the JavaScript stage? Or may this lead in some circumstances to the sort of near-empty search that will return
  //  the entire document, or every paragraph, or every math node, as a match (which is fine if that's what the user typed in to the
  //  search box, but not if it is the result of a failure to capture enough data in setting up the search).
  //The principle that emerges now is to just reflect in the XPath every piece of required structure that we're sure of. In many
  //  cases, this will be overkill, but in some it may provide the only nontrivial part of the search.
  //So, trying again:
  //  1. Try to identify either a single target node for the XPath search or a stretch of text for the FindService search. If one is found
  //     but later in the examination phase a better one is found, simply jettison the first one.
  //  2. When generating the XPath expression, the set of required ancestor nodes and attributes will need to be present for each node.
  //     How can this best be done? Or is it even true? As the nodes are processed and XPath is generated, can the ancestor in the
  //     search expression chain who knows about the requirement simply add it at his level? Given the recursive nature of constructing
  //     the string, this probably isn't safe, but then again it may be. Compiling an array and passing it in at each stage would
  //     result in unnecessary and arbitrarily redundant repetition. The best thing is probably to go with having the ancestor node
  //     append, to the presumably well-formed XPath for the node being searched for, a condition like
  //        [ancestor-or-self::*[@displaystyle][1][@displaystyle='true']]
  //  3. What will then remain to be done is to reflect all of the above in doing the JavaScript match checking that follows either the
  //     XPath or the FindService search. Presumably, more of this code has to be written before determining whether this is difficult
  //     to track or not - the hope has been that it isn't difficult, and that each node doing matching will be able to check its own
  //     set of conditions. It would seem, though, that that isn't going to do it. It is at this level that the array of conditions
  //     imposed by ancestor nodes may be necessary to keep passing in. Thus, if "displaystyle='true'" is to be a requirement on a
  //     binomial node, the search should succeed if only the binomial node has the attribute (though of course a "binomial node" is
  //     really a particular kind of <mrow>), though the attribute appears in the parent <math> node in the search expression? The
  //     question here is whether each level of the JavaScript should check all the requisite ancestor conditions...
  //Most of the caveats at this stage can be resolved if necessary later. The XPath vs. FindService choice, and generation of
  //  search strings, should be completable now so that they can be tried out on a number of examples. Conditions arising from
  //  ancestor nodes will be added by the ancestors, and the choice of target node and text vs. XPath will be pretty ad hoc for the
  //  time being. Something like "text if there are more than five characters in the text string or there are no nontrivial XPath
  //  candidates".

  //Addendum: The search for a sequence of siblings (or near-siblings, if you ignore <mrow> nesting an dthe like), at the top level
  //  has still not been well specified here. We probably need to keep track of which containing tags need to be polled once we find
  //  our target node? The expressions should look something like:
  //  .//mfrac[ancestor::parentTag//mo[@fence='true']]
  //Using the sibling axis wouldn't be too good - the nodes of interest may be cousins or uncles. the following:: axis would be 
  //  okay, I suppose, but unqualified would allow finding anything in the rest of the document. What would be ideal would be if
  //  we could flag an ancestor node - if we're in math, it would suffice to insist on being in the same <math> ancestor, for instance.
  //  But a priori the search pattern may just be a sequence of nodes - what common ancestor can we count on? I think the target node
  //  search must keep track as it descends of the top guy.

  this.compareTargetData = function(newCandidateData, oldCandidateData)
  {
    var returnData = oldCandidateData;
    var bUseNewData = false;
    switch(oldCandidateData.theType)
    {
      case "find":
        if ((newCandidateData.theType == "text") && (newCandidateData.theNode.textContent.length > this.LongTextLimit))
          bUseNewData = true;
      break;

      case "container":
      case "styleAttribs":
        switch(newCandidateData.theType)
        {
          case "find":
          case "text":
            bUseNewData =  true;
          break;
          default:
          break;
        }
      break;

      case "anonContainer":
        bUseNewData = ((newCandidateData.theType.length > 0) && (newCandidateData.theType != "anonContainer"));
      break;

      case "text":
        if ((newCandidateData.theType == "find") && (oldCandidateData.theNode.textContent.length <= this.LongTextLimit))
          bUseNewData = true;
        if (newCandidateData.theType == "text")
          bUseNewData = (newCandidateData.theNode.textContent.length > oldCandidateData.theNode.textContent.length);
      break;

      default:
        if (newCandidateData.theType.length > 0)
          bUseNewData = true;
      break;
    }

    if (bUseNewData)
    {
      returnData.theNode = newCandidateData.theNode;
      returnData.theType = newCandidateData.theType;
    }

    if ((oldCandidateData.topType == "anonContainer") && (newCandidateData.topNode != null) && (newCandidateData.topNode != oldCandidateData.topNode))
    {
      var relativePos = newCandidateData.topNode.compareDocumentPosition(oldCandidateData.topNode);
      if ( (relativePos & nsIDOMNode.DOCUENT_POSITION_CONTAINS) != 0 )
      {
        returnData.topNode = newCandidateData.topNode;
        returnData.topType = newCandidateData.topType;
      }
      else if ( (relativePos & nsIDOMNode.DOCUENT_POSITION_CONTAINED_BY) == 0 )
      {
        returnData.topNode = null;
        returnData.topType = "";
      }
    }

    return returnData;
  };

  this.findTargetNode = function(parentNode)
  {
    //This doesn't seem like the right thing yet - we need to decide whether we're looking at a text node or not,
    // and make some rational decision on a node set to search for - we may have a sequence of nodes, some containing
    // only text to search for (and being perhaps default container or paragraph nodes), and some with more structure.
    // They may not even be direct siblings. Should we return an array of nodes instead? But then we need to know the
    // real relationship between them in order to set up the search correctly.
    //One of the most important keys here may be to separate the nodes which must be contained in the search pattern as
    // opposed to nodes which may be "ambient" - for instance, a search pattern contained in a <math> tag need not find
    // a <math> tag in order to match - it need only find matching contents which are inside a <math> tag (so that the pattern
    // may be found within the contents of a <math> tag without the beginning of the <math>).

    dump("In msiFindUtils.js, in msiSearchManager.findTargetNode for node [" + parentNode.nodeName + "].\n");
    var nodeData = new Object();
    nodeData.theNode = parentNode;
    nodeData.theType = msiSearchUtils.getTypeOfNodeSearch(parentNode, this.mTargetEditor);
    if ( (nodeData.theType != "anonContainer") && (nodeData.theType != "styleAttribs") )
    {
      nodeData.topNode = parentNode;
      nodeData.topType = nodeData.theType;
    }
    else
    {
      nodeData.topNode = null;
      nodeData.topType = "";
    }

//    if (nodeData.theType == "find")
//      return nodeData;

    var childData = null;
    var childTop = null;
    for (var ix = 0; ix < parentNode.childNodes.length; ++ix)
    {
      childData = this.findTargetNode(parentNode.childNodes[ix]);
      nodeData = this.compareTargetData(childData, nodeData);
    }

    if (nodeData.topNode = null)
    {
      nodeData.topNode = parentNode;
      nodeData.topType = nodeData.theType;
    }
    return nodeData;
  };

  this.setUpSearch = function(axisStr, additionalConditionStr)
  {
    if ( (axisStr == null) || (axisStr.length == 0) )
      axisStr = "following::";

    var searchNodeData = this.findTargetNode(this.mSearchDocFragment);
    this.mTargetNode = searchNodeData.theNode;

    switch(searchNodeData.theType)
    {
      case "text":
        this.mTextSearchString = searchNodeData.theNode.textContent;
      break;

      case "find":
      case "container":
      case "anonContainer":
      case "styleAttribs":
        //START HERE??
        //Need to use the "topNode" stuff now, and deal with multiple sibling nodes at the outer level.
        var contentFormatter = new XPathFormatter(searchNodeData.theNode, this.mSearchFlags);
        var prefixStr = axisStr;
        this.mXPathSearchString = contentFormatter.prepareXPathStringForNode(prefixStr);
        if ( (additionalConditionStr != null) && (additionalConditionStr.length != 0) )
          this.mXPathSearchString += additionalConditionStr;
      break;
    }
  };

//  this.getSearchString = function()
//  {
//    if (this.mTextSearchString != null)
//      return this.mTextSearchString;
//    return this.mXPathSearchString;
//  };

  this.getXPathSearchString = function()
  {
    return this.mXPathSearchString;
  };

  this.getTextSearchString = function()
  {
    return this.mTextSearchString;
  };
}