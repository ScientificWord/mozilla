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
#include "mimethtm.h"
#include "prmem.h"
#include "plstr.h"
#include "prlog.h"
#include "prprf.h"
#include "msgCore.h"
#include "nsMimeStringResources.h"
#include "mimemoz2.h"

#define MIME_SUPERCLASS mimeInlineTextClass
MimeDefClass(MimeInlineTextHTML, MimeInlineTextHTMLClass,
			 mimeInlineTextHTMLClass, &MIME_SUPERCLASS);

static int MimeInlineTextHTML_parse_line (char *, PRInt32, MimeObject *);
static int MimeInlineTextHTML_parse_eof (MimeObject *, PRBool);
static int MimeInlineTextHTML_parse_begin (MimeObject *obj);

static int
MimeInlineTextHTMLClassInitialize(MimeInlineTextHTMLClass *clazz)
{
  MimeObjectClass *oclass = (MimeObjectClass *) clazz;
  PR_ASSERT(!oclass->class_initialized);
  oclass->parse_begin = MimeInlineTextHTML_parse_begin;
  oclass->parse_line  = MimeInlineTextHTML_parse_line;
  oclass->parse_eof   = MimeInlineTextHTML_parse_eof;

  return 0;
}

static int
MimeInlineTextHTML_parse_begin (MimeObject *obj)
{
  int status = ((MimeObjectClass*)&mimeLeafClass)->parse_begin(obj);
  if (status < 0) return status;
  
  if (!obj->output_p) return 0;

  // Set a default font (otherwise unicode font will be used since the data is UTF-8).
  if (nsMimeOutput::nsMimeMessageBodyDisplay == obj->options->format_out ||
      nsMimeOutput::nsMimeMessagePrintOutput == obj->options->format_out)
  {
    char buf[256];            // local buffer for html tag
    PRInt32 fontSize;         // default font size
    PRInt32 fontSizePercentage;   // size percentage
    nsCAutoString fontLang;       // langgroup of the font. 
    if (NS_SUCCEEDED(GetMailNewsFont(obj, PR_FALSE, &fontSize, &fontSizePercentage,fontLang)))
    {
      PR_snprintf(buf, 256, "<div class=\"moz-text-html\"  lang=\"%s\">", 
                  fontLang.get());
      status = MimeObject_write(obj, buf, strlen(buf), PR_FALSE);
    }
    else
    {
      status = MimeObject_write(obj, "<div class=\"moz-text-html\">", 27, PR_FALSE);
    }
    if(status<0) return status;
  }

  MimeInlineTextHTML  *textHTML = (MimeInlineTextHTML *) obj;
  
  textHTML->charset = nsnull;
  
  /* If this HTML part has a Content-Base header, and if we're displaying
	 to the screen (that is, not writing this part "raw") then translate
   that Content-Base header into a <BASE> tag in the HTML.
  */
  if (obj->options &&
    obj->options->write_html_p &&
    obj->options->output_fn)
  {
    char *base_hdr = MimeHeaders_get (obj->headers, HEADER_CONTENT_BASE,
      PR_FALSE, PR_FALSE);
    
    /* rhp - for MHTML Spec changes!!! */
    if (!base_hdr)
    {
      base_hdr = MimeHeaders_get (obj->headers, HEADER_CONTENT_LOCATION, PR_FALSE, PR_FALSE);
    }
    /* rhp - for MHTML Spec changes!!! */
    
    if (base_hdr)
    {
      char *buf = (char *) PR_MALLOC(strlen(base_hdr) + 20);
      const char *in;
      char *out;
      if (!buf)
        return MIME_OUT_OF_MEMORY;
      
        /* The value of the Content-Base header is a number of "words".
        Whitespace in this header is not significant -- it is assumed
        that any real whitespace in the URL has already been encoded,
        and whitespace has been inserted to allow the lines in the
        mail header to be wrapped reasonably.  Creators are supposed
        to insert whitespace every 40 characters or less.
      */
      PL_strcpy(buf, "<BASE HREF=\"");
      out = buf + strlen(buf);
      
      for (in = base_hdr; *in; in++)
        /* ignore whitespace and quotes */
        if (!nsCRT::IsAsciiSpace(*in) && *in != '"')
          *out++ = *in;
        
        /* Close the tag and argument. */
        *out++ = '"';
        *out++ = '>';
        *out++ = 0;
        
        PR_Free(base_hdr);
        
        status = MimeObject_write(obj, buf, strlen(buf), PR_FALSE);
        PR_Free(buf);
        if (status < 0) return status;
    }
  }
  
  // rhp: For a change, we will write out a separator after formatted text
  //      bodies.
  status = MimeObject_write_separator(obj);
  if (status < 0) return status;
  
  return 0;
}


static int
MimeInlineTextHTML_parse_line (char *line, PRInt32 length, MimeObject *obj)
{
  MimeInlineTextHTML  *textHTML = (MimeInlineTextHTML *) obj;

  if (!obj->output_p) 
    return 0;

  if (!obj->options || !obj->options->output_fn)
    return 0;

  if (!textHTML->charset)
  {
    char * cp;
    // First, try to detect a charset via a META tag!
    if ((cp = PL_strncasestr(line, "META", length)) && 
        (cp = PL_strncasestr(cp, "HTTP-EQUIV=", length - (int)(cp - line))) && 
        (cp = PL_strncasestr(cp, "CONTENT=", length - (int)(cp - line))) && 
        (cp = PL_strncasestr(cp, "CHARSET=", length - (int)(cp - line))) 
        ) 
    {
      char* cp1 = cp + 8;  //8 for the length of "CHARSET="
      char* cp2 = PL_strnpbrk(cp1, " \"\'", length - (int)(cp1 - line));
      if (cp2)
      {
        char* charset = PL_strndup(cp1, (int)(cp2 - cp1));
 
        // Fix bug 101434, in this case since this parsing is a char* 
        // operation, a real UTF-16 or UTF-32 document won't be parse 
        // correctly, if it got parse, it cannot be UTF-16 nor UTF-32
        // there fore, we ignore them if somehow we got that value
        // 6 == strlen("UTF-16") or strlen("UTF-32"), this will cover
        // UTF-16, UTF-16BE, UTF-16LE, UTF-32, UTF-32BE, UTF-32LE
        if ((charset != nsnull) &&
            nsCRT::strncasecmp(charset, "UTF-16", 6) &&
            nsCRT::strncasecmp(charset, "UTF-32", 6))
        {
          textHTML->charset = charset; 

          // write out the data without the charset part...
          if (textHTML->charset)
          {
            int err = MimeObject_write(obj, line, cp - line, PR_TRUE);
            if (err == 0)
              err = MimeObject_write(obj, cp2, length - (int)(cp2 - line), PR_TRUE);

            return err;
          }
        }
        PR_FREEIF(charset);
      }
    }
  }

  // Now, just write out the data...
  return MimeObject_write(obj, line, length, PR_TRUE);
}

static int
MimeInlineTextHTML_parse_eof (MimeObject *obj, PRBool abort_p)
{
  int status;
  MimeInlineTextHTML  *textHTML = (MimeInlineTextHTML *) obj;
  if (obj->closed_p) return 0;

  PR_FREEIF(textHTML->charset);

  /* Run parent method first, to flush out any buffered data. */
  status = ((MimeObjectClass*)&MIME_SUPERCLASS)->parse_eof(obj, abort_p);
  if (status < 0) return status;

  if (nsMimeOutput::nsMimeMessageBodyDisplay == obj->options->format_out ||
      nsMimeOutput::nsMimeMessagePrintOutput == obj->options->format_out)
    status = MimeObject_write(obj, "</div>", 6, PR_FALSE);

  return 0;
}
