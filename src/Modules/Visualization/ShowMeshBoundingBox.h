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


#ifndef MODULES_VISUALIZATION_SHOW_MESH_BOUNDING_BOX_H
#define MODULES_VISUALIZATION_SHOW_MESH_BOUNDING_BOX_H

#include <Core/GeometryPrimitives/BBox.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Modules/Visualization/share.h>

namespace SCIRun {
namespace Modules {
namespace Visualization {
  class ShowMeshBoundingBoxImpl
  {
  public:
    ShowMeshBoundingBoxImpl();
    void setSize(int x, int y, int z);
    void setBBox(const Core::Geometry::BBox& bbox);
    Graphics::Datatypes::GeometryHandle makeGeomemtry(const Core::GeometryIDGenerator& id);
  private:
    RenderState getRenderState();
    void addLineToAxis(Graphics::GlyphGeom& glyphs, const Core::Geometry::Point& base,
                       const Core::Geometry::Vector& axis, const Core::Datatypes::ColorRGB& col);
    void addLinesToAxisEachFace(Graphics::GlyphGeom& glyphs, const Core::Geometry::Point& base,
                                const Core::Geometry::Vector& axis,
                                const Core::Geometry::Vector& dir1,
                                const Core::Geometry::Vector& dir2, double offset,
                                const Core::Datatypes::ColorRGB& col);
    void addLinesToAxis(Graphics::GlyphGeom& glyphs, int count, const Core::Geometry::Point& base,
                        const Core::Geometry::Vector& axis, const Core::Geometry::Vector& dir1,
                        const Core::Geometry::Vector& dir2, const Core::Datatypes::ColorRGB& col);
    Core::Geometry::BBox bbox_;
    int x_ = 2;
    int y_ = 2;
    int z_ = 2;
  };

  class SCISHARE ShowMeshBoundingBox : public Dataflow::Networks::GeometryGeneratingModule,
    public Has1InputPort<FieldPortTag>, public Has1OutputPort<GeometryPortTag>
  {
  public:
    ShowMeshBoundingBox();
    virtual void execute();
    virtual void setStateDefaults();

    INPUT_PORT(0, InputField, Field);
    OUTPUT_PORT(0, OutputGeom, GeometryObject);

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

    static const Core::Algorithms::AlgorithmParameterName XSize;
    static const Core::Algorithms::AlgorithmParameterName YSize;
    static const Core::Algorithms::AlgorithmParameterName ZSize;
  private:
    ShowMeshBoundingBoxImpl impl_;
  };
}}}

#endif

