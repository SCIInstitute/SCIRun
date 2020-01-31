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

double identityMat[] = {1,0,0,0,
                        0,1,0,0,
                        0,0,1,0,
                        0,0,0,1};

const static double testMat[] = {1.2, 3.1, 7.9, 3.8,
                                 6.6, 9.5, 2.4, 8.5,
                                 2.4, 7.4, 5.1, 1.7,
                                 0.0, 0.0, 0.0, 1.0};

Point testPos(3.5, 2.7, 8.8);
Vector testRot[] ={Vector(0.5, 0.9, 0.1),
                   Vector(0.2, 0.1, 0.8),
                   Vector(0.9, 0.3, 0.1)};
double expectedMatFromRot[] = {0.5, 0.2, 0.9, 0.0,
                               0.9, 0.1, 0.3, 0.0,
                               0.1, 0.8, 0.1, 0.0,
                               0.0, 0.0, 0.0, 1.0};
double expectedMatFromPosRot[] = {0.5, 0.2, 0.9, 3.5,
                                  0.9, 0.1, 0.3, 2.7,
                                  0.1, 0.8, 0.1, 8.8,
                                  0.0, 0.0, 0.0, 1.0};

void checkTransformEqual(const double* mat, const Transform t)
{
  Transform expected(mat);
  EXPECT_EQ(expected, t);
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
      EXPECT_EQ(testMat[i*4 + j], t.get_mat_val(i, j));
}

TEST(TransformTests, IdentityConstructor)
{
  Transform t;
  checkTransformEqual(identityMat, t);
  checkTransformEqual(identityMat, Transform::Identity());
}

TEST(TransformTests, VectorConstructor)
{
  double firstVal = expectedMatFromPosRot[0];
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  checkTransformEqual(expectedMatFromPosRot, t);

  // Test that original array is not changed
  t.set_mat_val(0, 0, 3.3);
  EXPECT_EQ(expectedMatFromPosRot[0], firstVal);
}

TEST(TransformTests, CopyConstructor)
{
  Transform t1(testPos, testRot[0], testRot[1], testRot[2]);
  Transform t2(t1);
  checkTransformEqual(expectedMatFromPosRot, t2);
}

TEST(TransformTests, Equals)
{
  Transform t1(testPos, testRot[0], testRot[1], testRot[2]);
  Transform t2 = t1;
  checkTransformEqual(expectedMatFromPosRot, t2);
}

TEST(TransformTests, GetSetMatVal)
{
  Transform t;
  EXPECT_EQ(t.get_mat_val(1, 2), 0);
  EXPECT_EQ(t.get_mat_val(2, 2), 1);
  t.set_mat_val(1, 2, 5);
  EXPECT_EQ(5, t.get_mat_val(1, 2));
}

TEST(TransformTests, LoadBasis)
{
  Transform t;
  t.load_basis(testPos, testRot[0], testRot[1], testRot[2]);
  checkTransformEqual(expectedMatFromPosRot, t);
}

TEST(TransformTests, LoadFrame)
{
  Transform t;
  t.load_frame(testRot[0], testRot[1], testRot[2]);
  checkTransformEqual(expectedMatFromRot, t);
}

TEST(TransformTests, ChangeBasis)
{
  Transform t;
  t.change_basis(testRot[0], testRot[1], testRot[2]);
  checkTransformEqual(expectedMatFromRot, t);
}

TEST(TransformTests, LoadIdentity)
{
  Transform t(testMat);
  t.load_identity();
  checkTransformEqual(identityMat, t);
}

TEST(TransformTests, GetRotation)
{
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  auto rot = t.get_rotation();
  for(int i = 0; i < 3; ++i)
    EXPECT_EQ(testRot[i], rot[i]);
}

TEST(TransformTests, GetPosition)
{
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  auto pos = t.get_translation();
  EXPECT_EQ(testPos, pos);
}

TEST(TransformTests, Orthogonalize)
{
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  t.orthogonalize();
  auto rot = t.get_rotation();

  EXPECT_NEAR(0.0, Dot(rot[0], rot[1]), 1.0e-10);
  EXPECT_NEAR(0.0, Dot(rot[0], rot[2]), 1.0e-10);
  EXPECT_NEAR(0.0, Dot(rot[1], rot[2]), 1.0e-10);
  for(int i = 0; i < 3; ++i)
    EXPECT_NEAR(testRot[i].length(), rot[i].length(), 1.0e-10);
}

TEST(TransformTests, Orthonormalize)
{
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  t.orthonormalize();
  auto rot = t.get_rotation();

  EXPECT_NEAR(0.0, Dot(rot[0], rot[1]), 1.0e-10);
  EXPECT_NEAR(0.0, Dot(rot[0], rot[2]), 1.0e-10);
  EXPECT_NEAR(0.0, Dot(rot[1], rot[2]), 1.0e-10);
  for(int i = 0; i < 3; ++i)
    EXPECT_NEAR(1.0, rot[i].length(), 1.0e-10);
}
