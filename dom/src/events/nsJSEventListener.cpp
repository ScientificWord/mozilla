/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
#include "nsJSEventListener.h"
#include "nsJSUtils.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsIServiceManager.h"
#include "nsIScriptSecurityManager.h"
#include "nsIScriptContext.h"
#include "nsIScriptGlobalObject.h"
#include "nsIScriptRuntime.h"
#include "nsIXPConnect.h"
#include "nsIPrivateDOMEvent.h"
#include "nsGUIEvent.h"
#include "nsContentUtils.h"
#include "nsDOMScriptObjectHolder.h"
#include "nsIMutableArray.h"
#include "nsVariant.h"


#ifdef NS_DEBUG
#include "nsIJSContextStack.h"
#include "nsDOMJSUtils.h"

#include "nspr.h" // PR_fprintf

class EventListenerCounter
{
public:
  ~EventListenerCounter() {
  }
};

static EventListenerCounter sEventListenerCounter;
#endif

/*
 * nsJSEventListener implementation
 */
nsJSEventListener::nsJSEventListener(nsIScriptContext *aContext,
                                     void *aScopeObject,
                                     nsISupports *aTarget)
  : nsIJSEventListener(aContext, aScopeObject, aTarget),
    mReturnResult(nsReturnResult_eNotSet)
{
  // aScopeObject is the inner window's JS object, which we need to lock
  // until we are done with it.
  NS_ASSERTION(aScopeObject && aContext,
               "EventListener with no context or scope?");
  nsContentUtils::HoldScriptObject(aContext->GetScriptTypeID(), this,
                                   &NS_CYCLE_COLLECTION_NAME(nsJSEventListener),
                                   aScopeObject, PR_FALSE);
}

nsJSEventListener::~nsJSEventListener() 
{
  if (mContext)
    nsContentUtils::DropScriptObjects(mContext->GetScriptTypeID(), this,
                                &NS_CYCLE_COLLECTION_NAME(nsJSEventListener));
}

NS_IMPL_CYCLE_COLLECTION_CLASS(nsJSEventListener)
NS_IMPL_CYCLE_COLLECTION_ROOT_BEGIN(nsJSEventListener)
  if (tmp->mContext &&
      tmp->mContext->GetScriptTypeID() == nsIProgrammingLanguage::JAVASCRIPT) {
    NS_DROP_JS_OBJECTS(tmp, nsJSEventListener);
    tmp->mScopeObject = nsnull;
  }
NS_IMPL_CYCLE_COLLECTION_ROOT_END
NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(nsJSEventListener)
  NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(mTarget)
  if (tmp->mContext) {
    if (tmp->mScopeObject) {
      nsContentUtils::DropScriptObjects(tmp->mContext->GetScriptTypeID(), tmp,
                                  &NS_CYCLE_COLLECTION_NAME(nsJSEventListener));
      tmp->mScopeObject = nsnull;
    }
    NS_IMPL_CYCLE_COLLECTION_UNLINK_NSCOMPTR(mContext)
  }
NS_IMPL_CYCLE_COLLECTION_UNLINK_END
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(nsJSEventListener)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mTarget)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mContext)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_SCRIPT_OBJECTS
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_IMPL_CYCLE_COLLECTION_TRACE_BEGIN(nsJSEventListener)
  NS_IMPL_CYCLE_COLLECTION_TRACE_MEMBER_CALLBACK(tmp->mContext->GetScriptTypeID(),
                                                 mScopeObject)
NS_IMPL_CYCLE_COLLECTION_TRACE_END

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(nsJSEventListener)
  NS_INTERFACE_MAP_ENTRY(nsIDOMEventListener)
  NS_INTERFACE_MAP_ENTRY(nsIJSEventListener)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIDOMEventListener)
NS_INTERFACE_MAP_END

NS_IMPL_CYCLE_COLLECTING_ADDREF_AMBIGUOUS(nsJSEventListener, nsIDOMEventListener)
NS_IMPL_CYCLE_COLLECTING_RELEASE_AMBIGUOUS(nsJSEventListener, nsIDOMEventListener)

//static nsString onPrefix = "on";

void
nsJSEventListener::SetEventName(nsIAtom* aName)
{
  mEventName = aName;
}

nsresult
nsJSEventListener::HandleEvent(nsIDOMEvent* aEvent)
{
  nsresult rv;
  nsCOMPtr<nsIArray> iargv;
  nsAutoString eventString;
  nsCOMPtr<nsIAtom> atomName;

  if (!mEventName) {
    if (NS_OK != aEvent->GetType(eventString)) {
      //JS can't handle this event yet or can't handle it at all
      return NS_OK;
    }
    //if (mReturnResult == nsReturnResult_eNotSet) {
      if (eventString.EqualsLiteral("error") ||
          eventString.EqualsLiteral("mouseover")) {
        mReturnResult = nsReturnResult_eReverseReturnResult;
      }
      else {
        mReturnResult = nsReturnResult_eDoNotReverseReturnResult;
      }
    //}
    eventString.Assign(NS_LITERAL_STRING("on") + eventString);
	atomName = do_GetAtom(eventString);
  }
  else {
    mEventName->ToString(eventString);
	atomName = mEventName;
  }


  nsScriptObjectHolder funcval(mContext);
  rv = mContext->GetBoundEventHandler(mTarget, mScopeObject, atomName,
                                      funcval);
  NS_ENSURE_SUCCESS(rv, rv);
  if (!funcval)
    return NS_OK;

  PRBool handledScriptError = PR_FALSE;
  if (eventString.EqualsLiteral("onerror")) {
    nsCOMPtr<nsIPrivateDOMEvent> priv(do_QueryInterface(aEvent));
    NS_ENSURE_TRUE(priv, NS_ERROR_UNEXPECTED);

    nsEvent* event;
    priv->GetInternalNSEvent(&event);
    if (event->message == NS_LOAD_ERROR &&
        event->eventStructType == NS_SCRIPT_ERROR_EVENT) {
      nsScriptErrorEvent *scriptEvent =
        static_cast<nsScriptErrorEvent*>(event);
      // Create a temp argv for the error event.
      nsCOMPtr<nsIMutableArray> tempargv = 
        do_CreateInstance(NS_ARRAY_CONTRACTID, &rv);
      if (NS_FAILED(rv)) return rv;
      // Append the event args.
      nsCOMPtr<nsIWritableVariant>
          var(do_CreateInstance(NS_VARIANT_CONTRACTID, &rv));
      NS_ENSURE_SUCCESS(rv, rv);
      rv = var->SetAsWString(scriptEvent->errorMsg);
      NS_ENSURE_SUCCESS(rv, rv);
      rv = tempargv->AppendElement(var, PR_FALSE);
      NS_ENSURE_SUCCESS(rv, rv);
      // filename
      var = do_CreateInstance(NS_VARIANT_CONTRACTID, &rv);
      NS_ENSURE_SUCCESS(rv, rv);
      rv = var->SetAsWString(scriptEvent->fileName);
      NS_ENSURE_SUCCESS(rv, rv);
      rv = tempargv->AppendElement(var, PR_FALSE);
      NS_ENSURE_SUCCESS(rv, rv);
      // line number
      var = do_CreateInstance(NS_VARIANT_CONTRACTID, &rv);
      NS_ENSURE_SUCCESS(rv, rv);
      rv = var->SetAsUint32(scriptEvent->lineNr);
      NS_ENSURE_SUCCESS(rv, rv);
      rv = tempargv->AppendElement(var, PR_FALSE);
      NS_ENSURE_SUCCESS(rv, rv);

      // And set the real argv
      iargv = do_QueryInterface(tempargv);

      handledScriptError = PR_TRUE;
    }
  }

  if (!handledScriptError) {
    nsCOMPtr<nsIMutableArray> tempargv = 
      do_CreateInstance(NS_ARRAY_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;
    NS_ENSURE_TRUE(tempargv != nsnull, NS_ERROR_OUT_OF_MEMORY);
    rv = tempargv->AppendElement(aEvent, PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);
    iargv = do_QueryInterface(tempargv);
  }

  // mContext is the same context which event listener manager pushes
  // to JS context stack.
#ifdef NS_DEBUG
  JSContext* cx = nsnull;
  nsCOMPtr<nsIJSContextStack> stack =
    do_GetService("@mozilla.org/js/xpc/ContextStack;1");
  NS_ASSERTION(stack && NS_SUCCEEDED(stack->Peek(&cx)) && cx &&
               GetScriptContextFromJSContext(cx) == mContext,
               "JSEventListener has wrong script context?");
#endif
  nsCOMPtr<nsIVariant> vrv;
  rv = mContext->CallEventHandler(mTarget, mScopeObject, funcval, iargv,
                                  getter_AddRefs(vrv));

  if (NS_SUCCEEDED(rv)) {
    PRUint16 dataType = nsIDataType::VTYPE_VOID;
    if (vrv)
      vrv->GetDataType(&dataType);
    if (eventString.EqualsLiteral("onbeforeunload")) {
      nsCOMPtr<nsIPrivateDOMEvent> priv(do_QueryInterface(aEvent));
      NS_ENSURE_TRUE(priv, NS_ERROR_UNEXPECTED);

      nsEvent* event;
      priv->GetInternalNSEvent(&event);
      NS_ENSURE_TRUE(event && event->message == NS_BEFORE_PAGE_UNLOAD,
                     NS_ERROR_UNEXPECTED);

      nsBeforePageUnloadEvent *beforeUnload =
        static_cast<nsBeforePageUnloadEvent *>(event);

      if (dataType != nsIDataType::VTYPE_VOID) {
        aEvent->PreventDefault();

        // Set the text in the beforeUnload event as long as it wasn't
        // already set (through event.returnValue, which takes
        // precedence over a value returned from a JS function in IE)
        if ((dataType == nsIDataType::VTYPE_DOMSTRING ||
             dataType == nsIDataType::VTYPE_CHAR_STR ||
             dataType == nsIDataType::VTYPE_WCHAR_STR ||
             dataType == nsIDataType::VTYPE_STRING_SIZE_IS ||
             dataType == nsIDataType::VTYPE_WSTRING_SIZE_IS ||
             dataType == nsIDataType::VTYPE_CSTRING ||
             dataType == nsIDataType::VTYPE_ASTRING)
            && beforeUnload->text.IsEmpty()) {
          vrv->GetAsDOMString(beforeUnload->text);
        }
      }
    } else if (dataType == nsIDataType::VTYPE_BOOL) {
      // If the handler returned false and its sense is not reversed,
      // or the handler returned true and its sense is reversed from
      // the usual (false means cancel), then prevent default.
      PRBool brv;
      if (NS_SUCCEEDED(vrv->GetAsBool(&brv)) &&
          brv == (mReturnResult == nsReturnResult_eReverseReturnResult)) {
        aEvent->PreventDefault();
      }
    }
  }

  return rv;
}

/*
 * Factory functions
 */

nsresult
NS_NewJSEventListener(nsIScriptContext *aContext, void *aScopeObject,
                      nsISupports*aTarget, nsIDOMEventListener ** aReturn)
{
  nsJSEventListener* it =
    new nsJSEventListener(aContext, aScopeObject, aTarget);
  if (!it) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  NS_ADDREF(*aReturn = it);

  return NS_OK;
}
