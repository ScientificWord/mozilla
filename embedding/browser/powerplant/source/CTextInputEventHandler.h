/* -*- Mode: IDL; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
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
 * The Original Code is the Mozilla browser.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications, Inc.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Frank Yung-Fong Tang <ftang@netscape.com>
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
 
#ifndef CTextInputEventHandler_h__
#define CTextInputEventHandler_h__

#include <CarbonEvents.h>
#include <TextServices.h>

#include "nsIMacTextInputEventSink.h"

#include "CBrowserShell.h"

class CTextInputEventHandler
{
public:
  CTextInputEventHandler() {};
  virtual ~CTextInputEventHandler() {};
  
  virtual OSStatus HandleAll( EventHandlerCallRef inHandlerCallRef, 
                              EventRef inEvent);
protected:  
  virtual OSStatus HandleUnicodeForKeyEvent( CBrowserShell* aBrowserShell, 
                                             EventHandlerCallRef inHandlerCallRef, 
                                             EventRef inEvent);
  virtual OSStatus HandleUpdateActiveInputArea( CBrowserShell* sink, 
                                                EventHandlerCallRef inHandlerCallRef, 
                                                EventRef inEvent);
  virtual OSStatus HandleOffsetToPos( CBrowserShell* aBrowserShell, 
                                      EventHandlerCallRef inHandlerCallRef, 
                                      EventRef inEvent);
  virtual OSStatus HandlePosToOffset( CBrowserShell* aBrowserShell, 
                                      EventHandlerCallRef inHandlerCallRef, 
                                      EventRef inEvent);
  virtual OSStatus HandleGetSelectedText( CBrowserShell* aBrowserShell, 
                                      EventHandlerCallRef inHandlerCallRef, 
                                      EventRef inEvent);
  
  virtual CBrowserShell* GetGeckoTarget();
 
private:
  OSStatus GetText(EventRef inEvent, nsString& outString);
  OSStatus GetScriptLang(EventRef inEvent, ScriptLanguageRecord& outSlr);
};

void InitializeTextInputEventHandling();

#endif /* CTextInputEventHandler_h__ */

