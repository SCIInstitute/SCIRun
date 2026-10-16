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

  VtkGeometryObjectHandle convertPass(const SpireSubPass& pass, std::string& skipReason)
  {
    if (isScreenSpace(pass))
    {
      skipReason = "screen-space text/legend";
      return nullptr;
    }

    const auto& vbo = pass.vbo;
    const auto& ibo = pass.ibo;
    if (!vbo.data || !ibo.data || vbo.data->getBufferSize() == 0 || ibo.data->getBufferSize() == 0)
    {
      skipReason = "empty buffers";
      return nullptr;
    }

    const auto layout = layoutOf(vbo);
    if (!layout.pos || layout.stride == 0 || vbo.data->getBufferSize() % layout.stride != 0)
    {
      skipReason = "unrecognized vertex layout";
      return nullptr;
    }

    const size_t perPrim = verticesPerPrimitive(ibo.prim);
    const size_t numIndices = ibo.indexSize ? ibo.data->getBufferSize() / ibo.indexSize : 0;
    if (!readIndex(ibo.data->getBuffer(), 0, ibo.indexSize) || perPrim == 0 || numIndices < perPrim)
    {
      skipReason = "unrecognized index buffer";
      return nullptr;
    }

    const char* vtx = vbo.data->getBuffer();
    const char* idx = ibo.data->getBuffer();
    const size_t numVerts = vbo.data->getBufferSize() / layout.stride;

    const auto* colorMapTexture = layout.texCoords && pass.texture.name == "ColorMap"
      && pass.texture.width > 0 && pass.texture.bitmap.size() >= 4 * pass.texture.width
      ? &pass.texture : nullptr;

    auto obj = std::make_shared<VtkGeometryObject>();

    // Uniform color mirrors the Phong/Flat shaders: uDiffuseColor, alpha from uTransparency.
    auto color = pass.renderState.defaultColor;
    obj->material.color[0] = static_cast<float>(color.r());
    obj->material.color[1] = static_cast<float>(color.g());
    obj->material.color[2] = static_cast<float>(color.b());
    if (auto diffuse = findUniform(pass, "uDiffuseColor"))
    {
      obj->material.color[0] = diffuse->data.r;
      obj->material.color[1] = diffuse->data.g;
      obj->material.color[2] = diffuse->data.b;
    }
    // Same test as RenderBasicSys: anything else is drawn with blending off, so alpha is moot.
    const auto& rs = pass.renderState;
    const bool transparent = rs.get(RenderState::ActionFlags::USE_TRANSPARENCY)
      || rs.get(RenderState::ActionFlags::USE_TRANSPARENT_EDGES)
      || rs.get(RenderState::ActionFlags::USE_TRANSPARENT_NODES);
    if (transparent && !colorMapTexture)
    {
      if (auto transparency = findUniform(pass, "uTransparency"))
        obj->material.opacity = transparency->data.x;
    }

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToFloat();
    points->SetNumberOfPoints(static_cast<vtkIdType>(numVerts));

    vtkSmartPointer<vtkFloatArray> normals;
    if (layout.normal)
    {
      normals = vtkSmartPointer<vtkFloatArray>::New();
      normals->SetName("Normals");
      normals->SetNumberOfComponents(3);
      normals->SetNumberOfTuples(static_cast<vtkIdType>(numVerts));
    }

    vtkSmartPointer<vtkUnsignedCharArray> colors;
    if (layout.color || colorMapTexture)
    {
      colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
      colors->SetName("Colors");
      colors->SetNumberOfComponents(4);
      colors->SetNumberOfTuples(static_cast<vtkIdType>(numVerts));
    }

    auto toByte = [](float c) { return static_cast<unsigned char>(std::clamp(c, 0.f, 1.f) * 255.f + 0.5f); };

    for (size_t i = 0; i < numVerts; ++i)
    {
      const size_t base = i * layout.stride;
      float p[3];
      for (int k = 0; k < 3; ++k)
        p[k] = readAt<float>(vtx, base + *layout.pos + k * sizeof(float));
      points->SetPoint(static_cast<vtkIdType>(i), p);
      obj->box.extend(Point(p[0], p[1], p[2]));

      if (normals)
      {
        float n[3];
        for (int k = 0; k < 3; ++k)
          n[k] = readAt<float>(vtx, base + *layout.normal + k * sizeof(float));
        normals->SetTypedTuple(static_cast<vtkIdType>(i), n);
      }

      if (!colors) continue;
      unsigned char rgba[4];
      if (colorMapTexture)
      {
        // Front faces sample .y (Phong_ColorMap.fs); x and y only differ on two-sided cell data.
        const float t = readAt<float>(vtx, base + *layout.texCoords + sizeof(float));
        const size_t width = colorMapTexture->width;
        const size_t texel = std::min(width - 1, static_cast<size_t>(std::max(0.f, t) * width));
        std::copy_n(&colorMapTexture->bitmap[4 * texel], 4, rgba);
        // The colormap shaders take alpha from the texture alone.
        if (!transparent)
          rgba[3] = 255;
      }
      else
      {
        for (int k = 0; k < 3; ++k)
          rgba[k] = toByte(readAt<float>(vtx, base + *layout.color + k * sizeof(float)));
        // Flat_Color/Phong_Color ignore vertex alpha in favor of uTransparency.
        rgba[3] = toByte(obj->material.opacity);
      }
      colors->SetTypedTuple(static_cast<vtkIdType>(i), rgba);
    }

    auto cells = vtkSmartPointer<vtkCellArray>::New();
    cells->AllocateEstimate(static_cast<vtkIdType>(numIndices / perPrim), static_cast<vtkIdType>(perPrim));
    std::vector<vtkIdType> cell(perPrim);
    size_t badIndices = 0;
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

    if (cells->GetNumberOfCells() == 0)
    {
      skipReason = "no valid primitives";
      return nullptr;
    }

    auto poly = vtkSmartPointer<vtkPolyData>::New();
    poly->SetPoints(points);
    if (normals) poly->GetPointData()->SetNormals(normals);
    if (colors) poly->GetPointData()->SetScalars(colors);

    switch (ibo.prim)
    {
    case SpireIBO::PRIMITIVE::POINTS: poly->SetVerts(cells); break;
    case SpireIBO::PRIMITIVE::LINES: poly->SetLines(cells); break;
    case SpireIBO::PRIMITIVE::TRIANGLES:
    case SpireIBO::PRIMITIVE::QUADS: poly->SetPolys(cells); break;
    }

    // Points and lines stay NO_TYPE: SPHERE/EDGE would make VtkRenderer swap in glyphs and
    // tubes, but these were already tessellated (or drawn as GL points/lines) upstream.
    if (ibo.prim == SpireIBO::PRIMITIVE::TRIANGLES)
      obj->type = GeometryType::TRI_SURFACE;
    else if (ibo.prim == SpireIBO::PRIMITIVE::QUADS)
      obj->type = GeometryType::QUAD_SURFACE;

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
      auto obj = convertPass(pass, reason);
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
