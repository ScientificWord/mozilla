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
/* ====================================================================
 * reg.c
 * XP Registry functions
 * ====================================================================
 */

/* TODO:
 *  - Replace 'malloc' in NR_RegPack with the Netscape XP equivalent
 *  - Solve DOS 'errno' problem mentioned below
 *  - Solve rename across volume problem described in VR_PackRegistry
 */

/* Preprocessor Defines
 *  STANDALONE_REGISTRY - define if not linking with Navigator
 *  NOCACHE_HDR         - define if multi-process access to registry
 *  SELF_REPAIR         - undefine to skip header update on open
 *  VERIFY_READ         - define TRUE to double-check short reads
 *
#define NOCACHE_HDR     1
 */
#define SELF_REPAIR     1
#ifdef DEBUG
#define VERIFY_READ     1
#endif

#include <stdio.h>
#include <string.h>

#ifdef STANDALONE_REGISTRY
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#if defined(XP_MAC) || defined(XP_MACOSX)
  #include <Errors.h>
#endif

#else

#include "prtypes.h"
#include "prlog.h"
#include "prerror.h"
#include "prprf.h"

#endif /*STANDALONE_REGISTRY*/

#if defined(SUNOS4)
#include <unistd.h>  /* for SEEK_SET */
#endif /* SUNOS4 */

#include "reg.h"
#include "NSReg.h"

#if defined(XP_MAC)
#define MAX_PATH 512
#elif defined(XP_MACOSX)
#define MAX_PATH PATH_MAX
#elif defined(XP_UNIX)
#ifndef MAX_PATH
#define MAX_PATH 1024
#endif
#elif defined(XP_OS2)
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#elif defined(WIN32)
#define MAX_PATH _MAX_PATH
#elif defined(XP_BEOS)
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

 
 /* NOTE! It is EXREMELY important that node names be in UTF-8; otherwise
 * backwards path search for delim char will fail for multi-byte/Unicode names
 */

/* ====================================================================
 * Overview
 * --------------------------------------------------------------------
 *
 *  Layers:
 *      Interface
 *          Path Parsing
 *              Key/Entry Management
 *                  Block I/O
 *                      Virtual I/O
 *
 * The functions in this file search and add to a binary Registry file
 * quite efficiently.  So efficiently, squeezing out space left by
 * deleted and updated objects requires a separate "pack" operation.
 *
 * Terms:
 * As used here, a 'key' is a node in the tree. The root of the tree
 * exists in an otherwise empty Registry as is itself a key.  Every key
 * has 0 or more sub-keys. Every key also has 0 or more 'entry's. Both
 * entries and keys have names. Entries also have values associated.
 * Names and values are simply strings of characters. These strings
 * may be quoted so that they can include path delimiter and equals
 * sign characters which are otherwise reserved.
 * ====================================================================
 */

/* --------------------------------------------------------------------
 * Module Global Data
 *
 * use of this data must be protected by the reglist lock
 * --------------------------------------------------------------------
 */

#if !defined(STANDALONE_REGISTRY)
static PRLock   *reglist_lock = NULL;
#endif

static REGFILE  *RegList = NULL;
static int32    regStartCount = 0;
char            *globalRegName = NULL;
static char     *user_name = NULL;




#if defined(XP_MAC) || defined(XP_MACOSX)

void nr_MacAliasFromPath(const char * fileName, void ** alias, int32 * length);
char * nr_PathFromMacAlias(const void * alias, uint32 aliasLength);

#include <Aliases.h>
#include <TextUtils.h>
#include <Memory.h>
#include <Folders.h>

#ifdef XP_MACOSX
  #include "MoreFilesX.h"
#else
  #include "FullPath.h"
#endif

static void copyCStringToPascal(Str255 dest, const char *src)
{
    size_t copyLen = strlen(src);
    if (copyLen > 255)
        copyLen = 255;
    BlockMoveData(src, &dest[1], copyLen);
    dest[0] = copyLen;
}

#ifdef XP_MACOSX
static OSErr isFileInTrash(FSRef *fsRef, PRBool *inTrash)
{
    OSErr err;
    FSCatalogInfo catalogInfo;

    if (fsRef == NULL || inTrash == NULL)
        return paramErr;
    *inTrash = PR_FALSE;

    err = FSGetCatalogInfo(fsRef, kFSCatInfoVolume, &catalogInfo, NULL, NULL, NULL);
    if (err == noErr)
    {
        FSRef trashFSRef, currFSRef, parentFSRef;
        err = FSFindFolder(catalogInfo.volume, kTrashFolderType, false, &trashFSRef);
        if (err == noErr)
        {
            /* FSRefGetParentRef returns noErr and a zeroed FSRef when it reaches the top */
            for (currFSRef = *fsRef;
                 (FSGetParentRef(&currFSRef, &parentFSRef) == noErr && FSRefValid(&parentFSRef));
                 currFSRef = parentFSRef)
            {
                if (FSCompareFSRefs(&parentFSRef, &trashFSRef) == noErr)
                {
                    *inTrash = PR_TRUE;
                    break;
                }
            }
        }
    }
    return err;
}
#else
static OSErr isFileInTrash(FSSpec *fileSpec, PRBool *inTrash)
{
    OSErr err;
    short vRefNum;
    long dirID;
    
    if (fileSpec == NULL || inTrash == NULL)
        return paramErr;
    *inTrash = PR_FALSE;
    
    /* XXX - Only works if the file is in the top level of the trash dir */
    err = FindFolder(fileSpec->vRefNum, kTrashFolderType, false, &vRefNum, &dirID);
    if (err == noErr)
        if (dirID == fileSpec->parID)  /* File is inside the trash */
            *inTrash = PR_TRUE;
    
    return err;
}
#endif

/* returns an alias as a malloc'd pointer.
 * On failure, *alias is NULL
 */
void nr_MacAliasFromPath(const char * fileName, void ** alias, int32 * length)
{
    OSErr err;
    Str255 pascalName;
    FSRef fsRef;
    FSSpec fs;
    AliasHandle macAlias;
    *alias = NULL;
    *length = 0;
    
#ifdef XP_MACOSX
    err = FSPathMakeRef((const UInt8*)fileName, &fsRef, NULL);
    if ( err != noErr )
        return;
    err = FSNewAlias(NULL, &fsRef, &macAlias);
#else
    copyCStringToPascal(pascalName, fileName);
    err = FSMakeFSSpec(0, 0, pascalName, &fs);
    if ( err != noErr )
        return;
    err = NewAlias(NULL, &fs, &macAlias);
#endif
    
    if ( (err != noErr) || ( macAlias == NULL ))
        return;
    *length = GetHandleSize( (Handle) macAlias );
    *alias = XP_ALLOC( *length );
    if ( *alias == NULL )
    {
        DisposeHandle((Handle)macAlias);
        return;
    }
    HLock( (Handle) macAlias );
    XP_MEMCPY(*alias, *macAlias , *length);
    HUnlock( (Handle) macAlias );
    DisposeHandle( (Handle) macAlias);
    return;
}

/* resolves an alias, and returns a full path to the Mac file
 * If the alias changed, it would be nice to update our alias pointers
 */
char * nr_PathFromMacAlias(const void * alias, uint32 aliasLength)
{
    OSErr           err;
    AliasHandle     h           = NULL;
    Handle          fullPath    = NULL;
    short           fullPathLength;
    char *          cpath       = NULL;
    PRBool          inTrash;
    FSRef           fsRef;
    FSCatalogInfo   catalogInfo;
    UInt8           pathBuf[MAX_PATH];
    FSSpec          fs;
    Boolean         wasChanged; /* Change flag, it would be nice to change the alias on disk 
                        if the file location changed */
    
    
    XP_MEMSET( &fs, '\0', sizeof(FSSpec) );
    
    
    /* Copy the alias to a handle and resolve it */
    h = (AliasHandle) NewHandle(aliasLength);
    if ( h == NULL)
        goto fail;
        
        
    HLock( (Handle) h);
    XP_MEMCPY( *h, alias, aliasLength );
    HUnlock( (Handle) h);
    
#ifdef XP_MACOSX
    err = FSResolveAlias(NULL, h, &fsRef, &wasChanged);
    if (err != noErr)
        goto fail;

    /* if the alias has changed and the file is now in the trash,
       assume that user has deleted it and that we do not want to look at it */
    if (wasChanged && (isFileInTrash(&fsRef, &inTrash) == noErr) && inTrash)
        goto fail;
    err = FSRefMakePath(&fsRef, pathBuf, sizeof(pathBuf));
    if (err != noErr)
        goto fail;
    fullPathLength = XP_STRLEN(pathBuf);
    cpath = (char*) XP_ALLOC(fullPathLength + 1);
    if ( cpath == NULL)
        goto fail;
    XP_MEMCPY(cpath, pathBuf, fullPathLength + 1);
#else    
    err = ResolveAlias(NULL, h, &fs, &wasChanged);
    if (err != noErr)
        goto fail;
    
    /* if the alias has changed and the file is now in the trash,
       assume that user has deleted it and that we do not want to look at it */
    if (wasChanged && (isFileInTrash(&fs, &inTrash) == noErr) && inTrash)
        goto fail;
    
    /* Get the full path and create a char * out of it */

    err = GetFullPath(fs.vRefNum, fs.parID,fs.name, &fullPathLength, &fullPath);
    if ( (err != noErr) || (fullPath == NULL) )
        goto fail;
    
    cpath = (char*) XP_ALLOC(fullPathLength + 1);
    if ( cpath == NULL)
        goto fail;
    
    HLock( fullPath );
    XP_MEMCPY(cpath, *fullPath, fullPathLength);
    cpath[fullPathLength] = 0;
    HUnlock( fullPath );
#endif    
    /* Drop through */
fail:
    if (h != NULL)
        DisposeHandle( (Handle) h);
    if (fullPath != NULL)
        DisposeHandle( fullPath);
    return cpath;
}

#endif


/* --------------------------------------------------------------------
 * Registry List management
 * --------------------------------------------------------------------
 */
static void nr_AddNode(REGFILE* pReg);
static void nr_DeleteNode(REGFILE *pReg);
static REGFILE* vr_findRegFile(const char *filename);

/* -------------------------------------------------------------------- */

static void nr_AddNode(REGFILE* pReg)
{
    /* add node to head of list */
    pReg->next = RegList;
    pReg->prev = NULL;

    RegList = pReg;

    if ( pReg->next != NULL ) {
        pReg->next->prev = pReg;
    }
}

static void nr_DeleteNode(REGFILE* pReg)
{
    /* if at head of list... */
    if ( pReg->prev == NULL ) {
        RegList = pReg->next;
    }
    else {
        pReg->prev->next = pReg->next;
    }

    if ( pReg->next != NULL ) {
        pReg->next->prev = pReg->prev;
    }

    /* free memory */
#ifndef STANDALONE_REGISTRY
    if ( pReg->lock != NULL )
        PR_DestroyLock( pReg->lock );
#endif
    XP_FREEIF( pReg->filename );
    XP_FREE( pReg );
}

static REGFILE* vr_findRegFile(const char *filename)
{
    REGFILE *pReg;

    pReg = RegList;
    while( pReg != NULL ) {
#if defined(XP_UNIX) && !defined(XP_MACOSX) || defined XP_BEOS
        if ( 0 == XP_STRCMP( filename, pReg->filename ) ) {
#else
        if ( 0 == XP_STRCASECMP( filename, pReg->filename ) ) {
#endif
            break;
        }
        pReg = pReg->next;
    }

    return pReg;
}


/* --------------------------------------------------------------------
 * Virtual I/O
 *  Platform-specifics go in this section
 * --------------------------------------------------------------------
 */
static REGERR nr_OpenFile(const char *path, FILEHANDLE *fh);
static REGERR nr_CloseFile(FILEHANDLE *fh); /* Note: fh is a pointer */
static REGERR nr_ReadFile(FILEHANDLE fh, REGOFF offset, int32 len, void *buffer);
static REGERR nr_WriteFile(FILEHANDLE fh, REGOFF offset, int32 len, void *buffer);
static REGERR nr_LockRange(FILEHANDLE fh, REGOFF offset, int32 len);
static REGERR nr_UnlockRange(FILEHANDLE fh, REGOFF offset, int32 len);
static int32  nr_GetFileLength(FILEHANDLE fh);
/* -------------------------------------------------------------------- */

#ifdef STANDALONE_REGISTRY
static REGERR nr_OpenFile(const char *path, FILEHANDLE *fh)
{
    XP_ASSERT( path != NULL );
    XP_ASSERT( fh != NULL );

    /* Open the file for exclusive random read/write */
    (*fh) = vr_fileOpen(path, XP_FILE_UPDATE_BIN);
    if ( !VALID_FILEHANDLE(*fh) )
    {
        switch (errno)
        {
#if defined(XP_MAC) || defined(XP_MACOSX)
        case fnfErr:
#else
        case ENOENT:    /* file not found */
#endif
            return REGERR_NOFILE;

#if defined(XP_MAC) || defined(XP_MACOSX)
        case opWrErr:
#else
        case EROFS:     /* read-only file system */
        case EACCES:    /* file in use or read-only file*/
#endif
            /* try read only */
            (*fh) = vr_fileOpen(path, XP_FILE_READ_BIN);
            if ( VALID_FILEHANDLE(*fh) )
                return REGERR_READONLY;
            else
                return REGERR_FAIL;

        default:
            return REGERR_FAIL;
        }
    }

    return REGERR_OK;

}   /* OpenFile */
#else
static REGERR nr_OpenFile(const char *path, FILEHANDLE *fh)
{
    PR_ASSERT( path != NULL );
    PR_ASSERT( fh != NULL );

    /* Open the file for exclusive random read/write */
    *fh = XP_FileOpen(path, XP_FILE_UPDATE_BIN);
    if ( !VALID_FILEHANDLE(*fh) )
    {
        XP_StatStruct st;
        if ( XP_Stat( path, &st ) != 0 )
        {
            /* file doesn't exist, so create */
            *fh = XP_FileOpen(path, XP_FILE_TRUNCATE_BIN);
        }
    }

    if ( !VALID_FILEHANDLE(*fh) )
    {
      /* For whatever reason we failed every attempt of getting */
      /* a read/write registry. Let's try a read-only registry. */
      (*fh) = XP_FileOpen(path, XP_FILE_READ_BIN);
      if ( VALID_FILEHANDLE(*fh) )
        return REGERR_READONLY;
      else
        /* we are in big trouble now */
        return REGERR_FAIL;
    }

    /* succeded in getting a read/write registry */
    return REGERR_OK;

}   /* OpenFile */
#endif


static REGERR nr_CloseFile(FILEHANDLE *fh)
{
    /* NOTE: 'fh' is a pointer, unlike other Close functions
     *       This is necessary so that nr_CloseFile can set it to NULL
     */

    XP_ASSERT( fh != NULL );
    if ( VALID_FILEHANDLE(*fh) )
        XP_FileClose(*fh);
    (*fh) = NULL;
    return REGERR_OK;

}   /* CloseFile */



static REGERR nr_ReadFile(FILEHANDLE fh, REGOFF offset, int32 len, void *buffer)
{
#if VERIFY_READ
    #define        FILLCHAR  0xCC
    unsigned char* p;
    unsigned char* dbgend = (unsigned char*)buffer+len;
#endif

    int32 readlen;
    REGERR err = REGERR_OK;

    XP_ASSERT(len > 0);
    XP_ASSERT(buffer != NULL);
    XP_ASSERT(fh != NULL);

#if VERIFY_READ
    XP_MEMSET(buffer, FILLCHAR, len);
#endif

    if (XP_FileSeek(fh, offset, SEEK_SET) != 0 ) {
        err = REGERR_FAIL;
    }
    else {
        readlen = XP_FileRead(buffer, len, fh );
        /* PR_READ() returns an unreliable length, check EOF separately */
        if (readlen < 0) {
#if !defined(STANDALONE_REGISTRY) || (!defined(XP_MAC) && !defined(XP_MACOSX))
    #if defined(STANDALONE_REGISTRY)
            if (errno == EBADF) /* bad file handle, not open for read, etc. */
    #else
            if (PR_GetError() == PR_BAD_DESCRIPTOR_ERROR)
    #endif
                err = REGERR_FAIL;
            else
#endif
                err = REGERR_BADREAD;
        }
        else if (readlen < len) {
#if VERIFY_READ
            /* PR_READ() says we hit EOF but return length is unreliable. */
            /* If buffer has new data beyond what PR_READ() says it got */
            /* we'll assume the read was OK--this is a gamble but */
            /* missing errors will cause fewer problems than too many. */
            p = (unsigned char*)buffer + readlen;
            while ( (p < dbgend) && (*p == (unsigned char)FILLCHAR) ) {
                p++;
            }

            /* really was EOF if it's all FILLCHAR's */
            if ( p == dbgend ) {
                err = REGERR_BADREAD;
            }
#else
            err = REGERR_BADREAD;
#endif
        }
    }

    return err;

}   /* ReadFile */



static REGERR nr_WriteFile(FILEHANDLE fh, REGOFF offset, int32 len, void *buffer)
{

    /* Note: 'offset' will commonly be the end of the file, in which
     * case this function extends the file to 'offset'+'len'. This may
     * be a two-step operation on some platforms.
     */
    XP_ASSERT(len > 0);
    XP_ASSERT(buffer);
    XP_ASSERT(fh != NULL);

    if (XP_FileSeek(fh, offset, SEEK_SET) != 0)
        return REGERR_FAIL;

    if ((int32)XP_FileWrite(buffer, len, fh) != len)
    {
        /* disk full or some other catastrophic error */
        return REGERR_FAIL;
    }

    return REGERR_OK;

}   /* WriteFile */



static REGERR nr_LockRange(FILEHANDLE fh, REGOFF offset, int32 len)
{
    /* TODO: Implement XP lock function with built-in retry. */

    return REGERR_OK;

}   /* LockRange */



static REGERR nr_UnlockRange(FILEHANDLE fh, REGOFF offset, int32 len)
{
    /* TODO: Implement XP unlock function with built-in retry. */

    return REGERR_OK;

}   /* UnlockRange */



#if SELF_REPAIR
static int32 nr_GetFileLength(FILEHANDLE fh)
{
    int32 length;
    int32 curpos;

    curpos = XP_FileTell(fh);
    XP_FileSeek(fh, 0, SEEK_END);
    length = XP_FileTell(fh);
    XP_FileSeek(fh, curpos, SEEK_SET);
    return length;

}   /* GetFileLength */
#endif



/* --------------------------------------------------------------------
 * Numeric converters
 * --------------------------------------------------------------------
 * The converters read and write integers in a common format so we
 * can transport registries without worrying about endian problems.
 *
 * The buffers *MUST* be the appropriate size!
 * --------------------------------------------------------------------
 */
static uint32 nr_ReadLong(char *buffer);
static uint16 nr_ReadShort(char *buffer);
static void   nr_WriteLong(uint32 num, char *buffer);
static void   nr_WriteShort(uint16 num, char *buffer);
/* -------------------------------------------------------------------- */



static uint16 nr_ReadShort(char *buffer)
{
    uint16 val;
    uint8 *p = (uint8*)buffer;
 
    val = (uint16)(*p + (uint16)( *(p+1) * 0x100 ));

    return val;
}



static uint32 nr_ReadLong(char *buffer)
{
    uint32 val;
    uint8 *p = (uint8*)buffer;

    val = *p
        + (uint32)(*(p+1) * 0x100L)
        + (uint32)(*(p+2) * 0x10000L )
        + (uint32)(*(p+3) * 0x1000000L );

    return val;
}



static void  nr_WriteLong(uint32 num, char *buffer)
{
    uint8 *p = (uint8*)buffer;
    *p++ = (uint8)(num & 0x000000FF);
    num /= 0x100;
    *p++ = (uint8)(num & 0x000000FF);
    num /= 0x100;
    *p++ = (uint8)(num & 0x000000FF);
    num /= 0x100;
    *p   = (uint8)(num & 0x000000FF);
}



static void  nr_WriteShort(uint16 num, char *buffer)
{
    uint8 *p = (uint8*)buffer;

    *p = (uint8)(num & 0x00FF);
    *(p+1) = (uint8)(num / 0x100);
}



/* --------------------------------------------------------------------
 * Block I/O
 * --------------------------------------------------------------------
 */
static REGERR nr_ReadHdr(REGFILE *reg); /* Reads the file header, creates file if empty */
static REGERR nr_WriteHdr(REGFILE *reg);    /* Writes the file header */
static REGERR nr_CreateRoot(REGFILE *reg);

static REGERR nr_Lock(REGFILE *reg);
static REGERR nr_Unlock(REGFILE *reg);

static REGERR nr_ReadDesc(REGFILE *reg, REGOFF offset, REGDESC *desc);      /* reads a desc */
static REGERR nr_ReadName(REGFILE *reg, REGDESC *desc, uint32 buflen, char *buf);
static REGERR nr_ReadData(REGFILE *reg, REGDESC *desc, uint32 buflen, char *buf);

static REGERR nr_WriteDesc(REGFILE *reg, REGDESC *desc);                    /* writes a desc */
static REGERR nr_WriteString(REGFILE *reg, char *string, REGDESC *desc);    /* writes a string */
static REGERR nr_WriteData(REGFILE *reg, char *string, uint32 len, REGDESC *desc);  /* writes a string */

static REGERR nr_AppendDesc(REGFILE *reg, REGDESC *desc, REGOFF *result);   /* adds a desc */
static REGERR nr_AppendName(REGFILE *reg, char *name, REGDESC *desc);       /* adds a name */
static REGERR nr_AppendString(REGFILE *reg, char *string, REGDESC *desc);   /* adds a string */
static REGERR nr_AppendData(REGFILE *reg, char *string, uint32 len, REGDESC *desc); /* adds a string */

static XP_Bool nr_IsValidUTF8(char *string);    /* checks if a string is UTF-8 encoded */
/* -------------------------------------------------------------------- */



static REGERR nr_ReadHdr(REGFILE *reg)
{

    int err;
    long filelength;
    char hdrBuf[sizeof(REGHDR)];

    XP_ASSERT(reg);
    reg->hdrDirty = 0;

    err = nr_ReadFile(reg->fh, 0, sizeof(REGHDR), &hdrBuf);

    switch (err)
    {
    case REGERR_BADREAD:
        /* header doesn't exist, so create one */
        err = nr_CreateRoot(reg);
        break;

    case REGERR_OK:
        /* header read successfully -- convert */
        reg->hdr.magic    = nr_ReadLong ( hdrBuf + HDR_MAGIC );
        reg->hdr.verMajor = nr_ReadShort( hdrBuf + HDR_VERMAJOR );
        reg->hdr.verMinor = nr_ReadShort( hdrBuf + HDR_VERMINOR );
        reg->hdr.avail    = nr_ReadLong ( hdrBuf + HDR_AVAIL );
        reg->hdr.root     = nr_ReadLong ( hdrBuf + HDR_ROOT );

        /* check to see if it's the right file type */
        if (reg->hdr.magic != MAGIC_NUMBER) {
            err = REGERR_BADMAGIC;
            break;
        }

        /* Check registry version
         * If the major version is bumped we're incompatible
         * (minor version just means some new features were added)
         *
         * Upgrade code will go here in the future...
         */
        if ( reg->hdr.verMajor > MAJOR_VERSION ) {
            err = REGERR_REGVERSION;
            break;
        }

#if SELF_REPAIR
        if ( reg->inInit && !(reg->readOnly) ) {
            filelength = nr_GetFileLength(reg->fh);
            if (reg->hdr.avail != filelength)
            {
                reg->hdr.avail = filelength;
                reg->hdrDirty = 1;
#if NOCACHE_HDR
                err = nr_WriteHdr(reg);
#endif
            }
        }
#endif  /* SELF_REPAIR */
        break;

    default:
        /* unexpected error from nr_ReadFile()*/
        XP_ASSERT(FALSE);
        err = REGERR_FAIL;
        break;
    }   /* switch */

    return err;

}   /* ReadHdr */



static REGERR nr_WriteHdr(REGFILE *reg)
{
    REGERR err;
    char hdrBuf[sizeof(REGHDR)];

    XP_ASSERT(reg);

    if (reg->readOnly)
        return REGERR_READONLY;

    /* convert to XP int format */
    nr_WriteLong ( reg->hdr.magic,    hdrBuf + HDR_MAGIC );
    nr_WriteShort( reg->hdr.verMajor, hdrBuf + HDR_VERMAJOR );
    nr_WriteShort( reg->hdr.verMinor, hdrBuf + HDR_VERMINOR );
    nr_WriteLong ( reg->hdr.avail,    hdrBuf + HDR_AVAIL );
    nr_WriteLong ( reg->hdr.root,     hdrBuf + HDR_ROOT );

    /* err = nr_WriteFile(reg->fh, 0, sizeof(REGHDR), &reg->hdr); */
    err = nr_WriteFile(reg->fh, 0, sizeof(hdrBuf), &hdrBuf);

    if (err == REGERR_OK)
        reg->hdrDirty = 0;

    return err;

}   /* WriteHdr */



static REGERR nr_CreateRoot(REGFILE *reg)
{
    /* Called when an empty file is detected by ReadHdr */
    REGERR err;
    REGDESC root;

    XP_ASSERT(reg);

    /* Create 'hdr' */
    reg->hdr.magic      = MAGIC_NUMBER;
    reg->hdr.verMajor   = MAJOR_VERSION;
    reg->hdr.verMinor   = MINOR_VERSION;
    reg->hdr.root       = 0;
    reg->hdr.avail      = HDRRESERVE;

    /* Create root descriptor */
    root.location   = 0;
    root.left       = 0;
    root.value      = 0;
    root.down       = 0;
    root.type       = REGTYPE_KEY;
    root.valuelen   = 0;
    root.valuebuf   = 0;
    root.parent     = 0;

    err = nr_AppendName(reg, ROOTKEY_STR, &root);
    if (err != REGERR_OK)
        return err;

    err = nr_AppendDesc(reg, &root, &reg->hdr.root);
    if (err != REGERR_OK)
        return err;

    return nr_WriteHdr(reg);    /* actually commit to disk */

    /* Create standard top-level nodes */

}   /* CreateRoot */



static REGERR nr_Lock(REGFILE *reg)
{
    REGERR status;

    /* lock file */
    status = nr_LockRange(reg->fh, 0, sizeof(REGHDR));

    if (status == REGERR_OK)
    {
        /* lock the object */
        PR_Lock( reg->lock );

#if NOCACHE_HDR
        /* try to refresh header info */
        status = nr_ReadHdr(reg);
        if ( status != REGERR_OK ) {
            PR_Unlock( reg->lock );
        }
#endif
    }

    return status;
}   /* Lock */



static REGERR nr_Unlock(REGFILE *reg)
{
    PR_Unlock( reg->lock );

    return nr_UnlockRange(reg->fh, 0, sizeof(REGHDR));
}   /* Unlock */



static REGERR nr_ReadDesc(REGFILE *reg, REGOFF offset, REGDESC *desc)
{

    REGERR err;
    char descBuf[ DESC_SIZE ];

    XP_ASSERT(reg);
    XP_ASSERT(offset >= HDRRESERVE);
    XP_ASSERT(offset < reg->hdr.avail);
    XP_ASSERT(desc);

    err = nr_ReadFile(reg->fh, offset, DESC_SIZE, &descBuf);
    if (err == REGERR_OK)
    {
        desc->location  = nr_ReadLong ( descBuf + DESC_LOCATION );
        desc->name      = nr_ReadLong ( descBuf + DESC_NAME );
        desc->namelen   = nr_ReadShort( descBuf + DESC_NAMELEN );
        desc->type      = nr_ReadShort( descBuf + DESC_TYPE );
        desc->left      = nr_ReadLong ( descBuf + DESC_LEFT );
        desc->value     = nr_ReadLong ( descBuf + DESC_VALUE );
        desc->valuelen  = nr_ReadLong ( descBuf + DESC_VALUELEN );
        desc->parent    = nr_ReadLong ( descBuf + DESC_PARENT );

        if ( TYPE_IS_ENTRY(desc->type) ) {
            desc->down = 0;
            desc->valuebuf  = nr_ReadLong( descBuf + DESC_VALUEBUF );
        }
        else {  /* TYPE is KEY */
            desc->down      = nr_ReadLong( descBuf + DESC_DOWN );
            desc->valuebuf  = 0;
        }

        if (desc->location != offset)
            err = REGERR_BADLOCN;
        else if ( desc->type & REGTYPE_DELETED )
            err = REGERR_DELETED;
    }

    return err;

}   /* ReadDesc */



static REGERR nr_ReadName(REGFILE *reg, REGDESC *desc, uint32 buflen, char *buf)
{

    REGERR err;

    XP_ASSERT(reg);
    XP_ASSERT(desc->name > 0);
    XP_ASSERT(desc->name < reg->hdr.avail);
    XP_ASSERT(buflen > 0);
    XP_ASSERT(buf);

    if ( desc->namelen > buflen )
        return REGERR_BUFTOOSMALL;

    err = nr_ReadFile(reg->fh, desc->name, desc->namelen, buf);

    buf[buflen-1] = '\0';   /* avoid runaways */

    return err;

}   /* ReadName */



static REGERR nr_ReadData(REGFILE *reg, REGDESC *desc, uint32 buflen, char *buf)
{

    REGERR err;

    XP_ASSERT(reg);
    XP_ASSERT(desc->value > 0);
    XP_ASSERT(desc->value < reg->hdr.avail);
    XP_ASSERT(buflen > 0);
    XP_ASSERT(buf);

    if ( desc->valuelen > buflen )
        return REGERR_BUFTOOSMALL;

    err = nr_ReadFile(reg->fh, desc->value, desc->valuelen, buf);

    return err;

}   /* nr_ReadData */



static REGERR nr_WriteDesc(REGFILE *reg, REGDESC *desc)
{
    char descBuf[ DESC_SIZE ];

    XP_ASSERT(reg);
    XP_ASSERT(desc);
    XP_ASSERT( desc->location >= HDRRESERVE );
    XP_ASSERT( desc->location < reg->hdr.avail );

    if (reg->readOnly)
        return REGERR_READONLY;

    /* convert to XP int format */
    nr_WriteLong ( desc->location,  descBuf + DESC_LOCATION );
    nr_WriteLong ( desc->name,      descBuf + DESC_NAME );
    nr_WriteShort( desc->namelen,   descBuf + DESC_NAMELEN );
    nr_WriteShort( desc->type,      descBuf + DESC_TYPE );
    nr_WriteLong ( desc->left,      descBuf + DESC_LEFT );
    nr_WriteLong ( desc->value,     descBuf + DESC_VALUE );
    nr_WriteLong ( desc->valuelen,  descBuf + DESC_VALUELEN );
    nr_WriteLong ( desc->parent,    descBuf + DESC_PARENT );

    if ( TYPE_IS_ENTRY(desc->type) ) {
        XP_ASSERT( 0 == desc->down );
        nr_WriteLong( desc->valuebuf,  descBuf + DESC_VALUEBUF );
    }
    else {  /* TYPE is KEY */
        XP_ASSERT( 0 == desc->valuebuf );
        nr_WriteLong( desc->down,      descBuf + DESC_DOWN );
    }

    return nr_WriteFile(reg->fh, desc->location, DESC_SIZE, descBuf);
}   /* nr_WriteDesc */



static REGERR nr_AppendDesc(REGFILE *reg, REGDESC *desc, REGOFF *result)
{

    REGERR err;
    char descBuf[ DESC_SIZE ];

    XP_ASSERT(reg);
    XP_ASSERT(desc);
    XP_ASSERT(result);

    *result = 0;

    if (reg->readOnly)
        return REGERR_READONLY;

    desc->location = reg->hdr.avail;

    /* convert to XP int format */
    nr_WriteLong ( desc->location,  descBuf + DESC_LOCATION );
    nr_WriteLong ( desc->name,      descBuf + DESC_NAME );
    nr_WriteShort( desc->namelen,   descBuf + DESC_NAMELEN );
    nr_WriteShort( desc->type,      descBuf + DESC_TYPE );
    nr_WriteLong ( desc->left,      descBuf + DESC_LEFT );
    nr_WriteLong ( desc->value,     descBuf + DESC_VALUE );
    nr_WriteLong ( desc->valuelen,  descBuf + DESC_VALUELEN );
    nr_WriteLong ( desc->parent,    descBuf + DESC_PARENT );

    if ( TYPE_IS_ENTRY(desc->type) ) {
        XP_ASSERT( 0 == desc->down );
        nr_WriteLong( desc->valuebuf,  descBuf + DESC_VALUEBUF );
    }
    else {  /* TYPE is KEY */
        XP_ASSERT( 0 == desc->valuebuf );
        nr_WriteLong( desc->down,      descBuf + DESC_DOWN );
    }

    err = nr_WriteFile(reg->fh, reg->hdr.avail, DESC_SIZE, descBuf);

    if (err == REGERR_OK)
    {
        *result = reg->hdr.avail;
        reg->hdr.avail += DESC_SIZE;
        reg->hdrDirty = 1;
#if NOCACHE_HDR
        err = nr_WriteHdr(reg);
#endif
    }

    return err;

}   /* AppendDesc */



static REGERR nr_AppendName(REGFILE *reg, char *name, REGDESC *desc)
{
    REGERR err;
    int len;
    char *p;

    XP_ASSERT(reg);
    XP_ASSERT(name);
    XP_ASSERT(desc);

    if (!nr_IsValidUTF8(name))
        return REGERR_BADUTF8;
    if (reg->readOnly)
        return REGERR_READONLY;

    len = XP_STRLEN(name) + 1;

    /* check for valid name parameter */
    if ( len == 1 )
        return REGERR_PARAM;

    if ( len > MAXREGNAMELEN )
        return REGERR_NAMETOOLONG;

    for ( p = name; (*p != 0); p++ ) {
        if ( INVALID_NAME_CHAR(*p) )
            return REGERR_BADNAME;
    }

    /* save the name */
    err = nr_WriteFile(reg->fh, reg->hdr.avail, len, name);

    /* if write successful update the desc and hdr */
    if (err == REGERR_OK)
    {
        desc->namelen = (uint16)len;
        desc->name = reg->hdr.avail;
        reg->hdr.avail += len;
        reg->hdrDirty = 1;
#if NOCACHE_HDR
        err = nr_WriteHdr(reg);
#endif
    }

    return err;

}   /* nr_AppendName */



static REGERR nr_WriteString(REGFILE *reg, char *string, REGDESC *desc)
{
    uint32 len;

    XP_ASSERT(string);
    if (!nr_IsValidUTF8(string))
        return REGERR_BADUTF8;
    if (reg->readOnly)
        return REGERR_READONLY;
    len = XP_STRLEN(string) + 1;

    return nr_WriteData( reg, string, len, desc );

}   /* nr_WriteString */



static REGERR nr_WriteData(REGFILE *reg, char *string, uint32 len, REGDESC *desc)
{
    REGERR err;

    XP_ASSERT(reg);
    XP_ASSERT(string);
    XP_ASSERT(desc);

    if (reg->readOnly)
        return REGERR_READONLY;

    if ( len == 0 )
        return REGERR_PARAM;

    if ( len > MAXREGVALUELEN )
        return REGERR_NAMETOOLONG;

    /* save the data in the same place if it fits */
    if ( len <= desc->valuebuf ) {
        err = nr_WriteFile( reg->fh, desc->value, len, string );
        if ( err == REGERR_OK ) {
            desc->valuelen = len;
        }
    }
    else {
        /* otherwise append new data */
        err = nr_AppendData( reg, string, len, desc );
    }

    return err;

}   /* nr_WriteData */



static REGERR nr_AppendString(REGFILE *reg, char *string, REGDESC *desc)
{
    uint32 len;

    XP_ASSERT(string);
    if (!nr_IsValidUTF8(string))
        return REGERR_BADUTF8;
    if (reg->readOnly)
        return REGERR_READONLY;
    len = XP_STRLEN(string) + 1;

    return nr_AppendData( reg, string, len, desc );

}   /* nr_AppendString */



static REGERR nr_AppendData(REGFILE *reg, char *string, uint32 len, REGDESC *desc)
{
    REGERR err;

    XP_ASSERT(reg);
    XP_ASSERT(string);
    XP_ASSERT(desc);

    if (reg->readOnly)
        return REGERR_READONLY;

    if ( len == 0 )
        return REGERR_PARAM;

    if ( len > MAXREGVALUELEN )
        return REGERR_NAMETOOLONG;

    /* save the string */
    err = nr_WriteFile(reg->fh, reg->hdr.avail, len, string);
    if (err == REGERR_OK)
    {
        desc->value     = reg->hdr.avail;
        desc->valuelen  = len;
        desc->valuebuf  = len;

        reg->hdr.avail += len;
        reg->hdrDirty   = 1;
#if NOCACHE_HDR
        err = nr_WriteHdr(reg);
#endif
    }

    return err;

}   /* nr_AppendData */

static XP_Bool nr_IsValidUTF8(char *string)
{
    int follow = 0;
    char *c;
    unsigned char ch;

    XP_ASSERT(string);
    if ( !string )
        return FALSE;

    for ( c = string; *c != '\0'; c++ )
    {
        ch = (unsigned char)*c;
        if( follow == 0 )
        {
            /* expecting an initial byte */
            if ( ch <= 0x7F )
            {
                /* standard byte -- do nothing */
            }
            else if ((0xC0 & ch) == 0x80)
            {
                /* follow byte illegal here */
                return FALSE;
            }
            else if ((0xE0 & ch) == 0xC0)
            {
                follow = 1;
            }
            else if ((0xF0 & ch) == 0xE0)
            {
                follow = 2;
            }
            else
            { 
                /* unexpected (unsupported) initial byte */
                return FALSE;
            }
        }
        else 
        {
            XP_ASSERT( follow > 0 );
            if ((0xC0 & ch) == 0x80)
            {
                /* expecting follow byte and found one */
                follow--;
            }
            else 
            {
                /* invalid state */
                return FALSE;
            }
        }
    } /* for */

    if ( follow != 0 )
    {
        /* invalid state -- interrupted character */
        return FALSE;
    }
    
    return TRUE;
}   /* checks if a string is UTF-8 encoded */

/* --------------------------------------------------------------------
 * Path Parsing
 * --------------------------------------------------------------------
 */
static REGERR nr_NextName(const char *pPath, char *buf, uint32 bufsize, const char **newPath);
static REGERR nr_RemoveName(char *path);
static REGERR nr_CatName(REGFILE *reg, REGOFF node, char *path, uint32 bufsize,
                    REGDESC *desc);
static REGERR nr_ReplaceName(REGFILE *reg, REGOFF node, char *path,
                    uint32 bufsize, REGDESC *desc);
/* -------------------------------------------------------------------- */


/* Scans path at 'pPath' and copies next name segment into 'buf'.
 * Also sets 'newPath' to point at the next segment of pPath.
 */
static REGERR nr_NextName(const char *pPath, char *buf, uint32 bufsize, const char **newPath)
{
    uint32 len = 0;
    REGERR err = REGERR_OK;

    /* initialization and validation */
    XP_ASSERT(buf);

    *newPath = NULL;
    *buf = '\0';

    if ( pPath==NULL || *pPath=='\0' )
        return REGERR_NOMORE;

    /* ... skip an initial path delimiter */
    if ( *pPath == PATHDEL ) {
        pPath++;

        if ( *pPath == '\0' )
            return REGERR_NOMORE;
    }

    /* ... missing name segment or initial blank are errors*/
    if ( *pPath == PATHDEL || *pPath == ' ' )
        return REGERR_BADNAME;

    /* copy first path segment into return buf */
    while ( *pPath != '\0' && *pPath != PATHDEL )
    {
        if ( len == bufsize ) {
            err = REGERR_NAMETOOLONG;
            break;
        }
        if ( *pPath < ' ' && *pPath > 0 )
            return REGERR_BADNAME;

        *buf++ = *pPath++;
        len++;
    }
    *buf = '\0';

    /* ... name segment can't end with blanks, either */
    if ( ' ' == *(buf-1) )
        return REGERR_BADNAME;

    /* return a pointer to the start of the next segment */
    *newPath = pPath;

    return err;

}   /* nr_NextName */




static REGERR nr_CatName(REGFILE *reg, REGOFF node, char *path, uint32 bufsize, REGDESC *desc)
{
    REGERR err = REGERR_OK;

    char   *p;
    uint32 len = XP_STRLEN(path);

    if (len > 0)
    {
        p = &path[len-1];
        if (*p != PATHDEL)
        {
            if ( len < bufsize ) {
                p++;
                *p = PATHDEL;
                len++;
            }
            else
                err = REGERR_BUFTOOSMALL;
        }
        p++;    /* point one past PATHDEL */
    }
    else
        p = path;

    if ( err == REGERR_OK ) {
        err = nr_ReadDesc( reg, node, desc );
        if ( err == REGERR_OK ) {
            err = nr_ReadName( reg, desc, bufsize-len, p );
        }
    }

    return err;

}   /* CatName */



static REGERR nr_ReplaceName(REGFILE *reg, REGOFF node, char *path, uint32 bufsize, REGDESC *desc)
{
    /* NOTE! It is EXREMELY important that names be in UTF-8; otherwise
     * the backwards path search will fail for multi-byte/Unicode names
     */

    char   *p;
    uint32 len;
    REGERR err;

    XP_ASSERT(path);

    len = XP_STRLEN(path);
    if ( len > bufsize )
        return REGERR_PARAM;

    if ( len > 0 ) {
        p = &path[len-1];

        while ((p > path) && (*p != PATHDEL)) {
            --p;
            --len;
        }
        if ( *p == PATHDEL ) {
            p++; 
            len++;
        }
    }
    else
        p = path;


    err = nr_ReadDesc( reg, node, desc );
    if ( err == REGERR_OK ) {
        err = nr_ReadName( reg, desc, bufsize-len, p );
    }

    return err;

}   /* ReplaceName */


static REGERR nr_RemoveName(char *path)
{
    /* Typical inputs:
     * path = "/Machine/4.0/"   output = "/Machine"
     * path = "/Machine"        output = ""
     * path = ""                output = REGERR_NOMORE
     *
     * NOTE! It is EXREMELY important that names be in UTF-8; otherwise
     * the backwards path search will fail for multi-byte/Unicode names
     */

    int len = XP_STRLEN(path);
    char *p;
    if (len < 1)
        return REGERR_NOMORE;

    p = &path[len-1];
    /* if last char is '/', ignore it */
    if (*p == PATHDEL)
        p--;

    while ((p > path) && (*p != PATHDEL))
        p--;

/*  if (*p != PATHDEL)
        return REGERR_NOMORE;
*/

    *p = '\0';
    return REGERR_OK;

}   /* RemoveName */



/* --------------------------------------------------------------------
 * Key/Entry Management
 * --------------------------------------------------------------------
 */
static REGERR nr_Find(REGFILE *reg, REGOFF offParent, const char *pPath,
    REGDESC *pDesc, REGOFF *pPrev, REGOFF *pParent, XP_Bool raw);

static REGERR nr_FindAtLevel(REGFILE *reg, REGOFF offFirst, const char *pName,
    REGDESC *pDesc, REGOFF *pOffPrev);

static REGERR nr_CreateSubKey(REGFILE *reg, REGOFF parent, REGDESC *pDesc,
                              char *name);
static REGERR nr_CreateEntryString(REGFILE *reg, REGDESC *pParent, 
    char *name, char *value);
static REGERR nr_CreateEntry(REGFILE *reg, REGDESC *pParent, char *name,
    uint16 type, char *buffer, uint32 length);
/* -------------------------------------------------------------------- */



static REGERR nr_Find(REGFILE *reg,
            REGOFF offParent,
            const char *pPath,
            REGDESC *pDesc,
            REGOFF *pPrev,
            REGOFF *pParent,
            XP_Bool raw)
{

    REGERR  err;
    REGDESC desc;
    REGOFF  offPrev = 0;
    char    namebuf[MAXREGNAMELEN];
    const char    *p;

    XP_ASSERT( pPath != NULL );
    XP_ASSERT( offParent >= HDRRESERVE );
    XP_ASSERT( VALID_FILEHANDLE( reg->fh ) );

    if (pPrev)
        *pPrev = 0;
    if (pParent)
        *pParent = 0;

    /* read starting desc */
    err = nr_ReadDesc( reg, offParent, &desc);

    if (raw == TRUE) {
        if ( err == REGERR_OK ) {
            /* save current location as parent of next segment */
            offParent = desc.location;
            /* look for name at next level down */
            err = nr_FindAtLevel(reg, desc.down, pPath, &desc, &offPrev);
        }
    }
    else {
        /* Walk 'path', reading keys into 'desc' */
        p = pPath;
        while ( err == REGERR_OK ) 
        {
            err = nr_NextName(p, namebuf, sizeof(namebuf), &p);

            if ( err == REGERR_OK ) {
                /* save current location as parent of next segment */
                offParent = desc.location;
                /* look for name at next level down */
                err = nr_FindAtLevel(reg, desc.down, namebuf, &desc, &offPrev);
            }
        }
    }

    if ( (raw == FALSE && err == REGERR_NOMORE) ||
            (raw == TRUE && err == REGERR_OK) ) {
        /* we found all the segments of the path--success! */
        err = REGERR_OK;

        if (pDesc) {
            COPYDESC(pDesc, &desc);
        }
        if (pPrev) {
            *pPrev = offPrev;
        }
        if (pParent) {
            *pParent = offParent;
        }
    }
    
    return err;

}   /* nr_Find */




/* nr_FindAtLevel -- looks for a node matching "pName" on the level starting
 *                   with "offset".  Returns REGERR_OK if found, REGERR_NOFIND
 *                   if not (plus other error conditions).
 *
 *                   If pDesc and pOffPrev are valid pointers *AND* the name is
 *                   found then pDesc will point at the REGDESC of the node and
 *                   pOffPrev will be the offset of the desc for the previous
 *                   node at the same level.  
 *
 *                   If the node is *NOT* found (REGERR_NOFIND is returned)
 *                   pDesc will point at the REGDESC of the last found node
 *                   (as will pOffPrev). If some other error is returned then
 *                   THese values must not be used.
 */
static REGERR nr_FindAtLevel(REGFILE *reg,
                             REGOFF offset,
                             const char *pName,
                             REGDESC *pDesc,
                             REGOFF *pOffPrev)
{
    char    namebuf[MAXREGNAMELEN];
    REGDESC desc;
    REGERR  err;
    REGOFF  prev = 0;

    /* Note: offset=0 when there's no 'down' or 'left' */
    XP_ASSERT(reg);
    XP_ASSERT(offset < reg->hdr.avail);
    XP_ASSERT(pName);
    XP_ASSERT(*pName);

    while ( offset != 0 )
    {
        /* get name of next node */
        err = nr_ReadDesc(reg, offset, &desc);
        if (err != REGERR_OK)
            return err;

        err = nr_ReadName(reg, &desc, sizeof(namebuf), namebuf);
        if (err != REGERR_OK)
            return err;

        /* check to see if it's the one we want */
        if (XP_STRCMP(namebuf, pName) == 0) {
            /* Found it! Signaled by non-zero offset */
            break;
        }

        /* advance to the next node */
        prev = offset;
        offset = desc.left;
    }

    if ( pDesc != NULL && (prev || offset)) {
        /* prev and offset BOTH null means we never loaded a desc */
        COPYDESC( pDesc, &desc );
    }
    if ( pOffPrev != NULL ) {
        *pOffPrev = prev;
    }

    if ( offset != 0 ) /* if we found one */
        return REGERR_OK;
    else
        return REGERR_NOFIND;
}   /* FindAtLevel */



static REGERR nr_CreateSubKey(REGFILE *reg,
                              REGOFF parent,
                              REGDESC *pDesc,
                              char *name)
{
    /* nr_CreateSubKey does NO error checking--callers *MUST*
     * ensure that there are no duplicates
     */
    REGDESC desc;
    REGERR err;

    XP_ASSERT(reg);
    XP_ASSERT(pDesc);
    XP_ASSERT(name);

    err = nr_AppendName(reg, name, &desc);
    if (err != REGERR_OK)
        return err;

    desc.type = REGTYPE_KEY;
    desc.left = 0;
    desc.down = 0;
    desc.value = 0;
    desc.valuelen = 0;
    desc.valuebuf = 0;
    desc.parent   = parent;

    if ( parent == pDesc->location ) {
        /* It's a parent desc, so no siblings */
        err = nr_AppendDesc(reg, &desc, &pDesc->down);
    }
    else {
        /* It's a sibling desc */
        XP_ASSERT( pDesc->left == 0 ); /* not the end of chain! */
        err = nr_AppendDesc(reg, &desc, &pDesc->left);
    }
    if (err != REGERR_OK)
        return err;

    /* write out the fixed up parent/sibling desc */
    err = nr_WriteDesc(reg, pDesc);
    COPYDESC(pDesc, &desc);

    return err;

}   /* nr_CreateSubKey */



static REGERR nr_CreateEntryString(REGFILE *reg, REGDESC *pParent, char *name, char *value)
{
    REGDESC desc;
    REGERR  err;

    XP_ASSERT(reg);
    XP_ASSERT(pParent);
    XP_ASSERT(name);
    XP_ASSERT(value);

    XP_MEMSET( &desc, 0, sizeof(REGDESC) );

    err = nr_AppendName(reg, name, &desc);
    if (err != REGERR_OK)
        return err;

    err = nr_AppendString(reg, value, &desc);
    if (err != REGERR_OK)
        return err;

    desc.type = REGTYPE_ENTRY_STRING_UTF;
    desc.left = pParent->value;
    desc.down = 0;
    desc.parent = pParent->location;

    err = nr_AppendDesc(reg, &desc, &pParent->value);
    if (err != REGERR_OK)
        return err;

    /* printf("nr_AddEntry: %s=%s @0x%lx\n", name, value, pParent->value); */

    return nr_WriteDesc(reg, pParent);

}   /* nr_CreateEntryString */



static REGERR nr_CreateEntry(REGFILE *reg, REGDESC *pParent, char *name,
    uint16 type, char *value, uint32 length)
{
    REGDESC desc;
    REGERR  err;

    XP_ASSERT(reg);
    XP_ASSERT(pParent);
    XP_ASSERT(name);
    XP_ASSERT(value);

    XP_MEMSET( &desc, 0, sizeof(REGDESC) );

    err = nr_AppendName(reg, name, &desc);
    if (err != REGERR_OK)
        return err;

    err = nr_AppendData(reg, value, length, &desc);
    if (err != REGERR_OK)
        return err;

    desc.type = type;
    desc.left = pParent->value;
    desc.down = 0;
    desc.parent = pParent->location;

    err = nr_AppendDesc(reg, &desc, &pParent->value);
    if (err != REGERR_OK)
        return err;

    /* printf("nr_AddEntry: %s=%s @0x%lx\n", name, value, pParent->value); */

    return nr_WriteDesc(reg, pParent);

}   /* nr_CreateEntry */




/* ---------------------------------------------------------------------
 * Intermediate API
 * ---------------------------------------------------------------------
 */
static REGOFF  nr_TranslateKey( REGFILE *reg, RKEY key );
static REGERR  nr_InitStdRkeys( REGFILE *reg );
static XP_Bool nr_ProtectedNode( REGFILE *reg, REGOFF key );
static REGERR  nr_RegAddKey( REGFILE *reg, RKEY key, char *path, RKEY *newKey, XP_Bool raw );
static REGERR  nr_RegDeleteKey( REGFILE *reg, RKEY key, char *path, XP_Bool raw );
static REGERR  nr_RegOpen( const char *filename, HREG *hReg );
static REGERR  nr_RegClose( HREG hReg );
static char*   nr_GetUsername();
static const char* nr_GetRegName (const char *name);
static int     nr_RegSetBufferSize( HREG hReg, int bufsize );

/* --------------------------------------------------------------------- */


static REGOFF nr_TranslateKey( REGFILE *reg, RKEY key )
{
    REGOFF retKey = 0;

    /* if it's a special key  */
    if ( key < HDRRESERVE )  {
        /* ...translate it */
        switch (key)
        {
            case ROOTKEY:
                retKey = reg->hdr.root;
                break;

            case ROOTKEY_VERSIONS:
                retKey = reg->rkeys.versions;
                break;

            case ROOTKEY_USERS:
                retKey = reg->rkeys.users;
                break;

            case ROOTKEY_COMMON:
                retKey = reg->rkeys.common;
                break;

#ifndef STANDALONE_REGISTRY
            case ROOTKEY_CURRENT_USER:
                if ( reg->rkeys.current_user == 0 ) {
                    /* not initialized--find the current user key */
                    RKEY    userkey = 0;
                    REGERR  err;
                    char*   profName;

                    profName = nr_GetUsername();
                    if ( NULL != profName ) {
                        /* Don't assign a slot for missing or magic profile */
                        if ( '\0' == *profName ||
                            0 == XP_STRCMP(ASW_MAGIC_PROFILE_NAME, profName)) 
                        {
                            err = REGERR_FAIL;
                        } else {
                            err = nr_RegAddKey( reg, reg->rkeys.users, profName, &userkey, FALSE );
                        }
                        XP_FREE(profName);
                    }
                    else {
                        err = nr_RegAddKey( reg, reg->rkeys.users, "default", &userkey, FALSE );
                    }

                    if ( err == REGERR_OK ) {
                        reg->rkeys.current_user = userkey;
                    }
                }
                retKey = reg->rkeys.current_user;
                break;
#endif /* !STANDALONE_REGISTRY */

            case ROOTKEY_PRIVATE:
                retKey = reg->rkeys.privarea;
                break;

            default:
                /* not a valid key */
                retKey = 0;
                break;
        }
    }
    else {
        /* ...otherwise it's fine as-is */
        retKey = (REGOFF)key;
    }
    return ( retKey );
}  /* nr_TranslateKey */



static REGERR nr_InitStdRkeys( REGFILE *reg )
{
    REGERR      err = REGERR_OK;
    RKEY        key;

    XP_ASSERT( reg != NULL );

    /* initialize to invalid key values */
    XP_MEMSET( &reg->rkeys, 0, sizeof(STDNODES) );

    /* Add each key before looking it up.  Adding an already
     * existing key is harmless, and these MUST exist.
     */

    /* ROOTKEY_USERS */
    err = nr_RegAddKey( reg, reg->hdr.root, ROOTKEY_USERS_STR, &key, FALSE );
    if ( err != REGERR_OK )
        return err;
    reg->rkeys.users = key;

    /* ROOTKEY_COMMON */
    err = nr_RegAddKey( reg, reg->hdr.root, ROOTKEY_COMMON_STR, &key, FALSE );
    if ( err != REGERR_OK ) 
        return err;
    reg->rkeys.common = key;

    /* ROOTKEY_VERSIONS */
    err = nr_RegAddKey( reg, reg->hdr.root, ROOTKEY_VERSIONS_STR, &key, FALSE );
    if ( err != REGERR_OK )
        return err;
    reg->rkeys.versions = key;

    /* ROOTKEY_CURRENT_USER */
    /* delay until first use -- see nr_TranslateKey */

    /* ROOTKEY_PRIVATE */
    err = nr_RegAddKey( reg, reg->hdr.root, ROOTKEY_PRIVATE_STR, &key, FALSE );
    if ( err != REGERR_OK ) 
        return err;
    reg->rkeys.privarea = key;

    return err;
}   /* nr_InitStdRkeys */



static XP_Bool nr_ProtectedNode( REGFILE *reg, REGOFF key )
{
    if ( (key == reg->hdr.root) ||
         (key == reg->rkeys.users) ||
         (key == reg->rkeys.versions) ||
         (key == reg->rkeys.common) ||
         (key == reg->rkeys.current_user) )
    {
        return TRUE;
    }
    else
        return FALSE;
}



static REGERR nr_RegAddKey( REGFILE *reg, RKEY key, char *path, RKEY *newKey, XP_Bool raw )
{
    REGERR      err;
    REGDESC     desc;
    REGOFF      start;
    REGOFF      parent;
    char        namebuf[MAXREGNAMELEN];
    char        *p;

    XP_ASSERT( regStartCount > 0 );
    XP_ASSERT( reg != NULL );
    XP_ASSERT( path != NULL );
    XP_ASSERT( *path != '\0' );
    XP_ASSERT( VALID_FILEHANDLE( reg->fh ) );

    /* have to translate again in case this is an internal call */
    start = nr_TranslateKey( reg, key );
    if ( start == 0 )
        return REGERR_PARAM;

    /* Get starting desc */
    err = nr_ReadDesc( reg, start, &desc );

    if (raw == TRUE) {
        if ( err == REGERR_OK) {
            /* look for name at next level down */
            parent = desc.location;
            err = nr_FindAtLevel(reg, desc.down, path, &desc, 0);

            /* if key is not found */
            if ( err == REGERR_NOFIND ) {
                /* add it as a sub-key to the last found key */
                err = nr_CreateSubKey(reg, parent, &desc, path);
            }
        }
    }
    else {
        /* Walk 'path', reading keys into 'desc' */
        p = path;
        while ( err == REGERR_OK ) {

            /* get next name on the path */
            err = nr_NextName(p, namebuf, sizeof(namebuf), &p);
            if ( err == REGERR_OK ) {
                /* look for name at next level down */
                parent = desc.location;
                err = nr_FindAtLevel(reg, desc.down, namebuf, &desc, 0);

                /* if key is not found */
                if ( err == REGERR_NOFIND ) {
                    /* add it as a sub-key to the last found key */
                    err = nr_CreateSubKey(reg, parent, &desc, namebuf);
                }
            }
        }
    }

    /* it's good to have processed the whole path */
    if ( (raw == FALSE && err == REGERR_NOMORE) ||
         (raw == TRUE && err == REGERR_OK) ) 
    {
        err = REGERR_OK;

        /* return new key if the caller wants it */
        if ( newKey != NULL ) {
            *newKey = desc.location;
        }
    }

    return err;

}   /* nr_RegAddKey */




static REGERR nr_RegDeleteKey( REGFILE *reg, RKEY key, char *path, XP_Bool raw )
{
    REGERR      err;
    REGOFF      start;
    REGDESC     desc;
    REGDESC     predecessor;
    REGOFF      offPrev;
    REGOFF      offParent;
    REGOFF*     link;

    XP_ASSERT( regStartCount > 0 );
    XP_ASSERT( reg != NULL );
    XP_ASSERT( VALID_FILEHANDLE( reg->fh ) );

    start = nr_TranslateKey( reg, key );
    if ( path == NULL || *path == '\0' || start == 0 )
        return REGERR_PARAM;

    /* find the specified key */
    err = nr_Find( reg, start, path, &desc, &offPrev, &offParent, raw );
    if ( err == REGERR_OK ) {

        XP_ASSERT( !TYPE_IS_ENTRY( desc.type ) );

        /* make sure it's childless and not a top-level key */
        if ( (desc.down == 0) && !nr_ProtectedNode( reg, desc.location ) ) {

            /* Are we the first on our level? */
            if ( offPrev == 0 ) {
                /* Yes: link to parent's "down" pointer */
                err = nr_ReadDesc( reg, offParent, &predecessor );
                link = &(predecessor.down);
            }
            else {
                /* No: link using predecessor's "left" pointer */
                err = nr_ReadDesc( reg, offPrev, &predecessor );
                link = &(predecessor.left);
            }

            /* If we read the predecessor desc OK */
            if (err == REGERR_OK) {
                XP_ASSERT( *link == desc.location );

                /* link predecessor to next, removing current node from chain */
                *link = desc.left;

                /* Write the updated predecessor */
                err = nr_WriteDesc( reg, &predecessor );
                if ( err == REGERR_OK ) {
                    /* Mark key deleted to prevent bogus use by anyone
                     * who is holding an RKEY for that node
                     */
                    desc.type |= REGTYPE_DELETED;
                    err = nr_WriteDesc( reg, &desc );
                }
            }
        }
        else {
            /* specified node is protected from deletion */
            err = REGERR_FAIL;
        }
    }

    return err;

}   /* nr_RegDeleteKey */



static int nr_RegSetBufferSize( HREG hReg, int bufsize )
{
    REGERR      err = REGERR_OK;
    REGHANDLE*  reghnd = (REGHANDLE*)hReg;
    REGFILE*    reg;
    XP_Bool     needDelete = FALSE;
    int         newSize;

    /* verify handle */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return -1;

    reg = reghnd->pReg;

    PR_Lock( reg->lock );
 
    newSize = XP_FileSetBufferSize( reg->fh, bufsize );

    PR_Unlock( reg->lock );

    return newSize;
}



static REGERR nr_RegOpen( const char *filename, HREG *hReg )
{
    REGERR    status = REGERR_OK;
    REGFILE   *pReg;
    REGHANDLE *pHandle;

    XP_ASSERT( regStartCount > 0 );

    /* initialize output handle in case of error */
    if ( hReg == NULL ) {
        return REGERR_PARAM;
    }
    *hReg = NULL;
    
    /* Look for named file in list of open registries */
    filename = nr_GetRegName( filename );
    if (filename == NULL) {
        filename = "";
    }
    pReg = vr_findRegFile( filename );

    /* if registry not already open */
    if (pReg == NULL) {

        /* ...then open it */
        pReg = (REGFILE*)XP_ALLOC( sizeof(REGFILE) );
        if ( pReg == NULL ) {
            status = REGERR_MEMORY;
            goto bail;
        }
        XP_MEMSET(pReg, 0, sizeof(REGFILE));

        pReg->inInit = TRUE;
        pReg->filename = XP_STRDUP(filename);
        if (pReg->filename == NULL) {
            XP_FREE( pReg );
            status = REGERR_MEMORY;
            goto bail;
        }

        status = nr_OpenFile( filename, &(pReg->fh) );
        if (status == REGERR_READONLY) {
            /* Open, but read only */
            pReg->readOnly = TRUE;
            status = REGERR_OK;
        }
        if ( status != REGERR_OK ) {
            XP_FREE( pReg->filename );
            XP_FREE( pReg );

            goto bail;
        }

        /* ...read and validate the header */
        status = nr_ReadHdr( pReg );
        if ( status != REGERR_OK ) {
            nr_CloseFile( &(pReg->fh) );
            XP_FREE( pReg->filename );
            XP_FREE( pReg );
            goto bail;
        }

        /* ...other misc initialization */
        pReg->refCount = 0;

#ifndef STANDALONE_REGISTRY
        pReg->uniqkey = PR_Now();
#endif

        status = nr_InitStdRkeys( pReg );
        if ( status == REGERR_OK ) {
            /* ...and add it to the list */
            nr_AddNode( pReg );
        }
        else {
            nr_CloseFile( &(pReg->fh) );
            XP_FREE( pReg->filename );
            XP_FREE( pReg );
            goto bail;
        }

#ifndef STANDALONE_REGISTRY
        pReg->lock = PR_NewLock();
#endif

        /* now done with everything that needs to protect the header */
        pReg->inInit = FALSE;
    }

    /* create a new handle to the regfile */
    pHandle = (REGHANDLE*)XP_ALLOC( sizeof(REGHANDLE) );
    if ( pHandle == NULL ) {
        /* we can't create the handle */
        if ( pReg->refCount == 0 ) {
            /* we've just opened it so close it and remove node */
            nr_CloseFile( &(pReg->fh) );
            nr_DeleteNode( pReg );
        }

        status = REGERR_MEMORY;
        goto bail;
    }

    pHandle->magic   = MAGIC_NUMBER;
    pHandle->pReg    = pReg;

    /* success: bump the reference count and return the handle */
    pReg->refCount++;
    *hReg = (void*)pHandle;

bail:
    return status;

}   /* nr_RegOpen */



static REGERR nr_RegClose( HREG hReg )
{
    REGERR      err = REGERR_OK;
    REGHANDLE*  reghnd = (REGHANDLE*)hReg;
    REGFILE*    reg;
    XP_Bool     needDelete = FALSE;

    XP_ASSERT( regStartCount > 0 );

    /* verify handle */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    reg = reghnd->pReg;

    PR_Lock( reg->lock );
    if ( err == REGERR_OK )
    {
        XP_ASSERT( VALID_FILEHANDLE(reg->fh) );

        /* save changed header info */
        if ( reg->hdrDirty ) {
            nr_WriteHdr( reg );
        }

        /* lower REGFILE user count */
        reg->refCount--;

        /* if registry is no longer in use */
        if ( reg->refCount < 1 ) 
        {
            /* ...then close the file */
            nr_CloseFile( &(reg->fh) );

            /* ...and mark REGFILE node for deletion from list */
            needDelete = TRUE;
        }
        else
        {
            /* ...otherwise make sure any writes are flushed */
            XP_FileFlush( reg->fh );
        }

        reghnd->magic = 0;    /* prevent accidental re-use */  
        PR_Unlock( reg->lock );

        if ( needDelete )
            nr_DeleteNode( reg );

        XP_FREE( reghnd );
    }

    return err;

}   /* nr_RegClose */



static char *nr_GetUsername()
{
  if (NULL == user_name) {
    return "default";
  } else {
    return user_name;
  }
}

static const char* nr_GetRegName (const char *name)
{
    if (name == NULL || *name == '\0') {
        XP_ASSERT( globalRegName != NULL );
        return globalRegName;
    } else {
        return name;
    }
}




/* ---------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------- */


/* ---------------------------------------------------------------------
 * NR_RegGetUsername - Gets a copy of the current username
 *
 * Parameters:
 *   A variable which, on exit will contain an alloc'ed string which is a
 *   copy of the current username.
 *
 * DO NOT USE -- OBSOLETE
 * ---------------------------------------------------------------------
 */

VR_INTERFACE(REGERR) NR_RegGetUsername(char **name)
{
    /* XXX: does this need locking? */

    if ( name == NULL )
        return REGERR_PARAM;

    *name = XP_STRDUP(nr_GetUsername());

    if ( NULL == *name )
        return REGERR_MEMORY;

    return REGERR_OK;
}


/* ---------------------------------------------------------------------
 * NR_RegSetBufferSize - Set the buffer size
 *
 * Parameters:
 *     name     - name of the current user
 *
 * Output:
 * ---------------------------------------------------------------------
 */

VR_INTERFACE(int) NR_RegSetBufferSize( HREG hReg, int bufsize )
{
    int      newSize;

    PR_Lock( reglist_lock );

    newSize = nr_RegSetBufferSize( hReg, bufsize );

    PR_Unlock(reglist_lock);

    return newSize;
}


/* ---------------------------------------------------------------------
 * NR_RegSetUsername - Set the current username
 * 
 * If the current user profile name is not set then trying to use
 * HKEY_CURRENT_USER will result in an error.
 *
 * Parameters:
 *     name     - name of the current user
 *
 * Output:
 * ---------------------------------------------------------------------
 */

VR_INTERFACE(REGERR) NR_RegSetUsername(const char *name)
{
    char *tmp;

    if ( name == NULL || *name == '\0' )
        return REGERR_PARAM;

    tmp = XP_STRDUP(name);
    if (NULL == tmp) {
        return REGERR_MEMORY;
    }

    PR_Lock( reglist_lock );

    XP_FREEIF(user_name);
    user_name = tmp;

/* XXX: changing the username should go through and clear out the current.user
   for each open registry. */

    PR_Unlock( reglist_lock );
  
    return REGERR_OK;
}




#ifndef STANDALONE_REGISTRY
/* ---------------------------------------------------------------------
 * NR_RegGetUniqueName
 * 
 * Returns a unique name that can be used for anonymous key/value names
 *
 * Parameters:
 *     hReg     - handle of open registry
 *     outbuf   - where to put the string
 *     buflen   - how big the buffer is
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegGetUniqueName(HREG hReg, char* outbuf, uint32 buflen)
{
    PRUint64    one;
    REGERR      err;
    REGFILE*    reg;
    static PRUint64 uniqkey;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    reg = ((REGHANDLE*)hReg)->pReg;

    if ( !outbuf )
        return REGERR_PARAM;

    if ( buflen <= (sizeof(PRUint64)*2) )
        return REGERR_BUFTOOSMALL;

    if ( LL_IS_ZERO(uniqkey) )
        uniqkey = PR_Now();

    PR_snprintf(outbuf,buflen,"%llx",uniqkey);

    /* increment counter for next time */
    LL_I2L(one,1);
    LL_ADD(uniqkey, uniqkey, one);

    return REGERR_OK;
}
#endif


       
       
/* ---------------------------------------------------------------------
 * NR_RegOpen - Open a netscape XP registry
 *
 * Parameters:
 *    filename   - registry file to open. NULL or ""  opens the standard
 *                 local registry.
 *    hReg       - OUT: handle to opened registry
 *
 * Output:
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegOpen( const char *filename, HREG *hReg )
{
    REGERR    status = REGERR_OK;

#if !defined(STANDALONE_REGISTRY)
    /* you must call NR_StartupRegistry() first */
    if ( regStartCount <= 0 )
        return REGERR_FAIL;
#endif

    PR_Lock(reglist_lock);

    status = nr_RegOpen( filename, hReg );

    PR_Unlock(reglist_lock);

    return status;

}   /* NR_RegOpen */




/* ---------------------------------------------------------------------
 * NR_RegClose - Close a netscape XP registry
 *
 * Parameters:
 *    hReg     - handle of open registry to be closed.
 *
 * After calling this routine the handle is no longer valid
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegClose( HREG hReg )
{
    REGERR      err = REGERR_OK;

    PR_Lock( reglist_lock );

    err = nr_RegClose( hReg );

    PR_Unlock(reglist_lock);

    return err;

}   /* NR_RegClose */




/* ---------------------------------------------------------------------
 * NR_RegFlush - Manually flush data in a netscape XP registry
 *
 * Parameters:
 *    hReg     - handle of open registry to be flushed.
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegFlush( HREG hReg )
{
    REGERR      err;
    REGFILE*    reg;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* can't flush a read-only registry */
    if ( reg->readOnly )
        return REGERR_READONLY;

    /* lock the registry file */
    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        if ( reg->hdrDirty ) {
            nr_WriteHdr( reg );
        }

        XP_FileFlush( reg->fh );

        /* unlock the registry */
        nr_Unlock( reg );
    }

    return err;

} /* NR_RegFlush */




/* ---------------------------------------------------------------------
 * NR_RegIsWritable - Check read/write status of open registry
 *
 * Parameters:
 *    hReg     - handle of open registry to query
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegIsWritable( HREG hReg )
{
    REGERR      err;
    REGFILE*    reg;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    reg = ((REGHANDLE*)hReg)->pReg;

    if ( reg->readOnly )
        return REGERR_READONLY;
    else
        return REGERR_OK;

}   /* NR_RegIsWritable */



/* ---------------------------------------------------------------------
 * NR_RegAddKey - Add a key node to the registry
 *
 *      This routine is simply a wrapper to perform user input
 *      validation and translation from HREG and standard key
 *      values into the internal format
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - registry key obtained from NR_RegGetKey(),
 *               or one of the standard top-level keys
 *    path     - relative path of key to be added.  Intermediate
 *               nodes will also be added if necessary.
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegAddKey( HREG hReg, RKEY key, char *path, RKEY *newKey )
{
    REGERR      err;
    REGOFF      start;
    REGFILE*    reg;

    /* prevent use of return value in case errors aren't checked */
    if ( newKey != NULL )
        *newKey = 0;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    reg = ((REGHANDLE*)hReg)->pReg;

    if ( path == NULL || *path == '\0' || reg == NULL )
        return REGERR_PARAM;

    /* lock the registry file */
    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        /* ... don't allow additional children of ROOTKEY */
        start = nr_TranslateKey( reg, key );
        if ( start != 0 && start != reg->hdr.root )
        {
            err = nr_RegAddKey( reg, start, path, newKey, FALSE );
        }
        else
            err = REGERR_PARAM;

        /* unlock the registry */
        nr_Unlock( reg );
    }

    return err;
}   /* NR_RegAddKey */




/* ---------------------------------------------------------------------
 * NR_RegAddKeyRaw - Add a key node to the registry
 *
 *      This routine is different from NR_RegAddKey() in that it takes 
 *      a keyname rather than a path.
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - registry key obtained from NR_RegGetKey(),
 *               or one of the standard top-level keys
 *    keyname  - name of key to be added. No parsing of this
 *               name happens.
 *    newkey   - if not null the RKEY of the new key is returned
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegAddKeyRaw( HREG hReg, RKEY key, char *keyname, RKEY *newKey )
{
    REGERR      err;
    REGOFF      start;
    REGFILE*    reg;

    /* prevent use of return value in case errors aren't checked */
    if ( newKey != NULL )
        *newKey = 0;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    reg = ((REGHANDLE*)hReg)->pReg;

    if ( keyname == NULL || *keyname == '\0' || reg == NULL )
        return REGERR_PARAM;

    /* lock the registry file */
    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        /* ... don't allow additional children of ROOTKEY */
        start = nr_TranslateKey( reg, key );
        if ( start != 0 && start != reg->hdr.root ) 
        {
            err = nr_RegAddKey( reg, start, keyname, newKey, TRUE );
        }
        else
            err = REGERR_PARAM;

        /* unlock the registry */
        nr_Unlock( reg );
    }

    return err;
}   /* NR_RegAddKeyRaw */




/* ---------------------------------------------------------------------
 * NR_RegDeleteKey - Delete the specified key
 *
 * Note that delete simply orphans blocks and makes no attempt
 * to reclaim space in the file. Use NR_RegPack()
 *
 * Cannot be used to delete keys with child keys
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - starting node RKEY, typically one of the standard ones.
 *    path     - relative path of key to delete
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegDeleteKey( HREG hReg, RKEY key, char *path )
{
    REGERR      err;
    REGFILE*    reg;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        err = nr_RegDeleteKey( reg, key, path, FALSE );
        nr_Unlock( reg );
    }

    return err;
}   /* NR_RegDeleteKey */




/* ---------------------------------------------------------------------
 * NR_RegDeleteKeyRaw - Delete the specified raw key
 *
 * Note that delete simply orphans blocks and makes no attempt
 * to reclaim space in the file. Use NR_RegPack()
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY or parent to the raw key you wish to delete
 *    keyname  - name of child key to delete
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegDeleteKeyRaw( HREG hReg, RKEY key, char *keyname )
{
    REGERR      err;
    REGFILE*    reg;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        err = nr_RegDeleteKey( reg, key, keyname, TRUE );
        nr_Unlock( reg );
    }

    return err;
}   /* NR_RegDeleteKeyRaw */




/* ---------------------------------------------------------------------
 * NR_RegGetKey - Get the RKEY value of a node from its path
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - starting node RKEY, typically one of the standard ones.
 *    path     - relative path of key to find.  (a blank path just gives you
 *               the starting key--useful for verification, VersionRegistry)
 *    result   - if successful the RKEY of the specified sub-key
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegGetKey( HREG hReg, RKEY key, const char *path, RKEY *result )
{
    REGERR      err;
    REGOFF      start;
    REGFILE*    reg;
    REGDESC     desc;

    XP_ASSERT( regStartCount > 0 );

    /* prevent use of return value in case errors aren't checked */
    if ( result != NULL )
        *result = (RKEY)0;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( path == NULL || result == NULL )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        start = nr_TranslateKey( reg, key );
        if ( start != 0 )
        {
            /* find the specified key ( if it's valid )*/
            err = nr_Find( reg, start, path, &desc, 0, 0, FALSE );
            if ( err == REGERR_OK ) {
                *result = (RKEY)desc.location;
            }
        }
        else {
            err = REGERR_PARAM;
        }

        nr_Unlock( reg );
    }

    return err;

}   /* NR_RegGetKey */




/* ---------------------------------------------------------------------
 * NR_RegGetKeyRaw - Get the RKEY value of a node from its keyname
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - starting node RKEY, typically one of the standard ones.
 *    keyname  - keyname of key to find.  (a blank keyname just gives you
 *               the starting key--useful for verification, VersionRegistry)
 *    result   - if successful the RKEY of the specified sub-key
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegGetKeyRaw( HREG hReg, RKEY key, char *keyname, RKEY *result )
{
    REGERR      err;
    REGOFF      start;
    REGFILE*    reg;
    REGDESC     desc;

    XP_ASSERT( regStartCount > 0 );

    /* prevent use of return value in case errors aren't checked */
    if ( result != NULL )
        *result = (RKEY)0;

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( keyname == NULL || result == NULL )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        start = nr_TranslateKey( reg, key );
        if ( start != 0 )
        {
            /* find the specified key ( if it's valid )*/
            err = nr_Find( reg, start, keyname, &desc, 0, 0, TRUE );
            if ( err == REGERR_OK ) {
                *result = (RKEY)desc.location;
            }
        }
        else {
            err = REGERR_PARAM;
        }

        nr_Unlock( reg );
    }

    return err;

}   /* NR_RegGetKeyRaw */




/* ---------------------------------------------------------------------
 * NR_RegGetEntryInfo - Get some basic info about the entry data
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY of key that contains entry--obtain with NR_RegGetKey()
 *    name     - name of entry
 *    info     - return: Entry info object
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegGetEntryInfo( HREG hReg, RKEY key, char *name, 
                            REGINFO *info )
{
    REGERR      err;
    REGFILE*    reg;
    REGDESC     desc;
    
    XP_ASSERT( regStartCount > 0 );

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( name == NULL || *name == '\0' || info == NULL || key == 0 )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        /* read starting desc */
        err = nr_ReadDesc( reg, key, &desc);
        if ( err == REGERR_OK ) 
        {
            /* if the named entry exists */
            err = nr_FindAtLevel( reg, desc.value, name, &desc, NULL );
            if ( err == REGERR_OK ) 
            {
                /* ... return the values */
                if ( info->size == sizeof(REGINFO) )
                {
                    info->entryType   = desc.type;
                    info->entryLength = desc.valuelen;
                }
                else
                {
                    /* uninitialized (maybe invalid) REGINFO structure */
                    err = REGERR_PARAM;
                }
            }
        }

        nr_Unlock( reg );
    }

    return err;

}   /* NR_RegGetEntryInfo */



       
/* ---------------------------------------------------------------------
 * NR_RegGetEntryString - Get the UTF string value associated with the
 *                       named entry of the specified key.
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY of key that contains entry--obtain with NR_RegGetKey()
 *    name     - name of entry
 *    buffer   - destination for string
 *    bufsize  - size of buffer
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegGetEntryString( HREG  hReg, RKEY  key, char  *name,
                            char  *buffer, uint32 bufsize)
{
    REGERR      err;
    REGFILE*    reg;
    REGDESC     desc;

    XP_ASSERT( regStartCount > 0 );

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( name==NULL || *name=='\0' || buffer==NULL || bufsize==0 || key==0 )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        /* read starting desc */
        err = nr_ReadDesc( reg, key, &desc);
        if ( err == REGERR_OK ) 
        {
            /* if the named entry exists */
            err = nr_FindAtLevel( reg, desc.value, name, &desc, NULL );
            if ( err == REGERR_OK ) 
            {
                /* read the string */
                if ( desc.type == REGTYPE_ENTRY_STRING_UTF ) 
                {
                    err = nr_ReadData( reg, &desc, bufsize, buffer );
                    /* prevent run-away strings */
                    buffer[bufsize-1] = '\0';
                }
                else {
                    err = REGERR_BADTYPE;
                }
            }
        }

        nr_Unlock( reg );
    }

    return err;

}   /* NR_RegGetEntryString */




/* ---------------------------------------------------------------------
 * NR_RegGetEntry - Get the value data associated with the
 *                  named entry of the specified key.
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY of key that contains entry--obtain with NR_RegGetKey()
 *    name     - name of entry
 *    buffer   - destination for data
 *    size     - in:  size of buffer
 *               out: size of actual data (incl. \0 term. for strings)
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegGetEntry( HREG hReg, RKEY key, char *name,
    void *buffer, uint32 *size )
{
    REGERR      err;
    REGFILE*    reg;
    REGDESC     desc;
    char        *tmpbuf = NULL;  /* malloc a tmp buffer to convert XP int arrays */
    uint32      nInt;
    uint32      *pISrc;
    uint32      *pIDest;
    XP_Bool     needFree = FALSE;

    XP_ASSERT( regStartCount > 0 );

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( name==NULL || *name=='\0' || buffer==NULL || size==NULL || key==0 )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    err = nr_Lock( reg );
    if ( err == REGERR_OK )
    {
        /* read starting desc */
        err = nr_ReadDesc( reg, key, &desc);
        if ( err == REGERR_OK )
        {
            /* if the named entry exists */
            err = nr_FindAtLevel( reg, desc.value, name, &desc, NULL );
            if ( err == REGERR_OK )
            {
                if ( desc.valuelen > *size ) {
                    err = REGERR_BUFTOOSMALL;
                }
                else if ( desc.valuelen == 0 ) {
                    err = REGERR_FAIL;
                }
                else switch (desc.type)
                {
                /* platform independent array of 32-bit integers */
                case REGTYPE_ENTRY_INT32_ARRAY:
                    tmpbuf = (char*)XP_ALLOC( desc.valuelen );
                    if ( tmpbuf != NULL ) 
                    {
                        needFree = TRUE;
                        err = nr_ReadData( reg, &desc, desc.valuelen, tmpbuf );
                        if ( REGERR_OK == err )
                        {
                            /* convert int array */
                            nInt = (desc.valuelen / INTSIZE);
                            pISrc = (uint32*)tmpbuf;
                            pIDest = (uint32*)buffer;
                            for(; nInt > 0; nInt--, pISrc++, pIDest++) {
                                *pIDest = nr_ReadLong((char*)pISrc);
                            }
                        }
                    }
                    else
                        err = REGERR_MEMORY;
                    break;

                case REGTYPE_ENTRY_STRING_UTF:
                    tmpbuf = (char*)buffer;
                    err = nr_ReadData( reg, &desc, *size, tmpbuf );
                    /* prevent run-away strings */
                    tmpbuf[(*size)-1] = '\0';
                    break;

                case REGTYPE_ENTRY_FILE:

                    err = nr_ReadData( reg, &desc, *size, (char*)buffer );
#if defined(XP_MAC) || defined(XP_MACOSX)
                    if (err == 0)
                    {
                        tmpbuf = nr_PathFromMacAlias(buffer, *size);
                        if (tmpbuf == NULL) 
                        {
                            buffer = NULL;
                            err = REGERR_NOFILE; /* must match nr_GetPathname() in VerReg.c */
                        }
                        else 
                        {
                            needFree = TRUE;

                            if (XP_STRLEN(tmpbuf) < *size) /* leave room for \0 */
                                XP_STRCPY(buffer, tmpbuf);
                            else 
                                err = REGERR_BUFTOOSMALL;
                        }
                    }
#endif
                    break;
                
                case REGTYPE_ENTRY_BYTES:
                default:              /* return raw data for unknown types */
                    err = nr_ReadData( reg, &desc, *size, (char*)buffer );
                    break;
                }

                /* return the actual data size */
                *size = desc.valuelen;
            }
        }

        nr_Unlock( reg );
    }

    if (needFree)
        XP_FREE(tmpbuf);

    return err;

}   /* NR_RegGetEntry */




/* ---------------------------------------------------------------------
 * NR_RegSetEntryString - Store a UTF-8 string value associated with the
 *                       named entry of the specified key.  Used for
 *                       both creation and update.
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY of key that contains entry--obtain with NR_RegGetKey()
 *    name     - name of entry
 *    buffer   - UTF-8 String to store
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegSetEntryString( HREG hReg, RKEY key, char *name,
                                     char *buffer )
{
    REGERR      err;
    REGFILE*    reg;
    REGDESC     desc;
    REGDESC     parent;

    XP_ASSERT( regStartCount > 0 );

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( name == NULL || *name == '\0' || buffer == NULL || key == 0 )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err != REGERR_OK )
        return err;

    /* read starting desc */
    err = nr_ReadDesc( reg, key, &parent);
    if ( err == REGERR_OK ) {

        /* if the named entry already exists */
        err = nr_FindAtLevel( reg, parent.value, name, &desc, NULL );
        if ( err == REGERR_OK ) {
            /* then update the existing one */
            err = nr_WriteString( reg, buffer, &desc );
            if ( err == REGERR_OK ) {
                desc.type = REGTYPE_ENTRY_STRING_UTF;
                err = nr_WriteDesc( reg, &desc );
            }
        }
        else if ( err == REGERR_NOFIND ) {
            /* otherwise create a new entry */
            err = nr_CreateEntryString( reg, &parent, name, buffer );
        }
        /* other errors fall through */
    }

    /* unlock registry */
    nr_Unlock( reg );

    return err;

}   /* NR_RegSetEntryString */




/* ---------------------------------------------------------------------
 * NR_RegSetEntry - Store value data associated with the named entry
 *                  of the specified key.  Used for both creation and update.
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY of key that contains entry--obtain with NR_RegGetKey()
 *    name     - name of entry
 *    type     - type of data to be stored
 *    buffer   - data to store
 *    size     - length of data to store in bytes
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegSetEntry( HREG hReg, RKEY key, char *name, uint16 type,
    void *buffer, uint32 size )
{
    REGERR      err;
    REGFILE*    reg;
    REGDESC     desc;
    REGDESC     parent;
    char        *data = NULL;
    uint32      nInt;
    uint32      *pIDest;
    uint32      *pISrc;
    XP_Bool     needFree = FALSE;
    int32       datalen = size;

    XP_ASSERT( regStartCount > 0 );

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( name==NULL || *name=='\0' || buffer==NULL || size==0 || key==0 )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* validate type and convert numerics to XP format */
    switch (type)
    {
        case REGTYPE_ENTRY_BYTES:
            data = (char*)buffer;
            break;

        case REGTYPE_ENTRY_FILE:

#if defined(XP_MAC) || defined(XP_MACOSX)
            nr_MacAliasFromPath(buffer, (void **)&data, &datalen);
            if (data)
                needFree = TRUE;
#else
            data = (char*)buffer;   
#endif
            break;


        case REGTYPE_ENTRY_STRING_UTF:
            data = (char*)buffer;
            /* string must be null terminated */
            if ( data[size-1] != '\0' )
                return REGERR_PARAM;
            break;


        case REGTYPE_ENTRY_INT32_ARRAY:
            /* verify no partial integers */
            if ( (size % INTSIZE) != 0 )
                return REGERR_PARAM;

            /* get a conversion buffer */
            data = (char*)XP_ALLOC(size);
            if ( data == NULL )
                return REGERR_MEMORY;
            else
                needFree = TRUE;

            /* convert array to XP format */
            nInt = ( size / INTSIZE );
            pIDest = (uint32*)data;
            pISrc  = (uint32*)buffer;

            for( ; nInt > 0; nInt--, pIDest++, pISrc++) {
                nr_WriteLong( *pISrc, (char*)pIDest );
            }
            break;


        default:
            return REGERR_BADTYPE;
    }

    /* lock registry */
    err = nr_Lock( reg );
    if ( REGERR_OK == err )
    {
        /* read starting desc */
        err = nr_ReadDesc( reg, key, &parent);
        if ( err == REGERR_OK ) 
        {
            /* if the named entry already exists */
            err = nr_FindAtLevel( reg, parent.value, name, &desc, NULL );
            if ( err == REGERR_OK ) 
            {
                /* then update the existing one */
                err = nr_WriteData( reg, data, datalen, &desc );
                if ( err == REGERR_OK ) 
                {
                    desc.type = type;
                    err = nr_WriteDesc( reg, &desc );
                }
            }
            else if ( err == REGERR_NOFIND ) 
            {
                /* otherwise create a new entry */
                err = nr_CreateEntry( reg, &parent, name, type, data, datalen );
            }
            else {
                /* other errors fall through */
            }
        }

        /* unlock registry */
        nr_Unlock( reg );
    }

    if (needFree)
        XP_FREE(data);

    return err;

}   /* NR_RegSetEntry */




/* ---------------------------------------------------------------------
 * NR_RegDeleteEntry - Delete the named entry
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY of key that contains entry--obtain with NR_RegGetKey()
 *    name     - name of entry
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegDeleteEntry( HREG hReg, RKEY key, char *name )
{
    REGERR      err;
    REGFILE*    reg;
    REGDESC     desc;
    REGDESC     parent;
    REGOFF      offPrev;

    XP_ASSERT( regStartCount > 0 );

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( name == NULL || *name == '\0' || key == 0)
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err != REGERR_OK )
        return err;

    /* read starting desc */
    err = nr_ReadDesc( reg, key, &parent);
    if ( err == REGERR_OK ) {

        /* look up the named entry */
        err = nr_FindAtLevel( reg, parent.value, name, &desc, &offPrev );
        if ( err == REGERR_OK ) {

            XP_ASSERT( TYPE_IS_ENTRY( desc.type ) );

            /* if entry is the head of a chain */
            if ( offPrev == 0 ) {
                /* hook parent key to next entry */
                XP_ASSERT( parent.value == desc.location );
                parent.value = desc.left;
            }
            else {
                /* otherwise hook previous entry to next */
                err = nr_ReadDesc( reg, offPrev, &parent );
                parent.left = desc.left;
            }
            /* write out changed desc for previous node */
            if ( err == REGERR_OK ) {
                err = nr_WriteDesc( reg, &parent );
                /* zap the deleted desc because an enum state may contain a
                 * reference to a specific entry node
                 */
                if ( err == REGERR_OK ) {
                    desc.type |= REGTYPE_DELETED;
                    err = nr_WriteDesc( reg, &desc );
                }
            }
        }
    }

    /* unlock registry */
    nr_Unlock( reg );

    return err;

}   /* NR_RegDeleteEntry */




/* ---------------------------------------------------------------------
 * NR_RegEnumSubkeys - Enumerate the subkey names for the specified key
 *
 * Returns REGERR_NOMORE at end of enumeration.
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY of key to enumerate--obtain with NR_RegGetKey()
 *    eState   - enumerations state, must contain NULL to start
 *    buffer   - location to store subkey names.  Once an enumeration
 *               is started user must not modify contents since values
 *               are built using the previous contents.
 *    bufsize  - size of buffer for names
 *    style    - 0 returns direct child keys only, REGENUM_DESCEND
 *               returns entire sub-tree
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegEnumSubkeys( HREG hReg, RKEY key, REGENUM *state,
                                    char *buffer, uint32 bufsize, uint32 style)
{
    REGERR      err;
    REGFILE*    reg;
    REGDESC     desc;

    XP_ASSERT( regStartCount > 0 );

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( key == 0 || state == NULL || buffer == NULL )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err != REGERR_OK )
        return err;

    desc.down     = 0; /* initialize to quiet warnings */
    desc.location = 0;

    /* verify starting key */
    key = nr_TranslateKey( reg, key );
    if ( key == 0 )
        err = REGERR_PARAM;
    else if ( *state == 0 )
        err = nr_ReadDesc( reg, key, &desc);
    else
        err = REGERR_OK;

    if ( err == REGERR_OK )
    {
        /* if in initial state and no children return now */
        if ( *state == 0 && desc.down == 0 ) 
        {
            err = REGERR_NOMORE;
        }
        else switch ( style )
        {
          case REGENUM_CHILDREN:
            *buffer = '\0';
            if ( *state == 0 ) 
            {
                /* initial state: get first child (.down) */
                err = nr_ReplaceName( reg, desc.down, buffer, bufsize, &desc );
            }
            else 
            {
                /* get sibling (.left) of current key */
                err = nr_ReadDesc( reg, *state, &desc );
                if ( err == REGERR_OK || REGERR_DELETED == err )
                {
                    /* it's OK for the current (state) node to be deleted */
                    if ( desc.left != 0 ) 
                    {
                        err = nr_ReplaceName( reg, desc.left, 
                                    buffer, bufsize, &desc );
                    }
                    else
                        err = REGERR_NOMORE;
                }
            }
            break;


          case REGENUM_DESCEND:
            if ( *state == 0 ) 
            {
                /* initial state */
                *buffer = '\0';
                err = nr_ReplaceName( reg, desc.down, buffer, bufsize, &desc );
            }
            else 
            {
                /* get last position */
                err = nr_ReadDesc( reg, *state, &desc );
                if ( REGERR_OK != err && REGERR_DELETED != err ) 
                {
                    /* it is OK for the state node to be deleted
                     * (the *next* node MUST be "live", though).
                     * bail out on any other error */
                    break;
                }

                if ( desc.down != 0 ) {
                    /* append name of first child key */
                    err = nr_CatName( reg, desc.down, buffer, bufsize, &desc );
                }
                else if ( desc.left != 0 ) {
                    /* replace last segment with next sibling */
                    err = nr_ReplaceName( reg, desc.left, 
                                buffer, bufsize, &desc );
                }
                else {
                  /* done with level, pop up as many times as necessary */
                    while ( err == REGERR_OK ) 
                    {
                        if ( desc.parent != key && desc.parent != 0 ) 
                        {
                            err = nr_RemoveName( buffer );
                            if ( err == REGERR_OK ) 
                            {
                                err = nr_ReadDesc( reg, desc.parent, &desc );
                                if ( err == REGERR_OK && desc.left != 0 ) 
                                {
                                    err = nr_ReplaceName( reg, desc.left, 
                                                buffer, bufsize, &desc );
                                    break;  /* found a node */
                                }
                            }
                        }
                        else
                            err = REGERR_NOMORE;
                    }
                }
            }
            break;


          case REGENUM_DEPTH_FIRST:
            if ( *state == 0 ) 
            {
                /* initial state */

                *buffer = '\0';
                err = nr_ReplaceName( reg, desc.down, buffer, bufsize, &desc );
                while ( REGERR_OK == err && desc.down != 0 )
                {
                    /* start as far down the tree as possible */
                    err = nr_CatName( reg, desc.down, buffer, bufsize, &desc );
                }
            }
            else 
            {
                /* get last position */
                err = nr_ReadDesc( reg, *state, &desc );
                if ( REGERR_OK != err && REGERR_DELETED != err ) 
                {
                    /* it is OK for the state node to be deleted
                     * (the *next* node MUST be "live", though).
                     * bail out on any other error */
                    break;
                }

                if ( desc.left != 0 )
                {
                    /* get sibling, then descend as far as possible */
                    err = nr_ReplaceName(reg, desc.left, buffer,bufsize,&desc);

                    while ( REGERR_OK == err && desc.down != 0 ) 
                    {
                        err = nr_CatName(reg, desc.down, buffer,bufsize,&desc);
                    }
                }
                else 
                {
                    /* pop up to parent */
                    if ( desc.parent != key && desc.parent != 0 )
                    {
                        err = nr_RemoveName( buffer );
                        if ( REGERR_OK == err )
                        {
                            /* validate parent key */
                            err = nr_ReadDesc( reg, desc.parent, &desc );
                        }
                    }
                    else 
                        err = REGERR_NOMORE;
                }
            }
            break;


          default:
            err = REGERR_PARAM;
            break;
        }
    }

    /* set enum state to current key */
    if ( err == REGERR_OK ) {
        *state = desc.location;
    }

    /* unlock registry */
    nr_Unlock( reg );

    return err;

}   /* NR_RegEnumSubkeys */




/* ---------------------------------------------------------------------
 * NR_RegEnumEntries - Enumerate the entry names for the specified key
 *
 * Returns REGERR_NOMORE at end of enumeration.
 *
 * Parameters:
 *    hReg     - handle of open registry
 *    key      - RKEY of key that contains entry--obtain with NR_RegGetKey()
 *    eState   - enumerations state, must contain NULL to start
 *    buffer   - location to store entry names
 *    bufsize  - size of buffer for names
 *    info     - optional REGINFO for the entry. If not NULL must be 
 *               initialized as in NR_RegGetEntryInfo()
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegEnumEntries( HREG hReg, RKEY key, REGENUM *state,
                            char *buffer, uint32 bufsize, REGINFO *info )
{
    REGERR      err;
    REGFILE*    reg;
    REGDESC     desc;

    XP_ASSERT( regStartCount > 0 );

    /* verify parameters */
    err = VERIFY_HREG( hReg );
    if ( err != REGERR_OK )
        return err;

    if ( key == 0 || state == NULL || buffer == NULL )
        return REGERR_PARAM;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err != REGERR_OK )
        return err;
    
    /* verify starting key */
    err = nr_ReadDesc( reg, key, &desc);
    if ( err == REGERR_OK )
    {
        if ( *state == 0 ) 
        {
            /* initial state--get first entry */
            if ( desc.value != 0 ) 
            {
                *buffer = '\0';
                err =  nr_ReplaceName( reg, desc.value, buffer, bufsize, &desc );
            }
            else  
            { 
                /* there *are* no entries */
                err = REGERR_NOMORE;
            }
        }
        else 
        {
            /* 'state' stores previous entry */
            err = nr_ReadDesc( reg, *state, &desc );
            if ( err == REGERR_OK  || err == REGERR_DELETED ) 
            {
                /* get next entry in chain */
                if ( desc.left != 0 ) 
                {
                    *buffer = '\0';
                    err =  nr_ReplaceName( reg, desc.left, buffer, bufsize, &desc );
                }
                else 
                {
                    /* at end of chain */
                    err = REGERR_NOMORE;
                }
            }
        }

        /* if we found an entry */
        if ( err == REGERR_OK ) 
        {
            /* set enum state to current entry */
            *state = desc.location;

            /* return REGINFO if requested */
            if ( info != NULL && info->size >= sizeof(REGINFO) ) 
            {
                info->entryType   = desc.type;
                info->entryLength = desc.valuelen;
            }
        }
    }

    /* unlock registry */
    nr_Unlock( reg );

    return err;

}   /* NR_RegEnumEntries */





/* --------------------------------------------------------------------
 * Registry Packing
 * --------------------------------------------------------------------
 */
#ifndef STANDALONE_REGISTRY
#include "VerReg.h"

#ifdef RESURRECT_LATER
static REGERR nr_createTempRegName( char *filename, uint32 filesize );
static REGERR nr_addNodesToNewReg( HREG hReg, RKEY rootkey, HREG hRegNew, void *userData, nr_RegPackCallbackFunc fn );
/* -------------------------------------------------------------------- */
static REGERR nr_createTempRegName( char *filename, uint32 filesize )
{
    struct stat statbuf;
    XP_Bool nameFound = FALSE;
    char tmpname[MAX_PATH+1];
    uint32 len;
    int err;

    XP_STRCPY( tmpname, filename );
    len = XP_STRLEN(tmpname);
    if (len < filesize) {
        tmpname[len-1] = '~';
        tmpname[len] = '\0';
        remove(tmpname);
        if ( stat(tmpname, &statbuf) != 0 )
            nameFound = TRUE;
    }
    len++;
    while (!nameFound && len < filesize ) {
        tmpname[len-1] = '~';
        tmpname[len] = '\0';
        remove(tmpname);
        if ( stat(tmpname, &statbuf) != 0 )
            nameFound = TRUE;
        else
            len++;
    }  
    if (nameFound) {
        XP_STRCPY(filename, tmpname);
        err = REGERR_OK;
    } else {
        err = REGERR_FAIL;
    }
   return err;
}

static REGERR nr_addNodesToNewReg( HREG hReg, RKEY rootkey, HREG hRegNew, void *userData, nr_RegPackCallbackFunc fn )
{
    char keyname[MAXREGPATHLEN+1] = {0};
    char entryname[MAXREGPATHLEN+1] = {0};
    void *buffer;
    uint32 bufsize = 2024;
    uint32 datalen;
    REGENUM state = 0;
    REGENUM entrystate = 0;
    REGINFO info;
    int err = REGERR_OK;
    int status = REGERR_OK;
    RKEY key;
    RKEY newKey;
    REGFILE* reg;
    REGFILE* regNew;
    static int32 cnt = 0;
    static int32 prevCnt = 0;

    reg = ((REGHANDLE*)hReg)->pReg;
    regNew = ((REGHANDLE*)hRegNew)->pReg;

    buffer = XP_ALLOC(bufsize);
    if ( buffer == NULL ) {
        err = REGERR_MEMORY;
        return err;
    }

    while (err == REGERR_OK)
    {
        err = NR_RegEnumSubkeys( hReg, rootkey, &state, keyname, sizeof(keyname), REGENUM_DESCEND );
        if ( err != REGERR_OK )
            break;
        err = NR_RegAddKey( hRegNew, rootkey, keyname, &newKey );
        if ( err != REGERR_OK )
            break;
        cnt++;
        if (cnt >= prevCnt + 15) 
        {
            fn(userData, regNew->hdr.avail, reg->hdr.avail);
            prevCnt = cnt;
        }
        err = NR_RegGetKey( hReg, rootkey, keyname, &key );
        if ( err != REGERR_OK )
            break;
        entrystate = 0;
        status = REGERR_OK;
        while (status == REGERR_OK) {
            info.size = sizeof(REGINFO);
            status = NR_RegEnumEntries( hReg, key, &entrystate, entryname, 
                                        sizeof(entryname), &info );
            if ( status == REGERR_OK ) {
                XP_ASSERT( bufsize >= info.entryLength );
                datalen = bufsize;
                status = NR_RegGetEntry( hReg, key, entryname, buffer, &datalen );
                XP_ASSERT( info.entryLength == datalen );
                if ( status == REGERR_OK ) {
                    /* copy entry */
                    status = NR_RegSetEntry( hRegNew, newKey, entryname, 
                                info.entryType, buffer, info.entryLength );
                }
            } 
        }
        if ( status != REGERR_NOMORE ) {
            /* pass real error to outer loop */
            err = status;
        }
    }

    if ( err == REGERR_NOMORE )
        err = REGERR_OK;

    XP_FREEIF(buffer);
    return err;
}
#endif /* RESURRECT_LATER */



/* ---------------------------------------------------------------------
 * NR_RegPack    - Pack an open registry.  
 *                Registry is locked the entire time.
 *
 * Parameters:
 *    hReg     - handle of open registry to pack
 * ---------------------------------------------------------------------
 */
VR_INTERFACE(REGERR) NR_RegPack( HREG hReg, void *userData, nr_RegPackCallbackFunc fn)
{
    return REGERR_FAIL; /* XXX resurrect after mozilla beta 1 */
#if RESURRECT_LATER
    XP_File  fh;
    REGFILE* reg;
    HREG hRegTemp;
    char tempfilename[MAX_PATH+1] = {0};
    char oldfilename[MAX_PATH+1] = {0};

    XP_Bool bCloseTempFile = FALSE;

    int err = REGERR_OK;
    RKEY key;

    XP_ASSERT( regStartCount > 0 );
    if ( regStartCount <= 0 )
        return REGERR_FAIL;

    reg = ((REGHANDLE*)hReg)->pReg;

    /* lock registry */
    err = nr_Lock( reg );
    if ( err != REGERR_OK )
        return err; 

    PR_Lock(reglist_lock); 
    XP_STRCPY(tempfilename, reg->filename);
    err = nr_createTempRegName(tempfilename, sizeof(tempfilename));
    if ( err != REGERR_OK )
        goto safe_exit; 
     
    /* force file creation */
    fh = vr_fileOpen(tempfilename, XP_FILE_WRITE_BIN);
    if ( !VALID_FILEHANDLE(fh) ) {
        err = REGERR_FAIL;
        goto safe_exit;
    }
    XP_FileClose(fh);

    err = NR_RegOpen(tempfilename, &hRegTemp);
    if ( err != REGERR_OK )
        goto safe_exit;
    bCloseTempFile = TRUE;

    /* must open temp file first or we get the same name twice */
    XP_STRCPY(oldfilename, reg->filename);
    err = nr_createTempRegName(oldfilename, sizeof(oldfilename));
    if ( err != REGERR_OK )
        goto safe_exit; 
     
    key = ROOTKEY_PRIVATE;
    err = nr_addNodesToNewReg( hReg, key, hRegTemp, userData, fn);
    if ( err != REGERR_OK  )
        goto safe_exit;
    key = ROOTKEY_VERSIONS;
    err = nr_addNodesToNewReg( hReg, key, hRegTemp, userData, fn);
    if ( err != REGERR_OK  )
        goto safe_exit;
    key = ROOTKEY_COMMON;
    err = nr_addNodesToNewReg( hReg, key, hRegTemp, userData, fn);
    if ( err != REGERR_OK  )
        goto safe_exit;
    key = ROOTKEY_USERS;
    err = nr_addNodesToNewReg( hReg, key, hRegTemp, userData, fn);
    if ( err != REGERR_OK  )
        goto safe_exit;

    err = NR_RegClose(hRegTemp);
    bCloseTempFile = FALSE;
  
    /* close current reg file so we can rename it */
    XP_FileClose(reg->fh);
   
    /* rename current reg file out of the way */
    err = nr_RenameFile(reg->filename, oldfilename);
    if ( err == -1 ) {
        /* rename failed, get rid of the new registry and reopen the old one*/
        remove(tempfilename);
        reg->fh = vr_fileOpen(reg->filename, XP_FILE_UPDATE_BIN);
        goto safe_exit;
    }

    /* rename packed registry to the correct name */
    err = nr_RenameFile(tempfilename, reg->filename);
    if ( err == -1 ) {
        /* failure, recover original registry */
        err = nr_RenameFile(oldfilename, reg->filename);
        remove(tempfilename);
        reg->fh = vr_fileOpen(reg->filename, XP_FILE_UPDATE_BIN);
        goto safe_exit;
    
    } else {
        remove(oldfilename); 
    }
    reg->fh = vr_fileOpen(reg->filename, XP_FILE_UPDATE_BIN);

safe_exit:
    if ( bCloseTempFile ) {
        NR_RegClose(hRegTemp);
    }
    PR_Unlock( reglist_lock );
    nr_Unlock(reg);
    return err;
#endif /* RESURRECT_LATER */
}


#ifdef XP_MAC
#pragma export reset
#endif

#endif /* STANDALONE_REGISTRY */






/* ---------------------------------------------------------------------
 * ---------------------------------------------------------------------
 * Registry initialization and shut-down
 * ---------------------------------------------------------------------
 * ---------------------------------------------------------------------
 */

#include "VerReg.h"

#ifndef STANDALONE_REGISTRY
extern PRLock *vr_lock;
#endif 



#if defined(XP_UNIX) && !defined(XP_MACOSX) && !defined(STANDALONE_REGISTRY)
extern XP_Bool bGlobalRegistry;
#endif

#ifdef XP_MAC
#pragma export on
#endif

VR_INTERFACE(REGERR) NR_StartupRegistry(void)
{
    REGERR status = REGERR_OK;

#ifndef STANDALONE_REGISTRY
    if ( reglist_lock == NULL ) {
        reglist_lock = PR_NewLock();
    }

    if ( reglist_lock != NULL ) {
        PR_Lock( reglist_lock );
    }
    else {
        XP_ASSERT( reglist_lock );
        status = REGERR_FAIL;
    }
#endif

    if ( status == REGERR_OK )
    {
        ++regStartCount;
        if ( regStartCount == 1 )
        {
            /* first time only initialization */
            vr_findGlobalRegName();

#ifndef STANDALONE_REGISTRY
            /* initialization for version registry */
            vr_lock = PR_NewLock();
            XP_ASSERT( vr_lock != NULL );
#if defined(XP_UNIX) && !defined(XP_MACOSX)
            bGlobalRegistry = ( getenv(UNIX_GLOBAL_FLAG) != NULL );
#endif
#endif 
        } /* if ( regStartCount == 1 ) */

        PR_Unlock( reglist_lock );
    }

    return status;
}   /* NR_StartupRegistry */

VR_INTERFACE(void) NR_ShutdownRegistry(void)
{
    REGFILE* pReg;
    XP_Bool  bDestroyLocks = FALSE;

    /* people should track whether NR_StartupRegistry() was successful
     * and not call this if it fails... but they won't so we'll try to
     * handle that case gracefully.
     */
#ifndef STANDALONE_REGISTRY
    if ( reglist_lock == NULL ) 
        return;  /* was not started successfully */
#else
    if ( regStartCount == 0 )
        return;  /* was not started successfully */
#endif

    PR_Lock( reglist_lock );

    --regStartCount;
    if ( regStartCount == 0 )
    {
        /* shutdown for real. */

        /* close any forgotten open registries */
        while ( RegList != NULL ) 
        {
            pReg = RegList;
            if ( pReg->hdrDirty ) {
                nr_WriteHdr( pReg );
            }
            nr_CloseFile( &(pReg->fh) );
            nr_DeleteNode( pReg );
        }
    
        XP_FREEIF(user_name);
        XP_FREEIF(globalRegName);
        XP_FREEIF(verRegName);

        bDestroyLocks = TRUE;
    }

    PR_Unlock( reglist_lock );

#ifndef STANDALONE_REGISTRY    
    if ( bDestroyLocks ) 
    {
        PR_DestroyLock( reglist_lock );
        reglist_lock = NULL;

        PR_DestroyLock(vr_lock);
        vr_lock = NULL;
    }
#endif

}   /* NR_ShutdownRegistry */

#ifdef XP_MAC
#pragma export reset
#endif

/* EOF: reg.c */
