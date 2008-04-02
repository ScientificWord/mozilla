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
 * The Original Code is support for icons in native menu items on Mac OS X.
 *
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Mark Mentovai <mark@moxienet.com> (Original Author)
 *  Josh Aas <josh@mozilla.com>
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
 * Retrieves and displays icons in native menu items on Mac OS X.
 */


#ifndef nsMenuItemIconX_h_
#define nsMenuItemIconX_h_


#include "nsCOMPtr.h"
#include "imgIDecoderObserver.h"

class nsIURI;
class nsIContent;
class imgIRequest;
class nsIMenu;

#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>


class nsMenuItemIconX : public imgIDecoderObserver
{
public:
  nsMenuItemIconX(nsISupports* aMenuItem,
                 nsIMenu*     aMenu,
                 nsIContent*  aContent,
                 NSMenuItem* aNativeMenuItem);
private:
  ~nsMenuItemIconX();

public:
  NS_DECL_ISUPPORTS
  NS_DECL_IMGICONTAINEROBSERVER
  NS_DECL_IMGIDECODEROBSERVER

  // SetupIcon succeeds if it was able to set up the icon, or if there should
  // be no icon, in which case it clears any existing icon but still succeeds.
  nsresult SetupIcon();

  // GetIconURI fails if the item should not have any icon.
  nsresult GetIconURI(nsIURI** aIconURI);

  // LoadIcon will set a placeholder image and start a load request for the
  // icon.  The request may not complete until after LoadIcon returns.
  nsresult LoadIcon(nsIURI* aIconURI);

  // ShouldLoadSync returns PR_TRUE if the LoadIcon should load the icon
  // synchronously.
  PRBool ShouldLoadSync(nsIURI* aURI);

protected:
  nsCOMPtr<nsIContent>  mContent;
  nsCOMPtr<imgIRequest> mIconRequest;
  nsISupports*          mMenuItem;
  nsIMenu*              mMenu;
  MenuRef               mMenuRef;
  PRUint16              mMenuItemIndex;
  PRPackedBool          mLoadedIcon;
  PRPackedBool          mSetIcon;
  NSMenuItem*           mNativeMenuItem;
};

#endif // nsMenuItemIconX_h_
