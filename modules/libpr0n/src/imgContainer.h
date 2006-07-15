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
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@netscape.com>
 *   Chris Saari <saari@netscape.com>
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

#ifndef __imgContainer_h__
#define __imgContainer_h__

#include "nsCOMPtr.h"
#include "imgIContainer.h"
#include "gfxIImageFrame.h"
#include "nsIProperties.h"


#define NS_IMGCONTAINER_CID \
{ /* 27f0682c-ff64-4dd2-ae7a-668e59f2fd38 */         \
     0x27f0682c,                                     \
     0xff64,                                         \
     0x4dd2,                                         \
    {0xae, 0x7a, 0x66, 0x8e, 0x59, 0xf2, 0xfd, 0x38} \
}

class imgContainer : public imgIContainer, public nsIProperties
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IMGICONTAINER
  NS_FORWARD_SAFE_NSIPROPERTIES(mProperties)

  imgContainer();
  virtual ~imgContainer();

private:
  nsIntSize mSize;
  nsCOMPtr<gfxIImageFrame> mFrame;
  nsCOMPtr<nsIProperties>  mProperties;
};

#endif /* __imgContainer_h__ */
