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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "nsXMLElement.h"
#include "nsGkAtoms.h"
#include "nsHTMLAtoms.h"
#include "nsLayoutAtoms.h"
#include "nsIDocument.h"
#include "nsIAtom.h"
#include "nsNetUtil.h"
#include "nsIEventListenerManager.h"
#include "nsIDocShell.h"
#include "nsIEventStateManager.h"
#include "nsIDOMEvent.h"
#include "nsINameSpaceManager.h"
#include "nsINodeInfo.h"
#include "nsIURL.h"
#include "nsIIOService.h"
#include "nsNetCID.h"
#include "nsIServiceManager.h"
#include "nsXPIDLString.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIScriptSecurityManager.h"
#include "nsIRefreshURI.h"
#include "nsStyleConsts.h"
#include "nsIPresShell.h"
#include "nsGUIEvent.h"
#include "nsPresContext.h"
#include "nsIDOMCSSStyleDeclaration.h"
#include "nsIDOMViewCSS.h"
#include "nsIXBLService.h"
#include "nsIBindingManager.h"
#include "nsLayoutAtoms.h"
#include "nsEventDispatcher.h"
#include "nsContentErrors.h"

static nsIContent::AttrValuesArray strings[] =
  {&nsLayoutAtoms::_new, &nsLayoutAtoms::replace, &nsLayoutAtoms::embed, nsnull};

nsresult
NS_NewXMLElement(nsIContent** aInstancePtrResult, nsINodeInfo *aNodeInfo)
{
  nsXMLElement* it = new nsXMLElement(aNodeInfo);
  if (!it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  NS_ADDREF(*aInstancePtrResult = it);

  return NS_OK;
}

nsXMLElement::nsXMLElement(nsINodeInfo *aNodeInfo)
  : nsGenericElement(aNodeInfo),
    mIsLink(PR_FALSE)
{
}

nsXMLElement::~nsXMLElement()
{
}


// QueryInterface implementation for nsXMLElement
NS_IMETHODIMP 
nsXMLElement::QueryInterface(REFNSIID aIID, void** aInstancePtr)
{
  NS_ENSURE_ARG_POINTER(aInstancePtr);
  *aInstancePtr = nsnull;

  nsresult rv = nsGenericElement::QueryInterface(aIID, aInstancePtr);

  if (NS_SUCCEEDED(rv))
    return rv;

  nsISupports *inst = nsnull;

  if (aIID.Equals(NS_GET_IID(nsIDOMNode))) {
    inst = NS_STATIC_CAST(nsIDOMNode *, this);
  } else if (aIID.Equals(NS_GET_IID(nsIDOMElement))) {
    inst = NS_STATIC_CAST(nsIDOMElement *, this);
  } else if (aIID.Equals(NS_GET_IID(nsIXMLContent))) {
    inst = NS_STATIC_CAST(nsIXMLContent *, this);
  } else if (aIID.Equals(NS_GET_IID(nsIClassInfo))) {
    inst = nsContentUtils::GetClassInfoInstance(eDOMClassInfo_Element_id);
    NS_ENSURE_TRUE(inst, NS_ERROR_OUT_OF_MEMORY);
  } else {
    return PostQueryInterface(aIID, aInstancePtr);
  }

  NS_ADDREF(inst);

  *aInstancePtr = inst;

  return NS_OK;
}


NS_IMPL_ADDREF_INHERITED(nsXMLElement, nsGenericElement)
NS_IMPL_RELEASE_INHERITED(nsXMLElement, nsGenericElement)

nsresult
nsXMLElement::SetAttr(PRInt32 aNameSpaceID, nsIAtom* aName, nsIAtom* aPrefix,
                      const nsAString& aValue, PRBool aNotify)
{
  if (aNameSpaceID == kNameSpaceID_XLink && aName == nsHTMLAtoms::type) { 

    // NOTE: This really is a link according to the XLink spec,
    //       we do not need to check other attributes. If there
    //       is no href attribute, then this link is simply
    //       untraversible [XLink 3.2].
    mIsLink = aValue.EqualsLiteral("simple");

    // We will check for actuate="onLoad" in MaybeTriggerAutoLink
  }

  return nsGenericElement::SetAttr(aNameSpaceID, aName, aPrefix, aValue,
                                   aNotify);
}

static nsresult
DocShellToPresContext(nsIDocShell *aShell, nsPresContext **aPresContext)
{
  *aPresContext = nsnull;

  nsresult rv;
  nsCOMPtr<nsIDocShell> ds = do_QueryInterface(aShell,&rv);
  if (NS_FAILED(rv))
    return rv;

  return ds->GetPresContext(aPresContext);
}

static inline
nsresult SpecialAutoLoadReturn(nsresult aRv, nsLinkVerb aVerb)
{
  if (NS_SUCCEEDED(aRv)) {
    switch(aVerb) {
      case eLinkVerb_Embed:
        aRv = NS_XML_AUTOLINK_EMBED;
        break;
      case eLinkVerb_New:
        aRv = NS_XML_AUTOLINK_NEW;
        break;
      case eLinkVerb_Replace:
        aRv = NS_XML_AUTOLINK_REPLACE;
        break;
      default:
        aRv = NS_XML_AUTOLINK_UNDEFINED;
        break;
    }
  }
  return aRv;
}

NS_IMETHODIMP
nsXMLElement::MaybeTriggerAutoLink(nsIDocShell *aShell)
{
  NS_ENSURE_ARG_POINTER(aShell);

  nsresult rv = NS_OK;

  if (mIsLink) {
    do {
      // actuate="onLoad" ?
      if (AttrValueIs(kNameSpaceID_XLink, nsLayoutAtoms::actuate,
                      nsLayoutAtoms::onLoad, eCaseMatters)) {

        // Disable in Mail/News for now. We may want a pref to control
        // this at some point.
        nsCOMPtr<nsIDocShellTreeItem> docShellItem(do_QueryInterface(aShell));
        if (docShellItem) {
          nsCOMPtr<nsIDocShellTreeItem> rootItem;
          docShellItem->GetRootTreeItem(getter_AddRefs(rootItem));
          nsCOMPtr<nsIDocShell> docshell(do_QueryInterface(rootItem));
          if (docshell) {
            PRUint32 appType;
            if (NS_SUCCEEDED(docshell->GetAppType(&appType)) &&
                appType == nsIDocShell::APP_TYPE_MAIL) {
              return NS_OK;
            }
          }
        }

        // show= ?
        nsLinkVerb verb = eLinkVerb_Undefined; // basically means same as replace
        PRBool stop = PR_FALSE;

        // XXX Should probably do this using atoms 
        switch (FindAttrValueIn(kNameSpaceID_XLink, nsLayoutAtoms::show,
                                strings, eCaseMatters)) {
          // We should just act like an HTML link with target="_blank" and if
          // someone diverts or blocks those, that's fine with us.  We don't
          // care.
          case 0: verb = eLinkVerb_New; break;
          // We want to actually stop processing the current document now.
          // We do this by returning the correct value so that the one
          // that called us knows to stop processing.
          case 1: verb = eLinkVerb_Replace; break;
          // XXX TODO
          case 2: stop = PR_TRUE; break;
        }

        if (stop)
          break;

        // Get our URI
        nsCOMPtr<nsIURI> uri = nsContentUtils::GetXLinkURI(this);
        if (!uri)
          break;

        nsCOMPtr<nsPresContext> pc;
        rv = DocShellToPresContext(aShell, getter_AddRefs(pc));
        if (NS_SUCCEEDED(rv)) {
          rv = TriggerLink(pc, verb, uri, EmptyString(), PR_TRUE, PR_FALSE);

          return SpecialAutoLoadReturn(rv,verb);
        }
      }
    } while (0);
  }

  return rv;
}

nsresult
nsXMLElement::PostHandleEvent(nsEventChainPostVisitor& aVisitor)
{
  nsresult rv = NS_OK;
  if (mIsLink && nsEventStatus_eIgnore == aVisitor.mEventStatus) {
    nsIDocument *document = GetCurrentDoc();
    switch (aVisitor.mEvent->message) {
    case NS_MOUSE_LEFT_BUTTON_DOWN:
      {
        if (aVisitor.mPresContext) {
          aVisitor.mPresContext->EventStateManager()->
            SetContentState(this, NS_EVENT_STATE_ACTIVE | NS_EVENT_STATE_FOCUS);

          aVisitor.mEventStatus = nsEventStatus_eConsumeDoDefault;
        }
      }
      break;

    case NS_MOUSE_LEFT_CLICK:
      {
        if (nsEventStatus_eConsumeNoDefault != aVisitor.mEventStatus &&
            aVisitor.mPresContext) {
          nsInputEvent* inputEvent =
            NS_STATIC_CAST(nsInputEvent*, aVisitor.mEvent);
          if (inputEvent->isControl || inputEvent->isMeta ||
              inputEvent->isAlt || inputEvent->isShift) {
            break;  // let the click go through so we can handle it in JS/XUL
          }
          nsAutoString href;
          nsLinkVerb verb = eLinkVerb_Undefined; // basically means same as replace
          nsCOMPtr<nsIURI> uri = nsContentUtils::GetXLinkURI(this);
          if (!uri) {
            aVisitor.mEventStatus = nsEventStatus_eConsumeDoDefault;
            break;
          }

          // XXX Should probably do this using atoms 
          switch (FindAttrValueIn(kNameSpaceID_XLink, nsLayoutAtoms::show,
                                  strings, eCaseMatters)) {
            case 0: verb = eLinkVerb_New; break;
            case 1: verb = eLinkVerb_Replace; break;
            case 2: verb = eLinkVerb_Embed; break;
          }

          nsAutoString target;
          GetAttr(kNameSpaceID_XLink, nsLayoutAtoms::_moz_target, target);
          rv = TriggerLink(aVisitor.mPresContext, verb, uri,
                           target, PR_TRUE, PR_TRUE);

          aVisitor.mEventStatus = nsEventStatus_eConsumeDoDefault;
        }
      }
      break;

    case NS_MOUSE_RIGHT_BUTTON_DOWN:
      // XXX Bring up a contextual menu provided by the application
      break;

    case NS_KEY_PRESS:
      if (aVisitor.mEvent->eventStructType == NS_KEY_EVENT) {
        nsKeyEvent* keyEvent = NS_STATIC_CAST(nsKeyEvent*, aVisitor.mEvent);
        if (keyEvent->keyCode == NS_VK_RETURN) {
          nsEventStatus status = nsEventStatus_eIgnore;
          rv = DispatchClickEvent(aVisitor.mPresContext, keyEvent, this,
                                  PR_FALSE, &status);
          if (NS_SUCCEEDED(rv)) {
            aVisitor.mEventStatus = nsEventStatus_eConsumeNoDefault;
          }
        }
      }
      break;

    case NS_MOUSE_ENTER_SYNTH:
      {
        if (aVisitor.mPresContext) {
          nsCOMPtr<nsIURI> uri = nsContentUtils::GetXLinkURI(this);
          if (!uri) {
            aVisitor.mEventStatus = nsEventStatus_eConsumeDoDefault;
            break;
          }

          rv = TriggerLink(aVisitor.mPresContext, eLinkVerb_Replace, uri,
                           EmptyString(), PR_FALSE, PR_TRUE);

          aVisitor.mEventStatus = nsEventStatus_eConsumeDoDefault;
        }
      }
      break;

      // XXX this doesn't seem to do anything yet
    case NS_MOUSE_EXIT_SYNTH:
      {
        if (aVisitor.mPresContext) {
          rv = LeaveLink(aVisitor.mPresContext);
          aVisitor.mEventStatus = nsEventStatus_eConsumeDoDefault;
        }
      }
      break;

    default:
      break;
    }
  }

  return rv;
}

PRBool
nsXMLElement::IsFocusable(PRInt32 *aTabIndex)
{
  nsCOMPtr<nsIURI> linkURI = nsContentUtils::GetLinkURI(this);
  if (linkURI) {
    if (aTabIndex) {
      *aTabIndex = ((sTabFocusModel & eTabFocus_linksMask) == 0 ? -1 : 0);
    }
    return PR_TRUE;
  }

  if (aTabIndex) {
    *aTabIndex = -1;
  }

  return PR_FALSE;
}


NS_IMPL_DOM_CLONENODE(nsXMLElement)
