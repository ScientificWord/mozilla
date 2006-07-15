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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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
#ifndef nsIMutationObserver_h___
#define nsIMutationObserver_h___

#include "nsISupports.h"

class nsIAtom;
class nsIContent;
class nsIDocument;
class nsINode;

#define NS_IMUTATION_OBSERVER_IID \
{ 0x0864a23d, 0x824b, 0x48be, \
 { 0x9d, 0x50, 0xff, 0x30, 0x2f, 0xf2, 0x79, 0xfe } }

// Mutation observer interface
class nsIMutationObserver : public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IMUTATION_OBSERVER_IID)

  /**
   * Notification that the node value of a data node (text, cdata, pi, comment)
   * has changed.
   *
   * This notification is not sent when a piece of content is
   * added/removed from the document (the other notifications are used
   * for that).
   *
   * @param aDocument The owner-document of aContent. Can be null.
   * @param aContent  The piece of content that changed. Is never null.
   * @param aAppend   Whether the change was an append
   */
  virtual void CharacterDataChanged(nsIDocument *aDocument,
                                    nsIContent* aContent,
                                    PRBool aAppend) = 0;

  /**
   * Notification that an attribute of an element has changed.
   *
   * @param aDocument    The owner-document of aContent. Can be null.
   * @param aContent     The element whose attribute changed
   * @param aNameSpaceID The namespace id of the changed attribute
   * @param aAttribute   The name of the changed attribute
   * @param aModType     Whether or not the attribute was added, changed, or
   *                     removed. The constants are defined in
   *                     nsIDOMMutationEvent.h.
   */
  virtual void AttributeChanged(nsIDocument* aDocument,
                                nsIContent*  aContent,
                                PRInt32      aNameSpaceID,
                                nsIAtom*     aAttribute,
                                PRInt32      aModType) = 0;

  /**
   * Notification that one or more content nodes have been appended to the
   * child list of another node in the tree.
   *
   * @param aDocument  The owner-document of aContent. Can be null.
   * @param aContainer The container that had new children appended. Is never
   *                   null.
   * @param aNewIndexInContainer the index in the container of the first
   *                   new child
   */
  virtual void ContentAppended(nsIDocument *aDocument,
                               nsIContent* aContainer,
                               PRInt32     aNewIndexInContainer) = 0;

  /**
   * Notification that a content node has been inserted as child to another
   * node in the tree.
   *
   * @param aDocument  The owner-document of aContent, or, when aContainer
   *                   is null, the container that had the child inserted.
   *                   Can be null.
   * @param aContainer The container that had new a child inserted. Can be
   *                   null to indicate that the child was inserted into
   *                   aDocument
   * @param aChild     The newly inserted child.
   * @param aIndexInContainer The index in the container of the new child.
   */
  virtual void ContentInserted(nsIDocument *aDocument,
                               nsIContent* aContainer,
                               nsIContent* aChild,
                               PRInt32 aIndexInContainer) = 0;

  /**
   * Notification that a content node has been removed from the child list of
   * another node in the tree.
   *
   * @param aDocument  The owner-document of aContent, or, when aContainer
   *                   is null, the container that had the child removed.
   *                   Can be null.
   * @param aContainer The container that had new a child removed. Can be
   *                   null to indicate that the child was removed from
   *                   aDocument.
   * @param aChild     The child that was removed.
   * @param aIndexInContainer The index in the container which the child used
   *                          to have.
   */
  virtual void ContentRemoved(nsIDocument *aDocument,
                              nsIContent* aContainer,
                              nsIContent* aChild,
                              PRInt32 aIndexInContainer) = 0;

 /**
   * The node is in the process of being destroyed. Calling QI on the node is
   * not supported, however it is possible to get children and flags through
   * nsINode as well as calling IsNodeOfType(eCONTENT) and casting to
   * nsIContent to get attributes.
   *
   * NOTE: This notification is only called on observers registered directly
   * on the node. This is because when the node is destroyed it can not have
   * any ancestors. If you want to know when a descendant node is being
   * removed from the observed node, use the ContentRemoved notification.
   * 
   * @param aNode The node being destroyed.
   */
  virtual void NodeWillBeDestroyed(const nsINode *aNode) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIMutationObserver, NS_IMUTATION_OBSERVER_IID)

#define NS_DECL_NSIMUTATIONOBSERVER                                          \
    virtual void CharacterDataChanged(nsIDocument* aDocument,                \
                                      nsIContent* aContent,                  \
                                      PRBool aAppend);                       \
    virtual void AttributeChanged(nsIDocument* aDocument,                    \
                                  nsIContent* aContent,                      \
                                  PRInt32 aNameSpaceID,                      \
                                  nsIAtom* aAttribute,                       \
                                  PRInt32 aModType);                         \
    virtual void ContentAppended(nsIDocument* aDocument,                     \
                                 nsIContent* aContainer,                     \
                                 PRInt32 aNewIndexInContainer);              \
    virtual void ContentInserted(nsIDocument* aDocument,                     \
                                 nsIContent* aContainer,                     \
                                 nsIContent* aChild,                         \
                                 PRInt32 aIndexInContainer);                 \
    virtual void ContentRemoved(nsIDocument* aDocument,                      \
                                nsIContent* aContainer,                      \
                                nsIContent* aChild,                          \
                                PRInt32 aIndexInContainer);                  \
    virtual void NodeWillBeDestroyed(const nsINode* aNode);


#define NS_IMPL_NSIMUTATIONOBSERVER_CORE_STUB(_class)                     \
void                                                                      \
_class::NodeWillBeDestroyed(const nsINode* aNode)                               \
{                                                                         \
}

#define NS_IMPL_NSIMUTATIONOBSERVER_CONTENT(_class)                       \
void                                                                      \
_class::CharacterDataChanged(nsIDocument* aDocument,                      \
                             nsIContent* aContent,                        \
                             PRBool aAppend)                              \
{                                                                         \
}                                                                         \
void                                                                      \
_class::AttributeChanged(nsIDocument* aDocument,                          \
                         nsIContent* aContent,                            \
                         PRInt32 aNameSpaceID,                            \
                         nsIAtom* aAttribute,                             \
                         PRInt32 aModType)                                \
{                                                                         \
}                                                                         \
void                                                                      \
_class::ContentAppended(nsIDocument* aDocument,                           \
                        nsIContent* aContainer,                           \
                        PRInt32 aNewIndexInContainer)                     \
{                                                                         \
}                                                                         \
void                                                                      \
_class::ContentInserted(nsIDocument* aDocument,                           \
                        nsIContent* aContainer,                           \
                        nsIContent* aChild,                               \
                        PRInt32 aIndexInContainer)                        \
{                                                                         \
}                                                                         \
void                                                                      \
_class::ContentRemoved(nsIDocument* aDocument,                            \
                       nsIContent* aContainer,                            \
                       nsIContent* aChild,                                \
                       PRInt32 aIndexInContainer)                         \
{                                                                         \
}

#endif /* nsIMutationObserver_h___ */
