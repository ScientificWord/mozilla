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
 * The Initial Developer of the Original Code is Neil Deakin
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Laurent Jouanneau <laurent.jouanneau@disruptive-innovations.com>
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

#ifndef nsXULTemplateQueryProcessorStorage_h__
#define nsXULTemplateQueryProcessorStorage_h__

#include "nsIXULTemplateBuilder.h"
#include "nsIXULTemplateQueryProcessor.h"

#include "nsISimpleEnumerator.h"
#include "nsCOMArray.h"
#include "nsIVariant.h"

#include "mozIStorageValueArray.h"
#include "mozIStorageStatement.h"
#include "mozIStorageConnection.h"

class nsXULTemplateQueryProcessorStorage;

class nsXULTemplateResultSetStorage : public nsISimpleEnumerator
{
private:

    nsCOMPtr<mozIStorageStatement> mStatement;

    nsCOMArray<nsIAtom> mColumnNames;

public:

    // nsISupports interface
    NS_DECL_ISUPPORTS

    // nsISimpleEnumerator interface
    NS_DECL_NSISIMPLEENUMERATOR

    nsXULTemplateResultSetStorage(mozIStorageStatement* aStatement);

    PRInt32 GetColumnIndex(nsIAtom* aColumnName);

    void FillColumnValues(nsCOMArray<nsIVariant>& aArray);

};

class nsXULTemplateQueryProcessorStorage : public nsIXULTemplateQueryProcessor
{
public:

    nsXULTemplateQueryProcessorStorage();

    // nsISupports interface
    NS_DECL_ISUPPORTS

    // nsIXULTemplateQueryProcessor interface
    NS_DECL_NSIXULTEMPLATEQUERYPROCESSOR

private:

    nsCOMPtr<mozIStorageConnection> mStorageConnection;
    PRBool mGenerationStarted;
};

#endif // nsXULTemplateQueryProcessorStorage_h__
