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
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2003
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   IBM Corporation
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

/*
 * A struct that represents the value (type and actual data) of an
 * attribute.
 */

#ifndef nsAttrValue_h___
#define nsAttrValue_h___

#include "nscore.h"
#include "nsString.h"
#include "nsStringBuffer.h"
#include "nsColor.h"
#include "nsCaseTreatment.h"

typedef unsigned long PtrBits;
class nsAString;
class nsIAtom;
class nsICSSStyleRule;
class nsISVGValue;
class nsIDocument;
template<class E> class nsCOMArray;
template<class E> class nsTPtrArray;

#define NS_ATTRVALUE_MAX_STRINGLENGTH_ATOM 12

#define NS_ATTRVALUE_BASETYPE_MASK (PtrBits(3))
#define NS_ATTRVALUE_POINTERVALUE_MASK (~NS_ATTRVALUE_BASETYPE_MASK)

#define NS_ATTRVALUE_INTEGERTYPE_BITS 4
#define NS_ATTRVALUE_INTEGERTYPE_MASK (PtrBits((1 << NS_ATTRVALUE_INTEGERTYPE_BITS) - 1))
#define NS_ATTRVALUE_INTEGERTYPE_MULTIPLIER (1 << NS_ATTRVALUE_INTEGERTYPE_BITS)
#define NS_ATTRVALUE_INTEGERTYPE_MAXVALUE ((1 << (31 - NS_ATTRVALUE_INTEGERTYPE_BITS)) - 1)
#define NS_ATTRVALUE_INTEGERTYPE_MINVALUE (-NS_ATTRVALUE_INTEGERTYPE_MAXVALUE - 1)

#define NS_ATTRVALUE_ENUMTABLEINDEX_BITS (32 - 16 - NS_ATTRVALUE_INTEGERTYPE_BITS)
#define NS_ATTRVALUE_ENUMTABLEINDEX_MAXVALUE ((1 << NS_ATTRVALUE_ENUMTABLEINDEX_BITS) - 1)
#define NS_ATTRVALUE_ENUMTABLEINDEX_MASK (PtrBits((1 << NS_ATTRVALUE_ENUMTABLEINDEX_BITS) - 1))

/**
 * A class used to construct a nsString from a nsStringBuffer (we might
 * want to move this to nsString at some point).
 */
class nsCheapString : public nsString {
public:
  nsCheapString(nsStringBuffer* aBuf)
  {
    if (aBuf)
      aBuf->ToString(aBuf->StorageSize()/2 - 1, *this);
  }
};

class nsAttrValue {
public:
  nsAttrValue();
  nsAttrValue(const nsAttrValue& aOther);
  explicit nsAttrValue(const nsAString& aValue);
  explicit nsAttrValue(nsICSSStyleRule* aValue);
#ifdef MOZ_SVG
  explicit nsAttrValue(nsISVGValue* aValue);
#endif
  ~nsAttrValue();

  static nsresult Init();
  static void Shutdown();

  // This has to be the same as in ValueBaseType
  enum ValueType {
    eString =       0x00, //   00
                          //   01  this value indicates an 'misc' struct
    eAtom =         0x02, //   10
    eInteger =      0x03, // 0011
    eColor =        0x07, // 0111
    eEnum =         0x0B, // 1011  This should eventually die
    ePercent =      0x0F, // 1111
    // Values below here won't matter, they'll be stored in the 'misc' struct
    // anyway
    eCSSStyleRule = 0x10,
    eAtomArray =    0x11 
#ifdef MOZ_SVG
    ,eSVGValue =    0x12
#endif
  };

  ValueType Type() const;

  void Reset();

  void SetTo(const nsAttrValue& aOther);
  void SetTo(const nsAString& aValue);
  void SetTo(PRInt16 aInt);
  void SetTo(nsICSSStyleRule* aValue);
#ifdef MOZ_SVG
  void SetTo(nsISVGValue* aValue);
#endif

  void SwapValueWith(nsAttrValue& aOther);

  void ToString(nsAString& aResult) const;

  // Methods to get value. These methods do not convert so only use them
  // to retrieve the datatype that this nsAttrValue has.
  inline PRBool IsEmptyString() const;
  const nsCheapString GetStringValue() const;
  inline nsIAtom* GetAtomValue() const;
  inline PRInt32 GetIntegerValue() const;
  PRBool GetColorValue(nscolor& aColor) const;
  inline PRInt16 GetEnumValue() const;
  inline float GetPercentValue() const;
  inline nsCOMArray<nsIAtom>* GetAtomArrayValue() const;
  inline nsICSSStyleRule* GetCSSStyleRuleValue() const;
#ifdef MOZ_SVG
  inline nsISVGValue* GetSVGValue() const;
#endif

  // Methods to get access to atoms we may have
  // Returns the number of atoms we have; 0 if we have none.  It's OK
  // to call this without checking the type first; it handles that.
  PRInt32 GetAtomCount() const;
  // Returns the atom at aIndex (0-based).  Do not call this with
  // aIndex >= GetAtomCount().
  nsIAtom* AtomAt(PRInt32 aIndex) const;

  PRUint32 HashValue() const;
  PRBool Equals(const nsAttrValue& aOther) const;
  PRBool Equals(const nsAString& aValue, nsCaseTreatment aCaseSensitive) const;
  PRBool Equals(nsIAtom* aValue, nsCaseTreatment aCaseSensitive) const;

  /**
   * Returns true if this AttrValue is equal to the given atom, or is an
   * array which contains the given atom.
   */
  PRBool Contains(nsIAtom* aValue, nsCaseTreatment aCaseSensitive) const;

  void ParseAtom(const nsAString& aValue);
  void ParseAtomArray(const nsAString& aValue);
  void ParseStringOrAtom(const nsAString& aValue);

  /**
   * Structure for a mapping from int (enum) values to strings.  When you use
   * it you generally create an array of them.
   * Instantiate like this:
   * EnumTable myTable[] = {
   *   { "string1", 1 },
   *   { "string2", 2 },
   *   { 0 }
   * }
   */
  struct EnumTable {
    /** The string the value maps to */
    const char* tag;
    /** The enum value that maps to this string */
    PRInt16 value;
  };

  /**
   * Parse into an enum value.
   *
   * @param aValue the string to find the value for
   * @param aTable the enumeration to map with
   * @param aResult the enum mapping [OUT]
   * @return whether the enum value was found or not
   */
  PRBool ParseEnumValue(const nsAString& aValue,
                        const EnumTable* aTable,
                        PRBool aCaseSensitive = PR_FALSE);

  /**
   * Parse a string into an integer. Can optionally parse percent (n%).
   * This method explicitly sets a lower bound of zero on the element,
   * whether it be percent or raw integer.
   *
   * @param aString the string to parse
   * @param aCanBePercent PR_TRUE if it can be a percent value (%)
   * @return whether the value could be parsed
   */
  PRBool ParseSpecialIntValue(const nsAString& aString,
                              PRBool aCanBePercent);


  /**
   * Parse a string value into an integer.
   *
   * @param aString the string to parse
   * @return whether the value could be parsed
   */
  PRBool ParseIntValue(const nsAString& aString) {
    return ParseIntWithBounds(aString, NS_ATTRVALUE_INTEGERTYPE_MINVALUE,
                              NS_ATTRVALUE_INTEGERTYPE_MAXVALUE);
  }

  /**
   * Parse a string value into an integer with minimum value and maximum value.
   *
   * @param aString the string to parse
   * @param aMin the minimum value (if value is less it will be bumped up)
   * @param aMax the maximum value (if value is greater it will be chopped down)
   * @return whether the value could be parsed
   */
  PRBool ParseIntWithBounds(const nsAString& aString, PRInt32 aMin,
                            PRInt32 aMax = NS_ATTRVALUE_INTEGERTYPE_MAXVALUE);

  /**
   * Parse a string into a color.
   *
   * @param aString the string to parse
   * @param aDocument the document (to find out whether we're in quirks mode)
   * @return whether the value could be parsed
   */
  PRBool ParseColor(const nsAString& aString, nsIDocument* aDocument);

private:
  // These have to be the same as in ValueType
  enum ValueBaseType {
    eStringBase =    eString,    // 00
    eOtherBase =     0x01,       // 01
    eAtomBase =      eAtom,      // 10
    eIntegerBase =   0x03        // 11
  };

  struct MiscContainer
  {
    ValueType mType;
    union {
      nscolor mColor;
      nsICSSStyleRule* mCSSStyleRule;
      nsCOMArray<nsIAtom>* mAtomArray;
#ifdef MOZ_SVG
      nsISVGValue* mSVGValue;
#endif
    };
  };

  inline ValueBaseType BaseType() const;

  inline void SetPtrValueAndType(void* aValue, ValueBaseType aType);
  inline void SetIntValueAndType(PRInt32 aValue, ValueType aType);
  inline void ResetIfSet();

  inline void* GetPtr() const;
  inline MiscContainer* GetMiscContainer() const;
  inline PRInt32 GetIntInternal() const;

  PRBool EnsureEmptyMiscContainer();
  PRBool EnsureEmptyAtomArray();

  static nsTPtrArray<const EnumTable>* sEnumTableArray;

  PtrBits mBits;
};

/**
 * Implementation of inline methods
 */

inline nsIAtom*
nsAttrValue::GetAtomValue() const
{
  NS_PRECONDITION(Type() == eAtom, "wrong type");
  return reinterpret_cast<nsIAtom*>(GetPtr());
}

inline PRInt32
nsAttrValue::GetIntegerValue() const
{
  NS_PRECONDITION(Type() == eInteger, "wrong type");
  return GetIntInternal();
}

inline PRInt16
nsAttrValue::GetEnumValue() const
{
  NS_PRECONDITION(Type() == eEnum, "wrong type");
  // We don't need to worry about sign extension here since we're
  // returning an PRInt16 which will cut away the top bits.
  return static_cast<PRInt16>
                    (GetIntInternal() >> NS_ATTRVALUE_ENUMTABLEINDEX_BITS);
}

inline float
nsAttrValue::GetPercentValue() const
{
  NS_PRECONDITION(Type() == ePercent, "wrong type");
  return static_cast<float>(GetIntInternal()) /
         100.0f;
}

inline nsCOMArray<nsIAtom>*
nsAttrValue::GetAtomArrayValue() const
{
  NS_PRECONDITION(Type() == eAtomArray, "wrong type");
  return GetMiscContainer()->mAtomArray;
}

inline nsICSSStyleRule*
nsAttrValue::GetCSSStyleRuleValue() const
{
  NS_PRECONDITION(Type() == eCSSStyleRule, "wrong type");
  return GetMiscContainer()->mCSSStyleRule;
}

#ifdef MOZ_SVG
inline nsISVGValue*
nsAttrValue::GetSVGValue() const
{
  NS_PRECONDITION(Type() == eSVGValue, "wrong type");
  return GetMiscContainer()->mSVGValue;
}
#endif

inline nsAttrValue::ValueBaseType
nsAttrValue::BaseType() const
{
  return static_cast<ValueBaseType>(mBits & NS_ATTRVALUE_BASETYPE_MASK);
}

inline void
nsAttrValue::SetPtrValueAndType(void* aValue, ValueBaseType aType)
{
  NS_ASSERTION(!(NS_PTR_TO_INT32(aValue) & ~NS_ATTRVALUE_POINTERVALUE_MASK),
               "pointer not properly aligned, this will crash");
  mBits = reinterpret_cast<PtrBits>(aValue) | aType;
}

inline void
nsAttrValue::SetIntValueAndType(PRInt32 aValue, ValueType aType)
{
#ifdef DEBUG
  {
    PRInt32 tmp = aValue * NS_ATTRVALUE_INTEGERTYPE_MULTIPLIER;
    NS_ASSERTION(tmp / NS_ATTRVALUE_INTEGERTYPE_MULTIPLIER == aValue,
                 "Integer too big to fit");
  }
#endif
  mBits = (aValue * NS_ATTRVALUE_INTEGERTYPE_MULTIPLIER) | aType;
}

inline void
nsAttrValue::ResetIfSet()
{
  if (mBits) {
    Reset();
  }
}

inline void*
nsAttrValue::GetPtr() const
{
  NS_ASSERTION(BaseType() != eIntegerBase,
               "getting pointer from non-pointer");
  return reinterpret_cast<void*>(mBits & NS_ATTRVALUE_POINTERVALUE_MASK);
}

inline nsAttrValue::MiscContainer*
nsAttrValue::GetMiscContainer() const
{
  NS_ASSERTION(BaseType() == eOtherBase, "wrong type");
  return static_cast<MiscContainer*>(GetPtr());
}

inline PRInt32
nsAttrValue::GetIntInternal() const
{
  NS_ASSERTION(BaseType() == eIntegerBase,
               "getting integer from non-integer");
  // Make sure we get a signed value.
  // Lets hope the optimizer optimizes this into a shift. Unfortunatly signed
  // bitshift right is implementaion dependant.
  return static_cast<PRInt32>(mBits & ~NS_ATTRVALUE_INTEGERTYPE_MASK) /
         NS_ATTRVALUE_INTEGERTYPE_MULTIPLIER;
}

inline PRBool
nsAttrValue::IsEmptyString() const
{
  return !mBits;
}

#endif
