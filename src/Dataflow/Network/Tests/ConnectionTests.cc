/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Tests/MockModule.h>
#include <Dataflow/Network/Tests/MockPorts.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::_;

class ConnectionTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    DefaultValue<InputPortHandle>::Set(InputPortHandle());
    DefaultValue<OutputPortHandle>::Set(OutputPortHandle());

    dummyInputPort.reset(new NiceMock<MockInputPort>);
    dummyOutputPort.reset(new NiceMock<MockOutputPort>);
    inputModule.reset(new NiceMock<MockModule>);
    outputModule.reset(new NiceMock<MockModule>);
  }

  void setModuleExpectations()
  {
    EXPECT_CALL(*inputModule, get_input_port(2)).WillOnce(Return(dummyInputPort));
    EXPECT_CALL(*outputModule, get_output_port(1)).WillOnce(Return(dummyOutputPort));
  }

  MockInputPortPtr dummyInputPort;
  MockOutputPortPtr dummyOutputPort;
  MockModulePtr inputModule;
  MockModulePtr outputModule;
};

TEST_F(ConnectionTests, CtorThrowsWithNullModules)
{
  ASSERT_THROW(Connection(ModuleHandle(), 1, ModuleHandle(), 2, "fake"), NullPointerException);
  ModuleHandle dummy(new MockModule);
  ASSERT_THROW(Connection(dummy, 1, ModuleHandle(), 2, "fake"), NullPointerException);
  ASSERT_THROW(Connection(ModuleHandle(), 1, dummy, 2, "fake"), NullPointerException);
}

TEST_F(ConnectionTests, CtorThrowsWhenPortsDontExistOnModules)
{
  ASSERT_THROW(Connection(outputModule, 1, inputModule, 2, "test"), NullPointerException);
}

TEST_F(ConnectionTests, CtorSetsPortsViaModules)
{
  setModuleExpectations();
  Connection c(outputModule, 1, inputModule, 2, "test");
  ASSERT_TRUE(c.imod_.get() != nullptr);
  ASSERT_TRUE(c.omod_.get() != nullptr);
  ASSERT_TRUE(c.iport_.get() != nullptr);
  ASSERT_TRUE(c.oport_.get() != nullptr);
  ASSERT_EQ(c.imod_, inputModule);
  ASSERT_EQ(c.omod_, outputModule);
  ASSERT_EQ(c.oport_, dummyOutputPort);
  ASSERT_EQ(c.iport_, dummyInputPort);
  ASSERT_EQ("test", c.id_.id_);
}

TEST_F(ConnectionTests, CtorConnectsSelfToPorts)
{
  setModuleExpectations();
  EXPECT_CALL(*dummyInputPort, attach(_));
  EXPECT_CALL(*dummyOutputPort, attach(_));
  Connection c(outputModule, 1, inputModule, 2, "test");
}

TEST_F(ConnectionTests, DtorDisconnectsSelfFromPorts)
{
  setModuleExpectations();
  Connection c(outputModule, 1, inputModule, 2, "test");
  EXPECT_CALL(*dummyInputPort, detach(&c));
  EXPECT_CALL(*dummyOutputPort, detach(&c));
}

std::ostream& operator<<(std::ostream& o, const ConnectionDescription& desc)
{
  return o << ConnectionId::create(desc).id_;
}

TEST(ConnectionIdTests, CanParseConnectionIdString)
{
  ConnectionDescription desc(
    OutgoingConnectionDescription(ModuleId("mod:1"), 2), 
    IncomingConnectionDescription(ModuleId("mod:2"), 1));
  ConnectionId id = ConnectionId::create(desc);
  std::cout << id.id_ << std::endl;
  EXPECT_EQ("mod:1_p#2_@to@_mod:2_p#1", id.id_);
  ConnectionDescription descParsed = id.describe();
  EXPECT_EQ(desc, descParsed);
}