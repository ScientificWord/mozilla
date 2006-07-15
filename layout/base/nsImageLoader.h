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

/* class to notify frames of background image loads */

#include "nsStubImageDecoderObserver.h"

class nsPresContext;
class nsIFrame;
class nsIURI;

#include "imgIRequest.h"
#include "nsCOMPtr.h"

class nsImageLoader : public nsStubImageDecoderObserver
{
public:
  nsImageLoader();
  virtual ~nsImageLoader();

  NS_DECL_ISUPPORTS

  // imgIDecoderObserver (override nsStubImageDecoderObserver)
  NS_IMETHOD OnStartContainer(imgIRequest *aRequest, imgIContainer *aImage);
  NS_IMETHOD OnStopFrame(imgIRequest *aRequest, gfxIImageFrame *aFrame);
  // Do not override OnDataAvailable since background images are not
  // displayed incrementally; they are displayed after the entire image
  // has been loaded.
  // Note: Images referenced by the <img> element are displayed
  // incrementally in nsImageFrame.cpp.

  // imgIContainerObserver (override nsStubImageDecoderObserver)
  NS_IMETHOD FrameChanged(imgIContainer *aContainer, gfxIImageFrame *newframe,
                          nsRect * dirtyRect);

  void Init(nsIFrame *aFrame, nsPresContext *aPresContext);
  nsresult Load(imgIRequest *aImage);

  void Destroy();

  nsIFrame *GetFrame() { return mFrame; }
  imgIRequest *GetRequest() { return mRequest; }

private:
  void RedrawDirtyFrame(const nsRect* aDamageRect);

private:
  nsIFrame *mFrame;
  nsPresContext *mPresContext;
  nsCOMPtr<imgIRequest> mRequest;
};
