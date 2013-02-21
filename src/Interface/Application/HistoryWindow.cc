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
#include <Interface/Application/HistoryWindow.h>
#include <Interface/Application/NetworkEditor.h>
#include <Dataflow/Serialization/Network/NetworkDescriptionSerialization.h>
#include <Dataflow/Serialization/Network/XMLSerializer.h>

//TODO: factory
#include <Dataflow/Engine/Controller/HistoryItemImpl.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Dataflow::Engine;

HistoryWindow::HistoryWindow(QWidget* parent /* = 0 */) : QDockWidget(parent) 
{
  setupUi(this);
  networkXMLTextEdit_->setTabStopWidth(15);
  connect(historyListWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(displayInfo(QListWidgetItem*)));
}

void HistoryWindow::showFile(const QString& path)
{
  setWindowTitle("History: " + path);
  QFile xmlFile(path);
  if(!xmlFile.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
    std::cout << "Could not open file: " << path.toStdString() << std::endl;
    return;
  }
 
  networkXMLTextEdit_->setPlainText(xmlFile.readAll());
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
  QString xmlText() const 
  {
    return xmlText_;
  }
private:
  HistoryItemHandle info_;
  QString xmlText_;
};

void HistoryWindow::addHistoryItem(HistoryItemHandle item)
{
  new HistoryWindowListItem(item, historyListWidget_);
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
  networkXMLTextEdit_->clear();
}

//TODO: separate out

GuiActionCommandHistoryConverter::GuiActionCommandHistoryConverter(NetworkEditor* editor) : editor_(editor) {}

void GuiActionCommandHistoryConverter::moduleAdded(const std::string& name, SCIRun::Dataflow::Networks::ModuleHandle module)
{
  HistoryItemHandle item(new ModuleAddHistoryItem(name, editor_->saveNetwork()));
  Q_EMIT historyItemCreated(item);
}

void GuiActionCommandHistoryConverter::moduleRemoved(const std::string& name)
{
  HistoryItemHandle item(new ModuleRemovedHistoryItem(name, editor_->saveNetwork()));
  Q_EMIT historyItemCreated(item);
}