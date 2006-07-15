/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Pierre Phaneuf <pp@ludusdesign.com>
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

// Utility that converts file encoded in one charset codepage to
// another encoding

#include "nscore.h"
#include "nsString.h"
#include "nsIServiceManager.h"
#include "nsICharsetConverterManager.h"
#include "nsIUnicodeEncoder.h"
#include "nsIUnicodeDecoder.h"

#include "nsICharsetAlias.h"

static NS_DEFINE_CID(kCharsetConverterManagerCID, NS_ICHARSETCONVERTERMANAGER_CID);

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void usage()
{
  printf(
    "nsconv -f fromcode -t tocode infile outfile\n"
    "nsconv -f fromcode -t tocode infile > outfile\n"
    "nsconv -f fromcode -t tocode < infile > outfile\n"
    );
}

#define INBUFSIZE (1024*16)
#define MEDBUFSIZE (1024*16*2)
#define OUTBUFSIZE (1024*16*8)
char inbuffer[INBUFSIZE];
char outbuffer[OUTBUFSIZE];
PRUnichar  medbuffer[MEDBUFSIZE];

int main(int argc, const char** argv)
{
  nsIUnicodeEncoder* encoder = nsnull;
  nsIUnicodeDecoder* decoder = nsnull;
  FILE* fin = 0;
  FILE* fout = 0;
  FILE* infile = 0;
  FILE* outfile = 0;
  nsresult res= NS_OK;

  NS_InitXPCOM2(nsnull, nsnull, nsnull);

  // get ccMain;
  nsCOMPtr<nsICharsetConverterManager> ccMain =
      do_GetService(kCharsetConverterManagerCID, &res);
  if(NS_FAILED(res))
  {
    fprintf(stderr, "Cannot get Character Converter Manager %x\n", res);
    return -1;
  }

  // Get the charset alias manager
  nsCOMPtr<nsICharsetAlias> aliasmgr =
      do_GetService(NS_CHARSETALIAS_CONTRACTID, &res);
  if (NS_FAILED(res))
  {
    fprintf(stderr, "Cannot get Charset Alias Manager %x\n", res);
    return -1;
  }

  int i;
  if(argc > 4)
  {
    for(i =0; i < argc; i++)
    {
      if(strcmp(argv[i], "-f") == 0)
      {
        // User has specified the charset to convert from
        nsCAutoString str;

        // First check if a charset alias was given, 
        // and convert to the canonical name
        res = aliasmgr->GetPreferred(nsDependentCString(argv[i+1]), str);
        if (NS_FAILED(res))
        {
          fprintf(stderr, "Cannot get charset alias for %s %x\n",
                  argv[i+1], res);
          goto error_exit;
        }

        // Finally create the decoder
        res = ccMain->GetUnicodeDecoder(str.get(), &decoder);
        if(NS_FAILED(res)) {
          fprintf(stderr, "Cannot get Unicode decoder %s %x\n", 
                  argv[i+1],res);
          goto error_exit;
        }

      }

      if(strcmp(argv[i], "-t") == 0)
      {
        // User has specified which charset to convert to
        nsCAutoString str;

        // First check if a charset alias was given, 
        // and convert to the canonical name
        res = aliasmgr->GetPreferred(nsDependentCString(argv[i+1]), str);
        if (NS_FAILED(res))
        {
          fprintf(stderr, "Cannot get charset alias for %s %x\n",
                  argv[i+1], res);
          goto error_exit;
        }

        // Finally create the encoder 
        res = ccMain->GetUnicodeEncoderRaw(str.get(), &encoder);
        if(NS_FAILED(res)) {
          fprintf(stderr, "Cannot get Unicode encoder %s %x\n", 
                  argv[i+1],res);
          goto error_exit;
        }
      }
    }

    if (argc > 5)
    {
      // The user has specified an input file 
      // if we have more than four arguments
      fin = infile = fopen(argv[5], "rb");
      if(NULL == infile) 
      {  
        usage();
        fprintf(stderr,"cannot open input file %s\n", argv[5]);
        goto error_exit; 
      }

      if (argc > 6)
      {
        // The user has specified an output file
        // if we have more than four arguments
        fout = outfile = fopen(argv[6], "ab");
        if(NULL == outfile) 
        {  
          usage();
          fprintf(stderr,"cannot open output file %s\n", argv[6]);
          goto error_exit; 
        }
      }
      else
        fout = stdout;
    }
    else
    {
      // No inputfiles are given. Read and write
      // to/from standard in and standard out
      fin = stdin;
      fout = stdout;
    }
    
    PRInt32 insize,medsize,outsize;
    while((insize=fread(inbuffer, 1,INBUFSIZE, fin)) > 0)
    {
      medsize=MEDBUFSIZE;
        
      res = decoder->Convert(inbuffer,&insize, medbuffer, &medsize);
      if(NS_FAILED(res)) {
        fprintf(stderr, "failed in decoder->Convert %x\n",res);
        goto error_exit;
      }
      outsize = OUTBUFSIZE;
      res = encoder->Convert(medbuffer, &medsize, outbuffer,&outsize);
      if(NS_FAILED(res)) {
        fprintf(stderr, "failed in encoder->Convert %x\n",res);
        goto error_exit;
      }
      fwrite(outbuffer, 1, outsize, fout);

    }
     
    // Clean up 
    if (infile != 0)
      fclose(infile);
    if (outfile != 0)
      fclose(outfile);
    fprintf(stderr, "Done!\n");
    NS_IF_RELEASE(encoder);
    NS_IF_RELEASE(decoder);
    return 0;
  }
  usage();
  error_exit:
  // Clean up after
  if (infile != 0)
    fclose(infile);
  if (outfile != 0)
    fclose(outfile);
  NS_IF_RELEASE(encoder);
  NS_IF_RELEASE(decoder);
  return -1;
}
