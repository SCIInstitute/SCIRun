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
   Last modification : 10/24/2013 ported from SCIRun4
*/



#include <Core/Algorithms/Math/GetSubMatrix.h>
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


bool GetSubMatrixAlgo::run(SparseRowMatrixHandle stiff, DenseColumnMatrixHandle rhs, DenseMatrixHandle x, SparseRowMatrixHandle& output_stiff, 
DenseColumnMatrixHandle& output_rhs) const
{
     
 return true;
}
    
AlgorithmInputName GetSubMatrixAlgo::INPUT_Matrix("INPUT_Matrix");
AlgorithmInputName GetSubMatrixAlgo::Optional_Range_Bounds("Optional_Range_Bounds");
AlgorithmInputName GetSubMatrixAlgo::OUTPUT_Matrix("OUTPUT_Matrix");

AlgorithmOutput GetSubMatrixAlgo::run_generic(const AlgorithmInput & input) const
{ 
  
  auto input_matrix = input.get<MatrixHandle>(INPUT_Matrix);
  auto input_rhs = input.get<DenseMatrix>(Optional_Range_Bounds);
  MatrixHandle output_matrix;
/*
  if (input_lhs->nrows() != input_lhs->ncols()) 
  {
    THROW_ALGORITHM_PROCESSING_ERROR("Stiffness matrix input needs to be a sparse squared matrix! ");
  }

  SparseRowMatrixHandle output_lhs;
  DenseColumnMatrixHandle output_rhs;
  if (!run(input_lhs,input_rhs,input_x,output_lhs,output_rhs))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");
*/
  AlgorithmOutput output;
  output[OUTPUT_Matrix] = output_matrix;

  return output;
}

GetSubMatrixAlgo::GetSubMatrixAlgo() {}
GetSubMatrixAlgo::~GetSubMatrixAlgo() {}

//} // end namespace SCIRun
