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

#ifndef nsPNGDecoder_h__
#define nsPNGDecoder_h__

#include "imgIDecoder.h"

#include "imgIContainer.h"
#include "imgIDecoderObserver.h"
#include "gfxIImageFrame.h"
#include "imgILoad.h"


#include "nsCOMPtr.h"

#include "png.h"

#include "lcms.h"

#define NS_PNGDECODER_CID \
{ /* 36fa00c2-1dd2-11b2-be07-d16eeb4c50ed */         \
     0x36fa00c2,                                     \
     0x1dd2,                                         \
     0x11b2,                                         \
    {0xbe, 0x07, 0xd1, 0x6e, 0xeb, 0x4c, 0x50, 0xed} \
}

class nsPNGDecoder : public imgIDecoder
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IMGIDECODER

  nsPNGDecoder();
  virtual ~nsPNGDecoder();

  void CreateFrame(png_uint_32 x_offset, png_uint_32 y_offset, 
                   PRInt32 width, PRInt32 height, gfx_format format);
  void SetAnimFrameInfo();
  
  void EndImageFrame();

public:
  nsCOMPtr<imgIContainer> mImage;
  nsCOMPtr<gfxIImageFrame> mFrame;
  nsCOMPtr<imgILoad> mImageLoad;
  nsCOMPtr<imgIDecoderObserver> mObserver; // this is just qi'd from mRequest for speed

  png_structp mPNG;
  png_infop mInfo;
  PRUint8 *mCMSLine;
  PRUint8 *interlacebuf;
  cmsHPROFILE mInProfile;
  cmsHTRANSFORM mTransform;

  gfx_format format;
  PRUint8 mChannels;
  PRPackedBool mError;
  PRPackedBool mFrameHasNoAlpha;
  PRPackedBool mFrameIsHidden;
};

#endif // nsPNGDecoder_h__
