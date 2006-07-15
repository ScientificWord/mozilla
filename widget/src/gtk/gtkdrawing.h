/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>  (Original Author)
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

/**
 * gtkdrawing.h: GTK widget rendering utilities
 *
 * gtkdrawing provides an API for rendering GTK widgets in the
 * current theme to a pixmap or window, without requiring an actual
 * widget instantiation, similar to the Macintosh Appearance Manager
 * or Windows XP's DrawThemeBackground() API.
 */

#ifndef _GTK_DRAWING_H_
#define _GTK_DRAWING_H_

#include <gdk/gdk.h>
#include <gtk/gtkstyle.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*** type definitions ***/
typedef struct {
  guint8 active;
  guint8 focused;
  guint8 inHover;
  guint8 disabled;
  guint8 isDefault;
  guint8 canDefault;
  gint32 curpos; /* curpos and maxpos are used for scrollbars */
  gint32 maxpos;
} GtkWidgetState;

typedef struct {
  gint slider_width;
  gint trough_border;
  gint stepper_size;
  gint stepper_spacing;
  gint min_slider_size;
} MozGtkScrollbarMetrics;

/** flags for tab state **/
typedef enum {
  /* the first tab in the group */
  MOZ_GTK_TAB_FIRST           = 1 << 0,
  /* the tab just before the selected tab */
  MOZ_GTK_TAB_BEFORE_SELECTED = 1 << 1,
  /* the selected tab */
  MOZ_GTK_TAB_SELECTED        = 1 << 2
} GtkTabFlags;

/* function type for moz_gtk_enable_style_props */
typedef gint (*style_prop_t)(GtkStyle*, const gchar*, gint);

/*** result/error codes ***/
#define MOZ_GTK_SUCCESS 0
#define MOZ_GTK_UNKNOWN_WIDGET -1
#define MOZ_GTK_UNSAFE_THEME -2

/*** widget type constants ***/
typedef enum {
  /* Paints a GtkButton. flags is a GtkReliefStyle. */
  MOZ_GTK_BUTTON,
  /* Paints a GtkCheckButton. flags is a boolean, 1=checked, 0=not checked. */
  MOZ_GTK_CHECKBUTTON,
  /* Paints a GtkRadioButton. flags is a boolean, 1=checked, 0=not checked. */
  MOZ_GTK_RADIOBUTTON,
  /**
   * Paints the button of a GtkScrollbar. flags is a GtkArrowType giving
   * the arrow direction.
   */
  MOZ_GTK_SCROLLBAR_BUTTON,
  /* Paints the trough (track) of a GtkScrollbar. */
  MOZ_GTK_SCROLLBAR_TRACK_HORIZONTAL,
  MOZ_GTK_SCROLLBAR_TRACK_VERTICAL,
  /* Paints the slider (thumb) of a GtkScrollbar. */
  MOZ_GTK_SCROLLBAR_THUMB_HORIZONTAL,
  MOZ_GTK_SCROLLBAR_THUMB_VERTICAL,
  /* Paints a GtkScale. */
  MOZ_GTK_SCALE_HORIZONTAL,
  MOZ_GTK_SCALE_VERTICAL,
  /* Paints a GtkScale thumb. */
  MOZ_GTK_SCALE_THUMB_HORIZONTAL,
  MOZ_GTK_SCALE_THUMB_VERTICAL,
  /* Paints the gripper of a GtkHandleBox. */
  MOZ_GTK_GRIPPER,
  /* Paints a GtkEntry. */
  MOZ_GTK_ENTRY,
  /* Paints a GtkOptionMenu. */
  MOZ_GTK_DROPDOWN,
  /* Paints a dropdown arrow (a GtkButton containing a down GtkArrow). */
  MOZ_GTK_DROPDOWN_ARROW,
  /* Paints the container part of a GtkCheckButton. */
  MOZ_GTK_CHECKBUTTON_CONTAINER,
  /* Paints the container part of a GtkRadioButton. */
  MOZ_GTK_RADIOBUTTON_CONTAINER,
  /* Paints the label of a GtkCheckButton (focus outline) */
  MOZ_GTK_CHECKBUTTON_LABEL,
  /* Paints the label of a GtkRadioButton (focus outline) */
  MOZ_GTK_RADIOBUTTON_LABEL,
  /* Paints the background of a GtkHandleBox. */
  MOZ_GTK_TOOLBAR,
  /* Paints a GtkToolTip */
  MOZ_GTK_TOOLTIP,
  /* Paints a GtkFrame (e.g. a status bar panel). */
  MOZ_GTK_FRAME,
  /* Paints a GtkProgressBar. */
  MOZ_GTK_PROGRESSBAR,
  /* Paints a progress chunk of a GtkProgressBar. */
  MOZ_GTK_PROGRESS_CHUNK,
  /* Paints a tab of a GtkNotebook. flags is a GtkTabFlags, defined above. */
  MOZ_GTK_TAB,
  /* Paints the background and border of a GtkNotebook. */
  MOZ_GTK_TABPANELS,
  /* Paints the background of the menu bar. */
  MOZ_GTK_MENUBAR,
  /* Paints the background of menus, context menus. */
  MOZ_GTK_MENUPOPUP,
  /* Paints items of menubar and popups. */
  MOZ_GTK_MENUITEM,
  MOZ_GTK_CHECKMENUITEM,
  MOZ_GTK_RADIOMENUITEM,
  /* Paints the background of a window, dialog or page. */
  MOZ_GTK_WINDOW
} GtkThemeWidgetType;

/*** General library functions ***/
/**
 * Initializes the drawing library.  You must call this function
 * prior to using any other functionality.
 * returns: MOZ_GTK_SUCCESS if there were no errors
 *          MOZ_GTK_UNSAFE_THEME if the current theme engine is known
 *                               to crash with gtkdrawing.
 */
gint moz_gtk_init();

/**
 * Enable GTK+ 1.2.9+ theme enhancements. You must provide a pointer
 * to the GTK+ 1.2.9+ function "gtk_style_get_prop_experimental".
 * styleGetProp:  pointer to gtk_style_get_prop_experimental
 * 
 * returns: MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_enable_style_props(style_prop_t styleGetProp);

/**
 * Perform cleanup of the drawing library. You should call this function
 * when your program exits, or you no longer need the library.
 *
 * returns: MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_shutdown();


/*** Widget drawing ***/
/**
 * Paint a widget in the current theme.
 * widget:   a constant giving the widget to paint
 * rect:     the bounding rectangle for the widget
 * cliprect: a clipprect rectangle for this painting operation
 * state:    the state of the widget.  ignored for some widgets.
 * flags:    widget-dependant flags; see the GtkThemeWidgetType definition.
 */
gint
moz_gtk_widget_paint(GtkThemeWidgetType widget, GdkDrawable* drawable,
                     GdkRectangle* rect, GdkRectangle* cliprect,
                     GtkWidgetState* state, gint flags);


/*** Widget metrics ***/
/**
 * Get the border size of a widget
 * xthickness:  [OUT] the widget's left/right border
 * ythickness:  [OUT] the widget's top/bottom border
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_get_widget_border(GtkThemeWidgetType widget, gint* xthickness,
                               gint* ythickness);

/**
 * Get the desired size of a GtkCheckButton
 * indicator_size:     [OUT] the indicator size
 * indicator_spacing:  [OUT] the spacing between the indicator and its
 *                     container
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint
moz_gtk_checkbox_get_metrics(gint* indicator_size, gint* indicator_spacing);

/**
 * Get the desired size of a GtkRadioButton
 * indicator_size:     [OUT] the indicator size
 * indicator_spacing:  [OUT] the spacing between the indicator and its
 *                     container
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint
moz_gtk_radio_get_metrics(gint* indicator_size, gint* indicator_spacing);

/** Get the focus metrics for a button, checkbox, or radio button.
 * interior_focus:     [OUT] whether the focus is drawn around the
 *                           label (TRUE) or around the whole container (FALSE)
 * focus_width:        [OUT] the width of the focus line
 * focus_pad:          [OUT] the padding between the focus line and children
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint
moz_gtk_button_get_focus(gboolean* interior_focus,
                         gint* focus_width, gint* focus_pad);
gint
moz_gtk_checkbox_get_focus(gboolean* interior_focus,
                           gint* focus_width, gint* focus_pad);
gint
moz_gtk_radio_get_focus(gboolean* interior_focus,
                        gint* focus_width, gint* focus_pad);

/**
 * Get the desired size of a GtkScale thumb
 * orient:           [IN] the scale orientation
 * thumb_length:     [OUT] the length of the thumb
 * thumb_height:     [OUT] the height of the thumb
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint
moz_gtk_get_scalethumb_metrics(GtkOrientation orient, gint* thumb_length, gint* thumb_height);

/**
 * Get the desired metrics for a GtkScrollbar
 * metrics:          [IN]  struct which will contain the metrics
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint
moz_gtk_get_scrollbar_metrics(MozGtkScrollbarMetrics* metrics);

/**
 * Get the desired size of a dropdown arrow button
 * width:   [OUT] the desired width
 * height:  [OUT] the desired height
 *
 * returns:    MOZ_GTK_SUCCESS if there was no error, an error code otherwise
 */
gint moz_gtk_get_dropdown_arrow_size(gint* width, gint* height);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
