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
 *   Ryan Cassin <rcassin@supernova.org>
 *   Daniel Glazman <glazman@netscape.com>
 *   Charles Manske <cmanske@netscape.com>
 *   Kathleen Brade <brade@netscape.com>
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


#include "nsIEditor.h"
#include "nsIHTMLEditor.h"
#include "nsIHTMLAbsPosEditor.h"

#include "nsIDOMElement.h"
#include "nsIAtom.h"

#include "nsIClipboard.h"

#include "nsCOMPtr.h"

#include "nsComposerCommands.h"
#include "nsReadableUtils.h"
#include "nsUnicharUtils.h"
#include "nsICommandParams.h"
#include "nsComponentManagerUtils.h"
#include "nsCRT.h"
#include "nsISelection.h"
#include "nsISelectionController.h"
#include "msiITagListManager.h"

//prototype
nsresult GetListState(nsIEditor *aEditor, nsString &itemTag, PRBool *aMixed, nsString & _retval);

nsresult RemoveOneProperty(nsIHTMLEditor *aEditor,const nsString& aProp,
                           const nsString &aAttr);
nsresult RemoveTextProperty(nsIEditor *aEditor, const PRUnichar *prop,
                            const PRUnichar *attr);
nsresult SetTextProperty(nsIEditor *aEditor, const PRUnichar *prop,
                         const PRUnichar *attr, const PRUnichar *value);


//defines
#define STATE_ENABLED  "state_enabled"
#define STATE_ALL "state_all"
#define STATE_ANY "state_any"
#define STATE_MIXED "state_mixed"
#define STATE_BEGIN "state_begin"
#define STATE_END "state_end"
#define STATE_ATTRIBUTE "state_attribute"
#define STATE_DATA "state_data"


nsBaseComposerCommand::nsBaseComposerCommand()
{
}

NS_IMPL_ISUPPORTS1(nsBaseComposerCommand, nsIControllerCommand)


nsBaseStateUpdatingCommand::nsBaseStateUpdatingCommand(const char* aTagName)
: nsBaseComposerCommand()
{
  mTagName.AssignWithConversion(aTagName);
}

nsBaseStateUpdatingCommand::~nsBaseStateUpdatingCommand()
{
}

NS_IMPL_ISUPPORTS_INHERITED0(nsBaseStateUpdatingCommand, nsBaseComposerCommand)

NS_IMETHODIMP
nsBaseStateUpdatingCommand::IsCommandEnabled(const char *aCommandName,
                                             nsISupports *refCon,
                                             PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
nsBaseStateUpdatingCommand::DoCommand(const char *aCommandName,
                                      nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;

  return ToggleState(editor, mTagName);
}

NS_IMETHODIMP
nsBaseStateUpdatingCommand::DoCommandParams(const char *aCommandName,
                                            nsICommandParams *aParams,
                                            nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsBaseStateUpdatingCommand::GetCommandStateParams(const char *aCommandName,
                                                  nsICommandParams *aParams,
                                                  nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (editor)
    return GetCurrentState(editor, mTagName, aParams);

  return NS_OK;
}
nsBaseTagUpdatingCommand::nsBaseTagUpdatingCommand(void)
: nsBaseComposerCommand()
{
}

nsBaseTagUpdatingCommand::~nsBaseTagUpdatingCommand()
{
}

NS_IMPL_ISUPPORTS_INHERITED0(nsBaseTagUpdatingCommand, nsBaseComposerCommand)

NS_IMETHODIMP
nsBaseTagUpdatingCommand::IsCommandEnabled(const char *aCommandName,
                                             nsISupports *refCon,
                                             PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  *outCmdEnabled = PR_FALSE;
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(refCon);
  if (htmlEditor)
  {
    *outCmdEnabled = PR_TRUE;
  }
  return NS_OK;
}


NS_IMETHODIMP
nsBaseTagUpdatingCommand::DoCommand(const char *aCommandName,
                                      nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;

  return ToggleState(editor, mTagName);
}

NS_IMETHODIMP
nsBaseTagUpdatingCommand::DoCommandParams(const char *aCommandName,
                                            nsICommandParams *aParams,
                                            nsISupports *refCon)
{
  nsXPIDLString tagName;
  nsresult rv;
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;
  if (aParams) {
    nsXPIDLCString s;
    rv = aParams->GetCStringValue(STATE_ATTRIBUTE, getter_Copies(s));
    if (NS_SUCCEEDED(rv))
      tagName.AssignWithConversion(s);
    else
      rv = aParams->GetStringValue(STATE_ATTRIBUTE, tagName);
  }
  return ToggleState(editor, tagName);
}

NS_IMETHODIMP
nsBaseTagUpdatingCommand::GetCommandStateParams(const char *aCommandName,
                                                  nsICommandParams *aParams,
                                                  nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;
  return GetCurrentState(editor, aParams);

  return NS_OK;
}

nsresult
nsBaseTagUpdatingCommand::GetCurrentTagState(nsIEditor *aEditor, nsString& aTagClass,
                                        nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_NOT_INITIALIZED;

  nsresult rv = NS_OK;
  nsString strTagName;

  PRBool firstOfSelectionHasProp = PR_TRUE;
  PRBool anyOfSelectionHasProp = PR_TRUE;
  PRBool allOfSelectionHasProp = PR_TRUE;

  rv = htmlEditor->GetInnermostTag(  aTagClass,
                                     &firstOfSelectionHasProp,
                                     &anyOfSelectionHasProp,
                                     &allOfSelectionHasProp,
                                     strTagName);
  aParams->SetStringValue(STATE_ATTRIBUTE, strTagName);
  aParams->SetBooleanValue(STATE_ENABLED, NS_SUCCEEDED(rv));
  aParams->SetBooleanValue(STATE_ALL, allOfSelectionHasProp);
  aParams->SetBooleanValue(STATE_ANY, anyOfSelectionHasProp);
  aParams->SetBooleanValue(STATE_MIXED, anyOfSelectionHasProp
           && !allOfSelectionHasProp);
  aParams->SetBooleanValue(STATE_BEGIN, firstOfSelectionHasProp);
  aParams->SetBooleanValue(STATE_END, allOfSelectionHasProp);//not completely accurate
  return NS_OK;
}


nsParaTagUpdatingCommand::nsParaTagUpdatingCommand(void)
: nsBaseTagUpdatingCommand()
{
}

nsresult
nsParaTagUpdatingCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  if (newState.Length() > 0) return htmlEditor->SetParagraphFormat(newState);
  return NS_OK; //BBM should be bad data
}

NS_IMETHODIMP
nsParaTagUpdatingCommand::DoCommand(const char *aCommandName,
                                      nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;

  return SetState(editor, mTagName);
}

NS_IMETHODIMP
nsParaTagUpdatingCommand::DoCommandParams(const char *aCommandName,
                                            nsICommandParams *aParams,
                                            nsISupports *refCon)
{
  nsXPIDLString tagName;
  nsresult rv;
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;
  if (aParams) {
    nsXPIDLCString s;
    rv = aParams->GetCStringValue(STATE_ATTRIBUTE, getter_Copies(s));
    if (NS_SUCCEEDED(rv))
      tagName.AssignWithConversion(s);
    else
      rv = aParams->GetStringValue(STATE_ATTRIBUTE, tagName);
  }
 // if (!tagName.
  return SetState(editor, tagName);
}

/*****/

nsFrontMTagUpdatingCommand::nsFrontMTagUpdatingCommand(void)
: nsBaseTagUpdatingCommand()
{
}

nsresult
nsFrontMTagUpdatingCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;
  nsCOMPtr<msiITagListManager> tlmgr;
  htmlEditor->GetTagListManager( getter_AddRefs(tlmgr));

  if (newState.Length() > 0)
  {
    nsAutoString  realclass;
    tlmgr->GetStringPropertyForTag( newState, nsnull, NS_LITERAL_STRING("realtagclass"), realclass);
    if (realclass.EqualsLiteral("paratag"))
    {
      return htmlEditor->SetParagraphFormat(newState);
    }
    else if (realclass.EqualsLiteral("othertag"))
    {
      nsCOMPtr<nsIDOMElement> element;
      htmlEditor->CreateElementWithDefaults(newState, getter_AddRefs(element));
      return htmlEditor->InsertElementAtSelection(element, true);
    }
    // env or structure -- we assume text tags never arise here.
    else {
      return htmlEditor->SetStructureTag(newState);
    }
  }
  return NS_OK; //BBM should be bad data
}

NS_IMETHODIMP
nsFrontMTagUpdatingCommand::DoCommand(const char *aCommandName,
                                      nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;

  return SetState(editor, mTagName);
}

NS_IMETHODIMP
nsFrontMTagUpdatingCommand::DoCommandParams(const char *aCommandName,
                                            nsICommandParams *aParams,
                                            nsISupports *refCon)
{
  nsXPIDLString tagName;
  nsresult rv;
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;
  if (aParams) {
    nsXPIDLCString s;
    rv = aParams->GetCStringValue(STATE_ATTRIBUTE, getter_Copies(s));
    if (NS_SUCCEEDED(rv))
      tagName.AssignWithConversion(s);
    else
      rv = aParams->GetStringValue(STATE_ATTRIBUTE, tagName);
  }
 // if (!tagName.
  return SetState(editor, tagName);
}


nsListTagUpdatingCommand::nsListTagUpdatingCommand(void)
: nsBaseTagUpdatingCommand()
{
}


nsresult
nsListTagUpdatingCommand::GetCurrentState(nsIEditor *aEditor,
                               nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need editor here");

  PRBool bMixed;
  nsAutoString tagStr;
  nsresult rv = GetListState(aEditor, mTagName, &bMixed, tagStr);
  if (NS_FAILED(rv)) return rv;

  // Need to use mTagName????
  PRBool inList = tagStr.Equals(mTagName);
  aParams->SetBooleanValue(STATE_ALL, !bMixed && inList);
  aParams->SetBooleanValue(STATE_MIXED, bMixed);
  aParams->SetBooleanValue(STATE_ENABLED, PR_TRUE);
  return NS_OK;
}



nsresult
nsListTagUpdatingCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_NOT_INITIALIZED;

  PRBool inList;
  nsresult rv;
  nsCOMPtr<nsICommandParams> params =
      do_CreateInstance(NS_COMMAND_PARAMS_CONTRACTID,&rv);
  if (NS_FAILED(rv) || !params)
    return rv;
  rv = GetCurrentState(aEditor, /*newState,*/ params);
  rv = params->GetBooleanValue(STATE_ALL,&inList);
  if (NS_FAILED(rv))
    return rv;
  if (NS_FAILED(rv)) return rv;

  if (inList)
  {
    // To remove a list, first get what kind of list we're in
    PRBool bMixed;
    nsAutoString tagStr;
    nsAutoString itemTagName;
    rv = GetListState(aEditor, mTagName, &bMixed, tagStr);
    if (NS_FAILED(rv)) return rv;
    if (!tagStr.IsEmpty())
    {
      if (!bMixed)
      {
        rv = htmlEditor->RemoveList(nsDependentString(tagStr));
      }
    }
  }
  else
  {
    nsAutoString itemType; itemType.Assign(mTagName);
    // Set to the requested paragraph type
    //XXX Note: This actually doesn't work for "LI",
    //    but we currently don't use this for non DL lists anyway.
    // Problem: won't this replace any current block paragraph style?
    rv = htmlEditor->SetParagraphFormat(itemType);
  }

  return rv;
}

nsresult
nsListTagUpdatingCommand::ToggleState(nsIEditor *aEditor, nsString & aTagName)
{
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(aEditor);
  if (!editor)
    return NS_NOINTERFACE;
  PRBool inList;
  // Need to use mTagName????
  nsresult rv;
  nsCOMPtr<nsICommandParams> params =
      do_CreateInstance(NS_COMMAND_PARAMS_CONTRACTID,&rv);
  if (NS_FAILED(rv) || !params)
    return rv;

  rv = GetCurrentState(aEditor, params);
//  rv = GetCurrentState(aEditor, mTagName, params);//
  rv = params->GetBooleanValue(STATE_ALL,&inList);
  if (NS_FAILED(rv))
    return rv;

  if (inList)
    rv = editor->RemoveList(mTagName);
  else
  {
    rv = editor->MakeOrChangeList(mTagName, PR_FALSE, EmptyString());
  }

  return rv;
}



NS_IMETHODIMP
nsListTagUpdatingCommand::DoCommand(const char *aCommandName,
                                      nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;
  return ToggleState(editor,mTagName);
}

NS_IMETHODIMP
nsListTagUpdatingCommand::DoCommandParams(const char *aCommandName,
                                            nsICommandParams *aParams,
                                            nsISupports *refCon)
{
  nsXPIDLString tagName;
  nsresult rv;
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;
  if (aParams) {
    nsXPIDLCString s;
    rv = aParams->GetCStringValue(STATE_ATTRIBUTE, getter_Copies(s));
    if (NS_SUCCEEDED(rv))
      tagName.AssignWithConversion(s);
    else
      rv = aParams->GetStringValue(STATE_ATTRIBUTE, tagName);
  }
  mTagName.Assign(tagName);
  return ToggleState(editor, tagName);
}

nsStructTagUpdatingCommand::nsStructTagUpdatingCommand(void)
: nsBaseTagUpdatingCommand()
{
}

NS_IMETHODIMP
nsStructTagUpdatingCommand::DoCommand(const char *aCommandName,
                                      nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;

  return SetState(editor, mTagName);
}

NS_IMETHODIMP
nsStructTagUpdatingCommand::DoCommandParams(const char *aCommandName,
                                            nsICommandParams *aParams,
                                            nsISupports *refCon)
{
  nsXPIDLString tagName;
  nsresult rv;
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;
  if (aParams) {
    nsXPIDLCString s;
    rv = aParams->GetCStringValue(STATE_ATTRIBUTE, getter_Copies(s));
    if (NS_SUCCEEDED(rv))
      tagName.AssignWithConversion(s);
    else
      rv = aParams->GetStringValue(STATE_ATTRIBUTE, tagName);
  }
  return SetState(editor, tagName);
}

nsresult
nsEnvTagUpdatingCommand::ToggleState(nsIEditor *aEditor, nsString& aTagName)
{
//  printf("nsEnvTagUpdatingCommand::ToggleState(%S)\n",aTagName.get());
  return NS_OK;
}


nsresult
nsStructTagUpdatingCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  if (newState.Length() > 0) return htmlEditor->SetStructureTag(newState);
  return NS_OK; //BBM should be bad data
}

nsEnvTagUpdatingCommand::nsEnvTagUpdatingCommand(void)
: nsBaseTagUpdatingCommand()
{
}

NS_IMETHODIMP
nsEnvTagUpdatingCommand::DoCommand(const char *aCommandName,
                                      nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;

  return SetState(editor, mTagName);
}

NS_IMETHODIMP
nsEnvTagUpdatingCommand::DoCommandParams(const char *aCommandName,
                                            nsICommandParams *aParams,
                                            nsISupports *refCon)
{
  nsXPIDLString tagName;
  nsresult rv;
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (!editor) return NS_ERROR_NOT_INITIALIZED;
  if (aParams) {
    nsXPIDLCString s;
    rv = aParams->GetCStringValue(STATE_ATTRIBUTE, getter_Copies(s));
    if (NS_SUCCEEDED(rv))
      tagName.AssignWithConversion(s);
    else
      rv = aParams->GetStringValue(STATE_ATTRIBUTE, tagName);
  }
  return SetState(editor, tagName);
}

nsresult
nsEnvTagUpdatingCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  // We call SetStructureTag because environments are very close to structures, and the code forks based on
  // the actual tag type.
  if (newState.Length() > 0) return htmlEditor->SetStructureTag(newState);
  return NS_OK; //BBM should be bad data
}

NS_IMETHODIMP
nsPasteNoFormattingCommand::IsCommandEnabled(const char * aCommandName,
                                             nsISupports *refCon,
                                             PRBool *outCmdEnabled)
{
  NS_ENSURE_ARG_POINTER(outCmdEnabled);
  *outCmdEnabled = PR_FALSE;

  // This command is only implemented by nsIHTMLEditor, since
  //  pasting in a plaintext editor automatically only supplies
  //  "unformatted" text
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(refCon);
  if (!htmlEditor)
    return NS_ERROR_NOT_IMPLEMENTED;

  nsCOMPtr<nsIEditor> editor = do_QueryInterface(htmlEditor);
  if (!editor)
    return NS_ERROR_INVALID_ARG;

  return editor->CanPaste(nsIClipboard::kGlobalClipboard, outCmdEnabled);
}


NS_IMETHODIMP
nsPasteNoFormattingCommand::DoCommand(const char *aCommandName,
                                      nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(refCon);
  if (!htmlEditor)
    return NS_ERROR_NOT_IMPLEMENTED;

  return htmlEditor->PasteNoFormatting(nsIClipboard::kGlobalClipboard);
}

NS_IMETHODIMP
nsPasteNoFormattingCommand::DoCommandParams(const char *aCommandName,
                                            nsICommandParams *aParams,
                                            nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsPasteNoFormattingCommand::GetCommandStateParams(const char *aCommandName,
                                                  nsICommandParams *aParams,
                                                  nsISupports *refCon)
{
  NS_ENSURE_ARG_POINTER(aParams);

  PRBool enabled = PR_FALSE;
  nsresult rv = IsCommandEnabled(aCommandName, refCon, &enabled);
  NS_ENSURE_SUCCESS(rv, rv);

  return aParams->SetBooleanValue(STATE_ENABLED, enabled);
}

nsTextTagUpdatingCommand::nsTextTagUpdatingCommand(void)
: nsBaseTagUpdatingCommand()
{
}


nsresult
nsTextTagUpdatingCommand::ToggleState(nsIEditor *aEditor, nsString & aTagName)
{
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor)
    return NS_ERROR_NO_INTERFACE;

  //create some params now...
  nsresult rv;
  nsCOMPtr<nsICommandParams> params =
      do_CreateInstance(NS_COMMAND_PARAMS_CONTRACTID,&rv);
  if (NS_FAILED(rv) || !params)
    return rv;

  // tags "href" and "name" are special cases in the core editor
  // they are used to remove named anchor/link and shouldn't be used for insertion
  PRBool doTagRemoval;
  if (aTagName.Equals(NS_LITERAL_STRING("href")) ||
      aTagName.Equals(NS_LITERAL_STRING("name")))
    doTagRemoval = PR_TRUE;
  else
  {
    // check current selection; set doTagRemoval if formatting should be removed
//    rv = GetCurrentState(aEditor,params);
//    if (NS_FAILED(rv))
//      return rv;
//    rv = params->GetBooleanValue(STATE_ALL, &doTagRemoval);
//    if (NS_FAILED(rv))
//      return rv;
    nsCOMPtr<msiITagListManager> taglistManager;
    htmlEditor->GetTagListManager( getter_AddRefs(taglistManager));
    if (taglistManager) taglistManager->SelectionContainedInTag( aTagName, nsnull, &doTagRemoval );  }

  if (doTagRemoval)
    rv = RemoveTextProperty(aEditor, aTagName.get(), nsnull);
  else
  {
    // Superscript and Subscript styles are mutually exclusive
    nsAutoString removeName;
    aEditor->BeginTransaction();

    if (aTagName.EqualsLiteral("sub"))
    {
      removeName.AssignWithConversion("sup");
      rv = RemoveTextProperty(aEditor ,aTagName.get(), nsnull);
    }
    else if (aTagName.EqualsLiteral("sup"))
    {
      removeName.AssignWithConversion("sub");
      rv = RemoveTextProperty(aEditor, aTagName.get(), nsnull);
    }
    if (NS_SUCCEEDED(rv))
      rv = SetTextProperty(aEditor,aTagName.get(), nsnull, nsnull);

    aEditor->EndTransaction();
  }

  // The next few lines shouldn't be necessary -- BBM
  nsCOMPtr<nsISelectionController>selCon;
  aEditor->GetSelectionController(getter_AddRefs(selCon));
  if (selCon)
    selCon->RepaintSelection(nsISelectionController::SELECTION_NORMAL);
  return rv;
}


nsresult
nsStructTagUpdatingCommand::ToggleState(nsIEditor *aEditor, nsString & aTagName)
{
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor)
    return NS_ERROR_NO_INTERFACE;

  //create some params now...
  nsresult rv;
  nsCOMPtr<nsICommandParams> params =
      do_CreateInstance(NS_COMMAND_PARAMS_CONTRACTID,&rv);
  if (NS_FAILED(rv) || !params)
    return rv;

  // tags "href" and "name" are special cases in the core editor
  // they are used to remove named anchor/link and shouldn't be used for insertion
  PRBool doTagRemoval;
  if (aTagName.Equals(NS_LITERAL_STRING("href")) ||
      aTagName.Equals(NS_LITERAL_STRING("name")))
    doTagRemoval = PR_TRUE;
  else
  {
    // check current selection; set doTagRemoval if formatting should be removed
    rv = GetCurrentState(aEditor, params);
    if (NS_FAILED(rv))
      return rv;
    rv = params->GetBooleanValue(STATE_ALL, &doTagRemoval);
    if (NS_FAILED(rv))
      return rv;
  }

  if (doTagRemoval)
    rv = RemoveTextProperty(aEditor, aTagName.get(), nsnull);
  else
  {
    // Superscript and Subscript styles are mutually exclusive
    nsAutoString removeName;
    aEditor->BeginTransaction();

    if (aTagName.Equals(NS_LITERAL_STRING("sub")))
    {
      removeName.AssignWithConversion("sup");
      rv = RemoveTextProperty(aEditor,aTagName.get(), nsnull);
    }
    else if (aTagName.Equals(NS_LITERAL_STRING("sup")))
    {
      removeName.AssignWithConversion("sub");
      rv = RemoveTextProperty(aEditor, aTagName.get(), nsnull);
    }
    if (NS_SUCCEEDED(rv))
      rv = SetTextProperty(aEditor,aTagName.get(), nsnull, nsnull);

    aEditor->EndTransaction();
  }

  return rv;
}

/* nsOtherTagUpdatingCommand::nsOtherTagUpdatingCommand(void)
: nsBaseTagUpdatingCommand()
{
}


nsresult
nsOtherTagUpdatingCommand::ToggleState(nsIEditor *aEditor, nsString & aTagName)
{
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor)
    return NS_ERROR_NO_INTERFACE;

  //create some params now...
  nsresult rv;
  nsCOMPtr<nsICommandParams> params =
      do_CreateInstance(NS_COMMAND_PARAMS_CONTRACTID,&rv);
  if (NS_FAILED(rv) || !params)
    return rv;

  // tags "href" and "name" are special cases in the core editor
  // they are used to remove named anchor/link and shouldn't be used for insertion
  PRBool doTagRemoval;
  if (aTagName.Equals(NS_LITERAL_STRING("href")) ||
      aTagName.Equals(NS_LITERAL_STRING("name")))
    doTagRemoval = PR_TRUE;
  else
  {
    // check current selection; set doTagRemoval if formatting should be removed
    rv = GetCurrentState(aEditor, params);
    if (NS_FAILED(rv))
      return rv;
    rv = params->GetBooleanValue(STATE_ALL, &doTagRemoval);
    if (NS_FAILED(rv))
      return rv;
  }

  if (doTagRemoval)
    rv = RemoveTextProperty(aEditor, aTagName.get(), nsnull);
  else
  {
    // Superscript and Subscript styles are mutually exclusive
    nsAutoString removeName;
    aEditor->BeginTransaction();

    if (aTagName.Equals(NS_LITERAL_STRING("sub")))
    {
      removeName.AssignWithConversion("sup");
      rv = RemoveTextProperty(aEditor,aTagName.get(), nsnull);
    }
    else if (aTagName.Equals(NS_LITERAL_STRING("sup")))
    {
      removeName.AssignWithConversion("sub");
      rv = RemoveTextProperty(aEditor, aTagName.get(), nsnull);
    }
    if (NS_SUCCEEDED(rv))
      rv = SetTextProperty(aEditor,aTagName.get(), nsnull, nsnull);

    aEditor->EndTransaction();
  }

  return rv;
}
*/

nsStyleUpdatingCommand::nsStyleUpdatingCommand(const char* aTagName)
: nsBaseStateUpdatingCommand(aTagName)
{
}

nsresult
nsStyleUpdatingCommand::GetCurrentState(nsIEditor *aEditor,
                                        nsString & aTagName,
                                        nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_NOT_INITIALIZED;

  nsresult rv = NS_OK;

  PRBool firstOfSelectionHasProp = PR_FALSE;
  PRBool anyOfSelectionHasProp = PR_FALSE;
  PRBool allOfSelectionHasProp = PR_FALSE;

  nsCOMPtr<nsIAtom> styleAtom = do_GetAtom(aTagName);
  rv = htmlEditor->GetInlineProperty(styleAtom, EmptyString(),
                                     EmptyString(),
                                     &firstOfSelectionHasProp,
                                     &anyOfSelectionHasProp,
                                     &allOfSelectionHasProp);

  aParams->SetBooleanValue(STATE_ENABLED, NS_SUCCEEDED(rv));
  aParams->SetBooleanValue(STATE_ALL, allOfSelectionHasProp);
  aParams->SetBooleanValue(STATE_ANY, anyOfSelectionHasProp);
  aParams->SetBooleanValue(STATE_MIXED, anyOfSelectionHasProp
           && !allOfSelectionHasProp);
  aParams->SetBooleanValue(STATE_BEGIN, firstOfSelectionHasProp);
  aParams->SetBooleanValue(STATE_END, allOfSelectionHasProp);//not completely accurate
  return NS_OK;
}

nsresult
nsStyleUpdatingCommand::ToggleState(nsIEditor *aEditor, nsString & aTagName)
{
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor)
    return NS_ERROR_NO_INTERFACE;

  //create some params now...
  nsresult rv;
  nsCOMPtr<nsICommandParams> params =
      do_CreateInstance(NS_COMMAND_PARAMS_CONTRACTID,&rv);
  if (NS_FAILED(rv) || !params)
    return rv;

  // tags "href" and "name" are special cases in the core editor
  // they are used to remove named anchor/link and shouldn't be used for insertion
  nsAutoString tagName;
  tagName.Assign(aTagName);
  PRBool doTagRemoval;
  if (tagName.EqualsLiteral("href") ||
      tagName.EqualsLiteral("name"))
    doTagRemoval = PR_TRUE;
  else
  {
    // check current selection; set doTagRemoval if formatting should be removed
    rv = GetCurrentState(aEditor, aTagName, params);
    if (NS_FAILED(rv))
      return rv;
    rv = params->GetBooleanValue(STATE_ALL, &doTagRemoval);
    if (NS_FAILED(rv))
      return rv;
  }

  if (doTagRemoval)
    rv = RemoveTextProperty(aEditor, tagName.get(), nsnull);
  else
  {
    // Superscript and Subscript styles are mutually exclusive
    nsAutoString removeName;
    aEditor->BeginTransaction();

    if (tagName.EqualsLiteral("sub"))
    {
      removeName.AssignLiteral("sup");
      rv = RemoveTextProperty(aEditor,tagName.get(), nsnull);
    }
    else if (tagName.EqualsLiteral("sup"))
    {
      removeName.AssignLiteral("sub");
      rv = RemoveTextProperty(aEditor, tagName.get(), nsnull);
    }
    if (NS_SUCCEEDED(rv))
      rv = SetTextProperty(aEditor,tagName.get(), nsnull, nsnull);

    aEditor->EndTransaction();
  }

  return rv;
}

nsListCommand::nsListCommand(const char* aTagName)
: nsBaseStateUpdatingCommand(aTagName)
{
}

nsresult
nsListCommand::GetCurrentState(nsIEditor *aEditor, nsString& aTagName,
                               nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need editor here");

  PRBool bMixed;
  nsAutoString tagStr;
  nsresult rv = GetListState(aEditor, mTagName, &bMixed, tagStr);
  if (NS_FAILED(rv)) return rv;

  // Need to use mTagName????
  PRBool inList = mTagName.Equals(aTagName);
  aParams->SetBooleanValue(STATE_ALL, !bMixed && inList);
  aParams->SetBooleanValue(STATE_MIXED, bMixed);
  aParams->SetBooleanValue(STATE_ENABLED, PR_TRUE);
  return NS_OK;
}

nsresult
nsListCommand::ToggleState(nsIEditor *aEditor, nsString & aTagName)
{
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(aEditor);
  if (!editor)
    return NS_NOINTERFACE;
  PRBool inList;
  // Need to use mTagName????
  nsresult rv;
  nsCOMPtr<nsICommandParams> params =
      do_CreateInstance(NS_COMMAND_PARAMS_CONTRACTID,&rv);
  if (NS_FAILED(rv) || !params)
    return rv;

  rv = GetCurrentState(aEditor, mTagName, params);
  rv = params->GetBooleanValue(STATE_ALL,&inList);
  if (NS_FAILED(rv))
    return rv;

  if (inList)
    rv = editor->RemoveList(mTagName);
  else
  {
    rv = editor->MakeOrChangeList(mTagName, PR_FALSE, EmptyString());
  }

  return rv;
}


nsListItemCommand::nsListItemCommand(const char* aTagName)
: nsBaseStateUpdatingCommand(aTagName)
{
}

nsresult
nsListItemCommand::GetCurrentState(nsIEditor *aEditor, nsString& aTagName,
                                   nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need editor here");
  // 39584
  nsCOMPtr<nsIHTMLEditor>  htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor)
    return NS_NOINTERFACE;

  PRBool bMixed, bLI, bDT, bDD;
  nsresult rv = htmlEditor->GetListItemState(&bMixed, &bLI, &bDT, &bDD);
  if (NS_FAILED(rv)) return rv;

  PRBool inList = PR_FALSE;
  if (!bMixed)
  {
    if (bLI) inList = mTagName.EqualsLiteral("li");
    else if (bDT) inList =  mTagName.EqualsLiteral("dt");
    else if (bDD) inList =  mTagName.EqualsLiteral("dd");
  }

  aParams->SetBooleanValue(STATE_ALL, !bMixed && inList);
  aParams->SetBooleanValue(STATE_MIXED, bMixed);

  return NS_OK;
}

nsresult
nsListItemCommand::ToggleState(nsIEditor *aEditor, nsString& aTagName)
{
  NS_ASSERTION(aEditor, "Need editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_NOT_INITIALIZED;

  PRBool inList;
  // Need to use mTagName????
  nsresult rv;
  nsCOMPtr<nsICommandParams> params =
      do_CreateInstance(NS_COMMAND_PARAMS_CONTRACTID,&rv);
  if (NS_FAILED(rv) || !params)
    return rv;
  rv = GetCurrentState(aEditor, mTagName, params);
  rv = params->GetBooleanValue(STATE_ALL,&inList);
  if (NS_FAILED(rv))
    return rv;
  if (NS_FAILED(rv)) return rv;

  if (inList)
  {
    // To remove a list, first get what kind of list we're in
    PRBool bMixed;
    nsAutoString   tagStr;
    rv = GetListState(aEditor,mTagName, &bMixed, tagStr);
    if (NS_FAILED(rv)) return rv;
    if (!tagStr.IsEmpty())
    {
      if (!bMixed)
      {
        rv = htmlEditor->RemoveList(nsDependentString(tagStr));
      }
    }
  }
  else
  {
    // Set to the requested paragraph type
    //XXX Note: This actually doesn't work for "LI",
    //    but we currently don't use this for non DL lists anyway.
    // Problem: won't this replace any current block paragraph style?
    rv = htmlEditor->SetParagraphFormat(mTagName);
  }

  return rv;
}


NS_IMETHODIMP
nsRemoveListCommand::IsCommandEnabled(const char * aCommandName,
                                      nsISupports *refCon,
                                      PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  if (editor)
  {
    // It is enabled if we are in any list type
    PRBool bMixed;
    nsAutoString tagStr;
// BBM must fix this!!!    nsresult rv = GetListState(editor, mTagName, &bMixed, tagStr);
//    if (NS_FAILED(rv)) return rv;

    *outCmdEnabled = bMixed ? PR_TRUE : (!tagStr.IsEmpty());

  }
  else
    *outCmdEnabled = PR_FALSE;

  return NS_OK;
}


NS_IMETHODIMP
nsRemoveListCommand::DoCommand(const char *aCommandName, nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);

  nsresult rv = NS_OK;
  if (editor)
  {
    // This removes any list type
    rv = editor->RemoveList(EmptyString());
  }

  return rv;
}

NS_IMETHODIMP
nsRemoveListCommand::DoCommandParams(const char *aCommandName,
                                     nsICommandParams *aParams,
                                     nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsRemoveListCommand::GetCommandStateParams(const char *aCommandName,
                                           nsICommandParams *aParams,
                                           nsISupports *refCon)
{
  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED,outCmdEnabled);
}


NS_IMETHODIMP
nsIndentCommand::IsCommandEnabled(const char * aCommandName,
                                  nsISupports *refCon, PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
nsIndentCommand::DoCommand(const char *aCommandName, nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);

  nsresult rv = NS_OK;
  if (editor)
  {
    rv = editor->Indent(NS_LITERAL_STRING("indent"));
  }

  return rv;
}

NS_IMETHODIMP
nsIndentCommand::DoCommandParams(const char *aCommandName,
                                 nsICommandParams *aParams,
                                 nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsIndentCommand::GetCommandStateParams(const char *aCommandName,
                                       nsICommandParams *aParams,
                                       nsISupports *refCon)
{
  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED,outCmdEnabled);
}


//OUTDENT

NS_IMETHODIMP
nsOutdentCommand::IsCommandEnabled(const char * aCommandName,
                                   nsISupports *refCon,
                                   PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(refCon);
  if (htmlEditor)
  {
    PRBool canIndent, canOutdent;
    htmlEditor->GetIndentState(&canIndent, &canOutdent);
    *outCmdEnabled = canOutdent;
  }
  else
    *outCmdEnabled = PR_FALSE;

  return NS_OK;
}


NS_IMETHODIMP
nsOutdentCommand::DoCommand(const char *aCommandName, nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(refCon);
  if (htmlEditor)
    return htmlEditor->Indent(NS_LITERAL_STRING("outdent"));

  return NS_OK;
}

NS_IMETHODIMP
nsOutdentCommand::DoCommandParams(const char *aCommandName,
                                  nsICommandParams *aParams,
                                  nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsOutdentCommand::GetCommandStateParams(const char *aCommandName,
                                        nsICommandParams *aParams,
                                        nsISupports *refCon)
{
  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED,outCmdEnabled);
}



nsMultiStateCommand::nsMultiStateCommand()
: nsBaseComposerCommand()
{
}

nsMultiStateCommand::~nsMultiStateCommand()
{
}

NS_IMPL_ISUPPORTS_INHERITED0(nsMultiStateCommand, nsBaseComposerCommand)

NS_IMETHODIMP
nsMultiStateCommand::IsCommandEnabled(const char * aCommandName,
                                      nsISupports *refCon,
                                      PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  // should be disabled sometimes, like if the current selection is an image
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
nsMultiStateCommand::DoCommand(const char *aCommandName, nsISupports *refCon)
{
#ifdef DEBUG
  printf("who is calling nsMultiStateCommand::DoCommand \
          (no implementation)? %s\n", aCommandName);
#endif

  return NS_OK;
}

NS_IMETHODIMP
nsMultiStateCommand::DoCommandParams(const char *aCommandName,
                                     nsICommandParams *aParams,
                                     nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);

  nsresult rv = NS_OK;
  if (editor)
  {
      nsAutoString tString;

      if (aParams) {
        nsXPIDLCString s;
        rv = aParams->GetCStringValue(STATE_ATTRIBUTE, getter_Copies(s));
        if (NS_SUCCEEDED(rv))
          tString.AssignWithConversion(s);
        else
          rv = aParams->GetStringValue(STATE_ATTRIBUTE, tString);
      }

      rv = SetState(editor, tString);
  }

  return rv;
}

NS_IMETHODIMP
nsMultiStateCommand::GetCommandStateParams(const char *aCommandName,
                                           nsICommandParams *aParams,
                                           nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  nsresult rv = NS_OK;
  if (editor)
  {
      rv = GetCurrentState(editor, aParams);
  }
  return rv;
}


nsParagraphStateCommand::nsParagraphStateCommand()
: nsMultiStateCommand()
{
}

nsresult
nsParagraphStateCommand::GetCurrentState(nsIEditor *aEditor,
                                         nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need an editor here");

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  PRBool outMixed;
  nsAutoString outStateString;
  nsresult rv = htmlEditor->GetParagraphState(&outMixed, outStateString);
  if (NS_SUCCEEDED(rv))
  {
    nsCAutoString tOutStateString;
    tOutStateString.AssignWithConversion(outStateString);
    aParams->SetBooleanValue(STATE_MIXED,outMixed);
    aParams->SetCStringValue(STATE_ATTRIBUTE, tOutStateString.get());
  }
  return rv;
}


nsresult
nsParagraphStateCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  return htmlEditor->SetParagraphFormat(newState);
}


nsFontFaceStateCommand::nsFontFaceStateCommand()
: nsMultiStateCommand()
{
}

nsresult
nsFontFaceStateCommand::GetCurrentState(nsIEditor *aEditor,
                                        nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  nsAutoString outStateString;
  PRBool outMixed;
  nsresult rv = htmlEditor->GetFontFaceState(&outMixed, outStateString);
  if (NS_SUCCEEDED(rv))
  {
    aParams->SetBooleanValue(STATE_MIXED,outMixed);
    aParams->SetCStringValue(STATE_ATTRIBUTE, NS_ConvertUTF16toUTF8(outStateString).get());
  }
  return rv;
}


nsresult
nsFontFaceStateCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  nsresult rv;
  nsCOMPtr<nsIAtom> ttAtom = do_GetAtom("tt");
  nsCOMPtr<nsIAtom> fontAtom = do_GetAtom("font");

  if (newState.EqualsLiteral("tt"))
  {
    // The old "teletype" attribute
    rv = htmlEditor->SetInlineProperty(ttAtom, EmptyString(),
                                       EmptyString());
    // Clear existing font face
    rv = htmlEditor->RemoveInlineProperty(fontAtom, NS_LITERAL_STRING("face"));
  }
  else
  {
    // Remove any existing TT nodes
    rv = htmlEditor->RemoveInlineProperty(ttAtom, EmptyString());

    if (newState.IsEmpty() || newState.EqualsLiteral("normal")) {
      rv = htmlEditor->RemoveInlineProperty(fontAtom, NS_LITERAL_STRING("face"));
    } else {
      rv = htmlEditor->SetInlineProperty(fontAtom, NS_LITERAL_STRING("face"),
                                         newState);
    }
  }

  return rv;
}


nsFontSizeStateCommand::nsFontSizeStateCommand()
  : nsMultiStateCommand()
{
}

//  nsCAutoString tOutStateString;
//  tOutStateString.AssignWithConversion(outStateString);
nsresult
nsFontSizeStateCommand::GetCurrentState(nsIEditor *aEditor,
                                        nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_INVALID_ARG;

  nsAutoString outStateString;
  nsCOMPtr<nsIAtom> fontAtom = do_GetAtom("font");
  PRBool firstHas, anyHas, allHas;
  nsresult rv = htmlEditor->GetInlinePropertyWithAttrValue(fontAtom,
                                         NS_LITERAL_STRING("size"),
                                         EmptyString(),
                                         &firstHas, &anyHas, &allHas,
                                         outStateString);
  if (NS_FAILED(rv)) return rv;

  nsCAutoString tOutStateString;
  tOutStateString.AssignWithConversion(outStateString);
  aParams->SetBooleanValue(STATE_MIXED, anyHas && !allHas);
  aParams->SetCStringValue(STATE_ATTRIBUTE, tOutStateString.get());
  aParams->SetBooleanValue(STATE_ENABLED, PR_TRUE);

  return rv;
}


// acceptable values for "newState" are:
//   -2
//   -1
//    0
//   +1
//   +2
//   +3
//   medium
//   normal
nsresult
nsFontSizeStateCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_INVALID_ARG;

  nsresult rv;
  nsCOMPtr<nsIAtom> fontAtom = do_GetAtom("font");
  if (newState.IsEmpty() ||
      newState.EqualsLiteral("normal") ||
      newState.EqualsLiteral("medium")) {
    // remove any existing font size, big or small
    rv = htmlEditor->RemoveInlineProperty(fontAtom, NS_LITERAL_STRING("size"));
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIAtom> bigAtom = do_GetAtom("big");
    rv = htmlEditor->RemoveInlineProperty(bigAtom, EmptyString());
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIAtom> smallAtom = do_GetAtom("small");
    rv = htmlEditor->RemoveInlineProperty(smallAtom, EmptyString());
    if (NS_FAILED(rv)) return rv;
  } else {
    // set the size
    rv = htmlEditor->SetInlineProperty(fontAtom, NS_LITERAL_STRING("size"),
                                       newState);
  }

  return rv;
}

nsFontColorStateCommand::nsFontColorStateCommand()
: nsMultiStateCommand()
{
}

nsresult
nsFontColorStateCommand::GetCurrentState(nsIEditor *aEditor,
                                         nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need an editor here");

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  PRBool outMixed;
  nsAutoString outStateString;
  nsresult rv = htmlEditor->GetFontColorState(&outMixed, outStateString);
  if (NS_SUCCEEDED(rv))
  {
    nsCAutoString tOutStateString;
    tOutStateString.AssignWithConversion(outStateString);
    aParams->SetBooleanValue(STATE_MIXED,outMixed);
    aParams->SetCStringValue(STATE_ATTRIBUTE, tOutStateString.get());
  }
  return rv;
}

nsresult
nsFontColorStateCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  nsresult rv;
  nsCOMPtr<nsIAtom> fontAtom = do_GetAtom("font");

  if (newState.IsEmpty() || newState.EqualsLiteral("normal")) {
    rv = htmlEditor->RemoveInlineProperty(fontAtom, NS_LITERAL_STRING("color"));
  } else {
    rv = htmlEditor->SetInlineProperty(fontAtom, NS_LITERAL_STRING("color"),
                                       newState);
  }

  return rv;
}


nsHighlightColorStateCommand::nsHighlightColorStateCommand()
: nsMultiStateCommand()
{
}

nsresult
nsHighlightColorStateCommand::GetCurrentState(nsIEditor *aEditor,
                                              nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  PRBool outMixed;
  nsAutoString outStateString;
  nsresult rv = htmlEditor->GetHighlightColorState(&outMixed, outStateString);
  if (NS_SUCCEEDED(rv))
  {
    nsCAutoString tOutStateString;
    tOutStateString.AssignWithConversion(outStateString);
    aParams->SetBooleanValue(STATE_MIXED,outMixed);
    aParams->SetCStringValue(STATE_ATTRIBUTE, tOutStateString.get());
  }
  return rv;
}

nsresult
nsHighlightColorStateCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  nsresult rv;
  nsCOMPtr<nsIAtom> fontAtom = do_GetAtom("font");

  if (newState.IsEmpty() || newState.EqualsLiteral("normal")) {
//    rv = RemoveOneProperty(htmlEditor, NS_LITERAL_STRING("font"), NS_LITERAL_STRING("bgcolor"));
    rv = htmlEditor->RemoveInlineProperty(fontAtom, NS_LITERAL_STRING("bgcolor"));
  } else {
    rv = htmlEditor->SetCSSInlineProperty(fontAtom, NS_LITERAL_STRING("bgcolor"),
                                          newState);
  }

  return rv;
}

NS_IMETHODIMP
nsHighlightColorStateCommand::IsCommandEnabled(const char * aCommandName,
                                               nsISupports *refCon,
                                               PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(refCon);
  *outCmdEnabled = htmlEditor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}



nsBackgroundColorStateCommand::nsBackgroundColorStateCommand()
: nsMultiStateCommand()
{
}

nsresult
nsBackgroundColorStateCommand::GetCurrentState(nsIEditor *aEditor,
                                               nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need an editor here");

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  PRBool outMixed;
  nsAutoString outStateString;
  nsresult rv =  htmlEditor->GetBackgroundColorState(&outMixed, outStateString);
  if (NS_SUCCEEDED(rv))
  {
    nsCAutoString tOutStateString;
    tOutStateString.AssignWithConversion(outStateString);
    aParams->SetBooleanValue(STATE_MIXED,outMixed);
    aParams->SetCStringValue(STATE_ATTRIBUTE, tOutStateString.get());
  }
  return rv;
}

nsresult
nsBackgroundColorStateCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  return htmlEditor->SetBackgroundColor(newState);
}


nsAlignCommand::nsAlignCommand()
: nsMultiStateCommand()
{
}

nsresult
nsAlignCommand::GetCurrentState(nsIEditor *aEditor, nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need an editor here");

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  nsIHTMLEditor::EAlignment firstAlign;
  PRBool outMixed;
  nsresult rv = htmlEditor->GetAlignment(&outMixed, &firstAlign);

  if (NS_FAILED(rv))
    return rv;

  nsAutoString outStateString;
  switch (firstAlign)
  {
    default:
    case nsIHTMLEditor::eLeft:
      outStateString.AssignLiteral("left");
      break;

    case nsIHTMLEditor::eCenter:
      outStateString.AssignLiteral("center");
      break;

    case nsIHTMLEditor::eRight:
      outStateString.AssignLiteral("right");
      break;

    case nsIHTMLEditor::eJustify:
      outStateString.AssignLiteral("justify");
      break;
  }
  nsCAutoString tOutStateString;
  tOutStateString.AssignWithConversion(outStateString);
  aParams->SetBooleanValue(STATE_MIXED,outMixed);
  aParams->SetCStringValue(STATE_ATTRIBUTE, tOutStateString.get());
  return NS_OK;
}

nsresult
nsAlignCommand::SetState(nsIEditor *aEditor, nsString& newState)
{
  NS_ASSERTION(aEditor, "Need an editor here");

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  return htmlEditor->Align(newState);
}




nsAbsolutePositioningCommand::nsAbsolutePositioningCommand()
: nsBaseStateUpdatingCommand("")
{
}

NS_IMETHODIMP
nsAbsolutePositioningCommand::IsCommandEnabled(const char * aCommandName,
                                               nsISupports *aCommandRefCon,
                                               PRBool *_retval)
{
  NS_ASSERTION(aCommandRefCon, "Need an editor here");

  nsCOMPtr<nsIHTMLAbsPosEditor> htmlEditor = do_QueryInterface(aCommandRefCon);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  htmlEditor->GetAbsolutePositioningEnabled(_retval);
  return NS_OK;
}

nsresult
nsAbsolutePositioningCommand::GetCurrentState(nsIEditor *aEditor, nsString& aTagName, nsICommandParams *aParams)
{
  NS_ASSERTION(aEditor, "Need an editor here");

  nsCOMPtr<nsIHTMLAbsPosEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  PRBool isEnabled;
  htmlEditor->GetAbsolutePositioningEnabled(&isEnabled);
  if (!isEnabled) {
    aParams->SetBooleanValue(STATE_MIXED,PR_FALSE);
    aParams->SetCStringValue(STATE_ATTRIBUTE, "");
    return NS_OK;
  }

  nsCOMPtr<nsIDOMElement>  elt;
  nsresult rv = htmlEditor->GetAbsolutelyPositionedSelectionContainer(getter_AddRefs(elt));
  if (NS_FAILED(rv))
    return rv;

  nsAutoString outStateString;
  if (elt)
    outStateString.AssignLiteral("absolute");

  aParams->SetBooleanValue(STATE_MIXED,PR_FALSE);
  aParams->SetCStringValue(STATE_ATTRIBUTE, NS_ConvertUTF16toUTF8(outStateString).get());
  return NS_OK;
}

nsresult
nsAbsolutePositioningCommand::ToggleState(nsIEditor *aEditor, nsString& aTagName)
{
  NS_ASSERTION(aEditor, "Need an editor here");

  nsCOMPtr<nsIHTMLAbsPosEditor> htmlEditor = do_QueryInterface(aEditor);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  nsCOMPtr<nsIDOMElement>  elt;
  nsresult rv = htmlEditor->GetAbsolutelyPositionedSelectionContainer(getter_AddRefs(elt));
  if (NS_FAILED(rv)) return rv;

  if (elt) {
    // we have to remove positioning on an element
    rv = htmlEditor->AbsolutePositionSelection(PR_FALSE);
  }
  else {
    rv = htmlEditor->AbsolutePositionSelection(PR_TRUE);
  }
  return rv;
}



NS_IMETHODIMP
nsDecreaseZIndexCommand::IsCommandEnabled(const char * aCommandName,
                                          nsISupports *refCon,
                                          PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIHTMLAbsPosEditor> htmlEditor = do_QueryInterface(refCon);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  htmlEditor->GetAbsolutePositioningEnabled(outCmdEnabled);
  if (!(*outCmdEnabled))
    return NS_OK;

  nsCOMPtr<nsIDOMElement> positionedElement;
  htmlEditor->GetPositionedElement(getter_AddRefs(positionedElement));
  *outCmdEnabled = PR_FALSE;
  if (positionedElement) {
    PRInt32 z;
    nsresult res = htmlEditor->GetElementZIndex(positionedElement, &z);
    if (NS_FAILED(res)) return res;
    *outCmdEnabled = (z > 0);
  }

  return NS_OK;
}

NS_IMETHODIMP
nsDecreaseZIndexCommand::DoCommand(const char *aCommandName,
                                   nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLAbsPosEditor> htmlEditor = do_QueryInterface(refCon);
  if (!htmlEditor)
    return NS_ERROR_NOT_IMPLEMENTED;

  return htmlEditor->RelativeChangeZIndex(-1);
}

NS_IMETHODIMP
nsDecreaseZIndexCommand::DoCommandParams(const char *aCommandName,
                                         nsICommandParams *aParams,
                                         nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsDecreaseZIndexCommand::GetCommandStateParams(const char *aCommandName,
                                               nsICommandParams *aParams,
                                               nsISupports *refCon)
{
  NS_ENSURE_ARG_POINTER(aParams);

  PRBool enabled = PR_FALSE;
  nsresult rv = IsCommandEnabled(aCommandName, refCon, &enabled);
  NS_ENSURE_SUCCESS(rv, rv);

  return aParams->SetBooleanValue(STATE_ENABLED, enabled);
}


NS_IMETHODIMP
nsIncreaseZIndexCommand::IsCommandEnabled(const char * aCommandName,
                                          nsISupports *refCon,
                                          PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIHTMLAbsPosEditor> htmlEditor = do_QueryInterface(refCon);
  if (!htmlEditor) return NS_ERROR_FAILURE;

  htmlEditor->GetAbsolutePositioningEnabled(outCmdEnabled);
  if (!(*outCmdEnabled))
    return NS_OK;

  nsCOMPtr<nsIDOMElement> positionedElement;
  htmlEditor->GetPositionedElement(getter_AddRefs(positionedElement));
  *outCmdEnabled = (nsnull != positionedElement);
  return NS_OK;
}

NS_IMETHODIMP
nsIncreaseZIndexCommand::DoCommand(const char *aCommandName,
                                   nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLAbsPosEditor> htmlEditor = do_QueryInterface(refCon);
  if (!htmlEditor)
    return NS_ERROR_NOT_IMPLEMENTED;

  return htmlEditor->RelativeChangeZIndex(1);
}

NS_IMETHODIMP
nsIncreaseZIndexCommand::DoCommandParams(const char *aCommandName,
                                         nsICommandParams *aParams,
                                         nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsIncreaseZIndexCommand::GetCommandStateParams(const char *aCommandName,
                                               nsICommandParams *aParams,
                                               nsISupports *refCon)
{
  NS_ENSURE_ARG_POINTER(aParams);

  PRBool enabled = PR_FALSE;
  nsresult rv = IsCommandEnabled(aCommandName, refCon, &enabled);
  NS_ENSURE_SUCCESS(rv, rv);

  return aParams->SetBooleanValue(STATE_ENABLED, enabled);
}



NS_IMETHODIMP
nsRemoveStylesCommand::IsCommandEnabled(const char * aCommandName,
                                        nsISupports *refCon,
                                        PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  // test if we have any styles?
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}



NS_IMETHODIMP
nsRemoveStylesCommand::DoCommand(const char *aCommandName,
                                 nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);

  nsresult rv = NS_OK;
  if (editor)
  {
    rv = editor->RemoveAllInlineProperties();
  }

  return rv;
}

NS_IMETHODIMP
nsRemoveStylesCommand::DoCommandParams(const char *aCommandName,
                                       nsICommandParams *aParams,
                                       nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsRemoveStylesCommand::GetCommandStateParams(const char *aCommandName,
                                             nsICommandParams *aParams,
                                             nsISupports *refCon)
{
  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED,outCmdEnabled);
}


NS_IMETHODIMP
nsRemoveStructCommand::IsCommandEnabled(const char * aCommandName,
                                        nsISupports *refCon,
                                        PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  // test if we have any styles?
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}



NS_IMETHODIMP
nsRemoveStructCommand::DoCommand(const char *aCommandName,
                                 nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(editor);
  nsCOMPtr<nsISelection> selection;
  editor->GetSelection(getter_AddRefs(selection));

  nsresult rv = NS_OK;
  if (editor)
  {
    rv = htmlEditor->RemoveStructureAboveSelection(selection);
  }

  return rv;
}

NS_IMETHODIMP
nsRemoveStructCommand::DoCommandParams(const char *aCommandName,
                                       nsICommandParams *aParams,
                                       nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsRemoveStructCommand::GetCommandStateParams(const char *aCommandName,
                                             nsICommandParams *aParams,
                                             nsISupports *refCon)
{
  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED,outCmdEnabled);
}


NS_IMETHODIMP
nsRemoveEnvCommand::IsCommandEnabled(const char * aCommandName,
                                        nsISupports *refCon,
                                        PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  // test if we have any environments?
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}



NS_IMETHODIMP
nsRemoveEnvCommand::DoCommand(const char *aCommandName,
                                 nsISupports *refCon)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(editor);
  nsCOMPtr<nsISelection> selection;
  editor->GetSelection(getter_AddRefs(selection));

  nsresult rv = NS_OK;
  if (htmlEditor) {
    editor->BeginTransaction();
    rv = htmlEditor->RemoveEnvAboveSelection(selection);
    editor->EndTransaction();
  }
  return rv;
}

NS_IMETHODIMP
nsRemoveEnvCommand::DoCommandParams(const char *aCommandName,
                                       nsICommandParams *aParams,
                                       nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsRemoveEnvCommand::GetCommandStateParams(const char *aCommandName,
                                             nsICommandParams *aParams,
                                             nsISupports *refCon)
{
  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED,outCmdEnabled);
}


NS_IMETHODIMP
nsIncreaseFontSizeCommand::IsCommandEnabled(const char * aCommandName,
                                            nsISupports *refCon,
                                            PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  // test if we have any styles?
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
nsIncreaseFontSizeCommand::DoCommand(const char *aCommandName,
                                     nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);

  nsresult rv = NS_OK;
  if (editor)
  {
    rv = editor->IncreaseFontSize();
  }

  return rv;
}

NS_IMETHODIMP
nsIncreaseFontSizeCommand::DoCommandParams(const char *aCommandName,
                                           nsICommandParams *aParams,
                                           nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsIncreaseFontSizeCommand::GetCommandStateParams(const char *aCommandName,
                                                 nsICommandParams *aParams,
                                                 nsISupports *refCon)
{
  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED,outCmdEnabled);
}


NS_IMETHODIMP
nsDecreaseFontSizeCommand::IsCommandEnabled(const char * aCommandName,
                                            nsISupports *refCon,
                                            PRBool *outCmdEnabled)
{
  nsCOMPtr<nsIEditor> editor = do_QueryInterface(refCon);
  // test if we are at min size?
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
nsDecreaseFontSizeCommand::DoCommand(const char *aCommandName,
                                     nsISupports *refCon)
{
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);

  nsresult rv = NS_OK;
  if (editor)
  {
    rv = editor->DecreaseFontSize();
  }

  return rv;
}

NS_IMETHODIMP
nsDecreaseFontSizeCommand::DoCommandParams(const char *aCommandName,
                                           nsICommandParams *aParams,
                                           nsISupports *refCon)
{
  return DoCommand(aCommandName, refCon);
}

NS_IMETHODIMP
nsDecreaseFontSizeCommand::GetCommandStateParams(const char *aCommandName,
                                                 nsICommandParams *aParams,
                                                 nsISupports *refCon)
{
  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED,outCmdEnabled);
}


NS_IMETHODIMP
nsInsertHTMLCommand::IsCommandEnabled(const char * aCommandName,
                                      nsISupports *refCon,
                                      PRBool *outCmdEnabled)
{
  NS_ENSURE_ARG_POINTER(outCmdEnabled);
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}


NS_IMETHODIMP
nsInsertHTMLCommand::DoCommand(const char *aCommandName, nsISupports *refCon)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsInsertHTMLCommand::DoCommandParams(const char *aCommandName,
                                     nsICommandParams *aParams,
                                     nsISupports *refCon)
{
  NS_ENSURE_ARG_POINTER(aParams);
  NS_ENSURE_ARG_POINTER(refCon);

  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);
  if (!editor)
    return NS_ERROR_NOT_IMPLEMENTED;

  // Get HTML source string to insert from command params
  nsAutoString html;
  nsresult rv = aParams->GetStringValue(STATE_DATA, html);
  NS_ENSURE_SUCCESS(rv, rv);

  if (!html.IsEmpty())
    return editor->InsertHTML(html);

  return NS_OK;
}

NS_IMETHODIMP
nsInsertHTMLCommand::GetCommandStateParams(const char *aCommandName,
                                           nsICommandParams *aParams,
                                           nsISupports *refCon)
{
  NS_ENSURE_ARG_POINTER(aParams);
  NS_ENSURE_ARG_POINTER(refCon);

  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED, outCmdEnabled);
}


NS_IMPL_ISUPPORTS_INHERITED0(nsInsertTagCommand, nsBaseComposerCommand)

nsInsertTagCommand::nsInsertTagCommand(const char* aTagName)
: nsBaseComposerCommand()
, mTagName(NS_ConvertUTF8toUTF16(nsDependentCString(aTagName)))
{
}

nsInsertTagCommand::~nsInsertTagCommand()
{
}

NS_IMETHODIMP
nsInsertTagCommand::IsCommandEnabled(const char * aCommandName,
                                     nsISupports *refCon,
                                     PRBool *outCmdEnabled)
{
  NS_ENSURE_ARG_POINTER(outCmdEnabled);
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);
  *outCmdEnabled = editor ? PR_TRUE : PR_FALSE;
  return NS_OK;
}


// corresponding STATE_ATTRIBUTE is: src (img) and href (a)
NS_IMETHODIMP
nsInsertTagCommand::DoCommand(const char *aCmdName, nsISupports *refCon)
{
  if (mTagName.EqualsLiteral("hr"))
  {
    nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);
    if (!editor)
      return NS_ERROR_NOT_IMPLEMENTED;

    nsCOMPtr<nsIDOMElement> domElem;
    nsresult rv;
    rv = editor->CreateElementWithDefaults(mTagName,
                                           getter_AddRefs(domElem));
    if (NS_FAILED(rv))
      return rv;

    return editor->InsertElementAtSelection(domElem, PR_TRUE);
  }

  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsInsertTagCommand::DoCommandParams(const char *aCommandName,
                                    nsICommandParams *aParams,
                                    nsISupports *refCon)
{
  NS_ENSURE_ARG_POINTER(refCon);

  // inserting an hr shouldn't have an parameters, just call DoCommand for that
  if (mTagName.EqualsLiteral("hr"))
    return DoCommand(aCommandName, refCon);

  NS_ENSURE_ARG_POINTER(aParams);

  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(refCon);
  if (!editor)
    return NS_ERROR_NOT_IMPLEMENTED;

  // do we have an href to use for creating link?
  nsXPIDLCString s;
  nsresult rv = aParams->GetCStringValue(STATE_ATTRIBUTE, getter_Copies(s));
  NS_ENSURE_SUCCESS(rv, rv);
  nsAutoString attrib; attrib.AssignWithConversion(s);

  if (attrib.IsEmpty())
    return NS_ERROR_INVALID_ARG;

  // filter out tags we don't know how to insert
  nsAutoString attributeType;
  if (mTagName.EqualsLiteral("a")) {
    attributeType.AssignLiteral("href");
  } else if (mTagName.EqualsLiteral("img")) {
    attributeType.AssignLiteral("src");
  } else {
    return NS_ERROR_NOT_IMPLEMENTED;
  }

  nsCOMPtr<nsIDOMElement> domElem;
  rv = editor->CreateElementWithDefaults(mTagName,
                                         getter_AddRefs(domElem));
  if (NS_FAILED(rv))
    return rv;

  rv = domElem->SetAttribute(attributeType, attrib);
  if (NS_FAILED(rv))
    return rv;

  // do actual insertion
  if (mTagName.EqualsLiteral("a"))
    return editor->InsertLinkAroundSelection(domElem);

  return editor->InsertElementAtSelection(domElem, PR_TRUE);
}

NS_IMETHODIMP
nsInsertTagCommand::GetCommandStateParams(const char *aCommandName,
                                          nsICommandParams *aParams,
                                          nsISupports *refCon)
{
  NS_ENSURE_ARG_POINTER(aParams);
  NS_ENSURE_ARG_POINTER(refCon);

  PRBool outCmdEnabled = PR_FALSE;
  IsCommandEnabled(aCommandName, refCon, &outCmdEnabled);
  return aParams->SetBooleanValue(STATE_ENABLED, outCmdEnabled);
}


/****************************/
//HELPER METHODS
/****************************/

nsresult
GetListState(nsIEditor *aEditor, nsString & itemTagName, PRBool *aMixed, nsString &_retval)
{
  if (!aMixed || !aEditor)
    return NS_ERROR_NULL_POINTER;
  *aMixed = PR_FALSE;

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor);
  nsresult err = NS_ERROR_NO_INTERFACE;
  if (htmlEditor)
  {
    PRBool bOL, bUL, bDL;
    err = htmlEditor->GetListState(aMixed, &bOL, &bUL, &bDL);
    if (NS_SUCCEEDED(err))
    {
      if (!*aMixed)
      {
        nsCOMPtr<msiITagListManager> manager;
        htmlEditor->GetTagListManager(getter_AddRefs(manager));
        nsAutoString  htmllistparent;
        manager->GetStringPropertyForTag( itemTagName, nsnull, NS_LITERAL_STRING("htmllistparent"), htmllistparent);
        if (htmllistparent.Length() >0)
          _retval = htmllistparent;

      }
    }
  }
  return err;
}

nsresult
RemoveOneProperty(nsIHTMLEditor *aEditor,const nsString& aProp,
                  const nsString &aAttr)
{
  if (!aEditor)
    return NS_ERROR_NOT_INITIALIZED;

  /// XXX Hack alert! Look in nsIEditProperty.h for this
  nsCOMPtr<nsIAtom> styleAtom = do_GetAtom(aProp);
  if (! styleAtom)
    return NS_ERROR_OUT_OF_MEMORY;

  return aEditor->RemoveInlineProperty(styleAtom, aAttr);
}


// the name of the attribute here should be the contents of the appropriate
// tag, e.g. 'b' for bold, 'i' for italics.
nsresult
RemoveTextProperty(nsIEditor *aEditor, const PRUnichar *prop,
                   const PRUnichar *attr)
{
  if (!aEditor)
    return NS_ERROR_NOT_INITIALIZED;
  nsCOMPtr<nsIHTMLEditor> editor = do_QueryInterface(aEditor);
  if (!editor)
    return NS_ERROR_INVALID_ARG;
  // OK, I'm really hacking now. This is just so that
  //     we can accept 'all' as input.
  nsAutoString  allStr(prop);

  ToLowerCase(allStr);
  PRBool    doingAll = (allStr.EqualsLiteral("all"));
  nsresult  err = NS_OK;

  if (doingAll)
  {
    err = editor->RemoveAllInlineProperties();
  }
  else
  {
    nsAutoString  aProp(prop);
    nsAutoString  aAttr(attr);
    err = RemoveOneProperty(editor,aProp, aAttr);
  }

  return err;
}

// the name of the attribute here should be the contents of the appropriate
// tag, e.g. 'b' for bold, 'i' for italics.
nsresult
SetTextProperty(nsIEditor *aEditor, const PRUnichar *prop,
                const PRUnichar *attr, const PRUnichar *value)
{
  //static initialization
  static const PRUnichar sEmptyStr = PRUnichar('\0');

  if (!aEditor)
    return NS_ERROR_NOT_INITIALIZED;

  /// XXX Hack alert! Look in nsIEditProperty.h for this
  nsCOMPtr<nsIAtom> styleAtom = do_GetAtom(prop);
  if (! styleAtom)
    return NS_ERROR_OUT_OF_MEMORY;

  nsresult  err = NS_NOINTERFACE;

  nsCOMPtr<nsIHTMLEditor> htmlEditor = do_QueryInterface(aEditor,&err);
  if (htmlEditor)
    err = htmlEditor->SetInlineProperty(styleAtom,
                                nsDependentString(attr?attr:&sEmptyStr),
                                nsDependentString(value?value:&sEmptyStr));

  return err;
}
