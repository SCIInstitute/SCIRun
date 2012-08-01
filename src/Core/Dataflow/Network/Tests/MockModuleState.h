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

#ifndef MOCK_MODULE_STATE_H
#define MOCK_MODULE_STATE_H

#include <Core/Dataflow/Network/ModuleStateInterface.h>
#include <gmock/gmock.h>

namespace SCIRun {
  namespace Domain {
    namespace Networks {
      namespace Mocks
      {
        class MockModuleState : public ModuleStateInterface
        {
        public:
          MOCK_METHOD2(setValue, void(const Name&, const SCIRun::Algorithms::AlgorithmParameter::Value&));
          MOCK_CONST_METHOD1(getValue, const Value(const Name&));
          MOCK_CONST_METHOD0(getKeys, Keys());
          MOCK_CONST_METHOD1(getTransientValue, const TransientValue(const std::string&));
          MOCK_METHOD2(setTransientValue, void(const std::string&, const TransientValue&));
          MOCK_METHOD1(connect_state_changed, boost::signals::connection(state_changed_sig_t::slot_function_type));
        };

        typedef boost::shared_ptr<MockModuleState> MockModuleStatePtr;

        class MockModuleStateFactory : public ModuleStateInterfaceFactory
        {
        public:
          virtual ModuleStateInterface* make_state(const std::string& name) const
          {
            return new ::testing::NiceMock<MockModuleState>;
          }
        };
      }
    }
  }
}

#endif
