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
 *   Jens Bannmann <jens.b@web.de>
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

#include "jsapi.h"
#include "nsJSFile.h"
#include "nscore.h"
#include "nsIScriptContext.h"

#include "nsBuildID.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsInstall.h"
#include "nsInstallFile.h"
#include "nsInstallTrigger.h"
#include "nsIPromptService.h"

#include "nsIDOMInstallVersion.h"

#include <stdio.h>

#ifdef _WINDOWS
#include "nsJSWinReg.h"
#include "nsJSWinProfile.h"

extern JSClass WinRegClass;
extern JSClass WinProfileClass;
#endif

#include "nsJSFileSpecObj.h"
extern JSClass FileSpecObjectClass;

extern JSClass FileOpClass;

PR_STATIC_CALLBACK(JSBool)
GetInstallProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

PR_STATIC_CALLBACK(JSBool)
SetInstallProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

PR_STATIC_CALLBACK(void)
FinalizeInstall(JSContext *cx, JSObject *obj);

/***********************************************************************/
//
// class for Install
//
JSClass InstallClass = {
  "Install",
  JSCLASS_HAS_PRIVATE,
  JS_PropertyStub,
  JS_PropertyStub,
  GetInstallProperty,
  SetInstallProperty,
  JS_EnumerateStub,
  JS_ResolveStub,
  JS_ConvertStub,
  FinalizeInstall
};

//
// Install property ids
//
enum Install_slots
{
  INSTALL_PLATFORM        = -1,
  INSTALL_JARFILE         = -2,
  INSTALL_ARCHIVE         = -3,
  INSTALL_ARGUMENTS       = -4,
  INSTALL_URL             = -5,
  INSTALL_FLAGS           = -6,
  INSTALL_FINALSTATUS     = -7,
  INSTALL_INSTALL         = -8,
  INSTALL_INSTALLED_FILES = -9
};

// prototype for fileOp object
JSObject *gFileOpProto = nsnull;

// prototype for filespec objects
JSObject *gFileSpecProto = nsnull;


JSObject *gFileOpObject = nsnull;
/***********************************************************************/
//
// Install Properties Getter
//
PR_STATIC_CALLBACK(JSBool)
GetInstallProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  nsInstall *a = (nsInstall*)JS_GetPrivate(cx, obj);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == a) {
    return JS_TRUE;
  }

  if (JSVAL_IS_INT(id))
  {
    switch(JSVAL_TO_INT(id)) {
      case INSTALL_PLATFORM:
      {
        nsCAutoString prop;
        a->GetInstallPlatform(prop);
        *vp = STRING_TO_JSVAL( JS_NewStringCopyZ(cx, prop.get()) );
        break;
      }

      case INSTALL_ARCHIVE:
      case INSTALL_JARFILE:
      {
        nsInstallFolder* folder = new nsInstallFolder();
        if ( folder )
        {
          folder->Init(a->GetJarFileLocation(),EmptyString());
          JSObject* fileSpecObject =
              JS_NewObject(cx, &FileSpecObjectClass, gFileSpecProto, NULL);

          if (fileSpecObject)
          {
            JS_SetPrivate(cx, fileSpecObject, folder);
            *vp = OBJECT_TO_JSVAL(fileSpecObject);
          }
          else
              delete folder;
        }
        break;
      }

      case INSTALL_ARGUMENTS:
      {
        nsAutoString prop;

        a->GetInstallArguments(prop);
        *vp = STRING_TO_JSVAL( JS_NewUCStringCopyN(cx, NS_REINTERPRET_CAST(const jschar*, prop.get()), prop.Length()) );

        break;
      }

      case INSTALL_URL:
      {
        nsString prop;

        a->GetInstallURL(prop);
        *vp = STRING_TO_JSVAL( JS_NewUCStringCopyN(cx, NS_REINTERPRET_CAST(const jschar*, prop.get()), prop.Length()) );

        break;
      }

      case INSTALL_FLAGS:
          *vp = INT_TO_JSVAL( a->GetInstallFlags() );
          break;

      case INSTALL_FINALSTATUS:
          *vp = INT_TO_JSVAL( a->GetFinalStatus() );
          break;

      case INSTALL_INSTALL:
          *vp = OBJECT_TO_JSVAL(obj);
          break;

      case INSTALL_INSTALLED_FILES:
          *vp = BOOLEAN_TO_JSVAL( a->InInstallTransaction() );
          break;

      default:
        return JS_TRUE;
    }
  }
  else {
    return JS_TRUE;
  }

  return PR_TRUE;
}

/***********************************************************************/
//
// Install Properties Setter
//
PR_STATIC_CALLBACK(JSBool)
SetInstallProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
  nsInstall *a = (nsInstall*)JS_GetPrivate(cx, obj);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == a) {
    return JS_TRUE;
  }

  if (JSVAL_IS_INT(id)) {
    switch(JSVAL_TO_INT(id)) {
      case 0:
      default:
          return JS_TRUE;
    }
  }
  else {
    return JS_TRUE;
  }

  return PR_TRUE;
}


static void PR_CALLBACK FinalizeInstall(JSContext *cx, JSObject *obj)
{
    nsInstall *nativeThis = (nsInstall*)JS_GetPrivate(cx, obj);
    delete nativeThis;
}

/***********************************************************************/
/*                        JS Utilities                                 */
/***********************************************************************/
void ConvertJSValToStr(nsString&  aString,
                      JSContext* aContext,
                      jsval      aValue)
{
  JSString *jsstring;

  if ( !JSVAL_IS_NULL(aValue) &&
       (jsstring = JS_ValueToString(aContext, aValue)) != nsnull)
  {
    aString.Assign(NS_REINTERPRET_CAST(const PRUnichar*, JS_GetStringChars(jsstring)));
  }
  else
  {
    aString.Truncate();
  }
}

void ConvertStrToJSVal(const nsString& aProp,
                      JSContext* aContext,
                      jsval* aReturn)
{
  JSString *jsstring = JS_NewUCStringCopyN(aContext, NS_REINTERPRET_CAST(const jschar*, aProp.get()), aProp.Length());
  // set the return value
  *aReturn = STRING_TO_JSVAL(jsstring);
}

PRBool ConvertJSValToBool(PRBool* aProp,
                         JSContext* aContext,
                         jsval aValue)
{
  JSBool temp;
  if(JSVAL_IS_BOOLEAN(aValue) && JS_ValueToBoolean(aContext, aValue, &temp))
  {
    *aProp = (PRBool)temp;
  }
  else
  {
    JS_ReportError(aContext, "Parameter must be a boolean");
    return JS_FALSE;
  }

  return JS_TRUE;
}

PRBool ConvertJSValToObj(nsISupports** aSupports,
                        REFNSIID aIID,
                        JSClass* aClass,
                        JSContext* aContext,
                        jsval aValue)
{
  if (JSVAL_IS_NULL(aValue)) {
    *aSupports = nsnull;
    return JS_TRUE;
  }
  if (!JSVAL_IS_OBJECT(aValue)) {
    JS_ReportError(aContext, "Parameter must be an object");
    return JS_FALSE;
  }
  JSObject* jsobj = JSVAL_TO_OBJECT(aValue);
  JSClass* jsclass = JS_GetClass(aContext, jsobj);
  if (!jsclass ||
      !(jsclass->flags & JSCLASS_HAS_PRIVATE) ||
      !(jsclass->flags & JSCLASS_PRIVATE_IS_NSISUPPORTS)) {
    JS_ReportError(aContext,
                   "Parameter isn't an object that implements nsISupports");
    return JS_FALSE;
  }
  if (!JS_InstanceOf(aContext,
                     jsobj,
                     aClass,
                     nsnull))
  {
    char buf[128];
    sprintf(buf, "Parameter isn't an instance of type %s", aClass->name);
    JS_ReportError(aContext, buf);
    return JS_FALSE;
  }
  nsISupports *supports = (nsISupports *)JS_GetPrivate(aContext, jsobj);
  if (!supports) {
    NS_ERROR("This should not happen.");
    JS_ReportError(aContext,
                   "JSObject has JSCLASS_PRIVATE_IS_NSISUPPORTS flag but has a null private slot, please file a bug.");
    return JS_FALSE;
  }
  if (NS_FAILED(supports->QueryInterface(aIID, (void **)aSupports))) {
    char buf[128];
    sprintf(buf, "Parameter must be of type %s", aClass->name);
    JS_ReportError(aContext, buf);
    return JS_FALSE;
  }

  return JS_TRUE;
}


void ConvertJSvalToVersionString(nsString& versionString, JSContext* cx, jsval argument)
{
    versionString.SetLength(0);

    if( JSVAL_IS_OBJECT(argument) )
    {
        if(!JSVAL_IS_NULL(argument))
        {
            JSObject* jsobj   = JSVAL_TO_OBJECT(argument);
            JSClass*  jsclass = JS_GetClass(cx, jsobj);

            if ((nsnull != jsclass) && (jsclass->flags & JSCLASS_HAS_PRIVATE))
            {
                nsIDOMInstallVersion* version = (nsIDOMInstallVersion*)JS_GetPrivate(cx, jsobj);
                version->ToString(versionString);
            }
        }
    }
    else
    {
        ConvertJSValToStr(versionString, cx, argument);
    }
}
/***********************************************************************/
/***********************************************************************/

//
// Native method AbortInstall
//
PR_STATIC_CALLBACK(JSBool)
InstallAbortInstall(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  int32   b0;

  *rval = JSVAL_VOID;

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 1)
  {
    //  public int AbortInstall(Number aErrorNumber);
    if(JS_ValueToInt32(cx, argv[0], &b0))
    {
      nativeThis->AbortInstall(b0);
    }
    else
    {
      JS_ReportError(cx, "Parameter must be a number");
    }
  }
  else if(argc == 0)
  {
    //  public int AbortInstall(void);
    nativeThis->AbortInstall(nsInstall::INSTALL_CANCELLED);
  }

  return JS_TRUE;
}


//
// Native method AddDirectory
//
PR_STATIC_CALLBACK(JSBool)
InstallAddDirectory(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;
  nsAutoString b0;
  nsAutoString b1;
  nsAutoString b2;
  nsAutoString b3;
  nsAutoString b4;
  JSObject *jsObj;
  nsInstallFolder *folder;
  PRInt32 flags;

  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc == 1)
  {
    // public int AddDirectory (String jarSourcePath)

    ConvertJSValToStr(b0, cx, argv[0]);

    if(NS_OK == nativeThis->AddDirectory(b0, &nativeRet))
    {
      *rval = INT_TO_JSVAL(nativeRet);
    }
  }
  else if (argc == 4)
  {
    //  public int AddDirectory ( String registryName,
    //                            String jarSourcePath,
    //                            Object localDirSpec,
    //                            String relativeLocalPath);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSValToStr(b1, cx, argv[1]);
    ConvertJSValToStr(b3, cx, argv[3]);
    if (argv[2] == JSVAL_NULL || !JSVAL_IS_OBJECT(argv[2])) //argv[2] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[2]);
    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);

    if(NS_OK == nativeThis->AddDirectory(b0, b1, folder, b3, &nativeRet))
    {
      *rval = INT_TO_JSVAL(nativeRet);
    }
  }
  else if (argc == 5)
  {
    //  public int AddDirectory ( String registryName,
    //                            String version,  --OR-- VersionInfo version
    //                            String jarSourcePath,
    //                            Object localDirSpec,
    //                            String relativeLocalPath);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSvalToVersionString(b1, cx, argv[1]);
    ConvertJSValToStr(b2, cx, argv[2]);
    ConvertJSValToStr(b4, cx, argv[4]);

    if ((argv[3] == JSVAL_NULL) || !JSVAL_IS_OBJECT(argv[3])) //argv[3] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[3]);
    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);

    if(NS_OK == nativeThis->AddDirectory(b0, b1, b2, folder, b4, &nativeRet))
    {
      *rval = INT_TO_JSVAL(nativeRet);
    }
  }
  else if (argc >= 6)
  {
     //   public int AddDirectory (  String registryName,
     //                              String version,        --OR--     VersionInfo version,
     //                              String jarSourcePath,
     //                              Object localDirSpec,
     //                              String relativeLocalPath,
     //                              Int    flags);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSvalToVersionString(b1, cx, argv[1]);
    ConvertJSValToStr(b2, cx, argv[2]);
    ConvertJSValToStr(b4, cx, argv[4]);

    if ((argv[3] == JSVAL_NULL) || !JSVAL_IS_OBJECT(argv[3])) //argv[3] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[3]);
    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    if(JSVAL_IS_INT(argv[5]))
      flags = JSVAL_TO_INT(argv[5]);
    else
      flags = 0;

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);

    if(NS_OK == nativeThis->AddDirectory(b0, b1, b2, folder, b4, flags, &nativeRet))
    {
      *rval = INT_TO_JSVAL(nativeRet);
    }
  }
  else
  {
    JS_ReportError(cx, "Install.AddDirectory() parameters error");
  }

  return JS_TRUE;
}


//
// Native method AddSubcomponent
//
PR_STATIC_CALLBACK(JSBool)
InstallAddSubcomponent(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;
  nsAutoString b0;
  nsAutoString b1;
  nsAutoString b2;
  nsAutoString b3;
  nsAutoString b4;
  JSObject* jsObj;
  nsInstallFolder* folder;
  PRInt32 flags = 0;
  nsresult rv;

  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 6)
  {
    //  public int AddSubcomponent ( String registryName,
    //                               String version,        --OR--     VersionInfo version,
    //                               String jarSourcePath,
    //                               Object localDirSpec,
    //                               String relativeLocalPath,
    //                               Int    flags);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSvalToVersionString(b1, cx, argv[1]);
    ConvertJSValToStr(b2, cx, argv[2]);
    ConvertJSValToStr(b4, cx, argv[4]);

    if ((argv[3] == JSVAL_NULL) || !JSVAL_IS_OBJECT(argv[3])) //argv[3] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[3]);
    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    if(JSVAL_IS_INT(argv[5]))
      flags = JSVAL_TO_INT(argv[5]);
    else
      flags = 0;

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);

    jsrefcount saveDepth;
    saveDepth = JS_SuspendRequest(cx);//Need to suspend use of thread or deadlock occurs
    rv= nativeThis->AddSubcomponent(b0, b1, b2, folder, b4, flags, &nativeRet);
    JS_ResumeRequest(cx, saveDepth);

    if (NS_SUCCEEDED(rv))
      *rval = INT_TO_JSVAL(nativeRet);
  }
  else if(argc >= 5)
  {
    //  public int AddSubcomponent ( String registryName,
    //                               String version,  --OR-- VersionInfo version
    //                               String jarSourcePath,
    //                               Object localDirSpec,
    //                               String relativeLocalPath);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSvalToVersionString(b1, cx, argv[1]);
    ConvertJSValToStr(b2, cx, argv[2]);
    ConvertJSValToStr(b4, cx, argv[4]);

    if ((argv[3] == JSVAL_NULL) || !JSVAL_IS_OBJECT(argv[3])) //argv[3] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[3]);

    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);

    jsrefcount saveDepth;
    saveDepth = JS_SuspendRequest(cx);//Need to suspend use of thread or deadlock occurs
    rv = nativeThis->AddSubcomponent(b0, b1, b2, folder, b4, &nativeRet);
    JS_ResumeRequest(cx, saveDepth);

    if (NS_SUCCEEDED(rv))
      *rval = INT_TO_JSVAL(nativeRet);
  }
  else if(argc >= 4)
  {
    //  public int AddSubcomponent ( String registryName,
    //                               String jarSourcePath,
    //                               Object localDirSpec,
    //                               String relativeLocalPath);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSValToStr(b1, cx, argv[1]);
    ConvertJSValToStr(b3, cx, argv[3]);

    if ((argv[2] == JSVAL_NULL) || !JSVAL_IS_OBJECT(argv[2])) //argv[2] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[2]);
    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);

    if(NS_OK == nativeThis->AddSubcomponent(b0, b1, folder, b3, &nativeRet))
    {
      *rval = INT_TO_JSVAL(nativeRet);
    }
  }
  else if(argc >= 1)
  {
    //  public int AddSubcomponent ( String jarSourcePath);

    ConvertJSValToStr(b0, cx, argv[0]);

    if(NS_OK == nativeThis->AddSubcomponent(b0, &nativeRet))
    {
      *rval = INT_TO_JSVAL(nativeRet);
    }
  }
  else
  {
    JS_ReportError(cx, "Install.addFile parameter error");
  }

  return JS_TRUE;
}


//
// Native method DeleteComponent
//
PR_STATIC_CALLBACK(JSBool)
InstallDeleteComponent(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  // this function was once documented but never supported. Return an error,
  // but don't remove which would kill scripts that reference this.
  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);
  return JS_TRUE;
}

//
// Native method Execute
//
PR_STATIC_CALLBACK(JSBool)
InstallExecute(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;
  nsAutoString b0;
  nsAutoString b1;
  PRBool       blocking = PR_FALSE;

  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 3)
  {
    //  public int Execute ( String jarSourcePath,
    //                       String args,
    //                       Bool   blocking);

    ConvertJSValToStr(b1, cx, argv[1]);
    ConvertJSValToBool(&blocking, cx,argv[2]);
  }
  else if(argc >= 2)
  {
    if(JSVAL_IS_BOOLEAN(argv[1]))
    {
      //  public int Execute ( String jarSourcePath,
      //                       Bool   blocking);
      ConvertJSValToBool(&blocking, cx, argv[1]);
    }
    else
    {
      //  public int Execute ( String jarSourcePath,
      //                       String args);
      ConvertJSValToStr(b1, cx, argv[1]);
    }
  }

  if(argc >= 1)
  {
    //  public int Execute ( String jarSourcePath);
    ConvertJSValToStr(b0, cx, argv[0]);

    jsrefcount saveDepth;
    saveDepth = JS_SuspendRequest(cx);//Need to suspend use of thread or deadlock occurs
    nativeThis->Execute(b0, b1, blocking, &nativeRet);
    JS_ResumeRequest(cx, saveDepth);

    *rval = INT_TO_JSVAL(nativeRet);
  }
  else
  {
    JS_ReportWarning(cx, "Function Execute requires 1 parameter");
  }

  return JS_TRUE;
}


//
// Native method FinalizeInstall
//
PR_STATIC_CALLBACK(JSBool)
InstallFinalizeInstall(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;

  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  jsrefcount saveDepth;
  saveDepth = JS_SuspendRequest(cx);//Need to suspend use of thread or deadlock occurs
  //  public int FinalizeInstall (void);
  nsresult rv = nativeThis->FinalizeInstall(&nativeRet);
  JS_ResumeRequest(cx, saveDepth);

  if (NS_SUCCEEDED(rv))
    *rval = INT_TO_JSVAL(nativeRet);

  return JS_TRUE;
}


//
// Native method Gestalt
//
PR_STATIC_CALLBACK(JSBool)
InstallGestalt(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;
  nsAutoString b0;

  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 1)
  {
    //  public int Gestalt ( String selector,
    //                       long   *response);

    ConvertJSValToStr(b0, cx, argv[0]);

    if(NS_OK != nativeThis->Gestalt(b0, &nativeRet))
    {
      return JS_FALSE;
    }

    *rval = INT_TO_JSVAL(nativeRet);
  }
  else
  {
    JS_ReportError(cx, "Function Gestalt requires 1 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method GetComponentFolder
//
PR_STATIC_CALLBACK(JSBool)
InstallGetComponentFolder(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  nsInstallFolder* folder;
  nsAutoString b0;
  nsAutoString b1;

  *rval = JSVAL_NULL;

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 2)
  {
    //  public int GetComponentFolder ( String registryName,
    //                                  String subDirectory);
    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSValToStr(b1, cx, argv[1]);

    if(NS_OK != nativeThis->GetComponentFolder(b0, b1, &folder))
    {
      // error!
      return JS_FALSE;
    }
  }
  else if(argc >= 1)
  {
    //  public int GetComponentFolder ( String registryName);

    ConvertJSValToStr(b0, cx, argv[0]);

    if(NS_OK != nativeThis->GetComponentFolder(b0, &folder))
    {
      // error!
      return JS_FALSE;
    }
  }
  else
  {
    JS_ReportError(cx, "Function GetComponentFolder requires 2 parameters");
    return JS_FALSE;
  }

  // if we couldn't find one return null rval from here
  if(nsnull == folder)
  {
    return JS_TRUE;
  }

  /* Now create the new JSObject */
  JSObject* fileSpecObject;

  fileSpecObject = JS_NewObject(cx, &FileSpecObjectClass, gFileSpecProto, NULL);
  if (fileSpecObject == NULL)
    return JS_FALSE;

  JS_SetPrivate(cx, fileSpecObject, folder);
  if (fileSpecObject == NULL)
    return JS_FALSE;

  *rval = OBJECT_TO_JSVAL(fileSpecObject);

  return JS_TRUE;
}


//
// Native method GetFolder
//
PR_STATIC_CALLBACK(JSBool)
InstallGetFolder(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  JSObject *jsObj;
  nsAutoString b0;
  nsAutoString b1;
  nsInstallFolder *folder = nsnull;

  *rval = JSVAL_NULL;

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 2)
  {
    ConvertJSValToStr(b1, cx, argv[1]); // we know that the second param must be a string
    if(JSVAL_IS_STRING(argv[0])) // check if the first argument is a string
    {
      ConvertJSValToStr(b0, cx, argv[0]);
      if(NS_OK != nativeThis->GetFolder(b0, b1, &folder))
        return JS_TRUE;
    }
    else /* it must be an object */
    {

      if ((argv[0] == JSVAL_NULL) || !JSVAL_IS_OBJECT(argv[0])) //argv[0] MUST be a jsval
      {
        *rval = JSVAL_NULL;
        JS_ReportError(cx, "GetFolder:Invalid Parameter");
        return JS_TRUE;
      }

      jsObj = JSVAL_TO_OBJECT(argv[0]);
      if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
      {
        *rval = JSVAL_NULL;
        JS_ReportError(cx, "GetFolder:Invalid Parameter");
        return JS_TRUE;
      }

      folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);
      if (!folder)
      {
        JS_ReportError(cx, "GetFolder:Invalid Parameter");
        return JS_FALSE;
      }
      else if(NS_OK != nativeThis->GetFolder(*folder, b1, &folder))
        return JS_TRUE;
    }

    if(nsnull == folder)
      return JS_TRUE;
  }
  else if(argc >= 1)
  {
    //  public int GetFolder ( String folderName);

    ConvertJSValToStr(b0, cx, argv[0]);

    if(NS_OK != nativeThis->GetFolder(b0, &folder))
    {
      return JS_TRUE;
    }

    if(nsnull == folder)
      return JS_TRUE;
  }
  else
  {
    JS_ReportError(cx, "Function GetFolder requires at least 1 parameter");
    return JS_FALSE;
  }

  if ( folder )
  {
    /* Now create the new JSObject */
    JSObject* fileSpecObject;

    fileSpecObject = JS_NewObject(cx, &FileSpecObjectClass, gFileSpecProto, NULL);
    if (fileSpecObject == NULL)
      return JS_FALSE;

    JS_SetPrivate(cx, fileSpecObject, folder);
    if (fileSpecObject == NULL)
      return JS_FALSE;

    *rval = OBJECT_TO_JSVAL(fileSpecObject);
  }


  return JS_TRUE;
}



//
// Native method GetLastError
//
PR_STATIC_CALLBACK(JSBool)
InstallGetLastError(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;


  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  //  public int GetLastError (void);

  if(NS_OK == nativeThis->GetLastError(&nativeRet)) {
    *rval = INT_TO_JSVAL(nativeRet);
  }

  return JS_TRUE;
}


//
// Native method GetWinProfile
//
PR_STATIC_CALLBACK(JSBool)
InstallGetWinProfile(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  *rval = JSVAL_NULL;

#ifdef _WINDOWS
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);
  if (!nativeThis)
    return JS_FALSE;

  nsAutoString b0;
  nsAutoString b1;

  if(argc >= 2)
  {
    //  public int GetWinProfile (Object folder,
    //                            String file);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSValToStr(b1, cx, argv[1]);

    if(NS_OK != nativeThis->GetWinProfile(b0, b1, cx, &WinProfileClass, rval))
    {
      return JS_FALSE;
    }
  }
  else
  {
    JS_ReportError(cx, "Function GetWinProfile requires 0 parameters");
    return JS_FALSE;
  }
#endif

  return JS_TRUE;
}


//
// Native method GetWinRegistry
//
PR_STATIC_CALLBACK(JSBool)
InstallGetWinRegistry(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  *rval = JSVAL_NULL;

#ifdef _WINDOWS

  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);
  if (!nativeThis)
    return JS_FALSE;

  //  public int GetWinRegistry (void);
  if(NS_OK != nativeThis->GetWinRegistry(cx, &WinRegClass, rval))
  {
    *rval = JSVAL_NULL;
    return JS_TRUE;
  }
#endif

  return JS_TRUE;
}


//
// Native method LoadResources
//
PR_STATIC_CALLBACK(JSBool)
InstallLoadResources(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  nsAutoString b0;

  *rval = JSVAL_NULL;

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if (argc >= 1)
  {
    ConvertJSValToStr(b0, cx, argv[0]);
    if (NS_OK != nativeThis->LoadResources(cx, b0, rval))
    {
      return JS_FALSE;
    }
  }
  else
  {
    JS_ReportError(cx, "Function LoadResources requires 1 parameter");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method Patch
//
PR_STATIC_CALLBACK(JSBool)
InstallPatch(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;
  nsAutoString b0;
  nsAutoString b1;
  nsAutoString b2;
  nsAutoString b3;
  nsAutoString b4;

  JSObject *jsObj;

  nsInstallFolder *folder = nsnull;


  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 5)
  {
    //  public int Patch (String registryName,
    //                    String version,        --OR-- VersionInfo version,
    //                    String jarSourcePath,
    //                    Object localDirSpec,
    //                    String relativeLocalPath);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSvalToVersionString(b1, cx, argv[1]);
    ConvertJSValToStr(b2, cx, argv[2]);
    ConvertJSValToStr(b3, cx, argv[4]);

    if ((argv[3] == JSVAL_NULL) || !JSVAL_IS_OBJECT(argv[3])) //argv[3] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[3]);
    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);

    if(NS_OK != nativeThis->Patch(b0, b1, b2, folder, b3, &nativeRet))
    {
        return JS_FALSE;
    }

    *rval = INT_TO_JSVAL(nativeRet);
  }
  else if(argc >= 4)
  {
    //  public int Patch (String registryName,
    //                    String jarSourcePath,
    //                    Object localDirSpec,
    //                    String relativeLocalPath);

    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSValToStr(b1, cx, argv[1]);
    ConvertJSValToStr(b2, cx, argv[3]);

    if ((argv[2] == JSVAL_NULL) || !JSVAL_IS_OBJECT(argv[2])) //argv[2] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[2]);
    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);

    if(NS_OK != nativeThis->Patch(b0, b1, folder, b2, &nativeRet))
    {
      return JS_FALSE;
    }

    *rval = INT_TO_JSVAL(nativeRet);
  }
  else
  {
    JS_ReportError(cx, "Function Patch requires 5 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}

//
// Native method RegisterChrome
//
//    int registerChrome(
//         int  type,
//         FileSpecObject chrome,
//         String extraPath)
PR_STATIC_CALLBACK(JSBool)
InstallRegisterChrome(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  // If there's no private data, this must be the prototype, so ignore
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  if (nsnull == nativeThis) {
    *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);
    return JS_TRUE;
  }

  // Now validate the arguments
  uint32 chromeType = 0;
  nsIFile* chrome = nsnull;
  if ( argc >=2 )
  {
    JS_ValueToECMAUint32(cx, argv[0], &chromeType);

    if (argv[1] != JSVAL_NULL && JSVAL_IS_OBJECT(argv[1]))
    {
      JSObject* jsObj = JSVAL_TO_OBJECT(argv[1]);
      if (JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
      {
        nsInstallFolder* folder = (nsInstallFolder*)JS_GetPrivate(cx,jsObj);
        if (folder)
        {
          chrome = folder->GetFileSpec();
        }
      }
    }
  }
  nsAutoString path;
  if (argc >= 3) {
    ConvertJSValToStr(path, cx, argv[2]);
  }

  *rval = INT_TO_JSVAL(nativeThis->RegisterChrome(chrome, chromeType, NS_ConvertUTF16toUTF8(path).get()));

  return JS_TRUE;
}


//
// Native method RefreshPlugins
//
PR_STATIC_CALLBACK(JSBool)
InstallRefreshPlugins(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  // If there's no private data, this must be the prototype, so ignore
  if (!nativeThis)
  {
    *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);
  }
  else
  {
    PRBool reloadPages = PR_TRUE;
    if (argc >= 1)
      reloadPages = JSVAL_TO_BOOLEAN(argv[0]);

    *rval = INT_TO_JSVAL(nativeThis->RefreshPlugins(reloadPages));
  }
  return JS_TRUE;
}


//
// Native method ResetError
//
PR_STATIC_CALLBACK(JSBool)
InstallResetError(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  *rval = JSVAL_VOID;

  // If there's no private data, this must be the prototype, so ignore
  if (!nativeThis)
    return JS_TRUE;

  // Supported forms:
  //    void resetError()
  //    void resetError(int error)
  int32 val = 0;
  if ( argc >= 1 )
    JS_ValueToECMAInt32(cx, argv[0], &val);

  nativeThis->ResetError(val);
  return JS_TRUE;
}


//
// Native method SetPackageFolder
//
PR_STATIC_CALLBACK(JSBool)
InstallSetPackageFolder(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  nsAutoString b0;
  JSObject *jsObj;
  nsInstallFolder *folder;

  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 1)
  {
    //  public int SetPackageFolder (Object folder);

    if ((argv[0] == JSVAL_NULL)  || !JSVAL_IS_OBJECT(argv[0])) //argv[0] MUST be a jsval
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    jsObj = JSVAL_TO_OBJECT(argv[0]);
    if (!JS_InstanceOf(cx, jsObj, &FileSpecObjectClass, nsnull))
    {
      *rval = INT_TO_JSVAL(nsInstall::INVALID_ARGUMENTS);
      nativeThis->SaveError(nsInstall::INVALID_ARGUMENTS);
      return JS_TRUE;
    }

    folder = (nsInstallFolder*)JS_GetPrivate(cx, jsObj);
    if (!folder)
    {
      JS_ReportError(cx, "setPackageFolder:Invalid Parameter");
      return JS_FALSE;
    }
    else
      if(NS_OK != nativeThis->SetPackageFolder(*folder))
        return JS_FALSE;

    *rval = JSVAL_ZERO;
  }
  else
  {
    JS_ReportError(cx, "Function SetPackageFolder requires 1 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}

//
// Native method StartInstall
//
PR_STATIC_CALLBACK(JSBool)
InstallStartInstall(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;
  nsAutoString b0;
  nsAutoString b1;
  nsAutoString b2;

  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc == 3 || argc == 4)
  {
    //  public int StartInstall (String userPackageName,
    //                           String package,
    //                           String version); --OR-- VersionInfo version
    //                           flags?
    ConvertJSValToStr(b0, cx, argv[0]);
    ConvertJSValToStr(b1, cx, argv[1]);
    ConvertJSvalToVersionString(b2, cx, argv[2]);

    jsrefcount saveDepth;
    saveDepth = JS_SuspendRequest(cx);//Need to suspend use of thread or deadlock occurs

    nsresult rv = nativeThis->StartInstall(b0, b1, b2, &nativeRet);

    JS_ResumeRequest(cx, saveDepth); //Resume the suspened thread
    if (NS_FAILED(rv))
    {
        return JS_FALSE;
    }

    *rval = INT_TO_JSVAL(nativeRet);
  }
  else
  {
    JS_ReportError(cx, "Function StartInstall requires 3 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method Uninstall
//
PR_STATIC_CALLBACK(JSBool)
InstallUninstall(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);

  PRInt32 nativeRet;
  nsAutoString b0;

  *rval = INT_TO_JSVAL(nsInstall::UNEXPECTED_ERROR);

  // If there's no private data, this must be the prototype, so ignore
  if (nsnull == nativeThis) {
    return JS_TRUE;
  }

  if(argc >= 1)
  {
    //  public int Uninstall (String packageName);

    ConvertJSValToStr(b0, cx, argv[0]);

    if(NS_OK != nativeThis->Uninstall(b0, &nativeRet))
    {
      return JS_FALSE;
    }

    *rval = INT_TO_JSVAL(nativeRet);
  }
  else
  {
    JS_ReportError(cx, "Function Uninstall requires 1 parameters");
    return JS_FALSE;
  }

  return JS_TRUE;
}


/*START HACK FOR DEBUGGING UNTIL ALERTS WORK*/

PR_STATIC_CALLBACK(JSBool)
InstallTRACE(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsAutoString b0;

  ConvertJSValToStr(b0, cx, argv[0]);

  char *tempStr;
  tempStr = ToNewCString(b0);
  printf("Install:\t%s\n", tempStr);

  Recycle(tempStr);

  return JS_TRUE;
}

/*END HACK FOR DEBUGGING UNTIL ALERTS WORK*/


//
// Native method LogComment
//
PR_STATIC_CALLBACK(JSBool)
InstallLogComment(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);
  if (!nativeThis)
    return JS_FALSE;

  nsAutoString b0;

  *rval = JSVAL_NULL;

  if(argc >= 1)
  {
    //  public int LogComment (String aComment);

    ConvertJSValToStr(b0, cx, argv[0]);

    nativeThis->LogComment(b0);
  }
  else
  {
    JS_ReportError(cx, "Function LogComment requires 1 parameter");
    return JS_FALSE;
  }

  return JS_TRUE;
}


//
// Native method InstallAlert
//
PR_STATIC_CALLBACK(JSBool)
InstallAlert(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);
  if (!nativeThis)
    return JS_FALSE;

  nsAutoString b0;

  *rval = JSVAL_NULL;

  if(argc == 1)
  {
    //  public int InstallAlert (String aComment);

    ConvertJSValToStr(b0, cx, argv[0]);

    jsrefcount saveDepth = JS_SuspendRequest(cx);//Need to suspend use of thread or deadlock occurs

    nativeThis->Alert(b0);
    JS_ResumeRequest(cx, saveDepth);

  }
  else
  {
    JS_ReportError(cx, "Function Alert requires 1 parameter");
    return JS_FALSE;
  }

  return JS_TRUE;
}

//
// Native method InstallConfirm
//
PR_STATIC_CALLBACK(JSBool)
InstallConfirm(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  nsInstall *nativeThis =
    (nsInstall*)JS_GetInstancePrivate(cx, obj, &InstallClass, argv);
  if (!nativeThis)
    return JS_FALSE;

  nsAutoString text;
  nsAutoString title;
  PRUint32 buttonFlags = nsIPromptService::STD_OK_CANCEL_BUTTONS;
  nsAutoString button0Title;
  nsAutoString button1Title;
  nsAutoString button2Title;
  nsAutoString checkMsg;
  JSObject* checkObj = 0;
  jsval jsCheckState = 0;
  PRBool checkState = PR_FALSE;
  PRInt32 nativeRet;

  *rval = JSVAL_FALSE;

  if(argc == 0)
  {
    JS_ReportError(cx, "Function Confirm requires at least 1 parameter");
    return JS_FALSE;
  }

  //  public int InstallConfirm (String aComment);
  ConvertJSValToStr(text, cx, argv[0]);

  //  Any additional parameters are optional.
  //  public int InstallConfirm (String aText,
  //                             String aDialogTitle,
  //                             Number aButtonFlags,
  //                             String aButton0Title,
  //                             String aButton1Title,
  //                             String aButton2Title,
  //                             String aCheckMsg,
  //                             Object aCheckState);
  if (argc > 1)
    ConvertJSValToStr(title, cx, argv[1]);

  if(argc > 2)
  {
    if(!JSVAL_IS_INT(argv[2]))
    {
      JS_ReportError(cx, "Parameter 'aButtonFlags' must be a number");
      return JS_FALSE;
    }
    buttonFlags = JSVAL_TO_INT(argv[2]);
  }

  if (argc > 3)
    ConvertJSValToStr(button0Title, cx, argv[3]);
  if (argc > 4)
    ConvertJSValToStr(button1Title, cx, argv[4]);
  if (argc > 5)
    ConvertJSValToStr(button2Title, cx, argv[5]);
  if (argc > 6)
    ConvertJSValToStr(checkMsg, cx, argv[6]);

  if (argc > 7 && !JSVAL_IS_PRIMITIVE(argv[7])) 
  {
    checkObj = JSVAL_TO_OBJECT(argv[7]);
    if (!JS_GetProperty(cx, checkObj, "value", &jsCheckState) ||
      !JSVAL_IS_BOOLEAN(jsCheckState))
    {
      JS_ReportError(cx, "Parameter 'aCheckState' must have a boolean 'value' property");
      return JS_FALSE;
    }
    JS_ValueToBoolean(cx, jsCheckState, &checkState);
  }

  jsrefcount saveDepth = JS_SuspendRequest(cx); //Need to suspend use of thread or deadlock occurs

  nativeThis->ConfirmEx(title, text, buttonFlags, button0Title, button1Title, button2Title, checkMsg, &checkState, &nativeRet);

  JS_ResumeRequest(cx, saveDepth);

  // Only save back checkState when an object was passed as 8th parameter
  if (checkObj)
  {
    jsCheckState = BOOLEAN_TO_JSVAL(checkState);
    JS_SetProperty(cx, checkObj, "value", &jsCheckState);
  }

  // To maintain compatibility with the classic one-arg confirm that returned 1
  // for OK and 0 for cancel/close, we have to swap 0 and 1 here, as confirmEx
  // always returns 0 for the first (i.e. OK) button.
  if (nativeRet == 0)
    nativeRet = 1; // 0 = first button = OK => true(1)
  else if (nativeRet == 1)
    nativeRet = 0; // 1 = second button or window closed = Cancel => false(0)

  *rval = INT_TO_JSVAL(nativeRet);

  return JS_TRUE;
}

//
// Install class properties
//
static JSPropertySpec InstallProperties[] =
{
  {"platform",          INSTALL_PLATFORM,           JSPROP_ENUMERATE | JSPROP_READONLY},
  {"jarfile",           INSTALL_JARFILE,            JSPROP_ENUMERATE | JSPROP_READONLY},
  {"archive",           INSTALL_ARCHIVE,            JSPROP_ENUMERATE | JSPROP_READONLY},
  {"arguments",         INSTALL_ARGUMENTS,          JSPROP_ENUMERATE | JSPROP_READONLY},
  {"url",               INSTALL_URL,                JSPROP_ENUMERATE | JSPROP_READONLY},
  {"flags",             INSTALL_FLAGS,              JSPROP_ENUMERATE | JSPROP_READONLY},
  {"_finalStatus",      INSTALL_FINALSTATUS,        JSPROP_READONLY},
  {"Install",           INSTALL_INSTALL,            JSPROP_READONLY},
  {"_installedFiles",   INSTALL_INSTALLED_FILES,    JSPROP_READONLY},
  {0}
};


static JSConstDoubleSpec install_constants[] =
{
    { nsInstall::BAD_PACKAGE_NAME,           "BAD_PACKAGE_NAME"             },
    { nsInstall::UNEXPECTED_ERROR,           "UNEXPECTED_ERROR"             },
    { nsInstall::ACCESS_DENIED,              "ACCESS_DENIED"                },
    { nsInstall::EXECUTION_ERROR,            "EXECUTION_ERROR"              },
    { nsInstall::NO_INSTALL_SCRIPT,          "NO_INSTALL_SCRIPT"            },


    { nsInstall::CANT_READ_ARCHIVE,          "CANT_READ_ARCHIVE"            },
    { nsInstall::INVALID_ARGUMENTS,          "INVALID_ARGUMENTS"            },

    { nsInstall::USER_CANCELLED,             "USER_CANCELLED"               },
    { nsInstall::INSTALL_NOT_STARTED,        "INSTALL_NOT_STARTED"          },

    { nsInstall::NO_SUCH_COMPONENT,          "NO_SUCH_COMPONENT"            },
    { nsInstall::DOES_NOT_EXIST,             "DOES_NOT_EXIST"               },
    { nsInstall::READ_ONLY,                  "READ_ONLY"                    },
    { nsInstall::IS_DIRECTORY,               "IS_DIRECTORY"                 },
    { nsInstall::NETWORK_FILE_IS_IN_USE,     "NETWORK_FILE_IS_IN_USE"       },
    { nsInstall::APPLE_SINGLE_ERR,           "APPLE_SINGLE_ERR"             },

    { nsInstall::PATCH_BAD_DIFF,             "PATCH_BAD_DIFF"               },
    { nsInstall::PATCH_BAD_CHECKSUM_TARGET,  "PATCH_BAD_CHECKSUM_TARGET"    },
    { nsInstall::PATCH_BAD_CHECKSUM_RESULT,  "PATCH_BAD_CHECKSUM_RESULT"    },
    { nsInstall::UNINSTALL_FAILED,           "UNINSTALL_FAILED"             },
    { nsInstall::PACKAGE_FOLDER_NOT_SET,     "PACKAGE_FOLDER_NOT_SET"       },
    { nsInstall::EXTRACTION_FAILED,          "EXTRACTION_FAILED"            },
    { nsInstall::FILENAME_ALREADY_USED,      "FILENAME_ALREADY_USED"        },
    { nsInstall::INSTALL_CANCELLED,          "INSTALL_CANCELLED"            },
    { nsInstall::DOWNLOAD_ERROR,             "DOWNLOAD_ERROR"               },
    { nsInstall::SCRIPT_ERROR,               "SCRIPT_ERROR"                 },
    { nsInstall::ALREADY_EXISTS,             "ALREADY_EXISTS"               },
    { nsInstall::IS_FILE,                    "IS_FILE"                      },
    { nsInstall::SOURCE_DOES_NOT_EXIST,      "SOURCE_DOES_NOT_EXIST"        },
    { nsInstall::SOURCE_IS_DIRECTORY,        "SOURCE_IS_DIRECTORY"          },
    { nsInstall::SOURCE_IS_FILE,             "SOURCE_IS_FILE"               },
    { nsInstall::INSUFFICIENT_DISK_SPACE,    "INSUFFICIENT_DISK_SPACE"      },

    { nsInstall::UNABLE_TO_LOCATE_LIB_FUNCTION, "UNABLE_TO_LOCATE_LIB_FUNCTION"},
    { nsInstall::UNABLE_TO_LOAD_LIBRARY,     "UNABLE_TO_LOAD_LIBRARY"       },
    { nsInstall::CHROME_REGISTRY_ERROR,      "CHROME_REGISTRY_ERROR"        },
    { nsInstall::MALFORMED_INSTALL,          "MALFORMED_INSTALL"            },

    { nsInstall::KEY_ACCESS_DENIED,          "KEY_ACCESS_DENIED"            },
    { nsInstall::KEY_DOES_NOT_EXIST,         "KEY_DOES_NOT_EXIST"           },
    { nsInstall::VALUE_DOES_NOT_EXIST,       "VALUE_DOES_NOT_EXIST"         },

    { nsInstall::GESTALT_UNKNOWN_ERR,        "GESTALT_UNKNOWN_ERR"          },
    { nsInstall::GESTALT_INVALID_ARGUMENT,   "GESTALT_INVALID_ARGUMENT"     },

    { nsInstall::SUCCESS,                    "SUCCESS"                      },
    { nsInstall::REBOOT_NEEDED,              "REBOOT_NEEDED"                },
    { nsInstall::INVALID_SIGNATURE,          "INVALID_SIGNATURE"            },
    { nsInstall::INVALID_HASH,               "INVALID_HASH"                 },
    { nsInstall::INVALID_HASH_TYPE,          "INVALID_HASH_TYPE"            },

    // these are flags supported by confirm
    { nsIPromptService::BUTTON_POS_0,            "BUTTON_POS_0"             },
    { nsIPromptService::BUTTON_POS_1,            "BUTTON_POS_1"             },
    { nsIPromptService::BUTTON_POS_2,            "BUTTON_POS_2"             },
    { nsIPromptService::BUTTON_TITLE_OK,         "BUTTON_TITLE_OK"          },
    { nsIPromptService::BUTTON_TITLE_CANCEL,     "BUTTON_TITLE_CANCEL"      },
    { nsIPromptService::BUTTON_TITLE_YES,        "BUTTON_TITLE_YES"         },
    { nsIPromptService::BUTTON_TITLE_NO,         "BUTTON_TITLE_NO"          },
    { nsIPromptService::BUTTON_TITLE_SAVE,       "BUTTON_TITLE_SAVE"        },
    { nsIPromptService::BUTTON_TITLE_DONT_SAVE,  "BUTTON_TITLE_DONT_SAVE"   },
    { nsIPromptService::BUTTON_TITLE_REVERT,     "BUTTON_TITLE_REVERT"      },
    { nsIPromptService::BUTTON_TITLE_IS_STRING,  "BUTTON_TITLE_IS_STRING"   },
    { nsIPromptService::BUTTON_POS_0_DEFAULT,    "BUTTON_POS_0_DEFAULT"     },
    { nsIPromptService::BUTTON_POS_1_DEFAULT,    "BUTTON_POS_1_DEFAULT"     },
    { nsIPromptService::BUTTON_POS_2_DEFAULT,    "BUTTON_POS_2_DEFAULT"     },
    { nsIPromptService::BUTTON_DELAY_ENABLE,     "BUTTON_DELAY_ENABLE"      },
    { nsIPromptService::STD_OK_CANCEL_BUTTONS,   "STD_OK_CANCEL_BUTTONS"    },
    { nsIPromptService::STD_YES_NO_BUTTONS,      "STD_YES_NO_BUTTONS"       },

    // these are bitwise values supported by addFile
    { DO_NOT_UNINSTALL,                      "DO_NOT_UNINSTALL"             },
    { WIN_SHARED_FILE,                       "WIN_SHARED_FILE"              },
    { WIN_SYSTEM_FILE,                       "WIN_SYSTEM_FILE"              },

    { CHROME_SKIN,                           "SKIN"                         },
    { CHROME_LOCALE,                         "LOCALE"                       },
    { CHROME_CONTENT,                        "CONTENT"                      },
    { CHROME_ALL,                            "PACKAGE"                      },
    { CHROME_PROFILE,                        "PROFILE_CHROME"               },
    { CHROME_DELAYED,                        "DELAYED_CHROME"               },
    { CHROME_SELECT,                         "SELECT_CHROME"                },

    { NS_BUILD_ID,                           "buildID"                      },

    {0}
};

//
// Install class methods
//
static JSFunctionSpec InstallMethods[] =
{
/*START HACK FOR DEBUGGING UNTIL ALERTS WORK*/
  {"TRACE",                     InstallTRACE,                   1,0,0},
/*END HACK FOR DEBUGGING UNTIL ALERTS WORK*/
  // -- new forms that match prevailing javascript style --
  {"addDirectory",              InstallAddDirectory,            6,0,0},
  {"addFile",                   InstallAddSubcomponent,         6,0,0},
  {"alert",                     InstallAlert,                   1,0,0},
  {"cancelInstall",             InstallAbortInstall,            1,0,0},
  {"confirm",                   InstallConfirm,                 8,0,0},
  {"execute",                   InstallExecute,                 2,0,0},
  {"gestalt",                   InstallGestalt,                 1,0,0},
  {"getComponentFolder",        InstallGetComponentFolder,      2,0,0},
  {"getFolder",                 InstallGetFolder,               2,0,0},
  {"getLastError",              InstallGetLastError,            0,0,0},
  {"getWinProfile",             InstallGetWinProfile,           2,0,0},
  {"getWinRegistry",            InstallGetWinRegistry,          0,0,0},
  {"initInstall",               InstallStartInstall,            4,0,0},
  {"loadResources",             InstallLoadResources,           1,0,0},
  {"logComment",                InstallLogComment,              1,0,0},
  {"patch",                     InstallPatch,                   5,0,0},
  {"performInstall",            InstallFinalizeInstall,         0,0,0},
  {"registerChrome",            InstallRegisterChrome,          2,0,0},
  {"refreshPlugins",            InstallRefreshPlugins,          1,0,0},
  {"resetError",                InstallResetError,              1,0,0},
//  {"selectChrome",              InstallSelectChrome,            2,0,0},
  {"setPackageFolder",          InstallSetPackageFolder,        1,0,0},
  {"uninstall",                 InstallUninstall,               1,0,0},

  // the raw file methods are deprecated, use the File object instead
  {"dirCreate",                 InstallFileOpDirCreate,                1,0,0},
  {"dirGetParent",              InstallFileOpDirGetParent,             1,0,0},
  {"dirRemove",                 InstallFileOpDirRemove,                2,0,0},
  {"dirRename",                 InstallFileOpDirRename,                2,0,0},
  {"fileCopy",                  InstallFileOpFileCopy,                 2,0,0},
  {"fileDelete",                InstallFileOpFileRemove,               1,0,0},
  {"fileExists",                InstallFileOpFileExists,               1,0,0},
  {"fileExecute",               InstallFileOpFileExecute,              2,0,0},
  {"fileGetNativeVersion",      InstallFileOpFileGetNativeVersion,     1,0,0},
  {"fileGetDiskSpaceAvailable", InstallFileOpFileGetDiskSpaceAvailable,1,0,0},
  {"fileGetModDate",            InstallFileOpFileGetModDate,           1,0,0},
  {"fileGetSize",               InstallFileOpFileGetSize,              1,0,0},
  {"fileIsDirectory",           InstallFileOpFileIsDirectory,          1,0,0},
  {"fileIsWritable",            InstallFileOpFileIsWritable,           1,0,0},
  {"fileIsFile",                InstallFileOpFileIsFile,               1,0,0},
  {"fileModDateChanged",        InstallFileOpFileModDateChanged,       2,0,0},
  {"fileMove",                  InstallFileOpFileMove,                 2,0,0},
  {"fileRename",                InstallFileOpFileRename,               2,0,0},
  {"fileWindowsShortcut",       InstallFileOpFileWindowsShortcut,      7,0,0},
  {"fileMacAlias",              InstallFileOpFileMacAlias,             2,0,0},
  {"fileUnixLink",              InstallFileOpFileUnixLink,             2,0,0},

  // -- documented but never supported --
  {"deleteRegisteredFile",      InstallDeleteComponent,         1,0,0},

  // -- obsolete forms for temporary compatibility --
  {"abortInstall",              InstallAbortInstall,            1,0,0},
  {"finalizeInstall",           InstallFinalizeInstall,         0,0,0},
  {"startInstall",              InstallStartInstall,            4,0,0},
  {nsnull,nsnull,0,0,0}
};


JSObject * InitXPInstallObjects(JSContext *jscontext,
                             nsIFile* jarfile,
                             const PRUnichar* url,
                             const PRUnichar* args,
                             PRUint32 flags,
                             CHROMEREG_IFACE* reg,
                             nsIZipReader * theJARFile)
{
  JSObject *installObject;
  nsInstall *nativeInstallObject;

  // new global object
  installObject = JS_NewObject(jscontext, &InstallClass, nsnull, nsnull);
  if (!installObject)
    return nsnull;

  if (!JS_DefineProperty(jscontext, installObject, InstallClass.name,
                         OBJECT_TO_JSVAL(installObject), NULL, NULL, 0))
    return nsnull;

  if (!JS_DefineProperties(jscontext, installObject, InstallProperties))
    return nsnull;

  if (!JS_DefineFunctions(jscontext, installObject, InstallMethods))
    return nsnull;

  if (!JS_DefineConstDoubles(jscontext, installObject, install_constants))
    return nsnull;

  nativeInstallObject = new nsInstall(theJARFile);
  if (!nativeInstallObject)
    return nsnull;

  nativeInstallObject->SetJarFileLocation(jarfile);
  nativeInstallObject->SetInstallArguments(nsAutoString(args));
  nativeInstallObject->SetInstallURL(nsAutoString(url));
  nativeInstallObject->SetInstallFlags(flags);
  nativeInstallObject->SetChromeRegistry(reg);

  JS_SetPrivate(jscontext, installObject, nativeInstallObject);
  nativeInstallObject->SetScriptObject(installObject);


  //
  // Initialize and create the FileOp object
  //
  if(NS_OK != InitXPFileOpObjectPrototype(jscontext, installObject, &gFileOpProto))
  {
    return nsnull;
  }

  gFileOpObject = JS_NewObject(jscontext, &FileOpClass, gFileOpProto, nsnull);
  if (gFileOpObject == nsnull)
    return nsnull;

  JS_SetPrivate(jscontext, gFileOpObject, nativeInstallObject);

  if (!JS_DefineProperty (jscontext,
                          installObject,
                          "File", 
                          OBJECT_TO_JSVAL(gFileOpObject),
                          JS_PropertyStub, 
                          JS_PropertyStub, 
                          JSPROP_READONLY | JSPROP_PERMANENT))
    return nsnull;

  //
  // Initialize the FileSpecObject
  //
  if(NS_OK != InitFileSpecObjectPrototype(jscontext, installObject, &gFileSpecProto))
  {
    return nsnull;
  }


  //
  // Initialize platform specific helper objects
  //
#ifdef _WINDOWS
  JSObject *winRegPrototype     = nsnull;
  JSObject *winProfilePrototype = nsnull;

  if(NS_OK != InitWinRegPrototype(jscontext, installObject, &winRegPrototype))
  {
    return nsnull;
  }
  nativeInstallObject->SaveWinRegPrototype(winRegPrototype);

  if(NS_OK != InitWinProfilePrototype(jscontext, installObject, &winProfilePrototype))
  {
    return nsnull;
  }
  nativeInstallObject->SaveWinProfilePrototype(winProfilePrototype);
#endif

  return installObject;
}

