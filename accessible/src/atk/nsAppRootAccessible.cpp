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
 * The Initial Developer of the Original Code is
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Bolian Yin (bolian.yin@sun.com)
 *   Ginn Chen (ginn.chen@sun.com)
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

#include "nsCOMPtr.h"
#include "nsMai.h"
#include "nsAppRootAccessible.h"
#include "prlink.h"
#include "prenv.h"
#include "nsIPrefBranch.h"
#include "nsIServiceManager.h"
#include "nsAutoPtr.h"

#include <gtk/gtk.h>
#include <atk/atk.h>

typedef GType (* AtkGetTypeType) (void);
GType g_atk_hyperlink_impl_type = G_TYPE_INVALID;
static PRBool sATKChecked = PR_FALSE;
static PRLibrary *sATKLib = nsnull;
static const char sATKLibName[] = "libatk-1.0.so.0";
static const char sATKHyperlinkImplGetTypeSymbol[] =
    "atk_hyperlink_impl_get_type";
static const char sAccEnv [] = "GNOME_ACCESSIBILITY";
static const char sSysPrefService [] =
    "@mozilla.org/system-preference-service;1";
static const char sAccessibilityKey [] =
    "config.use_system_prefs.accessibility";

/* gail function pointer */
static guint (* gail_add_global_event_listener) (GSignalEmissionHook listener,
                                                 const gchar *event_type);
static void (* gail_remove_global_event_listener) (guint remove_listener);
static void (* gail_remove_key_event_listener) (guint remove_listener);
static AtkObject * (*gail_get_root) (void);

/* maiutil */

static guint mai_util_add_global_event_listener(GSignalEmissionHook listener,
                                                const gchar *event_type);
static void mai_util_remove_global_event_listener(guint remove_listener);
static guint mai_util_add_key_event_listener(AtkKeySnoopFunc listener,
                                             gpointer data);
static void mai_util_remove_key_event_listener(guint remove_listener);
static AtkObject *mai_util_get_root(void);
static G_CONST_RETURN gchar *mai_util_get_toolkit_name(void);
static G_CONST_RETURN gchar *mai_util_get_toolkit_version(void);


/* Misc */

static void _listener_info_destroy(gpointer data);
static guint add_listener (GSignalEmissionHook listener,
                           const gchar *object_type,
                           const gchar *signal,
                           const gchar *hook_data,
                           guint gail_listenerid = 0);
static AtkKeyEventStruct *atk_key_event_from_gdk_event_key(GdkEventKey *key);
static gboolean notify_hf(gpointer key, gpointer value, gpointer data);
static void insert_hf(gpointer key, gpointer value, gpointer data);
static gint mai_key_snooper(GtkWidget *the_widget, GdkEventKey *event,
                            gpointer func_data);

static GHashTable *listener_list = NULL;
static gint listener_idx = 1;

#define MAI_TYPE_UTIL              (mai_util_get_type ())
#define MAI_UTIL(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                                    MAI_TYPE_UTIL, MaiUtil))
#define MAI_UTIL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), \
                                    MAI_TYPE_UTIL, MaiUtilClass))
#define MAI_IS_UTIL(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                                    MAI_TYPE_UTIL))
#define MAI_IS_UTIL_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                                    MAI_TYPE_UTIL))
#define MAI_UTIL_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                                    MAI_TYPE_UTIL, MaiUtilClass))

static GHashTable *key_listener_list = NULL;
static guint key_snooper_id = 0;

G_BEGIN_DECLS
typedef void (*GnomeAccessibilityInit) (void);
typedef void (*GnomeAccessibilityShutdown) (void);
G_END_DECLS

struct MaiUtil
{
    AtkUtil parent;
    GList *listener_list;
};

struct MaiKeyEventInfo
{
    AtkKeyEventStruct *key_event;
    gpointer func_data;
};

union AtkKeySnoopFuncPointer
{
    AtkKeySnoopFunc func_ptr;
    gpointer data;
};

struct GnomeAccessibilityModule
{
    const char *libName;
    PRLibrary *lib;
    const char *initName;
    GnomeAccessibilityInit init;
    const char *shutdownName;
    GnomeAccessibilityShutdown shutdown;
};

struct MaiUtilClass
{
    AtkUtilClass parent_class;
};

GType mai_util_get_type (void);
static void mai_util_class_init(MaiUtilClass *klass);

/* supporting */
PRLogModuleInfo *gMaiLog = NULL;

#define MAI_VERSION MOZILLA_VERSION
#define MAI_NAME "Gecko"

struct MaiUtilListenerInfo
{
    gint key;
    guint signal_id;
    gulong hook_id;
    // For window create/destory/minimize/maximize/restore/activate/deactivate
    // events, we'll chain gail_util's add/remove_global_event_listener.
    // So we store the listenerid returned by gail's add_global_event_listener
    // in this structure to call gail's remove_global_event_listener later.
    guint gail_listenerid;
};

static GnomeAccessibilityModule sAtkBridge = {
#ifdef AIX
    "libatk-bridge.a(libatk-bridge.so.0)", NULL,
#else
    "libatk-bridge.so", NULL,
#endif
    "gnome_accessibility_module_init", NULL,
    "gnome_accessibility_module_shutdown", NULL
};

static GnomeAccessibilityModule sGail = {
    "libgail.so", NULL,
    "gnome_accessibility_module_init", NULL,
    "gnome_accessibility_module_shutdown", NULL
};

GType
mai_util_get_type(void)
{
    static GType type = 0;

    if (!type) {
        static const GTypeInfo tinfo = {
            sizeof(MaiUtilClass),
            (GBaseInitFunc) NULL, /* base init */
            (GBaseFinalizeFunc) NULL, /* base finalize */
            (GClassInitFunc) mai_util_class_init, /* class init */
            (GClassFinalizeFunc) NULL, /* class finalize */
            NULL, /* class data */
            sizeof(MaiUtil), /* instance size */
            0, /* nb preallocs */
            (GInstanceInitFunc) NULL, /* instance init */
            NULL /* value table */
        };

        type = g_type_register_static(ATK_TYPE_UTIL,
                                      "MaiUtil", &tinfo, GTypeFlags(0));
    }
    return type;
}

/* intialize the the atk interface (function pointers) with MAI implementation.
 * When atk bridge get loaded, these interface can be used.
 */
static void
mai_util_class_init(MaiUtilClass *klass)
{
    AtkUtilClass *atk_class;
    gpointer data;

    data = g_type_class_peek(ATK_TYPE_UTIL);
    atk_class = ATK_UTIL_CLASS(data);

    // save gail function pointer
    gail_add_global_event_listener = atk_class->add_global_event_listener;
    gail_remove_global_event_listener = atk_class->remove_global_event_listener;
    gail_remove_key_event_listener = atk_class->remove_key_event_listener;
    gail_get_root = atk_class->get_root;

    atk_class->add_global_event_listener =
        mai_util_add_global_event_listener;
    atk_class->remove_global_event_listener =
        mai_util_remove_global_event_listener;
    atk_class->add_key_event_listener = mai_util_add_key_event_listener;
    atk_class->remove_key_event_listener = mai_util_remove_key_event_listener;
    atk_class->get_root = mai_util_get_root;
    atk_class->get_toolkit_name = mai_util_get_toolkit_name;
    atk_class->get_toolkit_version = mai_util_get_toolkit_version;

    listener_list = g_hash_table_new_full(g_int_hash, g_int_equal, NULL,
                                          _listener_info_destroy);
}

static guint
mai_util_add_global_event_listener(GSignalEmissionHook listener,
                                   const gchar *event_type)
{
    guint rc = 0;
    gchar **split_string;

    split_string = g_strsplit (event_type, ":", 3);

    if (split_string) {
        if (!strcmp ("window", split_string[0])) {
            guint gail_listenerid = 0;
            if (gail_add_global_event_listener) {
                // call gail's function to track gtk native window events
                gail_listenerid =
                    gail_add_global_event_listener(listener, event_type);
            }
            
            rc = add_listener (listener, "MaiAtkObject", split_string[1],
                               event_type, gail_listenerid);
        }
        else {
            rc = add_listener (listener, split_string[1], split_string[2],
                               event_type);
        }
        g_strfreev(split_string);
    }
    return rc;
}

static void
mai_util_remove_global_event_listener(guint remove_listener)
{
    if (remove_listener > 0) {
        MaiUtilListenerInfo *listener_info;
        gint tmp_idx = remove_listener;

        listener_info = (MaiUtilListenerInfo *)
            g_hash_table_lookup(listener_list, &tmp_idx);

        if (listener_info != NULL) {
            if (gail_remove_global_event_listener &&
                listener_info->gail_listenerid) {
              gail_remove_global_event_listener(listener_info->gail_listenerid);
            }
            
            /* Hook id of 0 and signal id of 0 are invalid */
            if (listener_info->hook_id != 0 && listener_info->signal_id != 0) {
                /* Remove the emission hook */
                g_signal_remove_emission_hook(listener_info->signal_id,
                                              listener_info->hook_id);

                /* Remove the element from the hash */
                g_hash_table_remove(listener_list, &tmp_idx);
            }
            else {
                g_warning("Invalid listener hook_id %ld or signal_id %d\n",
                          listener_info->hook_id, listener_info->signal_id);
            }
        }
        else {
            // atk-bridge is initialized with gail (e.g. yelp)
            // try gail_remove_global_event_listener
            if (gail_remove_global_event_listener) {
                return gail_remove_global_event_listener(remove_listener);
            }

            g_warning("No listener with the specified listener id %d",
                      remove_listener);
        }
    }
    else {
        g_warning("Invalid listener_id %d", remove_listener);
    }
}

static AtkKeyEventStruct *
atk_key_event_from_gdk_event_key (GdkEventKey *key)
{
    AtkKeyEventStruct *event = g_new0(AtkKeyEventStruct, 1);
    switch (key->type) {
    case GDK_KEY_PRESS:
        event->type = ATK_KEY_EVENT_PRESS;
        break;
    case GDK_KEY_RELEASE:
        event->type = ATK_KEY_EVENT_RELEASE;
        break;
    default:
        g_assert_not_reached ();
        return NULL;
    }
    event->state = key->state;
    event->keyval = key->keyval;
    event->length = key->length;
    if (key->string && key->string [0] && 
        (key->state & GDK_CONTROL_MASK ||
         g_unichar_isgraph (g_utf8_get_char (key->string)))) {
        event->string = key->string;
    }
    else if (key->type == GDK_KEY_PRESS ||
             key->type == GDK_KEY_RELEASE) {
        event->string = gdk_keyval_name (key->keyval);	    
    }
    event->keycode = key->hardware_keycode;
    event->timestamp = key->time;

    MAI_LOG_DEBUG(("MaiKey:\tsym %u\n\tmods %x\n\tcode %u\n\ttime %lx\n",
                   (unsigned int) event->keyval,
                   (unsigned int) event->state,
                   (unsigned int) event->keycode,
                   (unsigned long int) event->timestamp));
    return event;
}

static gboolean
notify_hf(gpointer key, gpointer value, gpointer data)
{
    MaiKeyEventInfo *info = (MaiKeyEventInfo *)data;
    AtkKeySnoopFuncPointer atkKeySnoop;
    atkKeySnoop.data = value;
    return (atkKeySnoop.func_ptr)(info->key_event, info->func_data) ? TRUE : FALSE;
}

static void
insert_hf(gpointer key, gpointer value, gpointer data)
{
    GHashTable *new_table = (GHashTable *) data;
    g_hash_table_insert (new_table, key, value);
}

static gint
mai_key_snooper(GtkWidget *the_widget, GdkEventKey *event, gpointer func_data)
{
    /* notify each AtkKeySnoopFunc in turn... */

    MaiKeyEventInfo *info = g_new0(MaiKeyEventInfo, 1);
    gint consumed = 0;
    if (key_listener_list) {
        GHashTable *new_hash = g_hash_table_new(NULL, NULL);
        g_hash_table_foreach (key_listener_list, insert_hf, new_hash);
        info->key_event = atk_key_event_from_gdk_event_key (event);
        info->func_data = func_data;
        consumed = g_hash_table_foreach_steal (new_hash, notify_hf, info);
        g_hash_table_destroy (new_hash);
        g_free(info->key_event);
    }
    g_free(info);
    return (consumed ? 1 : 0);
}

static guint
mai_util_add_key_event_listener (AtkKeySnoopFunc listener,
                                 gpointer data)
{
    NS_ENSURE_TRUE(listener, 0);

    static guint key=0;

    if (!key_listener_list) {
        key_listener_list = g_hash_table_new(NULL, NULL);
        key_snooper_id = gtk_key_snooper_install(mai_key_snooper, data);
    }
    AtkKeySnoopFuncPointer atkKeySnoop;
    atkKeySnoop.func_ptr = listener;
    g_hash_table_insert(key_listener_list, GUINT_TO_POINTER (key++),
                        atkKeySnoop.data);
    return key;
}

static void
mai_util_remove_key_event_listener (guint remove_listener)
{
    if (!key_listener_list) {
        // atk-bridge is initialized with gail (e.g. yelp)
        // try gail_remove_key_event_listener
        return gail_remove_key_event_listener(remove_listener);
    }

    g_hash_table_remove(key_listener_list, GUINT_TO_POINTER (remove_listener));
    if (g_hash_table_size(key_listener_list) == 0) {
        gtk_key_snooper_remove(key_snooper_id);
    }
}

AtkObject *
mai_util_get_root(void)
{
    nsRefPtr<nsApplicationAccessibleWrap> root =
        nsAccessNode::GetApplicationAccessible();

    if (root)
        return root->GetAtkObject();

    // We've shutdown, try to use gail instead
    // (to avoid assert in spi_atk_tidy_windows())
    if (gail_get_root)
        return gail_get_root();

    return nsnull;
}

G_CONST_RETURN gchar *
mai_util_get_toolkit_name(void)
{
    return MAI_NAME;
}

G_CONST_RETURN gchar *
mai_util_get_toolkit_version(void)
{
    return MAI_VERSION;
}

void
_listener_info_destroy(gpointer data)
{
    g_free(data);
}

guint
add_listener (GSignalEmissionHook listener,
              const gchar *object_type,
              const gchar *signal,
              const gchar *hook_data,
              guint gail_listenerid)
{
    GType type;
    guint signal_id;
    gint rc = 0;

    type = g_type_from_name(object_type);
    if (type) {
        signal_id = g_signal_lookup(signal, type);
        if (signal_id > 0) {
            MaiUtilListenerInfo *listener_info;

            rc = listener_idx;

            listener_info =  (MaiUtilListenerInfo *)
                g_malloc(sizeof(MaiUtilListenerInfo));
            listener_info->key = listener_idx;
            listener_info->hook_id =
                g_signal_add_emission_hook(signal_id, 0, listener,
                                           g_strdup(hook_data),
                                           (GDestroyNotify)g_free);
            listener_info->signal_id = signal_id;
            listener_info->gail_listenerid = gail_listenerid;

            g_hash_table_insert(listener_list, &(listener_info->key),
                                listener_info);
            listener_idx++;
        }
        else {
            g_warning("Invalid signal type %s\n", signal);
        }
    }
    else {
        g_warning("Invalid object type %s\n", object_type);
    }
    return rc;
}

static nsresult LoadGtkModule(GnomeAccessibilityModule& aModule);

// nsApplicationAccessibleWrap

nsApplicationAccessibleWrap::nsApplicationAccessibleWrap():
    nsApplicationAccessible()
{
    MAI_LOG_DEBUG(("======Create AppRootAcc=%p\n", (void*)this));
}

nsApplicationAccessibleWrap::~nsApplicationAccessibleWrap()
{
    MAI_LOG_DEBUG(("======Destory AppRootAcc=%p\n", (void*)this));
    nsAccessibleWrap::ShutdownAtkObject();
}

NS_IMETHODIMP
nsApplicationAccessibleWrap::Init()
{
    // XXX following code is copied from widget/src/gtk2/nsWindow.cpp
    // we should put it to somewhere that can be used from both modules
    // see bug 390761

    // check if accessibility enabled/disabled by environment variable
    PRBool isGnomeATEnabled = PR_FALSE;
    const char *envValue = PR_GetEnv(sAccEnv);
    if (envValue) {
        isGnomeATEnabled = !!atoi(envValue);
    } else {
        //check gconf-2 setting
        nsresult rv;
        nsCOMPtr<nsIPrefBranch> sysPrefService =
            do_GetService(sSysPrefService, &rv);
        if (NS_SUCCEEDED(rv) && sysPrefService) {
            sysPrefService->GetBoolPref(sAccessibilityKey, &isGnomeATEnabled);
        }
    }

    if (isGnomeATEnabled) {
        // load and initialize gail library
        nsresult rv = LoadGtkModule(sGail);
        if (NS_SUCCEEDED(rv)) {
            (*sGail.init)();
        }
        else {
            MAI_LOG_DEBUG(("Fail to load lib: %s\n", sGail.libName));
        }

        MAI_LOG_DEBUG(("Mozilla Atk Implementation initializing\n"));
        // Initialize the MAI Utility class
        // it will overwrite gail_util
        g_type_class_unref(g_type_class_ref(MAI_TYPE_UTIL));

        // load and initialize atk-bridge library
        rv = LoadGtkModule(sAtkBridge);
        if (NS_SUCCEEDED(rv)) {
            // init atk-bridge
            (*sAtkBridge.init)();
        }
        else
            MAI_LOG_DEBUG(("Fail to load lib: %s\n", sAtkBridge.libName));
    }

    return nsApplicationAccessible::Init();
}

void
nsApplicationAccessibleWrap::Unload()
{
    if (sAtkBridge.lib) {
        // Do not shutdown/unload atk-bridge,
        // an exit function registered will take care of it
        // if (sAtkBridge.shutdown)
        //     (*sAtkBridge.shutdown)();
        // PR_UnloadLibrary(sAtkBridge.lib);
        sAtkBridge.lib = NULL;
        sAtkBridge.init = NULL;
        sAtkBridge.shutdown = NULL;
    }
    if (sGail.lib) {
        // Do not shutdown gail because
        // 1) Maybe it's not init-ed by us. e.g. GtkEmbed
        // 2) We need it to avoid assert in spi_atk_tidy_windows
        // if (sGail.shutdown)
        //   (*sGail.shutdown)();
        // PR_UnloadLibrary(sGail.lib);
        sGail.lib = NULL;
        sGail.init = NULL;
        sGail.shutdown = NULL;
    }
    // if (sATKLib) {
    //     PR_UnloadLibrary(sATKLib);
    //     sATKLib = nsnull;
    // }
}

NS_IMETHODIMP
nsApplicationAccessibleWrap::GetNativeInterface(void **aOutAccessible)
{
    *aOutAccessible = nsnull;

    if (!mAtkObject) {
        mAtkObject =
            reinterpret_cast<AtkObject *>
                            (g_object_new(MAI_TYPE_ATK_OBJECT, NULL));
        NS_ENSURE_TRUE(mAtkObject, NS_ERROR_OUT_OF_MEMORY);

        atk_object_initialize(mAtkObject, this);
        mAtkObject->role = ATK_ROLE_INVALID;
        mAtkObject->layer = ATK_LAYER_INVALID;
    }

    *aOutAccessible = mAtkObject;
    return NS_OK;
}

nsresult
nsApplicationAccessibleWrap::AddRootAccessible(nsIAccessible *aRootAccWrap)
{
    NS_ENSURE_ARG_POINTER(aRootAccWrap);

    // add by weak reference
    nsresult rv = nsApplicationAccessible::AddRootAccessible(aRootAccWrap);
    NS_ENSURE_SUCCESS(rv, rv);

    AtkObject *atkAccessible = nsAccessibleWrap::GetAtkObject(aRootAccWrap);
    atk_object_set_parent(atkAccessible, mAtkObject);

    PRUint32 count = 0;
    mChildren->GetLength(&count);
    g_signal_emit_by_name(mAtkObject, "children_changed::add", count - 1,
                          atkAccessible, NULL);

#ifdef MAI_LOGGING
    if (NS_SUCCEEDED(rv)) {
        MAI_LOG_DEBUG(("\nAdd RootAcc=%p OK, count=%d\n",
                       (void*)aRootAccWrap, count));
    }
    else
        MAI_LOG_DEBUG(("\nAdd RootAcc=%p Failed, count=%d\n",
                       (void*)aRootAccWrap, count));
#endif

    return rv;
}

nsresult
nsApplicationAccessibleWrap::RemoveRootAccessible(nsIAccessible *aRootAccWrap)
{
    NS_ENSURE_ARG_POINTER(aRootAccWrap);

    PRUint32 index = 0;
    nsresult rv = NS_ERROR_FAILURE;

    // we must use weak ref to get the index
    nsCOMPtr<nsIWeakReference> weakPtr = do_GetWeakReference(aRootAccWrap);
    rv = mChildren->IndexOf(0, weakPtr, &index);

    AtkObject *atkAccessible = nsAccessibleWrap::GetAtkObject(aRootAccWrap);
    atk_object_set_parent(atkAccessible, NULL);
    g_signal_emit_by_name(mAtkObject, "children_changed::remove", index,
                          atkAccessible, NULL);

#ifdef MAI_LOGGING
    PRUint32 count = 0;
    mChildren->GetLength(&count);

    if (NS_SUCCEEDED(rv)) {
        rv = mChildren->RemoveElementAt(index);
        MAI_LOG_DEBUG(("\nRemove RootAcc=%p, count=%d\n",
                       (void*)aRootAccWrap, (count-1)));
    }
    else
        MAI_LOG_DEBUG(("\nFail to Remove RootAcc=%p, count=%d\n",
                       (void*)aRootAccWrap, count));
#else
    NS_ENSURE_SUCCESS(rv, rv);
    rv = mChildren->RemoveElementAt(index);

#endif
    InvalidateChildren();
    return rv;
}

void
nsApplicationAccessibleWrap::PreCreate()
{
    if (!sATKChecked) {
        sATKLib = PR_LoadLibrary(sATKLibName);
        if (sATKLib) {
            AtkGetTypeType pfn_atk_hyperlink_impl_get_type = (AtkGetTypeType) PR_FindFunctionSymbol(sATKLib, sATKHyperlinkImplGetTypeSymbol);
            if (pfn_atk_hyperlink_impl_get_type) {
                g_atk_hyperlink_impl_type = pfn_atk_hyperlink_impl_get_type();
            }
        }
        sATKChecked = PR_TRUE;
    }
}

static nsresult
LoadGtkModule(GnomeAccessibilityModule& aModule)
{
    NS_ENSURE_ARG(aModule.libName);

    if (!(aModule.lib = PR_LoadLibrary(aModule.libName))) {

        MAI_LOG_DEBUG(("Fail to load lib: %s in default path\n", aModule.libName));

        //try to load the module with "gtk-2.0/modules" appended
        char *curLibPath = PR_GetLibraryPath();
        nsCAutoString libPath(curLibPath);
        libPath.Append(":/usr/lib");
        MAI_LOG_DEBUG(("Current Lib path=%s\n", libPath.get()));
        PR_FreeLibraryName(curLibPath);

        PRInt16 loc1 = 0, loc2 = 0;
        PRInt16 subLen = 0;
        while (loc2 >= 0) {
            loc2 = libPath.FindChar(':', loc1);
            if (loc2 < 0)
                subLen = libPath.Length() - loc1;
            else
                subLen = loc2 - loc1;
            nsCAutoString sub(Substring(libPath, loc1, subLen));
            sub.Append("/gtk-2.0/modules/");
            sub.Append(aModule.libName);
            aModule.lib = PR_LoadLibrary(sub.get());
            if (aModule.lib) {
                MAI_LOG_DEBUG(("Ok, load %s from %s\n", aModule.libName, sub.get()));
                break;
            }
            loc1 = loc2+1;
        }
        if (!aModule.lib) {
            MAI_LOG_DEBUG(("Fail to load %s\n", aModule.libName));
            return NS_ERROR_FAILURE;
        }
    }

    //we have loaded the library, try to get the function ptrs
    if (!(aModule.init = PR_FindFunctionSymbol(aModule.lib,
                                               aModule.initName)) ||
        !(aModule.shutdown = PR_FindFunctionSymbol(aModule.lib,
                                                   aModule.shutdownName))) {

        //fail, :(
        MAI_LOG_DEBUG(("Fail to find symbol %s in %s",
                       aModule.init ? aModule.shutdownName : aModule.initName,
                       aModule.libName));
        PR_UnloadLibrary(aModule.lib);
        aModule.lib = NULL;
        return NS_ERROR_FAILURE;
    }
    return NS_OK;
}
