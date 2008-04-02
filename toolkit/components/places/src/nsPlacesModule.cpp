#include "nsIGenericFactory.h"

#include "nsAnnoProtocolHandler.h"
#include "nsAnnotationService.h"
#include "nsNavHistory.h"
#include "nsNavBookmarks.h"
#include "nsFaviconService.h"
#include "nsDocShellCID.h"

NS_GENERIC_FACTORY_SINGLETON_CONSTRUCTOR(nsNavHistory,
                                         nsNavHistory::GetSingleton)
NS_GENERIC_FACTORY_CONSTRUCTOR(nsAnnoProtocolHandler)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsAnnotationService, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsNavBookmarks, Init)
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(nsFaviconService, Init)

static const nsModuleComponentInfo components[] =
{
  { "Browser Navigation History",
    NS_NAVHISTORYSERVICE_CID,
    NS_NAVHISTORYSERVICE_CONTRACTID,
    nsNavHistoryConstructor },

  { "Browser Navigation History",
    NS_NAVHISTORYSERVICE_CID,
    "@mozilla.org/browser/global-history;2",
    nsNavHistoryConstructor },

  { "Browser Navigation History",
    NS_NAVHISTORYSERVICE_CID,
    "@mozilla.org/autocomplete/search;1?name=history",
    nsNavHistoryConstructor },

  { "Download Navigation History",
    NS_NAVHISTORYSERVICE_CID,
    NS_DOWNLOADHISTORY_CONTRACTID,
    nsNavHistoryConstructor },

  { "Page Annotation Service",
    NS_ANNOTATIONSERVICE_CID,
    NS_ANNOTATIONSERVICE_CONTRACTID,
    nsAnnotationServiceConstructor },

  { "Annotation Protocol Handler",
    NS_ANNOPROTOCOLHANDLER_CID,
    NS_NETWORK_PROTOCOL_CONTRACTID_PREFIX "moz-anno",
    nsAnnoProtocolHandlerConstructor },

  { "Browser Bookmarks Service",
    NS_NAVBOOKMARKSSERVICE_CID,
    NS_NAVBOOKMARKSSERVICE_CONTRACTID,
    nsNavBookmarksConstructor },

  { "Favicon Service",
    NS_FAVICONSERVICE_CID,
    NS_FAVICONSERVICE_CONTRACTID,
    nsFaviconServiceConstructor },

};

NS_IMPL_NSGETMODULE(nsPlacesModule, components)
