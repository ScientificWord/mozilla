/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1998-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Norris Boyd  <nboyd@atg.com>
 *   Mitch Stoltz <mstoltz@netscape.com>
 *   Christopher A. Aillon <christopher@aillon.com>
 *   Giorgio Maone <g.maone@informaction.com>
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

#ifndef nsScriptSecurityManager_h__
#define nsScriptSecurityManager_h__

#include "nsIScriptSecurityManager.h"
#include "nsIPrincipal.h"
#include "jsapi.h"
#include "jsdbgapi.h"
#include "nsIXPCSecurityManager.h"
#include "nsInterfaceHashtable.h"
#include "nsHashtable.h"
#include "nsCOMPtr.h"
#include "nsIPrefService.h"
#include "nsISecurityPref.h"
#include "nsIChannelEventSink.h"
#include "nsIJSContextStack.h"
#include "nsIObserver.h"
#include "pldhash.h"
#include "plstr.h"

class nsIDocShell;
class nsString;
class nsIClassInfo;
class nsIIOService;
class nsIXPConnect;
class nsIStringBundle;
class nsSystemPrincipal;
struct ClassPolicy;
class ClassInfoData;
class DomainPolicy;

#if defined(DEBUG_mstoltz) || defined(DEBUG_caillon)
#define DEBUG_CAPS_HACKER
#endif

#ifdef DEBUG_CAPS_HACKER
#define DEBUG_CAPS_CheckPropertyAccessImpl
#define DEBUG_CAPS_LookupPolicy
#define DEBUG_CAPS_CheckComponentPermissions
#endif

#if 0
#define DEBUG_CAPS_CanCreateWrapper
#define DEBUG_CAPS_CanCreateInstance
#define DEBUG_CAPS_CanGetService
#define DEBUG_CAPS_DomainPolicyLifeCycle
#endif

/////////////////////
// PrincipalKey //
/////////////////////

class PrincipalKey : public PLDHashEntryHdr
{
public:
    typedef const nsIPrincipal* KeyType;
    typedef const nsIPrincipal* KeyTypePointer;

    PrincipalKey(const nsIPrincipal* key)
      : mKey(const_cast<nsIPrincipal*>(key))
    {
    }

    PrincipalKey(const PrincipalKey& toCopy)
      : mKey(toCopy.mKey)
    {
    } 

    ~PrincipalKey()
    {
    }

    KeyType GetKey() const
    {
        return mKey;
    }

    PRBool KeyEquals(KeyTypePointer aKey) const
    {
        PRBool eq;
        mKey->Equals(const_cast<nsIPrincipal*>(aKey),
                     &eq);
        return eq;
    }

    static KeyTypePointer KeyToPointer(KeyType aKey)
    {
        return aKey;
    }

    static PLDHashNumber HashKey(KeyTypePointer aKey)
    {
        PRUint32 hash;
        const_cast<nsIPrincipal*>(aKey)->GetHashValue(&hash);
        return PLDHashNumber(hash);
    }

    enum { ALLOW_MEMMOVE = PR_TRUE };

private:
    nsCOMPtr<nsIPrincipal> mKey;
};

////////////////////
// Policy Storage //
////////////////////

// Property Policy
union SecurityLevel
{
    PRInt32  level;
    char*    capability;
};

// Security levels
// These values all have the low bit set (except UNDEFINED_ACCESS)
// to distinguish them from pointer values, because no pointer
// to allocated memory ever has the low bit set. A SecurityLevel
// contains either one of these constants or a pointer to a string
// representing the name of a capability.

#define SCRIPT_SECURITY_UNDEFINED_ACCESS 0
#define SCRIPT_SECURITY_ACCESS_IS_SET_BIT 1
#define SCRIPT_SECURITY_NO_ACCESS \
  ((1 << 0) | SCRIPT_SECURITY_ACCESS_IS_SET_BIT)
#define SCRIPT_SECURITY_SAME_ORIGIN_ACCESS \
  ((1 << 1) | SCRIPT_SECURITY_ACCESS_IS_SET_BIT)
#define SCRIPT_SECURITY_ALL_ACCESS \
  ((1 << 2) | SCRIPT_SECURITY_ACCESS_IS_SET_BIT)

#define SECURITY_ACCESS_LEVEL_FLAG(_sl) \
           ((_sl.level == 0) || \
            (_sl.level & SCRIPT_SECURITY_ACCESS_IS_SET_BIT))


struct PropertyPolicy : public PLDHashEntryHdr
{
    jsval          key;  // property name as jsval
    SecurityLevel  mGet;
    SecurityLevel  mSet;
};

PR_STATIC_CALLBACK(PRBool)
InitPropertyPolicyEntry(PLDHashTable *table,
                     PLDHashEntryHdr *entry,
                     const void *key)
{
    PropertyPolicy* pp = (PropertyPolicy*)entry;
    pp->key = (jsval)key;
    pp->mGet.level = SCRIPT_SECURITY_UNDEFINED_ACCESS;
    pp->mSet.level = SCRIPT_SECURITY_UNDEFINED_ACCESS;
    return PR_TRUE;
}

PR_STATIC_CALLBACK(void)
ClearPropertyPolicyEntry(PLDHashTable *table, PLDHashEntryHdr *entry)
{
    PropertyPolicy* pp = (PropertyPolicy*)entry;
    pp->key = JSVAL_VOID;
}

// Class Policy
#define NO_POLICY_FOR_CLASS (ClassPolicy*)1

struct ClassPolicy : public PLDHashEntryHdr
{
    char* key;
    PLDHashTable* mPolicy;

    // Note: the DomainPolicy owns us, so if if dies we will too.  Hence no
    // need to refcount it here (and in fact, we'd probably leak if we tried).
    DomainPolicy* mDomainWeAreWildcardFor;
};

PR_STATIC_CALLBACK(void)
ClearClassPolicyEntry(PLDHashTable *table, PLDHashEntryHdr *entry)
{
    ClassPolicy* cp = (ClassPolicy *)entry;
    if (cp->key)
    {
        PL_strfree(cp->key);
        cp->key = nsnull;
    }
    PL_DHashTableDestroy(cp->mPolicy);
}

// Note: actual impl is going to be after the DomainPolicy class definition,
// since we need to access members of DomainPolicy in the impl
PR_STATIC_CALLBACK(void)
MoveClassPolicyEntry(PLDHashTable *table,
                     const PLDHashEntryHdr *from,
                     PLDHashEntryHdr *to);

PR_STATIC_CALLBACK(PRBool)
InitClassPolicyEntry(PLDHashTable *table,
                     PLDHashEntryHdr *entry,
                     const void *key)
{
    static PLDHashTableOps classPolicyOps =
    {
        PL_DHashAllocTable,
        PL_DHashFreeTable,
        PL_DHashVoidPtrKeyStub,
        PL_DHashMatchEntryStub,
        PL_DHashMoveEntryStub,
        ClearPropertyPolicyEntry,
        PL_DHashFinalizeStub,
        InitPropertyPolicyEntry
    };

    ClassPolicy* cp = (ClassPolicy*)entry;
    cp->mDomainWeAreWildcardFor = nsnull;
    cp->key = PL_strdup((const char*)key);
    if (!cp->key)
        return PR_FALSE;
    cp->mPolicy = PL_NewDHashTable(&classPolicyOps, nsnull,
                                   sizeof(PropertyPolicy), 16);
    if (!cp->mPolicy) {
        PL_strfree(cp->key);
        cp->key = nsnull;
        return PR_FALSE;
    }
    return PR_TRUE;
}

// Domain Policy
class DomainPolicy : public PLDHashTable
{
public:
    DomainPolicy() : mWildcardPolicy(nsnull),
                     mRefCount(0)
    {
        mGeneration = sGeneration;

#ifdef DEBUG_CAPS_DomainPolicyLifeCycle
        ++sObjects;
        _printPopulationInfo();
#endif

    }

    PRBool Init()
    {
        static const PLDHashTableOps domainPolicyOps =
        {
            PL_DHashAllocTable,
            PL_DHashFreeTable,
            PL_DHashStringKey,
            PL_DHashMatchStringKey,
            MoveClassPolicyEntry,
            ClearClassPolicyEntry,
            PL_DHashFinalizeStub,
            InitClassPolicyEntry
        };

        return PL_DHashTableInit(this, &domainPolicyOps, nsnull,
                                 sizeof(ClassPolicy), 16);
    }

    ~DomainPolicy()
    {
        PL_DHashTableFinish(this);
        NS_ASSERTION(mRefCount == 0, "Wrong refcount in DomainPolicy dtor");
#ifdef DEBUG_CAPS_DomainPolicyLifeCycle
        printf("DomainPolicy deleted with mRefCount = %d\n", mRefCount);
        --sObjects;
        _printPopulationInfo();
#endif

    }

    void Hold()
    {
        mRefCount++;
    }

    void Drop()
    {
        if (--mRefCount == 0)
            delete this;
    }
    
    static void InvalidateAll()
    {
        sGeneration++;
    }
    
    PRBool IsInvalid()
    {
        return mGeneration != sGeneration; 
    }
    
    ClassPolicy* mWildcardPolicy;

private:
    PRUint32 mRefCount;
    PRUint32 mGeneration;
    static PRUint32 sGeneration;
    
#ifdef DEBUG_CAPS_DomainPolicyLifeCycle
    static PRUint32 sObjects;
    static void _printPopulationInfo();
#endif

};

PR_STATIC_CALLBACK(void)
MoveClassPolicyEntry(PLDHashTable *table,
                     const PLDHashEntryHdr *from,
                     PLDHashEntryHdr *to)
{
    memcpy(to, from, table->entrySize);

    // Now update the mDefaultPolicy pointer that points to us, if any.
    ClassPolicy* cp = static_cast<ClassPolicy*>(to);
    if (cp->mDomainWeAreWildcardFor) {
        NS_ASSERTION(cp->mDomainWeAreWildcardFor->mWildcardPolicy ==
                     static_cast<const ClassPolicy*>(from),
                     "Unexpected wildcard policy on mDomainWeAreWildcardFor");
        cp->mDomainWeAreWildcardFor->mWildcardPolicy = cp;
    }
}

/////////////////////////////
// nsScriptSecurityManager //
/////////////////////////////
#define NS_SCRIPTSECURITYMANAGER_CID \
{ 0x7ee2a4c0, 0x4b93, 0x17d3, \
{ 0xba, 0x18, 0x00, 0x60, 0xb0, 0xf1, 0x99, 0xa2 }}

class nsScriptSecurityManager : public nsIScriptSecurityManager,
                                public nsIPrefSecurityCheck,
                                public nsIChannelEventSink,
                                public nsIObserver
{
public:
    static void Shutdown();
    
    NS_DEFINE_STATIC_CID_ACCESSOR(NS_SCRIPTSECURITYMANAGER_CID)
        
    NS_DECL_ISUPPORTS
    NS_DECL_NSISCRIPTSECURITYMANAGER
    NS_DECL_NSIXPCSECURITYMANAGER
    NS_DECL_NSIPREFSECURITYCHECK
    NS_DECL_NSICHANNELEVENTSINK
    NS_DECL_NSIOBSERVER

    static nsScriptSecurityManager*
    GetScriptSecurityManager();

    static nsSystemPrincipal*
    SystemPrincipalSingletonConstructor();

    JSContext* GetCurrentJSContext();

    JSContext* GetSafeJSContext();

    /**
     * Utility method for comparing two URIs.  For security purposes, two URIs
     * are equivalent if their schemes, hosts, and ports (if any) match.  This
     * method returns true if aSubjectURI and aObjectURI have the same origin,
     * false otherwise.
     */
    static PRBool SecurityCompareURIs(nsIURI* aSourceURI, nsIURI* aTargetURI);

    static nsresult 
    ReportError(JSContext* cx, const nsAString& messageTag,
                nsIURI* aSource, nsIURI* aTarget);

private:

    // GetScriptSecurityManager is the only call that can make one
    nsScriptSecurityManager();
    virtual ~nsScriptSecurityManager();

    static JSBool JS_DLL_CALLBACK
    CheckObjectAccess(JSContext *cx, JSObject *obj,
                      jsval id, JSAccessMode mode,
                      jsval *vp);

    // Returns null if a principal cannot be found; generally callers
    // should error out at that point.
    static nsIPrincipal*
    doGetObjectPrincipal(JSObject *obj
#ifdef DEBUG
                         , PRBool aAllowShortCircuit = PR_TRUE
#endif
                         );

    // Returns null if a principal cannot be found.  Note that rv can be NS_OK
    // when this happens -- this means that there was no JS running.
    nsIPrincipal*
    doGetSubjectPrincipal(nsresult* rv);
    
    nsresult
    CheckPropertyAccessImpl(PRUint32 aAction,
                            nsAXPCNativeCallContext* aCallContext,
                            JSContext* cx, JSObject* aJSObject,
                            nsISupports* aObj, nsIURI* aTargetURI,
                            nsIClassInfo* aClassInfo,
                            const char* aClassName, jsval aProperty,
                            void** aCachedClassPolicy);

    nsresult
    CheckSameOriginPrincipalInternal(nsIPrincipal* aSubject,
                                     nsIPrincipal* aObject,
                                     PRBool aIsCheckConnect);

    nsresult
    CheckSameOriginDOMProp(nsIPrincipal* aSubject, 
                           nsIPrincipal* aObject,
                           PRUint32 aAction,
                           PRBool aIsCheckConnect);

    nsresult
    LookupPolicy(nsIPrincipal* principal,
                 ClassInfoData& aClassData, jsval aProperty,
                 PRUint32 aAction,
                 ClassPolicy** aCachedClassPolicy,
                 SecurityLevel* result);

    nsresult
    CreateCodebasePrincipal(nsIURI* aURI, nsIPrincipal** result);

    // This is just like the API method, but it doesn't check that the subject
    // name is non-empty or aCertificate is non-null, and it doesn't change the
    // certificate in the table (if any) in any way if aModifyTable is false.
    nsresult
    DoGetCertificatePrincipal(const nsACString& aCertFingerprint,
                              const nsACString& aSubjectName,
                              const nsACString& aPrettyName,
                              nsISupports* aCertificate,
                              nsIURI* aURI,
                              PRBool aModifyTable,
                              nsIPrincipal **result);

    // Returns null if a principal cannot be found.  Note that rv can be NS_OK
    // when this happens -- this means that there was no script for the
    // context.  Callers MUST pass in a non-null rv here.
    static nsIPrincipal*
    GetSubjectPrincipal(JSContext* cx, nsresult* rv);

    // Returns null if a principal cannot be found.  Note that rv can be NS_OK
    // when this happens -- this means that there was no script for the frame.
    // Callers MUST pass in a non-null rv here.
    static nsIPrincipal*
    GetFramePrincipal(JSContext* cx, JSStackFrame* fp, nsresult* rv);
                                                     
    // Returns null if a principal cannot be found.  Note that rv can be NS_OK
    // when this happens -- this means that there was no script.  Callers MUST
    // pass in a non-null rv here.
    static nsIPrincipal*
    GetScriptPrincipal(JSContext* cx, JSScript* script, nsresult* rv);

    // Returns null if a principal cannot be found.  Note that rv can be NS_OK
    // when this happens -- this means that there was no script associated
    // with the function object, and no global object associated with the scope
    // of obj (the last object on its parent chain).  If the caller is walking
    // the JS stack, fp must point to the current frame in the stack iteration.
    // Callers MUST pass in a non-null rv here.
    static nsIPrincipal*
    GetFunctionObjectPrincipal(JSContext* cx, JSObject* obj, JSStackFrame *fp,
                               nsresult* rv);

    // Returns null if a principal cannot be found.  Note that rv can be NS_OK
    // when this happens -- this means that there was no script
    // running.  Callers MUST pass in a non-null rv here.
    static nsIPrincipal*
    GetPrincipalAndFrame(JSContext *cx,
                         JSStackFrame** frameResult,
                         nsresult* rv);

    static PRBool
    CheckConfirmDialog(JSContext* cx, nsIPrincipal* aPrincipal,
                       const char* aCapability, PRBool *checkValue);

    static void
    FormatCapabilityString(nsAString& aCapability);

    nsresult
    SavePrincipal(nsIPrincipal* aToSave);

    nsresult
    CheckXPCPermissions(nsISupports* aObj,
                        const char* aObjectSecurityLevel);

    nsresult
    Init();
    
    nsresult
    InitPrefs();

    static nsresult 
    GetPrincipalPrefNames(const char* prefBase,
                          nsCString& grantedPref,
                          nsCString& deniedPref,
                          nsCString& subjectNamePref);

    nsresult
    InitPolicies();

    nsresult
    InitDomainPolicy(JSContext* cx, const char* aPolicyName,
                     DomainPolicy* aDomainPolicy);

    nsresult
    InitPrincipals(PRUint32 prefCount, const char** prefNames,
                   nsISecurityPref* securityPref);


    /* encapsulate the file comparison rules */
    static PRBool SecurityCompareFileURIs(nsIURI* aSourceURI,
                                          nsIURI* aTargetURI);

#ifdef XPC_IDISPATCH_SUPPORT
    // While this header is included outside of caps, this class isn't 
    // referenced so this should be fine.
    nsresult
    CheckComponentPermissions(JSContext *cx, const nsCID &aCID);
#endif
#ifdef DEBUG_CAPS_HACKER
    void
    PrintPolicyDB();
#endif

    // JS strings we need to clean up on shutdown
    static jsval sEnabledID;

    inline void
    ScriptSecurityPrefChanged();

    static const char sJSEnabledPrefName[];
    static const char sJSMailEnabledPrefName[];
    static const char sFileOriginPolicyPrefName[];

    nsObjectHashtable* mOriginToPolicyMap;
    DomainPolicy* mDefaultPolicy;
    nsObjectHashtable* mCapabilities;

    nsCOMPtr<nsIPrefBranch> mPrefBranch;
    nsCOMPtr<nsISecurityPref> mSecurityPref;
    nsCOMPtr<nsIPrincipal> mSystemPrincipal;
    nsCOMPtr<nsIPrincipal> mSystemCertificate;
    nsInterfaceHashtable<PrincipalKey, nsIPrincipal> mPrincipals;
    nsCOMPtr<nsIThreadJSContextStack> mJSContextStack;
    PRPackedBool mIsJavaScriptEnabled;
    PRPackedBool mIsMailJavaScriptEnabled;
    PRPackedBool mIsWritingPrefs;
    PRPackedBool mPolicyPrefsChanged;
#ifdef XPC_IDISPATCH_SUPPORT    
    PRPackedBool mXPCDefaultGrantAll;
    static const char sXPCDefaultGrantAllName[];
#endif

    static PRInt32 sFileURIOriginPolicy;

    static nsIIOService    *sIOService;
    static nsIXPConnect    *sXPConnect;
    static nsIStringBundle *sStrBundle;
    static JSRuntime       *sRuntime;
};

// Levels for file: URI same-origin policy:
//   self:        same-origin only with itself
//   samedir:     same-origin with files having the same path
//   subdir:      same-origin with files having longer paths (asymetric)
//   anyfile:     same-origin with any other file: URI (but not directories)
//   traditional: any local file, any directory
#define FILEURI_SOP_SELF        0
#define FILEURI_SOP_SAMEDIR     1
#define FILEURI_SOP_SUBDIR      2
#define FILEURI_SOP_ANYFILE     3
#define FILEURI_SOP_TRADITIONAL 4

#endif // nsScriptSecurityManager_h__
