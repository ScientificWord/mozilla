/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
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
 * Christopher Blizzard <blizzard@mozilla.org>.  
 * Portions created by the Initial Developer are Copyright (C) 2002
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

// welcome to the dumping ground for font metrics stuff that has to
// know about both the xft and core fonts code.

#ifdef MOZ_ENABLE_XFT
#include "nsFontMetricsXft.h"
#include "nsIPref.h"
#include "nsServiceManagerUtils.h"
#include "prenv.h"
#endif /* MOZ_ENABLE_XFT */

#ifdef MOZ_ENABLE_COREXFONTS
#include "nsFontMetricsGTK.h"
#endif

#ifdef MOZ_ENABLE_PANGO
#include "nsFontMetricsPango.h"
#include "prenv.h"
#endif

#include "nsFontMetricsUtils.h"

PRUint32
NS_FontMetricsGetHints(void)
{
#ifdef MOZ_ENABLE_PANGO
    if (NS_IsPangoEnabled()) {
        return nsFontMetricsPango::GetHints();
    }
#endif
#ifdef MOZ_ENABLE_XFT
    if (NS_IsXftEnabled()) {
        return nsFontMetricsXft::GetHints();
    }
#endif

#ifdef MOZ_ENABLE_COREXFONTS
    return nsFontMetricsGTK::GetHints();
#endif
}

nsresult
NS_FontMetricsFamilyExists(nsIDeviceContext *aDevice, const nsString &aName)
{
#ifdef MOZ_ENABLE_PANGO
    if (NS_IsPangoEnabled()) {
        return nsFontMetricsPango::FamilyExists(aDevice, aName);
    }
#endif
#ifdef MOZ_ENABLE_XFT
    // try to fall through to the core fonts if xft fails
    if (NS_IsXftEnabled()) {
        return nsFontMetricsXft::FamilyExists(aDevice, aName);
    }
#endif

#ifdef MOZ_ENABLE_COREXFONTS
    return nsFontMetricsGTK::FamilyExists(aDevice, aName);
#endif
}

#if defined(MOZ_ENABLE_XFT) && defined(MOZ_ENABLE_COREXFONTS)

PRBool
NS_IsXftEnabled(void)
{
    static PRBool been_here = PR_FALSE;
    static PRBool cachedXftSetting = PR_TRUE;

    if (!been_here) {
        been_here = PR_TRUE;
        nsCOMPtr<nsIPref> prefService;
        prefService = do_GetService(NS_PREF_CONTRACTID);
        if (!prefService)
            return PR_TRUE;

        nsresult rv;

        rv = prefService->GetBoolPref("fonts.xft.enabled", &cachedXftSetting);

        // Yes, this makes sense.  If xft is compiled in and there's no
        // pref, it's automatically enabled.  If there's no pref, check
        // the environment.
        if (NS_FAILED(rv)) {
            char *val = PR_GetEnv("GDK_USE_XFT");

            if (val && val[0] == '0') {
                cachedXftSetting = PR_FALSE;
                goto end;
            }

            cachedXftSetting = PR_TRUE;
        }
    }

 end:

    return cachedXftSetting;
}

#endif

#if defined(MOZ_ENABLE_PANGO) && (defined(MOZ_ENABLE_XFT) || defined(MOZ_ENABLE_COREXFONTS))

PRBool
NS_IsPangoEnabled(void)
{
    char *val = PR_GetEnv("MOZ_DISABLE_PANGO");
    if (val)
        return FALSE;

    return TRUE;
}

#endif
