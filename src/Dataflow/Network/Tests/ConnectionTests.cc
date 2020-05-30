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

  MockInputPortPtr dummyInputPort;
  MockOutputPortPtr dummyOutputPort;
  MockModulePtr inputModule;
  MockModulePtr outputModule;
};

TEST_F(ConnectionTests, CtorThrowsWithNullPorts)
{
  ASSERT_THROW(Connection(OutputPortHandle(), InputPortHandle(), "fake", false), NullPointerException);
  ASSERT_THROW(Connection(dummyOutputPort, InputPortHandle(), "fake", false), NullPointerException);
  ASSERT_THROW(Connection(OutputPortHandle(), dummyInputPort, "fake", false), NullPointerException);
}

TEST_F(ConnectionTests, CtorSetsPortsViaModules)
{
  Connection c(dummyOutputPort, dummyInputPort, "test", false);
  ASSERT_TRUE(c.iport_ != nullptr);
  ASSERT_TRUE(c.oport_ != nullptr);
  ASSERT_EQ(c.oport_, dummyOutputPort);
  ASSERT_EQ(c.iport_, dummyInputPort);
  ASSERT_EQ("test", c.id_.id_);
}

TEST_F(ConnectionTests, CtorConnectsSelfToPorts)
{
  EXPECT_CALL(*dummyInputPort, attach(_));
  EXPECT_CALL(*dummyOutputPort, attach(_));
  Connection c(dummyOutputPort, dummyInputPort, "test", false);
}

TEST_F(ConnectionTests, DtorDisconnectsSelfFromPorts)
{
  Connection c(dummyOutputPort, dummyInputPort, "test", false);
  EXPECT_CALL(*dummyInputPort, detach(&c));
  EXPECT_CALL(*dummyOutputPort, detach(&c));
}

TEST(ConnectionIdTests, CanParseConnectionIdString)
{
  ConnectionDescription desc(
    OutgoingConnectionDescription(ModuleId("mod:1"), PortId(0, "name1")),
    IncomingConnectionDescription(ModuleId("mod:2"), PortId(0, "name2")));
  ConnectionId id = ConnectionId::create(desc);
  std::cout << id.id_ << std::endl;
  EXPECT_EQ("mod:1_p#name1:0#_@to@_mod:2_p#name2:0#", id.id_);
  ConnectionDescription descParsed = id.describe();
  EXPECT_EQ(desc, descParsed);
}
