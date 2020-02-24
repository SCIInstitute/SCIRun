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


#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <Dataflow/Network/Tests/MockModule.h>
#include <Dataflow/Network/Tests/MockPorts.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/ModuleStateInterface.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core::Algorithms;
using namespace boost::assign;
using ::testing::Return;
using ::testing::NiceMock;

ModuleDescription MockModuleFactory::lookupDescription(const ModuleLookupInfo& info) const
{
  ModuleDescription d;
  d.lookupInfo_ = info;
  d.output_ports_ += OutputPortDescription(PortId(0, "o1"), "d1", false);
  d.input_ports_ += InputPortDescription(PortId(0, "i1"), "d1", false), InputPortDescription(PortId(0, "i2"), "d1", false), InputPortDescription(PortId(0, "i3"), "d2", false);
  return d;
}

ModuleHandle MockModuleFactory::create(const ModuleDescription& info) const
{
  MockModulePtr module(new NiceMock<MockModule>);

  ON_CALL(*module, name()).WillByDefault(Return(info.lookupInfo_.module_name_));

  ON_CALL(*module, numInputPorts()).WillByDefault(Return(info.input_ports_.size()));
  size_t portIndex = 0;
  std::vector<InputPortHandle> inputs;
  for (const InputPortDescription& d : info.input_ports_)
  {
    MockInputPortPtr inputPort(new NiceMock<MockInputPort>);
    ON_CALL(*inputPort, get_typename()).WillByDefault(Return(PortColorLookup::toColor(d.datatype)));
    ON_CALL(*inputPort, id()).WillByDefault(Return(PortId(d.id)));
    ON_CALL(*module, hasInputPort(d.id)).WillByDefault(Return(true));
    inputs.push_back(inputPort);
    //this line is troublesome with the threaded gmock verifiers on Mac. I can disable it and we can test the functionality in ConnectionTests.
    //EXPECT_CALL(*module, get_input_port(portIndex)).WillRepeatedly(Return(inputPort));
    portIndex++;
  }
  ON_CALL(*module, inputPorts()).WillByDefault(Return(inputs));

  ON_CALL(*module, numOutputPorts()).WillByDefault(Return(info.output_ports_.size()));
  portIndex = 0;
  std::vector<OutputPortHandle> outputs;
  for (const OutputPortDescription& d : info.output_ports_)
  {
    MockOutputPortPtr outputPort(new NiceMock<MockOutputPort>);
    ON_CALL(*outputPort, get_typename()).WillByDefault(Return(PortColorLookup::toColor(d.datatype)));
    ON_CALL(*outputPort, id()).WillByDefault(Return(PortId(d.id)));
    ON_CALL(*module, hasOutputPort(d.id)).WillByDefault(Return(true));
    outputs.push_back(outputPort);
    //this line is troublesome with the threaded gmock verifiers on Mac. I can disable it and we can test the functionality in ConnectionTests.
    //EXPECT_CALL(*module, get_output_port(portIndex)).WillRepeatedly(Return(outputPort));
    portIndex++;
  }
  ON_CALL(*module, outputPorts()).WillByDefault(Return(outputs));

  ModuleId id("module", ++moduleCounter_);
  ON_CALL(*module, id()).WillByDefault(Return(id));

  //not used now, commenting out.
  //if (stateFactory_)
  //{
  //  ModuleStateHandle state(stateFactory_->make_state(info.lookupInfo_.module_name_));
  //  stateMap_[module] = state;
  //  EXPECT_CALL(*module, get_state()).WillRepeatedly(Return(state));
  //}

  return module;
}

void MockModuleFactory::setStateFactory(ModuleStateFactoryHandle stateFactory)
{
  stateFactory_ = stateFactory;
}

void MockModuleFactory::setAlgorithmFactory(AlgorithmFactoryHandle algoFactory)
{
}

void MockModuleFactory::setReexecutionFactory(ReexecuteStrategyFactoryHandle reexFactory)
{
}

const ModuleDescriptionMap& MockModuleFactory::getAllAvailableModuleDescriptions() const
{
  static ModuleDescriptionMap empty;
  return empty;
}
