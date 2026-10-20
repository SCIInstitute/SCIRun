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



#ifdef WITH_VTK
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkUnsignedCharArray.h>
#endif

#include <Core/Algorithms/Visualization/ConvertGeometryToVtkAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/VTK/VtkGeometry.h>
#include <Graphics/Datatypes/GeometryImpl.h>

#include <algorithm>
#include <atomic>
#include <cstring>
#include <optional>

using namespace SCIRun;
using namespace Core::Algorithms;
using namespace Core::Algorithms::Visualization;
using namespace Core::Datatypes;
using namespace Core::Geometry;
using namespace Graphics::Datatypes;

const AlgorithmInputName ConvertGeometryToVtkAlgo::Geometry("Geometry");
const AlgorithmOutputName ConvertGeometryToVtkAlgo::VtkGeometry("VtkGeometry");

#ifdef WITH_VTK
namespace
{
  // Composites only expose their children through addToList, one level at a time.
  void flatten(const GeometryBaseHandle& geom, std::vector<GeometryHandle>& out)
  {
    GeomList children;
    geom->addToList(geom, children);
    if (children.size() == 1 && *children.begin() == geom)
    {
      if (auto spire = std::dynamic_pointer_cast<GeometryObjectSpire>(geom))
        out.push_back(spire);
      return;
    }
    for (const auto& child : children)
      flatten(child, out);
  }

  struct VertexLayout
  {
    size_t stride{0};
    std::optional<size_t> pos, normal, color, texCoords;
  };

  VertexLayout layoutOf(const SpireVBO& vbo)
  {
    VertexLayout layout;
    for (const auto& attrib : vbo.attributes)
    {
      if (attrib.name == "aPos" && attrib.sizeInBytes == 3 * sizeof(float))
        layout.pos = layout.stride;
      else if (attrib.name == "aNormal" && attrib.sizeInBytes == 3 * sizeof(float))
        layout.normal = layout.stride;
      else if (attrib.name == "aColor" && attrib.sizeInBytes == 4 * sizeof(float))
        layout.color = layout.stride;
      else if (attrib.name == "aTexCoords" && attrib.sizeInBytes == 2 * sizeof(float))
        layout.texCoords = layout.stride;
      layout.stride += attrib.sizeInBytes;
    }
    return layout;
  }

  // VarBuffer data is packed with no alignment guarantee.
  template <typename T>
  T readAt(const char* buffer, size_t offset)
  {
    T value;
    std::memcpy(&value, buffer + offset, sizeof(T));
    return value;
  }

  std::optional<uint32_t> readIndex(const char* buffer, size_t i, size_t indexSize)
  {
    switch (indexSize)
    {
    case 1: return readAt<uint8_t>(buffer, i);
    case 2: return readAt<uint16_t>(buffer, 2 * i);
    case 4: return readAt<uint32_t>(buffer, 4 * i);
    default: return std::nullopt;
    }
  }

  const SpireSubPass::Uniform* findUniform(const SpireSubPass& pass, const std::string& name)
  {
    auto it = std::find_if(pass.mUniforms.begin(), pass.mUniforms.end(),
      [&name](const SpireSubPass::Uniform& u) { return u.name == name; });
    return it == pass.mUniforms.end() ? nullptr : &*it;
  }

  // Text and legends are drawn in screen space; their aPos is not a world position.
  bool isScreenSpace(const SpireSubPass& pass)
  {
    return pass.renderState.get(RenderState::ActionFlags::IS_TEXT)
      || pass.programName == "Shaders/ColorMapLegend"
      || pass.programName == "Shaders/TextBuilder";
  }

  size_t verticesPerPrimitive(SpireIBO::PRIMITIVE prim)
  {
    switch (prim)
    {
    case SpireIBO::PRIMITIVE::POINTS: return 1;
    case SpireIBO::PRIMITIVE::LINES: return 2;
    case SpireIBO::PRIMITIVE::TRIANGLES: return 3;
    case SpireIBO::PRIMITIVE::QUADS: return 4;
    }
    return 0;
  }

  // Same test as RenderBasicSys: anything else is drawn with blending off, so alpha is moot.
  bool isTransparent(const RenderState& rs)
  {
    return rs.get(RenderState::ActionFlags::USE_TRANSPARENCY)
      || rs.get(RenderState::ActionFlags::USE_TRANSPARENT_EDGES)
      || rs.get(RenderState::ActionFlags::USE_TRANSPARENT_NODES);
  }

  const SpireTexture2D* colorMapTextureOf(const SpireSubPass& pass, const VertexLayout& layout)
  {
    const auto& tex = pass.texture;
    const bool usable = layout.texCoords && tex.name == "ColorMap" && tex.width > 0 && tex.bitmap.size() >= 4 * tex.width;
    return usable ? &tex : nullptr;
  }

  std::optional<std::string> whyUnconvertible(const SpireSubPass& pass, const VertexLayout& layout)
  {
    const auto& vbo = pass.vbo;
    const auto& ibo = pass.ibo;
    if (isScreenSpace(pass))
      return "screen-space text/legend";
    if (!vbo.data || !ibo.data || vbo.data->getBufferSize() == 0 || ibo.data->getBufferSize() == 0)
      return "empty buffers";
    if (!layout.pos || layout.stride == 0 || vbo.data->getBufferSize() % layout.stride != 0)
      return "unrecognized vertex layout";
    const size_t perPrim = verticesPerPrimitive(ibo.prim);
    const size_t numIndices = ibo.indexSize ? ibo.data->getBufferSize() / ibo.indexSize : 0;
    if (!readIndex(ibo.data->getBuffer(), 0, ibo.indexSize) || perPrim == 0 || numIndices < perPrim)
      return "unrecognized index buffer";
    return std::nullopt;
  }

  // Uniform color mirrors the Phong/Flat shaders: uDiffuseColor, alpha from uTransparency.
  VtkGeometryObject::Material materialOf(const SpireSubPass& pass, bool transparent, bool colorMapped)
  {
    VtkGeometryObject::Material material;
    const auto& color = pass.renderState.defaultColor;
    material.color[0] = static_cast<float>(color.r());
    material.color[1] = static_cast<float>(color.g());
    material.color[2] = static_cast<float>(color.b());
    if (auto diffuse = findUniform(pass, "uDiffuseColor"))
    {
      material.color[0] = diffuse->data.r;
      material.color[1] = diffuse->data.g;
      material.color[2] = diffuse->data.b;
    }
    // The colormap shaders take alpha from the texture alone.
    if (transparent && !colorMapped)
    {
      if (auto transparency = findUniform(pass, "uTransparency"))
        material.opacity = transparency->data.x;
    }
    return material;
  }

  unsigned char toByte(float c)
  {
    return static_cast<unsigned char>(std::clamp(c, 0.f, 1.f) * 255.f + 0.5f);
  }

  struct VertexDecoder
  {
    const char* buffer;
    const VertexLayout& layout;
    const SpireTexture2D* colorMap;
    bool transparent;
    float opacity;

    void vec3(size_t vertex, size_t offset, float out[3]) const
    {
      for (int k = 0; k < 3; ++k)
        out[k] = readAt<float>(buffer, vertex * layout.stride + offset + k * sizeof(float));
    }

    void color(size_t vertex, unsigned char rgba[4]) const
    {
      if (colorMap)
      {
        // Front faces sample .y (Phong_ColorMap.fs); x and y only differ on two-sided cell data.
        const float t = readAt<float>(buffer, vertex * layout.stride + *layout.texCoords + sizeof(float));
        const size_t width = colorMap->width;
        const size_t texel = std::min(width - 1, static_cast<size_t>(std::max(0.f, t) * width));
        std::copy_n(&colorMap->bitmap[4 * texel], 4, rgba);
        if (!transparent)
          rgba[3] = 255;
        return;
      }
      float c[3];
      vec3(vertex, *layout.color, c);
      for (int k = 0; k < 3; ++k)
        rgba[k] = toByte(c[k]);
      // Flat_Color/Phong_Color ignore vertex alpha in favor of uTransparency.
      rgba[3] = toByte(opacity);
    }
  };

  vtkSmartPointer<vtkPolyData> decodeVertices(const VertexDecoder& decoder, size_t numVerts, BBox& box)
  {
    const auto n = static_cast<vtkIdType>(numVerts);
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToFloat();
    points->SetNumberOfPoints(n);

    vtkSmartPointer<vtkFloatArray> normals;
    if (decoder.layout.normal)
    {
      normals = vtkSmartPointer<vtkFloatArray>::New();
      normals->SetName("Normals");
      normals->SetNumberOfComponents(3);
      normals->SetNumberOfTuples(n);
    }

    vtkSmartPointer<vtkUnsignedCharArray> colors;
    if (decoder.layout.color || decoder.colorMap)
    {
      colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
      colors->SetName("Colors");
      colors->SetNumberOfComponents(4);
      colors->SetNumberOfTuples(n);
    }

    for (vtkIdType i = 0; i < n; ++i)
    {
      float v[3];
      decoder.vec3(i, *decoder.layout.pos, v);
      points->SetPoint(i, v);
      box.extend(Point(v[0], v[1], v[2]));
      if (normals)
      {
        decoder.vec3(i, *decoder.layout.normal, v);
        normals->SetTypedTuple(i, v);
      }
      if (colors)
      {
        unsigned char rgba[4];
        decoder.color(i, rgba);
        colors->SetTypedTuple(i, rgba);
      }
    }

    auto poly = vtkSmartPointer<vtkPolyData>::New();
    poly->SetPoints(points);
    if (normals) poly->GetPointData()->SetNormals(normals);
    if (colors) poly->GetPointData()->SetScalars(colors);
    return poly;
  }

  vtkSmartPointer<vtkCellArray> decodeCells(const SpireIBO& ibo, size_t numVerts, size_t& badIndices)
  {
    const size_t perPrim = verticesPerPrimitive(ibo.prim);
    const size_t numIndices = ibo.data->getBufferSize() / ibo.indexSize;
    const char* idx = ibo.data->getBuffer();

    auto cells = vtkSmartPointer<vtkCellArray>::New();
    cells->AllocateEstimate(static_cast<vtkIdType>(numIndices / perPrim), static_cast<vtkIdType>(perPrim));
    std::vector<vtkIdType> cell(perPrim);
    for (size_t i = 0; i + perPrim <= numIndices; i += perPrim)
    {
      bool valid = true;
      for (size_t k = 0; k < perPrim; ++k)
      {
        const auto index = *readIndex(idx, i + k, ibo.indexSize);
        valid = valid && index < numVerts;
        cell[k] = static_cast<vtkIdType>(index);
      }
      if (valid)
        cells->InsertNextCell(static_cast<vtkIdType>(perPrim), cell.data());
      else
        ++badIndices;
    }
    return cells;
  }

  void attachCells(vtkPolyData* poly, SpireIBO::PRIMITIVE prim, vtkCellArray* cells)
  {
    switch (prim)
    {
    case SpireIBO::PRIMITIVE::POINTS: poly->SetVerts(cells); break;
    case SpireIBO::PRIMITIVE::LINES: poly->SetLines(cells); break;
    case SpireIBO::PRIMITIVE::TRIANGLES:
    case SpireIBO::PRIMITIVE::QUADS: poly->SetPolys(cells); break;
    }
  }

  // Points and lines stay NO_TYPE: SPHERE/EDGE would make VtkRenderer swap in glyphs and
  // tubes, but these were already tessellated (or drawn as GL points/lines) upstream.
  GeometryType geometryTypeOf(SpireIBO::PRIMITIVE prim)
  {
    switch (prim)
    {
    case SpireIBO::PRIMITIVE::TRIANGLES: return GeometryType::TRI_SURFACE;
    case SpireIBO::PRIMITIVE::QUADS: return GeometryType::QUAD_SURFACE;
    default: return GeometryType::NO_TYPE;
    }
  }

  VtkGeometryObjectHandle convertPass(
      const GeometryHandle& leaf,
      const SpireSubPass& pass, std::string& skipReason)
  {
    const auto layout = layoutOf(pass.vbo);
    if (auto reason = whyUnconvertible(pass, layout))
    {
      skipReason = *reason;
      return nullptr;
    }

    const auto* colorMap = colorMapTextureOf(pass, layout);
    const bool transparent = isTransparent(pass.renderState);
    auto obj = std::make_shared<VtkGeometryObject>();
    obj->setObjectName(leaf->uniqueID());
    obj->material = materialOf(pass, transparent, colorMap != nullptr);

    const size_t numVerts = pass.vbo.data->getBufferSize() / layout.stride;
    const VertexDecoder decoder{ pass.vbo.data->getBuffer(), layout, colorMap, transparent, obj->material.opacity };
    auto poly = decodeVertices(decoder, numVerts, obj->box);

    size_t badIndices = 0;
    auto cells = decodeCells(pass.ibo, numVerts, badIndices);
    if (cells->GetNumberOfCells() == 0)
    {
      skipReason = "no valid primitives";
      return nullptr;
    }
    attachCells(poly, pass.ibo.prim, cells);

    obj->type = geometryTypeOf(pass.ibo.prim);
    obj->tfn.colors = { obj->material.color[0], obj->material.color[1], obj->material.color[2] };
    obj->tfn.opacities = { obj->material.opacity };
    obj->tfn.range = { 0.f, 1.f };
    obj->dataObject = poly;

    if (badIndices > 0)
      skipReason = std::to_string(badIndices) + " primitive(s) with out-of-range indices dropped";
    return obj;
  }
}
#endif

AlgorithmOutput ConvertGeometryToVtkAlgo::run([[maybe_unused]] const AlgorithmInput& input) const
{
#ifndef WITH_VTK
  THROW_ALGORITHM_PROCESSING_ERROR("ConvertGeometryToVtk requires a WITH_VTK build.");
#else
  auto geom = input.get<GeometryObject>(Geometry);
  ENSURE_ALGORITHM_INPUT_NOT_NULL(geom, "Geometry input");

  std::vector<GeometryHandle> leaves;
  flatten(geom, leaves);

  static std::atomic<uint64_t> versionNumber{0};
  std::vector<VtkGeometryObjectHandle> converted;
  size_t skipped = 0;
  for (const auto& leaf : leaves)
  {
    for (const auto& pass : leaf->passes())
    {
      std::string reason;
      auto obj = convertPass(leaf, pass, reason);
      if (obj)
      {
        obj->version = ++versionNumber;
        converted.push_back(obj);
      }
      else
        ++skipped;
      if (!reason.empty())
        remark("Pass " + pass.passName + ": " + (obj ? reason : "skipped, " + reason));
    }
  }

  if (converted.empty())
    THROW_ALGORITHM_INPUT_ERROR("No convertible render passes in geometry input.");
  if (skipped > 0)
    warning(std::to_string(skipped) + " render pass(es) could not be converted and were skipped.");

  AlgorithmOutput output;
  if (converted.size() == 1)
    output[VtkGeometry] = converted.front();
  else
    output[VtkGeometry] = std::make_shared<CompositeVtkGeometryObject>(converted);
  return output;
#endif
}
