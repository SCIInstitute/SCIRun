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

   Author:              Moritz Dannhauer
   Last Modification:   April 13 2014
   TODO:                check Sparse inputs, Matrix inputs broken
*/


#include <gtest/gtest.h>

#include <Core/Algorithms/Math/GetMatrixSliceAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/Tests/MatrixTestCases.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

TEST(GetMatrixSliceAlgoTests, ThrowsOnNullInput)
{
  GetMatrixSliceAlgo algo;
  EXPECT_THROW(algo.runImpl(nullptr, 0, true), AlgorithmInputException);
}

TEST(GetMatrixSliceAlgoTests, CanGetColumnOrRowDense)
{
  GetMatrixSliceAlgo algo;

  DenseMatrixHandle m1(SCIRun::TestUtils::matrix1H());

  for (int i = 0; i < m1->ncols(); ++i)
  {
    auto col = algo.runImpl(m1, i, true);
    DenseMatrix expected(m1->col(i));
    //std::cout << "expected column:\n" << expected << std::endl;
    EXPECT_EQ(expected, *castMatrix::toDense(col.get<0>()));
    EXPECT_EQ(m1->ncols() - 1, col.get<1>());
  }
  for (int i = 0; i < m1->nrows(); ++i)
  {
    auto row = algo.runImpl(m1, i, false);
    DenseMatrix expected(m1->row(i));
    EXPECT_EQ(expected, *castMatrix::toDense(row.get<0>()));
    EXPECT_EQ(m1->nrows() - 1, row.get<1>());
  }
}

TEST(GetMatrixSliceAlgoTests, CanGetColumnOrRowSparse)
{
  GetMatrixSliceAlgo algo;

  SparseRowMatrixHandle m1(SCIRun::TestUtils::matrix1sparse());

  for (int i = 0; i < m1->ncols(); ++i)
  {
    auto col = algo.runImpl(m1, i, true);
    DenseMatrix expected(SCIRun::TestUtils::matrix1H()->col(i));
    ASSERT_TRUE(col.get<0>() != nullptr);
    //std::cout << "expected\n" << expected << "\n\nactual\n" << *convertMatrix::toDense(castMatrix::toSparse(col.get<0>())) << std::endl;
    EXPECT_EQ(expected, *convertMatrix::toDense(castMatrix::toSparse(col.get<0>())));
    EXPECT_EQ(m1->ncols() - 1, col.get<1>());
  }
  for (int i = 0; i < m1->nrows(); ++i)
  {
    auto row = algo.runImpl(m1, i, false);
    SparseRowMatrix expected(m1->row(i));
    ASSERT_TRUE(row.get<0>() != nullptr);
    EXPECT_SPARSE_EQ(expected, *castMatrix::toSparse(row.get<0>()));
    EXPECT_EQ(m1->nrows() - 1, row.get<1>());
  }
}

TEST(GetMatrixSliceAlgoTests, DISABLED_RunGenericWorks)
{
  GetMatrixSliceAlgo algo;

  DenseMatrixHandle m1(SCIRun::TestUtils::matrix1H());
  //auto output = algo.run()

  FAIL() << "todo";
}

TEST(GetMatrixSliceAlgoTests, ThrowsForOutOfRangeIndex)
{
  GetMatrixSliceAlgo algo;

  DenseMatrixHandle m1(SCIRun::TestUtils::matrix1H());

  EXPECT_THROW(algo.runImpl(m1, m1->ncols(), true), AlgorithmInputException);
  EXPECT_THROW(algo.runImpl(m1, m1->ncols()+1, true), AlgorithmInputException);
  EXPECT_THROW(algo.runImpl(m1, -1, true), AlgorithmInputException);

  EXPECT_THROW(algo.runImpl(m1, m1->nrows(), false), AlgorithmInputException);
  EXPECT_THROW(algo.runImpl(m1, m1->nrows()+1, false), AlgorithmInputException);
  EXPECT_THROW(algo.runImpl(m1, -1, false), AlgorithmInputException);
}

TEST(GetMatrixSliceAlgoTests, DISABLED_TestDoubleTranspose)
{
  auto m = SCIRun::TestUtils::matrix1sparse();
  std::cout << m->castForPrinting() << std::endl;

/*
  for (int i = 0; i < m->nrows(); ++i)
  {
    std::cout << i << "\n---\n" << m->row(i) << std::endl;
  }
  */

/* this code doesn't work--produces either truncated columns or lots of zeroes/garbage
  for (int j = 0; j < m->ncols(); ++j)
  {
    std::cout << j << "\n---\n" << m->col(j) << std::endl;
  }
  */

  auto spEv = m->transpose().eval();
/*
  std::cout << "transpose:\n\n" << m->transpose() << std::endl;
  std::cout << "type: " << typeid(m->transpose()).name() << std::endl;
  std::cout << "transpose dims: " << m->transpose().rows() << " x " << m->transpose().cols() << std::endl;

  std::cout << "transpose eval:\n\n" << spEv << std::endl;
  std::cout << "type: " << typeid(spEv).name() << std::endl;
  std::cout << "transpose eval dims: " << spEv.rows() << " x " << spEv.cols() << std::endl;

  std::cout << "looping over rows BUGGY VERSION:\n";
  for (int i = 0; i < spEv.rows(); ++i)
  {
    std::cout << i << "\n---\n" << spEv.row(i) << std::endl;
  }
  std::cout << "looping over rows DAN VERSION:\n";
  for (int i = 0; i < spEv.rows(); ++i)
  {
    std::cout << i << "\n---\n" << spEv.block(i, 0, 1, spEv.cols()) << std::endl;
  }

  std::cout << "looping over cols:\n";
  for (int i = 0; i < spEv.cols(); ++i)
  {
    std::cout << i << "\n---\n" << spEv.col(i) << std::endl;
  }
  */

  std::cout << "looping over cols of original:\n";
  for (int i = 0; i < spEv.rows(); ++i)
  {
    std::cout << i << "\n---\n" << spEv.block(i, 0, 1, spEv.cols()).transpose() << std::endl;
  }

  for (int i = 0; i < m->ncols(); ++i)
  {
    std::cout << i << "\n---\n" << m->getColumn(i).castForPrinting() << std::endl;
  }
}
