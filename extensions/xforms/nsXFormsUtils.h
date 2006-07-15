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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
 *  Allan Beaufour <abeaufour@novell.com>
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

#ifndef nsXFormsUtils_h_
#define nsXFormsUtils_h_

#include "prtypes.h"
#include "nsCOMPtr.h"
#include "nsCOMArray.h"
#include "nsIDOMNode.h"
#include "nsIDocument.h"
#include "nsIDOMXPathResult.h"
#include "nsIModelElementPrivate.h"
#include "nsIScriptError.h"
#include "nsVoidArray.h"
#include "nsIDOMWindowInternal.h"

class nsIDOMElement;
class nsIXFormsModelElement;
class nsIURI;
class nsString;
class nsIMutableArray;
class nsIDOMEvent;

#define NS_NAMESPACE_XFORMS              "http://www.w3.org/2002/xforms"
#define NS_NAMESPACE_XHTML               "http://www.w3.org/1999/xhtml"
#define NS_NAMESPACE_XML_SCHEMA          "http://www.w3.org/2001/XMLSchema"
#define NS_NAMESPACE_XML_SCHEMA_INSTANCE "http://www.w3.org/2001/XMLSchema-instance"
#define NS_NAMESPACE_MOZ_XFORMS_TYPE     "http://www.mozilla.org/projects/xforms/2005/type"
#define NS_NAMESPACE_SOAP_ENVELOPE       "http://schemas.xmlsoap.org/soap/envelope/"
#define NS_NAMESPACE_MOZ_XFORMS_LAZY     "http://www.mozilla.org/projects/xforms/2005/lazy"

/**
 * Error codes
 */

#define NS_OK_XFORMS_NOREFRESH \
NS_ERROR_GENERATE_SUCCESS(NS_ERROR_MODULE_GENERAL, 1)
#define NS_OK_XFORMS_DEFERRED \
NS_ERROR_GENERATE_SUCCESS(NS_ERROR_MODULE_GENERAL, 2)

#define NS_ERROR_XFORMS_CALCUATION_EXCEPTION \
NS_ERROR_GENERATE_FAILURE(NS_ERROR_MODULE_GENERAL, 3001)

/**
 * XForms event types
 *
 * @see http://www.w3.org/TR/xforms/slice4.html#rpm-events
 *
 * @note nsXFormsModelElement::SetSingleState() assumes a specific order of
 * the events from eEvent_Valid to eEvent_Disabled.
 */

enum nsXFormsEvent {
  eEvent_ModelConstruct,
  eEvent_ModelConstructDone,
  eEvent_Ready,
  eEvent_ModelDestruct,
  eEvent_Previous,
  eEvent_Next,
  eEvent_Focus,
  eEvent_Help,
  eEvent_Hint,
  eEvent_Rebuild,
  eEvent_Refresh,
  eEvent_Revalidate,
  eEvent_Recalculate,
  eEvent_Reset,
  eEvent_Submit,
  eEvent_DOMActivate,
  eEvent_ValueChanged,
  eEvent_Select,
  eEvent_Deselect,
  eEvent_ScrollFirst,
  eEvent_ScrollLast,
  eEvent_Insert,
  eEvent_Delete,
  eEvent_Valid,
  eEvent_Invalid,
  eEvent_DOMFocusIn,
  eEvent_DOMFocusOut,
  eEvent_Readonly,
  eEvent_Readwrite,
  eEvent_Required,
  eEvent_Optional,
  eEvent_Enabled,
  eEvent_Disabled,
  eEvent_InRange,
  eEvent_OutOfRange,
  eEvent_SubmitDone,
  eEvent_SubmitError,
  eEvent_BindingException,
  eEvent_LinkException,
  eEvent_LinkError,
  eEvent_ComputeException,
  eEvent_MozHintOff
};

struct EventData
{
  const char *name;
  PRBool      canCancel;
  PRBool      canBubble;
};

extern const EventData sXFormsEventsEntries[42];

// Default intrinsic state for XForms Controls
extern const PRInt32 kDefaultIntrinsicState;

// Disabled intrinsic state for XForms Controls
extern const PRInt32 kDisabledIntrinsicState;

/**
 * This class has static helper methods that don't fit into a specific place
 * in the class hierarchy.
 */
class nsXFormsUtils
{
public:

  /**
   * Possible values for any |aElementFlags| parameters.  These may be
   * bitwise OR'd together.
   */
  enum {
    /**
     * The element has a "model" attribute.
     */
    ELEMENT_WITH_MODEL_ATTR = 1 << 0
  };

  /**
   * Initialize nsXFormsUtils.
   */
  static NS_HIDDEN_(nsresult)
    Init();

  /**
   * Locate the model that is a parent of |aBindElement|.  This method walks
   * up the content tree looking for the containing model.
   *
   * @return                  Whether it's a reference to an outermost bind
   */
  static NS_HIDDEN_(PRBool)
    GetParentModel(nsIDOMElement           *aBindElement,
                   nsIModelElementPrivate **aModel);

  /**
   * Find the evaluation context for an element.
   *
   * That is, the model it is bound to (|aModel|), and if applicable the
   * \<bind\> element that it uses (|aBindElement| and the context node
   * (|aContextNode|).
   *
   * @param aElement          The element
   * @param aElementFlags     Flags describing characteristics of aElement
   * @param aModel            The \<model\> for the element
   * @param aBindElement      The \<bind\> the element is bound to (if any)
   * @param aOuterBind        Whether the \<bind\> is an outermost bind
   * @param aParentControl    The parent control, ie the one setting the context
   * @param aContextNode      The context node for the element
   * @param aContextPosition  The context position for the element
   * @param aContextSize      The context size for the element
   */
  static NS_HIDDEN_(nsresult)
    GetNodeContext(nsIDOMElement           *aElement,
                   PRUint32                 aElementFlags,
                   nsIModelElementPrivate **aModel,
                   nsIDOMElement          **aBindElement,
                   PRBool                  *aOuterBind,
                   nsIXFormsControl       **aParentControl,
                   nsIDOMNode             **aContextNode,
                   PRInt32                 *aContextPosition = nsnull,
                   PRInt32                 *aContextSize = nsnull);

  /**
   * Locate the model for an element.
   *
   * @note Actually it is just a shortcut for GetNodeContext().
   *
   * @param aElement          The element
   * @param aParentControl    The parent control setting the context
   * @param aElementFlags     Flags describing characteristics of aElement
   * @param aContextNode      The context node
   * @return                  The model
   */
  static NS_HIDDEN_(already_AddRefed<nsIModelElementPrivate>)
    GetModel(nsIDOMElement     *aElement,
             nsIXFormsControl **aParentControl = nsnull,
             PRUint32           aElementFlags = ELEMENT_WITH_MODEL_ATTR,
             nsIDOMNode       **aContextNode = nsnull);


  /**
   * Evaluate a 'bind' or |aBindingAttr| attribute on |aElement|.
   * |aResultType| is used as the desired result type for the XPath evaluation.
   *
   * The model element (if applicable) is located as part of this evaluation,
   * and returned (addrefed) in |aModel|
   *
   * The return value is an XPathResult as returned from
   * nsIXFormsXPathEvaluator::Evaluate().
   */
  static NS_HIDDEN_(nsresult)
    EvaluateNodeBinding(nsIDOMElement           *aElement,
                        PRUint32                 aElementFlags,
                        const nsString          &aBindingAttr,
                        const nsString          &aDefaultRef,
                        PRUint16                 aResultType,
                        nsIModelElementPrivate **aModel,
                        nsIDOMXPathResult      **aResult,
                        PRBool                  *aUsesModelBind,
                        nsIXFormsControl       **aParentControl = nsnull,
                        nsCOMArray<nsIDOMNode>  *aDeps = nsnull,
                        nsStringArray           *aIndexesUsed = nsnull);

  /**
   * Convenience method for doing XPath evaluations.  This gets a
   * nsIXFormsXPathEvaluator from |aContextNode|'s ownerDocument, and calls
   * nsIXFormsXPathEvaluator::Evalute using the given expression, context node,
   * namespace resolver, and result type.
   */
  static NS_HIDDEN_(nsresult)
    EvaluateXPath(const nsAString        &aExpression,
                  nsIDOMNode             *aContextNode,
                  nsIDOMNode             *aResolverNode,
                  PRUint16                aResultType,
                  nsIDOMXPathResult     **aResult,
                  PRInt32                 aContextPosition = 1,
                  PRInt32                 aContextSize = 1,
                  nsCOMArray<nsIDOMNode> *aSet = nsnull,
                  nsStringArray          *aIndexesUsed = nsnull);

  /**
   * Given a node in the instance data, get its string value according
   * to section 8.1.1 of the XForms specification.
   */
  static NS_HIDDEN_(void) GetNodeValue(nsIDOMNode *aDataNode,
                                       nsAString  &aNodeValue);

  /**
   * Convenience method for doing XPath evaluations to get bound node
   * for an element.  Also returns the associated model if aModel != null.
   * Returns PR_TRUE if the evaluation succeeds.
   */
  static NS_HIDDEN_(PRBool)
    GetSingleNodeBinding(nsIDOMElement* aElement, nsIDOMNode** aNode,
                         nsIModelElementPrivate** aModel);

  /**
   * Convenience method for doing XPath evaluations to get string value
   * for an element.
   * Returns PR_TRUE if the evaluation succeeds.
   */
  static NS_HIDDEN_(PRBool)
    GetSingleNodeBindingValue(nsIDOMElement* aElement, nsString& aValue);

  /**
   * Dispatch an XForms event.  aDefaultActionEnabled is returned indicating
   * if the default action of the dispatched event was enabled.  aSrcElement
   * is passed for events targeted at models.  If the model doesn't exist, yet,
   * then the event dispatching is deferred.  Once DOMContentLoaded is detected
   * we'll grab the model from aSrcElement and dispatch the event to that
   * model.
   */
  static NS_HIDDEN_(nsresult)
    DispatchEvent(nsIDOMNode* aTarget, nsXFormsEvent aEvent,
                  PRBool *aDefaultActionEnabled = nsnull,
                  nsIDOMElement *aSrcElement = nsnull);

  static NS_HIDDEN_(nsresult)
    DispatchDeferredEvents(nsIDOMDocument* aDocument);

  /**
   * Sets aEvent trusted if aRelatedNode is in chrome.
   * When dispatching events in chrome, they should be set trusted
   * because by default event listeners in chrome handle only trusted
   * events.
   * Should be called before any event dispatching in XForms.
   */
  static NS_HIDDEN_(nsresult)
    SetEventTrusted(nsIDOMEvent* aEvent, nsIDOMNode* aRelatedNode);

  /**
   * Returns PR_TRUE unless aTarget is in chrome and aEvent is not trusted.
   * This should be used always before handling events. Otherwise if XForms
   * is used in chrome, it may try to handle events that can be synthesized 
   * by untrusted content. I.e. content documents may create events using
   * document.createEvent() and then fire them using target.dispatchEvent();
   */
  static NS_HIDDEN_(PRBool)
    EventHandlingAllowed(nsIDOMEvent* aEvent, nsIDOMNode* aTarget);

  /**
   * Returns PR_TRUE, if aEvent is an XForms event, and sets the values
   * of aCancelable and aBubbles parameters according to the event type.
   */
  static NS_HIDDEN_(PRBool)
    IsXFormsEvent(const nsAString& aEvent,
                  PRBool& aCancelable,
                  PRBool& aBubbles);

  /**
   * Checks if aEvent is a predefined event and sets the values
   * of aCancelable and aBubbles parameters according to the event type.
   */
  static NS_HIDDEN_(void)
    GetEventDefaults(const nsAString& aEvent,
                     PRBool& aCancelable,
                     PRBool& aBubbles);

  /**
   * Clone the set of IIDs in |aIIDList| into |aOutArray|.
   * |aOutCount| is set to |aIIDCount|.
   */
  static NS_HIDDEN_(nsresult) CloneScriptingInterfaces(const nsIID *aIIDList,
                                                       unsigned int aIIDCount,
                                                       PRUint32 *aOutCount,
                                                       nsIID ***aOutArray);

  /**
   * Returns the context for the element, if set by a parent node.
   *
   * Controls inheriting from nsIXFormsContextControl sets the context for its
   * children.
   *
   * @param aElement          The document element of the caller
   * @param aModel            The model for |aElement| (if (!*aModel), it is set)
   * @param aParentControl    The parent control setting the context
   * @param aContextNode      The resulting context node
   * @param aContextPosition  The resulting context position
   * @param aContextSize      The resulting context size
   */
  static NS_HIDDEN_(nsresult) FindParentContext(nsIDOMElement           *aElement,
                                                nsIModelElementPrivate **aModel,
                                                nsIXFormsControl       **aParentControl,
                                                nsIDOMNode             **aContextNode,
                                                PRInt32                 *aContextPosition,
                                                PRInt32                 *aContextSize);

  /** Connection type used by CheckConnectionAllowed */
  enum ConnectionType {
    /** Send data, such as doing submission */
    kXFormsActionSend = 1,

    /** Load data, such as getting external instance data */
    kXFormsActionLoad = 2,

    /** Send and Load data, which is replace=instance */
    kXFormsActionLoadSend = 3
  };

  /**
   * Check whether a connecion to aTestURI from aElement is allowed.
   *
   * @param  aElement          The element trying to access the resource
   * @param  aTestURI          The uri we are trying to connect to
   * @param  aType             The type of connection (see ConnectionType)
   * @return                   Whether connection is allowed
   */
  static NS_HIDDEN_(PRBool) CheckConnectionAllowed(nsIDOMElement *aElement,
                                                   nsIURI        *aTestURI,
                                                   ConnectionType aType = kXFormsActionLoad);

  /**
   * @return true if aNode is element, its namespace URI is 
   * "http://www.w3.org/2002/xforms" and its name is aName.
   */
  static NS_HIDDEN_(PRBool) IsXFormsElement(nsIDOMNode* aNode, 
                                            const nsAString& aName);
  /**
   * Returns true if the given element is an XForms label.
   */
  static NS_HIDDEN_(PRBool) IsLabelElement(nsIDOMNode *aElement);
  
  /**
   * Tries to focus a form control and returns true if succeeds.
   */
  static NS_HIDDEN_(PRBool) FocusControl(nsIDOMElement *aElement);

  /**
   * Sorts the array and removes duplicate entries
   */
  static NS_HIDDEN_(void) MakeUniqueAndSort(nsCOMArray<nsIDOMNode> *aArray);

  /**
   * Returns the <xf:instance> for a given instance data node.
   */
  static NS_HIDDEN_(nsresult) GetInstanceNodeForData(nsIDOMNode *aInstanceDataNode,
                                                     nsIDOMNode  **aInstanceNode);

  /**
   * Returns the type bound to the given node.
   *
   * @param aInstanceData   An instance data node or attribute on an instance
   *                        data node from which to retrieve type.
   * @param aType           On return, type of given node.
   * @param aNSUri          On return, namespace URI of aType.
   */
  static NS_HIDDEN_(nsresult) ParseTypeFromNode(nsIDOMNode *aInstanceData,
                                                nsAString  &aType,
                                                nsAString  &aNSUri);

  /**
   * Outputs to the Error console.
   *
   * @param aMessageName      Name of string to output, which is loaded from
                              xforms.properties
   * @param aParams           Optional parameters for the loaded string
   * @param aParamLength      Amount of params in aParams
   * @param aElement          If set, is used to determine and output the
                              location of the file the XForm is located in
   * @param aContext          If set, the node is used to output what element
                              caused the error
   * @param aErrorType        Type of error in form of an nsIScriptError flag
   */
  static NS_HIDDEN_(void) ReportError(const nsString   &aMessageName,
                                      const PRUnichar **aParams,
                                      PRUint32          aParamLength,
                                      nsIDOMNode       *aElement,
                                      nsIDOMNode       *aContext,
                                      PRUint32          aErrorFlag = nsIScriptError::errorFlag);

  /**
   * Simple version of ReportError(), used when reporting without message
   * arguments and file location and node is taken from same element (or
   * nsnull).
   *
   * @param aMessageName      Name of string
   * @param aElement          Element to use for location and context
   * @param aErrorType        Type of error in form of an nsIScriptError flag
   */
  static NS_HIDDEN_(void) ReportError(const nsString &aMessageName,
                                      nsIDOMNode     *aElement = nsnull,
                                      PRUint32        aErrorFlag = nsIScriptError::errorFlag)
    {
      nsXFormsUtils::ReportError(aMessageName, nsnull, 0, aElement, aElement, aErrorFlag);
    }

  /**
   * Returns whether the an elements document is ready to bind data (all
   * instance documents loaded, etc.).
   *
   * @param aElement         The element to check for
   */
  static NS_HIDDEN_(PRBool) IsDocumentReadyForBind(nsIDOMElement *aElement);

  /**
   * Retrieve an element by id, handling (cloned) elements inside repeats.
   *
   * @param aDoc              The document to get element from
   * @param aId               The id of the element
   * @param aOnlyXForms       Only search for XForms elements
   * @param aCaller           The caller (or rather the caller's DOM element),
                              ignored if nsnull
   * @param aElement          The element (or nsnull if not found)
   */
  static NS_HIDDEN_(nsresult) GetElementById(nsIDOMDocument   *aDoc,
                                             const nsAString  &aId,
                                             const PRBool      aOnlyXForms,
                                             nsIDOMElement    *aCaller,
                                             nsIDOMElement   **aElement);
  
  /**
   * Shows an error dialog for fatal errors.
   *
   * The dialog can be disabled via the |xforms.disablePopup| preference.
   *
   * @param aElement         Element the exception occured at
   * @param aName            The name to use for the new window
   * @return                 Whether handling was successful
   */
  static NS_HIDDEN_(PRBool) HandleFatalError(nsIDOMElement   *aElement,
                                             const nsAString &aName);

  /**
   * Returns whether the given NamedNodeMaps of Entities are equal
   *
   */
  static NS_HIDDEN_(PRBool) AreEntitiesEqual(nsIDOMNamedNodeMap *aEntities1,
                                             nsIDOMNamedNodeMap *aEntities2);

  /**
   * Returns whether the given NamedNodeMaps of Notations are equal
   *
   */
  static NS_HIDDEN_(PRBool) AreNotationsEqual(nsIDOMNamedNodeMap *aNotations1,
                                              nsIDOMNamedNodeMap *aNotations2);

  /**
   * Returns whether the given nodes are equal as described in the isEqualNode
   * function defined in the DOM Level 3 Core spec.
   * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/DOM3-Core.html#core-Node3-isEqualNode
   *
   * XXX: this is just temporary until isEqualNode is implemented in Mozilla
   * (https://bugzilla.mozilla.org/show_bug.cgi?id=159167)
   *
   * @param aFirstNode          The first node to compare
   * @param aSecondNode         The second node to compare
   * @param aAlreadyNormalized  Whether the two nodes and their children, etc.
   *                            have already been normalized to allow for
   *                            more accurate child node comparisons, as
   *                            recommended in the DOM Level 3 Core spec.
   */
  static NS_HIDDEN_(PRBool) AreNodesEqual(nsIDOMNode *aFirstNode,
                                          nsIDOMNode *aSecondNode,
                                          PRBool      aAlreadyNormalized = PR_FALSE);

  /**
   * Retrieve the window object from the given document
   *
   * @param aDoc              The document to get window object from
   * @param aWindow           The found window object
   */
  static NS_HIDDEN_(nsresult) GetWindowFromDocument(nsIDOMDocument        *aDoc,
                                                    nsIDOMWindowInternal **aWindow);

private:
  /**
   * Do same origin checks on aBaseDocument and aTestURI. Hosts can be
   * whitelisted through the XForms permissions.
   *
   * @note The function assumes that the caller does not pass null arguments
   *
   * @param  aBaseDocument     The document the XForms lives in
   * @param  aTestURI          The uri we are trying to connect to
   * @param  aType             The type of connection (see ConnectionType)
   * @return                   Whether connection is allowed
   *
   */
  static NS_HIDDEN_(PRBool) CheckSameOrigin(nsIDocument   *aBaseDocument,
                                            nsIURI        *aTestURI,
                                            ConnectionType aType = kXFormsActionLoad);

  /**
   * Check content policy for loading the specificed aTestURI.
   *
   * @note The function assumes that the caller does not pass null arguments
   *
   * @param  aElement          The element trying to load the content
   * @param  aBaseDocument     The document the XForms lives in
   * @param  aTestURI          The uri we are trying to load
   * @return                   Whether loading is allowed.
   */
  static NS_HIDDEN_(PRBool) CheckContentPolicy(nsIDOMElement *aElement,
                                               nsIDocument   *aDoc,
                                               nsIURI        *aURI);

};

#endif
