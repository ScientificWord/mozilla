/*
  Copyright 1999-2005 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Pixel Iterator Methods.
*/
#ifndef _MAGICKWAND_PIXEL_ITERATOR_H
#define _MAGICKWAND_PIXEL_ITERATOR_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include "wand/magick-wand.h"
#include "wand/pixel-wand.h"

typedef struct _PixelIterator
  PixelIterator;

extern WandExport char
  *PixelGetIteratorException(const PixelIterator *,ExceptionType *);

extern WandExport long
  PixelGetIteratorRow(PixelIterator *);

extern WandExport MagickBooleanType
  IsPixelIterator(const PixelIterator *),
  PixelClearIteratorException(PixelIterator *),
  PixelSetIteratorRow(PixelIterator *,const long),
  PixelSyncIterator(PixelIterator *);

extern WandExport PixelIterator
  *DestroyPixelIterator(PixelIterator *),
  *NewPixelIterator(MagickWand *),
  *NewPixelRegionIterator(MagickWand *,const long,const long,
    const unsigned long,const unsigned long);

extern WandExport PixelWand
  **PixelGetCurrentIteratorRow(PixelIterator *,unsigned long *),
  **PixelGetNextIteratorRow(PixelIterator *,unsigned long *),
  **PixelGetPreviousIteratorRow(PixelIterator *,unsigned long *);

extern WandExport void
  ClearPixelIterator(PixelIterator *),
  PixelResetIterator(PixelIterator *),
  PixelSetFirstIteratorRow(PixelIterator *),
  PixelSetLastIteratorRow(PixelIterator *);

/*
  Deprecated.
*/
extern WandExport char
  *PixelIteratorGetException(const PixelIterator *,ExceptionType *);

extern WandExport PixelWand
  **PixelGetNextRow(PixelIterator *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
