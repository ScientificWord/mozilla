/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 *   Ashish Bhatt <ashishbhatt@netscape.com>
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

// File Overview....
//
// Test cases for the nsIDOMWindow Interface


#include "stdafx.h"
#include "QaUtils.h"
#include <stdio.h>
#include "domWindow.h"
#include "resource.h"


CDomWindow::CDomWindow(nsIWebBrowser* mWebBrowser)
{
	qaWebBrowser = mWebBrowser ;
}

CDomWindow::~CDomWindow()
{

}


void CDomWindow::OnStartTests(UINT nMenuID)
{
	// Calls  all or indivdual test cases on the basis of the 
	// option selected from menu.

	switch(nMenuID)
	{
	case ID_INTERFACES_NSIDOMWINDOW_RUNALLTESTS :
		RunAllTests();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETDOMDOCUMENT :
		GetDocument();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETPARENT : 
		GetParent();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETSCROLLBARS :
		GetScrollbars();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETFRAMES :
		GetFrames();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETNAME :
		//GetName();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETTEXTZOOM : 
		GetTextZoom();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_SETTEXTZOOM : 
		SetTextZoom();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETSCSOLLX : 
		GetScrollX();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETSCROLLY :
		GetScrollY();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_SCROLLTO :
		ScrollTo();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_SCROLLBY :
		ScrollBy();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_GETSELECTION :
		GetSelection();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_SCROLLBYLINES :
		ScrollByLines();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_SCROLLBYPAGES :
		ScrollByPages();
		break;
	case ID_INTERFACES_NSIDOMWINDOW_SIZETOCONTENT :
		SizeToContent();
		break;
	default :
		AfxMessageBox("Not added menu handler for this menu item");
		break;
	}
}
void CDomWindow::RunAllTests()
{
	PRInt32 scrollX = 0 ;
	PRInt32 scrollY = 0;
	PRInt32 bVisible =0;
	float fTextZoom = 0.0;

	nsCOMPtr<nsIDOMWindow> oDomWindow;
	//nsCOMPtr<nsIDOMWindow> oDomWindowTop;
	//nsCOMPtr<nsIDOMWindow> oDomWindowParent;

	nsCOMPtr<nsIDOMBarProp> oDomBarProp;
	nsCOMPtr<nsIDOMWindowCollection> oDomWindowCol;
	nsCOMPtr<nsIDOMDocument> oDomDocument;
	nsCOMPtr<nsISelection> oSelection;
	
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 1);
	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}

/*	rv = oDomWindow->GetTop(getter_AddRefs(oDomWindowTop));
    RvTestResult(rv, "nsIDOMWindow::GetTop()' rv test", 1);
	if (!oDomWindowTop)
	{
		AfxMessageBox("Cannot create Dom Window Top Object");
		return;
	}

	rv = oDomWindow->GetParent(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIDOMWindow::GetParent()' rv test", 1);
	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Parent Object");
		return;
	}

*/

	rv = oDomWindow->GetScrollbars(getter_AddRefs(oDomBarProp));
    RvTestResult(rv, "nsIDOMWindow::GetScrollbars()' rv test", 1);
	if (!oDomBarProp)
	{
		AfxMessageBox("Cannot create Dom Window Scroll Bar Object");
		return;
	}

	rv = oDomWindow->GetFrames(getter_AddRefs(oDomWindowCol));
    RvTestResult(rv, "nsIDOMWindow::GetFrames()' rv test", 1);
	if (!oDomWindowCol)
	{
		AfxMessageBox("Cannot create Dom Window Collection Object");
		return;
	}

	rv = oDomWindow->GetDocument(getter_AddRefs(oDomDocument));
    RvTestResult(rv, "nsIDOMWindow::GetDocument()' rv test", 1);
	if (!oDomDocument)
	{
		AfxMessageBox("Cannot create Dom Document Object");
		return;
	}

	rv = oDomWindow->GetSelection(getter_AddRefs(oSelection));
    RvTestResult(rv, "nsIDOMWindow::GetSelection()' rv test", 1);
	if (!oSelection)
	{
		AfxMessageBox("Cannot get the  Selection Object");
		return;
	}

	//  nsIDOMBarProp's only attribute
	//rv = oDomBarProp->GetVisible(&bVisible);


	rv = oDomWindow->GetTextZoom(&fTextZoom);
	    RvTestResult(rv, "nsIDOMWindow::GetTextZoom()' rv test", 1);

	rv = oDomWindow->SetTextZoom(12.0);
	//fTextZoom = 4.0 ;
	rv = oDomWindow->SetTextZoom(fTextZoom);
	    RvTestResult(rv, "nsIDOMWindow::SetTextZoom()' rv test", 1);

	rv = oDomWindow->GetScrollX(&scrollX);
	   RvTestResult(rv, "nsIDOMWindow::GetScrollX()' rv test", 1);

	rv = oDomWindow->GetScrollY(&scrollY);
	   RvTestResult(rv, "nsIDOMWindow::GetScrollY()' rv test", 1);

	rv = oDomWindow->ScrollTo(100,100);
	   RvTestResult(rv, "nsIDOMWindow::ScrollTo()' rv test", 1);


	rv = oDomWindow->ScrollBy(5, 5);
	   RvTestResult(rv, "nsIDOMWindow::ScrollBy()' rv test", 1);

	rv = oDomWindow->ScrollByLines(5);
	   RvTestResult(rv, "nsIDOMWindow::ScrollByLines()' rv test", 1);

	rv = oDomWindow->ScrollByPages(1);
	   RvTestResult(rv, "nsIDOMWindow::ScrollByPages()' rv test", 1);

	//rv = oDomWindow->SizeToContent();
	//   RvTestResult(rv, "nsIDOMWindow::SizeToContent()' rv test", 1);

}

void CDomWindow::GetTop()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	nsCOMPtr<nsIDOMWindow> oDomWindowTop;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);
	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}

    rv = oDomWindow->GetTop(getter_AddRefs(oDomWindowTop));
    RvTestResult(rv, "nsIDOMWindow::GetTop()' rv test", 1);
	if (!oDomWindowTop)
	{
		AfxMessageBox("Cannot create Dom Window Top Object");
		return;
	}
}

void CDomWindow::GetParent()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	nsCOMPtr<nsIDOMWindow> oDomWindowParent;

	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);
	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}

	rv = oDomWindow->GetParent(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIDOMWindow::GetParent()' rv test", 0);
	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Parent Object");
		return;
	}

}


void CDomWindow::GetScrollbars()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	nsCOMPtr<nsIDOMBarProp> oDomBarProp;

	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);
	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}

	rv = oDomWindow->GetScrollbars(getter_AddRefs(oDomBarProp));
    RvTestResult(rv, "nsIDOMWindow::GetScrollbars()' rv test", 0);
	if (!oDomBarProp)
	{
		AfxMessageBox("Cannot create Dom Window Scroll Bar Object");
		return;
	}

}

void CDomWindow::GetFrames()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	nsCOMPtr<nsIDOMWindowCollection> oDomWindowCol;

	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);
	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->GetFrames(getter_AddRefs(oDomWindowCol));
    RvTestResult(rv, "nsIDOMWindow::GetFrames()' rv test", 0);
	if (!oDomWindowCol)
	{
		AfxMessageBox("Cannot create Dom Window Collection Object");
		return;
	}
}

void CDomWindow::GetDocument()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	nsCOMPtr<nsIDOMDocument> oDomDocument;

	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);
	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}

	rv = oDomWindow->GetDocument(getter_AddRefs(oDomDocument));
    RvTestResult(rv, "nsIDOMWindow::GetDocument()' rv test",0);
	if (!oDomDocument)
	{
		AfxMessageBox("Cannot create Dom Document Object");
		return;
	}
}

void CDomWindow::GetSelection()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	nsCOMPtr<nsISelection> oSelection;

	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test",0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}

	rv = oDomWindow->GetSelection(getter_AddRefs(oSelection));
    RvTestResult(rv, "nsIDOMWindow::GetSelection()' rv test", 0);
	if (!oSelection)
	{
		AfxMessageBox("Cannot get the  Selection Object");
		return;
	}
}

void CDomWindow::GetTextZoom()
{
	float fTextZoom = 0.0 ;
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->GetTextZoom(&fTextZoom);
	RvTestResult(rv, "nsIDOMWindow::GetTextZoom()' rv test", 0);
}

void CDomWindow::SetTextZoom()
{
	float fTextZoom = 0.0 ;

	fTextZoom = 12.0;
	rv = GetDOMOWindowObject()->SetTextZoom(fTextZoom);
	RvTestResult(rv, "nsIDOMWindow::SetTextZoom()' rv test", 0);
}

void CDomWindow::GetScrollX()
{
	PRInt32 scrollX = 0 ;
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->GetScrollX(&scrollX);
	RvTestResult(rv, "nsIDOMWindow::GetScrollX()' rv test", 0);
}

void CDomWindow::GetScrollY()
{	
	PRInt32 scrollY = 0 ;
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->GetScrollY(&scrollY);
	RvTestResult(rv, "nsIDOMWindow::GetScrollY()' rv test", 0);
}

void CDomWindow::ScrollTo()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->ScrollTo(100,100);
    RvTestResult(rv, "nsIDOMWindow::ScrollTo()' rv test", 0);
}


void CDomWindow::ScrollBy()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->ScrollBy(5, 5);
	RvTestResult(rv, "nsIDOMWindow::ScrollBy()' rv test", 0);
}

void CDomWindow::ScrollByLines()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->ScrollByLines(5);
    RvTestResult(rv, "nsIDOMWindow::ScrollByLines()' rv test",0);

}

void CDomWindow::ScrollByPages()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->ScrollByPages(1);
	RvTestResult(rv, "nsIDOMWindow::ScrollByPages()' rv test", 0);
}

void CDomWindow::SizeToContent()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
    RvTestResult(rv, "nsIWebBrowser::GetContentDOMWindow()' rv test", 0);

	if (!oDomWindow)
	{
		AfxMessageBox("Cannot create Dom Window Object");
		return;
	}
	rv = oDomWindow->SizeToContent();
	RvTestResult(rv, "nsIDOMWindow::SizeToContent()' rv test", 0);
}


nsIDOMWindow* CDomWindow::GetDOMOWindowObject()
{
	nsCOMPtr<nsIDOMWindow> oDomWindow;
	rv = qaWebBrowser->GetContentDOMWindow(getter_AddRefs(oDomWindow));
	if (!oDomWindow)
		AfxMessageBox("Cannot create Dom Window Object");
	return oDomWindow;

}
