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


#ifndef Graphics_Graphics_Glyphs_GlyphGeom_H
#define Graphics_Graphics_Glyphs_GlyphGeom_H

#include <Graphics/Datatypes/RenderFieldState.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Datatypes/Dyadic3DTensor.h>
#include <Core/Math/TrigTable.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/Datatypes/Color.h>
#include <Graphics/Glyphs/GlyphConstructor.h>

#include <Graphics/Glyphs/share.h>

namespace SCIRun {
namespace Graphics {
class SCISHARE GlyphGeom
{
private:
  GlyphConstructor constructor_;

public:
  GlyphGeom();

  void buildObject(Datatypes::GeometryObjectSpire& geom, const std::string& uniqueNodeID,
                   const bool isTransparent, const double transparencyValue,
                   const Datatypes::ColorScheme& colorScheme, RenderState state,
                   const Core::Geometry::BBox& bbox, const bool isClippable = true,
                   const Core::Datatypes::ColorMapHandle colorMap = nullptr);
  void addArrow(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius,
                double ratio, int resolution, const Core::Datatypes::ColorRGB& color1,
                const Core::Datatypes::ColorRGB& color2, bool render_cylinder_base,
                bool render_cone_base, bool showNormals, double showNormalsScale);
  void addSphere(const Core::Geometry::Point& p, double radius, int resolution,
                 const Core::Datatypes::ColorRGB& color, bool showNormals, double showNormalsScale);
  void addBox(const Core::Geometry::Point& center, Core::Datatypes::Dyadic3DTensor& t, double scale,
              Core::Datatypes::ColorRGB& color, bool normalize, bool showNormals, double showNormalsScale);
  void addEllipsoid(const Core::Geometry::Point& center, Core::Datatypes::Dyadic3DTensor& t, double scale,
                    int resolution, const Core::Datatypes::ColorRGB& color, bool normalize, bool showNormals, double showNormalsScale);
  void addSuperquadricTensor(const Core::Geometry::Point& center, Core::Datatypes::Dyadic3DTensor& t,
                             double scale, int resolution, const Core::Datatypes::ColorRGB& color,
                             bool normalize, double emphasis, bool showNormals, double showNormalsScale);
  void addCylinder(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius,
                   int resolution, const Core::Datatypes::ColorRGB& color1,
                   const Core::Datatypes::ColorRGB& color2,
                   bool showNormals, double showNormalsScale,
                   bool renderBase1 = false, bool renderBase2 = false);
  void addCylinder(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius1,
                   double radius2, int resolution, const Core::Datatypes::ColorRGB& color1,
                   const Core::Datatypes::ColorRGB& color2,
                   bool showNormals, double showNormalsScale,
                   bool renderBase1 = false, bool renderBase2 = false);
  void addCone(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius,
               int resolution, bool renderBase, const Core::Datatypes::ColorRGB& color1,
               const Core::Datatypes::ColorRGB& color2, bool showNormals, double showNormalsScale);
  void addDisk(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius,
               int resolution, const Core::Datatypes::ColorRGB& color1,
               const Core::Datatypes::ColorRGB& color2, bool showNormals, double showNormalsScale);
  void addComet(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius,
                int resolution, const Core::Datatypes::ColorRGB& color1,
                const Core::Datatypes::ColorRGB& color2, double sphere_extrusion, bool showNormals, double showNormalsScale);
  void addTorus(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
                double major_radius, double minor_radius, int resolution,
                const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2, bool showNormals, double showNormalsScale);
  void addClippingPlane(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
                        const Core::Geometry::Point& p3, const Core::Geometry::Point& p4,
                        double radius, int resolution, const Core::Datatypes::ColorRGB& color1,
                        const Core::Datatypes::ColorRGB& color2);
  void addPlane(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
                const Core::Geometry::Point& p3, const Core::Geometry::Point& p4,
                const Core::Datatypes::ColorRGB& color1);

  void addLine(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
               const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
  void addNeedle(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
                 const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
  void addPoint(const Core::Geometry::Point& p, const Core::Datatypes::ColorRGB& color);
  void addSuperquadricSurface(const Core::Geometry::Point& center, Core::Datatypes::Dyadic3DTensor& t,
                              double scale, int resolution, const Core::Datatypes::ColorRGB& color,
                              double A, double B, bool showNormals, double showNormalsScale);
  void generateSphere(const Core::Geometry::Point& center, double radius, int resolution,
                      const Core::Datatypes::ColorRGB& color, bool showNormals, double showNormalsScale);
  void generatePlane(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
                     const Core::Geometry::Point& p3, const Core::Geometry::Point& p4,
                     const Core::Datatypes::ColorRGB& color);
};
}}

#endif //Graphics_Graphics_Glyphs_GlyphGeom_H
