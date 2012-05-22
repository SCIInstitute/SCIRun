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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Core/Dataflow/Network/Network.h>
#include <Core/Dataflow/Network/HardCodedModuleFactory.h>
#include <Core/Dataflow/Network/ModuleInterface.h>
#include <Core/Dataflow/Network/ConnectionId.h>
#include <Core/Dataflow/Network/Tests/MockNetwork.h>
#include <Modules/Basic/ReceiveScalar.h>
#include <Modules/Basic/SendScalar.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Basic;
using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::Networks::Mocks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

#if 0
TEST(BasicNetworkTest, SendAndReceiveScalarValueUsingManualExecution)
{
  ModuleFactoryHandle mf(new HardCodedModuleFactory);
  Network firstBasicNetwork(mf);

  ModuleLookupInfo sendInfo;
  sendInfo.module_name_ = "SendScalar";
  ModuleHandle send = firstBasicNetwork.add_module(sendInfo);
  ModuleLookupInfo receiveInfo;
  receiveInfo.module_name_ = "ReceiveScalar";
  ModuleHandle receive = firstBasicNetwork.add_module(receiveInfo);

  EXPECT_EQ(2, firstBasicNetwork.nmodules());

  firstBasicNetwork.connect(send, 0, receive, 0);
  EXPECT_EQ(1, firstBasicNetwork.nconnections());

  const double dataToSend = 3.14;
  SendScalarModule* sendScalar = dynamic_cast<SendScalarModule*>(send.get());
  EXPECT_TRUE(sendScalar != 0);
  
  sendScalar->setScalar(dataToSend);
  
  //manually execute the network, in the correct order.
  send->execute();
  receive->execute();

  ReceiveScalarModule* receiveScalar = dynamic_cast<ReceiveScalarModule*>(receive.get());
  EXPECT_TRUE(receiveScalar != 0);
  EXPECT_EQ(dataToSend, receiveScalar->latestReceivedValue());
}










#endif