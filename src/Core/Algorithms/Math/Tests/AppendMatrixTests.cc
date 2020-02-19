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

#include <Core/Algorithms/Math/AppendMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/Tests/MatrixTestCases.h>


using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;

TEST(AppendMatrixAlgorithmTests, CanAppendRows)
{
  AppendMatrixAlgorithm algo;

  DenseMatrixHandle m1(SCIRun::TestUtils::matrixNonSquare().clone());
  DenseMatrixHandle m2(SCIRun::TestUtils::matrixNonSquare().clone());
  auto result = algo.run(AppendMatrixAlgorithm::Inputs(m1, m2), AppendMatrixAlgorithm::ROWS);

  EXPECT_EQ(6, result->nrows());
  EXPECT_EQ(4, result->ncols());

  std::cout << *result << std::endl;
}

TEST(AppendMatrixAlgorithmTests, CanAppendColumns)
{
  AppendMatrixAlgorithm algo;

  DenseMatrixHandle m1(SCIRun::TestUtils::matrixNonSquare().clone());
  DenseMatrixHandle m2(SCIRun::TestUtils::matrixNonSquare().clone());
  auto result = algo.run(AppendMatrixAlgorithm::Inputs(m1, m2), AppendMatrixAlgorithm::COLUMNS);

  EXPECT_EQ(3, result->nrows());
  EXPECT_EQ(8, result->ncols());

  std::cout << *result << std::endl;
}

TEST(AppendMatrixAlgorithmTests, ReturnsNullWithSizeMismatch)
{
  /// @todo: should return with error.
  AppendMatrixAlgorithm algo;

  DenseMatrixHandle m1(SCIRun::TestUtils::matrixNonSquare().clone());
  DenseMatrixHandle m2(SCIRun::TestUtils::Zero.clone());
  auto result = algo.run(AppendMatrixAlgorithm::Inputs(m1, m2), AppendMatrixAlgorithm::ROWS);

  ASSERT_FALSE(result);
}

TEST(AppendMatrixAlgorithmTests, NullInputReturnsDummyValues)
{
  AppendMatrixAlgorithm algo;

  auto result = algo.run(AppendMatrixAlgorithm::Inputs(), AppendMatrixAlgorithm::ROWS);
  EXPECT_FALSE(result);
}

TEST(AppendMatrixAlgorithmTests, AppendSquareSparseMatrix)
{
  AppendMatrixAlgorithm algo;
  int nr_rows=3, nr_cols=nr_rows;
  SparseRowMatrix m1(nr_rows,nr_cols);
  for(int i=0;i<nr_rows;i++)
    m1.insert(i,i) = i+1;

  SparseRowMatrix m2(nr_rows,nr_cols);
  for(int i=0;i<nr_rows;i++)
    m2.insert(i,i) = i+nr_rows;

  auto result = algo.run(AppendMatrixAlgorithm::Inputs(boost::make_shared<SparseRowMatrix>(m1), boost::make_shared<SparseRowMatrix>(m2)), AppendMatrixAlgorithm::ROWS);
  auto out = boost::dynamic_pointer_cast<SparseRowMatrix>(result);

  for (Eigen::Index k = 0; k < out->nrows(); ++k)
  {
    for (SparseRowMatrix::InnerIterator it(*out, k); it; ++it)
    {
      if (it.row()<nr_rows)
       ASSERT_TRUE(m1.coeffRef(it.row(),it.col())==it.value());
      else
       ASSERT_TRUE(m2.coeffRef(it.row()-nr_rows,it.col())==it.value());
    }
  }

  result = algo.run(AppendMatrixAlgorithm::Inputs(boost::make_shared<SparseRowMatrix>(m1), boost::make_shared<SparseRowMatrix>(m2)), AppendMatrixAlgorithm::COLUMNS);
  out = boost::dynamic_pointer_cast<SparseRowMatrix>(result);

  for (Eigen::Index k = 0; k < out->nrows(); ++k)
  {
    for (SparseRowMatrix::InnerIterator it(*out, k); it; ++it)
    {
      if (it.col()<nr_cols)
       ASSERT_TRUE(m1.coeffRef(it.row(),it.col())==it.value());
      else
       ASSERT_TRUE(m2.coeffRef(it.row(),it.col()-nr_cols)==it.value());
    }
  }

}

TEST(AppendMatrixAlgorithmTests, AppendDenseColumnMatrix)
{
  AppendMatrixAlgorithm algo;
  int nr_comp1=3, nr_comp2=2; /// assumption for the code in this function: the second column vector is shorter
  DenseColumnMatrixHandle m1(boost::make_shared<DenseColumnMatrix>(nr_comp1));
  DenseColumnMatrixHandle m2(boost::make_shared<DenseColumnMatrix>(nr_comp2));

  for (int i=0;i<nr_comp1;i++)
    (*m1)(i) = i+1;

  for (int i=0;i<nr_comp2;i++)
    (*m2)(i) = 2*(i+1);

  auto result = algo.run(AppendMatrixAlgorithm::Inputs(m1, m2), AppendMatrixAlgorithm::COLUMNS);
  EXPECT_TRUE(result==nullptr);
       result = algo.run(AppendMatrixAlgorithm::Inputs(m1, m2), AppendMatrixAlgorithm::ROWS);

 auto out = boost::dynamic_pointer_cast<DenseColumnMatrix>(result);
 for (int i = 0; i < out->nrows(); i++)
  if(i<nr_comp1)
   EXPECT_EQ((*m1)(i),(*out)(i));
  else
   EXPECT_EQ((*m2)(i-nr_comp1),(*out)(i));

}

TEST(AppendMatrixAlgorithmTests, MixingMatrixInputTypes)
{
  AppendMatrixAlgorithm algo;
  int nr_rows=3, nr_cols=nr_rows;
  SparseRowMatrix sparse_mat(nr_rows,nr_cols);

  for(int i=0;i<nr_rows;i++)
   sparse_mat.insert(i,i) = i;

  int count=0;

  DenseMatrixHandle dense_mat(boost::make_shared<DenseMatrix>(nr_rows,nr_cols));
  for(int i=0;i<nr_rows;i++)
   for(int j=0;j<nr_rows;j++)
     (*dense_mat)(i,j)=++count;

  auto result = algo.run(AppendMatrixAlgorithm::Inputs(dense_mat, boost::make_shared<SparseRowMatrix>(sparse_mat)), AppendMatrixAlgorithm::ROWS);
  EXPECT_TRUE(result==nullptr);
}
