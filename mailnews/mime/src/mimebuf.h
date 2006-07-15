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
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK *****
 * This Original Code has been modified by IBM Corporation. Modifications made by IBM 
 * described herein are Copyright (c) International Business Machines Corporation, 2000.
 * Modifications to Mozilla code or documentation identified per MPL Section 3.3
 *
 * Date             Modified by     Description of modification
 * 04/20/2000       IBM Corp.      OS/2 VisualAge build.
 */
 
#ifndef _MIMEBUF_H_
#define _MIMEBUF_H_

extern "C" int mime_GrowBuffer (PRUint32 desired_size,
						   PRUint32 element_size, PRUint32 quantum,
						   char **buffer, PRInt32 *size);

extern "C" int mime_LineBuffer (const char *net_buffer, PRInt32 net_buffer_size,
						   char **bufferP, PRInt32 *buffer_sizeP,
						   PRInt32 *buffer_fpP,
						   PRBool convert_newlines_p,
						   PRInt32 (*PR_CALLBACK per_line_fn) (char *line, PRInt32
												 line_length, void *closure),
						   void *closure);
						   
extern "C" int mime_ReBuffer (const char *net_buffer, PRInt32 net_buffer_size,
						 PRUint32 desired_buffer_size,
						 char **bufferP, PRUint32 *buffer_sizeP,
						 PRUint32 *buffer_fpP,
						 PRInt32 (*per_buffer_fn) (char *buffer,
												 PRUint32 buffer_size,
												 void *closure),
						 void *closure);


#endif /* _MIMEBUF_H_ */
