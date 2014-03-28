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

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::TestUtils;

namespace {
	/* 0; 1; 2; */
	DenseMatrixHandle x1()
	{
		DenseMatrixHandle m(boost::make_shared<DenseMatrix>(3,1));
		for (int i = 0; i < m->rows(); ++ i)
			for (int j = 0; j < m->cols(); ++ j)
				(*m)(i, j) = i;
		return m;
	}

	/* makes how many rows given to be 0 */
	DenseMatrixHandle x_zero(int rows)
	{
		DenseMatrixHandle m(boost::make_shared<DenseMatrix>(rows,1));
		for (int i = 0; i < m->rows(); ++ i)
			for (int j = 0; j < m->cols(); ++ j)
				(*m)(i, j) = i;
		return m;
	}
	
	/* left hand side (LHS) stiff matrices for testing */
	// symmetric matrix
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
	// non symmetric matrix
	SparseRowMatrixHandle LHS_not_sym() 
	{
		SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,3));
		m->insert(0,0) = 1;
		m->insert(0,1) = 8;
		m->insert(0,2) = 3;
		m->insert(1,0) = 7;
		m->insert(1,1) = 4;
		m->insert(1,2) = -12;
		m->insert(2,0) = 3;
		m->insert(2,1) = -5;
		m->insert(2,2) = 6;
		m->makeCompressed();
		return m;
	}
	// non-square matrix
	SparseRowMatrixHandle LHS_non_sqr() 
	{
		SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,5));
		m->setZero();
		m->makeCompressed();
		return m;
	}
	
	/* right hand side (RHS) vector of 0s = rows */
	DenseColumnMatrixHandle rhs_zero(int rows)
	{
		DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(rows));
		m->setZero();
		return m;
	}

	/* right hand side (RHS) vector of [1;2;3] */
	DenseColumnMatrixHandle rhs()
	{
		DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));
		(*m)(0,0) = 1;
		(*m)(1,0) = 2;
		(*m)(2,0) = 3;
		return m;
	}
}

TEST (AddKnownsToLinearSystemAlgo, bad_parameters)
{	
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	
	AddKnownsToLinearSystemAlgo algo;

	// this test makes sure an exception is thrown for non symmetrical, but square, LHS matrix
	EXPECT_THROW (algo.run(LHS_not_sym(),rhs_zero(3),x1(),output_stiff,output_rhs), AlgorithmInputException);
	 
	// this test makes sure an exception is thrown for a non square matrix
	EXPECT_THROW (algo.run(LHS_non_sqr(),rhs_zero(3),x1(),output_stiff,output_rhs), AlgorithmInputException);
}

TEST (AddKnownsToLinearSystemAlgo, good_parameters)
{	
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	
	AddKnownsToLinearSystemAlgo algo;
	
	// this test makes sure no exceptions are thrown when a symmetric matric is used for the LHS
	EXPECT_TRUE(algo.run(LHS(),rhs_zero(3),x1(),output_stiff,output_rhs));
}

// This test checks if the output data of rhs is the same with different x vectors
TEST (AddKnownsToLinearSystemAlgo, checking_values_1)
{	
	AddKnownsToLinearSystemAlgo algo;

	SparseRowMatrixHandle output_stiff1;
	DenseColumnMatrixHandle output_rhs1;
	algo.run(LHS(),rhs_zero(),x1(),output_stiff1,output_rhs1);

	for (int r=0; r < output_rhs1->rows(); r++)
		std::cout << "rhs[" << r << "] = " << (*output_rhs1)[r] << std::endl;

	for (int r=0; r < output_stiff1->rows(); r++)
	{
		for (int c=0; c < output_stiff1->cols(); c++)
			std::cout << output_stiff1->coeff(r,c);
		std::cout << std::endl;
	}

	// rerrunning algo.run with a new x vector. rhs should be the same
	SparseRowMatrixHandle output_stiff2;
	DenseColumnMatrixHandle output_rhs2;
	algo.run(LHS(),rhs_zero(),x_zero(3),output_stiff2,output_rhs2);

	for (int r=0; r < output_rhs2->rows(); r++)
		std::cout << "rhs[" << r << "] = " << (*output_rhs2)[r] << std::endl;

	for (int r=0; r < output_stiff2->rows(); r++)
	{
		for (int c=0; c < output_stiff2->cols(); c++)
			std::cout << output_stiff2->coeff(r,c);
		std::cout << std::endl;
	}
}

// Seeing what happens when x = 0, = #'s
TEST (AddKnownsToLinearSystemAlgo, checking_values_2)
{	
	AddKnownsToLinearSystemAlgo algo;
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	algo.run(LHS(),rhs_zero(3),x_zero(3),output_stiff,output_rhs);
	
	

}

TEST (AddKnownsToLinearSystemAlgo, for_debugging)
{	
	system("pause");
}

// TODO test the output stiff

// TODO test the outout rhs