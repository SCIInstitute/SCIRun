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


#ifndef Graphics_Glyphs_GLYPH_CONSTRUCTOR_H
#define Graphics_Glyphs_GLYPH_CONSTRUCTOR_H

#include <Core/GeometryPrimitives/Vector.h>
#include <Core/GeometryPrimitives/GeomFwd.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/Datatypes/Color.h>
#include <Core/Datatypes/ColorMap.h>
#include <Core/Math/TrigTable.h>
#include <Graphics/Glyphs/share.h>

namespace SCIRun {
namespace Graphics {
class SCISHARE GlyphConstructor
{
public:
  GlyphConstructor();
  void buildObject(Graphics::Datatypes::GeometryObjectSpire& geom, const std::string& uniqueNodeID,
                   const bool isTransparent, const double transparencyValue,
                   const Graphics::Datatypes::ColorScheme& colorScheme, RenderState state,
                   const Graphics::Datatypes::SpireIBO::PRIMITIVE& primIn,
                   const Core::Geometry::BBox& bbox,
                   const bool isClippable = true,
                   const Core::Datatypes::ColorMapHandle colorMap = nullptr);

  uint32_t setOffset();
  bool normalsValid() const;
  void addVertex(const Core::Geometry::Vector& point, const Core::Geometry::Vector& normal,
                 const Core::Datatypes::ColorRGB& color);
  void addVertex(const Core::Geometry::Vector& point, const Core::Datatypes::ColorRGB& color);
  void addLine(const Core::Geometry::Vector& point1, const Core::Geometry::Vector& point2,
               const Core::Datatypes::ColorRGB& color1, const Core::Datatypes::ColorRGB& color2);
  void addPoint(const Core::Geometry::Vector& point, const Core::Datatypes::ColorRGB& color);
  void addLineIndex();
  void addIndex(size_t i);
  void addIndices(size_t i1, size_t i2, size_t i3);
  void addIndicesToOffset(size_t i1, size_t i2, size_t i3);
  void addIndexToOffset(size_t i);
  size_t getCurrentIndex() const;
  void popIndicesNTimes(int n);

private:
  std::vector<SinCosTable> tables_;
  std::vector<Core::Geometry::Vector> points_;
  std::vector<Core::Geometry::Vector> normals_;
  std::vector<Core::Datatypes::ColorRGB> colors_;
  std::vector<size_t> indices_;
  size_t numVBOElements_ = 0;
  size_t lineIndex_ = 0;
  uint32_t offset_ = 0;
};
}}

#endif

