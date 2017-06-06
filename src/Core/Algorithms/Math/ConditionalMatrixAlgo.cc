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
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

ConditionalMatrixAlgo::ConditionalMatrixAlgo()
{
  //set parameter defaults for UI
  addOption(Parameters::Operator, "Anonzero", "boolop|andop|orop|lessop|lesseqop|eqop|greateqop|greatop");
  addOption(Parameters::Method, "value", "value|size");
  addOption(Parameters::Method2, "value", "value|size");
  addOption(Parameters::OutputFormat, "null" "null,first,second,third");
  addOption(Parameters::OutputFormat2, "null" "null,first,second,third");
}


AlgorithmOutput ConditionalMatrixAlgo::run(const AlgorithmInput& input) const
{
  auto matrixa = input.get<Matrix>(Variables::MatrixA);
  auto matrixb = input.get<Matrix>(Variables::MatrixB);
  auto possout = input.get<Matrix>(Variables::PossibleOutput);
    
  AlgorithmOutput output;
  
  //sparse support not fully implemented yet.
  if (!matrix_is::dense(input_matrix))
  {
    //TODO implement something with sparse
    error("ConditionalMatrix: Currently only works with dense matrices");
    output[Variables::OutputMatrix] = null;
    output[Variables::ConditionMatrix] = null;
    return output;
  }
  auto mat  = matrix_cast::as_dense (input_matrix);
  DenseMatrixHandle out_matrix;
  
  //pull parameter from UI
    std::string valoptA = getOption(Parameters::Method);
    std::string valoptB = getOption(Parameters::Method2);
    std::string cond_statement = getOption(Parameters::Operator);
    std::string then_result = getOption(Parameters::OutputFormat);
    std::string else_result = getOption(Parameters::OutputFormat2);
  
  
  output[Variables::OutputMatrix] = null;
  output[Variables::ConditionMatrix] = null;
  return output;
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



  
  
