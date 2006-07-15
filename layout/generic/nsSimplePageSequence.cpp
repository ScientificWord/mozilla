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
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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
#include "nsCOMPtr.h" 
#include "nsReadableUtils.h"
#include "nsSimplePageSequence.h"
#include "nsPresContext.h"
#include "nsIRenderingContext.h"
#include "nsHTMLAtoms.h"
#include "nsIDeviceContext.h"
#include "nsIViewManager.h"
#include "nsIPresShell.h"
#include "nsIFontMetrics.h"
#include "nsIPrintSettings.h"
#include "nsPageFrame.h"
#include "nsStyleConsts.h"
#include "nsRegion.h"
#include "nsLayoutAtoms.h"
#include "nsCSSFrameConstructor.h"
#include "nsContentUtils.h"
#include "nsDisplayList.h"

// DateTime Includes
#include "nsDateTimeFormatCID.h"

#define OFFSET_NOT_SET -1

// Print Options
#include "nsIPrintSettings.h"
#include "nsIPrintOptions.h"
#include "nsGfxCIID.h"
#include "nsIServiceManager.h"

static const char sPrintOptionsContractID[] = "@mozilla.org/gfx/printsettings-service;1";

//

#include "prlog.h"
#ifdef PR_LOGGING 
PRLogModuleInfo * kLayoutPrintingLogMod = PR_NewLogModule("printing-layout");
#define PR_PL(_p1)  PR_LOG(kLayoutPrintingLogMod, PR_LOG_DEBUG, _p1)
#else
#define PR_PL(_p1)
#endif

// This object a shared by all the nsPageFrames 
// parented to a SimplePageSequenceFrame
nsSharedPageData::nsSharedPageData() :
  mDateTimeStr(nsnull),
  mHeadFootFont(nsnull),
  mPageNumFormat(nsnull),
  mPageNumAndTotalsFormat(nsnull),
  mDocTitle(nsnull),
  mDocURL(nsnull),
  mReflowSize(0,0),
  mReflowMargin(0,0,0,0),
  mShadowSize(0,0),
  mExtraMargin(0,0,0,0),
  mEdgePaperMargin(0,0,0,0),
  mPageContentXMost(0),
  mPageContentSize(0)
{
}

nsSharedPageData::~nsSharedPageData()
{
  nsMemory::Free(mDateTimeStr);
  if (mHeadFootFont) delete mHeadFootFont;
  nsMemory::Free(mPageNumFormat);
  nsMemory::Free(mPageNumAndTotalsFormat);
  if (mDocTitle) nsMemory::Free(mDocTitle);
  if (mDocURL) nsMemory::Free(mDocURL);
}

nsIFrame*
NS_NewSimplePageSequenceFrame(nsIPresShell* aPresShell, nsStyleContext* aContext)
{
  return new (aPresShell) nsSimplePageSequenceFrame(aContext);
}

nsSimplePageSequenceFrame::nsSimplePageSequenceFrame(nsStyleContext* aContext) :
  nsContainerFrame(aContext),
  mTotalPages(-1),
  mSelectionHeight(-1),
  mYSelOffset(0)
{
  nscoord halfInch = NS_INCHES_TO_TWIPS(0.5);
  mMargin.SizeTo(halfInch, halfInch, halfInch, halfInch);

  mPageData = new nsSharedPageData();
  NS_ASSERTION(mPageData != nsnull, "Can't be null!");
  if (mPageData->mHeadFootFont == nsnull) {
    mPageData->mHeadFootFont = new nsFont("serif", NS_FONT_STYLE_NORMAL,NS_FONT_VARIANT_NORMAL,
                               NS_FONT_WEIGHT_NORMAL,0,NSIntPointsToTwips(10));
  }

  nsresult rv;
  mPageData->mPrintOptions = do_GetService(sPrintOptionsContractID, &rv);
  if (NS_SUCCEEDED(rv) && mPageData->mPrintOptions) {
    // now get the default font form the print options
    mPageData->mPrintOptions->GetDefaultFont(*mPageData->mHeadFootFont);
  }

  // Doing this here so we only have to go get these formats once
  SetPageNumberFormat("pagenumber",  "%1$d", PR_TRUE);
  SetPageNumberFormat("pageofpages", "%1$d of %2$d", PR_FALSE);
}

nsSimplePageSequenceFrame::~nsSimplePageSequenceFrame()
{
  if (mPageData) delete mPageData;
}

nsresult
nsSimplePageSequenceFrame::QueryInterface(const nsIID& aIID, void** aInstancePtr)
{
  NS_PRECONDITION(0 != aInstancePtr, "null ptr");
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER;
  }
  if (aIID.Equals(NS_GET_IID(nsIPageSequenceFrame))) {
    *aInstancePtr = (void*)(nsIPageSequenceFrame*)this;
    return NS_OK;
  }
  return nsContainerFrame::QueryInterface(aIID, aInstancePtr);
}

//----------------------------------------------------------------------

// Creates a continuing page frame
nsresult
nsSimplePageSequenceFrame::CreateContinuingPageFrame(nsPresContext* aPresContext,
                                                     nsIFrame*       aPageFrame,
                                                     nsIFrame**      aContinuingPage)
{
  // Create the continuing frame
  return aPresContext->PresShell()->FrameConstructor()->
    CreateContinuingFrame(aPresContext, aPageFrame, this, aContinuingPage);
}

void
nsSimplePageSequenceFrame::GetEdgePaperMarginCoord(char* aPrefName,
                                                   nscoord& aCoord)
{
  nsresult rv = mPageData->mPrintOptions->
    GetPrinterPrefInt(mPageData->mPrintSettings, 
                      NS_ConvertASCIItoUTF16(aPrefName).get(),
                      &aCoord);

  if (NS_SUCCEEDED(rv)) {
    nscoord inchInTwips = NS_INCHES_TO_TWIPS(1.0);
    aCoord = PR_MAX(NS_INCHES_TO_TWIPS(float(aCoord)/100.0f), 0);
    aCoord = PR_MIN(aCoord, inchInTwips); // an inch is still probably excessive
  }
}

void
nsSimplePageSequenceFrame::GetEdgePaperMargin(nsMargin& aMargin)
{
  aMargin.SizeTo(0,0,0,0);
  GetEdgePaperMarginCoord("print_edge_top",    aMargin.top);
  GetEdgePaperMarginCoord("print_edge_left",   aMargin.left);
  GetEdgePaperMarginCoord("print_edge_bottom", aMargin.bottom);
  GetEdgePaperMarginCoord("print_edge_right",  aMargin.right);
}

NS_IMETHODIMP
nsSimplePageSequenceFrame::Reflow(nsPresContext*          aPresContext,
                                  nsHTMLReflowMetrics&     aDesiredSize,
                                  const nsHTMLReflowState& aReflowState,
                                  nsReflowStatus&          aStatus)
{
  NS_PRECONDITION(aPresContext->IsRootPaginatedDocument(),
                  "A Page Sequence is only for real pages");
  DO_GLOBAL_REFLOW_COUNT("nsSimplePageSequenceFrame", aReflowState.reason);
  DISPLAY_REFLOW(aPresContext, this, aReflowState, aDesiredSize, aStatus);
  NS_FRAME_TRACE_REFLOW_IN("nsSimplePageSequenceFrame::Reflow");

  aStatus = NS_FRAME_COMPLETE;  // we're always complete

  if (eReflowReason_Resize == aReflowState.reason) {
    // Return our desired size
    aDesiredSize.height  = mSize.height;
    aDesiredSize.width   = mSize.width;
    aDesiredSize.ascent  = aDesiredSize.height;
    aDesiredSize.descent = 0;
    aDesiredSize.mOverflowArea = nsRect(0, 0, aDesiredSize.width,
                                        aDesiredSize.height);
    FinishAndStoreOverflow(&aDesiredSize);
    return NS_OK;
  }

  // Turn on the scaling of twips so any of the scrollbars
  // in the UI no longer get scaled
  PRBool isPrintPreview =
    aPresContext->Type() == nsPresContext::eContext_PrintPreview;
  if (isPrintPreview) {
    aPresContext->SetScalingOfTwips(PR_TRUE);
  }

  // See if we can get a Print Settings from the Context
  if (!mPageData->mPrintSettings &&
      aPresContext->Medium() == nsLayoutAtoms::print) {
      mPageData->mPrintSettings = aPresContext->GetPrintSettings();
  }

  // now get out margins
  if (mPageData->mPrintSettings) {
    mPageData->mPrintSettings->GetMarginInTwips(mMargin);
    PRInt16 printType;
    mPageData->mPrintSettings->GetPrintRange(&printType);
    mPrintRangeType = printType;
  }

  // *** Special Override ***
  // If this is a sub-sdoc (meaning it doesn't take the whole page)
  // and if this Document is in the upper left hand corner
  // we need to suppress the top margin or it will reflow too small

  nsSize pageSize = aPresContext->GetPageSize();

  mPageData->mReflowSize = pageSize;
  mPageData->mReflowMargin = mMargin;

  // Compute the size of each page and the x coordinate that each page will
  // be placed at
  GetEdgePaperMargin(mPageData->mEdgePaperMargin);
  nscoord extraThreshold = PR_MAX(pageSize.width, pageSize.height)/10;
  PRInt32 gapInTwips = nsContentUtils::GetIntPref("print.print_extra_margin");

  gapInTwips = PR_MAX(gapInTwips, 0);
  gapInTwips = PR_MIN(gapInTwips, extraThreshold); // clamp to 1/10 of the largest dim of the page

  nscoord extraGap = nscoord(gapInTwips);

  nscoord  deadSpaceGap;
  GetDeadSpaceValue(&deadSpaceGap);

  nsMargin extraMargin(0,0,0,0);
  nsSize   shadowSize(0,0);
  if (aPresContext->IsScreen()) {
    extraMargin.SizeTo(extraGap, extraGap, extraGap, extraGap);
    nscoord fourPixels = aPresContext->IntScaledPixelsToTwips(4);
    shadowSize.SizeTo(fourPixels, fourPixels);
  }

  mPageData->mShadowSize      = shadowSize;
  mPageData->mExtraMargin     = extraMargin;

  const nscoord x = deadSpaceGap;
  nscoord y = deadSpaceGap;// Running y-offset for each page

  nsSize reflowPageSize(0,0);

  nsSize availSize(pageSize.width + shadowSize.width + extraMargin.LeftRight(),
                   pageSize.height + shadowSize.height +
                   extraMargin.TopBottom());

  // See if it's an incremental reflow command
  if (!aPresContext->IsDynamic() &&
      eReflowReason_Incremental == aReflowState.reason) {
    // XXX Skip Incremental reflow, 
    // in fact, all we want is the initial reflow
    y = mRect.height;
  } else {
    nsReflowReason  reflowReason = aReflowState.reason;

    // Tile the pages vertically
    nsHTMLReflowMetrics kidSize(nsnull);
    for (nsIFrame* kidFrame = mFrames.FirstChild(); nsnull != kidFrame; ) {
      // Reflow the page
      nsHTMLReflowState kidReflowState(aPresContext, aReflowState, kidFrame,
                                       availSize, reflowReason);
      nsReflowStatus  status;

      kidReflowState.mComputedWidth  = kidReflowState.availableWidth;
      //kidReflowState.mComputedHeight = kidReflowState.availableHeight;
      PR_PL(("AV W: %d   H: %d\n", kidReflowState.availableWidth, kidReflowState.availableHeight));

      // Set the shared data into the page frame before reflow
      nsPageFrame * pf = NS_STATIC_CAST(nsPageFrame*, kidFrame);
      pf->SetSharedPageData(mPageData);

      // Place and size the page. If the page is narrower than our
      // max width then center it horizontally
      ReflowChild(kidFrame, aPresContext, kidSize, kidReflowState, x, y, 0, status);

      reflowPageSize.SizeTo(kidSize.width, kidSize.height);

      FinishReflowChild(kidFrame, aPresContext, nsnull, kidSize, x, y, 0);
      y += kidSize.height;

      // Leave a slight gap between the pages
      y += deadSpaceGap;

      // Is the page complete?
      nsIFrame* kidNextInFlow = kidFrame->GetNextInFlow();

      if (NS_FRAME_IS_COMPLETE(status)) {
        NS_ASSERTION(nsnull == kidNextInFlow, "bad child flow list");
      } else if (nsnull == kidNextInFlow) {
        // The page isn't complete and it doesn't have a next-in-flow, so
        // create a continuing page
        nsIFrame*     continuingPage;
        CreateContinuingPageFrame(aPresContext, kidFrame, &continuingPage);

        // Add it to our child list
        kidFrame->SetNextSibling(continuingPage);
        reflowReason = eReflowReason_Initial;
      }

      // Get the next page
      kidFrame = kidFrame->GetNextSibling();
    }

    // Get Total Page Count
    nsIFrame* page;
    PRInt32 pageTot = 0;
    for (page = mFrames.FirstChild(); page; page = page->GetNextSibling()) {
      pageTot++;
    }

    // Set Page Number Info
    PRInt32 pageNum = 1;
    for (page = mFrames.FirstChild(); page; page = page->GetNextSibling()) {
      nsPageFrame * pf = NS_STATIC_CAST(nsPageFrame*, page);
      if (pf != nsnull) {
        pf->SetPageNumInfo(pageNum, pageTot);
      }
      pageNum++;
    }

    // Create current Date/Time String
    if (!mDateFormatter)
      mDateFormatter = do_CreateInstance(NS_DATETIMEFORMAT_CONTRACTID);
#ifndef WINCE
    NS_ENSURE_TRUE(mDateFormatter, NS_ERROR_FAILURE);

    nsAutoString formattedDateString;
    time_t ltime;
    time( &ltime );
    if (NS_SUCCEEDED(mDateFormatter->FormatTime(nsnull /* nsILocale* locale */,
                                                kDateFormatShort,
                                                kTimeFormatNoSeconds,
                                                ltime,
                                                formattedDateString))) {
      PRUnichar * uStr = ToNewUnicode(formattedDateString);
      SetDateTimeStr(uStr); // memory will be freed
    }
#endif
  }

  // Return our desired size
  aDesiredSize.height  = y; // includes page heights and dead space
  aDesiredSize.width   = x + availSize.width + deadSpaceGap;
  aDesiredSize.ascent  = aDesiredSize.height;
  aDesiredSize.descent = 0;

  aDesiredSize.mOverflowArea = nsRect(0, 0, aDesiredSize.width,
                                      aDesiredSize.height);
  FinishAndStoreOverflow(&aDesiredSize);

  // cache the size so we can set the desired size 
  // for the other reflows that happen
  mSize.width  = aDesiredSize.width;
  mSize.height = aDesiredSize.height;

  // Turn off the scaling of twips so any of the scrollbars
  // in the document get scaled
  if (isPrintPreview) {
    aPresContext->SetScalingOfTwips(PR_FALSE);
  }

  NS_FRAME_TRACE_REFLOW_OUT("nsSimplePageSequeceFrame::Reflow", aStatus);
  NS_FRAME_SET_TRUNCATION(aStatus, aReflowState, aDesiredSize);
  return NS_OK;
}

//----------------------------------------------------------------------

#ifdef DEBUG
NS_IMETHODIMP
nsSimplePageSequenceFrame::GetFrameName(nsAString& aResult) const
{
  return MakeFrameName(NS_LITERAL_STRING("SimplePageSequence"), aResult);
}
#endif

//====================================================================
//== Asynch Printing
//====================================================================
NS_IMETHODIMP
nsSimplePageSequenceFrame::GetCurrentPageNum(PRInt32* aPageNum)
{
  NS_ENSURE_ARG_POINTER(aPageNum);

  *aPageNum = mPageNum;
  return NS_OK;
}

NS_IMETHODIMP
nsSimplePageSequenceFrame::GetNumPages(PRInt32* aNumPages)
{
  NS_ENSURE_ARG_POINTER(aNumPages);

  *aNumPages = mTotalPages;
  return NS_OK;
}

NS_IMETHODIMP
nsSimplePageSequenceFrame::IsDoingPrintRange(PRBool* aDoing)
{
  NS_ENSURE_ARG_POINTER(aDoing);

  *aDoing = mDoingPageRange;
  return NS_OK;
}

NS_IMETHODIMP
nsSimplePageSequenceFrame::GetPrintRange(PRInt32* aFromPage, PRInt32* aToPage)
{
  NS_ENSURE_ARG_POINTER(aFromPage);
  NS_ENSURE_ARG_POINTER(aToPage);

  *aFromPage = mFromPageNum;
  *aToPage   = mToPageNum;
  return NS_OK;
}

// Helper Function
void 
nsSimplePageSequenceFrame::SetPageNumberFormat(const char* aPropName, const char* aDefPropVal, PRBool aPageNumOnly)
{
  // Doing this here so we only have to go get these formats once
  nsXPIDLString pageNumberFormat;
  // Now go get the Localized Page Formating String
  nsresult rv =
    nsContentUtils::GetLocalizedString(nsContentUtils::ePRINTING_PROPERTIES,
                                       aPropName, pageNumberFormat);
  if (NS_FAILED(rv)) { // back stop formatting
    pageNumberFormat.AssignASCII(aDefPropVal);
  }

  // Sets the format into a static data member which will own the memory and free it
  PRUnichar* uStr = ToNewUnicode(pageNumberFormat);
  if (uStr != nsnull) {
    SetPageNumberFormat(uStr, aPageNumOnly); // nsPageFrame will own the memory
  }

}

NS_IMETHODIMP
nsSimplePageSequenceFrame::StartPrint(nsPresContext*   aPresContext,
                                      nsIPrintSettings* aPrintSettings,
                                      PRUnichar*        aDocTitle,
                                      PRUnichar*        aDocURL)
{
  NS_ENSURE_ARG_POINTER(aPresContext);
  NS_ENSURE_ARG_POINTER(aPrintSettings);

  if (!mPageData->mPrintSettings) {
    mPageData->mPrintSettings = aPrintSettings;
  }

  // Only set them if they are not null
  if (aDocTitle) mPageData->mDocTitle = aDocTitle;
  if (aDocURL) mPageData->mDocURL   = aDocURL;

  aPrintSettings->GetStartPageRange(&mFromPageNum);
  aPrintSettings->GetEndPageRange(&mToPageNum);
  aPrintSettings->GetMarginInTwips(mMargin);

  mDoingPageRange = nsIPrintSettings::kRangeSpecifiedPageRange == mPrintRangeType ||
                    nsIPrintSettings::kRangeSelection == mPrintRangeType;

  // If printing a range of pages make sure at least the starting page
  // number is valid
  PRInt32 totalPages = mFrames.GetLength();

  if (mDoingPageRange) {
    if (mFromPageNum > totalPages) {
      return NS_ERROR_INVALID_ARG;
    }
  }

  // Begin printing of the document
  nsresult rv = NS_OK;

#if defined(DEBUG_rods) || defined(DEBUG_dcone)
  {
    nsIView* seqView = GetView();
    nsRect rect = GetRect();
    PR_PL(("Seq Frame: %p - [%5d,%5d,%5d,%5d] ", this, rect.x, rect.y, rect.width, rect.height));
    PR_PL(("view: %p ", seqView));
    if (seqView) {
      nsRect viewRect = seqView->GetBounds();
      PR_PL((" [%5d,%5d,%5d,%5d]", viewRect.x, viewRect.y, viewRect.width, viewRect.height));
    }
    PR_PL(("\n"));
  }

  {
    PRInt32 pageNum = 1;
    for (nsIFrame* page = mFrames.FirstChild(); page;
         page = page->GetNextSibling()) {
      nsIView* view = page->GetView();
      NS_ASSERTION(view, "no page view");
      nsRect rect = page->GetRect();
      nsRect viewRect = view->GetBounds();
      PR_PL((" Page: %p  No: %d - [%5d,%5d,%5d,%5d] ", page, pageNum, rect.x, rect.y, rect.width, rect.height));
      PR_PL((" [%5d,%5d,%5d,%5d]\n", viewRect.x, viewRect.y, viewRect.width, viewRect.height));
      pageNum++;
    }
  }
  //printf("***** Setting aPresContext %p is painting selection %d\n", aPresContext, nsIPrintSettings::kRangeSelection == mPrintRangeType);
#endif

  // Determine if we are rendering only the selection
  aPresContext->SetIsRenderingOnlySelection(nsIPrintSettings::kRangeSelection == mPrintRangeType);


  if (mDoingPageRange) {
    // XXX because of the hack for making the selection all print on one page
    // we must make sure that the page is sized correctly before printing.
    PRInt32 width, height;
    aPresContext->DeviceContext()->GetDeviceSurfaceDimensions(width, height);

    PRInt32 pageNum = 1;
    nscoord y = 0;//mMargin.top;

    for (nsIFrame* page = mFrames.FirstChild(); page;
         page = page->GetNextSibling()) {
      nsIView* view = page->GetView();
      NS_ASSERTION(view, "no page view");

      nsIViewManager* vm = view->GetViewManager();
      NS_ASSERTION(vm, "no view manager");

      if (pageNum < mFromPageNum || pageNum > mToPageNum) {
        // Hide the pages that won't be printed to the Viewmanager
        // doesn't put them in the display list. Also, makde
        // sure the child views don't get asked to print
        // but my guess is that there won't be any
        vm->SetViewVisibility(view, nsViewVisibility_kHide);
        nsRegion emptyRegion;
        vm->SetViewChildClipRegion(view, &emptyRegion);
      } else {
        nsRect rect = page->GetRect();
        rect.y = y;
        rect.height = height;
        page->SetRect(rect);

        nsRect viewRect = view->GetBounds();
        viewRect.y = y;
        viewRect.height = height;
        vm->MoveViewTo(view, viewRect.x, viewRect.y);
        viewRect.x = 0;
        viewRect.y = 0;
        vm->ResizeView(view, viewRect);
        y += rect.height + mMargin.top + mMargin.bottom;
      }
      pageNum++;
    }

    // adjust total number of pages
    if (nsIPrintSettings::kRangeSelection != mPrintRangeType) {
      totalPages = pageNum - 1;
    }
  }

  // XXX - This wouldn't have to be done each time
  // but it isn't that expensive and this the best place 
  // to have access to a localized file properties file
  // 
  // Note: because this is done here it makes a little bit harder
  // to have UI for setting the header/footer font name and size
  //
  // Get default font name and size to be used for the headers and footers
  nsXPIDLString fontName;
  rv = nsContentUtils::GetLocalizedString(nsContentUtils::ePRINTING_PROPERTIES,
                                         "fontname", fontName);
  if (NS_FAILED(rv)) {
    fontName.AssignLiteral("serif");
  }

  nsXPIDLString fontSizeStr;
  nscoord      pointSize = 10;;
  rv = nsContentUtils::GetLocalizedString(nsContentUtils::ePRINTING_PROPERTIES,
                                          "fontsize", fontSizeStr);
  if (NS_SUCCEEDED(rv)) {
    PRInt32 errCode;
    pointSize = fontSizeStr.ToInteger(&errCode);
    if (NS_FAILED(errCode)) {
      pointSize = 10;
    }
  }
  mPageData->mPrintOptions->SetFontNamePointSize(fontName, pointSize);

  // Doing this here so we only have to go get these formats once
  SetPageNumberFormat("pagenumber",  "%1$d", PR_TRUE);
  SetPageNumberFormat("pageofpages", "%1$d of %2$d", PR_FALSE);

  mPageNum          = 1;
  mCurrentPageFrame = mFrames.FirstChild();

  if (mTotalPages == -1) {
    mTotalPages = totalPages;
  }

  return rv;
}

NS_IMETHODIMP
nsSimplePageSequenceFrame::PrintNextPage()
{
  // Print each specified page
  // pageNum keeps track of the current page and what pages are printing
  //
  // printedPageNum keeps track of the current page number to be printed
  // Note: When print al the pages or a page range the printed page shows the
  // actual page number, when printing selection it prints the page number starting
  // with the first page of the selection. For example if the user has a 
  // selection that starts on page 2 and ends on page 3, the page numbers when
  // print are 1 and then two (which is different than printing a page range, where
  // the page numbers would have been 2 and then 3)

  if (mCurrentPageFrame == nsnull) {
    return NS_ERROR_FAILURE;
  }

  PRBool printEvenPages, printOddPages;
  mPageData->mPrintSettings->GetPrintOptions(nsIPrintSettings::kPrintEvenPages, &printEvenPages);
  mPageData->mPrintSettings->GetPrintOptions(nsIPrintSettings::kPrintOddPages, &printOddPages);

  // Begin printing of the document
  nsIDeviceContext *dc = GetPresContext()->DeviceContext();

  nsIViewManager* vm = GetPresContext()->GetViewManager();

  nsresult rv = NS_OK;

  // See whether we should print this page
  mPrintThisPage = PR_TRUE;

  // If printing a range of pages check whether the page number is in the
  // range of pages to print
  if (mDoingPageRange) {
    if (mPageNum < mFromPageNum) {
      mPrintThisPage = PR_FALSE;
    } else if (mPageNum > mToPageNum) {
      mPageNum++;
      mCurrentPageFrame = nsnull;
      return NS_OK;
    }
  }

  // Check for printing of odd and even pages
  if (mPageNum & 0x1) {
    if (!printOddPages) {
      mPrintThisPage = PR_FALSE;  // don't print odd numbered page
    }
  } else {
    if (!printEvenPages) {
      mPrintThisPage = PR_FALSE;  // don't print even numbered page
    }
  }
  
  if (nsIPrintSettings::kRangeSelection == mPrintRangeType) {
    mPrintThisPage = PR_TRUE;
  }

  if (mPrintThisPage) {
    // XXX This is temporary fix for printing more than one page of a selection
    // This does a poor man's "dump" pagination (see Bug 89353)
    // It has laid out as one long page and now we are just moving or view up/down 
    // one page at a time and printing the contents of what is exposed by the rect.
    // currently this does not work for IFrames
    // I will soon improve this to work with IFrames 
    PRBool  continuePrinting = PR_TRUE;
    PRInt32 width, height;
    dc->GetDeviceSurfaceDimensions(width, height);
    nsRect clipRect(0, 0, width, height);
    height -= mMargin.top + mMargin.bottom;
    width  -= mMargin.left + mMargin.right;
    nscoord selectionY = height;
    nsIView* containerView = nsnull;
    nsRect   containerRect;
    if (mSelectionHeight > -1) {
      nsIFrame* childFrame = mFrames.FirstChild();
      nsIFrame* conFrame = childFrame->GetFirstChild(nsnull);
      containerView = conFrame->GetView();
      NS_ASSERTION(containerView, "Container view can't be null!");
      containerRect = containerView->GetBounds();
      containerRect.y -= mYSelOffset;

      vm->MoveViewTo(containerView, containerRect.x, containerRect.y);
      nsRect r(0, 0, containerRect.width, containerRect.height);
      vm->ResizeView(containerView, r, PR_FALSE);
      clipRect.SetRect(mMargin.left, mMargin.right, width, height);
    }

    // cast the frame to be a page frame
    nsPageFrame * pf = NS_STATIC_CAST(nsPageFrame*, mCurrentPageFrame);
    pf->SetPageNumInfo(mPageNum, mTotalPages);
    pf->SetSharedPageData(mPageData);

    PRInt32 printedPageNum = 1;
    while (continuePrinting) {
      if (GetPresContext()->IsRootPaginatedDocument()) {
        PR_PL(("\n"));
        PR_PL(("***************** BeginPage *****************\n"));
        rv = dc->BeginPage();
        NS_ENSURE_SUCCESS(rv, rv);
      }


      // Print the page
      nsIView* view = mCurrentPageFrame->GetView();

      NS_ASSERTION(view, "no page view");

      PR_PL(("SeqFr::Paint -> %p PageNo: %d  View: %p", pf, mPageNum, view));

      vm->SetViewContentTransparency(view, PR_FALSE);

      vm->Display(view, 0, 0, clipRect);

      if (mSelectionHeight > -1 && selectionY < mSelectionHeight) {
        selectionY += height;

        printedPageNum++;
        pf->SetPageNumInfo(printedPageNum, mTotalPages);
        containerRect.y -= height;
        containerRect.height += height;
        vm->MoveViewTo(containerView, containerRect.x, containerRect.y);
        nsRect r(0, 0, containerRect.width, containerRect.height);
        vm->ResizeView(containerView, r, PR_FALSE);

        PR_PL(("***************** End Page (PrintNextPage) *****************\n"));
        rv = dc->EndPage();
        NS_ENSURE_SUCCESS(rv, rv);
      } else {
        continuePrinting = PR_FALSE;
      }
    }
  }
  return rv;
}

NS_IMETHODIMP
nsSimplePageSequenceFrame::DoPageEnd()
{
  nsresult rv = NS_OK;
  if (GetPresContext()->IsRootPaginatedDocument() && mPrintThisPage) {
    PR_PL(("***************** End Page (DoPageEnd) *****************\n"));
    rv = GetPresContext()->DeviceContext()->EndPage();
    NS_ENSURE_SUCCESS(rv, rv);
  }

  mPageNum++;

  if (mCurrentPageFrame) {
    mCurrentPageFrame = mCurrentPageFrame->GetNextSibling();
  }
  
  return rv;
}

//------------------------------------------------------------------------------
NS_IMETHODIMP
nsSimplePageSequenceFrame::BuildDisplayList(nsDisplayListBuilder*   aBuilder,
                                            const nsRect&           aDirtyRect,
                                            const nsDisplayListSet& aLists)
{
  nsresult rv = DisplayBorderBackgroundOutline(aBuilder, aLists);
  NS_ENSURE_SUCCESS(rv, rv);

  // Treat each page as a psuedo-stack so everything goes in the Content() list.
  return
    BuildDisplayListForNonBlockChildren(aBuilder, aDirtyRect, aLists,
                                        DISPLAY_CHILD_FORCE_PSEUDO_STACKING_CONTEXT);
}

nsIAtom*
nsSimplePageSequenceFrame::GetType() const
{
  return nsLayoutAtoms::sequenceFrame; 
}

//------------------------------------------------------------------------------
void
nsSimplePageSequenceFrame::SetPageNumberFormat(PRUnichar * aFormatStr, PRBool aForPageNumOnly)
{ 
  NS_ASSERTION(aFormatStr != nsnull, "Format string cannot be null!");
  NS_ASSERTION(mPageData != nsnull, "mPageData string cannot be null!");

  if (aForPageNumOnly) {
    if (mPageData->mPageNumFormat != nsnull) {
      nsMemory::Free(mPageData->mPageNumFormat);
    }
    mPageData->mPageNumFormat = aFormatStr;
  } else {
    if (mPageData->mPageNumAndTotalsFormat != nsnull) {
      nsMemory::Free(mPageData->mPageNumAndTotalsFormat);
    }
    mPageData->mPageNumAndTotalsFormat = aFormatStr;
  }
}

//------------------------------------------------------------------------------
void
nsSimplePageSequenceFrame::SetDateTimeStr(PRUnichar * aDateTimeStr)
{ 
  NS_ASSERTION(aDateTimeStr != nsnull, "DateTime string cannot be null!");
  NS_ASSERTION(mPageData != nsnull, "mPageData string cannot be null!");

  if (mPageData->mDateTimeStr != nsnull) {
    nsMemory::Free(mPageData->mDateTimeStr);
  }
  mPageData->mDateTimeStr = aDateTimeStr;
}

//------------------------------------------------------------------------------
// For Shrink To Fit
//
// Return the percentage that the page needs to shrink to 
//
NS_IMETHODIMP
nsSimplePageSequenceFrame::GetSTFPercent(float& aSTFPercent)
{
  NS_ENSURE_TRUE(mPageData, NS_ERROR_UNEXPECTED);
  aSTFPercent = 1.0f;
  if (mPageData && (mPageData->mPageContentXMost > mPageData->mPageContentSize)) {
    aSTFPercent = float(mPageData->mPageContentSize) / float(mPageData->mPageContentXMost);
  }
  return NS_OK;
}
