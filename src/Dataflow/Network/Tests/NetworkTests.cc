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


#include <gtest/gtest.h>
#include <boost/assign.hpp>
#include <boost/assign/list_of.hpp>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/Connection.h>
#include <Dataflow/Network/ModuleDescription.h>
#include <Dataflow/Network/Tests/MockModule.h>
#include <Dataflow/Network/Tests/MockPorts.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Core/Algorithms/Base/AlgorithmFwd.h>

using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Core::Algorithms;
using namespace boost::assign;
using ::testing::DefaultValue;
using ::testing::NiceMock;


class NetworkTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    DefaultValue<InputPortHandle>::Set(InputPortHandle(new NiceMock<MockInputPort>));
    DefaultValue<OutputPortHandle>::Set(OutputPortHandle(new NiceMock<MockOutputPort>));
    DefaultValue<boost::signals2::connection>::Set(boost::signals2::connection());
    DefaultValue<PortId>::Set(PortId());
    moduleFactory_.reset(new MockModuleFactory);
  }

  virtual void TearDown()
  {
    moduleFactory_.reset();
    sf_.reset();
  }

  ModuleFactoryHandle moduleFactory_;
  static ModuleStateFactoryHandle sf_;
  static AlgorithmFactoryHandle af_;
  static ReexecuteStrategyFactoryHandle reex_;
};

ModuleStateFactoryHandle NetworkTests::sf_;
AlgorithmFactoryHandle NetworkTests::af_;
ReexecuteStrategyFactoryHandle NetworkTests::reex_;

TEST_F(NetworkTests, CanAddAndRemoveModules)
{
  Network network(moduleFactory_, sf_, af_, reex_);

  EXPECT_EQ(0, network.nmodules());

  ModuleLookupInfo mli;
  mli.module_name_ = "Module1";
  ModuleHandle m = network.add_module(mli);
  EXPECT_EQ(mli.module_name_, m->name());
  EXPECT_EQ(1, network.nmodules());
  EXPECT_EQ(m, network.module(0));
  EXPECT_TRUE(network.remove_module(ModuleId(m->id())));
  EXPECT_EQ(0, network.nmodules());
  EXPECT_FALSE(network.remove_module(ModuleId("not in the network4")));
}

TEST_F(NetworkTests, CanAddAndRemoveConnections)
{
  Network network(moduleFactory_, sf_, af_, reex_);

  ModuleLookupInfo mli1;
  mli1.module_name_ = "Module1";
  ModuleHandle m1 = network.add_module(mli1);
  ModuleLookupInfo mli2;
  mli2.module_name_ = "Module2";
  ModuleHandle m2 = network.add_module(mli2);

  EXPECT_EQ(2, network.nmodules());
  EXPECT_EQ(m1, network.module(0));
  EXPECT_EQ(m2, network.module(1));

  /// @todo: yucky, yucky test code now. needs refactoring for readability!
  ConnectionId connId = network.connect(ConnectionOutputPort(m1, 0), ConnectionInputPort(m2, 1));
  EXPECT_EQ(1, network.nconnections());
  EXPECT_EQ("module:1_p#o1:0#_@to@_module:2_p#i2:0#", connId.id_);

  EXPECT_TRUE(network.disconnect(connId));
  EXPECT_EQ(0, network.nconnections());
}

TEST_F(NetworkTests, CannotMakeSameConnectionTwice)
{
  Network network(moduleFactory_, sf_, af_, reex_);

  ModuleLookupInfo mli1;
  mli1.module_name_ = "Module1";
  ModuleHandle m1 = network.add_module(mli1);
  ModuleLookupInfo mli2;
  mli2.module_name_ = "Module2";
  ModuleHandle m2 = network.add_module(mli2);

  ConnectionId connId = network.connect(ConnectionOutputPort(m1, 0), ConnectionInputPort(m2, 1));
  EXPECT_EQ(1, network.nconnections());
  EXPECT_EQ("module:1_p#o1:0#_@to@_module:2_p#i2:0#", connId.id_);

  ConnectionId connIdEmpty = network.connect(ConnectionOutputPort(m1, 0), ConnectionInputPort(m2, 1));
  EXPECT_EQ(1, network.nconnections());
  //not sure what to return here.
  EXPECT_EQ("", connIdEmpty.id_);

  EXPECT_TRUE(network.disconnect(connId));
  EXPECT_EQ(0, network.nconnections());

  connId = network.connect(ConnectionOutputPort(m1, 0), ConnectionInputPort(m2, 1));
  EXPECT_EQ(1, network.nconnections());
  EXPECT_EQ("module:1_p#o1:0#_@to@_module:2_p#i2:0#", connId.id_);
}

/// @todo: this verification pushed up to higher layer.
TEST_F(NetworkTests, DISABLED_ConnectionsMustHaveMatchingPortTypes)
{
  Network network(moduleFactory_, sf_, af_, reex_);

  ModuleLookupInfo mli1;
  mli1.module_name_ = "Module1";
  ModuleHandle m1 = network.add_module(mli1);
  ModuleLookupInfo mli2;
  mli2.module_name_ = "Module2";
  ModuleHandle m2 = network.add_module(mli2);

  EXPECT_THROW(network.connect(ConnectionOutputPort(m1, 0), ConnectionInputPort(m2, 2)), SCIRun::Core::InvalidArgumentException);
}

TEST_F(NetworkTests, CannotConnectNonExistentPorts)
{
  Network network(moduleFactory_, sf_, af_, reex_);

  ModuleLookupInfo mli1;
  mli1.module_name_ = "Module1";
  ModuleHandle m1 = network.add_module(mli1);
  ModuleLookupInfo mli2;
  mli2.module_name_ = "Module2";
  ModuleHandle m2 = network.add_module(mli2);

  EXPECT_THROW(network.connect(ConnectionOutputPort(m1, 3), ConnectionInputPort(m2, 2)), std::out_of_range);
}
