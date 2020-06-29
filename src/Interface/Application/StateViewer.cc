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
#include <QMenu>
#include <Interface/Application/StateViewer.h>
#include <Interface/Application/NetworkEditor.h>
#include <Interface/Application/ModuleProxyWidget.h>
#include <Interface/Application/ModuleWidget.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Core;

static const std::map<int, QString> variableTypeLabels =
  {
    {0, "int"},
    {1, "double"},
    {2, "string"},
    {3, "boolean"},
    {4, "option"},
    {5, "list"}
  };

StateViewer::StateViewer(NetworkEditor* network, QWidget* parent) : QDialog(parent)
{
  setupUi(this);

  stateTreeWidget_->clear();
  stateTreeWidget_->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(stateTreeWidget_, &QTreeWidget::customContextMenuRequested, this, &StateViewer::prepareMenu);
  for (const auto& item : network->scene()->items())
  {
    if (auto w = dynamic_cast<ModuleProxyWidget*>(item))
    {
      auto mod = new QTreeWidgetItem();
      auto widget = w->getModuleWidget();
      mod->setText(0, QString::fromStdString(widget->getModuleId()));
      stateTreeWidget_->addTopLevelItem(mod);
      auto module = widget->getModule();
      auto stateMap = module->cstate();
      auto keys = stateMap->getKeys();
      for (const auto& key : keys)
      {
        auto val = stateMap->getValue(key);
        std::ostringstream ostr;
        ostr << val.value();
        auto treeVal = new QTreeWidgetItem(mod);
        treeVal->setText(0, QString::fromStdString(key.name()));
        treeVal->setText(1, variableTypeLabels.at(val.value().which()));
        treeVal->setText(2, QString::fromStdString(ostr.str()));
      }
    }
  }
  stateTreeWidget_->resizeColumnToContents(0);
  stateTreeWidget_->sortByColumn(0, Qt::AscendingOrder);
}

void StateViewer::prepareMenu(const QPoint& pos)
{
  auto item = stateTreeWidget_->itemAt(pos);
  if (!item)
    return;

  if (!item->parent())
    return;

  auto parent = item->parent();
  auto moduleId = parent->text(0);
  auto stateId = item->text(0);
  auto stateType = item->text(1);
  auto stateValue = item->text(2);

  static auto quoteAString = [](const QString& val, const QString& type)
  {
    if (type == "string")
      return "'" + val + "'";
    return val;
  };

  auto getCode = new QAction("Copy Python state GET code to clipboard", this);
  connect(getCode, &QAction::triggered, [=]() { 
    QGuiApplication::clipboard()->setText(
      tr("scirun_get_module_state('%1', '%2')")
        .arg(moduleId).arg(stateId)); });
  auto setCode = new QAction("Copy Python state SET code to clipboard", this);
  connect(setCode, &QAction::triggered, [=]() {
    QGuiApplication::clipboard()->setText(
      tr("scirun_set_module_state('%1', '%2', %3)")
        .arg(moduleId).arg(stateId).arg(quoteAString(stateValue, stateType))); });

  QMenu menu(this);
  menu.addAction(getCode);
  menu.addAction(setCode);

  menu.exec(stateTreeWidget_->mapToGlobal(pos));
}