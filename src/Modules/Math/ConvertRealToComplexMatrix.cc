/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

   author: Moritz Dannhauer
   last change: 05/06/16
*/

#include <Modules/Math/ConvertRealToComplexMatrix.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

const ModuleLookupInfo ConvertRealToComplexMatrix::staticInfo_("ConvertRealToComplexMatrix", "Converters", "SCIRun");

ConvertRealToComplexMatrix::ConvertRealToComplexMatrix() : Module(staticInfo_,false)
{
  INITIALIZE_PORT(RealPartMatrix);
  INITIALIZE_PORT(ComplexPartMatrix);
  INITIALIZE_PORT(Output);
}


void ConvertRealToComplexMatrix::execute()
{
  auto input_matrix1 = getRequiredInput(RealPartMatrix);
  auto input_matrix2 = getRequiredInput(ComplexPartMatrix);
  
  if (needToExecute())
  {
    update_state(Executing);
    
    if(!input_matrix1 || !input_matrix2)
    {
     error("One of the input matrices is empty.");
     return;					    
    }
    
    if (!matrixIs::dense(input_matrix1))
    {
      //TODO implement something with sparse
      error("Currently only works with dense matrices (check first module input).");
      return;
    }
    
    if (!matrixIs::dense(input_matrix2))
    {
      //TODO implement something with sparse
      error("Currently only works with dense matrices (check second module input).");
      return;
    }
    
    auto nr_cols_mat1=input_matrix1->ncols(), nr_rows_mat1=input_matrix1->nrows(), 
         nr_cols_mat2=input_matrix2->ncols(), nr_rows_mat2=input_matrix2->nrows();
    
    if(nr_cols_mat1!=nr_cols_mat2 || nr_rows_mat1!=nr_rows_mat2)
    {
     error("Input matrices do not have same number of rows or columns.");
     return;
    }
    
    auto out(boost::make_shared<ComplexDenseMatrix>(nr_rows_mat1,nr_cols_mat1));
    auto mat1 = castMatrix::toDense (input_matrix1), mat2 = castMatrix::toDense (input_matrix2);
    
    for(auto i=0; i<nr_rows_mat1; i++)
     for(auto j=0; j<nr_cols_mat1; j++)
        (*out)(i,j) = complex((*mat1)(i,j),(*mat2)(i,j));  
    
    sendOutput(Output,out);
  }
}
