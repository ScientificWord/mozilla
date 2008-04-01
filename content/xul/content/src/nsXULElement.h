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
 *   Peter Annema <disttsc@bart.nl>
 *   Mike Shaver <shaver@mozilla.org>
 *   Ben Goodger <ben@netscape.com>
 *   Mark Hammond <mhammond@skippinet.com.au>
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

  The base XUL element class and associates.

*/

#ifndef nsXULElement_h__
#define nsXULElement_h__

// XXX because nsIEventListenerManager has broken includes
#include "nsIDOMEvent.h"
#include "nsIServiceManager.h"
#include "nsIAtom.h"
#include "nsINodeInfo.h"
#include "nsIControllers.h"
#include "nsICSSParser.h"
#include "nsIDOMElement.h"
#include "nsIDOMEventTarget.h"
#include "nsIDOM3EventTarget.h"
#include "nsIDOMXULElement.h"
#include "nsIDOMXULMultSelectCntrlEl.h"
#include "nsIEventListenerManager.h"
#include "nsIRDFCompositeDataSource.h"
#include "nsIRDFResource.h"
#include "nsIScriptObjectOwner.h"
#include "nsBindingManager.h"
#include "nsIURI.h"
#include "nsIXULTemplateBuilder.h"
#include "nsIBoxObject.h"
#include "nsIXBLService.h"
#include "nsICSSOMFactory.h"
#include "nsLayoutCID.h"
#include "nsAttrAndChildArray.h"
#include "nsGkAtoms.h"
#include "nsAutoPtr.h"
#include "nsGenericElement.h"
#include "nsDOMScriptObjectHolder.h"

class nsIDocument;
class nsString;
class nsIDocShell;
class nsICSSStyleRule;

class nsIObjectInputStream;
class nsIObjectOutputStream;
class nsIScriptGlobalObjectOwner;

static NS_DEFINE_CID(kCSSParserCID, NS_CSSPARSER_CID);

////////////////////////////////////////////////////////////////////////

#ifdef XUL_PROTOTYPE_ATTRIBUTE_METERING
#define XUL_PROTOTYPE_ATTRIBUTE_METER(counter) (nsXULPrototypeAttribute::counter++)
#else
#define XUL_PROTOTYPE_ATTRIBUTE_METER(counter) ((void) 0)
#endif


/**

  A prototype attribute for an nsXULPrototypeElement.

 */

class nsXULPrototypeAttribute
{
public:
    nsXULPrototypeAttribute()
        : mName(nsGkAtoms::id),  // XXX this is a hack, but names have to have a value
          mEventHandler(nsnull)
    {
        XUL_PROTOTYPE_ATTRIBUTE_METER(gNumAttributes);
        MOZ_COUNT_CTOR(nsXULPrototypeAttribute);
    }

    ~nsXULPrototypeAttribute();

    nsAttrName mName;
    nsAttrValue mValue;
    // mEventHandler is only valid for the language ID specified in the
    // containing nsXULPrototypeElement.  We would ideally use
    // nsScriptObjectHolder, but want to avoid the extra lang ID.
    void* mEventHandler;

#ifdef XUL_PROTOTYPE_ATTRIBUTE_METERING
    /**
      If enough attributes, on average, are event handlers, it pays to keep
      mEventHandler here, instead of maintaining a separate mapping in each
      nsXULElement associating those mName values with their mEventHandlers.
      Assume we don't need to keep mNameSpaceID along with mName in such an
      event-handler-only name-to-function-pointer mapping.

      Let
        minAttrSize  = sizeof(mNodeInof) + sizeof(mValue)
        mappingSize  = sizeof(mNodeInfo) + sizeof(mEventHandler)
        elemOverhead = nElems * sizeof(MappingPtr)

      Then
        nAttrs * minAttrSize + nEventHandlers * mappingSize + elemOverhead
        > nAttrs * (minAttrSize + mappingSize - sizeof(mNodeInfo))
      which simplifies to
        nEventHandlers * mappingSize + elemOverhead
        > nAttrs * (mappingSize - sizeof(mNodeInfo))
      or
        nEventHandlers + (nElems * sizeof(MappingPtr)) / mappingSize
        > nAttrs * (1 - sizeof(mNodeInfo) / mappingSize)

      If nsCOMPtr and all other pointers are the same size, this reduces to
        nEventHandlers + nElems / 2 > nAttrs / 2

      To measure how many attributes are event handlers, compile XUL source
      with XUL_PROTOTYPE_ATTRIBUTE_METERING and watch the counters below.
      Plug into the above relation -- if true, it pays to put mEventHandler
      in nsXULPrototypeAttribute rather than to keep a separate mapping.

      Recent numbers after opening four browser windows:
        nElems 3537, nAttrs 2528, nEventHandlers 1042
      giving 1042 + 3537/2 > 2528/2 or 2810 > 1264.

      As it happens, mEventHandler also makes this struct power-of-2 sized,
      8 words on most architectures, which makes for strength-reduced array
      index-to-pointer calculations.
     */
    static PRUint32   gNumElements;
    static PRUint32   gNumAttributes;
    static PRUint32   gNumEventHandlers;
    static PRUint32   gNumCacheTests;
    static PRUint32   gNumCacheHits;
    static PRUint32   gNumCacheSets;
    static PRUint32   gNumCacheFills;
#endif /* !XUL_PROTOTYPE_ATTRIBUTE_METERING */
};


/**

  A prototype content model element that holds the "primordial" values
  that have been parsed from the original XUL document. A
  'lightweight' nsXULElement may delegate its representation to this
  structure, which is shared.

 */

class nsXULPrototypeNode
{
public:
    enum Type { eType_Element, eType_Script, eType_Text, eType_PI };

    Type                     mType;

    nsAutoRefCnt             mRefCnt;

    virtual ~nsXULPrototypeNode() {}
    virtual nsresult Serialize(nsIObjectOutputStream* aStream,
                               nsIScriptGlobalObject* aGlobal,
                               const nsCOMArray<nsINodeInfo> *aNodeInfos) = 0;
    virtual nsresult Deserialize(nsIObjectInputStream* aStream,
                                 nsIScriptGlobalObject* aGlobal,
                                 nsIURI* aDocumentURI,
                                 const nsCOMArray<nsINodeInfo> *aNodeInfos) = 0;

#ifdef NS_BUILD_REFCNT_LOGGING
    virtual const char* ClassName() = 0;
    virtual PRUint32 ClassSize() = 0;
#endif

    void AddRef() {
        ++mRefCnt;
        NS_LOG_ADDREF(this, mRefCnt, ClassName(), ClassSize());
    }
    void Release()
    {
        --mRefCnt;
        NS_LOG_RELEASE(this, mRefCnt, ClassName());
        if (mRefCnt == 0)
            delete this;
    }
    /**
     * The prototype document must call ReleaseSubtree when it is going
     * away.  This makes the parents through the tree stop owning their
     * children, whether or not the parent's reference count is zero.
     * Individual elements may still own individual prototypes, but
     * those prototypes no longer remember their children to allow them
     * to be constructed.
     */
    virtual void ReleaseSubtree() { Release(); }

    NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_NATIVE_CLASS(nsXULPrototypeNode)

protected:
    nsXULPrototypeNode(Type aType)
        : mType(aType), mRefCnt(1) {}
};

class nsXULPrototypeElement : public nsXULPrototypeNode
{
public:
    nsXULPrototypeElement()
        : nsXULPrototypeNode(eType_Element),
          mNumChildren(0),
          mChildren(nsnull),
          mNumAttributes(0),
          mAttributes(nsnull),
          mHasIdAttribute(PR_FALSE),
          mHasClassAttribute(PR_FALSE),
          mHasStyleAttribute(PR_FALSE),
          mHoldsScriptObject(PR_FALSE),
          mScriptTypeID(nsIProgrammingLanguage::UNKNOWN)
    {
        NS_LOG_ADDREF(this, 1, ClassName(), ClassSize());
    }

    virtual ~nsXULPrototypeElement()
    {
        UnlinkJSObjects();
        Unlink();
        NS_ASSERTION(!mChildren && mNumChildren == 0,
                     "ReleaseSubtree not called");
    }

#ifdef NS_BUILD_REFCNT_LOGGING
    virtual const char* ClassName() { return "nsXULPrototypeElement"; }
    virtual PRUint32 ClassSize() { return sizeof(*this); }
#endif

    virtual void ReleaseSubtree()
    {
      if (mChildren) {
        for (PRInt32 i = mNumChildren-1; i >= 0; i--) {
          if (mChildren[i])
            mChildren[i]->ReleaseSubtree();
        }
        mNumChildren = 0;
        delete[] mChildren;
        mChildren = nsnull;
      }

      nsXULPrototypeNode::ReleaseSubtree();
    }

    virtual nsresult Serialize(nsIObjectOutputStream* aStream,
                               nsIScriptGlobalObject* aGlobal,
                               const nsCOMArray<nsINodeInfo> *aNodeInfos);
    virtual nsresult Deserialize(nsIObjectInputStream* aStream,
                                 nsIScriptGlobalObject* aGlobal,
                                 nsIURI* aDocumentURI,
                                 const nsCOMArray<nsINodeInfo> *aNodeInfos);

    nsresult SetAttrAt(PRUint32 aPos, const nsAString& aValue, nsIURI* aDocumentURI);

    void UnlinkJSObjects();
    void Unlink();

    PRUint32                 mNumChildren;
    nsXULPrototypeNode**     mChildren;           // [OWNER]

    nsCOMPtr<nsINodeInfo>    mNodeInfo;           // [OWNER]

    PRUint32                 mNumAttributes;
    nsXULPrototypeAttribute* mAttributes;         // [OWNER]
    
    PRPackedBool             mHasIdAttribute:1;
    PRPackedBool             mHasClassAttribute:1;
    PRPackedBool             mHasStyleAttribute:1;
    PRPackedBool             mHoldsScriptObject:1;

    // The language ID can not be set on a per-node basis, but is tracked
    // so that the language ID from the originating root can be used
    // (eg, when a node from an overlay ends up in our document, that node
    // must use its original script language, not our document's default.
    PRUint16                 mScriptTypeID;
    static void ReleaseGlobals()
    {
        NS_IF_RELEASE(sCSSParser);
    }

protected:
    static nsICSSParser* GetCSSParser()
    {
        if (!sCSSParser) {
            CallCreateInstance(kCSSParserCID, &sCSSParser);
            if (sCSSParser) {
                sCSSParser->SetCaseSensitive(PR_TRUE);
                sCSSParser->SetQuirkMode(PR_FALSE);
            }
        }
        return sCSSParser;
    }
    static nsICSSParser* sCSSParser;
};

class nsXULDocument;

class nsXULPrototypeScript : public nsXULPrototypeNode
{
public:
    nsXULPrototypeScript(PRUint32 aLangID, PRUint32 aLineNo, PRUint32 version);
    virtual ~nsXULPrototypeScript();

#ifdef NS_BUILD_REFCNT_LOGGING
    virtual const char* ClassName() { return "nsXULPrototypeScript"; }
    virtual PRUint32 ClassSize() { return sizeof(*this); }
#endif

    virtual nsresult Serialize(nsIObjectOutputStream* aStream,
                               nsIScriptGlobalObject* aGlobal,
                               const nsCOMArray<nsINodeInfo> *aNodeInfos);
    nsresult SerializeOutOfLine(nsIObjectOutputStream* aStream,
                                nsIScriptGlobalObject* aGlobal);
    virtual nsresult Deserialize(nsIObjectInputStream* aStream,
                                 nsIScriptGlobalObject* aGlobal,
                                 nsIURI* aDocumentURI,
                                 const nsCOMArray<nsINodeInfo> *aNodeInfos);
    nsresult DeserializeOutOfLine(nsIObjectInputStream* aInput,
                                  nsIScriptGlobalObject* aGlobal);

    nsresult Compile(const PRUnichar* aText, PRInt32 aTextLength,
                     nsIURI* aURI, PRUint32 aLineNo,
                     nsIDocument* aDocument,
                     nsIScriptGlobalObjectOwner* aGlobalOwner);

    void UnlinkJSObjects()
    {
        if (mScriptObject.mObject) {
            nsContentUtils::DropScriptObjects(mScriptObject.mLangID, this,
                                              &NS_CYCLE_COLLECTION_NAME(nsXULPrototypeNode));
            mScriptObject.mObject = nsnull;
        }
    }

    void Set(nsScriptObjectHolder &aHolder)
    {
        NS_ASSERTION(mScriptObject.mLangID == aHolder.getScriptTypeID(),
                     "Wrong language, this will leak the previous object.");

        mScriptObject.mLangID = aHolder.getScriptTypeID();
        Set((void*)aHolder);
    }
    void Set(void *aObject)
    {
        NS_ASSERTION(!mScriptObject.mObject, "Leaking script object.");

        nsresult rv = nsContentUtils::HoldScriptObject(mScriptObject.mLangID,
                                                       this,
                                                       &NS_CYCLE_COLLECTION_NAME(nsXULPrototypeNode),
                                                       aObject, PR_FALSE);
        if (NS_SUCCEEDED(rv)) {
            mScriptObject.mObject = aObject;
        }
    }

    struct ScriptObjectHolder
    {
        ScriptObjectHolder(PRUint32 aLangID) : mLangID(aLangID),
                                               mObject(nsnull)
        {
        }
        PRUint32 mLangID;
        void* mObject;
    };
    nsCOMPtr<nsIURI>         mSrcURI;
    PRUint32                 mLineNo;
    PRPackedBool             mSrcLoading;
    PRPackedBool             mOutOfLine;
    nsXULDocument*           mSrcLoadWaiters;   // [OWNER] but not COMPtr
    PRUint32                 mLangVersion;
    ScriptObjectHolder       mScriptObject;
};

class nsXULPrototypeText : public nsXULPrototypeNode
{
public:
    nsXULPrototypeText()
        : nsXULPrototypeNode(eType_Text)
    {
        NS_LOG_ADDREF(this, 1, ClassName(), ClassSize());
    }

    virtual ~nsXULPrototypeText()
    {
    }

#ifdef NS_BUILD_REFCNT_LOGGING
    virtual const char* ClassName() { return "nsXULPrototypeText"; }
    virtual PRUint32 ClassSize() { return sizeof(*this); }
#endif

    virtual nsresult Serialize(nsIObjectOutputStream* aStream,
                               nsIScriptGlobalObject* aGlobal,
                               const nsCOMArray<nsINodeInfo> *aNodeInfos);
    virtual nsresult Deserialize(nsIObjectInputStream* aStream,
                                 nsIScriptGlobalObject* aGlobal,
                                 nsIURI* aDocumentURI,
                                 const nsCOMArray<nsINodeInfo> *aNodeInfos);

    nsString                 mValue;
};

class nsXULPrototypePI : public nsXULPrototypeNode
{
public:
    nsXULPrototypePI()
        : nsXULPrototypeNode(eType_PI)
    {
        NS_LOG_ADDREF(this, 1, ClassName(), ClassSize());
    }

    virtual ~nsXULPrototypePI()
    {
    }

#ifdef NS_BUILD_REFCNT_LOGGING
    virtual const char* ClassName() { return "nsXULPrototypePI"; }
    virtual PRUint32 ClassSize() { return sizeof(*this); }
#endif

    virtual nsresult Serialize(nsIObjectOutputStream* aStream,
                               nsIScriptGlobalObject* aGlobal,
                               const nsCOMArray<nsINodeInfo> *aNodeInfos);
    virtual nsresult Deserialize(nsIObjectInputStream* aStream,
                                 nsIScriptGlobalObject* aGlobal,
                                 nsIURI* aDocumentURI,
                                 const nsCOMArray<nsINodeInfo> *aNodeInfos);

    nsString                 mTarget;
    nsString                 mData;
};

////////////////////////////////////////////////////////////////////////

/**

  The XUL element.

 */

#define XUL_ELEMENT_LAZY_STATE_OFFSET NODE_TYPE_SPECIFIC_BITS_OFFSET

class nsScriptEventHandlerOwnerTearoff;

class nsXULElement : public nsGenericElement, public nsIDOMXULElement
{
public:
    /**
     * These flags are used to maintain bookkeeping information for partially-
     * constructed content.
     *
     *   eChildrenMustBeRebuilt
     *     The element's children are invalid or unconstructed, and should
     *     be reconstructed.
     *
     *   eTemplateContentsBuilt
     *     Child content that is built from a XUL template has been
     *     constructed. 
     *
     *   eContainerContentsBuilt
     *     Child content that is built by following the ``containment''
     *     property in a XUL template has been built.
     */
    enum LazyState {
        eChildrenMustBeRebuilt  = 0x1,
        eTemplateContentsBuilt  = 0x2,
        eContainerContentsBuilt = 0x4
    };

    /** Typesafe, non-refcounting cast from nsIContent.  Cheaper than QI. **/
    static nsXULElement* FromContent(nsIContent *aContent)
    {
        if (aContent->IsNodeOfType(eXUL))
            return static_cast<nsXULElement*>(aContent);
        return nsnull;
    }

public:
    static nsIXBLService* GetXBLService() {
        if (!gXBLService)
            CallGetService("@mozilla.org/xbl;1", &gXBLService);
        return gXBLService;
    }
    static void ReleaseGlobals() {
        NS_IF_RELEASE(gXBLService);
        NS_IF_RELEASE(gCSSOMFactory);
    }

protected:
    // pseudo-constants
    static nsIXBLService*       gXBLService;
    static nsICSSOMFactory*     gCSSOMFactory;

public:
    static nsresult
    Create(nsXULPrototypeElement* aPrototype, nsIDocument* aDocument,
           PRBool aIsScriptable, nsIContent** aResult);

    // nsISupports
    NS_DECL_ISUPPORTS_INHERITED
    NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED_NO_UNLINK(nsXULElement,
                                                       nsGenericElement)

    // nsINode
    virtual PRUint32 GetChildCount() const;
    virtual nsIContent *GetChildAt(PRUint32 aIndex) const;
    virtual PRInt32 IndexOf(nsINode* aPossibleChild) const;
    virtual nsresult PreHandleEvent(nsEventChainPreVisitor& aVisitor);
    virtual nsresult InsertChildAt(nsIContent* aKid, PRUint32 aIndex,
                                   PRBool aNotify);

    // nsIContent
    virtual void UnbindFromTree(PRBool aDeep, PRBool aNullParent);
    virtual nsresult RemoveChildAt(PRUint32 aIndex, PRBool aNotify);
    virtual nsIAtom *GetIDAttributeName() const;
    virtual nsIAtom *GetClassAttributeName() const;
    virtual PRBool GetAttr(PRInt32 aNameSpaceID, nsIAtom* aName,
                           nsAString& aResult) const;
    virtual PRBool HasAttr(PRInt32 aNameSpaceID, nsIAtom* aName) const;
    virtual PRBool AttrValueIs(PRInt32 aNameSpaceID, nsIAtom* aName,
                               const nsAString& aValue,
                               nsCaseTreatment aCaseSensitive) const;
    virtual PRBool AttrValueIs(PRInt32 aNameSpaceID, nsIAtom* aName,
                               nsIAtom* aValue,
                               nsCaseTreatment aCaseSensitive) const;
    virtual PRInt32 FindAttrValueIn(PRInt32 aNameSpaceID,
                                    nsIAtom* aName,
                                    AttrValuesArray* aValues,
                                    nsCaseTreatment aCaseSensitive) const;
    virtual nsresult UnsetAttr(PRInt32 aNameSpaceID, nsIAtom* aName,
                               PRBool aNotify);
    virtual const nsAttrName* GetAttrNameAt(PRUint32 aIndex) const;
    virtual PRUint32 GetAttrCount() const;
    virtual void DestroyContent();

#ifdef DEBUG
    virtual void List(FILE* out, PRInt32 aIndent) const;
    virtual void DumpContent(FILE* out, PRInt32 aIndent,PRBool aDumpAll) const
    {
    }
#endif

    virtual void SetFocus(nsPresContext* aPresContext);
    virtual void RemoveFocus(nsPresContext* aPresContext);
    virtual void PerformAccesskey(PRBool aKeyCausesActivation,
                                  PRBool aIsTrustedEvent);

    virtual nsIContent *GetBindingParent() const;
    virtual PRBool IsNodeOfType(PRUint32 aFlags) const;
    virtual PRBool IsFocusable(PRInt32 *aTabIndex = nsnull);
    virtual nsIAtom* GetID() const;
    virtual const nsAttrValue* GetClasses() const;

    NS_IMETHOD WalkContentStyleRules(nsRuleWalker* aRuleWalker);
    virtual nsICSSStyleRule* GetInlineStyleRule();
    NS_IMETHOD SetInlineStyleRule(nsICSSStyleRule* aStyleRule, PRBool aNotify);
    virtual nsChangeHint GetAttributeChangeHint(const nsIAtom* aAttribute,
                                                PRInt32 aModType) const;
    NS_IMETHOD_(PRBool) IsAttributeMapped(const nsIAtom* aAttribute) const;

    // XUL element methods
    PRUint32 PeekChildCount() const
    { return mAttrsAndChildren.ChildCount(); }
    void SetLazyState(LazyState aFlags)
    { SetFlags(aFlags << XUL_ELEMENT_LAZY_STATE_OFFSET); }
    void ClearLazyState(LazyState aFlags)
    { UnsetFlags(aFlags << XUL_ELEMENT_LAZY_STATE_OFFSET); }
    PRBool GetLazyState(LazyState aFlag)
    { return !!(GetFlags() & (aFlag << XUL_ELEMENT_LAZY_STATE_OFFSET)); }

    // nsIDOMNode
    NS_FORWARD_NSIDOMNODE(nsGenericElement::)

    // nsIDOMElement
    NS_FORWARD_NSIDOMELEMENT(nsGenericElement::)

    // nsIDOMXULElement
    NS_DECL_NSIDOMXULELEMENT

    virtual nsresult Clone(nsINodeInfo *aNodeInfo, nsINode **aResult) const;
    virtual PRInt32 IntrinsicState() const;

    nsresult GetStyle(nsIDOMCSSStyleDeclaration** aStyle);

    virtual void RecompileScriptEventListeners();

    // This function should ONLY be used by BindToTree implementations.
    // The function exists solely because XUL elements store the binding
    // parent as a member instead of in the slots, as nsGenericElement does.
    void SetXULBindingParent(nsIContent* aBindingParent)
    {
      mBindingParent = aBindingParent;
    }

protected:
    // XXX This can be removed when nsNodeUtils::CloneAndAdopt doesn't need
    //     access to mPrototype anymore.
    friend class nsNodeUtils;

    nsXULElement(nsINodeInfo* aNodeInfo);

    // Implementation methods
    nsresult EnsureContentsGenerated(void) const;

    nsresult ExecuteOnBroadcastHandler(nsIDOMElement* anElement, const nsAString& attrName);

    static nsresult
    ExecuteJSCode(nsIDOMElement* anElement, nsEvent* aEvent);

    // Helper routine that crawls a parent chain looking for a tree element.
    NS_IMETHOD GetParentTree(nsIDOMXULMultiSelectControlElement** aTreeElement);

    nsresult AddPopupListener(nsIAtom* aName);

    class nsXULSlots : public nsGenericElement::nsDOMSlots
    {
    public:
       nsXULSlots(PtrBits aFlags);
       virtual ~nsXULSlots();
    };

    virtual nsINode::nsSlots* CreateSlots();

    // Required fields
    nsRefPtr<nsXULPrototypeElement>     mPrototype;

    /**
     * The nearest enclosing content node with a binding
     * that created us. [Weak]
     */
    nsIContent*                         mBindingParent;

    /**
     * Abandon our prototype linkage, and copy all attributes locally
     */
    nsresult MakeHeavyweight();

    /**
     * Get the attr info for the given namespace ID and attribute name.
     * The namespace ID must not be kNameSpaceID_Unknown and the name
     * must not be null.
     */
    virtual nsAttrInfo GetAttrInfo(PRInt32 aNamespaceID, nsIAtom* aName) const;

    const nsAttrValue* FindLocalOrProtoAttr(PRInt32 aNameSpaceID,
                                            nsIAtom *aName) const {
        return nsXULElement::GetAttrInfo(aNameSpaceID, aName).mValue;
    }

    virtual nsresult BeforeSetAttr(PRInt32 aNamespaceID, nsIAtom* aName,
                                   const nsAString* aValue, PRBool aNotify);
    virtual nsresult AfterSetAttr(PRInt32 aNamespaceID, nsIAtom* aName,
                                  const nsAString* aValue, PRBool aNotify);

    virtual PRBool ParseAttribute(PRInt32 aNamespaceID,
                                  nsIAtom* aAttribute,
                                  const nsAString& aValue,
                                  nsAttrValue& aResult);

    virtual nsresult
      GetEventListenerManagerForAttr(nsIEventListenerManager** aManager,
                                     nsISupports** aTarget,
                                     PRBool* aDefer);
  
    /**
     * Return our prototype's attribute, if one exists.
     */
    nsXULPrototypeAttribute *FindPrototypeAttribute(PRInt32 aNameSpaceID,
                                                    nsIAtom *aName) const;
    /**
     * Add a listener for the specified attribute, if appropriate.
     */
    void AddListenerFor(const nsAttrName& aName,
                        PRBool aCompileEventHandlers);
    void MaybeAddPopupListener(nsIAtom* aLocalName);


    nsresult HideWindowChrome(PRBool aShouldHide);

    void SetTitlebarColor(nscolor aColor);

    const nsAttrName* InternalGetExistingAttrNameFromQName(const nsAString& aStr) const;

    void RemoveBroadcaster(const nsAString & broadcasterId);

protected:
    // Internal accessor. This shadows the 'Slots', and returns
    // appropriate value.
    nsIControllers *Controllers() {
      nsDOMSlots* slots = GetExistingDOMSlots();
      return slots ? slots->mControllers : nsnull; 
    }

    void UnregisterAccessKey(const nsAString& aOldValue);
    PRBool BoolAttrIsTrue(nsIAtom* aName);

    friend nsresult
    NS_NewXULElement(nsIContent** aResult, nsINodeInfo *aNodeInfo);

    static already_AddRefed<nsXULElement>
    Create(nsXULPrototypeElement* aPrototype, nsINodeInfo *aNodeInfo,
           PRBool aIsScriptable);

    friend class nsScriptEventHandlerOwnerTearoff;
};

#endif // nsXULElement_h__
