/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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
   
   Author            : Moritz Dannhauer
   Author            : Spencer Frisby
   Last modification : 4/23/2014
   
*/

#include <Core/Algorithms/Math/AddKnownsToLinearSystem.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>

#include <Core/GeometryPrimitives/Point.h>
#include <Core/GeometryPrimitives/Tensor.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;

bool AddKnownsToLinearSystemAlgo::run(SparseRowMatrixHandle stiff,
									  DenseColumnMatrixHandle rhs,
									  DenseMatrixHandle x,
									  SparseRowMatrixHandle& output_stiff,
									  DenseColumnMatrixHandle& output_rhs) const
{
  SparseRowMatrixFromMap::Values additionalData;
	
	// Making sure the stiff matrix (left hand side) is symmetric
  if (!isSymmetricMatrix(*stiff))
    THROW_ALGORITHM_INPUT_ERROR("matrix A is not symmetrical");
    
	// Storing the number of columns in m and rows in n from the stiff matrix, m == n
  unsigned int m = static_cast<unsigned int>(stiff->ncols()), 
               n = static_cast<unsigned int>(stiff->nrows());
	
	// Checking if the rhs matrix is allocated and that the dimenions agree with the stiff matrix
	if (!rhs)
	{
        THROW_ALGORITHM_INPUT_ERROR("Could not allocate new b matrix");
	}
	else if ( !(((rhs->ncols() == m) && (rhs->nrows() == 1)) || ((rhs->ncols() == 1) && (rhs->nrows() == m))) )
	{
		THROW_ALGORITHM_INPUT_ERROR("The dimensions of vector b do not match the dimensions of matrix A"); 
  }
    
	// casting rhs to be a column
	auto rhsCol = matrix_cast::as_column(rhs);
  if (!rhsCol) rhsCol = matrix_convert::to_column(rhs);
	
	// Checking if x matrix was given and that the dimenions agree with the stiff matrix
  if (!x)
	{
		THROW_ALGORITHM_INPUT_ERROR("No x vector was given");
	}
	else if ( !(((x->ncols() == m) && (x->nrows() == 1)) || ((x->ncols() == 1) && (x->nrows() == m))) )
	{
		THROW_ALGORITHM_INPUT_ERROR("The dimensions of vector x do not match the dimensions of matrix A");
  } 
	
	// casting x to be a column
	auto xCol = matrix_cast::as_column(x);
	if (!xCol) xCol = matrix_convert::to_column(x);  
    
	// cnt used for updating the progress bar
	index_type cnt = 0;
 
	bool just_copying_inputs = true;

  // performs calculation adjustments for setting row and col values to zero
  for (index_type p=0; p<m; p++)
	{
		// making sure the rhs vector is finite
		if (!IsFinite((*rhsCol)[p]))
			THROW_ALGORITHM_INPUT_ERROR("NaN exist in the b vector");
		if (IsFinite((*x).coeff(p)))
		{
      just_copying_inputs = false;
			for (index_type i=0; i<m; i++)
			{
				if (i!=p)
				{
					(*rhsCol).coeffRef(i) += -(*stiff).coeff(i,p) * (*xCol).coeff(p);
					additionalData[i][p] = 0.0;
          additionalData[p][i] = 0.0;
				}
				else
				{
					additionalData[p][p] = 1.0;
				}
			}
		}
		cnt++;
		if (cnt == 10)
		{
			cnt = 0;
			update_progress((double)p/m);
		}
	}
  
  // assigns value for right hand side vector
  for (index_type p=0; p<m; p++)
	{
		if (IsFinite((*x).coeff(p)))
		{
      just_copying_inputs = false;
			for (index_type i=0; i<m; i++)
			{
				if (i == p)
				{
					(*rhsCol)[p] = (*xCol).coeff(p);
				}
			}
		}
		cnt++;
		if (cnt == 10)
		{
			cnt = 0;
			update_progress((double)p/m);
		}
	}
  
  
	if (just_copying_inputs)
    remark("X vector does not contain any knowns! Copying inputs to outputs.");
	
	output_stiff = SparseRowMatrixFromMap::appendToSparseMatrix(m, n, *stiff, additionalData);
	output_rhs = rhsCol;
	output_stiff->makeCompressed();
	return true;
}

AlgorithmInputName AddKnownsToLinearSystemAlgo::LHS_Matrix("LHS_Matrix");
AlgorithmInputName AddKnownsToLinearSystemAlgo::RHS_Vector("RHS_Vector");
AlgorithmInputName AddKnownsToLinearSystemAlgo::X_Vector("X_Vector");
AlgorithmInputName AddKnownsToLinearSystemAlgo::OutPutLHSMatrix("OutPutLHSMatrix");
AlgorithmInputName AddKnownsToLinearSystemAlgo::OutPutRHSVector("OutPutRHSVector");

AlgorithmOutput AddKnownsToLinearSystemAlgo::run_generic(const AlgorithmInput & input) const
{ 
	auto input_lhs = input.get<SparseRowMatrix>(LHS_Matrix);
	auto input_rhs = input.get<DenseColumnMatrix>(RHS_Vector);
	auto input_x = input.get<DenseMatrix>(X_Vector);

	if (input_lhs->nrows() != input_lhs->ncols()) 
	{
		THROW_ALGORITHM_INPUT_ERROR("Stiffness matrix input needs to be a sparse squared matrix! ");
	}

	SparseRowMatrixHandle output_lhs;
	DenseColumnMatrixHandle output_rhs;
  
	if (!run(input_lhs,input_rhs,input_x,output_lhs,output_rhs))
		THROW_ALGORITHM_INPUT_ERROR("False returned on legacy run call.");

	AlgorithmOutput output; 
	output[OutPutLHSMatrix] = output_lhs;
	output[OutPutRHSVector] = output_rhs;

	return output;
}
