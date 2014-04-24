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
	// x vector of zeros
	DenseMatrixHandle x_zero (int rows)
	{
		DenseMatrixHandle m(boost::make_shared<DenseMatrix>(rows,1));
		for (int i = 0; i < m->rows(); ++ i)
			for (int j = 0; j < m->cols(); ++ j)
				(*m)(i, j) = 0;
		return m;
	}
	// x vector [1; NaN; 2;] 
	DenseMatrixHandle x_one_nan ()
	{
		DenseMatrixHandle m (boost::make_shared<DenseMatrix>(3,1));
		(*m)(0, 0) = 1;
		(*m)(1, 0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(2, 0) = 2;
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
	// x vector of all ones
	DenseMatrixHandle x_ones (int row, int col)
	{
		DenseMatrixHandle m (boost::make_shared<DenseMatrix>(row,col));
		for (int i = 0; i < m->rows(); ++ i)
			for (int j = 0; j < m->cols(); ++ j)
				(*m)(i, j) = 1;
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

	// non square LHS (Stiff) matrix
	SparseRowMatrixHandle LHS_non_sqr() 
	{
		SparseRowMatrixHandle m(boost::make_shared<SparseRowMatrix>(3,5));
		m->setZero();
		m->makeCompressed();
		return m;
	}
	
	// RHS vector of zeros
	DenseColumnMatrixHandle rhs_zero(int rows)
	{
		DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(rows));
		m->setZero();
		return m;
	}

	// RHS vector [1;2;3]
	DenseColumnMatrixHandle rhs()
	{
		DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));
		(*m)(0,0) = 1;
		(*m)(1,0) = 2;
		(*m)(2,0) = 3;
		return m;
	}

	// RHS vector with NaN
	DenseColumnMatrixHandle rhs_nan()
	{
		DenseColumnMatrixHandle m(boost::make_shared<DenseColumnMatrix>(3));
		(*m)(0,0) = 1;
		(*m)(1,0) = std::numeric_limits<double>::quiet_NaN();
		(*m)(2,0) = 3;
		return m;
	}

    // HELPER METHODS
    
	// method for displaying stiff / left hand side matrix (SparseRowMatrixHandle)
	void dispStiffOutput (SparseRowMatrixHandle ouput)
	{
        std::cout << "stiff matrix output:" << std::endl;
		for (int r=0; r < ouput->rows(); r++)
		{
			for (int c=0; c < ouput->cols(); c++)
				std::cout << ouput->coeff(r,c);
			std::cout << std::endl;
		}
	}
	// method for displaying right hand side vector (DenseColumnMatrixHandle)
	void dispRHS (DenseColumnMatrixHandle output)
	{
		for (int r=0; r < output->rows(); r++)
		std::cout << "rhs[" << r << "] = " << (*output)[r] << std::endl;
	}
	// method for displaying x vector (DenseMatrixHandle)
	void dispX (DenseMatrixHandle x)
	{
		for (int r=0; r < x->rows(); r++)
		std::cout << "x(" << r << ",0) = " << (*x)(r,0) << std::endl;
	}
		
}

TEST (AddKnownsToLinearSystemAlgo, Bad_LHS_Input)
{	
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;

	// this test makes sure an exception is thrown for non symmetrical, but square, LHS matrix
	EXPECT_THROW (algo.run(LHS_not_sym(),rhs_zero(3),x_num(),output_stiff,output_rhs), AlgorithmInputException);
	 
	// this test makes sure an exception is thrown for a non square matrix
	EXPECT_THROW (algo.run(LHS_non_sqr(),rhs_zero(3),x_num(),output_stiff,output_rhs), AlgorithmInputException);
}

TEST (AddKnownsToLinearSystemAlgo, Good_LHS_Input)
{	
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	
	// this test makes sure no exceptions are thrown when a symmetric matric is used for the LHS
	EXPECT_TRUE(algo.run(LHS(),rhs_zero(3),x_num(),output_stiff,output_rhs));
}

TEST (AddKnownsToLinearSystemAlgo, Bad_RHS_Input)
{
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	
	// this test makes sure an exception is thrown for rhs containing NaN
	EXPECT_THROW (algo.run(LHS(),rhs_nan(),x_num(),output_stiff,output_rhs), AlgorithmInputException);
}

// Looking at what having NaN in x does, and different values of x does
// RHS is always zero, LHS is always symmetric
TEST (AddKnownsToLinearSystemAlgo, X_Contains_NaN)
{	
	SparseRowMatrixHandle output_stiff;
	DenseColumnMatrixHandle output_rhs;
	AddKnownsToLinearSystemAlgo algo;
	
	DenseMatrixHandle x = x_one_nan();
	algo.run(LHS(),rhs_zero(3),x,output_stiff,output_rhs);
	std::cout << "x contains one NaN" << std::endl;
	dispX(x);
	dispRHS(output_rhs);
	dispStiffOutput(output_stiff);

	std::cout << "x contains all NaN" << std::endl;
	algo.run(LHS(),rhs_zero(3),x_all_nan(),output_stiff,output_rhs);
	dispRHS(output_rhs);
	dispStiffOutput(output_stiff);

	std::cout << "x contains numbers" << std::endl;
	DenseMatrixHandle xNum = x_num();
	algo.run(LHS(),rhs_zero(3),xNum,output_stiff,output_rhs);
	dispX(xNum);
	dispRHS(output_rhs);
	dispStiffOutput(output_stiff);

	std::cout << "x contains all 0s" << std::endl;
	DenseMatrixHandle x_0 = x_zero(3);
	algo.run(LHS(),rhs_zero(3),x_0,output_stiff,output_rhs);
	dispRHS(output_rhs);
	dispStiffOutput(output_stiff);

	std::cout << "x contains all 1s" << std::endl;
	DenseMatrixHandle x_1 = x_ones(3,1);
	algo.run(LHS(),rhs_zero(3),x_1,output_stiff,output_rhs);
	dispRHS(output_rhs);
	dispStiffOutput(output_stiff);
}

TEST (AddKnownsToLinearSystemAlgo, for_debugging)
{	
	system("pause");
}

// TODO test the output stiff

// TODO test the outout rhs