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

/*

  Some tests for nsAutoLock.

 */

#include "nsAutoLock.h"
#include "prthread.h"

PRLock* gLock;
int gCount;

static void PR_CALLBACK run(void* arg)
{
    for (int i = 0; i < 1000000; ++i) {
        nsAutoLock guard(gLock);
        ++gCount;
        PR_ASSERT(gCount == 1);
        --gCount;
    }
}


int main(int argc, char** argv)
{
    gLock = PR_NewLock();
    gCount = 0;

    // This shouldn't compile
    //nsAutoLock* l1 = new nsAutoLock(theLock);
    //delete l1;

    // Create a block-scoped lock. This should compile.
    {
        nsAutoLock l2(gLock);
    }

    // Fork a thread to access the shared variable in a tight loop
    PRThread* t1 =
        PR_CreateThread(PR_SYSTEM_THREAD,
                        run,
                        nsnull,
                        PR_PRIORITY_NORMAL,
                        PR_GLOBAL_THREAD,
                        PR_JOINABLE_THREAD,
                        0);

    // ...and now do the same thing ourselves
    run(nsnull);

    // Wait for the background thread to finish, if necessary.
    PR_JoinThread(t1);
    return 0;
}
