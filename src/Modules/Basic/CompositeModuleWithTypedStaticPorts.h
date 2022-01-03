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


#ifndef MODULES_BASIC_COMPOSITEMODULEWITHTYPEDSTATICPORTS_H
#define MODULES_BASIC_COMPOSITEMODULEWITHTYPEDSTATICPORTS_H

#include <Modules/Basic/CompositeModuleWithStaticPorts.h>
#include <Modules/Basic/share.h>

namespace SCIRun {
  namespace Modules {
    namespace Basic {

      class SCISHARE CompositeModuleWithTypedStaticPorts : public SCIRun::Dataflow::Networks::Module,
            public Has7InputPorts<FieldPortTag, FieldPortTag, FieldPortTag, MatrixPortTag, MatrixPortTag, StringPortTag, ColorMapPortTag>,
            public Has8OutputPorts<FieldPortTag, FieldPortTag, FieldPortTag, MatrixPortTag, MatrixPortTag, StringPortTag, ColorMapPortTag, GeometryPortTag>
      {
      public:
        CompositeModuleWithTypedStaticPorts();
       ~CompositeModuleWithTypedStaticPorts() override;
        void execute() override;
        void setStateDefaults() override;

        INPUT_PORT(0, Input0, Field);
        INPUT_PORT(1, Input1, Field);
        INPUT_PORT(2, Input2, Field);
        INPUT_PORT(3, Input3, Matrix);
        INPUT_PORT(4, Input4, Matrix);
        INPUT_PORT(5, Input5, String);
        INPUT_PORT(6, Input6, ColorMap);

        OUTPUT_PORT(0, Output0, Field);
        OUTPUT_PORT(1, Output1, Field);
        OUTPUT_PORT(2, Output2, Field);
        OUTPUT_PORT(3, Output3, Matrix);
        OUTPUT_PORT(4, Output4, Matrix);
        OUTPUT_PORT(5, Output5, String);
        OUTPUT_PORT(6, Output6, ColorMap);
        OUTPUT_PORT(7, Output7, GeometryObject);

        MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUI)
       private:
        std::unique_ptr<class CompositeModuleTypedImpl> impl_;
      };
 }}}

#endif
