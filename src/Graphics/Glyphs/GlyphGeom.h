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

#ifndef Graphics_Graphics_Glyphs_H
#define Graphics_Graphics_Glyphs_H


#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Geometry.h>
#include <Graphics/Glyphs/share.h>

namespace SCIRun {
  namespace Graphics {

    class SCISHARE GlyphGeom
    {
    public:
      typedef std::vector<std::pair<Core::Geometry::Point, Core::Geometry::Vector>> QuadStrip;

      GlyphGeom();

      Core::Datatypes::GeometryHandle getObject();

      void addArrow(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double radius, double length, int nu = 20, int nv = 0);
      void addBox(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double x_side, double y_side, double z_side);
      void addCylinder(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double radius1, double length, int nu = 20, int nv = 2);
      void addSphere(const Core::Geometry::Point& center, double radius, int nu=20, int nv=20, int half=0);

    private:
      Core::Datatypes::GeometryHandle objectHandle_;

      void generateBox(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double x_side, double y_side, double z_side, std::vector<QuadStrip>& quadstrips);
      void generateCylinder(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double radius1, double radius2, double length, int nu, int nv, std::vector<QuadStrip>& quadstrips);
      void generateEllipsoid(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double scales, int nu, int nv, int half, std::vector<QuadStrip>& quadstrips);

      
    };
  } //Graphics
} //SCIRun
#endif //Graphics_Graphics_Glyphs_H