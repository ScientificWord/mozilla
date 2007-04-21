// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

/******

  This file contains the list of all msi MathML Edit Strings
  It is designed to be used as inline input to msiMMLEditStrings.cpp
  through C preprocessing.
  
  All entires must be enclosed in the macro:
  MSIMMLEDITSTRING(_name, _value)
  
  The first argument to MSIMMLEDITSTRING is the C++ identifier of the atom
  The second argument is the string value of the atom
  A string gets implemented as static const nsString

 ******/
 
// MathML PresentationElements
MSI_ATOM(maction, "maction")
MSI_ATOM(maligngroup, "maligngroup")
MSI_ATOM(malignmark, "malignmark")
MSI_ATOM(math,"math")
MSI_ATOM(menclose, "menclose")
MSI_ATOM(merror, "merror")
MSI_ATOM(mfenced, "mfenced")
MSI_ATOM(mfrac, "mfrac")
MSI_ATOM(mglyph, "mglyph")
MSI_ATOM(mi, "mi")
MSI_ATOM(mlabeledtr, "mlabeledtr")
MSI_ATOM(mmultiscripts, "mmultiscripts")
MSI_ATOM(mn, "mn")
MSI_ATOM(mo, "mo")
MSI_ATOM(mover, "mover")
MSI_ATOM(mpadded, "mpadded")
MSI_ATOM(mphantom, "mphantom")
MSI_ATOM(mroot, "mroot")
MSI_ATOM(mrow, "mrow")
MSI_ATOM(ms, "ms")
MSI_ATOM(mspace, "mspace")
MSI_ATOM(msqrt, "msqrt")
MSI_ATOM(mstyle, "mstyle")
MSI_ATOM(msub, "msub")
MSI_ATOM(msubsup, "msubsup")
MSI_ATOM(msup, "msup")
MSI_ATOM(mtable, "mtable")
MSI_ATOM(mtd, "mtd")
MSI_ATOM(mtext, "mtext")
MSI_ATOM(mtr, "mtr")
MSI_ATOM(munder, "munder")
MSI_ATOM(munderover, "munderover")
 
MSI_ATOM(enginefunction, "enginefunction")
MSI_ATOM(msicaretpos, "msicaretpos")
MSI_ATOM(msicaretpostext, "msicaretpostext")
MSI_ATOM(msiclass, "msiclass")
MSI_ATOM(msimathname, "msimathname")
MSI_ATOM(msiunit, "msiunit")
MSI_ATOM(msitrue, "true")
MSI_ATOM(msifalse, "false")
MSI_ATOM(tempinput, "tempinput")
MSI_ATOM(emptyString, "")


// Math attributes
MSI_ATOM(display, "display")
MSI_ATOM(mode, "mode")
MSI_ATOM(block, "block")
MSI_ATOM(msiinline, "inline")
MSI_ATOM(xmlns, "xmlns")


MSI_ATOM(form, "form")
MSI_ATOM(prefix, "prefix")
MSI_ATOM(postfix, "postfix")
MSI_ATOM(infix, "infix")
MSI_ATOM(stretchy, "stretchy")
MSI_ATOM(fence, "fence")
MSI_ATOM(accent,"accent")
MSI_ATOM(largeop, "largeop")
MSI_ATOM(separator, "separator")
MSI_ATOM(movablelimits, "movablelimits")
MSI_ATOM(symmetric, "symmetric")
MSI_ATOM(rspace, "rspace")
MSI_ATOM(lspace, "lspace")
MSI_ATOM(maxsize, "maxsize")
MSI_ATOM(minsize, "minsize")
MSI_ATOM(msiLimitPlacement, "msiLimitPlacement")
MSI_ATOM(msiLimitsAtRight, "msiLimitsAtRight")
MSI_ATOM(msiLimitsAboveBelow, "msiLimitsAboveBelow")
MSI_ATOM(msiBoundFence, "msiBoundFence")
MSI_ATOM(msiSingleNodeStyle, "msiSingleNodeStyle")
MSI_ATOM(linethickness, "linethickness")
MSI_ATOM(displaystyle, "displaystyle")
MSI_ATOM(subscriptshift, "subscriptshift")
MSI_ATOM(superscriptshift, "superscriptshift")
MSI_ATOM(accentunder, "accentunder")


//MFenced attributes
//TODO These should be temporary since we won't create MFenced objects
MSI_ATOM(open, "open")
MSI_ATOM(close, "close")
MSI_ATOM(separators, "separators")

//Coalesce attributes
MSI_ATOM(coalesceswitch, "coalesceswitch")
MSI_ATOM(coalescestop, "stop")
MSI_ATOM(coalescestart, "start")

//Decorations
MSI_ATOM(label, "label")


// Duplicates from nsGKAtomList
MSI_ATOM(textFrame, "TextFrame")
