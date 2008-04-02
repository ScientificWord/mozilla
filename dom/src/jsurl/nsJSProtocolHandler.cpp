/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim: set ts=4 sw=4 et tw=78: */
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
#include "nsCOMPtr.h"
#include "nsAutoPtr.h"
#include "jsapi.h"
#include "nsCRT.h"
#include "nsDOMError.h"
#include "nsXPIDLString.h"
#include "nsReadableUtils.h"
#include "nsJSProtocolHandler.h"
#include "nsStringStream.h"
#include "nsNetUtil.h"

#include "nsIComponentManager.h"
#include "nsIGenericFactory.h"
#include "nsIServiceManager.h"
#include "nsIURI.h"
#include "nsIScriptContext.h"
#include "nsIScriptGlobalObject.h"
#include "nsIScriptGlobalObjectOwner.h"
#include "nsIPrincipal.h"
#include "nsIScriptSecurityManager.h"
#include "nsIInterfaceRequestor.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIWindowMediator.h"
#include "nsPIDOMWindow.h"
#include "nsIDOMDocument.h"
#include "nsIConsoleService.h"
#include "nsXPIDLString.h"
#include "prprf.h"
#include "nsEscape.h"
#include "nsIWebNavigation.h"
#include "nsIDocShell.h"
#include "nsIContentViewer.h"
#include "nsIXPConnect.h"
#include "nsContentUtils.h"
#include "nsJSUtils.h"
#include "nsThreadUtils.h"
#include "nsIJSContextStack.h"
#include "nsIScriptChannel.h"
#include "nsIDocument.h"

class nsJSThunk : public nsIInputStream
{
public:
    nsJSThunk();

    NS_DECL_ISUPPORTS
    NS_FORWARD_SAFE_NSIINPUTSTREAM(mInnerStream)

    nsresult Init(nsIURI* uri);
    nsresult EvaluateScript(nsIChannel *aChannel,
                            PopupControlState aPopupState,
                            PRUint32 aExecutionPolicy,
                            nsPIDOMWindow *aOriginalInnerWindow);

protected:
    virtual ~nsJSThunk();

    nsCOMPtr<nsIURI>            mURI;
    nsCOMPtr<nsIInputStream>    mInnerStream;
};

//
// nsISupports implementation...
//
NS_IMPL_THREADSAFE_ISUPPORTS1(nsJSThunk, nsIInputStream)


nsJSThunk::nsJSThunk()
{
}

nsJSThunk::~nsJSThunk()
{
}

nsresult nsJSThunk::Init(nsIURI* uri)
{
    NS_ENSURE_ARG_POINTER(uri);

    mURI = uri;
    return NS_OK;
}

static PRBool
IsISO88591(const nsString& aString)
{
    for (nsString::const_char_iterator c = aString.BeginReading(),
                                   c_end = aString.EndReading();
         c < c_end; ++c) {
        if (*c > 255)
            return PR_FALSE;
    }
    return PR_TRUE;
}

static
nsIScriptGlobalObject* GetGlobalObject(nsIChannel* aChannel)
{
    // Get the global object owner from the channel
    nsCOMPtr<nsIScriptGlobalObjectOwner> globalOwner;
    NS_QueryNotificationCallbacks(aChannel, globalOwner);
    NS_ASSERTION(globalOwner, 
                 "Unable to get an nsIScriptGlobalObjectOwner from the "
                 "channel!");
    if (!globalOwner) {
        return nsnull;
    }

    // So far so good: get the script context from its owner.
    nsIScriptGlobalObject* global = globalOwner->GetScriptGlobalObject();

    NS_ASSERTION(global,
                 "Unable to get an nsIScriptGlobalObject from the "
                 "ScriptGlobalObjectOwner!");
    return global;
}

nsresult nsJSThunk::EvaluateScript(nsIChannel *aChannel,
                                   PopupControlState aPopupState,
                                   PRUint32 aExecutionPolicy,
                                   nsPIDOMWindow *aOriginalInnerWindow)
{
    if (aExecutionPolicy == nsIScriptChannel::NO_EXECUTION) {
        // Nothing to do here.
        return NS_ERROR_DOM_RETVAL_UNDEFINED;
    }
    
    NS_ENSURE_ARG_POINTER(aChannel);

    // Get principal of code for execution
    nsCOMPtr<nsISupports> owner;
    aChannel->GetOwner(getter_AddRefs(owner));
    nsCOMPtr<nsIPrincipal> principal = do_QueryInterface(owner);
    if (!principal) {
        // No execution without a principal!
        NS_ASSERTION(!owner, "Non-principal owner?");
        NS_WARNING("No principal to execute JS with");
        return NS_ERROR_DOM_RETVAL_UNDEFINED;
    }

    // Get the script string to evaluate...
    nsCAutoString script;
    nsresult rv = mURI->GetPath(script);
    if (NS_FAILED(rv)) return rv;

    // Get the global object we should be running on.
    nsIScriptGlobalObject* global = GetGlobalObject(aChannel);
    if (!global) {
        return NS_ERROR_FAILURE;
    }

    nsCOMPtr<nsPIDOMWindow> win(do_QueryInterface(global));

    // Push our popup control state
    nsAutoPopupStatePusher popupStatePusher(win, aPopupState);

    // Make sure we still have the same inner window as we used to.
    nsPIDOMWindow *innerWin = win->GetCurrentInnerWindow();

    if (innerWin != aOriginalInnerWindow) {
        return NS_ERROR_UNEXPECTED;
    }

    nsCOMPtr<nsIScriptGlobalObject> innerGlobal = do_QueryInterface(innerWin);

    JSObject *globalJSObject = innerGlobal->GetGlobalJSObject();

    nsCOMPtr<nsIDOMWindow> domWindow(do_QueryInterface(global, &rv));
    if (NS_FAILED(rv)) {
        return NS_ERROR_FAILURE;
    }

    // So far so good: get the script context from its owner.
    nsCOMPtr<nsIScriptContext> scriptContext = global->GetContext();
    if (!scriptContext)
        return NS_ERROR_FAILURE;

    // Unescape the script
    NS_UnescapeURL(script);

    // Get the url.
    nsCAutoString url;
    rv = mURI->GetSpec(url);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIScriptSecurityManager> securityManager;
    securityManager = do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv))
        return rv;

    PRBool useSandbox =
        (aExecutionPolicy == nsIScriptChannel::EXECUTE_IN_SANDBOX);

    if (!useSandbox) {
        //-- Don't outside a sandbox unless the script principal subsumes the
        //   principal of the context.
        nsCOMPtr<nsIPrincipal> objectPrincipal;
        rv = securityManager->
            GetObjectPrincipal((JSContext*)scriptContext->GetNativeContext(),
                               globalJSObject,
                               getter_AddRefs(objectPrincipal));
        if (NS_FAILED(rv))
            return rv;

        PRBool subsumes;
        rv = principal->Subsumes(objectPrincipal, &subsumes);
        if (NS_FAILED(rv))
            return rv;

        useSandbox = !subsumes;
    }

    nsString result;
    PRBool isUndefined;

    // Finally, we have everything needed to evaluate the expression.

    if (useSandbox) {
        // We were asked to use a sandbox, or the channel owner isn't allowed
        // to execute in this context.  Evaluate the javascript URL in a
        // sandbox to prevent it from accessing data it doesn't have
        // permissions to access.

        // First check to make sure it's OK to evaluate this script to
        // start with.  For example, script could be disabled.
        JSContext *cx = (JSContext*)scriptContext->GetNativeContext();
        JSAutoRequest ar(cx);

        PRBool ok;
        rv = securityManager->CanExecuteScripts(cx, principal, &ok);
        if (NS_FAILED(rv)) {
            return rv;
        }

        if (!ok) {
            // Treat this as returning undefined from the script.  That's what
            // nsJSContext does.
            return NS_ERROR_DOM_RETVAL_UNDEFINED;
        }

        nsIXPConnect *xpc = nsContentUtils::XPConnect();

        nsCOMPtr<nsIXPConnectJSObjectHolder> sandbox;
        rv = xpc->CreateSandbox(cx, principal, getter_AddRefs(sandbox));
        NS_ENSURE_SUCCESS(rv, rv);

        jsval rval = JSVAL_VOID;
        nsAutoGCRoot root(&rval, &rv);
        if (NS_FAILED(rv)) {
            return rv;
        }

        // Push our JSContext on the context stack so the JS_ValueToString call (and
        // JS_ReportPendingException, if relevant) will use the principal of cx.
        // Note that we do this as late as possible to make popping simpler.
        nsCOMPtr<nsIJSContextStack> stack =
            do_GetService("@mozilla.org/js/xpc/ContextStack;1", &rv);
        if (NS_SUCCEEDED(rv)) {
            rv = stack->Push(cx);
        }
        if (NS_FAILED(rv)) {
            return rv;
        }

        rv = xpc->EvalInSandboxObject(NS_ConvertUTF8toUTF16(script), cx,
                                      sandbox, PR_TRUE, &rval);

        // Propagate and report exceptions that happened in the
        // sandbox.
        if (JS_IsExceptionPending(cx)) {
            JS_ReportPendingException(cx);
            isUndefined = PR_TRUE;
        } else {
            isUndefined = rval == JSVAL_VOID;
        }

        if (!isUndefined && NS_SUCCEEDED(rv)) {
            NS_ASSERTION(JSVAL_IS_STRING(rval), "evalInSandbox is broken");
            result = nsDependentJSString(JSVAL_TO_STRING(rval));
        }

        stack->Pop(nsnull);
    } else {
        // No need to use the sandbox, evaluate the script directly in
        // the given scope.
        rv = scriptContext->EvaluateString(NS_ConvertUTF8toUTF16(script),
                                           globalJSObject, // obj
                                           principal,
                                           url.get(),      // url
                                           1,              // line no
                                           nsnull,
                                           &result,
                                           &isUndefined);

        // If there's an error on cx as a result of that call, report
        // it now -- either we're just running under the event loop,
        // so we shouldn't propagate JS exceptions out of here, or we
        // can't be sure that our caller is JS (and if it's not we'll
        // lose the error), or it might be JS that then proceeds to
        // cause an error of its own (which will also make us lose
        // this error).
        JSContext *cx = (JSContext*)scriptContext->GetNativeContext();
        JSAutoRequest ar(cx);
        ::JS_ReportPendingException(cx);
    }
    
    if (NS_FAILED(rv)) {
        rv = NS_ERROR_MALFORMED_URI;
    }
    else if (isUndefined) {
        rv = NS_ERROR_DOM_RETVAL_UNDEFINED;
    }
    else {
        char *bytes;
        PRUint32 bytesLen;
        NS_NAMED_LITERAL_CSTRING(isoCharset, "ISO-8859-1");
        NS_NAMED_LITERAL_CSTRING(utf8Charset, "UTF-8");
        const nsCString *charset;
        if (IsISO88591(result)) {
            // For compatibility, if the result is ISO-8859-1, we use
            // ISO-8859-1, so that people can compatibly create images
            // using javascript: URLs.
            bytes = ToNewCString(result);
            bytesLen = result.Length();
            charset = &isoCharset;
        }
        else {
            bytes = ToNewUTF8String(result, &bytesLen);
            charset = &utf8Charset;
        }
        aChannel->SetContentCharset(*charset);
        if (bytes)
            rv = NS_NewByteInputStream(getter_AddRefs(mInnerStream),
                                       bytes, bytesLen,
                                       NS_ASSIGNMENT_ADOPT);
        else
            rv = NS_ERROR_OUT_OF_MEMORY;
    }

    return rv;
}

////////////////////////////////////////////////////////////////////////////////

class nsJSChannel : public nsIChannel,
                    public nsIStreamListener,
                    public nsIScriptChannel
{
public:
    nsJSChannel();

    NS_DECL_ISUPPORTS
    NS_DECL_NSIREQUEST
    NS_DECL_NSICHANNEL
    NS_DECL_NSIREQUESTOBSERVER
    NS_DECL_NSISTREAMLISTENER
    NS_DECL_NSISCRIPTCHANNEL

    nsresult Init(nsIURI *aURI);

    // Actually evaluate the script.
    void EvaluateScript();
    
protected:
    virtual ~nsJSChannel();

    nsresult StopAll();

    void NotifyListener();

    void CleanupStrongRefs();
    
protected:
    nsCOMPtr<nsIChannel>    mStreamChannel;
    nsCOMPtr<nsIStreamListener> mListener;  // Our final listener
    nsCOMPtr<nsISupports> mContext; // The context passed to AsyncOpen
    nsCOMPtr<nsPIDOMWindow> mOriginalInnerWindow;  // The inner window our load
                                                   // started against.
    // If we blocked onload on a document in AsyncOpen, this is the document we
    // did it on.
    nsCOMPtr<nsIDocument>   mDocumentOnloadBlockedOn;

    nsresult mStatus; // Our status

    nsLoadFlags             mLoadFlags;
    nsLoadFlags             mActualLoadFlags; // See AsyncOpen

    nsRefPtr<nsJSThunk>     mIOThunk;
    PopupControlState       mPopupState;
    PRUint32                mExecutionPolicy;
    PRPackedBool            mIsAsync;
    PRPackedBool            mIsActive;
    PRPackedBool            mOpenedStreamChannel;
};

nsJSChannel::nsJSChannel() :
    mStatus(NS_OK),
    mLoadFlags(LOAD_NORMAL),
    mActualLoadFlags(LOAD_NORMAL),
    mPopupState(openOverridden),
    mExecutionPolicy(EXECUTE_IN_SANDBOX),
    mIsAsync(PR_TRUE),
    mIsActive(PR_FALSE),
    mOpenedStreamChannel(PR_FALSE)
{
}

nsJSChannel::~nsJSChannel()
{
}

nsresult nsJSChannel::StopAll()
{
    nsresult rv = NS_ERROR_UNEXPECTED;
    nsCOMPtr<nsIWebNavigation> webNav;
    NS_QueryNotificationCallbacks(mStreamChannel, webNav);

    NS_ASSERTION(webNav, "Can't get nsIWebNavigation from channel!");
    if (webNav) {
        rv = webNav->Stop(nsIWebNavigation::STOP_ALL);
    }

    return rv;
}

nsresult nsJSChannel::Init(nsIURI *aURI)
{
    nsresult rv;

    // Create the nsIStreamIO layer used by the nsIStreamIOChannel.
    mIOThunk = new nsJSThunk();
    if (!mIOThunk)
        return NS_ERROR_OUT_OF_MEMORY;

    // Create a stock input stream channel...
    // Remember, until AsyncOpen is called, the script will not be evaluated
    // and the underlying Input Stream will not be created...
    nsCOMPtr<nsIChannel> channel;

    // If the resultant script evaluation actually does return a value, we
    // treat it as html.
    rv = NS_NewInputStreamChannel(getter_AddRefs(channel), aURI, mIOThunk,
                                  NS_LITERAL_CSTRING("text/html"));
    if (NS_FAILED(rv)) return rv;

    rv = mIOThunk->Init(aURI);
    if (NS_SUCCEEDED(rv)) {
        mStreamChannel = channel;
    }

    return rv;
}

//
// nsISupports implementation...
//

NS_IMPL_ADDREF(nsJSChannel)
NS_IMPL_RELEASE(nsJSChannel)

NS_INTERFACE_MAP_BEGIN(nsJSChannel)
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIChannel)
    NS_INTERFACE_MAP_ENTRY(nsIRequest)
    NS_INTERFACE_MAP_ENTRY(nsIChannel)
    NS_INTERFACE_MAP_ENTRY(nsIRequestObserver)
    NS_INTERFACE_MAP_ENTRY(nsIStreamListener)
    NS_INTERFACE_MAP_ENTRY(nsIScriptChannel)
NS_INTERFACE_MAP_END

//
// nsIRequest implementation...
//

NS_IMETHODIMP
nsJSChannel::GetName(nsACString &aResult)
{
    return mStreamChannel->GetName(aResult);
}

NS_IMETHODIMP
nsJSChannel::IsPending(PRBool *aResult)
{
    *aResult = mIsActive;
    return NS_OK;
}

NS_IMETHODIMP
nsJSChannel::GetStatus(nsresult *aResult)
{
    if (NS_SUCCEEDED(mStatus) && mOpenedStreamChannel) {
        return mStreamChannel->GetStatus(aResult);
    }
    
    *aResult = mStatus;
        
    return NS_OK;
}

NS_IMETHODIMP
nsJSChannel::Cancel(nsresult aStatus)
{
    mStatus = aStatus;

    if (mOpenedStreamChannel) {
        mStreamChannel->Cancel(aStatus);
    }
    
    return NS_OK;
}

NS_IMETHODIMP
nsJSChannel::Suspend()
{
    return mStreamChannel->Suspend();
}

NS_IMETHODIMP
nsJSChannel::Resume()
{
    return mStreamChannel->Resume();
}

//
// nsIChannel implementation
//

NS_IMETHODIMP
nsJSChannel::GetOriginalURI(nsIURI * *aURI)
{
    return mStreamChannel->GetOriginalURI(aURI);
}

NS_IMETHODIMP
nsJSChannel::SetOriginalURI(nsIURI *aURI)
{
    return mStreamChannel->SetOriginalURI(aURI);
}

NS_IMETHODIMP
nsJSChannel::GetURI(nsIURI * *aURI)
{
    return mStreamChannel->GetURI(aURI);
}

NS_IMETHODIMP
nsJSChannel::Open(nsIInputStream **aResult)
{
    nsresult rv = mIOThunk->EvaluateScript(mStreamChannel, mPopupState,
                                           mExecutionPolicy,
                                           mOriginalInnerWindow);
    NS_ENSURE_SUCCESS(rv, rv);

    return mStreamChannel->Open(aResult);
}

NS_IMETHODIMP
nsJSChannel::AsyncOpen(nsIStreamListener *aListener, nsISupports *aContext)
{
    NS_ENSURE_ARG(aListener);

    // First make sure that we have a usable inner window; we'll want to make
    // sure that we execute against that inner and no other.
    nsIScriptGlobalObject* global = GetGlobalObject(this);
    if (!global) {
        return NS_ERROR_NOT_AVAILABLE;
    }

    nsCOMPtr<nsPIDOMWindow> win(do_QueryInterface(global));
    NS_ASSERTION(win, "Our global is not a window??");

    // Make sure we create a new inner window if one doesn't already exist (see
    // bug 306630).
    mOriginalInnerWindow = win->EnsureInnerWindow();
    if (!mOriginalInnerWindow) {
        return NS_ERROR_NOT_AVAILABLE;
    }
    
    mListener = aListener;
    mContext = aContext;

    mIsActive = PR_TRUE;

    // Temporarily set the LOAD_BACKGROUND flag to suppress load group observer
    // notifications (and hence nsIWebProgressListener notifications) from
    // being dispatched.  This is required since we suppress LOAD_DOCUMENT_URI,
    // which means that the DocLoader would not generate document start and
    // stop notifications (see bug 257875).
    mActualLoadFlags = mLoadFlags;
    mLoadFlags |= LOAD_BACKGROUND;

    // Add the javascript channel to its loadgroup so that we know if
    // network loads were canceled or not...
    nsCOMPtr<nsILoadGroup> loadGroup;
    mStreamChannel->GetLoadGroup(getter_AddRefs(loadGroup));
    if (loadGroup) {
        nsresult rv = loadGroup->AddRequest(this, nsnull);
        if (NS_FAILED(rv)) {
            mIsActive = PR_FALSE;
            CleanupStrongRefs();
            return rv;
        }
    }

    mDocumentOnloadBlockedOn =
        do_QueryInterface(mOriginalInnerWindow->GetExtantDocument());
    if (mDocumentOnloadBlockedOn) {
        // If we're a document channel, we need to actually block onload on our
        // _parent_ document.  This is because we don't actually set our
        // LOAD_DOCUMENT_URI flag, so a docloader we're loading in as the
        // document channel will claim to not be busy, and our parent's onload
        // could fire too early.
        nsLoadFlags loadFlags;
        mStreamChannel->GetLoadFlags(&loadFlags);
        if (loadFlags & LOAD_DOCUMENT_URI) {
            mDocumentOnloadBlockedOn =
                mDocumentOnloadBlockedOn->GetParentDocument();
        }
    }
    if (mDocumentOnloadBlockedOn) {
        mDocumentOnloadBlockedOn->BlockOnload();
    }


    mPopupState = win->GetPopupControlState();

    nsRunnableMethod<nsJSChannel>::Method method;
    if (mIsAsync) {
        // post an event to do the rest
        method = &nsJSChannel::EvaluateScript;
    } else {   
        EvaluateScript();
        if (mOpenedStreamChannel) {
            // That will handle notifying things
            return NS_OK;
        }

        NS_ASSERTION(NS_FAILED(mStatus), "We should have failed _somehow_");

        // mStatus is going to be NS_ERROR_DOM_RETVAL_UNDEFINED if we didn't
        // have any content resulting from the execution and NS_BINDING_ABORTED
        // if something we did causes our own load to be stopped.  Return
        // success in those cases, and error out in all others.
        if (mStatus != NS_ERROR_DOM_RETVAL_UNDEFINED &&
            mStatus != NS_BINDING_ABORTED) {
            // Note that calling EvaluateScript() handled removing us from the
            // loadgroup and marking us as not active anymore.
            CleanupStrongRefs();
            return mStatus;
        }

        // We're returning success from asyncOpen(), but we didn't open a
        // stream channel.  We'll have to notify ourselves, but make sure to do
        // it asynchronously.
        method = &nsJSChannel::NotifyListener;            
    }

    nsCOMPtr<nsIRunnable> ev = new nsRunnableMethod<nsJSChannel>(this, method);
    nsresult rv = NS_DispatchToCurrentThread(ev);

    if (NS_FAILED(rv)) {
        loadGroup->RemoveRequest(this, nsnull, rv);
        mIsActive = PR_FALSE;
        CleanupStrongRefs();
    }
    return rv;
}

void
nsJSChannel::EvaluateScript()
{
    // Synchronously execute the script...
    // mIsActive is used to indicate the the request is 'busy' during the
    // the script evaluation phase.  This means that IsPending() will 
    // indicate the the request is busy while the script is executing...

    // Note that we want to be in the loadgroup and pending while we evaluate
    // the script, so that we find out if the loadgroup gets canceled by the
    // script execution (in which case we shouldn't pump out data even if the
    // script returns it).
    
    if (NS_SUCCEEDED(mStatus)) {
        nsresult rv = mIOThunk->EvaluateScript(mStreamChannel, mPopupState,
                                               mExecutionPolicy,
                                               mOriginalInnerWindow);

        // Note that evaluation may have canceled us, so recheck mStatus again
        if (NS_FAILED(rv) && NS_SUCCEEDED(mStatus)) {
            mStatus = rv;
        }
    }
    
    // Remove the javascript channel from its loadgroup...
    nsCOMPtr<nsILoadGroup> loadGroup;
    mStreamChannel->GetLoadGroup(getter_AddRefs(loadGroup));
    if (loadGroup) {
        loadGroup->RemoveRequest(this, nsnull, mStatus);
    }

    // Reset load flags to their original value...
    mLoadFlags = mActualLoadFlags;

    // We're no longer active, it's now up to the stream channel to do
    // the loading, if needed.
    mIsActive = PR_FALSE;

    if (NS_FAILED(mStatus)) {
        if (mIsAsync) {
            NotifyListener();
        }
        return;
    }
    
    // EvaluateScript() succeeded, and we were not canceled, that
    // means there's data to parse as a result of evaluating the
    // script.

    // Get the stream channels load flags (!= mLoadFlags).
    nsLoadFlags loadFlags;
    mStreamChannel->GetLoadFlags(&loadFlags);

    if (loadFlags & LOAD_DOCUMENT_URI) {
        // We're loaded as the document channel. If we go on,
        // we'll blow away the current document. Make sure that's
        // ok. If so, stop all pending network loads.

        nsCOMPtr<nsIDocShell> docShell;
        NS_QueryNotificationCallbacks(mStreamChannel, docShell);
        if (docShell) {
            nsCOMPtr<nsIContentViewer> cv;
            docShell->GetContentViewer(getter_AddRefs(cv));

            if (cv) {
                PRBool okToUnload;

                if (NS_SUCCEEDED(cv->PermitUnload(&okToUnload)) &&
                    !okToUnload) {
                    // The user didn't want to unload the current
                    // page, translate this into an undefined
                    // return from the javascript: URL...
                    mStatus = NS_ERROR_DOM_RETVAL_UNDEFINED;
                }
            }
        }

        if (NS_SUCCEEDED(mStatus)) {
            mStatus = StopAll();
        }
    }

    if (NS_FAILED(mStatus)) {
        if (mIsAsync) {
            NotifyListener();
        }
        return;
    }
    
    mStatus = mStreamChannel->AsyncOpen(this, mContext);
    if (NS_SUCCEEDED(mStatus)) {
        // mStreamChannel will call OnStartRequest and OnStopRequest on
        // us, so we'll be sure to call them on our listener.
        mOpenedStreamChannel = PR_TRUE;

        // Now readd ourselves to the loadgroup so we can receive
        // cancellation notifications.
        mIsActive = PR_TRUE;
        if (loadGroup) {
            mStatus = loadGroup->AddRequest(this, nsnull);

            // If AddRequest failed, that's OK.  The key is to make sure we get
            // cancelled if needed, and that call just canceled us if it
            // failed.  We'll still get notified by the stream channel when it
            // finishes.
        }
        
    } else if (mIsAsync) {
        NotifyListener();
    }

    return;
}

void
nsJSChannel::NotifyListener()
{
    mListener->OnStartRequest(this, mContext);
    mListener->OnStopRequest(this, mContext, mStatus);

    CleanupStrongRefs();
}

void
nsJSChannel::CleanupStrongRefs()
{
    mListener = nsnull;
    mContext = nsnull;
    mOriginalInnerWindow = nsnull;
    if (mDocumentOnloadBlockedOn) {
        mDocumentOnloadBlockedOn->UnblockOnload(PR_FALSE);
        mDocumentOnloadBlockedOn = nsnull;
    }
}

NS_IMETHODIMP
nsJSChannel::GetLoadFlags(nsLoadFlags *aLoadFlags)
{
    *aLoadFlags = mLoadFlags;

    return NS_OK;
}

NS_IMETHODIMP
nsJSChannel::SetLoadFlags(nsLoadFlags aLoadFlags)
{
    // Since the javascript channel is never the actual channel that
    // any data is loaded through, don't ever set the
    // LOAD_DOCUMENT_URI flag on it, since that could lead to two
    // 'document channels' in the loadgroup if a javascript: URL is
    // loaded while a document is being loaded in the same window.

    // XXXbz this, and a whole lot of other hackery, could go away if we'd just
    // cancel the current document load on javascript: load start like IE does.
    
    mLoadFlags = aLoadFlags & ~LOAD_DOCUMENT_URI;

    // ... but the underlying stream channel should get this bit, if
    // set, since that'll be the real document channel if the
    // javascript: URL generated data.

    return mStreamChannel->SetLoadFlags(aLoadFlags);
}

NS_IMETHODIMP
nsJSChannel::GetLoadGroup(nsILoadGroup* *aLoadGroup)
{
    return mStreamChannel->GetLoadGroup(aLoadGroup);
}

NS_IMETHODIMP
nsJSChannel::SetLoadGroup(nsILoadGroup* aLoadGroup)
{
    if (aLoadGroup) {
        PRBool streamPending;
        nsresult rv = mStreamChannel->IsPending(&streamPending);
        NS_ENSURE_SUCCESS(rv, rv);

        if (streamPending) {
            nsCOMPtr<nsILoadGroup> curLoadGroup;
            mStreamChannel->GetLoadGroup(getter_AddRefs(curLoadGroup));

            if (aLoadGroup != curLoadGroup) {
                // Move the stream channel to our new loadgroup.  Make sure to
                // add it before removing it, so that we don't trigger onload
                // by accident.
                aLoadGroup->AddRequest(mStreamChannel, nsnull);
                if (curLoadGroup) {
                    curLoadGroup->RemoveRequest(mStreamChannel, nsnull,
                                                NS_BINDING_RETARGETED);
                }
            }
        }
    }
    
    return mStreamChannel->SetLoadGroup(aLoadGroup);
}

NS_IMETHODIMP
nsJSChannel::GetOwner(nsISupports* *aOwner)
{
    return mStreamChannel->GetOwner(aOwner);
}

NS_IMETHODIMP
nsJSChannel::SetOwner(nsISupports* aOwner)
{
    return mStreamChannel->SetOwner(aOwner);
}

NS_IMETHODIMP
nsJSChannel::GetNotificationCallbacks(nsIInterfaceRequestor* *aCallbacks)
{
    return mStreamChannel->GetNotificationCallbacks(aCallbacks);
}

NS_IMETHODIMP
nsJSChannel::SetNotificationCallbacks(nsIInterfaceRequestor* aCallbacks)
{
    return mStreamChannel->SetNotificationCallbacks(aCallbacks);
}

NS_IMETHODIMP 
nsJSChannel::GetSecurityInfo(nsISupports * *aSecurityInfo)
{
    return mStreamChannel->GetSecurityInfo(aSecurityInfo);
}

NS_IMETHODIMP
nsJSChannel::GetContentType(nsACString &aContentType)
{
    return mStreamChannel->GetContentType(aContentType);
}

NS_IMETHODIMP
nsJSChannel::SetContentType(const nsACString &aContentType)
{
    return mStreamChannel->SetContentType(aContentType);
}

NS_IMETHODIMP
nsJSChannel::GetContentCharset(nsACString &aContentCharset)
{
    return mStreamChannel->GetContentCharset(aContentCharset);
}

NS_IMETHODIMP
nsJSChannel::SetContentCharset(const nsACString &aContentCharset)
{
    return mStreamChannel->SetContentCharset(aContentCharset);
}

NS_IMETHODIMP
nsJSChannel::GetContentLength(PRInt32 *aContentLength)
{
    return mStreamChannel->GetContentLength(aContentLength);
}

NS_IMETHODIMP
nsJSChannel::SetContentLength(PRInt32 aContentLength)
{
    return mStreamChannel->SetContentLength(aContentLength);
}

NS_IMETHODIMP
nsJSChannel::OnStartRequest(nsIRequest* aRequest,
                            nsISupports* aContext)
{
    NS_ENSURE_TRUE(aRequest == mStreamChannel, NS_ERROR_UNEXPECTED);

    return mListener->OnStartRequest(this, aContext);    
}

NS_IMETHODIMP
nsJSChannel::OnDataAvailable(nsIRequest* aRequest,
                             nsISupports* aContext, 
                             nsIInputStream* aInputStream,
                             PRUint32 aOffset,
                             PRUint32 aCount)
{
    NS_ENSURE_TRUE(aRequest == mStreamChannel, NS_ERROR_UNEXPECTED);

    return mListener->OnDataAvailable(this, aContext, aInputStream, aOffset,
                                      aCount);
}

NS_IMETHODIMP
nsJSChannel::OnStopRequest(nsIRequest* aRequest,
                           nsISupports* aContext,
                           nsresult aStatus)
{
    NS_ENSURE_TRUE(aRequest == mStreamChannel, NS_ERROR_UNEXPECTED);

    nsCOMPtr<nsIStreamListener> listener = mListener;

    CleanupStrongRefs();

    // Make sure aStatus matches what GetStatus() returns
    if (NS_FAILED(mStatus)) {
        aStatus = mStatus;
    }
    
    nsresult rv = listener->OnStopRequest(this, aContext, aStatus);

    nsCOMPtr<nsILoadGroup> loadGroup;
    mStreamChannel->GetLoadGroup(getter_AddRefs(loadGroup));
    if (loadGroup) {
        loadGroup->RemoveRequest(this, nsnull, mStatus);
    }

    mIsActive = PR_FALSE;

    return rv;
}

NS_IMETHODIMP
nsJSChannel::SetExecutionPolicy(PRUint32 aPolicy)
{
    NS_ENSURE_ARG(aPolicy <= EXECUTE_NORMAL);
    
    mExecutionPolicy = aPolicy;
    return NS_OK;
}

NS_IMETHODIMP
nsJSChannel::GetExecutionPolicy(PRUint32* aPolicy)
{
    *aPolicy = mExecutionPolicy;
    return NS_OK;
}

NS_IMETHODIMP
nsJSChannel::SetExecuteAsync(PRBool aIsAsync)
{
    if (!mIsActive) {
        mIsAsync = aIsAsync;
    }
    // else ignore this call
    NS_WARN_IF_FALSE(!mIsActive, "Calling SetExecuteAsync on active channel?");

    return NS_OK;
}

NS_IMETHODIMP
nsJSChannel::GetExecuteAsync(PRBool* aIsAsync)
{
    *aIsAsync = mIsAsync;
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////

nsJSProtocolHandler::nsJSProtocolHandler()
{
}

nsresult
nsJSProtocolHandler::Init()
{
    return NS_OK;
}

nsJSProtocolHandler::~nsJSProtocolHandler()
{
}

NS_IMPL_ISUPPORTS1(nsJSProtocolHandler, nsIProtocolHandler)

NS_METHOD
nsJSProtocolHandler::Create(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
    if (aOuter)
        return NS_ERROR_NO_AGGREGATION;

    nsJSProtocolHandler* ph = new nsJSProtocolHandler();
    if (!ph)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(ph);
    nsresult rv = ph->Init();
    if (NS_SUCCEEDED(rv)) {
        rv = ph->QueryInterface(aIID, aResult);
    }
    NS_RELEASE(ph);
    return rv;
}

nsresult 
nsJSProtocolHandler::EnsureUTF8Spec(const nsAFlatCString &aSpec, const char *aCharset, 
                                    nsACString &aUTF8Spec)
{
  aUTF8Spec.Truncate();

  nsresult rv;
  
  if (!mTextToSubURI) {
    mTextToSubURI = do_GetService(NS_ITEXTTOSUBURI_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);
  }
  nsAutoString uStr;
  rv = mTextToSubURI->UnEscapeNonAsciiURI(nsDependentCString(aCharset), aSpec, uStr);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!IsASCII(uStr))
    NS_EscapeURL(NS_ConvertUTF16toUTF8(uStr), esc_AlwaysCopy | esc_OnlyNonASCII, aUTF8Spec);

  return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// nsIProtocolHandler methods:

NS_IMETHODIMP
nsJSProtocolHandler::GetScheme(nsACString &result)
{
    result = "javascript";
    return NS_OK;
}

NS_IMETHODIMP
nsJSProtocolHandler::GetDefaultPort(PRInt32 *result)
{
    *result = -1;        // no port for javascript: URLs
    return NS_OK;
}

NS_IMETHODIMP
nsJSProtocolHandler::GetProtocolFlags(PRUint32 *result)
{
    *result = URI_NORELATIVE | URI_NOAUTH | URI_INHERITS_SECURITY_CONTEXT |
        URI_LOADABLE_BY_ANYONE | URI_NON_PERSISTABLE;
    return NS_OK;
}

NS_IMETHODIMP
nsJSProtocolHandler::NewURI(const nsACString &aSpec,
                            const char *aCharset, // ignore charset info
                            nsIURI *aBaseURI,
                            nsIURI **result)
{
    nsresult rv;

    // javascript: URLs (currently) have no additional structure beyond that
    // provided by standard URLs, so there is no "outer" object given to
    // CreateInstance.

    nsIURI* url;
    rv = CallCreateInstance(NS_SIMPLEURI_CONTRACTID, &url);

    if (NS_FAILED(rv))
        return rv;

    if (!aCharset || !nsCRT::strcasecmp("UTF-8", aCharset))
      rv = url->SetSpec(aSpec);
    else {
      nsCAutoString utf8Spec;
      rv = EnsureUTF8Spec(PromiseFlatCString(aSpec), aCharset, utf8Spec);
      if (NS_SUCCEEDED(rv)) {
        if (utf8Spec.IsEmpty())
          rv = url->SetSpec(aSpec);
        else
          rv = url->SetSpec(utf8Spec);
      }
    }

    if (NS_FAILED(rv)) {
        NS_RELEASE(url);
        return rv;
    }

    *result = url;
    return rv;
}

NS_IMETHODIMP
nsJSProtocolHandler::NewChannel(nsIURI* uri, nsIChannel* *result)
{
    nsresult rv;
    nsJSChannel * channel;

    NS_ENSURE_ARG_POINTER(uri);

    channel = new nsJSChannel();
    if (!channel) {
        return NS_ERROR_OUT_OF_MEMORY;
    }
    NS_ADDREF(channel);

    rv = channel->Init(uri);
    if (NS_SUCCEEDED(rv)) {
        *result = channel;
        NS_ADDREF(*result);
    }
    NS_RELEASE(channel);
    return rv;
}

NS_IMETHODIMP 
nsJSProtocolHandler::AllowPort(PRInt32 port, const char *scheme, PRBool *_retval)
{
    // don't override anything.  
    *_retval = PR_FALSE;
    return NS_OK;
}

