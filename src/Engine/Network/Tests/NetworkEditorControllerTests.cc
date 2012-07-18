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
#include <Engine/Network/NetworkEditorController.h>
#include <Core/Dataflow/Network/ModuleInterface.h>
#include <Core/Dataflow/Network/ConnectionId.h>
#include <Core/Dataflow/Network/Tests/MockNetwork.h>

using namespace SCIRun;
using namespace SCIRun::Engine;
using namespace SCIRun::Domain::Networks;
using namespace SCIRun::Domain::Networks::Mocks;
using ::testing::_;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class SlotClassForNetworkEditorController
{
public:
  virtual ~SlotClassForNetworkEditorController() {}
  virtual void moduleAddedSlot(const std::string&, ModuleHandle) = 0;
  virtual void moduleRemovedSlot(const std::string& id) = 0;
  virtual void connectionAddedSlot(const ConnectionId&) = 0;
};

class DummySlotClassForNetworkEditorController : public SlotClassForNetworkEditorController
{
public:
  MOCK_METHOD2(moduleAddedSlot, void(const std::string&, ModuleHandle));
  MOCK_METHOD1(moduleRemovedSlot, void(const std::string&));
  MOCK_METHOD1(connectionAddedSlot, void(const ConnectionId&));
};

class NetworkEditorControllerTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    DefaultValue<ModuleHandle>::Set(ModuleHandle());
    DefaultValue<ConnectionId>::Set(ConnectionId(""));
    mockNetwork_.reset(new NiceMock<MockNetwork>);
  }
  MockNetworkPtr mockNetwork_;
  DummySlotClassForNetworkEditorController slots_;
};

TEST_F(NetworkEditorControllerTests, CanAddAndRemoveModulesWithSignalling)
{
  NetworkEditorController controller(mockNetwork_);
  
  controller.connectModuleAdded(boost::bind(&SlotClassForNetworkEditorController::moduleAddedSlot, &slots_, _1, _2));
  controller.connectModuleRemoved(boost::bind(&SlotClassForNetworkEditorController::moduleRemovedSlot, &slots_, _1));
  
  EXPECT_CALL(slots_, moduleAddedSlot(_,_)).Times(1);
  EXPECT_CALL(*mockNetwork_, add_module(_)).Times(1);
  controller.addModule("m1");
  
  EXPECT_CALL(slots_, moduleRemovedSlot(_)).Times(1);
  EXPECT_CALL(*mockNetwork_, remove_module("m1")).Times(1);
  controller.removeModule("m1");
}

TEST_F(NetworkEditorControllerTests, CanAddAndRemoveConnectionWithSignalling)
{
  NetworkEditorController controller(mockNetwork_);

  controller.connectConnectionAdded(boost::bind(&SlotClassForNetworkEditorController::connectionAddedSlot, &slots_, _1));

  EXPECT_CALL(slots_, connectionAddedSlot(_)).Times(1);
  EXPECT_CALL(*mockNetwork_, connect(_,_,_,_)).Times(1).WillOnce(Return(ConnectionId("non empty string")));
  controller.addConnection(ConnectionDescription("m1", 1, "m2", 2));


  //TODO
  //EXPECT_TRUE(false);
}