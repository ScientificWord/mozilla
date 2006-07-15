/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
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
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Bolian Yin (bolian.yin@sun.com)
 *   Ginn Chen (ginn.chen@sun.com)
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

#include "nsMai.h"
#include "nsRootAccessibleWrap.h"
#include "nsAppRootAccessible.h"
#include "nsIDOMWindow.h"
#include "nsPIDOMWindow.h"
#include "nsIFocusController.h"

nsRootAccessibleWrap::nsRootAccessibleWrap(nsIDOMNode *aDOMNode,
                                           nsIWeakReference* aShell):
    nsRootAccessible(aDOMNode, aShell)
{
    MAI_LOG_DEBUG(("New Root Acc=%p\n", (void*)this));
}

NS_IMETHODIMP nsRootAccessibleWrap::Init()
{
    nsresult rv = nsRootAccessible::Init();
    nsAppRootAccessible *root = nsAppRootAccessible::Create();
    if (root) {
        root->AddRootAccessible(this);
    }
    return rv;
}

nsRootAccessibleWrap::~nsRootAccessibleWrap()
{
    MAI_LOG_DEBUG(("Delete Root Acc=%p\n", (void*)this));
}

NS_IMETHODIMP nsRootAccessibleWrap::Shutdown()
{
    nsAppRootAccessible *root = nsAppRootAccessible::Create();
    if (root) {
        root->RemoveRootAccessible(this);
    }
    return nsRootAccessible::Shutdown();
}

NS_IMETHODIMP nsRootAccessibleWrap::GetParent(nsIAccessible **  aParent)
{
    nsAppRootAccessible *root = nsAppRootAccessible::Create();
    nsresult rv = NS_OK;
    if (root) {
        NS_IF_ADDREF(*aParent = root);
    }
    else {
        *aParent = nsnull;
        rv = NS_ERROR_FAILURE;
    }
    return rv;
}

NS_IMETHODIMP nsRootAccessibleWrap::GetRole(PRUint32 *_retval)
{
    *_retval = ROLE_FRAME;
    return NS_OK;
}
