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


#ifndef MODULES_BASIC_COMPOSITEMODULEWITHSTATICPORTS_H
#define MODULES_BASIC_COMPOSITEMODULEWITHSTATICPORTS_H

#include <Dataflow/Network/Module.h>
#include <Modules/Basic/share.h>

namespace SCIRun {
  namespace Core::Algorithms::Python
  {
    ALGORITHM_PARAMETER_DECL(NetworkXml);
    ALGORITHM_PARAMETER_DECL(PortSettings);
  }
  namespace Modules {
    namespace Basic {

      class SCISHARE CompositeModuleWithStaticPorts : public SCIRun::Dataflow::Networks::Module,
            public Has7InputPorts<DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag>,
            public Has8OutputPorts<DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag, DatatypePortTag>
      {
      public:
        CompositeModuleWithStaticPorts();
       ~CompositeModuleWithStaticPorts() override;
        void execute() override;
        void setStateDefaults() override;

        INPUT_PORT(0, Input0, Datatype);
        INPUT_PORT(1, Input1, Datatype);
        INPUT_PORT(2, Input2, Datatype);
        INPUT_PORT(3, Input3, Datatype);
        INPUT_PORT(4, Input4, Datatype);
        INPUT_PORT(5, Input5, Datatype);
        INPUT_PORT(6, Input6, Datatype);

        OUTPUT_PORT(0, Output0, Datatype);
        OUTPUT_PORT(1, Output1, Datatype);
        OUTPUT_PORT(2, Output2, Datatype);
        OUTPUT_PORT(3, Output3, Datatype);
        OUTPUT_PORT(4, Output4, Datatype);
        OUTPUT_PORT(5, Output5, Datatype);
        OUTPUT_PORT(6, Output6, Datatype);
        OUTPUT_PORT(7, Output7, Datatype);

        MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUI)
       private:
        std::unique_ptr<class CompositeModuleImpl> impl_;
      };
 }}}

#endif
