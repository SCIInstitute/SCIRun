/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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
#include <Core/Logging/Log.h>
#include <iostream>
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/copy.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;

ColorMap::ColorMap(ColorMapStrategyHandle color, const std::string& name, const size_t resolution, const double shift,
  const bool invert, const double rescale_scale, const double rescale_shift)
  : color_(color), nameInfo_(name), resolution_(resolution), shift_(shift),
  invert_(invert), rescale_scale_(rescale_scale), rescale_shift_(rescale_shift)
{

}

ColorMap* ColorMap::clone() const
{
  return new ColorMap(*this);
}

namespace detail
{
  class Rainbow : public ColorMapStrategy
  {
  public:
    virtual ColorRGB getColorMapVal(double v) const override;
  };

  class OldRainbow : public ColorMapStrategy
  {
  public:
    virtual ColorRGB getColorMapVal(double v) const override;
  };

  class Blackbody : public ColorMapStrategy
  {
  public:
    virtual ColorRGB getColorMapVal(double v) const override;
  };

  class Grayscale : public ColorMapStrategy
  {
  public:
    virtual ColorRGB getColorMapVal(double v) const override;
  };

  class OrangeBlackLime : public ColorMapStrategy
  {
  public:
    virtual ColorRGB getColorMapVal(double v) const override;
  };

  class Darkhue : public ColorMapStrategy
  {
  public:
    virtual Core::Datatypes::ColorRGB getColorMapVal(double v) const override;
  };

  class BPSeismic : public ColorMapStrategy
  {
  public:
    virtual Core::Datatypes::ColorRGB getColorMapVal(double v) const override;
  };

  typedef boost::function<ColorMapStrategy*()> ColorMapMaker;
  static std::map<std::string, ColorMapMaker> colorMapFactoryMap =
  {
    { "Rainbow", boost::factory<Rainbow*>() },
    { "Old Rainbow", boost::factory<OldRainbow*>() },
    { "Blackbody", boost::factory<Blackbody*>() },
    { "Grayscale", boost::factory<Grayscale*>() },
    { "Orange,Black,Lime", boost::factory<OrangeBlackLime*>() },
    { "Darkhue", boost::factory<Darkhue*>() },
    { "BP Seismic", boost::factory<BPSeismic*>() }
  };
}

ColorMapHandle StandardColorMapFactory::create(const std::string& name, const size_t &res,
  const double &shift, const bool &invert,
  const double &rescale_scale, const double &rescale_shift)
{
  using namespace detail;
  ColorMapStrategyHandle color;
  auto maker = colorMapFactoryMap.find(name);
  if (maker != colorMapFactoryMap.end())
    color.reset(maker->second());
  else
  {
    Log::get() << ERROR_LOG << "Color map name not implemented/recognized. Returning Rainbow." << std::endl;
    color.reset(new Rainbow);
  }

  return boost::make_shared<ColorMap>(color, name, res, shift, invert, rescale_scale, rescale_shift);
}

StandardColorMapFactory::NameList StandardColorMapFactory::getList()
{
  std::vector<std::string> names;
  boost::copy(detail::colorMapFactoryMap | boost::adaptors::map_keys, std::back_inserter(names));
  return names;
}

/**
 * @name getTransformedColor
 * @brief This method transforms the raw data into ColorMap space.
 * This includes the resolution, the gamma shift, and data rescaling (using data min/max)
 * @param v The input value from raw data that will be transformed (usually into [0,1] space).
 * @return The scalar double value transformed into ColorMap space from raw data.
 */
double ColorMap::getTransformedValue(double f) const
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
/**
 * @name getColorMapVal
 * @brief This method returns the RGB value for the current colormap parameters.
 * The input comes from raw data values. To scale to data, ColorMap
 *           must be created with those parameters. The input is transformed, then
 *           used to select a color from a set of color maps (currently defined by
 *           strings.
 * @param v The input value from raw data that will be mapped to a color.
 * @return The RGB value mapped from the transformed input into the ColorMap's named map.
 */
ColorRGB ColorMap::getColorMapVal(double v) const
{
  double f = getTransformedValue(v);
  //now grab the RGB
  auto colorWithoutAlpha = color_->getColorMapVal(f);
  //TODO:
  //return applyAlpha(f, colorWithoutAlpha);
  return colorWithoutAlpha;
}
/*
ColorRGB applyAlpha(double transformed, colorWithoutAlpha)
...easy


double alpha(double transformedValue)
{
  // interpolate in alphaLookup_
}
*/

/**
 * @name valueToColor
 * @brief Takes a scalar value and directly passes into getColorMap.
 * @param The raw data value as a scalar double.
 * @return The RGB value mapped from the scalar.
 */
ColorRGB ColorMap::valueToColor(double scalar) const {
    return getColorMapVal(scalar);
}
/**
 * @name valueToColor
 * @brief Takes a tensor value and creates an RGB value.
 * @param The raw data value as a tensor.
 * @return The RGB value mapped from the tensor.
 */
ColorRGB ColorMap::valueToColor(const Tensor &tensor) const {
    //TODO this is probably not implemented correctly.
    //return ColorRGB(getTransformedColor(fabs(tensor.xx())), getTransformedColor(fabs(tensor.yy())), getTransformedColor(fabs(tensor.zz())));
  double eigen1, eigen2, eigen3;
  Tensor ten = tensor;
  ten.get_eigenvalues(eigen1, eigen2, eigen3);
  double primaryEigen = std::max(std::max(eigen1, eigen2), eigen3);
  return getColorMapVal(primaryEigen);
}
/**
 * @name valueToColor
 * @brief Takes a vector value and creates an RGB value.
 * @param The raw data value as a vector.
 * @return The RGB value mapped from the vector.
 */
ColorRGB ColorMap::valueToColor(const Vector &vector) const {
    //TODO this is probably not implemented correctly.
   // return ColorRGB(getTransformedColor(fabs(vector.x())),getTransformedColor(fabs(vector.y())), getTransformedColor(fabs(vector.z())));
  return getColorMapVal(vector.length());
}

// This Rainbow takes into account scientific visualization recommendations.
// It tones down the yellow/cyan values so they don't appear to
// be "brighter" than the other colors. All colors "appear" to be the
// same brightness.
// Blue -> Dark Cyan -> Green -> Orange -> Red
ColorRGB detail::Rainbow::getColorMapVal(double f) const
{
  ColorRGB col;
  if (f < 0.25)
    col = ColorRGB(0., f*3., 1. - f);
  else if (0.25 <= f && f < 0.5)
    col = ColorRGB(0., f + 0.5, 1.5 - f*3.);
  else if (0.5 <= f && f < 0.75)
    col = ColorRGB(4.*f - 2., 2. - 2.*f, 0.);
  else
    col = ColorRGB(1., 2. - 2.*f, 0.);
  return col;
}

//The Old Rainbow that simply transitions from blue to red 1 color at a time.
// Blue -> Cyan -> Green -> Yellow -> Red
ColorRGB detail::OldRainbow::getColorMapVal(double f) const
{
  ColorRGB col;
  if (f < 0.25)
    col = ColorRGB(0., 4.*f, 1.);
  else if (0.25 <= f && f < 0.5)
    col = ColorRGB(0., 1., (.5 - f)*4.);
  else if (0.5 <= f && f < 0.75)
    col = ColorRGB((f - 0.5)*4., 1., 0.);
  else
    col = ColorRGB(1., (1. - f)*4., 0.);
  return col;
}

// This map is designed to appear like a heat-map, where "cooler" (lower) values
// are darker and approach black, and "hotter" (higher) values are lighter
// and approach white. In between, you have the red, orange, and yellow transitions.
ColorRGB detail::Blackbody::getColorMapVal(double f) const
{
  ColorRGB col;
  if (f < 0.333333)
    col = ColorRGB(f * 3., 0., 0.);
  else if (f < 0.6666666)
    col = ColorRGB(1., (f - 0.333333) * 3., 0.);
  else
    col = ColorRGB(1., 1., (f - 0.6666666) * 3.);
  return col;
}

// A very simple black to white map with grays in between.
ColorRGB detail::Grayscale::getColorMapVal(double f) const
{
  ColorRGB col;
  col = ColorRGB(f, f, f);
  return col;
}

// This color scheme sets a transition of color that goes
// Orange -> Black -> Lime
ColorRGB detail::OrangeBlackLime::getColorMapVal(double f) const
{
  ColorRGB col;
  if (f < 0.5)
    col = ColorRGB((0.5 - f) * 2., 0.5 - f, 0.);
  else
    col = ColorRGB(0., (f - 0.5) * 2., 0.);
  return col;
}

// This color scheme sets a transition of color that goes
// Blue -> White -> Red
ColorRGB detail::BPSeismic::getColorMapVal(double f) const
{
  ColorRGB col;
  if (f < 0.5)
    col = ColorRGB(f*2, f*2, 1);
  else
    col = ColorRGB(1, (1-f)*2, (1-f)*2);
  return col;
}

ColorRGB detail::Darkhue::getColorMapVal(double f) const
{
  ColorRGB col;
  if (f < 0.25)
    col = ColorRGB(0., 0., (f * 4.) * 0.333333);
  else if (0.25 <= f && f < 0.5)
    col = ColorRGB((f - 0.25) * 2., 0., f + ((0.5 - f) * 0.333333));
  else if (0.5 <= f && f < 0.75)
    col = ColorRGB(f + (f - 0.5), 0., f - ((f - 0.5) * 3.));
  else
    col = ColorRGB(1., (f - 0.75) * 4., (f - 0.75) * 2.6666666);
  return col;
}

std::string ColorMap::getColorMapName() const { return nameInfo_; }
size_t ColorMap::getColorMapResolution() const { return resolution_; }
double ColorMap::getColorMapShift() const { return shift_; }
bool ColorMap::getColorMapInvert() const { return invert_; }
double ColorMap::getColorMapRescaleScale() const { return rescale_scale_; }
double ColorMap::getColorMapRescaleShift() const { return rescale_shift_; }
