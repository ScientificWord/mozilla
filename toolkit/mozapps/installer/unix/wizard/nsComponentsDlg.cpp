/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Samir Gehani <sgehani@netscape.com>
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

#include "nsComponentsDlg.h"
#include "nsXInstaller.h"
#include "check_on.xpm"
#include "check_off.xpm"
#include <gdk/gdkkeysyms.h>

static nsSetupType     *sCustomST; // cache a pointer to the custom setup type
static GtkWidget       *sDescLong;
static gint             sCurrRowSelected;

nsComponentsDlg::nsComponentsDlg() :
    mMsg0(NULL),
    mCompList(NULL)
{
}

nsComponentsDlg::~nsComponentsDlg()
{
    XI_IF_FREE(mMsg0);
    XI_IF_DELETE(mCompList);
}

void
nsComponentsDlg::Back(GtkWidget *aWidget, gpointer aData)
{
    DUMP("Back");
    if (aData != gCtx->cdlg) return;

    // hide this notebook page
    gCtx->cdlg->Hide(nsXInstallerDlg::BACKWARD_MOVE);

    // disconnect this dlg's nav btn signal handlers
    gtk_signal_disconnect(GTK_OBJECT(gCtx->back), gCtx->backID);
    gtk_signal_disconnect(GTK_OBJECT(gCtx->next), gCtx->nextID);

    gCtx->sdlg->Show(nsXInstallerDlg::BACKWARD_MOVE);
}

void
nsComponentsDlg::Next(GtkWidget *aWidget, gpointer aData)
{
    DUMP("Next");
    if (aData != gCtx->cdlg) return;

	if (OK != nsSetupTypeDlg::VerifyDiskSpace())
	    return;

    // hide this notebook page
    gCtx->cdlg->Hide(nsXInstallerDlg::FORWARD_MOVE);

    // disconnect this dlg's nav btn signal handlers
    gtk_signal_disconnect(GTK_OBJECT(gCtx->back), gCtx->backID);
    gtk_signal_disconnect(GTK_OBJECT(gCtx->next), gCtx->nextID);

    // show the next dlg
    gCtx->idlg->Show(nsXInstallerDlg::FORWARD_MOVE);
}

int
nsComponentsDlg::Parse(nsINIParser *aParser)
{
    int err = OK;
    char *showDlg = NULL;
    int bufsize = 0;

    char *currSec = (char *) malloc(strlen(COMPONENT) + 3);
    if (!currSec) return E_MEM;
    XI_VERIFY(gCtx);

    /* optional keys */
    err = aParser->GetStringAlloc(DLG_COMPONENTS, MSG0, &mMsg0, &bufsize);
    if (err != OK && err != nsINIParser::E_NO_KEY) goto BAIL; else err = OK;

    bufsize = 5;
    err = aParser->GetStringAlloc(DLG_COMPONENTS, SHOW_DLG, &showDlg, &bufsize);
    if (err != OK && err != nsINIParser::E_NO_KEY) goto BAIL; else err = OK;
    if (bufsize != 0 && showDlg)
    {
        if (0 == strncmp(showDlg, "TRUE", 4))
            mShowDlg = nsXInstallerDlg::SHOW_DIALOG;
        else if (0 == strncmp(showDlg, "FALSE", 5))
            mShowDlg = nsXInstallerDlg::SKIP_DIALOG;
    }

    bufsize = 0;
    err = aParser->GetStringAlloc(DLG_COMPONENTS, TITLE, &mTitle, &bufsize);
    if (err != OK  && err != nsINIParser::E_NO_KEY) goto BAIL; else err = OK;
    if (bufsize == 0)
            XI_IF_FREE(mTitle); 

BAIL:
    XI_IF_FREE(currSec);

    return err;
}

int
nsComponentsDlg::Show(int aDirection)
{
    int err = OK;
    int customSTIndex = 0, i;
    int numRows = 0;
    int currRow = 0;
    GtkWidget *hbox = NULL;

    XI_VERIFY(gCtx);
    XI_VERIFY(gCtx->notebook);

    if (mWidgetsInit == FALSE)
    {
        customSTIndex = gCtx->sdlg->GetNumSetupTypes();
        sCustomST = gCtx->sdlg->GetSetupTypeList();
        for (i=1; i<customSTIndex; i++)
            sCustomST = sCustomST->GetNext();
        DUMP(sCustomST->GetDescShort());

        // create a new table and add it as a page of the notebook
        mTable = gtk_table_new(5, 1, FALSE);
        gtk_notebook_append_page(GTK_NOTEBOOK(gCtx->notebook), mTable, NULL);
        mPageNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(gCtx->notebook));
        gtk_widget_show(mTable);

        // 1st row: a label (msg0)
        // insert a static text widget in the first row
        GtkWidget *msg0 = gtk_label_new(mMsg0);
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), msg0, FALSE, FALSE, 0);
        gtk_widget_show(hbox);
        gtk_table_attach(GTK_TABLE(mTable), hbox, 0, 1, 1, 2,
                         static_cast<GtkAttachOptions>(GTK_FILL | GTK_EXPAND),
			             GTK_FILL, 20, 20);
        gtk_widget_show(msg0);

        // 2nd row: a CList with a check box for each row (short desc)
        GtkWidget *list = NULL;
        GtkWidget *scrollwin = NULL;
        GtkStyle *style = NULL;
        GdkBitmap *ch_mask = NULL;
        GdkPixmap *checked = NULL;
        GdkBitmap *un_mask = NULL;
        GdkPixmap *unchecked = NULL;
        gchar *dummy[2] = { " ", " " };
        nsComponent *currComp = sCustomST->GetComponents()->GetHead();
        GtkWidget *descLongTable = NULL;
        GtkWidget *frame = NULL;

        scrollwin = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

        list = gtk_clist_new(2);
        gtk_clist_set_selection_mode(GTK_CLIST(list), GTK_SELECTION_BROWSE);
        gtk_clist_column_titles_hide(GTK_CLIST(list));
        gtk_clist_set_column_auto_resize(GTK_CLIST(list), 0, TRUE);
        gtk_clist_set_column_auto_resize(GTK_CLIST(list), 1, TRUE);

        // determine number of rows we'll need
        numRows = sCustomST->GetComponents()->GetLengthVisible();
        for (i = 0; i < numRows; i++)
            gtk_clist_append(GTK_CLIST(list), dummy);
    
        style = gtk_widget_get_style(gCtx->window);
        checked = gdk_pixmap_create_from_xpm_d(gCtx->window->window, &ch_mask, 
                  &style->bg[GTK_STATE_NORMAL], (gchar **)check_on_xpm);
        unchecked = gdk_pixmap_create_from_xpm_d(gCtx->window->window, &un_mask,
                    &style->bg[GTK_STATE_NORMAL], (gchar **)check_off_xpm);

        while ((currRow < numRows) && currComp) // paranoia!
        {
            if (!currComp->IsInvisible())
            {
                if (currComp->IsSelected()) 
                    gtk_clist_set_pixmap(GTK_CLIST(list), currRow, 0, 
                                         checked, ch_mask);
                else
                    gtk_clist_set_pixmap(GTK_CLIST(list), currRow, 0, 
                                         unchecked, un_mask);

                gtk_clist_set_text(GTK_CLIST(list), currRow, 1,
                                   currComp->GetDescShort());
                currRow++;
            }

            currComp = currComp->GetNext();
        }

        // by default, first row selected upon Show()
        sCurrRowSelected = 0; 

        gtk_signal_connect(GTK_OBJECT(list), "select_row",
                           GTK_SIGNAL_FUNC(RowSelected), NULL);
        gtk_signal_connect(GTK_OBJECT(list), "key_press_event",
                           GTK_SIGNAL_FUNC(KeyPressed), NULL);
        gtk_container_add(GTK_CONTAINER(scrollwin), list);
        gtk_widget_show(list);
        gtk_widget_show(scrollwin);

        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), scrollwin, TRUE, TRUE, 0);
        gtk_widget_show(hbox);
        gtk_table_attach(GTK_TABLE(mTable), hbox, 0, 1, 2, 3,
                         static_cast<GtkAttachOptions>(GTK_FILL | GTK_EXPAND),
			static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL),
			 20, 0);

        // XXX     3rd row: labels for ds avail and ds reqd

        // 4th row: a frame with a label (long desc)
        descLongTable = gtk_table_new(1, 1, FALSE);
        gtk_widget_show(descLongTable);

        gtk_table_attach(GTK_TABLE(mTable), descLongTable, 0, 1, 4, 5,
            static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL),
            static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL),
			20, 20);
        frame = gtk_frame_new(gCtx->Res("DESCRIPTION"));
        gtk_table_attach_defaults(GTK_TABLE(descLongTable), frame, 0, 1, 0, 1);
        gtk_widget_show(frame);

        sDescLong = gtk_label_new(
            sCustomST->GetComponents()->GetFirstVisible()->GetDescLong());
        gtk_label_set_line_wrap(GTK_LABEL(sDescLong), TRUE);
        hbox = gtk_hbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), sDescLong, FALSE, FALSE, 20);
        gtk_widget_show(hbox);

        gtk_table_attach_defaults(GTK_TABLE(descLongTable), hbox, 0, 1, 0, 1);
        gtk_widget_show(sDescLong);

        mWidgetsInit = TRUE;
    }
    else
    {
        gtk_notebook_set_page(GTK_NOTEBOOK(gCtx->notebook), mPageNum);
        gtk_widget_show(mTable);
    }

    // signal connect the buttons
    gCtx->backID = gtk_signal_connect(GTK_OBJECT(gCtx->back), "clicked",
                   GTK_SIGNAL_FUNC(nsComponentsDlg::Back), gCtx->cdlg);
    gCtx->nextID = gtk_signal_connect(GTK_OBJECT(gCtx->next), "clicked",
                   GTK_SIGNAL_FUNC(nsComponentsDlg::Next), gCtx->cdlg);

    // show back btn again after setup type dlg where we couldn't go back
    gtk_widget_set_sensitive(gCtx->back, TRUE);

    if (aDirection == nsXInstallerDlg::BACKWARD_MOVE) // from install dlg
      gtk_button_set_label(GTK_BUTTON(gCtx->next), GTK_STOCK_GO_FORWARD);

    return err;
}

int
nsComponentsDlg::Hide(int aDirection)
{
    gtk_widget_hide(mTable);

    return OK;
}

int
nsComponentsDlg::SetMsg0(char *aMsg)
{
    if (!aMsg)
        return E_PARAM;

    mMsg0 = aMsg;
    
    return OK;
}

char *
nsComponentsDlg::GetMsg0()
{
    if (mMsg0)
        return mMsg0;

    return NULL;
}

int
nsComponentsDlg::SetCompList(nsComponentList *aCompList)
{
    if (!aCompList)
        return E_PARAM;

    mCompList = aCompList;

    return OK;
}

nsComponentList *
nsComponentsDlg::GetCompList()
{
    if (mCompList)
        return mCompList;

    return NULL;
}

void
nsComponentsDlg::RowSelected(GtkWidget *aWidget, gint aRow, gint aColumn,
                             GdkEventButton *aEvent, gpointer aData)
{
    DUMP("RowSelected");

    sCurrRowSelected = aRow;

    // only toggle row selection state for clicks on the row
    if (aColumn == -1 && !aEvent)
        return;

    ToggleRowSelection(aWidget, aRow,
                       (aColumn == 0 && aEvent->button == 1));
}

gboolean
nsComponentsDlg::KeyPressed(GtkWidget *aWidget, GdkEventKey *aEvent, 
                            gpointer aData)
{
  DUMP("KeyPressed");

  if (aEvent->keyval == GDK_space)
      ToggleRowSelection(aWidget, sCurrRowSelected, TRUE);

  return FALSE;
}

void
nsComponentsDlg::ToggleRowSelection(GtkWidget *aWidget, gint aRow,
                                    gboolean aToggleState)
{
    int numRows = 0, currRow = 0;
    GtkStyle *style = NULL;
    GdkBitmap *ch_mask = NULL;
    GdkPixmap *checked = NULL;
    GdkBitmap *un_mask = NULL;
    GdkPixmap *unchecked = NULL;
    nsComponent *currComp = sCustomST->GetComponents()->GetHead();
    
    style = gtk_widget_get_style(gCtx->window);
    checked = gdk_pixmap_create_from_xpm_d(gCtx->window->window, &ch_mask, 
              &style->bg[GTK_STATE_NORMAL], (gchar **)check_on_xpm);
    unchecked = gdk_pixmap_create_from_xpm_d(gCtx->window->window, &un_mask,
                &style->bg[GTK_STATE_NORMAL], (gchar **)check_off_xpm);

    numRows = sCustomST->GetComponents()->GetLengthVisible();
    while ((currRow < numRows) && currComp) // paranoia!
    {
        if (!currComp->IsInvisible())
        {
            if (aRow == currRow)
            {
                // update long desc
                gtk_label_set_text(GTK_LABEL(sDescLong),
                                   currComp->GetDescLong());
                gtk_widget_show(sDescLong);

                if (aToggleState) {
                  if (currComp->IsSelected()) {
                    DUMP("Toggling off...");
                    currComp->SetUnselected();
                  } else {
                    DUMP("Toggling on...");
                    currComp->SetSelected();
                  }
                }
                currComp->ResolveDependees(currComp->IsSelected(),
                                            sCustomST->GetComponents());
                break;
            }
            currRow++;
        }
        currComp = currComp->GetNext();
    }

    // after resolving dependees redraw all checkboxes in one fell swoop
    currRow = 0;
    currComp = sCustomST->GetComponents()->GetHead();
    while ((currRow < numRows) && currComp) // paranoia!
    {
        if (!currComp->IsInvisible())
        {
            if (currComp->IsSelected())
            {
                gtk_clist_set_pixmap(GTK_CLIST(aWidget), currRow, 0, 
                                     checked, ch_mask);
            }
            else
            {
                gtk_clist_set_pixmap(GTK_CLIST(aWidget), currRow, 0, 
                                     unchecked, un_mask);
            }
            currRow++;
        }
        currComp = currComp->GetNext();
    }
}
