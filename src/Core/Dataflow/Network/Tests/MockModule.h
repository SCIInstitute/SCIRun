/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#ifndef MOCK_MODULE_H
#define MOCK_MODULE_H

#include <Core/Dataflow/Network/ModuleInterface.h>
#include <Core/Dataflow/Network/ModuleStateInterface.h>
#include <Core/Dataflow/Network/ModuleFactory.h>
#include <Core/Dataflow/Network/ModuleDescription.h>
#include <gmock/gmock.h>

namespace SCIRun {
  namespace Domain {
    namespace Networks {
      namespace Mocks
      {
        class MockModule : public ModuleInterface
        {
        public:
          MOCK_METHOD0(execute, void());
          MOCK_METHOD0(get_state, ModuleStateHandle());
          MOCK_METHOD1(set_state, void(ModuleStateHandle));
          MOCK_METHOD2(send_output_handle, void(size_t, SCIRun::Domain::Datatypes::DatatypeHandle));
          MOCK_METHOD1(get_input_handle, SCIRun::Domain::Datatypes::DatatypeHandleOption(size_t));
          MOCK_CONST_METHOD0(get_module_name, std::string());
          MOCK_CONST_METHOD1(get_output_port, OutputPortHandle(size_t));
          MOCK_CONST_METHOD1(get_input_port, InputPortHandle(size_t));
          MOCK_CONST_METHOD0(num_input_ports, size_t());
          MOCK_CONST_METHOD0(num_output_ports, size_t());
          MOCK_CONST_METHOD0(get_id, std::string());
          MOCK_CONST_METHOD0(has_ui, bool());
          MOCK_METHOD1(set_id, void(const std::string&));
          MOCK_CONST_METHOD0(get_info, const ModuleLookupInfo&());
        };

        typedef boost::shared_ptr<MockModule> MockModulePtr;

        class MockModuleFactory : public ModuleFactory
        {
        public:
          MockModuleFactory() : moduleCounter_(0) {}
          virtual ModuleDescription lookupDescription(const ModuleLookupInfo& info);
          virtual ModuleHandle create(const ModuleDescription& info);
          virtual void setStateFactory(ModuleStateFactoryHandle stateFactory);
          virtual void setRenderer(SCIRun::Domain::Networks::RendererInterface* renderer);
        private:
          size_t moduleCounter_;
          ModuleStateFactoryHandle stateFactory_;
          std::map<MockModulePtr, ModuleStateHandle> stateMap_;
        };
      }
    }
  }
}

#endif
