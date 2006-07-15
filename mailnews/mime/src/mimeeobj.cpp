/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

#include "nsCOMPtr.h"
#include "mimeeobj.h"
#include "prmem.h"
#include "plstr.h"
#include "prlog.h"
#include "nsMimeStringResources.h"
#include "mimemoz2.h"
#include "nsCRT.h"
#include "mimemapl.h"
#include "nsMimeTypes.h"


#define MIME_SUPERCLASS mimeLeafClass
MimeDefClass(MimeExternalObject, MimeExternalObjectClass,
			 mimeExternalObjectClass, &MIME_SUPERCLASS);

static int MimeExternalObject_initialize (MimeObject *);
static void MimeExternalObject_finalize (MimeObject *);
static int MimeExternalObject_parse_begin (MimeObject *);
static int MimeExternalObject_parse_buffer (const char *, PRInt32, MimeObject *);
static int MimeExternalObject_parse_line (char *, PRInt32, MimeObject *);
static int MimeExternalObject_parse_decoded_buffer (const char*, PRInt32, MimeObject*);
static PRBool MimeExternalObject_displayable_inline_p (MimeObjectClass *clazz,
														MimeHeaders *hdrs);

static int
MimeExternalObjectClassInitialize(MimeExternalObjectClass *clazz)
{
  MimeObjectClass *oclass = (MimeObjectClass *) clazz;
  MimeLeafClass   *lclass = (MimeLeafClass *) clazz;

  NS_ASSERTION(!oclass->class_initialized, "1.1 <rhp@netscape.com> 19 Mar 1999 12:00");
  oclass->initialize   = MimeExternalObject_initialize;
  oclass->finalize     = MimeExternalObject_finalize;
  oclass->parse_begin  = MimeExternalObject_parse_begin;
  oclass->parse_buffer = MimeExternalObject_parse_buffer;
  oclass->parse_line   = MimeExternalObject_parse_line;
  oclass->displayable_inline_p = MimeExternalObject_displayable_inline_p;
  lclass->parse_decoded_buffer = MimeExternalObject_parse_decoded_buffer;
  return 0;
}


static int
MimeExternalObject_initialize (MimeObject *object)
{
  return ((MimeObjectClass*)&MIME_SUPERCLASS)->initialize(object);
}

static void
MimeExternalObject_finalize (MimeObject *object)
{
  ((MimeObjectClass*)&MIME_SUPERCLASS)->finalize(object);
}


static int
MimeExternalObject_parse_begin (MimeObject *obj)
{
  int status;
  
  status = ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_begin(obj);
  if (status < 0) return status;
  
  // If we're writing this object, and we're doing it in raw form, then
  // now is the time to inform the backend what the type of this data is.
  //
  if (obj->output_p &&
    obj->options &&
    !obj->options->write_html_p &&
    !obj->options->state->first_data_written_p)
  {
    status = MimeObject_output_init(obj, 0);
    if (status < 0) return status;
    NS_ASSERTION(obj->options->state->first_data_written_p, "1.1 <rhp@netscape.com> 19 Mar 1999 12:00");
  }
    
  //
  // If we're writing this object as HTML, do all the work now -- just write
  // out a table with a link in it.  (Later calls to the `parse_buffer' method
  // will simply discard the data of the object itself.)
  //
  if (obj->options &&
      obj->output_p &&
      obj->options->write_html_p &&
      obj->options->output_fn)
  {
    MimeDisplayOptions newopt = *obj->options;  // copy it 
    char *id = 0;
    char *id_url = 0;
    char *id_name = 0;
    nsXPIDLCString id_imap;
    PRBool all_headers_p = obj->options->headers == MimeHeadersAll;
    
    id = mime_part_address (obj);
    if (obj->options->missing_parts)
      id_imap.Adopt(mime_imap_part_address (obj));
    if (! id) return MIME_OUT_OF_MEMORY;
    
    if (obj->options && obj->options->url)
    {
      const char *url = obj->options->url;
      if (id_imap && id)
      {
        // if this is an IMAP part. 
        id_url = mime_set_url_imap_part(url, id_imap.get(), id);
      }
      else
      {
        // This is just a normal MIME part as usual. 
        id_url = mime_set_url_part(url, id, PR_TRUE);
      }
      if (!id_url)
      {
        PR_Free(id);
        return MIME_OUT_OF_MEMORY;
      }
    }
    if (!nsCRT::strcmp (id, "0"))
    {
      PR_Free(id);
      id = MimeGetStringByID(MIME_MSG_ATTACHMENT);
    }
    else
    {
      const char *p = "Part ";  
      char *s = (char *)PR_MALLOC(strlen(p) + strlen(id) + 1);
      if (!s)
      {
        PR_Free(id);
        PR_Free(id_url);
        return MIME_OUT_OF_MEMORY;
      }
      // we have a valid id
      if (id)
        id_name = mime_find_suggested_name_of_part(id, obj);
      PL_strcpy(s, p);
      PL_strcat(s, id);
      PR_Free(id);
      id = s;
    }
    
    if (all_headers_p &&
    // Don't bother showing all headers on this part if it's the only
    // part in the message: in that case, we've already shown these
    // headers.
    obj->options->state &&
    obj->options->state->root == obj->parent)
    all_headers_p = PR_FALSE;
    
    newopt.fancy_headers_p = PR_TRUE;
    newopt.headers = (all_headers_p ? MimeHeadersAll : MimeHeadersSome);    

/******    
RICHIE SHERRY
GOTTA STILL DO THIS FOR QUOTING!
     status = MimeHeaders_write_attachment_box (obj->headers, &newopt,
                                                 obj->content_type,
                                                 obj->encoding,
                                                 id_name? id_name : id, id_url, 0)
*****/

    // obj->options really owns the storage for this.
    newopt.part_to_load = nsnull;
    newopt.default_charset = nsnull;
    PR_FREEIF(id);
    PR_FREEIF(id_url);
    PR_FREEIF(id_name);
    if (status < 0) return status;
  }
  
  return 0;
}

static int
MimeExternalObject_parse_buffer (const char *buffer, PRInt32 size, MimeObject *obj)
{
  NS_ASSERTION(!obj->closed_p, "1.1 <rhp@netscape.com> 19 Mar 1999 12:00");
  if (obj->closed_p) return -1;

  if (obj->output_p &&
	  obj->options &&
	  !obj->options->write_html_p)
	{
	  /* The data will be base64-decoded and passed to
		 MimeExternalObject_parse_decoded_buffer. */
	  return ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_buffer(buffer, size,
																obj);
	}
  else
	{
	  /* Otherwise, simply ignore the data. */
	  return 0;
	}
}


static int
MimeExternalObject_parse_decoded_buffer (const char *buf, PRInt32 size,
										 MimeObject *obj)
{
  /* This is called (by MimeLeafClass->parse_buffer) with blocks of data
	 that have already been base64-decoded.  This will only be called in
	 the case where we're not emitting HTML, and want access to the raw
	 data itself.

	 We override the `parse_decoded_buffer' method provided by MimeLeaf
	 because, unlike most children of MimeLeaf, we do not want to line-
	 buffer the decoded data -- we want to simply pass it along to the
	 backend, without going through our `parse_line' method.
   */
  if (!obj->output_p ||
	  !obj->options ||
	  obj->options->write_html_p)
	{
	  NS_ASSERTION(0, "1.1 <rhp@netscape.com> 19 Mar 1999 12:00");
	  return -1;
	}

  return MimeObject_write(obj, buf, size, PR_TRUE);
}


static int
MimeExternalObject_parse_line (char *line, PRInt32 length, MimeObject *obj)
{
  /* This method should never be called (externals do no line buffering).
   */
  NS_ASSERTION(0, "1.1 <rhp@netscape.com> 19 Mar 1999 12:00");
  return -1;
}

static PRBool
MimeExternalObject_displayable_inline_p (MimeObjectClass *clazz,
										 MimeHeaders *hdrs)
{
  return PR_FALSE;
}
