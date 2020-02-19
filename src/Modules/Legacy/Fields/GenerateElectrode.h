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


#ifndef MODULES_LEGACY_FIELDS_GENERATEELECTRODE_H
#define MODULES_LEGACY_FIELDS_GENERATEELECTRODE_H

#include <Dataflow/Network/GeometryGeneratingModule.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun
{
  namespace Core
  {
    namespace Algorithms
    {
      namespace Fields
      {
        ALGORITHM_PARAMETER_DECL(ElectrodeLength);
        ALGORITHM_PARAMETER_DECL(ElectrodeThickness);
        ALGORITHM_PARAMETER_DECL(NumberOfControlPoints);
        ALGORITHM_PARAMETER_DECL(ElectrodeType);
        ALGORITHM_PARAMETER_DECL(ElectrodeResolution);
        class GenerateElectrodeImpl;
      }
    }
  }

  namespace Modules
  {
    namespace Fields
    {

      class SCISHARE GenerateElectrode : public SCIRun::Dataflow::Networks::GeometryGeneratingModule,
        public Has2InputPorts<FieldPortTag, MatrixPortTag>,
        public Has4OutputPorts<FieldPortTag, GeometryPortTag, FieldPortTag, MatrixPortTag>
      {
      public:
        GenerateElectrode();
        void execute() override;
        void setStateDefaults() override;

        INPUT_PORT(0, InputField, Field);
        INPUT_PORT(1, ParameterMatrixIn, DenseMatrix);
        OUTPUT_PORT(0, OutputField, Field);
        OUTPUT_PORT(1, ElectrodeWidget, GeometryObject);
        OUTPUT_PORT(2, ControlPoints, Field);
        OUTPUT_PORT(3, ParameterMatrixOut, DenseMatrix);

        MODULE_TRAITS_AND_INFO(NoAlgoOrUI);
        //TODO: enable after UI is written
        //MODULE_TRAITS_AND_INFO(ModuleHasUI);

      private:
        SharedPointer<Core::Algorithms::Fields::GenerateElectrodeImpl> impl_;
      };
    }
  }
};

#endif
