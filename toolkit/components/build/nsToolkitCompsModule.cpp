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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Joe Hewitt <hewitt@netscape.com> (Original Author)
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

#include "nsIGenericFactory.h"
#include "nsAppStartup.h"
#include "nsUserInfo.h"
#include "nsXPFEComponentsCID.h"
#include "nsToolkitCompsCID.h"
#ifdef ALERTS_SERVICE
#include "nsAlertsService.h"
#endif

#ifndef MOZ_SUITE
// XXX Suite isn't ready to include this just yet
#ifdef MOZ_RDF
#include "nsDownloadManager.h"
#include "nsDownloadProxy.h"
#endif
#endif // MOZ_SUITE

#include "nsTypeAheadFind.h"

#ifdef MOZ_URL_CLASSIFIER
#include "nsUrlClassifierDBService.h"
#include "nsUrlClassifierStreamUpdater.h"
#include "nsUrlClassifierUtils.h"
#include "nsUrlClassifierHashCompleter.h"
#include "nsDocShellCID.h"
#endif

#ifdef MOZ_FEEDS
#include "nsScriptableUnescapeHTML.h"
#endif

#include "nsAutoCompleteController.h"
#include "nsAutoCompleteSimpleResult.h"
#include "nsAutoCompleteStringArray.h"
#include "msiKeyMap.h"



/////////////////////////////////////////////////////////////////////////////

NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsAppStartup, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsUserInfo)

#ifdef ALERTS_SERVICE
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAlertsService)
#endif

#ifndef MOZ_SUITE
// XXX Suite isn't ready to include this just yet
#ifdef MOZ_RDF
NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(nsDownloadManager,
                                         nsDownloadManager::GetSingleton) 
NS_GENERIC_FACTORY_CONSTRUCTOR(nsDownloadProxy)
#endif
#endif // MOZ_SUITE

NS_GENERIC_FACTORY_CONSTRUCTOR(nsTypeAheadFind)

#ifdef MOZ_URL_CLASSIFIER
NS_GENERIC_FACTORY_CONSTRUCTOR(nsUrlClassifierStreamUpdater)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsUrlClassifierUtils, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsUrlClassifierHashCompleter, Init)

static NS_IMETHODIMP
nsUrlClassifierDBServiceConstructor(nsISupports *aOuter, REFNSIID aIID,
                                    void **aResult)
{
    nsresult rv;
    NS_ENSURE_ARG_POINTER(aResult);
    NS_ENSURE_NO_AGGREGATION(aOuter);

    nsUrlClassifierDBService *inst = nsUrlClassifierDBService::GetInstance(&rv);
    if (NULL == inst) {
        return rv;
    }
    /* NS_ADDREF(inst); */
    rv = inst->QueryInterface(aIID, aResult);
    NS_RELEASE(inst);

    return rv;
}
#endif

#ifdef MOZ_FEEDS
NS_GENERIC_FACTORY_CONSTRUCTOR(nsScriptableUnescapeHTML)
#endif

NS_GENERIC_FACTORY_CONSTRUCTOR(nsAutoCompleteController)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAutoCompleteSimpleResult)
NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(nsAutoCompleteSearchStringArray, nsAutoCompleteSearchStringArray::GetInstance)
#ifdef MOZ_MORK
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAutoCompleteMdbResult)
#endif

NS_GENERIC_FACTORY_CONSTRUCTOR(msiKeyMap)
/////////////////////////////////////////////////////////////////////////////

static const nsModuleComponentInfo components[] =
{
  { "App Startup Service",
    NS_TOOLKIT_APPSTARTUP_CID,
    NS_APPSTARTUP_CONTRACTID,
    nsAppStartupConstructor },

 
  { "User Info Service",
    NS_USERINFO_CID,
    NS_USERINFO_CONTRACTID,
    nsUserInfoConstructor },
#ifdef ALERTS_SERVICE
  { "Alerts Service",
    NS_ALERTSSERVICE_CID, 
    NS_ALERTSERVICE_CONTRACTID,
    nsAlertsServiceConstructor },
#endif
#ifndef MOZ_SUITE
// XXX Suite isn't ready to include this just yet
#ifdef MOZ_RDF
  { "Download Manager",
    NS_DOWNLOADMANAGER_CID,
    NS_DOWNLOADMANAGER_CONTRACTID,
    nsDownloadManagerConstructor },
  { "Download",
    NS_DOWNLOAD_CID,
    NS_TRANSFER_CONTRACTID,
    nsDownloadProxyConstructor },
#endif
#endif // MOZ_SUITE
  { "TypeAheadFind Component",
    NS_TYPEAHEADFIND_CID,
    NS_TYPEAHEADFIND_CONTRACTID,
    nsTypeAheadFindConstructor
  },
#ifdef MOZ_URL_CLASSIFIER
  { "Url Classifier DB Service",
    NS_URLCLASSIFIERDBSERVICE_CID,
    NS_URLCLASSIFIERDBSERVICE_CONTRACTID,
    nsUrlClassifierDBServiceConstructor },
  { "Url Classifier DB Service",
    NS_URLCLASSIFIERDBSERVICE_CID,
    NS_URICLASSIFIERSERVICE_CONTRACTID,
    nsUrlClassifierDBServiceConstructor },
  { "Url Classifier Stream Updater",
    NS_URLCLASSIFIERSTREAMUPDATER_CID,
    NS_URLCLASSIFIERSTREAMUPDATER_CONTRACTID,
    nsUrlClassifierStreamUpdaterConstructor },
  { "Url Classifier Utils",
    NS_URLCLASSIFIERUTILS_CID,
    NS_URLCLASSIFIERUTILS_CONTRACTID,
    nsUrlClassifierUtilsConstructor },
  { "Url Classifier Hash Completer",
    NS_URLCLASSIFIERHASHCOMPLETER_CID,
    NS_URLCLASSIFIERHASHCOMPLETER_CONTRACTID,
    nsUrlClassifierHashCompleterConstructor },
#endif
#ifdef MOZ_FEEDS
  { "Unescape HTML",
    NS_SCRIPTABLEUNESCAPEHTML_CID,
    NS_SCRIPTABLEUNESCAPEHTML_CONTRACTID,
    nsScriptableUnescapeHTMLConstructor },
#endif
 { "AutoComplete Controller",
   NS_AUTOCOMPLETECONTROLLER_CID, 
   NS_AUTOCOMPLETECONTROLLER_CONTRACTID,
   nsAutoCompleteControllerConstructor },

 { "AutoComplete Simple Result",
   NS_AUTOCOMPLETESIMPLERESULT_CID, 
   NS_AUTOCOMPLETESIMPLERESULT_CONTRACTID,
   nsAutoCompleteSimpleResultConstructor },

 { "String Array Autocomplete",
   NS_STRINGARRAYAUTOCOMPLETE_CID,
   NS_STRINGARRAYAUTOCOMPLETE_CONTRACTID,
   nsAutoCompleteSearchStringArrayConstructor },

#ifdef MOZ_MORK
 { "AutoComplete Mdb Result",
   NS_AUTOCOMPLETEMDBRESULT_CID, 
   NS_AUTOCOMPLETEMDBRESULT_CONTRACTID,
   nsAutoCompleteMdbResultConstructor },
#endif     
{ 
  "Key Mapping Service",
  MSI_KEYMAP_CID,
  MSI_KEYMAP_CONTRACTID,
  msiKeyMapConstructor },
};


NS_IMPL_NSGETMODULE(nsToolkitCompsModule, components)
