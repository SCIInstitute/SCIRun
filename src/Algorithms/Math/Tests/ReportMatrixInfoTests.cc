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

#include <fstream>
#include <Algorithms/Math/ReportMatrixInfo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>

using namespace SCIRun::Domain::Datatypes;
using namespace SCIRun::Algorithms::Math;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(new DenseMatrix(3, 4));
    for (size_t i = 0; i < m->nrows(); ++ i)
      for (size_t j = 0; j < m->ncols(); ++ j)
        (*m)(i, j) = 3.0 * i + j - 5;
    return m;
  }
  const DenseMatrix Zero(DenseMatrix::zero_matrix(3,3));
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsMatrixType)
{
  ReportMatrixInfoAlgorithm algo;

  DenseMatrixHandle m(matrix1());
  ReportMatrixInfoAlgorithm::Outputs result = algo.run(m);
#ifdef WIN32
  const std::string expectedType = "class SCIRun::Domain::Datatypes::DenseMatrixGeneric<double>";
  EXPECT_EQ(expectedType, result.get<0>());
#else
  const std::string expectedType = ""; //fill this in later
  //EXPECT_EQ(expectedType, result.get<0>());
#endif
  EXPECT_TRUE(result.get<0>().find("DenseMatrix") != std::string::npos);
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsRowAndColumnCount)
{
  ReportMatrixInfoAlgorithm algo;

  DenseMatrixHandle m(matrix1());
  ReportMatrixInfoAlgorithm::Outputs result = algo.run(m);
  EXPECT_EQ(3, result.get<1>());
  EXPECT_EQ(4, result.get<2>());
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsNumberOfElements)
{
  ReportMatrixInfoAlgorithm algo;

  DenseMatrixHandle m(matrix1());
  ReportMatrixInfoAlgorithm::Outputs result = algo.run(m);
  
  EXPECT_EQ(12, result.get<3>());
}

TEST(ReportMatrixInfoAlgorithmTests, ReportsMinimumAndMaximum)
{
  ReportMatrixInfoAlgorithm algo;

  DenseMatrixHandle m(matrix1());
  ReportMatrixInfoAlgorithm::Outputs result = algo.run(m);
  EXPECT_EQ(-5, result.get<4>());
  EXPECT_EQ(4, result.get<5>());
}

TEST(ReportMatrixInfoAlgorithmTests, NullInputReturnsDummyValues)
{
  ReportMatrixInfoAlgorithm algo;

  ReportMatrixInfoAlgorithm::Outputs result = algo.run(DenseMatrixHandle());
  EXPECT_EQ("<null>", result.get<0>());
  EXPECT_EQ(0, result.get<1>());
  EXPECT_EQ(0, result.get<2>());
  EXPECT_EQ(0, result.get<3>());
  EXPECT_EQ(0, result.get<4>());
  EXPECT_EQ(0, result.get<5>());
}

TEST(BigVectorFieldFile, DISABLED_MakeIt)
{
  const size_t dim = 10;
  const size_t cols = 3 * dim * dim * dim;
  DenseMatrix m(6, cols);
  size_t col = 0;
  for (int d = 0; d < 3; ++d)
  {
    for (size_t i = 0; i < dim; ++i)
    {
      for (size_t j = 0; j < dim; ++j)
      {
        for (size_t k = 0; k < dim; ++k)
        {
          m(0, col) = (col % 3) == 0;
          m(1, col) = (col % 3) == 1;
          m(2, col) = (col % 3) == 2;
          m(3, col) = i;
          m(4, col) = j;
          m(5, col) = k;
          col++;
        }
      }
    }
  }
  std::ofstream file("E:\\git\\SCIRunGUIPrototype\\src\\Samples\\danBigMatrix.txt");
  file << m;
}

TEST(BigVectorFieldFile, DISABLED_ReadIt)
{
  std::ifstream file("E:\\git\\SCIRunGUIPrototype\\src\\Samples\\danBigMatrix.txt");
  DenseMatrix m;
  file >> m;
  EXPECT_EQ(6, m.nrows());
  EXPECT_EQ(3000, m.ncols());
}