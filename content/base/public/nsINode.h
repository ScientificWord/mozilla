/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla.org code.
 *
 * The Initial Developer of the Original Code is Mozilla.com.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *         Boris Zbarsky <bzbarsky@mit.edu> (Original Author)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsINode_h___
#define nsINode_h___

#include "nsPIDOMEventTarget.h"
#include "nsEvent.h"
#include "nsPropertyTable.h"
#include "nsTObserverArray.h"
#include "nsINodeInfo.h"
#include "nsCOMPtr.h"

class nsIContent;
class nsIDocument;
class nsIDOMEvent;
class nsIPresShell;
class nsPresContext;
class nsEventChainVisitor;
class nsEventChainPreVisitor;
class nsEventChainPostVisitor;
class nsIEventListenerManager;
class nsIPrincipal;
class nsVoidArray;
class nsIMutationObserver;
class nsChildContentList;
class nsNodeWeakReference;
class nsNodeSupportsWeakRefTearoff;
class nsIEditor;

enum {
  // This bit will be set if the node doesn't have nsSlots
  NODE_DOESNT_HAVE_SLOTS =       0x00000001U,

  // This bit will be set if the node has a listener manager in the listener
  // manager hash
  NODE_HAS_LISTENERMANAGER =     0x00000002U,

  // Whether this node has had any properties set on it
  NODE_HAS_PROPERTIES =          0x00000004U,

  // Whether this node is anonymous
  // NOTE: Should only be used on nsIContent nodes
  NODE_IS_ANONYMOUS =            0x00000008U,

  // Whether this node may have a frame
  // NOTE: Should only be used on nsIContent nodes
  NODE_MAY_HAVE_FRAME =          0x00000010U,

  // Forces the XBL code to treat this node as if it were
  // in the document and therefore should get bindings attached.
  NODE_FORCE_XBL_BINDINGS =      0x00000020U,

  // Whether a binding manager may have a pointer to this
  NODE_MAY_BE_IN_BINDING_MNGR =  0x00000040U,

  NODE_IS_EDITABLE =             0x00000080U,

  // Optimizations to quickly check whether element may have ID, class or style
  // attributes. Not all element implementations may use these!
  NODE_MAY_HAVE_ID =             0x00000100U,
  NODE_MAY_HAVE_CLASS =          0x00000200U,
  NODE_MAY_HAVE_STYLE =          0x00000400U,

  NODE_IS_INSERTION_PARENT =     0x00000800U,

  // Node has an :empty or :-moz-only-whitespace selector
  NODE_HAS_EMPTY_SELECTOR =      0x00001000U,

  // A child of the node has a selector such that any insertion,
  // removal, or appending of children requires restyling the parent.
  NODE_HAS_SLOW_SELECTOR =       0x00002000U,

  // A child of the node has a :first-child, :-moz-first-node,
  // :only-child, :last-child or :-moz-last-node selector.
  NODE_HAS_EDGE_CHILD_SELECTOR = 0x00004000U,

  // A child of the node has a selector such that any insertion or
  // removal of children requires restyling the parent (but append is
  // OK).
  NODE_HAS_SLOW_SELECTOR_NOAPPEND
                               = 0x00008000U,

  NODE_ALL_SELECTOR_FLAGS =      NODE_HAS_EMPTY_SELECTOR |
                                 NODE_HAS_SLOW_SELECTOR |
                                 NODE_HAS_EDGE_CHILD_SELECTOR |
                                 NODE_HAS_SLOW_SELECTOR_NOAPPEND,

  // Four bits for the script-type ID
  NODE_SCRIPT_TYPE_OFFSET =               16,

  NODE_SCRIPT_TYPE_SIZE =                  4,

  // Remaining bits are node type specific.
  NODE_TYPE_SPECIFIC_BITS_OFFSET =
    NODE_SCRIPT_TYPE_OFFSET + NODE_SCRIPT_TYPE_SIZE
};

// Useful inline function for getting a node given an nsIContent and an
// nsIDocument.  Returns the first argument cast to nsINode if it is non-null,
// otherwise returns the second (which may be null).  We use type variables
// instead of nsIContent* and nsIDocument* because the actual types must be
// known for the cast to work.
template<class C, class D>
inline nsINode* NODE_FROM(C& aContent, D& aDocument)
{
  if (aContent)
    return static_cast<nsINode*>(aContent);
  return static_cast<nsINode*>(aDocument);
}


// IID for the nsINode interface
#define NS_INODE_IID \
{ 0x6f69dd90, 0x318d, 0x40ac, \
  { 0xb8, 0xb8, 0x99, 0xb8, 0xa7, 0xbb, 0x9a, 0x58 } }

/**
 * An internal interface that abstracts some DOMNode-related parts that both
 * nsIContent and nsIDocument share.  An instance of this interface has a list
 * of nsIContent children and provides access to them.
 */
class nsINode : public nsPIDOMEventTarget {
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_INODE_IID)

  friend class nsNodeUtils;
  friend class nsNodeWeakReference;
  friend class nsNodeSupportsWeakRefTearoff;

#ifdef MOZILLA_INTERNAL_API
  nsINode(nsINodeInfo* aNodeInfo)
    : mNodeInfo(aNodeInfo),
      mParentPtrBits(0),
      mFlagsOrSlots(NODE_DOESNT_HAVE_SLOTS)
  {
  }
#endif

  virtual ~nsINode();

  /**
   * Bit-flags to pass (or'ed together) to IsNodeOfType()
   */
  enum {
    /** nsIContent nodes */
    eCONTENT             = 1 << 0,
    /** nsIDocument nodes */
    eDOCUMENT            = 1 << 1,
    /** nsIAttribute nodes */
    eATTRIBUTE           = 1 << 2,
    /** elements */
    eELEMENT             = 1 << 3,
    /** text nodes */
    eTEXT                = 1 << 4,
    /** xml processing instructions */
    ePROCESSING_INSTRUCTION = 1 << 5,
    /** comment nodes */
    eCOMMENT             = 1 << 6,
    /** html elements */
    eHTML                = 1 << 7,
    /** form control elements */
    eHTML_FORM_CONTROL   = 1 << 8,
    /** XUL elements */
    eXUL                 = 1 << 9,
    /** svg elements */
    eSVG                 = 1 << 10,
    /** document fragments */
    eDOCUMENT_FRAGMENT   = 1 << 11,
    /** data nodes (comments, PIs, text). Nodes of this type always
     returns a non-null value for nsIContent::GetText() */
    eDATA_NODE           = 1 << 12,
    /** nsMathMLElement */
    eMATHML              = 1 << 13
  };

  /**
   * API for doing a quick check if a content is of a given
   * type, such as HTML, XUL, Text, ...  Use this when you can instead of
   * checking the tag.
   *
   * @param aFlags what types you want to test for (see above)
   * @return whether the content matches ALL flags passed in
   */
  virtual PRBool IsNodeOfType(PRUint32 aFlags) const = 0;

  /**
   * Get the number of children
   * @return the number of children
   */
  virtual PRUint32 GetChildCount() const = 0;

  /**
   * Get a child by index
   * @param aIndex the index of the child to get
   * @return the child, or null if index out of bounds
   */
  virtual nsIContent* GetChildAt(PRUint32 aIndex) const = 0;

  /**
   * Get the index of a child within this content
   * @param aPossibleChild the child to get the index of.
   * @return the index of the child, or -1 if not a child
   *
   * If the return value is not -1, then calling GetChildAt() with that value
   * will return aPossibleChild.
   */
  virtual PRInt32 IndexOf(nsINode* aPossibleChild) const = 0;

  /**
   * Return the "owner document" of this node.  Note that this is not the same
   * as the DOM ownerDocument -- that's null for Document nodes, whereas for a
   * nsIDocument GetOwnerDocument returns the document itself.  For nsIContent
   * implementations the two are the same.
   */
  nsIDocument *GetOwnerDoc() const
  {
    return mNodeInfo->GetDocument();
  }

  /**
   * Returns true if the content has an ancestor that is a document.
   *
   * @return whether this content is in a document tree
   */
  PRBool IsInDoc() const
  {
    return mParentPtrBits & PARENT_BIT_INDOCUMENT;
  }

  /**
   * Get the document that this content is currently in, if any. This will be
   * null if the content has no ancestor that is a document.
   *
   * @return the current document
   */
  nsIDocument *GetCurrentDoc() const
  {
    return IsInDoc() ? GetOwnerDoc() : nsnull;
  }

  /**
   * Insert a content node at a particular index.  This method handles calling
   * BindToTree on the child appropriately.
   *
   * @param aKid the content to insert
   * @param aIndex the index it is being inserted at (the index it will have
   *        after it is inserted)
   * @param aNotify whether to notify the document (current document for
   *        nsIContent, and |this| for nsIDocument) that the insert has
   *        occurred
   *
   * @throws NS_ERROR_DOM_HIERARCHY_REQUEST_ERR if one attempts to have more
   * than one element node as a child of a document.  Doing this will also
   * assert -- you shouldn't be doing it!  Check with
   * nsIDocument::GetRootContent() first if you're not sure.  Apart from this
   * one constraint, this doesn't do any checking on whether aKid is a valid
   * child of |this|.
   *
   * @throws NS_ERROR_OUT_OF_MEMORY in some cases (from BindToTree).
   */
  virtual nsresult InsertChildAt(nsIContent* aKid, PRUint32 aIndex,
                                 PRBool aNotify) = 0;

  /**
   * Append a content node to the end of the child list.  This method handles
   * calling BindToTree on the child appropriately.
   *
   * @param aKid the content to append
   * @param aNotify whether to notify the document (current document for
   *        nsIContent, and |this| for nsIDocument) that the append has
   *        occurred
   *
   * @throws NS_ERROR_DOM_HIERARCHY_REQUEST_ERR if one attempts to have more
   * than one element node as a child of a document.  Doing this will also
   * assert -- you shouldn't be doing it!  Check with
   * nsIDocument::GetRootContent() first if you're not sure.  Apart from this
   * one constraint, this doesn't do any checking on whether aKid is a valid
   * child of |this|.
   *
   * @throws NS_ERROR_OUT_OF_MEMORY in some cases (from BindToTree).
   */
  nsresult AppendChildTo(nsIContent* aKid, PRBool aNotify)
  {
    return InsertChildAt(aKid, GetChildCount(), aNotify);
  }
  
  /**
   * Remove a child from this node.  This method handles calling UnbindFromTree
   * on the child appropriately.
   *
   * @param aIndex the index of the child to remove
   * @param aNotify whether to notify the document (current document for
   *        nsIContent, and |this| for nsIDocument) that the remove has
   *        occurred
   *
   * Note: If there is no child at aIndex, this method will simply do nothing.
   */
  virtual nsresult RemoveChildAt(PRUint32 aIndex, PRBool aNotify) = 0;

  /**
   * Get a property associated with this node.
   *
   * @param aPropertyName  name of property to get.
   * @param aStatus        out parameter for storing resulting status.
   *                       Set to NS_PROPTABLE_PROP_NOT_THERE if the property
   *                       is not set.
   * @return               the property. Null if the property is not set
   *                       (though a null return value does not imply the
   *                       property was not set, i.e. it can be set to null).
   */
  void* GetProperty(nsIAtom *aPropertyName,
                    nsresult *aStatus = nsnull) const
  {
    return GetProperty(0, aPropertyName, aStatus);
  }

  /**
   * Get a property associated with this node.
   *
   * @param aCategory      category of property to get.
   * @param aPropertyName  name of property to get.
   * @param aStatus        out parameter for storing resulting status.
   *                       Set to NS_PROPTABLE_PROP_NOT_THERE if the property
   *                       is not set.
   * @return               the property. Null if the property is not set
   *                       (though a null return value does not imply the
   *                       property was not set, i.e. it can be set to null).
   */
  virtual void* GetProperty(PRUint16 aCategory,
                            nsIAtom *aPropertyName,
                            nsresult *aStatus = nsnull) const;

  /**
   * Set a property to be associated with this node. This will overwrite an
   * existing value if one exists. The existing value is destroyed using the
   * destructor function given when that value was set.
   *
   * @param aPropertyName  name of property to set.
   * @param aValue         new value of property.
   * @param aDtor          destructor function to be used when this property
   *                       is destroyed.
   * @param aTransfer      if PR_TRUE the property will not be deleted when the
   *                       ownerDocument of the node changes, if PR_FALSE it
   *                       will be deleted.
   *
   * @return NS_PROPTABLE_PROP_OVERWRITTEN (success value) if the property
   *                                       was already set
   * @throws NS_ERROR_OUT_OF_MEMORY if that occurs
   */
  nsresult SetProperty(nsIAtom *aPropertyName,
                       void *aValue,
                       NSPropertyDtorFunc aDtor = nsnull,
                       PRBool aTransfer = PR_FALSE)
  {
    return SetProperty(0, aPropertyName, aValue, aDtor, aTransfer);
  }

  /**
   * Set a property to be associated with this node. This will overwrite an
   * existing value if one exists. The existing value is destroyed using the
   * destructor function given when that value was set.
   *
   * @param aCategory       category of property to set.
   * @param aPropertyName   name of property to set.
   * @param aValue          new value of property.
   * @param aDtor           destructor function to be used when this property
   *                        is destroyed.
   * @param aTransfer       if PR_TRUE the property will not be deleted when the
   *                        ownerDocument of the node changes, if PR_FALSE it
   *                        will be deleted.
   * @param aOldValue [out] previous value of property.
   *
   * @return NS_PROPTABLE_PROP_OVERWRITTEN (success value) if the property
   *                                       was already set
   * @throws NS_ERROR_OUT_OF_MEMORY if that occurs
   */
  virtual nsresult SetProperty(PRUint16 aCategory,
                               nsIAtom *aPropertyName,
                               void *aValue,
                               NSPropertyDtorFunc aDtor = nsnull,
                               PRBool aTransfer = PR_FALSE,
                               void **aOldValue = nsnull);

  /**
   * Destroys a property associated with this node. The value is destroyed
   * using the destruction function given when that value was set.
   *
   * @param aPropertyName  name of property to destroy.
   *
   * @throws NS_PROPTABLE_PROP_NOT_THERE if the property was not set
   */
  nsresult DeleteProperty(nsIAtom *aPropertyName)
  {
    return DeleteProperty(0, aPropertyName);
  }

  /**
   * Destroys a property associated with this node. The value is destroyed
   * using the destruction function given when that value was set.
   *
   * @param aCategory      category of property to destroy.
   * @param aPropertyName  name of property to destroy.
   *
   * @throws NS_PROPTABLE_PROP_NOT_THERE if the property was not set
   */
  virtual nsresult DeleteProperty(PRUint16 aCategory, nsIAtom *aPropertyName);

  /**
   * Unset a property associated with this node. The value will not be
   * destroyed but rather returned. It is the caller's responsibility to
   * destroy the value after that point.
   *
   * @param aPropertyName  name of property to unset.
   * @param aStatus        out parameter for storing resulting status.
   *                       Set to NS_PROPTABLE_PROP_NOT_THERE if the property
   *                       is not set.
   * @return               the property. Null if the property is not set
   *                       (though a null return value does not imply the
   *                       property was not set, i.e. it can be set to null).
   */
  void* UnsetProperty(nsIAtom  *aPropertyName,
                      nsresult *aStatus = nsnull)
  {
    return UnsetProperty(0, aPropertyName, aStatus);
  }

  /**
   * Unset a property associated with this node. The value will not be
   * destroyed but rather returned. It is the caller's responsibility to
   * destroy the value after that point.
   *
   * @param aCategory      category of property to unset.
   * @param aPropertyName  name of property to unset.
   * @param aStatus        out parameter for storing resulting status.
   *                       Set to NS_PROPTABLE_PROP_NOT_THERE if the property
   *                       is not set.
   * @return               the property. Null if the property is not set
   *                       (though a null return value does not imply the
   *                       property was not set, i.e. it can be set to null).
   */
  virtual void* UnsetProperty(PRUint16 aCategory,
                              nsIAtom *aPropertyName,
                              nsresult *aStatus = nsnull);
  
  PRBool HasProperties() const
  {
    return HasFlag(NODE_HAS_PROPERTIES);
  }

  /**
   * Return the principal of this node.  This is guaranteed to never be a null
   * pointer.
   */
  nsIPrincipal* NodePrincipal() const {
    return mNodeInfo->NodeInfoManager()->DocumentPrincipal();
  }

  /**
   * Get the parent nsIContent for this node.
   * @return the parent, or null if no parent or the parent is not an nsIContent
   */
  nsIContent* GetParent() const
  {
    return NS_LIKELY(mParentPtrBits & PARENT_BIT_PARENT_IS_CONTENT) ?
           reinterpret_cast<nsIContent*>
                           (mParentPtrBits & ~kParentBitMask) :
           nsnull;
  }

  /**
   * Get the parent nsINode for this node. This can be either an nsIContent,
   * an nsIDocument or an nsIAttribute.
   * @return the parent node
   */
  nsINode* GetNodeParent() const
  {
    return reinterpret_cast<nsINode*>(mParentPtrBits & ~kParentBitMask);
  }

  /**
   * Adds a mutation observer to be notified when this node, or any of its
   * descendants, are modified. The node will hold a weak reference to the
   * observer, which means that it is the responsibility of the observer to
   * remove itself in case it dies before the node.  If an observer is added
   * while observers are being notified, it may also be notified.  In general,
   * adding observers while inside a notification is not a good idea.
   */
  virtual void AddMutationObserver(nsIMutationObserver* aMutationObserver);

  /**
   * Removes a mutation observer.
   */
  virtual void RemoveMutationObserver(nsIMutationObserver* aMutationObserver);

  /**
   * Clones this node. This needs to be overriden by all node classes. aNodeInfo
   * should be identical to this node's nodeInfo, except for the document which
   * may be different. When cloning an element, all attributes of the element
   * will be cloned. The children of the node will not be cloned.
   *
   * @param aNodeInfo the nodeinfo to use for the clone
   * @param aResult the clone
   */
  virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const = 0;

  /**
   * Checks if a node has the same ownerDocument as this one. Note that this
   * actually compares nodeinfo managers because nodes always have one, even
   * when they don't have an ownerDocument. If this function returns PR_TRUE
   * it doesn't mean that the nodes actually have an ownerDocument.
   *
   * @param aOther Other node to check
   * @return Whether the owner documents of this node and of aOther are the
   *         same.
   */
  PRBool HasSameOwnerDoc(nsINode *aOther)
  {
    // We compare nodeinfo managers because nodes always have one, even when
    // they don't have an ownerDocument.
    return mNodeInfo->NodeInfoManager() == aOther->mNodeInfo->NodeInfoManager();
  }

  // This class can be extended by subclasses that wish to store more
  // information in the slots.
  class nsSlots
  {
  public:
    nsSlots(PtrBits aFlags)
      : mFlags(aFlags),
        mChildNodes(nsnull),
        mWeakReference(nsnull)
    {
    }

    // If needed we could remove the vtable pointer this dtor causes by
    // putting a DestroySlots function on nsINode
    virtual ~nsSlots();

    /**
     * Storage for flags for this node. These are the same flags as the
     * mFlagsOrSlots member, but these are used when the slots class
     * is allocated.
     */
    PtrBits mFlags;

    /**
     * A list of mutation observers
     */
    nsTObserverArray<nsIMutationObserver*> mMutationObservers;

    /**
     * An object implementing nsIDOMNodeList for this content (childNodes)
     * @see nsIDOMNodeList
     * @see nsGenericHTMLElement::GetChildNodes
     *
     * MSVC 7 doesn't like this as an nsRefPtr
     */
    nsChildContentList* mChildNodes;

    /**
     * Weak reference to this node
     */
    nsNodeWeakReference* mWeakReference;
  };

  /**
   * Functions for managing flags and slots
   */
#ifdef DEBUG
  nsSlots* DebugGetSlots()
  {
    return GetSlots();
  }
#endif

  PRBool HasFlag(PtrBits aFlag) const
  {
    return !!(GetFlags() & aFlag);
  }

  PtrBits GetFlags() const
  {
    return NS_UNLIKELY(HasSlots()) ? FlagsAsSlots()->mFlags : mFlagsOrSlots;
  }

  void SetFlags(PtrBits aFlagsToSet)
  {
    NS_ASSERTION(!(aFlagsToSet & (NODE_IS_ANONYMOUS | NODE_MAY_HAVE_FRAME)) ||
                 IsNodeOfType(eCONTENT),
                 "Flag only permitted on nsIContent nodes");
    PtrBits* flags = HasSlots() ? &FlagsAsSlots()->mFlags :
                                  &mFlagsOrSlots;
    *flags |= aFlagsToSet;
  }

  void UnsetFlags(PtrBits aFlagsToUnset)
  {
    PtrBits* flags = HasSlots() ? &FlagsAsSlots()->mFlags :
                                  &mFlagsOrSlots;
    *flags &= ~aFlagsToUnset;
  }

  void SetEditableFlag(PRBool aEditable)
  {
    if (aEditable) {
      SetFlags(NODE_IS_EDITABLE);
    }
    else {
      UnsetFlags(NODE_IS_EDITABLE);
    }
  }

  PRBool IsEditable() const
  {
#ifdef _IMPL_NS_LAYOUT
    return IsEditableInternal();
#else
    return IsEditableExternal();
#endif
  }

  /**
   * Get the root content of an editor. So, this node must be a descendant of
   * an editor. Note that this should be only used for getting input or textarea
   * editor's root content. This method doesn't support HTML editors.
   */
  nsIContent* GetTextEditorRootContent(nsIEditor** aEditor = nsnull);

  /**
   * Get the nearest selection root, ie. the node that will be selected if the
   * user does "Select All" while the focus is in this node. Note that if this
   * node is not in an editor, the result comes from the nsFrameSelection that
   * is related to aPresShell, so the result might not be the ancestor of this
   * node.
   */
  nsIContent* GetSelectionRootContent(nsIPresShell* aPresShell);

protected:

  // Override this function to create a custom slots class.
  virtual nsINode::nsSlots* CreateSlots();

  PRBool HasSlots() const
  {
    return !(mFlagsOrSlots & NODE_DOESNT_HAVE_SLOTS);
  }

  nsSlots* FlagsAsSlots() const
  {
    NS_ASSERTION(HasSlots(), "check HasSlots first");
    return reinterpret_cast<nsSlots*>(mFlagsOrSlots);
  }

  nsSlots* GetExistingSlots() const
  {
    return HasSlots() ? FlagsAsSlots() : nsnull;
  }

  nsSlots* GetSlots()
  {
    if (HasSlots()) {
      return FlagsAsSlots();
    }

    nsSlots* slots = CreateSlots();
    if (slots) {
      mFlagsOrSlots = reinterpret_cast<PtrBits>(slots);
    }

    return slots;
  }

  nsTObserverArray<nsIMutationObserver*> *GetMutationObservers()
  {
    return HasSlots() ? &FlagsAsSlots()->mMutationObservers : nsnull;
  }

  PRBool IsEditableInternal() const;
  virtual PRBool IsEditableExternal() const
  {
    return IsEditableInternal();
  }

  nsCOMPtr<nsINodeInfo> mNodeInfo;

  enum { PARENT_BIT_INDOCUMENT = 1 << 0, PARENT_BIT_PARENT_IS_CONTENT = 1 << 1 };
  enum { kParentBitMask = 0x3 };

  PtrBits mParentPtrBits;

  /**
   * Used for either storing flags for this node or a pointer to
   * this contents nsContentSlots. See the definition of the
   * NODE_* macros for the layout of the bits in this
   * member.
   */
  PtrBits mFlagsOrSlots;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsINode, NS_INODE_IID)

#endif /* nsINode_h___ */
