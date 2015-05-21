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

#include <Modules/Legacy/Bundle/GetFieldsFromBundle.h>
#include <Interface/Modules/Bundle/GetFieldsFromBundleDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Modules::Bundles;

GetFieldsFromBundleDialog::GetFieldsFromBundleDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
}

void GetFieldsFromBundleDialog::pullSpecial()
{
  auto names = optional_any_cast_or_default<std::vector<std::string>>(state_->getTransientValue(GetFieldsFromBundle::FieldNameList.name()));
  for (const auto& name : names)
    fieldObjectListWidget_->addItem(QString::fromStdString(name));

  field1NameLineEdit_->setText(QString::fromStdString(state_->getValue(GetFieldsFromBundle::FieldNames[0]).toString()));
  field2NameLineEdit_->setText(QString::fromStdString(state_->getValue(GetFieldsFromBundle::FieldNames[1]).toString()));
  field3NameLineEdit_->setText(QString::fromStdString(state_->getValue(GetFieldsFromBundle::FieldNames[2]).toString()));
  field4NameLineEdit_->setText(QString::fromStdString(state_->getValue(GetFieldsFromBundle::FieldNames[3]).toString()));
  field5NameLineEdit_->setText(QString::fromStdString(state_->getValue(GetFieldsFromBundle::FieldNames[4]).toString()));
  field6NameLineEdit_->setText(QString::fromStdString(state_->getValue(GetFieldsFromBundle::FieldNames[5]).toString()));
  /*
  for (int i = 0; i < GetFieldsFromBundle::NUM_BUNDLE_OUT; ++i)
  {
    field
  }
  */

  //auto numFields = state_->getValue(SCIRun::Modules::Bundles::InsertFieldsIntoBundle::NumFields).toInt();
  //tableWidget->setRowCount(numFields);
  //for (int i = 0; i < numFields; ++i)
  //{
  //  auto name = new QTableWidgetItem(tr("Field %1").arg(i+1));
  //  tableWidget->setItem(i, 0, name);
  //  auto check = new QTableWidgetItem();
  //  check->setCheckState(Qt::Checked);
  //  check->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
  //  tableWidget->setItem(i, 1, check);
  //  auto info = new QTableWidgetItem("[unknown, populated upon execute]");
  //  //info->setFlags(Qt::NoItemFlags);
  //  tableWidget->setItem(i, 2, info);
  //}
  //tableWidget->resizeColumnsToContents();
}
