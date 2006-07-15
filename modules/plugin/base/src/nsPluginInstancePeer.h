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

#ifndef nsPluginInstancePeer_h___
#define nsPluginInstancePeer_h___

#include "nsIPluginInstancePeer2.h"
#include "nsIWindowlessPlugInstPeer.h"
#include "nsIPluginTagInfo2.h"
#include "nsIPluginInstanceOwner.h"
#ifdef OJI
#include "nsIJVMPluginTagInfo.h"
#endif
#include "nsPIPluginInstancePeer.h"

class nsPluginInstancePeerImpl : public nsIPluginInstancePeer2,
                                 public nsIWindowlessPluginInstancePeer,
                                 public nsIPluginTagInfo2,
#ifdef OJI
                                 public nsIJVMPluginTagInfo,
#endif
                                 public nsPIPluginInstancePeer
								
{
public:
  nsPluginInstancePeerImpl();
  virtual ~nsPluginInstancePeerImpl();

  NS_DECL_ISUPPORTS

  // nsIPluginInstancePeer interface

  NS_IMETHOD
  GetValue(nsPluginInstancePeerVariable variable, void *value);

  NS_IMETHOD
  GetMIMEType(nsMIMEType *result);

  NS_IMETHOD
  GetMode(nsPluginMode *result);

  NS_IMETHOD
  NewStream(nsMIMEType type, const char* target, nsIOutputStream* *result);

  NS_IMETHOD
  ShowStatus(const char* message);

  NS_IMETHOD
  SetWindowSize(PRUint32 width, PRUint32 height);

  // nsIPluginInstancePeer2 interface

  NS_IMETHOD
  GetJSWindow(JSObject* *outJSWindow);
  
  NS_IMETHOD
  GetJSThread(PRUint32 *outThreadID);

  NS_IMETHOD
  GetJSContext(JSContext* *outContext);

  // nsIWindowlessPluginInstancePeer

  // (Corresponds to NPN_InvalidateRect.)
  NS_IMETHOD
  InvalidateRect(nsPluginRect *invalidRect);

  // (Corresponds to NPN_InvalidateRegion.)
  NS_IMETHOD
  InvalidateRegion(nsPluginRegion invalidRegion);

  // (Corresponds to NPN_ForceRedraw.)
  NS_IMETHOD
  ForceRedraw(void);

  /* The tag info interfaces all pass through calls to the 
     nsPluginInstanceOwner (see nsObjectFrame.cpp) */

  //nsIPluginTagInfo interface

  NS_IMETHOD
  GetAttributes(PRUint16& n, const char*const*& names, const char*const*& values);

  NS_IMETHOD
  GetAttribute(const char* name, const char* *result);

  NS_IMETHOD
  GetDOMElement(nsIDOMElement* *result);

  //nsIPluginTagInfo2 interface

  NS_IMETHOD
  GetTagType(nsPluginTagType *result);

  NS_IMETHOD
  GetTagText(const char* *result);

  NS_IMETHOD
  GetParameters(PRUint16& n, const char*const*& names, const char*const*& values);

  NS_IMETHOD
  GetParameter(const char* name, const char* *result);
  
  NS_IMETHOD
  GetDocumentBase(const char* *result);
  
  NS_IMETHOD
  GetDocumentEncoding(const char* *result);
  
  NS_IMETHOD
  GetAlignment(const char* *result);
  
  NS_IMETHOD
  GetWidth(PRUint32 *result);
  
  NS_IMETHOD
  GetHeight(PRUint32 *result);
  
  NS_IMETHOD
  GetBorderVertSpace(PRUint32 *result);
  
  NS_IMETHOD
  GetBorderHorizSpace(PRUint32 *result);

  NS_IMETHOD
  GetUniqueID(PRUint32 *result);

  //nsIJVMPluginTagInfo interface

  NS_IMETHOD
  GetCode(const char* *result);

  NS_IMETHOD
  GetCodeBase(const char* *result);

  NS_IMETHOD
  GetArchive(const char* *result);

  NS_IMETHOD
  GetName(const char* *result);

  NS_IMETHOD
  GetMayScript(PRBool *result);

  // nsPIPluginInstancePeer interface

  NS_IMETHOD
  GetOwner(nsIPluginInstanceOwner **aOwner);

  //locals

  nsresult Initialize(nsIPluginInstanceOwner *aOwner,
                      const nsMIMEType aMimeType);

  nsresult SetOwner(nsIPluginInstanceOwner *aOwner);

private:
  nsIPluginInstance       *mInstance; //we don't add a ref to this
  nsIPluginInstanceOwner  *mOwner;    //we don't add a ref to this
  nsMIMEType              mMIMEType;
  PRUint32                mThreadID;
  PRBool                  mStopped;
};

#endif
