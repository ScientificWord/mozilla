//NOTE!! If you want to turn loggin back on in this file, search for //log at the beginning of lines

var bLoggingSearchStuff = false;
//log  bLoggingSearchStuff = true;

var msiSearchUtils = 
{
  //Some defined constants
  completedMatch: 1,
  partialMatch: 2,
  cannotMatch: 3,

  isMatching : function(matchValue)
  {
    return (matchValue != this.cannotMatch);
  },

//the following list needs fleshing out a lot!!
  shouldSearchForAttribute : function(attrName, attrValue, parentNode)
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
      case "xmlns":
        returnVal = false;
      break;
      case "type":
        if (attrValue == "_moz")
          returnVal = false;
      break;
      case "tempinput":
        returnVal = false;
      break;
      case "temp":
        returnVal = false;
      break;
      default:
        if (attrName.indexOf("xmlns:") == 0)
          return false;
      break;
    }
    return returnVal;
  },

  //This list isn't full!!
  attributeIsInheritable : function(attrName)
  {
    switch(attrName)
    {
      case "limitPlacement":
        return false;
      break;
      default:
        return true;
      break;
    }
  },

  shouldAddAttributeToInheritedList : function(attrName, attrVal, parentNode)
  {
    return ( this.shouldSearchForAttribute(attrName, attrVal, parentNode) && this.attributeIsInheritable(attrName) );
  },

//////Note that "refEditor" is used to determine tagtypes. If may be passed in as null.
  shouldAddNodeToInheritedList : function(targNode, refEditor)
  {
    var nodeType = this.getTypeOfNodeSearch(targNode, refEditor);
    switch(nodeType)
    {
      case "textContainer":
      case "blockContainer":
      case "mathContainer":
        return true;
      break;
      default:
      break;
    }
    return false;
  },
  
  isEmptyElement : function(aNode)
  {
    if (msiNavigationUtils.isEmptyInputBox(aNode))
      return true;

    var nodeName = msiGetBaseNodeName(aNode);
  	var childNodes = msiNavigationUtils.getSignificantContents(aNode);
    if (this.isMRowLike(nodeName) && (childNodes.length > 1))
      return false;
    
  	if (nodeName in this.mathChildPositions)
    {
      for (var ii = 1; (ii <= childNodes.length) && (this.namedChildFromPosition(nodeName, ii) != ""); ++ii)
      {
        if (!msiNavigationUtils.isEmptyInputBox(childNodes[ii-1]))
          return false;
      }
      return true;
    }

    if (childNodes.length > 1)
      return false;
    if ( (childNodes == null) || (childNodes.length == 0) || msiNavigationUtils.isEmptyInputBox(childNodes[0]))
      return true;
    return false;
  },

  //This method returns true for objects whose contents are "implied mrows".
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

//////Note that "anEditor" is used to determine tagtypes. If may be passed in as null.
  isContainer : function(aNode, anEditor)
  {
    var nodeName = msiGetBaseNodeName(aNode);
    if (this.isMRowLike(nodeName))
      return true;
    switch( this.getTypeOfNodeSearch(aNode, anEditor) )
    {
      case "mathContainer":
      case "anonContainer":
      case "textContainer":
      case "blockContainer":
      case "styleAttribs":
        return true;
      break;
    }
    return false;
  },

//////Note that "anEditor" is used to determine tagtypes. If may be passed in as null.
  //Is this even right now?
  getContainerParent : function(aNode, anEditor)
  {
    var candidate = aNode.parentNode;
    var retVal = null;
    while ( (retVal == null) && (candidate != null) )
    {
      if (!this.isContainer(candidate, anEditor))
        retVal = candidate;
      else if ( (candidate.parentNode != null) && (!this.isContainer(candidate.parentNode, anEditor)) )
        retVal = candidate;
      candidate = candidate.parentNode;
    }
    return retVal;
  },

  getSingleRangeContent : function(targRange)
  {
    var retVal = null;
    if (targRange.startContainer == targRange.endContainer)  //is this condition necessary? Assume so, for now
    {
      if (targRange.startContainer.childNodes && targRange.startContainer.childNodes.length)
      {
        for (var ix = targRange.startOffset; ix < targRange.endOffset; ++ix)
        {
          if (!msiNavigationUtils.isIgnorableWhitespace(targRange.startContainer.childNodes[ix]))
          {
            if (retVal)     //already found one - so can't return a single child
            {
              retVal = null;
              break;
            }
            retVal = targRange.startContainer.childNodes[ix];
          }
        }
      }
    }
    return retVal;
  },

  getSingleChild : function(aNode)
  {
    var retVal = null;
    if (aNode.childNodes)
    {
      for (var ix = 0; ix < aNode.childNodes.length; ++ix)
      {
        if (!msiNavigationUtils.isIgnorableWhitespace(aNode.childNodes[ix]))
        {
          if (retVal)     //already found one - so can't return a single child
          {
            retVal = null;
            break;
          }
          retVal = aNode.childNodes[ix];
        }
      }
    }
    return retVal;
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
    }  
    return "";  
  },

  getMatchNodeForChild : function(sourceNode, targNode, nWhichChild)
  {
  	var nPosition = 0;
    var targNodeName = msiGetBaseNodeName(targNode);
    var childName = this.namedChildFromPosition(msiGetBaseNodeName(sourceNode), nWhichChild + 1);
    if (childName.length > 0)
      nPosition = this.positionFromNamedChild(targNodeName, childName);
    
    if (nPosition > 0)
    {
      if (this.isMRowLike(targNodeName))
      	return targNode;
      return msiNavigationUtils.getIndexedSignificantChild(targNode, nPosition - 1);
    }  
    return null;  
  },

//////Note that "anEditor" is used to determine tagtypes. If may be passed in as null(?)
  getTypeOfNodeSearch : function(aNode, anEditor)
  {
    var retType = "find";
    switch(msiGetBaseNodeName(aNode))
    {
      case "math":
      case "mphantom":
        retType = "mathContainer";
      break;

      case "mrow":
      case "#document-fragment":
        if (msiNavigationUtils.isFence(aNode))
          retType = "mathContainer";
        else
          retType = "anonContainer";
      break;

      case "mstyle":
        retType = "styleAttribs";
      break;

      default:
        if (this.isMathTemplate(aNode))
          retType = "mathTemplate";
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
            case "listtag":
            case "paratag":
            case "envtag":
            case "frontmtag":
              retType = "blockContainer";
            break;

            case "texttag":
              retType = "textContainer";
            break;

            case "othertag":
            default:
            break;
          }
        } 
      break;
    }
    return retType;
  },

  isMathTemplate : function(aNode)
  {
    var nodeName = msiGetBaseNodeName(aNode);
  	return (nodeName in this.mathChildPositions);
  },

  isTemplate : function(aNode)  //this function is intended to perhaps be more general, but for now it's the same as isMathTemplate()
  {
    return this.isMathTemplate(aNode);
  },

  getSearchableContentNodes : function(aNode)
  {
    var retArray = new Array();
    var theContents = msiNavigationUtils.getSignificantContents(aNode);
    if (!this.isMRowLike(msiGetBaseNodeName(aNode)))
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
  } //,

//  getNextMatchNodeToLeft : function(aNode)
//  {
//    if (!aNode)
//      return null;
//
//    var retNode = aNode.previousSibling;
//    if (retNode != null)
//    {
//      if (this.isMRowLike(retNode))  //Is this enough?? What if we traverse into a different span or something? Rethink...again...
//        retNode = msiNavigationUtils.getLastSignificantChild(retNode);
//    }
//    else if (aNode.parentNode != null)
//    {
//      retNode = this.getNextMatchNodeToLeft(aNode.parentNode);
//    }
//    return retNode;
//  },

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

      case "math":  //If we're not outer-level, don't want to suppress the "local-name()='math'" selector. (If we are outer-level, it's handled below.)
      break;

      case "mrow":
      case "mstyle":
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
        if (msiSearchUtils.shouldSearchForAttribute(theAttr.name, theAttr.value, this.mNode))
//          attrStr += "[@" + theAttr.name + "=" + theAttr.value + "]";
          attrStr += "[@" + theAttr.name + "=\"" + theAttr.value + "\"]";
      }
    }
    return attrStr;
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
    var theContents = msiSearchUtils.getSearchableContentNodes(this.mNode);
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
        if (!msiNavigationUtils.isEmptyInputBox(theContents[jx]))
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
          if (!msiNavigationUtils.isEmptyInputBox(this.mNode))
            contentsStr = "[text()=\"" + msiNavigationUtils.getLeafNodeText(this.mNode) + "\"]";
        break;

        default:
          var theContents = msiNavigationUtils.getSignificantContents(this.mNode);
          for (var jx = 0; jx < theContents.length; ++jx)
          {
            //START HERE! do test for empty content, as in an input box. Strangely, we don't seem to have any such test already available.
          	if (!msiNavigationUtils.isEmptyInputBox(theContents[jx]))
            {
              var matchPosition = msiSearchUtils.getMatchPositionForChild(ourBaseName, targNodeName, jx);
              if ( (matchPosition.length > 0) && (matchPosition != "0") )
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
      var theContents = msiSearchUtils.getSearchableContentNodes(this.mNode);
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
    msiKludgeLogString( "in XPathFormatter.prepareXPathStringForNode, for node {" + msiGetBaseNodeName(this.mNode) + "} with prefix string {" + prefixStr + "}; returning {" + retStr + "}.\n", ["search"] );
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
    msiKludgeLogString( "In msiFindUtils.js, in msiSearchManager initializer, targEditorElement is null!\n", ["search"] );
  else
    msiKludgeLogString( "In msiFindUtils.js, in msiSearchManager initializer, targEditorElement is [" + targEditorElement.id + "].\n", ["search"]);
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
    switch(aNode.localName)
    {
      case "dialogbase":
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
  //  "mathContainer"
  //  "textContainer":
  //  "blockContainer":
  //   - Any of these mean that the node is to be found in the search, BUT it can't necessarily be looked for directly. The
  //                most important cases are nodes which provide an environment of some sort (such as <math> or a paragraph or 
  //                environment tag, so that a match may take place entirely within them. The key is to look for the contents 
  //                first; if they're found, we check ancestors for these tags.
  //  "anonContainer" - This means the node's contents are the real objects of interest, and the node need not be present at all in
  //                    a match. The key example is <mrow> (except for those which are really fences); though we treat <mstyle> 
  //                    similarly for most purposes, it's in some sense closer to the "container" class above, but is handled below.
  //  "styleAttribs" - This is a node - and <mstyle> is the main or only one - which need not appear in a match or in the ancestor
  //                    chain of a match at all, but which imposes attributes which should be looked for in the ancestor chain.
  //  "mathTemplate" - These nodes must find a candidate node to match, and must match specified children against specified
  //                   children of the target (not necessarily the same, for instance, if a <msup> is matching an <msubsup>).
  //  "text"  - This pretty much means that it's a text node as usual. The point is that as a search type, it means we'll use
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

  this.compareTargetData = function(newCandidateData, oldCandidateData, bCompareTopNodes)
  {
    var returnData = {
      theNode : oldCandidateData.theNode,
      theType : oldCandidateData.theType,
      topNode : oldCandidateData.topNode,
      topType : oldCandidateData.topType
    };
    var bUseNewData = false;
    switch(oldCandidateData.theType)
    {
      case "find":
      case "mathTemplate":
        if ((newCandidateData.theType == "text") && (newCandidateData.theNode.textContent.length > this.LongTextLimit))
          bUseNewData = true;
      break;

      case "mathContainer":
      case "textContainer":
      case "blockContainer":
      case "styleAttribs":
        switch(newCandidateData.theType)
        {
          case "find":
          case "mathTemplate":
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
        if ( ( (newCandidateData.theType == "find") || (newCandidateData.theType == "mathTemplate") )&& (oldCandidateData.theNode.textContent.length <= this.LongTextLimit))
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

    if (!bCompareTopNodes)
    {
      returnData.topNode = newCandidateData.topNode;
      returnData.topType = newCandidateData.topType;
    }
    else if ((oldCandidateData.topType == "anonContainer") && (newCandidateData.topNode != null) && (newCandidateData.topNode != oldCandidateData.topNode))
    {
      var relativePos = newCandidateData.topNode.compareDocumentPosition(oldCandidateData.topNode);
      msiKludgeLogString( "In msiFindUtils.js, in msiSearchManager.compareTargetData for new candidate node [" + newCandidateData.theNode.nodeName + "], old candidate node [" + newCandidateData.theNode.nodeName +"]; compareDocumentPosition returned [" + relativePos + "].\n", ["search"]);
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

    var nodeData = new Object();
    var parentType = msiSearchUtils.getTypeOfNodeSearch(parentNode, this.mTargetEditor);
    nodeData.theNode = parentNode;
    nodeData.theType = parentType;
    msiKludgeLogString( "In msiFindUtils.js, in msiSearchManager.findTargetNode for node [" + parentNode.nodeName + "], reported type [" + nodeData.theType + "].\n", ["search"] );
    if ( (nodeData.theType != "anonContainer") && (nodeData.theType != "styleAttribs") )
    {
      nodeData.topNode = parentNode;
      nodeData.topType = parentType;
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
    var childDataStr = "";
    var topNodeStr = "";
    for (var ix = 0; ix < parentNode.childNodes.length; ++ix)
    {
      childData = this.findTargetNode(parentNode.childNodes[ix]);
      nodeData = this.compareTargetData(childData, nodeData, (ix > 0));
      if (childData.topNode != null)
        childDataStr = childData.topNode.nodeName;
      if (nodeData.topNode != null)
        topNodeStr = nodeData.topNode.nodeName;
      msiKludgeLogString( "In msiFindUtils.js, msiSearchManager.findTargetNode for node [" + parentNode.nodeName + "], after comparing child data with top node [" + childDataStr + "], nodeData now has top node [" + topNodeStr + "].\n", ["search"] );
    }

    //If nodeData.topNode isn't null, a node has been found which contains all requisite children. We use it if we're an anonContainer.
    if ( (nodeData.topNode == null) || ( (parentType != "anonContainer") && (parentType != "styleAttribs") ) )
    {
      nodeData.topNode = parentNode;
      nodeData.topType = parentType;
    }
    var topNodeStr = "";
    if (nodeData.topNode != null)
    {
      topNodeStr = nodeData.topNode.nodeName + "] of type [" + nodeData.topType + "] with content [" + nodeData.topNode.textContent;
    }
    msiKludgeLogString( "Leaving msiFindUtils.js, in msiSearchManager.findTargetNode for node [" + parentNode.nodeName + "], top node is [" + topNodeStr + "].\n", ["search"] );
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
      case "mathTemplate":
      case "textContainer":
      case "blockContainer":
      case "mathContainer":
      case "anonContainer":
      case "styleAttribs":
        //START HERE??
        //Need to use the "topNode" stuff now, and deal with multiple sibling nodes at the outer level.
        var topNodeStr = "";
        if (searchNodeData.topNode != null)
        {
          topNodeStr = searchNodeData.topNode.nodeName + "] of type [" + searchNodeData.topType + "] with content [" + searchNodeData.topNode.textContent;
        }
        msiKludgeLogString( "In msiFindUtils.js, msiSearchManager.setUpSearch, target node is [" + this.mTargetNode.nodeName + "] with content [" + this.mTargetNode.textContent + "], while topNode is [" + topNodeStr + "].\n", ["search"] );
        var contentFormatter = new XPathFormatter(searchNodeData.theNode, this.mSearchFlags);
        var prefixStr = axisStr;
        this.mXPathSearchString = contentFormatter.prepareXPathStringForNode(prefixStr);
        if ( (additionalConditionStr != null) && (additionalConditionStr.length != 0) )
          this.mXPathSearchString += additionalConditionStr;
        //NOTE! The following, insisting as it does on finding a single parent node containing all these children, will prevent
        //  a search from matching, for instance, two contiguous pieces of math separated by an empty text node. If we want to
        //  somehow allow for matching across containers, this XPath stuff would need to change. (I don't have a good candidate in mind yet.)
        if ( (searchNodeData.topNode != null) && (searchNodeData.topNode != searchNodeData.theNode) )
        {
          switch(searchNodeData.topType)
          {
            case "container":
            case "mathContainer":
            case "anonContainer":
            case "styleAttribs":
              var topContentFormatter = new XPathFormatter(searchNodeData.topNode, this.mSearchFlags + "n");
              this.mXPathSearchString += "[" + topContentFormatter.prepareXPathStringForNode("ancestor::") + "]";
            break;
          }
        }
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

////  //This function assumes that we have a match of what's to our right with the various ranges (probably just one) in rangeArray.
////  //The first thing to be done is to determine whether we can extend the match at all. If we're some sort of container and we
////  //  have more to our left to match, the answer is tentatively yes. If we're a template of some sort (like a <mfrac>), the answer
////  //  is probably no. 
////  //Then we have to examine each of the ranges to see what we encounter if we look to the left; if we're leaving an object, 
////  //  can our match extend across its boundary? This is generally a difficult one to answer. If the matching node is a template, 
////  //  we really don't want to be in this function anyway, in some sense; a template should try to match each of its component pieces.
////  //  But does that mean we don't end up here or want to sometimes return true? Generally, though, we can return false in that case? What
////  //  should happen if we're a fraction that has matched its denominator and wants to match its numerator, or a fraction which is
////  //  empty and thus matches any fraction? It remains true that we must match an entire <mfrac>, and then can't extend beyond it.
////  this.extendToLeft = function(rangeArray, sourceNode, offset)
////  {
////    while (rangeArray.length > 0)
////    {
////      if (offset > 0)
////
////    }
////    var nextNode = null;
////    var targParent = null;
////    var nextOffset = 0;
////    var nextToTest = null;
////    if ((offset < 0) || (offset > this.mNode.childNodes.length))  //offset = -1 is passed in if the current matching position is to our right
////      nextNode = this.getLastChild();
////    else
////      nextNode = this.mNode.childNodes[offset - 1];
////
////    for (var ix = 0; ix < rangeArray.length; ++ix)
////    {
////      targParent = rangeArray[ix].startContainer;
////      nextOffset = rangeArray[ix].startOffset;
////      NOW WHAT??  Want to look at the targParent to see whether something of our kind even CAN extend into something of their kind.
////      For instance, if we're a template object like a radical, we only extend against other radicals? Or does it depend on our offset?
////    }
////  };
////
////  this.extendToRight = function(rangeArray)
////  {
////  };

//////Idea is:
//////  i) When we've completed matching a node, we move up to its "container parent" rather than to its preceding sibling. The point
//////     is that the container parent, since it's part of the match pattern, has to match something, and is in a better position to
//////     decide whether the match can even potentially be extended or not.
////// ii) If we're going to try extending, the our container parent will begin by finding the (possibly extended) child preceding
//////     the last matched node and try matching that.
  this.verifySearch = function(targRange)
  {
    var foundTargetNode = targRange.commonAncestorContainer;
    if (targRange.startContainer == targRange.endContainer)
    {
      if ( ((targRange.endOffset - targRange.startOffset) <= 1) && (targRange.startContainer.childNodes) )
        foundTargetNode = targRange.startContainer.childNodes[targRange.startOffset];
    }
    var returnVal = false;
    var ourRange = createMsiMatchingRange();
    var tempRange = null;
    var nextNode = null;
//    var targRange = null;
    var nextNode = this.mTargetNode;
//    var ourRanges = new Array();
    var ourMatch = createMatchNode(this.mTargetNode, this.mSearchFlags, this.mTargetEditor);

//    targRange = this.mTargetEditor.document.createRange();
//    targRange.selectNode(foundTargetNode);
    var matchVal = ourMatch.matchARange( targRange );
    returnVal = msiSearchUtils.isMatching(matchVal);
    if (!returnVal)
      return false;

//    if (!msiNavigationUtils.nodeHasContentBeforeRangeStart(targRange, targRange.startContainer))
//      targRange.setStartBefore(targRange.startContainer);
//    if (!msiNavigationUtils.nodeHasContentAfterRangeEnd(targRange, targRange.endContainer))
//      targRange.setEndAfter(targRange.endContainer);

    ourRange.selectNode(this.mTargetNode);
    tempRange = ourRange.cloneRange();
//    ourRanges.push(targRange);
    while (returnVal)
    {
//      nextNode = msiSearchUtils.getNextMatchNodeToLeft(this.mTargetNode);
      nextNode = nextNode.parentNode;
      if (nextNode == null)
        break;
      ourMatch = createMatchNode(nextNode, this.mSearchFlags, this.mTargetEditor);
      //Want to write something like:
      switch( ourMatch.extendMatchToLeft(targRange, ourRange, this.mTargetEditor) )
      {
        case msiSearchUtils.completedMatch:
        case msiSearchUtils.partialMatch:
          returnVal = true;
        break;
        case msiSearchUtils.cannotMatch:
        default:
          returnVal = false;
        break;
      }
    }
    if (!returnVal)
      return returnVal;

    nextNode = ourRange.endContainer;
    do
    {
//      nextNode = msiSearchUtils.getNextMatchNodeToRight(this.mTargetNode);
//      nextNode = nextNode.parentNode;
      if (nextNode == null)
        break;
      ourMatch = createMatchNode(nextNode, this.mSearchFlags, this.mTargetEditor);
      //Want to write something like:
//      switch( ourMatch.extendToRight( ourRanges ))
//      switch( ourMatch.extendMatchToRight(targRange, ourRange, this.mTargetEditor))
      switch( ourMatch.extendMatchToRight(targRange, tempRange, this.mTargetEditor))
      {
        case msiSearchUtils.completedMatch:
        case msiSearchUtils.partialMatch:
          returnVal = true;
        break;
        case msiSearchUtils.cannotMatch:
        default:
          returnVal = false;
        break;
//        case msiSearchutils.finishedToLeft:
      // What may be missing is matching attributes (or more than attributes??) coming from the parent of the search node?
      }
      nextNode = nextNode.parentNode;
    } while (returnVal);
    msiMatchNode.prototype.adjoinRange(ourRange, tempRange);

    return returnVal;
  };
}

//function msiGenericMatchNode(theNode, theFlags, inheritedAttrs)
function msiMatchNode() {}
msiMatchNode.prototype =
{

  init : function(aNode, theFlags, refEditor)
  {
    this.mNode = aNode;
    this.mFlags = theFlags;
    this.mEditor = refEditor;
    this.getInheritedAttribs(aNode, refEditor);
    this.mSearchType = msiSearchUtils.getTypeOfNodeSearch(aNode, refEditor);
    this.lastOffset = msiNavigationUtils.lastOffset(this.mNode);
  },

  matchIsCaseSensitive : function()
  {
    return (this.mFlags) && (this.mFlags.indexOf("i") < 0);  //"i" for "insensitive"?
  },

  getInheritedAttribs : function(ourNode, refEditor)
  {
    var theAttrName = "";
    var theAttrVal = "";
    if (!this.mInheritedAttrs)
      this.mInheritedAttrs = new Object();
    if (!this.mNeededAncestors)
      this.mNeededAncestors = new Array();
    for (var targNode = ourNode; targNode; targNode = targNode.parentNode)
    {
      for (var jx = 0; (targNode.attributes) && (jx < targNode.attributes.length); ++jx)
      {
        theAttrName = targNode.attributes.item(jx).nodeName;
        theAttrVal = targNode.attributes.item(jx).textContent;
        if (msiSearchUtils.shouldAddAttributeToInheritedList(theAttrName, theAttrVal, targNode) )
        {
          if (!(theAttrName in this.mInheritedAttrs))
            this.mInheritedAttrs[theAttrName] = theAttrVal;
        }
      }
      if (msiSearchUtils.shouldAddNodeToInheritedList(targNode, refEditor))
        this.mNeededAncestors.push( targNode.nodeName );
    }
  },

  checkInheritedAttribute : function(targNode, attrName, attrVal)
  {
    for (var parNode = targNode; parNode; parNode = parNode.parentNode)
    {
      if (!("hasAttribute" in parNode))
      {
        msiKludgeLogString( "In msiMatchNode.checkInheritedAttribute, no 'hasAttribute' function for parNode [" + parNode.nodeName + ", " + parNode.textContent + "] - searching for attribute [" + attrName + " = " + attrVal + "].\n", ["search"] );
        //Then just continue looping
      }
      else if (parNode.hasAttribute(attrName))
      {
        if (attrVal == parNode.getAttribute(attrName))
          return true;
        msiKludgeLogString( "In msiMatchNode.checkInheritedAttribute, hasAttribute failed for attribute [" + attrName + " = " + attrVal + "]; actual value was [" + parNode.getAttribute(attrName) + ".\n", ["search"] );
        return false;
      }
    }
    msiKludgeLogString( "In msiMatchNode.checkInheritedAttribute, hasAttribute failed for attribute [" + attrName + " = " + attrVal + "].\n", ["search"] );
    return false;
  },

  checkNeededAncestor : function(targNode, ancestorName)
  {
    for (var parNode = targNode; parNode; parNode = parNode.parentNode)
    {
      if (parNode.nodeName == ancestorName)
        return true;
    }
    msiKludgeLogString( "In msiMatchNode.checkNeededAncestor, failed to find ancestor [" + ancestorName + "].\n", ["search"] );
    return false;
  },

  //Important to be clear about when this function is called. It is only to match a node against the contents of a range. 
  matchARange : function(targRange)
  {
    var retVal = msiSearchUtils.cannotMatch;
    var localTargRange = null;
    var ourRange = null;
    var aTargNode = msiSearchUtils.getSingleRangeContent(targRange);
    if (aTargNode)
      return this.match(aTargNode, targRange); //Note that this call will possibly change targRange, as it's really an "out" value for that function.
    //What if targRange is in a text node? Can we only match then if this.mNode is a text node? However, if this.mNode is a text node we
    //  should be using a different function.
    //Otherwise, targRange contains several children of some parent. We find the first child and try to match it if it's legal for us
    //  to do so - note that templates will be using different functions here. If that succeeds, then we proceed via extendMatchRight.

//rwa 12-23-08 Try just going through this bit for all cases (commented out the "if"):
//    if (msiSearchUtils.isContainer(this.mNode))
//    {
      localTargRange = this.mEditor.document.createRange();
      localTargRange.setStart(targRange.startContainer, targRange.startOffset);
      ourRange = createMsiMatchingRange();
      ourRange.setStart(this.mNode, 0);
      ourRange.setEnd(this.mNode, 0);
      retVal = this.extendMatchToRight(localTargRange, ourRange, this.mEditor);
      if (msiSearchUtils.isMatching(retVal))  //check whether we got all of us
      {
        if (this.haveContentAfterRangeEnd(ourRange))
          retVal = msiSearchUtils.cannotMatch;
        else
        {
          targRange.setStart(localTargRange.startContainer, localTargRange.startOffset);
          targRange.setEnd(localTargRange.endContainer, localTargRange.endOffset);
        }
      }
//    }
    return retVal;
  },

  match : function(targNode, targRange)
  {
    var matched = msiSearchUtils.cannotMatch;
    var ourRange = null;
    var ourTargRange = this.mEditor.document.createRange();
    ourTargRange.setStart(targNode, 0);
    ourTargRange.setEnd(targNode, 0);
//log    msiKludgeLogString( "Entering match for node [" + this.describe() + "]; targRange is [" + this.describeMsiRange(targRange) + "];\n  ourTargRange is [" + this.describeMsiRange(ourTargRange) + "].\n", ["search"] );
    var localTargNode = null;
    var bCanMatch = this.doStructuralNodeMatch(targNode);
    if (bCanMatch)
      matched = msiSearchUtils.completedMatch;
    else if (!this.mustMatchNode())
      bCanMatch = this.canExtendInside(targNode);

    //  The failure of doStructuralNodeMatch() may mean that, regardless of finding a specific node to match, we failed to
    //  find required attributes in the target.
    if (bCanMatch)
    {
      if (!msiSearchUtils.isEmptyElement(this.mNode))
      {
        ourRange = createMsiMatchingRange();
        ourRange.setStart(this.mNode, 0);
        ourRange.setEnd(this.mNode, 0);
        matched = this.extendMatchToRight(ourTargRange, ourRange, this.mEditor);
        if (this.haveContentAfterRangeEnd(ourRange))
          matched = msiSearchUtils.cannotMatch;
      }
    }
    else if (this.canExtendInside(targNode))
    {
      localTargNode = msiSearchUtils.getSingleChild(targNode);
      if (localTargNode)
        matched = this.match(localTargNode, ourTargRange);
    }
    if (msiSearchUtils.isMatching(matched))
    {
      targRange.setStart( ourTargRange.startContainer, ourTargRange.startOffset );
      targRange.setEnd( ourTargRange.endContainer, ourTargRange.endOffset );
      //experimental
      if (ourRange && matched == msiSearchUtils.completedMatch)  // BBM: ?? OurRange is local; it will get lost!
      {
        ourRange.setStartBefore(this.mNode);
        ourRange.setEndAfter(this.mNode);
      }  
    }

//log    if (!bCanMatch)
//log      msiKludgeLogString("doStructuralNodeMatch failed in msiMatchNode.match(), with target [" + targNode.nodeName + "] and targRange [" + targRange.toString() + "], for matching node [" + this.describe() + "].\n", ["search"] );
//log    else
//log      msiKludgeLogString( "Returning [" + this.matchReturnString(matched) + "] from msiMatchNode.match(), with target [" + targNode.nodeName + "] and targRange [" + targRange.toString() + "], for matching node [" + this.describe() + "].\n", ["search"] );

    return matched;
  },

//Note that one essential problem is ensuring that "extendMatchLeft/Right" is called in a reasonable way - the situation could arise
//  where a parent calls a child to extend and the child passes the match back to the parent (when it encounters the left edge of the
//  target node, for instance - I believe this should only be handled by the parent?). We'll avoid this by disallowing passing the
//  match off to one's parent - instead, we'll add a return value of "reachedEndOfTarget"?
//(The rationale behind wanting the parent node (or the function caller) to deal with moving left in the target? 
// I think the real plan should be: [still not right]
//   (i) check whether we can extend against this target.
//  (ii) if we get a return of "pieceByPiece" or "incremental" or whatever we want to use, we go to our current offset and ask that
//       object to extend against the same target.
// (iii) 

  offsetIsAtEnd : function(aNode, anOffset)
  {
    if (anOffset < 0)
      return true;
    return msiNavigationUtils.positionIsAtEnd(aNode, anOffset);
  },

  offsetIsAtStart : function(aNode, anOffset)
  {
    if (anOffset == 0)
      return true;
    return msiNavigationUtils.positionIsAtStart(aNode, anOffset);
  },

  //The mustMatchNode() property means that this node must match a specific and unique node in the target. Nodes which are instead
  //  defined by their contents may still make requirements on the target, but in particular may match across several target nodes,
  //  and in that case may need to check each target node individually. That will be reflected by the targNodeIsCompatible() method.
  mustMatchNode : function()
  {
    switch(this.mSearchType)
    {
      case "mathTemplate":
        return true;
      break;
      case "mathContainer":
      case "anonContainer":
      case "textContainer":
      case "text":
        return false;
      break;
      case "blockContainer":
        return false;         //may want to handle this one differently
      break;
      default:
      break;
    }
    return true;
  },

  //in the default implementation, this simply checks for a nodeName match
  doStructuralNodeMatch : function(aTarget)
  {
    if (this.mNode.nodeName != aTarget.nodeName)
      return false;
    //Should we include some sort of namespace check here? Probably...
    return this.targNodeIsCompatible(aTarget);
  },

  targNodeIsCompatible : function(targNode)
  {
    for (var anAttr in this.mInheritedAttrs)
    {
      if ( !this.checkInheritedAttribute(targNode, anAttr, this.mInheritedAttrs[anAttr]) )
        return false;
    }
    for (var ix = 0; ix < this.mNeededAncestors.length; ++ix)
    {
      if ( !this.checkNeededAncestor(targNode, this.mNeededAncestors[ix]) )
        return false;
    }
    return true;
  },


//The functions extendMatchToLeft and extendMatchToRight are the control loops for progressive matching. The responsibility of 
//  these functions is limited to:
//    (i) Determining when we're done - e.g., we've succeeded or failed in matching this.mNode;
//   (ii) Setting up other conditions of the match, such as whether we must find a node corresponding to this one and whether
//          that node must contain our current search point (this is the case if we're extending a match out from the numerator of
//          a fraction, for instance - we have to find an <mfrac> containing the already matched stuff);
//  (iii) Controlling the iteration through child objects, including controlling the extension of the matching range in the target.
//          This involves also determining when the current target position should be moved into a child or out into a parent node.
//  The design intends for this to be a one-shot deal - we should match as much of this.mNode as is possible, and if we can't get
//    some nontrivial piece matched, we have to report a fail.
//The actual nitty-gritty work of matching this node is done in the doLeftMatchCheck and doRightMatchCheck functions.

//It's necessary to get good control of both "ourRange" and "matchRange". Matching fails (or wrongly may succeed?) if either is
//  changed incorrectly.

//Remaining questions:
//  (i) Do we want to allow matchRange to be modified as we go, or should we be using a local copy? Now using local copy.
// (ii) Should we be doing anything different when we go through the loop with bNewTargNode rather than just bTargOffsetChanged?
//      Isn't there a compatibility check we have to do with the next targNode before charging into it, or can we leave that for our
//      children which may be trying to match against it? Assume we can - the exception being text nodes. How to catch them in the
//      outer loop (other than by completely overriding the extendMatchToLeft() function, which is of course a possibility)?
//(iii) Is there some reason why doMatchCheck() should sometimes do less than a full doStructuralNodeMatch()? Assume no.
  extendMatchToLeft : function(matchRange, ourRange)
  {
//log    msiKludgeLogString( "Entering extendMatchToLeft for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(matchRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"]);
    var localTargRange = matchRange.cloneRange();

    var retVal = msiSearchUtils.partialMatch;
//    var bMustMatchContainingNode = ( this.mustMatchNode() && !this.offsetIsAtEnd(this.mNode, ourOffset) );
//    var bMustMatchContainingNode = ( this.mustMatchNode() && ourRange.startIsInside(this.mNode) );
    var bStartedInside = ourRange.endIsInside(this.mNode);
    var bMustMatchContainingNode = ( this.mustMatchNode() && bStartedInside );
//RWA 11-22-08 trial, amended 12-5-08:
    if (!bStartedInside)  //point of this is that if we started inside our node, whatever has already been matched has already been matched from within this node.
      localTargRange.collapse(true);  //(The point of keeping "localTargRange" around at all is to distinguish what was matched from our siblings or ancestors from what was matched by us.)
    var lastGoodLocalRange = localTargRange.cloneRange();

    var theTarget = matchRange.startContainer;
    var origTarget = theTarget;
    var currTarget = theTarget;
    var theOffset = matchRange.startOffset;
    var currOffset = theOffset;
    var bNewTargNode = true;
    var bTargOffsetChanged = true;
    var localMatch = msiSearchUtils.partialMatch;
    var localMatchNode = null;
    var bMoreToGo = this.haveContentBeforeRangeStart(ourRange);
    if (!bMoreToGo)
    {
//log      msiKludgeLogString( "Initial false from this.haveContentBeforeRangeStart in extendMatchToLeft for node [" + this.describe() + "]!\n", ["search"] );
      msiNavigationUtils.comparePositions(this.mNode, 0, ourRange.startContainer, ourRange.startOffset, bLoggingSearchStuff);
      bMoreToGo = true;  //experimental
    }
    while (bTargOffsetChanged && bMoreToGo)
//    while (bTargOffsetChanged)
    {
      currTarget = theTarget;
      currOffset = theOffset;
      if (bMustMatchContainingNode)
      {
        retVal = this.doLeftMatchCheck(localTargRange, ourRange, origTarget, bNewTargNode);
        if (!origTarget)
          bMustMatchContainingNode = false;  //this return means we've been matched
      }
      else
        retVal = this.doLeftMatchCheck(localTargRange, ourRange, null, bNewTargNode);
      theTarget = localTargRange.startContainer;
      theOffset = localTargRange.startOffset;
      bNewTargNode = (theTarget != currTarget);
      bTargOffsetChanged = (bNewTargNode || (currOffset != theOffset));
      if (!bTargOffsetChanged)
      {
        bTargOffsetChanged = this.adjustLeftStartingTargPosition(localTargRange, ourRange);
//      if (this.adjustLeftStartingTargPosition(localTargRange, ourRange, refEditor))
//      {
//log        if (bTargOffsetChanged)
//log//        bTargOffsetChanged = true;
//log          msiKludgeLogString( "adjustLeftStartingTargPosition() call changed offset in extendMatchToLeft for node [" + this.describe() + "]; localTargRange is [" + this.describeMsiRange(localTargRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
      }
      if (this.targNodeIsCompatible(localTargRange.startContainer))
        lastGoodLocalRange.setStart(localTargRange.startContainer, localTargRange.startOffset);
      bMoreToGo = this.haveContentBeforeRangeStart(ourRange);
    }
    if (bMoreToGo)
      retVal = msiSearchUtils.cannotMatch;
    localTargRange.setStart(lastGoodLocalRange.startContainer, lastGoodLocalRange.startOffset);
    if (msiSearchUtils.isMatching(retVal)) 
    {
      if (!this.mustMatchNode())  //in this case we can reach here successfully without having checked compatibility match:
      {
        if (!this.targNodeIsCompatible(localTargRange.startContainer))
        {
//log          msiKludgeLogString( "In extendMatchToLeft, targNodeIsCompatible failed for node [" + this.describe() + "]; localTargRange is [" + this.describeMsiRange(localTargRange) + "]; ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
          retVal = msiSearchUtils.cannotMatch;
        }
      }
    }
    if (msiSearchUtils.isMatching(retVal))
    {
      ourRange.setStartBefore(this.mNode);
      this.adjoinRange(matchRange, localTargRange);
      retVal = msiSearchUtils.completedMatch;
//      matchRange.setStart( theTarget, theOffset );
    }  
//log    msiKludgeLogString( "Returning [" + this.matchReturnString(retVal) + "] from extendMatchToLeft for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(matchRange) + "].\n", ["search"] );
    return retVal;
  },


  extendMatchToRight : function(matchRange, ourRange)
  {
//log    msiKludgeLogString( "Entering extendMatchToRight for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(matchRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
    var localTargRange = matchRange.cloneRange();
    var retVal = msiSearchUtils.partialMatch;
//    var bMustMatchContainingNode = ( this.mustMatchNode() && !this.offsetIsAtStart(this.mNode, ourOffset) );
//    var bMustMatchContainingNode = ( this.mustMatchNode() && ourRange.endIsInside(this.mNode) );
    var bStartedInside = ourRange.startIsInside(this.mNode);
    var bMustMatchContainingNode = ( this.mustMatchNode() && bStartedInside );
//RWA 11-22-08 trial, amended 12-5-08:
    if (!bStartedInside)  //point of this is that if we started inside our node, whatever has already been matched has already been matched from within this node.
      localTargRange.collapse(false);  //(The point of keeping "localTargRange" around at all is to distinguish what was matched from our siblings or ancestors from what was matched by us.)
    var lastGoodLocalRange = localTargRange.cloneRange();

    var theTarget = matchRange.endContainer;
    var origTarget = theTarget;
    var currTarget = theTarget;
    var theOffset = matchRange.endOffset;
    var currOffset = theOffset;
    var localMatchNode = null;
    var bNewTargNode = true;
    var bTargOffsetChanged = true;
    var localMatch = msiSearchUtils.partialMatch;
    var localMatchNode = null;
    var bMoreToGo = this.haveContentAfterRangeEnd(ourRange);
    if (!bMoreToGo)
    {
//log      msiKludgeLogString( "Initial false from haveContentAfterRangeEnd in extendMatchToRight for node [" + this.describe() + "]!\n", ["search"] );
      msiNavigationUtils.comparePositions(this.mNode, msiNavigationUtils.lastOffset(this.mNode), ourRange.endContainer, ourRange.endOffset, bLoggingSearchStuff);
      bMoreToGo = true;  //experimental
    }
    while (bTargOffsetChanged && bMoreToGo)
//    while (bTargOffsetChanged)
    {
      currTarget = localTargRange.endContainer;
      currOffset = localTargRange.endOffset;
      if (bMustMatchContainingNode)
      {
        retVal = this.doRightMatchCheck(localTargRange, ourRange, origTarget, bNewTargNode);
        if (!origTarget)
          bMustMatchContainingNode = false;  //this return means we've been matched
      }
      else
        retVal = this.doRightMatchCheck(localTargRange, ourRange, null, bNewTargNode);
//log      msiKludgeLogString( "Got [" + this.matchReturnString(retVal) + "] back from doRightMatchCheck call in extendMatchToRight for node [" + this.describe() + "]; localTargRange is [" + this.describeMsiRange(localTargRange) + "]; ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
//      theTarget = localTargRange.endContainer;
//      theOffset = localTargRange.endOffset;
      bNewTargNode = (localTargRange.endContainer != currTarget);
      bTargOffsetChanged = (bNewTargNode || (currOffset != localTargRange.endOffset));
      if (!bTargOffsetChanged)
      {
        bTargOffsetChanged = this.adjustRightStartingTargPosition(localTargRange, ourRange);
        bNewTargNode = (localTargRange.endContainer != currTarget);
//      if (this.adjustRightStartingTargPosition(localTargRange, ourRange, refEditor))
//      {
//        bTargOffsetChanged = true;
//log        if (bTargOffsetChanged)
//log          msiKludgeLogString( "adjustRightStartingTargPosition() call changed offset in extendMatchToRight for node [" + this.describe() + "]; localTargRange is [" + this.describeMsiRange(localTargRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
//        else
//          msiKludgeLogString( "adjustRightStartingTargPosition() call did not change offset in extendMatchToRight for node [" + this.describe() + "]; localTargRange is [" + this.describeMsiRange(localTargRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
      }
      if (this.targNodeIsCompatible(localTargRange.endContainer))
        lastGoodLocalRange.setEnd(localTargRange.endContainer, localTargRange.endOffset);
      bMoreToGo = this.haveContentAfterRangeEnd(ourRange);
    }
    if (bMoreToGo)
      retVal = msiSearchUtils.cannotMatch;

    localTargRange.setEnd( lastGoodLocalRange.endContainer, lastGoodLocalRange.endOffset );
    if (msiSearchUtils.isMatching(retVal)) 
    {
      if (!this.mustMatchNode())  //in this case we can reach here successfully without having checked compatibility match:
      {
        if (!this.targNodeIsCompatible(localTargRange.endContainer))
        {
//log          msiKludgeLogString( "In extendMatchToRight, targNodeIsCompatible failed for node [" + this.describe() + "]; localTargRange is [" + this.describeMsiRange(localTargRange) + "]; ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
          retVal = msiSearchUtils.cannotMatch;
        }
      }
    }

    if (msiSearchUtils.isMatching(retVal))
    {
      ourRange.setEndAfter(this.mNode);
      this.adjoinRange(matchRange, localTargRange);
      retVal = msiSearchUtils.completedMatch;
    }  
//log    msiKludgeLogString( "Returning [" + this.matchReturnString(retVal) + "] from extendMatchToRight for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(matchRange) + "]; ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
    return retVal;
  },


//The functions doLeftMatchCheck and doRightMatchCheck are intended to settle the question "Can this.mNode match the next thing at
//  the left (right) of ourRange with the next thing at the left (right) of targRange?" NOTE: If ourRange extends to the left end
//  of our content, then we should do a compatibility check on this node and - if successful - set ourRange.start to the left of us.

//The parameters here:
//  "matchRange" represents the range of target nodes that have been matched
//  "ourRange" represents the range of source nodes that have matched
//  "origTarget" is passed in non-null when we've been recursively called by a node which has to match a specific node and didn't yet;
//    it points to the target node we were looking at when that match failed, and in that case we need to ensure we match a node which
//    is at least that high in the heirarchy. (In other words, it represents a level where we may have found, for instance, an mrow
//    inside an mfrac but we still need to find the containing mfrac. This implementation may still be suspect.)
//  "bNewTargNode" signals that we're encountering a new target node in the outer loop which calls this function. It's used in
//    the "derived class" implementations; should perhaps be used here as well.
  doLeftMatchCheck : function(targRange, ourRange, origTarget, bNewTargNode)
  {
    //The "bNeedNodeMatch" cases other than "bMustMatchContainingNode" simply mean that we're looking through the target's children 
    // trying to match our node rather than its children - thus our offset is at the end of an object which must find a matching node. 
    // Again, we try to match as long as the targOffset can be safely moved left, but in this case we don't try to match our children 
    // here; once we find a matching node, we'll set "bNeedNodeMatch" to false and move theTarget inside it.
//      else if (bNeedNodeMatch)
//log    msiKludgeLogString( "Entering doLeftMatchCheck for node [" + this.describe() + "]\n", ["search"] );
    var retVal = msiSearchUtils.partialMatch;
    var bMustMatchContainingNode = false;
    var localNodeToMatch = null;
    var localMatcher = null;
    var localMatch = msiSearchUtils.partialMatch;
    var targNode = targRange.startContainer;
    var targOffset = targRange.startOffset;
    var ourLocalRange = ourRange.cloneRange();
    if (origTarget != null)
      bMustMatchContainingNode = true;
    var bNeedNodeMatch = this.mustMatchNode();
    if (!bMustMatchContainingNode && bNeedNodeMatch)
    {
//NOTE! The following may in fact be needed, but it seems wrong. Keep an eye out for this case.
//  The question, if we extend to start matching a node (presumably a parent) without being properly inside it (so for instance
//  moving to the next child in a container and asking it to extend the match), can it ever be the case that it's the target position's
//  container node that it should match? This really isn't even yet clear...
//      if (this.offsetIsAtEnd(targNode, targOffset))  //In this case, the entire targNode is a candidate for us to match??
//        bNeedNodeMatch = !this.doStructuralNodeMatch(targNode);
      if ( (targOffset > 0) && targNode.childNodes && (targOffset <= targNode.childNodes.length) )
        bNeedNodeMatch = !this.doStructuralNodeMatch(targNode.childNodes[targOffset-1]);

      //So we now match "targNode"; move on to match our contents if appropriate.
      if (!bNeedNodeMatch && targNode.childNodes)
      {
        if (targOffset > targNode.childNodes.length)
        {
          msiKludgeLogString( "In msiMatchNode.doLeftMatchCheck() for node [" + this.mNode.nodeName + "], problem! targNode is [" + targNode.nodeName + "], and targOffset is [" + targOffset + "] while length of target child nodes is [" + targNode.childNodes.length + "].\n", ["search"] );
          targOffset = targNode.childNodes.length;
        }
        //If targOffset is 0, we'll go through and drop out of the calling loop (since targOffset and targNode will be unchanged),
        //  and this is appropriate; if ourOffset is 0 we'll report matched, otherwise we've got content and the target doesn't, so
        //  we should report unmatched.
        if (targOffset > 0)  
        {
          targNode = targNode.childNodes[targOffset - 1];
          targOffset = msiNavigationUtils.lastOffset(targNode);
//log          msiKludgeLogString( "In doLeftMatchCheck for node [" + this.describe() + "], moving end of target to [" + this.describeNode(targNode) + ", offset " + targOffset + "].\n", ["search"] );
          targRange.setStart(targNode, targOffset);
        }
        ourRange.setStart(this.mNode, msiNavigationUtils.lastOffset(this.mNode));
      }
    }
//rwa 11-25-08 experiment!    else
    if (!bNeedNodeMatch || bMustMatchContainingNode)
    {
      //The "bMustMatchContainingNode" case is handled by continuing to move to the left in the target until either we can't
      //  legally move left anymore (in which case we've failed to match our required node and reached something nontrivial to the
      //  left in the target, which should signal a general failure), or we do manage to find a match. In this case we can continue
      //  to check our child nodes against the target's children and extend the match left as we go. NOTE: The case of a template,
      //  where the positional matching of children is strict and dependant on the nodeName of both the matching node and the target,
      //  has to be handled separately. (However, it should perhaps be handled in this function anyway, as it needs to use the
      //  targOffset and targNode moving as long as it can? So how should that be written in?)
      if (bMustMatchContainingNode)
      {
        if (msiNavigationUtils.isAncestor(theTarget, origTarget))
          bMustMatchContainingNode = !this.doStructuralNodeMatch(theTarget);
        if (!bMustMatchContainingNode)
          origTarget = null;
      }
      //Now, if appropriate, look at the next child node.
      if (this.haveContentBeforeRangeStart(ourRange) && (!bMustMatchContainingNode || !msiSearchUtils.isTemplate(this.mNode)) )
      {
        localNodeToMatch = this.nextChildToLeft(ourRange);
//        if (ourOffset > this.mNode.childNodes.length)
//        {
//          dump("In msiMatchNode.doLeftMatchCheck() for node [" + this.mNode.nodeName + "], problem! ourOffset is [" + ourOffset + "] while length of child nodes is [" + this.mNode.childNodes.length + "].\n");
//          ourOffset = this.mNode.childNodes.length;
//        }
        localMatch = msiSearchUtils.cannotMatch;
        if (localNodeToMatch != null)
        {
          ourLocalRange.setEnd(ourRange.endContainer, ourRange.endOffset);
          ourLocalRange.setStartAfter(localNodeToMatch);
          localMatcher = createMatchNode(localNodeToMatch, this.mFlags, this.mEditor);
//          localMatch = localMatcher.extendMatchToLeft(targRange, ourRange);
          localMatch = localMatcher.extendMatchToLeft(targRange, ourLocalRange);
          if (msiSearchUtils.isMatching(localMatch))
            ourRange.setStart(ourLocalRange.startContainer, ourLocalRange.startOffset);
        }
        else
        {
//log          msiKludgeLogString( "localNodeToMatch is null in doLeftMatchCheck for node [" + this.describe() + "]!\n", ["search"] );
          msiNavigationUtils.comparePositions(this.mNode, 0, ourRange.startContainer, ourRange.startOffset, bLoggingSearchStuff);
        }

        if (!msiSearchUtils.isMatching(localMatch))
        {
//log          var localNodeToMatchStr = "null";
//log          if (localMatcher)
//log            localNodeToMatchStr = localMatcher.describe();
//log          msiKludgeLogString( "Returning cannotMatch from doLeftMatchCheck for node [" + this.describe() + "]; localMatcher was [" + localNodeToMatchStr + "]\n", ["search"] );
          return msiSearchUtils.cannotMatch;
        }
//        if (retVal = msiSearchUtils.completedMatch && localMatch != null)
//          ourRange.setStartBefore(localNodeToMatch);

//        --ourOffset;
//        targOffset = matchRange.startOffset;
//        targNode = matchRange.startContainer;
      }
    }
//log    msiKludgeLogString( "Returning [" + this.matchReturnString(retVal) + "] from doLeftMatchCheck for node [" + this.describe() + "]\n", ["search"] );
    return retVal;
  },

//See "doLeftMatchCheck" above for a description of parameters.
  doRightMatchCheck : function(matchRange, ourRange, origTarget, bNewTargNode)
  {
    //The "bNeedNodeMatch" cases other than "bMustMatchContainingNode" simply mean that we're looking through the target's children 
    // trying to match our node rather than its children - thus our offset is at the end of an object which must find a matching node. 
    // Again, we try to match as long as the targOffset can be safely moved right, but in this case we don't try to match our children 
    // here; once we find a matching node, we'll set "bNeedNodeMatch" to false and move theTarget inside it.
//      else if (bNeedNodeMatch)
//log    msiKludgeLogString( "Entering doRightMatchCheck for node [" + this.describe() + "]\n", ["search"] );
    var bMustMatchContainingNode = false;
    var targNode = matchRange.endContainer;
    var targOffset = matchRange.endOffset;
    if (origTarget != null)
      bMustMatchContainingNode = true;
    var retVal = msiSearchUtils.partialMatch;
    var localNodeToMatch = null;
    var localMatcher = null;
    var localMatch = msiSearchUtils.partialMatch;
    var ourLocalRange = ourRange.cloneRange();
    var bNeedNodeMatch = this.mustMatchNode();
    if (!bMustMatchContainingNode && bNeedNodeMatch)
    {
//NOTE! The following may in fact be needed, but it seems wrong. Keep an eye out for this case.
//      if (this.offsetIsAtStart(targNode, targOffset))  //In this case, the entire targNode is a candidate for us to match
//        bNeedNodeMatch = !this.doStructuralNodeMatch(targNode);

      if ( (targOffset >= 0) && targNode.childNodes && (targOffset < targNode.childNodes.length) )
        bNeedNodeMatch = !this.doStructuralNodeMatch(targNode.childNodes[targOffset]);
      //So we now match "targNode"; move on to match our contents if appropriate.

      if (!bNeedNodeMatch && targNode.childNodes)
      {
        if (targOffset > targNode.childNodes.length)
        {
          msiKludgeLogString( "In msiMatchNode.doRightMatchCheck() for node [" + this.mNode.nodeName + "], problem! targNode is [" + targNode.nodeName + "], and targOffset is [" + targOffset + "] while length of target child nodes is [" + targNode.childNodes.length + "].\n", ["search"] );
          targOffset = targNode.childNodes.length;
        }
        //If targOffset is at end, we'll go through and drop out of the calling loop (since targOffset and targNode will be unchanged),
        //  and this is appropriate; if we're at end in our pattern we'll report matched, otherwise we've got content and the target doesn't, so
        //  we should report unmatched.
        if (!this.offsetIsAtEnd(targNode, targOffset))
        {
          targNode = targNode.childNodes[targOffset];
          targOffset = 0;
//log          msiKludgeLogString( "In doRightMatchCheck for node [" + this.describe() + "], moving end of target to [" + this.describeNode(targNode) + ", offset " + targOffset + "].\n", ["search"] );
          matchRange.setEnd(targNode, targOffset);
        }
      }
    }
//rwa 11-25-08 experiment!    else
    if (!bNeedNodeMatch || bMustMatchContainingNode)
    {
      //The "bMustMatchContainingNode" case is handled by continuing to move to the left in the target until either we can't
      //  legally move left anymore (in which case we've failed to match our required node and reached something nontrivial to the
      //  left in the target, which should signal a general failure), or we do manage to find a match. In this case we can continue
      //  to check our child nodes against the target's children and extend the match left as we go. NOTE: The case of a template,
      //  where the positional matching of children is strict and dependant on the nodeName of both the matching node and the target,
      //  has to be handled separately. (However, it should perhaps be handled in this function anyway, as it needs to use the
      //  targOffset and targNode moving as long as it can? So how should that be written in?)
      if (bMustMatchContainingNode)
      {
        if (msiNavigationUtils.isAncestor(targNode, origTarget))
          bMustMatchContainingNode = !this.doStructuralNodeMatch(targNode);
        if (!bMustMatchContainingNode)
          origTarget = null;
      }
      //Now, if appropriate, look at the next child node.
//      if ( (ourOffset > 0) && (!bMustMatchContainingNode || !msiSearchUtils.isTemplate(this.mNode)) )
      if ( this.haveContentAfterRangeEnd(ourRange) && (!bMustMatchContainingNode || !msiSearchUtils.isTemplate(this.mNode)) )
      {
//        if (ourOffset > this.mNode.childNodes.length)
//        {
//          dump("In msiMatchNode.doLeftMatchCheck() for node [" + this.mNode.nodeName + "], problem! ourOffset is [" + ourOffset + "] while length of child nodes is [" + this.mNode.childNodes.length + "].\n");
//          ourOffset = this.mNode.childNodes.length;
//        }
        localNodeToMatch = this.nextChildToRight(ourRange);
        localMatch = msiSearchUtils.cannotMatch;
        if (localNodeToMatch != null)
        {
          ourLocalRange.setStart(ourRange.startContainer, ourRange.startOffset);
          ourLocalRange.setEndBefore(localNodeToMatch);
          localMatcher = createMatchNode(localNodeToMatch, this.mFlags, this.mEditor);
//          localMatch = localMatcher.extendMatchToRight(matchRange, ourRange, this.mEditor);
          localMatch = localMatcher.extendMatchToRight(matchRange, ourLocalRange);
          if (msiSearchUtils.isMatching(localMatch))
            ourRange.setEnd(ourLocalRange.endContainer, ourLocalRange.endOffset);
        }
        else
        {
//log          msiKludgeLogString( "localNodeToMatch is null in doRightMatchCheck for node [" + this.describe() + "]!\n", ["search"] );
          msiNavigationUtils.comparePositions(this.mNode, msiNavigationUtils.lastOffset(this.mNode), ourRange.endContainer, ourRange.endOffset, bLoggingSearchStuff);
        }
        if (!msiSearchUtils.isMatching(localMatch))
        {
//log          msiKludgeLogString( "Returning cannotMatch from doRightMatchCheck for node [" + this.describe() + "]\n", ["search"] );
          return msiSearchUtils.cannotMatch;
        }
//        retVal = localMatch;
//        if (retVal = msiSearchUtils.completedMatch)
//          ourRange.setEndAfter(localNodeToMatch);
//        --ourOffset;
//        targOffset = matchRange.startOffset;
//        targNode = matchRange.startContainer;
      }
    }
//log    msiKludgeLogString( "Returning [" + this.matchReturnString(retVal) + "] from doRightMatchCheck for node [" + this.describe() + "]\n", ["search"] );
    return retVal;
  },

////  this.extendMatchToLeft = function(matchRange, ourOffset, refEditor)
////  {
////    var retVal = msiSearchUtils.partialMatch;
////    if (ourOffset < 0)
////      ourOffset = msiNavigationUtils.lastOffset()
////    var theTarget = matchRange.startContainer;
////    var theOffset = matchRange.startOffset;
//////    this.adjustLeftStartingTargPosition(theTarget, theOffset, ourOffset);
////
////    var bNewTargNode = true;
////    var bTargOffsetChanged = true;
////    while ( (retVal == msiSearchUtils.partialMatch) && (ourOffset > 0) )
////    {
////      if (bNewTargNode)
////      {
////        if (!this.checkNodeStructuralMatch(theTarget, theOffset))
////          retVal = msiSearchUtils.cannotMatch;
////        bNewTargNode = false;
////      }
////      if (!bTargOffsetChanged && (ourOffset == 0) )
////        return msiSearchUtils.completedMatch;
////
////      
////      bTargOffsetChanged = this.adjustLeftStartingTargPosition(theTarget, theOffset, ourOffset, refEditor);
////
////      var theTarget = this.getNextTargetToLeft(theTarget, theOffset);
////      if (theTarget == null)
////        return msiSearchUtils.cannotMatch;
////      var nodeType = msiSearchUtils.getTypeOfNodeSearch(theTarget);
//////      if (nodeType == "blockContainer")
////      if (this.shouldSkipOverWhiteSpaceWhenMatchingBoundary(nodeType))
////        ourOffset = moveLeftOfTrailingWhiteSpace(ourOffset);
////      switch(this.canMatch(nodeType))
////      {
////        START HERE! This isnt right! What do we do to indicate that we just want to move the left position outside an object and try again?
////        case msiSearchUtils.matchPieceByPiece:
////        case msiSearchUtils.matchOne:
////        //Now try to match our right-hand end against targText's, where right-hand end means, of course, taking offsets into account.
////        {
////          var targText = theTarget.textContent;
////          if ( (theOffset >= 0) && (theOffset < targText.length) )
////            targText = targText.substr(0, theOffset);
////          var matchLen = ourOffset;
////          if (ourOffset < 0)
////            matchLen = this.mText.length; ?????
////          if (targText.length < this.mText
////        }
////        break;
////        default:
////          retVal = msiSearchUtils.cannotMatch;
////        break;
////      }
////    }
////  };

  //We've been passed a position inside theTarget. Can we move any further to the left to try to match?
  //Answer is yes if:
  //   (i) We're at the beginning of theTarget and can move outside it;
  //  (ii) We can move inside the object just to our left;
  // (iii) What's to our left is ignorable whitespace.
//  adjustLeftStartingTargPosition : function(theTarget, theOffset, ourRange, refEditor)
//  {
//    var retVal = false;
//    var bGoInside = false;
//    var bGoOutside = false;
//    var newNode = null;
//    if (theOffset < 0)
//      theOffset = msiNavigationUtils.lastOffset(theTarget);
//    if (this.offsetIsAtStart(theTarget, theOffset))
//    {
//      bGoOutside = this.canExtendOutside(theTarget);
//    }
//    else if (theTarget.nodeType == nsIDOMNode.TEXT_NODE)
//    //In this case we can't do anything unless theTarget is ignorable whitespace, in which case we'll step outside:
//    {
//      bGoOutside = msiNavigationUtils.isIgnorableWhitespace(theTarget);
//    }
//    else //Look at child nodes:
//    {
//      if (theOffset > theTarget.childNodes.length)
//        theOffset = theTarget.childNodes.length;
//      for ( ; !bGoInside && (theOffset > 0); --theOffset)
//      {
//        newNode = theTarget.childNodes[theOffset - 1];  //Note that theOffset isn't 0, since we didn't fall into the "offsetIsAtStart" case.
//        if (this.canExtendInside(newNode))
//          bGoInside = true;
//        else if (msiNavigationUtils.isIgnorableWhitespace(newNode))
//          retVal = true;
//        else
//          break;
//      }
//    }
//
//    if (bGoOutside)
//    {
//      theOffset = msiNavigationUtils.offsetInParent(theTarget);
//      theTarget = theTarget.parentNode;
//      retVal = true;
//    }
//    else if (bGoInside && (newNode != null))
//    {
//      theTarget = newNode;
//      theOffset = -1;
//      retVal = true;
//    }
//    return retVal;
//  },

  adjustLeftStartingTargPosition : function(targRange, ourRange)
  {
    var retVal = false;
    var bGoInside = false;
    var bGoOutside = false;
    var newNode = null;
    var theTarget = targRange.startContainer;
    var theOffset = targRange.startOffset;

//    if (this.offsetIsAtStart(theTarget, theOffset))
    if (!msiNavigationUtils.nodeHasContentBeforeRangeStart(targRange, theTarget))
    {
      if (!msiNavigationUtils.nodeHasContentAfterRangeEnd(targRange, theTarget))  //our match extends to right outside theTarget, so we can extend to left?
        bGoOutside = true;
      else
        bGoOutside = this.canExtendOutside(theTarget);
    }
    else if (theTarget.nodeType == nsIDOMNode.TEXT_NODE)
    //In this case we can't do anything unless theTarget is ignorable whitespace, in which case we'll step outside:
    {
      bGoOutside = msiNavigationUtils.isIgnorableWhitespace(theTarget);
    }
    else //Look at child nodes:
    {
      if (theOffset > theTarget.childNodes.length)
        theOffset = theTarget.childNodes.length;
      for ( ; !bGoInside && (theOffset > 0); --theOffset)
      {
        newNode = theTarget.childNodes[theOffset - 1];  //Note that theOffset isn't 0, since we didn't fall into the "offsetIsAtStart" case.
        if (this.canExtendInside(newNode))
          bGoInside = true;
        else if (msiNavigationUtils.isIgnorableWhitespace(newNode))
          retVal = true;
        else
          break;
      }
    }

    if (bGoOutside)
    {
      targRange.setStartBefore(theTarget);
//      theOffset = msiNavigationUtils.offsetInParent(theTarget);
//      theTarget = theTarget.parentNode;
      retVal = true;
    }
    else if (bGoInside && (newNode != null))
    {
      theTarget = newNode;
      theOffset = msiNavigationUtils.lastOffset(newNode);
      targRange.setStart(theTarget, theOffset);
      retVal = true;
    }
    return retVal;
  },

  //We've been passed a position inside theTarget. Can we move any further to the right to try to match?
  //Answer is yes if:
  //   (i) We're at the end of theTarget and can move outside it;
  //  (ii) We can move inside the object just to our right;
  // (iii) What's to our right is ignorable whitespace.
//  adjustRightStartingTargPosition : function(theTarget, theOffset, ourRange, refEditor)
//  {
//    var retVal = false;
//    var bGoInside = false;
//    var bGoOutside = false;
//    var newNode = null;
//    if ( (theOffset > theTarget.childNodes.length) || (theOffset < 0) )
//      theOffset = msiNavigationUtils.lastOffset(theTarget);  //is this the right thing to do?? Do we want -1 as an offset to mean "coming in" or "coming in from the right"?
//
//    if (this.offsetIsAtEnd(theTarget, theOffset))
//    {
//      bGoOutside = this.canExtendOutside(theTarget);
//    }
//    else if (theTarget.nodeType == nsIDOMNode.TEXT_NODE)
//    //In this case we can't do anything unless theTarget is ignorable whitespace, in which case we'll step outside:
//    {
//      bGoOutside = msiNavigationUtils.isIgnorableWhitespace(theTarget);
//    }
//    else //Look at child nodes:
//    {
//      for ( ; !bGoInside && (theOffset < theTarget.childNodes.length); ++theOffset)
//      {
//        newNode = theTarget.childNodes[theOffset];  //Note that theOffset isn't at the end, since we didn't fall into the "offsetIsAtStart" case.
//        if (this.canExtendInside(newNode))
//          bGoInside = true;
//        else if (msiNavigationUtils.isIgnorableWhitespace(newNode))
//          retVal = true;
//        else
//          break;
//      }
//    }
//
//    if (bGoOutside)
//    {
//      theOffset = msiNavigationUtils.offsetInParent(theTarget) + 1;
//      theTarget = theTarget.parentNode;
//      retVal = true;
//    }
//    else if (bGoInside && (newNode != null))
//    {
//      theTarget = newNode;
//      theOffset = 0;
//      retVal = true;
//    }
//    return retVal;
//  },

//NOTE: "targRange" is taken as extending from the start of what this.mNode matched to the end of what we've thus far matched.
  adjustRightStartingTargPosition : function(targRange, ourRange)
  {
//log    var logStr = "In adjustRightStartingTargPosition for node [" + this.describe() + "]; ";
    var theTarget = targRange.endContainer;
    var theOffset = targRange.endOffset;
    var retVal = false;
    var bGoInside = false;
    var bGoOutside = false;
    var newNode = null;
    if ( (theOffset > theTarget.childNodes.length) || (theOffset < 0) )
      theOffset = msiNavigationUtils.lastOffset(theTarget);  //is this the right thing to do?? Do we want -1 as an offset to mean "coming in" or "coming in from the right"?

//    if (this.offsetIsAtEnd(theTarget, theOffset))
    if (!msiNavigationUtils.nodeHasContentAfterRangeEnd(targRange, theTarget))
    {
      if (!msiNavigationUtils.nodeHasContentBeforeRangeStart(targRange, theTarget))  //our match started outside theTarget, so we can extend?
        bGoOutside = true;
      else
        bGoOutside = this.canExtendOutside(theTarget);
    }
    else if (theTarget.nodeType == nsIDOMNode.TEXT_NODE)
    //In this case we can't do anything unless theTarget is ignorable whitespace, in which case we'll step outside:
    {
      bGoOutside = msiNavigationUtils.isIgnorableWhitespace(theTarget);
    }
    else //Look at child nodes:
    {
      for ( ; !bGoInside && (theOffset < theTarget.childNodes.length); ++theOffset)
      {
        newNode = theTarget.childNodes[theOffset];  //Note that theOffset isn't at the end, since we didn't fall into the "offsetIsAtStart" case.
        if (this.canExtendInside(newNode))
          bGoInside = true;
        else
        {
//log          msiKludgeLogString( "In adjustRightStartingPosition for node [" + this.describe() + "], can't go inside [" + this.describeNode(newNode) + "].\n", ["search"] );
          if (msiNavigationUtils.isIgnorableWhitespace(newNode))
            retVal = true;
          else
            break;
        }
      }
    }

    if (bGoOutside)
    {
      targRange.setEndAfter(theTarget);
//      theOffset = msiNavigationUtils.offsetInParent(theTarget) + 1;
//      theTarget = theTarget.parentNode;
      retVal = true;
    }
    else if (bGoInside && (newNode != null))
    {
//      theTarget = newNode;
//      theOffset = 0;
      targRange.setEnd(newNode, 0);
      retVal = true;
    }
    return retVal;
  },

//  getNextTargetToLeft : function(aNode, anOffset)
//  {
//    var targNode = aNode;
//    while (anOffset == 0)
//    {
//      if (!this.canExtendOutside(aNode) || (!aNode.parentNode))
//        return null;
//      anOffset = msiNavigationUtils.offsetInParent(aNode);
//      aNode = aNode.parentNode;
//    }
////    if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
//    if ( (aNode.childNodes) && (anOffset > 0) && (anOffset <= aNode.childNodes.length) )
//    {
//      aNode = aNode.childNodes[anOffset - 1];
//      anOffset = -1;
//    }
//  },
//
//  getNextTargetToRight : function(aNode, anOffset)
//  {
//    var targNode = aNode;
//    while ( (anOffset < 0) || (anOffset >= aNode.childNodes.length) )
//    {
//      if (!this.canExtendOutside(aNode) || (!aNode.parentNode))
//        return null;
//      anOffset = msiNavigationUtils.offsetInParent(aNode);
//      aNode = aNode.parentNode;
//    }
////    if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
//    if ( (aNode.childNodes) && (anOffset >= 0) && (anOffset < aNode.childNodes.length) )
//    {
//      aNode = aNode.childNodes[anOffset];
//      anOffset = 0;
//    }
//  },

  canExtendOutside : function(aNode)
  {
    return this.nodeCanExtendOutside(this.mNode, aNode, this.mFlags, this.mEditor);
  },

  nodeCanExtendOutside : function(ourNode, targNode, matchFlags, refEditor)
  {
    var matchInfo = null;
    if (msiSearchUtils.isTemplate(targNode))
      return false;
    if (targNode.parentNode && msiSearchUtils.isTemplate(targNode.parentNode))
    {
      if (msiSearchUtils.isTemplate(ourNode))
      {
        var targetName = msiGetBaseNodeName(targNode);
        matchInfo = XPathStringFormatterBase.getMatchingNodeInfo( msiGetBaseNodeName(ourNode), matchFlags);
        for (var ix = 0; ix < matchInfo.length; ++ix)
        {
          if (targetName == matchInfo[ix].theName)
            return true;
        }
      }
      return false;
    }

    var nodeType = msiSearchUtils.getTypeOfNodeSearch(ourNode, refEditor);
    var targType = msiSearchUtils.getTypeOfNodeSearch(targNode, refEditor);
    switch(nodeType)
    {
      case "text":
        return this.nodeCanExtendOutside(ourNode.parentNode, targNode, matchFlags, refEditor);
      break;

      case "textContainer":
      case "blockContainer":
      case "anonContainer":
        return true;
      break;
      case "mathTemplate":  
      case "mathContainer":
      case "find":
      default:
        return false;
      break;
    }
  },

  canExtendInside : function(aNode)
  {
    return this.nodeCanExtendInside(this.mNode, aNode, this.mFlags, this.mEditor);
  },

  nodeCanExtendInside : function(ourNode, targNode, matchFlags, refEditor)
  {
    if (msiSearchUtils.isTemplate(targNode))
      return false;

    var nodeType = msiSearchUtils.getTypeOfNodeSearch(ourNode, refEditor);
    if (nodeType == "text")
      return this.nodeCanExtendInside(ourNode.parentNode, targNode);

    var targType = msiSearchUtils.getTypeOfNodeSearch(targNode, refEditor);
    switch(targType)
    {
      case "text":
      case "textContainer":
      case "blockContainer":
      case "anonContainer":
        return true;
      break;
      case "mathContainer":
        switch(nodeType)         //is this really what we want here??
        {
          case "mathContainer":
          case "blockContainer":
          case "anonContainer":
            return true;
          break;
          default:
            return false;
          break;
        }
      break;
      case "mathTemplate":  
      case "find":
      default:
        return false;
      break;
    }
  },

//  lastOffset : function(aNode)
//  {
//    if (aNode.nodeType == nsIDOMNode.TEXT_NODE)
//      return aNode.textContent.length;
//    if (aNode.childNodes)
//      return aNode.childNodes.length;
//    return 0;
//  },

  haveContentBeforeRangeStart : function(aRange)
  {
    if (( aRange.startContainer == this.mNode) && (aRange.startOffset == 0) )
      return false;
    return msiNavigationUtils.nodeHasContentBeforeRangeStart(aRange, this.mNode);
  },

//  nodeHasContentBeforeRangeStart : function(aRange, aNode)
//  {
//    var retVal = false;
//    var compVal = msiNavigationUtils.comparePositions(aNode, 0, aRange.startContainer, aRange.startOffset);
//    if (compVal < 0) //start of aNode is before start position of aRange
//      retVal = true;
////    var compNode = msiNavigationUtils.getNodeBeforePosition(aRange.startContainer, aRange.startOffset);
////    if (compNode && msiNavigationUtils.isAncestor(aNode, compNode))
////      retVal = true;
////    else if (msiNavigationUtils.isAncestor(aRange.startContainer, aNode))
////      retVal = false;  //in this case, compNode should have been aNode or contained it if aNode had content before aRange
////    else  //No content of aNode is immediately to the left of the range start; now we only want to return true if rangeStart is altogether before aNode.
////    {
////      compNode = aRange.startContainer;
////      compVal = aRange.startContainer.compareDocumentPosition(aNode);
////      retVal = ( (compVal & Node.DOCUMENT_POSITION_FOLLOWING) != null);
////    }
//    return retVal;  
//  },
//
  haveContentAfterRangeEnd : function(aRange)
  {
    if (( aRange.startContainer == this.mNode) && (aRange.startOffset == this.lastOffset) )
      return false;
    return msiNavigationUtils.nodeHasContentAfterRangeEnd(aRange, this.mNode);
  },

//  nodeHasContentAfterRangeEnd : function(aRange, aNode)
//  {
//    var retVal = false;
//    var anOffset = msiNavigationUtils.lastOffset(aNode);
//    var compVal = msiNavigationUtils.comparePositions(aNode, anOffset, aRange.endContainer, aRange.endOffset);
//    if (compVal > 0) //last offset in aNode is after end position of aRange
//      retVal = true;
//    
////    var compNode = msiNavigationUtils.getNodeAfterPosition(aRange.endContainer, aRange.endOffset);
////    if (compNode && msiNavigationUtils.isAncestor(aNode, compNode))
////      retVal = true;
////    else if (msiNavigationUtils.isAncestor(aRange.endContainer, aNode))
////      retVal = false;
////    else  //No content of aNode is immediately to the right of the range end; now we only want to return true if rangeEnd is altogether before aNode.
////    {
////      compVal = aRange.endContainer.compareDocumentPosition(aNode);
////      retVal = ( (compVal & Node.DOCUMENT_POSITION_PRECEDING) != null);
////    }
//    return retVal;  
//  },
//
  nextChildToLeft : function(aRange)
  {
    return this.nextNodeChildToLeft(this.mNode, aRange);
  },

  nextNodeChildToLeft : function(aNode, aRange)
  {
    var compNode = null;
// rwa 11-25-08    if (msiNavigationUtils.isAncestor(aNode, aRange.startContainer))
    if (msiNavigationUtils.isAncestor(aRange.startContainer, aNode))
    {
      compNode = msiNavigationUtils.getNodeBeforePosition(aRange.startContainer, aRange.startOffset);
      if (compNode && compNode.parentNode == aNode)
        return compNode;
    }
    else
    {
      compNode = msiNavigationUtils.getNodeBeforePosition(aRange.startContainer, aRange.startOffset);
      if (compNode == aNode)
        return msiNavigationUtils.getLastSignificantChild(compNode);
    }
    return null;
  },

  nextChildToRight : function(aRange)
  {
    return this.nextNodeChildToRight(this.mNode, aRange);
  },

  nextNodeChildToRight : function(aNode, aRange)
  {

    var compNode = null;
// rwa 11-25-08    if (msiNavigationUtils.isAncestor(aNode, aRange.endContainer))
    if (msiNavigationUtils.isAncestor(aRange.endContainer, aNode))
    {
      compNode = msiNavigationUtils.getNodeAfterPosition(aRange.endContainer, aRange.endOffset);
      if (compNode && compNode.parentNode == aNode)
        return compNode;
    }
    else
    {
      compNode = msiNavigationUtils.getNodeAfterPosition(aRange.endContainer, aRange.endOffset);
      if (compNode == aNode)
        return msiNavigationUtils.getFirstSignificantChild(compNode);
    }
//log    msiKludgeLogString( "In nextNodeChildToRight for node [" + this.describeNode(aNode) + "], compNode is [" + this.describeNode(compNode) + "]; returning null.\n", ["search"] );
    return null;
  },

  adjoinRange : function(oldRange, newRange)
  {
//    if (oldRange.compareBoundaryPoints(nsIDOMRange.START_TO_START, newRange) < 0)
    if (msiNavigationUtils.comparePositions(newRange.startContainer, newRange.startOffset, oldRange.startContainer, oldRange.startOffset) < 0)
      oldRange.setStart(newRange.startContainer, newRange.startOffset);
//    if (oldRange.compareBoundaryPoints(nsIDOMRange.END_TO_END, newRange) > 0)
    if (msiNavigationUtils.comparePositions(newRange.endContainer, newRange.endOffset, oldRange.endContainer, oldRange.endOffset) > 0)
      oldRange.setEnd(newRange.endContainer, newRange.endOffset);
  },

  matchReturnString : function(matchedValue)
  {
    switch(matchedValue)
    {
      case msiSearchUtils.completedMatch: 
        return "completedMatch";          break;
      case msiSearchUtils.partialMatch:
        return "partialMatch";            break;
      case msiSearchUtils.cannotMatch:
        return "cannotMatch";             break;
      default:
        return "unrecognized value";      break;
    }
  },

  describe : function()
  {
//    var descStr = "msiMatchNode, with mNode [" + this.mNode.nodeName + "]";
    var descStr = "msiMatchNode, with mNode [" + this.describeNode(this.mNode) + "]";
    return descStr;
  },

  describeNode : function(aNode)
  {
    var descStr = "null";
    if (aNode)
      descStr = aNode.nodeName + "[" + aNode.textContent + "]";
    return descStr;
  },

  describeMsiRange : function(ourRange)
  {
    var descStr = "start " + ourRange.startContainer.nodeName + "[" + ourRange.startContainer.textContent + "]," + ourRange.startOffset;
    descStr += "; end " + ourRange.endContainer.nodeName + "[" + ourRange.endContainer.textContent + "]," + ourRange.endOffset;
    return descStr;
  }

};

function msiTextMatchNode() {}
msiTextMatchNode.prototype =
{
  m_spaceSplitRE : /(\s+)/,
//  mWhiteSpaceTestRE : /^\s*$/,

  init : function(aNode, theFlags, refEditor)
  {
    this.mNode = aNode;
    this.mFlags = theFlags;
    this.mEditor = refEditor;
    this.mText = aNode.textContent;
//    this.mTextPieces = aNode.textContent.split(this.m_spaceSplitRE);
    this.mTextPieces = this.makePiecesArrayFromString(aNode.textContent);
    this.getInheritedAttribs(aNode, refEditor);
    this.mSearchType = msiSearchUtils.getTypeOfNodeSearch(aNode, refEditor);
  },

////  this.extendMatchToLeft = function(matchRange, ourOffset, refEditor)
////  {
////    var retVal = msiSearchUtils.partialMatch;
////    if (this.mText && (ourOffset < 0) )
////      ourOffset = this.mText.length;
////
////  }

//  this.extendMatchToLeft = function(matchRange, ourOffset, refEditor)
//  {
//    var retVal = msiSearchUtils.partialMatch;
//    if (this.mText && (ourOffset < 0) )
//      ourOffset = this.mText.length;
//    if (ourOffset == 0)
//      return msiSearchUtils.completedMatch;
//    var theTarget = matchRange.startContainer;
//    var theOffset = matchRange.startOffset;
//    while ( (retVal == msiSearchUtils.partialMatch) && (ourOffset > 0) )
//    {
//      var theTarget = this.getNextTargetToLeft(theTarget, theOffset);
//      if (theTarget == null)
//        return msiSearchUtils.cannotMatch;
//      var nodeType = msiSearchUtils.getTypeOfNodeSearch(theTarget);
////      if (nodeType == "blockContainer")
//      if (this.shouldSkipOverWhiteSpaceWhenMatchingBoundary(nodeType))
//        ourOffset = moveLeftOfTrailingWhiteSpace(ourOffset);
//      switch(this.canMatch(nodeType))
//      {
//        START HERE! This isnt right! What do we do to indicate that we just want to move the left position outside an object and try again?
//        case msiSearchUtils.matchPieceByPiece:
//        case msiSearchUtils.matchOne:
//        //Now try to match our right-hand end against targText's, where right-hand end means, of course, taking offsets into account.
//        {
//          var targText = theTarget.textContent;
//          if ( (theOffset >= 0) && (theOffset < targText.length) )
//            targText = targText.substr(0, theOffset);
//          var matchLen = ourOffset;
//          if (ourOffset < 0)
//            matchLen = this.mText.length; ?????
//          if (targText.length < this.mText
//        }
//        break;
//        default:
//          retVal = msiSearchUtils.cannotMatch;
//        break;
//      }
//    }
//  };

//  match : function(targetNode, targRange)
//  {
//    var matched = msiSearchUtils.cannotMatch;
//
//    return matched;
//  },

  doLeftMatchCheck : function(matchRange, ourRange, origTarget, bNewTargNode)
  {
    var bMatched = false;
    var targNode = matchRange.startContainer;
    var targOffset = matchRange.startOffset;
    var bMustMatchContainingNode = false;
    if (origTarget != null)
      bMustMatchContainingNode = true;
    var retVal = msiSearchUtils.partialMatch;
    var bNeedNodeMatch = false;
    if (ourRange.startContainer != this.mNode)
    {
      var localNodeToMatch = this.nextNodeChildToLeft(this.mNode.parentNode, ourRange);
      if (localNodeToMatch == this.mNode)
      {
        ourRange.setStart(this.mNode, this.mText.length);
        bNewTargNode = true;
      }
      else
      {
        msiKludgeLogString( "In msiTextMatchNode.doLeftMatchCheck, incoming 'ourRange' doesn't point to this text node!\n", ["search"] );
        return msiSearchUtils.cannotMatch;
      }
    }
    var ourOffset = ourRange.startOffset;
    if (bNewTargNode)
      bNeedNodeMatch = !this.doStructuralNodeMatch(targNode);
    
    if (!bNeedNodeMatch)
    {
      //We know we're looking at a #text node, so we can just move on to examining the string.

      var targText = targNode.textContent;
      if ( (targOffset < 0) || (targOffset > targText.length) )
        targOffset = targText.length;
//      var targPieces = targText.split(this.m_spaceSplitRE);
      var targPieces = this.makePiecesArrayFromString(targText);
      var targPos = this.totalOffsetToPieceAndOffset(targPieces, targOffset);
      var ourPos = this.totalOffsetToPieceAndOffset(this.mTextPieces, ourOffset);

//      var matchLen = ourOffset;
//      var strToMatch = this.mText;
//      if (ourOffset < 0)
//        matchLen = this.mText.length;
//      else
//        strToMatch = this.mText.substr(0, matchLen);
//
//      bMatched = this.doMatchString(strToMatch, targText);
      bMatched = true;
      do
      {
        this.adjustPositionAndOffsetLeft(targPieces, targPos);
        this.adjustPositionAndOffsetLeft(this.mTextPieces, ourPos);
        if ( !( (ourPos.nPiece > 0) || (ourPos.nOffset > 0) ) || !( (targPos.nPiece > 0) || (targPos.nOffset > 0) ) )
          break;
        bMatched = this.doMatchSubStringLeft(ourPos, targPieces, targPos);
      }
      while (bMatched);

      if (bMatched)
      {
        //Now, adjust ourOffset and the target stuff and match ranges.
        ourOffset = this.pieceAndOffsetToTotalOffset(this.mTextPieces, ourPos);
        targOffset = this.pieceAndOffsetToTotalOffset(targPieces, targPos);
        if (ourOffset <= 0)
        {
          ourRange.setStartBefore(this.mNode);
          retVal = msiSearchUtils.completedMatch;
        }
        else
          ourRange.startOffset = ourOffset;
        if (targOffset <= 0)
          matchRange.setStartBefore(targNode);
        else
          matchRange.setStart(targNode, targOffset);
      }

//      targOffset = matchRange.startOffset;
//      targNode = matchRange.startContainer;
    }
    return retVal;
  },

  doRightMatchCheck : function(matchRange, ourRange, origTarget, bNewTargNode)
  {
//log    msiKludgeLogString( "Entering doRightMatchCheck for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(matchRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
    var bMatched = false;
    var targNode = matchRange.endContainer;
    var targOffset = matchRange.endOffset;
    var bMustMatchContainingNode = false;
    if (origTarget != null)
      bMustMatchContainingNode = true;
    var retVal = msiSearchUtils.partialMatch;
    if (ourRange.endContainer != this.mNode)
    {
      var localNodeToMatch = this.nextNodeChildToRight(this.mNode.parentNode, ourRange);
      if (localNodeToMatch == this.mNode)
      {
        ourRange.setEnd(this.mNode, 0);
        bNewTargNode = true;
      }
      else
      {
//log        msiKludgeLogString( "In msiTextMatchNode.doRightMatchCheck, incoming 'ourRange' is [" + this.describeMsiRange(ourRange) + "]; doesn't point to this text node!\n", ["search"] );
        return msiSearchUtils.cannotMatch;
      }
    }
    var ourOffset = ourRange.endOffset;
    var bNeedNodeMatch = false;
    if (bNewTargNode)
      bNeedNodeMatch = !this.doStructuralNodeMatch(targNode);
    
    if (!bNeedNodeMatch)
    {
      //We know we're looking at a #text node, so we can just move on to examining the string.

      var targText = targNode.textContent;
      if (targOffset < 0)
        targOffset = 0;
      else if (targOffset > targText.length)
        targOffset = targText.length;
//      var targPieces = targText.split(this.m_spaceSplitRE);
      var targPieces = this.makePiecesArrayFromString(targText);
      var targPos = this.totalOffsetToPieceAndOffset(targPieces, targOffset);
      var ourPos = this.totalOffsetToPieceAndOffset(this.mTextPieces, ourOffset);

//      var matchLen = ourOffset;
//      var strToMatch = this.mText;
//      if (ourOffset < 0)
//        matchLen = this.mText.length;
//      else
//        strToMatch = this.mText.substr(0, matchLen);
//
//      bMatched = this.doMatchString(strToMatch, targText);
      bMatched = true;
      do
      {
        this.adjustPositionAndOffsetRight(targPieces, targPos);
        this.adjustPositionAndOffsetRight(this.mTextPieces, ourPos);
        if (  !( (ourPos.nPiece < this.mTextPieces.length - 1) || ((ourPos.nPiece == this.mTextPieces.length - 1) && (ourPos.nOffset < this.mTextPieces[ourPos.nPiece].length)) ) 
                       || !( (targPos.nPiece < targPieces.length - 1) || ((targPos.nPiece == targPieces.length - 1) && (targPos.nOffset < targPieces[targPos.nPiece].length)) )  )
          break;
        bMatched = this.doMatchSubStringRight(ourPos, targPieces, targPos);
      }
      while ( bMatched );

      if (bMatched)
      {
        //Now, adjust ourOffset and the target stuff and match ranges.
        ourOffset = this.pieceAndOffsetToTotalOffset(this.mTextPieces, ourPos);
        targOffset = this.pieceAndOffsetToTotalOffset(targPieces, targPos);
        if (ourOffset >= this.mText.length)
        {
          retVal = msiSearchUtils.completedMatch;
          ourRange.setEndAfter(this.mNode);
        }
        else
          ourRange.endOffset = ourOffset;
        if (targOffset >= targText.length)
          matchRange.setEndAfter(targNode);
        else
          matchRange.setEnd(targNode, targOffset);
      }

//      targOffset = matchRange.startOffset;
//      targNode = matchRange.startContainer;
    }
//log    msiKludgeLogString( "Returning [" + this.matchReturnString(retVal) + "] from doRightMatchCheck for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(matchRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
    return retVal;
  },

  makePiecesArrayFromString : function(aString)
  {
    var targPieces = aString.split(this.m_spaceSplitRE);
    if ( (targPieces.length > 1) && !targPieces[0].length )
      targPieces.shift();
    if ( (targPieces.length > 1) && !targPieces[targPieces.length - 1].length )
      targPieces.pop();
    return targPieces;
  },

  adjustPositionAndOffsetLeft : function(thePieces, thePos)
  {
    var bNewPos = false;
    if (thePos.nOffset == 0)
    {
      --thePos.nPiece;
      bNewPos = true;
      if (thePos.nPiece < 0)
      {
        bNewPos = false;
        thePos.nOffset = -1;
      }
      else
        thePos.nOffset = thePieces[thePos.nPiece].length;
    }
    return bNewPos;
  },

  adjustPositionAndOffsetRight : function(thePieces, thePos)
  {
    var bNewPos = false;
    if (thePos.nPiece < 0)
    {
      thePos.nPiece = 0;
      thePos.nOffset = 0;
      bNewPos = true;
    }
    else if (thePos.nPiece >= thePieces.length)
    {
      thePos.nOffset = 0;
    }
    else if (thePos.nOffset >= thePieces[thePos.nPiece].length)
    {
      ++thePos.nPiece;
      thePos.nOffset = 0;
      bNewPos = true;
    }
    return bNewPos;
  },

  doMatchSubStringLeft : function(ourPosition, targetPieces, targetPosition)
  {
    var bMatched = false;
    var nMatchlen = 0;
//log    if (ourPosition.nPiece >= this.mTextPieces.length)
//log      msiKludgeLogString( "In msiTextMatchNode.doMatchSubStringLeft, nPiece is too big [" + nPiece + "] for node [" + this.describe() + "].\n", ["search"] );

    var ourPiece = this.mTextPieces[ourPosition.nPiece];
//log    if (!ourPiece)
//log      msiKludgeLogString( "In msiTextMatchNode.doMatchSubStringLeft, ourPiece is too null, for nPiece [" + nPiece + "] and node [" + this.describe() + "].\n", ["search"] );
    if (ourPosition.nOffset < ourPiece.length)
      ourPiece = ourPiece.substr(0, ourPosition.nOffset);
    var targPiece = targetPieces[targetPosition.nPiece];
    if (targetPosition.nOffset < targPiece.length)
      targPiece = targPiece.substr(0, targetPosition.nOffset);

    if (msiNavigationUtils.isWhiteSpace(ourPiece))
    {
      bMatched = (ourPiece == targPiece);
      if (!bMatched && msiNavigationUtils.isWhiteSpace(targPiece))
      {
        if (ourPiece.length < targPiece.length)
          bMatched = true;
      }
      if (bMatched)
      {
        ourPosition.nOffset -= ourPiece.length;
        targetPosition.nOffset -= targPiece.length;
      }
    }
    else
    {
      if (!this.matchIsCaseSensitive())
      {
        ourPiece = ourPiece.toLowerCase();
        targPiece = targPiece.toLowerCase();
      }
      nMatchlen = ourPiece.length;
      if (targPiece.length < nMatchlen)
        nMatchlen = targPiece.length;
      bMatched = (ourPiece.substr(ourPiece.length - nMatchlen) == targPiece.substr(targPiece.length - nMatchlen));
//      bMatched = (ourPiece.substr(nMatchlen) == targPiece.substr(nMatchlen));
      if (bMatched)
      {
        ourPosition.nOffset -= nMatchlen;
        targetPosition.nOffset -= nMatchlen;
      }
    }

    return bMatched;
  },

  doMatchSubStringRight : function(ourPosition, targetPieces, targetPosition)
  {
    var bMatched = false;
    var nMatchlen = 0;
//log    if (ourPosition.nPiece >= this.mTextPieces.length)
//log      msiKludgeLogString( "In msiTextMatchNode.doMatchSubStringRight, nPiece is too big [" + nPiece + "] for node [" + this.describe() + "].\n", ["search"] );
    var ourPiece = this.mTextPieces[ourPosition.nPiece];
//log    if (!ourPiece)
//log      msiKludgeLogString( "In msiTextMatchNode.doMatchSubStringRight, ourPiece is too null, for nPiece [" + nPiece + "] and node [" + this.describe() + "].\n", ["search"] );
    if (ourPosition.nOffset > 0)
      ourPiece = ourPiece.substr(ourPosition.nOffset);
    var targPiece = targetPieces[targetPosition.nPiece];
    if (targetPosition.nOffset > 0)
      targPiece = targPiece.substr(targetPosition.nOffset);

    if (msiNavigationUtils.isWhiteSpace(ourPiece))
    {
      bMatched = (ourPiece == targPiece);
      if (!bMatched && msiNavigationUtils.isWhiteSpace(targPiece))
      {
        if (ourPiece.length < targPiece.length)
          bMatched = true;
      }
      if (bMatched)
      {
        ourPosition.nOffset += ourPiece.length;
        targetPosition.nOffset += targPiece.length;
      }
    }
    else
    {
      if (!this.matchIsCaseSensitive())
      {
        ourPiece = ourPiece.toLowerCase();
        targPiece = targPiece.toLowerCase();
      }

      nMatchlen = ourPiece.length;
      if (targPiece.length < nMatchlen)
        nMatchlen = targPiece.length;
      bMatched = (ourPiece.substr(0, nMatchlen) == targPiece.substr(0, nMatchlen));
      if (bMatched)
      {
        ourPosition.nOffset += nMatchlen;
        targetPosition.nOffset += nMatchlen;
      }
    }

    return bMatched;
  },

//  doMatchString : function(strToMatch, targText)
//  {
//    if (targText.length < strToMatch.length)
//      bMatched = 
//    else  //look to match our whole string
//      bMatched = ??
//  },

//Note that problems arise when the totalOffset is at the right end of the string!
  totalOffsetToPieceAndOffset : function(pieceArray, totalOffset)
  {
    var retVal = new Object();
    retVal.nPiece = -1;
    retVal.nOffset = -1;
    for (var ix = 0; ix < pieceArray.length; ++ix)
    {
      if (totalOffset <= pieceArray[ix].length)
      {
        retVal.nPiece = ix;
        retVal.nOffset = totalOffset;
        break;
      }
      totalOffset -= pieceArray[ix].length;
    }
    return retVal;
  },

  pieceAndOffsetToTotalOffset : function(pieceArray, posObj)
  {
    var retVal = posObj.nOffset;
    if (retVal < 0)
      retVal = 0;  //is this right????
    for (var ix = 0; ix < posObj.nPiece; ++ix)
    {
      retVal += pieceArray[ix].length;
    }
    return retVal;
  },

  shouldSkipOverWhiteSpaceWhenMatchingBoundary : function(nodeType)
  {
    switch(nodeType)
    {
      case "blockContainer":
      case "mathTemplate":  
      case "mathContainer":
      case "find":
        return true;
      break;
      default:
        return false;
      break;
    }
  },

  moveLeftOfTrailingWhiteSpace : function(anOffset)
  {
    var leftText = this.mText.substr(0, anOffset);
    if (leftText.length == 0)
      return 0;
    var regExp = /(\s+)$/;
    var foundIndex = leftText.search(regExp);
    if (foundIndex < 0)
      return anOffset;
    return foundIndex;
  },

  moveRightOfLeadingWhiteSpace : function(anOffset)
  {
    var rightText = this.mText.substring(anOffset);
//    if (rightText.length == 0)  this looks unnecessary
//      return this.mText.length;
    var regExp = /\S/;
    var foundIndex = rightText.search(regExp);
    if (foundIndex < 0)
      return this.mText.length;  //move to the end if there's no non-whitespace
    return (foundIndex + anOffset);
  },

  canExtendOutside : function(aNode)
  {
    var nodeType = msiSearchUtils.getTypeOfNodeSearch(aNode, this.mEditor);
    switch(nodeType)
    {
      case "textContainer":
      case "blockContainer":
      case "text":
        return true;
      break;
      case "mathTemplate":  
      case "mathContainer":
      case "find":
      default:
        return false;
      break;
    }
  },

//  isWhiteSpace : function(aTextPiece)
//  {
//    return ( this.mWhiteSpaceTestRE.test(aTextPiece) );
//  },

  describe : function()
  {
    var descStr = "msiTextMatchNode, with mNode [" + this.mNode.nodeName + "], with [" + this.mTextPieces.length + "] text pieces: ";
    for (var ix = 0; ix < this.mTextPieces.length; ++ix)
    {
      if (ix > 0)
        descStr += ",";
      descStr += "[" + this.mTextPieces[ix] + "]";
    }
//    descStr += "\n";
    return descStr;
  }

}

msiTextMatchNode.prototype.__proto__ = msiMatchNode.prototype;

function msiTemplateMatchNode() {}
msiTemplateMatchNode.prototype =
{
  init : function(theNode, theFlags, refEditor)
  {
    this.mNode = theNode;
    this.mFlags = theFlags;
    this.mEditor = refEditor;
    this.mMatchInfo = XPathStringFormatterBase.getMatchingNodeInfo( msiGetBaseNodeName(theNode), theFlags);
    this.getInheritedAttribs(theNode, refEditor);
    this.mSearchType = msiSearchUtils.getTypeOfNodeSearch(theNode, refEditor);
  },

  match : function(targetNode, targRange)
  {
    var matched = msiSearchUtils.cannotMatch;
    var targNodeName = msiGetBaseNodeName(targetNode);
    var ourBaseName = msiGetBaseNodeName(this.mNode);
    var bMatched = false;
    var childMatchInfo = null;
    var localTargRange = this.mEditor.document.createRange();
    localTargRange.setStart(targetNode, 0);
    localTargRange.setEnd(targetNode, 0);
//log    msiKludgeLogString( "Entering match for node [" + this.describe() + "]; targRange is [" + this.describeMsiRange(targRange) + "];\n  ourTargRange is [" + this.describeMsiRange(localTargRange) + "].\n", ["search"] );
    var bTemplateMayMatch = false;
    for (var ix = 0; ix < this.mMatchInfo.length; ++ix)
    {
      if (targNodeName == this.mMatchInfo[ix].theName)
      {
        bMatched = bTemplateMayMatch = true;
        var theContents = msiNavigationUtils.getSignificantContents(this.mNode);
        var targetContents = msiNavigationUtils.getSignificantContents(targetNode);
        for (var jx = 0; bMatched && (jx < theContents.length); ++jx)
        {
        	if (!msiNavigationUtils.isEmptyInputBox(theContents[jx]))
          {
            var matchPosition = msiSearchUtils.getMatchPositionForChild(ourBaseName, targNodeName, jx);
            if ( (matchPosition.length > 0) && (matchPosition != "0") )
            {
              if ( (matchPosition == "*") || msiSearchUtils.isMRowLike(targNodeName))
              {
              //What to do here? We're a well-behaved template, like an mroot, but we're trying to match, perhaps
              //  a msqrt. In this case try to match our contents against all of theirs?
                msiKludgeLogString( "In msiMatchNode.match(), with target [" + targNodeName + "], in MRowLike clause.\n", ["search"] );
                childMatchInfo = createMatchNode(theContents[jx], this.mFlags, this.mEditor);
//                localTargRange.selectNode(targetNode);
                localTargRange.setStartBefore(targetNode);
                localTargRange.collapse(true);
                bMatched = msiSearchUtils.isMatching(childMatchInfo.match(targetNode, localTargRange)); //this is wrong?
              }  
              else  
              {
                msiKludgeLogString( "In msiMatchNode.match(), with target [" + targNodeName + "], in non-MRowLike clause.\n", ["search"] );
                var nMatchPos = Number(matchPosition) - 1;  //the positions returned above are one-based rather than zero
                if (nMatchPos < targetContents.length)
                {
                  childMatchInfo = createMatchNode(theContents[jx], this.mFlags, this.mEditor);
//                  localTargRange.selectNode(targetContents[nMatchPos]);
                  localTargRange.setStartBefore(targetContents[nMatchPos]);
                  localTargRange.collapse(true);
                  bMatched = msiSearchUtils.isMatching(childMatchInfo.match(targetContents[nMatchPos], localTargRange));
                }
                else
                  bMatched = false;
              }  
            }
          }  
        }
        break;
      }
    }
    if (bMatched)
    {
      targRange.selectNode(targetNode);
      matched = msiSearchUtils.completedMatch;
    }
    else
      matched = msiSearchUtils.cannotMatch;
//log    if (bTemplateMayMatch)
//log      msiKludgeLogString( "Returning [" + this.matchReturnString(matched) + "] from msiMatchNode.match(), with target [" + targetNode.nodeName + "] and targRange [" + targRange.toString() + "], for matching node [" + this.describe() + "].\n", ["search"] );
//log    else
//log      msiKludgeLogString( "Template structural mismatch from msiMatchNode.match(), with target [" + targNodeName + "] and targRange [" + targRange.toString() + "], for matching node [" + this.describe() + "].\n", ["search"] );
    return matched;
  },

  doLeftMatchCheck : function(matchRange, ourRange, origTarget, bNewTargNode)
  {

    //The "bNeedNodeMatch" cases other than "bMustMatchContainingNode" simply mean that we're looking through the target's children 
    // trying to match our node rather than its children - thus our offset is at the end of an object which must find a matching node. 
    // Again, we try to match as long as the targOffset can be safely moved left, but in this case we don't try to match our children 
    // here; once we find a matching node, we'll set "bNeedNodeMatch" to false and move theTarget inside it.
//      else if (bNeedNodeMatch)
    var bMustMatchContainingNode = false;
    var targNode = matchRange.startContainer;
    var targOffset = matchRange.startOffset;
    if (origTarget != null)
      bMustMatchContainingNode = true;
    var retVal = msiSearchUtils.cannotMatch;
    var localTargRange = this.mEditor.document.createRange();
    if (!bMustMatchContainingNode)
    {
//NOTE! The following may in fact be needed, but it seems wrong. Keep an eye out for this case.
//      if (this.offsetIsAtEnd(targNode, targOffset))  //In this case, the entire targNode is a candidate for us to match??
//        bNeedNodeMatch = !this.doStructuralNodeMatch(targNode);
      if ( (targOffset > 0) && targNode.childNodes && (targOffset <= targNode.childNodes.length) )
      {
//        localTargRange.selectNode( targNode.childNodes[targOffset-1] );
        localTargRange.setEndAfter(targNode.childNodes[targOffset-1]);
        localTargRange.collapse(false);
        retVal = this.match(targNode.childNodes[targOffset-1], localTargRange);
      }
      if (msiSearchUtils.isMatching(retVal))
      {
//        --targOffset;
//        matchRange.startOffset = targOffset;
        this.adjoinRange(matchRange, localTargRange);
        ourRange.setStartBefore(this.mNode);
      }
    }
    else
    {
      if (msiNavigationUtils.isAncestor(targNode, origTarget))
      {
//        localtargRange.selectNode(targNode);
        localTargRange.setEndAfter(targNode);
        localTargRange.collapse(false);
        bMustMatchContainingNode = !this.match(targNode, localTargRange);
      }
      if (msiSearchUtils.isMatching(retVal))
      {
        ourRange.setStartBefore(this.mNode);
////        --ourOffset;
//        matchRange.setStartBefore(targNode);
        this.adjoinRange(matchRange, localTargRange);
////        targOffset = matchRange.startOffset;
////        targNode = matchRange.startContainer;
      }
    }
    return retVal;
  },

  doRightMatchCheck : function(matchRange, ourRange, origTarget, bNewTargNode)
  {

    //The "bNeedNodeMatch" cases other than "bMustMatchContainingNode" simply mean that we're looking through the target's children 
    // trying to match our node rather than its children - thus our offset is at the end of an object which must find a matching node. 
    // Again, we try to match as long as the targOffset can be safely moved left, but in this case we don't try to match our children 
    // here; once we find a matching node, we'll set "bNeedNodeMatch" to false and move theTarget inside it.
//      else if (bNeedNodeMatch)
//log    msiKludgeLogString( "Entering doRightMatchCheck for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(matchRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
    var bMustMatchContainingNode = false;
    var targNode = matchRange.endContainer;
    var targOffset = matchRange.endOffset;
    if (origTarget != null)
      bMustMatchContainingNode = true;
    var retVal = msiSearchUtils.cannotMatch;
    var localTargRange = this.mEditor.document.createRange();
    if (!bMustMatchContainingNode)
    {
//NOTE! The following may in fact be needed, but it seems wrong. Keep an eye out for this case.
//      if (this.offsetIsAtEnd(targNode, targOffset))  //In this case, the entire targNode is a candidate for us to match??
//        bNeedNodeMatch = !this.doStructuralNodeMatch(targNode);
      if ( (targOffset >= 0) && targNode.childNodes && (targOffset < targNode.childNodes.length) )
      {
//        localTargRange.selectNode( targNode.childNodes[targOffset]);
        localTargRange.setStartBefore(targNode.childNodes[targOffset]);
        localTargRange.collapse(true);
        retVal = this.match(targNode.childNodes[targOffset], localTargRange);
      }
      if (msiSearchUtils.isMatching(retVal))
      {
        this.adjoinRange(matchRange, localTargRange);
//        ++targOffset;
//        matchRange.endOffset = targOffset;
        ourRange.setEndAfter(this.mNode);
      }
    }
    else
    {
      if (msiNavigationUtils.isAncestor(targNode, origTarget))
      {
//        localTargRange.selectNode(targNode);
        localTargRange.setStartBefore(targNode);
        localTargRange.collapse(true);
        bMustMatchContainingNode = !this.match(targNode, localTargRange);
      }
      if (msiSearchUtils.isMatching(retVal))
      {
//        --ourOffset;
        ourRange.setEndAfter(this.mNode);
        this.adjoinRange(matchRange, localTargRange);
//        matchRange.setEndAfter(targNode);
////        targOffset = matchRange.startOffset;
////        targNode = matchRange.startContainer;
      }
    }
//log    msiKludgeLogString( "Returning [" + this.matchReturnString(retVal) + "] from doRightMatchCheck for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(matchRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
    return retVal;
  },

  describe : function()
  {
    var descStr = "msiTemplateMatchNode, with mNode [" + this.mNode.nodeName + "]";
    return descStr;
  }

};

msiTemplateMatchNode.prototype.__proto__ = msiMatchNode.prototype;

//The msiContainerTemplateMatchNode currently applies only to <msqrt>. The problem this addresses is that an <msqrt> can match
//  like a hybrid template if it's matching an <mroot> node with "root" equal to 2 - i.e., matching contents means getting them
//  from the first child of the target node. (This also provides a convenient place to encode the check on the root value being 2.)
function msiContainerTemplateMatchNode() {}
msiContainerTemplateMatchNode.prototype = 
{
  init: function(theNode, theFlags, refEditor)
  {
    this.mNode = theNode;
    this.mFlags = theFlags;
    this.mEditor = refEditor;
    this.mContents = msiSearchUtils.getSearchableContentNodes(this.mNode);
    this.getInheritedAttribs(theNode, refEditor);
    this.mSearchType = msiSearchUtils.getTypeOfNodeSearch(theNode, refEditor);
  },

  doStructuralNodeMatch : function(aTarget)
  {
    var retVal = false;
    switch(msiGetBaseNodeName(this.mNode))
    {
      case "msqrt":
        if (msiGetBaseNodeName(aTarget) == "mroot")
        {
          var rootChild = msiNavigationUtils.getIndexedSignificantChild(aTarget, 1);
          if (!rootChild || msiSearchUtils.isEmptyElement(rootChild) || (rootChild.textContent == "2"))
            return this.targNodeIsCompatible(aTarget);
          else
            return false;
        }
      break;
      default:
        //The default case is handled below, for now...
      break;
    }
    return msiMatchNode.prototype.doStructuralNodeMatch.call(this, aTarget);
  },

  match : function(aTarget, targRange)
  {
    //start here - idea is to use msiMatchNode.match.call(this, aTarget) if aTarget matches this, otherwise if aTarget is an 
    //  mroot with root value 2 and we're msqrt, we'll move the target range into aTarget.childNodes[0] and use msiMatchNode.extendToRight.call().
    var retVal = msiSearchUtils.cannotMatch;
    if (!this.doStructuralNodeMatch(aTarget))
      return retVal;

    var localTargNode = msiSearchUtils.getMatchNodeForChild(this.mNode, aTarget, 0);
    if (!localTargNode)
    {
//log      msiKludgeLogString("In match() for [" + this.describe() + "], getMatchNodeForChild returned null! We're returning cannotMatch.\n", "search" );
      return retVal;
    }

    var localMatch = msiSearchUtils.partialMatch;
    var localNodeToMatch = null;
    var localMatcher = null;
    var localTargRange = targRange.cloneRange();
    var currTarget = null;
    var currOffset = 0;
    var bNewTargNode = true;
    localTargRange.setStart(localTargNode, 0);
    localTargRange.setEnd(localTargNode, 0);
    var ourRange = createMsiMatchingRange();
    ourRange.setStart(this.mNode, 0);
    ourRange.setEnd(this.mNode, 0);
//    msiMatchNode.prototype.extendMatchToRight.call(this, 

    var bMoreToGo = this.haveContentAfterRangeEnd(ourRange);
    var bTargOffsetChanged = true;
    if (msiSearchUtils.isEmptyElement(this.mNode))
    {
      bMoreToGo = false;
      localMatch = msiSearchUtils.completedMatch;
      localTargRange.setEnd(localTargRange.endContainer, msiNavigationUtils.lastOffset(localTargRange.endContainer));
    }
    while (bTargOffsetChanged && bMoreToGo)
    {
      currTarget = localTargRange.endContainer;
      currOffset = localTargRange.endOffset;

      localNodeToMatch = this.nextChildToRight(ourRange);
      localMatch = msiSearchUtils.cannotMatch;
      if (localNodeToMatch != null)
      {
        localMatcher = createMatchNode(localNodeToMatch, this.mFlags, this.mEditor);
        localMatch = localMatcher.extendMatchToRight(localTargRange, ourRange, this.mEditor);
      }
      else
      {
//log        msiKludgeLogString( "localNodeToMatch is null in doRightMatchCheck for node [" + this.describe() + "]!\n", ["search"] );
        msiNavigationUtils.comparePositions(this.mNode, msiNavigationUtils.lastOffset(this.mNode), ourRange.endContainer, ourRange.endOffset, bLoggingSearchStuff);
      }

      bNewTargNode = (localTargRange.endContainer != currTarget);
      bTargOffsetChanged = (bNewTargNode || (currOffset != localTargRange.endOffset));
      if (!bTargOffsetChanged)
      {
        bTargOffsetChanged = this.adjustRightStartingTargPosition(localTargRange, ourRange);
//      if (this.adjustRightStartingTargPosition(localTargRange, ourRange, refEditor))
//      {
//        bTargOffsetChanged = true;
//log        if (bTargOffsetChanged)
//log          msiKludgeLogString( "adjustRightStartingTargPosition() call changed offset in extendMatchToRight for node [" + this.describe() + "]; localTargRange is [" + this.describeMsiRange(localTargRange) + "];\n  ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
      }
      bMoreToGo = this.haveContentAfterRangeEnd(ourRange);
    }

//What do we want to do with the value of localMatch?
    if (bMoreToGo)
      retVal = msiSearchUtils.cannotMatch;
    else if (msiSearchUtils.isMatching(localMatch))
    {
      ourRange.setStartBefore(this.mNode);
      ourRange.setEndAfter(this.mNode);
      this.adjoinRange(targRange, localTargRange);
      retVal = msiSearchUtils.completedMatch;
    }

//log    msiKludgeLogString( "Returning [" + this.matchReturnString(retVal) + "] from match for node [" + this.describe() + "]; matchRange is [" + this.describeMsiRange(targRange) + "]; ourRange is [" + this.describeMsiRange(ourRange) + "].\n", ["search"] );
    return retVal;
  },
  
  describe : function()
  {
    var descStr = "msiContainerTemplateMatchNode, with mNode [" + this.mNode.nodeName + "]";
    return descStr;
  }
};

msiContainerTemplateMatchNode.prototype.__proto__ = msiTemplateMatchNode.prototype;

function msiMathContainerMatchNode() {}
msiMathContainerMatchNode.prototype =
{

  init: function(theNode, theFlags, refEditor)
  {
    this.mNode = theNode;
    this.mFlags = theFlags;
    this.mEditor = refEditor;
    this.mContents = msiSearchUtils.getSearchableContentNodes(this.mNode);
    this.getInheritedAttribs(theNode, refEditor);
    this.mSearchType = msiSearchUtils.getTypeOfNodeSearch(theNode, refEditor);
  },

  doStructuralNodeMatch : function(aTarget)
  {
    return this.targNodeIsCompatible(aTarget);
  },

//  getLastChild : function()
//  {
//    var retNode = null;
//    if ( (this.mContents != null) && (this.mContents.length > 0) )
//      retNode = this.mContents[this.mContents.length - 1];
//    return retNode;
//  },
//
//  match : function(targetNode)
//  {
//    WHAT GOES HERE? anything? or just use the base function?
//  },

//  //This function assumes that we have a match of what's to our right with the various ranges (probably just one) in rangeArray.
//  //The first thing to be done is to determine whether we can extend the match at all. If we're some sort of container and we
//  //  have more to our left to match, the answer is tentatively yes. If we're a template of some sort (like a <mfrac>), the answer
//  //  is probably no. 
//  //Then we have to examine each of the ranges to see what we encounter if we look to the left; if we're leaving an object, 
//  //  can our match extend across its boundary? This is generally a difficult one to answer. If we're a template, we really don't
//  //  want to be in this function anyway, in some sense; a template should try to match each of its component pieces. But does
//  //  that mean we don't end up here or want to sometimes return true? Generally, though, we can return false in that case? What
//  //  should happen if we're a fraction that has matched its denominator and wants to match its numerator, or a fraction which is
//  //  empty and thus matches any fraction? It remains true that we must match an entire <mfrac>, and then can't extend beyond it.
//  extendToLeft : function(rangeArray, offset)
//  {
//    if (offset == 0)  //what should this mean? We want to extend to left but we're at our beginning - return still extending?
//      return msiSearchUtils.completedMatch;  //Means we've completed our requirements and the match is still good.
//
//    var nextNode = null;
//    var targParent = null;
//    var nextOffset = 0;
//    var nextToTest = null;
//    if ((offset < 0) || (offset > this.mNode.childNodes.length))  //offset = -1 is passed in if the current matching position is to our right
//      nextNode = this.getLastChild();
//    else
//      nextNode = this.mNode.childNodes[offset - 1];
//
//    for (var ix = rangeArray.length - 1; rangeArray >= 0; --ix)
//    {
//      targParent = rangeArray[ix].startContainer;
//      nextOffset = rangeArray[ix].startOffset;
//      if (msiSearchUtils.isContainer(targParent))
//      {
//      }
//      else
//        rangeArray.splice(ix, 1);  //remove this one - we can't extend it
////      NOW WHAT??  Want to look at the targParent to see whether something of our kind even CAN extend into something of their kind.
////      For instance, if we're a template object like a radical, we only extend against other radicals? Or does it depend on our offset?
//    }
//  },

//  doLeftMatchCheck : function(matchRange, targNode, targOffset, ourOffset, origTarget, bNewTargNode)
//  {
//    WHAT GOES HERE? anything? or just use the base function?
//  }

  describe : function()
  {
    var descStr = "msiMathContainerMatchNode, with mNode [" + this.mNode.nodeName + "]";
    return descStr;
  }
};

msiMathContainerMatchNode.prototype.__proto__ = msiMatchNode.prototype;

function msiTextContainerMatchNode() {}
msiTextContainerMatchNode.prototype = 
{
  init: function(theNode, theFlags, refEditor)
  {
    this.mNode = theNode;
    this.mFlags = theFlags;
    this.mEditor = refEditor;
//    this.mContents = msiSearchUtils.getSearchableContentNodes(this.mNode);
    this.getInheritedAttribs(theNode, refEditor);
    this.mSearchType = msiSearchUtils.getTypeOfNodeSearch(theNode, refEditor);
  },

  doStructuralNodeMatch : function(aTarget)
  {
    return this.targNodeIsCompatible(aTarget);
  },

  describe : function()
  {
    var descStr = "msiTextContainerMatchNode, with mNode [" + this.mNode.nodeName + "]";
    return descStr;
  }
};

msiTextContainerMatchNode.prototype.__proto__ = msiMatchNode.prototype;

function msiBlockContainerMatchNode() {}
msiBlockContainerMatchNode.prototype = 
{
  doStructuralNodeMatch : function(aTarget)
  {
    return this.targNodeIsCompatible(aTarget);
  },

  describe : function()
  {
    var descStr = "msiBlockContainerMatchNode, with mNode [" + this.mNode.nodeName + "]";
    return descStr;
  }
};

msiBlockContainerMatchNode.prototype.__proto__ = msiMatchNode.prototype;

function msiAnonContainerMatchNode() {}
msiAnonContainerMatchNode.prototype =
{
//  init: function(theNode, theFlags, refEditor)
//  {
//    this.mNode = theNode;
//    this.mFlags = theFlags;
//    this.mEditor = refEditor;
//    this.getInheritedAttribs(theNode, refEditor);
//    this.mSearchType = msiSearchUtils.getTypeOfNodeSearch(theNode, refEditor);
//  },

  doStructuralNodeMatch : function(aTarget)
  {
    return this.targNodeIsCompatible(aTarget);
  },

  describe : function()
  {
    var descStr = "msiAnonContainerMatchNode, with mNode [" + this.mNode.nodeName + "]";
    return descStr;
  }

};

msiAnonContainerMatchNode.prototype.__proto__ = msiMatchNode.prototype;

//Now let's try again to get the classification right.
//The following should be the "matchNode" types:
//   (i) mathContainer - this matches generally if its contents match; there may be some structural matching (as in <msqrt> or <mphantom>).
//        As such, it matches "matchPieceByPiece", and determines extending a match piece by piece.
//  (ii) mathTemplate - this matches only if the target is one of its allowed types and if corresponding pieces match.
//        Thus it matches "matchCorrespondingPieces"; extending only occurs if a structural match is present.
// (iii) mathLeaf - this matches only if the target is like it and the contents match.
//        It matches "matchSingleObject" and rarely extends.
//  (iv) textContainer - this matches things like texttags. It matches "matchPieceByPiece"; there will generally be structural
//        matching in the ancestor axis (that is, we'll need to ensure we're inside the proper text tag regardless of whatever else
//        we may be in). Generally can extend. Question: Do we need to differentiate block-level structures here? Unclear...
//   (v) textNodes - just text nodes, as the name implies. Match "matchPieceByPiece", and can generally extend matches.

//Each matchNode should have the functions:
//   (i) extendLeft(offset, targetNode, targetOffset)
//  (ii) extendRight(offset, targetNode, targetOffset)
// (iii) match(targetNode, targetOffset)
//  and what else?

function createMatchNode(theNode, theFlags, refEditor)
{
//  var theProto = null;
  var retVal = null;
  var nodeType = msiSearchUtils.getTypeOfNodeSearch(theNode, refEditor);
//  if (msiSearchUtils.isMathTemplate(theNode))
  switch(nodeType)
  {
    case "mathTemplate":  
//      theProto = msiTemplateMatchNode;
      if ( msiSearchUtils.isMRowLike(msiGetBaseNodeName(theNode)) )
        retVal = new msiContainerTemplateMatchNode();
      else
        retVal = new msiTemplateMatchNode();
    break;
//  if (msiSearchUtils.isMRowLike(theNode))
    case "mathContainer":
//      theProto = msiMathContainerMatchNode;
      retVal = new msiMathContainerMatchNode();
    break;
    case "textContainer":
//      theProto = msiTextContainerMatchNode;
      retVal = new msiTextContainerMatchNode();
    break;
    case "blockContainer":
//      theProto = msiBlockContainerMatchNode;
      retVal = new msiBlockContainerMatchNode();
    break;
    case "text":
//      theProto = msiTextMatchNode;
      retVal = new msiTextMatchNode();
    break;
    case "find":
//      theProto = msiMatchNode;
      retVal = new msiMatchNode();
    break;
    case "anonContainer":
      retVal = new msiAnonContainerMatchNode();
    break;
  }
//  if (theProto)
  if (retVal)
  {
//    var newFunc = function() {};
//    newFunc.prototype = new theProto();
//    retVal = new newFunc();
////    retVal.prototype = theProto;
    var logStr = "Creating a matchNode of type [" + nodeType + "] for node [" + theNode.nodeName + "].\n";
//    var logStr = "Creating a matchNode of type [" + nodeType + "]; it has members:\n";
    msiKludgeLogString(logStr, ["search"]);
    try
    {
      retVal.init(theNode, theFlags, refEditor);
//      logStr = "  init succeeded; retVal has [";
//      for (var aProp in retVal)
//      {
//        logStr += aProp + ",  ";
//      }
//      dump(logStr + "] properties.\n");
    } catch(exc)
    {
      msiKludgeLogString("Unable to init msiMatchNode due to exception: " + exc + "\n", ["search"]);
      logStr = "  Members of retVal are:\n";
      for (var aProp in retVal)
      {
        logStr += "  " + aProp + ":      [" + retVal[aProp] + "]\n";
      }
      msiKludgeLogString(logStr + "\n", ["search"]);
//      dump(logStr);
    }
  }
  else
    msiKludgeLogString( "Failed to create a matchNode, node of type [" + nodeType + "] for node [" + theNode.nodeName + "].\n", ["search"] );
  return retVal;
}

function msiMatchingRange() {}
msiMatchingRange.prototype =
{
  startContainer : null,
  startOffset : 0,
  endContainer : null,
  endOffset : 0,

  setStart : function(aNode, anOffset)
  {
    this.startContainer = aNode;
    this.startOffset = anOffset;
  },

  setEnd : function(aNode, anOffset)
  {
    this.endContainer = aNode;
    this.endOffset = anOffset;
  },

  setStartBefore : function(aNode)
  {
//    try
//    {
    var parent = aNode.parentNode;
    if (parent)
    {
      var parOffset = msiNavigationUtils.offsetInParent(aNode);
      this.setStart(parent, parOffset);
    }
    else
      this.setStart(aNode, 0);
//    } catch(exc) {dump("Exception in msiMatchingRange.setStartBefore [" + exc + "]; aNode is [" + aNode.nodeName + "].\n");}
  },

  setEndBefore : function(aNode)
  {
//    try
//    {
    var parent = aNode.parentNode;
    if (parent)
    {
      var parOffset = msiNavigationUtils.offsetInParent(aNode);
      this.setEnd(parent, parOffset);
    }
    else
      this.setEnd(aNode, 0);
//    } catch(exc) {dump("Exception in msiMatchingRange.setStartBefore [" + exc + "]; aNode is [" + aNode.nodeName + "].\n");}
  },

  setStartAfter : function(aNode)
  {
//    try
//    {
    var parent = aNode.parentNode;
    if (parent)
    {
      var parOffset = msiNavigationUtils.offsetInParent(aNode);
      this.setStart(parent, parOffset + 1);
    }
    else
      this.setStart(aNode, msiNavigationUtils.lastOffset(aNode));
//    } catch(exc) {dump("Exception in msiMatchingRange.setEndAfter [" + exc + "]; aNode is [" + aNode.nodeName + "].\n");}
  },

  setEndAfter : function(aNode)
  {
//    try
//    {
    var parent = aNode.parentNode;
    if (parent)
    {
      var parOffset = msiNavigationUtils.offsetInParent(aNode);
      this.setEnd(parent, parOffset + 1);
    }
    else
      this.setEnd(aNode, msiNavigationUtils.lastOffset(aNode));
//    } catch(exc) {dump("Exception in msiMatchingRange.setEndAfter [" + exc + "]; aNode is [" + aNode.nodeName + "].\n");}
  },

  selectNode : function(aNode)
  {
    this.setStartBefore(aNode);
    this.setEndAfter(aNode);
  },

  collapse : function(toStart)
  {
    if (toStart)
    {
      this.endContainer = this.startContainer;
      this.endOffset = this.startOffset;
    }
    else
    {
      this.startContainer = this.endContainer;
      this.startOffset = this.endOffset;
    }
  },

  collapsed : function()
  {
    return (this.startContainer == this.endContainer) && (this.startOffset == this.endOffset);
  },

  startIsInside : function(aNode)
  {
    for (var aTarg = this.startContainer; aTarg != null; aTarg = aTarg.parentNode)
    {
      if (aTarg == aNode)
        return true;
    }
    return false;
  },

  endIsInside : function(aNode)
  {
    for (var aTarg = this.endContainer; aTarg != null; aTarg = aTarg.parentNode)
    {
      if (aTarg == aNode)
        return true;
    }
    return false;
  },

  cloneRange : function()
  {
    var retVal = createMsiMatchingRange();
    retVal.setStart(this.startContainer, this.startOffset);
    retVal.setEnd(this.endContainer, this.endOffset);
    return retVal;
  },

  comparePoint : function(aNode, anOffset)
  {

    var startVal = msiNavigationUtils.comparePositions(aNode, anOffset, this.startContainer, this.startOffset);
    var endVal = msiNavigationUtils.comparePositions(aNode, anOffset, this.endContainer, this.endOffset);
    if (startVal < 0)
      return -1;  //aNode comes before our start
    if (endVal > 0)
      return 1;  //aNode comes after our end
    //Otherwise, do we assume that aNode is between the two????? We know startVal >= 0 and endVal <= 0, so that should be enough
    return 0;
  },

  compareBoundaryPoints : function(howToCompare, refRange)
  {
    switch(howToCompare)
    {
      case nsIDOMRange.END_TO_END:
        return msiNavigationUtils.comparePositions(this.endContainer, this.endOffset, this.endContainer, refRange.endOffset);
      break;
      case nsIDOMRange.END_TO_START:
        return msiNavigationUtils.comparePositions(this.endContainer, this.endOffset, this.startContainer, refRange.startOffset);
      break;
      case nsIDOMRange.START_TO_END:
        return msiNavigationUtils.comparePositions(this.startContainer, this.startOffset, this.endContainer, refRange.endOffset);
      break;
      case nsIDOMRange.START_TO_START:
      default:  //It should really be an error if "howToCompare" is anything else, but we'll just compare starts and have done with it.
        return msiNavigationUtils.comparePositions(this.startContainer, this.startOffset, this.startContainer, refRange.startOffset);
      break;
    }
  }
};

function createMsiMatchingRange()
{
  var msiRange = new msiMatchingRange();
//  msiRange.prototype = msiMatchingRange;
  return msiRange;
}

//We need to look into some of the following situations:
//  <math><msub><mrow><mi>a</mi><mo>+</mo><mi>b</mi></mrow><mn>3</mn></msub><mo>&invisTimes;</mo><mrow><mo fence="true">{</mo><mi>a</mi><mo>-</mo><mi>b</mi><mo fence="true">}</mo></mrow></math>
//  Suppose this is to be matched by <math><mi>a</mi><mo>-</mo><mi>b</mi></math>, and that the initial candidate node has been flagged as the first <mi>a</mi>.
//  How to write the code to check to the left and right to extend the match (and in this case determine that it doesn't match)?
//  What if instead we find a target in the numerator of an <mfrac> and want to decide whether it extends left and right?

//A further classification scheme (!??) seems to be called for. The question we really want to answer has to do with the "mode of
//  matching" of one node against another. The essentials MAY be distinguishable as:
//  (i) Can this node match against part of the target node? That is, must a match be made against the entire target node or should
//      we consider pieces of it? Example: a #text node may "always" match a piece (substring) of a target.
// (ii) Can a part of this node match against the target node?
//(iii) Can this node match against the target node at all?
//The idea is to use this classification to determine our responses. So we'd like to write "extendMatchLeft" in terms like:
//    if (this.canMatchPart(targetNode))
//      this.extendLeft(getNextNodeToLeft(targetNode, targetOffset); ??
//    else if (this.canPartiallyMatch(targetNode)
//      this.getNextPieceToLeft(ourOffset).extendMatchToLeft();
//  or:
//    switch(getMatchMode(targetNode))
//      case pieceByPiece:  this should mean parts of us can match parts of them
//      case canMatchPart:  this should mean all of us could match parts of them?? should this be different?
//      case pieceCanMatch:  this should mean a piece of us can match all of them??
//We want to go through a loop to extend a match. Each time we should try to move the left edge of the range in the target further
//  left; if we can't we have to get out. At each point, we want to examine the next object to the left (until we exit the current object!
//  at which point things have to pass to our parent); if the matching should be done by one of our children we should pass it on
//  to the child, otherwise try to match it ourselves.
//The code we're trying to write would look something like:
//    if (??)
//      then move match target into subobject and go through loop again?

//////A problem still not well handled is what a math template, for instance, should do when an extendLeft() reaches its end inside a
//////  child node and gets handed (via a controlling function) up to the math template. The key issue is to recognize what the real
//////  target node is - it's still essential that the wrong target not be handled by the math template. Is this somehow taken care of
//////  by the position of the selection edge as it comes in to the extendLeft call? For instance, it may be that we're an <mfrac> and
//////  the match started (and has thus far succeeded) inside of our numerator. In this case, we need several things to happen. First,
//////  the match position in the target should be at the left of the nodes that matched our numerator; does this allow us to check for a
//////  corresponding <mfrac> in the right place? The key here come from the "ourOffset" parameter. If "ourOffset" indicates (-1, or our
//////  complete childNodes.length) that none of our subobjects have been involved in the match, then we should look to the left in the
//////  target for a matching math template. If the offset is anything else, the previous matching must have occurred within one of our
//////  subobject "fields", and the key is to hopefully retain the information about which fields have matched, pass to the parent of the
//////  target node (in all cases????? check this) and check it for a match, then check the remaining unmatched fields.
//////So, generically? If an object is called upon to extend a match to the left, and the offset parameter indicates that the match includes
//////  already some subNodes of the object, then the object must look at the parent of the just-matched node. 
////
//////This function is intended to answer the question "Can matchNode match part of targNode?"
////function canMatchPart(matchNode, targNode)
////{
////  var retVal = false;
////  var matchName = msiGetBaseNodeName(matchNode);
////  var targName = msiGetBaseNodeName(targNode);
////  if (msiSearchUtils.isContainer(matchNode))
////  {
////    if (matchName == "msqrt")
////
////  }
////}

//////What we really need is a classificatino of nodes that's more like:
//////  canMatchPieceByPiece  - this would be <mrow> and implied mrow's(?)
//////  canMatchCorrespondingPieces - this would be math templates
//////  canMatchSingleObject and this would be what?
//////The function should be called with a matchNode and a targetNode. The big question is - how and when? For instance,
//////  matchNode of <mfrac> with targetNode of <mrow> would produce an answer of canMatchSingleObject, but we'd need to know
//////  for sure that the match being attempted is not, for instance, the <mfrac> against the contents of an <mfrac>'s denominator.
//////I think this will work if we're careful how we call the function. One scenario, for instance, would be that we've matched the
//////  numerator and denomiator of an <mfrac> and are moving out, getting the <mfrac> as the next node to try to extend. On the
//////  target side we may find that we've moved out of an <mrow> and the parent is again an <mrow>. Since this match shouldn't work,
//////  the next attempt will be to see whether we're at the left end of the <mrow> (using the msiNavigationUtils function), and exit
//////  to its parent again if we are. If we then encounter an <mfrac>, then we're presumably okay. ????
////function canExtendLeft(matchingParent, targetParent, theFlags, refEditor)
////{
////  var retVal = msiSearchUtils.matchPieceByPiece;
////  if (msiSearchUtils.isContainer(matchingParent))
////  {
////    if (!msiSearchUtils.isContainer(targetParent))
////    {
////      retVal = msiSearchUtils.cannotMatch;
////      //Except for?? May want to make an exception, for instance, for a/b matching the corresponding <mfrac>?
////    }
////  }
////  else if (msiSearchUtils.isMathTemplate(matchingParent))
////  {
//////    if (msiSearchUtils.isContainer(targetParent))
//////    {
//////      retVal = msiSearchUtils.canMatchOnePiece; ???
//////    }
//////    else
//////    {
////      retVal = msiSearchUtils.cannotMatch;  //really?
////      var matchInfo = XPathStringFormatterBase.getMatchingNodeInfo( msiGetBaseNodeName(matchingParent), theFlags);
////      var targetName = msiGetBaseNodeName(targetParent);
////      for (var ix = 0; ix < matchInfo.length; ++ix)
////      {
////        if (matchInfo[ix].theName == targetName)
////        {
////          retVal = msiSearchUtils.matchCorrespondingPieces;
////          break;
////        }
////      }
//////    }
////  else if (msiNavigationUtils.isMathMLLeafNode(matchingParent))
////  {
////  }
////  else
////  {
////    switch(msiGetBaseNodeName(matchingParent))  //??
////    {
////
////    }
////  }
////}
//////Need to start over!!!
