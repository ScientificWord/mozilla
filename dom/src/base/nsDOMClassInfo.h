/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=2 sw=2 et tw=80: */
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
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Johnny Stenback <jst@netscape.com> (original author)
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

#ifndef nsDOMClassInfo_h___
#define nsDOMClassInfo_h___

#include "nsIDOMClassInfo.h"
#include "nsIXPCScriptable.h"
#include "jsapi.h"
#include "nsIScriptSecurityManager.h"
#include "nsIScriptContext.h"
#include "nsDOMJSUtils.h" // for GetScriptContextFromJSContext

class nsIDOMWindow;
class nsIDOMNSHTMLOptionCollection;
class nsIPluginInstance;
class nsIForm;
class nsIDOMNode;
class nsIDOMNodeList;
class nsIDOMDocument;
class nsIDOMGCParticipant;
class nsIHTMLDocument;
class nsGlobalWindow;

struct nsDOMClassInfoData;

typedef nsIClassInfo* (*nsDOMClassInfoConstructorFnc)
  (nsDOMClassInfoData* aData);


struct nsDOMClassInfoData
{
  const char *mName;
  union {
    nsDOMClassInfoConstructorFnc mConstructorFptr;
    nsDOMClassInfoExternalConstructorFnc mExternalConstructorFptr;
  } u;

  nsIClassInfo *mCachedClassInfo; // low bit is set to 1 if external,
                                  // so be sure to mask if necessary!
  const nsIID *mProtoChainInterface;
  const nsIID **mInterfaces;
  PRUint32 mScriptableFlags : 31; // flags must not use more than 31 bits!
  PRBool mHasClassInterface : 1;
#ifdef NS_DEBUG
  PRUint32 mDebugID;
#endif
};

struct nsExternalDOMClassInfoData : public nsDOMClassInfoData
{
  const nsCID *mConstructorCID;
};


typedef unsigned long PtrBits;

// To be used with the nsDOMClassInfoData::mCachedClassInfo pointer.
// The low bit is set when we created a generic helper for an external
// (which holds on to the nsDOMClassInfoData).
#define GET_CLEAN_CI_PTR(_ptr) (nsIClassInfo*)(PtrBits(_ptr) & ~0x1)
#define MARK_EXTERNAL(_ptr) (nsIClassInfo*)(PtrBits(_ptr) | 0x1)
#define IS_EXTERNAL(_ptr) (PtrBits(_ptr) & 0x1)


class nsDOMClassInfo : public nsIXPCScriptable,
                       public nsIClassInfo
{
public:
  nsDOMClassInfo(nsDOMClassInfoData* aData);
  virtual ~nsDOMClassInfo();

  NS_DECL_NSIXPCSCRIPTABLE

  NS_DECL_ISUPPORTS

  NS_DECL_NSICLASSINFO

  // Helper method that returns a *non* refcounted pointer to a
  // helper. So please note, don't release this pointer, if you do,
  // you better make sure you've addreffed before release.
  //
  // Whaaaaa! I wanted to name this method GetClassInfo, but nooo,
  // some of Microsoft devstudio's headers #defines GetClassInfo to
  // GetClassInfoA so I can't, those $%#@^! bastards!!! What gives
  // them the right to do that?

  static nsIClassInfo* GetClassInfoInstance(nsDOMClassInfoID aID);
  static nsIClassInfo* GetClassInfoInstance(nsDOMClassInfoData* aData);

  static void ShutDown();

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsDOMClassInfo(aData);
  }

  static nsresult WrapNative(JSContext *cx, JSObject *scope,
                             nsISupports *native, const nsIID& aIID,
                             jsval *vp,
                             // aHolder keeps the jsval alive while
                             // there's a ref to it
                             nsIXPConnectJSObjectHolder** aHolder);
  static nsresult ThrowJSException(JSContext *cx, nsresult aResult);

  static nsresult InitDOMJSClass(JSContext *cx, JSObject *obj);

  static JSClass sDOMJSClass;

  /**
   * Get our JSClass pointer for the XPCNativeWrapper class
   */
  static const JSClass* GetXPCNativeWrapperClass() {
    return sXPCNativeWrapperClass;
  }
  
  /**
   * Set our JSClass pointer for the XPCNativeWrapper class
   */
  static void SetXPCNativeWrapperClass(JSClass* aClass) {
    NS_ASSERTION(!sXPCNativeWrapperClass,
                 "Double set of sXPCNativeWrapperClass");
    sXPCNativeWrapperClass = aClass;
  }

  static PRBool ObjectIsNativeWrapper(JSContext* cx, JSObject* obj)
  {
#ifdef DEBUG
    {
      nsIScriptContext *scx = GetScriptContextFromJSContext(cx);

      NS_PRECONDITION(!scx || !scx->IsContextInitialized() ||
                      sXPCNativeWrapperClass,
                      "Must know what the XPCNativeWrapper class is!");
    }
#endif

    return sXPCNativeWrapperClass &&
      ::JS_GetClass(cx, obj) == sXPCNativeWrapperClass;
  }

  /**
   * Note that the XPConnect wrapper should be protected from garbage
   * collection as long as the GC participant is reachable.
   *
   * A preservation with a given key overwrites any previous
   * preservation with that key.
   *
   * No strong references are held as a result of this function call, so
   * the caller is responsible for calling |ReleaseWrapper| sometime
   * before |aParticipant|'s destructor runs.
   *
   * aRootWhenExternallyReferenced should be true if the preservation is
   * for an event handler that could be triggered by the participant
   * being externally referenced by a network load.
   */
  static nsresult PreserveWrapper(void* aKey,
                                  nsIXPConnectJSObjectHolder* (*aKeyToWrapperFunc)(void* aKey),
                                  nsIDOMGCParticipant *aParticipant,
                                  PRBool aRootWhenExternallyReferenced);


  /**
   * Easier way to call the above just for DOM nodes (and better, since
   * we get the performance benefits of having the same identity function).
   * The call to |PreserveWrapper| is made with |aKey| == |aWrapper|.
   *
   * The caller need not call |ReleaseWrapper| since the node's
   * wrapper's scriptable helper does so in its finalize callback.
   */
  static nsresult PreserveNodeWrapper(nsIXPConnectWrappedNative *aWrapper,
                                      PRBool aRootWhenExternallyReferenced =
                                        PR_FALSE);

  /**
   * Undoes the effects of any prior |PreserveWrapper| calls made with
   * |aKey|.
   */
  static void ReleaseWrapper(void* aKey);

  /**
   * Mark all preserved wrappers reachable from |aDOMNode| via DOM APIs.
   */
  static void MarkReachablePreservedWrappers(nsIDOMGCParticipant *aParticipant,
                                             JSContext *cx, void *arg);

  /**
   * Add/remove |aParticipant| from the table of externally referenced
   * participants.  Does not maintain a count, so an object should not
   * be added when it is already in the table.
   *
   * The table of externally referenced participants is a list of
   * participants that should be marked at GC-time **if they are a
   * participant in the preserved wrapper table added with
   * aRootWhenExternallyReferenced**, whether or not they are reachable
   * from marked participants.  This should be used for participants
   * that hold onto scripts (typically onload or onerror handlers) that
   * can be triggered at the end of a currently-ongoing operation
   * (typically a network request) and that could thus expose the
   * participant to script again in the future even though it is not
   * otherwise reachable.
   *
   * The caller is responsible for ensuring that the GC participant is
   * alive while it is in this table; the table does not own a reference.
   *
   * UnsetExternallyReferenced must be called exactly once for every
   * successful SetExternallyReferenced call, and in no other cases.
   */
  static nsresult SetExternallyReferenced(nsIDOMGCParticipant *aParticipant);
  static void UnsetExternallyReferenced(nsIDOMGCParticipant *aParticipant);

  /**
   * Classify the wrappers for use by |MarkReachablePreservedWrappers|
   * during the GC.  Returns false to indicate failure (out-of-memory).
   */
  static PRBool BeginGCMark(JSContext *cx);

  /**
   * Clean up data structures (and strong references) created by
   * |BeginGCMark|.
   */
  static void EndGCMark();

protected:
  const nsDOMClassInfoData* mData;

  static nsresult Init();
  static nsresult RegisterClassName(PRInt32 aDOMClassInfoID);
  static nsresult RegisterClassProtos(PRInt32 aDOMClassInfoID);
  static nsresult RegisterExternalClasses();
  nsresult ResolveConstructor(JSContext *cx, JSObject *obj,
                              JSObject **objp);

  // Checks if id is a number and returns the number, if aIsNumber is
  // non-null it's set to true if the id is a number and false if it's
  // not a number. If id is not a number this method returns -1
  static PRInt32 GetArrayIndexFromId(JSContext *cx, jsval id,
                                     PRBool *aIsNumber = nsnull);

  static inline PRBool IsReadonlyReplaceable(jsval id)
  {
    return (id == sTop_id          ||
            id == sParent_id       ||
            id == sScrollbars_id   ||
            id == sContent_id      ||
            id == sMenubar_id      ||
            id == sToolbar_id      ||
            id == sLocationbar_id  ||
            id == sPersonalbar_id  ||
            id == sStatusbar_id    ||
            id == sDirectories_id  ||
            id == sControllers_id  ||
            id == sScrollX_id      ||
            id == sScrollY_id      ||
            id == sScrollMaxX_id   ||
            id == sScrollMaxY_id   ||
            id == sLength_id       ||
            id == sFrames_id       ||
            id == sSelf_id);
  }

  static inline PRBool IsWritableReplaceable(jsval id)
  {
    return (id == sInnerHeight_id  ||
            id == sInnerWidth_id   ||
            id == sOpener_id       ||
            id == sOuterHeight_id  ||
            id == sOuterWidth_id   ||
            id == sScreenX_id      ||
            id == sScreenY_id      ||
            id == sStatus_id       ||
            id == sName_id);
  }

  nsresult doCheckPropertyAccess(JSContext *cx, JSObject *obj, jsval id,
                                 nsIXPConnectWrappedNative *wrapper,
                                 PRUint32 accessMode, PRBool isWindow);

  static JSClass sDOMConstructorProtoClass;
  static JSFunctionSpec sDOMJSClass_methods[];

  static nsIXPConnect *sXPConnect;
  static nsIScriptSecurityManager *sSecMan;

  // nsIXPCScriptable code
  static nsresult DefineStaticJSVals(JSContext *cx);

  static PRBool sIsInitialized;
  static PRBool sDisableDocumentAllSupport;
  static PRBool sDisableGlobalScopePollutionSupport;

  static jsval sTop_id;
  static jsval sParent_id;
  static jsval sScrollbars_id;
  static jsval sLocation_id;
  static jsval sConstructor_id;
  static jsval s_content_id;
  static jsval sContent_id;
  static jsval sMenubar_id;
  static jsval sToolbar_id;
  static jsval sLocationbar_id;
  static jsval sPersonalbar_id;
  static jsval sStatusbar_id;
  static jsval sDirectories_id;
  static jsval sControllers_id;
  static jsval sLength_id;
  static jsval sInnerHeight_id;
  static jsval sInnerWidth_id;
  static jsval sOuterHeight_id;
  static jsval sOuterWidth_id;
  static jsval sScreenX_id;
  static jsval sScreenY_id;
  static jsval sStatus_id;
  static jsval sName_id;
  static jsval sOnmousedown_id;
  static jsval sOnmouseup_id;
  static jsval sOnclick_id;
  static jsval sOndblclick_id;
  static jsval sOncontextmenu_id;
  static jsval sOnmouseover_id;
  static jsval sOnmouseout_id;
  static jsval sOnkeydown_id;
  static jsval sOnkeyup_id;
  static jsval sOnkeypress_id;
  static jsval sOnmousemove_id;
  static jsval sOnfocus_id;
  static jsval sOnblur_id;
  static jsval sOnsubmit_id;
  static jsval sOnreset_id;
  static jsval sOnchange_id;
  static jsval sOnselect_id;
  static jsval sOnload_id;
  static jsval sOnbeforeunload_id;
  static jsval sOnunload_id;
  static jsval sOnpageshow_id;
  static jsval sOnpagehide_id;
  static jsval sOnabort_id;
  static jsval sOnerror_id;
  static jsval sOnpaint_id;
  static jsval sOnresize_id;
  static jsval sOnscroll_id;
  static jsval sScrollIntoView_id;
  static jsval sScrollX_id;
  static jsval sScrollY_id;
  static jsval sScrollMaxX_id;
  static jsval sScrollMaxY_id;
  static jsval sOpen_id;
  static jsval sItem_id;
  static jsval sNamedItem_id;
  static jsval sEnumerate_id;
  static jsval sNavigator_id;
  static jsval sDocument_id;
  static jsval sWindow_id;
  static jsval sFrames_id;
  static jsval sSelf_id;
  static jsval sOpener_id;
  static jsval sAdd_id;
  static jsval sAll_id;
  static jsval sTags_id;
  static jsval sAddEventListener_id;
  static jsval sBaseURIObject_id;
  static jsval sNodePrincipal_id;
  static jsval sDocumentURIObject_id;

  static const JSClass *sObjectClass;
  static const JSClass *sXPCNativeWrapperClass;

public:
  static PRBool sDoSecurityCheckInAddProperty;
};

typedef nsDOMClassInfo nsDOMGenericSH;

// Scriptable helper for implementations of nsIDOMGCParticipant that
// need a mark callback.
class nsDOMGCParticipantSH : public nsDOMGenericSH
{
protected:
  nsDOMGCParticipantSH(nsDOMClassInfoData* aData) : nsDOMGenericSH(aData)
  {
  }

  virtual ~nsDOMGCParticipantSH()
  {
  }

public:
  NS_IMETHOD Finalize(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                      JSObject *obj);
  NS_IMETHOD Mark(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                  JSObject *obj, void *arg, PRUint32 *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsDOMGCParticipantSH(aData);
  }
};

// EventProp scriptable helper, this class should be the base class of
// all objects that should support things like
// obj.onclick=function{...}

class nsEventReceiverSH : public nsDOMGCParticipantSH
{
protected:
  nsEventReceiverSH(nsDOMClassInfoData* aData) : nsDOMGCParticipantSH(aData)
  {
  }

  virtual ~nsEventReceiverSH()
  {
  }

  static PRBool ReallyIsEventName(jsval id, jschar aFirstChar);

  static inline PRBool IsEventName(jsval id)
  {
    NS_ASSERTION(JSVAL_IS_STRING(id), "Don't pass non-string jsval's here!");

    jschar *str = ::JS_GetStringChars(JSVAL_TO_STRING(id));

    if (str[0] == 'o' && str[1] == 'n') {
      return ReallyIsEventName(id, str[2]);
    }

    return PR_FALSE;
  }

  static JSBool JS_DLL_CALLBACK AddEventListenerHelper(JSContext *cx,
                                                       JSObject *obj,
                                                       uintN argc, jsval *argv,
                                                       jsval *rval);

  nsresult RegisterCompileHandler(nsIXPConnectWrappedNative *wrapper,
                                  JSContext *cx, JSObject *obj, jsval id,
                                  PRBool compile, PRBool remove,
                                  PRBool *did_define);

public:
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp,
                         PRBool *_retval);
  NS_IMETHOD AddProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
};


// Window scriptable helper

class nsWindowSH : public nsEventReceiverSH
{
protected:
  nsWindowSH(nsDOMClassInfoData* aData) : nsEventReceiverSH(aData)
  {
  }

  virtual ~nsWindowSH()
  {
  }

  static nsresult GlobalResolve(nsGlobalWindow *aWin, JSContext *cx,
                                JSObject *obj, JSString *str, PRUint32 flags,
                                PRBool *did_resolve);

public:
  NS_IMETHOD PreCreate(nsISupports *nativeObj, JSContext *cx,
                       JSObject *globalObj, JSObject **parentObj);
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD AddProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD DelProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD NewEnumerate(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                          JSObject *obj, PRUint32 enum_op, jsval *statep,
                          jsid *idp, PRBool *_retval);
  NS_IMETHOD Finalize(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                      JSObject *obj);
  NS_IMETHOD Equality(nsIXPConnectWrappedNative *wrapper, JSContext * cx,
                      JSObject * obj, jsval val, PRBool *bp);
  NS_IMETHOD OuterObject(nsIXPConnectWrappedNative *wrapper, JSContext * cx,
                         JSObject * obj, JSObject * *_retval);
  NS_IMETHOD InnerObject(nsIXPConnectWrappedNative *wrapper, JSContext * cx,
                         JSObject * obj, JSObject * *_retval);

  static JSBool JS_DLL_CALLBACK GlobalScopePolluterNewResolve(JSContext *cx,
                                                              JSObject *obj,
                                                              jsval id,
                                                              uintN flags,
                                                              JSObject **objp);
  static JSBool JS_DLL_CALLBACK GlobalScopePolluterGetProperty(JSContext *cx,
                                                               JSObject *obj,
                                                               jsval id,
                                                               jsval *vp);
  static JSBool JS_DLL_CALLBACK SecurityCheckOnSetProp(JSContext *cx,
                                                       JSObject *obj, jsval id,
                                                       jsval *vp);
  static void InvalidateGlobalScopePolluter(JSContext *cx, JSObject *obj);
  static nsresult InstallGlobalScopePolluter(JSContext *cx, JSObject *obj,
                                             nsIHTMLDocument *doc);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsWindowSH(aData);
  }
};


// Location scriptable helper

class nsLocationSH : public nsDOMGenericSH
{
protected:
  nsLocationSH(nsDOMClassInfoData* aData) : nsDOMGenericSH(aData)
  {
  }

  virtual ~nsLocationSH()
  {
  }

public:
  NS_IMETHOD CheckAccess(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, PRUint32 mode,
                         jsval *vp, PRBool *_retval);

  NS_IMETHOD PreCreate(nsISupports *nativeObj, JSContext *cx,
                       JSObject *globalObj, JSObject **parentObj);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsLocationSH(aData);
  }
};


// Navigator scriptable helper

class nsNavigatorSH : public nsDOMGenericSH
{
protected:
  nsNavigatorSH(nsDOMClassInfoData* aData) : nsDOMGenericSH(aData)
  {
  }

  virtual ~nsNavigatorSH()
  {
  }

public:
  NS_IMETHOD PreCreate(nsISupports *nativeObj, JSContext *cx,
                       JSObject *globalObj, JSObject **parentObj);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsNavigatorSH(aData);
  }
};


// DOM Node helper, this class deals with setting the parent for the
// wrappers

class nsNodeSH : public nsEventReceiverSH
{
protected:
  nsNodeSH(nsDOMClassInfoData* aData) : nsEventReceiverSH(aData)
  {
  }

  virtual ~nsNodeSH()
  {
  }

  // Helper to check whether a capability is enabled
  PRBool IsCapabilityEnabled(const char* aCapability);

  inline PRBool IsPrivilegedScript() {
    return IsCapabilityEnabled("UniversalXPConnect");
  }

  // Helper to define a void property with JSPROP_SHARED; this can do all the
  // work so it's safe to just return whatever it returns.  |obj| is the object
  // we're defining on, |id| is the name of the prop.  This must be a string
  // jsval.  |objp| is the out param if we define successfully.
  nsresult DefineVoidProp(JSContext* cx, JSObject* obj, jsval id,
                          JSObject** objp);

public:
  NS_IMETHOD PreCreate(nsISupports *nativeObj, JSContext *cx,
                       JSObject *globalObj, JSObject **parentObj);
  NS_IMETHOD AddProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD GetFlags(PRUint32 *aFlags);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsNodeSH(aData);
  }
};


// Element helper

class nsElementSH : public nsNodeSH
{
protected:
  nsElementSH(nsDOMClassInfoData* aData) : nsNodeSH(aData)
  {
  }

  virtual ~nsElementSH()
  {
  }

public:
  NS_IMETHOD PostCreate(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsElementSH(aData);
  }
};


// Generic array scriptable helper

class nsGenericArraySH : public nsDOMClassInfo
{
protected:
  nsGenericArraySH(nsDOMClassInfoData* aData) : nsDOMClassInfo(aData)
  {
  }

  virtual ~nsGenericArraySH()
  {
  }
  
public:
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD Enumerate(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                       JSObject *obj, PRBool *_retval);
  
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsGenericArraySH(aData);
  }
};


// NodeList scriptable helper

class nsArraySH : public nsGenericArraySH
{
protected:
  nsArraySH(nsDOMClassInfoData* aData) : nsGenericArraySH(aData)
  {
  }

  virtual ~nsArraySH()
  {
  }

  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsArraySH(aData);
  }
};


// NamedArray helper

class nsNamedArraySH : public nsArraySH
{
protected:
  nsNamedArraySH(nsDOMClassInfoData* aData) : nsArraySH(aData)
  {
  }

  virtual ~nsNamedArraySH()
  {
  }

  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult) = 0;

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
};


// NamedNodeMap helper

class nsNamedNodeMapSH : public nsNamedArraySH
{
protected:
  nsNamedNodeMapSH(nsDOMClassInfoData* aData) : nsNamedArraySH(aData)
  {
  }

  virtual ~nsNamedNodeMapSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsNamedNodeMapSH(aData);
  }
};


// HTMLCollection helper

class nsHTMLCollectionSH : public nsNamedArraySH
{
protected:
  nsHTMLCollectionSH(nsDOMClassInfoData* aData) : nsNamedArraySH(aData)
  {
  }

  virtual ~nsHTMLCollectionSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLCollectionSH(aData);
  }
};


// ContentList helper

class nsContentListSH : public nsHTMLCollectionSH
{
protected:
  nsContentListSH(nsDOMClassInfoData* aData) : nsHTMLCollectionSH(aData)
  {
  }

  virtual ~nsContentListSH()
  {
  }

public:
  NS_IMETHOD PreCreate(nsISupports *nativeObj, JSContext *cx,
                       JSObject *globalObj, JSObject **parentObj);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsContentListSH(aData);
  }
};



// FomrControlList helper

class nsFormControlListSH : public nsHTMLCollectionSH
{
protected:
  nsFormControlListSH(nsDOMClassInfoData* aData) : nsHTMLCollectionSH(aData)
  {
  }

  virtual ~nsFormControlListSH()
  {
  }

  // Override nsNamedArraySH::GetNamedItem() since our NamedItem() can
  // return either a nsIDOMNode or a nsIHTMLCollection
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsFormControlListSH(aData);
  }
};


// Document helper, for document.location and document.on*

class nsDocumentSH : public nsNodeSH
{
public:
  nsDocumentSH(nsDOMClassInfoData* aData) : nsNodeSH(aData)
  {
  }

  virtual ~nsDocumentSH()
  {
  }

public:
  NS_IMETHOD AddProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD DelProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD GetFlags(PRUint32* aFlags);
  NS_IMETHOD PostCreate(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsDocumentSH(aData);
  }
};


// HTMLDocument helper

class nsHTMLDocumentSH : public nsDocumentSH
{
protected:
  nsHTMLDocumentSH(nsDOMClassInfoData* aData) : nsDocumentSH(aData)
  {
  }

  virtual ~nsHTMLDocumentSH()
  {
  }

  static nsresult ResolveImpl(JSContext *cx,
                              nsIXPConnectWrappedNative *wrapper, jsval id,
                              nsISupports **result);
  static JSBool JS_DLL_CALLBACK DocumentOpen(JSContext *cx, JSObject *obj,
                                             uintN argc, jsval *argv,
                                             jsval *rval);
  static JSBool GetDocumentAllNodeList(JSContext *cx, JSObject *obj,
                                       nsIDOMDocument *doc,
                                       nsIDOMNodeList **nodeList);

public:
  static JSBool JS_DLL_CALLBACK DocumentAllGetProperty(JSContext *cx,
                                                       JSObject *obj, jsval id,
                                                       jsval *vp);
  static JSBool JS_DLL_CALLBACK DocumentAllNewResolve(JSContext *cx,
                                                      JSObject *obj, jsval id,
                                                      uintN flags,
                                                      JSObject **objp);
  static void JS_DLL_CALLBACK ReleaseDocument(JSContext *cx, JSObject *obj);
  static JSBool JS_DLL_CALLBACK CallToGetPropMapper(JSContext *cx,
                                                    JSObject *obj, uintN argc,
                                                    jsval *argv, jsval *rval);
  static JSBool JS_DLL_CALLBACK DocumentAllHelperGetProperty(JSContext *cx,
                                                             JSObject *obj,
                                                             jsval id,
                                                             jsval *vp);
  static JSBool JS_DLL_CALLBACK DocumentAllHelperNewResolve(JSContext *cx,
                                                            JSObject *obj,
                                                            jsval id,
                                                            uintN flags,
                                                            JSObject **objp);
  static JSBool JS_DLL_CALLBACK DocumentAllTagsNewResolve(JSContext *cx,
                                                          JSObject *obj,
                                                          jsval id,
                                                          uintN flags,
                                                          JSObject **objp);

  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLDocumentSH(aData);
  }
};


// HTMLElement helper

class nsHTMLElementSH : public nsElementSH
{
protected:
  nsHTMLElementSH(nsDOMClassInfoData* aData) : nsElementSH(aData)
  {
  }

  virtual ~nsHTMLElementSH()
  {
  }

  static JSBool JS_DLL_CALLBACK ScrollIntoView(JSContext *cx, JSObject *obj,
                                               uintN argc, jsval *argv,
                                               jsval *rval);

public:
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLElementSH(aData);
  }
};


// HTMLFormElement helper

class nsHTMLFormElementSH : public nsHTMLElementSH
{
protected:
  nsHTMLFormElementSH(nsDOMClassInfoData* aData) : nsHTMLElementSH(aData)
  {
  }

  virtual ~nsHTMLFormElementSH()
  {
  }

  static nsresult FindNamedItem(nsIForm *aForm, JSString *str,
                                nsISupports **aResult);

public:
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp,
                         PRBool *_retval);

  NS_IMETHOD NewEnumerate(nsIXPConnectWrappedNative *wrapper,
                          JSContext *cx, JSObject *obj,
                          PRUint32 enum_op, jsval *statep,
                          jsid *idp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLFormElementSH(aData);
  }
};


// HTML[I]FrameElement helper

class nsHTMLFrameElementSH : public nsHTMLElementSH
{
protected:
  nsHTMLFrameElementSH(nsDOMClassInfoData* aData) : nsHTMLElementSH(aData)
  {
  }

  virtual ~nsHTMLFrameElementSH()
  {
  }

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD AddProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD DelProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLFrameElementSH(aData);
  }
};


// HTMLSelectElement helper

class nsHTMLSelectElementSH : public nsHTMLElementSH
{
protected:
  nsHTMLSelectElementSH(nsDOMClassInfoData* aData) : nsHTMLElementSH(aData)
  {
  }

  virtual ~nsHTMLSelectElementSH()
  {
  }

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp,
                         PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsresult SetOption(JSContext *cx, jsval *vp, PRUint32 aIndex,
                            nsIDOMNSHTMLOptionCollection *aOptCollection);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLSelectElementSH(aData);
  }
};


// Base helper for external HTML object (such as a plugin or an
// applet)

class nsHTMLExternalObjSH : public nsHTMLElementSH
{
protected:
  nsHTMLExternalObjSH(nsDOMClassInfoData* aData) : nsHTMLElementSH(aData)
  {
  }

  virtual ~nsHTMLExternalObjSH()
  {
  }

  nsresult GetPluginInstance(nsIXPConnectWrappedNative *aWrapper,
                             nsIPluginInstance **aResult);

  virtual nsresult GetPluginJSObject(JSContext *cx, JSObject *obj,
                                     nsIPluginInstance *plugin_inst,
                                     JSObject **plugin_obj,
                                     JSObject **plugin_proto) = 0;

public:
  NS_IMETHOD PostCreate(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj);
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD Call(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                  JSObject *obj, PRUint32 argc, jsval *argv, jsval *vp,
                  PRBool *_retval);
};


// HTMLAppletElement helper

class nsHTMLAppletElementSH : public nsHTMLExternalObjSH
{
protected:
  nsHTMLAppletElementSH(nsDOMClassInfoData* aData) : nsHTMLExternalObjSH(aData)
  {
  }

  virtual ~nsHTMLAppletElementSH()
  {
  }

  virtual nsresult GetPluginJSObject(JSContext *cx, JSObject *obj,
                                     nsIPluginInstance *plugin_inst,
                                     JSObject **plugin_obj,
                                     JSObject **plugin_proto);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLAppletElementSH(aData);
  }
};


// HTMLEmbed/ObjectElement helper

class nsHTMLPluginObjElementSH : public nsHTMLAppletElementSH
{
protected:
  nsHTMLPluginObjElementSH(nsDOMClassInfoData* aData)
    : nsHTMLAppletElementSH(aData)
  {
  }

  virtual ~nsHTMLPluginObjElementSH()
  {
  }

  virtual nsresult GetPluginJSObject(JSContext *cx, JSObject *obj,
                                     nsIPluginInstance *plugin_inst,
                                     JSObject **plugin_obj,
                                     JSObject **plugin_proto);

public:
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLPluginObjElementSH(aData);
  }
};


// HTMLOptionsCollection helper

class nsHTMLOptionsCollectionSH : public nsHTMLCollectionSH
{
protected:
  nsHTMLOptionsCollectionSH(nsDOMClassInfoData* aData)
    : nsHTMLCollectionSH(aData)
  {
  }

  virtual ~nsHTMLOptionsCollectionSH()
  {
  }

  static JSBool JS_DLL_CALLBACK Add(JSContext *cx, JSObject *obj, uintN argc,
                                    jsval *argv, jsval *rval);

public:
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHTMLOptionsCollectionSH(aData);
  }
};


// Plugin helper

class nsPluginSH : public nsNamedArraySH
{
protected:
  nsPluginSH(nsDOMClassInfoData* aData) : nsNamedArraySH(aData)
  {
  }

  virtual ~nsPluginSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsPluginSH(aData);
  }
};


// PluginArray helper

class nsPluginArraySH : public nsNamedArraySH
{
protected:
  nsPluginArraySH(nsDOMClassInfoData* aData) : nsNamedArraySH(aData)
  {
  }

  virtual ~nsPluginArraySH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsPluginArraySH(aData);
  }
};


// MimeTypeArray helper

class nsMimeTypeArraySH : public nsNamedArraySH
{
protected:
  nsMimeTypeArraySH(nsDOMClassInfoData* aData) : nsNamedArraySH(aData)
  {
  }

  virtual ~nsMimeTypeArraySH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsMimeTypeArraySH(aData);
  }
};


// String array helper

class nsStringArraySH : public nsGenericArraySH
{
protected:
  nsStringArraySH(nsDOMClassInfoData* aData) : nsGenericArraySH(aData)
  {
  }

  virtual ~nsStringArraySH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAString& aResult) = 0;

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
};


// History helper

class nsHistorySH : public nsStringArraySH
{
protected:
  nsHistorySH(nsDOMClassInfoData* aData) : nsStringArraySH(aData)
  {
  }

  virtual ~nsHistorySH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAString& aResult);

public:
  NS_IMETHOD GetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsHistorySH(aData);
  }
};

// StringList scriptable helper

class nsStringListSH : public nsStringArraySH
{
protected:
  nsStringListSH(nsDOMClassInfoData* aData) : nsStringArraySH(aData)
  {
  }

  virtual ~nsStringListSH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAString& aResult);

public:
  // Inherit GetProperty, Enumerate from nsStringArraySH
  
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsStringListSH(aData);
  }
};


// MediaList helper

class nsMediaListSH : public nsStringArraySH
{
protected:
  nsMediaListSH(nsDOMClassInfoData* aData) : nsStringArraySH(aData)
  {
  }

  virtual ~nsMediaListSH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAString& aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsMediaListSH(aData);
  }
};


// StyleSheetList helper

class nsStyleSheetListSH : public nsArraySH
{
protected:
  nsStyleSheetListSH(nsDOMClassInfoData* aData) : nsArraySH(aData)
  {
  }

  virtual ~nsStyleSheetListSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsStyleSheetListSH(aData);
  }
};


// CSSValueList helper

class nsCSSValueListSH : public nsArraySH
{
protected:
  nsCSSValueListSH(nsDOMClassInfoData* aData) : nsArraySH(aData)
  {
  }

  virtual ~nsCSSValueListSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsCSSValueListSH(aData);
  }
};


// CSSStyleDeclaration helper

class nsCSSStyleDeclSH : public nsStringArraySH
{
protected:
  nsCSSStyleDeclSH(nsDOMClassInfoData* aData) : nsStringArraySH(aData)
  {
  }

  virtual ~nsCSSStyleDeclSH()
  {
  }

  virtual nsresult GetStringAt(nsISupports *aNative, PRInt32 aIndex,
                               nsAString& aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsCSSStyleDeclSH(aData);
  }
};


// CSSRuleList helper

class nsCSSRuleListSH : public nsArraySH
{
protected:
  nsCSSRuleListSH(nsDOMClassInfoData* aData) : nsArraySH(aData)
  {
  }

  virtual ~nsCSSRuleListSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsCSSRuleListSH(aData);
  }
};


#ifdef MOZ_XUL
// TreeColumns helper

class nsTreeColumnsSH : public nsNamedArraySH
{
protected:
  nsTreeColumnsSH(nsDOMClassInfoData* aData) : nsNamedArraySH(aData)
  {
  }

  virtual ~nsTreeColumnsSH()
  {
  }

  // Override nsArraySH::GetItemAt() since our list isn't a
  // nsIDOMNodeList
  virtual nsresult GetItemAt(nsISupports *aNative, PRUint32 aIndex,
                             nsISupports **aResult);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsTreeColumnsSH(aData);
  }
};
#endif

// WebApps Storage helpers

class nsStorageSH : public nsNamedArraySH
{
protected:
  nsStorageSH(nsDOMClassInfoData* aData) : nsNamedArraySH(aData)
  {
  }

  virtual ~nsStorageSH()
  {
  }

  NS_IMETHOD NewResolve(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                        JSObject *obj, jsval id, PRUint32 flags,
                        JSObject **objp, PRBool *_retval);
  NS_IMETHOD SetProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD DelProperty(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval id, jsval *vp, PRBool *_retval);
  NS_IMETHOD NewEnumerate(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                          JSObject *obj, PRUint32 enum_op, jsval *statep,
                          jsid *idp, PRBool *_retval);

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsStorageSH(aData);
  }
};

class nsStorageListSH : public nsNamedArraySH
{
protected:
  nsStorageListSH(nsDOMClassInfoData* aData) : nsNamedArraySH(aData)
  {
  }

  virtual ~nsStorageListSH()
  {
  }

  // Override nsNamedArraySH::GetNamedItem()
  virtual nsresult GetNamedItem(nsISupports *aNative, const nsAString& aName,
                                nsISupports **aResult);

public:
  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsStorageListSH(aData);
  }
};


// Event handler 'this' translator class, this is called by XPConnect
// when a "function interface" (nsIDOMEventListener) is called, this
// class extracts 'this' fomr the first argument to the called
// function (nsIDOMEventListener::HandleEvent(in nsIDOMEvent)), this
// class will pass back nsIDOMEvent::currentTarget to be used as
// 'this'.

class nsEventListenerThisTranslator : public nsIXPCFunctionThisTranslator
{
public:
  nsEventListenerThisTranslator()
  {
  }

  virtual ~nsEventListenerThisTranslator()
  {
  }

  // nsISupports
  NS_DECL_ISUPPORTS

  // nsIXPCFunctionThisTranslator
  NS_DECL_NSIXPCFUNCTIONTHISTRANSLATOR
};

class nsDOMConstructorSH : public nsDOMGenericSH
{
protected:
  nsDOMConstructorSH(nsDOMClassInfoData* aData) : nsDOMGenericSH(aData)
  {
  }

  virtual ~nsDOMConstructorSH()
  {
  }

public:
  NS_IMETHOD Construct(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                       JSObject *obj, PRUint32 argc, jsval *argv,
                       jsval *vp, PRBool *_retval);

  NS_IMETHOD HasInstance(nsIXPConnectWrappedNative *wrapper, JSContext *cx,
                         JSObject *obj, jsval val, PRBool *bp,
                         PRBool *_retval);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsDOMConstructorSH(aData);
  }
};

class nsNonDOMObjectSH : public nsDOMGenericSH
{
protected:
  nsNonDOMObjectSH(nsDOMClassInfoData* aData) : nsDOMGenericSH(aData)
  {
  }

  virtual ~nsNonDOMObjectSH()
  {
  }

public:
  NS_IMETHOD GetFlags(PRUint32 *aFlags);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsNonDOMObjectSH(aData);
  }
};

// Need this to override GetFlags() on nsNodeSH
class nsAttributeSH : public nsNodeSH
{
protected:
  nsAttributeSH(nsDOMClassInfoData* aData) : nsNodeSH(aData)
  {
  }

  virtual ~nsAttributeSH()
  {
  }

public:
  NS_IMETHOD GetFlags(PRUint32 *aFlags);

  static nsIClassInfo *doCreate(nsDOMClassInfoData* aData)
  {
    return new nsAttributeSH(aData);
  }
};

void InvalidateContextAndWrapperCache();


/**
 * nsIClassInfo helper macros
 */

#define NS_DOM_INTERFACE_MAP_ENTRY_CLASSINFO(_class)                          \
  if (aIID.Equals(NS_GET_IID(nsIClassInfo))) {                                \
    foundInterface =                                                          \
      nsDOMClassInfo::GetClassInfoInstance(eDOMClassInfo_##_class##_id);      \
    if (!foundInterface) {                                                    \
      *aInstancePtr = nsnull;                                                 \
      return NS_ERROR_OUT_OF_MEMORY;                                          \
    }                                                                         \
  } else

#endif /* nsDOMClassInfo_h___ */
