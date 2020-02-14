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


#ifndef MODULES_LEGACY_FINITEELEMENTS_APPLYFEMCURRENTSOURCE_H__
#define MODULES_LEGACY_FINITEELEMENTS_APPLYFEMCURRENTSOURCE_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/FiniteElements/share.h>

namespace SCIRun {
  namespace Modules {
    namespace FiniteElements {

      class SCISHARE ApplyFEMCurrentSource : public Dataflow::Networks::Module,
        public Has4InputPorts<FieldPortTag, FieldPortTag, MatrixPortTag, MatrixPortTag>,
        public Has2OutputPorts<MatrixPortTag, MatrixPortTag>
      {
        LEGACY_BIOPSE_MODULE
      public:
        ApplyFEMCurrentSource();
        void setStateDefaults() override;
        void execute() override;

        INPUT_PORT(0, Mesh, Field);
        INPUT_PORT(1, Sources, Field);
        INPUT_PORT(2, Mapping, Matrix);
        INPUT_PORT(3, Input_RHS, Matrix);
        OUTPUT_PORT(0, Output_RHS, Matrix);
        OUTPUT_PORT(1, Output_Weights, Matrix);

        static const Core::Algorithms::AlgorithmParameterName SourceNode;
        static const Core::Algorithms::AlgorithmParameterName SinkNode;
        static const Core::Algorithms::AlgorithmParameterName ModelType;

        MODULE_TRAITS_AND_INFO(ModuleHasUI)
      };

    }
  }
}

#endif
