/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * Christopher Blizzard <blizzard@mozilla.org>.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Christopher Blizzard <blizzard@mozilla.org>
 *   Peter Hartshorn <peter@igelaus.com.au>
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

#ifndef nsDragService_h__
#define nsDragService_h__

#include "nsBaseDragService.h"
#include "nsIDragService.h"
#include "nsIDragSessionXlib.h"
#include "nsGUIEvent.h"
#include "nsIWidget.h"
#include "nsWidget.h"

/**
 * xlib shell based on gtk
 */

class nsDragService : public nsBaseDragService, public nsIDragSessionXlib
{

public:

  NS_DECL_ISUPPORTS_INHERITED

  nsDragService();
  virtual ~nsDragService();

  // nsIDragService
  NS_IMETHOD InvokeDragSession (nsIDOMNode *aDOMNode,
                                nsISupportsArray * anArrayTransferables,
                                nsIScriptableRegion * aRegion,
                                PRUint32 aActionType);
  NS_IMETHOD StartDragSession();
  NS_IMETHOD EndDragSession();

  // nsIDragSession
  NS_IMETHOD GetCurrentSession     (nsIDragSession **aSession);
  NS_IMETHOD SetCanDrop            (PRBool           aCanDrop);
  NS_IMETHOD GetCanDrop            (PRBool          *aCanDrop);
  NS_IMETHOD GetNumDropItems       (PRUint32 * aNumItems);
  NS_IMETHOD GetData               (nsITransferable * aTransferable, PRUint32 anItemIndex);
  NS_IMETHOD IsDataFlavorSupported (const char *aDataFlavor, PRBool *_retval);

  //nsIDragSessionXlib
  NS_IMETHOD IsDragging(PRBool *result);
  NS_IMETHOD UpdatePosition(PRInt32 x, PRInt32 y);

protected:
  static nsEventStatus PR_CALLBACK Callback(nsGUIEvent *event);

private:
  static nsWidget *sWidget;
  static Window sWindow;
  static XlibRgbHandle *sXlibRgbHandle;
  static Display *sDisplay;
  static PRBool mDragging;
  PRBool mCanDrop;

  nsWidget *mDropWidget;
  nsCOMPtr <nsISupportsArray> mSourceDataItems;

  void CreateDragCursor(PRUint32 aActionType);
};

#endif // nsDragService_h__
