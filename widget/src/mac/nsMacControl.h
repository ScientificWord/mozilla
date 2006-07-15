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

#ifndef nsMacControl_h__
#define nsMacControl_h__

#include "nsIServiceManager.h"
#include "nsICharsetConverterManager.h"
#include "nsChildWindow.h"
#include <Controls.h>

class nsMacControl : public nsChildWindow
{
private:
	typedef nsChildWindow Inherited;

public:
						nsMacControl();
	virtual				~nsMacControl();

	NS_IMETHOD 			Create(nsIWidget *aParent,
				              const nsRect &aRect,
				              EVENT_CALLBACK aHandleEventFunction,
				              nsIDeviceContext *aContext = nsnull,
				              nsIAppShell *aAppShell = nsnull,
				              nsIToolkit *aToolkit = nsnull,
				              nsWidgetInitData *aInitData = nsnull);
	NS_IMETHOD			Destroy();

	virtual void		SetControlType(short type)	{mControlType = type;}
	short				GetControlType()			{return mControlType;}

	// event handling
	virtual PRBool		OnPaint(nsPaintEvent & aEvent);
	virtual PRBool		DispatchMouseEvent(nsMouseEvent &aEvent);
	static pascal OSStatus	ControlEventHandler(EventHandlerCallRef aHandlerCallRef, EventRef aEvent, void* aUserData);
	static pascal OSStatus	WindowEventHandler(EventHandlerCallRef aHandlerCallRef, EventRef aEvent, void* aUserData);
    
	// nsIWidget interface
	NS_IMETHOD			Enable(PRBool bState);
	NS_IMETHOD			Show(PRBool aState);
	NS_IMETHODIMP		SetFont(const nsFont &aFont);

	// Mac string utilities
	// (they really should be elsewhere but, well, only the Mac controls use them)
	static void 		StringToStr255(const nsAString& aText, Str255& aStr255);
	static void 		Str255ToString(const Str255& aStr255, nsString& aText);

protected:
	
	nsresult			CreateOrReplaceMacControl(short inControlType);

 	void				ClearControl();

	virtual void		GetRectForMacControl(nsRect &outRect);
	virtual ControlPartCode	GetControlHiliteState();
	void			SetupControlHiliteState();

	void				SetupMacControlFont();
	void				ControlChanged(PRInt32 aNewValue);
	void				NSStringSetControlTitle(ControlHandle theControl, nsString title);
	void				SetupMacControlFontForScript(short theScript);
	static void			GetFileSystemCharset(nsCString & fileSystemCharset);

	OSStatus			InstallEventHandlerOnControl();
	void				RemoveEventHandlerFromControl();

	PRBool				IsQDStateOK();

	nsString			mLabel;
	PRBool				mWidgetArmed;
	PRBool				mMouseInButton;

	PRInt32				mValue;
	PRInt32				mMin;
	PRInt32				mMax;
	ControlHandle			mControl;
	short				mControlType;
	EventHandlerRef			mControlEventHandler;
	EventHandlerRef			mWindowEventHandler;

	nsString			mLastLabel;
	nsRect				mLastBounds;
	PRInt32				mLastValue;
	PRInt16				mLastHilite;

	static nsIUnicodeEncoder*     mUnicodeEncoder;
	static nsIUnicodeDecoder*     mUnicodeDecoder;
};

#endif // nsMacControl_h__
