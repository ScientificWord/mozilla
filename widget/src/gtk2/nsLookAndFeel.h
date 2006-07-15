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
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
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

#ifndef __nsLookAndFeel
#define __nsLookAndFeel
#include "nsXPLookAndFeel.h"
#include "nsCOMPtr.h"
#include <gtk/gtk.h>

class nsLookAndFeel: public nsXPLookAndFeel {
public:
    nsLookAndFeel();
    virtual ~nsLookAndFeel();

    nsresult NativeGetColor(const nsColorID aID, nscolor &aColor);
    NS_IMETHOD GetMetric(const nsMetricID aID, PRInt32 & aMetric);
    NS_IMETHOD GetMetric(const nsMetricFloatID aID, float & aMetric);
    NS_IMETHOD LookAndFeelChanged();

protected:
    GtkStyle *mStyle;
    GtkWidget *mWidget;

    // Cached colors, we have to create a dummy widget to actually
    // get the style
    static PRBool sColorsInitialized;
    static nscolor sInfoBackground;
    static nscolor sInfoText;
    static nscolor sMenuBackground;
    static nscolor sMenuText;
    static nscolor sMenuHover;
    static nscolor sMenuHoverText;
    static nscolor sButtonBackground;
    static nscolor sButtonText;
    static nscolor sButtonOuterLightBorder;
    static nscolor sButtonInnerDarkBorder;

    static void InitColors();
    void InitWidget() {
        mWidget = gtk_invisible_new();
        gtk_object_ref(GTK_OBJECT(mWidget));
        gtk_object_sink(GTK_OBJECT(mWidget));
        gtk_widget_ensure_style(mWidget);
        mStyle = gtk_widget_get_style(mWidget);
    }
};

#endif
