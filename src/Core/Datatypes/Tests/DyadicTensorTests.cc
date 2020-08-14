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

#include <Core/Datatypes/Dyadic3DTensor.h>
#include <Core/Datatypes/TensorFwd.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include "Core/Utils/Exception.h"

using namespace SCIRun;
using namespace Core::Datatypes;
using namespace ::testing;

std::vector<std::vector<double>> stdEigvecs = {{1.6, 0.9, 4.3}, {4.0, 6.4, 7}, {6, 34, 1}};
const std::string eigvecsString = "[1.6 0.9 4.3 4 6.4 7 6 34 1]";

std::vector<Eigen::Vector3d> getEigvecs()
{
  std::vector<Eigen::Vector3d> eigvecs(3);
  for (int i = 0; i < 3; ++i)
    eigvecs[i] = Eigen::Map<Eigen::Vector3d>(stdEigvecs[i].data());
  return eigvecs;
}

TEST(Dyadic3DTensorTest, ConstructTensorWithEigenColumnMatrices1)
{
  Dyadic3DTensor t(getEigvecs());
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, ConstructTensorWithEigenColumnMatrices2)
{
  auto eigvecs = getEigvecs();
  Dyadic3DTensor t(eigvecs[0], eigvecs[1], eigvecs[2]);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, ConstructTensorWithDoubles)
{
  Dyadic3DTensor t(1, 2, 3, 4, 5, 6);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(Dyadic3DTensorTest, CanConstructWithColumnMatrixOfSixValues)
{
  Eigen::Matrix<double, 6, 1> vec;
  vec << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0;
  Dyadic3DTensor t = symmetricTensorFromSixElementArray(vec);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(Dyadic3DTensorTest, CannotConstructWithColumnMatrixOfFiveValues)
{
  Eigen::Matrix<double, 5, 1> vec;
  vec << 1.0, 2.0, 3.0, 4.0, 5.0;
  ASSERT_ANY_THROW(symmetricTensorFromSixElementArray(vec));
}

TEST(Dyadic3DTensorTest, CanConstructWithVectorOfSixValues)
{
  std::vector<double> list = {1, 2, 3, 4, 5, 6};
  Dyadic3DTensor t = symmetricTensorFromSixElementArray(list);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(Dyadic3DTensorTest, CanConstructWithInitializerListOfSixValues)
{
  Dyadic3DTensor t = symmetricTensorFromSixElementArray({1, 2, 3, 4, 5, 6});
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithLessThanThreeEigenEigenvectors)
{
  auto eigvecs = getEigvecs();
  ASSERT_ANY_THROW(Dyadic3DTensor t({eigvecs[0], eigvecs[1]}));
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithMoreThanThreeEigenEigenvectors)
{
  auto eigvecs = getEigvecs();
  ASSERT_ANY_THROW(Dyadic3DTensor t({eigvecs[0], eigvecs[1], eigvecs[2], eigvecs[0]}));
}

TEST(DyadicTensorTest, MoveConstructor)
{
  Dyadic3DTensor t(getEigvecs());
  Dyadic3DTensor t2(std::move(t));
  std::stringstream ss;
  ss << t2;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, LinearCertainty)
{
  Dyadic3DTensor t(
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 5, 0}), Eigen::Vector3d({0, 0, 2}));
  ASSERT_EQ(3.0 / 8.0, t.linearCertainty());
}

TEST(Dyadic3DTensorTest, PlanarCertainty)
{
  Dyadic3DTensor t(
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 5, 0}), Eigen::Vector3d({0, 0, 2}));
  ASSERT_EQ(1.0 / 4.0, t.planarCertainty());
}

TEST(Dyadic3DTensorTest, SphericalCertainty)
{
  Dyadic3DTensor t(
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 5, 0}), Eigen::Vector3d({0, 0, 2}));
  ASSERT_EQ(3.0 / 8.0, t.sphericalCertainty());
}

TEST(Dyadic3DTensorTest, CertaintySum)
{
  Dyadic3DTensor t(
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 5, 0}), Eigen::Vector3d({0, 0, 2}));
  ASSERT_EQ(1.0, t.linearCertainty() + t.planarCertainty() + t.sphericalCertainty());
}

TEST(DyadicTensorTest, CanConstructWithMatrix)
{
  auto mat = Eigen::Matrix3d();
  mat(0, 0) = 1;
  mat(1, 1) = 4;
  mat(2, 2) = 6;
  mat(0, 1) = mat(1, 0) = 2;
  mat(0, 2) = mat(2, 0) = 3;
  mat(1, 2) = mat(2, 1) = 5;

  DyadicTensorGeneric<double, 3> t(mat);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(DyadicTensorTest, ConstructTensorWithEigenVectors)
{
  std::vector<Eigen::Vector4d> eigvecs = {
      Eigen::Vector4d(), Eigen::Vector4d(), Eigen::Vector4d(), Eigen::Vector4d()};
  int n = 0;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      eigvecs[i][j] = ++n;
  Dyadic4DTensor t(eigvecs);
  std::stringstream ss;
  ss << t;

  ASSERT_EQ("[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16]", ss.str());
}

TEST(DyadicTensorTest, StringConversion)
{
  std::vector<Eigen::Vector3d> vecs = {
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 2, 0}), Eigen::Vector3d({0, 0, 3})};
  Dyadic3DTensor t(getEigvecs());
  DyadicTensorGeneric<double, 3> t2(vecs);

  std::stringstream ss;
  ss << t;
  ss >> t2;
  ASSERT_TRUE(t == t2);
}

TEST(DyadicTensorTest, GetEigenvalues)
{
  std::vector<Eigen::Vector3d> vecs = {
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 2, 0}), Eigen::Vector3d({0, 0, 3})};
  Dyadic3DTensor t(vecs);
  auto eigvals = t.getEigenvalues();
  ASSERT_EQ(3.0, eigvals[0]);
  ASSERT_EQ(2.0, eigvals[1]);
  ASSERT_EQ(1.0, eigvals[2]);
}

TEST(DyadicTensorTest, GetEigenvectors)
{
  std::vector<Eigen::Vector3d> vecs = {
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 2, 0}), Eigen::Vector3d({0, 0, 3})};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      vecs[i][j] = (i == j) ? i + 1 : 0;

  Dyadic3DTensor t(vecs);
  auto eigvecs = t.getEigenvectors();

  ASSERT_EQ(Eigen::Vector3d({0, 0, 1}), eigvecs[0]);
  ASSERT_EQ(Eigen::Vector3d({0, 1, 0}), eigvecs[1]);
  ASSERT_EQ(Eigen::Vector3d({1, 0, 0}), eigvecs[2]);
}

TEST(DyadicTensorTest, GetEigenvector)
{
  std::vector<Eigen::Vector3d> vecs = {
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 2, 0}), Eigen::Vector3d({0, 0, 3})};
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      vecs[i][j] = (i == j) ? i + 1 : 0;

  Dyadic3DTensor t(vecs);

  ASSERT_EQ(Eigen::Vector3d({0, 1, 0}), t.getEigenvector(1));
}

TEST(DyadicTensorTest, Equivalent)
{
  std::vector<Eigen::Vector3d> vecs = {
      Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 2, 0}), Eigen::Vector3d({0, 0, 3})};
  Dyadic3DTensor t(getEigvecs());
  DyadicTensorGeneric<double, 3> t2(vecs);
  auto t3 = DyadicTensorGeneric<double, 3>();

  ASSERT_TRUE(t != t2);
  ASSERT_TRUE(t2 != t);
  ASSERT_TRUE(t != t2);
  ASSERT_TRUE(t2 != t);
  ASSERT_TRUE(t == t);
  ASSERT_TRUE(t2 == t2);
  ASSERT_TRUE(t2 != t3);
}

TEST(DyadicTensorTest, DifferentDimensionsNotEquivalent)
{
  Dyadic3DTensor t(
      {Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 1, 0}), Eigen::Vector3d({0, 0, 1})});
  Dyadic4DTensor t2({Eigen::Vector4d({1, 0, 0, 0}), Eigen::Vector4d({0, 1, 0, 0}),
      Eigen::Vector4d({0, 0, 1, 0}), Eigen::Vector4d({0, 0, 0, 1})});

  ASSERT_DEATH(t != t2, "");
}

TEST(DyadicTensorTest, EqualsOperatorTensor)
{
  Dyadic3DTensor t(
      {Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 1, 0}), Eigen::Vector3d({0, 0, 1})});
  Dyadic3DTensor t2 = t;

  ASSERT_TRUE(t == t2);

  t2(1, 1) = 3;

  ASSERT_TRUE(t != t2);
}

TEST(DyadicTensorTest, EqualsOperatorDouble)
{
  Dyadic3DTensor t(
      {Eigen::Vector3d({1, 0, 0}), Eigen::Vector3d({0, 1, 0}), Eigen::Vector3d({0, 0, 1})});
  Dyadic3DTensor t2(
      {Eigen::Vector3d({5, 5, 5}), Eigen::Vector3d({5, 5, 5}), Eigen::Vector3d({5, 5, 5})});

  t = 5;

  ASSERT_TRUE(t == t2);
}

TEST(DyadicTensorTest, PlusEqualsTensorOperator)
{
  Dyadic2DTensor t({Eigen::Vector2d({2, 8}), Eigen::Vector2d({5, 3})});
  Dyadic2DTensor t2({Eigen::Vector2d({6, 3}), Eigen::Vector2d({4, 6})});
  Dyadic2DTensor expected({Eigen::Vector2d({8, 11}), Eigen::Vector2d({9, 9})});

  Dyadic2DTensor result = t;
  result += t2;

  ASSERT_TRUE(expected == result);
}

// Coefficient wise multiplication
TEST(DyadicTensorTest, MultiplyTensorOperator)
{
  Dyadic2DTensor t({Eigen::Vector2d({2, 8}), Eigen::Vector2d({5, 3})});
  Dyadic2DTensor t2({Eigen::Vector2d({6, 3}), Eigen::Vector2d({4, 6})});
  Dyadic2DTensor expected({Eigen::Vector2d({12, 24}), Eigen::Vector2d({20, 18})});

  Dyadic2DTensor result = t * t2;

  ASSERT_TRUE(expected == result);
}

// Coefficient wise multiplication
TEST(DyadicTensorTest, MultiplyDoubleOperator)
{
  Dyadic2DTensor t({Eigen::Vector2d({2, 8}), Eigen::Vector2d({5, 3})});
  Dyadic2DTensor expected({Eigen::Vector2d({6, 24}), Eigen::Vector2d({15, 9})});

  Dyadic2DTensor result = t * 3.0;

  ASSERT_TRUE(expected == result);
}

// Coefficient wise multiplication
TEST(DyadicTensorTest, MultiplyDoubleReverseOperator)
{
  Dyadic2DTensor t({Eigen::Vector2d({2, 8}), Eigen::Vector2d({5, 3})});
  Dyadic2DTensor expected({Eigen::Vector2d({6, 24}), Eigen::Vector2d({15, 9})});

  Dyadic2DTensor result = 3.0 * t;

  ASSERT_TRUE(expected == result);
}

// Coefficient wise multiplication
TEST(DyadicTensorTest, MultiplyEqualsTensorOperator)
{
  Dyadic2DTensor t({Eigen::Vector2d({2, 8}), Eigen::Vector2d({5, 3})});
  Dyadic2DTensor t2({Eigen::Vector2d({6, 3}), Eigen::Vector2d({4, 6})});
  Dyadic2DTensor expected({Eigen::Vector2d({12, 24}), Eigen::Vector2d({20, 18})});

  Dyadic2DTensor result = t;
  result *= t2;

  ASSERT_TRUE(expected == result);
}

// Analog Equivalent of matrix multiplication
TEST(DyadicTensorTest, Contraction)
{
  Dyadic2DTensor t({Eigen::Vector2d({2, 8}), Eigen::Vector2d({5, 3})});
  Dyadic2DTensor t2({Eigen::Vector2d({6, 3}), Eigen::Vector2d({4, 6})});
  Dyadic2DTensor expected({Eigen::Vector2d({27, 57}), Eigen::Vector2d({38, 50})});

  Dyadic2DTensor result = t.contract(t2);

  ASSERT_TRUE(expected == result);
}

TEST(DyadicTensorTest, MinusOperator)
{
  Dyadic2DTensor t({Eigen::Vector2d({2, 8}), Eigen::Vector2d({5, 3})});
  Dyadic2DTensor t2({Eigen::Vector2d({6, 3}), Eigen::Vector2d({4, 6})});
  Dyadic2DTensor expected({Eigen::Vector2d({-4, 5}), Eigen::Vector2d({1, -3})});

  Dyadic2DTensor result = t - t2;

  ASSERT_TRUE(expected == result);
}

TEST(DyadicTensorTest, MinusEqualsOperator)
{
  Dyadic2DTensor t({Eigen::Vector2d({2, 8}), Eigen::Vector2d({5, 3})});
  Dyadic2DTensor t2({Eigen::Vector2d({6, 3}), Eigen::Vector2d({4, 6})});
  Dyadic2DTensor expected({Eigen::Vector2d({-4, 5}), Eigen::Vector2d({1, -3})});

  Dyadic2DTensor result = t;
  result -= t2;

  ASSERT_TRUE(expected == result);
}

TEST(DyadicTensorTest, FrobeniusNorm)
{
  Dyadic2DTensor t({Eigen::Vector2d({3, 0}), Eigen::Vector2d({0, 6})});
  ASSERT_EQ(std::sqrt(45), t.frobeniusNorm());
}

TEST(DyadicTensorTest, MaxNorm)
{
  Dyadic2DTensor t({Eigen::Vector2d({3, 0}), Eigen::Vector2d({0, 6})});
  ASSERT_EQ(6, t.maxNorm());
}

TEST(DyadicTensorTest, SetEigens)
{
  Dyadic2DTensor t({Eigen::Vector2d({3, 0}), Eigen::Vector2d({0, 6})});
  std::vector<double> eigvals = {3, 4};
  std::vector<Eigen::Vector2d> eigvecs = {Eigen::Vector2d({0, 1}), Eigen::Vector2d({1, 0})};
  t.setEigens(eigvecs, eigvals);
  ASSERT_EQ(eigvals, t.getEigenvalues());
  ASSERT_EQ(eigvecs, t.getEigenvectors());
}

TEST(DyadicTensorTest, SetEigensFail1)
{
  Dyadic2DTensor t({Eigen::Vector2d({3, 0}), Eigen::Vector2d({0, 6})});
  ASSERT_ANY_THROW(t.setEigens({Eigen::Vector2d({0, 1}), Eigen::Vector2d({1, 0})}, {3, 4, 5}));
}

TEST(DyadicTensorTest, EigenSolver)
{
  Eigen::Matrix3d m;
  m << 3, 0, 0, 0, 2, 0.5, 0, 0.5, 1;
  auto t = Dyadic3DTensor(m);

  std::vector<Eigen::Vector3d> expected = {Eigen::Vector3d({1.0, 0, 0}),
      Eigen::Vector3d({0, 0.923879, 0.382684}), Eigen::Vector3d({0, -0.382681, 0.923881})};

  auto eigvecs = t.getEigenvectors();

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      ASSERT_NEAR(expected[i][j], eigvecs[i][j], 0.00001);
}

TEST(DyadicTensorTest, NonSymmetricTestFail)
{
  Eigen::Matrix3d m;
  m << 3, 0, 0, 0, 2, 0, 0, 0.5, 1;
  ASSERT_ANY_THROW(auto t = Dyadic3DTensor(m));
}
