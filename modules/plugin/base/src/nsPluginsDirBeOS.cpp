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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Alex Musil
 *   Makoto Hamanaka <VYA04230@nifty.com>
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

/*
	nsPluginsDirBeOS.cpp
	
	BeOS implementation of the nsPluginsDir/nsPluginsFile classes.
	
	Based on nsPluginsDirUNIX.cpp r1.12 by Alex Musil
 */

#include "nsPluginsDir.h"
#include "prlink.h"
#include "plstr.h"
#include "prmem.h"
#include "nsReadableUtils.h"
#include "nsString.h"

#include <File.h>
#include <AppFileInfo.h>
#include <Message.h>
#include <String.h>

//#define NS_PLUGIN_BEOS_DEBUG

/* Local helper functions */
 
static char* GetFileName(const char* pathname)
{
        const char* filename = nsnull;
                
        // this is most likely a path, so skip to the filename
        filename = PL_strrchr(pathname, '/');
        if(filename)
                ++filename;
        else
                filename = pathname;

        return PL_strdup(filename);
}

static nsresult GetMimeExtensions(const char *mimeType, char *extensions, int extLen)
{
    // check variables
    if (!mimeType || !extensions || extLen < 1) return NS_ERROR_FAILURE;
    extensions[0] = '\0';
    
    // make mime object
    BMimeType mime(mimeType) ;
    if (mime.InitCheck() != B_OK)
        return NS_ERROR_FAILURE;
    
    // get extensions : comma separated (if multiple extensions in a mime-type)
    // ex) "jpg,jpeg"
    BString extStr("");
    BMessage extMsg;
    mime.GetFileExtensions(&extMsg);
    uint32 type;
    int32 types_num;
    if (extMsg.GetInfo("extensions", &type, &types_num) != B_OK
        || type != B_STRING_TYPE || types_num == 0)
        return NS_ERROR_FAILURE;
    
    for (int i = 0 ; i < types_num ; i ++) {
        const char *ext;
        if (extMsg.FindString("extensions", i, &ext) != B_OK) {
            break;
        }
        if (i > 0)
            extStr.Append(",");
        extStr.Append(ext);
    }
    PL_strncpyz(extensions, extStr.String(), extLen) ;
    
    return NS_OK;
}

///////////////////////////////////////////////////////////////////////////

/* nsPluginsDir implementation */

PRBool nsPluginsDir::IsPluginFile(nsIFile* file)
{
	return PR_TRUE;
}

///////////////////////////////////////////////////////////////////////////

/* nsPluginFile implementation */

nsPluginFile::nsPluginFile(nsIFile* spec)
:	mPlugin(spec)
{
	// nada
}

nsPluginFile::~nsPluginFile()
{
	// nada
}

/**
 * Loads the plugin into memory using NSPR's shared-library loading
 * mechanism. Handles platform differences in loading shared libraries.
 */
nsresult nsPluginFile::LoadPlugin(PRLibrary* &outLibrary)
{
        nsCAutoString path;
        nsresult rv = mPlugin->GetNativePath(path);
        if (NS_OK != rv) {
            return rv;
        }
        pLibrary = outLibrary = PR_LoadLibrary(path.get());

#ifdef NS_DEBUG
        printf("LoadPlugin() %s returned %lx\n",path,(unsigned long)pLibrary);
#endif

        return NS_OK;
}

typedef char* (*BeOS_Plugin_GetMIMEDescription)();


/**
 * Obtains all of the information currently available for this plugin.
 */
nsresult nsPluginFile::GetPluginInfo(nsPluginInfo& info)
{
    nsCAutoString fpath;
    nsresult rv = mPlugin->GetNativePath(fpath);
    if (NS_OK != rv) {
        return rv;
    }
    const char *path = fpath.get();
    int i;

#ifdef NS_PLUGIN_BEOS_DEBUG
    printf("nsPluginFile::GetPluginInfo() an attempt to load MIME String\n");
    printf("path = <%s>\n", path);
#endif

    // get supported mime types
    BFile file(path, B_READ_ONLY);
    if (file.InitCheck() != B_OK)
        return NS_ERROR_FAILURE;

    BAppFileInfo appinfo(&file);
    if (appinfo.InitCheck() != B_OK)
        return NS_ERROR_FAILURE;

    BMessage msg;
    if (appinfo.GetSupportedTypes(&msg) != B_OK)
        return NS_ERROR_FAILURE;

    uint32 type;
    int32 types_num;
    if (msg.GetInfo("types", &type, &types_num) != B_OK
        || type != B_STRING_TYPE)
        return NS_ERROR_FAILURE;

    // set mime types to plugin info
    info.fMimeTypeArray =(char **)PR_Malloc(types_num * sizeof(char *));
    info.fMimeDescriptionArray =(char **)PR_Malloc(types_num * sizeof(char *));
    info.fExtensionArray =(char **)PR_Malloc(types_num * sizeof(char *));

    for (i = 0 ; i < types_num ; i ++) {
        // get mime string
        const char *mtype;
        if (msg.FindString("types", i, &mtype) != B_OK) {
            types_num = i;
            break;
        }
        
        // get (short)description for the mime
        char desc[B_MIME_TYPE_LENGTH+1] = "";
        BMimeType mime(mtype) ;
        if (mime.InitCheck() == B_OK)
            mime.GetShortDescription(desc);
        
        // get file extensions for the mime
        char extensions[B_MIME_TYPE_LENGTH+1] = "";
        GetMimeExtensions(mtype, extensions, B_MIME_TYPE_LENGTH+1);

        #ifdef NS_PLUGIN_BEOS_DEBUG
            printf("  mime = %30s | %10s | %15s |\n", 
                mtype, extensions, desc);
        #endif
        
        info.fMimeTypeArray[i] = PL_strdup( mtype ? mtype : (char *)"" ) ;
        info.fMimeDescriptionArray[i] = PL_strdup( desc ) ;
        info.fExtensionArray[i] = PL_strdup( extensions );
    }

    // get name and description of this plugin
    version_info vinfo;
    if (appinfo.GetVersionInfo(&vinfo, B_APP_VERSION_KIND) == B_OK
        && *vinfo.short_info) {
        // XXX convert UTF-8 2byte chars to 1 byte chars, to avoid string corruption
        info.fName = ToNewCString(NS_ConvertUTF8toUTF16(vinfo.short_info));
        info.fDescription = ToNewCString(NS_ConvertUTF8toUTF16(vinfo.long_info));
    } else {
        // use filename as its name
        info.fName = GetFileName(path);
        info.fDescription = PL_strdup("");
    }

    info.fVariantCount = types_num;
    info.fFileName = PL_strdup(path);


#ifdef NS_PLUGIN_BEOS_DEBUG
    printf("info.fFileName = %s\n", info.fFileName);
    printf("info.fName = %s\n", info.fName);
    printf("info.fDescription = %s\n", info.fDescription);
#endif

    return NS_OK;
}

nsresult nsPluginFile::FreePluginInfo(nsPluginInfo& info)
{
    if (info.fName)
        PL_strfree(info.fName);

    if (info.fDescription)
        PL_strfree(info.fDescription);

    for (PRUint32 i = 0; i < info.fVariantCount; i++) {
        if (info.fMimeTypeArray[i])
            PL_strfree(info.fMimeTypeArray[i]);

        if (info.fMimeDescriptionArray[i])
            PL_strfree(info.fMimeDescriptionArray[i]);

        if (info.fExtensionArray[i])
            PL_strfree(info.fExtensionArray[i]);
    }

    PR_FREEIF(info.fMimeTypeArray);
    PR_FREEIF(info.fMimeDescriptionArray);
    PR_FREEIF(info.fExtensionArray);

    if (info.fFileName)
        PL_strfree(info.fFileName);

    return NS_OK;
}
