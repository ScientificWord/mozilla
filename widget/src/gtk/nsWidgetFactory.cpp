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
 * Christopher Blizzard.
 * Portions created by the Initial Developer are Copyright (C) 2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Christopher Blizzzard <blizzard@mozilla.org>
 *   Stuart Parmenter <pavlov@netscape.com>
 *   Dan Rosen <dr@netscape.com>
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
#include "nsIModule.h"
#include "nsCOMPtr.h"
#include "nsWidgetsCID.h"

#include "nsWindow.h"
#include "nsAppShell.h"
#include "nsAppShellSingleton.h"
#include "nsToolkit.h"
#include "nsLookAndFeel.h"
#include "nsTransferable.h"
#include "nsClipboard.h"
#include "nsClipboardHelper.h"
#include "nsHTMLFormatConverter.h"
#include "nsDragService.h"
#include "nsSound.h"
#include "nsBidiKeyboard.h"
#include "nsScreenManagerGtk.h"
#ifdef NATIVE_THEME_SUPPORT
#include "nsNativeThemeGTK.h"
#endif

#include "nsGtkIMEHelper.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(nsWindow)
NS_GENERIC_FACTORY_CONSTRUCTOR(ChildWindow)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsToolkit)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsLookAndFeel)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsTransferable)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsClipboard)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsClipboardHelper)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsHTMLFormatConverter)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDragService)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsSound)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsBidiKeyboard)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScreenManagerGtk)
#ifdef NATIVE_THEME_SUPPORT
NS_GENERIC_FACTORY_CONSTRUCTOR(nsNativeThemeGTK)
#endif

static const nsModuleComponentInfo components[] =
{
  { "Gtk nsWindow",
    NS_WINDOW_CID,
    "@mozilla.org/widgets/window/gtk;1",
    nsWindowConstructor },
  { "Gtk Child nsWindow",
    NS_CHILD_CID,
    "@mozilla.org/widgets/child_window/gtk;1",
    ChildWindowConstructor },
  { "Gtk AppShell",
    NS_APPSHELL_CID,
    "@mozilla.org/widget/appshell/gtk;1",
    nsAppShellConstructor },
  { "Gtk Toolkit",
    NS_TOOLKIT_CID,
    "@mozilla.org/widget/toolkit/gtk;1",
    nsToolkitConstructor },
  { "Gtk Look And Feel",
    NS_LOOKANDFEEL_CID,
    "@mozilla.org/widget/lookandfeel;1",
    nsLookAndFeelConstructor },
  { "Gtk Sound",
    NS_SOUND_CID,
    //    "@mozilla.org/widget/sound/gtk;1"
    "@mozilla.org/sound;1",
    nsSoundConstructor },
  { "Transferable",
    NS_TRANSFERABLE_CID,
    //    "@mozilla.org/widget/transferable/gtk;1",
    "@mozilla.org/widget/transferable;1",
    nsTransferableConstructor },
  { "Gtk Clipboard",
    NS_CLIPBOARD_CID,
    //    "@mozilla.org/widget/clipboard/gtk;1",
    "@mozilla.org/widget/clipboard;1",
    nsClipboardConstructor },
  { "Clipboard Helper",
    NS_CLIPBOARDHELPER_CID,
    "@mozilla.org/widget/clipboardhelper;1",
    nsClipboardHelperConstructor },
  { "HTML Format Converter",
    NS_HTMLFORMATCONVERTER_CID,
    "@mozilla.org/widget/htmlformatconverter;1",
    nsHTMLFormatConverterConstructor },
  { "Gtk Drag Service",
    NS_DRAGSERVICE_CID,
    //    "@mozilla.org/widget/dragservice/gtk;1",
    "@mozilla.org/widget/dragservice;1",
    nsDragServiceConstructor },
  { "Gtk Bidi Keyboard",
    NS_BIDIKEYBOARD_CID,
    "@mozilla.org/widget/bidikeyboard;1",
    nsBidiKeyboardConstructor },
  { "Gtk Screen Manager",
    NS_SCREENMANAGER_CID,
    "@mozilla.org/gfx/screenmanager;1",
    nsScreenManagerGtkConstructor },
#ifdef NATIVE_THEME_SUPPORT
   { "Native Theme Renderer",
    NS_THEMERENDERER_CID,
    "@mozilla.org/chrome/chrome-native-theme;1",
    nsNativeThemeGTKConstructor }
#endif
};

PR_STATIC_CALLBACK(void)
nsWidgetGTKModuleDtor(nsIModule *self)
{
  nsWindow::ReleaseGlobals();
  nsGtkIMEHelper::Shutdown();
  nsAppShellShutdown(self);
}

NS_IMPL_NSGETMODULE_WITH_CTOR_DTOR(nsWidgetGTKModule,
                                   components,
                                   nsAppShellInit,
                                   nsWidgetGTKModuleDtor)
