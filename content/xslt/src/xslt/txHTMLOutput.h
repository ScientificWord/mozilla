/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is TransforMiiX XSLT processor code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Peter Van der Beken <peterv@propagandism.org>
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

#ifndef TRANSFRMX_HTML_OUTPUT_H
#define TRANSFRMX_HTML_OUTPUT_H

#include "txXMLOutput.h"

class txHTMLOutput : public txXMLOutput
{
public:
    txHTMLOutput(txOutputFormat* aFormat, ostream* aOut);

    /**
     * Init/release table with shorthands.
     */
    static nsresult init();
    static void shutdown();

    nsresult attribute(const nsAString& aName, const PRInt32 aNsID,
                       const nsAString& aValue);
    nsresult characters(const nsAString& aData, PRBool aDOE);
    nsresult endElement(const nsAString& aName, const PRInt32 aNsID);
    nsresult processingInstruction(const nsAString& aTarget,
                                   const nsAString& aData);
    nsresult startDocument();
    nsresult startElement(const nsAString& aName, const PRInt32 aNsID);
    TX_DECL_TXAOUTPUTXMLEVENTHANDLER

private:
    void closeStartTag(MBool aUseEmptyElementShorthand);
    MBool isShorthandElement(const nsAString& aName);
    MBool isShorthandAttribute(const nsAString& aLocalName);

    txStack mCurrentElements;
};

#endif
