/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* ex: set tabstop=8 softtabstop=2 shiftwidth=2 expandtab: */
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
 *   Brian Stell <bstell@netscape.com>
 *   Louie Zhao  <louie.zhao@sun.com>
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

#include "gfx-config.h"
#include "nsFT2FontNode.h"
#include "nsFontDebug.h"

#if (!defined(MOZ_ENABLE_FREETYPE2))

void nsFT2FontNode::FreeGlobals() {}

nsresult nsFT2FontNode::InitGlobals()
{
  FREETYPE_FONT_PRINTF(("freetype not compiled in"));
  NS_WARNING("freetype not compiled in");
  return NS_OK;
}

void nsFT2FontNode::GetFontNames(const char* aPattern,
                                 nsFontNodeArray* aNodes) {}

#else

#include "nsFreeType.h"
#include "nsFontFreeType.h"
#include "nsIServiceManager.h"
#include "nsIMutableArray.h"
#include "nsArrayUtils.h"

nsHashtable* nsFT2FontNode::mFreeTypeNodes = nsnull;
PRBool       nsFT2FontNode::sInited = PR_FALSE;
nsIFontCatalogService* nsFT2FontNode::sFcs = nsnull;

void
nsFT2FontNode::FreeGlobals()
{
  NS_IF_RELEASE(sFcs);

  if (mFreeTypeNodes) {
    mFreeTypeNodes->Reset(FreeNode, nsnull);
    delete mFreeTypeNodes;
    mFreeTypeNodes = nsnull;
  }
  sInited = PR_FALSE;
}

nsresult
nsFT2FontNode::InitGlobals()
{
  NS_ASSERTION(sInited==PR_FALSE,
               "nsFT2FontNode::InitGlobals called more than once");
  
  sInited = PR_TRUE;

  nsresult rv = ::CallGetService("@mozilla.org/gfx/xfontcatalogservice;1",
                                 &sFcs);

  if (NS_FAILED(rv)) {
    NS_ERROR("Font Catalog Service init failed\n");
    return NS_ERROR_FAILURE;
  }

  mFreeTypeNodes = new nsHashtable();
  if (!mFreeTypeNodes)
    return NS_ERROR_FAILURE;
  
  LoadNodeTable();
  WeightTableInitCorrection(nsFreeTypeFont::sLinearWeightTable,
                            nsFreeType2::gAATTDarkTextMinValue,
                            nsFreeType2::gAATTDarkTextGain);
  
  return NS_OK;
}

void
nsFT2FontNode::GetFontNames(const char* aPattern, nsFontNodeArray* aNodes)
{
  int j;
  PRBool rslt;
  PRUint32 count, i;
  char *pattern, *foundry, *family, *charset, *encoding;
  const char *charSetName;
  nsFontNode *node;
  nsCOMPtr<nsIArray> arrayFC;
  nsCAutoString familyTmp, languageTmp;

  FONT_CATALOG_PRINTF(("looking for FreeType font matching %s", aPattern));
  nsCAutoString patt(aPattern);
  ToLowerCase(patt);
  pattern = strdup(patt.get());
  NS_ASSERTION(pattern, "failed to copy pattern");
  if (!pattern)
    goto cleanup_and_return;

  rslt = ParseXLFD(pattern, &foundry, &family, &charset, &encoding);
  if (!rslt)
    goto cleanup_and_return;

  // unable to handle "name-charset-*"
  if (charset && !encoding) {
    goto cleanup_and_return;
  }

  if (family)
    familyTmp.Assign(family);

  sFcs->GetFontCatalogEntries(familyTmp, languageTmp, 0, 0, 0, 0,
                              getter_AddRefs(arrayFC));
  if (!arrayFC)
    goto cleanup_and_return;
  arrayFC->GetLength(&count);
  for (i = 0; i < count; i++) {
    nsCOMPtr<nsITrueTypeFontCatalogEntry> fce = do_QueryElementAt(arrayFC, i);
    if (!fce)
      continue;
    nsCAutoString foundryName, familyName;
    PRUint32 flags, codePageRange1, codePageRange2;
    PRUint16 weight, width;
    fce->GetFamilyName(familyName);
    fce->GetFlags(&flags);
    fce->GetWidth(&width);
    fce->GetWeight(&weight);
    fce->GetCodePageRange1(&codePageRange1);
    fce->GetCodePageRange2(&codePageRange2);
    if (!charset) { // get all encoding
      FONT_CATALOG_PRINTF(("found FreeType %s-%s-*-*", foundryName.get(),
                           familyName.get()));
      for (j=0; j<32; j++) {
        unsigned long bit = 1 << j;
        if (bit & codePageRange1) {
          charSetName = nsFreeType2::GetRange1CharSetName(bit);
          NS_ASSERTION(charSetName, "failed to get charset name");
          if (!charSetName)
            continue;
          node = LoadNode(fce, charSetName, aNodes);
        }
        if (bit & codePageRange2) {
          charSetName = nsFreeType2::GetRange2CharSetName(bit);
          if (!charSetName)
            continue;
          LoadNode(fce, charSetName, aNodes);
        }
      }
      if (foundryName.IsEmpty() && !familyName.IsEmpty() && flags&FCE_FLAGS_SYMBOL) {
        // the "registry-encoding" is not used but LoadNode will fail without
        // some value for this
        LoadNode(fce, "symbol-fontspecific", aNodes);
      }
    }

    if (charset && encoding) { // get this specific encoding
      PRUint32 cpr1_bits, cpr2_bits;
      nsCAutoString charsetName(charset);
      charsetName.Append('-');
      charsetName.Append(encoding);
      CharSetNameToCodeRangeBits(charsetName.get(), &cpr1_bits, &cpr2_bits);
      if (!(cpr1_bits & codePageRange1)
          && !(cpr2_bits & codePageRange2))
        continue;
      FONT_CATALOG_PRINTF(("found FreeType -%s-%s-%s",
                           familyName.get(),charset,encoding));
      LoadNode(fce, charsetName.get(), aNodes);
    }
  }

  FREE_IF(pattern);
  return;

cleanup_and_return:
  FONT_CATALOG_PRINTF(("nsFT2FontNode::GetFontNames failed"));
  FREE_IF(pattern);
  return;
}

nsFontNode*
nsFT2FontNode::LoadNode(nsITrueTypeFontCatalogEntry *aFce,
                        const char *aCharSetName,
                        nsFontNodeArray* aNodes)
{
  nsFontCharSetMap *charSetMap = GetCharSetMap(aCharSetName);
  if (!charSetMap->mInfo) {
    return nsnull;
  }
  nsCAutoString nodeName, familyName;
  aFce->GetVendorID(nodeName);
  aFce->GetFamilyName(familyName);

  nodeName.Append('-');
  nodeName.Append(familyName);
  nodeName.Append('-');
  nodeName.Append(aCharSetName);
  nsCStringKey key(nodeName);
  nsFontNode* node = (nsFontNode*) mFreeTypeNodes->Get(&key);
  if (!node) {
    node = new nsFontNode;
    if (!node) {
      return nsnull;
    }
    mFreeTypeNodes->Put(&key, node);
    node->mName = nodeName;
    nsFontCharSetMap *charSetMap = GetCharSetMap(aCharSetName);
    node->mCharSetInfo = charSetMap->mInfo;
  }

  PRInt64 styleFlags;
  PRUint16 fceWeight, fceWidth;
  aFce->GetStyleFlags(&styleFlags);
  aFce->GetWidth(&fceWidth);
  aFce->GetWeight(&fceWeight);

  int styleIndex;
  if (styleFlags & FT_STYLE_FLAG_ITALIC)
    styleIndex = NS_FONT_STYLE_ITALIC;
  else
    styleIndex = NS_FONT_STYLE_NORMAL;
  nsFontStyle* style = node->mStyles[styleIndex];
  if (!style) {
    style = new nsFontStyle;
    if (!style) {
      return nsnull;
    }
    node->mStyles[styleIndex] = style;
  }

  int weightIndex = WEIGHT_INDEX(fceWeight);
  nsFontWeight* weight = style->mWeights[weightIndex];
  if (!weight) {
    weight = new nsFontWeight;
    if (!weight) {
      return nsnull;
    }
    style->mWeights[weightIndex] = weight;
  }

  nsFontStretch* stretch = weight->mStretches[fceWidth];
  if (!stretch) {
    stretch = new nsFontStretch;
    if (!stretch) {
      return nsnull;
    }
    weight->mStretches[fceWidth] = stretch;
  }
  if (!stretch->mFreeTypeFaceID) {
    stretch->mFreeTypeFaceID = aFce;
  }
  if (aNodes) {
    int i, n, found = 0;
    n = aNodes->Count();
    for (i=0; i<n; i++) {
      if (aNodes->GetElement(i) == node) {
        found = 1;
      }
    }
    if (!found) {
      aNodes->AppendElement(node);
    }
  }
  return node;
}

PRBool
nsFT2FontNode::LoadNodeTable()
{
  int j;
  nsCOMPtr<nsIArray> arrayFC;
  nsCAutoString family, language;
  sFcs->GetFontCatalogEntries(family, language, 0, 0, 0, 0,
                              getter_AddRefs(arrayFC));
  if (!arrayFC)
    return PR_FALSE;
  PRUint32 count, i;
  arrayFC->GetLength(&count);
  for (i = 0; i < count; i++) {
    const char *charsetName;
    nsCOMPtr<nsITrueTypeFontCatalogEntry> fce = do_QueryElementAt(arrayFC, i);
    if (!fce)
      continue;
    PRUint32 flags, codePageRange1, codePageRange2;
    PRUint16 weight, width;
    fce->GetFlags(&flags);
    fce->GetWidth(&width);
    fce->GetWeight(&weight);
    fce->GetCodePageRange1(&codePageRange1);
    fce->GetCodePageRange2(&codePageRange2);
    if ((!flags&FCE_FLAGS_ISVALID)
        || (weight < 100) || (weight > 900) || (width > 8))
      continue;
    for (j=0; j<32; j++) {
      unsigned long bit = 1 << j;
      if (!(bit & codePageRange1))
        continue;
      charsetName = nsFreeType2::GetRange1CharSetName(bit);
      NS_ASSERTION(charsetName, "failed to get charset name");
      if (!charsetName)
        continue;
      LoadNode(fce, charsetName, nsnull);
    }
    for (j=0; j<32; j++) {
      unsigned long bit = 1 << j;
      if (!(bit & codePageRange2))
        continue;
      charsetName = nsFreeType2::GetRange2CharSetName(bit);
      if (!charsetName)
        continue;
      LoadNode(fce, charsetName, nsnull);
    }
  }
  return PR_TRUE;
}

//
// Parse XLFD
// The input is a typical XLFD string.
//
// the XLFD entries look like this:
//  -adobe-courier-medium-r-normal--12-120-75-75-m-70-iso8859-1
//  -adobe-courier-medium-r-*-*-12-*-*-*-*-*-*-*
//
// the fields are:
// -foundry-family-weight-slant-width-style-pixelsize-pointsize-
//    resolution_x-resolution_y-spacing-avg_width-registry-encoding
//
// see ftp://ftp.x.org/pub/R6.4/xc/doc/hardcopy/XLFD
//
PRBool
nsFT2FontNode::ParseXLFD(char *aPattern, char** aFoundry, char** aFamily,
                         char** aCharset, char** aEncoding)
{
  char *p;
  int i;

  *aFoundry  = nsnull;
  *aFamily   = nsnull;
  *aCharset  = nsnull;
  *aEncoding = nsnull;

  // start of pattern
  p = aPattern;
  NS_ASSERTION(*p == '-',"garbled pattern: does not start with a '-'");
  if (*p++ != '-')
    return PR_FALSE;

  // foundry
  NS_ASSERTION(*p,"garbled pattern: unexpected end of pattern");
  if (!*p)
    return PR_FALSE;
  if (*p == '*')
    *aFoundry = nsnull;
  else
    *aFoundry = p;
  while (*p && (*p!='-'))
    p++;
  if (!*p)
    return PR_TRUE; // short XLFD
  NS_ASSERTION(*p == '-',"garbled pattern: cannot find end of foundry");
  *p++ = '\0';

  // family
  if (!*p)
    return PR_TRUE; // short XLFD
  if (*p == '*')
    *aFamily = nsnull;
  else
    *aFamily = p;
  while (*p && (*p!='-'))
    p++;
  if (!*p)
    return PR_TRUE; // short XLFD
  NS_ASSERTION(*p == '-',"garbled pattern: cannot find end of family");
  *p++ = '\0';

  // skip forward to charset
  for (i=0; i<10; i++) {
    while (*p && (*p!='-'))
      p++;
    if (!*p)
      return PR_TRUE; // short XLFD
    *p++ = '\0';
  }

  // charset
  NS_ASSERTION(*p,"garbled pattern: unexpected end of pattern");
  if (!*p)
    return PR_FALSE;
  if (*p == '*')
    *aCharset = nsnull;
  else
    *aCharset = p;
  while (*p && (*p!='-'))
    p++;
  if (!*p)
    return PR_TRUE; // short XLFD
  NS_ASSERTION(*p == '-',"garbled pattern: cannot find end of charset");
  *p++ = '\0';

  // encoding
  NS_ASSERTION(*p,"garbled pattern: unexpected end of pattern");
  if (!*p)
    return PR_FALSE;
  if (*p == '*')
    *aEncoding = nsnull;
  else
    *aEncoding = p;
  while (*p && (*p!='-'))
    p++;
  if (*p)
    return PR_TRUE; // short XLFD
  return PR_TRUE;
}

#endif

