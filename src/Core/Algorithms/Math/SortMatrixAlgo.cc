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

//#include <vector>
//#include <iostream>
//#include <iterator>
#include <algorithm>

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
  std::cout<<"running algo"<<std::endl;
  
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
  
  std::cout<<"setup, starting second algorithm"<<std::endl;
  Sort(mat,return_matrix,method);
  
  output[Variables::OutputMatrix] = return_matrix;
  return output;
  
}


bool
SortMatrixAlgo::Sort(DenseMatrixHandle input, DenseMatrixHandle& output,int method) const
{
  if (!input)
  {
    error("SortAscending: no input matrix found");
    return false;
  }
  
  size_type nrows = input->nrows();
  size_type ncols = input->ncols();
  
    std::cout<<"original size " <<nrows<<" rows "<<ncols<<" cols"<<std::endl;

      std::cout<<"set matrix size"<<std::endl;
  output.reset(new DenseMatrix(*input));
  double *data = output->data();

  if (!output)
  {
    error("ApplyRowOperation: could not create output matrix");
    return false;
  }
  
  //output -> resize(nrows*ncols,1);
  //size_type n = output->nrows();
  size_type n = nrows*ncols;
  
  //*data=*d_in;
  
  std::cout<<"resized.  "<<n <<"  rows"<<std::endl;
  size_type nrows_c = input->nrows();
  size_type ncols_c = input->ncols();
  std::cout<<"original size check " <<nrows_c<<" rows "<<ncols_c<<" cols"<<std::endl;
  

  std::cout<<"original matrix = ";
  for (index_type l=0;l<n;l++) std::cout<<data[l]<<" ";
  std::cout<<std::endl;
  
  Quicksort(data,0,n-1);
      std::cout<<"quicksort done"<<std::endl;
  
  std::cout<<"sorted matrix = ";
  for (index_type l=0;l<n;l++) std::cout<<data[l]<<" ";
  std::cout<<std::endl;
  
  if (method==1)
  {
    output.reset(new DenseMatrix(output -> reverse()));
          std::cout<<"descending"<<std::endl;
  }
  
  //std::cout<<"final matrix = ";
  //for (index_type l=0;l<n;l++) std::cout<<data[l]<<" ";
  //std::cout<<std::endl;
  std::cout<<"final matrix = "<<*output<<std::endl;
  
  //output -> resize(nrows,ncols);
  
  return true;
}

bool
SortMatrixAlgo::Quicksort(double* input, index_type lo, index_type hi) const
{
  index_type ind;
  //std::cout<<"lo = "<< lo << ";  hi = "<<hi<<std::endl;
  if (lo<hi)
  {
    
    
    ind=Partition(input,lo,hi);
    //std::cout<<"ind = "<< ind<<std::endl;
    
    //std::cout<<"input = ";
    //for (index_type l=0;l<6;l++) std::cout<<input[l]<<" ";
    //std::cout<<std::endl;
    
    Quicksort(input,lo,ind-1);
    Quicksort(input,ind+1,hi);
  }
  
  return true;
}

index_type
SortMatrixAlgo::Partition(double* input, index_type lo, index_type hi) const
{
  index_type ind=lo;
  
  double pivot = input[hi];
  double tmp;
  //ind = lo;
  //std::cout<<"lo = "<< lo << "; hi = "<<hi<<std::endl;
  for (index_type k=lo;k<hi;k++)
  {
    //std::cout<<"ind = "<<ind<<"; k = "<<k<<std::endl;
    //std::cout<<"comparing "<<input[k]<<" & "<<pivot<<std::endl;
    if (input[k]<=pivot)
    {
      //std::cout<<"swaping "<<ind<<" & "<<k<<std::endl;
      tmp=input[ind];
      input[ind]=input[k];
      input[k]=tmp;
      ind+=1;
    }
  }
  //std::cout<<"swaping "<<ind<<" & "<<hi<<std::endl;
  tmp=input[ind];
  input[ind]=input[hi];
  input[hi]=tmp;
  
  //std::cout<<"ind = "<<ind<<std::endl;
  
  return ind;
}



  
  
