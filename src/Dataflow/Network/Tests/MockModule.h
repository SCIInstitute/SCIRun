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


#ifndef MOCK_MODULE_H
#define MOCK_MODULE_H

#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Dataflow/Network/ModuleFactory.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <boost/any.hpp>
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
          MOCK_METHOD0(executeWithSignals, bool());
          MOCK_METHOD0(get_state, ModuleStateHandle());
          MOCK_CONST_METHOD0(cstate, const ModuleStateHandle());
          MOCK_METHOD1(setState, void(ModuleStateHandle));
          MOCK_METHOD2(send_output_handle, void(const PortId&, SCIRun::Core::Datatypes::DatatypeHandle));
          MOCK_METHOD1(get_input_handle, SCIRun::Core::Datatypes::DatatypeHandleOption(const PortId&));
          MOCK_METHOD1(get_dynamic_input_handles, std::vector<SCIRun::Core::Datatypes::DatatypeHandleOption>(const PortId&));
          MOCK_CONST_METHOD0(name, std::string());
          MOCK_CONST_METHOD1(getOutputPort, OutputPortHandle(const PortId&));
          MOCK_CONST_METHOD1(hasOutputPort, bool(const PortId&));
          MOCK_CONST_METHOD1(findOutputPortsWithName, std::vector<OutputPortHandle>(const std::string&));
          MOCK_CONST_METHOD0(outputPorts, std::vector<OutputPortHandle>());
          MOCK_CONST_METHOD1(hasInputPort, bool(const PortId&));
          MOCK_METHOD1(getInputPort, InputPortHandle(const PortId&));
          MOCK_CONST_METHOD1(findInputPortsWithName, std::vector<InputPortHandle>(const std::string&));
          MOCK_CONST_METHOD0(inputPorts, std::vector<InputPortHandle>());
          MOCK_CONST_METHOD0(numInputPorts, size_t());
          MOCK_CONST_METHOD0(numOutputPorts, size_t());
          MOCK_CONST_METHOD0(id, ModuleId());
          MOCK_CONST_METHOD0(hasUI, bool());
          MOCK_CONST_METHOD0(isDeprecated, bool());
          MOCK_CONST_METHOD0(hasDynamicPorts, bool());
          MOCK_CONST_METHOD0(metadata, const MetadataMap&());
          MOCK_CONST_METHOD0(helpPageUrl, std::string());
          MOCK_CONST_METHOD0(newHelpPageUrl, std::string());
          MOCK_CONST_METHOD0(replacementModuleName, std::string());
          MOCK_METHOD1(setUiVisible, void(bool));
          MOCK_METHOD1(setId, void(const std::string&));
          MOCK_CONST_METHOD0(info, const ModuleLookupInfo&());
          MOCK_METHOD1(setLogger, void(SCIRun::Core::Logging::LoggerHandle));
          MOCK_CONST_METHOD0(getLogger, SCIRun::Core::Logging::LoggerHandle());
          MOCK_CONST_METHOD0(getUpdaterFunc, SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc());
          MOCK_METHOD1(setUpdaterFunc, void(SCIRun::Core::Algorithms::AlgorithmStatusReporter::UpdaterFunc));
          MOCK_METHOD1(setUiToggleFunc, void(UiToggleFunc));
          MOCK_METHOD1(connectExecuteBegins, boost::signals2::connection(const ExecuteBeginsSignalType::slot_type&));
          MOCK_METHOD1(connectExecuteEnds, boost::signals2::connection(const ExecuteEndsSignalType::slot_type&));
          MOCK_METHOD1(connectErrorListener, boost::signals2::connection(const ErrorSignalType::slot_type&));
          MOCK_CONST_METHOD0(needToExecute, bool());
          MOCK_CONST_METHOD0(isStoppable, bool());
          MOCK_METHOD0(setStateDefaults, void());
          MOCK_CONST_METHOD0(getAlgorithm, SCIRun::Core::Algorithms::AlgorithmHandle());
          MOCK_METHOD0(executionState, SCIRun::Dataflow::Networks::ModuleExecutionState&());
          MOCK_METHOD1(addPortConnection, void(const boost::signals2::connection&));
          MOCK_CONST_METHOD0(getReexecutionStrategy, ModuleReexecutionStrategyHandle());
          MOCK_METHOD1(setReexecutionStrategy, void(ModuleReexecutionStrategyHandle));
          MOCK_METHOD1(enqueueExecuteAgain, void(bool));
          MOCK_METHOD1(connectExecuteSelfRequest, boost::signals2::connection(const ExecutionSelfRequestSignalType::slot_type&));
          MOCK_METHOD1(setExecutionDisabled, void(bool));
          MOCK_CONST_METHOD0(executionDisabled, bool(void));
          MOCK_CONST_METHOD0(legacyPackageName, std::string());
          MOCK_CONST_METHOD0(legacyModuleName, std::string());
          MOCK_CONST_METHOD0(isImplementationDisabled, bool());
          MOCK_METHOD1(setProgrammableInputPortEnabled, void(bool));
          MOCK_CONST_METHOD1(checkForVirtualConnection, bool(const ModuleInterface&));
          MOCK_CONST_METHOD0(description, std::string());
        };

        typedef boost::shared_ptr<MockModule> MockModulePtr;

        class MockModuleFactory : public ModuleFactory
        {
        public:
          MockModuleFactory() : moduleCounter_(0) {}
          ModuleDescription lookupDescription(const ModuleLookupInfo& info) const override;
          ModuleHandle create(const ModuleDescription& info) const override;
          void setStateFactory(ModuleStateFactoryHandle stateFactory) override;
          void setAlgorithmFactory(Core::Algorithms::AlgorithmFactoryHandle algoFactory) override;
          void setReexecutionFactory(ReexecuteStrategyFactoryHandle reexFactory) override;
          const ModuleDescriptionMap& getAllAvailableModuleDescriptions() const override;
          const DirectModuleDescriptionLookupMap& getDirectModuleDescriptionLookupMap() const override { throw "not implemented"; }
          bool moduleImplementationExists(const std::string& name) const override { throw "not implemented"; }
        private:
          mutable size_t moduleCounter_;
          ModuleStateFactoryHandle stateFactory_;
        };
      }
    }
  }
}

namespace boost {
inline std::ostream& operator<<(std::ostream& o, const boost::any& a)
{
  return o << "boost::any";
}
}

#include <boost/optional/optional_io.hpp>

#ifdef WIN32
#define ENABLE_ON_WINDOWS(test) test
#else
#define ENABLE_ON_WINDOWS(test) DISABLED_##test
#endif

#endif
