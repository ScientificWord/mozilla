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

#include "necko-config.h"

#include "nsCOMPtr.h"
#include "nsIModule.h"
#include "nsIClassInfoImpl.h"
#include "nsIGenericFactory.h"
#include "nsIComponentManager.h"
#include "nsIServiceManager.h"
#include "nsICategoryManager.h"
#include "nsSocketProviderService.h"
#include "nscore.h"
#include "nsSimpleURI.h"
#include "nsSimpleNestedURI.h"
#include "nsLoadGroup.h"
#include "nsStreamLoader.h"
#include "nsUnicharStreamLoader.h"
#include "nsFileStreams.h"
#include "nsBufferedStreams.h"
#include "nsMIMEInputStream.h"
#include "nsSOCKSSocketProvider.h"
#include "nsCacheService.h"
#include "nsMimeTypes.h"
#include "nsNetStrings.h"

#include "nsNetCID.h"

#if defined(XP_MACOSX)
#define BUILD_APPLEFILE_DECODER 1
#else
#define BUILD_BINHEX_DECODER 1
#endif

///////////////////////////////////////////////////////////////////////////////

#include "nsIOService.h"
NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(nsIOService, nsIOService::GetInstance)

#include "nsDNSService2.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsDNSService, Init)
  
#include "nsProtocolProxyService.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsProtocolProxyService, Init)
NS_DECL_CLASSINFO(nsProtocolProxyService)

#include "nsStreamTransportService.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsStreamTransportService, Init)

#include "nsSocketTransportService2.h"
#undef LOG
#undef LOG_ENABLED
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsSocketTransportService, Init)

#include "nsServerSocket.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsServerSocket)

#include "nsUDPSocketProvider.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsUDPSocketProvider)

#include "nsAsyncStreamCopier.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAsyncStreamCopier)

#include "nsInputStreamPump.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsInputStreamPump)

#include "nsInputStreamChannel.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsInputStreamChannel, Init)

#include "nsDownloader.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDownloader)

#include "nsSyncStreamListener.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsSyncStreamListener, Init)

NS_GENERIC_FACTORY_CONSTRUCTOR(nsSafeFileOutputStream)

NS_GENERIC_AGGREGATED_CONSTRUCTOR_INIT(nsLoadGroup, Init)

#include "nsEffectiveTLDService.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsEffectiveTLDService, Init)

///////////////////////////////////////////////////////////////////////////////

extern NS_METHOD
net_NewIncrementalDownload(nsISupports *, const nsIID &, void **);

#define NS_INCREMENTALDOWNLOAD_CLASSNAME \
    "nsIncrementalDownload"
#define NS_INCREMENTALDOWNLOAD_CID \
{ /* a62af1ba-79b3-4896-8aaf-b148bfce4280 */         \
    0xa62af1ba,                                      \
    0x79b3,                                          \
    0x4896,                                          \
    {0x8a, 0xaf, 0xb1, 0x48, 0xbf, 0xce, 0x42, 0x80} \
}

///////////////////////////////////////////////////////////////////////////////

#include "nsStreamConverterService.h"

#ifdef BUILD_APPLEFILE_DECODER
#include "nsAppleFileDecoder.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAppleFileDecoder)
#endif

///////////////////////////////////////////////////////////////////////////////

#include "nsMIMEHeaderParamImpl.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsMIMEHeaderParamImpl)
///////////////////////////////////////////////////////////////////////////////

#include "nsRequestObserverProxy.h"
#include "nsSimpleStreamListener.h"
#include "nsDirIndexParser.h"
#include "nsDirIndex.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsRequestObserverProxy)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsSimpleStreamListener)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsDirIndexParser, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDirIndex)

///////////////////////////////////////////////////////////////////////////////

#include "nsStreamListenerTee.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsStreamListenerTee)

///////////////////////////////////////////////////////////////////////////////

#ifdef NECKO_COOKIES
#include "nsCookieService.h"
NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(nsCookieService, nsCookieService::GetSingleton)
#endif

///////////////////////////////////////////////////////////////////////////////
// protocols
///////////////////////////////////////////////////////////////////////////////

// about:blank is mandatory
#include "nsAboutProtocolHandler.h"
#include "nsAboutBlank.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAboutProtocolHandler)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsSafeAboutProtocolHandler)

#ifdef NECKO_PROTOCOL_about
// about
#ifdef NS_BUILD_REFCNT_LOGGING
#include "nsAboutBloat.h"
#endif
#include "nsAboutCache.h"
#include "nsAboutCacheEntry.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAboutCacheEntry)
#endif
  
#ifdef NECKO_PROTOCOL_file
// file
#include "nsFileProtocolHandler.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsFileProtocolHandler, Init)
#endif

#ifdef NECKO_PROTOCOL_ftp
// ftp
#include "nsFtpProtocolHandler.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsFtpProtocolHandler, Init)
#endif

#ifdef NECKO_PROTOCOL_http
// http/https
#include "nsHttpHandler.h"
#include "nsHttpAuthManager.h"
#include "nsHttpBasicAuth.h"
#include "nsHttpDigestAuth.h"
#include "nsHttpNTLMAuth.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsHttpNTLMAuth)
#undef LOG
#undef LOG_ENABLED
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsHttpHandler, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsHttpsHandler, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsHttpAuthManager, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsHttpBasicAuth)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsHttpDigestAuth)
#endif // !NECKO_PROTOCOL_http
  
#ifdef NECKO_PROTOCOL_res
// resource
#include "nsResProtocolHandler.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsResProtocolHandler, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsResURL)
#endif

#ifdef NECKO_PROTOCOL_gopher
#include "nsGopherHandler.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsGopherHandler)
#endif

#ifdef NECKO_PROTOCOL_viewsource
#include "nsViewSourceHandler.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsViewSourceHandler)
#endif

#ifdef NECKO_PROTOCOL_data
#include "nsDataHandler.h"
#endif

///////////////////////////////////////////////////////////////////////////////

#include "nsURIChecker.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsURIChecker)

///////////////////////////////////////////////////////////////////////////////

#include "nsURLParsers.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsNoAuthURLParser)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAuthURLParser)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsStdURLParser)

#include "nsStandardURL.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsStandardURL)

NS_GENERIC_AGGREGATED_CONSTRUCTOR(nsSimpleURI)

NS_GENERIC_FACTORY_CONSTRUCTOR(nsSimpleNestedURI)

///////////////////////////////////////////////////////////////////////////////

#include "nsIDNService.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsIDNService, Init)

///////////////////////////////////////////////////////////////////////////////
#if defined(XP_WIN) && !defined(WINCE)
#include "nsNotifyAddrListener.h"
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsNotifyAddrListener, Init)
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef NECKO_PROTOCOL_ftp
#include "nsFTPDirListingConv.h"
nsresult NS_NewFTPDirListingConv(nsFTPDirListingConv** result);
#endif

#ifdef NECKO_PROTOCOL_gopher
#include "nsGopherDirListingConv.h"
NS_GENERIC_FACTORY_CONSTRUCTOR(nsGopherDirListingConv)
#endif

#include "nsMultiMixedConv.h"
#include "nsHTTPCompressConv.h"
#include "mozTXTToHTMLConv.h"
#include "nsUnknownDecoder.h"
#include "nsTXTToHTMLConv.h"
#include "nsIndexedToHTML.h"
#ifdef BUILD_BINHEX_DECODER
#include "nsBinHexDecoder.h"
#endif

nsresult NS_NewMultiMixedConv (nsMultiMixedConv** result);
nsresult MOZ_NewTXTToHTMLConv (mozTXTToHTMLConv** result);
nsresult NS_NewHTTPCompressConv  (nsHTTPCompressConv ** result);
nsresult NS_NewNSTXTToHTMLConv(nsTXTToHTMLConv** result);
nsresult NS_NewStreamConv(nsStreamConverterService **aStreamConv);

#define FTP_TO_INDEX                 "?from=text/ftp-dir&to=application/http-index-format"
#define GOPHER_TO_INDEX              "?from=text/gopher-dir&to=application/http-index-format"
#define INDEX_TO_HTML                "?from=application/http-index-format&to=text/html"
#define MULTI_MIXED_X                "?from=multipart/x-mixed-replace&to=*/*"
#define MULTI_MIXED                  "?from=multipart/mixed&to=*/*"
#define MULTI_BYTERANGES             "?from=multipart/byteranges&to=*/*"
#define UNKNOWN_CONTENT              "?from=" UNKNOWN_CONTENT_TYPE "&to=*/*"
#define GZIP_TO_UNCOMPRESSED         "?from=gzip&to=uncompressed"
#define XGZIP_TO_UNCOMPRESSED        "?from=x-gzip&to=uncompressed"
#define COMPRESS_TO_UNCOMPRESSED     "?from=compress&to=uncompressed"
#define XCOMPRESS_TO_UNCOMPRESSED    "?from=x-compress&to=uncompressed"
#define DEFLATE_TO_UNCOMPRESSED      "?from=deflate&to=uncompressed"
#define PLAIN_TO_HTML                "?from=text/plain&to=text/html"

#ifdef BUILD_BINHEX_DECODER
#define BINHEX_TO_WILD               "?from=application/mac-binhex40&to=*/*"
#endif

static const char *const sStreamConverterArray[] = {
    FTP_TO_INDEX,
    GOPHER_TO_INDEX,
    INDEX_TO_HTML,
    MULTI_MIXED_X,
    MULTI_MIXED,
    MULTI_BYTERANGES,
    UNKNOWN_CONTENT,
    GZIP_TO_UNCOMPRESSED,
    XGZIP_TO_UNCOMPRESSED,
    COMPRESS_TO_UNCOMPRESSED,
    XCOMPRESS_TO_UNCOMPRESSED,
    DEFLATE_TO_UNCOMPRESSED,
#ifdef BUILD_BINHEX_DECODER
    BINHEX_TO_WILD,
#endif
    PLAIN_TO_HTML
};

static const PRUint32 sStreamConverterCount =
    NS_ARRAY_LENGTH(sStreamConverterArray);

// each stream converter must add its from/to key to the category manager
// in RegisterStreamConverters(). This provides a string representation
// of each registered converter, rooted in the NS_ISTREAMCONVERTER_KEY
// category. These keys are then (when the stream converter service
// needs to chain converters together) enumerated and parsed to build
// a graph of converters for potential chaining.
static NS_METHOD
RegisterStreamConverters(nsIComponentManager *aCompMgr, nsIFile *aPath,
                         const char *registryLocation,
                         const char *componentType,
                         const nsModuleComponentInfo *info) {
    nsCOMPtr<nsICategoryManager> catmgr =
        do_GetService(NS_CATEGORYMANAGER_CONTRACTID);
    NS_ENSURE_STATE(catmgr);

    for (PRUint32 count = 0; count < sStreamConverterCount; ++count) {
        catmgr->AddCategoryEntry(NS_ISTREAMCONVERTER_KEY,
                                 sStreamConverterArray[count], "",
                                 PR_TRUE, PR_TRUE, nsnull);
    }
    return NS_OK;
}

// same as RegisterStreamConverters except the reverse.
static NS_METHOD
UnregisterStreamConverters(nsIComponentManager *aCompMgr, nsIFile *aPath,
                           const char *registryLocation,
                           const nsModuleComponentInfo *info) {
    nsCOMPtr<nsICategoryManager> catmgr =
        do_GetService(NS_CATEGORYMANAGER_CONTRACTID);
    NS_ENSURE_STATE(catmgr);

    for (PRUint32 count = 0; count < sStreamConverterCount; ++count) {
        catmgr->DeleteCategoryEntry(NS_ISTREAMCONVERTER_KEY, 
                                    sStreamConverterArray[count], 
                                    PR_TRUE);
    }
    return NS_OK;
}

#ifdef BUILD_BINHEX_DECODER
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBinHexDecoder)
#endif

static NS_IMETHODIMP                 
CreateNewStreamConvServiceFactory(nsISupports* aOuter, REFNSIID aIID, void **aResult) 
{
    if (!aResult) {                                                  
        return NS_ERROR_INVALID_POINTER;                             
    }
    if (aOuter) {                                                    
        *aResult = nsnull;                                           
        return NS_ERROR_NO_AGGREGATION;                              
    }   
    nsStreamConverterService* inst = nsnull;
    nsresult rv = NS_NewStreamConv(&inst);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
        return rv;                                                   
    } 
    rv = inst->QueryInterface(aIID, aResult);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
    }                                                                
    NS_RELEASE(inst);             /* get rid of extra refcnt */      
    return rv;              
}

#ifdef NECKO_PROTOCOL_ftp
static NS_IMETHODIMP                 
CreateNewFTPDirListingConv(nsISupports* aOuter, REFNSIID aIID, void **aResult) 
{
    if (!aResult) {                                                  
        return NS_ERROR_INVALID_POINTER;                             
    }
    if (aOuter) {                                                    
        *aResult = nsnull;                                           
        return NS_ERROR_NO_AGGREGATION;                              
    }   
    nsFTPDirListingConv* inst = nsnull;
    nsresult rv = NS_NewFTPDirListingConv(&inst);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
        return rv;                                                   
    } 
    rv = inst->QueryInterface(aIID, aResult);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
    }                                                                
    NS_RELEASE(inst);             /* get rid of extra refcnt */      
    return rv;              
}
#endif

static NS_IMETHODIMP                 
CreateNewMultiMixedConvFactory(nsISupports* aOuter, REFNSIID aIID, void **aResult) 
{
    if (!aResult) {                                                  
        return NS_ERROR_INVALID_POINTER;                             
    }
    if (aOuter) {                                                    
        *aResult = nsnull;                                           
        return NS_ERROR_NO_AGGREGATION;                              
    }   
    nsMultiMixedConv* inst = nsnull;
    nsresult rv = NS_NewMultiMixedConv(&inst);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
        return rv;                                                   
    } 
    rv = inst->QueryInterface(aIID, aResult);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
    }                                                                
    NS_RELEASE(inst);             /* get rid of extra refcnt */      
    return rv;              
}

static NS_IMETHODIMP                 
CreateNewTXTToHTMLConvFactory(nsISupports* aOuter, REFNSIID aIID, void **aResult) 
{
    if (!aResult) {                                                  
        return NS_ERROR_INVALID_POINTER;                             
    }
    if (aOuter) {                                                    
        *aResult = nsnull;                                           
        return NS_ERROR_NO_AGGREGATION;                              
    }   
    mozTXTToHTMLConv* inst = nsnull;
    nsresult rv = MOZ_NewTXTToHTMLConv(&inst);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
        return rv;                                                   
    } 
    rv = inst->QueryInterface(aIID, aResult);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
    }                                                                
    NS_RELEASE(inst);             /* get rid of extra refcnt */      
    return rv;              
}

static NS_IMETHODIMP                 
CreateNewHTTPCompressConvFactory (nsISupports* aOuter, REFNSIID aIID, void **aResult) 
{
    if (!aResult) {                                                  
        return NS_ERROR_INVALID_POINTER;                             
    }
    if (aOuter) {                                                    
        *aResult = nsnull;                                           
        return NS_ERROR_NO_AGGREGATION;                              
    }   
    nsHTTPCompressConv* inst = nsnull;
    nsresult rv = NS_NewHTTPCompressConv (&inst);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
        return rv;                                                   
    } 
    rv = inst->QueryInterface(aIID, aResult);
    if (NS_FAILED(rv)) {                                             
        *aResult = nsnull;                                           
    }                                                                
    NS_RELEASE(inst);             /* get rid of extra refcnt */      
    return rv;              
}

static NS_IMETHODIMP
CreateNewUnknownDecoderFactory(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  nsresult rv;

  if (!aResult) {
    return NS_ERROR_NULL_POINTER;
  }
  *aResult = nsnull;

  if (aOuter) {
    return NS_ERROR_NO_AGGREGATION;
  }

  nsUnknownDecoder *inst;
  
  inst = new nsUnknownDecoder();
  if (!inst) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  NS_ADDREF(inst);
  rv = inst->QueryInterface(aIID, aResult);
  NS_RELEASE(inst);

  return rv;
}

static NS_IMETHODIMP
CreateNewBinaryDetectorFactory(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  nsresult rv;

  if (!aResult) {
    return NS_ERROR_NULL_POINTER;
  }
  *aResult = nsnull;

  if (aOuter) {
    return NS_ERROR_NO_AGGREGATION;
  }

  nsBinaryDetector* inst = new nsBinaryDetector();
  if (!inst) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  NS_ADDREF(inst);
  rv = inst->QueryInterface(aIID, aResult);
  NS_RELEASE(inst);

  return rv;
}

static NS_IMETHODIMP
CreateNewNSTXTToHTMLConvFactory(nsISupports *aOuter, REFNSIID aIID, void **aResult)
{
  nsresult rv;

  if (!aResult) {
    return NS_ERROR_NULL_POINTER;
  }
  *aResult = nsnull;

  if (aOuter) {
    return NS_ERROR_NO_AGGREGATION;
  }

  nsTXTToHTMLConv *inst;
  
  inst = new nsTXTToHTMLConv();
  if (!inst) return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(inst);
  rv = inst->Init();
  if (NS_FAILED(rv)) {
    delete inst;
    return rv;
  }
  rv = inst->QueryInterface(aIID, aResult);
  NS_RELEASE(inst);

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// Module implementation for the net library

// Net module startup hook
PR_STATIC_CALLBACK(nsresult) nsNetStartup(nsIModule *neckoModule)
{
    gNetStrings = new nsNetStrings();
    return gNetStrings ? NS_OK : NS_ERROR_OUT_OF_MEMORY;
}


// Net module shutdown hook
static void PR_CALLBACK nsNetShutdown(nsIModule *neckoModule)
{
    // Release the url parser that the stdurl is holding.
    nsStandardURL::ShutdownGlobalObjects();

    // Release buffer cache
    NS_IF_RELEASE(nsIOService::gBufferCache);

    // Release global state used by the URL helper module.
    net_ShutdownURLHelper();
#ifdef XP_MACOSX
    net_ShutdownURLHelperOSX();
#endif

    // Release necko strings
    delete gNetStrings;
    gNetStrings = nsnull;
}

static const nsModuleComponentInfo gNetModuleInfo[] = {
    { NS_IOSERVICE_CLASSNAME,
      NS_IOSERVICE_CID,
      NS_IOSERVICE_CONTRACTID,
      nsIOServiceConstructor },
    { NS_IOSERVICE_CLASSNAME,
      NS_IOSERVICE_CID,
      NS_NETUTIL_CONTRACTID,
      nsIOServiceConstructor },
    { NS_STREAMTRANSPORTSERVICE_CLASSNAME,
      NS_STREAMTRANSPORTSERVICE_CID,
      NS_STREAMTRANSPORTSERVICE_CONTRACTID,
      nsStreamTransportServiceConstructor },
    { NS_SOCKETTRANSPORTSERVICE_CLASSNAME,
      NS_SOCKETTRANSPORTSERVICE_CID,
      NS_SOCKETTRANSPORTSERVICE_CONTRACTID,
      nsSocketTransportServiceConstructor },
    { NS_SERVERSOCKET_CLASSNAME,
      NS_SERVERSOCKET_CID,
      NS_SERVERSOCKET_CONTRACTID,
      nsServerSocketConstructor },
    { NS_SOCKETPROVIDERSERVICE_CLASSNAME,
      NS_SOCKETPROVIDERSERVICE_CID,
      NS_SOCKETPROVIDERSERVICE_CONTRACTID,
      nsSocketProviderService::Create },
    { NS_DNSSERVICE_CLASSNAME,
      NS_DNSSERVICE_CID,
      NS_DNSSERVICE_CONTRACTID,
      nsDNSServiceConstructor },
    { NS_IDNSERVICE_CLASSNAME,
      NS_IDNSERVICE_CID,
      NS_IDNSERVICE_CONTRACTID,
      nsIDNServiceConstructor },
    { NS_EFFECTIVETLDSERVICE_CLASSNAME,
      NS_EFFECTIVETLDSERVICE_CID,
      NS_EFFECTIVETLDSERVICE_CONTRACTID,
      nsEffectiveTLDServiceConstructor },
    { NS_SIMPLEURI_CLASSNAME,
      NS_SIMPLEURI_CID,
      NS_SIMPLEURI_CONTRACTID,
      nsSimpleURIConstructor },
    { "Simple Nested URI", 
      NS_SIMPLENESTEDURI_CID,
      nsnull,
      nsSimpleNestedURIConstructor },
    { NS_ASYNCSTREAMCOPIER_CLASSNAME,
      NS_ASYNCSTREAMCOPIER_CID,
      NS_ASYNCSTREAMCOPIER_CONTRACTID,
      nsAsyncStreamCopierConstructor },
    { NS_INPUTSTREAMPUMP_CLASSNAME,
      NS_INPUTSTREAMPUMP_CID,
      NS_INPUTSTREAMPUMP_CONTRACTID,
      nsInputStreamPumpConstructor },
    { NS_INPUTSTREAMCHANNEL_CLASSNAME,
      NS_INPUTSTREAMCHANNEL_CID,
      NS_INPUTSTREAMCHANNEL_CONTRACTID,
      nsInputStreamChannelConstructor },
    { NS_STREAMLOADER_CLASSNAME, 
      NS_STREAMLOADER_CID,
      NS_STREAMLOADER_CONTRACTID,
      nsStreamLoader::Create },
    { NS_UNICHARSTREAMLOADER_CLASSNAME, 
      NS_UNICHARSTREAMLOADER_CID,
      NS_UNICHARSTREAMLOADER_CONTRACTID,
      nsUnicharStreamLoader::Create },
    { NS_DOWNLOADER_CLASSNAME,
      NS_DOWNLOADER_CID,
      NS_DOWNLOADER_CONTRACTID,
      nsDownloaderConstructor },
    { NS_SYNCSTREAMLISTENER_CLASSNAME,
      NS_SYNCSTREAMLISTENER_CID,
      NS_SYNCSTREAMLISTENER_CONTRACTID,
      nsSyncStreamListenerConstructor },
    { NS_REQUESTOBSERVERPROXY_CLASSNAME,
      NS_REQUESTOBSERVERPROXY_CID,
      NS_REQUESTOBSERVERPROXY_CONTRACTID,
      nsRequestObserverProxyConstructor },
    { NS_SIMPLESTREAMLISTENER_CLASSNAME,
      NS_SIMPLESTREAMLISTENER_CID,
      NS_SIMPLESTREAMLISTENER_CONTRACTID,
      nsSimpleStreamListenerConstructor },
    { NS_STREAMLISTENERTEE_CLASSNAME,
      NS_STREAMLISTENERTEE_CID,
      NS_STREAMLISTENERTEE_CONTRACTID,
      nsStreamListenerTeeConstructor },
    { NS_LOADGROUP_CLASSNAME,
      NS_LOADGROUP_CID,
      NS_LOADGROUP_CONTRACTID,
      nsLoadGroupConstructor },
    { NS_LOCALFILEINPUTSTREAM_CLASSNAME, 
      NS_LOCALFILEINPUTSTREAM_CID,
      NS_LOCALFILEINPUTSTREAM_CONTRACTID,
      nsFileInputStream::Create },
    { NS_LOCALFILEOUTPUTSTREAM_CLASSNAME, 
      NS_LOCALFILEOUTPUTSTREAM_CID,
      NS_LOCALFILEOUTPUTSTREAM_CONTRACTID,
      nsFileOutputStream::Create },
    { NS_SAFELOCALFILEOUTPUTSTREAM_CLASSNAME,
      NS_SAFELOCALFILEOUTPUTSTREAM_CID,
      NS_SAFELOCALFILEOUTPUTSTREAM_CONTRACTID,
      nsSafeFileOutputStreamConstructor
    },
    
    { "URIChecker",
      NS_URICHECKER_CID,
      NS_URICHECKER_CONTRACT_ID,
      nsURICheckerConstructor
    },

    { NS_INCREMENTALDOWNLOAD_CLASSNAME,
      NS_INCREMENTALDOWNLOAD_CID,
      NS_INCREMENTALDOWNLOAD_CONTRACTID,
      net_NewIncrementalDownload
    },

    // The register functions for the built-in 
    // parsers just need to be called once.
    { NS_STDURLPARSER_CLASSNAME,
      NS_STDURLPARSER_CID,
      NS_STDURLPARSER_CONTRACTID,
      nsStdURLParserConstructor},
    { NS_NOAUTHURLPARSER_CLASSNAME,
      NS_NOAUTHURLPARSER_CID,
      NS_NOAUTHURLPARSER_CONTRACTID,
      nsNoAuthURLParserConstructor },
    { NS_AUTHURLPARSER_CLASSNAME,
      NS_AUTHURLPARSER_CID,
      NS_AUTHURLPARSER_CONTRACTID,
      nsAuthURLParserConstructor },

    { NS_STANDARDURL_CLASSNAME,
      NS_STANDARDURL_CID,
      NS_STANDARDURL_CONTRACTID,
      nsStandardURLConstructor },

    { NS_BUFFEREDINPUTSTREAM_CLASSNAME, 
      NS_BUFFEREDINPUTSTREAM_CID,
      NS_BUFFEREDINPUTSTREAM_CONTRACTID,
      nsBufferedInputStream::Create },
    { NS_BUFFEREDOUTPUTSTREAM_CLASSNAME, 
      NS_BUFFEREDOUTPUTSTREAM_CID,
      NS_BUFFEREDOUTPUTSTREAM_CONTRACTID,
      nsBufferedOutputStream::Create },
    { NS_MIMEINPUTSTREAM_CLASSNAME,
      NS_MIMEINPUTSTREAM_CID,
      NS_MIMEINPUTSTREAM_CONTRACTID,
      nsMIMEInputStreamConstructor },
    { NS_PROTOCOLPROXYSERVICE_CLASSNAME,
      NS_PROTOCOLPROXYSERVICE_CID,
      NS_PROTOCOLPROXYSERVICE_CONTRACTID,
      nsProtocolProxyServiceConstructor,
      nsnull, nsnull, nsnull,
      NS_CI_INTERFACE_GETTER_NAME(nsProtocolProxyService),
      nsnull,
      &NS_CLASSINFO_NAME(nsProtocolProxyService),
      nsIClassInfo::SINGLETON },

    // from netwerk/streamconv:

    // this converter is "always" built.
    // HACK-ALERT, register *all* converters
    // in this converter's component manager 
    // registration. just piggy backing here until
    // you can add registration functions to
    // the generic module macro.
    { "Stream Converter Service", 
      NS_STREAMCONVERTERSERVICE_CID,
      NS_STREAMCONVERTERSERVICE_CONTRACTID,
      CreateNewStreamConvServiceFactory,
      RegisterStreamConverters,   // registers *all* converters
      UnregisterStreamConverters  // unregisters *all* converters
    },
    
#ifdef BUILD_APPLEFILE_DECODER
    { NS_APPLEFILEDECODER_CLASSNAME, 
      NS_APPLEFILEDECODER_CID,
      NS_IAPPLEFILEDECODER_CONTRACTID, 
      nsAppleFileDecoderConstructor
    },
#endif

#ifdef NECKO_PROTOCOL_ftp
    // from netwerk/streamconv/converters:
    { "FTPDirListingConverter", 
      NS_FTPDIRLISTINGCONVERTER_CID, 
      NS_ISTREAMCONVERTER_KEY FTP_TO_INDEX, 
      CreateNewFTPDirListingConv
    },
#endif

#ifdef NECKO_PROTOCOL_gopher
    { "GopherDirListingConverter",
      NS_GOPHERDIRLISTINGCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY GOPHER_TO_INDEX,
      nsGopherDirListingConvConstructor
    },
#endif

    { "Indexed to HTML Converter", 
      NS_NSINDEXEDTOHTMLCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY INDEX_TO_HTML, 
      nsIndexedToHTML::Create
    },

    { "Directory Index Parser",
      NS_DIRINDEXPARSER_CID,
      NS_DIRINDEXPARSER_CONTRACTID,
      nsDirIndexParserConstructor
    },

    { "MultiMixedConverter", 
      NS_MULTIMIXEDCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY MULTI_MIXED_X, 
      CreateNewMultiMixedConvFactory
    },

    { "MultiMixedByteRange", 
      NS_MULTIMIXEDCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY MULTI_BYTERANGES, 
      CreateNewMultiMixedConvFactory
    },

    { "MultiMixedConverter2",
      NS_MULTIMIXEDCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY MULTI_MIXED,
      CreateNewMultiMixedConvFactory
    },

    { "Unknown Content-Type Decoder",
      NS_UNKNOWNDECODER_CID,
      NS_ISTREAMCONVERTER_KEY UNKNOWN_CONTENT,
      CreateNewUnknownDecoderFactory
    },

    { "Unknown Content-Type Decoder",
      NS_UNKNOWNDECODER_CID,
      NS_GENERIC_CONTENT_SNIFFER,
      CreateNewUnknownDecoderFactory
    },

    { "Binary Detector",
      NS_BINARYDETECTOR_CID,
      NS_BINARYDETECTOR_CONTRACTID,
      CreateNewBinaryDetectorFactory,
      nsBinaryDetector::Register
    },

    { "HttpCompressConverter", 
      NS_HTTPCOMPRESSCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY GZIP_TO_UNCOMPRESSED,
      CreateNewHTTPCompressConvFactory
    },

    { "HttpCompressConverter", 
      NS_HTTPCOMPRESSCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY XGZIP_TO_UNCOMPRESSED,
      CreateNewHTTPCompressConvFactory
    },
    { "HttpCompressConverter", 
      NS_HTTPCOMPRESSCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY COMPRESS_TO_UNCOMPRESSED,
      CreateNewHTTPCompressConvFactory
    },
    { "HttpCompressConverter", 
      NS_HTTPCOMPRESSCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY XCOMPRESS_TO_UNCOMPRESSED,
      CreateNewHTTPCompressConvFactory
    },
    { "HttpCompressConverter", 
      NS_HTTPCOMPRESSCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY DEFLATE_TO_UNCOMPRESSED,
      CreateNewHTTPCompressConvFactory
    },
    { "NSTXTToHTMLConverter",
      NS_NSTXTTOHTMLCONVERTER_CID,
      NS_ISTREAMCONVERTER_KEY PLAIN_TO_HTML,
      CreateNewNSTXTToHTMLConvFactory
    },
#ifdef BUILD_BINHEX_DECODER
    { "nsBinHexConverter", NS_BINHEXDECODER_CID,
      NS_ISTREAMCONVERTER_KEY BINHEX_TO_WILD,
      nsBinHexDecoderConstructor
    },
#endif
    // This is not a real stream converter, it's just
    // registering its cid factory here.
    { "HACK-TXTToHTMLConverter", 
      MOZITXTTOHTMLCONV_CID,
      MOZ_TXTTOHTMLCONV_CONTRACTID, 
      CreateNewTXTToHTMLConvFactory
    },

    { "Directory Index",
      NS_DIRINDEX_CID,
      "@mozilla.org/dirIndex;1",
      nsDirIndexConstructor
    },

    // from netwerk/mime:
    { "mime header param", 
      NS_MIMEHEADERPARAM_CID,
      NS_MIMEHEADERPARAM_CONTRACTID,
      nsMIMEHeaderParamImplConstructor
    },

#ifdef NECKO_PROTOCOL_file
    // from netwerk/protocol/file:
    { NS_FILEPROTOCOLHANDLER_CLASSNAME,
      NS_FILEPROTOCOLHANDLER_CID,  
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "file", 
      nsFileProtocolHandlerConstructor
    },
#endif

#ifdef NECKO_PROTOCOL_http
    { "HTTP Handler",
      NS_HTTPPROTOCOLHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "http",
      nsHttpHandlerConstructor },

    { "HTTPS Handler",
      NS_HTTPSPROTOCOLHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "https",
      nsHttpsHandlerConstructor },

    { "HTTP Basic Auth Encoder",
      NS_HTTPBASICAUTH_CID,
      NS_HTTP_AUTHENTICATOR_CONTRACTID_PREFIX "basic",
      nsHttpBasicAuthConstructor },

    { "HTTP Digest Auth Encoder",
      NS_HTTPDIGESTAUTH_CID,
      NS_HTTP_AUTHENTICATOR_CONTRACTID_PREFIX "digest",
      nsHttpDigestAuthConstructor },

    { "HTTP NTLM Auth Encoder",
      NS_HTTPNTLMAUTH_CID,
      NS_HTTP_AUTHENTICATOR_CONTRACTID_PREFIX "ntlm",
      nsHttpNTLMAuthConstructor },

    { NS_HTTPAUTHMANAGER_CLASSNAME,
      NS_HTTPAUTHMANAGER_CID,
      NS_HTTPAUTHMANAGER_CONTRACTID,
      nsHttpAuthManagerConstructor },
#endif // !NECKO_PROTOCOL_http
      
#ifdef NECKO_PROTOCOL_ftp
    // from netwerk/protocol/ftp:
    { NS_FTPPROTOCOLHANDLER_CLASSNAME,
      NS_FTPPROTOCOLHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "ftp",
      nsFtpProtocolHandlerConstructor
    },
#endif

#ifdef NECKO_PROTOCOL_res
    // from netwerk/protocol/res:
    { NS_RESPROTOCOLHANDLER_CLASSNAME,
      NS_RESPROTOCOLHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "resource",
      nsResProtocolHandlerConstructor
    },
    { NS_RESURL_CLASSNAME, // needed only for fastload
      NS_RESURL_CID,
      nsnull,
      nsResURLConstructor
    },
#endif

    // from netwerk/protocol/about (about:blank is mandatory):
    { NS_ABOUTPROTOCOLHANDLER_CLASSNAME, 
      NS_ABOUTPROTOCOLHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "about", 
      nsAboutProtocolHandlerConstructor
    },
    { NS_SAFEABOUTPROTOCOLHANDLER_CLASSNAME,
      NS_SAFEABOUTPROTOCOLHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "moz-safe-about", 
      nsSafeAboutProtocolHandlerConstructor
    },
    { "about:blank", 
      NS_ABOUT_BLANK_MODULE_CID,
      NS_ABOUT_MODULE_CONTRACTID_PREFIX "blank", 
      nsAboutBlank::Create
    },
#ifdef NECKO_PROTOCOL_about
#ifdef NS_BUILD_REFCNT_LOGGING
    { "about:bloat", 
      NS_ABOUT_BLOAT_MODULE_CID,
      NS_ABOUT_MODULE_CONTRACTID_PREFIX "bloat", 
      nsAboutBloat::Create
    },
#endif
    { "about:cache", 
      NS_ABOUT_CACHE_MODULE_CID,
      NS_ABOUT_MODULE_CONTRACTID_PREFIX "cache", 
      nsAboutCache::Create
    },
    { "about:cache-entry",
      NS_ABOUT_CACHE_ENTRY_MODULE_CID,
      NS_ABOUT_MODULE_CONTRACTID_PREFIX "cache-entry",
      nsAboutCacheEntryConstructor
    },
#endif

    {  "nsSOCKSSocketProvider",
       NS_SOCKSSOCKETPROVIDER_CID,
       NS_NETWORK_SOCKET_CONTRACTID_PREFIX "socks",
       nsSOCKSSocketProvider::CreateV5
    },

    {  "nsSOCKS4SocketProvider",
       NS_SOCKS4SOCKETPROVIDER_CID,
       NS_NETWORK_SOCKET_CONTRACTID_PREFIX "socks4",
       nsSOCKSSocketProvider::CreateV4
    },

    {  "nsUDPSocketProvider",
       NS_UDPSOCKETPROVIDER_CID,
       NS_NETWORK_SOCKET_CONTRACTID_PREFIX "udp",
       nsUDPSocketProviderConstructor
    },

    {  NS_CACHESERVICE_CLASSNAME,
       NS_CACHESERVICE_CID,
       NS_CACHESERVICE_CONTRACTID,
       nsCacheService::Create
    },

#ifdef NECKO_COOKIES
    { NS_COOKIEMANAGER_CLASSNAME,
      NS_COOKIEMANAGER_CID,
      NS_COOKIEMANAGER_CONTRACTID,
      nsCookieServiceConstructor
    },

    { NS_COOKIESERVICE_CLASSNAME,
      NS_COOKIESERVICE_CID,
      NS_COOKIESERVICE_CONTRACTID,
      nsCookieServiceConstructor
    },
#endif

#ifdef NECKO_PROTOCOL_gopher
    //gopher:
    { "The Gopher Protocol Handler", 
      NS_GOPHERHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "gopher",
      nsGopherHandlerConstructor
    },
#endif

#ifdef NECKO_PROTOCOL_data
    // from netwerk/protocol/data:
    { "Data Protocol Handler", 
      NS_DATAPROTOCOLHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "data", 
      nsDataHandler::Create},
#endif

#ifdef NECKO_PROTOCOL_viewsource
    // from netwerk/protocol/viewsource:
    { "The ViewSource Protocol Handler", 
      NS_VIEWSOURCEHANDLER_CID,
      NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "view-source",
      nsViewSourceHandlerConstructor
    },
#endif

#if defined(XP_WIN) && !defined(WINCE)
    { NS_NETWORK_LINK_SERVICE_CLASSNAME,
      NS_NETWORK_LINK_SERVICE_CID,
      NS_NETWORK_LINK_SERVICE_CONTRACTID,
      nsNotifyAddrListenerConstructor
    },
#endif
};

NS_IMPL_NSGETMODULE_WITH_CTOR_DTOR(necko, gNetModuleInfo,
                                   nsNetStartup, nsNetShutdown)
