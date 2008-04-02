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
 *   Daniel Witte (dwitte@stanford.edu)
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

#include "TestCommon.h"
#include "nsIServiceManager.h"
#include "nsICookieService.h"
#include "nsICookieManager.h"
#include "nsICookieManager2.h"
#include "nsICookie2.h"
#include <stdio.h>
#include "plstr.h"
#include "prprf.h"
#include "nsNetUtil.h"
#include "nsNetCID.h"
#include "nsStringAPI.h"
#include "nsIPrefBranch.h"
#include "nsIPrefService.h"

static NS_DEFINE_CID(kCookieServiceCID, NS_COOKIESERVICE_CID);
static NS_DEFINE_CID(kPrefServiceCID,   NS_PREFSERVICE_CID);

// various pref strings
static const char kCookiesPermissions[] = "network.cookie.cookieBehavior";
static const char kCookiesDisabledForMailNews[] = "network.cookie.disableCookieForMailNews";
static const char kCookiesLifetimeEnabled[] = "network.cookie.lifetime.enabled";
static const char kCookiesLifetimeDays[] = "network.cookie.lifetime.days";
static const char kCookiesLifetimeCurrentSession[] = "network.cookie.lifetime.behavior";
static const char kCookiesP3PString[] = "network.cookie.p3p";
static const char kCookiesAskPermission[] = "network.cookie.warnAboutCookies";

nsresult
SetACookie(nsICookieService *aCookieService, const char *aSpec1, const char *aSpec2, const char* aCookieString, const char *aServerTime)
{
    nsCOMPtr<nsIURI> uri1, uri2;
    NS_NewURI(getter_AddRefs(uri1), aSpec1);
    if (aSpec2)
        NS_NewURI(getter_AddRefs(uri2), aSpec2);

    printf("    for host \"%s\": SET ", aSpec1);
    nsresult rv = aCookieService->SetCookieStringFromHttp(uri1, uri2, nsnull, (char *)aCookieString, aServerTime, nsnull);
    // the following code is useless. the cookieservice blindly returns NS_OK
    // from SetCookieString. we have to call GetCookie to see if the cookie was
    // set correctly...
    if (NS_FAILED(rv)) {
        printf("nothing\n");
    } else {
        printf("\"%s\"\n", aCookieString);
    }
    return rv;
}

nsresult
SetACookieNoHttp(nsICookieService *aCookieService, const char *aSpec, const char* aCookieString)
{
    nsCOMPtr<nsIURI> uri;
    NS_NewURI(getter_AddRefs(uri), aSpec);

    printf("    for host \"%s\": SET ", aSpec);
    nsresult rv = aCookieService->SetCookieString(uri, nsnull, (char *)aCookieString, nsnull);
    // the following code is useless. the cookieservice blindly returns NS_OK
    // from SetCookieString. we have to call GetCookie to see if the cookie was
    // set correctly...
    if (NS_FAILED(rv)) {
        printf("nothing\n");
    } else {
        printf("\"%s\"\n", aCookieString);
    }
    return rv;
}

// returns PR_TRUE if cookie(s) for the given host were found; else PR_FALSE.
// the cookie string is returned via aCookie.
PRBool
GetACookie(nsICookieService *aCookieService, const char *aSpec1, const char *aSpec2, char **aCookie)
{
    nsCOMPtr<nsIURI> uri1, uri2;
    NS_NewURI(getter_AddRefs(uri1), aSpec1);
    if (aSpec2)
        NS_NewURI(getter_AddRefs(uri2), aSpec2);

    printf("             \"%s\": GOT ", aSpec1);
    nsresult rv = aCookieService->GetCookieStringFromHttp(uri1, uri2, nsnull, aCookie);
    if (NS_FAILED(rv)) printf("XXX GetCookieString() failed!\n");
    if (!*aCookie) {
        printf("nothing\n");
    } else {
        printf("\"%s\"\n", *aCookie);
    }
    return *aCookie != nsnull;
}

// returns PR_TRUE if cookie(s) for the given host were found; else PR_FALSE.
// the cookie string is returned via aCookie.
PRBool
GetACookieNoHttp(nsICookieService *aCookieService, const char *aSpec, char **aCookie)
{
    nsCOMPtr<nsIURI> uri;
    NS_NewURI(getter_AddRefs(uri), aSpec);

    printf("             \"%s\": GOT ", aSpec);
    nsresult rv = aCookieService->GetCookieString(uri, nsnull, aCookie);
    if (NS_FAILED(rv)) printf("XXX GetCookieString() failed!\n");
    if (!*aCookie) {
        printf("nothing\n");
    } else {
        printf("\"%s\"\n", *aCookie);
    }
    return *aCookie != nsnull;
}

// some #defines for comparison rules
#define MUST_BE_NULL     0
#define MUST_EQUAL       1
#define MUST_CONTAIN     2
#define MUST_NOT_CONTAIN 3
#define MUST_NOT_EQUAL   4

// a simple helper function to improve readability:
// takes one of the #defined rules above, and performs the appropriate test.
// PR_TRUE means the test passed; PR_FALSE means the test failed.
static inline PRBool
CheckResult(const char *aLhs, PRUint32 aRule, const char *aRhs = nsnull)
{
    switch (aRule) {
        case MUST_BE_NULL:
            return !aLhs || !*aLhs;

        case MUST_EQUAL:
            return !PL_strcmp(aLhs, aRhs);

        case MUST_NOT_EQUAL:
            return PL_strcmp(aLhs, aRhs);

        case MUST_CONTAIN:
            return PL_strstr(aLhs, aRhs) != nsnull;

        case MUST_NOT_CONTAIN:
            return PL_strstr(aLhs, aRhs) == nsnull;

        default:
            return PR_FALSE; // failure
    }
}

// helper function that ensures the first aSize elements of aResult are
// PR_TRUE (i.e. all tests succeeded). prints the result of the tests (if any
// tests failed, it prints the zero-based index of each failed test).
PRBool
PrintResult(const PRBool aResult[], PRUint32 aSize)
{
    PRBool failed = PR_FALSE;
    printf("*** tests ");
    for (PRUint32 i = 0; i < aSize; ++i) {
        if (!aResult[i]) {
            failed = PR_TRUE;
            printf("%d ", i);
        }
    }
    if (failed) {
        printf("FAILED!\a\n");
    } else {
        printf("passed.\n");
    }
    return !failed;
}

void
InitPrefs(nsIPrefBranch *aPrefBranch)
{
    // init some relevant prefs, so the tests don't go awry.
    // we use the most restrictive set of prefs we can.
    aPrefBranch->SetIntPref(kCookiesPermissions, 1); // 'reject foreign'
    aPrefBranch->SetBoolPref(kCookiesDisabledForMailNews, PR_TRUE);
    aPrefBranch->SetBoolPref(kCookiesLifetimeEnabled, PR_TRUE);
    aPrefBranch->SetIntPref(kCookiesLifetimeCurrentSession, 0);
    aPrefBranch->SetIntPref(kCookiesLifetimeDays, 1);
    aPrefBranch->SetBoolPref(kCookiesAskPermission, PR_FALSE);
}

class ScopedXPCOM
{
public:
  ScopedXPCOM() : rv(NS_InitXPCOM2(nsnull, nsnull, nsnull)) { }
  ~ScopedXPCOM()
  {
    if (NS_SUCCEEDED(rv))
      NS_ShutdownXPCOM(nsnull);
  }

  nsresult rv;
};

int
main(PRInt32 argc, char *argv[])
{
    if (test_common_init(&argc, &argv) != 0)
        return -1;

    PRBool allTestsPassed = PR_TRUE;

    ScopedXPCOM xpcom;
    if (NS_FAILED(xpcom.rv))
      return -1;

    {
      nsresult rv0;

      nsCOMPtr<nsICookieService> cookieService =
        do_GetService(kCookieServiceCID, &rv0);
      if (NS_FAILED(rv0)) return -1;

      nsCOMPtr<nsIPrefBranch> prefBranch =
        do_GetService(kPrefServiceCID, &rv0);
      if (NS_FAILED(rv0)) return -1;

      InitPrefs(prefBranch);

      PRBool rv[20];
      nsCString cookie;

      // call NS_NewURI just to force chrome registrations, so all our
      // printf'ed messages are together.
      {
        nsCOMPtr<nsIURI> foo;
        NS_NewURI(getter_AddRefs(foo), "http://foo.com");
      }
      printf("\n");


      /* The basic idea behind these tests is the following:
       *
       * we set() some cookie, then try to get() it in various ways. we have
       * several possible tests we perform on the cookie string returned from
       * get():
       *
       * a) check whether the returned string is null (i.e. we got no cookies
       *    back). this is used e.g. to ensure a given cookie was deleted
       *    correctly, or to ensure a certain cookie wasn't returned to a given
       *    host.
       * b) check whether the returned string exactly matches a given string.
       *    this is used where we want to make sure our cookie service adheres to
       *    some strict spec (e.g. ordering of multiple cookies), or where we
       *    just know exactly what the returned string should be.
       * c) check whether the returned string contains/does not contain a given
       *    string. this is used where we don't know/don't care about the
       *    ordering of multiple cookies - we just want to make sure the cookie
       *    string contains them all, in some order.
       *
       * the results of each individual testing operation from CheckResult() is
       * stored in an array of bools, which is then checked against the expected
       * outcomes (all successes), by PrintResult(). the overall result of all
       * tests to date is kept in |allTestsPassed|, for convenient display at the
       * end.
       *
       * Interpreting the output:
       * each setting/getting operation will print output saying exactly what
       * it's doing and the outcome, respectively. this information is only
       * useful for debugging purposes; the actual result of the tests is
       * printed at the end of each block of tests. this will either be "all
       * tests passed" or "tests X Y Z failed", where X, Y, Z are the indexes
       * of rv (i.e. zero-based). at the conclusion of all tests, the overall
       * passed/failed result is printed.
       *
       * NOTE: this testsuite is not yet comprehensive or complete, and is
       * somewhat contrived - still under development, and needs improving!
       */

      // *** basic tests
      printf("*** Beginning basic tests...\n");

      // test some basic variations of the domain & path
      SetACookie(cookieService, "http://www.basic.com", nsnull, "test=basic", nsnull);
      GetACookie(cookieService, "http://www.basic.com", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_EQUAL, "test=basic");
      GetACookie(cookieService, "http://www.basic.com/testPath/testfile.txt", nsnull, getter_Copies(cookie));
      rv[1] = CheckResult(cookie.get(), MUST_EQUAL, "test=basic");
      GetACookie(cookieService, "http://www.basic.com./", nsnull, getter_Copies(cookie));
      rv[2] = CheckResult(cookie.get(), MUST_EQUAL, "test=basic");
      GetACookie(cookieService, "http://www.basic.com.", nsnull, getter_Copies(cookie));
      rv[3] = CheckResult(cookie.get(), MUST_EQUAL, "test=basic");
      GetACookie(cookieService, "http://www.basic.com./testPath/testfile.txt", nsnull, getter_Copies(cookie));
      rv[4] = CheckResult(cookie.get(), MUST_EQUAL, "test=basic");
      GetACookie(cookieService, "http://www.basic2.com/", nsnull, getter_Copies(cookie));
      rv[5] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://www.basic.com", nsnull, "test=basic; max-age=-1", nsnull);
      GetACookie(cookieService, "http://www.basic.com/", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_BE_NULL);

      allTestsPassed = PrintResult(rv, 7) && allTestsPassed;


      // *** domain tests
      printf("*** Beginning domain tests...\n");

      // test some variations of the domain & path, for different domains of
      // a domain cookie
      SetACookie(cookieService, "http://www.domain.com", nsnull, "test=domain; domain=domain.com", nsnull);
      GetACookie(cookieService, "http://domain.com", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_EQUAL, "test=domain");
      GetACookie(cookieService, "http://domain.com.", nsnull, getter_Copies(cookie));
      rv[1] = CheckResult(cookie.get(), MUST_EQUAL, "test=domain");
      GetACookie(cookieService, "http://www.domain.com", nsnull, getter_Copies(cookie));
      rv[2] = CheckResult(cookie.get(), MUST_EQUAL, "test=domain");
      GetACookie(cookieService, "http://foo.domain.com", nsnull, getter_Copies(cookie));
      rv[3] = CheckResult(cookie.get(), MUST_EQUAL, "test=domain");
      SetACookie(cookieService, "http://www.domain.com", nsnull, "test=domain; domain=domain.com; max-age=-1", nsnull);
      GetACookie(cookieService, "http://domain.com", nsnull, getter_Copies(cookie));
      rv[4] = CheckResult(cookie.get(), MUST_BE_NULL);

      SetACookie(cookieService, "http://www.domain.com", nsnull, "test=domain; domain=.domain.com", nsnull);
      GetACookie(cookieService, "http://domain.com", nsnull, getter_Copies(cookie));
      rv[5] = CheckResult(cookie.get(), MUST_EQUAL, "test=domain");
      GetACookie(cookieService, "http://www.domain.com", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_EQUAL, "test=domain");
      GetACookie(cookieService, "http://bah.domain.com", nsnull, getter_Copies(cookie));
      rv[7] = CheckResult(cookie.get(), MUST_EQUAL, "test=domain");
      SetACookie(cookieService, "http://www.domain.com", nsnull, "test=domain; domain=.domain.com; max-age=-1", nsnull);
      GetACookie(cookieService, "http://domain.com", nsnull, getter_Copies(cookie));
      rv[8] = CheckResult(cookie.get(), MUST_BE_NULL);

      SetACookie(cookieService, "http://www.domain.com", nsnull, "test=domain; domain=.foo.domain.com", nsnull);
      GetACookie(cookieService, "http://foo.domain.com", nsnull, getter_Copies(cookie));
      rv[9] = CheckResult(cookie.get(), MUST_BE_NULL);

      SetACookie(cookieService, "http://www.domain.com", nsnull, "test=domain; domain=moose.com", nsnull);
      GetACookie(cookieService, "http://foo.domain.com", nsnull, getter_Copies(cookie));
      rv[10] = CheckResult(cookie.get(), MUST_BE_NULL);

      allTestsPassed = PrintResult(rv, 11) && allTestsPassed;


      // *** path tests
      printf("*** Beginning path tests...\n");

      // test some variations of the domain & path, for different paths of
      // a path cookie
      SetACookie(cookieService, "http://path.net/path/file", nsnull, "test=path; path=/path", nsnull);
      GetACookie(cookieService, "http://path.net/path", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_EQUAL, "test=path");
      GetACookie(cookieService, "http://path.net/path/", nsnull, getter_Copies(cookie));
      rv[1] = CheckResult(cookie.get(), MUST_EQUAL, "test=path");
      GetACookie(cookieService, "http://path.net/path/hithere.foo", nsnull, getter_Copies(cookie));
      rv[2] = CheckResult(cookie.get(), MUST_EQUAL, "test=path");
      GetACookie(cookieService, "http://path.net/path?hithere/foo", nsnull, getter_Copies(cookie));
      rv[3] = CheckResult(cookie.get(), MUST_EQUAL, "test=path");
      GetACookie(cookieService, "http://path.net/path2", nsnull, getter_Copies(cookie));
      rv[4] = CheckResult(cookie.get(), MUST_BE_NULL);
      GetACookie(cookieService, "http://path.net/path2/", nsnull, getter_Copies(cookie));
      rv[5] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://path.net/path/file", nsnull, "test=path; path=/path; max-age=-1", nsnull);
      GetACookie(cookieService, "http://path.net/path/", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_BE_NULL);

      SetACookie(cookieService, "http://path.net/path/file", nsnull, "test=path; path=/path/", nsnull);
      GetACookie(cookieService, "http://path.net/path", nsnull, getter_Copies(cookie));
      rv[7] = CheckResult(cookie.get(), MUST_EQUAL, "test=path");
      GetACookie(cookieService, "http://path.net/path/", nsnull, getter_Copies(cookie));
      rv[8] = CheckResult(cookie.get(), MUST_EQUAL, "test=path");
      SetACookie(cookieService, "http://path.net/path/file", nsnull, "test=path; path=/path/; max-age=-1", nsnull);
      GetACookie(cookieService, "http://path.net/path/", nsnull, getter_Copies(cookie));
      rv[9] = CheckResult(cookie.get(), MUST_BE_NULL);

      // note that a site can set a cookie for a path it's not on.
      // this is an intentional deviation from spec (see comments in
      // nsCookieService::CheckPath()), so we test this functionality too
      SetACookie(cookieService, "http://path.net/path/file", nsnull, "test=path; path=/foo/", nsnull);
      GetACookie(cookieService, "http://path.net/path", nsnull, getter_Copies(cookie));
      rv[10] = CheckResult(cookie.get(), MUST_BE_NULL);
      GetACookie(cookieService, "http://path.net/foo", nsnull, getter_Copies(cookie));
      rv[11] = CheckResult(cookie.get(), MUST_EQUAL, "test=path");
      SetACookie(cookieService, "http://path.net/path/file", nsnull, "test=path; path=/foo/; max-age=-1", nsnull);
      GetACookie(cookieService, "http://path.net/foo/", nsnull, getter_Copies(cookie));
      rv[12] = CheckResult(cookie.get(), MUST_BE_NULL);

      // bug 373228: make sure cookies with paths longer than 1024 bytes,
      // and cookies with paths or names containing tabs, are rejected.
      // the following cookie has a path > 1024 bytes explicitly specified in the cookie
      SetACookie(cookieService, "http://path.net/", nsnull, "test=path; path=/1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890/", nsnull);
      GetACookie(cookieService, "http://path.net/1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", nsnull, getter_Copies(cookie));
      rv[13] = CheckResult(cookie.get(), MUST_BE_NULL);
      // the following cookie has a path > 1024 bytes implicitly specified by the uri path
      SetACookie(cookieService, "http://path.net/1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890/", nsnull, "test=path", nsnull);
      GetACookie(cookieService, "http://path.net/1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890/", nsnull, getter_Copies(cookie));
      rv[14] = CheckResult(cookie.get(), MUST_BE_NULL);
      // the following cookie includes a tab in the path
      SetACookie(cookieService, "http://path.net/", nsnull, "test=path; path=/foo\tbar/", nsnull);
      GetACookie(cookieService, "http://path.net/foo\tbar/", nsnull, getter_Copies(cookie));
      rv[15] = CheckResult(cookie.get(), MUST_BE_NULL);
      // the following cookie includes a tab in the name
      SetACookie(cookieService, "http://path.net/", nsnull, "test\ttabs=tab", nsnull);
      GetACookie(cookieService, "http://path.net/", nsnull, getter_Copies(cookie));
      rv[16] = CheckResult(cookie.get(), MUST_BE_NULL);
      // the following cookie includes a tab in the value - allowed
      SetACookie(cookieService, "http://path.net/", nsnull, "test=tab\ttest", nsnull);
      GetACookie(cookieService, "http://path.net/", nsnull, getter_Copies(cookie));
      rv[17] = CheckResult(cookie.get(), MUST_EQUAL, "test=tab\ttest");
      SetACookie(cookieService, "http://path.net/", nsnull, "test=tab\ttest; max-age=-1", nsnull);
      GetACookie(cookieService, "http://path.net/", nsnull, getter_Copies(cookie));
      rv[18] = CheckResult(cookie.get(), MUST_BE_NULL);

      allTestsPassed = PrintResult(rv, 19) && allTestsPassed;


      // *** expiry & deletion tests
      // XXX add server time str parsing tests here
      printf("*** Beginning expiry & deletion tests...\n");

      // test some variations of the expiry time,
      // and test deletion of previously set cookies
      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=expiry; max-age=-1", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=expiry; max-age=0", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[1] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=expiry; expires=Thu, 10 Apr 1980 16:33:12 GMT", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[2] = CheckResult(cookie.get(), MUST_BE_NULL);

      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=expiry; max-age=60", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[3] = CheckResult(cookie.get(), MUST_EQUAL, "test=expiry");
      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=expiry; max-age=-20", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[4] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=expiry; max-age=60", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[5] = CheckResult(cookie.get(), MUST_EQUAL, "test=expiry");
      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=expiry; expires=Thu, 10 Apr 1980 16:33:12 GMT", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=expiry; max-age=60", nsnull);
      SetACookie(cookieService, "http://expireme.org/", nsnull, "newtest=expiry; max-age=60", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[7] = CheckResult(cookie.get(), MUST_CONTAIN, "test=expiry");
      rv[8] = CheckResult(cookie.get(), MUST_CONTAIN, "newtest=expiry");
      SetACookie(cookieService, "http://expireme.org/", nsnull, "test=differentvalue; max-age=0", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[9] = CheckResult(cookie.get(), MUST_EQUAL, "newtest=expiry");
      SetACookie(cookieService, "http://expireme.org/", nsnull, "newtest=evendifferentvalue; max-age=0", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[10] = CheckResult(cookie.get(), MUST_BE_NULL);

      SetACookie(cookieService, "http://foo.expireme.org/", nsnull, "test=expiry; domain=.expireme.org; max-age=60", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[11] = CheckResult(cookie.get(), MUST_EQUAL, "test=expiry");
      SetACookie(cookieService, "http://bar.expireme.org/", nsnull, "test=differentvalue; domain=.expireme.org; max-age=0", nsnull);
      GetACookie(cookieService, "http://expireme.org/", nsnull, getter_Copies(cookie));
      rv[12] = CheckResult(cookie.get(), MUST_BE_NULL);

      allTestsPassed = PrintResult(rv, 13) && allTestsPassed;


      // *** multiple cookie tests
      printf("*** Beginning multiple cookie tests...\n");

      // test the setting of multiple cookies, and test the order of precedence
      // (a later cookie overwriting an earlier one, in the same header string)
      SetACookie(cookieService, "http://multiple.cookies/", nsnull, "test=multiple; domain=.multiple.cookies \n test=different \n test=same; domain=.multiple.cookies \n newtest=ciao \n newtest=foo; max-age=-6 \n newtest=reincarnated", nsnull);
      GetACookie(cookieService, "http://multiple.cookies/", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_NOT_CONTAIN, "test=multiple");
      rv[1] = CheckResult(cookie.get(), MUST_CONTAIN, "test=different");
      rv[2] = CheckResult(cookie.get(), MUST_CONTAIN, "test=same");
      rv[3] = CheckResult(cookie.get(), MUST_NOT_CONTAIN, "newtest=ciao");
      rv[4] = CheckResult(cookie.get(), MUST_NOT_CONTAIN, "newtest=foo");
      rv[5] = CheckResult(cookie.get(), MUST_CONTAIN, "newtest=reincarnated") != nsnull;
      SetACookie(cookieService, "http://multiple.cookies/", nsnull, "test=expiry; domain=.multiple.cookies; max-age=0", nsnull);
      GetACookie(cookieService, "http://multiple.cookies/", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_NOT_CONTAIN, "test=same");
      SetACookie(cookieService, "http://multiple.cookies/", nsnull,  "\n test=different; max-age=0 \n", nsnull);
      GetACookie(cookieService, "http://multiple.cookies/", nsnull, getter_Copies(cookie));
      rv[7] = CheckResult(cookie.get(), MUST_NOT_CONTAIN, "test=different");
      SetACookie(cookieService, "http://multiple.cookies/", nsnull,  "newtest=dead; max-age=0", nsnull);
      GetACookie(cookieService, "http://multiple.cookies/", nsnull, getter_Copies(cookie));
      rv[8] = CheckResult(cookie.get(), MUST_BE_NULL);

      allTestsPassed = PrintResult(rv, 9) && allTestsPassed;


      // *** foreign cookie tests
      printf("*** Beginning foreign cookie tests...\n");

      // test the blocking of foreign cookies, under various circumstances.
      // order of URI arguments is hostURI, firstURI
      SetACookie(cookieService, "http://yahoo.com/", "http://yahoo.com/", "test=foreign; domain=.yahoo.com", nsnull);
      GetACookie(cookieService, "http://yahoo.com/", "http://yahoo.com/", getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      SetACookie(cookieService, "http://weather.yahoo.com/", "http://yahoo.com/", "test=foreign; domain=.yahoo.com", nsnull);
      GetACookie(cookieService, "http://notweather.yahoo.com/", "http://sport.yahoo.com/", getter_Copies(cookie));
      rv[1] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      SetACookie(cookieService, "http://moose.yahoo.com/", "http://canada.yahoo.com/", "test=foreign; domain=.yahoo.com", nsnull);
      GetACookie(cookieService, "http://yahoo.com/", "http://sport.yahoo.com/", getter_Copies(cookie));
      rv[2] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      GetACookie(cookieService, "http://sport.yahoo.com/", "http://yahoo.com/", getter_Copies(cookie));
      rv[3] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      SetACookie(cookieService, "http://jack.yahoo.com/", "http://jill.yahoo.com/", "test=foreign; domain=.yahoo.com; max-age=0", nsnull);
      GetACookie(cookieService, "http://jane.yahoo.com/", "http://yahoo.com/", getter_Copies(cookie));
      rv[4] = CheckResult(cookie.get(), MUST_BE_NULL);

      SetACookie(cookieService, "http://moose.yahoo.com/", "http://foo.moose.yahoo.com/", "test=foreign; domain=.yahoo.com", nsnull);
      GetACookie(cookieService, "http://yahoo.com/", "http://yahoo.com/", getter_Copies(cookie));
      rv[5] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      SetACookie(cookieService, "http://foo.bar.yahoo.com/", "http://yahoo.com/", "test=foreign; domain=.yahoo.com", nsnull);
      GetACookie(cookieService, "http://yahoo.com/", "http://yahoo.com/", getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      SetACookie(cookieService, "http://foo.bar.yahoo.com/", "http://yahoo.com/", "test=foreign; domain=.yahoo.com; max-age=0", nsnull);
      GetACookie(cookieService, "http://yahoo.com/", "http://yahoo.com/", getter_Copies(cookie));
      rv[7] = CheckResult(cookie.get(), MUST_BE_NULL);

      // test handling of IP addresses by the foreign blocking algo
      SetACookie(cookieService, "http://192.168.54.33/", "http://192.168.54.33/", "test=foreign; domain=192.168.54.33", nsnull);
      GetACookie(cookieService, "http://192.168.54.33/", "http://192.168.54.33/", getter_Copies(cookie));
      rv[8] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      GetACookie(cookieService, "http://192.168.54.33./", "http://.192.168.54.33../", getter_Copies(cookie));
      rv[9] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      GetACookie(cookieService, "http://192.168.54.33/", nsnull, getter_Copies(cookie));
      rv[10] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      GetACookie(cookieService, "http://192.168.54.33/", "http://148.168.54.33", getter_Copies(cookie));
      rv[11] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://192.168.54.33/", "http://192.168.54.33/", "test=foreign; domain=192.168.54.33; max-age=0", nsnull);
      GetACookie(cookieService, "http://192.168.54.33/", "http://192.168.54.33/", getter_Copies(cookie));
      rv[12] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://192.168.54.33/", "http://148.168.54.33/", "test=foreign; domain=192.168.54.33", nsnull);
      GetACookie(cookieService, "http://192.168.54.33/", "http://192.168.54.33/", getter_Copies(cookie));
      rv[13] = CheckResult(cookie.get(), MUST_BE_NULL);

      // test the case where the host is an eTLD, e.g. http://co.tv/ (a legitimate site)
      SetACookie(cookieService, "http://co.uk/", "http://co.uk/", "test=foreign; domain=.co.uk", nsnull);
      GetACookie(cookieService, "http://co.uk/", "http://co.uk/", getter_Copies(cookie));
      // should be rejected, can't set a domain cookie for .co.uk
      rv[14] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://co.uk/", "http://co.uk/", "test=foreign", nsnull);
      GetACookie(cookieService, "http://co.uk/", "http://co.uk/", getter_Copies(cookie));
      // should be allowed, hostURI == firstURI and it's not a domain cookie
      rv[15] = CheckResult(cookie.get(), MUST_EQUAL, "test=foreign");
      GetACookie(cookieService, "http://oblivious.co.uk/", nsnull, getter_Copies(cookie));
      rv[16] = CheckResult(cookie.get(), MUST_BE_NULL);
      // remove cookie
      SetACookie(cookieService, "http://co.uk/", "http://co.uk/", "test=foreign; max-age=0", nsnull);
      GetACookie(cookieService, "http://co.uk/", "http://co.uk/", getter_Copies(cookie));
      rv[17] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://co.uk/", "http://evil.co.uk/", "test=foreign", nsnull);
      GetACookie(cookieService, "http://co.uk/", "http://co.uk/", getter_Copies(cookie));
      // should be rejected, hostURI != firstURI and hostURI is an eTLD
      rv[18] = CheckResult(cookie.get(), MUST_BE_NULL);

      allTestsPassed = PrintResult(rv, 19) && allTestsPassed;


      // *** parser tests
      printf("*** Beginning parser tests...\n");

      // test the cookie header parser, under various circumstances.
      SetACookie(cookieService, "http://parser.test/", nsnull, "test=parser; domain=.parser.test; ;; ;=; ,,, ===,abc,=; abracadabra! max-age=20;=;;", nsnull);
      GetACookie(cookieService, "http://parser.test/", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_EQUAL, "test=parser");
      SetACookie(cookieService, "http://parser.test/", nsnull, "test=parser; domain=.parser.test; max-age=0", nsnull);
      GetACookie(cookieService, "http://parser.test/", nsnull, getter_Copies(cookie));
      rv[1] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://parser.test/", nsnull, "test=\"fubar! = foo;bar\\\";\" parser; domain=.parser.test; max-age=6\nfive; max-age=2.63,", nsnull);
      GetACookie(cookieService, "http://parser.test/", nsnull, getter_Copies(cookie));
      rv[2] = CheckResult(cookie.get(), MUST_CONTAIN, "test=\"fubar! = foo;bar\\\";\"");
      rv[3] = CheckResult(cookie.get(), MUST_CONTAIN, "five");
      SetACookie(cookieService, "http://parser.test/", nsnull, "test=kill; domain=.parser.test; max-age=0 \n five; max-age=0", nsnull);
      GetACookie(cookieService, "http://parser.test/", nsnull, getter_Copies(cookie));
      rv[4] = CheckResult(cookie.get(), MUST_BE_NULL);

      // test the handling of VALUE-only cookies (see bug 169091),
      // i.e. "six" should assume an empty NAME, which allows other VALUE-only
      // cookies to overwrite it
      SetACookie(cookieService, "http://parser.test/", nsnull, "six", nsnull);
      GetACookie(cookieService, "http://parser.test/", nsnull, getter_Copies(cookie));
      rv[5] = CheckResult(cookie.get(), MUST_EQUAL, "six");
      SetACookie(cookieService, "http://parser.test/", nsnull, "seven", nsnull);
      GetACookie(cookieService, "http://parser.test/", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_EQUAL, "seven");
      SetACookie(cookieService, "http://parser.test/", nsnull, " =eight", nsnull);
      GetACookie(cookieService, "http://parser.test/", nsnull, getter_Copies(cookie));
      rv[7] = CheckResult(cookie.get(), MUST_EQUAL, "eight");
      SetACookie(cookieService, "http://parser.test/", nsnull, "test=six", nsnull);
      GetACookie(cookieService, "http://parser.test/", nsnull, getter_Copies(cookie));
      rv[9] = CheckResult(cookie.get(), MUST_CONTAIN, "test=six");

      allTestsPassed = PrintResult(rv, 10) && allTestsPassed;


      // *** mailnews tests
      printf("*** Beginning mailnews tests...\n");

      // test some mailnews cookies to ensure blockage.
      // we use null firstURI's deliberately, since we have hacks to deal with
      // this situation...
      SetACookie(cookieService, "mailbox://mail.co.uk/", nsnull, "test=mailnews", nsnull);
      GetACookie(cookieService, "mailbox://mail.co.uk/", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_BE_NULL);
      GetACookie(cookieService, "http://mail.co.uk/", nsnull, getter_Copies(cookie));
      rv[1] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://mail.co.uk/", nsnull, "test=mailnews", nsnull);
      GetACookie(cookieService, "mailbox://mail.co.uk/", nsnull, getter_Copies(cookie));
      rv[2] = CheckResult(cookie.get(), MUST_BE_NULL);
      GetACookie(cookieService, "http://mail.co.uk/", nsnull, getter_Copies(cookie));
      rv[3] = CheckResult(cookie.get(), MUST_EQUAL, "test=mailnews");
      SetACookie(cookieService, "http://mail.co.uk/", nsnull, "test=mailnews; max-age=0", nsnull);
      GetACookie(cookieService, "http://mail.co.uk/", nsnull, getter_Copies(cookie));
      rv[4] = CheckResult(cookie.get(), MUST_BE_NULL);

      // test non-null firstURI's, i) from mailnews ii) not from mailnews
      SetACookie(cookieService, "mailbox://mail.co.uk/", "http://mail.co.uk/", "test=mailnews", nsnull);
      GetACookie(cookieService, "http://mail.co.uk/", nsnull, getter_Copies(cookie));
      rv[5] = CheckResult(cookie.get(), MUST_BE_NULL);
      SetACookie(cookieService, "http://mail.co.uk/", "mailbox://mail.co.uk/", "test=mailnews", nsnull);
      GetACookie(cookieService, "http://mail.co.uk/", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_BE_NULL);

      allTestsPassed = PrintResult(rv, 7) && allTestsPassed;


      // *** path ordering tests
      printf("*** Beginning path ordering tests...\n");

      // test that cookies are returned in path order - longest to shortest.
      // if the header doesn't specify a path, it's taken from the host URI.
      SetACookie(cookieService, "http://multi.path.tests/", nsnull, "test1=path; path=/one/two/three", nsnull);
      SetACookie(cookieService, "http://multi.path.tests/", nsnull, "test2=path; path=/one \n test3=path; path=/one/two/three/four \n test4=path; path=/one/two \n test5=path; path=/one/two/", nsnull);
      SetACookie(cookieService, "http://multi.path.tests/one/two/three/four/five/", nsnull, "test6=path", nsnull);
      SetACookie(cookieService, "http://multi.path.tests/one/two/three/four/five/six/", nsnull, "test7=path; path=", nsnull);
      SetACookie(cookieService, "http://multi.path.tests/", nsnull, "test8=path; path=/", nsnull);
      GetACookie(cookieService, "http://multi.path.tests/one/two/three/four/five/six/", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_EQUAL, "test7=path; test6=path; test3=path; test1=path; test5=path; test4=path; test2=path; test8=path");

      allTestsPassed = PrintResult(rv, 1) && allTestsPassed;


      // *** httponly tests 
      printf("*** Beginning httponly tests...\n");

      // Since this cookie is NOT set via http, setting it fails
      SetACookieNoHttp(cookieService, "http://httponly.test/", "test=httponly; httponly");
      GetACookie(cookieService, "http://httponly.test/", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_BE_NULL);
      // Since this cookie is set via http, it can be retrieved
      SetACookie(cookieService, "http://httponly.test/", nsnull, "test=httponly; httponly", nsnull);
      GetACookie(cookieService, "http://httponly.test/", nsnull, getter_Copies(cookie));
      rv[1] = CheckResult(cookie.get(), MUST_EQUAL, "test=httponly");
      // ... but not by web content
      GetACookieNoHttp(cookieService, "http://httponly.test/", getter_Copies(cookie));
      rv[2] = CheckResult(cookie.get(), MUST_BE_NULL);
      // Non-Http cookies should not replace HttpOnly cookies
      SetACookie(cookieService, "http://httponly.test/", nsnull, "test=httponly; httponly", nsnull);
      SetACookieNoHttp(cookieService, "http://httponly.test/", "test=not-httponly");
      GetACookie(cookieService, "http://httponly.test/", nsnull, getter_Copies(cookie));
      rv[3] = CheckResult(cookie.get(), MUST_EQUAL, "test=httponly");
      // ... and, if an HttpOnly cookie already exists, should not be set at all
      GetACookieNoHttp(cookieService, "http://httponly.test/", getter_Copies(cookie));
      rv[4] = CheckResult(cookie.get(), MUST_BE_NULL);
      // Non-Http cookies should not delete HttpOnly cookies
      SetACookie(cookieService, "http://httponly.test/", nsnull, "test=httponly; httponly", nsnull);
      SetACookieNoHttp(cookieService, "http://httponly.test/", "test=httponly; max-age=-1");
      GetACookie(cookieService, "http://httponly.test/", nsnull, getter_Copies(cookie));
      rv[5] = CheckResult(cookie.get(), MUST_EQUAL, "test=httponly");
      // ... but HttpOnly cookies should
      SetACookie(cookieService, "http://httponly.test/", nsnull, "test=httponly; httponly; max-age=-1", nsnull);
      GetACookie(cookieService, "http://httponly.test/", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_BE_NULL);
      // Non-Httponly cookies can replace HttpOnly cookies when set over http
      SetACookie(cookieService, "http://httponly.test/", nsnull, "test=httponly; httponly", nsnull);
      SetACookie(cookieService, "http://httponly.test/", nsnull, "test=not-httponly", nsnull);
      GetACookieNoHttp(cookieService, "http://httponly.test/", getter_Copies(cookie));
      rv[7] = CheckResult(cookie.get(), MUST_EQUAL, "test=not-httponly");
      // scripts should not be able to set httponly cookies by replacing an existing non-httponly cookie
      SetACookie(cookieService, "http://httponly.test/", nsnull, "test=not-httponly", nsnull);
      SetACookieNoHttp(cookieService, "http://httponly.test/", "test=httponly; httponly");
      GetACookieNoHttp(cookieService, "http://httponly.test/", getter_Copies(cookie));
      rv[8] = CheckResult(cookie.get(), MUST_EQUAL, "test=not-httponly");

      allTestsPassed = PrintResult(rv, 9) && allTestsPassed;


      // *** nsICookieManager{2} interface tests
      printf("*** Beginning nsICookieManager{2} interface tests...\n");
      nsCOMPtr<nsICookieManager> cookieMgr = do_GetService(NS_COOKIEMANAGER_CONTRACTID, &rv0);
      if (NS_FAILED(rv0)) return -1;
      nsCOMPtr<nsICookieManager2> cookieMgr2 = do_QueryInterface(cookieMgr);
      if (!cookieMgr2) return -1;
      
      // first, ensure a clean slate
      rv[0] = NS_SUCCEEDED(cookieMgr->RemoveAll());
      // add some cookies
      rv[1] = NS_SUCCEEDED(cookieMgr2->Add(NS_LITERAL_CSTRING("cookiemgr.test"), // domain
                                           NS_LITERAL_CSTRING("/foo"),           // path
                                           NS_LITERAL_CSTRING("test1"),          // name
                                           NS_LITERAL_CSTRING("yes"),            // value
                                           PR_FALSE,                             // is secure
                                           PR_FALSE,                             // is httponly
                                           PR_TRUE,                              // is session
                                           LL_MAXINT));                          // expiry time
      rv[2] = NS_SUCCEEDED(cookieMgr2->Add(NS_LITERAL_CSTRING("cookiemgr.test"), // domain
                                           NS_LITERAL_CSTRING("/foo"),           // path
                                           NS_LITERAL_CSTRING("test2"),          // name
                                           NS_LITERAL_CSTRING("yes"),            // value
                                           PR_FALSE,                             // is secure
                                           PR_TRUE,                              // is httponly
                                           PR_TRUE,                              // is session
                                           LL_MAXINT));                          // expiry time
      rv[3] = NS_SUCCEEDED(cookieMgr2->Add(NS_LITERAL_CSTRING("new.domain"),     // domain
                                           NS_LITERAL_CSTRING("/rabbit"),        // path
                                           NS_LITERAL_CSTRING("test3"),          // name
                                           NS_LITERAL_CSTRING("yes"),            // value
                                           PR_FALSE,                             // is secure
                                           PR_FALSE,                             // is httponly
                                           PR_TRUE,                              // is session
                                           LL_MAXINT));                          // expiry time
      // confirm using enumerator
      nsCOMPtr<nsISimpleEnumerator> enumerator;
      rv[4] = NS_SUCCEEDED(cookieMgr->GetEnumerator(getter_AddRefs(enumerator)));
      PRInt32 i = 0;
      PRBool more;
      nsCOMPtr<nsICookie2> newDomainCookie;
      while (NS_SUCCEEDED(enumerator->HasMoreElements(&more)) && more) {
        nsCOMPtr<nsISupports> cookie;
        if (NS_FAILED(enumerator->GetNext(getter_AddRefs(cookie)))) break;
        ++i;
        
        // keep tabs on the third cookie, so we can check it later
        nsCOMPtr<nsICookie2> cookie2(do_QueryInterface(cookie));
        if (!cookie2) break;
        nsCAutoString domain;
        cookie2->GetRawHost(domain);
        if (domain == NS_LITERAL_CSTRING("new.domain"))
          newDomainCookie = cookie2;
      }
      rv[5] = i == 3;
      // check the httpOnly attribute of the second cookie is honored
      GetACookie(cookieService, "http://cookiemgr.test/foo/", nsnull, getter_Copies(cookie));
      rv[6] = CheckResult(cookie.get(), MUST_CONTAIN, "test2=yes");
      GetACookieNoHttp(cookieService, "http://cookiemgr.test/foo/", getter_Copies(cookie));
      rv[7] = CheckResult(cookie.get(), MUST_NOT_CONTAIN, "test2=yes");
      // check CountCookiesFromHost()
      PRUint32 hostCookies = 0;
      rv[8] = NS_SUCCEEDED(cookieMgr2->CountCookiesFromHost(NS_LITERAL_CSTRING("cookiemgr.test"), &hostCookies)) &&
              hostCookies == 2;
      // check CookieExists() using the third cookie
      PRBool found;
      rv[9] = NS_SUCCEEDED(cookieMgr2->CookieExists(newDomainCookie, &found)) && found;
      // remove the cookie, block it, and ensure it can't be added again
      rv[10] = NS_SUCCEEDED(cookieMgr->Remove(NS_LITERAL_CSTRING("new.domain"), // domain
                                              NS_LITERAL_CSTRING("test3"),      // name
                                              NS_LITERAL_CSTRING("/rabbit"),    // path
                                              PR_TRUE));                        // is blocked
      rv[11] = NS_SUCCEEDED(cookieMgr2->CookieExists(newDomainCookie, &found)) && !found;
      rv[12] = NS_SUCCEEDED(cookieMgr2->Add(NS_LITERAL_CSTRING("new.domain"),     // domain
                                            NS_LITERAL_CSTRING("/rabbit"),        // path
                                            NS_LITERAL_CSTRING("test3"),          // name
                                            NS_LITERAL_CSTRING("yes"),            // value
                                            PR_FALSE,                             // is secure
                                            PR_FALSE,                             // is httponly
                                            PR_TRUE,                              // is session
                                            LL_MININT));                          // expiry time
      rv[13] = NS_SUCCEEDED(cookieMgr2->CookieExists(newDomainCookie, &found)) && !found;
      // double-check RemoveAll() using the enumerator
      rv[14] = NS_SUCCEEDED(cookieMgr->RemoveAll());
      rv[15] = NS_SUCCEEDED(cookieMgr->GetEnumerator(getter_AddRefs(enumerator))) &&
               NS_SUCCEEDED(enumerator->HasMoreElements(&more)) &&
               !more;

      allTestsPassed = PrintResult(rv, 16) && allTestsPassed;


      // *** eviction and creation ordering tests
      printf("*** Beginning eviction and creation ordering tests...\n");

      // test that cookies are
      // a) returned by order of creation time (oldest first, newest last)
      // b) evicted by order of lastAccessed time, if the limit on cookies per host (50) is reached
      nsCAutoString name;
      nsCAutoString expected;
      for (PRInt32 i = 0; i < 60; ++i) {
        name = NS_LITERAL_CSTRING("test");
        name.AppendInt(i);
        name += NS_LITERAL_CSTRING("=creation");
        SetACookie(cookieService, "http://creation.ordering.tests/", nsnull, name.get(), nsnull);

        if (i == 9) {
          // sleep a couple of seconds, to make sure the first 10 cookies are older than
          // subsequent ones (timer resolution varies on different platforms).
          PR_Sleep(2 * PR_TicksPerSecond());
        }

        if (i >= 10) {
          expected += name;
          if (i < 59)
            expected += NS_LITERAL_CSTRING("; ");
        }
      }
      GetACookie(cookieService, "http://creation.ordering.tests/", nsnull, getter_Copies(cookie));
      rv[0] = CheckResult(cookie.get(), MUST_EQUAL, expected.get());

      // test that cookies are evicted by order of lastAccessed time, if the limit on total cookies
      // (1000) is reached
      nsCAutoString host;
      for (PRInt32 i = 0; i < 1010; ++i) {
        host = NS_LITERAL_CSTRING("http://eviction.");
        host.AppendInt(i);
        host += NS_LITERAL_CSTRING(".tests/");
        SetACookie(cookieService, host.get(), nsnull, "test=eviction", nsnull);

        if (i == 9) {
          // sleep a couple of seconds, to make sure the first 10 cookies are older than
          // subsequent ones (timer resolution varies on different platforms).
          PR_Sleep(2 * PR_TicksPerSecond());
        }
      }
      rv[1] = NS_SUCCEEDED(cookieMgr->GetEnumerator(getter_AddRefs(enumerator)));
      i = 0;
      rv[2] = PR_FALSE; // init to failure in case we break from the while loop
      while (NS_SUCCEEDED(enumerator->HasMoreElements(&more)) && more) {
        nsCOMPtr<nsISupports> cookie;
        if (NS_FAILED(enumerator->GetNext(getter_AddRefs(cookie)))) break;
        ++i;
        
        // keep tabs on the third cookie, so we can check it later
        nsCOMPtr<nsICookie2> cookie2(do_QueryInterface(cookie));
        if (!cookie2) break;
        nsCAutoString domain;
        cookie2->GetRawHost(domain);
        PRInt32 hostNumber;
        PRInt32 numInts = PR_sscanf(domain.get(), "eviction.%ld.tests", &hostNumber);
        if (numInts != 1 || hostNumber < 10) break;
      }
      rv[2] = i == 1000;

      allTestsPassed = PrintResult(rv, 3) && allTestsPassed;


      // XXX the following are placeholders: add these tests please!
      // *** "noncompliant cookie" tests
      // *** IP address tests
      // *** speed tests


      printf("\n*** Result: %s!\n\n", allTestsPassed ? "all tests passed" : "TEST(S) FAILED");

    }
    
    return allTestsPassed ? 0 : 1;
}
