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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
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

#ifndef nsXFormsStubElement_h_
#define nsXFormsStubElement_h_

#include "nsIXTFElement.h"
#include "nsIXFormsControlBase.h"
#include "nsIDOMElement.h"

/**
 * nsRepeatState is used to indicate whether the element is inside a
 * \<repeat\> or \<itemset\> template. If it is, there is no need
 * to refresh the widget bound to the element.
 *
 *  eType_Unknown - repeat state has yet to be determined
 *  eType_Template - element lives inside the template (an original element
 *                   from the document that is hidden from the user)
 *  eType_GeneratedContent - A generated clone of an element from the template.
 *                           It is generated as the repeat or itemset processes
 *                           the nodeset that it is bound to.  For every node in
 *                           the nodeset, the repeat or itemset will go through
 *                           its template and create a clone for every element
 *                           in the template.  A user will see these elements.
 *  eType_NotApplicable - element lives in the DOM and is not contained
 *                        by a repeat or an itemset
 */
enum nsRepeatState {
  eType_Unknown,
  eType_Template,
  eType_GeneratedContent,
  eType_NotApplicable
};

/**
 * An implementation of a generic XForms element.
 */
class nsXFormsStubElement : public nsIXTFElement
{
protected:
  // We need a virtual destructor so that when a subclass does
  // NS_IMPL_ISUPPORTS_INHERITED, our Release() implementation calls the
  // derived class destructor.
  virtual ~nsXFormsStubElement() {}

  /**
   * This is processed when an XForms control or XForms action has been inserted
   * under a parent node AND has been inserted into a document.
   * It checks the ancestors of the element and returns an nsRepeatState
   * depending on the element's place in the document.
   *
   * @param aParent           The new parent of the XForms control
   */
  virtual nsRepeatState UpdateRepeatState(nsIDOMNode *aParent);

  nsRepeatState mRepeatState;

  /**
   * State that tells whether control has a parent or not.  This could be
   * false even if the control has a parent.  Just means that the element
   * doesn't need to track whether it has a parent or not (i.e. xf:choices)
   */
  PRPackedBool                        mHasParent;

  /**
   * State that tells whether control has a document or not.  This could be
   * false even if the control has a document.  Just means that the element
   * doesn't need to track whether it has a document or not (i.e. xf:choices)
   */
  PRPackedBool                        mHasDoc;

public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIXTFELEMENT

  /** Constructor */
  nsXFormsStubElement() :
    mRepeatState(eType_Unknown),
    mHasParent(PR_FALSE),
    mHasDoc(PR_FALSE)
    {};

  /**
   * Get/Set the repeat state for the xforms control or action.  The repeat
   * state indicates whether the control or action lives inside a context
   * container, a repeat element, an itemset or none of the above.
   */
  virtual nsRepeatState GetRepeatState();
  virtual void SetRepeatState(nsRepeatState aState);

};

/* Factory methods */
NS_HIDDEN_(nsresult)
NS_NewXFormsStubElement(nsIXTFElement **aResult);
#endif
