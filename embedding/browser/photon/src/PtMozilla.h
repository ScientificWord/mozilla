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
 * Christopher Blizzard.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Christopher Blizzard <blizzard@mozilla.org>
 *   Brian Edmond <briane@qnx.com>
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


#ifndef __PT_MOZILLA_H_INCLUDED
#define __PT_MOZILLA_H_INCLUDED

#include <stdlib.h>
#include <string.h>

#include <photon/PtWebClient.h>
#include <photon/PpProto.h>

#if 0
#include "nsIInputStream.h"
#include "nsILoadGroup.h"
#include "nsIChannel.h"
#include "nsIContentViewer.h"
#include "nsIStreamListener.h"
#include "nsISHistory.h"
#include "nsIHistoryEntry.h"
#endif

/*
 * PtMozilla public
 */

extern PtWidgetClassRef_t *PtMozilla;

class EmbedDownload; /* forward declaration */

/* Resources */

#define Pt_ARG_MOZ_GET_URL					Pt_RESOURCE( 104,  0 )
#define Pt_ARG_MOZ_NAVIGATE_PAGE		Pt_RESOURCE( 104,  1 )
#define Pt_ARG_MOZ_RELOAD						Pt_RESOURCE( 104,  2 )
#define Pt_ARG_MOZ_STOP							Pt_RESOURCE( 104,  3 )
#define Pt_ARG_MOZ_PRINT						Pt_RESOURCE( 104,  4 )
#define Pt_ARG_MOZ_COMMAND        	Pt_RESOURCE( 104,  5 )
#define Pt_ARG_MOZ_OPTION						Pt_RESOURCE( 104,  6 )
#define Pt_ARG_MOZ_ENCODING					Pt_RESOURCE( 104,  7 )
#define Pt_ARG_MOZ_WEB_DATA					Pt_RESOURCE( 104,  8 )
#define Pt_ARG_MOZ_WEB_DATA_URL			Pt_RESOURCE( 104,  9 )
#define Pt_ARG_MOZ_GET_CONTEXT			Pt_RESOURCE( 104,  10 )
#define Pt_ARG_MOZ_UNKNOWN_RESP			Pt_RESOURCE( 104,  11 )
#define Pt_ARG_MOZ_DOWNLOAD					Pt_RESOURCE( 104,  12 )
#define Pt_ARG_MOZ_GET_HISTORY			Pt_RESOURCE( 104,  13 )
#define Pt_ARG_MOZ_AUTH_CTRL				Pt_RESOURCE( 104, 14 ) // used internally for authentification
#define Pt_ARG_MOZ_UNKNOWN_CTRL			Pt_RESOURCE( 104, 15 ) // used internally for downloading

#define Pt_CB_MOZ_PROGRESS					Pt_RESOURCE( 104,  20 )
#define Pt_CB_MOZ_START							Pt_RESOURCE( 104,  21 )
#define Pt_CB_MOZ_COMPLETE					Pt_RESOURCE( 104,  22 )
#define Pt_CB_MOZ_NET_STATE         Pt_RESOURCE( 104,  23 )
#define Pt_CB_MOZ_URL								Pt_RESOURCE( 104,  24 )
#define Pt_CB_MOZ_INFO							Pt_RESOURCE( 104,  25 )
#define Pt_CB_MOZ_OPEN            	Pt_RESOURCE( 104,  26 )
#define Pt_CB_MOZ_NEW_WINDOW       	Pt_RESOURCE( 104,  27 )
#define Pt_CB_MOZ_DIALOG       			Pt_RESOURCE( 104,  28 )
#define Pt_CB_MOZ_AUTHENTICATE     	Pt_RESOURCE( 104,  29 )
#define Pt_CB_MOZ_PROMPT	     			Pt_RESOURCE( 104,  30 )
#define Pt_CB_MOZ_NEW_AREA      		Pt_RESOURCE( 104,  31 )
#define Pt_CB_MOZ_VISIBILITY        Pt_RESOURCE( 104,  32 )
#define Pt_CB_MOZ_DESTROY        		Pt_RESOURCE( 104,  33 )
#define Pt_CB_MOZ_EVENT	        		Pt_RESOURCE( 104,  35 )
#define Pt_CB_MOZ_CONTEXT        		Pt_RESOURCE( 104,  36 )
#define Pt_CB_MOZ_PRINT_STATUS     	Pt_RESOURCE( 104,  37 )
#define Pt_CB_MOZ_WEB_DATA_REQ     	Pt_RESOURCE( 104,  38 )
#define Pt_CB_MOZ_UNKNOWN						Pt_RESOURCE( 104,  39 )
#define Pt_CB_MOZ_ERROR							Pt_RESOURCE( 104,  40 )
#define Pt_CB_MOZ_DOWNLOAD					Pt_RESOURCE( 104,  41 )


#define MAX_URL_LENGTH						1024

typedef enum
{
  MOZ_EMBED_FLAG_RELOADNORMAL = 0,
  MOZ_EMBED_FLAG_RELOADBYPASSCACHE = 1,
  MOZ_EMBED_FLAG_RELOADBYPASSPROXY = 2,
  MOZ_EMBED_FLAG_RELOADBYPASSPROXYANDCACHE = 3,
  MOZ_EMBED_FLAG_RELOADCHARSETCHANGE = 4
} MozEmbedReloadFlags;

// progress callback, can be itemized or full
#define Pt_MOZ_PROGRESS		1
#define Pt_MOZ_PROGRESS_ALL	2
typedef struct mozilla_progress_t
{
	int 	type; // unused at this time
	int32_t 	cur;
	int32_t 	max;
} PtMozillaProgressCb_t;

// url change callback, also used for open callback
// the open callback returns Pt_END to cancel the open of that site
typedef struct mozilla_url_t
{
	char *url;
} PtMozillaUrlCb_t;

// info callback for Java Script, link messages, titles
#define Pt_MOZ_INFO_TITLE		1
#define Pt_MOZ_INFO_JSSTATUS	2
#define Pt_MOZ_INFO_LINK		3
#define Pt_MOZ_INFO_SSL			4 // uses status field
	// status can be
	#define Pt_SSL_STATE_IS_INSECURE	0x1
	#define Pt_SSL_STATE_IS_BROKEN		0x2
	#define Pt_SSL_STATE_IS_SECURE		0x4
	#define Pt_SSL_STATE_SECURE_HIGH	0x8
	#define Pt_SSL_STATE_SECURE_MED		0x10
	#define Pt_SSL_STATE_SECURE_LOW		0x20
#define Pt_MOZ_INFO_CONNECT	5

typedef struct mozilla_info_t
{
	int				type;
	unsigned long	status;
	char 			*data;
} PtMozillaInfoCb_t;

// authentication callback, fill username and password in
// return Pt_END from the callback to cancel authentication
typedef struct mozilla_authenticate_t
{
	char *title;
	char *realm;
	char user[128];
	char pass[128];
} PtMozillaAuthenticateCb_t;

// dialog callback, the _CHECK ones use message and expect an answer in ret_value
// the CONFIRM dialogs must return Pt_CONTINUE to confirm and Pt_END to deny
#define Pt_MOZ_DIALOG_ALERT			1
#define Pt_MOZ_DIALOG_ALERT_CHECK	2
#define Pt_MOZ_DIALOG_CONFIRM		3
#define Pt_MOZ_DIALOG_CONFIRM_CHECK	4
typedef struct mozilla_dialog_t
{
	int 	type;
	PtWidget_t *parent;
	char 	*title;
	char 	*text;
	char 	*checkbox_message;
	int 	ret_value;
} PtMozillaDialogCb_t;

// raw net state callback
// flags can be:
// 		STATE_START
// 		STATE_REDIRECTING
// 		STATE_TRANSFERRING
// 		STATE_NEGOTIATING
// 		STATE_STOP
// 		STATE_IS_REQUEST
// 		STATE_IS_DOCUMENT
// 		STATE_IS_NETWORK
// 		STATE_IS_WINDOW
typedef struct mozilla_net_state_t
{
        int 			flags;
        unsigned int 	status;
		const char *url;
		char *message;
} PtMozillaNetStateCb_t;

// new window callback
typedef enum
{
  Pt_MOZ_FLAG_DEFAULTCHROME = 1U,
  Pt_MOZ_FLAG_WINDOWBORDERSON = 2U,
  Pt_MOZ_FLAG_WINDOWCLOSEON = 4U,
  Pt_MOZ_FLAG_WINDOWRESIZEON = 8U,
  Pt_MOZ_FLAG_MENUBARON = 16U,
  Pt_MOZ_FLAG_TOOLBARON = 32U,
  Pt_MOZ_FLAG_LOCATIONBARON = 64U,
  Pt_MOZ_FLAG_STATUSBARON = 128U,
  Pt_MOZ_FLAG_PERSONALTOOLBARON = 256U,
  Pt_MOZ_FLAG_SCROLLBARSON = 512U,
  Pt_MOZ_FLAG_TITLEBARON = 1024U,
  Pt_MOZ_FLAG_EXTRACHROMEON = 2048U,
  Pt_MOZ_FLAG_ALLCHROME = 4094U,
  Pt_MOZ_FLAG_WINDOWRAISED = 33554432U,
  Pt_MOZ_FLAG_WINDOWLOWERED = 67108864U,
  Pt_MOZ_FLAG_CENTERSCREEN = 134217728U,
  Pt_MOZ_FLAG_DEPENDENT = 268435456U,
  Pt_MOZ_FLAG_MODAL = 536870912U,
  Pt_MOZ_FLAG_OPENASDIALOG = 1073741824U,
  Pt_MOZ_FLAG_OPENASCHROME = 2147483648U
} PtMozillaWindowFlags;
typedef struct mozilla_new_window_t
{
	PtWidget_t 		*widget;
	unsigned int	window_flags;
	PhDim_t				window_size;
} PtMozillaNewWindowCb_t;

typedef struct mozilla_prompt_t
{
	char *title;
	char *text;
	char *dflt_resp;
	char response[128];
} PtMozillaPromptCb_t;

/* for PtMozillaNewAreaCb_t flags */
#define Pt_MOZ_NEW_AREA_SET_SIZE					0x1
#define Pt_MOZ_NEW_AREA_SET_POSITION			0x2
#define Pt_MOZ_NEW_AREA_SET_AREA					0x4

typedef struct mozilla_new_area_t
{
	PhArea_t area;
	unsigned int flags;
} PtMozillaNewAreaCb_t;

typedef struct mozilla_visibility_t
{
	int show;
} PtMozillaVisibilityCb_t;

// context menu
#define Pt_MOZ_CONTEXT_NONE		0x0
#define Pt_MOZ_CONTEXT_LINK		0x1
#define Pt_MOZ_CONTEXT_IMAGE	0x2
#define Pt_MOZ_CONTEXT_DOCUMENT	0x4	
#define Pt_MOZ_CONTEXT_TEXT		0x8
#define Pt_MOZ_CONTEXT_INPUT	0x10
typedef struct mozilla_context_t
{
	unsigned flags;
	PRInt32 x, y;
} PtMozillaContextCb_t;

typedef struct mozilla_data_request_cb_t {
	int type;
	int length;
	char *url;
} PtMozillaDataRequestCb_t;

typedef struct {
	short   response;
	char    filename[PATH_MAX];
	char    url[MAX_URL_LENGTH];
	} PtMozUnknownResp_t;

typedef struct mozilla_event_t
{
	int flag;
} PtMozillaEvent_t;

// print status
#define Pt_MOZ_PRINT_START		1
#define Pt_MOZ_PRINT_COMPLETE	2
#define Pt_MOZ_PRINT_PROGRESS	3
typedef struct mozilla_print_status_t
{
	int status;
	unsigned int cur;
	unsigned int max;
} PtMozillaPrintStatusCb_t;

// convienience functions
#define Pt_MOZ_PREF_BOOL	1
#define Pt_MOZ_PREF_CHAR	2
#define Pt_MOZ_PREF_INT		3
#define Pt_MOZ_PREF_COLOR	4
void MozSetPreference(PtWidget_t *widget, int type, char *pref, void *data);

// mozilla commands
enum
{
	Pt_MOZ_COMMAND_CUT,
	Pt_MOZ_COMMAND_COPY,
	Pt_MOZ_COMMAND_PASTE,
	Pt_MOZ_COMMAND_SELECTALL,
	Pt_MOZ_COMMAND_CLEAR,
	Pt_MOZ_COMMAND_FIND,
	Pt_MOZ_COMMAND_SAVEAS
};

typedef struct {
	short   response;
	char    user[255];
	char    pass[255];
	PtModalCtrl_t ctrl;
	} PtMozillaAuthCtrl_t;

typedef struct {
	short response, waiting;
	char *filename;
	long download_ticket;
	PtModalCtrl_t ctrl;
	} PtMozillaUnknownCtrl_t;

class EmbedPrivate;

typedef struct Pt_mozilla_client_widget 
{
	PtContainerWidget_t	container;

	// Mozilla interfaces
	EmbedPrivate *EmbedRef;

	char				url[MAX_URL_LENGTH];
	int 				navigate_flags;
	char 				disable_exception_dlg, disable_new_windows, toActivate, spare[1];

	int fDownloadCount;
	EmbedDownload **fDownload;

	/* text_zoom is the text zooming as set by the client ( 100 = 100% ) */
	short int		text_zoom;
	/* actual_text_zoom is the text zooming as returned by the browser - when a window is being created
		the client tries to set the text zooming, but there is no content loaded. Use actual_text_zoom
		to remember to set the zooming when the content is loaded */
	short int actual_text_zoom;

	char *rightClickUrl_image; /* keep the url the user clicked on, to provide it latter for Pt_ARG_WEB_GET_CONTEXT */
	char *rightClickUrl_link; /* keep the url the user clicked on, to provide it latter for Pt_ARG_WEB_GET_CONTEXT */

	PtMozillaAuthCtrl_t *moz_auth_ctrl;
	PtMozillaUnknownCtrl_t *moz_unknown_ctrl;

	// callbacks
	PtCallbackList_t 	*title_cb;
	PtCallbackList_t 	*net_state_cb;
	PtCallbackList_t 	*info_cb;
	PtCallbackList_t 	*progress_cb;
	PtCallbackList_t 	*start_cb;
	PtCallbackList_t 	*complete_cb;
	PtCallbackList_t 	*url_cb;
	PtCallbackList_t 	*event_cb;
	PtCallbackList_t 	*resize_cb;
	PtCallbackList_t 	*new_window_cb;
	PtCallbackList_t 	*destroy_cb;
	PtCallbackList_t 	*visibility_cb; /* obsolete - delete it latter */
	PtCallbackList_t 	*open_cb;
	PtCallbackList_t	*dialog_cb;
	PtCallbackList_t 	*auth_cb;
	PtCallbackList_t 	*prompt_cb;
	PtCallbackList_t 	*context_cb;
	PtCallbackList_t 	*print_status_cb;
	PtCallbackList_t 	*web_data_req_cb;
	PtCallbackList_t 	*web_unknown_cb;
	PtCallbackList_t 	*web_error_cb;
	PtCallbackList_t 	*web_download_cb;
} PtMozillaWidget_t;

/* Widget union */
typedef union Pt_mozilla_union 
{
	PtWidget_t				core;
	PtBasicWidget_t			basic;
	PtContainerWidget_t		cntnr;
	PtMozillaWidget_t		moz;
} PtMozillaUnion_t;


#endif
