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
 *   David Hyatt <hyatt@netscape.com> (Original Author)
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

#ifndef nsXBLProtoImpl_h__
#define nsXBLProtoImpl_h__

#include "nsMemory.h"
#include "nsXBLPrototypeHandler.h"
#include "nsXBLProtoImplMember.h"
#include "nsXBLPrototypeBinding.h"

class nsIXPConnectJSObjectHolder;

class nsXBLProtoImpl
{
public:
  nsXBLProtoImpl() 
    : mClassObject(nsnull),
      mMembers(nsnull),
      mConstructor(nsnull),
      mDestructor(nsnull)
  { 
    MOZ_COUNT_CTOR(nsXBLProtoImpl); 
  };
  ~nsXBLProtoImpl() 
  { 
    MOZ_COUNT_DTOR(nsXBLProtoImpl);
    // Note: the constructor and destructor are in mMembers, so we'll
    // clean them up automatically.
    for (nsXBLProtoImplMember* curr = mMembers; curr; curr=curr->GetNext())
      curr->Destroy(mClassObject != nsnull);
    delete mMembers; 
  };
  
  nsresult InstallImplementation(nsXBLPrototypeBinding* aBinding, nsIContent* aBoundElement);
  nsresult InitTargetObjects(nsXBLPrototypeBinding* aBinding, nsIScriptContext* aContext, 
                             nsIContent* aBoundElement, 
                             nsIXPConnectJSObjectHolder** aScriptObjectHolder,
                             void** aTargetClassObject);
  nsresult CompilePrototypeMembers(nsXBLPrototypeBinding* aBinding);

  void SetMemberList(nsXBLProtoImplMember* aMemberList) { delete mMembers; mMembers = aMemberList; };

protected:
  // Function to call if compilation of a member fails.  When this is called,
  // all members before aBrokenMember are compiled, compilation of
  // aBrokenMember failed, and members after aBrokenMember are uncompiled.
  // This function assumes that aBrokenMember is _not_ compiled.
  void DestroyMembers(nsXBLProtoImplMember* aBrokenMember);
  
public:
  nsCString mClassName; // The name of the class. 

protected:
  void* mClassObject;   // The class object for the binding. We'll use this to pre-compile properties 
                        // and methods for the binding.

  nsXBLProtoImplMember* mMembers; // The members of an implementation are chained in this singly-linked list.
  
public:
  nsXBLProtoImplAnonymousMethod* mConstructor; // Our class constructor.
  nsXBLProtoImplAnonymousMethod* mDestructor;  // Our class destructor.
};

nsresult
NS_NewXBLProtoImpl(nsXBLPrototypeBinding* aBinding, 
                   const PRUnichar* aClassName, 
                   nsXBLProtoImpl** aResult);

#endif // nsXBLProtoImpl_h__
