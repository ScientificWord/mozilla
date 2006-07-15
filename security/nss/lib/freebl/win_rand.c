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
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
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

#include "secrng.h"
#ifdef XP_WIN
#include <windows.h>

#if defined(_WIN32_WCE)
#include <stdlib.h>	/* Win CE puts lots of stuff here. */
#include "prprf.h"	/* for PR_snprintf */
#else
#include <time.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <stdio.h>

#ifndef _WIN32
#define VTD_Device_ID   5
#define OP_OVERRIDE     _asm _emit 0x66
#include <dos.h>
#endif

#include "prio.h"

static PRInt32  filesToRead;
static DWORD    totalFileBytes;
static DWORD    maxFileBytes	= 250000;	/* 250 thousand */
static DWORD    dwNumFiles, dwReadEvery;

static BOOL
CurrentClockTickTime(LPDWORD lpdwHigh, LPDWORD lpdwLow)
{
#ifdef _WIN32
    LARGE_INTEGER   liCount;

    if (!QueryPerformanceCounter(&liCount))
        return FALSE;

    *lpdwHigh = liCount.u.HighPart;
    *lpdwLow = liCount.u.LowPart;
    return TRUE;

#else   /* is WIN16 */
    BOOL    bRetVal;
    FARPROC lpAPI;
    WORD    w1, w2, w3, w4;

    // Get direct access to the VTD and query the current clock tick time
    _asm {
        xor   di, di
        mov   es, di
        mov   ax, 1684h
        mov   bx, VTD_Device_ID
        int   2fh
        mov   ax, es
        or    ax, di
        jz    EnumerateFailed

        ; VTD API is available. First store the API address (the address actually
        ; contains an instruction that causes a fault, the fault handler then
        ; makes the ring transition and calls the API in the VxD)
        mov   word ptr lpAPI, di
        mov   word ptr lpAPI+2, es
        mov   ax, 100h      ; API function to VTD_Get_Real_Time
;       call  dword ptr [lpAPI]
        call  [lpAPI]

        ; Result is in EDX:EAX which we will get 16-bits at a time
        mov   w2, dx
        OP_OVERRIDE
        shr   dx,10h        ; really "shr edx, 16"
        mov   w1, dx

        mov   w4, ax
        OP_OVERRIDE
        shr   ax,10h        ; really "shr eax, 16"
        mov   w3, ax

        mov   bRetVal, 1    ; return TRUE
        jmp   EnumerateExit

      EnumerateFailed:
        mov   bRetVal, 0    ; return FALSE

      EnumerateExit:
    }

    *lpdwHigh = MAKELONG(w2, w1);
    *lpdwLow = MAKELONG(w4, w3);

    return bRetVal;
#endif  /* is WIN16 */
}

size_t RNG_GetNoise(void *buf, size_t maxbuf)
{
    DWORD   dwHigh, dwLow, dwVal;
    int     n = 0;
    int     nBytes;

    if (maxbuf <= 0)
        return 0;

    CurrentClockTickTime(&dwHigh, &dwLow);

    // get the maximally changing bits first
    nBytes = sizeof(dwLow) > maxbuf ? maxbuf : sizeof(dwLow);
    memcpy((char *)buf, &dwLow, nBytes);
    n += nBytes;
    maxbuf -= nBytes;

    if (maxbuf <= 0)
        return n;

    nBytes = sizeof(dwHigh) > maxbuf ? maxbuf : sizeof(dwHigh);
    memcpy(((char *)buf) + n, &dwHigh, nBytes);
    n += nBytes;
    maxbuf -= nBytes;

    if (maxbuf <= 0)
        return n;

    // get the number of milliseconds that have elapsed since Windows started
    dwVal = GetTickCount();

    nBytes = sizeof(dwVal) > maxbuf ? maxbuf : sizeof(dwVal);
    memcpy(((char *)buf) + n, &dwVal, nBytes);
    n += nBytes;
    maxbuf -= nBytes;

    if (maxbuf <= 0)
        return n;

#if defined(_WIN32_WCE)
    {
    // get the number of milliseconds elapsed since Windows CE was started. 
    DWORD  tickCount = GetTickCount();
    nBytes = (sizeof tickCount) > maxbuf ? maxbuf : (sizeof tickCount);
    memcpy(((char *)buf) + n, &tickCount, nBytes);
    n += nBytes;
    }
#else
    {
    time_t  sTime;
    // get the time in seconds since midnight Jan 1, 1970
    time(&sTime);
    nBytes = sizeof(sTime) > maxbuf ? maxbuf : sizeof(sTime);
    memcpy(((char *)buf) + n, &sTime, nBytes);
    n += nBytes;
    }
#endif

    return n;
}

#if defined(_WIN32_WCE)
static BOOL
EnumSystemFilesWithNSPR(const char * dirName, 
			BOOL recursive,
                        PRInt32 (*func)(const char *))
{
    PRDir *      pDir;
    PRDirEntry * pEntry;
    BOOL         rv 		= FALSE;

    pDir = PR_OpenDir(dirName);
    if (!pDir)
    	return rv;
    while ((pEntry = PR_ReadDir(pDir, PR_SKIP_BOTH|PR_SKIP_HIDDEN)) != NULL) {
	PRStatus    status;
	PRInt32     count;
	PRInt32     stop;
	PRFileInfo  fileInfo;
	char        szFileName[_MAX_PATH];

        count = (PRInt32)PR_snprintf(szFileName, sizeof szFileName, "%s\\%s", 
	                             dirName, PR_DirName(pEntry));
	if (count < 1)
	    continue;
	status = PR_GetFileInfo(szFileName, &fileInfo);
	if (status != PR_SUCCESS)
	    continue;
	if (fileInfo.type == PR_FILE_FILE) {
	    stop = (*func)(szFileName);
	    rv = TRUE;
	    if (stop)
	        break;
	    continue;
    	}
	if (recursive && fileInfo.type == PR_FILE_DIRECTORY) {
	    rv |= EnumSystemFilesWithNSPR(szFileName, recursive, func);
	}
    }
    PR_CloseDir(pDir);
    return rv;
}
#endif

static BOOL
EnumSystemFiles(PRInt32 (*func)(const char *))
{
#if defined(_WIN32_WCE)
    BOOL rv = FALSE;
    rv |= EnumSystemFilesWithNSPR("\\Windows\\Temporary Internet Files", TRUE, func);
    rv |= EnumSystemFilesWithNSPR("\\Temp",    FALSE, func);
    rv |= EnumSystemFilesWithNSPR("\\Windows", FALSE, func);
    return rv;
#else
    int                 iStatus;
    char                szSysDir[_MAX_PATH];
    char                szFileName[_MAX_PATH];
#ifdef _WIN32
    struct _finddata_t  fdData;
    long                lFindHandle;
#else
    struct _find_t  fdData;
#endif

    if (!GetSystemDirectory(szSysDir, sizeof(szSysDir)))
        return FALSE;

    // tack *.* on the end so we actually look for files. this will
    // not overflow
    strcpy(szFileName, szSysDir);
    strcat(szFileName, "\\*.*");

#ifdef _WIN32
    lFindHandle = _findfirst(szFileName, &fdData);
    if (lFindHandle == -1)
        return FALSE;
#else
    if (_dos_findfirst(szFileName, _A_NORMAL | _A_RDONLY | _A_ARCH | _A_SUBDIR, &fdData) != 0)
        return FALSE;
#endif

    do {
        // pass the full pathname to the callback
        sprintf(szFileName, "%s\\%s", szSysDir, fdData.name);
        (*func)(szFileName);

#ifdef _WIN32
        iStatus = _findnext(lFindHandle, &fdData);
#else
        iStatus = _dos_findnext(&fdData);
#endif
    } while (iStatus == 0);

#ifdef _WIN32
    _findclose(lFindHandle);
#endif

    return TRUE;
#endif
}

static PRInt32
CountFiles(const char *file)
{
    dwNumFiles++;
    return 0;
}

static PRInt32
ReadFiles(const char *file)
{
    if ((dwNumFiles % dwReadEvery) == 0) {
	++filesToRead;
    }
    if (filesToRead) {
	DWORD    prevFileBytes = totalFileBytes;
        RNG_FileForRNG(file);
	if (prevFileBytes < totalFileBytes) {
	    --filesToRead;
	}
    }
    dwNumFiles++;
    return (totalFileBytes >= maxFileBytes);
}

static void
ReadSystemFiles()
{
    // first count the number of files
    dwNumFiles = 0;
    if (!EnumSystemFiles(CountFiles))
        return;

    RNG_RandomUpdate(&dwNumFiles, sizeof(dwNumFiles));

    // now read 10 files
    filesToRead = 10;
    if (dwNumFiles == 0)
        return;

    dwReadEvery = dwNumFiles / 10;
    if (dwReadEvery == 0)
        dwReadEvery = 1;  // less than 10 files

    dwNumFiles = 0;
    EnumSystemFiles(ReadFiles);
}

void RNG_SystemInfoForRNG(void)
{
    DWORD           dwVal;
    char            buffer[256];
    int             nBytes;
#ifdef _WIN32
    MEMORYSTATUS    sMem;
    HANDLE          hVal;
#if !defined(_WIN32_WCE)
    DWORD           dwSerialNum;
    DWORD           dwComponentLen;
    DWORD           dwSysFlags;
    char            volName[128];
    DWORD           dwSectors, dwBytes, dwFreeClusters, dwNumClusters;
#endif
#else
    int             iVal;
    HTASK           hTask;
    WORD            wDS, wCS;
    LPSTR           lpszEnv;
#endif

    nBytes = RNG_GetNoise(buffer, 20);  // get up to 20 bytes
    RNG_RandomUpdate(buffer, nBytes);

#ifdef _WIN32
    sMem.dwLength = sizeof(sMem);
    GlobalMemoryStatus(&sMem);                // assorted memory stats
    RNG_RandomUpdate(&sMem, sizeof(sMem));
#if !defined(_WIN32_WCE)
    dwVal = GetLogicalDrives();
    RNG_RandomUpdate(&dwVal, sizeof(dwVal));  // bitfields in bits 0-25
#endif
#else
    dwVal = GetFreeSpace(0);
    RNG_RandomUpdate(&dwVal, sizeof(dwVal));

    _asm    mov wDS, ds;
    _asm    mov wCS, cs;
    RNG_RandomUpdate(&wDS, sizeof(wDS));
    RNG_RandomUpdate(&wCS, sizeof(wCS));
#endif

#ifdef _WIN32
#if !defined(_WIN32_WCE)
    dwVal = sizeof(buffer);
    if (GetComputerName(buffer, &dwVal))
        RNG_RandomUpdate(buffer, dwVal);
#endif
/* XXX This is code that got yanked because of NSPR20.  We should put it
 * back someday.
 */
#ifdef notdef
    {
    POINT ptVal;
    GetCursorPos(&ptVal);
    RNG_RandomUpdate(&ptVal, sizeof(ptVal));
    }

    dwVal = GetQueueStatus(QS_ALLINPUT);      // high and low significant
    RNG_RandomUpdate(&dwVal, sizeof(dwVal));

    {
    HWND hWnd;
    hWnd = GetClipboardOwner();               // 2 or 4 bytes
    RNG_RandomUpdate((void *)&hWnd, sizeof(hWnd));
    }

    {
    UUID sUuid;
    UuidCreate(&sUuid);                       // this will fail on machines with no ethernet
    RNG_RandomUpdate(&sUuid, sizeof(sUuid));  // boards. shove the bits in regardless
    }
#endif

    hVal = GetCurrentProcess();               // 4 byte handle of current task
    RNG_RandomUpdate(&hVal, sizeof(hVal));

    dwVal = GetCurrentProcessId();            // process ID (4 bytes)
    RNG_RandomUpdate(&dwVal, sizeof(dwVal));

#if !defined(_WIN32_WCE)
    volName[0] = '\0';
    buffer[0] = '\0';
    GetVolumeInformation(NULL,
                         volName,
                         sizeof(volName),
                         &dwSerialNum,
                         &dwComponentLen,
                         &dwSysFlags,
                         buffer,
                         sizeof(buffer));

    RNG_RandomUpdate(volName,         strlen(volName));
    RNG_RandomUpdate(&dwSerialNum,    sizeof(dwSerialNum));
    RNG_RandomUpdate(&dwComponentLen, sizeof(dwComponentLen));
    RNG_RandomUpdate(&dwSysFlags,     sizeof(dwSysFlags));
    RNG_RandomUpdate(buffer,          strlen(buffer));

    if (GetDiskFreeSpace(NULL, &dwSectors, &dwBytes, &dwFreeClusters, &dwNumClusters)) {
        RNG_RandomUpdate(&dwSectors,      sizeof(dwSectors));
        RNG_RandomUpdate(&dwBytes,        sizeof(dwBytes));
        RNG_RandomUpdate(&dwFreeClusters, sizeof(dwFreeClusters));
        RNG_RandomUpdate(&dwNumClusters,  sizeof(dwNumClusters));
    }
#endif
#else   /* is WIN16 */
    hTask = GetCurrentTask();
    RNG_RandomUpdate((void *)&hTask, sizeof(hTask));

    iVal = GetNumTasks();
    RNG_RandomUpdate(&iVal, sizeof(iVal));      // number of running tasks

    lpszEnv = GetDOSEnvironment();
    while (*lpszEnv != '\0') {
        RNG_RandomUpdate(lpszEnv, strlen(lpszEnv));

        lpszEnv += strlen(lpszEnv) + 1;
    }
#endif  /* is WIN16 */

    // now let's do some files
    ReadSystemFiles();

    nBytes = RNG_GetNoise(buffer, 20);  // get up to 20 bytes
    RNG_RandomUpdate(buffer, nBytes);
}

#if defined(_WIN32_WCE)
void RNG_FileForRNG(const char *filename)
{
    PRFileDesc *    file;
    int             nBytes;
    PRFileInfo      infoBuf;
    unsigned char   buffer[1024];

    /* windows doesn't initialize all the bytes in the stat buf,
     * so initialize them all here to avoid UMRs.
     */
    memset(&infoBuf, 0, sizeof infoBuf);

    if (PR_GetFileInfo(filename, &infoBuf) < 0)
        return;

    RNG_RandomUpdate((unsigned char*)&infoBuf, sizeof(infoBuf));

    file = PR_Open(filename, PR_RDONLY, 0);
    if (file != NULL) {
        for (;;) {
            PRInt32 bytes = PR_Read(file, buffer, sizeof buffer);

            if (bytes <= 0)
                break;

            RNG_RandomUpdate(buffer, bytes);
            totalFileBytes += bytes;
            if (totalFileBytes > maxFileBytes)
                break;
        }

        PR_Close(file);
    }

    nBytes = RNG_GetNoise(buffer, 20);  // get up to 20 bytes
    RNG_RandomUpdate(buffer, nBytes);
}

#else /* not WinCE */

void RNG_FileForRNG(const char *filename)
{
    FILE*           file;
    int             nBytes;
    struct stat     stat_buf;
    unsigned char   buffer[1024];

   /* static DWORD    totalFileBytes = 0; */

    /* windows doesn't initialize all the bytes in the stat buf,
     * so initialize them all here to avoid UMRs.
     */
    memset(&stat_buf, 0, sizeof stat_buf);

    if (stat((char *)filename, &stat_buf) < 0)
        return;

    RNG_RandomUpdate((unsigned char*)&stat_buf, sizeof(stat_buf));

    file = fopen((char *)filename, "r");
    if (file != NULL) {
        for (;;) {
            size_t  bytes = fread(buffer, 1, sizeof(buffer), file);

            if (bytes == 0)
                break;

            RNG_RandomUpdate(buffer, bytes);
            totalFileBytes += bytes;
            if (totalFileBytes > maxFileBytes)
                break;
        }

        fclose(file);
    }

    nBytes = RNG_GetNoise(buffer, 20);  // get up to 20 bytes
    RNG_RandomUpdate(buffer, nBytes);
}

#endif  /* not WinCE */
#endif  /* is XP_WIN */
