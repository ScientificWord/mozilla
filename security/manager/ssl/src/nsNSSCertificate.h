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
 * The Original Code is the Netscape security libraries.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ian McGreer <mcgreer@netscape.com>
 *   Javier Delgadillo <javi@netscape.com>
 *   Kai Engert <kengert@redhat.com>
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

#ifndef _NS_NSSCERTIFICATE_H_
#define _NS_NSSCERTIFICATE_H_

#include "nsIX509Cert.h"
#include "nsIX509Cert2.h"
#include "nsIX509Cert3.h"
#include "nsIX509CertDB.h"
#include "nsIX509CertList.h"
#include "nsIASN1Object.h"
#include "nsISMimeCert.h"
#include "nsNSSShutDown.h"
#include "nsISimpleEnumerator.h"

#include "nsNSSCertHeader.h"

class nsINSSComponent;
class nsIASN1Sequence;

/* Certificate */
class nsNSSCertificate : public nsIX509Cert,
                         public nsIX509Cert2,
                         public nsIX509Cert3,
                         public nsISMimeCert,
                         public nsNSSShutDownObject
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIX509CERT
  NS_DECL_NSIX509CERT2
  NS_DECL_NSIX509CERT3
  NS_DECL_NSISMIMECERT

  nsNSSCertificate(CERTCertificate *cert);
  /* from a request? */
  virtual ~nsNSSCertificate();
  nsresult FormatUIStrings(const nsAutoString &nickname, nsAutoString &nickWithSerial, nsAutoString &details);
  static nsNSSCertificate* ConstructFromDER(char *certDER, int derLen);

  static char* defaultServerNickname(CERTCertificate* cert);

private:
  CERTCertificate *mCert;
  PRBool           mPermDelete;
  PRUint32         mCertType;
  nsCOMPtr<nsIASN1Object> mASN1Structure;
  nsresult CreateASN1Struct();
  nsresult CreateTBSCertificateASN1Struct(nsIASN1Sequence **retSequence,
                                          nsINSSComponent *nssComponent);
  nsresult GetSortableDate(PRTime aTime, nsAString &_aSortableDate);
  virtual void virtualDestroyNSSReference();
  void destructorSafeDestroyNSSReference();
};

class nsNSSCertList: public nsIX509CertList
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIX509CERTLIST

  nsNSSCertList(CERTCertList *certList = nsnull, PRBool adopt = PR_FALSE);
  virtual ~nsNSSCertList();

  static CERTCertList *DupCertList(CERTCertList *aCertList);
private:
  CERTCertList *mCertList;
};

class nsNSSCertListEnumerator: public nsISimpleEnumerator
{
public:
   NS_DECL_ISUPPORTS
   NS_DECL_NSISIMPLEENUMERATOR

   nsNSSCertListEnumerator(CERTCertList *certList);
   virtual ~nsNSSCertListEnumerator();
private:
   CERTCertList *mCertList;
};


#define NS_NSS_LONG 4
#define NS_NSS_GET_LONG(x) ((((unsigned long)((x)[0])) << 24) | \
                            (((unsigned long)((x)[1])) << 16) | \
                            (((unsigned long)((x)[2])) <<  8) | \
                             ((unsigned long)((x)[3])) )
#define NS_NSS_PUT_LONG(src,dest) (dest)[0] = (((src) >> 24) & 0xff); \
                                  (dest)[1] = (((src) >> 16) & 0xff); \
                                  (dest)[2] = (((src) >>  8) & 0xff); \
                                  (dest)[3] = ((src) & 0xff); 




#endif /* _NS_NSSCERTIFICATE_H_ */
