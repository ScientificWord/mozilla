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
 *   Daniel Kraft <d@domob.eu>
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

/**
 * nsStyledElement is the base for elements supporting styling via the
 * id/class/style attributes; it is a common base for their support in HTML,
 * SVG and MathML.
 */

#ifndef __NS_STYLEDELEMENT_H_
#define __NS_STYLEDELEMENT_H_

#include "nsString.h"
#include "nsGenericElement.h"

class nsICSSStyleRule;

typedef nsGenericElement nsStyledElementBase;

class nsStyledElement : public nsStyledElementBase
{

protected:

  inline nsStyledElement(nsINodeInfo *aNodeInfo)
    : nsStyledElementBase(aNodeInfo)
  {}

public:

  // nsIContent interface methods for styling
  virtual nsIAtom* GetClassAttributeName() const;
  virtual nsIAtom* GetIDAttributeName() const;
  virtual const nsAttrValue* GetClasses() const;

  virtual nsICSSStyleRule* GetInlineStyleRule();
  NS_IMETHOD SetInlineStyleRule(nsICSSStyleRule* aStyleRule, PRBool aNotify);

  virtual nsresult BindToTree(nsIDocument* aDocument, nsIContent* aParent,
                              nsIContent* aBindingParent,
                              PRBool aCompileEventHandlers);

  /**
   * Parse a style attr value into a CSS rulestruct (or, if there is no
   * document, leave it as a string) and return as nsAttrValue.
   * Note: this function is used by other classes than nsStyledElement
   *
   * @param aValue the value to parse
   * @param aResult the resulting HTMLValue [OUT]
   */
  static void ParseStyleAttribute(nsIContent* aContent,
                                  const nsAString& aValue,
                                  nsAttrValue& aResult,
                                  PRBool aForceInDataDoc);

  static void Shutdown();
  
protected:

  virtual PRBool ParseAttribute(PRInt32 aNamespaceID, nsIAtom* aAttribute,
                                const nsAString& aValue, nsAttrValue& aResult);

  nsresult GetStyle(nsIDOMCSSStyleDeclaration** aStyle);

  /**
   * Create the style struct from the style attr.  Used when an element is
   * first put into a document.  Only has an effect if the old value is a
   * string.  If aForceInDataDoc is true, will reparse even if we're in a data
   * document.
   */
  nsresult  ReparseStyleAttribute(PRBool aForceInDataDoc);
};

#endif // __NS_STYLEDELEMENT_H_
