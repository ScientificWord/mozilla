/* -*- Mode: C++; tab-width: 3; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is the Mozilla browser.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications, Inc.
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Conrad Carlen <ccarlen@netscape.com>
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

#include "CThrobber.h"

#include <LString.h>
#include <LStream.h>
#include <UDrawingState.h>
#include <UResourceMgr.h>

#include <QuickTimeComponents.h>

//*****************************************************************************
//***    CThrobber: constructors/destructor
//*****************************************************************************

CThrobber::CThrobber() :
   mMovieResID(-1), mMovieHandle(nil),
   mMovie(nil), mMovieController(nil)
{
}


CThrobber::CThrobber(LStream*	inStream) :
   LControl(inStream),
   mMovieResID(-1), mMovieHandle(nil),
   mMovie(nil), mMovieController(nil)
{
   *inStream >> mMovieResID;
}


CThrobber::~CThrobber()
{
    if (mMovieController)
        ::DisposeMovieController(mMovieController);
    if (mMovieHandle)
        ::DisposeHandle(mMovieHandle);   
}


void CThrobber::FinishCreateSelf()
{
   CreateMovie();
}


void CThrobber::ShowSelf()
{
    LControl::ShowSelf();
    
    if (!mMovieController)
        return;
    
    ::MCSetClip(mMovieController, NULL, NULL);
}


void CThrobber::HideSelf()
{
    LControl::HideSelf();
    
    if (!mMovieController)
        return;    
    
    StRegion emptyRgn;
    ::MCSetClip(mMovieController, NULL, emptyRgn);
}


void CThrobber::DrawSelf()
{
    if (mMovieController)
        ::MCDraw(mMovieController, GetMacWindow());
}


void CThrobber::ResizeFrameBy(SInt16		inWidthDelta,
                					SInt16		inHeightDelta,
                					Boolean	   inRefresh)
{
	LControl::ResizeFrameBy(inWidthDelta, inHeightDelta, inRefresh);
	
	if (!mMovieController)
	    return;
	    
	FocusDraw();

	Rect newFrame;
	::MCGetControllerBoundsRect(mMovieController, &newFrame);
	newFrame.right += inWidthDelta;
	newFrame.bottom += inHeightDelta;
	::MCSetControllerBoundsRect(mMovieController, &newFrame);
}


void CThrobber::MoveBy(SInt32		inHorizDelta,
				           SInt32		inVertDelta,
							  Boolean	inRefresh)
{
	LControl::MoveBy(inHorizDelta, inVertDelta, inRefresh);

	if (!mMovieController)
	    return;
 
 	FocusDraw();
 
	Rect newFrame;	
	::MCGetControllerBoundsRect(mMovieController, &newFrame);
	::OffsetRect(&newFrame, inHorizDelta, inVertDelta);
	::MCSetControllerBoundsRect(mMovieController, &newFrame);
}


void CThrobber::SpendTime(const EventRecord	&inMacEvent)
{
	FocusDraw();
	::MCIsPlayerEvent(mMovieController, &inMacEvent);
}

	
void CThrobber::Start()
{
	if (!mMovieController)
	    return;
 
    ::StartMovie(mMovie);
    StartRepeating();
}


void CThrobber::Stop()
{
	if (!mMovieController)
	    return;
 
    StopRepeating();
    ::StopMovie(mMovie);
    ::GoToBeginningOfMovie(mMovie);
    Refresh();
}


void CThrobber::CreateMovie()
{
    OSErr err;
    Handle dataRef = NULL;
        
    try {
        // Get the movie from our resource
        StApplicationContext appResContext;
        
        mMovieHandle = ::Get1Resource('GIF ', 128);
        ThrowIfResFail_(mMovieHandle);
        ::DetachResource(mMovieHandle);
        
        // Create a dataRef handle - from TN2018
    	err = ::PtrToHand(&mMovieHandle, &dataRef, sizeof(Handle));
    	ThrowIfError_(err);
    	err = ::PtrAndHand("\p", dataRef, 1); 
    	ThrowIfError_(err);
        long	fileTypeAtom[3];
    	fileTypeAtom[0] = sizeof(long) * 3;
    	fileTypeAtom[1] = kDataRefExtensionMacOSFileType;
    	fileTypeAtom[2] = kQTFileTypeGIF;
    	err = ::PtrAndHand(fileTypeAtom, dataRef, sizeof(long) * 3);
    	ThrowIfError_(err);
    	
    	err = ::NewMovieFromDataRef(&mMovie, newMovieActive, NULL, dataRef, HandleDataHandlerSubType);
        ThrowIfError_(err);
    	DisposeHandle(dataRef);
    	dataRef = NULL;
    	
    	// Make a controller for the movie
    	Rect	movieBounds;
    	::GetMovieBox(mMovie, &movieBounds);
    	::MacOffsetRect(&movieBounds, (SInt16) -movieBounds.left, (SInt16) -movieBounds.top);
    	::SetMovieBox(mMovie, &movieBounds);

    	::SetMovieGWorld(mMovie, (CGrafPtr) GetMacPort(), nil);

    	Rect	frame;
    	CalcLocalFrameRect(frame);
    	mMovieController = ::NewMovieController(mMovie, &frame, mcTopLeftMovie + mcNotVisible);
        
    	::MCDoAction(mMovieController, mcActionSetLooping, (void *)1);
    	::MCMovieChanged(mMovieController, mMovie);
    }
    catch (...) {
        if (dataRef)
            DisposeHandle(dataRef);
        
        // Don't rethrow
    }
}
