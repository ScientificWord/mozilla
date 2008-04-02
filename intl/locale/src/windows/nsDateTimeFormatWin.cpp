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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "nsDateTimeFormatWin.h"
#include "nsIServiceManager.h"
#include "nsIComponentManager.h"
#include "nsLocaleCID.h"
#include "nsILocaleService.h"
#include "nsIWin32Locale.h"
#include "nsUnicharUtils.h"
#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "prmem.h"


#define NSDATETIMEFORMAT_BUFFER_LEN  80

NS_IMPL_THREADSAFE_ISUPPORTS1(nsDateTimeFormatWin, nsIDateTimeFormat)


// init this interface to a specified locale
nsresult nsDateTimeFormatWin::Initialize(nsILocale* locale)
{
  nsAutoString localeStr;
  nsresult res = NS_OK;

  // use cached info if match with stored locale
  if (NULL == locale) {
    if (!mLocale.IsEmpty() && 
        mLocale.Equals(mAppLocale, nsCaseInsensitiveStringComparator())) {
      return NS_OK;
    }
  }
  else {
    res = locale->GetCategory(NS_LITERAL_STRING("NSILOCALE_TIME"), localeStr);
    if (NS_SUCCEEDED(res) && !localeStr.IsEmpty()) {
      if (!mLocale.IsEmpty() && 
          mLocale.Equals(localeStr, nsCaseInsensitiveStringComparator())) {
        return NS_OK;
      }
    }
  }

  // default LCID (en-US)
  mLCID = 1033;

  // get locale string, use app default if no locale specified
  if (NULL == locale) {
    nsCOMPtr<nsILocaleService> localeService = 
             do_GetService(NS_LOCALESERVICE_CONTRACTID);
    if (localeService) {
      nsCOMPtr<nsILocale> appLocale;
      res = localeService->GetApplicationLocale(getter_AddRefs(appLocale));
      if (NS_SUCCEEDED(res)) {
        res = appLocale->GetCategory(NS_LITERAL_STRING("NSILOCALE_TIME"), 
			             localeStr);
        if (NS_SUCCEEDED(res) && !localeStr.IsEmpty()) {
          mAppLocale.Assign(localeStr); // cache app locale name
        }
      }
    }
  }
  else {
    res = locale->GetCategory(NS_LITERAL_STRING("NSILOCALE_TIME"), localeStr);
  }

  // Get LCID and charset name from locale, if available
  if (NS_SUCCEEDED(res) && !localeStr.IsEmpty()) {
    mLocale.Assign(localeStr); // cache locale name

    nsCOMPtr <nsIWin32Locale> win32Locale = do_GetService(NS_WIN32LOCALE_CONTRACTID);
    if (win32Locale) {
      res = win32Locale->GetPlatformLocale(mLocale, (LCID *) &mLCID);
    }
  }

  return res;
}

// performs a locale sensitive date formatting operation on the time_t parameter
nsresult nsDateTimeFormatWin::FormatTime(nsILocale* locale, 
                                         const nsDateFormatSelector  dateFormatSelector, 
                                         const nsTimeFormatSelector timeFormatSelector, 
                                         const time_t  timetTime, 
                                         nsAString& stringOut)
{
  return FormatTMTime(locale, dateFormatSelector, timeFormatSelector, localtime( &timetTime ), stringOut);
}

// performs a locale sensitive date formatting operation on the struct tm parameter
nsresult nsDateTimeFormatWin::FormatTMTime(nsILocale* locale, 
                                           const nsDateFormatSelector  dateFormatSelector, 
                                           const nsTimeFormatSelector timeFormatSelector, 
                                           const struct tm*  tmTime, 
                                           nsAString& stringOut)
{
  SYSTEMTIME system_time;
  DWORD dwFlags_Date = 0, dwFlags_Time = 0;
  int dateLen, timeLen;
  PRUnichar dateBuffer[NSDATETIMEFORMAT_BUFFER_LEN], timeBuffer[NSDATETIMEFORMAT_BUFFER_LEN];

  // set up locale data
  (void) Initialize(locale);

  // Map tm to SYSTEMTIME
  system_time.wYear = 1900 + tmTime->tm_year;
  system_time.wMonth = tmTime->tm_mon + 1;
  system_time.wDayOfWeek = tmTime->tm_wday;
  system_time.wDay = tmTime->tm_mday;
  system_time.wHour = tmTime->tm_hour;
  system_time.wMinute = tmTime->tm_min;
  system_time.wSecond = tmTime->tm_sec;
  system_time.wMilliseconds = 0;

  // Map to WinAPI date format
  switch (dateFormatSelector) {
  case kDateFormatLong:
    dwFlags_Date = DATE_LONGDATE;
    break;
  case kDateFormatShort:
    dwFlags_Date = DATE_SHORTDATE;
    break;
  case kDateFormatWeekday:
    dwFlags_Date = 0;
    break;
  case kDateFormatYearMonth:
    dwFlags_Date = 0;     // TODO:only availabe NT5
    break;
  }

  // Map to WinAPI time format
  switch (timeFormatSelector) {
  case kTimeFormatSeconds:
    dwFlags_Time = 0;
    break;
  case kTimeFormatNoSeconds:
    dwFlags_Time = TIME_NOSECONDS;
    break;
  case kTimeFormatSecondsForce24Hour:
    dwFlags_Time = TIME_FORCE24HOURFORMAT;
    break;
  case kTimeFormatNoSecondsForce24Hour:
    dwFlags_Time = TIME_NOSECONDS + TIME_FORCE24HOURFORMAT;
    break;
  }

  // Call GetDateFormatW
  if (dateFormatSelector == kDateFormatNone) {
    dateLen = 0;
  }
  else {
    if (dateFormatSelector == kDateFormatYearMonth) {
      dateLen = nsGetDateFormatW(0, &system_time, "yyyy/MM", 
                                 dateBuffer, NSDATETIMEFORMAT_BUFFER_LEN);
    }
    else if (dateFormatSelector == kDateFormatWeekday) {
      dateLen = nsGetDateFormatW(0, &system_time, "ddd", 
                                 dateBuffer, NSDATETIMEFORMAT_BUFFER_LEN);
    }
    else {
      dateLen = nsGetDateFormatW(dwFlags_Date, &system_time, NULL, 
                                 dateBuffer, NSDATETIMEFORMAT_BUFFER_LEN);
    }
    if (dateLen != 0) {
      dateLen--;  // Since the count includes the terminating null.
    }
  }

  // Call GetTimeFormatW
  if (timeFormatSelector == kTimeFormatNone) {
    timeLen = 0;
  }
  else {
    timeLen = nsGetTimeFormatW(dwFlags_Time, &system_time, NULL, 
                               timeBuffer, NSDATETIMEFORMAT_BUFFER_LEN);
    if (timeLen != 0) {
      timeLen--;  // Since the count includes the terminating null.
    }
  }

  NS_ASSERTION(NSDATETIMEFORMAT_BUFFER_LEN >= (PRUint32) (dateLen + 1), "internal date buffer is not large enough");
  NS_ASSERTION(NSDATETIMEFORMAT_BUFFER_LEN >= (PRUint32) (timeLen + 1), "internal time buffer is not large enough");

  // Copy the result
  stringOut.Truncate();
  if (dateLen != 0 && timeLen != 0) {
    stringOut.Assign(dateBuffer, dateLen);
    stringOut.Append((PRUnichar *)(L" "), 1);
    stringOut.Append(timeBuffer, timeLen);
  }
  else if (dateLen != 0 && timeLen == 0) {
    stringOut.Assign(dateBuffer, dateLen);
  }
  else if (dateLen == 0 && timeLen != 0) {
    stringOut.Assign(timeBuffer, timeLen);
  }

  return NS_OK;
}

// performs a locale sensitive date formatting operation on the PRTime parameter
nsresult nsDateTimeFormatWin::FormatPRTime(nsILocale* locale, 
                                           const nsDateFormatSelector  dateFormatSelector, 
                                           const nsTimeFormatSelector timeFormatSelector, 
                                           const PRTime  prTime, 
                                           nsAString& stringOut)
{
  PRExplodedTime explodedTime;
  PR_ExplodeTime(prTime, PR_LocalTimeParameters, &explodedTime);

  return FormatPRExplodedTime(locale, dateFormatSelector, timeFormatSelector, &explodedTime, stringOut);
}

// performs a locale sensitive date formatting operation on the PRExplodedTime parameter
nsresult nsDateTimeFormatWin::FormatPRExplodedTime(nsILocale* locale, 
                                                   const nsDateFormatSelector  dateFormatSelector, 
                                                   const nsTimeFormatSelector timeFormatSelector, 
                                                   const PRExplodedTime*  explodedTime, 
                                                   nsAString& stringOut)
{
  struct tm  tmTime;
  memset( &tmTime, 0, sizeof(tmTime) );

  tmTime.tm_yday = explodedTime->tm_yday;
  tmTime.tm_wday = explodedTime->tm_wday;
  tmTime.tm_year = explodedTime->tm_year;
  tmTime.tm_year -= 1900;
  tmTime.tm_mon = explodedTime->tm_month;
  tmTime.tm_mday = explodedTime->tm_mday;
  tmTime.tm_hour = explodedTime->tm_hour;
  tmTime.tm_min = explodedTime->tm_min;
  tmTime.tm_sec = explodedTime->tm_sec;

  return FormatTMTime(locale, dateFormatSelector, timeFormatSelector, &tmTime, stringOut);
}

int nsDateTimeFormatWin::nsGetTimeFormatW(DWORD dwFlags, const SYSTEMTIME *lpTime,
                                          const char* format, PRUnichar *timeStr, int cchTime)
{
  int len = 0;
  len = GetTimeFormatW(mLCID, dwFlags, lpTime, 
                       format ?
                       const_cast<LPCWSTR>
                                 (NS_ConvertASCIItoUTF16(format).get()) :
                       NULL,
                       (LPWSTR) timeStr, cchTime);
  return len;
}

int nsDateTimeFormatWin::nsGetDateFormatW(DWORD dwFlags, const SYSTEMTIME *lpDate,
                                          const char* format, PRUnichar *dateStr, int cchDate)
{
  int len = 0;
  len = GetDateFormatW(mLCID, dwFlags, lpDate, 
                       format ?
                       const_cast<LPCWSTR>
                                 (NS_ConvertASCIItoUTF16(format).get()) :
                       NULL,
                       (LPWSTR) dateStr, cchDate);
  return len;
}
