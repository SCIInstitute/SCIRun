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
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Engine/Controller/HistoryItem.h>
#include <Dataflow/Engine/Controller/HistoryManager.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using ::testing::_;
using ::testing::Eq;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;


class MockNetworkIO : public NetworkIOInterface
{
public:
  MOCK_CONST_METHOD0(saveNetwork, NetworkFileHandle());
  MOCK_METHOD1(loadNetwork, void(const NetworkFileHandle&));
};

typedef boost::shared_ptr<MockNetworkIO> MockNetworkIOPtr;

class HistoryManagerTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    DefaultValue<ModuleHandle>::Set(ModuleHandle());
    DefaultValue<ConnectionId>::Set(ConnectionId(""));
    controller_.reset(new NiceMock<MockNetworkIO>);
  }
  
  class DummyHistoryItem : public HistoryItem
  {
  public:
    explicit DummyHistoryItem(const std::string& name) : name_(name) {}
    virtual std::string name() const { return name_; }
    virtual NetworkFileHandle memento() const { return NetworkFileHandle(); }
  private:
    std::string name_;
  };

  HistoryItemHandle item(const std::string& name)
  {
    return HistoryItemHandle(new DummyHistoryItem(name));
  }

  MockNetworkIOPtr controller_;
  NetworkExecutorHandle null_;
};

TEST_F(HistoryManagerTests, CanAddItems)
{
  HistoryManager manager(controller_);
  
  EXPECT_EQ(0, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  manager.addItem(item("1"));
  manager.addItem(item("2"));

  EXPECT_EQ(2, manager.undoSize());
}

TEST_F(HistoryManagerTests, CanClear)
{
  HistoryManager manager(controller_);

  EXPECT_EQ(0, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  manager.addItem(item("1"));
  manager.addItem(item("2"));

  EXPECT_EQ(2, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  manager.clearAll();

  EXPECT_EQ(0, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());
}

TEST_F(HistoryManagerTests, CanUndoItem)
{
  HistoryManager manager(controller_);

  manager.addItem(item("1"));
  manager.addItem(item("2"));

  EXPECT_EQ(2, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  EXPECT_CALL(*controller_, loadNetwork(_)).Times(1);
  auto undone = manager.undo();

  EXPECT_EQ("2", undone->name());
  EXPECT_EQ(1, manager.undoSize());
  EXPECT_EQ(1, manager.redoSize());
}

TEST_F(HistoryManagerTests, CanRedoUndoneItem)
{
  HistoryManager manager(controller_);

  manager.addItem(item("1"));
  manager.addItem(item("2"));

  {
    EXPECT_CALL(*controller_, loadNetwork(_)).Times(1);
    auto undone = manager.undo();

    EXPECT_EQ("2", undone->name());
    EXPECT_EQ(1, manager.undoSize());
    EXPECT_EQ(1, manager.redoSize());
  }

  {
    EXPECT_CALL(*controller_, loadNetwork(_)).Times(1);
    auto redone = manager.redo();
    EXPECT_EQ("2", redone->name());
    EXPECT_EQ(2, manager.undoSize());
    EXPECT_EQ(0, manager.redoSize());
  }
}

TEST_F(HistoryManagerTests, CannotUndoWhenEmpty)
{
  HistoryManager manager(controller_);

  EXPECT_EQ(0, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  EXPECT_CALL(*controller_, loadNetwork(_)).Times(0);
  auto undone = manager.undo();
  EXPECT_FALSE(undone);
}

TEST_F(HistoryManagerTests, CannotRedoWhenEmpty)
{
  HistoryManager manager(controller_);

  manager.addItem(item("1"));
  EXPECT_EQ(1, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  EXPECT_CALL(*controller_, loadNetwork(_)).Times(0);
  auto redone = manager.redo();
  EXPECT_FALSE(redone);
}

TEST_F(HistoryManagerTests, CanUndoAll)
{
  HistoryManager manager(controller_);

  manager.addItem(item("1"));
  manager.addItem(item("2"));
  manager.addItem(item("3"));

  EXPECT_EQ(3, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  EXPECT_CALL(*controller_, loadNetwork(_)).Times(3);
  auto undone = manager.undoAll();
  EXPECT_EQ(3, undone.size());

  EXPECT_EQ(0, manager.undoSize());
  EXPECT_EQ(3, manager.redoSize());
}

TEST_F(HistoryManagerTests, CanRedoAll)
{
  HistoryManager manager(controller_);

  manager.addItem(item("1"));
  manager.addItem(item("2"));
  manager.addItem(item("3"));

  EXPECT_EQ(3, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  {
    EXPECT_CALL(*controller_, loadNetwork(_)).Times(3);
    auto undone = manager.undoAll();
    EXPECT_EQ(3, undone.size());

    EXPECT_EQ(0, manager.undoSize());
    EXPECT_EQ(3, manager.redoSize());
  }

  {
    EXPECT_CALL(*controller_, loadNetwork(_)).Times(3);
    auto redone = manager.redoAll();
    EXPECT_EQ(3, redone.size());

    EXPECT_EQ(3, manager.undoSize());
    EXPECT_EQ(0, manager.redoSize());
  }
}

TEST_F(HistoryManagerTests, AddItemWipesOutRedoStack)
{
  HistoryManager manager(controller_);

  manager.addItem(item("1"));
  manager.addItem(item("2"));
  manager.addItem(item("3"));

  EXPECT_EQ(3, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  manager.undo();

  EXPECT_EQ(2, manager.undoSize());
  EXPECT_EQ(1, manager.redoSize());

  manager.addItem(item("4"));

  EXPECT_EQ(3, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());
}