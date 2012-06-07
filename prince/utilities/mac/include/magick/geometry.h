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

  MagickCore image geometry methods.
*/
#ifndef _MAGICKCORE_GEOMETRY_H
#define _MAGICKCORE_GEOMETRY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
#undef NoValue
  NoValue = 0x0000,
#undef XValue
  PsiValue = 0x0001,
  XValue = 0x0001,
#undef YValue
  XiValue = 0x0002,
  YValue = 0x0002,
#undef WidthValue
  RhoValue = 0x0004,
  WidthValue = 0x0004,
#undef HeightValue
  SigmaValue = 0x0008,
  HeightValue = 0x0008,
  ChiValue = 0x0010,
  XiNegative = 0x0020,
#undef XNegative
  XNegative = 0x0020,
  PsiNegative = 0x0040,
#undef YNegative
  YNegative = 0x0040,
  ChiNegative = 0x0080,
  PercentValue = 0x1000,
  AspectValue = 0x2000,
  LessValue = 0x4000,
  GreaterValue = 0x8000,
  AreaValue = 0x10000,
  DecimalValue = 0x20000,
#undef AllValues
  AllValues = 0x7fffffff
} GeometryFlags;

typedef struct _GeometryInfo
{
  double
    rho,
    sigma,
    xi,
    psi,
    chi;
} GeometryInfo;

extern MagickExport char
  *GetPageGeometry(const char *);

extern MagickExport MagickBooleanType
  IsGeometry(const char *),
  IsSceneGeometry(const char *,const MagickBooleanType);

extern MagickExport MagickStatusType
  GetGeometry(const char *,long *,long *,unsigned long *,unsigned long *),
  ParseAbsoluteGeometry(const char *,RectangleInfo *),
  ParseAffineGeometry(const char *,AffineMatrix *),
  ParseGeometry(const char *,GeometryInfo *),
  ParseGravityGeometry(Image *,const char *,RectangleInfo *),
  ParseMetaGeometry(const char *,long *,long *,unsigned long *,unsigned long *),
  ParsePageGeometry(Image *,const char *,RectangleInfo *),
  ParseSizeGeometry(Image *,const char *,RectangleInfo *);

extern MagickExport void
  SetGeometry(const Image *,RectangleInfo *),
  SetGeometryInfo(GeometryInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
