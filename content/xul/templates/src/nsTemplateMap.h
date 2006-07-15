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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Chris Waterson <waterson@netscape.com>
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

#ifndef nsTemplateMap_h__
#define nsTemplateMap_h__

#include "pldhash.h"
#include "nsXULElement.h"

class nsTemplateMap {
protected:
    struct Entry {
        PLDHashEntryHdr mHdr;
        nsIContent*     mContent;
        nsIContent*     mTemplate;
    };

    PLDHashTable mTable;

    void
    Init() { PL_DHashTableInit(&mTable, PL_DHashGetStubOps(), nsnull, sizeof(Entry), PL_DHASH_MIN_SIZE); }

    void
    Finish() { PL_DHashTableFinish(&mTable); }

public:
    nsTemplateMap() { Init(); }

    ~nsTemplateMap() { Finish(); }

    void
    Put(nsIContent* aContent, nsIContent* aTemplate) {
        NS_ASSERTION(PL_DHASH_ENTRY_IS_FREE(PL_DHashTableOperate(&mTable, aContent, PL_DHASH_LOOKUP)),
                     "aContent already in map");

        Entry* entry =
            NS_REINTERPRET_CAST(Entry*, PL_DHashTableOperate(&mTable, aContent, PL_DHASH_ADD));

        if (entry) {
            entry->mContent = aContent;
            entry->mTemplate = aTemplate;
        }
    }

    void
    Remove(nsIContent* aContent) {
        NS_ASSERTION(PL_DHASH_ENTRY_IS_BUSY(PL_DHashTableOperate(&mTable, aContent, PL_DHASH_LOOKUP)),
                     "aContent not in map");

        PL_DHashTableOperate(&mTable, aContent, PL_DHASH_REMOVE);

        PRUint32 count;

        // If possible, use the special nsXULElement interface to
        // "peek" at the child count without accidentally creating
        // children as a side effect, since we're about to rip 'em
        // outta the map anyway.
        nsXULElement *xulcontent = nsXULElement::FromContent(aContent);
        if (xulcontent) {
            count = xulcontent->PeekChildCount();
        }
        else {
            count = aContent->GetChildCount();
        }

        for (PRUint32 i = 0; i < count; ++i) {
            Remove(aContent->GetChildAt(i));
        }
    }


    void
    GetTemplateFor(nsIContent* aContent, nsIContent** aResult) {
        Entry* entry =
            NS_REINTERPRET_CAST(Entry*, PL_DHashTableOperate(&mTable, aContent, PL_DHASH_LOOKUP));

        if (PL_DHASH_ENTRY_IS_BUSY(&entry->mHdr))
            NS_IF_ADDREF(*aResult = entry->mTemplate);
        else
            *aResult = nsnull;
    }

    void
    Clear() { Finish(); Init(); }
};

#endif // nsTemplateMap_h__

