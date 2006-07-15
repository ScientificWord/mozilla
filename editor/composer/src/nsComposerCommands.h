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
 * Portions created by the Initial Developer are Copyright (C) 1998-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ryan Cassin      <rcassin@supernova.org>
 *   Daniel Glazman   <glazman@netscape.com>
 *   Charles Manske   <cmanske@netscape.com>
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

#ifndef nsComposerCommands_h_
#define nsComposerCommands_h_

#include "nsIControllerCommand.h"
#include "nsString.h"

class nsIEditor;

// This is a virtual base class for commands registered with the composer controller.
// Note that such commands are instantiated once per composer, so can store state.
// Also note that IsCommandEnabled can be called with an editor that may not
// have an editor yet (because the document is loading). Most commands will want
// to return false in this case.
// Don't hold on to any references to the editor or document from
// your command. This will cause leaks. Also, be aware that the document the
// editor is editing can change under you (if the user Reverts the file, for
// instance).
class nsBaseComposerCommand : public nsIControllerCommand
{
public:

              nsBaseComposerCommand();
  virtual     ~nsBaseComposerCommand() {}
    
  // nsISupports
  NS_DECL_ISUPPORTS
    
  // nsIControllerCommand. Declared longhand so we can make them pure virtual
  NS_IMETHOD IsCommandEnabled(const char * aCommandName, nsISupports *aCommandRefCon, PRBool *_retval) = 0;
  NS_IMETHOD DoCommand(const char * aCommandName, nsISupports *aCommandRefCon) = 0;

};


#define NS_DECL_COMPOSER_COMMAND(_cmd)                  \
class _cmd : public nsBaseComposerCommand               \
{                                                       \
public:                                                 \
  NS_DECL_NSICONTROLLERCOMMAND                          \
};

// virtual base class for commands that need to save and update Boolean state (like styles etc)
class nsBaseStateUpdatingCommand : public nsBaseComposerCommand
{
public:

              nsBaseStateUpdatingCommand(const char* aTagName);
  virtual     ~nsBaseStateUpdatingCommand();
    
  NS_DECL_ISUPPORTS_INHERITED

  NS_DECL_NSICONTROLLERCOMMAND

protected:

  // get the current state (on or off) for this style or block format
  virtual nsresult  GetCurrentState(nsIEditor *aEditor, const char* aTagName, nsICommandParams *aParams) = 0;
  
  // add/remove the style
  virtual nsresult  ToggleState(nsIEditor *aEditor, const char* aTagName) = 0;

protected:

  const char* mTagName;
};


// Shared class for the various style updating commands like bold, italics etc.
// Suitable for commands whose state is either 'on' or 'off'.
class nsStyleUpdatingCommand : public nsBaseStateUpdatingCommand
{
public:

            nsStyleUpdatingCommand(const char* aTagName);
           
protected:

  // get the current state (on or off) for this style or block format
  virtual nsresult  GetCurrentState(nsIEditor *aEditor, const char* aTagName, nsICommandParams *aParams);
  
  // add/remove the style
  virtual nsresult  ToggleState(nsIEditor *aEditor, const char* aTagName);
  
};


class nsInsertTagCommand : public nsBaseComposerCommand
{
public:

              nsInsertTagCommand(const char* aTagName);
  virtual     ~nsInsertTagCommand();
    
  NS_DECL_ISUPPORTS_INHERITED

  NS_DECL_NSICONTROLLERCOMMAND

protected:

  const char* mTagName;
};


class nsListCommand : public nsBaseStateUpdatingCommand
{
public:

            nsListCommand(const char* aTagName);

protected:

  // get the current state (on or off) for this style or block format
  virtual nsresult  GetCurrentState(nsIEditor *aEditor, const char* aTagName, nsICommandParams *aParams);
  
  // add/remove the style
  virtual nsresult  ToggleState(nsIEditor *aEditor, const char* aTagName);
};

class nsListItemCommand : public nsBaseStateUpdatingCommand
{
public:

            nsListItemCommand(const char* aTagName);

protected:

  // get the current state (on or off) for this style or block format
  virtual nsresult  GetCurrentState(nsIEditor *aEditor, const char* aTagName, nsICommandParams *aParams);
  
  // add/remove the style
  virtual nsresult  ToggleState(nsIEditor *aEditor, const char* aTagName);
};

// Base class for commands whose state consists of a string (e.g. para format)
class nsMultiStateCommand : public nsBaseComposerCommand
{
public:
  
                   nsMultiStateCommand();
  virtual          ~nsMultiStateCommand();
  
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSICONTROLLERCOMMAND

protected:

  virtual nsresult GetCurrentState(nsIEditor *aEditor, nsICommandParams* aParams) =0;
  virtual nsresult SetState(nsIEditor *aEditor, nsString& newState) = 0;
  
};


class nsParagraphStateCommand : public nsMultiStateCommand
{
public:
                   nsParagraphStateCommand();

protected:

  virtual nsresult GetCurrentState(nsIEditor *aEditor, nsICommandParams* aParams);
  virtual nsresult SetState(nsIEditor *aEditor, nsString& newState);
};

class nsFontFaceStateCommand : public nsMultiStateCommand
{
public:
                   nsFontFaceStateCommand();

protected:

  virtual nsresult GetCurrentState(nsIEditor *aEditor, nsICommandParams* aParams);
  virtual nsresult SetState(nsIEditor *aEditor, nsString& newState);
};

class nsFontSizeStateCommand : public nsMultiStateCommand
{
public:
                   nsFontSizeStateCommand();

protected:

  virtual nsresult GetCurrentState(nsIEditor *aEditor,
                                   nsICommandParams* aParams);
  virtual nsresult SetState(nsIEditor *aEditor, nsString& newState);
};

class nsHighlightColorStateCommand : public nsMultiStateCommand
{
public:
                   nsHighlightColorStateCommand();

protected:

  NS_IMETHOD IsCommandEnabled(const char *aCommandName, nsISupports *aCommandRefCon, PRBool *_retval);
  virtual nsresult GetCurrentState(nsIEditor *aEditor, nsICommandParams* aParams);
  virtual nsresult SetState(nsIEditor *aEditor, nsString& newState);

};

class nsFontColorStateCommand : public nsMultiStateCommand
{
public:
                   nsFontColorStateCommand();

protected:

  virtual nsresult GetCurrentState(nsIEditor *aEditor, nsICommandParams* aParams);
  virtual nsresult SetState(nsIEditor *aEditor, nsString& newState);
};

class nsAlignCommand : public nsMultiStateCommand
{
public:
                   nsAlignCommand();

protected:

  virtual nsresult GetCurrentState(nsIEditor *aEditor, nsICommandParams* aParams);
  virtual nsresult SetState(nsIEditor *aEditor, nsString& newState);
};

class nsBackgroundColorStateCommand : public nsMultiStateCommand
{
public:
                   nsBackgroundColorStateCommand();

protected:

  virtual nsresult GetCurrentState(nsIEditor *aEditor, nsICommandParams* aParams);
  virtual nsresult SetState(nsIEditor *aEditor, nsString& newState);
};

class nsAbsolutePositioningCommand : public nsBaseStateUpdatingCommand
{
public:
                   nsAbsolutePositioningCommand();

protected:

  NS_IMETHOD IsCommandEnabled(const char *aCommandName, nsISupports *aCommandRefCon, PRBool *_retval);
  virtual nsresult  GetCurrentState(nsIEditor *aEditor, const char* aTagName, nsICommandParams *aParams);
  virtual nsresult  ToggleState(nsIEditor *aEditor, const char* aTagName);
};

// composer commands

NS_DECL_COMPOSER_COMMAND(nsCloseCommand)
NS_DECL_COMPOSER_COMMAND(nsDocumentStateCommand)
NS_DECL_COMPOSER_COMMAND(nsSetDocumentStateCommand)
NS_DECL_COMPOSER_COMMAND(nsSetDocumentOptionsCommand)
//NS_DECL_COMPOSER_COMMAND(nsPrintingCommands)

NS_DECL_COMPOSER_COMMAND(nsDecreaseZIndexCommand)
NS_DECL_COMPOSER_COMMAND(nsIncreaseZIndexCommand)

// Generic commands

// File menu
NS_DECL_COMPOSER_COMMAND(nsNewCommands)   // handles 'new' anything

// Edit menu
NS_DECL_COMPOSER_COMMAND(nsPasteNoFormattingCommand)

// Block transformations
NS_DECL_COMPOSER_COMMAND(nsIndentCommand)
NS_DECL_COMPOSER_COMMAND(nsOutdentCommand)

NS_DECL_COMPOSER_COMMAND(nsRemoveListCommand)
NS_DECL_COMPOSER_COMMAND(nsRemoveStylesCommand)
NS_DECL_COMPOSER_COMMAND(nsIncreaseFontSizeCommand)
NS_DECL_COMPOSER_COMMAND(nsDecreaseFontSizeCommand)

// Insert content commands
NS_DECL_COMPOSER_COMMAND(nsInsertHTMLCommand)

#endif // nsComposerCommands_h_
