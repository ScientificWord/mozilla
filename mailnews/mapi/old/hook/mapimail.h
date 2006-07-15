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
 * ***** END LICENSE BLOCK ***** */
#ifndef _MAPI_MAIL_H_
#define _MAPI_MAIL_H_

#include "nscpmapi.h"
#include <structs.h>    // for MWContext

//extern "C" {

//
// This function will create a composition window and either do
// a blind send or pop up the compose window for the user to 
// complete the operation
//
// Return: appropriate MAPI return code...
//
//
extern "C" LONG
DoFullMAPIMailOperation(MAPISendMailType      *sendMailPtr,
											  const char            *pInitialText,
                        BOOL                  winShowFlag);

//
// This function will create a composition window and just attach
// the attachments of interest and pop up the window...
//
// Return: appropriate MAPI return code...
//
//
extern "C" LONG
DoPartialMAPIMailOperation(MAPISendDocumentsType *sendDocPtr);

//
// This function will save a message into the Communicator "Drafts"
// folder with no UI showing.
//
// Return: appropriate MAPI return code...
//
//
extern "C" LONG 
DoMAPISaveMailOperation(MAPISendMailType      *sendMailPtr,
											  const char            *pInitialText);

//
// This will fire off a "get mail in background operation" in an
// async. fashion.
//
extern "C" void
MAPIGetNewMessagesInBackground(void);


// } // extern "C"

#endif // _MAPI_MAIL_H_
