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
 * The Original Code is Mozilla Communicator client code.
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

#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include <string.h>

#include "plugin.h"

// return TRUE if we've never seen this MIME type before
BOOL IsNewMimeType(PSZ mime)   
{
  ULONG keysize = 512;
  char keybuf[512];

  PrfQueryProfileString(HINI_USERPROFILE, OS2INI_PLACE, mime, "", keybuf, keysize);
  if (keybuf[0] != '\0') {
    return FALSE;
  }
  else 
  {
    if (!(PrfWriteProfileString(HINI_USERPROFILE, OS2INI_PLACE, mime, "(none)")))
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, "Error adding MIME type value", "Default Plugin", 0, MB_OK);
    return TRUE;
  }
}

// string length in pixels for the specific window (selected font)
static int getWindowStringLength(HWND hWnd, PSZ lpsz)
{
  HPS hPS = WinGetPS(hWnd);

  POINTL ptls[5];
  GpiQueryTextBox(hPS, strlen(lpsz), lpsz, 5, ptls);
  POINTL pt;
  pt.x = ptls[TXTBOX_CONCAT].x;
  pt.y = ptls[TXTBOX_TOPLEFT].y - ptls[TXTBOX_BOTTOMLEFT].y;
  WinReleasePS(hPS);
  return (int)pt.x;
}

/****************************************************************/
/*                                                              */
/* void SetDlgItemTextWrapped(HWND hWnd, int iID, PSZ szText)   */
/*                                                              */
/* helper to wrap long lines in a static control, which do not  */
/* wrap automatically if they do not have space characters      */
/*                                                              */
/****************************************************************/
void SetDlgItemTextWrapped(HWND hWnd, int iID, PSZ szText)
{
  HWND hWndStatic = WinWindowFromID(hWnd, iID);
  if(!szText || !*szText)
  {
    WinSetDlgItemText(hWnd, iID, "");
    return;
  }

  RECTL rc;
  WinQueryWindowRect(hWndStatic, &rc);

  int iStaticLength = rc.xRight - rc.xLeft;
  int iStringLength = getWindowStringLength(hWndStatic, szText);

  if(iStringLength <= iStaticLength)
  {
    WinSetDlgItemText(hWnd, iID, szText);
    return;
  }

  int iBreaks = iStringLength/iStaticLength;
  if(iBreaks <= 0)
    return;

  char * pBuf = new char[iStringLength + iBreaks + 1];
  if(pBuf == NULL)
    return;

  strcpy(pBuf, "");

  int iStart = 0;
  int iLines = 0;
  for(int i = 0; i < iStringLength; i++)
  {
    char * sz = &szText[iStart];
    int iIndex = i - iStart;
    char ch = sz[iIndex + 1];

    sz[iIndex + 1] = '\0';

    int iLength = getWindowStringLength(hWndStatic, sz);

    if(iLength < iStaticLength)
    {
      sz[iIndex + 1] = ch;
      if(iLines == iBreaks)
      {
        strcat(pBuf, sz);
        break;
      }
      continue;
    }

    sz[iIndex + 1] = ch;  // restore zeroed element
    i--;                  // go one step back

    ch = sz[iIndex];
    sz[iIndex] = '\0';    // terminate string one char shorter

    strcat(pBuf, sz);    // append the string
    strcat(pBuf, " ");   // append space character for successful wrapping

    iStart += strlen(sz);// shift new start position
    sz[iIndex] = ch;      // restore zeroed element
    iLines++;             // count lines
  }

  WinSetDlgItemText(hWnd, iID, pBuf);

  delete [] pBuf;
}
