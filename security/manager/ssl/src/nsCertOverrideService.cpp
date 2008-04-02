/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Red Hat, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Kai Engert <kengert@redhat.com>
 *   Ehsan Akhgari <ehsan.akhgari@gmail.com>
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

#include "nsCertOverrideService.h"
#include "nsIX509Cert.h"
#include "nsNSSCertificate.h"
#include "nsCRT.h"
#include "nsAppDirectoryServiceDefs.h"
#include "nsStreamUtils.h"
#include "nsNetUtil.h"
#include "nsILineInputStream.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"
#include "nsISupportsPrimitives.h"
#include "nsPromiseFlatString.h"
#include "nsProxiedService.h"
#include "nsStringBuffer.h"
#include "nsAutoLock.h"
#include "nsAutoPtr.h"
#include "nspr.h"
#include "pk11pub.h"
#include "certdb.h"
#include "sechash.h"
#include "ssl.h" // For SSL_ClearSessionCache

#include "nsNSSCleaner.h"
NSSCleanupAutoPtrClass(CERTCertificate, CERT_DestroyCertificate)

static const char kCertOverrideFileName[] = "cert_override.txt";

void
nsCertOverride::convertBitsToString(OverrideBits ob, nsACString &str)
{
  str.Truncate();

  if (ob & ob_Mismatch)
    str.Append('M');

  if (ob & ob_Untrusted)
    str.Append('U');

  if (ob & ob_Time_error)
    str.Append('T');
}

void
nsCertOverride::convertStringToBits(const nsACString &str, OverrideBits &ob)
{
  const nsPromiseFlatCString &flat = PromiseFlatCString(str);
  const char *walk = flat.get();

  ob = ob_None;

  for ( ; *walk; ++walk)
  {
    switch (*walk)
    {
      case 'm':
      case 'M':
        ob = (OverrideBits)(ob | ob_Mismatch);
        break;

      case 'u':
      case 'U':
        ob = (OverrideBits)(ob | ob_Untrusted);
        break;

      case 't':
      case 'T':
        ob = (OverrideBits)(ob | ob_Time_error);
        break;

      default:
        break;
    }
  }
}

NS_IMPL_THREADSAFE_ISUPPORTS3(nsCertOverrideService, 
                              nsICertOverrideService,
                              nsIObserver,
                              nsISupportsWeakReference)

nsCertOverrideService::nsCertOverrideService()
{
  monitor = PR_NewMonitor();
}

nsCertOverrideService::~nsCertOverrideService()
{
  if (monitor)
    PR_DestroyMonitor(monitor);
}

nsresult
nsCertOverrideService::Init()
{
  if (!mSettingsTable.Init())
    return NS_ERROR_OUT_OF_MEMORY;

  mOidTagForStoringNewHashes = SEC_OID_SHA256;

  SECOidData *od = SECOID_FindOIDByTag(mOidTagForStoringNewHashes);
  if (!od)
    return NS_ERROR_FAILURE;

  char *dotted_oid = CERT_GetOidString(&od->oid);
  if (!dotted_oid)
    return NS_ERROR_FAILURE;

  mDottedOidForStoringNewHashes = dotted_oid;
  PR_smprintf_free(dotted_oid);

  // cache mSettingsFile
  NS_GetSpecialDirectory(NS_APP_USER_PROFILE_50_DIR, getter_AddRefs(mSettingsFile));
  if (mSettingsFile) {
    mSettingsFile->AppendNative(NS_LITERAL_CSTRING(kCertOverrideFileName));
  }

  Read();

  nsresult rv;
  NS_WITH_ALWAYS_PROXIED_SERVICE(nsIObserverService, mObserverService,
                                 "@mozilla.org/observer-service;1",
                                 NS_PROXY_TO_MAIN_THREAD, &rv);

  if (mObserverService) {
    mObserverService->AddObserver(this, "profile-before-change", PR_TRUE);
    mObserverService->AddObserver(this, "profile-do-change", PR_TRUE);
    mObserverService->AddObserver(this, "shutdown-cleanse", PR_TRUE);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsCertOverrideService::Observe(nsISupports     *aSubject,
                               const char      *aTopic,
                               const PRUnichar *aData)
{
  // check the topic
  if (!nsCRT::strcmp(aTopic, "profile-before-change")) {
    // The profile is about to change,
    // or is going away because the application is shutting down.

    nsAutoMonitor lock(monitor);

    if (!nsCRT::strcmp(aData, NS_LITERAL_STRING("shutdown-cleanse").get())) {
      RemoveAllFromMemory();
      // delete the storage file
      if (mSettingsFile) {
        mSettingsFile->Remove(PR_FALSE);
      }
    } else {
      RemoveAllFromMemory();
    }

  } else if (!nsCRT::strcmp(aTopic, "profile-do-change")) {
    // The profile has already changed.
    // Now read from the new profile location.
    // we also need to update the cached file location

    nsAutoMonitor lock(monitor);

    nsresult rv = NS_GetSpecialDirectory(NS_APP_USER_PROFILE_50_DIR, getter_AddRefs(mSettingsFile));
    if (NS_SUCCEEDED(rv)) {
      mSettingsFile->AppendNative(NS_LITERAL_CSTRING(kCertOverrideFileName));
    }
    Read();

  }

  return NS_OK;
}

void
nsCertOverrideService::RemoveAllFromMemory()
{
  nsAutoMonitor lock(monitor);
  mSettingsTable.Clear();
}

nsresult
nsCertOverrideService::Read()
{
  nsAutoMonitor lock(monitor);

  nsresult rv;
  nsCOMPtr<nsIInputStream> fileInputStream;
  rv = NS_NewLocalFileInputStream(getter_AddRefs(fileInputStream), mSettingsFile);
  if (NS_FAILED(rv)) {
    return rv;
  }

  nsCOMPtr<nsILineInputStream> lineInputStream = do_QueryInterface(fileInputStream, &rv);
  if (NS_FAILED(rv)) {
    return rv;
  }

  nsCAutoString buffer;
  PRBool isMore = PR_TRUE;
  PRInt32 hostIndex = 0, algoIndex, fingerprintIndex, overrideBitsIndex, dbKeyIndex;

  /* file format is:
   *
   * host:port \t fingerprint-algorithm \t fingerprint \t override-mask \t dbKey
   *
   *   where override-mask is a sequence of characters,
   *     M meaning hostname-Mismatch-override
   *     U meaning Untrusted-override
   *     T meaning Time-error-override (expired/not yet valid) 
   *
   * if this format isn't respected we move onto the next line in the file.
   */

  while (isMore && NS_SUCCEEDED(lineInputStream->ReadLine(buffer, &isMore))) {
    if (buffer.IsEmpty() || buffer.First() == '#') {
      continue;
    }

    // this is a cheap, cheesy way of parsing a tab-delimited line into
    // string indexes, which can be lopped off into substrings. just for
    // purposes of obfuscation, it also checks that each token was found.
    // todo: use iterators?
    if ((algoIndex         = buffer.FindChar('\t', hostIndex)         + 1) == 0 ||
        (fingerprintIndex  = buffer.FindChar('\t', algoIndex)         + 1) == 0 ||
        (overrideBitsIndex = buffer.FindChar('\t', fingerprintIndex)  + 1) == 0 ||
        (dbKeyIndex        = buffer.FindChar('\t', overrideBitsIndex) + 1) == 0) {
      continue;
    }

    const nsASingleFragmentCString &host = Substring(buffer, hostIndex, algoIndex - hostIndex - 1);
    const nsASingleFragmentCString &algo_string = Substring(buffer, algoIndex, fingerprintIndex - algoIndex - 1);
    const nsASingleFragmentCString &fingerprint = Substring(buffer, fingerprintIndex, overrideBitsIndex - fingerprintIndex - 1);
    const nsASingleFragmentCString &bits_string = Substring(buffer, overrideBitsIndex, dbKeyIndex - overrideBitsIndex - 1);
    const nsASingleFragmentCString &db_key = Substring(buffer, dbKeyIndex, buffer.Length() - dbKeyIndex);

    nsCertOverride::OverrideBits bits;
    nsCertOverride::convertStringToBits(bits_string, bits);

    AddEntryToList(host, 
                   PR_FALSE, // not temporary
                   algo_string, fingerprint, bits, db_key);
  }

  return NS_OK;
}

PR_STATIC_CALLBACK(PLDHashOperator)
WriteEntryCallback(nsCertOverrideEntry *aEntry,
                   void *aArg)
{
  static const char kTab[] = "\t";

  nsIOutputStream *rawStreamPtr = (nsIOutputStream *)aArg;

  nsresult rv;

  if (rawStreamPtr && aEntry)
  {
    const nsCertOverride &settings = aEntry->mSettings;
    if (settings.mIsTemporary)
      return PL_DHASH_NEXT;

    nsCAutoString bits_string;
    nsCertOverride::convertBitsToString(settings.mOverrideBits, 
                                            bits_string);

    rawStreamPtr->Write(settings.mHostWithPortUTF8.get(), settings.mHostWithPortUTF8.Length(), &rv);
    rawStreamPtr->Write(kTab, sizeof(kTab) - 1, &rv);
    rawStreamPtr->Write(settings.mFingerprintAlgOID.get(), 
                        settings.mFingerprintAlgOID.Length(), &rv);
    rawStreamPtr->Write(kTab, sizeof(kTab) - 1, &rv);
    rawStreamPtr->Write(settings.mFingerprint.get(), 
                        settings.mFingerprint.Length(), &rv);
    rawStreamPtr->Write(kTab, sizeof(kTab) - 1, &rv);
    rawStreamPtr->Write(bits_string.get(), 
                        bits_string.Length(), &rv);
    rawStreamPtr->Write(kTab, sizeof(kTab) - 1, &rv);
    rawStreamPtr->Write(settings.mDBKey.get(), settings.mDBKey.Length(), &rv);
    rawStreamPtr->Write(NS_LINEBREAK, NS_LINEBREAK_LEN, &rv);
  }

  return PL_DHASH_NEXT;
}

nsresult
nsCertOverrideService::Write()
{
  nsAutoMonitor lock(monitor);

  if (!mSettingsFile) {
    return NS_ERROR_NULL_POINTER;
  }

  nsresult rv;
  nsCOMPtr<nsIOutputStream> fileOutputStream;
  rv = NS_NewSafeLocalFileOutputStream(getter_AddRefs(fileOutputStream),
                                       mSettingsFile,
                                       -1,
                                       0600);
  if (NS_FAILED(rv)) {
    NS_ERROR("failed to open cert_warn_settings.txt for writing");
    return rv;
  }

  // get a buffered output stream 4096 bytes big, to optimize writes
  nsCOMPtr<nsIOutputStream> bufferedOutputStream;
  rv = NS_NewBufferedOutputStream(getter_AddRefs(bufferedOutputStream), fileOutputStream, 4096);
  if (NS_FAILED(rv)) {
    return rv;
  }

  static const char kHeader[] =
      "# PSM Certificate Override Settings file" NS_LINEBREAK
      "# This is a generated file!  Do not edit." NS_LINEBREAK;

  /* see ::Read for file format */

  bufferedOutputStream->Write(kHeader, sizeof(kHeader) - 1, &rv);

  nsIOutputStream *rawStreamPtr = bufferedOutputStream;
  mSettingsTable.EnumerateEntries(WriteEntryCallback, rawStreamPtr);

  // All went ok. Maybe except for problems in Write(), but the stream detects
  // that for us
  nsCOMPtr<nsISafeOutputStream> safeStream = do_QueryInterface(bufferedOutputStream);
  NS_ASSERTION(safeStream, "expected a safe output stream!");
  if (safeStream) {
    rv = safeStream->Finish();
    if (NS_FAILED(rv)) {
      NS_WARNING("failed to save cert warn settings file! possible dataloss");
      return rv;
    }
  }

  return NS_OK;
}

static nsresult
GetCertFingerprintByOidTag(CERTCertificate* nsscert,
                           SECOidTag aOidTag, 
                           nsCString &fp)
{
  unsigned int hash_len = HASH_ResultLenByOidTag(aOidTag);
  nsRefPtr<nsStringBuffer> fingerprint = nsStringBuffer::Alloc(hash_len);
  if (!fingerprint)
    return NS_ERROR_OUT_OF_MEMORY;

  PK11_HashBuf(aOidTag, (unsigned char*)fingerprint->Data(), 
               nsscert->derCert.data, nsscert->derCert.len);

  SECItem fpItem;
  fpItem.data = (unsigned char*)fingerprint->Data();
  fpItem.len = hash_len;

  fp.Adopt(CERT_Hexify(&fpItem, 1));
  return NS_OK;
}

static nsresult
GetCertFingerprintByOidTag(nsIX509Cert *aCert,
                           SECOidTag aOidTag, 
                           nsCString &fp)
{
  nsCOMPtr<nsIX509Cert2> cert2 = do_QueryInterface(aCert);
  if (!cert2)
    return NS_ERROR_FAILURE;

  CERTCertificate* nsscert = cert2->GetCert();
  if (!nsscert)
    return NS_ERROR_FAILURE;

  CERTCertificateCleaner nsscertCleaner(nsscert);
  return GetCertFingerprintByOidTag(nsscert, aOidTag, fp);
}

static nsresult
GetCertFingerprintByDottedOidString(CERTCertificate* nsscert,
                                    const nsCString &dottedOid, 
                                    nsCString &fp)
{
  SECItem oid;
  oid.data = nsnull;
  oid.len = 0;
  SECStatus srv = SEC_StringToOID(nsnull, &oid, 
                    dottedOid.get(), dottedOid.Length());
  if (srv != SECSuccess)
    return NS_ERROR_FAILURE;

  SECOidTag oid_tag = SECOID_FindOIDTag(&oid);
  SECITEM_FreeItem(&oid, PR_FALSE);

  if (oid_tag == SEC_OID_UNKNOWN)
    return NS_ERROR_FAILURE;

  return GetCertFingerprintByOidTag(nsscert, oid_tag, fp);
}

static nsresult
GetCertFingerprintByDottedOidString(nsIX509Cert *aCert,
                                    const nsCString &dottedOid, 
                                    nsCString &fp)
{
  nsCOMPtr<nsIX509Cert2> cert2 = do_QueryInterface(aCert);
  if (!cert2)
    return NS_ERROR_FAILURE;

  CERTCertificate* nsscert = cert2->GetCert();
  if (!nsscert)
    return NS_ERROR_FAILURE;

  CERTCertificateCleaner nsscertCleaner(nsscert);
  return GetCertFingerprintByDottedOidString(nsscert, dottedOid, fp);
}

NS_IMETHODIMP
nsCertOverrideService::RememberValidityOverride(const nsAString & aHostNameWithPort, 
                                                nsIX509Cert *aCert,
                                                PRUint32 aOverrideBits, 
                                                PRBool aTemporary)
{
  NS_ENSURE_ARG_POINTER(aCert);
  if (aHostNameWithPort.IsEmpty())
    return NS_ERROR_INVALID_ARG;

  nsCOMPtr<nsIX509Cert2> cert2 = do_QueryInterface(aCert);
  if (!cert2)
    return NS_ERROR_FAILURE;

  CERTCertificate* nsscert = cert2->GetCert();
  if (!nsscert)
    return NS_ERROR_FAILURE;

  CERTCertificateCleaner nsscertCleaner(nsscert);

  nsCAutoString nickname;
  nickname = nsNSSCertificate::defaultServerNickname(nsscert);
  if (nickname.IsEmpty())
    return NS_ERROR_FAILURE;

  PK11SlotInfo *slot = PK11_GetInternalKeySlot();
  if (!slot)
    return NS_ERROR_FAILURE;

  SECStatus srv = PK11_ImportCert(slot, nsscert, CK_INVALID_HANDLE, 
                                  const_cast<char*>(nickname.get()), PR_FALSE);
  PK11_FreeSlot(slot);

  if (srv != SECSuccess)
    return NS_ERROR_FAILURE;

  nsCString myHostPort;
  myHostPort = NS_ConvertUTF16toUTF8(aHostNameWithPort);

  PRInt32 find_colon = myHostPort.FindChar(':');
  if (find_colon == -1) {
    myHostPort.AppendLiteral(":443");
  }

  nsCAutoString fpStr;
  nsresult rv = GetCertFingerprintByOidTag(nsscert, 
                  mOidTagForStoringNewHashes, fpStr);
  if (NS_FAILED(rv))
    return rv;

  char *dbkey = NULL;
  rv = aCert->GetDbKey(&dbkey);
  if (NS_FAILED(rv) || !dbkey)
    return rv;

  // change \n and \r to spaces in the possibly multi-line-base64-encoded key
  for (char *dbkey_walk = dbkey;
       *dbkey_walk;
      ++dbkey_walk) {
    char c = *dbkey_walk;
    if (c == '\r' || c == '\n') {
      *dbkey_walk = ' ';
    }
  }

  {
    nsAutoMonitor lock(monitor);
    AddEntryToList(myHostPort,
                   aTemporary, 
                   mDottedOidForStoringNewHashes, fpStr, 
                   (nsCertOverride::OverrideBits)aOverrideBits, 
                   nsDependentCString(dbkey));
    Write();
  }

  PR_Free(dbkey);
  return NS_OK;
}

NS_IMETHODIMP
nsCertOverrideService::HasMatchingOverride(const nsAString & aHostNameWithPort, 
                                           nsIX509Cert *aCert, 
                                           PRUint32 *aOverrideBits,
                                           PRBool *aIsTemporary,
                                           PRBool *_retval)
{
  if (aHostNameWithPort.IsEmpty())
    return NS_ERROR_INVALID_ARG;

  NS_ENSURE_ARG_POINTER(aCert);
  NS_ENSURE_ARG_POINTER(aOverrideBits);
  NS_ENSURE_ARG_POINTER(aIsTemporary);
  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = PR_FALSE;
  *aOverrideBits = nsCertOverride::ob_None;

  NS_ConvertUTF16toUTF8 hp8(aHostNameWithPort);
  nsCertOverride settings;

  {
    nsAutoMonitor lock(monitor);
    nsCertOverrideEntry *entry = mSettingsTable.GetEntry(hp8.get());
  
    if (!entry)
      return NS_OK;
  
    settings = entry->mSettings; // copy
  }

  *aOverrideBits = settings.mOverrideBits;
  *aIsTemporary = settings.mIsTemporary;

  nsCAutoString fpStr;
  nsresult rv;

  if (settings.mFingerprintAlgOID.Equals(mDottedOidForStoringNewHashes)) {
    rv = GetCertFingerprintByOidTag(aCert, mOidTagForStoringNewHashes, fpStr);
  }
  else {
    rv = GetCertFingerprintByDottedOidString(aCert, settings.mFingerprintAlgOID, fpStr);
  }
  if (NS_FAILED(rv))
    return rv;

  *_retval = settings.mFingerprint.Equals(fpStr);
  return NS_OK;
}

NS_IMETHODIMP
nsCertOverrideService::GetValidityOverride(const nsAString & aHostNameWithPort, 
                                           nsACString & aHashAlg, 
                                           nsACString & aFingerprint, 
                                           PRUint32 *aOverrideBits,
                                           PRBool *aIsTemporary,
                                           PRBool *_found)
{
  NS_ENSURE_ARG_POINTER(_found);
  NS_ENSURE_ARG_POINTER(aIsTemporary);
  NS_ENSURE_ARG_POINTER(aOverrideBits);
  *_found = PR_FALSE;
  *aOverrideBits = nsCertOverride::ob_None;

  NS_ConvertUTF16toUTF8 hp8(aHostNameWithPort);
  nsCertOverride settings;

  {
    nsAutoMonitor lock(monitor);
    nsCertOverrideEntry *entry = mSettingsTable.GetEntry(hp8.get());
  
    if (entry) {
      *_found = PR_TRUE;
      settings = entry->mSettings; // copy
    }
  }

  if (*_found) {
    *aOverrideBits = settings.mOverrideBits;
    *aIsTemporary = settings.mIsTemporary;
    aFingerprint = settings.mFingerprint;
    aHashAlg = settings.mFingerprintAlgOID;
  }

  return NS_OK;
}

nsresult
nsCertOverrideService::AddEntryToList(const nsACString &hostWithPortUTF8, 
                                      const PRBool aIsTemporary,
                                      const nsACString &fingerprintAlgOID, 
                                      const nsACString &fingerprint,
                                      nsCertOverride::OverrideBits ob,
                                      const nsACString &dbKey)
{
  const nsPromiseFlatCString &flat = PromiseFlatCString(hostWithPortUTF8);

  {
    nsAutoMonitor lock(monitor);
    nsCertOverrideEntry *entry = mSettingsTable.PutEntry(flat.get());

    if (!entry) {
      NS_ERROR("can't insert a null entry!");
      return NS_ERROR_OUT_OF_MEMORY;
    }

    nsCertOverride &settings = entry->mSettings;
    settings.mHostWithPortUTF8 = hostWithPortUTF8;
    settings.mIsTemporary = aIsTemporary;
    settings.mFingerprintAlgOID = fingerprintAlgOID;
    settings.mFingerprint = fingerprint;
    settings.mOverrideBits = ob;
    settings.mDBKey = dbKey;
  }

  return NS_OK;
}

NS_IMETHODIMP
nsCertOverrideService::ClearValidityOverride(const nsAString & aHostNameWithPort)
{
  NS_ConvertUTF16toUTF8 hp8(aHostNameWithPort);
  {
    nsAutoMonitor lock(monitor);
    mSettingsTable.RemoveEntry(hp8.get());
    Write();
  }
  SSL_ClearSessionCache();
  return NS_OK;
}

NS_IMETHODIMP
nsCertOverrideService::GetAllOverrideHostsWithPorts(PRUint32 *aCount, 
                                                        PRUnichar ***aHostsWithPortsArray)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

static PRBool
matchesDBKey(nsIX509Cert *cert, const char *match_dbkey)
{
  char *dbkey = NULL;
  nsresult rv = cert->GetDbKey(&dbkey);
  if (NS_FAILED(rv) || !dbkey)
    return PR_FALSE;

  PRBool found_mismatch = PR_FALSE;
  const char *key1 = dbkey;
  const char *key2 = match_dbkey;

  // skip over any whitespace when comparing
  while (*key1 && *key2) {
    char c1 = *key1;
    char c2 = *key2;
    
    switch (c1) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        ++key1;
        continue;
    }

    switch (c2) {
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        ++key2;
        continue;
    }

    if (c1 != c2) {
      found_mismatch = PR_TRUE;
      break;
    }

    ++key1;
    ++key2;
  }

  PR_Free(dbkey);
  return !found_mismatch;
}

struct nsCertAndBoolsAndInt
{
  nsIX509Cert *cert;
  PRBool aCheckTemporaries;
  PRBool aCheckPermanents;
  PRUint32 counter;

  SECOidTag mOidTagForStoringNewHashes;
  nsCString mDottedOidForStoringNewHashes;
};

PR_STATIC_CALLBACK(PLDHashOperator)
FindMatchingCertCallback(nsCertOverrideEntry *aEntry,
                         void *aArg)
{
  nsCertAndBoolsAndInt *cai = (nsCertAndBoolsAndInt *)aArg;

  if (cai && aEntry)
  {
    const nsCertOverride &settings = aEntry->mSettings;
    PRBool still_ok = PR_TRUE;

    if ((settings.mIsTemporary && !cai->aCheckTemporaries)
        ||
        (!settings.mIsTemporary && !cai->aCheckPermanents)) {
      still_ok = PR_FALSE;
    }

    if (still_ok && matchesDBKey(cai->cert, settings.mDBKey.get())) {
      nsCAutoString cert_fingerprint;
      nsresult rv;
      if (settings.mFingerprintAlgOID.Equals(cai->mDottedOidForStoringNewHashes)) {
        rv = GetCertFingerprintByOidTag(cai->cert,
               cai->mOidTagForStoringNewHashes, cert_fingerprint);
      }
      else {
        rv = GetCertFingerprintByDottedOidString(cai->cert,
               settings.mFingerprintAlgOID, cert_fingerprint);
      }
      if (NS_SUCCEEDED(rv) &&
          settings.mFingerprint.Equals(cert_fingerprint)) {
        cai->counter++;
      }
    }
  }

  return PL_DHASH_NEXT;
}

NS_IMETHODIMP
nsCertOverrideService::IsCertUsedForOverrides(nsIX509Cert *aCert, 
                                              PRBool aCheckTemporaries,
                                              PRBool aCheckPermanents,
                                              PRUint32 *_retval)
{
  NS_ENSURE_ARG(aCert);
  NS_ENSURE_ARG(_retval);

  nsCertAndBoolsAndInt cai;
  cai.cert = aCert;
  cai.aCheckTemporaries = aCheckTemporaries;
  cai.aCheckPermanents = aCheckPermanents;
  cai.counter = 0;
  cai.mOidTagForStoringNewHashes = mOidTagForStoringNewHashes;
  cai.mDottedOidForStoringNewHashes = mDottedOidForStoringNewHashes;

  {
    nsAutoMonitor lock(monitor);
    mSettingsTable.EnumerateEntries(FindMatchingCertCallback, &cai);
  }
  *_retval = cai.counter;
  return NS_OK;
}

struct nsCertAndPointerAndCallback
{
  nsIX509Cert *cert;
  void *userdata;
  nsCertOverrideService::CertOverrideEnumerator enumerator;

  SECOidTag mOidTagForStoringNewHashes;
  nsCString mDottedOidForStoringNewHashes;
};

PR_STATIC_CALLBACK(PLDHashOperator)
EnumerateCertOverridesCallback(nsCertOverrideEntry *aEntry,
                               void *aArg)
{
  nsCertAndPointerAndCallback *capac = (nsCertAndPointerAndCallback *)aArg;

  if (capac && aEntry)
  {
    const nsCertOverride &settings = aEntry->mSettings;

    if (!capac->cert) {
      (*capac->enumerator)(settings, capac->userdata);
    }
    else {
      if (matchesDBKey(capac->cert, settings.mDBKey.get())) {
        nsCAutoString cert_fingerprint;
        nsresult rv;
        if (settings.mFingerprintAlgOID.Equals(capac->mDottedOidForStoringNewHashes)) {
          rv = GetCertFingerprintByOidTag(capac->cert,
                 capac->mOidTagForStoringNewHashes, cert_fingerprint);
        }
        else {
          rv = GetCertFingerprintByDottedOidString(capac->cert,
                 settings.mFingerprintAlgOID, cert_fingerprint);
        }
        if (NS_SUCCEEDED(rv) &&
            settings.mFingerprint.Equals(cert_fingerprint)) {
          (*capac->enumerator)(settings, capac->userdata);
        }
      }
    }
  }

  return PL_DHASH_NEXT;
}

nsresult 
nsCertOverrideService::EnumerateCertOverrides(nsIX509Cert *aCert,
                         CertOverrideEnumerator enumerator,
                         void *aUserData)
{
  nsCertAndPointerAndCallback capac;
  capac.cert = aCert;
  capac.userdata = aUserData;
  capac.enumerator = enumerator;
  capac.mOidTagForStoringNewHashes = mOidTagForStoringNewHashes;
  capac.mDottedOidForStoringNewHashes = mDottedOidForStoringNewHashes;

  {
    nsAutoMonitor lock(monitor);
    mSettingsTable.EnumerateEntries(EnumerateCertOverridesCallback, &capac);
  }
  return NS_OK;
}
