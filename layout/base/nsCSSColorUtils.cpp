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

/* functions that manipulate colors */

#include "nsCSSColorUtils.h"
#include <math.h>

// Weird color computing code stolen from winfe which was stolen
// from the xfe which was written originally by Eric Bina. So there.

#define RED_LUMINOSITY        299
#define GREEN_LUMINOSITY      587
#define BLUE_LUMINOSITY       114
#define INTENSITY_FACTOR      25
#define LIGHT_FACTOR          0
#define LUMINOSITY_FACTOR     75

#define MAX_COLOR             255
#define COLOR_DARK_THRESHOLD  51
#define COLOR_LIGHT_THRESHOLD 204

#define COLOR_LITE_BS_FACTOR 45
#define COLOR_LITE_TS_FACTOR 70

#define COLOR_DARK_BS_FACTOR 30
#define COLOR_DARK_TS_FACTOR 50

#define LIGHT_GRAY NS_RGB(192, 192, 192)
#define DARK_GRAY  NS_RGB(96, 96, 96)
#define WHITE      NS_RGB(255, 255, 255)
#define BLACK      NS_RGB(0, 0, 0)

#define MAX_BRIGHTNESS  254
#define MAX_DARKNESS     0
 
void NS_Get3DColors(nscolor aResult[2], nscolor aBackgroundColor)
{
  int rb = NS_GET_R(aBackgroundColor);
  int gb = NS_GET_G(aBackgroundColor);
  int bb = NS_GET_B(aBackgroundColor);
  
  int brightness = NS_GetBrightness(rb,gb,bb);

  int f0, f1;
  if (brightness < COLOR_DARK_THRESHOLD) {
    f0 = COLOR_DARK_BS_FACTOR;
    f1 = COLOR_DARK_TS_FACTOR;
  } else if (brightness > COLOR_LIGHT_THRESHOLD) {
    f0 = COLOR_LITE_BS_FACTOR;
    f1 = COLOR_LITE_TS_FACTOR;
  } else {
    f0 = COLOR_DARK_BS_FACTOR +
      (brightness *
       (COLOR_LITE_BS_FACTOR - COLOR_DARK_BS_FACTOR) / MAX_COLOR);
    f1 = COLOR_DARK_TS_FACTOR +
      (brightness *
       (COLOR_LITE_TS_FACTOR - COLOR_DARK_TS_FACTOR) / MAX_COLOR);
  }

  int r = rb - (f0 * rb / 100);
  int g = gb - (f0 * gb / 100);
  int b = bb - (f0 * bb / 100);
  aResult[0] = NS_RGB(r, g, b);
  if ((r == rb) && (g == gb) && (b == bb)) {
    aResult[0] = (aBackgroundColor == BLACK) ? DARK_GRAY : BLACK;
  }

  r = rb + (f1 * (MAX_COLOR - rb) / 100);
  if (r > 255) r = 255;
  g = gb + (f1 * (MAX_COLOR - gb) / 100);
  if (g > 255) g = 255;
  b = bb + (f1 * (MAX_COLOR - bb) / 100);
  if (b > 255) b = 255;
  aResult[1] = NS_RGB(r, g, b);
  if ((r == rb) && (g == gb) && (b == bb)) {
    aResult[1] = (aBackgroundColor == WHITE) ? LIGHT_GRAY : WHITE;
  }
}

void NS_GetSpecial3DColors(nscolor aResult[2],
											   nscolor aBackgroundColor,
											   nscolor aBorderColor)
{

  PRUint8 f0, f1;
  PRUint8 r, g, b;

  PRUint8 rb = NS_GET_R(aBorderColor);
  PRUint8 gb = NS_GET_G(aBorderColor);
  PRUint8 bb = NS_GET_B(aBorderColor);

  // This needs to be optimized.
  // Calculating background brightness again and again is 
  // a waste of time!!!. Just calculate it only once.
  // .....somehow!!!

  PRUint8 red = NS_GET_R(aBackgroundColor);
  PRUint8 green = NS_GET_G(aBackgroundColor);
  PRUint8 blue = NS_GET_B(aBackgroundColor);
  
  PRUint8 elementBrightness = NS_GetBrightness(rb,gb,bb);
  PRUint8 backgroundBrightness = NS_GetBrightness(red, green, blue);


  if (backgroundBrightness < COLOR_DARK_THRESHOLD) {
    f0 = COLOR_DARK_BS_FACTOR;
    f1 = COLOR_DARK_TS_FACTOR;
	if(elementBrightness == MAX_DARKNESS)
	{
       rb = NS_GET_R(DARK_GRAY);
       gb = NS_GET_G(DARK_GRAY);
       bb = NS_GET_B(DARK_GRAY);
	}
  }else if (backgroundBrightness > COLOR_LIGHT_THRESHOLD) {
    f0 = COLOR_LITE_BS_FACTOR;
    f1 = COLOR_LITE_TS_FACTOR;
	if(elementBrightness == MAX_BRIGHTNESS)
	{
       rb = NS_GET_R(LIGHT_GRAY);
       gb = NS_GET_G(LIGHT_GRAY);
       bb = NS_GET_B(LIGHT_GRAY);
	}
  }else {
    f0 = COLOR_DARK_BS_FACTOR +
      (backgroundBrightness *
       (COLOR_LITE_BS_FACTOR - COLOR_DARK_BS_FACTOR) / MAX_COLOR);
    f1 = COLOR_DARK_TS_FACTOR +
      (backgroundBrightness *
       (COLOR_LITE_TS_FACTOR - COLOR_DARK_TS_FACTOR) / MAX_COLOR);
  }
  
  
  r = rb - (f0 * rb / 100);
  g = gb - (f0 * gb / 100);
  b = bb - (f0 * bb / 100);
  aResult[0] = NS_RGB(r, g, b);

  r = rb + (f1 * (MAX_COLOR - rb) / 100);
  g = gb + (f1 * (MAX_COLOR - gb) / 100);
  b = bb + (f1 * (MAX_COLOR - bb) / 100);
  aResult[1] = NS_RGB(r, g, b);
}

int NS_GetBrightness(PRUint8 aRed, PRUint8 aGreen, PRUint8 aBlue)
{

  PRUint8 intensity = (aRed + aGreen + aBlue) / 3;

  PRUint8 luminosity = NS_GetLuminosity(NS_RGB(aRed, aGreen, aBlue)) / 1000;
 
  return ((intensity * INTENSITY_FACTOR) +
          (luminosity * LUMINOSITY_FACTOR)) / 100;
}

PRInt32 NS_GetLuminosity(nscolor aColor)
{
  return (NS_GET_R(aColor) * RED_LUMINOSITY +
          NS_GET_G(aColor) * GREEN_LUMINOSITY +
          NS_GET_B(aColor) * BLUE_LUMINOSITY);
}

// Function to convert RGB color space into the HSV colorspace
// Hue is the primary color defined from 0 to 359 degrees
// Saturation is defined from 0 to 255.  The higher the number.. the deeper the color
// Value is the brightness of the color. 0 is black, 255 is white.  
void
NS_RGB2HSV(nscolor aColor,PRUint16 &aHue,PRUint16 &aSat,PRUint16 &aValue)
{
PRUint8  r,g,b;
PRInt16  delta,min,max,r1,b1,g1;
float    hue;

  r = NS_GET_R(aColor);
  g = NS_GET_G(aColor);
  b = NS_GET_B(aColor);

  if (r>g) {
    max = r;
    min = g;
  } else {
    max = g;
    min = r;
  }

  if (b>max) {
    max = b;
  }
  if (b<min) {
    min = b;
  }

  // value or brightness will always be the max of all the colors(RGB)
  aValue = max;   
  delta = max-min;
  aSat = (max!=0)?((delta*255)/max):0;
  r1 = r;
  b1 = b;
  g1 = g;

  if (aSat==0) {
    hue = 1000;
  } else {
    if(r==max){
      hue=(float)(g1-b1)/(float)delta;
    } else if (g1==max) {
      hue= 2.0f+(float)(b1-r1)/(float)delta;
    } else { 
      hue = 4.0f+(float)(r1-g1)/(float)delta;
    }
  }

  if(hue<999) {
    hue*=60;
    if(hue<0){
      hue+=360;
    }
  } else {
    hue=0;
  }

  aHue = (PRUint16)hue;
}

// Function to convert HSV color space into the RGB colorspace
// Hue is the primary color defined from 0 to 359 degrees
// Saturation is defined from 0 to 255.  The higher the number.. the deeper the color
// Value is the brightness of the color. 0 is black, 255 is white.  
void
NS_HSV2RGB(nscolor &aColor,PRUint16 aHue,PRUint16 aSat,PRUint16 aValue)
{
PRUint16  r=0,g=0,b=0;
PRUint16  i,p,q,t;
double    h,f,percent;

  if ( aSat == 0 ){
    // achromatic color, no hue is defined
    r = aValue;
    g = aValue;
    b = aValue;
  } else {
    // hue in in degrees around the color wheel defined from
    // 0 to 360 degrees.  
    if (aHue >= 360) {
      aHue = 0;
    }

    // we break the color wheel into 6 areas.. these
    // areas define how the saturation and value define the color.
    // reds behave differently than the blues
    h = (double)aHue / 60.0;
    i = (PRUint16) floor(h);
    f = h-(double)i;
    percent = ((double)aValue/255.0);   // this needs to be a value from 0 to 1, so a percentage
                                        // can be calculated of the saturation.
    p = (PRUint16)(percent*(255-aSat));
    q = (PRUint16)(percent*(255-(aSat*f)));
    t = (PRUint16)(percent*(255-(aSat*(1.0-f))));

    // i is guaranteed to never be larger than 5.
    switch(i){
      case 0: r = aValue; g = t; b = p;break;
      case 1: r = q; g = aValue; b = p;break;
      case 2: r = p; g = aValue; b = t;break;
      case 3: r = p; g = q; b = aValue;break;
      case 4: r = t; g = p; b = aValue;break;
      case 5: r = aValue; g = p; b = q;break;
    }
  }
  aColor = NS_RGB(r,g,b);
}
