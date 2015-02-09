/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   
   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
/// @todo Documentation Core/Datatypes/ColorMap.cc

#include <Core/Datatypes/ColorMap.h>

using namespace SCIRun::Core::Datatypes;

ColorMap::ColorMap(const std::string& name, const size_t resolution, const double shift)
: name_(name), resolution_(resolution), shift_(shift) {}

ColorMap* ColorMap::clone() const
{
  return new ColorMap(name_);
}

ColorMapHandle StandardColorMapFactory::create(const std::string& name)
{
  if (name == "Rainbow")
    return ColorMapHandle(rainbow_.clone());
  if (name == "Grayscale")
    return ColorMapHandle(grayscale_.clone());
  if (name == "Blackbody")
    return ColorMapHandle(blackbody_.clone());
  THROW_INVALID_ARGUMENT("Unknown standard colormap name: " + name);
}

float ColorMap::Hue_2_RGB(float v1, float v2, float vH) {
   if ( vH < 0 ) vH += 1.;
   if ( vH > 1 ) vH -= 1.;
   if ( ( 6 * vH ) < 1 ) return ( v1 + ( v2 - v1 ) * 6. * vH );
   if ( ( 2 * vH ) < 1 ) return ( v2 );
   if ( ( 3 * vH ) < 2 ) return ( v1 + ( v2 - v1 ) * ( ( .666667 ) - vH ) * 6. );
   return ( v1 );
}

ColorRGB ColorMap::hslToRGB(float h, float s, float l) {
    float r,g,b;
    if ( s == 0. ) {
       r = g = b = l ;
    } else {
       float var_1, var_2;
       if ( l < 0.5 ) var_2 = l * ( 1. + s );
       else           var_2 = ( l + s ) - ( s * l );

       var_1 = 2 * l - var_2;

       r = Hue_2_RGB( var_1, var_2, h + ( .333333 ) );
       g = Hue_2_RGB( var_1, var_2, h );
       b = Hue_2_RGB( var_1, var_2, h - ( .333333 ) );
    }
    return ColorRGB(r,g,b);
}

ColorRGB ColorMap::getColorMapVal(float v) {
    //@todo this will not be needed with rescale color map.
    v = std::min(std::max(0.f,v),1.f);
    double shift = (shift_+1.)/2.;
    //apply the resolution
    v = static_cast<double>((static_cast<int>(v * static_cast<float>(resolution_)))) / static_cast<double>(resolution_ - 1);
    ColorRGB col;
    if (name_ == "Rainbow")
        col = hslToRGB((1.0-v) * 0.8, 0.95, 0.5);
    else if (name_ == "Blackbody") {
        if (v < 0.333333)
            col = ColorRGB(v * 3., 0., 0.);
        else if (v < 0.6666667)
            col = ColorRGB(1.,(v - 0.333333) * 3., 0.);
        else
            col = ColorRGB(1., 1., (v - 0.6666667) * 3.);
    } else if (name_ == "Grayscale")
        col = hslToRGB(0., 0., v);
    return col;
}

ColorMap StandardColorMapFactory::rainbow_("Rainbow");
ColorMap StandardColorMapFactory::grayscale_("Grayscale");
ColorMap StandardColorMapFactory::blackbody_("Blackbody");
