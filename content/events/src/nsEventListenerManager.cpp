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

#include "nsISupports.h"
#include "nsGUIEvent.h"
#include "nsDOMEvent.h"
#include "nsEventListenerManager.h"
#include "nsICaret.h"
#include "nsIDOMNSEvent.h"
#include "nsIDOMEventListener.h"
#include "nsIDOMMouseListener.h"
#include "nsIDOMMouseMotionListener.h"
#include "nsIDOMContextMenuListener.h"
#include "nsIDOMKeyListener.h"
#include "nsIDOMFocusListener.h"
#include "nsIDOMFormListener.h"
#include "nsIDOMLoadListener.h"
#include "nsIDOMDragListener.h"
#include "nsIDOMPaintListener.h"
#include "nsIDOMTextListener.h"
#include "nsIDOMCompositionListener.h"
#include "nsIDOMXULListener.h"
#include "nsIDOMScrollListener.h"
#include "nsIDOMMutationListener.h"
#include "nsIDOMUIListener.h"
#include "nsIDOMPageTransitionListener.h"
#include "nsITextControlFrame.h"
#ifdef MOZ_SVG
#include "nsIDOMSVGListener.h"
#include "nsIDOMSVGZoomListener.h"
#include "nsSVGAtoms.h"
#endif // MOZ_SVG
#include "nsIEventStateManager.h"
#include "nsPIDOMWindow.h"
#include "nsIPrivateDOMEvent.h"
#include "nsIJSEventListener.h"
#include "prmem.h"
#include "nsIScriptGlobalObject.h"
#include "nsIScriptRuntime.h"
#include "nsLayoutAtoms.h"
#include "nsLayoutUtils.h"
#ifdef MOZ_XUL
// XXXbz the fact that this is ifdef MOZ_XUL is good indication that
// it doesn't belong here...
#include "nsITreeBoxObject.h"
#include "nsITreeColumns.h"
#include "nsIDOMXULMultSelectCntrlEl.h"
#endif
#include "nsINameSpaceManager.h"
#include "nsIContent.h"
#include "nsIFrame.h"
#include "nsIView.h"
#include "nsIViewManager.h"
#include "nsIScrollableView.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nsIScriptSecurityManager.h"
#include "nsDOMError.h"
#include "nsIJSContextStack.h"
#include "nsIDocument.h"
#include "nsIPresShell.h"
#include "nsMutationEvent.h"
#include "nsIXPConnect.h"
#include "nsDOMCID.h"
#include "nsIScriptObjectOwner.h" // for nsIScriptEventHandlerOwner
#include "nsIFocusController.h"
#include "nsIDOMElement.h"
#include "nsIBoxObject.h"
#include "nsIDOMNSDocument.h"
#include "nsIWidget.h"
#include "nsContentUtils.h"
#include "nsJSUtils.h"
#include "nsIDOMEventGroup.h"
#include "nsContentCID.h"
#include "nsEventDispatcher.h"
#include "nsDOMJSUtils.h"
#include "nsDOMScriptObjectHolder.h"

static NS_DEFINE_CID(kDOMScriptObjectFactoryCID,
                     NS_DOM_SCRIPT_OBJECT_FACTORY_CID);
static NS_DEFINE_CID(kDOMEventGroupCID, NS_DOMEVENTGROUP_CID);

typedef
NS_STDCALL_FUNCPROTO(nsresult,
                     GenericHandler,
                     nsIDOMEventListener, HandleEvent, 
                     (nsIDOMEvent*));

/*
 * Things here are not as they appear.  Namely, |ifaceListener| below is
 * not really a pointer to the nsIDOMEventListener interface, and aMethod is
 * not really a pointer-to-member for nsIDOMEventListener.  They both
 * actually refer to the event-type-specific listener interface.  The casting
 * magic allows us to use a single dispatch method.  This relies on the
 * assumption that nsIDOMEventListener and the event type listener interfaces
 * have the same object layout and will therefore have compatible
 * pointer-to-member implementations.
 */

static nsresult DispatchToInterface(nsIDOMEvent* aEvent,
                                    nsIDOMEventListener* aListener,
                                    GenericHandler aMethod,
                                    const nsIID& aIID,
                                    PRBool* aHasInterface)
{
  nsIDOMEventListener* ifaceListener = nsnull;
  nsresult rv = NS_OK;
  aListener->QueryInterface(aIID, (void**) &ifaceListener);
  if (ifaceListener) {
    *aHasInterface = PR_TRUE;
    rv = (ifaceListener->*aMethod)(aEvent);
    NS_RELEASE(ifaceListener);
  }
  return rv;
}

struct EventDispatchData
{
  PRUint32 message;
  GenericHandler method;
  PRUint8 bits;
};

struct EventTypeData
{
  const EventDispatchData* events;
  int                      numEvents;
  const nsIID*             iid;
};

#define HANDLER(x) NS_REINTERPRET_CAST(GenericHandler, x)

static const EventDispatchData sMouseEvents[] = {
  { NS_MOUSE_LEFT_BUTTON_DOWN,   HANDLER(&nsIDOMMouseListener::MouseDown),
    NS_EVENT_BITS_MOUSE_MOUSEDOWN },
  { NS_MOUSE_MIDDLE_BUTTON_DOWN, HANDLER(&nsIDOMMouseListener::MouseDown),
    NS_EVENT_BITS_MOUSE_MOUSEDOWN },
  { NS_MOUSE_RIGHT_BUTTON_DOWN,  HANDLER(&nsIDOMMouseListener::MouseDown),
    NS_EVENT_BITS_MOUSE_MOUSEDOWN },
  { NS_MOUSE_LEFT_BUTTON_UP,     HANDLER(&nsIDOMMouseListener::MouseUp),
    NS_EVENT_BITS_MOUSE_MOUSEUP },
  { NS_MOUSE_MIDDLE_BUTTON_UP,   HANDLER(&nsIDOMMouseListener::MouseUp),
    NS_EVENT_BITS_MOUSE_MOUSEUP },
  { NS_MOUSE_RIGHT_BUTTON_UP,    HANDLER(&nsIDOMMouseListener::MouseUp),
    NS_EVENT_BITS_MOUSE_MOUSEUP },
  { NS_MOUSE_LEFT_CLICK,         HANDLER(&nsIDOMMouseListener::MouseClick),
    NS_EVENT_BITS_MOUSE_CLICK },
  { NS_MOUSE_MIDDLE_CLICK,       HANDLER(&nsIDOMMouseListener::MouseClick),
    NS_EVENT_BITS_MOUSE_CLICK },
  { NS_MOUSE_RIGHT_CLICK,        HANDLER(&nsIDOMMouseListener::MouseClick),
    NS_EVENT_BITS_MOUSE_CLICK },
  { NS_MOUSE_LEFT_DOUBLECLICK,   HANDLER(&nsIDOMMouseListener::MouseDblClick),
    NS_EVENT_BITS_MOUSE_DBLCLICK },
  { NS_MOUSE_MIDDLE_DOUBLECLICK, HANDLER(&nsIDOMMouseListener::MouseDblClick),
    NS_EVENT_BITS_MOUSE_DBLCLICK },
  { NS_MOUSE_RIGHT_DOUBLECLICK,  HANDLER(&nsIDOMMouseListener::MouseDblClick),
    NS_EVENT_BITS_MOUSE_DBLCLICK },
  { NS_MOUSE_ENTER_SYNTH,        HANDLER(&nsIDOMMouseListener::MouseOver),
    NS_EVENT_BITS_MOUSE_MOUSEOVER },
  { NS_MOUSE_EXIT_SYNTH,         HANDLER(&nsIDOMMouseListener::MouseOut),
    NS_EVENT_BITS_MOUSE_MOUSEOUT }
};

static const EventDispatchData sMouseMotionEvents[] = {
  { NS_MOUSE_MOVE, HANDLER(&nsIDOMMouseMotionListener::MouseMove),
    NS_EVENT_BITS_MOUSEMOTION_MOUSEMOVE }
};

static const EventDispatchData sContextMenuEvents[] = {
  { NS_CONTEXTMENU,     HANDLER(&nsIDOMContextMenuListener::ContextMenu),
    NS_EVENT_BITS_CONTEXT_MENU },
  { NS_CONTEXTMENU_KEY, HANDLER(&nsIDOMContextMenuListener::ContextMenu),
    NS_EVENT_BITS_CONTEXT_MENU }
};

static const EventDispatchData sCompositionEvents[] = {
  { NS_COMPOSITION_START,  HANDLER(&nsIDOMCompositionListener::HandleStartComposition),
    NS_EVENT_BITS_COMPOSITION_START },
  { NS_COMPOSITION_END,    HANDLER(&nsIDOMCompositionListener::HandleEndComposition),
    NS_EVENT_BITS_COMPOSITION_END },
  { NS_COMPOSITION_QUERY,  HANDLER(&nsIDOMCompositionListener::HandleQueryComposition),
    NS_EVENT_BITS_COMPOSITION_QUERY },
  { NS_RECONVERSION_QUERY, HANDLER(&nsIDOMCompositionListener::HandleQueryReconversion),
    NS_EVENT_BITS_COMPOSITION_RECONVERSION },
  { NS_QUERYCARETRECT,  HANDLER(&nsIDOMCompositionListener::HandleQueryCaretRect),
    NS_EVENT_BITS_COMPOSITION_QUERYCARETRECT }
};

static const EventDispatchData sTextEvents[] = {
  {NS_TEXT_TEXT,HANDLER(&nsIDOMTextListener::HandleText),NS_EVENT_BITS_TEXT_TEXT},
};

static const EventDispatchData sKeyEvents[] = {
  {NS_KEY_UP,   HANDLER(&nsIDOMKeyListener::KeyUp),   NS_EVENT_BITS_KEY_KEYUP},
  {NS_KEY_DOWN, HANDLER(&nsIDOMKeyListener::KeyDown), NS_EVENT_BITS_KEY_KEYDOWN},
  {NS_KEY_PRESS,HANDLER(&nsIDOMKeyListener::KeyPress),NS_EVENT_BITS_KEY_KEYPRESS},
};

static const EventDispatchData sFocusEvents[] = {
  {NS_FOCUS_CONTENT,HANDLER(&nsIDOMFocusListener::Focus),NS_EVENT_BITS_FOCUS_FOCUS},
  {NS_BLUR_CONTENT, HANDLER(&nsIDOMFocusListener::Blur), NS_EVENT_BITS_FOCUS_BLUR }
};

static const EventDispatchData sFormEvents[] = {
  {NS_FORM_SUBMIT,  HANDLER(&nsIDOMFormListener::Submit),NS_EVENT_BITS_FORM_SUBMIT},
  {NS_FORM_RESET,   HANDLER(&nsIDOMFormListener::Reset), NS_EVENT_BITS_FORM_RESET},
  {NS_FORM_CHANGE,  HANDLER(&nsIDOMFormListener::Change),NS_EVENT_BITS_FORM_CHANGE},
  {NS_FORM_SELECTED,HANDLER(&nsIDOMFormListener::Select),NS_EVENT_BITS_FORM_SELECT},
  {NS_FORM_INPUT,   HANDLER(&nsIDOMFormListener::Input), NS_EVENT_BITS_FORM_INPUT}
};

static const EventDispatchData sLoadEvents[] = {
  {NS_PAGE_LOAD,   HANDLER(&nsIDOMLoadListener::Load),  NS_EVENT_BITS_LOAD_LOAD},
  {NS_IMAGE_LOAD,  HANDLER(&nsIDOMLoadListener::Load),  NS_EVENT_BITS_LOAD_LOAD},
  {NS_SCRIPT_LOAD, HANDLER(&nsIDOMLoadListener::Load),  NS_EVENT_BITS_LOAD_LOAD},
  {NS_PAGE_UNLOAD, HANDLER(&nsIDOMLoadListener::Unload),NS_EVENT_BITS_LOAD_UNLOAD},
  {NS_IMAGE_ERROR, HANDLER(&nsIDOMLoadListener::Error), NS_EVENT_BITS_LOAD_ERROR},
  {NS_SCRIPT_ERROR,HANDLER(&nsIDOMLoadListener::Error), NS_EVENT_BITS_LOAD_ERROR},
  {NS_BEFORE_PAGE_UNLOAD,HANDLER(&nsIDOMLoadListener::BeforeUnload), NS_EVENT_BITS_LOAD_BEFORE_UNLOAD},
};

static const EventDispatchData sPaintEvents[] = {
  {NS_PAINT,       HANDLER(&nsIDOMPaintListener::Paint), NS_EVENT_BITS_PAINT_PAINT},
  {NS_RESIZE_EVENT,HANDLER(&nsIDOMPaintListener::Resize),NS_EVENT_BITS_PAINT_RESIZE},
  {NS_SCROLL_EVENT,HANDLER(&nsIDOMPaintListener::Scroll),NS_EVENT_BITS_PAINT_SCROLL}
};

static const EventDispatchData sDragEvents[] = {
  {NS_DRAGDROP_ENTER,     HANDLER(&nsIDOMDragListener::DragEnter),  NS_EVENT_BITS_DRAG_ENTER},
  {NS_DRAGDROP_OVER_SYNTH,HANDLER(&nsIDOMDragListener::DragOver),   NS_EVENT_BITS_DRAG_OVER},
  {NS_DRAGDROP_EXIT_SYNTH,HANDLER(&nsIDOMDragListener::DragExit),   NS_EVENT_BITS_DRAG_EXIT},
  {NS_DRAGDROP_DROP,      HANDLER(&nsIDOMDragListener::DragDrop),   NS_EVENT_BITS_DRAG_DROP},
  {NS_DRAGDROP_GESTURE,   HANDLER(&nsIDOMDragListener::DragGesture),NS_EVENT_BITS_DRAG_GESTURE}
};

static const EventDispatchData sScrollEvents[] = {
  { NS_SCROLLPORT_OVERFLOW,        HANDLER(&nsIDOMScrollListener::Overflow),
    NS_EVENT_BITS_SCROLLPORT_OVERFLOW },
  { NS_SCROLLPORT_UNDERFLOW,       HANDLER(&nsIDOMScrollListener::Underflow),
    NS_EVENT_BITS_SCROLLPORT_UNDERFLOW },
  { NS_SCROLLPORT_OVERFLOWCHANGED, HANDLER(&nsIDOMScrollListener::OverflowChanged),
    NS_EVENT_BITS_SCROLLPORT_OVERFLOWCHANGED }
};

static const EventDispatchData sXULEvents[] = {
  { NS_XUL_POPUP_SHOWING,  HANDLER(&nsIDOMXULListener::PopupShowing),
    NS_EVENT_BITS_XUL_POPUP_SHOWING },
  { NS_XUL_POPUP_SHOWN,    HANDLER(&nsIDOMXULListener::PopupShown),
    NS_EVENT_BITS_XUL_POPUP_SHOWN },
  { NS_XUL_POPUP_HIDING,   HANDLER(&nsIDOMXULListener::PopupHiding),
    NS_EVENT_BITS_XUL_POPUP_HIDING },
  { NS_XUL_POPUP_HIDDEN,   HANDLER(&nsIDOMXULListener::PopupHidden),
    NS_EVENT_BITS_XUL_POPUP_HIDDEN },
  { NS_XUL_CLOSE,          HANDLER(&nsIDOMXULListener::Close),
    NS_EVENT_BITS_XUL_CLOSE },
  { NS_XUL_COMMAND,        HANDLER(&nsIDOMXULListener::Command),
    NS_EVENT_BITS_XUL_COMMAND },
  { NS_XUL_BROADCAST,      HANDLER(&nsIDOMXULListener::Broadcast),
    NS_EVENT_BITS_XUL_BROADCAST },
  { NS_XUL_COMMAND_UPDATE, HANDLER(&nsIDOMXULListener::CommandUpdate),
    NS_EVENT_BITS_XUL_COMMAND_UPDATE }
};

static const EventDispatchData sMutationEvents[] = {
  { NS_MUTATION_SUBTREEMODIFIED,
    HANDLER(&nsIDOMMutationListener::SubtreeModified),
    NS_EVENT_BITS_MUTATION_SUBTREEMODIFIED },
  { NS_MUTATION_NODEINSERTED,
    HANDLER(&nsIDOMMutationListener::NodeInserted),
    NS_EVENT_BITS_MUTATION_NODEINSERTED },
  { NS_MUTATION_NODEREMOVED,
    HANDLER(&nsIDOMMutationListener::NodeRemoved),
    NS_EVENT_BITS_MUTATION_NODEREMOVED },
  { NS_MUTATION_NODEINSERTEDINTODOCUMENT,
    HANDLER(&nsIDOMMutationListener::NodeInsertedIntoDocument),
    NS_EVENT_BITS_MUTATION_NODEINSERTEDINTODOCUMENT },
  { NS_MUTATION_NODEREMOVEDFROMDOCUMENT,
    HANDLER(&nsIDOMMutationListener::NodeRemovedFromDocument),
    NS_EVENT_BITS_MUTATION_NODEREMOVEDFROMDOCUMENT },
  { NS_MUTATION_ATTRMODIFIED,
    HANDLER(&nsIDOMMutationListener::AttrModified),
    NS_EVENT_BITS_MUTATION_ATTRMODIFIED },
  { NS_MUTATION_CHARACTERDATAMODIFIED,
    HANDLER(&nsIDOMMutationListener::CharacterDataModified),
    NS_EVENT_BITS_MUTATION_CHARACTERDATAMODIFIED }
};

static const EventDispatchData sUIEvents[] = {
  { NS_UI_ACTIVATE, HANDLER(&nsIDOMUIListener::Activate),
    NS_EVENT_BITS_UI_ACTIVATE },
  { NS_UI_FOCUSIN, HANDLER(&nsIDOMUIListener::FocusIn),
    NS_EVENT_BITS_UI_FOCUSIN },
  { NS_UI_FOCUSOUT, HANDLER(&nsIDOMUIListener::FocusOut),
    NS_EVENT_BITS_UI_FOCUSOUT }
};

static const EventDispatchData sPageTransitionEvents[] = {
  { NS_PAGE_SHOW, HANDLER(&nsIDOMPageTransitionListener::PageShow),
    NS_EVENT_BITS_PAGETRANSITION_SHOW },
  { NS_PAGE_HIDE, HANDLER(&nsIDOMPageTransitionListener::PageHide),
    NS_EVENT_BITS_PAGETRANSITION_HIDE }
};

#ifdef MOZ_SVG
static const EventDispatchData sSVGEvents[] = {
  { NS_SVG_LOAD, HANDLER(&nsIDOMSVGListener::Load),
    NS_EVENT_BITS_SVG_LOAD },
  { NS_SVG_UNLOAD, HANDLER(&nsIDOMSVGListener::Unload),
    NS_EVENT_BITS_SVG_UNLOAD },
  { NS_SVG_ABORT, HANDLER(&nsIDOMSVGListener::Abort),
    NS_EVENT_BITS_SVG_ABORT },
  { NS_SVG_ERROR, HANDLER(&nsIDOMSVGListener::Error),
    NS_EVENT_BITS_SVG_ERROR },
  { NS_SVG_RESIZE, HANDLER(&nsIDOMSVGListener::Resize),
    NS_EVENT_BITS_SVG_RESIZE },
  { NS_SVG_SCROLL, HANDLER(&nsIDOMSVGListener::Scroll),
    NS_EVENT_BITS_SVG_SCROLL }
};

static const EventDispatchData sSVGZoomEvents[] = {
  { NS_SVG_ZOOM, HANDLER(&nsIDOMSVGZoomListener::Zoom),
    NS_EVENT_BITS_SVGZOOM_ZOOM }
};
#endif // MOZ_SVG

#define IMPL_EVENTTYPEDATA(type) \
{ \
  s##type##Events, \
  NS_ARRAY_LENGTH(s##type##Events), \
  &NS_GET_IID(nsIDOM##type##Listener) \
}
 
// IMPORTANT: indices match up with eEventArrayType_ enum values

static const EventTypeData sEventTypes[] = {
  IMPL_EVENTTYPEDATA(Mouse),
  IMPL_EVENTTYPEDATA(MouseMotion),
  IMPL_EVENTTYPEDATA(ContextMenu),
  IMPL_EVENTTYPEDATA(Key),
  IMPL_EVENTTYPEDATA(Load),
  IMPL_EVENTTYPEDATA(Focus),
  IMPL_EVENTTYPEDATA(Form),
  IMPL_EVENTTYPEDATA(Drag),
  IMPL_EVENTTYPEDATA(Paint),
  IMPL_EVENTTYPEDATA(Text),
  IMPL_EVENTTYPEDATA(Composition),
  IMPL_EVENTTYPEDATA(XUL),
  IMPL_EVENTTYPEDATA(Scroll),
  IMPL_EVENTTYPEDATA(Mutation),
  IMPL_EVENTTYPEDATA(UI),
  IMPL_EVENTTYPEDATA(PageTransition)
#ifdef MOZ_SVG
 ,
  IMPL_EVENTTYPEDATA(SVG),
  IMPL_EVENTTYPEDATA(SVGZoom)
#endif // MOZ_SVG
};

// Strong references to event groups
nsIDOMEventGroup* gSystemEventGroup;
nsIDOMEventGroup* gDOM2EventGroup;

PRUint32 nsEventListenerManager::mInstanceCount = 0;

nsEventListenerManager::nsEventListenerManager() :
  mManagerType(NS_ELM_NONE),
  mListenersRemoved(PR_FALSE),
  mSingleListenerType(eEventArrayType_None),
  mSingleListener(nsnull),
  mMultiListeners(nsnull),
  mGenericListeners(nsnull),
  mTarget(nsnull)
{
  ++mInstanceCount;
}

static PRBool PR_CALLBACK
GenericListenersHashEnum(nsHashKey *aKey, void *aData, void* closure)
{
  nsVoidArray* listeners = NS_STATIC_CAST(nsVoidArray*, aData);
  if (listeners) {
    PRInt32 i, count = listeners->Count();
    nsListenerStruct *ls;
    for (i = count-1; i >= 0; --i) {
      ls = (nsListenerStruct*)listeners->ElementAt(i);
      if (ls) {
        delete ls;
      }
    }
    delete listeners;
  }
  return PR_TRUE;
}

nsEventListenerManager::~nsEventListenerManager() 
{
  RemoveAllListeners();

  --mInstanceCount;
  if(mInstanceCount == 0) {
    NS_IF_RELEASE(gSystemEventGroup);
    NS_IF_RELEASE(gDOM2EventGroup);
  }
}

nsresult
nsEventListenerManager::RemoveAllListeners()
{
  mListenersRemoved = PR_TRUE;

  ReleaseListeners(&mSingleListener);
  if (!mSingleListener) {
    mSingleListenerType = eEventArrayType_None;
    mManagerType &= ~NS_ELM_SINGLE;
  }

  if (mMultiListeners) {
    // XXX probably should just be i < Count()
    for (PRInt32 i=0; i<EVENT_ARRAY_TYPE_LENGTH && i < mMultiListeners->Count(); i++) {
      nsVoidArray* listeners;
      listeners = NS_STATIC_CAST(nsVoidArray*, mMultiListeners->ElementAt(i));
      ReleaseListeners(&listeners);
    }
    delete mMultiListeners;
    mMultiListeners = nsnull;
    mManagerType &= ~NS_ELM_MULTI;
  }

  if (mGenericListeners) {
    mGenericListeners->Enumerate(GenericListenersHashEnum, nsnull);
    //hash destructor
    delete mGenericListeners;
    mGenericListeners = nsnull;
    mManagerType &= ~NS_ELM_HASH;
  }

  return NS_OK;
}

void
nsEventListenerManager::Shutdown()
{
    sAddListenerID = JSVAL_VOID;

    nsDOMEvent::Shutdown();
}

NS_IMPL_ADDREF(nsEventListenerManager)
NS_IMPL_RELEASE(nsEventListenerManager)


NS_INTERFACE_MAP_BEGIN(nsEventListenerManager)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIEventListenerManager)
   NS_INTERFACE_MAP_ENTRY(nsIEventListenerManager)
   NS_INTERFACE_MAP_ENTRY(nsIDOMEventTarget)
   NS_INTERFACE_MAP_ENTRY(nsIDOM3EventTarget)
   NS_INTERFACE_MAP_ENTRY(nsIDOMEventReceiver)
NS_INTERFACE_MAP_END


nsVoidArray*
nsEventListenerManager::GetListenersByType(EventArrayType aType, 
                                           nsHashKey* aKey, PRBool aCreate)
{
  NS_ASSERTION(aType >= 0,"Negative EventListenerType?");
  //Look for existing listeners
  if (aType == eEventArrayType_Hash && aKey && (mManagerType & NS_ELM_HASH)) {
    if (mGenericListeners && mGenericListeners->Exists(aKey)) {
      nsVoidArray* listeners = NS_STATIC_CAST(nsVoidArray*, mGenericListeners->Get(aKey));
      return listeners;
    }
  }
  else if (mManagerType & NS_ELM_SINGLE) {
    if (mSingleListenerType == aType) {
      return mSingleListener;
    }
  }
  else if (mManagerType & NS_ELM_MULTI) {
    if (mMultiListeners) {
      PRInt32 index = aType;
      if (index < mMultiListeners->Count()) {
        nsVoidArray* listeners;
        listeners = NS_STATIC_CAST(nsVoidArray*, mMultiListeners->ElementAt(index));
        if (listeners) {
          return listeners;
        }
      }
    }
  }

  //If we've gotten here we didn't find anything.  See if we should create something.
  if (aCreate) {
    if (aType == eEventArrayType_Hash && aKey) {
      if (!mGenericListeners) {
        mGenericListeners = new nsHashtable();
        if (!mGenericListeners) {
          //out of memory
          return nsnull;
        }
      }
      NS_ASSERTION(!(mGenericListeners->Get(aKey)), "Found existing generic listeners, should be none");
      nsVoidArray* listeners;
      listeners = new nsAutoVoidArray();
      if (!listeners) {
        //out of memory
        return nsnull;
      }
      mGenericListeners->Put(aKey, listeners);
      mManagerType |= NS_ELM_HASH;
      return listeners;
    }
    else {
      if (mManagerType & NS_ELM_SINGLE) {
        //Change single type into multi, then add new listener with the code for the 
        //multi type below
        NS_ASSERTION(!mMultiListeners, "Found existing multi listener array, should be none");
        mMultiListeners = new nsAutoVoidArray();
        if (!mMultiListeners) {
          //out of memory
          return nsnull;
        }

        //Move single listener to multi array
        mMultiListeners->ReplaceElementAt((void*)mSingleListener, mSingleListenerType);
        mSingleListener = nsnull;

        mManagerType &= ~NS_ELM_SINGLE;
        mManagerType |= NS_ELM_MULTI;
        // we'll fall through into the multi case
      }

      if (mManagerType & NS_ELM_MULTI) {
        PRInt32 index = aType;
        if (index >= 0) {
          nsVoidArray* listeners;
          NS_ASSERTION(index >= mMultiListeners->Count() || !mMultiListeners->ElementAt(index), "Found existing listeners, should be none");
          listeners = new nsAutoVoidArray();
          if (!listeners) {
            //out of memory
            return nsnull;
          }
          mMultiListeners->ReplaceElementAt((void*)listeners, index);
          return listeners;
        }
      }
      else {
        //We had no pre-existing type.  This is our first non-hash listener.
        //Create the single listener type
        NS_ASSERTION(!mSingleListener, "Found existing single listener array, should be none");
        mSingleListener = new nsAutoVoidArray();
        if (!mSingleListener) {
          //out of memory
          return nsnull;
        }
        mSingleListenerType = aType;
        mManagerType |= NS_ELM_SINGLE;
        return mSingleListener;
      }
    }
  }

  return nsnull;
}

EventArrayType
nsEventListenerManager::GetTypeForIID(const nsIID& aIID)
{ 
  if (aIID.Equals(NS_GET_IID(nsIDOMMouseListener)))
      return eEventArrayType_Mouse;

  if (aIID.Equals(NS_GET_IID(nsIDOMMouseMotionListener)))
    return eEventArrayType_MouseMotion;

  if (aIID.Equals(NS_GET_IID(nsIDOMContextMenuListener)))
    return eEventArrayType_ContextMenu;

  if (aIID.Equals(NS_GET_IID(nsIDOMKeyListener)))
    return eEventArrayType_Key;

  if (aIID.Equals(NS_GET_IID(nsIDOMLoadListener)))
    return eEventArrayType_Load;

  if (aIID.Equals(NS_GET_IID(nsIDOMFocusListener)))
    return eEventArrayType_Focus;

  if (aIID.Equals(NS_GET_IID(nsIDOMFormListener)))
    return eEventArrayType_Form;

  if (aIID.Equals(NS_GET_IID(nsIDOMDragListener)))
    return eEventArrayType_Drag;

  if (aIID.Equals(NS_GET_IID(nsIDOMPaintListener)))
    return eEventArrayType_Paint;

  if (aIID.Equals(NS_GET_IID(nsIDOMTextListener)))
    return eEventArrayType_Text;

  if (aIID.Equals(NS_GET_IID(nsIDOMCompositionListener)))
    return eEventArrayType_Composition;

  if (aIID.Equals(NS_GET_IID(nsIDOMXULListener)))
    return eEventArrayType_XUL;

  if (aIID.Equals(NS_GET_IID(nsIDOMScrollListener)))
    return eEventArrayType_Scroll;

  if (aIID.Equals(NS_GET_IID(nsIDOMMutationListener)))
    return eEventArrayType_Mutation;

  if (aIID.Equals(NS_GET_IID(nsIDOMUIListener)))
    return eEventArrayType_DOMUI;

#ifdef MOZ_SVG
  if (aIID.Equals(NS_GET_IID(nsIDOMSVGListener)))
    return eEventArrayType_SVG;

  if (aIID.Equals(NS_GET_IID(nsIDOMSVGZoomListener)))
    return eEventArrayType_SVGZoom;
#endif // MOZ_SVG

  return eEventArrayType_None;
}

void
nsEventListenerManager::ReleaseListeners(nsVoidArray** aListeners)
{
  if (nsnull != *aListeners) {
    PRInt32 i, count = (*aListeners)->Count();
    nsListenerStruct *ls;
    for (i = 0; i < count; i++) {
      ls = (nsListenerStruct*)(*aListeners)->ElementAt(i);
      if (ls) {
        delete ls;
      }
    }
    delete *aListeners;
    *aListeners = nsnull;
  }
}

/**
* Sets events listeners of all types. 
* @param an event listener
*/

nsresult
nsEventListenerManager::AddEventListener(nsIDOMEventListener *aListener, 
                                         EventArrayType aType, 
                                         PRInt32 aSubType,
                                         nsHashKey* aKey,
                                         PRInt32 aFlags,
                                         nsIDOMEventGroup* aEvtGrp)
{
  NS_ENSURE_TRUE(aListener, NS_ERROR_FAILURE);

  nsVoidArray* listeners = GetListenersByType(aType, aKey, PR_TRUE);

  //We asked the GetListenersByType to create the array if it had to.  If it didn't
  //then we're out of memory (or a bug was added which passed in an unsupported
  //event type)
  if (!listeners) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  // For mutation listeners, we need to update the global bit on the DOM window.
  // Otherwise we won't actually fire the mutation event.
  if (aType == eEventArrayType_Mutation) {
    // Go from our target to the nearest enclosing DOM window.
    nsCOMPtr<nsPIDOMWindow> window;
    nsCOMPtr<nsIDocument> document;
    nsCOMPtr<nsINode> node(do_QueryInterface(mTarget));
    if (node)
      document = node->GetOwnerDoc();
    if (document)
      window = document->GetInnerWindow();
    else window = do_QueryInterface(mTarget);
    if (window) {
      NS_ASSERTION(window->IsInnerWindow(),
                   "Setting mutation listener bits on outer window?");
      window->SetMutationListeners(aSubType);
    }
  }
  
  PRBool isSame = PR_FALSE;
  PRUint16 group = 0;
  nsCOMPtr<nsIDOMEventGroup> sysGroup;
  GetSystemEventGroupLM(getter_AddRefs(sysGroup));
  if (sysGroup) {
    sysGroup->IsSameEventGroup(aEvtGrp, &isSame);
    if (isSame) {
      group = NS_EVENT_FLAG_SYSTEM_EVENT;
    }
  }

  PRBool found = PR_FALSE;
  nsListenerStruct* ls;

  for (PRInt32 i=0; i<listeners->Count(); i++) {
    ls = (nsListenerStruct*)listeners->ElementAt(i);
    nsRefPtr<nsIDOMEventListener> iListener = ls->mListener.Get();
    if (iListener == aListener && ls->mFlags == aFlags &&
        ls->mGroupFlags == group) {
      ls->mSubType |= aSubType;
      found = PR_TRUE;
      break;
    }
  }

  if (!found) {
    ls = new nsListenerStruct;
    if (!ls) {
      return NS_ERROR_OUT_OF_MEMORY;
    }

    nsCOMPtr<nsIDOMGCParticipant> participant = do_QueryInterface(mTarget);
    NS_ASSERTION(participant, "must implement nsIDOMGCParticipant");
    ls->mListener.Set(aListener, participant);
    ls->mFlags = aFlags;
    ls->mSubType = aSubType;
    ls->mHandlerIsString = 0;
    ls->mGroupFlags = group;
    listeners->AppendElement((void*)ls);
  }

  return NS_OK;
}

nsresult
nsEventListenerManager::RemoveEventListener(nsIDOMEventListener *aListener, 
                                            EventArrayType aType, 
                                            PRInt32 aSubType,
                                            nsHashKey* aKey,
                                            PRInt32 aFlags,
                                            nsIDOMEventGroup* aEvtGrp)
{
  nsVoidArray* listeners = GetListenersByType(aType, aKey, PR_FALSE);

  if (!listeners) {
    return NS_OK;
  }

  nsListenerStruct* ls;
  aFlags &= ~NS_PRIV_EVENT_UNTRUSTED_PERMITTED;

  for (PRInt32 i=0; i<listeners->Count(); i++) {
    ls = (nsListenerStruct*)listeners->ElementAt(i);
    nsRefPtr<nsIDOMEventListener> iListener = ls->mListener.Get();
    if (iListener == aListener &&
        (ls->mFlags & ~NS_PRIV_EVENT_UNTRUSTED_PERMITTED) == aFlags) {
      ls->mSubType &= ~aSubType;
      if (ls->mSubType == NS_EVENT_BITS_NONE) {
        listeners->RemoveElement((void*)ls);
        delete ls;
      }
      break;
    }
  }

  return NS_OK;
}

nsresult
nsEventListenerManager::AddEventListenerByIID(nsIDOMEventListener *aListener, 
                                              const nsIID& aIID,
                                              PRInt32 aFlags)
{
  AddEventListener(aListener, GetTypeForIID(aIID), NS_EVENT_BITS_NONE, nsnull,
                   aFlags, nsnull);
  return NS_OK;
}

NS_IMETHODIMP
nsEventListenerManager::RemoveEventListenerByIID(nsIDOMEventListener *aListener, 
                                                 const nsIID& aIID,
                                                 PRInt32 aFlags)
{
  RemoveEventListener(aListener, GetTypeForIID(aIID), NS_EVENT_BITS_NONE,
                      nsnull, aFlags, nsnull);
  return NS_OK;
}

nsresult
nsEventListenerManager::GetIdentifiersForType(nsIAtom* aType,
                                              EventArrayType* aArrayType,
                                              PRInt32* aFlags)
{
  if (aType == nsLayoutAtoms::onmousedown) {
    *aArrayType = eEventArrayType_Mouse;
    *aFlags = NS_EVENT_BITS_MOUSE_MOUSEDOWN;
  }
  else if (aType == nsLayoutAtoms::onmouseup) {
    *aArrayType = eEventArrayType_Mouse;
    *aFlags = NS_EVENT_BITS_MOUSE_MOUSEUP;
  }
  else if (aType == nsLayoutAtoms::onclick) {
    *aArrayType = eEventArrayType_Mouse;
    *aFlags = NS_EVENT_BITS_MOUSE_CLICK;
  }
  else if (aType == nsLayoutAtoms::ondblclick) {
    *aArrayType = eEventArrayType_Mouse;
    *aFlags = NS_EVENT_BITS_MOUSE_DBLCLICK;
  }
  else if (aType == nsLayoutAtoms::onmouseover) {
    *aArrayType = eEventArrayType_Mouse;
    *aFlags = NS_EVENT_BITS_MOUSE_MOUSEOVER;
  }
  else if (aType == nsLayoutAtoms::onmouseout) {
    *aArrayType = eEventArrayType_Mouse;
    *aFlags = NS_EVENT_BITS_MOUSE_MOUSEOUT;
  }
  else if (aType == nsLayoutAtoms::onkeydown) {
    *aArrayType = eEventArrayType_Key;
    *aFlags = NS_EVENT_BITS_KEY_KEYDOWN;
  }
  else if (aType == nsLayoutAtoms::onkeyup) {
    *aArrayType = eEventArrayType_Key;
    *aFlags = NS_EVENT_BITS_KEY_KEYUP;
  }
  else if (aType == nsLayoutAtoms::onkeypress) {
    *aArrayType = eEventArrayType_Key;
    *aFlags = NS_EVENT_BITS_KEY_KEYPRESS;
  }
  else if (aType == nsLayoutAtoms::onmousemove) {
    *aArrayType = eEventArrayType_MouseMotion;
    *aFlags = NS_EVENT_BITS_MOUSEMOTION_MOUSEMOVE;
  }
  else if (aType == nsLayoutAtoms::oncontextmenu) {
    *aArrayType = eEventArrayType_ContextMenu;
    *aFlags = NS_EVENT_BITS_CONTEXTMENU;
  }
  else if (aType == nsLayoutAtoms::onfocus) {
    *aArrayType = eEventArrayType_Focus;
    *aFlags = NS_EVENT_BITS_FOCUS_FOCUS;
  }
  else if (aType == nsLayoutAtoms::onblur) {
    *aArrayType = eEventArrayType_Focus;
    *aFlags = NS_EVENT_BITS_FOCUS_BLUR;
  }
  else if (aType == nsLayoutAtoms::onsubmit) {
    *aArrayType = eEventArrayType_Form;
    *aFlags = NS_EVENT_BITS_FORM_SUBMIT;
  }
  else if (aType == nsLayoutAtoms::onreset) {
    *aArrayType = eEventArrayType_Form;
    *aFlags = NS_EVENT_BITS_FORM_RESET;
  }
  else if (aType == nsLayoutAtoms::onchange) {
    *aArrayType = eEventArrayType_Form;
    *aFlags = NS_EVENT_BITS_FORM_CHANGE;
  }
  else if (aType == nsLayoutAtoms::onselect) {
    *aArrayType = eEventArrayType_Form;
    *aFlags = NS_EVENT_BITS_FORM_SELECT;
  }
  else if (aType == nsLayoutAtoms::oninput) {
    *aArrayType = eEventArrayType_Form;
    *aFlags = NS_EVENT_BITS_FORM_INPUT;
  }
  else if (aType == nsLayoutAtoms::onload) {
    *aArrayType = eEventArrayType_Load;
    *aFlags = NS_EVENT_BITS_LOAD_LOAD;
  }
  else if (aType == nsLayoutAtoms::onbeforeunload) {
    *aArrayType = eEventArrayType_Load;
    *aFlags = NS_EVENT_BITS_LOAD_BEFORE_UNLOAD;
  }
  else if (aType == nsLayoutAtoms::onunload) {
    *aArrayType = eEventArrayType_Load;
    *aFlags = NS_EVENT_BITS_LOAD_UNLOAD;
  }
  else if (aType == nsLayoutAtoms::onabort) {
    *aArrayType = eEventArrayType_Load;
    *aFlags = NS_EVENT_BITS_LOAD_ABORT;
  }
  else if (aType == nsLayoutAtoms::onerror) {
    *aArrayType = eEventArrayType_Load;
    *aFlags = NS_EVENT_BITS_LOAD_ERROR;
  }
  else if (aType == nsLayoutAtoms::onpaint) {
    *aArrayType = eEventArrayType_Paint;
    *aFlags = NS_EVENT_BITS_PAINT_PAINT;
  }
  else if (aType == nsLayoutAtoms::onresize) {
    *aArrayType = eEventArrayType_Paint;
    *aFlags = NS_EVENT_BITS_PAINT_RESIZE;
  }
  else if (aType == nsLayoutAtoms::onscroll) {
    *aArrayType = eEventArrayType_Paint;
    *aFlags = NS_EVENT_BITS_PAINT_SCROLL;
  } // extened this to handle IME related events
  else if (aType == nsLayoutAtoms::onpopupshowing) {
    *aArrayType = eEventArrayType_XUL; 
    *aFlags = NS_EVENT_BITS_XUL_POPUP_SHOWING;
  }
  else if (aType == nsLayoutAtoms::onpopupshown) {
    *aArrayType = eEventArrayType_XUL; 
    *aFlags = NS_EVENT_BITS_XUL_POPUP_SHOWN;
  }
  else if (aType == nsLayoutAtoms::onpopuphiding) {
    *aArrayType = eEventArrayType_XUL; 
    *aFlags = NS_EVENT_BITS_XUL_POPUP_HIDING;
  }
  else if (aType == nsLayoutAtoms::onpopuphidden) {
    *aArrayType = eEventArrayType_XUL; 
    *aFlags = NS_EVENT_BITS_XUL_POPUP_HIDDEN;
  }
  else if (aType == nsLayoutAtoms::onclose) {
    *aArrayType = eEventArrayType_XUL; 
    *aFlags = NS_EVENT_BITS_XUL_CLOSE;
  }
  else if (aType == nsLayoutAtoms::oncommand) {
    *aArrayType = eEventArrayType_XUL; 
    *aFlags = NS_EVENT_BITS_XUL_COMMAND;
  }
  else if (aType == nsLayoutAtoms::onbroadcast) {
    *aArrayType = eEventArrayType_XUL;
    *aFlags = NS_EVENT_BITS_XUL_BROADCAST;
  }
  else if (aType == nsLayoutAtoms::oncommandupdate) {
    *aArrayType = eEventArrayType_XUL;
    *aFlags = NS_EVENT_BITS_XUL_COMMAND_UPDATE;
  }
  else if (aType == nsLayoutAtoms::onoverflow) {
    *aArrayType = eEventArrayType_Scroll;
    *aFlags = NS_EVENT_BITS_SCROLLPORT_OVERFLOW;
  }
  else if (aType == nsLayoutAtoms::onunderflow) {
    *aArrayType = eEventArrayType_Scroll;
    *aFlags = NS_EVENT_BITS_SCROLLPORT_UNDERFLOW;
  }
  else if (aType == nsLayoutAtoms::onoverflowchanged) {
    *aArrayType = eEventArrayType_Scroll;
    *aFlags = NS_EVENT_BITS_SCROLLPORT_OVERFLOWCHANGED;
  }
  else if (aType == nsLayoutAtoms::ondragenter) {
    *aArrayType = eEventArrayType_Drag;
    *aFlags = NS_EVENT_BITS_DRAG_ENTER;
  }
  else if (aType == nsLayoutAtoms::ondragover) {
    *aArrayType = eEventArrayType_Drag; 
    *aFlags = NS_EVENT_BITS_DRAG_OVER;
  }
  else if (aType == nsLayoutAtoms::ondragexit) {
    *aArrayType = eEventArrayType_Drag; 
    *aFlags = NS_EVENT_BITS_DRAG_EXIT;
  }
  else if (aType == nsLayoutAtoms::ondragdrop) {
    *aArrayType = eEventArrayType_Drag; 
    *aFlags = NS_EVENT_BITS_DRAG_DROP;
  }
  else if (aType == nsLayoutAtoms::ondraggesture) {
    *aArrayType = eEventArrayType_Drag; 
    *aFlags = NS_EVENT_BITS_DRAG_GESTURE;
  }
  else if (aType == nsLayoutAtoms::onDOMSubtreeModified) {
    *aArrayType = eEventArrayType_Mutation;
    *aFlags = NS_EVENT_BITS_MUTATION_SUBTREEMODIFIED;
  }
  else if (aType == nsLayoutAtoms::onDOMNodeInserted) {
    *aArrayType = eEventArrayType_Mutation;
    *aFlags = NS_EVENT_BITS_MUTATION_NODEINSERTED;
  }
  else if (aType == nsLayoutAtoms::onDOMNodeRemoved) {
    *aArrayType = eEventArrayType_Mutation;
    *aFlags = NS_EVENT_BITS_MUTATION_NODEREMOVED;
  }
  else if (aType == nsLayoutAtoms::onDOMNodeInsertedIntoDocument) {
    *aArrayType = eEventArrayType_Mutation;
    *aFlags = NS_EVENT_BITS_MUTATION_NODEINSERTEDINTODOCUMENT;
  }
  else if (aType == nsLayoutAtoms::onDOMNodeRemovedFromDocument) {
    *aArrayType = eEventArrayType_Mutation;
    *aFlags = NS_EVENT_BITS_MUTATION_NODEREMOVEDFROMDOCUMENT;
  }
  else if (aType == nsLayoutAtoms::onDOMAttrModified) {
    *aArrayType = eEventArrayType_Mutation;
    *aFlags = NS_EVENT_BITS_MUTATION_ATTRMODIFIED;
  }
  else if (aType == nsLayoutAtoms::onDOMCharacterDataModified) {
    *aArrayType = eEventArrayType_Mutation;
    *aFlags = NS_EVENT_BITS_MUTATION_CHARACTERDATAMODIFIED;
  }
  else if (aType == nsLayoutAtoms::onDOMActivate) {
    *aArrayType = eEventArrayType_DOMUI;
    *aFlags = NS_EVENT_BITS_UI_ACTIVATE;
  }
  else if (aType == nsLayoutAtoms::onDOMFocusIn) {
    *aArrayType = eEventArrayType_DOMUI;
    *aFlags = NS_EVENT_BITS_UI_FOCUSIN;
  }
  else if (aType == nsLayoutAtoms::onDOMFocusOut) {
    *aArrayType = eEventArrayType_DOMUI;
    *aFlags = NS_EVENT_BITS_UI_FOCUSOUT;
  }
  else if (aType == nsLayoutAtoms::oncompositionstart) {
    *aArrayType = eEventArrayType_Composition;
    *aFlags = NS_EVENT_BITS_COMPOSITION_START;
  }
  else if (aType == nsLayoutAtoms::oncompositionend) {
    *aArrayType = eEventArrayType_Composition;
    *aFlags = NS_EVENT_BITS_COMPOSITION_END;
  }
  else if (aType == nsLayoutAtoms::ontext) {
    *aArrayType = eEventArrayType_Text;
    *aFlags = NS_EVENT_BITS_TEXT_TEXT;
  }
  else if (aType == nsLayoutAtoms::onpageshow) {
    *aArrayType = eEventArrayType_PageTransition;
    *aFlags = NS_EVENT_BITS_PAGETRANSITION_SHOW;
  }
  else if (aType == nsLayoutAtoms::onpagehide) {
    *aArrayType = eEventArrayType_PageTransition;
    *aFlags = NS_EVENT_BITS_PAGETRANSITION_HIDE;
  }
#ifdef MOZ_SVG
  else if (aType == nsLayoutAtoms::onSVGLoad) {
    *aArrayType = eEventArrayType_SVG;
    *aFlags = NS_EVENT_BITS_SVG_LOAD;
  }
  else if (aType == nsLayoutAtoms::onSVGUnload) {
    *aArrayType = eEventArrayType_SVG;
    *aFlags = NS_EVENT_BITS_SVG_UNLOAD;
  }
  else if (aType == nsLayoutAtoms::onSVGAbort) {
    *aArrayType = eEventArrayType_SVG;
    *aFlags = NS_EVENT_BITS_SVG_ABORT;
  }
  else if (aType == nsLayoutAtoms::onSVGError) {
    *aArrayType = eEventArrayType_SVG;
    *aFlags = NS_EVENT_BITS_SVG_ERROR;
  }
  else if (aType == nsLayoutAtoms::onSVGResize) {
    *aArrayType = eEventArrayType_SVG;
    *aFlags = NS_EVENT_BITS_SVG_RESIZE;
  }
  else if (aType == nsLayoutAtoms::onSVGScroll) {
    *aArrayType = eEventArrayType_SVG;
    *aFlags = NS_EVENT_BITS_SVG_SCROLL;
  }
  else if (aType == nsLayoutAtoms::onSVGZoom) {
    *aArrayType = eEventArrayType_SVGZoom;
    *aFlags = NS_EVENT_BITS_SVGZOOM_ZOOM;
  }
#endif // MOZ_SVG
  else {
    return NS_ERROR_FAILURE;
  }
  return NS_OK;
}

NS_IMETHODIMP
nsEventListenerManager::AddEventListenerByType(nsIDOMEventListener *aListener, 
                                               const nsAString& aType,
                                               PRInt32 aFlags,
                                               nsIDOMEventGroup* aEvtGrp)
{
  PRInt32 subType;
  EventArrayType arrayType;
  nsCOMPtr<nsIAtom> atom = do_GetAtom(NS_LITERAL_STRING("on") + aType);

  if (NS_OK == GetIdentifiersForType(atom, &arrayType, &subType)) {
    AddEventListener(aListener, arrayType, subType, nsnull, aFlags, aEvtGrp);
  }
  else {
    const nsPromiseFlatString& flatString = PromiseFlatString(aType); 
    nsStringKey key(flatString);
    AddEventListener(aListener, eEventArrayType_Hash, NS_EVENT_BITS_NONE, &key, aFlags, aEvtGrp);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsEventListenerManager::RemoveEventListenerByType(nsIDOMEventListener *aListener, 
                                                  const nsAString& aType,
                                                  PRInt32 aFlags,
                                                  nsIDOMEventGroup* aEvtGrp)
{
  PRInt32 subType;
  EventArrayType arrayType;
  nsCOMPtr<nsIAtom> atom = do_GetAtom(NS_LITERAL_STRING("on") + aType);

  if (NS_OK == GetIdentifiersForType(atom, &arrayType, &subType)) {
    RemoveEventListener(aListener, arrayType, subType, nsnull, aFlags, aEvtGrp);
  }
  else {
    const nsPromiseFlatString& flatString = PromiseFlatString(aType); 
    nsStringKey key(flatString);
    RemoveEventListener(aListener, eEventArrayType_Hash, NS_EVENT_BITS_NONE, &key, aFlags, aEvtGrp);
  }

  return NS_OK;
}

nsListenerStruct*
nsEventListenerManager::FindJSEventListener(EventArrayType aType)
{
  nsVoidArray *listeners = GetListenersByType(aType, nsnull, PR_FALSE);
  if (listeners) {
    // Run through the listeners for this type and see if a script
    // listener is registered
    nsListenerStruct *ls;
    for (PRInt32 i=0; i < listeners->Count(); i++) {
      ls = (nsListenerStruct*)listeners->ElementAt(i);
      if (ls->mFlags & NS_PRIV_EVENT_FLAG_SCRIPT) {
        return ls;
      }
    }
  }

  return nsnull;
}

nsresult
nsEventListenerManager::SetJSEventListener(nsIScriptContext *aContext,
                                           void *aScopeObject,
                                           nsISupports *aObject,
                                           nsIAtom* aName,
                                           PRBool aIsString,
                                           PRBool aPermitUntrustedEvents)
{
  nsresult rv = NS_OK;
  nsListenerStruct *ls;
  PRInt32 flags;
  EventArrayType arrayType;

  NS_ENSURE_SUCCESS(GetIdentifiersForType(aName, &arrayType, &flags),
                    NS_ERROR_FAILURE);

  ls = FindJSEventListener(arrayType);

  if (nsnull == ls) {
    // If we didn't find a script listener or no listeners existed
    // create and add a new one.
    nsCOMPtr<nsIDOMEventListener> scriptListener;
    rv = NS_NewJSEventListener(aContext, aScopeObject, aObject,
                               getter_AddRefs(scriptListener));
    if (NS_SUCCEEDED(rv)) {
      AddEventListener(scriptListener, arrayType, NS_EVENT_BITS_NONE, nsnull,
                       NS_EVENT_FLAG_BUBBLE | NS_PRIV_EVENT_FLAG_SCRIPT, nsnull);

      ls = FindJSEventListener(arrayType);
    }
  }

  if (NS_SUCCEEDED(rv) && ls) {
    // Set flag to indicate possible need for compilation later
    if (aIsString) {
      ls->mHandlerIsString |= flags;
    }
    else {
      ls->mHandlerIsString &= ~flags;
    }

    // Set subtype flags based on event
    ls->mSubType |= flags;

    if (aPermitUntrustedEvents) {
      ls->mFlags |= NS_PRIV_EVENT_UNTRUSTED_PERMITTED;
    }
  }

  return rv;
}

NS_IMETHODIMP
nsEventListenerManager::AddScriptEventListener(nsISupports *aObject,
                                               nsIAtom *aName,
                                               const nsAString& aBody,
                                               PRUint32 aLanguage,
                                               PRBool aDeferCompilation,
                                               PRBool aPermitUntrustedEvents)
{
  NS_PRECONDITION(aLanguage != nsIProgrammingLanguage::UNKNOWN,
                  "Must know the language for the script event listener");
  nsIScriptContext *context = nsnull;

  // |aPermitUntrustedEvents| is set to False for chrome - events
  // *generated* from an unknown source are not allowed.
  // However, for script languages with no 'sandbox', we want to reject
  // such scripts based on the source of their code, not just the source
  // of the event.
  if (aPermitUntrustedEvents && 
      aLanguage != nsIProgrammingLanguage::JAVASCRIPT) {
    NS_WARNING("Discarding non-JS event listener from untrusted source");
    return NS_ERROR_FAILURE;
  }

  nsCOMPtr<nsINode> node(do_QueryInterface(aObject));

  nsCOMPtr<nsIDocument> doc;

  nsISupports *objiSupp = aObject;
  nsCOMPtr<nsIScriptGlobalObject> global;

  if (node) {
    // Try to get context from doc
    doc = node->GetOwnerDoc();
    if (doc)
      global = doc->GetScriptGlobalObject();
    if (global) {
      // This might be the first reference to this language in the global
      // We must init the language before we attempt to fetch its context.
      if (NS_FAILED(global->EnsureScriptEnvironment(aLanguage))) {
        NS_WARNING("Failed to setup script environment for this language");
        // but fall through and let the inevitable failure below handle it.
      }

      context = global->GetScriptContext(aLanguage);
      NS_ASSERTION(context, "Failed to get language context from global");
    }
  } else {
    nsCOMPtr<nsPIDOMWindow> win(do_QueryInterface(aObject));
    if (win) {
      NS_ASSERTION(win->IsInnerWindow(),
                   "Event listener added to outer window!");

      nsCOMPtr<nsIDOMDocument> domdoc;
      win->GetDocument(getter_AddRefs(domdoc));
      doc = do_QueryInterface(domdoc);
      global = do_QueryInterface(win);
    } else {
      global = do_QueryInterface(aObject);
    }
    if (global) {
      // As above - ensure the global is setup for the language.
      if (NS_FAILED(global->EnsureScriptEnvironment(aLanguage))) {
        NS_WARNING("Failed to setup script environment for this language");
        // but fall through and let the inevitable failure below handle it.
      }
      context = global->GetScriptContext(aLanguage);
    }
  }

  if (!context) {
    NS_ASSERTION(aLanguage == nsIProgrammingLanguage::JAVASCRIPT,
                 "Need a multi-language stack?!?!?");
    // I've only ever seen the above fire when something else has gone wrong -
    // in normal processing, languages other than JS should not be able to get
    // here, as these languages are only called via nsIScriptContext

    // OTOH, maybe using JS will do here - all we need is the global - try and
    // keep going...
    JSContext* cx = nsnull;
    // Get JSContext from stack, or use the safe context (and hidden
    // window global) if no JS is running.
    nsCOMPtr<nsIThreadJSContextStack> stack =
      do_GetService("@mozilla.org/js/xpc/ContextStack;1");
    NS_ENSURE_TRUE(stack, NS_ERROR_FAILURE);
    NS_ENSURE_SUCCESS(stack->Peek(&cx), NS_ERROR_FAILURE);

    if (!cx) {
      stack->GetSafeJSContext(&cx);
      NS_ENSURE_TRUE(cx, NS_ERROR_FAILURE);
    }

    context = nsJSUtils::GetDynamicScriptContext(cx);
    NS_ENSURE_TRUE(context, NS_ERROR_FAILURE);

    global = context->GetGlobalObject();
    // but if the language is *not* js , we now have the wrong context.
    context = global->GetScriptContext(aLanguage);
    NS_ENSURE_TRUE(context, NS_ERROR_FAILURE);
  }
  if (!global) {
    NS_ERROR("Context reachable, but no scope reachable in "
             "AddScriptEventListener()!");

    return NS_ERROR_NOT_AVAILABLE;
  }

  void *scope = global->GetScriptGlobal(aLanguage);
  nsresult rv;

  if (!aDeferCompilation) {
    nsCOMPtr<nsIScriptEventHandlerOwner> handlerOwner =
      do_QueryInterface(aObject);

    nsScriptObjectHolder handler(context);
    PRBool done = PR_FALSE;

    if (handlerOwner) {
      rv = handlerOwner->GetCompiledEventHandler(aName, handler);
      if (NS_SUCCEEDED(rv) && handler) {
        rv = context->BindCompiledEventHandler(aObject, scope, aName, handler);
        if (NS_FAILED(rv))
          return rv;
        done = PR_TRUE;
      }
    }

    if (!done) {
      PRUint32 lineNo = 0;
      nsCAutoString url (NS_LITERAL_CSTRING("-moz-evil:lying-event-listener"));
      if (doc) {
        nsIURI *uri = doc->GetDocumentURI();
        if (uri) {
          uri->GetSpec(url);
          lineNo = 1;
        }
      }

      if (handlerOwner) {
        // Always let the handler owner compile the event handler, as
        // it may want to use a special context or scope object.
        rv = handlerOwner->CompileEventHandler(context, aObject, aName,
                                               aBody, url.get(), lineNo, handler);
      }
      else {
        PRInt32 nameSpace = kNameSpaceID_Unknown;
        if (node && node->IsNodeOfType(nsINode::eCONTENT)) {
          nsIContent* content =
            NS_STATIC_CAST(nsIContent*,
                           NS_STATIC_CAST(nsINode*, node.get()));
          nameSpace = content->GetNameSpaceID();
        }
        else if (doc) {
          nsCOMPtr<nsIContent> root = doc->GetRootContent();
          if (root)
            nameSpace = root->GetNameSpaceID();
        }
        PRUint32 argCount;
        const char **argNames;
        nsContentUtils::GetEventArgNames(nameSpace, aName, &argCount,
                                         &argNames);

        rv = context->CompileEventHandler(aName, argCount, argNames,
                                          aBody,
                                          url.get(), lineNo,
                                          handler);
        NS_ENSURE_SUCCESS(rv, rv);
        // And bind it.
        rv = context->BindCompiledEventHandler(aObject, scope,
                                               aName, handler);
      }
      if (NS_FAILED(rv)) return rv;
    }
  }

  return SetJSEventListener(context, scope, objiSupp, aName, aDeferCompilation,
                            aPermitUntrustedEvents);
}

nsresult
nsEventListenerManager::RemoveScriptEventListener(nsIAtom *aName)
{
  nsresult result = NS_OK;
  nsListenerStruct *ls;
  PRInt32 flags;
  EventArrayType arrayType;

  NS_ENSURE_SUCCESS(GetIdentifiersForType(aName, &arrayType, &flags),
                    NS_ERROR_FAILURE);
  ls = FindJSEventListener(arrayType);

  if (ls) {
    ls->mSubType &= ~flags;
    if (ls->mSubType == NS_EVENT_BITS_NONE) {
      //Get the listeners array so we can remove ourselves from it
      nsVoidArray* listeners;
      listeners = GetListenersByType(arrayType, nsnull, PR_FALSE);
      NS_ENSURE_TRUE(listeners, NS_ERROR_FAILURE);
      listeners->RemoveElement((void*)ls);
      delete ls;
    }
  }

  return result;
}

jsval
nsEventListenerManager::sAddListenerID = JSVAL_VOID;

NS_IMETHODIMP
nsEventListenerManager::RegisterScriptEventListener(nsIScriptContext *aContext,
                                                    void *aScope,
                                                    nsISupports *aObject, 
                                                    nsIAtom *aName)
{
  // Check that we have access to set an event listener. Prevents
  // snooping attacks across domains by setting onkeypress handlers,
  // for instance.
  // You'd think it'd work just to get the JSContext from aContext,
  // but that's actually the JSContext whose private object parents
  // the object in aObject.
  nsresult rv;
  nsCOMPtr<nsIJSContextStack> stack =
    do_GetService("@mozilla.org/js/xpc/ContextStack;1", &rv);
  if (NS_FAILED(rv))
    return rv;
  JSContext *cx;
  if (NS_FAILED(rv = stack->Peek(&cx)))
    return rv;

  if (cx) {
    if (sAddListenerID == JSVAL_VOID) {
      JSAutoRequest ar(cx);
      sAddListenerID =
        STRING_TO_JSVAL(::JS_InternString(cx, "addEventListener"));
    }

    if (aContext->GetScriptTypeID() == nsIProgrammingLanguage::JAVASCRIPT) {
        nsCOMPtr<nsIXPConnectJSObjectHolder> holder;
        rv = nsContentUtils::XPConnect()->
          WrapNative(cx, (JSObject *)aScope, aObject, NS_GET_IID(nsISupports),
                     getter_AddRefs(holder));
        NS_ENSURE_SUCCESS(rv, rv);
        JSObject *jsobj = nsnull;
      
        rv = holder->GetJSObject(&jsobj);
        NS_ENSURE_SUCCESS(rv, rv);

        rv = nsContentUtils::GetSecurityManager()->
          CheckPropertyAccess(cx, jsobj,
                              "EventTarget",
                              sAddListenerID,
                              nsIXPCSecurityManager::ACCESS_SET_PROPERTY);
        if (NS_FAILED(rv)) {
          // XXX set pending exception on the native call context?
          return rv;
        }
    } else {
        NS_WARNING("Skipping CheckPropertyAccess for non JS language");
    }
        
  }

  // Untrusted events are always permitted for non-chrome script
  // handlers.
  return SetJSEventListener(aContext, aScope, aObject, aName,
                            PR_FALSE, !nsContentUtils::IsCallerChrome());
}

nsresult
nsEventListenerManager::CompileScriptEventListener(nsIScriptContext *aContext, 
                                                   void *aScope,
                                                   nsISupports *aObject, 
                                                   nsIAtom *aName,
                                                   PRBool *aDidCompile)
{
  nsresult rv = NS_OK;
  nsListenerStruct *ls;
  PRInt32 subType;
  EventArrayType arrayType;

  *aDidCompile = PR_FALSE;

  rv = GetIdentifiersForType(aName, &arrayType, &subType);
  NS_ENSURE_SUCCESS(rv, rv);

  ls = FindJSEventListener(arrayType);

  if (!ls) {
    //nothing to compile
    return NS_OK;
  }

  if (ls->mHandlerIsString & subType) {
    rv = CompileEventHandlerInternal(aContext, aScope, aObject, aName,
                                     ls, /*XXX fixme*/nsnull, subType);
  }

  // Set *aDidCompile to true even if we didn't really compile
  // anything right now, if we get here it means that this event
  // handler has been compiled at some point, that's good enough for
  // us.

  *aDidCompile = PR_TRUE;

  return rv;
}

nsresult
nsEventListenerManager::CompileEventHandlerInternal(nsIScriptContext *aContext,
                                                    void *aScope,
                                                    nsISupports *aObject,
                                                    nsIAtom *aName,
                                                    nsListenerStruct *aListenerStruct,
                                                    nsISupports* aCurrentTarget,
                                                    PRUint32 aSubType)
{
  nsresult result = NS_OK;

  nsCOMPtr<nsIScriptEventHandlerOwner> handlerOwner =
    do_QueryInterface(aObject);
  nsScriptObjectHolder handler(aContext);

  if (handlerOwner) {
    result = handlerOwner->GetCompiledEventHandler(aName,
                                                   handler);
    if (NS_SUCCEEDED(result) && handler) {
      // XXXmarkh - why do we bind here, but not after compilation below?
      result = aContext->BindCompiledEventHandler(aObject, aScope, aName, handler);
      aListenerStruct->mHandlerIsString &= ~aSubType;
    }
  }

  if (aListenerStruct->mHandlerIsString & aSubType) {
    // This should never happen for anything but content
    // XXX I don't like that we have to reference content
    // from here. The alternative is to store the event handler
    // string on the JS object itself.
    nsCOMPtr<nsIContent> content = do_QueryInterface(aObject);
    NS_ASSERTION(content, "only content should have event handler attributes");
    if (content) {
      nsAutoString handlerBody;
      nsIAtom* attrName = aName;
#ifdef MOZ_SVG
      if (aName == nsLayoutAtoms::onSVGLoad)
        attrName = nsSVGAtoms::onload;
      else if (aName == nsLayoutAtoms::onSVGUnload)
        attrName = nsSVGAtoms::onunload;
      else if (aName == nsLayoutAtoms::onSVGAbort)
        attrName = nsSVGAtoms::onabort;
      else if (aName == nsLayoutAtoms::onSVGError)
        attrName = nsSVGAtoms::onerror;
      else if (aName == nsLayoutAtoms::onSVGResize)
        attrName = nsSVGAtoms::onresize;
      else if (aName == nsLayoutAtoms::onSVGScroll)
        attrName = nsSVGAtoms::onscroll;
      else if (aName == nsLayoutAtoms::onSVGZoom)
        attrName = nsSVGAtoms::onzoom;
#endif // MOZ_SVG

      content->GetAttr(kNameSpaceID_None, attrName, handlerBody);

      PRUint32 lineNo = 0;
      nsCAutoString url (NS_LITERAL_CSTRING("javascript:alert('TODO: FIXME')"));
      nsIDocument* doc = nsnull;
      nsCOMPtr<nsINode> node = do_QueryInterface(aCurrentTarget);
      if (node) {
        doc = node->GetOwnerDoc();
      }
      if (doc) {
        nsIURI *uri = doc->GetDocumentURI();
        if (uri) {
          uri->GetSpec(url);
          lineNo = 1;
        }
      }

      if (handlerOwner) {
        // Always let the handler owner compile the event
        // handler, as it may want to use a special
        // context or scope object.
        result = handlerOwner->CompileEventHandler(aContext, aObject, aName,
                                                   handlerBody,
                                                   url.get(), lineNo,
                                                   handler);
      }
      else {
        PRUint32 argCount;
        const char **argNames;
        nsContentUtils::GetEventArgNames(content->GetNameSpaceID(), aName,
                                         &argCount, &argNames);

        result = aContext->CompileEventHandler(aName,
                                               argCount, argNames,
                                               handlerBody,
                                               url.get(), lineNo,
                                               handler);
        NS_ENSURE_SUCCESS(result, result);
        // And bind it.
        result = aContext->BindCompiledEventHandler(aObject, aScope,
                                                    aName, handler);
        NS_ENSURE_SUCCESS(result, result);
      }

      if (NS_SUCCEEDED(result)) {
        aListenerStruct->mHandlerIsString &= ~aSubType;
      }
    }
  }

  return result;
}

nsresult
nsEventListenerManager::HandleEventSubType(nsListenerStruct* aListenerStruct,
                                           nsIDOMEventListener* aListener,
                                           nsIDOMEvent* aDOMEvent,
                                           nsISupports* aCurrentTarget,
                                           PRUint32 aSubType,
                                           PRUint32 aPhaseFlags)
{
  nsresult result = NS_OK;

  // If this is a script handler and we haven't yet
  // compiled the event handler itself
  if ((aListenerStruct->mFlags & NS_PRIV_EVENT_FLAG_SCRIPT) &&
      (aListenerStruct->mHandlerIsString & aSubType)) {
    nsCOMPtr<nsIJSEventListener> jslistener = do_QueryInterface(aListener);
    if (jslistener) {
      nsAutoString eventString;
      if (NS_SUCCEEDED(aDOMEvent->GetType(eventString))) {
        nsCOMPtr<nsIAtom> atom = do_GetAtom(NS_LITERAL_STRING("on") + eventString);

        result = CompileEventHandlerInternal(jslistener->GetEventContext(),
                                             jslistener->GetEventScope(),
                                             jslistener->GetEventTarget(),
                                             atom, aListenerStruct,
                                             aCurrentTarget,
                                             aSubType);
      }
    }
  }

  // nsCxPusher will automatically push and pop the current cx onto the
  // context stack
  nsCxPusher pusher(aCurrentTarget);

  if (NS_SUCCEEDED(result)) {
    // nsIDOMEvent::currentTarget is set in nsEventDispatcher.
    result = aListener->HandleEvent(aDOMEvent);
  }

  return result;
}

/**
* Causes a check for event listeners and processing by them if they exist.
* @param an event listener
*/

nsresult
nsEventListenerManager::HandleEvent(nsPresContext* aPresContext,
                                    nsEvent* aEvent, nsIDOMEvent** aDOMEvent,
                                    nsISupports* aCurrentTarget,
                                    PRUint32 aFlags,
                                    nsEventStatus* aEventStatus)
{
  NS_ENSURE_ARG_POINTER(aEventStatus);
  nsresult ret = NS_OK;

  if (aEvent->flags & NS_EVENT_FLAG_STOP_DISPATCH) {
    return ret;
  }

  //Set the value of the internal PreventDefault flag properly based on aEventStatus
  if (*aEventStatus == nsEventStatus_eConsumeNoDefault) {
    aEvent->flags |= NS_EVENT_FLAG_NO_DEFAULT;
  }
  PRUint16 currentGroup = aFlags & NS_EVENT_FLAG_SYSTEM_EVENT;

  /* Without this addref, certain events, notably ones bound to
     keys which cause window deletion, can destroy this object
     before we're ready. */
  nsCOMPtr<nsIEventListenerManager> kungFuDeathGrip(this);
  nsVoidArray *listeners = nsnull;

  if (aEvent->message == NS_CONTEXTMENU || aEvent->message == NS_CONTEXTMENU_KEY) {
    ret = FixContextMenuEvent(aPresContext, aCurrentTarget, aEvent, aDOMEvent);
    if (NS_FAILED(ret)) {
      NS_WARNING("failed to fix context menu event target");
      ret = NS_OK;
    }
  }


  const EventTypeData* typeData = nsnull;
  const EventDispatchData* dispData = nsnull;

  if (aEvent->message == NS_USER_DEFINED_EVENT) {
    listeners = GetListenersByType(eEventArrayType_Hash, aEvent->userType, PR_FALSE);
  } else {
    for (PRInt32 i = 0; i < eEventArrayType_Hash; ++i) {
      typeData = &sEventTypes[i];
      for (PRInt32 j = 0; j < typeData->numEvents; ++j) {
        dispData = &(typeData->events[j]);
        if (aEvent->message == dispData->message) {
          listeners = GetListenersByType((EventArrayType)i, nsnull, PR_FALSE);
          goto found;
        }
      }
    }
  }

 found:
  if (listeners) {
    if (!*aDOMEvent) {
      ret = nsEventDispatcher::CreateEvent(aPresContext, aEvent,
                                           EmptyString(), aDOMEvent);
    }

    if (NS_SUCCEEDED(ret)) {
      PRInt32 count = listeners->Count();
      nsVoidArray originalListeners(count);
      originalListeners = *listeners;

      nsAutoPopupStatePusher popupStatePusher(nsDOMEvent::GetEventPopupControlState(aEvent));

      for (PRInt32 k = 0; !mListenersRemoved && listeners && k < count; ++k) {
        nsListenerStruct* ls = NS_STATIC_CAST(nsListenerStruct*, originalListeners.FastElementAt(k));
        // Don't fire the listener if it's been removed

        if (listeners->IndexOf(ls) != -1 && ls->mFlags & aFlags &&
            ls->mGroupFlags == currentGroup &&
            (NS_IS_TRUSTED_EVENT(aEvent) ||
             ls->mFlags & NS_PRIV_EVENT_UNTRUSTED_PERMITTED)) {
          nsRefPtr<nsIDOMEventListener> eventListener = ls->mListener.Get();
          NS_ASSERTION(eventListener, "listener wasn't preserved properly");
          if (eventListener) {
            // Try the type-specific listener interface
            PRBool hasInterface = PR_FALSE;
            if (typeData)
              DispatchToInterface(*aDOMEvent, eventListener,
                                  dispData->method, *typeData->iid,
                                  &hasInterface);

            // If it doesn't implement that, call the generic HandleEvent()
            if (!hasInterface && (ls->mSubType == NS_EVENT_BITS_NONE ||
                                  ls->mSubType & dispData->bits)) {
              HandleEventSubType(ls, eventListener, *aDOMEvent, aCurrentTarget,
                                 dispData ? dispData->bits : NS_EVENT_BITS_NONE,
                                 aFlags);
            }
          }
        }
      }
    }
  }

  if (aEvent->flags & NS_EVENT_FLAG_NO_DEFAULT) {
    *aEventStatus = nsEventStatus_eConsumeNoDefault;
  }

  return NS_OK;
}

/**
* Creates a DOM event
*/

NS_IMETHODIMP
nsEventListenerManager::CreateEvent(nsPresContext* aPresContext,
                                    nsEvent* aEvent,
                                    const nsAString& aEventType,
                                    nsIDOMEvent** aDOMEvent)
{
  return nsEventDispatcher::CreateEvent(aPresContext, aEvent,
                                        aEventType, aDOMEvent);
}

NS_IMETHODIMP
nsEventListenerManager::Disconnect()
{
  mTarget = nsnull;

  // Bug 323807: nsDOMClassInfo::PreserveWrapper (and
  // nsIDOMGCParticipant) require that we remove all event listeners now
  // to remove any weak references in the nsDOMClassInfo's preserved
  // wrapper table to the target.
  return RemoveAllListeners();
}

NS_IMETHODIMP
nsEventListenerManager::SetListenerTarget(nsISupports* aTarget)
{
  NS_PRECONDITION(aTarget, "unexpected null pointer");

  //WEAK reference, must be set back to nsnull when done by calling Disconnect
  mTarget = aTarget;
  return NS_OK;
}

NS_IMETHODIMP
nsEventListenerManager::GetSystemEventGroupLM(nsIDOMEventGroup **aGroup)
{
  if (!gSystemEventGroup) {
    nsresult result;
    nsCOMPtr<nsIDOMEventGroup> group(do_CreateInstance(kDOMEventGroupCID,&result));
    if (NS_FAILED(result))
      return result;

    gSystemEventGroup = group;
    NS_ADDREF(gSystemEventGroup);
  }

  *aGroup = gSystemEventGroup;
  NS_ADDREF(*aGroup);
  return NS_OK;
}

nsresult
nsEventListenerManager::GetDOM2EventGroup(nsIDOMEventGroup **aGroup)
{
  if (!gDOM2EventGroup) {
    nsresult result;
    nsCOMPtr<nsIDOMEventGroup> group(do_CreateInstance(kDOMEventGroupCID,&result));
    if (NS_FAILED(result))
      return result;

    gDOM2EventGroup = group;
    NS_ADDREF(gDOM2EventGroup);
  }

  *aGroup = gDOM2EventGroup;
  NS_ADDREF(*aGroup);
  return NS_OK;
}

// nsIDOMEventTarget interface
NS_IMETHODIMP 
nsEventListenerManager::AddEventListener(const nsAString& aType, 
                                         nsIDOMEventListener* aListener, 
                                         PRBool aUseCapture)
{
  PRInt32 flags = aUseCapture ? NS_EVENT_FLAG_CAPTURE : NS_EVENT_FLAG_BUBBLE;

  return AddEventListenerByType(aListener, aType, flags, nsnull);
}

NS_IMETHODIMP 
nsEventListenerManager::RemoveEventListener(const nsAString& aType, 
                                            nsIDOMEventListener* aListener, 
                                            PRBool aUseCapture)
{
  PRInt32 flags = aUseCapture ? NS_EVENT_FLAG_CAPTURE : NS_EVENT_FLAG_BUBBLE;
  
  return RemoveEventListenerByType(aListener, aType, flags, nsnull);
}

NS_IMETHODIMP
nsEventListenerManager::DispatchEvent(nsIDOMEvent* aEvent, PRBool *_retval)
{
  nsCOMPtr<nsIContent> targetContent(do_QueryInterface(mTarget));
  if (!targetContent) {
    // nothing to dispatch on -- bad!
    return NS_ERROR_FAILURE;
  }
  
  nsCOMPtr<nsIDocument> document = targetContent->GetOwnerDoc();

  // Do nothing if the element does not belong to a document
  if (!document) {
    return NS_OK;
  }

  // Obtain a presentation shell
  nsIPresShell *shell = document->GetShellAt(0);
  nsCOMPtr<nsPresContext> context;
  if (shell) {
    context = shell->GetPresContext();
  }

  nsEventStatus status = nsEventStatus_eIgnore;
  nsresult rv =
    nsEventDispatcher::DispatchDOMEvent(targetContent, nsnull, aEvent,
                                        context, &status);
  *_retval = (status != nsEventStatus_eConsumeNoDefault);
  return rv;
}

// nsIDOM3EventTarget interface
NS_IMETHODIMP 
nsEventListenerManager::AddGroupedEventListener(const nsAString& aType, 
                                                nsIDOMEventListener* aListener, 
                                                PRBool aUseCapture,
                                                nsIDOMEventGroup* aEvtGrp)
{
  PRInt32 flags = aUseCapture ? NS_EVENT_FLAG_CAPTURE : NS_EVENT_FLAG_BUBBLE;

  return AddEventListenerByType(aListener, aType, flags, aEvtGrp);
}

NS_IMETHODIMP 
nsEventListenerManager::RemoveGroupedEventListener(const nsAString& aType, 
                                            nsIDOMEventListener* aListener, 
                                            PRBool aUseCapture,
                                            nsIDOMEventGroup* aEvtGrp)
{
  PRInt32 flags = aUseCapture ? NS_EVENT_FLAG_CAPTURE : NS_EVENT_FLAG_BUBBLE;
  
  return RemoveEventListenerByType(aListener, aType, flags, aEvtGrp);
}

NS_IMETHODIMP
nsEventListenerManager::CanTrigger(const nsAString & type, PRBool *_retval)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsEventListenerManager::IsRegisteredHere(const nsAString & type, PRBool *_retval)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

// nsIDOMEventReceiver interface
NS_IMETHODIMP 
nsEventListenerManager::AddEventListenerByIID(nsIDOMEventListener *aListener, 
                                              const nsIID& aIID)
{
  return AddEventListenerByIID(aListener, aIID, NS_EVENT_FLAG_BUBBLE);
}

NS_IMETHODIMP 
nsEventListenerManager::RemoveEventListenerByIID(nsIDOMEventListener *aListener, const nsIID& aIID)
{
  return RemoveEventListenerByIID(aListener, aIID, NS_EVENT_FLAG_BUBBLE);
}

NS_IMETHODIMP 
nsEventListenerManager::GetListenerManager(PRBool aCreateIfNotFound,
                                           nsIEventListenerManager** aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);
  NS_ADDREF(*aResult = this);
  return NS_OK;
}
 
NS_IMETHODIMP 
nsEventListenerManager::HandleEvent(nsIDOMEvent *aEvent)
{
  PRBool defaultActionEnabled;
  return DispatchEvent(aEvent, &defaultActionEnabled);
}

NS_IMETHODIMP
nsEventListenerManager::GetSystemEventGroup(nsIDOMEventGroup **aGroup)
{
  return GetSystemEventGroupLM(aGroup);
}

nsresult
nsEventListenerManager::FixContextMenuEvent(nsPresContext* aPresContext,
                                            nsISupports* aCurrentTarget,
                                            nsEvent* aEvent,
                                            nsIDOMEvent** aDOMEvent)
{
  nsIPresShell* shell = aPresContext->GetPresShell();
  if (!shell) {
    // Nothing to do.
    return NS_OK;
  }

  nsresult ret = NS_OK;

  if (nsnull == *aDOMEvent) {
    // If we're here because of the key-equiv for showing context menus, we
    // have to twiddle with the NS event to make sure the context menu comes
    // up in the upper left of the relevant content area before we create
    // the DOM event. Since we never call InitMouseEvent() on the event, 
    // the client X/Y will be 0,0. We can make use of that if the widget is null.
    if (aEvent->message == NS_CONTEXTMENU_KEY) {
      NS_IF_RELEASE(((nsGUIEvent*)aEvent)->widget);
      aPresContext->GetViewManager()->GetWidget(&((nsGUIEvent*)aEvent)->widget);
      aEvent->refPoint.x = 0;
      aEvent->refPoint.y = 0;
    }
    ret = NS_NewDOMMouseEvent(aDOMEvent, aPresContext, NS_STATIC_CAST(nsInputEvent*, aEvent));
    NS_ENSURE_SUCCESS(ret, ret);
  }

  // see if we should use the caret position for the popup
  if (aEvent->message == NS_CONTEXTMENU_KEY) {
    nsPoint caretPoint;
    if (PrepareToUseCaretPosition(((nsGUIEvent*)aEvent)->widget,
                                  shell, caretPoint)) {
      // caret position is good
      aEvent->refPoint.x = caretPoint.x;
      aEvent->refPoint.y = caretPoint.y;
      return NS_OK;
    }
  }

  // If we're here because of the key-equiv for showing context menus, we
  // have to reset the event target to the currently focused element. Get it
  // from the focus controller.
  nsCOMPtr<nsIDOMEventTarget> currentTarget = do_QueryInterface(aCurrentTarget);
  nsCOMPtr<nsIDOMElement> currentFocus;

  if (aEvent->message == NS_CONTEXTMENU_KEY) {
    nsIDocument *doc = shell->GetDocument();
    if (doc) {
      nsPIDOMWindow* privWindow = doc->GetWindow();
      if (privWindow) {
        nsIFocusController *focusController =
          privWindow->GetRootFocusController();
        if (focusController)
          focusController->GetFocusedElement(getter_AddRefs(currentFocus));
      }
    }
  }

  if (currentFocus) {
    // Reset event coordinates relative to focused frame in view
    nsPoint targetPt;
    GetCoordinatesFor(currentFocus, aPresContext, shell, targetPt);
    aEvent->refPoint.x = targetPt.x;
    aEvent->refPoint.y = targetPt.y;

    currentTarget = do_QueryInterface(currentFocus);
    nsCOMPtr<nsIPrivateDOMEvent> pEvent(do_QueryInterface(*aDOMEvent));
    pEvent->SetTarget(currentTarget);
  }

  return ret;
}

// nsEventListenerManager::PrepareToUseCaretPosition
//
//    This checks to see if we should use the caret position for popup context
//    menus. Returns true if the caret position should be used, and the window
//    coordinates of that position are placed into WindowX/Y. This function
//    will also scroll the window as needed to make the caret visible.
//
//    The event widget should be the widget that generated the event, and
//    whose coordinate system the resulting event's refPoint should be
//    relative to.

PRBool
nsEventListenerManager::PrepareToUseCaretPosition(nsIWidget* aEventWidget,
                                                  nsIPresShell* aShell,
                                                  nsPoint& aTargetPt)
{
  nsresult rv;

  // check caret visibility
  nsCOMPtr<nsICaret> caret;
  rv = aShell->GetCaret(getter_AddRefs(caret));
  NS_ENSURE_SUCCESS(rv, PR_FALSE);
  NS_ENSURE_TRUE(caret, PR_FALSE);

  PRBool caretVisible = PR_FALSE;
  rv = caret->GetCaretVisible(&caretVisible);
  if (NS_FAILED(rv) || ! caretVisible)
    return PR_FALSE;

  // caret selection, watch out: GetCaretDOMSelection can return null but NS_OK
  nsCOMPtr<nsISelection> domSelection;
  rv = caret->GetCaretDOMSelection(getter_AddRefs(domSelection));
  NS_ENSURE_SUCCESS(rv, PR_FALSE);
  NS_ENSURE_TRUE(domSelection, PR_FALSE);

  // since the match could be an anonymous textnode inside a
  // <textarea> or text <input>, we need to get the outer frame
  // note: frames are not refcounted
  nsIFrame* frame = nsnull; // may be NULL
  nsCOMPtr<nsIDOMNode> node;
  rv = domSelection->GetFocusNode(getter_AddRefs(node));
  NS_ENSURE_SUCCESS(rv, PR_FALSE);
  NS_ENSURE_TRUE(node, PR_FALSE);
  nsCOMPtr<nsIContent> content(do_QueryInterface(node));
  for ( ; content; content = content->GetParent()) {
    if (!content->IsNativeAnonymous()) {
      frame = aShell->GetPrimaryFrameFor(content);
      NS_ASSERTION(frame, "No frame for focused content?");
      break;
    }
  }

  // It seems like selCon->ScrollSelectionIntoView should be enough, but it's
  // not. The problem is that scrolling the selection into view when it is
  // below the current viewport will align the top line of the frame exactly
  // with the bottom of the window. This is fine, BUT, the popup event causes
  // the control to be re-focused which does this exact call to
  // ScrollFrameIntoView, which has a one-pixel disagreement of whether the
  // frame is actually in view. The result is that the frame is aligned with
  // the top of the window, but the menu is still at the bottom.
  //
  // Doing this call first forces the frame to be in view, eliminating the
  // problem. The only difference in the result is that if your cursor is in
  // an edit box below the current view, you'll get the edit box aligned with
  // the top of the window. This is arguably better behavior anyway.
  if (frame) {
    rv = aShell->ScrollFrameIntoView(frame, NS_PRESSHELL_SCROLL_IF_NOT_VISIBLE,
                                     NS_PRESSHELL_SCROLL_IF_NOT_VISIBLE);
    NS_ENSURE_SUCCESS(rv, PR_FALSE);
  }

  // Actually scroll the selection (ie caret) into view. Note that this must
  // be synchronous since we will be checking the caret position on the screen.
  //
  // Be easy about errors, and just don't scroll in those cases. Better to have
  // the correct menu at a weird place than the wrong menu.
  nsCOMPtr<nsISelectionController> selCon;
  if (frame)
    frame->GetSelectionController(aShell->GetPresContext(),
                                  getter_AddRefs(selCon));
  else
    selCon = do_QueryInterface(aShell);
  if (selCon) {
    rv = selCon->ScrollSelectionIntoView(nsISelectionController::SELECTION_NORMAL,
        nsISelectionController::SELECTION_FOCUS_REGION, PR_TRUE);
    NS_ENSURE_SUCCESS(rv, PR_FALSE);
  }

  // get caret position relative to some view (normally the same as the
  // event widget view, but this is not guaranteed)
  PRBool isCollapsed;
  nsIView* view;
  nsRect caretCoords;
  rv = caret->GetCaretCoordinates(nsICaret::eRenderingViewCoordinates,
                                  domSelection, &caretCoords, &isCollapsed,
                                  &view);
  NS_ENSURE_SUCCESS(rv, PR_FALSE);

  // in case the view used for caret coordinates was something else, we need
  // to bring those coordinates into the space of the widget view
  nsIView* widgetView = nsIView::GetViewFor(aEventWidget);
  NS_ENSURE_TRUE(widgetView, PR_FALSE);
  nsPoint viewToWidget;
  widgetView->GetNearestWidget(&viewToWidget);
  nsPoint viewDelta = view->GetOffsetTo(widgetView) + viewToWidget;

  // caret coordinates are in twips, convert to pixels
  float t2p = aShell->GetPresContext()->TwipsToPixels();
  aTargetPt.x = NSTwipsToIntPixels(viewDelta.x + caretCoords.x + caretCoords.width, t2p);
  aTargetPt.y = NSTwipsToIntPixels(viewDelta.y + caretCoords.y + caretCoords.height, t2p);

  return PR_TRUE;
}

// Get coordinates relative to root view for element, 
// first ensuring the element is onscreen
void
nsEventListenerManager::GetCoordinatesFor(nsIDOMElement *aCurrentEl, 
                                          nsPresContext *aPresContext,
                                          nsIPresShell *aPresShell, 
                                          nsPoint& aTargetPt)
{
  nsCOMPtr<nsIContent> focusedContent(do_QueryInterface(aCurrentEl));
  nsIFrame *frame = aPresShell->GetPrimaryFrameFor(focusedContent);
  if (frame) {
    aPresShell->ScrollFrameIntoView(frame, NS_PRESSHELL_SCROLL_ANYWHERE,
                                           NS_PRESSHELL_SCROLL_ANYWHERE);

    nsPoint frameOrigin(0, 0);

    // Get the frame's origin within its view
    nsIView *view = frame->GetClosestView(&frameOrigin);
    NS_ASSERTION(view, "No view for frame");

    nsIViewManager* vm = aPresShell->GetViewManager();
    nsIView *rootView = nsnull;
    vm->GetRootView(rootView);
    NS_ASSERTION(rootView, "No root view in pres shell");

    // View's origin within its root view
    frameOrigin += view->GetOffsetTo(rootView);

    // Start context menu down and to the right from top left of frame
    // use the lineheight. This is a good distance to move the context
    // menu away from the top left corner of the frame. If we always 
    // used the frame height, the context menu could end up far away,
    // for example when we're focused on linked images.
    // On the other hand, we want to use the frame height if it's less
    // than the current line height, so that the context menu appears
    // associated with the correct frame.
    nscoord extra = frame->GetSize().height;
    nsIScrollableView *scrollView =
      nsLayoutUtils::GetNearestScrollingView(view, nsLayoutUtils::eEither);
    if (scrollView) {
      nscoord scrollViewLineHeight;
      scrollView->GetLineHeight(&scrollViewLineHeight);
      if (extra > scrollViewLineHeight) {
        extra = scrollViewLineHeight; 
      }
    }

    PRInt32 extraPixelsY = 0;
#ifdef MOZ_XUL
    // Tree view special case (tree items have no frames)
    // Get the focused row and add its coordinates, which are already in pixels
    // XXX Boris, should we create a new interface so that event listener manager doesn't
    // need to know about trees? Something like nsINodelessChildCreator which
    // could provide the current focus coordinates?
    nsCOMPtr<nsIDOMXULElement> xulElement(do_QueryInterface(aCurrentEl));
    if (xulElement) {
      nsCOMPtr<nsIBoxObject> box;
      xulElement->GetBoxObject(getter_AddRefs(box));
      nsCOMPtr<nsITreeBoxObject> treeBox(do_QueryInterface(box));
      if (treeBox) {
        // Factor in focused row
        nsCOMPtr<nsIDOMXULMultiSelectControlElement> multiSelect =
          do_QueryInterface(aCurrentEl);
        NS_ASSERTION(multiSelect, "No multi select interface for tree");

        PRInt32 currentIndex;
        multiSelect->GetCurrentIndex(&currentIndex);
        if (currentIndex >= 0) {
          treeBox->EnsureRowIsVisible(currentIndex);
          PRInt32 firstVisibleRow, rowHeight;
          treeBox->GetFirstVisibleRow(&firstVisibleRow);
          treeBox->GetRowHeight(&rowHeight);
          extraPixelsY = (currentIndex - firstVisibleRow + 1) * rowHeight;
          extra = 0;

          nsCOMPtr<nsITreeColumns> cols;
          treeBox->GetColumns(getter_AddRefs(cols));
          if (cols) {
            nsCOMPtr<nsITreeColumn> col;
            cols->GetFirstColumn(getter_AddRefs(col));
            if (col) {
              nsCOMPtr<nsIDOMElement> colElement;
              col->GetElement(getter_AddRefs(colElement));
              nsCOMPtr<nsIContent> colContent(do_QueryInterface(colElement));
              if (colContent) {
                frame = aPresShell->GetPrimaryFrameFor(colContent);
                if (frame) {
                  frameOrigin.y += frame->GetSize().height;
                }
              }
            }
          }
        }
      }
    }
#endif

    // Convert from twips to pixels
    float t2p = aPresContext->TwipsToPixels();
    aTargetPt.x = NSTwipsToIntPixels(frameOrigin.x + extra, t2p);
    aTargetPt.y = NSTwipsToIntPixels(frameOrigin.y + extra, t2p) + extraPixelsY;
  }
}

PRBool
nsEventListenerManager::HasUnloadListeners()
{
  nsVoidArray *listeners = GetListenersByType(eEventArrayType_Load, nsnull,
                                              PR_FALSE);
  if (listeners) {
    PRInt32 count = listeners->Count();
    for (PRInt32 i = 0; i < count; ++i) {
      PRUint32 subtype = NS_STATIC_CAST(nsListenerStruct*,
                                        listeners->FastElementAt(i))->mSubType;
      if (subtype == NS_EVENT_BITS_NONE ||
          subtype & (NS_EVENT_BITS_LOAD_UNLOAD |
                     NS_EVENT_BITS_LOAD_BEFORE_UNLOAD))
        return PR_TRUE;
    }
  }

  return PR_FALSE;
}

nsresult
NS_NewEventListenerManager(nsIEventListenerManager** aInstancePtrResult) 
{
  nsIEventListenerManager* l = new nsEventListenerManager();

  if (!l) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  
  return CallQueryInterface(l, aInstancePtrResult);
}
