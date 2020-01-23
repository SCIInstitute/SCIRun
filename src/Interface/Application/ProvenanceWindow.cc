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


#include <QtGui>
#include <iostream>
#include <Dataflow/Engine/Controller/ProvenanceManager.h>
#include <Interface/Application/ProvenanceWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>

//TODO: factory
#include <Dataflow/Engine/Controller/ProvenanceItemImpl.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Engine;

ProvenanceWindow::ProvenanceWindow(ProvenanceManagerHandle provenanceManager, QWidget* parent /* = 0 */) : QDockWidget(parent),
  provenanceManager_(provenanceManager),
  lastUndoRow_(-1),
  networkEditor_(provenanceManager->networkIO())
{
  setupUi(this);
  // TODO deprecated
  //networkXMLTextEdit_->setTabStopWidth(15);

  connect(provenanceListWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(displayInfo(QListWidgetItem*)));
  connect(provenanceListWidget_, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(displayInfo(QListWidgetItem*)));
  connect(undoButton_, SIGNAL(clicked()), this, SLOT(undo()));
  connect(redoButton_, SIGNAL(clicked()), this, SLOT(redo()));
  connect(undoAllButton_, SIGNAL(clicked()), this, SLOT(undoAll()));
  connect(redoAllButton_, SIGNAL(clicked()), this, SLOT(redoAll()));
  connect(clearButton_, SIGNAL(clicked()), this, SLOT(clear()));
  connect(itemMaxSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(setMaxItems(int)));
  setMaxItems(10);
  setUndoEnabled(false);
  setRedoEnabled(false);
}

void ProvenanceWindow::showFile(SCIRun::Dataflow::Networks::NetworkFileHandle file)
{
  std::ostringstream ostr;
  XMLSerializer::save_xml(*file, ostr, "networkFile");
  QString xmlText = QString::fromStdString(ostr.str());
  networkXMLTextEdit_->setPlainText(xmlText);
  provenanceManager_->setInitialState(file);
}

//TODO:
/*
Key thing: when undoing/redoing, since we're loading from scratch, need to scoped-switch off the signals for provenance window.
*/

class ProvenanceWindowListItem : public QListWidgetItem
{
public:
  explicit ProvenanceWindowListItem(ProvenanceItemHandle info, QListWidget* parent = nullptr) :
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
  void setAsUndo()
  {
    QFont f = font();
    f.setItalic(false);
    setFont(f);
    setBackground(Qt::white);
  }
  void setAsRedo()
  {
    QFont f = font();
    f.setItalic(true);
    setFont(f);
    setBackground(Qt::lightGray);
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
  ProvenanceItemHandle info_;
  QString xmlText_;
};

void ProvenanceWindow::addProvenanceItem(ProvenanceItemHandle item)
{
  for (int i = provenanceListWidget_->count() - 1; i > lastUndoRow_; --i)
    delete provenanceListWidget_->takeItem(i);

  if (provenanceListWidget_->count() == maxItems_)
  {
    delete provenanceListWidget_->takeItem(0);
  }
  else
  {
    lastUndoRow_++;
  }

  new ProvenanceWindowListItem(item, provenanceListWidget_);
  setRedoEnabled(false);
  setUndoEnabled(true);

  provenanceManager_->addItem(item);
}

void ProvenanceWindow::displayInfo(QListWidgetItem* item)
{
  auto provenanceItem = dynamic_cast<ProvenanceWindowListItem*>(item);
  if (provenanceItem)
  {
    networkXMLTextEdit_->setPlainText(provenanceItem->xmlText());
  }
}

void ProvenanceWindow::clear()
{
  provenanceManager_->setInitialState(networkEditor_->saveNetwork());
  provenanceListWidget_->clear();
  provenanceManager_->clearAll();
  lastUndoRow_ = -1;
  setUndoEnabled(false);
  setRedoEnabled(false);

  networkXMLTextEdit_->clear();
}

void ProvenanceWindow::setMaxItems(int max)
{
  if (maxItems_ == max)
    return;

  maxItems_ = max;
  itemMaxSpinBox_->setValue(max);
  for (int i = 0; i < provenanceListWidget_->count() - max; ++i)
  {
    delete provenanceListWidget_->takeItem(0);
  }
}

void ProvenanceWindow::setUndoEnabled(bool enable)
{
  undoButton_->setEnabled(enable);
  undoAllButton_->setEnabled(enable);
  Q_EMIT undoStateChanged(enable);
}

void ProvenanceWindow::setRedoEnabled(bool enable)
{
  redoButton_->setEnabled(enable);
  redoAllButton_->setEnabled(enable);
  Q_EMIT redoStateChanged(enable);
}

void ProvenanceWindow::undo()
{
  auto item = provenanceListWidget_->item(lastUndoRow_);
  if (item)
  {
    auto provenanceItem = dynamic_cast<ProvenanceWindowListItem*>(item);
    provenanceItem->setAsRedo();

    {
      Q_EMIT modifyingNetwork(true);
      auto undone = provenanceManager_->undo();
      Q_EMIT modifyingNetwork(false);
      Q_EMIT networkModified();
      if (undone->name() != provenanceItem->name())
        std::cout << "Inconsistency in provenance items. TODO: emit logical error here." << std::endl;
    }

    lastUndoRow_--;
    setRedoEnabled(true);
    if (lastUndoRow_ == -1)
    {
      setUndoEnabled(false);
    }
  }
  else
    std::cout << "Logical error: provenance item is null" << std::endl;
}

void ProvenanceWindow::redo()
{
  auto item = provenanceListWidget_->item(lastUndoRow_ + 1);
  if (item)
  {
    auto provenanceItem = dynamic_cast<ProvenanceWindowListItem*>(item);
    provenanceItem->setAsUndo();

    {
      Q_EMIT modifyingNetwork(true);
      auto redone = provenanceManager_->redo();
      Q_EMIT modifyingNetwork(false);
      Q_EMIT networkModified();
      if (redone->name() != provenanceItem->name())
        std::cout << "Inconsistency in provenance items. TODO: emit logical error here." << std::endl;
    }

    lastUndoRow_++;
    setUndoEnabled(true);
    if (lastUndoRow_ == provenanceListWidget_->count() - 1)
    {
      setRedoEnabled(false);
    }
  }
  else
    std::cout << "Logical error: provenance item is null" << std::endl;
}

void ProvenanceWindow::undoAll()
{
  for (int row = 0; row < provenanceListWidget_->count(); ++row)
  {
    auto provenanceItem = dynamic_cast<ProvenanceWindowListItem*>(provenanceListWidget_->item(row));
    provenanceItem->setAsRedo();
  }
  lastUndoRow_ = -1;

  Q_EMIT modifyingNetwork(true);
  provenanceManager_->undoAll();
  Q_EMIT modifyingNetwork(false);
  Q_EMIT networkModified();
  setUndoEnabled(false);
  setRedoEnabled(true);
}

void ProvenanceWindow::redoAll()
{
  for (int row = 0; row < provenanceListWidget_->count(); ++row)
  {
    auto provenanceItem = dynamic_cast<ProvenanceWindowListItem*>(provenanceListWidget_->item(row));
    provenanceItem->setAsUndo();
  }
  lastUndoRow_ = provenanceListWidget_->count() - 1;

  Q_EMIT modifyingNetwork(true);
  provenanceManager_->redoAll();
  Q_EMIT modifyingNetwork(false);
  Q_EMIT networkModified();
  setUndoEnabled(true);
  setRedoEnabled(false);
}

//----------------------------------------------------------
//TODO: separate out

GuiActionProvenanceConverter::GuiActionProvenanceConverter(NetworkEditor* editor) :
  editor_(editor),
  provenanceManagerModifyingNetwork_(false)
{}

void GuiActionProvenanceConverter::moduleAdded(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module)
{
  if (!provenanceManagerModifyingNetwork_)
  {
    ProvenanceItemHandle item(boost::make_shared<ModuleAddedProvenanceItem>(name, editor_->saveNetwork()));
    Q_EMIT provenanceItemCreated(item);
  }
}

void GuiActionProvenanceConverter::moduleRemoved(const ModuleId& id)
{
  if (!provenanceManagerModifyingNetwork_)
  {
    ProvenanceItemHandle item(boost::make_shared<ModuleRemovedProvenanceItem>(id, editor_->saveNetwork()));
    Q_EMIT provenanceItemCreated(item);
  }
}

void GuiActionProvenanceConverter::connectionAdded(const SCIRun::Dataflow::Networks::ConnectionDescription& cd)
{
  if (!provenanceManagerModifyingNetwork_)
  {
    ProvenanceItemHandle item(boost::make_shared<ConnectionAddedProvenanceItem>(cd, editor_->saveNetwork()));
    Q_EMIT provenanceItemCreated(item);
  }
}

void GuiActionProvenanceConverter::connectionRemoved(const SCIRun::Dataflow::Networks::ConnectionId& id)
{
  if (!provenanceManagerModifyingNetwork_)
  {
    ProvenanceItemHandle item(boost::make_shared<ConnectionRemovedProvenanceItem>(id, editor_->saveNetwork()));
    Q_EMIT provenanceItemCreated(item);
  }
}

void GuiActionProvenanceConverter::moduleMoved(const SCIRun::Dataflow::Networks::ModuleId& id, double newX, double newY)
{
  if (!provenanceManagerModifyingNetwork_)
  {
    ProvenanceItemHandle item(boost::make_shared<ModuleMovedProvenanceItem>(id, newX, newY, editor_->saveNetwork()));
    Q_EMIT provenanceItemCreated(item);
  }
}

void GuiActionProvenanceConverter::networkBeingModifiedByProvenanceManager(bool inProgress)
{
  provenanceManagerModifyingNetwork_ = inProgress;
}
