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
