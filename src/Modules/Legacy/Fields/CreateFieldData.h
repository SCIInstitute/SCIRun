/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef MODULES_LEGACY_FIELDS_CREATEFIELDDATA_H__
#define MODULES_LEGACY_FIELDS_CREATEFIELDDATA_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Fields {

      /// @class CreateFieldData
      /// @brief This module assigns a value to each element or node of the mesh
      /// based on a given function, that is based on the location of nodes and
      /// elements and properties of the elements.

      class SCISHARE CreateFieldData : public Dataflow::Networks::Module,
        public Has3InputPorts<FieldPortTag, StringPortTag, DynamicPortTag<MatrixPortTag>>,
        public Has1OutputPort<FieldPortTag>
      {
      public:
        CreateFieldData();

        virtual void execute() override;
        virtual void setStateDefaults() override;
        virtual bool hasDynamicPorts() const override { return true; }

        INPUT_PORT(0, InputField, LegacyField);
        INPUT_PORT(1, Function, String);
        INPUT_PORT_DYNAMIC(2, DataArray, Matrix);
        OUTPUT_PORT(0, OutputField, LegacyField);

        MODULE_TRAITS_AND_INFO(ModuleHasUI)

        static const Core::Algorithms::AlgorithmParameterName FunctionString;
        static const  Core::Algorithms::AlgorithmParameterName FormatString;
        static const Core::Algorithms::AlgorithmParameterName BasisString;
        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
      };

    }
  }
}

#endif
