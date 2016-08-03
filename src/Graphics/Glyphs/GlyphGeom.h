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

#ifndef Graphics_Graphics_Glyphs_GlyphGeom_H
#define Graphics_Graphics_Glyphs_GlyphGeom_H

#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Core/Math/TrigTable.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/Datatypes/Color.h>

#include <Graphics/Glyphs/share.h>

namespace SCIRun {
  namespace Graphics {

    class SCISHARE GlyphGeom
    {
    public:
      typedef std::vector<std::pair<Core::Geometry::Point, Core::Geometry::Vector>> QuadStrip;

      GlyphGeom();
      
      void getBufferInfo(int64_t& numVBOElements, std::vector<Core::Geometry::Vector>& points,
        std::vector<Core::Geometry::Vector>& normals, std::vector<Core::Datatypes::ColorRGB>& colors, std::vector<uint32_t>& indices);
      
      void buildObject(Datatypes::GeometryHandle geom, const std::string& uniqueNodeID, const bool isTransparent, const double transparencyValue,
        const Datatypes::ColorScheme& colorScheme, RenderState state,
        const Datatypes::SpireIBO::PRIMITIVE& primIn, const Core::Geometry::BBox& bbox);

      void addArrow(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius, double resolution, 
        const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
      void addSphere(const Core::Geometry::Point& p, double radius, double resolution, const Core::Datatypes::ColorRGB& color);
      void addEllipsoid(const Core::Geometry::Point& p, double radius1, double radius2, double resolution, const Core::Datatypes::ColorRGB& color);
      void addCylinder(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius, double resolution,
        const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
      void addCone(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, double radius, double resolution,
        const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
      void addClippingPlane(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
        const Core::Geometry::Point& p3, const Core::Geometry::Point& p4, double radius, double resolution,
        const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
      void addPlane(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
        const Core::Geometry::Point& p3, const Core::Geometry::Point& p4,
        const Core::Datatypes::ColorRGB& color1);

      void addLine(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2,
        const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
      void addNeedle(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, 
        const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
      void addPoint(const Core::Geometry::Point& p, const Core::Datatypes::ColorRGB& color);

      //From SCIRun4
      void addArrow(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double radius, double length, int nu = 20, int nv = 0);
      void addBox(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double x_side, double y_side, double z_side);
      void addCylinder(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double radius1, double length, int nu = 20, int nv = 2);
      void addSphere(const Core::Geometry::Point& center, double radius, int nu=20, int nv=20, int half=0);      
      
    private:
      std::vector<SinCosTable> tables_;
      std::vector<Core::Geometry::Vector> points_;
      std::vector<Core::Geometry::Vector> normals_;
      std::vector<Core::Datatypes::ColorRGB> colors_;
      std::vector<uint32_t> indices_;
      int64_t numVBOElements_;
      uint32_t lineIndex_;
            
      void generateCylinder(const  Core::Geometry::Point& p1, const  Core::Geometry::Point& p2, double radius1, double radius2, double resolution, const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2,
        int64_t& numVBOElements, std::vector<Core::Geometry::Vector>& points, std::vector<Core::Geometry::Vector>& normals, std::vector<uint32_t>& indices, std::vector<Core::Datatypes::ColorRGB>& colors);
      void generateEllipsoid(const Core::Geometry::Point& center, double radius1, double radius2, double resolution, const Core::Datatypes::ColorRGB& color,
        int64_t& numVBOElements, std::vector<Core::Geometry::Vector>& points, std::vector<Core::Geometry::Vector>& normals, std::vector<uint32_t>& indices, std::vector<Core::Datatypes::ColorRGB>& colors);
      void generateSphere(const Core::Geometry::Point& center, double radius1, double radius2, double resolution, const Core::Datatypes::ColorRGB& color,
        int64_t& numVBOElements, std::vector<Core::Geometry::Vector>& points, std::vector<Core::Geometry::Vector>& normals, std::vector<uint32_t>& indices, std::vector<Core::Datatypes::ColorRGB>& colors);
      void generateLine(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2,
        int64_t& numVBOElements, std::vector<Core::Geometry::Vector>& points, std::vector<uint32_t>& indices, std::vector<Core::Datatypes::ColorRGB>& colors);
      void generatePoint(const Core::Geometry::Point& p, const Core::Datatypes::ColorRGB& color,
        int64_t& numVBOElements, std::vector<Core::Geometry::Vector>& points, std::vector<uint32_t>& indices, std::vector<Core::Datatypes::ColorRGB>& colors);
      void generatePlane(const Core::Geometry::Point& p1, const Core::Geometry::Point& p2, 
        const Core::Geometry::Point& p3, const Core::Geometry::Point& p4, const Core::Datatypes::ColorRGB& color,
        int64_t& numVBOElements, std::vector<Core::Geometry::Vector>& points, std::vector<Core::Geometry::Vector>& normals, std::vector<uint32_t>& indices, std::vector<Core::Datatypes::ColorRGB>& colors);

      //From SCIRun4
      void generateBox(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double x_side, double y_side, double z_side, std::vector<QuadStrip>& quadstrips);
      void generateCylinder(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double radius1, double radius2, double length, int nu, int nv, std::vector<QuadStrip>& quadstrips);
      void generateEllipsoid(const Core::Geometry::Point& center, const Core::Geometry::Vector& t, double scales, int nu, int nv, int half, std::vector<QuadStrip>& quadstrips);
      void generateTransforms(const Core::Geometry::Point& center, const Core::Geometry::Vector& normal, Core::Geometry::Transform& trans, Core::Geometry::Transform& rotate);
      
      
    };
  } //Graphics
} //SCIRun
#endif //Graphics_Graphics_Glyphs_GlyphGeom_H