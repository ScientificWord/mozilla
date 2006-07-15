/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: Mozilla-sample-code 1.0
 *
 * Copyright (c) 2002 Netscape Communications Corporation and
 * other contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this Mozilla sample software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/* PromptService is intended to override the default Mozilla PromptService,
   giving nsIPrompt implementations of our own design, rather than using
   Mozilla's. Do this by building this into a component and registering the
   factory with the same CID/ContractID as Mozilla's (see MfcEmbed.cpp).
*/

#include "stdafx.h"
#include "Dialogs.h"
#include "PromptService.h"
#include "nsCOMPtr.h"
#include "nsMemory.h"
#include "nsString.h"
#include "nsReadableUtils.h"
#include "nsIDOMWindow.h"
#include "nsIEmbeddingSiteWindow.h"
#include "nsIFactory.h"
#include "nsIPromptService.h"
#include "nsIServiceManager.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWindowWatcher.h"

static HINSTANCE gInstance;

//*****************************************************************************
// ResourceState
//***************************************************************************** 

class ResourceState {
public:
  ResourceState() {
    mPreviousInstance = ::AfxGetResourceHandle();
    ::AfxSetResourceHandle(gInstance);
  }
  ~ResourceState() {
    ::AfxSetResourceHandle(mPreviousInstance);
  }
private:
  HINSTANCE mPreviousInstance;
};


//*****************************************************************************
// CPromptService
//*****************************************************************************

class CPromptService: public nsIPromptService {
public:
                 CPromptService();
  virtual       ~CPromptService();

  NS_DECL_ISUPPORTS
  NS_DECL_NSIPROMPTSERVICE

private:
  nsCOMPtr<nsIWindowWatcher> mWWatch;
  CWnd *CWndForDOMWindow(nsIDOMWindow *aWindow);
};

//*****************************************************************************

NS_IMPL_ISUPPORTS1(CPromptService, nsIPromptService)

CPromptService::CPromptService() :
  mWWatch(do_GetService(NS_WINDOWWATCHER_CONTRACTID))
{
}

CPromptService::~CPromptService() {
}

CWnd *
CPromptService::CWndForDOMWindow(nsIDOMWindow *aWindow)
{
  nsCOMPtr<nsIWebBrowserChrome> chrome;
  CWnd *val = 0;

  if (mWWatch) {
    nsCOMPtr<nsIDOMWindow> fosterParent;
    if (!aWindow) { // it will be a dependent window. try to find a foster parent.
      mWWatch->GetActiveWindow(getter_AddRefs(fosterParent));
      aWindow = fosterParent;
    }
    mWWatch->GetChromeForWindow(aWindow, getter_AddRefs(chrome));
  }

  if (chrome) {
    nsCOMPtr<nsIEmbeddingSiteWindow> site(do_QueryInterface(chrome));
    if (site) {
      HWND w;
      site->GetSiteWindow(reinterpret_cast<void **>(&w));
      val = CWnd::FromHandle(w);
    }
  }

  return val;
}

NS_IMETHODIMP CPromptService::Alert(nsIDOMWindow *parent, const PRUnichar *dialogTitle,
                                    const PRUnichar *text)
{
  USES_CONVERSION;
  CWnd *wnd = CWndForDOMWindow(parent);
  if (wnd)
    wnd->MessageBox(W2CT(text), W2CT(dialogTitle), MB_OK | MB_ICONEXCLAMATION);
  else
    ::MessageBox(0, W2CT(text), W2CT(dialogTitle), MB_OK | MB_ICONEXCLAMATION);

  return NS_OK;
}

NS_IMETHODIMP CPromptService::AlertCheck(nsIDOMWindow *parent,
                                         const PRUnichar *dialogTitle,
                                         const PRUnichar *text,
                                         const PRUnichar *checkboxMsg,
                                         PRBool *checkValue)
{
  ResourceState setState;
  USES_CONVERSION;

  CWnd *wnd = CWndForDOMWindow(parent);
  CAlertCheckDialog dlg(wnd, W2CT(dialogTitle), W2CT(text),
                    W2CT(checkboxMsg), checkValue ? *checkValue : 0);

  dlg.DoModal();

  *checkValue = dlg.m_bCheckBoxValue;

  return NS_OK;
}

NS_IMETHODIMP CPromptService::Confirm(nsIDOMWindow *parent,
                                      const PRUnichar *dialogTitle,
                                      const PRUnichar *text,
                                      PRBool *_retval)
{
  USES_CONVERSION;
  CWnd *wnd = CWndForDOMWindow(parent);
  int choice;

  if (wnd)
    choice = wnd->MessageBox(W2CT(text), W2CT(dialogTitle),
                      MB_YESNO | MB_ICONEXCLAMATION);
  else
    choice = ::MessageBox(0, W2CT(text), W2CT(dialogTitle),
                      MB_YESNO | MB_ICONEXCLAMATION);

  *_retval = choice == IDYES ? PR_TRUE : PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP CPromptService::ConfirmCheck(nsIDOMWindow *parent,
                                           const PRUnichar *dialogTitle,
                                           const PRUnichar *text,
                                           const PRUnichar *checkboxMsg,
                                           PRBool *checkValue,
                                           PRBool *_retval)
{
    ResourceState setState;
    USES_CONVERSION;

    CWnd *wnd = CWndForDOMWindow(parent);
    CConfirmCheckDialog dlg(wnd, W2CT(dialogTitle), W2CT(text),
                    W2CT(checkboxMsg), checkValue ? *checkValue : 0,
                    _T("Yes"), _T("No"), NULL);

    int iBtnClicked = dlg.DoModal();

    *checkValue = dlg.m_bCheckBoxValue;

    *_retval = iBtnClicked == 0 ? PR_TRUE : PR_FALSE;

    return NS_OK;
}

NS_IMETHODIMP CPromptService::Prompt(nsIDOMWindow *parent,
                                     const PRUnichar *dialogTitle,
                                     const PRUnichar *text,
                                     PRUnichar **value,
                                     const PRUnichar *checkboxMsg,
                                     PRBool *checkValue,
                                     PRBool *_retval)
{
  ResourceState setState;
  USES_CONVERSION;

  CWnd *wnd = CWndForDOMWindow(parent);
  CPromptDialog dlg(wnd, W2CT(dialogTitle), W2CT(text),
                    text && *text ? W2CT(text) : 0,
                    checkValue != nsnull, W2CT(checkboxMsg),
                    checkValue ? *checkValue : 0);
  if(dlg.DoModal() == IDOK) {
    // Get the value entered in the editbox of the PromptDlg
    if (value && *value) {
      nsMemory::Free(*value);
      *value = nsnull;
    }
    USES_CONVERSION;
    nsString csPromptEditValue;
    csPromptEditValue.Assign(T2CW(dlg.m_csPromptAnswer.GetBuffer(0)));

    *value = ToNewUnicode(csPromptEditValue);

    *_retval = PR_TRUE;
  } else
    *_retval = PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP CPromptService::PromptUsernameAndPassword(nsIDOMWindow *parent,
                                                        const PRUnichar *dialogTitle,
                                                        const PRUnichar *text,
                                                        PRUnichar **username,
                                                        PRUnichar **password,
                                                        const PRUnichar *checkboxMsg,
                                                        PRBool *checkValue,
                                                        PRBool *_retval)
{
  ResourceState setState;
  USES_CONVERSION;

  CWnd *wnd = CWndForDOMWindow(parent);
  CPromptUsernamePasswordDialog dlg(wnd, W2CT(dialogTitle), W2CT(text),
                    username && *username ? W2CT(*username) : 0,
                    password && *password ? W2CT(*password) : 0, 
                    checkValue != nsnull, W2CT(checkboxMsg),
                    checkValue ? *checkValue : 0);

  if (dlg.DoModal() == IDOK) {
    // Get the username entered
    if (username && *username) {
        nsMemory::Free(*username);
        *username = nsnull;
    }
    USES_CONVERSION;
    nsString csUserName;
    csUserName.Assign(T2CW(dlg.m_csUserName.GetBuffer(0)));
    *username = ToNewUnicode(csUserName);

    // Get the password entered
    if (password && *password) {
      nsMemory::Free(*password);
      *password = nsnull;
    }
    nsString csPassword;
    csPassword.Assign(T2CW(dlg.m_csPassword.GetBuffer(0)));
    *password = ToNewUnicode(csPassword);

    if(checkValue)		
      *checkValue = dlg.m_bCheckBoxValue;

    *_retval = PR_TRUE;
  } else
    *_retval = PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP CPromptService::PromptPassword(nsIDOMWindow *parent,
                                             const PRUnichar *dialogTitle,
                                             const PRUnichar *text,
                                             PRUnichar **password,
                                             const PRUnichar *checkboxMsg,
                                             PRBool *checkValue,
                                             PRBool *_retval)
{
  ResourceState setState;
  USES_CONVERSION;

  CWnd *wnd = CWndForDOMWindow(parent);
  CPromptPasswordDialog dlg(wnd, W2CT(dialogTitle), W2CT(text),
                            password && *password ? W2CT(*password) : 0,
                            checkValue != nsnull, W2CT(checkboxMsg),
                            checkValue ? *checkValue : 0);
  if(dlg.DoModal() == IDOK) {
    // Get the password entered
    if (password && *password) {
        nsMemory::Free(*password);
        *password = nsnull;
    }
    USES_CONVERSION;
    nsString csPassword;
    csPassword.Assign(T2CW(dlg.m_csPassword.GetBuffer(0)));
    *password = ToNewUnicode(csPassword);

    if(checkValue)
      *checkValue = dlg.m_bCheckBoxValue;

    *_retval = PR_TRUE;
  } else
    *_retval = PR_FALSE;

  return NS_OK;
}

NS_IMETHODIMP CPromptService::Select(nsIDOMWindow *parent,
                                     const PRUnichar *dialogTitle,
                                     const PRUnichar *text, PRUint32 count,
                                     const PRUnichar **selectList,
                                     PRInt32 *outSelection,
                                     PRBool *_retval)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CPromptService::ConfirmEx(nsIDOMWindow *parent,
                                        const PRUnichar *dialogTitle,
                                        const PRUnichar *text,
                                        PRUint32 buttonFlags,
                                        const PRUnichar *button0Title,
                                        const PRUnichar *button1Title,
                                        const PRUnichar *button2Title,
                                        const PRUnichar *checkMsg,
                                        PRBool *checkValue,
                                        PRInt32 *buttonPressed)
{
    ResourceState setState;
    USES_CONVERSION;

    // First, determine the button titles based on buttonFlags
    const PRUnichar* buttonStrings[] = { button0Title, button1Title, button2Title };
    CString csBtnTitles[3];

    for(int i=0; i<3; i++)
    {
        switch(buttonFlags & 0xff) {
            case BUTTON_TITLE_OK:
                csBtnTitles[i] = "Ok";
                break;
            case BUTTON_TITLE_CANCEL:
                csBtnTitles[i] = "Cancel";
                break;
            case BUTTON_TITLE_YES:
                csBtnTitles[i] = "Yes";
                break;
            case BUTTON_TITLE_NO:
                csBtnTitles[i] = "No";
                break;
            case BUTTON_TITLE_SAVE:
                csBtnTitles[i] = "Save";
                break;
            case BUTTON_TITLE_DONT_SAVE:
                csBtnTitles[i] = "DontSave";
                break;
            case BUTTON_TITLE_REVERT:
                csBtnTitles[i] = "Revert";
                break;
            case BUTTON_TITLE_IS_STRING:
                csBtnTitles[i] = W2CT(buttonStrings[i]);
                break;
        }
   
        buttonFlags >>= 8;    
    }

    CWnd *wnd = CWndForDOMWindow(parent);
    CConfirmCheckDialog dlg(wnd, W2CT(dialogTitle), W2CT(text),
        checkMsg ? W2CT(checkMsg) : NULL, checkValue ? *checkValue : 0,
                    csBtnTitles[0].IsEmpty() ? NULL : (LPCTSTR)csBtnTitles[0], 
                    csBtnTitles[1].IsEmpty() ? NULL : (LPCTSTR)csBtnTitles[1], 
                    csBtnTitles[2].IsEmpty() ? NULL : (LPCTSTR)csBtnTitles[2]);

    *buttonPressed = dlg.DoModal();

    if(checkValue)
        *checkValue = dlg.m_bCheckBoxValue;

    return NS_OK;    
}
 
//*****************************************************************************
// CPromptServiceFactory
//*****************************************************************************   

class CPromptServiceFactory : public nsIFactory {
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIFACTORY

  CPromptServiceFactory();
  virtual ~CPromptServiceFactory();
};

//*****************************************************************************

NS_IMPL_ISUPPORTS1(CPromptServiceFactory, nsIFactory)

CPromptServiceFactory::CPromptServiceFactory() {
}

CPromptServiceFactory::~CPromptServiceFactory() {
}

NS_IMETHODIMP CPromptServiceFactory::CreateInstance(nsISupports *aOuter, const nsIID & aIID, void **aResult)
{
  NS_ENSURE_ARG_POINTER(aResult);
  
  *aResult = NULL;  
  CPromptService *inst = new CPromptService;    
  if (!inst)
    return NS_ERROR_OUT_OF_MEMORY;
    
  nsresult rv = inst->QueryInterface(aIID, aResult);
  if (rv != NS_OK) {  
    // We didn't get the right interface, so clean up  
    delete inst;  
  }  
    
  return rv;
}

NS_IMETHODIMP CPromptServiceFactory::LockFactory(PRBool lock)
{
  return NS_OK;
}

//*****************************************************************************

void InitPromptService(HINSTANCE instance) {

  gInstance = instance;
}

nsresult NS_NewPromptServiceFactory(nsIFactory** aFactory)
{
  NS_ENSURE_ARG_POINTER(aFactory);
  *aFactory = nsnull;
  
  CPromptServiceFactory *result = new CPromptServiceFactory;
  if (!result)
    return NS_ERROR_OUT_OF_MEMORY;
    
  NS_ADDREF(result);
  *aFactory = result;
  
  return NS_OK;
}
