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
#include <Dataflow/Network/ModuleInterface.h>
#include <Dataflow/Network/Tests/MockNetwork.h>
#include <Dataflow/Engine/Controller/HistoryItem.h>
#include <Dataflow/Engine/Controller/HistoryManager.h>

using namespace SCIRun;
using namespace SCIRun::Dataflow::Engine;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Networks::Mocks;
using ::testing::_;
using ::testing::Eq;
using ::testing::NiceMock;
using ::testing::DefaultValue;
using ::testing::Return;


class HistoryManagerTests : public ::testing::Test
{
protected:
  virtual void SetUp()
  {
    DefaultValue<ModuleHandle>::Set(ModuleHandle());
    DefaultValue<ConnectionId>::Set(ConnectionId(""));
    mockNetwork_.reset(new NiceMock<MockNetwork>);
    controller_.reset(new NetworkEditorController(mockNetwork_, null_));
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

  MockNetworkPtr mockNetwork_;
  NetworkEditorControllerHandle controller_;
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

  auto undone = manager.undo();

  EXPECT_EQ("2", undone->name());
  EXPECT_EQ(1, manager.undoSize());
  EXPECT_EQ(1, manager.redoSize());
}