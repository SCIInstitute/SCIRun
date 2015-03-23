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
   author: Moritz Dannhauer
   last change: 4/13/14
   TODO: check Sparse inputs, Matrix inputs broken
*/

#include <gtest/gtest.h>

#include <Core/Algorithms/Math/GetMatrixSliceAlgo.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixIO.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

using namespace SCIRun::Core;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

namespace
{
  DenseMatrixHandle matrix1()
  {
    DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3, 4));
    for (int i = 0; i < m->rows(); i++)
      for (int j = 0; j < m->cols(); j++)
        (*m)(i, j) = 3.0 * i + j - 5;
    return m;
  }

  SparseRowMatrixHandle matrix2()
  {
    return matrix_convert::to_sparse(matrix1());
  }
}

TEST(GetMatrixSliceAlgoTests, ThrowsOnNullInput)
{
  GetMatrixSliceAlgo algo;
  EXPECT_THROW(algo.runImpl(nullptr, 0, true), AlgorithmInputException);
}

TEST(GetMatrixSliceAlgoTests, CanGetColumnOrRowDense)
{
  GetMatrixSliceAlgo algo;

  DenseMatrixHandle m1(matrix1());

  for (int i = 0; i < m1->ncols(); ++i)
  {
    auto col = algo.runImpl(m1, i, true);
    DenseMatrix expected(m1->col(i));
    EXPECT_EQ(expected, *matrix_cast::as_dense(col.get<0>()));
    EXPECT_EQ(m1->ncols() - 1, col.get<1>());
  }
  for (int i = 0; i < m1->nrows(); ++i)
  {
    auto row = algo.runImpl(m1, i, false);
    DenseMatrix expected(m1->row(i));
    EXPECT_EQ(expected, *matrix_cast::as_dense(row.get<0>()));
    EXPECT_EQ(m1->nrows() - 1, row.get<1>());
  }
}

TEST(GetMatrixSliceAlgoTests, CanGetColumnOrRowSparse)
{
  GetMatrixSliceAlgo algo;

  SparseRowMatrixHandle m1(matrix2());

  for (int i = 0; i < m1->ncols(); ++i)
  {
    EXPECT_THROW(algo.runImpl(m1, i, true), AlgorithmProcessingException);
    /* TODO: fix in #822
    SparseRowMatrix expected(m1->col(i));
    ASSERT_TRUE(col.get<0>() != nullptr);
    EXPECT_EQ(expected, *matrix_cast::as_sparse(col.get<0>()));
    EXPECT_EQ(m1->ncols() - 1, col.get<1>());
    */
  }
  for (int i = 0; i < m1->nrows(); ++i)
  {
    auto row = algo.runImpl(m1, i, false);
    SparseRowMatrix expected(m1->row(i));
    ASSERT_TRUE(row.get<0>() != nullptr);
    EXPECT_EQ(expected, *matrix_cast::as_sparse(row.get<0>()));
    EXPECT_EQ(m1->nrows() - 1, row.get<1>());
  }
}

TEST(GetMatrixSliceAlgoTests, DISABLED_RunGenericWorks)
{
  GetMatrixSliceAlgo algo;

  DenseMatrixHandle m1(matrix1());
  //auto output = algo.run_generic()

  FAIL() << "todo";
}

TEST(GetMatrixSliceAlgoTests, ThrowsForOutOfRangeIndex)
{
  GetMatrixSliceAlgo algo;

  DenseMatrixHandle m1(matrix1());

  EXPECT_THROW(algo.runImpl(m1, m1->ncols(), true), AlgorithmInputException);
  EXPECT_THROW(algo.runImpl(m1, m1->ncols()+1, true), AlgorithmInputException);
  EXPECT_THROW(algo.runImpl(m1, -1, true), AlgorithmInputException);

  EXPECT_THROW(algo.runImpl(m1, m1->nrows(), false), AlgorithmInputException);
  EXPECT_THROW(algo.runImpl(m1, m1->nrows()+1, false), AlgorithmInputException);
  EXPECT_THROW(algo.runImpl(m1, -1, false), AlgorithmInputException);


}
