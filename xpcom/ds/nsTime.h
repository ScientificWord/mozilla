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

#ifndef nsTime_h__
#define nsTime_h__

#include "prtime.h"
#include "nsInt64.h"
#include "nscore.h"

/**
 * This class encapsulates full 64-bit time functionality and
 * provides simple arithmetic and conversion operations.
 */

// If you ever decide that you need to add a non-inline method to this
// class, be sure to change the class declaration to "class NS_BASE
// nsTime".

class nsTime : public nsInt64
{
public:
    /**
     * Construct the current time.
     */
    nsTime(void) : nsInt64(PR_Now()) {
    }

    /**
     * Construct the time from a string.
     */
    nsTime(const char* dateStr, PRBool defaultToGMT) {
        PRInt64 theTime;
        PRStatus status = PR_ParseTimeString(dateStr, defaultToGMT, &theTime);
        if (status == PR_SUCCESS)
            mValue = theTime;
        else
            mValue = LL_ZERO;
    }

    /**
     * Construct a time from a PRTime.
     */
    nsTime(const PRTime aTime) : nsInt64(aTime) {
    }

    /**
     * Construct a time from a 64-bit value.
     */
    nsTime(const nsInt64& aTime) : nsInt64(aTime) {
    }

    /**
     * Construct a time from another time.
     */
    nsTime(const nsTime& aTime) : nsInt64(aTime.mValue) {
    }

    // ~nsTime(void) -- XXX destructor unnecessary

    /**
     * Assign one time to another.
     */
    const nsTime& operator =(const nsTime& aTime) {
        mValue = aTime.mValue;
        return *this;
    }

    /**
     * Convert a nsTime object to a PRTime
     */
    operator PRTime(void) const {
        return mValue;
    }
};

/**
 * Determine if one time is strictly less than another
 */
inline const PRBool
operator <(const nsTime& aTime1, const nsTime& aTime2) {
    return aTime1.mValue < aTime2.mValue;
}

/**
 * Determine if one time is less than or equal to another
 */
inline const PRBool
operator <=(const nsTime& aTime1, const nsTime& aTime2) {
    return aTime1.mValue <= aTime2.mValue;
}

/**
 * Determine if one time is strictly greater than another
 */
inline const PRBool
operator >(const nsTime& aTime1, const nsTime& aTime2) {
    return aTime1.mValue > aTime2.mValue;
}

/**
 * Determine if one time is greater than or equal to another
 */
inline const PRBool
operator >=(const nsTime& aTime1, const nsTime& aTime2) {
    return aTime1.mValue >= aTime2.mValue;
}

#endif // nsTime_h__
