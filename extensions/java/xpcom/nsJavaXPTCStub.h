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
 * The Original Code is Java XPCOM Bindings.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Javier Pedemonte (jhpedemonte@gmail.com)
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

#ifndef _nsJavaXPTCStub_h_
#define _nsJavaXPTCStub_h_

#include "xptcall.h"
#include "jni.h"
#include "nsVoidArray.h"
#include "nsIInterfaceInfo.h"
#include "nsCOMPtr.h"
#include "nsWeakReference.h"
#include "nsJavaXPTCStubWeakRef.h"


#define NS_JAVAXPTCSTUB_IID \
{0x88dd8130, 0xebe6, 0x4431, {0x9d, 0xa7, 0xe6, 0xb7, 0x54, 0x74, 0xfb, 0x21}}

class nsJavaXPTCStub : public nsXPTCStubBase,
                       public nsSupportsWeakReference
{
  friend class nsJavaXPTCStubWeakRef;

public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISUPPORTSWEAKREFERENCE
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_JAVAXPTCSTUB_IID)

  nsJavaXPTCStub(jobject aJavaObject, nsIInterfaceInfo *aIInfo);

  virtual ~nsJavaXPTCStub();

  // return a refcounted pointer to the InterfaceInfo for this object
  // NOTE: on some platforms this MUST not fail or we crash!
  NS_IMETHOD GetInterfaceInfo(nsIInterfaceInfo **aInfo);

  // call this method and return result
  NS_IMETHOD CallMethod(PRUint16 aMethodIndex,
                        const nsXPTMethodInfo *aInfo,
                        nsXPTCMiniVariant *aParams);

  // getter for mJavaObject
  jobject GetJavaObject();

  // Deletes the strong global ref for the Java object, so it can be garbage
  // collected if necessary.  See DestroyXPTCMappingEnum().
  void DeleteStrongRef();

private:
  NS_IMETHOD_(nsrefcnt) AddRefInternal();
  NS_IMETHOD_(nsrefcnt) ReleaseInternal();

  // Deletes this object and its members.  Called by ReleaseInternal() and
  // ReleaseWeakRef().
  void Destroy();

  // When a nsJavaXPTCStubWeakRef associated with this object is released, it
  // calls this function to let this object know that there is one less weak
  // ref.  If there are no more weakrefs referencing this object, and no one
  // holds a strong ref, then this function takes care of deleting the object.
  void ReleaseWeakRef();

  // returns a weak reference to a child supporting the specified interface
  nsJavaXPTCStub * FindStubSupportingIID(const nsID &aIID);

  // returns true if this stub supports the specified interface
  PRBool SupportsIID(const nsID &aIID);

  nsresult SetupJavaParams(const nsXPTParamInfo &aParamInfo,
                           const nsXPTMethodInfo* aMethodInfo,
                           PRUint16 aMethodIndex,
                           nsXPTCMiniVariant* aDispatchParams,
                           nsXPTCMiniVariant &aVariant,
                           jvalue &aJValue, nsACString &aMethodSig);
  nsresult GetRetvalSig(const nsXPTParamInfo* aParamInfo,
                        const nsXPTMethodInfo* aMethodInfo,
                        PRUint16 aMethodIndex,
                        nsXPTCMiniVariant* aDispatchParams,
                        nsACString &aRetvalSig);
  nsresult FinalizeJavaParams(const nsXPTParamInfo &aParamInfo,
                              const nsXPTMethodInfo* aMethodInfo,
                              PRUint16 aMethodIndex,
                              nsXPTCMiniVariant* aDispatchParams,
                              nsXPTCMiniVariant &aVariant,
                              jvalue &aJValue);
  nsresult SetXPCOMRetval();

  jobject                     mJavaWeakRef;
  jobject                     mJavaStrongRef;
  jint                        mJavaRefHashCode;
  nsCOMPtr<nsIInterfaceInfo>  mIInfo;

  nsVoidArray     mChildren; // weak references (cleared by the children)
  nsJavaXPTCStub *mMaster;   // strong reference

  nsAutoRefCnt    mWeakRefCnt;  // count for number of associated weak refs
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsJavaXPTCStub, NS_JAVAXPTCSTUB_IID)

#endif // _nsJavaXPTCStub_h_
