/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim:expandtab:shiftwidth=2:tabstop=2:cin:
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the Mozilla browser.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications, Inc.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Scott MacGregor <mscott@netscape.com>
 *   Bill Law <law@netscape.com>
 *   Christian Biesinger <cbiesinger@web.de>
 *   Dan Mosedale <dmose@mozilla.org>
 *   Myk Melez <myk@mozilla.org>
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

#include "nsExternalHelperAppService.h"
#include "nsCExternalHandlerService.h"
#include "nsIURI.h"
#include "nsIURL.h"
#include "nsIFile.h"
#include "nsIFileURL.h"
#include "nsIChannel.h"
#include "nsIDirectoryService.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsICategoryManager.h"
#include "nsXPIDLString.h"
#include "nsUnicharUtils.h"
#include "nsIStringEnumerator.h"
#include "nsMemory.h"
#include "nsIStreamListener.h"
#include "nsIMIMEService.h"
#include "nsILoadGroup.h"
#include "nsIWebProgressListener.h"
#include "nsITransfer.h"
#include "nsReadableUtils.h"
#include "nsIRequest.h"
#include "nsDirectoryServiceDefs.h"
#include "nsIInterfaceRequestor.h"
#include "nsThreadUtils.h"
#include "nsAutoPtr.h"
#include "nsIMutableArray.h"

// used to access our datastore of user-configured helper applications
#include "nsIHandlerService.h"
#include "nsIMIMEInfo.h"
#include "nsIRefreshURI.h" // XXX needed to redirect according to Refresh: URI
#include "nsIDocumentLoader.h" // XXX needed to get orig. channel and assoc. refresh uri
#include "nsIHelperAppLauncherDialog.h"
#include "nsIContentDispatchChooser.h"
#include "nsNetUtil.h"
#include "nsIIOService.h"
#include "nsNetCID.h"
#include "nsChannelProperties.h"

#include "nsMimeTypes.h"
// used for header disposition information.
#include "nsIHttpChannel.h"
#include "nsIEncodedChannel.h"
#include "nsIMultiPartChannel.h"
#include "nsIFileChannel.h"
#include "nsIObserverService.h" // so we can be a profile change observer
#include "nsIPropertyBag2.h" // for the 64-bit content length

#ifdef XP_MACOSX
#include "nsILocalFileMac.h"
#include "nsIInternetConfigService.h"
#include "nsIAppleFileDecoder.h"
#elif defined(XP_OS2)
#include "nsILocalFileOS2.h"
#endif

#include "nsIPluginHost.h" // XXX needed for ext->type mapping (bug 233289)
#include "nsEscape.h"

#include "nsIStringBundle.h" // XXX needed to localize error msgs
#include "nsIPrompt.h"

#include "nsITextToSubURI.h" // to unescape the filename
#include "nsIMIMEHeaderParam.h"

#include "nsIPrefService.h"
#include "nsIWindowWatcher.h"

#include "nsIDownloadHistory.h" // to mark downloads as visited
#include "nsDocShellCID.h"

#include "nsIDOMWindow.h"
#include "nsIDOMWindowInternal.h"
#include "nsIDocShell.h"

#include "nsCRT.h"

#include "nsLocalHandlerApp.h"

#include "nsIRandomGenerator.h"
#include "plbase64.h"
#include "prmem.h"

// Buffer file writes in 32kb chunks
#define BUFFERED_OUTPUT_SIZE (1024 * 32)

#ifdef PR_LOGGING
PRLogModuleInfo* nsExternalHelperAppService::mLog = nsnull;
#endif

// Using level 3 here because the OSHelperAppServices use a log level
// of PR_LOG_DEBUG (4), and we want less detailed output here
// Using 3 instead of PR_LOG_WARN because we don't output warnings
#define LOG(args) PR_LOG(mLog, 3, args)
#define LOG_ENABLED() PR_LOG_TEST(mLog, 3)

static const char NEVER_ASK_PREF_BRANCH[] = "browser.helperApps.neverAsk.";
static const char NEVER_ASK_FOR_SAVE_TO_DISK_PREF[] = "saveToDisk";
static const char NEVER_ASK_FOR_OPEN_FILE_PREF[]    = "openFile";

static NS_DEFINE_CID(kPluginManagerCID, NS_PLUGINMANAGER_CID);

/**
 * Contains a pointer to the helper app service, set in its constructor
 */
nsExternalHelperAppService* gExtProtSvc;

// Helper functions for Content-Disposition headers

/**
 * Given a URI fragment, unescape it
 * @param aFragment The string to unescape
 * @param aURI The URI from which this fragment is taken. Only its character set
 *             will be used.
 * @param aResult [out] Unescaped string.
 */
static nsresult UnescapeFragment(const nsACString& aFragment, nsIURI* aURI,
                                 nsAString& aResult)
{
  // First, we need a charset
  nsCAutoString originCharset;
  nsresult rv = aURI->GetOriginCharset(originCharset);
  NS_ENSURE_SUCCESS(rv, rv);

  // Now, we need the unescaper
  nsCOMPtr<nsITextToSubURI> textToSubURI = do_GetService(NS_ITEXTTOSUBURI_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  return textToSubURI->UnEscapeURIForUI(originCharset, aFragment, aResult);
}

/**
 * UTF-8 version of UnescapeFragment.
 * @param aFragment The string to unescape
 * @param aURI The URI from which this fragment is taken. Only its character set
 *             will be used.
 * @param aResult [out] Unescaped string, UTF-8 encoded.
 * @note It is safe to pass the same string for aFragment and aResult.
 * @note When this function fails, aResult will not be modified.
 */
static nsresult UnescapeFragment(const nsACString& aFragment, nsIURI* aURI,
                                 nsACString& aResult)
{
  nsAutoString result;
  nsresult rv = UnescapeFragment(aFragment, aURI, result);
  if (NS_SUCCEEDED(rv))
    CopyUTF16toUTF8(result, aResult);
  return rv;
}

/** Gets the content-disposition header from a channel, using nsIHttpChannel
 * or nsIMultipartChannel if available
 * @param aChannel The channel to extract the disposition header from
 * @param aDisposition Reference to a string where the header is to be stored
 */
static void ExtractDisposition(nsIChannel* aChannel, nsACString& aDisposition)
{
  aDisposition.Truncate();
  // First see whether this is an http channel
  nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(aChannel));
  if (httpChannel) 
  {
    httpChannel->GetResponseHeader(NS_LITERAL_CSTRING("content-disposition"), aDisposition);
  }
  if (aDisposition.IsEmpty())
  {
    nsCOMPtr<nsIMultiPartChannel> multipartChannel(do_QueryInterface(aChannel));
    if (multipartChannel)
    {
      multipartChannel->GetContentDisposition(aDisposition);
    }
  }

}

/** Extracts the filename out of a content-disposition header
 * @param aFilename [out] The filename. Can be empty on error.
 * @param aDisposition Value of a Content-Disposition header
 * @param aURI Optional. Will be used to get a fallback charset for the
 *        filename, if it is QI'able to nsIURL
 * @param aMIMEHeaderParam Optional. Pointer to a nsIMIMEHeaderParam class, so
 *        that it doesn't need to be fetched by this function.
 */
static void GetFilenameFromDisposition(nsAString& aFilename,
                                       const nsACString& aDisposition,
                                       nsIURI* aURI = nsnull,
                                       nsIMIMEHeaderParam* aMIMEHeaderParam = nsnull)
{
  aFilename.Truncate();
  nsCOMPtr<nsIMIMEHeaderParam> mimehdrpar(aMIMEHeaderParam);
  if (!mimehdrpar) {
    mimehdrpar = do_GetService(NS_MIMEHEADERPARAM_CONTRACTID);
    if (!mimehdrpar)
      return;
  }

  nsCOMPtr<nsIURL> url = do_QueryInterface(aURI);

  nsCAutoString fallbackCharset;
  if (url)
    url->GetOriginCharset(fallbackCharset);
  // Get the value of 'filename' parameter
  nsresult rv = mimehdrpar->GetParameter(aDisposition, "filename", fallbackCharset, 
                                         PR_TRUE, nsnull, aFilename);
  if (NS_FAILED(rv) || aFilename.IsEmpty())
    // Try 'name' parameter, instead.
    rv = mimehdrpar->GetParameter(aDisposition, "name", fallbackCharset, PR_TRUE, 
                                  nsnull, aFilename);
}

/**
 * Given a channel, returns the filename and extension the channel has.
 * This uses the URL and other sources (nsIMultiPartChannel).
 * Also gives back whether the channel requested external handling (i.e.
 * whether Content-Disposition: attachment was sent)
 * @param aChannel The channel to extract the filename/extension from
 * @param aFileName [out] Reference to the string where the filename should be
 *        stored. Empty if it could not be retrieved.
 *        WARNING - this filename may contain characters which the OS does not
 *        allow as part of filenames!
 * @param aExtension [out] Reference to the string where the extension should
 *        be stored. Empty if it could not be retrieved. Stored in UTF-8.
 * @param aAllowURLExtension (optional) Get the extension from the URL if no
 *        Content-Disposition header is present. Default is true.
 * @retval true The server sent Content-Disposition:attachment or equivalent
 * @retval false Content-Disposition: inline or no content-disposition header
 *         was sent.
 */
static PRBool GetFilenameAndExtensionFromChannel(nsIChannel* aChannel,
                                                 nsString& aFileName,
                                                 nsCString& aExtension,
                                                 PRBool aAllowURLExtension = PR_TRUE)
{
  aExtension.Truncate();
  /*
   * If the channel is an http or part of a multipart channel and we
   * have a content disposition header set, then use the file name
   * suggested there as the preferred file name to SUGGEST to the
   * user.  we shouldn't actually use that without their
   * permission... otherwise just use our temp file
   */
  nsCAutoString disp;
  ExtractDisposition(aChannel, disp);
  PRBool handleExternally = PR_FALSE;
  nsCOMPtr<nsIURI> uri;
  nsresult rv;
  aChannel->GetURI(getter_AddRefs(uri));
  // content-disposition: has format:
  // disposition-type < ; name=value >* < ; filename=value > < ; name=value >*
  if (!disp.IsEmpty()) 
  {
    nsCOMPtr<nsIMIMEHeaderParam> mimehdrpar = do_GetService(NS_MIMEHEADERPARAM_CONTRACTID, &rv);
    if (NS_FAILED(rv))
      return PR_FALSE;

    nsCAutoString fallbackCharset;
    uri->GetOriginCharset(fallbackCharset);
    // Get the disposition type
    nsAutoString dispToken;
    rv = mimehdrpar->GetParameter(disp, "", fallbackCharset, PR_TRUE, 
                                  nsnull, dispToken);
    // RFC 2183, section 2.8 says that an unknown disposition
    // value should be treated as "attachment"
    // XXXbz this code is duplicated in nsDocumentOpenInfo::DispatchContent.
    // Factor it out!  Maybe store it in the nsDocumentOpenInfo?
    if (NS_FAILED(rv) || 
        (// Some broken sites just send
         // Content-Disposition: ; filename="file"
         // screen those out here.
         !dispToken.IsEmpty() &&
         !dispToken.LowerCaseEqualsLiteral("inline") &&
        // Broken sites just send
        // Content-Disposition: filename="file"
        // without a disposition token... screen those out.
        !dispToken.EqualsIgnoreCase("filename", 8)) &&
        // Also in use is Content-Disposition: name="file"
        !dispToken.EqualsIgnoreCase("name", 4)) 
    {
      // We have a content-disposition of "attachment" or unknown
      handleExternally = PR_TRUE;
    }

    // We may not have a disposition type listed; some servers suck.
    // But they could have listed a filename anyway.
    GetFilenameFromDisposition(aFileName, disp, uri, mimehdrpar);

  } // we had a disp header 

  // If the disposition header didn't work, try the filename from nsIURL
  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));
  if (url && aFileName.IsEmpty())
  {
    if (aAllowURLExtension) {
      url->GetFileExtension(aExtension);
      UnescapeFragment(aExtension, url, aExtension);

      // Windows ignores terminating dots. So we have to as well, so
      // that our security checks do "the right thing"
      // In case the aExtension consisted only of the dot, the code below will
      // extract an aExtension from the filename
      aExtension.Trim(".", PR_FALSE);
    }

    // try to extract the file name from the url and use that as a first pass as the
    // leaf name of our temp file...
    nsCAutoString leafName;
    url->GetFileName(leafName);
    if (!leafName.IsEmpty())
    {
      rv = UnescapeFragment(leafName, url, aFileName);
      if (NS_FAILED(rv))
      {
        CopyUTF8toUTF16(leafName, aFileName); // use escaped name
      }
    }
  }

  // Extract Extension, if we have a filename; otherwise,
  // truncate the string
  if (aExtension.IsEmpty()) {
    if (!aFileName.IsEmpty())
    {
      // Windows ignores terminating dots. So we have to as well, so
      // that our security checks do "the right thing"
      aFileName.Trim(".", PR_FALSE);

      // XXX RFindCharInReadable!!
      nsAutoString fileNameStr(aFileName);
      PRInt32 idx = fileNameStr.RFindChar(PRUnichar('.'));
      if (idx != kNotFound)
        CopyUTF16toUTF8(StringTail(fileNameStr, fileNameStr.Length() - idx - 1), aExtension);
    }
  }


  return handleExternally;
}

/**
 * Structure for storing extension->type mappings.
 * @see defaultMimeEntries
 */
struct nsDefaultMimeTypeEntry {
  const char* mMimeType;
  const char* mFileExtension;
};

/**
 * Default extension->mimetype mappings. These are not overridable.
 * If you add types here, make sure they are lowercase, or you'll regret it.
 */
static nsDefaultMimeTypeEntry defaultMimeEntries [] = 
{
  // The following are those extensions that we're asked about during startup,
  // sorted by order used
  { IMAGE_GIF, "gif" },
  { TEXT_XML, "xml" },
  { APPLICATION_RDF, "rdf" },
  { TEXT_XUL, "xul" },
  { IMAGE_PNG, "png" },
  // -- end extensions used during startup
  { TEXT_CSS, "css" },
  { IMAGE_JPG, "jpeg" },
  { IMAGE_JPG, "jpg" },
  { TEXT_HTML, "html" },
  { TEXT_HTML, "htm" },
  { APPLICATION_XPINSTALL, "xpi" },
  { "application/xhtml+xml", "xhtml" },
  { "application/xhtml+xml", "xht" },
  { "application/xhtml+xml", "sci" },
  { TEXT_PLAIN, "txt" }
};

/**
 * This is a small private struct used to help us initialize some
 * default mime types.
 */
struct nsExtraMimeTypeEntry {
  const char* mMimeType; 
  const char* mFileExtensions;
  const char* mDescription;
  PRUint32 mMactype;
  PRUint32 mMacCreator;
};

#ifdef XP_MACOSX
#define MAC_TYPE(x) x
#else
#define MAC_TYPE(x) 0
#endif

/**
 * This table lists all of the 'extra' content types that we can deduce from particular
 * file extensions.  These entries also ensure that we provide a good descriptive name
 * when we encounter files with these content types and/or extensions.  These can be
 * overridden by user helper app prefs.
 * If you add types here, make sure they are lowercase, or you'll regret it.
 */
static nsExtraMimeTypeEntry extraMimeEntries [] =
{
#if defined(VMS)
  { APPLICATION_OCTET_STREAM, "exe,com,bin,sav,bck,pcsi,dcx_axpexe,dcx_vaxexe,sfx_axpexe,sfx_vaxexe", "Binary File", 0, 0 },
#elif defined(XP_MACOSX) // don't define .bin on the mac...use internet config to look that up...
  { APPLICATION_OCTET_STREAM, "exe,com", "Binary File", 0, 0 },
#else
  { APPLICATION_OCTET_STREAM, "exe,com,bin", "Binary File", 0, 0 },
#endif
  { APPLICATION_GZIP2, "gz", "gzip", 0, 0 },
  { "application/x-arj", "arj", "ARJ file", 0,0 },
  { APPLICATION_XPINSTALL, "xpi", "XPInstall Install", MAC_TYPE('xpi*'), MAC_TYPE('MOSS') },
  { APPLICATION_POSTSCRIPT, "ps,eps,ai", "Postscript File", 0, 0 },
  { APPLICATION_JAVASCRIPT, "js", "Javascript Source File", MAC_TYPE('TEXT'), MAC_TYPE('ttxt') },
  { IMAGE_ART, "art", "ART Image", 0, 0 },
  { IMAGE_BMP, "bmp", "BMP Image", 0, 0 },
  { IMAGE_GIF, "gif", "GIF Image", 0,0 },
  { IMAGE_ICO, "ico,cur", "ICO Image", 0, 0 },
  { IMAGE_JPG, "jpeg,jpg,jfif,pjpeg,pjp", "JPEG Image", 0, 0 },
  { IMAGE_PNG, "png", "PNG Image", 0, 0 },
  { IMAGE_TIFF, "tiff,tif", "TIFF Image", 0, 0 },
  { IMAGE_XBM, "xbm", "XBM Image", 0, 0 },
  { "image/svg+xml", "svg", "Scalable Vector Graphics", MAC_TYPE('svg '), MAC_TYPE('ttxt') },
  { MESSAGE_RFC822, "eml", "RFC-822 data", MAC_TYPE('TEXT'), MAC_TYPE('MOSS') },
  { TEXT_PLAIN, "txt,text", "Text File", MAC_TYPE('TEXT'), MAC_TYPE('ttxt') },
  { TEXT_HTML, "html,htm,shtml,ehtml", "HyperText Markup Language", MAC_TYPE('TEXT'), MAC_TYPE('MOSS') },
  { "application/xhtml+xml", "xhtml,xht,sci", "Extensible HyperText Markup Language", MAC_TYPE('TEXT'), MAC_TYPE('ttxt') },
  { APPLICATION_RDF, "rdf", "Resource Description Framework", MAC_TYPE('TEXT'), MAC_TYPE('ttxt') },
  { TEXT_XUL, "xul", "XML-Based User Interface Language", MAC_TYPE('TEXT'), MAC_TYPE('ttxt') },
  { TEXT_XML, "xml,xsl,xbl", "Extensible Markup Language", MAC_TYPE('TEXT'), MAC_TYPE('ttxt') },
  { TEXT_CSS, "css", "Style Sheet", MAC_TYPE('TEXT'), MAC_TYPE('ttxt') },
};

#undef MAC_TYPE

/**
 * File extensions for which decoding should be disabled.
 * NOTE: These MUST be lower-case and ASCII.
 */
static nsDefaultMimeTypeEntry nonDecodableExtensions [] = {
  { APPLICATION_GZIP, "gz" }, 
  { APPLICATION_GZIP, "tgz" },
  { APPLICATION_ZIP, "zip" },
  { APPLICATION_COMPRESS, "z" },
  { APPLICATION_GZIP, "svgz" }
};

NS_IMPL_ISUPPORTS6(
  nsExternalHelperAppService,
  nsIExternalHelperAppService,
  nsPIExternalAppLauncher,
  nsIExternalProtocolService,
  nsIMIMEService,
  nsIObserver,
  nsISupportsWeakReference)

nsExternalHelperAppService::nsExternalHelperAppService()
{
  gExtProtSvc = this;
}
nsresult nsExternalHelperAppService::Init()
{
  // Add an observer for profile change
  nsresult rv = NS_OK;
  nsCOMPtr<nsIObserverService> obs = do_GetService("@mozilla.org/observer-service;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

#ifdef PR_LOGGING
  if (!mLog) {
    mLog = PR_NewLogModule("HelperAppService");
    if (!mLog)
      return NS_ERROR_OUT_OF_MEMORY;
  }
#endif

  return obs->AddObserver(this, "profile-before-change", PR_TRUE);
}

nsExternalHelperAppService::~nsExternalHelperAppService()
{
  gExtProtSvc = nsnull;
}

NS_IMETHODIMP nsExternalHelperAppService::DoContent(const nsACString& aMimeContentType,
                                                    nsIRequest *aRequest,
                                                    nsIInterfaceRequestor *aWindowContext,
                                                    PRBool aForceSave,
                                                    nsIStreamListener ** aStreamListener)
{
  nsAutoString fileName;
  nsCAutoString fileExtension;
  PRUint32 reason = nsIHelperAppLauncherDialog::REASON_CANTHANDLE;
  nsresult rv;

  // Get the file extension and name that we will need later
  nsCOMPtr<nsIChannel> channel = do_QueryInterface(aRequest);
  if (channel) {
    // Check if we have a POST request, in which case we don't want to use
    // the url's extension
    PRBool allowURLExt = PR_TRUE;
    nsCOMPtr<nsIHttpChannel> httpChan = do_QueryInterface(channel);
    if (httpChan) {
      nsCAutoString requestMethod;
      httpChan->GetRequestMethod(requestMethod);
      allowURLExt = !requestMethod.Equals("POST");
    }

    nsCOMPtr<nsIURI> uri;
    channel->GetURI(getter_AddRefs(uri));

    // Check if we had a query string - we don't want to check the URL
    // extension if a query is present in the URI
    // If we already know we don't want to check the URL extension, don't
    // bother checking the query
    if (uri && allowURLExt) {
      nsCOMPtr<nsIURL> url = do_QueryInterface(uri);

      if (url) {
        nsCAutoString query;

        // We only care about the query for HTTP and HTTPS URLs
        PRBool isHTTP, isHTTPS;
        rv = uri->SchemeIs("http", &isHTTP);
        if (NS_FAILED(rv))
          isHTTP = PR_FALSE;
        rv = uri->SchemeIs("https", &isHTTPS);
        if (NS_FAILED(rv))
          isHTTPS = PR_FALSE;

        if (isHTTP || isHTTPS)
          url->GetQuery(query);

        // Only get the extension if the query is empty; if it isn't, then the
        // extension likely belongs to a cgi script and isn't helpful
        allowURLExt = query.IsEmpty();
      }
    }
    // Extract name & extension
    PRBool isAttachment = GetFilenameAndExtensionFromChannel(channel, fileName,
                                                             fileExtension,
                                                             allowURLExt);
    LOG(("Found extension '%s' (filename is '%s', handling attachment: %i)",
         fileExtension.get(), NS_ConvertUTF16toUTF8(fileName).get(),
         isAttachment));
    if (isAttachment)
      reason = nsIHelperAppLauncherDialog::REASON_SERVERREQUEST;
  }

  LOG(("HelperAppService::DoContent: mime '%s', extension '%s'\n",
       PromiseFlatCString(aMimeContentType).get(), fileExtension.get()));

  // we get the mime service here even though we're the default implementation of it,
  // so it's possible to override only the mime service and not need to reimplement the
  // whole external helper app service itself
  nsCOMPtr<nsIMIMEService> mimeSvc(do_GetService(NS_MIMESERVICE_CONTRACTID));
  NS_ENSURE_TRUE(mimeSvc, NS_ERROR_FAILURE);

  // Try to find a mime object by looking at the mime type/extension
  nsCOMPtr<nsIMIMEInfo> mimeInfo;
  if (aMimeContentType.Equals(APPLICATION_GUESS_FROM_EXT, nsCaseInsensitiveCStringComparator())) {
    nsCAutoString mimeType;
    if (!fileExtension.IsEmpty()) {
      mimeSvc->GetFromTypeAndExtension(EmptyCString(), fileExtension, getter_AddRefs(mimeInfo));
      if (mimeInfo) {
        mimeInfo->GetMIMEType(mimeType);

        LOG(("OS-Provided mime type '%s' for extension '%s'\n", 
             mimeType.get(), fileExtension.get()));
      }
    }

    if (fileExtension.IsEmpty() || mimeType.IsEmpty()) {
      // Extension lookup gave us no useful match
      mimeSvc->GetFromTypeAndExtension(NS_LITERAL_CSTRING(APPLICATION_OCTET_STREAM), fileExtension,
                                       getter_AddRefs(mimeInfo));
      mimeType.AssignLiteral(APPLICATION_OCTET_STREAM);
    }
    if (channel)
      channel->SetContentType(mimeType);
    // Don't overwrite SERVERREQUEST
    if (reason == nsIHelperAppLauncherDialog::REASON_CANTHANDLE)
      reason = nsIHelperAppLauncherDialog::REASON_TYPESNIFFED;
  } 
  else {
    mimeSvc->GetFromTypeAndExtension(aMimeContentType, fileExtension,
                                     getter_AddRefs(mimeInfo));
  } 
  LOG(("Type/Ext lookup found 0x%p\n", mimeInfo.get()));

  // No mimeinfo -> we can't continue. probably OOM.
  if (!mimeInfo)
    return NS_ERROR_OUT_OF_MEMORY;

  *aStreamListener = nsnull;
  // We want the mimeInfo's primary extension to pass it to
  // nsExternalAppHandler
  nsCAutoString buf;
  mimeInfo->GetPrimaryExtension(buf);

  nsExternalAppHandler * handler = new nsExternalAppHandler(mimeInfo,
                                                            buf,
                                                            aWindowContext,
                                                            fileName,
                                                            reason,
                                                            aForceSave);
  if (!handler)
    return NS_ERROR_OUT_OF_MEMORY;
  NS_ADDREF(*aStreamListener = handler);
  
  return NS_OK;
}

NS_IMETHODIMP nsExternalHelperAppService::ApplyDecodingForExtension(const nsACString& aExtension,
                                                                    const nsACString& aEncodingType,
                                                                    PRBool *aApplyDecoding)
{
  *aApplyDecoding = PR_TRUE;
  PRUint32 i;
  for(i = 0; i < NS_ARRAY_LENGTH(nonDecodableExtensions); ++i) {
    if (aExtension.LowerCaseEqualsASCII(nonDecodableExtensions[i].mFileExtension) &&
        aEncodingType.LowerCaseEqualsASCII(nonDecodableExtensions[i].mMimeType)) {
      *aApplyDecoding = PR_FALSE;
      break;
    }
  }
  return NS_OK;
}

nsresult nsExternalHelperAppService::GetFileTokenForPath(const PRUnichar * aPlatformAppPath,
                                                         nsIFile ** aFile)
{
  nsDependentString platformAppPath(aPlatformAppPath);
  // First, check if we have an absolute path
  nsILocalFile* localFile = nsnull;
  nsresult rv = NS_NewLocalFile(platformAppPath, PR_TRUE, &localFile);
  if (NS_SUCCEEDED(rv)) {
    *aFile = localFile;
    PRBool exists;
    if (NS_FAILED((*aFile)->Exists(&exists)) || !exists) {
      NS_RELEASE(*aFile);
      return NS_ERROR_FILE_NOT_FOUND;
    }
    return NS_OK;
  }


  // Second, check if file exists in mozilla program directory
  rv = NS_GetSpecialDirectory(NS_XPCOM_CURRENT_PROCESS_DIR, aFile);
  if (NS_SUCCEEDED(rv)) {
    rv = (*aFile)->Append(platformAppPath);
    if (NS_SUCCEEDED(rv)) {
      PRBool exists = PR_FALSE;
      rv = (*aFile)->Exists(&exists);
      if (NS_SUCCEEDED(rv) && exists)
        return NS_OK;
    }
    NS_RELEASE(*aFile);
  }


  return NS_ERROR_NOT_AVAILABLE;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// begin external protocol service default implementation...
//////////////////////////////////////////////////////////////////////////////////////////////////////
NS_IMETHODIMP nsExternalHelperAppService::ExternalProtocolHandlerExists(const char * aProtocolScheme,
                                                                        PRBool * aHandlerExists)
{
  nsCOMPtr<nsIHandlerInfo> handlerInfo;
  nsresult rv = GetProtocolHandlerInfo(nsDependentCString(aProtocolScheme), 
                                       getter_AddRefs(handlerInfo));
  NS_ENSURE_SUCCESS(rv, rv);

  // See if we have any known possible handler apps for this
  nsCOMPtr<nsIMutableArray> possibleHandlers;
  handlerInfo->GetPossibleApplicationHandlers(getter_AddRefs(possibleHandlers));

  PRUint32 length;
  possibleHandlers->GetLength(&length);
  if (length) {
    *aHandlerExists = PR_TRUE;
    return NS_OK;
  }

  // if not, fall back on an os-based handler
  return OSProtocolHandlerExists(aProtocolScheme, aHandlerExists);
}

NS_IMETHODIMP nsExternalHelperAppService::IsExposedProtocol(const char * aProtocolScheme, PRBool * aResult)
{
  // by default, no protocol is exposed.  i.e., by default all link clicks must
  // go through the external protocol service.  most applications override this
  // default behavior.
  *aResult = PR_FALSE;

  nsCOMPtr<nsIPrefBranch> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID);
  if (prefs)
  {
    PRBool val;
    nsresult rv;

    // check the per protocol setting first.  it always takes precidence.
    // if not set, then use the global setting.

    nsCAutoString name;
    name = NS_LITERAL_CSTRING("network.protocol-handler.expose.")
         + nsDependentCString(aProtocolScheme);
    rv = prefs->GetBoolPref(name.get(), &val);
    if (NS_SUCCEEDED(rv))
    {
      *aResult = val;
    }
    else
    {
      rv = prefs->GetBoolPref("network.protocol-handler.expose-all", &val);
      if (NS_SUCCEEDED(rv) && val)
        *aResult = PR_TRUE;
    }
  }
  return NS_OK;
}

NS_IMETHODIMP nsExternalHelperAppService::LoadUrl(nsIURI * aURL)
{
  return LoadURI(aURL, nsnull);
}

static const char kExternalProtocolPrefPrefix[]  = "network.protocol-handler.external.";
static const char kExternalProtocolDefaultPref[] = "network.protocol-handler.external-default";

NS_IMETHODIMP 
nsExternalHelperAppService::LoadURI(nsIURI *aURI,
                                    nsIInterfaceRequestor *aWindowContext)
{
  NS_ENSURE_ARG_POINTER(aURI);

  nsCAutoString spec;
  aURI->GetSpec(spec);

  if (spec.Find("%00") != -1)
    return NS_ERROR_MALFORMED_URI;

  spec.ReplaceSubstring("\"", "%22");
  spec.ReplaceSubstring("`", "%60");
  
  nsCOMPtr<nsIIOService> ios(do_GetIOService());
  nsCOMPtr<nsIURI> uri;
  nsresult rv = ios->NewURI(spec, nsnull, nsnull, getter_AddRefs(uri));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCAutoString scheme;
  uri->GetScheme(scheme);
  if (scheme.IsEmpty())
    return NS_OK; // must have a scheme

  nsCOMPtr<nsIPrefBranch> prefs(do_GetService(NS_PREFSERVICE_CONTRACTID));
  if (!prefs)
    return NS_OK; // deny if we can't check prefs

  // Deny load if the prefs say to do so
  nsCAutoString externalPref(kExternalProtocolPrefPrefix);
  externalPref += scheme;
  PRBool allowLoad  = PR_FALSE;
  rv = prefs->GetBoolPref(externalPref.get(), &allowLoad);
  if (NS_FAILED(rv))
  {
    // no scheme-specific value, check the default
    rv = prefs->GetBoolPref(kExternalProtocolDefaultPref, &allowLoad);
  }
  if (NS_FAILED(rv) || !allowLoad)
    return NS_OK; // explicitly denied or missing default pref

 
  nsCOMPtr<nsIHandlerInfo> handler;
  rv = GetProtocolHandlerInfo(scheme, getter_AddRefs(handler));
  NS_ENSURE_SUCCESS(rv, rv);

  nsHandlerInfoAction preferredAction;
  handler->GetPreferredAction(&preferredAction);
  PRBool alwaysAsk = PR_TRUE;
  handler->GetAlwaysAskBeforeHandling(&alwaysAsk);

  // if we are not supposed to ask, and the preferred action is to use
  // a helper app or the system default, we just launch the URI.
  if (!alwaysAsk && (preferredAction == nsIHandlerInfo::useHelperApp ||
                     preferredAction == nsIHandlerInfo::useSystemDefault))
    return handler->LaunchWithURI(uri, aWindowContext);
  
  nsCOMPtr<nsIContentDispatchChooser> chooser =
    do_CreateInstance("@mozilla.org/content-dispatch-chooser;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);
  
  return chooser->Ask(handler, aWindowContext, uri,
                      nsIContentDispatchChooser::REASON_CANNOT_HANDLE);
}

NS_IMETHODIMP nsExternalHelperAppService::GetApplicationDescription(const nsACString& aScheme, nsAString& _retval)
{
  // this method should only be implemented by each OS specific implementation of this service.
  return NS_ERROR_NOT_IMPLEMENTED;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Methods related to deleting temporary files on exit
//////////////////////////////////////////////////////////////////////////////////////////////////////

NS_IMETHODIMP nsExternalHelperAppService::DeleteTemporaryFileOnExit(nsIFile * aTemporaryFile)
{
  nsresult rv = NS_OK;
  PRBool isFile = PR_FALSE;
  nsCOMPtr<nsILocalFile> localFile (do_QueryInterface(aTemporaryFile, &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  // as a safety measure, make sure the nsIFile is really a file and not a directory object.
  localFile->IsFile(&isFile);
  if (!isFile) return NS_OK;

  mTemporaryFilesList.AppendObject(localFile);

  return NS_OK;
}

void nsExternalHelperAppService::FixFilePermissions(nsILocalFile* aFile)
{
  // This space intentionally left blank
}

nsresult nsExternalHelperAppService::ExpungeTemporaryFiles()
{
  PRInt32 numEntries = mTemporaryFilesList.Count();
  nsILocalFile* localFile;
  for (PRInt32 index = 0; index < numEntries; index++)
  {
    localFile = mTemporaryFilesList[index];
    if (localFile) {
      // First make the file writable, since the temp file is probably readonly.
      localFile->SetPermissions(0600);
      localFile->Remove(PR_FALSE);
    }
  }

  mTemporaryFilesList.Clear();

  return NS_OK;
}

static const char kExternalWarningPrefPrefix[] = 
  "network.protocol-handler.warn-external.";
static const char kExternalWarningDefaultPref[] = 
  "network.protocol-handler.warn-external-default";

NS_IMETHODIMP
nsExternalHelperAppService::GetProtocolHandlerInfo(const nsACString &aScheme,
                                                   nsIHandlerInfo **aHandlerInfo)
{
  // XXX enterprise customers should be able to turn this support off with a
  // single master pref (maybe use one of the "exposed" prefs here?)

  PRBool exists;
  nsresult rv = GetProtocolHandlerInfoFromOS(aScheme, &exists, aHandlerInfo);
  if (NS_FAILED(rv)) {
    // Either it knows nothing, or we ran out of memory
    return NS_ERROR_FAILURE;
  }
  
  rv = NS_ERROR_FAILURE;
  nsCOMPtr<nsIHandlerService> handlerSvc = do_GetService(NS_HANDLERSERVICE_CONTRACTID);
  if (handlerSvc)
    rv = handlerSvc->FillHandlerInfo(*aHandlerInfo, EmptyCString());

  if (NS_SUCCEEDED(rv))
    return NS_OK;
  
  return SetProtocolHandlerDefaults(*aHandlerInfo, exists);
}

NS_IMETHODIMP
nsExternalHelperAppService::GetProtocolHandlerInfoFromOS(const nsACString &aScheme,
                                                         PRBool *found,
                                                         nsIHandlerInfo **aHandlerInfo)
{
  // intended to be implemented by the subclass
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsExternalHelperAppService::SetProtocolHandlerDefaults(nsIHandlerInfo *aHandlerInfo,
                                                       PRBool aOSHandlerExists)
{
  // this type isn't in our database, so we've only got an OS default handler,
  // if one exists

  if (aOSHandlerExists) {
    // we've got a default, so use it
    aHandlerInfo->SetPreferredAction(nsIHandlerInfo::useSystemDefault);

    // whether or not to ask the user depends on the warning preference
    nsCOMPtr<nsIPrefBranch> prefs(do_GetService(NS_PREFSERVICE_CONTRACTID));
    if (!prefs)
      return NS_OK; // deny if we can't check prefs

    nsCAutoString scheme;
    aHandlerInfo->GetType(scheme);
    
    nsCAutoString warningPref(kExternalWarningPrefPrefix);
    warningPref += scheme;
    PRBool warn = PR_TRUE;
    nsresult rv = prefs->GetBoolPref(warningPref.get(), &warn);
    if (NS_FAILED(rv)) {
      // no scheme-specific value, check the default
      prefs->GetBoolPref(kExternalWarningDefaultPref, &warn);
    }
    aHandlerInfo->SetAlwaysAskBeforeHandling(warn);
  } else {
    // If no OS default existed, we set the preferred action to alwaysAsk. 
    // This really means not initialized (i.e. there's no available handler)
    // to all the code...
    aHandlerInfo->SetPreferredAction(nsIHandlerInfo::alwaysAsk);
  }

  return NS_OK;
}
 
// XPCOM profile change observer
NS_IMETHODIMP
nsExternalHelperAppService::Observe(nsISupports *aSubject, const char *aTopic, const PRUnichar *someData )
{
  if (!strcmp(aTopic, "profile-before-change")) {
    ExpungeTemporaryFiles();
  }
  return NS_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// begin external app handler implementation 
//////////////////////////////////////////////////////////////////////////////////////////////////////

NS_IMPL_THREADSAFE_ADDREF(nsExternalAppHandler)
NS_IMPL_THREADSAFE_RELEASE(nsExternalAppHandler)

NS_INTERFACE_MAP_BEGIN(nsExternalAppHandler)
   NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIStreamListener)
   NS_INTERFACE_MAP_ENTRY(nsIStreamListener)
   NS_INTERFACE_MAP_ENTRY(nsIRequestObserver)
   NS_INTERFACE_MAP_ENTRY(nsIHelperAppLauncher)   
   NS_INTERFACE_MAP_ENTRY(nsICancelable)
   NS_INTERFACE_MAP_ENTRY(nsITimerCallback)
NS_INTERFACE_MAP_END_THREADSAFE

nsExternalAppHandler::nsExternalAppHandler(nsIMIMEInfo * aMIMEInfo,
                                           const nsCSubstring& aTempFileExtension,
                                           nsIInterfaceRequestor* aWindowContext,
                                           const nsAString& aSuggestedFilename,
                                           PRUint32 aReason, PRBool aForceSave)
: mMimeInfo(aMIMEInfo)
, mWindowContext(aWindowContext)
, mWindowToClose(nsnull)
, mSuggestedFileName(aSuggestedFilename)
, mForceSave(aForceSave)
, mCanceled(PR_FALSE)
, mShouldCloseWindow(PR_FALSE)
, mReceivedDispositionInfo(PR_FALSE)
, mStopRequestIssued(PR_FALSE)
, mProgressListenerInitialized(PR_FALSE)
, mReason(aReason)
, mContentLength(-1)
, mProgress(0)
, mRequest(nsnull)
{

  // make sure the extention includes the '.'
  if (!aTempFileExtension.IsEmpty() && aTempFileExtension.First() != '.')
    mTempFileExtension = PRUnichar('.');
  AppendUTF8toUTF16(aTempFileExtension, mTempFileExtension);

  // replace platform specific path separator and illegal characters to avoid any confusion
  mSuggestedFileName.ReplaceChar(FILE_PATH_SEPARATOR FILE_ILLEGAL_CHARACTERS, '_');
  mTempFileExtension.ReplaceChar(FILE_PATH_SEPARATOR FILE_ILLEGAL_CHARACTERS, '_');
  
  // Make sure extension is correct.
  EnsureSuggestedFileName();

  gExtProtSvc->AddRef();
}

nsExternalAppHandler::~nsExternalAppHandler()
{
  // Not using NS_RELEASE, since we don't want to set gExtProtSvc to NULL
  gExtProtSvc->Release();
}

NS_IMETHODIMP nsExternalAppHandler::SetWebProgressListener(nsIWebProgressListener2 * aWebProgressListener)
{ 
  // this call back means we've successfully brought up the 
  // progress window so set the appropriate flag, even though
  // aWebProgressListener might be null
  
  if (mReceivedDispositionInfo)
    mProgressListenerInitialized = PR_TRUE;

  // Go ahead and register the progress listener....
  mWebProgressListener = aWebProgressListener;

  // while we were bringing up the progress dialog, we actually finished processing the
  // url. If that's the case then mStopRequestIssued will be true. We need to execute the
  // operation since we are actually done now.
  if (mStopRequestIssued && aWebProgressListener)
  {
    return ExecuteDesiredAction();
  }

  return NS_OK;
}

NS_IMETHODIMP nsExternalAppHandler::GetTargetFile(nsIFile** aTarget)
{
  if (mFinalFileDestination)
    *aTarget = mFinalFileDestination;
  else
    *aTarget = mTempFile;

  NS_IF_ADDREF(*aTarget);
  return NS_OK;
}

NS_IMETHODIMP nsExternalAppHandler::GetTargetFileIsExecutable(PRBool *aExec)
{
  // Use the real target if it's been set
  if (mFinalFileDestination)
    return mFinalFileDestination->IsExecutable(aExec);

  // Otherwise, use the stored executable-ness of the temporary
  *aExec = mTempFileIsExecutable;
  return NS_OK;
}

NS_IMETHODIMP nsExternalAppHandler::GetTimeDownloadStarted(PRTime* aTime)
{
  *aTime = mTimeDownloadStarted;
  return NS_OK;
}

NS_IMETHODIMP nsExternalAppHandler::CloseProgressWindow()
{
  // release extra state...
  mWebProgressListener = nsnull;
  return NS_OK;
}

void nsExternalAppHandler::RetargetLoadNotifications(nsIRequest *request)
{
  // we are going to run the downloading of the helper app in our own little docloader / load group context. 
  // so go ahead and force the creation of a load group and doc loader for us to use...
  nsCOMPtr<nsIChannel> aChannel = do_QueryInterface(request);
  if (!aChannel)
    return;

  // we need to store off the original (pre redirect!) channel that initiated the load. We do
  // this so later on, we can pass any refresh urls associated with the original channel back to the 
  // window context which started the whole process. More comments about that are listed below....
  // HACK ALERT: it's pretty bogus that we are getting the document channel from the doc loader. 
  // ideally we should be able to just use mChannel (the channel we are extracting content from) or
  // the default load channel associated with the original load group. Unfortunately because
  // a redirect may have occurred, the doc loader is the only one with a ptr to the original channel 
  // which is what we really want....

  // Note that we need to do this before removing aChannel from the loadgroup,
  // since that would mess with the original channel on the loader.
  nsCOMPtr<nsIDocumentLoader> origContextLoader =
    do_GetInterface(mWindowContext);
  if (origContextLoader)
    origContextLoader->GetDocumentChannel(getter_AddRefs(mOriginalChannel));

  nsCOMPtr<nsILoadGroup> oldLoadGroup;
  aChannel->GetLoadGroup(getter_AddRefs(oldLoadGroup));

  if(oldLoadGroup)
     oldLoadGroup->RemoveRequest(request, nsnull, NS_BINDING_RETARGETED);
      
  aChannel->SetLoadGroup(nsnull);
  aChannel->SetNotificationCallbacks(nsnull);

}

/**
 * Make mTempFileExtension contain an extension exactly when its previous value
 * is different from mSuggestedFileName's extension, so that it can be appended
 * to mSuggestedFileName and form a valid, useful leaf name.
 * This is required so that the (renamed) temporary file has the correct extension
 * after downloading to make sure the OS will launch the application corresponding
 * to the MIME type (which was used to calculate mTempFileExtension).  This prevents
 * a cgi-script named foobar.exe that returns application/zip from being named
 * foobar.exe and executed as an executable file. It also blocks content that
 * a web site might provide with a content-disposition header indicating
 * filename="foobar.exe" from being downloaded to a file with extension .exe
 * and executed.
 */
void nsExternalAppHandler::EnsureSuggestedFileName()
{
  // Make sure there is a mTempFileExtension (not "" or ".").
  // Remember that mTempFileExtension will always have the leading "."
  // (the check for empty is just to be safe).
  if (mTempFileExtension.Length() > 1)
  {
    // Get mSuggestedFileName's current extension.
    nsAutoString fileExt;
    PRInt32 pos = mSuggestedFileName.RFindChar('.');
    if (pos != kNotFound)
      mSuggestedFileName.Right(fileExt, mSuggestedFileName.Length() - pos);

    // Now, compare fileExt to mTempFileExtension.
    if (fileExt.Equals(mTempFileExtension, nsCaseInsensitiveStringComparator()))
    {
      // Matches -> mTempFileExtension can be empty
      mTempFileExtension.Truncate();
    }
  }
}

nsresult nsExternalAppHandler::SetUpTempFile(nsIChannel * aChannel)
{
  nsresult rv;

#ifdef XP_MACOSX
 // create a temp file for the data...and open it for writing.
 // use NS_MAC_DEFAULT_DOWNLOAD_DIR which gets download folder from InternetConfig
 // if it can't get download folder pref, then it uses desktop folder
  rv = NS_GetSpecialDirectory(NS_MAC_DEFAULT_DOWNLOAD_DIR,
                              getter_AddRefs(mTempFile));
#else
  // create a temp file for the data...and open it for writing.
  rv = NS_GetSpecialDirectory(NS_OS_TEMP_DIR, getter_AddRefs(mTempFile));
#endif
  NS_ENSURE_SUCCESS(rv, rv);

  // At this point, we do not have a filename for the temp file.  For security
  // purposes, this cannot be predictable, so we must use a cryptographic
  // quality PRNG to generate one.
  // We will request raw random bytes, and transform that to a base64 string,
  // as all characters from the base64 set are acceptable for filenames.  For
  // each three bytes of random data, we will get four bytes of ASCII.  Request
  // a bit more, to be safe, and truncate to the length we want in the end.

  const PRUint32 wantedFileNameLength = 8;
  const PRUint32 requiredBytesLength =
    static_cast<PRUint32>((wantedFileNameLength + 1) / 4 * 3);

  nsCOMPtr<nsIRandomGenerator> rg =
    do_GetService("@mozilla.org/security/random-generator;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  PRUint8 *buffer;
  rv = rg->GenerateRandomBytes(requiredBytesLength, &buffer);
  NS_ENSURE_SUCCESS(rv, rv);

  char *b64 = PL_Base64Encode(reinterpret_cast<const char *>(buffer),
                              requiredBytesLength, nsnull);
  NS_Free(buffer);
  buffer = nsnull;

  if (!b64)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ASSERTION(strlen(b64) >= wantedFileNameLength,
               "not enough bytes produced for conversion!");

  nsCAutoString tempLeafName(b64, wantedFileNameLength);
  PR_Free(b64);
  b64 = nsnull;

  // Base64 characters are alphanumeric (a-zA-Z0-9) and '+' and '/', so we need
  // to replace illegal characters -- notably '/'
  tempLeafName.ReplaceChar(FILE_PATH_SEPARATOR FILE_ILLEGAL_CHARACTERS, '_');

  // now append our extension.
  nsCAutoString ext;
  mMimeInfo->GetPrimaryExtension(ext);
  if (!ext.IsEmpty()) {
    ext.ReplaceChar(FILE_PATH_SEPARATOR FILE_ILLEGAL_CHARACTERS, '_');
    if (ext.First() != '.')
      tempLeafName.Append('.');
    tempLeafName.Append(ext);
  }

#ifdef XP_WIN
  // On windows, we need to temporarily create a dummy file with the correct
  // file extension to determine the executable-ness, so do this before adding
  // the extra .part extension.
  nsCOMPtr<nsIFile> dummyFile;
  rv = NS_GetSpecialDirectory(NS_OS_TEMP_DIR, getter_AddRefs(dummyFile));
  NS_ENSURE_SUCCESS(rv, rv);

  // Set the file name without .part
  rv = dummyFile->Append(NS_ConvertUTF8toUTF16(tempLeafName));
  NS_ENSURE_SUCCESS(rv, rv);
  rv = dummyFile->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0600);
  NS_ENSURE_SUCCESS(rv, rv);

  // Store executable-ness then delete
  dummyFile->IsExecutable(&mTempFileIsExecutable);
  dummyFile->Remove(PR_FALSE);
#endif

  // Add an additional .part to prevent the OS from running this file in the
  // default application.
  tempLeafName.Append(NS_LITERAL_CSTRING(".part"));

  rv = mTempFile->Append(NS_ConvertUTF8toUTF16(tempLeafName));
  // make this file unique!!!
  NS_ENSURE_SUCCESS(rv, rv);
  rv = mTempFile->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0600);
  NS_ENSURE_SUCCESS(rv, rv);

#ifndef XP_WIN
  // On other platforms, the file permission bits are used, so we can just call
  // IsExecutable
  mTempFile->IsExecutable(&mTempFileIsExecutable);
#endif

#ifdef XP_MACOSX
  // Now that the file exists set Mac type if the file has no extension
  // and we can determine a type.
  if (ext.IsEmpty() && mMimeInfo) {
    nsCOMPtr<nsILocalFileMac> macfile = do_QueryInterface(mTempFile);
    if (macfile) {
      PRUint32 type;
      mMimeInfo->GetMacType(&type);
      macfile->SetFileType(type);
    }
  }
#endif

  nsCOMPtr<nsIOutputStream> outputStream;
  rv = NS_NewLocalFileOutputStream(getter_AddRefs(outputStream), mTempFile,
                                   PR_WRONLY | PR_CREATE_FILE, 0600);
  if (NS_FAILED(rv)) {
    mTempFile->Remove(PR_FALSE);
    return rv;
  }

  mOutStream = NS_BufferOutputStream(outputStream, BUFFERED_OUTPUT_SIZE);

#ifdef XP_MACOSX
    nsCAutoString contentType;
    mMimeInfo->GetMIMEType(contentType);
    if (contentType.LowerCaseEqualsLiteral(APPLICATION_APPLEFILE) ||
        contentType.LowerCaseEqualsLiteral(MULTIPART_APPLEDOUBLE))
    {
      nsCOMPtr<nsIAppleFileDecoder> appleFileDecoder = do_CreateInstance(NS_IAPPLEFILEDECODER_CONTRACTID, &rv);
      if (NS_SUCCEEDED(rv))
      {
        rv = appleFileDecoder->Initialize(mOutStream, mTempFile);
        if (NS_SUCCEEDED(rv))
          mOutStream = do_QueryInterface(appleFileDecoder, &rv);
      }
    }
#endif

  return rv;
}

NS_IMETHODIMP nsExternalAppHandler::OnStartRequest(nsIRequest *request, nsISupports * aCtxt)
{
  NS_PRECONDITION(request, "OnStartRequest without request?");

  // Set mTimeDownloadStarted here as the download has already started and
  // we want to record the start time before showing the filepicker.
  mTimeDownloadStarted = PR_Now();

  mRequest = request;

  nsCOMPtr<nsIChannel> aChannel = do_QueryInterface(request);
  
  nsresult rv;
  
  nsCOMPtr<nsIFileChannel> fileChan(do_QueryInterface(request));
  mIsFileChannel = fileChan != nsnull;

  // Get content length
  nsCOMPtr<nsIPropertyBag2> props(do_QueryInterface(request, &rv));
  if (props) {
    rv = props->GetPropertyAsInt64(NS_CHANNEL_PROP_CONTENT_LENGTH,
                                   &mContentLength.mValue);
  }
  // If that failed, ask the channel
  if (NS_FAILED(rv) && aChannel) {
    PRInt32 len;
    aChannel->GetContentLength(&len);
    mContentLength = len;
  }

  // Determine whether a new window was opened specifically for this request
  if (props) {
    PRBool tmp = PR_FALSE;
    props->GetPropertyAsBool(NS_LITERAL_STRING("docshell.newWindowTarget"),
                             &tmp);
    mShouldCloseWindow = tmp;
  }

  // Now get the URI
  if (aChannel)
  {
    aChannel->GetURI(getter_AddRefs(mSourceUrl));
  }

  rv = SetUpTempFile(aChannel);
  if (NS_FAILED(rv)) {
    mCanceled = PR_TRUE;
    request->Cancel(rv);
    nsAutoString path;
    if (mTempFile)
      mTempFile->GetPath(path);
    SendStatusChange(kWriteError, rv, request, path);
    return NS_OK;
  }

  // Extract mime type for later use below.
  nsCAutoString MIMEType;
  mMimeInfo->GetMIMEType(MIMEType);

  // retarget all load notifications to our docloader instead of the original window's docloader...
  RetargetLoadNotifications(request);

  // Check to see if there is a refresh header on the original channel.
  if (mOriginalChannel) {
    nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(mOriginalChannel));
    if (httpChannel) {
      nsCAutoString refreshHeader;
      httpChannel->GetResponseHeader(NS_LITERAL_CSTRING("refresh"),
                                     refreshHeader);
      if (!refreshHeader.IsEmpty()) {
        mShouldCloseWindow = PR_FALSE;
      }
    }
  }

  // Close the underlying DOMWindow if there is no refresh header
  // and it was opened specifically for the download
  MaybeCloseWindow();

  nsCOMPtr<nsIEncodedChannel> encChannel = do_QueryInterface( aChannel );
  if (encChannel) 
  {
    // Turn off content encoding conversions if needed
    PRBool applyConversion = PR_TRUE;

    nsCOMPtr<nsIURL> sourceURL(do_QueryInterface(mSourceUrl));
    if (sourceURL)
    {
      nsCAutoString extension;
      sourceURL->GetFileExtension(extension);
      if (!extension.IsEmpty())
      {
        nsCOMPtr<nsIUTF8StringEnumerator> encEnum;
        encChannel->GetContentEncodings(getter_AddRefs(encEnum));
        if (encEnum)
        {
          PRBool hasMore;
          rv = encEnum->HasMore(&hasMore);
          if (NS_SUCCEEDED(rv) && hasMore)
          {
            nsCAutoString encType;
            rv = encEnum->GetNext(encType);
            if (NS_SUCCEEDED(rv) && !encType.IsEmpty())
            {
              NS_ASSERTION(gExtProtSvc, "Where did the service go?");
              gExtProtSvc->ApplyDecodingForExtension(extension, encType,
                                                     &applyConversion);
            }
          }
        }
      }    
    }

    encChannel->SetApplyConversion( applyConversion );
  }

  // now that the temp file is set up, find out if we need to invoke a dialog
  // asking the user what they want us to do with this content...

  // We can get here for three reasons: "can't handle", "sniffed type", or
  // "server sent content-disposition:attachment".  In the first case we want
  // to honor the user's "always ask" pref; in the other two cases we want to
  // honor it only if the default action is "save".  Opening attachments in
  // helper apps by default breaks some websites (especially if the attachment
  // is one part of a multipart document).  Opening sniffed content in helper
  // apps by default introduces security holes that we'd rather not have.

  // So let's find out whether the user wants to be prompted.  If he does not,
  // check mReason and the preferred action to see what we should do.

  PRBool alwaysAsk = PR_TRUE;
  mMimeInfo->GetAlwaysAskBeforeHandling(&alwaysAsk);
  if (alwaysAsk)
  {
    // But we *don't* ask if this mimeInfo didn't come from
    // our user configuration datastore and the user has said
    // at some point in the distant past that they don't
    // want to be asked.  The latter fact would have been
    // stored in pref strings back in the old days.
    NS_ASSERTION(gExtProtSvc, "Service gone away!?");

    PRBool mimeTypeIsInDatastore = PR_FALSE;
    nsCOMPtr<nsIHandlerService> handlerSvc = do_GetService(NS_HANDLERSERVICE_CONTRACTID);
    if (handlerSvc)
      handlerSvc->Exists(mMimeInfo, &mimeTypeIsInDatastore);
    if (!handlerSvc || !mimeTypeIsInDatastore)
    {
      if (!GetNeverAskFlagFromPref(NEVER_ASK_FOR_SAVE_TO_DISK_PREF, MIMEType.get()))
      {
        // Don't need to ask after all.
        alwaysAsk = PR_FALSE;
        // Make sure action matches pref (save to disk).
        mMimeInfo->SetPreferredAction(nsIMIMEInfo::saveToDisk);
      }
      else if (!GetNeverAskFlagFromPref(NEVER_ASK_FOR_OPEN_FILE_PREF, MIMEType.get()))
      {
        // Don't need to ask after all.
        alwaysAsk = PR_FALSE;
      }
    }
  }

  PRInt32 action = nsIMIMEInfo::saveToDisk;
  mMimeInfo->GetPreferredAction( &action );

  // OK, now check why we're here
  if (!alwaysAsk && mReason != nsIHelperAppLauncherDialog::REASON_CANTHANDLE) {
    // Force asking if we're not saving.  See comment back when we fetched the
    // alwaysAsk boolean for details.
    alwaysAsk = (action != nsIMIMEInfo::saveToDisk);
  }

  // if we were told that we _must_ save to disk without asking, all the stuff
  // before this is irrelevant; override it
  if (mForceSave) {
    alwaysAsk = PR_FALSE;
    action = nsIMIMEInfo::saveToDisk;
  }
  
  if (alwaysAsk)
  {
    // do this first! make sure we don't try to take an action until the user tells us what they want to do
    // with it...
    mReceivedDispositionInfo = PR_FALSE; 

    // invoke the dialog!!!!! use mWindowContext as the window context parameter for the dialog request
    mDialog = do_CreateInstance( NS_IHELPERAPPLAUNCHERDLG_CONTRACTID, &rv );
    NS_ENSURE_SUCCESS(rv, rv);

    // this will create a reference cycle (the dialog holds a reference to us as
    // nsIHelperAppLauncher), which will be broken in Cancel or
    // CreateProgressListener.
    rv = mDialog->Show( this, mWindowContext, mReason );

    // what do we do if the dialog failed? I guess we should call Cancel and abort the load....
  }
  else
  {
    mReceivedDispositionInfo = PR_TRUE; // no need to wait for a response from the user

    // We need to do the save/open immediately, then.
#ifdef XP_WIN
    /* We need to see whether the file we've got here could be
     * executable.  If it could, we had better not try to open it!
     * We can skip this check, though, if we have a setting to open in a
     * helper app.
     * This code mirrors the code in
     * nsExternalAppHandler::LaunchWithApplication so that what we
     * test here is as close as possible to what will really be
     * happening if we decide to execute
     */
    nsCOMPtr<nsIHandlerApp> prefApp;
    mMimeInfo->GetPreferredApplicationHandler(getter_AddRefs(prefApp));
    if (action != nsIMIMEInfo::useHelperApp || !prefApp) {
      nsCOMPtr<nsIFile> fileToTest;
      GetTargetFile(getter_AddRefs(fileToTest));
      if (fileToTest) {
        PRBool isExecutable;
        rv = fileToTest->IsExecutable(&isExecutable);
        if (NS_FAILED(rv) || isExecutable) {  // checking NS_FAILED, because paranoia is good
          action = nsIMIMEInfo::saveToDisk;
        }
      } else {   // Paranoia is good here too, though this really should not happen
        NS_WARNING("GetDownloadInfo returned a null file after the temp file has been set up! ");
        action = nsIMIMEInfo::saveToDisk;
      }
    }

#endif
    if (action == nsIMIMEInfo::useHelperApp ||
        action == nsIMIMEInfo::useSystemDefault)
    {
        rv = LaunchWithApplication(nsnull, PR_FALSE);
    }
    else // Various unknown actions go here too
    {
        rv = SaveToDisk(nsnull, PR_FALSE);
    }
  }

  // Now let's add the download to history
  nsCOMPtr<nsIDownloadHistory> dh(do_GetService(NS_DOWNLOADHISTORY_CONTRACTID));
  if (dh) {
    nsCOMPtr<nsIURI> referrer;
    if (aChannel)
      NS_GetReferrerFromChannel(aChannel, getter_AddRefs(referrer));
    dh->AddDownload(mSourceUrl, referrer, mTimeDownloadStarted);
  }

  return NS_OK;
}

// Convert error info into proper message text and send OnStatusChange notification
// to the web progress listener.
void nsExternalAppHandler::SendStatusChange(ErrorType type, nsresult rv, nsIRequest *aRequest, const nsAFlatString &path)
{
    nsAutoString msgId;
    switch(rv)
    {
    case NS_ERROR_OUT_OF_MEMORY:
        // No memory
        msgId.AssignLiteral("noMemory");
        break;

    case NS_ERROR_FILE_DISK_FULL:
    case NS_ERROR_FILE_NO_DEVICE_SPACE:
        // Out of space on target volume.
        msgId.AssignLiteral("diskFull");
        break;

    case NS_ERROR_FILE_READ_ONLY:
        // Attempt to write to read/only file.
        msgId.AssignLiteral("readOnly");
        break;

    case NS_ERROR_FILE_ACCESS_DENIED:
        if (type == kWriteError) {
          // Attempt to write without sufficient permissions.
          msgId.AssignLiteral("accessError");
        }
        else
        {
          msgId.AssignLiteral("launchError");
        }
        break;

    case NS_ERROR_FILE_NOT_FOUND:
    case NS_ERROR_FILE_TARGET_DOES_NOT_EXIST:
    case NS_ERROR_FILE_UNRECOGNIZED_PATH:
        // Helper app not found, let's verify this happened on launch
        if (type == kLaunchError) {
          msgId.AssignLiteral("helperAppNotFound");
          break;
        }
        // fall through

    default:
        // Generic read/write/launch error message.
        switch(type)
        {
        case kReadError:
          msgId.AssignLiteral("readError");
          break;
        case kWriteError:
          msgId.AssignLiteral("writeError");
          break;
        case kLaunchError:
          msgId.AssignLiteral("launchError");
          break;
        }
        break;
    }
    PR_LOG(nsExternalHelperAppService::mLog, PR_LOG_ERROR,
        ("Error: %s, type=%i, listener=0x%p, rv=0x%08X\n",
         NS_LossyConvertUTF16toASCII(msgId).get(), type, mWebProgressListener.get(), rv));
    PR_LOG(nsExternalHelperAppService::mLog, PR_LOG_ERROR,
        ("       path='%s'\n", NS_ConvertUTF16toUTF8(path).get()));

    // Get properties file bundle and extract status string.
    nsCOMPtr<nsIStringBundleService> s = do_GetService(NS_STRINGBUNDLE_CONTRACTID);
    if (s)
    {
        nsCOMPtr<nsIStringBundle> bundle;
        if (NS_SUCCEEDED(s->CreateBundle("chrome://global/locale/nsWebBrowserPersist.properties", getter_AddRefs(bundle))))
        {
            nsXPIDLString msgText;
            const PRUnichar *strings[] = { path.get() };
            if(NS_SUCCEEDED(bundle->FormatStringFromName(msgId.get(), strings, 1, getter_Copies(msgText))))
            {
              if (mWebProgressListener)
              {
                // We have a listener, let it handle the error.
                mWebProgressListener->OnStatusChange(nsnull, (type == kReadError) ? aRequest : nsnull, rv, msgText);
              }
              else
              {
                // We don't have a listener.  Simply show the alert ourselves.
                nsCOMPtr<nsIPrompt> prompter(do_GetInterface(mWindowContext));
                nsXPIDLString title;
                bundle->FormatStringFromName(NS_LITERAL_STRING("title").get(),
                                             strings,
                                             1,
                                             getter_Copies(title));
                if (prompter)
                {
                  prompter->Alert(title, msgText);
                }
              }
            }
        }
    }
}

NS_IMETHODIMP nsExternalAppHandler::OnDataAvailable(nsIRequest *request, nsISupports * aCtxt,
                                                  nsIInputStream * inStr, PRUint32 sourceOffset, PRUint32 count)
{
  nsresult rv = NS_OK;
  // first, check to see if we've been canceled....
  if (mCanceled) // then go cancel our underlying channel too
    return request->Cancel(NS_BINDING_ABORTED);

  // read the data out of the stream and write it to the temp file.
  if (mOutStream && count > 0)
  {
    PRUint32 numBytesRead = 0; 
    PRUint32 numBytesWritten = 0;
    mProgress += count;
    PRBool readError = PR_TRUE;
    while (NS_SUCCEEDED(rv) && count > 0) // while we still have bytes to copy...
    {
      readError = PR_TRUE;
      rv = inStr->Read(mDataBuffer, PR_MIN(count, DATA_BUFFER_SIZE - 1), &numBytesRead);
      if (NS_SUCCEEDED(rv))
      {
        if (count >= numBytesRead)
          count -= numBytesRead; // subtract off the number of bytes we just read
        else
          count = 0;
        readError = PR_FALSE;
        // Write out the data until something goes wrong, or, it is
        // all written.  We loop because for some errors (e.g., disk
        // full), we get NS_OK with some bytes written, then an error.
        // So, we want to write again in that case to get the actual
        // error code.
        const char *bufPtr = mDataBuffer; // Where to write from.
        while (NS_SUCCEEDED(rv) && numBytesRead)
        {
          numBytesWritten = 0;
          rv = mOutStream->Write(bufPtr, numBytesRead, &numBytesWritten);
          if (NS_SUCCEEDED(rv))
          {
            numBytesRead -= numBytesWritten;
            bufPtr += numBytesWritten;
            // Force an error if (for some reason) we get NS_OK but
            // no bytes written.
            if (!numBytesWritten)
            {
              rv = NS_ERROR_FAILURE;
            }
          }
        }
      }
    }
    if (NS_SUCCEEDED(rv))
    {
      // Send progress notification.
      if (mWebProgressListener)
      {
        mWebProgressListener->OnProgressChange64(nsnull, request, mProgress, mContentLength, mProgress, mContentLength);
      }
    }
    else
    {
      // An error occurred, notify listener.
      nsAutoString tempFilePath;
      if (mTempFile)
        mTempFile->GetPath(tempFilePath);
      SendStatusChange(readError ? kReadError : kWriteError, rv, request, tempFilePath);

      // Cancel the download.
      Cancel(rv);
    }
  }
  return rv;
}

NS_IMETHODIMP nsExternalAppHandler::OnStopRequest(nsIRequest *request, nsISupports *aCtxt, 
                                                  nsresult aStatus)
{
  mStopRequestIssued = PR_TRUE;
  mRequest = nsnull;
  // Cancel if the request did not complete successfully.
  if (!mCanceled && NS_FAILED(aStatus))
  {
    // Send error notification.
    nsAutoString tempFilePath;
    if (mTempFile)
      mTempFile->GetPath(tempFilePath);
    SendStatusChange( kReadError, aStatus, request, tempFilePath );

    Cancel(aStatus);
  }

  // first, check to see if we've been canceled....
  if (mCanceled)
    return NS_OK;

  // close the stream...
  if (mOutStream)
  {
    mOutStream->Close();
    mOutStream = nsnull;
  }

  // Do what the user asked for
  ExecuteDesiredAction();

  // At this point, the channel should still own us. So releasing the reference
  // to us in the nsITransfer should be ok.
  // This nsITransfer object holds a reference to us (we are its observer), so
  // we need to release the reference to break a reference cycle (and therefore
  // to prevent leaking)
  mWebProgressListener = nsnull;

  return NS_OK;
}

nsresult nsExternalAppHandler::ExecuteDesiredAction()
{
  nsresult rv = NS_OK;
  if (mProgressListenerInitialized && !mCanceled)
  {
    nsHandlerInfoAction action = nsIMIMEInfo::saveToDisk;
    mMimeInfo->GetPreferredAction(&action);
    if (action == nsIMIMEInfo::useHelperApp ||
        action == nsIMIMEInfo::useSystemDefault)
    {
      // Make sure the suggested name is unique since in this case we don't
      // have a file name that was guaranteed to be unique by going through
      // the File Save dialog
      rv = mFinalFileDestination->CreateUnique(nsIFile::NORMAL_FILE_TYPE, 0600);
      if (NS_SUCCEEDED(rv))
      {
        // Source and dest dirs should be == so this should just do a rename
        rv = MoveFile(mFinalFileDestination);
        if (NS_SUCCEEDED(rv))
          rv = OpenWithApplication();
      }
    }
    else // Various unknown actions go here too
    {
      // XXX Put progress dialog in barber-pole mode
      //     and change text to say "Copying from:".
      rv = MoveFile(mFinalFileDestination);
      if (NS_SUCCEEDED(rv) && action == nsIMIMEInfo::saveToDisk)
      {
        nsCOMPtr<nsILocalFile> destfile(do_QueryInterface(mFinalFileDestination));
        gExtProtSvc->FixFilePermissions(destfile);
      }
    }
    
    // Notify dialog that download is complete.
    // By waiting till this point, it ensures that the progress dialog doesn't indicate
    // success until we're really done.
    if(mWebProgressListener)
    {
      if (!mCanceled)
      {
        mWebProgressListener->OnProgressChange64(nsnull, nsnull, mProgress, mContentLength, mProgress, mContentLength);
      }
      mWebProgressListener->OnStateChange(nsnull, nsnull,
        nsIWebProgressListener::STATE_STOP |
        nsIWebProgressListener::STATE_IS_REQUEST |
        nsIWebProgressListener::STATE_IS_NETWORK, NS_OK);
    }
  }
  
  return rv;
}

NS_IMETHODIMP nsExternalAppHandler::GetMIMEInfo(nsIMIMEInfo ** aMIMEInfo)
{
  *aMIMEInfo = mMimeInfo;
  NS_ADDREF(*aMIMEInfo);
  return NS_OK;
}

NS_IMETHODIMP nsExternalAppHandler::GetSource(nsIURI ** aSourceURI)
{
  NS_ENSURE_ARG(aSourceURI);
  *aSourceURI = mSourceUrl;
  NS_IF_ADDREF(*aSourceURI);
  return NS_OK;
}

NS_IMETHODIMP nsExternalAppHandler::GetSuggestedFileName(nsAString& aSuggestedFileName)
{
  aSuggestedFileName = mSuggestedFileName;
  return NS_OK;
}

nsresult nsExternalAppHandler::InitializeDownload(nsITransfer* aTransfer)
{
  nsresult rv;
  
  nsCOMPtr<nsIURI> target;
  rv = NS_NewFileURI(getter_AddRefs(target), mFinalFileDestination);
  if (NS_FAILED(rv)) return rv;
  
  nsCOMPtr<nsILocalFile> lf(do_QueryInterface(mTempFile));
  rv = aTransfer->Init(mSourceUrl, target, EmptyString(),
                       mMimeInfo, mTimeDownloadStarted, lf, this);
  if (NS_FAILED(rv)) return rv;

  return rv;
}

nsresult nsExternalAppHandler::CreateProgressListener()
{
  // we are back from the helper app dialog (where the user chooses to save or open), but we aren't
  // done processing the load. in this case, throw up a progress dialog so the user can see what's going on...
  // Also, release our reference to mDialog. We don't need it anymore, and we
  // need to break the reference cycle.
  mDialog = nsnull;
  nsresult rv;
  
  nsCOMPtr<nsITransfer> tr = do_CreateInstance(NS_TRANSFER_CONTRACTID, &rv);
  if (NS_SUCCEEDED(rv))
    InitializeDownload(tr);

  if (tr)
    tr->OnStateChange(nsnull, mRequest, nsIWebProgressListener::STATE_START |
      nsIWebProgressListener::STATE_IS_REQUEST |
      nsIWebProgressListener::STATE_IS_NETWORK, NS_OK);

  // note we might not have a listener here if the QI() failed, or if
  // there is no nsITransfer object, but we still call
  // SetWebProgressListener() to make sure our progress state is sane
  // NOTE: This will set up a reference cycle (this nsITransfer has us set up as
  // its observer). This cycle will be broken in Cancel, CloseProgressWindow or
  // OnStopRequest.
  SetWebProgressListener(tr);

  return rv;
}

nsresult nsExternalAppHandler::PromptForSaveToFile(nsILocalFile ** aNewFile, const nsAFlatString &aDefaultFile, const nsAFlatString &aFileExtension)
{
  // invoke the dialog!!!!! use mWindowContext as the window context parameter for the dialog request
  // Convert to use file picker? No, then embeddors could not do any sort of
  // "AutoDownload" w/o showing a prompt
  nsresult rv = NS_OK;
  if (!mDialog)
  {
    // Get helper app launcher dialog.
    mDialog = do_CreateInstance( NS_IHELPERAPPLAUNCHERDLG_CONTRACTID, &rv );
    NS_ENSURE_SUCCESS(rv, rv);
  }

  // we want to explicitly unescape aDefaultFile b4 passing into the dialog. we can't unescape
  // it because the dialog is implemented by a JS component which doesn't have a window so no unescape routine is defined...

  // Now, be sure to keep |this| alive, and the dialog
  // If we don't do this, users that close the helper app dialog while the file
  // picker is up would cause Cancel() to be called, and the dialog would be
  // released, which would release this object too, which would crash.
  // See Bug 249143
  nsRefPtr<nsExternalAppHandler> kungFuDeathGrip(this);
  nsCOMPtr<nsIHelperAppLauncherDialog> dlg(mDialog);
  rv = mDialog->PromptForSaveToFile(this, 
                                    mWindowContext,
                                    aDefaultFile.get(),
                                    aFileExtension.get(),
                                    aNewFile);
  return rv;
}

nsresult nsExternalAppHandler::MoveFile(nsIFile * aNewFileLocation)
{
  nsresult rv = NS_OK;
  NS_ASSERTION(mStopRequestIssued, "uhoh, how did we get here if we aren't done getting data?");
 
  nsCOMPtr<nsILocalFile> fileToUse = do_QueryInterface(aNewFileLocation);

  // if the on stop request was actually issued then it's now time to actually perform the file move....
  if (mStopRequestIssued && fileToUse)
  {
    // Unfortunately, MoveTo will fail if a file already exists at the user specified location....
    // but the user has told us, this is where they want the file! (when we threw up the save to file dialog,
    // it told them the file already exists and do they wish to over write it. So it should be okay to delete
    // fileToUse if it already exists.
    PRBool equalToTempFile = PR_FALSE;
    PRBool filetoUseAlreadyExists = PR_FALSE;
    fileToUse->Equals(mTempFile, &equalToTempFile);
    fileToUse->Exists(&filetoUseAlreadyExists);
    if (filetoUseAlreadyExists && !equalToTempFile)
      fileToUse->Remove(PR_FALSE);

     // extract the new leaf name from the file location
     nsAutoString fileName;
     fileToUse->GetLeafName(fileName);
     nsCOMPtr<nsIFile> directoryLocation;
     rv = fileToUse->GetParent(getter_AddRefs(directoryLocation));
     if (directoryLocation)
     {
       rv = mTempFile->MoveTo(directoryLocation, fileName);
     }
     if (NS_FAILED(rv))
     {
       // Send error notification.        
       nsAutoString path;
       fileToUse->GetPath(path);
       SendStatusChange(kWriteError, rv, nsnull, path);
       Cancel(rv); // Cancel (and clean up temp file).
     }
#if defined(XP_OS2)
     else
     {
       // tag the file with its source URI
       nsCOMPtr<nsILocalFileOS2> localFileOS2 = do_QueryInterface(fileToUse);
       if (localFileOS2)
       {
         nsCAutoString url;
         mSourceUrl->GetSpec(url);
         localFileOS2->SetFileSource(url);
       }
     }
#endif
  }

  return rv;
}

// SaveToDisk should only be called by the helper app dialog which allows
// the user to say launch with application or save to disk. It doesn't actually 
// perform the save, it just prompts for the destination file name. The actual save
// won't happen until we are done downloading the content and are sure we've 
// shown a progress dialog. This was done to simplify the 
// logic that was showing up in this method. Internal callers who actually want
// to preform the save should call ::MoveFile

NS_IMETHODIMP nsExternalAppHandler::SaveToDisk(nsIFile * aNewFileLocation, PRBool aRememberThisPreference)
{
  nsresult rv = NS_OK;
  if (mCanceled)
    return NS_OK;

  mMimeInfo->SetPreferredAction(nsIMIMEInfo::saveToDisk);

  // The helper app dialog has told us what to do.
  mReceivedDispositionInfo = PR_TRUE;

  nsCOMPtr<nsILocalFile> fileToUse = do_QueryInterface(aNewFileLocation);
  if (!fileToUse)
  {
    nsAutoString leafName;
    mTempFile->GetLeafName(leafName);
    if (mSuggestedFileName.IsEmpty())
      rv = PromptForSaveToFile(getter_AddRefs(fileToUse), leafName, mTempFileExtension);
    else
    {
      nsAutoString fileExt;
      PRInt32 pos = mSuggestedFileName.RFindChar('.');
      if (pos >= 0)
        mSuggestedFileName.Right(fileExt, mSuggestedFileName.Length() - pos);
      if (fileExt.IsEmpty())
        fileExt = mTempFileExtension;

      rv = PromptForSaveToFile(getter_AddRefs(fileToUse), mSuggestedFileName, fileExt);
    }

    if (NS_FAILED(rv) || !fileToUse) {
      Cancel(NS_BINDING_ABORTED);
      return NS_ERROR_FAILURE;
    }
  }
  
  mFinalFileDestination = do_QueryInterface(fileToUse);

  // Move what we have in the final directory, but append .part
  // to it, to indicate that it's unfinished.
  // do not do that if we're already done
  if (mFinalFileDestination && !mStopRequestIssued)
  {
    nsCOMPtr<nsIFile> movedFile;
    mFinalFileDestination->Clone(getter_AddRefs(movedFile));
    if (movedFile) {
      // Get the old leaf name and append .part to it
      nsAutoString name;
      mFinalFileDestination->GetLeafName(name);
      name.AppendLiteral(".part");
      movedFile->SetLeafName(name);

      nsCOMPtr<nsIFile> dir;
      movedFile->GetParent(getter_AddRefs(dir));

      mOutStream->Close();

      rv = mTempFile->MoveTo(dir, name);
      if (NS_SUCCEEDED(rv)) // if it failed, we just continue with $TEMP
        mTempFile = movedFile;

      nsCOMPtr<nsIOutputStream> outputStream;
      rv = NS_NewLocalFileOutputStream(getter_AddRefs(outputStream), mTempFile,
                                       PR_WRONLY | PR_APPEND, 0600);
      if (NS_FAILED(rv)) { // (Re-)opening the output stream failed. bad luck.
        nsAutoString path;
        mTempFile->GetPath(path);
        SendStatusChange(kWriteError, rv, nsnull, path);
        Cancel(rv);
        return NS_OK;
      }

      mOutStream = NS_BufferOutputStream(outputStream, BUFFERED_OUTPUT_SIZE);
    }
  }

  if (!mProgressListenerInitialized)
    CreateProgressListener();

  // now that the user has chosen the file location to save to, it's okay to fire the refresh tag
  // if there is one. We don't want to do this before the save as dialog goes away because this dialog
  // is modal and we do bad things if you try to load a web page in the underlying window while a modal
  // dialog is still up. 
  ProcessAnyRefreshTags();

  return NS_OK;
}


nsresult nsExternalAppHandler::OpenWithApplication()
{
  nsresult rv = NS_OK;
  if (mCanceled)
    return NS_OK;
  
  // we only should have gotten here if the on stop request had been fired already.

  NS_ASSERTION(mStopRequestIssued, "uhoh, how did we get here if we aren't done getting data?");
  // if a stop request was already issued then proceed with launching the application.
  if (mStopRequestIssued)
  {
    PRBool deleteTempFileOnExit;
    nsCOMPtr<nsIPrefBranch> prefs(do_GetService(NS_PREFSERVICE_CONTRACTID));
    if (!prefs || NS_FAILED(prefs->GetBoolPref(
        "browser.helperApps.deleteTempFileOnExit", &deleteTempFileOnExit))) {
      // No prefservice or no pref set; use default value
#if !defined(XP_MACOSX)
      // Mac users have been very verbal about temp files being deleted on
      // app exit - they don't like it - but we'll continue to do this on
      // other platforms for now.
      deleteTempFileOnExit = PR_TRUE;
#else
      deleteTempFileOnExit = PR_FALSE;
#endif
    }

    // make the tmp file readonly so users won't edit it and lose the changes
    // only if we're going to delete the file
    if (deleteTempFileOnExit)
      mFinalFileDestination->SetPermissions(0400);

    rv = mMimeInfo->LaunchWithFile(mFinalFileDestination);        
    if (NS_FAILED(rv))
    {
      // Send error notification.
      nsAutoString path;
      mFinalFileDestination->GetPath(path);
      SendStatusChange(kLaunchError, rv, nsnull, path);
      Cancel(rv); // Cancel, and clean up temp file.
    }
    else if (deleteTempFileOnExit) {
      NS_ASSERTION(gExtProtSvc, "Service gone away!?");
      gExtProtSvc->DeleteTemporaryFileOnExit(mFinalFileDestination);
    }
  }

  return rv;
}

// LaunchWithApplication should only be called by the helper app dialog which allows
// the user to say launch with application or save to disk. It doesn't actually 
// perform launch with application. That won't happen until we are done downloading
// the content and are sure we've showna progress dialog. This was done to simplify the 
// logic that was showing up in this method. 
NS_IMETHODIMP nsExternalAppHandler::LaunchWithApplication(nsIFile * aApplication, PRBool aRememberThisPreference)
{
  if (mCanceled)
    return NS_OK;

  // user has chosen to launch using an application, fire any refresh tags now...
  ProcessAnyRefreshTags(); 
  
  mReceivedDispositionInfo = PR_TRUE; 
  if (mMimeInfo && aApplication) {
    PlatformLocalHandlerApp_t *handlerApp =
      new PlatformLocalHandlerApp_t(EmptyString(), aApplication);
    mMimeInfo->SetPreferredApplicationHandler(handlerApp);
  }

  // Now check if the file is local, in which case we won't bother with saving
  // it to a temporary directory and just launch it from where it is
  nsCOMPtr<nsIFileURL> fileUrl(do_QueryInterface(mSourceUrl));
  if (fileUrl && mIsFileChannel)
  {
    Cancel(NS_BINDING_ABORTED);
    nsCOMPtr<nsIFile> file;
    nsresult rv = fileUrl->GetFile(getter_AddRefs(file));

    if (NS_SUCCEEDED(rv))
    {
      rv = mMimeInfo->LaunchWithFile(file);
      if (NS_SUCCEEDED(rv))
        return NS_OK;
    }
    nsAutoString path;
    if (file)
      file->GetPath(path);
    // If we get here, an error happened
    SendStatusChange(kLaunchError, rv, nsnull, path);
    return rv;
  }

  // Now that the user has elected to launch the downloaded file with a helper app, we're justified in
  // removing the 'salted' name.  We'll rename to what was specified in mSuggestedFileName after the
  // download is done prior to launching the helper app.  So that any existing file of that name won't
  // be overwritten we call CreateUnique() before calling MoveFile().  Also note that we use the same
  // directory as originally downloaded to so that MoveFile() just does an in place rename.
   
  nsCOMPtr<nsIFile> fileToUse;
  
  // The directories specified here must match those specified in SetUpTempFile().  This avoids
  // having to do a copy of the file when it finishes downloading and the potential for errors
  // that would introduce
#ifdef XP_MACOSX
  NS_GetSpecialDirectory(NS_MAC_DEFAULT_DOWNLOAD_DIR, getter_AddRefs(fileToUse));
#else
  NS_GetSpecialDirectory(NS_OS_TEMP_DIR, getter_AddRefs(fileToUse));
#endif

  if (mSuggestedFileName.IsEmpty())
  {
    // Keep using the leafname of the temp file, since we're just starting a helper
    mTempFile->GetLeafName(mSuggestedFileName);
  }

#ifdef XP_WIN
  fileToUse->Append(mSuggestedFileName + mTempFileExtension);
#else
  fileToUse->Append(mSuggestedFileName);  
#endif
  
  // We'll make sure this results in a unique name later

  mFinalFileDestination = do_QueryInterface(fileToUse);

  // launch the progress window now that the user has picked the desired action.
  if (!mProgressListenerInitialized)
   CreateProgressListener();

  return NS_OK;
}

NS_IMETHODIMP nsExternalAppHandler::Cancel(nsresult aReason)
{
  NS_ENSURE_ARG(NS_FAILED(aReason));
  // XXX should not ignore the reason

  mCanceled = PR_TRUE;
  // Break our reference cycle with the helper app dialog (set up in
  // OnStartRequest)
  mDialog = nsnull;
  // shutdown our stream to the temp file
  if (mOutStream)
  {
    mOutStream->Close();
    mOutStream = nsnull;
  }

  // Clean up after ourselves and delete the temp file only if the user
  // canceled the helper app dialog (we didn't get the disposition info yet).
  // We leave the partial file for everything else because it could be useful
  // e.g., resume a download
  if (mTempFile && !mReceivedDispositionInfo)
  {
    mTempFile->Remove(PR_FALSE);
    mTempFile = nsnull;
  }

  // Release the listener, to break the reference cycle with it (we are the
  // observer of the listener).
  mWebProgressListener = nsnull;

  return NS_OK;
}

void nsExternalAppHandler::ProcessAnyRefreshTags()
{
   // one last thing, try to see if the original window context supports a refresh interface...
   // Sometimes, when you download content that requires an external handler, there is
   // a refresh header associated with the download. This refresh header points to a page
   // the content provider wants the user to see after they download the content. How do we
   // pass this refresh information back to the caller? For now, try to get the refresh URI 
   // interface. If the window context where the request originated came from supports this
   // then we can force it to process the refresh information (if there is any) from this channel.
   if (mWindowContext && mOriginalChannel)
   {
     nsCOMPtr<nsIRefreshURI> refreshHandler (do_GetInterface(mWindowContext));
     if (refreshHandler) {
        refreshHandler->SetupRefreshURI(mOriginalChannel);
     }
     mOriginalChannel = nsnull;
   }
}

PRBool nsExternalAppHandler::GetNeverAskFlagFromPref(const char * prefName, const char * aContentType)
{
  // Search the obsolete pref strings.
  nsresult rv;
  nsCOMPtr<nsIPrefService> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
  nsCOMPtr<nsIPrefBranch> prefBranch;
  if (prefs)
    rv = prefs->GetBranch(NEVER_ASK_PREF_BRANCH, getter_AddRefs(prefBranch));
  if (NS_SUCCEEDED(rv) && prefBranch)
  {
    nsXPIDLCString prefCString;
    nsCAutoString prefValue;
    rv = prefBranch->GetCharPref(prefName, getter_Copies(prefCString));
    if (NS_SUCCEEDED(rv) && !prefCString.IsEmpty())
    {
      NS_UnescapeURL(prefCString);
      nsACString::const_iterator start, end;
      prefCString.BeginReading(start);
      prefCString.EndReading(end);
      if (CaseInsensitiveFindInReadable(nsDependentCString(aContentType), start, end))
        return PR_FALSE;
    }
  }
  // Default is true, if not found in the pref string.
  return PR_TRUE;
}

nsresult nsExternalAppHandler::MaybeCloseWindow()
{
  nsCOMPtr<nsIDOMWindow> window(do_GetInterface(mWindowContext));
  nsCOMPtr<nsIDOMWindowInternal> internalWindow = do_QueryInterface(window);
  NS_ENSURE_STATE(internalWindow);

  if (mShouldCloseWindow) {
    // Reset the window context to the opener window so that the dependent
    // dialogs have a parent
    nsCOMPtr<nsIDOMWindowInternal> opener;
    internalWindow->GetOpener(getter_AddRefs(opener));

    PRBool isClosed;
    if (opener && NS_SUCCEEDED(opener->GetClosed(&isClosed)) && !isClosed) {
      mWindowContext = do_GetInterface(opener);

      // Now close the old window.  Do it on a timer so that we don't run
      // into issues trying to close the window before it has fully opened.
      NS_ASSERTION(!mTimer, "mTimer was already initialized once!");
      mTimer = do_CreateInstance("@mozilla.org/timer;1");
      if (!mTimer) {
        return NS_ERROR_FAILURE;
      }

      mTimer->InitWithCallback(this, 0, nsITimer::TYPE_ONE_SHOT);
      mWindowToClose = internalWindow;
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsExternalAppHandler::Notify(nsITimer* timer)
{
  NS_ASSERTION(mWindowToClose, "No window to close after timer fired");

  mWindowToClose->Close();
  mWindowToClose = nsnull;
  mTimer = nsnull;

  return NS_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The following section contains our nsIMIMEService implementation and related methods.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// nsIMIMEService methods
NS_IMETHODIMP nsExternalHelperAppService::GetFromTypeAndExtension(const nsACString& aMIMEType, const nsACString& aFileExt, nsIMIMEInfo **_retval) 
{
  NS_PRECONDITION(!aMIMEType.IsEmpty() ||
                  !aFileExt.IsEmpty(), 
                  "Give me something to work with");
  LOG(("Getting mimeinfo from type '%s' ext '%s'\n",
        PromiseFlatCString(aMIMEType).get(), PromiseFlatCString(aFileExt).get()));

  *_retval = nsnull;

  // OK... we need a type. Get one.
  nsCAutoString typeToUse(aMIMEType);
  if (typeToUse.IsEmpty()) {
    nsresult rv = GetTypeFromExtension(aFileExt, typeToUse);
    if (NS_FAILED(rv))
      return NS_ERROR_NOT_AVAILABLE;
  }

  // We promise to only send lower case mime types to the OS
  ToLowerCase(typeToUse);

  // (1) Ask the OS for a mime info
  PRBool found;
  *_retval = GetMIMEInfoFromOS(typeToUse, aFileExt, &found).get();
  LOG(("OS gave back 0x%p - found: %i\n", *_retval, found));
  // If we got no mimeinfo, something went wrong. Probably lack of memory.
  if (!*_retval)
    return NS_ERROR_OUT_OF_MEMORY;

  // (2) Now, let's see if we can find something in our datastore
  // This will not overwrite the OS information that interests us
  // (i.e. default application, default app. description)
  nsresult rv;
  nsCOMPtr<nsIHandlerService> handlerSvc = do_GetService(NS_HANDLERSERVICE_CONTRACTID);
  if (handlerSvc) {
    rv = handlerSvc->FillHandlerInfo(*_retval, EmptyCString());
    LOG(("Data source: Via type: retval 0x%08x\n", rv));
    found = found || NS_SUCCEEDED(rv);

    if (!found || NS_FAILED(rv)) {
      // No type match, try extension match
      if (!aFileExt.IsEmpty()) {
        nsCAutoString overrideType;
        rv = handlerSvc->GetTypeFromExtension(aFileExt, overrideType);
        if (NS_SUCCEEDED(rv)) {
          rv = handlerSvc->FillHandlerInfo(*_retval, overrideType);
          LOG(("Data source: Via ext: retval 0x%08x\n", rv));
          found = found || NS_SUCCEEDED(rv);
        }
      }
    }
  }

  // (3) No match yet. Ask extras.
  if (!found) {
    rv = NS_ERROR_FAILURE;
#ifdef XP_WIN
    /* XXX Gross hack to wallpaper over the most common Win32
     * extension issues caused by the fix for bug 116938.  See bug
     * 120327, comment 271 for why this is needed.  Not even sure we
     * want to remove this once we have fixed all this stuff to work
     * right; any info we get from extras on this type is pretty much
     * useless....
     */
    if (!typeToUse.Equals(APPLICATION_OCTET_STREAM, nsCaseInsensitiveCStringComparator()))
#endif
      rv = FillMIMEInfoForMimeTypeFromExtras(typeToUse, *_retval);
    LOG(("Searched extras (by type), rv 0x%08X\n", rv));
    // If that didn't work out, try file extension from extras
    if (NS_FAILED(rv) && !aFileExt.IsEmpty()) {
      rv = FillMIMEInfoForExtensionFromExtras(aFileExt, *_retval);
      LOG(("Searched extras (by ext), rv 0x%08X\n", rv));
    }
  }

  // Finally, check if we got a file extension and if yes, if it is an
  // extension on the mimeinfo, in which case we want it to be the primary one
  if (!aFileExt.IsEmpty()) {
    PRBool matches = PR_FALSE;
    (*_retval)->ExtensionExists(aFileExt, &matches);
    LOG(("Extension '%s' matches mime info: %i\n", PromiseFlatCString(aFileExt).get(), matches));
    if (matches)
      (*_retval)->SetPrimaryExtension(aFileExt);
  }

#ifdef PR_LOGGING
  if (LOG_ENABLED()) {
    nsCAutoString type;
    (*_retval)->GetMIMEType(type);

    nsCAutoString ext;
    (*_retval)->GetPrimaryExtension(ext);
    LOG(("MIME Info Summary: Type '%s', Primary Ext '%s'\n", type.get(), ext.get()));
  }
#endif

  return NS_OK;
}

NS_IMETHODIMP nsExternalHelperAppService::GetTypeFromExtension(const nsACString& aFileExt, nsACString& aContentType) 
{
  // OK. We want to try the following sources of mimetype information, in this order:
  // 1. defaultMimeEntries array
  // 2. User-set preferences (managed by the handler service)
  // 3. OS-provided information
  // 4. our "extras" array
  // 5. Information from plugins
  // 6. The "ext-to-type-mapping" category

  nsresult rv = NS_OK;
  // First of all, check our default entries
  for (size_t i = 0; i < NS_ARRAY_LENGTH(defaultMimeEntries); i++)
  {
    if (aFileExt.LowerCaseEqualsASCII(defaultMimeEntries[i].mFileExtension)) {
      aContentType = defaultMimeEntries[i].mMimeType;
      return rv;
    }
  }

  // Check user-set prefs
  nsCOMPtr<nsIHandlerService> handlerSvc = do_GetService(NS_HANDLERSERVICE_CONTRACTID);
  if (handlerSvc)
    rv = handlerSvc->GetTypeFromExtension(aFileExt, aContentType);
  if (NS_SUCCEEDED(rv))
    return NS_OK;

  // Ask OS.
  PRBool found = PR_FALSE;
  nsCOMPtr<nsIMIMEInfo> mi = GetMIMEInfoFromOS(EmptyCString(), aFileExt, &found);
  if (mi && found)
    return mi->GetMIMEType(aContentType);

  // Check extras array.
  found = GetTypeFromExtras(aFileExt, aContentType);
  if (found)
    return NS_OK;

  const nsCString& flatExt = PromiseFlatCString(aFileExt);
  // Try the plugins
  const char* mimeType;
  nsCOMPtr<nsIPluginHost> pluginHost (do_GetService(kPluginManagerCID, &rv));
  if (NS_SUCCEEDED(rv)) {
    if (NS_SUCCEEDED(pluginHost->IsPluginEnabledForExtension(flatExt.get(), mimeType)))
    {
      aContentType = mimeType;
      return NS_OK;
    }
  }
  
  rv = NS_OK;
  // Let's see if an extension added something
  nsCOMPtr<nsICategoryManager> catMan(do_GetService("@mozilla.org/categorymanager;1"));
  if (catMan) {
    nsXPIDLCString type;
    rv = catMan->GetCategoryEntry("ext-to-type-mapping", flatExt.get(), getter_Copies(type));
    aContentType = type;
  }
  else {
    rv = NS_ERROR_NOT_AVAILABLE;
  }
  
  return rv;
}

NS_IMETHODIMP nsExternalHelperAppService::GetPrimaryExtension(const nsACString& aMIMEType, const nsACString& aFileExt, nsACString& _retval)
{
  NS_ENSURE_ARG(!aMIMEType.IsEmpty());

  nsCOMPtr<nsIMIMEInfo> mi;
  nsresult rv = GetFromTypeAndExtension(aMIMEType, aFileExt, getter_AddRefs(mi));
  if (NS_FAILED(rv))
    return rv;

  return mi->GetPrimaryExtension(_retval);
}

NS_IMETHODIMP nsExternalHelperAppService::GetTypeFromURI(nsIURI *aURI, nsACString& aContentType) 
{
  nsresult rv = NS_ERROR_NOT_AVAILABLE;
  aContentType.Truncate();

  // First look for a file to use.  If we have one, we just use that.
  nsCOMPtr<nsIFileURL> fileUrl = do_QueryInterface(aURI);
  if (fileUrl) {
    nsCOMPtr<nsIFile> file;
    rv = fileUrl->GetFile(getter_AddRefs(file));
    if (NS_SUCCEEDED(rv)) {
      rv = GetTypeFromFile(file, aContentType);
      if (NS_SUCCEEDED(rv)) {
        // we got something!
        return rv;
      }
    }
  }

  // Now try to get an nsIURL so we don't have to do our own parsing
  nsCOMPtr<nsIURL> url = do_QueryInterface(aURI);
  if (url) {
    nsCAutoString ext;
    rv = url->GetFileExtension(ext);
    if (NS_FAILED(rv))
      return rv;
    if (ext.IsEmpty())
      return NS_ERROR_NOT_AVAILABLE;

    UnescapeFragment(ext, url, ext);

    return GetTypeFromExtension(ext, aContentType);
  }
    
  // no url, let's give the raw spec a shot
  nsCAutoString specStr;
  rv = aURI->GetSpec(specStr);
  if (NS_FAILED(rv))
    return rv;
  UnescapeFragment(specStr, aURI, specStr);

  // find the file extension (if any)
  PRInt32 extLoc = specStr.RFindChar('.');
  PRInt32 specLength = specStr.Length();
  if (-1 != extLoc &&
      extLoc != specLength - 1 &&
      // nothing over 20 chars long can be sanely considered an
      // extension.... Dat dere would be just data.
      specLength - extLoc < 20) 
  {
    return GetTypeFromExtension(Substring(specStr, extLoc + 1), aContentType);
  }

  // We found no information; say so.
  return NS_ERROR_NOT_AVAILABLE;
}

NS_IMETHODIMP nsExternalHelperAppService::GetTypeFromFile(nsIFile* aFile, nsACString& aContentType)
{
  nsresult rv;
  nsCOMPtr<nsIMIMEInfo> info;

  // Get the Extension
  nsAutoString fileName;
  rv = aFile->GetLeafName(fileName);
  if (NS_FAILED(rv)) return rv;
 
  nsCAutoString fileExt;
  if (!fileName.IsEmpty())
  {
    PRInt32 len = fileName.Length(); 
    for (PRInt32 i = len; i >= 0; i--) 
    {
      if (fileName[i] == PRUnichar('.'))
      {
        CopyUTF16toUTF8(fileName.get() + i + 1, fileExt);
        break;
      }
    }
  }
  
#ifdef XP_MACOSX
  nsCOMPtr<nsILocalFileMac> macFile;
  macFile = do_QueryInterface( aFile, &rv );
  if (NS_SUCCEEDED( rv ) && fileExt.IsEmpty())
  {
    PRUint32 type, creator;
    macFile->GetFileType( (OSType*)&type );
    macFile->GetFileCreator( (OSType*)&creator );   
    nsCOMPtr<nsIInternetConfigService> icService (do_GetService(NS_INTERNETCONFIGSERVICE_CONTRACTID));
    if (icService)
    {
      rv = icService->GetMIMEInfoFromTypeCreator(type, creator, fileExt.get(), getter_AddRefs(info));
      if (NS_SUCCEEDED(rv))
        return info->GetMIMEType(aContentType);
    }
  }
#endif

  // Windows, unix and mac when no type match occured.   
  if (fileExt.IsEmpty())
    return NS_ERROR_FAILURE;    
  return GetTypeFromExtension(fileExt, aContentType);
}

nsresult nsExternalHelperAppService::FillMIMEInfoForMimeTypeFromExtras(
  const nsACString& aContentType, nsIMIMEInfo * aMIMEInfo)
{
  NS_ENSURE_ARG( aMIMEInfo );

  NS_ENSURE_ARG( !aContentType.IsEmpty() );

  // Look for default entry with matching mime type.
  nsCAutoString MIMEType(aContentType);
  ToLowerCase(MIMEType);
  PRInt32 numEntries = NS_ARRAY_LENGTH(extraMimeEntries);
  for (PRInt32 index = 0; index < numEntries; index++)
  {
      if ( MIMEType.Equals(extraMimeEntries[index].mMimeType) )
      {
          // This is the one. Set attributes appropriately.
          aMIMEInfo->SetFileExtensions(nsDependentCString(extraMimeEntries[index].mFileExtensions));
          aMIMEInfo->SetDescription(NS_ConvertASCIItoUTF16(extraMimeEntries[index].mDescription));
          aMIMEInfo->SetMacType(extraMimeEntries[index].mMactype);
          aMIMEInfo->SetMacCreator(extraMimeEntries[index].mMacCreator);

          return NS_OK;
      }
  }

  return NS_ERROR_NOT_AVAILABLE;
}

nsresult nsExternalHelperAppService::FillMIMEInfoForExtensionFromExtras(
  const nsACString& aExtension, nsIMIMEInfo * aMIMEInfo)
{
  nsCAutoString type;
  PRBool found = GetTypeFromExtras(aExtension, type);
  if (!found)
    return NS_ERROR_NOT_AVAILABLE;
  return FillMIMEInfoForMimeTypeFromExtras(type, aMIMEInfo);
}

PRBool nsExternalHelperAppService::GetTypeFromExtras(const nsACString& aExtension, nsACString& aMIMEType)
{
  NS_ASSERTION(!aExtension.IsEmpty(), "Empty aExtension parameter!");

  // Look for default entry with matching extension.
  nsDependentCString::const_iterator start, end, iter;
  PRInt32 numEntries = NS_ARRAY_LENGTH(extraMimeEntries);
  for (PRInt32 index = 0; index < numEntries; index++)
  {
      nsDependentCString extList(extraMimeEntries[index].mFileExtensions);
      extList.BeginReading(start);
      extList.EndReading(end);
      iter = start;
      while (start != end)
      {
          FindCharInReadable(',', iter, end);
          if (Substring(start, iter).Equals(aExtension,
                                            nsCaseInsensitiveCStringComparator()))
          {
              aMIMEType = extraMimeEntries[index].mMimeType;
              return PR_TRUE;
          }
          if (iter != end) {
            ++iter;
          }
          start = iter;
      }
  }

  return PR_FALSE;
}
