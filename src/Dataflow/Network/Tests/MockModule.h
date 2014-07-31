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

#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/ModuleFactory.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <gmock/gmock.h>

namespace SCIRun {
  namespace Dataflow {
    namespace Networks {
      namespace Mocks
      {
        class MockModule : public ModuleInterface
        {
        public:
          MOCK_METHOD0(execute, void());
          MOCK_METHOD0(do_execute, bool());
          MOCK_METHOD0(get_state, ModuleStateHandle());
          MOCK_METHOD1(set_state, void(ModuleStateHandle));
          MOCK_METHOD2(send_output_handle, void(const PortId&, SCIRun::Core::Datatypes::DatatypeHandle));
          MOCK_METHOD1(get_input_handle, SCIRun::Core::Datatypes::DatatypeHandleOption(const PortId&));
          MOCK_METHOD1(get_dynamic_input_handles, std::vector<SCIRun::Core::Datatypes::DatatypeHandleOption>(const PortId&));
          MOCK_CONST_METHOD0(get_module_name, std::string());
          MOCK_CONST_METHOD1(getOutputPort, OutputPortHandle(const PortId&));
          MOCK_CONST_METHOD1(hasOutputPort, bool(const PortId&));
          MOCK_CONST_METHOD1(findOutputPortsWithName, std::vector<OutputPortHandle>(const std::string&));
          MOCK_CONST_METHOD0(outputPorts, std::vector<OutputPortHandle>());
          MOCK_CONST_METHOD1(hasInputPort, bool(const PortId&));
          MOCK_CONST_METHOD1(getInputPort, InputPortHandle(const PortId&));
          MOCK_CONST_METHOD1(findInputPortsWithName, std::vector<InputPortHandle>(const std::string&));
          MOCK_CONST_METHOD0(inputPorts, std::vector<InputPortHandle>());
          MOCK_CONST_METHOD0(num_input_ports, size_t());
          MOCK_CONST_METHOD0(num_output_ports, size_t());
          MOCK_CONST_METHOD0(get_id, ModuleId());
          MOCK_CONST_METHOD0(has_ui, bool());
          MOCK_CONST_METHOD0(hasDynamicPorts, bool());
          MOCK_METHOD1(setUiVisible, void(bool));
          MOCK_METHOD1(set_id, void(const std::string&));
          MOCK_CONST_METHOD0(get_info, const ModuleLookupInfo&());
          MOCK_METHOD1(setLogger, void(SCIRun::Core::Logging::LoggerHandle));
          MOCK_CONST_METHOD0(getLogger, SCIRun::Core::Logging::LoggerHandle());
          MOCK_CONST_METHOD0(getUpdaterFunc, SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc());
          MOCK_METHOD1(setUpdaterFunc, void(SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc));
          MOCK_METHOD1(setUiToggleFunc, void(UiToggleFunc));
          MOCK_METHOD1(connectExecuteBegins, boost::signals2::connection(const ExecuteBeginsSignalType::slot_type&));
          MOCK_METHOD1(connectExecuteEnds, boost::signals2::connection(const ExecuteEndsSignalType::slot_type&));
          MOCK_METHOD1(connectErrorListener, boost::signals2::connection(const ErrorSignalType::slot_type&));
          MOCK_CONST_METHOD0(needToExecute, bool());
          MOCK_METHOD0(setStateDefaults, void());
          MOCK_CONST_METHOD0(getAlgorithm, SCIRun::Core::Algorithms::AlgorithmHandle());
          MOCK_CONST_METHOD0(executionState, SCIRun::Dataflow::Networks::ModuleInterface::ExecutionState());
          MOCK_METHOD1(setExecutionState, void(SCIRun::Dataflow::Networks::ModuleInterface::ExecutionState));
          MOCK_METHOD1(addPortConnection, void(const boost::signals2::connection&));
          MOCK_CONST_METHOD0(getReexecutionStrategy, ModuleReexecutionStrategyHandle());
          MOCK_METHOD1(setReexecutionStrategy, void(ModuleReexecutionStrategyHandle));
        };

        typedef boost::shared_ptr<MockModule> MockModulePtr;

        class MockModuleFactory : public ModuleFactory
        {
        public:
          MockModuleFactory() : moduleCounter_(0) {}
          virtual ModuleDescription lookupDescription(const ModuleLookupInfo& info);
          virtual ModuleHandle create(const ModuleDescription& info);
          virtual void setStateFactory(ModuleStateFactoryHandle stateFactory);
          virtual void setAlgorithmFactory(Core::Algorithms::AlgorithmFactoryHandle algoFactory);
          virtual const ModuleDescriptionMap& getAllAvailableModuleDescriptions() const;
        private:
          size_t moduleCounter_;
          ModuleStateFactoryHandle stateFactory_;
        };
      }
    }
  }
}

#ifdef WIN32
#define ENABLE_ON_WINDOWS(test) test
#else
#define ENABLE_ON_WINDOWS(test) DISABLED_##test
#endif

#endif
