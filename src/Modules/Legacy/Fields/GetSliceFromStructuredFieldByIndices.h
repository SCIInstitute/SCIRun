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


#ifndef MODULES_LEGACY_FIELDS_GetSliceFromStructuredFieldByIndices_H__
#define MODULES_LEGACY_FIELDS_GetSliceFromStructuredFieldByIndices_H__

#include <Dataflow/Network/Module.h>
#include <Core/Algorithms/Base/AlgorithmMacros.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Fields {
        ALGORITHM_PARAMETER_DECL(Dim_i);
        ALGORITHM_PARAMETER_DECL(Dim_j);
        ALGORITHM_PARAMETER_DECL(Dim_k);
        ALGORITHM_PARAMETER_DECL(Index_i);
        ALGORITHM_PARAMETER_DECL(Index_j);
        ALGORITHM_PARAMETER_DECL(Index_k);
        ALGORITHM_PARAMETER_DECL(Axis_ijk);
        ALGORITHM_PARAMETER_DECL(SpinBoxReexecute);
        ALGORITHM_PARAMETER_DECL(AxisReexecute);
        ALGORITHM_PARAMETER_DECL(SliderReexecute);
      }}}

  namespace Modules {
    namespace Fields {

      class SCISHARE GetSliceFromStructuredFieldByIndices : public Dataflow::Networks::Module,
        public Has2InputPorts<FieldPortTag, MatrixPortTag>,
        public Has2OutputPorts<FieldPortTag, MatrixPortTag>
      {
      public:
        GetSliceFromStructuredFieldByIndices();

        virtual void execute() override;
        virtual void setStateDefaults() override;

        INPUT_PORT(0, InputField, Field);
        INPUT_PORT(1, InputMatrix, Matrix);
        OUTPUT_PORT(0, OutputField, Field);
        OUTPUT_PORT(1, OutputMatrix, Matrix);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      };
    }
  }
}

#endif
