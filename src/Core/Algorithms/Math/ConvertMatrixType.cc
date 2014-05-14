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

#include <Core/Algorithms/Math/ConvertMatrixType.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
using namespace SCIRun::Core::Datatypes;

ConvertMatrixTypeAlgorithm::ConvertMatrixTypeAlgorithm()
{
  addParameter(PassThrough(), true);
  addParameter(ConvertToColumnMatrix(), false);
  addParameter(ConvertToDenseMatrix(), false);
  addParameter(ConvertToSparseRowMatrix(), false);
}

MatrixHandle ConvertMatrixTypeAlgorithm::run(MatrixHandle input_matrix) const
{
  if (!input_matrix)
  {
    THROW_ALGORITHM_INPUT_ERROR("No input matrix");    
  }
 
  MatrixHandle omH;
  std::ostringstream ostr1,ostr2;
  
  ostr1 << "Dimensions: (" << input_matrix->nrows() << "," << input_matrix->ncols() << ")";
  remark(ostr1.str());
  
  if(matrix_is::dense(input_matrix))
  {
    ostr2 << "Input Matrix Type: DENSE MATRIX";
  } else
  if (matrix_is::column(input_matrix))
  {
    ostr2 << "Input Matrix Type: COLUMN MATRIX";
  } else
  if (matrix_is::sparse(input_matrix))
  {
    ostr2 << "Input Matrix Type: SPARSE MATRIX";
  } else
  {
    THROW_ALGORITHM_INPUT_ERROR("Unknown input matrix type");      
  }
  
  remark(ostr2.str());
  
  if (get(PassThrough()).getBool()) 
  {
     return input_matrix;
  } else
  if (get(ConvertToColumnMatrix()).getBool() && !matrix_is::column(input_matrix))
  {
     if (input_matrix->ncols()!=1)
     {
       THROW_ALGORITHM_INPUT_ERROR("Input matrix needs to have a single column to be converted to column matrix type.");  
     }
     DenseColumnMatrixHandle output = matrix_convert::to_column(input_matrix);
     if (!output) 
      {
       THROW_ALGORITHM_INPUT_ERROR("Conversion to column matrix failed.");    
      }
      return output;
   } else
   if (get(ConvertToDenseMatrix()).getBool() && !matrix_is::dense(input_matrix))
   {
     auto output = matrix_convert::to_dense(input_matrix);
     if (!output) 
      {
       THROW_ALGORITHM_INPUT_ERROR("Conversion to dense matrix failed.");    
      }
     return output;
    } else
    if (get(ConvertToSparseRowMatrix()).getBool() && !matrix_is::sparse(input_matrix))
    {
     auto output = matrix_convert::to_sparse(input_matrix);
     if (!output) 
      {
       THROW_ALGORITHM_INPUT_ERROR("Conversion to sparse matrix failed.");    
      }
     return output;
    }  
    { 
     remark(" Datatype unknown or input and output data type are equal. Passing input matrix through. ");
     return input_matrix;
    }
    
}

AlgorithmInputName ConvertMatrixTypeAlgorithm::InputMatrix("InputMatrix");
AlgorithmOutputName ConvertMatrixTypeAlgorithm::ResultMatrix("ResultMatrix");
AlgorithmParameterName ConvertMatrixTypeAlgorithm::PassThrough() { return AlgorithmParameterName("PassThrough"); }
AlgorithmParameterName ConvertMatrixTypeAlgorithm::ConvertToColumnMatrix() { return AlgorithmParameterName("ConvertToColumnMatrix"); }
AlgorithmParameterName ConvertMatrixTypeAlgorithm::ConvertToDenseMatrix() { return AlgorithmParameterName("ConvertToDenseMatrix"); }
AlgorithmParameterName ConvertMatrixTypeAlgorithm::ConvertToSparseRowMatrix() { return AlgorithmParameterName("ConvertToSparseRowMatrix"); }

AlgorithmOutput ConvertMatrixTypeAlgorithm::run_generic(const AlgorithmInput& input) const
{
  auto input_matrix = input.get<Matrix>(Variables::InputMatrix);

  MatrixHandle output_matrix = run(input_matrix);
  
  AlgorithmOutput output;  
  output[Variables::ResultMatrix] = output_matrix;
  
  return output;
}
