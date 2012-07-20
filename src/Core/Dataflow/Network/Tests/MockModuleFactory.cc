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

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <Core/Dataflow/Network/Tests/MockModule.h>
#include <Core/Dataflow/Network/Tests/MockPorts.h>
#include <Core/Dataflow/Network/ModuleDescription.h>
#include <Core/Dataflow/Network/ModuleStateInterface.h>

using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::Networks::Mocks;
using namespace boost::assign;
using ::testing::Return;
using ::testing::NiceMock;

ModuleDescription MockModuleFactory::lookupDescription(const ModuleLookupInfo& info)
{
  ModuleDescription d;
  d.lookupInfo_ = info;
  d.output_ports_ += OutputPortDescription("o1", "d1", "c1");
  d.input_ports_ += InputPortDescription("i1", "d1", "c1"), InputPortDescription("i2", "d1", "c1");
  return d;
}

ModuleHandle MockModuleFactory::create(const ModuleDescription& info)
{
  MockModulePtr module(new NiceMock<MockModule>);

  EXPECT_CALL(*module, get_module_name()).WillRepeatedly(Return(info.lookupInfo_.module_name_));

  EXPECT_CALL(*module, num_input_ports()).WillRepeatedly(Return(info.input_ports_.size()));
  size_t portIndex = 0;
  BOOST_FOREACH(const InputPortDescription& d, info.input_ports_)
  {
    MockInputPortPtr inputPort(new NiceMock<MockInputPort>);
    EXPECT_CALL(*module, get_input_port(portIndex)).WillRepeatedly(Return(inputPort));
    portIndex++;
  }
  
  EXPECT_CALL(*module, num_output_ports()).WillRepeatedly(Return(info.output_ports_.size()));
  portIndex = 0;
  BOOST_FOREACH(const OutputPortDescription& d, info.output_ports_)
  {
    MockOutputPortPtr outputPort(new NiceMock<MockOutputPort>);
    EXPECT_CALL(*module, get_output_port(portIndex)).WillRepeatedly(Return(outputPort));
    portIndex++;
  }
  
  EXPECT_CALL(*module, get_id()).WillRepeatedly(Return("module" + boost::lexical_cast<std::string>(++moduleCounter_)));

  if (stateFactory_)
  {
    ModuleStateHandle state(stateFactory_->make_state(info.lookupInfo_.module_name_));
    stateMap_[module] = state;
    EXPECT_CALL(*module, get_state()).WillRepeatedly(Return(state));
  }

  return module;
}

void MockModuleFactory::setStateFactory(ModuleStateFactoryHandle stateFactory)
{
  stateFactory_ = stateFactory;
}

void MockModuleFactory::setRenderer(SCIRun::Domain::Networks::RendererInterface* renderer)
{
}
