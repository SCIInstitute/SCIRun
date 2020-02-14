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


/// @todo Documentation Modules/Legacy/FiniteElements/BuildTDCSMatrix.h

#ifndef MODULES_LEGACY_FINITEELEMENTS_BUILDTDCSMATRIX_H__
#define MODULES_LEGACY_FINITEELEMENTS_BUILDTDCSMATRIX_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/FiniteElements/share.h>

namespace SCIRun {
  namespace Modules {
    namespace FiniteElements {

      class SCISHARE BuildTDCSMatrix : public Dataflow::Networks::Module,
        public Has6InputPorts<MatrixPortTag, FieldPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag, MatrixPortTag>,
        public Has1OutputPort<MatrixPortTag>
      {
      public:
        BuildTDCSMatrix();

        void setStateDefaults() override
        {}

        void execute() override;

        INPUT_PORT(0, FEM_Stiffness_Matrix, SparseRowMatrix);
        INPUT_PORT(1, FEM_Mesh, Field);
        INPUT_PORT(2, Electrode_Element, DenseMatrix);
        INPUT_PORT(3, Electrode_Element_Type, DenseMatrix);
        INPUT_PORT(4, Electrode_Element_Definition, DenseMatrix);
        INPUT_PORT(5, Contact_Impedance, DenseMatrix);
        OUTPUT_PORT(0, TDCSMatrix, Matrix);

        MODULE_TRAITS_AND_INFO(ModuleHasAlgorithm)
      };

    }
  }
}

#endif
