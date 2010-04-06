/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

const Ci = Components.interfaces;
const SHOW_ALL = Ci.nsIDOMNodeFilter.SHOW_ALL;

/**
 * @class Static utility class. Contains utilities used for displaying and
 *        searching a HTML tree.
 */
Firebug.HTMLLib =
{
    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // Node Search Utilities
    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    /**
     * Constructs a NodeSearch instance.
     *
     * @class Class used to search a DOM tree for the given text. Will display
     *        the search results in a IO Box.
     *
     * @constructor
     * @param {String} text Text to search for
     * @param {Object} root Root of search. This may be an element or a document
     * @param {Object} panelNode Panel node containing the IO Box representing the DOM tree.
     * @param {Object} ioBox IO Box to display the search results in
     * @param {Object} walker Optional walker parameter.
     */
    NodeSearch: function(text, root, panelNode, ioBox, walker)
    {
        root = root.documentElement || root;
        walker = walker || new Firebug.HTMLLib.DOMWalker(root);
        var re = new ReversibleRegExp(text, "m");
        var matchCount = 0;

        /**
         * Finds the first match within the document.
         *
         * @param {boolean} revert true to search backward, false to search forward
         * @param {boolean} caseSensitive true to match exact case, false to ignore case
         * @return true if no more matches were found, but matches were found previously.
         */
        this.find = function(reverse, caseSensitive)
        {
            var match = this.findNextMatch(reverse, caseSensitive);
            if (match)
            {
                this.lastMatch = match;
                ++matchCount;

                var node = match.node;
                var nodeBox = this.openToNode(node, match.isValue);

                this.selectMatched(nodeBox, node, match, reverse);
            }
            else if (matchCount)
                return true;
            else
            {
                this.noMatch = true;
                dispatch([Firebug.A11yModel], 'onHTMLSearchNoMatchFound', [panelNode.ownerPanel, text]);
            }
        };

        /**
         * Resets the search to the beginning of the document.
         */
        this.reset = function()
        {
            delete this.lastMatch;
        };

        /**
         * Finds the next match in the document.
         *
         * The return value is an object with the fields
         * - node: Node that contains the match
         * - isValue: true if the match is a match due to the value of the node, false if it is due to the name
         * - match: Regular expression result from the match
         *
         * @param {boolean} revert true to search backward, false to search forward
         * @param {boolean} caseSensitive true to match exact case, false to ignore case
         * @return Match object if found
         */
        this.findNextMatch = function(reverse, caseSensitive)
        {
            var innerMatch = this.findNextInnerMatch(reverse, caseSensitive);
            if (innerMatch)
                return innerMatch;
            else
                this.reset();

            function walkNode() { return reverse ? walker.previousNode() : walker.nextNode(); }

            var node;
            while (node = walkNode())
            {
                if (node.nodeType == Node.TEXT_NODE)
                {
                    if (Firebug.HTMLLib.isSourceElement(node.parentNode))
                        continue;
                }

                var m = this.checkNode(node, reverse, caseSensitive);
                if (m)
                    return m;
            }
        };

        /**
         * Helper util used to scan the current search result for more results
         * in the same object.
         *
         * @private
         */
        this.findNextInnerMatch = function(reverse, caseSensitive)
        {
            if (this.lastMatch)
            {
                var lastMatchNode = this.lastMatch.node;
                var lastReMatch = this.lastMatch.match;
                var m = re.exec(lastReMatch.input, reverse, lastReMatch.caseSensitive, lastReMatch);
                if (m)
                {
                    return {
                        node: lastMatchNode,
                        isValue: this.lastMatch.isValue,
                        match: m
                    };
                }

                // May need to check the pair for attributes
                if (lastMatchNode.nodeType == Node.ATTRIBUTE_NODE
                        && this.lastMatch.isValue == !!reverse)
                {
                    return this.checkNode(lastMatchNode, reverse, caseSensitive, 1);
                }
            }
        };

        /**
         * Checks a given node for a search match.
         *
         * @private
         */
        this.checkNode = function(node, reverse, caseSensitive, firstStep)
        {
            var checkOrder;
            if (node.nodeType != Node.TEXT_NODE)
            {
                var nameCheck = { name: "nodeName", isValue: false, caseSensitive: false };
                var valueCheck = { name: "nodeValue", isValue: true, caseSensitive: caseSensitive };
                checkOrder = reverse ? [ valueCheck, nameCheck ] : [ nameCheck, valueCheck ];
            }
            else
            {
                checkOrder = [{name: "nodeValue", isValue: false, caseSensitive: caseSensitive }];
            }

            for (var i = firstStep || 0; i < checkOrder.length; i++) {
                var m = re.exec(node[checkOrder[i].name], reverse, checkOrder[i].caseSensitive);
                if (m)
                    return {
                        node: node,
                        isValue: checkOrder[i].isValue,
                        match: m
                    };
            }
        };

        /**
         * Opens the given node in the associated IO Box.
         *
         * @private
         */
        this.openToNode = function(node, isValue)
        {
            if (node.nodeType == Node.ELEMENT_NODE)
            {
                var nodeBox = ioBox.openToObject(node);
                return nodeBox.getElementsByClassName("nodeTag")[0];
            }
            else if (node.nodeType == Node.ATTRIBUTE_NODE)
            {
                var nodeBox = ioBox.openToObject(node.ownerElement);
                if (nodeBox)
                {
                    var attrNodeBox = Firebug.HTMLLib.findNodeAttrBox(nodeBox, node.nodeName);
                    if (isValue)
                        return getChildByClass(attrNodeBox, "nodeValue");
                    else
                        return getChildByClass(attrNodeBox, "nodeName");
                }
            }
            else if (node.nodeType == Node.TEXT_NODE)
            {
                var nodeBox = ioBox.openToObject(node);
                if (nodeBox)
                    return nodeBox;
                else
                {
                    var nodeBox = ioBox.openToObject(node.parentNode);
                    if (hasClass(nodeBox, "textNodeBox"))
                        nodeBox = Firebug.HTMLLib.getTextElementTextBox(nodeBox);
                    return nodeBox;
                }
            }
        };

        /**
         * Selects the search results.
         *
         * @private
         */
        this.selectMatched = function(nodeBox, node, match, reverse)
        {
            setTimeout(bindFixed(function()
            {
                var reMatch = match.match;
                this.selectNodeText(nodeBox, node, reMatch[0], reMatch.index, reverse, reMatch.caseSensitive);
                dispatch([Firebug.A11yModel], 'onHTMLSearchMatchFound', [panelNode.ownerPanel, match]);
            }, this));
        };

        /**
         * Select text node search results.
         *
         * @private
         */
        this.selectNodeText = function(nodeBox, node, text, index, reverse, caseSensitive)
        {
            var row;

            // If we are still inside the same node as the last search, advance the range
            // to the next substring within that node
            if (nodeBox == this.lastNodeBox)
            {
                row = this.textSearch.findNext(false, true, reverse, caseSensitive);
            }

            if (!row)
            {
                // Search for the first instance of the string inside the node
                function findRow(node) { return node.nodeType == Node.ELEMENT_NODE ? node : node.parentNode; }
                this.textSearch = new TextSearch(nodeBox, findRow);
                row = this.textSearch.find(text, reverse, caseSensitive);
                this.lastNodeBox = nodeBox;
            }

            if (row)
            {
                var trueNodeBox = getAncestorByClass(nodeBox, "nodeBox");
                setClass(trueNodeBox,'search-selection');

                scrollIntoCenterView(row, panelNode);
                var sel = panelNode.ownerDocument.defaultView.getSelection(); 
                sel.removeAllRanges();
                sel.addRange(this.textSearch.range);

                removeClass(trueNodeBox,'search-selection'); 
                return true;
            }
        };
    },

    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    /**  XXXjjb this code is no longer called and won't be in 1.5; if FireFinder works out we can delete this.
     * Constructs a SelectorSearch instance.
     *
     * @class Class used to search a DOM tree for elements matching the given
     *        CSS selector.
     *
     * @constructor
     * @param {String} text CSS selector to search for
     * @param {Document} doc Document to search
     * @param {Object} panelNode Panel node containing the IO Box representing the DOM tree.
     * @param {Object} ioBox IO Box to display the search results in
     */
    SelectorSearch: function(text, doc, panelNode, ioBox)
    {
        this.parent = new Firebug.HTMLLib.NodeSearch(text, doc, panelNode, ioBox);

        /**
         * Finds the first match within the document.
         *
         * @param {boolean} revert true to search backward, false to search forward
         * @param {boolean} caseSensitive true to match exact case, false to ignore case
         * @return true if no more matches were found, but matches were found previously.
         */
        this.find = this.parent.find;

        /**
         * Resets the search to the beginning of the document.
         */
        this.reset = this.parent.reset;

        /**
         * Opens the given node in the associated IO Box.
         *
         * @private
         */
        this.openToNode = this.parent.openToNode;

        try
        {
            // http://dev.w3.org/2006/webapi/selectors-api/
            this.matchingNodes = doc.querySelectorAll(text);
            this.matchIndex = 0;
        }
        catch(exc)
        {
            FBTrace.sysout("SelectorSearch FAILS "+exc, exc);
        }

        /**
         * Finds the next match in the document.
         *
         * The return value is an object with the fields
         * - node: Node that contains the match
         * - isValue: true if the match is a match due to the value of the node, false if it is due to the name
         * - match: Regular expression result from the match
         *
         * @param {boolean} revert true to search backward, false to search forward
         * @param {boolean} caseSensitive true to match exact case, false to ignore case
         * @return Match object if found
         */
        this.findNextMatch = function(reverse, caseSensitive)
        {
            if (!this.matchingNodes || !this.matchingNodes.length)
                return undefined;

            if (reverse)
            {
                if (this.matchIndex > 0)
                    return { node: this.matchingNodes[this.matchIndex--], isValue: false, match: "?XX?"};
                else
                    return undefined;
            }
            else
            {
                if (this.matchIndex < this.matchingNodes.length)
                    return { node: this.matchingNodes[this.matchIndex++], isValue: false, match: "?XX?"};
                else
                    return undefined;
            }
        };

        /**
         * Selects the search results.
         *
         * @private
         */
        this.selectMatched = function(nodeBox, node, match, reverse)
        {
            setTimeout(bindFixed(function()
            {
                ioBox.select(node, true, true);
                dispatch([Firebug.A11yModel], 'onHTMLSearchMatchFound', [panelNode.ownerPanel, match]);
            }, this));
        };
    },


    /**
     * Constructs a DOMWalker instance.
     *
     * @constructor
     * @class Implements an ordered traveral of the document, including attributes and
     *        iframe contents within the results.
     *
     *        Note that the order for attributes is not defined. This will follow the
     *        same order as the Element.attributes accessor.
     * @param {Element} root Element to traverse
     */
    DOMWalker: function(root)
    {
        var walker;
        var currentNode, attrIndex;
        var pastStart, pastEnd;
        var doc = root.ownerDocument;

        function createWalker(docElement) {
            var walk = doc.createTreeWalker(docElement, SHOW_ALL, null, true);
            walker.unshift(walk);
        }
        function getLastAncestor() {
            while (walker[0].lastChild()) {}
            return walker[0].currentNode;
        }

        /**
         * Move to the previous node.
         *
         * @return The previous node if one exists, undefined otherwise.
         */
        this.previousNode = function() {
            if (pastStart) {
                return undefined;
            }

            if (attrIndex) {
                attrIndex--;
            } else {
                var prevNode;
                if (currentNode == walker[0].root) {
                    if (walker.length > 1) {
                        walker.shift();
                        prevNode = walker[0].currentNode;
                    } else {
                        prevNode = undefined;
                    }
                } else {
                    if (!currentNode) {
                        prevNode = getLastAncestor();
                    } else {
                        prevNode = walker[0].previousNode();
                    }
                    if (!prevNode) {    // Really shouldn't occur, but to be safe
                        prevNode = walker[0].root;
                    }
                    while ((prevNode.nodeName || "").toUpperCase() == "IFRAME") {
                        createWalker(prevNode.contentDocument.documentElement);
                        prevNode = getLastAncestor();
                    }
                }
                currentNode = prevNode;
                attrIndex = ((prevNode || {}).attributes || []).length;
            }

            if (!currentNode) {
                pastStart = true;
            } else {
                pastEnd = false;
            }

            return this.currentNode();
        };

        /**
         * Move to the next node.
         *
         * @return The next node if one exists, otherwise undefined.
         */
        this.nextNode = function() {
            if (pastEnd) {
                return undefined;
            }

            if (!currentNode) {
                // We are working with a new tree walker
                currentNode = walker[0].root;
                attrIndex = 0;
            } else {
                // First check attributes
                var attrs = currentNode.attributes || [];
                if (attrIndex < attrs.length) {
                    attrIndex++;
                } else if ((currentNode.nodeName || "").toUpperCase() == "IFRAME") {
                    // Attributes have completed, check for iframe contents
                    createWalker(currentNode.contentDocument.documentElement);
                    currentNode = walker[0].root;
                    attrIndex = 0;
                } else {
                    // Next node
                    var nextNode = walker[0].nextNode();
                    while (!nextNode && walker.length > 1) {
                        walker.shift();
                        nextNode = walker[0].nextNode();
                    }
                    currentNode = nextNode;
                    attrIndex = 0;
                }
            }

            if (!currentNode) {
                pastEnd = true;
            } else {
                pastStart = false;
            }

            return this.currentNode();
        };

        /**
         * Retrieves the current node.
         *
         * @return The current node, if not past the beginning or end of the iteration.
         */
        this.currentNode = function() {
            if (!attrIndex) {
                return currentNode;
            } else {
                return currentNode.attributes[attrIndex-1];
            }
        };

        /**
         * Resets the walker position back to the initial position.
         */
        this.reset = function() {
            pastStart = false;
            pastEnd = false;
            walker = [];
            currentNode = undefined;
            attrIndex = 0;

            createWalker(root);
        };

        this.reset();
    },

    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // Node/Element Utilities
    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    /**
     * Determines if the given element is the source for a non-DOM resource such
     * as Javascript source or CSS definition.
     *
     * @param {Element} element Element to test
     * @return true if the element is a source element
     */
    isSourceElement: function(element)
    {
        var tag = element.localName.toLowerCase();
        return tag == "script" || tag == "link" || tag == "style"
            || (tag == "link" && element.getAttribute("rel") == "stylesheet");
    },

    /**
     * Retrieves the source URL for any external resource associated with a node.
     *
     * @param {Element} element Element to examine
     * @return URL of the external resouce.
     */
    getSourceHref: function(element)
    {
        var tag = element.localName.toLowerCase();
        if (tag == "script" && element.src)
            return element.src;
        else if (tag == "link")
            return element.href;
        else
            return null;
    },

    /**
     * Retrieves the source text for inline script and style elements.
     *
     * @param {Element} element Script or style element
     * @return Source text
     */
    getSourceText: function(element)
    {
        var tag = element.localName.toLowerCase();
        if (tag == "script" && !element.src)
            return element.textContent;
        else if (tag == "style")
            return element.textContent;
        else
            return null;
    },

    /**
     * Determines if the given element is a container element.
     *
     * @param {Element} element Element to test
     * @return True if the element is a container element.
     */
    isContainerElement: function(element)
    {
        var tag = element.localName.toLowerCase();
        switch (tag)
        {
            case "script":
            case "style":
            case "iframe":
            case "frame":
            case "tabbrowser":
            case "browser":
                return true;
            case "link":
                return element.getAttribute("rel") == "stylesheet";
            case "embed":
                return element.getSVGDocument();
        }
        return false;
    },

    /**
     * Determines if the given node has any children which are elements.
     *
     * @param {Element} element Element to test.
     * @return true if immediate children of type Element exist, false otherwise
     */
    hasNoElementChildren: function(element)
    {
        if (element.childElementCount != 0)  // FF 3.5+
            return false;

        // https://developer.mozilla.org/en/XBL/XBL_1.0_Reference/DOM_Interfaces
        if (element.ownerDocument instanceof Ci.nsIDOMDocumentXBL)
        {
            var anonChildren = element.ownerDocument.getAnonymousNodes(element);
            if (anonChildren)
            {
                for (var i = 0; i < anonChildren.length; i++)
                {
                    if (anonChildren[i].nodeType == Node.ELEMENT_NODE)
                        return false;
                }
            }
        }
        return true;
    },
    
    
    /**
     * Determines if the given node has any children which are comments.
     *
     * @param {Element} element Element to test.
     * @return true if immediate children of type Comment exist, false otherwise
     */
    hasCommentChildren: function(element)
    {
        if (element.hasChildNodes())
        {
            var children = element.childNodes;
            for (var i = 0; i < children.length; i++) 
            {
              if (children[i] instanceof Comment)
                 return true;
            }
        };
        return false;
    },


    /**
     * Determines if the given node consists solely of whitespace text.
     *
     * @param {Node} node Node to test.
     * @return true if the node is a whitespace text node
     */
    isWhitespaceText: function(node)
    {
        if (node instanceof HTMLAppletElement)
            return false;
        return node.nodeType == Node.TEXT_NODE && isWhitespace(node.nodeValue);
    },

    /**
     * Determines if a given element is empty. When the
     * {@link Firebug#showTextNodesWithWhitespace} parameter is true, an element is
     * considered empty if it has no child elements and is self closing. When
     * false, an element is considered empty if the only children are whitespace
     * nodes.
     *
     * @param {Element} element Element to test
     * @return true if the element is empty, false otherwise
     */
    isEmptyElement: function(element)
    {
        // XXXjjb the commented code causes issues 48, 240, and 244. I think the lines should be deleted.
        // If the DOM has whitespace children, then the element is not empty even if
        // we decide not to show the whitespace in the UI.

        // XXXsroussey reverted above but added a check for self closing tags
        if (Firebug.showTextNodesWithWhitespace)
        {
            return !element.firstChild && isSelfClosing(element);
        }
        else
        {
            for (var child = element.firstChild; child; child = child.nextSibling)
            {
                if (!Firebug.HTMLLib.isWhitespaceText(child))
                    return false;
            }
        }
        return isSelfClosing(element);
    },

    /**
     * Finds the next sibling of the given node. If the
     * {@link Firebug#showTextNodesWithWhitespace} parameter is set to true, the next
     * sibling may be a whitespace, otherwise the next is the first adjacent
     * non-whitespace node.
     *
     * @param {Node} node Node to analyze.
     * @return Next sibling node, if one exists
     */
    findNextSibling: function(node)
    {
        if (Firebug.showTextNodesWithWhitespace)
            return node.nextSibling;
        else
        {
            // only return a non-whitespace node
            for (var child = node.nextSibling; child; child = child.nextSibling)
            {
                if (!Firebug.HTMLLib.isWhitespaceText(child))
                    return child;
            }
        }
    },

    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // Domplate Utilities
    //* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    /**
     * Locates the attribute domplate node for a given element domplate. This method will
     * only examine notes marked with the "nodeAttr" class that are the direct
     * children of the given element.
     *
     * @param {Object} objectNodeBox The domplate element to look up the attribute for.
     * @param {String} attrName Attribute name
     * @return Attribute's domplate node
     */
    findNodeAttrBox: function(objectNodeBox, attrName)
    {
        var child = objectNodeBox.firstChild.lastChild.firstChild;
        for (; child; child = child.nextSibling)
        {
            if (hasClass(child, "nodeAttr") && child.childNodes[1].firstChild
                && child.childNodes[1].firstChild.nodeValue == attrName)
            {
                return child;
            }
        }
    },

    /**
     * Locates the text domplate node for a given text element domplate.
     * @param {Object} nodeBox Text element domplate
     * @return Element's domplate text node
     */
    getTextElementTextBox: function(nodeBox)
    {
        var nodeLabelBox = nodeBox.firstChild.lastChild;
        return getChildByClass(nodeLabelBox, "nodeText");
    }
};

}});
