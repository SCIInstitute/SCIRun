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


#if 0

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Dataflow/Network/Network.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Network/Tests/MockNetwork.h>
#include <Modules/Basic/ReceiveScalar.h>
#include <Modules/Basic/SendScalar.h>
#include <Modules/Factory/HardCodedModuleFactory.h>
#include <Core/Algorithms/Factory/HardCodedAlgorithmFactory.h>
#include <Dataflow/Network/Tests/MockModuleState.h>
#include <Dataflow/State/SimpleMapModuleState.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Modules::Factory;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using namespace SCIRun::Dataflow::State;
using namespace SCIRun::Core::Algorithms;

using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;


TEST(BasicNetworkTest, SendAndReceiveScalarValueUsingManualExecution)
{
  DefaultValue<boost::any>::Set(boost::any());

  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  ModuleStateFactoryHandle sf(new SimpleMapModuleStateFactory);
  AlgorithmFactoryHandle af(new HardCodedAlgorithmFactory);
  Network firstBasicNetwork(mf, sf, af, ReexecuteStrategyFactoryHandle());

  ModuleLookupInfo sendInfo;
  sendInfo.module_name_ = "SendScalar";
  ModuleHandle send = firstBasicNetwork.add_module(sendInfo);
  ModuleLookupInfo receiveInfo;
  receiveInfo.module_name_ = "ReceiveScalar";
  ModuleHandle receive = firstBasicNetwork.add_module(receiveInfo);

  EXPECT_EQ(2, firstBasicNetwork.nmodules());

  firstBasicNetwork.connect(ConnectionOutputPort(send, 0), ConnectionInputPort(receive, 0));
  EXPECT_EQ(1, firstBasicNetwork.nconnections());

  const double dataToSend = 3.14;
  SendScalarModule* sendScalar = dynamic_cast<SendScalarModule*>(send.get());
  EXPECT_TRUE(sendScalar != 0);

  sendScalar->get_state()->setValue(SendScalarModule::ValueToSend(), dataToSend);

  //manually execute the network, in the correct order.
  send->execute();
  receive->execute();

  ReceiveScalarModule* receiveScalar = dynamic_cast<ReceiveScalarModule*>(receive.get());
  EXPECT_TRUE(receiveScalar != 0);
  EXPECT_EQ(dataToSend, receiveScalar->latestReceivedValue());
}
#endif
