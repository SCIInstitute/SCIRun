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

#include <QtGui>
#include <iostream>
#include <Dataflow/Engine/Controller/HistoryManager.h>
#include <Interface/Application/HistoryWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>

//TODO: factory
#include <Dataflow/Engine/Controller/HistoryItemImpl.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Engine;

HistoryWindow::HistoryWindow(HistoryManagerHandle historyManager, QWidget* parent /* = 0 */) : 
  historyManager_(historyManager),
  lastUndoRow_(-1),
  QDockWidget(parent) 
{
  setupUi(this);
  networkXMLTextEdit_->setTabStopWidth(15);

  connect(historyListWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(displayInfo(QListWidgetItem*)));
  connect(historyListWidget_, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(displayInfo(QListWidgetItem*)));
  connect(undoButton_, SIGNAL(clicked()), this, SLOT(undo()));
  connect(redoButton_, SIGNAL(clicked()), this, SLOT(redo()));
  connect(undoAllButton_, SIGNAL(clicked()), this, SLOT(undoAll()));
  connect(redoAllButton_, SIGNAL(clicked()), this, SLOT(redoAll()));
}

void HistoryWindow::showFile(SCIRun::Dataflow::Networks::NetworkFileHandle file)
{
  std::ostringstream ostr;
  XMLSerializer::save_xml(*file, ostr, "networkFile");
  QString xmlText = QString::fromStdString(ostr.str());
  networkXMLTextEdit_->setPlainText(xmlText);
  historyManager_->setInitialState(file);
}

//TODO:
/*
Key thing: when undoing/redoing, since we're loading from scratch, need to scoped-switch off the signals for history window.
*/

class HistoryWindowListItem : public QListWidgetItem
{
public:
  HistoryWindowListItem(HistoryItemHandle info, QListWidget* parent = 0) : 
    QListWidgetItem(QString::fromStdString(info->name()), parent),
    info_(info) 
  {
    auto xml = info_->memento();
    if (xml)
    {
      std::ostringstream ostr;
      XMLSerializer::save_xml(*xml, ostr, "networkFile");
      xmlText_ = QString::fromStdString(ostr.str());
    }
    else
      xmlText_ = "<Unknown state for this item>";
  }
  ~HistoryWindowListItem()
  {
    //std::cout << "deleting " << info_->name() << std::endl;
  }
  void setAsUndo()
  {
    QFont f = font();
    f.setItalic(false);
    setFont(f);
    setBackgroundColor(Qt::white);
  }
  void setAsRedo()
  {
    QFont f = font();
    f.setItalic(true);
    setFont(f);
    setBackgroundColor(Qt::lightGray);
  }
  QString xmlText() const 
  {
    return xmlText_;
  }
  std::string name() const
  {
    return info_->name();
  }
private:
  HistoryItemHandle info_;
  QString xmlText_;
};

void HistoryWindow::addHistoryItem(HistoryItemHandle item)
{
  //std::cout << "removing from " << lastUndoRow_+1 << " to " << historyListWidget_->count() -1 << std::endl;
  for (int i = historyListWidget_->count() - 1; i > lastUndoRow_; --i)
    delete historyListWidget_->takeItem(i);

  new HistoryWindowListItem(item, historyListWidget_);
  lastUndoRow_++;
  //std::cout << "!! HistoryWindow::addHistoryItem, rowIndex = " << lastUndoRow_ << std::endl;
  setRedoEnabled(false);
  setUndoEnabled(true);

  historyManager_->addItem(item);
}

void HistoryWindow::displayInfo(QListWidgetItem* item)
{
  auto historyItem = dynamic_cast<HistoryWindowListItem*>(item);
  if (historyItem)
  {
    networkXMLTextEdit_->setPlainText(historyItem->xmlText());
  }
}

void HistoryWindow::clear()
{
  historyListWidget_->clear();
  historyManager_->clearAll();
  lastUndoRow_ = -1;
  setUndoEnabled(false);
  setRedoEnabled(false);

  networkXMLTextEdit_->clear();
}

void HistoryWindow::setUndoEnabled(bool enable)
{
  undoButton_->setEnabled(enable);
  undoAllButton_->setEnabled(enable);
}

void HistoryWindow::setRedoEnabled(bool enable)
{
  redoButton_->setEnabled(enable);
  redoAllButton_->setEnabled(enable);
}

void HistoryWindow::undo()
{
  //std::cout << "!! HistoryWindow::Undo pressed, rowIndex = " << lastUndoRow_ << std::endl;

  auto item = historyListWidget_->item(lastUndoRow_);
  if (item)
  {
    auto historyItem = dynamic_cast<HistoryWindowListItem*>(item);
    historyItem->setAsRedo();
    
    {
      Q_EMIT modifyingNetwork(true);
      auto undone = historyManager_->undo();
      Q_EMIT modifyingNetwork(false);
      //std::cout << "undoing " << undone->name() << std::endl;
      if (undone->name() != historyItem->name())
        std::cout << "Inconsistency in history items. TODO: emit logical error here." << std::endl;
    }

    lastUndoRow_--;
    setRedoEnabled(true);
    if (lastUndoRow_ == -1)
    {
      setUndoEnabled(false);
    }
  }
  else
    std::cout << "oops, item is null" << std::endl;
  //std::cout << "\trowIndex now = " << lastUndoRow_ << std::endl;
}

void HistoryWindow::redo()
{
  //std::cout << "!! HistoryWindow::Redo pressed, rowIndex = " << lastUndoRow_  << std::endl;

  auto item = historyListWidget_->item(lastUndoRow_ + 1);
  if (item)
  {
    auto historyItem = dynamic_cast<HistoryWindowListItem*>(item);
    historyItem->setAsUndo();

    {
      Q_EMIT modifyingNetwork(true);
      auto redone = historyManager_->redo();
      Q_EMIT modifyingNetwork(false);
      //std::cout << "redoing " << redone->name() << std::endl;
      if (redone->name() != historyItem->name())
        std::cout << "Inconsistency in history items. TODO: emit logical error here." << std::endl;
    }


    lastUndoRow_++;
    setUndoEnabled(true);
    if (lastUndoRow_ == historyListWidget_->count() - 1)
    {
      setRedoEnabled(false);
    }
  }
  else
    std::cout << "oops, item is null" << std::endl;
  //std::cout << "\trowIndex now = " << lastUndoRow_ << std::endl;
}

void HistoryWindow::undoAll()
{
  for (int row = 0; row < historyListWidget_->count(); ++row)
  {
    auto historyItem = dynamic_cast<HistoryWindowListItem*>(historyListWidget_->item(row));
    historyItem->setAsRedo();
  }
  lastUndoRow_ = -1;

  Q_EMIT modifyingNetwork(true);
  historyManager_->undoAll();
  Q_EMIT modifyingNetwork(false);
  setUndoEnabled(false);
  setRedoEnabled(true);
}

void HistoryWindow::redoAll()
{
  for (int row = 0; row < historyListWidget_->count(); ++row)
  {
    auto historyItem = dynamic_cast<HistoryWindowListItem*>(historyListWidget_->item(row));
    historyItem->setAsUndo();
  }
  lastUndoRow_ = historyListWidget_->count() - 1;

  Q_EMIT modifyingNetwork(true);
  historyManager_->redoAll();
  Q_EMIT modifyingNetwork(false);
  setUndoEnabled(true);
  setRedoEnabled(false);
}

//----------------------------------------------------------
//TODO: separate out

GuiActionCommandHistoryConverter::GuiActionCommandHistoryConverter(NetworkEditor* editor) : editor_(editor) {}

void GuiActionCommandHistoryConverter::moduleAdded(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module)
{
  if (!historyManagerModifyingNetwork_)
  {
    HistoryItemHandle item(new ModuleAddedHistoryItem(name, editor_->saveNetwork()));
    Q_EMIT historyItemCreated(item);
  }
}

void GuiActionCommandHistoryConverter::moduleRemoved(const std::string& id)
{
  if (!historyManagerModifyingNetwork_)
  {
    HistoryItemHandle item(new ModuleRemovedHistoryItem(id, editor_->saveNetwork()));
    Q_EMIT historyItemCreated(item);
  }
}

void GuiActionCommandHistoryConverter::connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription& cd)
{
  if (!historyManagerModifyingNetwork_)
  {
    HistoryItemHandle item(new ConnectionAddedHistoryItem(cd, editor_->saveNetwork()));
    Q_EMIT historyItemCreated(item);
  }
}

void GuiActionCommandHistoryConverter::connectionRemoved(const SCIRun::Dataflow::Networks::ConnectionId& id)
{
  if (!historyManagerModifyingNetwork_)
  {
    HistoryItemHandle item(new ConnectionRemovedHistoryItem(id, editor_->saveNetwork()));
    Q_EMIT historyItemCreated(item);
  }
}

void GuiActionCommandHistoryConverter::moduleMoved(const std::string& id, double newX, double newY)
{
  if (!historyManagerModifyingNetwork_)
  {
    HistoryItemHandle item(new ModuleMovedHistoryItem(id, newX, newY, editor_->saveNetwork()));
    Q_EMIT historyItemCreated(item);
  }
}

void GuiActionCommandHistoryConverter::networkBeingModifiedByHistoryManager(bool inProgress)
{
  historyManagerModifyingNetwork_ = inProgress;
}