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
 * Portions created by the Initial Developer are Copyright (C) 1994-2000
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

/****************************************************************************
 *  Read in a cert chain from one or more files, and verify the chain for
 *  some usage.
 *                                                                          *
 *  This code was modified from other code also kept in the NSS directory.
 ****************************************************************************/ 

#include <stdio.h>
#include <string.h>

#if defined(XP_UNIX)
#include <unistd.h>
#endif

#include "prerror.h"

#include "nssrenam.h"
#include "pk11func.h"
#include "seccomon.h"
#include "secutil.h"
#include "secmod.h"
#include "secitem.h"
#include "cert.h"


/* #include <stdlib.h> */
/* #include <errno.h> */
/* #include <fcntl.h> */
/* #include <stdarg.h> */

#include "nspr.h"
#include "plgetopt.h"
#include "prio.h"
#include "nss.h"

/* #include "vfyutil.h" */

#define RD_BUF_SIZE (60 * 1024)

int verbose;

char *password = NULL;

/* Function: char * myPasswd()
 * 
 * Purpose: This function is our custom password handler that is called by
 * SSL when retreiving private certs and keys from the database. Returns a
 * pointer to a string that with a password for the database. Password pointer
 * should point to dynamically allocated memory that will be freed later.
 */
char *
myPasswd(PK11SlotInfo *info, PRBool retry, void *arg)
{
    char * passwd = NULL;

    if ( (!retry) && arg ) {
	passwd = PORT_Strdup((char *)arg);
    }
    return passwd;
}

static void
Usage(const char *progName)
{
    fprintf(stderr, 
	    "Usage: %s [-d dbdir] certfile [certfile ...]\n",
            progName);
    exit(1);
}

/**************************************************************************
** 
** Error and information routines.
**
**************************************************************************/

void
errWarn(char *function)
{
    PRErrorCode  errorNumber = PR_GetError();
    const char * errorString = SECU_Strerror(errorNumber);

    fprintf(stderr, "Error in function %s: %d\n - %s\n",
		    function, errorNumber, errorString);
}

void
exitErr(char *function)
{
    errWarn(function);
    /* Exit gracefully. */
    /* ignoring return value of NSS_Shutdown as code exits with 1 anyway*/
    (void) NSS_Shutdown();
    PR_Cleanup();
    exit(1);
}

typedef struct certMemStr {
    struct certMemStr * next;
    CERTCertificate * cert;
} certMem;

certMem * theCerts;

void
rememberCert(CERTCertificate * cert)
{
    certMem * newCertMem = PORT_ZNew(certMem);
    if (newCertMem) {
	newCertMem->next = theCerts;
	newCertMem->cert = cert;
	theCerts = newCertMem;
    }
}

void
forgetCerts(void)
{
    certMem * oldCertMem;
    while (oldCertMem = theCerts) {
    	theCerts = oldCertMem->next;
	CERT_DestroyCertificate(oldCertMem->cert);
	PORT_Free(oldCertMem);
    }
    theCerts = NULL;
}


CERTCertificate *
readCertFile(const char * fileName, PRBool isAscii)
{
    unsigned char * pb;
    CERTCertificate * cert  = NULL;
    CERTCertDBHandle *defaultDB = NULL;
    PRFileDesc*     fd;
    PRInt32         cc      = -1;
    PRInt32         total;
    PRInt32         remaining;
    SECItem         item;
    static unsigned char certBuf[RD_BUF_SIZE];

    fd = PR_Open(fileName, PR_RDONLY, 0777); 
    if (!fd) {
	PRIntn err = PR_GetError();
    	fprintf(stderr, "open of %s failed, %d = %s\n", 
	        fileName, err, SECU_Strerror(err));
	return cert;
    }
    /* read until EOF or buffer is full */
    pb = certBuf;
    while (0 < (remaining = (sizeof certBuf) - (pb - certBuf))) {
	cc = PR_Read(fd, pb, remaining);
	if (cc == 0) 
	    break;
	if (cc < 0) {
	    PRIntn err = PR_GetError();
	    fprintf(stderr, "read of %s failed, %d = %s\n", 
	        fileName, err, SECU_Strerror(err));
	    break;
	}
	/* cc > 0 */
	pb += cc;
    }
    PR_Close(fd);
    if (cc < 0)
    	return cert;
    if (!remaining || cc > 0) { /* file was too big. */
	fprintf(stderr, "cert file %s was too big.\n", fileName);
	return cert;
    }
    total = pb - certBuf;
    if (!total) { /* file was empty */
	fprintf(stderr, "cert file %s was empty.\n", fileName);
	return cert;
    }
    if (isAscii) {
    	/* convert from Base64 to binary here ... someday */
    }
    item.type = siBuffer;
    item.data = certBuf;
    item.len  = total;
    defaultDB = CERT_GetDefaultCertDB();
    cert = CERT_NewTempCertificate(defaultDB, &item, 
                                   NULL     /* nickname */, 
                                   PR_FALSE /* isPerm */, 
				   PR_TRUE  /* copyDER */);
    if (!cert) {
	PRIntn err = PR_GetError();
	fprintf(stderr, "couldn't import %s, %d = %s\n",
	        fileName, err, SECU_Strerror(err));
    }
    return cert;
}

int
main(int argc, char *argv[], char *envp[])
{
    char *               certDir      = NULL;
    char *               progName     = NULL;
    CERTCertificate *    cert;
    CERTCertificate *    firstCert    = NULL;
    CERTCertDBHandle *   defaultDB    = NULL;
    PRBool               isAscii      = PR_FALSE;
    SECStatus            secStatus;
    SECCertificateUsage  certUsage    = certificateUsageSSLServer;
    PLOptState *         optstate;
    PLOptStatus          status;
    int                  rv = 1;

    PR_Init( PR_SYSTEM_THREAD, PR_PRIORITY_NORMAL, 1);

    progName = PL_strdup(argv[0]);

    optstate = PL_CreateOptState(argc, argv, "ad:ru:w:v");
    while ((status = PL_GetNextOpt(optstate)) == PL_OPT_OK) {
	switch(optstate->option) {
	case  0  : /* positional parameter */  goto breakout;
	case 'a' : isAscii  = PR_TRUE;                        break;
	case 'd' : certDir  = PL_strdup(optstate->value);     break;
	case 'r' : isAscii  = PR_FALSE;                       break;
	case 'u' : certUsage = ((SECCertificateUsage) 1) << PORT_Atoi(optstate->value); break;
	case 'w' : password = PL_strdup(optstate->value);     break;
	case 'v' : verbose++;                                 break;
	default  : Usage(progName);                           break;
	}
    }
breakout:
    if (status != PL_OPT_OK)
	Usage(progName);

    /* Set our password function callback. */
    PK11_SetPasswordFunc(myPasswd);

    /* Initialize the NSS libraries. */
    if (certDir) {
	secStatus = NSS_Init(certDir);
    } else {
	secStatus = NSS_NoDB_Init(NULL);

	/* load the builtins */
	SECMOD_AddNewModule("Builtins", DLL_PREFIX"nssckbi."DLL_SUFFIX, 0, 0);
    }
    if (secStatus != SECSuccess) {
	exitErr("NSS_Init");
    }
    SECU_RegisterDynamicOids();

    while (status == PL_OPT_OK) {
	switch(optstate->option) {
	default  : Usage(progName);                           break;
	case 'a' : isAscii  = PR_TRUE;                        break;
	case 'r' : isAscii  = PR_FALSE;                       break;
	case  0  : /* positional parameter */
	    cert = readCertFile(optstate->value, isAscii);
	    if (!cert) 
	        goto punt;
	    rememberCert(cert);
	    if (!firstCert)
	        firstCert = cert;
	    break;
	}
        status = PL_GetNextOpt(optstate);
    }
    if (status == PL_OPT_BAD || !firstCert)
	Usage(progName);

    /* NOW, verify the cert chain. */
    defaultDB = CERT_GetDefaultCertDB();
    secStatus = CERT_VerifyCertificate(defaultDB, firstCert, 
                                PR_TRUE /* check sig */,
				certUsage, 
				PR_Now(), 
				NULL, 		/* wincx  */
				NULL,		/* error log */
                                NULL);          /* returned usages */

    if (secStatus != SECSuccess) {
	PRIntn err = PR_GetError();
	fprintf(stderr, "Chain is bad, %d = %s\n", err, SECU_Strerror(err));
	SECU_printCertProblems(stderr, defaultDB, firstCert, 
			  PR_TRUE, certUsage, NULL, verbose);
    	rv = 1;
    } else {
    	fprintf(stderr, "Chain is good!\n");
	rv = 0;
    }

punt:
    forgetCerts();
    if (NSS_Shutdown() != SECSuccess) {
	SECU_PrintError(progName, "NSS_Shutdown");
	rv = 1;
    }
    PR_Cleanup();
    return rv;
}
