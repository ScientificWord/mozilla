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
 * The Original Code is XMLterm.
 *
 * The Initial Developer of the Original Code is
 * Ramalingam Saravanan.
 * Portions created by the Initial Developer are Copyright (C) 1999
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

// mozXMLTermShell.h: declaration of mozXMLTermShell
// which implements mozIXMLTermShell, providing an XPCONNECT wrapper
// to the XMLTerminal interface, thus allowing easy (and controlled)
// access from scripts

#include <stdio.h>

#include "nscore.h"
#include "nspr.h"
#include "nsCOMPtr.h"
#include "nsString.h"
#include "nsIGenericFactory.h"

#include "mozXMLT.h"
#include "mozIXMLTerminal.h"
#include "mozIXMLTermShell.h"


class mozXMLTermShell : public mozIXMLTermShell
{
 public: 

  mozXMLTermShell();
  virtual ~mozXMLTermShell();

  NS_DECL_ISUPPORTS
  NS_DECL_MOZIXMLTERMSHELL

  // Define a Create method to be used with a factory:
  static NS_METHOD
    Create(nsISupports *aOuter, REFNSIID aIID, void **aResult);

  static NS_METHOD
    RegisterProc(nsIComponentManager *aCompMgr,
                 nsIFile *aPath,
                 const char *registryLocation,
                 const char *componentType,
                 const nsModuleComponentInfo *info);

  static NS_METHOD
    UnregisterProc(nsIComponentManager *aCompMgr,
                   nsIFile *aPath,
                   const char *registryLocation,
                   const nsModuleComponentInfo *info);

  NS_IMETHOD Finalize(void);

protected:

  /** object initialization flag */
  PRBool mInitialized;

  /** non-owning reference to content window for XMLterm */		
  nsIDOMWindowInternal* mContentWindow;

  /** non-owning reference (??) to doc shell for content window */
  nsIDocShell* mContentAreaDocShell;

  /** owning reference to XMLTerminal object created by us */
  nsCOMPtr<mozIXMLTerminal> mXMLTerminal;

  static PRBool mLoggingInitialized;
};
