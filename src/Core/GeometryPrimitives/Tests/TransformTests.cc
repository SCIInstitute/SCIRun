/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun::Core::Geometry;

const static double testMat[] = {1.2, 3.1, 7.9, 3.8,
                                 6.6, 9.5, 2.4, 8.5,
                                 2.4, 7.4, 5.1, 1.7,
                                 0.0, 0.0, 0.0, 1.0};

Point testPos(3.5, 2.7, 8.8);
Vector testRot[] ={Vector(0.5, 0.9, 0.1),
                   Vector(0.2, 0.1, 0.8),
                   Vector(0.9, 0.3, 0.1)};

void checkTransformEqual(const Transform t, const double* mat)
{
  Transform expected(mat);
  EXPECT_EQ(t, expected);
}

TEST(TransformTests, CanPrint)
{
  Transform t;
  t.print();
}

TEST(TransformTests, ArrayConstructor)
{
  Transform t(testMat);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      EXPECT_EQ(t.get_mat_val(i, j), testMat[i*4 + j]);
}

TEST(TransformTests, IdentityConstructor)
{
  Transform t;
  double expectedMat[] = {1,0,0,0,
                          0,1,0,0,
                          0,0,1,0,
                          0,0,0,1};
  checkTransformEqual(t, expectedMat);
  checkTransformEqual(Transform::Identity(), expectedMat);
}

TEST(TransformTests, VectorConstructor)
{
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  double expectedMat[] = {0.5, 0.2, 0.9, 3.5,
                          0.9, 0.1, 0.3, 2.7,
                          0.1, 0.8, 0.1, 8.8,
                          0.0, 0.0, 0.0, 1.0};
  checkTransformEqual(t, expectedMat);
}

TEST(TransformTests, GetSetMatVal)
{
  Transform t;
  EXPECT_EQ(t.get_mat_val(2, 3), 0);
  EXPECT_EQ(t.get_mat_val(3, 3), 1);
  t.set_mat_val(2, 3, 5);
  EXPECT_EQ(t.get_mat_val(2, 3), 5);
}

TEST(TransformTests, LoadBasis)
{
  Transform t;
  t.load_basis(testPos, testRot[0], testRot[1], testRot[2]);
}
