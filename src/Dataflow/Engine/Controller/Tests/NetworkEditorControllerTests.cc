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
#include <gmock/gmock.h>
#include <Dataflow/Engine/Controller/NetworkEditorController.h>
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/ConnectionId.h>
#include <Dataflow/Network/NetworkSettings.h>
#include <Dataflow/Network/Tests/MockNetwork.h>
#include <Dataflow/Network/Tests/MockPorts.h>
#include <Dataflow/Engine/Scheduler/DesktopExecutionStrategyFactory.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using ::testing::_;
using ::testing::Eq;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;

class SlotClassForNetworkEditorController
{
public:
  virtual ~SlotClassForNetworkEditorController() {}
  virtual void moduleAddedSlot(const std::string&, ModuleHandle) = 0;
  virtual void moduleRemovedSlot(const std::string& id) = 0;
  virtual void connectionAddedSlot(const ConnectionDescription&) = 0;
  virtual void invalidConnectionSlot(const ConnectionDescription&) = 0;
  virtual void connectionRemovedSlot(const ConnectionId&) = 0;
};

class DummySlotClassForNetworkEditorController : public SlotClassForNetworkEditorController
{
public:
  MOCK_METHOD2(moduleAddedSlot, void(const std::string&, ModuleHandle));
  MOCK_METHOD1(moduleRemovedSlot, void(const std::string&));
  MOCK_METHOD1(connectionAddedSlot, void(const ConnectionDescription&));
  MOCK_METHOD1(invalidConnectionSlot, void(const ConnectionDescription&));
  MOCK_METHOD1(connectionRemovedSlot, void(const ConnectionId&));
};

class NetworkEditorControllerTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    DefaultValue<ModuleHandle>::Set(ModuleHandle());
    DefaultValue<ConnectionId>::Set(ConnectionId(""));
    mockNetwork_.reset(new NiceMock<MockNetwork>);
    port1.reset(new NiceMock<MockPortDescription>);
    port2.reset(new NiceMock<MockPortDescription>);
  }

  std::string portsHaveSameType()
  {
    std::string color("c1");
    EXPECT_CALL(*port1, get_typename()).WillRepeatedly(Return(color));
    EXPECT_CALL(*port2, get_typename()).WillRepeatedly(Return(color));
    return color;
  }

  void portsHaveDifferentType()
  {
    std::string color1("c1");
    std::string color2("c2");
    EXPECT_CALL(*port1, get_typename()).WillRepeatedly(Return(color1));
    EXPECT_CALL(*port2, get_typename()).WillRepeatedly(Return(color2));
  }

  std::pair<ModuleId,ModuleId> portsAreOnDifferentModules()
  {
    ModuleId moduleId1("m:1");
    ModuleId moduleId2("m:2");
    EXPECT_CALL(*port1, getUnderlyingModuleId()).WillRepeatedly(Return(moduleId1));
    EXPECT_CALL(*port2, getUnderlyingModuleId()).WillRepeatedly(Return(moduleId2));
    return std::make_pair(moduleId1, moduleId2);
  }

  std::pair<ModuleId,ModuleId> portsAreOnSameModule()
  {
    ModuleId moduleId1("m:1");
    EXPECT_CALL(*port1, getUnderlyingModuleId()).WillRepeatedly(Return(moduleId1));
    EXPECT_CALL(*port2, getUnderlyingModuleId()).WillRepeatedly(Return(moduleId1));
    return std::make_pair(moduleId1, moduleId1);
  }

  std::pair<PortId,PortId> setPortIds()
  {
    PortId id1(0, "1");
    PortId id2(0, "2");
    EXPECT_CALL(*port1, id()).WillRepeatedly(Return(id1));
    EXPECT_CALL(*port2, id()).WillRepeatedly(Return(id2));
    return std::make_pair(id1, id2);
  }

  MockNetworkPtr mockNetwork_;
  DummySlotClassForNetworkEditorController slots_;
  ExecutionStrategyFactoryHandle null_;
  MockPortDescriptionPtr port1, port2;
};

TEST_F(NetworkEditorControllerTests, CanAddAndRemoveModulesWithSignalling)
{
  NetworkEditorController controller(mockNetwork_, null_);

  controller.connectModuleAdded(boost::bind(&SlotClassForNetworkEditorController::moduleAddedSlot, &slots_, _1, _2));
  controller.connectModuleRemoved(boost::bind(&SlotClassForNetworkEditorController::moduleRemovedSlot, &slots_, _1));

  EXPECT_CALL(slots_, moduleAddedSlot(_,_)).Times(1);
  EXPECT_CALL(*mockNetwork_, add_module(_)).Times(1);
  controller.addModule(ModuleLookupInfo("m1", "cat", "pack"));

  EXPECT_CALL(slots_, moduleRemovedSlot(_)).Times(1);
  EXPECT_CALL(*mockNetwork_, remove_module(ModuleId("m1"))).Times(1);
  controller.removeModule(ModuleId("m1"));
}

TEST_F(NetworkEditorControllerTests, CanAddAndRemoveConnectionWithSignalling)
{
  NetworkEditorController controller(mockNetwork_, null_);

  controller.connectConnectionAdded(boost::bind(&SlotClassForNetworkEditorController::connectionAddedSlot, &slots_, _1));
  controller.connectConnectionRemoved(boost::bind(&SlotClassForNetworkEditorController::connectionRemovedSlot, &slots_, _1));

  EXPECT_CALL(slots_, connectionAddedSlot(_)).Times(1);
  EXPECT_CALL(*mockNetwork_, connect(_,_)).Times(1).WillOnce(Return(ConnectionId("non empty string")));

  portsHaveSameType();
  auto modIds = portsAreOnDifferentModules();
  auto indices = setPortIds();

  EXPECT_CALL(*port1, isInput()).WillRepeatedly(Return(false));
  EXPECT_CALL(*port2, isInput()).WillRepeatedly(Return(true));
  EXPECT_CALL(*port2, nconnections()).WillOnce(Return(0));

  controller.requestConnection(port1.get(), port2.get());

  ConnectionDescription desc(OutgoingConnectionDescription(modIds.first, indices.first), IncomingConnectionDescription(modIds.second, indices.second));
  {
    EXPECT_CALL(slots_, connectionRemovedSlot(_)).Times(1);
    EXPECT_CALL(*mockNetwork_, disconnect(_)).Times(1).WillOnce(Return(true));
    controller.removeConnection(ConnectionId::create(desc));
  }

  {
    EXPECT_CALL(slots_, connectionRemovedSlot(_)).Times(0);
    EXPECT_CALL(*mockNetwork_, disconnect(_)).Times(1).WillRepeatedly(Return(false));
    controller.removeConnection(ConnectionId::create(desc));
  }
}

TEST_F(NetworkEditorControllerTests, CannotConnectInputPortToInputPort)
{
  NetworkEditorController controller(mockNetwork_, null_);
  controller.connectInvalidConnection(boost::bind(&SlotClassForNetworkEditorController::invalidConnectionSlot, &slots_, _1));

  EXPECT_CALL(*mockNetwork_, connect(_,_)).Times(0);

  portsHaveSameType();
  auto modIds = portsAreOnDifferentModules();
  auto indices = setPortIds();

  EXPECT_CALL(*port1, isInput()).WillRepeatedly(Return(true));
  EXPECT_CALL(*port2, isInput()).WillRepeatedly(Return(true));
  EXPECT_CALL(*port2, nconnections()).WillOnce(Return(0));

  ConnectionDescription desc(OutgoingConnectionDescription(modIds.second, indices.second), IncomingConnectionDescription(modIds.first, indices.first));
  EXPECT_CALL(slots_, invalidConnectionSlot(Eq(desc))).Times(1);

  controller.requestConnection(port1.get(), port2.get());
}

TEST_F(NetworkEditorControllerTests, CannotConnectOutputPortToOutputPort)
{
  NetworkEditorController controller(mockNetwork_, null_);
  controller.connectInvalidConnection(boost::bind(&SlotClassForNetworkEditorController::invalidConnectionSlot, &slots_, _1));

  EXPECT_CALL(*mockNetwork_, connect(_,_)).Times(0);

  portsHaveSameType();
  auto modIds = portsAreOnDifferentModules();
  auto indices = setPortIds();

  EXPECT_CALL(*port1, isInput()).WillRepeatedly(Return(false));
  EXPECT_CALL(*port2, isInput()).WillRepeatedly(Return(false));

  ConnectionDescription desc(OutgoingConnectionDescription(modIds.first, indices.first), IncomingConnectionDescription(modIds.second, indices.second));
  EXPECT_CALL(slots_, invalidConnectionSlot(Eq(desc))).Times(1);

  controller.requestConnection(port1.get(), port2.get());
}

TEST_F(NetworkEditorControllerTests, CannotConnectToFullInputPort)
{
  NetworkEditorController controller(mockNetwork_, null_);
  controller.connectInvalidConnection(boost::bind(&SlotClassForNetworkEditorController::invalidConnectionSlot, &slots_, _1));

  EXPECT_CALL(*mockNetwork_, connect(_,_)).Times(0);

  portsHaveSameType();
  auto modIds = portsAreOnDifferentModules();
  auto indices = setPortIds();

  EXPECT_CALL(*port1, isInput()).WillRepeatedly(Return(true));
  EXPECT_CALL(*port1, nconnections()).WillOnce(Return(1));
  EXPECT_CALL(*port2, isInput()).WillRepeatedly(Return(false));

  ConnectionDescription desc(OutgoingConnectionDescription(modIds.second, indices.second), IncomingConnectionDescription(modIds.first, indices.first));
  EXPECT_CALL(slots_, invalidConnectionSlot(Eq(desc))).Times(1);

  controller.requestConnection(port1.get(), port2.get());
}

TEST_F(NetworkEditorControllerTests, CannotConnectBetweenSameModule)
{
  NetworkEditorController controller(mockNetwork_, null_);
  controller.connectInvalidConnection(boost::bind(&SlotClassForNetworkEditorController::invalidConnectionSlot, &slots_, _1));

  EXPECT_CALL(*mockNetwork_, connect(_,_)).Times(0);

  portsHaveSameType();
  auto modIds = portsAreOnSameModule();
  auto indices = setPortIds();

  EXPECT_CALL(*port1, isInput()).WillRepeatedly(Return(true));
  EXPECT_CALL(*port2, isInput()).WillRepeatedly(Return(false));

  ConnectionDescription desc(OutgoingConnectionDescription(modIds.second, indices.second), IncomingConnectionDescription(modIds.first, indices.first));
  EXPECT_CALL(slots_, invalidConnectionSlot(Eq(desc))).Times(1);

  controller.requestConnection(port1.get(), port2.get());
}

TEST_F(NetworkEditorControllerTests, CannotConnectBetweenDifferentPortTypes)
{
  NetworkEditorController controller(mockNetwork_, null_);
  controller.connectInvalidConnection(boost::bind(&SlotClassForNetworkEditorController::invalidConnectionSlot, &slots_, _1));

  EXPECT_CALL(*mockNetwork_, connect(_,_)).Times(0);

  portsHaveDifferentType();
  auto modIds = portsAreOnDifferentModules();
  auto indices = setPortIds();

  EXPECT_CALL(*port1, isInput()).WillRepeatedly(Return(true));
  EXPECT_CALL(*port2, isInput()).WillRepeatedly(Return(false));

  ConnectionDescription desc(OutgoingConnectionDescription(modIds.second, indices.second), IncomingConnectionDescription(modIds.first, indices.first));
  EXPECT_CALL(slots_, invalidConnectionSlot(Eq(desc))).Times(1);

  controller.requestConnection(port1.get(), port2.get());
}
