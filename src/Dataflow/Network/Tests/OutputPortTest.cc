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


#include <Dataflow/Network/Port.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Tests/MockModule.h>
#include <Dataflow/Network/Tests/MockPorts.h>
#include <Core/Datatypes/Scalar.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core::Datatypes;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::_;

class OutputPortTest : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    DefaultValue<InputPortHandle>::Set(InputPortHandle());
    DefaultValue<OutputPortHandle>::Set(OutputPortHandle());
    DefaultValue<DatatypeSinkInterfaceHandle>::Set(sink_);
  }
  DatatypeSinkInterfaceHandle sink_;
};

TEST_F(OutputPortTest, SendSomeData)
{
  Port::ConstructionParams pcp(PortId(0, "ScalarValue"), "Double", false);

  MockDatatypeSourcePtr mockSource(new NiceMock<MockDatatypeSource>);
  MockModulePtr outputModule(new NiceMock<MockModule>);
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, mockSource));

  MockInputPortPtr inputPort(new NiceMock<MockInputPort>);
  MockModulePtr inputModule(new NiceMock<MockModule>);
  //EXPECT_CALL(*inputPort, get_typename()).WillRepeatedly(Return(pcp.type_name));
  //EXPECT_CALL(*inputModule, getInputPort(2)).WillRepeatedly(Return(inputPort));
  //EXPECT_CALL(*outputModule, getOutputPort(1)).WillRepeatedly(Return(outputPort));

  Connection c(outputPort, inputPort, "test", false);

  const int dataValue = 2;
  DatatypeHandle dataToPush(new Int32(dataValue));

  EXPECT_CALL(*mockSource, cacheData(dataToPush));
  EXPECT_CALL(*mockSource, send(_));
  outputPort->sendData(dataToPush);
}

TEST_F(OutputPortTest, DataNotSentWhenNoConnectionsOnPort)
{
  MockModulePtr outputModule(new NiceMock<MockModule>);

  Port::ConstructionParams pcp(PortId(0, "ScalarValue"), "Double", false);

  MockDatatypeSourcePtr mockSource(new NiceMock<MockDatatypeSource>);
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, mockSource));

  const int dataValue = 2;
  DatatypeHandle dataToPush(new Int32(dataValue));

  EXPECT_CALL(*mockSource, cacheData(dataToPush)).Times(1);
  EXPECT_CALL(*mockSource, send(_)).Times(0);
  outputPort->sendData(dataToPush);
}

TEST_F(OutputPortTest, CanSendDataToMultipleConnections)
{
  Port::ConstructionParams pcp(PortId(0, "ScalarValue"), "Double", false);

  MockDatatypeSourcePtr mockSource(new NiceMock<MockDatatypeSource>);
  MockModulePtr outputModule(new NiceMock<MockModule>);
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, mockSource));

  MockInputPortPtr inputPort(new NiceMock<MockInputPort>);
  MockInputPortPtr inputPort2(new NiceMock<MockInputPort>);
  MockModulePtr inputModule(new NiceMock<MockModule>);
  //EXPECT_CALL(*inputModule, get_input_port(1)).WillRepeatedly(Return(inputPort));
  //EXPECT_CALL(*inputModule, get_input_port(2)).WillRepeatedly(Return(inputPort2));
  //EXPECT_CALL(*inputPort, get_typename()).WillRepeatedly(Return(pcp.type_name));
  //EXPECT_CALL(*inputPort2, get_typename()).WillRepeatedly(Return(pcp.type_name));
  //EXPECT_CALL(*outputModule, get_output_port(1)).WillRepeatedly(Return(outputPort));

  Connection c1(outputPort, inputPort, "test1", false);
  Connection c2(outputPort, inputPort2, "test2", false);
  EXPECT_EQ(2, outputPort->nconnections());

  const int dataValue = 2;
  DatatypeHandle dataToPush(new Int32(dataValue));

  EXPECT_CALL(*mockSource, cacheData(dataToPush)).Times(1);
  EXPECT_CALL(*mockSource, send(_)).Times(2);
  outputPort->sendData(dataToPush);
}
