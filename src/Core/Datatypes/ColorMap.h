/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


/// @todo Documentation Core/Datatypes/ColorMap.h

#ifndef CORE_DATATYPES_COLORMAP_H
#define CORE_DATATYPES_COLORMAP_H

#include <Core/Datatypes/Datatype.h>
#include <boost/noncopyable.hpp>
#include <Core/Datatypes/Color.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/Tensor.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class SCISHARE ColorMap : public Datatype
  {
  public:
    // Because colors need to be in the range [0,1], and SCIRun4 used [-1,1] for it's
    // default input range, we need to transform by default the data into [0,1] range.
    explicit ColorMap(const std::vector<ColorRGB>& color,
      const std::string& name = "Rainbow", const size_t resolution = 256, const double shift = 0.0,
      const bool invert = false, const double rescale_scale = 0.5, const double rescale_shift = 1.0,
      const std::vector<double>& alphaPoints = {});

    virtual ColorMap* clone() const override;

    std::vector<ColorRGB> getColorData() const {return colorData_;}
    std::string getColorMapName() const {return nameInfo_;}
    size_t getColorMapResolution() const {return resolution_;}
    double getColorMapShift() const {return shift_;}
    bool getColorMapInvert() const {return invert_;}
    double getColorMapRescaleScale() const {return rescale_scale_;}
    double getColorMapRescaleShift() const {return rescale_shift_;}
    std::vector<double> getAlphaLookup() const {return alphaLookup_;}

    ColorRGB valueToColor(double scalar) const;
    ColorRGB valueToColor(Core::Geometry::Tensor &tensor) const;
    ColorRGB valueToColor(const Core::Geometry::Vector &vector) const;

    double valueToIndex(double scalar) const;
    double valueToIndex(Core::Geometry::Tensor &tensor) const;
    double valueToIndex(const Core::Geometry::Vector &vector) const;

    virtual std::string dynamic_type_name() const override { return "ColorMap"; }

  private:
    ///<< Internal functions.
    Core::Datatypes::ColorRGB getColorMapVal(double v) const;
    double getTransformedValue(double v) const;
    ColorRGB applyAlpha(double transformed, ColorRGB colorWithoutAlpha) const;
    double alpha(double transformedValue) const;

    std::vector<ColorRGB> colorData_;
    std::string nameInfo_; //The colormap's name.
    size_t resolution_; //The resolution of the map [2,256].
    double shift_; //The gamma shift.
    bool invert_; //Whether to invert the map or not.
    double rescale_scale_; //Rescaling scale (usually 1. / (data_max - data_min) ).
    double rescale_shift_; //Rescaling shift (usually -data_min). Shift happens before scale.
    std::vector<double> alphaLookup_;
  };

  class SCISHARE StandardColorMapFactory : boost::noncopyable
  {
  public:
   // See explanation for defaults above in ColorMap Constructor
    static ColorMapHandle create(const std::string& name = "Rainbow",
      const size_t &resolution = 256, const double &shift = 0.0, const bool &invert = false,
      const double &rescale_scale = 0.5,  const double &rescale_shift = 1.0,
      const std::vector<double>& alphaPoints = {});

    static ColorMapHandle create(const std::vector<ColorRGB>& colorData,
      const std::string& name = "Custom", const size_t &resolution = 256, const double &shift = 0.0,
      const bool &invert = false, const double &rescale_scale = 0.5, const double &rescale_shift = 1.0,
      const std::vector<double>& alphaPoints = {});

    static std::vector<std::string> getList();
  private:
    StandardColorMapFactory() = delete;
  };

  // colormap helper for ospray transfer function
  class SCISHARE ColorMap_OSP_helper
  {
  public:
    std::vector<float> colorList;
    std::vector<float> opacityList;
    explicit ColorMap_OSP_helper(const std::string& name);
  };

}}}


#endif
