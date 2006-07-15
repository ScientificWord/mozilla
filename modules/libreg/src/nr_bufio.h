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
 * The Original Code is Mozilla Communicator.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Daniel Veditz <dveditz@netscape.com>
 *   Edward Kandrot <kandrot@netscape.com>
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

/* nr_bufio.h
 * Buffered I/O routines to improve registry performance
 * 
 * the routines mirror fopen(), fclose() et al
 *
 * __NOTE__: the filenames are *native* filenames, not NSPR names.
 */

#ifndef _NR_BUFIO_H_
#define _NR_BUFIO_H_

typedef struct BufioFileStruct BufioFile;

BufioFile*  bufio_Open(const char* name, const char* mode);
int         bufio_Close(BufioFile* file);
int         bufio_Seek(BufioFile* file, PRInt32 offset, int whence);
PRUint32    bufio_Read(BufioFile* file, char* dest, PRUint32 count);
PRUint32    bufio_Write(BufioFile* file, const char* src, PRUint32 count);
PRInt32     bufio_Tell(BufioFile* file);
int         bufio_Flush(BufioFile* file);
int         bufio_SetBufferSize(BufioFile* file, int bufsize);

#endif  /* _NR_BUFIO_H_ */

