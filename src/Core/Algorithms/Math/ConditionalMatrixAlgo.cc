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

#include <Core/Algorithms/Math/ConditionalMatrixAlgo.h>

#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Parser/ArrayMathEngine.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
//#include <Dataflow/Engine/Python/NetworkEditorPythonAPI.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;
//using namespace SCIRun::Core::Thread;

ConditionalMatrixAlgo::ConditionalMatrixAlgo()
{
  //set parameter defaults for UI
    
  addOption(Variables::Operator, "boolop", "boolop|andop|orop|lessop|lesseqop|eqop|greateqop|greatop");
  addOption(Variables::Method, "value", "value|size|norm");
  addOption(Variables::ObjectInfo, "value", "value|size|norm");
  addOption(Variables::FormatString, "first", "null|first|second|third|quit");
  addOption(Variables::FunctionString, "null", "null|first|second|third|quit");
}


AlgorithmOutput ConditionalMatrixAlgo::run(const AlgorithmInput& input) const
{
  auto matrixa = input.get<Matrix>(Variables::FirstMatrix);
  auto matrixb = input.get<Matrix>(Variables::SecondMatrix);
  auto possout = input.get<Matrix>(Variables::InputMatrix);
    
  AlgorithmOutput output;
  
  //sparse support not fully implemented yet.
  if (!matrixIs::dense(matrixa) || (matrixb && !matrixIs::dense(matrixb)))
  {
    //TODO implement something with sparse
    error("ConditionalMatrix: Currently only works with dense matrices");
    output[Variables::OutputMatrix] = 0;
    output[Variables::Solution] = 0;
    return output;
  }
  auto mata  = castMatrix::toDense (matrixa);
  auto matb  = castMatrix::toDense (matrixb);
    
  MatrixHandle out_matrix;
  DenseMatrixHandle cond_matrix;
  auto cmat = castMatrix::toDense(cond_matrix);
  double *data = cmat->data();
  
  bool cond_state= false;
  
  //pull parameter from UI
  std::string valoptA = getOption(Variables::Method);
  std::string valoptB = getOption(Variables::ObjectInfo);
  std::string cond_statement = getOption(Variables::Operator);
  std::string then_result = getOption(Variables::FormatString);
  std::string else_result = getOption(Variables::FunctionString);
  
  std::cout<<"valoptA ="<<valoptA<<std::endl;
  std::cout<<"valoptB ="<<valoptB<<std::endl;
  std::cout<<"cond_statement ="<<cond_statement<<std::endl;
  std::cout<<"then_result ="<<then_result<<std::endl;
  std::cout<<"else_result ="<<else_result<<std::endl;
  
  if (!runImpl(mata, matb, valoptA, valoptB, cond_statement, cond_state))
    THROW_ALGORITHM_PROCESSING_ERROR("Error running conditional matrix algorithm");
  
  
  
  if (cond_state)
  {
    data[0] = 1;
    return_value(out_matrix,then_result,matrixa,matrixb,possout);
  }
  else if (!cond_state)
  {
    data[0] = 0;
    return_value(out_matrix,else_result,matrixa,matrixb,possout);
  }
  else
    THROW_ALGORITHM_PROCESSING_ERROR("Error running conditional matrix algorithm");
  
  output[Variables::OutputMatrix] = out_matrix;
  output[Variables::Solution] = cond_matrix;
  return output;
}

bool ConditionalMatrixAlgo::runImpl(DenseMatrixHandle mata, DenseMatrixHandle matb, std::string valoptA, std::string valoptB, std::string cond_statement, bool cond_state) const
{
  size_t nrA = mata->nrows();
  size_t ncA = mata->ncols();
  size_t nrB = matb->nrows();
  size_t ncB = matb->ncols();
  
  DenseMatrixHandle compa, compb;
  double *data = compa->data();
  double *datb = compb->data();
  
  if ((valoptA == "size" && valoptB == "norm") || (valoptB == "size" && valoptA == "norm"))
  {
    THROW_ALGORITHM_PROCESSING_ERROR("Cannot compare size of one matrix to norm of the other");
    return false;
  }
  else if (valoptA == "size" && valoptB == "size")
  {
    data[0] = static_cast<double>(nrA);
    data[1] = static_cast<double>(ncA);
    
    datb[0] = static_cast<double>(nrB);
    datb[1] = static_cast<double>(ncB);
  }
  else if (valoptA == "size" && valoptB == "value")
  {
    if (nrB*ncB != 2)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Must compare size of one matrix to matrix with 2 elements");
      return false;
    }
    data[0] = static_cast<double>(nrA);
    data[1] = static_cast<double>(ncA);
    
    compb = matb;
  }
  else if (valoptB == "size" && valoptA == "value")
  {
    if (nrA*ncA != 2)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Must compare size of one matrix to matrix with 2 elements");
      return false;
    }
    datb[0] = static_cast<double>(nrB);
    datb[1] = static_cast<double>(ncB);
    
    compa = mata;
  }
  else if (valoptA == "norm" && valoptB == "value")
  {
    if (nrB*ncB != 1)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Must compare size of one matrix to matrix with 2 elements");
      return false;
    }
    data[0] = ComputeNorm(mata);
    compb = matb;
  }
  else if (valoptB == "norm" && valoptA == "value")
  {
    if (nrA*ncA != 1)
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Must compare size of one matrix to matrix with 2 elements");
      return false;
    }
    datb[0] = ComputeNorm(matb);
    compa = mata;
  }
  else if (valoptA == "norm" && valoptB == "norm")
  {
    data[0] = ComputeNorm(mata);
    datb[0] = ComputeNorm(matb);
  }
  else if (valoptA == "value" && valoptB == "value")
  {
    if ((nrA!=nrB) || (ncA!=ncB))
    {
      THROW_ALGORITHM_PROCESSING_ERROR("Matrix must be of the same size when comparing values");
      return false;
    }
    compa = mata;
    compb = matb;
  }
  else THROW_ALGORITHM_PROCESSING_ERROR("Choosen options do not make sense.");
  
  cond_state = CompareMatrix(compa, compb, cond_statement);
  
  
  return true;
    
}


double ConditionalMatrixAlgo::ComputeNorm(DenseMatrixHandle mat) const
{
  return 0;
}

bool ConditionalMatrixAlgo::CompareMatrix(DenseMatrixHandle mata, DenseMatrixHandle matb,std::string cond_statement) const
{
  return true;
}

// determine the output
 bool ConditionalMatrixAlgo::return_value(MatrixHandle out_matrix,std::string result_statement, MatrixHandle first, MatrixHandle second, MatrixHandle possout) const
{
  std::string exit_status = "running";
  
  if (result_statement == "first") out_matrix = first;
  else if (result_statement == "second" && second) out_matrix = second;
  else if (result_statement == "third" && possout) out_matrix = possout;
  else if (result_statement == "null") out_matrix = 0;
  else if (result_statement == "quit")
  {
    out_matrix = 0;
    exit_status = "supposed to quit";
    std::cout<<"exit status:"<<exit_status<<std::endl;
  }
  else
  {
    out_matrix = 0;
    THROW_ALGORITHM_PROCESSING_ERROR("Error running conditional matrix algorithm");
  }
}


//bool
//ConditionalMatrixAlgo::Sort(DenseMatrixHandle input, DenseMatrixHandle& output,int method) const
//{
//  if (!input)
//  {
//    error("SortAscending: no input matrix found");
//    return false;
//  }
//  //get size of original matrix
//  size_type nrows = input->nrows();
//  size_type ncols = input->ncols();
//  //copy original matrix for processing
//  output.reset(new DenseMatrix(*input));
//  //pointer to matrix data
//  double *data = output->data();
//
//  if (!output)
//  {
//    error("ApplyRowOperation: could not create output matrix");
//    return false;
//  }
//  
//  size_type n = nrows*ncols;
//  //call the sorting functions
//  Quicksort(data,0,n-1);
//  
//  if (method==1)
//  {
//    //if set to descending, reverse the order.
//    output.reset(new DenseMatrix(output -> reverse()));
//  }
//  return true;
//}
//
//bool
//ConditionalMatrixAlgo::Quicksort(double* input, index_type lo, index_type hi) const
//{
//  //splits matrix based on Partition function
//  index_type ind;
//  if (lo<hi)
//  {
//    ind=Partition(input,lo,hi);
//    Quicksort(input,lo,ind-1);
//    Quicksort(input,ind+1,hi);
//  }
//  return true;
//}
//
//index_type
//ConditionalMatrixAlgo::Partition(double* input, index_type lo, index_type hi) const
//{
//  // places the last entry in its proper place in relation to the other
//  // entries, ie, smaller values before and larger values after.
//  index_type ind=lo;
//  
//  double pivot = input[hi];
//  double tmp;
//  for (index_type k=lo;k<hi;k++)
//  {
//    if (input[k]<=pivot)
//    {
//      tmp=input[ind];
//      input[ind]=input[k];
//      input[k]=tmp;
//      ind+=1;
//    }
//  }
//  tmp=input[ind];
//  input[ind]=input[hi];
//  input[hi]=tmp;
//  return ind;
//}



  
  
