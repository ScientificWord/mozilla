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
 * Novell, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Allan Beaufour <abeaufour@novell.com>
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

#ifndef __NSXFORMSACCESSORS_H__
#define __NSXFORMSACCESSORS_H__

#include "nsIClassInfo.h"
#include "nsIXFormsAccessors.h"
#include "nsIDelegateInternal.h"

class nsIDOMElement;

/**
 * Implementation of the nsIXFormsAccessors object. It is always owned by a
 * nsIXFormsDelegate.
 */
class nsXFormsAccessors : public nsIXFormsAccessors,
                          public nsIClassInfo
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICLASSINFO
  NS_DECL_NSIXFORMSACCESSORS

  /** Constructor */
  nsXFormsAccessors(nsIDelegateInternal* aDelegate, nsIDOMElement* aElement)
    : mDelegate(aDelegate), mElement(aElement) 
  {
  }

  /** Called by the owning delegate when it itself is destroyed */
  void Destroy();

protected:
  /**
   * Checks the status of the model item properties
   *
   * @param aState       The state to check
   * @para  aStateVal    The returned state
   */
  nsresult GetState(PRInt32 aState, PRBool *aStateVal);

  /** The delegate owning us */
  nsIDelegateInternal*   mDelegate;

  /** The control DOM element */
  nsIDOMElement*         mElement;
};

#endif
