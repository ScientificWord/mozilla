/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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
 * The Original Code is Oracle Corporation code.
 *
 * The Initial Developer of the Original Code is Oracle Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@pavlov.net>
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

#ifndef GFX_WINDOWSSURFACE_H
#define GFX_WINDOWSSURFACE_H

#include "gfxASurface.h"
#include "gfxImageSurface.h"

#include <windows.h>

class THEBES_API gfxWindowsSurface : public gfxASurface {
public:
    enum {
        FLAG_TAKE_DC = (1 << 0),
        FLAG_FOR_PRINTING = (1 << 1),
        FLAG_ENH_METAFILE = (1 << 2),
        FLAG_OLD_METAFILE = (1 << 3),
        FLAG_METAFILE = (FLAG_ENH_METAFILE | FLAG_OLD_METAFILE)
    };

    gfxWindowsSurface(HWND wnd);
    gfxWindowsSurface(HDC dc, PRUint32 flags = 0);

    // Create a DIB surface
    gfxWindowsSurface(const gfxIntSize& size,
                      gfxImageFormat imageFormat = ImageFormatRGB24);

    // Create a DDB surface; dc may be NULL to use the screen DC
    gfxWindowsSurface(HDC dc,
                      const gfxIntSize& size,
                      gfxImageFormat imageFormat = ImageFormatRGB24);

    gfxWindowsSurface(cairo_surface_t *csurf);

    virtual ~gfxWindowsSurface();

    HDC GetDC() { return mDC; }

    already_AddRefed<gfxImageSurface> GetImageSurface();

    already_AddRefed<gfxWindowsSurface> OptimizeToDDB(HDC dc,
                                                      const gfxIntSize& size,
                                                      gfxImageFormat format);

    nsresult BeginPrinting(const nsAString& aTitle, const nsAString& aPrintToFileName);
    nsresult EndPrinting();
    nsresult AbortPrinting();
    nsresult BeginPage();
    nsresult EndPage();

    virtual PRInt32 GetDefaultContextFlags() const;

protected:
    PRPackedBool mOwnsDC;
    PRPackedBool mForPrinting;
    PRPackedBool mMetafile;

    HDC mDC;

private:
    HWND mWnd;
};

class THEBES_API gfxWindowsMetafileSurface : public gfxWindowsSurface {
public:
  gfxWindowsMetafileSurface(HDC dc, PRBool oldStyle=PR_FALSE, PRUint32 flags = FLAG_TAKE_DC);
//rwa12-18-12  gfxWindowsMetafileSurface(HDC dc, gfxIntSize& himetSize, PRBool oldStyle=PR_FALSE, PRUint32 flags = FLAG_TAKE_DC);
  ~gfxWindowsMetafileSurface();

//  static already_AddRefed<gfxWindowsMetafileSurface> CreateWindowsMetafileSurface(gfxWindowsSurface* refSurface, 
//                                                             const gfxIntSize& size);

  virtual void Finish();
//  HENHMETAFILE GetEnhMetaFile() {return mHEnhMetafile;}
  virtual nsresult GetEnhMetaFileCopy(nsNativeMetafile*& outMetafile );
  virtual nsresult GetMetaFilePictCopy(void* refDC, nsNativeMetafile*& outMetafile);
  virtual nsresult WriteFile(const nsAString& filename);
  virtual nsresult WriteMetafilePictFile(const nsAString& filename, void* refDC);

  PRBool  IsOldStyleMetafile();

private:
  HENHMETAFILE mHEnhMetafile;
//rwa12-18-12  HMETAFILE    mHMetafile;
//rwa12-18-12  gfxIntSize   mHiMetricExtent;
};

#endif /* GFX_WINDOWSSURFACE_H */
