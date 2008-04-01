/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Gao, Ming (gaoming@cn.ibm.com)
 *   Neo Liu(nian.liu@sun.com)
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

#include "nsAccessibleWrap.h"
#include "nsMaiInterfaceImage.h"

extern "C" const gchar* getDescriptionCB(AtkObject* aAtkObj);

void
imageInterfaceInitCB(AtkImageIface *aIface)
{
    g_return_if_fail(aIface != NULL);

    aIface->get_image_position = getImagePositionCB;
    aIface->get_image_description = getImageDescriptionCB;
    aIface->get_image_size = getImageSizeCB;

}

void
getImagePositionCB(AtkImage *aImage, gint *aAccX, gint *aAccY,
                   AtkCoordType aCoordType)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aImage));
    if (!accWrap) 
      return;

    nsCOMPtr<nsIAccessibleImage> image;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleImage),
                            getter_AddRefs(image));
    if (!image)
      return;

    PRUint32 geckoCoordType = (aCoordType == ATK_XY_WINDOW) ?
      nsIAccessibleCoordinateType::COORDTYPE_WINDOW_RELATIVE :
      nsIAccessibleCoordinateType::COORDTYPE_SCREEN_RELATIVE;

    // Returned in screen coordinates
    image->GetImagePosition(geckoCoordType, aAccX, aAccY);
}

const gchar *
getImageDescriptionCB(AtkImage *aImage)
{
   return getDescriptionCB(ATK_OBJECT(aImage));
}

void
getImageSizeCB(AtkImage *aImage, gint *aAccWidth, gint *aAccHeight)
{
    nsAccessibleWrap *accWrap = GetAccessibleWrap(ATK_OBJECT(aImage));
    if (!accWrap) 
      return;

    nsCOMPtr<nsIAccessibleImage> image;
    accWrap->QueryInterface(NS_GET_IID(nsIAccessibleImage),
                            getter_AddRefs(image));
    if (!image)
      return;

    image->GetImageSize(aAccWidth, aAccHeight);
}
