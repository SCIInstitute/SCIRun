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

  class ColorMapStrategy;
  typedef boost::shared_ptr<ColorMapStrategy> ColorMapStrategyHandle;

  class SCISHARE ColorMap : public Datatype
  {
  public:
    // Because colors need to be in the range [0,1], and SCIRun4 used [-1,1] for it's
    // default input range, we need to transform by default the data into [0,1] range.
    explicit ColorMap(ColorMapStrategyHandle color,
      const std::string& name = "Rainbow", const size_t resolution = 256,
                        const double shift = 0.0, const bool invert = false,
                        const double rescale_scale = .5, const double rescale_shift = 1.);
                        //TODO: pass in alpha vector
    virtual ColorMap* clone() const override;

    ColorMapStrategyHandle getColorStrategy() const { return color_; }
    std::string getColorMapName() const;
    size_t getColorMapResolution() const;
    double getColorMapShift() const;
    bool getColorMapInvert() const;
    double getColorMapRescaleScale() const;
    double getColorMapRescaleShift() const;

    ColorRGB valueToColor(double scalar) const;
    ColorRGB valueToColor(const Core::Geometry::Tensor &tensor) const;
    ColorRGB valueToColor(const Core::Geometry::Vector &vector) const;

    virtual std::string dynamic_type_name() const override { return "ColorMap"; }

  private:
    ///<< Internal functions.
    Core::Datatypes::ColorRGB getColorMapVal(double v) const;
    double getTransformedValue(double v) const;

    ColorMapStrategyHandle color_;
    ///<< The colormap's name.
    std::string nameInfo_;
    ///<< The resolution of the map [2,256].
    size_t resolution_;
    ///<< The gamma shift.
    double shift_;
    ///<< Whether to invert the map or not.
    bool invert_;
    ///<< Rescaling scale (usually 1. / (data_max - data_min) ).
    double rescale_scale_;
    ///<< Rescaling shift (usually -data_min). Shift happens before scale.
    double rescale_shift_;

    std::vector<double> alphaLookup_;
  };

  class SCISHARE ColorMapStrategy
  {
  public:
    virtual ~ColorMapStrategy() {}
    virtual ColorRGB getColorMapVal(double v) const = 0;
  };

  //TODO: not sure this needs to be abstract
  class SCISHARE AlphaMapping
  {
  public:
    virtual ~AlphaMapping() {}
    virtual double alpha(double transformedValue) const = 0;
  };

  class SCISHARE StandardColorMapFactory : boost::noncopyable
  {
  public:
   // See explanation for defaults above in ColorMap Constructor
    static ColorMapHandle create(const std::string& name = "Rainbow", const size_t &resolution = 256,
                                    const double &shift = 0.0, const bool &invert = false,
                                    const double &rescale_scale = .5, const double &rescale_shift = 1.);
    typedef std::vector<std::string> NameList;
    static NameList getList();
  private:
    StandardColorMapFactory() = delete;
  };

}}}


#endif
