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
 *   Dean Tessman <dean_tessman@hotmail.com>
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

#include "nsMenuDismissalListener.h"
#include "nsIMenuParent.h"
#include "nsMenuFrame.h"
#include "nsIPopupBoxObject.h"


nsMenuDismissalListener* nsMenuDismissalListener::sInstance = nsnull;

/*
 * nsMenuDismissalListener implementation
 */

NS_IMPL_ADDREF(nsMenuDismissalListener)
NS_IMPL_RELEASE(nsMenuDismissalListener)
NS_INTERFACE_MAP_BEGIN(nsMenuDismissalListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMMouseListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMEventListener)
  NS_INTERFACE_MAP_ENTRY(nsIMenuRollup)
  NS_INTERFACE_MAP_ENTRY(nsIRollupListener)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMMouseListener)
NS_INTERFACE_MAP_END


////////////////////////////////////////////////////////////////////////

nsMenuDismissalListener::nsMenuDismissalListener() :
  mEnabled(PR_TRUE)
{
  mMenuParent = nsnull;
}

nsMenuDismissalListener::~nsMenuDismissalListener() 
{
}

nsMenuDismissalListener*
nsMenuDismissalListener::GetInstance()
{
  if (!sInstance) {
    sInstance = new nsMenuDismissalListener();
    NS_IF_ADDREF(sInstance);
  }
  return sInstance;
}

/* static */ void
nsMenuDismissalListener::Shutdown()
{
  if (sInstance) {
    sInstance->Unregister();
    NS_RELEASE(sInstance);
  }
}


nsIMenuParent*
nsMenuDismissalListener::GetCurrentMenuParent()
{
  return mMenuParent;
}

void
nsMenuDismissalListener::SetCurrentMenuParent(nsIMenuParent* aMenuParent)
{
  if (aMenuParent == mMenuParent)
    return;

  mMenuParent = aMenuParent;

  if (!aMenuParent) {
    Shutdown();
    return;
  }

  Unregister();
  Register();
}

NS_IMETHODIMP
nsMenuDismissalListener::Rollup()
{
  if (mEnabled) {
    if (mMenuParent) {
      AddRef();
      mMenuParent->HideChain();
      mMenuParent->DismissChain();
      Release();
    }
    else
      Shutdown();
  }
  return NS_OK;
}

////////////////////////////////////////////////////////////////////////
NS_IMETHODIMP nsMenuDismissalListener::ShouldRollupOnMouseWheelEvent(PRBool *aShouldRollup) 
{ 
  *aShouldRollup = PR_FALSE; 
  return NS_OK;
}


// uggggh.

// a menu should not roll up if activated by a mouse activate message (eg. X-mouse)
NS_IMETHODIMP nsMenuDismissalListener::ShouldRollupOnMouseActivate(PRBool *aShouldRollup) 
{ 
  *aShouldRollup = PR_FALSE; 
  return NS_OK;
}

NS_IMETHODIMP
nsMenuDismissalListener::GetSubmenuWidgetChain(nsISupportsArray **_retval)
{
  NS_NewISupportsArray ( _retval );
  nsIMenuParent *curr = mMenuParent;
  while ( curr ) {
    nsCOMPtr<nsIWidget> widget;
    curr->GetWidget ( getter_AddRefs(widget) );
    nsCOMPtr<nsISupports> genericWidget ( do_QueryInterface(widget) );
    (**_retval).AppendElement ( genericWidget );
    
    // move up the chain
    nsIFrame* currAsFrame = nsnull;
    if ( NS_SUCCEEDED(CallQueryInterface(curr, &currAsFrame)) ) {
      nsIMenuFrame *menuFrame = nsnull;
      nsIFrame *parentFrame = currAsFrame->GetParent();
      if (parentFrame) {
        CallQueryInterface(parentFrame, &menuFrame);
      }
      if ( menuFrame ) {
        curr = menuFrame->GetMenuParent ();       // Advance to next parent
      }
      else {
        // we are a menuParent but not a menuFrame. This is probably the case
        // of the menu bar. Nothing to do here, really.
        return NS_OK;
      }
    }
    else {
      // We've run into a menu parent that isn't a frame at all. Not good.
      NS_WARNING ( "nsIMenuParent that is not a nsIFrame" );
      return NS_ERROR_FAILURE;
    }
  } // foreach parent menu
  
  return NS_OK; 
}


void
nsMenuDismissalListener::Register()
{
  if (mWidget)
    return;

  nsCOMPtr<nsIWidget> widget;
  mMenuParent->GetWidget(getter_AddRefs(widget));
  if (!widget) {
    Shutdown();
    return;
  }

  PRBool consumeOutsideClicks = PR_FALSE;
  mMenuParent->ConsumeOutsideClicks(consumeOutsideClicks);
  widget->CaptureRollupEvents(this, PR_TRUE, consumeOutsideClicks);
  mWidget = widget;

  mMenuParent->AttachedDismissalListener();
}

void
nsMenuDismissalListener::Unregister()
{
  if (mWidget) {
    mWidget->CaptureRollupEvents(this, PR_FALSE, PR_FALSE);
    mWidget = nsnull;
  }
}

void
nsMenuDismissalListener::EnableListener(PRBool aEnabled)
{
  mEnabled = aEnabled;
}

