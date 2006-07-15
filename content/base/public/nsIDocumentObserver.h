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
#ifndef nsIDocumentObserver_h___
#define nsIDocumentObserver_h___

#include "nsISupports.h"
#include "nsIMutationObserver.h"

class nsIAtom;
class nsIContent;
class nsIPresShell;
class nsIStyleSheet;
class nsIStyleRule;
class nsString;
class nsIDocument;

#define NS_IDOCUMENT_OBSERVER_IID \
{ 0x4e14e321, 0xa4bb, 0x49f8, \
  { 0xa5, 0x7a, 0x23, 0x63, 0x66, 0x8d, 0x14, 0xd0 } }

typedef PRUint32 nsUpdateType;

#define UPDATE_CONTENT_MODEL 0x00000001
#define UPDATE_STYLE         0x00000002
#define UPDATE_CONTENT_STATE 0x00000004
#define UPDATE_ALL (UPDATE_CONTENT_MODEL | UPDATE_STYLE | UPDATE_CONTENT_STATE)

// Document observer interface
class nsIDocumentObserver : public nsIMutationObserver
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_IDOCUMENT_OBSERVER_IID)

  /**
   * Notify that a content model update is beginning. This call can be
   * nested.
   */
  virtual void BeginUpdate(nsIDocument *aDocument,
                           nsUpdateType aUpdateType) = 0;

  /**
   * Notify that a content model update is finished. This call can be
   * nested.
   */
  virtual void EndUpdate(nsIDocument *aDocument, nsUpdateType aUpdateType) = 0;

  /**
   * Notify the observer that a document load is beginning.
   */
  virtual void BeginLoad(nsIDocument *aDocument) = 0;

  /**
   * Notify the observer that a document load has finished. Note that
   * the associated reflow of the document will be done <b>before</b>
   * EndLoad is invoked, not after.
   */
  virtual void EndLoad(nsIDocument *aDocument) = 0;

  /**
   * Notification that the state of a content node has changed. 
   * (ie: gained or lost focus, became active or hovered over)
   * This method is called automatically by content objects 
   * when their state is changed (therefore there is normally 
   * no need to invoke this method directly).  The notification 
   * is passed to any IDocumentObservers. The notification is 
   * passed on to all of the document observers. <p>
   *
   * This notification is not sent when a piece of content is
   * added/removed from the document or the content itself changed 
   * (the other notifications are used for that).
   *
   * The optional second content node is to allow optimization
   * of the case where state moves from one node to another
   * (as is likely for :focus and :hover)
   *
   * Either content node may be nsnull, but not both
   *
   * @param aDocument The document being observed
   * @param aContent1 the piece of content that changed
   * @param aContent2 optional second piece of content that changed
   */
  virtual void ContentStatesChanged(nsIDocument* aDocument,
                                    nsIContent* aContent1,
                                    nsIContent* aContent2,
                                    PRInt32 aStateMask) = 0;

  /**
   * A StyleSheet has just been added to the document.  This method is
   * called automatically when a StyleSheet gets added to the
   * document, even if the stylesheet is not applicable. The
   * notification is passed on to all of the document observers.   
   *
   * @param aDocument The document being observed
   * @param aStyleSheet the StyleSheet that has been added
   * @param aDocumentSheet True if sheet is in document's style sheet list,
   *                       false if sheet is not (i.e., UA or user sheet)
   */
  virtual void StyleSheetAdded(nsIDocument *aDocument,
                               nsIStyleSheet* aStyleSheet,
                               PRBool aDocumentSheet) = 0;

  /**
   * A StyleSheet has just been removed from the document.  This
   * method is called automatically when a StyleSheet gets removed
   * from the document, even if the stylesheet is not applicable. The
   * notification is passed on to all of the document observers.
   *
   * @param aDocument The document being observed
   * @param aStyleSheet the StyleSheet that has been removed
   * @param aDocumentSheet True if sheet is in document's style sheet list,
   *                       false if sheet is not (i.e., UA or user sheet)
   */
  virtual void StyleSheetRemoved(nsIDocument *aDocument,
                                 nsIStyleSheet* aStyleSheet,
                                 PRBool aDocumentSheet) = 0;
  
  /**
   * A StyleSheet has just changed its applicable state.
   * This method is called automatically when the applicable state
   * of a StyleSheet gets changed. The style sheet passes this
   * notification to the document. The notification is passed on 
   * to all of the document observers.
   *
   * @param aDocument The document being observed
   * @param aStyleSheet the StyleSheet that has changed state
   * @param aApplicable PR_TRUE if the sheet is applicable, PR_FALSE if
   *        it is not applicable
   */
  virtual void StyleSheetApplicableStateChanged(nsIDocument *aDocument,
                                                nsIStyleSheet* aStyleSheet,
                                                PRBool aApplicable) = 0;

  /**
   * A StyleRule has just been modified within a style sheet.
   * This method is called automatically when the rule gets
   * modified. The style sheet passes this notification to 
   * the document. The notification is passed on to all of 
   * the document observers.
   *
   * Since nsIStyleRule objects are immutable, there is a new object
   * replacing the old one.  However, the use of this method (rather
   * than StyleRuleAdded and StyleRuleRemoved) implies that the new rule
   * matches the same elements and has the same priority (weight,
   * origin, specificity) as the old one.  (However, if it is a CSS
   * style rule, there may be a change in whether it has an important
   * rule.)
   *
   * @param aDocument The document being observed
   * @param aStyleSheet the StyleSheet that contians the rule
   * @param aOldStyleRule The rule being removed.  This rule may not be
   *                      fully valid anymore -- however, it can still
   *                      be used for pointer comparison and
   *                      |QueryInterface|.
   * @param aNewStyleRule The rule being added.
   */
  virtual void StyleRuleChanged(nsIDocument *aDocument,
                                nsIStyleSheet* aStyleSheet,
                                nsIStyleRule* aOldStyleRule,
                                nsIStyleRule* aNewStyleRule) = 0;

  /**
   * A StyleRule has just been added to a style sheet.
   * This method is called automatically when the rule gets
   * added to the sheet. The style sheet passes this
   * notification to the document. The notification is passed on 
   * to all of the document observers.
   *
   * @param aDocument The document being observed
   * @param aStyleSheet the StyleSheet that has been modified
   * @param aStyleRule the rule that was added
   */
  virtual void StyleRuleAdded(nsIDocument *aDocument,
                              nsIStyleSheet* aStyleSheet,
                              nsIStyleRule* aStyleRule) = 0;

  /**
   * A StyleRule has just been removed from a style sheet.
   * This method is called automatically when the rule gets
   * removed from the sheet. The style sheet passes this
   * notification to the document. The notification is passed on 
   * to all of the document observers.
   *
   * @param aDocument The document being observed
   * @param aStyleSheet the StyleSheet that has been modified
   * @param aStyleRule the rule that was removed
   */
  virtual void StyleRuleRemoved(nsIDocument *aDocument,
                                nsIStyleSheet* aStyleSheet,
                                nsIStyleRule* aStyleRule) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsIDocumentObserver, NS_IDOCUMENT_OBSERVER_IID)

#define NS_DECL_NSIDOCUMENTOBSERVER                                          \
    virtual void BeginUpdate(nsIDocument* aDocument, nsUpdateType aUpdateType);\
    virtual void EndUpdate(nsIDocument* aDocument, nsUpdateType aUpdateType);\
    virtual void BeginLoad(nsIDocument* aDocument);                          \
    virtual void EndLoad(nsIDocument* aDocument);                            \
    virtual void ContentStatesChanged(nsIDocument* aDocument,                \
                                      nsIContent* aContent1,                 \
                                      nsIContent* aContent2,                 \
                                      PRInt32 aStateMask);                   \
    virtual void StyleSheetAdded(nsIDocument* aDocument,                     \
                                 nsIStyleSheet* aStyleSheet,                 \
                                 PRBool aDocumentSheet);                     \
    virtual void StyleSheetRemoved(nsIDocument* aDocument,                   \
                                   nsIStyleSheet* aStyleSheet,               \
                                   PRBool aDocumentSheet);                   \
    virtual void StyleSheetApplicableStateChanged(nsIDocument* aDocument,    \
                                                  nsIStyleSheet* aStyleSheet,\
                                                  PRBool aApplicable);       \
    virtual void StyleRuleChanged(nsIDocument* aDocument,                    \
                                  nsIStyleSheet* aStyleSheet,                \
                                  nsIStyleRule* aOldStyleRule,               \
                                  nsIStyleRule* aNewStyleRule);              \
    virtual void StyleRuleAdded(nsIDocument* aDocument,                      \
                                nsIStyleSheet* aStyleSheet,                  \
                                nsIStyleRule* aStyleRule);                   \
    virtual void StyleRuleRemoved(nsIDocument* aDocument,                    \
                                  nsIStyleSheet* aStyleSheet,                \
                                  nsIStyleRule* aStyleRule);                 \
    NS_DECL_NSIMUTATIONOBSERVER


#define NS_IMPL_NSIDOCUMENTOBSERVER_CORE_STUB(_class)                     \
void                                                                      \
_class::BeginUpdate(nsIDocument* aDocument, nsUpdateType aUpdateType)     \
{                                                                         \
}                                                                         \
void                                                                      \
_class::EndUpdate(nsIDocument* aDocument, nsUpdateType aUpdateType)       \
{                                                                         \
}                                                                         \
NS_IMPL_NSIMUTATIONOBSERVER_CORE_STUB(_class)

#define NS_IMPL_NSIDOCUMENTOBSERVER_LOAD_STUB(_class)                     \
void                                                                      \
_class::BeginLoad(nsIDocument* aDocument)                                 \
{                                                                         \
}                                                                         \
void                                                                      \
_class::EndLoad(nsIDocument* aDocument)                                   \
{                                                                         \
}

#define NS_IMPL_NSIDOCUMENTOBSERVER_STATE_STUB(_class)                    \
void                                                                      \
_class::ContentStatesChanged(nsIDocument* aDocument,                      \
                             nsIContent* aContent1,                       \
                             nsIContent* aContent2,                       \
                             PRInt32 aStateMask)                          \
{                                                                         \
}

#define NS_IMPL_NSIDOCUMENTOBSERVER_CONTENT(_class)                       \
NS_IMPL_NSIMUTATIONOBSERVER_CONTENT(_class)

#define NS_IMPL_NSIDOCUMENTOBSERVER_STYLE_STUB(_class)                    \
void                                                                      \
_class::StyleSheetAdded(nsIDocument* aDocument,                           \
                        nsIStyleSheet* aStyleSheet,                       \
                        PRBool aDocumentSheet)                            \
{                                                                         \
}                                                                         \
void                                                                      \
_class::StyleSheetRemoved(nsIDocument* aDocument,                         \
                          nsIStyleSheet* aStyleSheet,                     \
                          PRBool aDocumentSheet)                          \
{                                                                         \
}                                                                         \
void                                                                      \
_class::StyleSheetApplicableStateChanged(nsIDocument* aDocument,          \
                                         nsIStyleSheet* aStyleSheet,      \
                                         PRBool aApplicable)              \
{                                                                         \
}                                                                         \
void                                                                      \
_class::StyleRuleChanged(nsIDocument* aDocument,                          \
                         nsIStyleSheet* aStyleSheet,                      \
                         nsIStyleRule* aOldStyleRule,                     \
                         nsIStyleRule* aNewStyleRule)                     \
{                                                                         \
}                                                                         \
void                                                                      \
_class::StyleRuleAdded(nsIDocument* aDocument,                            \
                       nsIStyleSheet* aStyleSheet,                        \
                       nsIStyleRule* aStyleRule)                          \
{                                                                         \
}                                                                         \
void                                                                      \
_class::StyleRuleRemoved(nsIDocument* aDocument,                          \
                         nsIStyleSheet* aStyleSheet,                      \
                         nsIStyleRule* aStyleRule)                        \
{                                                                         \
}

#endif /* nsIDocumentObserver_h___ */
