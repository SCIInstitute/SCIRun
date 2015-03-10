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

#include <Core/Math/MiscMath.h>
#include <Core/Datatypes/ColorMap.h>
#include <iostream>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

ColorMap::ColorMap(const std::string& name, const size_t resolution, const double shift,
  const bool invert, const double rescale_scale, const double rescale_shift)
  : name_(name), resolution_(resolution), shift_(shift),
  invert_(invert), rescale_scale_(rescale_scale), rescale_shift_(rescale_shift)
{

}

ColorMap* ColorMap::clone() const
{
  return new ColorMap(*this);
}

ColorMapHandle StandardColorMapFactory::create(const std::string& name, const size_t &res,
  const double &shift, const bool &invert,
  const double &rescale_scale, const double &rescale_shift)
{
  if (!(name == "Rainbow" ||
    name == "Old Rainbow" ||
    name == "Blackbody" ||
    name == "Grayscale"))
    THROW_INVALID_ARGUMENT("Color map name not implemented/recognized.");

  return boost::make_shared<ColorMap>(name, res, shift, invert, rescale_scale, rescale_shift);
}

double ColorMap::Hue_2_RGB(double v1, double v2, double vH) 
{
  if (vH < 0) vH += 1.;
  if (vH > 1) vH -= 1.;
  if ((6 * vH) < 1) return (v1 + (v2 - v1) * 6. * vH);
  if ((2 * vH) < 1) return (v2);
  if ((3 * vH) < 2) return (v1 + (v2 - v1) * ((.666667) - vH) * 6.);
  return (v1);
}

ColorRGB ColorMap::hslToRGB(double h, double s, double l) 
{
  double r, g, b;
  if (s == 0.) {
    r = g = b = l;
  }
  else {
    double var_1, var_2;
    if (l < 0.5) var_2 = l * (1. + s);
    else           var_2 = (l + s) - (s * l);

    var_1 = 2 * l - var_2;

    r = Hue_2_RGB(var_1, var_2, h + (.333333));
    g = Hue_2_RGB(var_1, var_2, h);
    b = Hue_2_RGB(var_1, var_2, h - (.333333));
  }
  return ColorRGB(std::min(std::max(r, 0.), 1.),
    std::min(std::max(g, 0.), 1.),
    std::min(std::max(b, 0.), 1.));
}


double ColorMap::getTransformedColor(double f) const 
{
  /////////////////////////////////////////////////
  //TODO: this seemingly useless code fixes a nasty crash bug on Windows. Don't delete it until a proper fix is implemented!
  static bool x = true;
  if (x)
  {
    std::cout << "";// this;// << " " << name_ << " " << resolution_ << " " << shift_ << " " << invert_ << std::endl;
    x = false;
  }
  /////////////////////////////////////////////////

  const double rescaled01 = static_cast<double>((f + rescale_shift_) * rescale_scale_);

  double v = std::min(std::max(0., rescaled01), 1.);
  double shift = shift_;
  if (invert_) {
    v = 1.f - v;
    shift *= -1.;
  }
  //apply the resolution
  v = static_cast<double>((static_cast<int>(v *
    static_cast<double>(resolution_)))) /
    static_cast<double>(resolution_ - 1);
  // the shift is a gamma.
  double denom = std::tan(M_PI_2 * (0.5 - std::min(std::max(shift, -0.99), 0.99) * 0.5));
  // make sure we don't hit divide by zero
  if (std::isnan(denom)) denom = 0.f;
  denom = std::max(denom, 0.001);
  v = std::pow(v, (1. / denom));
  return std::min(std::max(0.,v),1.);
}

ColorRGB ColorMap::getColorMapVal(double v) const 
{
  double f = getTransformedColor(v);
  //now grab the RGB
  ColorRGB col;
  if (name_ == "Rainbow") {
    if (0. <= f && f < 0.25)
      col = ColorRGB(0., f*3., 1. - f);
    else if (0.25 <= f && f < 0.5)
      col = ColorRGB(0., f + 0.5, 1.5 - f*3.);
    else if (0.5 <= f && f < 0.75)
      col = ColorRGB(4.*f - 2., 2. - 2.*f, 0.);
    else if (0.75 <= f && f <= 1.0)
      col = ColorRGB(1., 2. - 2.*f, 0.);
  }
  else if (name_ == "Old Rainbow") {
    col = hslToRGB((1. - f) * 0.675, 0.95, 0.5);
  }
  else if (name_ == "Blackbody") {
    if (f < 0.333333)
      col = ColorRGB(std::min(std::max(f * 3., 0.), 1.), 0., 0.);
    else if (f < 0.6666666)
      col = ColorRGB(1., std::min(std::max((f - 0.333333) * 3., 0.), 1.), 0.);
    else
      col = ColorRGB(1., 1., std::min(std::max((f - 0.6666666) * 3., 0.), 1.));
  }
  else if (name_ == "Grayscale")
    col = hslToRGB(0., 0., f);
  return col;
}

ColorRGB ColorMap::valueToColor(double scalar) const {
    return getColorMapVal(scalar);
}

ColorRGB ColorMap::valueToColor(const Tensor &tensor) const {
    //TODO this is not implemented.
    return ColorRGB(getTransformedColor(fabs(tensor.xx())),
                    getTransformedColor(fabs(tensor.yy())),
                    getTransformedColor(fabs(tensor.zz())));
}
ColorRGB ColorMap::valueToColor(const Vector &vector) const {
    //TODO is this correct?
    return ColorRGB(getTransformedColor(fabs(vector.x())),
                    getTransformedColor(fabs(vector.y())),
                    getTransformedColor(fabs(vector.z())));

}

std::string ColorMap::getColorMapName() const { return name_; }
size_t ColorMap::getColorMapResolution() const { return resolution_; }
double ColorMap::getColorMapShift() const { return shift_; }
bool ColorMap::getColorMapInvert() const { return invert_; }
double ColorMap::getColorMapRescaleScale() const { return rescale_scale_; }
double ColorMap::getColorMapRescaleShift() const { return rescale_shift_; }
void ColorMap::setColorMapRescaleScale(double scale) { rescale_scale_ = scale; }
void ColorMap::setColorMapRescaleShift(double shift) { rescale_shift_ = shift; }
