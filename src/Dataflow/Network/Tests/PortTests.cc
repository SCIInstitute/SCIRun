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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>

using namespace SCIRun::Core;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::DefaultValue;

class PortTests : public ::testing::Test
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

TEST_F(PortTests, CtorThrowsWithEmptyArguments)
{
  ASSERT_THROW(InputPort(0,                  Port::ConstructionParams("Matrix",   "ForwardMatrix"), DatatypeSinkInterfaceHandle()),  NullPointerException);
  ASSERT_THROW(InputPort(inputModule.get(),  Port::ConstructionParams("",         "ForwardMatrix"), DatatypeSinkInterfaceHandle()),  InvalidArgumentException);
  ASSERT_THROW(InputPort(inputModule.get(),  Port::ConstructionParams("Matrix",   ""            ), DatatypeSinkInterfaceHandle()),  InvalidArgumentException);
  ASSERT_THROW(InputPort(inputModule.get(),  Port::ConstructionParams("Matrix",   "ForwardMatrix"), DatatypeSinkInterfaceHandle()),            InvalidArgumentException);
}

TEST_F(PortTests, AggregatesConnections)
{
  Port::ConstructionParams pcp("ForwardMatrix", "Matrix");
  InputPortHandle inputPort(new InputPort(inputModule.get(), pcp, DatatypeSinkInterfaceHandle()));
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, DatatypeSourceInterfaceHandle()));
  EXPECT_CALL(*inputModule, get_input_port(2)).WillOnce(Return(inputPort));
  EXPECT_CALL(*outputModule, get_output_port(1)).WillOnce(Return(outputPort));

  ASSERT_EQ(0, inputPort->nconnections());
  ASSERT_EQ(0, outputPort->nconnections());
  {
    Connection c(outputModule, 1, inputModule, 2, "test");
    //connection added on construction
    ASSERT_EQ(1, inputPort->nconnections());
    ASSERT_EQ(1, outputPort->nconnections());
    ASSERT_EQ(&c, inputPort->connection(0));
    ASSERT_EQ(&c, outputPort->connection(0));
  }
  //and removed on destruction
  ASSERT_EQ(0, inputPort->nconnections());
  ASSERT_EQ(0, outputPort->nconnections());
}

TEST_F(PortTests, InputPortTakesAtMostOneConnection)
{
  Port::ConstructionParams pcp("ForwardMatrix", "Matrix");
  InputPortHandle inputPort(new InputPort(inputModule.get(), pcp, DatatypeSinkInterfaceHandle()));
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp, DatatypeSourceInterfaceHandle()));
  EXPECT_CALL(*inputModule, get_input_port(2)).WillRepeatedly(Return(inputPort));
  EXPECT_CALL(*outputModule, get_output_port(1)).WillRepeatedly(Return(outputPort));

  ASSERT_EQ(0, inputPort->nconnections());
  ASSERT_EQ(0, outputPort->nconnections());
  Connection c(outputModule, 1, inputModule, 2, "test");
  ASSERT_EQ(1, inputPort->nconnections());
  ASSERT_EQ(1, outputPort->nconnections());

  //shouldn't be able to connect a second output to the same input.
  EXPECT_THROW(Connection c(outputModule, 1, inputModule, 2, "test"), InvalidArgumentException);

  OutputPortHandle outputPort2(new OutputPort(outputModule.get(), pcp, DatatypeSourceInterfaceHandle()));
  EXPECT_CALL(*outputModule, get_output_port(2)).WillRepeatedly(Return(outputPort2));
  EXPECT_THROW(Connection c(outputModule, 2, inputModule, 2, "test"), InvalidArgumentException);
}

//TODO: this verification pushed up to higher layer.
TEST_F(PortTests, DISABLED_CannotConnectPortsWithDifferentDatatypes)
{
  Port::ConstructionParams pcp1("ForwardMatrix", "Matrix");
  Port::ConstructionParams pcp2("VectorField", "Field");
  InputPortHandle inputPort(new InputPort(inputModule.get(), pcp1, DatatypeSinkInterfaceHandle()));
  OutputPortHandle outputPort(new OutputPort(outputModule.get(), pcp2, DatatypeSourceInterfaceHandle()));
  EXPECT_CALL(*inputModule, get_input_port(2)).WillOnce(Return(inputPort));
  EXPECT_CALL(*outputModule, get_output_port(1)).WillOnce(Return(outputPort));

  ASSERT_EQ(0, inputPort->nconnections());
  ASSERT_EQ(0, outputPort->nconnections());
  {
    EXPECT_THROW(Connection c(outputModule, 1, inputModule, 2, "test"), InvalidArgumentException);
    //connection constructor should throw for type mismatch
    ASSERT_EQ(0, inputPort->nconnections());
    ASSERT_EQ(0, outputPort->nconnections());
  }
}

