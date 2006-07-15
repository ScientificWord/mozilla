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
 *   Seth Spitzer <sspitzer@netscape.com>
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
#ifndef __nsDirectoryDataSource_h
#define __nsDirectoryDataSource_h
 

#include "nsAbRDFDataSource.h"
#include "nsIRDFService.h"
#include "nsIAbListener.h"
#include "nsIAbDirectory.h"
#include "nsDirPrefs.h"
#include "nsIAbListener.h"
#include "nsISupportsArray.h"
#include "nsIObserver.h"
#include "nsWeakReference.h"
#include "nsICollation.h"

/**
 * The addressbook data source.
 */
class nsAbDirectoryDataSource : public nsAbRDFDataSource,
							    public nsIAbListener, public nsIObserver, public nsSupportsWeakReference
{
private:
	PRBool	mInitialized;

public:
  
	NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_NSIABLISTENER
  NS_DECL_NSIOBSERVER

	nsAbDirectoryDataSource(void);
	virtual ~nsAbDirectoryDataSource (void);
	virtual nsresult Init();

	// nsIRDFDataSource methods
	NS_IMETHOD GetURI(char* *uri);

	NS_IMETHOD GetTarget(nsIRDFResource* source,
					   nsIRDFResource* property,
					   PRBool tv,
					   nsIRDFNode** target);

	NS_IMETHOD GetTargets(nsIRDFResource* source,
						nsIRDFResource* property,    
						PRBool tv,
						nsISimpleEnumerator** targets);

	NS_IMETHOD Assert(nsIRDFResource* source,
					nsIRDFResource* property, 
					nsIRDFNode* target,
					PRBool tv);

	NS_IMETHOD HasAssertion(nsIRDFResource* source,
						  nsIRDFResource* property,
						  nsIRDFNode* target,
						  PRBool tv,
						  PRBool* hasAssertion);

	NS_IMETHOD HasArcOut(nsIRDFResource *aSource, nsIRDFResource *aArc, PRBool *result);

	NS_IMETHOD ArcLabelsOut(nsIRDFResource* source,
						  nsISimpleEnumerator** labels); 

	NS_IMETHOD IsCommandEnabled(nsISupportsArray/*<nsIRDFResource>*/* aSources,
							  nsIRDFResource*   aCommand,
							  nsISupportsArray/*<nsIRDFResource>*/* aArguments,
							  PRBool* aResult);

	NS_IMETHOD DoCommand(nsISupportsArray/*<nsIRDFResource>*/* aSources,
					   nsIRDFResource*   aCommand,
					   nsISupportsArray/*<nsIRDFResource>*/* aArguments);

protected:

	nsresult createDirectoryNode(nsIAbDirectory* directory, nsIRDFResource* property,
                                 nsIRDFNode** target);
	nsresult createDirectoryNameNode(nsIAbDirectory *directory,
                                     nsIRDFNode **target);
	nsresult createDirectoryUriNode(nsIAbDirectory *directory,
                                     nsIRDFNode **target);
	nsresult createDirectoryChildNode(nsIAbDirectory *directory,
                                      nsIRDFNode **target);
	nsresult createDirectoryIsMailListNode(nsIAbDirectory *directory,
                                      nsIRDFNode **target);
  nsresult createDirectoryIsRemoteNode(nsIAbDirectory *directory,
    nsIRDFNode **target);
  nsresult createDirectoryIsSecureNode(nsIAbDirectory *directory,
    nsIRDFNode **target);
	nsresult createDirectoryIsWriteableNode(nsIAbDirectory *directory,
                                            nsIRDFNode **target);
  nsresult createDirectoryTreeNameSortNode(nsIAbDirectory *directory,
                                            nsIRDFNode **target);
	nsresult getDirectoryArcLabelsOut(nsIAbDirectory *directory,
										   nsISupportsArray **arcs);

  nsresult DoModifyDirectory(nsISupportsArray *parentDir,
                              nsISupportsArray *arguments);
	nsresult DoDeleteFromDirectory(nsISupportsArray *parentDirs,
							  nsISupportsArray *delDirs);
	nsresult DoDeleteCardsFromDirectory(nsIAbDirectory *directory,
							  nsISupportsArray *delDirs);

	nsresult DoDirectoryAssert(nsIAbDirectory *directory, 
					nsIRDFResource *property, nsIRDFNode *target);
	nsresult DoDirectoryHasAssertion(nsIAbDirectory *directory, 
							 nsIRDFResource *property, nsIRDFNode *target,
							 PRBool tv, PRBool *hasAssertion);

	nsresult GetTargetHasAssertion(nsIRDFDataSource *dataSource, nsIRDFResource* dirResource,
							   nsIRDFResource *property,PRBool tv, nsIRDFNode *target,PRBool* hasAssertion);
  nsresult CreateCollationKey(const nsString &aSource,  PRUint8 **aKey, PRUint32 *aLength);

  nsCOMPtr<nsIRDFResource> kNC_Child;
  nsCOMPtr<nsIRDFResource> kNC_DirName;
  nsCOMPtr<nsIRDFResource> kNC_DirUri;
  nsCOMPtr<nsIRDFResource> kNC_IsMailList;
  nsCOMPtr<nsIRDFResource> kNC_IsRemote;
  nsCOMPtr<nsIRDFResource> kNC_IsSecure;
  nsCOMPtr<nsIRDFResource> kNC_IsWriteable;
  nsCOMPtr<nsIRDFResource> kNC_DirTreeNameSort;
  nsCOMPtr<nsICollation> mCollationKeyGenerator;
  
  // commands
  nsCOMPtr<nsIRDFResource> kNC_Modify;
  nsCOMPtr<nsIRDFResource> kNC_Delete;
  nsCOMPtr<nsIRDFResource> kNC_DeleteCards;
  
  //Cached literals
  nsCOMPtr<nsIRDFNode> kTrueLiteral;
  nsCOMPtr<nsIRDFNode> kFalseLiteral;

private:
  nsresult Cleanup();
};

nsresult NS_NewAbDirectoryDataSource(const nsIID& iid, void **result);


#endif
