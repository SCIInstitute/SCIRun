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

#include <Dataflow/Network/Port.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/Tests/MockModule.h>
#include <Dataflow/Network/Tests/MockPorts.h>
#include <Dataflow/Network/Tests/SimpleSourceSink.h>
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

class InputPortTest : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    DefaultValue<InputPortHandle>::Set(InputPortHandle());
    DefaultValue<OutputPortHandle>::Set(OutputPortHandle());
    
    inputModule.reset(new NiceMock<MockModule>);
    outputModule.reset(new NiceMock<MockModule>);
  }

  MockModulePtr inputModule;
  MockModulePtr outputModule;
};


TEST_F(InputPortTest, GetDataReturnsEmptyWhenNoConnectionPresent)
{
  Port::ConstructionParams pcp("ForwardMatrix", "Matrix", false);

  MockDatatypeSinkPtr sink(new NiceMock<MockDatatypeSink>);

  InputPortHandle inputPort(new InputPort(inputModule.get(), pcp, sink));

  //no connection hooked up, so should exit before going to the sink.
  EXPECT_CALL(*sink, waitForData()).Times(0);
  EXPECT_CALL(*sink, receive()).Times(0);
  DatatypeHandleOption data = inputPort->getData();
  EXPECT_FALSE(data);
}

//let's just use all "real" objects to see if it works.
TEST_F(InputPortTest, GetDataWaitsAndReceivesData)
{
  Port::ConstructionParams pcp("ForwardMatrix", "Matrix", false);

  boost::shared_ptr<SimpleSink> sink(new SimpleSink);

  InputPortHandle inputPort(new InputPort(inputModule.get(), pcp, sink));

  boost::shared_ptr<SimpleSource> source(new SimpleSource);
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, source));
  EXPECT_CALL(*inputModule, get_input_port(2)).WillOnce(Return(inputPort));
  EXPECT_CALL(*outputModule, get_output_port(1)).WillOnce(Return(outputPort));

  Connection c(outputModule, 1, inputModule, 2, "test");

  const int dataValue = 2;
  DatatypeHandle dataToPush(new Int32(dataValue));
  outputPort->sendData(dataToPush);
  
  DatatypeHandleOption data = inputPort->getData();
  EXPECT_TRUE(data);
  EXPECT_EQ(dataValue, (*data)->as<Int32>()->value());
}