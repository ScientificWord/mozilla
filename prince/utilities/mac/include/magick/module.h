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

  MagickCore module methods.
*/
#ifndef _MAGICKCORE_MODULE_H
#define _MAGICKCORE_MODULE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <time.h>

typedef enum
{
  MagickCoderModule,
  MagickFilterModule
} MagickModuleType;

typedef struct _ModuleInfo
{
  char
    *path,
    *tag;

  void
    *handle,
    (*register_module)(void),
    (*unregister_module)(void);

  time_t
    load_time;

  MagickBooleanType
    stealth;

  struct _ModuleInfo
    *previous,
    *next;  /* deprecated, use GetModuleInfoList() */

  unsigned long
    signature;
} ModuleInfo;

extern MagickExport char
  **GetModuleList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport const ModuleInfo
  *GetModuleInfo(const char *,ExceptionInfo *),
  **GetModuleInfoList(const char *,unsigned long *,ExceptionInfo *);

extern MagickExport MagickBooleanType
  ExecuteModuleProcess(const char *,Image **,const int,char **),
  ExecuteStaticModuleProcess(const char *,Image **,const int,char **),
  ListModuleInfo(FILE *,ExceptionInfo *),
  OpenModule(const char *,ExceptionInfo *),
  OpenModules(ExceptionInfo *);

extern MagickExport void
  DestroyModuleList(void),
  RegisterStaticModules(void),
  UnregisterStaticModules(void);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
