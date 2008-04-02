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

#ifndef ns4xPlugin_h__
#define ns4xPlugin_h__

#include "nsIFactory.h"
#include "nsIPlugin.h"
#include "nsIPluginInstancePeer.h"
#include "nsIWindowlessPlugInstPeer.h"
#include "prlink.h"  // for PRLibrary
#include "npupp.h"
#include "nsPluginHostImpl.h"

////////////////////////////////////////////////////////////////////////

/*
 * Use this macro before each exported function
 * (between the return address and the function
 * itself), to ensure that the function has the
 * right calling conventions on Win16.
 */

/* XXX NP_CALLBACK should be the same as NP_LOADDS in npapi.h which differs
   for WIN16 and maybe WIN64? */
#ifdef XP_OS2
#define NP_CALLBACK _System
#else
#define NP_CALLBACK
#endif

#if defined(XP_WIN)
#define NS_4XPLUGIN_CALLBACK(_type, _name) _type (__stdcall * _name)
#elif defined(XP_OS2)
#define NS_4XPLUGIN_CALLBACK(_type, _name) _type (_System * _name)
#else
#define NS_4XPLUGIN_CALLBACK(_type, _name) _type (* _name)
#endif

////////////////////////////////////////////////////////////////////////

// XXX These are defined in platform specific FE directories right now :-/

#if defined(XP_WIN) || defined(XP_UNIX) || defined(XP_BEOS) || defined(XP_OS2)
typedef NS_4XPLUGIN_CALLBACK(NPError, NP_GETENTRYPOINTS) (NPPluginFuncs* pCallbacks);
typedef NS_4XPLUGIN_CALLBACK(NPError, NP_PLUGININIT) (const NPNetscapeFuncs* pCallbacks);
typedef NS_4XPLUGIN_CALLBACK(NPError, NP_PLUGINUNIXINIT) (const NPNetscapeFuncs* pCallbacks,NPPluginFuncs* fCallbacks);
typedef NS_4XPLUGIN_CALLBACK(NPError, NP_PLUGINSHUTDOWN) (void);
#endif

#ifdef XP_MACOSX
typedef NS_4XPLUGIN_CALLBACK(NPError, NP_PLUGINSHUTDOWN) (void);
typedef NS_4XPLUGIN_CALLBACK(NPError, NP_MAIN) (NPNetscapeFuncs* nCallbacks, NPPluginFuncs* pCallbacks, NPP_ShutdownUPP* unloadUpp);

/*  Since WebKit supports getting function pointers via NP_GetEntryPoints and
 *  sending function pointers via NP_Initialize, it would be nice if we
 *  supported that too. We can't do it on PPC because there is no standard for
 *  whether or not function pointers returned via NP_GetEntryPoints or sent
 *  via NP_Initialize are supposed to be wrapped with tvector glue. However,
 *  since there are no tvectors on Intel we can do it on that arch.
 */
#ifndef __POWERPC__
#define MACOSX_GETENTRYPOINT_SUPPORT 1
typedef NS_4XPLUGIN_CALLBACK(NPError, NP_GETENTRYPOINTS) (NPPluginFuncs* pCallbacks);
typedef NS_4XPLUGIN_CALLBACK(NPError, NP_PLUGININIT) (const NPNetscapeFuncs* pCallbacks);
#endif

#endif

class nsIServiceManagerObsolete;
class nsIMemory;

////////////////////////////////////////////////////////////////////////

/**
 * A 5.0 wrapper for a 4.x style plugin.
 */

class ns4xPlugin : public nsIPlugin
{
public:
  ns4xPlugin(NPPluginFuncs* callbacks, PRLibrary* aLibrary,
             NP_PLUGINSHUTDOWN aShutdown,
             nsIServiceManagerObsolete* serviceMgr);
  virtual ~ns4xPlugin(void);

  NS_DECL_ISUPPORTS
  NS_DECL_NSIFACTORY
  NS_DECL_NSIPLUGIN
  
  ////////////////////////////////////////////////////////////////////
  // ns4xPlugin-specific methods

  /**
   * A static factory method for constructing 4.x plugins. Constructs
   * and initializes an ns4xPlugin object, and returns it in
   * <b>result</b>.
   */
   
  static nsresult
  CreatePlugin(nsIServiceManagerObsolete* aServiceMgr,
               const char* aFileName,
               const char* aFullPath,
               PRLibrary* aLibrary,
               nsIPlugin** aResult);

#ifdef XP_MACOSX
  void SetPluginRefNum(short aRefNum);
#endif

protected:
  /**
   * Ensures that the static CALLBACKS is properly initialized
   */
  static void CheckClassInitialized(void);


#ifdef XP_MACOSX
  short fPluginRefNum;
#ifdef MACOSX_GETENTRYPOINT_SUPPORT
  PRBool usesGetEntryPoints;
#endif
#endif

  /**
   * The plugin-side callbacks that the browser calls. One set of
   * plugin callbacks for each plugin.
   */
  NPPluginFuncs fCallbacks;
  PRLibrary*    fLibrary;

  NP_PLUGINSHUTDOWN fShutdownEntry;

  /**
   * The browser-side callbacks that a 4.x-style plugin calls.
   */
  static NPNetscapeFuncs CALLBACKS;
};


PR_BEGIN_EXTERN_C
NPObject* NP_CALLBACK
_getwindowobject(NPP npp);

NPObject* NP_CALLBACK
_getpluginelement(NPP npp);

NPIdentifier NP_CALLBACK
_getstringidentifier(const NPUTF8* name);

void NP_CALLBACK
_getstringidentifiers(const NPUTF8** names, int32_t nameCount,
                      NPIdentifier *identifiers);

bool NP_CALLBACK
_identifierisstring(NPIdentifier identifiers);

NPIdentifier NP_CALLBACK
_getintidentifier(int32_t intid);

NPUTF8* NP_CALLBACK
_utf8fromidentifier(NPIdentifier identifier);

int32_t NP_CALLBACK
_intfromidentifier(NPIdentifier identifier);

NPObject* NP_CALLBACK
_createobject(NPP npp, NPClass* aClass);

NPObject* NP_CALLBACK
_retainobject(NPObject* npobj);

void NP_CALLBACK
_releaseobject(NPObject* npobj);

bool NP_CALLBACK
_invoke(NPP npp, NPObject* npobj, NPIdentifier method, const NPVariant *args,
        uint32_t argCount, NPVariant *result);

bool NP_CALLBACK
_invokeDefault(NPP npp, NPObject* npobj, const NPVariant *args,
               uint32_t argCount, NPVariant *result);

bool NP_CALLBACK
_evaluate(NPP npp, NPObject* npobj, NPString *script, NPVariant *result);

bool NP_CALLBACK
_getproperty(NPP npp, NPObject* npobj, NPIdentifier property,
             NPVariant *result);

bool NP_CALLBACK
_setproperty(NPP npp, NPObject* npobj, NPIdentifier property,
             const NPVariant *value);

bool NP_CALLBACK
_removeproperty(NPP npp, NPObject* npobj, NPIdentifier property);

bool NP_CALLBACK
_hasproperty(NPP npp, NPObject* npobj, NPIdentifier propertyName);

bool NP_CALLBACK
_hasmethod(NPP npp, NPObject* npobj, NPIdentifier methodName);

bool NP_CALLBACK
_enumerate(NPP npp, NPObject *npobj, NPIdentifier **identifier,
           uint32_t *count);

bool NP_CALLBACK
_construct(NPP npp, NPObject* npobj, const NPVariant *args,
           uint32_t argCount, NPVariant *result);

void NP_CALLBACK
_releasevariantvalue(NPVariant *variant);

void NP_CALLBACK
_setexception(NPObject* npobj, const NPUTF8 *message);

PR_END_EXTERN_C

const char *
PeekException();

void
PopException();

void
OnPluginDestroy(NPP instance);

void
OnShutdown();

void
EnterAsyncPluginThreadCallLock();
void
ExitAsyncPluginThreadCallLock();

class NPPStack
{
public:
  static NPP Peek()
  {
    return sCurrentNPP;
  }

protected:
  static NPP sCurrentNPP;
};

// XXXjst: The NPPAutoPusher stack is a bit redundant now that
// PluginDestructionGuard exists, and could thus be replaced by code
// that uses the PluginDestructionGuard list of plugins on the
// stack. But they're not identical, and to minimize code changes
// we're keeping both for the moment, and making NPPAutoPusher inherit
// the PluginDestructionGuard class to avoid having to keep two
// separate objects on the stack since we always want a
// PluginDestructionGuard where we use an NPPAutoPusher.

class NPPAutoPusher : public NPPStack,
                      protected PluginDestructionGuard
{
public:
  NPPAutoPusher(NPP npp)
    : PluginDestructionGuard(npp),
      mOldNPP(sCurrentNPP)
  {
    NS_ASSERTION(npp, "Uh, null npp passed to NPPAutoPusher!");

    sCurrentNPP = npp;
  }

  ~NPPAutoPusher()
  {
    sCurrentNPP = mOldNPP;
  }

private:
  NPP mOldNPP;
};

class NPPExceptionAutoHolder
{
public:
  NPPExceptionAutoHolder();
  ~NPPExceptionAutoHolder();

protected:
  char *mOldException;
};

#endif // ns4xPlugin_h__
