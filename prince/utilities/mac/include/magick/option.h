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

  MagickCore option methods.
*/
#ifndef _MAGICKCORE_OPTION_H
#define _MAGICKCORE_OPTION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  /* Options that are listed/parsed from tables in "options.c" */
  MagickUndefinedOptions = -1,
  MagickAlignOptions = 0,
  MagickBooleanOptions,
  MagickChannelOptions,
  MagickClassOptions,
  MagickClipPathOptions,
  MagickColorspaceOptions,
  MagickCompositeOptions,
  MagickCommandOptions,
  MagickCompressionOptions,
  MagickDataTypeOptions,
  MagickDecorationOptions,
  MagickDisposeOptions,
  MagickEndianOptions,
  MagickEvaluateOptions,
  MagickFillRuleOptions,
  MagickFilterOptions,
  MagickGravityOptions,
  MagickImageOptions,
  MagickIntentOptions,
  MagickInterlaceOptions,
  MagickInterpolateOptions,
  MagickLayersOptions,
  MagickLineCapOptions,
  MagickLineJoinOptions,
  MagickListOptions,
  MagickLogEventOptions,
  MagickMetricOptions,
  MagickMethodOptions,
  MagickModeOptions,
  MagickMogrifyOptions,
  MagickNoiseOptions,
  MagickOrientationOptions,
  MagickPreviewOptions,
  MagickPrimitiveOptions,
  MagickQuantumFormatOptions,
  MagickResolutionOptions,
  MagickResourceOptions,
  MagickStorageOptions,
  MagickStretchOptions,
  MagickStyleOptions,
  MagickVirtualPixelOptions,
  /* Special Option Lists Indexed to a Specific List Function for that Option */
  MagickCoderOptions,
  MagickColorOptions,
  MagickConfigureOptions,
  MagickDelegateOptions,
  MagickFormatOptions,
  MagickMimeOptions,
  MagickLocaleOptions,
  MagickLogOptions,
  MagickMagicOptions,
  MagickModuleOptions,
  /* MagickResourceOptions, -- duplicate Function context dependant */
  MagickThresholdOptions,
  MagickTypeOptions
} MagickOption;

typedef struct _OptionInfo
{
  const char
    *mnemonic;

  long
    type;
} OptionInfo;

extern MagickExport char
  **GetMagickOptions(const MagickOption),
  *GetNextImageOption(const ImageInfo *);

extern MagickExport const char
  *GetImageOption(const ImageInfo *,const char *),
  *MagickOptionToMnemonic(const MagickOption,const long);

extern MagickExport long
  ParseChannelOption(const char *),
  ParseMagickOption(const MagickOption,const MagickBooleanType,const char *);

extern MagickExport MagickBooleanType
  CloneImageOptions(ImageInfo *,const ImageInfo *),
  DefineImageOption(ImageInfo *,const char *),
  DeleteImageOption(ImageInfo *,const char *),
  IsMagickOption(const char *),
  ListMagickOptionInfo(FILE *, const MagickOption, ExceptionInfo *),
  RemoveImageOption(ImageInfo *,const char *),
  SetImageOption(ImageInfo *,const char *,const char *),
  SyncImageOptions(const ImageInfo *,Image *);

extern MagickExport void
  DestroyImageOptions(ImageInfo *),
  ResetImageOptionIterator(const ImageInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
