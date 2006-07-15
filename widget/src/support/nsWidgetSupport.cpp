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

#include "nsWidgetSupport.h"
#include "nsRect.h"
#include "nsIWidget.h"

#ifdef XP_MAC
#define WIDGET_SUPPORT_EXPORT(returnType) \
        PR_PUBLIC_API(returnType)
#else
#define WIDGET_SUPPORT_EXPORT(returnType) \
        returnType
#endif

WIDGET_SUPPORT_EXPORT(nsresult)
NS_ShowWidget(nsISupports* aWidget, PRBool aShow)
{
  nsCOMPtr<nsIWidget> widget = do_QueryInterface(aWidget);
  if (widget) {
    widget->Show(aShow);
  }

  return NS_OK;
}

WIDGET_SUPPORT_EXPORT(nsresult)
NS_MoveWidget(nsISupports* aWidget, PRUint32 aX, PRUint32 aY)
{
  nsCOMPtr<nsIWidget> widget = do_QueryInterface(aWidget);
  if (widget) {
    widget->Move(aX, aY);
  }

  return NS_OK;
}

WIDGET_SUPPORT_EXPORT(nsresult)
NS_EnableWidget(nsISupports* aWidget, PRBool aEnable)
{
  nsCOMPtr<nsIWidget> widget = do_QueryInterface(aWidget);
  if (widget) {
    widget->Enable(aEnable);
  }

  return NS_OK;
}

WIDGET_SUPPORT_EXPORT(nsresult)
NS_SetFocusToWidget(nsISupports* aWidget)
{
  nsCOMPtr<nsIWidget> widget = do_QueryInterface(aWidget);
  if (widget) {
    widget->SetFocus();
  }

  return NS_OK;
}

WIDGET_SUPPORT_EXPORT(nsresult)
NS_GetWidgetNativeData(nsISupports* aWidget, void** aNativeData)
{
  void *result = nsnull;
  nsCOMPtr<nsIWidget> widget = do_QueryInterface(aWidget);
  if (widget) {
    result = widget->GetNativeData(NS_NATIVE_WIDGET);
  }

  *aNativeData = result;

  return NS_OK;
}
