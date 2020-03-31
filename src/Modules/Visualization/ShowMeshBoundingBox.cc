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


#include <Core/Algorithms/Visualization/RenderFieldState.h>
#include <Modules/Visualization/ShowMeshBoundingBox.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Glyphs/GlyphGeom.h>

using namespace SCIRun;
using namespace Modules::Visualization;
using namespace Dataflow::Networks;
using namespace Core;
using namespace Core::Algorithms;
using namespace Core::Datatypes;
using namespace Core::Geometry;
using namespace Graphics;
using namespace Graphics::Datatypes;

MODULE_INFO_DEF(ShowMeshBoundingBox, Visualization, SCIRun);

namespace SCIRun {
namespace Modules {
namespace Visualization {
  class ShowMeshBoundingBoxImpl
  {
  public:
    ShowMeshBoundingBoxImpl();
    void setSize(int x, int y, int z);
    void setBBox(const BBox& bbox);
    GeometryHandle makeGeometry(const GeometryIDGenerator& id);
  private:
    RenderState getRenderState();
    void addLineToAxis(GlyphGeom& glyphs, const Point& base, const Vector& axis, const ColorRGB& col);
    void addLinesToAxisEachFace(GlyphGeom& glyphs, const Point& base, const Vector& axis,
                                const Vector& dir1, const Vector& dir2, double offset,
                                const ColorRGB& col);
    void addLinesToAxis(GlyphGeom& glyphs, int count, const Point& base, const Vector& axis,
                        const Vector& dir1, const Vector& dir2, const ColorRGB& col);
    BBox bbox_;
    int x_ = 2;
    int y_ = 2;
    int z_ = 2;
  };
}}}

ShowMeshBoundingBoxImpl::ShowMeshBoundingBoxImpl()
{}

void ShowMeshBoundingBoxImpl::setSize(int x, int y, int z)
{
  x_ = x;
  y_ = y;
  z_ = z;
}

void ShowMeshBoundingBoxImpl::setBBox(const BBox& bbox)
{
  bbox_ = bbox;
}

void ShowMeshBoundingBoxImpl::addLineToAxis(GlyphGeom& glyphs, const Point& base, const Vector& axis,
                                            const ColorRGB& col)
{
  glyphs.addLine(base, base + axis, col, col);
}

void ShowMeshBoundingBoxImpl::addLinesToAxisEachFace(GlyphGeom& glyphs, const Point& base,
                                                     const Vector& axis, const Vector& dir1,
                                                     const Vector& dir2, double offset,
                                                     const ColorRGB& col)
{
  Vector off1 = offset * dir1;
  Vector off2 = offset * dir2;
  addLineToAxis(glyphs, base + off1,        axis, col);
  addLineToAxis(glyphs, base + off1 + dir2, axis, col);
  addLineToAxis(glyphs, base + off2,        axis, col);
  addLineToAxis(glyphs, base + off2 + dir1, axis, col);
}

void ShowMeshBoundingBoxImpl::addLinesToAxis(GlyphGeom& glyphs, int count, const Point& base,
                                             const Vector& axis, const Vector& dir1,
                                             const Vector& dir2, const ColorRGB& col)
{
  if (count == 1)
    addLinesToAxisEachFace(glyphs, base, axis, dir1, dir2, 0.5, col);
  else if (count > 1)
  {
    addLineToAxis(glyphs, base, axis, col);
    addLineToAxis(glyphs, base + dir1, axis, col);
    addLineToAxis(glyphs, base + dir2, axis, col);
    addLineToAxis(glyphs, base + dir1 + dir2, axis, col);

    for (int i = 1; i < count-1; ++i)
    {
      double offset = double(i) / double(count-1);
      addLinesToAxisEachFace(glyphs, base, axis, dir1, dir2, offset, col);
    }
  }
}

RenderState ShowMeshBoundingBoxImpl::getRenderState()
{
  RenderState renState;
  renState.set(RenderState::USE_NORMALS, false);
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);
  renState.mGlyphType = RenderState::GlyphType::LINE_GLYPH;
  renState.set(RenderState::USE_DEFAULT_COLOR, false);
  return renState;
}

GeometryHandle ShowMeshBoundingBoxImpl::makeGeometry(const GeometryIDGenerator& idGen)
{
  auto min = bbox_.get_min();
  auto x = Vector(bbox_.x_length(), 0, 0);
  auto y = Vector(0, bbox_.y_length(), 0);
  auto z = Vector(0, 0, bbox_.z_length());

  GlyphGeom glyphs;
  addLinesToAxis(glyphs, x_, min, x, y, z, ColorRGB(1,0,0));
  addLinesToAxis(glyphs, y_, min, y, x, z, ColorRGB(0,1,0));
  addLinesToAxis(glyphs, z_, min, z, x, y, ColorRGB(0,0,1));

  auto geom(boost::make_shared<GeometryObjectSpire>(idGen, "ShowMeshBoundingBox", true));
  glyphs.buildObject(*geom, geom->uniqueID(), false, 1.0, ColorScheme::COLOR_IN_SITU,
                     getRenderState(), SpireIBO::PRIMITIVE::LINES, bbox_, true, nullptr);
  return geom;
}

ShowMeshBoundingBox::ShowMeshBoundingBox() : GeometryGeneratingModule(staticInfo_),
                                             impl_(new ShowMeshBoundingBoxImpl)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputGeom);
}

void ShowMeshBoundingBox::setStateDefaults()
{
  auto state = get_state();
  state->setValue(XSize, 2);
  state->setValue(YSize, 2);
  state->setValue(ZSize, 2);
}

void
ShowMeshBoundingBox::execute()
{
  auto input = getRequiredInput(InputField);
  if (needToExecute())
  {
    if (inputsChanged())
      impl_->setBBox(input->vmesh()->get_bounding_box());

    auto state = get_state();
    impl_->setSize(state->getValue(XSize).toInt(), state->getValue(YSize).toInt(),
                   state->getValue(ZSize).toInt());

    sendOutput(OutputGeom, impl_->makeGeometry(*this));
  }
}

const AlgorithmParameterName ShowMeshBoundingBox::XSize("XSize");
const AlgorithmParameterName ShowMeshBoundingBox::YSize("YSize");
const AlgorithmParameterName ShowMeshBoundingBox::ZSize("ZSize");
