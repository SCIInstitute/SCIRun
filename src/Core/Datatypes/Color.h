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


/// @todo Documentation Core/Datatypes/Color.h

#ifndef CORE_DATATYPES_COLORRGB_H
#define CORE_DATATYPES_COLORRGB_H

#include <iosfwd>
#include <Core/Datatypes/Datatype.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Algorithms/Base/Variable.h>
#include <Core/Datatypes/share.h>

namespace SCIRun {
namespace Core {
namespace Datatypes {

  class SCISHARE ColorRGB
  {
  private:
    double r_, g_, b_, a_;
  public:
    ColorRGB();
    explicit ColorRGB(const std::string& rgb);
    explicit ColorRGB(double v);
    ColorRGB(double r, double g, double b);
    ColorRGB(double r, double g, double b, double a);
    explicit ColorRGB(unsigned int rgbHexValue);
    //adjust alpha while copying
    //ColorRGB(const ColorRGB& color, double a);

    // These equality operations should use floating point comparisons.
    bool operator==(const ColorRGB& c) const {
      return ((r_==c.r_)&&(g_==c.g_)&&(b_==c.b_)&&(a_==c.a_));
    }

    bool operator!=(const ColorRGB& c) const {
      return !(*this == c);
    }

    /// \todo Add normalization function to normalize 0 - 255 to 0 - 1.0.
    ///       useful when reading colors from strings.

    double r() const {return r_;}
    double g() const {return g_;}
    double b() const {return b_;}
    double a() const {return a_;}

    std::string toString() const;
  };

  typedef boost::shared_ptr<ColorRGB> ColorRGBHandle;

  SCISHARE std::ostream& operator<<(std::ostream& out, const ColorRGB& color);

  struct SCISHARE ViewSceneFeedback : ModuleFeedback
  {
    Geometry::Transform transform;
    std::string selectionName;
    std::tuple<int,int> windowSize;

    bool matchesWithModuleId(const std::string& modId) const;
  };

  struct SCISHARE MeshComponentSelectionFeedback : ModuleFeedback
  {
    MeshComponentSelectionFeedback() {}
    MeshComponentSelectionFeedback(const std::string& mod, const std::string& comp, bool sel) :
      moduleId(mod), component(comp), selected(sel) {}
    std::string moduleId;
    std::string component;
    bool selected {false};
  };

}}}


#endif
