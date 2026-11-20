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
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;

class ImageMeshTests : public ::testing::Test
{
protected:
  void SetUp() override
  {
    // 3x3 nodes over [0,2]x[0,2] in the z=1 plane: 2x2 unit cells.
    FieldInformation fi("ImageMesh", 1, "double");
    mesh_ = CreateMesh(fi, 3, 3, Point(0, 0, 1), Point(2, 2, 1));
    mesh_->vmesh()->synchronize(Mesh::FIND_CLOSEST_ELEM_E);
  }

  Point closestPoint(const Point& p, VMesh::Elem::index_type& elem) const
  {
    Point result;
    elem = 0;
    EXPECT_TRUE(mesh_->vmesh()->find_closest_elem(result, elem, p));
    return result;
  }

  MeshHandle mesh_;
};

TEST_F(ImageMeshTests, ClosestElemProjectsOntoPlane)
{
  VMesh::Elem::index_type elem;
  auto r = closestPoint(Point(0.5, 0.5, 4.0), elem);
  EXPECT_NEAR(0.5, r.x(), 1e-12);
  EXPECT_NEAR(0.5, r.y(), 1e-12);
  EXPECT_NEAR(1.0, r.z(), 1e-12);
  EXPECT_EQ(0, elem);
}

TEST_F(ImageMeshTests, ClosestElemReachesLastRowOfCells)
{
  VMesh::Elem::index_type elem;
  auto r = closestPoint(Point(1.5, 1.5, -3.0), elem);
  EXPECT_NEAR(1.5, r.x(), 1e-12);
  EXPECT_NEAR(1.5, r.y(), 1e-12);
  EXPECT_NEAR(1.0, r.z(), 1e-12);
  EXPECT_EQ(3, elem);
}

TEST_F(ImageMeshTests, ClosestElemClampsToFarCorner)
{
  VMesh::Elem::index_type elem;
  auto r = closestPoint(Point(5, 5, 1), elem);
  EXPECT_NEAR(2.0, r.x(), 1e-12);
  EXPECT_NEAR(2.0, r.y(), 1e-12);
  EXPECT_EQ(3, elem);
}
