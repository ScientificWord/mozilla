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

  MagickCore exception methods.
*/
#ifndef _MAGICKCORE_EXCEPTION_H
#define _MAGICKCORE_EXCEPTION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <stdarg.h>

typedef void
  (*ErrorHandler)(const ExceptionType,const char *,const char *);

typedef void
  (*FatalErrorHandler)(const ExceptionType,const char *,const char *);

typedef void
  (*WarningHandler)(const ExceptionType,const char *,const char *);

extern MagickExport char
  *GetExceptionMessage(const int);

extern MagickExport const char
  *GetLocaleExceptionMessage(const ExceptionType,const char *);

extern MagickExport ErrorHandler
  SetErrorHandler(ErrorHandler);

extern MagickExport ExceptionInfo
  *AcquireExceptionInfo(void),
  *DestroyExceptionInfo(ExceptionInfo *);

extern MagickExport FatalErrorHandler
  SetFatalErrorHandler(FatalErrorHandler);

extern MagickExport MagickBooleanType
  ThrowException(ExceptionInfo *,const ExceptionType,const char *,
    const char *),
  ThrowMagickException(ExceptionInfo *,const char *,const char *,
    const unsigned long,const ExceptionType,const char *,const char *,...)
    magick_attribute((format (printf,7,8))),
  ThrowMagickExceptionList(ExceptionInfo *,const char *,const char *,
    const unsigned long,const ExceptionType,const char *,const char *,va_list)
    magick_attribute((format (printf,7,0)));

extern MagickExport void
  CatchException(ExceptionInfo *),
  ClearMagickException(ExceptionInfo *),
  GetExceptionInfo(ExceptionInfo *),
  InheritException(ExceptionInfo *,const ExceptionInfo *),
  MagickError(const ExceptionType,const char *,const char *),
  MagickFatalError(const ExceptionType,const char *,const char *),
  MagickWarning(const ExceptionType,const char *,const char *);

extern MagickExport WarningHandler
  SetWarningHandler(WarningHandler);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
