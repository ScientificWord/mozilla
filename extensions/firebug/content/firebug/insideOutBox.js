/* See license.txt for terms of usage */

FBL.ns(function() { with (FBL) {

/**
 * View interface used to populate an InsideOutBox object.
 *
 * All views must implement this interface (directly or via duck typing).
 */
top.InsideOutBoxView = {
    /**
     * Retrieves the parent object for a given child object.
     */
    getParentObject: function(child) {},

    /**
     * Retrieves a given child node.
     *
     * If both index and previousSibling are passed, the implementation
     * may assume that previousSibling will be the return for getChildObject
     * with index-1.
     */
    getChildObject: function(parent, index, previousSibling) {},

    /**
     * Renders the HTML representation of the object. Should return an HTML
     * object which will be displayed to the user.
     */
    createObjectBox: function(object, isRoot) {}
};

/**
 * Creates a tree based on objects provided by a separate "view" object.
 *
 * Construction uses an "inside-out" algorithm, meaning that the view's job is first
 * to tell us the ancestry of each object, and secondarily its descendants.
 */
top.InsideOutBox = function(view, box)
{
    this.view = view;
    this.box = box;

    this.rootObject = null;

    this.rootObjectBox = null;
    this.selectedObjectBox = null;
    this.highlightedObjectBox = null;

    this.onMouseDown = bind(this.onMouseDown, this);
    box.addEventListener("mousedown", this.onMouseDown, false);
};

InsideOutBox.prototype =
{
    destroy: function()
    {
        this.box.removeEventListener("mousedown", this.onMouseDown, false);
    },

    highlight: function(object)
    {
        var objectBox = this.createObjectBox(object);
        this.highlightObjectBox(objectBox);
        return objectBox;
    },

    openObject: function(object)
    {
        var firstChild = this.view.getChildObject(object, 0);
        if (firstChild)
            object = firstChild;

        var objectBox = this.createObjectBox(object);
        this.openObjectBox(objectBox);
        return objectBox;
    },

    openToObject: function(object)
    {
        var objectBox = this.createObjectBox(object);
        this.openObjectBox(objectBox);
        return objectBox;
    },

    select: function(object, makeBoxVisible, forceOpen, noScrollIntoView)
    {
        var objectBox = this.createObjectBox(object);
        this.selectObjectBox(objectBox, forceOpen);
        if (makeBoxVisible)
        {
            this.openObjectBox(objectBox);
            if (!noScrollIntoView)
                scrollIntoCenterView(objectBox);
        }
        return objectBox;
    },

    expandObject: function(object)
    {
        var objectBox = this.createObjectBox(object);
        if (objectBox)
            this.expandObjectBox(objectBox);
    },

    contractObject: function(object)
    {
        var objectBox = this.createObjectBox(object);
        if (objectBox)
            this.contractObjectBox(objectBox);
    },

    highlightObjectBox: function(objectBox)
    {
        if (this.highlightedObjectBox)
        {
            removeClass(this.highlightedObjectBox, "highlighted");

            var highlightedBox = this.getParentObjectBox(this.highlightedObjectBox);
            for (; highlightedBox; highlightedBox = this.getParentObjectBox(highlightedBox))
                removeClass(highlightedBox, "highlightOpen");
        }

        this.highlightedObjectBox = objectBox;

        if (objectBox)
        {
            setClass(objectBox, "highlighted");

            var highlightedBox = this.getParentObjectBox(objectBox);
            for (; highlightedBox; highlightedBox = this.getParentObjectBox(highlightedBox))
                setClass(highlightedBox, "highlightOpen");

           scrollIntoCenterView(objectBox);
        }
    },

    selectObjectBox: function(objectBox, forceOpen)
    {
        var isSelected = this.selectedObjectBox && objectBox == this.selectedObjectBox;
        if (!isSelected)
        {
            removeClass(this.selectedObjectBox, "selected");
            dispatch([Firebug.A11yModel], 'onObjectBoxUnselected', [this.selectedObjectBox]);
            this.selectedObjectBox = objectBox;

            if (objectBox)
            {
                setClass(objectBox, "selected");

                // Force it open the first time it is selected
                if (forceOpen)
                    this.toggleObjectBox(objectBox, true);
            }
        }
        dispatch([Firebug.A11yModel], 'onObjectBoxSelected', [objectBox]);
    },

    openObjectBox: function(objectBox)
    {
        if (objectBox)
        {
            // Set all of the node's ancestors to be permanently open
            var parentBox = this.getParentObjectBox(objectBox);
            var labelBox;
            for (; parentBox; parentBox = this.getParentObjectBox(parentBox))
            {
                setClass(parentBox, "open");
                labelBox = parentBox.getElementsByClassName('nodeLabelBox').item(0);
                if (labelBox)
                    labelBox.setAttribute('aria-expanded', 'true')
            }
        }
    },

    expandObjectBox: function(objectBox)
    {
        var nodeChildBox = this.getChildObjectBox(objectBox);
        if (!nodeChildBox)
            return;

        if (!objectBox.populated)
        {
            var firstChild = this.view.getChildObject(objectBox.repObject, 0);
            this.populateChildBox(firstChild, nodeChildBox);
        }
        var labelBox = objectBox.getElementsByClassName('nodeLabelBox').item(0);
        if (labelBox)
            labelBox.setAttribute('aria-expanded', 'true');
        setClass(objectBox, "open");
    },

    contractObjectBox: function(objectBox)
    {
        removeClass(objectBox, "open");
        var nodeLabel = objectBox.getElementsByClassName("nodeLabel").item(0);
        var labelBox = nodeLabel.getElementsByClassName('nodeLabelBox').item(0);
        if (labelBox)
            labelBox.setAttribute('aria-expanded', 'false');
    },

    toggleObjectBox: function(objectBox, forceOpen)
    {
        var isOpen = hasClass(objectBox, "open");
        var nodeLabel = objectBox.getElementsByClassName("nodeLabel").item(0);
        var labelBox = nodeLabel.getElementsByClassName('nodeLabelBox').item(0);
        if (labelBox)
            labelBox.setAttribute('aria-expanded', isOpen);
        if (!forceOpen && isOpen)
            this.contractObjectBox(objectBox);

        else if (!isOpen)
            this.expandObjectBox(objectBox);
    },

    getNextObjectBox: function(objectBox)
    {
        return findNext(objectBox, isVisibleTarget, false, this.box);
    },

    getPreviousObjectBox: function(objectBox)
    {
        return findPrevious(objectBox, isVisibleTarget, true, this.box);
    },

    /**
     * Creates all of the boxes for an object, its ancestors, and siblings.
     */
    createObjectBox: function(object)
    {
        if (!object)
            return null;

        this.rootObject = this.getRootNode(object);

        // Get or create all of the boxes for the target and its ancestors
        var objectBox = this.createObjectBoxes(object, this.rootObject);

        if (!objectBox)
            return null;
        else if (object == this.rootObject)
            return objectBox;
        else
            return this.populateChildBox(object, objectBox.parentNode);
    },

    /**
     * Creates all of the boxes for an object, its ancestors, and siblings up to a root.
     */
    createObjectBoxes: function(object, rootObject)
    {
        if (!object)
            return null;

        if (object == rootObject)
        {
            if (!this.rootObjectBox || this.rootObjectBox.repObject != rootObject)
            {
                if (this.rootObjectBox)
                {
                    try {
                        this.box.removeChild(this.rootObjectBox);
                    } catch (exc) {
                    }
                }

                this.highlightedObjectBox = null;
                this.selectedObjectBox = null;
                this.rootObjectBox = this.view.createObjectBox(object, true);
                this.box.appendChild(this.rootObjectBox);
            }
            return this.rootObjectBox;
        }
        else
        {
            var parentNode = this.view.getParentObject(object);

            var parentObjectBox = this.createObjectBoxes(parentNode, rootObject);
            if (!parentObjectBox)
                return null;

            var parentChildBox = this.getChildObjectBox(parentObjectBox);
            if (!parentChildBox)
                return null;

            var childObjectBox = this.findChildObjectBox(parentChildBox, object);
            return childObjectBox
                ? childObjectBox
                : this.populateChildBox(object, parentChildBox);
        }
    },

    findObjectBox: function(object)
    {
        if (!object)
            return null;

        if (object == this.rootObject)
            return this.rootObjectBox;
        else
        {
            var parentNode = this.view.getParentObject(object);
            var parentObjectBox = this.findObjectBox(parentNode);
            if (!parentObjectBox)
                return null;

            var parentChildBox = this.getChildObjectBox(parentObjectBox);
            if (!parentChildBox)
                return null;

            return this.findChildObjectBox(parentChildBox, object);
        }
    },

    appendChildBox: function(parentNodeBox, repObject)
    {
        var childBox = this.getChildObjectBox(parentNodeBox);
        var objectBox = this.findChildObjectBox(childBox, repObject);
        if (objectBox)
            return objectBox;

        objectBox = this.view.createObjectBox(repObject);
        if (objectBox)
        {
            var childBox = this.getChildObjectBox(parentNodeBox);
            childBox.appendChild(objectBox);
        }
        return objectBox;
    },

    insertChildBoxBefore: function(parentNodeBox, repObject, nextSibling)
    {
        var childBox = this.getChildObjectBox(parentNodeBox);
        var objectBox = this.findChildObjectBox(childBox, repObject);
        if (objectBox)
            return objectBox;

        objectBox = this.view.createObjectBox(repObject);
        if (objectBox)
        {
            var siblingBox = this.findChildObjectBox(childBox, nextSibling);
            childBox.insertBefore(objectBox, siblingBox);
        }
        return objectBox;
    },

    removeChildBox: function(parentNodeBox, repObject)
    {
        var childBox = this.getChildObjectBox(parentNodeBox);
        var objectBox = this.findChildObjectBox(childBox, repObject);
        if (objectBox)
            childBox.removeChild(objectBox);
    },

    populateChildBox: function(repObject, nodeChildBox)  // We want all children of the parent of repObject.
    {
        if (!repObject)
            return null;

        var parentObjectBox = getAncestorByClass(nodeChildBox, "nodeBox");
        if (parentObjectBox.populated)
            return this.findChildObjectBox(nodeChildBox, repObject);

        var lastSiblingBox = this.getChildObjectBox(nodeChildBox);
        var siblingBox = nodeChildBox.firstChild;
        var targetBox = null;

        var view = this.view;

        var targetSibling = null;
        var parentNode = view.getParentObject(repObject);
        for (var i = 0; 1; ++i)
        {
            targetSibling = view.getChildObject(parentNode, i, targetSibling);
            if (!targetSibling)
                break;

            // Check if we need to start appending, or continue to insert before
            if (lastSiblingBox && lastSiblingBox.repObject == targetSibling)
                lastSiblingBox = null;

            if (!siblingBox || siblingBox.repObject != targetSibling)
            {
                var newBox = view.createObjectBox(targetSibling);
                if (newBox)
                {
                    if (lastSiblingBox)
                        nodeChildBox.insertBefore(newBox, lastSiblingBox);
                    else
                        nodeChildBox.appendChild(newBox);
                }

                siblingBox = newBox;
            }

            if (targetSibling == repObject)
                targetBox = siblingBox;

            if (siblingBox && siblingBox.repObject == targetSibling)
                siblingBox = siblingBox.nextSibling;
        }

        if (targetBox)
            parentObjectBox.populated = true;
        return targetBox;
    },

    getParentObjectBox: function(objectBox)
    {
        var parent = objectBox.parentNode ? objectBox.parentNode.parentNode : null;
        return parent && parent.repObject ? parent : null;
    },

    getChildObjectBox: function(objectBox)
    {
        return objectBox.getElementsByClassName("nodeChildBox").item(0);
    },

    findChildObjectBox: function(parentNodeBox, repObject)
    {
        for (var childBox = parentNodeBox.firstChild; childBox; childBox = childBox.nextSibling)
        {
            if (childBox.repObject == repObject)
                return childBox;
        }
    },

    /**
     * Determines if the given node is an ancestor of the current root.
     */
    isInExistingRoot: function(node)
    {
        var parentNode = node;
        while (parentNode && parentNode != this.rootObject)
        {
            var parentNode = this.view.getParentObject(parentNode);
        }
        return parentNode == this.rootObject;
    },

    getRootNode: function(node)
    {
        while (1)
        {
            var parentNode = this.view.getParentObject(node);
            if (!parentNode)
                return node;
            else
                node = parentNode;
        }
        return null;
    },

    // ********************************************************************************************

    onMouseDown: function(event)
    {
        var hitTwisty = false;
        for (var child = event.target; child; child = child.parentNode)
        {
            if (hasClass(child, "twisty"))
                hitTwisty = true;
            else if (child.repObject)
            {
                if (hitTwisty)
                    this.toggleObjectBox(child);
                break;
            }
        }
    }
};

// ************************************************************************************************
// Local Helpers

function isVisibleTarget(node)
{
    if (node.repObject && node.repObject.nodeType == Node.ELEMENT_NODE)
    {
        for (var parent = node.parentNode; parent; parent = parent.parentNode)
        {
            if (hasClass(parent, "nodeChildBox")
                && !hasClass(parent.parentNode, "open")
                && !hasClass(parent.parentNode, "highlightOpen"))
                return false;
        }
        return true;
    }
}

function formatNode(object)
{
    if (object)
        return (object.localName ? object.localName : object);
    else
        return "(null object)";
}

function getObjectPath(element, aView)
{
    var path = [];
    for (; element; element = aView.getParentObject(element))
        path.push(element);

    return path;
}

}});
