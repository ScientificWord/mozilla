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
 * Portions created by the Initial Developer are Copyright (C) 2003
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Original Author: Aaron Leventhal (aaronl@netscape.com)
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

#include "nsAccessibleWrap.h"
#include "nsAccessibilityAtoms.h"
#include "nsIAccessibleSelectable.h"
#include "nsIAccessibleWin32Object.h"
#include "nsIMutableArray.h"
#include "nsIDOMDocument.h"
#include "nsIFrame.h"
#include "nsIScrollableFrame.h"
#include "nsINameSpaceManager.h"
#include "nsINodeInfo.h"
#include "nsIPrefService.h"
#include "nsIServiceManager.h"
#include "nsTextFormatter.h"
#include "nsIView.h"

// for the COM IEnumVARIANT solution in get_AccSelection()
#define _ATLBASE_IMPL
#include <atlbase.h>
extern CComModule _Module;
#define _ATLCOM_IMPL
#include <atlcom.h>

/* For documentation of the accessibility architecture,
 * see http://lxr.mozilla.org/seamonkey/source/accessible/accessible-docs.html
 */

//#define DEBUG_LEAKS

#ifdef DEBUG_LEAKS
static gAccessibles = 0;
#endif

CComModule _Module;

EXTERN_C GUID CDECL CLSID_Accessible =
{ 0x61044601, 0xa811, 0x4e2b, { 0xbb, 0xba, 0x17, 0xbf, 0xab, 0xd3, 0x29, 0xd7 } };

/*
 * Class nsAccessibleWrap
 */

//-----------------------------------------------------
// construction
//-----------------------------------------------------
nsAccessibleWrap::nsAccessibleWrap(nsIDOMNode* aNode, nsIWeakReference *aShell):
  nsAccessible(aNode, aShell), mEnumVARIANTPosition(0)
{
}

//-----------------------------------------------------
// destruction
//-----------------------------------------------------
nsAccessibleWrap::~nsAccessibleWrap()
{
}

//-----------------------------------------------------
// IUnknown interface methods - see iunknown.h for documentation
//-----------------------------------------------------
STDMETHODIMP_(ULONG) nsAccessibleWrap::AddRef()
{
  return nsAccessNode::AddRef();
}

STDMETHODIMP_(ULONG) nsAccessibleWrap::Release()
{
  return nsAccessNode::Release();
}

// Microsoft COM QueryInterface
STDMETHODIMP nsAccessibleWrap::QueryInterface(REFIID iid, void** ppv)
{
  *ppv = NULL;

  if (IID_IUnknown == iid || IID_IDispatch == iid || IID_IAccessible == iid)
    *ppv = NS_STATIC_CAST(IAccessible*, this);
  else if (IID_IEnumVARIANT == iid && !gIsEnumVariantSupportDisabled) {
    long numChildren;
    get_accChildCount(&numChildren);
    if (numChildren > 0)  // Don't support this interface for leaf elements
      *ppv = NS_STATIC_CAST(IEnumVARIANT*, this);
  }
  else if (IID_IServiceProvider == iid) {
    *ppv = NS_STATIC_CAST(IServiceProvider*, this);
  }

  if (NULL == *ppv)
    return nsAccessNodeWrap::QueryInterface(iid, ppv);

  (NS_REINTERPRET_CAST(IUnknown*, *ppv))->AddRef();
  return S_OK;
}

//-----------------------------------------------------
// IAccessible methods
//-----------------------------------------------------


STDMETHODIMP nsAccessibleWrap::AccessibleObjectFromWindow(HWND hwnd,
                                                          DWORD dwObjectID,
                                                          REFIID riid,
                                                          void **ppvObject)
{
  // open the dll dynamically
  if (!gmAccLib)
    gmAccLib =::LoadLibrary("OLEACC.DLL");

  if (gmAccLib) {
    if (!gmAccessibleObjectFromWindow)
      gmAccessibleObjectFromWindow = (LPFNACCESSIBLEOBJECTFROMWINDOW)GetProcAddress(gmAccLib,"AccessibleObjectFromWindow");

    if (gmAccessibleObjectFromWindow)
      return gmAccessibleObjectFromWindow(hwnd, dwObjectID, riid, ppvObject);
  }

  return E_FAIL;
}

STDMETHODIMP nsAccessibleWrap::NotifyWinEvent(DWORD event,
                                              HWND hwnd,
                                              LONG idObjectType,
                                              LONG idObject)
{
  if (gmNotifyWinEvent)
    return gmNotifyWinEvent(event, hwnd, idObjectType, idObject);

  return E_FAIL;
}

STDMETHODIMP nsAccessibleWrap::get_accParent( IDispatch __RPC_FAR *__RPC_FAR *ppdispParent)
{
  *ppdispParent = NULL;
  if (!mWeakShell)
    return E_FAIL;  // We've been shut down

  nsIFrame *frame = GetFrame();
  HWND hwnd = 0;
  if (frame) {
    nsIView *view = frame->GetViewExternal();
    if (view) {
      // This code is essentially our implementation of WindowFromAccessibleObject,
      // because MSAA iterates get_accParent() until it sees an object of ROLE_WINDOW
      // to know where the window for a given accessible is. We must expose the native
      // window accessible that MSAA creates for us. This must be done for the document
      // object as well as any layout that creates its own window (e.g. via overflow: scroll)
      nsIWidget *widget = view->GetWidget();
      if (widget) {
        hwnd = (HWND)widget->GetNativeData(NS_NATIVE_WINDOW);
        NS_ASSERTION(hwnd, "No window handle for window");
        nsIView *rootView;
        view->GetViewManager()->GetRootView(rootView);
        if (rootView == view) {
          // If the current object has a widget but was created by an
          // outer object with its own outer window, then
          // we want the native accessible for that outer window
          hwnd = ::GetParent(hwnd);
          NS_ASSERTION(hwnd, "No window handle for window");
        }
      }
      else {
        // If a frame is a scrollable frame, then it has one window for the client area,
        // not an extra parent window for just the scrollbars
        nsIScrollableFrame *scrollFrame = nsnull;
        CallQueryInterface(frame, &scrollFrame);
        if (scrollFrame) {
          hwnd = (HWND)scrollFrame->GetScrolledFrame()->GetWindow()->GetNativeData(NS_NATIVE_WINDOW);
          NS_ASSERTION(hwnd, "No window handle for window");
        }
      }
    }

    if (hwnd && SUCCEEDED(AccessibleObjectFromWindow(hwnd, OBJID_WINDOW, IID_IAccessible,
                                              (void**)ppdispParent))) {
      return S_OK;
    }
  }

  nsCOMPtr<nsIAccessible> xpParentAccessible;
  GetParent(getter_AddRefs(xpParentAccessible));
  NS_ASSERTION(xpParentAccessible, "No parent accessible where we're not direct child of window");
  if (!xpParentAccessible) {
    return E_UNEXPECTED;
  }
  *ppdispParent = NativeAccessible(xpParentAccessible);

  return S_OK;
}

STDMETHODIMP nsAccessibleWrap::get_accChildCount( long __RPC_FAR *pcountChildren)
{
  *pcountChildren = 0;
  if (MustPrune(this)) {
    return NS_OK;
  }

  PRInt32 numChildren;
  GetChildCount(&numChildren);
  *pcountChildren = numChildren;

  return S_OK;
}

STDMETHODIMP nsAccessibleWrap::get_accChild(
      /* [in] */ VARIANT varChild,
      /* [retval][out] */ IDispatch __RPC_FAR *__RPC_FAR *ppdispChild)
{
  *ppdispChild = NULL;

  if (!mWeakShell || varChild.vt != VT_I4)
    return E_FAIL;

  if (varChild.lVal == CHILDID_SELF) {
    *ppdispChild = NS_STATIC_CAST(IDispatch*, this);
    AddRef();
    return S_OK;
  }

  nsCOMPtr<nsIAccessible> childAccessible;
  if (!MustPrune(this)) {
    GetChildAt(varChild.lVal - 1, getter_AddRefs(childAccessible));
    *ppdispChild = NativeAccessible(childAccessible);
  }

  return (*ppdispChild)? S_OK: E_FAIL;
}

STDMETHODIMP nsAccessibleWrap::get_accName(
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ BSTR __RPC_FAR *pszName)
{
  *pszName = NULL;
  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));
  if (xpAccessible) {
    nsAutoString name;
    if (NS_FAILED(xpAccessible->GetName(name)))
      return S_FALSE;
    if (!name.IsVoid()) {
      *pszName = ::SysAllocString(name.get());
    }
    NS_ASSERTION(mIsInitialized, "Access node was not initialized");
  }

  return S_OK;
}


STDMETHODIMP nsAccessibleWrap::get_accValue(
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ BSTR __RPC_FAR *pszValue)
{
  *pszValue = NULL;
  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));
  if (xpAccessible) {
    nsAutoString value;
    if (NS_FAILED(xpAccessible->GetValue(value)))
      return S_FALSE;

    *pszValue = ::SysAllocString(value.get());
  }

  return S_OK;
}

NS_IMETHODIMP nsAccessibleWrap::GetDescription(nsAString& aDescription)
{
  // For items that are a choice in a list of choices,
  // use MSAA description field to shoehorn positional info, it's becoming
  // a defacto standard use for the field.
  // Tree items override this, because they provide the current level as well

  aDescription.Truncate();
  PRUint32 currentRole;
  nsresult rv = GetFinalRole(&currentRole);
  if (NS_FAILED(rv) ||
      (currentRole != ROLE_LISTITEM && currentRole != ROLE_MENUITEM &&
       currentRole != ROLE_RADIOBUTTON && currentRole != ROLE_PAGETAB &&
       currentRole != ROLE_OUTLINEITEM)) {
    nsAutoString description;
    nsAccessible::GetDescription(description);
    if (!description.IsEmpty()) {
      // Signal to screen readers that this description is speakable
      // and is not a formatted positional information description
      // Don't localize the "Description: " part of this string, it will be
      // parsed out by assistive technologies.
      aDescription = NS_LITERAL_STRING("Description: ") + description;
    }
    return NS_OK;
  }

  nsCOMPtr<nsIAccessible> parent;
  GetParent(getter_AddRefs(parent));
  if (!parent) {
    return NS_ERROR_FAILURE;
  }

  PRInt32 indexInParent = 0, numSiblings = 0;

  nsCOMPtr<nsIAccessible> sibling, nextSibling;
  parent->GetFirstChild(getter_AddRefs(sibling));
  NS_ENSURE_TRUE(sibling, NS_ERROR_FAILURE);

  PRBool foundCurrent = PR_FALSE;
  PRUint32 siblingRole;
  while (sibling) {
    sibling->GetFinalRole(&siblingRole);
    if (siblingRole == currentRole) {
      ++ numSiblings;
      if (!foundCurrent) {
        ++ indexInParent;
        if (sibling == this) {
          foundCurrent = PR_TRUE;
        }
      }
    }
    sibling->GetNextSibling(getter_AddRefs(nextSibling));
    sibling = nextSibling;
  }

  // Don't localize the string "of" -- that's just the format of this string.
  // The AT will parse the relevant numbers out and add its own localization.
  if (currentRole == ROLE_OUTLINEITEM) {
    PRUint32 level = 1;
    nsCOMPtr<nsIAccessible> nextParent;
    while (parent) {
      parent->GetFinalRole(&currentRole);
      if (currentRole != ROLE_GROUPING) {
        break;
      }
      ++level;
      parent->GetParent(getter_AddRefs(nextParent));
      parent.swap(nextParent);
    }

    // Count the number of tree item children
    PRInt32 numChildren = 0;
    nsCOMPtr<nsIAccessible> groupSibling;
    GetNextSibling(getter_AddRefs(groupSibling));
    if (groupSibling) {
      groupSibling->GetFinalRole(&currentRole);
      if (currentRole == ROLE_GROUPING) {
        // Accessible that groups child tree items
        nsCOMPtr<nsIAccessible> child;
        groupSibling->GetFirstChild(getter_AddRefs(child));
        while (child) {
          child->GetFinalRole(&currentRole);
          numChildren += (currentRole == ROLE_OUTLINEITEM);
          nsCOMPtr<nsIAccessible> nextChild;
          child->GetNextSibling(getter_AddRefs(nextChild));
          child.swap(nextChild);
        }
      }
    }

    // This must be a DHTML tree item -- XUL tree items impl GetDescription()
    nsTextFormatter::ssprintf(aDescription,
                              NS_LITERAL_STRING("L%d, %d of %d with %d").get(),
                              level, indexInParent, numSiblings, numChildren);
  }
  else {
    nsTextFormatter::ssprintf(aDescription, NS_LITERAL_STRING("%d of %d").get(),
                              indexInParent, numSiblings);
  }
  return NS_OK;
}

STDMETHODIMP nsAccessibleWrap::get_accDescription(
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ BSTR __RPC_FAR *pszDescription)
{
  *pszDescription = NULL;
  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));
  if (xpAccessible) {
     nsAutoString description;
     if (NS_FAILED(xpAccessible->GetDescription(description)))
       return S_FALSE;

     *pszDescription = ::SysAllocString(description.get());
  }

  return S_OK;
}

STDMETHODIMP nsAccessibleWrap::get_accRole(
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ VARIANT __RPC_FAR *pvarRole)
{
  VariantInit(pvarRole);

  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));

  if (!xpAccessible)
    return E_FAIL;

  PRUint32 role = 0;
  if (NS_FAILED(xpAccessible->GetFinalRole(&role)))
    return E_FAIL;

  // Begin check for extended roles that need to be mapped to something known
  if (role == ROLE_ENTRY || role == ROLE_PASSWORD_TEXT) {
    role = ROLE_TEXT_LEAF;
  }

  // -- Try enumerated role
  if (role != ROLE_NOTHING && role != ROLE_CLIENT) {
    pvarRole->vt = VT_I4;
    pvarRole->lVal = role;  // Normal enumerated role
    return S_OK;
  }

  // -- Try BSTR role
  // Could not map to known enumerated MSAA role like ROLE_BUTTON
  // Use BSTR role to expose role attribute or tag name + namespace
  nsCOMPtr<nsIDOMNode> domNode;
  nsCOMPtr<nsIAccessNode> accessNode(do_QueryInterface(xpAccessible));
  NS_ASSERTION(accessNode, "No accessnode for accessible");
  accessNode->GetDOMNode(getter_AddRefs(domNode));
  nsIContent *content = GetRoleContent(domNode);
  NS_ASSERTION(content, "No content for accessible");
  if (content && content->IsNodeOfType(nsINode::eELEMENT)) {
    nsAutoString roleString;
    if (role != ROLE_CLIENT && GetRoleAttribute(content, roleString)) {
      nsINodeInfo *nodeInfo = content->NodeInfo();
      nodeInfo->GetName(roleString);
      nsAutoString nameSpaceURI;
      nodeInfo->GetNamespaceURI(nameSpaceURI);
      if (!nameSpaceURI.IsEmpty()) {
        // Only append name space if different from that of current document
        roleString += NS_LITERAL_STRING(", ") + nameSpaceURI;
      }
    }
    if (!roleString.IsEmpty()) {
      pvarRole->vt = VT_BSTR;
      pvarRole->bstrVal = ::SysAllocString(roleString.get());
      return S_OK;
    }
  }
  return E_FAIL;
}

STDMETHODIMP nsAccessibleWrap::get_accState(
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ VARIANT __RPC_FAR *pvarState)
{
  VariantInit(pvarState);
  pvarState->vt = VT_I4;
  pvarState->lVal = 0;

  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));
  if (!xpAccessible)
    return E_FAIL;

  PRUint32 state;
  if (NS_FAILED(xpAccessible->GetFinalState(&state)))
    return E_FAIL;

  pvarState->lVal = state;

  return S_OK;
}


STDMETHODIMP nsAccessibleWrap::get_accHelp(
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ BSTR __RPC_FAR *pszHelp)
{
  *pszHelp = NULL;
  return S_FALSE;
}

STDMETHODIMP nsAccessibleWrap::get_accHelpTopic(
      /* [out] */ BSTR __RPC_FAR *pszHelpFile,
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ long __RPC_FAR *pidTopic)
{
  *pszHelpFile = NULL;
  *pidTopic = 0;
  return E_NOTIMPL;
}

STDMETHODIMP nsAccessibleWrap::get_accKeyboardShortcut(
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ BSTR __RPC_FAR *pszKeyboardShortcut)
{
  *pszKeyboardShortcut = NULL;
  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));
  if (xpAccessible) {
    nsAutoString shortcut;
    nsresult rv = xpAccessible->GetKeyboardShortcut(shortcut);
    if (NS_FAILED(rv))
      return S_FALSE;

    *pszKeyboardShortcut = ::SysAllocString(shortcut.get());
    return S_OK;
  }
  return S_FALSE;
}

STDMETHODIMP nsAccessibleWrap::get_accFocus(
      /* [retval][out] */ VARIANT __RPC_FAR *pvarChild)
{
  // VT_EMPTY:    None. This object does not have the keyboard focus itself
  //              and does not contain a child that has the keyboard focus.
  // VT_I4:       lVal is CHILDID_SELF. The object itself has the keyboard focus.
  // VT_I4:       lVal contains the child ID of the child element with the keyboard focus.
  // VT_DISPATCH: pdispVal member is the address of the IDispatch interface
  //              for the child object with the keyboard focus.

  if (!mDOMNode) {
    return E_FAIL; // This node is shut down
  }

  VariantInit(pvarChild);

  // Return the current IAccessible child that has focus
  nsCOMPtr<nsIAccessible> focusedAccessible;
  GetFocusedChild(getter_AddRefs(focusedAccessible));
  if (focusedAccessible == this) {
    pvarChild->vt = VT_I4;
    pvarChild->lVal = CHILDID_SELF;
  }
  else if (focusedAccessible) {
    pvarChild->vt = VT_DISPATCH;
    pvarChild->pdispVal = NativeAccessible(focusedAccessible);
  }
  else {
    pvarChild->vt = VT_EMPTY;   // No focus or focus is not a child
  }

  return S_OK;
}

/**
  * This method is called when a client wants to know which children of a node
  *  are selected. Currently we only handle this for HTML selects, which are the
  *  only nsIAccessible objects to implement nsIAccessibleSelectable.
  *
  * The VARIANT return value arguement is expected to either contain a single IAccessible
  *  or an IEnumVARIANT of IAccessibles. We return the IEnumVARIANT regardless of the number
  *  of options selected, unless there are none selected in which case we return an empty
  *  VARIANT.
  *
  * The typedefs at the beginning set up the structure that will contain an array
  *  of the IAccessibles. It implements the IEnumVARIANT interface, allowing us to
  *  use it to return the IAccessibles in the VARIANT.
  *
  * We get the selected options from the select's accessible object and then put create
  *  IAccessible objects for them and put those in the CComObject<EnumeratorType>
  *  object. Then we put the CComObject<EnumeratorType> object in the VARIANT and return.
  *
  * returns a VT_EMPTY VARIANT if:
  *  - there are no options in the select
  *  - none of the options are selected
  *  - there is an error QIing to IEnumVARIANT
  *  - The object is not the type that can have children selected
  */
STDMETHODIMP nsAccessibleWrap::get_accSelection(VARIANT __RPC_FAR *pvarChildren)
{
  typedef VARIANT                      ItemType;              /* type of the object to be stored in container */
  typedef ItemType                     EnumeratorExposedType; /* the type of the item exposed by the enumerator interface */
  typedef IEnumVARIANT                 EnumeratorInterface;   /* a COM enumerator ( IEnumXXXXX ) interface */
  typedef _Copy<EnumeratorExposedType> EnumeratorCopyPolicy;  /* Copy policy class */
  typedef CComEnum<EnumeratorInterface,
                   &__uuidof(EnumeratorInterface),
                   EnumeratorExposedType,
                   EnumeratorCopyPolicy > EnumeratorType;

  IEnumVARIANT* pUnk = NULL;
  CComObject<EnumeratorType>* pEnum = NULL;
  VariantInit(pvarChildren);
  pvarChildren->vt = VT_EMPTY;

  nsCOMPtr<nsIAccessibleSelectable> select;
  nsAccessNode::QueryInterface(NS_GET_IID(nsIAccessibleSelectable), getter_AddRefs(select));

  if (select) {  // do we have an nsIAccessibleSelectable?
    // we have an accessible that can have children selected
    nsCOMPtr<nsIArray> selectedOptions;
    // gets the selected options as nsIAccessibles.
    select->GetSelectedChildren(getter_AddRefs(selectedOptions));
    if (selectedOptions) { // false if the select has no children or none are selected
      PRUint32 length;
      selectedOptions->GetLength(&length);
      CComVariant* optionArray = new CComVariant[length]; // needs to be a CComVariant to go into the EnumeratorType object

      // 1) Populate an array to store in the enumeration
      for (PRUint32 i = 0 ; i < length ; i++) {
        nsCOMPtr<nsIAccessible> tempAccess;
        selectedOptions->QueryElementAt(i, NS_GET_IID(nsIAccessible),
                                        getter_AddRefs(tempAccess));
        if (tempAccess) {
          optionArray[i] = NativeAccessible(tempAccess);
        }
      }

      // 2) Create and initialize the enumeration
      HRESULT hr = CComObject<EnumeratorType>::CreateInstance(&pEnum);
      pEnum->Init(&optionArray[0], &optionArray[length], NULL, AtlFlagCopy);
      pEnum->QueryInterface(IID_IEnumVARIANT, reinterpret_cast<void**>(&pUnk));
      delete [] optionArray; // clean up, the Init call copies the data (AtlFlagCopy)

      // 3) Put the enumerator in the VARIANT
      if (!pUnk)
        return NS_ERROR_FAILURE;
      pvarChildren->vt = VT_UNKNOWN;    // this must be VT_UNKNOWN for an IEnumVARIANT
      pvarChildren->punkVal = pUnk;
    }
  }
  return S_OK;
}

STDMETHODIMP nsAccessibleWrap::get_accDefaultAction(
      /* [optional][in] */ VARIANT varChild,
      /* [retval][out] */ BSTR __RPC_FAR *pszDefaultAction)
{
  *pszDefaultAction = NULL;
  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));
  if (xpAccessible) {
    nsAutoString defaultAction;
    if (NS_FAILED(xpAccessible->GetActionName(0, defaultAction)))
      return S_FALSE;

    *pszDefaultAction = ::SysAllocString(defaultAction.get());
  }

  return S_OK;
}

STDMETHODIMP nsAccessibleWrap::accSelect(
      /* [in] */ long flagsSelect,
      /* [optional][in] */ VARIANT varChild)
{
  // currently only handle focus and selection
  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));

  if (flagsSelect & (SELFLAG_TAKEFOCUS|SELFLAG_TAKESELECTION|SELFLAG_REMOVESELECTION))
  {
    if (flagsSelect & SELFLAG_TAKEFOCUS)
      xpAccessible->TakeFocus();

    if (flagsSelect & SELFLAG_TAKESELECTION)
      xpAccessible->TakeSelection();

    if (flagsSelect & SELFLAG_ADDSELECTION)
      xpAccessible->SetSelected(PR_TRUE);

    if (flagsSelect & SELFLAG_REMOVESELECTION)
      xpAccessible->SetSelected(PR_FALSE);

    if (flagsSelect & SELFLAG_EXTENDSELECTION)
      xpAccessible->ExtendSelection();

    return S_OK;
  }

  return E_FAIL;
}

STDMETHODIMP nsAccessibleWrap::accLocation(
      /* [out] */ long __RPC_FAR *pxLeft,
      /* [out] */ long __RPC_FAR *pyTop,
      /* [out] */ long __RPC_FAR *pcxWidth,
      /* [out] */ long __RPC_FAR *pcyHeight,
      /* [optional][in] */ VARIANT varChild)
{
  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));

  if (xpAccessible) {
    PRInt32 x, y, width, height;
    if (NS_FAILED(xpAccessible->GetBounds(&x, &y, &width, &height)))
      return E_FAIL;

    *pxLeft = x;
    *pyTop = y;
    *pcxWidth = width;
    *pcyHeight = height;
    return S_OK;
  }

  return E_FAIL;
}

STDMETHODIMP nsAccessibleWrap::accNavigate(
      /* [in] */ long navDir,
      /* [optional][in] */ VARIANT varStart,
      /* [retval][out] */ VARIANT __RPC_FAR *pvarEndUpAt)
{
  nsCOMPtr<nsIAccessible> xpAccessibleStart, xpAccessibleResult;
  GetXPAccessibleFor(varStart, getter_AddRefs(xpAccessibleStart));
  if (!xpAccessibleStart)
    return E_FAIL;

  VariantInit(pvarEndUpAt);
  PRUint32 xpRelation = 0;

  switch(navDir) {
    case NAVDIR_DOWN:
      xpAccessibleStart->GetAccessibleBelow(getter_AddRefs(xpAccessibleResult));
      break;
    case NAVDIR_FIRSTCHILD:
      if (!MustPrune(xpAccessibleStart)) {
        xpAccessibleStart->GetFirstChild(getter_AddRefs(xpAccessibleResult));
      }
      break;
    case NAVDIR_LASTCHILD:
      if (!MustPrune(xpAccessibleStart)) {
        xpAccessibleStart->GetLastChild(getter_AddRefs(xpAccessibleResult));
      }
      break;
    case NAVDIR_LEFT:
      xpAccessibleStart->GetAccessibleToLeft(getter_AddRefs(xpAccessibleResult));
      break;
    case NAVDIR_NEXT:
      xpAccessibleStart->GetNextSibling(getter_AddRefs(xpAccessibleResult));
      break;
    case NAVDIR_PREVIOUS:
      xpAccessibleStart->GetPreviousSibling(getter_AddRefs(xpAccessibleResult));
      break;
    case NAVDIR_RIGHT:
      xpAccessibleStart->GetAccessibleToRight(getter_AddRefs(xpAccessibleResult));
      break;
    case NAVDIR_UP:
      xpAccessibleStart->GetAccessibleAbove(getter_AddRefs(xpAccessibleResult));
      break;
    // MSAA relationship extensions to accNavigate
    case NAVRELATION_CONTROLLED_BY:    xpRelation = RELATION_CONTROLLED_BY;    break;
    case NAVRELATION_CONTROLLER_FOR:   xpRelation = RELATION_CONTROLLER_FOR;   break;
    case NAVRELATION_LABEL_FOR:        xpRelation = RELATION_LABEL_FOR;        break;
    case NAVRELATION_LABELLED_BY:      xpRelation = RELATION_LABELLED_BY;      break;
    case NAVRELATION_MEMBER_OF:        xpRelation = RELATION_MEMBER_OF;        break;
    case NAVRELATION_NODE_CHILD_OF:    xpRelation = RELATION_NODE_CHILD_OF;    break;
    case NAVRELATION_FLOWS_TO:         xpRelation = RELATION_FLOWS_TO;         break;
    case NAVRELATION_FLOWS_FROM:       xpRelation = RELATION_FLOWS_FROM;       break;
    case NAVRELATION_SUBWINDOW_OF:     xpRelation = RELATION_SUBWINDOW_OF;     break;
    case NAVRELATION_EMBEDS:           xpRelation = RELATION_EMBEDS;           break;
    case NAVRELATION_EMBEDDED_BY:      xpRelation = RELATION_EMBEDDED_BY;      break;
    case NAVRELATION_POPUP_FOR:        xpRelation = RELATION_POPUP_FOR;        break;
    case NAVRELATION_PARENT_WINDOW_OF: xpRelation = RELATION_PARENT_WINDOW_OF; break;
    case NAVRELATION_DEFAULT_BUTTON:   xpRelation = RELATION_DEFAULT_BUTTON;   break;
    case NAVRELATION_DESCRIBED_BY:     xpRelation = RELATION_DESCRIBED_BY;     break;
    case NAVRELATION_DESCRIPTION_FOR:  xpRelation = RELATION_DESCRIPTION_FOR;  break;
  }

  pvarEndUpAt->vt = VT_EMPTY;

  if (xpRelation) {
    nsresult rv = GetAccessibleRelated(xpRelation,
                                       getter_AddRefs(xpAccessibleResult));
    if (rv == NS_ERROR_NOT_IMPLEMENTED) {
      return E_NOTIMPL;
    }
  }

  if (xpAccessibleResult) {
    pvarEndUpAt->pdispVal = NativeAccessible(xpAccessibleResult);
    pvarEndUpAt->vt = VT_DISPATCH;
    return NS_OK;
  }
  return E_FAIL;
}

STDMETHODIMP nsAccessibleWrap::accHitTest(
      /* [in] */ long xLeft,
      /* [in] */ long yTop,
      /* [retval][out] */ VARIANT __RPC_FAR *pvarChild)
{
  VariantInit(pvarChild);

  // convert to window coords
  nsCOMPtr<nsIAccessible> xpAccessible;

  xLeft = xLeft;
  yTop = yTop;

  if (MustPrune(this)) {
    xpAccessible = this;
  }
  else {
    GetChildAtPoint(xLeft, yTop, getter_AddRefs(xpAccessible));
  }

  // if we got a child
  if (xpAccessible) {
    // if the child is us
    if (xpAccessible == NS_STATIC_CAST(nsIAccessible*, this)) {
      pvarChild->vt = VT_I4;
      pvarChild->lVal = CHILDID_SELF;
    } else { // its not create an Accessible for it.
      pvarChild->vt = VT_DISPATCH;
      pvarChild->pdispVal = NativeAccessible(xpAccessible);
      nsCOMPtr<nsIAccessNode> accessNode(do_QueryInterface(xpAccessible));
      NS_ASSERTION(accessNode, "Unable to QI to nsIAccessNode");
      nsCOMPtr<nsIDOMNode> domNode;
      accessNode->GetDOMNode(getter_AddRefs(domNode));
      if (!domNode) {
        // Has already been shut down
        pvarChild->vt = VT_EMPTY;
        return E_FAIL;
      }
    }
  } else {
    // no child at that point
    pvarChild->vt = VT_EMPTY;
    return E_FAIL;
  }

  return S_OK;
}

STDMETHODIMP nsAccessibleWrap::accDoDefaultAction(
      /* [optional][in] */ VARIANT varChild)
{
  nsCOMPtr<nsIAccessible> xpAccessible;
  GetXPAccessibleFor(varChild, getter_AddRefs(xpAccessible));

  if (!xpAccessible || FAILED(xpAccessible->DoAction(0))) {
    return E_FAIL;
  }
  return S_OK;
}

STDMETHODIMP nsAccessibleWrap::put_accName(
      /* [optional][in] */ VARIANT varChild,
      /* [in] */ BSTR szName)
{
  return E_NOTIMPL;
}

STDMETHODIMP nsAccessibleWrap::put_accValue(
      /* [optional][in] */ VARIANT varChild,
      /* [in] */ BSTR szValue)
{
  return E_NOTIMPL;
}

#include "mshtml.h"

STDMETHODIMP
nsAccessibleWrap::QueryService(REFGUID guidService, REFIID iid, void** ppv)
{
  /**
   * To get an ISimpleDOMNode, ISimpleDOMDocument or ISimpleDOMText
   * from an IAccessible you have to use IServiceProvider like this:
   * --------------------------------------------------------------
   * ISimpleDOMDocument *pAccDoc = NULL;
   * IServiceProvider *pServProv = NULL;
   * pAcc->QueryInterface(IID_IServiceProvider, (void**)&pServProv);
   * if (pServProv) {
   *   const GUID unused;
   *   pServProv->QueryService(unused, IID_ISimpleDOMDocument, (void**)&pAccDoc);
   *   pServProv->Release();
   * }
   */

  return QueryInterface(iid, ppv);
}


STDMETHODIMP
nsAccessibleWrap::Next(ULONG aNumElementsRequested, VARIANT FAR* pvar, ULONG FAR* aNumElementsFetched)
{
  // If there are two clients using this at the same time, and they are
  // each using a different mEnumVariant position it would be bad, because
  // we have only 1 object and can only keep of mEnumVARIANT position once

  // Children already cached via QI to IEnumVARIANT
  *aNumElementsFetched = 0;

  PRInt32 numChildren;
  GetChildCount(&numChildren);

  if (aNumElementsRequested <= 0 || !pvar ||
      mEnumVARIANTPosition >= numChildren) {
    return E_FAIL;
  }

  VARIANT varStart;
  VariantInit(&varStart);
  varStart.lVal = CHILDID_SELF;
  varStart.vt = VT_I4;

  accNavigate(NAVDIR_FIRSTCHILD, varStart, &pvar[0]);

  for (long childIndex = 0; pvar[*aNumElementsFetched].vt == VT_DISPATCH; ++childIndex) {
    PRBool wasAccessibleFetched = PR_FALSE;
    nsAccessibleWrap *msaaAccessible =
      NS_STATIC_CAST(nsAccessibleWrap*, pvar[*aNumElementsFetched].pdispVal);
    if (!msaaAccessible)
      break;
    if (childIndex >= mEnumVARIANTPosition) {
      if (++*aNumElementsFetched >= aNumElementsRequested)
        break;
      wasAccessibleFetched = PR_TRUE;
    }
    msaaAccessible->accNavigate(NAVDIR_NEXT, varStart, &pvar[*aNumElementsFetched] );
    if (!wasAccessibleFetched)
      msaaAccessible->nsAccessNode::Release(); // this accessible will not be received by the caller
  }

  mEnumVARIANTPosition += NS_STATIC_CAST(PRUint16, *aNumElementsFetched);
  return NOERROR;
}

STDMETHODIMP
nsAccessibleWrap::Skip(ULONG aNumElements)
{
  mEnumVARIANTPosition += NS_STATIC_CAST(PRUint16, aNumElements);

  PRInt32 numChildren;
  GetChildCount(&numChildren);

  if (mEnumVARIANTPosition > numChildren)
  {
    mEnumVARIANTPosition = numChildren;
    return S_FALSE;
  }
  return NOERROR;
}

STDMETHODIMP
nsAccessibleWrap::Reset(void)
{
  mEnumVARIANTPosition = 0;
  return NOERROR;
}

STDMETHODIMP
nsAccessibleWrap::Clone(IEnumVARIANT FAR* FAR* ppenum)
{
  // Clone could be bad, the cloned items aren't tracked for shutdown
  // Then again, as long as the client releases the items in time, we're okay
  *ppenum = nsnull;

  nsAccessibleWrap *accessibleWrap = new nsAccessibleWrap(mDOMNode, mWeakShell);
  if (!accessibleWrap)
    return E_FAIL;

  IAccessible *msaaAccessible = NS_STATIC_CAST(IAccessible*, accessibleWrap);
  msaaAccessible->AddRef();
  QueryInterface(IID_IEnumVARIANT, (void**)ppenum);
  if (*ppenum)
    (*ppenum)->Skip(mEnumVARIANTPosition); // QI addrefed
  msaaAccessible->Release();

  return NOERROR;
}


// For IDispatch support
STDMETHODIMP
nsAccessibleWrap::GetTypeInfoCount(UINT *p)
{
  *p = 0;
  return E_NOTIMPL;
}

// For IDispatch support
STDMETHODIMP nsAccessibleWrap::GetTypeInfo(UINT i, LCID lcid, ITypeInfo **ppti)
{
  *ppti = 0;
  return E_NOTIMPL;
}

// For IDispatch support
STDMETHODIMP
nsAccessibleWrap::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
                           UINT cNames, LCID lcid, DISPID *rgDispId)
{
  return E_NOTIMPL;
}

// For IDispatch support
STDMETHODIMP nsAccessibleWrap::Invoke(DISPID dispIdMember, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS *pDispParams,
    VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
  return E_NOTIMPL;
}


NS_IMETHODIMP nsAccessibleWrap::GetNativeInterface(void **aOutAccessible)
{
  *aOutAccessible = NS_STATIC_CAST(IAccessible*, this);
  NS_ADDREF_THIS();
  return NS_OK;
}


//------- Helper methods ---------

IDispatch *nsAccessibleWrap::NativeAccessible(nsIAccessible *aXPAccessible)
{
  nsCOMPtr<nsIAccessibleWin32Object> accObject(do_QueryInterface(aXPAccessible));
  if (accObject) {
    void* hwnd;
    accObject->GetHwnd(&hwnd);
    if (hwnd) {
      IDispatch *retval = nsnull;
      AccessibleObjectFromWindow(NS_REINTERPRET_CAST(HWND, hwnd),
        OBJID_WINDOW, IID_IAccessible, (void **) &retval);
      return retval;
    }
  }

  IAccessible *msaaAccessible;
  aXPAccessible->GetNativeInterface((void**)&msaaAccessible);

  return NS_STATIC_CAST(IDispatch*, msaaAccessible);
}


void nsAccessibleWrap::GetXPAccessibleFor(const VARIANT& aVarChild, nsIAccessible **aXPAccessible)
{
  *aXPAccessible = nsnull;
  if (!mWeakShell)
    return; // Fail, we don't want to do anything after we've shut down

  // if its us real easy - this seems to always be the case
  if (aVarChild.lVal == CHILDID_SELF) {
    *aXPAccessible = NS_STATIC_CAST(nsIAccessible*, this);
  }
  else if (MustPrune(this)) {
    return;
  }
  else {
    // XXX It is rare to use a VARIANT with a child num
    // so optimizing this is not a priority
    // We can come back it do it later, if there are perf problems
    // with a specific assistive technology
    nsCOMPtr<nsIAccessible> xpAccessible, nextAccessible;
    GetFirstChild(getter_AddRefs(xpAccessible));
    for (PRInt32 index = 0; xpAccessible; index ++) {
      if (!xpAccessible)
        break; // Failed
      if (aVarChild.lVal == index) {
        *aXPAccessible = xpAccessible;
        break;
      }
      nextAccessible = xpAccessible;
      nextAccessible->GetNextSibling(getter_AddRefs(xpAccessible));
    }
  }
  NS_IF_ADDREF(*aXPAccessible);
}

