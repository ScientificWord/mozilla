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

// Special stuff for the Macintosh implementation of command-line service.

#ifndef nsCommandLineServiceMac_h_
#define nsCommandLineServiceMac_h_

#include <Files.h>

#include "nscore.h"
#include "nsError.h"
#include "nsString.h"

#include "nsAEDefs.h"

#ifdef __cplusplus

class nsMacCommandLine
{
public:


  enum
  {
    kArgsGrowSize      = 20  
  };

                  nsMacCommandLine();
                  ~nsMacCommandLine();

  nsresult        Initialize(int& argc, char**& argv);
  
  nsresult        AddToCommandLine(const char* inArgText);
  nsresult        AddToCommandLine(const char* inOptionString, const FSSpec& inFileSpec);
  nsresult        AddToEnvironmentVars(const char* inArgText);

  OSErr           HandleOpenOneDoc(const FSSpec& inFileSpec, OSType inFileType);
  OSErr           HandlePrintOneDoc(const FSSpec& inFileSpec, OSType fileType);

	OSErr						DispatchURLToNewBrowser(const char* url);
	  
  OSErr						Quit(TAskSave askSave);
  
protected:

  OSErr           OpenURL(const char* aURL);

  nsresult        OpenWindow(const char *chrome, const PRUnichar *url);
    
  char**          mArgs;              // array of arg pointers (augmented argv)
  PRUint32        mArgsAllocated;     // number of slots available in mArgs
  PRUint32        mArgsUsed;          // number of slots used in mArgs

  PRBool          mStartedUp;

public:

  static nsMacCommandLine& GetMacCommandLine() { return sMacCommandLine; }
  
private:

  static nsMacCommandLine sMacCommandLine;
  
};

#endif    //__cplusplus


#ifdef __cplusplus
extern "C" {
#endif

nsresult InitializeMacCommandLine(int& argc, char**& argv);

#ifdef __cplusplus
}
#endif


#endif // nsCommandLineServiceMac_h_
