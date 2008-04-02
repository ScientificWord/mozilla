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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Josh Aas <josh@mozilla.com>
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


#include "npapi.h"

#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>

#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define PLUGINFINDER_COMMAND_BEGINNING "javascript:window.open(\""
#define PLUGINFINDER_COMMAND_END "\",\"plugin\",\"toolbar=no,status=no,resizable=no,scrollbars=no,height=252,width=626\");"
#define PLUGINFINDER_COMMAND_END2 "\",\"plugin\",\"toolbar=no,status=no,resizable=yes,scrollbars=yes,height=252,width=626\");"


// Instance state information about the plugin.
class CPlugin
{
public:
  enum HiliteState { kUnhilited = 0, kHilited = 1 };  

  static NPError Initialize();
  static void Shutdown();

  // no ctor because CPlugin is allocated and constructed by hand.
  // ideally, this should use placement |new|.

  void Constructor(NPP instance, NPMIMEType type, uint16 mode, int16 argc, char* argn[], char* argv[]);
  void Destructor();

  void SetWindow(NPWindow* window);
  void Print(NPPrint* printInfo);
  Boolean HandleEvent(EventRecord*);

protected:
  void Draw(HiliteState hilite);
  void DrawString(const unsigned char* text, short width, short height, short centerX, Rect drawRect);
  void MouseDown();

  Boolean FocusDraw();
  void RestoreDraw();
      
  void DetermineURL(int16 argc, char* argn[], char* argv[]);
  char* MakeDefaultURL(void);
  void AddMimeTypeToList(StringPtr cTypeString);
  Boolean CheckMimeTypes();
  void AskAndLoadURL();
  void RefreshPluginPage();
      
  Ptr New(UInt32 size);
  void Delete(Ptr ptr);

  Boolean IsPluginHidden(int16 argc, char* argn[], char* argv[]);

private:
  static CIconHandle sIconHandle;
  static CursHandle sHandCursor;
  static char* sAltText;
  static char* sInstallCommand;
  static char* sDefaultPage;
  static char* sRefreshText;
  static char* sJavaScriptPage;

  NPP fInstance;
  NPWindow* fWindow;
  uint16 fMode;
  NPMIMEType fType;
  char* fPageURL;
  char* fFileURL;
  NPBool m_bOffline;
  NPBool m_bJavaScript;

  GrafPtr fSavePort;
  RgnHandle fSaveClip;
  Rect fRevealedRect;
  short fSavePortTop;
  short fSavePortLeft;
  Boolean fUserInstalledPlugin;
  Boolean fHiddenPlugin;
  Boolean fAskedLoadURL;
};

CIconHandle CPlugin::sIconHandle = NULL;
CursHandle CPlugin::sHandCursor = NULL;
char* CPlugin::sAltText = NULL;
char* CPlugin::sInstallCommand = NULL; 
char* CPlugin::sDefaultPage = NULL;
char* CPlugin::sRefreshText = NULL;
char* CPlugin::sJavaScriptPage = NULL;

extern short gResFile;

// 'cicn'
const short rBrokenPluginIcon = 326;

// 'CURS'
const short rHandCursor = 128;

// 'STR '
const short rDefaultPluginURL = 128;
const short rAltTextString = 129;
const short rJavaScriptInstallCommand = 130;
const short rRefreshTextString = 131;
const short rJavaScriptPageURL = 132;

// 'STR#'
const short rTypeListStrings = 129;

static const char szPluginFinderCommandBeginning[] = PLUGINFINDER_COMMAND_BEGINNING;
static const char szPluginFinderCommandEnd[] = PLUGINFINDER_COMMAND_END;


NPError NPP_Initialize(void)
{
  return CPlugin::Initialize();
}


void NPP_Shutdown(void)
{
  CPlugin::Shutdown();
}


NPError NPP_New(NPMIMEType type, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData*)
{
  if (!instance)
    return NPERR_INVALID_INSTANCE_ERROR;
    
  CPlugin* This = (CPlugin*) (char*)NPN_MemAlloc(sizeof(CPlugin));
  instance->pdata = This;
  if (This) {
    This->Constructor(instance, type, mode, argc, argn, argv);
    return NPERR_NO_ERROR;
  }
  else {
    return NPERR_OUT_OF_MEMORY_ERROR;
  }
}


NPError NP_LOADDS
NPP_Destroy(NPP instance, NPSavedData** /*save*/)
{
  if (!instance)
    return NPERR_INVALID_INSTANCE_ERROR;

  CPlugin* This = (CPlugin*) instance->pdata;
  if (This) {
    This->Destructor();
    NPN_MemFree(This);
    instance->pdata = NULL;
  }

  return NPERR_NO_ERROR;
}


NPError NPP_SetWindow(NPP instance, NPWindow* window)
{
  if (!instance)
    return NPERR_INVALID_INSTANCE_ERROR;

  CPlugin* This = (CPlugin*) instance->pdata;
  if (This)
    This->SetWindow(window);

  return NPERR_NO_ERROR;
}


NPError NP_LOADDS
NPP_NewStream(NPP instance,
              NPMIMEType /*type*/,
              NPStream* /*stream*/, 
              NPBool /*seekable*/,
              uint16* /*stype*/)
{
  if (!instance)
    return NPERR_INVALID_INSTANCE_ERROR;

  return NPERR_NO_ERROR;
}


int32 STREAMBUFSIZE = 0X0FFFFFFF;   // If we are reading from a file in NPAsFile
                                    // mode so we can take any size stream in our
                                    // write call (since we ignore it)


int32 NP_LOADDS
NPP_WriteReady(NPP /*instance*/, NPStream* /*stream*/)
{
  return STREAMBUFSIZE; // Number of bytes ready to accept in NPP_Write()
}


int32 NP_LOADDS
NPP_Write(NPP /*instance*/, NPStream* /*stream*/, int32 /*offset*/, int32 len, void* /*buffer*/)
{
  return len; // The number of bytes accepted
}


NPError NP_LOADDS
NPP_DestroyStream(NPP instance, NPStream* /*stream*/, NPError /*reason*/)
{
  if (!instance)
    return NPERR_INVALID_INSTANCE_ERROR;

  return NPERR_NO_ERROR;
}


void NP_LOADDS
NPP_StreamAsFile(NPP /*instance*/, NPStream */*stream*/, const char* /*fname*/)
{
}


void NP_LOADDS
NPP_Print(NPP instance, NPPrint* printInfo)
{
  if (!printInfo)
    return;

  if (instance) {
    if (printInfo->mode == NP_FULL) {
      printInfo->print.fullPrint.pluginPrinted = FALSE; // Do the default
    }
    else { // If not fullscreen, we must be embedded
      CPlugin* This = (CPlugin*)instance->pdata;
      if (This)
        This->Print(printInfo);
    }
  }
}


// Mac-only
int16 NPP_HandleEvent(NPP instance, void* event)
{
  if (instance) {
    CPlugin* This = (CPlugin*) instance->pdata;
    if (This && event)
      return This->HandleEvent((EventRecord*) event);
  }

  return FALSE;
}


void NPP_URLNotify(NPP /*instance*/, const char* /*url*/, NPReason /*reason*/, void* /*notifyData*/)
{
}


#ifdef OJI
jref NPP_GetJavaClass(void)
{
  return NULL;
}
#endif /* OJI */


NPError CPlugin::Initialize()
{
  Handle string;
  short saveResFile = CurResFile();

  UseResFile(gResFile);

  // Get Resources
  CPlugin::sIconHandle = GetCIcon(rBrokenPluginIcon);
  CPlugin::sHandCursor = GetCursor(rHandCursor);

  // Get "alt text" string
  string = Get1Resource('STR ', rAltTextString);
  if (string && *string) {
    short stringLen = (*string)[0];
    CPlugin::sAltText = (char*)NPN_MemAlloc(stringLen +1);
    if (CPlugin::sAltText) {
      short src = 1;
      short dest = 0;
      while (src <= stringLen)
        CPlugin::sAltText[dest++] = (*string)[src++];
      CPlugin::sAltText[dest++] = 0;
    }
  }
  ReleaseResource(string);

  // Get "refresh text" string
  string = Get1Resource('STR ', rRefreshTextString);
  if (string && *string) {
    short stringLen = (*string)[0];
    CPlugin::sRefreshText = (char*)NPN_MemAlloc(stringLen + 1);
    if (CPlugin::sRefreshText) {
      short src = 1;
      short dest = 0;
      while (src <= stringLen)
        CPlugin::sRefreshText[dest++] = (*string)[src++];
      CPlugin::sRefreshText[dest++] = 0;
    }
  }
  ReleaseResource(string);

  // Get JavaScript install command string
  string = Get1Resource('STR ', rJavaScriptInstallCommand);
  if (string && *string) {
    short stringLen = (*string)[0];
    CPlugin::sInstallCommand = (char*)NPN_MemAlloc(stringLen + 1);
    if (CPlugin::sInstallCommand) {
      short src = 1;
      short dest = 0;
      while (src <= stringLen)
        CPlugin::sInstallCommand[dest++] = (*string)[src++];
      CPlugin::sInstallCommand[dest++] = 0;
    }
  }
  ReleaseResource(string);

  // Get default plug-in page URL
  string = Get1Resource('STR ', rDefaultPluginURL);
  if (string && *string) {
    short stringLen = (*string)[0];
    CPlugin::sDefaultPage = (char*)NPN_MemAlloc(stringLen + 1);
    if (CPlugin::sDefaultPage) {
      short src = 1;
      short dest = 0;
      while (src <= stringLen)
        CPlugin::sDefaultPage[dest++] = (*string)[src++];
      CPlugin::sDefaultPage[dest++] = 0;
    }
  }
  ReleaseResource(string);

  // Get javascript plug-in page URL
  string = Get1Resource('STR ', rJavaScriptPageURL);
  if (string && *string) {
    short stringLen = (*string)[0];
    CPlugin::sJavaScriptPage = (char*)NPN_MemAlloc(stringLen + 1);
    if (CPlugin::sJavaScriptPage) {
      short src = 1;
      short dest = 0;
      while (src <= stringLen)
        CPlugin::sJavaScriptPage[dest++] = (*string)[src++];
      CPlugin::sJavaScriptPage[dest++] = 0;
    }
  }
  ReleaseResource(string);

  UseResFile(saveResFile);

  return NPERR_NO_ERROR;
}


void CPlugin::Shutdown()
{
  if (CPlugin::sIconHandle)
    ::ReleaseResource((Handle) CPlugin::sIconHandle);
  if (CPlugin::sHandCursor)
    ::ReleaseResource((Handle) CPlugin::sHandCursor);

  if (CPlugin::sAltText)
    NPN_MemFree(CPlugin::sAltText);
  if (CPlugin::sInstallCommand)
    NPN_MemFree(CPlugin::sInstallCommand);
  if (CPlugin::sDefaultPage)
    NPN_MemFree(CPlugin::sDefaultPage);
  if (CPlugin::sRefreshText)
    NPN_MemFree(CPlugin::sRefreshText);
}


void CPlugin::Constructor(NPP instance, NPMIMEType type, uint16 mode, int16 argc, char* argn[], char* argv[])
{
  fWindow = NULL;
  fPageURL = NULL;
  fFileURL = NULL;
  fInstance = instance;
  fMode = mode; // Mode is NP_EMBED, NP_FULL, or NP_BACKGROUND (see npapi.h)
  fAskedLoadURL = false;
  fUserInstalledPlugin = false;
  
  // Save a copy of our mime type string
  short typeLength = strlen(type);
  fType = (char*)NPN_MemAlloc(typeLength+1);
  if (fType)
    strcpy(fType, type);
  
  // Make a handy region for use in FocusDraw
  fSaveClip = NewRgn();

  // determine if the plugin is specified as HIDDEN
  fHiddenPlugin = IsPluginHidden(argc, argn, argv);

  // Get some information about our environment
  NPN_GetValue(fInstance, NPNVisOfflineBool, (void *)&m_bOffline);
  NPN_GetValue(fInstance, NPNVjavascriptEnabledBool, (void *)&m_bJavaScript);

  // Figure out what URL we will go to
  DetermineURL(argc, argn, argv);
}


void CPlugin::Destructor()
{
  if (fSaveClip)
    DisposeRgn(fSaveClip);
  
  if (fType)
    NPN_MemFree(fType);
  if (fFileURL)
    NPN_MemFree(fFileURL);
  if (fPageURL)
    NPN_MemFree(fPageURL);
}    


void CPlugin::SetWindow(NPWindow* window)
{
  fWindow = window;
}


// To print, we need to retrieve the printing window from the printInfo,
// temporarily make it our window, draw into it, and restore the window.
void CPlugin::Print(NPPrint* printInfo)
{
  NPWindow* printWindow = &(printInfo->print.embedPrint.window);

  NPWindow* oldWindow = fWindow;
  fWindow = printWindow;

  if (FocusDraw()) {
    Draw(kUnhilited);
    RestoreDraw();
  }

  fWindow = oldWindow;
}


Boolean CPlugin::HandleEvent(EventRecord* ev)
{
  Boolean eventHandled = false;
  
  switch (ev->what)
  {
    case mouseDown:
      MouseDown();
      eventHandled = true;
      break;

    case updateEvt:
      if (FocusDraw()) {
        Draw(kUnhilited);
        RestoreDraw();
      }
      eventHandled = true;
      break;

    case NPEventType_AdjustCursorEvent:
      if (CPlugin::sHandCursor)
        SetCursor(*CPlugin::sHandCursor);
      if (fUserInstalledPlugin) {
        if (CPlugin::sRefreshText)
          NPN_Status(fInstance, CPlugin::sRefreshText);        
      }
      else {
        if (CPlugin::sAltText)
          NPN_Status(fInstance, CPlugin::sAltText);
      }
      eventHandled = true;
      break;

    case nullEvent:
      // NOTE: We have to wait until idle time
      // to ask the user if they want to visit
      // the URL to avoid reentering XP code.
      if (!fAskedLoadURL) {
        if (CheckMimeTypes())
          AskAndLoadURL();
        fAskedLoadURL = true;
      }
      break;
    default:
      break;
  }

  return eventHandled;
}


void CPlugin::Draw(HiliteState hilite)
{
  UInt8 *pTheText;
  SInt32 height = fWindow->height;
  SInt32 width = fWindow->width;
  SInt32 centerX = (width) >> 1;
  SInt32 centerY = (height) >> 1;
  Rect drawRect;
  RGBColor black = { 0x0000, 0x0000, 0x0000 };
  RGBColor white = { 0xFFFF, 0xFFFF, 0xFFFF };
  RGBColor hiliteColor = { 0x0000, 0x0000, 0x0000 };
  short transform;

  drawRect.top = 0;
  drawRect.left = 0;
  drawRect.bottom = height;
  drawRect.right = width;

  if (height < 4 && width < 4)
    return;

  PenNormal();
  RGBForeColor(&black);
  RGBBackColor(&white);

  Pattern qdWhite;
  FillRect(&drawRect, GetQDGlobalsWhite(&qdWhite));

  if (hilite == kHilited) {
    hiliteColor.red = 0xFFFF;
    transform = ttSelected;
  }
  else {
    hiliteColor.blue = 0xFFFF;
    transform = ttNone;
  }

  RGBForeColor(&hiliteColor);
  FrameRect(&drawRect);

  if (height > 32 && width > 32 && CPlugin::sIconHandle) {
    drawRect.top = centerY - 16;
    drawRect.bottom = centerY + 16;
    drawRect.left = centerX - 16;
    drawRect.right = centerX + 16;
    PlotCIconHandle(&drawRect, atAbsoluteCenter, transform, CPlugin::sIconHandle);
  }

  if (fUserInstalledPlugin) {
    pTheText = (unsigned char*)CPlugin::sRefreshText;
  }
  else {
    pTheText = (unsigned char*)CPlugin::sAltText;
  }
  DrawString(pTheText, width, height, centerX, drawRect);
}


// Track the click in our plugin by drawing the icon enabled or disabled
// as the user moves the mouse in and out with the button held down.  If
// they let up the mouse while still inside, get the URL.
void  CPlugin::MouseDown()
{
  if (FocusDraw()) {
    Draw(kHilited);
    Boolean inside = true;
  
    // evil CPU-hogging loop on Mac OS X!
    while (StillDown()) {
      Point localMouse;
      GetMouse(&localMouse);
      Boolean insideNow = ::PtInRect(localMouse, &fRevealedRect);

      if (insideNow != inside) {
        Draw(insideNow ? kHilited : kUnhilited);
        inside = insideNow;
      }
    }
    
    if (inside) {
      Draw(kUnhilited);
      if (!fUserInstalledPlugin)
        AskAndLoadURL();
      else
        RefreshPluginPage();
    }

    RestoreDraw();
  }
}


Boolean CPlugin::FocusDraw()
{
  if (!fWindow)
    return false;
    
  NP_Port* npport = (NP_Port*) fWindow->window;
  CGrafPtr ourPort = npport->port;
  
  if (fWindow->clipRect.left < fWindow->clipRect.right) {
    GetPort(&fSavePort);
    SetPort((GrafPtr) ourPort);
    Rect portRect;
    GetPortBounds(ourPort, &portRect);
    fSavePortTop = portRect.top;
    fSavePortLeft = portRect.left;
    GetClip(fSaveClip);
    
    fRevealedRect.top = fWindow->clipRect.top + npport->porty;
    fRevealedRect.left = fWindow->clipRect.left + npport->portx;
    fRevealedRect.bottom = fWindow->clipRect.bottom + npport->porty;
    fRevealedRect.right = fWindow->clipRect.right + npport->portx;
    SetOrigin(npport->portx, npport->porty);
    ClipRect(&fRevealedRect);

    return true;
  }
  else {
    return false;
  }
}


void CPlugin::RestoreDraw()
{
  SetOrigin(fSavePortLeft, fSavePortTop);
  SetClip(fSaveClip);
  SetPort(fSavePort);
}


// Get a URL from either the parameters passed from the EMBED.
// Append "?" and our mime type and save for later use.
void CPlugin::DetermineURL(int16 argc, char* argn[], char* argv[])
{
  char* url;
  SInt32 additionalLength = 0;
  SInt32 i;

  // Appended to the URL will be a "?" and the mime type of this instance.  This lets the server
  // do something intelligent with a CGI script.

  if (fType)
    additionalLength += (strlen(fType) + 1); // Add 1 for '?'

  // The page designer can specify a URL where the plugin for this type can be downloaded.  Here
  // we scan the arguments for this attribute and save it away if we
  // find it for later use by LoadPluginURL().
  for (i = 0; i < argc; i++) {
    if ((strcasecmp(argn[i], "PLUGINSPAGE") == 0) || (strcasecmp(argn[i], "CODEBASE") == 0)) {
      url = argv[i];
      fPageURL = (char*)NPN_MemAlloc(strlen(url) + 1 + additionalLength);  // Add 1 for '\0'
      if (fPageURL) {
        if (additionalLength > 0) {
          sprintf(fPageURL, "%s?%s", url, fType);
        }
        else {
          strcpy(fPageURL, url);  
        }
      }
      break;
    } else if ((strcasecmp(argn[i], "PLUGINURL") == 0) || (strcasecmp(argn[i], "CLASSID") == 0)) {
      url = argv[i];
      if (CPlugin::sInstallCommand) {
        // Allocate a new string
        fFileURL = (char*)NPN_MemAlloc(strlen(CPlugin::sInstallCommand) + 1 + strlen(url));  
        if (fFileURL)
          sprintf(fFileURL, CPlugin::sInstallCommand, url);
      }
      break;
    }
  }
}



// Get a URL from our resources.  Append "?" and our mime type and save for later use.
char *CPlugin::MakeDefaultURL(void)
{
  char *pDefURL = NULL;
  SInt32 additionalLength = 0;

  // Appended to the URL will be a "?" and the mime type of this instance.  This lets the server
  // do something intelligent with a CGI script.

  if (fType)
    additionalLength += (strlen(fType) + 1);    // Add 1 for '?'

  if (!m_bJavaScript) {
    if (CPlugin::sDefaultPage) {
      pDefURL = (char*)NPN_MemAlloc(strlen(CPlugin::sDefaultPage) + 1 + additionalLength);
      if (pDefURL) {
        if (additionalLength > 0) {
          sprintf(pDefURL, "%s?%s", CPlugin::sDefaultPage, fType);
        }
        else {
          strcpy(pDefURL, CPlugin::sDefaultPage);  
        }
      }
    }
  }
  else {
    if (CPlugin::sJavaScriptPage) {
      pDefURL = (char*)NPN_MemAlloc(strlen(szPluginFinderCommandBeginning) +
            strlen(CPlugin::sJavaScriptPage) +
            additionalLength + strlen(szPluginFinderCommandEnd) + 1);
      if (pDefURL) {
        sprintf(pDefURL, "%s%s%s%s", szPluginFinderCommandBeginning,
            CPlugin::sJavaScriptPage, fType, szPluginFinderCommandEnd);
      }
    }
  }
  return(pDefURL);
}



// Check the mime type of this instance against our list
// of types we�ve seen before.  If we find our type in the
// list, return false; otherwise, return true.
//
// type 'STR#' {
//      integer = $$Countof(StringArray);
//      array StringArray {
//              pstring;
//      };
void CPlugin::AddMimeTypeToList(StringPtr cTypeString)
{
  CFStringRef pluginKey = CFSTR("DefaultPluginSeenTypes"); // don't release this
  CFStringRef mimeType = ::CFStringCreateWithPascalString(kCFAllocatorDefault, cTypeString, kCFStringEncodingASCII);
  CFArrayRef prefsList = (CFArrayRef)::CFPreferencesCopyAppValue(pluginKey, kCFPreferencesCurrentApplication);
  Boolean foundType = false;

  if (!prefsList) {
    CFStringRef stringArray[1];
    stringArray[0] = mimeType;

    prefsList = ::CFArrayCreate(kCFAllocatorDefault, (const void **)stringArray, 1, &kCFTypeArrayCallBacks);
    if (prefsList) {
      ::CFPreferencesSetAppValue(pluginKey, prefsList, kCFPreferencesCurrentApplication);
      ::CFRelease(prefsList);
    }
  }
  else {
    if (::CFGetTypeID(prefsList) == ::CFArrayGetTypeID()) {
      // first make sure it's not in the list
      CFIndex count = ::CFArrayGetCount(prefsList);
      for (CFIndex i = 0; i < count; i ++) {
        CFStringRef item = (CFStringRef)::CFArrayGetValueAtIndex(prefsList, i); // does not retain
        if (item && (::CFGetTypeID(item) == ::CFStringGetTypeID()) &&
            (::CFStringCompareWithOptions(item, mimeType,
                                          CFRangeMake(0, ::CFStringGetLength(item)), kCFCompareCaseInsensitive) == kCFCompareEqualTo)) {
          foundType = true;
          break;
        }
      }

      if (!foundType && !fHiddenPlugin) {
        CFMutableArrayRef typesArray = ::CFArrayCreateMutableCopy(kCFAllocatorDefault, 0, (CFArrayRef)prefsList);
        if (typesArray) {
          ::CFArrayAppendValue(typesArray, mimeType);
          ::CFPreferencesSetAppValue(pluginKey, typesArray, kCFPreferencesCurrentApplication);
        }
      }
    }
    ::CFRelease(prefsList);
  }
  ::CFRelease(mimeType);
}


// Check the mime type of this instance against our list
// of types we�ve seen before.  If we find our type in the
// list, return false; otherwise, return true.
//
// type 'STR#' {
//      integer = $$Countof(StringArray);
//      array StringArray {
//              pstring;
//      };
Boolean CPlugin::CheckMimeTypes()
{
  Boolean failedToFind = true;

  CFStringRef pluginKey = CFSTR("DefaultPluginSeenTypes"); // don't release this
  CFStringRef mimeType  = ::CFStringCreateWithCString(kCFAllocatorDefault, fType, kCFStringEncodingASCII);
  CFArrayRef prefsList = (CFArrayRef)::CFPreferencesCopyAppValue(pluginKey, kCFPreferencesCurrentApplication);
  if (prefsList) {
    if (::CFGetTypeID(prefsList) == ::CFArrayGetTypeID()) {
      CFIndex count = ::CFArrayGetCount(prefsList);
      for (CFIndex i = 0; i < count; i ++) {
        CFStringRef item = (CFStringRef)::CFArrayGetValueAtIndex(prefsList, i); // does not retain
        if (item &&
            (::CFGetTypeID(item) == ::CFStringGetTypeID()) &&
            (::CFStringCompareWithOptions(item, mimeType,
                                          CFRangeMake(0, ::CFStringGetLength(item)), kCFCompareCaseInsensitive) == kCFCompareEqualTo)) {
          failedToFind = false;
          break;
        }
      }
    }
    ::CFRelease(prefsList);
  }
  ::CFRelease(mimeType);
  return failedToFind;
}


void CPlugin::AskAndLoadURL()
{
  char *pTheURL;
  SInt32 dwLen;
  Str255 ourType;

  if (!m_bOffline) {
    // Convert the mime-type C string to a Pascal string.
    dwLen = strlen(fType);
    if (dwLen > 255) {    // don't blow out the Str255
      dwLen = 255;
    }
    BlockMoveData(fType, &ourType[1], dwLen);
    ourType[0] = dwLen;

    // NOTE: We need to set the cursor because almost always we will have set it to the
    // hand cursor before we get here.
    Cursor qdArrow;
    SetCursor(GetQDGlobalsArrow(&qdArrow));

    // Now that we�ve queried the user about this mime type,
    // add it to our list so we won�t bug them again.
    AddMimeTypeToList(ourType);
    
    // If the user clicked "Get the Plug-in", either execute the
    // JavaScript file-installation URL, or ask Netscape to open
    // a new window with the page URL.  The title of the window
    // is arbitrary since it has nothing to do with the actual
    // window title shown to the user (it�s only used internally).
    NPN_PushPopupsEnabledState(fInstance, true);

    if (fFileURL) {
      (void) NPN_GetURL(fInstance, fFileURL, "_current");
    }
    else if (fPageURL) {
      NPN_GetURL(fInstance, fPageURL, "_blank");
    }
    else {
      pTheURL = MakeDefaultURL();
      if (!m_bJavaScript)
        NPN_GetURL(fInstance, pTheURL, "_blank");
      else
        NPN_GetURL(fInstance, pTheURL, NULL);
      NPN_MemFree(pTheURL);
    }

    NPN_PopPopupsEnabledState(fInstance);

    fUserInstalledPlugin = true;
    if (FocusDraw()) {
      Draw(kUnhilited);
      RestoreDraw();
    }
  }
}


void CPlugin::RefreshPluginPage()
{
  (void)NPN_GetURL(fInstance, "javascript:navigator.plugins.refresh(true);", "_self");
}


void CPlugin::DrawString(const unsigned char* text, short width, short height, short centerX, Rect drawRect)
{
  short length, textHeight, textWidth;

  if (!text)
    return;

  length = strlen((char*)text);
  TextFont(20);
  TextFace(underline);
  TextMode(srcCopy);
  TextSize(10);

  FontInfo fontInfo;
  GetFontInfo(&fontInfo);

  textHeight = fontInfo.ascent + fontInfo.descent + fontInfo.leading;
  textWidth = TextWidth(text, 0, length);

  if (width > textWidth && height > textHeight + 32) {
    MoveTo(centerX - (textWidth >> 1), drawRect.bottom + textHeight);
    DrawText(text, 0, length);
  }    
}


Boolean CPlugin::IsPluginHidden(int16 argc, char* argn[], char* argv[])
{
  for (int i = 0; i < argc; i++) {
    if (!strcasecmp(argn[i], "HIDDEN")) {
      if (!strcasecmp(argv[i], "TRUE"))
        return true;
    }
  }
  return false;
}
