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

#include <Core/GeometryPrimitives/Transform.h>
#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Vector.h>

using namespace SCIRun::Core::Geometry;

const static double errorOfMargin = 1.0e-10;

const static double identityMat[] =
  {1,0,0,0,
   0,1,0,0,
   0,0,1,0,
   0,0,0,1};

const static double testMat[] =
  {1.2, 3.1, 7.9, 3.8,
   6.6, 9.5, 2.4, 8.5,
   2.4, 7.4, 5.1, 1.7,
   0.0, 0.0, 0.0, 1.0};
const static std::string expectedStringFromTestMat =
  "[1.2, 6.6, 2.4, 0; 3.1, 9.5, 7.4, 0; 7.9, 2.4, 5.1, 0; 3.8, 8.5, 1.7, 1]";

const static Point testPos(3.5, 2.7, 8.8);
const static Vector testRot[] =
  {Vector(0.5, 0.9, 0.1),
   Vector(0.2, 0.1, 0.8),
   Vector(0.9, 0.3, 0.1)};
const static double expectedMatFromRot[] =
  {0.5, 0.2, 0.9, 0.0,
   0.9, 0.1, 0.3, 0.0,
   0.1, 0.8, 0.1, 0.0,
   0.0, 0.0, 0.0, 1.0};
const static double expectedMatFromPosRot[] =
  {0.5, 0.2, 0.9, 3.5,
   0.9, 0.1, 0.3, 2.7,
   0.1, 0.8, 0.1, 8.8,
   0.0, 0.0, 0.0, 1.0};

void checkTransformEqual(const double *mat, const Transform t)
{
  Transform expected(mat);
  EXPECT_EQ(expected, t);
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

TEST(TransformTests, CanPrint)
{
  Transform t;
  t.print();
}

TEST(TransformTests, StringOut)
{
  Transform t(testMat);
  std::stringstream ss;
  ss << t;
  EXPECT_EQ(expectedStringFromTestMat, ss.str());
}

TEST(TransformTests, StringIn)
{
  Transform t;
  std::stringstream ss(expectedStringFromTestMat);
  ss >> t;
  Transform expectedT(testMat);
  EXPECT_EQ(expectedT, t);
}

TEST(TransformTests, TransformFromString)
{
  Transform t = transformFromString(expectedStringFromTestMat);
  Transform expectedT(testMat);
  EXPECT_EQ(expectedT, t);
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

  EXPECT_NEAR(0.0, Dot(rot[0], rot[1]), errorOfMargin);
  EXPECT_NEAR(0.0, Dot(rot[0], rot[2]), errorOfMargin);
  EXPECT_NEAR(0.0, Dot(rot[1], rot[2]), errorOfMargin);
  for(int i = 0; i < 3; ++i)
    EXPECT_NEAR(testRot[i].length(), rot[i].length(), errorOfMargin);
}

TEST(TransformTests, Orthonormalize)
{
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  t.orthonormalize();
  auto rot = t.get_rotation();

  EXPECT_NEAR(0.0, Dot(rot[0], rot[1]), errorOfMargin);
  EXPECT_NEAR(0.0, Dot(rot[0], rot[2]), errorOfMargin);
  EXPECT_NEAR(0.0, Dot(rot[1], rot[2]), errorOfMargin);
  for(int i = 0; i < 3; ++i)
    EXPECT_NEAR(1.0, rot[i].length(), errorOfMargin);
}

TEST(TransformTests, MultplicationVector)
{
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  auto vec = t * Vector(0.5, 2.0, 1.5);
  EXPECT_NEAR(vec.x(), 2.0, errorOfMargin);
  EXPECT_NEAR(vec.y(), 1.1, errorOfMargin);
  EXPECT_NEAR(vec.z(), 1.8, errorOfMargin);
}

TEST(TransformTests, MultplicationPositionZero)
{
  // Same as vector transformation since point at zero
  Transform t(Point(0,0,0), testRot[0], testRot[1], testRot[2]);
  auto point = t * Point(0.5, 2.0, 1.5);
  EXPECT_NEAR(point.x(), 2.0, errorOfMargin);
  EXPECT_NEAR(point.y(), 1.1, errorOfMargin);
  EXPECT_NEAR(point.z(), 1.8, errorOfMargin);
}

TEST(TransformTests, MultplicationPosition)
{
  Transform t(testPos, testRot[0], testRot[1], testRot[2]);
  auto point = t * Point(0.5, 2.0, 1.5);
  EXPECT_NEAR(point.x(), 5.5, errorOfMargin);
  EXPECT_NEAR(point.y(), 3.8, errorOfMargin);
  EXPECT_NEAR(point.z(), 10.6, errorOfMargin);
}

TEST(TransformTests, MultplicationTransform)
{
  Transform t1(testPos, testRot[0], testRot[1], testRot[2]);
  Transform t2(Point(0,0,0), Vector(1,0,0), Vector(0,2,0), Vector(0,0,3));
  Transform t3 = t2 * t1;
  auto vecs = t3.get_rotation();
  for(int i = 0; i < 3; ++i)
    EXPECT_EQ(vecs[i], Vector(testRot[i].x(), testRot[i].y() * 2, testRot[i].z() * 3));
}
