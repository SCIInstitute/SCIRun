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

#include <gtest/gtest.h>

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Widgets/BoundingBoxWidget.h>
#include <Graphics/Widgets/SphereWidget.h>
#include <Graphics/Widgets/SuperquadricWidget.h>
#include <Graphics/Widgets/Tests/WidgetTestingUtility.h>
#include <cmath>
#include <cstring>

using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Geometry;

namespace
{
  struct GlyphMesh
  {
    std::vector<Vector> points;
    std::vector<Vector> normals;
    std::vector<uint32_t> indices;

    Vector centroid() const
    {
      Vector sum(0, 0, 0);
      for (const auto& p : points) sum += p;
      return sum / static_cast<double>(points.size());
    }

    double extentAlong(const Vector& axis) const
    {
      const auto c = centroid();
      double extent = 0;
      for (const auto& p : points) extent = std::max(extent, std::abs(Dot(p - c, axis)));
      return extent;
    }

    // +1 or -1 when every triangle's winding agrees with its vertex normals the same way; 0 if mixed.
    int windingSign() const
    {
      int sign = 0;
      for (size_t t = 0; t + 2 < indices.size(); t += 3)
      {
        const auto& p0 = points[indices[t]];
        auto faceNormal = Cross(points[indices[t + 1]] - p0, points[indices[t + 2]] - p0);
        if (faceNormal.length() < 1e-9) continue;
        auto vertexNormal = normals[indices[t]] + normals[indices[t + 1]] + normals[indices[t + 2]];
        const int s = Dot(faceNormal, vertexNormal) > 0 ? 1 : -1;
        if (sign == 0) sign = s;
        else if (s != sign) return 0;
      }
      return sign;
    }
  };

  GlyphMesh readMesh(const GeometryObjectSpire& geom)
  {
    GlyphMesh mesh;
    const auto& vbo = geom.vbos().front();
    size_t stride = 0, normalOffset = 0;
    for (const auto& attrib : vbo.attributes)
    {
      if (attrib.name == "aNormal") normalOffset = stride;
      stride += attrib.sizeInBytes;
    }

    const char* buffer = vbo.data->getBuffer();
    for (size_t offset = 0; offset + stride <= vbo.data->getBufferSize(); offset += stride)
    {
      float pos[3], normal[3];
      std::memcpy(pos, buffer + offset, sizeof(pos));
      std::memcpy(normal, buffer + offset + normalOffset, sizeof(normal));
      mesh.points.emplace_back(pos[0], pos[1], pos[2]);
      mesh.normals.emplace_back(normal[0], normal[1], normal[2]);
    }

    const auto& ibo = geom.ibos().front();
    mesh.indices.resize(ibo.data->getBufferSize() / sizeof(uint32_t));
    std::memcpy(mesh.indices.data(), ibo.data->getBuffer(), mesh.indices.size() * sizeof(uint32_t));
    return mesh;
  }

  template <class Subwidget>
  std::vector<GlyphMesh> meshesOf(const std::vector<Vector>& scaledEigvecs)
  {
    StubGeometryIDGenerator idGen;
    BoundingBoxWidget box({{idGen, "box"}, makeShared<RealGlyphFactory>()},
      {{1.0, "", {0, 0, 0}, {{-10, -10, -10}, {10, 10, 10}}, 10}, {{0, 0, 0}, scaledEigvecs}});

    std::vector<GlyphMesh> meshes;
    for (auto w = box.subwidgetBegin(); w != box.subwidgetEnd(); ++w)
      if (auto sub = std::dynamic_pointer_cast<Subwidget>(*w))
        meshes.push_back(readMesh(*sub));
    return meshes;
  }

  std::vector<GlyphMesh> cornerMeshes(const std::vector<Vector>& scaledEigvecs)
  {
    return meshesOf<SuperquadricWidget>(scaledEigvecs);
  }

  const double s = std::sqrt(0.5);
  const Vector x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
  const Vector rx(s, s, 0), ry(-s, s, 0);
}

TEST(BoundingBoxWidgetTest, CornersAreAlignedWithRotatedBox)
{
  const auto reference = cornerMeshes({2 * x, 3 * y, 4 * z});
  const auto rotated = cornerMeshes({2 * rx, 3 * ry, 4 * z});
  ASSERT_EQ(8, reference.size());
  ASSERT_EQ(8, rotated.size());

  // The glyph's tessellation isn't symmetric across axes, so compare axis by axis.
  const auto tol = 1e-3 * reference[0].extentAlong(x);
  for (const auto& corner : rotated)
  {
    EXPECT_NEAR(reference[0].extentAlong(x), corner.extentAlong(rx), tol);
    EXPECT_NEAR(reference[0].extentAlong(y), corner.extentAlong(ry), tol);
    EXPECT_NEAR(reference[0].extentAlong(z), corner.extentAlong(z), tol);
  }
}

// A mirrored corner flips its winding, and the shader then lights it inside-out.
TEST(BoundingBoxWidgetTest, CornerWindingMatchesFaceSpheres)
{
  const auto expected = meshesOf<SphereWidget>({2 * x, 3 * y, 4 * z})[0].windingSign();
  ASSERT_NE(0, expected);

  // The last frame is left-handed; the corners must still come out right-handed.
  for (const auto& frame : {std::vector<Vector>{2 * x, 3 * y, 4 * z},
                            std::vector<Vector>{2 * rx, 3 * ry, 4 * z},
                            std::vector<Vector>{2 * rx, 3 * ry, -4 * z}})
    for (const auto& corner : cornerMeshes(frame))
      EXPECT_EQ(expected, corner.windingSign());
}
