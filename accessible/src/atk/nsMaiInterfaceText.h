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

#ifndef __MAI_INTERFACE_TEXT_H__
#define __MAI_INTERFACE_TEXT_H__

#include "nsMai.h"
#include "nsIAccessibleText.h"

G_BEGIN_DECLS

void textInterfaceInitCB(AtkTextIface *aIface);

/* text interface callbacks */
gchar *getTextCB(AtkText *aText,
                 gint aStartOffset, gint aEndOffset);
gchar *getTextAfterOffsetCB(AtkText *aText, gint aOffset,
                            AtkTextBoundary aBoundaryType,
                            gint *aStartOffset, gint *aEndOffset);
gchar *getTextAtOffsetCB(AtkText *aText, gint aOffset,
                         AtkTextBoundary aBoundaryType,
                         gint *aStartOffset, gint *aEndOffset);
gunichar getCharacterAtOffsetCB(AtkText *aText, gint aOffset);
gchar *getTextBeforeOffsetCB(AtkText *aText, gint aOffset,
                             AtkTextBoundary aBoundaryType,
                             gint *aStartOffset, gint *aEndOffset);
gint getCaretOffsetCB(AtkText *aText);
AtkAttributeSet *getRunAttributesCB(AtkText *aText, gint aOffset,
                                    gint *aStartOffset,
                                    gint *aEndOffset);
AtkAttributeSet* getDefaultAttributesCB(AtkText *aText);
void getCharacterExtentsCB(AtkText *aText, gint aOffset,
                           gint *aX, gint *aY,
                           gint *aWidth, gint *aHeight,
                           AtkCoordType aCoords);
void getRangeExtentsCB(AtkText *aText, gint aStartOffset,
                       gint aEndOffset, AtkCoordType aCoords,
                       AtkTextRectangle *aRect);
gint getCharacterCountCB(AtkText *aText);
gint getOffsetAtPointCB(AtkText *aText,
                        gint aX, gint aY,
                        AtkCoordType aCoords);
gint getTextSelectionCountCB(AtkText *aText);
gchar *getTextSelectionCB(AtkText *aText, gint aSelectionNum,
                          gint *aStartOffset, gint *aEndOffset);

// set methods
gboolean addTextSelectionCB(AtkText *aText,
                            gint aStartOffset,
                            gint aEndOffset);
gboolean removeTextSelectionCB(AtkText *aText,
                               gint aSelectionNum);
gboolean setTextSelectionCB(AtkText *aText, gint aSelectionNum,
                            gint aStartOffset, gint aEndOffset);
gboolean setCaretOffsetCB(AtkText *aText, gint aOffset);

/*************************************************
 // signal handlers
 //
    void TextChangedCB(AtkText *aText, gint aPosition, gint aLength);
    void TextCaretMovedCB(AtkText *aText, gint aLocation);
    void TextSelectionChangedCB(AtkText *aText);
*/
G_END_DECLS

#endif /* __MAI_INTERFACE_TEXT_H__ */
