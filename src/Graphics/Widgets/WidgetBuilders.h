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

#ifndef Graphics_Graphics_Widgets_WidgetBuilders_H
#define Graphics_Graphics_Widgets_WidgetBuilders_H

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/Widget.h>
#include <Graphics/Widgets/share.h>

namespace SCIRun
{
  namespace Graphics
  {
    namespace Datatypes
    {
      template <class Derived>
      class CommonWidgetBuilder
      {
      public:
        explicit CommonWidgetBuilder(const Core::GeometryIDGenerator& g) : idGenerator_(g) {}
        Derived& tag(const std::string& t) { tag_ = t; return static_cast<Derived&>(*this); }
        Derived& scale(double s) { scale_ = s; return static_cast<Derived&>(*this); }
        Derived& defaultColor(const std::string& c) { defaultColor_ = c; return static_cast<Derived&>(*this); }
        Derived& origin(const Core::Geometry::Point& p) { origin_ = p; return static_cast<Derived&>(*this); }
        Derived& boundingBox(const Core::Geometry::BBox& b) { bbox_ = b; return static_cast<Derived&>(*this); }
        Derived& resolution(int r) { resolution_ = r; return static_cast<Derived&>(*this); }
      protected:
        const Core::GeometryIDGenerator& idGenerator_;
        std::string tag_;
        TransformMappingParams mapping_; //later
        double scale_ {0};
        std::string defaultColor_;
        Core::Geometry::Point origin_;
        Core::Geometry::BBox bbox_;
        int resolution_ {0};
      };

      class SCISHARE SphereWidgetBuilder : public CommonWidgetBuilder<SphereWidgetBuilder>
      {
      public:
        using CommonWidgetBuilder::CommonWidgetBuilder;
        SphereWidgetBuilder& centerPoint(const Core::Geometry::Point& c) { point_ = c; return *this; };
        WidgetHandle build() const;
      private:
        Core::Geometry::Point point_;
      };
    }
  }
}

#endif
