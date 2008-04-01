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
 *   Dan Rosen <dr@netscape.com>
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

#ifndef nsXULDocument_h__
#define nsXULDocument_h__

#include "nsCOMPtr.h"
#include "nsXULPrototypeDocument.h"
#include "nsXULPrototypeCache.h"
#include "nsTArray.h"

#include "nsXMLDocument.h"
#include "nsElementMap.h"
#include "nsForwardReference.h"
#include "nsIContent.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOMXULCommandDispatcher.h"
#include "nsIDOMXULDocument.h"
#include "nsCOMArray.h"
#include "nsIURI.h"
#include "nsIXULDocument.h"
#include "nsScriptLoader.h"
#include "nsIStreamListener.h"
#include "nsICSSLoaderObserver.h"

class nsIRDFResource;
class nsIRDFService;
class nsIXULPrototypeCache;
class nsIFocusController;
#if 0 // XXXbe save me, scc (need NSCAP_FORWARD_DECL(nsXULPrototypeScript))
class nsIObjectInputStream;
class nsIObjectOutputStream;
class nsIXULPrototypeScript;
#else
#include "nsIObjectInputStream.h"
#include "nsIObjectOutputStream.h"
#include "nsXULElement.h"
#endif
#include "nsURIHashKey.h"
#include "nsInterfaceHashtable.h"
 
struct JSObject;
struct PRLogModuleInfo;

/**
 * The XUL document class
 */
class nsXULDocument : public nsXMLDocument,
                      public nsIXULDocument,
                      public nsIDOMXULDocument,
                      public nsIStreamLoaderObserver,
                      public nsICSSLoaderObserver
{
public:
    nsXULDocument();
    virtual ~nsXULDocument();

    // nsISupports interface
    NS_DECL_ISUPPORTS_INHERITED
    NS_DECL_NSISTREAMLOADEROBSERVER

    // nsIDocument interface
    virtual void Reset(nsIChannel* aChannel, nsILoadGroup* aLoadGroup);
    virtual void ResetToURI(nsIURI *aURI, nsILoadGroup* aLoadGroup,
                            nsIPrincipal* aPrincipal);

    virtual nsresult StartDocumentLoad(const char* aCommand,
                                       nsIChannel *channel,
                                       nsILoadGroup* aLoadGroup,
                                       nsISupports* aContainer,
                                       nsIStreamListener **aDocListener,
                                       PRBool aReset = PR_TRUE,
                                       nsIContentSink* aSink = nsnull);

    virtual void SetContentType(const nsAString& aContentType);

    virtual void EndLoad();

    // nsIMutationObserver interface
    NS_DECL_NSIMUTATIONOBSERVER_CONTENTAPPENDED
    NS_DECL_NSIMUTATIONOBSERVER_CONTENTINSERTED
    NS_DECL_NSIMUTATIONOBSERVER_CONTENTREMOVED
    NS_DECL_NSIMUTATIONOBSERVER_ATTRIBUTECHANGED

    // nsIXULDocument interface
    NS_IMETHOD AddElementForID(const nsAString& aID, nsIContent* aElement);
    NS_IMETHOD RemoveElementForID(const nsAString& aID, nsIContent* aElement);
    NS_IMETHOD GetElementsForID(const nsAString& aID,
                                nsCOMArray<nsIContent>& aElements);

    NS_IMETHOD GetScriptGlobalObjectOwner(nsIScriptGlobalObjectOwner** aGlobalOwner);
    NS_IMETHOD AddSubtreeToDocument(nsIContent* aElement);
    NS_IMETHOD RemoveSubtreeFromDocument(nsIContent* aElement);
    NS_IMETHOD SetTemplateBuilderFor(nsIContent* aContent,
                                     nsIXULTemplateBuilder* aBuilder);
    NS_IMETHOD GetTemplateBuilderFor(nsIContent* aContent,
                                     nsIXULTemplateBuilder** aResult);
    NS_IMETHOD OnPrototypeLoadDone(PRBool aResumeWalk);
    PRBool OnDocumentParserError();

    // nsIDOMNode interface overrides
    NS_IMETHOD CloneNode(PRBool deep, nsIDOMNode **_retval);

    // nsIDOMDocument interface overrides
    NS_IMETHOD GetElementById(const nsAString & elementId,
                              nsIDOMElement **_retval); 

    // nsIDOMXULDocument interface
    NS_DECL_NSIDOMXULDOCUMENT

    // nsIDOMNSDocument
    NS_IMETHOD GetContentType(nsAString& aContentType);

    // nsICSSLoaderObserver
    NS_IMETHOD StyleSheetLoaded(nsICSSStyleSheet* aSheet,
                                PRBool aWasAlternate,
                                nsresult aStatus);

    static PRBool
    MatchAttribute(nsIContent* aContent,
                   PRInt32 aNameSpaceID,
                   nsIAtom* aAttrName,
                   void* aData);

    NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED_NO_UNLINK(nsXULDocument,
                                                       nsXMLDocument)

protected:
    // Implementation methods
    friend nsresult
    NS_NewXULDocument(nsIXULDocument** aResult);

    nsresult Init(void);
    nsresult StartLayout(void);

    nsresult
    AddElementToMap(nsIContent* aElement);

    nsresult
    RemoveElementFromMap(nsIContent* aElement);

    nsresult GetViewportSize(PRInt32* aWidth, PRInt32* aHeight);

    static PRIntn
    RemoveElementsFromMapByContent(const PRUnichar* aID,
                                   nsIContent* aElement,
                                   void* aClosure);

    void SetIsPopup(PRBool isPopup) { mIsPopup = isPopup; }

    nsresult PrepareToLoad(nsISupports* aContainer,
                           const char* aCommand,
                           nsIChannel* aChannel,
                           nsILoadGroup* aLoadGroup,
                           nsIParser** aResult);

    nsresult
    PrepareToLoadPrototype(nsIURI* aURI,
                           const char* aCommand,
                           nsIPrincipal* aDocumentPrincipal,
                           nsIParser** aResult);

    nsresult 
    LoadOverlayInternal(nsIURI* aURI, PRBool aIsDynamic, PRBool* aShouldReturn,
                        PRBool* aFailureFromContent);

    nsresult ApplyPersistentAttributes();
    nsresult ApplyPersistentAttributesToElements(nsIRDFResource* aResource,
                                                 nsCOMArray<nsIContent>& aElements);

    nsresult
    AddElementToDocumentPre(nsIContent* aElement);

    nsresult
    AddElementToDocumentPost(nsIContent* aElement);

    nsresult
    ExecuteOnBroadcastHandlerFor(nsIContent* aBroadcaster,
                                 nsIDOMElement* aListener,
                                 nsIAtom* aAttr);

    void GetFocusController(nsIFocusController** aFocusController);

    PRInt32 GetDefaultNamespaceID() const
    {
        return kNameSpaceID_XUL;
    }

protected:
    // pseudo constants
    static PRInt32 gRefCnt;

    static nsIAtom** kIdentityAttrs[];

    static nsIRDFService* gRDFService;
    static nsIRDFResource* kNC_persist;
    static nsIRDFResource* kNC_attribute;
    static nsIRDFResource* kNC_value;

    static nsXULPrototypeCache* gXULCache;

    static PRLogModuleInfo* gXULLog;

    nsresult
    Persist(nsIContent* aElement, PRInt32 aNameSpaceID, nsIAtom* aAttribute);

    // IMPORTANT: The ownership implicit in the following member
    // variables has been explicitly checked and set using nsCOMPtr
    // for owning pointers and raw COM interface pointers for weak
    // (ie, non owning) references. If you add any members to this
    // class, please make the ownership explicit (pinkerton, scc).
    // NOTE, THIS IS STILL IN PROGRESS, TALK TO PINK OR SCC BEFORE
    // CHANGING

    nsXULDocument*             mNextSrcLoadWaiter;  // [OWNER] but not COMPtr

    nsElementMap               mElementMap;
    nsCOMPtr<nsIRDFDataSource> mLocalStore;
    PRPackedBool               mIsPopup;
    PRPackedBool               mApplyingPersistedAttrs;
    PRPackedBool               mIsWritingFastLoad;
    PRPackedBool               mDocumentLoaded;
    /**
     * Since ResumeWalk is interruptible, it's possible that last
     * stylesheet finishes loading while the PD walk is still in
     * progress (waiting for an overlay to finish loading).
     * mStillWalking prevents DoneLoading (and StartLayout) from being
     * called in this situation.
     */
    PRPackedBool               mStillWalking;

    /**
     * An array of style sheets, that will be added (preserving order) to the
     * document after all of them are loaded (in DoneWalking).
     */
    nsCOMArray<nsICSSStyleSheet> mOverlaySheets;

    nsCOMPtr<nsIDOMXULCommandDispatcher>     mCommandDispatcher; // [OWNER] of the focus tracker

    // Maintains the template builders that have been attached to
    // content elements
    typedef nsInterfaceHashtable<nsISupportsHashKey, nsIXULTemplateBuilder>
        BuilderTable;
    BuilderTable* mTemplateBuilderTable;

    PRUint32 mPendingSheets;

    /*
     * XXX dr
     * ------
     * We used to have two pointers into the content model: mPopupNode and
     * mTooltipNode, which were used to retrieve the objects triggering a
     * popup or tooltip. You need that access because your reference has
     * disappeared by the time you click on a popup item or do whatever
     * with a tooltip. These were owning references (no cycles, as pinkerton
     * pointed out, since we're still parent-child).
     *
     * We still have mTooltipNode, but mPopupNode has moved to the
     * FocusController. The APIs (IDL attributes popupNode and tooltipNode)
     * are still here for compatibility and ease of use, but we should
     * probably move the mTooltipNode over to FocusController at some point
     * as well, for consistency.
     */

    nsCOMPtr<nsIDOMNode>    mTooltipNode;          // [OWNER] element triggering the tooltip

    /**
     * Context stack, which maintains the state of the Builder and allows
     * it to be interrupted.
     */
    class ContextStack {
    protected:
        struct Entry {
            nsXULPrototypeElement* mPrototype;
            nsIContent*            mElement;
            PRInt32                mIndex;
            Entry*                 mNext;
        };

        Entry* mTop;
        PRInt32 mDepth;

    public:
        ContextStack();
        ~ContextStack();

        PRInt32 Depth() { return mDepth; }

        nsresult Push(nsXULPrototypeElement* aPrototype, nsIContent* aElement);
        nsresult Pop();
        nsresult Peek(nsXULPrototypeElement** aPrototype, nsIContent** aElement, PRInt32* aIndex);

        nsresult SetTopIndex(PRInt32 aIndex);

        PRBool IsInsideXULTemplate();
    };

    friend class ContextStack;
    ContextStack mContextStack;

    enum State { eState_Master, eState_Overlay };
    State mState;

    /**
     * An array of overlay nsIURIs that have yet to be resolved. The
     * order of the array is significant: overlays at the _end_ of the
     * array are resolved before overlays earlier in the array (i.e.,
     * it is a stack).
     *
     * In the current implementation the order the overlays are loaded
     * in is as follows: first overlays from xul-overlay PIs, in the
     * same order as in the document, then the overlays from the chrome
     * registry.
     */
    nsCOMArray<nsIURI> mUnloadedOverlays;

    /**
     * Load the transcluded script at the specified URI. If the
     * prototype construction must 'block' until the load has
     * completed, aBlock will be set to true.
     */
    nsresult LoadScript(nsXULPrototypeScript *aScriptProto, PRBool* aBlock);

    /**
     * Execute the precompiled script object scoped by this XUL document's
     * containing window object, and using its associated script context.
     */
    nsresult ExecuteScript(nsIScriptContext *aContext, void* aScriptObject);

    /**
     * Helper method for the above that uses aScript to find the appropriate
     * script context and object.
     */
    nsresult ExecuteScript(nsXULPrototypeScript *aScript);

    /**
     * Create a delegate content model element from a prototype.
     * Note that the resulting content node is not bound to any tree
     */
    nsresult CreateElementFromPrototype(nsXULPrototypeElement* aPrototype,
                                        nsIContent** aResult);

    /**
     * Create a hook-up element to which content nodes can be attached for
     * later resolution.
     */
    nsresult CreateOverlayElement(nsXULPrototypeElement* aPrototype, nsIContent** aResult);

    /**
     * Add attributes from the prototype to the element.
     */
    nsresult AddAttributes(nsXULPrototypeElement* aPrototype, nsIContent* aElement);

    /**
     * The prototype-script of the current transcluded script that is being
     * loaded.  For document.write('<script src="nestedwrite.js"><\/script>')
     * to work, these need to be in a stack element type, and we need to hold
     * the top of stack here.
     */
    nsXULPrototypeScript* mCurrentScriptProto;

    /**
     * Check if a XUL template builder has already been hooked up.
     */
    static nsresult
    CheckTemplateBuilderHookup(nsIContent* aElement, PRBool* aNeedsHookup);

    /**
     * Create a XUL template builder on the specified node.
     */
    static nsresult
    CreateTemplateBuilder(nsIContent* aElement);

    /**
     * Add the current prototype's style sheets (currently it's just
     * style overlays from the chrome registry) to the document.
     */
    nsresult AddPrototypeSheets();


protected:
    /* Declarations related to forward references. 
     *
     * Forward references are declarations which are added to the temporary
     * list (mForwardReferences) during the document (or overlay) load and
     * are resolved later, when the document loading is almost complete.
     */

    /**
     * The list of different types of forward references to resolve. After
     * a reference is resolved, it is removed from this array (and
     * automatically deleted)
     */
    nsTArray<nsAutoPtr<nsForwardReference> > mForwardReferences;

    /** Indicates what kind of forward references are still to be processed. */
    nsForwardReference::Phase mResolutionPhase;

    /**
     * Adds aRef to the mForwardReferences array. Takes the ownership of aRef.
     */
    nsresult AddForwardReference(nsForwardReference* aRef);

    /**
     * Resolve all of the document's forward references.
     */
    nsresult ResolveForwardReferences();

    /**
     * Used to resolve broadcaster references
     */
    class BroadcasterHookup : public nsForwardReference
    {
    protected:
        nsXULDocument* mDocument;              // [WEAK]
        nsCOMPtr<nsIContent> mObservesElement; // [OWNER]
        PRBool mResolved;

    public:
        BroadcasterHookup(nsXULDocument* aDocument,
                          nsIContent* aObservesElement)
            : mDocument(aDocument),
              mObservesElement(aObservesElement),
              mResolved(PR_FALSE)
        {
        }

        virtual ~BroadcasterHookup();

        virtual Phase GetPhase() { return eHookup; }
        virtual Result Resolve();
    };

    friend class BroadcasterHookup;


    /**
     * Used to hook up overlays
     */
    class OverlayForwardReference : public nsForwardReference
    {
    protected:
        nsXULDocument* mDocument;      // [WEAK]
        nsCOMPtr<nsIContent> mOverlay; // [OWNER]
        PRBool mResolved;

        nsresult Merge(nsIContent* aTargetNode, nsIContent* aOverlayNode, PRBool aNotify);

    public:
        OverlayForwardReference(nsXULDocument* aDocument, nsIContent* aOverlay)
            : mDocument(aDocument), mOverlay(aOverlay), mResolved(PR_FALSE) {}

        virtual ~OverlayForwardReference();

        virtual Phase GetPhase() { return eConstruction; }
        virtual Result Resolve();
    };

    friend class OverlayForwardReference;

    class TemplateBuilderHookup : public nsForwardReference
    {
    protected:
        nsCOMPtr<nsIContent> mElement; // [OWNER]

    public:
        TemplateBuilderHookup(nsIContent* aElement)
            : mElement(aElement) {}

        virtual Phase GetPhase() { return eHookup; }
        virtual Result Resolve();
    };

    friend class TemplateBuilderHookup;

    // The out params of FindBroadcaster only have values that make sense when
    // the method returns NS_FINDBROADCASTER_FOUND.  In all other cases, the
    // values of the out params should not be relied on (though *aListener and
    // *aBroadcaster do need to be released if non-null, of course).
    nsresult
    FindBroadcaster(nsIContent* aElement,
                    nsIDOMElement** aListener,
                    nsString& aBroadcasterID,
                    nsString& aAttribute,
                    nsIDOMElement** aBroadcaster);

    nsresult
    CheckBroadcasterHookup(nsIContent* aElement,
                           PRBool* aNeedsHookup,
                           PRBool* aDidResolve);

    void
    SynchronizeBroadcastListener(nsIDOMElement   *aBroadcaster,
                                 nsIDOMElement   *aListener,
                                 const nsAString &aAttr);

    static
    nsresult
    InsertElement(nsIContent* aParent, nsIContent* aChild, PRBool aNotify);

    static 
    nsresult
    RemoveElement(nsIContent* aParent, nsIContent* aChild);

    /**
     * The current prototype that we are walking to construct the
     * content model.
     */
    nsRefPtr<nsXULPrototypeDocument> mCurrentPrototype;

    /**
     * The master document (outermost, .xul) prototype, from which
     * all subdocuments get their security principals.
     */
    nsRefPtr<nsXULPrototypeDocument> mMasterPrototype;

    /**
     * Owning references to all of the prototype documents that were
     * used to construct this document.
     */
    nsTArray< nsRefPtr<nsXULPrototypeDocument> > mPrototypes;

    /**
     * Prepare to walk the current prototype.
     */
    nsresult PrepareToWalk();

    /**
     * Creates a processing instruction based on aProtoPI and inserts
     * it to the DOM (as the aIndex-th child of aParent).
     */
    nsresult
    CreateAndInsertPI(const nsXULPrototypePI* aProtoPI,
                      nsINode* aParent, PRUint32 aIndex);

    /**
     * Inserts the passed <?xml-stylesheet ?> PI at the specified
     * index. Loads and applies the associated stylesheet
     * asynchronously.
     * The prototype document walk can happen before the stylesheets
     * are loaded, but the final steps in the load process (see
     * DoneWalking()) are not run before all the stylesheets are done
     * loading.
     */
    nsresult
    InsertXMLStylesheetPI(const nsXULPrototypePI* aProtoPI,
                          nsINode* aParent,
                          PRUint32 aIndex,
                          nsIContent* aPINode);

    /**
     * Inserts the passed <?xul-overlay ?> PI at the specified index.
     * Schedules the referenced overlay URI for further processing.
     */
    nsresult
    InsertXULOverlayPI(const nsXULPrototypePI* aProtoPI,
                       nsINode* aParent,
                       PRUint32 aIndex,
                       nsIContent* aPINode);

    /**
     * Add overlays from the chrome registry to the set of unprocessed
     * overlays still to do.
     */
    nsresult AddChromeOverlays();

    /**
     * Resume (or initiate) an interrupted (or newly prepared)
     * prototype walk.
     */
    nsresult ResumeWalk();

    /**
     * Called at the end of ResumeWalk() and from StyleSheetLoaded().
     * Expects that both the prototype document walk is complete and
     * all referenced stylesheets finished loading.
     */
    nsresult DoneWalking();

    /**
     * Report that an overlay failed to load
     * @param aURI the URI of the overlay that failed to load
     */
    void ReportMissingOverlay(nsIURI* aURI);
    
#if defined(DEBUG_waterson) || defined(DEBUG_hyatt)
    // timing
    nsTime mLoadStart;
#endif

    class CachedChromeStreamListener : public nsIStreamListener {
    protected:
        nsXULDocument* mDocument;
        PRPackedBool   mProtoLoaded;

        virtual ~CachedChromeStreamListener();

    public:
        CachedChromeStreamListener(nsXULDocument* aDocument,
                                   PRBool aProtoLoaded);

        NS_DECL_ISUPPORTS
        NS_DECL_NSIREQUESTOBSERVER
        NS_DECL_NSISTREAMLISTENER
    };

    friend class CachedChromeStreamListener;


    class ParserObserver : public nsIRequestObserver {
    protected:
        nsRefPtr<nsXULDocument> mDocument;
        nsRefPtr<nsXULPrototypeDocument> mPrototype;
        virtual ~ParserObserver();

    public:
        ParserObserver(nsXULDocument* aDocument,
                       nsXULPrototypeDocument* aPrototype);

        NS_DECL_ISUPPORTS
        NS_DECL_NSIREQUESTOBSERVER
    };

    friend class ParserObserver;

    /**
     * A map from a broadcaster element to a list of listener elements.
     */
    PLDHashTable* mBroadcasterMap;

    nsInterfaceHashtable<nsURIHashKey,nsIObserver> mOverlayLoadObservers;
    nsInterfaceHashtable<nsURIHashKey,nsIObserver> mPendingOverlayLoadNotifications;
    
    PRBool mInitialLayoutComplete;
private:
    // helpers

};

#endif // nsXULDocument_h__
