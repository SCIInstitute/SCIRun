/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>


using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3, 4));
    for (int i = 0; i < m->rows(); ++ i)
      for (int j = 0; j < m->cols(); ++ j)
        (*m)(i, j) = 3.0 * i + j - 5;
    return m;
  }
  const DenseMatrix Zero(DenseMatrix::Zero(3,3));
}

TEST(AppendMatrixAlgorithmTests, CanAppendRows)
{
  AppendMatrixAlgorithm algo;

  DenseMatrixHandle m1(matrix1());
  DenseMatrixHandle m2(matrix1());
  AppendMatrixAlgorithm::Outputs result = algo.run(AppendMatrixAlgorithm::Inputs(m1, m2), AppendMatrixAlgorithm::ROWS);

  EXPECT_EQ(6, result->rows());
  EXPECT_EQ(4, result->cols());

  std::cout << *result << std::endl;
}

TEST(AppendMatrixAlgorithmTests, CanAppendColumns)
{
  AppendMatrixAlgorithm algo;

  DenseMatrixHandle m1(matrix1());
  DenseMatrixHandle m2(matrix1());
  AppendMatrixAlgorithm::Outputs result = algo.run(AppendMatrixAlgorithm::Inputs(m1, m2), AppendMatrixAlgorithm::COLUMNS);

  EXPECT_EQ(3, result->rows());
  EXPECT_EQ(8, result->cols());

  std::cout << *result << std::endl;
}

TEST(AppendMatrixAlgorithmTests, ReturnsNullWithSizeMismatch)
{
  /// @todo: should return with error.
  AppendMatrixAlgorithm algo;

  DenseMatrixHandle m1(matrix1());
  DenseMatrixHandle m2(Zero.clone());
  AppendMatrixAlgorithm::Outputs result = algo.run(AppendMatrixAlgorithm::Inputs(m1, m2), AppendMatrixAlgorithm::ROWS);

  ASSERT_FALSE(result);
}

TEST(AppendMatrixAlgorithmTests, NullInputReturnsDummyValues)
{
  AppendMatrixAlgorithm algo;

  AppendMatrixAlgorithm::Outputs result = algo.run(AppendMatrixAlgorithm::Inputs(), AppendMatrixAlgorithm::ROWS);
  EXPECT_FALSE(result);
}