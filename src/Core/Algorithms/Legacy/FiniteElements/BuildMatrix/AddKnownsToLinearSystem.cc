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
   Last modification : 10/24/2013 converted from SCIRun4 and extended
*/



#include <Core/Algorithms/Legacy/FiniteElements/BuildMatrix/AddKnownsToLinearSystem.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/DenseMatrix.h>
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

class TDCSMatrixBuilder
{
public:
  TDCSMatrixBuilder() 
  {
      
  }


private:
  /*VMesh *mesh_;

  std::vector<unsigned int> electrodes_;
  
  SCIRun::Core::Datatypes::SparseRowMatrixHandle stiffnessMatrix_, tdcs_;
  SCIRun::Core::Datatypes::DenseMatrixHandle electrodeElements_, electrodeElementType_, electrodeElementDefinition_, contactImpedanceInformation_;
  unsigned int electrodeElementsRows_, electrodeElementTypeRows_, electrodeElementDefinitionRows_;
  unsigned int electrodeElementsCols_, electrodeElementTypeCols_, electrodeElementDefinitionCols_;
  unsigned int mesh_nrnodes_, number_electrodes_;*/

};

bool AddKnownsToLinearSystemAlgo::run(Datatypes::SparseRowMatrixHandle stiff, Datatypes::DenseMatrixHandle mesh, Datatypes::DenseMatrixHandle rhs, Datatypes::SparseRowMatrixHandle& output_stiff, Datatypes::DenseMatrixHandle&
output_rhs) const
{
    SparseRowMatrixFromMap::Values additionalData;
    unsigned int m=static_cast<unsigned int>(stiff->ncols()),n=static_cast<unsigned int>(stiff->nrows());
    additionalData[0][0]=1;
    output_stiff = SparseRowMatrixFromMap::appendToSparseMatrix(m, n, *stiff, additionalData);

    
    output_rhs = rhs;
    output_stiff->makeCompressed();
    //std::cout<< " 1 " <<std::endl; 
    //VField* hField = mesh->vfield();
   // std::cout<< " 2 " <<std::endl; 
  
 /*
  if (a_in.get_rep() == 0)
  {
    error("No input matrix A");
    algo_end();
    return (false);
  }

  if (! matrix_is::sparse(a_in))
  {
    if (convert_matrix_types)
    {
      warning("Matrix A is not sparse - converting.");
      MatrixHandle tmp = a_in->sparse();
      a_in = tmp;
    }
    else
    {
      error("Matrix A is not sparse");
      algo_end();
      return (false);
    }
  }

  if (a_in->ncols() != a_in->nrows())
  {
    error("Matrix A is not square");
    return (false);  
  }

  // TODO: if matrix A is sparse (checked and/or converted),
  // then is this call to sparse needed?
  // If the matrix is already sparse, then the pointer to the
  // sparse matrix object is returned...
  a_out = a_in->sparse();
  a_out.detach();  

  size_type m = a_in->nrows();
  
  // Check and process RHS vector
  
  if (b_in.get_rep() == 0)
  {
    b_out = new DenseMatrix(m,1);
    if (b_out.get_rep() == 0)
    {
      error("Could not allocate new b matrix");
      algo_end(); return (false);       
    }
    
    b_out->zero();
  }
  else
  {
    if (b_in->nrows() * b_in->ncols() != m)
    {
      error("The dimensions of matrix b do not match the dimensions of matrix A");
      algo_end(); return (false); 
    }
  
    b_out = b_in->dense();
    b_out.detach();
    
    if (b_out.get_rep() == 0)
    {
      error("Could not allocate new b matrix");
      algo_end(); return (false);      
    }
  }
  
  if (x_known.get_rep() == 0)
  {
    error("No x vector was given");
    algo_end(); return (false);    
  }
  
  if (x_known->nrows() * x_known->ncols() != m)
  {
    error("The dimensions of matrix x do not match the dimensions of matrix A");
    algo_end(); return (false);           
  }

  MatrixHandle Temp;
  
  Temp = x_known->dense();
  x_known = Temp;
    
  if (x_known.get_rep() == 0)
  {
    error("Could not allocate x matrix");
    algo_end(); return (false);      
  }
  
  double* x_ptr = x_known->get_data_pointer();
  double* b_ptr = b_out->get_data_pointer();
  
  index_type *idx_nzero; 
  double *val_nzero = 0;
  size_type  idx_nzerosize;
  index_type idx_nzerostride;
  
  index_type* rows = a_out->get_rows();
  index_type* columns = a_out->get_cols();
  double* a = a_out->get_vals();
  
  index_type cnt = 0;
  
  index_type knowns = 0;
  index_type unknowns = 0;

  for (index_type p=0; p<m;p++)
  {
    if (airExists(x_ptr[p]))
    {
      knowns++;
      a_out->getRowNonzerosNoCopy(p, idx_nzerosize, idx_nzerostride, idx_nzero, val_nzero);
      
      for (index_type i=rows[p]; i<rows[p+1]; i++)
      {
        index_type j = columns[i];
        b_ptr[j] += - x_ptr[p] * a[i]; 
      }    
    }
    else
    {
      unknowns++;
    }
    cnt++;
    if (cnt == 1000)
    {
      cnt = 0;
      update_progress(p, 2*m);
    }
  }
  
  cnt = 0;
  for (index_type p=0; p<m;p++)
  {
    if (airExists(x_ptr[p]))
    {
      for (index_type i=rows[p]; i<rows[p+1]; i++)
      {
        index_type j = columns[i];
        a_out->put(p, j, 0.0);
        a_out->put(j, p, 0.0); 
      }
      
      //! updating dirichlet node and corresponding entry in b_ptr
      a_out->put(p, p, 1.0);
      b_ptr[p] = x_ptr[p];
    }
    cnt++;
    if (cnt == 1000)
    {
      cnt = 0;
      update_progress(p+m, 2*m);
    }
  }

  algo_end();
  return (true);
 
 */
  
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
  std::cout<< " 7 " <<std::endl;
  AlgorithmOutput output;
  output[OutPutLHSMatrix] = output_lhs;
  output[OutPutRHSMatrix] = output_rhs;
  std::cout<< " 8 " <<std::endl;
  return output;
}

AddKnownsToLinearSystemAlgo::AddKnownsToLinearSystemAlgo() {}
AddKnownsToLinearSystemAlgo::~AddKnownsToLinearSystemAlgo() {}

//} // end namespace SCIRun
