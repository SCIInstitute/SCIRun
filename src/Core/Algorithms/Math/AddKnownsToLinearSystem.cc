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
   Last modification : 10/24/2013 converted from SCIRun4 and improved
*/



#include <Core/Algorithms/Math/AddKnownsToLinearSystem.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
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

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Geometry;


bool AddKnownsToLinearSystemAlgo::run(Datatypes::SparseRowMatrixHandle stiff, Datatypes::DenseMatrixHandle rhs, Datatypes::DenseMatrixHandle x, Datatypes::SparseRowMatrixHandle& output_stiff, Datatypes::DenseMatrixHandle&
output_rhs) const
{
    SparseRowMatrixFromMap::Values additionalData;

    if (!isSymmetricMatrix(stiff))
    {
      THROW_ALGORITHM_PROCESSING_ERROR("LHS matrix is not symmetrical");
    } 
       
    unsigned int m=static_cast<unsigned int>(stiff->ncols()),n=static_cast<unsigned int>(stiff->nrows());
    
    if (!rhs) 
      {
        THROW_ALGORITHM_PROCESSING_ERROR("Could not allocate new b matrix");  
      } else
       if ( !(((rhs->ncols() == m) && (rhs->nrows() == 1))) || !(((rhs->ncols() == 1) && (rhs->nrows() == m))))
       {
	 THROW_ALGORITHM_PROCESSING_ERROR("The dimensions of matrix b do not match the dimensions of matrix A"); 
       }
    
    auto rhsCol = matrix_cast::as_column(rhs);
    if (!rhsCol) rhsCol = matrix_convert::to_column(rhs); 
    
    
    if (!x)
     {
      THROW_ALGORITHM_PROCESSING_ERROR("No x vector was given");
     } else
       if ( !(((x->ncols() == m) && (x->nrows() == 1)) || ((x->ncols() == 1) && (x->nrows() == m))) )
       {
         THROW_ALGORITHM_PROCESSING_ERROR("The dimensions of matrix x do not match the dimensions of matrix A");
       } 
      
     auto xCol = matrix_cast::as_column(x);
     if (!xCol) rhsCol = matrix_convert::to_column(x);  
       
     index_type cnt = 0;
 
     index_type knowns = 0;
     index_type unknowns = 0;
     
     for (index_type p=0; p<m;p++)
     {
      if (std::isnormal((*rhsCol).coeff(p)))
      {  
        knowns++;
	for (index_type i=0; i<m; i++)
	{
	  if (i!=p) 
	  {
	   (*rhsCol).coeffRef(i) -= (*stiff).coeff(i,p) * (*xCol).coeff(p); 
	    additionalData[i][p]=0.0;
	  }
	  else
	  {          
	   (*rhsCol)[p] = (*xCol).coeff(p);
	    additionalData[p][p]=1.0; 
	  }	    
	}	           
      } else
      {
        unknowns++;
      }
      cnt++;
      if (cnt == 1000)
      {
        cnt = 0;
	update_progress((double)p/m);
      }
     } 

     output_stiff = SparseRowMatrixFromMap::appendToSparseMatrix(m, n, *stiff, additionalData);

     output_rhs = matrix_cast::as_dense(rhsCol);
     output_stiff->makeCompressed();
     
 return true;
}
    
AlgorithmInputName AddKnownsToLinearSystemAlgo::LHS_Matrix("LHS_Matrix");
AlgorithmInputName AddKnownsToLinearSystemAlgo::RHS_Vector("RHS_Vector");
AlgorithmInputName AddKnownsToLinearSystemAlgo::X_Vector("X_Vector");
AlgorithmInputName AddKnownsToLinearSystemAlgo::OutPutLHSMatrix("OutPutLHSMatrix");
AlgorithmInputName AddKnownsToLinearSystemAlgo::OutPutRHSMatrix("OutPutRHSMatrix");

AlgorithmOutput AddKnownsToLinearSystemAlgo::run_generic(const AlgorithmInput & input) const
{ 
  
  auto input_lhs = input.get<SparseRowMatrix>(LHS_Matrix);
  auto input_rhs = input.get<DenseMatrix>(RHS_Vector);
  auto input_x = input.get<DenseMatrix>(X_Vector);

  if (input_lhs->nrows() != input_lhs->ncols()) 
  {
    THROW_ALGORITHM_PROCESSING_ERROR("Stiffness matrix input needs to be a sparse squared matrix! ");
  }

  SparseRowMatrixHandle output_lhs;
  DenseMatrixHandle output_rhs;
  if (!run(input_lhs,input_rhs,input_x,output_lhs,output_rhs))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[OutPutLHSMatrix] = output_lhs;
  output[OutPutRHSMatrix] = output_rhs;

  return output;
}

AddKnownsToLinearSystemAlgo::AddKnownsToLinearSystemAlgo() {}
AddKnownsToLinearSystemAlgo::~AddKnownsToLinearSystemAlgo() {}

//} // end namespace SCIRun
