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
#include <Interface/Modules/Base/ModuleDialogBasic.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

ModuleDialogBasic::ModuleDialogBasic(const std::string& name, QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(nullptr, parent)
{
  setupUi(this);
  setModal(false);
  setWindowTitle(QString::fromStdString(name));
}

SubnetDialog::SubnetDialog(const std::string& name, ModuleStateHandle state, QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setModal(false);
  setWindowTitle(QString::fromStdString(name));
  WidgetStyleMixin::tableHeaderStyle(moduleTableWidget_);
}

void SubnetDialog::pullSpecial()
{
  auto tableValues = state_->getValue(Core::Algorithms::Name("ModuleInfo")).toVector();

  moduleTableWidget_->setRowCount(static_cast<int>(tableValues.size()));

  int i = 0;
  int j = 0;
  for (const auto& row : tableValues)
  {
    for (const auto& ap : row.toVector())
    {
      auto tmpstr = ap.toString();
      auto item = new QTableWidgetItem(QString::fromStdString(tmpstr));
      moduleTableWidget_->setItem(i, j, item);
      j++;
    }
    i++;
    j = 0;
  }

  moduleTableWidget_->resizeColumnsToContents();
}
