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

#include <Interface/Modules/Forward/BuildBEMatrixDialog.h>
#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Forward;

namespace TableColumns
{
  const int MinColumn = 0;

  const int FieldName = 0;
  const int FieldType = 1;
  const int BoundaryCondition = 2;
  const int InsideConductivity = 3;
  const int OutsideConductivity = 4;

  const int MaxColumn = OutsideConductivity + 1;
}

BuildBEMatrixDialog::BuildBEMatrixDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  WidgetStyleMixin::tableHeaderStyle(this->tableWidget);
  tableWidget->resizeColumnsToContents();

  connect(tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(pushTable(int,int)));
}

void BuildBEMatrixDialog::updateFromPortChange(int numPorts, const std::string&)
{
  //std::cout << "updateFromPortChange " << numPorts << std::endl;
  auto oldRowCount = tableWidget->rowCount();
  tableWidget->setRowCount(numPorts - 1);
  tableWidget->blockSignals(true);
  for (int i = oldRowCount; i < tableWidget->rowCount(); ++i)
  {
    using namespace TableColumns;
    tableWidget->setItem(i, FieldName, new QTableWidgetItem("field" + QString::number(i)));
    tableWidget->setItem(i, FieldType, new QTableWidgetItem("[populated on execute]"));
    tableWidget->setCellWidget(i, BoundaryCondition, makeComboBoxItem(i));
    tableWidget->setCellWidget(i, InsideConductivity, makeDoubleEntryItem(i, InsideConductivity));
    tableWidget->setCellWidget(i, OutsideConductivity, makeDoubleEntryItem(i, OutsideConductivity));

    // type is readonly
    auto type = tableWidget->item(i, FieldType);
    type->setFlags(type->flags() & ~Qt::ItemIsEditable);
    pushTableRow(i);
  }
  pull();
  tableWidget->resizeColumnsToContents();
  tableWidget->blockSignals(false);
}

QComboBox* BuildBEMatrixDialog::makeComboBoxItem(int i) const
{
  QStringList bcList;
  bcList << "Measurement (Neumann)" << "Source (Dirichlet)";
  QComboBox* bcBox = new QComboBox();
  bcBox->addItems(bcList);
  bcBox->setCurrentIndex(i == 0 ? 1 : 0);
  connect(bcBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pushBoundaryConditions()));
  return bcBox;
}

QDoubleSpinBox* BuildBEMatrixDialog::makeDoubleEntryItem(int row, int col) const
{
  auto spin = new QDoubleSpinBox();
  spin->setValue((row + col + 1) % 2);
  const char* slot = col == TableColumns::InsideConductivity ? SLOT(pushInsides()) : SLOT(pushOutsides());
  connect(spin, SIGNAL(valueChanged(double)), this, slot);
  return spin;
}

void BuildBEMatrixDialog::pushTable(int row, int col)
{
  using namespace TableColumns;
  if (FieldName == col)
    pushNames();
  else if (BoundaryCondition == col)
    pushBoundaryConditions();
  else if (InsideConductivity == col)
    pushInsides();
  else if (OutsideConductivity == col)
    pushOutsides();
}

void BuildBEMatrixDialog::pushTableRow(int row)
{
  using namespace TableColumns;
  for (int col = MinColumn; col < MaxColumn; ++col)
    pushTable(row, col);
}

void BuildBEMatrixDialog::pushNames()
{
  using namespace TableColumns;
  if (!pulling_)
  {
    VariableList names;
    for (int i = 0; i < tableWidget->rowCount(); ++i)
    {
      auto item = tableWidget->item(i, FieldName);
      names.push_back(makeVariable("", item->text().toStdString()));
    }
    state_->setValue(Parameters::FieldNameList, names);
  }
}

void BuildBEMatrixDialog::pushBoundaryConditions()
{
  using namespace TableColumns;
  if (!pulling_)
  {
    VariableList names;
    for (int i = 0; i < tableWidget->rowCount(); ++i)
    {
      auto box = qobject_cast<QComboBox*>(tableWidget->cellWidget(i, BoundaryCondition));
      names.push_back(makeVariable("", box->currentText().toStdString()));
    }
    state_->setValue(Parameters::BoundaryConditionList, names);
  }
}

void BuildBEMatrixDialog::pushInsides()
{
  using namespace TableColumns;
  if (!pulling_)
  {
    VariableList names;
    for (int i = 0; i < tableWidget->rowCount(); ++i)
    {
      auto box = qobject_cast<QDoubleSpinBox*>(tableWidget->cellWidget(i, InsideConductivity));
      names.push_back(makeVariable("", box->value()));
    }
    state_->setValue(Parameters::InsideConductivityList, names);
  }
}

void BuildBEMatrixDialog::pushOutsides()
{
  using namespace TableColumns;
  if (!pulling_)
  {
    VariableList names;
    for (int i = 0; i < tableWidget->rowCount(); ++i)
    {
      auto box = qobject_cast<QDoubleSpinBox*>(tableWidget->cellWidget(i, OutsideConductivity));
      names.push_back(makeVariable("", box->value()));
    }
    state_->setValue(Parameters::OutsideConductivityList, names);
  }
}

void BuildBEMatrixDialog::pullSpecial()
{
  pullNames();
  pullFieldTypes();
  pullBoundaryConditions();
  pullInsides();
  pullOutsides();
}

void BuildBEMatrixDialog::pullNames()
{
  using namespace TableColumns;
  Pulling p(this);
  auto nameList = state_->getValue(Parameters::FieldNameList).toVector();
  const int rows = std::min(static_cast<int>(nameList.size()), tableWidget->rowCount());
  for (int row = 0; row < rows; ++row)
  {
    auto item = tableWidget->item(row, FieldName);
    item->setText(QString::fromStdString(nameList[row].toString()));
  }
}

void BuildBEMatrixDialog::pullFieldTypes()
{
  using namespace TableColumns;
  Pulling p(this);
  auto typeList = transient_value_cast<FieldTypeListType>(state_->getTransientValue(Parameters::FieldTypeList));
  const int rows = std::min(static_cast<int>(typeList.size()), tableWidget->rowCount());
  for (int row = 0; row < rows; ++row)
  {
    auto item = tableWidget->item(row, FieldType);
    item->setText(QString::fromStdString(typeList[row]));
  }
}

void BuildBEMatrixDialog::pullBoundaryConditions()
{
  using namespace TableColumns;
  Pulling p(this);
  auto bdyList = state_->getValue(Parameters::BoundaryConditionList).toVector();
  const int rows = std::min(static_cast<int>(bdyList.size()), tableWidget->rowCount());
  for (int row = 0; row < rows; ++row)
  {
    auto box = qobject_cast<QComboBox*>(tableWidget->cellWidget(row, BoundaryCondition));
    auto str = QString::fromStdString(bdyList[row].toString());
    box->setCurrentIndex(box->findText(str));
  }
}

void BuildBEMatrixDialog::pullInsides()
{
  using namespace TableColumns;
  Pulling p(this);
  auto condList = state_->getValue(Parameters::InsideConductivityList).toVector();
  const int rows = std::min(static_cast<int>(condList.size()), tableWidget->rowCount());
  for (int row = 0; row < rows; ++row)
  {
    auto box = qobject_cast<QDoubleSpinBox*>(tableWidget->cellWidget(row, InsideConductivity));
    box->setValue(condList[row].toDouble());
  }
}

void BuildBEMatrixDialog::pullOutsides()
{
  using namespace TableColumns;
  Pulling p(this);
  auto condList = state_->getValue(Parameters::OutsideConductivityList).toVector();
  const int rows = std::min(static_cast<int>(condList.size()), tableWidget->rowCount());
  for (int row = 0; row < rows; ++row)
  {
    auto box = qobject_cast<QDoubleSpinBox*>(tableWidget->cellWidget(row, OutsideConductivity));
    box->setValue(condList[row].toDouble());
  }
}
