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


#ifndef MODULES_LEGACY_FIELDS_CREATELATVOL_H__
#define MODULES_LEGACY_FIELDS_CREATELATVOL_H__

///@author
///   Michael Callahan
///   Department of Computer Science
///   University of Utah
///@date  March 2001
///

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Fields {

	///@class CreateLatVol
	///@brief Make an LatVolField that fits the source field.

      class SCISHARE CreateLatVol : public Dataflow::Networks::Module,
        public Has2InputPorts<FieldPortTag, MatrixPortTag>,
        public Has1OutputPort<FieldPortTag>
      {
      public:
        CreateLatVol();

        virtual void execute() override;
        virtual void setStateDefaults() override;

        INPUT_PORT(0, InputField, Field);
        INPUT_PORT(1, LatVolSize, DenseMatrix);
        OUTPUT_PORT(0, OutputField, Field);

        static const Core::Algorithms::AlgorithmParameterName XSize;
        static const Core::Algorithms::AlgorithmParameterName YSize;
        static const Core::Algorithms::AlgorithmParameterName ZSize;
        static const Core::Algorithms::AlgorithmParameterName PadPercent;
        static const Core::Algorithms::AlgorithmParameterName DataAtLocation;
        static const Core::Algorithms::AlgorithmParameterName ElementSizeNormalized;

        enum DataLocation { NODES, CELLS, NONE };
        enum MeshDimensions { ELEMENTS_NORMALIZED, ELEMENT_SIZE_ONE };

        MODULE_TRAITS_AND_INFO(ModuleHasUI)

      private:
        enum DataTypeEnum { SCALAR, VECTOR, TENSOR };
      };
    }
  }
}

#endif
