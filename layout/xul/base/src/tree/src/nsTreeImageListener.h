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
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dave Hyatt <hyatt@mozilla.org> (Original Author)
 *   Jan Varga <varga@ku.sk>
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

#ifndef nsTreeImageListener_h__
#define nsTreeImageListener_h__

#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsITreeColumns.h"
#include "nsStubImageDecoderObserver.h"

class nsITreeBoxObject;

// The interface for our image listener.
// {90586540-2D50-403e-8DCE-981CAA778444}
#define NS_ITREEIMAGELISTENER_IID \
{ 0x90586540, 0x2d50, 0x403e, { 0x8d, 0xce, 0x98, 0x1c, 0xaa, 0x77, 0x84, 0x44 } }

class nsITreeImageListener : public nsISupports
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_ITREEIMAGELISTENER_IID)

  NS_IMETHOD AddCell(PRInt32 aIndex, nsITreeColumn* aCol) = 0;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsITreeImageListener, NS_ITREEIMAGELISTENER_IID)

// This class handles image load observation.
class nsTreeImageListener : public nsStubImageDecoderObserver, public nsITreeImageListener
{
public:
  nsTreeImageListener(nsITreeBoxObject* aTree);
  ~nsTreeImageListener();

  NS_DECL_ISUPPORTS
  // imgIDecoderObserver (override nsStubImageDecoderObserver)
  NS_IMETHOD OnStartContainer(imgIRequest *aRequest, imgIContainer *aImage);
  NS_IMETHOD OnDataAvailable(imgIRequest *aRequest, gfxIImageFrame *aFrame,
                             const nsRect *aRect);
  // imgIContainerObserver (override nsStubImageDecoderObserver)
  NS_IMETHOD FrameChanged(imgIContainer *aContainer, gfxIImageFrame *newframe,
                          nsRect * dirtyRect);

  NS_IMETHOD AddCell(PRInt32 aIndex, nsITreeColumn* aCol);
 
  friend class nsTreeBodyFrame;

protected:
  void UnsuppressInvalidation() { mInvalidationSuppressed = PR_FALSE; }
  void Invalidate();

private:
  nsITreeBoxObject* mTree;

  // A guard that prevents us from recursive painting.
  PRBool mInvalidationSuppressed;

  class InvalidationArea {
    public:
      InvalidationArea(nsITreeColumn* aCol);
      ~InvalidationArea() { delete mNext; }

      friend class nsTreeImageListener;

    protected:
      void AddRow(PRInt32 aIndex);
      nsITreeColumn* GetCol() { return mCol.get(); }
      PRInt32 GetMin() { return mMin; }
      PRInt32 GetMax() { return mMax; }
      InvalidationArea* GetNext() { return mNext; }
      void SetNext(InvalidationArea* aNext) { mNext = aNext; }

    private:
      nsCOMPtr<nsITreeColumn> mCol;
      PRInt32                 mMin;
      PRInt32                 mMax;
      InvalidationArea*       mNext;
  };

  InvalidationArea* mInvalidationArea;
};

#endif // nsTreeImageListener_h__
