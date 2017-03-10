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

   author: Moritz Dannhauer & Kimia Shayestehfard
   last change: 02/16/17
*/

#include <Modules/Math/ConvertComplexToRealMatrix.h>

#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

using namespace SCIRun::Modules::Math;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

MODULE_INFO_DEF(ConvertComplexToRealMatrix, Converters, SCIRun)

ConvertComplexToRealMatrix::ConvertComplexToRealMatrix() : Module(staticInfo_,false)
{
  INITIALIZE_PORT(InputComplexMatrix);
  INITIALIZE_PORT(OutputRealPartMatrix);
  INITIALIZE_PORT(OutputComplexPartMatrix);
}

void ConvertComplexToRealMatrix::execute()
{
  
  auto input_complex_matrix = getRequiredInput(InputComplexMatrix);

  if (needToExecute())
  {
    update_state(Executing);
    
    auto input_dense = boost::dynamic_pointer_cast<ComplexDenseMatrix>(input_complex_matrix);
    auto input_sparse = boost::dynamic_pointer_cast<ComplexSparseRowMatrix>(input_complex_matrix);
    
    if (!input_dense && !input_sparse)
    {
     error("Number of Rows or Columns are zero");
     return;
    }
    
    auto nr_cols=input_dense->ncols(), nr_rows=input_dense->nrows();
   
    if (nr_rows==0 || nr_cols==0)
    {
     error("Number of Rows or Columns are zero");
     return;
    }
   
    MatrixHandle output_real,output_imag;
    
    if(input_dense)
    {
      output_real = boost::make_shared<DenseMatrix>(input_dense->real());  
      output_imag= boost::make_shared<DenseMatrix>(input_dense->imag()); 
    } else
    {
     output_real = boost::make_shared<SparseRowMatrix>(input_sparse->real());
    output_imag = boost::make_shared<SparseRowMatrix>(input_sparse->imag());
    } 
       
    sendOutput(OutputRealPartMatrix,output_real);
    sendOutput(OutputComplexPartMatrix,output_imag);
  }
}
