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
*/

#include <Core/Algorithms/Math/SortMatrixAlgo.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Math/MiscMath.h>

#include <sstream>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

SortMatrixAlgo::SortMatrixAlgo()
{
  addParameter(Variables::Method, 0);
}


AlgorithmOutput SortMatrixAlgo::run_generic(const AlgorithmInput& input) const
{
  auto input_matrix = input.get<Matrix>(Variables::InputMatrix);
  AlgorithmOutput output;
  
  if (!matrix_is::dense(input_matrix))
  {
    //TODO implement something with sparse
    error("SortMatrix: Currently only works with dense matrices");
    output[Variables::OutputMatrix] = 0;
    return output;
  }
  auto mat  = matrix_cast::as_dense (input_matrix);
  
  DenseMatrixHandle return_matrix;
  
  auto method = get(Variables::Method).toInt();
  
  switch (method)
  {
    case 0:
      SortAscending(mat,return_matrix);
      break;
    case 1:
      SortDescending(mat,return_matrix);
      break;
  }
  
  output[Variables::OutputMatrix] = return_matrix;
  return output;
  
}


bool
SortMatrixAlgo::SortAscending(DenseMatrixHandle input, DenseMatrixHandle& output) const
{
  if (!input)
  {
    error("SortAscending: no input matrix found");
    return false;
  }
  
  size_type nrows = input->nrows();
  size_type ncols = input->ncols();

  output.reset(new DenseMatrix(nrows, 1));
  double *dest = output->data();
  
  for (index_type q=0; q<nrows; q++) dest[q] = 0.0;
  
  DenseMatrixHandle return_matrix;
  
  if (!output)
  {
    error("ApplyRowOperation: could not create output matrix");
    return false;  
  }
 
  
  double* data = input ->data();
  
  size_type m = input->nrows();
  size_type n = input->ncols();


  return true;
}


bool
SortMatrixAlgo::SortDescending(DenseMatrixHandle input, DenseMatrixHandle& output) const
{
  
  if (!input)
  {
    error("SortDescending: no input matrix found");
    return false;
  }
  
  /*
  DenseMatrixHandle t(new DenseMatrix(input->transpose()));
  if(!(ApplyRowOperation(t,t,method))) return false;
  output.reset(new DenseMatrix(t->transpose()));
  
  */
  return true;
} 
