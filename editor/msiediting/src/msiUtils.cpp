// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#include "nsIEditor.h"
#include "nsIDOMDocument.h"
#include "nsIDOMElement.h"
#include "nsIDOMNode.h"
#include "nsIDOMNodeList.h"
#include "nsIDOMAttr.h"
#include "nsIContent.h"
#include "nsISelection.h"
#include "nsCOMPtr.h"
#include "nsIDOMText.h"
#include "nsIDOMCharacterData.h"
#include "nsString.h"
#include "nsIDOMDocumentTraversal.h"
#include "nsIDOMTreeWalker.h"
#include "nsIDOMNodeFilter.h"
#include "msiIMathMLEditor.h"
#include "nsIMutableArray.h"
#include "nsISimpleEnumerator.h"
#include "nsComponentManagerUtils.h"
#include "nsIPrivateDOMEvent.h"
#include "nsGUIEvent.h"
#include "nsIServiceManager.h"


#include "msiUtils.h"
#include "nsEditor.h"
#include "msiIMMLEditDefines.h"
#include "msiIMathMLInsertion.h"
#include "msiIMathMLEditingBC.h"
#include "msiMaction.h"
#include "msiMaligngroup.h"
#include "msiMalignmark.h"
#include "msiMath.h"
#include "msiMenclose.h"
#include "msiMerror.h"
#include "msiMfenced.h"
#include "msiMfrac.h"
#include "msiMglyph.h"
#include "msiMi.h"
#include "msiMlabeledtr.h"
#include "msiMmultiscripts.h"
#include "msiMn.h"
#include "msiMo.h"
#include "msiMover.h"
#include "msiMpadded.h"
#include "msiMphantom.h"
#include "msiMroot.h"
#include "msiMrow.h"
#include "msiMs.h"
#include "msiMspace.h"
#include "msiMsqrt.h"
#include "msiMstyle.h"
#include "msiMsub.h"
#include "msiMsubsup.h"
#include "msiMsup.h"
#include "msiMtable.h"
#include "msiMtd.h"
#include "msiMtext.h"
#include "msiMtr.h"
#include "msiMunder.h"
#include "msiMunderover.h"
#include "msiEditingAtoms.h"
#include "msiNameSpaceUtils.h"

static PRBool initalized = PR_FALSE;

void msiUtils::Initalize()
{
  initalized = PR_TRUE;
  return;
}    

nsresult msiUtils::GetMathMLInsertionInterface(nsIEditor *editor,
                                             nsIDOMNode * node,
                                             PRUint32   offset,
                                             nsCOMPtr<msiIMathMLInsertion> & msiEditing)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
    if (msiEditor)
      res = msiEditor->GetMathMLInsertionInterface(node, offset, getter_AddRefs(msiEditing));
  }
  return res;
}                  

nsresult msiUtils::GetMathMLEditingBC(nsIEditor *editor,
                                      nsIDOMNode * node,
                                      PRUint32   offset,
                                      bool       clean,
                                      nsCOMPtr<msiIMathMLEditingBC> & editingBC)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
    if (msiEditor)
      res = msiEditor->GetMathMLEditingBC(node, offset, clean, getter_AddRefs(editingBC));
  }
  return res;
}                      

nsresult msiUtils::GetMathMLCaretInterface(nsIEditor *editor,
                                           nsIDOMNode * node,
                                           PRUint32   offset,
                                           nsCOMPtr<msiIMathMLCaret> & msiEditing)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
    if (msiEditor)
      res = msiEditor->GetMathMLCaretInterface(node, offset, getter_AddRefs(msiEditing));
  }
  return res;
}                      

nsresult msiUtils::SetupPassOffInsertToParent(nsIEditor * editor,
                                              nsIDOMNode * child,
                                              PRBool incrementOffset,
                                              nsCOMPtr<msiIMathMLInsertion> & msiEditing)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMNode> parent;
  PRUint32 offset(0);
  if (editor && child)
  {
    GetIndexOfChildInParent(child, offset);
    if (incrementOffset)
      offset += 1;
    child->GetParentNode(getter_AddRefs(parent));
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
    if (msiEditor && parent)
      res = msiEditor->GetMathMLInsertionInterface(parent, offset, getter_AddRefs(msiEditing));
  }
  return res;
}                                    

nsresult msiUtils::SetupPassOffCaretToParent(nsIEditor * editor,
                                             nsIDOMNode * child,
                                             PRBool incrementOffset,
                                             nsCOMPtr<msiIMathMLCaret> & msiEditing)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMNode> parent;
  PRUint32 offset(0);
  if (editor && child)
  {
    res = GetIndexOfChildInParent(child, offset);
    if (NS_FAILED(res))
      return res;
    if (incrementOffset)
      offset += 1;
    child->GetParentNode(getter_AddRefs(parent));
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
    if (msiEditor && parent)
      res = msiEditor->GetMathMLCaretInterface(parent, offset, getter_AddRefs(msiEditing));
  }
  return res;
}                                            
        
                      
// Unicode characters we think should be operators.
// See section 4.9 of Unicode 3.0 spec. and DerivedCoreProperties.txt
// I (and SWP) disagree with their classification in many places.

static
struct {
  PRUnichar lower;
  PRUnichar higher;
} OperatorList[] =
{
  // list must be sorted
  {0x0021,0x0021},  //  !
  {0x0025,0x0025},  // %
  {0x0026,0x0026},  // &
  {0x0028,0x002A},  // ()*
  {0x002B,0x002B},  // PLUS SIGN
  {0x002C,0x002F},  // ,-./
  {0x003C,0x003E},  // LESS-THAN SIGN..GREATER-THAN SIGN
  {0x003F,0x0040},  // ?@
  {0x005B,0x005D},  // [\]^
  {0x005E,0x005E},  // CIRCUMFLEX ACCENT
  {0x007B,0x007B},  // {
  {0x007C,0x007C},  // VERTICAL LINE
  {0x007D,0x007D},  // }
  {0x007E,0x007E},  // TILDE
  {0x00AC,0x00AC},  // NOT SIGN
  {0x00B1,0x00B1},  // PLUS-MINUS SIGN
  {0x00B7,0x00B7},  // middle dot
  {0x00D7,0x00D7},  // MULTIPLICATION SIGN
  {0x00F7,0x00F7},  // DIVISION SIGN
// ?? 03D0..03D2    ; Math # L&   [3] GREEK BETA SYMBOL..GREEK UPSILON WITH HOOK SYMBOL
// ?? 03D5          ; Math # L&       GREEK PHI SYMBOL
// ?? 03F0..03F1    ; Math # L&   [2] GREEK KAPPA SYMBOL..GREEK RHO SYMBOL
// ?? 03F4..03F5    ; Math # L&   [2] GREEK CAPITAL THETA SYMBOL..GREEK LUNATE EPSILON SYMBOL
// ?? 03F6          ; Math # Sm       GREEK REVERSED LUNATE EPSILON SYMBOL
  {0x2016,0x2016},  //        DOUBLE VERTICAL LINE
  {0x2032,0x2034},  //  PRIME..TRIPLE PRIME
  {0x2040,0x2040},  //        CHARACTER TIE
  {0x2044,0x2044},  //        FRACTION SLASH
  {0x2052,0x2052},  //        COMMERCIAL MINUS SIGN
  {0x2061,0x2063},  //  FUNCTION APPLICATION..INVISIBLE SEPARATOR
  {0x207A,0x207C},  //  SUPERSCRIPT PLUS SIGN..SUPERSCRIPT EQUALS SIGN
  {0x207D,0x207D},  //        SUPERSCRIPT LEFT PARENTHESIS
  {0x207E,0x207E},  //        SUPERSCRIPT RIGHT PARENTHESIS
  {0x208A,0x208C},  //  SUBSCRIPT PLUS SIGN..SUBSCRIPT EQUALS SIGN
  {0x208D,0x208D},  //        SUBSCRIPT LEFT PARENTHESIS
  {0x208E,0x208E},  //        SUBSCRIPT RIGHT PARENTHESIS
  {0x20D0,0x20DC},  //  COMBINING LEFT HARPOON ABOVE..COMBINING FOUR DOTS ABOVE
  {0x20E1,0x20E1},  //        COMBINING LEFT RIGHT ARROW ABOVE
  {0x20E5,0x20E6},  //  COMBINING REVERSE SOLIDUS OVERLAY..COMBINING DOUBLE VERTICAL STROKE OVERLAY
// ?? 2102          ; Math # L&       DOUBLE-STRUCK CAPITAL C
// ?? 210A..2113    ; Math # L&  [10] SCRIPT SMALL G..SCRIPT SMALL L
// ?? 2115          ; Math # L&       DOUBLE-STRUCK CAPITAL N
// ?? 2119..211D    ; Math # L&   [5] DOUBLE-STRUCK CAPITAL P..DOUBLE-STRUCK CAPITAL R
// ?? 2124          ; Math # L&       DOUBLE-STRUCK CAPITAL Z
// ?? 2128          ; Math # L&       BLACK-LETTER CAPITAL Z
  {0x2129,0x2129},  //  TURNED GREEK SMALL LETTER IOTA
// ?? 212C..212D    ; Math # L&   [2] SCRIPT CAPITAL B..BLACK-LETTER CAPITAL C
// ?? 212F..2131    ; Math # L&   [3] SCRIPT SMALL E..SCRIPT CAPITAL F
// ?? 2133..2134    ; Math # L&   [2] SCRIPT CAPITAL M..SCRIPT SMALL O
// ?? 2135..2138    ; Math # Lo   [4] ALEF SYMBOL..DALET SYMBOL
// ?? 213D..213F    ; Math # L&   [3] DOUBLE-STRUCK SMALL GAMMA..DOUBLE-STRUCK CAPITAL PI
// ?? 2140..2144    ; Math # Sm   [5] DOUBLE-STRUCK N-ARY SUMMATION..TURNED SANS-SERIF CAPITAL Y
// ?? 2145..2149    ; Math # L&   [5] DOUBLE-STRUCK ITALIC CAPITAL D..DOUBLE-STRUCK ITALIC SMALL J
  {0x214B,0x214B},  // TURNED AMPERSAND
  {0x2190,0x221D},  //  Arrows, Operators
// ?? 221E INFINITY (Unicode layout sucks)
  {0x221F,0x22FF},  //  Arrows, Operators
// ?? 2190..2194    ; Math # Sm   [5] LEFTWARDS ARROW..LEFT RIGHT ARROW
// ?? 219A..219B    ; Math # Sm   [2] LEFTWARDS ARROW WITH STROKE..RIGHTWARDS ARROW WITH STROKE
// ?? 21A0          ; Math # Sm       RIGHTWARDS TWO HEADED ARROW
// ?? 21A3          ; Math # Sm       RIGHTWARDS ARROW WITH TAIL
// ?? 21A6          ; Math # Sm       RIGHTWARDS ARROW FROM BAR
// ?? 21AE          ; Math # Sm       LEFT RIGHT ARROW WITH STROKE
// ?? 21CE..21CF    ; Math # Sm   [2] LEFT RIGHT DOUBLE ARROW WITH STROKE..RIGHTWARDS DOUBLE ARROW WITH STROKE
// ?? 21D2          ; Math # Sm       RIGHTWARDS DOUBLE ARROW
// ?? 21D4          ; Math # Sm       LEFT RIGHT DOUBLE ARROW
// ?? 21F4..22FF    ; Math # Sm [268] RIGHT ARROW WITH SMALL CIRCLE..Z NOTATION BAG MEMBERSHIP
  {0x2308,0x230B},  // LEFT CEILING..RIGHT FLOOR
  {0x2320,0x2321},  // TOP HALF INTEGRAL..BOTTOM HALF INTEGRAL
	{0x2329,0x232A},
  {0x237C,0x237C},  // RIGHT ANGLE WITH DOWNWARDS ZIGZAG ARROW
  {0x239B,0x23B3},  // LEFT PARENTHESIS UPPER HOOK..SUMMATION BOTTOM
  {0x23B7,0x23B7},  // RADICAL SYMBOL BOTTOM
  {0x23D0,0x23D0},  // VERTICAL LINE EXTENSION
  {0x25A0,0x25FF},  //  Geometric shapes
// ?? 25B7          ; Math # Sm       WHITE RIGHT-POINTING TRIANGLE
// ?? 25C1          ; Math # Sm       WHITE LEFT-POINTING TRIANGLE
// ?? 25F8..25FF    ; Math # Sm   [8] UPPER LEFT TRIANGLE..LOWER RIGHT TRIANGLE
  {0x266D,0x266F},  // Music flat..MUSIC SHARP SIGN
  {0x27D0,0x2AFF},  // etc.
// ?? 27D0..27E5    ; Math # Sm  [22] WHITE DIAMOND WITH CENTRED DOT..WHITE SQUARE WITH RIGHTWARDS TICK
// ?? 27E6          ; Math # Ps       MATHEMATICAL LEFT WHITE SQUARE BRACKET
// ?? 27E7          ; Math # Pe       MATHEMATICAL RIGHT WHITE SQUARE BRACKET
// ?? 27E8          ; Math # Ps       MATHEMATICAL LEFT ANGLE BRACKET
// ?? 27E9          ; Math # Pe       MATHEMATICAL RIGHT ANGLE BRACKET
// ?? 27EA          ; Math # Ps       MATHEMATICAL LEFT DOUBLE ANGLE BRACKET
// ?? 27EB          ; Math # Pe       MATHEMATICAL RIGHT DOUBLE ANGLE BRACKET
// ?? 27F0..27FF    ; Math # Sm  [16] UPWARDS QUADRUPLE ARROW..LONG RIGHTWARDS SQUIGGLE ARROW
// ?? 2900..2982    ; Math # Sm [131] RIGHTWARDS TWO-HEADED ARROW WITH VERTICAL STROKE..Z NOTATION TYPE COLON
// ?? 2983          ; Math # Ps       LEFT WHITE CURLY BRACKET
// ?? 2984          ; Math # Pe       RIGHT WHITE CURLY BRACKET
// ?? 2985          ; Math # Ps       LEFT WHITE PARENTHESIS
// ?? 2986          ; Math # Pe       RIGHT WHITE PARENTHESIS
// ?? 2987          ; Math # Ps       Z NOTATION LEFT IMAGE BRACKET
// ?? 2988          ; Math # Pe       Z NOTATION RIGHT IMAGE BRACKET
// ?? 2989          ; Math # Ps       Z NOTATION LEFT BINDING BRACKET
// ?? 298A          ; Math # Pe       Z NOTATION RIGHT BINDING BRACKET
// ?? 298B          ; Math # Ps       LEFT SQUARE BRACKET WITH UNDERBAR
// ?? 298C          ; Math # Pe       RIGHT SQUARE BRACKET WITH UNDERBAR
// ?? 298D          ; Math # Ps       LEFT SQUARE BRACKET WITH TICK IN TOP CORNER
// ?? 298E          ; Math # Pe       RIGHT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
// ?? 298F          ; Math # Ps       LEFT SQUARE BRACKET WITH TICK IN BOTTOM CORNER
// ?? 2990          ; Math # Pe       RIGHT SQUARE BRACKET WITH TICK IN TOP CORNER
// ?? 2991          ; Math # Ps       LEFT ANGLE BRACKET WITH DOT
// ?? 2992          ; Math # Pe       RIGHT ANGLE BRACKET WITH DOT
// ?? 2993          ; Math # Ps       LEFT ARC LESS-THAN BRACKET
// ?? 2994          ; Math # Pe       RIGHT ARC GREATER-THAN BRACKET
// ?? 2995          ; Math # Ps       DOUBLE LEFT ARC GREATER-THAN BRACKET
// ?? 2996          ; Math # Pe       DOUBLE RIGHT ARC LESS-THAN BRACKET
// ?? 2997          ; Math # Ps       LEFT BLACK TORTOISE SHELL BRACKET
// ?? 2998          ; Math # Pe       RIGHT BLACK TORTOISE SHELL BRACKET
// ?? 2999..29D7    ; Math # Sm  [63] DOTTED FENCE..BLACK HOURGLASS
// ?? 29D8          ; Math # Ps       LEFT WIGGLY FENCE
// ?? 29D9          ; Math # Pe       RIGHT WIGGLY FENCE
// ?? 29DA          ; Math # Ps       LEFT DOUBLE WIGGLY FENCE
// ?? 29DB          ; Math # Pe       RIGHT DOUBLE WIGGLY FENCE
// ?? 29DC..29FB    ; Math # Sm  [32] INCOMPLETE INFINITY..TRIPLE PLUS
// ?? 29FC          ; Math # Ps       LEFT-POINTING CURVED ANGLE BRACKET
// ?? 29FD          ; Math # Pe       RIGHT-POINTING CURVED ANGLE BRACKET
// ?? 29FE..2AFF    ; Math # Sm [258] TINY..N-ARY WHITE VERTICAL BAR
  {0xFB29,0xFB29},  // HEBREW LETTER ALTERNATIVE PLUS SIGN
  {0xFE61,0xFE66},
// ?? FE61          ; Math # Po       SMALL ASTERISK
// ?? FE62          ; Math # Sm       SMALL PLUS SIGN
// ?? FE63          ; Math # Pd       SMALL HYPHEN-MINUS
// ?? FE64..FE66    ; Math # Sm   [3] SMALL LESS-THAN SIGN..SMALL EQUALS SIGN
  {0xFE68,0xFE68},  //  SMALL REVERSE SOLIDUS
  {0xFF0B,0xFF0B},  //  FULLWIDTH PLUS SIGN
  {0xFF1C,0xFF1E},  //  FULLWIDTH LESS-THAN SIGN..FULLWIDTH GREATER-THAN SIGN
  {0xFF3C,0xFF3C},  //  FULLWIDTH REVERSE SOLIDUS
  {0xFF3E,0xFF3E},  //  FULLWIDTH CIRCUMFLEX ACCENT
  {0xFF5C,0xFF5C},  //  FULLWIDTH VERTICAL LINE
  {0xFF5E,0xFF5E},  //  FULLWIDTH TILDE
  {0xFFE2,0xFFE2},  //  FULLWIDTH NOT SIGN
  {0xFFE9,0xFFEC}   //  HALFWIDTH LEFTWARDS ARROW..HALFWIDTH DOWNWARDS ARROW
  // list must be sorted
};

// A linear search through the list is OK, since most chars we'll see are ASCII
static
PRBool CharacterIsOperator(PRUint32 ch)
{
  const int listLength = sizeof(OperatorList)/sizeof(OperatorList[0]);
  
  for (int i = 0; i < listLength; i++) {
    if (OperatorList[i].lower <= ch && ch <= OperatorList[i].higher)
      return PR_TRUE;
    else if (OperatorList[i].higher > ch)
      return PR_FALSE;
  }
  return PR_FALSE;
}

PRInt32 msiUtils::GetMathMLNodeTypeFromCharacter(PRUint32 character)
{
  if (character < 0xFF && isdigit(character&0xFF))
    return msiIMathMLEditingBC::MATHML_MN;
  else if (CharacterIsOperator(character))
    return msiIMathMLEditingBC::MATHML_MO;
  else
    return msiIMathMLEditingBC::MATHML_MI;

}
nsresult msiUtils::CreateMathMLElement(nsIEditor* editor, nsIAtom* type,
                                        nsCOMPtr<nsIDOMElement> & mmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  NS_ASSERTION(editor && type, "Null editor or nodetype!");
  if (!editor || !type)
    return NS_ERROR_FAILURE;
  nsCOMPtr<nsIDOMDocument> domDoc;
  editor->GetDocument(getter_AddRefs(domDoc));
  NS_ASSERTION(domDoc, "Editor GetDocument return Null DOMDocument!");
  nsAutoString name;
  type->ToString(name);
  if (domDoc && !name.IsEmpty())
  {
    nsAutoString mmlnsURI;
    msiNameSpaceUtils::GetNameSpaceURI(kNameSpaceID_MathML, mmlnsURI);
    res = domDoc->CreateElementNS(mmlnsURI, name, getter_AddRefs(mmlElement));
  }
  return res;  
}                                       

nsresult msiUtils::CreateMathElement(nsIEditor * editor,
                                     PRBool isDisplay,
                                     PRBool markCaret,
                                     PRUint32 & flags,
                                     nsCOMPtr<nsIDOMElement> & mathElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  res = CreateMathMLElement(editor, msiEditingAtoms::math, mathElement);
  if (NS_SUCCEEDED(res) && mathElement) 
  {
    nsAutoString xmlnsURI, mmlnsURI;
    PRBool nestInMrow = MROW_PURGE_NONE == GetMrowPurgeMode() ? PR_TRUE : PR_FALSE; 
    msiNameSpaceUtils::GetNameSpaceURI(kNameSpaceID_MathML, mmlnsURI);
    msiNameSpaceUtils::GetNameSpaceURI(kNameSpaceID_XMLNS, xmlnsURI);
    nsAutoString xmlns, display, block;
    xmlns = NS_LITERAL_STRING("xmlns");
    msiEditingAtoms::display->ToString(display);
    msiEditingAtoms::block->ToString(block);
    mathElement->SetAttributeNS(xmlnsURI, xmlns, mmlnsURI);
    if (isDisplay)
      mathElement->SetAttribute(display, block);
    nsCOMPtr<nsIDOMElement> inputbox;
    res = CreateInputbox(editor, nestInMrow, markCaret, flags, inputbox);
    nsCOMPtr<nsIDOMNode> inputboxNode(do_QueryInterface(inputbox));
    if (NS_SUCCEEDED(res) && inputboxNode) 
    {
      nsCOMPtr<nsIDOMNode> dontcare;
      res = mathElement->AppendChild(inputboxNode, getter_AddRefs(dontcare));
    }
  }
  return res;
}

nsresult msiUtils::CreateMathMLLeafElement(nsIEditor * editor,
                                           const nsAString & text,
                                           PRUint32 tagType,
                                           PRUint32 caretPos,
                                           PRUint32 & flags, 
                                           nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMElement> leafElement;
  NS_ASSERTION(!text.IsEmpty(), "Trying to create leaf with empty data string.");
  if (!text.IsEmpty())
  {
    res = NS_OK;
    nsIAtom * elementAtom;
    if (tagType == msiIMathMLEditingBC::MATHML_MO)
      elementAtom = msiEditingAtoms::mo;
    else if (tagType == msiIMathMLEditingBC::MATHML_MN)
      elementAtom = msiEditingAtoms::mn;
    else
      elementAtom = msiEditingAtoms::mi;
    res = CreateMathMLElement(editor, elementAtom, leafElement);
    if (NS_SUCCEEDED(res) && leafElement) 
    {
      nsCOMPtr<nsIDOMText> textNode;
      nsCOMPtr<nsIDOMDocument> domDoc;
      NS_ASSERTION(editor, "Null editor!");
      if (editor)
      {
        editor->GetDocument(getter_AddRefs(domDoc));
        NS_ASSERTION(domDoc, "Editor GetDocument return Null DOMDocument!");
      }
      if (domDoc)
      {
        res = domDoc->CreateTextNode(text, getter_AddRefs(textNode));
        if (NS_SUCCEEDED(res)) 
        {
          nsCOMPtr<nsIDOMNode> resultNode;
          nsCOMPtr<nsIDOMNode> newNode(do_QueryInterface(textNode));  
          res = leafElement->AppendChild(newNode, getter_AddRefs(resultNode));
          if (NS_SUCCEEDED(res)) 
          {
            if (caretPos <= msiIMathMLEditingBC::LAST_VALID)
            {
              nsCOMPtr<nsIDOMNode> leafNode(do_QueryInterface(leafElement));  
              if (leafNode)
                MarkCaretPosition(editor, leafNode, caretPos, flags, PR_TRUE, PR_TRUE);
            }
            mathmlElement = leafElement;
          }  
        }
      }  
    }    
  }    
  return res;
}

nsresult msiUtils::CreateMathMLLeafElement(nsIEditor * editor,
                                           PRUint32 character,
                                           PRUint32 tagType, 
                                           PRUint32 caretPos, 
                                           PRUint32 & flags,
                                           nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(character, "Trying to create leaf with character == 0");
  if (character)
  {
    nsAutoString text(static_cast<PRUnichar>(character));
    res = CreateMathMLLeafElement(editor, text, tagType, caretPos, flags, mathmlElement);
  }    
  return res;
}

nsresult msiUtils::CreateMathMLLeafElement(nsIEditor *editor,
                                           const nsAString & text,
                                           PRUint32 caretPos,
                                           PRUint32 & flags,
                                           nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE);
  NS_ASSERTION(text[0], "Trying to create leaf with character == 0");
  if (text[0])
  {
    PRUint32 tagType = GetMathMLNodeTypeFromCharacter(text[0]);
//    nsAutoString text((PRUnichar)character);
    res = CreateMathMLLeafElement(editor, text, tagType, caretPos, flags, mathmlElement);
  }
  return res;  
}                                 

nsresult msiUtils::CreateMathOperator(nsIEditor * editor,
                                      const nsAString & text,
                                      PRUint32 caretPos,
                                      PRUint32 & flags,
                                      PRUint32 attrFlags,
                                      const nsAString & lspace,                                 
                                      const nsAString & rspace,                                 
                                      const nsAString & minsize,                                 
                                      const nsAString & maxsize,
                                      nsCOMPtr<nsIDOMElement> & mathElement)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMElement> moElement;
  res = CreateMathMLLeafElement(editor, text, msiIMathMLEditingBC::MATHML_MO, caretPos, flags, moElement);
  if (NS_SUCCEEDED(res) && moElement)   
  {
    nsAutoString form, prefix, postfix, infix, stretchy, msitrue, msifalse, fence;
    nsAutoString msiBoundFence, largeop, separator, movablelimits, symmetric;
    nsAutoString lspace, rspace, maxsize, minsize, accent;
    nsAutoString msiLimitPlacement, msiLimitsAtRight, msiLimitsAboveBelow, displaystyle; 
    msiEditingAtoms::form->ToString(form);
    msiEditingAtoms::prefix->ToString(prefix);
    msiEditingAtoms::postfix->ToString(postfix);
    msiEditingAtoms::infix->ToString(infix);
    msiEditingAtoms::stretchy->ToString(stretchy);
    msiEditingAtoms::msitrue->ToString(msitrue);
    msiEditingAtoms::msifalse->ToString(msifalse);
    msiEditingAtoms::fence->ToString(fence);
    msiEditingAtoms::largeop->ToString(largeop);
    msiEditingAtoms::msiBoundFence->ToString(msiBoundFence);
    msiEditingAtoms::separator->ToString(separator);
    msiEditingAtoms::movablelimits->ToString(movablelimits);
    msiEditingAtoms::symmetric->ToString(symmetric);
    msiEditingAtoms::lspace->ToString(lspace);
    msiEditingAtoms::rspace->ToString(rspace);
    msiEditingAtoms::maxsize->ToString(maxsize);
    msiEditingAtoms::minsize->ToString(minsize);
    msiEditingAtoms::accent->ToString(accent);
    msiEditingAtoms::msiLimitPlacement->ToString(msiLimitPlacement);
    msiEditingAtoms::msiLimitsAtRight->ToString(msiLimitsAtRight);
    msiEditingAtoms::msiLimitsAboveBelow->ToString(msiLimitsAboveBelow);
    msiEditingAtoms::displaystyle->ToString(displaystyle);
    
    if (attrFlags & msiIMMLEditDefines::MO_ATTR_prefix)
      res = moElement->SetAttribute(form, prefix);
    else if (attrFlags & msiIMMLEditDefines::MO_ATTR_postfix)
      res = moElement->SetAttribute(form, postfix);
    else if (attrFlags & msiIMMLEditDefines::MO_ATTR_infix)
      res = moElement->SetAttribute(form, infix);
    
    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_stretchy_T))
      res = moElement->SetAttribute(stretchy, msitrue);
    else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_stretchy_F))
      res = moElement->SetAttribute(stretchy, msifalse);
    
    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_fence_T))
      res = moElement->SetAttribute(fence, msitrue);
    else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_fence_F))
      res = moElement->SetAttribute(fence, msifalse);
    
    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_boundFence))
      res = moElement->SetAttribute(msiBoundFence, msitrue);
    
    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_accent_T))
      res = moElement->SetAttribute(accent, msitrue);
    else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_accent_F))
      res = moElement->SetAttribute(accent, msifalse);
    
    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_largeop_T))
      res = moElement->SetAttribute(largeop, msitrue);
    else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_largeop_F))
      res = moElement->SetAttribute(largeop, msifalse);
    
    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_separator_T))
      res = moElement->SetAttribute(separator, msitrue);
    else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_separator_F))
      res = moElement->SetAttribute(separator, msifalse);
    
    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_movablelimits_T))
      res = moElement->SetAttribute(movablelimits, msitrue);
    else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_movablelimits_F))
      res = moElement->SetAttribute(movablelimits, msifalse);
    
    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_symmetric_T))
      res = moElement->SetAttribute(symmetric, msitrue);
    else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_symmetric_F))
      res = moElement->SetAttribute(symmetric, msifalse);
    
    if (NS_SUCCEEDED(res) && !lspace.IsEmpty())
      res = moElement->SetAttribute(lspace, lspace);
    
    if (NS_SUCCEEDED(res) && !rspace.IsEmpty())
      res = moElement->SetAttribute(rspace, rspace);
    
    if (NS_SUCCEEDED(res) && !maxsize.IsEmpty())
      res = moElement->SetAttribute(maxsize, maxsize);
    
    if (NS_SUCCEEDED(res) && !minsize.IsEmpty())
      res = moElement->SetAttribute(minsize, minsize);

    if (NS_SUCCEEDED(res))
    {
      if (attrFlags & msiIMMLEditDefines::MO_ATTR_atRightLimits)
        res = moElement->SetAttribute(msiLimitPlacement, msiLimitsAtRight);
      else if (attrFlags & msiIMMLEditDefines::MO_ATTR_aboveBelowLimits)
        res = moElement->SetAttribute(msiLimitPlacement, msiLimitsAboveBelow);
    }

    if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_sizeFlags))
    {
      nsCOMPtr<nsIDOMElement> styleElement;
      res = WrapNodeInMStyle(editor, moElement, styleElement);
      if (NS_SUCCEEDED(res) && styleElement)
      {
        if (attrFlags & msiIMMLEditDefines::MO_ATTR_displaySize)
          res = styleElement->SetAttribute(displaystyle, msitrue);
        else if (attrFlags & msiIMMLEditDefines::MO_ATTR_smallSize)
          res = styleElement->SetAttribute(displaystyle, msifalse);
        mathElement = styleElement;
      }
    }
    else
      mathElement = moElement;
  } 
  return res;
}                            

nsresult msiUtils::CreateInputbox(nsIEditor *editor,
                                  PRBool nestInRow,
                                  PRBool markCaret,
                                  PRUint32 & flags,
                                  nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsIDOMElement> inputbox;
  PRUint32 dummyFlags(msiIMathMLInsertion::FLAGS_NONE);
  nsAutoString text(PRUnichar(0x200A)); // hair width space, so cursor shows
  res = CreateMathMLLeafElement(editor, text, msiIMathMLEditingBC::MATHML_MI, -1, dummyFlags, inputbox);
  if (NS_SUCCEEDED(res) && inputbox)
  {
    nsAutoString tempinput, msitrue;
    msiEditingAtoms::tempinput->ToString(tempinput);
    msiEditingAtoms::msitrue->ToString(msitrue);
    res = inputbox->SetAttribute(tempinput, msitrue);
    if (NS_SUCCEEDED(res) && markCaret)
    {
        nsCOMPtr<nsIDOMNode> inputnode(do_QueryInterface(inputbox));
        if (inputnode)
          MarkCaretPosition(editor, inputnode, 1, flags, PR_TRUE, PR_TRUE);
    }
    if (nestInRow)
      res = CreateMRow(editor, inputbox, mathmlElement);
     else
       mathmlElement = inputbox;
  }
  return res;
}

nsresult msiUtils::CreateMSubOrMSup(nsIEditor * editor,
                                    PRBool isSup,
                                    nsIDOMNode * base,
                                    nsIDOMNode * script,
                                    PRBool scriptInRow,
                                    PRBool markCaret,
                                    PRUint32 & flags,
                                    const nsAString & scriptShift,                                 
                                    nsCOMPtr<nsIDOMElement> & mathmlElement)
{  
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMElement> subOrSup;
  nsCOMPtr<nsIDOMNode> theBase, theScript, tmpScript;
  tmpScript = nsnull;
  nsAutoString subscriptshift, superscriptshift;
  msiEditingAtoms::subscriptshift->ToString(subscriptshift);
  msiEditingAtoms::superscriptshift->ToString(superscriptshift);
  if (isSup)
  {
    res = CreateMathMLElement(editor, msiEditingAtoms::msup, subOrSup);
    if (NS_SUCCEEDED(res) && subOrSup && !scriptShift.IsEmpty())
      subOrSup->SetAttribute(superscriptshift, scriptShift);
  }
  else
  {
    res = CreateMathMLElement(editor, msiEditingAtoms::msub, subOrSup);
    if (NS_SUCCEEDED(res) && subOrSup && !scriptShift.IsEmpty())
      subOrSup->SetAttribute(subscriptshift, scriptShift);
  }
  if (NS_SUCCEEDED(res) && subOrSup) 
  {
    if (base)
      theBase = base;
    else
    {
      nsCOMPtr<nsIDOMElement> inputbox;
      PRBool locMarkCaret(markCaret);
      if (locMarkCaret && script == nsnull)
        locMarkCaret = PR_FALSE;  // mark script inputbox instead
      // BBM
      locMarkCaret = PR_TRUE;
      res = CreateInputbox(editor, PR_FALSE, locMarkCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        theBase = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    }
    if (script)
      tmpScript = script;
    else
    {    
      nsCOMPtr<nsIDOMElement> inputbox;
      res = CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        tmpScript = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res) && tmpScript) 
    {
      if (scriptInRow)
      {
        nsCOMPtr<nsIDOMElement> mrow;
        res = msiUtils::CreateMRow(editor, tmpScript, mrow);
        if (NS_SUCCEEDED(res) && mrow)
          theScript = do_QueryInterface(mrow);
      }
      else
        theScript = tmpScript;
    }
    if (NS_SUCCEEDED(res) && subOrSup && theBase && theScript) 
    {
      nsCOMPtr<nsIDOMNode> dontcare, dontcare1;
      res = subOrSup->AppendChild(theBase, getter_AddRefs(dontcare));
      if (NS_SUCCEEDED(res)) 
        res = subOrSup->AppendChild(theScript, getter_AddRefs(dontcare1));
      if (NS_SUCCEEDED(res))
        mathmlElement = subOrSup; 
    }  
  }
  return res;
}   

nsresult msiUtils::CreateMSubSup(nsIEditor * editor,
                                 nsIDOMNode * base,
                                 nsIDOMNode * subscript,
                                 nsIDOMNode * supscript,
                                 PRBool subscriptInRow,
                                 PRBool supscriptInRow,
                                 PRBool markCaret,
                                 PRUint32 & flags,
                                 const nsAString & subscriptShift,                                 
                                 const nsAString & supscriptShift,                                 
                                 nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMElement> subsup;
  nsCOMPtr<nsIDOMNode> theBase, thesubScript, tmpsubScript, thesupScript, tmpsupScript;
  tmpsubScript = nsnull;
  tmpsupScript = nsnull;
  res = CreateMathMLElement(editor, msiEditingAtoms::msubsup, subsup);
  nsAutoString subscriptshift, superscriptshift;
  msiEditingAtoms::subscriptshift->ToString(subscriptshift);
  msiEditingAtoms::superscriptshift->ToString(superscriptshift);
  if (NS_SUCCEEDED(res) && subsup) 
  {
    if (!subscriptShift.IsEmpty())
      subsup->SetAttribute(subscriptshift, subscriptShift);
    if (!supscriptShift.IsEmpty())
      subsup->SetAttribute(superscriptshift, supscriptShift);
    if (base)
      theBase = base;
    else
    {
      nsCOMPtr<nsIDOMElement> inputbox;
      PRBool locMarkCaret(markCaret);
      if (locMarkCaret && (subscript == nsnull || supscript == nsnull))
        locMarkCaret = PR_FALSE;  // mark script inputbox instead
      res = CreateInputbox(editor, PR_FALSE, locMarkCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        theBase = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    }
    if (subscript)
      tmpsubScript = subscript;
    else
    {    
      nsCOMPtr<nsIDOMElement> inputbox;
      PRBool locMarkCaret(markCaret);
      if (locMarkCaret && supscript == nsnull)
        locMarkCaret = PR_FALSE;  // mark supscript inputbox instead
      res = CreateInputbox(editor, PR_FALSE, locMarkCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        tmpsubScript = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res) && tmpsubScript) 
    {
      if (subscriptInRow)
      {
        nsCOMPtr<nsIDOMElement> mrow;
        res = msiUtils::CreateMRow(editor, tmpsubScript, mrow);
        if (NS_SUCCEEDED(res) && mrow)
          thesubScript = do_QueryInterface(mrow);
      }
      else
        thesubScript = tmpsubScript;
    }

    if (supscript)
      tmpsupScript = supscript;
    else
    {    
      nsCOMPtr<nsIDOMElement> inputbox;
      res = CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        tmpsupScript = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res) && tmpsupScript) 
    {
      if (supscriptInRow)
      {
        nsCOMPtr<nsIDOMElement> mrow;
        res = msiUtils::CreateMRow(editor, tmpsupScript, mrow);
        if (NS_SUCCEEDED(res) && mrow)
          thesupScript = do_QueryInterface(mrow);
      }
      else
        thesupScript = tmpsupScript;
    }
    if (NS_SUCCEEDED(res) && subsup && theBase && thesubScript && thesupScript) 
    {
      nsCOMPtr<nsIDOMNode> dontcare, dontcare1, dontcare2;
      res = subsup->AppendChild(theBase, getter_AddRefs(dontcare));
      if (NS_SUCCEEDED(res)) 
        res = subsup->AppendChild(thesubScript, getter_AddRefs(dontcare1));
      if (NS_SUCCEEDED(res)) 
        res = subsup->AppendChild(thesupScript, getter_AddRefs(dontcare2));
      if (NS_SUCCEEDED(res))
        mathmlElement = subsup; 
    }  
  }
  return res;
}

nsresult msiUtils::CreateDecoration(nsIEditor * editor,
                                   nsIDOMNode * base,
                                   const nsAString & above,
                                   const nsAString & below,
                                   const nsAString & aroundNotation,
                                   const nsAString & aroundType,
                                   PRBool createInputBox,
                                   PRBool markCaret,
                                   PRUint32 & flags,
                                   nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && (!above.IsEmpty() || !below.IsEmpty() || !aroundNotation.IsEmpty() || !aroundType.IsEmpty()))
  {
    res = NS_OK;
    PRBool isOverUnder = !above.IsEmpty() && !below.IsEmpty();
    PRBool isOver = !above.IsEmpty() && below.IsEmpty();
    PRBool isOverOrUnder = !above.IsEmpty() || !below.IsEmpty();
    PRUint32 dummyFlags(0);
    nsCOMPtr<nsIDOMElement> over, under, enclosed;
    nsCOMPtr<nsIDOMNode> overNode, underNode, enclosedNode;
    nsAutoString emptyString;
    if (!above.IsEmpty() && !(msiEditingAtoms::label->Equals(above)))
    {
      res = CreateMathOperator(editor, above, msiIMathMLEditingBC::INVALID, dummyFlags, 
                               msiIMMLEditDefines::MO_ATTR_stretchy_T, emptyString,
                               emptyString, emptyString, emptyString, over); 
      dummyFlags = 0;
      if (NS_SUCCEEDED(res) && over)
        overNode = do_QueryInterface(over);
    }
    if (NS_SUCCEEDED(res) &&!below.IsEmpty() && !(msiEditingAtoms::label->Equals(below)))
    {
      res = CreateMathOperator(editor, below, msiIMathMLEditingBC::INVALID, dummyFlags, 
                               msiIMMLEditDefines::MO_ATTR_stretchy_T, emptyString,
                               emptyString, emptyString, emptyString, under); 
      if (NS_SUCCEEDED(res) && under)
        underNode = do_QueryInterface(under);
    }
    if (NS_SUCCEEDED(res) && isOverUnder)
      res = CreateMunderover(editor, base, underNode, overNode, PR_FALSE, PR_FALSE,
                             createInputBox, PR_TRUE, flags, emptyString, emptyString, enclosed);
    else if (NS_SUCCEEDED(res) && isOverOrUnder)
    {
      nsCOMPtr<nsIDOMNode> script = isOver ? overNode : underNode;
      res = CreateMunderOrMover(editor, isOver, base, script, PR_FALSE, createInputBox, PR_TRUE, flags, 
                                emptyString, enclosed);
    }
    if (NS_SUCCEEDED(res))
    {
      if (enclosed)
        enclosedNode = do_QueryInterface(enclosed);
      else
        enclosedNode = base;
    }
    if (NS_SUCCEEDED(res) && !aroundNotation.IsEmpty())
    {
      res = CreateMenclose(editor, aroundNotation, aroundType, enclosedNode, createInputBox, markCaret, flags, mathmlElement);
    }
    else if (NS_SUCCEEDED(res) && enclosed)
      mathmlElement = enclosed;
  }
  return res;
}                                   
                                   
nsresult msiUtils::CreateMunderOrMover(nsIEditor *editor,
                                       PRBool isOver,
                                       nsIDOMNode * base,
                                       nsIDOMNode * script,
                                       PRBool scriptInRow,
                                       PRBool createInputBox,
                                       PRBool markCaret,
                                       PRUint32 & flags,
                                       const nsAString & isAccent,                                 
                                       nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr<nsISelection> sel;
  nsCOMPtr<nsIDOMElement> underOrOver;
  nsCOMPtr<nsIDOMNode> theBase, theScript, tmpScript;
  tmpScript = nsnull;
  nsAutoString accent, accentunder;
  msiEditingAtoms::accent->ToString(accent);
  msiEditingAtoms::accentunder->ToString(accentunder);
  
  if (isOver)
  {
    res = CreateMathMLElement(editor, msiEditingAtoms::mover, underOrOver);
    if (NS_SUCCEEDED(res) && underOrOver && !isAccent.IsEmpty())
      underOrOver->SetAttribute(accent, isAccent);
  }
  else
  {
    res = CreateMathMLElement(editor, msiEditingAtoms::munder, underOrOver);
    if (NS_SUCCEEDED(res) && underOrOver && !isAccent.IsEmpty())
      underOrOver->SetAttribute(accentunder, isAccent);
  }
  if (NS_SUCCEEDED(res) && underOrOver) 
  {
    if (base)
      theBase = base;
    else if (createInputBox)
    {
      nsCOMPtr<nsIDOMElement> inputbox;
      PRBool locMarkCaret(markCaret);
      if (locMarkCaret && script == nsnull)
        locMarkCaret = PR_FALSE;  // mark script inputbox instead
      res = CreateInputbox(editor, PR_FALSE, locMarkCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        theBase = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    } else
    {
      nsCOMPtr<nsIArray> nodeArray;  //deliberately empty
      nsCOMPtr<nsIDOMElement> baseElement;
      CreateMRow(editor, (nsIArray *) nodeArray, baseElement);
      theBase = baseElement;
    }

    if (script)
      tmpScript = script;
    else
    {    
      nsCOMPtr<nsIDOMElement> inputbox;
      res = CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        tmpScript = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res) && tmpScript) 
    {
      if (scriptInRow)
      {
        nsCOMPtr<nsIDOMElement> mrow;
        res = msiUtils::CreateMRow(editor, tmpScript, mrow);
        if (NS_SUCCEEDED(res) && mrow)
          theScript = do_QueryInterface(mrow);
      }
      else
        theScript = tmpScript;
    }
    if (NS_SUCCEEDED(res) && underOrOver && theBase && theScript) 
    {
      nsCOMPtr<nsIDOMNode> dontcare, dontcare1;
      res = underOrOver->AppendChild(theBase, getter_AddRefs(dontcare));
      if (NS_SUCCEEDED(res)) 
        res = underOrOver->AppendChild(theScript, getter_AddRefs(dontcare1));
      if (NS_SUCCEEDED(res)) {
        res = editor->GetSelection(getter_AddRefs(sel));
        if (NS_SUCCEEDED(res))
          res = sel->Collapse(theScript, 0);
      }
      if (NS_SUCCEEDED(res))
        mathmlElement = underOrOver; 
    }  
  }
  return res;
}   

nsresult msiUtils::CreateMunderover(nsIEditor * editor,
                                    nsIDOMNode * base,
                                    nsIDOMNode * underscript,
                                    nsIDOMNode * overscript,
                                    PRBool underscriptInRow,
                                    PRBool overscriptInRow,
                                    PRBool createInputBox, 
                                    PRBool markCaret,
                                    PRUint32 & flags,
                                    const nsAString & isunderAccent,                                 
                                    const nsAString & isoverAccent,                                 
                                    nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMElement> underover;
  nsCOMPtr<nsIDOMNode> theBase, theunderScript, tmpunderScript, theoverScript, tmpoverScript;
  res = CreateMathMLElement(editor, msiEditingAtoms::munderover, underover);
  nsAutoString accent, accentunder;
  msiEditingAtoms::accent->ToString(accent);
  msiEditingAtoms::accentunder->ToString(accentunder);
  if (NS_SUCCEEDED(res) && underover) 
  {
    if (!isunderAccent.IsEmpty())
      underover->SetAttribute(accentunder, isunderAccent);
    if (!isoverAccent.IsEmpty())
      underover->SetAttribute(accent, isoverAccent);
    if (base)
      theBase = base;
    else if (createInputBox)
    {
      nsCOMPtr<nsIDOMElement> inputbox;
      PRBool locMarkCaret(markCaret);
      if (locMarkCaret && (underscript == nsnull || overscript == nsnull))
        locMarkCaret = PR_FALSE;  // mark script inputbox instead
      res = CreateInputbox(editor, PR_FALSE, locMarkCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        theBase = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    } else
    {
      nsCOMPtr<nsIArray> nodeArray;  //deliberately empty
      nsCOMPtr<nsIDOMElement> baseElement;
      CreateMRow(editor, (nsIArray *) nodeArray, baseElement);
      theBase = baseElement;
    }
    if (underscript)
      tmpunderScript = underscript;
    else
    {    
      nsCOMPtr<nsIDOMElement> inputbox;
      PRBool locMarkCaret(markCaret);
      if (locMarkCaret && overscript == nsnull)
        locMarkCaret = PR_FALSE;  // mark overscript inputbox instead
      res = CreateInputbox(editor, PR_FALSE, locMarkCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        tmpunderScript = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res) && tmpunderScript) 
    {
      if (underscriptInRow)
      {
        nsCOMPtr<nsIDOMElement> mrow;
        res = msiUtils::CreateMRow(editor, tmpunderScript, mrow);
        if (NS_SUCCEEDED(res) && mrow)
          theunderScript = do_QueryInterface(mrow);
      }
      else
        theunderScript = tmpunderScript;
    }

    if (overscript)
      tmpoverScript = overscript;
    else
    {    
      nsCOMPtr<nsIDOMElement> inputbox;
      res = CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        tmpoverScript = do_QueryInterface(inputbox);
      else
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res) && tmpoverScript) 
    {
      if (overscriptInRow)
      {
        nsCOMPtr<nsIDOMElement> mrow;
        res = msiUtils::CreateMRow(editor, tmpoverScript, mrow);
        if (NS_SUCCEEDED(res) && mrow)
          theoverScript = do_QueryInterface(mrow);
      }
      else
        theoverScript = tmpoverScript;
    }
    if (NS_SUCCEEDED(res) && underover && theBase && theunderScript && theoverScript) 
    {
      nsCOMPtr<nsIDOMNode> dontcare, dontcare1, dontcare2;
      res = underover->AppendChild(theBase, getter_AddRefs(dontcare));
      if (NS_SUCCEEDED(res)) 
        res = underover->AppendChild(theunderScript, getter_AddRefs(dontcare1));
      if (NS_SUCCEEDED(res)) 
        res = underover->AppendChild(theoverScript, getter_AddRefs(dontcare2));
      if (NS_SUCCEEDED(res))
        mathmlElement = underover; 
    }  
  }
  return res;
}

nsresult msiUtils::CreateMenclose(nsIEditor *editor,
                                  const nsAString & notationAttr,
                                  const nsAString & typeAttr,
                                  nsIDOMNode * child,
                                  PRBool makeInputBox,
                                  PRBool markCaret,
                                  PRUint32 & flags,
                                  nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMNode> enclosed;
  nsCOMPtr<nsIDOMElement> enclose;
  res = CreateMathMLElement(editor, msiEditingAtoms::menclose, enclose);

  if (NS_SUCCEEDED(res) && enclose) 
  {
    nsAutoString notation, msiTypeAttr;
    msiEditingAtoms::typeAttr->ToString(msiTypeAttr);
    msiEditingAtoms::notation->ToString(notation);
    if (NS_SUCCEEDED(res) && !notationAttr.IsEmpty())
      res = enclose->SetAttribute(notation, notationAttr);
    if (NS_SUCCEEDED(res) && !typeAttr.IsEmpty())
      res = enclose->SetAttribute(msiTypeAttr, typeAttr);
  }
    
  if (NS_SUCCEEDED(res) && enclose) 
  {
    if (child)
      enclosed = child;
    else if (makeInputBox)
    {
      nsCOMPtr<nsIDOMElement> inputbox;
      res = CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        enclosed = do_QueryInterface(inputbox);
    } else
    {
      nsCOMPtr<nsIArray> nodeArray;  //deliberately empty
      nsCOMPtr<nsIDOMElement> enclosedElement = do_QueryInterface(enclosed);
      CreateMRow(editor, (nsIArray *) nodeArray, enclosedElement);
      enclosed = enclosedElement;
    }
    if (NS_SUCCEEDED(res) && enclosed)  
    {
      nsCOMPtr<nsIDOMNode> dontcare;   //TODO -- do these need to be saved for undo!!!!!!!
      res = enclose->AppendChild(enclosed, getter_AddRefs(dontcare));
      if (NS_SUCCEEDED(res))
        mathmlElement = enclose;
    }  
  }
  return res;
} 

nsresult msiUtils::CreateMsqrt(nsIEditor *editor,
                               nsIDOMNode * child,
                               PRBool makeInputBox,
                               PRBool markCaret,
                               PRUint32 & flags,
                               nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMNode> radicand;
  nsCOMPtr<nsIDOMElement> sqrt;
  res = CreateMathMLElement(editor, msiEditingAtoms::msqrt, sqrt);
  if (NS_SUCCEEDED(res) && sqrt) 
  {
    if (child)
      radicand = child;
    else if (makeInputBox)
    {
      nsCOMPtr<nsIDOMElement> inputbox;
      res = CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
      if (NS_SUCCEEDED(res) && inputbox) 
        radicand = do_QueryInterface(inputbox);
    } else
    {
      nsCOMPtr<nsIArray> nodeArray;  //deliberately empty
      nsCOMPtr<nsIDOMElement> radElement = do_QueryInterface(radicand);
      CreateMRow(editor, (nsIArray *) nodeArray, radElement);
      radicand = radElement;
    }
    if (NS_SUCCEEDED(res) && radicand)  
    {
      nsCOMPtr<nsIDOMNode> dontcare;   //TODO -- do these need to be saved for undo!!!!!!!
      res = sqrt->AppendChild(radicand, getter_AddRefs(dontcare));
      if (NS_SUCCEEDED(res))
        mathmlElement = sqrt;
    }  
  }
  return res;
} 

nsresult msiUtils::CreateMroot(nsIEditor * editor,
                               nsIDOMNode * radicand,
                               nsIDOMNode * index,
                               PRBool createInputBox,
                               PRBool markCaret,
                               PRUint32 & flags,
                               nsCOMPtr<nsIDOMElement> & mathmlElement)
{  
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMNode> loc_radicand, loc_index;
  nsCOMPtr<nsIDOMElement> root;
  res = CreateMathMLElement(editor, msiEditingAtoms::mroot, root);
  if (NS_SUCCEEDED(res) && root) 
  {
    if (radicand)
      loc_radicand = radicand;
    else if (createInputBox)
    {
      nsCOMPtr<nsIDOMElement> inputbox;
      CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
      NS_ASSERTION(inputbox, "CreateInputbox failed.");
      if (inputbox) 
        loc_radicand = do_QueryInterface(inputbox);
    } else
    {
      nsCOMPtr<nsIArray> nodeArray;  //deliberately empty
      nsCOMPtr<nsIDOMElement> radElement = do_QueryInterface(loc_radicand);
      CreateMRow(editor, (nsIArray *) nodeArray, radElement);
      loc_radicand = radElement;
    }
    if (index)
      loc_index = index;
    else
    {
      PRBool doMarkCaret = (radicand && markCaret) ? PR_TRUE : PR_FALSE;
      nsCOMPtr<nsIDOMElement> inputbox;
      res = CreateInputbox(editor, PR_FALSE, doMarkCaret, flags, inputbox);
      NS_ASSERTION(inputbox, "CreateInputbox failed.");
      if (inputbox) 
        loc_index = do_QueryInterface(inputbox);
    }
    if (loc_radicand && loc_index)
    {
      nsCOMPtr<nsIDOMNode> dontcare;
      res = root->AppendChild(loc_radicand, getter_AddRefs(dontcare));
      dontcare = nsnull;
      if (NS_SUCCEEDED(res)) 
        res = root->AppendChild(loc_index, getter_AddRefs(dontcare));
      if (NS_SUCCEEDED(res)) 
          mathmlElement = root;
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  return res;
}        

nsresult msiUtils::CreateMathname(nsIEditor * editor,
                                  const nsAString & name,
                                  PRUint32 & flags,
                                  PRBool isUnit,
                                  nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMNode> theChild;
  nsCOMPtr<nsIDOMElement> mathname;
  PRUint32 caretPos(msiIMathMLEditingBC::INVALID);
  nsAutoString msimathname, msitrue, msiunit;
  if (isUnit) 
    msiEditingAtoms::msiunit->ToString(msiunit);
  else
    msiEditingAtoms::msimathname->ToString(msimathname);
  msiEditingAtoms::msitrue->ToString(msitrue);
  if (name.Length() > 0)
    caretPos = name.Length();
  // Some mathnames are <mo>'s. Which ones?
  if (name.EqualsLiteral("mod")) {
    res = CreateMathMLLeafElement(editor, name, msiIMathMLEditingBC::MATHML_MO, caretPos, flags, mathname);
  } else {
    res = CreateMathMLLeafElement(editor, name, msiIMathMLEditingBC::MATHML_MI, caretPos, flags, mathname);
  }
  if (NS_SUCCEEDED(res) && mathname) 
  {
    mathname->SetAttribute(isUnit?msiunit:msimathname, msitrue);
    mathmlElement = mathname;
  }
  return res;
}                               

nsresult msiUtils::CreateEngineFunction(nsIEditor * editor,
                                        const nsAString & name,
                                        PRUint32 & flags,
                                        nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMNode> theChild;
  nsCOMPtr<nsIDOMElement> mathname;
  res = msiUtils::CreateMathname(editor, name, flags, PR_FALSE, mathname);
  nsAutoString msiclass, enginefunction;
  msiEditingAtoms::msiclass->ToString(msiclass);
  msiEditingAtoms::enginefunction->ToString(enginefunction);
  if (NS_SUCCEEDED(res) && mathname) 
  {
    mathname->SetAttribute(msiclass, enginefunction);
    mathmlElement = mathname;
  }
  return res;
}                               
            
nsresult msiUtils::CreateMRowFence(nsIEditor * editor,
                                   nsIDOMNode * child,
                                   PRBool createInputBox,
                                   const nsAString & open,
                                   const nsAString & close,
                                   PRBool markCaret,
                                   PRUint32 & flags,
                                   PRUint32 & attrFlags,
                                   nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  PRUint32 dummyFlags(msiIMathMLInsertion::FLAGS_NONE);
  nsCOMPtr<nsIDOMNode> theChild;
  nsCOMPtr<nsIDOMElement> fence;
  nsCOMPtr<nsIDOMElement> opening;
  nsCOMPtr<nsIDOMElement> closing;
  nsAutoString emptyString;
  PRUint32 commonflags(msiIMMLEditDefines::MO_ATTR_fence_T | msiIMMLEditDefines::MO_ATTR_stretchy_T | msiIMMLEditDefines::MO_ATTR_symmetric_T);
  commonflags |= (attrFlags & msiIMMLEditDefines::MO_ATTR_boundFence);
  res = CreateMathOperator(editor, open,  msiIMathMLEditingBC::INVALID, dummyFlags,
                           msiIMMLEditDefines::MO_ATTR_prefix|commonflags, emptyString, emptyString, 
                           emptyString, emptyString, opening); 
  if (NS_SUCCEEDED(res)) 
    res = CreateMathOperator(editor, close, msiIMathMLEditingBC::INVALID, dummyFlags,
                             msiIMMLEditDefines::MO_ATTR_postfix|commonflags, emptyString, emptyString, 
                             emptyString, emptyString, closing); 
  if (NS_SUCCEEDED(res))
  {
    nsCOMPtr<nsIDOMNode> openNode(do_QueryInterface(opening));
    nsCOMPtr<nsIDOMNode> closeNode(do_QueryInterface(closing));
    if (openNode && closeNode)
    {
      nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
      if (NS_SUCCEEDED(res) && mutableArray)
      {
        res = mutableArray->AppendElement(openNode, PR_FALSE);
        if (NS_SUCCEEDED(res))
        {
          if (child)
            res = mutableArray->AppendElement(child, PR_FALSE);
          else if (createInputBox)
          {
            nsCOMPtr<nsIDOMElement> inputbox;
            res = CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
            NS_ASSERTION(inputbox, "CreateInputbox failed.");
            if (NS_SUCCEEDED(res) && inputbox) 
              res = mutableArray->AppendElement(inputbox, PR_FALSE);
          }
        }
        if (NS_SUCCEEDED(res))
          res = mutableArray->AppendElement(closeNode, PR_FALSE);
        if (NS_SUCCEEDED(res))
        {
          nsCOMPtr<nsIArray> nodeArray(do_QueryInterface(mutableArray));
          res = CreateMRow(editor, nodeArray, fence);
        }
      }    
    }
  }
  if (NS_SUCCEEDED(res)) 
    mathmlElement = fence;
  return res;
} 
                              
nsresult msiUtils::CreateMRow(nsIEditor *editor,
                              nsIDOMNode * child,
                              nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  res = CreateMathMLElement(editor, msiEditingAtoms::mrow, mathmlElement);
  if (NS_SUCCEEDED(res) && mathmlElement && child) 
  {
    nsCOMPtr<nsIDOMNode> dontcare;
    res = mathmlElement->AppendChild(child, getter_AddRefs(dontcare));
  }
  return res;
}        

nsresult msiUtils::CreateMRow(nsIEditor * editor,
                              nsIArray * nodeArray,
                              nsCOMPtr<nsIDOMElement> & mathmlElement)
{  
  nsresult res(NS_ERROR_FAILURE); 
  res = CreateMathMLElement(editor, msiEditingAtoms::mrow, mathmlElement);
  if (NS_SUCCEEDED(res) && mathmlElement) 
  {
    nsCOMPtr<nsISimpleEnumerator> enumerator;
    if (nodeArray)
      nodeArray->Enumerate(getter_AddRefs(enumerator));
    if (enumerator)
    {
      PRBool someMore(PR_FALSE);
      while (NS_SUCCEEDED(res) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
      {
        nsCOMPtr<nsISupports> isupp;
        if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
        {
          res = NS_ERROR_FAILURE; 
          break;
        }
        nsCOMPtr<nsIDOMNode> child(do_QueryInterface(isupp));
        if (child) 
        {
          nsCOMPtr<nsIDOMNode> dontcare;
          res = mathmlElement->AppendChild(child, getter_AddRefs(dontcare));
        }
      }  
    }
  }
  return res;
}        

nsresult msiUtils::CreateMfrac(nsIEditor * editor,
                               nsIDOMNode * num,
                               nsIDOMNode * denom,
                               PRBool createInputBox,
                               PRBool markCaret,
                               PRUint32 & flags,
                               const nsAString & lineThickness,
                               PRUint32 & attrFlags,
                               nsCOMPtr<nsIDOMElement> & mathmlElement)
{  
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMNode> numerator, denominator;
  nsCOMPtr<nsIDOMElement> frac;
  nsCOMPtr<nsIDOMNode> installedNum;
  nsCOMPtr<nsIDOMNode> installedDen;
  nsCOMPtr<nsIDOMElement> numInputbox;
  nsCOMPtr<nsIDOMElement> denInputbox;
  nsAutoString linethickness, displaystyle, msitrue, msifalse;
  msiEditingAtoms::linethickness->ToString(linethickness);
  msiEditingAtoms::displaystyle->ToString(displaystyle);
  msiEditingAtoms::msitrue->ToString(msitrue);
  msiEditingAtoms::msifalse->ToString(msifalse);
  
  res = CreateMathMLElement(editor, msiEditingAtoms::mfrac, frac);
  
  if (NS_SUCCEEDED(res) && frac) 
  {
    if (NS_SUCCEEDED(res) && !lineThickness.IsEmpty())
      res = frac->SetAttribute(linethickness, lineThickness);

    if (num)
      numerator = num;
    else if (createInputBox)
    {
      CreateInputbox(editor, PR_FALSE, markCaret, flags, numInputbox);
      if (numInputbox) 
        numerator = do_QueryInterface(numInputbox);
    }
    else
    {
      nsCOMPtr<nsIArray> nodeArray;  //deliberately empty
      nsCOMPtr<nsIDOMElement> numElement = do_QueryInterface(numerator);
      CreateMRow(editor, (nsIArray *) nodeArray, numElement);
      numerator = numElement;
    }
    if (denom)
      denominator = denom;
    else
    {
      PRBool doMarkCaret(PR_FALSE);
      if (num && markCaret)
        doMarkCaret = PR_TRUE;
      res = CreateInputbox(editor, PR_FALSE, doMarkCaret, flags, denInputbox);
      if (denInputbox) 
        denominator = do_QueryInterface(denInputbox);
    }
                         
    if (numerator && denominator)
    {
      res = frac->AppendChild(numerator, getter_AddRefs(installedNum));
      if (NS_SUCCEEDED(res)) 
        res = frac->AppendChild(denominator, getter_AddRefs(installedDen));
      if (NS_SUCCEEDED(res))
      {
        if (attrFlags & (msiIMMLEditDefines::MO_ATTR_displaySize|msiIMMLEditDefines::MO_ATTR_smallSize))
        {
          nsCOMPtr<nsIDOMElement> styleElem;
          res = WrapNodeInMStyle(editor, frac, styleElem);
          if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_displaySize))
            res = styleElem->SetAttribute(displaystyle, msitrue);
          else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_smallSize))
            res = styleElem->SetAttribute(displaystyle, msifalse);
          mathmlElement = styleElem;
        }
        else
          mathmlElement = frac;
        // Now place the cursor. If new, the numerator has an input box. Put it there.
        nsCOMPtr<nsISelection> sel;
        res = editor->GetSelection(getter_AddRefs(sel));
        nsCOMPtr<nsIDOMNode> cursorNode;
        if (createInputBox)
          cursorNode = installedNum;
        else
          cursorNode = installedDen;  // Otherwuse the denominator has one.
        res = sel->Collapse(cursorNode, 0);
      }
    }
    else 
      res = NS_ERROR_FAILURE;
  }
  return res;
} 

nsresult msiUtils::CreateBinomial(nsIEditor * editor,
                                  nsIDOMNode * num,
                                  nsIDOMNode * denom,
                                  PRBool createInputBox,
                                  PRBool markCaret,
                                  PRUint32 & flags,
                                  const nsAString & opening,
                                  const nsAString & closing,
                                  const nsAString & lineThickness,
                                  PRUint32 & attrFlags,
                                  nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMElement> frac;
//  PRUint32 fracFlags = attrFlags & (msiIMMLEditDefines::MO_ATTR_sizeFlags);
  PRUint32 fracFlags = msiIMMLEditDefines::MO_ATTR_none;
  res = CreateMfrac(editor, num, denom, createInputBox, markCaret, flags, lineThickness, fracFlags, frac);

  if (NS_SUCCEEDED(res) && frac)
  {
    if (!opening.IsEmpty() && !closing.IsEmpty())
    {
      nsCOMPtr<nsIDOMElement> mrowElement;
      nsCOMPtr<nsIDOMNode> child = do_QueryInterface(frac);   //TODO -- do these need to be saved for undo!!!!!!!
      PRUint32 fenceFlags = msiIMMLEditDefines::MO_ATTR_boundFence;
      res = CreateMRowFence(editor, child, PR_FALSE, opening, closing, PR_FALSE, flags, fenceFlags, mrowElement);
      if (NS_SUCCEEDED(res)) 
        mathmlElement = mrowElement;
      else 
        res = NS_ERROR_FAILURE;
    }
    else
      mathmlElement = frac;

    if (NS_SUCCEEDED(res) && mathmlElement)
    {
      if (attrFlags & (msiIMMLEditDefines::MO_ATTR_displaySize|msiIMMLEditDefines::MO_ATTR_smallSize))
      {
        nsAutoString displaystyle, msitrue, msifalse;
        msiEditingAtoms::displaystyle->ToString(displaystyle);
        msiEditingAtoms::msitrue->ToString(msitrue);
        msiEditingAtoms::msifalse->ToString(msifalse);

        nsCOMPtr<nsIDOMElement> styleElem;
        res = WrapNodeInMStyle(editor, mathmlElement, styleElem);
        if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_displaySize))
          res = styleElem->SetAttribute(displaystyle, msitrue);
        else if (NS_SUCCEEDED(res) && (attrFlags & msiIMMLEditDefines::MO_ATTR_smallSize))
          res = styleElem->SetAttribute(displaystyle, msifalse);
        mathmlElement = styleElem;
      }
    }

  }
  else 
    res = NS_ERROR_FAILURE;

  return res;
}

nsresult msiUtils::CreateMtd(nsIEditor * editor,
                             PRBool markCaret,
                             PRUint32 & flags,
                             nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMElement> cell;
  res = CreateMathMLElement(editor, msiEditingAtoms::mtd, cell);
  if (NS_SUCCEEDED(res) && cell) 
  {
    nsCOMPtr<nsIDOMElement> inputbox;
    res = CreateInputbox(editor, PR_FALSE, markCaret, flags, inputbox);
    if (NS_SUCCEEDED(res) && inputbox) 
    {
      nsCOMPtr<nsIDOMNode> dontcare;
      res = cell->AppendChild(inputbox, getter_AddRefs(dontcare));
      if (NS_SUCCEEDED(res))
        mathmlElement = cell;
    }  
  }
  return res;
}                            

nsresult msiUtils::CreateMtr(nsIEditor * editor,
                            PRUint32 numCols,
                            PRBool isLabeledTr, // label not in column count
                            PRBool markCaret,
                            PRUint32 & flags,
                            nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMElement> row;
  mathmlElement = nsnull;
  if (isLabeledTr)
  {
    res = CreateMathMLElement(editor, msiEditingAtoms::mlabeledtr, row);
      numCols += 1;
  }
  else
    res = CreateMathMLElement(editor, msiEditingAtoms::mtr, row);
  if (NS_SUCCEEDED(res) && row)
  {
    for (PRUint32 i=0; i < numCols && NS_SUCCEEDED(res); i++)
    {
      PRBool doMarkCaret(PR_FALSE);
      if (i == 0 && markCaret && !isLabeledTr)
       doMarkCaret = PR_TRUE;
      else if (i == 1 && markCaret && isLabeledTr)
       doMarkCaret = PR_TRUE;
      nsCOMPtr<nsIDOMElement> cell;
      res = CreateMtd(editor, doMarkCaret, flags, cell);
      if (NS_SUCCEEDED(res) && cell)
      {
        nsCOMPtr<nsIDOMNode> dontcare; 
        res = row->AppendChild(cell, getter_AddRefs(dontcare));
      }
      else
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res)) 
        mathmlElement = row;
  } 
  return res;
}                            

nsresult msiUtils::CreateMtable(nsIEditor * editor,
                               PRUint32 numRows,
                               PRUint32 numCols,
                               const nsAString & rowSignature,
                               PRBool markCaret,
                               PRUint32 & flags,
                               nsCOMPtr<nsIDOMElement> & mathmlElement,
                               const nsAString & delim)
{
  nsresult res(NS_ERROR_FAILURE); 
  nsCOMPtr<nsIDOMElement> table;
  nsCOMPtr<nsIDOMElement> row;
  PRUint32 fenceflags = 0;
  nsAutoString right;
  mathmlElement = nsnull;
  res = CreateMathMLElement(editor, msiEditingAtoms::mtable, table);
  if (NS_SUCCEEDED(res) && table)
  {
    PRBool isLabeledTr(PR_FALSE); // use row signature to determine 
    for (PRUint32 i=0; i < numRows && NS_SUCCEEDED(res); i++)
    {
      PRBool doMarkCaret = (markCaret && i == 0);
      nsCOMPtr<nsIDOMElement> row;
      res = CreateMtr(editor, numCols, isLabeledTr, doMarkCaret, flags, row);
      if (NS_SUCCEEDED(res) && row)
      {
        nsCOMPtr<nsIDOMNode> dontcare; 
        res = table->AppendChild(row, getter_AddRefs(dontcare));
      }
      else
        res = NS_ERROR_FAILURE;  
    }
    if (NS_SUCCEEDED(res)) {
      if (delim.Length() > 0) {
        // create an mrow and fill it with mo, mtable, mo where the mo's are fence characters
        if (delim.EqualsLiteral("[")) right = NS_LITERAL_STRING("]");
        else if (delim.EqualsLiteral("(")) right = NS_LITERAL_STRING(")");
        else if (delim.EqualsLiteral("{")) right = NS_LITERAL_STRING("}");
        else // no need for fence 
        {
          mathmlElement = table;
          return res;
        }
        res = CreateMRowFence(editor, (nsIDOMNode*)table, PR_FALSE, delim, right, PR_FALSE, flags, fenceflags, row);
        if (NS_SUCCEEDED(res))
        {
          mathmlElement = row;
          return res;
        }
      }
    }
    if (NS_SUCCEEDED(res)) 
        mathmlElement = table;
  } 
  return res;
}                            

nsresult msiUtils::CloneNode(nsIDOMNode * node,
                             nsCOMPtr <nsIDOMNode> & clone)
{
  nsresult res(NS_ERROR_FAILURE);
  if (node)
  {
    PRBool deep(PR_TRUE); //TODO -- what is the purpose of deep?
    res = node->CloneNode(deep, getter_AddRefs(clone));
  }
  return res;  
}

nsresult msiUtils::CloneChildNode(nsIDOMNode * parent,
                                  PRUint32 indexOfChild,
                                  nsCOMPtr <nsIDOMNode> & clone)
{ 
  nsresult res(NS_ERROR_FAILURE);                            
  nsCOMPtr<nsIDOMNode> kid;
  res = GetChildNode(parent, indexOfChild, kid);
  if (NS_SUCCEEDED(res) && kid)
    res =msiUtils::CloneNode(kid, clone);
  return res;
}    

nsresult msiUtils::WrapNodeInMStyle(nsIEditor * editor, nsIDOMNode * node, nsCOMPtr<nsIDOMElement> & mathmlElement)
{
  nsresult res(NS_ERROR_FAILURE); 
  res = CreateMathMLElement(editor, msiEditingAtoms::mstyle, mathmlElement);
  if (NS_SUCCEEDED(res) && mathmlElement) 
  {
    if (node)
    {
      nsCOMPtr<nsIDOMNode> dontcare;   //TODO -- do these need to be saved for undo!!!!!!!
      res = mathmlElement->AppendChild(node, getter_AddRefs(dontcare));
    }
  }
  return res;
}

nsresult msiUtils::GetIndexOfChildInParent(nsIDOMNode * child, PRUint32 &index)
{
  nsresult res(NS_ERROR_FAILURE);
  nsCOMPtr <nsIDOMNode> parent;
  if (child)
  {
    res = child->GetParentNode(getter_AddRefs(parent));
    if (NS_SUCCEEDED(res) && parent)
    {
      nsCOMPtr<nsIDOMNodeList> children;
      res = parent->GetChildNodes(getter_AddRefs(children));
      if (NS_SUCCEEDED(res) && children) 
      {
        PRUint32 i, count;
        res = children->GetLength(&count);
        if (NS_SUCCEEDED(res))
        {
          for (i=0; i < count; i++) 
          {
            nsCOMPtr<nsIDOMNode> currChild;
            res = children->Item(i, getter_AddRefs(currChild));
            if (NS_SUCCEEDED(res) && currChild == child)
            {
              index = i;
              res = NS_OK;
              break;
            }
          }
        }
      }  
    }
  }
  NS_ASSERTION(res==NS_OK, "Call failing, caller probably not checking.");
  return res;
} 

nsresult msiUtils::GetNonWhitespaceChildren(nsIDOMNode * parent,
                                            nsCOMPtr<nsIArray> & children)
{
  nsresult res(NS_ERROR_FAILURE);
  children = nsnull;
  if (parent)
  {
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res) && mutableArray)
    { 
      nsCOMPtr<nsIDOMNode> currChild;
      res = parent->GetFirstChild(getter_AddRefs(currChild));
      while (NS_SUCCEEDED(res) && currChild)
      {
        nsCOMPtr<nsIDOMNode> nextChild;
        res = currChild->GetNextSibling(getter_AddRefs(nextChild));
        if (NS_SUCCEEDED(res) && !IsWhitespace(currChild))
          res = mutableArray->AppendElement(currChild, PR_FALSE);
        currChild = nextChild;    
      }
      if (NS_SUCCEEDED(res))
        children = do_QueryInterface(mutableArray);
    }
  }
  if (children)
    res = NS_OK;
  else
    res = NS_ERROR_FAILURE;  
  return res;  
}                                            

nsresult msiUtils::GetChildNode(nsIDOMNode * parent,
                                PRUint32 indexOfChild,
                                nsCOMPtr<nsIDOMNode> & child)
{
  nsresult res(NS_ERROR_FAILURE);
  child = nsnull;
  if (parent)
  {
    nsCOMPtr<nsIDOMNodeList> children;
    parent->GetChildNodes(getter_AddRefs(children));
    if (children) 
      children->Item(indexOfChild, getter_AddRefs(child));
    if (child)
      res = NS_OK;
  }
  else
   res = NS_ERROR_NULL_POINTER;
  return res; 
}                  

nsresult msiUtils::RemoveChildNode(nsIDOMNode * parent,
                                   PRUint32 indexOfChild,
                                   nsCOMPtr<nsIDOMNode> & removedChild)
{
  nsresult res(NS_ERROR_FAILURE);
  removedChild = nsnull;
  nsCOMPtr<nsIDOMNode> kid;
  if (parent)
  {
    res  = GetChildNode(parent, indexOfChild, kid);
    if (NS_SUCCEEDED(res) && kid)
      res = parent->RemoveChild(kid, getter_AddRefs(removedChild));
  }
  else
   res = NS_ERROR_NULL_POINTER;
  return res; 
}

nsresult msiUtils::RemoveIndexedChildNode(nsIEditor * editor,
                                   nsIDOMNode * parent,
                                   PRUint32 indexOfChild,
                                   nsCOMPtr<nsIDOMNode> & removedChild)
{
  nsresult res(NS_ERROR_FAILURE);
  removedChild = nsnull;
  nsCOMPtr<nsIDOMNode> kid;
  if (parent)
  {
    res  = GetChildNode(parent, indexOfChild, kid);
    if (NS_SUCCEEDED(res) && kid)
      editor->DeleteNode(kid);
  }
  else
   res = NS_ERROR_NULL_POINTER;
  return res; 
}                 
                 

nsresult msiUtils::ReplaceChildNode(nsIDOMNode * parent,
                                    PRUint32 indexOfChild,
                                    nsIDOMNode * newChild,
                                    nsCOMPtr<nsIDOMNode> & replacedChild)
{
  nsresult res(NS_ERROR_FAILURE);
  replacedChild = nsnull;
  nsCOMPtr<nsIDOMNode> kid;
  if (parent && newChild)
  {
    res  = GetChildNode(parent, indexOfChild, kid);
    if (NS_SUCCEEDED(res) && kid)
      res = parent->ReplaceChild(newChild, kid, getter_AddRefs(replacedChild));
  }
  else
   res = NS_ERROR_NULL_POINTER;
  return res; 
}                                

nsresult msiUtils::InsertChildren(nsIDOMNode * parent, 
                                  PRUint32 offset, 
                                  nsIArray * newChildList)
{
  nsresult res(NS_ERROR_FAILURE);
  if (parent)
  {
    PRUint32 numExistingKids(0);
    res = msiUtils::GetNumberofChildren(parent, numExistingKids);
    PRUint32 inputLength(0);
    if (NS_SUCCEEDED(res) && newChildList)
      newChildList->GetLength(&inputLength);
    PRInt32 index = inputLength -1;
    if (index >= 0)
    {
      nsCOMPtr<nsIDOMNode> before;
      if (offset >= numExistingKids)
      {
        nsCOMPtr<nsIDOMNode> currNode;
        res = newChildList->QueryElementAt(index, NS_GET_IID(nsIDOMNode), getter_AddRefs(currNode));
        if (NS_SUCCEEDED(res) && currNode)
          res = parent->AppendChild(currNode, getter_AddRefs(before));
        index -= 1;  
      }
      else
        msiUtils::GetChildNode(parent, offset, before);
      while (index >= 0 && NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsIDOMNode> currNode, newBefore;
        res = newChildList->QueryElementAt(index, NS_GET_IID(nsIDOMNode), getter_AddRefs(currNode));
        index -= 1;  
        if (NS_SUCCEEDED(res) && currNode)
          res = parent->InsertBefore(currNode, before, getter_AddRefs(newBefore));
        if (NS_SUCCEEDED(res) && newBefore)
          before = newBefore;
      }
    }
  }
  return res;      
}

PRUint32 msiUtils::GetMathmlNodeType(nsISupports* isupports)
{
  PRUint32 rv(msiIMathMLEditingBC::MATHML_UNKNOWN);
  if (isupports)
  {
    nsCOMPtr<msiIMathMLEditingBC> msiEditing(do_QueryInterface(isupports));
    if (msiEditing)
      msiEditing->GetMathmlType(&rv);
  }    
  return rv;
}
     
nsresult msiUtils::GetMathmlNodeType(nsIEditor * editor,
                                     nsIDOMNode * node,
                                     PRUint32 & nodetype)
{
  nsresult res(NS_ERROR_FAILURE);
  nodetype = msiIMathMLEditingBC::MATHML_UNKNOWN;
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditingBC> editingBC; 
    PRUint32 dontcare(0);
    GetMathMLEditingBC(editor, node, dontcare, false, editingBC);
    if (editingBC)
      res = editingBC->GetMathmlType(&nodetype);
  }    
  return res;
}  
     
PRBool msiUtils::IsWhitespace(nsIDOMNode * node)
{
  if (node)
  {
    nsCOMPtr<nsIContent> tc(do_QueryInterface(node));
    if (tc && tc->TextIsOnlyWhitespace())
      return PR_TRUE;
  }
  return PR_FALSE;    
}

PRBool msiUtils::IsInputbox(nsIEditor * editor,
                            nsIDOMNode * node)
{
  PRBool rv(PR_FALSE);
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditingBC> editingBC; 
    PRUint32 dontcare(0);
    GetMathMLEditingBC(editor, node, dontcare, false, editingBC);
    if (editingBC)
      rv = IsInputbox(editingBC);
  }    
  return rv;
}  
  
PRBool msiUtils::IsInputbox(nsISupports * isupports)
{
  PRUint32 mathmltype = GetMathmlNodeType(isupports);
  return (mathmltype == msiIMathMLEditingBC::MSI_INPUTBOX);
}

PRBool msiUtils::IsMleaf(nsIEditor * editor,
                         nsIDOMNode * node,
                         PRBool allowInputbox)
{
  PRBool rv(PR_FALSE);
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditingBC> editingBC; 
    PRUint32 dontcare(0);
    GetMathMLEditingBC(editor, node, dontcare, false, editingBC);
    if (editingBC)
      rv = IsMleaf(editingBC, allowInputbox);
  }    
  return rv;
}  
  
PRBool msiUtils::IsMleaf(nsISupports * isupports, 
                         PRBool allowInputbox)
{
  PRBool rv(PR_FALSE);
  PRUint32 mathmltype = GetMathmlNodeType(isupports);
  if (allowInputbox)
    rv = (mathmltype == msiIMathMLEditingBC::MSI_INPUTBOX ||
          mathmltype == msiIMathMLEditingBC::MATHML_MI    ||
          mathmltype == msiIMathMLEditingBC::MATHML_MN    ||
          mathmltype == msiIMathMLEditingBC::MATHML_MO     );
  else        
    rv = (mathmltype == msiIMathMLEditingBC::MATHML_MI    ||
          mathmltype == msiIMathMLEditingBC::MATHML_MN    ||
          mathmltype == msiIMathMLEditingBC::MATHML_MO     );
  return rv;
}

PRBool msiUtils::IsMrow(nsIEditor * editor,
                        nsIDOMNode * node)
{
  PRBool rv(PR_FALSE);
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditingBC> editingBC; 
    PRUint32 dontcare(0);
    GetMathMLEditingBC(editor, node, dontcare, false, editingBC);
    if (editingBC)
      rv = IsMrow(editingBC);
  }    
  return rv;
}  
  
PRBool msiUtils::IsMrow(nsISupports * isupports)
{
  PRUint32 mathmltype = GetMathmlNodeType(isupports);
  return (mathmltype == msiIMathMLEditingBC::MATHML_MROW);
}



PRBool msiUtils::IsEmpty(nsIDOMNode* pNode)
{
  PRUint32 number;
  GetNumberofChildren(pNode, number);
  return (number == 0);
}


// A more general version of previous functions
PRBool msiUtils::hasMMLType(nsIEditor * editor,	nsIDOMNode * node, 	unsigned short mmlType)
{
  PRBool rv(PR_FALSE);
  if (editor && node)
  {
    nsCOMPtr<msiIMathMLEditingBC> editingBC; 
    PRUint32 dontcare(0);
    GetMathMLEditingBC(editor, node, dontcare, false, editingBC);
    if (editingBC) {
	  PRUint32 mathmltype = GetMathmlNodeType(editingBC);

      rv = (mathmltype == mmlType);
	}
  }    
  return rv;
}

nsresult msiUtils::GetTableCell(nsIEditor * editor,
                                nsIDOMNode * node,
                                nsCOMPtr<nsIDOMNode> & mtdCell)
{
  PRUint32 mathmltype;
  nsresult res = GetMathmlNodeType(editor, node, mathmltype);
  if (NS_SUCCEEDED(res) && mathmltype == msiIMathMLEditingBC::MATHML_MTD) {
    mtdCell = node;
    return NS_OK;
  }
  nsCOMPtr<nsIDOMNode> thisMtd;
  res = msiUtils::GetMathTagParent(node, msiEditingAtoms::mtd, thisMtd);
  if (NS_SUCCEEDED(res) && thisMtd)
  {
    mtdCell = thisMtd;
    return NS_OK;
  }
  else
    return NS_ERROR_FAILURE;
}

PRBool msiUtils::NodeHasCaretMark(nsIDOMNode * node, PRUint32& pos, PRBool& caretOnText)
{
  PRBool rv(PR_FALSE);
  nsCOMPtr<nsIDOMElement> element;
  if (node)
    element = do_QueryInterface(node);
  if (element)
  {
    nsAutoString value1, value2, msicaretpos, msicaretpostext;
    msiEditingAtoms::msicaretpos->ToString(msicaretpos);
    msiEditingAtoms::msicaretpostext->ToString(msicaretpostext);
    element->GetAttribute(msicaretpos, value1);
    element->GetAttribute(msicaretpostext, value2);
    if (!value1.IsEmpty())
    {
      PRInt32 errorCode;
      pos = value1.ToInteger(&errorCode);
      caretOnText = PR_FALSE;
      rv = PR_TRUE;
    }
    else if (!value2.IsEmpty()) // set caret in text node
    {
      PRInt32 errorCode;
      pos = value2.ToInteger(&errorCode);
      caretOnText = PR_TRUE;
      rv = PR_TRUE;
    }
  }
  return rv;
}

nsresult msiUtils::MarkCaretPosition(nsIEditor * editor,
                                     nsIDOMNode * node,
                                     PRUint32 pos,
                                     PRUint32 & flags,
                                     PRBool caretOnText,
                                     PRBool overwrite)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && node && (pos <= msiIMathMLEditingBC::LAST_VALID || 
                         pos == msiIMathMLEditingBC::TO_LEFT    ||
                         pos == msiIMathMLEditingBC::TO_RIGHT)) 
  {
    res = NS_OK;
    nsCOMPtr<nsISelection> sel;
    res = editor->GetSelection(getter_AddRefs(sel));
    sel->Collapse(node, pos);
    PRBool caretMarked = (flags & msiIMathMLInsertion::FLAGS_CARET_MARKED) ? PR_TRUE : PR_FALSE;
    if (caretMarked && overwrite)
    {
      nsCOMPtr<nsIDOMNode> mathParent;
      GetMathParent(node, mathParent);
      ClearCaretPositionMark(editor, mathParent, PR_FALSE);
      caretMarked = PR_FALSE;
    } 
    if (!caretMarked)
    {
      res = NS_ERROR_FAILURE;
      nsCOMPtr<nsIDOMElement> element(do_QueryInterface(node));
      nsAutoString value, attribute;
      value.AppendInt(pos);
      if (caretOnText)
        msiEditingAtoms::msicaretpostext->ToString(attribute);
      else  
       msiEditingAtoms::msicaretpos->ToString(attribute);
      if (element)
      {
        res = element->SetAttribute(attribute, value);
        flags |= msiIMathMLInsertion::FLAGS_CARET_MARKED;
      }
    }  
  }
  return res;
}                                                 

//ljh The location of the caret is given via the temp attribute 
// _msi_caret_pos set on a node in the subtree of rootnode.
// the value of the attribute gives to offset.
  
nsresult msiUtils::doSetCaretPosition(nsIEditor * editor,
                                      nsISelection * selection,
                                      nsIDOMNode * rootnode)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && selection && rootnode)
  {
    nsCOMPtr<nsIDOMDocument> domDoc;
    editor->GetDocument(getter_AddRefs(domDoc));
    nsCOMPtr<nsIDOMNode> theNode;
    PRUint32 thePos(msiIMathMLEditingBC::INVALID);
    nsCOMPtr<nsIDOMDocumentTraversal> trav = do_QueryInterface(domDoc);
    nsAutoString msicaretpos, msicaretpostext;
    msiEditingAtoms::msicaretpos->ToString(msicaretpos);
    msiEditingAtoms::msicaretpostext->ToString(msicaretpostext);
    if (trav)
    {
      nsCOMPtr<nsIDOMTreeWalker> walker;
      res = trav->CreateTreeWalker(rootnode, nsIDOMNodeFilter::SHOW_ELEMENT,
                                   nsnull, PR_TRUE, getter_AddRefs(walker));
      if (NS_SUCCEEDED(res) && walker)
      {
        nsCOMPtr<nsIDOMNode> currentNode;
        walker->GetCurrentNode(getter_AddRefs(currentNode));
        
        PRBool found(PR_FALSE);
        while (currentNode && !found)
        {
          nsCOMPtr<nsIDOMElement> currElement(do_QueryInterface(currentNode));
          if (currElement)
          {
             nsAutoString value1, value2;
             currElement->GetAttribute(msicaretpos, value1);
             currElement->GetAttribute(msicaretpostext, value2);
             if (!value1.IsEmpty())
             {
               PRInt32 errorCode;
               thePos = value1.ToInteger(&errorCode);
               theNode = currentNode;
               currElement->RemoveAttribute(msicaretpos);
               found = PR_TRUE;
             }
             else if (!value2.IsEmpty()) // set caret in text node
             {
               PRInt32 errorCode;
               thePos = value2.ToInteger(&errorCode);
               if (thePos == msiIMathMLEditingBC::TO_LEFT || 
                   thePos == msiIMathMLEditingBC::TO_RIGHT) // bit of a shotgun approach
                 theNode == currentNode;
               else  
                 currentNode->GetFirstChild(getter_AddRefs(theNode));
               currElement->RemoveAttribute(msicaretpostext);
               found = PR_TRUE;
             }
          }
          if (!found)
            walker->NextNode(getter_AddRefs(currentNode));
        }
      }  
    } 
    if (theNode)
    {
      if (thePos == msiIMathMLEditingBC::TO_LEFT || thePos == msiIMathMLEditingBC::TO_RIGHT)
      {
          nsCOMPtr<nsIDOMNode> parent;
          PRUint32 offset(0);
          theNode->GetParentNode(getter_AddRefs(parent));
          GetIndexOfChildInParent(theNode, offset);
          if (thePos == msiIMathMLEditingBC::TO_RIGHT)
            offset += 1;
          if (parent)  
            res = selection->Collapse(parent, offset);
          else  
            res = NS_ERROR_FAILURE;
      }
      else
        res = selection->Collapse(theNode, thePos);
    }
    else
    {
      nsCOMPtr<nsIDOMNode> mathParent;
      GetMathParent(rootnode, mathParent);
      if (mathParent != rootnode)
        doSetCaretPosition(editor, selection, mathParent); // recursive call -- but only once
      else  
        res = NS_ERROR_FAILURE;
    }    
  }
  return res;
} 

nsresult msiUtils::doSetCaretPosition(nsISelection * selection,
                                      nsIDOMNode * node,
                                      PRUint32 offset)
{
  nsresult res(NS_ERROR_FAILURE);
  if (selection && node)
    res = selection->Collapse(node, offset);
  return res;
}                            

nsresult msiUtils::ClearCaretPositionMark(nsIEditor * editor, 
                                          nsIDOMNode * node,
                                          PRBool clearAll)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor && node)
  {
    nsCOMPtr<nsIDOMDocument> domDoc;
    editor->GetDocument(getter_AddRefs(domDoc));
    nsAutoString msicaretpos, msicaretpostext;
    msiEditingAtoms::msicaretpos->ToString(msicaretpos);
    msiEditingAtoms::msicaretpostext->ToString(msicaretpostext);
    nsCOMPtr<nsIDOMDocumentTraversal> trav = do_QueryInterface(domDoc);
    if (trav)
    {
      nsCOMPtr<nsIDOMTreeWalker> walker;
      res = trav->CreateTreeWalker(node, nsIDOMNodeFilter::SHOW_ELEMENT,
                                   nsnull, PR_TRUE, getter_AddRefs(walker));
      if (NS_SUCCEEDED(res) && walker)
      {
        nsCOMPtr<nsIDOMNode> currentNode;
        walker->GetCurrentNode(getter_AddRefs(currentNode));
        PRBool done(PR_FALSE);
        while (currentNode && !done)
        {
          nsCOMPtr<nsIDOMElement> currElement(do_QueryInterface(currentNode));
          if (currElement)
          {
             nsAutoString value1, value2;
             currElement->GetAttribute(msicaretpos, value1);
             currElement->GetAttribute(msicaretpostext, value2);
             if (!value1.IsEmpty())
             {
               currElement->RemoveAttribute(msicaretpos);
               if (!clearAll)
                 done = PR_TRUE;
             }
             else if (!value2.IsEmpty())
             {
               currElement->RemoveAttribute(msicaretpostext);
               if (!clearAll)
                 done = PR_TRUE;
             }
          }
          walker->NextNode(getter_AddRefs(currentNode));
        }
      }
    }  
  }
  return res;
}                                          

nsresult msiUtils::GetMathTagParent(nsIDOMNode * node,
                                    nsIAtom * tagAtom,
                                    nsCOMPtr<nsIDOMNode> & mathParent)
{
  nsresult res(NS_OK);
  mathParent = nsnull;
  nsCOMPtr<nsIDOMNode> p = node;
  if (!p)
   res = NS_ERROR_NULL_POINTER;
  while (p)
  {
    nsAutoString localName, nsURI;
    p->GetLocalName(localName);
    p->GetNamespaceURI(nsURI);
    PRInt32 nsID(kNameSpaceID_Unknown);
    msiNameSpaceUtils::GetNameSpaceID(nsURI, nsID);
    
    if (tagAtom->Equals(localName) && nsID == kNameSpaceID_MathML)
    {
      mathParent = p;
      p = nsnull;
      res = NS_OK;
    }
    else
    {
      nsCOMPtr<nsIDOMNode> temp = p;
      temp->GetParentNode(getter_AddRefs(p));
    }
  }
  return res;
}
nsresult msiUtils::GetMathParent(nsIDOMNode * node,
                                 nsCOMPtr<nsIDOMNode> & mathParent)
{
  return GetMathTagParent(node,msiEditingAtoms::math,mathParent);
}   

nsresult msiUtils::GetNumberofChildren(nsIDOMNode * node,
                                      PRUint32  & number)
{  
  //TODO -- need to worry about kruff nodes                                    
  nsresult res(NS_ERROR_FAILURE);
  if (node)
  {
    nsCOMPtr<nsIDOMNodeList> childNodes;
    res = node->GetChildNodes(getter_AddRefs(childNodes));
    if (NS_SUCCEEDED(res) && childNodes)
      res = childNodes->GetLength(&number);
  }
  return res;
}  


nsresult
MergeMath(nsIDOMNode * left, nsIDOMNode * right, nsIEditor * editor) {
  nsresult res = NS_OK;
  nsCOMPtr<nsIDOMNodeList> childNodes;
  nsAutoString tagName;
  PRBool isTemp;
  res = left->GetChildNodes(getter_AddRefs(childNodes));
  PRUint32 offset;
  PRInt32 selStartOffset;
  nsCOMPtr<nsIDOMNode> selStartNode;
  nsCOMPtr<nsIDOMNode> ignored;
  nsCOMPtr<nsIDOMElement> el;
  PRInt32 selEndOffset;
  nsCOMPtr<nsIDOMNode> selEndNode;
  childNodes->GetLength(&offset);
  nsEditor * realEditor = static_cast<nsEditor*>(editor);
  if (!realEditor) return NS_ERROR_FAILURE;
  nsCOMPtr<nsISelection> sel;
  res = realEditor->GetSelection(getter_AddRefs(sel));

  nsCOMPtr<nsIDOMNode> child;
  right->GetFirstChild(getter_AddRefs(child));
  while (child)
  {
    child->GetLocalName(tagName);
    if (tagName.EqualsLiteral("mi")) {
      el = do_QueryInterface(child);
      res = el->HasAttribute(NS_LITERAL_STRING("tempinput"), &isTemp);
      if (!isTemp) {
        res = realEditor->MoveNode(child, left, offset);
        offset++;
      } else
      {
        // throw away temp input box
        res = msiUtils::RemoveChildNode(right, 0, ignored);
        sel->Collapse(left, offset);
      }
    }
    else {
      res = realEditor->MoveNode(child, left, offset);
      offset++;
    }

    if (NS_FAILED(res)) return res;
    right->GetFirstChild(getter_AddRefs(child));
  }
  // The selection needs updating only if one of the nodes is *right 
  res = realEditor->GetStartNodeAndOffset(sel, getter_AddRefs(selStartNode), &selStartOffset);
  if (selStartNode == right) {
    sel->Collapse(left, selStartOffset + offset);
  }
  res = realEditor->GetEndNodeAndOffset(sel, getter_AddRefs(selEndNode), &selEndOffset);
  if (selEndNode == right) {
    sel->Extend(left, selEndOffset + offset);
  }
  realEditor->DeleteNode(right);
  return NS_OK;
}

nsresult
msiUtils::MergeMathTags(nsIDOMNode * node, PRBool pLookLeft, PRBool pLookRight, nsIEditor * editor)  // If the node is within mathematics, go up to the math
// node and check to see if it is adjacent to (except for white-space text nodes) another math node. If so,
// merge the nodes. Also if the selection is in text but between two math nodes which are adjacent except 
// for white-space text nodes, merge those math nodes also.
{
  nsCOMPtr<nsIDOMNode> mathParent;
  nsCOMPtr<nsIDOMNode> siblingNode;
  PRUint16 nodetype;
  nsAutoString nodeName;

  nsresult res;
  res = GetMathParent(node, mathParent);
  if (mathParent && pLookLeft) {
    // check to the left
    res = mathParent->GetPreviousSibling(getter_AddRefs(siblingNode));
    while (siblingNode != nsnull) {
      res = siblingNode->GetNodeType(&nodetype);
      if (nodetype == nsIDOMNode::TEXT_NODE)
      {
        if (!IsWhitespace(siblingNode)) {
          return NS_OK;
        }
      }
      else if (nodetype = nsIDOMNode::ELEMENT_NODE) {
        res = siblingNode->GetLocalName(nodeName);
        if (nodeName.EqualsLiteral("math")) {
          res = MergeMath(siblingNode, mathParent, editor);
          return NS_OK;
        }
        return NS_OK;
      }
      // The only possibility for looping again is that siblingNode is a white-space text node
      res = siblingNode->GetPreviousSibling(getter_AddRefs(siblingNode));
    }
  }
  if (mathParent && pLookRight) {
    res = mathParent->GetNextSibling(getter_AddRefs(siblingNode));
    while (siblingNode != nsnull) {
      res = siblingNode->GetNodeType(&nodetype);
      if (nodetype == nsIDOMNode::TEXT_NODE)
      {
        if (!IsWhitespace(siblingNode)) return NS_OK;
      }
      else if (nodetype = nsIDOMNode::ELEMENT_NODE) {
        res = siblingNode->GetLocalName(nodeName);
        if (nodeName.EqualsLiteral("math")) {
          MergeMath(mathParent, siblingNode, editor);
          return NS_OK;
        }
        return NS_OK;
      }
      res = siblingNode->GetNextSibling(getter_AddRefs(siblingNode));
    }
  }
}


nsresult msiUtils::GetRightMostCaretPosition(nsIEditor* editor,
                                             nsIDOMNode * node,
                                             PRUint32  & number)
{  
  //TODO -- need to worry about kruff nodes                                    
  nsresult res(NS_ERROR_FAILURE);
  if (node)
  {
    nsCOMPtr<nsIDOMText> text(do_QueryInterface(node));
    if (text)
    {
       nsCOMPtr<nsIDOMCharacterData> characterdata(do_QueryInterface(node));
       NS_ASSERTION(characterdata, "Yuck - IDOMCharacterData QueryInterface failed on an IDOMText node");
       if (characterdata)
          res = characterdata->GetLength(&number);
    }
    else if (editor && IsMleaf(editor, node, PR_TRUE))
    {
      if (IsInputbox(editor, node))
        number = 1;
      else
      {  
        nsCOMPtr<nsIDOMNode> child;
        node->GetFirstChild(getter_AddRefs(child));
        if (child)
        {
          nsCOMPtr<nsIDOMText> text(do_QueryInterface(child));
          if (text)
          {
             nsCOMPtr<nsIDOMCharacterData> characterdata(do_QueryInterface(text));
             NS_ASSERTION(characterdata, "Yuck - IDOMCharacterData QueryInterface failed on an IDOMText node");
             if (characterdata)
                res = characterdata->GetLength(&number);
          }
        }
      }  
    }
    else
    {
      nsCOMPtr<nsIDOMNodeList> childNodes;
      res = node->GetChildNodes(getter_AddRefs(childNodes));
      if (NS_SUCCEEDED(res) && childNodes)
        res = childNodes->GetLength(&number);
    }    
  }
  return res;
}  

nsresult msiUtils::AddToNodeList(nsIArray* nodeList, 
                                 nsIArray * addToFront, 
                                 nsIArray * addToEnd, 
                                 nsCOMPtr<nsIArray> & pArray)
{
  nsresult res(NS_ERROR_FAILURE);
  pArray = nsnull;
  if (addToFront || addToEnd)
  {
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res) && mutableArray)
    {
      if (addToFront)
      {
        nsCOMPtr<nsISimpleEnumerator> enumerator;
        addToFront->Enumerate(getter_AddRefs(enumerator));
        if (enumerator)
        {
          PRBool someMore(PR_FALSE);
          while (NS_SUCCEEDED(res) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
          {
            nsCOMPtr<nsISupports> isupp;
            if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
            {
              res = NS_ERROR_FAILURE; 
              break;
            }
            nsCOMPtr<nsIDOMNode> node(do_QueryInterface(isupp));
            if (node) 
              res = mutableArray->AppendElement(node, PR_FALSE);
          }
        }    
      }
      if (NS_SUCCEEDED(res))
      {
        nsCOMPtr<nsISimpleEnumerator> enumerator;
        if (nodeList)
          nodeList->Enumerate(getter_AddRefs(enumerator));
        if (enumerator)
        {
          PRBool someMore(PR_FALSE);
          while (NS_SUCCEEDED(res) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
          {
            nsCOMPtr<nsISupports> isupp;
            if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
            {
              res = NS_ERROR_FAILURE; 
              break;
            }
            nsCOMPtr<nsIDOMNode> node(do_QueryInterface(isupp));
            if (node) 
              res = mutableArray->AppendElement(node, PR_FALSE);
          }
        }    
      }
      if (NS_SUCCEEDED(res) && addToEnd)
      {
        nsCOMPtr<nsISimpleEnumerator> enumerator;
        addToEnd->Enumerate(getter_AddRefs(enumerator));
        if (enumerator)
        {
          PRBool someMore(PR_FALSE);
          while (NS_SUCCEEDED(res) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
          {
            nsCOMPtr<nsISupports> isupp;
            if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
            {
              res = NS_ERROR_FAILURE; 
              break;
            }
            nsCOMPtr<nsIDOMNode> node(do_QueryInterface(isupp));
            if (node) 
              res = mutableArray->AppendElement(node, PR_FALSE);
          }
        }    
      }
      if (NS_SUCCEEDED(res))
        pArray = do_QueryInterface(mutableArray);
    }
  }  
  else if (nodeList)
    pArray = do_QueryInterface(nodeList);
      
  if (pArray)
    res = NS_OK;
  else
    res = NS_ERROR_FAILURE;
  return res;  
}   

nsresult msiUtils::AppendToMutableList(nsCOMPtr<nsIMutableArray> & mutableList, 
                                       nsCOMPtr<nsIArray> & tobeAdded)
{
  nsresult res(NS_ERROR_FAILURE);
  if (mutableList)
  {
    res = NS_OK;
    if (tobeAdded)
    {
      nsCOMPtr<nsISimpleEnumerator> enumerator;
      tobeAdded->Enumerate(getter_AddRefs(enumerator));
      if (enumerator)
      {
        res = NS_OK;
        PRBool someMore(PR_FALSE);
        while (NS_SUCCEEDED(res) && NS_SUCCEEDED(enumerator->HasMoreElements(&someMore)) && someMore) 
        {
          nsCOMPtr<nsISupports> isupp;
          if (NS_FAILED(enumerator->GetNext(getter_AddRefs(isupp))))
          {
            res = NS_ERROR_FAILURE; 
            break;
          }
          res = mutableList->AppendElement(isupp, PR_FALSE);
        }
      }
    }
  }  
  return res;      
}                                       

nsresult msiUtils::RemoveNodesFromList(nsIArray* nodeList, 
                                       PRUint32 index,
                                       PRUint32 count,
                                       nsCOMPtr<nsIArray> & pArray)
{
  nsresult res(NS_ERROR_FAILURE);
  pArray = nsnull;
  PRUint32 numNodes(0);
  if (nodeList)
    nodeList->GetLength(&numNodes);
  if (index+1 <= numNodes && count)
  {
    nsCOMPtr<nsIMutableArray> mutableArray = do_CreateInstance(NS_ARRAY_CONTRACTID, &res);
    if (NS_SUCCEEDED(res) && mutableArray)
    {
      for (PRUint32 i=0; i<numNodes && NS_SUCCEEDED(res); i++)
      {
        nsCOMPtr<nsIDOMNode> node;
        res = nodeList->QueryElementAt(i, NS_GET_IID(nsIDOMNode), getter_AddRefs(node));
        if (NS_SUCCEEDED(res) && node && (i < index || i >= index+count)) 
           res = mutableArray->AppendElement(node, PR_FALSE);
      }
    }
    if (NS_SUCCEEDED(res))
      pArray = do_QueryInterface(mutableArray);   
  }
  else if (nodeList)
  {
    pArray = do_QueryInterface(nodeList);   
    res = NS_OK;
  }
  return res;  
}                                                 

nsresult msiUtils::GetMathmlNodeFromCaretInterface(nsCOMPtr<msiIMathMLCaret> & caret,
                                                   nsCOMPtr<nsIDOMNode> & mathmlNode)
{
  nsresult res(NS_ERROR_FAILURE);
  mathmlNode = nsnull;
  nsCOMPtr<msiIMathMLEditingBC> bc(do_QueryInterface(caret));
  if (bc)
    bc->GetMathmlNode(getter_AddRefs(mathmlNode));
  if (mathmlNode)
    res = NS_OK;
  return res;    
}
                                         
nsresult msiUtils::GetOffsetFromCaretInterface(nsCOMPtr<msiIMathMLCaret> & caret,
                                              PRUint32 & offset)
{
  nsresult res(NS_ERROR_FAILURE);
  offset = msiIMathMLEditingBC::INVALID;
  nsCOMPtr<msiIMathMLEditingBC> bc(do_QueryInterface(caret));
  if (bc)
    bc->GetOffset(&offset);
  if (offset <= msiIMathMLEditingBC::LAST_VALID)
    res = NS_OK;
  return res;    
}
                                      
nsresult msiUtils::ComparePoints(nsIEditor * editor,
                                 nsIDOMNode * node1, PRUint32 offset1,
                                 nsIDOMNode * node2, PRUint32 offset2,
                                 PRInt32 &comparison)
{
  nsresult res(NS_ERROR_FAILURE);
  if (editor)
  {
    nsCOMPtr<msiIMathMLEditor> msiEditor(do_QueryInterface(editor));
    if (msiEditor)
      res = msiEditor->ComparePoints(node1, offset1, node2, offset2, &comparison);
  }
  return res; 
}

PRUint32 msiUtils::GetMrowPurgeMode()
{
  //TODO - how should this be determined -- user preference??
  return MROW_PURGE_ALL;
}

nsresult msiUtils::SplitNode(nsIDOMNode * node, PRUint32 offset,
                             PRBool emptyOK,
                             nsCOMPtr<nsIDOMNode> & left,
                             nsCOMPtr<nsIDOMNode> & right)
{ 
  if (!node)
    return NS_ERROR_FAILURE;
  PRUint32 count;
  nsresult res = msiUtils::GetNumberofChildren(node, count);
  if (!emptyOK && offset == 0)
  {
    left = nsnull;
    res = msiUtils::CloneNode(node, right);
  }
  else if (!emptyOK && offset >= count)
  {
    right = nsnull;
    res = msiUtils::CloneNode(node, left);
  }
  else
  { 
    res = msiUtils::CloneNode(node, left);
    if (NS_SUCCEEDED(res))
      res = msiUtils::CloneNode(node, right);
    if (NS_SUCCEEDED(res) && left && right)
    {
      PRUint32 num_to_delete = count - offset;
      PRUint32 i(0);
      for (i = 1; i <= num_to_delete; i++)
      {
        nsCOMPtr<nsIDOMNode> child, dontcare;
        left->GetLastChild(getter_AddRefs(child));
        if (child)
          left->RemoveChild(child, getter_AddRefs(dontcare));
      }
      num_to_delete = offset;
      for (i = 1; i <= num_to_delete; i++)
      {
        nsCOMPtr<nsIDOMNode> child, dontcare;
        right->GetFirstChild(getter_AddRefs(child));
        if (child)
          right->RemoveChild(child, getter_AddRefs(dontcare));
      }
    }
    else 
      res = NS_ERROR_FAILURE;  
  } 
  return res;  
}

nsresult msiUtils::GetNSEventFromMouseEvent(nsIDOMMouseEvent* mouseEvent, 
                                            nsEvent ** nsEvent)
{
  nsresult res(NS_ERROR_FAILURE);
  if (!mouseEvent || !nsEvent)
    return NS_ERROR_FAILURE;
  *nsEvent = nsnull;
  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(mouseEvent));
  if (privateEvent)
     res = privateEvent->GetInternalNSEvent(nsEvent);
  return res;  
}

//nsresult msiUtils::GetGUIEventFromMouseEvent(nsIDOMMouseEvent* mouseEvent, 
//                                             nsGUIEvent ** guiEvent)
//{
//  nsresult res(NS_ERROR_FAILURE);
//  if (!mouseEvent || !guiEvent)
//    return NS_ERROR_FAILURE;
//  *guiEvent = nsnull;
//  nsEvent * internalEvent = nsnull;
//  nsCOMPtr<nsIPrivateDOMEvent> privateEvent(do_QueryInterface(mouseEvent));
//  if (privateEvent)
//     privateEvent->GetInternalNSEvent(&internalEvent);
//  if (internalEvent && internalEvent->eventStructType == NS_MOUSE_EVENT)
//    guiEvent = static_cast<nsGUIEvent*>(internalEvent);
//  if (!guiEvent)
//  {
//     NS_ASSERTION(PR_FALSE, "internal event is not a guiEvent.");
//    res = NS_ERROR_FAILURE;
//  }  
//  return res;  
//}
  
nsresult msiUtils::GetScreenPointFromMouseEvent(nsIDOMMouseEvent* mouseEvent, 
                                                nsPoint & point)                                     
{
  if (!mouseEvent)
    return NS_ERROR_FAILURE;
  PRInt32 screenX(NS_MAXSIZE), screenY(NS_MAXSIZE);
  mouseEvent->GetScreenX(&screenX);
  mouseEvent->GetScreenY(&screenY);
  point = nsPoint(screenX, screenY);
  return NS_OK;
}
