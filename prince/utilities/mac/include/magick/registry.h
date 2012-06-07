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

  MagickCore registry methods.
*/
#ifndef _MAGICKCORE_REGISTRY_H
#define _MAGICKCORE_REGISTRY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedRegistryType,
  ImageRegistryType,
  ImageInfoRegistryType
} RegistryType;

extern MagickExport Image
  *GetImageFromMagickRegistry(const char *,long *id,ExceptionInfo *);

extern MagickExport long
  SetMagickRegistry(const RegistryType,const void *,const size_t,
    ExceptionInfo *);

extern MagickExport MagickBooleanType
  DeleteMagickRegistry(const long);

extern MagickExport void
  DestroyMagickRegistry(void),
  *GetMagickRegistry(const long,RegistryType *,size_t *,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
