/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


/// @todo Documentation Modules/Legacy/Math/AddKnownsToLinearSystem.h

#ifndef MODULES_LEGACY_Math_AddKnownsToLinearSystem_H__
#define MODULES_LEGACY_Math_AddKnownsToLinearSystem_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Math/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Math {

      class SCISHARE AddKnownsToLinearSystem : public Dataflow::Networks::Module,
        public Has3InputPorts<MatrixPortTag, MatrixPortTag, MatrixPortTag>,
        public Has2OutputPorts<MatrixPortTag, MatrixPortTag>
      {
      public:
        AddKnownsToLinearSystem();
        virtual void setStateDefaults() override {}
        virtual void execute() override;

        INPUT_PORT(0, LHS_Matrix, SparseRowMatrix);
        INPUT_PORT(1, RHS_Vector, DenseMatrix);
	      INPUT_PORT(2, X_Vector, DenseMatrix);
        OUTPUT_PORT(0, OutPutLHSMatrix, SparseRowMatrix);
	      OUTPUT_PORT(1, OutPutRHSVector, DenseColumnMatrix);
	      MODULE_TRAITS_AND_INFO(ModuleHasAlgorithm)
      };

    }
  }
}

#endif
