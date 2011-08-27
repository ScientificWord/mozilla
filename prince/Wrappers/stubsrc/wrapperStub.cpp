/*
 *  OS X Stubloader
 *  Original stubloader code by Benjamin Smedberg <benjamin@smedbergs.us> Copyright (c) 2005
 *  Adapted for OS X by Walter Johnson <zinkyu@gmail.com>, Sept. 2008
 */

//Include the stuff from Mozilla Glue that we need
#ifndef XP_MACOSX
#include "nscore.h"
#include "nsModule.h"
#include "prlink.h"
#endif
#include "nsILocalFile.h"
#include "nsStringAPI.h"
#include "nsCOMPtr.h"
#ifdef XP_MACOSX


//Include things from the mach-o libraries that we need for loading the libraries.
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#endif

static char const *const kDependentLibraries[] =
{
	// dependent1.dll on windows, libdependent1.so on linux, libdependent1.dylib on Mac

  MOZ_DLL_PREFIX "QtCore.4" MOZ_DLL_SUFFIX,
  nsnull
	
	// NOTE: if the dependent libs themselves depend on other libs, the subdependencies
	// should be listed first.
};

// component.dll on windows, libcomponent.so on linux, libcomponent.dylib on Mac
static char kRealComponent[] = MOZ_DLL_PREFIX "wrappers" MOZ_DLL_SUFFIX;

#ifdef XP_MACOSX
// Forward declaration of a convienience method to look up a function symbol.
static void* LookupSymbol(const mach_header* aLib, const char* aSymbolName);
#endif 

extern "C" NS_EXPORT nsresult 
NSGetModule(nsIComponentManager* aCompMgr, nsIFile* aLocation, nsIModule* *aResult)
{
	nsresult rv;
	
	// This is not the real component. We want to load the dependent libraries
	// of the real component, then the component itself, and call NSGetModule on
	// the component.
	
	// Assume that we're in <extensiondir>/components, and we want to find
	// <extensiondir>/libraries
	
	nsCOMPtr<nsIFile> libraries;
	rv = aLocation->GetParent(getter_AddRefs(libraries));
	if (NS_FAILED(rv))
		return rv;
	
	nsCOMPtr<nsILocalFile> library(do_QueryInterface(libraries));
	if (!library)
		return NS_ERROR_UNEXPECTED;
	
	nsCString path;
	// loop through and load dependent libraries
	for (char const *const *dependent = kDependentLibraries;
		 *dependent;
		 ++dependent) 
	{
		library->SetNativeLeafName(nsDependentCString(*dependent));
#ifndef XP_MACOSX
    PRLibrary *lib;
    library->Load(&lib);
    // 1) We don't care if this failed!
#else
		rv = library->GetNativePath(path);
		if (NS_FAILED(rv)) return rv;
		//At this point, we would have used PRLibrary *lib;  library->Load(&lib);  
		//But we can't use that in OS X.  Instead, we use NSAddImage.
		
		NSAddImage(path.get(), NSADDIMAGE_OPTION_RETURN_ON_ERROR |
							   NSADDIMAGE_OPTION_WITH_SEARCHING |	
							   NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME);	
		// 1) We don't care if this failed!  If you update your dependencies in your component, 
		//    but forget to remove a dependant library in the stubloader, then we don't want to 
		//    fail loading the component since the dependant library isn't required.
		//
		// 2) We are going to leak this library.  We don't care about that either.  
		//    NSAddImage adds the image to the process.  When the process terminates, the library
		//    will be properly unloaded.
#endif
	}
	
	library->SetNativeLeafName(NS_LITERAL_CSTRING(kRealComponent));

#ifndef XP_MACOSX
  PRLibrary *lib;
  rv = library->Load(&lib);
  if (NS_FAILED(rv))
    return rv;
#else
	rv = library->GetNativePath(path);
	if (NS_FAILED(rv)) return rv;
	
	const mach_header * componentlib = NSAddImage(path.get(), NSADDIMAGE_OPTION_RETURN_ON_ERROR |
		NSADDIMAGE_OPTION_WITH_SEARCHING | NSADDIMAGE_OPTION_MATCH_FILENAME_BY_INSTALLNAME);	
	if (componentlib == NULL){
	    NSLinkEditErrors linkEditError;
	    int errorNum;
	    const char *fileName;
	    const char *errorString;
	    NSLinkEditError(&linkEditError, &errorNum, &fileName, &errorString);
		errorNum++;
	}
	if (componentlib == NULL) return NS_ERROR_UNEXPECTED;
#endif
	
#ifdef XP_MACOSX
	//Find the NSGetModule procedure of the real component and “pass the buck”.
	nsGetModuleProc getmoduleproc = (nsGetModuleProc)LookupSymbol(componentlib, "_NSGetModule");
	if (!getmoduleproc) return NS_ERROR_FAILURE;
	return getmoduleproc(aCompMgr, aLocation, aResult);
#else
  nsGetModuleProc getmoduleproc = (nsGetModuleProc)
    PR_FindFunctionSymbol(lib, NS_GET_MODULE_SYMBOL);

  if (!getmoduleproc)
    return NS_ERROR_FAILURE;

  return getmoduleproc(aCompMgr, aLocation, aResult);
#endif
}

#ifdef XP_MACOSX
// Convienience method grabbed from 
// http://mxr.mozilla.org/mozilla-central/source/xpcom/glue/standalone/nsGlueLinkingOSX.cpp .
// Deprecated API calls have been removed.
static void* LookupSymbol(const mach_header* aLib, const char* aSymbolName)
{
	NSSymbol sym = nsnull;
	if (aLib) {
		sym = NSLookupSymbolInImage(aLib, aSymbolName, 
		          NSLOOKUPSYMBOLINIMAGE_OPTION_BIND | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR);
	}
	if (!sym)
		return nsnull;
	
	return NSAddressOfSymbol(sym);
}

#endif