/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set tw=80 expandtab softtabstop=2 ts=2 sw=2: */
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
 *   Mats Palmgren <mats.palmgren@bredband.net>
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
#include "nscore.h"
#include "nsGenericHTMLElement.h"
#include "nsCOMPtr.h"
#include "nsIAtom.h"
#include "nsINodeInfo.h"
#include "nsIContentViewer.h"
#include "nsICSSParser.h"
#include "nsICSSLoader.h"
#include "nsICSSStyleRule.h"
#include "nsCSSStruct.h"
#include "nsCSSDeclaration.h"
#include "nsIDocument.h"
#include "nsIDocumentEncoder.h"
#include "nsIDOMHTMLDocument.h"
#include "nsIDOMAttr.h"
#include "nsIDOMEventReceiver.h"
#include "nsIDOMNamedNodeMap.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMDocumentFragment.h"
#include "nsIDOMNSHTMLElement.h"
#include "nsIDOMElementCSSInlineStyle.h"
#include "nsIDOMWindow.h"
#include "nsIDOMDocument.h"
#include "nsIEventListenerManager.h"
#include "nsIFocusController.h"
#include "nsMappedAttributes.h"
#include "nsHTMLStyleSheet.h"
#include "nsIHTMLDocument.h"
#include "nsILink.h"
#include "nsILinkHandler.h"
#include "nsPIDOMWindow.h"
#include "nsIStyleRule.h"
#include "nsISupportsArray.h"
#include "nsIURL.h"
#include "nsNetUtil.h"
#include "nsEscape.h"
#include "nsStyleConsts.h"
#include "nsIFrame.h"
#include "nsIScrollableFrame.h"
#include "nsIScrollableView.h"
#include "nsIScrollableViewProvider.h"
#include "nsRange.h"
#include "nsIPresShell.h"
#include "nsPresContext.h"
#include "nsIDocShell.h"
#include "nsIView.h"
#include "nsIViewManager.h"
#include "nsINameSpaceManager.h"
#include "nsDOMError.h"
#include "nsIScriptLoader.h"
#include "nsRuleData.h"

#include "nsPresState.h"
#include "nsILayoutHistoryState.h"

#include "nsHTMLParts.h"
#include "nsContentUtils.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsLayoutAtoms.h"
#include "nsHTMLAtoms.h"
#include "nsIEventStateManager.h"
#include "nsIDOMEvent.h"
#include "nsIDOMNSEvent.h"
#include "nsIPrivateDOMEvent.h"
#include "nsDOMCID.h"
#include "nsIServiceManager.h"
#include "nsDOMCSSDeclaration.h"
#include "nsICSSOMFactory.h"
#include "prprf.h"
#include "prmem.h"
#include "nsITextControlFrame.h"
#include "nsIForm.h"
#include "nsIFormControl.h"
#include "nsIDOMHTMLFormElement.h"
#include "nsILanguageAtomService.h"

#include "nsIDOMMutationEvent.h"
#include "nsMutationEvent.h"

#include "nsIBindingManager.h"
#include "nsXBLBinding.h"

#include "nsRuleWalker.h"

#include "nsIObjectFrame.h"
#include "nsLayoutAtoms.h"
#include "xptinfo.h"
#include "nsIInterfaceInfoManager.h"
#include "nsIServiceManager.h"

#include "nsIParser.h"
#include "nsParserCIID.h"
#include "nsIHTMLContentSink.h"
#include "nsLayoutCID.h"
#include "nsContentCID.h"

#include "nsIDOMText.h"
#include "nsITextContent.h"
#include "nsCOMArray.h"
#include "nsNodeInfoManager.h"

#include "nsIEditor.h"
#include "nsIEditorIMESupport.h"
#include "nsEventDispatcher.h"
#include "nsLayoutUtils.h"

// XXX todo: add in missing out-of-memory checks

//----------------------------------------------------------------------

#ifdef GATHER_ELEMENT_USEAGE_STATISTICS

// static objects that have constructors are kinda bad, but we don't
// care here, this is only debugging code!

static nsHashtable sGEUS_ElementCounts;

void GEUS_ElementCreated(nsINodeInfo *aNodeInfo)
{
  nsAutoString name;
  aNodeInfo->GetLocalName(name);

  nsStringKey key(name);

  PRInt32 count = (PRInt32)sGEUS_ElementCounts.Get(&key);

  count++;

  sGEUS_ElementCounts.Put(&key, (void *)count);
}

PRBool GEUS_enum_func(nsHashKey *aKey, void *aData, void *aClosure)
{
  const PRUnichar *name_chars = ((nsStringKey *)aKey)->GetString();
  NS_ConvertUTF16toUTF8 name(name_chars);

  printf ("%s %d\n", name.get(), aData);

  return PR_TRUE;
}

void GEUS_DumpElementCounts()
{
  printf ("Element count statistics:\n");

  sGEUS_ElementCounts.Enumerate(GEUS_enum_func, nsnull);

  printf ("End of element count statistics:\n");
}

nsresult
nsGenericHTMLElement::Init(nsINodeInfo *aNodeInfo)
{
  GEUS_ElementCreated(aNodeInfo);

  return nsGenericElement::Init(aNodeInfo);
}

#endif


class nsGenericHTMLElementTearoff : public nsIDOMNSHTMLElement,
                                    public nsIDOMElementCSSInlineStyle
{
  NS_DECL_ISUPPORTS

  nsGenericHTMLElementTearoff(nsGenericHTMLElement *aElement)
    : mElement(aElement)
  {
    NS_ADDREF(mElement);
  }

  virtual ~nsGenericHTMLElementTearoff()
  {
    NS_RELEASE(mElement);
  }

  NS_FORWARD_NSIDOMNSHTMLELEMENT(mElement->)
  NS_FORWARD_NSIDOMELEMENTCSSINLINESTYLE(mElement->)

private:
  nsGenericHTMLElement *mElement;
};


NS_IMPL_ADDREF(nsGenericHTMLElementTearoff)
NS_IMPL_RELEASE(nsGenericHTMLElementTearoff)

NS_INTERFACE_MAP_BEGIN(nsGenericHTMLElementTearoff)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNSHTMLElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMElementCSSInlineStyle)
NS_INTERFACE_MAP_END_AGGREGATED(mElement)


static nsICSSOMFactory* gCSSOMFactory = nsnull;
static NS_DEFINE_CID(kCSSOMFactoryCID, NS_CSSOMFACTORY_CID);

NS_IMPL_INT_ATTR(nsGenericHTMLElement, TabIndex, tabindex)

nsresult
nsGenericHTMLElement::DOMQueryInterface(nsIDOMHTMLElement *aElement,
                                        REFNSIID aIID, void **aInstancePtr)
{
  nsISupports *inst = nsnull;

  if (aIID.Equals(NS_GET_IID(nsIDOMNode))) {
    inst = NS_STATIC_CAST(nsIDOMNode *, aElement);
  } else if (aIID.Equals(NS_GET_IID(nsIDOMElement))) {
    inst = NS_STATIC_CAST(nsIDOMElement *, aElement);
  } else if (aIID.Equals(NS_GET_IID(nsIDOMHTMLElement))) {
    inst = NS_STATIC_CAST(nsIDOMHTMLElement *, aElement);
  } else if (aIID.Equals(NS_GET_IID(nsIDOMNSHTMLElement))) {
    inst = NS_STATIC_CAST(nsIDOMNSHTMLElement *,
                          new nsGenericHTMLElementTearoff(this));
    NS_ENSURE_TRUE(inst, NS_ERROR_OUT_OF_MEMORY);
  } else if (aIID.Equals(NS_GET_IID(nsIDOMElementCSSInlineStyle))) {
    inst = NS_STATIC_CAST(nsIDOMElementCSSInlineStyle *,
                          new nsGenericHTMLElementTearoff(this));
    NS_ENSURE_TRUE(inst, NS_ERROR_OUT_OF_MEMORY);
  } else {
    return NS_NOINTERFACE;
  }

  NS_ADDREF(inst);

  *aInstancePtr = inst;

  return NS_OK;
}

/* static */ void
nsGenericHTMLElement::Shutdown()
{
  NS_IF_RELEASE(gCSSOMFactory);
}

nsresult
nsGenericHTMLElement::CopyInnerTo(nsGenericElement* aDst, PRBool aDeep) const
{
  nsresult rv = NS_OK;
  PRInt32 i, count = GetAttrCount();
  nsAutoString value;

  for (i = 0; i < count; ++i) {
    // XXX Once we have access to existing nsDOMAttributes for this element, we
    //     should call CloneNode or ImportNode on them.
    const nsAttrName *name = mAttrsAndChildren.AttrNameAt(i);
    const nsAttrValue *value = mAttrsAndChildren.AttrAt(i);
    if (name->Equals(nsHTMLAtoms::style, kNameSpaceID_None) &&
        value->Type() == nsAttrValue::eCSSStyleRule) {
      // We can't just set this as a string, because that will fail
      // to reparse the string into style data until the node is
      // inserted into the document.  Clone the HTMLValue instead.
      nsCOMPtr<nsICSSRule> ruleClone;
      rv = value->GetCSSStyleRuleValue()->Clone(*getter_AddRefs(ruleClone));
      NS_ENSURE_SUCCESS(rv, rv);

      nsCOMPtr<nsICSSStyleRule> styleRule = do_QueryInterface(ruleClone);
      NS_ENSURE_TRUE(styleRule, NS_ERROR_UNEXPECTED);

      rv = aDst->SetInlineStyleRule(styleRule, PR_FALSE);
      NS_ENSURE_SUCCESS(rv, rv);

      continue;
    }

    nsAutoString valStr;
    value->ToString(valStr);
    rv = aDst->SetAttr(name->NamespaceID(), name->LocalName(),
                       name->GetPrefix(), valStr, PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  // Copy the baseuri and basetarget
  void* prop;
  if ((prop = GetProperty(nsHTMLAtoms::htmlBaseHref))) {
    rv = aDst->SetProperty(nsHTMLAtoms::htmlBaseHref, prop,
                           nsPropertyTable::SupportsDtorFunc);
    if (NS_SUCCEEDED(rv)) {
      NS_ADDREF(NS_STATIC_CAST(nsIURI*, prop));
    }
  }
  if ((prop = GetProperty(nsHTMLAtoms::htmlBaseTarget))) {
    rv = aDst->SetProperty(nsHTMLAtoms::htmlBaseTarget, prop,
                           nsPropertyTable::SupportsDtorFunc);
    if (NS_SUCCEEDED(rv)) {
      NS_ADDREF(NS_STATIC_CAST(nsIAtom*, prop));
    }
  }

  nsIDocument *newDoc = aDst->GetOwnerDoc();

  if (aDeep) {
    nsIDocument *doc = GetOwnerDoc();
    if (doc == newDoc) {
      rv = CloneChildrenTo(aDst);
    }
    else {
      nsCOMPtr<nsIDOMDocument> domDoc = do_QueryInterface(newDoc);
      rv = ImportChildrenTo(aDst, domDoc);
    }
  }

  return rv;
}

nsresult
nsGenericHTMLElement::GetTagName(nsAString& aTagName)
{
  return GetNodeName(aTagName);
}

NS_IMETHODIMP
nsGenericHTMLElement::SetAttribute(const nsAString& aName,
                                   const nsAString& aValue)
{
  const nsAttrName* name = InternalGetExistingAttrNameFromQName(aName);

  if (!name) {
    nsresult rv = nsContentUtils::CheckQName(aName, PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);

    nsCOMPtr<nsIAtom> nameAtom;
    if (mNodeInfo->NamespaceEquals(kNameSpaceID_None)) {
      nsAutoString lower;
      ToLowerCase(aName, lower);
      nameAtom = do_GetAtom(lower);
    }
    else {
      nameAtom = do_GetAtom(aName);
    }
    NS_ENSURE_TRUE(nameAtom, NS_ERROR_OUT_OF_MEMORY);

    return SetAttr(kNameSpaceID_None, nameAtom, aValue, PR_TRUE);
  }

  return SetAttr(name->NamespaceID(), name->LocalName(), name->GetPrefix(),
                 aValue, PR_TRUE);
}

nsresult
nsGenericHTMLElement::GetNodeName(nsAString& aNodeName)
{
  mNodeInfo->GetQualifiedName(aNodeName);

  if (mNodeInfo->NamespaceEquals(kNameSpaceID_None))
    ToUpperCase(aNodeName);

  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetLocalName(nsAString& aLocalName)
{
  mNodeInfo->GetLocalName(aLocalName);

  if (mNodeInfo->NamespaceEquals(kNameSpaceID_None)) {
    // No namespace, this means we're dealing with a good ol' HTML
    // element, so uppercase the local name.

    ToUpperCase(aLocalName);
  }

  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetElementsByTagName(const nsAString& aTagname,
                                           nsIDOMNodeList** aReturn)
{
  nsAutoString tagName(aTagname);

  // Only lowercase the name if this element has no namespace (i.e.
  // it's a HTML element, not an XHTML element).
  if (mNodeInfo && mNodeInfo->NamespaceEquals(kNameSpaceID_None))
    ToLowerCase(tagName);

  return nsGenericElement::GetElementsByTagName(tagName, aReturn);
}

nsresult
nsGenericHTMLElement::GetElementsByTagNameNS(const nsAString& aNamespaceURI,
                                             const nsAString& aLocalName,
                                             nsIDOMNodeList** aReturn)
{
  nsAutoString localName(aLocalName);

  // Only lowercase the name if this element has no namespace (i.e.
  // it's a HTML element, not an XHTML element).
  if (mNodeInfo && mNodeInfo->NamespaceEquals(kNameSpaceID_None))
    ToLowerCase(localName);

  return nsGenericElement::GetElementsByTagNameNS(aNamespaceURI, localName,
                                                  aReturn);
}

// Implementation for nsIDOMHTMLElement
nsresult
nsGenericHTMLElement::GetId(nsAString& aId)
{
  GetAttr(kNameSpaceID_None, nsHTMLAtoms::id, aId);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::SetId(const nsAString& aId)
{
  SetAttr(kNameSpaceID_None, nsHTMLAtoms::id, aId, PR_TRUE);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetTitle(nsAString& aTitle)
{
  GetAttr(kNameSpaceID_None, nsHTMLAtoms::title, aTitle);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::SetTitle(const nsAString& aTitle)
{
  SetAttr(kNameSpaceID_None, nsHTMLAtoms::title, aTitle, PR_TRUE);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetLang(nsAString& aLang)
{
  GetAttr(kNameSpaceID_None, nsHTMLAtoms::lang, aLang);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::SetLang(const nsAString& aLang)
{
  SetAttr(kNameSpaceID_None, nsHTMLAtoms::lang, aLang, PR_TRUE);
  return NS_OK;
}

static const nsAttrValue::EnumTable kDirTable[] = {
  { "ltr", NS_STYLE_DIRECTION_LTR },
  { "rtl", NS_STYLE_DIRECTION_RTL },
  { 0 }
};

nsresult
nsGenericHTMLElement::GetDir(nsAString& aDir)
{
  const nsAttrValue* attr = mAttrsAndChildren.GetAttr(nsHTMLAtoms::dir);

  if (attr && attr->Type() == nsAttrValue::eEnum) {
    attr->ToString(aDir);
  }
  else {
    aDir.Truncate();
  }

  return NS_OK;
}

nsresult
nsGenericHTMLElement::SetDir(const nsAString& aDir)
{
  SetAttr(kNameSpaceID_None, nsHTMLAtoms::dir, aDir, PR_TRUE);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetClassName(nsAString& aClassName)
{
  GetAttr(kNameSpaceID_None, nsHTMLAtoms::_class, aClassName);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::SetClassName(const nsAString& aClassName)
{
  SetAttr(kNameSpaceID_None, nsHTMLAtoms::_class, aClassName, PR_TRUE);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetStyle(nsIDOMCSSStyleDeclaration** aStyle)
{
  nsDOMSlots *slots = GetDOMSlots();

  if (!slots->mStyle) {
    // Just in case...
    ReparseStyleAttribute();

    nsresult rv;
    if (!gCSSOMFactory) {
      rv = CallGetService(kCSSOMFactoryCID, &gCSSOMFactory);
      if (NS_FAILED(rv)) {
        return rv;
      }
    }

    rv = gCSSOMFactory->CreateDOMCSSAttributeDeclaration(this,
                                                         getter_AddRefs(slots->mStyle));
    if (NS_FAILED(rv)) {
      return rv;
    }
  }

  // Why bother with QI?
  NS_IF_ADDREF(*aStyle = slots->mStyle);
  return NS_OK;
}

void
nsGenericHTMLElement::RecreateFrames()
{
  nsIDocument* document = GetCurrentDoc();
  
  if (!document) {
    return;
  }

  PRInt32 numShells = document->GetNumberOfShells();
  for (PRInt32 i = 0; i < numShells; ++i) {
    nsIPresShell *shell = document->GetShellAt(i);
    if (shell) {
      nsIFrame* frame = shell->GetPrimaryFrameFor(this);
      if (frame) {
        shell->RecreateFramesFor(this);
      }
    }
  }
}

static PRBool
IsBody(nsIContent *aContent)
{
  return (aContent->NodeInfo()->Equals(nsHTMLAtoms::body) &&
          aContent->IsNodeOfType(nsINode::eHTML));
}

static PRBool
IsOffsetParent(nsIContent *aContent)
{
  nsINodeInfo *ni = aContent->NodeInfo();

  return ((ni->Equals(nsHTMLAtoms::td) ||
           ni->Equals(nsHTMLAtoms::table) ||
           ni->Equals(nsHTMLAtoms::th)) &&
          aContent->IsNodeOfType(nsINode::eHTML));
}

void
nsGenericHTMLElement::GetOffsetRect(nsRect& aRect, nsIContent** aOffsetParent)
{
  *aOffsetParent = nsnull;

  aRect.x = aRect.y = 0;
  aRect.Empty();

  nsIDocument *document = GetCurrentDoc();
  if (!document) {
    return;
  }

  // Flush all pending notifications so that our frames are up to date.  Make
  // sure to do this first thing, since it may end up destroying our document's
  // presshell.
  document->FlushPendingNotifications(Flush_Layout);

  // Get Presentation shell 0
  nsIPresShell *presShell = document->GetShellAt(0);

  if (!presShell) {
    return;
  }

  // Get the Presentation Context from the Shell
  nsPresContext *context = presShell->GetPresContext();
  if (!context) {
    return;
  }

  // Get the Frame for our content
  nsIFrame* frame = presShell->GetPrimaryFrameFor(this);

  if (!frame) {
    return;
  }

  // Get the union of all rectangles in this and continuation frames
  nsRect rcFrame = nsLayoutUtils::GetAllInFlowBoundingRect(frame);

  nsIContent *docElement = document->GetRootContent();

  // Find the frame parent whose content's tagName either matches
  // the tagName passed in or is the document element.
  nsIFrame* parent = nsnull;
  PRBool done = PR_FALSE;

  nsIContent* content = frame->GetContent();

  if (content) {
    if (IsBody(content) || content == docElement) {
      done = PR_TRUE;

      parent = frame;
    }
  }

  nsPoint origin(0, 0);

  if (!done) {
    PRBool is_absolutely_positioned = PR_FALSE;
    PRBool is_positioned = PR_FALSE;

    origin = frame->GetPositionIgnoringScrolling();

    const nsStyleDisplay* display = frame->GetStyleDisplay();

    if (display->IsPositioned()) {
      if (display->IsAbsolutelyPositioned()) {
        // If the primary frame or a parent is absolutely positioned
        // (fixed or absolute) we stop walking up the frame parent
        // chain

        is_absolutely_positioned = PR_TRUE;
      }

      // We need to know if the primary frame is positioned later on.
      is_positioned = PR_TRUE;
    }

    parent = frame->GetParent();

    while (parent) {
      display = parent->GetStyleDisplay();

      if (display->IsPositioned()) {
        // Stop at the first *parent* that is positioned (fixed,
        // absolute, or relatiive)

        *aOffsetParent = parent->GetContent();
        NS_IF_ADDREF(*aOffsetParent);

        break;
      }

      // Add the parent's origin to our own to get to the
      // right coordinate system

      if (!is_absolutely_positioned) {
        origin += parent->GetPositionIgnoringScrolling();
      }

      content = parent->GetContent();

      if (content) {
        // If we've hit the document element, break here
        if (content == docElement) {
          break;
        }

        // If the tag of this frame is a offset parent tag and this
        // element is *not* positioned, break here. Also break if we
        // hit the body element.
        if ((!is_positioned && IsOffsetParent(content)) || IsBody(content)) {
          *aOffsetParent = content;
          NS_ADDREF(*aOffsetParent);

          break;
        }
      }

      parent = parent->GetParent();
    }

    if (is_absolutely_positioned && !*aOffsetParent) {
      // If this element is absolutely positioned, but we don't have
      // an offset parent it means this element is an absolutely
      // positioned child that's not nested inside another positioned
      // element, in this case the element's frame's parent is the
      // frame for the HTML element so we fail to find the body in the
      // parent chain. We want the offset parent in this case to be
      // the body, so we just get the body element from the document.

      nsCOMPtr<nsIDOMHTMLDocument> html_doc(do_QueryInterface(document));

      if (html_doc) {
        nsCOMPtr<nsIDOMHTMLElement> html_element;

        html_doc->GetBody(getter_AddRefs(html_element));

        if (html_element) {
          CallQueryInterface(html_element, aOffsetParent);
        }
      }
    }
  }

  // For the origin, add in the border for the frame

#if 0
  // We used to do this to include the border of the frame in the
  // calculations, but I think that's wrong. My tests show that we
  // work more like IE if we don't do this, so lets try this and see
  // if people agree.
  const nsStyleBorder* border = frame->GetStyleBorder();
  origin.x += border->GetBorderWidth(NS_SIDE_LEFT);
  origin.y += border->GetBorderWidth(NS_SIDE_TOP);
#endif

  // And subtract out the border for the parent
  if (parent) {
    PRBool includeBorder = PR_TRUE;  // set to false if border-box sizing is used
    const nsStylePosition* position = parent->GetStylePosition();
    if (position->mBoxSizing == NS_STYLE_BOX_SIZING_BORDER) {
      includeBorder = PR_FALSE;
    }
    
    if (includeBorder) {
      const nsStyleBorder* border = parent->GetStyleBorder();
      origin.x -= border->GetBorderWidth(NS_SIDE_LEFT);
      origin.y -= border->GetBorderWidth(NS_SIDE_TOP);
    }
  }

  // XXX We should really consider subtracting out padding for
  // content-box sizing, but we should see what IE does....
  
  // Get the scale from that Presentation Context
  float scale;
  scale = context->TwipsToPixels();

  // Convert to pixels using that scale
  aRect.x = NSTwipsToIntPixels(origin.x, scale);
  aRect.y = NSTwipsToIntPixels(origin.y, scale);
  aRect.width = NSTwipsToIntPixels(rcFrame.width, scale);
  aRect.height = NSTwipsToIntPixels(rcFrame.height, scale);
}

nsresult
nsGenericHTMLElement::GetOffsetTop(PRInt32* aOffsetTop)
{
  nsRect rcFrame;
  nsCOMPtr<nsIContent> parent;
  GetOffsetRect(rcFrame, getter_AddRefs(parent));

  *aOffsetTop = rcFrame.y;

  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetOffsetLeft(PRInt32* aOffsetLeft)
{
  nsRect rcFrame;
  nsCOMPtr<nsIContent> parent;
  GetOffsetRect(rcFrame, getter_AddRefs(parent));

  *aOffsetLeft = rcFrame.x;

  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetOffsetWidth(PRInt32* aOffsetWidth)
{
  nsRect rcFrame;
  nsCOMPtr<nsIContent> parent;
  GetOffsetRect(rcFrame, getter_AddRefs(parent));

  *aOffsetWidth = rcFrame.width;

  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetOffsetHeight(PRInt32* aOffsetHeight)
{
  nsRect rcFrame;
  nsCOMPtr<nsIContent> parent;
  GetOffsetRect(rcFrame, getter_AddRefs(parent));

  *aOffsetHeight = rcFrame.height;

  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetOffsetParent(nsIDOMElement** aOffsetParent)
{
  nsRect rcFrame;
  nsCOMPtr<nsIContent> parent;
  GetOffsetRect(rcFrame, getter_AddRefs(parent));

  if (parent) {
    CallQueryInterface(parent, aOffsetParent);
  } else {
    *aOffsetParent = nsnull;
  }

  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetInnerHTML(nsAString& aInnerHTML)
{
  aInnerHTML.Truncate();

  nsCOMPtr<nsIDocument> doc = GetOwnerDoc();
  if (!doc) {
    return NS_OK; // We rely on the document for doing HTML conversion
  }

  nsCOMPtr<nsIDOMNode> thisNode(do_QueryInterface(NS_STATIC_CAST(nsIContent *,
                                                                 this)));
  nsresult rv = NS_OK;

  nsAutoString contentType;
  if (!doc->IsCaseSensitive()) {
    // All case-insensitive documents are HTML as far as we're concerned
    contentType.AssignLiteral("text/html");
  } else {
    doc->GetContentType(contentType);
  }
  
  nsCOMPtr<nsIDocumentEncoder> docEncoder;
  docEncoder =
    do_CreateInstance(PromiseFlatCString(
        nsDependentCString(NS_DOC_ENCODER_CONTRACTID_BASE) +
        NS_ConvertUTF16toUTF8(contentType)
      ).get());
  if (!docEncoder && doc->IsCaseSensitive()) {
    // This could be some type for which we create a synthetic document.  Try
    // again as XML
    contentType.AssignLiteral("application/xml");
    docEncoder = do_CreateInstance(NS_DOC_ENCODER_CONTRACTID_BASE "application/xml");
  }

  NS_ENSURE_TRUE(docEncoder, NS_ERROR_FAILURE);

  nsCOMPtr<nsIDOMDocument> domDoc = do_QueryInterface(doc);
  rv = docEncoder->Init(domDoc, contentType,
                        nsIDocumentEncoder::OutputEncodeBasicEntities |
                        // Output DOM-standard newlines
                        nsIDocumentEncoder::OutputLFLineBreak |
                        // Don't do linebreaking that's not present in the source
                        nsIDocumentEncoder::OutputRaw);
  NS_ENSURE_SUCCESS(rv, rv);

  docEncoder->SetContainerNode(thisNode);
  return docEncoder->EncodeToString(aInnerHTML);
}

nsresult
nsGenericHTMLElement::SetInnerHTML(const nsAString& aInnerHTML)
{
  // Remove childnodes
  nsNode3Tearoff::SetTextContent(this, EmptyString());

  nsCOMPtr<nsIDOMDocumentFragment> df;

  nsCOMPtr<nsIDocument> doc = GetOwnerDoc();

  // Strong ref since appendChild can fire events
  nsCOMPtr<nsIScriptLoader> loader;
  PRBool scripts_enabled = PR_FALSE;

  if (doc) {
    loader = doc->GetScriptLoader();
    if (loader) {
      loader->GetEnabled(&scripts_enabled);
    }
  }

  if (scripts_enabled) {
    // Don't let scripts execute while setting .innerHTML.

    loader->SetEnabled(PR_FALSE);
  }

  nsCOMPtr<nsIDOMNode> thisNode(do_QueryInterface(NS_STATIC_CAST(nsIContent *,
                                                                 this)));
  nsresult rv = nsContentUtils::CreateContextualFragment(thisNode, aInnerHTML,
                                                         getter_AddRefs(df));
  if (NS_SUCCEEDED(rv)) {
    nsCOMPtr<nsIDOMNode> tmpNode;
    rv = thisNode->AppendChild(df, getter_AddRefs(tmpNode));
  }

  if (scripts_enabled) {
    // If we disabled scripts, re-enable them now that we're
    // done. Don't fire JS timeouts when enabling the context here.

    loader->SetEnabled(PR_TRUE);
  }

  return rv;
}

void
nsGenericHTMLElement::GetScrollInfo(nsIScrollableView **aScrollableView,
                                    float *aP2T, float *aT2P,
                                    nsIFrame **aFrame)
{
  *aScrollableView = nsnull;
  *aP2T = 0.0f;
  *aT2P = 0.0f;

  nsIDocument *document = GetCurrentDoc();
  if (!document) {
    return;
  }

  // Flush all pending notifications so that our frames are up to date.  Make
  // sure to do this first thing, since it may end up destroying our document's
  // presshell.
  document->FlushPendingNotifications(Flush_Layout);

  // Get the presentation shell
  nsIPresShell *presShell = document->GetShellAt(0);
  if (!presShell) {
    return;
  }

  // Get the presentation context
  nsPresContext *presContext = presShell->GetPresContext();
  if (!presContext) {
    return;
  }

  // Get the primary frame for this element
  nsIFrame *frame = presShell->GetPrimaryFrameFor(this);
  if (!frame) {
    return;
  }

  if (aFrame) {
    *aFrame = frame;
  }

  *aP2T = presContext->PixelsToTwips();
  *aT2P = presContext->TwipsToPixels();

  // Get the scrollable frame
  nsIScrollableFrame *scrollFrame = nsnull;
  CallQueryInterface(frame, &scrollFrame);

  if (!scrollFrame) {
    nsIScrollableViewProvider *scrollProvider = nsnull;
    CallQueryInterface(frame, &scrollProvider);
    if (scrollProvider) {
      *aScrollableView = scrollProvider->GetScrollableView();
      if (*aScrollableView) {
        return;
      }
    }

    PRBool quirksMode = InNavQuirksMode(document);
    if ((quirksMode && mNodeInfo->Equals(nsHTMLAtoms::body)) ||
        (!quirksMode && mNodeInfo->Equals(nsHTMLAtoms::html))) {
      // In quirks mode, the scroll info for the body element should map to the
      // scroll info for the nearest scrollable frame above the body element
      // (i.e. the root scrollable frame).  This is what IE6 does in quirks
      // mode.  In strict mode the root scrollable frame corresponds to the
      // html element in IE6, so we map the scroll info for the html element to
      // the root scrollable frame.

      do {
        frame = frame->GetParent();

        if (!frame) {
          break;
        }

        CallQueryInterface(frame, &scrollFrame);
      } while (!scrollFrame);
    }

    if (!scrollFrame) {
      return;
    }
  }

  // Get the scrollable view
  *aScrollableView = scrollFrame->GetScrollableView();

  return;
}


nsresult
nsGenericHTMLElement::GetScrollTop(PRInt32* aScrollTop)
{
  NS_ENSURE_ARG_POINTER(aScrollTop);
  *aScrollTop = 0;

  nsIScrollableView *view = nsnull;
  nsresult rv = NS_OK;
  float p2t, t2p;

  GetScrollInfo(&view, &p2t, &t2p);

  if (view) {
    nscoord xPos, yPos;
    rv = view->GetScrollPosition(xPos, yPos);

    *aScrollTop = NSTwipsToIntPixels(yPos, t2p);
  }

  return rv;
}

nsresult
nsGenericHTMLElement::SetScrollTop(PRInt32 aScrollTop)
{
  nsIScrollableView *view = nsnull;
  nsresult rv = NS_OK;
  float p2t, t2p;

  GetScrollInfo(&view, &p2t, &t2p);

  if (view) {
    nscoord xPos, yPos;

    rv = view->GetScrollPosition(xPos, yPos);

    if (NS_SUCCEEDED(rv)) {
      rv = view->ScrollTo(xPos, NSIntPixelsToTwips(aScrollTop, p2t),
                          NS_VMREFRESH_IMMEDIATE);
    }
  }

  return rv;
}

nsresult
nsGenericHTMLElement::GetScrollLeft(PRInt32* aScrollLeft)
{
  NS_ENSURE_ARG_POINTER(aScrollLeft);
  *aScrollLeft = 0;

  nsIScrollableView *view = nsnull;
  nsresult rv = NS_OK;
  float p2t, t2p;

  GetScrollInfo(&view, &p2t, &t2p);

  if (view) {
    nscoord xPos, yPos;
    rv = view->GetScrollPosition(xPos, yPos);

    *aScrollLeft = NSTwipsToIntPixels(xPos, t2p);
  }

  return rv;
}

nsresult
nsGenericHTMLElement::SetScrollLeft(PRInt32 aScrollLeft)
{
  nsIScrollableView *view = nsnull;
  nsresult rv = NS_OK;
  float p2t, t2p;

  GetScrollInfo(&view, &p2t, &t2p);

  if (view) {
    nscoord xPos, yPos;
    rv = view->GetScrollPosition(xPos, yPos);

    if (NS_SUCCEEDED(rv)) {
      rv = view->ScrollTo(NSIntPixelsToTwips(aScrollLeft, p2t),
                          yPos, NS_VMREFRESH_IMMEDIATE);
    }
  }

  return rv;
}

nsresult
nsGenericHTMLElement::GetScrollHeight(PRInt32* aScrollHeight)
{
  NS_ENSURE_ARG_POINTER(aScrollHeight);
  *aScrollHeight = 0;

  nsIScrollableView *scrollView = nsnull;
  nsresult rv = NS_OK;
  float p2t, t2p;

  GetScrollInfo(&scrollView, &p2t, &t2p);

  if (!scrollView) {
    return GetOffsetHeight(aScrollHeight);
  }

  // xMax and yMax is the total length of our container
  nscoord xMax, yMax;
  rv = scrollView->GetContainerSize(&xMax, &yMax);

  *aScrollHeight = NSTwipsToIntPixels(yMax, t2p);

  return rv;
}

nsresult
nsGenericHTMLElement::GetScrollWidth(PRInt32* aScrollWidth)
{
  NS_ENSURE_ARG_POINTER(aScrollWidth);
  *aScrollWidth = 0;

  nsIScrollableView *scrollView = nsnull;
  nsresult rv = NS_OK;
  float p2t, t2p;

  GetScrollInfo(&scrollView, &p2t, &t2p);

  if (!scrollView) {
    return GetOffsetWidth(aScrollWidth);
  }

  nscoord xMax, yMax;
  rv = scrollView->GetContainerSize(&xMax, &yMax);

  *aScrollWidth = NSTwipsToIntPixels(xMax, t2p);

  return rv;
}

// static
const nsSize
nsGenericHTMLElement::GetClientAreaSize(nsIFrame *aFrame)
{
  nsRect rect = aFrame->GetRect();

  const nsStyleBorder* border = aFrame->GetStyleBorder();

  nsMargin border_size;
  border->CalcBorderFor(aFrame, border_size);

  rect.Deflate(border_size);

  return nsSize(rect.width, rect.height);
}

nsresult
nsGenericHTMLElement::GetClientHeight(PRInt32* aClientHeight)
{
  NS_ENSURE_ARG_POINTER(aClientHeight);
  *aClientHeight = 0;

  nsIScrollableView *scrollView = nsnull;
  float p2t, t2p;
  nsIFrame *frame = nsnull;

  GetScrollInfo(&scrollView, &p2t, &t2p, &frame);

  if (scrollView) {
    nsRect r = scrollView->View()->GetBounds();

    *aClientHeight = NSTwipsToIntPixels(r.height, t2p);
  } else if (frame &&
             (frame->GetStyleDisplay()->mDisplay != NS_STYLE_DISPLAY_INLINE ||
              (frame->GetStateBits() & NS_FRAME_REPLACED_ELEMENT))) {
    // Special case code to make clientHeight work even when there isn't
    // a scroll view, see bug 180552 and bug 227567.

    *aClientHeight = NSTwipsToIntPixels(GetClientAreaSize(frame).height, t2p);
  }

  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetClientWidth(PRInt32* aClientWidth)
{
  NS_ENSURE_ARG_POINTER(aClientWidth);
  *aClientWidth = 0;

  nsIScrollableView *scrollView = nsnull;
  float p2t, t2p;
  nsIFrame *frame = nsnull;

  GetScrollInfo(&scrollView, &p2t, &t2p, &frame);

  if (scrollView) {
    nsRect r = scrollView->View()->GetBounds();

    *aClientWidth = NSTwipsToIntPixels(r.width, t2p);
  } else if (frame &&
             (frame->GetStyleDisplay()->mDisplay != NS_STYLE_DISPLAY_INLINE ||
              (frame->GetStateBits() & NS_FRAME_REPLACED_ELEMENT))) {
    // Special case code to make clientWidth work even when there isn't
    // a scroll view, see bug 180552 and bug 227567.

    *aClientWidth = NSTwipsToIntPixels(GetClientAreaSize(frame).width, t2p);
  }

  return NS_OK;
}

nsresult
nsGenericHTMLElement::ScrollIntoView(PRBool aTop)
{
  nsIDocument *document = GetCurrentDoc();

  if (!document) {
    return NS_OK;
  }

  // Flush all pending notifications so that our frames are up to date.  Make
  // sure to do this first thing, since it may end up destroying our document's
  // presshell.
  document->FlushPendingNotifications(Flush_Layout);
  
  // Get the presentation shell
  nsIPresShell *presShell = document->GetShellAt(0);
  if (!presShell) {
    return NS_OK;
  }

  // Get the primary frame for this element
  nsIFrame *frame = presShell->GetPrimaryFrameFor(this);
  if (!frame) {
    return NS_OK;
  }

  PRIntn vpercent = aTop ? NS_PRESSHELL_SCROLL_TOP :
    NS_PRESSHELL_SCROLL_ANYWHERE;

  presShell->ScrollFrameIntoView(frame, vpercent,
                                 NS_PRESSHELL_SCROLL_ANYWHERE);

  return NS_OK;
}


PRBool
nsGenericHTMLElement::InNavQuirksMode(nsIDocument* aDoc)
{
  nsCOMPtr<nsIHTMLDocument> doc(do_QueryInterface(aDoc));
  if (!doc) {
    return PR_FALSE;
  }

  return doc->GetCompatibilityMode() == eCompatibility_NavQuirks;
}

nsresult
nsGenericHTMLElement::BindToTree(nsIDocument* aDocument, nsIContent* aParent,
                                 nsIContent* aBindingParent,
                                 PRBool aCompileEventHandlers)
{
  nsresult rv = nsGenericElement::BindToTree(aDocument, aParent,
                                             aBindingParent,
                                             aCompileEventHandlers);
  NS_ENSURE_SUCCESS(rv, rv);

  // XXXbz if we already have a style attr parsed, this won't do
  // anything... need to fix that.
  ReparseStyleAttribute();

  if (aDocument) {
    // If we're in a document now, let our mapped attrs know what their new
    // sheet is.
    nsHTMLStyleSheet* sheet = aDocument->GetAttributeStyleSheet();
    if (sheet) {
      mAttrsAndChildren.SetMappedAttrStyleSheet(sheet);
    }
  }

  return rv;
}

already_AddRefed<nsIDOMHTMLFormElement>
nsGenericHTMLElement::FindForm(nsIForm* aCurrentForm)
{
  nsIContent* content = this;
  while (content) {
    // If the current ancestor is a form, return it as our form
    if (content->Tag() == nsHTMLAtoms::form &&
        content->IsNodeOfType(nsINode::eHTML)) {
      nsIDOMHTMLFormElement* form;
      CallQueryInterface(content, &form);

      return form;
    }

    nsIContent *prevContent = content;
    content = prevContent->GetParent();

    if (!content && aCurrentForm) {
      // We got to the root of the subtree we're in, and we're being removed
      // from the DOM (the only time we get into this method with a non-null
      // aCurrentForm).  Check whether aCurrentForm is in the same subtree.  If
      // it is, we want to return aCurrentForm, since this case means that
      // we're one of those inputs-in-a-table that have a hacked mForm pointer
      // and a subtree containing both us and the form got removed from the
      // DOM.
      nsCOMPtr<nsIContent> formCOMPtr = do_QueryInterface(aCurrentForm);
      NS_ASSERTION(formCOMPtr, "aCurrentForm isn't an nsIContent?");
      // Use an nsIContent temporary to reduce addref/releasing as we go up the
      // tree
      nsINode* iter = formCOMPtr;
      do {
        iter = iter->GetNodeParent();
        if (iter == prevContent) {
          nsIDOMHTMLFormElement* form;
          CallQueryInterface(aCurrentForm, &form);
          return form;
        }
      } while (iter);
    }
    
    if (content) {
      PRInt32 i = content->IndexOf(prevContent);

      if (i < 0) {
        // This means 'prevContent' is anonymous content, form controls in
        // anonymous content can't refer to the real form, if they do
        // they end up in form.elements n' such, and that's wrong...

        return nsnull;
      }
    }
  }

  return nsnull;
}

static PRBool
IsArea(nsIContent *aContent)
{
  return (aContent->Tag() == nsHTMLAtoms::area &&
          aContent->IsNodeOfType(nsINode::eHTML));
}

nsresult
nsGenericHTMLElement::PostHandleEventForAnchors(nsEventChainPostVisitor& aVisitor)
{
  NS_PRECONDITION(nsCOMPtr<nsILink>(do_QueryInterface(this)),
                  "should be called only when |this| implements |nsILink|");

  nsresult rv = NS_OK;

  if (!aVisitor.mPresContext) {
    // We need a pres context to do link stuff. Some events (e.g. mutation
    // events) don't have one.
    // XXX: ideally, shouldn't we be able to do what we need without one?
    return rv; 
  }

  if (!NS_IS_TRUSTED_EVENT(aVisitor.mEvent)) {
    return rv;
  }

  //Need to check if we hit an imagemap area and if so see if we're handling
  //the event on that map or on a link farther up the tree.  If we're on a
  //link farther up, do nothing.
  nsCOMPtr<nsIContent> target;

  aVisitor.mPresContext->EventStateManager()->
    GetEventTargetContent(aVisitor.mEvent, getter_AddRefs(target));

  if (target && IsArea(target) && !IsArea(this)) {
    return rv;
  }

  if (aVisitor.mEventStatus != nsEventStatus_eConsumeNoDefault) {

    // We'll use the equivalent of |GetHrefUTF8| on the
    // nsILink interface to get a canonified URL that has been
    // correctly escaped and URL-encoded for the document's charset.

    nsCOMPtr<nsIURI> hrefURI;
    GetHrefURIForAnchors(getter_AddRefs(hrefURI));

    // Only bother to handle the mouse event if there was an href
    // specified.
    if (hrefURI) {
      switch (aVisitor.mEvent->message) {
      case NS_MOUSE_LEFT_BUTTON_DOWN:
        {
          // don't make the link grab the focus if there is no link handler
          nsILinkHandler *handler = aVisitor.mPresContext->GetLinkHandler();
          nsIDocument *document = GetCurrentDoc();
          if (handler && document && ShouldFocus(this)) {
            // If the window is not active, do not allow the focus to bring the
            // window to the front.  We update the focus controller, but do
            // nothing else.
            nsPIDOMWindow *win = document->GetWindow();
            if (win) {
              nsIFocusController *focusController =
                win->GetRootFocusController();
              PRBool isActive = PR_FALSE;
              focusController->GetActive(&isActive);
              if (!isActive) {
                nsCOMPtr<nsIDOMElement> domElement = do_QueryInterface(this);
                if(domElement)
                  focusController->SetFocusedElement(domElement);
                break;
              }
            }

            aVisitor.mPresContext->EventStateManager()->
              SetContentState(this,
                              NS_EVENT_STATE_ACTIVE | NS_EVENT_STATE_FOCUS);
          }
        }
        break;

      case NS_MOUSE_LEFT_CLICK:
        if (aVisitor.mEvent->eventStructType == NS_MOUSE_EVENT) {
          nsInputEvent* inputEvent =
            NS_STATIC_CAST(nsInputEvent*, aVisitor.mEvent);
          if (inputEvent->isControl || inputEvent->isMeta ||
              inputEvent->isAlt ||inputEvent->isShift) {
            break;
          }

          // The default action is simply to dispatch DOMActivate
          nsIPresShell *shell = aVisitor.mPresContext->GetPresShell();
          if (shell) {
            // single-click
            nsUIEvent actEvent(NS_IS_TRUSTED_EVENT(aVisitor.mEvent),
                               NS_UI_ACTIVATE, 1);
            nsEventStatus status = nsEventStatus_eIgnore;

            rv = shell->HandleDOMEventWithTarget(this, &actEvent, &status);
          }
        }
        break;

      case NS_UI_ACTIVATE:
        {
          nsAutoString target;
          GetAttr(kNameSpaceID_None, nsHTMLAtoms::target, target);
          if (target.IsEmpty()) {
            GetBaseTarget(target);
          }
          rv = TriggerLink(aVisitor.mPresContext, eLinkVerb_Replace, hrefURI,
                           target, PR_TRUE, PR_TRUE);
        }
        break;

      case NS_KEY_PRESS:
        if (aVisitor.mEvent->eventStructType == NS_KEY_EVENT) {
          nsKeyEvent* keyEvent = NS_STATIC_CAST(nsKeyEvent*, aVisitor.mEvent);
          if (keyEvent->keyCode == NS_VK_RETURN) {
            nsEventStatus status = nsEventStatus_eIgnore;
            rv = DispatchClickEvent(aVisitor.mPresContext, keyEvent, this,
                                    PR_FALSE, &status);
            if (NS_SUCCEEDED(rv)) {
              aVisitor.mEventStatus = nsEventStatus_eConsumeNoDefault;
            }
          }
        }
        break;

      // Set the status bar the same for focus and mouseover
      case NS_MOUSE_ENTER_SYNTH:
        aVisitor.mEventStatus = nsEventStatus_eConsumeNoDefault;
      case NS_FOCUS_CONTENT:
      {
        nsAutoString target;
        GetAttr(kNameSpaceID_None, nsHTMLAtoms::target, target);
        if (target.IsEmpty()) {
          GetBaseTarget(target);
        }
        rv = TriggerLink(aVisitor.mPresContext, eLinkVerb_Replace,
                         hrefURI, target, PR_FALSE, PR_TRUE);
      }
      break;

      case NS_MOUSE_EXIT_SYNTH:
      {
        aVisitor.mEventStatus = nsEventStatus_eConsumeNoDefault;
        rv = LeaveLink(aVisitor.mPresContext);
      }
      break;

      default:
        break;
      }
    }
  }
  return rv;
}

nsresult
nsGenericHTMLElement::GetHrefURIForAnchors(nsIURI** aURI)
{
  // This is used by the three nsILink implementations and
  // nsHTMLStyleElement.

  // Get href= attribute (relative URI).

  // We use the nsAttrValue's copy of the URI string to avoid copying.
  const nsAttrValue* attr = mAttrsAndChildren.GetAttr(nsHTMLAtoms::href);
  if (attr) {
    // Get base URI.
    nsCOMPtr<nsIURI> baseURI = GetBaseURI();

    // Get absolute URI.
    nsresult rv = nsContentUtils::NewURIWithDocumentCharset(aURI,
                                                            attr->GetStringValue(),
                                                            GetOwnerDoc(),
                                                            baseURI);
    if (NS_FAILED(rv)) {
      *aURI = nsnull;
    }
  }
  else {
    // Absolute URI is null to say we have no HREF.
    *aURI = nsnull;
  }

  return NS_OK;
}

PRBool nsGenericHTMLElement::IsEventName(nsIAtom* aName)
{
  const char* name;
  aName->GetUTF8String(&name);

  if (name[0] != 'o' || name[1] != 'n') {
    return PR_FALSE;
  }

  return (aName == nsLayoutAtoms::onclick                       ||
          aName == nsLayoutAtoms::ondblclick                    ||
          aName == nsLayoutAtoms::onmousedown                   ||
          aName == nsLayoutAtoms::onmouseup                     ||
          aName == nsLayoutAtoms::onmouseover                   ||
          aName == nsLayoutAtoms::onmouseout                    ||
          aName == nsLayoutAtoms::onkeydown                     ||
          aName == nsLayoutAtoms::onkeyup                       ||
          aName == nsLayoutAtoms::onkeypress                    ||
          aName == nsLayoutAtoms::onmousemove                   ||
          aName == nsLayoutAtoms::onload                        ||
          aName == nsLayoutAtoms::onunload                      ||
          aName == nsLayoutAtoms::onbeforeunload                ||
          aName == nsLayoutAtoms::onpageshow                    ||
          aName == nsLayoutAtoms::onpagehide                    ||
          aName == nsLayoutAtoms::onabort                       ||
          aName == nsLayoutAtoms::onerror                       ||
          aName == nsLayoutAtoms::onfocus                       ||
          aName == nsLayoutAtoms::onblur                        ||
          aName == nsLayoutAtoms::onsubmit                      ||
          aName == nsLayoutAtoms::onreset                       ||
          aName == nsLayoutAtoms::onchange                      ||
          aName == nsLayoutAtoms::onselect                      || 
          aName == nsLayoutAtoms::onpaint                       ||
          aName == nsLayoutAtoms::onresize                      ||
          aName == nsLayoutAtoms::onscroll                      ||
          aName == nsLayoutAtoms::oninput                       ||
          aName == nsLayoutAtoms::oncontextmenu                 ||
          aName == nsLayoutAtoms::onDOMAttrModified             ||
          aName == nsLayoutAtoms::onDOMCharacterDataModified    || 
          aName == nsLayoutAtoms::onDOMSubtreeModified          ||
          aName == nsLayoutAtoms::onDOMNodeInsertedIntoDocument || 
          aName == nsLayoutAtoms::onDOMNodeRemovedFromDocument  ||
          aName == nsLayoutAtoms::onDOMNodeInserted             || 
          aName == nsLayoutAtoms::onDOMNodeRemoved              ||
          aName == nsLayoutAtoms::onDOMActivate                 ||
          aName == nsLayoutAtoms::onDOMFocusIn                  ||
          aName == nsLayoutAtoms::onDOMFocusOut);
}

nsresult
nsGenericHTMLElement::AfterSetAttr(PRInt32 aNamespaceID, nsIAtom* aName,
                                   const nsAString* aValue, PRBool aNotify)
{
  if (aNamespaceID == kNameSpaceID_None && IsEventName(aName) && aValue) {
    nsresult rv = AddScriptEventListener(aName, *aValue);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return nsGenericElement::AfterSetAttr(aNamespaceID, aName,
                                        aValue, aNotify);
}

nsresult
nsGenericHTMLElement::GetEventListenerManagerForAttr(nsIEventListenerManager** aManager,
                                                     nsISupports** aTarget,
                                                     PRBool* aDefer)
{
  // Attributes on the body and frameset tags get set on the global object
  if (mNodeInfo->Equals(nsHTMLAtoms::body) ||
      mNodeInfo->Equals(nsHTMLAtoms::frameset)) {
    nsPIDOMWindow *win;

    // If we have a document, and it has a window, add the event
    // listener on the window (the inner window). If not, proceed as
    // normal.
    // XXXbz sXBL/XBL2 issue: should we instead use GetCurrentDoc() here,
    // override BindToTree for those classes and munge event listeners there?
    nsIDocument *document = GetOwnerDoc();
    nsresult rv = NS_OK;
    if (document && (win = document->GetInnerWindow())) {
      nsCOMPtr<nsIDOMEventReceiver> receiver(do_QueryInterface(win));
      NS_ENSURE_TRUE(receiver, NS_ERROR_FAILURE);

      rv = receiver->GetListenerManager(PR_TRUE, aManager);

      if (NS_SUCCEEDED(rv)) {
        NS_ADDREF(*aTarget = win);
      }
      *aDefer = PR_FALSE;
    } else {
      *aManager = nsnull;
      *aTarget = nsnull;
      *aDefer = PR_FALSE;
    }

    return rv;
  }

  return nsGenericElement::GetEventListenerManagerForAttr(aManager, aTarget,
                                                          aDefer);
}

nsresult
nsGenericHTMLElement::UnsetAttr(PRInt32 aNameSpaceID, nsIAtom* aAttribute,
                                PRBool aNotify)
{
  // Check for event handlers
  if (aNameSpaceID == kNameSpaceID_None &&
      IsEventName(aAttribute)) {
    nsCOMPtr<nsIEventListenerManager> manager;
    GetListenerManager(PR_FALSE, getter_AddRefs(manager));

    if (manager) {
      manager->RemoveScriptEventListener(aAttribute);
    }
  }

  return nsGenericElement::UnsetAttr(aNameSpaceID, aAttribute, aNotify);
}

const nsAttrValue*
nsGenericHTMLElement::GetClasses() const
{
  return mAttrsAndChildren.GetAttr(nsHTMLAtoms::_class);
}

nsIAtom *
nsGenericHTMLElement::GetIDAttributeName() const
{
  return nsHTMLAtoms::id;
}

nsIAtom *
nsGenericHTMLElement::GetClassAttributeName() const
{
  return nsHTMLAtoms::_class;
}

nsresult
nsGenericHTMLElement::WalkContentStyleRules(nsRuleWalker* aRuleWalker)
{
  mAttrsAndChildren.WalkMappedAttributeStyleRules(aRuleWalker);
  return NS_OK;
}

nsICSSStyleRule*
nsGenericHTMLElement::GetInlineStyleRule()
{ 
  const nsAttrValue* attrVal = mAttrsAndChildren.GetAttr(nsHTMLAtoms::style);
  
  if (attrVal) {
    if (attrVal->Type() != nsAttrValue::eCSSStyleRule) {
      ReparseStyleAttribute();
      attrVal = mAttrsAndChildren.GetAttr(nsHTMLAtoms::style);
      // hopefully attrVal->Type() is now nsAttrValue::eCSSStyleRule
    }

    if (attrVal->Type() == nsAttrValue::eCSSStyleRule) {
      return attrVal->GetCSSStyleRuleValue();
    }
  }

  return nsnull;
}

nsresult
nsGenericHTMLElement::SetInlineStyleRule(nsICSSStyleRule* aStyleRule,
                                         PRBool aNotify)
{
  PRBool modification = PR_FALSE;
  nsAutoString oldValueStr;

  PRBool hasListeners = aNotify &&
    nsContentUtils::HasMutationListeners(this,
                                         NS_EVENT_BITS_MUTATION_ATTRMODIFIED);

  // There's no point in comparing the stylerule pointers since we're always
  // getting a new stylerule here. And we can't compare the stringvalues of
  // the old and the new rules since both will point to the same declaration
  // and thus will be the same.
  if (hasListeners) {
    // save the old attribute so we can set up the mutation event properly
    // XXXbz if the old rule points to the same declaration as the new one,
    // this is getting the new attr value, not the old one....
    modification = GetAttr(kNameSpaceID_None, nsHTMLAtoms::style,
                           oldValueStr);
  }
  else if (aNotify && IsInDoc()) {
    modification = !!mAttrsAndChildren.GetAttr(nsHTMLAtoms::style);
  }

  nsAttrValue attrValue(aStyleRule);

  return SetAttrAndNotify(kNameSpaceID_None, nsHTMLAtoms::style, nsnull, oldValueStr,
                          attrValue, modification, hasListeners, aNotify);
}

already_AddRefed<nsIURI>
nsGenericHTMLElement::GetBaseURI() const
{
  nsIDocument* doc = GetOwnerDoc();

  void* prop;
  if (HasFlag(NODE_HAS_PROPERTIES) && (prop = GetProperty(nsHTMLAtoms::htmlBaseHref))) {
    nsIURI* uri = NS_STATIC_CAST(nsIURI*, prop);
    NS_ADDREF(uri);
    
    return uri;
  }

  // If we are a plain old HTML element (not XHTML), don't bother asking the
  // base class -- our base URI is determined solely by the document base.
  if (mNodeInfo->NamespaceEquals(kNameSpaceID_None)) {
    if (doc) {
      nsIURI *uri = doc->GetBaseURI();
      NS_IF_ADDREF(uri);

      return uri;
    }

    return nsnull;
  }

  return nsGenericElement::GetBaseURI();
}

void
nsGenericHTMLElement::GetBaseTarget(nsAString& aBaseTarget) const
{
  void* prop;
  if (HasFlag(NODE_HAS_PROPERTIES) && (prop = GetProperty(nsHTMLAtoms::htmlBaseTarget))) {
    NS_STATIC_CAST(nsIAtom*, prop)->ToString(aBaseTarget);
    
    return;
  }

  nsIDocument* ownerDoc = GetOwnerDoc();
  if (ownerDoc) {
    ownerDoc->GetBaseTarget(aBaseTarget);
  } else {
    aBaseTarget.Truncate();
  }
}

#ifdef DEBUG
void
nsGenericHTMLElement::ListAttributes(FILE* out) const
{
  PRUint32 index, count = GetAttrCount();

  for (index = 0; index < count; index++) {
    // name
    const nsAttrName* name = GetAttrNameAt(index);

    nsAutoString buffer;
    name->LocalName()->ToString(buffer);

    // value
    nsAutoString value;
    GetAttr(name->NamespaceID(), name->LocalName(), value);
    buffer.AppendLiteral("=\"");
    for (int i = value.Length(); i >= 0; --i) {
      if (value[i] == PRUnichar('"'))
        value.Insert(PRUnichar('\\'), PRUint32(i));
    }
    buffer.Append(value);
    buffer.AppendLiteral("\"");

    fputs(" ", out);
    fputs(NS_LossyConvertUTF16toASCII(buffer).get(), out);
  }
}

void
nsGenericHTMLElement::List(FILE* out, PRInt32 aIndent) const
{
  PRInt32 index;
  for (index = aIndent; --index >= 0; ) fputs("  ", out);

  nsAutoString buf;
  mNodeInfo->GetQualifiedName(buf);
  fputs(NS_LossyConvertUTF16toASCII(buf).get(), out);

  fprintf(out, "@%p", (void*)this);

  ListAttributes(out);

  fprintf(out, " intrinsicstate=[%08x]", IntrinsicState());
  fprintf(out, " refcount=%d<", mRefCnt.get());

  fputs("\n", out);
  PRInt32 kids = GetChildCount();
  for (index = 0; index < kids; index++) {
    nsIContent *kid = GetChildAt(index);
    kid->List(out, aIndent + 1);
  }
  for (index = aIndent; --index >= 0; ) fputs("  ", out);

  fputs(">\n", out);
}

void
nsGenericHTMLElement::DumpContent(FILE* out, PRInt32 aIndent,
                                  PRBool aDumpAll) const
{
  PRInt32 index;
  for (index = aIndent; --index >= 0; ) fputs("  ", out);

  nsAutoString buf;
  mNodeInfo->GetQualifiedName(buf);
  fputs("<",out);
  fputs(NS_LossyConvertUTF16toASCII(buf).get(), out);

  if(aDumpAll) ListAttributes(out);

  fputs(">",out);

  if(aIndent) fputs("\n", out);
  PRInt32 kids = GetChildCount();
  for (index = 0; index < kids; index++) {
    nsIContent *kid = GetChildAt(index);
    PRInt32 indent = aIndent ? aIndent + 1 : 0;
    kid->DumpContent(out, indent, aDumpAll);
  }
  for (index = aIndent; --index >= 0; ) fputs("  ", out);
  fputs("</",out);
  fputs(NS_LossyConvertUTF16toASCII(buf).get(), out);
  fputs(">",out);

  if(aIndent) fputs("\n", out);
}
#endif


PRBool
nsGenericHTMLElement::IsNodeOfType(PRUint32 aFlags) const
{
  return !(aFlags & ~(eCONTENT | eELEMENT | eHTML));
}

//----------------------------------------------------------------------


PRBool
nsGenericHTMLElement::ParseAttribute(PRInt32 aNamespaceID,
                                     nsIAtom* aAttribute,
                                     const nsAString& aValue,
                                     nsAttrValue& aResult)
{
  if (aNamespaceID == kNameSpaceID_None) {
    if (aAttribute == nsHTMLAtoms::dir) {
      return aResult.ParseEnumValue(aValue, kDirTable);
    }
    if (aAttribute == nsHTMLAtoms::style) {
      ParseStyleAttribute(this, mNodeInfo->NamespaceEquals(kNameSpaceID_XHTML),
                          aValue, aResult);
      return PR_TRUE;
    }
    if (aAttribute == nsHTMLAtoms::_class) {
      aResult.ParseAtomArray(aValue);

      return PR_TRUE;
    }
  
    if (aAttribute == nsHTMLAtoms::tabindex) {
      return aResult.ParseIntWithBounds(aValue, -32768, 32767);
    }

    if (aAttribute == nsHTMLAtoms::name && !aValue.IsEmpty()) {
      // Store name as an atom.  name="" means that the element has no name,
      // not that it has an emptystring as the name.
      aResult.ParseAtom(aValue);
      return PR_TRUE;
    }
  }

  return nsGenericElement::ParseAttribute(aNamespaceID, aAttribute, aValue,
                                          aResult);
}

PRBool
nsGenericHTMLElement::IsAttributeMapped(const nsIAtom* aAttribute) const
{
  static const MappedAttributeEntry* const map[] = {
    sCommonAttributeMap
  };
  
  return FindAttributeDependence(aAttribute, map, NS_ARRAY_LENGTH(map));
}

PRBool
nsGenericHTMLElement::SetMappedAttribute(nsIDocument* aDocument,
                                         nsIAtom* aName,
                                         nsAttrValue& aValue,
                                         nsresult* aRetval)
{
  NS_PRECONDITION(aDocument == GetCurrentDoc(), "Unexpected document");
  nsHTMLStyleSheet* sheet = aDocument ?
    aDocument->GetAttributeStyleSheet() : nsnull;
  
  *aRetval = mAttrsAndChildren.SetAndTakeMappedAttr(aName, aValue,
                                                    this, sheet);
  return PR_TRUE;
}


nsMapRuleToAttributesFunc
nsGenericHTMLElement::GetAttributeMappingFunction() const
{
  return &MapCommonAttributesInto;
}

// static
nsIFormControlFrame*
nsGenericHTMLElement::GetFormControlFrameFor(nsIContent* aContent,
                                             nsIDocument* aDocument,
                                             PRBool aFlushContent)
{
  if (aFlushContent) {
    // Cause a flush of content, so we get up-to-date frame
    // information
    aDocument->FlushPendingNotifications(Flush_Frames);
  }
  nsIFrame* frame = GetPrimaryFrameFor(aContent, aDocument);
  if (frame) {
    nsIFormControlFrame* form_frame = nsnull;
    CallQueryInterface(frame, &form_frame);
    if (form_frame) {
      return form_frame;
    }

    // If we have generated content, the primary frame will be a
    // wrapper frame..  out real frame will be in its child list.
    for (frame = frame->GetFirstChild(nsnull);
         frame;
         frame = frame->GetNextSibling()) {
      CallQueryInterface(frame, &form_frame);
      if (form_frame) {
        return form_frame;
      }
    }
  }

  return nsnull;
}

/* static */ nsresult
nsGenericHTMLElement::GetPrimaryPresState(nsGenericHTMLElement* aContent,
                                          nsPresState** aPresState)
{
  NS_ENSURE_ARG_POINTER(aPresState);
  *aPresState = nsnull;

  nsresult result = NS_OK;

  nsCOMPtr<nsILayoutHistoryState> history;
  nsCAutoString key;
  GetLayoutHistoryAndKey(aContent, getter_AddRefs(history), key);

  if (history) {
    // Get the pres state for this key, if it doesn't exist, create one
    result = history->GetState(key, aPresState);
    if (!*aPresState) {
      result = NS_NewPresState(aPresState);
      if (NS_SUCCEEDED(result)) {
        result = history->AddState(key, *aPresState);
      }
    }
  }

  return result;
}


nsresult
nsGenericHTMLElement::GetLayoutHistoryAndKey(nsGenericHTMLElement* aContent,
                                             nsILayoutHistoryState** aHistory,
                                             nsACString& aKey)
{
  //
  // Get the pres shell
  //
  nsCOMPtr<nsIDocument> doc = aContent->GetDocument();
  if (!doc) {
    return NS_OK;
  }

  //
  // Get the history (don't bother with the key if the history is not there)
  //
  *aHistory = doc->GetLayoutHistoryState().get();
  if (!*aHistory) {
    return NS_OK;
  }

  //
  // Get the state key
  //
  nsresult rv = nsContentUtils::GenerateStateKey(aContent, doc,
                                                 nsIStatefulFrame::eNoID,
                                                 aKey);
  if (NS_FAILED(rv)) {
    NS_RELEASE(*aHistory);
    return rv;
  }

  // If the state key is blank, this is anonymous content or for
  // whatever reason we are not supposed to save/restore state.
  if (aKey.IsEmpty()) {
    NS_RELEASE(*aHistory);
    return NS_OK;
  }

  // Add something unique to content so layout doesn't muck us up
  aKey += "-C";

  return rv;
}

PRBool
nsGenericHTMLElement::RestoreFormControlState(nsGenericHTMLElement* aContent,
                                              nsIFormControl* aControl)
{
  nsCOMPtr<nsILayoutHistoryState> history;
  nsCAutoString key;
  nsresult rv = GetLayoutHistoryAndKey(aContent, getter_AddRefs(history), key);
  if (!history) {
    return PR_FALSE;
  }

  nsPresState *state;
  // Get the pres state for this key
  rv = history->GetState(key, &state);
  if (state) {
    PRBool result = aControl->RestoreState(state);
    history->RemoveState(key);
    return result;
  }

  return PR_FALSE;
}

// XXX This creates a dependency between content and frames
nsPresContext*
nsGenericHTMLElement::GetPresContext()
{
  // Get the document
  nsIDocument* doc = GetDocument();
  if (doc) {
    // Get presentation shell 0
    nsIPresShell *presShell = doc->GetShellAt(0);
    if (presShell) {
      return presShell->GetPresContext();
    }
  }

  return nsnull;
}

static const nsAttrValue::EnumTable kAlignTable[] = {
  { "left",      NS_STYLE_TEXT_ALIGN_LEFT },
  { "right",     NS_STYLE_TEXT_ALIGN_RIGHT },

  { "top",       NS_STYLE_VERTICAL_ALIGN_TOP },
  { "middle",    NS_STYLE_VERTICAL_ALIGN_MIDDLE_WITH_BASELINE },
  { "bottom",    NS_STYLE_VERTICAL_ALIGN_BASELINE },

  { "center",    NS_STYLE_VERTICAL_ALIGN_MIDDLE_WITH_BASELINE },
  { "baseline",  NS_STYLE_VERTICAL_ALIGN_BASELINE },

  { "texttop",   NS_STYLE_VERTICAL_ALIGN_TEXT_TOP },
  { "absmiddle", NS_STYLE_VERTICAL_ALIGN_MIDDLE },
  { "abscenter", NS_STYLE_VERTICAL_ALIGN_MIDDLE },
  { "absbottom", NS_STYLE_VERTICAL_ALIGN_BOTTOM },
  { 0 }
};

static const nsAttrValue::EnumTable kDivAlignTable[] = {
  { "left", NS_STYLE_TEXT_ALIGN_MOZ_LEFT },
  { "right", NS_STYLE_TEXT_ALIGN_MOZ_RIGHT },
  { "center", NS_STYLE_TEXT_ALIGN_MOZ_CENTER },
  { "middle", NS_STYLE_TEXT_ALIGN_MOZ_CENTER },
  { "justify", NS_STYLE_TEXT_ALIGN_JUSTIFY },
  { 0 }
};

static const nsAttrValue::EnumTable kFrameborderTable[] = {
  { "yes", NS_STYLE_FRAME_YES },
  { "no", NS_STYLE_FRAME_NO },
  { "1", NS_STYLE_FRAME_1 },
  { "0", NS_STYLE_FRAME_0 },
  { 0 }
};

static const nsAttrValue::EnumTable kScrollingTable[] = {
  { "yes", NS_STYLE_FRAME_YES },
  { "no", NS_STYLE_FRAME_NO },
  { "on", NS_STYLE_FRAME_ON },
  { "off", NS_STYLE_FRAME_OFF },
  { "scroll", NS_STYLE_FRAME_SCROLL },
  { "noscroll", NS_STYLE_FRAME_NOSCROLL },
  { "auto", NS_STYLE_FRAME_AUTO },
  { 0 }
};

static const nsAttrValue::EnumTable kTableVAlignTable[] = {
  { "top",     NS_STYLE_VERTICAL_ALIGN_TOP },
  { "middle",  NS_STYLE_VERTICAL_ALIGN_MIDDLE },
  { "bottom",  NS_STYLE_VERTICAL_ALIGN_BOTTOM },
  { "baseline",NS_STYLE_VERTICAL_ALIGN_BASELINE },
  { 0 }
};

PRBool
nsGenericHTMLElement::ParseAlignValue(const nsAString& aString,
                                      nsAttrValue& aResult)
{
  return aResult.ParseEnumValue(aString, kAlignTable);
}

//----------------------------------------

// Vanilla table as defined by the html4 spec...
static const nsAttrValue::EnumTable kTableHAlignTable[] = {
  { "left",   NS_STYLE_TEXT_ALIGN_LEFT },
  { "right",  NS_STYLE_TEXT_ALIGN_RIGHT },
  { "center", NS_STYLE_TEXT_ALIGN_CENTER },
  { "char",   NS_STYLE_TEXT_ALIGN_CHAR },
  { "justify",NS_STYLE_TEXT_ALIGN_JUSTIFY },
  { 0 }
};

// This table is used for TABLE when in compatability mode
static const nsAttrValue::EnumTable kCompatTableHAlignTable[] = {
  { "left",   NS_STYLE_TEXT_ALIGN_LEFT },
  { "right",  NS_STYLE_TEXT_ALIGN_RIGHT },
  { "center", NS_STYLE_TEXT_ALIGN_CENTER },
  { "char",   NS_STYLE_TEXT_ALIGN_CHAR },
  { "justify",NS_STYLE_TEXT_ALIGN_JUSTIFY },
  { "abscenter", NS_STYLE_TEXT_ALIGN_CENTER },
  { "absmiddle", NS_STYLE_TEXT_ALIGN_CENTER },
  { "middle", NS_STYLE_TEXT_ALIGN_CENTER },
  { 0 }
};

PRBool
nsGenericHTMLElement::ParseTableHAlignValue(const nsAString& aString,
                                            nsAttrValue& aResult) const
{
  if (InNavQuirksMode(GetOwnerDoc())) {
    return aResult.ParseEnumValue(aString, kCompatTableHAlignTable);
  }
  return aResult.ParseEnumValue(aString, kTableHAlignTable);
}

//----------------------------------------

// These tables are used for TD,TH,TR, etc (but not TABLE)
static const nsAttrValue::EnumTable kTableCellHAlignTable[] = {
  { "left",   NS_STYLE_TEXT_ALIGN_MOZ_LEFT },
  { "right",  NS_STYLE_TEXT_ALIGN_MOZ_RIGHT },
  { "center", NS_STYLE_TEXT_ALIGN_MOZ_CENTER },
  { "char",   NS_STYLE_TEXT_ALIGN_CHAR },
  { "justify",NS_STYLE_TEXT_ALIGN_JUSTIFY },
  { 0 }
};

static const nsAttrValue::EnumTable kCompatTableCellHAlignTable[] = {
  { "left",   NS_STYLE_TEXT_ALIGN_MOZ_LEFT },
  { "right",  NS_STYLE_TEXT_ALIGN_MOZ_RIGHT },
  { "center", NS_STYLE_TEXT_ALIGN_MOZ_CENTER },
  { "char",   NS_STYLE_TEXT_ALIGN_CHAR },
  { "justify",NS_STYLE_TEXT_ALIGN_JUSTIFY },

  // The following are non-standard but necessary for Nav4 compatibility
  { "middle", NS_STYLE_TEXT_ALIGN_MOZ_CENTER },
  // allow center and absmiddle to map to NS_STYLE_TEXT_ALIGN_CENTER and
  // NS_STYLE_TEXT_ALIGN_CENTER to map to center by using the following order
  { "center", NS_STYLE_TEXT_ALIGN_CENTER },
  { "absmiddle", NS_STYLE_TEXT_ALIGN_CENTER },
  { 0 }
};

PRBool
nsGenericHTMLElement::ParseTableCellHAlignValue(const nsAString& aString,
                                                nsAttrValue& aResult) const
{
  if (InNavQuirksMode(GetOwnerDoc())) {
    return aResult.ParseEnumValue(aString, kCompatTableCellHAlignTable);
  }
  return aResult.ParseEnumValue(aString, kTableCellHAlignTable);
}

//----------------------------------------

PRBool
nsGenericHTMLElement::ParseTableVAlignValue(const nsAString& aString,
                                            nsAttrValue& aResult)
{
  return aResult.ParseEnumValue(aString, kTableVAlignTable);
}

PRBool
nsGenericHTMLElement::ParseDivAlignValue(const nsAString& aString,
                                         nsAttrValue& aResult) const
{
  return aResult.ParseEnumValue(aString, kDivAlignTable);
}

PRBool
nsGenericHTMLElement::ParseImageAttribute(nsIAtom* aAttribute,
                                          const nsAString& aString,
                                          nsAttrValue& aResult)
{
  if ((aAttribute == nsHTMLAtoms::width) ||
      (aAttribute == nsHTMLAtoms::height)) {
    return aResult.ParseSpecialIntValue(aString, PR_TRUE, PR_FALSE);
  }
  else if ((aAttribute == nsHTMLAtoms::hspace) ||
           (aAttribute == nsHTMLAtoms::vspace) ||
           (aAttribute == nsHTMLAtoms::border)) {
    return aResult.ParseIntWithBounds(aString, 0);
  }
  return PR_FALSE;
}

PRBool
nsGenericHTMLElement::ParseFrameborderValue(const nsAString& aString,
                                            nsAttrValue& aResult)
{
  return aResult.ParseEnumValue(aString, kFrameborderTable);
}

PRBool
nsGenericHTMLElement::ParseScrollingValue(const nsAString& aString,
                                          nsAttrValue& aResult)
{
  return aResult.ParseEnumValue(aString, kScrollingTable);
}

nsresult
nsGenericHTMLElement::ReparseStyleAttribute()
{
  const nsAttrValue* oldVal = mAttrsAndChildren.GetAttr(nsHTMLAtoms::style);
  
  if (oldVal && oldVal->Type() != nsAttrValue::eCSSStyleRule) {
    nsAttrValue attrValue;
    nsAutoString stringValue;
    oldVal->ToString(stringValue);
    ParseStyleAttribute(this, mNodeInfo->NamespaceEquals(kNameSpaceID_XHTML),
                        stringValue, attrValue);
    // Don't bother going through SetInlineStyleRule, we don't want to fire off
    // mutation events or document notifications anyway
    nsresult rv = mAttrsAndChildren.SetAndTakeAttr(nsHTMLAtoms::style, attrValue);
    NS_ENSURE_SUCCESS(rv, rv);
  }
  
  return NS_OK;
}

void
nsGenericHTMLElement::ParseStyleAttribute(nsIContent* aContent,
                                          PRBool aCaseSensitive,
                                          const nsAString& aValue,
                                          nsAttrValue& aResult)
{
  nsresult result = NS_OK;
  nsIDocument* doc = aContent->GetOwnerDoc();

  if (doc) {
    PRBool isCSS = PR_TRUE; // assume CSS until proven otherwise

    if (!aContent->IsNativeAnonymous()) {  // native anonymous content
                                           // always assumes CSS
      nsAutoString styleType;
      doc->GetHeaderData(nsHTMLAtoms::headerContentStyleType, styleType);
      if (!styleType.IsEmpty()) {
        static const char textCssStr[] = "text/css";
        isCSS = (styleType.EqualsIgnoreCase(textCssStr, sizeof(textCssStr) - 1));
      }
    }

    if (isCSS) {
      nsICSSLoader* cssLoader = doc->CSSLoader();
      nsCOMPtr<nsICSSParser> cssParser;
      result = cssLoader->GetParserFor(nsnull, getter_AddRefs(cssParser));
      if (cssParser) {
        nsCOMPtr<nsIURI> baseURI = aContent->GetBaseURI();

        nsCOMPtr<nsICSSStyleRule> rule;
        result = cssParser->ParseStyleAttribute(aValue, doc->GetDocumentURI(),
                                                baseURI,
                                                getter_AddRefs(rule));
        cssLoader->RecycleParser(cssParser);

        if (rule) {
          aResult.SetTo(rule);

          return;
        }
      }
    }
  }

  aResult.SetTo(aValue);
}

/**
 * Handle attributes common to all html elements
 */
void
nsGenericHTMLElement::MapCommonAttributesInto(const nsMappedAttributes* aAttributes,
                                              nsRuleData* aData)
{
  if (aData->mSID == eStyleStruct_Visibility) {
    const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::lang);
    if (value && value->Type() == nsAttrValue::eString) {
      aData->mDisplayData->mLang.SetStringValue(value->GetStringValue(),
                                                eCSSUnit_String);
    }
  }
}



/* static */ const nsGenericHTMLElement::MappedAttributeEntry
nsGenericHTMLElement::sCommonAttributeMap[] = {
  { &nsHTMLAtoms::lang },
  { nsnull }
};

/* static */ const nsGenericElement::MappedAttributeEntry
nsGenericHTMLElement::sImageMarginSizeAttributeMap[] = {
  { &nsHTMLAtoms::width },
  { &nsHTMLAtoms::height },
  { &nsHTMLAtoms::hspace },
  { &nsHTMLAtoms::vspace },
  { nsnull }
};

/* static */ const nsGenericElement::MappedAttributeEntry
nsGenericHTMLElement::sImageAlignAttributeMap[] = {
  { &nsHTMLAtoms::align },
  { nsnull }
};

/* static */ const nsGenericElement::MappedAttributeEntry
nsGenericHTMLElement::sDivAlignAttributeMap[] = {
  { &nsHTMLAtoms::align },
  { nsnull }
};

/* static */ const nsGenericElement::MappedAttributeEntry
nsGenericHTMLElement::sImageBorderAttributeMap[] = {
  { &nsHTMLAtoms::border },
  { nsnull }
};

/* static */ const nsGenericElement::MappedAttributeEntry
nsGenericHTMLElement::sBackgroundAttributeMap[] = {
  { &nsHTMLAtoms::background },
  { &nsHTMLAtoms::bgcolor },
  { nsnull }
};

/* static */ const nsGenericElement::MappedAttributeEntry
nsGenericHTMLElement::sBackgroundColorAttributeMap[] = {
  { &nsHTMLAtoms::bgcolor },
  { nsnull }
};

/* static */ const nsGenericElement::MappedAttributeEntry
nsGenericHTMLElement::sScrollingAttributeMap[] = {
  { &nsHTMLAtoms::scrolling },
  { nsnull }
};

void
nsGenericHTMLElement::MapImageAlignAttributeInto(const nsMappedAttributes* aAttributes,
                                                 nsRuleData* aRuleData)
{
  if (aRuleData->mSID == eStyleStruct_Display || aRuleData->mSID == eStyleStruct_TextReset) {
    const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::align);
    if (value && value->Type() == nsAttrValue::eEnum) {
      PRInt32 align = value->GetEnumValue();
      if (aRuleData->mSID == eStyleStruct_Display && aRuleData->mDisplayData->mFloat.GetUnit() == eCSSUnit_Null) {
        if (align == NS_STYLE_TEXT_ALIGN_LEFT)
          aRuleData->mDisplayData->mFloat.SetIntValue(NS_STYLE_FLOAT_LEFT, eCSSUnit_Enumerated);
        else if (align == NS_STYLE_TEXT_ALIGN_RIGHT)
          aRuleData->mDisplayData->mFloat.SetIntValue(NS_STYLE_FLOAT_RIGHT, eCSSUnit_Enumerated);
      }
      else if (aRuleData->mSID == eStyleStruct_TextReset && aRuleData->mTextData->mVerticalAlign.GetUnit() == eCSSUnit_Null) {
        switch (align) {
        case NS_STYLE_TEXT_ALIGN_LEFT:
        case NS_STYLE_TEXT_ALIGN_RIGHT:
          break;
        default:
          aRuleData->mTextData->mVerticalAlign.SetIntValue(align, eCSSUnit_Enumerated);
          break;
        }
      }
    }
  }
}

void
nsGenericHTMLElement::MapDivAlignAttributeInto(const nsMappedAttributes* aAttributes,
                                               nsRuleData* aRuleData)
{
  if (aRuleData->mSID == eStyleStruct_Text) {
    if (aRuleData->mTextData->mTextAlign.GetUnit() == eCSSUnit_Null) {
      // align: enum
      const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::align);
      if (value && value->Type() == nsAttrValue::eEnum)
        aRuleData->mTextData->mTextAlign.SetIntValue(value->GetEnumValue(), eCSSUnit_Enumerated);
    }
  }
}


void
nsGenericHTMLElement::MapImageMarginAttributeInto(const nsMappedAttributes* aAttributes,
                                                  nsRuleData* aData)
{
  if (aData->mSID != eStyleStruct_Margin)
    return;

  const nsAttrValue* value;

  // hspace: value
  value = aAttributes->GetAttr(nsHTMLAtoms::hspace);
  if (value) {
    nsCSSValue hval;
    if (value->Type() == nsAttrValue::eInteger)
      hval.SetFloatValue((float)value->GetIntegerValue(), eCSSUnit_Pixel);
    else if (value->Type() == nsAttrValue::ePercent)
      hval.SetPercentValue(value->GetPercentValue());

    if (hval.GetUnit() != eCSSUnit_Null) {
      nsCSSRect& margin = aData->mMarginData->mMargin;
      if (margin.mLeft.GetUnit() == eCSSUnit_Null)
        margin.mLeft = hval;
      if (margin.mRight.GetUnit() == eCSSUnit_Null)
        margin.mRight = hval;
    }
  }

  // vspace: value
  value = aAttributes->GetAttr(nsHTMLAtoms::vspace);
  if (value) {
    nsCSSValue vval;
    if (value->Type() == nsAttrValue::eInteger)
      vval.SetFloatValue((float)value->GetIntegerValue(), eCSSUnit_Pixel);
    else if (value->Type() == nsAttrValue::ePercent)
      vval.SetPercentValue(value->GetPercentValue());
  
    if (vval.GetUnit() != eCSSUnit_Null) {
      nsCSSRect& margin = aData->mMarginData->mMargin;
      if (margin.mTop.GetUnit() == eCSSUnit_Null)
        margin.mTop = vval;
      if (margin.mBottom.GetUnit() == eCSSUnit_Null)
        margin.mBottom = vval;
    }
  }
}

void
nsGenericHTMLElement::MapImageSizeAttributesInto(const nsMappedAttributes* aAttributes,
                                                 nsRuleData* aData)
{
  if (aData->mSID != eStyleStruct_Position)
    return;

  // width: value
  if (aData->mPositionData->mWidth.GetUnit() == eCSSUnit_Null) {
    const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::width);
    if (value && value->Type() == nsAttrValue::eInteger)
      aData->mPositionData->mWidth.SetFloatValue((float)value->GetIntegerValue(), eCSSUnit_Pixel);
    else if (value && value->Type() == nsAttrValue::ePercent)
      aData->mPositionData->mWidth.SetPercentValue(value->GetPercentValue());
  }

  // height: value
  if (aData->mPositionData->mHeight.GetUnit() == eCSSUnit_Null) {
    const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::height);
    if (value && value->Type() == nsAttrValue::eInteger)
      aData->mPositionData->mHeight.SetFloatValue((float)value->GetIntegerValue(), eCSSUnit_Pixel); 
    else if (value && value->Type() == nsAttrValue::ePercent)
      aData->mPositionData->mHeight.SetPercentValue(value->GetPercentValue());    
  }
}

void
nsGenericHTMLElement::MapImageBorderAttributeInto(const nsMappedAttributes* aAttributes,
                                                  nsRuleData* aData)
{
  if (aData->mSID != eStyleStruct_Border)
    return;

  // border: pixels
  const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::border);
  if (!value)
    return;
  
  nscoord val = 0;
  if (value->Type() == nsAttrValue::eInteger)
    val = value->GetIntegerValue();

  nsCSSRect& borderWidth = aData->mMarginData->mBorderWidth;
  if (borderWidth.mLeft.GetUnit() == eCSSUnit_Null)
    borderWidth.mLeft.SetFloatValue((float)val, eCSSUnit_Pixel);
  if (borderWidth.mTop.GetUnit() == eCSSUnit_Null)
    borderWidth.mTop.SetFloatValue((float)val, eCSSUnit_Pixel);
  if (borderWidth.mRight.GetUnit() == eCSSUnit_Null)
    borderWidth.mRight.SetFloatValue((float)val, eCSSUnit_Pixel);
  if (borderWidth.mBottom.GetUnit() == eCSSUnit_Null)
    borderWidth.mBottom.SetFloatValue((float)val, eCSSUnit_Pixel);

  nsCSSRect& borderStyle = aData->mMarginData->mBorderStyle;
  if (borderStyle.mLeft.GetUnit() == eCSSUnit_Null)
    borderStyle.mLeft.SetIntValue(NS_STYLE_BORDER_STYLE_SOLID, eCSSUnit_Enumerated);
  if (borderStyle.mTop.GetUnit() == eCSSUnit_Null)
    borderStyle.mTop.SetIntValue(NS_STYLE_BORDER_STYLE_SOLID, eCSSUnit_Enumerated);
  if (borderStyle.mRight.GetUnit() == eCSSUnit_Null)
    borderStyle.mRight.SetIntValue(NS_STYLE_BORDER_STYLE_SOLID, eCSSUnit_Enumerated);
  if (borderStyle.mBottom.GetUnit() == eCSSUnit_Null)
    borderStyle.mBottom.SetIntValue(NS_STYLE_BORDER_STYLE_SOLID, eCSSUnit_Enumerated);

  nsCSSRect& borderColor = aData->mMarginData->mBorderColor;
  if (borderColor.mLeft.GetUnit() == eCSSUnit_Null)
    borderColor.mLeft.SetIntValue(NS_STYLE_COLOR_MOZ_USE_TEXT_COLOR, eCSSUnit_Enumerated);
  if (borderColor.mTop.GetUnit() == eCSSUnit_Null)
    borderColor.mTop.SetIntValue(NS_STYLE_COLOR_MOZ_USE_TEXT_COLOR, eCSSUnit_Enumerated);
  if (borderColor.mRight.GetUnit() == eCSSUnit_Null)
    borderColor.mRight.SetIntValue(NS_STYLE_COLOR_MOZ_USE_TEXT_COLOR, eCSSUnit_Enumerated);
  if (borderColor.mBottom.GetUnit() == eCSSUnit_Null)
    borderColor.mBottom.SetIntValue(NS_STYLE_COLOR_MOZ_USE_TEXT_COLOR, eCSSUnit_Enumerated);
}

void
nsGenericHTMLElement::MapBackgroundInto(const nsMappedAttributes* aAttributes,
                                                  nsRuleData* aData)
{
  if (aData->mSID != eStyleStruct_Background)
    return;

  if (aData->mColorData->mBackImage.GetUnit() == eCSSUnit_Null) {
    // background
    const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::background);
    if (value && value->Type() == nsAttrValue::eString) {
      nsAutoString spec(value->GetStringValue());
      if (!spec.IsEmpty()) {
        // Resolve url to an absolute url
        // XXX this breaks if the HTML element has an xml:base
        // attribute (the xml:base will not be taken into account)
        // as well as elements with _baseHref set. We need to be able
        // to get to the element somehow, or store the base URI in the
        // attributes.
        nsIDocument* doc = aData->mPresContext->Document();
        nsCOMPtr<nsIURI> uri;
        nsresult rv = nsContentUtils::NewURIWithDocumentCharset(
            getter_AddRefs(uri), spec, doc, doc->GetBaseURI());
        if (NS_SUCCEEDED(rv)) {
          nsCSSValue::Image *img =
            new nsCSSValue::Image(uri, spec.get(), doc->GetDocumentURI(),
                                  doc, PR_TRUE);
          if (img) {
            if (img->mString) {
              aData->mColorData->mBackImage.SetImageValue(img);
            }
            else
              delete img;
          }
        }
      }
      else if (aData->mPresContext->CompatibilityMode() ==
               eCompatibility_NavQuirks) {
        // in NavQuirks mode, allow the empty string to set the
        // background to empty
        aData->mColorData->mBackImage.SetNoneValue();
      }
    }
  }
}

void
nsGenericHTMLElement::MapBGColorInto(const nsMappedAttributes* aAttributes,
                                     nsRuleData* aData)
{
  if (aData->mSID != eStyleStruct_Background)
    return;

  if (aData->mColorData->mBackColor.GetUnit() == eCSSUnit_Null) {
    const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::bgcolor);
    nscolor color;
    if (value && value->GetColorValue(color)) {
      aData->mColorData->mBackColor.SetColorValue(color);
    }
  }
}

void
nsGenericHTMLElement::MapBackgroundAttributesInto(const nsMappedAttributes* aAttributes,
                                                  nsRuleData* aData)
{
  MapBackgroundInto(aAttributes, aData);
  MapBGColorInto(aAttributes, aData);
}

void
nsGenericHTMLElement::MapScrollingAttributeInto(const nsMappedAttributes* aAttributes,
                                                nsRuleData* aData)
{
  if (aData->mSID != eStyleStruct_Display)
    return;

  // scrolling
  nsCSSValue* overflowValues[2] = {
    &aData->mDisplayData->mOverflowX,
    &aData->mDisplayData->mOverflowY,
  };
  for (PRUint32 i = 0; i < NS_ARRAY_LENGTH(overflowValues); ++i) {
    if (overflowValues[i]->GetUnit() == eCSSUnit_Null) {
      const nsAttrValue* value = aAttributes->GetAttr(nsHTMLAtoms::scrolling);
      if (value && value->Type() == nsAttrValue::eEnum) {
        PRInt32 mappedValue;
        switch (value->GetEnumValue()) {
          case NS_STYLE_FRAME_ON:
          case NS_STYLE_FRAME_SCROLL:
          case NS_STYLE_FRAME_YES:
            mappedValue = NS_STYLE_OVERFLOW_SCROLL;
            break;

          case NS_STYLE_FRAME_OFF:
          case NS_STYLE_FRAME_NOSCROLL:
          case NS_STYLE_FRAME_NO:
            mappedValue = NS_STYLE_OVERFLOW_HIDDEN;
            break;
        
          case NS_STYLE_FRAME_AUTO:
            mappedValue = NS_STYLE_OVERFLOW_AUTO;
            break;

          default:
            NS_NOTREACHED("unexpected value");
            mappedValue = NS_STYLE_OVERFLOW_AUTO;
            break;
        }
        overflowValues[i]->SetIntValue(mappedValue, eCSSUnit_Enumerated);
      }
    }
  }
}

//----------------------------------------------------------------------

nsresult
nsGenericHTMLElement::ReplaceContentsWithText(const nsAString& aText,
                                              PRBool aNotify)
{
  PRUint32 count = GetChildCount();
  nsresult rv = NS_OK;

  nsCOMPtr<nsIDOMText> textChild;

  if (count > 0) {
    // if we already have a DOMText child, reuse it.
    textChild = do_QueryInterface(GetChildAt(0));

    PRUint32 lastChild = textChild ? 1 : 0;
    PRUint32 i = count - 1;
    while (i-- > lastChild) {
      RemoveChildAt(i, aNotify);
    }
  }

  if (textChild) {
    rv = textChild->SetData(aText);
  } else {
    nsCOMPtr<nsITextContent> text;
    rv = NS_NewTextNode(getter_AddRefs(text), mNodeInfo->NodeInfoManager());
    NS_ENSURE_SUCCESS(rv, rv);

    text->SetText(aText, PR_TRUE);

    rv = InsertChildAt(text, 0, aNotify);
  }    
      
  return rv;
}

nsresult
nsGenericHTMLElement::GetAttrHelper(nsIAtom* aAttr, nsAString& aValue)
{
  GetAttr(kNameSpaceID_None, aAttr, aValue);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::SetAttrHelper(nsIAtom* aAttr, const nsAString& aValue)
{
  return SetAttr(kNameSpaceID_None, aAttr, aValue, PR_TRUE);
}

nsresult
nsGenericHTMLElement::GetStringAttrWithDefault(nsIAtom* aAttr,
                                               const char* aDefault,
                                               nsAString& aResult)
{
  if (!GetAttr(kNameSpaceID_None, aAttr, aResult)) {
    CopyASCIItoUTF16(aDefault, aResult);
  }
  return NS_OK;
}

nsresult
nsGenericHTMLElement::SetBoolAttr(nsIAtom* aAttr, PRBool aValue)
{
  if (aValue) {
    return SetAttr(kNameSpaceID_None, aAttr, EmptyString(), PR_TRUE);
  }

  return UnsetAttr(kNameSpaceID_None, aAttr, PR_TRUE);
}

nsresult
nsGenericHTMLElement::GetBoolAttr(nsIAtom* aAttr, PRBool* aValue) const
{
  *aValue = HasAttr(kNameSpaceID_None, aAttr);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetIntAttr(nsIAtom* aAttr, PRInt32 aDefault, PRInt32* aResult)
{
  const nsAttrValue* attrVal = mAttrsAndChildren.GetAttr(aAttr);
  if (attrVal && attrVal->Type() == nsAttrValue::eInteger) {
    *aResult = attrVal->GetIntegerValue();
  }
  else {
    *aResult = aDefault;
  }
  return NS_OK;
}

nsresult
nsGenericHTMLElement::SetIntAttr(nsIAtom* aAttr, PRInt32 aValue)
{
  nsAutoString value;
  value.AppendInt(aValue);

  return SetAttr(kNameSpaceID_None, aAttr, value, PR_TRUE);
}

nsresult
nsGenericHTMLElement::GetURIAttr(nsIAtom* aAttr, nsAString& aResult)
{
  nsAutoString attrValue;
  if (!GetAttr(kNameSpaceID_None, aAttr, attrValue)) {
    aResult.Truncate();

    return NS_OK;
  }

  nsCOMPtr<nsIURI> baseURI = GetBaseURI();
  nsCOMPtr<nsIURI> attrURI;
  nsresult rv =
    nsContentUtils::NewURIWithDocumentCharset(getter_AddRefs(attrURI),
                                              attrValue, GetOwnerDoc(),
                                              baseURI);
  if (NS_FAILED(rv)) {
    // Just use the attr value as the result...
    aResult = attrValue;

    return NS_OK;
  }

  NS_ASSERTION(attrURI,
               "nsContentUtils::NewURIWithDocumentCharset return value lied");

  nsCAutoString spec;
  attrURI->GetSpec(spec);
  CopyUTF8toUTF16(spec, aResult);
  return NS_OK;
}

nsresult
nsGenericHTMLElement::GetURIListAttr(nsIAtom* aAttr, nsAString& aResult)
{
  aResult.Truncate();

  nsAutoString value;
  if (!GetAttr(kNameSpaceID_None, aAttr, value))
    return NS_OK;

  nsIDocument* doc = GetOwnerDoc(); 
  nsCOMPtr<nsIURI> baseURI = GetBaseURI();

  // Value contains relative URIs split on spaces (U+0020)
  const PRUnichar *start = value.BeginReading();
  const PRUnichar *end   = value.EndReading();
  const PRUnichar *iter  = start;
  for (;;) {
    if (iter < end && *iter != ' ') {
      ++iter;
    } else {  // iter is pointing at either end or a space
      while (*start == ' ' && start < iter)
        ++start;
      if (iter != start) {
        if (!aResult.IsEmpty())
          aResult.Append(PRUnichar(' '));
        const nsSubstring& uriPart = Substring(start, iter);
        nsCOMPtr<nsIURI> attrURI;
        nsContentUtils::NewURIWithDocumentCharset(getter_AddRefs(attrURI),
                                                  uriPart, doc, baseURI);
        if (attrURI) {
          nsCAutoString spec;
          attrURI->GetSpec(spec);
          AppendUTF8toUTF16(spec, aResult);
        } else {
          aResult.Append(uriPart);
        }
      }
      start = iter = iter + 1;
      if (iter >= end)
        break;
    }
  }

  return NS_OK;
}

//----------------------------------------------------------------------

NS_IMPL_INT_ATTR_DEFAULT_VALUE(nsGenericHTMLFrameElement, TabIndex, tabindex, 0)

nsGenericHTMLFormElement::nsGenericHTMLFormElement(nsINodeInfo *aNodeInfo)
  : nsGenericHTMLElement(aNodeInfo)
{
  mForm = nsnull;
}

nsGenericHTMLFormElement::~nsGenericHTMLFormElement()
{
  // Clean up.  Set the form to nsnull so it knows we went away.
  SetForm(nsnull);
}

NS_INTERFACE_MAP_BEGIN(nsGenericHTMLFormElement)
  NS_INTERFACE_MAP_ENTRY(nsIFormControl)
NS_INTERFACE_MAP_END_INHERITING(nsGenericHTMLElement)


PRBool
nsGenericHTMLFormElement::IsNodeOfType(PRUint32 aFlags) const
{
  return !(aFlags & ~(eCONTENT | eELEMENT | eHTML | eHTML_FORM_CONTROL));
}

NS_IMETHODIMP
nsGenericHTMLFormElement::SetForm(nsIDOMHTMLFormElement* aForm,
                                  PRBool aRemoveFromForm)
{
  nsAutoString nameVal, idVal;

  if (aForm || (mForm && aRemoveFromForm)) {
    GetAttr(kNameSpaceID_None, nsHTMLAtoms::name, nameVal);
    GetAttr(kNameSpaceID_None, nsHTMLAtoms::id, idVal);
  }

  if (mForm && aRemoveFromForm) {
    mForm->RemoveElement(this);

    if (!nameVal.IsEmpty()) {
      mForm->RemoveElementFromTable(this, nameVal);
    }

    if (!idVal.IsEmpty()) {
      mForm->RemoveElementFromTable(this, idVal);
    }
  }

  if (aForm) {
    // keep a *weak* ref to the form here
    CallQueryInterface(aForm, &mForm);
    mForm->Release();
  } else {
    mForm = nsnull;
  }

  if (mForm) {
    mForm->AddElement(this);

    if (!nameVal.IsEmpty()) {
      mForm->AddElementToTable(this, nameVal);
    }

    if (!idVal.IsEmpty()) {
      mForm->AddElementToTable(this, idVal);
    }
  }

  return NS_OK;
}

NS_IMETHODIMP
nsGenericHTMLFormElement::GetForm(nsIDOMHTMLFormElement** aForm)
{
  NS_ENSURE_ARG_POINTER(aForm);
  *aForm = nsnull;

  if (mForm) {
    CallQueryInterface(mForm, aForm);
  }

  return NS_OK;
}

PRUint32
nsGenericHTMLFormElement::GetDesiredIMEState()
{
  nsCOMPtr<nsIEditor> editor = nsnull;
  nsresult rv = GetEditorInternal(getter_AddRefs(editor));
  if (NS_FAILED(rv) || !editor)
    return nsGenericHTMLElement::GetDesiredIMEState();
  nsCOMPtr<nsIEditorIMESupport> imeEditor = do_QueryInterface(editor);
  if (!imeEditor)
    return nsGenericHTMLElement::GetDesiredIMEState();
  PRUint32 state;
  rv = imeEditor->GetPreferredIMEState(&state);
  if (NS_FAILED(rv))
    return nsGenericHTMLElement::GetDesiredIMEState();
  return state;
}

PRBool
nsGenericHTMLFrameElement::IsFocusable(PRInt32 *aTabIndex)
{
  if (!nsGenericHTMLElement::IsFocusable(aTabIndex)) {
    return PR_FALSE;
  }

  // If there is no subdocument, docshell or content viewer, it's not tabbable
  PRBool isFocusable = PR_FALSE;
  nsIDocument *doc = GetCurrentDoc();
  if (doc) {
    // XXXbz should this use GetOwnerDoc() for GetSubDocumentFor?
    // sXBL/XBL2 issue!
    nsIDocument *subDoc = doc->GetSubDocumentFor(this);
    if (subDoc) {
      nsCOMPtr<nsISupports> container = subDoc->GetContainer();
      nsCOMPtr<nsIDocShell> docShell(do_QueryInterface(container));
      if (docShell) {
        nsCOMPtr<nsIContentViewer> contentViewer;
        docShell->GetContentViewer(getter_AddRefs(contentViewer));
        if (contentViewer) {
          isFocusable = PR_TRUE;
          nsCOMPtr<nsIContentViewer> zombieViewer;
          contentViewer->GetPreviousViewer(getter_AddRefs(zombieViewer));
          if (zombieViewer) {
            // If there are 2 viewers for the current docshell, that 
            // means the current document is a zombie document.
            // Only navigate into the frame/iframe if it's not a zombie.
            isFocusable = PR_FALSE;
          }
        }
      }
    }
  }

  if (!isFocusable && aTabIndex) {
    *aTabIndex = -1;
  }

  return isFocusable;
}

nsresult
nsGenericHTMLFormElement::BindToTree(nsIDocument* aDocument,
                                     nsIContent* aParent,
                                     nsIContent* aBindingParent,
                                     PRBool aCompileEventHandlers)
{
  nsresult rv = nsGenericHTMLElement::BindToTree(aDocument, aParent,
                                                 aBindingParent,
                                                 aCompileEventHandlers);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!mForm && aParent) {
    // We now have a parent, so we may have picked up an ancestor form.  Search
    // for it.  Note that if mForm is already set we don't want to do this,
    // because that means someone (probably the content sink) has already set
    // it to the right value.  Also note that even if being bound here didn't
    // change our parent, we still need to search, since our parent chain
    // probably changed _somewhere_.
    FindAndSetForm();
  }
    
  return rv;
}

void
nsGenericHTMLFormElement::UnbindFromTree(PRBool aDeep, PRBool aNullParent)
{
  // Save state before doing anything
  SaveState();

  if (mForm) {
    // Might need to unset mForm
    if (aNullParent) {
      // No more parent means no more form
      SetForm(nsnull);
    } else {
      // Recheck whether we should still have an mForm.
      nsCOMPtr<nsIDOMHTMLFormElement> form = FindForm(mForm);
      if (!form) {
        SetForm(nsnull);
      }
    }
  }

  nsGenericHTMLElement::UnbindFromTree(aDeep, aNullParent);
}

nsresult
nsGenericHTMLFormElement::BeforeSetAttr(PRInt32 aNameSpaceID, nsIAtom* aName,
                                        const nsAString* aValue, PRBool aNotify)
{
  if (aNameSpaceID == kNameSpaceID_None) {
    nsAutoString tmp;

    // remove the control from the hashtable as needed

    if (mForm && (aName == nsHTMLAtoms::name || aName == nsHTMLAtoms::id)) {
      GetAttr(kNameSpaceID_None, aName, tmp);

      if (!tmp.IsEmpty()) {
        mForm->RemoveElementFromTable(this, tmp);
      }
    }

    if (mForm && aName == nsHTMLAtoms::type) {
      GetAttr(kNameSpaceID_None, nsHTMLAtoms::name, tmp);

      if (!tmp.IsEmpty()) {
        mForm->RemoveElementFromTable(this, tmp);
      }

      GetAttr(kNameSpaceID_None, nsHTMLAtoms::id, tmp);

      if (!tmp.IsEmpty()) {
        mForm->RemoveElementFromTable(this, tmp);
      }

      mForm->RemoveElement(this);
    }
  }

  return nsGenericHTMLElement::BeforeSetAttr(aNameSpaceID, aName,
                                             aValue, aNotify);
}

nsresult
nsGenericHTMLFormElement::AfterSetAttr(PRInt32 aNameSpaceID, nsIAtom* aName,
                                       const nsAString* aValue, PRBool aNotify)
{
  if (aNameSpaceID == kNameSpaceID_None) {
    // add the control to the hashtable as needed

    if (mForm && (aName == nsHTMLAtoms::name || aName == nsHTMLAtoms::id) &&
        aValue) {
      if (!aValue->IsEmpty()) {
        mForm->AddElementToTable(this, *aValue);
      }
    }

    if (mForm && aName == nsHTMLAtoms::type) {
      nsAutoString tmp;

      GetAttr(kNameSpaceID_None, nsHTMLAtoms::name, tmp);

      if (!tmp.IsEmpty()) {
        mForm->AddElementToTable(this, tmp);
      }

      GetAttr(kNameSpaceID_None, nsHTMLAtoms::id, tmp);

      if (!tmp.IsEmpty()) {
        mForm->AddElementToTable(this, tmp);
      }

      mForm->AddElement(this);
    }

    // And notify on content state changes, if any
    
    if (aNotify && aName == nsHTMLAtoms::disabled && CanBeDisabled()) {
      nsIDocument* document = GetCurrentDoc();
      if (document) {
        mozAutoDocUpdate upd(document, UPDATE_CONTENT_STATE, PR_TRUE);
        document->ContentStatesChanged(this, nsnull, NS_EVENT_STATE_DISABLED |
                                       NS_EVENT_STATE_ENABLED);
      }
    }
  }

  return nsGenericHTMLElement::AfterSetAttr(aNameSpaceID, aName,
                                            aValue, aNotify);
}

nsresult
nsGenericHTMLFormElement::UnsetAttr(PRInt32 aNameSpaceID, nsIAtom* aName,
                                    PRBool aNotify)
{
  BeforeSetAttr(aNameSpaceID, aName, nsnull, aNotify);
  
  nsresult rv = nsGenericHTMLElement::UnsetAttr(aNameSpaceID, aName, aNotify);

  AfterSetAttr(aNameSpaceID, aName, nsnull, aNotify);

  return rv;
}

PRBool
nsGenericHTMLFormElement::CanBeDisabled() const
{
  PRInt32 type = GetType();
  // It's easier to test the types that _cannot_ be disabled
  return
    type != NS_FORM_LABEL &&
    type != NS_FORM_LEGEND &&
    type != NS_FORM_FIELDSET &&
    type != NS_FORM_OBJECT;
}

void
nsGenericHTMLFormElement::FindAndSetForm()
{
  nsCOMPtr<nsIDOMHTMLFormElement> form = FindForm();
  if (form) {
    SetForm(form);  // always succeeds
  }
}

PRInt32
nsGenericHTMLFormElement::IntrinsicState() const
{
  PRInt32 state = nsGenericHTMLElement::IntrinsicState();

  if (CanBeDisabled()) {
    // :enabled/:disabled
    PRBool disabled;
    GetBoolAttr(nsHTMLAtoms::disabled, &disabled);
    if (disabled) {
      state |= NS_EVENT_STATE_DISABLED;
      state &= ~NS_EVENT_STATE_ENABLED;
    } else {
      state &= ~NS_EVENT_STATE_DISABLED;
      state |= NS_EVENT_STATE_ENABLED;
    }
  }

  return state;
}

//----------------------------------------------------------------------

nsGenericHTMLFrameElement::~nsGenericHTMLFrameElement()
{
  if (mFrameLoader) {
    mFrameLoader->Destroy();
  }
}

NS_INTERFACE_MAP_BEGIN(nsGenericHTMLFrameElement)
  NS_INTERFACE_MAP_ENTRY(nsIDOMNSHTMLFrameElement)
  NS_INTERFACE_MAP_ENTRY(nsIChromeEventHandler)
  NS_INTERFACE_MAP_ENTRY(nsIFrameLoaderOwner)
NS_INTERFACE_MAP_END_INHERITING(nsGenericHTMLElement)

nsresult
nsGenericHTMLFrameElement::GetContentDocument(nsIDOMDocument** aContentDocument)
{
  NS_PRECONDITION(aContentDocument, "Null out param");
  *aContentDocument = nsnull;

  nsCOMPtr<nsIDOMWindow> win;
  GetContentWindow(getter_AddRefs(win));

  if (!win) {
    return NS_OK;
  }

  return win->GetDocument(aContentDocument);
}

NS_IMETHODIMP
nsGenericHTMLFrameElement::GetContentWindow(nsIDOMWindow** aContentWindow)
{
  NS_PRECONDITION(aContentWindow, "Null out param");
  *aContentWindow = nsnull;

  nsresult rv = EnsureFrameLoader();
  NS_ENSURE_SUCCESS(rv, rv);

  if (!mFrameLoader) {
    return NS_OK;
  }

  PRBool depthTooGreat = PR_FALSE;
  mFrameLoader->GetDepthTooGreat(&depthTooGreat);
  if (depthTooGreat) {
    // Claim to have no contentWindow
    return NS_OK;
  }
  
  nsCOMPtr<nsIDocShell> doc_shell;
  mFrameLoader->GetDocShell(getter_AddRefs(doc_shell));

  nsCOMPtr<nsPIDOMWindow> win(do_GetInterface(doc_shell));

  if (!win) {
    return NS_OK;
  }

  NS_ASSERTION(win->IsOuterWindow(),
               "Uh, this window should always be an outer window!");

  return CallQueryInterface(win, aContentWindow);
}

nsresult
nsGenericHTMLFrameElement::EnsureFrameLoader()
{
  if (!GetParent() || !IsInDoc() || mFrameLoader) {
    // If frame loader is there, we just keep it around, cached
    return NS_OK;
  }

  mFrameLoader = new nsFrameLoader(this);
  if (!mFrameLoader)
    return NS_ERROR_OUT_OF_MEMORY;

  return NS_OK;
}

NS_IMETHODIMP
nsGenericHTMLFrameElement::GetFrameLoader(nsIFrameLoader **aFrameLoader)
{
  NS_IF_ADDREF(*aFrameLoader = mFrameLoader);
  return NS_OK;
}

nsresult
nsGenericHTMLFrameElement::LoadSrc()
{
  nsresult rv = EnsureFrameLoader();
  NS_ENSURE_SUCCESS(rv, rv);

  if (!mFrameLoader) {
    return NS_OK;
  }

  rv = mFrameLoader->LoadFrame();
#ifdef DEBUG
  if (NS_FAILED(rv)) {
    NS_WARNING("failed to load URL");
  }
#endif

  return rv;
}

nsresult
nsGenericHTMLFrameElement::BindToTree(nsIDocument* aDocument,
                                      nsIContent* aParent,
                                      nsIContent* aBindingParent,
                                      PRBool aCompileEventHandlers)
{
  nsresult rv = nsGenericHTMLElement::BindToTree(aDocument, aParent,
                                                 aBindingParent,
                                                 aCompileEventHandlers);
  NS_ENSURE_SUCCESS(rv, rv);

  if (aDocument) {
    // We're in a document now.  Kick off the frame load.
    LoadSrc();
  }
  
  return rv;
}

void
nsGenericHTMLFrameElement::UnbindFromTree(PRBool aDeep, PRBool aNullParent)
{
  if (mFrameLoader) {
    // This iframe is being taken out of the document, destroy the
    // iframe's frame loader (doing that will tear down the window in
    // this iframe).
    // XXXbz we really want to only partially destroy the frame
    // loader... we don't want to tear down the docshell.  Food for
    // later bug.
    mFrameLoader->Destroy();
    mFrameLoader = nsnull;
  }

  nsGenericHTMLElement::UnbindFromTree(aDeep, aNullParent);
}

nsresult
nsGenericHTMLFrameElement::SetAttr(PRInt32 aNameSpaceID, nsIAtom* aName,
                                   nsIAtom* aPrefix, const nsAString& aValue,
                                   PRBool aNotify)
{
  nsresult rv = nsGenericHTMLElement::SetAttr(aNameSpaceID, aName, aPrefix,
                                              aValue, aNotify);
  
  if (NS_SUCCEEDED(rv) && aNameSpaceID == kNameSpaceID_None &&
      aName == nsHTMLAtoms::src) {
    return LoadSrc();
  }

  return rv;
}

NS_IMETHODIMP
nsGenericHTMLFrameElement::PreHandleChromeEvent(nsEventChainPreVisitor& aVisitor)
{
  return PreHandleEvent(aVisitor);
}

NS_IMETHODIMP
nsGenericHTMLFrameElement::PostHandleChromeEvent(nsEventChainPostVisitor& aVisitor)
{
  return NS_OK;
}

//----------------------------------------------------------------------

void
nsGenericHTMLElement::SetElementFocus(PRBool aDoFocus)
{
  nsCOMPtr<nsPresContext> presContext = GetPresContext();
  if (!presContext)
    return;

  if (aDoFocus) {
    if (IsInDoc()) {
      // Make sure that our frames are up to date so we focus the right thing.
      GetCurrentDoc()->FlushPendingNotifications(Flush_Frames);
    }

    SetFocus(presContext);

    presContext->EventStateManager()->MoveCaretToFocus();
    return;
  }

  RemoveFocus(presContext);
}

nsresult
nsGenericHTMLElement::Blur()
{
  if (ShouldFocus(this)) {
    SetElementFocus(PR_FALSE);
  }

  return NS_OK;
}

nsresult
nsGenericHTMLElement::Focus()
{
  // Generic HTML elements are focusable only if tabindex explicitly set.
  // SetFocus() will check to see if we're focusable and then
  // call into esm to do the work of focusing.
  if (ShouldFocus(this)) {
    SetElementFocus(PR_TRUE);
  }

  return NS_OK;
}

void
nsGenericHTMLElement::RemoveFocus(nsPresContext *aPresContext)
{
  if (!aPresContext) 
    return;

  if (IsNodeOfType(eHTML_FORM_CONTROL)) {
    nsIFormControlFrame* formControlFrame = GetFormControlFrame(PR_FALSE);
    if (formControlFrame) {
      formControlFrame->SetFocus(PR_FALSE, PR_FALSE);
    }
  }
  
  if (IsInDoc()) {
    aPresContext->EventStateManager()->SetContentState(nsnull,
                                                       NS_EVENT_STATE_FOCUS);
  }
}

PRBool
nsGenericHTMLElement::IsFocusable(PRInt32 *aTabIndex)
{
  PRInt32 tabIndex = 0;   // Default value for non HTML elements with -moz-user-focus
  GetTabIndex(&tabIndex);

  // Just check for disabled attribute on all HTML elements
  PRBool disabled = HasAttr(kNameSpaceID_None, nsHTMLAtoms::disabled);
  if (disabled) {
    tabIndex = -1;
  }

  if (aTabIndex) {
    *aTabIndex = tabIndex;
  }

  // If a tabindex is specified at all, or the default tabindex is 0, we're focusable
  return tabIndex >= 0 || (!disabled && HasAttr(kNameSpaceID_None, nsHTMLAtoms::tabindex));
}

void
nsGenericHTMLElement::RegUnRegAccessKey(PRBool aDoReg)
{
  // first check to see if we have an access key
  nsAutoString accessKey;
  GetAttr(kNameSpaceID_None, nsHTMLAtoms::accesskey, accessKey);
  if (accessKey.IsEmpty()) {
    return;
  }

  // We have an access key, so get the ESM from the pres context.
  nsPresContext *presContext = GetPresContext();

  if (presContext) {
    nsIEventStateManager *esm = presContext->EventStateManager();

    // Register or unregister as appropriate.
    if (aDoReg) {
      esm->RegisterAccessKey(this, (PRUint32)accessKey.First());
    } else {
      esm->UnregisterAccessKey(this, (PRUint32)accessKey.First());
    }
  }
}

// static
nsresult
nsGenericHTMLElement::SetProtocolInHrefString(const nsAString &aHref,
                                              const nsAString &aProtocol,
                                              nsAString &aResult)
{
  aResult.Truncate();
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv))
    return rv;

  nsAString::const_iterator start, end;
  aProtocol.BeginReading(start);
  aProtocol.EndReading(end);
  nsAString::const_iterator iter(start);
  FindCharInReadable(':', iter, end);
  uri->SetScheme(NS_ConvertUTF16toUTF8(Substring(start, iter)));
   
  nsCAutoString newHref;
  uri->GetSpec(newHref);

  CopyUTF8toUTF16(newHref, aResult);

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::SetHostnameInHrefString(const nsAString &aHref,
                                              const nsAString &aHostname,
                                              nsAString &aResult)
{
  aResult.Truncate();
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv))
    return rv;

  uri->SetHost(NS_ConvertUTF16toUTF8(aHostname));

  nsCAutoString newHref;
  uri->GetSpec(newHref);

  CopyUTF8toUTF16(newHref, aResult);

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::SetPathnameInHrefString(const nsAString &aHref,
                                              const nsAString &aPathname,
                                              nsAString &aResult)
{
  aResult.Truncate();
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri, &rv));
  if (NS_FAILED(rv))
    return rv;

  url->SetFilePath(NS_ConvertUTF16toUTF8(aPathname));

  nsCAutoString newHref;
  uri->GetSpec(newHref);

  CopyUTF8toUTF16(newHref, aResult);

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::SetHostInHrefString(const nsAString &aHref,
                                          const nsAString &aHost,
                                          nsAString &aResult)
{
  // Can't simply call nsURI::SetHost, because that would treat the name as an
  // IPv6 address (like http://[server:443]/)

  aResult.Truncate();
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv))
    return rv;

  nsCAutoString scheme, userpass, path;
  uri->GetScheme(scheme);
  uri->GetUserPass(userpass);
  uri->GetPath(path);

  CopyASCIItoUTF16(scheme, aResult);
  aResult.AppendLiteral("://");
  if (!userpass.IsEmpty()) {
    AppendUTF8toUTF16(userpass, aResult);
    aResult.Append(PRUnichar('@'));
  }
  aResult.Append(aHost);
  AppendUTF8toUTF16(path, aResult);

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::SetSearchInHrefString(const nsAString &aHref,
                                            const nsAString &aSearch,
                                            nsAString &aResult)
{
  aResult.Truncate();
  nsCOMPtr<nsIURI> uri;

  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri, &rv));
  if (NS_FAILED(rv))
    return rv;

  url->SetQuery(NS_ConvertUTF16toUTF8(aSearch));

  nsCAutoString newHref;
  uri->GetSpec(newHref);

  CopyUTF8toUTF16(newHref, aResult);

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::SetHashInHrefString(const nsAString &aHref,
                                          const nsAString &aHash,
                                          nsAString &aResult)
{
  aResult.Truncate();
  nsCOMPtr<nsIURI> uri;

  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv))
    return rv;

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri, &rv));
  if (NS_FAILED(rv))
    return rv;

  rv = url->SetRef(NS_ConvertUTF16toUTF8(aHash));

  nsCAutoString newHref;
  uri->GetSpec(newHref);

  CopyUTF8toUTF16(newHref, aResult);

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::SetPortInHrefString(const nsAString &aHref,
                                          const nsAString &aPort,
                                          nsAString &aResult)
{
  aResult.Truncate();
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);

  if (NS_FAILED(rv))
    return rv;

  PRInt32 port;
  port = nsString(aPort).ToInteger((PRInt32*)&rv);
  if (NS_FAILED(rv))
    return rv;

  uri->SetPort(port);

  nsCAutoString newHref;
  uri->GetSpec(newHref);

  CopyUTF8toUTF16(newHref, aResult);

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::GetProtocolFromHrefString(const nsAString& aHref,
                                                nsAString& aProtocol,
                                                nsIDocument *aDocument)
{
  aProtocol.Truncate();

  nsIIOService* ioService = nsContentUtils::GetIOService();
  NS_ENSURE_TRUE(ioService, NS_ERROR_FAILURE);

  nsCAutoString protocol;

  nsresult rv =
    ioService->ExtractScheme(NS_ConvertUTF16toUTF8(aHref), protocol);

  if (NS_SUCCEEDED(rv)) {
    CopyASCIItoUTF16(protocol, aProtocol);
  } else {
    // set the protocol to the protocol of the base URI.

    if (aDocument) {
      nsIURI *uri = aDocument->GetBaseURI();
      if (uri) {
        uri->GetScheme(protocol);
      }
    }

    if (protocol.IsEmpty()) {
      // set the protocol to http since it is the most likely protocol
      // to be used.
      aProtocol.AssignLiteral("http");
    } else {
      CopyASCIItoUTF16(protocol, aProtocol);
    }
  }
  aProtocol.Append(PRUnichar(':'));

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::GetHostFromHrefString(const nsAString& aHref,
                                            nsAString& aHost)
{
  aHost.Truncate();
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv)) {
    if (rv == NS_ERROR_MALFORMED_URI) {
      // Don't throw from these methods!  Not a valid URI means return
      // empty string.
      rv = NS_OK;
    }
    return rv;
  }

  nsCAutoString hostport;
  rv = uri->GetHostPort(hostport);

  // Failure to get the hostport from the URI isn't necessarily an
  // error. Some URI's just don't have a hostport.

  if (NS_SUCCEEDED(rv)) {
    CopyUTF8toUTF16(hostport, aHost);
  }

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::GetHostnameFromHrefString(const nsAString& aHref,
                                                nsAString& aHostname)
{
  aHostname.Truncate();
  nsCOMPtr<nsIURI> url;
  nsresult rv = NS_NewURI(getter_AddRefs(url), aHref);
  if (NS_FAILED(rv)) {
    if (rv == NS_ERROR_MALFORMED_URI) {
      // Don't throw from these methods!  Not a valid URI means return
      // empty string.
      rv = NS_OK;
    }
    return rv;
  }

  nsCAutoString host;
  rv = url->GetHost(host);

  if (NS_SUCCEEDED(rv)) {
    // Failure to get the host from the URI isn't necessarily an
    // error. Some URI's just don't have a host.

    CopyUTF8toUTF16(host, aHostname);
  }

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::GetPathnameFromHrefString(const nsAString& aHref,
                                                nsAString& aPathname)
{
  aPathname.Truncate();
  nsCOMPtr<nsIURI> uri;

  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv)) {
    if (rv == NS_ERROR_MALFORMED_URI) {
      rv = NS_OK;
    }
    return rv;
  }

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));

  if (!url) {
    // If this is not a URL, we can't get the pathname from the URI

    return NS_OK;
  }

  nsCAutoString file;
  rv = url->GetFilePath(file);
  if (NS_FAILED(rv))
    return rv;

  CopyUTF8toUTF16(file, aPathname);

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::GetSearchFromHrefString(const nsAString& aHref,
                                              nsAString& aSearch)
{
  aSearch.Truncate();
  nsCOMPtr<nsIURI> uri;

  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv)) {
    if (rv == NS_ERROR_MALFORMED_URI) {
      rv = NS_OK;
    }
    return rv;
  }

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));

  if (!url) {
    // If this is not a URL, we can't get the query from the URI

    return NS_OK;
  }

  nsCAutoString search;
  rv = url->GetQuery(search);
  if (NS_FAILED(rv))
    return rv;

  if (!search.IsEmpty()) {
    CopyUTF8toUTF16(NS_LITERAL_CSTRING("?") + search, aSearch);
  }

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::GetPortFromHrefString(const nsAString& aHref,
                                            nsAString& aPort)
{
  aPort.Truncate();
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv)) {
    if (rv == NS_ERROR_MALFORMED_URI) {
      rv = NS_OK;
    }
    return rv;
  }

  PRInt32 port;
  rv = uri->GetPort(&port);

  if (NS_SUCCEEDED(rv)) {
    // Failure to get the port from the URI isn't necessarily an
    // error. Some URI's just don't have a port.

    if (port == -1) {
      return NS_OK;
    }

    nsAutoString portStr;
    portStr.AppendInt(port, 10);
    aPort.Assign(portStr);
  }

  return NS_OK;
}

// static
nsresult
nsGenericHTMLElement::GetHashFromHrefString(const nsAString& aHref,
                                            nsAString& aHash)
{
  aHash.Truncate();
  nsCOMPtr<nsIURI> uri;

  nsresult rv = NS_NewURI(getter_AddRefs(uri), aHref);
  if (NS_FAILED(rv)) {
    if (rv == NS_ERROR_MALFORMED_URI) {
      rv = NS_OK;
    }
    return rv;
  }

  nsCOMPtr<nsIURL> url(do_QueryInterface(uri));

  if (!url) {
    // If this is not a URL, we can't get the hash part from the URI

    return NS_OK;
  }

  nsCAutoString ref;
  rv = url->GetRef(ref);
  if (NS_FAILED(rv))
    return rv;
  NS_UnescapeURL(ref); // XXX may result in random non-ASCII bytes!

  if (!ref.IsEmpty()) {
    aHash.Assign(PRUnichar('#'));
    AppendASCIItoUTF16(ref, aHash);
  }
  return NS_OK;
}

const nsAttrName*
nsGenericHTMLElement::InternalGetExistingAttrNameFromQName(const nsAString& aStr) const
{
  if (mNodeInfo->NamespaceEquals(kNameSpaceID_None)) {
    nsAutoString lower;
    ToLowerCase(aStr, lower);
    return mAttrsAndChildren.GetExistingAttrNameFromQName(
      NS_ConvertUTF16toUTF8(lower));
  }

  return mAttrsAndChildren.GetExistingAttrNameFromQName(
    NS_ConvertUTF16toUTF8(aStr));
}

nsresult
nsGenericHTMLElement::GetEditor(nsIEditor** aEditor)
{
  *aEditor = nsnull;

  if (!nsContentUtils::IsCallerTrustedForWrite())
    return NS_ERROR_DOM_SECURITY_ERR;

  return GetEditorInternal(aEditor);
}

nsresult
nsGenericHTMLElement::GetEditorInternal(nsIEditor** aEditor)
{
  *aEditor = nsnull;

  nsIFormControlFrame *fcFrame = GetFormControlFrame(PR_FALSE);
  if (fcFrame) {
    nsITextControlFrame *textFrame = nsnull;
    CallQueryInterface(fcFrame, &textFrame);
    if (textFrame) {
      return textFrame->GetEditor(aEditor);
    }
  }

  return NS_OK;
}
