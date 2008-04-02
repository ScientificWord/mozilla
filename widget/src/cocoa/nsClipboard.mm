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
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Josh Aas <josh@mozilla.com>
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

#include "nsCOMPtr.h"
#include "nsClipboard.h"
#include "nsString.h"
#include "nsISupportsPrimitives.h"
#include "nsXPIDLString.h"
#include "nsPrimitiveHelpers.h"
#include "nsMemory.h"
#include "nsIImage.h"
#include "nsILocalFile.h"
#include "nsStringStream.h"
#include "nsDragService.h"
#include "nsEscape.h"
#include "nsPrintfCString.h"
#include "nsObjCExceptions.h"

// Screenshots use the (undocumented) png pasteboard type.
#define IMAGE_PASTEBOARD_TYPES NSTIFFPboardType, @"Apple PNG pasteboard type", nil

#ifdef MOZ_LOGGING
#define FORCE_PR_LOG
#endif
#include "prlog.h"

#ifdef PR_LOGGING
extern PRLogModuleInfo* sCocoaLog;
#endif


nsClipboard::nsClipboard() : nsBaseClipboard()
{
  mChangeCount = 0;
}


nsClipboard::~nsClipboard()
{
}


// We separate this into its own function because after an @try, all local
// variables within that function get marked as volatile, and our C++ type 
// system doesn't like volatile things.
static NSData* 
GetDataFromPasteboard(NSPasteboard* aPasteboard, NSString* aType)
{
  NSData *data = nil;
  @try {
    data = [aPasteboard dataForType:aType];
  } @catch (NSException* e) {
    NS_WARNING(nsPrintfCString(256, "Exception raised while getting data from the pasteboard: \"%s - %s\"", 
                               [[e name] UTF8String], [[e reason] UTF8String]).get());
  }
  return data;
}


NS_IMETHODIMP
nsClipboard::SetNativeClipboardData(PRInt32 aWhichClipboard)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if ((aWhichClipboard != kGlobalClipboard) || !mTransferable)
    return NS_ERROR_FAILURE;

  mIgnoreEmptyNotification = PR_TRUE;

  NSDictionary* pasteboardOutputDict = PasteboardDictFromTransferable(mTransferable);
  if (!pasteboardOutputDict)
    return NS_ERROR_FAILURE;

  // write everything out to the general pasteboard
  unsigned int outputCount = [pasteboardOutputDict count];
  NSArray* outputKeys = [pasteboardOutputDict allKeys];
  NSPasteboard* generalPBoard = [NSPasteboard generalPasteboard];
  [generalPBoard declareTypes:outputKeys owner:nil];
  for (unsigned int i = 0; i < outputCount; i++) {
    NSString* currentKey = [outputKeys objectAtIndex:i];
    id currentValue = [pasteboardOutputDict valueForKey:currentKey];
    if (currentKey == NSStringPboardType ||
        currentKey == kCorePboardType_url ||
        currentKey == kCorePboardType_urld ||
        currentKey == kCorePboardType_urln)
      [generalPBoard setString:currentValue forType:currentKey];
    else
      [generalPBoard setData:currentValue forType:currentKey];
  }

  mChangeCount = [generalPBoard changeCount];

  mIgnoreEmptyNotification = PR_FALSE;

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


NS_IMETHODIMP
nsClipboard::GetNativeClipboardData(nsITransferable* aTransferable, PRInt32 aWhichClipboard)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  if ((aWhichClipboard != kGlobalClipboard) || !aTransferable)
    return NS_ERROR_FAILURE;

  NSPasteboard* cocoaPasteboard = [NSPasteboard generalPasteboard];
  if (!cocoaPasteboard)
    return NS_ERROR_FAILURE;

  // get flavor list that includes all acceptable flavors (including ones obtained through conversion)
  nsCOMPtr<nsISupportsArray> flavorList;
  nsresult rv = aTransferable->FlavorsTransferableCanImport(getter_AddRefs(flavorList));
  if (NS_FAILED(rv))
    return NS_ERROR_FAILURE;

  PRUint32 flavorCount;
  flavorList->Count(&flavorCount);

  // If we were the last ones to put something on the pasteboard, then just use the cached
  // transferable. Otherwise clear it because it isn't relevant any more.
  if (mChangeCount == [cocoaPasteboard changeCount]) {
    if (mTransferable) {
      for (PRUint32 i = 0; i < flavorCount; i++) {
        nsCOMPtr<nsISupports> genericFlavor;
        flavorList->GetElementAt(i, getter_AddRefs(genericFlavor));
        nsCOMPtr<nsISupportsCString> currentFlavor(do_QueryInterface(genericFlavor));
        if (!currentFlavor)
          continue;

        nsXPIDLCString flavorStr;
        currentFlavor->ToString(getter_Copies(flavorStr));

        nsCOMPtr<nsISupports> dataSupports;
        PRUint32 dataSize = 0;
        rv = mTransferable->GetTransferData(flavorStr, getter_AddRefs(dataSupports), &dataSize);
        if (NS_SUCCEEDED(rv)) {
          aTransferable->SetTransferData(flavorStr, dataSupports, dataSize);
          return NS_OK; // maybe try to fill in more types? Is there a point?
        }
      }
    }
  }
  else {
    nsBaseClipboard::EmptyClipboard(kGlobalClipboard);
  }

  // at this point we can't satisfy the request from cache data so let's look
  // for things other people put on the system clipboard

  for (PRUint32 i = 0; i < flavorCount; i++) {
    nsCOMPtr<nsISupports> genericFlavor;
    flavorList->GetElementAt(i, getter_AddRefs(genericFlavor));
    nsCOMPtr<nsISupportsCString> currentFlavor(do_QueryInterface(genericFlavor));
    if (!currentFlavor)
      continue;

    nsXPIDLCString flavorStr;
    currentFlavor->ToString(getter_Copies(flavorStr)); // i has a flavr

    // printf("looking for clipboard data of type %s\n", flavorStr.get());

    if (flavorStr.EqualsLiteral(kUnicodeMime)) {
      NSString* pString = [cocoaPasteboard stringForType:NSStringPboardType];
      if (!pString)
        continue;

      NSData* stringData = [pString dataUsingEncoding:NSUnicodeStringEncoding];
      unsigned int dataLength = [stringData length];
      void* clipboardDataPtr = malloc(dataLength);
      if (!clipboardDataPtr)
        return NS_ERROR_OUT_OF_MEMORY;
      [stringData getBytes:clipboardDataPtr];

      // The DOM only wants LF, so convert from MacOS line endings to DOM line endings.
      PRInt32 signedDataLength = dataLength;
      nsLinebreakHelpers::ConvertPlatformToDOMLinebreaks(flavorStr, &clipboardDataPtr, &signedDataLength);
      dataLength = signedDataLength;

      // skip BOM (Byte Order Mark to distinguish little or big endian)      
      PRUnichar* clipboardDataPtrNoBOM = (PRUnichar*)clipboardDataPtr;
      if ((dataLength > 2) &&
          ((clipboardDataPtrNoBOM[0] == 0xFEFF) ||
           (clipboardDataPtrNoBOM[0] == 0xFFFE))) {
        dataLength -= sizeof(PRUnichar);
        clipboardDataPtrNoBOM += 1;
      }

      nsCOMPtr<nsISupports> genericDataWrapper;
      nsPrimitiveHelpers::CreatePrimitiveForData(flavorStr, clipboardDataPtrNoBOM, dataLength,
                                                 getter_AddRefs(genericDataWrapper));
      aTransferable->SetTransferData(flavorStr, genericDataWrapper, dataLength);
      free(clipboardDataPtr);
      break;
    }
    else if (flavorStr.EqualsLiteral(kJPEGImageMime) ||
             flavorStr.EqualsLiteral(kPNGImageMime) ||
             flavorStr.EqualsLiteral(kGIFImageMime)) {
      // Figure out if there's data on the pasteboard we can grab (sanity check)
      NSString *type = [cocoaPasteboard availableTypeFromArray:[NSArray arrayWithObjects:IMAGE_PASTEBOARD_TYPES]];
      if (!type)
        continue;

      // Read data off the clipboard
      NSData *pasteboardData = GetDataFromPasteboard(cocoaPasteboard, type);
      if (!pasteboardData)
        continue;

      // Figure out what type we're converting to
      CFStringRef outputType = NULL; 
      if (flavorStr.EqualsLiteral(kJPEGImageMime))
        outputType = CFSTR("public.jpeg");
      else if (flavorStr.EqualsLiteral(kPNGImageMime))
        outputType = CFSTR("public.png");
      else if (flavorStr.EqualsLiteral(kGIFImageMime))
        outputType = CFSTR("com.compuserve.gif");
      else
        continue;

      // Use ImageIO to interpret the data on the clipboard and transcode.
      // Note that ImageIO, like all CF APIs, allows NULLs to propagate freely
      // and safely in most cases (like ObjC). A notable exception is CFRelease.
      NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:
                                (NSNumber*)kCFBooleanTrue, kCGImageSourceShouldAllowFloat,
                                (type == NSTIFFPboardType ? @"public.tiff" : @"public.png"),
                                kCGImageSourceTypeIdentifierHint, nil];

      CGImageSourceRef source = CGImageSourceCreateWithData((CFDataRef)pasteboardData, 
                                                            (CFDictionaryRef)options);
      NSMutableData *encodedData = [NSMutableData data];
      CGImageDestinationRef dest = CGImageDestinationCreateWithData((CFMutableDataRef)encodedData,
                                                                    outputType,
                                                                    1, NULL);
      CGImageDestinationAddImageFromSource(dest, source, 0, NULL);
      PRBool successfullyConverted = CGImageDestinationFinalize(dest);

      if (successfullyConverted) {
        // Put the converted data in a form Gecko can understand
        nsCOMPtr<nsIInputStream> byteStream;
        NS_NewByteInputStream(getter_AddRefs(byteStream), (const char*)[encodedData bytes],
                                   [encodedData length], NS_ASSIGNMENT_COPY);
  
        aTransferable->SetTransferData(flavorStr, byteStream, sizeof(nsIInputStream*));
      }

      if (dest)
        CFRelease(dest);
      if (source)
        CFRelease(source);
      
      if (successfullyConverted)
        break;
      else
        continue;
    }
  }

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// returns true if we have *any* of the passed in flavors available for pasting
NS_IMETHODIMP
nsClipboard::HasDataMatchingFlavors(const char** aFlavorList, PRUint32 aLength,
                                    PRInt32 aWhichClipboard, PRBool* outResult)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NSRESULT;

  *outResult = PR_FALSE;

  if ((aWhichClipboard != kGlobalClipboard) || !aFlavorList)
    return NS_OK;

  // first see if we have data for this in our cached transferable
  if (mTransferable) {    
    nsCOMPtr<nsISupportsArray> transferableFlavorList;
    nsresult rv = mTransferable->FlavorsTransferableCanImport(getter_AddRefs(transferableFlavorList));
    if (NS_SUCCEEDED(rv)) {
      PRUint32 transferableFlavorCount;
      transferableFlavorList->Count(&transferableFlavorCount);
      for (PRUint32 j = 0; j < transferableFlavorCount; j++) {
        nsCOMPtr<nsISupports> transferableFlavorSupports;
        transferableFlavorList->GetElementAt(j, getter_AddRefs(transferableFlavorSupports));
        nsCOMPtr<nsISupportsCString> currentTransferableFlavor(do_QueryInterface(transferableFlavorSupports));
        if (!currentTransferableFlavor)
          continue;
        nsXPIDLCString transferableFlavorStr;
        currentTransferableFlavor->ToString(getter_Copies(transferableFlavorStr));

        for (PRUint32 k = 0; k < aLength; k++) {
          if (transferableFlavorStr.Equals(aFlavorList[k])) {
            *outResult = PR_TRUE;
            return NS_OK;
          }
        }
      }      
    }    
  }

  NSPasteboard* generalPBoard = [NSPasteboard generalPasteboard];

  for (PRUint32 i = 0; i < aLength; i++) {
    if (!strcmp(aFlavorList[i], kUnicodeMime)) {
      NSString* availableType = [generalPBoard availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]];
      if (availableType && [availableType isEqualToString:NSStringPboardType]) {
        *outResult = PR_TRUE;
        break;
      }
    } else if (!strcmp(aFlavorList[i], kJPEGImageMime) ||
               !strcmp(aFlavorList[i], kPNGImageMime) ||
               !strcmp(aFlavorList[i], kGIFImageMime)) {
      NSString* availableType = [generalPBoard availableTypeFromArray:
                                  [NSArray arrayWithObjects:IMAGE_PASTEBOARD_TYPES]];
      if (availableType) {
        *outResult = PR_TRUE;
        break;
      }
    }
  }

  return NS_OK;

  NS_OBJC_END_TRY_ABORT_BLOCK_NSRESULT;
}


// This function converts anything that other applications might understand into the system format
// and puts it into a dictionary which it returns.
// static
NSDictionary* 
nsClipboard::PasteboardDictFromTransferable(nsITransferable* aTransferable)
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  if (!aTransferable)
    return nil;

  NSMutableDictionary* pasteboardOutputDict = [NSMutableDictionary dictionary];

  nsCOMPtr<nsISupportsArray> flavorList;
  nsresult rv = aTransferable->FlavorsTransferableCanExport(getter_AddRefs(flavorList));
  if (NS_FAILED(rv))
    return nil;

  PRUint32 flavorCount;
  flavorList->Count(&flavorCount);
  for (PRUint32 i = 0; i < flavorCount; i++) {
    nsCOMPtr<nsISupports> genericFlavor;
    flavorList->GetElementAt(i, getter_AddRefs(genericFlavor));
    nsCOMPtr<nsISupportsCString> currentFlavor(do_QueryInterface(genericFlavor));
    if (!currentFlavor)
      continue;

    nsXPIDLCString flavorStr;
    currentFlavor->ToString(getter_Copies(flavorStr));

    PR_LOG(sCocoaLog, PR_LOG_ALWAYS, ("writing out clipboard data of type %s (%d)\n", flavorStr.get(), i));

    if (flavorStr.EqualsLiteral(kUnicodeMime)) {
      void* data = nsnull;
      PRUint32 dataSize = 0;
      nsCOMPtr<nsISupports> genericDataWrapper;
      rv = aTransferable->GetTransferData(flavorStr, getter_AddRefs(genericDataWrapper), &dataSize);
      nsPrimitiveHelpers::CreateDataFromPrimitive(flavorStr, genericDataWrapper, &data, dataSize);
      
      NSString* nativeString = [NSString stringWithCharacters:(const unichar*)data length:(dataSize / sizeof(PRUnichar))];
      // be nice to Carbon apps, normalize the receiver's contents using Form C.
      nativeString = [nativeString precomposedStringWithCanonicalMapping];
      [pasteboardOutputDict setObject:nativeString forKey:NSStringPboardType];
      
      nsMemory::Free(data);
    }
    else if (flavorStr.EqualsLiteral(kPNGImageMime) || flavorStr.EqualsLiteral(kJPEGImageMime) ||
             flavorStr.EqualsLiteral(kGIFImageMime) || flavorStr.EqualsLiteral(kNativeImageMime)) {
      PRUint32 dataSize = 0;
      nsCOMPtr<nsISupports> transferSupports;
      aTransferable->GetTransferData(flavorStr, getter_AddRefs(transferSupports), &dataSize);
      nsCOMPtr<nsISupportsInterfacePointer> ptrPrimitive(do_QueryInterface(transferSupports));
      if (!ptrPrimitive)
        continue;

      nsCOMPtr<nsISupports> primitiveData;
      ptrPrimitive->GetData(getter_AddRefs(primitiveData));

      nsCOMPtr<nsIImage> image(do_QueryInterface(primitiveData));
      if (!image) {
        NS_WARNING("Image isn't an nsIImage in transferable");
        continue;
      }

      if (NS_FAILED(image->LockImagePixels(PR_FALSE)))
        continue;

      PRInt32 height = image->GetHeight();
      PRInt32 stride = image->GetLineStride();
      PRInt32 width = image->GetWidth();
      if ((stride % 4 != 0) || (height < 1) || (width < 1))
        continue;

      // Create a CGImageRef with the bits from the image, taking into account
      // the alpha ordering and endianness of the machine so we don't have to
      // touch the bits ourselves.
      CGDataProviderRef dataProvider = CGDataProviderCreateWithData(NULL,
                                                                    image->GetBits(),
                                                                    stride * height,
                                                                    NULL);
      CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
      CGImageRef imageRef = CGImageCreate(width,
                                          height,
                                          8,
                                          32,
                                          stride,
                                          colorSpace,
                                          kCGBitmapByteOrder32Host | kCGImageAlphaFirst,
                                          dataProvider,
                                          NULL,
                                          0,
                                          kCGRenderingIntentDefault);
      CGColorSpaceRelease(colorSpace);
      CGDataProviderRelease(dataProvider);

      // Convert the CGImageRef to TIFF data.
      CFMutableDataRef tiffData = CFDataCreateMutable(kCFAllocatorDefault, 0);
      CGImageDestinationRef destRef = CGImageDestinationCreateWithData(tiffData,
                                                                       CFSTR("public.tiff"),
                                                                       1,
                                                                       NULL);
      CGImageDestinationAddImage(destRef, imageRef, NULL);
      PRBool successfullyConverted = CGImageDestinationFinalize(destRef);

      CGImageRelease(imageRef);
      if (destRef)
        CFRelease(destRef);

      if (NS_FAILED(image->UnlockImagePixels(PR_FALSE)) || !successfullyConverted) {
        if (tiffData)
          CFRelease(tiffData);
        continue;
      }

      [pasteboardOutputDict setObject:(NSMutableData*)tiffData forKey:NSTIFFPboardType];
      if (tiffData)
        CFRelease(tiffData);
    }
    else if (flavorStr.EqualsLiteral(kFilePromiseMime)) {
      [pasteboardOutputDict setObject:[NSArray arrayWithObject:@""] forKey:NSFilesPromisePboardType];      
    }
    else if (flavorStr.EqualsLiteral(kURLMime)) {
      PRUint32 len = 0;
      nsCOMPtr<nsISupports> genericURL;
      rv = aTransferable->GetTransferData(flavorStr, getter_AddRefs(genericURL), &len);
      nsCOMPtr<nsISupportsString> urlObject(do_QueryInterface(genericURL));

      nsAutoString url;
      urlObject->GetData(url);

      // A newline embedded in the URL means that the form is actually URL + title.
      PRInt32 newlinePos = url.FindChar(PRUnichar('\n'));
      if (newlinePos >= 0) {
        url.Truncate(newlinePos);

        nsAutoString urlTitle;
        urlObject->GetData(urlTitle);
        urlTitle.Mid(urlTitle, newlinePos + 1, len - (newlinePos + 1));

        NSString *nativeTitle = [[NSString alloc] initWithCharacters:urlTitle.get() length:urlTitle.Length()];
        // be nice to Carbon apps, normalize the receiver's contents using Form C.
        [pasteboardOutputDict setObject:[nativeTitle precomposedStringWithCanonicalMapping] forKey:kCorePboardType_urln];
        // Also put the title out as 'urld', since some recipients will look for that.
        [pasteboardOutputDict setObject:[nativeTitle precomposedStringWithCanonicalMapping] forKey:kCorePboardType_urld];
        [nativeTitle release];
      }

      // The Finder doesn't like getting random binary data aka
      // Unicode, so change it into an escaped URL containing only
      // ASCII.
      nsCAutoString utf8Data = NS_ConvertUTF16toUTF8(url.get(), url.Length());
      nsCAutoString escData;
      NS_EscapeURL(utf8Data.get(), utf8Data.Length(), esc_OnlyNonASCII|esc_AlwaysCopy, escData);

      // printf("Escaped url is %s, length %d\n", escData.get(), escData.Length());

      NSString *nativeURL = [NSString stringWithUTF8String:escData.get()];
      [pasteboardOutputDict setObject:nativeURL forKey:kCorePboardType_url];
    }
    // If it wasn't a type that we recognize as exportable we don't put it on the system
    // clipboard. We'll just access it from our cached transferable when we need it.
  }

  return pasteboardOutputDict;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}
