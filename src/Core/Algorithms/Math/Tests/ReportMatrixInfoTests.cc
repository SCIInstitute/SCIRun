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

#include <fstream>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Testing/Utils/SCIRunUnitTests.h>

using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::TestUtils;

namespace
{
  DenseMatrixHandle matrix1Dense()
  {
    auto m(makeShared<DenseMatrix>(3, 4));
    for (int i = 0; i < m->rows(); ++ i)
      for (int j = 0; j < m->cols(); ++ j)
        (*m)(i, j) = 3.0 * i + j - 5;
    return m;
  }
  SparseRowMatrixHandle matrix1Sparse()
  {
    auto m(makeShared<SparseRowMatrix>(5,5));
    m->insert(0,0) = 1;
    m->insert(1,2) = -1;
    m->insert(4,4) = 2;
    /// @todo: remove this when NonZeroIterator is ready
    m->makeCompressed();
    return m;
  }
  DenseColumnMatrixHandle matrix1DenseColumn()
  {
    auto m(makeShared<DenseColumnMatrix>(4));
    m -> setZero();
    *m << 1,2,3,4;
    return m;
  }
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsMatrixType)
{
  ReportMatrixInfoAlgorithm algo;

  MatrixHandle m(matrix1Dense());
  auto result = algo.runImpl(m);
  EXPECT_EQ("DenseMatrix", result.get<0>());
  m = matrix1Sparse();
  result = algo.runImpl(m);
  EXPECT_EQ("SparseRowMatrix", result.get<0>());
  m = matrix1DenseColumn();
  result = algo.runImpl(m);
  EXPECT_EQ("DenseColumnMatrix", result.get<0>());
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsRowAndColumnCount)
{
  ReportMatrixInfoAlgorithm algo;

  MatrixHandle m(matrix1Dense());
  auto result = algo.runImpl(m);
  EXPECT_EQ(3, result.get<1>());
  EXPECT_EQ(4, result.get<2>());
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsNumberOfElements)
{
  ReportMatrixInfoAlgorithm algo;

  MatrixHandle m(matrix1Dense());
  auto result = algo.runImpl(m);

  EXPECT_EQ(12, result.get<3>());
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsNumberOfNonzeroElementsForSparse)
{
  ReportMatrixInfoAlgorithm algo;

  MatrixHandle m(matrix1Sparse());
  auto result = algo.runImpl(m);

  EXPECT_EQ(3, result.get<3>());
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsMinimumAndMaximum)
{
  ReportMatrixInfoAlgorithm algo;

  MatrixHandle m(matrix1Dense());
  auto result = algo.runImpl(m);
  EXPECT_EQ(-5, result.get<4>());
  EXPECT_EQ(4, result.get<5>());
  m = matrix1Sparse();
  result = algo.runImpl(m);
  EXPECT_EQ(-1, result.get<4>());
  EXPECT_EQ(2, result.get<5>());
}

TEST(ReportMatrixInfoAlgorithmTests, NullInputThrows)
{
  ReportMatrixInfoAlgorithm algo;

  EXPECT_THROW(algo.runImpl(DenseMatrixHandle()), AlgorithmInputException);
}

TEST(ReportMatrixInfoAlgorithmTests, EmptyInputDoesNotThrow)
{
  ReportMatrixInfoAlgorithm algo;
  auto empty(makeShared<DenseMatrix>());

  auto result = algo.runImpl(empty);
  EXPECT_EQ(0, result.get<1>());
  EXPECT_EQ(0, result.get<2>());
  EXPECT_EQ(0, result.get<3>());
  EXPECT_EQ(0, result.get<4>());
  EXPECT_EQ(0, result.get<5>());
}
