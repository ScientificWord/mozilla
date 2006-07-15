/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
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

/* A namespace class for static content utilities. */

#ifndef nsContentUtils_h___
#define nsContentUtils_h___

#include "jspubtd.h"
#include "nsAString.h"
#include "nsIStatefulFrame.h"
#include "nsIPref.h"
#include "nsINodeInfo.h"
#include "nsNodeInfoManager.h"
#include "nsContentList.h"
#include "nsDOMClassInfoID.h"
#include "nsIClassInfo.h"
#include "nsIDOM3Node.h"

class nsIDOMScriptObjectFactory;
class nsIXPConnect;
class nsINode;
class nsIContent;
class nsIDOMNode;
class nsIDocument;
class nsIDocShell;
class nsINameSpaceManager;
class nsIScriptSecurityManager;
class nsIJSContextStack;
class nsIThreadJSContextStack;
class nsIParserService;
class nsIIOService;
class nsIURI;
class imgIDecoderObserver;
class imgIRequest;
class imgILoader;
class nsIPrefBranch;
class nsIImage;
class nsIImageLoadingContent;
class nsIDOMHTMLFormElement;
class nsIDOMDocument;
class nsIConsoleService;
class nsIStringBundleService;
class nsIStringBundle;
class nsIContentPolicy;
class nsILineBreaker;
class nsIWordBreaker;
class nsIJSRuntimeService;
class nsIEventListenerManager;
class nsIScriptContext;
class nsIScriptGlobalObject;
template<class E> class nsCOMArray;
class nsIPref;
class nsVoidArray;
struct JSRuntime;
#ifdef MOZ_XTF
class nsIXTFService;
#endif

extern const char kLoadAsData[];

class nsContentUtils
{
public:
  static nsresult Init();

  // You MUST pass the old ownerDocument of aContent in as aOldDocument and the
  // new one as aNewDocument.  aNewParent is allowed to be null; in that case
  // aNewDocument will be assumed to be the parent.  Note that at this point
  // the actual ownerDocument of aContent may not yet be aNewDocument.
  // XXXbz but then if it gets wrapped after we do this call but before its
  // ownerDocument actually changes, things will break...
  static nsresult ReparentContentWrapper(nsIContent *aNode,
                                         nsIContent *aNewParent,
                                         nsIDocument *aNewDocument,
                                         nsIDocument *aOldDocument);

  /**
   * When a document's scope changes (e.g., from document.open(), call this
   * function to move all content wrappers from the old scope to the new one.
   */
  static nsresult ReparentContentWrappersInScope(nsIScriptGlobalObject *aOldScope,
                                                 nsIScriptGlobalObject *aNewScope);

  static PRBool   IsCallerChrome();

  static PRBool   IsCallerTrustedForRead();

  static PRBool   IsCallerTrustedForWrite();

  /*
   * Returns true if the nodes are both in the same document or
   * if neither is in a document.
   * Returns false if the nodes are not in the same document.
   */
  static PRBool   InSameDoc(nsIDOMNode *aNode,
                            nsIDOMNode *aOther);

  /**
   * Do not ever pass null pointers to this method.  If one of your
   * nsIContents is null, you have to decide for yourself what
   * "IsDescendantOf" really means.
   *
   * @param  aPossibleDescendant node to test for being a descendant of
   *         aPossibleAncestor
   * @param  aPossibleAncestor node to test for being an ancestor of
   *         aPossibleDescendant
   * @return PR_TRUE if aPossibleDescendant is a descendant of
   *         aPossibleAncestor (or is aPossibleAncestor).  PR_FALSE
   *         otherwise.
   */
  static PRBool ContentIsDescendantOf(nsINode* aPossibleDescendant,
                                      nsINode* aPossibleAncestor);

  /*
   * This method fills the |aArray| with all ancestor nodes of |aNode|
   * including |aNode| at the zero index.
   *
   * These elements were |nsIDOMNode*|s before casting to |void*| and must
   * be cast back to |nsIDOMNode*| on usage, or bad things will happen.
   */
  static nsresult GetAncestors(nsIDOMNode* aNode,
                               nsVoidArray* aArray);

  /*
   * This method fills |aAncestorNodes| with all ancestor nodes of |aNode|
   * including |aNode| (QI'd to nsIContent) at the zero index.
   * For each ancestor, there is a corresponding element in |aAncestorOffsets|
   * which is the IndexOf the child in relation to its parent.
   *
   * The elements of |aAncestorNodes| were |nsIContent*|s before casting to
   * |void*| and must be cast back to |nsIContent*| on usage, or bad things
   * will happen.
   *
   * This method just sucks.
   */
  static nsresult GetAncestorsAndOffsets(nsIDOMNode* aNode,
                                         PRInt32 aOffset,
                                         nsVoidArray* aAncestorNodes,
                                         nsVoidArray* aAncestorOffsets);

  /*
   * The out parameter, |aCommonAncestor| will be the closest node, if any,
   * to both |aNode| and |aOther| which is also an ancestor of each.
   * Returns an error if the two nodes are disconnected and don't have
   * a common ancestor.
   */
  static nsresult GetCommonAncestor(nsIDOMNode *aNode,
                                    nsIDOMNode *aOther,
                                    nsIDOMNode** aCommonAncestor);

  /**
   * Returns the common ancestor, if any, for two nodes. Returns null if the
   * nodes are disconnected.
   */
  static nsINode* GetCommonAncestor(nsINode* aNode1,
                                    nsINode* aNode2);

  /**
   * Compares the document position of nodes.
   *
   * @param aNode1 The node whose position is being compared to the reference
   *               node
   * @param aNode2 The reference node
   *
   * @return  The document position flags of the nodes. aNode1 is compared to
   *          aNode2, i.e. if aNode1 is before aNode2 then
   *          DOCUMENT_POSITION_PRECEDING will be set.
   *
   * @see nsIDOMNode
   * @see nsIDOM3Node
   */
  static PRUint16 ComparePosition(nsINode* aNode1,
                                  nsINode* aNode2);

  /**
   * Returns true if aNode1 is before aNode2 in the same connected
   * tree.
   */
  static PRBool PositionIsBefore(nsINode* aNode1,
                                 nsINode* aNode2)
  {
    return (ComparePosition(aNode1, aNode2) &
      (nsIDOM3Node::DOCUMENT_POSITION_PRECEDING |
       nsIDOM3Node::DOCUMENT_POSITION_DISCONNECTED)) ==
      nsIDOM3Node::DOCUMENT_POSITION_PRECEDING;
  }

  /**
   * Brute-force search of the element subtree rooted at aContent for
   * an element with the given id.  aId must be nonempty, otherwise
   * this method may return nodes even if they have no id!
   */
  static nsIContent* MatchElementId(nsIContent *aContent,
                                    const nsAString& aId);

  /**
   * Similar to above, but to be used if one already has an atom for the ID
   */
  static nsIContent* MatchElementId(nsIContent *aContent,
                                    nsIAtom* aId);

  /**
   * Given a URI containing an element reference (#whatever),
   * resolve it to the target content element with the given ID.
   *
   * If aFromContent is anonymous XBL content then the URI
   * must refer to its binding document and we will return
   * a node in the same anonymous content subtree as aFromContent,
   * if one exists with the correct ID.
   *
   * @param aFromContent the context of the reference;
   *   currently we only support references to elements in the
   *   same document as the context, so this must be non-null
   *
   * @return the element, or nsnull on failure
   */
  static nsIContent* GetReferencedElement(nsIURI* aURI,
                                          nsIContent *aFromContent);

  /**
   * Reverses the document position flags passed in.
   *
   * @param   aDocumentPosition   The document position flags to be reversed.
   *
   * @return  The reversed document position flags.
   *
   * @see nsIDOMNode
   * @see nsIDOM3Node
   */
  static PRUint16 ReverseDocumentPosition(PRUint16 aDocumentPosition);

  static PRUint32 CopyNewlineNormalizedUnicodeTo(const nsAString& aSource,
                                                 PRUint32 aSrcOffset,
                                                 PRUnichar* aDest,
                                                 PRUint32 aLength,
                                                 PRBool& aLastCharCR);

  static PRUint32 CopyNewlineNormalizedUnicodeTo(nsReadingIterator<PRUnichar>& aSrcStart, const nsReadingIterator<PRUnichar>& aSrcEnd, nsAString& aDest);

  static nsISupports *
  GetClassInfoInstance(nsDOMClassInfoID aID);

  static const nsDependentSubstring TrimCharsInSet(const char* aSet,
                                                   const nsAString& aValue);

  static const nsDependentSubstring TrimWhitespace(const nsAString& aStr,
                                                   PRBool aTrimTrailing = PR_TRUE);

  static void Shutdown();

  /**
   * Checks whether two nodes come from the same origin. aTrustedNode is
   * considered 'safe' in that a user can operate on it and that it isn't
   * a js-object that implements nsIDOMNode.
   * Never call this function with the first node provided by script, it
   * must always be known to be a 'real' node!
   */
  static nsresult CheckSameOrigin(nsIDOMNode* aTrustedNode,
                                  nsIDOMNode* aUnTrustedNode);

  // Check if the (JS) caller can access aNode.
  static PRBool CanCallerAccess(nsIDOMNode *aNode);

  /**
   * Get the docshell through the JS context that's currently on the stack.
   * If there's no JS context currently on the stack aDocShell will be null.
   *
   * @param aDocShell The docshell or null if no JS context
   */
  static nsIDocShell *GetDocShellFromCaller();

  /**
   * The two GetDocumentFrom* functions below allow a caller to get at a
   * document that is relevant to the currently executing script.
   *
   * GetDocumentFromCaller gets its document by looking at the last called
   * function and finding the document that the function itself relates to.
   * For example, consider two windows A and B in the same origin. B has a
   * function which does something that ends up needing the current document.
   * If a script in window A were to call B's function, GetDocumentFromCaller
   * would find that function (in B) and return B's document.
   *
   * GetDocumentFromContext gets its document by looking at the currently
   * executing context's global object and returning its document. Thus,
   * given the example above, GetDocumentFromCaller would see that the
   * currently executing script was in window A, and return A's document.
   */
  /**
   * Get the document from the currently executing function. This will return
   * the document that the currently executing function is in/from.
   *
   * @return The document or null if no JS Context.
   */
  static nsIDOMDocument *GetDocumentFromCaller();

  /**
   * Get the document through the JS context that's currently on the stack.
   * If there's no JS context currently on the stack it will return null.
   * This will return the document of the calling script.
   *
   * @return The document or null if no JS context
   */
  static nsIDOMDocument *GetDocumentFromContext();

  // Check if a node is in the document prolog, i.e. before the document
  // element.
  static PRBool InProlog(nsINode *aNode);

  static nsIParserService* GetParserService();

  static nsINameSpaceManager* NameSpaceManager()
  {
    return sNameSpaceManager;
  }

  static nsIIOService* GetIOService()
  {
    return sIOService;
  }

  static imgILoader* GetImgLoader()
  {
    return sImgLoader;
  }

#ifdef MOZ_XTF
  static nsIXTFService* GetXTFService();
#endif

  /**
   * Get the cache security manager service. Can return null if the layout
   * module has been shut down.
   */
  static nsIScriptSecurityManager* GetSecurityManager()
  {
    return sSecurityManager;
  }

  static nsresult GenerateStateKey(nsIContent* aContent,
                                   nsIDocument* aDocument,
                                   nsIStatefulFrame::SpecialStateID aID,
                                   nsACString& aKey);

  /**
   * Create a new nsIURI from aSpec, using aBaseURI as the base.  The
   * origin charset of the new nsIURI will be the document charset of
   * aDocument.
   */
  static nsresult NewURIWithDocumentCharset(nsIURI** aResult,
                                            const nsAString& aSpec,
                                            nsIDocument* aDocument,
                                            nsIURI* aBaseURI);

  /**
   * Convert aInput (in charset aCharset) to UTF16 in aOutput.
   *
   * @param aCharset the name of the charset; if empty, we assume UTF8
   */
  static nsresult ConvertStringFromCharset(const nsACString& aCharset,
                                           const nsACString& aInput,
                                           nsAString& aOutput);

  /**
   * Determine whether aContent is in some way associated with aForm.  If the
   * form is a container the only elements that are considered to be associated
   * with a form are the elements that are contained within the form. If the
   * form is a leaf element then all elements will be accepted into this list,
   * since this can happen due to content fixup when a form spans table rows or
   * table cells.
   */
  static PRBool BelongsInForm(nsIDOMHTMLFormElement *aForm,
                              nsIContent *aContent);

  static nsresult CheckQName(const nsAString& aQualifiedName,
                             PRBool aNamespaceAware = PR_TRUE);

  static nsresult SplitQName(nsIContent* aNamespaceResolver,
                             const nsAFlatString& aQName,
                             PRInt32 *aNamespace, nsIAtom **aLocalName);

  static nsresult LookupNamespaceURI(nsIContent* aNamespaceResolver,
                                     const nsAString& aNamespacePrefix,
                                     nsAString& aNamespaceURI);

  static nsresult GetNodeInfoFromQName(const nsAString& aNamespaceURI,
                                       const nsAString& aQualifiedName,
                                       nsNodeInfoManager* aNodeInfoManager,
                                       nsINodeInfo** aNodeInfo);

  static void SplitExpatName(const PRUnichar *aExpatName, nsIAtom **aPrefix,
                             nsIAtom **aTagName, PRInt32 *aNameSpaceID);

  static nsAdoptingCString GetCharPref(const char *aPref);
  static PRPackedBool GetBoolPref(const char *aPref,
                                  PRBool aDefault = PR_FALSE);
  static PRInt32 GetIntPref(const char *aPref, PRInt32 aDefault = 0);
  static nsAdoptingString GetLocalizedStringPref(const char *aPref);
  static nsAdoptingString GetStringPref(const char *aPref);
  static void RegisterPrefCallback(const char *aPref,
                                   PrefChangedFunc aCallback,
                                   void * aClosure);
  static void UnregisterPrefCallback(const char *aPref,
                                     PrefChangedFunc aCallback,
                                     void * aClosure);
  static nsIPrefBranch *GetPrefBranch()
  {
    return sPrefBranch;
  }

  static nsILineBreaker* LineBreaker()
  {
    return sLineBreaker;
  }

  static nsIWordBreaker* WordBreaker()
  {
    return sWordBreaker;
  }

  /**
   * @return PR_TRUE if aContent has an attribute aName in namespace aNameSpaceID,
   * and the attribute value is non-empty.
   */
  static PRBool HasNonEmptyAttr(nsIContent* aContent, PRInt32 aNameSpaceID,
                                nsIAtom* aName);

  /**
   * Method to do security and content policy checks on the image URI
   *
   * @param aURI uri of the image to be loaded
   * @param aContext the context the image is loaded in (eg an element)
   * @param aLoadingDocument the document we belong to
   * @param aImageBlockingStatus the nsIContentPolicy blocking status for this
   *        image.  This will be set even if a security check fails for the
   *        image, to some reasonable REJECT_* value.  This out param will only
   *        be set if it's non-null.
   * @return PR_TRUE if the load can proceed, or PR_FALSE if it is blocked.
   *         Note that aImageBlockingStatus, if set will always be an ACCEPT
   *         status if PR_TRUE is returned and always be a REJECT_* status if
   *         PR_FALSE is returned.
   */
  static PRBool CanLoadImage(nsIURI* aURI,
                             nsISupports* aContext,
                             nsIDocument* aLoadingDocument,
                             PRInt16* aImageBlockingStatus = nsnull);
  /**
   * Method to start an image load.  This does not do any security checks.
   *
   * @param aURI uri of the image to be loaded
   * @param aLoadingDocument the document we belong to
   * @param aObserver the observer for the image load
   * @param aLoadFlags the load flags to use.  See nsIRequest
   * @return the imgIRequest for the image load
   */
  static nsresult LoadImage(nsIURI* aURI,
                            nsIDocument* aLoadingDocument,
                            nsIURI* aReferrer,
                            imgIDecoderObserver* aObserver,
                            PRInt32 aLoadFlags,
                            imgIRequest** aRequest);

  /**
   * Method to get an nsIImage from an image loading content
   *
   * @param aContent The image loading content.  Must not be null.
   * @param aRequest The image request [out]
   * @return the nsIImage corresponding to the first frame of the image
   */
  static already_AddRefed<nsIImage> GetImageFromContent(nsIImageLoadingContent* aContent, imgIRequest **aRequest = nsnull);

  /**
   * Method that decides whether a content node is draggable
   *
   * @param aContent The content node to test.
   * @return whether it's draggable
   */
  static PRBool ContentIsDraggable(nsIContent* aContent) {
    return IsDraggableImage(aContent) || IsDraggableLink(aContent);
  }

  /**
   * Method that decides whether a content node is a draggable image
   *
   * @param aContent The content node to test.
   * @return whether it's a draggable image
   */
  static PRBool IsDraggableImage(nsIContent* aContent);

  /**
   * Method that decides whether a content node is a draggable link
   *
   * @param aContent The content node to test.
   * @return whether it's a draggable link
   */
  static PRBool IsDraggableLink(nsIContent* aContent);

  /**
   * Method that gets the URI of the link content.  If the content
   * isn't a link, return null.
   *
   * @param aContent The link content
   * @return the URI the link points to
   */
  static already_AddRefed<nsIURI> GetLinkURI(nsIContent* aContent);

  /**
   * Method that gets the XLink uri for a content node, if it's an XLink
   *
   * @param aContent The content node, possibly an XLink
   * @return Null if aContent is not an XLink, the URI it points to otherwise
   */
  static already_AddRefed<nsIURI> GetXLinkURI(nsIContent* aContent);

  /**
   * Convenience method to create a new nodeinfo that differs only by name
   * from aNodeInfo.
   */
  static nsresult NameChanged(nsINodeInfo *aNodeInfo, nsIAtom *aName,
                              nsINodeInfo** aResult)
  {
    nsNodeInfoManager *niMgr = aNodeInfo->NodeInfoManager();

    return niMgr->GetNodeInfo(aName, aNodeInfo->GetPrefixAtom(),
                              aNodeInfo->NamespaceID(), aResult);
  }

  /**
   * Convenience method to create a new nodeinfo that differs only by prefix
   * from aNodeInfo.
   */
  static nsresult PrefixChanged(nsINodeInfo *aNodeInfo, nsIAtom *aPrefix,
                                nsINodeInfo** aResult)
  {
    nsNodeInfoManager *niMgr = aNodeInfo->NodeInfoManager();

    return niMgr->GetNodeInfo(aNodeInfo->NameAtom(), aPrefix,
                              aNodeInfo->NamespaceID(), aResult);
  }

  /**
   * Returns the appropriate event argument names for the specified
   * namespace and event name.  Added because we need to switch between
   * SVG's "evt" and the rest of the world's "event", and because onerror
   * takes 3 args.
   */
  static void GetEventArgNames(PRInt32 aNameSpaceID, nsIAtom *aEventName,
                               PRUint32 *aArgCount, const char*** aArgNames);

  /**
   * Return the nsIXPConnect service.
   */
  static nsIXPConnect *XPConnect()
  {
    return sXPConnect;
  }

  /**
   * Report a localized error message to the error console.
   *   @param aFile Properties file containing localized message.
   *   @param aMessageName Name of localized message.
   *   @param aParams Parameters to be substituted into localized message.
   *   @param aParamsLength Length of aParams.
   *   @param aURI URI of resource containing error (may be null).
   *   @param aSourceLine The text of the line that contains the error (may be
              empty).
   *   @param aLineNumber Line number within resource containing error.
   *   @param aColumnNumber Column number within resource containing error.
   *   @param aErrorFlags See nsIScriptError.
   *   @param aCategory Name of module reporting error.
   */
  enum PropertiesFile {
    eCSS_PROPERTIES,
    eXBL_PROPERTIES,
    eXUL_PROPERTIES,
    eLAYOUT_PROPERTIES,
    eFORMS_PROPERTIES,
    ePRINTING_PROPERTIES,
    eDOM_PROPERTIES,
    eBRAND_PROPERTIES,
    eCOMMON_DIALOG_PROPERTIES,
    PropertiesFile_COUNT
  };
  static nsresult ReportToConsole(PropertiesFile aFile,
                                  const char *aMessageName,
                                  const PRUnichar **aParams,
                                  PRUint32 aParamsLength,
                                  nsIURI* aURI,
                                  const nsAFlatString& aSourceLine,
                                  PRUint32 aLineNumber,
                                  PRUint32 aColumnNumber,
                                  PRUint32 aErrorFlags,
                                  const char *aCategory);

  /**
   * Get the localized string named |aKey| in properties file |aFile|.
   */
  static nsresult GetLocalizedString(PropertiesFile aFile,
                                     const char* aKey,
                                     nsXPIDLString& aResult);

  /**
   * Fill (with the parameters given) the localized string named |aKey| in
   * properties file |aFile|.
   */
  static nsresult FormatLocalizedString(PropertiesFile aFile,
                                        const char* aKey,
                                        const PRUnichar **aParams,
                                        PRUint32 aParamsLength,
                                        nsXPIDLString& aResult);

  /**
   * Returns true if aDocument is a chrome document
   */
  static PRBool IsChromeDoc(nsIDocument *aDocument);

  /**
   * Notify XPConnect if an exception is pending on aCx.
   */
  static void NotifyXPCIfExceptionPending(JSContext *aCx);

  /**
   * Release *aSupportsPtr when the shutdown notification is received
   */
  static nsresult ReleasePtrOnShutdown(nsISupports** aSupportsPtr) {
    NS_ASSERTION(aSupportsPtr, "Expect to crash!");
    NS_ASSERTION(*aSupportsPtr, "Expect to crash!");
    return sPtrsToPtrsToRelease->AppendElement(aSupportsPtr) ? NS_OK :
      NS_ERROR_OUT_OF_MEMORY;
  }

  /**
   * Return the content policy service
   */
  static nsIContentPolicy *GetContentPolicy();

  /**
   * Make sure that whatever value *aPtr contains at any given moment is
   * protected from JS GC until we remove the GC root.  A call to this that
   * succeeds MUST be matched by a call to RemoveJSGCRoot to avoid leaking.
   */
  static nsresult AddJSGCRoot(jsval* aPtr, const char* aName) {
    return AddJSGCRoot((void*)aPtr, aName);
  }

  /**
   * Make sure that whatever object *aPtr is pointing to at any given moment is
   * protected from JS GC until we remove the GC root.  A call to this that
   * succeeds MUST be matched by a call to RemoveJSGCRoot to avoid leaking.
   */
  static nsresult AddJSGCRoot(JSObject** aPtr, const char* aName) {
    return AddJSGCRoot((void*)aPtr, aName);
  }

  /**
   * Make sure that whatever object *aPtr is pointing to at any given moment is
   * protected from JS GC until we remove the GC root.  A call to this that
   * succeeds MUST be matched by a call to RemoveJSGCRoot to avoid leaking.
   */
  static nsresult AddJSGCRoot(void* aPtr, const char* aName);  

  /**
   * Remove aPtr as a JS GC root
   */
  static nsresult RemoveJSGCRoot(jsval* aPtr) {
    return RemoveJSGCRoot((void*)aPtr);
  }
  static nsresult RemoveJSGCRoot(JSObject** aPtr) {
    return RemoveJSGCRoot((void*)aPtr);
  }
  static nsresult RemoveJSGCRoot(void* aPtr);

  /**
   * Quick helper to determine whether there are any mutation listeners
   * of a given type that apply to this content or any of its ancestors.
   *
   * @param aNode  The node to search for listeners
   * @param aType  The type of listener (NS_EVENT_BITS_MUTATION_*)
   *
   * @return true if there are mutation listeners of the specified type
   */
  static PRBool HasMutationListeners(nsINode* aNode,
                                     PRUint32 aType);

  /**
   * This method creates and dispatches a trusted event.
   * Works only with events which can be created by calling
   * nsIDOMDocumentEvent::CreateEvent() with parameter "Events".
   * @param aDoc           The document which will be used to create the event.
   * @param aTarget        The target of the event, should be QIable to
   *                       nsIDOMEventTarget.
   * @param aEventName     The name of the event.
   * @param aCanBubble     Whether the event can bubble.
   * @param aCancelable    Is the event cancelable.
   * @param aDefaultAction Set to true if default action should be taken,
   *                       see nsIDOMEventTarget::DispatchEvent.
   */
  static nsresult DispatchTrustedEvent(nsIDocument* aDoc,
                                       nsISupports* aTarget,
                                       const nsAString& aEventName,
                                       PRBool aCanBubble,
                                       PRBool aCancelable,
                                       PRBool *aDefaultAction = nsnull);

  /**
   * Add aRange to the list of ranges with a start- or endpoint containing
   * aNode. aCreated will be set to PR_TRUE if this call created a new list
   * (meaning the list was empty before the call to AddToRangeList).
   *
   * @param aNode The node contained in the start- or endpoint of aRange.
   * @param aRange The range containing aNode in its start- or endpoint.
   * @param aCreated [out] Set to PR_TRUE if a new list was created.
   */
  static nsresult AddToRangeList(nsINode *aNode, nsIDOMRange *aRange,
                                 PRBool *aCreated);

  /**
   * Remove aRange from the list of ranges with a start- or endpoint containing
   * aNode. This will return PR_TRUE if aRange was the last range in the list.
   *
   * @param aNode The node for which to remove aRange.
   * @param aRange The range to remove.
   * @return PR_TRUE if aRange was the last range in the list.
   */
  static PRBool RemoveFromRangeList(nsINode *aNode, nsIDOMRange *aRange);

  /**
   * Look up the list of ranges containing aNode.
   *
   * @param aNode The node for which to look up the range list.
   * @return The range list if one exists.
   */
  static const nsVoidArray* LookupRangeList(const nsINode *aNode);

  /**
   * Remove the list of ranges containing aNode as their start- or endpoint.
   *
   * @param aNode The node for which to remove the range list.
   */
  static void RemoveRangeList(nsINode *aNode);

  /**
   * Get the eventlistener manager for aNode. If a new eventlistener manager
   * was created, aCreated is set to PR_TRUE.
   *
   * @param aNode The node for which to get the eventlistener manager.
   * @param aCreateIfNotFound If PR_FALSE, returns a listener manager only if
   *                          one already exists.
   * @param aResult [out] Set to the eventlistener manager for aNode.
   * @param aCreated [out] Set to PR_TRUE if a new eventlistener manager was
   *                       created.
   */
  static nsresult GetListenerManager(nsINode *aNode,
                                     PRBool aCreateIfNotFound,
                                     nsIEventListenerManager **aResult,
                                     PRBool *aCreated);

  /**
   * Remove the eventlistener manager for aNode.
   *
   * @param aNode The node for which to remove the eventlistener manager.
   */
  static void RemoveListenerManager(nsINode *aNode);

  static PRBool IsInitialized()
  {
    return sInitialized;
  }

  /**
   * Checks if the localname/prefix/namespace triple is valid wrt prefix
   * and namespace according to the Namespaces in XML and DOM Code
   * specfications.
   *
   * @param aLocalname localname of the node
   * @param aPrefix prefix of the node
   * @param aNamespaceID namespace of the node
   */
  static PRBool IsValidNodeName(nsIAtom *aLocalName, nsIAtom *aPrefix,
                                PRInt32 aNamespaceID);

  /**
   * Associate an object aData to aKey on node aNode. If aData is null any
   * previously registered object and UserDataHandler associated to aKey on
   * aNode will be removed.
   * Should only be used to implement the DOM Level 3 UserData API.
   *
   * @param aNode canonical nsINode pointer of the node to add aData to
   * @param aKey the key to associate the object to
   * @param aData the object to associate to aKey on aNode (may be nulll)
   * @param aHandler the UserDataHandler to call when the node is
   *                 cloned/deleted/imported/renamed (may be nulll)
   * @param aResult [out] the previously registered object for aKey on aNode, if
   *                      any
   * @return whether adding the object and UserDataHandler succeeded
   */
  static nsresult SetUserData(nsINode *aNode, nsIAtom *aKey, nsIVariant *aData,
                              nsIDOMUserDataHandler *aHandler,
                              nsIVariant **aResult);

  /**
   * Call the UserDataHandler associated with aKey on node aNode.
   * Should only be used to implement the DOM Level 3 UserData API.
   *
   * @param aDocument the document that contains the property table for aNode
   * @param aOperation the type of operation that is being performed on the
   *                   node. @see nsIDOMUserDataHandler
   * @param aNode canonical nsINode pointer of the node to call the
   *                UserDataHandler for
   * @param aSource the node that aOperation is being performed on, or null if
   *                the operation is a deletion
   * @param aDest the newly created node if any, or null
   */
  static void CallUserDataHandler(nsIDocument *aDocument, PRUint16 aOperation,
                                  const nsINode *aNode, nsIDOMNode *aSource,
                                  nsIDOMNode *aDest);

  /**
   * Copy the objects and UserDataHandlers for node aNode from aOldDocument to
   * the current ownerDocument of aNode.
   * Should only be used to implement the DOM Level 3 UserData API.
   *
   * @param aOldDocument the old document
   * @param aNode canonical nsINode pointer of the node to copy objects
   *              and UserDataHandlers for
   */
  static void CopyUserData(nsIDocument *aOldDocument, const nsINode *aNode);

  /**
   * Creates a DocumentFragment from text using a context node to resolve
   * namespaces.
   *
   * @param aContextNode the node which is used to resolve namespaces
   * @param aFragment the string which is parsed to a DocumentFragment
   * @param aReturn [out] the created DocumentFragment
   */
  static nsresult CreateContextualFragment(nsIDOMNode* aContextNode,
                                           const nsAString& aFragment,
                                           nsIDOMDocumentFragment** aReturn);

  /**
   * Creates a new XML document, setting the document's container to be the
   * docshell of whatever script is on the JSContext stack.
   *
   * @param aNamespaceURI Namespace for the root element to create and insert in
   *                      the document. Only used if aQualifiedName is not
   *                      empty.
   * @param aQualifiedName Qualified name for the root element to create and
   *                       insert in the document. If empty no root element will
   *                       be created.
   * @param aDoctype Doctype node to insert in the document.
   * @param aDocumentURI URI of the document. Must not be null.
   * @param aBaseURI Base URI of the document. Must not be null.
   * @param aPrincipal Prinicpal of the document. Must not be null.
   * @param aResult [out] The document that was created.
   */
  static nsresult CreateDocument(const nsAString& aNamespaceURI, 
                                 const nsAString& aQualifiedName, 
                                 nsIDOMDocumentType* aDoctype,
                                 nsIURI* aDocumentURI,
                                 nsIURI* aBaseURI,
                                 nsIPrincipal* aPrincipal,
                                 nsIDOMDocument** aResult);

private:
  static nsresult doReparentContentWrapper(nsIContent *aChild,
                                           JSContext *cx,
                                           JSObject *aOldGlobal,
                                           JSObject *aNewGlobal);

  static nsresult EnsureStringBundle(PropertiesFile aFile);

  static nsIDOMScriptObjectFactory *sDOMScriptObjectFactory;

  static nsIXPConnect *sXPConnect;

  static nsIScriptSecurityManager *sSecurityManager;

  static nsIThreadJSContextStack *sThreadJSContextStack;

  static nsIParserService *sParserService;

  static nsINameSpaceManager *sNameSpaceManager;

  static nsIIOService *sIOService;

#ifdef MOZ_XTF
  static nsIXTFService *sXTFService;
#endif

  static nsIPrefBranch *sPrefBranch;

  static nsIPref *sPref;

  static imgILoader* sImgLoader;

  static nsIConsoleService* sConsoleService;

  static nsIStringBundleService* sStringBundleService;
  static nsIStringBundle* sStringBundles[PropertiesFile_COUNT];

  static nsIContentPolicy* sContentPolicyService;
  static PRBool sTriedToGetContentPolicy;

  static nsILineBreaker* sLineBreaker;
  static nsIWordBreaker* sWordBreaker;

  // Holds pointers to nsISupports* that should be released at shutdown
  static nsVoidArray* sPtrsToPtrsToRelease;

  // For now, we don't want to automatically clean this up in Shutdown(), since
  // consumers might unfortunately end up wanting to use it after that
  static nsIJSRuntimeService* sJSRuntimeService;
  static JSRuntime* sScriptRuntime;
  static PRInt32 sScriptRootCount;
  
  static PRBool sInitialized;
};


class nsCxPusher
{
public:
  nsCxPusher(nsISupports *aCurrentTarget);
  ~nsCxPusher();

  void Push(nsISupports *aCurrentTarget);
  void Pop();

private:
  nsCOMPtr<nsIJSContextStack> mStack;
  nsCOMPtr<nsIScriptContext> mScx;
  PRBool mScriptIsRunning;
};

class nsAutoGCRoot {
public:
  // aPtr should be the pointer to the jsval we want to protect
  nsAutoGCRoot(jsval* aPtr, nsresult* aResult) :
    mPtr(aPtr)
  {
    mResult = *aResult =
      nsContentUtils::AddJSGCRoot(aPtr, "nsAutoGCRoot");
  }

  // aPtr should be the pointer to the JSObject* we want to protect
  nsAutoGCRoot(JSObject** aPtr, nsresult* aResult) :
    mPtr(aPtr)
  {
    mResult = *aResult =
      nsContentUtils::AddJSGCRoot(aPtr, "nsAutoGCRoot");
  }

  // aPtr should be the pointer to the thing we want to protect
  nsAutoGCRoot(void* aPtr, nsresult* aResult) :
    mPtr(aPtr)
  {
    mResult = *aResult =
      nsContentUtils::AddJSGCRoot(aPtr, "nsAutoGCRoot");
  }

  ~nsAutoGCRoot() {
    if (NS_SUCCEEDED(mResult)) {
      nsContentUtils::RemoveJSGCRoot(mPtr);
    }
  }

private:
  void* mPtr;
  nsresult mResult;
};

#define NS_AUTO_GCROOT_PASTE2(tok,line) tok##line
#define NS_AUTO_GCROOT_PASTE(tok,line) \
  NS_AUTO_GCROOT_PASTE2(tok,line)
#define NS_AUTO_GCROOT(ptr, result) \ \
  nsAutoGCRoot NS_AUTO_GCROOT_PASTE(_autoGCRoot_, __LINE__) \
  (ptr, result)

#define NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(_class)                      \
  if (aIID.Equals(NS_GET_IID(nsIClassInfo))) {                                \
    foundInterface =                                                          \
      nsContentUtils::GetClassInfoInstance(eDOMClassInfo_##_class##_id);      \
    if (!foundInterface) {                                                    \
      *aInstancePtr = nsnull;                                                 \
      return NS_ERROR_OUT_OF_MEMORY;                                          \
    }                                                                         \
  } else

#define NS_INTERFACE_MAP_ENTRY_TEAROFF(_interface, _allocator)                \
  if (aIID.Equals(NS_GET_IID(_interface))) {                                  \
    foundInterface = NS_STATIC_CAST(_interface *, _allocator);                \
    if (!foundInterface) {                                                    \
      *aInstancePtr = nsnull;                                                 \
      return NS_ERROR_OUT_OF_MEMORY;                                          \
    }                                                                         \
  } else

#endif /* nsContentUtils_h___ */
