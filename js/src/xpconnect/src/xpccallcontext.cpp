/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   John Bandhauer <jband@netscape.com> (original author)
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

/* Call context. */

#include "xpcprivate.h"

XPCCallContext::XPCCallContext(XPCContext::LangType callerLanguage,
                               JSContext* cx    /* = nsnull  */,
                               JSObject* obj    /* = nsnull  */,
                               JSObject* funobj /* = nsnull  */,
                               jsval name       /* = 0       */,
                               uintN argc       /* = NO_ARGS */,
                               jsval *argv      /* = nsnull  */,
                               jsval *rval      /* = nsnull  */)
    :   mState(INIT_FAILED),
        mXPC(nsXPConnect::GetXPConnect()),
        mThreadData(nsnull),
        mXPCContext(nsnull),
        mJSContext(cx),
        mContextPopRequired(JS_FALSE),
        mDestroyJSContextInDestructor(JS_FALSE),
        mCallerLanguage(callerLanguage),
        mCallee(nsnull)
{
    // Mark our internal string wrappers as not used. Make sure we do
    // this before any early returns, as the destructor will assert
    // based on this.
    StringWrapperEntry *se =
        reinterpret_cast<StringWrapperEntry*>(&mStringWrapperData);

    PRUint32 i;
    for(i = 0; i < XPCCCX_STRING_CACHE_SIZE; ++i)
    {
        se[i].mInUse = PR_FALSE;
    }

    if(!mXPC)
        return;

    NS_ADDREF(mXPC);

    mThreadData = XPCPerThreadData::GetData(mJSContext);

    if(!mThreadData)
        return;

    XPCJSContextStack* stack = mThreadData->GetJSContextStack();
    JSContext* topJSContext;

    if(!stack || NS_FAILED(stack->Peek(&topJSContext)))
    {
        NS_ERROR("bad!");
        mJSContext = nsnull;
        return;
    }

    if(!mJSContext)
    {
        // This is slightly questionable. If called without an explicit
        // JSContext (generally a call to a wrappedJS) we will use the JSContext
        // on the top of the JSContext stack - if there is one - *before*
        // falling back on the safe JSContext.
        // This is good AND bad because it makes calls from JS -> native -> JS
        // have JS stack 'continuity' for purposes of stack traces etc.
        // Note: this *is* what the pre-XPCCallContext xpconnect did too.

        if(topJSContext)
            mJSContext = topJSContext;
        else if(NS_FAILED(stack->GetSafeJSContext(&mJSContext)) || !mJSContext)
            return;
    }

    // Get into the request as early as we can to avoid problems with scanning
    // callcontexts on other threads from within the gc callbacks.

    if(mCallerLanguage == NATIVE_CALLER)
        JS_BeginRequest(mJSContext);

    if(topJSContext != mJSContext)
    {
        if(NS_FAILED(stack->Push(mJSContext)))
        {
            NS_ERROR("bad!");
            return;
        }
        mContextPopRequired = JS_TRUE;
    }

    // Try to get the JSContext -> XPCContext mapping from the cache.
    // FWIW... quicky tests show this hitting ~ 95% of the time.
    // That is a *lot* of locking we can skip in nsXPConnect::GetContext.
    mXPCContext = mThreadData->GetRecentXPCContext(mJSContext);

    if(!mXPCContext)
    {
        if(!(mXPCContext = nsXPConnect::GetContext(mJSContext, mXPC)))
            return;

        // Fill the cache.
        mThreadData->SetRecentContext(mJSContext, mXPCContext);
    }

    mPrevCallerLanguage = mXPCContext->SetCallingLangType(mCallerLanguage);

    // hook into call context chain for our thread
    mPrevCallContext = mThreadData->SetCallContext(this);

    mState = HAVE_CONTEXT;

    if(!obj)
        return;

    mMethodIndex = 0xDEAD;
    mOperandJSObject = obj;

    mState = HAVE_OBJECT;

    mTearOff = nsnull;
    mWrapper = XPCWrappedNative::GetWrappedNativeOfJSObject(mJSContext, obj,
                                                            funobj,
                                                            &mCurrentJSObject,
                                                            &mTearOff);
    if(!mWrapper)
        return;

    DEBUG_CheckWrapperThreadSafety(mWrapper);

    mFlattenedJSObject = mWrapper->GetFlatJSObject();

    if(mTearOff)
        mScriptableInfo = nsnull;
    else
        mScriptableInfo = mWrapper->GetScriptableInfo();

    if(name)
        SetName(name);

    if(argc != NO_ARGS)
        SetArgsAndResultPtr(argc, argv, rval);

    CHECK_STATE(HAVE_OBJECT);
}

void
XPCCallContext::SetName(jsval name)
{
    CHECK_STATE(HAVE_OBJECT);

    mName = name;

#ifdef XPC_IDISPATCH_SUPPORT
    mIDispatchMember = nsnull;
#endif
    if(mTearOff)
    {
        mSet = nsnull;
        mInterface = mTearOff->GetInterface();
        mMember = mInterface->FindMember(name);
        mStaticMemberIsLocal = JS_TRUE;
        if(mMember && !mMember->IsConstant())
            mMethodIndex = mMember->GetIndex();
    }
    else
    {
        mSet = mWrapper ? mWrapper->GetSet() : nsnull;

        if(mSet &&
           mSet->FindMember(name, &mMember, &mInterface,
                            mWrapper->HasProto() ?
                                mWrapper->GetProto()->GetSet() :
                                nsnull,
                            &mStaticMemberIsLocal))
        {
            if(mMember && !mMember->IsConstant())
                mMethodIndex = mMember->GetIndex();
        }
        else
        {
            mMember = nsnull;
            mInterface = nsnull;
            mStaticMemberIsLocal = JS_FALSE;
        }
    }

    mState = HAVE_NAME;
}

void
XPCCallContext::SetCallInfo(XPCNativeInterface* iface, XPCNativeMember* member,
                            JSBool isSetter)
{
    // We are going straight to the method info and need not do a lookup
    // by id.

    // don't be tricked if method is called with wrong 'this'
    if(mTearOff && mTearOff->GetInterface() != iface)
        mTearOff = nsnull;

    mSet = nsnull;
    mInterface = iface;
    mMember = member;
    mMethodIndex = mMember->GetIndex() + (isSetter ? 1 : 0);
    mName = mMember->GetName();

    if(mState < HAVE_NAME)
        mState = HAVE_NAME;
#ifdef XPC_IDISPATCH_SUPPORT
    mIDispatchMember = nsnull;
#endif
}

void
XPCCallContext::SetArgsAndResultPtr(uintN argc,
                                    jsval *argv,
                                    jsval *rval)
{
    CHECK_STATE(HAVE_OBJECT);

    mArgc   = argc;
    mArgv   = argv;
    mRetVal = rval;

    mExceptionWasThrown = mReturnValueWasSet = JS_FALSE;
    mState = HAVE_ARGS;
}

nsresult
XPCCallContext::CanCallNow()
{
    nsresult rv;
    
    if(!HasInterfaceAndMember())
        return NS_ERROR_UNEXPECTED;
    if(mState < HAVE_ARGS)
        return NS_ERROR_UNEXPECTED;

    if(!mTearOff)
    {
        mTearOff = mWrapper->FindTearOff(*this, mInterface, JS_FALSE, &rv);
        if(!mTearOff || mTearOff->GetInterface() != mInterface)
        {
            mTearOff = nsnull;    
            return NS_FAILED(rv) ? rv : NS_ERROR_UNEXPECTED;
        }
    }

    // Refresh in case FindTearOff extended the set
    mSet = mWrapper->GetSet();

    mState = READY_TO_CALL;
    return NS_OK;
}

void
XPCCallContext::SystemIsBeingShutDown()
{
    // XXX This is pretty questionable since the per thread cleanup stuff
    // can be making this call on one thread for call contexts on another
    // thread.
    NS_WARNING("Shutting Down XPConnect even through there is a live XPCCallContext");
    mThreadData = nsnull;
    mXPCContext = nsnull;
    mState = SYSTEM_SHUTDOWN;
    if(mPrevCallContext)
        mPrevCallContext->SystemIsBeingShutDown();
}

XPCCallContext::~XPCCallContext()
{
    // do cleanup...

    if(mXPCContext)
    {
        mXPCContext->SetCallingLangType(mPrevCallerLanguage);

#ifdef DEBUG
        XPCCallContext* old = mThreadData->SetCallContext(mPrevCallContext);
        NS_ASSERTION(old == this, "bad pop from per thread data");
#else
        (void) mThreadData->SetCallContext(mPrevCallContext);
#endif
    }

    if(mContextPopRequired)
    {
        XPCJSContextStack* stack = mThreadData->GetJSContextStack();
        NS_ASSERTION(stack, "bad!");
        if(stack)
        {
#ifdef DEBUG
            JSContext* poppedCX;
            nsresult rv = stack->Pop(&poppedCX);
            NS_ASSERTION(NS_SUCCEEDED(rv) && poppedCX == mJSContext, "bad pop");
#else
            (void) stack->Pop(nsnull);
#endif
        }
    }

    if(mJSContext)
    {
        if(mCallerLanguage == NATIVE_CALLER)
            JS_EndRequest(mJSContext);
        
        if(mDestroyJSContextInDestructor)
        {
#ifdef DEBUG_xpc_hacker
            printf("!xpc - doing deferred destruction of JSContext @ %0x\n", 
                   mJSContext);
#endif
            NS_ASSERTION(!mThreadData->GetJSContextStack() || 
                         !mThreadData->GetJSContextStack()->
                            DEBUG_StackHasJSContext(mJSContext),
                         "JSContext still in threadjscontextstack!");
        
            JS_DestroyContext(mJSContext);
            mXPC->SyncJSContexts();
        }
        else
        {
            // Don't clear newborns if JS frames (compilation or execution)
            // are active!  Doing so violates ancient invariants in the JS
            // engine, and it's not necessary to fix JS component leaks.
            if(!mJSContext->fp)
                JS_ClearNewbornRoots(mJSContext);
        }
    }

#ifdef DEBUG
    {
        StringWrapperEntry *se =
            reinterpret_cast<StringWrapperEntry*>(&mStringWrapperData);

        PRUint32 i;
        for(i = 0; i < XPCCCX_STRING_CACHE_SIZE; ++i)
        {
            NS_ASSERTION(!se[i].mInUse, "Uh, string wrapper still in use!");
        }
    }
#endif

    NS_IF_RELEASE(mXPC);
}

XPCReadableJSStringWrapper *
XPCCallContext::NewStringWrapper(PRUnichar *str, PRUint32 len)
{
    StringWrapperEntry *se =
        reinterpret_cast<StringWrapperEntry*>(&mStringWrapperData);

    PRUint32 i;
    for(i = 0; i < XPCCCX_STRING_CACHE_SIZE; ++i)
    {
        StringWrapperEntry& ent = se[i];

        if(!ent.mInUse)
        {
            ent.mInUse = PR_TRUE;

            // Construct the string using placement new.

            return new (&ent.mString) XPCReadableJSStringWrapper(str, len);
        }
    }

    // All our internal string wrappers are used, allocate a new string.

    return new XPCReadableJSStringWrapper(str, len);
}

void
XPCCallContext::DeleteString(nsAString *string)
{
    StringWrapperEntry *se =
        reinterpret_cast<StringWrapperEntry*>(&mStringWrapperData);

    PRUint32 i;
    for(i = 0; i < XPCCCX_STRING_CACHE_SIZE; ++i)
    {
        StringWrapperEntry& ent = se[i];
        if(string == &ent.mString)
        {
            // One of our internal strings is no longer in use, mark
            // it as such and destroy the string.

            ent.mInUse = PR_FALSE;
            ent.mString.~XPCReadableJSStringWrapper();

            return;
        }
    }

    // We're done with a string that's not one of our internal
    // strings, delete it.
    delete string;
}

/* readonly attribute nsISupports Callee; */
NS_IMETHODIMP
XPCCallContext::GetCallee(nsISupports * *aCallee)
{
    nsISupports* temp = mWrapper ? mWrapper->GetIdentityObject() : nsnull;
    NS_IF_ADDREF(temp);
    *aCallee = temp;
    return NS_OK;
}

/* readonly attribute PRUint16 CalleeMethodIndex; */
NS_IMETHODIMP
XPCCallContext::GetCalleeMethodIndex(PRUint16 *aCalleeMethodIndex)
{
    *aCalleeMethodIndex = mMethodIndex;
    return NS_OK;
}

/* readonly attribute nsIXPConnectWrappedNative CalleeWrapper; */
NS_IMETHODIMP
XPCCallContext::GetCalleeWrapper(nsIXPConnectWrappedNative * *aCalleeWrapper)
{
    nsIXPConnectWrappedNative* temp = mWrapper;
    NS_IF_ADDREF(temp);
    *aCalleeWrapper = temp;
    return NS_OK;
}

/* readonly attribute XPCNativeInterface CalleeInterface; */
NS_IMETHODIMP
XPCCallContext::GetCalleeInterface(nsIInterfaceInfo * *aCalleeInterface)
{
    nsIInterfaceInfo* temp = mInterface->GetInterfaceInfo();
    NS_IF_ADDREF(temp);
    *aCalleeInterface = temp;
    return NS_OK;
}

/* readonly attribute nsIClassInfo CalleeClassInfo; */
NS_IMETHODIMP
XPCCallContext::GetCalleeClassInfo(nsIClassInfo * *aCalleeClassInfo)
{
    nsIClassInfo* temp = mWrapper ? mWrapper->GetClassInfo() : nsnull;
    NS_IF_ADDREF(temp);
    *aCalleeClassInfo = temp;
    return NS_OK;
}

/* readonly attribute JSContextPtr JSContext; */
NS_IMETHODIMP
XPCCallContext::GetJSContext(JSContext * *aJSContext)
{
    *aJSContext = mJSContext;
    return NS_OK;
}

/* readonly attribute PRUint32 Argc; */
NS_IMETHODIMP
XPCCallContext::GetArgc(PRUint32 *aArgc)
{
    *aArgc = (PRUint32) mArgc;
    return NS_OK;
}

/* readonly attribute JSValPtr ArgvPtr; */
NS_IMETHODIMP
XPCCallContext::GetArgvPtr(jsval * *aArgvPtr)
{
    *aArgvPtr = mArgv;
    return NS_OK;
}

/* readonly attribute JSValPtr RetValPtr; */
NS_IMETHODIMP
XPCCallContext::GetRetValPtr(jsval * *aRetValPtr)
{
    *aRetValPtr = mRetVal;
    return NS_OK;
}

/* attribute PRBool ExceptionWasThrown; */
NS_IMETHODIMP
XPCCallContext::GetExceptionWasThrown(PRBool *aExceptionWasThrown)
{
    *aExceptionWasThrown = mExceptionWasThrown;
    return NS_OK;
}
NS_IMETHODIMP
XPCCallContext::SetExceptionWasThrown(PRBool aExceptionWasThrown)
{
    mExceptionWasThrown = aExceptionWasThrown;
    return NS_OK;
}

/* attribute PRBool ReturnValueWasSet; */
NS_IMETHODIMP
XPCCallContext::GetReturnValueWasSet(PRBool *aReturnValueWasSet)
{
    *aReturnValueWasSet = mReturnValueWasSet;
    return NS_OK;
}
NS_IMETHODIMP
XPCCallContext::SetReturnValueWasSet(PRBool aReturnValueWasSet)
{
    mReturnValueWasSet = aReturnValueWasSet;
    return NS_OK;
}

#ifdef XPC_IDISPATCH_SUPPORT

void
XPCCallContext::SetIDispatchInfo(XPCNativeInterface* iface, 
                                 void * member)
{
    // We are going straight to the method info and need not do a lookup
    // by id.

    // don't be tricked if method is called with wrong 'this'
    if(mTearOff && mTearOff->GetInterface() != iface)
        mTearOff = nsnull;

    mSet = nsnull;
    mInterface = iface;
    mMember = nsnull;
    mIDispatchMember = member;
    mName = reinterpret_cast<XPCDispInterface::Member*>(member)->GetName();

    if(mState < HAVE_NAME)
        mState = HAVE_NAME;
}

#endif
