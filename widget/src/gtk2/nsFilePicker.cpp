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
 * The Original Code is the Mozilla GTK2 File Chooser.
 *
 * The Initial Developer of the Original Code is Red Hat, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Christopher Aillon <caillon@redhat.com>
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

#include <gtk/gtkwindow.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkimage.h>

#include "nsIFileURL.h"
#include "nsIURI.h"
#include "nsIWidget.h"
#include "nsILocalFile.h"
#include "nsIStringBundle.h"

#include "nsArrayEnumerator.h"
#include "nsMemory.h"
#include "nsEnumeratorUtils.h"
#include "nsNetUtil.h"
#include "nsReadableUtils.h"
#include "mozcontainer.h"

#include "prmem.h"
#include "prlink.h"

#include "nsFilePicker.h"
#include "nsAccessibilityHelper.h"

#define DECL_FUNC_PTR(func) static _##func##_fn _##func
#define GTK_FILE_CHOOSER(widget) ((GtkFileChooser*) widget)

#define MAX_PREVIEW_SIZE 180

PRLibrary    *nsFilePicker::mGTK24 = nsnull;
nsILocalFile *nsFilePicker::mPrevDisplayDirectory = nsnull;

// XXX total ass.  We should really impose a build-time requirement on gtk2.4
// and then check at run-time whether the user is actually running gtk2.4.
// We should then decide to load the component (or not) from the component mgr.
// We don't really have a mechanism to do that, though....

typedef struct _GtkFileChooser GtkFileChooser;
typedef struct _GtkFileFilter GtkFileFilter;

/* Copied from gtkfilechooser.h */
typedef enum
{
  GTK_FILE_CHOOSER_ACTION_OPEN,
  GTK_FILE_CHOOSER_ACTION_SAVE,
  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
} GtkFileChooserAction;


typedef gchar* (*_gtk_file_chooser_get_filename_fn)(GtkFileChooser *chooser);
typedef GSList* (*_gtk_file_chooser_get_filenames_fn)(GtkFileChooser *chooser);
typedef gchar* (*_gtk_file_chooser_get_uri_fn)(GtkFileChooser *chooser);
typedef GSList* (*_gtk_file_chooser_get_uris_fn)(GtkFileChooser *chooser);
typedef GtkWidget* (*_gtk_file_chooser_dialog_new_fn)(const gchar *title,
                                                      GtkWindow *parent,
                                                      GtkFileChooserAction action,
                                                      const gchar *first_button_text,
                                                      ...);
typedef void (*_gtk_file_chooser_set_select_multiple_fn)(GtkFileChooser* chooser, gboolean truth);
typedef void (*_gtk_file_chooser_set_do_overwrite_confirmation_fn)(GtkFileChooser* chooser, gboolean do_confirm);
typedef void (*_gtk_file_chooser_set_current_name_fn)(GtkFileChooser* chooser, const gchar* name);
typedef void (*_gtk_file_chooser_set_current_folder_fn)(GtkFileChooser* chooser, const gchar* folder);
typedef void (*_gtk_file_chooser_add_filter_fn)(GtkFileChooser* chooser, GtkFileFilter* filter);
typedef void (*_gtk_file_chooser_set_filter_fn)(GtkFileChooser* chooser, GtkFileFilter* filter);
typedef GtkFileFilter* (*_gtk_file_chooser_get_filter_fn)(GtkFileChooser* chooser);
typedef GSList* (*_gtk_file_chooser_list_filters_fn)(GtkFileChooser* chooser);
typedef GtkFileFilter* (*_gtk_file_filter_new_fn)();
typedef void (*_gtk_file_filter_add_pattern_fn)(GtkFileFilter* filter, const gchar* pattern);
typedef void (*_gtk_file_filter_set_name_fn)(GtkFileFilter* filter, const gchar* name);
typedef char* (*_gtk_file_chooser_get_preview_filename_fn)(GtkFileChooser *chooser);
typedef void (*_gtk_file_chooser_set_preview_widget_active_fn)(GtkFileChooser *chooser, gboolean active);
typedef void (*_gtk_image_set_from_pixbuf_fn)(GtkImage *image, GdkPixbuf *pixbuf);
typedef void (*_gtk_file_chooser_set_preview_widget_fn)(GtkFileChooser *chooser, GtkWidget *preview_widget);
typedef GtkWidget* (*_gtk_image_new_fn)();
typedef void (*_gtk_misc_set_padding_fn)(GtkMisc *misc, gint xpad, gint ypad);
typedef void (*_gtk_file_chooser_set_local_only_fn)(GtkFileChooser *chooser, gboolean local_only);

DECL_FUNC_PTR(gtk_file_chooser_get_filename);
DECL_FUNC_PTR(gtk_file_chooser_get_filenames);
DECL_FUNC_PTR(gtk_file_chooser_get_uri);
DECL_FUNC_PTR(gtk_file_chooser_get_uris);
DECL_FUNC_PTR(gtk_file_chooser_dialog_new);
DECL_FUNC_PTR(gtk_file_chooser_set_select_multiple);
DECL_FUNC_PTR(gtk_file_chooser_set_do_overwrite_confirmation);
DECL_FUNC_PTR(gtk_file_chooser_set_current_name);
DECL_FUNC_PTR(gtk_file_chooser_set_current_folder);
DECL_FUNC_PTR(gtk_file_chooser_add_filter);
DECL_FUNC_PTR(gtk_file_chooser_set_filter);
DECL_FUNC_PTR(gtk_file_chooser_get_filter);
DECL_FUNC_PTR(gtk_file_chooser_list_filters);
DECL_FUNC_PTR(gtk_file_filter_new);
DECL_FUNC_PTR(gtk_file_filter_add_pattern);
DECL_FUNC_PTR(gtk_file_filter_set_name);
DECL_FUNC_PTR(gtk_file_chooser_get_preview_filename);
DECL_FUNC_PTR(gtk_file_chooser_set_preview_widget_active);
DECL_FUNC_PTR(gtk_image_set_from_pixbuf);
DECL_FUNC_PTR(gtk_file_chooser_set_preview_widget);
DECL_FUNC_PTR(gtk_image_new);
DECL_FUNC_PTR(gtk_misc_set_padding);
DECL_FUNC_PTR(gtk_file_chooser_set_local_only);

static GtkWindow *
get_gtk_window_for_nsiwidget(nsIWidget *widget)
{
  // Get native GdkWindow
  GdkWindow *gdk_win = GDK_WINDOW(widget->GetNativeData(NS_NATIVE_WIDGET));
  if (!gdk_win)
    return NULL;

  // Get the container
  gpointer user_data = NULL;
  gdk_window_get_user_data(gdk_win, &user_data);
  if (!user_data)
    return NULL;

  // Make sure its really a container
  MozContainer *parent_container = MOZ_CONTAINER(user_data);
  if (!parent_container)
    return NULL;

  // Get its toplevel
  return GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(parent_container)));
}

static PRLibrary *
LoadVersionedLibrary(const char* libName, const char* libVersion)
{
  char *platformLibName = PR_GetLibraryName(nsnull, libName);
  nsCAutoString versionLibName(platformLibName);
  versionLibName.Append(libVersion);
  PR_FreeLibraryName(platformLibName);
  return PR_LoadLibrary(versionLibName.get());
}

/* static */
nsresult
nsFilePicker::LoadSymbolsGTK24()
{
  static PRBool initialized;
  if (initialized) {
    return NS_OK;
  }

  #define GET_LIBGTK_FUNC_BASE(func, onerr)                  \
    PR_BEGIN_MACRO \
    _##func = (_##func##_fn) PR_FindFunctionSymbol(mGTK24, #func); \
    if (!_##func) { \
      NS_WARNING("Can't load gtk symbol " #func); \
      onerr \
    } \
    PR_END_MACRO

  #define GET_LIBGTK_FUNC(func) \
    GET_LIBGTK_FUNC_BASE(func, return NS_ERROR_NOT_AVAILABLE;)

  #define GET_LIBGTK_FUNC_OPT(func) \
    GET_LIBGTK_FUNC_BASE(func, ;)

  PRFuncPtr func = PR_FindFunctionSymbolAndLibrary("gtk_file_chooser_get_filename",
                                                   &mGTK24);
  if (mGTK24) {
    _gtk_file_chooser_get_filename = (_gtk_file_chooser_get_filename_fn)func;
  } else {
    // XXX hmm, this seems to fail when gtk 2.4 is already loaded...
    mGTK24 = LoadVersionedLibrary("gtk-2", ".4");
    if (!mGTK24) {
      return NS_ERROR_NOT_AVAILABLE;
    }
    GET_LIBGTK_FUNC(gtk_file_chooser_get_filename);
  }

  GET_LIBGTK_FUNC(gtk_file_chooser_get_filenames);
  GET_LIBGTK_FUNC(gtk_file_chooser_get_uri);
  GET_LIBGTK_FUNC(gtk_file_chooser_get_uris);
  GET_LIBGTK_FUNC(gtk_file_chooser_dialog_new);
  GET_LIBGTK_FUNC(gtk_file_chooser_set_select_multiple);
  GET_LIBGTK_FUNC_OPT(gtk_file_chooser_set_do_overwrite_confirmation);
  GET_LIBGTK_FUNC(gtk_file_chooser_set_current_name);
  GET_LIBGTK_FUNC(gtk_file_chooser_set_current_folder);
  GET_LIBGTK_FUNC(gtk_file_chooser_add_filter);
  GET_LIBGTK_FUNC(gtk_file_chooser_set_filter);
  GET_LIBGTK_FUNC(gtk_file_chooser_get_filter);
  GET_LIBGTK_FUNC(gtk_file_chooser_list_filters);
  GET_LIBGTK_FUNC(gtk_file_filter_new);
  GET_LIBGTK_FUNC(gtk_file_filter_add_pattern);
  GET_LIBGTK_FUNC(gtk_file_filter_set_name);
  GET_LIBGTK_FUNC(gtk_file_chooser_get_preview_filename);
  GET_LIBGTK_FUNC(gtk_file_chooser_set_preview_widget_active);
  GET_LIBGTK_FUNC(gtk_image_set_from_pixbuf);
  GET_LIBGTK_FUNC(gtk_file_chooser_set_preview_widget);
  GET_LIBGTK_FUNC(gtk_image_new);
  GET_LIBGTK_FUNC(gtk_misc_set_padding);
  GET_LIBGTK_FUNC(gtk_file_chooser_set_local_only);

  initialized = PR_TRUE;

  // Woot.
  return NS_OK;
}

void
nsFilePicker::Shutdown()
{
  if (mGTK24) {
    PR_UnloadLibrary(mGTK24);
    mGTK24 = nsnull;
  }

  NS_IF_RELEASE(mPrevDisplayDirectory);
}

// Ok, lib loading crap is done... the real code starts here:

static GtkFileChooserAction
GetGtkFileChooserAction(PRInt16 aMode)
{
  GtkFileChooserAction action;

  switch (aMode) {
    case nsIFilePicker::modeSave:
    action = GTK_FILE_CHOOSER_ACTION_SAVE;
    break;

    case nsIFilePicker::modeGetFolder:
    action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    break;

    default:
    NS_WARNING("Unknown nsIFilePicker mode");
    // fall through

    case nsIFilePicker::modeOpen:
    case nsIFilePicker::modeOpenMultiple:
    action = GTK_FILE_CHOOSER_ACTION_OPEN;
    break;
  }

  return action;
}


static void
UpdateFilePreviewWidget(GtkFileChooser *file_chooser,
                        gpointer preview_widget_voidptr)
{
  GtkImage *preview_widget = GTK_IMAGE(preview_widget_voidptr);
  char *image_filename = _gtk_file_chooser_get_preview_filename(file_chooser);

  if (!image_filename) {
    _gtk_file_chooser_set_preview_widget_active(file_chooser, FALSE);
    return;
  }

  // We do this so GTK scales down images that are too big, but not scale up images that are too small
  GdkPixbuf *preview_pixbuf = gdk_pixbuf_new_from_file(image_filename, NULL);
  if (!preview_pixbuf) {
    g_free(image_filename);
    _gtk_file_chooser_set_preview_widget_active(file_chooser, FALSE);
    return;
  }
  if (gdk_pixbuf_get_width(preview_pixbuf) > MAX_PREVIEW_SIZE || gdk_pixbuf_get_height(preview_pixbuf) > MAX_PREVIEW_SIZE) {
    g_object_unref(preview_pixbuf);
    preview_pixbuf = gdk_pixbuf_new_from_file_at_size(image_filename, MAX_PREVIEW_SIZE, MAX_PREVIEW_SIZE, NULL);
  }

  g_free(image_filename);
  if (!preview_pixbuf) {
    _gtk_file_chooser_set_preview_widget_active(file_chooser, FALSE);
    return;
  }

  // This is the easiest way to do center alignment without worrying about containers
  // Minimum 3px padding each side (hence the 6) just to make things nice
  gint x_padding = (MAX_PREVIEW_SIZE + 6 - gdk_pixbuf_get_width(preview_pixbuf)) / 2;
  _gtk_misc_set_padding(GTK_MISC(preview_widget), x_padding, 0);

  _gtk_image_set_from_pixbuf(preview_widget, preview_pixbuf);
  g_object_unref(preview_pixbuf);
  _gtk_file_chooser_set_preview_widget_active(file_chooser, TRUE);
}

NS_IMPL_ISUPPORTS1(nsFilePicker, nsIFilePicker)

nsFilePicker::nsFilePicker()
  : mMode(nsIFilePicker::modeOpen),
    mSelectedType(0)
{
}

nsFilePicker::~nsFilePicker()
{
}

void
ReadMultipleFiles(gpointer filename, gpointer array)
{
  nsCOMPtr<nsILocalFile> localfile;
  nsresult rv = NS_NewNativeLocalFile(nsDependentCString(static_cast<char*>(filename)),
                                      PR_FALSE,
                                      getter_AddRefs(localfile));
  if (NS_SUCCEEDED(rv)) {
    nsCOMArray<nsILocalFile>& files = *static_cast<nsCOMArray<nsILocalFile>*>(array);
    files.AppendObject(localfile);
  }

  g_free(filename);
}

void
nsFilePicker::ReadValuesFromFileChooser(GtkWidget *file_chooser)
{
  mFiles.Clear();

  if (mMode == nsIFilePicker::modeOpenMultiple) {
    mFileURL.Truncate();

    GSList *list = _gtk_file_chooser_get_filenames (GTK_FILE_CHOOSER(file_chooser));
    g_slist_foreach(list, ReadMultipleFiles, static_cast<gpointer>(&mFiles));
    g_slist_free(list);
  } else {
    gchar *filename = _gtk_file_chooser_get_uri (GTK_FILE_CHOOSER(file_chooser));
    mFileURL.Assign(filename);
    g_free(filename);
  }

  GtkFileFilter *filter = _gtk_file_chooser_get_filter (GTK_FILE_CHOOSER(file_chooser));
  GSList *filter_list = _gtk_file_chooser_list_filters (GTK_FILE_CHOOSER(file_chooser));

  mSelectedType = static_cast<PRInt16>(g_slist_index (filter_list, filter));
  g_slist_free(filter_list);

  // Remember last used directory.
  nsCOMPtr<nsILocalFile> file;
  GetFile(getter_AddRefs(file));
  if (file) {
    nsCOMPtr<nsIFile> dir;
    file->GetParent(getter_AddRefs(dir));
    nsCOMPtr<nsILocalFile> localDir(do_QueryInterface(dir));
    if (localDir) {
      localDir.swap(mPrevDisplayDirectory);
    }
  }
}

NS_IMETHODIMP
nsFilePicker::Init(nsIDOMWindow *aParent, const nsAString &aTitle, PRInt16 aMode)
{
  nsresult rv = LoadSymbolsGTK24();
  if (NS_FAILED(rv)) {
    return rv;
  }

  return nsBaseFilePicker::Init(aParent, aTitle, aMode);
}

void
nsFilePicker::InitNative(nsIWidget *aParent,
                         const nsAString& aTitle,
                         PRInt16 aMode)
{
  mParentWidget = aParent;
  mTitle.Assign(aTitle);
  mMode = aMode;
}

NS_IMETHODIMP
nsFilePicker::AppendFilters(PRInt32 aFilterMask)
{
  mAllowURLs = !!(aFilterMask & filterAllowURLs);
  return nsBaseFilePicker::AppendFilters(aFilterMask);
}

NS_IMETHODIMP
nsFilePicker::AppendFilter(const nsAString& aTitle, const nsAString& aFilter)
{
  if (aFilter.EqualsLiteral("..apps")) {
    // No platform specific thing we can do here, really....
    return NS_OK;
  }

  nsCAutoString filter, name;
  CopyUTF16toUTF8(aFilter, filter);
  CopyUTF16toUTF8(aTitle, name);

  mFilters.AppendCString(filter);
  mFilterNames.AppendCString(name);

  return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::SetDefaultString(const nsAString& aString)
{
  mDefault = aString;

  return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::GetDefaultString(nsAString& aString)
{
  // Per API...
  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
nsFilePicker::SetDefaultExtension(const nsAString& aExtension)
{
  mDefaultExtension = aExtension;

  return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::GetDefaultExtension(nsAString& aExtension)
{
  aExtension = mDefaultExtension;

  return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::GetFilterIndex(PRInt32 *aFilterIndex)
{
  *aFilterIndex = mSelectedType;

  return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::SetFilterIndex(PRInt32 aFilterIndex)
{
  mSelectedType = aFilterIndex;

  return NS_OK;
}

NS_IMETHODIMP
nsFilePicker::GetFile(nsILocalFile **aFile)
{
  NS_ENSURE_ARG_POINTER(aFile);

  *aFile = nsnull;
  nsCOMPtr<nsIURI> uri;
  nsresult rv = GetFileURL(getter_AddRefs(uri));
  if (!uri)
    return rv;

  nsCOMPtr<nsIFileURL> fileURL(do_QueryInterface(uri, &rv));
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIFile> file;
  rv = fileURL->GetFile(getter_AddRefs(file));
  NS_ENSURE_SUCCESS(rv, rv);

  return CallQueryInterface(file, aFile);
}

NS_IMETHODIMP
nsFilePicker::GetFileURL(nsIURI **aFileURL)
{
  *aFileURL = nsnull;
  return NS_NewURI(aFileURL, mFileURL);
}

NS_IMETHODIMP
nsFilePicker::GetFiles(nsISimpleEnumerator **aFiles)
{
  NS_ENSURE_ARG_POINTER(aFiles);

  if (mMode == nsIFilePicker::modeOpenMultiple) {
    return NS_NewArrayEnumerator(aFiles, mFiles);
  }

  return NS_ERROR_FAILURE;
}

PRBool
confirm_overwrite_file (GtkWidget *parent, nsILocalFile* file)
{
  nsCOMPtr<nsIStringBundleService> sbs = do_GetService(NS_STRINGBUNDLE_CONTRACTID);
  nsCOMPtr<nsIStringBundle> bundle;
  nsresult rv = sbs->CreateBundle("chrome://global/locale/filepicker.properties",
                                  getter_AddRefs(bundle));
  if (NS_FAILED(rv)) {
    return PR_FALSE;
  }

  nsAutoString leafName;
  file->GetLeafName(leafName);
  const PRUnichar *formatStrings[] =
  {
    leafName.get()
  };

  nsXPIDLString title, message;
  bundle->GetStringFromName(NS_LITERAL_STRING("confirmTitle").get(),
                            getter_Copies(title));
  bundle->FormatStringFromName(NS_LITERAL_STRING("confirmFileReplacing").get(),
                               formatStrings, NS_ARRAY_LENGTH(formatStrings),
                               getter_Copies(message));

  GtkWindow *parent_window = GTK_WINDOW(parent);
  GtkWidget *dialog;
  
  dialog = gtk_message_dialog_new(parent_window,
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_QUESTION,
                                  GTK_BUTTONS_YES_NO,
                                  NS_ConvertUTF16toUTF8(message).get());
  gtk_window_set_title(GTK_WINDOW(dialog), NS_ConvertUTF16toUTF8(title).get());
  if (parent_window && parent_window->group) {
    gtk_window_group_add_window(parent_window->group, GTK_WINDOW(dialog));
  }

  PRBool result = (RunDialog(GTK_DIALOG (dialog)) == GTK_RESPONSE_YES);

  gtk_widget_destroy (dialog);

  return result;
}

NS_IMETHODIMP
nsFilePicker::Show(PRInt16 *aReturn)
{
  NS_ENSURE_ARG_POINTER(aReturn);

  nsXPIDLCString title;
  title.Adopt(ToNewUTF8String(mTitle));

  GtkWindow *parent_widget = get_gtk_window_for_nsiwidget(mParentWidget);

  GtkFileChooserAction action = GetGtkFileChooserAction(mMode);
  const gchar *accept_button = (mMode == GTK_FILE_CHOOSER_ACTION_SAVE)
                               ? GTK_STOCK_SAVE : GTK_STOCK_OPEN;
  GtkWidget *file_chooser =
      _gtk_file_chooser_dialog_new(title, parent_widget, action,
                                   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                   accept_button, GTK_RESPONSE_ACCEPT,
                                   NULL);
  if (mAllowURLs) {
    _gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(file_chooser), FALSE);
  }

  if (mMode == GTK_FILE_CHOOSER_ACTION_OPEN || mMode == GTK_FILE_CHOOSER_ACTION_SAVE) {
    GtkWidget *img_preview = _gtk_image_new();
    _gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(file_chooser), img_preview);
    g_signal_connect(file_chooser, "update-preview", G_CALLBACK(UpdateFilePreviewWidget), img_preview);
  }

  if (parent_widget && parent_widget->group) {
    gtk_window_group_add_window(parent_widget->group, GTK_WINDOW(file_chooser));
  }

  if (mMode == nsIFilePicker::modeOpenMultiple) {
    _gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER(file_chooser), TRUE);
  } else if (mMode == nsIFilePicker::modeSave) {
    char *default_filename = ToNewUTF8String(mDefault);
    _gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(file_chooser),
                                       static_cast<const gchar*>(default_filename));
    nsMemory::Free(default_filename);
  }

  gtk_dialog_set_default_response(GTK_DIALOG(file_chooser), GTK_RESPONSE_ACCEPT);

  nsCAutoString directory;
  if (mDisplayDirectory) {
    mDisplayDirectory->GetNativePath(directory);
  } else if (mPrevDisplayDirectory) {
    mPrevDisplayDirectory->GetNativePath(directory);
  }

  if (!directory.IsEmpty()) {
    _gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
                                         directory.get());
  }

  PRInt32 count = mFilters.Count();
  for (PRInt32 i = 0; i < count; ++i) {
    // This is fun... the GTK file picker does not accept a list of filters
    // so we need to split out each string, and add it manually.

    char **patterns = g_strsplit(mFilters[i]->get(), ";", -1);
    if (!patterns) {
      return NS_ERROR_OUT_OF_MEMORY;
    }

    GtkFileFilter *filter = _gtk_file_filter_new ();
    for (int j = 0; patterns[j] != NULL; ++j) {
      _gtk_file_filter_add_pattern (filter, g_strstrip (patterns[j]));
    }

    g_strfreev(patterns);

    if (!mFilterNames[i]->IsEmpty()) {
      // If we have a name for our filter, let's use that.
      const char *filter_name = mFilterNames[i]->get();
      _gtk_file_filter_set_name (filter, filter_name);
    } else {
      // If we don't have a name, let's just use the filter pattern.
      const char *filter_pattern = mFilters[i]->get();
      _gtk_file_filter_set_name (filter, filter_pattern);
    }

    _gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (file_chooser), filter);

    // Set the initially selected filter
    if (mSelectedType == i) {
      _gtk_file_chooser_set_filter (GTK_FILE_CHOOSER(file_chooser), filter);
    }
  }

  PRBool checkForOverwrite = PR_TRUE;
  if (_gtk_file_chooser_set_do_overwrite_confirmation) {
    checkForOverwrite = PR_FALSE;
    // Only available in GTK 2.8
    _gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(file_chooser), PR_TRUE);
  }

  gint response = RunDialog(GTK_DIALOG (file_chooser));

  switch (response) {
    case GTK_RESPONSE_ACCEPT:
    ReadValuesFromFileChooser(file_chooser);
    *aReturn = nsIFilePicker::returnOK;
    if (mMode == nsIFilePicker::modeSave) {
      nsCOMPtr<nsILocalFile> file;
      GetFile(getter_AddRefs(file));
      if (file) {
        PRBool exists = PR_FALSE;
        file->Exists(&exists);
        if (exists) {
          PRBool overwrite = !checkForOverwrite ||
            confirm_overwrite_file (file_chooser, file);

          if (overwrite) {
            *aReturn = nsIFilePicker::returnReplace;
          } else {
            *aReturn = nsIFilePicker::returnCancel;
          }
        }
      }
    }
    break;

    case GTK_RESPONSE_CANCEL:
    case GTK_RESPONSE_CLOSE:
    case GTK_RESPONSE_DELETE_EVENT:
    *aReturn = nsIFilePicker::returnCancel;
    break;

    default:
    NS_WARNING("Unexpected response");
    *aReturn = nsIFilePicker::returnCancel;
    break;
  }

  gtk_widget_destroy(file_chooser);

  return NS_OK;
}
