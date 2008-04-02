/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
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
 * The Initial Developer of the Original Code is Christopher Blizzard
 * <blizzard@mozilla.org>.  Portions created by the Initial Developer
 * are Copyright (C) 2001 the Initial Developer. All Rights Reserved.
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

#include "nsIGenericFactory.h"
#include "nsWidgetsCID.h"
#include "nsAppShell.h"
#include "nsAppShellSingleton.h"
#include "nsBaseWidget.h"
#include "nsLookAndFeel.h"
#include "nsWindow.h"
#include "nsTransferable.h"
#include "nsClipboardHelper.h"
#include "nsHTMLFormatConverter.h"
#include "nsClipboard.h"
#include "nsDragService.h"
#include "nsFilePicker.h"
#include "nsSound.h"
#include "nsBidiKeyboard.h"
#include "nsNativeKeyBindings.h"
#include "nsScreenManagerGtk.h"
#include "nsPrintOptionsGTK.h"
#include "nsPrintSession.h"
#include "nsDeviceContextSpecG.h"
#include "nsIPrefService.h"
#include "nsIPrefBranch.h"
#include "nsImageToPixbuf.h"
#include "nsIdleServiceGTK.h"
#include "nsPrintDialogGTK.h"

#ifdef NATIVE_THEME_SUPPORT
#include "nsNativeThemeGTK.h"
#endif

#include "nsIComponentRegistrar.h"
#include "nsComponentManagerUtils.h"
#include "nsAutoPtr.h"
#include <gtk/gtk.h>

/* from nsFilePicker.js */
#define XULFILEPICKER_CID \
  { 0x54ae32f8, 0x1dd2, 0x11b2, \
    { 0xa2, 0x09, 0xdf, 0x7c, 0x50, 0x53, 0x70, 0xf8} }
static NS_DEFINE_CID(kXULFilePickerCID, XULFILEPICKER_CID);
static NS_DEFINE_CID(kNativeFilePickerCID, NS_FILEPICKER_CID);

NS_GENERIC_FACTORY_CONSTRUCTOR(nsWindow)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsChildWindow)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsLookAndFeel)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsTransferable)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBidiKeyboard)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsClipboardHelper)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsHTMLFormatConverter)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsClipboard, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDragService)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsSound)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScreenManagerGtk)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDeviceContextSpecGTK)
#ifdef NATIVE_THEME_SUPPORT
NS_GENERIC_FACTORY_CONSTRUCTOR(nsNativeThemeGTK)
#endif
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsPrintOptionsGTK, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsPrinterEnumeratorGTK)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsPrintSession, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsImageToPixbuf)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsIdleServiceGTK)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsPrintDialogServiceGTK, Init)

static NS_IMETHODIMP
nsFilePickerConstructor(nsISupports *aOuter, REFNSIID aIID,
                        void **aResult)
{
  *aResult = nsnull;
  if (aOuter != nsnull) {
    return NS_ERROR_NO_AGGREGATION;
  }

  PRBool allowPlatformPicker = PR_TRUE;
  nsCOMPtr<nsIPrefBranch> prefs = do_GetService(NS_PREFSERVICE_CONTRACTID);
  if (prefs) {
    PRBool prefAllow;
    nsresult rv = prefs->GetBoolPref("ui.allow_platform_file_picker",
                                     &prefAllow);
    if (NS_SUCCEEDED(rv)) {
        allowPlatformPicker = prefAllow;
    }
  }
  
  nsCOMPtr<nsIFilePicker> picker;
  if (allowPlatformPicker && gtk_check_version(2,6,3) == NULL) {
      picker = new nsFilePicker;
  } else {
    picker = do_CreateInstance(kXULFilePickerCID);
  }

  if (!picker) {
    return NS_ERROR_OUT_OF_MEMORY;
  }

  return picker->QueryInterface(aIID, aResult);
}

static NS_IMETHODIMP
nsNativeKeyBindingsConstructor(nsISupports *aOuter, REFNSIID aIID,
                               void **aResult,
                               NativeKeyBindingsType aKeyBindingsType)
{
    nsresult rv;

    nsNativeKeyBindings *inst;

    *aResult = NULL;
    if (NULL != aOuter) {
        rv = NS_ERROR_NO_AGGREGATION;
        return rv;
    }

    NS_NEWXPCOM(inst, nsNativeKeyBindings);
    if (NULL == inst) {
        rv = NS_ERROR_OUT_OF_MEMORY;
        return rv;
    }
    NS_ADDREF(inst);
    inst->Init(aKeyBindingsType);
    rv = inst->QueryInterface(aIID, aResult);
    NS_RELEASE(inst);

    return rv;
}

static NS_IMETHODIMP
nsNativeKeyBindingsInputConstructor(nsISupports *aOuter, REFNSIID aIID,
                                    void **aResult)
{
    return nsNativeKeyBindingsConstructor(aOuter, aIID, aResult,
                                          eKeyBindings_Input);
}

static NS_IMETHODIMP
nsNativeKeyBindingsTextAreaConstructor(nsISupports *aOuter, REFNSIID aIID,
                                       void **aResult)
{
    return nsNativeKeyBindingsConstructor(aOuter, aIID, aResult,
                                          eKeyBindings_TextArea);
}

static const nsModuleComponentInfo components[] =
{
    { "Gtk2 Window",
      NS_WINDOW_CID,
      "@mozilla.org/widget/window/gtk;1",
      nsWindowConstructor },
    { "Gtk2 Child Window",
      NS_CHILD_CID,
      "@mozilla.org/widgets/child_window/gtk;1",
      nsChildWindowConstructor },
    { "Gtk2 AppShell",
      NS_APPSHELL_CID,
      "@mozilla.org/widget/appshell/gtk;1",
      nsAppShellConstructor },
    { "Gtk2 Look And Feel",
      NS_LOOKANDFEEL_CID,
      "@mozilla.org/widget/lookandfeel;1",
      nsLookAndFeelConstructor },
    { "Gtk2 File Picker",
      NS_FILEPICKER_CID,
      "@mozilla.org/filepicker;1",
      nsFilePickerConstructor },
    { "Gtk2 Sound",
      NS_SOUND_CID,
      "@mozilla.org/sound;1",
      nsSoundConstructor },
  { "Transferable",
    NS_TRANSFERABLE_CID,
    "@mozilla.org/widget/transferable;1",
    nsTransferableConstructor },
  { "Gtk Clipboard",
    NS_CLIPBOARD_CID,
    "@mozilla.org/widget/clipboard;1",
    nsClipboardConstructor },
  { "Clipboard Helper",
    NS_CLIPBOARDHELPER_CID,
    "@mozilla.org/widget/clipboardhelper;1",
    nsClipboardHelperConstructor },
  { "Gtk Drag Service",
    NS_DRAGSERVICE_CID,
    "@mozilla.org/widget/dragservice;1",
    nsDragServiceConstructor },
  { "HTML Format Converter",
    NS_HTMLFORMATCONVERTER_CID,
    "@mozilla.org/widget/htmlformatconverter;1",
    nsHTMLFormatConverterConstructor },
  { "Gtk2 Bidi Keyboard",
    NS_BIDIKEYBOARD_CID,
    "@mozilla.org/widget/bidikeyboard;1",
    nsBidiKeyboardConstructor },
  { "Input Native Keybindings",
    NS_NATIVEKEYBINDINGSINPUT_CID,
    NS_NATIVEKEYBINDINGSINPUT_CONTRACTID,
    nsNativeKeyBindingsInputConstructor },
  { "TextArea Native Keybindings",
    NS_NATIVEKEYBINDINGSTEXTAREA_CID,
    NS_NATIVEKEYBINDINGSTEXTAREA_CONTRACTID,
    nsNativeKeyBindingsTextAreaConstructor },
  { "Editor Native Keybindings",
    NS_NATIVEKEYBINDINGSEDITOR_CID,
    NS_NATIVEKEYBINDINGSEDITOR_CONTRACTID,
    nsNativeKeyBindingsTextAreaConstructor },
  { "Gtk Screen Manager",
    NS_SCREENMANAGER_CID,
    "@mozilla.org/gfx/screenmanager;1",
    nsScreenManagerGtkConstructor },
#ifdef NATIVE_THEME_SUPPORT
   { "Native Theme Renderer",
    NS_THEMERENDERER_CID,
    "@mozilla.org/chrome/chrome-native-theme;1",
     nsNativeThemeGTKConstructor },
#endif
  { "PrintSettings Service",
    NS_PRINTSETTINGSSERVICE_CID,
    "@mozilla.org/gfx/printsettings-service;1",
    nsPrintOptionsGTKConstructor },
  { "Gtk Printer Enumerator",
    NS_PRINTER_ENUMERATOR_CID,
    //    "@mozilla.org/gfx/printer_enumerator/gtk;1",
    "@mozilla.org/gfx/printerenumerator;1",
    nsPrinterEnumeratorGTKConstructor },
  { "Print Session",
    NS_PRINTSESSION_CID,
    "@mozilla.org/gfx/printsession;1",
    nsPrintSessionConstructor },
  { "Gtk Device Context Spec",
    NS_DEVICE_CONTEXT_SPEC_CID,
    //    "@mozilla.org/gfx/device_context_spec/gtk;1",
    "@mozilla.org/gfx/devicecontextspec;1",
    nsDeviceContextSpecGTKConstructor },
  { "Image to gdk-pixbuf converter",
    NS_IMAGE_TO_PIXBUF_CID,
    "@mozilla.org/widget/image-to-gdk-pixbuf;1",
    nsImageToPixbufConstructor },
  { "User Idle Service",
    NS_IDLE_SERVICE_CID,
    "@mozilla.org/widget/idleservice;1",
    nsIdleServiceGTKConstructor },
  { "Native Print Dialog",
    NS_PRINTDIALOGSERVICE_CID,
    NS_PRINTDIALOGSERVICE_CONTRACTID,
    nsPrintDialogServiceGTKConstructor },
};

PR_STATIC_CALLBACK(void)
nsWidgetGtk2ModuleDtor(nsIModule *aSelf)
{
  nsFilePicker::Shutdown();
  nsSound::Shutdown();
  nsWindow::ReleaseGlobals();
  nsAppShellShutdown(aSelf);
}

NS_IMPL_NSGETMODULE_WITH_CTOR_DTOR(nsWidgetGtk2Module,
                                   components,
                                   nsAppShellInit,
                                   nsWidgetGtk2ModuleDtor)
