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


class MockNetworkIO : public NetworkIOInterface<std::string>
{
public:
  MOCK_CONST_METHOD0(saveNetwork, std::string());
  MOCK_METHOD1(loadNetwork, void(const std::string&));
  MOCK_METHOD0(clear, void());
};

typedef boost::shared_ptr<MockNetworkIO> MockNetworkIOPtr;

class HistoryManagerTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    //DefaultValue<ModuleHandle>::Set(ModuleHandle());
//    DefaultValue<ConnectionId>::Set(ConnectionId(""));
    controller_.reset(new NiceMock<MockNetworkIO>);
  }
  
  class DummyHistoryItem : public HistoryItem<std::string>
  {
  public:
    explicit DummyHistoryItem(const std::string& name) : name_(name) {}
    virtual std::string name() const { return name_; }
    virtual std::string memento() const { return name_; }
  private:
    std::string name_;
  };

  HistoryItem<std::string>::Handle item(const std::string& name)
  {
    return HistoryItem<std::string>::Handle(new DummyHistoryItem(name));
  }

  MockNetworkIOPtr controller_;
  NetworkExecutorHandle null_;
};

TEST_F(HistoryManagerTests, CanAddItems)
{
  HistoryManager<std::string> manager(controller_.get());
  
  EXPECT_EQ(0, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  manager.addItem(item("1"));
  manager.addItem(item("2"));

  EXPECT_EQ(2, manager.undoSize());
}

TEST_F(HistoryManagerTests, CanClear)
{
  HistoryManager<std::string> manager(controller_.get());

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
  HistoryManager<std::string> manager(controller_.get());

  manager.addItem(item("1"));
  manager.addItem(item("2"));

  EXPECT_EQ(2, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  EXPECT_CALL(*controller_, clear()).Times(1);
  EXPECT_CALL(*controller_, loadNetwork("1")).Times(1);
  auto undone = manager.undo();

  EXPECT_EQ("2", undone->name());
  EXPECT_EQ(1, manager.undoSize());
  EXPECT_EQ(1, manager.redoSize());
}

TEST_F(HistoryManagerTests, CanRedoUndoneItem)
{
  HistoryManager<std::string> manager(controller_.get());

  manager.addItem(item("1"));
  manager.addItem(item("2"));

  {
    EXPECT_CALL(*controller_, clear()).Times(1);
    EXPECT_CALL(*controller_, loadNetwork("1")).Times(1);
    auto undone = manager.undo();

    EXPECT_EQ("2", undone->name());
    EXPECT_EQ(1, manager.undoSize());
    EXPECT_EQ(1, manager.redoSize());
  }

  {
    EXPECT_CALL(*controller_, clear()).Times(1);
    EXPECT_CALL(*controller_, loadNetwork("2")).Times(1);
    auto redone = manager.redo();
    EXPECT_EQ("2", redone->name());
    EXPECT_EQ(2, manager.undoSize());
    EXPECT_EQ(0, manager.redoSize());
  }
}

TEST_F(HistoryManagerTests, CannotUndoWhenEmpty)
{
  HistoryManager<std::string> manager(controller_.get());

  EXPECT_EQ(0, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  EXPECT_CALL(*controller_, clear()).Times(0);
  EXPECT_CALL(*controller_, loadNetwork(_)).Times(0);
  auto undone = manager.undo();
  EXPECT_FALSE(undone);
}

TEST_F(HistoryManagerTests, CannotRedoWhenEmpty)
{
  HistoryManager<std::string> manager(controller_.get());

  manager.addItem(item("1"));
  EXPECT_EQ(1, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  EXPECT_CALL(*controller_, loadNetwork(_)).Times(0);
  auto redone = manager.redo();
  EXPECT_FALSE(redone);
}

//TODO: need test case (no situation for it yet) for "undo all does not completely clear the network"
TEST_F(HistoryManagerTests, CanUndoAll)
{
  HistoryManager<std::string> manager(controller_.get());

  manager.addItem(item("1"));
  manager.addItem(item("2"));
  manager.addItem(item("3"));

  EXPECT_EQ(3, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  EXPECT_CALL(*controller_, clear()).Times(1);
  EXPECT_CALL(*controller_, loadNetwork(_)).Times(0);
  auto undone = manager.undoAll();
  EXPECT_EQ(3, undone.size());

  EXPECT_EQ(0, manager.undoSize());
  EXPECT_EQ(3, manager.redoSize());
}

TEST_F(HistoryManagerTests, CanRedoAll)
{
  HistoryManager<std::string> manager(controller_.get());

  manager.addItem(item("1"));
  manager.addItem(item("2"));
  manager.addItem(item("3"));

  EXPECT_EQ(3, manager.undoSize());
  EXPECT_EQ(0, manager.redoSize());

  {
    EXPECT_CALL(*controller_, clear()).Times(1);
    EXPECT_CALL(*controller_, loadNetwork(_)).Times(0);
    auto undone = manager.undoAll();
    EXPECT_EQ(3, undone.size());

    EXPECT_EQ(0, manager.undoSize());
    EXPECT_EQ(3, manager.redoSize());
  }

  {
    EXPECT_CALL(*controller_, clear()).Times(1);
    EXPECT_CALL(*controller_, loadNetwork("3")).Times(1);
    auto redone = manager.redoAll();
    EXPECT_EQ(3, redone.size());

    EXPECT_EQ(3, manager.undoSize());
    EXPECT_EQ(0, manager.redoSize());
  }
}

TEST_F(HistoryManagerTests, AddItemWipesOutRedoStack)
{
  HistoryManager<std::string> manager(controller_.get());

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

TEST_F(HistoryManagerTests, LoadFileSetsInitialState)
{
  HistoryManager<std::string> manager(controller_.get());

  manager.setInitialState("initial");

  manager.addItem(item("1"));

  EXPECT_CALL(*controller_, clear()).Times(1);
  EXPECT_CALL(*controller_, loadNetwork("initial")).Times(1);
  manager.undo();
}