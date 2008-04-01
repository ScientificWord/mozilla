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
 * The Original Code is the Mozilla SVG project.
 *
 * The Initial Developer of the Original Code is
 * Crocodile Clips Ltd..
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alex Fritze <alex.fritze@crocodile-clips.com> (original author)
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

#ifndef __NS_SVGELEMENT_H__
#define __NS_SVGELEMENT_H__

/*
  nsSVGElement is the base class for all SVG content elements.
  It implements all the common DOM interfaces and handles attributes.
*/

#include "nsString.h"
#include "nsCOMPtr.h"
#include "nsIDOMSVGElement.h"
#include "nsGenericElement.h"
#include "nsStyledElement.h"
#include "nsISVGValue.h"
#include "nsISVGValueObserver.h"
#include "nsWeakReference.h"
#include "nsICSSStyleRule.h"

class nsSVGSVGElement;
class nsSVGLength2;
class nsSVGNumber2;
class nsSVGInteger;
class nsSVGAngle;
class nsSVGBoolean;
class nsSVGEnum;
struct nsSVGEnumMapping;

typedef nsStyledElement nsSVGElementBase;

class nsSVGElement : public nsSVGElementBase,    // nsIContent
                     public nsISVGValueObserver  // :nsISupportsWeakReference
{
protected:
  nsSVGElement(nsINodeInfo *aNodeInfo);
  nsresult Init();
  virtual ~nsSVGElement();

public:
  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED

  // nsIContent interface methods

  virtual nsresult BindToTree(nsIDocument* aDocument, nsIContent* aParent,
                              nsIContent* aBindingParent,
                              PRBool aCompileEventHandlers);

  virtual nsresult UnsetAttr(PRInt32 aNameSpaceID, nsIAtom* aAttribute,
                             PRBool aNotify);

  virtual PRBool IsNodeOfType(PRUint32 aFlags) const;

  virtual already_AddRefed<nsIURI> GetBaseURI() const;

  NS_IMETHOD WalkContentStyleRules(nsRuleWalker* aRuleWalker);

  static const MappedAttributeEntry sFillStrokeMap[];
  static const MappedAttributeEntry sGraphicsMap[];
  static const MappedAttributeEntry sTextContentElementsMap[];
  static const MappedAttributeEntry sFontSpecificationMap[];
  static const MappedAttributeEntry sGradientStopMap[];
  static const MappedAttributeEntry sViewportsMap[];
  static const MappedAttributeEntry sMarkersMap[];
  static const MappedAttributeEntry sColorMap[];
  static const MappedAttributeEntry sFiltersMap[];
  static const MappedAttributeEntry sFEFloodMap[];
  static const MappedAttributeEntry sLightingEffectsMap[];

  // nsIDOMNode
  NS_IMETHOD IsSupported(const nsAString& aFeature, const nsAString& aVersion,
                         PRBool* aReturn);
  
  // nsIDOMSVGElement
  NS_IMETHOD GetId(nsAString & aId);
  NS_IMETHOD SetId(const nsAString & aId);
  NS_IMETHOD GetOwnerSVGElement(nsIDOMSVGSVGElement** aOwnerSVGElement);
  NS_IMETHOD GetViewportElement(nsIDOMSVGElement** aViewportElement);

  // nsISVGValueObserver
  NS_IMETHOD WillModifySVGObservable(nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);
  NS_IMETHOD DidModifySVGObservable (nsISVGValue* observable,
                                     nsISVGValue::modificationType aModType);

  // nsISupportsWeakReference
  // implementation inherited from nsSupportsWeakReference

  // Gets the element that establishes the rectangular viewport against which
  // we should resolve percentage lengths (our "coordinate context"). Returns
  // nsnull for outer <svg> or SVG without an <svg> parent (invalid SVG).
  nsSVGSVGElement* GetCtx();

  virtual void DidChangeLength(PRUint8 aAttrEnum, PRBool aDoSetAttr);
  virtual void DidChangeNumber(PRUint8 aAttrEnum, PRBool aDoSetAttr);
  virtual void DidChangeInteger(PRUint8 aAttrEnum, PRBool aDoSetAttr);
  virtual void DidChangeAngle(PRUint8 aAttrEnum, PRBool aDoSetAttr);
  virtual void DidChangeBoolean(PRUint8 aAttrEnum, PRBool aDoSetAttr);
  virtual void DidChangeEnum(PRUint8 aAttrEnum, PRBool aDoSetAttr);

  void GetAnimatedLengthValues(float *aFirst, ...);
  void GetAnimatedNumberValues(float *aFirst, ...);
  void GetAnimatedIntegerValues(PRInt32 *aFirst, ...);

  virtual void RecompileScriptEventListeners();

protected:
  virtual nsresult BeforeSetAttr(PRInt32 aNamespaceID, nsIAtom* aName,
                                 const nsAString* aValue, PRBool aNotify);
  virtual nsresult AfterSetAttr(PRInt32 aNamespaceID, nsIAtom* aName,
                                const nsAString* aValue, PRBool aNotify);
  virtual PRBool ParseAttribute(PRInt32 aNamespaceID, nsIAtom* aAttribute,
                                const nsAString& aValue, nsAttrValue& aResult);

  // Hooks for subclasses
  virtual PRBool IsEventName(nsIAtom* aName);

  void UpdateContentStyleRule();
  nsISVGValue* GetMappedAttribute(PRInt32 aNamespaceID, nsIAtom* aName);
  nsresult AddMappedSVGValue(nsIAtom* aName, nsISupports* aValue,
                             PRInt32 aNamespaceID = kNameSpaceID_None);
  
  static nsIAtom* GetEventNameForAttr(nsIAtom* aAttr);

  struct LengthInfo {
    nsIAtom** mName;
    float     mDefaultValue;
    PRUint8   mDefaultUnitType;
    PRUint8   mCtxType;
  };

  struct LengthAttributesInfo {
    nsSVGLength2* mLengths;
    LengthInfo*   mLengthInfo;
    PRUint32      mLengthCount;

    LengthAttributesInfo(nsSVGLength2 *aLengths,
                         LengthInfo *aLengthInfo,
                         PRUint32 aLengthCount) :
      mLengths(aLengths), mLengthInfo(aLengthInfo), mLengthCount(aLengthCount)
      {}

    void Reset(PRUint8 aAttrEnum);
  };

  struct NumberInfo {
    nsIAtom** mName;
    float     mDefaultValue;
  };

  struct NumberAttributesInfo {
    nsSVGNumber2* mNumbers;
    NumberInfo*   mNumberInfo;
    PRUint32      mNumberCount;

    NumberAttributesInfo(nsSVGNumber2 *aNumbers,
                         NumberInfo *aNumberInfo,
                         PRUint32 aNumberCount) :
      mNumbers(aNumbers), mNumberInfo(aNumberInfo), mNumberCount(aNumberCount)
      {}

    void Reset(PRUint8 aAttrEnum);
  };

  struct IntegerInfo {
    nsIAtom** mName;
    PRInt32   mDefaultValue;
  };

  struct IntegerAttributesInfo {
    nsSVGInteger* mIntegers;
    IntegerInfo*  mIntegerInfo;
    PRUint32      mIntegerCount;

    IntegerAttributesInfo(nsSVGInteger *aIntegers,
                          IntegerInfo *aIntegerInfo,
                          PRUint32 aIntegerCount) :
      mIntegers(aIntegers), mIntegerInfo(aIntegerInfo), mIntegerCount(aIntegerCount)
      {}

    void Reset(PRUint8 aAttrEnum);
  };

  struct AngleInfo {
    nsIAtom** mName;
    float     mDefaultValue;
    PRUint8   mDefaultUnitType;
  };

  struct AngleAttributesInfo {
    nsSVGAngle* mAngles;
    AngleInfo*  mAngleInfo;
    PRUint32    mAngleCount;

    AngleAttributesInfo(nsSVGAngle *aAngles,
                        AngleInfo *aAngleInfo,
                        PRUint32 aAngleCount) :
      mAngles(aAngles), mAngleInfo(aAngleInfo), mAngleCount(aAngleCount)
      {}

    void Reset(PRUint8 aAttrEnum);
  };

  struct BooleanInfo {
    nsIAtom**    mName;
    PRPackedBool mDefaultValue;
  };

  struct BooleanAttributesInfo {
    nsSVGBoolean* mBooleans;
    BooleanInfo*  mBooleanInfo;
    PRUint32      mBooleanCount;

    BooleanAttributesInfo(nsSVGBoolean *aBooleans,
                          BooleanInfo *aBooleanInfo,
                          PRUint32 aBooleanCount) :
      mBooleans(aBooleans), mBooleanInfo(aBooleanInfo), mBooleanCount(aBooleanCount)
      {}

    void Reset(PRUint8 aAttrEnum);
  };

  friend class nsSVGEnum;

  struct EnumInfo {
    nsIAtom**         mName;
    nsSVGEnumMapping* mMapping;
    PRUint16          mDefaultValue;
  };

  struct EnumAttributesInfo {
    nsSVGEnum* mEnums;
    EnumInfo*  mEnumInfo;
    PRUint32   mEnumCount;

    EnumAttributesInfo(nsSVGEnum *aEnums,
                       EnumInfo *aEnumInfo,
                       PRUint32 aEnumCount) :
      mEnums(aEnums), mEnumInfo(aEnumInfo), mEnumCount(aEnumCount)
      {}

    void Reset(PRUint8 aAttrEnum);
  };

  virtual LengthAttributesInfo GetLengthInfo();
  virtual NumberAttributesInfo GetNumberInfo();
  virtual IntegerAttributesInfo GetIntegerInfo();
  virtual AngleAttributesInfo GetAngleInfo();
  virtual BooleanAttributesInfo GetBooleanInfo();
  virtual EnumAttributesInfo GetEnumInfo();

  static nsSVGEnumMapping sSVGUnitTypesMap[];

  /* read <number-optional-number> */
  PRBool
  ParseNumberOptionalNumber(nsIAtom* aAttribute, const nsAString& aValue,
                            PRUint32 aIndex1, PRUint32 aIndex2,
                            nsAttrValue& aResult);

  /* read <integer-optional-integer> */
  PRBool
  ParseIntegerOptionalInteger(nsIAtom* aAttribute, const nsAString& aValue,
                              PRUint32 aIndex1, PRUint32 aIndex2,
                              nsAttrValue& aResult);

  static nsresult ReportAttributeParseFailure(nsIDocument* aDocument,
                                              nsIAtom* aAttribute,
                                              const nsAString& aValue);

private:
  void ResetOldStyleBaseType(nsISVGValue *svg_value);

  nsCOMPtr<nsICSSStyleRule> mContentStyleRule;
  nsAttrAndChildArray mMappedAttributes;

  PRPackedBool mSuppressNotification;
};

/**
 * A macro to implement the NS_NewSVGXXXElement() functions.
 */
#define NS_IMPL_NS_NEW_SVG_ELEMENT(_elementName)                             \
nsresult                                                                     \
NS_NewSVG##_elementName##Element(nsIContent **aResult,                       \
                                 nsINodeInfo *aNodeInfo)                     \
{                                                                            \
  nsSVG##_elementName##Element *it =                                         \
    new nsSVG##_elementName##Element(aNodeInfo);                             \
  if (!it)                                                                   \
    return NS_ERROR_OUT_OF_MEMORY;                                           \
                                                                             \
  NS_ADDREF(it);                                                             \
                                                                             \
  nsresult rv = it->Init();                                                  \
                                                                             \
  if (NS_FAILED(rv)) {                                                       \
    NS_RELEASE(it);                                                          \
    return rv;                                                               \
  }                                                                          \
                                                                             \
  *aResult = it;                                                             \
                                                                             \
  return rv;                                                                 \
}

#endif // __NS_SVGELEMENT_H__
