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

#ifndef nsIEditorCommand_h_
#define nsIEditorCommand_h_

#include "nsCOMPtr.h"
#include "nsIControllerCommand.h"
#include "nsIAtom.h"

// This is a virtual base class for commands registered with the editor controller.
// Note that such commands can be shared by more than on editor instance, so
// MUST be stateless. Any state must be stored via the refCon (an nsIEditor).
class nsBaseEditorCommand : public nsIControllerCommand
{
public:

              nsBaseEditorCommand();
  virtual     ~nsBaseEditorCommand() {}
    
  NS_DECL_ISUPPORTS
    
  NS_IMETHOD  IsCommandEnabled(const char * aCommandName, nsISupports *aCommandRefCon, PRBool *_retval) = 0;
  NS_IMETHOD  DoCommand(const char *aCommandName, nsISupports *aCommandRefCon) = 0;
  
};


#define NS_DECL_EDITOR_COMMAND(_cmd)                    \
class _cmd : public nsBaseEditorCommand                 \
{                                                       \
public:                                                 \
  NS_IMETHOD IsCommandEnabled(const char * aCommandName, nsISupports *aCommandRefCon, PRBool *_retval); \
  NS_IMETHOD DoCommand(const char *aCommandName, nsISupports *aCommandRefCon); \
  NS_IMETHOD DoCommandParams(const char *aCommandName,nsICommandParams *aParams, nsISupports *aCommandRefCon); \
  NS_IMETHOD GetCommandStateParams(const char *aCommandName,nsICommandParams *aParams, nsISupports *aCommandRefCon); \
};



// basic editor commands
NS_DECL_EDITOR_COMMAND(nsUndoCommand)
NS_DECL_EDITOR_COMMAND(nsRedoCommand)
NS_DECL_EDITOR_COMMAND(nsClearUndoCommand)

NS_DECL_EDITOR_COMMAND(nsCutCommand)
NS_DECL_EDITOR_COMMAND(nsCutOrDeleteCommand)
NS_DECL_EDITOR_COMMAND(nsCopyCommand)
NS_DECL_EDITOR_COMMAND(nsCopyOrDeleteCommand)
NS_DECL_EDITOR_COMMAND(nsPasteCommand)
NS_DECL_EDITOR_COMMAND(nsSwitchTextDirectionCommand)
NS_DECL_EDITOR_COMMAND(nsDeleteCommand)
NS_DECL_EDITOR_COMMAND(nsSelectAllCommand)

NS_DECL_EDITOR_COMMAND(nsSelectionMoveCommands)

// Insert content commands
NS_DECL_EDITOR_COMMAND(nsInsertPlaintextCommand)
NS_DECL_EDITOR_COMMAND(nsPasteQuotationCommand)


#if 0
// template for new command
NS_IMETHODIMP
nsFooCommand::IsCommandEnabled(const char * aCommandName, nsISupports *aCommandRefCon, PRBool *_retval)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


NS_IMETHODIMP
nsFooCommand::DoCommand(const char *aCommandName, const nsAString & aCommandParams, nsISupports *aCommandRefCon)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}


#endif

#endif // nsIEditorCommand_h_
