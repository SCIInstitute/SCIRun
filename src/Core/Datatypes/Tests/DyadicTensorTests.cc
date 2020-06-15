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
#include <Core/Datatypes/DyadicTensor.h>
#include <Core/Datatypes/MatrixFwd.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include "Core/Utils/Exception.h"

using namespace SCIRun;
using namespace Core::Datatypes;
using namespace Core::Geometry;
using namespace ::testing;

const std::vector<Vector> nativeEigvecs = {Vector(1.6, 0.9, 4.3), Vector(4.0, 6.4, 7), Vector(6, 34, 1)};
const std::string eigvecsString = "[1.6 0.9 4.3 4 6.4 7 6 34 1]";

std::vector<DenseColumnMatrix> getEigenEigvecs()
{
  std::vector<DenseColumnMatrix> eigvecs(3);
  for (int i = 0; i < 3; ++i)
  {
    eigvecs[i] = DenseColumnMatrix(3);
    for (int j = 0; j < 3; ++j)
      eigvecs[i][j] = nativeEigvecs[i][j];
  }
  return eigvecs;
}

TEST(Dyadic3DTensorTest, CanConstructTensorWithNativeVectors1)
{
  Dyadic3DTensor<double> t(nativeEigvecs);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, CanConstructTensorWithNativeVectors2)
{
  Dyadic3DTensor<double> t(nativeEigvecs[0], nativeEigvecs[1], nativeEigvecs[2]);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, CanConstructTensorWithEigenVectors1)
{
  Dyadic3DTensor<double> t(getEigenEigvecs());
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, CanConstructTensorWithEigenVectors2)
{
  auto eigvecs = getEigenEigvecs();
  Dyadic3DTensor<double> t(eigvecs[0], eigvecs[1], eigvecs[2]);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ(eigvecsString, ss.str());
}

TEST(Dyadic3DTensorTest, CanConstructTensorWithDoubles)
{
  Dyadic3DTensor<double> t(1, 2, 3, 4, 5, 6);
  std::stringstream ss;
  ss << t;
  ASSERT_EQ("[1 2 3 2 4 5 3 5 6]", ss.str());
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithLessThanThreeNativeEigenvectors)
{
  ASSERT_DEATH(Dyadic3DTensor<double> t({nativeEigvecs[0], nativeEigvecs[1]}), "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithLessThanThreeEigenEigenvectors)
{
  auto eigvecs = getEigenEigvecs();
  ASSERT_DEATH(Dyadic3DTensor<double> t({eigvecs[0], eigvecs[1]}), "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithMoreThanThreeEigenvectors)
{
  ASSERT_DEATH(Dyadic3DTensor<double> t(
                   {nativeEigvecs[0], nativeEigvecs[1], nativeEigvecs[2], nativeEigvecs[0]}),
      "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithMoreThanThreeEigenEigenvectors)
{
  auto eigvecs = getEigenEigvecs();
  ASSERT_DEATH(Dyadic3DTensor<double> t({eigvecs[0], eigvecs[1], eigvecs[2], eigvecs[0]}), "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithLessThanSixDoubles)
{
  ASSERT_DEATH(Dyadic3DTensor<double> t({1, 2, 3, 4, 5}), "");
}

TEST(Dyadic3DTensorTest, CannotConstructTensorWithMoreThanSixDoubles)
{
  ASSERT_DEATH(Dyadic3DTensor<double> t({1, 2, 3, 4, 5, 6, 7}), "");
}

TEST(DyadicTensorTest, CanConstructTensorWithEigenVectors)
{
  std::vector<DenseColumnMatrix> nativeEigvecs = {
      DenseColumnMatrix(4), DenseColumnMatrix(4), DenseColumnMatrix(4), DenseColumnMatrix(4)};
  int n = 0;
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      nativeEigvecs[i][j] = ++n;
  DyadicTensor<double> t(nativeEigvecs);
  std::stringstream ss;
  ss << t;

  ASSERT_EQ("[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16]", ss.str());
}

TEST(DyadicTensorTest, StringConversionTest)
{
  Dyadic3DTensor<double> t(nativeEigvecs);
  Dyadic3DTensor<double> t2();
  std::stringstream ss;
  ss << t;
  // ss >> t2;
}
