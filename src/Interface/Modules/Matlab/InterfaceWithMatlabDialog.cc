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

#include <Interface/Modules/Matlab/InterfaceWithMatlabDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QFileDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

InterfaceWithMatlabDialog::InterfaceWithMatlabDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  numMatrixPorts_(0), numFieldPorts_(0), numStringPorts_(0)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  WidgetStyleMixin::tabStyle(matlabEngineTabWidget_);
  WidgetStyleMixin::tabStyle(dataTabWidget_);
  WidgetStyleMixin::tabStyle(matrixTabWidget_);
  WidgetStyleMixin::tableHeaderStyle(matrixInputTableWidget_);
  addTextEditManager(matlabCodeTextEdit_, Variables::FunctionString);
}

namespace TableColumns
{
  const int MinColumn = 0;

  const int InputPortID = 0;
  const int InputName = 1;
  const int InputDataType = 2;
  const int InputArrayType = 3;

  const int MaxColumn = InputArrayType + 1;
}

void InterfaceWithMatlabDialog::updateFromPortChange(int numPorts, const std::string& portName)
{
  qDebug() << "ports:" << numPorts << "name: " << QString::fromStdString(portName);

  if (portName.find("Matrix") != std::string::npos)
  {
    qDebug() << "adjust matrix input table";
    auto oldRowCount = matrixInputTableWidget_->rowCount();
    if (numPorts > totalInputPorts())
      numMatrixPorts_++;
    else
      numMatrixPorts_--;
    auto newRowCount = numMatrixPorts_ - 1;
    qDebug() << "oldRowCount" << oldRowCount << "newRowCount" << newRowCount;

    matrixInputTableWidget_->setRowCount(newRowCount);
    matrixInputTableWidget_->blockSignals(true);
    for (int i = oldRowCount; i < matrixInputTableWidget_->rowCount(); ++i)
    {
      using namespace TableColumns;
      matrixInputTableWidget_->setItem(i, InputPortID, new QTableWidgetItem(QString::fromStdString(portName) + QString::number(i+1)));
      matrixInputTableWidget_->setItem(i, InputName, new QTableWidgetItem("i" + QString::number(i+1)));
      matrixInputTableWidget_->setCellWidget(i, InputDataType, makeInputDataTypeComboBoxItem());
      matrixInputTableWidget_->setCellWidget(i, InputArrayType, makeInputArrayTypeComboBoxItem());

      // type is readonly
      auto port = matrixInputTableWidget_->item(i, InputPortID);
      port->setFlags(port->flags() & ~Qt::ItemIsEditable);
      pushTableRow(i);
    }
    pull();
    matrixInputTableWidget_->resizeColumnsToContents();
    matrixInputTableWidget_->blockSignals(false);




  }
  else if (portName.find("Field") != std::string::npos)
  {
    qDebug() << "adjust field input table";
    auto oldRowCount = matrixInputTableWidget_->rowCount();
    if (numPorts > totalInputPorts())
      numFieldPorts_++;
    else
      numFieldPorts_--;
    auto newRowCount = numFieldPorts_ - 1;
    qDebug() << "oldRowCount" << oldRowCount << "newRowCount" << newRowCount;
  }
  else if (portName.find("String") != std::string::npos)
  {
    qDebug() << "adjust string input table";
    auto oldRowCount = matrixInputTableWidget_->rowCount();
    if (numPorts > totalInputPorts())
      numStringPorts_++;
    else
      numStringPorts_--;
    auto newRowCount = numStringPorts_ - 1;
    qDebug() << "oldRowCount" << oldRowCount << "newRowCount" << newRowCount;
  }
  else if (portName.find("Nrrd") != std::string::npos)
  {
    qDebug() << "adjust Nrrd input table";
  }


}

QComboBox* InterfaceWithMatlabDialog::makeInputDataTypeComboBoxItem() const
{
  QStringList bcList;
  bcList << "same as data" << "double" << "etc"; //TODO complete list
  QComboBox* bcBox = new QComboBox();
  bcBox->addItems(bcList);
  bcBox->setCurrentIndex(0);
  connect(bcBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pushMatrixInput()));
  return bcBox;
}


QComboBox* InterfaceWithMatlabDialog::makeInputArrayTypeComboBoxItem() const
{
  QStringList bcList;
  bcList << "numeric array" << "struct array";
  QComboBox* bcBox = new QComboBox();
  bcBox->addItems(bcList);
  bcBox->setCurrentIndex(0);
  connect(bcBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pushMatrixInput()));
  return bcBox;
}

void InterfaceWithMatlabDialog::pushMatrixInput()
{
  qDebug() << "pushMatrixInput";
}

void InterfaceWithMatlabDialog::pushTableRow(int row)
{
  using namespace TableColumns;
  for (int col = MinColumn; col < MaxColumn; ++col)
    pushTable(row, col);
}

void InterfaceWithMatlabDialog::pushTable(int row, int col)
{
  qDebug() << "pushTable";
  using namespace TableColumns;
/*  if (FieldName == col)
    pushNames();
  else if (BoundaryCondition == col)
    pushBoundaryConditions();
  else if (InsideConductivity == col)
    pushInsides();
  else if (OutsideConductivity == col)
    pushOutsides();
    */
}
