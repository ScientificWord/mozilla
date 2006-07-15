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
 *   Lars Knoll <knoll@kde.org>
 *   Zack Rusin <zack@kde.org>
 *   Denis Issoupov <denis@macadamian.com>
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

#include "nsDragService.h"
#include "nsIServiceManager.h"
#include "nsXPCOM.h"
#include "nsISupportsPrimitives.h"
#include "nsXPIDLString.h"
#include "nsPrimitiveHelpers.h"
#include "nsReadableUtils.h"
#include "nsClipboard.h"
#include "nsMime.h"
#include "nsCRT.h"

#include <qapplication.h>
#include <qclipboard.h>

//----------------------------------------------------------
nsMimeStoreData::nsMimeStoreData(const QCString &name, const QByteArray &data)
{
    flavorName = name;
    flavorData = data;
}

nsMimeStoreData::nsMimeStoreData(const char *name, void *rawdata, PRInt32 rawlen)
{
    flavorName = name;
    flavorData.assign((char*)rawdata,(unsigned int)rawlen);
}

//----------------------------------------------------------
nsMimeStore::nsMimeStore()
{
    mMimeStore.setAutoDelete(TRUE);
}

nsMimeStore::~nsMimeStore()
{
}

const char* nsMimeStore::format(int n) const
{
    if (n >= (int)mMimeStore.count())
        return 0;

    // because of const

    const nsMimeStoreData* msd;
    msd = mMimeStore.at(n);

    return msd->flavorName;
}

QByteArray nsMimeStore::encodedData(const char* name) const
{
    QByteArray qba;

    QString mime(name);

    /* XXX: for some reason in here we're getting mimetype
     * with charset, eg "text/plain;charset=UTF-8" so here
     * we remove the charset
     */
    int n = mime.findRev(";");
    mime = mime.remove(n, mime.length() - n);

    // because of const
    nsMimeStoreData* msd;
    QPtrList<nsMimeStoreData>::const_iterator it = mMimeStore.begin();
    while ((msd = *it)) {
        if (mime.utf8() == msd->flavorName) {
            qba = msd->flavorData;
            return qba;
        }
        ++it;
    }
#ifdef NS_DEBUG
    printf("nsMimeStore::encodedData requested unknown %s\n", name);
#endif
    return qba;
}

PRBool nsMimeStore::ContainsFlavor(const char* name)
{
    for (nsMimeStoreData *msd = mMimeStore.first(); msd; msd = mMimeStore.next()) {
        if (!strcmp(name, msd->flavorName))
            return PR_TRUE;
    }
    return PR_FALSE;
}

PRBool nsMimeStore::AddFlavorData(const char* name, const QByteArray &data)
{
    if (ContainsFlavor(name))
        return PR_FALSE;
    mMimeStore.append(new nsMimeStoreData(name, data));

    // we're done unless we're given text/unicode,
    // and text/plain is not already advertised,
    if (strcmp(name, kUnicodeMime) || ContainsFlavor(kTextMime))
        return PR_TRUE;
    // in which case we also advertise text/plain
    // which we will convert on our own in nsDataObj::GetText().

    // let's text/plain be first for stupid programs
    // Ownership of |as| is transfered to mMimeStore
    mMimeStore.insert(0,new nsMimeStoreData(kTextMime,data.data(),data.count() + 1));
    return PR_TRUE;
}

//----------------------------------------------------------
nsDragObject::nsDragObject(nsMimeStore* mimeStore,QWidget* dragSource,
                           const char* name)
    : QDragObject(dragSource, name)
{
    if (!mimeStore)
        NS_ASSERTION(PR_TRUE, "Invalid  pointer.");

    mMimeStore = mimeStore;
}

nsDragObject::~nsDragObject()
{
    delete mMimeStore;
}

const char* nsDragObject::format(int i) const
{
    if (i >= (int)mMimeStore->count())
        return 0;

    const char* frm = mMimeStore->format(i);
#ifdef NS_DEBUG
    printf("nsDragObject::format i=%i %s\n",i, frm);
#endif
    return frm;
}

QByteArray nsDragObject::encodedData(const char* frm) const
{
#ifdef NS_DEBUG
    printf("nsDragObject::encodedData %s\n",frm);
#endif
    return mMimeStore->encodedData(frm);
}
