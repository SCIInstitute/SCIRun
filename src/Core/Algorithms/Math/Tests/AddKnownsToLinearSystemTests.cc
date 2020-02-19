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


#include <Testing/Utils/SCIRunUnitTests.h>
#include <gtest/gtest.h>
#include <Core/Algorithms/Math/AddKnownsToLinearSystem.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixComparison.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/MatrixIO.h>
#include <limits>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::TestUtils;

namespace
{
	// x vector [3; 4; 5;]
	DenseMatrixHandle x_num ()
	{
		DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
		for (int i = 0; i < m->rows(); ++ i)
			for (int j = 0; j < m->cols(); ++ j)
				(*m)(i, j) = i+3;
		return m;
	}
	// x vector [1; nan; 2]
	DenseMatrixHandle x_one_nan ()
	{
		DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
		(*m)(0, 0) = 1;
		(*m)(1, 0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(2, 0) = 2;
		return m;
	}
  // x vector [3; nan; nan]
	DenseMatrixHandle x_two_nan ()
	{
		DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
		(*m)(0, 0) = 3;
		(*m)(1, 0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(2, 0) = std::numeric_limits<double>::quiet_NaN();
		return m;
	}
	// x vector [NaN; NaN; NaN;]
	DenseMatrixHandle x_all_nan ()
	{
		DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
		(*m)(0, 0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(1, 0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(2, 0) = std::numeric_limits<double>::quiet_NaN();
		return m;
	}

	// symmetric LHS (stiff) matrix
	SparseRowMatrixHandle LHS()
	{
		SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
		m->insert(0,0) = 1;
		m->insert(0,1) = 7;
		m->insert(0,2) = 3;
		m->insert(1,0) = 7;
		m->insert(1,1) = 4;
		m->insert(1,2) = -5;
		m->insert(2,0) = 3;
		m->insert(2,1) = -5;
		m->insert(2,2) = 6;
		m->makeCompressed();
		return m;
	}
	// non symmetric LHS (stiff) matrix
	// SparseRowMatrixHandle LHS_not_sym()
	// {
	// 	SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
	// 	m->insert(0,0) = 1;
	// 	m->insert(0,1) = 8;
	// 	m->insert(0,2) = 3;
	// 	m->insert(1,0) = 7;
	// 	m->insert(1,1) = 4;
	// 	m->insert(1,2) = -12;
	// 	m->insert(2,0) = 3;
	// 	m->insert(2,1) = -5;
	// 	m->insert(2,2) = 6;
	// 	m->makeCompressed();
	// 	return m;
	// }
	// non square LHS (Stiff) matrix
	SparseRowMatrixHandle LHS_non_sqr()
	{
		SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,5));
		m->setZero();
		m->makeCompressed();
		return m;
	}

	// RHS vector of zeros
	DenseMatrixHandle rhs_zero(int rows)
	{
		DenseMatrixHandle m(boost::make_shared<DenseMatrix>(rows,1));
		m->setZero();
		return m;
	}
	// RHS vector [1;2;3]
	DenseMatrixHandle rhs()
	{
		DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
		(*m)(0,0) = 1;
		(*m)(1,0) = 2;
		(*m)(2,0) = 3;
		return m;
	}
	// RHS vector with NaN
	DenseMatrixHandle rhs_nan()
	{
		DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
		(*m)(0,0) = 1;
		(*m)(1,0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(2,0) = 3;
		return m;
	}
}

// this test makes sure an exception is thrown for a non square matrix
TEST (AddKnownsToLinearSystemTests, Non_Square_LHS)
{
  SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
  EXPECT_THROW (algo.run(LHS_non_sqr(),rhs_zero(3),x_num(),output_stiff,output_rhs), AlgorithmInputException);
}

// this test makes sure no exceptions are thrown when a symmetric matric is used for the LHS
TEST (AddKnownsToLinearSystemTests, Square_and_Symmetric_LHS)
{
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	EXPECT_TRUE(algo.run(LHS(),rhs_zero(3),x_num(),output_stiff,output_rhs));
}

// this test makes sure an exception is thrown when rhs contains NaN
TEST (AddKnownsToLinearSystemTests, RHS_Contains_NaN)
{
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	EXPECT_THROW (algo.run(LHS(),rhs_nan(),x_num(),output_stiff,output_rhs), AlgorithmInputException);
}

// this test compares the output from SCIRun4 when X contains a single NaN, RHS is all 0
TEST (AddKnownsToLinearSystemTests, X_contains_one_NaN_and_RHS_contains_0)
{
  // this information was obtained from SCIRun4.7
  SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
  m->insert(0,0) = 1;
  m->insert(0,1) = 0;
  m->insert(0,2) = 0;
  m->insert(1,0) = 0;
  m->insert(1,1) = 4;
  m->insert(1,2) = 0;
  m->insert(2,0) = 0;
  m->insert(2,1) = 0;
  m->insert(2,2) = 1;
  m->makeCompressed();
  DenseMatrixHandle ro(boost::make_shared<DenseMatrix>(3,1));
  (*ro)(0,0) = 1;
  (*ro)(1,0) = 3;
  (*ro)(2,0) = 2;

  SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	algo.run(LHS(),rhs_zero(3),x_one_nan(),output_stiff,output_rhs);
  // making sure each element of LHS are equal
  for (int r=0; r < m->rows(); r++)
  {
    for (int c=0; c < m->cols(); c++)
    {
      EXPECT_EQ(output_stiff->coeff(r,c),m->coeff(r,c));
    }
  }
  // making sure each element of RHS are equal
  for (int r=0; r < ro->rows(); r++)
    EXPECT_EQ((*output_rhs)[r],(*ro)(r,0));
}

// this test compares the output from SCIRun4 when X contains two NaN, RHS is all 0
TEST (AddKnownsToLinearSystemTests, X_contains_two_NaN_and_RHS_contains_0)
{
  // this information was obtained from SCIRun4.7
  SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
  m->insert(0,0) = 1;
  m->insert(0,1) = 0;
  m->insert(0,2) = 0;
  m->insert(1,0) = 0;
  m->insert(1,1) = 4;
  m->insert(1,2) = -5;
  m->insert(2,0) = 0;
  m->insert(2,1) = -5;
  m->insert(2,2) = 6;
  m->makeCompressed();
  DenseMatrixHandle ro(boost::make_shared<DenseMatrix>(3,1));
  (*ro)(0,0) = 3;
  (*ro)(1,0) = -21;
  (*ro)(2,0) = -9;

  SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	algo.run(LHS(),rhs_zero(3),x_two_nan(),output_stiff,output_rhs);
  // making sure each element of LHS are equal
  for (int r=0; r < m->rows(); r++)
  {
    for (int c=0; c < m->cols(); c++)
    {
      EXPECT_EQ(output_stiff->coeff(r,c),m->coeff(r,c));
    }
  }
  // making sure each element of RHS are equal
  for (int r=0; r < ro->rows(); r++)
    EXPECT_EQ((*output_rhs)[r],(*ro)(r,0));
}

// this test compares the output from SCIRun4 when X contains a single NaN, RHS is [1,2,3]
TEST (AddKnownsToLinearSystemTests, X_contains_one_NaN_and_RHS_contains_numbers)
{
  // this information was obtained from SCIRun4.7
  SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
  m->insert(0,0) = 1;
  m->insert(0,1) = 0;
  m->insert(0,2) = 0;
  m->insert(1,0) = 0;
  m->insert(1,1) = 4;
  m->insert(1,2) = 0;
  m->insert(2,0) = 0;
  m->insert(2,1) = 0;
  m->insert(2,2) = 1;
  m->makeCompressed();
  DenseMatrixHandle ro(boost::make_shared<DenseMatrix>(3,1));
  (*ro)(0,0) = 1;
  (*ro)(1,0) = 5;
  (*ro)(2,0) = 2;

  SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	algo.run(LHS(),rhs(),x_one_nan(),output_stiff,output_rhs);
  // making sure each element of LHS are equal
  for (int r=0; r < m->rows(); r++)
  {
    for (int c=0; c < m->cols(); c++)
    {
      EXPECT_EQ(output_stiff->coeff(r,c),m->coeff(r,c));
    }
  }
  // making sure each element of RHS are equal
  for (int r=0; r < ro->rows(); r++)
    EXPECT_EQ((*output_rhs)[r],(*ro)(r,0));
}

// this test compares the output from SCIRun4 when X contains two NaN, RHS is [1,2,3]
TEST (AddKnownsToLinearSystemTests, X_contains_two_NaN_and_RHS_contains_numbers)
{
  // this information was obtained from SCIRun 4.7
  SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
  m->insert(0,0) = 1;
  m->insert(0,1) = 0;
  m->insert(0,2) = 0;
  m->insert(1,0) = 0;
  m->insert(1,1) = 4;
  m->insert(1,2) = -5;
  m->insert(2,0) = 0;
  m->insert(2,1) = -5;
  m->insert(2,2) = 6;
  m->makeCompressed();
  DenseMatrixHandle ro(boost::make_shared<DenseMatrix>(3,1));
  (*ro)(0,0) = 3;
  (*ro)(1,0) = -19;
  (*ro)(2,0) = -6;

  SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	algo.run(LHS(),rhs(),x_two_nan(),output_stiff,output_rhs);
  // making sure each element of LHS are equal
  for (int r=0; r < m->rows(); r++)
  {
    for (int c=0; c < m->cols(); c++)
    {
      EXPECT_EQ(output_stiff->coeff(r,c),m->coeff(r,c));
    }
  }
  // making sure each element of RHS are equal
  for (int r=0; r < ro->rows(); r++)
    EXPECT_EQ((*output_rhs)[r],(*ro)(r,0));
}

// test to make sure when X contains all NaN that outputs are copied from inputs
TEST (AddKnownsToLinearSystemTests, X_is_NaN)
{
  // this is what is expected to be returned (the input)
  SparseRowMatrixHandle m = LHS();
  DenseMatrixHandle ro = rhs_zero(3);

  SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
  algo.run(LHS(),rhs_zero(3),x_all_nan(),output_stiff,output_rhs);

  // making sure each element of LHS are equal
  for (int r=0; r < m->rows(); r++)
  {
    for (int c=0; c < m->cols(); c++)
    {
      EXPECT_EQ(output_stiff->coeff(r,c),m->coeff(r,c));
    }
  }
  // making sure each element of RHS are equal
  for (int r=0; r < ro->rows(); r++)
    EXPECT_EQ((*output_rhs)[r],(*ro)(r,0));
}
