/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Modules/Legacy/Bundle/InsertFieldsIntoBundle.h>
#include <Interface/Modules/Bundle/InsertFieldsIntoBundleDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

InsertFieldsIntoBundleDialog::InsertFieldsIntoBundleDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  WidgetStyleMixin::tableHeaderStyle(tableWidget);
}

void InsertFieldsIntoBundleDialog::pullSpecial()
{
  auto numFields = transient_value_cast<int>(state_->getTransientValue(SCIRun::Modules::Bundles::InsertFieldsIntoBundle::NumFields.name()));
  tableWidget->setRowCount(numFields);
  for (int i = 0; i < numFields; ++i)
  {
    auto name = new QTableWidgetItem(tr("Field %1").arg(i+1));
    tableWidget->setItem(i, 0, name);
    auto check = new QTableWidgetItem();
    check->setCheckState(Qt::Checked);
    check->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
    tableWidget->setItem(i, 1, check);
    auto info = new QTableWidgetItem("[unknown, populated upon execute]");
    //info->setFlags(Qt::NoItemFlags);
    tableWidget->setItem(i, 2, info);
  }
  tableWidget->resizeColumnsToContents();
}
