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

#include "nsIServiceManager.h"

// Gee this seems simple! It's for testing for memory leaks with Purify.

void main(int argc, char* argv[])
{
    nsresult rv;
    nsIServiceManager* servMgr;
    rv = NS_InitXPCOM2(&servMgr, NULL, NULL);
    NS_ASSERTION(NS_SUCCEEDED(rv), "NS_InitXPCOM failed");

    // try loading a component and releasing it to see if it leaks
    if (argc > 1 && argv[1] != nsnull) {
        char* cidStr = argv[1];
        nsISupports* obj = nsnull;
        if (cidStr[0] == '{') {
            nsCID cid;
            cid.Parse(cidStr);
            rv = CallCreateInstance(cid, &obj);
        }
        else {
            // contractID case:
            rv = CallCreateInstance(cidStr, &obj);
        }
        if (NS_SUCCEEDED(rv)) {
            printf("Successfully created %s\n", cidStr);
            NS_RELEASE(obj);
        }
        else {
            printf("Failed to create %s (%x)\n", cidStr, rv);
        }
    }

    rv = NS_ShutdownXPCOM(servMgr);
    NS_ASSERTION(NS_SUCCEEDED(rv), "NS_ShutdownXPCOM failed");
}
