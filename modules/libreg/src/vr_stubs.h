/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Daniel Veditz <dveditz@netscape.com>
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
/* vr_stubs.h
 *
 * XP code stubs for stand-alone registry library
 *
 */

#ifndef _VR_STUBS_H_
#define _VR_STUBS_H_

#ifdef STANDALONE_REGISTRY

#include <errno.h>
#include <string.h>
#ifdef XP_MAC
#include "macstdlibextras.h"  /* For strcasecmp and strncasecmp */
#include <extras.h>
#endif

#else

#include "prio.h"
#include "prlog.h"
#include "prmem.h"
#include "plstr.h"

#endif /* STANDALONE_REGISTRY*/

#ifdef XP_MAC
#include <stat.h>
#else
#if ( defined(BSDI) && !defined(BSDI_2) ) || defined(XP_OS2_EMX)
#include <sys/types.h>
#endif
#include <sys/stat.h>
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#if defined(__cplusplus)
# define XP_CPLUSPLUS
# define XP_IS_CPLUSPLUS 1
#else
# define XP_IS_CPLUSPLUS 0
#endif

#if defined(XP_CPLUSPLUS)
# define XP_BEGIN_PROTOS extern "C" {
# define XP_END_PROTOS }
#else
# define XP_BEGIN_PROTOS
# define XP_END_PROTOS
#endif


#ifdef STANDALONE_REGISTRY

#define USE_STDIO_MODES

#define XP_FileSeek(file,offset,whence) fseek((file), (offset), (whence))
#define XP_FileRead(dest,count,file)    fread((dest), 1, (count), (file))
#define XP_FileWrite(src,count,file)    fwrite((src), 1, (count), (file))
#define XP_FileTell(file)               ftell(file)
#define XP_FileFlush(file)              fflush(file)
#define XP_FileClose(file)              fclose(file)
#define XP_FileSetBufferSize(file,bufsize) (-1)

#define XP_ASSERT(x)        ((void)0)

#define XP_STRCAT(a,b)      strcat((a),(b))
#define XP_ATOI             atoi
#define XP_STRNCPY(a,b,n)   strncpy((a),(b),(n))
#define XP_STRCPY(a,b)      strcpy((a),(b))
#define XP_STRLEN(x)        strlen(x)
#define XP_SPRINTF          sprintf
#define XP_FREE(x)          free((x))
#define XP_ALLOC(x)         malloc((x))
#define XP_FREEIF(x)        if ((x)) free((x))
#define XP_STRCMP(x,y)      strcmp((x),(y))
#define XP_STRNCMP(x,y,n)   strncmp((x),(y),(n))
#define XP_STRDUP(s)        strdup((s))
#define XP_MEMCPY(d, s, l)  memcpy((d), (s), (l))
#define XP_MEMSET(d, c, l)  memset((d), (c), (l))

#define PR_Lock(a)          ((void)0)
#define PR_Unlock(a)        ((void)0)

#if defined(XP_WIN) || defined(XP_OS2)
  #define XP_STRCASECMP(x,y)  stricmp((x),(y))
  #define XP_STRNCASECMP(x,y,n) strnicmp((x),(y),(n))
#else
  #define XP_STRCASECMP(x,y)  strcasecmp((x),(y))
  #define XP_STRNCASECMP(x,y,n) strncasecmp((x),(y),(n))
#endif /* XP_WIN || XP_OS2 */

typedef FILE          * XP_File;

#else /* not standalone, use NSPR */


/*-------------------------------------*/
/* Alternate fileI/O function mappings */
/*-------------------------------------*/

#if USE_MMAP_REGISTRY_IO
  /*-----------------------------------------------*/
  /* NSPR mememory-mapped I/O (write through)      */
  /* unfortunately this isn't supported on the Mac */
  /*-----------------------------------------------*/
#define USE_NSPR_MODES

#include "mmapio.h"
#define XP_FileSeek(file,offset,whence) mmio_FileSeek((file),(offset),(whence))
#define XP_FileRead(dest,count,file)    mmio_FileRead((file), (dest), (count))
#define XP_FileWrite(src,count,file)    mmio_FileWrite((file), (src), (count))
#define XP_FileTell(file)               mmio_FileTell(file)
#define XP_FileClose(file)              mmio_FileClose(file)
#define XP_FileOpen(path, mode)         mmio_FileOpen((path), mode )
#define XP_FileFlush(file)              ((void)1)
#define XP_FileSetBufferSize(file, bufsize) (-1)

typedef MmioFile* XP_File;

#elif USE_BUFFERED_REGISTRY_IO
  /*-----------------------------------------------*/
  /* home-grown XP buffering                       */
  /* writes are buffered too so use flush!         */
  /*-----------------------------------------------*/
#define USE_STDIO_MODES

#include "nr_bufio.h"
#define XP_FileSeek(file,offset,whence) bufio_Seek((file),(offset),(whence))
#define XP_FileRead(dest,count,file)    bufio_Read((file), (dest), (count))
#define XP_FileWrite(src,count,file)    bufio_Write((file), (src), (count))
#define XP_FileTell(file)               bufio_Tell(file)
#define XP_FileClose(file)              bufio_Close(file)
#define XP_FileOpen(path, mode)         bufio_Open((path), (mode))
#define XP_FileFlush(file)              bufio_Flush(file)
#define XP_FileSetBufferSize(file,bufsize) bufio_SetBufferSize(file,bufsize)


typedef BufioFile* XP_File;

#else
  /*-----------------------------------------------*/
  /* standard NSPR file I/O                        */
  /*-----------------------------------------------*/
#define USE_NSPR_MODES
/*
** Note that PR_Seek returns the offset (if successful) and -1 otherwise.  So
** to make this code work
**           if (XP_FileSeek(fh, offset, SEEK_SET) != 0)  { error handling }
** we return 1 if PR_Seek() returns a negative value, and 0 otherwise
*/
#define XP_FileSeek(file,offset,whence) (PR_Seek((file), (offset), (whence)) < 0)
#define XP_FileRead(dest,count,file)    PR_Read((file), (dest), (count))
#define XP_FileWrite(src,count,file)    PR_Write((file), (src), (count))
#define XP_FileTell(file)               PR_Seek(file, 0, PR_SEEK_CUR)
#define XP_FileOpen(path, mode)         PR_Open((path), mode )
#define XP_FileClose(file)              PR_Close(file)
#define XP_FileFlush(file)              PR_Sync(file)
#define XP_FileSetBufferSize(file,bufsize) (-1)

typedef PRFileDesc* XP_File;

#endif /*USE_MMAP_REGISTRY_IO*/



#define XP_ASSERT(x)        PR_ASSERT((x))

#define XP_STRCAT(a,b)      PL_strcat((a),(b))
#define XP_ATOI             PL_atoi
#define XP_STRCPY(a,b)      PL_strcpy((a),(b))
#define XP_STRNCPY(a,b,n)   PL_strncpy((a),(b),(n))
#define XP_STRLEN(x)        PL_strlen(x)
#define XP_SPRINTF          sprintf
#define XP_FREE(x)          PR_Free((x))
#define XP_ALLOC(x)         PR_Malloc((x))
#define XP_FREEIF(x)        PR_FREEIF(x)
#define XP_STRCMP(x,y)      PL_strcmp((x),(y))
#define XP_STRNCMP(x,y,n)   PL_strncmp((x),(y),(n))
#define XP_STRDUP(s)        PL_strdup((s))
#define XP_MEMCPY(d, s, l)  memcpy((d), (s), (l))
#define XP_MEMSET(d, c, l)  memset((d), (c), (l))

#define XP_STRCASECMP(x,y)  PL_strcasecmp((x),(y))
#define XP_STRNCASECMP(x,y,n) PL_strncasecmp((x),(y),(n))


#endif /*STANDALONE_REGISTRY*/

/*--- file open modes for stdio ---*/
#ifdef USE_STDIO_MODES
#define XP_FILE_READ             "r"
#define XP_FILE_READ_BIN         "rb"
#define XP_FILE_WRITE            "w"
#define XP_FILE_WRITE_BIN        "wb"
#define XP_FILE_UPDATE           "r+"
#define XP_FILE_TRUNCATE         "w+"
#ifdef SUNOS4
/* XXX SunOS4 hack -- make this universal by using r+b and w+b */
#define XP_FILE_UPDATE_BIN       "r+"
#define XP_FILE_TRUNCATE_BIN     "w+"
#else
#define XP_FILE_UPDATE_BIN       "rb+"
#define XP_FILE_TRUNCATE_BIN     "wb+"
#endif
#endif /* USE_STDIO_MODES */

/*--- file open modes for NSPR file I/O ---*/
#ifdef USE_NSPR_MODES
#define XP_FILE_READ             PR_RDONLY, 0644
#define XP_FILE_READ_BIN         PR_RDONLY, 0644
#define XP_FILE_WRITE            PR_WRONLY, 0644
#define XP_FILE_WRITE_BIN        PR_WRONLY, 0644
#define XP_FILE_UPDATE           (PR_RDWR|PR_CREATE_FILE), 0644
#define XP_FILE_TRUNCATE         (PR_RDWR | PR_TRUNCATE), 0644
#define XP_FILE_UPDATE_BIN       PR_RDWR|PR_CREATE_FILE, 0644
#define XP_FILE_TRUNCATE_BIN     (PR_RDWR | PR_TRUNCATE), 0644

#ifdef SEEK_SET
    #undef SEEK_SET
    #undef SEEK_CUR
    #undef SEEK_END
    #define SEEK_SET PR_SEEK_SET
    #define SEEK_CUR PR_SEEK_CUR
    #define SEEK_END PR_SEEK_END
#endif
#endif /* USE_NSPR_MODES */





#ifdef STANDALONE_REGISTRY /* included from prmon.h otherwise */
#include "prtypes.h"
#endif /*STANDALONE_REGISTRY*/

typedef int XP_Bool;

typedef struct stat    XP_StatStruct;
#define  XP_Stat(file,data)     stat((file),(data))

#if defined(XP_MAC)
 extern int nr_RenameFile(char *from, char *to);
#else
    XP_BEGIN_PROTOS
    #define nr_RenameFile(from, to)    rename((from), (to))
    XP_END_PROTOS
#endif



XP_BEGIN_PROTOS

extern char* globalRegName;
extern char* verRegName;

extern void vr_findGlobalRegName();
extern char* vr_findVerRegName();


#ifdef STANDALONE_REGISTRY /* included from prmon.h otherwise */

extern XP_File vr_fileOpen(const char *name, const char * mode);


#else
#define vr_fileOpen PR_Open
#endif /* STANDALONE_REGISTRY */

XP_END_PROTOS

#endif /* _VR_STUBS_H_ */
