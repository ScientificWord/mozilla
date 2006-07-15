/* vim:set ts=4 sw=4 sts=4 et cindent: */
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
 * The Original Code is the SSPI NegotiateAuth Module
 *
 * The Initial Developer of the Original Code is IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@meer.net>
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

//
// Negotiate Authentication Support Module
//
// Described by IETF Internet draft: draft-brezak-kerberos-http-00.txt
// (formerly draft-brezak-spnego-http-04.txt)
//
// Also described here:
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dnsecure/html/http-sso-1.asp
//

#include "nsAuthSSPI.h"
#include "nsIServiceManager.h"
#include "nsIDNSService.h"
#include "nsIDNSRecord.h"
#include "nsNetCID.h"
#include "nsCOMPtr.h"

#define SEC_SUCCESS(Status) ((Status) >= 0)

#ifndef KERB_WRAP_NO_ENCRYPT
#define KERB_WRAP_NO_ENCRYPT 0x80000001
#endif

#ifndef SECBUFFER_PADDING
#define SECBUFFER_PADDING 9
#endif

#ifndef SECBUFFER_STREAM
#define SECBUFFER_STREAM 10
#endif

//-----------------------------------------------------------------------------

static const char *const pTypeName [] = {
    "Kerberos",
    "Negotiate",
    "NTLM"
};

#ifdef DEBUG
#define CASE_(_x) case _x: return # _x;
static const char *MapErrorCode(int rc)
{
    switch (rc) {
    CASE_(SEC_E_OK)
    CASE_(SEC_I_CONTINUE_NEEDED)
    CASE_(SEC_I_COMPLETE_NEEDED)
    CASE_(SEC_I_COMPLETE_AND_CONTINUE)
    CASE_(SEC_E_INCOMPLETE_MESSAGE)
    CASE_(SEC_I_INCOMPLETE_CREDENTIALS)
    CASE_(SEC_E_INVALID_HANDLE)
    CASE_(SEC_E_TARGET_UNKNOWN)
    CASE_(SEC_E_LOGON_DENIED)
    CASE_(SEC_E_INTERNAL_ERROR)
    CASE_(SEC_E_NO_CREDENTIALS)
    CASE_(SEC_E_NO_AUTHENTICATING_AUTHORITY)
    CASE_(SEC_E_INSUFFICIENT_MEMORY)
    CASE_(SEC_E_INVALID_TOKEN)
    }
    return "<unknown>";
}
#else
#define MapErrorCode(_rc) ""
#endif

//-----------------------------------------------------------------------------

static HINSTANCE                 sspi_lib; 
static PSecurityFunctionTable    sspi;

static nsresult
InitSSPI()
{
    PSecurityFunctionTable (*initFun)(void);

    LOG(("  InitSSPI\n"));

    sspi_lib = LoadLibrary("secur32.dll");
    if (!sspi_lib) {
        sspi_lib = LoadLibrary("security.dll");
        if (!sspi_lib) {
            LOG(("SSPI library not found"));
            return NS_ERROR_UNEXPECTED;
        }
    }

    initFun = (PSecurityFunctionTable (*)(void))
            GetProcAddress(sspi_lib, "InitSecurityInterfaceA");
    if (!initFun) {
        LOG(("InitSecurityInterfaceA not found"));
        return NS_ERROR_UNEXPECTED;
    }

    sspi = initFun();
    if (!sspi) {
        LOG(("InitSecurityInterfaceA failed"));
        return NS_ERROR_UNEXPECTED;
    }

    return NS_OK;
}

//-----------------------------------------------------------------------------

static nsresult
MakeSN(const char *principal, nsCString &result)
{
    nsresult rv;

    nsCAutoString buf(principal);

    // The service name looks like "protocol@hostname", we need to map
    // this to a value that SSPI expects.  To be consistent with IE, we
    // need to map '@' to '/' and canonicalize the hostname.
    PRInt32 index = buf.FindChar('@');
    if (index == kNotFound)
        return NS_ERROR_UNEXPECTED;
    
    nsCOMPtr<nsIDNSService> dns = do_GetService(NS_DNSSERVICE_CONTRACTID, &rv);
    if (NS_FAILED(rv))
        return rv;

    // This could be expensive if our DNS cache cannot satisfy the request.
    // However, we should have at least hit the OS resolver once prior to
    // reaching this code, so provided the OS resolver has this information
    // cached, we should not have to worry about blocking on this function call
    // for very long.  NOTE: because we ask for the canonical hostname, we
    // might end up requiring extra network activity in cases where the OS
    // resolver might not have enough information to satisfy the request from
    // its cache.  This is not an issue in versions of Windows up to WinXP.
    nsCOMPtr<nsIDNSRecord> record;
    rv = dns->Resolve(Substring(buf, index + 1),
                      nsIDNSService::RESOLVE_CANONICAL_NAME,
                      getter_AddRefs(record));
    if (NS_FAILED(rv))
        return rv;

    nsCAutoString cname;
    rv = record->GetCanonicalName(cname);
    if (NS_SUCCEEDED(rv)) {
        result = StringHead(buf, index) + NS_LITERAL_CSTRING("/") + cname;
        LOG(("Using SPN of [%s]\n", result.get()));
    }
    return rv;
}

//-----------------------------------------------------------------------------

nsAuthSSPI::nsAuthSSPI(pType package)
    : mServiceFlags(REQ_DEFAULT)
    , mMaxTokenLen(0)
    , mPackage(package)
{
    memset(&mCred, 0, sizeof(mCred));
    memset(&mCtxt, 0, sizeof(mCtxt));
}

nsAuthSSPI::~nsAuthSSPI()
{
    Reset();

    if (mCred.dwLower || mCred.dwUpper) {
#ifdef __MINGW32__
        (sspi->FreeCredentialsHandle)(&mCred);
#else
        (sspi->FreeCredentialHandle)(&mCred);
#endif
        memset(&mCred, 0, sizeof(mCred));
    }
}

void
nsAuthSSPI::Reset()
{
    if (mCtxt.dwLower || mCtxt.dwUpper) {
        (sspi->DeleteSecurityContext)(&mCtxt);
        memset(&mCtxt, 0, sizeof(mCtxt));
    }
}

NS_IMPL_ISUPPORTS1(nsAuthSSPI, nsIAuthModule)

NS_IMETHODIMP
nsAuthSSPI::Init(const char *serviceName,
                 PRUint32    serviceFlags,
                 const PRUnichar *domain,
                 const PRUnichar *username,
                 const PRUnichar *password)
{
    LOG(("  nsAuthSSPI::Init\n"));

    // we don't expect to be passed any user credentials
    NS_ASSERTION(!domain && !username && !password, "unexpected credentials");

    // if we're configured for SPNEGO (Negotiate) or Kerberos, then it's critical 
    // that the caller supply a service name to be used.
    if (mPackage != PACKAGE_TYPE_NTLM)
        NS_ENSURE_TRUE(serviceName && *serviceName, NS_ERROR_INVALID_ARG);

    nsresult rv;

    // XXX lazy initialization like this assumes that we are single threaded
    if (!sspi) {
        rv = InitSSPI();
        if (NS_FAILED(rv))
            return rv;
    }

    SEC_CHAR *package;

    package = (SEC_CHAR *) pTypeName[(int)mPackage];

    if (mPackage != PACKAGE_TYPE_NTLM)
    {
        rv = MakeSN(serviceName, mServiceName);
        if (NS_FAILED(rv))
            return rv;
        mServiceFlags = serviceFlags;
    }

    SECURITY_STATUS rc;

    PSecPkgInfo pinfo;
    rc = (sspi->QuerySecurityPackageInfo)(package, &pinfo);
    if (rc != SEC_E_OK) {
        LOG(("%s package not found\n", package));
        return NS_ERROR_UNEXPECTED;
    }
    mMaxTokenLen = pinfo->cbMaxToken;
    (sspi->FreeContextBuffer)(pinfo);

    TimeStamp useBefore;

    rc = (sspi->AcquireCredentialsHandle)(NULL,
                                          package,
                                          SECPKG_CRED_OUTBOUND,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          &mCred,
                                          &useBefore);
    if (rc != SEC_E_OK)
        return NS_ERROR_UNEXPECTED;

    return NS_OK;
}

NS_IMETHODIMP
nsAuthSSPI::GetNextToken(const void *inToken,
                         PRUint32    inTokenLen,
                         void      **outToken,
                         PRUint32   *outTokenLen)
{
    SECURITY_STATUS rc;
    TimeStamp ignored;

    DWORD ctxAttr, ctxReq = 0;
    CtxtHandle *ctxIn;
    SecBufferDesc ibd, obd;
    SecBuffer ib, ob;

    LOG(("entering nsAuthSSPI::GetNextToken()\n"));

    if (mServiceFlags & REQ_DELEGATE)
        ctxReq |= ISC_REQ_DELEGATE;
    if (mServiceFlags & REQ_MUTUAL_AUTH)
        ctxReq |= ISC_REQ_MUTUAL_AUTH;

    if (inToken) {
        ib.BufferType = SECBUFFER_TOKEN;
        ib.cbBuffer = inTokenLen;
        ib.pvBuffer = (void *) inToken;
        ibd.ulVersion = SECBUFFER_VERSION;
        ibd.cBuffers = 1;
        ibd.pBuffers = &ib;
        ctxIn = &mCtxt;
    }
    else {
        // If there is no input token, then we are starting a new
        // authentication sequence.  If we have already initialized our
        // security context, then we're in trouble because it means that the
        // first sequence failed.  We need to bail or else we might end up in
        // an infinite loop.
        if (mCtxt.dwLower || mCtxt.dwUpper) {
            LOG(("Cannot restart authentication sequence!"));
            return NS_ERROR_UNEXPECTED;
        }

        ctxIn = NULL;
    }

    obd.ulVersion = SECBUFFER_VERSION;
    obd.cBuffers = 1;
    obd.pBuffers = &ob;
    ob.BufferType = SECBUFFER_TOKEN;
    ob.cbBuffer = mMaxTokenLen;
    ob.pvBuffer = nsMemory::Alloc(ob.cbBuffer);
    if (!ob.pvBuffer)
        return NS_ERROR_OUT_OF_MEMORY;
    memset(ob.pvBuffer, 0, ob.cbBuffer);

    SEC_CHAR *sn;

    if (mPackage == PACKAGE_TYPE_NTLM)
        sn = NULL;
    else
        sn = (SEC_CHAR *) mServiceName.get();

    rc = (sspi->InitializeSecurityContext)(&mCred,
                                           ctxIn,
                                           sn,
                                           ctxReq,
                                           0,
                                           SECURITY_NATIVE_DREP,
                                           inToken ? &ibd : NULL,
                                           0,
                                           &mCtxt,
                                           &obd,
                                           &ctxAttr,
                                           &ignored);
    if (rc == SEC_I_CONTINUE_NEEDED || rc == SEC_E_OK) {
        if (!ob.cbBuffer) {
            nsMemory::Free(ob.pvBuffer);
            ob.pvBuffer = NULL;
        }
        *outToken = ob.pvBuffer;
        *outTokenLen = ob.cbBuffer;

        if (rc == SEC_E_OK)
            return NS_SUCCESS_AUTH_FINISHED;

        return NS_OK;
    }

    LOG(("InitializeSecurityContext failed [rc=%d:%s]\n", rc, MapErrorCode(rc)));
    Reset();
    nsMemory::Free(ob.pvBuffer);
    return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsAuthSSPI::Unwrap(const void *inToken,
                   PRUint32    inTokenLen,
                   void      **outToken,
                   PRUint32   *outTokenLen)
{
    SECURITY_STATUS rc;
    SecBufferDesc ibd;
    SecBuffer ib[2];

    ibd.cBuffers = 2;
    ibd.pBuffers = ib;
    ibd.ulVersion = SECBUFFER_VERSION; 

    // SSPI Buf
    ib[0].BufferType = SECBUFFER_STREAM;
    ib[0].cbBuffer = inTokenLen;
    ib[0].pvBuffer = nsMemory::Alloc(ib[0].cbBuffer);
    if (!ib[0].pvBuffer)
        return NS_ERROR_OUT_OF_MEMORY;
    
    memcpy(ib[0].pvBuffer, inToken, inTokenLen);

    // app data
    ib[1].BufferType = SECBUFFER_DATA;
    ib[1].cbBuffer = 0;
    ib[1].pvBuffer = NULL;

    rc = (sspi->DecryptMessage)(
                                &mCtxt,
                                &ibd,
                                0, // no sequence numbers
                                NULL
                                );

    if (SEC_SUCCESS(rc)) {
        *outToken = ib[1].pvBuffer;
        *outTokenLen = ib[1].cbBuffer;
    }
    else
        nsMemory::Free(ib[1].pvBuffer);

    nsMemory::Free(ib[0].pvBuffer);

    if (!SEC_SUCCESS(rc))
        return NS_ERROR_FAILURE;

    return NS_OK;
}

// utility class used to free memory on exit
class secBuffers
{
public:

    SecBuffer ib[3];

    secBuffers() { memset(&ib, 0, sizeof(ib)); }

    ~secBuffers() 
    {
        if (ib[0].pvBuffer)
            nsMemory::Free(ib[0].pvBuffer);

        if (ib[1].pvBuffer)
            nsMemory::Free(ib[1].pvBuffer);

        if (ib[2].pvBuffer)
            nsMemory::Free(ib[2].pvBuffer);
    }
};

NS_IMETHODIMP
nsAuthSSPI::Wrap(const void *inToken,
                 PRUint32    inTokenLen,
                 PRBool      confidential,
                 void      **outToken,
                 PRUint32   *outTokenLen)
{
    SECURITY_STATUS rc;

    SecBufferDesc ibd;
    secBuffers bufs;
    SecPkgContext_Sizes sizes;

    rc = (sspi->QueryContextAttributes)(
         &mCtxt,
         SECPKG_ATTR_SIZES,
         &sizes);

    if (!SEC_SUCCESS(rc))  
        return NS_ERROR_FAILURE;
    
    ibd.cBuffers = 3;
    ibd.pBuffers = bufs.ib;
    ibd.ulVersion = SECBUFFER_VERSION;
    
    // SSPI
    bufs.ib[0].cbBuffer = sizes.cbSecurityTrailer;
    bufs.ib[0].BufferType = SECBUFFER_TOKEN;
    bufs.ib[0].pvBuffer = nsMemory::Alloc(sizes.cbSecurityTrailer);

    if (!bufs.ib[0].pvBuffer)
        return NS_ERROR_OUT_OF_MEMORY;

    // APP Data
    bufs.ib[1].BufferType = SECBUFFER_DATA;
    bufs.ib[1].pvBuffer = nsMemory::Alloc(inTokenLen);
    bufs.ib[1].cbBuffer = inTokenLen;
    
    if (!bufs.ib[1].pvBuffer)
        return NS_ERROR_OUT_OF_MEMORY;

    memcpy(bufs.ib[1].pvBuffer, inToken, inTokenLen);

    // SSPI
    bufs.ib[2].BufferType = SECBUFFER_PADDING;
    bufs.ib[2].cbBuffer = sizes.cbBlockSize;
    bufs.ib[2].pvBuffer = nsMemory::Alloc(bufs.ib[2].cbBuffer);

    if (!bufs.ib[2].pvBuffer)
        return NS_ERROR_OUT_OF_MEMORY;

    rc = (sspi->EncryptMessage)(&mCtxt,
          confidential ? 0 : KERB_WRAP_NO_ENCRYPT,
         &ibd, 0);

    if (SEC_SUCCESS(rc)) {
        int len  = bufs.ib[0].cbBuffer + bufs.ib[1].cbBuffer + bufs.ib[2].cbBuffer;
        char *p = (char *) nsMemory::Alloc(len);

        if (!p)
            return NS_ERROR_OUT_OF_MEMORY;
				
        *outToken = (void *) p;
        *outTokenLen = len;

        memcpy(p, bufs.ib[0].pvBuffer, bufs.ib[0].cbBuffer);
        p += bufs.ib[0].cbBuffer;

        memcpy(p,bufs.ib[1].pvBuffer, bufs.ib[1].cbBuffer);
        p += bufs.ib[1].cbBuffer;

        memcpy(p,bufs.ib[2].pvBuffer, bufs.ib[2].cbBuffer);
        
        return NS_OK;
    }

    return NS_ERROR_FAILURE;
}
