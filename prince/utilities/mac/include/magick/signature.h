/*
  Copyright 1999-2006 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore digital signature methods.
*/
#ifndef _MAGICKCORE_SIGNATURE_H
#define _MAGICKCORE_SIGNATURE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickSignatureSize  64

typedef struct _SignatureInfo
{
  unsigned long
    digest[8],
    low_order,
    high_order;

  long
    offset;

  unsigned char
    message[MagickSignatureSize];

  MagickBooleanType
    lsb_first;

  unsigned long
    signature;
} SignatureInfo;

extern MagickExport MagickBooleanType
  SignatureImage(Image *);

extern MagickExport void
  FinalizeSignature(SignatureInfo *),
  GetSignatureInfo(SignatureInfo *),
  UpdateSignature(SignatureInfo *,const unsigned char *,const size_t);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
