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

/*
	MRJPlugin.cpp
	
	XP COM Plugin Implementation.
	
	by Patrick C. Beard.
 */

#include "MRJPlugin.h"
#include "MRJSession.h"
#include "MRJContext.h"
#include "MRJFrame.h"
#include "MRJConsole.h"
#include "EmbeddedFramePluginInstance.h"

#include "nsIServiceManager.h"
#include "nsMemory.h"
#include "nsIComponentManager.h"
#include "nsIJVMManager.h"
#include "nsIJVMPluginTagInfo.h"
#include "nsIPluginManager2.h"
#include "nsIPluginInstancePeer.h"
#include "nsIWindowlessPlugInstPeer.h"
#include "LiveConnectNativeMethods.h"
#include "CSecureEnv.h"
#include "EventFilter.h"
#include "nsObsoleteModuleLoading.h"

#include <Resources.h>

static nsIServiceManager* theServiceManager = NULL;
static nsIServiceManagerObsolete* theServiceManagerObsolete = NULL;

extern nsIServiceManager* theServiceManager;	// needs to be in badaptor.cpp.
extern nsIPluginManager* thePluginManager;		// now in badaptor.cpp.
extern nsIPlugin* thePlugin;

nsIPluginManager2* thePluginManager2 = NULL;
nsIMemory* theMemoryAllocator = NULL;		// should also be provided by badaptor.cpp.

FSSpec thePluginSpec;
short thePluginRefnum = -1;

// Common interface IDs.

static NS_DEFINE_IID(kPluginCID, NS_PLUGIN_CID);
static NS_DEFINE_IID(kPluginManagerCID, NS_PLUGINMANAGER_CID);
#ifdef MRJPLUGIN_4X	
static NS_DEFINE_IID(kMemoryCID, NS_MEMORY_CID);
#endif
static NS_DEFINE_IID(kJVMManagerCID, NS_JVMMANAGER_CID);

static NS_DEFINE_IID(kIWindowlessPluginInstancePeerIID, NS_IWINDOWLESSPLUGININSTANCEPEER_IID);

const char* MRJPlugin::PLUGIN_VERSION = "eerieQuarkDoll.v.b1";


nsresult MRJPlugin::GetService(const nsCID& aCID, const nsIID& aIID, void* *aService)
{
    if (theServiceManager)
        return theServiceManager->GetService(aCID, aIID, aService);
    if (theServiceManagerObsolete)
        return theServiceManagerObsolete->GetService(aCID, aIID, (nsISupports **)aService);
    return NS_ERROR_FAILURE;
}

nsresult MRJPlugin::GetService(const char* aContractID, const nsIID& aIID, void* *aService)
{
    if (theServiceManager)
        return theServiceManager->GetServiceByContractID(aContractID, aIID, aService);
    if (theServiceManagerObsolete)
        return theServiceManagerObsolete->GetService(aContractID, aIID, (nsISupports **)aService);
    return NS_ERROR_FAILURE;
}

#pragma export on

nsresult NSGetFactory(nsISupports* serviceManager, const nsCID &aClass, const char *aClassName, const char *aContractID, nsIFactory **aFactory)
{
    if (theServiceManager == NULL && theServiceManagerObsolete == NULL) {
        if (NS_FAILED(serviceManager->QueryInterface(NS_GET_IID(nsIServiceManager), (void**)&theServiceManager)))
            if (NS_FAILED(serviceManager->QueryInterface(NS_GET_IID(nsIServiceManagerObsolete), (void**)&theServiceManagerObsolete)))
                return NS_ERROR_FAILURE;

        // Our global operator new wants to use nsIMalloc to do all of its allocation.
        // This should be available from the Service Manager.
#ifdef MRJPLUGIN_4X    
        if (MRJPlugin::GetService(kMemoryCID, NS_GET_IID(nsIMemory), (void**)&theMemoryAllocator) != NS_OK)
            return NS_ERROR_FAILURE;
#else
        if (NS_FAILED(MRJPlugin::GetService("@mozilla.org/xpcom/memory-service;1", NS_GET_IID(nsIMemory), (void **)&theMemoryAllocator)))
            return NS_ERROR_FAILURE;
#endif
    }

    if (!aClass.Equals(kPluginCID))
        return NS_NOINTERFACE;

    MRJPlugin* pluginFactory = new MRJPlugin();
    if (!pluginFactory)
        return NS_ERROR_OUT_OF_MEMORY;

    pluginFactory->AddRef();
    *aFactory = pluginFactory;
    return NS_OK;
}

#pragma export off

extern "C" {

pascal OSErr __initialize(const CFragInitBlock *initBlock);
pascal void __terminate(void);

#if defined(MRJPLUGIN_GC)
pascal OSErr __NSInitialize(const CFragInitBlock* initBlock);
pascal void __NSTerminate(void);
#define __initialize __NSInitialize
#define __terminate __NSTerminate
#endif

pascal OSErr MRJPlugin__initialize(const CFragInitBlock *initBlock);
pascal void MRJPlugin__terminate(void);

}

pascal OSErr MRJPlugin__initialize(const CFragInitBlock *initBlock)
{
	OSErr err = __initialize(initBlock);
	if (err != noErr) return err;

	if (initBlock->fragLocator.where == kDataForkCFragLocator) {
		thePluginSpec = *initBlock->fragLocator.u.onDisk.fileSpec;
	
		// is it always the case that the plugin's resource file is open now?
		thePluginRefnum = ::FSpOpenResFile(&thePluginSpec, fsRdPerm);
	}
	
	return noErr;
}

pascal void MRJPlugin__terminate()
{
#ifdef MRJPLUGIN_4X	
	// Make sure the event filters are removed.
	RemoveEventFilters();
#endif

	__terminate();

    // last ditch release of the memory allocator.
    if (theMemoryAllocator != NULL) {
        theMemoryAllocator->Release();
        theMemoryAllocator = NULL;
    }
}

//
// The MEAT of the plugin.
//

#pragma mark *** MRJPlugin ***

const InterfaceInfo MRJPlugin::sInterfaces[] = {
	{ NS_IPLUGIN_IID, INTERFACE_OFFSET(MRJPlugin, nsIPlugin) },
	{ NS_IJVMPLUGIN_IID, INTERFACE_OFFSET(MRJPlugin, nsIJVMPlugin) },
	{ NS_IRUNNABLE_IID, INTERFACE_OFFSET(MRJPlugin, nsIRunnable) },
};
const UInt32 MRJPlugin::kInterfaceCount = sizeof(sInterfaces) / sizeof(InterfaceInfo);

MRJPlugin::MRJPlugin()
	:	SupportsMixin(this, sInterfaces, kInterfaceCount),
		mManager(NULL), mThreadManager(NULL), mSession(NULL), mConsole(NULL), mIsEnabled(false), mPluginThreadID(NULL)
{
    // make this singleton instance visible.
    ::thePlugin = this;
}

MRJPlugin::~MRJPlugin()
{
    // make sure the plugin is no longer visible.
    ::thePlugin = NULL;

    // Release the console.
    NS_IF_RELEASE(mConsole);

    // tear down the MRJ session, if it exists.
    delete mSession;

    // Release the manager?
    NS_IF_RELEASE(mManager);
    NS_IF_RELEASE(mThreadManager);
}

/**
 * MRJPlugin aggregates MRJConsole, so that it can be QI'd to be an nsIJVMConsole.
 * To save code size, we use the SupportsMixin class instead of the macros in 
 * nsAgg.h. SupportsMixin::queryInterface, addRef, and release are all local
 * operations, regardless of aggregation. The capitalized versions take aggregation
 * into account.
 */
NS_METHOD MRJPlugin::QueryInterface(const nsIID& aIID, void** instancePtr)
{
    nsresult result = queryInterface(aIID, instancePtr);
    if (result == NS_NOINTERFACE) {
        if (!mConsole)
            return NS_ERROR_NOT_AVAILABLE;

        result = mConsole->queryInterface(aIID, instancePtr);
    }
    return result;
}

NS_METHOD MRJPlugin::CreateInstance(nsISupports *aOuter, const nsIID& aIID, void **aResult)
{
	nsresult result = StartupJVM();
	if (result == NS_OK) {
		MRJPluginInstance* instance = new MRJPluginInstance(this);
		if (instance == nsnull)
			return NS_ERROR_OUT_OF_MEMORY;
		result = instance->QueryInterface(aIID, aResult);
		if (result != NS_OK)
			delete instance;
	}
	return result;
}

#define NS_APPLET_MIME_TYPE "application/x-java-applet"

NS_METHOD MRJPlugin::CreatePluginInstance(nsISupports *aOuter, REFNSIID aIID, const char* aPluginMIMEType, void **aResult)
{
    nsresult result = NS_NOINTERFACE;

    if (::strcmp(aPluginMIMEType, "application/x-java-frame") == 0) {
        // create a special plugin instance that manages an embedded frame.
        EmbeddedFramePluginInstance* instance = new EmbeddedFramePluginInstance();
        if (!instance)
            return NS_ERROR_OUT_OF_MEMORY;
            
        nsresult result = instance->QueryInterface(aIID, aResult);
        if (result != NS_OK)
            delete instance;
    } else {
        // assume it's some kind of an applet.
        result = CreateInstance(aOuter, aIID, aResult);
    }
    return result;
}

NS_METHOD MRJPlugin::Initialize()
{
    nsresult result = NS_OK;

    // try to get a plugin manager.
    if (thePluginManager == NULL) {
        result = MRJPlugin::GetService(kPluginManagerCID, NS_GET_IID(nsIPluginManager), (void**)&thePluginManager);
        if (result != NS_OK || thePluginManager == NULL)
            return NS_ERROR_FAILURE;
    }

    // see if the enhanced plugin manager exists.
    if (thePluginManager2 == NULL) {
        if (thePluginManager->QueryInterface(NS_GET_IID(nsIPluginManager2), (void**)&thePluginManager2) != NS_OK)
            thePluginManager2 = NULL;
    }

    // try to get a JVM manager. we have to be able to run without one.
    if (MRJPlugin::GetService(kJVMManagerCID, NS_GET_IID(nsIJVMManager), (void**)&mManager) != NS_OK)
        mManager = NULL;
    
    // try to get a Thread manager.
    if (mManager != NULL) {
        if (mManager->QueryInterface(NS_GET_IID(nsIThreadManager), (void**)&mThreadManager) != NS_OK)
            mThreadManager = NULL;

        if (mThreadManager != NULL)
            mThreadManager->GetCurrentThread(&mPluginThreadID);
    }

    // create a console, only if there's user interface for it.
    if (thePluginManager2 != NULL) {
        mConsole = new MRJConsole(this);
        NS_IF_ADDREF(mConsole);
    }

    return result;
}

NS_METHOD MRJPlugin::Shutdown()
{
    // shutdown LiveConnect.
    ShutdownLiveConnectSupport();

    // release our reference to the plugin manager(s).
    NS_IF_RELEASE(thePluginManager2);
    NS_IF_RELEASE(thePluginManager);

    // release our reference to the service manager.
    NS_IF_RELEASE(theServiceManager);
    NS_IF_RELEASE(theServiceManagerObsolete);
    
    return NS_OK;
}

NS_METHOD MRJPlugin::GetMIMEDescription(const char* *result)
{
	*result = NS_JVM_MIME_TYPE;
	return NS_OK;
}

NS_METHOD MRJPlugin::GetValue(nsPluginVariable variable, void *value)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_METHOD MRJPlugin::SetValue(nsPluginVariable variable, void *value)
{
	return NS_ERROR_FAILURE;
}

MRJSession* MRJPlugin::getSession()
{
	StartupJVM();
	return mSession;
}

nsIJVMManager* MRJPlugin::getManager()
{
	return mManager;
}

nsIThreadManager* MRJPlugin::getThreadManager()
{
	return mThreadManager;
}

NS_METHOD MRJPlugin::StartupJVM()
{
	if (mSession == NULL) {
		// start a session with MRJ.
		mSession = new MRJSession();
		if (mSession->getStatus() != noErr) {
			// how can we signal an error?
			delete mSession;
			mSession = NULL;
			return NS_ERROR_FAILURE;
		}
#if 0
		// Apply the initialization args.
		if (initArgs != NULL && initArgs->version >= nsJVMInitArgs_Version) {
			const char* classPathAdditions = initArgs->classpathAdditions;
			if (classPathAdditions != NULL) {
				// what format will this be in? UNIX paths, separated by ':' characters.
				char* paths = new char[1 + strlen(classPathAdditions)];
				if (paths != NULL) {
					strcpy(paths, classPathAdditions);
					char* path = strtok(paths, ":");
					while (path != NULL) {
						static char urlPrefix[] = { "file://" };
						char* fileURL = new char[sizeof(urlPrefix) + strlen(path)];
						if (fileURL != NULL) {
							strcat(strcpy(fileURL, urlPrefix), path);
							mSession->addURLToClassPath(fileURL);
							delete[] fileURL;
						}
						path = strtok(NULL, ":");
					}
					delete[] paths;
				}
			}
		}
#endif

		// Add "MRJPlugin.jar" to the class path.
		FSSpec jarFileSpec = { thePluginSpec.vRefNum, thePluginSpec.parID, "\pMRJPlugin.jar" };
		mSession->addToClassPath(jarFileSpec);

		InitLiveConnectSupport(this);

#if 0
		// start our idle thread.
		if (mThreadManager != NULL) {
			PRUint32 threadID;
			mThreadManager->CreateThread(&threadID, this);
		}
#endif

		mIsEnabled = true;
	}
	return NS_OK;
}

NS_METHOD MRJPlugin::ShutdownJVM(PRBool fullShutdown)
{
	if (fullShutdown) {
        nsresult rv = ShutdownLiveConnectSupport();
	
		if (mSession != NULL) {
			delete mSession;
			mSession = NULL;
		}
	}
	return NS_OK;
}

NS_METHOD MRJPlugin::AddToClassPath(const char* dirPath)
{
	if (mSession != NULL) {
		mSession->addToClassPath(dirPath);
		return NS_OK;
	}
	return NS_ERROR_FAILURE;
}

NS_METHOD MRJPlugin::GetClassPath(const char* *result)
{
	char* classPath = mSession->getProperty("java.class.path");
	*result = classPath;
	return (classPath != NULL ? NS_OK : NS_ERROR_FAILURE);
}

NS_METHOD MRJPlugin::GetJavaWrapper(JNIEnv* env, jint jsobj, jobject *jobj)
{
	// use jsobj as key into a table.
	// if not in the table, then create a new netscape.javascript.JSObject that references this.
	*jobj = Wrap_JSObject(env, jsobj);
	return NS_OK;
}

NS_METHOD MRJPlugin::UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj)
{
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_METHOD MRJPlugin::GetJavaVM(JavaVM* *result)
{
	*result = NULL;
	if (StartupJVM() == NS_OK) {
		*result = mSession->getJavaVM();
		return NS_OK;
	}
	return NS_ERROR_FAILURE;
}

nsrefcnt MRJPlugin::GetJNIEnv(JNIEnv* *result)
{
	JNIEnv* env = NULL;
	if (StartupJVM() == NS_OK) {
#if 1
		env = mSession->getCurrentEnv();
#else
		JDK1_1AttachArgs args;
		JavaVM* vm = mSession->getJavaVM();
		jint result = vm->AttachCurrentThread(&env, &args);
		if (result != 0)
			env = NULL;
#endif
	}
	*result = env;
	return 1;
}

nsrefcnt MRJPlugin::ReleaseJNIEnv(JNIEnv* env)
{
	return 0;
}

NS_METHOD MRJPlugin::CreateSecureEnv(JNIEnv* proxyEnv, nsISecureEnv* *outSecureEnv)
{
	*outSecureEnv = NULL;
	nsresult rv = StartupJVM();
	if (rv == NS_OK) {
		// Need to spawn a new JVM communication thread here.
		NS_DEFINE_IID(kISecureEnvIID, NS_ISECUREENV_IID);
		rv = CSecureEnv::Create(this, proxyEnv, kISecureEnvIID, (void**)outSecureEnv);
	}
	return rv;
}

NS_METHOD MRJPlugin::SpendTime(PRUint32 timeMillis)
{
	nsresult result = NS_OK;
	// Only do this if there aren't any plugin instances.
	if (MRJPluginInstance::getInstances() == NULL) {
	    if (mSession == NULL)
        	result = StartupJVM();
    	if (mSession != NULL)
    		mSession->idle(timeMillis);
	}
	return result;
}

NS_METHOD MRJPlugin::Run()
{
	while (mSession != NULL) {
		mSession->idle();
		mThreadManager->Sleep();
	}
	return NS_OK;
}

MRJPluginInstance* MRJPlugin::getPluginInstance(jobject applet)
{
	JNIEnv* env = mSession->getCurrentEnv();
	MRJPluginInstance* instance = MRJPluginInstance::getInstances();
	while (instance != NULL) {
		jobject object = NULL;
		if (instance->GetJavaObject(&object) == NS_OK && env->IsSameObject(applet, object)) {
			instance->AddRef();
			return instance;
		}
		instance = instance->getNextInstance();
	}
	return NULL;
}

MRJPluginInstance* MRJPlugin::getPluginInstance(JNIEnv* jenv)
{
	// Apple will provide an API that maps a JNIEnv to an JMAWTContextRef. We can map this to the MRJContext/Applet/Instance.
	MRJPluginInstance* instance = MRJPluginInstance::getInstances();
	if (&::JMJNIToAWTContext != NULL) {
		JMAWTContextRef contextRef = ::JMJNIToAWTContext(mSession->getSessionRef(), jenv);
		if (contextRef != NULL) {
			while (instance != NULL) {
				if (instance->getContext()->getContextRef() == contextRef) {
					instance->AddRef();
					return instance;
				}
				instance = instance->getNextInstance();
			}
		}
	} else {
		if (instance != NULL) {
			instance->AddRef();
			return instance;
		}
	}
	return NULL;
}

Boolean MRJPlugin::inPluginThread()
{
	Boolean result = false;
	nsPluginThread *currentThreadID = NULL;
	
	if (mThreadManager != NULL)
		mThreadManager->GetCurrentThread(&currentThreadID);
	if ((NULL != currentThreadID) && (NULL != mPluginThreadID)) {
		if (currentThreadID == mPluginThreadID) {
			result = true;
		}
	}
	
	return result;
}

#pragma mark *** MRJPluginInstance ***

const InterfaceInfo MRJPluginInstance::sInterfaces[] = {
	{ NS_IPLUGININSTANCE_IID, INTERFACE_OFFSET(MRJPluginInstance, nsIPluginInstance) },
	{ NS_IJVMPLUGININSTANCE_IID, INTERFACE_OFFSET(MRJPluginInstance, nsIJVMPluginInstance) },
	{ NS_IEVENTHANDLER_IID, INTERFACE_OFFSET(MRJPluginInstance, nsIEventHandler) },
};
const UInt32 MRJPluginInstance::kInterfaceCount = sizeof(sInterfaces) / sizeof(InterfaceInfo);

MRJPluginInstance::MRJPluginInstance(MRJPlugin* plugin)
	:	SupportsMixin(this, sInterfaces, kInterfaceCount),
		mPeer(NULL), mWindowlessPeer(NULL),
		mPlugin(plugin), mSession(plugin->getSession()),
		mContext(NULL), mApplet(NULL), mPluginWindow(NULL),
		mNext(NULL)
{
	// add this instance to the instance list.
	pushInstance();

	// Tell the plugin we are retaining a reference.
	mPlugin->AddRef();
}

MRJPluginInstance::~MRJPluginInstance()
{
    // Remove this instance from the global list.
    popInstance();

#if 0
    delete mContext;

    if (mPlugin != NULL) {
        mPlugin->Release();
    }

    if (mWindowlessPeer != NULL) {
        mWindowlessPeer->Release();
    }

    if (mPeer != NULL) {
        mPeer->Release();
    }

    if (mApplet != NULL) {
        JNIEnv* env = mSession->getCurrentEnv();
        env->DeleteGlobalRef(mApplet);
    }
#endif
}

static const char* kGetCodeBaseScriptURL = "javascript:var href = window.location.href; href.substring(0, href.lastIndexOf('/') + 1)";
static const char* kGetDocumentBaseScriptURL = "javascript:window.location";

static bool hasTagInfo(nsISupports* supports)
{
	nsIJVMPluginTagInfo* tagInfo;
	if (supports->QueryInterface(NS_GET_IID(nsIJVMPluginTagInfo), (void **)&tagInfo) == NS_OK) {
		NS_RELEASE(tagInfo);
		return true;
	}
	return false;
}

NS_METHOD MRJPluginInstance::Initialize(nsIPluginInstancePeer* peer)
{
    // Tell the peer we are retaining a reference.
    mPeer = peer;
    mPeer->AddRef();

    // See if we have a windowless peer.
    nsresult result = mPeer->QueryInterface(kIWindowlessPluginInstancePeerIID, (void **)&mWindowlessPeer);
    if (result != NS_OK) mWindowlessPeer = NULL;

    // create a context for the applet we will run.
    mContext = new MRJContext(mSession, this);
    if (!mContext)
        return NS_ERROR_OUT_OF_MEMORY;

    if (hasTagInfo(mPeer)) {
        mContext->processAppletTag();
        mContext->createContext();
    } else {
        // we'll be using JavaScript to create windows.
        // fire up a JavaScript URL to get the current document's location.
        nsIPluginInstance* pluginInstance = this;
        nsIPluginStreamListener* listener = this;
        result = thePluginManager->GetURL(pluginInstance, kGetDocumentBaseScriptURL, NULL, listener);
    }

    return NS_OK;
}

NS_METHOD MRJPluginInstance::OnDataAvailable(nsIPluginStreamInfo* pluginInfo, nsIInputStream* input, PRUint32 length)
{
	// hopefully all our data is available.
	char* documentBase = new char[length + 1];
	if (documentBase != NULL) {
		if (input->Read(documentBase, length, &length) == NS_OK) {
			// We've delayed processing the applet tag, because we
			// don't know the location of the current document yet.
			documentBase[length] = '\0';
			
			// set up the default document location, which can be used to compute relative CODEBASE, etc.
			mContext->setDocumentBase(documentBase);
			delete[] documentBase;
			
			mContext->processAppletTag();
			mContext->createContext();
			
			// SetWindow may be called at an inopportune time.
			if (mPluginWindow != NULL)
				mContext->setWindow(mPluginWindow);
		}
	}
	return NS_OK;
}

NS_METHOD MRJPluginInstance::GetPeer(nsIPluginInstancePeer* *result)
{
	mPeer->AddRef();
	*result = mPeer;
	return NS_OK;
}

NS_METHOD MRJPluginInstance::Start()
{
	// Take this moment to show the applet's frames (if any).
	mContext->showFrames();
	
	mContext->resumeApplet();
	
	return NS_OK;
}

NS_METHOD MRJPluginInstance::Stop()
{
	// Take this moment to hide the applet's frames.
	mContext->hideFrames();

	mContext->suspendApplet();

	return NS_OK;
}

NS_METHOD MRJPluginInstance::Destroy()
{
	// Use this opportunity to break any cycles that might exist, and reduce
	// reference counts to their minimum values.
	if (mContext != NULL) {
		delete mContext;
		mContext = NULL;
	}

	if (mPlugin != NULL) {
		mPlugin->Release();
		mPlugin = NULL;
	}

	if (mWindowlessPeer != NULL) {
		mWindowlessPeer->Release();
		mWindowlessPeer = NULL;
	}

	if (mPeer != NULL) {
		mPeer->Release();
		mPeer = NULL;
	}

	if (mApplet != NULL) {
		JNIEnv* env = mSession->getCurrentEnv();
		env->DeleteGlobalRef(mApplet);
		mApplet = NULL;
	}

	return NS_OK;
}

/** FIXME:  Need an intelligent way to track changes to the NPPluginWindow. */

NS_METHOD MRJPluginInstance::SetWindow(nsPluginWindow* pluginWindow)
{
	mPluginWindow = pluginWindow;

	mContext->setWindow(pluginWindow);

   	return NS_OK;
}

NS_METHOD MRJPluginInstance::HandleEvent(nsPluginEvent* pluginEvent, PRBool* eventHandled)
{
	*eventHandled = PR_TRUE;
	Boolean isUpdate;

	if (pluginEvent != NULL) {
		EventRecord* event = pluginEvent->event;

#if 0
		// Check for clipping changes.
		if (event->what == nsPluginEventType_ClippingChangedEvent) {
			mContext->setClipping(RgnHandle(event->message));
			return NS_OK;
		}
#else
		// Check for coordinate/clipping changes.
		isUpdate = (event->what == updateEvt);
		inspectInstance(isUpdate);
#endif
	
		if (event->what == nullEvent) {
			// Give MRJ another quantum of time.
			mSession->idle(kDefaultJMTime);	// now SpendTime does this.

#if 0			
			// check for pending update events.
			if (CheckUpdate(event)) {
				MRJFrame* frame = mContext->findFrame(WindowRef(event->message));
				if (frame != NULL)
					frame->update();
			}
#endif

		} else {
			MRJFrame* frame = mContext->findFrame(WindowRef(pluginEvent->window));
			if (frame != NULL) {
				switch (event->what) {
				case nsPluginEventType_GetFocusEvent:
					frame->focusEvent(true);
					break;
				
				case nsPluginEventType_LoseFocusEvent:
					frame->focusEvent(false);
					break;

				case nsPluginEventType_AdjustCursorEvent:
					frame->idle(event->modifiers);
					break;
				
				case nsPluginEventType_MenuCommandEvent:
					frame->menuSelected(event->message, event->modifiers);
					break;
					
				default:
					*eventHandled = frame->handleEvent(event);
					break;
				}
			}
		}
	}
	
	return NS_OK;
}

NS_METHOD MRJPluginInstance::Print(nsPluginPrint* platformPrint)
{
	if (platformPrint->mode == nsPluginMode_Embedded) {
		mContext->printApplet(&platformPrint->print.embedPrint.window);
		return NS_OK;
	}
	return NS_ERROR_NOT_IMPLEMENTED;
}

NS_METHOD MRJPluginInstance::GetValue(nsPluginInstanceVariable variable, void *value)
{
	switch (variable) {
	case nsPluginInstanceVariable_WindowlessBool:
		*(PRBool*)value = PR_FALSE;
		break;
	case nsPluginInstanceVariable_TransparentBool:
		*(PRBool*)value = PR_FALSE;
		break;
	case nsPluginInstanceVariable_DoCacheBool:
		*(PRBool*)value = PR_FALSE;
		break;
	}
	return NS_OK;
}

NS_METHOD MRJPluginInstance::GetJavaObject(jobject *result)
{
	if (mApplet == NULL) {
		jobject applet = mContext->getApplet();
		JNIEnv* env = mSession->getCurrentEnv();
		mApplet = env->NewGlobalRef(applet);
	}
	*result = mApplet;
	return NS_OK;
}

// Accessing the list of instances.

static MRJPluginInstance* theInstances = NULL;

void MRJPluginInstance::pushInstance()
{
	mNext = theInstances;
	theInstances = this;
}

void MRJPluginInstance::popInstance()
{
	MRJPluginInstance** link = &theInstances;
	MRJPluginInstance* instance  = *link;
	while (instance != NULL) {
		if (instance == this) {
			*link = mNext;
			mNext = NULL;
			break;
		}
		link = &instance->mNext;
		instance = *link;
	}
}

MRJPluginInstance* MRJPluginInstance::getInstances()
{
	return theInstances;
}

MRJPluginInstance* MRJPluginInstance::getNextInstance()
{
	return mNext;
}

MRJContext* MRJPluginInstance::getContext()
{
	return mContext;
}

MRJSession* MRJPluginInstance::getSession()
{
	return mSession;
}

void MRJPluginInstance::inspectInstance(Boolean isUpdateEvt)
{
    if (mContext != NULL && mContext->inspectWindow() && !isUpdateEvt && mWindowlessPeer != NULL)
        mWindowlessPeer->ForceRedraw();
}
