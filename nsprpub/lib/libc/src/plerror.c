/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is the Netscape Portable Runtime (NSPR).
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-2000
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

/*
** File:plerror.c
** Description: Simple routine to print translate the calling thread's
**  error numbers and print them to "syserr".
*/

#include "plerror.h"

#include "prprf.h"
#include "prerror.h"

PR_IMPLEMENT(void) PL_FPrintError(PRFileDesc *fd, const char *msg)
{
static const char *tags[] =
{
    "PR_OUT_OF_MEMORY_ERROR",
    "PR_BAD_DESCRIPTOR_ERROR", 
    "PR_WOULD_BLOCK_ERROR",
    "PR_ACCESS_FAULT_ERROR", 
    "PR_INVALID_METHOD_ERROR", 
    "PR_ILLEGAL_ACCESS_ERROR", 
    "PR_UNKNOWN_ERROR",
    "PR_PENDING_INTERRUPT_ERROR",
    "PR_NOT_IMPLEMENTED_ERROR",
    "PR_IO_ERROR", 
    "PR_IO_TIMEOUT_ERROR", 
    "PR_IO_PENDING_ERROR", 
    "PR_DIRECTORY_OPEN_ERROR", 
    "PR_INVALID_ARGUMENT_ERROR", 
    "PR_ADDRESS_NOT_AVAILABLE_ERROR",
    "PR_ADDRESS_NOT_SUPPORTED_ERROR",
    "PR_IS_CONNECTED_ERROR", 
    "PR_BAD_ADDRESS_ERROR",
    "PR_ADDRESS_IN_USE_ERROR", 
    "PR_CONNECT_REFUSED_ERROR",
    "PR_NETWORK_UNREACHABLE_ERROR",
    "PR_CONNECT_TIMEOUT_ERROR",
    "PR_NOT_CONNECTED_ERROR",
    "PR_LOAD_LIBRARY_ERROR", 
    "PR_UNLOAD_LIBRARY_ERROR", 
    "PR_FIND_SYMBOL_ERROR",
    "PR_INSUFFICIENT_RESOURCES_ERROR", 
    "PR_DIRECTORY_LOOKUP_ERROR", 
    "PR_TPD_RANGE_ERROR",
    "PR_PROC_DESC_TABLE_FULL_ERROR", 
    "PR_SYS_DESC_TABLE_FULL_ERROR",
    "PR_NOT_SOCKET_ERROR", 
    "PR_NOT_TCP_SOCKET_ERROR", 
    "PR_SOCKET_ADDRESS_IS_BOUND_ERROR",
    "PR_NO_ACCESS_RIGHTS_ERROR", 
    "PR_OPERATION_NOT_SUPPORTED_ERROR",
    "PR_PROTOCOL_NOT_SUPPORTED_ERROR", 
    "PR_REMOTE_FILE_ERROR",
    "PR_BUFFER_OVERFLOW_ERROR",
    "PR_CONNECT_RESET_ERROR",
    "PR_RANGE_ERROR",
    "PR_DEADLOCK_ERROR", 
    "PR_FILE_IS_LOCKED_ERROR", 
    "PR_FILE_TOO_BIG_ERROR", 
    "PR_NO_DEVICE_SPACE_ERROR",
    "PR_PIPE_ERROR", 
    "PR_NO_SEEK_DEVICE_ERROR", 
    "PR_IS_DIRECTORY_ERROR", 
    "PR_LOOP_ERROR", 
    "PR_NAME_TOO_LONG_ERROR",
    "PR_FILE_NOT_FOUND_ERROR", 
    "PR_NOT_DIRECTORY_ERROR",
    "PR_READ_ONLY_FILESYSTEM_ERROR", 
    "PR_DIRECTORY_NOT_EMPTY_ERROR",
    "PR_FILESYSTEM_MOUNTED_ERROR", 
    "PR_NOT_SAME_DEVICE_ERROR",
    "PR_DIRECTORY_CORRUPTED_ERROR",
    "PR_FILE_EXISTS_ERROR",
    "PR_MAX_DIRECTORY_ENTRIES_ERROR",
    "PR_INVALID_DEVICE_STATE_ERROR", 
    "PR_DEVICE_IS_LOCKED_ERROR", 
    "PR_NO_MORE_FILES_ERROR",
    "PR_END_OF_FILE_ERROR",
    "PR_FILE_SEEK_ERROR",
    "PR_FILE_IS_BUSY_ERROR", 
    "<unused error code>",
    "PR_IN_PROGRESS_ERROR",
    "PR_ALREADY_INITIATED_ERROR",
    "PR_GROUP_EMPTY_ERROR",
    "PR_INVALID_STATE_ERROR",
    "PR_NETWORK_DOWN_ERROR",
    "PR_SOCKET_SHUTDOWN_ERROR",
    "PR_CONNECT_ABORTED_ERROR",
    "PR_HOST_UNREACHABLE_ERROR",
    "PR_MAX_ERROR"
};

PRErrorCode error = PR_GetError();
PRInt32 oserror = PR_GetOSError();
PRIntn thoseIKnowAbout = sizeof(tags) / sizeof(char*);
PRIntn lastError = PR_NSPR_ERROR_BASE + thoseIKnowAbout;

	if (NULL != msg) PR_fprintf(fd, "%s: ", msg);
    if ((error < PR_NSPR_ERROR_BASE) || (error > lastError))
        PR_fprintf(
			fd, " (%d)OUT OF RANGE, oserror = %d\n", error, oserror);
    else
        PR_fprintf(
            fd, "%s(%d), oserror = %d\n",
            tags[error - PR_NSPR_ERROR_BASE], error, oserror);
}  /* PL_FPrintError */

PR_IMPLEMENT(void) PL_PrintError(const char *msg)
{
	static PRFileDesc *fd = NULL;
	if (NULL == fd) fd = PR_GetSpecialFD(PR_StandardError);
	PL_FPrintError(fd, msg);
}  /* PL_PrintError */

#if defined(WIN16)
/*
** libmain() is a required function for win16
**
*/
int CALLBACK LibMain( HINSTANCE hInst, WORD wDataSeg, 
  WORD cbHeapSize, LPSTR lpszCmdLine )
{
return TRUE;
}
#endif /* WIN16 */





/* plerror.c */
