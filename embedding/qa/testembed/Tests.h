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
 *   David Epstein <depstein@netscape.com>
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

// Tests.h : header file for QA test cases
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _TESTS_H
#define _TESTS_H

#if _MSC_VER > 1000
	#pragma once
#endif

#include "BrowserView.h"
#include "BrowserImpl.h"
#include "StdAfx.h"
#include "UrlDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CTESTS class

class CBrowserImpl;
class CBrowserView;

class CTests:public CWnd
{
public:
	CTests(nsIWebBrowser* mWebBrowser,
			   nsIBaseWindow* mBaseWindow,
			   nsIWebNavigation* mWebNav,
			   CBrowserImpl *mpBrowserImpl);
	virtual ~CTests();

	// Some helper methods

	// Mozilla interfaces
	//
	//static 
	nsCOMPtr<nsIWebBrowser> qaWebBrowser;
	nsCOMPtr<nsIBaseWindow> qaBaseWindow;
	nsCOMPtr<nsIWebNavigation> qaWebNav;
	CBrowserImpl	*qaBrowserImpl;
	CUrlDialog myDialog;

	// local test methods

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTests)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

private:
	// Individual interface Objects

	// local test variables
	PRBool exists;
	nsCOMPtr<nsIURI> theURI;
	nsCOMPtr<nsIChannel> theChannel;
	UINT nCommandID;
protected:
	//{{AFX_MSG(CTests)
	afx_msg void OnUpdateNavBack(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNavForward(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNavStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCut(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCopy(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePaste(CCmdUI* pCmdUI);
	afx_msg void OnTestsChangeUrl();
	afx_msg void OnTestsGlobalHistory();
	afx_msg void OnTestsCreateFile();
	afx_msg void OnTestsCreateprofile();
	afx_msg void OnTestsAddTooltipListener();
	afx_msg void OnTestsAddWebProgListener();
	afx_msg void OnTestsAddHistoryListener();
	afx_msg void OnTestsRemovehistorylistener();
	afx_msg void OnTestsAddUriContentListenerByWebBrowser();
	afx_msg void OnTestsAddUriContentListenerByUriLoader();
	afx_msg void OnTestsAddUriContentListenerByOpenUri();
	afx_msg void OnTestsNSNewChannelAndAsyncOpen();
	afx_msg void OnTestsIOServiceNewURI();
	afx_msg void OnTestsProtocolHandlerNewURI();
	afx_msg void OnInterfacesNsifile();
	afx_msg void OnToolsRemoveGHPage();
	afx_msg void OnToolsRemoveAllGH();
	afx_msg void OnToolsViewLogfile();
	afx_msg void OnToolsDeleteLogfile();
	afx_msg void OnToolsTestYourMethod();
	afx_msg void OnToolsTestYourMethod2();
	afx_msg void OnVerifybugs70228();
	afx_msg void OnVerifybugs90195();
	afx_msg void OnVerifybugs169617();
	afx_msg void OnVerifybugs170274();
    afx_msg void OnPasteTest();
    afx_msg void OnCopyTest();
    afx_msg void OnSelectAllTest();
    afx_msg void OnSelectNoneTest();
    afx_msg void OnCutSelectionTest();
    afx_msg void copyLinkLocationTest();
    afx_msg void canCopySelectionTest();
    afx_msg void canCutSelectionTest();
    afx_msg void canPasteTest();
	afx_msg void OnInterfacesNsirequest();
	afx_msg void OnInterfacesNsidomwindow();
	afx_msg void OnInterfacesNsidirectoryservice();
	afx_msg void OnInterfacesNsiselection();
	afx_msg void OnInterfacesNsiprofile();
	afx_msg void OnInterfacesNsishistory();
	afx_msg void OnInterfacesNsiwebnav();
	afx_msg void OnInterfacesNsiclipboardcommands();
	afx_msg void OnInterfacesNsiobserverservice();
	afx_msg void OnInterfacesNsiwebbrowser();
	afx_msg void OnInterfacesNsiwebprogress();
	afx_msg void OnInterfacesNsiwebbrowfind();
	afx_msg void OnInterfacesNsieditingsession();
	afx_msg void OnInterfacesNsicommandmgr();
	afx_msg void OnInterfacesNsicmdparams();
	afx_msg void OnInterfacesRunalltestcases();
	afx_msg void OnInterfacesNsichannel();
	afx_msg void OnInterfacesNsihttpchannel();

	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	// individual nsIFile tests
	void InitWithPathTest(nsILocalFile *);
	void AppendRelativePathTest(nsILocalFile *);
	void FileCreateTest(nsILocalFile *);
	void FileExistsTest(nsILocalFile *);
	void FileCopyTest(nsILocalFile *, nsILocalFile *);
	void FileMoveTest(nsILocalFile *, nsILocalFile *);
};

#endif //_TESTS_H

