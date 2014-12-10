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

#include <Interface/Modules/Forward/BuildBEMatrixDialog.h>
#include <Core/Algorithms/Legacy/Forward/BuildBEMatrixAlgo.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Forward;

BuildBEMatrixDialog::BuildBEMatrixDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  tableWidget->resizeColumnsToContents();

  connect(tableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(pushTable(int,int)));
  //connect(SpecifyROI_tabWidget, SIGNAL(cellChanged(int,int)), this, SLOT(push()));
}

void BuildBEMatrixDialog::updateFromPortChange(int numPorts)
{
  std::cout << "updateFromPortChange " << numPorts << std::endl;
  auto oldRowCount = tableWidget->rowCount();
  tableWidget->setRowCount(numPorts - 1);

  for (int i = oldRowCount; i < tableWidget->rowCount(); ++i)
  {
    tableWidget->setItem(i, 0, new QTableWidgetItem("field" + QString::number(i)));
    tableWidget->setItem(i, 1, new QTableWidgetItem("[populated on execute]"));
    tableWidget->setCellWidget(i, 2, makeComboBoxItem(i));
    tableWidget->setCellWidget(i, 3, makeDoubleEntryItem(i, 3));
    tableWidget->setCellWidget(i, 4, makeDoubleEntryItem(i, 4));

    // type is readonly
    auto type = tableWidget->item(i, 1);
    type->setFlags(type->flags() & ~Qt::ItemIsEditable);
  }

  pull();
  tableWidget->resizeColumnsToContents();
}

QComboBox* BuildBEMatrixDialog::makeComboBoxItem(int i) const
{
  QStringList bcList;
  bcList << "Measurement (Neumann)" << "Source (Dirichlet)";
  QComboBox* bcBox = new QComboBox();
  bcBox->addItems(bcList);
  bcBox->setCurrentIndex(i == 0 ? 0 : 1);
  //connect(InputPorts, SIGNAL(currentIndexChanged(int)), this, SLOT(pushComboBoxChange(int)));
  return bcBox;
}

QDoubleSpinBox* BuildBEMatrixDialog::makeDoubleEntryItem(int row, int col) const
{
  auto spin = new QDoubleSpinBox();
  spin->setValue((row + col + 1) % 2);
  //connect(InputPorts, SIGNAL(currentIndexChanged(int)), this, SLOT(pushComboBoxChange(int)));
  return spin;
}

void BuildBEMatrixDialog::pushTable(int row, int col)
{
  if (0 == col)
    pushNames();
  else if (2 == col)
    pushBoundaryConditions();
  else if (3 == col)
    pushInsides();
  else if (4 == col)
    pushOutsides();

  //if (!pulling_)
  //{
  //  QPalette* palette = new QPalette();
  //  palette->setColor(QPalette::Text,Qt::red);
  //  StatisticsTableGroupBox->setPalette(*palette);

  //  /// get user specified ROI data, put it in a DenseMatrix and ship it to state
  //  auto X = SpecifyROI_tabWidget->item(0,0)->text().toDouble();
  //  auto Y = SpecifyROI_tabWidget->item(0,1)->text().toDouble();
  //  auto Z = SpecifyROI_tabWidget->item(0,2)->text().toDouble();
  //  auto material = SpecifyROI_tabWidget->item(0,3)->text().toDouble();
  //  auto radius = SpecifyROI_tabWidget->item(0,4)->text().toDouble();
  //  DenseMatrixHandle specROI(new DenseMatrix(5,1));
  //  (*specROI) << X, Y, Z, material, radius;

  //  state_->setTransientValue(GenerateROIStatisticsAlgorithm::SpecifyROI, specROI);
  //}
}

void BuildBEMatrixDialog::pushNames()
{
  if (!pulling_)
  {
    VariableList names;
    for (int i = 0; i < tableWidget->rowCount(); ++i)
    {
      auto item = tableWidget->item(i, 0);
      std::cout << "pushing name: " << item->text().toStdString() << std::endl;
      names.push_back(makeVariable("", item->text().toStdString()));
    }
    state_->setValue(Parameters::FieldNameList, names);
  }
}

void BuildBEMatrixDialog::pushBoundaryConditions()
{

}

void BuildBEMatrixDialog::pushInsides()
{

}

void BuildBEMatrixDialog::pushOutsides()
{

}

void BuildBEMatrixDialog::pull()
{
  std::cout << "BE pull" << std::endl;
  pullNames();
  //Pulling p(this);
  //auto tableHandle = optional_any_cast_or_default<VariableHandle>(state_->getTransientValue(Parameters::StatisticsTableValues));

  //if (tableHandle)
  //{
  //  auto all_elc_values = tableHandle->toVector();
  //  StatisticsOutput_tableWidget->setRowCount(static_cast<int>(all_elc_values.size()));

  //  //TODO: use this example to improve Variable::List syntactical sugar

  //  /// get the result data from the algorithm and put it in the GUI table
  //  for (int i=0; i<all_elc_values.size(); i++)
  //  {
  //    auto col = (all_elc_values[i]).toVector();

  //    int j = 0;
  //    BOOST_FOREACH(const AlgorithmParameter& ap, col)
  //    {
  //      auto tmpstr = ap.toString();

  //      auto item = new QTableWidgetItem(QString::fromStdString(tmpstr));
  //      item->setFlags(item->flags() & ~Qt::ItemIsEditable);
  //      StatisticsOutput_tableWidget->setItem(i, j, item);
  //      ++j;
  //    }
  //  }
  //}
  //else
  //{
  //  // ?? log a message ? unsure.
  //}

  ///// get the strings PhysicalUnit / CoordinateSpaceLabel from state (directly from module level) and show it above GUI tables
  //std::string PhysicalUnitString = optional_any_cast_or_default<std::string>(state_->getTransientValue(Parameters::PhysicalUnitStr));  /// change GUI Labels due to physical unit and used coordinate space
  //if (!PhysicalUnitString.empty())
  //{
  //  StatisticsTableGroupBox->setTitle("Statistics for ROIs: " + QString::fromStdString(PhysicalUnitString));
  //}

  //std::string CoordinateSpaceLabelStr = optional_any_cast_or_default<std::string>(state_->getTransientValue(Parameters::CoordinateSpaceLabelStr));  /// change GUI Labels due to physical unit and used coordinate space
  //if (!CoordinateSpaceLabelStr.empty())
  //{
  //  ROITableGroupBox->setTitle("Specify ROI: " + QString::fromStdString(CoordinateSpaceLabelStr));
  //}
}

void BuildBEMatrixDialog::pullNames()
{
  Pulling p(this);
  auto nameList = state_->getValue(Parameters::FieldNameList).toVector();
  const int rows = std::min(static_cast<int>(nameList.size()), tableWidget->rowCount());
  for (int row = 0; row < rows; ++row)
  {
    auto item = tableWidget->item(row, 0);
    item->setText(QString::fromStdString(nameList[row].toString()));
  }
}

void BuildBEMatrixDialog::pullFieldTypes()
{

}

void BuildBEMatrixDialog::pullBoundaryConditions()
{

}

void BuildBEMatrixDialog::pullInsides()
{

}

void BuildBEMatrixDialog::pullOutsides()
{

}
