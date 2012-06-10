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

  MagickCore graphic gems methods.
*/
#ifndef _MAGICKCORE_GEM_H
#define _MAGICKCORE_GEM_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <magick/cache-view.h>
#include <magick/pixel.h>

extern MagickExport double
  ExpandAffine(const AffineMatrix *);

extern MagickExport MagickPixelPacket
  InterpolatePixelColor(const Image *,ViewInfo *,InterpolatePixelMethod,
    const double,const double,ExceptionInfo *);

extern MagickExport unsigned long
  GetOptimalKernelWidth(const double,const double),
  GetOptimalKernelWidth1D(const double,const double),
  GetOptimalKernelWidth2D(const double,const double);

extern MagickExport void
  Contrast(const int,Quantum *,Quantum *,Quantum *),
  HSBTransform(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  HSLTransform(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  HWBTransform(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  Hull(const long,const long,const unsigned long,const unsigned long,Quantum *,
    Quantum *,const int),
  ModulateHSB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ModulateHSL(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  ModulateHWB(const double,const double,const double,Quantum *,Quantum *,
    Quantum *),
  TransformHSB(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  TransformHSL(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  TransformHWB(const Quantum,const Quantum,const Quantum,double *,double *,
    double *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
