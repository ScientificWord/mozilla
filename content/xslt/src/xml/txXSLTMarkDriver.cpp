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
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Axel Hecht <axel@pike.org> (Original Author)
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
 * Driver for running XSLTMark on standalone
 *
 * See http://www.datapower.com/XSLTMark/
 */

#include "txStandaloneStylesheetCompiler.h"
#include "txStandaloneXSLTProcessor.h"
#include "nsXPCOM.h"
#include "xmExternalDriver.hpp"
#ifdef MOZ_JPROF
#include "jprof.h"
#endif

class txDriverProcessor : public txStandaloneXSLTProcessor,
                          public xmExternalDriver
{
public:
    txDriverProcessor() : mXML(0), mOut(0)
    {
    }

    int loadStylesheet (char * filename)
    {
        txParsedURL url;
        url.init(NS_ConvertASCIItoUTF16(filename));
        nsresult rv =
            TX_CompileStylesheetPath(url, getter_AddRefs(mStylesheet));
        return NS_SUCCEEDED(rv) ? 0 : 1;
    }
    int setInputDocument (char * filename)
    {
        delete mXML;
        mXML = parsePath(nsDependentCString(filename), mObserver);
        return mXML ? 0 : 1;
    }
    int openOutput (char * outputFilename)
    {
        mOut = new ofstream(outputFilename);
        return mXML ? 0 : 1;
    }
    int runTransform ()
    {
        if (!mXML || !mStylesheet || !mOut)
            return 1;
        nsresult rv = transform(*mXML, mStylesheet, *mOut, mObserver);
        return NS_FAILED(rv);
    }
    int closeOutput ()
    {
        if (mOut)
            mOut->close();
        delete mOut;
        mOut = 0;
        return 0;
    }
    int terminate()
    {
        delete mXML;
        mXML = 0;
        if (mOut && mOut->is_open())
            mOut->close();
        delete mOut;
        mOut = 0;
        mStylesheet = 0;
        return 0;
    }
    ~txDriverProcessor()
    {
        delete mXML;
        delete mOut;
    }
private:
    txXPathNode *mXML;
    nsRefPtr<txStylesheet> mStylesheet;
    SimpleErrorObserver mObserver;
    ofstream* mOut;
};

int main (int argc, char ** argv)
{
    txDriverProcessor driver;
#ifdef MOZ_JPROF
    setupProfilingStuff();
#endif
    NS_InitXPCOM2(nsnull, nsnull, nsnull);
    if (!txDriverProcessor::init())
        return 1;
    driver.main (argc, argv);
    txDriverProcessor::shutdown();
    NS_ShutdownXPCOM(nsnull);
    return 0;
}
