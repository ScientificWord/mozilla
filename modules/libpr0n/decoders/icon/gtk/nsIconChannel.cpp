/* vim:set ts=2 sw=2 sts=2 cin et: */
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
 * The Original Code is the Mozilla icon channel for gnome.
 *
 * The Initial Developer of the Original Code is
 * Christian Biesinger <cbiesinger@web.de>.
 * Portions created by the Initial Developer are Copyright (C) 2004
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

#include <stdlib.h>
#include <unistd.h>

#ifdef MOZ_ENABLE_GNOMEUI
// Older versions of these headers seem to be missing an extern "C"
extern "C" {
#include <libgnome/libgnome.h>
#include <libgnomeui/gnome-icon-theme.h>
#include <libgnomeui/gnome-icon-lookup.h>

#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-ops.h>
}
#endif

#include <gtk/gtkwidget.h>
#include <gtk/gtkiconfactory.h>
#include <gtk/gtkimage.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkfixed.h>
#include <gtk/gtkversion.h>

#include "nsIMIMEService.h"

#include "nsIStringBundle.h"

#include "nsNetUtil.h"
#include "nsIURL.h"
#include "prlink.h"

#include "nsIconChannel.h"

NS_IMPL_ISUPPORTS2(nsIconChannel,
                   nsIRequest,
                   nsIChannel)

#ifdef MOZ_ENABLE_GNOMEUI
// These let us have a soft dependency on libgnomeui rather than a hard one. These are just basically the prototypes
// of the functions in the libraries.
typedef char* (*_GnomeIconLookup_fn)(GtkIconTheme *icon_theme, GnomeThumbnailFactory *thumbnail_factory,
                                     const char *file_uri, const char *custom_icon, GnomeVFSFileInfo *file_info,
                                     const char *mime_type, GnomeIconLookupFlags flags, GnomeIconLookupResultFlags *result);
typedef GnomeIconTheme* (*_GnomeIconThemeNew_fn)(void);
typedef char* (*_GnomeIconThemeLookupIcon_fn)(GnomeIconTheme *theme, const char *icon_name, int size,
                                              const GnomeIconData **icon_data, int *base_size);
typedef int (*_GnomeInit_fn)(const char *app_id, const char *app_version, int argc, char **argv, const struct poptOption *options,
                             int flags, poptContext *return_ctx);
typedef GnomeProgram* (*_GnomeProgramGet_fn)(void);
typedef GnomeVFSResult (*_GnomeVFSGetFileInfo_fn)(const gchar *text_uri, GnomeVFSFileInfo *info, GnomeVFSFileInfoOptions options);
typedef void (*_GnomeVFSFileInfoClear_fn)(GnomeVFSFileInfo *info);

static PRLibrary* gLibGnomeUI = nsnull;
static PRLibrary* gLibGnome = nsnull;
static PRLibrary* gLibGnomeVFS = nsnull;
static PRBool gTriedToLoadGnomeLibs = PR_FALSE;

static _GnomeIconLookup_fn _gnome_icon_lookup = nsnull;
static _GnomeIconThemeNew_fn _gnome_icon_theme_new = nsnull;
static _GnomeIconThemeLookupIcon_fn _gnome_icon_theme_lookup_icon = nsnull;
static _GnomeInit_fn _gnome_init = nsnull;
static _GnomeProgramGet_fn _gnome_program_get = nsnull;
static _GnomeVFSGetFileInfo_fn _gnome_vfs_get_file_info = nsnull;
static _GnomeVFSFileInfoClear_fn _gnome_vfs_file_info_clear = nsnull;
#endif

static nsresult
moz_gdk_pixbuf_to_channel(GdkPixbuf* aPixbuf, nsIURI *aURI,
                          nsIChannel **aChannel)
{
  int width = gdk_pixbuf_get_width(aPixbuf);
  int height = gdk_pixbuf_get_height(aPixbuf);
  NS_ENSURE_TRUE(height < 256 && width < 256 && height > 0 && width > 0 &&
                 gdk_pixbuf_get_colorspace(aPixbuf) == GDK_COLORSPACE_RGB &&
                 gdk_pixbuf_get_bits_per_sample(aPixbuf) == 8 &&
                 gdk_pixbuf_get_has_alpha(aPixbuf) &&
                 gdk_pixbuf_get_n_channels(aPixbuf) == 4,
                 NS_ERROR_UNEXPECTED);

  const int n_channels = 4;
  gsize buf_size = 2 + n_channels * height * width;
  PRUint8 * const buf = (PRUint8*)NS_Alloc(buf_size);
  NS_ENSURE_TRUE(buf, NS_ERROR_OUT_OF_MEMORY);
  PRUint8 *out = buf;

  *(out++) = width;
  *(out++) = height;

  const guchar * const pixels = gdk_pixbuf_get_pixels(aPixbuf);
  int rowextra = gdk_pixbuf_get_rowstride(aPixbuf) - width * n_channels;

  // encode the RGB data and the A data
  const guchar * in = pixels;
  for (int y = 0; y < height; ++y, in += rowextra) {
    for (int x = 0; x < width; ++x) {
      PRUint8 r = *(in++);
      PRUint8 g = *(in++);
      PRUint8 b = *(in++);
      PRUint8 a = *(in++);
#define DO_PREMULTIPLY(c_) PRUint8(PRUint16(c_) * PRUint16(a) / PRUint16(255))
#ifdef IS_LITTLE_ENDIAN
      *(out++) = DO_PREMULTIPLY(b);
      *(out++) = DO_PREMULTIPLY(g);
      *(out++) = DO_PREMULTIPLY(r);
      *(out++) = a;
#else
      *(out++) = a;
      *(out++) = DO_PREMULTIPLY(r);
      *(out++) = DO_PREMULTIPLY(g);
      *(out++) = DO_PREMULTIPLY(b);
#endif
#undef DO_PREMULTIPLY
    }
  }

  NS_ASSERTION(out == buf + buf_size, "size miscalculation");

  nsresult rv;
  nsCOMPtr<nsIStringInputStream> stream =
    do_CreateInstance("@mozilla.org/io/string-input-stream;1", &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = stream->AdoptData((char*)buf, buf_size);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = NS_NewInputStreamChannel(aChannel, aURI, stream,
                                NS_LITERAL_CSTRING("image/icon"));
  return rv;
}

static GtkWidget *gProtoWindow = nsnull;
static GtkWidget *gStockImageWidget = nsnull;
#ifdef MOZ_ENABLE_GNOMEUI
static GnomeIconTheme *gIconTheme = nsnull;
#endif

#if GTK_CHECK_VERSION(2,4,0)
static GtkIconFactory *gIconFactory = nsnull;
#endif

static void
ensure_stock_image_widget()
{
  if (!gProtoWindow) {
    gProtoWindow = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_widget_realize(gProtoWindow);
    GtkWidget* protoLayout = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(gProtoWindow), protoLayout);

    gStockImageWidget = gtk_image_new();
    gtk_container_add(GTK_CONTAINER(protoLayout), gStockImageWidget);
    gtk_widget_realize(gStockImageWidget);
  }
}

#if GTK_CHECK_VERSION(2,4,0)
static void
ensure_icon_factory()
{
  if (!gIconFactory) {
    gIconFactory = gtk_icon_factory_new();
    gtk_icon_factory_add_default(gIconFactory);
    g_object_unref(gIconFactory);
  }
}
#endif

#ifdef MOZ_ENABLE_GNOMEUI
static nsresult
ensure_libgnomeui()
{
  // Attempt to get the libgnomeui symbol references. We do it this way so that stock icons from Init()
  // don't get held back by InitWithGnome()'s libgnomeui dependency.
  if (!gTriedToLoadGnomeLibs) {
    gLibGnomeUI = PR_LoadLibrary("libgnomeui-2.so.0");
    if (!gLibGnomeUI)
      return NS_ERROR_NOT_AVAILABLE;

    _gnome_init = (_GnomeInit_fn)PR_FindFunctionSymbol(gLibGnomeUI, "gnome_init_with_popt_table");
    _gnome_icon_theme_new = (_GnomeIconThemeNew_fn)PR_FindFunctionSymbol(gLibGnomeUI, "gnome_icon_theme_new");
    _gnome_icon_lookup = (_GnomeIconLookup_fn)PR_FindFunctionSymbol(gLibGnomeUI, "gnome_icon_lookup");
    _gnome_icon_theme_lookup_icon = (_GnomeIconThemeLookupIcon_fn)PR_FindFunctionSymbol(gLibGnomeUI, "gnome_icon_theme_lookup_icon");

    if (!_gnome_init || !_gnome_icon_theme_new || !_gnome_icon_lookup || !_gnome_icon_theme_lookup_icon) {
      PR_UnloadLibrary(gLibGnomeUI);
      gLibGnomeUI = nsnull;
      return NS_ERROR_NOT_AVAILABLE;
    }
  }

  if (!gLibGnomeUI)
    return NS_ERROR_NOT_AVAILABLE;

  return NS_OK;
}

static nsresult
ensure_libgnome()
{
  if (!gTriedToLoadGnomeLibs) {
    gLibGnome = PR_LoadLibrary("libgnome-2.so.0");
    if (!gLibGnome)
      return NS_ERROR_NOT_AVAILABLE;

    _gnome_program_get = (_GnomeProgramGet_fn)PR_FindFunctionSymbol(gLibGnome, "gnome_program_get");
    if (!_gnome_program_get) {
      PR_UnloadLibrary(gLibGnome);
      gLibGnome = nsnull;
      return NS_ERROR_NOT_AVAILABLE;
    }
  }

  if (!gLibGnome)
    return NS_ERROR_NOT_AVAILABLE;

  return NS_OK;
}

static nsresult
ensure_libgnomevfs()
{
  if (!gTriedToLoadGnomeLibs) {
    gLibGnomeVFS = PR_LoadLibrary("libgnomevfs-2.so.0");
    if (!gLibGnomeVFS)
      return NS_ERROR_NOT_AVAILABLE;

    _gnome_vfs_get_file_info = (_GnomeVFSGetFileInfo_fn)PR_FindFunctionSymbol(gLibGnomeVFS, "gnome_vfs_get_file_info");
    _gnome_vfs_file_info_clear = (_GnomeVFSFileInfoClear_fn)PR_FindFunctionSymbol(gLibGnomeVFS, "gnome_vfs_file_info_clear");
    if (!_gnome_vfs_get_file_info || !_gnome_vfs_file_info_clear) {
      PR_UnloadLibrary(gLibGnomeVFS);
      gLibGnomeVFS = nsnull;
      return NS_ERROR_NOT_AVAILABLE;
    }
  }

  if (!gLibGnomeVFS)
    return NS_ERROR_NOT_AVAILABLE;

  return NS_OK;
}
#endif

static GtkIconSize
moz_gtk_icon_size(const char *name)
{
  if (strcmp(name, "button") == 0)
    return GTK_ICON_SIZE_BUTTON;

  if (strcmp(name, "menu") == 0)
    return GTK_ICON_SIZE_MENU;

  if (strcmp(name, "toolbar") == 0)
    return GTK_ICON_SIZE_LARGE_TOOLBAR;

  if (strcmp(name, "toolbarsmall") == 0)
    return GTK_ICON_SIZE_SMALL_TOOLBAR;

  if (strcmp(name, "dnd") == 0)
    return GTK_ICON_SIZE_DND;

  if (strcmp(name, "dialog") == 0)
    return GTK_ICON_SIZE_DIALOG;

  return GTK_ICON_SIZE_MENU;
}

#ifdef MOZ_ENABLE_GNOMEUI
nsresult
nsIconChannel::InitWithGnome(nsIMozIconURI *aIconURI)
{
  nsresult rv;

  if (NS_FAILED(ensure_libgnomeui()) || NS_FAILED(ensure_libgnome()) || NS_FAILED(ensure_libgnomevfs())) {
    gTriedToLoadGnomeLibs = PR_TRUE;
    return NS_ERROR_NOT_AVAILABLE;
  }

  gTriedToLoadGnomeLibs = PR_TRUE;

  if (!_gnome_program_get()) {
    // Get the brandShortName from the string bundle to pass to GNOME
    // as the application name.  This may be used for things such as
    // the title of grouped windows in the panel.
    nsCOMPtr<nsIStringBundleService> bundleService = 
      do_GetService(NS_STRINGBUNDLE_CONTRACTID);

    NS_ASSERTION(bundleService, "String bundle service must be present!");

    nsCOMPtr<nsIStringBundle> bundle;
    bundleService->CreateBundle("chrome://branding/locale/brand.properties",
                                getter_AddRefs(bundle));
    nsAutoString appName;

    if (bundle) {
      bundle->GetStringFromName(NS_LITERAL_STRING("brandShortName").get(),
                                getter_Copies(appName));
    } else {
      NS_WARNING("brand.properties not present, using default application name");
      appName.Assign(NS_LITERAL_STRING("Gecko"));
    }

    char* empty[] = { "" };
    _gnome_init(NS_ConvertUTF16toUTF8(appName).get(), "1.0", 1, empty, NULL, 0, NULL);
  }

  nsCAutoString iconSizeString;
  aIconURI->GetIconSize(iconSizeString);

  PRUint32 iconSize;

  if (iconSizeString.IsEmpty()) {
    rv = aIconURI->GetImageSize(&iconSize);
    NS_ASSERTION(NS_SUCCEEDED(rv), "GetImageSize failed");
  } else {
    int size;
    
    GtkIconSize icon_size = moz_gtk_icon_size(iconSizeString.get());
    gtk_icon_size_lookup(icon_size, &size, NULL);
    iconSize = size;
  }

  nsCAutoString type;
  aIconURI->GetContentType(type);

  GnomeVFSFileInfo fileInfo = {0};
  fileInfo.refcount = 1; // In case some GnomeVFS function addrefs and releases it

  nsCAutoString spec;
  nsCOMPtr<nsIURI> fileURI;
  rv = aIconURI->GetIconFile(getter_AddRefs(fileURI));
  if (fileURI) {
    fileURI->GetAsciiSpec(spec);
    // Only ask gnome-vfs for a GnomeVFSFileInfo for file: uris, to avoid a
    // network request
    PRBool isFile;
    if (NS_SUCCEEDED(fileURI->SchemeIs("file", &isFile)) && isFile) {
      _gnome_vfs_get_file_info(spec.get(), &fileInfo, GNOME_VFS_FILE_INFO_DEFAULT);
    }
    else {
      // We have to get a leaf name from our uri...
      nsCOMPtr<nsIURL> url(do_QueryInterface(fileURI));
      if (url) {
        nsCAutoString name;
        // The filename we get is UTF-8-compatible, which matches gnome expectations.
        // See also: http://lists.gnome.org/archives/gnome-vfs-list/2004-March/msg00049.html
        // "Whenever we can detect the charset used for the URI type we try to
        //  convert it to/from utf8 automatically inside gnome-vfs."
        // I'll interpret that as "otherwise, this field is random junk".
        url->GetFileName(name);
        fileInfo.name = g_strdup(name.get());
      }
      // If this is no nsIURL, nothing we can do really.

      if (!type.IsEmpty()) {
        fileInfo.valid_fields = GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE;
        fileInfo.mime_type = g_strdup(type.get());
      }
    }
  }


  if (type.IsEmpty()) {
    nsCOMPtr<nsIMIMEService> ms(do_GetService("@mozilla.org/mime;1"));
    if (ms) {
      nsCAutoString fileExt;
      aIconURI->GetFileExtension(fileExt);
      ms->GetTypeFromExtension(fileExt, type);
    }
  }

  // Get the icon theme
  if (!gIconTheme) {
    gIconTheme = _gnome_icon_theme_new();

    if (!gIconTheme) {
      _gnome_vfs_file_info_clear(&fileInfo);
      return NS_ERROR_NOT_AVAILABLE;
    }
  }

  char* name = _gnome_icon_lookup(gIconTheme, NULL, spec.get(), NULL, &fileInfo,
                                  type.get(), GNOME_ICON_LOOKUP_FLAGS_NONE,
                                  NULL);

  _gnome_vfs_file_info_clear(&fileInfo);
  if (!name)
    return NS_ERROR_NOT_AVAILABLE;

  char* file = _gnome_icon_theme_lookup_icon(gIconTheme, name, iconSize,
                                             NULL, NULL);
  g_free(name);
  if (!file)
    return NS_ERROR_NOT_AVAILABLE;

  // Create a GdkPixbuf buffer and scale it
  GError *err = nsnull;
  GdkPixbuf* buf = gdk_pixbuf_new_from_file(file, &err);
  g_free(file);
  if (!buf) {
    if (err)
      g_error_free(err);
    return NS_ERROR_UNEXPECTED;
  }

  GdkPixbuf* scaled = buf;
  if (gdk_pixbuf_get_width(buf)  != iconSize &&
      gdk_pixbuf_get_height(buf) != iconSize) {
    // scale...
    scaled = gdk_pixbuf_scale_simple(buf, iconSize, iconSize,
                                     GDK_INTERP_BILINEAR);
    gdk_pixbuf_unref(buf);
    if (!scaled)
      return NS_ERROR_OUT_OF_MEMORY;
  }

  // XXX Respect icon state
  
  rv = moz_gdk_pixbuf_to_channel(scaled, aIconURI,
                                 getter_AddRefs(mRealChannel));
  gdk_pixbuf_unref(scaled);
  return rv;
}
#endif

nsresult
nsIconChannel::Init(nsIURI* aURI)
{
  nsCOMPtr<nsIMozIconURI> iconURI = do_QueryInterface(aURI);
  NS_ASSERTION(iconURI, "URI is not an nsIMozIconURI");

  nsCAutoString stockIcon;
  iconURI->GetStockIcon(stockIcon);
  if (stockIcon.IsEmpty()) {
#ifdef MOZ_ENABLE_GNOMEUI
    return InitWithGnome(iconURI);
#else
    return NS_ERROR_NOT_AVAILABLE;
#endif
  }

  nsCAutoString iconSizeString;
  iconURI->GetIconSize(iconSizeString);

  nsCAutoString iconStateString;
  iconURI->GetIconState(iconStateString);

  GtkIconSize icon_size = moz_gtk_icon_size(iconSizeString.get());
   
  ensure_stock_image_widget();

  gboolean sensitive = strcmp(iconStateString.get(), "disabled");
  gtk_widget_set_sensitive (gStockImageWidget, sensitive);

  GdkPixbuf *icon = gtk_widget_render_icon(gStockImageWidget, stockIcon.get(),
                                           icon_size, NULL);
#if GTK_CHECK_VERSION(2,4,0)
  if (!icon) {
    ensure_icon_factory();
      
    GtkIconSet *icon_set = gtk_icon_set_new();
    GtkIconSource *icon_source = gtk_icon_source_new();
    
    gtk_icon_source_set_icon_name(icon_source, stockIcon.get());
    gtk_icon_set_add_source(icon_set, icon_source);
    gtk_icon_factory_add(gIconFactory, stockIcon.get(), icon_set);
    gtk_icon_set_unref(icon_set);
    gtk_icon_source_free(icon_source);

    icon = gtk_widget_render_icon(gStockImageWidget, stockIcon.get(),
                                  icon_size, NULL);
  }
#endif

  if (!icon)
    return NS_ERROR_NOT_AVAILABLE;
  
  nsresult rv = moz_gdk_pixbuf_to_channel(icon, iconURI,
                                          getter_AddRefs(mRealChannel));

  gdk_pixbuf_unref(icon);

  return rv;
}

void
nsIconChannel::Shutdown() {
  if (gProtoWindow) {
    gtk_widget_destroy(gProtoWindow);
    gProtoWindow = nsnull;
    gStockImageWidget = nsnull;
  }
#ifdef MOZ_ENABLE_GNOMEUI
  if (gIconTheme) {
    g_object_unref(G_OBJECT(gIconTheme));
    gIconTheme = nsnull;
  }
  gTriedToLoadGnomeLibs = PR_FALSE;
  if (gLibGnomeUI) {
    PR_UnloadLibrary(gLibGnomeUI);
    gLibGnomeUI = nsnull;
  }
  if (gLibGnome) {
    PR_UnloadLibrary(gLibGnome);
    gLibGnome = nsnull;
  }
  if (gLibGnomeVFS) {
    PR_UnloadLibrary(gLibGnomeVFS);
    gLibGnomeVFS = nsnull;
  }
#endif
}
