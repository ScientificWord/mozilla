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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Patrick Beard <beard@netscape.com> (original author)
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

#include "ProxyClassLoader.h"

#include "jsapi.h"
#include "jsjava.h"
#include "prprf.h"

#include "nsIServiceManager.h"
#include "nsIScriptSecurityManager.h"
#include "nsIJSContextStack.h"
#include "nsIPrincipal.h"
#include "nsNetUtil.h"
#include "ProxyJNI.h"
#include "nsCNullSecurityContext.h"

/**
 * Obtain the netscape.oji.ProxyClassLoader instance associated with the
 * document of the currently running script. For now, store a LiveConnect
 * wrapper for this instance in window.navigator.javaclasses. There
 * hopefully aren't any security concerns with exposing this to scripts,
 * as the constructor is private, and the class loader itself can
 * only load classes from the document's URL and below.
 */
static nsresult getScriptClassLoader(JNIEnv* env, jobject* classloader)
{
    // get the current JSContext from the context stack service.
    nsresult rv;
    nsCOMPtr<nsIJSContextStack> contexts =
        do_GetService("@mozilla.org/js/xpc/ContextStack;1", &rv);
    if (NS_FAILED(rv)) return rv;
    JSContext* cx;
    rv = contexts->Peek(&cx);
    if (NS_FAILED(rv)) return rv;
    if (!cx) return NS_ERROR_NOT_AVAILABLE;
    
    // lookup "window.navigator.javaclasses", if it exists, this is the class
    // loader bound to this page.
    JSObject* window = JS_GetGlobalObject(cx);
    if (!window) return NS_ERROR_FAILURE;

    jsval navigator = JSVAL_NULL;
    if (!JS_LookupProperty(cx, window, "navigator", &navigator))
        return NS_ERROR_FAILURE;
    
    jsval javaclasses = JSVAL_NULL;
    if (!JSVAL_IS_PRIMITIVE(navigator)) {
        uintN attrs;
        JSBool found;

        // Make sure that we pull out the correct javaclasses object that we
        // set.  Since content can't spoof READONLY or PERMANANT properties,
        // their presence on this property indicates that this truely is the
        // correct object.
        JSObject *obj = JSVAL_TO_OBJECT(navigator);
        if (!JS_GetPropertyAttributes(cx, obj, "javaclasses", &attrs, &found))
            return NS_ERROR_FAILURE;
        if ((~attrs & (JSPROP_READONLY | JSPROP_PERMANENT)) == 0 &&
            !JS_GetProperty(cx, obj, "javaclasses", &javaclasses)) {
            return NS_ERROR_FAILURE;
        }

        // Unwrap this, the way LiveConnect does it. Note that this function
        // checks if javaclasses is primitive or not.
        if (JSJ_ConvertJSValueToJavaObject(cx, javaclasses, classloader))
            return NS_OK;
    }

    // use default netscape.oji.ProxyClassLoaderFactory (which is no longer
    // supported in recent JRE) as the classloader
    jclass netscape_oji_ProxyClassLoaderFac =
        env->FindClass("netscape/oji/ProxyClassLoaderFactory");
    if (!netscape_oji_ProxyClassLoaderFac) {
        env->ExceptionClear();
        return NS_ERROR_FAILURE;
    }
    jmethodID staticMethodID =
        env->GetStaticMethodID(netscape_oji_ProxyClassLoaderFac,
                               "createClassLoader",
                               "(Ljava/lang/String;)Ljava/lang/ClassLoader;");
    if (!staticMethodID) {
        env->ExceptionClear();
        return NS_ERROR_FAILURE;
    }

    // Obtain the URL of the document of the currently running script. This will
    // be used as the default location to download classes from.
    nsCOMPtr<nsIScriptSecurityManager> secMan =
             do_GetService(NS_SCRIPTSECURITYMANAGER_CONTRACTID, &rv);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIPrincipal> principal, sysprincipal;
    rv = secMan->GetPrincipalFromContext(cx, getter_AddRefs(principal));
    if (NS_FAILED(rv)) return rv;

    rv = secMan->GetSystemPrincipal(getter_AddRefs(sysprincipal));
    if (NS_FAILED(rv)) return rv;

    PRBool equals;
    rv = principal->Equals(sysprincipal, &equals);
    // Can't get URI from system principal
    if (NS_FAILED(rv)) return rv;
    if (equals) return NS_ERROR_FAILURE;

    nsCOMPtr<nsIURI> codebase;
    rv = principal->GetURI(getter_AddRefs(codebase));
    if (NS_FAILED(rv)) return rv;

    // create a netscape.oji.ProxyClassLoader instance.
    nsCAutoString spec;
    rv = codebase->GetSpec(spec);
    if (NS_FAILED(rv)) return rv;
    
    jstring jspec = env->NewStringUTF(spec.get());
    if (!jspec) {
        env->ExceptionClear();
        return NS_ERROR_FAILURE;
    }

    // In order to have permission to create classloader, we need to grant
    // enough permission
    nsISecurityContext* origContext = nsnull;
    if (NS_FAILED(GetSecurityContext(env, &origContext))) {
        return NS_ERROR_FAILURE;
    }
    nsCOMPtr<nsISecurityContext> nullContext(new nsCNullSecurityContext());
    if (!nullContext) {
        return NS_ERROR_OUT_OF_MEMORY;
    }
    
    SetSecurityContext(env, nullContext);
    *classloader = env->CallStaticObjectMethod(netscape_oji_ProxyClassLoaderFac,
                                               staticMethodID, jspec);
    SetSecurityContext(env, origContext);
    if (!*classloader) {
        env->ExceptionClear();
        return NS_ERROR_FAILURE;
    }

    env->DeleteLocalRef(jspec);
    env->DeleteLocalRef(netscape_oji_ProxyClassLoaderFac);
    
    // now, cache the class loader in "window.navigator.javaclasses"
    if (!JSVAL_IS_PRIMITIVE(navigator) &&
        JSJ_ConvertJavaObjectToJSValue(cx, *classloader, &javaclasses) &&
        !JS_DefineProperty(cx, JSVAL_TO_OBJECT(navigator), "javaclasses",
                           javaclasses, NULL, NULL, JSPROP_ENUMERATE |
                           JSPROP_READONLY | JSPROP_PERMANENT)) {
        return NS_ERROR_FAILURE;
    }
    
    return NS_OK;
}

jclass ProxyFindClass(JNIEnv* env, const char* name)
{
    do {
        // TODO:  prevent recursive call to ProxyFindClass, if netscape.oji.ProxyClassLoader
        // isn't found by getScriptClassLoader().
        jobject classloader;
        jthrowable jException = env->ExceptionOccurred();
        if (jException != NULL) {
            // Clean up exception
            env->ExceptionClear();
            // Release local ref
            env->DeleteLocalRef(jException);
        }
        nsresult rv = getScriptClassLoader(env, &classloader);
        if (NS_FAILED(rv)) break;

        jclass netscape_oji_ProxyClassLoader = env->GetObjectClass(classloader);
        jmethodID loadClassID = env->GetMethodID(netscape_oji_ProxyClassLoader, "loadClass",
                                                 "(Ljava/lang/String;)Ljava/lang/Class;");
        env->DeleteLocalRef(netscape_oji_ProxyClassLoader);
        if (!loadClassID) {
            env->ExceptionClear();
            break;
        }
        jstring jname = env->NewStringUTF(name);
        jvalue jargs[1]; jargs[0].l = jname;
        jclass c = (jclass) env->CallObjectMethodA(classloader, loadClassID, jargs);
        env->DeleteLocalRef(jname);
        return c;
    } while (0);
    return 0;
}
