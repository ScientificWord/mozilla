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

#include "nsCOMPtr.h"
#include "nsIDocument.h"
#include "nsIContent.h"
#include "nsIDOMDocument.h"
#include "nsIDocumentViewer.h"
#include "nsIDocumentObserver.h"
#include "nsIComponentManager.h"
#include "nsIDocShell.h"
#include "prinrval.h"

#include "nsMenuX.h"
#include "nsMenuBarX.h"
#include "nsIMenu.h"
#include "nsIMenuBar.h"
#include "nsIMenuItem.h"
#include "nsIMenuListener.h"
#include "nsPresContext.h"
#include "nsIMenuCommandDispatcher.h"

#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "plstr.h"

#include "nsINameSpaceManager.h"
#include "nsWidgetAtoms.h"
#include "nsIXBLService.h"
#include "nsIServiceManager.h"

#include "nsGUIEvent.h"

#include "nsCRT.h"


static OSStatus InstallMyMenuEventHandler(MenuRef menuRef, void* userData, EventHandlerRef* outHandler) ;

static PRInt16 gMacMenuIDCountX = nsMenuBarX::kAppleMenuID + 1;
static PRBool gConstructingMenu = PR_FALSE;
  
#if DEBUG
nsInstanceCounter   gMenuCounterX("nsMenuX");
#endif

// CIDs
#include "nsWidgetsCID.h"
static NS_DEFINE_CID(kMenuCID,     NS_MENU_CID);
static NS_DEFINE_CID(kMenuItemCID, NS_MENUITEM_CID);

// Refcounted class for dummy menu items, like separators and help menu items.
class nsDummyMenuItemX : public nsISupports {
public:
    NS_DECL_ISUPPORTS

    nsDummyMenuItemX()
    {
    }
};

NS_IMETHODIMP_(nsrefcnt) nsDummyMenuItemX::AddRef() { return ++mRefCnt; }
NS_METHOD nsDummyMenuItemX::Release() { return --mRefCnt; }
NS_IMPL_QUERY_INTERFACE0(nsDummyMenuItemX)
static nsDummyMenuItemX gDummyMenuItemX;

//-------------------------------------------------------------------------
NS_IMPL_ISUPPORTS4(nsMenuX, nsIMenu, nsIMenuListener, nsIChangeObserver, nsISupportsWeakReference)

//
// nsMenuX constructor
//
nsMenuX::nsMenuX()
    :   mNumMenuItems(0), mParent(nsnull), mManager(nsnull), mMacMenuID(0),
        mMacMenuHandle(nsnull), mIsEnabled(PR_TRUE), mDestroyHandlerCalled(PR_FALSE),
        mNeedsRebuild(PR_TRUE), mConstructed(PR_FALSE), mVisible(PR_TRUE), mHandler(nsnull)
{
#if DEBUG
  ++gMenuCounterX;
#endif 
}


//
// nsMenuX destructor
//
nsMenuX::~nsMenuX()
{
  RemoveAll();

  if ( mMacMenuHandle ) {
    if ( mHandler )
      ::RemoveEventHandler(mHandler);
    ::ReleaseMenu(mMacMenuHandle);
  }
  
  // alert the change notifier we don't care no more
  mManager->Unregister(mMenuContent);

#if DEBUG
  --gMenuCounterX;
#endif
}


//
// Create
//
NS_METHOD 
nsMenuX::Create(nsISupports * aParent, const nsAString &aLabel, const nsAString &aAccessKey, 
                nsIChangeManager* aManager, nsIDocShell* aShell, nsIContent* aNode )
{
  mDocShellWeakRef = do_GetWeakReference(aShell);
  mMenuContent = aNode;

  // register this menu to be notified when changes are made to our content object
  mManager = aManager;			// weak ref
  nsCOMPtr<nsIChangeObserver> changeObs ( do_QueryInterface(NS_STATIC_CAST(nsIChangeObserver*, this)) );
  mManager->Register(mMenuContent, changeObs);

  NS_ASSERTION ( mMenuContent, "Menu not given a dom node at creation time" );
  NS_ASSERTION ( mManager, "No change manager given, can't tell content model updates" );

  mParent = aParent;
  // our parent could be either a menu bar (if we're toplevel) or a menu (if we're a submenu)
  nsCOMPtr<nsIMenuBar> menubar = do_QueryInterface(aParent);
  nsCOMPtr<nsIMenu> menu = do_QueryInterface(aParent);
  NS_ASSERTION(menubar || menu, "Menu parent not a menu bar or menu!" );

  SetLabel(aLabel);
  SetAccessKey(aAccessKey);

  if (mMenuContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::hidden,
                                nsWidgetAtoms::_true, eCaseMatters) ||
      mMenuContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::collapsed,
                                nsWidgetAtoms::_true, eCaseMatters))
    mVisible = PR_FALSE;

  if (menubar && mMenuContent->GetChildCount() == 0)
    mVisible = PR_FALSE;

  // We call MenuConstruct here because keyboard commands are dependent upon
  // native menu items being created. If we only call MenuConstruct when a menu
  // is actually selected, then we can't access keyboard commands until the
  // menu gets selected, which is bad.
  nsMenuEvent fake(PR_TRUE, 0, nsnull);
  MenuConstruct(fake, nsnull, nsnull, nsnull);
  
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::GetParent(nsISupports*& aParent)
{
  aParent = mParent;
  NS_IF_ADDREF(aParent);
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::GetLabel(nsString &aText)
{
  aText = mLabel;
  return NS_OK;
}


//-------------------------------------------------------------------------
NS_METHOD nsMenuX::SetLabel(const nsAString &aText)
{
  mLabel = aText;

  // first time? create the menu handle, attach event handler to it.
  if (mMacMenuHandle == nsnull) {
    mMacMenuID = gMacMenuIDCountX++;
    mMacMenuHandle = NSStringNewMenu(mMacMenuID, mLabel);
  }
  
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::GetAccessKey(nsString &aText)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::SetAccessKey(const nsAString &aText)
{
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::AddItem(nsISupports* aItem)
{
    nsresult rv = NS_ERROR_FAILURE;
    if (aItem) {
        // Figure out what we're adding
        nsCOMPtr<nsIMenuItem> menuItem(do_QueryInterface(aItem));
        if (menuItem) {
            rv = AddMenuItem(menuItem);
        } else {
            nsCOMPtr<nsIMenu> menu(do_QueryInterface(aItem));
            if (menu)
                rv = AddMenu(menu);
        }
    }
    return rv;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::AddMenuItem(nsIMenuItem * aMenuItem)
{
  if(!aMenuItem) return NS_ERROR_NULL_POINTER;

  mMenuItemsArray.AppendElement(aMenuItem);    // owning ref
  PRUint32 currItemIndex;
  mMenuItemsArray.Count(&currItemIndex);

  mNumMenuItems++;

  nsAutoString label;
  aMenuItem->GetLabel(label);
  InsertMenuItemWithTruncation ( label, currItemIndex );
  
  // I want to be internationalized too!
  nsAutoString keyEquivalent(NS_LITERAL_STRING(" "));
  aMenuItem->GetShortcutChar(keyEquivalent);
  if (!keyEquivalent.IsEmpty() && !keyEquivalent.EqualsLiteral(" ")) {
    ToUpperCase(keyEquivalent);
    short inKey = NS_LossyConvertUTF16toASCII(keyEquivalent).First();
    ::SetItemCmd(mMacMenuHandle, currItemIndex, inKey);
    //::SetMenuItemKeyGlyph(mMacMenuHandle, mNumMenuItems, 0x61);
  }

  PRUint8 modifiers;
  aMenuItem->GetModifiers(&modifiers);
  PRUint8 macModifiers = kMenuNoModifiers;
  if (knsMenuItemShiftModifier & modifiers)
    macModifiers |= kMenuShiftModifier;

  if (knsMenuItemAltModifier & modifiers)
    macModifiers |= kMenuOptionModifier;

  if (knsMenuItemControlModifier & modifiers)
    macModifiers |= kMenuControlModifier;

  if (!(knsMenuItemCommandModifier & modifiers))
    macModifiers |= kMenuNoCommandModifier;

  ::SetMenuItemModifiers(mMacMenuHandle, currItemIndex, macModifiers);

  // set its command. we get the unique command id from the menubar
  nsCOMPtr<nsIMenuCommandDispatcher> dispatcher ( do_QueryInterface(mManager) );
  if ( dispatcher ) {
    PRUint32 commandID = 0L;
    dispatcher->Register(aMenuItem, &commandID);
    if ( commandID )
      ::SetMenuItemCommandID(mMacMenuHandle, currItemIndex, commandID);
  }
  
  PRBool isEnabled;
  aMenuItem->GetEnabled(&isEnabled);
  if(isEnabled)
    ::EnableMenuItem(mMacMenuHandle, currItemIndex);
  else
    ::DisableMenuItem(mMacMenuHandle, currItemIndex);

  PRBool isChecked;
  aMenuItem->GetChecked(&isChecked);
  if(isChecked)
    ::CheckMenuItem(mMacMenuHandle, currItemIndex, true);
  else
    ::CheckMenuItem(mMacMenuHandle, currItemIndex, false);

  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::AddMenu(nsIMenu * aMenu)
{
  // Add a submenu
  if (!aMenu) return NS_ERROR_NULL_POINTER;

  nsCOMPtr<nsISupports>  supports = do_QueryInterface(aMenu);
  if (!supports) return NS_ERROR_NO_INTERFACE;

  mMenuItemsArray.AppendElement(supports);   // owning ref
  PRUint32 currItemIndex;
  mMenuItemsArray.Count(&currItemIndex);

  mNumMenuItems++;

  // We have to add it as a menu item and then associate it with the item
  nsAutoString label;
  aMenu->GetLabel(label);
  InsertMenuItemWithTruncation ( label, currItemIndex );

  PRBool isEnabled;
  aMenu->GetEnabled(&isEnabled);
  if (isEnabled)
    ::EnableMenuItem(mMacMenuHandle, currItemIndex);
  else
    ::DisableMenuItem(mMacMenuHandle, currItemIndex);	    

  MenuHandle childMenu;
  if (aMenu->GetNativeData((void**)&childMenu) == NS_OK)
    ::SetMenuItemHierarchicalMenu((MenuHandle) mMacMenuHandle, currItemIndex, childMenu);
  
  return NS_OK;
}


//
// InsertMenuItemWithTruncation
//
// Insert a new item in this menu with index |inItemIndex| with the text |inItemLabel|,
// middle-truncated to a certain pixel width with an elipsis.
//
void
nsMenuX :: InsertMenuItemWithTruncation ( nsAutoString & inItemLabel, PRUint32 inItemIndex )
{
  // ::TruncateThemeText() doesn't take the number of characters to truncate to, it takes a pixel with
  // to fit the string in. Ugh. I talked it over with sfraser and we couldn't come up with an 
  // easy way to compute what this should be given the system font, etc, so we're just going
  // to hard code it to something reasonable and bigger fonts will just have to deal.
  const short kMaxItemPixelWidth = 300;

  CFMutableStringRef labelRef = ::CFStringCreateMutable ( kCFAllocatorDefault, inItemLabel.Length() );
  ::CFStringAppendCharacters ( labelRef, (UniChar*)inItemLabel.get(), inItemLabel.Length() );
  ::TruncateThemeText(labelRef, kThemeMenuItemFont, kThemeStateActive, kMaxItemPixelWidth, truncMiddle, NULL);
  ::InsertMenuItemTextWithCFString(mMacMenuHandle, labelRef, inItemIndex, 0, 0);
  ::CFRelease(labelRef);

} // InsertMenuItemWithTruncation


//-------------------------------------------------------------------------
NS_METHOD nsMenuX::AddSeparator()
{
  // HACK - We're not really appending an nsMenuItem but it 
  // needs to be here to make sure that event dispatching isn't off by one.
  mMenuItemsArray.AppendElement(&gDummyMenuItemX);   // owning ref
  PRUint32  numItems;
  mMenuItemsArray.Count(&numItems);
  ::InsertMenuItem(mMacMenuHandle, "\p(-", numItems);
  mNumMenuItems++;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::GetItemCount(PRUint32 &aCount)
{
  return mMenuItemsArray.Count(&aCount);
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::GetItemAt(const PRUint32 aPos, nsISupports *& aMenuItem)
{
  mMenuItemsArray.GetElementAt(aPos, &aMenuItem);
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::InsertItemAt(const PRUint32 aPos, nsISupports * aMenuItem)
{
  NS_ASSERTION(0, "Not implemented");
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::RemoveItem(const PRUint32 aPos)
{
  NS_WARNING("Not implemented");
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::RemoveAll()
{
  if (mMacMenuHandle != NULL) {    
    // clear command id's
    nsCOMPtr<nsIMenuCommandDispatcher> dispatcher ( do_QueryInterface(mManager) );
    if ( dispatcher ) {
      for ( unsigned int i = 1; i <= mNumMenuItems; ++i ) {
        PRUint32 commandID = 0L;
        OSErr err = ::GetMenuItemCommandID(mMacMenuHandle, i, (unsigned long*)&commandID);
        if ( !err )
          dispatcher->Unregister(commandID);
      }
    }
    ::DeleteMenuItems(mMacMenuHandle, 1, ::CountMenuItems(mMacMenuHandle));
  }
  
  mMenuItemsArray.Clear();    // remove all items
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::GetNativeData(void ** aData)
{
  *aData = mMacMenuHandle;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::SetNativeData(void * aData)
{
  mMacMenuHandle = (MenuHandle) aData;
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::AddMenuListener(nsIMenuListener * aMenuListener)
{
  mListener = aMenuListener;    // strong ref
  return NS_OK;
}

//-------------------------------------------------------------------------
NS_METHOD nsMenuX::RemoveMenuListener(nsIMenuListener * aMenuListener)
{
  if (aMenuListener == mListener)
    mListener = nsnull;
  return NS_OK;
}


//-------------------------------------------------------------------------
//
// nsIMenuListener interface
//
//-------------------------------------------------------------------------
nsEventStatus nsMenuX::MenuItemSelected(const nsMenuEvent & aMenuEvent)
{
  // all this is now handled by Carbon Events.
  return nsEventStatus_eConsumeNoDefault;
}

//-------------------------------------------------------------------------
nsEventStatus nsMenuX::MenuSelected(const nsMenuEvent & aMenuEvent)
{
  //printf("MenuSelected called for %s \n", NS_LossyConvertUTF16toASCII(mLabel).get());
  nsEventStatus eventStatus = nsEventStatus_eIgnore;

  // Determine if this is the correct menu to handle the event
  MenuHandle selectedMenuHandle = (MenuHandle) aMenuEvent.mCommand;

  if (mMacMenuHandle == selectedMenuHandle) {
    // Open the node.
    mMenuContent->SetAttr(kNameSpaceID_None, nsWidgetAtoms::open, NS_LITERAL_STRING("true"), PR_TRUE);
  

    // Fire our oncreate handler. If we're told to stop, don't build the menu at all
    PRBool keepProcessing = OnCreate();

    if (!mNeedsRebuild || !keepProcessing)
      return nsEventStatus_eConsumeNoDefault;

    if (!mConstructed || mNeedsRebuild) {
      if (mNeedsRebuild)
        RemoveAll();

      nsCOMPtr<nsIDocShell> docShell = do_QueryReferent(mDocShellWeakRef);
      if (!docShell) {
        NS_ERROR("No doc shell");
        return nsEventStatus_eConsumeNoDefault;
      }

      MenuConstruct(aMenuEvent, nsnull /* mParentWindow */, nsnull, docShell);
      mConstructed = true;
    } 

    OnCreated();  // Now that it's built, fire the popupShown event.

    eventStatus = nsEventStatus_eConsumeNoDefault;  
  } 
  else {
    // Make sure none of our submenus are the ones that should be handling this
    PRUint32    numItems;
    mMenuItemsArray.Count(&numItems);
    for (PRUint32 i = numItems; i > 0; i--) {
      nsCOMPtr<nsISupports>     menuSupports = getter_AddRefs(mMenuItemsArray.ElementAt(i - 1));    
      nsCOMPtr<nsIMenu>         submenu = do_QueryInterface(menuSupports);
      nsCOMPtr<nsIMenuListener> menuListener = do_QueryInterface(submenu);
      if (menuListener) {
        eventStatus = menuListener->MenuSelected(aMenuEvent);
        if (nsEventStatus_eIgnore != eventStatus)
          return eventStatus;
      }  
    }
  }

  return eventStatus;
}

//-------------------------------------------------------------------------
nsEventStatus nsMenuX::MenuDeselected(const nsMenuEvent & aMenuEvent)
{
  // Destroy the menu
  if (mConstructed) {
    MenuDestruct(aMenuEvent);
    mConstructed = false;
  }
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
nsEventStatus nsMenuX::MenuConstruct(
    const nsMenuEvent & aMenuEvent,
    nsIWidget         * aParentWindow, 
    void              * /* menuNode */,
	  void              * aDocShell)
{
  mConstructed = false;
  gConstructingMenu = PR_TRUE;
  
  // reset destroy handler flag so that we'll know to fire it next time this menu goes away.
  mDestroyHandlerCalled = PR_FALSE;
  
  //printf("nsMenuX::MenuConstruct called for %s = %d \n", NS_LossyConvertUTF16toASCII(mLabel).get(), mMacMenuHandle);
  // Begin menuitem inner loop
  
  // Retrieve our menupopup.
  nsCOMPtr<nsIContent> menuPopup;
  GetMenuPopupContent(getter_AddRefs(menuPopup));
  if (!menuPopup)
    return nsEventStatus_eIgnore;
      
  // Iterate over the kids
  PRUint32 count = menuPopup->GetChildCount();
  for ( PRUint32 i = 0; i < count; ++i ) {
    nsIContent *child = menuPopup->GetChildAt(i);
    if ( child ) {
      // depending on the type, create a menu item, separator, or submenu
      nsIAtom *tag = child->Tag();
      if ( tag == nsWidgetAtoms::menuitem )
        LoadMenuItem(this, child);
      else if ( tag == nsWidgetAtoms::menuseparator )
        LoadSeparator(child);
      else if ( tag == nsWidgetAtoms::menu )
        LoadSubMenu(this, child);
    }
  } // for each menu item
  
  gConstructingMenu = PR_FALSE;
  mNeedsRebuild = PR_FALSE;
  //printf("  Done building, mMenuItemVoidArray.Count() = %d \n", mMenuItemVoidArray.Count());
  
  return nsEventStatus_eIgnore;
}


//-------------------------------------------------------------------------
nsEventStatus nsMenuX::MenuDestruct(const nsMenuEvent & aMenuEvent)
{
  //printf("nsMenuX::MenuDestruct() called for %s \n", NS_LossyConvertUTF16toASCII(mLabel).get());
  
  // Fire our ondestroy handler. If we're told to stop, don't destroy the menu
  PRBool keepProcessing = OnDestroy();
  if ( keepProcessing ) {
    if(mNeedsRebuild) {
        mConstructed = false;
        //printf("  mMenuItemVoidArray.Count() = %d \n", mMenuItemVoidArray.Count());
    } 
    // Close the node.
    mMenuContent->UnsetAttr(kNameSpaceID_None, nsWidgetAtoms::open, PR_TRUE);

    OnDestroyed();
  }
  
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
nsEventStatus nsMenuX::CheckRebuild(PRBool & aNeedsRebuild)
{
  aNeedsRebuild = PR_TRUE; //mNeedsRebuild;
  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
nsEventStatus nsMenuX::SetRebuild(PRBool aNeedsRebuild)
{
  if (!gConstructingMenu)
    mNeedsRebuild = aNeedsRebuild;

  return nsEventStatus_eIgnore;
}

//-------------------------------------------------------------------------
/**
* Set enabled state
*
*/
NS_METHOD nsMenuX::SetEnabled(PRBool aIsEnabled)
{
  mIsEnabled = aIsEnabled;

  if ( aIsEnabled )
    ::EnableMenuItem(mMacMenuHandle, 0);
  else
    ::DisableMenuItem(mMacMenuHandle, 0);

  return NS_OK;
}


/**
* Get enabled state
*
*/
NS_METHOD nsMenuX::GetEnabled(PRBool* aIsEnabled)
{
  NS_ENSURE_ARG_POINTER(aIsEnabled);
  *aIsEnabled = mIsEnabled;
  return NS_OK;
}


/**
* Get GetMenuContent
*
*/
NS_METHOD nsMenuX::GetMenuContent(nsIContent ** aMenuContent)
{
  NS_ENSURE_ARG_POINTER(aMenuContent);
  NS_IF_ADDREF(*aMenuContent = mMenuContent);
	return NS_OK;
}


/*
    Support for Carbon Menu Manager.
 */

static pascal OSStatus MyMenuEventHandler(EventHandlerCallRef myHandler, EventRef event, void* userData)
{
  OSStatus result = eventNotHandledErr;

  UInt32 kind = ::GetEventKind(event);
  if (kind == kEventMenuTargetItem) {
    // get the position of the menu item we want
    nsIMenu* targetMenu = reinterpret_cast<nsIMenu*>(userData);
    PRUint16 aPos;
    ::GetEventParameter(event, kEventParamMenuItemIndex, typeMenuItemIndex, NULL, sizeof(MenuItemIndex), NULL, &aPos);
    aPos--; // subtract 1 from aPos because Carbon menu positions start at 1 not 0
    
    nsISupports* aTargetMenuItem;
    targetMenu->GetItemAt((PRUint32)aPos, aTargetMenuItem);
    
    // Send DOM event
    // If the QI fails, we're over a submenu and we shouldn't send the event
    nsCOMPtr<nsIMenuItem> bTargetMenuItem(do_QueryInterface(aTargetMenuItem));
    if (bTargetMenuItem) {
      PRBool handlerCalledPreventDefault; // but we don't actually care
      bTargetMenuItem->DispatchDOMEvent(NS_LITERAL_STRING("DOMMenuItemActive"), &handlerCalledPreventDefault);
    }
  }
  else if (kind == kEventMenuOpening || kind == kEventMenuClosed) {
    nsISupports* supports = reinterpret_cast<nsISupports*>(userData);
    nsCOMPtr<nsIMenuListener> listener(do_QueryInterface(supports));
    if (listener) {
      MenuRef menuRef;
      ::GetEventParameter(event, kEventParamDirectObject, typeMenuRef, NULL, sizeof(menuRef), NULL, &menuRef);
      nsMenuEvent menuEvent(PR_TRUE, NS_MENU_SELECTED, nsnull);
      menuEvent.time = PR_IntervalNow();
      menuEvent.mCommand = (PRUint32) menuRef;
      if (kind == kEventMenuOpening)
        listener->MenuSelected(menuEvent);
      else
        listener->MenuDeselected(menuEvent);
    }
  }
  
  return result;
}

static OSStatus InstallMyMenuEventHandler(MenuRef menuRef, void* userData, EventHandlerRef* outHandler)
{
  // install the event handler for the various carbon menu events.
  static EventTypeSpec eventList[] = {
      { kEventClassMenu, kEventMenuOpening },
      { kEventClassMenu, kEventMenuClosed },
      { kEventClassMenu, kEventMenuTargetItem }
  };
  static EventHandlerUPP gMyMenuEventHandlerUPP = NewEventHandlerUPP(&MyMenuEventHandler);
  return ::InstallMenuEventHandler(menuRef, gMyMenuEventHandlerUPP,
                                   sizeof(eventList) / sizeof(EventTypeSpec), eventList,
                                   userData, outHandler);
}

//-------------------------------------------------------------------------
MenuHandle nsMenuX::NSStringNewMenu(short menuID, nsString& menuTitle)
{
  MenuRef menuRef;
  OSStatus status = ::CreateNewMenu(menuID, 0, &menuRef);
  NS_ASSERTION(status == noErr,"nsMenuX::NSStringNewMenu: NewMenu failed.");
  CFStringRef titleRef = ::CFStringCreateWithCharacters(kCFAllocatorDefault, (UniChar*)menuTitle.get(), menuTitle.Length());
  NS_ASSERTION(titleRef,"nsMenuX::NSStringNewMenu: CFStringCreateWithCharacters failed.");
  if (titleRef) {
    ::SetMenuTitleWithCFString(menuRef, titleRef);
    ::CFRelease(titleRef);
  }
  
  status = InstallMyMenuEventHandler(menuRef, this, &mHandler);
  NS_ASSERTION(status == noErr,"nsMenuX::NSStringNewMenu: InstallMyMenuEventHandler failed.");

  return menuRef;
}


//----------------------------------------
void nsMenuX::LoadMenuItem( nsIMenu* inParentMenu, nsIContent* inMenuItemContent )
{
  if ( !inMenuItemContent )
    return;

  // if menu should be hidden, bail
  if (inMenuItemContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::hidden,
                                     nsWidgetAtoms::_true, eCaseMatters))
    return;
  
  nsCOMPtr<nsIDOMDocument> domDocument = do_QueryInterface(inMenuItemContent->GetDocument());
  if (!domDocument)
    return;
  
  // We want the enabled state from the command element, not the menu item element.
  // The command element is more up-to-date and it matters for keyboard shortcuts
  // that can be invoked without opening the menu for the item. Sync menu item's
  // enabled state with its command element now.
  nsAutoString ourCommand;
  inMenuItemContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::command, ourCommand);
  if (!ourCommand.IsEmpty()) {
    // get the command DOM element
    nsCOMPtr<nsIDOMElement> commandElt;
    domDocument->GetElementById(ourCommand, getter_AddRefs(commandElt));
    if (commandElt) {
      nsCOMPtr<nsIContent> commandContent = do_QueryInterface(commandElt);
      nsAutoString menuItemDisabled;
      nsAutoString commandDisabled;
      inMenuItemContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::disabled, menuItemDisabled);
      commandContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::disabled, commandDisabled);
      if (!commandDisabled.Equals(menuItemDisabled)) {
        // The menu's disabled state needs to be updated to match the command
        if (commandDisabled.IsEmpty()) 
          inMenuItemContent->UnsetAttr(kNameSpaceID_None, nsWidgetAtoms::disabled, PR_TRUE);
        else
          inMenuItemContent->SetAttr(kNameSpaceID_None, nsWidgetAtoms::disabled, commandDisabled, PR_TRUE);
      }
    }
  }

  // Create nsMenuItem
  nsCOMPtr<nsIMenuItem> pnsMenuItem = do_CreateInstance ( kMenuItemCID ) ;
  if (pnsMenuItem) {
    nsAutoString menuitemName;
    
    inMenuItemContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::label, menuitemName);

    // Bug 164155 - Carbon interprets a leading hyphen on a menu item as
    // a separator. We want a real menu item, so "escape" the hyphen by inserting
    // a zero width space (Unicode 8204) before it.
    if ( menuitemName[0] == '-' )
      menuitemName.Assign( NS_LITERAL_STRING("\u200c") + menuitemName );

    //printf("menuitem %s \n", NS_LossyConvertUTF16toASCII(menuitemName).get());
    
    static nsIContent::AttrValuesArray strings[] =
      {&nsWidgetAtoms::checkbox, &nsWidgetAtoms::radio, nsnull};
    nsIMenuItem::EMenuItemType itemType = nsIMenuItem::eRegular;
    switch (inMenuItemContent->FindAttrValueIn(kNameSpaceID_None, nsWidgetAtoms::type,
                                               strings, eCaseMatters)) {
      case 0: itemType = nsIMenuItem::eCheckbox; break;
      case 1: itemType = nsIMenuItem::eRadio; break;
    }
      
    nsCOMPtr<nsIDocShell>  docShell = do_QueryReferent(mDocShellWeakRef);
    if (!docShell)
      return;

    // Create the item.
    pnsMenuItem->Create(inParentMenu, menuitemName, PR_FALSE, itemType, 
                        mManager, docShell, inMenuItemContent);   

    //
    // Set key shortcut and modifiers
    //
    
    nsAutoString keyValue;
    inMenuItemContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::key, keyValue);

    // Try to find the key node.
    nsCOMPtr<nsIDOMElement> keyElement;
    if (!keyValue.IsEmpty())
      domDocument->GetElementById(keyValue, getter_AddRefs(keyElement));
    if ( keyElement ) {
      nsCOMPtr<nsIContent> keyContent ( do_QueryInterface(keyElement) );
      nsAutoString keyChar(NS_LITERAL_STRING(" "));
      keyContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::key, keyChar);
	    if(!keyChar.EqualsLiteral(" ")) 
        pnsMenuItem->SetShortcutChar(keyChar);
        
      PRUint8 modifiers = knsMenuItemNoModifier;
	    nsAutoString modifiersStr;
      keyContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::modifiers, modifiersStr);
		  char* str = ToNewCString(modifiersStr);
		  char* newStr;
		  char* token = nsCRT::strtok( str, ", \t", &newStr );
		  while( token != NULL ) {
		    if (PL_strcmp(token, "shift") == 0)
		      modifiers |= knsMenuItemShiftModifier;
		    else if (PL_strcmp(token, "alt") == 0) 
		      modifiers |= knsMenuItemAltModifier;
		    else if (PL_strcmp(token, "control") == 0) 
		      modifiers |= knsMenuItemControlModifier;
		    else if ((PL_strcmp(token, "accel") == 0) ||
		             (PL_strcmp(token, "meta") == 0)) {
          modifiers |= knsMenuItemCommandModifier;
		    }
		    
		    token = nsCRT::strtok( newStr, ", \t", &newStr );
		  }
		  nsMemory::Free(str);

	    pnsMenuItem->SetModifiers ( modifiers );
    }

    if (inMenuItemContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::checked,
                                       nsWidgetAtoms::_true, eCaseMatters))
      pnsMenuItem->SetChecked(PR_TRUE);
    else
      pnsMenuItem->SetChecked(PR_FALSE);
      
    nsCOMPtr<nsISupports> supports ( do_QueryInterface(pnsMenuItem) );
    inParentMenu->AddItem(supports);         // Parent now owns menu item
  }
}


void 
nsMenuX::LoadSubMenu( nsIMenu * pParentMenu, nsIContent* inMenuItemContent )
{
  // if menu should be hidden, bail
  if (inMenuItemContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::hidden,
                                     nsWidgetAtoms::_true, eCaseMatters))
    return;
  
  nsAutoString menuName; 
  inMenuItemContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::label, menuName);
  //printf("Creating Menu [%s] \n", NS_LossyConvertUTF16toASCII(menuName).get());

  // Create nsMenu
  nsCOMPtr<nsIMenu> pnsMenu ( do_CreateInstance(kMenuCID) );
  if (pnsMenu) {
    // Call Create
    nsCOMPtr<nsIDocShell> docShell = do_QueryReferent(mDocShellWeakRef);
    if (!docShell)
        return;
    nsCOMPtr<nsISupports> supports(do_QueryInterface(pParentMenu));
    pnsMenu->Create(supports, menuName, EmptyString(), mManager, docShell, inMenuItemContent);

    // set if it's enabled or disabled
    if (inMenuItemContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::disabled,
                                       nsWidgetAtoms::_true, eCaseMatters))
      pnsMenu->SetEnabled ( PR_FALSE );
    else
      pnsMenu->SetEnabled ( PR_TRUE );

    // Make nsMenu a child of parent nsMenu. The parent takes ownership
    nsCOMPtr<nsISupports> supports2 ( do_QueryInterface(pnsMenu) );
	  pParentMenu->AddItem(supports2);
  }     
}


void
nsMenuX::LoadSeparator ( nsIContent* inMenuItemContent ) 
{
  // if item should be hidden, bail
  if (inMenuItemContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::hidden,
                                     nsWidgetAtoms::_true, eCaseMatters))
    return;

  AddSeparator();
}



//
// OnCreate
//
// Fire our oncreate handler. Returns TRUE if we should keep processing the event,
// FALSE if the handler wants to stop the creation of the menu
//
PRBool
nsMenuX::OnCreate()
{
  nsEventStatus status = nsEventStatus_eIgnore;
  nsMouseEvent event(PR_TRUE, NS_XUL_POPUP_SHOWING, nsnull,
                     nsMouseEvent::eReal);
  
  nsCOMPtr<nsIContent> popupContent;
  GetMenuPopupContent(getter_AddRefs(popupContent));

  nsCOMPtr<nsIDocShell> docShell = do_QueryReferent(mDocShellWeakRef);
  if (!docShell) {
    NS_ERROR("No doc shell");
    return PR_FALSE;
  }
  nsCOMPtr<nsPresContext> presContext;
  MenuHelpersX::DocShellToPresContext(docShell, getter_AddRefs(presContext) );
  if ( presContext ) {
    nsresult rv = NS_OK;
    nsIContent* dispatchTo = popupContent ? popupContent : mMenuContent;
    rv = dispatchTo->DispatchDOMEvent(&event, nsnull, presContext, &status);
    if ( NS_FAILED(rv) || status == nsEventStatus_eConsumeNoDefault )
      return PR_FALSE;
 }

  // the menu is going to show and the oncreate handler has executed. We
  // now need to walk our menu items, checking to see if any of them have
  // a command attribute. If so, several apptributes must potentially
  // be updated.
  if (popupContent) {
    nsCOMPtr<nsIDOMDocument> domDoc(do_QueryInterface(popupContent->GetDocument()));

    PRUint32 count = popupContent->GetChildCount();
    for (PRUint32 i = 0; i < count; i++) {
      nsIContent *grandChild = popupContent->GetChildAt(i);
      if (grandChild->Tag() == nsWidgetAtoms::menuitem) {
        // See if we have a command attribute.
        nsAutoString command;
        grandChild->GetAttr(kNameSpaceID_None, nsWidgetAtoms::command, command);
        if (!command.IsEmpty()) {
          // We do! Look it up in our document
          nsCOMPtr<nsIDOMElement> commandElt;
          domDoc->GetElementById(command, getter_AddRefs(commandElt));
          nsCOMPtr<nsIContent> commandContent(do_QueryInterface(commandElt));

          if ( commandContent ) {
            nsAutoString commandDisabled, menuDisabled;
            commandContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::disabled, commandDisabled);
            grandChild->GetAttr(kNameSpaceID_None, nsWidgetAtoms::disabled, menuDisabled);
            if (!commandDisabled.Equals(menuDisabled)) {
              // The menu's disabled state needs to be updated to match the command.
              if (commandDisabled.IsEmpty()) 
                grandChild->UnsetAttr(kNameSpaceID_None, nsWidgetAtoms::disabled, PR_TRUE);
              else grandChild->SetAttr(kNameSpaceID_None, nsWidgetAtoms::disabled, commandDisabled, PR_TRUE);
            }

            // The menu's value and checked states need to be updated to match the command.
            // Note that (unlike the disabled state) if the command has *no* value for either, we
            // assume the menu is supplying its own.
            nsAutoString commandChecked, menuChecked;
            commandContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::checked, commandChecked);
            grandChild->GetAttr(kNameSpaceID_None, nsWidgetAtoms::checked, menuChecked);
            if (!commandChecked.Equals(menuChecked)) {
              if (!commandChecked.IsEmpty()) 
                grandChild->SetAttr(kNameSpaceID_None, nsWidgetAtoms::checked, commandChecked, PR_TRUE);
            }

            nsAutoString commandValue, menuValue;
            commandContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::label, commandValue);
            grandChild->GetAttr(kNameSpaceID_None, nsWidgetAtoms::label, menuValue);
            if (!commandValue.Equals(menuValue)) {
              if (!commandValue.IsEmpty()) 
                grandChild->SetAttr(kNameSpaceID_None, nsWidgetAtoms::label, commandValue, PR_TRUE);
            }
          }
        }
      }
    }
  }
  
  return PR_TRUE;
}

PRBool
nsMenuX::OnCreated()
{
  nsEventStatus status = nsEventStatus_eIgnore;
  nsMouseEvent event(PR_TRUE, NS_XUL_POPUP_SHOWN, nsnull, nsMouseEvent::eReal);
  
  nsCOMPtr<nsIContent> popupContent;
  GetMenuPopupContent(getter_AddRefs(popupContent));

  nsCOMPtr<nsIDocShell> docShell = do_QueryReferent(mDocShellWeakRef);
  if (!docShell) {
    NS_ERROR("No doc shell");
    return PR_FALSE;
  }
  nsCOMPtr<nsPresContext> presContext;
  MenuHelpersX::DocShellToPresContext(docShell, getter_AddRefs(presContext) );
  if ( presContext ) {
    nsresult rv = NS_OK;
    nsIContent* dispatchTo = popupContent ? popupContent : mMenuContent;
    rv = dispatchTo->DispatchDOMEvent(&event, nsnull, presContext, &status);
    if ( NS_FAILED(rv) || status == nsEventStatus_eConsumeNoDefault )
      return PR_FALSE;
 }
  
  return PR_TRUE;
}

//
// OnDestroy
//
// Fire our ondestroy handler. Returns TRUE if we should keep processing the event,
// FALSE if the handler wants to stop the destruction of the menu
//
PRBool
nsMenuX::OnDestroy()
{
  if ( mDestroyHandlerCalled )
    return PR_TRUE;

  nsEventStatus status = nsEventStatus_eIgnore;
  nsMouseEvent event(PR_TRUE, NS_XUL_POPUP_HIDING, nsnull,
                     nsMouseEvent::eReal);
  
  nsCOMPtr<nsIDocShell>  docShell = do_QueryReferent(mDocShellWeakRef);
  if (!docShell) {
    NS_WARNING("No doc shell so can't run the OnDestroy");
    return PR_FALSE;
  }

  nsCOMPtr<nsIContent> popupContent;
  GetMenuPopupContent(getter_AddRefs(popupContent));

  nsCOMPtr<nsPresContext> presContext;
  MenuHelpersX::DocShellToPresContext (docShell, getter_AddRefs(presContext) );
  if (presContext )  {
    nsresult rv = NS_OK;
    nsIContent* dispatchTo = popupContent ? popupContent : mMenuContent;
    rv = dispatchTo->DispatchDOMEvent(&event, nsnull, presContext, &status);

    mDestroyHandlerCalled = PR_TRUE;
    
    if ( NS_FAILED(rv) || status == nsEventStatus_eConsumeNoDefault )
      return PR_FALSE;
  }
  return PR_TRUE;
}

PRBool
nsMenuX::OnDestroyed()
{
  nsEventStatus status = nsEventStatus_eIgnore;
  nsMouseEvent event(PR_TRUE, NS_XUL_POPUP_HIDDEN, nsnull,
                     nsMouseEvent::eReal);
  
  nsCOMPtr<nsIDocShell>  docShell = do_QueryReferent(mDocShellWeakRef);
  if (!docShell) {
    NS_WARNING("No doc shell so can't run the OnDestroy");
    return PR_FALSE;
  }

  nsCOMPtr<nsIContent> popupContent;
  GetMenuPopupContent(getter_AddRefs(popupContent));

  nsCOMPtr<nsPresContext> presContext;
  MenuHelpersX::DocShellToPresContext (docShell, getter_AddRefs(presContext) );
  if (presContext )  {
    nsresult rv = NS_OK;
    nsIContent* dispatchTo = popupContent ? popupContent : mMenuContent;
    rv = dispatchTo->DispatchDOMEvent(&event, nsnull, presContext, &status);

    mDestroyHandlerCalled = PR_TRUE;
    
    if ( NS_FAILED(rv) || status == nsEventStatus_eConsumeNoDefault )
      return PR_FALSE;
  }
  return PR_TRUE;
}
//
// GetMenuPopupContent
//
// Find the |menupopup| child in the |popup| representing this menu. It should be one
// of a very few children so we won't be iterating over a bazillion menu items to find
// it (so the strcmp won't kill us).
//
void
nsMenuX::GetMenuPopupContent(nsIContent** aResult)
{
  if (!aResult )
    return;
  *aResult = nsnull;
  
  nsresult rv;
  nsCOMPtr<nsIXBLService> xblService = 
           do_GetService("@mozilla.org/xbl;1", &rv);
  if ( !xblService )
    return;
  
  PRUint32 count = mMenuContent->GetChildCount();

  for (PRUint32 i = 0; i < count; i++) {
    PRInt32 dummy;
    nsIContent *child = mMenuContent->GetChildAt(i);
    nsCOMPtr<nsIAtom> tag;
    xblService->ResolveTag(child, &dummy, getter_AddRefs(tag));
    if (tag == nsWidgetAtoms::menupopup) {
      *aResult = child;
      NS_ADDREF(*aResult);
      return;
    }
  }

} // GetMenuPopupContent


//
// CountVisibleBefore
//
// Determines how many menus are visible among the siblings that are before me.
// It doesn't matter if I am visible. Note that this will always count the Apple
// menu, since we always put it in there.
//
nsresult 
nsMenuX :: CountVisibleBefore ( PRUint32* outVisibleBefore )
{
  NS_ASSERTION ( outVisibleBefore, "bad index param" );
  
  nsCOMPtr<nsIMenuBar> menubarParent = do_QueryInterface(mParent);
  if (!menubarParent) return NS_ERROR_FAILURE;

  PRUint32 numMenus = 0;
  menubarParent->GetMenuCount(numMenus);
  
  // Find this menu among the children of my parent menubar
  PRBool gotThisMenu = PR_FALSE;
  *outVisibleBefore = 1;                            // start at 1, the apple menu will always be there
  for ( PRUint32 i = 0; i < numMenus; ++i ) {
    nsCOMPtr<nsIMenu> currMenu;
    menubarParent->GetMenuAt(i, *getter_AddRefs(currMenu));
    
    // we found ourselves, break out
    if ( currMenu == NS_STATIC_CAST(nsIMenu*, this) ) {
      gotThisMenu = PR_TRUE;
      break;
    }
      
    // check the current menu to see if it is visible (not hidden, not collapsed). If
    // it is, count it.
    if (currMenu) {
      nsCOMPtr<nsIContent> menuContent;
      currMenu->GetMenuContent(getter_AddRefs(menuContent));
      if ( menuContent ) {
        if ( menuContent->GetChildCount() > 0 ||
             !menuContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::hidden,
                                       nsWidgetAtoms::_true, eCaseMatters) &&
             !menuContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::collapsed,
                                       nsWidgetAtoms::_true, eCaseMatters))
          ++(*outVisibleBefore);
      }
    }
    
  } // for each menu

  return gotThisMenu ? NS_OK : NS_ERROR_FAILURE;

} // CountVisibleBefore


NS_IMETHODIMP
nsMenuX::ChangeNativeEnabledStatusForMenuItem(nsIMenuItem* aMenuItem, PRBool aEnabled)
{
  // look for the menu item given
  PRUint32 menuItemCount;
  mMenuItemsArray.Count(&menuItemCount);
    
  for (PRUint32 i = 0; i < menuItemCount; i++) {
    nsISupports* currItem;
    mMenuItemsArray.GetElementAt(i, &currItem);
    if (currItem == aMenuItem) {
      if (aEnabled)
        ::EnableMenuItem(mMacMenuHandle, i + 1);
      else
        ::DisableMenuItem(mMacMenuHandle, i + 1);
      break;
    }
  }

  return NS_OK;
}


#pragma mark -

//
// nsIChangeObserver
//


NS_IMETHODIMP
nsMenuX::AttributeChanged(nsIDocument *aDocument, PRInt32 aNameSpaceID, nsIContent *aContent, nsIAtom *aAttribute)
{
  if (gConstructingMenu)
    return NS_OK;

  // ignore the |open| attribute, which is by far the most common
  if ( aAttribute == nsWidgetAtoms::open )
    return NS_OK;
    
  nsCOMPtr<nsIMenuBar> menubarParent = do_QueryInterface(mParent);

  if (aAttribute == nsWidgetAtoms::disabled) {
    SetRebuild(PR_TRUE);
   
    if (mMenuContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::disabled,
                                  nsWidgetAtoms::_true, eCaseMatters))
      SetEnabled(PR_FALSE);
    else
      SetEnabled(PR_TRUE);
      
    ::DrawMenuBar();
  } 
  else if (aAttribute == nsWidgetAtoms::label) {
    SetRebuild(PR_TRUE);
    
    mMenuContent->GetAttr(kNameSpaceID_None, nsWidgetAtoms::label, mLabel);

    // invalidate my parent. If we're a submenu parent, we have to rebuild
    // the parent menu in order for the changes to be picked up. If we're
    // a regular menu, just change the title and redraw the menubar.
    if ( !menubarParent ) {
      nsCOMPtr<nsIMenuListener> parentListener ( do_QueryInterface(mParent) );
      parentListener->SetRebuild(PR_TRUE);
    }
    else {
      // reuse the existing menu, to avoid rebuilding the root menu bar.
      NS_ASSERTION(mMacMenuHandle != NULL, "nsMenuX::AttributeChanged: invalid menu handle.");
      RemoveAll();
      CFStringRef titleRef = ::CFStringCreateWithCharacters(kCFAllocatorDefault, (UniChar*)mLabel.get(), mLabel.Length());
      NS_ASSERTION(titleRef, "nsMenuX::AttributeChanged: CFStringCreateWithCharacters failed.");
      ::SetMenuTitleWithCFString(mMacMenuHandle, titleRef);
      ::CFRelease(titleRef);
      
      ::DrawMenuBar();
    }
  }
  else if (aAttribute == nsWidgetAtoms::hidden || aAttribute == nsWidgetAtoms::collapsed) {
    SetRebuild(PR_TRUE);
      
      if (mMenuContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::hidden,
                                    nsWidgetAtoms::_true, eCaseMatters) ||
          mMenuContent->AttrValueIs(kNameSpaceID_None, nsWidgetAtoms::collapsed,
                                    nsWidgetAtoms::_true, eCaseMatters)) {
        if ( mVisible ) {
          if ( menubarParent ) {
            PRUint32 indexToRemove = 0;
            if ( NS_SUCCEEDED(CountVisibleBefore(&indexToRemove)) ) {
              ++indexToRemove;                // if there are N siblings before me, my index is N+1
              void *clientData = nsnull;
              menubarParent->GetNativeData ( clientData );
              if ( clientData ) {
                MenuRef menubar = reinterpret_cast<MenuRef>(clientData);
                ::SetMenuItemHierarchicalMenu(menubar, indexToRemove, nsnull);
                ::DeleteMenuItem(menubar, indexToRemove);
                mVisible = PR_FALSE;
              }
            }
          } // if on the menubar
          else {
            // hide this submenu
            NS_ASSERTION(PR_FALSE, "nsMenuX::AttributeChanged: WRITE HIDE CODE FOR SUBMENU.");
          }
        } // if visible
        else
          NS_WARNING("You're hiding the menu twice, please stop");
      } // if told to hide menu
      else {
        if ( !mVisible ) {
          if ( menubarParent ) {
            PRUint32 insertAfter = 0;
            if ( NS_SUCCEEDED(CountVisibleBefore(&insertAfter)) ) {
              void *clientData = nsnull;
              menubarParent->GetNativeData ( clientData );
              if ( clientData ) {
                MenuRef menubar = reinterpret_cast<MenuRef>(clientData);
                // Shove this menu into its rightful place in the menubar. It doesn't matter
                // what title we pass to InsertMenuItem() because when we stuff the actual menu
                // handle in, the correct title goes with it.
                ::InsertMenuItem(menubar, "\pPlaceholder", insertAfter);
                ::SetMenuItemHierarchicalMenu(menubar, insertAfter + 1, mMacMenuHandle);  // add 1 to get index of inserted item
                mVisible = PR_TRUE;
              }
            }
          } // if on menubar
          else {
            // show this submenu
            NS_ASSERTION(PR_FALSE, "nsMenuX::AttributeChanged: WRITE SHOW CODE FOR SUBMENU.");
          }
        } // if not visible
      } // if told to show menu

      if (menubarParent) {
        ::DrawMenuBar();
      }
  }

  return NS_OK;
  
} // AttributeChanged


NS_IMETHODIMP
nsMenuX :: ContentRemoved(nsIDocument *aDocument, nsIContent *aChild, PRInt32 aIndexInContainer)
{  
  if (gConstructingMenu)
    return NS_OK;

    SetRebuild(PR_TRUE);

  RemoveItem(aIndexInContainer);
  mManager->Unregister(aChild);

  return NS_OK;
  
} // ContentRemoved


NS_IMETHODIMP
nsMenuX :: ContentInserted(nsIDocument *aDocument, nsIContent *aChild, PRInt32 aIndexInContainer)
{  
  if(gConstructingMenu)
    return NS_OK;

    SetRebuild(PR_TRUE);
  
  return NS_OK;
  
} // ContentInserted
