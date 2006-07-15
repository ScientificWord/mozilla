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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Aaron Reed <aaronr@us.ibm.com>
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

#ifndef TRANSFRMX_XFORMS_FUNCTIONS_H
#define TRANSFRMX_XFORMS_FUNCTIONS_H

#include "txExpr.h"
#include "nsIDOMNode.h"

#define NS_NAMESPACE_XFORMS              "http://www.w3.org/2002/xforms"
#define NS_NAMESPACE_SCHEMA              "http://www.w3.org/1999/XMLSchema"

/*
 * Represents the XPath XForms Function Calls
 */
class XFormsFunctionCall : public FunctionCall {

public:

    enum XFormsFunctions {
        AVG,                  // avg()
        BOOLEANFROMSTRING,    // boolean-from-string()
        COUNTNONEMPTY,        // count-non-empty()
        DAYSFROMDATE,         // days-from-date()
        IF,                   // if()
        INDEX,                // index()
        INSTANCE,             // instance()
        MAX,                  // max()
        MIN,                  // min()
        MONTHS,               // months()
        NOW,                  // now()
        PROPERTY,             // property()
        SECONDS,              // seconds()
        SECONDSFROMDATETIME   // seconds-from-dateTime()
    };

    /*
     * Creates a Number function of the given type
     */
    XFormsFunctionCall(XFormsFunctions aType, nsIDOMNode *resolverNode=nsnull);

    TX_DECL_FUNCTION;

private:
    XFormsFunctions mType;
    nsCOMPtr<nsIDOMNode> mResolverNode;
};

#endif
