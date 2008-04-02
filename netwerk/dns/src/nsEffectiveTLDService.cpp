//* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Effective-TLD Service
 *
 * The Initial Developer of the Original Code is
 * Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pamela Greene <pamg.bugs@gmail.com> (original author)
 *   Daniel Witte <dwitte@stanford.edu>
 *   Jeff Walden <jwalden+code@mit.edu>
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

// This service reads a file of rules describing TLD-like domain names.  For a
// complete description of the expected file format and parsing rules, see
// http://wiki.mozilla.org/Gecko:Effective_TLD_Service

#include "nsEffectiveTLDService.h"
#include "nsIIDNService.h"
#include "nsNetUtil.h"
#include "prnetdb.h"

NS_IMPL_ISUPPORTS1(nsEffectiveTLDService, nsIEffectiveTLDService)

// ----------------------------------------------------------------------

static const ETLDEntry gEntries[] =
#include "etld_data.inc"
;

// ----------------------------------------------------------------------

nsresult
nsEffectiveTLDService::Init()
{
  // We'll probably have to rehash at least once, since nsTHashtable doesn't
  // use a perfect hash, but at least we'll save a few rehashes along the way.
  // Next optimization here is to precompute the hash using something like
  // gperf, but one step at a time.  :-)
  if (!mHash.Init(NS_ARRAY_LENGTH(gEntries) - 1))
    return NS_ERROR_OUT_OF_MEMORY;

  nsresult rv;
  mIDNService = do_GetService(NS_IDNSERVICE_CONTRACTID, &rv);
  if (NS_FAILED(rv)) return rv;

  // Initialize eTLD hash from static array
  for (PRUint32 i = 0; i < NS_ARRAY_LENGTH(gEntries) - 1; i++) {
#ifdef DEBUG
    nsDependentCString name(gEntries[i].domain);
    nsCAutoString normalizedName(gEntries[i].domain);
    NS_ASSERTION(NS_SUCCEEDED(NormalizeHostname(normalizedName)),
                 "normalization failure!");
    NS_ASSERTION(name.Equals(normalizedName), "domain not normalized!");
#endif
    nsDomainEntry *entry = mHash.PutEntry(gEntries[i].domain);
    NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);
    entry->SetData(&gEntries[i]);
  }
  return NS_OK;
}

// External function for dealing with URI's correctly.
// Pulls out the host portion from an nsIURI, and calls through to
// GetPublicSuffixFromHost().
NS_IMETHODIMP
nsEffectiveTLDService::GetPublicSuffix(nsIURI     *aURI,
                                       nsACString &aPublicSuffix)
{
  NS_ENSURE_ARG_POINTER(aURI);

  nsCOMPtr<nsIURI> innerURI = NS_GetInnermostURI(aURI);
  NS_ENSURE_ARG_POINTER(innerURI);

  nsCAutoString host;
  nsresult rv = innerURI->GetAsciiHost(host);
  if (NS_FAILED(rv)) return rv;

  return GetBaseDomainInternal(host, 0, aPublicSuffix);
}

// External function for dealing with URI's correctly.
// Pulls out the host portion from an nsIURI, and calls through to
// GetBaseDomainFromHost().
NS_IMETHODIMP
nsEffectiveTLDService::GetBaseDomain(nsIURI     *aURI,
                                     PRUint32    aAdditionalParts,
                                     nsACString &aBaseDomain)
{
  NS_ENSURE_ARG_POINTER(aURI);

  nsCOMPtr<nsIURI> innerURI = NS_GetInnermostURI(aURI);
  NS_ENSURE_ARG_POINTER(innerURI);

  nsCAutoString host;
  nsresult rv = innerURI->GetAsciiHost(host);
  if (NS_FAILED(rv)) return rv;

  return GetBaseDomainInternal(host, aAdditionalParts + 1, aBaseDomain);
}

// External function for dealing with a host string directly: finds the public
// suffix (e.g. co.uk) for the given hostname. See GetBaseDomainInternal().
NS_IMETHODIMP
nsEffectiveTLDService::GetPublicSuffixFromHost(const nsACString &aHostname,
                                               nsACString       &aPublicSuffix)
{
  // Create a mutable copy of the hostname and normalize it to ACE.
  // This will fail if the hostname includes invalid characters.
  nsCAutoString normHostname(aHostname);
  nsresult rv = NormalizeHostname(normHostname);
  if (NS_FAILED(rv)) return rv;

  return GetBaseDomainInternal(normHostname, 0, aPublicSuffix);
}

// External function for dealing with a host string directly: finds the base
// domain (e.g. www.co.uk) for the given hostname and number of subdomain parts
// requested. See GetBaseDomainInternal().
NS_IMETHODIMP
nsEffectiveTLDService::GetBaseDomainFromHost(const nsACString &aHostname,
                                             PRUint32          aAdditionalParts,
                                             nsACString       &aBaseDomain)
{
  // Create a mutable copy of the hostname and normalize it to ACE.
  // This will fail if the hostname includes invalid characters.
  nsCAutoString normHostname(aHostname);
  nsresult rv = NormalizeHostname(normHostname);
  if (NS_FAILED(rv)) return rv;

  return GetBaseDomainInternal(normHostname, aAdditionalParts + 1, aBaseDomain);
}

// Finds the base domain for a host, with requested number of additional parts.
// This will fail, generating an error, if the host is an IPv4/IPv6 address,
// if more subdomain parts are requested than are available, or if the hostname
// includes characters that are not valid in a URL. Normalization is performed
// on the host string and the result will be in UTF8.
nsresult
nsEffectiveTLDService::GetBaseDomainInternal(nsCString  &aHostname,
                                             PRUint32    aAdditionalParts,
                                             nsACString &aBaseDomain)
{
  if (aHostname.IsEmpty())
    return NS_ERROR_INVALID_ARG;

  // chomp any trailing dot, and keep track of it for later
  PRBool trailingDot = aHostname.Last() == '.';
  if (trailingDot)
    aHostname.Truncate(aHostname.Length() - 1);

  // Check if we're dealing with an IPv4/IPv6 hostname, and return
  PRNetAddr addr;
  PRStatus result = PR_StringToNetAddr(aHostname.get(), &addr);
  if (result == PR_SUCCESS)
    return NS_ERROR_HOST_IS_IP_ADDRESS;

  // Walk up the domain tree, most specific to least specific,
  // looking for matches at each level.  Note that a given level may
  // have multiple attributes (e.g. IsWild() and IsNormal()).
  const char *prevDomain = nsnull;
  const char *currDomain = aHostname.get();
  const char *nextDot = strchr(currDomain, '.');
  const char *end = currDomain + aHostname.Length();
  const char *eTLD = currDomain;
  while (1) {
    nsDomainEntry *entry = mHash.GetEntry(currDomain);
    if (entry) {
      if (entry->IsWild() && prevDomain) {
        // wildcard rules imply an eTLD one level inferior to the match.
        eTLD = prevDomain;
        break;

      } else if (entry->IsNormal() || !nextDot) {
        // specific match, or we've hit the top domain level
        eTLD = currDomain;
        break;

      } else if (entry->IsException()) {
        // exception rules imply an eTLD one level superior to the match.
        eTLD = nextDot + 1;
        break;
      }
    }

    if (!nextDot) {
      // we've hit the top domain level; use it by default.
      eTLD = currDomain;
      break;
    }

    prevDomain = currDomain;
    currDomain = nextDot + 1;
    nextDot = strchr(currDomain, '.');
  }

  // count off the number of requested domains.
  const char *begin = aHostname.get();
  const char *iter = eTLD;
  while (1) {
    if (iter == begin)
      break;

    if (*(--iter) == '.' && aAdditionalParts-- == 0) {
      ++iter;
      ++aAdditionalParts;
      break;
    }
  }

  if (aAdditionalParts != 0)
    return NS_ERROR_INSUFFICIENT_DOMAIN_LEVELS;

  aBaseDomain = Substring(iter, end);
  // add on the trailing dot, if applicable
  if (trailingDot)
    aBaseDomain.Append('.');

  return NS_OK;
}

// Normalizes the given hostname, component by component.  ASCII/ACE
// components are lower-cased, and UTF-8 components are normalized per
// RFC 3454 and converted to ACE.
nsresult
nsEffectiveTLDService::NormalizeHostname(nsCString &aHostname)
{
  if (!IsASCII(aHostname)) {
    nsresult rv = mIDNService->ConvertUTF8toACE(aHostname, aHostname);
    if (NS_FAILED(rv))
      return rv;
  }

  ToLowerCase(aHostname);
  return NS_OK;
}
