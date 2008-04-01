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
 *   Robert Churchill <rjc@netscape.com>
 *   David Hyatt <hyatt@netscape.com>
 *   Chris Waterson <waterson@netscape.com>
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Neil Deakin <enndeakin@sympatico.ca>
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


#ifndef nsXULTemplateQueryProcessorRDF_h__
#define nsXULTemplateQueryProcessorRDF_h__

#include "nsIContent.h"
#include "nsIRDFContainer.h"
#include "nsIRDFContainerUtils.h"
#include "nsIRDFDataSource.h"
#include "nsIRDFObserver.h"
#include "nsIRDFService.h"
#include "nsIXULTemplateBuilder.h"
#include "nsIXULTemplateQueryProcessor.h"
#include "nsICollation.h"
#include "nsCollationCID.h"

#include "nsFixedSizeAllocator.h"
#include "nsResourceSet.h"
#include "nsRuleNetwork.h"
#include "nsRDFQuery.h"
#include "nsRDFBinding.h"
#include "nsXULTemplateResultSetRDF.h"
#include "nsVoidArray.h"
#include "nsCOMArray.h"
#include "nsIArray.h"
#include "nsString.h"
#include "nsClassHashtable.h"
#include "nsRefPtrHashtable.h"
#include "nsCycleCollectionParticipant.h"

#include "prlog.h"
#ifdef PR_LOGGING
extern PRLogModuleInfo* gXULTemplateLog;
#endif

class nsIRDFCompositeDataSource;
class nsXULTemplateResultRDF;

/**
 * An object that generates results from a query on an RDF graph
 */
class nsXULTemplateQueryProcessorRDF : public nsIXULTemplateQueryProcessor,
                                       public nsIRDFObserver
{
public:

    nsXULTemplateQueryProcessorRDF();

    ~nsXULTemplateQueryProcessorRDF();

    nsresult InitGlobals();

    // nsISupports interface
    NS_DECL_CYCLE_COLLECTING_ISUPPORTS
    NS_DECL_CYCLE_COLLECTION_CLASS_AMBIGUOUS(nsXULTemplateQueryProcessorRDF,
                                             nsIXULTemplateQueryProcessor)

    // nsIXULTemplateQueryProcessor interface
    NS_DECL_NSIXULTEMPLATEQUERYPROCESSOR
   
    // nsIRDFObserver interface
    NS_DECL_NSIRDFOBSERVER

    /*
     * Propagate all changes through the rule network when an assertion is
     * added to the graph, adding any new results.
     */
    nsresult
    Propagate(nsIRDFResource* aSource,
              nsIRDFResource* aProperty,
              nsIRDFNode* aTarget);

    /*
     * Retract all changes through the rule network when an assertion is
     * removed from the graph, removing any results that no longer match.
     */
    nsresult
    Retract(nsIRDFResource* aSource,
            nsIRDFResource* aProperty,
            nsIRDFNode* aTarget);

    /*
     * Synchronize results when the graph changes, updating their bindings.
     */
    nsresult
    SynchronizeAll(nsIRDFResource* aSource,
                   nsIRDFResource* aProperty,
                   nsIRDFNode* aOldTarget,
                   nsIRDFNode* aNewTarget);

    /*
     * Return true if a resource is a container
     */
    nsresult
    CheckContainer(nsIRDFResource* aTargetResource,
                   PRBool* aIsContainer);

    /*
     * Check if a resource does not have any children
     */
    nsresult
    CheckEmpty(nsIRDFResource* aTargetResource,
               PRBool* aIsEmpty);

    /**
     * Check if a resource is a separator
     */
    nsresult
    CheckIsSeparator(nsIRDFResource* aResource, PRBool* aIsSeparator);

    /*
     * Compute the containment properties which are additional arcs which
     * indicate that a node is a container, in additional to the RDF container
     * tests. The computed list is stored in mContainmentProperties
     */
    nsresult
    ComputeContainmentProperties(nsIDOMNode* aRootNode);

    /**
     * Compile a query that uses the extended template syntax. The last
     * compiled node of the query is returned as aLastNode. This node will
     * have been added to mAllTests which owns the node.
     */
    nsresult
    CompileExtendedQuery(nsRDFQuery* aQuery,
                         nsIContent* aConditions,
                         TestNode** aLastNode);

    /**
     * Compile a single query child and return the compiled node in aResult.
     * This node will have been added to mAllTests which owns the node and
     * set as a child of aParentNode.
     */
    virtual nsresult
    CompileQueryChild(nsIAtom* aTag,
                      nsRDFQuery* aQuery,
                      nsIContent* aConditions,
                      TestNode* aParentNode,
                      TestNode** aResult);

    /**
     * Parse the value of a property test assertion for a condition or a simple
     * rule based on the parseType attribute into the appropriate literal type.
     */
    nsresult ParseLiteral(const nsString& aParseType, 
                          const nsString& aValue,
                          nsIRDFNode** aResult);

    /**
     * Compile a <triple> condition and return the compiled node in aResult.
     * This node will have been added to mAllTests which owns the node and
     * set as a child of aParentNode.
     */
    nsresult
    CompileTripleCondition(nsRDFQuery* aQuery,
                           nsIContent* aCondition,
                           TestNode* aParentNode,
                           TestNode** aResult);

    /**
     * Compile a <member> condition and return the compiled node in aResult.
     * This node will have been added to mAllTests which owns the node and
     * set as a child of aParentNode.
     */
    nsresult
    CompileMemberCondition(nsRDFQuery* aQuery,
                           nsIContent* aCondition,
                           TestNode* aParentNode,
                           TestNode** aResult);

    /**
     * Add the default rules shared by all simple queries. This creates
     * the content start node followed by a member test. The member TestNode
     * is returned in aChildNode. Both nodes will have been added to mAllTests
     * which owns the nodes.
     */
    nsresult
    AddDefaultSimpleRules(nsRDFQuery* aQuery,
                          TestNode** aChildNode);

    /**
     * Compile a query that's specified using the simple template
     * syntax. Each  TestNode is created in a chain, the last compiled node
     * is returned as aLastNode. All nodes will have been added to mAllTests
     * which owns the nodes.
     */
    nsresult
    CompileSimpleQuery(nsRDFQuery* aQuery,
                      nsIContent* aQueryElement,
                      TestNode** aLastNode);

    RDFBindingSet*
    GetBindingsForRule(nsIDOMNode* aRule);

    /*
     * Indicate that a result is dependant on a particular resource. When an
     * assertion is added to or removed from the graph involving that
     * resource, that result must be recalculated.
     */
    nsresult
    AddBindingDependency(nsXULTemplateResultRDF* aResult,
                         nsIRDFResource* aResource);

    /**
     * Remove a dependency a result has on a particular resource.
     */
    nsresult
    RemoveBindingDependency(nsXULTemplateResultRDF* aResult,
                            nsIRDFResource* aResource);

    /**
     * A memory element is a hash of an RDF triple. One exists for each triple
     * that was involved in generating a result. This function adds this to a
     * map, keyed by memory element, when the value is a list of results that
     * depend on that memory element. When an RDF triple is removed from the
     * datasource, RetractElement is called, and this map is examined to
     * determine which results are no longer valid.
     */
    nsresult
    AddMemoryElements(const Instantiation& aInst,
                      nsXULTemplateResultRDF* aResult);

    /**
     * Remove the memory elements associated with a result when the result is
     * no longer being used.
     */
    nsresult
    RemoveMemoryElements(const Instantiation& aInst,
                         nsXULTemplateResultRDF* aResult);

    /**
     * Remove the results associated with a memory element since the
     * RDF triple the memory element is a hash of has been removed.
     */
    void RetractElement(const MemoryElement& aMemoryElement);

    /**
     * Return the index of a result's resource in its RDF container
     */
    PRInt32
    GetContainerIndexOf(nsIXULTemplateResult* aResult);

    /**
     * Given a result and a predicate to sort on, get the target value of
     * the triple to use for sorting. The sort predicate is the predicate
     * with '?sort=true' appended.
     */
    nsresult
    GetSortValue(nsIXULTemplateResult* aResult,
                 nsIRDFResource* aPredicate,
                 nsIRDFResource* aSortPredicate,
                 nsISupports** aResultNode);

    nsIRDFDataSource* GetDataSource() { return mDB; }

    nsIXULTemplateBuilder* GetBuilder() { return mBuilder; }

    nsResourceSet& ContainmentProperties() { return mContainmentProperties; }

#ifdef PR_LOGGING
    nsresult
    Log(const char* aOperation,
        nsIRDFResource* aSource,
        nsIRDFResource* aProperty,
        nsIRDFNode* aTarget);

#define LOG(_op, _src, _prop, _targ) \
    Log(_op, _src, _prop, _targ)

#else
#define LOG(_op, _src, _prop, _targ)
#endif

protected:
    // We are an observer of the composite datasource. The cycle is
    // broken when the document is destroyed.
    nsCOMPtr<nsIRDFDataSource> mDB;

    // weak reference to the builder, cleared when the document is destroyed
    nsIXULTemplateBuilder* mBuilder;

    // true if the query processor has been initialized
    PRBool mQueryProcessorRDFInited;

    // true if results have been generated. Once set, bindings can no longer
    // be added. If they were, the binding value arrays for results that have
    // already been generated would be the wrong size
    PRBool mGenerationStarted;

    // nesting level for RDF batch notifications
    PRInt32 mUpdateBatchNest;

    // containment properties that are checked to determine if a resource is
    // a container
    nsResourceSet mContainmentProperties;

    // the end node of the default simple node hierarchy
    TestNode* mSimpleRuleMemberTest;

    // the reference variable
    nsCOMPtr<nsIAtom> mRefVariable;

    // the last ref that was calculated, used for simple rules
    nsCOMPtr<nsIXULTemplateResult> mLastRef;

    /**
     * A map between nsIRDFNodes that form the left-hand side (the subject) of
     * a <binding> and an array of nsIXULTemplateResults. When a new assertion
     * is added to the graph involving a particular rdf node, it is looked up
     * in this binding map. If it exists, the corresponding results must then
     * be synchronized.
     */
    nsClassHashtable<nsISupportsHashKey,
                     nsCOMArray<nsXULTemplateResultRDF> > mBindingDependencies;

    /**
     * A map between memory elements and an array of nsIXULTemplateResults.
     * When a triple is unasserted from the graph, the corresponding results
     * no longer match so they must be removed.
     */
    nsClassHashtable<nsUint32HashKey,
                     nsCOMArray<nsXULTemplateResultRDF> > mMemoryElementToResultMap;

    // map of the rules to the bindings for those rules.
    // XXXndeakin this might be better just as an array since there is usually
    //            ten or fewer rules
    nsRefPtrHashtable<nsISupportsHashKey, RDFBindingSet> mRuleToBindingsMap;

    /**
     * The queries
     */
    nsCOMArray<nsITemplateRDFQuery> mQueries;

    /**
     * All of the RDF tests in the rule network, which are checked when a new
     * assertion is added to the graph. This is a subset of mAllTests, which
     * also includes non-RDF tests.
     */
    ReteNodeSet mRDFTests;

    /**
     * All of the tests in the rule network, owned by this list
     */
    ReteNodeSet mAllTests;

    // pseudo-constants
    static nsrefcnt gRefCnt;

public:
    static nsIRDFService*            gRDFService;
    static nsIRDFContainerUtils*     gRDFContainerUtils;
    static nsIRDFResource*           kNC_BookmarkSeparator;
    static nsIRDFResource*           kRDF_type;
};

#endif // nsXULTemplateQueryProcessorRDF_h__
