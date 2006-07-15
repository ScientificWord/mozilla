/* -*- Mode: c++; tab-width: 2; indent-tabs-mode: nil; -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

#include "nscore.h"
#include "nsMacEventHandler.h"
#include "nsMacTSMMessagePump.h"
#include "nsString.h"
#include <Script.h>
#include <TextServices.h>
#include <AEDataModel.h>

#include "nsCarbonHelpers.h"


//-------------------------------------------------------------------------
//
// TSM AE Handler routines UPPs 
//
//-------------------------------------------------------------------------

AEEventHandlerUPP nsMacTSMMessagePump::mPos2OffsetUPP = NULL;
AEEventHandlerUPP nsMacTSMMessagePump::mOffset2PosUPP = NULL;
AEEventHandlerUPP nsMacTSMMessagePump::mUpdateUPP = NULL;
AEEventHandlerUPP nsMacTSMMessagePump::mKeyboardUPP = NULL;
AEEventHandlerUPP nsMacTSMMessagePump::mGetSelectedTextUPP = NULL;

//-------------------------------------------------------------------------
//
// Constructor/Destructors
//
//-------------------------------------------------------------------------
nsMacTSMMessagePump::nsMacTSMMessagePump()
{
	OSErr	err;

	mPos2OffsetUPP = NewAEEventHandlerUPP(nsMacTSMMessagePump::PositionToOffsetHandler);
	NS_ASSERTION(mPos2OffsetUPP!=NULL, "nsMacTSMMessagePump::InstallTSMAEHandlers: NewAEEventHandlerUPP[Pos2Pffset] failed");

	mOffset2PosUPP = NewAEEventHandlerUPP(nsMacTSMMessagePump::OffsetToPositionHandler);
	NS_ASSERTION(mPos2OffsetUPP!=NULL, "nsMacTSMMessagePump::InstallTSMAEHandlers: NewAEEventHandlerUPP[Pos2Pffset] failed");

  mUpdateUPP = NewAEEventHandlerUPP(nsMacTSMMessagePump::UnicodeUpdateHandler);
  NS_ASSERTION(mPos2OffsetUPP!=NULL, "nsMacTSMMessagePump::InstallTSMAEHandlers: NewAEEventHandlerUPP[Pos2Pffset] failed");

  err = AEInstallEventHandler(kTextServiceClass, kPos2Offset, mPos2OffsetUPP, (long)this, false);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::InstallTSMAEHandlers: AEInstallEventHandler[Pos2Offset] failed");

  err = AEInstallEventHandler(kTextServiceClass, kOffset2Pos, mOffset2PosUPP, (long)this, false);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::InstallTSMAEHandlers: AEInstallEventHandler[Offset2Pos] failed");

  err = AEInstallEventHandler(kTextServiceClass, kUpdateActiveInputArea, mUpdateUPP, (long)this, false);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::InstallTSMAEHandlers: AEInstallEventHandler[Update] failed");

  mKeyboardUPP = NewAEEventHandlerUPP(nsMacTSMMessagePump::UnicodeNotFromInputMethodHandler);
  NS_ASSERTION(mKeyboardUPP!=NULL, "nsMacTSMMessagePump::InstallTSMAEHandlers: NewAEEventHandlerUPP[FromInputMethod] failed");

  err = AEInstallEventHandler(kTextServiceClass, kUnicodeNotFromInputMethod, mKeyboardUPP, (long)this, false);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::InstallTSMAEHandlers: AEInstallEventHandler[FromInputMethod] failed");

  mGetSelectedTextUPP = NewAEEventHandlerUPP(nsMacTSMMessagePump::UnicodeGetSelectedTextHandler);
  NS_ASSERTION(mGetSelectedTextUPP!=NULL, "nsMacTSMMessagePump::InstallTSMAEHandlers: NewAEEventHandlerUPP[GetSelectedText] failed");

  err = AEInstallEventHandler(kTextServiceClass, kGetSelectedText, mGetSelectedTextUPP, (long)this, false);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::InstallTSMAEHandlers: AEInstallEventHandler[GetSelectedText] failed");

}

nsMacTSMMessagePump::~nsMacTSMMessagePump()
{
	OSErr	err;
	
	err = AERemoveEventHandler(kTextServiceClass, kPos2Offset, mPos2OffsetUPP, false);
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::RemoveTSMAEHandlers: AERemoveEventHandler[Pos2Offset] failed");

	err = AERemoveEventHandler(kTextServiceClass, kOffset2Pos, mOffset2PosUPP, false);
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::RemoveTSMAEHandlers: AERemoveEventHandler[Offset2Pos] failed");

	err = AERemoveEventHandler(kTextServiceClass, kUpdateActiveInputArea, mUpdateUPP, false);
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::RemoveTSMAEHandlers: AERemoveEventHandler[Update] failed");

 	::DisposeAEEventHandlerUPP(mPos2OffsetUPP);
 	::DisposeAEEventHandlerUPP(mOffset2PosUPP);
 	::DisposeAEEventHandlerUPP(mUpdateUPP);

  err = AERemoveEventHandler(kTextServiceClass, kUnicodeNotFromInputMethod, mKeyboardUPP, false);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::RemoveTSMAEHandlers: AERemoveEventHandler[FromInputMethod] failed");
  ::DisposeAEEventHandlerUPP(mKeyboardUPP);

  err = AERemoveEventHandler(kTextServiceClass, kGetSelectedText, mGetSelectedTextUPP, false);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::RemoveTSMAEHandlers: AERemoveEventHandler[GetSelectedText] failed");
  ::DisposeAEEventHandlerUPP(mGetSelectedTextUPP);

}

//-------------------------------------------------------------------------
nsMacTSMMessagePump* nsMacTSMMessagePump::gSingleton = nsnull;
//-------------------------------------------------------------------------
nsMacTSMMessagePump* nsMacTSMMessagePump::GetSingleton()
{
	if (nsnull == gSingleton)
	{
		gSingleton = new nsMacTSMMessagePump();
		NS_ASSERTION(gSingleton!=NULL, "nsMacTSMMessagePump::GetSingleton: Unable to create TSM Message Pump.");
	}
	return gSingleton;
}
//-------------------------------------------------------------------------
void nsMacTSMMessagePump::Shutdown()
{
	if (gSingleton) {
		delete gSingleton;
		gSingleton = nsnull;
	}
}

//-------------------------------------------------------------------------
//
// TSM AE Handler routines
//
//-------------------------------------------------------------------------

pascal OSErr nsMacTSMMessagePump::PositionToOffsetHandler(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
	OSErr 				err;
	DescType			returnedType;
	nsMacEventHandler*	eventHandler;	
	Size				actualSize;
	Point				thePoint;
	long				offset;
	short				regionClass;

	//
	// Extract the nsMacEventHandler for this TSMDocument.  It's stored as the TSMDocument's
	//	refcon
	//
	err = AEGetParamPtr(theAppleEvent, keyAETSMDocumentRefcon, typeLongInteger, &returnedType,
						&eventHandler, sizeof(eventHandler), &actualSize);
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::PositionToOffsetHandler: AEGetParamPtr[TSMRefcon] failed");
	if (err!=noErr) 
		return err;
	
	//
	// Extract the Position parameter.
	//
	err = AEGetParamPtr(theAppleEvent, keyAECurrentPoint, typeQDPoint, &returnedType,
						&thePoint, sizeof(thePoint), &actualSize);
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::PositionToOffsetHandler: AGGetParamPtr[Point] failed");
	if (err!=noErr) 
		return err;

	//
	// pass the request to the widget system
	//
	offset = eventHandler->HandlePositionToOffset(thePoint, &regionClass);
	
	//
	// build up the AE reply (need offset and region class)
	//
	err = AEPutParamPtr(reply, keyAEOffset, typeLongInteger, &offset, sizeof(offset));
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::PositionToOffsetHandler: AEPutParamPtr failed");
	if (err!=noErr) 
		return err;
	
	err = AEPutParamPtr(reply, keyAERegionClass, typeShortInteger, &regionClass, sizeof(regionClass));
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::PositionToOffsetHandler: AEPutParamPtr failed");
	if (err!=noErr) 
		return err;

	return noErr;
}
pascal OSErr nsMacTSMMessagePump::OffsetToPositionHandler(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
	OSErr 				err;
	DescType			returnedType;
	nsMacEventHandler*	eventHandler;	
	Size				actualSize;
	Point				thePoint;
	long				offset;
	nsresult			res;

	//
	// Extract the nsMacEvenbtHandler for this TSMDocument.  It's stored as the refcon.
	//
	err = AEGetParamPtr(theAppleEvent, keyAETSMDocumentRefcon, typeLongInteger, &returnedType,
						&eventHandler, sizeof(eventHandler), &actualSize);
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::OffsetToPositionHandler: AEGetParamPtr[TSMRefcon] failed.");
	if (err!=noErr) 
		return err;
	
	//
	// Extract the Offset parameter
	//
	err = AEGetParamPtr(theAppleEvent, keyAEOffset, typeLongInteger, &returnedType,
						&offset, sizeof(offset), &actualSize);
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::PositionToOffsetHandler: AEGetParamPtr[Offset] failed.");
	if (err!=noErr) 
		return err;
	
	//
	// Pass the OffsetToPosition request to the widgets to handle
	//
	res = eventHandler->HandleOffsetToPosition(offset, &thePoint);
	NS_ASSERTION(NS_SUCCEEDED(res), "nsMacMessagePup::PositionToOffsetHandler: OffsetToPosition handler failed.");
	if (NS_FAILED(res)) 
		return paramErr;
	
	//
	// build up the reply (point)
	//
	err = AEPutParamPtr(reply, keyAEPoint, typeQDPoint, &thePoint, sizeof(Point));
	NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::PositionToOffsetHandler: AEPutParamPtr[Point] failed.");
	if (err!=noErr) 
		return err;
	
	return noErr;
}

static OSErr GetAppleEventTSMData(const AppleEvent *inAE, nsMacEventHandler **outEventHandler, AEDesc *outText)
{
  *outEventHandler = nsnull;
  //*outText = nsnull;

  //
  // refcon stores the nsMacEventHandler
  //
  DescType returnedType;
  Size actualSize;
  OSErr err = ::AEGetParamPtr(inAE, keyAETSMDocumentRefcon, typeLongInteger, &returnedType,
                              outEventHandler, sizeof(nsMacEventHandler *), &actualSize);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::GetAppleEventTSMData: AEGetParamPtr[TSMRefcon] failed.");
  if (err)
    return err;
 
  //
  // get text
  //
  err = ::AEGetParamDesc(inAE, keyAETheData, typeUnicodeText, outText);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::GetAppleEventTSMData: AEGetParamDesc[Text] failed.");
  return err;
}

static OSErr AETextToString(AEDesc &aAEDesc, nsString& aOutString, Size& text_size)
{
  OSErr err = noErr;
  PRUnichar* unicodeTextPtr;
  text_size = 0;
  aOutString.Truncate(0);


  text_size = ::AEGetDescDataSize(&aAEDesc) / 2;
  if (!EnsureStringLength(aOutString, text_size + 1))
    return memFullErr;
  unicodeTextPtr = aOutString.BeginWriting();
  err = AEGetDescData(&aAEDesc, (void *) unicodeTextPtr, text_size * 2);
  if (err!=noErr) 
    return err;

  unicodeTextPtr[text_size ] = PRUnichar('\0'); // null terminate it.
  return noErr;
}



pascal OSErr nsMacTSMMessagePump::UnicodeUpdateHandler(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
  OSErr                 err = noErr;
  DescType              returnedType;
  nsMacEventHandler*    eventHandler;
  Size                  actualSize;
  AEDesc                text, hiliteRangeArray;
  long                  fixLength;
  nsresult              res;
  TextRangeArray*       hiliteRangePtr;

  err = GetAppleEventTSMData(theAppleEvent, &eventHandler, &text);
  if (err) 
    return err;

  //
  // length of converted text
  //
  err = ::AEGetParamPtr(theAppleEvent, keyAEFixLength, typeLongInteger, &returnedType,
                        &fixLength, sizeof(fixLength), &actualSize);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::UnicodeUpdateHandler: AEGetParamPtr[fixlen] failed.");
  if (err) 
    return err;

  //
  // extract the hilite ranges (optional param)
  //
  err = ::AEGetParamDesc(theAppleEvent, keyAEHiliteRange, typeTextRangeArray, &hiliteRangeArray);
  NS_ASSERTION(err==noErr||err==errAEDescNotFound, "nsMacTSMMessagePump::UnicodeUpdateHandler: AEGetParamDesc[hiliteRangeArray] failed.");
  if (errAEDescNotFound == err)
  {
    hiliteRangePtr = NULL;
  } 
  else if (noErr == err)
  { 

    Size hiliteRangeSize = ::AEGetDescDataSize(&hiliteRangeArray);
    hiliteRangePtr = (TextRangeArray *) NewPtr(hiliteRangeSize);
    if (!hiliteRangePtr)
    {
      err = MemError();
      goto err2;
    }
    err = ::AEGetDescData(&hiliteRangeArray, (void *)hiliteRangePtr, hiliteRangeSize);
    if (noErr != err)
    {
      goto err1; 
    }
  }
  else
  {
    goto err3;
  }

  // grab the text now
  {
    nsAutoString unicodeText;
    Size text_size;
    err = AETextToString(text, unicodeText, text_size);
    if (noErr == err)
    {
      res = eventHandler->UnicodeHandleUpdateInputArea(unicodeText.get(), text_size, fixLength / 2, hiliteRangePtr);
      NS_ASSERTION(NS_SUCCEEDED(res), "nsMacMessagePump::UnicodeUpdateHandler: HandleUpdated failed.");
      if (NS_FAILED(res))
        err = paramErr;
        // fall to cleanup below
    }
  }

  //
  // clean up
  //
err1:
    if (hiliteRangePtr)
      ::DisposePtr((Ptr)hiliteRangePtr);

err2:
  (void)::AEDisposeDesc(&hiliteRangeArray);
err3:
  (void)::AEDisposeDesc(&text);

  return err;
}

pascal OSErr nsMacTSMMessagePump::UnicodeNotFromInputMethodHandler(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
  OSErr               err;
  nsMacEventHandler*  eventHandler;
  AEDesc              text;
  DescType            returnedType;
  Size                actualSize;
  EventRecord         event;

  err = GetAppleEventTSMData(theAppleEvent, &eventHandler, &text);
  if (err != noErr) 
    return err;
  
  err = ::AEGetParamPtr(theAppleEvent, keyAETSMEventRecord, typeLowLevelEventRecord, &returnedType,
                        &event, sizeof(event), &actualSize);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::UnicodeNotFromInputMethodHandler: AEGetParamPtr[event] failed.");
  if (noErr != err)
  {
    (void)::AEDisposeDesc(&text);
    return err;
  }

  nsAutoString unicodeText;
  Size text_size;
  err = AETextToString(text, unicodeText, text_size);
  if (noErr == err)
  {
    nsresult res;
    res = eventHandler->HandleUKeyEvent(unicodeText.get(), text_size, event);
    NS_ASSERTION(NS_SUCCEEDED(res), "nsMacMessagePump::UnicodeNotFromInputMethodHandler: HandleUpdated failed.");
    if (NS_FAILED(res)) {
      err = paramErr;
      // fall to cleanup below
    }
  }

  (void)::AEDisposeDesc(&text);
  return err;
}


pascal OSErr nsMacTSMMessagePump::UnicodeGetSelectedTextHandler(const AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon)
{
  OSErr err;  
  DescType returnedType;
  nsMacEventHandler*  eventHandler;  
  Size actualSize;
  long maxReturnSize = 0;
  long returnSize;
  nsresult res;

  //
  // Extract the nsMacEvenbtHandler for this TSMDocument.  It's stored as the refcon.
  //
  err = AEGetParamPtr(theAppleEvent, keyAETSMDocumentRefcon, typeLongInteger, &returnedType,
            &eventHandler, sizeof(eventHandler), &actualSize);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::UnicodeGetSelectedTextHandler: AEGetParamPtr[TSMRefcon] failed.");
  if (err!=noErr) 
    return err;
  
  //
  // Extract the Offset parameter
  //
  err = AEGetParamPtr(theAppleEvent, keyAEBufferSize, typeLongInteger, &returnedType,
            &maxReturnSize, sizeof(maxReturnSize), &actualSize);
  // optional parameter, ignore if we cannot get it.
  
  //
  // Pass the OffsetToPosition request to the widgets to handle
  //
  nsAutoString outString;
  res = eventHandler->HandleUnicodeGetSelectedText(outString);
  NS_ASSERTION(NS_SUCCEEDED(res), "nsMacMessagePup::UnicodeGetSelectedTextHandler: HandleGetSelectedText handler failed.");
  if (NS_FAILED(res)) 
    return paramErr;
  
  //
  // build up the reply (point)
  //
  returnSize = outString.Length()*sizeof(PRUnichar);
  if ((maxReturnSize >0) && (returnSize > maxReturnSize))
    returnSize = maxReturnSize & ~1L; // Round down
  
  err = AEPutParamPtr(reply, keyAETheData, typeUnicodeText, outString.get(), returnSize);
  NS_ASSERTION(err==noErr, "nsMacTSMMessagePump::UnicodeGetSelectedTextHandler: AEPutParamPtr[Point] failed.");
  if (err!=noErr) 
    return err;
  
  return noErr;
}
