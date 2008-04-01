/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set sw=2 ts=2 et tw=80: */
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
 *   Pierre Phaneuf <pp@ludusdesign.com>
 *   Kathleen Brade <brade@netscape.com>
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
#include "nsICharsetAlias.h"

#include "nsCOMPtr.h"
#include "nsXPIDLString.h"
#include "nsPrintfCString.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsHTMLDocument.h"
#include "nsIParserFilter.h"
#include "nsIHTMLContentSink.h"
#include "nsIXMLContentSink.h"
#include "nsHTMLParts.h"
#include "nsHTMLStyleSheet.h"
#include "nsGkAtoms.h"
#include "nsIPresShell.h"
#include "nsPresContext.h"
#include "nsIDOMNode.h" // for Find
#include "nsIDOMNodeList.h"
#include "nsIDOMElement.h"
#include "nsIDOMText.h"
#include "nsIDOMComment.h"
#include "nsIDOMDOMImplementation.h"
#include "nsIDOMDocumentType.h"
#include "nsPIDOMWindow.h"
#include "nsIDOMHTMLFormElement.h"
#include "nsDOMString.h"
#include "nsIStreamListener.h"
#include "nsIURI.h"
#include "nsIIOService.h"
#include "nsNetUtil.h"
#include "nsIContentViewerContainer.h"
#include "nsIContentViewer.h"
#include "nsIMarkupDocumentViewer.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsDocShellLoadTypes.h"
#include "nsIWebNavigation.h"
#include "nsIBaseWindow.h"
#include "nsIWebShellServices.h"
#include "nsIScriptContext.h"
#include "nsIXPConnect.h"
#include "nsContentList.h"
#include "nsDOMError.h"
#include "nsIPrincipal.h"
#include "nsIScriptSecurityManager.h"
#include "nsAttrName.h"
#include "nsNodeUtils.h"

#include "nsNetCID.h"
#include "nsIIOService.h"
#include "nsICookieService.h"

#include "nsIServiceManager.h"
#include "nsIConsoleService.h"
#include "nsIComponentManager.h"
#include "nsParserCIID.h"
#include "nsIDOMHTMLElement.h"
#include "nsIDOMHTMLMapElement.h"
#include "nsIDOMHTMLBodyElement.h"
#include "nsINameSpaceManager.h"
#include "nsGenericHTMLElement.h"
#include "nsGenericDOMNodeList.h"
#include "nsICSSLoader.h"
#include "nsIHttpChannel.h"
#include "nsIFile.h"
#include "nsIEventListenerManager.h"
#include "nsISelectElement.h"
#include "nsFrameSelection.h"
#include "nsISelectionPrivate.h"//for toStringwithformat code

#include "nsICharsetDetector.h"
#include "nsICharsetDetectionAdaptor.h"
#include "nsCharsetDetectionAdaptorCID.h"
#include "nsICharsetAlias.h"
#include "nsContentUtils.h"
#include "nsJSUtils.h"
#include "nsIDocumentCharsetInfo.h"
#include "nsIDocumentEncoder.h" //for outputting selection
#include "nsICharsetResolver.h"
#include "nsICachingChannel.h"
#include "nsICacheEntryDescriptor.h"
#include "nsIJSContextStack.h"
#include "nsIDocumentViewer.h"
#include "nsIWyciwygChannel.h"
#include "nsIScriptElement.h"
#include "nsIScriptError.h"
#include "nsIMutableArray.h"
#include "nsArrayUtils.h"
#include "nsIEffectiveTLDService.h"
#include "nsIEventStateManager.h"

#include "nsIPrompt.h"
//AHMED 12-2
#include "nsBidiUtils.h"

#include "nsIEditingSession.h"
#include "nsIEditor.h"
#include "nsNodeInfoManager.h"
#include "nsIEditor.h"
#include "nsIEditorDocShell.h"
#include "nsIEditorStyleSheets.h"
#include "nsIInlineSpellChecker.h"
#include "nsRange.h"

#define NS_MAX_DOCUMENT_WRITE_DEPTH 20

#define DETECTOR_CONTRACTID_MAX 127
static char g_detector_contractid[DETECTOR_CONTRACTID_MAX + 1];
static PRBool gInitDetector = PR_FALSE;
static PRBool gPlugDetector = PR_FALSE;

#include "prmem.h"
#include "prtime.h"

// Find/Search Includes
const PRInt32 kForward  = 0;
const PRInt32 kBackward = 1;

//#define DEBUG_charset

// Entries in an nsSmallVoidArray must not have the low bit set, so
// can't use "1" for ID_NOT_IN_DOCUMENT.  "2" should be a perfectly
// reasonable value, though -- no real nsIContent* will be equal to 2.
#define ID_NOT_IN_DOCUMENT ((nsIContent *)2)
#define NAME_NOT_VALID ((nsBaseContentList*)1)

// Returns the name atom of aContent, if the content is a named item
// and has a name atom.
static nsIAtom* IsNamedItem(nsIContent* aContent);

static NS_DEFINE_CID(kCParserCID, NS_PARSER_CID);

PRUint32       nsHTMLDocument::gWyciwygSessionCnt = 0;

// this function will return false if the command is not recognized
// inCommandID will be converted as necessary for internal operations
// inParam will be converted as necessary for internal operations
// outParam will be Empty if no parameter is needed or if returning a boolean
// outIsBoolean will determine whether to send param as a boolean or string
// outBooleanParam will not be set unless outIsBoolean
static PRBool ConvertToMidasInternalCommand(const nsAString & inCommandID,
                                            const nsAString & inParam,
                                            nsACString& outCommandID,
                                            nsACString& outParam,
                                            PRBool& isBoolean,
                                            PRBool& boolValue);

static PRBool ConvertToMidasInternalCommand(const nsAString & inCommandID,
                                            nsACString& outCommandID);
static int PR_CALLBACK
MyPrefChangedCallback(const char*aPrefName, void* instance_data)
{
  const nsAdoptingString& detector_name =
    nsContentUtils::GetLocalizedStringPref("intl.charset.detector");

  if (detector_name.Length() > 0) {
    PL_strncpy(g_detector_contractid, NS_CHARSET_DETECTOR_CONTRACTID_BASE,
               DETECTOR_CONTRACTID_MAX);
    PL_strncat(g_detector_contractid,
               NS_ConvertUTF16toUTF8(detector_name).get(),
               DETECTOR_CONTRACTID_MAX);
    gPlugDetector = PR_TRUE;
  } else {
    g_detector_contractid[0]=0;
    gPlugDetector = PR_FALSE;
  }

  return 0;
}

// ==================================================================
// =
// ==================================================================
nsresult
NS_NewHTMLDocument(nsIDocument** aInstancePtrResult)
{
  nsHTMLDocument* doc = new nsHTMLDocument();
  NS_ENSURE_TRUE(doc, NS_ERROR_OUT_OF_MEMORY);

  NS_ADDREF(doc);
  nsresult rv = doc->Init();

  if (NS_FAILED(rv)) {
    NS_RELEASE(doc);
  }

  *aInstancePtrResult = doc;

  return rv;
}

class IdAndNameMapEntry : public PLDHashEntryHdr
{
public:
  IdAndNameMapEntry(nsIAtom* aKey) :
    mKey(aKey), mNameContentList(nsnull), mIdContentList()
  {
  }

  ~IdAndNameMapEntry()
  {
    if (mNameContentList && mNameContentList != NAME_NOT_VALID) {
      NS_RELEASE(mNameContentList);
    }
  }

  nsIContent* GetIdContent() {
    return static_cast<nsIContent*>(mIdContentList.SafeElementAt(0));
  }

  PRBool AddIdContent(nsIContent* aContent);

  PRBool RemoveIdContent(nsIContent* aContent) {
    // XXXbz should this ever Compact() I guess when all the content is gone
    // we'll just get cleaned up in the natural order of things...
    return mIdContentList.RemoveElement(aContent) &&
      mIdContentList.Count() == 0;
  }

  void FlagIDNotInDocument() {
    NS_ASSERTION(mIdContentList.Count() == 0,
                 "Flagging ID not in document when we have content?");
    // Note that if this fails that's OK; this is just an optimization
    mIdContentList.AppendElement(ID_NOT_IN_DOCUMENT);
  }

  nsCOMPtr<nsIAtom> mKey;
  nsBaseContentList *mNameContentList;
  nsRefPtr<nsContentList> mDocAllList;
private:
  nsSmallVoidArray mIdContentList;
};

PRBool
IdAndNameMapEntry::AddIdContent(nsIContent* aContent)
{
  NS_PRECONDITION(aContent, "Must have content");
  NS_PRECONDITION(mIdContentList.IndexOf(nsnull) == -1,
                  "Why is null in our list?");
  NS_PRECONDITION(aContent != ID_NOT_IN_DOCUMENT,
                  "Bogus content pointer");

  if (GetIdContent() == ID_NOT_IN_DOCUMENT) {
    NS_ASSERTION(mIdContentList.Count() == 1, "Bogus count");
    return mIdContentList.ReplaceElementAt(aContent, 0);
  }

  // Common case
  if (mIdContentList.Count() == 0) {
    return mIdContentList.AppendElement(aContent);
  }

  // We seem to have multiple content nodes for the same id, or we're doing our
  // top-down registration when the id table is going live.  Search for the
  // right place to insert the content.
  PRInt32 start = 0;
  PRInt32 end = mIdContentList.Count();
  do {
    NS_ASSERTION(start < end, "Bogus start/end");
    
    PRInt32 cur = (start + end) / 2;
    NS_ASSERTION(cur >= start && cur < end, "What happened here?");

    nsIContent* curContent = static_cast<nsIContent*>(mIdContentList[cur]);
    if (curContent == aContent) {
      // Already in the list, so already in the right spot.  Get out of here.
      return PR_TRUE;
    }

    if (nsContentUtils::PositionIsBefore(aContent, curContent)) {
      end = cur;
    } else {
      start = cur + 1;
    }
  } while (start != end);
  
  return mIdContentList.InsertElementAt(aContent, start);
}


PR_STATIC_CALLBACK(PLDHashNumber)
IdAndNameHashHashKey(PLDHashTable *table, const void *key)
{
  // Our key is an nsIAtom*, so just shift it and use that as the hash.
  return NS_PTR_TO_INT32(key) >> 2;
}

PR_STATIC_CALLBACK(PRBool)
IdAndNameHashMatchEntry(PLDHashTable *table, const PLDHashEntryHdr *entry,
                        const void *key)
{
  const IdAndNameMapEntry *e =
    static_cast<const IdAndNameMapEntry *>(entry);
  const nsIAtom *atom = static_cast<const nsIAtom *>(key);

  return atom == e->mKey;
}

PR_STATIC_CALLBACK(void)
IdAndNameHashClearEntry(PLDHashTable *table, PLDHashEntryHdr *entry)
{
  IdAndNameMapEntry *e = static_cast<IdAndNameMapEntry *>(entry);

  // An entry is being cleared, let the entry do its own cleanup.
  e->~IdAndNameMapEntry();
}

PR_STATIC_CALLBACK(PRBool)
IdAndNameHashInitEntry(PLDHashTable *table, PLDHashEntryHdr *entry,
                       const void *key)
{
  nsIAtom *atom = const_cast<nsIAtom *>
                            (static_cast<const nsIAtom*>(key));

  // Inititlize the entry with placement new
  new (entry) IdAndNameMapEntry(atom);
  return PR_TRUE;
}

  // NOTE! nsDocument::operator new() zeroes out all members, so don't
  // bother initializing members to 0.

nsHTMLDocument::nsHTMLDocument()
  : nsDocument("text/html"),
    mDefaultNamespaceID(kNameSpaceID_None)
{

  // NOTE! nsDocument::operator new() zeroes out all members, so don't
  // bother initializing members to 0.

  mDefaultElementType = kNameSpaceID_XHTML;
  mCompatMode = eCompatibility_NavQuirks;
}

nsHTMLDocument::~nsHTMLDocument()
{
  if (mIdAndNameHashTable.ops) {
    PL_DHashTableFinish(&mIdAndNameHashTable);
  }
}

NS_IMPL_CYCLE_COLLECTION_CLASS(nsHTMLDocument)

PR_STATIC_CALLBACK(PLDHashOperator)
IdAndNameMapEntryTraverse(PLDHashTable *table, PLDHashEntryHdr *hdr,
                          PRUint32 number, void *arg)
{
  nsCycleCollectionTraversalCallback *cb =
    static_cast<nsCycleCollectionTraversalCallback*>(arg);
  IdAndNameMapEntry *entry = static_cast<IdAndNameMapEntry*>(hdr);

  if (entry->mNameContentList != NAME_NOT_VALID)
    cb->NoteXPCOMChild(entry->mNameContentList);

  cb->NoteXPCOMChild(static_cast<nsIDOMNodeList*>(entry->mDocAllList));

  return PL_DHASH_NEXT;
}

NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN_INHERITED(nsHTMLDocument, nsDocument)
  if (tmp->mIdAndNameHashTable.ops) {
    PL_DHashTableEnumerate(&tmp->mIdAndNameHashTable,
                           IdAndNameMapEntryTraverse,
                           &cb);
  }
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMARRAY(mImageMaps)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mImages)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mApplets)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mEmbeds)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mLinks)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mAnchors)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR(mFragmentParser)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR_AMBIGUOUS(mForms, nsIDOMNodeList)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE_NSCOMPTR_AMBIGUOUS(mFormControls,
                                                       nsIDOMNodeList)
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_IMPL_ADDREF_INHERITED(nsHTMLDocument, nsDocument)
NS_IMPL_RELEASE_INHERITED(nsHTMLDocument, nsDocument)


// QueryInterface implementation for nsHTMLDocument
NS_INTERFACE_TABLE_HEAD_CYCLE_COLLECTION_INHERITED(nsHTMLDocument)
  NS_INTERFACE_TABLE_INHERITED3(nsHTMLDocument,
                                nsIHTMLDocument,
                                nsIDOMHTMLDocument,
                                nsIDOMNSHTMLDocument)
  NS_INTERFACE_TABLE_TO_MAP_SEGUE
  NS_INTERFACE_MAP_ENTRY_CONTENT_CLASSINFO(HTMLDocument)
NS_INTERFACE_MAP_END_INHERITING(nsDocument)


nsresult
nsHTMLDocument::Init()
{
  nsresult rv = nsDocument::Init();
  NS_ENSURE_SUCCESS(rv, rv);

  // Now reset the case-sensitivity of the CSSLoader, since we default
  // to being HTML, not XHTML.  Also, reset the compatibility mode to
  // match our compat mode.
  CSSLoader()->SetCaseSensitive(IsXHTML());
  CSSLoader()->SetCompatibilityMode(mCompatMode);

  static PLDHashTableOps hash_table_ops =
  {
    PL_DHashAllocTable,
    PL_DHashFreeTable,
    IdAndNameHashHashKey,
    IdAndNameHashMatchEntry,
    PL_DHashMoveEntryStub,
    IdAndNameHashClearEntry,
    PL_DHashFinalizeStub,
    IdAndNameHashInitEntry
  };

  PRBool ok = PL_DHashTableInit(&mIdAndNameHashTable, &hash_table_ops, nsnull,
                                sizeof(IdAndNameMapEntry), 16);
  if (!ok) {
    mIdAndNameHashTable.ops = nsnull;

    return NS_ERROR_OUT_OF_MEMORY;
  }

  PrePopulateHashTables();

  return NS_OK;
}


void
nsHTMLDocument::Reset(nsIChannel* aChannel, nsILoadGroup* aLoadGroup)
{
  nsDocument::Reset(aChannel, aLoadGroup);

  if (aChannel) {
    aChannel->GetLoadFlags(&mLoadFlags);
  }
}

void
nsHTMLDocument::ResetToURI(nsIURI *aURI, nsILoadGroup *aLoadGroup,
                           nsIPrincipal* aPrincipal)
{
  mLoadFlags = nsIRequest::LOAD_NORMAL;

  nsDocument::ResetToURI(aURI, aLoadGroup, aPrincipal);

  InvalidateHashTables();
  PrePopulateHashTables();

  mImages = nsnull;
  mApplets = nsnull;
  mEmbeds = nsnull;
  mLinks = nsnull;
  mAnchors = nsnull;

  mImageMaps.Clear();
  mForms = nsnull;

  NS_ASSERTION(!mWyciwygChannel,
               "nsHTMLDocument::Reset() - Wyciwyg Channel  still exists!");

  mWyciwygChannel = nsnull;

  // Make the content type default to "text/html", we are a HTML
  // document, after all. Once we start getting data, this may be
  // changed.
  mContentType = "text/html";
}

nsStyleSet::sheetType
nsHTMLDocument::GetAttrSheetType()
{
  if (IsXHTML()) {
    return nsDocument::GetAttrSheetType();
  }
  
  return nsStyleSet::eHTMLPresHintSheet;
}

nsresult
nsHTMLDocument::CreateShell(nsPresContext* aContext,
                            nsIViewManager* aViewManager,
                            nsStyleSet* aStyleSet,
                            nsIPresShell** aInstancePtrResult)
{
  return doCreateShell(aContext, aViewManager, aStyleSet, mCompatMode,
                       aInstancePtrResult);
}

// The following Try*Charset will return PR_FALSE only if the charset source
// should be considered (ie. aCharsetSource < thisCharsetSource) but we failed
// to get the charset from this source.

PRBool
nsHTMLDocument::TryHintCharset(nsIMarkupDocumentViewer* aMarkupDV,
                               PRInt32& aCharsetSource, nsACString& aCharset)
{
  if (aMarkupDV) {
    PRInt32 requestCharsetSource;
    nsresult rv = aMarkupDV->GetHintCharacterSetSource(&requestCharsetSource);

    if(NS_SUCCEEDED(rv) && kCharsetUninitialized != requestCharsetSource) {
      nsCAutoString requestCharset;
      rv = aMarkupDV->GetHintCharacterSet(requestCharset);
      aMarkupDV->SetHintCharacterSetSource((PRInt32)(kCharsetUninitialized));

      if(requestCharsetSource <= aCharsetSource)
        return PR_TRUE;

      if(NS_SUCCEEDED(rv)) {
        aCharsetSource = requestCharsetSource;
        aCharset = requestCharset;

        return PR_TRUE;
      }
    }
  }
  return PR_FALSE;
}


PRBool
nsHTMLDocument::TryUserForcedCharset(nsIMarkupDocumentViewer* aMarkupDV,
                                     nsIDocumentCharsetInfo*  aDocInfo,
                                     PRInt32& aCharsetSource,
                                     nsACString& aCharset)
{
  nsresult rv = NS_OK;

  if(kCharsetFromUserForced <= aCharsetSource)
    return PR_TRUE;

  nsCAutoString forceCharsetFromDocShell;
  if (aMarkupDV) {
    rv = aMarkupDV->GetForceCharacterSet(forceCharsetFromDocShell);
  }

  if(NS_SUCCEEDED(rv) && !forceCharsetFromDocShell.IsEmpty()) {
    aCharset = forceCharsetFromDocShell;
    //TODO: we should define appropriate constant for force charset
    aCharsetSource = kCharsetFromUserForced;
  } else if (aDocInfo) {
    nsCOMPtr<nsIAtom> csAtom;
    aDocInfo->GetForcedCharset(getter_AddRefs(csAtom));
    if (csAtom) {
      csAtom->ToUTF8String(aCharset);
      aCharsetSource = kCharsetFromUserForced;
      aDocInfo->SetForcedCharset(nsnull);
      return PR_TRUE;
    }
  }

  return PR_FALSE;
}

PRBool
nsHTMLDocument::TryCacheCharset(nsICacheEntryDescriptor* aCacheDescriptor,
                                PRInt32& aCharsetSource,
                                nsACString& aCharset)
{
  nsresult rv;

  if (kCharsetFromCache <= aCharsetSource) {
    return PR_TRUE;
  }

  nsXPIDLCString cachedCharset;
  rv = aCacheDescriptor->GetMetaDataElement("charset",
                                           getter_Copies(cachedCharset));
  if (NS_SUCCEEDED(rv) && !cachedCharset.IsEmpty())
  {
    aCharset = cachedCharset;
    aCharsetSource = kCharsetFromCache;

    return PR_TRUE;
  }

  return PR_FALSE;
}

PRBool
nsHTMLDocument::TryBookmarkCharset(nsIDocShell* aDocShell,
                                   nsIChannel* aChannel,
                                   PRInt32& aCharsetSource,
                                   nsACString& aCharset)
{
  if (kCharsetFromBookmarks <= aCharsetSource) {
    return PR_TRUE;
  }

  if (!aChannel) {
    return PR_FALSE;
  }

  nsCOMPtr<nsICharsetResolver> bookmarksResolver =
    do_GetService("@mozilla.org/embeddor.implemented/bookmark-charset-resolver;1");

  if (!bookmarksResolver) {
    return PR_FALSE;
  }

  PRBool wantCharset;         // ignored for now
  nsCAutoString charset;
  nsCOMPtr<nsIWebNavigation> webNav(do_QueryInterface(aDocShell));
  nsCOMPtr<nsISupports> closure;
  nsresult rv = bookmarksResolver->RequestCharset(webNav,
                                                  aChannel,
                                                  &wantCharset,
                                                  getter_AddRefs(closure),
                                                  charset);
  // FIXME: Bug 337790
  NS_ASSERTION(!wantCharset, "resolved charset notification not implemented!");

  if (NS_SUCCEEDED(rv) && !charset.IsEmpty()) {
    aCharset = charset;
    aCharsetSource = kCharsetFromBookmarks;
    return PR_TRUE;
  }

  return PR_FALSE;
}

static PRBool
CheckSameOrigin(nsINode* aNode1, nsINode* aNode2)
{
  NS_PRECONDITION(aNode1, "Null node?");
  NS_PRECONDITION(aNode2, "Null node?");

  PRBool equal;
  return
    NS_SUCCEEDED(aNode1->NodePrincipal()->
                   Equals(aNode2->NodePrincipal(), &equal)) &&
    equal;
}

PRBool
nsHTMLDocument::TryParentCharset(nsIDocumentCharsetInfo*  aDocInfo,
                                 nsIDocument* aParentDocument,
                                 PRInt32& aCharsetSource,
                                 nsACString& aCharset)
{
  if (aDocInfo) {
    PRInt32 source;
    nsCOMPtr<nsIAtom> csAtom;
    PRInt32 parentSource;
    aDocInfo->GetParentCharsetSource(&parentSource);
    if (kCharsetFromParentForced <= parentSource)
      source = kCharsetFromParentForced;
    else if (kCharsetFromHintPrevDoc == parentSource) {
      // Make sure that's OK
      if (!aParentDocument || !CheckSameOrigin(this, aParentDocument)) {
        return PR_FALSE;
      }
      
      // if parent is posted doc, set this prevent autodections
      // I'm not sure this makes much sense... but whatever.
      source = kCharsetFromHintPrevDoc;
    }
    else if (kCharsetFromCache <= parentSource) {
      // Make sure that's OK
      if (!aParentDocument || !CheckSameOrigin(this, aParentDocument)) {
        return PR_FALSE;
      }

      source = kCharsetFromParentFrame;
    }
    else
      return PR_FALSE;

    if (source < aCharsetSource)
      return PR_TRUE;

    aDocInfo->GetParentCharset(getter_AddRefs(csAtom));
    if (csAtom) {
      csAtom->ToUTF8String(aCharset);
      aCharsetSource = source;
      return PR_TRUE;
    }
  }
  return PR_FALSE;
}

PRBool
nsHTMLDocument::UseWeakDocTypeDefault(PRInt32& aCharsetSource,
                                      nsACString& aCharset)
{
  if (kCharsetFromWeakDocTypeDefault <= aCharsetSource)
    return PR_TRUE;
  // fallback value in case docshell return error
  aCharset.AssignLiteral("ISO-8859-1");

  const nsAdoptingString& defCharset =
    nsContentUtils::GetLocalizedStringPref("intl.charset.default");

  if (!defCharset.IsEmpty()) {
    LossyCopyUTF16toASCII(defCharset, aCharset);
    aCharsetSource = kCharsetFromWeakDocTypeDefault;
  }
  return PR_TRUE;
}

PRBool
nsHTMLDocument::TryDefaultCharset( nsIMarkupDocumentViewer* aMarkupDV,
                                   PRInt32& aCharsetSource,
                                   nsACString& aCharset)
{
  if(kCharsetFromUserDefault <= aCharsetSource)
    return PR_TRUE;

  nsCAutoString defaultCharsetFromDocShell;
  if (aMarkupDV) {
    nsresult rv =
      aMarkupDV->GetDefaultCharacterSet(defaultCharsetFromDocShell);
    if(NS_SUCCEEDED(rv)) {
      aCharset = defaultCharsetFromDocShell;

      aCharsetSource = kCharsetFromUserDefault;
      return PR_TRUE;
    }
  }
  return PR_FALSE;
}

void
nsHTMLDocument::StartAutodetection(nsIDocShell *aDocShell, nsACString& aCharset,
                                   const char* aCommand)
{
  nsCOMPtr <nsIParserFilter> cdetflt;

  nsresult rv_detect;
  if(!gInitDetector) {
    const nsAdoptingString& detector_name =
      nsContentUtils::GetLocalizedStringPref("intl.charset.detector");

    if(!detector_name.IsEmpty()) {
      PL_strncpy(g_detector_contractid, NS_CHARSET_DETECTOR_CONTRACTID_BASE,
                 DETECTOR_CONTRACTID_MAX);
      PL_strncat(g_detector_contractid,
                 NS_ConvertUTF16toUTF8(detector_name).get(),
                 DETECTOR_CONTRACTID_MAX);
      gPlugDetector = PR_TRUE;
    }

    nsContentUtils::RegisterPrefCallback("intl.charset.detector",
                                         MyPrefChangedCallback,
                                         nsnull);

    gInitDetector = PR_TRUE;
  }

  if (gPlugDetector) {
    nsCOMPtr <nsICharsetDetector> cdet =
      do_CreateInstance(g_detector_contractid, &rv_detect);
    if (NS_SUCCEEDED(rv_detect)) {
      cdetflt = do_CreateInstance(NS_CHARSET_DETECTION_ADAPTOR_CONTRACTID,
                                  &rv_detect);

      nsCOMPtr<nsICharsetDetectionAdaptor> adp = do_QueryInterface(cdetflt);
      if (adp) {
        nsCOMPtr<nsIWebShellServices> wss = do_QueryInterface(aDocShell);
        if (wss) {
          rv_detect = adp->Init(wss, cdet, this, mParser,
                                PromiseFlatCString(aCharset).get(), aCommand);

          if (mParser)
            mParser->SetParserFilter(cdetflt);
        }
      }
    }
    else {
      // IF we cannot create the detector, don't bother to
      // create one next time.
      gPlugDetector = PR_FALSE;
    }
  }
}

void
nsHTMLDocument::SetDocumentCharacterSet(const nsACString& aCharSetID)
{
  nsDocument::SetDocumentCharacterSet(aCharSetID);
  // Make sure to stash this charset on our channel as needed if it's a wyciwyg
  // channel.
  nsCOMPtr<nsIWyciwygChannel> wyciwygChannel = do_QueryInterface(mChannel);
  if (wyciwygChannel) {
    wyciwygChannel->SetCharsetAndSource(GetDocumentCharacterSetSource(),
                                        aCharSetID);
  }
}

nsresult
nsHTMLDocument::StartDocumentLoad(const char* aCommand,
                                  nsIChannel* aChannel,
                                  nsILoadGroup* aLoadGroup,
                                  nsISupports* aContainer,
                                  nsIStreamListener **aDocListener,
                                  PRBool aReset,
                                  nsIContentSink* aSink)
{
  nsCAutoString contentType;
  aChannel->GetContentType(contentType);

  if (contentType.Equals("application/xhtml+xml") &&
      (!aCommand || nsCRT::strcmp(aCommand, "view-source") != 0)) {
    // We're parsing XHTML as XML, remember that.

    mDefaultNamespaceID = kNameSpaceID_XHTML;
    mCompatMode = eCompatibility_FullStandards;
  }
#ifdef DEBUG
  else {
    NS_ASSERTION(mDefaultNamespaceID == kNameSpaceID_None,
                 "Hey, someone forgot to reset mDefaultNamespaceID!!!");
  }
#endif

  CSSLoader()->SetCaseSensitive(IsXHTML());
  CSSLoader()->SetCompatibilityMode(mCompatMode);
  
  PRBool needsParser = PR_TRUE;
  if (aCommand)
  {
    if (!nsCRT::strcmp(aCommand, "view delayedContentLoad")) {
      needsParser = PR_FALSE;
    }
  }

  nsCOMPtr<nsICacheEntryDescriptor> cacheDescriptor;
  nsresult rv = nsDocument::StartDocumentLoad(aCommand,
                                              aChannel, aLoadGroup,
                                              aContainer,
                                              aDocListener, aReset);
  if (NS_FAILED(rv)) {
    return rv;
  }

  // Store the security info for future use with wyciwyg channels.
  aChannel->GetSecurityInfo(getter_AddRefs(mSecurityInfo));

  nsCOMPtr<nsIURI> uri;
  rv = aChannel->GetURI(getter_AddRefs(uri));
  if (NS_FAILED(rv)) {
    return rv;
  }

  nsCOMPtr<nsICachingChannel> cachingChan = do_QueryInterface(aChannel);
  if (cachingChan) {
    nsCOMPtr<nsISupports> cacheToken;
    cachingChan->GetCacheToken(getter_AddRefs(cacheToken));
    if (cacheToken)
      cacheDescriptor = do_QueryInterface(cacheToken);
  }

  if (needsParser) {
    mParser = do_CreateInstance(kCParserCID, &rv);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  nsCOMPtr<nsIDocShell> docShell(do_QueryInterface(aContainer));

  nsCOMPtr<nsIDocumentCharsetInfo> dcInfo;
  docShell->GetDocumentCharsetInfo(getter_AddRefs(dcInfo));
  PRInt32 textType = GET_BIDI_OPTION_TEXTTYPE(GetBidiOptions());

  // Look for the parent document.  Note that at this point we don't have our
  // content viewer set up yet, and therefore do not have a useful
  // mParentDocument.

  // in this block of code, if we get an error result, we return it
  // but if we get a null pointer, that's perfectly legal for parent
  // and parentContentViewer
  nsCOMPtr<nsIDocShellTreeItem> docShellAsItem(do_QueryInterface(docShell));
  NS_ENSURE_TRUE(docShellAsItem, NS_ERROR_FAILURE);

  nsCOMPtr<nsIDocShellTreeItem> parentAsItem;
  docShellAsItem->GetSameTypeParent(getter_AddRefs(parentAsItem));

  nsCOMPtr<nsIDocShell> parent(do_QueryInterface(parentAsItem));
  nsCOMPtr<nsIDocument> parentDocument;
  nsCOMPtr<nsIContentViewer> parentContentViewer;
  if (parent) {
    rv = parent->GetContentViewer(getter_AddRefs(parentContentViewer));
    NS_ENSURE_SUCCESS(rv, rv);
    nsCOMPtr<nsIDocumentViewer> docViewer =
      do_QueryInterface(parentContentViewer);
    if (docViewer) {
      docViewer->GetDocument(getter_AddRefs(parentDocument));
    }
  }

  //
  // The following logic is mirrored in nsWebShell::Embed!
  //
  nsCOMPtr<nsIMarkupDocumentViewer> muCV;
  PRBool muCVIsParent = PR_FALSE;
  nsCOMPtr<nsIContentViewer> cv;
  docShell->GetContentViewer(getter_AddRefs(cv));
  if (cv) {
     muCV = do_QueryInterface(cv);
  } else {
    muCV = do_QueryInterface(parentContentViewer);
    if (muCV) {
      muCVIsParent = PR_TRUE;
    }
  }

  nsCAutoString scheme;
  uri->GetScheme(scheme);

  nsCAutoString urlSpec;
  uri->GetSpec(urlSpec);
#ifdef DEBUG_charset
  printf("Determining charset for %s\n", urlSpec.get());
#endif

  // These are the charset source and charset for our document
  PRInt32 charsetSource;
  nsCAutoString charset;

  // These are the charset source and charset for the parser.  This can differ
  // from that for the document if the channel is a wyciwyg channel.
  PRInt32 parserCharsetSource;
  nsCAutoString parserCharset;

  nsCOMPtr<nsIWyciwygChannel> wyciwygChannel;
  
  if (IsXHTML()) {
    charsetSource = kCharsetFromDocTypeDefault;
    charset.AssignLiteral("UTF-8");
    TryChannelCharset(aChannel, charsetSource, charset);
    parserCharsetSource = charsetSource;
    parserCharset = charset;
  } else {
    charsetSource = kCharsetUninitialized;
    wyciwygChannel = do_QueryInterface(aChannel);

    // The following charset resolving calls has implied knowledge
    // about charset source priority order. Each try will return true
    // if the source is higher or equal to the source as its name
    // describes. Some try call might change charset source to
    // multiple values, like TryHintCharset and TryParentCharset. It
    // should be always safe to try more sources.
    if (!TryUserForcedCharset(muCV, dcInfo, charsetSource, charset)) {
      TryHintCharset(muCV, charsetSource, charset);
      TryParentCharset(dcInfo, parentDocument, charsetSource, charset);

      // Don't actually get the charset from the channel if this is a
      // wyciwyg channel; it'll always be UTF-16
      if (!wyciwygChannel &&
          TryChannelCharset(aChannel, charsetSource, charset)) {
        // Use the channel's charset (e.g., charset from HTTP
        // "Content-Type" header).
      }
      else if (!scheme.EqualsLiteral("about") &&          // don't try to access bookmarks for about:blank
               TryBookmarkCharset(docShell, aChannel, charsetSource, charset)) {
        // Use the bookmark's charset.
      }
      else if (cacheDescriptor && !urlSpec.IsEmpty() &&
               TryCacheCharset(cacheDescriptor, charsetSource, charset)) {
        // Use the cache's charset.
      }
      else if (TryDefaultCharset(muCV, charsetSource, charset)) {
        // Use the default charset.
        // previous document charset might be inherited as default charset.
      }
      else {
        // Use the weak doc type default charset
        UseWeakDocTypeDefault(charsetSource, charset);
      }
    }

    PRBool isPostPage = PR_FALSE;
    // check if current doc is from POST command
    nsCOMPtr<nsIHttpChannel> httpChannel(do_QueryInterface(aChannel));
    if (httpChannel) {
      nsCAutoString methodStr;
      rv = httpChannel->GetRequestMethod(methodStr);
      isPostPage = (NS_SUCCEEDED(rv) &&
                    methodStr.EqualsLiteral("POST"));
    }

    if (isPostPage && muCV && kCharsetFromHintPrevDoc > charsetSource) {
      nsCAutoString requestCharset;
      muCV->GetPrevDocCharacterSet(requestCharset);
      if (!requestCharset.IsEmpty()) {
        charsetSource = kCharsetFromHintPrevDoc;
        charset = requestCharset;
      }
    }

    if (wyciwygChannel) {
      // We know for sure that the parser needs to be using UTF16.
      parserCharset = "UTF-16";
      parserCharsetSource = charsetSource < kCharsetFromChannel ?
        kCharsetFromChannel : charsetSource;
        
      nsCAutoString cachedCharset;
      PRInt32 cachedSource;
      rv = wyciwygChannel->GetCharsetAndSource(&cachedSource, cachedCharset);
      if (NS_SUCCEEDED(rv)) {
        if (cachedSource > charsetSource) {
          charsetSource = cachedSource;
          charset = cachedCharset;
        }
      } else {
        // Don't propagate this error.
        rv = NS_OK;
      }
      
    } else {
      parserCharset = charset;
      parserCharsetSource = charsetSource;
    }

    if(kCharsetFromAutoDetection > charsetSource && !isPostPage) {
      StartAutodetection(docShell, charset, aCommand);
    }

    // ahmed
    // Check if 864 but in Implicit mode !
    // XXXbz why is this happening after StartAutodetection ?
    if ((textType == IBMBIDI_TEXTTYPE_LOGICAL) &&
        (charset.LowerCaseEqualsLiteral("ibm864"))) {
      charset.AssignLiteral("IBM864i");
    }
  }

  SetDocumentCharacterSetSource(charsetSource);
  SetDocumentCharacterSet(charset);

  // set doc charset to muCV for next document.
  // Don't propagate this back up to the parent document if we have one.
  if (muCV && !muCVIsParent)
    muCV->SetPrevDocCharacterSet(charset);

  if(cacheDescriptor) {
    NS_ASSERTION(charset == parserCharset,
                 "How did those end up different here?  wyciwyg channels are "
                 "not nsICachingChannel");
    rv = cacheDescriptor->SetMetaDataElement("charset",
                                             charset.get());
    NS_ASSERTION(NS_SUCCEEDED(rv),"cannot SetMetaDataElement");
  }

  // Set the parser as the stream listener for the document loader...
  if (mParser) {
    rv = CallQueryInterface(mParser, aDocListener);
    if (NS_FAILED(rv)) {
      return rv;
    }

#ifdef DEBUG_charset
    printf(" charset = %s source %d\n",
          charset.get(), charsetSource);
#endif
    mParser->SetDocumentCharset(parserCharset, parserCharsetSource);
    mParser->SetCommand(aCommand);

    // create the content sink
    nsCOMPtr<nsIContentSink> sink;

    if (aSink)
      sink = aSink;
    else {
      if (IsXHTML()) {
        nsCOMPtr<nsIXMLContentSink> xmlsink;
        rv = NS_NewXMLContentSink(getter_AddRefs(xmlsink), this, uri,
                                  docShell, aChannel);

        sink = xmlsink;
      } else {
        nsCOMPtr<nsIHTMLContentSink> htmlsink;

        rv = NS_NewHTMLContentSink(getter_AddRefs(htmlsink), this, uri,
                                   docShell, aChannel);

        sink = htmlsink;
      }
      NS_ENSURE_SUCCESS(rv, rv);

      NS_ASSERTION(sink,
                   "null sink with successful result from factory method");
    }

    mParser->SetContentSink(sink);
    // parser the content of the URI
    mParser->Parse(uri, nsnull, (void *)this);
  }

  return rv;
}

void
nsHTMLDocument::StopDocumentLoad()
{
  // If we're writing (i.e., there's been a document.open call), then
  // nsDocument::StopDocumentLoad will do the wrong thing and simply terminate
  // our parser.
  if (mWriteState != eNotWriting) {
    Close();
  } else {
    nsDocument::StopDocumentLoad();
  }
}

// static
void
nsHTMLDocument::DocumentWriteTerminationFunc(nsISupports *aRef)
{
  nsCOMPtr<nsIArray> arr = do_QueryInterface(aRef);
  NS_ASSERTION(arr, "Must have array!");

  nsCOMPtr<nsIDocument> doc = do_QueryElementAt(arr, 0);
  NS_ASSERTION(doc, "Must have document!");
  
  nsCOMPtr<nsIParser> parser = do_QueryElementAt(arr, 1);
  NS_ASSERTION(parser, "Must have parser!");

  nsHTMLDocument *htmldoc = static_cast<nsHTMLDocument*>(doc.get());

  // Check whether htmldoc still has the same parser.  If not, it's
  // not for us to mess with it.
  if (htmldoc->mParser != parser) {
    return;
  }

  // If the document is in the middle of a document.write() call, this
  // most likely means that script on a page document.write()'d out a
  // script tag that did location="..." and we're right now finishing
  // up executing the script that was written with
  // document.write(). Since there's still script on the stack (the
  // script that called document.write()) we don't want to release the
  // parser now, that would cause the next document.write() call to
  // cancel the load that was initiated by the location="..." in the
  // script that was written out by document.write().

  if (!htmldoc->mWriteLevel && htmldoc->mWriteState != eDocumentOpened) {
    // Release the document's parser so that the call to EndLoad()
    // doesn't just return early and set the termination function again.

    htmldoc->mParser = nsnull;
  }

  htmldoc->EndLoad();
}

void
nsHTMLDocument::EndLoad()
{
  if (mParser && mWriteState != eDocumentClosed) {
    nsCOMPtr<nsIJSContextStack> stack =
      do_GetService("@mozilla.org/js/xpc/ContextStack;1");

    if (stack) {
      JSContext *cx = nsnull;
      stack->Peek(&cx);

      if (cx) {
        nsIScriptContext *scx = nsJSUtils::GetDynamicScriptContext(cx);

        if (scx) {
          // The load of the document was terminated while we're
          // called from within JS and we have a parser (i.e. we're in
          // the middle of doing document.write()). In stead of
          // releasing the parser and ending the document load
          // directly, we'll make that happen once the script is done
          // executing. This way subsequent document.write() calls
          // won't end up creating a new parser and interrupting other
          // loads that were started while the script was
          // running. I.e. this makes the following case work as
          // expected:
          //
          //   document.write("foo");
          //   location.href = "http://www.mozilla.org";
          //   document.write("bar");

          nsresult rv;

          nsCOMPtr<nsIMutableArray> arr =
            do_CreateInstance(NS_ARRAY_CONTRACTID, &rv);
          if (NS_SUCCEEDED(rv)) {
            rv = arr->AppendElement(static_cast<nsIDocument*>(this),
                                    PR_FALSE);
            if (NS_SUCCEEDED(rv)) {
              rv = arr->AppendElement(mParser, PR_FALSE);
              if (NS_SUCCEEDED(rv)) {
                rv = scx->SetTerminationFunction(DocumentWriteTerminationFunc,
                                                 arr);
                // If we fail to set the termination function, just go ahead
                // and EndLoad now.  The slight bugginess involved is better
                // than leaking.
                if (NS_SUCCEEDED(rv)) {
                  return;
                }
              }
            }
          }
        }
      }
    }
  }

  // Reset this now, since we're really done "loading" this document.written
  // document.
  NS_ASSERTION(mWriteState == eNotWriting || mWriteState == ePendingClose ||
               mWriteState == eDocumentClosed, "EndLoad called early");
  mWriteState = eNotWriting;

  PRBool turnOnEditing =
    mParser && (HasFlag(NODE_IS_EDITABLE) || mContentEditableCount > 0);
  // Note: nsDocument::EndLoad nulls out mParser.
  nsDocument::EndLoad();
  if (turnOnEditing) {
    EditingStateChanged();
  }
}

NS_IMETHODIMP
nsHTMLDocument::SetTitle(const nsAString& aTitle)
{
  return nsDocument::SetTitle(aTitle);
}

nsresult
nsHTMLDocument::AddImageMap(nsIDOMHTMLMapElement* aMap)
{
  // XXX We should order the maps based on their order in the document.
  // XXX Otherwise scripts that add/remove maps with duplicate names
  // XXX will cause problems
  NS_PRECONDITION(nsnull != aMap, "null ptr");
  if (nsnull == aMap) {
    return NS_ERROR_NULL_POINTER;
  }
  if (mImageMaps.AppendObject(aMap)) {
    return NS_OK;
  }
  return NS_ERROR_OUT_OF_MEMORY;
}

void
nsHTMLDocument::RemoveImageMap(nsIDOMHTMLMapElement* aMap)
{
  NS_PRECONDITION(nsnull != aMap, "null ptr");
  mImageMaps.RemoveObject(aMap);
}

nsIDOMHTMLMapElement *
nsHTMLDocument::GetImageMap(const nsAString& aMapName)
{
  nsAutoString name;
  PRUint32 i, n = mImageMaps.Count();
  nsIDOMHTMLMapElement *firstMatch = nsnull;

  for (i = 0; i < n; ++i) {
    nsIDOMHTMLMapElement *map = mImageMaps[i];
    NS_ASSERTION(map, "Null map in map list!");

    PRBool match;
    nsresult rv;

    if (IsXHTML()) {
      rv = map->GetId(name);

      match = name.Equals(aMapName);
    } else {
      rv = map->GetName(name);

      match = name.Equals(aMapName, nsCaseInsensitiveStringComparator());
    }

    if (match && NS_SUCCEEDED(rv)) {
      // Quirk: if the first matching map is empty, remember it, but keep
      // searching for a non-empty one, only use it if none was found (bug 264624).
      if (mCompatMode == eCompatibility_NavQuirks) {
        nsCOMPtr<nsIDOMHTMLCollection> mapAreas;
        rv = map->GetAreas(getter_AddRefs(mapAreas));
        if (NS_SUCCEEDED(rv) && mapAreas) {
          PRUint32 length = 0;
          mapAreas->GetLength(&length);
          if (length == 0) {
            if (!firstMatch) {
              firstMatch = map;
            }
            continue;
          }
        }
      }
      return map;
    }
  }

  return firstMatch;
}

void
nsHTMLDocument::SetCompatibilityMode(nsCompatibility aMode)
{
  NS_ASSERTION(!IsXHTML() || aMode == eCompatibility_FullStandards,
               "Bad compat mode for XHTML document!");

  mCompatMode = aMode;
  CSSLoader()->SetCompatibilityMode(mCompatMode);
  nsCOMPtr<nsIPresShell> shell = GetPrimaryShell();
  if (shell) {
    nsPresContext *pc = shell->GetPresContext();
    if (pc) {
      pc->CompatibilityModeChanged();
    }
  }
}

void
nsHTMLDocument::ContentAppended(nsIDocument* aDocument,
                                nsIContent* aContainer,
                                PRInt32 aNewIndexInContainer)
{
  NS_ASSERTION(aDocument == this, "unexpected doc");

  PRUint32 count = aContainer->GetChildCount();
  for (PRUint32 i = aNewIndexInContainer; i < count; ++i) {
    RegisterNamedItems(aContainer->GetChildAt(i));
  }
}

void
nsHTMLDocument::ContentInserted(nsIDocument* aDocument,
                                nsIContent* aContainer,
                                nsIContent* aContent,
                                PRInt32 aIndexInContainer)
{
  NS_ASSERTION(aDocument == this, "unexpected doc");

  NS_ABORT_IF_FALSE(aContent, "Null content!");

  RegisterNamedItems(aContent);
}

void
nsHTMLDocument::ContentRemoved(nsIDocument* aDocument,
                               nsIContent* aContainer,
                               nsIContent* aChild,
                               PRInt32 aIndexInContainer)
{
  NS_ASSERTION(aDocument == this, "unexpected doc");

  NS_ABORT_IF_FALSE(aChild, "Null content!");

  UnregisterNamedItems(aChild);
}

void
nsHTMLDocument::AttributeWillChange(nsIContent* aContent, PRInt32 aNameSpaceID,
                                    nsIAtom* aAttribute)
{
  NS_ABORT_IF_FALSE(aContent, "Null content!");
  NS_PRECONDITION(aAttribute, "Must have an attribute that's changing!");

  if (!IsXHTML() && aAttribute == nsGkAtoms::name &&
      aNameSpaceID == kNameSpaceID_None) {
    nsIAtom* name = IsNamedItem(aContent);
    if (name) {
      nsresult rv = RemoveFromNameTable(name, aContent);

      if (NS_FAILED(rv)) {
        return;
      }
    }
  } else if (aAttribute == aContent->GetIDAttributeName() &&
             aNameSpaceID == kNameSpaceID_None) {
    nsresult rv = RemoveFromIdTable(aContent);

    if (NS_FAILED(rv)) {
      return;
    }
  }

  nsDocument::AttributeWillChange(aContent, aNameSpaceID, aAttribute);
}

void
nsHTMLDocument::AttributeChanged(nsIDocument* aDocument,
                                 nsIContent* aContent, PRInt32 aNameSpaceID,
                                 nsIAtom* aAttribute, PRInt32 aModType,
                                 PRUint32 aStateMask)
{
  NS_ASSERTION(aDocument == this, "unexpected doc");

  NS_ABORT_IF_FALSE(aContent, "Null content!");
  NS_PRECONDITION(aAttribute, "Must have an attribute that's changing!");

  if (!IsXHTML() && aAttribute == nsGkAtoms::name &&
      aNameSpaceID == kNameSpaceID_None) {

    nsIAtom* name = IsNamedItem(aContent);
    if (name) {
      UpdateNameTableEntry(name, aContent);
    }
  } else if (aAttribute == aContent->GetIDAttributeName() &&
             aNameSpaceID == kNameSpaceID_None) {
    nsIAtom* id = aContent->GetID();
    if (id) {
      UpdateIdTableEntry(id, aContent);
    }
  }
}

PRBool
nsHTMLDocument::IsCaseSensitive()
{
  return IsXHTML();
}

//
// nsIDOMDocument interface implementation
//
NS_IMETHODIMP
nsHTMLDocument::CreateElement(const nsAString& aTagName,
                              nsIDOMElement** aReturn)
{
  *aReturn = nsnull;
  nsresult rv;

  nsAutoString tagName(aTagName);

  // if we are in quirks, allow surrounding '<' '>' for IE compat
  if (mCompatMode == eCompatibility_NavQuirks &&
      tagName.Length() > 2 &&
      tagName.First() == '<' &&
      tagName.Last() == '>') {
    tagName = Substring(tagName, 1, tagName.Length() - 2); 
  }

  rv = nsContentUtils::CheckQName(tagName, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);

//  if (!IsXHTML()) {
//    ToLowerCase(tagName);
//  }

  nsCOMPtr<nsIAtom> name = do_GetAtom(tagName);

  nsCOMPtr<nsIContent> content;
  rv = CreateElem(name, nsnull, GetDefaultNamespaceID(), PR_TRUE,
                  getter_AddRefs(content));
  NS_ENSURE_SUCCESS(rv, rv);

  return CallQueryInterface(content, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::CreateElementNS(const nsAString& aNamespaceURI,
                                const nsAString& aQualifiedName,
                                nsIDOMElement** aReturn)
{
  return nsDocument::CreateElementNS(aNamespaceURI, aQualifiedName, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::CreateProcessingInstruction(const nsAString& aTarget,
                                            const nsAString& aData,
                                            nsIDOMProcessingInstruction** aReturn)
{
  if (IsXHTML()) {
    return nsDocument::CreateProcessingInstruction(aTarget, aData, aReturn);
  }

  // There are no PIs for HTML
  *aReturn = nsnull;

  return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
}

NS_IMETHODIMP
nsHTMLDocument::CreateCDATASection(const nsAString& aData,
                                   nsIDOMCDATASection** aReturn)
{
  if (IsXHTML()) {
    return nsDocument::CreateCDATASection(aData, aReturn);
  }

  // There are no CDATASections in HTML
  *aReturn = nsnull;

  return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
}

NS_IMETHODIMP
nsHTMLDocument::CreateEntityReference(const nsAString& aName,
                                      nsIDOMEntityReference** aReturn)
{
  if (IsXHTML()) {
    return nsDocument::CreateEntityReference(aName, aReturn);
  }

  // There are no EntityReferences in HTML
  *aReturn = nsnull;

  return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
}

NS_IMETHODIMP
nsHTMLDocument::GetDoctype(nsIDOMDocumentType** aDocumentType)
{
  return nsDocument::GetDoctype(aDocumentType);
}

NS_IMETHODIMP
nsHTMLDocument::GetImplementation(nsIDOMDOMImplementation** aImplementation)
{
  return nsDocument::GetImplementation(aImplementation);
}

NS_IMETHODIMP
nsHTMLDocument::GetDocumentElement(nsIDOMElement** aDocumentElement)
{
  return nsDocument::GetDocumentElement(aDocumentElement);
}

NS_IMETHODIMP
nsHTMLDocument::CreateDocumentFragment(nsIDOMDocumentFragment** aReturn)
{
  return nsDocument::CreateDocumentFragment(aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::CreateComment(const nsAString& aData, nsIDOMComment** aReturn)
{
  return nsDocument::CreateComment(aData, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::CreateAttribute(const nsAString& aName, nsIDOMAttr** aReturn)
{
  return nsDocument::CreateAttribute(aName, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::CreateTextNode(const nsAString& aData, nsIDOMText** aReturn)
{
  return nsDocument::CreateTextNode(aData, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::GetElementsByTagName(const nsAString& aTagname,
                                     nsIDOMNodeList** aReturn)
{
  nsAutoString tmp(aTagname);
//  if (!IsXHTML()) {
//    ToLowerCase(tmp); // HTML elements are lower case internally.
//  }
  return nsDocument::GetElementsByTagName(tmp, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::GetBaseURI(nsAString &aURI)
{
  aURI.Truncate();
  nsIURI *uri = mDocumentBaseURI; // WEAK

  if (!uri) {
    uri = mDocumentURI;
  }

  if (uri) {
    nsCAutoString spec;
    uri->GetSpec(spec);

    CopyUTF8toUTF16(spec, aURI);
  }

  return NS_OK;
}

// nsIDOM3Document interface implementation
NS_IMETHODIMP
nsHTMLDocument::GetXmlEncoding(nsAString& aXmlEncoding)
{
  if (IsXHTML()) {
    return nsDocument::GetXmlEncoding(aXmlEncoding);
  }

  SetDOMStringToNull(aXmlEncoding);

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetXmlStandalone(PRBool *aXmlStandalone)
{
  if (IsXHTML()) {
    return nsDocument::GetXmlStandalone(aXmlStandalone);
  }

  *aXmlStandalone = PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetXmlStandalone(PRBool aXmlStandalone)
{
  if (IsXHTML()) {
    return nsDocument::SetXmlStandalone(aXmlStandalone);
  }

  return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
}


NS_IMETHODIMP
nsHTMLDocument::GetXmlVersion(nsAString& aXmlVersion)
{
  if (IsXHTML()) {
    return nsDocument::GetXmlVersion(aXmlVersion);
  }

  SetDOMStringToNull(aXmlVersion);

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetXmlVersion(const nsAString& aXmlVersion)
{
  if (IsXHTML()) {
    return nsDocument::SetXmlVersion(aXmlVersion);
  }

  return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
}

//
// nsIDOMHTMLDocument interface implementation
//
NS_IMETHODIMP
nsHTMLDocument::GetTitle(nsAString& aTitle)
{
  return nsDocument::GetTitle(aTitle);
}

NS_IMETHODIMP
nsHTMLDocument::GetReferrer(nsAString& aReferrer)
{
  return nsDocument::GetReferrer(aReferrer);
}

void
nsHTMLDocument::GetDomainURI(nsIURI **aURI)
{
  nsIPrincipal *principal = NodePrincipal();

  principal->GetDomain(aURI);
  if (!*aURI) {
    principal->GetURI(aURI);
  }
}


NS_IMETHODIMP
nsHTMLDocument::GetDomain(nsAString& aDomain)
{
  nsCOMPtr<nsIURI> uri;
  GetDomainURI(getter_AddRefs(uri));

  if (!uri) {
    return NS_ERROR_FAILURE;
  }

  nsCAutoString hostName;

  if (NS_SUCCEEDED(uri->GetHost(hostName))) {
    CopyUTF8toUTF16(hostName, aDomain);
  } else {
    // If we can't get the host from the URI (e.g. about:, javascript:,
    // etc), just return an null string.
    SetDOMStringToNull(aDomain);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetDomain(const nsAString& aDomain)
{
  if (aDomain.IsEmpty())
    return NS_ERROR_DOM_BAD_DOCUMENT_DOMAIN;

  // Create new URI
  nsCOMPtr<nsIURI> uri;
  GetDomainURI(getter_AddRefs(uri));

  if (!uri) {
    return NS_ERROR_FAILURE;
  }

  nsCAutoString newURIString;
  if (NS_FAILED(uri->GetScheme(newURIString)))
    return NS_ERROR_FAILURE;
  nsCAutoString path;
  if (NS_FAILED(uri->GetPath(path)))
    return NS_ERROR_FAILURE;
  newURIString.AppendLiteral("://");
  AppendUTF16toUTF8(aDomain, newURIString);
  newURIString.Append(path);

  nsCOMPtr<nsIURI> newURI;
  if (NS_FAILED(NS_NewURI(getter_AddRefs(newURI), newURIString)))
    return NS_ERROR_FAILURE;

  // Check new domain - must be a superdomain of the current host
  // For example, a page from foo.bar.com may set domain to bar.com,
  // but not to ar.com, baz.com, or fi.foo.bar.com.
  nsCAutoString current, domain;
  if (NS_FAILED(uri->GetAsciiHost(current)))
    current.Truncate();
  if (NS_FAILED(newURI->GetAsciiHost(domain)))
    domain.Truncate();

  PRBool ok = current.Equals(domain);
  if (current.Length() > domain.Length() &&
      StringEndsWith(current, domain) &&
      current.CharAt(current.Length() - domain.Length() - 1) == '.') {
    // We're golden if the new domain is the current page's base domain or a
    // subdomain of it.
    nsCOMPtr<nsIEffectiveTLDService> tldService =
      do_GetService(NS_EFFECTIVETLDSERVICE_CONTRACTID);
    if (!tldService)
      return NS_ERROR_NOT_AVAILABLE;

    nsCAutoString currentBaseDomain;
    ok = NS_SUCCEEDED(tldService->GetBaseDomain(uri, 0, currentBaseDomain));
    NS_ASSERTION(StringEndsWith(domain, currentBaseDomain) ==
                 (domain.Length() >= currentBaseDomain.Length()),
                 "uh-oh!  slight optimization wasn't valid somehow!");
    ok = ok && domain.Length() >= currentBaseDomain.Length();
  }
  if (!ok) {
    // Error: illegal domain
    return NS_ERROR_DOM_BAD_DOCUMENT_DOMAIN;
  }

  return NodePrincipal()->SetDomain(newURI);
}

NS_IMETHODIMP
nsHTMLDocument::GetURL(nsAString& aURL)
{
  nsCAutoString str;

  if (mDocumentURI) {
    mDocumentURI->GetSpec(str);
  }

  CopyUTF8toUTF16(str, aURL);

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetBody(nsIDOMHTMLElement** aBody)
{
  *aBody = nsnull;

  nsIContent* body = GetBodyContent();

  if (body) {
    // There is a body element, return that as the body.
    return CallQueryInterface(body, aBody);
  }

  // The document is most likely a frameset document so look for the
  // outer most frameset element
  nsCOMPtr<nsIDOMNodeList> nodeList;

  nsresult rv;
  if (IsXHTML()) {
    rv = GetElementsByTagNameNS(NS_LITERAL_STRING("http://www.w3.org/1999/xhtml"),
                                NS_LITERAL_STRING("frameset"),
                                getter_AddRefs(nodeList));
  } else {
    rv = GetElementsByTagName(NS_LITERAL_STRING("frameset"),
                              getter_AddRefs(nodeList));
  }
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIDOMNode> node;
  nodeList->Item(0, getter_AddRefs(node));

  return node ? CallQueryInterface(node, aBody) : NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetBody(nsIDOMHTMLElement* aBody)
{
  nsCOMPtr<nsIContent> newBody = do_QueryInterface(aBody);
  nsIContent* root = GetRootContent();

  // The body element must be either a body tag or a frameset tag. And we must
  // have a html root tag, otherwise GetBody will not return the newly set
  // body.
  if (!newBody || !(newBody->Tag() == nsGkAtoms::body ||
                    newBody->Tag() == nsGkAtoms::frameset) ||
      !root || !root->IsNodeOfType(nsINode::eHTML) ||
      root->Tag() != nsGkAtoms::html) {
    return NS_ERROR_DOM_HIERARCHY_REQUEST_ERR;
  }

  nsCOMPtr<nsIDOMElement> rootElem = do_QueryInterface(root);
  nsCOMPtr<nsIDOMNode> tmp;

  // Use DOM methods so that we pass through the appropriate security checks.
  nsCOMPtr<nsIDOMNode> currentBody = do_QueryInterface(GetBodyContent());
  if (currentBody) {
    return rootElem->ReplaceChild(aBody, currentBody, getter_AddRefs(tmp));
  }

  return rootElem->AppendChild(aBody, getter_AddRefs(tmp));
}

NS_IMETHODIMP
nsHTMLDocument::GetImages(nsIDOMHTMLCollection** aImages)
{
  if (!mImages) {
    mImages = new nsContentList(this, nsGkAtoms::img, mDefaultNamespaceID);
    if (!mImages) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }

  *aImages = mImages;
  NS_ADDREF(*aImages);

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetApplets(nsIDOMHTMLCollection** aApplets)
{
  if (!mApplets) {
    mApplets = new nsContentList(this, nsGkAtoms::applet,
                                 mDefaultNamespaceID);
    if (!mApplets) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }

  *aApplets = mApplets;
  NS_ADDREF(*aApplets);

  return NS_OK;
}

PRBool
nsHTMLDocument::MatchLinks(nsIContent *aContent, PRInt32 aNamespaceID,
                           nsIAtom* aAtom, void* aData)
{
  nsIDocument* doc = aContent->GetCurrentDoc();

  if (doc) {
    NS_ASSERTION(aContent->IsInDoc(),
                 "This method should never be called on content nodes that "
                 "are not in a document!");
#ifdef DEBUG
    {
      nsCOMPtr<nsIHTMLDocument> htmldoc =
        do_QueryInterface(aContent->GetCurrentDoc());
      NS_ASSERTION(htmldoc,
                   "Huh, how did this happen? This should only be used with "
                   "HTML documents!");
    }
#endif

    nsINodeInfo *ni = aContent->NodeInfo();
    PRInt32 namespaceID = doc->GetDefaultNamespaceID();

    nsIAtom *localName = ni->NameAtom();
    if (ni->NamespaceID() == namespaceID &&
        (localName == nsGkAtoms::a || localName == nsGkAtoms::area)) {
      return aContent->HasAttr(kNameSpaceID_None, nsGkAtoms::href);
    }
  }

  return PR_FALSE;
}

NS_IMETHODIMP
nsHTMLDocument::GetLinks(nsIDOMHTMLCollection** aLinks)
{
  if (!mLinks) {
    mLinks = new nsContentList(this, MatchLinks, nsnull, nsnull);
    if (!mLinks) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }

  *aLinks = mLinks;
  NS_ADDREF(*aLinks);

  return NS_OK;
}

PRBool
nsHTMLDocument::MatchAnchors(nsIContent *aContent, PRInt32 aNamespaceID,
                             nsIAtom* aAtom, void* aData)
{
  NS_ASSERTION(aContent->IsInDoc(),
               "This method should never be called on content nodes that "
               "are not in a document!");
#ifdef DEBUG
  {
    nsCOMPtr<nsIHTMLDocument> htmldoc =
      do_QueryInterface(aContent->GetCurrentDoc());
    NS_ASSERTION(htmldoc,
                 "Huh, how did this happen? This should only be used with "
                 "HTML documents!");
  }
#endif

  PRInt32 namespaceID = aContent->GetCurrentDoc()->GetDefaultNamespaceID();
  if (aContent->NodeInfo()->Equals(nsGkAtoms::a, namespaceID)) {
    return aContent->HasAttr(kNameSpaceID_None, nsGkAtoms::name);
  }

  return PR_FALSE;
}

NS_IMETHODIMP
nsHTMLDocument::GetAnchors(nsIDOMHTMLCollection** aAnchors)
{
  if (!mAnchors) {
    mAnchors = new nsContentList(this, MatchAnchors, nsnull, nsnull);
    if (!mAnchors) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }

  *aAnchors = mAnchors;
  NS_ADDREF(*aAnchors);

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetCookie(nsAString& aCookie)
{
  aCookie.Truncate(); // clear current cookie in case service fails;
                      // no cookie isn't an error condition.

  if (mDisableCookieAccess) {
    return NS_OK;
  }

  // not having a cookie service isn't an error
  nsCOMPtr<nsICookieService> service = do_GetService(NS_COOKIESERVICE_CONTRACTID);
  if (service) {
    // Get a URI from the document principal. We use the original
    // codebase in case the codebase was changed by SetDomain
    nsCOMPtr<nsIURI> codebaseURI;
    NodePrincipal()->GetURI(getter_AddRefs(codebaseURI));

    if (!codebaseURI) {
      // Document's principal is not a codebase (may be system), so
      // can't set cookies

      return NS_OK;
    }

    nsXPIDLCString cookie;
    service->GetCookieString(codebaseURI, mChannel, getter_Copies(cookie));
    CopyASCIItoUTF16(cookie, aCookie);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetCookie(const nsAString& aCookie)
{
  if (mDisableCookieAccess) {
    return NS_OK;
  }

  // not having a cookie service isn't an error
  nsCOMPtr<nsICookieService> service = do_GetService(NS_COOKIESERVICE_CONTRACTID);
  if (service && mDocumentURI) {
    nsCOMPtr<nsIPrompt> prompt;
    nsCOMPtr<nsPIDOMWindow> window = GetWindow();
    if (window) {
      window->GetPrompter(getter_AddRefs(prompt));
    }

    nsCOMPtr<nsIURI> codebaseURI;
    NodePrincipal()->GetURI(getter_AddRefs(codebaseURI));

    if (!codebaseURI) {
      // Document's principal is not a codebase (may be system), so
      // can't set cookies

      return NS_OK;
    }

    NS_LossyConvertUTF16toASCII cookie(aCookie);
    service->SetCookieString(codebaseURI, prompt, cookie.get(), mChannel);
  }

  return NS_OK;
}

// XXX TBI: accepting arguments to the open method.
nsresult
nsHTMLDocument::OpenCommon(const nsACString& aContentType, PRBool aReplace)
{
  if (IsXHTML()) {
    // No calling document.open() on XHTML

    return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
  }

  nsresult rv = NS_OK;

  // If we already have a parser we ignore the document.open call.
  if (mParser) {

    return NS_OK;
  }

  NS_ASSERTION(nsContentUtils::CanCallerAccess(static_cast<nsIDOMHTMLDocument*>(this)),
               "XOW should have caught this!");

  if (!aContentType.EqualsLiteral("text/html") &&
      !aContentType.EqualsLiteral("text/plain")) {
    NS_WARNING("Unsupported type; fix the caller");
    return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
  }

  // check whether we're in the middle of unload.  If so, ignore this call.
  nsCOMPtr<nsIDocShell> shell = do_QueryReferent(mDocumentContainer);
  if (shell) {
    PRBool inUnload;
    shell->GetIsInUnload(&inUnload);
    if (inUnload) {
      return NS_OK;
    }
  }

  // Note: We want to use GetDocumentFromContext here because this document
  // should inherit the security information of the document that's opening us,
  // (since if it's secure, then it's presumeably trusted).
  nsCOMPtr<nsIDocument> callerDoc =
    do_QueryInterface(nsContentUtils::GetDocumentFromContext());

  // Grab a reference to the calling documents security info (if any)
  // and principal as it may be lost in the call to Reset().
  nsCOMPtr<nsISupports> securityInfo;
  if (callerDoc) {
    securityInfo = callerDoc->GetSecurityInfo();
  }

  nsCOMPtr<nsIPrincipal> callerPrincipal;
  nsIScriptSecurityManager *secMan = nsContentUtils::GetSecurityManager();

  secMan->GetSubjectPrincipal(getter_AddRefs(callerPrincipal));

  if (!callerPrincipal) {
    // If we're called from C++ we can't do a document.open w/o
    // changing the principal of the document to something like
    // about:blank (as that's the only sane thing to do when we don't
    // know the origin of this call), and since we can't change the
    // principals of a document for security reasons we'll have to
    // refuse to go ahead with this call.

    return NS_ERROR_DOM_SECURITY_ERR;
  }

  // We're called from script. Make sure the script is from the same
  // origin, not just that the caller can access the document. This is
  // needed to keep document principals from ever changing, which is
  // needed because of the way we use our XOW code, and is a sane
  // thing to do anyways.

  PRBool equals = PR_FALSE;
  if (NS_FAILED(callerPrincipal->Equals(NodePrincipal(), &equals)) ||
      !equals) {
    return NS_ERROR_DOM_SECURITY_ERR;
  }

  // The URI for the document after this call. Get it from the calling
  // principal (if available), or set it to "about:blank" if no
  // principal is reachable.
  nsCOMPtr<nsIURI> uri;
  callerPrincipal->GetURI(getter_AddRefs(uri));

  if (!uri) {
    rv = NS_NewURI(getter_AddRefs(uri),
                   NS_LITERAL_CSTRING("about:blank"));
    NS_ENSURE_SUCCESS(rv, rv);
  }

  nsCOMPtr<nsIDocShell> docshell = do_QueryReferent(mDocumentContainer);

  // Stop current loads targeted at the window this document is in.
  if (mScriptGlobalObject && docshell) {
    nsCOMPtr<nsIContentViewer> cv;
    docshell->GetContentViewer(getter_AddRefs(cv));

    if (cv) {
      PRBool okToUnload;
      rv = cv->PermitUnload(&okToUnload);

      if (NS_SUCCEEDED(rv) && !okToUnload) {
        // We don't want to unload, so stop here, but don't throw an
        // exception.
        return NS_OK;
      }
    }

    nsCOMPtr<nsIWebNavigation> webnav(do_QueryInterface(docshell));
    webnav->Stop(nsIWebNavigation::STOP_NETWORK);
  }

  // The open occurred after the document finished loading.
  // So we reset the document and create a new one.
  nsCOMPtr<nsIChannel> channel;
  nsCOMPtr<nsILoadGroup> group = do_QueryReferent(mDocumentLoadGroup);

  rv = NS_NewChannel(getter_AddRefs(channel), uri, nsnull, group);

  if (NS_FAILED(rv)) {
    return rv;
  }

  // Set the caller principal, if any, on the channel so that we'll
  // make sure to use it when we reset.
  rv = channel->SetOwner(callerPrincipal);
  NS_ENSURE_SUCCESS(rv, rv);

  // Before we reset the doc notify the globalwindow of the change,
  // but only if we still have a window (i.e. our window object the
  // current inner window in our outer window).

  // Hold onto ourselves on the offchance that we're down to one ref
  nsCOMPtr<nsIDOMDocument> kungFuDeathGrip =
    do_QueryInterface((nsIHTMLDocument*)this);

  nsPIDOMWindow *window = GetInnerWindow();
  if (window) {
    // Remember the old scope in case the call to SetNewDocument changes it.
    nsCOMPtr<nsIScriptGlobalObject> oldScope(do_QueryReferent(mScopeObject));

    rv = window->SetNewDocument(this, nsnull, PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);

    // Now make sure we're not flagged as the initial document anymore, now
    // that we've had stuff done to us.  From now on, if anyone tries to
    // document.open() us, they get a new inner window.
    SetIsInitialDocument(PR_FALSE);

    nsCOMPtr<nsIScriptGlobalObject> newScope(do_QueryReferent(mScopeObject));
    if (oldScope && newScope != oldScope) {
      nsContentUtils::ReparentContentWrappersInScope(oldScope, newScope);
    }
  }

  // XXX This is a nasty workaround for a scrollbar code bug
  // (http://bugzilla.mozilla.org/show_bug.cgi?id=55334).

  // Hold on to our root element
  nsCOMPtr<nsIContent> root = GetRootContent();

  if (root) {
    PRInt32 rootIndex = mChildren.IndexOfChild(root);
    NS_ASSERTION(rootIndex >= 0, "Root must be in list!");
    
    PRUint32 count = root->GetChildCount();

    // Remove all the children from the root.
    while (count-- > 0) {
      root->RemoveChildAt(count, PR_TRUE);
    }

    count = root->GetAttrCount();

    // Remove all attributes from the root element
    while (count-- > 0) {
      const nsAttrName* name = root->GetAttrNameAt(count);
      root->UnsetAttr(name->NamespaceID(), name->LocalName(), PR_FALSE);
    }

    // Remove the root from the childlist
    mChildren.RemoveChildAt(rootIndex);
    mCachedRootContent = nsnull;
  }

  // Call Reset(), this will now do the full reset, except removing
  // the root from the document, doing that confuses the scrollbar
  // code in mozilla since the document in the root element and all
  // the anonymous content (i.e. scrollbar elements) is set to
  // null.

  Reset(channel, group);

  if (root) {
    // Tear down the frames for the root element.
    MOZ_AUTO_DOC_UPDATE(this, UPDATE_CONTENT_MODEL, PR_TRUE);
    nsNodeUtils::ContentRemoved(this, root, 0);

    // Put the root element back into the document, we don't notify
    // the document about this insertion since the sink will do that
    // for us and that'll create frames for the root element and the
    // scrollbars work as expected (since the document in the root
    // element was never set to null)

    mChildren.AppendChild(root);
  }

  if (IsEditingOn()) {
    // Reset() blows away all event listeners in the document, and our
    // editor relies heavily on those. Midas is turned on, to make it
    // work, re-initialize it to give it a chance to add its event
    // listeners again.

    TurnEditingOff();
    EditingStateChanged();
  }

  // Zap the old title -- otherwise it would hang around until document.close()
  // (which might never come) if the new document doesn't explicitly set one.
  // Void the title to make sure that we actually respect any titles set by the
  // new document.
  SetTitle(EmptyString());
  mDocumentTitle.SetIsVoid(PR_TRUE);

  // Store the security info of the caller now that we're done
  // resetting the document.
  mSecurityInfo = securityInfo;

  mParser = do_CreateInstance(kCParserCID, &rv);

  // This will be propagated to the parser when someone actually calls write()
  mContentType = aContentType;

  mWriteState = eDocumentOpened;

  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIHTMLContentSink> sink;

    rv = NS_NewHTMLContentSink(getter_AddRefs(sink), this, uri, docshell,
                               channel);
    NS_ENSURE_SUCCESS(rv, rv);

    mParser->SetContentSink(sink);
  }

  // Prepare the docshell and the document viewer for the impending
  // out of band document.write()
  if (docshell) {
    docshell->PrepareForNewContentModel();

    // Now check whether we were opened with a "replace" argument.  If
    // so, we need to tell the docshell to not create a new history
    // entry for this load. Otherwise, make sure that we're doing a normal load,
    // not whatever type of load was previously done on this docshell.
    docshell->SetLoadType(aReplace ? LOAD_NORMAL_REPLACE : LOAD_NORMAL);

    nsCOMPtr<nsIContentViewer> cv;
    docshell->GetContentViewer(getter_AddRefs(cv));
    nsCOMPtr<nsIDocumentViewer> docViewer = do_QueryInterface(cv);
    if (docViewer) {
      docViewer->LoadStart(static_cast<nsIHTMLDocument *>(this));
    }
  }

  // Add a wyciwyg channel request into the document load group
  NS_ASSERTION(!mWyciwygChannel, "nsHTMLDocument::OpenCommon(): wyciwyg "
               "channel already exists!");

  // In case the editor is listening and will see the new channel
  // being added, make sure mWriteLevel is non-zero so that the editor
  // knows that document.open/write/close() is being called on this
  // document.
  ++mWriteLevel;

  CreateAndAddWyciwygChannel();

  --mWriteLevel;

  return rv;
}

NS_IMETHODIMP
nsHTMLDocument::Open()
{
  nsCOMPtr<nsIDOMDocument> doc;
  return Open(NS_LITERAL_CSTRING("text/html"), PR_FALSE, getter_AddRefs(doc));
}

NS_IMETHODIMP
nsHTMLDocument::Open(const nsACString& aContentType, PRBool aReplace,
                     nsIDOMDocument** aReturn)
{
  nsresult rv = OpenCommon(aContentType, aReplace);
  NS_ENSURE_SUCCESS(rv, rv);

  return CallQueryInterface(this, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::Clear()
{
  // This method has been deprecated
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::Close()
{
  if (IsXHTML()) {
    // No calling document.close() on XHTML!

    return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
  }

  nsresult rv = NS_OK;

  if (mParser && mWriteState == eDocumentOpened) {
    mPendingScripts.RemoveElement(GenerateParserKey());

    mWriteState = mPendingScripts.Count() == 0
                  ? eDocumentClosed
                  : ePendingClose;

    ++mWriteLevel;
    rv = mParser->Parse(EmptyString(), mParser->GetRootContextKey(),
                        mContentType, PR_TRUE);
    --mWriteLevel;

    // XXX Make sure that all the document.written content is
    // reflowed.  We should remove this call once we change
    // nsHTMLDocument::OpenCommon() so that it completely destroys the
    // earlier document's content and frame hierarchy.  Right now, it
    // re-uses the earlier document's root content object and
    // corresponding frame objects.  These re-used frame objects think
    // that they have already been reflowed, so they drop initial
    // reflows.  For certain cases of document.written content, like a
    // frameset document, the dropping of the initial reflow means
    // that we end up in document.close() without appended any reflow
    // commands to the reflow queue and, consequently, without adding
    // the dummy layout request to the load group.  Since the dummy
    // layout request is not added to the load group, the onload
    // handler of the frameset fires before the frames get reflowed
    // and loaded.  That is the long explanation for why we need this
    // one line of code here!
    // XXXbz as far as I can tell this may not be needed anymore; all
    // the testcases in bug 57636 pass without this line...  Leaving
    // it be for now, though.  In any case, there's no reason to do
    // this if we have no presshell, since in that case none of the
    // above about reusing frames applies.
    if (GetPrimaryShell()) {
      FlushPendingNotifications(Flush_Layout);
    }

    // Remove the wyciwyg channel request from the document load group
    // that we added in OpenCommon().  If all other requests between
    // document.open() and document.close() have completed, then this
    // method should cause the firing of an onload event.
    NS_ASSERTION(mWyciwygChannel, "nsHTMLDocument::Close(): Trying to remove "
                 "non-existent wyciwyg channel!");
    RemoveWyciwygChannel();
    NS_ASSERTION(!mWyciwygChannel, "nsHTMLDocument::Close(): "
                 "nsIWyciwygChannel could not be removed!");
  }

  return NS_OK;
}

nsresult
nsHTMLDocument::WriteCommon(const nsAString& aText,
                            PRBool aNewlineTerminate)
{
  mTooDeepWriteRecursion =
    (mWriteLevel > NS_MAX_DOCUMENT_WRITE_DEPTH || mTooDeepWriteRecursion);
  NS_ENSURE_STATE(!mTooDeepWriteRecursion);

  if (IsXHTML()) {
    // No calling document.write*() on XHTML!

    return NS_ERROR_DOM_NOT_SUPPORTED_ERR;
  }

  nsresult rv = NS_OK;

  void *key = GenerateParserKey();
  if (mWriteState == eDocumentClosed ||
      (mWriteState == ePendingClose &&
       mPendingScripts.IndexOf(key) == kNotFound)) {
    mWriteState = eDocumentClosed;
    mParser->Terminate();
    NS_ASSERTION(!mParser, "mParser should have been null'd out");
  }

  if (!mParser) {
    rv = Open();

    // If Open() fails, or if it didn't create a parser (as it won't
    // if the user chose to not discard the current document through
    // onbeforeunload), don't write anything.
    if (NS_FAILED(rv) || !mParser) {
      return rv;
    }
  }

  static NS_NAMED_LITERAL_STRING(new_line, "\n");

  // Save the data in cache
  if (mWyciwygChannel) {
    if (!aText.IsEmpty()) {
      mWyciwygChannel->WriteToCacheEntry(aText);
    }

    if (aNewlineTerminate) {
      mWyciwygChannel->WriteToCacheEntry(new_line);
    }
  }

  ++mWriteLevel;

  // This could be done with less code, but for performance reasons it
  // makes sense to have the code for two separate Parse() calls here
  // since the concatenation of strings costs more than we like. And
  // why pay that price when we don't need to?
  if (aNewlineTerminate) {
    rv = mParser->Parse(aText + new_line,
                        key, mContentType,
                        (mWriteState == eNotWriting || (mWriteLevel > 1)));
  } else {
    rv = mParser->Parse(aText,
                        key, mContentType,
                        (mWriteState == eNotWriting || (mWriteLevel > 1)));
  }

  --mWriteLevel;

  mTooDeepWriteRecursion = (mWriteLevel != 0 && mTooDeepWriteRecursion);

  return rv;
}

NS_IMETHODIMP
nsHTMLDocument::Write(const nsAString& aText)
{
  return WriteCommon(aText, PR_FALSE);
}

NS_IMETHODIMP
nsHTMLDocument::Writeln(const nsAString& aText)
{
  return WriteCommon(aText, PR_TRUE);
}

nsresult
nsHTMLDocument::ScriptWriteCommon(PRBool aNewlineTerminate)
{
  nsAXPCNativeCallContext *ncc = nsnull;

  nsresult rv = nsContentUtils::XPConnect()->
    GetCurrentNativeCallContext(&ncc);
  NS_ENSURE_SUCCESS(rv, rv);

  if (ncc) {
    // We're called from JS, concatenate the extra arguments into
    // string_buffer
    PRUint32 i, argc;

    ncc->GetArgc(&argc);

    JSContext *cx = nsnull;
    rv = ncc->GetJSContext(&cx);
    NS_ENSURE_SUCCESS(rv, rv);

    jsval *argv = nsnull;
    ncc->GetArgvPtr(&argv);
    NS_ENSURE_TRUE(argv, NS_ERROR_UNEXPECTED);

    if (argc == 1) {
      JSAutoRequest ar(cx);

      JSString *jsstr = JS_ValueToString(cx, argv[0]);
      NS_ENSURE_TRUE(jsstr, NS_ERROR_OUT_OF_MEMORY);

      nsDependentString str(reinterpret_cast<const PRUnichar *>
                                            (::JS_GetStringChars(jsstr)),
                          ::JS_GetStringLength(jsstr));

      return WriteCommon(str, aNewlineTerminate);
    }

    if (argc > 1) {
      nsAutoString string_buffer;

      for (i = 0; i < argc; ++i) {
        JSAutoRequest ar(cx);

        JSString *str = JS_ValueToString(cx, argv[i]);
        NS_ENSURE_TRUE(str, NS_ERROR_OUT_OF_MEMORY);

        string_buffer.Append(reinterpret_cast<const PRUnichar *>
                                             (::JS_GetStringChars(str)),
                             ::JS_GetStringLength(str));
      }

      return WriteCommon(string_buffer, aNewlineTerminate);
    }
  }

  // No arguments...
  return WriteCommon(EmptyString(), aNewlineTerminate);
}

NS_IMETHODIMP
nsHTMLDocument::Write()
{
  return ScriptWriteCommon(PR_FALSE);
}

NS_IMETHODIMP
nsHTMLDocument::Writeln()
{
  return ScriptWriteCommon(PR_TRUE);
}

NS_IMETHODIMP
nsHTMLDocument::GetElementById(const nsAString& aElementId,
                               nsIDOMElement** aReturn)
{
  NS_ENSURE_ARG_POINTER(aReturn);
  *aReturn = nsnull;

  nsCOMPtr<nsIAtom> idAtom(do_GetAtom(aElementId));
  NS_ENSURE_TRUE(idAtom, NS_ERROR_OUT_OF_MEMORY);

  // We don't have to flush before we do the initial hashtable lookup, since if
  // the id is already in the hashtable it couldn't have been removed without
  // us being notified (all removals notify immediately, as far as I can tell).
  // So do the lookup first.
  IdAndNameMapEntry *entry =
    static_cast<IdAndNameMapEntry *>
               (PL_DHashTableOperate(&mIdAndNameHashTable, idAtom,
                                        PL_DHASH_ADD));
  NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);

  nsIContent *e = entry->GetIdContent();

  if (!e || e == ID_NOT_IN_DOCUMENT) {
    // Now we have to flush.  It could be that we have a cached "not in
    // document" or know nothing about this ID yet but more content has been
    // added to the document since.  Note that we have to flush notifications,
    // so that the entry will get updated properly.
    
    // Make sure to stash away the current generation so we can check whether
    // the table changes when we flush.
    PRUint32 generation = mIdAndNameHashTable.generation;
  
    FlushPendingNotifications(Flush_ContentAndNotify);

    if (generation != mIdAndNameHashTable.generation) {
      // Table changed, so the entry pointer is no longer valid; look up the
      // entry again, adding if necessary (the adding may be necessary in case
      // the flush actually deleted entries).
      entry =
        static_cast<IdAndNameMapEntry *>
                   (PL_DHashTableOperate(&mIdAndNameHashTable, idAtom,
                                            PL_DHASH_ADD));
      NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);
    }

    // We could now have a new entry, or the entry could have been
    // updated, so update e to point to the current entry's
    // mIdContent.
    e = entry->GetIdContent();
  }

  if (e == ID_NOT_IN_DOCUMENT) {
    // We've looked for this id before and we didn't find it, so it
    // won't be in the document now either (since the
    // mIdAndNameHashTable is live for entries in the table)

    return NS_OK;
  }

  if (!e) {
    // If IdTableIsLive(), no need to look for the element in the document,
    // since we're fully maintaining our table's state as the DOM mutates.
    nsIContent* root = GetRootContent();
    if (!IdTableIsLive()) {
      if (IdTableShouldBecomeLive()) {
        // Just make sure our table is up to date and call this method again
        // to look up in the hashtable.
        if (root) {
          RegisterNamedItems(root);
        }
        return GetElementById(aElementId, aReturn);
      }

      if (root && CheckGetElementByIdArg(aElementId)) {
        e = nsContentUtils::MatchElementId(root, idAtom);
      }
    }

    if (!e) {
#ifdef DEBUG
      // No reason to call MatchElementId if !IdTableIsLive, since
      // we'd have done just that already
      if (IdTableIsLive() && root && !aElementId.IsEmpty()) {
        nsIContent* eDebug =
          nsContentUtils::MatchElementId(root, idAtom);
        NS_ASSERTION(!eDebug,
                     "We got null for |e| but MatchElementId found something?");
      }
#endif
      // There is no element with the given id in the document, cache
      // the fact that it's not in the document
      entry->FlagIDNotInDocument();

      return NS_OK;
    }

    // We found an element with a matching id, store that in the hash
    if (NS_UNLIKELY(!entry->AddIdContent(e))) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }

  return CallQueryInterface(e, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::ImportNode(nsIDOMNode* aImportedNode,
                           PRBool aDeep,
                           nsIDOMNode** aReturn)
{
  return nsDocument::ImportNode(aImportedNode, aDeep, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::CreateAttributeNS(const nsAString& aNamespaceURI,
                                  const nsAString& aQualifiedName,
                                  nsIDOMAttr** aReturn)
{
  return nsDocument::CreateAttributeNS(aNamespaceURI, aQualifiedName, aReturn);
}

NS_IMETHODIMP
nsHTMLDocument::GetElementsByTagNameNS(const nsAString& aNamespaceURI,
                                       const nsAString& aLocalName,
                                       nsIDOMNodeList** aReturn)
{
  nsAutoString tmp(aLocalName);

//  if (!IsXHTML()) {
//    ToLowerCase(tmp); // HTML elements are lower case internally.
//  }

  return nsDocument::GetElementsByTagNameNS(aNamespaceURI, tmp, aReturn);
}

PRBool
nsHTMLDocument::MatchNameAttribute(nsIContent* aContent, PRInt32 aNamespaceID,
                                   nsIAtom* aAtom, void* aData)
{
  NS_PRECONDITION(aContent, "Must have content node to work with!");
  nsString* elementName = static_cast<nsString*>(aData);
  return aContent->AttrValueIs(kNameSpaceID_None, nsGkAtoms::name,
                               *elementName, eCaseMatters);
}

NS_IMETHODIMP
nsHTMLDocument::GetElementsByName(const nsAString& aElementName,
                                  nsIDOMNodeList** aReturn)
{
  void* elementNameData = new nsString(aElementName);
  NS_ENSURE_TRUE(elementNameData, NS_ERROR_OUT_OF_MEMORY);
  nsContentList* elements =
    new nsContentList(this,
                      MatchNameAttribute,
                      nsContentUtils::DestroyMatchString,
                      elementNameData);
  NS_ENSURE_TRUE(elements, NS_ERROR_OUT_OF_MEMORY);

  *aReturn = elements;
  NS_ADDREF(*aReturn);

  return NS_OK;
}

void
nsHTMLDocument::ScriptLoading(nsIScriptElement *aScript)
{
  if (mWriteState == eNotWriting) {
    return;
  }

  mPendingScripts.AppendElement(aScript);
}

void
nsHTMLDocument::ScriptExecuted(nsIScriptElement *aScript)
{
  if (mWriteState == eNotWriting) {
    return;
  }

  mPendingScripts.RemoveElement(aScript);
  if (mPendingScripts.Count() == 0 && mWriteState == ePendingClose) {
    // The last pending script just finished, terminate our parser now.
    mWriteState = eDocumentClosed;
  }
}

void
nsHTMLDocument::AddedForm()
{
  ++mNumForms;
}

void
nsHTMLDocument::RemovedForm()
{
  --mNumForms;
}

PRInt32
nsHTMLDocument::GetNumFormsSynchronous()
{
  return mNumForms;
}

nsresult
nsHTMLDocument::GetBodySize(PRInt32* aWidth,
                            PRInt32* aHeight)
{
  *aWidth = *aHeight = 0;

  FlushPendingNotifications(Flush_Layout);

  nsCOMPtr<nsIPresShell> shell = GetPrimaryShell();
  
  if (!shell)
    return NS_OK;

  // Find the <body> element: this is what we'll want to use for the
  // document's width and height values.
  nsIContent* body = GetBodyContent();
  if (!body) {
    return NS_OK;
  }

  // Now grab its frame
  nsIFrame* frame = shell->GetPrimaryFrameFor(body);
  if (!frame)
    return NS_OK;
  
  nsSize size = frame->GetSize();

  *aWidth = nsPresContext::AppUnitsToIntCSSPixels(size.width);
  *aHeight = nsPresContext::AppUnitsToIntCSSPixels(size.height);

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetWidth(PRInt32* aWidth)
{
  NS_ENSURE_ARG_POINTER(aWidth);

  PRInt32 height;
  return GetBodySize(aWidth, &height);
}

NS_IMETHODIMP
nsHTMLDocument::GetHeight(PRInt32* aHeight)
{
  NS_ENSURE_ARG_POINTER(aHeight);

  PRInt32 width;
  return GetBodySize(&width, aHeight);
}

NS_IMETHODIMP
nsHTMLDocument::GetAlinkColor(nsAString& aAlinkColor)
{
  aAlinkColor.Truncate();

  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->GetALink(aAlinkColor);
  } else if (mAttrStyleSheet) {
    nscolor color;
    nsresult rv = mAttrStyleSheet->GetActiveLinkColor(color);
    if (NS_SUCCEEDED(rv)) {
      NS_RGBToHex(color, aAlinkColor);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetAlinkColor(const nsAString& aAlinkColor)
{
  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->SetALink(aAlinkColor);
  } else if (mAttrStyleSheet) {
    nsAttrValue value;
    if (value.ParseColor(aAlinkColor, this)) {
      nscolor color;
      value.GetColorValue(color);
      mAttrStyleSheet->SetActiveLinkColor(color);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetLinkColor(nsAString& aLinkColor)
{
  aLinkColor.Truncate();

  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->GetLink(aLinkColor);
  } else if (mAttrStyleSheet) {
    nscolor color;
    nsresult rv = mAttrStyleSheet->GetLinkColor(color);
    if (NS_SUCCEEDED(rv)) {
      NS_RGBToHex(color, aLinkColor);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetLinkColor(const nsAString& aLinkColor)
{
  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->SetLink(aLinkColor);
  } else if (mAttrStyleSheet) {
    nsAttrValue value;
    if (value.ParseColor(aLinkColor, this)) {
      nscolor color;
      value.GetColorValue(color);
      mAttrStyleSheet->SetLinkColor(color);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetVlinkColor(nsAString& aVlinkColor)
{
  aVlinkColor.Truncate();

  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->GetVLink(aVlinkColor);
  } else if (mAttrStyleSheet) {
    nscolor color;
    nsresult rv = mAttrStyleSheet->GetVisitedLinkColor(color);
    if (NS_SUCCEEDED(rv)) {
      NS_RGBToHex(color, aVlinkColor);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetVlinkColor(const nsAString& aVlinkColor)
{
  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->SetVLink(aVlinkColor);
  } else if (mAttrStyleSheet) {
    nsAttrValue value;
    if (value.ParseColor(aVlinkColor, this)) {
      nscolor color;
      value.GetColorValue(color);
      mAttrStyleSheet->SetVisitedLinkColor(color);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetBgColor(nsAString& aBgColor)
{
  aBgColor.Truncate();

  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->GetBgColor(aBgColor);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetBgColor(const nsAString& aBgColor)
{
  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->SetBgColor(aBgColor);
  }
  // XXXldb And otherwise?

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetFgColor(nsAString& aFgColor)
{
  aFgColor.Truncate();

  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->GetText(aFgColor);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetFgColor(const nsAString& aFgColor)
{
  nsCOMPtr<nsIDOMHTMLBodyElement> body = do_QueryInterface(GetBodyContent());

  if (body) {
    body->SetText(aFgColor);
  }
  // XXXldb And otherwise?

  return NS_OK;
}


NS_IMETHODIMP
nsHTMLDocument::GetEmbeds(nsIDOMHTMLCollection** aEmbeds)
{
  if (!mEmbeds) {
    mEmbeds = new nsContentList(this, nsGkAtoms::embed, mDefaultNamespaceID);
    if (!mEmbeds) {
      return NS_ERROR_OUT_OF_MEMORY;
    }
  }

  *aEmbeds = mEmbeds;
  NS_ADDREF(*aEmbeds);

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::GetSelection(nsAString& aReturn)
{
  aReturn.Truncate();

  nsCOMPtr<nsIConsoleService> consoleService
    (do_GetService("@mozilla.org/consoleservice;1"));

  if (consoleService) {
    consoleService->LogStringMessage(NS_LITERAL_STRING("Deprecated method document.getSelection() called.  Please use window.getSelection() instead.").get());
  }

  nsIDOMWindow *window = GetWindow();
  NS_ENSURE_TRUE(window, NS_OK);

  nsCOMPtr<nsISelection> selection;
  nsresult rv = window->GetSelection(getter_AddRefs(selection));
  NS_ENSURE_TRUE(selection && NS_SUCCEEDED(rv), rv);

  nsXPIDLString str;

  rv = selection->ToString(getter_Copies(str));

  aReturn.Assign(str);

  return rv;
}

static void
ReportUseOfDeprecatedMethod(nsHTMLDocument* aDoc, const char* aWarning)
{
  nsContentUtils::ReportToConsole(nsContentUtils::eDOM_PROPERTIES,
                                  aWarning,
                                  nsnull, 0,
                                  static_cast<nsIDocument*>(aDoc)->
                                    GetDocumentURI(),
                                  EmptyString(), 0, 0,
                                  nsIScriptError::warningFlag,
                                  "DOM Events");
}

NS_IMETHODIMP
nsHTMLDocument::CaptureEvents(PRInt32 aEventFlags)
{
  ReportUseOfDeprecatedMethod(this, "UseOfCaptureEventsWarning");
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::ReleaseEvents(PRInt32 aEventFlags)
{
  ReportUseOfDeprecatedMethod(this, "UseOfReleaseEventsWarning");
  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::RouteEvent(nsIDOMEvent* aEvt)
{
  ReportUseOfDeprecatedMethod(this, "UseOfRouteEventWarning");
  return NS_OK;
}

// readonly attribute DOMString compatMode;
// Returns "BackCompat" if we are in quirks mode, "CSS1Compat" if we are
// in almost standards or full standards mode. See bug 105640.  This was
// implemented to match MSIE's compatMode property
NS_IMETHODIMP
nsHTMLDocument::GetCompatMode(nsAString& aCompatMode)
{
  NS_ASSERTION(mCompatMode == eCompatibility_NavQuirks ||
               mCompatMode == eCompatibility_AlmostStandards ||
               mCompatMode == eCompatibility_FullStandards,
               "mCompatMode is neither quirks nor strict for this document");

  if (mCompatMode == eCompatibility_NavQuirks) {
    aCompatMode.AssignLiteral("BackCompat");
  } else {
    aCompatMode.AssignLiteral("CSS1Compat");
  }

  return NS_OK;
}

// Mapped to document.embeds for NS4 compatibility
NS_IMETHODIMP
nsHTMLDocument::GetPlugins(nsIDOMHTMLCollection** aPlugins)
{
  *aPlugins = nsnull;

  return GetEmbeds(aPlugins);
}

PR_STATIC_CALLBACK(PLDHashOperator)
IdAndNameMapEntryRemoveCallback(PLDHashTable *table, PLDHashEntryHdr *hdr,
                                PRUint32 number, void *arg)
{
  return PL_DHASH_REMOVE;
}


void
nsHTMLDocument::InvalidateHashTables()
{
  PL_DHashTableEnumerate(&mIdAndNameHashTable, IdAndNameMapEntryRemoveCallback,
                         nsnull);
}

static nsresult
ReserveNameInHash(const char* aName, PLDHashTable *aHash)
{
  nsCOMPtr<nsIAtom> atom(do_GetAtom(aName));
  NS_ENSURE_TRUE(atom, NS_ERROR_OUT_OF_MEMORY);
  
  IdAndNameMapEntry *entry =
    static_cast<IdAndNameMapEntry *>
               (PL_DHashTableOperate(aHash, atom, PL_DHASH_ADD));

  NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);

  entry->mNameContentList = NAME_NOT_VALID;

  return NS_OK;
}

// Pre-fill the name hash with names that are likely to be resolved in
// this document to avoid walking the tree looking for elements with
// these names.

nsresult
nsHTMLDocument::PrePopulateHashTables()
{
  nsresult rv = NS_OK;

  rv = ReserveNameInHash("write", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("writeln", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("open", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("close", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("forms", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("elements", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("characterSet", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("nodeType", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("parentNode", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = ReserveNameInHash("cookie", &mIdAndNameHashTable);
  NS_ENSURE_SUCCESS(rv, rv);

  return rv;
}

static nsIAtom*
IsNamedItem(nsIContent* aContent)
{
  // Only the content types reflected in Level 0 with a NAME
  // attribute are registered. Images, layers and forms always get
  // reflected up to the document. Applets and embeds only go
  // to the closest container (which could be a form).
  nsGenericHTMLElement* elm = nsGenericHTMLElement::FromContent(aContent);
  if (!elm) {
    return nsnull;
  }

  nsIAtom* tag = elm->Tag();
  if (tag != nsGkAtoms::img    &&
      tag != nsGkAtoms::form   &&
      tag != nsGkAtoms::applet &&
      tag != nsGkAtoms::embed  &&
      tag != nsGkAtoms::object) {
    return nsnull;
  }

  const nsAttrValue* val = elm->GetParsedAttr(nsGkAtoms::name);
  if (val && val->Type() == nsAttrValue::eAtom) {
    return val->GetAtomValue();
  }

  return nsnull;
}

nsresult
nsHTMLDocument::UpdateNameTableEntry(nsIAtom* aName,
                                     nsIContent *aContent)
{
  NS_ASSERTION(!IsXHTML(), 
  "nsHTMLDocument::UpdateNameTableEntry Don't call me on an XHTML document!!!");

  IdAndNameMapEntry *entry =
    static_cast<IdAndNameMapEntry *>
               (PL_DHashTableOperate(&mIdAndNameHashTable, aName,
                                        PL_DHASH_LOOKUP));

  if (PL_DHASH_ENTRY_IS_FREE(entry)) {
    return NS_OK;
  }

  nsBaseContentList *list = entry->mNameContentList;

  if (!list || list == NAME_NOT_VALID) {
    return NS_OK;
  }

  // NOTE: this indexof is absolutely needed, since we don't flush
  // content notifications when we do document.foo resolution.  So
  // aContent may be in our list already and just now getting notified
  // for!
  if (list->IndexOf(aContent, PR_FALSE) < 0) {
    list->AppendElement(aContent);
  }

  return NS_OK;
}

nsresult
nsHTMLDocument::UpdateIdTableEntry(nsIAtom* aId, nsIContent *aContent)
{
  PRBool liveTable = IdTableIsLive();
  PLDHashOperator op = liveTable ? PL_DHASH_ADD : PL_DHASH_LOOKUP;
  IdAndNameMapEntry *entry =
    static_cast<IdAndNameMapEntry *>
               (PL_DHashTableOperate(&mIdAndNameHashTable, aId,
                                        op));

  NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);

  if (PL_DHASH_ENTRY_IS_BUSY(entry) &&
      NS_UNLIKELY(!entry->AddIdContent(aContent))) {
    // failed to update...
    return NS_ERROR_OUT_OF_MEMORY;
  }

  return NS_OK;
}

nsresult
nsHTMLDocument::RemoveFromNameTable(nsIAtom* aName, nsIContent *aContent)
{
  NS_ASSERTION(!IsXHTML(), 
  "nsHTMLDocument::RemoveFromNameTable Don't call me on an XHTML document!!!");

  IdAndNameMapEntry *entry =
    static_cast<IdAndNameMapEntry *>
               (PL_DHashTableOperate(&mIdAndNameHashTable, aName,
                                        PL_DHASH_LOOKUP));

  if (PL_DHASH_ENTRY_IS_BUSY(entry) && entry->mNameContentList &&
      entry->mNameContentList != NAME_NOT_VALID) {
    entry->mNameContentList->RemoveElement(aContent);
  }

  return NS_OK;
}

nsresult
nsHTMLDocument::RemoveFromIdTable(nsIContent *aContent)
{
  nsIAtom* id = aContent->GetID();

  if (!id) {
    return NS_OK;
  }

  IdAndNameMapEntry *entry =
    static_cast<IdAndNameMapEntry *>
               (PL_DHashTableOperate(&mIdAndNameHashTable, id,
                                        PL_DHASH_LOOKUP));

  if (PL_DHASH_ENTRY_IS_FREE(entry)) {
    return NS_OK;
  }

  if (entry->RemoveIdContent(aContent)) {
    PL_DHashTableRawRemove(&mIdAndNameHashTable, entry);
  }

  return NS_OK;
}

nsresult
nsHTMLDocument::UnregisterNamedItems(nsIContent *aContent)
{
  if (aContent->IsNodeOfType(nsINode::eTEXT)) {
    // Text nodes are not named items nor can they have children.
    return NS_OK;
  }

  nsresult rv = NS_OK;

  if (!IsXHTML()) {
    nsIAtom* name = IsNamedItem(aContent);
    if (name) {
      rv = RemoveFromNameTable(name, aContent);

      if (NS_FAILED(rv)) {
        return rv;
      }
    }
  }

  rv = RemoveFromIdTable(aContent);

  if (NS_FAILED(rv)) {
    return rv;
  }

  PRUint32 i, count = aContent->GetChildCount();

  for (i = 0; i < count; ++i) {
    UnregisterNamedItems(aContent->GetChildAt(i));
  }

  return NS_OK;
}

nsresult
nsHTMLDocument::RegisterNamedItems(nsIContent *aContent)
{
  if (aContent->IsNodeOfType(nsINode::eTEXT)) {
    // Text nodes are not named items nor can they have children.
    return NS_OK;
  }

  if (!IsXHTML()) {
    nsIAtom* name = IsNamedItem(aContent);
    if (name) {
      UpdateNameTableEntry(name, aContent);
    }
  }

  nsIAtom* id = aContent->GetID();
  if (id) {
    nsresult rv = UpdateIdTableEntry(id, aContent);
      
    if (NS_FAILED(rv)) {
      return rv;
    }
  }

  PRUint32 i, count = aContent->GetChildCount();

  for (i = 0; i < count; ++i) {
    RegisterNamedItems(aContent->GetChildAt(i));
  }

  return NS_OK;
}

static void
FindNamedItems(nsIAtom* aName, nsIContent *aContent,
               IdAndNameMapEntry& aEntry, PRBool aIsXHTML)
{
  NS_ASSERTION(aEntry.mNameContentList,
               "Entry w/o content list passed to FindNamedItems()!");
  NS_ASSERTION(aEntry.mNameContentList != NAME_NOT_VALID,
               "Entry that should never have a list passed to FindNamedItems()!");

  if (aContent->IsNodeOfType(nsINode::eTEXT)) {
    // Text nodes are not named items nor can they have children.
    return;
  }

  nsAutoString value;

  if (!aIsXHTML && aName == IsNamedItem(aContent)) {
    aEntry.mNameContentList->AppendElement(aContent);
  }

  if (!aEntry.GetIdContent() &&
      // Maybe this node has the right id?
      aName == aContent->GetID()) {
    aEntry.AddIdContent(aContent);
  }

  PRUint32 i, count = aContent->GetChildCount();

  for (i = 0; i < count; ++i) {
    FindNamedItems(aName, aContent->GetChildAt(i), aEntry, aIsXHTML);
  }
}

nsresult
nsHTMLDocument::ResolveName(const nsAString& aName,
                            nsIDOMHTMLFormElement *aForm,
                            nsISupports **aResult)
{
  *aResult = nsnull;

  if (IsXHTML()) {
    // We don't dynamically resolve names on XHTML documents.

    return NS_OK;
  }

  nsCOMPtr<nsIAtom> name(do_GetAtom(aName));

  // We have built a table and cache the named items. The table will
  // be updated as content is added and removed.

  IdAndNameMapEntry *entry =
    static_cast<IdAndNameMapEntry *>
               (PL_DHashTableOperate(&mIdAndNameHashTable, name,
                                     PL_DHASH_ADD));
  NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);

  if (entry->mNameContentList == NAME_NOT_VALID) {
    // There won't be any named items by this name -- it's reserved
    return NS_OK;
  }

  // Now we know we _might_ have items.  Before looking at
  // entry->mNameContentList, make sure to flush out content (see
  // bug 69826).
  // This is a perf killer while the document is loading!

  // Make sure to stash away the current generation so we can check whether the
  // table changes when we flush.
  PRUint32 generation = mIdAndNameHashTable.generation;
  
  // If we already have an entry->mNameContentList, we need to flush out
  // notifications too, so that it will get updated properly.
  FlushPendingNotifications(entry->mNameContentList ?
                            Flush_ContentAndNotify : Flush_Content);

  if (generation != mIdAndNameHashTable.generation) {
    // Table changed, so the entry pointer is no longer valid; look up the
    // entry again, adding if necessary (the adding may be necessary in case
    // the flush actually deleted entries).
    entry =
      static_cast<IdAndNameMapEntry *>
                 (PL_DHashTableOperate(&mIdAndNameHashTable, name,
                                       PL_DHASH_ADD));
    NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);
  }
    

  nsBaseContentList *list = entry->mNameContentList;

  if (!list) {
#ifdef DEBUG_jst
    {
      printf ("nsHTMLDocument name cache miss for name '%s'\n",
              NS_ConvertUTF16toUTF8(aName).get());
    }
#endif

    list = new nsBaseContentList();
    NS_ENSURE_TRUE(list, NS_ERROR_OUT_OF_MEMORY);

    entry->mNameContentList = list;
    NS_ADDREF(entry->mNameContentList);

    nsIContent* root = GetRootContent();
    if (root && !aName.IsEmpty()) {
      // We'll never get here if !IsXHTML(), so we can just pass
      // PR_FALSE to FindNamedItems().
      FindNamedItems(name, root, *entry, PR_FALSE);
    }
  }

  PRUint32 length;
  list->GetLength(&length);

  if (length > 0) {
    if (length == 1) {
      // Only one element in the list, return the element instead of
      // returning the list

      nsCOMPtr<nsIDOMNode> node;

      list->Item(0, getter_AddRefs(node));

      nsCOMPtr<nsIContent> ourContent(do_QueryInterface(node));
      if (aForm && ourContent &&
          !nsContentUtils::BelongsInForm(aForm, ourContent)) {
        // This is not the content you are looking for
        node = nsnull;
      }

      *aResult = node;
      NS_IF_ADDREF(*aResult);

      return NS_OK;
    }

    // The list contains more than one element, return the whole
    // list, unless...

    if (aForm) {
      // ... we're called from a form, in that case we create a
      // nsFormNameContentList which will filter out the elements in the
      // list that don't belong to aForm

      nsFormContentList *fc_list = new nsFormContentList(aForm, *list);
      NS_ENSURE_TRUE(fc_list, NS_ERROR_OUT_OF_MEMORY);

      PRUint32 len;
      fc_list->GetLength(&len);

      if (len < 2) {
        // After the nsFormContentList is done filtering there's either
        // nothing or one element in the list.  Return that element, or null
        // if there's no element in the list.

        nsCOMPtr<nsIDOMNode> node;

        fc_list->Item(0, getter_AddRefs(node));

        NS_IF_ADDREF(*aResult = node);

        delete fc_list;

        return NS_OK;
      }

      list = fc_list;
    }

    return CallQueryInterface(list, aResult);
  }

  // No named items were found, see if there's one registerd by id for
  // aName. If we get this far, FindNamedItems() will have been called
  // for aName, so we're guaranteed that if there is an element with
  // the id aName, it'll be entry's IdContent.

  nsIContent *e = entry->GetIdContent();

  if (e && e != ID_NOT_IN_DOCUMENT && e->IsNodeOfType(nsINode::eHTML)) {
    nsIAtom *tag = e->Tag();

    if ((tag == nsGkAtoms::embed  ||
         tag == nsGkAtoms::img    ||
         tag == nsGkAtoms::object ||
         tag == nsGkAtoms::applet) &&
        (!aForm || nsContentUtils::BelongsInForm(aForm, e))) {
      NS_ADDREF(*aResult = e);
    }
  }

  return NS_OK;
}

//----------------------------

/* virtual */ nsIContent*
nsHTMLDocument::GetBodyContentExternal()
{
  return GetBodyContent();
}

nsIContent*
nsHTMLDocument::GetBodyContent()
{
  // Loop backwards because any non-elements, such as doctypes and PIs
  // are likely to appear before the root element.
  PRUint32 i;
  for (i = mChildren.ChildCount(); i > 0; --i) {
    nsIContent* html = mChildren.ChildAt(i - 1);
    if (html->Tag() == nsGkAtoms::html &&
        html->IsNodeOfType(nsINode::eHTML)) {

      // Look for body inside html
      for (i = html->GetChildCount(); i > 0; --i) {
        nsIContent* body = html->GetChildAt(i - 1);
        if (body->Tag() == nsGkAtoms::body &&
            body->IsNodeOfType(nsINode::eHTML)) {
          return body;
        }
      }

      break;
    }
  }

  return nsnull;
}

// forms related stuff

NS_IMETHODIMP
nsHTMLDocument::GetForms(nsIDOMHTMLCollection** aForms)
{
  nsContentList *forms = nsHTMLDocument::GetForms();
  if (!forms)
    return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(*aForms = forms);
  return NS_OK;
}

nsContentList*
nsHTMLDocument::GetForms()
{
  if (!mForms)
    mForms = new nsContentList(this, nsGkAtoms::form, mDefaultNamespaceID);

  return mForms;
}

static PRBool MatchFormControls(nsIContent* aContent, PRInt32 aNamespaceID,
                                nsIAtom* aAtom, void* aData)
{
  return aContent->IsNodeOfType(nsIContent::eHTML_FORM_CONTROL);
}

nsContentList*
nsHTMLDocument::GetFormControls()
{
  if (!mFormControls) {
    mFormControls = new nsContentList(this, MatchFormControls, nsnull, nsnull);
  }

  return mFormControls;
}

nsresult
nsHTMLDocument::CreateAndAddWyciwygChannel(void)
{
  nsresult rv = NS_OK;
  nsCAutoString url, originalSpec;

  mDocumentURI->GetSpec(originalSpec);

  // Generate the wyciwyg url
  url = NS_LITERAL_CSTRING("wyciwyg://")
      + nsPrintfCString("%d", gWyciwygSessionCnt++)
      + NS_LITERAL_CSTRING("/")
      + originalSpec;

  nsCOMPtr<nsIURI> wcwgURI;
  NS_NewURI(getter_AddRefs(wcwgURI), url);

  // Create the nsIWyciwygChannel to store out-of-band
  // document.write() script to cache
  nsCOMPtr<nsIChannel> channel;
  // Create a wyciwyg Channel
  rv = NS_NewChannel(getter_AddRefs(channel), wcwgURI);
  NS_ENSURE_SUCCESS(rv, rv);

  mWyciwygChannel = do_QueryInterface(channel);

  mWyciwygChannel->SetSecurityInfo(mSecurityInfo);

  // Note: we want to treat this like a "previous document" hint so that,
  // e.g. a <meta> tag in the document.write content can override it.
  SetDocumentCharacterSetSource(kCharsetFromHintPrevDoc);
  mWyciwygChannel->SetCharsetAndSource(kCharsetFromHintPrevDoc,
                                       GetDocumentCharacterSet());

  // Use our new principal
  channel->SetOwner(NodePrincipal());

  // Inherit load flags from the original document's channel
  channel->SetLoadFlags(mLoadFlags);

  nsCOMPtr<nsILoadGroup> loadGroup = GetDocumentLoadGroup();

  // Use the Parent document's loadgroup to trigger load notifications
  if (loadGroup && channel) {
    rv = channel->SetLoadGroup(loadGroup);
    NS_ENSURE_SUCCESS(rv, rv);

    nsLoadFlags loadFlags = 0;
    channel->GetLoadFlags(&loadFlags);
    loadFlags |= nsIChannel::LOAD_DOCUMENT_URI;
    channel->SetLoadFlags(loadFlags);

    channel->SetOriginalURI(wcwgURI);

    rv = loadGroup->AddRequest(mWyciwygChannel, nsnull);
    NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to add request to load group.");
  }

  return rv;
}

nsresult
nsHTMLDocument::RemoveWyciwygChannel(void)
{
  nsCOMPtr<nsILoadGroup> loadGroup = GetDocumentLoadGroup();

  // note there can be a write request without a load group if
  // this is a synchronously constructed about:blank document
  if (loadGroup && mWyciwygChannel) {
    mWyciwygChannel->CloseCacheEntry(NS_OK);
    loadGroup->RemoveRequest(mWyciwygChannel, nsnull, NS_OK);
  }

  mWyciwygChannel = nsnull;

  return NS_OK;
}

void *
nsHTMLDocument::GenerateParserKey(void)
{
  if (!mScriptLoader) {
    // If we don't have a script loader, then the parser probably isn't parsing
    // anything anyway, so just return null.
    return nsnull;
  }

  // The script loader provides us with the currently executing script element,
  // which is guaranteed to be unique per script.
  return mScriptLoader->GetCurrentScript();
}

/* attribute DOMString designMode; */
NS_IMETHODIMP
nsHTMLDocument::GetDesignMode(nsAString & aDesignMode)
{
  if (HasFlag(NODE_IS_EDITABLE)) {
    aDesignMode.AssignLiteral("on");
  }
  else {
    aDesignMode.AssignLiteral("off");
  }
  return NS_OK;
}

void
nsHTMLDocument::EndUpdate(nsUpdateType aUpdateType)
{
  nsDocument::EndUpdate(aUpdateType);

  if (mUpdateNestLevel == 0 && mContentEditableCount > 0 != IsEditingOn()) {
    EditingStateChanged();
  }
}

nsresult
nsHTMLDocument::ChangeContentEditableCount(nsIContent *aElement,
                                           PRInt32 aChange)
{
  NS_ASSERTION(mContentEditableCount + aChange >= 0,
               "Trying to decrement too much.");

  mContentEditableCount += aChange;

  if (mParser ||
      (mUpdateNestLevel > 0 && mContentEditableCount > 0 != IsEditingOn())) {
    return NS_OK;
  }

  EditingState oldState = mEditingState;

  nsresult rv = EditingStateChanged();
  NS_ENSURE_SUCCESS(rv, rv);

  if (oldState == mEditingState && mEditingState == eContentEditable) {
    // We just changed the contentEditable state of a node, we need to reset
    // the spellchecking state of that node.
    nsCOMPtr<nsIDOMNode> node = do_QueryInterface(aElement);
    if (node) {
      nsPIDOMWindow *window = GetWindow();
      if (!window)
        return NS_ERROR_FAILURE;

      nsIDocShell *docshell = window->GetDocShell();
      if (!docshell)
        return NS_ERROR_FAILURE;

      nsCOMPtr<nsIEditorDocShell> editorDocShell =
        do_QueryInterface(docshell, &rv);
      NS_ENSURE_SUCCESS(rv, rv);

      nsCOMPtr<nsIEditor> editor;
      editorDocShell->GetEditor(getter_AddRefs(editor));
      if (editor) {
        nsCOMPtr<nsIDOMRange> range;
        rv = NS_NewRange(getter_AddRefs(range));
        NS_ENSURE_SUCCESS(rv, rv);

        rv = range->SelectNode(node);
        NS_ENSURE_SUCCESS(rv, rv);

        nsCOMPtr<nsIInlineSpellChecker> spellChecker;
        rv = editor->GetInlineSpellChecker(PR_FALSE,
                                           getter_AddRefs(spellChecker));
        NS_ENSURE_SUCCESS(rv, rv);

        if (spellChecker) {
          rv = spellChecker->SpellCheckRange(range);
          NS_ENSURE_SUCCESS(rv, rv);
        }
      }
    }
  }

  return NS_OK;
}

static PRBool
DocAllResultMatch(nsIContent* aContent, PRInt32 aNamespaceID, nsIAtom* aAtom,
                  void* aData)
{
  if (aContent->GetID() == aAtom) {
    return PR_TRUE;
  }

  nsGenericHTMLElement* elm = nsGenericHTMLElement::FromContent(aContent);
  if (!elm || aContent->GetNameSpaceID() != kNameSpaceID_None) {
    return PR_FALSE;
  }

  nsIAtom* tag = elm->Tag();
  if (tag != nsGkAtoms::img    &&
      tag != nsGkAtoms::form   &&
      tag != nsGkAtoms::applet &&
      tag != nsGkAtoms::embed  &&
      tag != nsGkAtoms::object &&
      tag != nsGkAtoms::input) {
    return PR_FALSE;
  }

  const nsAttrValue* val = elm->GetParsedAttr(nsGkAtoms::name);
  return val && val->Type() == nsAttrValue::eAtom &&
         val->GetAtomValue() == aAtom;
}


nsresult
nsHTMLDocument::GetDocumentAllResult(const nsAString& aID, nsISupports** aResult)
{
  *aResult = nsnull;

  PLDHashOperator op = IdTableIsLive() ? PL_DHASH_LOOKUP : PL_DHASH_ADD;

  nsCOMPtr<nsIAtom> id = do_GetAtom(aID);
  IdAndNameMapEntry *entry =
    static_cast<IdAndNameMapEntry *>
               (PL_DHashTableOperate(&mIdAndNameHashTable, id, op));
  NS_ENSURE_TRUE(entry, NS_ERROR_OUT_OF_MEMORY);

  // If we did a lookup and it failed, there are no items with this id
  if (PL_DHASH_ENTRY_IS_FREE(entry)) {
    NS_ASSERTION(IdTableIsLive(), "should have gotten a busy entry");

    return NS_OK;
  }

  nsIContent* root = GetRootContent();
  if (!root) {
    return NS_OK;
  }

  if (!entry->mDocAllList) {
    entry->mDocAllList = new nsContentList(root, DocAllResultMatch,
                                           nsnull, nsnull, PR_TRUE, id);
    NS_ENSURE_TRUE(entry->mDocAllList, NS_ERROR_OUT_OF_MEMORY);
  }

  // Check if there are more than 1 entries. Do this by getting the second one
  // rather than the length since getting the length always requires walking
  // the entire document.

  nsIContent* cont = entry->mDocAllList->Item(1, PR_TRUE);
  if (cont) {
    NS_ADDREF(*aResult = static_cast<nsIDOMNodeList*>(entry->mDocAllList));

    return NS_OK;
  }

  // There's only 0 or 1 items. Return the first one or null.
  NS_IF_ADDREF(*aResult = entry->mDocAllList->Item(0, PR_TRUE));

  return NS_OK;
}

static void
NotifyEditableStateChange(nsINode *aNode, nsIDocument *aDocument,
                          PRBool aEditable)
{
  PRUint32 i, n = aNode->GetChildCount();
  for (i = 0; i < n; ++i) {
    nsIContent *child = aNode->GetChildAt(i);
    if (child->HasFlag(NODE_IS_EDITABLE) != aEditable) {
      aDocument->ContentStatesChanged(child, nsnull,
                                      NS_EVENT_STATE_MOZ_READONLY |
                                      NS_EVENT_STATE_MOZ_READWRITE);
    }
    NotifyEditableStateChange(child, aDocument, aEditable);
  }
}

void
nsHTMLDocument::TearingDownEditor(nsIEditor *aEditor)
{
  if (IsEditingOn()) {
    mEditingState = eTearingDown;

    nsCOMPtr<nsIEditorStyleSheets> editorss = do_QueryInterface(aEditor);
    if (editorss) {
      editorss->RemoveOverrideStyleSheet(NS_LITERAL_STRING("resource://gre/res/contenteditable.css"));
      if (mEditingState == eDesignMode)
        editorss->RemoveOverrideStyleSheet(NS_LITERAL_STRING("resource://gre/res/designmode.css"));
    }
  }
}

nsresult
nsHTMLDocument::TurnEditingOff()
{
  NS_ASSERTION(mEditingState != eOff, "Editing is already off.");

  nsPIDOMWindow *window = GetWindow();
  if (!window)
    return NS_ERROR_FAILURE;

  nsIDocShell *docshell = window->GetDocShell();
  if (!docshell)
    return NS_ERROR_FAILURE;

  nsresult rv;
  nsCOMPtr<nsIEditingSession> editSession = do_GetInterface(docshell, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  // turn editing off
  rv = editSession->TearDownEditorOnWindow(window);
  NS_ENSURE_SUCCESS(rv, rv);

  mEditingState = eOff;

  return NS_OK;
}

static PRBool HasPresShell(nsPIDOMWindow *aWindow)
{
  nsIDocShell *docShell = aWindow->GetDocShell();
  if (!docShell)
    return PR_FALSE;
  nsCOMPtr<nsIPresShell> presShell;
  docShell->GetPresShell(getter_AddRefs(presShell));
  return presShell != nsnull;
}

nsresult
nsHTMLDocument::EditingStateChanged()
{
  if (mEditingState == eSettingUp || mEditingState == eTearingDown) {
    // XXX We shouldn't recurse.
    return NS_OK;
  }

  PRBool designMode = HasFlag(NODE_IS_EDITABLE);
  EditingState newState = designMode ? eDesignMode :
                          (mContentEditableCount > 0 ? eContentEditable : eOff);
  if (mEditingState == newState) {
    // No changes in editing mode.
    return NS_OK;
  }

  if (newState == eOff) {
    // Editing is being turned off.
    return TurnEditingOff();
  }

  // get editing session
  nsPIDOMWindow *window = GetWindow();
  if (!window)
    return NS_ERROR_FAILURE;

  nsIDocShell *docshell = window->GetDocShell();
  if (!docshell)
    return NS_ERROR_FAILURE;

  nsresult rv;
  nsCOMPtr<nsIEditingSession> editSession = do_GetInterface(docshell, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!HasPresShell(window)) {
    // We should not make the window editable or setup its editor.
    // It's probably style=display:none.
    return NS_OK;
  }

  PRBool makeWindowEditable = mEditingState == eOff;
  if (makeWindowEditable) {
    // Editing is being turned on (through designMode or contentEditable)
    // Turn on editor.
    // XXX This can cause flushing which can change the editing state, so make
    //     sure to avoid recursing.
    EditingState oldState = mEditingState;
    mEditingState = eSettingUp;

    rv = editSession->MakeWindowEditable(window, "html", PR_FALSE, PR_FALSE,
                                         PR_TRUE);
    NS_ENSURE_SUCCESS(rv, rv);

    mEditingState = oldState;
  }

  // XXX Need to call TearDownEditorOnWindow for all failures.
  nsCOMPtr<nsIEditorDocShell> editorDocShell =
    do_QueryInterface(docshell, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIEditor> editor;
  editorDocShell->GetEditor(getter_AddRefs(editor));
  if (!editor)
    return NS_ERROR_FAILURE;

  nsCOMPtr<nsIEditorStyleSheets> editorss = do_QueryInterface(editor, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  editorss->AddOverrideStyleSheet(NS_LITERAL_STRING("resource://gre/res/contenteditable.css"));

  // Should we update the editable state of all the nodes in the document? We
  // need to do this when the designMode value changes, as that overrides
  // specific states on the elements.
  PRBool updateState;

  PRBool spellRecheckAll = PR_FALSE;
  if (designMode) {
    // designMode is being turned on (overrides contentEditable).
    editorss->AddOverrideStyleSheet(NS_LITERAL_STRING("resource://gre/res/designmode.css"));

    // Disable scripting and plugins.
    rv = editSession->DisableJSAndPlugins(window);
    NS_ENSURE_SUCCESS(rv, rv);

    updateState = PR_TRUE;
    spellRecheckAll = mEditingState == eContentEditable;
  }
  else if (mEditingState == eDesignMode) {
    // designMode is being turned off (contentEditable is still on).
    editorss->RemoveOverrideStyleSheet(NS_LITERAL_STRING("resource://gre/res/designmode.css"));

    rv = editSession->RestoreJSAndPlugins(window);
    NS_ENSURE_SUCCESS(rv, rv);

    updateState = PR_TRUE;
  }
  else {
    // contentEditable is being turned on (and designMode is off).
    updateState = PR_FALSE;
  }

  mEditingState = newState;

  if (makeWindowEditable) {
    // Set the editor to not insert br's on return when in p
    // elements by default.
    // XXX Do we only want to do this for designMode?
    PRBool unused;
    rv = ExecCommand(NS_LITERAL_STRING("insertBrOnReturn"), PR_FALSE,
                     NS_LITERAL_STRING("false"), &unused);

    if (NS_FAILED(rv)) {
      // Editor setup failed. Editing is not on after all.
      // XXX Should we reset the editable flag on nodes?
      editSession->TearDownEditorOnWindow(window);
      mEditingState = eOff;

      return rv;
    }
  }

  if (updateState) {
    mozAutoDocUpdate upd(this, UPDATE_CONTENT_STATE, PR_TRUE);
    NotifyEditableStateChange(this, this, !designMode);
  }

  // Resync the editor's spellcheck state.
  if (spellRecheckAll) {
    nsCOMPtr<nsISelectionController> selcon;
    nsresult rv = editor->GetSelectionController(getter_AddRefs(selcon));
    NS_ENSURE_SUCCESS(rv, rv); 

    nsCOMPtr<nsISelection> spellCheckSelection;
    rv = selcon->GetSelection(nsISelectionController::SELECTION_SPELLCHECK,
                              getter_AddRefs(spellCheckSelection));
    if (NS_SUCCEEDED(rv)) {
      spellCheckSelection->RemoveAllRanges();
    }
  }
  editor->SyncRealTimeSpell();

  return NS_OK;
}

NS_IMETHODIMP
nsHTMLDocument::SetDesignMode(const nsAString & aDesignMode)
{
  nsresult rv = NS_OK;

  if (!nsContentUtils::IsCallerTrustedForWrite()) {
    nsCOMPtr<nsIPrincipal> subject;
    nsIScriptSecurityManager *secMan = nsContentUtils::GetSecurityManager();
    rv = secMan->GetSubjectPrincipal(getter_AddRefs(subject));
    NS_ENSURE_SUCCESS(rv, rv);
    if (subject) {
      PRBool subsumes;
      rv = subject->Subsumes(NodePrincipal(), &subsumes);
      NS_ENSURE_SUCCESS(rv, rv);

      NS_ENSURE_TRUE(subsumes, NS_ERROR_DOM_PROP_ACCESS_DENIED);
    }
  }

  PRBool editableMode = HasFlag(NODE_IS_EDITABLE);
  if (aDesignMode.LowerCaseEqualsASCII(editableMode ? "off" : "on")) {
    SetEditableFlag(!editableMode);

    return EditingStateChanged();
  }

  return NS_OK;
}

nsresult
nsHTMLDocument::GetMidasCommandManager(nsICommandManager** aCmdMgr)
{
  // initialize return value
  NS_ENSURE_ARG_POINTER(aCmdMgr);

  // check if we have it cached
  if (mMidasCommandManager) {
    NS_ADDREF(*aCmdMgr = mMidasCommandManager);
    return NS_OK;
  }

  *aCmdMgr = nsnull;

  nsPIDOMWindow *window = GetWindow();
  if (!window)
    return NS_ERROR_FAILURE;

  nsIDocShell *docshell = window->GetDocShell();
  if (!docshell)
    return NS_ERROR_FAILURE;

  mMidasCommandManager = do_GetInterface(docshell);
  if (!mMidasCommandManager)
    return NS_ERROR_FAILURE;

  NS_ADDREF(*aCmdMgr = mMidasCommandManager);

  return NS_OK;
}


struct MidasCommand {
  const char*  incomingCommandString;
  const char*  internalCommandString;
  const char*  internalParamString;
  PRPackedBool useNewParam;
  PRPackedBool convertToBoolean;
};

static const struct MidasCommand gMidasCommandTable[] = {
  { "bold",          "cmd_bold",            "", PR_TRUE,  PR_FALSE },
  { "italic",        "cmd_italic",          "", PR_TRUE,  PR_FALSE },
  { "underline",     "cmd_underline",       "", PR_TRUE,  PR_FALSE },
  { "strikethrough", "cmd_strikethrough",   "", PR_TRUE,  PR_FALSE },
  { "subscript",     "cmd_subscript",       "", PR_TRUE,  PR_FALSE },
  { "superscript",   "cmd_superscript",     "", PR_TRUE,  PR_FALSE },
  { "cut",           "cmd_cut",             "", PR_TRUE,  PR_FALSE },
  { "copy",          "cmd_copy",            "", PR_TRUE,  PR_FALSE },
  { "paste",         "cmd_paste",           "", PR_TRUE,  PR_FALSE },
  { "delete",        "cmd_delete",          "", PR_TRUE,  PR_FALSE },
  { "selectall",     "cmd_selectAll",       "", PR_TRUE,  PR_FALSE },
  { "undo",          "cmd_undo",            "", PR_TRUE,  PR_FALSE },
  { "redo",          "cmd_redo",            "", PR_TRUE,  PR_FALSE },
  { "indent",        "cmd_indent",          "", PR_TRUE,  PR_FALSE },
  { "outdent",       "cmd_outdent",         "", PR_TRUE,  PR_FALSE },
  { "backcolor",     "cmd_backgroundColor", "", PR_FALSE, PR_FALSE },
  { "forecolor",     "cmd_fontColor",       "", PR_FALSE, PR_FALSE },
  { "hilitecolor",   "cmd_highlight",       "", PR_FALSE, PR_FALSE },
  { "fontname",      "cmd_fontFace",        "", PR_FALSE, PR_FALSE },
  { "fontsize",      "cmd_fontSize",        "", PR_FALSE, PR_FALSE },
  { "increasefontsize", "cmd_increaseFont", "", PR_FALSE, PR_FALSE },
  { "decreasefontsize", "cmd_decreaseFont", "", PR_FALSE, PR_FALSE },
  { "inserthorizontalrule", "cmd_insertHR", "", PR_TRUE,  PR_FALSE },
  { "createlink",    "cmd_insertLinkNoUI",  "", PR_FALSE, PR_FALSE },
  { "insertimage",   "cmd_insertImageNoUI", "", PR_FALSE, PR_FALSE },
  { "inserthtml",    "cmd_insertHTML",      "", PR_FALSE, PR_FALSE },
  { "gethtml",       "cmd_getContents",     "", PR_FALSE, PR_FALSE },
  { "justifyleft",   "cmd_align",       "left", PR_TRUE,  PR_FALSE },
  { "justifyright",  "cmd_align",      "right", PR_TRUE,  PR_FALSE },
  { "justifycenter", "cmd_align",     "center", PR_TRUE,  PR_FALSE },
  { "justifyfull",   "cmd_align",    "justify", PR_TRUE,  PR_FALSE },
  { "removeformat",  "cmd_removeStyles",    "", PR_TRUE,  PR_FALSE },
  { "unlink",        "cmd_removeLinks",     "", PR_TRUE,  PR_FALSE },
  { "insertorderedlist",   "cmd_ol",        "", PR_TRUE,  PR_FALSE },
  { "insertunorderedlist", "cmd_ul",        "", PR_TRUE,  PR_FALSE },
  { "insertparagraph", "cmd_paragraphState", "p", PR_TRUE, PR_FALSE },
  { "formatblock",   "cmd_paragraphState",  "", PR_FALSE, PR_FALSE },
  { "heading",       "cmd_paragraphState",  "", PR_FALSE, PR_FALSE },
  { "styleWithCSS",  "cmd_setDocumentUseCSS", "", PR_FALSE, PR_TRUE },
  { "contentReadOnly", "cmd_setDocumentReadOnly", "", PR_FALSE, PR_TRUE },
  { "insertBrOnReturn", "cmd_insertBrOnReturn", "", PR_FALSE, PR_TRUE },
  { "enableObjectResizing", "cmd_enableObjectResizing", "", PR_FALSE, PR_TRUE },
  { "enableInlineTableEditing", "cmd_enableInlineTableEditing", "", PR_FALSE, PR_TRUE },
#if 0
// no editor support to remove alignments right now
  { "justifynone",   "cmd_align",           "", PR_TRUE,  PR_FALSE },

// the following will need special review before being turned on
  { "saveas",        "cmd_saveAs",          "", PR_TRUE,  PR_FALSE },
  { "print",         "cmd_print",           "", PR_TRUE,  PR_FALSE },
#endif
  { NULL, NULL, NULL, PR_FALSE, PR_FALSE }
};

#define MidasCommandCount ((sizeof(gMidasCommandTable) / sizeof(struct MidasCommand)) - 1)

static const char* const gBlocks[] = {
  "ADDRESS",
  "BLOCKQUOTE",
  "DD",
  "DIV",
  "DL",
  "DT",
  "H1",
  "H2",
  "H3",
  "H4",
  "H5",
  "H6",
  "P",
  "PRE"
};

static PRBool
ConvertToMidasInternalCommandInner(const nsAString & inCommandID,
                                   const nsAString & inParam,
                                   nsACString& outCommandID,
                                   nsACString& outParam,
                                   PRBool& outIsBoolean,
                                   PRBool& outBooleanValue,
                                   PRBool aIgnoreParams)
{
  NS_ConvertUTF16toUTF8 convertedCommandID(inCommandID);

  // Hack to support old boolean commands that were backwards (see bug 301490).
  PRBool invertBool = PR_FALSE;
  if (convertedCommandID.LowerCaseEqualsLiteral("usecss")) {
    convertedCommandID.Assign("styleWithCSS");
    invertBool = PR_TRUE;
  }
  else if (convertedCommandID.LowerCaseEqualsLiteral("readonly")) {
    convertedCommandID.Assign("contentReadOnly");
    invertBool = PR_TRUE;
  }

  PRUint32 i;
  PRBool found = PR_FALSE;
  for (i = 0; i < MidasCommandCount; ++i) {
    if (convertedCommandID.Equals(gMidasCommandTable[i].incomingCommandString,
                                  nsCaseInsensitiveCStringComparator())) {
      found = PR_TRUE;
      break;
    }
  }

  if (found) {
    // set outCommandID (what we use internally)
    outCommandID.Assign(gMidasCommandTable[i].internalCommandString);

    // set outParam & outIsBoolean based on flags from the table
    outIsBoolean = gMidasCommandTable[i].convertToBoolean;

    if (!aIgnoreParams) {
      if (gMidasCommandTable[i].useNewParam) {
        outParam.Assign(gMidasCommandTable[i].internalParamString);
      }
      else {
        // handle checking of param passed in
        if (outIsBoolean) {
          // if this is a boolean value and it's not explicitly false
          // (e.g. no value) we default to "true". For old backwards commands
          // we invert the check (see bug 301490).
          if (invertBool) {
            outBooleanValue = inParam.LowerCaseEqualsLiteral("false");
          }
          else {
            outBooleanValue = !inParam.LowerCaseEqualsLiteral("false");
          }
          outParam.Truncate();
        }
        else {
          // check to see if we need to convert the parameter
          if (outCommandID.EqualsLiteral("cmd_paragraphState")) {
            const PRUnichar *start = inParam.BeginReading();
            const PRUnichar *end = inParam.EndReading();
            if (start != end && *start == '<' && *(end - 1) == '>') {
              ++start;
              --end;
            }

            NS_ConvertUTF16toUTF8 convertedParam(Substring(start, end));
            PRUint32 j;
            for (j = 0; j < NS_ARRAY_LENGTH(gBlocks); ++j) {
              if (convertedParam.Equals(gBlocks[j],
                                        nsCaseInsensitiveCStringComparator())) {
                outParam.Assign(gBlocks[j]);
                break;
              }
            }

            return j != NS_ARRAY_LENGTH(gBlocks);
          }
          else {
            CopyUTF16toUTF8(inParam, outParam);
          }
        }
      }
    }
  } // end else for useNewParam (do convert existing param)
  else {
    // reset results if the command is not found in our table
    outCommandID.SetLength(0);
    outParam.SetLength(0);
    outIsBoolean = PR_FALSE;
  }

  return found;
}

static PRBool
ConvertToMidasInternalCommand(const nsAString & inCommandID,
                              const nsAString & inParam,
                              nsACString& outCommandID,
                              nsACString& outParam,
                              PRBool& outIsBoolean,
                              PRBool& outBooleanValue)
{
  return ConvertToMidasInternalCommandInner(inCommandID, inParam, outCommandID,
                                            outParam, outIsBoolean,
                                            outBooleanValue, PR_FALSE);
}

static PRBool
ConvertToMidasInternalCommand(const nsAString & inCommandID,
                              nsACString& outCommandID)
{
  nsCAutoString dummyCString;
  nsAutoString dummyString;
  PRBool dummyBool;
  return ConvertToMidasInternalCommandInner(inCommandID, dummyString,
                                            outCommandID, dummyCString,
                                            dummyBool, dummyBool, PR_TRUE);
}

jsval
nsHTMLDocument::sCutCopyInternal_id = JSVAL_VOID;
jsval
nsHTMLDocument::sPasteInternal_id = JSVAL_VOID;

/* Helper function to check security of clipboard commands. If aPaste is */
/* true, we check paste, else we check cutcopy */
nsresult
nsHTMLDocument::DoClipboardSecurityCheck(PRBool aPaste)
{
  nsresult rv = NS_ERROR_FAILURE;

  nsCOMPtr<nsIJSContextStack> stack =
    do_GetService("@mozilla.org/js/xpc/ContextStack;1");

  if (stack) {
    JSContext *cx = nsnull;
    stack->Peek(&cx);
    if (!cx) {
      return NS_OK;
    }

    JSAutoRequest ar(cx);

    NS_NAMED_LITERAL_CSTRING(classNameStr, "Clipboard");

    nsIScriptSecurityManager *secMan = nsContentUtils::GetSecurityManager();

    if (aPaste) {
      if (nsHTMLDocument::sPasteInternal_id == JSVAL_VOID) {
        nsHTMLDocument::sPasteInternal_id =
          STRING_TO_JSVAL(::JS_InternString(cx, "paste"));
      }
      rv = secMan->CheckPropertyAccess(cx, nsnull, classNameStr.get(),
                                       nsHTMLDocument::sPasteInternal_id,
                                       nsIXPCSecurityManager::ACCESS_GET_PROPERTY);
    } else {
      if (nsHTMLDocument::sCutCopyInternal_id == JSVAL_VOID) {
        nsHTMLDocument::sCutCopyInternal_id =
          STRING_TO_JSVAL(::JS_InternString(cx, "cutcopy"));
      }
      rv = secMan->CheckPropertyAccess(cx, nsnull, classNameStr.get(),
                                       nsHTMLDocument::sCutCopyInternal_id,
                                       nsIXPCSecurityManager::ACCESS_GET_PROPERTY);
    }
  }
  return rv;
}

/* TODO: don't let this call do anything if the page is not done loading */
/* boolean execCommand(in DOMString commandID, in boolean doShowUI,
                                               in DOMString value); */
NS_IMETHODIMP
nsHTMLDocument::ExecCommand(const nsAString & commandID,
                            PRBool doShowUI,
                            const nsAString & value,
                            PRBool *_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);

  //  for optional parameters see dom/src/base/nsHistory.cpp: HistoryImpl::Go()
  //  this might add some ugly JS dependencies?

  *_retval = PR_FALSE;

  // if editing is not on, bail
  if (!IsEditingOn())
    return NS_ERROR_FAILURE;

  // if they are requesting UI from us, let's fail since we have no UI
  if (doShowUI)
    return NS_OK;

  nsresult rv = NS_OK;

  if (commandID.LowerCaseEqualsLiteral("gethtml"))
    return NS_ERROR_FAILURE;

  if (commandID.LowerCaseEqualsLiteral("cut") ||
      (commandID.LowerCaseEqualsLiteral("copy"))) {
    rv = DoClipboardSecurityCheck(PR_FALSE);
  } else if (commandID.LowerCaseEqualsLiteral("paste")) {
    rv = DoClipboardSecurityCheck(PR_TRUE);
  }

  if (NS_FAILED(rv))
    return rv;

  // get command manager and dispatch command to our window if it's acceptable
  nsCOMPtr<nsICommandManager> cmdMgr;
  GetMidasCommandManager(getter_AddRefs(cmdMgr));
  if (!cmdMgr)
    return NS_ERROR_FAILURE;

  nsIDOMWindow *window = GetWindow();
  if (!window)
    return NS_ERROR_FAILURE;

  nsCAutoString cmdToDispatch, paramStr;
  PRBool isBool, boolVal;
  if (!ConvertToMidasInternalCommand(commandID, value,
                                     cmdToDispatch, paramStr, isBool, boolVal))
    return NS_OK;

  if (!isBool && paramStr.IsEmpty()) {
    rv = cmdMgr->DoCommand(cmdToDispatch.get(), nsnull, window);
  } else {
    // we have a command that requires a parameter, create params
    nsCOMPtr<nsICommandParams> cmdParams = do_CreateInstance(
                                            NS_COMMAND_PARAMS_CONTRACTID, &rv);
    if (!cmdParams)
      return NS_ERROR_OUT_OF_MEMORY;

    if (isBool)
      rv = cmdParams->SetBooleanValue("state_attribute", boolVal);
    else if (cmdToDispatch.Equals("cmd_fontFace"))
      rv = cmdParams->SetStringValue("state_attribute", value);
    else if (cmdToDispatch.Equals("cmd_insertHTML"))
      rv = cmdParams->SetStringValue("state_data", value);
    else
      rv = cmdParams->SetCStringValue("state_attribute", paramStr.get());
    if (NS_FAILED(rv))
      return rv;
    rv = cmdMgr->DoCommand(cmdToDispatch.get(), cmdParams, window);
  }

  *_retval = NS_SUCCEEDED(rv);

  return rv;
}

/* TODO: don't let this call do anything if the page is not done loading */
/* boolean execCommandShowHelp(in DOMString commandID); */
NS_IMETHODIMP
nsHTMLDocument::ExecCommandShowHelp(const nsAString & commandID,
                                    PRBool *_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = PR_FALSE;

  // if editing is not on, bail
  if (!IsEditingOn())
    return NS_ERROR_FAILURE;

  return NS_ERROR_NOT_IMPLEMENTED;
}

/* boolean queryCommandEnabled(in DOMString commandID); */
NS_IMETHODIMP
nsHTMLDocument::QueryCommandEnabled(const nsAString & commandID,
                                    PRBool *_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = PR_FALSE;

  // if editing is not on, bail
  if (!IsEditingOn())
    return NS_ERROR_FAILURE;

  // get command manager and dispatch command to our window if it's acceptable
  nsCOMPtr<nsICommandManager> cmdMgr;
  GetMidasCommandManager(getter_AddRefs(cmdMgr));
  if (!cmdMgr)
    return NS_ERROR_FAILURE;

  nsIDOMWindow *window = GetWindow();
  if (!window)
    return NS_ERROR_FAILURE;

  nsCAutoString cmdToDispatch, paramStr;
  if (!ConvertToMidasInternalCommand(commandID, cmdToDispatch))
    return NS_ERROR_NOT_IMPLEMENTED;

  return cmdMgr->IsCommandEnabled(cmdToDispatch.get(), window, _retval);
}

/* boolean queryCommandIndeterm (in DOMString commandID); */
NS_IMETHODIMP
nsHTMLDocument::QueryCommandIndeterm(const nsAString & commandID,
                                     PRBool *_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = PR_FALSE;

  // if editing is not on, bail
  if (!IsEditingOn())
    return NS_ERROR_FAILURE;

  // get command manager and dispatch command to our window if it's acceptable
  nsCOMPtr<nsICommandManager> cmdMgr;
  GetMidasCommandManager(getter_AddRefs(cmdMgr));
  if (!cmdMgr)
    return NS_ERROR_FAILURE;

  nsIDOMWindow *window = GetWindow();
  if (!window)
    return NS_ERROR_FAILURE;

  nsCAutoString cmdToDispatch, paramToCheck;
  PRBool dummy;
  if (!ConvertToMidasInternalCommand(commandID, commandID,
                                     cmdToDispatch, paramToCheck, dummy, dummy))
    return NS_ERROR_NOT_IMPLEMENTED;

  nsresult rv;
  nsCOMPtr<nsICommandParams> cmdParams = do_CreateInstance(
                                           NS_COMMAND_PARAMS_CONTRACTID, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = cmdMgr->GetCommandState(cmdToDispatch.get(), window, cmdParams);
  if (NS_FAILED(rv))
    return rv;

  // if command does not have a state_mixed value, this call fails, so we fail too,
  // which is what is expected
  rv = cmdParams->GetBooleanValue("state_mixed", _retval);
  return rv;
}

/* boolean queryCommandState(in DOMString commandID); */
NS_IMETHODIMP
nsHTMLDocument::QueryCommandState(const nsAString & commandID, PRBool *_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = PR_FALSE;

  // if editing is not on, bail
  if (!IsEditingOn())
    return NS_ERROR_FAILURE;

  // get command manager and dispatch command to our window if it's acceptable
  nsCOMPtr<nsICommandManager> cmdMgr;
  GetMidasCommandManager(getter_AddRefs(cmdMgr));
  if (!cmdMgr)
    return NS_ERROR_FAILURE;

  nsIDOMWindow *window = GetWindow();
  if (!window)
    return NS_ERROR_FAILURE;

  nsCAutoString cmdToDispatch, paramToCheck;
  PRBool dummy, dummy2;
  if (!ConvertToMidasInternalCommand(commandID, commandID,
                                     cmdToDispatch, paramToCheck, dummy, dummy2))
    return NS_ERROR_NOT_IMPLEMENTED;

  nsresult rv;
  nsCOMPtr<nsICommandParams> cmdParams = do_CreateInstance(
                                           NS_COMMAND_PARAMS_CONTRACTID, &rv);
  if (!cmdParams)
    return NS_ERROR_OUT_OF_MEMORY;

  rv = cmdMgr->GetCommandState(cmdToDispatch.get(), window, cmdParams);
  if (NS_FAILED(rv))
    return rv;

  // handle alignment as a special case (possibly other commands too?)
  // Alignment is special because the external api is individual
  // commands but internally we use cmd_align with different
  // parameters.  When getting the state of this command, we need to
  // return the boolean for this particular alignment rather than the
  // string of 'which alignment is this?'
  if (cmdToDispatch.Equals("cmd_align")) {
    char * actualAlignmentType = nsnull;
    rv = cmdParams->GetCStringValue("state_attribute", &actualAlignmentType);
    if (NS_SUCCEEDED(rv) && actualAlignmentType && actualAlignmentType[0]) {
      *_retval = paramToCheck.Equals(actualAlignmentType);
    }
    if (actualAlignmentType)
      nsMemory::Free(actualAlignmentType);
  }
  else {
    rv = cmdParams->GetBooleanValue("state_all", _retval);
    if (NS_FAILED(rv))
      *_retval = PR_FALSE;
  }

  return rv;
}

/* boolean queryCommandSupported(in DOMString commandID); */
NS_IMETHODIMP
nsHTMLDocument::QueryCommandSupported(const nsAString & commandID,
                                      PRBool *_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = PR_FALSE;

  // if editing is not on, bail
  if (!IsEditingOn())
    return NS_ERROR_FAILURE;

  return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString queryCommandText(in DOMString commandID); */
NS_IMETHODIMP
nsHTMLDocument::QueryCommandText(const nsAString & commandID,
                                 nsAString & _retval)
{
  _retval.SetLength(0);

  // if editing is not on, bail
  if (!IsEditingOn())
    return NS_ERROR_FAILURE;

  return NS_ERROR_NOT_IMPLEMENTED;
}

/* DOMString queryCommandValue(in DOMString commandID); */
NS_IMETHODIMP
nsHTMLDocument::QueryCommandValue(const nsAString & commandID,
                                  nsAString &_retval)
{
  _retval.SetLength(0);

  // if editing is not on, bail
  if (!IsEditingOn())
    return NS_ERROR_FAILURE;

  // get command manager and dispatch command to our window if it's acceptable
  nsCOMPtr<nsICommandManager> cmdMgr;
  GetMidasCommandManager(getter_AddRefs(cmdMgr));
  if (!cmdMgr)
    return NS_ERROR_FAILURE;

  nsIDOMWindow *window = GetWindow();
  if (!window)
    return NS_ERROR_FAILURE;

  nsCAutoString cmdToDispatch, paramStr;
  if (!ConvertToMidasInternalCommand(commandID, cmdToDispatch))
    return NS_ERROR_NOT_IMPLEMENTED;

  // create params
  nsresult rv;
  nsCOMPtr<nsICommandParams> cmdParams = do_CreateInstance(
                                           NS_COMMAND_PARAMS_CONTRACTID, &rv);
  if (!cmdParams)
    return NS_ERROR_OUT_OF_MEMORY;

  // this is a special command since we are calling "DoCommand rather than
  // GetCommandState like the other commands
  if (cmdToDispatch.Equals("cmd_getContents"))
  {
    rv = cmdParams->SetBooleanValue("selection_only", PR_TRUE);
    if (NS_FAILED(rv)) return rv;
    rv = cmdParams->SetCStringValue("format", "text/html");
    if (NS_FAILED(rv)) return rv;
    rv = cmdMgr->DoCommand(cmdToDispatch.get(), cmdParams, window);
    if (NS_FAILED(rv)) return rv;
    return cmdParams->GetStringValue("result", _retval);
  }

  rv = cmdParams->SetCStringValue("state_attribute", paramStr.get());
  if (NS_FAILED(rv))
    return rv;

  rv = cmdMgr->GetCommandState(cmdToDispatch.get(), window, cmdParams);
  if (NS_FAILED(rv))
    return rv;

  nsXPIDLCString cStringResult;
  rv = cmdParams->GetCStringValue("state_attribute",
                                  getter_Copies(cStringResult));
  CopyUTF8toUTF16(cStringResult, _retval);

  return rv;
}

#ifdef DEBUG
nsresult
nsHTMLDocument::CreateElem(nsIAtom *aName, nsIAtom *aPrefix,
                           PRInt32 aNamespaceID, PRBool aDocumentDefaultType,
                           nsIContent** aResult)
{
  NS_ASSERTION(!aDocumentDefaultType || IsXHTML() ||
               aNamespaceID == kNameSpaceID_None,
               "HTML elements in an HTML document should have "
               "kNamespaceID_None as their namespace ID.");

  if (IsXHTML() &&
      (aDocumentDefaultType || aNamespaceID == kNameSpaceID_XHTML)) {
    nsCAutoString name, lcName;
    aName->ToUTF8String(name);
//    ToLowerCase(name, lcName);
//    NS_ASSERTION(lcName.Equals(name),
//                 "aName should be lowercase, fix caller.");
  }

  return nsDocument::CreateElem(aName, aPrefix, aNamespaceID,
                                aDocumentDefaultType, aResult);
}
#endif
