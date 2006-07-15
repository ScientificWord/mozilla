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

#include "gfxWindowsSurface.h"
#include "nsString.h"

#include <cairo-win32.h>

gfxWindowsSurface::gfxWindowsSurface(HWND wnd) :
    mOwnsDC(PR_TRUE), mWnd(wnd)
{
    mDC = ::GetDC(mWnd);
    Init(cairo_win32_surface_create(mDC));
}

gfxWindowsSurface::gfxWindowsSurface(HDC dc, PRBool deleteDC) :
    mOwnsDC(deleteDC), mDC(dc),mWnd(nsnull)
{
    Init(cairo_win32_surface_create(mDC));
}

gfxWindowsSurface::gfxWindowsSurface(HDC dc, unsigned long width, unsigned long height, gfxImageFormat imageFormat) :
    mOwnsDC(PR_FALSE), mWnd(nsnull)
{
    HBITMAP bmp = nsnull;

    // At some point we might want to revisit creating a DDB
    // with CreateCompatibleDC, but from stuff that I've read
    // that's only useful for terminal services clients.

    Init(cairo_win32_surface_create_with_dib((cairo_format_t)imageFormat,
                                             width, height));

    mDC = cairo_win32_surface_get_dc(CairoSurface());
}


gfxWindowsSurface::gfxWindowsSurface(cairo_surface_t *csurf) :
    mOwnsDC(PR_FALSE), mWnd(nsnull)
{
    mDC = cairo_win32_surface_get_dc(csurf);

    Init(csurf, PR_TRUE);
}

gfxWindowsSurface::~gfxWindowsSurface()
{
    Destroy();

    if (mOwnsDC) {
        if (mWnd)
            ::ReleaseDC(mWnd, mDC);
        else
            ::DeleteDC(mDC);
    }
}

static char*
GetACPString(const nsAString& aStr)
{
    int acplen = aStr.Length() * 2 + 1;
    char * acp = new char[acplen];
    if(acp) {
        int outlen = ::WideCharToMultiByte(CP_ACP, 0, 
                                           PromiseFlatString(aStr).get(),
                                           aStr.Length(),
                                           acp, acplen, NULL, NULL);
        if (outlen > 0)
            acp[outlen] = '\0';  // null terminate
    }
    return acp;
}

nsresult gfxWindowsSurface::BeginPrinting(const nsAString& aTitle,
                                          const nsAString& aPrintToFileName)
{
#define DOC_TITLE_LENGTH 30
    DOCINFO docinfo;

    nsString titleStr;
    titleStr = aTitle;
    if (titleStr.Length() > DOC_TITLE_LENGTH) {
        titleStr.SetLength(DOC_TITLE_LENGTH-3);
        titleStr.AppendLiteral("...");
    }
    char *title = GetACPString(titleStr);

    char *docName = nsnull;
    if (!aPrintToFileName.IsEmpty()) {
        docName = ToNewCString(aPrintToFileName);
    }

    docinfo.cbSize = sizeof(docinfo);
    docinfo.lpszDocName = title ? title : "Mozilla Document";
    docinfo.lpszOutput = docName;
    docinfo.lpszDatatype = NULL;
    docinfo.fwType = 0;

    ::StartDoc(mDC, &docinfo);
        
    delete [] title;
    if (docName != nsnull) nsMemory::Free(docName);

    return NS_OK;
}

nsresult gfxWindowsSurface::EndPrinting()
{
    ::EndDoc(mDC);
    return NS_OK;
}

nsresult gfxWindowsSurface::AbortPrinting()
{
    ::AbortDoc(mDC);
    return NS_OK;
}

nsresult gfxWindowsSurface::BeginPage()
{
    ::StartPage(mDC);
    return NS_OK;
}

nsresult gfxWindowsSurface::EndPage()
{
    ::EndPage(mDC);
    return NS_OK;
}
