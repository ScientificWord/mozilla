/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __XP_H__
#define __XP_H__

/**************************************************/
/*                                                */
/*                   Windows                      */
/*                                                */
/**************************************************/
#ifdef XP_WIN

#include <windows.h>
#include <io.h>
#include <stdio.h>

#include "npapi.h"
#include "npupp.h"

#define XP_HFILE HFILE

#endif //XP_WIN

/**************************************************/
/*                                                */
/*                     OS/2                       */
/*                                                */
/**************************************************/
#ifdef XP_OS2

#define INCL_PM
#define INCL_WIN

#include <os2.h>
#include <io.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>    /* stat() */
#include <sys/stat.h>

#include "npapi.h"
#include "npupp.h"

#define XP_HFILE FILE*

#endif //XP_OS2

/**************************************************/
/*                                                */
/*                    Unix                        */
/*                                                */
/**************************************************/
#ifdef XP_UNIX

#include <stdio.h>
#include <string.h>       /* strcat() */
#include <sys/types.h>    /* stat() */
#include <sys/stat.h>
#include <stdlib.h>       /* atoi() */
#include <assert.h>       /* assert() */
#include <ctype.h>        /* isprint() */

#include "npapi.h"
#include "npupp.h"

#define XP_HFILE FILE*

#define HIBYTE(i) (i >> 8)
#define LOBYTE(i) (i & 0xff)

#endif //XP_UNIX

/**************************************************/
/*                                                */
/*                     Mac                        */
/*                                                */
/**************************************************/
#ifdef XP_MAC

#include <Processes.h>
#include <Gestalt.h>
#include <CodeFragments.h>
#include <Timer.h>
#include <Resources.h>
#include <ToolUtils.h>

// A4Stuff.h contains the definition of EnterCodeResource and 
// EnterCodeResource, used for setting up the code resource�s
// globals for 68K (analagous to the function SetCurrentA5
// defined by the toolbox).
//
#include <A4Stuff.h>

#include "jri.h"
#include "npapi.h"

// The Mixed Mode procInfos defined in npupp.h assume Think C-
// style calling conventions.  These conventions are used by
// Metrowerks with the exception of pointer return types, which
// in Metrowerks 68K are returned in A0, instead of the standard
// D0. Thus, since NPN_MemAlloc and NPN_UserAgent return pointers,
// Mixed Mode will return the values to a 68K plugin in D0, but 
// a 68K plugin compiled by Metrowerks will expect the result in
// A0.  The following pragma forces Metrowerks to use D0 instead.
//
#ifdef __MWERKS__
#ifndef powerc
#pragma pointers_in_D0
#endif
#endif

#include "npupp.h"

#ifdef __MWERKS__
#ifndef powerc
#pragma pointers_in_A0
#endif
#endif

// The following fix for static initializers (which fixes a preious
// incompatibility with some parts of PowerPlant, was submitted by 
// Jan Ulbrich.
#ifdef __MWERKS__
	#ifdef __cplusplus
	extern "C" {
	#endif
		#ifndef powerc
			extern void	__InitCode__(void);
		#else
			extern void __sinit(void);
			#define __InitCode__ __sinit
		#endif
		extern void	__destroy_global_chain(void);
	#ifdef __cplusplus
	}
	#endif // __cplusplus
#endif // __MWERKS__

// Wrapper functions for all calls from Netscape to the plugin.
// These functions let the plugin developer just create the APIs
// as documented and defined in npapi.h, without needing to 
// install those functions in the function table or worry about
// setting up globals for 68K plugins.
NPError Private_Initialize(void);
void    Private_Shutdown(void);
NPError Private_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved);
NPError Private_Destroy(NPP instance, NPSavedData** save);
NPError Private_SetWindow(NPP instance, NPWindow* window);
NPError Private_NewStream(NPP instance, NPMIMEType type, NPStream* stream, NPBool seekable, uint16* stype);
NPError Private_DestroyStream(NPP instance, NPStream* stream, NPError reason);
int32   Private_WriteReady(NPP instance, NPStream* stream);
int32   Private_Write(NPP instance, NPStream* stream, int32 offset, int32 len, void* buffer);
void    Private_StreamAsFile(NPP instance, NPStream* stream, const char* fname);
void    Private_Print(NPP instance, NPPrint* platformPrint);
int16   Private_HandleEvent(NPP instance, void* event);
void    Private_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData);
jref    Private_GetJavaClass(void);
NPError Private_GetValue(NPP instance, NPPVariable variable, void *result);
NPError Private_SetValue(NPP instance, NPNVariable variable, void *value);

#define XP_HFILE FILE*

#endif //XP_MAC

/**************************************************/
/*                                                */
/*                     Misc                       */
/*                                                */
/**************************************************/
#if defined(XP_OS2)
typedef unsigned short WORD;
typedef char* LPSTR;
typedef unsigned long DWORD;
typedef void *LPVOID;

#define wsprintf sprintf
#define IDYES MBID_YES
#define MB_ICONERROR MB_ERROR
#endif

#if !defined(XP_WIN) && !defined(XP_OS2)

#ifndef FALSE
#define FALSE false
#endif

#ifndef TRUE
#define TRUE true
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif

#ifndef UINT
#define UINT unsigned int
#endif

#ifndef LPSTR
#define LPSTR char*
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef BOOL
#define BOOL bool
#endif

#if (defined XP_UNIX) /* For IRIX */
#ifndef bool
#define bool unsigned char
#endif
#endif

#endif //!XP_WIN && !XP_OS2

// File utilities
BOOL XP_IsFile(LPSTR szFileName);
XP_HFILE XP_CreateFile(LPSTR szFileName);
XP_HFILE XP_OpenFile(LPSTR szFileName);
void XP_CloseFile(XP_HFILE hFile);
void XP_DeleteFile(LPSTR szFileName);
DWORD XP_WriteFile(XP_HFILE hFile, void * pBuf, int iSize);
void XP_FlushFileBuffers(XP_HFILE hFile);

// Private profile utilities
DWORD XP_GetPrivateProfileString(LPSTR szSection, LPSTR szKey, LPSTR szDefault, LPSTR szString, DWORD dwSize, LPSTR szFileName);
int XP_GetPrivateProfileInt(LPSTR szSection, LPSTR szKey, int iDefault, LPSTR szFileName);
BOOL XP_WritePrivateProfileString(LPSTR szSection, LPSTR szKey, LPSTR szString, LPSTR szFileName);
BOOL XP_WritePrivateProfileInt(LPSTR szSection, LPSTR szKey, int iValue, LPSTR szFileName);

// Misc utilities
DWORD XP_GetTickCount();
void XP_Sleep(DWORD dwSleepTime);

#endif // __XP_H__
