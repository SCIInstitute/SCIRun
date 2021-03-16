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


/// @file SplitFieldByConnectedRegion.h
/// @brief
///  Splits a domain into separate fields as defined by the input field's connectivity.
///
/// @author
///  Moritz Dannhauer (ported from SCIRun4)
///
/// @details
///  This is simply the module that grabs the input field and passes it to the algorithm along with the GUI settings from the state object.
///  After execution, it also receives the 8 output fields and the bundle which contains all sub fields that could be separated.

#ifndef MODULES_LEGACY_FIELDS_SPLITFIELDBYCONNECTEDREGION_H__
#define MODULES_LEGACY_FIELDS_SPLITFIELDBYCONNECTEDREGION_H__

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Fields/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Fields {

      class SCISHARE SplitFieldByConnectedRegion : public Dataflow::Networks::Module,
        public Has1InputPort<FieldPortTag>,
        public Has8OutputPorts<FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag, FieldPortTag>
      {
      public:
        SplitFieldByConnectedRegion();

        void execute() override;
        void setStateDefaults() override;

        INPUT_PORT(0, InputField, Field);
        OUTPUT_PORT(0, OutputField1, Field);
	      OUTPUT_PORT(1, OutputField2, Field);
        OUTPUT_PORT(2, OutputField3, Field);
        OUTPUT_PORT(3, OutputField4, Field);
        OUTPUT_PORT(4, OutputField5, Field);
        OUTPUT_PORT(5, OutputField6, Field);
        OUTPUT_PORT(6, OutputField7, Field);
        OUTPUT_PORT(7, OutputField8, Field);

        MODULE_TRAITS_AND_INFO(ModuleHasUIAndAlgorithm)
      };

    }
  }
}

#endif
