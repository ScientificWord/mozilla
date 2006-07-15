/*
 * This file essentially replicates NSPR's source for the functions that
 * map system-specific error codes to NSPR error codes.  We would use 
 * NSPR's functions, instead of duplicating them, but they're private.
 * As long as SSL's server session cache code must do platform native I/O
 * to accomplish its job, and NSPR's error mapping functions remain private,
 * This code will continue to need to be replicated.
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
/* $Id$ */

/*  NSPR doesn't make these functions public, so we have to duplicate
**  them in NSS.
*/
extern void nss_MD_win32_map_accept_error(PRInt32 err);
extern void nss_MD_win32_map_acceptex_error(PRInt32 err);
extern void nss_MD_win32_map_access_error(PRInt32 err);
extern void nss_MD_win32_map_bind_error(PRInt32 err);
extern void nss_MD_win32_map_close_error(PRInt32 err);
extern void nss_MD_win32_map_closedir_error(PRInt32 err);
extern void nss_MD_win32_map_connect_error(PRInt32 err);
extern void nss_MD_win32_map_default_error(PRInt32 err);
extern void nss_MD_win32_map_delete_error(PRInt32 err);
extern void nss_MD_win32_map_fstat_error(PRInt32 err);
extern void nss_MD_win32_map_fsync_error(PRInt32 err);
extern void nss_MD_win32_map_gethostname_error(PRInt32 err);
extern void nss_MD_win32_map_getpeername_error(PRInt32 err);
extern void nss_MD_win32_map_getsockname_error(PRInt32 err);
extern void nss_MD_win32_map_getsockopt_error(PRInt32 err);
extern void nss_MD_win32_map_listen_error(PRInt32 err);
extern void nss_MD_win32_map_lockf_error(PRInt32 err);
extern void nss_MD_win32_map_lseek_error(PRInt32 err);
extern void nss_MD_win32_map_mkdir_error(PRInt32 err);
extern void nss_MD_win32_map_open_error(PRInt32 err);
extern void nss_MD_win32_map_opendir_error(PRInt32 err);
extern void nss_MD_win32_map_read_error(PRInt32 err);
extern void nss_MD_win32_map_readdir_error(PRInt32 err);
extern void nss_MD_win32_map_recv_error(PRInt32 err);
extern void nss_MD_win32_map_recvfrom_error(PRInt32 err);
extern void nss_MD_win32_map_rename_error(PRInt32 err);
extern void nss_MD_win32_map_rmdir_error(PRInt32 err);
extern void nss_MD_win32_map_select_error(PRInt32 err);
extern void nss_MD_win32_map_send_error(PRInt32 err);
extern void nss_MD_win32_map_sendto_error(PRInt32 err);
extern void nss_MD_win32_map_setsockopt_error(PRInt32 err);
extern void nss_MD_win32_map_shutdown_error(PRInt32 err);
extern void nss_MD_win32_map_socket_error(PRInt32 err);
extern void nss_MD_win32_map_stat_error(PRInt32 err);
extern void nss_MD_win32_map_transmitfile_error(PRInt32 err);
extern void nss_MD_win32_map_write_error(PRInt32 err);
