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


#ifndef CORE_ALGORITHMS_FINITEELEMENTS_AddKnownsToLinearSystem_H
#define CORE_ALGORITHMS_FINITEELEMENTS_AddKnownsToLinearSystem_H 1

//! Datatypes that the algorithm uses
#include <Core/Datatypes/MatrixFwd.h>
//! Base class for algorithm
#include <Core/Algorithms/Base/AlgorithmBase.h>
/// for Windows support
#include <Core/Algorithms/Math/share.h>

namespace SCIRun {
	namespace Core {
		namespace Algorithms {
			namespace Math {

class SCISHARE AddKnownsToLinearSystemAlgo : public AlgorithmBase
{
  public:

    static AlgorithmInputName LHS_Matrix;
    static AlgorithmInputName RHS_Vector;
    static AlgorithmInputName X_Vector;
    static AlgorithmOutputName OutPutLHSMatrix;
    static AlgorithmOutputName OutPutRHSVector;

  bool run(Datatypes::SparseRowMatrixHandle stiff, Datatypes::DenseColumnMatrixHandle rhs, Datatypes::DenseMatrixHandle x, Datatypes::SparseRowMatrixHandle& output_stiff, Datatypes::DenseColumnMatrixHandle& output_rhs) const;
  virtual AlgorithmOutput run_generic(const AlgorithmInput &) const;
  
}; // end namespace SCIRun

			}}}}
#endif 
