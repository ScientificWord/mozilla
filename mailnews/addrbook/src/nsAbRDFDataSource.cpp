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
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Mark Banner <mark@standard8.demon.co.uk>
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

#include "nsAbRDFDataSource.h"
#include "nsAbBaseCID.h"
#include "nsIAbDirectory.h"
#include "nsIAddrBookSession.h"
#include "nsIAbCard.h"

#include "rdf.h"
#include "nsIRDFService.h"
#include "nsRDFCID.h"
#include "nsIRDFNode.h"
#include "nsEnumeratorUtils.h"
#include "nsIProxyObjectManager.h"

#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsXPIDLString.h"
#include "nsAutoLock.h"
#include "nsIServiceManager.h"
#include "nsThreadUtils.h"

// this is used for notification of observers using nsVoidArray
typedef struct _nsAbRDFNotification {
  nsIRDFDataSource *datasource;
  nsIRDFResource *subject;
  nsIRDFResource *property;
  nsIRDFNode *object;
} nsAbRDFNotification;
                                                

nsresult nsAbRDFDataSource::createNode(const PRUnichar *str, nsIRDFNode **node)
{
	*node = nsnull;
	nsresult rv; 
    nsCOMPtr<nsIRDFService> rdf(do_GetService("@mozilla.org/rdf/rdf-service;1", &rv)); 
	NS_ENSURE_SUCCESS(rv, rv); // always check this before proceeding 
	nsCOMPtr<nsIRDFLiteral> value;
	rv = rdf->GetLiteral(str, getter_AddRefs(value));
	if (NS_SUCCEEDED(rv)) 
	{
		NS_IF_ADDREF(*node = value);
	}
	return rv;
}

nsresult nsAbRDFDataSource::createBlobNode(PRUint8 *value, PRUint32 &length, nsIRDFNode **node, nsIRDFService *rdfService)
{
  NS_ENSURE_ARG_POINTER(node);
  NS_ENSURE_ARG_POINTER(rdfService);

  *node = nsnull;
  nsCOMPtr<nsIRDFBlob> blob;
  nsresult rv = rdfService->GetBlobLiteral(value, length, getter_AddRefs(blob));
  NS_ENSURE_SUCCESS(rv,rv);
  NS_IF_ADDREF(*node = blob);
  return rv;
}

PRBool nsAbRDFDataSource::changeEnumFunc(nsISupports *aElement, void *aData)
{
  nsAbRDFNotification* note = (nsAbRDFNotification *)aData;
  nsIRDFObserver* observer = (nsIRDFObserver *)aElement;

  observer->OnChange(note->datasource,
                     note->subject,
                     note->property,
                     nsnull, note->object);
  return PR_TRUE;
}

PRBool nsAbRDFDataSource::assertEnumFunc(nsISupports *aElement, void *aData)
{
  nsAbRDFNotification *note = (nsAbRDFNotification *)aData;
  nsIRDFObserver* observer = (nsIRDFObserver *)aElement;
  
  observer->OnAssert(note->datasource,
                     note->subject,
                     note->property,
                     note->object);
  return PR_TRUE;
}

PRBool nsAbRDFDataSource::unassertEnumFunc(nsISupports *aElement, void *aData)
{
  nsAbRDFNotification* note = (nsAbRDFNotification *)aData;
  nsIRDFObserver* observer = (nsIRDFObserver *)aElement;

  observer->OnUnassert(note->datasource,
                       note->subject,
                       note->property,
                       note->object);
  return PR_TRUE;
}

nsresult nsAbRDFDataSource::CreateProxyObserver (nsIRDFObserver* observer,
	nsIRDFObserver** proxyObserver)
{
	nsresult rv;

	// Proxy the observer on the UI thread
	/*
	 * TODO
	 * Currenly using NS_PROXY_ASYNC, however
	 * this can flood the event queue if
	 * rate of events on the observer is
	 * greater that the time to process the
	 * events.
	 * This causes the UI to pause.
	 */
	rv = NS_GetProxyForObject (
    NS_PROXY_TO_MAIN_THREAD,
		NS_GET_IID(nsIRDFObserver),
		observer,
		NS_PROXY_ASYNC | NS_PROXY_ALWAYS,
		(void** )proxyObserver);

	return rv;
}

nsresult nsAbRDFDataSource::CreateProxyObservers ()
{
	nsresult rv = NS_OK;

	PRUint32 nObservers;
	mObservers->Count (&nObservers);

	if (!mProxyObservers)
	{
		rv = NS_NewISupportsArray(getter_AddRefs(mProxyObservers));
		NS_ENSURE_SUCCESS(rv, rv);
	}

	PRUint32 nProxyObservers;
	mProxyObservers->Count (&nProxyObservers);

	/*
	 * For all the outstanding observers that
	 * have not been proxied
	 */
	for (PRUint32 i = nProxyObservers; i < nObservers; i++)
	{
		nsCOMPtr<nsISupports> supports;
		rv = mObservers->GetElementAt (i, getter_AddRefs (supports));
		NS_ENSURE_SUCCESS(rv, rv);

		nsCOMPtr<nsIRDFObserver> observer (do_QueryInterface (supports, &rv));
		NS_ENSURE_SUCCESS(rv, rv);
		
		// Create the proxy
		nsCOMPtr<nsIRDFObserver> proxyObserver;
		rv = CreateProxyObserver (observer, getter_AddRefs (proxyObserver));
		NS_ENSURE_SUCCESS(rv, rv);

		mProxyObservers->AppendElement(proxyObserver);
	}

	return rv;
}

nsresult nsAbRDFDataSource::NotifyObservers(nsIRDFResource *subject,
	nsIRDFResource *property,
	nsIRDFNode *object,
	PRBool assert,
	PRBool change)
{
	NS_ASSERTION(!(change && assert),
                 "Can't change and assert at the same time!\n");

	if(!mLock)
	{
		NS_ERROR("Error in AutoLock resource in nsAbRDFDataSource::NotifyObservers()");
		return NS_ERROR_OUT_OF_MEMORY;
	}

	nsresult rv;

	nsAutoLock lockGuard (mLock);

	if (!mObservers)
		return NS_OK;


	/*
	 * TODO
	 * Is the main thread always guaranteed to be
	 * the UI thread?
	 *
	 * Note that this also binds the data source
	 * to the UI which is supposedly the only
	 * place where it is used, but what about
	 * RDF datasources that are not UI specific
	 * but are used in the UI?
	 */
	nsCOMPtr<nsISupportsArray> observers;
	if (NS_IsMainThread())
	{
		/*
		 * Since this is the UI Thread use the
		 * observers list directly for performance
		 */
		observers = mObservers;
	}
	else
	{
		/*
		 * This is a different thread to the UI
		 * thread need to use proxies to the
		 * observers
		 *
		 * Create the proxies
		 */
		rv = CreateProxyObservers ();
		NS_ENSURE_SUCCESS (rv, rv);

		observers = mProxyObservers;
	}

	nsAbRDFNotification note = { this, subject, property, object };
	if (change)
		observers->EnumerateForwards(changeEnumFunc, &note);
	else if (assert)
		observers->EnumerateForwards(assertEnumFunc, &note);
	else
		observers->EnumerateForwards(unassertEnumFunc, &note);

	return NS_OK;
}

nsresult nsAbRDFDataSource::NotifyPropertyChanged(nsIRDFResource *resource,
	nsIRDFResource *propertyResource,
	const PRUnichar *oldValue, 
	const PRUnichar *newValue)
{
	nsCOMPtr<nsIRDFNode> newValueNode;
	createNode(newValue, getter_AddRefs(newValueNode));
	NotifyObservers(resource, propertyResource, newValueNode, PR_FALSE, PR_TRUE);
	return NS_OK;
}


nsAbRDFDataSource::nsAbRDFDataSource():
  mObservers(nsnull),
  mProxyObservers(nsnull),
  mLock(nsnull)
{
	mLock = PR_NewLock ();
}

nsAbRDFDataSource::~nsAbRDFDataSource (void)
{
	if(mLock)
		PR_DestroyLock (mLock);
}

NS_IMPL_THREADSAFE_ISUPPORTS1(nsAbRDFDataSource, nsIRDFDataSource)

 // nsIRDFDataSource methods
NS_IMETHODIMP nsAbRDFDataSource::GetURI(char* *uri)
{
    NS_NOTREACHED("should be implemented by a subclass");
    return NS_ERROR_UNEXPECTED;
}

NS_IMETHODIMP nsAbRDFDataSource::GetSource(nsIRDFResource* property,
                                               nsIRDFNode* target,
                                               PRBool tv,
                                               nsIRDFResource** source /* out */)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::GetTarget(nsIRDFResource* source,
                                               nsIRDFResource* property,
                                               PRBool tv,
                                               nsIRDFNode** target)
{
    return NS_RDF_NO_VALUE;
}


NS_IMETHODIMP nsAbRDFDataSource::GetSources(nsIRDFResource* property,
                                                nsIRDFNode* target,
                                                PRBool tv,
                                                nsISimpleEnumerator** sources)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::GetTargets(nsIRDFResource* source,
                                                nsIRDFResource* property,    
                                                PRBool tv,
                                                nsISimpleEnumerator** targets)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::Assert(nsIRDFResource* source,
                      nsIRDFResource* property, 
                      nsIRDFNode* target,
                      PRBool tv)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::Unassert(nsIRDFResource* source,
                        nsIRDFResource* property,
                        nsIRDFNode* target)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::Change(nsIRDFResource *aSource,
                                              nsIRDFResource *aProperty,
                                              nsIRDFNode *aOldTarget,
                                              nsIRDFNode *aNewTarget)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::Move(nsIRDFResource *aOldSource,
                                            nsIRDFResource *aNewSource,
                                            nsIRDFResource *aProperty,
                                            nsIRDFNode *aTarget)
{
    return NS_RDF_NO_VALUE;
}


NS_IMETHODIMP nsAbRDFDataSource::HasAssertion(nsIRDFResource* source,
                            nsIRDFResource* property,
                            nsIRDFNode* target,
                            PRBool tv,
                            PRBool* hasAssertion)
{
    *hasAssertion = PR_FALSE;
    return NS_OK;
}

NS_IMETHODIMP nsAbRDFDataSource::AddObserver(nsIRDFObserver* observer)
{
	if(!mLock)
	{
		NS_ERROR("Error in AutoLock resource in nsAbRDFDataSource::AddObservers()");
		return NS_ERROR_OUT_OF_MEMORY;
	}

	nsresult rv;

	// Lock the whole method
	nsAutoLock lockGuard (mLock);

	if (!mObservers)
	{
		rv = NS_NewISupportsArray(getter_AddRefs(mObservers));
		NS_ENSURE_SUCCESS(rv, rv);
	}

	// Do not add if already present
	PRInt32 i;
	mObservers->GetIndexOf (observer, &i);
	if (i >= 0)
		return NS_OK;

	mObservers->AppendElement(observer);

	/*
	 * If the proxy observers has been created
	 * then do the work here to avoid unecessary
	 * delay when performing the notify from a
	 * different thread
	 */
	if (mProxyObservers)
	{
		nsCOMPtr<nsIRDFObserver> proxyObserver;
		rv = CreateProxyObserver (observer,
			getter_AddRefs(proxyObserver));
		NS_ENSURE_SUCCESS(rv, rv);

		mProxyObservers->AppendElement (proxyObserver);
	}

	return NS_OK;
}

NS_IMETHODIMP nsAbRDFDataSource::RemoveObserver(nsIRDFObserver* observer)
{
	if(!mLock)
	{
		NS_ERROR("Error in AutoLock resource in nsAbRDFDataSource::RemoveObservers()");
		return NS_ERROR_OUT_OF_MEMORY;
	}

	// Lock the whole method
	nsAutoLock lockGuard (mLock);

	if (!mObservers)
		return NS_OK;

	PRInt32 i;
	mObservers->GetIndexOf (observer, &i);
	if (i >= 0)
	{
		mObservers->RemoveElementAt(i);

		if (mProxyObservers)
			mProxyObservers->RemoveElementAt(i);
	}

	return NS_OK;
}

NS_IMETHODIMP 
nsAbRDFDataSource::HasArcIn(nsIRDFNode *aNode, nsIRDFResource *aArc, PRBool *result)
{
  *result = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP 
nsAbRDFDataSource::HasArcOut(nsIRDFResource *aSource, nsIRDFResource *aArc, PRBool *result)
{
  *result = PR_FALSE;
  return NS_OK;
}

NS_IMETHODIMP nsAbRDFDataSource::ArcLabelsIn(nsIRDFNode* node,
                                                nsISimpleEnumerator** labels)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::ArcLabelsOut(nsIRDFResource* source,
                                                 nsISimpleEnumerator** labels)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::GetAllResources(nsISimpleEnumerator** aCursor)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP
nsAbRDFDataSource::GetAllCmds(nsIRDFResource* source,
                                      nsISimpleEnumerator/*<nsIRDFResource>*/** commands)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP
nsAbRDFDataSource::IsCommandEnabled(nsISupportsArray/*<nsIRDFResource>*/* aSources,
                                        nsIRDFResource*   aCommand,
                                        nsISupportsArray/*<nsIRDFResource>*/* aArguments,
                                        PRBool* aResult)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP nsAbRDFDataSource::DoCommand
(nsISupportsArray * aSources, nsIRDFResource* aCommand, nsISupportsArray * aArguments)
{
    return NS_RDF_NO_VALUE;
}

NS_IMETHODIMP
nsAbRDFDataSource::BeginUpdateBatch()
{
    return NS_OK;
}

NS_IMETHODIMP
nsAbRDFDataSource::EndUpdateBatch()
{
    return NS_OK;
}
