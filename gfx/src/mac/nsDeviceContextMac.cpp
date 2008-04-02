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
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

#include "nsDeviceContextMac.h"
#include "nsRenderingContextMac.h"
#include "nsIPrintingContext.h"
#include "nsString.h"
#include "nsHashtable.h"
#include "nsFont.h"
#include <Gestalt.h>
#include <Appearance.h>
#include <TextEncodingConverter.h>
#include <TextCommon.h>
#include <StringCompare.h>
#include <Fonts.h>
#include <Resources.h>
#include <MacWindows.h>
#include <FixMath.h>
#include "nsIPref.h"
#include "nsIServiceManager.h"
#include "nsQuickSort.h"
#include "nsUnicodeMappingUtil.h"
#include "nsCarbonHelpers.h"
#include "nsRegionMac.h"
#include "nsIScreenManager.h"
#include "nsIServiceManager.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"


PRUint32 nsDeviceContextMac::mPixelsPerInch = 96;
PRUint32 nsDeviceContextMac::sNumberOfScreens = 0;


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
nsDeviceContextMac :: nsDeviceContextMac()
  : DeviceContextImpl(),
    mOldPort(nsnull)
{
}

/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
nsDeviceContextMac :: ~nsDeviceContextMac()
{
}

/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac :: Init(nsNativeWidget aNativeWidget)
{
  // cache the screen manager service for later
  nsresult ignore;
  mScreenManager = do_GetService("@mozilla.org/gfx/screenmanager;1", &ignore);
  NS_ASSERTION ( mScreenManager, "No screen manager, we're in trouble" );
  if ( !mScreenManager )
    return NS_ERROR_FAILURE;

  // figure out how many monitors there are.
  if ( !sNumberOfScreens )
    mScreenManager->GetNumberOfScreens(&sNumberOfScreens);

	// get resolution. Ensure that mPixelsToTwips is integral or we 
	// run into serious rounding problems.
  double pix_inch = GetScreenResolution();		//Fix2X((**thepix).hRes);
  mPixelsToTwips = nscoord(NSIntPointsToTwips(72)/(float)pix_inch);
  mTwipsToPixels = 1.0f/mPixelsToTwips;
	
  return DeviceContextImpl::Init(aNativeWidget);
}


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac :: CreateRenderingContext(nsIRenderingContext *&aContext)
{
#ifdef NS_PRINT_PREVIEW
  // Defer to Alt when there is one
  if (mAltDC && ((mUseAltDC & kUseAltDCFor_CREATERC_PAINT) || (mUseAltDC & kUseAltDCFor_CREATERC_REFLOW))) {
    return mAltDC->CreateRenderingContext(aContext);
  }
#endif

  nsRenderingContextMac *pContext;
  nsresult              rv;

	pContext = new nsRenderingContextMac();
  if (nsnull != pContext) {
    NS_ADDREF(pContext);

    CGrafPtr  thePort;
   	::GetPort((GrafPtr*)&thePort);

    rv = pContext->Init(this, thePort);
  }
  else
    rv = NS_ERROR_OUT_OF_MEMORY;

  if (NS_OK != rv){
    NS_IF_RELEASE(pContext);
  }
  aContext = pContext;
  return rv;
}

/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac :: SupportsNativeWidgets(PRBool &aSupportsWidgets)
{
  //XXX it is very critical that this not lie!! MMP
  
  // ��� VERY IMPORTANT (pinkerton)
  // This routine should return true if the widgets behave like Win32
  // "windows", that is they paint themselves and the app never knows about
  // them or has to send them update events. We were returning false which
  // meant that raptor needed to make sure to redraw them. However, if we
  // return false, the widgets never get created because the CreateWidget()
  // call in nsView never creates the widget. If we return true (which makes
  // things work), we are lying because the controls really need those
  // precious update/repaint events.
  // 
  // The situation we need is the following:
  // - return false from SupportsNativeWidgets()
  // - Create() is called on widgets when the above case is true.
  // 
  // Raptor currently doesn't work this way and needs to be fixed.
  // (please remove this comment when this situation is rectified)
  
  if( nsnull != mSpec){
  	aSupportsWidgets = PR_FALSE;
  } else {
  	aSupportsWidgets = PR_TRUE;
  }
  
  //if (nsnull == mSurface)
    
  //else
   //aSupportsWidgets = PR_FALSE;

  return NS_OK;
}


// helper function to get the system font for a specific script
#define FONTNAME_MAX_UNICHRS sizeof(fontName255) * 2
nsresult 
GetSystemFontForScript(ThemeFontID aFontID, ScriptCode aScriptCode,
                       nsAFlatString& aFontName, SInt16& aFontSize,
                       Style& aFontStyle)
{
  Str255 fontName255;
  ::GetThemeFont(aFontID, aScriptCode, fontName255, &aFontSize, &aFontStyle);
  if (fontName255[0] == 255) {
    NS_WARNING("Too long fong name (> 254 chrs)");
    return NS_ERROR_FAILURE;
  }
  fontName255[fontName255[0]+1] = 0;
        
  OSStatus err;
  // the theme font could contains font name in different encoding. 
  // we need to covert them to unicode according to the font's text encoding.

  TECObjectRef converter = 0;
  TextEncoding unicodeEncoding = 
    ::CreateTextEncoding(kTextEncodingUnicodeDefault, 
                         kTextEncodingDefaultVariant,
                         kTextEncodingDefaultFormat);
                                                              
  FMFontFamily fontFamily;
  TextEncoding fontEncoding = 0;
  fontFamily = ::FMGetFontFamilyFromName(fontName255);
  err = ::FMGetFontFamilyTextEncoding(fontFamily, &fontEncoding);

  if (err != noErr) {
    NS_WARNING("Could not get the encoding for the font.");
    return NS_ERROR_FAILURE;
  }
  err = ::TECCreateConverter(&converter, fontEncoding, unicodeEncoding);
  if (err != noErr) {
    NS_WARNING("Could not create the converter.");
    return NS_ERROR_FAILURE;
  }
  PRUnichar unicodeFontName[FONTNAME_MAX_UNICHRS + 1];
  ByteCount actualInputLength, actualOutputLength;
  err = ::TECConvertText(converter, &fontName255[1], fontName255[0], 
                         &actualInputLength, 
                         (TextPtr)unicodeFontName,
                         FONTNAME_MAX_UNICHRS * sizeof(PRUnichar),
                         &actualOutputLength);  
  if (err != noErr) {
    NS_WARNING("Could not convert the font name.");
    return NS_ERROR_FAILURE;
  }

  ::TECDisposeConverter(converter);

  unicodeFontName[actualOutputLength / sizeof(PRUnichar)] = PRUnichar('\0');
  aFontName = nsDependentString(unicodeFontName);
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac :: GetSystemFont(nsSystemFontID aID, nsFont *aFont) const
{
  nsresult status = NS_OK;

  switch (aID) {
    //---------
    // CSS System Fonts
    //
    //   Important: don't chage the code below, or make sure to preserve
    //   some compatibility with MacIE5 - developers will appreciate.
    //   Run the testcase in bug 3371 in quirks mode and strict mode.
    //---------
        // css2
    case eSystemFont_Caption:
    case eSystemFont_Icon: 
    case eSystemFont_Menu: 
    case eSystemFont_MessageBox: 
    case eSystemFont_SmallCaption: 
    case eSystemFont_StatusBar: 
        // css3
    case eSystemFont_Window:
    case eSystemFont_Document:
    case eSystemFont_Workspace:
    case eSystemFont_Desktop:
    case eSystemFont_Info:
    case eSystemFont_Dialog:
    case eSystemFont_Button:
    case eSystemFont_PullDownMenu:
    case eSystemFont_List:
    case eSystemFont_Field:
        // moz
    case eSystemFont_Tooltips:
    case eSystemFont_Widget:
      float  dev2app;
      dev2app = DevUnitsToAppUnits();

      aFont->style       = NS_FONT_STYLE_NORMAL;
      aFont->weight      = NS_FONT_WEIGHT_NORMAL;
      aFont->decorations = NS_FONT_DECORATION_NONE;

      if (aID == eSystemFont_Window ||
          aID == eSystemFont_Document) {
            aFont->name.AssignLiteral("sans-serif");
            aFont->size = NSToCoordRound(aFont->size * 0.875f); // quick hack
      }
      else
      {
        ThemeFontID fontID = kThemeViewsFont;
        switch (aID)
        {
              // css2
          case eSystemFont_Caption:       fontID = kThemeSystemFont;         break;
          case eSystemFont_Icon:          fontID = kThemeViewsFont;          break;
          case eSystemFont_Menu:          fontID = kThemeSystemFont;         break;
          case eSystemFont_MessageBox:    fontID = kThemeSmallSystemFont;    break;
          case eSystemFont_SmallCaption:  fontID = kThemeSmallEmphasizedSystemFont;  break;
          case eSystemFont_StatusBar:     fontID = kThemeSmallSystemFont;    break;
              // css3
          //case eSystemFont_Window:      = 'sans-serif'
          //case eSystemFont_Document:    = 'sans-serif'
          case eSystemFont_Workspace:     fontID = kThemeViewsFont;          break;
          case eSystemFont_Desktop:       fontID = kThemeViewsFont;          break;
          case eSystemFont_Info:          fontID = kThemeViewsFont;          break;
          case eSystemFont_Dialog:        fontID = kThemeSystemFont;         break;
          case eSystemFont_Button:        fontID = kThemePushButtonFont;     break;
          case eSystemFont_PullDownMenu:  fontID = kThemeMenuItemFont;       break;
          case eSystemFont_List:          fontID = kThemeSystemFont;         break;
          case eSystemFont_Field:         fontID = kThemeApplicationFont;    break;
              // moz
          case eSystemFont_Tooltips:      fontID = kThemeSmallSystemFont;    break;
          case eSystemFont_Widget:        fontID = kThemeSmallSystemFont;    break;
          default: break;
        }

        nsAutoString fontName;
        SInt16 fontSize;
        Style fontStyle;

        ScriptCode sysScript = ::GetScriptManagerVariable (smSysScript);
        nsresult rv;
        rv = GetSystemFontForScript(fontID, smRoman,
                                    fontName, fontSize, fontStyle);
        if (NS_FAILED(rv))
          fontName = NS_LITERAL_STRING("Lucida Grande");

        if (sysScript != smRoman) {
          SInt16 localFontSize;
          Style localFontStyle;
          nsAutoString localSysFontName;
          rv = GetSystemFontForScript(fontID, sysScript,
                                      localSysFontName,
                                      localFontSize, localFontStyle);
          if (NS_SUCCEEDED(rv) && !fontName.Equals(localSysFontName)) {
            fontName += NS_LITERAL_STRING(",") + localSysFontName;
            fontSize = localFontSize;
            fontStyle = localFontStyle;
          }
        }
        aFont->name = fontName;        
        aFont->size = NSToCoordRound(float(fontSize) * dev2app);

        if (fontStyle & bold)
          aFont->weight = NS_FONT_WEIGHT_BOLD;
        if (fontStyle & italic)
          aFont->style = NS_FONT_STYLE_ITALIC;
        if (fontStyle & underline)
          aFont->decorations = NS_FONT_DECORATION_UNDERLINE;
      }
      break;

  }

  aFont->systemFont = PR_TRUE;

  return status;
}


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac::GetDepth(PRUint32& aDepth)
{
  /*
  nsCOMPtr<nsIScreen> screen;
  FindScreenForSurface ( getter_AddRefs(screen) );
  if ( screen ) {
    PRInt32 depth;
    screen->GetPixelDepth ( &depth );
    aDepth = static_cast<PRUint32>(depth);
  }
  else 
    aDepth = 1;
  */
  
  // The above seems correct, however, because of the way Mozilla 
  // rendering is set upQuickDraw will get confused when
  // blitting to a secondary screen with a different bit depth.
  // By always returning the bit depth of the primary screen, QD
  // can do the proper color mappings.
   
  if ( !mPrimaryScreen && mScreenManager )
    mScreenManager->GetPrimaryScreen ( getter_AddRefs(mPrimaryScreen) );  
    
  if(!mPrimaryScreen) {
    aDepth = 1;
    return NS_OK;
  }
   
  PRInt32 depth;
  mPrimaryScreen->GetPixelDepth ( &depth );
  aDepth = static_cast<PRUint32>(depth);
    
  return NS_OK;
}

/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac :: CheckFontExistence(const nsString& aFontName)
{
  	short fontNum;
	if (GetMacFontNumber(aFontName, fontNum))
		return NS_OK;
	else
		return NS_ERROR_FAILURE;
}


//
// FindScreenForSurface
//
// Determines which screen intersects the largest area of the given surface.
//
void
nsDeviceContextMac :: FindScreenForSurface ( nsIScreen** outScreen )
{
  // optimize for the case where we only have one monitor.
  if ( !mPrimaryScreen && mScreenManager )
    mScreenManager->GetPrimaryScreen ( getter_AddRefs(mPrimaryScreen) );  
  if ( sNumberOfScreens == 1 ) {
    NS_IF_ADDREF(*outScreen = mPrimaryScreen.get());
    return;
  }
  
  nsIWidget* widget = reinterpret_cast<nsIWidget*>(mWidget);      // PRAY!
  NS_ASSERTION ( widget, "No Widget --> No Window" );
  if ( !widget ) {
    NS_IF_ADDREF(*outScreen = mPrimaryScreen.get());              // bail out with the main screen just to be safe.
    return;
  }

#if !MOZ_WIDGET_COCOA
  // we have a widget stashed inside, get a native WindowRef out of it
 	WindowRef window = reinterpret_cast<WindowRef>(widget->GetNativeData(NS_NATIVE_DISPLAY));

  StPortSetter  setter(window);

  Rect bounds;
  ::GetWindowPortBounds ( window, &bounds );

  if ( mScreenManager ) {
    if ( !(bounds.top || bounds.left || bounds.bottom || bounds.right) ) {
      NS_WARNING ( "trying to find screen for sizeless window" );
      NS_IF_ADDREF(*outScreen = mPrimaryScreen.get());
    }
    else {
      // convert window bounds to global coordinates
      Point topLeft = { bounds.top, bounds.left };
      Point bottomRight = { bounds.bottom, bounds.right };
      ::LocalToGlobal ( &topLeft );
      ::LocalToGlobal ( &bottomRight );
      Rect globalWindowBounds = { topLeft.v, topLeft.h, bottomRight.v, bottomRight.h } ;
      
      mScreenManager->ScreenForRect ( globalWindowBounds.left, globalWindowBounds.top, 
                                       globalWindowBounds.bottom - globalWindowBounds.top, 
                                       globalWindowBounds.right - globalWindowBounds.left, outScreen );
    }
  }
  else
    *outScreen = nsnull;
#else
  // in cocoa, we don't have a windowPtr! bail out with the main screen
  NS_IF_ADDREF(*outScreen = mPrimaryScreen.get());
#endif
  
} // FindScreenForSurface


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac::GetDeviceSurfaceDimensions(PRInt32 & outWidth, PRInt32 & outHeight)
{
#ifdef NS_PRINT_PREVIEW
  // Defer to Alt when there is one
  if (mAltDC && (mUseAltDC & kUseAltDCFor_SURFACE_DIM)) {
    return mAltDC->GetDeviceSurfaceDimensions(outWidth, outHeight);
  }
#endif

	if( mSpec ) {
	  // we have a printer device
		outWidth = static_cast<PRInt32>((mPageRect.right-mPageRect.left)*mDevUnitsToAppUnits);
		outHeight = static_cast<PRInt32>((mPageRect.bottom-mPageRect.top)*mDevUnitsToAppUnits);
	}
	else {
    // we have a screen device. find the screen that the window is on and
    // return its dimensions.
    nsCOMPtr<nsIScreen> screen;
    FindScreenForSurface ( getter_AddRefs(screen) );
    if ( screen ) {     
      PRInt32 width, height, ignored;
      screen->GetRect ( &ignored, &ignored, &width, &height );
      
      outWidth =  NSToIntRound(width * mDevUnitsToAppUnits);
      outHeight =  NSToIntRound(height * mDevUnitsToAppUnits);
 	  }
	  else {
	    NS_WARNING ( "No screen for this surface. How odd" );
	    outHeight = 0;
	    outWidth = 0;
	  }
	}
  	
	return NS_OK;	
}


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 */
NS_IMETHODIMP
nsDeviceContextMac::GetRect(nsRect &aRect)
{
	if( mSpec ) {
	  // we have a printer device
	  aRect.x = 0;
	  aRect.y = 0;
		aRect.width = static_cast<nscoord>((mPageRect.right-mPageRect.left)*mDevUnitsToAppUnits);
		aRect.height = static_cast<nscoord>((mPageRect.bottom-mPageRect.top)*mDevUnitsToAppUnits);
	}
	else {
    // we have a screen device. find the screen that the window is on and
    // return its top/left coordinates.
    nsCOMPtr<nsIScreen> screen;
    FindScreenForSurface ( getter_AddRefs(screen) );
    if ( screen ) {
      PRInt32 x, y, width, height;
      screen->GetRect ( &x, &y, &width, &height );
      
      aRect.y =  NSToIntRound(y * mDevUnitsToAppUnits);
      aRect.x =  NSToIntRound(x * mDevUnitsToAppUnits);
      aRect.width =  NSToIntRound(width * mDevUnitsToAppUnits);
      aRect.height =  NSToIntRound(height * mDevUnitsToAppUnits);
 	  }
	  else {
	    NS_WARNING ( "No screen for this surface. How odd" );
	    aRect.x = aRect.y = aRect.width = aRect.height = 0;
	  }
	}

  return NS_OK;
  
} // GetDeviceTopLeft


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 */
NS_IMETHODIMP nsDeviceContextMac::GetClientRect(nsRect &aRect)
{
	if( mSpec ) {
	  // we have a printer device
	  aRect.x = aRect.y = 0;
		aRect.width = static_cast<nscoord>((mPageRect.right-mPageRect.left)*mDevUnitsToAppUnits);
		aRect.height = static_cast<nscoord>((mPageRect.bottom-mPageRect.top)*mDevUnitsToAppUnits);
	}
	else {
    // we have a screen device. find the screen that the window is on and
    // return its dimensions.
    nsCOMPtr<nsIScreen> screen;
    FindScreenForSurface ( getter_AddRefs(screen) );
    if ( screen ) {      
      PRInt32 x, y, width, height;
      screen->GetAvailRect ( &x, &y, &width, &height );
      
      aRect.y =  NSToIntRound(y * mDevUnitsToAppUnits);
      aRect.x =  NSToIntRound(x * mDevUnitsToAppUnits);
      aRect.width =  NSToIntRound(width * mDevUnitsToAppUnits);
      aRect.height =  NSToIntRound(height * mDevUnitsToAppUnits);
	  }
	  else {
	    NS_WARNING ( "No screen for this surface. How odd" );
	    aRect.x = aRect.y = aRect.width = aRect.height = 0;
	  }
	}
  	
	return NS_OK;	
}


#pragma mark -


//------------------------------------------------------------------------

NS_IMETHODIMP nsDeviceContextMac::GetDeviceContextFor(nsIDeviceContextSpec *aDevice,nsIDeviceContext *&aContext)
{
    GrafPtr curPort;	
    double pix_Inch;
    nsDeviceContextMac *macDC;

	aContext = new nsDeviceContextMac();
  if(nsnull == aContext){
    return NS_ERROR_OUT_OF_MEMORY;
  }
  NS_ADDREF(aContext);

	macDC = (nsDeviceContextMac*)aContext;
	macDC->mSpec = aDevice;
	
	::GetPort(&curPort);

    nsCOMPtr<nsIPrintingContext> printingContext = do_QueryInterface(aDevice);
    if (printingContext) {
        if (NS_FAILED(printingContext->GetPrinterResolution(&pix_Inch)))
            pix_Inch = 72.0;
        double top, left, bottom, right;
        printingContext->GetPageRect(&top, &left, &bottom, &right);
        Rect& pageRect = macDC->mPageRect;
        pageRect.top = (PRInt16)top, pageRect.left = (PRInt16)left;
        pageRect.bottom = (PRInt16)bottom, pageRect.right = (PRInt16)right;
    }


	((nsDeviceContextMac*)aContext)->Init(curPort);

	macDC->mTwipsToPixels = pix_Inch/(float)NSIntPointsToTwips(72);
	macDC->mPixelsToTwips = 1.0f/macDC->mTwipsToPixels;
    macDC->mAppUnitsToDevUnits = macDC->mTwipsToPixels;
    macDC->mDevUnitsToAppUnits = 1.0f / macDC->mAppUnitsToDevUnits;
  
    macDC->mCPixelScale = macDC->mTwipsToPixels / mTwipsToPixels;

	//((nsDeviceContextMac*)aContext)->Init(this);
    return NS_OK;
}


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac::BeginDocument(PRUnichar * aTitle, 
                                                PRUnichar*  aPrintToFileName,
                                                PRInt32     aStartPage, 
                                                PRInt32     aEndPage)
{
    nsresult rv = NS_ERROR_FAILURE;
    nsCOMPtr<nsIPrintingContext> printingContext = do_QueryInterface(mSpec);
    if (printingContext)
        rv = printingContext->BeginDocument(aTitle, aPrintToFileName, aStartPage, aEndPage);
    return rv;
}


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac::EndDocument(void)
{
    nsresult rv = NS_ERROR_FAILURE;
    nsCOMPtr<nsIPrintingContext> printingContext = do_QueryInterface(mSpec);
    if (printingContext)
        rv = printingContext->EndDocument();
    return rv;
}

/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac::AbortDocument(void)
{
    return EndDocument();
}


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac::BeginPage(void)
{
    nsresult rv = NS_ERROR_FAILURE;
    nsCOMPtr<nsIPrintingContext> printingContext = do_QueryInterface(mSpec);
    if (printingContext)
        rv = printingContext->BeginPage();
    return rv;
}


/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
NS_IMETHODIMP nsDeviceContextMac::EndPage(void)
{
    nsresult rv = NS_ERROR_FAILURE;
    nsCOMPtr<nsIPrintingContext> printingContext = do_QueryInterface(mSpec);
    if (printingContext)
        rv = printingContext->EndPage();
    return rv;
}


#pragma mark -

//------------------------------------------------------------------------

nsHashtable* nsDeviceContextMac :: gFontInfoList = nsnull;

class FontNameKey : public nsHashKey
{
public:
  FontNameKey(const nsString& aString);

  virtual PRUint32 HashCode(void) const;
  virtual PRBool Equals(const nsHashKey *aKey) const;
  virtual nsHashKey *Clone(void) const;

  nsAutoString  mString;
};

FontNameKey::FontNameKey(const nsString& aString)
{
	mString.Assign(aString);
}

PRUint32 FontNameKey::HashCode(void) const
{
  nsString str;
  ToLowerCase(mString, str);
  return nsCRT::HashCode(str.get());
}

PRBool FontNameKey::Equals(const nsHashKey *aKey) const
{
  return mString.Equals(((FontNameKey*)aKey)->mString,
                        nsCaseInsensitiveStringComparator());
}

nsHashKey* FontNameKey::Clone(void) const
{
  return new FontNameKey(mString);
}

#pragma mark -

/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
void nsDeviceContextMac :: InitFontInfoList()
{

	OSStatus err;
	if (!gFontInfoList) 
	{
		gFontInfoList = new nsHashtable();
		if (!gFontInfoList)
			return;

        // use the new Font Manager enumeration API.
        ATSFontFamilyIterator iter;
        err = ::ATSFontFamilyIteratorCreate(kATSFontContextLocal,
			NULL, NULL, // filter and its refcon
			kATSOptionFlagsDefaultScope,
			&iter);
        if (err != noErr)
            return;
        
		TextEncoding unicodeEncoding = ::CreateTextEncoding(kTextEncodingUnicodeDefault, 
															kTextEncodingDefaultVariant,
													 		kUnicodeUTF8Format);
        // enumerate all fonts.
        TECObjectRef converter = NULL;
        TextEncoding oldFontEncoding = 0;
        ATSFontFamilyRef fontFamily;
        while (::ATSFontFamilyIteratorNext(iter, &fontFamily) == noErr) {
		    // we'd like to use ATSFontFamilyGetName here, but it's ignorant of the
        // font encodings, resulting in garbage names for non-western fonts.
            Str255 fontName;
            err = ::ATSFontFamilyGetQuickDrawName(fontFamily, fontName);
            if (err != noErr || fontName[0] == 0 || fontName[1] == '.' || fontName[1] == '%')
                continue;
            TextEncoding fontEncoding;
            fontEncoding = ::ATSFontFamilyGetEncoding(fontFamily);
            if (oldFontEncoding != fontEncoding) {
                oldFontEncoding = fontEncoding;
                if (converter)
                    err = ::TECDisposeConverter(converter);
                err = ::TECCreateConverter(&converter, fontEncoding, unicodeEncoding);
                if (err != noErr)
                    continue;
            }
            // convert font name to UNICODE.
			char unicodeFontName[sizeof(fontName)];
			ByteCount actualInputLength, actualOutputLength;
			err = ::TECConvertText(converter, &fontName[1], fontName[0], &actualInputLength, 
										(TextPtr)unicodeFontName , sizeof(unicodeFontName), &actualOutputLength);	
			unicodeFontName[actualOutputLength] = '\0';

			nsString temp = NS_ConvertUTF8toUTF16(nsDependentCString(unicodeFontName));
    		FontNameKey key(temp);
			gFontInfoList->Put(&key, (void*)::FMGetFontFamilyFromATSFontFamilyRef(fontFamily));
        }
        if (converter)
            err = ::TECDisposeConverter(converter);
        err = ::ATSFontFamilyIteratorRelease(&iter);
	}
}



/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
bool nsDeviceContextMac :: GetMacFontNumber(const nsString& aFontName, short &aFontNum)
{
	//�TODO?: Maybe we shouldn't call that function so often. If nsFont could store the
	//				fontNum, nsFontMetricsMac::SetFont() wouldn't need to call this at all.
	InitFontInfoList();
    FontNameKey key(aFontName);
	aFontNum = (short) NS_PTR_TO_INT32(gFontInfoList->Get(&key));
	return (aFontNum != 0);
}


//------------------------------------------------------------------------
// Override to tweak font settings
nsresult nsDeviceContextMac::CreateFontAliasTable()
{
  nsresult result = NS_OK;

  if (nsnull == mFontAliasTable) {
    mFontAliasTable = new nsHashtable();
    if (nsnull != mFontAliasTable)
    {
			nsAutoString  fontTimes;              fontTimes.AssignLiteral("Times");
			nsAutoString  fontTimesNewRoman;      fontTimesNewRoman.AssignLiteral("Times New Roman");
			nsAutoString  fontTimesRoman;         fontTimesRoman.AssignLiteral("Times Roman");
			nsAutoString  fontArial;              fontArial.AssignLiteral("Arial");
			nsAutoString  fontHelvetica;          fontHelvetica.AssignLiteral("Helvetica");
			nsAutoString  fontCourier;            fontCourier.AssignLiteral("Courier");
			nsAutoString  fontCourierNew;         fontCourierNew.AssignLiteral("Courier New");
			nsAutoString  fontUnicode;            fontUnicode.AssignLiteral("Unicode");
			nsAutoString  fontBitstreamCyberbit;  fontBitstreamCyberbit.AssignLiteral("Bitstream Cyberbit");
			nsAutoString  fontNullStr;

      AliasFont(fontTimes, fontTimesNewRoman, fontTimesRoman, PR_FALSE);
      AliasFont(fontTimesRoman, fontTimesNewRoman, fontTimes, PR_FALSE);
      AliasFont(fontTimesNewRoman, fontTimesRoman, fontTimes, PR_FALSE);
      AliasFont(fontArial, fontHelvetica, fontNullStr, PR_FALSE);
      AliasFont(fontHelvetica, fontArial, fontNullStr, PR_FALSE);
      AliasFont(fontCourier, fontCourierNew, fontNullStr, PR_FALSE);		// changed from DeviceContextImpl
      AliasFont(fontCourierNew, fontCourier, fontNullStr, PR_FALSE);
      AliasFont(fontUnicode, fontBitstreamCyberbit, fontNullStr, PR_FALSE); // XXX ????
    }
    else {
      result = NS_ERROR_OUT_OF_MEMORY;
    }
  }
  return result;
}

#pragma mark -

/** ---------------------------------------------------
 *  See documentation in nsIDeviceContext.h
 *	@update 12/9/98 dwc
 */
PRUint32 nsDeviceContextMac::GetScreenResolution()
{
	static PRBool initialized = PR_FALSE;
	if (initialized)
		return mPixelsPerInch;
	initialized = PR_TRUE;

    nsresult rv;
    nsCOMPtr<nsIPref> prefs(do_GetService(NS_PREF_CONTRACTID, &rv));
    if (NS_SUCCEEDED(rv) && prefs) {
		PRInt32 intVal;
		if (NS_SUCCEEDED(prefs->GetIntPref("layout.css.dpi", &intVal)) && intVal > 0) {
			mPixelsPerInch = intVal;
		}
#if 0
// the code here will ignore the default setting of 96dpi and
// instead force approximately 84dpi. There's no real reason for this
// and we shipped Camino0.7 with 96dpi and got no complaints. As
// a result, I'm removing it, but leaving the code for posterity.
		else {
			short hppi, vppi;
			::ScreenRes(&hppi, &vppi);
			mPixelsPerInch = hppi * 1.17f;
		}
#endif
	}

	return mPixelsPerInch;
}


#pragma mark -
//------------------------------------------------------------------------
nsFontEnumeratorMac::nsFontEnumeratorMac()
{
}

NS_IMPL_ISUPPORTS1(nsFontEnumeratorMac, nsIFontEnumerator)

typedef struct EnumerateFamilyInfo
{
  PRUnichar** mArray;
  int         mIndex;
} EnumerateFamilyInfo;

typedef struct EnumerateFontInfo
{
  PRUnichar** mArray;
  int         mIndex;
  int         mCount;
  ScriptCode	mScript;
  nsGenericFontNameType mType;
} EnumerateFontInfo;



static int
CompareFontNames(const void* aArg1, const void* aArg2, void* aClosure)
{
  const PRUnichar* str1 = *((const PRUnichar**) aArg1);
  const PRUnichar* str2 = *((const PRUnichar**) aArg2);

  // XXX add nsICollation stuff

  return nsCRT::strcmp(str1, str2);
}
static PRBool
EnumerateFamily(nsHashKey *aKey, void *aData, void* closure)

{
  EnumerateFamilyInfo* info = (EnumerateFamilyInfo*) closure;
  PRUnichar** array = info->mArray;
  int j = info->mIndex;
  
  
  PRUnichar* str = ToNewUnicode(((FontNameKey*)aKey)->mString);
  if (!str) {
    for (j = j - 1; j >= 0; j--) {
      nsMemory::Free(array[j]);
    }
    info->mIndex = 0;
    return PR_FALSE;
  }
  array[j] = str;
  info->mIndex++;

  return PR_TRUE;
}

NS_IMETHODIMP
nsFontEnumeratorMac::EnumerateAllFonts(PRUint32* aCount, PRUnichar*** aResult)
{
  if (aCount) {
    *aCount = 0;
  }
  else {
    return NS_ERROR_NULL_POINTER;
  }
  if (aResult) {
    *aResult = nsnull;
  }
  else {
    return NS_ERROR_NULL_POINTER;
  }

	nsDeviceContextMac::InitFontInfoList();
	nsHashtable* list = nsDeviceContextMac::gFontInfoList;
	if(!list) {
		return NS_ERROR_FAILURE;
	}
	PRInt32 items = list->Count();
  PRUnichar** array = (PRUnichar**)
    nsMemory::Alloc(items * sizeof(PRUnichar*));
  if (!array) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  EnumerateFamilyInfo info = { array, 0 };
  list->Enumerate ( EnumerateFamily, &info);
  NS_ASSERTION( items == info.mIndex, "didn't get all the fonts");
  if (!info.mIndex) {
    nsMemory::Free(array);
    return NS_ERROR_OUT_OF_MEMORY;
  }

  NS_QuickSort(array, info.mIndex, sizeof(PRUnichar*),
    CompareFontNames, nsnull);

  *aCount = info.mIndex;
  *aResult = array;

  return NS_OK;
}

static PRBool
EnumerateFont(nsHashKey *aKey, void *aData, void* closure)

{
  EnumerateFontInfo* info = (EnumerateFontInfo*) closure;
  PRUnichar** array = info->mArray;
  int j = info->mCount;
  PRBool match = PR_FALSE;

  // we need to match the cast of FMFontFamily in nsDeviceContextMac :: InitFontInfoList()
  FMFontFamily fontFamily = (FMFontFamily) NS_PTR_TO_INT32(aData);
  TextEncoding fontEncoding;
  OSStatus status = ::FMGetFontFamilyTextEncoding(fontFamily, &fontEncoding);
  if (noErr == status) {
    ScriptCode script;
    status = ::RevertTextEncodingToScriptInfo(fontEncoding, &script, nsnull, nsnull);
    match = ((noErr == status) && (script == info->mScript));
  }

  if (match) {
	  PRUnichar* str = ToNewUnicode(((FontNameKey*)aKey)->mString);
	  if (!str) {
	    for (j = j - 1; j >= 0; j--) {
	      nsMemory::Free(array[j]);
	    }
	    info->mIndex = 0;
	    return PR_FALSE;
	  }
	  array[j] = str;
	  info->mCount++;
	}
	info->mIndex++;
  return PR_TRUE;
}
NS_IMETHODIMP
nsFontEnumeratorMac::EnumerateFonts(const char* aLangGroup,
  const char* aGeneric, PRUint32* aCount, PRUnichar*** aResult)
{
  if ((! aLangGroup) ||( !aGeneric ))
  	return NS_ERROR_NULL_POINTER;
  	
  if (aCount) {
    *aCount = 0;
  }
  else {
    return NS_ERROR_NULL_POINTER;
  }
  if (aResult) {
    *aResult = nsnull;
  }
  else {
    return NS_ERROR_NULL_POINTER;
  }

  if ((!strcmp(aLangGroup, "x-unicode")) ||
      (!strcmp(aLangGroup, "x-user-def"))) {
    return EnumerateAllFonts(aCount, aResult);
  }

	nsDeviceContextMac::InitFontInfoList();
	nsHashtable* list = nsDeviceContextMac::gFontInfoList;
	if(!list) {
		return NS_ERROR_FAILURE;
	}
	PRInt32 items = list->Count();
  PRUnichar** array = (PRUnichar**)
    nsMemory::Alloc(items * sizeof(PRUnichar*));
  if (!array) {
    return NS_ERROR_OUT_OF_MEMORY;
  }
  nsUnicodeMappingUtil* gUtil = nsUnicodeMappingUtil::GetSingleton();
	if(!gUtil) {
		return NS_ERROR_FAILURE;
	}
  
  nsAutoString GenName; GenName.AssignWithConversion(aGeneric);
  EnumerateFontInfo info = { array, 0 , 0, gUtil->MapLangGroupToScriptCode(aLangGroup) ,gUtil->MapGenericFontNameType(GenName) };
  list->Enumerate ( EnumerateFont, &info);
  if (!info.mIndex) {
    nsMemory::Free(array);
    return NS_ERROR_OUT_OF_MEMORY;
  }

  NS_QuickSort(array, info.mCount, sizeof(PRUnichar*),
    CompareFontNames, nsnull);

  *aCount = info.mCount;
  *aResult = array;

  return NS_OK;
}
NS_IMETHODIMP
nsFontEnumeratorMac::HaveFontFor(const char* aLangGroup,PRBool* aResult)
{
  NS_ENSURE_ARG_POINTER(aLangGroup);
  NS_ENSURE_ARG_POINTER(aResult);
  *aResult = PR_FALSE;
  PRUint32 count;
  PRUnichar **ptr;
  nsresult res = EnumerateFonts(aLangGroup, "", &count, &ptr);
  NS_ENSURE_SUCCESS(res, res);
  *aResult = (count > 0);
  PRUint32 i;
  for(i = 0 ; i < count; i++)
  	nsMemory::Free(ptr[i]);
  nsMemory::Free(ptr);
  return NS_OK;
}

NS_IMETHODIMP
nsFontEnumeratorMac::GetDefaultFont(const char *aLangGroup, 
  const char *aGeneric, PRUnichar **aResult)
{
  // aLangGroup=null or ""  means any (i.e., don't care)
  // aGeneric=null or ""  means any (i.e, don't care)

  NS_ENSURE_ARG_POINTER(aResult);
  *aResult = nsnull;

  return NS_OK;
}

NS_IMETHODIMP
nsFontEnumeratorMac::UpdateFontList(PRBool *updateFontList)
{
  *updateFontList = PR_FALSE; // always return false for now
  return NS_OK;
}
