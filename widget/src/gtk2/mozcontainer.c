/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "mozcontainer.h"
#include <gtk/gtkprivate.h>
#include <stdio.h>

#ifdef ACCESSIBILITY
#include <atk/atk.h>
#include "maiRedundantObjectFactory.h"
#endif 

/* init methods */
static void moz_container_class_init          (MozContainerClass *klass);
static void moz_container_init                (MozContainer      *container);

/* widget class methods */
static void moz_container_map                 (GtkWidget         *widget);
static void moz_container_unmap               (GtkWidget         *widget);
static void moz_container_realize             (GtkWidget         *widget);
static void moz_container_size_allocate       (GtkWidget         *widget,
                                               GtkAllocation     *allocation);

/* container class methods */
static void moz_container_remove      (GtkContainer      *container,
                                       GtkWidget         *child_widget);
static void moz_container_forall      (GtkContainer      *container,
                                       gboolean           include_internals,
                                       GtkCallback        callback,
                                       gpointer           callback_data);
static void moz_container_add         (GtkContainer      *container,
                                        GtkWidget        *widget);

typedef struct _MozContainerChild MozContainerChild;

struct _MozContainerChild {
    GtkWidget *widget;
    gint x;
    gint y;
};

static void moz_container_allocate_child (MozContainer      *container,
                                          MozContainerChild *child);
static MozContainerChild *
moz_container_get_child (MozContainer *container, GtkWidget *child);

static GtkContainerClass *parent_class = NULL;

/* public methods */

GtkType
moz_container_get_type(void)
{
    static GtkType moz_container_type = 0;

    if (!moz_container_type) {
        static GTypeInfo moz_container_info = {
            sizeof(MozContainerClass), /* class_size */
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moz_container_class_init, /* class_init */
            NULL, /* class_destroy */
            NULL, /* class_data */
            sizeof(MozContainer), /* instance_size */
            0, /* n_preallocs */
            (GInstanceInitFunc) moz_container_init, /* instance_init */
            NULL, /* value_table */
        };

        moz_container_type = g_type_register_static (GTK_TYPE_CONTAINER,
                                                     "MozContainer",
                                                     &moz_container_info, 0);
#ifdef ACCESSIBILITY
        /* Set a factory to return accessible object with ROLE_REDUNDANT for
         * MozContainer, so that gail won't send focus notification for it */
        atk_registry_set_factory_type(atk_get_default_registry(),
                                      moz_container_type,
                                      mai_redundant_object_factory_get_type());
#endif
    }

    return moz_container_type;
}

GtkWidget *
moz_container_new (void)
{
    MozContainer *container;

    container = gtk_type_new (MOZ_CONTAINER_TYPE);

    return GTK_WIDGET(container);
}

void
moz_container_put (MozContainer *container, GtkWidget *child_widget,
                   gint x, gint y)
{
    MozContainerChild *child;

    child = g_new (MozContainerChild, 1);

    child->widget = child_widget;
    child->x = x;
    child->y = y;

    /*  printf("moz_container_put %p %p %d %d\n", (void *)container,
        (void *)child_widget, x, y); */

    container->children = g_list_append (container->children, child);

    /* we assume that the caller of this function will have already set
       the parent GdkWindow because we can have many anonymous children. */
    gtk_widget_set_parent(child_widget, GTK_WIDGET(container));
}

void
moz_container_move (MozContainer *container, GtkWidget *child_widget,
                    gint x, gint y, gint width, gint height)
{
    MozContainerChild *child;
    GtkAllocation new_allocation;

    child = moz_container_get_child (container, child_widget);

    child->x = x;
    child->y = y;

    new_allocation.x = x;
    new_allocation.y = y;
    new_allocation.width = width;
    new_allocation.height = height;

    /* printf("moz_container_move %p %p will allocate to %d %d %d %d\n",
       (void *)container, (void *)child_widget,
       new_allocation.x, new_allocation.y,
       new_allocation.width, new_allocation.height); */

    gtk_widget_size_allocate(child_widget, &new_allocation);
}

/* This function updates the allocation on a child widget without
causing a size_allocate event to be generated on that widget.  This
should only be used for scrolling since it's assumed that the expose
event created by the scroll will update any widgets that come into view. */

void
moz_container_scroll_update (MozContainer *container, GtkWidget *child_widget,
                             gint x, gint y)
{
    MozContainerChild *child;
    GtkAllocation new_allocation;

    child = moz_container_get_child (container, child_widget);

    child->x = x;
    child->y = y;

    new_allocation.x = x;
    new_allocation.y = y;
    new_allocation.width = child_widget->allocation.width;
    new_allocation.height = child_widget->allocation.height;

    /* printf("moz_container_update %p %p will allocate to %d %d %d %d\n",
       (void *)container, (void *)child_widget,
       new_allocation.x, new_allocation.y,
       new_allocation.width, new_allocation.height); */

    gtk_widget_size_allocate(child_widget, &new_allocation);
}

/* static methods */

void
moz_container_class_init (MozContainerClass *klass)
{
    /*GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
      GtkObjectClass *object_class = GTK_OBJECT_CLASS (klass); */
    GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

    parent_class = g_type_class_peek_parent (klass);

    widget_class->map = moz_container_map;
    widget_class->unmap = moz_container_unmap;
    widget_class->realize = moz_container_realize;
    widget_class->size_allocate = moz_container_size_allocate;

    container_class->remove = moz_container_remove;
    container_class->forall = moz_container_forall;
    container_class->add = moz_container_add;
}

void
moz_container_init (MozContainer *container)
{
    GTK_WIDGET_SET_FLAGS(container, GTK_CAN_FOCUS);
    container->container.resize_mode = GTK_RESIZE_IMMEDIATE;
    gtk_widget_set_redraw_on_allocate(GTK_WIDGET(container),
                                      FALSE);

    /* Mozilla uses the the gdbrgb colormap and visual throughout the
       backend so for widgets we just use that colormap instead of the
       default one. */
    gtk_widget_set_colormap(GTK_WIDGET(container), gdk_rgb_get_colormap());
}

void
moz_container_map (GtkWidget *widget)
{
    MozContainer *container;
    GList *tmp_list;
    GtkWidget *tmp_child;

    g_return_if_fail (IS_MOZ_CONTAINER(widget));
    container = MOZ_CONTAINER (widget);

    GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);

    tmp_list = container->children;
    while (tmp_list) {
        tmp_child = ((MozContainerChild *)tmp_list->data)->widget;
    
        if (GTK_WIDGET_VISIBLE(tmp_child)) {
            if (!GTK_WIDGET_MAPPED(tmp_child))
                gtk_widget_map(tmp_child);
        }
        tmp_list = tmp_list->next;
    }

    gdk_window_show (widget->window);
}

void
moz_container_unmap (GtkWidget *widget)
{
    MozContainer *container;

    g_return_if_fail (IS_MOZ_CONTAINER (widget));
    container = MOZ_CONTAINER (widget);
  
    GTK_WIDGET_UNSET_FLAGS (widget, GTK_MAPPED);

    gdk_window_hide (widget->window);
}

void
moz_container_realize (GtkWidget *widget)
{
    GdkWindowAttr attributes;
    gint attributes_mask = 0;
    MozContainer *container;

    g_return_if_fail(IS_MOZ_CONTAINER(widget));

    container = MOZ_CONTAINER(widget);

    GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

    /* create the shell window */

    attributes.event_mask = gtk_widget_get_events (widget);
    attributes.event_mask |=  (GDK_EXPOSURE_MASK | GDK_STRUCTURE_MASK);
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.window_type = GDK_WINDOW_CHILD;

    attributes_mask |= GDK_WA_VISUAL | GDK_WA_COLORMAP |
        GDK_WA_X | GDK_WA_Y;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                     &attributes, attributes_mask);
    /*  printf("widget->window is %p\n", (void *)widget->window); */
    gdk_window_set_user_data (widget->window, container);

    widget->style = gtk_style_attach (widget->style, widget->window);

    /* set the back pixmap to None so that you don't end up with the gtk
       default which is BlackPixel */
    gdk_window_set_back_pixmap (widget->window, NULL, FALSE);
}

void
moz_container_size_allocate (GtkWidget     *widget,
                             GtkAllocation *allocation)
{
    MozContainer   *container;
    GList          *tmp_list;
    GtkAllocation   tmp_allocation;
    GtkRequisition  tmp_requisition;
    GtkWidget      *tmp_child;

    g_return_if_fail (IS_MOZ_CONTAINER (widget));

    /*  printf("moz_container_size_allocate %p %d %d %d %d\n",
        (void *)widget,
        allocation->x,
        allocation->y,
        allocation->width,
        allocation->height); */

    /* short circuit if you can */
    container = MOZ_CONTAINER (widget);
    if (!container->children &&
        widget->allocation.x == allocation->x &&
        widget->allocation.y == allocation->y &&
        widget->allocation.width == allocation->width &&
        widget->allocation.height == allocation->height) {
        return;
    }

    widget->allocation = *allocation;

    tmp_list = container->children;

    while (tmp_list) {
        MozContainerChild *child = tmp_list->data;

        moz_container_allocate_child (container, child);

        tmp_list = tmp_list->next;
    }

    if (GTK_WIDGET_REALIZED (widget)) {
        gdk_window_move_resize(widget->window,
                               widget->allocation.x,
                               widget->allocation.y,
                               widget->allocation.width,
                               widget->allocation.height);
    }
}

void
moz_container_remove (GtkContainer *container, GtkWidget *child_widget)
{
    MozContainerChild *child;
    MozContainer *moz_container;
    GList *tmp_list;

    g_return_if_fail (IS_MOZ_CONTAINER(container));
    g_return_if_fail (GTK_IS_WIDGET(child_widget));

    moz_container = MOZ_CONTAINER(container);

    child = moz_container_get_child (moz_container, child_widget);
    g_return_if_fail (child);

    if(child->widget == child_widget) {
        gtk_widget_unparent(child_widget);
    }

    moz_container->children = g_list_remove(moz_container->children, child);
    g_free(child);
}

void
moz_container_forall (GtkContainer *container, gboolean include_internals,
                      GtkCallback  callback, gpointer callback_data)
{
    MozContainer *moz_container;
    GList *tmp_list;
  
    g_return_if_fail (IS_MOZ_CONTAINER(container));
    g_return_if_fail (callback != NULL);

    moz_container = MOZ_CONTAINER(container);

    tmp_list = moz_container->children;
    while (tmp_list) {
        MozContainerChild *child;
        child = tmp_list->data;
        tmp_list = tmp_list->next;
        (* callback) (child->widget, callback_data);
    }
}

static void
moz_container_allocate_child (MozContainer *container,
                              MozContainerChild *child)
{
    GtkAllocation  allocation;
    GtkRequisition requisition;

    allocation.x = child->x;
    allocation.y = child->y;
    /* gtk_widget_get_child_requisition (child->widget, &requisition); */
    /* gtk_widget_size_request (child->widget, &requisition); */
    allocation.width = child->widget->allocation.width;
    allocation.height = child->widget->allocation.height;

    gtk_widget_size_allocate (child->widget, &allocation);
}

MozContainerChild *
moz_container_get_child (MozContainer *container, GtkWidget *child_widget)
{
    GList *tmp_list;

    tmp_list = container->children;
    while (tmp_list) {
        MozContainerChild *child;
    
        child = tmp_list->data;
        tmp_list = tmp_list->next;

        if (child->widget == child_widget)
            return child;
    }

    return NULL;
}

static void 
moz_container_add(GtkContainer *container, GtkWidget *widget)
{
    moz_container_put(MOZ_CONTAINER(container), widget, 0, 0);
}

