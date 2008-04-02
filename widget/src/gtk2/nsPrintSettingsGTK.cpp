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
 * The Initial Developer of the Original Code is Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Michael Ventnor <m.ventnor@gmail.com>
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

#include "nsPrintSettingsGTK.h"
#include "nsILocalFile.h"
#include "nsNetUtil.h"
#include <stdlib.h>

static
gboolean ref_printer(GtkPrinter *aPrinter, gpointer aData)
{
  ((nsPrintSettingsGTK*) aData)->SetGtkPrinter(aPrinter);
  return TRUE;
}

static
gboolean printer_enumerator(GtkPrinter *aPrinter, gpointer aData)
{
  if (gtk_printer_is_default(aPrinter))
    return ref_printer(aPrinter, aData);

  return FALSE; // Keep 'em coming...
}

static
GtkPaperSize* moz_gtk_paper_size_copy_to_new_custom(GtkPaperSize* oldPaperSize)
{
  // We make a "custom-ified" copy of the paper size so it can be changed later.
  return gtk_paper_size_new_custom(gtk_paper_size_get_name(oldPaperSize),
                                   gtk_paper_size_get_display_name(oldPaperSize),
                                   gtk_paper_size_get_width(oldPaperSize, GTK_UNIT_INCH),
                                   gtk_paper_size_get_height(oldPaperSize, GTK_UNIT_INCH),
                                   GTK_UNIT_INCH);
}

NS_IMPL_ISUPPORTS_INHERITED1(nsPrintSettingsGTK, 
                             nsPrintSettings, 
                             nsPrintSettingsGTK)

/** ---------------------------------------------------
 */
nsPrintSettingsGTK::nsPrintSettingsGTK() :
  mPageSetup(NULL),
  mPrintSettings(NULL),
  mGTKPrinter(NULL)
{
  // The aim here is to set up the objects enough that silent printing works well.
  // These will be replaced anyway if the print dialog is used.
  mPrintSettings = gtk_print_settings_new();
  mPageSetup = gtk_page_setup_new();

  SetOutputFormat(nsIPrintSettings::kOutputFormatNative);

  GtkPaperSize* defaultPaperSize = gtk_paper_size_new(NULL);
  mPaperSize = moz_gtk_paper_size_copy_to_new_custom(defaultPaperSize);
  gtk_paper_size_free(defaultPaperSize);
  SaveNewPageSize();
}

/** ---------------------------------------------------
 */
nsPrintSettingsGTK::~nsPrintSettingsGTK()
{
  if (mPageSetup) {
    g_object_unref(mPageSetup);
    mPageSetup = NULL;
  }
  if (mPrintSettings) {
    g_object_unref(mPrintSettings);
    mPrintSettings = NULL;
  }
  if (mGTKPrinter) {
    g_object_unref(mGTKPrinter);
    mGTKPrinter = NULL;
  }
  gtk_paper_size_free(mPaperSize);
}

/** ---------------------------------------------------
 */
nsPrintSettingsGTK::nsPrintSettingsGTK(const nsPrintSettingsGTK& aPS) :
  mPageSetup(NULL),
  mPrintSettings(NULL),
  mGTKPrinter(NULL),
  mPrintSelectionOnly(PR_FALSE)
{
  *this = aPS;
}

/** ---------------------------------------------------
 */
nsPrintSettingsGTK& nsPrintSettingsGTK::operator=(const nsPrintSettingsGTK& rhs)
{
  if (this == &rhs) {
    return *this;
  }
  
  nsPrintSettings::operator=(rhs);

  if (mPageSetup)
    g_object_unref(mPageSetup);
  mPageSetup = gtk_page_setup_copy(rhs.mPageSetup);

  if (mPrintSettings)
    g_object_unref(mPrintSettings);
  mPrintSettings = gtk_print_settings_copy(rhs.mPrintSettings);

  if (mGTKPrinter)
    g_object_unref(mGTKPrinter);
  mGTKPrinter = (GtkPrinter*) g_object_ref(rhs.mGTKPrinter);

  mPrintSelectionOnly = rhs.mPrintSelectionOnly;

  return *this;
}

/** -------------------------------------------
 */
nsresult nsPrintSettingsGTK::_Clone(nsIPrintSettings **_retval)
{
  NS_ENSURE_ARG_POINTER(_retval);
  *_retval = nsnull;
  
  nsPrintSettingsGTK *newSettings = new nsPrintSettingsGTK(*this);
  if (!newSettings)
    return NS_ERROR_FAILURE;
  *_retval = newSettings;
  NS_ADDREF(*_retval);
  return NS_OK;
}


/** -------------------------------------------
 */
NS_IMETHODIMP
nsPrintSettingsGTK::_Assign(nsIPrintSettings *aPS)
{
  nsPrintSettingsGTK *printSettingsGTK = static_cast<nsPrintSettingsGTK*>(aPS);
  if (!printSettingsGTK)
    return NS_ERROR_UNEXPECTED;
  *this = *printSettingsGTK;
  return NS_OK;
}

/** ---------------------------------------------------
 */
void
nsPrintSettingsGTK::SetGtkPageSetup(GtkPageSetup *aPageSetup)
{
  if (mPageSetup)
    g_object_unref(mPageSetup);
  
  mPageSetup = (GtkPageSetup*) g_object_ref(aPageSetup);

  // We make a custom copy of the GtkPaperSize so it can be mutable. If a
  // GtkPaperSize wasn't made as custom, its properties are immutable.
  GtkPaperSize* newPaperSize = gtk_page_setup_get_paper_size(aPageSetup);
  if (newPaperSize) { // Yes, this can be null
    gtk_paper_size_free(mPaperSize);
    mPaperSize = moz_gtk_paper_size_copy_to_new_custom(newPaperSize);
  }
  // If newPaperSize was not null, we must update our twin too (GtkPrintSettings).
  // If newPaperSize was null, we must set this object to use mPaperSize.
  SaveNewPageSize();
}

/** ---------------------------------------------------
 */
void
nsPrintSettingsGTK::SetGtkPrintSettings(GtkPrintSettings *aPrintSettings)
{
  if (mPrintSettings)
    g_object_unref(mPrintSettings);
  
  mPrintSettings = (GtkPrintSettings*) g_object_ref(aPrintSettings);

  GtkPaperSize* newPaperSize = gtk_print_settings_get_paper_size(aPrintSettings);
  if (newPaperSize) {
    gtk_paper_size_free(mPaperSize);
    mPaperSize = moz_gtk_paper_size_copy_to_new_custom(newPaperSize);
  }
  SaveNewPageSize();
}

/** ---------------------------------------------------
 */
void
nsPrintSettingsGTK::SetGtkPrinter(GtkPrinter *aPrinter)
{
  if (mGTKPrinter)
    g_object_unref(mGTKPrinter);
  
  mGTKPrinter = (GtkPrinter*) g_object_ref(aPrinter);
}

/**
 * Reimplementation of nsPrintSettings functions so that we get the values
 * from the GTK objects rather than our own variables.
 */

/* attribute long printRange; */
NS_IMETHODIMP nsPrintSettingsGTK::GetPrintRange(PRInt16 *aPrintRange)
{
  NS_ENSURE_ARG_POINTER(aPrintRange);
  if (mPrintSelectionOnly) {
    *aPrintRange = kRangeSelection;
    return NS_OK;
  }

  GtkPrintPages gtkRange = gtk_print_settings_get_print_pages(mPrintSettings);
  if (gtkRange == GTK_PRINT_PAGES_RANGES)
    *aPrintRange = kRangeSpecifiedPageRange;
  else
    *aPrintRange = kRangeAllPages;

  return NS_OK;
}
NS_IMETHODIMP nsPrintSettingsGTK::SetPrintRange(PRInt16 aPrintRange)
{
  if (aPrintRange == kRangeSelection) {
    mPrintSelectionOnly = PR_TRUE;
    return NS_OK;
  }

  mPrintSelectionOnly = PR_FALSE;
  if (aPrintRange == kRangeSpecifiedPageRange)
    gtk_print_settings_set_print_pages(mPrintSettings, GTK_PRINT_PAGES_RANGES);
  else
    gtk_print_settings_set_print_pages(mPrintSettings, GTK_PRINT_PAGES_ALL);
  return NS_OK;
}

/* attribute long startPageRange; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetStartPageRange(PRInt32 *aStartPageRange)
{
  gint ctRanges;
  GtkPageRange* lstRanges = gtk_print_settings_get_page_ranges(mPrintSettings, &ctRanges);

  // Make sure we got a range.
  if (ctRanges < 1) {
    *aStartPageRange = 1;
  } else {
    // GTK supports multiple page ranges; gecko only supports 1. So find
    // the lowest start page.
    PRInt32 start(lstRanges[0].start);
    for (gint ii = 1; ii < ctRanges; ii++) {
      start = PR_MIN(lstRanges[ii].start, start);
    }
    *aStartPageRange = start + 1;
  }

  g_free(lstRanges);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetStartPageRange(PRInt32 aStartPageRange)
{
  PRInt32 endRange;
  GetEndPageRange(&endRange);

  GtkPageRange gtkRange;
  gtkRange.start = aStartPageRange - 1;
  gtkRange.end = endRange - 1;

  gtk_print_settings_set_page_ranges(mPrintSettings, &gtkRange, 1);

  return NS_OK;
}

/* attribute long endPageRange; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetEndPageRange(PRInt32 *aEndPageRange)
{
  gint ctRanges;
  GtkPageRange* lstRanges = gtk_print_settings_get_page_ranges(mPrintSettings, &ctRanges);

  if (ctRanges < 1) {
    *aEndPageRange = 1;
  } else {
    PRInt32 end(lstRanges[0].end);
    for (gint ii = 1; ii < ctRanges; ii++) {
      end = PR_MAX(lstRanges[ii].end, end);
    }
    *aEndPageRange = end + 1;
  }

  g_free(lstRanges);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetEndPageRange(PRInt32 aEndPageRange)
{
  PRInt32 startRange;
  GetStartPageRange(&startRange);

  GtkPageRange gtkRange;
  gtkRange.start = startRange - 1;
  gtkRange.end = aEndPageRange - 1;

  gtk_print_settings_set_page_ranges(mPrintSettings, &gtkRange, 1);

  return NS_OK;
}

/* attribute boolean printReversed; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetPrintReversed(PRBool *aPrintReversed)
{
  *aPrintReversed = gtk_print_settings_get_reverse(mPrintSettings);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetPrintReversed(PRBool aPrintReversed)
{
  gtk_print_settings_set_reverse(mPrintSettings, aPrintReversed);
  return NS_OK;
}

/* attribute boolean printInColor; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetPrintInColor(PRBool *aPrintInColor)
{
  *aPrintInColor = gtk_print_settings_get_use_color(mPrintSettings);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetPrintInColor(PRBool aPrintInColor)
{
  gtk_print_settings_set_use_color(mPrintSettings, aPrintInColor);
  return NS_OK;
}

/* attribute short orientation; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetOrientation(PRInt32 *aOrientation)
{
  NS_ENSURE_ARG_POINTER(aOrientation);

  GtkPageOrientation gtkOrient = gtk_page_setup_get_orientation(mPageSetup);
  switch (gtkOrient) {
    case GTK_PAGE_ORIENTATION_LANDSCAPE:
    case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
      *aOrientation = kLandscapeOrientation;
      break;

    case GTK_PAGE_ORIENTATION_PORTRAIT:
    case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
    default:
      *aOrientation = kPortraitOrientation;
  }
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetOrientation(PRInt32 aOrientation)
{
  GtkPageOrientation gtkOrient;
  if (aOrientation == kLandscapeOrientation)
    gtkOrient = GTK_PAGE_ORIENTATION_LANDSCAPE;
  else
    gtkOrient = GTK_PAGE_ORIENTATION_PORTRAIT;

  gtk_print_settings_set_orientation(mPrintSettings, gtkOrient);
  gtk_page_setup_set_orientation(mPageSetup, gtkOrient);
  return NS_OK;
}

/* attribute wstring toFileName; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetToFileName(PRUnichar * *aToFileName)
{
  // Get the gtk output filename
  const char* gtk_output_uri = gtk_print_settings_get(mPrintSettings, GTK_PRINT_SETTINGS_OUTPUT_URI);
  if (!gtk_output_uri) {
    *aToFileName = ToNewUnicode(mToFileName);
    return NS_OK;
  }

  // Convert to an nsIFile
  nsCOMPtr<nsIFile> file;
  NS_GetFileFromURLSpec(nsDependentCString(gtk_output_uri), getter_AddRefs(file));

  // Extract the path
  nsAutoString path;
  file->GetPath(path);

  *aToFileName = ToNewUnicode(path);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetToFileName(const PRUnichar * aToFileName)
{
  if (aToFileName[0] == 0) {
    mToFileName.SetLength(0);
    gtk_print_settings_set(mPrintSettings, GTK_PRINT_SETTINGS_OUTPUT_URI, NULL);
    return NS_OK;
  }

  if (StringEndsWith(nsDependentString(aToFileName), NS_LITERAL_STRING(".ps"))) {
    gtk_print_settings_set(mPrintSettings, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, "ps");
  } else {
    gtk_print_settings_set(mPrintSettings, GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT, "pdf");
  }

  nsCOMPtr<nsILocalFile> file;
  NS_NewLocalFile(nsDependentString(aToFileName), PR_TRUE, getter_AddRefs(file));

  // Convert the nsIFile to a URL
  nsCAutoString url;
  NS_GetURLSpecFromFile(file, url);

  gtk_print_settings_set(mPrintSettings, GTK_PRINT_SETTINGS_OUTPUT_URI, url.get());
  mToFileName = aToFileName;

  return NS_OK;
}

NS_IMETHODIMP
nsPrintSettingsGTK::GetPrinterName(PRUnichar * *aPrinter)
{
  const char* gtkPrintName = gtk_print_settings_get_printer(mPrintSettings);
  if (!gtkPrintName) {
    if (GTK_IS_PRINTER(mGTKPrinter)) {
      gtkPrintName = gtk_printer_get_name(mGTKPrinter);
    } else {
      // This mimics what nsPrintSettingsImpl does when we try to Get before we Set
      nsXPIDLString nullPrintName;
      *aPrinter = ToNewUnicode(nullPrintName);
      return NS_OK;
    }
  }
  *aPrinter = ToNewUnicode(nsDependentCString(gtkPrintName));
  return NS_OK;
}

NS_IMETHODIMP
nsPrintSettingsGTK::SetPrinterName(const PRUnichar * aPrinter)
{
  NS_ConvertUTF16toUTF8 gtkPrinter(aPrinter);

  if (StringBeginsWith(gtkPrinter, NS_LITERAL_CSTRING("PostScript/"))) {
    // Don't bother importing this name
    gtkPrinter.AssignLiteral("");
  }

  if (StringBeginsWith(gtkPrinter, NS_LITERAL_CSTRING("CUPS/"))) {
    // Strip off "CUPS/"; GTK might recognize the rest
    gtkPrinter.Cut(0, strlen("CUPS/"));
  }

  if (!gtkPrinter.Equals(gtk_print_settings_get_printer(mPrintSettings))) {
    mIsInitedFromPrinter = PR_FALSE;
    mIsInitedFromPrefs = PR_FALSE;
    gtk_print_settings_set_printer(mPrintSettings, gtkPrinter.get());
  }

  return NS_OK;
}

/* attribute long numCopies; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetNumCopies(PRInt32 *aNumCopies)
{
  NS_ENSURE_ARG_POINTER(aNumCopies);
  *aNumCopies = gtk_print_settings_get_n_copies(mPrintSettings);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetNumCopies(PRInt32 aNumCopies)
{
  gtk_print_settings_set_n_copies(mPrintSettings, aNumCopies);
  return NS_OK;
}

/* attribute double edgeTop; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetEdgeTop(double *aEdgeTop)
{
  NS_ENSURE_ARG_POINTER(aEdgeTop);
  *aEdgeTop = gtk_page_setup_get_top_margin(mPageSetup, GTK_UNIT_INCH);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetEdgeTop(double aEdgeTop)
{
  gtk_page_setup_set_top_margin(mPageSetup, aEdgeTop, GTK_UNIT_INCH);
  return NS_OK;
}

/* attribute double edgeLeft; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetEdgeLeft(double *aEdgeLeft)
{
  NS_ENSURE_ARG_POINTER(aEdgeLeft);
  *aEdgeLeft = gtk_page_setup_get_left_margin(mPageSetup, GTK_UNIT_INCH);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetEdgeLeft(double aEdgeLeft)
{
  gtk_page_setup_set_left_margin(mPageSetup, aEdgeLeft, GTK_UNIT_INCH);
  return NS_OK;
}

/* attribute double edgeBottom; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetEdgeBottom(double *aEdgeBottom)
{
  NS_ENSURE_ARG_POINTER(aEdgeBottom);
  *aEdgeBottom = gtk_page_setup_get_bottom_margin(mPageSetup, GTK_UNIT_INCH);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetEdgeBottom(double aEdgeBottom)
{
  gtk_page_setup_set_bottom_margin(mPageSetup, aEdgeBottom, GTK_UNIT_INCH);
  return NS_OK;
}

/* attribute double edgeRight; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetEdgeRight(double *aEdgeRight)
{
  NS_ENSURE_ARG_POINTER(aEdgeRight);
  *aEdgeRight = gtk_page_setup_get_right_margin(mPageSetup, GTK_UNIT_INCH);
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetEdgeRight(double aEdgeRight)
{
  gtk_page_setup_set_right_margin(mPageSetup, aEdgeRight, GTK_UNIT_INCH);
  return NS_OK;
}

/* attribute double scaling; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetScaling(double *aScaling)
{
  *aScaling = gtk_print_settings_get_scale(mPrintSettings) / 100.0;
  return NS_OK;
}

NS_IMETHODIMP
nsPrintSettingsGTK::SetScaling(double aScaling)
{
  gtk_print_settings_set_scale(mPrintSettings, aScaling * 100.0);
  return NS_OK;
}

/* attribute wstring paperName; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetPaperName(PRUnichar * *aPaperName)
{
  NS_ENSURE_ARG_POINTER(aPaperName);
  *aPaperName = ToNewUnicode(NS_ConvertUTF8toUTF16(gtk_paper_size_get_name(mPaperSize)));
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetPaperName(const PRUnichar * aPaperName)
{
  NS_ConvertUTF16toUTF8 gtkPaperName(aPaperName);

  // Convert these Gecko names to GTK names
  if (gtkPaperName.EqualsIgnoreCase("letter"))
    gtkPaperName.AssignLiteral(GTK_PAPER_NAME_LETTER);
  else if (gtkPaperName.EqualsIgnoreCase("legal"))
    gtkPaperName.AssignLiteral(GTK_PAPER_NAME_LEGAL);

  // Try to get the display name from the name so our paper size fits in the Page Setup dialog.
  GtkPaperSize* paperSize = gtk_paper_size_new(gtkPaperName.get());
  char* displayName = strdup(gtk_paper_size_get_display_name(paperSize));
  gtk_paper_size_free(paperSize);

  paperSize = gtk_paper_size_new_custom(gtkPaperName.get(), displayName,
                                        gtk_paper_size_get_width(mPaperSize, GTK_UNIT_INCH),
                                        gtk_paper_size_get_height(mPaperSize, GTK_UNIT_INCH),
                                        GTK_UNIT_INCH);

  free(displayName);
  gtk_paper_size_free(mPaperSize);
  mPaperSize = paperSize;
  SaveNewPageSize();
  return NS_OK;
}

GtkUnit
nsPrintSettingsGTK::GetGTKUnit(PRInt16 aGeckoUnit)
{
  if (aGeckoUnit == kPaperSizeMillimeters)
    return GTK_UNIT_MM;
  else
    return GTK_UNIT_INCH;
}

void
nsPrintSettingsGTK::SaveNewPageSize()
{
  gtk_print_settings_set_paper_size(mPrintSettings, mPaperSize);
  gtk_page_setup_set_paper_size(mPageSetup, mPaperSize);
}

/* attribute double paperWidth; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetPaperWidth(double *aPaperWidth)
{
  NS_ENSURE_ARG_POINTER(aPaperWidth);
  *aPaperWidth = gtk_paper_size_get_width(mPaperSize, GetGTKUnit(mPaperSizeUnit));
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetPaperWidth(double aPaperWidth)
{
  gtk_paper_size_set_size(mPaperSize,
                          aPaperWidth,
                          gtk_paper_size_get_height(mPaperSize, GetGTKUnit(mPaperSizeUnit)),
                          GetGTKUnit(mPaperSizeUnit));
  SaveNewPageSize();
  return NS_OK;
}

/* attribute double paperHeight; */
NS_IMETHODIMP
nsPrintSettingsGTK::GetPaperHeight(double *aPaperHeight)
{
  NS_ENSURE_ARG_POINTER(aPaperHeight);
  *aPaperHeight = gtk_paper_size_get_height(mPaperSize, GetGTKUnit(mPaperSizeUnit));
  return NS_OK;
}
NS_IMETHODIMP
nsPrintSettingsGTK::SetPaperHeight(double aPaperHeight)
{
  gtk_paper_size_set_size(mPaperSize,
                          gtk_paper_size_get_width(mPaperSize, GetGTKUnit(mPaperSizeUnit)),
                          aPaperHeight,
                          GetGTKUnit(mPaperSizeUnit));
  SaveNewPageSize();
  return NS_OK;
}

NS_IMETHODIMP
nsPrintSettingsGTK::SetPaperSizeUnit(PRInt16 aPaperSizeUnit)
{
  // Convert units internally. e.g. they might have set the values while we're still in mm but
  // they change to inch just afterwards, expecting that their sizes are in inches.
  gtk_paper_size_set_size(mPaperSize,
                          gtk_paper_size_get_width(mPaperSize, GetGTKUnit(mPaperSizeUnit)),
                          gtk_paper_size_get_height(mPaperSize, GetGTKUnit(mPaperSizeUnit)),
                          GetGTKUnit(aPaperSizeUnit));
  SaveNewPageSize();

  mPaperSizeUnit = aPaperSizeUnit;
  return NS_OK;
}

// Get/Set our margins as an nsMargin
NS_IMETHODIMP
nsPrintSettingsGTK::SetEdgeInTwips(nsMargin& aEdge)
{
  gtk_page_setup_set_top_margin(mPageSetup, NS_TWIPS_TO_INCHES(aEdge.top), GTK_UNIT_INCH);
  gtk_page_setup_set_left_margin(mPageSetup, NS_TWIPS_TO_INCHES(aEdge.left), GTK_UNIT_INCH);
  gtk_page_setup_set_bottom_margin(mPageSetup, NS_TWIPS_TO_INCHES(aEdge.bottom), GTK_UNIT_INCH);
  gtk_page_setup_set_right_margin(mPageSetup, NS_TWIPS_TO_INCHES(aEdge.right), GTK_UNIT_INCH);
  return NS_OK;
}

NS_IMETHODIMP
nsPrintSettingsGTK::GetEdgeInTwips(nsMargin& aEdge)
{
  aEdge.SizeTo(NS_INCHES_TO_TWIPS(gtk_page_setup_get_left_margin(mPageSetup, GTK_UNIT_INCH)),
               NS_INCHES_TO_TWIPS(gtk_page_setup_get_top_margin(mPageSetup, GTK_UNIT_INCH)),
               NS_INCHES_TO_TWIPS(gtk_page_setup_get_right_margin(mPageSetup, GTK_UNIT_INCH)),
               NS_INCHES_TO_TWIPS(gtk_page_setup_get_bottom_margin(mPageSetup, GTK_UNIT_INCH)));
  return NS_OK;
}

NS_IMETHODIMP
nsPrintSettingsGTK::GetEffectivePageSize(double *aWidth, double *aHeight)
{
  *aWidth  = NS_INCHES_TO_TWIPS(gtk_paper_size_get_width(mPaperSize, GTK_UNIT_INCH));
  *aHeight = NS_INCHES_TO_TWIPS(gtk_paper_size_get_height(mPaperSize, GTK_UNIT_INCH));

  GtkPageOrientation gtkOrient = gtk_page_setup_get_orientation(mPageSetup);

  if (gtkOrient == GTK_PAGE_ORIENTATION_LANDSCAPE ||
      gtkOrient == GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE) {
    double temp = *aWidth;
    *aWidth = *aHeight;
    *aHeight = temp;
  }
  return NS_OK;
}

NS_IMETHODIMP
nsPrintSettingsGTK::SetupSilentPrinting()
{
  // We have to get a printer here, rather than when the print settings are constructed.
  // This is because when we request sync, GTK makes us wait in the *event loop* while waiting
  // for the enumeration to finish. We must do this when event loop runs are expected.
  gtk_enumerate_printers(printer_enumerator, this, NULL, TRUE);

  // XXX If no default printer set, get the first one.
  if (!GTK_IS_PRINTER(mGTKPrinter))
    gtk_enumerate_printers(ref_printer, this, NULL, TRUE);

  return NS_OK;
}
