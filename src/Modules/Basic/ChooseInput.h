/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#ifndef MODULES_BASIC_CHOOSEINPUT_H
#define MODULES_BASIC_CHOOSEINPUT_H

#include <Dataflow/Network/Module.h>
#include <Core/Algorithms/Base/AlgorithmBase.h>
#include <Modules/Basic/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace FlowControl {

        ALGORITHM_PARAMETER_DECL(PortIndex);
        ALGORITHM_PARAMETER_DECL(PortMax);

      }
    }
  }

  namespace Modules {
    namespace Basic {

      class SCISHARE ChooseInput : public SCIRun::Dataflow::Networks::Module,
        public Has1InputPort<DynamicPortTag<DatatypePortTag>>,
        public Has1OutputPort<DatatypePortTag>
      {
      public:
        ChooseInput();
        virtual void execute() override;
        virtual void setStateDefaults() override;
        virtual bool hasDynamicPorts() const override { return true; }
        virtual void portAddedSlot(const Dataflow::Networks::ModuleId& mid, const Dataflow::Networks::PortId& pid) override;
        virtual void portRemovedSlot(const Dataflow::Networks::ModuleId& mid, const Dataflow::Networks::PortId& pid) override;

        INPUT_PORT_DYNAMIC(0, Input, Datatype);
        OUTPUT_PORT(0, Output, Datatype);

        static const Dataflow::Networks::ModuleLookupInfo staticInfo_;
      private:
        void portChangeImpl();
      };
 }}}

#endif
