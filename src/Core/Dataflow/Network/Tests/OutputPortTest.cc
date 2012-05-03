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

#include <Core/Dataflow/Network/Port.h>
#include <Core/Dataflow/Network/Connection.h>
#include <Core/Dataflow/Network/Tests/MockModule.h>
#include <Core/Dataflow/Network/Tests/MockPorts.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>

using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::Networks::Mocks;
using namespace SCIRun::Domain::Datatypes;
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
    
    inputModule.reset(new NiceMock<MockModule>);
    outputModule.reset(new NiceMock<MockModule>);
  }

  MockModulePtr inputModule;
  MockModulePtr outputModule;
  DatatypeSinkInterfaceHandle sink_;
};

TEST_F(OutputPortTest, SendSomeData)
{
  Port::ConstructionParams pcp("Double", "ScalarVale", "cyan");

  MockDatatypeSourcePtr mockSource(new NiceMock<MockDatatypeSource>);
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, mockSource));

  MockInputPortPtr inputPort(new NiceMock<MockInputPort>);
  EXPECT_CALL(*inputModule, get_input_port(2)).WillOnce(Return(inputPort));
  EXPECT_CALL(*outputModule, get_output_port(1)).WillOnce(Return(outputPort));

  Connection c(outputModule, 1, inputModule, 2, "test");
  
  const int dataValue = 2;
  DatatypeHandle dataToPush(new Datatype(dataValue));
  
  EXPECT_CALL(*mockSource, send(_, dataToPush));
  outputPort->sendData(dataToPush);
}

TEST_F(OutputPortTest, DataNotSentWhenNoConnectionsOnPort)
{
  Port::ConstructionParams pcp("Double", "ScalarVale", "cyan");

  MockDatatypeSourcePtr mockSource(new NiceMock<MockDatatypeSource>);
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, mockSource));

  const int dataValue = 2;
  DatatypeHandle dataToPush(new Datatype(dataValue));

  EXPECT_CALL(*mockSource, send(_, dataToPush)).Times(0);
  outputPort->sendData(dataToPush);
}

TEST_F(OutputPortTest, CanSendDataToMultipleConnections)
{
  Port::ConstructionParams pcp("Double", "ScalarVale", "cyan");

  MockDatatypeSourcePtr mockSource(new NiceMock<MockDatatypeSource>);
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, mockSource));

  MockInputPortPtr inputPort(new NiceMock<MockInputPort>);
  MockInputPortPtr inputPort2(new NiceMock<MockInputPort>);
  EXPECT_CALL(*inputModule, get_input_port(1)).WillOnce(Return(inputPort));
  EXPECT_CALL(*inputModule, get_input_port(2)).WillOnce(Return(inputPort2));
  EXPECT_CALL(*outputModule, get_output_port(1)).WillRepeatedly(Return(outputPort));

  Connection c1(outputModule, 1, inputModule, 1, "test1");
  Connection c2(outputModule, 1, inputModule, 2, "test2");
  EXPECT_EQ(2, outputPort->nconnections());

  const int dataValue = 2;
  DatatypeHandle dataToPush(new Datatype(dataValue));

  EXPECT_CALL(*mockSource, send(_, dataToPush)).Times(2);
  outputPort->sendData(dataToPush);
}